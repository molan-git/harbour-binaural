#include "audioengine.h"
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

    // The carrier is the audible base frequency.
    // The beat frequency determines the difference between
    // the left and right channels.
    const double CarrierFrequency = 200.0;

    const double Amplitude = 16000.0;

    // The ambience crossfade lasts eight seconds.
    const int CrossfadeDuration = 8000;

    // The initial fade-in lasts two seconds.
    const int FadeInDuration = 2000;

    // Update fades every 50 milliseconds.
    const int FadeInterval = 50;
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

            const qint16 leftSample =
                static_cast<qint16>(leftValue * Amplitude);

            const qint16 rightSample =
                static_cast<qint16>(rightValue * Amplitude);

            samples[frame * 2] = leftSample;
            samples[frame * 2 + 1] = rightSample;

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
      m_crossfadeTimer(new QTimer(this)),
      m_binauralFadeTimer(new QTimer(this)),
      m_ambienceFadeTimer(new QTimer(this)),
      m_activeAmbiencePlayer(nullptr),
      m_fadingAmbiencePlayer(nullptr),
      m_beatFrequency(10.0),
      m_binauralVolume(50),
      m_ambienceVolume(50),
      m_crossfadePosition(0),
      m_crossfadeDuration(CrossfadeDuration),
      m_binauralFadePosition(0),
      m_ambienceFadePosition(0)
{
    m_crossfadeTimer->setInterval(FadeInterval);
    m_binauralFadeTimer->setInterval(FadeInterval);
    m_ambienceFadeTimer->setInterval(FadeInterval);

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

    m_ambiencePlayerA->setVolume(0);
    m_ambiencePlayerB->setVolume(0);

    qDebug() << "AudioEngine created";
}

AudioEngine::~AudioEngine()
{
    stop();
    stopAmbience();
}

void AudioEngine::setFrequencyBand(const QString &band)
{
    if (band == "Delta")
    {
        m_beatFrequency = 2.0;
    }
    else if (band == "Theta")
    {
        m_beatFrequency = 6.0;
    }
    else if (band == "Alpha")
    {
        m_beatFrequency = 10.0;
    }
    else if (band == "Beta")
    {
        m_beatFrequency = 20.0;
    }
    else if (band == "Gamma")
    {
        m_beatFrequency = 40.0;
    }
    else
    {
        return;
    }

    qDebug()
        << "Frequency band:"
        << band
        << "Beat:"
        << m_beatFrequency
        << "Hz";

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

    qDebug()
        << "Binaural volume:"
        << m_binauralVolume;
}

void AudioEngine::setAmbienceVolume(int volume)
{
    volume = qBound(0, volume, 100);

    m_ambienceVolume = volume;

    if (m_activeAmbiencePlayer &&
        !m_ambienceFadeTimer->isActive())
    {
        m_activeAmbiencePlayer->setVolume(
            m_ambienceVolume);
    }

    qDebug()
        << "Ambience volume:"
        << m_ambienceVolume;
}

void AudioEngine::setAmbience(const QString &ambience)
{
    QUrl mediaUrl;

    if (ambience == "Wind")
    {
        mediaUrl =
            QUrl("file:///usr/share/harbour-binaural/sounds/wind.mp3");
    }
    else if (ambience == "Sea Waves")
    {
        mediaUrl =
            QUrl("file:///usr/share/harbour-binaural/sounds/seawaves.mp3");
    }
    else if (ambience == "Crickets")
    {
        mediaUrl =
            QUrl("file:///usr/share/harbour-binaural/sounds/crickets.mp3");
    }
    else if (ambience == "Stream")
    {
        mediaUrl =
            QUrl("file:///usr/share/harbour-binaural/sounds/stream.mp3");
    }
    else if (ambience == "Rain")
    {
        mediaUrl =
            QUrl("file:///usr/share/harbour-binaural/sounds/rain.mp3");
    }
    else
    {
        qWarning()
            << "Unknown ambience:"
            << ambience;

        return;
    }

    stopAmbience();

    // Player A is used as the first active player.
    // Player B will be used for the next crossfade.
    m_activeAmbiencePlayer =
        m_ambiencePlayerA;

    m_activeAmbiencePlayer->setMedia(mediaUrl);

    // Start the ambience silently.
    // The fade timer will raise the volume to the
    // user's selected volume over two seconds.
    m_activeAmbiencePlayer->setVolume(0);

    m_ambienceFadePosition = 0;
    m_activeAmbiencePlayer->play();

    m_ambienceFadeTimer->start();

    qDebug()
        << "Ambience started:"
        << ambience;
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

void AudioEngine::checkForCrossfade(QMediaPlayer *player)
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

    // Start the next copy five seconds before
    // the current track reaches its end.
    if (remaining <= m_crossfadeDuration)
    {
        startCrossfade(player);
    }
}

void AudioEngine::startCrossfade(
    QMediaPlayer *fadingPlayer)
{
    QMediaPlayer *nextPlayer =
        fadingPlayer == m_ambiencePlayerA
            ? m_ambiencePlayerB
            : m_ambiencePlayerA;

    nextPlayer->stop();

    // Use the same media file again.
    nextPlayer->setMedia(
        fadingPlayer->currentMedia());

    nextPlayer->setPosition(0);

    // Start the new copy silently.
    nextPlayer->setVolume(0);
    nextPlayer->play();

    m_fadingAmbiencePlayer = fadingPlayer;
    m_activeAmbiencePlayer = nextPlayer;

    m_crossfadePosition = 0;

    m_ambienceFadeTimer->stop();
    m_crossfadeTimer->start();

    qDebug()
        << "Ambience crossfade started";
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
            static_cast<double>(m_crossfadePosition) /
            static_cast<double>(m_crossfadeDuration));

    // Fade the old player out.
    const int oldVolume =
        static_cast<int>(
            m_ambienceVolume * (1.0 - progress));

    // Fade the new player in.
    const int newVolume =
        static_cast<int>(
            m_ambienceVolume * progress);

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

        qDebug()
            << "Ambience crossfade completed";
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
            static_cast<double>(m_binauralFadePosition) /
            static_cast<double>(FadeInDuration));

    const double volume =
        (m_binauralVolume / 100.0) *
        progress;

    m_audioOutput->setVolume(volume);

    if (progress >= 1.0)
    {
        m_audioOutput->setVolume(
            m_binauralVolume / 100.0);

        m_binauralFadeTimer->stop();

        qDebug()
            << "Binaural fade-in completed";
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
            static_cast<double>(m_ambienceFadePosition) /
            static_cast<double>(FadeInDuration));

    const int volume =
        static_cast<int>(
            m_ambienceVolume * progress);

    m_activeAmbiencePlayer->setVolume(
        volume);

    if (progress >= 1.0)
    {
        m_activeAmbiencePlayer->setVolume(
            m_ambienceVolume);

        m_ambienceFadeTimer->stop();

        qDebug()
            << "Ambience fade-in completed";
    }
}

void AudioEngine::start()
{
    qDebug() << "AudioEngine start";

    if (m_audioOutput)
    {
        qDebug() << "Audio is already running";
        return;
    }

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
    {
        qWarning()
            << "No default audio output device found";
        return;
    }

    qDebug()
        << "Audio device:"
        << device.deviceName();

    if (!device.isFormatSupported(format))
    {
        qWarning()
            << "48 kHz stereo 16-bit PCM is not supported";

        format = device.nearestFormat(format);

        qDebug()
            << "Using nearest supported format:"
            << format.sampleRate()
            << "Hz,"
            << format.channelCount()
            << "channels,"
            << format.sampleSize()
            << "bit";
    }

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

    // Start at the user's selected volume.
    // A fade-in is intentionally not used here because
    // changing the output volume while starting the PCM
    // stream can produce an audible click.
    m_audioOutput->setVolume(
        m_binauralVolume / 100.0);

    m_audioOutput->start(
        m_generator);

    qDebug()
        << "Binaural tone started:"
        << CarrierFrequency
        << "Hz carrier,"
        << m_beatFrequency
        << "Hz beat";
}

void AudioEngine::stop()
{
    qDebug() << "AudioEngine stop";

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

    qDebug() << "Audio stopped";
}
