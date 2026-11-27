#ifndef SBGSEQUENCE_H
#define SBGSEQUENCE_H

#include <QString>
#include <QVector>

/*
 * Data model for a parsed SBaGen sequence (.sbg).
 *
 * The deterministic subset of the SBaGen format is supported --
 * the subset the well-known focus/relax/sleep example sequences
 * and the community contributions are built from:
 *
 *   # comment                    (full-line and end-of-line)
 *   -SE, -r, -F, -q ...          (option lines; accepted and ignored,
 *                                 the sequence always crossfades)
 *   name: 100+4.5/8 pink/40      (tone-set definition, '-' = off;
 *                                 a leading 'spin:' is accepted and
 *                                 its modulation ignored)
 *   name: wave:12/30 100+4.5/8   (optional leading 'wave:' keyword
 *                                 marks the set with a slow volume
 *                                 swell: period in seconds, depth
 *                                 in per cent below full level;
 *                                 'wave:12' defaults to depth 30 %)
 *   NOW name
 *   NOW+00:05:00 name            (relative to playback start)
 *   +00:05:00 name               (relative to the last-mentioned
 *                                 absolute time, as in SBaGen)
 *   00:05:00 name ...            (clock-style absolute time,
 *                                  treated as an offset from sequence
 *                                  start; also accepts appended
 *                                  relative groups, "12:00+00:00:20")
 *
 * Fade-in/fade-out specifications ("--", "==", "->", "<>", ...) in
 * time-sequence lines are accepted and read, but rendered with the
 * default smooth per-voice blend (see SequenceGenerator).
 *
 * Voice forms:
 *   carrier+beat/amp    binaural, right channel higher
 *   carrier-beat/amp    binaural, right channel lower
 *   carrier@beat/amp    isochronic pulse (rendered as an amplitude-
 *                       pulsed tone on both ears, matching the
 *                       IsochronicGenerator behaviour)
 *   carrier/amp         plain tone
 *   color/amp           noise voice (white/pink/brown/grey)
 *
 * Amplitude defaults to 20 when omitted; negative amplitudes and
 * tone-sets without voices (other than 'off') are errors.
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
    double offset = 0.0;     // Hz, binaural offset ('+' higher,
                             // '-' lower) or isochronic pulse
                             // frequency ('@')
    bool isochronic = false; // 'carrier@beat': amplitude pulse

    // Noise voices.
    int noiseColor = Pink;

    // SBaGen linear volume units (typical presets use 5..40).
    double amplitude = 0.0;
};

struct SbgToneSet
{
    QString name;
    QVector<SbgVoice> voices;   // empty = 'off' (silence)

    /*
     * Authored volume swell ('wave:period[/depth]' keyword): a
     * raised-cosine amplitude envelope applied to every voice of
     * the set, matching the colored-noise wave modulation.
     * waveDepth 0 = no swell; otherwise the volume dips to
     * (1 - waveDepth) of full level at the trough.
     */
    double wavePeriodSeconds = 0.0;
    double waveDepth = 0.0;

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
     * unsupported constructs are reported as errors rather than
     * silently misinterpreted.
     */
    bool parse(const QString &source,
               SbgSequence *out,
               QString *error = nullptr);

    // Generic .sbg presets shown in the sequence drawer.
    QVector<SbgPreset> builtinPresets();

    // Monroe 'Focus level' presets, shown only in the Focus section.
    QVector<SbgPreset> focusPresets();
}

#endif
