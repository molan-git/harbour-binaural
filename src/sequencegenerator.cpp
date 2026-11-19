#include "sequencegenerator.h"

#include <QtMath>
#include <cstring>

namespace
{
    const int DefaultSampleRate = 48000;

    const double TwoPi = 2.0 * M_PI;

    // SBaGen '-SE' spin: crossfade length between tone-sets.
    const qint64 SpinDurationMs = 10000;

    // Grace period of silence after the final fade before the
    // generator reports finished().
    const qint64 FinishPadMs = 1000;

    /*
     * SBaGen amplitude units -> int16 sample units.
     *
     * Typical presets use amplitudes 5..40 per voice; with several
     * voices summing, this lands in the same loudness region as
     * the existing BinauralGenerator (13000) and
     * ColoredNoiseGenerator (16000) masters. Single tuning point.
     */
    const double AmplitudeScale = 400.0;

    // Final limiter.
    const double PeakSample = 32000.0;
}

// ============================================================================
// LIFECYCLE
// ============================================================================

SequenceGenerator::SequenceGenerator(const SbgSequence &sequence,
                                     QObject *parent)
    : QIODevice(parent),
      m_sequence(sequence),
      m_sampleRate(DefaultSampleRate),
      m_frames(0),
      m_segment(0),
      m_durationMs(0),
      m_openEnded(false)
{
    buildRuntime();

    if (!m_sequence.events.isEmpty())
    {
        m_durationMs = m_sequence.events.last().timeMs;

        // Ends silent ('off') -> the sequence finishes by itself.
        // Ends active -> keep playing the last tone-set.
        m_openEnded =
            m_eventSetIndex.last() >= 0;
    }
}

void SequenceGenerator::setSampleRate(int sampleRate)
{
    if (sampleRate > 0)
        m_sampleRate = sampleRate;
}

void SequenceGenerator::start()
{
    open(QIODevice::ReadOnly);
}

// ============================================================================
// STATE
// ============================================================================

qint64 SequenceGenerator::elapsedMs() const
{
    if (m_sampleRate <= 0)
        return 0;

    return qRound(
        static_cast<double>(m_frames) * 1000.0
        / m_sampleRate);
}

qint64 SequenceGenerator::durationMs() const
{
    return m_durationMs;
}

bool SequenceGenerator::finished() const
{
    if (m_openEnded)
        return false;

    return elapsedMs() >= m_durationMs + FinishPadMs;
}

// ============================================================================
// RENDER
// ============================================================================

qint64 SequenceGenerator::readData(char *data, qint64 maxlen)
{
    const qint64 bytesPerFrame = 4;   // stereo, 16 bit

    const qint64 frameCount = maxlen / bytesPerFrame;

    qint16 *samples = reinterpret_cast<qint16 *>(data);

    const int eventCount = m_sequence.events.size();

    const double frameMs = 1000.0 / m_sampleRate;

    double tMs = static_cast<double>(m_frames) * frameMs;

    for (qint64 frame = 0; frame < frameCount; ++frame)
    {
        // Advance the current segment across event boundaries.
        while (m_segment + 1 < eventCount
               && tMs >= static_cast<double>(
                            m_sequence.events.at(m_segment + 1)
                                .timeMs))
        {
            ++m_segment;
        }

        /*
         * Crossfade weight of the current tone-set. During the
         * last SpinDurationMs of a segment the next tone-set
         * blends in; before that the weight is exactly 1 and the
         * next set is not rendered at all.
         */
        double weightCurrent = 1.0;

        if (m_segment + 1 < eventCount)
        {
            const qint64 boundaryMs =
                m_sequence.events.at(m_segment + 1).timeMs;

            const qint64 segmentStartMs =
                m_sequence.events.at(m_segment).timeMs;

            const qint64 fadeStartMs =
                qMax(segmentStartMs,
                     boundaryMs - SpinDurationMs);

            if (tMs > static_cast<double>(fadeStartMs)
                && boundaryMs > fadeStartMs)
            {
                weightCurrent =
                    static_cast<double>(boundaryMs - tMs)
                    / static_cast<double>(
                        boundaryMs - fadeStartMs);

                if (weightCurrent < 0.0)
                    weightCurrent = 0.0;
            }
        }

        double left = 0.0;
        double right = 0.0;

        renderSetFrame(m_eventSetIndex.at(m_segment),
                       &left, &right);

        if (weightCurrent < 1.0)
        {
            double nextLeft = 0.0;
            double nextRight = 0.0;

            renderSetFrame(m_eventSetIndex.at(m_segment + 1),
                           &nextLeft, &nextRight);

            const double weightNext = 1.0 - weightCurrent;

            left = left * weightCurrent
                + nextLeft * weightNext;

            right = right * weightCurrent
                + nextRight * weightNext;
        }

        samples[frame * 2] =
            static_cast<qint16>(
                qBound(-PeakSample, left, PeakSample));

        samples[frame * 2 + 1] =
            static_cast<qint16>(
                qBound(-PeakSample, right, PeakSample));

        ++m_frames;
        tMs += frameMs;
    }

    const qint64 usedBytes = frameCount * bytesPerFrame;

    if (usedBytes < maxlen)
    {
        std::memset(data + usedBytes,
                    0,
                    static_cast<size_t>(maxlen - usedBytes));
    }

    return maxlen;
}

qint64 SequenceGenerator::writeData(const char *, qint64)
{
    return 0;
}

// ============================================================================
// RUNTIME SETUP
// ============================================================================

void SequenceGenerator::buildRuntime()
{
    m_eventSetIndex.clear();
    m_setRuntime.clear();

    // Resolve event tone-set names to indices once.
    for (int i = 0; i < m_sequence.events.size(); ++i)
    {
        const QString &name =
            m_sequence.events.at(i).toneSetName;

        int index = -1;

        if (name != QLatin1String("-"))
        {
            for (int s = 0; s < m_sequence.toneSets.size(); ++s)
            {
                if (m_sequence.toneSets.at(s).name == name)
                {
                    index = s;
                    break;
                }
            }
        }

        m_eventSetIndex.append(index);
    }

    // Oscillator and noise state per tone-set voice.
    for (int s = 0; s < m_sequence.toneSets.size(); ++s)
    {
        const SbgToneSet &set = m_sequence.toneSets.at(s);

        SetRuntime runtime;

        for (int v = 0; v < set.voices.size(); ++v)
        {
            const SbgVoice &voice = set.voices.at(v);

            if (voice.type == SbgVoice::Tone)
            {
                ToneRuntime tone;
                tone.voiceIndex = v;
                tone.leftPhase = 0.0;
                tone.rightPhase = 0.0;

                runtime.tones.append(tone);
            }
            else
            {
                NoiseRuntime noise;
                noise.voiceIndex = v;
                noise.color = voice.noiseColor;

                resetNoiseState(noise);

                runtime.noises.append(noise);
            }
        }

        m_setRuntime.append(runtime);
    }
}

// ============================================================================
// PER-SET RENDERING
// ============================================================================

void SequenceGenerator::renderSetFrame(int setIndex,
                                       double *outLeft,
                                       double *outRight)
{
    double left = 0.0;
    double right = 0.0;

    if (setIndex >= 0)
    {
        const SbgToneSet &set = m_sequence.toneSets.at(setIndex);

        SetRuntime &runtime = m_setRuntime[setIndex];

        for (int i = 0; i < runtime.tones.size(); ++i)
        {
            ToneRuntime &tone = runtime.tones[i];

            const SbgVoice &voice =
                set.voices.at(tone.voiceIndex);

            const double gain =
                voice.amplitude * AmplitudeScale;

            left += qSin(tone.leftPhase * TwoPi) * gain;
            right += qSin(tone.rightPhase * TwoPi) * gain;

            tone.leftPhase +=
                voice.carrier / m_sampleRate;

            tone.rightPhase +=
                (voice.carrier + voice.offset) / m_sampleRate;

            if (tone.leftPhase >= 1.0)
                tone.leftPhase -= 1.0;

            if (tone.rightPhase >= 1.0)
                tone.rightPhase -= 1.0;
        }

        for (int i = 0; i < runtime.noises.size(); ++i)
        {
            NoiseRuntime &noise = runtime.noises[i];

            const SbgVoice &voice =
                set.voices.at(noise.voiceIndex);

            const double gain =
                voice.amplitude * AmplitudeScale;

            left += nextNoiseSample(noise, 0) * gain;
            right += nextNoiseSample(noise, 1) * gain;
        }
    }

    *outLeft = left;
    *outRight = right;
}

// ============================================================================
// NOISE DSP
// ============================================================================
//
// Mirrors ColoredNoiseGenerator so that sequence noise voices match
// the sound of the standalone colored-noise channel. If the two
// implementations ever need to diverge, extract a shared
// NoiseSource class and use it from both generators.

void SequenceGenerator::resetNoiseState(NoiseRuntime &noise)
{
    for (int channel = 0; channel < 2; ++channel)
    {
        noise.randomState[channel] =
            0x12345678u + channel * 0x13579BDFu;

        for (int i = 0; i < 7; ++i)
            noise.pinkState[channel][i] = 0.0;

        noise.brownState[channel] = 0.0;

        noise.greyLow[channel] = 0.0;
        noise.greyMid[channel] = 0.0;
        noise.greyHigh[channel] = 0.0;

        noise.whiteFiltered[channel] = 0.0;
    }
}

double SequenceGenerator::nextNoiseSample(NoiseRuntime &noise,
                                          int channel)
{
    // Raw white source: 32-bit LCG.
    auto nextRawWhite = [&noise, channel]()
    {
        noise.randomState[channel] =
            1664525u * noise.randomState[channel]
            + 1013904223u;

        return (noise.randomState[channel]
                / 2147483648.0) - 1.0;
    };

    // Smoothed white.
    auto nextWhite = [&noise, channel, &nextRawWhite]()
    {
        const double white = nextRawWhite();

        noise.whiteFiltered[channel] =
            noise.whiteFiltered[channel] * 0.82
            + white * 0.18;

        return noise.whiteFiltered[channel] * 0.82;
    };

    double value = 0.0;

    switch (noise.color)
    {
    case SbgVoice::White:
        value = nextWhite();
        break;

    case SbgVoice::Pink:
    {
        const double white = nextWhite();

        double *state = noise.pinkState[channel];

        state[0] = 0.99886 * state[0] + white * 0.0555179;
        state[1] = 0.99332 * state[1] + white * 0.0750759;
        state[2] = 0.96900 * state[2] + white * 0.1538520;
        state[3] = 0.86650 * state[3] + white * 0.3104856;
        state[4] = 0.55000 * state[4] + white * 0.5329522;
        state[5] = -0.7616 * state[5] - white * 0.0168980;

        const double pink =
            state[0] + state[1] + state[2] + state[3]
            + state[4] + state[5] + state[6]
            + white * 0.5362;

        state[6] = white * 0.115926;

        value = pink * 0.11;
        break;
    }

    case SbgVoice::Brown:
    {
        const double white = nextRawWhite();

        double &state = noise.brownState[channel];

        state = state * 0.995 + white * 0.018;

        state = qBound(-1.0, state, 1.0);

        value = state * 2.8;
        break;
    }

    case SbgVoice::Grey:
    {
        // Reuse the pink branch via a nested call.
        const int savedColor = noise.color;
        noise.color = SbgVoice::Pink;
        const double pink = nextNoiseSample(noise, channel);
        noise.color = savedColor;

        double &low = noise.greyLow[channel];
        double &mid = noise.greyMid[channel];
        double &high = noise.greyHigh[channel];

        low = low * 0.985 + pink * 0.015;
        mid = mid * 0.80 + pink * 0.20;
        high = high * 0.45 + pink * 0.55;

        const double grey =
            low * 0.25 + mid * 0.60 + high * 0.15;

        value = grey * 2.4;
        break;
    }
    }

    return qBound(-1.0, value, 1.0);
}
