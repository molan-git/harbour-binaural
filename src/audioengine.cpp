#include "audioengine.h"

#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QDebug>
#include <QUrl>
#include <QtMath>

namespace
{
    const int SampleRate = 48000;
    const int ChannelCount = 2;
    const int SampleSize = 16;

    const double CarrierFrequency = 200.0;
    const double Amplitude = 16000.0;

    // Crossfade for Color Noise has different lenght
    const int CrossfadeDuration = 8000;
    const int ColoredNoiseCrossfadeDuration = 4000;
    const int FadeInDuration = 1000;
    const int FadeInterval = 15;
}

class AudioGenerator : public QIODevice
{
public:
    explicit AudioGenerator(QObject *parent = nullptr)
        : QIODevice(parent),
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

protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        const int bytesPerFrame = 4;
        const qint64 frameCount = maxlen / bytesPerFrame;

        qint16 *samples =
            reinterpret_cast<qint16 *>(data);

        const double leftFrequency =
            CarrierFrequency;

        const double rightFrequency =
            CarrierFrequency + m_beatFrequency;

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
                leftFrequency / SampleRate;

            m_rightPhase +=
                rightFrequency / SampleRate;

            if (m_leftPhase >= 1.0)
                m_leftPhase -= 1.0;

            if (m_rightPhase >= 1.0)
                m_rightPhase -= 1.0;
        }

        return frameCount * bytesPerFrame;
    }

    qint64 writeData(const char *, qint64) override
    {
        return 0;
    }

private:
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
      m_crossfadeTimer(new QTimer(this)),
      m_binauralFadeTimer(new QTimer(this)),
      m_ambienceFadeTimer(new QTimer(this)),
      m_coloredNoiseFadeTimer(new QTimer(this)),
      m_coloredNoiseCrossfadeTimer(new QTimer(this)),
      m_activeAmbiencePlayer(nullptr),
      m_fadingAmbiencePlayer(nullptr),
      m_activeColoredNoisePlayer(nullptr),
      m_fadingColoredNoisePlayer(nullptr),
      m_beatFrequency(10.0),
      m_binauralVolume(50),
      m_ambienceVolume(50),
      m_coloredNoiseVolume(50),
      m_crossfadePosition(0),
      m_crossfadeDuration(CrossfadeDuration),
      m_binauralFadePosition(0),
      m_ambienceFadePosition(0),
      m_coloredNoiseFadePosition(0),
      m_coloredNoiseCrossfadePosition(0),
      m_coloredNoiseCrossfadeDuration(
          ColoredNoiseCrossfadeDuration)
{
    m_crossfadeTimer->setInterval(FadeInterval);
    m_binauralFadeTimer->setInterval(FadeInterval);
    m_ambienceFadeTimer->setInterval(FadeInterval);
    m_coloredNoiseFadeTimer->setInterval(FadeInterval);

    connect(
        m_ambiencePlayerA,
        &QMediaPlayer::positionChanged,
        this,
        [this](qint64)
        {
            checkForCrossfade(m_ambiencePlayerA);
        });

    connect(
        m_ambiencePlayerB,
        &QMediaPlayer::positionChanged,
        this,
        [this](qint64)
        {
            checkForCrossfade(m_ambiencePlayerB);
        });

    connect(
        m_crossfadeTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateCrossfade();
        });

    connect(
        m_binauralFadeTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateBinauralFadeIn();
        });

    connect(
        m_ambienceFadeTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateAmbienceFadeIn();
        });

    connect(
        m_coloredNoisePlayerA,
        &QMediaPlayer::positionChanged,
        this,
        [this](qint64)
        {
            checkColoredNoiseCrossfade(
                m_coloredNoisePlayerA);
        });

    connect(
        m_coloredNoisePlayerB,
        &QMediaPlayer::positionChanged,
        this,
        [this](qint64)
        {
            checkColoredNoiseCrossfade(
                m_coloredNoisePlayerB);
        });

    connect(
        m_coloredNoiseFadeTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            if (m_fadingColoredNoisePlayer)
                updateColoredNoiseCrossfade();
            else
                updateColoredNoiseFadeIn();
        });

    m_ambiencePlayerA->setVolume(0);
    m_ambiencePlayerB->setVolume(0);

    m_coloredNoisePlayerA->setVolume(0);
    m_coloredNoisePlayerB->setVolume(0);
}

AudioEngine::~AudioEngine()
{
    stop();
    stopAmbience();
    stopColoredNoise();
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
        m_generator->setBeatFrequency(m_beatFrequency);
}

void AudioEngine::setBinauralVolume(int volume)
{
    volume = qBound(0, volume, 100);

    m_binauralVolume = volume;

    if (m_audioOutput &&
        !m_binauralFadeTimer->isActive())
    {
        m_audioOutput->setVolume(
            m_binauralVolume / 100.0);
    }
}

void AudioEngine::setAmbienceVolume(int volume)
{
    volume = qBound(0, volume, 100);

    m_ambienceVolume = volume;

    if (m_activeAmbiencePlayer &&
        !m_ambienceFadeTimer->isActive() &&
        !m_crossfadeTimer->isActive())
    {
        m_activeAmbiencePlayer->setVolume(
            m_ambienceVolume);
    }
}

void AudioEngine::setColoredNoiseVolume(int volume)
{
    volume = qBound(0, volume, 100);

    m_coloredNoiseVolume = volume;

    const int actualVolume =
        qRound(m_coloredNoiseVolume * 0.3);

    if (m_activeColoredNoisePlayer &&
        !m_coloredNoiseFadeTimer->isActive())
    {
        m_activeColoredNoisePlayer->setVolume(
            actualVolume);
    }
}

void AudioEngine::setAmbience(const QString &ambience)
{
    QUrl mediaUrl;

    if (ambience == "Wind")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/wind.mp3");
    else if (ambience == "Sea Waves")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/seawaves.mp3");
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

    stopAmbience();

    m_activeAmbiencePlayer =
        m_ambiencePlayerA;

    m_activeAmbiencePlayer->setMedia(
        mediaUrl);

    m_activeAmbiencePlayer->setVolume(0);

    m_ambienceFadePosition = 0;

    m_activeAmbiencePlayer->play();

    m_ambienceFadeTimer->start();
}

void AudioEngine::stopAmbience()
{
    m_crossfadeTimer->stop();
    m_ambienceFadeTimer->stop();

    m_ambiencePlayerA->stop();
    m_ambiencePlayerB->stop();

    m_ambiencePlayerA->setVolume(0);
    m_ambiencePlayerB->setVolume(0);

    m_activeAmbiencePlayer = nullptr;
    m_fadingAmbiencePlayer = nullptr;

    m_crossfadePosition = 0;
    m_ambienceFadePosition = 0;
}

void AudioEngine::setColoredNoise(const QString &noise)
{
    QUrl mediaUrl;

    if (noise == "White")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/white.mp3");
    else if (noise == "Pink")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/pink.mp3");
    else if (noise == "Brown")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/brown.mp3");
    else if (noise == "Grey")
        mediaUrl = QUrl(
            "file:///usr/share/harbour-binaural/sounds/grey.mp3");
    else
        return;

    stopColoredNoise();

    m_activeColoredNoisePlayer =
        m_coloredNoisePlayerA;

    m_fadingColoredNoisePlayer = nullptr;

    m_activeColoredNoisePlayer->setMedia(
        mediaUrl);

    m_activeColoredNoisePlayer->setVolume(0);

    m_coloredNoiseFadePosition = 0;
    m_coloredNoiseCrossfadePosition = 0;

    m_activeColoredNoisePlayer->play();

    m_coloredNoiseFadeTimer->start();
}

void AudioEngine::stopColoredNoise()
{
    m_coloredNoiseFadeTimer->stop();
    m_coloredNoiseCrossfadeTimer->stop();

    m_coloredNoisePlayerA->stop();
    m_coloredNoisePlayerB->stop();

    m_coloredNoisePlayerA->setVolume(0);
    m_coloredNoisePlayerB->setVolume(0);

    m_activeColoredNoisePlayer = nullptr;
    m_fadingColoredNoisePlayer = nullptr;

    m_coloredNoiseFadePosition = 0;
    m_coloredNoiseCrossfadePosition = 0;
}

void AudioEngine::checkColoredNoiseCrossfade(
    QMediaPlayer *player)
{
    if (!player)
        return;

    if (player != m_activeColoredNoisePlayer)
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

    m_coloredNoiseFadeTimer->start();
}

void AudioEngine::updateColoredNoiseCrossfade()
{
    if (!m_activeColoredNoisePlayer ||
        !m_fadingColoredNoisePlayer)
    {
        m_coloredNoiseFadeTimer->stop();
        return;
    }

    m_coloredNoiseCrossfadePosition +=
        FadeInterval;

    const double progress =
        qMin(
            1.0,
            static_cast<double>(
                m_coloredNoiseCrossfadePosition)
            / static_cast<double>(
                m_coloredNoiseCrossfadeDuration));

    const int maximumVolume =
        qRound(m_coloredNoiseVolume * 0.3);

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
    {
        m_fadingColoredNoisePlayer->stop();
        m_fadingColoredNoisePlayer->setVolume(0);

        m_fadingColoredNoisePlayer = nullptr;

        m_activeColoredNoisePlayer->setVolume(
            maximumVolume);

        m_coloredNoiseFadeTimer->stop();

        m_coloredNoiseCrossfadePosition = 0;
    }
}

void AudioEngine::checkForCrossfade(
    QMediaPlayer *player)
{
    if (player != m_activeAmbiencePlayer)
        return;

    if (m_fadingAmbiencePlayer)
        return;

    const qint64 duration =
        player->duration();

    if (duration <= 0)
        return;

    const qint64 remaining =
        duration - player->position();

    if (remaining <= m_crossfadeDuration)
        startCrossfade(player);
}

void AudioEngine::startCrossfade(
    QMediaPlayer *fadingPlayer)
{
    QMediaPlayer *nextPlayer =
        fadingPlayer == m_ambiencePlayerA
            ? m_ambiencePlayerB
            : m_ambiencePlayerA;

    nextPlayer->stop();

    nextPlayer->setMedia(
        fadingPlayer->currentMedia());

    nextPlayer->setPosition(0);
    nextPlayer->setVolume(0);
    nextPlayer->play();

    m_fadingAmbiencePlayer =
        fadingPlayer;

    m_activeAmbiencePlayer =
        nextPlayer;

    m_crossfadePosition = 0;

    m_ambienceFadeTimer->stop();
    m_crossfadeTimer->start();
}

void AudioEngine::updateCrossfade()
{
    if (!m_activeAmbiencePlayer ||
        !m_fadingAmbiencePlayer)
    {
        m_crossfadeTimer->stop();
        return;
    }

    m_crossfadePosition +=
        FadeInterval;

    const double progress =
        qMin(
            1.0,
            static_cast<double>(
                m_crossfadePosition)
            / static_cast<double>(
                m_crossfadeDuration));

    const int oldVolume =
        static_cast<int>(
            m_ambienceVolume *
            (1.0 - progress));

    const int newVolume =
        static_cast<int>(
            m_ambienceVolume *
            progress);

    m_fadingAmbiencePlayer->setVolume(
        oldVolume);

    m_activeAmbiencePlayer->setVolume(
        newVolume);

    if (progress >= 1.0)
    {
        m_activeAmbiencePlayer->setVolume(
            m_ambienceVolume);

        m_fadingAmbiencePlayer->stop();
        m_fadingAmbiencePlayer->setVolume(0);

        m_fadingAmbiencePlayer = nullptr;

        m_crossfadeTimer->stop();
    }
}

void AudioEngine::updateBinauralFadeIn()
{
    if (!m_audioOutput)
    {
        m_binauralFadeTimer->stop();
        return;
    }

    m_binauralFadePosition +=
        FadeInterval;

    const double progress =
        qMin(
            1.0,
            static_cast<double>(
                m_binauralFadePosition)
            / static_cast<double>(
                FadeInDuration));

    m_audioOutput->setVolume(
        (m_binauralVolume / 100.0) *
        progress);

    if (progress >= 1.0)
    {
        m_audioOutput->setVolume(
            m_binauralVolume / 100.0);

        m_binauralFadeTimer->stop();
    }
}

void AudioEngine::updateAmbienceFadeIn()
{
    if (!m_activeAmbiencePlayer)
    {
        m_ambienceFadeTimer->stop();
        return;
    }

    m_ambienceFadePosition +=
        FadeInterval;

    const double progress =
        qMin(
            1.0,
            static_cast<double>(
                m_ambienceFadePosition)
            / static_cast<double>(
                FadeInDuration));

    m_activeAmbiencePlayer->setVolume(
        static_cast<int>(
            m_ambienceVolume *
            progress));

    if (progress >= 1.0)
    {
        m_activeAmbiencePlayer->setVolume(
            m_ambienceVolume);

        m_ambienceFadeTimer->stop();
    }
}

void AudioEngine::updateColoredNoiseFadeIn()
{
    if (!m_activeColoredNoisePlayer)
    {
        m_coloredNoiseFadeTimer->stop();
        return;
    }

    m_coloredNoiseFadePosition +=
        FadeInterval;

    const double progress =
        qMin(
            1.0,
            static_cast<double>(
                m_coloredNoiseFadePosition)
            / static_cast<double>(
                FadeInDuration));

    const double easedProgress =
        1.0 - qPow(1.0 - progress, 2.0);

    const int maximumVolume =
        qRound(m_coloredNoiseVolume * 0.3);

    m_activeColoredNoisePlayer->setVolume(
        qRound(
            maximumVolume *
            easedProgress));

    if (progress >= 1.0)
    {
        m_activeColoredNoisePlayer->setVolume(
            maximumVolume);

        m_coloredNoiseFadeTimer->stop();
    }
}

void AudioEngine::start()
{
    if (m_audioOutput)
        return;

    QAudioFormat format;

    format.setSampleRate(SampleRate);
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
    m_binauralFadeTimer->stop();

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

    m_binauralFadePosition = 0;
}
