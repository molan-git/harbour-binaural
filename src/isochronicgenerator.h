#ifndef ISOCHRONICGENERATOR_H
#define ISOCHRONICGENERATOR_H

#include <QIODevice>

/*
 * Isochronic engine: a sine tone (carrier frequency) that is
 * switched on and off at a regular pulse frequency.
 *
 *   Carrier:  200 Hz  -> the pitch you hear
 *   Pulse:    7.83 Hz  -> 7.83 on/off cycles per second
 *
 * Independent of the binaural engine: the identical signal is
 * emitted on both channels (no left/right difference). The
 * binaural generator owns the beat offset; this generator owns
 * only pulsed carrier content.
 *
 * Each pulse has short linear ramps (~3 ms) at its edges so
 * the on/off switching does not produce audible clicks.
 */

class IsochronicGenerator : public QIODevice
{
public:
    explicit IsochronicGenerator(QObject *parent = nullptr);

    void start();

    void setCarrierFrequency(double frequency);
    void setPulseFrequency(double frequency);
    void setSampleRate(int sampleRate);

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override;

private:
    double m_sampleRate;
    double m_carrierFrequency;
    double m_pulseFrequency;

    double m_carrierPhase;
    double m_pulsePhase;
};

#endif
