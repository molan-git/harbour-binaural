#ifndef AMBIENCECHANNEL_H
#define AMBIENCECHANNEL_H

#include <QObject>
#include <QMediaPlayer>
#include <QElapsedTimer>
#include <QTimer>
#include <QString>
#include <QUrl>

class AmbienceChannel : public QObject
{
    Q_OBJECT

public:
    explicit AmbienceChannel(QObject *parent = nullptr);
    ~AmbienceChannel() override;

    // Preset ambience: resolves the file name against the
    // built-in sounds directory and plays it looping.
    bool start(const QString &fileName);

    // Arbitrary URL (custom user sound); displayName is
    // reported back by fileName() for occupancy checks.
    bool startUrl(const QUrl &url, const QString &displayName);

    // false = play once and emit finished() at natural end
    // (used for custom-sound playlists).
    void setLooping(bool looping);

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

    bool m_looping = true;

    double m_sleepFactor = 1.0;

    // Watchdog / mix balance / dynamic crossfade length
    int m_watchdogRetries = 0;
    double m_mixScale = 1.0;
    qint64 m_crossfadeMs = 8000;   // ms, updated per crossfade
};

#endif
