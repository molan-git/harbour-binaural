# Binaural <img width="50" title="Page Home" src="https://github.com/molan-git/harbour-binaural/blob/main/icons/172x172/harbour-binaural.png">

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/G7W527ZZR9)

## About

Binaural is a sound app for [Sailfish OS](https://sailfishos.org) designed for relaxation, focus, sleep, and meditation. Create your own listening experience with binaural beats, colored noise, and ambient sounds. Mix different sounds together, adjust their volume levels, and find the combination that works best for you.

## Features

- Binaural beats with selectable frequency bands
- Colored noise: white, pink, brown, and grey
- Ambient sounds
- Mix binaural beats, noise, and ambient sounds
- Sleep timer (WIP)

## Download

Available on [OpenRepos.net](https://openrepos.net/content/molan/binaural)

## Audio Engine

The `AudioEngine` generates binaural beats in real time, sample-by-sample, using two sine waves: a fixed 200 Hz tone on the left channel and a slightly higher frequency on the right. The frequency difference creates the perceived binaural beat. The generated stereo 16-bit PCM audio is streamed through **`QAudioOutput`**, with the sample rate adapted to the audio device.

Colored noise and ambient sounds are provided as audio files and played using **`QMediaPlayer`**. They can be combined with the binaural signal, with ambient sounds supporting fade-in, fade-out, and crossfading.

## Requirements

- Sailfish OS 4.6 or newer
- Sailfish OS 4.6 SDK build target or newer

## Build

Clone or download this repository and import it into your Sailfish OS IDE using the `harbour-binaural.pro` project file. No additional configuration is required.

## Repository branches

- `master`: Release branch containing the current version of Binaural.
- `feature-*`: Branches used for testing features under development. 

## Contributions

Contributions to this project are very welcome. If you already know what you want to add or fix, please open a `Pull Request` with your proposal. You can also use `Discussions` to propose features or changes.

Please include an explanation of the changes or a brief changelog summary. Pull requests will be reviewed before they are merged. Please note that not all contributions may be accepted if they conflict with the project's philosophy, simplicity, or overall direction.

## Translations

Translations are managed with [Hosted Weblate](https://hosted.weblate.org/projects/harbour-binaural/). New languages can be added through `Weblate` or directly via `GitHub`. Incomplete translations may not be accepted or included in the app.

[![Translations](https://hosted.weblate.org/widgets/harbour-binaural/-/svg-badge.svg)](https://hosted.weblate.org/engage/harbour-binaural/)

## Screenshots

TBA

## Credits

* Inspired by [Metiq](https://github.com/metiq-xyz/android-app) for Android
* Colored noise generated with the [Metiq colored noise generator](https://github.com/metiq-xyz/colored-noise-generator)
* Audio and image asset credits: see [CREDITS.md](CREDITS.md)

## License

Licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.html).
