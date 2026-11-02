#ifndef AMBIENCECHANNEL_H
#define AMBIENCECHANNEL_H

#include <QObject>
#include <QMediaPlayer>
#include <QElapsedTimer>
#include <QTimer>
#include <QString>

class AmbienceChannel : public QObject
{
    Q_OBJECT

public:
    explicit AmbienceChannel(QObject *parent = nullptr);
    ~AmbienceChannel() override;

    bool start(const QString &fileName);

    void pause();
    void stop();
    void resume();
    void stopImmediately();

    bool isPlaying() const;
    bool isPaused() const;
    bool isStopping() const;

    QString fileName() const;

    void setVolume(int volume);
    int currentVolume() const;

    // Loudness compensation when several ambiences stack:
    // the engine sets 1/sqrt(activeCount) on every channel.
    void setMixScale(double scale);

    void applySleepVolume(int startVolume, double factor);

signals:
    void stateChanged();
    void finished();

private:
    void connectPlayer(QMediaPlayer *player);

    void updateFadeIn();
    void updateFadeOut();
    void updateCrossfade();

    void checkCrossfade(QMediaPlayer *player);

    // fadeMs <= 0 means "auto" (derived from the file duration)
    void startCrossfade(QMediaPlayer *fadingPlayer,
                        qint64 fadeMs = 0);

    void finishCrossfade();
    void handleEndOfMedia(QMediaPlayer *player);

    // Recovers a player that was stopped externally
    // (audio resource loss on Sailfish).
    void restartActivePlayer();

    void applyPendingVolume();
    int targetVolume() const;

    QMediaPlayer *m_playerA;
    QMediaPlayer *m_playerB;

    QMediaPlayer *m_activePlayer;
    QMediaPlayer *m_fadingPlayer;

    QTimer *m_fadeInTimer;
    QTimer *m_crossfadeTimer;
    QTimer *m_fadeOutTimer;
    QTimer *m_volumeTimer;
    QTimer *m_watchdogTimer;

    int m_volume;
    QString m_fileName;

    QElapsedTimer m_fadeInElapsed;
    QElapsedTimer m_crossfadeElapsed;
    qint64 m_fadeInElapsedBeforePause = 0;
    qint64 m_crossfadeElapsedBeforePause = 0;

    QElapsedTimer m_fadeElapsed;
    qint64 m_fadeElapsedBeforePause = 0;

    int m_fadeOutActiveVolume;
    int m_fadeOutFadingVolume;

    bool m_stopping;
    bool m_paused;

    bool m_resumeFadeIn;
    bool m_resumeCrossfade;

    double m_sleepFactor = 1.0;

    // Wwatchdog / mix balance / dynamic crossfade length
    int m_watchdogRetries = 0;
    double m_mixScale = 1.0;
    qint64 m_crossfadeMs = 8000;   // ms, updated per crossfade
};

#endif
