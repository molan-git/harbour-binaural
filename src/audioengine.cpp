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

    const int BinauralFadeInDuration = 0;

    const int AmbienceFadeInDuration = 1000;
    const int AmbienceCrossfadeDuration = 8000;

    const int ColoredNoiseFadeInDuration = 0;
    const int ColoredNoiseCrossfadeDuration = 4000;

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
      m_binauralFadeTimer(new QTimer(this)),
      m_ambienceFadeInTimer(new QTimer(this)),
      m_ambienceCrossfadeTimer(new QTimer(this)),
      m_coloredNoiseFadeInTimer(new QTimer(this)),
      m_coloredNoiseCrossfadeTimer(new QTimer(this)),
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
          ColoredNoiseCrossfadeDuration)
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

    connect(
        m_ambiencePlayerA,
        &QMediaPlayer::positionChanged,
        this,
        [this](qint64)
        {
            checkAmbienceCrossfade(
                m_ambiencePlayerA);
        });

    connect(
        m_ambiencePlayerB,
        &QMediaPlayer::positionChanged,
        this,
        [this](qint64)
        {
            checkAmbienceCrossfade(
                m_ambiencePlayerB);
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
        m_coloredNoiseCrossfadeTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            updateColoredNoiseCrossfade();
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
        m_generator->setBeatFrequency(
            m_beatFrequency);
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
        !m_ambienceFadeInTimer->isActive() &&
        !m_ambienceCrossfadeTimer->isActive())
    {
        m_activeAmbiencePlayer->setVolume(
            qRound(m_ambienceVolume * 0.6));
    }
}

void AudioEngine::setColoredNoiseVolume(int volume)
{
    volume = qBound(0, volume, 100);

    m_coloredNoiseVolume = volume;

    const int actualVolume =
        qRound(m_coloredNoiseVolume * 0.3);

    if (m_activeColoredNoisePlayer &&
        !m_coloredNoiseFadeInTimer->isActive() &&
        !m_coloredNoiseCrossfadeTimer->isActive())
    {
        m_activeColoredNoisePlayer->setVolume(
            actualVolume);
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

    stopAmbience();

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
    m_ambienceFadeInTimer->stop();
    m_ambienceCrossfadeTimer->stop();

    m_ambiencePlayerA->stop();
    m_ambiencePlayerB->stop();

    m_ambiencePlayerA->setVolume(0);
    m_ambiencePlayerB->setVolume(0);

    m_activeAmbiencePlayer = nullptr;
    m_fadingAmbiencePlayer = nullptr;

    m_ambienceFadeInPosition = 0;
    m_ambienceCrossfadePosition = 0;
}

void AudioEngine::setColoredNoise(
    const QString &noise)
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

    m_coloredNoiseFadeInPosition = 0;
    m_coloredNoiseCrossfadePosition = 0;

    m_activeColoredNoisePlayer->play();

    m_coloredNoiseFadeInTimer->start();
}

void AudioEngine::stopColoredNoise()
{
    m_coloredNoiseFadeInTimer->stop();
    m_coloredNoiseCrossfadeTimer->stop();

    m_coloredNoisePlayerA->stop();
    m_coloredNoisePlayerB->stop();

    m_coloredNoisePlayerA->setVolume(0);
    m_coloredNoisePlayerB->setVolume(0);

    m_activeColoredNoisePlayer = nullptr;
    m_fadingColoredNoisePlayer = nullptr;

    m_coloredNoiseFadeInPosition = 0;
    m_coloredNoiseCrossfadePosition = 0;
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
        qMin(
            1.0,
            static_cast<double>(
                m_binauralFadeInPosition)
            / static_cast<double>(
                BinauralFadeInDuration));

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
        qMin(
            1.0,
            static_cast<double>(
                m_ambienceFadeInPosition)
            / static_cast<double>(
                AmbienceFadeInDuration));

    const int maximumVolume =
        qRound(m_ambienceVolume * 0.6);

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
    m_ambienceCrossfadeTimer->start();
}

void AudioEngine::updateAmbienceCrossfade()
{
    if (!m_activeAmbiencePlayer ||
        !m_fadingAmbiencePlayer)
    {
        m_ambienceCrossfadeTimer->stop();
        return;
    }

    m_ambienceCrossfadePosition +=
        FadeInterval;

    const double progress =
        qMin(
            1.0,
            static_cast<double>(
                m_ambienceCrossfadePosition)
            / static_cast<double>(
                m_ambienceCrossfadeDuration));

    const int maximumVolume =
        qRound(m_ambienceVolume * 0.6);

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
    {
        m_activeAmbiencePlayer->setVolume(
            maximumVolume);

        m_fadingAmbiencePlayer->stop();
        m_fadingAmbiencePlayer->setVolume(0);

        m_fadingAmbiencePlayer = nullptr;

        m_ambienceCrossfadeTimer->stop();
        m_ambienceCrossfadePosition = 0;
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
        qMin(
            1.0,
            static_cast<double>(
                m_coloredNoiseFadeInPosition)
            / static_cast<double>(
                ColoredNoiseFadeInDuration));

    const double easedProgress =
        1.0 - qPow(
            1.0 - progress,
            2.0);

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
    m_coloredNoiseCrossfadeTimer->start();
}

void AudioEngine::updateColoredNoiseCrossfade()
{
    if (!m_activeColoredNoisePlayer ||
        !m_fadingColoredNoisePlayer)
    {
        m_coloredNoiseCrossfadeTimer->stop();
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

        m_coloredNoiseCrossfadeTimer->stop();
        m_coloredNoiseCrossfadePosition = 0;
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

    m_binauralFadeInPosition = 0;
}
