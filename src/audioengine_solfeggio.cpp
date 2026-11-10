#include "audioengine.h"
#include "solfeggiogenerator.h"

#include <QAudioFormat>
#include <QtMath>
#include <QVector>

namespace
{
    const int SolfeggioFadeOutDuration = 200;

    // The nine traditional solfeggio frequencies (Hz).
    const QVector<double> SolfeggioFrequencies = {
        174.0, 285.0, 396.0, 417.0, 528.0,
        639.0, 741.0, 852.0, 963.0
    };

    bool isSolfeggioFrequency(double frequency)
    {
        return SolfeggioFrequencies.contains(frequency);
    }
}

// ============================================================================
// STATE
// ============================================================================

bool AudioEngine::isSolfeggioPlaying() const
{
    return m_solfeggioChannel->isPlaying();
}

bool AudioEngine::isSolfeggioPaused() const
{
    return m_solfeggioChannel->isPaused();
}

// ============================================================================
// VOLUME
// ============================================================================

int AudioEngine::solfeggioTargetVolume() const
{
    return qRound(
        // MASTER VOLUME Adjuster Solfeggio (pure tones are
        // piercing at full level, keep them gentler)
        m_solfeggioVolume * 0.35);
}

void AudioEngine::setSolfeggioVolume(int volume)
{
    m_solfeggioVolume =
        qBound(0, volume, 100);

    m_solfeggioChannel->setVolume(
        static_cast<qreal>(
            solfeggioTargetVolume()) / 100.0);
}

// ============================================================================
// SOLFEGGIO
// ============================================================================

/*
 * Receives the selected frequencies (in Hz) from QML.
 * An empty list stops the channel.
 *
 * If the channel is already running, only the generator
 * content changes - the audio stream stays alive (same
 * behavior as setColoredNoise()).
 */
void AudioEngine::setSolfeggio(
    const QVariantList &frequencies)
{
    QVector<double> selected;

    for (const QVariant &variant : frequencies)
    {
        const double frequency = variant.toDouble();

        if (isSolfeggioFrequency(frequency)
            && !selected.contains(frequency))
        {
            selected.append(frequency);
        }
    }

    m_solfeggioFrequencies = selected;

    if (selected.isEmpty())
    {
        stopSolfeggio();
        return;
    }

    if (m_solfeggioChannel->isRunning())
    {
        m_solfeggioChannel->cancelFadeOut();

        m_solfeggioGenerator->setFrequencies(selected);

        m_solfeggioChannel->setVolume(
            static_cast<qreal>(
                solfeggioTargetVolume()) / 100.0);

        updatePlayingState();
        return;
    }

    auto *generator =
        new SolfeggioGenerator(m_solfeggioChannel);

    generator->setFrequencies(selected);

    m_solfeggioChannel->setVolume(
        static_cast<qreal>(
            solfeggioTargetVolume()) / 100.0);

    const bool started = m_solfeggioChannel->start(
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

    m_solfeggioGenerator = generator;

    updatePlayingState();
}

void AudioEngine::pauseSolfeggio()
{
    m_solfeggioChannel->pause();

    updatePlayingState();
}

void AudioEngine::resumeSolfeggio()
{
    m_solfeggioChannel->resume();
}

void AudioEngine::stopSolfeggio()
{
    if (!m_solfeggioChannel->isRunning())
        return;

    m_solfeggioChannel->beginFadeOut(
        SolfeggioFadeOutDuration);
}
