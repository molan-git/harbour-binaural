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
    void checkForCrossfade(QMediaPlayer *player);
    void startCrossfade(QMediaPlayer *fadingPlayer);
    void updateCrossfade();

    void updateBinauralFadeIn();
    void updateAmbienceFadeIn();
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

    QTimer *m_crossfadeTimer;
    QTimer *m_binauralFadeTimer;
    QTimer *m_ambienceFadeTimer;
    QTimer *m_coloredNoiseFadeTimer;
    QTimer *m_coloredNoiseCrossfadeTimer;

    QMediaPlayer *m_activeAmbiencePlayer;
    QMediaPlayer *m_fadingAmbiencePlayer;

    QMediaPlayer *m_activeColoredNoisePlayer;
    QMediaPlayer *m_fadingColoredNoisePlayer;

    double m_beatFrequency;

    int m_binauralVolume;
    int m_ambienceVolume;
    int m_coloredNoiseVolume;

    int m_crossfadePosition;
    int m_crossfadeDuration;

    int m_binauralFadePosition;
    int m_ambienceFadePosition;
    int m_coloredNoiseFadePosition;

    int m_coloredNoiseCrossfadePosition;
    int m_coloredNoiseCrossfadeDuration;
};

#endif
