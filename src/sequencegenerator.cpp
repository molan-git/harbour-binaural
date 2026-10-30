#include "sbgsequence.h"

#include <QtGlobal>

#include <algorithm>

// ============================================================================
// HELPERS
// ============================================================================

namespace
{
    QStringList splitTokens(const QString &line)
    {
        QStringList tokens;

        const QStringList parts = line.split(QLatin1Char(' '));

        for (int i = 0; i < parts.size(); ++i)
        {
            if (!parts.at(i).isEmpty())
                tokens.append(parts.at(i));
        }

        return tokens;
    }

    bool parseDouble(const QString &text, double *out)
    {
        bool ok = false;

        const double value = text.toDouble(&ok);

        if (ok)
            *out = value;

        return ok;
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

        *out = qRound(
            (hours * 3600.0
             + minutes * 60.0
             + seconds) * 1000.0);

        return true;
    }

    /*
     * "+HH:MM:SS", "+MM:SS" or "+SS" -> milliseconds offset.
     */
    bool parseRelativeTimeMs(const QString &token, qint64 *out)
    {
        if (!token.startsWith(QLatin1Char('+')))
            return false;

        return parseClockTimeMs(token.mid(1), out);
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
     *   300@6/10           sbagen-plus isochronic; approximated
     *                      here as a binaural '+offset' voice
     *   300/6              plain tone (no binaural offset)
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

        // Optional binaural offset: a single '+', '-' or '@'.
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
                "bad binaural offset: %1").arg(token);
            return false;
        }

        // '@' (isochronic) approximates a right-higher binaural beat.
        if (freqPart.at(signPos) == QLatin1Char('-'))
            out->offset = -out->offset;

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

    qint64 nowMs = 0;

    const QStringList lines = source.split(QLatin1Char('\n'));

    for (int lineNr = 0; lineNr < lines.size(); ++lineNr)
    {
        QString line = lines.at(lineNr).trimmed();

        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;

        /*
         * Option lines (-SE, -S, -TS, -m ...). The sequence engine
         * always crossfades between tone-sets, so -SE behaviour is
         * implicit; other options are accepted and ignored.
         */
        if (line.startsWith(QLatin1Char('-')))
            continue;

        // Timeline: "NOW name".
        if (line == QLatin1String("NOW")
            || line.startsWith(QLatin1String("NOW ")))
        {
            SbgTimelineEvent event;
            event.timeMs = nowMs;

            if (line.size() > 4)
                event.toneSetName = line.mid(4).trimmed();

            out->events.append(event);
            continue;
        }

        const QStringList tokens = splitTokens(line);

        if (tokens.isEmpty())
            continue;

        // Timeline: "+time name ...".
        if (tokens.at(0).startsWith(QLatin1Char('+')))
        {
            if (tokens.size() < 2)
            {
                if (error)
                    *error = QStringLiteral(
                        "line %1: expected '+time name'")
                        .arg(lineNr + 1);
                return false;
            }

            qint64 deltaMs = 0;

            if (!parseRelativeTimeMs(tokens.at(0), &deltaMs))
            {
                if (error)
                    *error = QStringLiteral(
                        "line %1: bad time: %2")
                        .arg(lineNr + 1)
                        .arg(tokens.at(0));
                return false;
            }

            nowMs += deltaMs;

            SbgTimelineEvent event;
            event.timeMs = nowMs;
            event.toneSetName = tokens.at(1);

            out->events.append(event);
            continue;
        }

        /*
         * Timeline: absolute clock-style entries ("0:00 ts" or
         * "00:05:00 ts-focus ->"). Treated as offsets from the
         * start of the sequence; trailing tokens such as the
         * sbagen-plus '->' marker are ignored.
         */
        if (tokens.at(0).at(0).isDigit())
        {
            if (tokens.size() < 2)
            {
                if (error)
                    *error = QStringLiteral(
                        "line %1: expected 'time name'")
                        .arg(lineNr + 1);
                return false;
            }

            qint64 timeMs = 0;

            if (!parseClockTimeMs(tokens.at(0), &timeMs))
            {
                if (error)
                    *error = QStringLiteral(
                        "line %1: bad time: %2")
                        .arg(lineNr + 1)
                        .arg(tokens.at(0));
                return false;
            }

            SbgTimelineEvent event;
            event.timeMs = timeMs;
            event.toneSetName = tokens.at(1);

            out->events.append(event);
            continue;
        }

        /*
         * Tone-set definition: "name: voice voice ...".
         * A leading "spin:" keyword marks a slowly spun voice
         * set; the spin modulation is ignored, the voices render
         * as ordinary binaural voices.
         */
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
                    continue;   // 'off' tone-set

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
// Attribution / licensing (also add to CREDITS.md):
//
//   The preset voice data is based on sequences published with
//   SBaGen (Copyright (c) 1999-2007 Jim Peters, http://uazu.net/sbagen/,
//   released under the GNU GPL v2) and the sbagen-plus example
//   bundle (https://github.com/ruanklein/sbagen-plus, GNU GPL v2).
//   The carrier/beat values are community measurements and are
//   reproduced as factual data; all comment text here is written
//   for this project, no file text is copied verbatim. Do not
//   copy the original .sbg comments into these strings: GPLv2
//   ("version 2" only, no "or later") text cannot be mixed into
//   the GPLv3-licensed app.
//
// The Monroe 'Focus level' tone-sets are approximations based on
// community-measured values, are not Monroe Institute products,
//   and imply no affiliation with or endorsement by the Monroe
//   Institute.
//
// The Focus-level files originally contain only a single
// continuous tone-set ("0:00 ts"); a 30-minute session timeline
// has been added for use as an app preset.
//
// sbagen-plus '@' isochronic voices are approximated as binaural
// beats (see parseVoice).

QVector<SbgPreset> Sbg::builtinPresets()
{
    QVector<SbgPreset> presets;

    // ------------------------------------------------------------------
    // Monroe Institute Focus levels (community-measured tone-sets).
    // ------------------------------------------------------------------

    SbgPreset focus3;
    focus3.title = QStringLiteral("Focus 3 (Sleep)");
    focus3.source = QStringLiteral(
        "# F3: 59[1.2]-110[1.3], 288[3.7]\n"
        "ts: 59+1.2/10 110+1.3/10 288+3.7/10\n"
        "off: -\n"
        "NOW ts\n"
        "+00:30:00 off\n");
    presets.append(focus3);

    SbgPreset focus10;
    focus10.title = QStringLiteral("Focus 10 (Body Asleep)");
    focus10.source = QStringLiteral(
        "# F10 \"Brain awake, body asleep\":\n"
        "# 100[1.5], 200[4.0], 250[4.0], 300[4.0]\n"
        "ts: 100+1.5/10 200+4.0/10 250+4.0/10 300+4.0/10\n"
        "off: -\n"
        "NOW ts\n"
        "+00:30:00 off\n");
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
        "+00:30:00 off\n");
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
        "+00:30:00 off\n");
    presets.append(focus15);

    SbgPreset focus21;
    focus21.title = QStringLiteral("Focus 21 (Other Energy Systems)");
    focus21.source = QStringLiteral(
        "# F21:\n"
        "# 200[4.0], 250[4.0], 300[4.0],\n"
        "# 600[16.2], 750[15.9], 900[16.2]\n"
        "ts: 200+4.0/10 250+4.0/10 300+4.0/10 "
        "600+16.2/10 750+15.9/10 900+16.2/10\n"
        "off: -\n"
        "NOW ts\n"
        "+00:30:00 off\n");
    presets.append(focus21);

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

    return presets;
}
