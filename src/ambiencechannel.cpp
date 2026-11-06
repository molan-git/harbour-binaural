#include "ambiencechannel.h"

#include <QDebug>
#include <QMediaContent>
#include <QUrl>
#include <QtMath>

namespace
{
    const int FadeInDuration = 200;
    const int FadeOutDuration = 200;
    const int CrossfadeDuration = 8000;

    const int FadeInterval = 15;
    const int VolumeUpdateInterval = 50;

    const int MasterVolumeFactorPercent = 60;

    const QString AmbienceBasePath =
        QStringLiteral("file:///usr/share/harbour-binaural/sounds/");
}

AmbienceChannel::AmbienceChannel(QObject *parent)
    : QObject(parent),
      m_playerA(new QMediaPlayer(this)),
      m_playerB(new QMediaPlayer(this)),
      m_activePlayer(nullptr),
      m_fadingPlayer(nullptr),
      m_fadeInTimer(new QTimer(this)),
      m_crossfadeTimer(new QTimer(this)),
      m_fadeOutTimer(new QTimer(this)),
      m_volumeTimer(new QTimer(this)),
      m_watchdogTimer(new QTimer(this)),
      m_volume(50),
      m_fadeOutActiveVolume(0),
      m_fadeOutFadingVolume(0),
      m_stopping(false),
      m_paused(false),
      m_resumeFadeIn(false),
      m_resumeCrossfade(false),
      m_sleepFactor(1.0)
{
    m_fadeInTimer->setInterval(FadeInterval);
    m_crossfadeTimer->setInterval(FadeInterval);
    m_fadeOutTimer->setInterval(FadeInterval);

    m_volumeTimer->setInterval(VolumeUpdateInterval);
    m_volumeTimer->setSingleShot(true);

    connectPlayer(m_playerA);
    connectPlayer(m_playerB);

    connect(m_fadeInTimer,
            &QTimer::timeout,
            this,
            [this]() {
                updateFadeIn();
            });

    connect(m_crossfadeTimer,
            &QTimer::timeout,
            this,
            [this]() {
                updateCrossfade();
            });

    connect(m_fadeOutTimer,
            &QTimer::timeout,
            this,
            [this]() {
                updateFadeOut();
            });

    connect(m_volumeTimer,
            &QTimer::timeout,
            this,
            [this]() {
                applyPendingVolume();
            });

    // External-stop watchdog (audio resource loss).
    m_watchdogTimer->setInterval(500);
    m_watchdogTimer->setSingleShot(true);

    connect(m_watchdogTimer,
            &QTimer::timeout,
            this,
            &AmbienceChannel::restartActivePlayer);

    m_playerA->setVolume(0);
    m_playerB->setVolume(0);
}

AmbienceChannel::~AmbienceChannel()
{
    m_fadeInTimer->stop();
    m_crossfadeTimer->stop();
    m_fadeOutTimer->stop();
    m_volumeTimer->stop();
    m_watchdogTimer->stop();

    m_playerA->stop();
    m_playerB->stop();
}

bool AmbienceChannel::isPlaying() const
{
    return m_activePlayer
        && m_activePlayer->state() == QMediaPlayer::PlayingState;
}

bool AmbienceChannel::isPaused() const
{
    return m_paused
        && m_activePlayer
        && m_activePlayer->state() == QMediaPlayer::PausedState;
}

bool AmbienceChannel::isStopping() const
{
    return m_stopping;
}

QString AmbienceChannel::fileName() const
{
    return m_fileName;
}

bool AmbienceChannel::start(const QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    return startUrl(QUrl(AmbienceBasePath + fileName), fileName);
}

bool AmbienceChannel::startUrl(const QUrl &url,
                               const QString &displayName)
{
    if (url.isEmpty() || !url.isValid())
        return false;

    stopImmediately();

    m_fileName = displayName;
    m_activePlayer = m_playerA;
    m_activePlayer->setMedia(QMediaContent(url));
    m_activePlayer->setVolume(0);

    m_fadeInElapsedBeforePause = 0;
    m_crossfadeElapsedBeforePause = 0;
    m_fadeInElapsed.invalidate();
    m_crossfadeElapsed.invalidate();

    m_stopping = false;
    m_paused = false;
    m_sleepFactor = 1.0;

    m_resumeFadeIn = true;
    m_resumeCrossfade = false;

    m_activePlayer->play();
    m_fadeInElapsed.start();
    m_fadeInTimer->start();

    emit stateChanged();

    return true;
}

void AmbienceChannel::setLooping(bool looping)
{
    m_looping = looping;
}

void AmbienceChannel::setCrossfadeEnabled(bool enabled)
{
    m_crossfadeEnabled = enabled;
}

void AmbienceChannel::pause()
{
    if (!m_activePlayer || m_paused)
        return;

    const bool activePlaying =
        m_activePlayer->state() == QMediaPlayer::PlayingState;

    const bool fadingPlaying =
        m_fadingPlayer
            && m_fadingPlayer->state() == QMediaPlayer::PlayingState;

    if (!activePlaying && !fadingPlaying)
        return;

    m_resumeFadeIn = m_fadeInTimer->isActive();
    m_resumeCrossfade = m_crossfadeTimer->isActive();

    if (m_stopping && m_fadeElapsed.isValid()) {
        m_fadeElapsedBeforePause += m_fadeElapsed.elapsed();
        m_fadeElapsed.invalidate();
    }

    if (m_fadeInTimer->isActive() && m_fadeInElapsed.isValid()) {
        m_fadeInElapsedBeforePause += m_fadeInElapsed.elapsed();
        m_fadeInElapsed.invalidate();
    }

    if (m_crossfadeTimer->isActive() && m_crossfadeElapsed.isValid()) {
        m_crossfadeElapsedBeforePause += m_crossfadeElapsed.elapsed();
        m_crossfadeElapsed.invalidate();
    }

    m_fadeInTimer->stop();
    m_crossfadeTimer->stop();
    m_fadeOutTimer->stop();
    m_volumeTimer->stop();

    m_activePlayer->pause();

    if (m_fadingPlayer)
        m_fadingPlayer->pause();

    m_paused = true;

    emit stateChanged();
}

void AmbienceChannel::resume()
{
    if (!m_activePlayer || !m_paused)
        return;

    m_paused = false;

    m_activePlayer->play();

    if (m_fadingPlayer)
        m_fadingPlayer->play();

    if (m_stopping) {
        m_fadeElapsed.start();
        m_fadeOutTimer->start();
    } else if (m_resumeCrossfade && m_fadingPlayer) {
        m_crossfadeElapsed.start();
        m_crossfadeTimer->start();
    } else if (m_resumeFadeIn) {
        m_fadeInElapsed.start();
        m_fadeInTimer->start();
    } else {
        applyPendingVolume();
    }

    emit stateChanged();
}

void AmbienceChannel::stop()
{
    if (!m_activePlayer || m_stopping)
        return;

    m_resumeFadeIn = false;
    m_resumeCrossfade = false;

    m_fadeInTimer->stop();
    m_crossfadeTimer->stop();
    m_volumeTimer->stop();

    m_fadeOutActiveVolume = m_activePlayer->volume();

    m_fadeOutFadingVolume =
        m_fadingPlayer ? m_fadingPlayer->volume() : 0;

    m_fadeElapsedBeforePause = 0;
    m_fadeElapsed.start();

    m_stopping = true;
    m_paused = false;

    m_fadeOutTimer->start();
}

void AmbienceChannel::stopImmediately()
{
    m_fadeInTimer->stop();
    m_crossfadeTimer->stop();
    m_fadeOutTimer->stop();
    m_volumeTimer->stop();
    m_watchdogTimer->stop();
    m_watchdogRetries = 0;

    m_playerA->stop();
    m_playerB->stop();

    m_playerA->setVolume(0);
    m_playerB->setVolume(0);

    m_activePlayer = nullptr;
    m_fadingPlayer = nullptr;

    m_fileName.clear();

    m_fadeInElapsed.invalidate();
    m_fadeInElapsedBeforePause = 0;
    m_crossfadeElapsed.invalidate();
    m_crossfadeElapsedBeforePause = 0;

    m_fadeElapsed.invalidate();
    m_fadeElapsedBeforePause = 0;

    m_fadeOutActiveVolume = 0;
    m_fadeOutFadingVolume = 0;

    m_stopping = false;
    m_paused = false;
    m_sleepFactor = 1.0;

    m_resumeFadeIn = true;
    m_resumeCrossfade = false;

    emit stateChanged();
}

void AmbienceChannel::setVolume(int volume)
{
    m_volume = qBound(0, volume, 100);
    m_volumeTimer->start();
}

int AmbienceChannel::currentVolume() const
{
    return m_activePlayer ? m_activePlayer->volume() : 0;
}

/*
 * Loudness compensation for stacked ambiences. The engine
 * sets 1/sqrt(activeCount); targetVolume() feeds all fade paths,
 * so the balance flows through fade-in, crossfade and sleep fade
 * automatically.
 */
void AmbienceChannel::setMixScale(double scale)
{
    m_mixScale = qBound(0.1, scale, 1.0);
    m_volumeTimer->start();      // debounced apply, like the slider
}

int AmbienceChannel::targetVolume() const
{
    return qRound(
        m_volume * MasterVolumeFactorPercent / 100.0
        * m_mixScale);
}

void AmbienceChannel::applySleepVolume(
    int startVolume,
    double factor)
{
    factor = qBound(0.0, factor, 1.0);
    m_sleepFactor = factor;

    // If a crossfade or fade-in is in progress, that fade is already
    // updating volume every 15 ms using m_sleepFactor — don't fight it.
    if (m_crossfadeTimer->isActive() || m_fadeInTimer->isActive())
        return;

    if (m_activePlayer) {
        m_activePlayer->setVolume(
            qRound(startVolume * factor));
    }

    if (m_fadingPlayer) {
        m_fadingPlayer->setVolume(0);
    }
}

void AmbienceChannel::applyPendingVolume()
{
    if (!m_activePlayer || m_stopping || m_paused)
        return;

    if (m_fadeInTimer->isActive()
        || m_crossfadeTimer->isActive()
        || m_fadeOutTimer->isActive()) {
        return;
    }

    // Honor the sleep-timer factor so a pause/resume during the
    // sleep fade-out does not jump back to full target volume.
    m_activePlayer->setVolume(
                qRound(targetVolume() * m_sleepFactor));
}

void AmbienceChannel::updateFadeIn()
{
    if (!m_activePlayer || m_paused || m_stopping) {
        m_fadeInTimer->stop();
        return;
    }

    const qint64 elapsed = m_fadeInElapsedBeforePause
            + m_fadeInElapsed.elapsed();

    const double progress = qMin(
                1.0,
                static_cast<double>(elapsed) / FadeInDuration);

    // Honor the sleep-timer factor so the fade-in does not fight
    // the global sleep fade-out.
    const int volume = qRound(targetVolume() * m_sleepFactor);

    m_activePlayer->setVolume(qRound(volume * progress));

    if (progress >= 1.0) {
        m_activePlayer->setVolume(volume);

        m_fadeInElapsedBeforePause = 0;
        m_fadeInElapsed.invalidate();
        m_resumeFadeIn = false;

        m_fadeInTimer->stop();
    }
}

void AmbienceChannel::updateFadeOut()
{
    if (!m_activePlayer || !m_stopping) {
        m_fadeOutTimer->stop();
        return;
    }

    if (m_paused)
        return;

    const qint64 elapsed = m_fadeElapsedBeforePause
        + m_fadeElapsed.elapsed();

    const double progress = qMin(
                1.0,
                static_cast<double>(elapsed) / FadeOutDuration);

    m_activePlayer->setVolume(
        qRound(m_fadeOutActiveVolume * (1.0 - progress)));

    if (m_fadingPlayer) {
        m_fadingPlayer->setVolume(
            qRound(m_fadeOutFadingVolume * (1.0 - progress)));
    }

    if (progress >= 1.0) {
        const bool shouldEmitFinished = m_stopping;

        stopImmediately();

        if (shouldEmitFinished)
            emit finished();
    }
}

void AmbienceChannel::checkCrossfade(QMediaPlayer *player)
{
    if (!player
        || player != m_activePlayer
        || m_fadingPlayer
        || m_stopping
        || m_paused) {
        return;
    }

    // Non-looping tracks must not loop-crossfade into themselves;
    // they play to their natural end instead.
    if (!m_looping || !m_crossfadeEnabled)
        return;

    const qint64 duration = player->duration();

    if (duration <= 0)
        return;

    /*
     * Crossfade at most half the file, never longer than
     * the default. Files too short to crossfade at all fall
     * through to the seamless EndOfMedia fallback.
     */
    const qint64 fadeMs =
        qMin<qint64>(CrossfadeDuration, duration / 2);

    if (fadeMs < 250)
        return;

    if (duration - player->position() <= fadeMs)
        startCrossfade(player, fadeMs);
}

void AmbienceChannel::startCrossfade(QMediaPlayer *fadingPlayer,
                                     qint64 fadeMs)
{
    if (!fadingPlayer
            || fadingPlayer != m_activePlayer
            || m_fadingPlayer
            || m_stopping
            || m_paused) {
        return;
    }

    // Remember the length of this crossfade
    // (updateCrossfade reads m_crossfadeMs).
    m_crossfadeMs =
        fadeMs > 0 ? fadeMs : CrossfadeDuration;

    QMediaPlayer *nextPlayer =
        fadingPlayer == m_playerA ? m_playerB : m_playerA;

    nextPlayer->stop();
    nextPlayer->setMedia(fadingPlayer->currentMedia());
    nextPlayer->setPosition(0);
    nextPlayer->setVolume(0);

    m_fadingPlayer = fadingPlayer;
    m_activePlayer = nextPlayer;

    m_crossfadeElapsedBeforePause = 0;
    m_fadeInElapsedBeforePause = 0;
    m_fadeInElapsed.invalidate();

    m_resumeFadeIn = false;
    m_resumeCrossfade = true;

    nextPlayer->play();

    m_fadeInTimer->stop();
    m_crossfadeElapsed.start();
    m_crossfadeTimer->start();
}

void AmbienceChannel::updateCrossfade()
{
    if (!m_activePlayer || !m_fadingPlayer
            || m_paused || m_stopping) {
        m_crossfadeTimer->stop();
        return;
    }

    const qint64 elapsed = m_crossfadeElapsedBeforePause
            + m_crossfadeElapsed.elapsed();

    const double progress = qMin(
                1.0,
                static_cast<double>(elapsed)
                    / m_crossfadeMs);

    // Apply sleep factor so crossfade respects the fade-out.
    const int volume = qRound(targetVolume() * m_sleepFactor);

    /*
     * Equal-power curves. With a linear fade the summed
     * loudness dips ~3 dB in the middle; with uncorrelated
     * material sqrt curves keep it constant through the seam.
     */
    m_fadingPlayer->setVolume(
                qRound(volume * qSqrt(1.0 - progress)));

    m_activePlayer->setVolume(
                qRound(volume * qSqrt(progress)));

    if (progress >= 1.0)
        finishCrossfade();
}

void AmbienceChannel::finishCrossfade()
{
    m_crossfadeTimer->stop();

    m_crossfadeElapsedBeforePause = 0;
    m_crossfadeElapsed.invalidate();
    m_resumeCrossfade = false;

    if (m_fadingPlayer) {
        m_fadingPlayer->stop();
        m_fadingPlayer->setVolume(0);
        m_fadingPlayer = nullptr;
    }

    if (m_activePlayer)
        m_activePlayer->setVolume(
            qRound(targetVolume() * m_sleepFactor));
}

void AmbienceChannel::handleEndOfMedia(QMediaPlayer *player)
{
    if (player == m_fadingPlayer) {
        player->stop();
        player->setVolume(0);
        m_fadingPlayer = nullptr;
        return;
    }

    if (player != m_activePlayer)
        return;

    if (m_stopping) {
        const bool shouldEmitFinished = m_stopping;

        stopImmediately();

        if (shouldEmitFinished)
            emit finished();

        return;
    }

    if (m_paused)
        return;

    /*
     * Non-looping track (custom sound playlist or single
     * play with loop off): end naturally. stop() runs the
     * short fade-out bookkeeping and emits finished() when
     * done — the engine then advances the playlist.
     */
    if (!m_looping) {
        stop();
        return;
    }

    /*
     * EndOfMedia without a running crossfade means the
     * position check never fired (very short file, or a
     * backend that reports duration() == 0). Use the free player
     * for a quick loop crossfade instead of a hard setPosition(0)
     * restart, which would click.
     */
    if (!m_fadingPlayer) {
        if (m_crossfadeEnabled) {
            startCrossfade(player, 500);
            return;
        }

        // Crossfade disabled: hard restart at the loop point.
        player->setPosition(0);
        player->play();
        return;
    }

    player->setPosition(0);
    player->play();
}

void AmbienceChannel::connectPlayer(QMediaPlayer *player)
{
    connect(player,
            &QMediaPlayer::positionChanged,
            this,
            [this, player](qint64) {
                checkCrossfade(player);
            });

    connect(player,
            &QMediaPlayer::mediaStatusChanged,
            this,
            [this, player](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::EndOfMedia) {
                    handleEndOfMedia(player);
                } else if (status == QMediaPlayer::InvalidMedia) {
                    qWarning()
                        << "AmbienceChannel: invalid media:"
                        << player->media()
                               .canonicalUrl()
                               .toString();

                    if (player == m_activePlayer
                        && !m_stopping) {
                        stopImmediately();
                        emit finished();
                    }
                }
            });

    connect(
        player,
        static_cast<void (QMediaPlayer::*)(
            QMediaPlayer::Error)>(&QMediaPlayer::error),
        this,
        [this, player](QMediaPlayer::Error error) {
            qWarning()
                << "AmbienceChannel: player error:"
                << error
                << player->errorString();

            if (player == m_activePlayer
                && error != QMediaPlayer::NoError
                && !m_stopping) {
                stopImmediately();
                emit finished();
            }
        });

    connect(player,
            &QMediaPlayer::stateChanged,
            this,
            [this, player](QMediaPlayer::State state) {

                if (state == QMediaPlayer::StoppedState
                    && player == m_activePlayer
                    && !m_stopping
                    && !m_paused) {
                    if (m_watchdogRetries < 5) {
                        ++m_watchdogRetries;
                        m_watchdogTimer->start();
                    } else {
                        stopImmediately();
                        emit finished();
                    }
                }

                if (state == QMediaPlayer::PlayingState) {
                    m_watchdogTimer->stop();
                    m_watchdogRetries = 0;
                }

                emit stateChanged();
            });
}

/*
 * Watchdog recovery: the active player was stopped externally
 * (audio resource seized by another app, or a backend hiccup).
 * Try to resume playback - the stateChanged handler stops the
 * watchdog again as soon as PlayingState is confirmed.
 */
void AmbienceChannel::restartActivePlayer()
{
    if (!m_activePlayer
        || m_stopping
        || m_paused
        || m_activePlayer->state()
               != QMediaPlayer::StoppedState)
        return;

    qWarning() << "AmbienceChannel: recovering player for"
               << m_fileName;

    m_activePlayer->play();
}
