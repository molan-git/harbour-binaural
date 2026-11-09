#include "audioengine.h"

#include <QUrl>
#include <QDebug>
#include <QVariantMap>

// ============================================================================
// STATE
// ============================================================================

bool AudioEngine::isCustomSoundPlaying() const
{
    return m_customChannel
        && m_customChannel->isPlaying();
}

bool AudioEngine::isCustomSoundPaused() const
{
    return m_customChannel
        && m_customChannel->isPaused();
}

bool AudioEngine::isCustomSoundPlaylistActive() const
{
    return m_playlistActive;
}

QUrl AudioEngine::customSoundSource() const
{
    return m_customSource;
}

QString AudioEngine::customSoundName() const
{
    return m_customName;
}

int AudioEngine::customSoundVolume() const
{
    return m_customVolume;
}

bool AudioEngine::customSoundLoop() const
{
    return m_customLoop;
}

bool AudioEngine::customSoundCrossfade() const
{
    return m_customCrossfade;
}

void AudioEngine::setCustomSoundSource(const QUrl &url)
{
    if (m_customSource == url)
        return;

    m_customSource = url;
    emit customSoundChanged();
}

void AudioEngine::setCustomSoundName(const QString &name)
{
    m_customName = name;
    emit customSoundChanged();
}

void AudioEngine::setCustomSoundVolume(int volume)
{
    m_customVolume = qBound(0, volume, 100);

    if (m_customChannel)
        m_customChannel->setVolume(m_customVolume);

    emit customSoundChanged();
}

void AudioEngine::setCustomSoundLoop(bool loop)
{
    m_customLoop = loop;
    m_playlistLoop = loop;

    if (m_customChannel && !m_playlistActive)
        m_customChannel->setLooping(loop);

    emit customSoundChanged();
}

void AudioEngine::setCustomSoundCrossfade(bool crossfade)
{
    if (m_customCrossfade == crossfade)
        return;

    m_customCrossfade = crossfade;

    if (m_customChannel)
        m_customChannel->setCrossfadeEnabled(crossfade);

    emit customSoundChanged();
}

void AudioEngine::restartCustomSound()
{
    if (m_playlistActive && !m_playlistUrls.isEmpty()) {
        playCustomTrack(0);
    } else {
        playCustomTrack(-1);
    }
}

// ============================================================================
// PLAYBACK
// ============================================================================

// index < 0: play m_customSource as a single sound.
void AudioEngine::playCustomTrack(int index)
{
    QUrl url;
    QString name;

    if (index >= 0) {
        if (index >= m_playlistUrls.size())
            return;

        url = m_playlistUrls.at(index);
        name = m_playlistNames.at(index);
        m_playlistIndex = index;
    } else {
        if (m_customSource.isEmpty() || !m_customSource.isValid())
            return;

        url = m_customSource;
        name = m_customName;
    }

    m_customManualStop = false;

    m_customChannel->setVolume(m_customVolume);
    m_customChannel->setCrossfadeEnabled(m_customCrossfade);

    // Playlist tracks play once; the engine advances on finished().
    m_customChannel->setLooping(m_playlistActive
                                ? false
                                : m_customLoop);

    if (!m_customChannel->startUrl(url, name)) {
        qWarning() << "AudioEngine: failed to start custom sound:"
                   << url.toString();
        return;
    }

    m_customSource = url;
    m_customName = name;

    emit customSoundChanged();
    updateAmbienceMixScale();
    updatePlayingState();
}

void AudioEngine::playCustomSound()
{
    m_playlistActive = false;
    playCustomTrack(-1);
}

void AudioEngine::stopCustomSound()
{
    if (!m_customChannel
        || m_customChannel->fileName().isEmpty())
        return;

    m_customManualStop = true;
    m_customChannel->stop();

    updateAmbienceMixScale();
    updatePlayingState();
}

void AudioEngine::pauseCustomSound()
{
    if (m_customChannel)
        m_customChannel->pause();

    updatePlayingState();
}

void AudioEngine::resumeCustomSound()
{
    if (m_customChannel)
        m_customChannel->resume();

    updatePlayingState();
}

void AudioEngine::startCustomSoundPlaylist(
    const QVariantList &files,
    bool loopFolder)
{
    QVector<QUrl> urls;
    QVector<QString> names;

    for (int i = 0; i < files.size(); ++i) {
        const QVariantMap entry = files.at(i).toMap();

        if (!entry.contains(QStringLiteral("url"))
            || !entry.contains(QStringLiteral("name")))
            continue;

        urls.append(entry.value(QStringLiteral("url")).toUrl());
        names.append(
            entry.value(QStringLiteral("name")).toString());
    }

    if (urls.isEmpty())
        return;

    m_playlistUrls = urls;
    m_playlistNames = names;
    m_playlistLoop = loopFolder;
    m_playlistActive = true;

    playCustomTrack(0);
}

void AudioEngine::playNextCustomSound()
{
    if (!m_playlistActive || m_playlistUrls.isEmpty())
        return;

    int next = (m_playlistIndex + 1) % m_playlistUrls.size();

    playCustomTrack(next);
}

void AudioEngine::playPreviousCustomSound()
{
    if (!m_playlistActive || m_playlistUrls.isEmpty())
        return;

    int previous = (m_playlistIndex - 1 + m_playlistUrls.size())
                   % m_playlistUrls.size();

    playCustomTrack(previous);
}

void AudioEngine::handleCustomTrackFinished()
{
    // Natural end of a non-looping track: advance the playlist.
    if (m_playlistActive && !m_customManualStop) {
        int next = m_playlistIndex + 1;

        if (next >= m_playlistUrls.size())
            next = m_playlistLoop ? 0 : -1;

        if (next >= 0) {
            playCustomTrack(next);
            return;
        }

        // Playlist finished on its own.
        m_playlistActive = false;
        emit customSoundChanged();
    }

    updateAmbienceMixScale();
    updatePlayingState();
}
