# <img src="../assets/screenshots/composite_spwx_title.png?raw=true"/>

The **sidplaywx** is a GUI player for Commodore 64 SID chip tunes aiming to provide a modern & comfortable SID tune playback experience on the PC.

The current alpha version is fully usable, supporting QoL features like seeking, drag & drop, unicode paths, DPI awareness and much more.

The **sidplaywx** uses [libsidplayfp](https://github.com/libsidplayfp/libsidplayfp) for ultimate quality in SID emulation, [wxWidgets](https://github.com/wxWidgets/wxWidgets) for native GUI on supported platforms, and [PortAudio](https://github.com/PortAudio/portaudio) for audio output.

## Screenshots
![Screenshot of the player application main window](../assets/screenshots/sidplaywx-player.png?raw=true)

![Screenshot of the Modify Playback window](../assets/screenshots/sidplaywx-playbackmod.png?raw=true)

![Screenshot of the Preferences window](../assets/screenshots/sidplaywx-preferences-long.png?raw=true)

## Platforms
This project uses the GCC and the CMake, and at the moment Windows and Linux are supported.

## Planned features for v1.0
The current version of the sidplaywx is 0.x.x (alpha), so in addition to bugfixes and common sense updates, I consider at least _these_ features are needed before the sidplaywx can graduate to version 1.0:
- Playlist improvements such as <del>duration columns</del>, reordering<del>, remembering last state and playlist file save/load</del> etc.
- Misc. necessary features <del>such as remembering window size & position</del> etc.
- Exporting tunes to WAV
- [<del>STIL</del>](https://www.hvsc.c64.org/download/C64Music/DOCUMENTS/STIL.txt) <del>support for displaying tune comments</del>
- <del>Native Linux support</del>
- <del>Theming support / dark theme</del>

## Download
* You can find a pre-built ready-to-use binary distribution(s) in the [Releases](https://github.com/bytespiller/sidplaywx/releases) sidebar.
	* More package formats are planned for Linux (only AppImage at the moment).

## Alternatives
Some alternatives to sidplaywx for playing the SID tunes I've tried and liked are:
* Windows: foobar2000 + foo_sid plugin
* Linux: DeaDBeeF player (note: it uses older/no longer maintained libsidplay2)
* Mac: I don't have a Mac so I haven't tried it, but this looks great: https://github.com/Alexco500/sidplay5

## Contributing, ideas, comments, issues
If you have an idea or a comment, feel free to post it in the [Discussions](https://github.com/bytespiller/sidplaywx/discussions). Issues can be reported [here](https://github.com/bytespiller/sidplaywx/issues). There is also an email address provided in the application's Help > About box.

## FAQ
* Where can I get SID tunes?
  * Check out the [High Voltage SID Collection](https://www.hvsc.c64.org) for a massive collection of SID tunes sorted by authors, and more!
* Why are some tunes crossed-out and cannot be played?
  * Small number of tunes require a C64 system ROM to play. You can find the C64 system ROM files in e.g., open source C64 emulators (or elsewhere on the internet) and import them via the sidplaywx's Preferences.
  * There are three main C64 system ROM types that the sidplaywx supports (and some tunes require):
    * KERNAL ROM (tune indicated with a RED crossout text if missing)
    * BASIC ROM (tune indicated with a BLUE crossout text if missing)
    * _CHARGEN ROM (you'll probably never need this one, so let's just ignore it for now)_
* When drag & dropping the files, why the playlist sometimes gets cleared?
  * By default, the sidplaywx will enqueue the files if dropped onto the playlist area, and clear (replace) the playlist if they are dropped onto the general player window area. This is configurable in the Preferences.
* Can sidplaywx open archive files?
  * Yes, but only the Zip format in its simplest variant is supported due to wxZip limitation.
* How come the seeking is so slow?
  * There is an "Instant seeking" option available which pre-renders the entire SID tune in the background. It is disabled by default, but if you enable it you will be able to seek instantly. See [release notes](https://github.com/bytespiller/sidplaywx/releases/tag/v0.7.0-beta) of the old release (note: "Instant seeking" was formerly named "Fast seeking") for details on how it works and what are the caveats.
  * SID tunes are actually small programs and not audio files like for example the MP3, so they have to be emulated linearly as fast as possible until the "seek" target is reached.
* Command-line options in sidplaywx?
  * Focus so far is on the GUI experience. There exists a [sidplayfp](https://github.com/libsidplayfp/sidplayfp/releases) console-based player in the libsidplayfp repo (not to be confused with a sidplaywx which is unrelated and unaffiliated project).
  * For now, you can pass the space-separated filenames. For example `sidplaywx Aces_High.sid Gunstar.sid` to open those two tunes. This "feature" is actually a side-effect of single-instance support, so there will be a slight delay if the app is already running and the single-instance option is enabled.

## Building
<details>
  <summary>Click to expand!</summary>

### Linux

#### Prerequisites
- GCC version with C++17 support is required (minimum I've tried is gcc-12).
- Don't forget the `sudo apt-get update` and `sudo apt-get install build-essential` as well as `sudo apt-get install cmake`

##### (libsidplayfp)
* **NOTE:** Building the libsidplayfp from its git master branch is more involved and not covered here. This guide assumes you're building one of the [source releases](https://github.com/libsidplayfp/libsidplayfp/releases) of the libsidplayfp which is simpler.
1. To enable C++20 set in the terminal `CXXFLAGS="$CXXFLAGS -std=c++20"`
2. Commands to build statically: `./configure LDFLAGS="-static" && make`
3. Copy the following *includes* (with their folder structures) to the appropriate `include` folder in the sidplaywx's `/deps/`:
	- `/builders/residfp.h`
	- `sidbuilder.h`, `SidConfig.h`, `siddefs.h`, `SidInfo.h`, `sidplayfp.h`, `SidTune.h`, `SidTuneInfo.h`, `sidversion.h`
4. Copy the `/src/.libs/libsidplayfp.a` (`.libs` is a *hidden* folder) to the appropriate `lib` folder in the sidplaywx's `/deps/`

##### (PortAudio)
0. Prerequisites: you must have installed the `libpulse-dev`, ALSA (`libasound2-dev`), `libsndio-dev`, `libjack-dev` BEFORE building the PortAudio, otherwise the resultant PortAudio may not find any devices.
	- See the main CMakeLists.txt for which ones are relevant.
1. `cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B dist && cmake --build dist`
	- Note: I had to use the git master version, had no luck with the stable version on the Xubuntu 24.04
2. Copy files from `/dist/include/` folder & the `libportaudio.a` file to the appropriate sidplaywx's `/deps/` folders.

##### (wxWidgets)
0. Prerequisites: if needed, install the `libgtk-3-dev` and `libcurl-dev` (the last one is needed for our "Check for updates" function).
1. `cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B dist && cmake --build dist`
	- Exceptional cases (if you get an `#include` error when building the sidplaywx):
		- You may need to call the `wx-config --cxxflags` to get necessary flags and update the equivalent variable in our sidplaywx CMakeLists.txt
		- May also be of interest:
			- `wx-config --libs`
			- https://wiki.wxwidgets.org/Wx-Config
			- https://docs.wxwidgets.org/3.2/overview_cmake.html
2. Copy files from `/dist/include/` & `/dist/lib/` to the appropriate sidplaywx's `/deps/` folders.

#### Building the sidplaywx
1. Copy contents of the `dev` folder (except the `icon_src` folder and the `SystemColorViewer.pyw`) to the `build` folder (create a `build` folder next to the `dev` folder)
2. You can rename the `CMakeLists_linux.txt` to `CMakeLists.txt`
3. `cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build`
- Tip: If the sidplaywx fails to launch after it was successfully built, try via terminal to see if something is missing.
- Tip: if trying to execute the sidplaywx in VSCode and getting exit code 177, you need to edit your config: `"terminal.integrated.env.linux": { "GTK_PATH": null }` and restart VSCode (or for one-off thing run this in VS terminal: `unset GTK_PATH`)

***

### Windows (10, 11)

#### Prerequisites
At the moment, the easiest way to build the sidplaywx on Windows is probably by using the [MSYS2](https://www.msys2.org/) environment.<br>
Note: you should install it into the default `C:\msys64\` path in order for some hardcoded cmake paths to work out-of-the-box.

Once installed you need to **first-time configure** it like so:
- **NOTE:** MSYS2 supports modern UCRT64 and legacy MINGW environments. UCRT64 is recommended these days, and these instructions assume using it.
1. Update package lists etc.: `pacman -Syu` and after restart (if needed): `pacman -Su`
2. Install develpment prerequisites: `pacman -S base-devel`
3. Install msvcrt-compatible gcc: `pacman -S mingw-w64-ucrt-x86_64-gcc`
4. Install cmake & make tools (e.g., PortAudio needs those): `pacman -S mingw-w64-ucrt-x86_64-cmake && pacman -S mingw-w64-ucrt-x86_64-make`
5. In your PATH environment variable add: "C:\msys64\ucrt64\bin"
- Extra if you need to install gdb separately for some reason: `pacman -S mingw-w64-ucrt-x86_64-gdb`
- IMPORTANT: run the terminal via **ucrt64.exe**, **not** default msys2 terminal (otherwise the proper gcc variant might not be used)!

##### (libsidplayfp)
* **NOTE:** Building the libsidplayfp from its git master branch is more involved and not covered here. This guide assumes you're building one of the [source releases](https://github.com/libsidplayfp/libsidplayfp/releases) of the libsidplayfp which is simpler.
1. `cd` (with MSYS2 i.e., UCRT64.exe terminal) into the libsidplayfp's root.
2. To specify either:
	1. C++20 – set this in the terminal: `CXXFLAGS="$CXXFLAGS -std=c++20"` (new)
	2. C++14 – set this in the terminal: `CXXFLAGS="$CXXFLAGS -std=c++0x"` (older versions of libsidplayfp)
3. Finally, run: `./configure LDFLAGS="-static" && make && make install`
	1. TIP: if doing this for the first time, break down these 3 commands (i.e., they are separated by `&&`) and run them one by one so you can catch any problems more easily.
4. Note: the lib will be automatically found by our cmake later (if you've installed the msys into the `C:\msys64\`) and it will get linked statically.
	
##### (PortAudio)
1. [Download](http://files.portaudio.com/download.html) the PortAudio stable source release or the master [from the github](https://github.com/PortAudio/portaudio).
2. You can use the regular Windows cmd:
	1. `cd` into the PortAudio's root.
	2. `cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release`
	3. `mingw32-make`
3. Copy files from `/dist/include/` folder & the `libportaudio.a` file to the appropriate sidplaywx's `/deps/` folders.
	
##### (wxWidgets)
1. Simply [download](https://www.wxwidgets.org/downloads/) the appropriate pre-built binaries for your compiler (e.g., GCC v14).
2. Copy headers & libs to the appropriate sidplaywx's `/deps/` folders.
3. IMPORTANT: additionally, in order to actually run the sidplaywx application after it's built, you need to copy the following wxWidgets' `.dll` files into the sidplaywx's **build** folder: `wxbaseVER_SUFFIX.dll`, `wxbaseVER_xml_SUFFIX.dll`, `wxmswVER_core_SUFFIX.dll`, `wxmswVER_propgrid_SUFFIX.dll` (the exact `VER` version and `_SUFFIX` suffix differs depending on wxWidgets & gcc version etc.).
4. TIP: You can also build the wxWidgets yourself in the similar manner to building the PortAudio (in case you want to use a specific GCC version not offered among pre-built binaries).

#### Building the sidplaywx
1. The main `CMakeLists.txt` should do the trick (I myself use the Visual Studio Code).
2. IMPORTANT: additionally, in order to actually run the sidplaywx application after it's built, you need to copy the following files into the sidplaywx's **build** folder:
	1. `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll` found in your appropriate MSYS bin folders (e.g., ucrt64 or mingw64).
	2. The entire `dev\theme` folder (so you end up with `build\theme`).
	3. The `dev\bundled-Songlengths.md5` file (so you end up with `build\bundled-Songlengths.md5`).
	4. The `dev\bundled-STIL.txt` file (so you end up with `build\bundled-STIL.txt`).
	- Tip: you can see the [release](https://github.com/bytespiller/sidplaywx/releases) package for example of bundled dependency files if you get stuck.
