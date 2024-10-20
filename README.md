# <img align="left" src="../assets/screenshots/sidplaywx_icon_64.png?raw=true"/> sidplaywx
The **sidplaywx** is a GUI player for Commodore 64 SID chip tunes aiming to provide a modern & comfortable SID tune playback experience on the PC.

The current alpha version is fully usable, supporting QoL features like seeking, drag & drop, unicode paths, DPI awareness and much more.

The **sidplaywx** uses [libsidplayfp](https://github.com/libsidplayfp/libsidplayfp) for ultimate quality in SID emulation, [wxWidgets](https://github.com/wxWidgets/wxWidgets) for native GUI on supported platforms, and [PortAudio](https://github.com/PortAudio/portaudio) for audio output.

## Screenshots
|Main window|
|-|
|<p align="center">![Screenshot of the player application main window](../assets/screenshots/sidplaywx-player.png?raw=true)</p>|
|Pictured: for tunes with subsongs, a _crown_ icon in the playlist indicates a default subsong. You may also encounter other indicators such as a _timer_ icon indicating an (optional) auto-skipping of (sub)songs shorter than (n) seconds, or a _chip_ (indicates a ROM file requirement).<br>Tunes can be seeked ([HVSC](https://www.hvsc.c64.org) *Songlengths.md5* database is supported, and default/fallback duration can be specified in the Preferences).<br>HVSC STIL info is also supported (blue text in the top right).|

<br>

|Modify Playback window|
|-|
|<p align="center">![Screenshot of the Modify Playback window](../assets/screenshots/sidplaywx-playbackmod.png?raw=true)</p>|
|Modification of ongoing playback: toggling individual SID voices (the active SIDs are indicated -- most normal tunes use a single SID chip, but there exist special stereo/multi-SID tunes which will use up to three). Realtime toggles to inhibit the filter and digi samples are also available. <br>Playback speed (not tempo) can also be modified (pitch is affected).<br>If any of these settings are changed, and they affect the currently playing tune, there will be a "MODIFIED" indication in the status bar.|

<br>

|Preferences window|
|-|
|<p align="center">![Screenshot of the Preferences window](../assets/screenshots/sidplaywx-preferences-long.png?raw=true)</p>|
|Each setting in the Preferences is accompanied with a help-text displayed in the bottom area. This way the Preferences also serves as a documentation/help source for sidplaywx's many features.|

## Platforms
While the project uses GCC and CMake, at the moment the focus is on the Windows version. A proper Linux support is planned before the v1.0 release.

## Planned features for v1.0
The current version of the sidplaywx is 0.x.x, so in addition to bugfixes and common sense updates, I consider at least _these_ features are needed before the sidplaywx can graduate to version 1.0:
- Playlist improvements such as <del>duration columns</del>, reordering<del>, remembering last state and playlist file save/load</del> etc.
- Misc. necessary features such as remembering window size & position etc.
- Exporting tunes to WAV
- [<del>STIL</del>](https://www.hvsc.c64.org/download/C64Music/DOCUMENTS/STIL.txt) <del>support for displaying tune comments</del>
- Native Linux support
  - Side note: while it currently works fine under Wine, there are some glitches such as STIL not being displayed (due to a layout problem) etc.
- Theming support / dark theme

## Download
* You can find a pre-built ready-to-use binary distribution(s) in the [Releases](https://github.com/bytespiller/sidplaywx/releases) sidebar.

## Alternatives
Some alternatives to sidplaywx for playing the SID tunes I've tried and liked are:
* Windows: foobar2000 + foo_sid plugin
* Linux: DeaDBeeF player (note: it uses older/no longer maintained libsidplay2)

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
    * <details>
        <summary>More details</summary>
        The libsidplayfp library (which sidplaywx uses) focuses on accuracy so it's much slower than e.g., libsidplay2 (which is virtually instantaneous, try it in the DeaDBeeF player on the Linux!). FWIW the seeking in the sidplaywx is already separately threaded and bypasses some SID mixing steps, audio rendering etc. so I think it's as fast as possible at the moment, unless you enable the "Instant seeking" option of course).
      </details>
* Command-line options in sidplaywx?
  * Focus so far is on the GUI experience. There exists a [sidplayfp](https://github.com/libsidplayfp/sidplayfp/releases) console-based player in the libsidplayfp repo (not to be confused with a sidplaywx which is unrelated and unaffiliated project).
  * For now, you can pass the space-separated filenames. For example `sidplaywx Aces_High.sid Gunstar.sid` to open those two tunes. This "feature" is actually a side-effect of single-instance support, so there will be a slight delay if the app is already running and the single-instance option is enabled.

## Building
<details>
  <summary>Click to expand!</summary>

### Windows (7, 10, 11)
At the moment, the easiest way to build the sidplaywx on Windows is probably by using the [MSYS2](https://www.msys2.org/) environment.<br>
Note: you should install it into the default `C:\msys64\` path in order for hardcoded cmake paths to work out-of-the-box.

Once installed you need to **first-time configure** it like so:
* **NOTE:** MSYS2 supports modern UCRT64 and legacy MINGW environments. UCRT64 is recommended these days.
* Update package lists etc.: `pacman -Syu` and after restart (if needed): `pacman -Su`
* Prerequisites: `pacman -S base-devel`
* msvcrt-compatible gcc:
  * UCRT64: `pacman -S mingw-w64-ucrt-x86_64-gcc`
  * MINGW: `pacman -S mingw-w64-x86_64-gcc`
* Tools (e.g., PortAudio needs those):
	 * UCRT64: `pacman -S mingw-w64-ucrt-x86_64-cmake && pacman -S mingw-w64-ucrt-x86_64-make`
	 * MINGW: `pacman -S mingw-w64-x86_64-cmake && pacman -S mingw-w64-x86_64-make`
* Extra if you need to install gdb separately for some reason:
	 * UCRT64: `pacman -S mingw-w64-ucrt-x86_64-gdb`
	 * MINGW: `pacman -S mingw-w64-x86_64-gdb`
* In your PATH environment variable add either the:
	 * UCRT64: "C:\msys64\ucrt64\bin"
	 * MINGW: "C:\msys64\mingw64\bin"
* IMPORTANT: run the terminal via **ucrt64.exe** (or mingw64.exe respectively), **not** default msys2 terminal (otherwise the proper gcc variant might not be used)!

Building libsidplayfp:
* **NOTE:** Building the libsidplayfp from the master is more involved and not covered here. This guide assumes you're building one of the [source releases](https://github.com/libsidplayfp/libsidplayfp/releases) of the libsidplayfp which is simpler.
* `cd` (with MSYS2 e.g., UCRT64.exe terminal) into the libsidplayfp's root.
* To specify either:
  * C++20 – set this in the terminal: `CXXFLAGS="$CXXFLAGS -std=c++20"` (new)
  * C++14 – set this in the terminal: `CXXFLAGS="$CXXFLAGS -std=c++0x"` (older versions of libsidplayfp)
* Finally, run: `./configure LDFLAGS="-static" && make && make install`
  * TIP: if doing this for the first time, break down these 3 commands (i.e., they are separated by `&&`) and run them one by one so you can catch any problems more easily.
* Note: the lib will be automatically found by our cmake later (if you've installed the msys into the `C:\msys64\`) and it will get linked statically.

Building Portaudio:
* [Download](http://files.portaudio.com/download.html) the PortAudio stable source release.
* `cd` (with MSYS2 e.g., UCRT64.exe terminal) into the PortAudio's root.
* `cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release`
* `mingw32-make`
* Copy headers and libs into the following new folder (so our cmake can find it automatically):
  * _headers_ into the `C:\Program Files\PortAudio\include\`
  * _libs_ into the `C:\Program Files\PortAudio\include\lib\`

wxWidgets:
* Simply [download](https://www.wxwidgets.org/downloads/) the appropriate pre-built binaries for your compiler (e.g., GCC v13).
* Copy them into the new folder:
  * _headers_ into the `C:\wxWidgets\include\` (with `msvc` and `wx` subfolders in there)
  * _libs_ into the `C:\wxWidgets\gcc_lib\`
* IMPORTANT: additionally, in order to actually run the sidplaywx application after it's built, you need to copy the following wxWidgets' `.dll` files into the sidplaywx's **build** folder: `wxbase315u_SUFFIX.dll`, `wxbase315u_xml_SUFFIX.dll`, `wxmsw315u_core_SUFFIX.dll`, `wxmsw315u_propgrid_SUFFIX.dll` (the exact `_SUFFIX` suffixes differ depending on gcc version etc.).
* TIP: You can also build the wxWidgets yourself in the similar manner to building the PortAudio (in case you want to use a specific GCC version not offered among pre-built binaries).

Finally building the actual **sidplaywx** application:
* The main `CMakeLists.txt` should do the trick (I myself use the Visual Studio Code).
* IMPORTANT: additionally, in order to actually run the sidplaywx application after it's built, you need to copy the following files into the sidplaywx's **build** folder:
  * `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll` found in your appropriate MSYS bin folders (e.g., ucrt64 or mingw64).
  * The entire `dev\theme` folder (so you end up with `build\theme`).
  * The `dev\bundled-Songlengths.md5` file (so you end up with `build\bundled-Songlengths.md5`).
  * The `dev\bundled-STIL.txt` file (so you end up with `build\bundled-STIL.txt`).
  * Tip: you can see the [release](https://github.com/bytespiller/sidplaywx/releases) package for example of bundled dependency files if you get stuck.

---

### Linux
_Coming soon, stay tuned..._
<br><br>
Please note that while the sidplaywx application runs fine under Wine, it is not fully supported that way (e.g., STIL isn't displayed due to a layout problem where the label is zero width, and there may be issues with text colors, styles etc.).
</details>
