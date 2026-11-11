#include "audioengine.h"
#include "isochronicgenerator.h"

#include <QAudioFormat>
#include <QMap>
#include <QtMath>

namespace
{
    double pulseForPreset(const QString &preset)
    {
        static const QMap<QString, double> presets = {
            { QStringLiteral("1 Hz"),     1.0 },
            { QStringLiteral("2 Hz"),     2.0 },
            { QStringLiteral("4 Hz"),     4.0 },
            { QStringLiteral("6 Hz"),     6.0 },
            { QStringLiteral("7.83 Hz"), 7.83 },
            { QStringLiteral("10 Hz"),   10.0 },
            { QStringLiteral("40 Hz"),   40.0 }
        };

        return presets.value(preset, -1.0);
    }

    const int IsochronicFadeOutDuration = 200;

    // Presets always use this carrier, same as the binaural
    // frequency bands.
    const double IsochronicPresetCarrier = 200.0;
}

// ============================================================================
// STATE
// ============================================================================

bool AudioEngine::isIsochronicPlaying() const
{
    return m_isochronicChannel->isPlaying();
}

bool AudioEngine::isIsochronicPaused() const
{
    return m_isochronicChannel->isPaused();
}

// ============================================================================
// VOLUME
// ============================================================================

int AudioEngine::isochronicTargetVolume() const
{
    return qRound(
        // MASTER VOLUME Adjuster Isochronic (pure tones are
        // piercing at full level, keep them gentler)
        m_isochronicVolume * 0.35);
}

void AudioEngine::setIsochronicVolume(int volume)
{
    m_isochronicVolume =
        qBound(0, volume, 100);

    m_isochronicChannel->setVolume(
        static_cast<qreal>(
            isochronicTargetVolume()) / 100.0);
}

// ============================================================================
// ISOCHRONIC
// ============================================================================

/*
 * Preset pulse frequencies (1 .. 40 Hz), carrier fixed at
 * 200 Hz - mirrors setFrequencyBand() of the binaural part:
 * stores the settings, a running generator is updated live,
 * the channel is started separately via startIsochronic().
 */
void AudioEngine::setIsochronicPreset(const QString &preset)
{
    const double pulse = pulseForPreset(preset);
    if (pulse < 0.0)
        return;

    m_isochronicCarrierFrequency = IsochronicPresetCarrier;
    m_isochronicPulseFrequency = pulse;

    if (m_isochronicGenerator)
    {
        m_isochronicGenerator->setCarrierFrequency(
            m_isochronicCarrierFrequency);
        m_isochronicGenerator->setPulseFrequency(
            m_isochronicPulseFrequency);
    }
}

/*
 * Custom carrier + pulse - mirrors setBinauralFrequency().
 * A running generator is updated live (no stream restart).
 */
void AudioEngine::setIsochronicFrequency(double carrier,
                                         double pulse)
{
    m_isochronicCarrierFrequency =
        qBound(20.0, carrier, 1000.0);
    m_isochronicPulseFrequency =
        qBound(0.5, pulse, 40.0);

    if (m_isochronicGenerator)
    {
        m_isochronicGenerator->setCarrierFrequency(
            m_isochronicCarrierFrequency);
        m_isochronicGenerator->setPulseFrequency(
            m_isochronicPulseFrequency);
    }
}

/*
 * Starts the isochronic channel - mirrors the binaural
 * start(): a running fade-out is cancelled, otherwise a new
 * generator is created and fed to the channel.
 */
void AudioEngine::startIsochronic()
{
    if (m_isochronicChannel->isRunning())
    {
        /*
         * Starting while a fade-out is running cancels
         * the fade-out and restores the requested volume.
         */
        if (m_isochronicChannel->isFadingOut())
        {
            m_isochronicChannel->cancelFadeOut();

            m_isochronicChannel->setVolume(
                static_cast<qreal>(
                    isochronicTargetVolume()) / 100.0);
        }

        return;
    }

    auto *generator =
        new IsochronicGenerator(m_isochronicChannel);

    m_isochronicChannel->setVolume(
        static_cast<qreal>(
            isochronicTargetVolume()) / 100.0);

    const bool started = m_isochronicChannel->start(
        generator,
        [this, generator](const QAudioFormat &format)
        {
            generator->setCarrierFrequency(
                m_isochronicCarrierFrequency);
            generator->setPulseFrequency(
                m_isochronicPulseFrequency);
            generator->setSampleRate(format.sampleRate());
            generator->start();
        });

    if (!started)
    {
        delete generator;
        return;
    }

    m_isochronicGenerator = generator;

    updatePlayingState();
}

void AudioEngine::pauseIsochronic()
{
    m_isochronicChannel->pause();

    updatePlayingState();
}

void AudioEngine::resumeIsochronic()
{
    m_isochronicChannel->resume();
}

void AudioEngine::stopIsochronic()
{
    if (!m_isochronicChannel->isRunning())
        return;

    m_isochronicChannel->beginFadeOut(
        IsochronicFadeOutDuration);
}
