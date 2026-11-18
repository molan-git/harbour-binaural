#ifndef SBGSEQUENCE_H
#define SBGSEQUENCE_H

#include <QString>
#include <QVector>

/*
 * Data model for a parsed SBaGen sequence (.sbg).
 *
 * Only the deterministic subset of the SBaGen format is
 * supported -- the subset that the well-known focus/relax/sleep
 * example sequences are built from:
 *
 *   # comment
 *   -SE                          (options line; sequence always crossfades)
 *   name: 100+4.5/8 pink/40      (tone-set definition, '-' = off;
 *                                 a leading 'spin:' is accepted and
 *                                 its modulation ignored)
 *   NOW name                     (timeline event at t=0)
 *   +00:05:00 name               (event after a relative offset)
 *   00:05:00 name ...            (clock-style time, treated as an
 *                                  offset from sequence start;
 *                                  trailing tokens such as '->'
 *                                  are ignored)
 *
 * Voice forms: carrier+beat/amp, carrier-beat/amp, carrier@beat/amp
 * (sbagen-plus isochronic, approximated as a binaural '+beat'
 * voice), carrier/amp (plain tone), color/amp (noise voice).
 * Amplitude defaults to 20 when omitted.
 *
 * Pure data only: runtime state (oscillator phases, noise filter
 * state) lives in SequenceGenerator, so a parsed sequence can be
 * reused and inspected freely.
 */

struct SbgVoice
{
    enum Type { Tone = 0, Noise = 1 };
    enum NoiseColor { White = 0, Pink = 1, Brown = 2, Grey = 3 };

    Type type = Tone;

    // Tone voices.
    double carrier = 0.0;    // Hz, left channel frequency
    double offset = 0.0;     // Hz, binaural offset added to the
                             // right channel ('+' higher, '-' lower)

    // Noise voices.
    int noiseColor = Pink;

    // SBaGen linear volume units (typical presets use 5..40).
    double amplitude = 0.0;
};

struct SbgToneSet
{
    QString name;
    QVector<SbgVoice> voices;   // empty = 'off' (silence)

    bool isOff() const { return voices.isEmpty(); }
};

struct SbgTimelineEvent
{
    qint64 timeMs = 0;
    QString toneSetName;       // "-" resolves to silence
};

struct SbgSequence
{
    QString title;
    qint64 durationMs = 0;     // time of the last timeline event

    QVector<SbgToneSet> toneSets;
    QVector<SbgTimelineEvent> events;   // chronological, first at t=0

    const SbgToneSet *toneSetByName(const QString &name) const;
};

struct SbgPreset
{
    QString title;    // user-facing, stable; also used as the preset id
    QString source;   // raw .sbg text
};

namespace Sbg
{
    /*
     * Parses .sbg text into a sequence. Returns false and fills
     * *error on malformed input. Unknown tone-set references and
     * unsupported constructs (absolute clock times, spin
     * variation '@', bell-form beats) are reported as errors
     * rather than silently misinterpreted.
     */
    bool parse(const QString &source,
               SbgSequence *out,
               QString *error = nullptr);

    // Presets embedded with the app (no runtime files needed).
    QVector<SbgPreset> builtinPresets();
}

#endif
