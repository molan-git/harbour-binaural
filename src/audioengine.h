#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include <QObject>
#include <QAudioOutput>
#include <QElapsedTimer>
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
    void updateBinauralFadeOut();
    void stopBinauralImmediately();

    void updateAmbienceFadeIn();
    void checkAmbienceCrossfade(QMediaPlayer *player);
    void startAmbienceCrossfade(QMediaPlayer *fadingPlayer);
    void updateAmbienceCrossfade();
    void finishAmbienceCrossfade();
    void handleAmbienceEndOfMedia(QMediaPlayer *player);
    void updateAmbienceFadeOut();
    void stopAmbienceImmediately();

    void updateColoredNoiseFadeIn();
    void checkColoredNoiseCrossfade(QMediaPlayer *player);
    void startColoredNoiseCrossfade(QMediaPlayer *fadingPlayer);
    void updateColoredNoiseCrossfade();
    void finishColoredNoiseCrossfade();
    void handleColoredNoiseEndOfMedia(QMediaPlayer *player);
    void updateColoredNoiseFadeOut();
    void stopColoredNoiseImmediately();
    void updateColoredNoiseWatchdog();

    int ambienceTargetVolume() const;
    int coloredNoiseTargetVolume() const;

    void connectAmbiencePlayer(QMediaPlayer *player);
    void connectColoredNoisePlayer(QMediaPlayer *player);

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
    QTimer *m_coloredNoiseWatchdogTimer;

    QTimer *m_binauralFadeOutTimer;
    QTimer *m_ambienceFadeOutTimer;
    QTimer *m_coloredNoiseFadeOutTimer;

    QMediaPlayer *m_activeAmbiencePlayer;
    QMediaPlayer *m_fadingAmbiencePlayer;

    QMediaPlayer *m_activeColoredNoisePlayer;
    QMediaPlayer *m_fadingColoredNoisePlayer;

    QElapsedTimer m_ambienceCrossfadeElapsed;
    QElapsedTimer m_coloredNoiseCrossfadeElapsed;

    QElapsedTimer m_binauralFadeOutElapsed;
    QElapsedTimer m_ambienceFadeOutElapsed;
    QElapsedTimer m_coloredNoiseFadeOutElapsed;

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

    double m_binauralFadeOutStartVolume;

    int m_ambienceFadeOutActiveVolume;
    int m_ambienceFadeOutFadingVolume;

    int m_coloredNoiseFadeOutActiveVolume;
    int m_coloredNoiseFadeOutFadingVolume;
};

#endif
