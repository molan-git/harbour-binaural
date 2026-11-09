#include "audioengine.h"

#include <QMap>
#include <QDebug>
#include <QtMath>

namespace
{
    const QMap<QString, QString> &ambienceFiles()
    {
        static const QMap<QString, QString> files = {
            { QStringLiteral("Wind"),     QStringLiteral("wind.mp3") },
            { QStringLiteral("Rain"),     QStringLiteral("rain.mp3") },
            { QStringLiteral("Stream"),   QStringLiteral("stream.mp3") },
            { QStringLiteral("Waves"),    QStringLiteral("waves.mp3") },
            { QStringLiteral("Lake"),     QStringLiteral("lake.mp3") },
            { QStringLiteral("Sailing"),  QStringLiteral("sailing.mp3") },
            { QStringLiteral("Birds"),    QStringLiteral("birds.mp3") },
            { QStringLiteral("Crickets"), QStringLiteral("crickets.mp3") },
            { QStringLiteral("Chimes"),   QStringLiteral("chimes.mp3") },
            { QStringLiteral("Fire"),     QStringLiteral("fire.mp3") }
        };

        return files;
    }
}

// ============================================================================
// STATE
// ============================================================================

bool AudioEngine::isAmbiencePlaying() const
{
    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        if (m_ambienceChannels[i]
            && m_ambienceChannels[i]->isPlaying()) {
            return true;
        }
    }

    return false;
}

bool AudioEngine::isAmbiencePaused() const
{
    bool foundPausedChannel = false;

    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        AmbienceChannel *channel = m_ambienceChannels[i];

        if (!channel)
            continue;

        if (channel->isPlaying())
            return false;

        if (channel->isPaused())
            foundPausedChannel = true;
    }

    return foundPausedChannel;
}

// ============================================================================
// PLAYBACK
// ============================================================================

void AudioEngine::setAmbience(const QString &ambience)
{
    const auto it = ambienceFiles().constFind(ambience);

    if (it == ambienceFiles().constEnd()) {
        qWarning() << "AudioEngine: unknown ambience:" << ambience;
        return;
    }

    // Do not start the same sound in multiple channels.
    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        AmbienceChannel *channel = m_ambienceChannels[i];

        if (channel
                && channel->fileName() == it.value()
                && !channel->isStopping())
            return;
    }

    // Shared three-slot budget: preset ambiences and the custom
    // sound channel draw from the same pool. When the custom sound
    // is active, only two ambience slots remain.
    const int allowedSlots = m_customChannel
        && m_customChannel->isPlaying()
        ? MaxAmbienceChannels - 1
        : MaxAmbienceChannels;

    int activeSlots = 0;

    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        AmbienceChannel *channel = m_ambienceChannels[i];

        if (channel
                && !channel->fileName().isEmpty()
                && !channel->isStopping())
            ++activeSlots;
    }

    if (activeSlots >= allowedSlots) {
        qWarning()
            << "AudioEngine: maximum of"
            << allowedSlots
            << "simultaneous ambiences reached"
            << "(custom sound active)";
        return;
    }

    // Find the first free channel.
    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        AmbienceChannel *channel = m_ambienceChannels[i];

        if (!channel || !channel->fileName().isEmpty())
            continue;

        channel->setVolume(m_ambienceVolume);

        if (!channel->start(it.value())) {
            qWarning()
                << "AudioEngine: failed to start ambience:"
                << ambience;
        }

        updateAmbienceMixScale();
        updatePlayingState();
        return;
    }

    // All allowed channels are occupied. Reject the extra sound.
    qWarning()
        << "AudioEngine: maximum of"
        << MaxAmbienceChannels
        << "simultaneous ambiences reached";
}

void AudioEngine::pauseAmbience()
{
    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        if (m_ambienceChannels[i])
            m_ambienceChannels[i]->pause();
    }

    updatePlayingState();
}

void AudioEngine::resumeAmbience()
{
    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        if (m_ambienceChannels[i])
            m_ambienceChannels[i]->resume();
    }

    updatePlayingState();
}

void AudioEngine::stopAmbience()
{
    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        if (m_ambienceChannels[i])
            m_ambienceChannels[i]->stop();
    }

    updateAmbienceMixScale();
    updatePlayingState();
}

void AudioEngine::stopAmbienceByName(const QString &ambience)
{
    const auto it = ambienceFiles().constFind(ambience);

    if (it == ambienceFiles().constEnd())
        return;

    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        AmbienceChannel *channel = m_ambienceChannels[i];

        if (channel && channel->fileName() == it.value()) {
            // Stop only the requested sound with a short fade-out,
            // consistent with the binaural and colored-noise channels.
            // AmbienceChannel emits finished() when the fade completes.
            channel->stop();
            break;
        }
    }

    updateAmbienceMixScale();
    updatePlayingState();
}

void AudioEngine::stopAmbiencesImmediately()
{
    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        if (m_ambienceChannels[i])
            m_ambienceChannels[i]->stopImmediately();
    }

    updateAmbienceMixScale();
    updatePlayingState();
}

// ============================================================================
// MIX BALANCE
// ============================================================================

/*
 * Keeps the perceived loudness of the ambience bed constant no
 * matter how many sounds are stacked: N uncorrelated sources with
 * 1/sqrt(N) gain sum to roughly the same loudness as one source.
 * The custom sound channel is counted as one voice of the same
 * bed, since it draws from the same three-slot budget.
 */

void AudioEngine::updateAmbienceMixScale()
{
    int active = 0;

    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        const AmbienceChannel *channel =
            m_ambienceChannels[i];

        if (channel
            && !channel->fileName().isEmpty()
            && !channel->isStopping())
            ++active;
    }

    if (m_customChannel
            && m_customChannel->isPlaying())
        ++active;

    const double scale = active > 0
        ? 1.0 / qSqrt(static_cast<double>(active))
        : 1.0;

    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        if (m_ambienceChannels[i])
            m_ambienceChannels[i]->setMixScale(scale);
    }

    if (m_customChannel)
        m_customChannel->setMixScale(scale);
}

// ============================================================================
// VOLUME
// ============================================================================

void AudioEngine::setAmbienceVolume(int volume)
{
    m_ambienceVolume = qBound(0, volume, 100);

    for (int i = 0; i < MaxAmbienceChannels; ++i) {
        if (m_ambienceChannels[i])
            m_ambienceChannels[i]->setVolume(m_ambienceVolume);
    }
}
