# SDL2 port

The SDL2 target is a native host for the reconstructed game. It reuses the game
logic, resource loaders, event queue, indexed framebuffer, and `ix` audio API.
Platform code is selected with `SDL_PORT` and must not alter the MSVC 4.20
reference build or its binary comparisons.

## Port boundary

- `src/sdl/` owns windows, input translation, timing, host audio, paths, and
  video presentation.
- SDL input is converted to the game's existing scan codes and event queue;
  gameplay code does not consume SDL events directly.
- The game still renders a 320x200 indexed framebuffer and 256-colour palette.
- Both video backends present it in a centered 4:3 viewport and map mouse input
  through that same viewport.
- Port-only state remains in SDL files. Original-address globals keep their
  reconstructed types and ownership.
- Native objects live under `out-modern/` and never enter `WC2.EXE` or
  `binary-comp` comparisons.

Normal native development builds are compiled with AddressSanitizer and
UndefinedBehaviorSanitizer. A sanitizer report is a port bug, even when the
original executable happened to tolerate the same memory access. Release
archives use `MODERN_RELEASE=1` to omit sanitizer instrumentation.

## Game data and audio

Kilrathi Saga data is the primary supported data set. A DOS install is
recognized by the `0xc1` flag its packet directories carry, and the host then
reads its compressed packet resources and drives its own audio.

Kilrathi Saga music is streamed from `STREAMS/*.STR` and its sound effects are
`GAMEDAT/SFX??.WAV`, neither of which a DOS install has. A DOS install instead
plays `GAMEDAT/MUSIC.A00`, whose packet sections are standard MIDI files
indexed by the very track number the game's music script queues; eighteen of the
sixty-seven sections are empty, and a track without a sequence simply reports
itself finished so the script moves on. `GAMEDAT/WING2.TIM` section 1 supplies
the 124 AdLib timbres those sequences play through.

The native port also restores WC2's original orchestral startup. Its graphics
remain in `TITLE.VGA` and `FIELD.V00`, while the synchronized score and its
sequence markers are in `MUSIC.R00` section 19. Both the DOS and Kilrathi Saga
data sets retain these packets. With Kilrathi Saga data, only that score is
mixed through the OriginFX player; normal music continues to use the recorded
`STREAMS` files.

The DOS release describes sound effects as compact OriginFX command records,
not sampled WAV or VOC files. The game still carries that table and retunes
entries in it while it runs, so the host reads the live table rather than a
copy, and synthesizes the effects on the same YM3812 the music runs on. Held
records, such as the afterburner, play until the flight code says the burn is
over.

In flight, both the sampled Kilrathi Saga effects and synthesized DOS effects
use the DOS left/right pan derived from the source's direction relative to the
current camera. Synthesized DOS effects also use the recovered DOS distance
falloff.

Speech is a separate optional pack and is not required. When its sampled speech
archives are present, the native port plays them with either supported data set.
DOS speech shares the SDL mixer with the synthesized OriginFX music and effects;
an installation without the pack continues to display the scripted text.

The native port keeps the original scripted caption visible while a voiced
cinematic line plays. The retail executable deliberately suppresses ordinary
cutscene text whenever it has speech loaded. In-flight radio communications
retain their original speech-or-text behavior.

## Enhanced renderer

`--enhanced` selects the optional OpenGL 3.2 renderer. It records the ordered
space-object layer and redraws it at output resolution with sharp-bilinear
sampling. Ships, missiles, mines, projectiles, asteroids, debris, explosions,
stars, planets, the intro logo, and launch-bay doors use this path. The cockpit,
HUD, text, palette effects, and other screens remain in the indexed base frame.

The default renderer always uses the original software drawing path. If the
enhanced renderer cannot record an object, that object also falls back to the
software path. Renderer-specific OpenGL state stays in `src/sdl/`; recovered
game files expose only narrow `SDL_PORT` hooks.

## Host controls and behavior

| Shortcut | Action |
| --- | --- |
| `Cmd+Enter` on macOS | Toggle fullscreen |
| `Alt+Enter` on Windows and Linux | Toggle fullscreen |
| `Cmd+Q` on macOS | Quit the game |
| Mouse wheel during spaceflight | Increase or decrease speed |
| `Esc` during spaceflight | Close communications, or pause |

Mouse flight steers by the pointer's distance from the viewport centre. The
host therefore confines it while unpaused spaceflight has focus, releases it
for pauses and modal prompts, and leaves it free everywhere outside flight.
Losing window focus also releases it until the game regains focus.

The native port restores the visual static on damaged cockpit displays. The
Kilrathi Saga raster hook retained the sound and redraw request but no longer
drew the DOS effect, so the host supplies its own display-only noise generator
without consuming the game's random sequence.

Background planets use their per-object sprite and scale in the native port.
The retail renderer grouped them with stars and dust and substituted the
constellation sheet, which made the intended planet art disappear and erased
the wrong footprint on the following frame.

On Windows the launcher disables the Input Method Editor for the process so
IME composition cannot intercept flight keys.

## Joystick input

The default `--joystick-mode=original` mode preserves the original X/Y and
two-button controls. SDL's mapped controller interface supplies the left stick
and A/B buttons on recognized gamepads. Other devices use their first two axes
and buttons. Device removal and reconnection are handled without restarting the
port.

The optional WCAT-style modes give each action its own button. On mapped
gamepads, buttons 1–4 are A/B/X/Y:

| Mode | Buttons 1–4 |
| --- | --- |
| `--joystick-mode=4button-2axis` | Fire, missile, afterburner, roll/throttle modifier |
| `--joystick-mode=4button-4axis` | Fire, missile, afterburner, cycle target |

The four-axis mode reads the mapped right stick, raw axes 3/4, or the X/Y axes
of a second joystick. Its default axis layout is `twin-stick-roll`; select a
different layout with `--joystick-axes=<layout>`:

| Layout | Horizontal and extra axes |
| --- | --- |
| `twin-stick-roll` | Primary X yaws; extra X rolls; extra Y is relative throttle |
| `twin-stick-yaw` | Primary X rolls; extra X yaws; extra Y is relative throttle |
| `hotas-yaw` | Primary X rolls; rudder yaws; final axis is linear throttle |
| `hotas-roll` | Primary X yaws; rudder rolls; final axis is linear throttle |
| `linear-throttle` | Primary X yaws; third axis is linear throttle; keyboard controls roll |
| `rudder-yaw` | Primary X rolls; third axis yaws |
| `rudder-roll` | Primary X yaws; third axis rolls |

Primary Y always controls pitch. Directional extra axes use a 25-percent
deadzone. For example:

```sh
./wc2-modern --joystick-mode=4button-4axis \
  --joystick-axes=hotas-yaw
```

Add `--joystick-debug` to print the detected device type and each joystick
axis, button, and hat event to the terminal.

Add `--joystick-rumble` to enable gentle heavy-weapon, damage, collision, and
afterburner feedback on supported controllers and force-feedback joysticks. It
is disabled by default; devices without rumble support continue without
feedback.

Gamepad Start pauses and resumes during spaceflight, Back acts as Escape, and
the Y face button answers Yes at `Y/N` prompts. Mapped SDL controllers use
their named buttons. Both WCAT-style modes also provide these spaceflight
bindings:

The four primary actions map to A/B/X/Y: fire guns, fire the selected release
weapon, hold afterburner, and the mode-specific fourth action.

| Control | Action |
| --- | --- |
| LB | Cycle guns |
| RB | Cycle missiles and other release weapons |
| LT or left-stick click | Navigation display; press again to open the map |
| RT or right-stick click | Autopilot |
| D-pad Up / Down | Full speed / stop |
| D-pad Left / Right | Communications / target lock |

While the communications list is open, D-pad Up/Down moves the highlighted
choice, Right selects it, and Left closes the list. The flight sticks and
A/B/X/Y actions remain active while choosing a response.

The raw 12-button fallback uses buttons 5/6 for LB/RB, 7/8 for LT/RT, 9/10
for Back/Start, 11/12 for the stick clicks, and the first hat for the D-pad.

## Development commands

```sh
make modern
make run-modern
make run-modern-dos
make run-modern-mission SERIES=1 MISSION=0
make run-modern-cutscene SERIES=3 MISSION=0
make modern-test-cutscenes
```

Arguments not owned by the SDL host are forwarded to WC2's recovered option
loader. For example, the executable's direct-flight form is
`Origin v1 t0 e ignored`; the final sentinel is required by the original
loader's one-token lookahead and is supplied automatically by
`run-modern-mission`.

For cutscene testing, `run-modern-cutscene` omits the direct-flight `e`, skips
the restored orchestral title and campaign menus, runs the campaign VM's
non-rendering state prepass followed by its selected rendered entry, and exits
when the VM returns. The equivalent executable command is
`wc2-modern --cutscene-only Origin v3 t0 ignored`.

`modern-test-cutscenes` runs all 50 populated base-campaign mission sequences
to completion with SDL's dummy video and audio drivers. Each sequence runs in
a fresh sanitizer-enabled process, with four running in parallel by default.
Set `MODERN_CUTSCENE_JOBS=1` for a serial sweep. Output is retained under
`out-modern/cutscene-asan/`; the target continues after a failure so one run
can identify every failing selector.

`make modern-test` runs the integrated sanitizer smoke check.
`make modern-test-full` adds the standalone SDL subsystem checks, including a
virtual-joystick test for expanded controls and gamepad key translation.
