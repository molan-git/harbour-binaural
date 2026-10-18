# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed


# The name of your application
TARGET = harbour-binaural


CONFIG += sailfishapp


# QtMultimedia for access to Sailfish's multimedia/audio APIs.
QT += multimedia


HEADERS += \
    src/audioengine.h 				\
    src/stmlib/stmlib.h				\
    src/stmlib/algorithms/note_stack.h		\
    src/stmlib/algorithms/pattern_predictor.h	\
    src/stmlib/algorithms/tiny_map.h		\
    src/stmlib/algorithms/voice_allocator.h	\
    src/stmlib/dsp/atan.h			\
    src/stmlib/dsp/cosine_oscillator.h		\
    src/stmlib/dsp/delay_line.h			\
    src/stmlib/dsp/dsp.h			\
    src/stmlib/dsp/filter.h			\
    src/stmlib/dsp/hysteresis_quantizer.h	\
    src/stmlib/dsp/limiter.h			\
    src/stmlib/dsp/parameter_interpolator.h	\
    src/stmlib/dsp/polyblep.h			\
    src/stmlib/dsp/rsqrt.h			\
    src/stmlib/dsp/sample_rate_converter.h	\
    src/stmlib/dsp/units.h			\
    src/stmlib/fft/shy_fft.h			\
    src/stmlib/midi/midi.h			\
    src/stmlib/utils/buffer_allocator.h		\
    src/stmlib/utils/crc32.h			\
    src/stmlib/utils/dsp.h			\
    src/stmlib/utils/gate_flags.h		\
    src/stmlib/utils/murmurhash3.h		\
    src/stmlib/utils/random.h			\
    src/stmlib/utils/ring_buffer.h		\
    src/stmlib/utils/stream_buffer.h		\
    src/braids/analog_oscillator.h		\
    src/braids/envelope.h			\
    src/braids/macro_oscillator.h		\
    src/braids/quantizer.h			\
    src/braids/resources.h			\
    src/braids/signature_waveshaper.h		\
    src/braids/vco_jitter_source.h		\
    src/braids/digital_oscillator.h		\
    src/braids/excitation.h			\
    src/braids/parameter_interpolation.h	\
    src/braids/quantizer_scales.h		\
    src/braids/settings.h			\
    src/braids/svf.h				\

SOURCES += src/binaural.cpp \
    src/audioengine.cpp \
    src/braids.h \
    src/braids/analog_oscillator.cc  \
    src/braids/digital_oscillator.cc  \
    src/braids/macro_oscillator.cc  \
    src/braids/quantizer.cc  \
    src/braids/resources.cc  \
    src/braids/settings.cc   \


DISTFILES += qml/harbour-binaural.qml \
    icons/216x216/harbour-binaural.png \
    qml/pages/CreditPage.qml \
    qml/pages/MainPage.qml \
    qml/cover/CoverPage.qml \
    qml/pages/AboutPage.qml \
    qml/pages/TimerPage.qml \
    qml/pages/components/InfoBinauralPage.qml \
    qml/pages/components/InfoColoredPage.qml \
    rpm/harbour-binaural.changes \
    rpm/harbour-binaural.changes.run.in \
    rpm/harbour-binaural.spec \
    qml/images/binaural-cover.svg \
    qml/images/icon-s-volume-1.svg \
    qml/images/icon-s-volume-2.svg \
    qml/images/icon-s-volume-3.svg \
    qml/images/icon-s-volume-4.svg \
    qml/images/icon-s-headset.svg \
    qml/images/image-binaural-graph.svg \
    qml/images/image-colornoise-graph.svg \
    translations/*.ts \
    harbour-binaural.desktop \
    sounds/wind.mp3 \
    sounds/crickets.mp3 \
    sounds/stream.mp3 \
    sounds/rain.mp3 \
    sounds/birds.mp3 \
    sounds/fire.mp3 \
    sounds/waves.mp3 \
    sounds/chimes.mp3



SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172


# To disable building translations every time, comment out the
# following CONFIG line.
CONFIG += sailfishapp_i18n


# German translation is enabled as an example.
TRANSLATIONS += translations/harbour-binaural-de.ts
TRANSLATIONS += translations/harbour-binaural-fr.ts
TRANSLATIONS += translations/harbour-binaural-it.ts
TRANSLATIONS += translations/harbour-binaural-sv.ts


# Install the audio files as application data.
#
# qmake's install system puts the files into the same
# directory that AudioEngine expects at runtime.
ambient.files = sounds/wind.mp3 \
                sounds/waves.mp3 \
                sounds/crickets.mp3 \
                sounds/stream.mp3 \
                sounds/rain.mp3 \
                sounds/birds.mp3 \
                sounds/fire.mp3 \
                sounds/chimes.mp3 \
                sounds/white.wav \
                sounds/pink.wav \
                sounds/brown.wav \
                sounds/grey.wav

ambient.path = /usr/share/harbour-binaural/sounds


INSTALLS += ambient
