#include "audioengine.h"

#include <QtMath>
#include <QTimer>

namespace
{
    // Total duration (ms) of the sleep-timer fade-out before all audio stops.
    const int SleepTimerFadeOutDuration = 8000;
    // Tick interval (ms) for the sleep-timer fade-out; smaller = smoother fade.
    const int FadeInterval = 15;
    // Debounce delay for applying pending colored-noise volume changes (ms).
    const int VolumeUpdateInterval = 50;
}

// Manages all audio channels (binaural, colored noise, solfeggio,
// ambience) and handles the sleep-timer fade-out.
AudioEngine::AudioEngine(QObject *parent)
    : QObject(parent),
      m_binauralChannel(nullptr),
      m_noiseChannel(nullptr),
      m_solfeggioChannel(nullptr),
      m_isochronicChannel(nullptr),
      m_binauralGenerator(nullptr),
      m_noiseGenerator(nullptr),
      m_solfeggioGenerator(nullptr),
      m_isochronicGenerator(nullptr),
      m_customChannel(nullptr),
      m_sleepTimerFadeTimer(new QTimer(this)),
      m_sleepTimerBinauralStartVolume(0.0),
      m_sleepTimerColoredNoiseStartVolume(0.0),
      m_sleepTimerSolfeggioStartVolume(0.0),
      m_sleepTimerIsochronicStartVolume(0.0),
      m_sleepTimerCustomStartVolume(0),
      m_playlistIndex(0),
      m_playlistLoop(false),
      m_playlistActive(false),
      m_customVolume(50),
      m_customLoop(true),
      m_customCrossfade(true),
      m_customManualStop(false),
      m_coloredNoiseVolumeTimer(new QTimer(this)),
      m_carrierFrequency(200.0),
      m_beatFrequency(10.0),
      m_isochronicCarrierFrequency(200.0),
      m_isochronicPulseFrequency(10.0),
      m_binauralVolume(50),
      m_ambienceVolume(50),
      m_coloredNoiseVolume(50),
      m_solfeggioVolume(50),
      m_isochronicVolume(50),
      m_noiseWaveEnabled(false),
      m_noiseWaveDepth(0.0),
      m_noiseWavePeriod(8.0)
{
    // Pre-allocate ambience channels and wire up state/finish signals.
    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        m_ambienceChannels[i] = new AmbienceChannel(this);
        m_sleepTimerAmbienceStartVolumes[i] = 0;

        m_ambienceChannels[i]->setVolume(m_ambienceVolume);

        connect(m_ambienceChannels[i],
                &AmbienceChannel::stateChanged,
                this,
                &AudioEngine::updateAmbienceState);

        // Rebalance the remaining channels when one ends,
        // dies (watchdog) or is released (invalid media).
        connect(m_ambienceChannels[i],
                &AmbienceChannel::finished,
                this,
                [this]() {
                    updateAmbienceMixScale();
                    updatePlayingState();
                });
    }

    // Custom sound channel: same bulletproof AmbienceChannel
    // machinery as the preset ambiences.
    m_customChannel = new AmbienceChannel(this);
    m_customChannel->setVolume(m_customVolume);

    connect(m_customChannel,
            &AmbienceChannel::stateChanged,
            this,
            &AudioEngine::updatePlayingState);

    connect(m_customChannel,
            &AmbienceChannel::finished,
            this,
            [this]() {
                handleCustomTrackFinished();
            });

    m_sleepTimerFadeTimer->setInterval(FadeInterval);

    m_coloredNoiseVolumeTimer->setInterval(VolumeUpdateInterval);
    m_coloredNoiseVolumeTimer->setSingleShot(true);

    m_binauralChannel =
        new PcmChannel(QStringLiteral("binaural"),
                       m_binauralVolume / 100.0,
                       this);

    m_noiseChannel =
        new PcmChannel(QStringLiteral("colored noise"),
                       static_cast<qreal>(
                           coloredNoiseTargetVolume()) / 100.0,
                       this);

    m_solfeggioChannel =
        new PcmChannel(QStringLiteral("solfeggio"),
                       static_cast<qreal>(
                           solfeggioTargetVolume()) / 100.0,
                       this);

    m_isochronicChannel =
        new PcmChannel(QStringLiteral("isochronic"),
                       static_cast<qreal>(
                           isochronicTargetVolume()) / 100.0,
                       this);

    connect(m_isochronicChannel,
            &PcmChannel::stateChanged,
            this,
            &AudioEngine::updatePlayingState);

    connect(m_isochronicChannel,
            &PcmChannel::stopped,
            this,
            [this]() {
                m_isochronicGenerator = nullptr;
            });

    connect(m_binauralChannel,
            &PcmChannel::stateChanged,
            this,
            &AudioEngine::updatePlayingState);

    connect(m_noiseChannel,
            &PcmChannel::stateChanged,
            this,
            &AudioEngine::updatePlayingState);

    connect(m_solfeggioChannel,
            &PcmChannel::stateChanged,
            this,
            &AudioEngine::updatePlayingState);

    // Clean up generator pointers when a channel is fully stopped.
    connect(m_binauralChannel,
            &PcmChannel::stopped,
            this,
            [this]() {
                m_binauralGenerator = nullptr;
            });

    connect(m_noiseChannel,
            &PcmChannel::stopped,
            this,
            [this]() {
                m_noiseGenerator = nullptr;
                m_currentColoredNoise.clear();
                m_coloredNoiseVolumeTimer->stop();
            });

    connect(m_solfeggioChannel,
            &PcmChannel::stopped,
            this,
            [this]() {
                m_solfeggioGenerator = nullptr;
                m_solfeggioFrequencies.clear();
            });

    connect(m_coloredNoiseVolumeTimer,
            &QTimer::timeout,
            this,
            [this]() {
                applyPendingColoredNoiseVolume();
            });

    connect(m_sleepTimerFadeTimer,
            &QTimer::timeout,
            this,
            [this]() {
                const double progress = qMin(
                        1.0,
                        static_cast<double>(
                            m_sleepTimerFadeElapsed.elapsed())
                        / SleepTimerFadeOutDuration);

                // Smoothstep (ease-in-out): zero slope at both ends,
                // gentle for a sleep fade.
                const double p = progress;
                const double factor = 1.0 - (3.0 * p * p - 2.0 * p * p * p);

                m_binauralChannel->setVolume(
                            m_sleepTimerBinauralStartVolume * factor);

                for (int i = 0; i < MaxAmbienceChannels; ++i) {
                    if (m_ambienceChannels[i]) {
                        m_ambienceChannels[i]->applySleepVolume(
                                    m_sleepTimerAmbienceStartVolumes[i],
                                    factor);
                    }
                }

                // Custom sound fades with the sleep timer too.
                if (m_customChannel) {
                    m_customChannel->applySleepVolume(
                                m_sleepTimerCustomStartVolume,
                                factor);
                }

                m_noiseChannel->setVolume(
                            m_sleepTimerColoredNoiseStartVolume * factor);

                m_solfeggioChannel->setVolume(
                            m_sleepTimerSolfeggioStartVolume * factor);

                m_isochronicChannel->setVolume(
                            m_sleepTimerIsochronicStartVolume * factor);

                if (progress >= 1.0) {
                    m_sleepTimerFadeTimer->stop();

                    m_binauralChannel->stopImmediately();
                    stopAmbiencesImmediately();
                    m_customChannel->stopImmediately();
                    m_noiseChannel->stopImmediately();
                    m_solfeggioChannel->stopImmediately();
                    m_isochronicChannel->stopImmediately();

                    updatePlayingState();
                }
            });
}

// Ensure all audio is stopped before the engine is torn down.
AudioEngine::~AudioEngine()
{
    m_sleepTimerFadeTimer->stop();

    m_binauralChannel->stopImmediately();
    stopAmbiencesImmediately();
    m_customChannel->stopImmediately();
    m_noiseChannel->stopImmediately();
    m_solfeggioChannel->stopImmediately();
    m_isochronicChannel->stopImmediately();
}

void AudioEngine::updatePlayingState()
{
    emit playingChanged();
    emit customSoundChanged();
}

void AudioEngine::updateAmbienceState()
{
    updatePlayingState();
}

// Captures current channel volumes as the fade-out starting point,
// then begins the linear ramp-down over SleepTimerFadeOutDuration.
void AudioEngine::fadeOutForSleepTimer()
{
    m_sleepTimerFadeTimer->stop();

    m_sleepTimerBinauralStartVolume =
            m_binauralChannel->volume();

    m_sleepTimerColoredNoiseStartVolume =
            m_noiseChannel->volume();

    m_sleepTimerSolfeggioStartVolume =
            m_solfeggioChannel->volume();

    m_sleepTimerIsochronicStartVolume =
            m_isochronicChannel->volume();

    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        m_sleepTimerAmbienceStartVolumes[i] =
                m_ambienceChannels[i]
                ? m_ambienceChannels[i]->currentVolume()
                : 0;
    }

    m_sleepTimerCustomStartVolume =
            m_customChannel->currentVolume();

    m_sleepTimerFadeElapsed.start();
    m_sleepTimerFadeTimer->start();
}
