#include "audioengine.h"
#include "sequencegenerator.h"
#include "sbgsequence.h"

#include <QAudioFormat>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QStandardPaths>
#include <QtMath>

namespace
{
    const int SequenceFadeOutDuration = 200;

    // Defined in audioengine.h as a class constant:
    //   static constexpr int SequencePollInterval = 250;
}

// ============================================================================
// STATE
// ============================================================================

bool AudioEngine::isSequencePlaying() const
{
    return m_sequenceChannel->isPlaying();
}

bool AudioEngine::isSequencePaused() const
{
    return m_sequenceChannel->isPaused();
}

QStringList AudioEngine::sequencePresets() const
{
    return m_sequencePresets;
}

double AudioEngine::sequenceProgress() const
{
    if (!m_sequenceGenerator)
        return 0.0;

    const qint64 duration =
        m_sequenceGenerator->durationMs();

    if (duration <= 0)
        return 0.0;

    return qBound(
        0.0,
        static_cast<double>(
            m_sequenceGenerator->elapsedMs())
            / duration,
        1.0);
}

int AudioEngine::sequenceRemainingSeconds() const
{
    if (!m_sequenceGenerator)
        return 0;

    const qint64 remainingMs = qMax<qint64>(
        0,
        m_sequenceGenerator->durationMs() -
        m_sequenceGenerator->elapsedMs());

    // Round up so the display doesn't reach zero prematurely.
    return static_cast<int>((remainingMs + 999) / 1000);
}

// ============================================================================
// PRESET SELECTION
// ============================================================================

void AudioEngine::startSequence(const QString &preset)
{
    const QString source =
        m_sequencePresetSources.value(preset);

    if (source.isEmpty())
        return;

    SbgSequence sequence;
    QString error;

    if (!Sbg::parse(source, &sequence, &error))
    {
        qWarning() << "Sequence:" << preset
                   << "parse error:" << error;
        return;
    }

    /*
     * Switching preset while running: the old generator is torn
     * down immediately (the 'stopped' signal clears
     * m_sequenceGenerator) before the new one starts.
     */
    if (m_sequenceChannel->isRunning())
        m_sequenceChannel->stopImmediately();

    auto *generator =
        new SequenceGenerator(sequence, m_sequenceChannel);

    m_sequenceChannel->setVolume(
        m_sequenceVolume / 100.0);

    const bool started = m_sequenceChannel->start(
        generator,
        [generator](const QAudioFormat &format)
        {
            generator->setSampleRate(
                format.sampleRate());

            generator->start();
        });

    if (!started)
    {
        delete generator;
        return;
    }

    m_sequenceGenerator = generator;
    m_activeSequenceName = preset;

    m_sequencePollTimer->start();

    updatePlayingState();
}

void AudioEngine::setCustomSequenceSource(const QString &name,
                                          const QString &source)
{
    if (name.isEmpty())
        return;

    /*
     * Derived custom-focus mixes ("Custom Focus 12", "Custom
     * Focus 21", ...) are transient: only the latest one is
     * kept, so switching the base preset does not accumulate
     * stale entries in the sequence list.
     */
    if (name.startsWith(QStringLiteral("Custom Focus")))
    {
        for (int i = m_sequencePresets.size() - 1; i >= 0; --i)
        {
            const QString &existing = m_sequencePresets.at(i);

            if (existing != name
                && existing.startsWith(
                    QStringLiteral("Custom Focus")))
            {
                m_sequencePresetSources.remove(existing);
                m_sequencePresets.removeAt(i);
            }
        }
    }

    if (!m_sequencePresets.contains(name))
        m_sequencePresets.append(name);

    m_sequencePresetSources.insert(name, source);
}

QVariantList AudioEngine::focusPresetVoices(
    const QString &preset) const
{
    const QString source =
        m_sequencePresetSources.value(preset);

    if (source.isEmpty())
        return QVariantList();

    SbgSequence sequence;

    QString error;

    if (!Sbg::parse(source, &sequence, &error))
        return QVariantList();

    /*
     * The first event whose tone-set contains tone voices
     * describes the mix the preset starts with.
     */
    for (int i = 0; i < sequence.events.size(); ++i)
    {
        const QString &name =
            sequence.events.at(i).toneSetName;

        if (name == QLatin1String("-")
            || name == QLatin1String("off"))
            continue;

        const SbgToneSet *set =
            sequence.toneSetByName(name);

        if (!set)
            continue;

        QVariantList voices;

        for (int v = 0; v < set->voices.size(); ++v)
        {
            const SbgVoice &voice = set->voices.at(v);

            if (voice.type != SbgVoice::Tone)
                continue;

            QVariantMap entry;
            entry.insert(QStringLiteral("carrier"),
                         voice.carrier);
            entry.insert(QStringLiteral("beat"),
                         qAbs(voice.offset));

            voices.append(entry);
        }

        if (!voices.isEmpty())
            return voices;
    }

    return QVariantList();
}

// ============================================================================
// IMPORTED SEQUENCES
// ============================================================================

/*
 * Imported .sbg files are copied into AppDataLocation/sequences
 * and reloaded from there on every start, so imports survive
 * restarts without extra bookkeeping.
 */
QString AudioEngine::importedSequencesDir() const
{
    return QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation)
        + QStringLiteral("/sequences");
}

void AudioEngine::loadImportedSequences()
{
    QDir dir(importedSequencesDir());

    if (!dir.exists())
        return;

    const QStringList files = dir.entryList(
        QStringList() << QStringLiteral("*.sbg")
                      << QStringLiteral("*.txt"),
        QDir::Files,
        QDir::Name);

    for (int i = 0; i < files.size(); ++i)
        registerImportedSequence(files.at(i));
}

/*
 * Reads a stored .sbg file and registers it as a sequence preset.
 * The display name is the file's preset name (its first comment
 * line) or, if the file has none, the file name without
 * extension. Names are kept unique across all presets.
 */
void AudioEngine::registerImportedSequence(const QString &fileName)
{
    QFile file(
        importedSequencesDir()
        + QLatin1Char('/')
        + fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    const QString source = QString::fromUtf8(file.readAll());

    SbgSequence sequence;
    QString error;

    if (!Sbg::parse(source, &sequence, &error))
    {
        qWarning() << "Imported sequence:"
                   << fileName
                   << "parse error:" << error;
        return;
    }

    QString name = sequence.title;

    if (name.isEmpty())
        name = QFileInfo(fileName).completeBaseName();

    const QString baseName = name;
    int suffix = 2;

    while (m_sequencePresetSources.contains(name))
    {
        name = baseName
            + QStringLiteral(" (")
            + QString::number(suffix++)
            + QLatin1Char(')');
    }

    m_sequencePresets.append(name);
    m_sequencePresetSources.insert(name, source);
    m_importedSequenceNames.append(name);
    m_importedSequenceFiles.insert(name, fileName);
}

/*
 * Validates, copies and registers an .sbg file picked by the
 * user. Returns true on success; on failure sequenceImportError
 * carries a user-readable message.
 */
bool AudioEngine::importSequenceFile(const QUrl &fileUrl)
{
    const QString path = fileUrl.isLocalFile()
        ? fileUrl.toLocalFile()
        : fileUrl.toString();

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit sequenceImportError(
            tr("Cannot read file: %1")
            .arg(QFileInfo(path).fileName()));
        return false;
    }

    const QString source = QString::fromUtf8(file.readAll());

    SbgSequence sequence;
    QString error;

    if (!Sbg::parse(source, &sequence, &error))
    {
        emit sequenceImportError(
            tr("Not a valid SBaGen sequence: %1")
            .arg(error));
        return false;
    }

    const QFileInfo info(path);

    // Keep the original name; normalise unknown suffixes to .sbg.
    const QString suffix = info.suffix().toLower();

    const QString fileName =
        (suffix == QLatin1String("sbg")
         || suffix == QLatin1String("txt"))
        ? info.fileName()
        : info.completeBaseName() + QStringLiteral(".sbg");

    QDir dir;

    if (!dir.mkpath(importedSequencesDir()))
    {
        emit sequenceImportError(
            tr("Cannot create the sequences folder"));
        return false;
    }

    QFile target(
        importedSequencesDir()
        + QLatin1Char('/')
        + fileName);

    if (!target.open(
            QIODevice::WriteOnly | QIODevice::Truncate))
    {
        emit sequenceImportError(
            tr("Cannot store the sequence file"));
        return false;
    }

    target.write(source.toUtf8());
    target.close();

    /*
     * Re-importing a previously imported file replaces its entry
     * (otherwise the uniqueness logic would add a suffix).
     */
    const QStringList replaced =
        m_importedSequenceFiles.keys(fileName);

    for (int i = 0; i < replaced.size(); ++i)
    {
        const QString oldName = replaced.at(i);

        m_sequencePresets.removeOne(oldName);
        m_sequencePresetSources.remove(oldName);
        m_importedSequenceNames.removeOne(oldName);
        m_importedSequenceFiles.remove(oldName);
    }

    registerImportedSequence(fileName);

    emit sequencePresetsChanged();

    return true;
}

void AudioEngine::removeImportedSequence(const QString &name)
{
    if (!m_importedSequenceNames.contains(name))
        return;

    if (m_activeSequenceName == name)
    {
        stopSequence();
        m_activeSequenceName.clear();
    }

    const QString fileName =
        m_importedSequenceFiles.value(name);

    if (!fileName.isEmpty())
        QFile::remove(
            importedSequencesDir()
            + QLatin1Char('/')
            + fileName);

    m_sequencePresetSources.remove(name);
    m_importedSequenceFiles.remove(name);
    m_importedSequenceNames.removeOne(name);
    m_sequencePresets.removeOne(name);

    emit sequencePresetsChanged();
}

bool AudioEngine::isImportedSequence(const QString &name) const
{
    return m_importedSequenceNames.contains(name);
}

// ============================================================================
// PAUSE / RESUME / STOP
// ============================================================================

void AudioEngine::pauseSequence()
{
    m_sequenceChannel->pause();

    updatePlayingState();
}

void AudioEngine::resumeSequence()
{
    m_sequenceChannel->resume();
}

void AudioEngine::stopSequence()
{
    if (!m_sequenceChannel->isRunning())
        return;

    m_sequenceChannel->beginFadeOut(
        SequenceFadeOutDuration);
}

// ============================================================================
// VOLUME
// ============================================================================

int AudioEngine::sequenceVolume() const
{
    return m_sequenceVolume;
}

void AudioEngine::setSequenceVolume(int volume)
{
    m_sequenceVolume = qBound(0, volume, 100);

    m_sequenceChannel->setVolume(
        m_sequenceVolume / 100.0);

    emit sequenceVolumeChanged();
}

// ============================================================================
// PROGRESS POLL
// ============================================================================

/*
 * Called from m_sequencePollTimer while a sequence runs: publishes
 * progress to QML and self-stops the channel once a sequence that
 * ends in 'off' has finished fading out.
 */
void AudioEngine::updateSequencePoll()
{
    if (!m_sequenceGenerator)
    {
        m_sequencePollTimer->stop();
        return;
    }

    if (m_sequenceGenerator->finished())
    {
        m_sequencePollTimer->stop();

        m_sequenceChannel->stopImmediately();

        m_activeSequenceName.clear();

        return;
    }

    emit sequenceProgressChanged();
}
