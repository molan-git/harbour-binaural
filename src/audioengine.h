#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include <QObject>
#include <QElapsedTimer>
#include <QTimer>
#include <QString>
#include <QUrl>
#include <QVariantList>
#include <QVector>

#include "ambiencechannel.h"
#include "pcmchannel.h"

class BinauralGenerator;
class ColoredNoiseGenerator;

class AudioEngine : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool binauralPlaying
               READ isBinauralPlaying
               NOTIFY playingChanged)

    Q_PROPERTY(bool ambiencePlaying
               READ isAmbiencePlaying
               NOTIFY playingChanged)

    Q_PROPERTY(bool coloredNoisePlaying
               READ isColoredNoisePlaying
               NOTIFY playingChanged)

    Q_PROPERTY(bool binauralPaused
               READ isBinauralPaused
               NOTIFY playingChanged)

    Q_PROPERTY(bool ambiencePaused
               READ isAmbiencePaused
               NOTIFY playingChanged)

    Q_PROPERTY(bool coloredNoisePaused
               READ isColoredNoisePaused
               NOTIFY playingChanged)

    Q_PROPERTY(bool customSoundPlaying
               READ isCustomSoundPlaying
               NOTIFY customSoundChanged)

    Q_PROPERTY(bool customSoundPaused
               READ isCustomSoundPaused
               NOTIFY customSoundChanged)

    Q_PROPERTY(bool customSoundPlaylistActive
               READ isCustomSoundPlaylistActive
               NOTIFY customSoundChanged)

    Q_PROPERTY(QUrl customSoundSource
               READ customSoundSource
               WRITE setCustomSoundSource
               NOTIFY customSoundChanged)

    Q_PROPERTY(QString customSoundName
               READ customSoundName
               WRITE setCustomSoundName
               NOTIFY customSoundChanged)

    Q_PROPERTY(int customSoundVolume
               READ customSoundVolume
               WRITE setCustomSoundVolume
               NOTIFY customSoundChanged)

    Q_PROPERTY(bool customSoundLoop
               READ customSoundLoop
               WRITE setCustomSoundLoop
               NOTIFY customSoundChanged)

    Q_PROPERTY(bool customSoundCrossfade
               READ customSoundCrossfade
               WRITE setCustomSoundCrossfade
               NOTIFY customSoundChanged)

public:
    static constexpr int MaxAmbienceChannels = 3;

    explicit AudioEngine(QObject *parent = nullptr);
    ~AudioEngine() override;

    bool isBinauralPlaying() const;
    bool isAmbiencePlaying() const;
    bool isColoredNoisePlaying() const;

    bool isBinauralPaused() const;
    bool isAmbiencePaused() const;
    bool isColoredNoisePaused() const;

    bool isCustomSoundPlaying() const;
    bool isCustomSoundPaused() const;
    bool isCustomSoundPlaylistActive() const;

    QUrl customSoundSource() const;
    QString customSoundName() const;
    int customSoundVolume() const;
    bool customSoundLoop() const;
    bool customSoundCrossfade() const;

    Q_INVOKABLE void setCustomSoundSource(const QUrl &source);
    Q_INVOKABLE void setCustomSoundName(const QString &name);
    Q_INVOKABLE void setCustomSoundVolume(int volume);
    Q_INVOKABLE void setCustomSoundLoop(bool loop);
    Q_INVOKABLE void setCustomSoundCrossfade(bool crossfade);

    Q_INVOKABLE void fadeOutForSleepTimer();

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void resumeBinaural();
    Q_INVOKABLE void pauseBinaural();
    Q_INVOKABLE void setFrequencyBand(const QString &band);
    Q_INVOKABLE void setBinauralFrequency(double carrier,
                                          double beat);

    // Ambience controls.
    Q_INVOKABLE void setAmbience(const QString &ambience);
    Q_INVOKABLE void pauseAmbience();
    Q_INVOKABLE void stopAmbience();
    Q_INVOKABLE void stopAmbienceByName(const QString &ambience);
    Q_INVOKABLE void resumeAmbience();

    Q_INVOKABLE void setColoredNoise(const QString &noise);
    Q_INVOKABLE void setNoiseWave(bool enabled,
                                  double depth,
                                  double periodSeconds);
    Q_INVOKABLE void pauseColoredNoise();
    Q_INVOKABLE void stopColoredNoise();
    Q_INVOKABLE void resumeColoredNoise();

    // Custom user sound (mp3/ogg/wav) and playlist controls.
    Q_INVOKABLE void playCustomSound();
    Q_INVOKABLE void stopCustomSound();
    Q_INVOKABLE void pauseCustomSound();
    Q_INVOKABLE void resumeCustomSound();
    Q_INVOKABLE void playNextCustomSound();
    Q_INVOKABLE void playPreviousCustomSound();
    Q_INVOKABLE void restartCustomSound();
    Q_INVOKABLE void startCustomSoundPlaylist(const QVariantList &files,
                                               bool loopFolder);

    Q_INVOKABLE void setBinauralVolume(int volume);
    Q_INVOKABLE void setAmbienceVolume(int volume);
    Q_INVOKABLE void setColoredNoiseVolume(int volume);

signals:
    void playingChanged();
    void customSoundChanged();

private:
    void updatePlayingState();

    // Ambience implementation.
    void stopAmbiencesImmediately();
    void updateAmbienceState();
    void updateAmbienceMixScale();

    // Custom sound implementation.
    void playCustomTrack(int index);
    void handleCustomTrackFinished();

    // Colored-noise volume.
    void applyPendingColoredNoiseVolume();
    int coloredNoiseTargetVolume() const;

    // Sleep timer.
    QTimer *m_sleepTimerFadeTimer;
    QElapsedTimer m_sleepTimerFadeElapsed;

    double m_sleepTimerBinauralStartVolume;
    int m_sleepTimerAmbienceStartVolumes[MaxAmbienceChannels];
    double m_sleepTimerColoredNoiseStartVolume;
    int m_sleepTimerCustomStartVolume;

    // PCM playback channels (own output, generator, fade-out).
    PcmChannel *m_binauralChannel;
    PcmChannel *m_noiseChannel;

    // Generators (parameter access only; lifetime owned by PcmChannel).
    BinauralGenerator *m_binauralGenerator;
    ColoredNoiseGenerator *m_noiseGenerator;

    // Three independent ambience channels.
    AmbienceChannel *m_ambienceChannels[MaxAmbienceChannels];

    // Dedicated custom user sound channel.
    AmbienceChannel *m_customChannel;

    // Custom sound playlist.
    QVector<QUrl> m_playlistUrls;
    QVector<QString> m_playlistNames;
    int m_playlistIndex;
    bool m_playlistLoop;
    bool m_playlistActive;

    // Custom sound settings.
    QUrl m_customSource;
    QString m_customName;
    int m_customVolume;
    bool m_customLoop;
    bool m_customCrossfade;
    bool m_customManualStop;

    // Colored-noise volume debounce timer.
    QTimer *m_coloredNoiseVolumeTimer;

    // Audio settings.
    double m_beatFrequency;
    double m_carrierFrequency;

    int m_binauralVolume;
    int m_ambienceVolume;
    int m_coloredNoiseVolume;

    QString m_currentColoredNoise;

    // Wave swell settings for the colored-noise channel.
    bool m_noiseWaveEnabled;
    double m_noiseWaveDepth;
    double m_noiseWavePeriod;
};

#endif
