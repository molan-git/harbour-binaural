#include "audioengine.h"
#include "sequencegenerator.h"
#include "sbgsequence.h"

#include <QAudioFormat>
#include <QDebug>
#include <QHash>
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

    m_sequencePollTimer->start();

    updatePlayingState();
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

void AudioEngine::setSequenceVolume(int volume)
{
    m_sequenceVolume = qBound(0, volume, 100);

    m_sequenceChannel->setVolume(
        m_sequenceVolume / 100.0);
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

        return;
    }

    emit sequenceProgressChanged();
}
