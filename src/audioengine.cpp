#include "audioengine.h"

#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QDebug>
#include <QMediaContent>
#include <QUrl>
#include <QtMath>

#include <cstring>

namespace
{
    const int DefaultSampleRate = 48000;
    const int ChannelCount = 2;
    const int SampleSize = 16;

    const double CarrierFrequency = 200.0;
    const double Amplitude = 16000.0;

    const int BinauralFadeInDuration = 0;

    const int AmbienceFadeInDuration = 200;
    const int AmbienceCrossfadeDuration = 8000;

    const int ColoredNoiseFadeInDuration = 200;
    const int ColoredNoiseCrossfadeDuration = 4000;

    const int FadeInterval = 15;
    const int ColoredNoiseWatchdogInterval = 2000;

    const int BinauralFadeOutDuration = 0;
    const int AmbienceFadeOutDuration = 200;
    const int ColoredNoiseFadeOutDuration = 200;

    // Rapid setVolume() calls in a row (slider drags) can stall the
    // GStreamer pipeline on SFOS 4.6 / Qt 5.6. Coalesce them: one real
    // update per interval, the newest value wins.
    const int VolumeUpdateInterval = 50;
}

class AudioGenerator : public QIODevice
{
public:
    explicit AudioGenerator(QObject *parent = nullptr)
        : QIODevice(parent),
          m_sampleRate(DefaultSampleRate),
          m_leftPhase(0.0),
          m_rightPhase(0.0),
          m_beatFrequency(10.0)
    {
    }

    void start()
    {
        open(QIODevice::ReadOnly);
    }

    void stop()
    {
        close();
        m_leftPhase = 0.0;
        m_rightPhase = 0.0;
    }

    void setBeatFrequency(double frequency)
    {
        m_beatFrequency = frequency;
    }

    // SFOS 4.6: if the device hands back a different format (e.g. 44100 Hz)
    // via nearestFormat(), the generator must follow, otherwise the carrier
    // and beat frequency detune.
    void setSampleRate(int sampleRate)
    {
        if (sampleRate > 0)
            m_sampleRate = static_cast<double>(sampleRate);
    }

protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        const int bytesPerFrame = 4; // stereo, 16 bit signed PCM
        const qint64 frameCount = maxlen / bytesPerFrame;

        qint16 *samples = reinterpret_cast<qint16 *>(data);

        const double leftFrequency = CarrierFrequency;
        const double rightFrequency = CarrierFrequency + m_beatFrequency;

        for (qint64 frame = 0; frame < frameCount; ++frame)
        {
            const double leftValue =
                qSin(m_leftPhase * 2.0 * M_PI);

            const double rightValue =
                qSin(m_rightPhase * 2.0 * M_PI);

            samples[frame * 2] =
                static_cast<qint16>(
                    leftValue * Amplitude);

            samples[frame * 2 + 1] =
                static_cast<qint16>(
                    rightValue * Amplitude);

            m_leftPhase +=
                leftFrequency / m_sampleRate;

            m_rightPhase +=
                rightFrequency / m_sampleRate;

            if (m_leftPhase >= 1.0)
                m_leftPhase -= 1.0;

            if (m_rightPhase >= 1.0)
                m_rightPhase -= 1.0;
        }

        // Zero-fill a trailing partial frame instead of dropping it.
        const qint64 remainder =
            maxlen - frameCount * bytesPerFrame;

        if (remainder > 0)
            std::memset(data + frameCount * bytesPerFrame, 0,
                        static_cast<size_t>(remainder));

        return maxlen;
    }

    qint64 writeData(const char *, qint64) override
    {
        return 0;
    }

private:
    double m_sampleRate;
    double m_leftPhase;
    double m_rightPhase;
    double m_beatFrequency;
};

AudioEngine::AudioEngine(QObject *parent)
    : QObject(parent),
      m_audioOutput(nullptr),
      m_generator(nullptr),
      m_ambiencePlayerA(new QMediaPlayer(this)),
      m_ambiencePlayerB(new QMediaPlayer(this)),
      m_coloredNoisePlayerA(new QMediaPlayer(this)),
      m_coloredNoisePlayerB(new QMediaPlayer(this)),
      m_binauralFadeTimer(new QTimer(this)),
      m_ambienceFadeInTimer(new QTimer(this)),
      m_ambienceCrossfadeTimer(new QTimer(this)),
      m_coloredNoiseFadeInTimer(new QTimer(this)),
      m_coloredNoiseCrossfadeTimer(new QTimer(this)),
      m_coloredNoiseWatchdogTimer(new QTimer(this)),
      m_binauralFadeOutTimer(new QTimer(this)),
      m_ambienceFadeOutTimer(new QTimer(this)),
      m_coloredNoiseFadeOutTimer(new QTimer(this)),
      m_ambienceVolumeTimer(new QTimer(this)),
      m_coloredNoiseVolumeTimer(new QTimer(this)),
      m_activeAmbiencePlayer(nullptr),
      m_fadingAmbiencePlayer(nullptr),
      m_activeColoredNoisePlayer(nullptr),
      m_fadingColoredNoisePlayer(nullptr),
      m_beatFrequency(10.0),
      m_binauralVolume(50),
      m_ambienceVolume(50),
      m_coloredNoiseVolume(50),
      m_binauralFadeInPosition(0),
      m_ambienceFadeInPosition(0),
      m_ambienceCrossfadePosition(0),
      m_ambienceCrossfadeDuration(
          AmbienceCrossfadeDuration),
      m_coloredNoiseFadeInPosition(0),
      m_coloredNoiseCrossfadePosition(0),
      m_coloredNoiseCrossfadeDuration(
          ColoredNoiseCrossfadeDuration),
      m_binauralFadeOutStartVolume(0.0),
      m_ambienceFadeOutActiveVolume(0),
      m_ambienceFadeOutFadingVolume(0),
      m_coloredNoiseFadeOutActiveVolume(0),
      m_coloredNoiseFadeOutFadingVolume(0)
{
    m_binauralFadeTimer->setInterval(
        FadeInterval);

    m_ambienceFadeInTimer->setInterval(
        FadeInterval);

    m_ambienceCrossfadeTimer->setInterval(
        FadeInterval);

    m_coloredNoiseFadeInTimer->setInterval(
        FadeInterval);

    m_coloredNoiseCrossfadeTimer->setInterval(
        FadeInterval);

    m_coloredNoiseWatchdogTimer->setInterval(
        ColoredNoiseWatchdogInterval);

    m_binauralFadeOutTimer->setInterval(
        FadeInterval);

    m_ambienceFadeOutTimer->setInterval(
        FadeInterval);

    m_coloredNoiseFadeOutTimer->setInterval(
        FadeInterval);

    m_ambienceVolumeTimer->setInterval(
        VolumeUpdateInterval);
    m_ambienceVolumeTimer->setSingleShot(true);

    m_coloredNoiseVolumeTimer->setInterval(
        VolumeUpdateInterval);
    m_coloredNoiseVolumeTimer->setSingleShot(true);

    connectAmbiencePlayer(m_ambiencePlayerA);
    connectAmbiencePlayer(m_ambiencePlayerB);
    connectColoredNoisePlayer(m_coloredNoisePlayerA);
    connectColoredNoisePlayer(m_coloredNoisePlayerB);

    connect(
        m_binauralFadeTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateBinauralFadeIn();
        });

    connect(
        m_ambienceFadeInTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateAmbienceFadeIn();
        });

    connect(
        m_ambienceCrossfadeTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateAmbienceCrossfade();
        });

    connect(
        m_coloredNoiseFadeInTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateColoredNoiseFadeIn();
        });

    connect(
        m_coloredNoiseCrossfadeTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateColoredNoiseCrossfade();
        });

    connect(
        m_coloredNoiseWatchdogTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateColoredNoiseWatchdog();
        });

    connect(
        m_binauralFadeOutTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateBinauralFadeOut();
        });

    connect(
        m_ambienceFadeOutTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateAmbienceFadeOut();
        });

    connect(
        m_coloredNoiseFadeOutTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateColoredNoiseFadeOut();
        });

    connect(
        m_ambienceVolumeTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            applyPendingAmbienceVolume();
        });

    connect(
        m_coloredNoiseVolumeTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            applyPendingColoredNoiseVolume();
        });

    m_ambiencePlayerA->setVolume(0);
    m_ambiencePlayerB->setVolume(0);

    m_coloredNoisePlayerA->setVolume(0);
    m_coloredNoisePlayerB->setVolume(0);
}

AudioEngine::~AudioEngine()
{
    // The destructor cannot wait for fade-out timers.
    stopBinauralImmediately();
    stopAmbienceImmediately();
    stopColoredNoiseImmediately();
}

void AudioEngine::connectAmbiencePlayer(
    QMediaPlayer *player)
{
    connect(
        player,
        &QMediaPlayer::positionChanged,
        this,
        [this, player](qint64)
        {
            checkAmbienceCrossfade(player);
        });

    // Recovery + diagnostics: without this, EndOfMedia or a GStreamer
    // error leaves the engine bookkeeping intact but produces silence.
    connect(
        player,
        &QMediaPlayer::mediaStatusChanged,
        this,
        [this, player](QMediaPlayer::MediaStatus status)
        {
            if (status == QMediaPlayer::EndOfMedia)
            {
                handleAmbienceEndOfMedia(player);
            }
            else if (status == QMediaPlayer::InvalidMedia)
            {
                qWarning() << "AudioEngine: ambience media invalid:"
                           << player->media().canonicalUrl().toString();
            }
        });

    connect(
        player,
        static_cast<void (QMediaPlayer::*)(QMediaPlayer::Error)>(
            &QMediaPlayer::error),
        this,
        [player](QMediaPlayer::Error error)
        {
            qWarning() << "AudioEngine: ambience player error:"
                       << error
                       << player->errorString();
        });

    // Qt 5.6 (SFOS): setVolume() calls made around play() can get lost in
    // the GStreamer backend. Re-apply the target volume once the player is
    // actually running.
    connect(
        player,
        &QMediaPlayer::stateChanged,
        this,
        [this, player](QMediaPlayer::State state)
        {
            if (state != QMediaPlayer::PlayingState)
                return;

            if (player != m_activeAmbiencePlayer)
                return;

            if (m_ambienceCrossfadeTimer->isActive())
                return; // crossfade owns the volume right now

            if (m_ambienceFadeOutTimer->isActive())
                return; // fade-out owns the volume right now

            if (AmbienceFadeInDuration > 0
                && m_ambienceFadeInTimer->isActive())
                return; // fade-in owns the volume right now

            player->setVolume(ambienceTargetVolume());
        });
}

void AudioEngine::connectColoredNoisePlayer(
    QMediaPlayer *player)
{
    connect(
        player,
        &QMediaPlayer::positionChanged,
        this,
        [this, player](qint64)
        {
            checkColoredNoiseCrossfade(player);
        });

    connect(
        player,
        &QMediaPlayer::mediaStatusChanged,
        this,
        [this, player](QMediaPlayer::MediaStatus status)
        {
            if (status == QMediaPlayer::EndOfMedia)
            {
                handleColoredNoiseEndOfMedia(player);
            }
            else if (status == QMediaPlayer::InvalidMedia)
            {
                qWarning() << "AudioEngine: colored noise media invalid:"
                           << player->media().canonicalUrl().toString();
            }
        });

    connect(
        player,
        static_cast<void (QMediaPlayer::*)(QMediaPlayer::Error)>(
            &QMediaPlayer::error),
        this,
        [this, player](QMediaPlayer::Error error)
        {
            qWarning() << "AudioEngine: colored noise player error:"
                       << error
                       << player->errorString();

            // A failed pipeline will not come back on its own. Restart
            // it immediately (the watchdog covers the StoppedState case).
            if (player != m_activeColoredNoisePlayer)
                return;

            if (m_coloredNoiseFadeOutTimer->isActive())
                return;

            // Qt 5.6 error enum: ResourceError, FormatError, NetworkError,
            // AccessDeniedError, ServiceMissingError, MediaIsPlaylist.
            // Restart on everything where a retry can help; FormatError
            // would just fail again, so do not loop on it.
            if (error == QMediaPlayer::ResourceError ||
                error == QMediaPlayer::NetworkError ||
                error == QMediaPlayer::AccessDeniedError ||
                error == QMediaPlayer::ServiceMissingError)
            {
                const QMediaContent media = player->media();

                player->setMedia(media);
                player->setPosition(0);
                player->setVolume(coloredNoiseTargetVolume());
                player->play();
            }
        });

    // Qt 5.6 (SFOS): setVolume() calls made around play() can get lost in
    // the GStreamer backend. Re-apply the target volume once the player is
    // actually running.
    connect(
        player,
        &QMediaPlayer::stateChanged,
        this,
        [this, player](QMediaPlayer::State state)
        {
            if (state != QMediaPlayer::PlayingState)
                return;

            if (player != m_activeColoredNoisePlayer)
                return;

            if (m_coloredNoiseCrossfadeTimer->isActive())
                return; // crossfade owns the volume right now

            if (m_coloredNoiseFadeOutTimer->isActive())
                return; // fade-out owns the volume right now

            if (ColoredNoiseFadeInDuration > 0
                && m_coloredNoiseFadeInTimer->isActive())
                return; // fade-in owns the volume right now

            player->setVolume(coloredNoiseTargetVolume());
        });
}

int AudioEngine::ambienceTargetVolume() const
{
    return qRound(m_ambienceVolume * 0.6);
}

int AudioEngine::coloredNoiseTargetVolume() const
{
    return qRound(m_coloredNoiseVolume * 0.3);
}

void AudioEngine::setFrequencyBand(const QString &band)
{
    if (band == "Delta")
        m_beatFrequency = 2.0;
    else if (band == "Delta/Theta")
        m_beatFrequency = 4.0;
    else if (band == "Theta")
        m_beatFrequency = 6.0;
    else if (band == "Alpha")
        m_beatFrequency = 10.0;
    else if (band == "Beta")
        m_beatFrequency = 20.0;
    else if (band == "Gamma")
        m_beatFrequency = 40.0;
    else
        return;

    if (m_generator)
        m_generator->setBeatFrequency(
            m_beatFrequency);
}

void AudioEngine::setBinauralVolume(int volume)
{
    volume = qBound(0, volume, 100);

    m_binauralVolume = volume;

    if (m_audioOutput &&
        !m_binauralFadeTimer->isActive() &&
        !m_binauralFadeOutTimer->isActive())
    {
        m_audioOutput->setVolume(
            m_binauralVolume / 100.0);
    }
}

void AudioEngine::setAmbienceVolume(int volume)
{
    volume = qBound(0, volume, 100);

    m_ambienceVolume = volume;

    // Coalesce rapid updates: only the newest value is actually applied,
    // at most one real setVolume() every VolumeUpdateInterval ms.
    m_ambienceVolumeTimer->start();
}

void AudioEngine::applyPendingAmbienceVolume()
{
    if (m_activeAmbiencePlayer &&
        !m_ambienceFadeInTimer->isActive() &&
        !m_ambienceCrossfadeTimer->isActive() &&
        !m_ambienceFadeOutTimer->isActive())
    {
        m_activeAmbiencePlayer->setVolume(
            ambienceTargetVolume());
    }
}

void AudioEngine::setColoredNoiseVolume(int volume)
{
    volume = qBound(0, volume, 100);

    m_coloredNoiseVolume = volume;

    // Coalesce rapid updates: only the newest value is actually applied,
    // at most one real setVolume() every VolumeUpdateInterval ms.
    m_coloredNoiseVolumeTimer->start();
}

void AudioEngine::applyPendingColoredNoiseVolume()
{
    if (m_activeColoredNoisePlayer &&
        !m_coloredNoiseFadeInTimer->isActive() &&
        !m_coloredNoiseCrossfadeTimer->isActive() &&
        !m_coloredNoiseFadeOutTimer->isActive())
    {
        m_activeColoredNoisePlayer->setVolume(
            coloredNoiseTargetVolume());
    }
}

void AudioEngine::setAmbience(
    const QString &ambience)
{
    QUrl mediaUrl;

    if (ambience == "Wind")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/wind.mp3");
    else if (ambience == "Waves")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/waves.mp3");
    else if (ambience == "Crickets")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/crickets.mp3");
    else if (ambience == "Stream")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/stream.mp3");
    else if (ambience == "Rain")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/rain.mp3");
    else if (ambience == "Birds")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/birds.mp3");
    else if (ambience == "Fire")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/fire.mp3");
    else if (ambience == "Chimes")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/chimes.mp3");
    else
        return;

    // Switching ambience: immediate stop, the new ambience has its own
    // fade-in. A graceful fade-out happens in stopAmbience().
    stopAmbienceImmediately();

    m_activeAmbiencePlayer =
        m_ambiencePlayerA;

    m_activeAmbiencePlayer->setMedia(
        mediaUrl);

    m_activeAmbiencePlayer->setVolume(0);

    m_ambienceFadeInPosition = 0;

    m_activeAmbiencePlayer->play();

    m_ambienceFadeInTimer->start();
}

void AudioEngine::stopAmbience()
{
    if (!m_activeAmbiencePlayer)
    {
        stopAmbienceImmediately();
        return;
    }

    if (m_ambienceFadeOutTimer->isActive())
        return; // already fading out

    m_ambienceFadeInTimer->stop();
    m_ambienceCrossfadeTimer->stop();

    // If a crossfade was running, both players are faded out together.
    m_ambienceFadeOutActiveVolume =
        m_activeAmbiencePlayer->volume();

    m_ambienceFadeOutFadingVolume =
        m_fadingAmbiencePlayer
            ? m_fadingAmbiencePlayer->volume()
            : 0;

    m_ambienceFadeOutElapsed.start();

    m_ambienceFadeOutTimer->start();
}

void AudioEngine::stopAmbienceImmediately()
{
    m_ambienceFadeInTimer->stop();
    m_ambienceCrossfadeTimer->stop();
    m_ambienceFadeOutTimer->stop();
    m_ambienceVolumeTimer->stop();

    m_ambiencePlayerA->stop();
    m_ambiencePlayerB->stop();

    m_ambiencePlayerA->setVolume(0);
    m_ambiencePlayerB->setVolume(0);

    m_activeAmbiencePlayer = nullptr;
    m_fadingAmbiencePlayer = nullptr;

    m_ambienceFadeInPosition = 0;
    m_ambienceCrossfadePosition = 0;

    m_ambienceFadeOutActiveVolume = 0;
    m_ambienceFadeOutFadingVolume = 0;
}

void AudioEngine::updateAmbienceFadeOut()
{
    if (!m_activeAmbiencePlayer)
    {
        m_ambienceFadeOutTimer->stop();
        return;
    }

    // Already gone (error/end of file) -> nothing left to fade.
    if (m_activeAmbiencePlayer->state() !=
        QMediaPlayer::PlayingState)
    {
        stopAmbienceImmediately();
        return;
    }

    const double progress =
        AmbienceFadeOutDuration > 0
            ? qMin(
                  1.0,
                  static_cast<double>(
                      m_ambienceFadeOutElapsed.elapsed())
                      / static_cast<double>(
                          AmbienceFadeOutDuration))
            : 1.0;

    m_activeAmbiencePlayer->setVolume(
        qRound(
            m_ambienceFadeOutActiveVolume *
            (1.0 - progress)));

    if (m_fadingAmbiencePlayer)
    {
        m_fadingAmbiencePlayer->setVolume(
            qRound(
                m_ambienceFadeOutFadingVolume *
                (1.0 - progress)));
    }

    if (progress >= 1.0)
        stopAmbienceImmediately();
}

void AudioEngine::setColoredNoise(
    const QString &noise)
{
    QUrl mediaUrl;

    if (noise == "White")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/white.wav");
    else if (noise == "Pink")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/pink.wav");
    else if (noise == "Brown")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/brown.wav");
    else if (noise == "Grey")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/grey.wav");
    else
        return;

    // Switching noise color: immediate stop, the new color has its own
    // fade-in. A graceful fade-out happens in stopColoredNoise().
    stopColoredNoiseImmediately();

    m_activeColoredNoisePlayer =
        m_coloredNoisePlayerA;

    m_fadingColoredNoisePlayer = nullptr;

    m_activeColoredNoisePlayer->setMedia(
        mediaUrl);

    m_activeColoredNoisePlayer->setVolume(0);

    m_coloredNoiseFadeInPosition = 0;
    m_coloredNoiseCrossfadePosition = 0;

    m_activeColoredNoisePlayer->play();

    m_coloredNoiseFadeInTimer->start();

    m_coloredNoiseWatchdogTimer->start();
}

void AudioEngine::stopColoredNoise()
{
    if (!m_activeColoredNoisePlayer)
    {
        stopColoredNoiseImmediately();
        return;
    }

    if (m_coloredNoiseFadeOutTimer->isActive())
        return; // already fading out

    m_coloredNoiseFadeInTimer->stop();
    m_coloredNoiseCrossfadeTimer->stop();

    // If a crossfade was running, both players are faded out together.
    m_coloredNoiseFadeOutActiveVolume =
        m_activeColoredNoisePlayer->volume();

    m_coloredNoiseFadeOutFadingVolume =
        m_fadingColoredNoisePlayer
            ? m_fadingColoredNoisePlayer->volume()
            : 0;

    m_coloredNoiseFadeOutElapsed.start();

    m_coloredNoiseFadeOutTimer->start();
}

void AudioEngine::stopColoredNoiseImmediately()
{
    m_coloredNoiseFadeInTimer->stop();
    m_coloredNoiseCrossfadeTimer->stop();
    m_coloredNoiseWatchdogTimer->stop();
    m_coloredNoiseFadeOutTimer->stop();
    m_coloredNoiseVolumeTimer->stop();

    m_coloredNoisePlayerA->stop();
    m_coloredNoisePlayerB->stop();

    m_coloredNoisePlayerA->setVolume(0);
    m_coloredNoisePlayerB->setVolume(0);

    m_activeColoredNoisePlayer = nullptr;
    m_fadingColoredNoisePlayer = nullptr;

    m_coloredNoiseFadeInPosition = 0;
    m_coloredNoiseCrossfadePosition = 0;

    m_coloredNoiseFadeOutActiveVolume = 0;
    m_coloredNoiseFadeOutFadingVolume = 0;
}

void AudioEngine::updateColoredNoiseFadeOut()
{
    if (!m_activeColoredNoisePlayer)
    {
        m_coloredNoiseFadeOutTimer->stop();
        return;
    }

    // Already gone (error/end of file) -> nothing left to fade.
    if (m_activeColoredNoisePlayer->state() !=
        QMediaPlayer::PlayingState)
    {
        stopColoredNoiseImmediately();
        return;
    }

    const double progress =
        ColoredNoiseFadeOutDuration > 0
            ? qMin(
                  1.0,
                  static_cast<double>(
                      m_coloredNoiseFadeOutElapsed.elapsed())
                      / static_cast<double>(
                          ColoredNoiseFadeOutDuration))
            : 1.0;

    m_activeColoredNoisePlayer->setVolume(
        qRound(
            m_coloredNoiseFadeOutActiveVolume *
            (1.0 - progress)));

    if (m_fadingColoredNoisePlayer)
    {
        m_fadingColoredNoisePlayer->setVolume(
            qRound(
                m_coloredNoiseFadeOutFadingVolume *
                (1.0 - progress)));
    }

    if (progress >= 1.0)
        stopColoredNoiseImmediately();
}

void AudioEngine::updateBinauralFadeIn()
{
    if (!m_audioOutput)
    {
        m_binauralFadeTimer->stop();
        return;
    }

    m_binauralFadeInPosition +=
        FadeInterval;

    const double progress =
        BinauralFadeInDuration > 0
            ? qMin(
                  1.0,
                  static_cast<double>(
                      m_binauralFadeInPosition)
                      / static_cast<double>(
                          BinauralFadeInDuration))
            : 1.0;

    m_audioOutput->setVolume(
        (m_binauralVolume / 100.0) *
        progress);

    if (progress >= 1.0)
    {
        m_audioOutput->setVolume(
            m_binauralVolume / 100.0);

        m_binauralFadeTimer->stop();
        m_binauralFadeInPosition = 0;
    }
}

void AudioEngine::updateAmbienceFadeIn()
{
    if (!m_activeAmbiencePlayer)
    {
        m_ambienceFadeInTimer->stop();
        return;
    }

    m_ambienceFadeInPosition +=
        FadeInterval;

    const double progress =
        AmbienceFadeInDuration > 0
            ? qMin(
                  1.0,
                  static_cast<double>(
                      m_ambienceFadeInPosition)
                      / static_cast<double>(
                          AmbienceFadeInDuration))
            : 1.0;

    const int maximumVolume =
        ambienceTargetVolume();

    m_activeAmbiencePlayer->setVolume(
        qRound(
            maximumVolume *
            progress));

    if (progress >= 1.0)
    {
        m_activeAmbiencePlayer->setVolume(
            maximumVolume);

        m_ambienceFadeInTimer->stop();
        m_ambienceFadeInPosition = 0;
    }
}

void AudioEngine::checkAmbienceCrossfade(
    QMediaPlayer *player)
{
    if (!player)
        return;

    if (player != m_activeAmbiencePlayer)
        return;

    if (m_ambienceFadeOutTimer->isActive())
        return;

    if (m_fadingAmbiencePlayer)
        return;

    const qint64 duration =
        player->duration();

    if (duration <= 0)
        return;

    const qint64 remaining =
        duration - player->position();

    if (remaining <=
        m_ambienceCrossfadeDuration)
    {
        startAmbienceCrossfade(player);
    }
}

void AudioEngine::startAmbienceCrossfade(
    QMediaPlayer *fadingPlayer)
{
    if (!fadingPlayer)
        return;

    if (fadingPlayer !=
        m_activeAmbiencePlayer)
        return;

    if (m_fadingAmbiencePlayer)
        return;

    QMediaPlayer *nextPlayer =
        fadingPlayer == m_ambiencePlayerA
            ? m_ambiencePlayerB
            : m_ambiencePlayerA;

    nextPlayer->stop();

    nextPlayer->setMedia(
        fadingPlayer->currentMedia());

    nextPlayer->setPosition(0);
    nextPlayer->setVolume(0);

    m_fadingAmbiencePlayer =
        fadingPlayer;

    m_activeAmbiencePlayer =
        nextPlayer;

    m_ambienceCrossfadePosition = 0;

    nextPlayer->play();

    m_ambienceFadeInTimer->stop();

    // Elapsed-time based progress: robust against skipped timer ticks
    // (display off, CPU throttling) on older devices.
    m_ambienceCrossfadeElapsed.start();

    m_ambienceCrossfadeTimer->start();
}

void AudioEngine::updateAmbienceCrossfade()
{
    if (!m_activeAmbiencePlayer ||
        !m_fadingAmbiencePlayer)
    {
        m_ambienceCrossfadeTimer->stop();
        m_ambienceCrossfadePosition = 0;
        return;
    }

    // The fading player already reached its end (or was interrupted):
    // complete the handover immediately instead of continuing to fade
    // against a dead player.
    if (m_fadingAmbiencePlayer->state() !=
        QMediaPlayer::PlayingState)
    {
        finishAmbienceCrossfade();
        return;
    }

    m_ambienceCrossfadePosition =
        static_cast<int>(
            m_ambienceCrossfadeElapsed.elapsed());

    const double progress =
        m_ambienceCrossfadeDuration > 0
            ? qMin(
                  1.0,
                  static_cast<double>(
                      m_ambienceCrossfadePosition)
                      / static_cast<double>(
                          m_ambienceCrossfadeDuration))
            : 1.0;

    const int maximumVolume =
        ambienceTargetVolume();

    const int oldVolume =
        qRound(
            maximumVolume *
            (1.0 - progress));

    const int newVolume =
        qRound(
            maximumVolume *
            progress);

    m_fadingAmbiencePlayer->setVolume(
        oldVolume);

    m_activeAmbiencePlayer->setVolume(
        newVolume);

    if (progress >= 1.0)
        finishAmbienceCrossfade();
}

void AudioEngine::finishAmbienceCrossfade()
{
    m_ambienceCrossfadeTimer->stop();
    m_ambienceCrossfadePosition = 0;

    if (m_fadingAmbiencePlayer)
    {
        m_fadingAmbiencePlayer->stop();
        m_fadingAmbiencePlayer->setVolume(0);
        m_fadingAmbiencePlayer = nullptr;
    }

    if (m_activeAmbiencePlayer)
    {
        m_activeAmbiencePlayer->setVolume(
            ambienceTargetVolume());
    }
}

void AudioEngine::handleAmbienceEndOfMedia(
    QMediaPlayer *player)
{
    // The fading player reached its end before the crossfade finished:
    // complete the handover now.
    if (player == m_fadingAmbiencePlayer)
    {
        if (m_ambienceFadeOutTimer->isActive())
        {
            // We are shutting down anyway: just retire this player.
            player->setVolume(0);
            m_fadingAmbiencePlayer = nullptr;
            return;
        }

        finishAmbienceCrossfade();
        return;
    }

    // The active player reached its end without the crossfade window
    // having been detected (e.g. duration() == 0 or sparse
    // positionChanged on Qt 5.6): hard restart instead of silence.
    if (player == m_activeAmbiencePlayer)
    {
        if (m_ambienceFadeOutTimer->isActive())
        {
            stopAmbienceImmediately();
            return;
        }

        player->setPosition(0);
        player->play();
    }
}

void AudioEngine::updateColoredNoiseFadeIn()
{
    if (!m_activeColoredNoisePlayer)
    {
        m_coloredNoiseFadeInTimer->stop();
        return;
    }

    m_coloredNoiseFadeInPosition +=
        FadeInterval;

    const double progress =
        ColoredNoiseFadeInDuration > 0
            ? qMin(
                  1.0,
                  static_cast<double>(
                      m_coloredNoiseFadeInPosition)
                      / static_cast<double>(
                          ColoredNoiseFadeInDuration))
            : 1.0;

    const double easedProgress =
        1.0 - qPow(
            1.0 - progress,
            2.0);

    const int maximumVolume =
        coloredNoiseTargetVolume();

    m_activeColoredNoisePlayer->setVolume(
        qRound(
            maximumVolume *
            easedProgress));

    if (progress >= 1.0)
    {
        m_activeColoredNoisePlayer->setVolume(
            maximumVolume);

        m_coloredNoiseFadeInTimer->stop();
        m_coloredNoiseFadeInPosition = 0;
    }
}

void AudioEngine::checkColoredNoiseCrossfade(
    QMediaPlayer *player)
{
    if (!player)
        return;

    if (player != m_activeColoredNoisePlayer)
        return;

    if (m_coloredNoiseFadeOutTimer->isActive())
        return;

    if (m_fadingColoredNoisePlayer)
        return;

    const qint64 duration =
        player->duration();

    if (duration <= 0)
        return;

    const qint64 remaining =
        duration - player->position();

    if (remaining <=
        m_coloredNoiseCrossfadeDuration)
    {
        startColoredNoiseCrossfade(player);
    }
}

void AudioEngine::startColoredNoiseCrossfade(
    QMediaPlayer *fadingPlayer)
{
    if (!fadingPlayer)
        return;

    if (fadingPlayer !=
        m_activeColoredNoisePlayer)
        return;

    if (m_fadingColoredNoisePlayer)
        return;

    QMediaPlayer *nextPlayer =
        fadingPlayer == m_coloredNoisePlayerA
            ? m_coloredNoisePlayerB
            : m_coloredNoisePlayerA;

    nextPlayer->stop();

    nextPlayer->setMedia(
        fadingPlayer->currentMedia());

    nextPlayer->setPosition(0);
    nextPlayer->setVolume(0);

    m_fadingColoredNoisePlayer =
        fadingPlayer;

    m_activeColoredNoisePlayer =
        nextPlayer;

    m_coloredNoiseCrossfadePosition = 0;

    nextPlayer->play();

    m_coloredNoiseFadeInTimer->stop();

    // Elapsed-time based progress: robust against skipped timer ticks
    // (display off, CPU throttling) on older devices.
    m_coloredNoiseCrossfadeElapsed.start();

    m_coloredNoiseCrossfadeTimer->start();
}

void AudioEngine::updateColoredNoiseCrossfade()
{
    if (!m_activeColoredNoisePlayer ||
        !m_fadingColoredNoisePlayer)
    {
        m_coloredNoiseCrossfadeTimer->stop();
        m_coloredNoiseCrossfadePosition = 0;
        return;
    }

    // The fading player already reached its end (or was interrupted):
    // complete the handover immediately instead of continuing to fade
    // against a dead player.
    if (m_fadingColoredNoisePlayer->state() !=
        QMediaPlayer::PlayingState)
    {
        finishColoredNoiseCrossfade();
        return;
    }

    m_coloredNoiseCrossfadePosition =
        static_cast<int>(
            m_coloredNoiseCrossfadeElapsed.elapsed());

    const double progress =
        m_coloredNoiseCrossfadeDuration > 0
            ? qMin(
                  1.0,
                  static_cast<double>(
                      m_coloredNoiseCrossfadePosition)
                      / static_cast<double>(
                          m_coloredNoiseCrossfadeDuration))
            : 1.0;

    const int maximumVolume =
        coloredNoiseTargetVolume();

    const double angle =
        progress * M_PI_2;

    const int oldVolume =
        qRound(
            maximumVolume *
            qCos(angle));

    const int newVolume =
        qRound(
            maximumVolume *
            qSin(angle));

    m_fadingColoredNoisePlayer->setVolume(
        oldVolume);

    m_activeColoredNoisePlayer->setVolume(
        newVolume);

    if (progress >= 1.0)
        finishColoredNoiseCrossfade();
}

void AudioEngine::finishColoredNoiseCrossfade()
{
    m_coloredNoiseCrossfadeTimer->stop();
    m_coloredNoiseCrossfadePosition = 0;

    if (m_fadingColoredNoisePlayer)
    {
        m_fadingColoredNoisePlayer->stop();
        m_fadingColoredNoisePlayer->setVolume(0);
        m_fadingColoredNoisePlayer = nullptr;
    }

    if (m_activeColoredNoisePlayer)
    {
        m_activeColoredNoisePlayer->setVolume(
            coloredNoiseTargetVolume());
    }
}

void AudioEngine::handleColoredNoiseEndOfMedia(
    QMediaPlayer *player)
{
    // The fading player reached its end before the crossfade finished:
    // complete the handover now.
    if (player == m_fadingColoredNoisePlayer)
    {
        if (m_coloredNoiseFadeOutTimer->isActive())
        {
            // We are shutting down anyway: just retire this player.
            player->setVolume(0);
            m_fadingColoredNoisePlayer = nullptr;
            return;
        }

        finishColoredNoiseCrossfade();
        return;
    }

    // The active player reached its end without the crossfade window
    // having been detected (e.g. duration() == 0 or sparse
    // positionChanged on Qt 5.6): hard restart instead of silence.
    if (player == m_activeColoredNoisePlayer)
    {
        if (m_coloredNoiseFadeOutTimer->isActive())
        {
            stopColoredNoiseImmediately();
            return;
        }

        player->setPosition(0);
        player->play();
    }
}

void AudioEngine::updateColoredNoiseWatchdog()
{
    if (!m_activeColoredNoisePlayer)
    {
        m_coloredNoiseWatchdogTimer->stop();
        return;
    }

    // No restarts while we are shutting the noise down.
    if (m_coloredNoiseFadeOutTimer->isActive())
        return;

    // Only restart a player that has actually fallen back to
    // StoppedState. A paused player is left alone: on SFOS the audio
    // policy pauses streams during calls/ringtones, and fighting that
    // would be wrong.
    if (m_activeColoredNoisePlayer->state() ==
        QMediaPlayer::StoppedState)
    {
        m_activeColoredNoisePlayer->setPosition(0);
        m_activeColoredNoisePlayer->play();
    }
}

void AudioEngine::start()
{
    if (m_audioOutput)
    {
        // Toggled back on while a fade-out was still running:
        // cancel it and restore the volume.
        if (m_binauralFadeOutTimer->isActive())
        {
            m_binauralFadeOutTimer->stop();
            m_audioOutput->setVolume(
                m_binauralVolume / 100.0);
        }

        return;
    }

    QAudioFormat format;

    format.setSampleRate(DefaultSampleRate);
    format.setChannelCount(ChannelCount);
    format.setSampleSize(SampleSize);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    const QAudioDeviceInfo device =
        QAudioDeviceInfo::defaultOutputDevice();

    if (device.isNull())
        return;

    if (!device.isFormatSupported(format))
        format = device.nearestFormat(format);

    m_generator =
        new AudioGenerator(this);

    m_generator->setBeatFrequency(
        m_beatFrequency);

    // Follow the format the device actually gave us, otherwise the
    // carrier and beat frequency detune on devices that hand back
    // e.g. 44100 Hz.
    m_generator->setSampleRate(
        format.sampleRate());

    m_generator->start();

    m_audioOutput =
        new QAudioOutput(
            device,
            format,
            this);

    m_audioOutput->setVolume(
        m_binauralVolume / 100.0);

    m_audioOutput->start(
        m_generator);
}

void AudioEngine::stop()
{
    if (!m_audioOutput)
        return;

    if (m_binauralFadeOutTimer->isActive())
        return; // already fading out

    m_binauralFadeTimer->stop();

    m_binauralFadeOutStartVolume =
        m_audioOutput->volume();

    m_binauralFadeOutElapsed.start();

    m_binauralFadeOutTimer->start();
}

void AudioEngine::updateBinauralFadeOut()
{
    if (!m_audioOutput)
    {
        m_binauralFadeOutTimer->stop();
        return;
    }

    const double progress =
        BinauralFadeOutDuration > 0
            ? qMin(
                  1.0,
                  static_cast<double>(
                      m_binauralFadeOutElapsed.elapsed())
                      / static_cast<double>(
                          BinauralFadeOutDuration))
            : 1.0;

    m_audioOutput->setVolume(
        m_binauralFadeOutStartVolume *
        (1.0 - progress));

    if (progress >= 1.0)
        stopBinauralImmediately();
}

void AudioEngine::stopBinauralImmediately()
{
    m_binauralFadeTimer->stop();
    m_binauralFadeOutTimer->stop();

    if (!m_audioOutput)
        return;

    m_audioOutput->stop();

    delete m_audioOutput;
    m_audioOutput = nullptr;

    if (m_generator)
    {
        m_generator->stop();

        delete m_generator;
        m_generator = nullptr;
    }

    m_binauralFadeInPosition = 0;
    m_binauralFadeOutStartVolume = 0.0;
}
