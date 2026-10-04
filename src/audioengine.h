#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include <QObject>
#include <QAudioOutput>
#include <QMediaPlayer>
#include <QTimer>

class AudioGenerator;

class AudioEngine : public QObject
{
    Q_OBJECT

public:
    explicit AudioEngine(QObject *parent = nullptr);
    ~AudioEngine();

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    Q_INVOKABLE void setFrequencyBand(const QString &band);

    Q_INVOKABLE void setAmbience(const QString &ambience);
    Q_INVOKABLE void stopAmbience();

    Q_INVOKABLE void setColoredNoise(const QString &noise);
    Q_INVOKABLE void stopColoredNoise();

    Q_INVOKABLE void setBinauralVolume(int volume);
    Q_INVOKABLE void setAmbienceVolume(int volume);
    Q_INVOKABLE void setColoredNoiseVolume(int volume);

private:
    void updateBinauralFadeIn();

    void updateAmbienceFadeIn();
    void checkAmbienceCrossfade(QMediaPlayer *player);
    void startAmbienceCrossfade(QMediaPlayer *fadingPlayer);
    void updateAmbienceCrossfade();

    void updateColoredNoiseFadeIn();
    void checkColoredNoiseCrossfade(QMediaPlayer *player);
    void startColoredNoiseCrossfade(QMediaPlayer *fadingPlayer);
    void updateColoredNoiseCrossfade();

    QAudioOutput *m_audioOutput;
    AudioGenerator *m_generator;

    QMediaPlayer *m_ambiencePlayerA;
    QMediaPlayer *m_ambiencePlayerB;

    QMediaPlayer *m_coloredNoisePlayerA;
    QMediaPlayer *m_coloredNoisePlayerB;

    QTimer *m_binauralFadeTimer;

    QTimer *m_ambienceFadeInTimer;
    QTimer *m_ambienceCrossfadeTimer;

    QTimer *m_coloredNoiseFadeInTimer;
    QTimer *m_coloredNoiseCrossfadeTimer;

    QMediaPlayer *m_activeAmbiencePlayer;
    QMediaPlayer *m_fadingAmbiencePlayer;

    QMediaPlayer *m_activeColoredNoisePlayer;
    QMediaPlayer *m_fadingColoredNoisePlayer;

    double m_beatFrequency;

    int m_binauralVolume;
    int m_ambienceVolume;
    int m_coloredNoiseVolume;

    int m_binauralFadeInPosition;

    int m_ambienceFadeInPosition;
    int m_ambienceCrossfadePosition;
    int m_ambienceCrossfadeDuration;

    int m_coloredNoiseFadeInPosition;
    int m_coloredNoiseCrossfadePosition;
    int m_coloredNoiseCrossfadeDuration;
};

#endif
