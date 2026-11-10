#ifndef SOLFEGGIOGENERATOR_H
#define SOLFEGGIOGENERATOR_H

#include <QIODevice>
#include <QVector>

/*
 * Solfeggio engine: pure sine tones at one or more of the
 * nine traditional solfeggio carrier frequencies
 * (174, 285, 396, 417, 528, 639, 741, 852, 963 Hz).
 *
 * Layered tones are summed with equal weight; the per-tone
 * amplitude is scaled by the active tone count so the mix
 * never clips.
 */

class SolfeggioGenerator : public QIODevice
{
public:
    explicit SolfeggioGenerator(QObject *parent = nullptr);

    void start();

    void setFrequencies(const QVector<double> &frequencies);
    void setSampleRate(int sampleRate);

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override;

private:
    double m_sampleRate;

    QVector<double> m_frequencies;
    QVector<double> m_phases;
};

#endif
