#ifndef SEQUENCEGENERATOR_H
#define SEQUENCEGENERATOR_H

#include <QIODevice>
#include <QVector>

#include "sbgsequence.h"

/*
 * Renders a parsed SBaGen sequence as stereo 16-bit PCM.
 *
 * Design notes:
 *
 * - The timeline position is derived from the number of frames
 *   actually rendered, so pausing the audio output (which stops
 *   readData calls) freezes the sequence at the right point and
 *   resuming continues seamlessly. No wall-clock timer needed.
 *
 * - Consecutive tone-sets are crossfaded over the last 10 s of
 *   each segment (SBaGen '-SE' spin behaviour). Each tone-set
 *   owns its own oscillator/noise state, so both sets render
 *   independently during the blend.
 *
 * - If the final tone-set is active (not 'off'), the sequence
 *   keeps playing it indefinitely until the channel is stopped;
 *   otherwise the generator reports finished() once the final
 *   fade to silence completes, and AudioEngine stops the channel.
 */

class SequenceGenerator : public QIODevice
{
public:
    explicit SequenceGenerator(const SbgSequence &sequence,
                               QObject *parent = nullptr);

    void setSampleRate(int sampleRate);
    void start();

    qint64 elapsedMs() const;
    qint64 durationMs() const;
    bool finished() const;

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override;

private:
    struct ToneRuntime
    {
        int voiceIndex = -1;
        double leftPhase = 0.0;
        double rightPhase = 0.0;
    };

    /*
     * Per-voice noise DSP state, mirroring ColoredNoiseGenerator
     * (independent deterministic state per channel). If these two
     * ever drift apart, extract a shared NoiseSource class.
     */
    struct NoiseRuntime
    {
        int voiceIndex = -1;
        int color = SbgVoice::Pink;

        quint32 randomState[2];
        double pinkState[2][7];
        double brownState[2];
        double greyLow[2];
        double greyMid[2];
        double greyHigh[2];
        double whiteFiltered[2];
    };

    struct SetRuntime
    {
        QVector<ToneRuntime> tones;
        QVector<NoiseRuntime> noises;
    };

    void buildRuntime();
    void resetNoiseState(NoiseRuntime &noise);
    double nextNoiseSample(NoiseRuntime &noise, int channel);
    void renderSetFrame(int setIndex, double *outLeft, double *outRight);

    SbgSequence m_sequence;

    // Parallel to m_sequence.events; -1 = silence ('off').
    QVector<int> m_eventSetIndex;

    // Parallel to m_sequence.toneSets.
    QVector<SetRuntime> m_setRuntime;

    double m_sampleRate;
    qint64 m_frames;      // frames rendered so far
    int m_segment;        // current timeline segment index

    qint64 m_durationMs;
    bool m_openEnded;
};

#endif
