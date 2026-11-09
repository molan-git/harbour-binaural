#ifndef COLOREDNOISEGENERATOR_H
#define COLOREDNOISEGENERATOR_H

#include <QIODevice>
#include <QtGlobal>
#include <QtCore/QObject>

class ColoredNoiseGenerator : public QIODevice
{
public:
    enum NoiseType
    {
        WhiteNoise = 0,
        PinkNoise,
        BrownNoise,
        GreyNoise
    };

    explicit ColoredNoiseGenerator(QObject *parent = nullptr);

    void setSampleRate(int sampleRate);
    int sampleRate() const;

    void setChannelCount(int channelCount);
    int channelCount() const;

    void setNoiseType(NoiseType type);
    NoiseType noiseType() const;

    /*
     * Ocean-wave swell modulation.
     * depth:         0.0 (off) .. 1.0 (dips to silence)
     * periodSeconds: duration of one full swell
     */
    void setWaveModulation(double depth,
                           double periodSeconds);

    void start();

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    double noiseGain() const;
    void resetNoiseState();

    double nextRawWhite(int channel);
    double nextWhite(int channel);
    double nextPink(int channel);
    double nextBrown(int channel);
    double nextGrey(int channel);

    int m_sampleRate;
    int m_channelCount;

    NoiseType m_noiseType;

    // Wave swell state (phase advances per frame, not per channel).
    double m_waveDepth;
    double m_wavePeriod;
    double m_wavePhase;

    quint32 m_randomState[2];

    double m_pinkState[2][7];
    double m_brownState[2];

    double m_greyLow[2];
    double m_greyMid[2];
    double m_greyHigh[2];

    double m_whiteFiltered[2];
};

#endif
