# Wing Commander II source reconstruction and SDL2 port

This project recreates the source of **Wing Commander II** as shipped in *Wing
Commander: The Kilrathi Saga* (1996). The reconstructed game core is C, the
`ix` audio library is C++, and the reference build uses Microsoft Visual C++
4.1 under [`wibo`](https://github.com/neuromancer/wibo) to reproduce the
original Win32 `WC2.EXE`.

A native SDL2 port and optional graphical launcher are available for Windows,
Linux, and macOS. They support Kilrathi Saga data and have partial support for
the original DOS game data.

No retail game data files are included.

Browse the [reconstructed Win32 source](https://neuromancer.github.io/wc2-re/)
with searchable Doxygen references, subsystem groups and assembly listings.

If you find this project useful, you can [support its development through
GitHub Sponsors](https://github.com/sponsors/neuromancer).

## Status

The reference build produces a **byte-identical `WC2.EXE`**: all 714,752 bytes
match the Kilrathi Saga retail executable, including code, data, padding, and
PE/debug metadata. `make compare-bytes` verifies the complete files without
normalization and fails on any difference.

The build uses the original MSVC 4.1 toolchain, a reproducible PDB seed, and
the retail link timestamp. `make report` and `make compare-functions` provide
function-level diagnostics; the whole-file comparison is the completion gate.

The SDL2 port runs the title sequence, campaign intro, pilot database,
save/load menus, cutscenes, and spaceflight, including firing, targeting,
cockpit displays, and music. A DOS install plays its own AdLib music and
synthesized OriginFX sound effects instead of the Kilrathi Saga streams and
waves. The optional OpenGL renderer redraws space objects at output resolution
while retaining the original indexed cockpit, HUD, and palette effects.

## Screenshots

The game captures were taken through DREAMM at the original 320x200
resolution.

| Title menu | Opening sequence |
| --- | --- |
| [![Wing Commander II: Vengeance of the Kilrathi title menu](screenshots/title-screen.png)](screenshots/title-screen.png) | [![Kilrathi figure in the Wing Commander II opening sequence](screenshots/kilrathi-intro.png)](screenshots/kilrathi-intro.png) |

| Cockpit navigation | External flight sequence |
| --- | --- |
| [![Cockpit navigation display during spaceflight](screenshots/cockpit-navigation.png)](screenshots/cockpit-navigation.png) | [![Two fighters during an external flight sequence](screenshots/external-flight.png)](screenshots/external-flight.png) |

## Download and run the SDL2 port

Download the archive for your platform from
[GitHub Releases](https://github.com/neuromancer/wc2-re/releases). Extract its
contents into an installed Kilrathi Saga or DOS Wing Commander II directory and
keep the bundled runtime libraries beside the executable. Start it with that
directory as the working directory:

```sh
# macOS or Linux
cd /path/to/WC2
./wc2-modern
```

```powershell
# Windows PowerShell
cd C:\path\to\WC2
.\wc2-modern.exe
```

The release executable includes the graphical launcher. Starting without
arguments opens it automatically. Supplying any command-line arguments starts
the game directly unless `--gui` is also present.

### Fixes and features

The native port includes these fixes and optional features:

| Fix or feature | Enable with |
| --- | --- |
| Kilrathi Saga or DOS game data, detected automatically | always on |
| Original orchestral startup and logo sequence, when its data is present | always on |
| Original cinematic captions alongside Speech Pack dialogue, with either data set | always on |
| Resizable window, fullscreen toggle, and mouse capture | always on |
| Aspect-correct 4:3 presentation and pointer mapping | always on |
| Mouse-wheel throttle control during spaceflight | always on |
| `Esc` pauses during spaceflight when communications are closed | always on |
| Pointer confined only during unpaused, focused spaceflight | always on |
| Automatic SDL gamepad mappings and hot-plug support | always on |
| Background planets drawn with their own sprite and scale (WCDX fix) | always on |
| Static on knocked-out cockpit displays | always on |
| Neutral adaptive difficulty and half-size asteroid collision radii | `--balanced-difficulty` |
| Cheat mode: invulnerability and no collisions | Launcher checkbox or `--cheats` |
| OpenGL space objects rendered at output resolution | `--enhanced` |
| Authentic 16-colour EGA dithering from the DOS installer | `--ega` |
| Heavy-weapon, damage, collision, and afterburner rumble | `--joystick-rumble` |
| WCAT-style four-button joystick layouts | `--joystick-mode=4button-2axis` or `4button-4axis` |
| Twin-stick, HOTAS, throttle, and rudder layouts | `--joystick-axes=<layout>` |
| Joystick diagnostics on stderr | `--joystick-debug` |
| Frame-rate counter | `-f` |
| Cockpitless view | `-c` |
| Optional graphical launcher | no arguments, or `--gui` with other options |

Options can be combined:

```sh
./wc2-modern --ega --enhanced --joystick-rumble \
  --joystick-mode=4button-4axis
```

### Enhanced renderer

The optional OpenGL renderer keeps the original indexed artwork, palettes,
cockpit, HUD, and text while redrawing the ordered space-object layer at output
resolution. The original software renderer remains the default; objects that
cannot use the enhanced path fall back to it automatically. See the
[SDL2 port documentation](docs/SDL2.md#enhanced-renderer) for implementation
details.

`--ega` converts the completed 320x200 indexed frame through the authentic
WC2 DOS installer dither table and standard EGA palette. It works with either
renderer. When combined with `--enhanced`, OpenGL still handles presentation
and scaling, but high-resolution space-object redrawing is disabled so the EGA
conversion is applied consistently to the whole frame.

| Cockpit flight | External flight sequence |
| --- | --- |
| [![Enhanced renderer showing a nearby ship and engine glow from the cockpit](screenshots/enhanced-cockpit-flight.png)](screenshots/enhanced-cockpit-flight.png) | [![Enhanced renderer showing two ships during an external flight sequence](screenshots/enhanced-space-objects.png)](screenshots/enhanced-space-objects.png) |

### SDL2 port controls

| Shortcut | Action |
| --- | --- |
| `Cmd+Enter` (macOS) | Toggle fullscreen |
| `Alt+Enter` (Windows and Linux) | Toggle fullscreen |
| `Cmd+Q` (macOS) | Quit the game |
| Mouse wheel (spaceflight) | Increase or decrease speed |
| `Esc` (spaceflight) | Close communications, or pause |
| Gamepad Start (spaceflight) | Pause or resume |
| Gamepad Back | Escape/back |
| Gamepad Y (`Y/N` prompts) | Confirm Yes |

The default joystick mode retains the original two-axis, two-button controls.
The optional four-button modes add dedicated afterburner, target, weapon,
navigation, autopilot, communications, and speed controls. See the
[SDL2 port documentation](docs/SDL2.md#joystick-input) for the full gamepad,
twin-stick, and HOTAS layouts.

## Build from source

Clone the submodules first:

```sh
git submodule update --init --recursive
```

### SDL2 port

Install a C/C++ compiler plus the SDL2 and LZO2 development packages, then run:

```sh
make -j modern
```

The executable is written to `out-modern/wc2-modern` (or
`out-modern/wc2-modern.exe` on Windows). `make run-modern` launches it with
Kilrathi Saga data in `data/wc2-full`; `make run-modern-dos` uses DOS data in
`data/dos`.

#### Optional graphical launcher

The optional Slint launcher works on Windows, Linux, and macOS. It is linked
statically into a separate executable, so the normal SDL2 build retains the
same dependencies. The capture below shows it running natively on macOS.

<p align="center">
  <a href="screenshots/graphical-launcher.png"><img src="screenshots/graphical-launcher.png" width="520" alt="Wing Commander II graphical launcher on macOS"></a>
</p>

Building it additionally requires CMake 3.21 or newer and Rust 1.92 or newer.
On Linux, install the D-Bus development files as well; the folder picker uses
the desktop portal instead of GTK:

```sh
make -j modern-gui
out-modern/wc2-modern-gui
```

`make run-modern-gui` opens it with the repository's Kilrathi Saga data path.
The launcher automatically finds WC2 installations beside the executable,
in the working directory, and in common Heroic, GOG, Steam, and Windows
EA/Origin library paths. It recognizes the DOS and Kilrathi Saga editions,
including Heroic's `Wing Commander II/DOS` and
`Wing Commander II/Kilrathi_Saga` layouts. Choose an installation from the
dropdown or use **Add path…** to select another directory containing `GAMEDAT`.
The launcher validates the data before enabling **Start game** and remembers
the selected directory and options in `wc2-launcher.ini` in the user's SDL
preferences directory. Explicit command-line options override saved settings;
WC1's launcher preferences are kept separate.

Its central title artwork is
decoded from the final `WC2LOGO.VGA` menu layers with `BRIEF.PAL`, cropped to a
small 247x101 image, and embedded in the executable. Slint 1.18.1 and [Native
File Dialog Extended 1.3.0](https://github.com/btzy/nativefiledialog-extended)
are fetched at pinned tags and linked statically into `wc2-modern-gui`. Release
archives install that integrated build under the usual `wc2-modern` name;
SDL2 and the other platform runtime libraries remain dynamic.

### Reconstructed Win32 build

The default target downloads and checksum-verifies the MSVC 4.1 package, then
builds `WC2.EXE`, `WC2.map`, and compiler-generated assembly under `out/`. The
build verifies `WC2.EXE` against the pinned retail SHA-256 and fails on any
mismatch, including when the executable is already up to date. The original
executable is not required to compile it:

```sh
make -j
```

To run it, provide a Kilrathi Saga disc image. The Makefile substitutes the
reconstructed executable and launches it under DREAMM:

```sh
make run ISO=/path/to/kilrathi-saga.iso
```

Use `make run-original` for the retail executable in the same environment, and
`make debug` to start DREAMM's debugger.

## Source browser

The [Doxygen website](https://neuromancer.github.io/wc2-re/) presents the
Win32 sources that reproduce the retail executable, with SDL-only branches
removed. Files are grouped by subsystem and listed in link order, using the
same theme and publishing pipeline as
[wc1-re](https://neuromancer.github.io/wc1-re/).

Install Doxygen 1.18.0, `unifdef` and Pygments (`pygmentize`), then run:

```sh
make site
```

Open `out/site/html/index.html` locally. This target needs neither game data
nor the MSVC toolchain. File descriptions live in `site/files.tsv`, so
documentation edits preserve the source line numbers used by live assertions
in the matching build. GitHub Actions publishes the site on pushes to `main`.

## Reconstruction workflow

[`binary-comp`](https://github.com/gg-sl-oss/binary-comp) is required only for
comparison and verification commands:

```sh
python3 -m pip install "binary-comp[all] @ git+https://github.com/gg-sl-oss/binary-comp.git"
make compare-func FUNC=WinMain
make verify
```

These look for the retail executable at `../releases/win32/WC2.EXE`; set
`ORIGINAL_SRC` to point elsewhere.

Contributor references:

- [compiler and flag evidence](docs/COMPILER.md);
- [matching patterns](docs/PATTERNS.md);
- [disassembly export workflow](docs/EXPORT.md);
- [compilation-unit order](docs/ORDER.md);
- [function naming policy](docs/LABELS.md);
- [SDL2 port architecture](docs/SDL2.md); and
- [release process](docs/RELEASING.md).

## Acknowledgements

Special thanks to:

- [Origin Systems](https://en.wikipedia.org/wiki/Origin_Systems), who created
  *Wing Commander II* and developed its *Kilrathi Saga* port;
- [Electronic Arts](https://www.ea.com/) for publishing *Wing Commander: The
  Kilrathi Saga*;
- [GOG](https://www.gog.com/en/game/wing_commander_1_2) for keeping *Wing
  Commander 1 + 2* readily available;
- AllTinker for the [W.C.A.T. overhaul](https://alltinker.itch.io/wcat), whose
  fixes and analysis of the DOS release have been valuable references;
- the [WCDX project](https://github.com/Bekenn/wcdx) for its pioneering
  compatibility work on the *Kilrathi Saga* release; and
- Aaron Giles for the [DREAMM emulator](https://dreamm.aarongiles.com/), used
  to run and study the original release, and
  [ymfm](https://github.com/aaronsgiles/ymfm), which powers the SDL2 port's
  AdLib emulation.

## License

See [LICENSE](LICENSE). OpenAI Codex and Anthropic Claude were used during the
reconstruction.
