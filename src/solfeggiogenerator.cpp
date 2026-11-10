#include "solfeggiogenerator.h"

#include <QtMath>
#include <cstring>

namespace
{
    const int DefaultSampleRate = 48000;

    // MASTER VOLUME Solfeggio (per-tone amplitude is this
    // value divided by the active tone count).
    const double SolfeggioAmplitude = 11000.0;
}

// ============================================================================
// SOLFEGGIO GENERATOR
// ============================================================================

SolfeggioGenerator::SolfeggioGenerator(QObject *parent)
    : QIODevice(parent),
      m_sampleRate(DefaultSampleRate)
{
}

void SolfeggioGenerator::start()
{
    open(QIODevice::ReadOnly);
}

void SolfeggioGenerator::setFrequencies(
    const QVector<double> &frequencies)
{
    m_frequencies = frequencies;
    m_phases.clear();
    m_phases.resize(frequencies.size());
}

void SolfeggioGenerator::setSampleRate(int sampleRate)
{
    if (sampleRate > 0)
        m_sampleRate = sampleRate;
}

qint64 SolfeggioGenerator::readData(char *data,
                                    qint64 maxlen)
{
    const qint64 bytesPerFrame = 4; // stereo, 16 bit
    const qint64 frameCount = maxlen / bytesPerFrame;

    qint16 *samples =
        reinterpret_cast<qint16 *>(data);

    const int toneCount = m_frequencies.size();
    const double toneAmplitude =
        toneCount > 0
            ? SolfeggioAmplitude / toneCount
            : 0.0;

    for (qint64 frame = 0; frame < frameCount; ++frame)
    {
        double mixed = 0.0;

        for (int tone = 0; tone < toneCount; ++tone)
        {
            mixed += qSin(m_phases[tone] * 2.0 * M_PI)
                     * toneAmplitude;

            m_phases[tone] +=
                m_frequencies[tone] / m_sampleRate;

            if (m_phases[tone] >= 1.0)
                m_phases[tone] -= 1.0;
        }

        const qint16 value =
            static_cast<qint16>(mixed);

        samples[frame * 2] = value;
        samples[frame * 2 + 1] = value;
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

qint64 SolfeggioGenerator::writeData(const char *, qint64)
{
    return 0;
}
