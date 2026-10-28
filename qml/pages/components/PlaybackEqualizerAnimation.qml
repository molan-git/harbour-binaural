import QtQuick 2.6
import Sailfish.Silica 1.0

/*
 * Playing indicator: 3 vertical bars, 4 independent animation looks.
 *
 *   - Animations NEVER touch bar height directly. They only animate
 *     scalar "phase" properties (spike / wavePhase / wobble).
 *   - Bar height is always computed by activeBarHeight(mode, index)
 *   - NOT-PLAYING / fade: a single "fade" factor (0 = idle, 1 = active)
 *     blends the computed height toward idleHeight. fade only changes
 *     on play/stop, so a Behavior is safe on it. The per-frame animated
 *     heights are NEVER intercepted -> smooth.
 *
 * MODES:
 *   0 = default        : 3 bars spike up/down synchronously,
 *   1 = binaural       : smooth wave travelling across the 3 bars
 *   2 = colored noise  : 3 bars wobble randomly, all capped
 *   3 = binaural+noise : a travelling wave that additionally wobbles
 *
 * Anything other than these exact states falls back to mode 0.
 * Not playing -> bars fade smoothly to idle height.
 */

Item {
    id: playingIndicator

    /* ---------------------------------------------------------------
     * EXTERNAL INTERFACE
     * --------------------------------------------------------------- */

    property var applicationWindow                      // app window, provides isPlaying/pausedMode/activeBand/activeNoise
    property color barColor: Theme.highlightColor       // color of the 3 bars
    property real sizeMultiplier: 1.0                   // overall scale of the whole indicator

    property var app: applicationWindow

    property bool playing: app && app.isPlaying && !app.pausedMode
    property bool binauralActive: app && app.activeBand !== ""
    property bool noiseActive: app && app.activeNoise !== ""

    // "anything else" = any ambience playing (alone or combined with anything)
    property bool ambienceActive: app && app.activeAmbiences.length > 0

    /* ---------------------------------------------------------------
     * MODE SELECTION
     * Mode 1: ONLY binaural alone
     * Mode 2: ONLY colored noise alone
     * Mode 3: ONLY binaural + colored noise together
     * An ambience playing — by itself or combined with binaural/noise
     * — always means mode 0 (default spikes).
     * --------------------------------------------------------------- */
    property int animationMode: {
        if (!playing || !app) return 0;                 // not playing -> default
        if (ambienceActive) return 0;                   // any ambience involved -> default
        if (binauralActive && noiseActive) return 3;    // exactly binaural + colored noise
        if (binauralActive) return 1;                   // only binaural
        if (noiseActive) return 2;                      // only colored noise
        return 0;                                       // everything else -> default
    }

    /* ---------------------------------------------------------------
     * GEOMETRY (shared by all modes)
     * --------------------------------------------------------------- */

    height: Math.min(
                Theme.iconSizeSmall * sizeMultiplier,
                parent ? parent.height * 0.7 : Theme.iconSizeSmall * sizeMultiplier
            )

    property real barWidth: Math.max(2, Theme.iconSizeSmall * sizeMultiplier / 6) // thickness of one bar
    property real barGap: barWidth        // horizontal gap between bars
    property real idleHeight: height * 0.15 // height at rest (bars shrink to this when not playing)
    property real maxBarHeight: height * 0.9 // 100% height = top cap for every mode

    width: barWidth * 3 + barGap * 2

    /* ---------------------------------------------------------------
     * FADE (stop / resume)
     * fade = 1  -> bars show their fully animated height
     * fade = 0  -> bars rest at idleHeight
     * It ONLY changes on play/stop, so the Behavior below is safe.
     * --------------------------------------------------------------- */

    property real fadeDuration: 400        // [ms] fade-out on stop / fade-in on resume. 0 = instant.

    property real fade: playing ? 1.0 : 0.0

    Behavior on fade {
        NumberAnimation {
            duration: fadeDuration
            easing.type: Easing.OutQuad
        }
    }

    /* ===============================================================
     * SECTION: MODE 0 CONFIG
     * ===============================================================
     * Look: all 3 bars spike up together and come back down together.
     * The center bar (index 1) always reaches 100% of maxBarHeight,
     * the two side bars only mode0SideBarFactor of it.
     * --------------------------------------------------------------- */

    property real mode0UpDuration: 400              // [ms] time of the upward spike (bigger = lazier spike)
    property real mode0DownDuration: 400            // [ms] time of falling back (bigger = lazier fall)
    property real mode0UpEasing: Easing.OutQuad     // easing while rising
    property real mode0DownEasing: Easing.InQuad    // easing while falling
    property real mode0SideBarFactor: 0.7           // [0..1] side bars reach only given amount (1.0 = all bars equal height)

    property real spike: 0   // animated phase 0..1, driven by Mode 0 animation only

    // === ANIMATION 0 (independent, runs only while playing in mode 0) ===
    SequentialAnimation {
        running: playing && animationMode === 0
        loops: Animation.Infinite
        NumberAnimation {
            target: playingIndicator; property: "spike"
            from: 0; to: 1
            duration: mode0UpDuration; easing.type: mode0UpEasing
        }
        NumberAnimation {
            target: playingIndicator; property: "spike"
            from: 1; to: 0
            duration: mode0DownDuration; easing.type: mode0DownEasing
        }
    }

    /* ===============================================================
     * SECTION: MODE 1 CONFIG — binaural wave
     * ===============================================================
     * Look: a smooth sine wave travels across the 3 bars.
     * Bars move up AND down around idleHeight like a wave.
     * --------------------------------------------------------------- */

    property real mode1Duration: 1000       // [ms] duration of one full wave cycle (bigger = slower wave)
    property real mode1WaveSpread: 1.5      // [rad] phase offset between neighbouring bars
                                            //   (bigger = more "diagonal" wave, 0 = all bars move in sync)

    property real wavePhase: 0              // animated phase 0..2*PI, driven by Mode 1 animation only

    // === ANIMATION 1 (independent, runs in modes 1 and 3) ===
    NumberAnimation {
        target: playingIndicator; property: "wavePhase"
        from: 0; to: 2 * Math.PI
        duration: mode1Duration; loops: Animation.Infinite
        running: playing && (animationMode === 1 || animationMode === 3)
    }

    /* ===============================================================
     * SECTION: MODE 2 CONFIG — colored noise wobble
     * ===============================================================
     * Look: all 3 bars jitter randomly between idleHeight and
     * mode2Cap of the available amplitude (never higher).
     * --------------------------------------------------------------- */

    property real mode2Cap: 0.6            // [0..1] bars grow to max 70% of the amplitude
    property real mode2Step1Duration: 120  // [ms] duration of the first random step (bigger = slower jitter)
    property real mode2Step2Duration: 170  // [ms] duration of the second random step
    property real mode2Pause: 30           // [ms] pause between jitter cycles (bigger = twitchier)
    property real mode2Frequency: 10       // wobble sine frequency (bigger = wilder jumps between steps)
    property real mode2BarOffset: 2.1      // per-bar phase offset (bigger = bars wobble more out of sync)

    property real wobble: 0                 // animated random value 0..1, driven by Mode 2 animation only

    // === ANIMATION 2 (independent, runs in modes 2 and 3) ===
    SequentialAnimation {
        running: playing && (animationMode === 2 || animationMode === 3)
        loops: Animation.Infinite
        NumberAnimation {
            target: playingIndicator; property: "wobble"
            to: Math.random(); duration: mode2Step1Duration; easing.type: Easing.OutQuad
        }
        NumberAnimation {
            target: playingIndicator; property: "wobble"
            to: Math.random(); duration: mode2Step2Duration; easing.type: Easing.InQuad
        }
        PauseAnimation { duration: mode2Pause }
    }

    /* ===============================================================
     * SECTION: MODE 3 CONFIG — wobbling wave (binaural + noise)
     * ===============================================================
     * Look: the Mode 1 wave, with the Mode 2 wobble ADDED on top.
     * Wave settings (speed/spread) come from the MODE 1 section,
     * wobble speed/frequency from the MODE 2 section — those two
     * animations run unchanged. Only the mixing happens here.
     * --------------------------------------------------------------- */

    property real mode3WobbleAmount: 0.4    // [0..1] how strongly the wobble is added on top of the wave
                                            //   (0 = pure wave, 1 = wobble dominates the wave)

    /* ===============================================================
     * HEIGHT COMPUTATION — do not add animation code below this line.
     * Each case only reads the config properties of its own section.
     * =============================================================== */

    // Fully animated height of a bar, from the phase values.
    function activeBarHeight(mode, i) {
        var amp = maxBarHeight - idleHeight  // usable amplitude all modes share
        var t = 0                            // normalized height factor 0..1

        switch (mode) {

        // ---- mode 0: bars follow the spike phase, sides capped ----
        case 0:
            t = spike
            var cap = (i === 1) ? 1.0 : mode0SideBarFactor  // center full, sides 80%
            return idleHeight + amp * cap * t

        // ---- mode 1: travelling sine wave ----
        case 1:
            t = 0.5 + 0.5 * Math.sin(wavePhase - i * mode1WaveSpread)
            return idleHeight + amp * t

        // ---- mode 2: random wobble, capped at mode2Cap ----
        case 2:
            t = 0.5 + 0.5 * Math.sin(wobble * mode2Frequency + i * mode2BarOffset)
            return idleHeight + amp * mode2Cap * t

        // ---- mode 3: wave + wobble, clamped to full range ----
        case 3:
            t = 0.5 + 0.5 * Math.sin(wavePhase - i * mode1WaveSpread)       // wave part (MODE 1)
                + mode3WobbleAmount * Math.sin(wobble * mode2Frequency
                                               + i * mode2BarOffset)        // wobble part (MODE 2)
            if (t < 0) t = 0
            if (t > 1) t = 1
            return idleHeight + amp * t
        }

        // any unknown mode -> rest at idle height
        return idleHeight
    }

    // Final displayed height: animated height blended toward idle by "fade".
    // fade -> 0 on stop (smooth shrink), fade -> 1 on play (bars wake up).
    function displayedBarHeight(mode, i) {
        var h = activeBarHeight(mode, i)
        return idleHeight + (h - idleHeight) * fade
    }

    /* ===============================================================
     * VISUALS — the 3 bars. Height is always derived, never
     * animated directly. No Behavior on height (that would choke
     * the per-frame animation); the fade lives in "fade" only.
     * =============================================================== */

    Repeater {
        model: 3

        Rectangle {
            width: playingIndicator.barWidth
            radius: width / 2                                   // rounded bar ends
            x: index * (playingIndicator.barWidth + playingIndicator.barGap)
            anchors.verticalCenter: parent.verticalCenter
            color: playingIndicator.barColor

            height: playingIndicator.displayedBarHeight(playingIndicator.animationMode, index)
        }
    }
}
