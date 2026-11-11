#include "isochronicgenerator.h"

#include <QtMath>
#include <cstring>

namespace
{
    const int DefaultSampleRate = 48000;

    const double DefaultCarrierFrequency = 200.0;
    const double DefaultPulseFrequency = 10.0;

    // MASTER VOLUME Isochronic (same level as the solfeggio
    // tones; the channel volume is scaled down by the engine).
    const double IsochronicAmplitude = 11000.0;

    // Fraction of the pulse cycle the tone is on (50 %).
    const double IsochronicDutyCycle = 0.5;

    // Click-free pulse edges: linear ramps of up to 3 ms.
    const double PulseRampSeconds = 0.003;
}

// ============================================================================
// ISOCHRONIC GENERATOR
// ============================================================================

IsochronicGenerator::IsochronicGenerator(QObject *parent)
    : QIODevice(parent),
      m_sampleRate(DefaultSampleRate),
      m_carrierFrequency(DefaultCarrierFrequency),
      m_pulseFrequency(DefaultPulseFrequency),
      m_carrierPhase(0.0),
      m_pulsePhase(0.0)
{
}

void IsochronicGenerator::start()
{
    open(QIODevice::ReadOnly);
}

void IsochronicGenerator::setCarrierFrequency(double frequency)
{
    // Keep the carrier in a musically and physiologically
    // sensible range (same bounds as the binaural carrier).
    m_carrierFrequency = qBound(20.0, frequency, 1000.0);
}

void IsochronicGenerator::setPulseFrequency(double frequency)
{
    m_pulseFrequency = qBound(0.5, frequency, 40.0);
}

void IsochronicGenerator::setSampleRate(int sampleRate)
{
    if (sampleRate > 0)
        m_sampleRate = sampleRate;
}

qint64 IsochronicGenerator::readData(char *data,
                                    qint64 maxlen)
{
    const qint64 bytesPerFrame = 4; // stereo, 16 bit
    const qint64 frameCount = maxlen / bytesPerFrame;

    qint16 *samples =
        reinterpret_cast<qint16 *>(data);

    /*
     * Pulse envelope:
     *
     *   m_pulsePhase runs 0.0 .. 1.0 over one pulse cycle.
     *   The tone is on for the first IsochronicDutyCycle of
     *   the cycle, with linear ramps at both edges so the
     *   switching stays click-free (also at 40 Hz, where the
     *   ramps are capped to 40 % of the on-time).
     */
    const double pulseCycleFrames =
        m_pulseFrequency > 0.0
            ? m_sampleRate / m_pulseFrequency
            : 1.0;

    const double onFrames =
        pulseCycleFrames * IsochronicDutyCycle;

    const double rampFrames = qMin(
        onFrames * 0.4,
        PulseRampSeconds * m_sampleRate);

    for (qint64 frame = 0; frame < frameCount; ++frame)
    {
        double envelope = 0.0;

        if (m_pulsePhase < IsochronicDutyCycle)
        {
            // Position (in frames) inside the on window.
            const double pos =
                m_pulsePhase * pulseCycleFrames;

            envelope = qMax(
                0.0,
                qMin(1.0,
                     qMin(pos / rampFrames,
                          (onFrames - pos) / rampFrames)));
        }

        const double value =
            qSin(m_carrierPhase * 2.0 * M_PI)
            * IsochronicAmplitude
            * envelope;

        const qint16 sample =
            static_cast<qint16>(value);

        samples[frame * 2] = sample;
        samples[frame * 2 + 1] = sample;

        m_carrierPhase +=
            m_carrierFrequency / m_sampleRate;

        m_pulsePhase +=
            m_pulseFrequency / m_sampleRate;

        if (m_carrierPhase >= 1.0)
            m_carrierPhase -= 1.0;

        if (m_pulsePhase >= 1.0)
            m_pulsePhase -= 1.0;
    }

    const qint64 usedBytes =
        frameCount * bytesPerFrame;

    if (usedBytes < maxlen)
    {
        std::memset(
            data + usedBytes,
            0,
            static_cast<size_t>(maxlen - usedBytes));
    }

    return maxlen;
}

qint64 IsochronicGenerator::writeData(const char *, qint64)
{
    return 0;
}
