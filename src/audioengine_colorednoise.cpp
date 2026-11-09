#include "audioengine.h"
#include "colorednoisegenerator.h"

#include <QAudioFormat>
#include <QtMath>

namespace
{
    const int ColoredNoiseFadeOutDuration = 200;
}

// ============================================================================
// STATE
// ============================================================================

bool AudioEngine::isColoredNoisePlaying() const
{
    return m_noiseChannel->isPlaying();
}

bool AudioEngine::isColoredNoisePaused() const
{
    return m_noiseChannel->isPaused();
}

// ============================================================================
// RESUME / VOLUME
// ============================================================================

void AudioEngine::resumeColoredNoise()
{
    m_noiseChannel->resume();
}

void AudioEngine::setColoredNoiseVolume(int volume)
{
    m_coloredNoiseVolume =
        qBound(0, volume, 100);

    m_coloredNoiseVolumeTimer->start();
}

int AudioEngine::coloredNoiseTargetVolume() const
{
    return qRound(
        // MASTER VOLUME Adjuster Colored Noise
        m_coloredNoiseVolume * 0.4);
}

void AudioEngine::applyPendingColoredNoiseVolume()
{
    m_noiseChannel->setVolume(
        static_cast<qreal>(
            coloredNoiseTargetVolume()) / 100.0);
}

void AudioEngine::setNoiseWave(bool enabled,
                               double depth,
                               double periodSeconds)
{
    m_noiseWaveEnabled = enabled;
    m_noiseWaveDepth =
        enabled ? qBound(0.0, depth, 1.0) : 0.0;
    m_noiseWavePeriod = qBound(0.5, periodSeconds, 60.0);

    if (m_noiseGenerator)
        m_noiseGenerator->setWaveModulation(
            m_noiseWaveDepth,
            m_noiseWavePeriod);
}

// ============================================================================
// COLORED NOISE
// ============================================================================

void AudioEngine::setColoredNoise(
    const QString &noise)
{
    ColoredNoiseGenerator::NoiseType type;

    if (noise == "White")
    {
        type = ColoredNoiseGenerator::WhiteNoise;
    }
    else if (noise == "Pink")
    {
        type = ColoredNoiseGenerator::PinkNoise;
    }
    else if (noise == "Brown")
    {
        type = ColoredNoiseGenerator::BrownNoise;
    }
    else if (noise == "Grey")
    {
        type = ColoredNoiseGenerator::GreyNoise;
    }
    else
    {
        return;
    }

    /*
     * If noise is already running, only change the
     * algorithm. The audio stream itself remains alive.
     */
    if (m_noiseChannel->isRunning())
    {
        m_noiseChannel->cancelFadeOut();

        m_noiseGenerator->setNoiseType(type);       

        m_noiseGenerator->setWaveModulation(
            m_noiseWaveEnabled ? m_noiseWaveDepth : 0.0,
            m_noiseWavePeriod);

        m_noiseChannel->setVolume(
            static_cast<qreal>(
                coloredNoiseTargetVolume()) / 100.0);

        m_currentColoredNoise = noise;
        updatePlayingState();
        return;
    }

    auto *generator =
        new ColoredNoiseGenerator(m_noiseChannel);

    generator->setNoiseType(type);

    generator->setWaveModulation(
        m_noiseWaveEnabled ? m_noiseWaveDepth : 0.0,
        m_noiseWavePeriod);

    m_noiseChannel->setVolume(
        static_cast<qreal>(
            coloredNoiseTargetVolume()) / 100.0);

    const bool started = m_noiseChannel->start(
        generator,
        [generator](const QAudioFormat &format)
        {
            generator->setSampleRate(
                format.sampleRate());

            generator->setChannelCount(
                format.channelCount());

            generator->start();
        });

    if (!started)
    {
        delete generator;
        return;
    }

    m_noiseGenerator = generator;

    m_currentColoredNoise = noise;

    updatePlayingState();
}

void AudioEngine::pauseColoredNoise()
{
    m_noiseChannel->pause();

    updatePlayingState();
}

void AudioEngine::stopColoredNoise()
{
    if (!m_noiseChannel->isRunning())
        return;

    m_noiseChannel->beginFadeOut(
                ColoredNoiseFadeOutDuration);
}
