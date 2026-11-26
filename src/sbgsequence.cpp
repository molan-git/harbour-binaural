#include "sbgsequence.h"

#include <QtGlobal>

#include <algorithm>

// ============================================================================
// HELPERS
// ============================================================================

namespace
{
    /*
     * Whitespace-based tokenizing: splits on spaces and tabs so
     * sequence files with tab indentation parse like their
     * space-indented equivalents.
     */
    QStringList splitTokens(const QString &line)
    {
        QStringList tokens;

        QString current;

        for (int i = 0; i < line.size(); ++i)
        {
            const QChar c = line.at(i);

            if (c.isSpace())
            {
                if (!current.isEmpty())
                {
                    tokens.append(current);
                    current.clear();
                }
            }
            else
            {
                current.append(c);
            }
        }

        if (!current.isEmpty())
            tokens.append(current);

        return tokens;
    }

    bool parseDouble(const QString &text, double *out)
    {
        bool ok = false;

        const double value = text.toDouble(&ok);

        if (ok && qIsFinite(value))
        {
            *out = value;
            return true;
        }

        return false;
    }

    /*
     * "HH:MM:SS", "MM:SS" or "SS" -> milliseconds.
     */
    bool parseClockTimeMs(const QString &token, qint64 *out)
    {
        const QStringList parts =
            token.split(QLatin1Char(':'));

        if (parts.isEmpty() || parts.size() > 3)
            return false;

        double hours = 0.0;
        double minutes = 0.0;
        double seconds = 0.0;

        if (parts.size() == 3)
        {
            if (!parseDouble(parts.at(0), &hours)
                || !parseDouble(parts.at(1), &minutes)
                || !parseDouble(parts.at(2), &seconds))
                return false;
        }
        else if (parts.size() == 2)
        {
            if (!parseDouble(parts.at(0), &minutes)
                || !parseDouble(parts.at(1), &seconds))
                return false;
        }
        else
        {
            if (!parseDouble(parts.at(0), &seconds))
                return false;
        }

        *out = qRound64(
            (hours * 3600.0
             + minutes * 60.0
             + seconds) * 1000.0);

        return true;
    }

    /*
     * "+HH:MM:SS[+MM:SS...]" -> milliseconds. A time token may
     * carry several relative groups, e.g. "12:00+00:00:20".
     */
    bool parseRelativeGroups(const QString &token, qint64 *out)
    {
        if (!token.startsWith(QLatin1Char('+')))
            return false;

        const QStringList parts =
            token.split(QLatin1Char('+'));

        if (parts.size() < 2)
            return false;

        qint64 totalMs = 0;

        for (int i = 1; i < parts.size(); ++i)
        {
            qint64 deltaMs = 0;

            if (!parseClockTimeMs(parts.at(i), &deltaMs))
                return false;

            totalMs += deltaMs;
        }

        *out = totalMs;

        return true;
    }

    /*
     * Fade-in/fade-out specification in a time-sequence line
     * ("--", "==", "->", "<>", "=<", ...). Two characters:
     * fade-in from {<, -, =}, fade-out to {>, -, =}.
     */
    bool isFadeSpec(const QString &token)
    {
        if (token.size() != 2)
            return false;

        const QChar in = token.at(0);
        const QChar out = token.at(1);

        const bool inOk = in == QLatin1Char('<')
            || in == QLatin1Char('-')
            || in == QLatin1Char('=');

        const bool outOk = out == QLatin1Char('>')
            || out == QLatin1Char('-')
            || out == QLatin1Char('=');

        return inOk && outOk;
    }

    bool noiseColorFromName(const QString &name, int *out)
    {
        const QString lower = name.toLower();

        if (lower == QLatin1String("white"))
            *out = SbgVoice::White;
        else if (lower == QLatin1String("pink"))
            *out = SbgVoice::Pink;
        else if (lower == QLatin1String("brown"))
            *out = SbgVoice::Brown;
        else if (lower == QLatin1String("grey")
                 || lower == QLatin1String("gray"))
            *out = SbgVoice::Grey;
        else
            return false;

        return true;
    }

    /*
     * Voice token forms:
     *   pink/40            noise voice
     *   100+4.5/8          tone, right channel higher
     *   200-4/6            tone, right channel lower
     *   300@6/10           isochronic pulse at 6 Hz
     *   300/6              plain tone (no offset/pulse)
     *
     * Amplitude defaults to 20 (a common SBaGen level) when omitted.
     */
    bool parseVoice(const QString &token, SbgVoice *out, QString *error)
    {
        // Allow callers to pass a null error string.
        QString localError;

        if (!error)
            error = &localError;

        // Optional "/amplitude" suffix shared by tones and noise.
        const int slash = token.indexOf(QLatin1Char('/'));

        const QString head =
            slash >= 0 ? token.left(slash) : token;

        double amplitude = 20.0;

        if (slash >= 0
            && !parseDouble(token.mid(slash + 1), &amplitude))
        {
            *error = QStringLiteral(
                "bad amplitude: %1").arg(token);
            return false;
        }

        if (amplitude < 0.0)
        {
            *error = QStringLiteral(
                "negative amplitude: %1").arg(token);
            return false;
        }

        // Noise voice: "color[/amplitude]".
        int noiseColor = -1;

        if (noiseColorFromName(head, &noiseColor))
        {
            out->type = SbgVoice::Noise;
            out->noiseColor = noiseColor;
            out->amplitude = amplitude;
            return true;
        }

        // Tone voice: "carrier[+-@offset][/amplitude]".
        out->type = SbgVoice::Tone;
        out->amplitude = amplitude;

        const QString &freqPart = head;

        // Optional offset/pulse marker: a single '+', '-' or '@'.
        int signPos = -1;

        for (int i = 0; i < freqPart.size(); ++i)
        {
            const QChar c = freqPart.at(i);

            if (c == QLatin1Char('+')
                || c == QLatin1Char('-')
                || c == QLatin1Char('@'))
            {
                signPos = i;
                break;
            }
        }

        if (signPos < 0)
        {
            if (!parseDouble(freqPart, &out->carrier))
            {
                *error = QStringLiteral(
                    "bad carrier frequency: %1").arg(token);
                return false;
            }

            out->offset = 0.0;
            out->isochronic = false;
            return true;
        }

        if (!parseDouble(freqPart.left(signPos), &out->carrier))
        {
            *error = QStringLiteral(
                "bad carrier frequency: %1").arg(token);
            return false;
        }

        if (!parseDouble(freqPart.mid(signPos + 1), &out->offset))
        {
            *error = QStringLiteral(
                "bad offset/pulse frequency: %1").arg(token);
            return false;
        }

        // '@' marks an isochronic pulse at the given rate.
        out->isochronic =
            freqPart.at(signPos) == QLatin1Char('@');

        // '-' places the lower tone in the right channel.
        if (!out->isochronic
            && freqPart.at(signPos) == QLatin1Char('-'))
            out->offset = -out->offset;

        return true;
    }

    /*
     * Two voices "match" across consecutive tone-sets when they
     * are the same kind of sound with identical parameters: such
     * voices keep playing seamlessly across the transition.
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
     * Extracts the tone-set name from the tokens that follow a
     * time specification, accepting an optional fade spec:
     *   "00:10:00 == ts1"  ->  "ts1"
     *   "0:58 -> t58"      ->  "t58"
     *   "00:00:15 ts1 ->"  ->  "ts1"
     */
    bool extractToneSetName(const QStringList &tokens,
                            int startIndex,
                            QString *name,
                            QString *error,
                            int lineNr)
    {
        if (tokens.size() <= startIndex)
        {
            if (error)
                *error = QStringLiteral(
                    "line %1: expected tone-set name")
                    .arg(lineNr + 1);
            return false;
        }

        const QString first = tokens.at(startIndex);

        if (isFadeSpec(first))
        {
            if (tokens.size() <= startIndex + 1)
            {
                if (error)
                    *error = QStringLiteral(
                        "line %1: fade spec without tone-set name")
                        .arg(lineNr + 1);
                return false;
            }

            *name = tokens.at(startIndex + 1);
            return true;
        }

        *name = first;
        return true;
    }
}

// ============================================================================
// LOOKUP
// ============================================================================

const SbgToneSet *SbgSequence::toneSetByName(const QString &name) const
{
    for (int i = 0; i < toneSets.size(); ++i)
    {
        if (toneSets.at(i).name == name)
            return &toneSets.at(i);
    }

    return nullptr;
}

// ============================================================================
// PARSER
// ============================================================================

bool Sbg::parse(const QString &source,
                SbgSequence *out,
                QString *error)
{
    if (!out)
        return false;

    out->title.clear();
    out->durationMs = 0;
    out->toneSets.clear();
    out->events.clear();

    /*
     * Relative times ("+00:05:00") are relative to the
     * last-mentioned absolute time ("NOW" or a clock time), as in
     * SBaGen -- they do not accumulate from line to line.
     */
    qint64 lastAbsoluteMs = 0;

    const QStringList lines = source.split(QLatin1Char('\n'));

    for (int lineNr = 0; lineNr < lines.size(); ++lineNr)
    {
        QString line = lines.at(lineNr);

        // Full-line and end-of-line comments.
        const int hash = line.indexOf(QLatin1Char('#'));

        if (hash >= 0)
            line.truncate(hash);

        line = line.trimmed();

        if (line.isEmpty())
            continue;

        /*
         * Option lines (-SE, -S, -TS, -m, -r, -F, -q ...). The
         * sequence engine always crossfades between tone-sets, so
         * -SE behaviour is implicit; other options are accepted
         * and ignored.
         */
        if (line.startsWith(QLatin1Char('-')))
            continue;

        const QStringList tokens = splitTokens(line);

        if (tokens.isEmpty())
            continue;

        // ======================================================
        // Time-sequence lines.
        // ======================================================

        // "NOW name" or "NOW+HH:MM:SS name".
        if (tokens.at(0) == QLatin1String("NOW")
            || tokens.at(0).startsWith(QLatin1String("NOW+")))
        {
            qint64 timeMs = 0;

            if (tokens.at(0).size() > 3)
            {
                qint64 deltaMs = 0;

                if (!parseRelativeGroups(
                        tokens.at(0).mid(3), &deltaMs))
                {
                    if (error)
                        *error = QStringLiteral(
                            "line %1: bad time: %2")
                            .arg(lineNr + 1)
                            .arg(tokens.at(0));
                    return false;
                }

                timeMs = deltaMs;
            }

            // NOW marks the playback start.
            lastAbsoluteMs = 0;

            SbgTimelineEvent event;
            event.timeMs = timeMs;

            QString name;

            if (!extractToneSetName(
                    tokens, 1, &name, error, lineNr))
                return false;

            event.toneSetName = name;
            out->events.append(event);
            continue;
        }

        // "+HH:MM:SS name": relative to the last absolute time.
        if (tokens.at(0).startsWith(QLatin1Char('+')))
        {
            qint64 deltaMs = 0;

            if (!parseRelativeGroups(tokens.at(0), &deltaMs))
            {
                if (error)
                    *error = QStringLiteral(
                        "line %1: bad time: %2")
                        .arg(lineNr + 1)
                        .arg(tokens.at(0));
                return false;
            }

            SbgTimelineEvent event;
            event.timeMs = lastAbsoluteMs + deltaMs;

            QString name;

            if (!extractToneSetName(
                    tokens, 1, &name, error, lineNr))
                return false;

            event.toneSetName = name;
            out->events.append(event);
            continue;
        }

        // Absolute clock time, optionally with relative groups
        // appended ("00:05:00 ts1 ->", "12:00+00:00:20 name").
        if (tokens.at(0).at(0).isDigit())
        {
            const QString timeToken = tokens.at(0);

            const int plus = timeToken.indexOf(QLatin1Char('+'));

            const QString base =
                plus >= 0 ? timeToken.left(plus) : timeToken;

            qint64 timeMs = 0;

            if (!parseClockTimeMs(base, &timeMs))
            {
                if (error)
                    *error = QStringLiteral(
                        "line %1: bad time: %2")
                        .arg(lineNr + 1)
                        .arg(tokens.at(0));
                return false;
            }

            if (plus >= 0)
            {
                qint64 deltaMs = 0;

                if (!parseRelativeGroups(
                        timeToken.mid(plus), &deltaMs))
                {
                    if (error)
                        *error = QStringLiteral(
                            "line %1: bad time: %2")
                            .arg(lineNr + 1)
                            .arg(tokens.at(0));
                    return false;
                }

                timeMs += deltaMs;
            }

            // Absolute times act as the base for later
            // relative times.
            lastAbsoluteMs = timeMs;

            SbgTimelineEvent event;
            event.timeMs = timeMs;

            QString name;

            if (!extractToneSetName(
                    tokens, 1, &name, error, lineNr))
                return false;

            event.toneSetName = name;
            out->events.append(event);
            continue;
        }

        // ======================================================
        // Tone-set definition: "name: voice voice ...".
        // ======================================================
        //
        // A leading "spin:" keyword marks a slowly spun voice
        // set; the spin modulation is ignored, the voices render
        // as ordinary voices.

        const int colon = line.indexOf(QLatin1Char(':'));

        if (colon > 0)
        {
            SbgToneSet set;
            set.name = line.left(colon).trimmed();

            QString voiceList = line.mid(colon + 1);

            if (voiceList.trimmed().startsWith(
                    QStringLiteral("spin:")))
            {
                voiceList = voiceList.trimmed().mid(
                    QStringLiteral("spin:").size());
            }

            const QStringList voices = splitTokens(voiceList);

            for (int i = 0; i < voices.size(); ++i)
            {
                const QString &token = voices.at(i);

                if (token == QLatin1String("-"))
                    continue;   // 'off' channel marker

                SbgVoice voice;

                if (!parseVoice(token, &voice, error))
                {
                    if (error)
                        *error = QStringLiteral(
                            "line %1: %2")
                            .arg(lineNr + 1)
                            .arg(*error);
                    return false;
                }

                set.voices.append(voice);
            }

            // 'off: -' is the legal silent tone-set; anything
            // else without voices is a mistake.
            if (set.voices.isEmpty()
                && set.name != QLatin1String("off"))
            {
                if (error)
                    *error = QStringLiteral(
                        "line %1: tone-set without voices: %2")
                        .arg(lineNr + 1)
                        .arg(set.name);
                return false;
            }

            out->toneSets.append(set);
            continue;
        }

        if (error)
            *error = QStringLiteral(
                "line %1: unsupported construct: %2")
                .arg(lineNr + 1)
                .arg(line);
        return false;
    }

    if (out->events.isEmpty())
    {
        if (error)
            *error = QStringLiteral(
                "no timeline events found");
        return false;
    }

    // Keep the timeline chronological regardless of line order.
    std::stable_sort(
        out->events.begin(),
        out->events.end(),
        [](const SbgTimelineEvent &a,
           const SbgTimelineEvent &b)
        {
            return a.timeMs < b.timeMs;
        });

    // Resolve event tone-set references.
    for (int i = 0; i < out->events.size(); ++i)
    {
        const SbgTimelineEvent &event = out->events.at(i);

        if (event.toneSetName == QLatin1String("-"))
            continue;

        if (!out->toneSetByName(event.toneSetName))
        {
            if (error)
                *error = QStringLiteral(
                    "unknown tone-set: %1")
                    .arg(event.toneSetName);
            return false;
        }
    }

    out->durationMs = out->events.last().timeMs;

    return true;
}

// ============================================================================
// BUILTIN PRESETS
// ============================================================================
//
// Preset sources are taken from the classic SBaGen example
// collections (SBaGen by Jim Peters; example bundles by the
// sbagen-plus project) and from the community contributions
// shipped with SBaGenX (posted to the sbagen-users mailing list).
// All sources are GPL-2.0 licensed.
//
// The Monroe 'Focus level' tone-sets are based on community-
// measured carrier/offset values, published with the original
// SBaGen distribution (focus.txt).
//
// The Focus-level files originally contain only a single
// continuous tone-set ("0:00 ts"); a 60-minute session timeline
// has been added for use as an app preset.
//
// sbagen-plus '@' isochronic voices are rendered as amplitude
// pulses on both ears (see parseVoice / SequenceGenerator).

QVector<SbgPreset> Sbg::builtinPresets()
{
    QVector<SbgPreset> presets;

    // ------------------------------------------------------------------
    // Sequences from the sbagen-plus example bundle.
    // ------------------------------------------------------------------

    SbgPreset concentration;
    concentration.title =
        QStringLiteral("Focus & Concentration (Beta)");
    concentration.source = QStringLiteral(
        "# Focus and Concentration - 60 minutes\n"
        "-SE\n"
        "ts-start: pink/30 300@12/20\n"
        "ts-focus: pink/30 300@15/25\n"
        "ts-deep-focus: pink/30 300@18/20\n"
        "ts-wind-down: pink/30 300@12/20\n"
        "off: -\n"
        "00:00:00 off\n"
        "00:00:15 ts-start\n"
        "00:10:00 ts-focus\n"
        "00:25:00 ts-deep-focus\n"
        "00:40:00 ts-wind-down\n"
        "00:55:00 off\n");
    presets.append(concentration);

    SbgPreset deepSleep;
    deepSleep.title = QStringLiteral("Deep Sleep Aid");
    deepSleep.source = QStringLiteral(
        "# Deep Sleep Aid - 45 minutes\n"
        "-SE\n"
        "ts-relax: brown/60 300+10/15\n"
        "ts-drowsy: brown/60 200+7/20\n"
        "ts-sleep: brown/70 100+3/25\n"
        "ts-deep-sleep: brown/70 100+1.5/20\n"
        "off: -\n"
        "00:00:00 off\n"
        "00:00:15 ts-relax\n"
        "00:10:00 ts-drowsy\n"
        "00:20:00 ts-sleep\n"
        "00:30:00 ts-deep-sleep\n"
        "00:45:00 off\n");
    presets.append(deepSleep);

    SbgPreset meditation;
    meditation.title = QStringLiteral("Simple Meditation");
    meditation.source = QStringLiteral(
        "# Simple 30-minute meditation sequence\n"
        "-SE\n"
        "ts-start: pink/40 200+10/15\n"
        "ts-deep: pink/40 200+6/20\n"
        "ts-end: pink/40 200+10/15\n"
        "off: -\n"
        "00:00:00 off\n"
        "00:00:15 ts-start\n"
        "00:15:00 ts-deep\n"
        "00:25:00 ts-end\n"
        "00:30:00 off\n");
    presets.append(meditation);

    SbgPreset creativity;
    creativity.title = QStringLiteral("Creativity Boost");
    creativity.source = QStringLiteral(
        "# Creativity Boost - 40 minutes\n"
        "-SE\n"
        "ts-relax: pink/40 200+10/20\n"
        "ts-creative: pink/40 200+8/15 300@6/10\n"
        "ts-deep-creative: pink/40 150+6/20 250@7/15\n"
        "ts-return: pink/40 200+10/20\n"
        "off: -\n"
        "00:00:00 off\n"
        "00:00:15 ts-relax\n"
        "00:10:00 ts-creative\n"
        "00:20:00 ts-deep-creative\n"
        "00:35:00 ts-return\n"
        "00:40:00 off\n");
    presets.append(creativity);

    SbgPreset stressRelief;
    stressRelief.title = QStringLiteral("Stress Relief");
    stressRelief.source = QStringLiteral(
        "# Stress Relief - 30 minutes\n"
        "-SE\n"
        "ts-start: white/30 250+10/20\n"
        "ts-relax: white/30 250+9/25\n"
        "ts-deep-relax: white/30 250+8/20\n"
        "ts-return: white/30 250+10/15\n"
        "off: -\n"
        "00:00:00 off\n"
        "00:00:15 ts-start\n"
        "00:05:00 ts-relax\n"
        "00:15:00 ts-deep-relax\n"
        "00:25:00 ts-return\n"
        "00:30:00 off\n");
    presets.append(stressRelief);

    SbgPreset energizer;
    energizer.title = QStringLiteral("Morning Energizer");
    energizer.source = QStringLiteral(
        "# Morning Energizer - 20 minutes\n"
        "-SE\n"
        "ts-wake: pink/30 300@10/20\n"
        "ts-energize: pink/30 300@14/25\n"
        "ts-alert: pink/30 300@18/20\n"
        "ts-balanced: pink/30 300@12/15\n"
        "off: -\n"
        "00:00:00 off\n"
        "00:00:15 ts-wake\n"
        "00:05:00 ts-energize\n"
        "00:10:00 ts-alert\n"
        "00:15:00 ts-balanced\n"
        "00:20:00 off\n");
    presets.append(energizer);

    // ------------------------------------------------------------------
    // Gateway Experience waves.
    // ------------------------------------------------------------------

    SbgPreset gatewayWave1;
    gatewayWave1.title = QStringLiteral("Gateway Wave I (F10→F12)");
    gatewayWave1.source = QStringLiteral(
        "# Gateway Wave I - 45 minutes\n"
        "# Focus 10 (body asleep) -> Focus 12 (expanded awareness) -> F10\n"
        "ts-f10: 100+1.5/10 200+4.0/10 250+4.0/10 300+4.0/10\n"
        "ts-f12: 100+1.5/10 200+4.0/10 250+4.0/10 300+4.0/10 "
        "400+10.0/10 500+10.1/10 600+4.8/10\n"
        "off: -\n"
        "00:00:00 off\n"
        "00:00:15 ts-f10\n"
        "00:20:00 ts-f12\n"
        "00:35:00 ts-f10\n"
        "00:45:00 off\n");
    presets.append(gatewayWave1);

    SbgPreset gatewayWave2;
    gatewayWave2.title = QStringLiteral("Gateway Wave II (F10→F15)");
    gatewayWave2.source = QStringLiteral(
        "# Gateway Wave II - 50 minutes\n"
        "# Focus 10 -> Focus 15 (no time) -> Focus 12 -> F10\n"
        "ts-f10: 100+1.5/10 200+4.0/10 250+4.0/10 300+4.0/10\n"
        "ts-f15: 100+1.5/10 200+4.0/10 250+4.0/10 300+4.0/10 "
        "500+7.05/10 630+7.1/10 750+7.0/10\n"
        "ts-f12: 100+1.5/10 200+4.0/10 250+4.0/10 300+4.0/10 "
        "400+10.0/10 500+10.1/10 600+4.8/10\n"
        "off: -\n"
        "00:00:00 off\n"
        "00:00:15 ts-f10\n"
        "00:15:00 ts-f15\n"
        "00:30:00 ts-f12\n"
        "00:40:00 ts-f10\n"
        "00:50:00 off\n");
    presets.append(gatewayWave2);

    SbgPreset gatewayWave3;
    gatewayWave3.title = QStringLiteral("Gateway Wave III (F10→F21)");
    gatewayWave3.source = QStringLiteral(
        "# Gateway Wave III - 60 minutes\n"
        "# Focus 10 -> F12 -> F15 -> F21 -> F15 -> F12 -> F10\n"
        "ts-f10: 100+1.5/10 200+4.0/10 250+4.0/10 300+4.0/10\n"
        "ts-f12: 100+1.5/10 200+4.0/10 250+4.0/10 300+4.0/10 "
        "400+10.0/10 500+10.1/10 600+4.8/10\n"
        "ts-f15: 100+1.5/10 200+4.0/10 250+4.0/10 300+4.0/10 "
        "500+7.05/10 630+7.1/10 750+7.0/10\n"
        "ts-f21: 200+4.0/10 250+4.0/10 300+4.0/10 "
        "600+16.2/10 750+15.9/10 900+16.2/10\n"
        "off: -\n"
        "00:00:00 off\n"
        "00:00:15 ts-f10\n"
        "00:10:00 ts-f12\n"
        "00:20:00 ts-f15\n"
        "00:30:00 ts-f21\n"
        "00:40:00 ts-f15\n"
        "00:50:00 ts-f12\n"
        "00:55:00 ts-f10\n"
        "01:00:00 off\n");
    presets.append(gatewayWave3);

    // ------------------------------------------------------------------
    // Community contributions, as shipped with SBaGenX
    // (examples/contrib; posted to the sbagen-users mailing list).
    // ------------------------------------------------------------------

    SbgPreset schumannCycle;
    schumannCycle.title = QStringLiteral("Schumann Cycle");
    schumannCycle.source = QStringLiteral(
        "# Schumann Cycle - posted by Crawford Hart to the\n"
        "# sbagen-users mailing list, 19 May 2006.\n"
        "# All seven Schumann Resonances, each in its own\n"
        "# harmonic box; each higher frequency holds 5 minutes.\n"
        "-SE\n"
        "ts1: pink/80 125.2+7.83/4 109.62-7.83/4\n"
        "ts2: pink/80 112-14/3 126+14/3\n"
        "ts3: pink/80 160+20/3 120-20/3\n"
        "ts4: pink/80 208+26/3 156-26/3\n"
        "ts5: pink/80 264+33/3 198-33/3\n"
        "ts6: pink/80 312+39/3 234-39/3\n"
        "ts7: pink/80 125.2+7.83/0 109.62-7.83/0 360+45/3 270-45/3\n"
        "ts8: pink/80 125.2+7.83/3 109.62-7.83/3 360+45/0 270-45/0\n"
        "off: -\n"
        "00:00:00 off ->\n"
        "00:00:15 ts1 ->\n"
        "00:10:00 == ts1\n"
        "00:10:02 ts2 ->\n"
        "00:15:00 == ts2\n"
        "00:15:02 ts3 ->\n"
        "00:20:00 == ts3\n"
        "00:20:02 ts4 ->\n"
        "00:25:00 == ts4\n"
        "00:25:02 ts5 ->\n"
        "00:30:00 == ts5\n"
        "00:30:02 ts6 ->\n"
        "00:35:00 == ts6\n"
        "00:35:02 ts7 ->\n"
        "00:40:00 ts7 ->\n"
        "00:42:00 ts8\n"
        "00:45:00 ts8 ->\n"
        "00:45:30 off\n");
    presets.append(schumannCycle);

    SbgPreset awakenedMind;
    awakenedMind.title = QStringLiteral("The Awakened Mind");
    awakenedMind.source = QStringLiteral(
        "# The Awakened Mind - posted by Crawford Hart to the\n"
        "# sbagen-users mailing list, 19 May 2006.\n"
        "# 20 minute program; four voices, one each in beta,\n"
        "# alpha, theta and delta. Slow drop in frequency while\n"
        "# the volume glides from beta- to delta-weighted.\n"
        "-SE\n"
        "off: -\n"
        "ts1: pink/60 750+15/8 500+10/4 350+7/0 100+2/0\n"
        "ts2: pink/60 750+14/0 500+9/8 350+6/6 100+2/8\n"
        "00:00:00 off ->\n"
        "00:00:15 ts1 ->\n"
        "00:20:00 ts2 ->\n"
        "00:20:30 off\n");
    presets.append(awakenedMind);

    SbgPreset ultimateAspirin;
    ultimateAspirin.title = QStringLiteral("Ultimate Aspirin");
    ultimateAspirin.source = QStringLiteral(
        "# Ultimate Aspirin - posted by Crawford Hart to the\n"
        "# sbagen-users mailing list, 19 May 2006.\n"
        "# Three frequencies said to promote endorphin release;\n"
        "# 2.5 and 4 Hz pair in turn with 38 Hz, all three play\n"
        "# in the middle of the cycle. Carriers sound in octaves.\n"
        "-SE\n"
        "ts1: pink/80 100+2.5/3 200+4/0 400+38/3\n"
        "ts2: pink/80 100+2.5/0 200+4/3 400+38/3\n"
        "ts3: pink/80 100+2.5/3 200+4/3 400+38/3\n"
        "off: -\n"
        "00:00:00 off ->\n"
        "00:00:15 ts1\n"
        "00:03:30 ts1 ->\n"
        "00:04:00 ts2\n"
        "00:06:00 ts2 ->\n"
        "00:06:30 ts3\n"
        "00:08:00 ts3 ->\n"
        "00:08:30 ts2\n"
        "00:10:30 ts2 ->\n"
        "00:11:00 ts1\n"
        "00:12:00 ts1 ->\n"
        "00:12:30 off\n");
    presets.append(ultimateAspirin);

    SbgPreset instantEinstein;
    instantEinstein.title = QStringLiteral("Instant Einstein");
    instantEinstein.source = QStringLiteral(
        "# Instant Einstein - posted by Crawford Hart to the\n"
        "# sbagen-users mailing list, 19 May 2006.\n"
        "# 15 minutes at 40 Hz in a two-voice harmonic box.\n"
        "-SE\n"
        "ts: pink/80 360-40/6 440+40/6\n"
        "off: -\n"
        "00:00:00 off ->\n"
        "00:00:15 ts\n"
        "00:15:00 ts ->\n"
        "00:15:30 off\n");
    presets.append(instantEinstein);

    SbgPreset inspiration;
    inspiration.title = QStringLiteral("Inspiration");
    inspiration.source = QStringLiteral(
        "# Inspiration - posted by Crawford Hart to the\n"
        "# sbagen-users mailing list, 1 Jun 2007.\n"
        "# 5 minutes harmonized carriers at 10 Hz alpha, a\n"
        "# 2-minute glide to 8 Hz, 10 minutes at 8 Hz, then a\n"
        "# low 1.5 Hz delta in a third harmonized carrier; ends\n"
        "# with a 3-minute glide back to 10 Hz.\n"
        "-SE\n"
        "off: -\n"
        "ts1: pink/80 100+1.5/0 200+10.5/3 400+10.5/3\n"
        "ts2: pink/80 100+1.5/0 200+8/3 400+8/3\n"
        "ts3: pink/80 100+1.5/4 200+7.5/3 400+7.5/3\n"
        "00:00:00 off ->\n"
        "00:00:10 ts1\n"
        "00:05:00 ts1 ->\n"
        "00:07:00 ts2\n"
        "00:17:00 ts2 ->\n"
        "00:18:00 ts3\n"
        "00:30:00 ts3 ->\n"
        "00:33:00 ts1 ->\n"
        "00:33:30 off\n");
    presets.append(inspiration);

    SbgPreset sonicCaffeine;
    sonicCaffeine.title = QStringLiteral("Sonic Caffeine");
    sonicCaffeine.source = QStringLiteral(
        "# Sonic Caffeine - posted by Crawford Hart to the\n"
        "# sbagen-users mailing list, 1 Jun 2007.\n"
        "# Focus and concentration; a 45-minute program ramping\n"
        "# up through several beta frequencies.\n"
        "-SE\n"
        "ts: pink/80 250+18/4\n"
        "ts1: pink/80 250+22/4\n"
        "ts2: pink/80 250+30/4\n"
        "ts3: pink/80 250+33/4\n"
        "ts4: pink/80 250+14/4\n"
        "off: -\n"
        "00:00:00 off ->\n"
        "00:00:10 ts\n"
        "00:05:00 ts ->\n"
        "00:05:10 ts1\n"
        "00:09:00 ts1 ->\n"
        "00:09:10 ts2\n"
        "00:15:00 ts2 ->\n"
        "00:15:10 ts3\n"
        "00:17:00 ts3 ->\n"
        "00:17:10 ts1\n"
        "00:20:00 ts1 ->\n"
        "00:20:10 ts2\n"
        "00:30:00 ts2 ->\n"
        "00:30:10 ts3\n"
        "00:35:00 ts3 ->\n"
        "00:35:10 ts2\n"
        "00:40:00 ts2 ->\n"
        "00:43:00 ts4\n"
        "00:45:00 ts4 ->\n"
        "00:45:30 off\n");
    presets.append(sonicCaffeine);

    SbgPreset runnersHigh;
    runnersHigh.title = QStringLiteral("Runner's High");
    runnersHigh.source = QStringLiteral(
        "# Runner's High - posted by Crawford Hart to the\n"
        "# sbagen-users mailing list, 1 Jun 2007.\n"
        "# A harmonic box reinforcing the endorphin-releasing\n"
        "# 38 Hz.\n"
        "-SE\n"
        "ts1: pink/80 111-38/4 187+38/3\n"
        "off: -\n"
        "00:00:00 off ->\n"
        "00:00:15 ts1\n"
        "00:20:00 ts1 ->\n"
        "00:20:30 off\n");
    presets.append(runnersHigh);

    SbgPreset pranaEnergizer;
    pranaEnergizer.title = QStringLiteral("Prana Energizer");
    pranaEnergizer.source = QStringLiteral(
        "# Prana Energizer - posted by Chaven R Yenketswamy to\n"
        "# the sbagen-users mailing list; reproduced from a\n"
        "# Brainwave Generator scan of a calm 5.2-5.9 Hz\n"
        "# interval, carriers raised for audibility.\n"
        "-SE\n"
        "ts1: 744+12.4/5.0 312+5.2/5.0\n"
        "ts2: 696+11.6/5.0 342+5.7/5.0\n"
        "ts3: 672+11.2/5.0 354+5.9/5.0\n"
        "off: -\n"
        "00:00:00 == off ->\n"
        "00:00:01 == ts1 ->\n"
        "00:01:30 == ts2 ->\n"
        "00:03:00 == ts3 ->\n"
        "00:03:01 == off ->\n");
    presets.append(pranaEnergizer);

    return presets;
}

// ============================================================================
// FOCUS PRESETS
// ============================================================================
//
// The Monroe 'Focus level' tone-sets (community-measured values,
// focus.txt from the original SBaGen distribution). Registered
// separately from builtinPresets so they appear only in the Focus
// section, not in the sequence drawer. F22 uses the same tone-set
// as F21, so the two share one preset.

QVector<SbgPreset> Sbg::focusPresets()
{
    QVector<SbgPreset> presets;

    SbgPreset focus3;
    focus3.title = QStringLiteral("Focus 3 (Sleep)");
    focus3.source = QStringLiteral(
        "# F3: 59[1.2]-110[1.3], 288[3.7]\n"
        "ts: 59+1.2/10 110+1.3/10 288+3.7/10\n"
        "off: -\n"
        "NOW ts\n"
        "+00:60:00 off\n");
    presets.append(focus3);

    SbgPreset focus10;
    focus10.title = QStringLiteral("Focus 10 (Body Asleep)");
    focus10.source = QStringLiteral(
        "# F10 \"Brain awake, body asleep\":\n"
        "# 100[1.5], 200[4.0], 250[4.0], 300[4.0]\n"
        "ts: 100+1.5/10 200+4.0/10 250+4.0/10 300+4.0/10\n"
        "off: -\n"
        "NOW ts\n"
        "+00:60:00 off\n");
    presets.append(focus10);

    SbgPreset focus12;
    focus12.title = QStringLiteral("Focus 12 (Expanded Awareness)");
    focus12.source = QStringLiteral(
        "# F12 \"Expanded consciousness\":\n"
        "# 100[1.5], 200[4.0], 250[4.0], 300[4.0],\n"
        "# 400[10.0], 500[10.1], 600[4.8]\n"
        "ts: 100+1.5/10 200+4.0/10 250+4.0/10 300+4.0/10 "
        "400+10.0/10 500+10.1/10 600+4.8/10\n"
        "off: -\n"
        "NOW ts\n"
        "+00:60:00 off\n");
    presets.append(focus12);

    SbgPreset focus15;
    focus15.title = QStringLiteral("Focus 15 (No Time)");
    focus15.source = QStringLiteral(
        "# F15 \"No time -- 3 hours seems like 2 minutes\":\n"
        "# 100[1.5], 200[4.0], 250[4.0], 300[4.0],\n"
        "# 500[7.05], 630[7.1], 750[7.0]\n"
        "ts: 100+1.5/10 200+4.0/10 250+4.0/10 300+4.0/10 "
        "500+7.05/10 630+7.1/10 750+7.0/10\n"
        "off: -\n"
        "NOW ts\n"
        "+00:60:00 off\n");
    presets.append(focus15);

    SbgPreset focus21;
    focus21.title = QStringLiteral("Focus 21/22 (Other Energy Systems)");
    focus21.source = QStringLiteral(
        "# F21/F22 (F22 uses the same tone-set as F21):\n"
        "# 200[4.0], 250[4.0], 300[4.0],\n"
        "# 600[16.2], 750[15.9], 900[16.2]\n"
        "ts: 200+4.0/10 250+4.0/10 300+4.0/10 "
        "600+16.2/10 750+15.9/10 900+16.2/10\n"
        "off: -\n"
        "NOW ts\n"
        "+00:60:00 off\n");
    presets.append(focus21);

    SbgPreset focus23;
    focus23.title = QStringLiteral("Focus 23");
    focus23.source = QStringLiteral(
        "# F23: 400[3.9], 503[4.0], 600[4.0], 750[3.9], 900[4.0]\n"
        "ts: 400+3.9/10 503+4.0/10 600+4.0/10 750+3.9/10 900+4.0/10\n"
        "off: -\n"
        "NOW ts\n"
        "+00:60:00 off\n");
    presets.append(focus23);

    SbgPreset focus24;
    focus24.title = QStringLiteral("Focus 24");
    focus24.source = QStringLiteral(
        "# F24: 50[0.75], 400[3.9], 503[4.0], 600[4.0], 750[4.0], 900[4.0]\n"
        "ts: 50+0.75/10 400+3.9/10 503+4.0/10 600+4.0/10 "
        "750+4.0/10 900+4.0/10\n"
        "off: -\n"
        "NOW ts\n"
        "+00:60:00 off\n");
    presets.append(focus24);

    SbgPreset focus25;
    focus25.title = QStringLiteral("Focus 25");
    focus25.source = QStringLiteral(
        "# F25: 503[4.0], 600[4.0], 750[4.0], 900[4.0]\n"
        "ts: 503+4.0/10 600+4.0/10 750+4.0/10 900+4.0/10\n"
        "off: -\n"
        "NOW ts\n"
        "+00:60:00 off\n");
    presets.append(focus25);

    return presets;
}
