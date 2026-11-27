#include "sequencegenerator.h"

#include <QtMath>
#include <cstring>

namespace
{
    const int DefaultSampleRate = 48000;

    const double TwoPi = 2.0 * M_PI;

    // Crossfade length between tone-sets (SBaGen '-SE' decay).
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

    // Isochronic pulse envelope (matches IsochronicGenerator).
    const double IsochronicDutyCycle = 0.5;
    const double PulseRampSeconds = 0.003;

    /*
     * Two voices "match" across consecutive tone-sets when they
     * are the same kind of sound with identical parameters: such
     * voices keep playing seamlessly across the transition
     * (mirror of the helper in sbgsequence.cpp).
     */
    bool voicesMatch(const SbgVoice &a, const SbgVoice &b)
    {
        if (a.type != b.type)
            return false;

        if (a.type == SbgVoice::Noise)
            return a.noiseColor == b.noiseColor;

        return a.isochronic == b.isochronic
            && a.carrier == b.carrier
            && a.offset == b.offset;
    }

    /*
     * Amplitude envelope of an isochronic pulse: on for the first
     * IsochronicDutyCycle of the cycle, with linear ramps at both
     * edges so the switching stays click-free (at high pulse
     * rates the ramps are capped to 40 % of the on-time).
     */
    double isochronicEnvelope(double pulsePhase,
                              double pulseFrequency,
                              int sampleRate)
    {
        if (pulseFrequency <= 0.0)
            return 0.0;

        const double cycleFrames =
            sampleRate / pulseFrequency;

        const double onFrames =
            cycleFrames * IsochronicDutyCycle;

        const double rampFrames = qMin(
            onFrames * 0.4,
            PulseRampSeconds * sampleRate);

        const double pos = pulsePhase * cycleFrames;

        if (pos >= onFrames)
            return 0.0;

        if (rampFrames <= 0.0)
            return 1.0;

        return qBound(
            0.0,
            qMin(pos / rampFrames,
                 (onFrames - pos) / rampFrames),
            1.0);
    }
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
    // The timeline starts by entering the first tone-set: any
    // bells in it are struck immediately.
    triggerBells(m_segment);

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
        // Continuing voices carry their runtime state over.
        while (m_segment + 1 < eventCount
               && tMs >= static_cast<double>(
                            m_sequence.events.at(m_segment + 1)
                                .timeMs))
        {
            carryOverRuntime(m_segment);
            ++m_segment;

            // Entering a period strikes the bells of the
            // new tone-set (as in SBaGen).
            triggerBells(m_segment);
        }

        /*
         * Fade progress towards the next boundary: 0 outside the
         * fade window, 1 at the boundary itself. During the
         * window, ending voices fade out and new voices fade in
         * with equal-power weights.
         */
        double fadePos = 0.0;

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
                fadePos =
                    (tMs - static_cast<double>(fadeStartMs))
                    / static_cast<double>(
                        boundaryMs - fadeStartMs);

                if (fadePos < 0.0)
                    fadePos = 0.0;

                if (fadePos > 1.0)
                    fadePos = 1.0;
            }
        }

        const bool fading = fadePos > 0.0;

        const double outWeight = qCos(fadePos * M_PI / 2.0);
        const double inWeight = qSin(fadePos * M_PI / 2.0);

        double left = 0.0;
        double right = 0.0;

        renderSegmentFrame(&left, &right,
                           fading, fadePos,
                           outWeight, inWeight);

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
    m_voiceMatchToNext.clear();
    m_voiceMatchToEnd.clear();

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
                tone.pulsePhase = 0.0;
                tone.wavePhase = 0.0;

                runtime.tones.append(tone);
            }
            else if (voice.type == SbgVoice::Noise)
            {
                NoiseRuntime noise;
                noise.voiceIndex = v;
                noise.color = voice.noiseColor;
                noise.wavePhase = 0.0;

                resetNoiseState(noise);

                runtime.noises.append(noise);
            }
            else
            {
                BellRuntime bell;
                bell.voiceIndex = v;

                runtime.bells.append(bell);
            }
        }

        m_setRuntime.append(runtime);
    }

    // Continuation matching across each event boundary.
    for (int i = 0; i < m_sequence.events.size(); ++i)
    {
        if (i + 1 >= m_sequence.events.size())
        {
            // No next event: keep the tables parallel to the
            // event list.
            m_voiceMatchToNext.append(QVector<int>());
            m_voiceMatchToEnd.append(QVector<int>());
            continue;
        }

        const int curIdx = m_eventSetIndex.at(i);
        const int nextIdx = m_eventSetIndex.at(i + 1);

        const int curCount =
            curIdx >= 0
                ? m_sequence.toneSets.at(curIdx).voices.size()
                : 0;

        const int nextCount =
            nextIdx >= 0
                ? m_sequence.toneSets.at(nextIdx).voices.size()
                : 0;

        QVector<int> toNext(curCount, -1);
        QVector<int> toCur(nextCount, -1);

        if (curIdx >= 0 && nextIdx >= 0)
        {
            const SbgToneSet &cur =
                m_sequence.toneSets.at(curIdx);

            const SbgToneSet &next =
                m_sequence.toneSets.at(nextIdx);

            if (curIdx == nextIdx)
            {
                /*
                 * The same tone-set is scheduled again: every
                 * voice continues seamlessly (an '==' re-mark).
                 */
                for (int j = 0; j < cur.voices.size(); ++j)
                    toNext[j] = j;

                for (int k = 0; k < next.voices.size(); ++k)
                    toCur[k] = k;
            }
            else
            {
                for (int j = 0; j < cur.voices.size(); ++j)
                {
                    for (int k = 0; k < next.voices.size(); ++k)
                    {
                        if (voicesMatch(cur.voices.at(j),
                                        next.voices.at(k)))
                        {
                            toNext[j] = k;
                            break;
                        }
                    }
                }

                for (int k = 0; k < next.voices.size(); ++k)
                {
                    for (int j = 0; j < cur.voices.size(); ++j)
                    {
                        if (voicesMatch(cur.voices.at(j),
                                        next.voices.at(k)))
                        {
                            toCur[k] = j;
                            break;
                        }
                    }
                }
            }
        }

        m_voiceMatchToNext.append(toNext);
        m_voiceMatchToEnd.append(toCur);
    }
}

void SequenceGenerator::triggerBells(int eventIndex)
{
    const int setIdx = m_eventSetIndex.at(eventIndex);

    if (setIdx < 0)
        return;

    const SbgToneSet &set = m_sequence.toneSets.at(setIdx);

    SetRuntime &runtime = m_setRuntime[setIdx];

    for (int i = 0; i < runtime.bells.size(); ++i)
    {
        BellRuntime &bell = runtime.bells[i];

        const SbgVoice &voice =
            set.voices.at(bell.voiceIndex);

        bell.phase = 0.0;
        bell.envelope = voice.amplitude;
        bell.framesToDecay =
            static_cast<int>(m_sampleRate / 20.0);
    }
}

void SequenceGenerator::carryOverRuntime(int eventIndex)
{
    if (eventIndex + 1 >= m_eventSetIndex.size())
        return;

    const int curIdx = m_eventSetIndex.at(eventIndex);
    const int nextIdx = m_eventSetIndex.at(eventIndex + 1);

    if (curIdx < 0 || nextIdx < 0 || curIdx == nextIdx)
        return;

    const QVector<int> &toCur =
        m_voiceMatchToEnd.at(eventIndex);

    SetRuntime &curRuntime = m_setRuntime[curIdx];
    SetRuntime &nextRuntime = m_setRuntime[nextIdx];

    // Continuing voices inherit the phase and filter state of
    // the voice they continue, so the sound carries over the
    // boundary without a restart.
    for (int j = 0; j < toCur.size(); ++j)
    {
        const int fromVoice = toCur.at(j);

        if (fromVoice < 0)
            continue;

        if (ToneRuntime *dst =
                findToneRuntime(nextRuntime, j))
        {
            if (ToneRuntime *src =
                    findToneRuntime(curRuntime, fromVoice))
                *dst = *src;
        }

        if (NoiseRuntime *dst =
                findNoiseRuntime(nextRuntime, j))
        {
            if (NoiseRuntime *src =
                    findNoiseRuntime(curRuntime, fromVoice))
                *dst = *src;
        }
    }
}

double SequenceGenerator::waveEnvelope(double depth,
                                       double periodSeconds,
                                       double &phase)
{
    if (depth <= 0.0 || periodSeconds <= 0.0)
        return 1.0;

    /*
     * Raised-cosine swell over the whole frame, matching the
     * colored-noise wave modulation. Range [1 - depth .. 1];
     * phase 0 is the trough, so fresh voices start quiet and
     * swell up from there.
     */
    const double envelope =
        (1.0 - depth)
        + depth * 0.5
            * (1.0 - qCos(phase * TwoPi));

    phase += 1.0 / (periodSeconds * m_sampleRate);

    if (phase >= 1.0)
        phase -= 1.0;

    return envelope;
}

SequenceGenerator::ToneRuntime *
SequenceGenerator::findToneRuntime(SetRuntime &runtime,
                                   int voiceIndex)
{
    for (int i = 0; i < runtime.tones.size(); ++i)
    {
        if (runtime.tones.at(i).voiceIndex == voiceIndex)
            return &runtime.tones[i];
    }

    return nullptr;
}

SequenceGenerator::NoiseRuntime *
SequenceGenerator::findNoiseRuntime(SetRuntime &runtime,
                                    int voiceIndex)
{
    for (int i = 0; i < runtime.noises.size(); ++i)
    {
        if (runtime.noises.at(i).voiceIndex == voiceIndex)
            return &runtime.noises[i];
    }

    return nullptr;
}

// ============================================================================
// PER-SEGMENT RENDERING
// ============================================================================

void SequenceGenerator::renderSegmentFrame(double *outLeft,
                                           double *outRight,
                                           bool fading,
                                           double fadePos,
                                           double outWeight,
                                           double inWeight)
{
    double left = 0.0;
    double right = 0.0;

    const int eventCount = m_sequence.events.size();

    const int curSetIdx = m_eventSetIndex.at(m_segment);

    const int nextSetIdx =
        m_segment + 1 < eventCount
            ? m_eventSetIndex.at(m_segment + 1)
            : -1;

    // ----------------------------------------------------------
    // Current tone-set.
    // ----------------------------------------------------------

    if (curSetIdx >= 0)
    {
        const SbgToneSet &set =
            m_sequence.toneSets.at(curSetIdx);

        SetRuntime &runtime = m_setRuntime[curSetIdx];

        const QVector<int> &toNext =
            m_voiceMatchToNext.at(m_segment);

        for (int i = 0; i < runtime.tones.size(); ++i)
        {
            ToneRuntime &tone = runtime.tones[i];

            const SbgVoice &voice =
                set.voices.at(tone.voiceIndex);

            double amplitude = voice.amplitude;
            double weight = 1.0;

            if (fading)
            {
                const int match =
                    toNext.at(tone.voiceIndex);

                if (match < 0)
                {
                    // Voice ends here: fade it out.
                    weight = outWeight;
                }
                else if (nextSetIdx >= 0 && fadePos > 0.0)
                {
                    // Voice continues: glide its amplitude
                    // towards the next tone-set's value.
                    const SbgVoice &nextVoice =
                        m_sequence.toneSets.at(nextSetIdx)
                            .voices.at(match);

                    if (nextVoice.amplitude
                        != voice.amplitude)
                    {
                        amplitude +=
                            (nextVoice.amplitude
                             - voice.amplitude) * fadePos;
                    }
                }
            }

            const double swell = waveEnvelope(
                set.waveDepth,
                set.wavePeriodSeconds,
                runtime.tones[i].wavePhase);

            renderToneVoice(voice, tone,
                            amplitude * AmplitudeScale
                                * swell,
                            weight,
                            &left, &right);
        }

        for (int i = 0; i < runtime.noises.size(); ++i)
        {
            NoiseRuntime &noise = runtime.noises[i];

            const SbgVoice &voice =
                set.voices.at(noise.voiceIndex);

            double amplitude = voice.amplitude;
            double weight = 1.0;

            if (fading)
            {
                const int match =
                    toNext.at(noise.voiceIndex);

                if (match < 0)
                {
                    weight = outWeight;
                }
                else if (nextSetIdx >= 0 && fadePos > 0.0)
                {
                    const SbgVoice &nextVoice =
                        m_sequence.toneSets.at(nextSetIdx)
                            .voices.at(match);

                    if (nextVoice.amplitude
                        != voice.amplitude)
                    {
                        amplitude +=
                            (nextVoice.amplitude
                             - voice.amplitude) * fadePos;
                    }
                }
            }

            const double swell = waveEnvelope(
                set.waveDepth,
                set.wavePeriodSeconds,
                runtime.noises[i].wavePhase);

            renderNoiseVoice(voice, noise,
                             amplitude * AmplitudeScale
                                 * swell,
                             weight,
                             &left, &right);
        }

        /*
         * Bell voices: a decaying pure sine, the same signal
         * into both ears (as in SBaGen). Bells do not take
         * part in the crossfade blending -- they are struck
         * at the boundary itself and ring out independently.
         */
        for (int i = 0; i < runtime.bells.size(); ++i)
        {
            BellRuntime &bell = runtime.bells[i];

            if (bell.envelope <= 0.0)
                continue;

            const SbgVoice &voice =
                set.voices.at(bell.voiceIndex);

            const double value =
                qSin(bell.phase * TwoPi)
                * bell.envelope * AmplitudeScale;

            left += value;
            right += value;

            bell.phase += voice.carrier / m_sampleRate;

            if (bell.phase >= 1.0)
                bell.phase -= 1.0;

            if (--bell.framesToDecay <= 0)
            {
                /*
                 * SBaGen decay: every 50 ms the envelope
                 * loses 1/12 of its value plus one unit,
                 * which rings a bell of amplitude 20 for
                 * roughly a second.
                 */
                bell.framesToDecay =
                    static_cast<int>(m_sampleRate / 20.0);

                bell.envelope =
                    bell.envelope * (11.0 / 12.0) - 1.0;

                if (bell.envelope < 0.0)
                    bell.envelope = 0.0;
            }
        }
    }

    // ----------------------------------------------------------
    // Voices of the next tone-set that are new: fade them in.
    // (Continuing voices are already sounding from above.)
    // ----------------------------------------------------------

    if (fading && fadePos < 1.0 && nextSetIdx >= 0
        && nextSetIdx != curSetIdx)
    {
        const SbgToneSet &nextSet =
            m_sequence.toneSets.at(nextSetIdx);

        SetRuntime &nextRuntime = m_setRuntime[nextSetIdx];

        const QVector<int> &toCur =
            m_voiceMatchToEnd.at(m_segment);

        for (int i = 0; i < nextRuntime.tones.size(); ++i)
        {
            ToneRuntime &tone = nextRuntime.tones[i];

            if (toCur.at(tone.voiceIndex) >= 0)
                continue;

            const SbgVoice &voice =
                nextSet.voices.at(tone.voiceIndex);

            const double swell = waveEnvelope(
                nextSet.waveDepth,
                nextSet.wavePeriodSeconds,
                nextRuntime.tones[i].wavePhase);

            renderToneVoice(voice, tone,
                            voice.amplitude * AmplitudeScale
                                * swell,
                            inWeight,
                            &left, &right);
        }

        for (int i = 0; i < nextRuntime.noises.size(); ++i)
        {
            NoiseRuntime &noise = nextRuntime.noises[i];

            if (toCur.at(noise.voiceIndex) >= 0)
                continue;

            const SbgVoice &voice =
                nextSet.voices.at(noise.voiceIndex);

            const double swell = waveEnvelope(
                nextSet.waveDepth,
                nextSet.wavePeriodSeconds,
                nextRuntime.noises[i].wavePhase);

            renderNoiseVoice(voice, noise,
                             voice.amplitude * AmplitudeScale
                                 * swell,
                             inWeight,
                             &left, &right);
        }
    }

    *outLeft = left;
    *outRight = right;
}

void SequenceGenerator::renderToneVoice(const SbgVoice &voice,
                                        ToneRuntime &tone,
                                        double gain,
                                        double weight,
                                        double *outLeft,
                                        double *outRight)
{
    if (weight <= 0.0)
        return;

    if (voice.isochronic)
    {
        // Amplitude-pulsed tone on both ears, matching the
        // standalone IsochronicGenerator.
        const double envelope =
            isochronicEnvelope(tone.pulsePhase,
                               voice.offset,
                               static_cast<int>(m_sampleRate));

        const double value =
            qSin(tone.leftPhase * TwoPi)
            * gain * envelope * weight;

        *outLeft += value;
        *outRight += value;

        tone.leftPhase +=
            voice.carrier / m_sampleRate;

        tone.pulsePhase +=
            voice.offset / m_sampleRate;

        if (tone.leftPhase >= 1.0)
            tone.leftPhase -= 1.0;

        if (tone.pulsePhase >= 1.0)
            tone.pulsePhase -= 1.0;

        return;
    }

    *outLeft +=
        qSin(tone.leftPhase * TwoPi)
        * gain * weight;

    *outRight +=
        qSin(tone.rightPhase * TwoPi)
        * gain * weight;

    tone.leftPhase +=
        voice.carrier / m_sampleRate;

    tone.rightPhase +=
        (voice.carrier + voice.offset) / m_sampleRate;

    if (tone.leftPhase >= 1.0)
        tone.leftPhase -= 1.0;

    if (tone.rightPhase >= 1.0)
        tone.rightPhase -= 1.0;
}

void SequenceGenerator::renderNoiseVoice(const SbgVoice &voice,
                                         NoiseRuntime &noise,
                                         double gain,
                                         double weight,
                                         double *outLeft,
                                         double *outRight)
{
    Q_UNUSED(voice);

    if (weight <= 0.0)
        return;

    *outLeft += nextNoiseSample(noise, 0) * gain * weight;
    *outRight += nextNoiseSample(noise, 1) * gain * weight;
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
    auto nextRawWhite = [&noise, channel]() -> double
    {
        noise.randomState[channel] =
            1664525u * noise.randomState[channel]
            + 1013904223u;

        return (noise.randomState[channel]
                / 2147483648.0) - 1.0;
    };

    // Smoothed white.
    auto nextWhite = [&noise, channel, &nextRawWhite]()
        -> double
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
