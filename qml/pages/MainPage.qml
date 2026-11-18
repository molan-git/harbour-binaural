import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import Opal.Tabs 1.0
import Binaural 1.0
import "components"

Page {
    id: page

    property string activeBand: ""
    property string activeNoise: ""
    property var activeAmbiences: []
    property var activeSolfeggio: []
    property string activeIsochronic: ""
    property bool isCustomIsochronic: false
    property bool isCustomBinaural: false
    property real currentCarrier: 200
    property real currentBeat: 10
    property bool noiseWavesEnabled: false

    property bool bandPlaying: audioEngine.binauralPlaying
    property bool ambiencePlaying: audioEngine.ambiencePlaying
    property bool noisePlaying: audioEngine.coloredNoisePlaying
    property bool solfeggioPlaying: audioEngine.solfeggioPlaying
    property bool isochronicPlaying: audioEngine.isochronicPlaying

    property bool customSoundPlaying: audioEngine.customSoundPlaying

    property bool customSoundSelected:
        audioEngine.customSoundSource.toString() !== ""

    // Anything selected - used by menus, bar, cover, pausedMode, sleep timer
    property bool hasActiveSounds:
        activeBand !== "" ||
        activeNoise !== "" ||
        activeAmbiences.length > 0 ||
        activeSolfeggio.length > 0 ||
        activeIsochronic !== "" ||
        customSoundSelected

    // Mix label ("Alpha & Custom", "Rain & White noise", ...)
    property string mixLabel: {
        var parts = []

        if (activeBand !== "")
            parts.push(SoundNames.label(activeBand))

        if (activeNoise !== "")
            parts.push(SoundNames.label(activeNoise))

        if (activeSolfeggio.length > 0)
            parts.push(SoundNames.label("Solfeggio"))

        if (activeIsochronic !== "")
            parts.push(SoundNames.label("Isochronic"))

        if (activeAmbiences.length === 1)
            parts.push(SoundNames.label(activeAmbiences[0]))
        else if (activeAmbiences.length > 1)
            parts.push(SoundNames.label("Ambient mix"))

        if (customSoundSelected)
            parts.push(SoundNames.label("Custom sound"))

        return parts.join(", ")
    }

    property bool isPlaying: bandPlaying ||
                             ambiencePlaying ||
                             noisePlaying ||
                             solfeggioPlaying ||
                             isochronicPlaying ||
                             customSoundPlaying

    property bool pausedMode: !isPlaying && hasActiveSounds

    AudioEngine {
        id: audioEngine
    }

    onIsPlayingChanged: {
        appWindow.isPlaying = isPlaying
    }

    onActiveBandChanged: checkSleepTimer()
    onActiveAmbiencesChanged: checkSleepTimer()
    onActiveNoiseChanged: checkSleepTimer()
    onActiveSolfeggioChanged: checkSleepTimer()
    onActiveIsochronicChanged: checkSleepTimer()
    onCustomSoundSelectedChanged: checkSleepTimer()

    function checkSleepTimer() {
        if (appWindow.sleepTimerRunning && !page.hasActiveSounds)
            appWindow.cancelSleepTimer()
    }

    function fadeOutForSleepTimer() {
        audioEngine.fadeOutForSleepTimer()
    }

    function resumeAll() {
        if (activeBand !== "") {
            if (audioEngine.binauralPaused) {
                audioEngine.resumeBinaural()
            } else if (!audioEngine.binauralPlaying) {
                audioEngine.start()
            }
        }

        if (activeAmbiences.length > 0) {
            if (audioEngine.ambiencePaused)
                audioEngine.resumeAmbience()

            for (var i = 0; i < activeAmbiences.length; ++i)
                audioEngine.setAmbience(activeAmbiences[i])
        }

        if (audioEngine.customSoundSource.toString() !== "") {
            if (audioEngine.customSoundPaused) {
                audioEngine.resumeCustomSound()
            } else if (!audioEngine.customSoundPlaying) {
                audioEngine.playCustomSound()
            }
        }

        if (activeNoise !== "") {
            if (audioEngine.coloredNoisePaused) {
                audioEngine.resumeColoredNoise()
            } else if (!audioEngine.coloredNoisePlaying) {
                audioEngine.setColoredNoise(activeNoise)
            }
        }

        if (activeSolfeggio.length > 0) {
            if (audioEngine.solfeggioPaused) {
                audioEngine.resumeSolfeggio()
            } else if (!audioEngine.solfeggioPlaying) {
                audioEngine.setSolfeggio(activeSolfeggio)
            }
        }

        if (activeIsochronic !== "") {
            if (audioEngine.isochronicPaused) {
                audioEngine.resumeIsochronic()
            } else if (!audioEngine.isochronicPlaying) {
                audioEngine.startIsochronic()
            }
        }
    }

    function togglePlayback() {
        if (isPlaying) {
            audioEngine.pauseBinaural()
            audioEngine.pauseAmbience()
            audioEngine.pauseColoredNoise()
            audioEngine.pauseSolfeggio()
            audioEngine.pauseIsochronic()
            audioEngine.pauseCustomSound()
            appWindow.pauseSleepTimer()
        } else {
            resumeAll()
            appWindow.resumeSleepTimer()
        }
    }

    function deselectAllSounds() {
        audioEngine.stop()
        audioEngine.stopAmbience()
        audioEngine.stopColoredNoise()
        audioEngine.stopSolfeggio()
        audioEngine.stopIsochronic()
        audioEngine.stopCustomSound()
        noiseWaveDrawer.open = false
        solfeggioDrawer.open = false
        isochronicDrawer.open = false

        page.activeBand = ""
        page.isCustomBinaural = false
        page.activeAmbiences = []
        page.activeNoise = ""
        page.activeSolfeggio = []
        page.activeIsochronic = ""
        page.isCustomIsochronic = false

        audioEngine.customSoundSource = ""
        audioEngine.customSoundName = ""

        ambienceDrawer.open = false
        binauralDrawer.open = false
        soundPickerDrawer.open = false
    }

    function beatBand(freq) {
        if (freq >= 0.5  && freq < 4)    return "Delta"
        if (freq >= 4    && freq < 8)    return "Theta"
        if (freq >= 8    && freq < 13)   return "Alpha"
        if (freq >= 13   && freq < 30)   return "Beta"
        if (freq >= 30   && freq <= 100) return "Gamma"
        return ""
    }

    function bandBeat(band) {
        if (band === "Delta")      return 2.0
        if (band === "Delta/Theta") return 4.0
        if (band === "Theta")      return 6.0
        if (band === "Alpha")      return 10.0
        if (band === "Beta")       return 20.0
        if (band === "Gamma")      return 40.0
        return 10.0
    }

    Component {
        id: infoBinauralPageComponent
        InfoBinauralPage {}
    }

    Component {
        id: infoColoredPageComponent
        InfoColoredPage {}
    }

    Component {
        id: infoSolfeggioPageComponent
        InfoSolfeggioPage {}
    }

    Component {
        id: infoIsochronicPageComponent
        InfoIsochronicPage {}
    }

    PageDrawer {
        id: ambienceDrawer

        drawerContent: AmbienceDrawer {
            id: ambienceSelector

            activeAmbiences: page.activeAmbiences
            ambiencePlaying: page.ambiencePlaying
            pausedMode: page.pausedMode
            customSoundSelected: page.customSoundSelected

            onCloseRequested: ambienceDrawer.open = false

            onAmbienceSelected: {
                if (page.activeAmbiences.indexOf(ambience) !== -1)
                    return

                if (page.activeAmbiences.length
                        + (page.customSoundSelected ? 1 : 0) >= 3)
                    return

                var wasPaused = page.pausedMode
                page.activeAmbiences = page.activeAmbiences.concat([ambience])
                audioEngine.setAmbience(ambience)

                if (wasPaused)
                    audioEngine.pauseAmbience()
            }

            onAmbienceDeselected: {
                var updated = page.activeAmbiences.slice()
                var index = updated.indexOf(ambience)

                if (index === -1)
                    return

                updated.splice(index, 1)
                page.activeAmbiences = updated
                audioEngine.stopAmbienceByName(ambience)
            }

            onAmbienceVolumeChanged: {
                audioEngine.setAmbienceVolume(volume)
            }
        }

        PageDrawer {
            id: binauralDrawer

            drawerContent: CustomBinauralDrawer {
                id: customBinauralPanel

                onCloseRequested: {
                    binauralDrawer.open = false
                }

                onFrequencyChanged: {
                    page.currentCarrier = carrier
                    page.currentBeat = beat

                    // Live update of the running generator
                    // (or store for the next start).
                    audioEngine.setBinauralFrequency(carrier, beat)

                    if (page.activeBand !== "") {
                        // Band buttons follow the beat live:
                        // e.g. Alpha -> Beta starts glowing.
                        page.activeBand = page.beatBand(beat)
                        page.isCustomBinaural = true
                    }
                }
            }

            PageDrawer {
                id: noiseWaveDrawer

                drawerContent: NoiseWaveDrawer {
                    id: noiseWavePanel

                    wavesActive: page.noiseWavesEnabled
                    playing: page.noisePlaying

                    onCloseRequested: {
                        noiseWaveDrawer.open = false
                    }

                    onWaveChanged: {
                        if (page.noiseWavesEnabled) {
                            audioEngine.setNoiseWave(
                                        true, depth, period)
                        }
                    }

                    onEnableRequested: {
                        audioEngine.setNoiseWave(
                                    true, depth, period)
                        page.noiseWavesEnabled = true
                    }

                    onDisableRequested: {
                        audioEngine.setNoiseWave(
                                    false, 0, 8)
                        page.noiseWavesEnabled = false
                        noiseWaveDrawer.open = false
                    }
                }

                PageDrawer {
                    id: solfeggioDrawer

                    drawerContent: SolfeggioDrawer {
                        id: solfeggioPanel

                        activeSolfeggio: page.activeSolfeggio
                        solfeggioPlaying: page.solfeggioPlaying
                        pausedMode: page.pausedMode

                        onCloseRequested: {
                            solfeggioDrawer.open = false
                        }

                        onSolfeggioSelected: {
                            if (page.activeSolfeggio.indexOf(frequency) !== -1)
                                return

                            var wasPaused = page.pausedMode
                            page.activeSolfeggio =
                                    page.activeSolfeggio.concat([frequency])

                            if (wasPaused)
                                return   // stays paused; resumeAll() starts it

                            audioEngine.setSolfeggio(page.activeSolfeggio)
                        }

                        onSolfeggioDeselected: {
                            var updated = page.activeSolfeggio.slice()
                            var index = updated.indexOf(frequency)

                            if (index === -1)
                                return

                            updated.splice(index, 1)
                            page.activeSolfeggio = updated

                            // setSolfeggio() stops the channel on an empty list
                            audioEngine.setSolfeggio(updated)
                        }

                        onSolfeggioVolumeChanged: {
                            audioEngine.setSolfeggioVolume(volume)
                        }
                    }

                    PageDrawer {
                        id: isochronicDrawer

                        drawerContent: CustomIsochronicDrawer {
                            id: customIsochronicPanel

                            customActive: page.isCustomIsochronic
                            playing: page.isochronicPlaying

                            onCloseRequested: {
                                isochronicDrawer.open = false
                            }

                            onFrequencyChanged: {
                                if (page.isCustomIsochronic)
                                    audioEngine.setIsochronicFrequency(carrier, pulse)
                            }

                            onStartRequested: {
                                if (page.pausedMode) {
                                    audioEngine.stopIsochronic()
                                    page.activeIsochronic = "Custom"
                                    page.isCustomIsochronic = true
                                    audioEngine.setIsochronicFrequency(carrier, pulse)
                                } else {
                                    page.activeIsochronic = "Custom"
                                    page.isCustomIsochronic = true
                                    audioEngine.setIsochronicFrequency(carrier, pulse)
                                    audioEngine.startIsochronic()
                                }
                            }

                            onStopRequested: {
                                page.activeIsochronic = ""
                                page.isCustomIsochronic = false
                                audioEngine.stopIsochronic()
                            }
                        }

                        PageDrawer {
                            id: soundPickerDrawer

                            drawerContent: SoundPickerDrawer {
                                id: soundPickerPanel

                                audioEngine: audioEngine
                                pausedMode: page.pausedMode

                                onCloseRequested:
                                    soundPickerDrawer.open = false
                            }

                            // The container is the Drawer foreground.
                            // The dimmer and click handler move with the foreground,
                            // leaving the exposed drawer background interactive.
                            Item {
                                id: foregroundContent

                                anchors.fill: parent

                                // Hosts the pulley menu and the page header
                                // outside the tabs, so they are shared by both
                                // tab pages and the menu works everywhere.
                                SilicaFlickable {
                                    id: foregroundFlick
                                    anchors.fill: parent
                                    contentHeight: height

                                    PullDownMenu {
                                        MenuItem {
                                            text: qsTr("About")
                                            onClicked: {
                                                pageStack.push(
                                                            Qt.resolvedUrl("AboutPage.qml")
                                                            )
                                            }
                                        }

                                        MenuItem {
                                            text: qsTr("Deselect all sounds")
                                            enabled: page.hasActiveSounds
                                            onClicked: {
                                                page.deselectAllSounds()
                                            }
                                        }

                                        MenuItem {
                                            text: qsTr("Timer")
                                            enabled: page.hasActiveSounds
                                            onClicked: {
                                                pageStack.push(
                                                            Qt.resolvedUrl("TimerPage.qml"),
                                                            { appWindow: appWindow }
                                                            )
                                            }
                                        }
                                    }

                                    Item {
                                        id: topSpacer
                                        width: parent.width
                                        height: Theme.paddingLarge
                                    }

                                    TabView {
                                        anchors.top: topSpacer.bottom
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.bottom: parent.bottom

                                        tabBarPosition: Qt.AlignTop

                                        // ─── STANDARD TAB ──────────────────────────

                                        Tab {
                                            title: qsTr("Standard")

                                            Component {
                                                TabItem {
                                                    flickable: standardTab

                                                    StandardTab {
                                                        id: standardTab

                                                        anchors.fill: parent

                                                        mainPage: page
                                                        engine: audioEngine

                                                        onOpenCustomBinauralRequested: {
                                                            customBinauralPanel.setValues(
                                                                        page.currentCarrier,
                                                                        page.currentBeat
                                                                        )

                                                            binauralDrawer.open = true
                                                        }

                                                        onOpenNoiseWaveRequested:
                                                            noiseWaveDrawer.open = true

                                                        onOpenAmbienceRequested:
                                                            ambienceDrawer.open = true

                                                        onOpenSoundPickerRequested:
                                                            soundPickerDrawer.open = true

                                                        onBinauralInfoRequested:
                                                            pageStack.push(infoBinauralPageComponent)

                                                        onColoredNoiseInfoRequested:
                                                            pageStack.push(infoColoredPageComponent)
                                                    }
                                                }
                                            }
                                        }

                                        // ─── EXTENDED TAB ──────────────────────────

                                        Tab {
                                            title: qsTr("Extended")

                                            Component {
                                                TabItem {
                                                    flickable: extendedTab

                                                    ExtendedTab {
                                                        id: extendedTab

                                                        anchors.fill: parent

                                                        mainPage: page
                                                        engine: audioEngine

                                                        customCarrierHz: customIsochronicPanel.carrierHz
                                                        customPulseHz: customIsochronicPanel.pulseHz

                                                        onOpenSolfeggioDrawerRequested:
                                                            solfeggioDrawer.open = true

                                                        onOpenIsochronicDrawerRequested:
                                                            isochronicDrawer.open = true

                                                        onSolfeggioInfoRequested:
                                                            pageStack.push(infoSolfeggioPageComponent)

                                                        onIsochronicInfoRequested:
                                                            pageStack.push(infoIsochronicPageComponent)
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
