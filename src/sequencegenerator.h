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
 * - Tone-set changes blend per voice, as in SBaGen: voices that
 *   continue from one tone-set to the next (same noise color, or
 *   same carrier/offset/isochronic parameters) keep playing
 *   seamlessly, with only their amplitude gliding if it changes.
 *   Ending voices fade out and new voices fade in with
 *   equal-power weights over the last 10 s of each segment,
 *   avoiding the ~3 dB level dip a linear crossfade produces for
 *   uncorrelated signals.
 *
 * - On segment changes, continuing voices inherit the oscillator
 *   phase and noise filter state of the voice they continue, so
 *   the sound carries across the boundary without a restart.
 *
 * - '@' isochronic voices render as an amplitude-pulsed tone on
 *   both ears (duty cycle 50 %, click-free edges), matching the
 *   standalone IsochronicGenerator.
 *
 * - 'bell+freq/amp' voices render as a decaying pure sine at
 *   'freq', played identically into both ears, as in SBaGen:
 *   the bell is struck every time the timeline enters a
 *   tone-set containing it (phase reset, envelope back to full
 *   amplitude), and the envelope then loses about 1/12 of its
 *   value every 50 ms until it fades to silence.
 *
 * - A tone-set marked with a leading "wave:period[/depth]"
 *   keyword plays with a slow raised-cosine volume swell (the
 *   same envelope as the colored-noise wave modulation). Every
 *   voice carries its own swell phase; continuing voices inherit
 *   it across tone-set boundaries, so the swell never restarts
 *   mid-sequence. New voices start at the swell trough and rise
 *   from there.
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
        double pulsePhase = 0.0;   // isochronic voices only
        double wavePhase = 0.0;    // authored swell ('wave:') only
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

        double wavePhase = 0.0;    // authored swell ('wave:') only
    };

    /*
     * Per-bell strike state, one entry per 'bell+freq/amp'
     * voice of the tone-set. The envelope is kept in SBaGen
     * amplitude units and decays towards zero; the bell sounds
     * while it is above zero.
     */
    struct BellRuntime
    {
        int voiceIndex = -1;
        double phase = 0.0;        // sine phase, cycles
        double envelope = 0.0;     // SBaGen amplitude units
        int framesToDecay = 0;     // frames until next decay step
    };

    struct SetRuntime
    {
        QVector<ToneRuntime> tones;
        QVector<NoiseRuntime> noises;
        QVector<BellRuntime> bells;
    };

    void buildRuntime();

    /*
     * Continuation matching across the boundary between events
     * 'i' and 'i + 1', built once in buildRuntime():
     *
     * - m_voiceMatchToNext.at(i): per voice of tone-set(i), the
     *   index of the matching voice in tone-set(i + 1), or -1.
     * - m_voiceMatchToEnd.at(i): per voice of tone-set(i + 1),
     *   the index of the matching voice in tone-set(i), or -1.
     */
    QVector<QVector<int>> m_voiceMatchToNext;
    QVector<QVector<int>> m_voiceMatchToEnd;

    void carryOverRuntime(int eventIndex);

    /*
     * Strikes every bell voice of the tone-set of event
     * 'eventIndex' (phase reset, envelope back to full
     * amplitude), as SBaGen does on entering a period.
     */
    void triggerBells(int eventIndex);

    ToneRuntime *findToneRuntime(SetRuntime &runtime,
                                 int voiceIndex);

    NoiseRuntime *findNoiseRuntime(SetRuntime &runtime,
                                   int voiceIndex);

    void renderSegmentFrame(double *outLeft,
                            double *outRight,
                            bool fading,
                            double fadePos,
                            double outWeight,
                            double inWeight);

    /*
     * Raised-cosine volume swell for 'wave:' tone-sets (range
     * [1 - depth .. 1]); returns 1.0 and leaves the phase alone
     * when the swell is disabled. Advances the phase by one
     * frame per call.
     */
    double waveEnvelope(double depth,
                        double periodSeconds,
                        double &phase);

    void renderToneVoice(const SbgVoice &voice,
                         ToneRuntime &tone,
                         double gain,
                         double weight,
                         double *outLeft,
                         double *outRight);

    void renderNoiseVoice(const SbgVoice &voice,
                          NoiseRuntime &noise,
                          double gain,
                          double weight,
                          double *outLeft,
                          double *outRight);

    void resetNoiseState(NoiseRuntime &noise);
    double nextNoiseSample(NoiseRuntime &noise, int channel);

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
