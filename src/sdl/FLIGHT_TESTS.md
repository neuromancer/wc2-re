# Flight fuzzing

`bin/checkFlight.c` is a libFuzzer harness for WC2, SO1 and SO2 spaceflight,
adapted from the flight fuzzer in `../wc1-re`, and `bin/runGameTests.py` runs
it in a disposable installation. It uses the real SDL event handling, flight
controls, combat simulation, cockpit and original renderer. Player damage,
collisions, enemy AI, weapons and audio remain enabled.

Build and run against an installation containing `GAMEDAT`:

```sh
make check-fuzz GAME_DATA=/path/to/WC2 FUZZ_ARGS='-- -max_total_time=3600'
make check-fuzz-so1 SO1_DATA=/path/to/WC2 FUZZ_ARGS='-- -max_total_time=3600'
make check-fuzz-so2 SO2_DATA=/path/to/WC2 FUZZ_ARGS='-- -max_total_time=3600'
```

`make fuzz-flight` builds `out-modern/fuzz/flight-fuzzer` with libFuzzer,
ASan and UBSan, apart from the normal SDL build; `FUZZ_CC` and `FUZZ_CXX`
select a Clang that ships libFuzzer, such as Homebrew's LLVM on macOS. UBSan
reports are printed and fuzzing continues; ASan reports, aborts and fatal game
exits are crashes. DOS and Kilrathi Saga data are accepted, and `STREAMS` is
copied when present. The runner copies assets into a temporary directory and
excludes existing saves, temporary campaign state, configuration and log
files; the source installation is not written, and the preferences the volume
keys save stay in the temporary directory too.

One fuzzer process flies one campaign, because each campaign loads its own
ship and mission data; run the campaigns as separate processes. The inventory
holds 47 base-game mission selectors (series 1–12) and 20 per installed
expansion. The ordinary Win32 reconstruction does not include this code, and
every game fix below is compiled only into the SDL port.

## Inputs

An input is a campaign byte (0 WC2, 1 SO1, 2 SO2, matching the process), the
series (1-based), the zero-based mission, a start byte, a four-byte
little-endian seed for the game's random generator, then pairs of action and
hold bytes:

- Start bits 0–3 give the flight-path legs travelled before control begins.
  With bit 7 set they instead name a nav point (0–9) entered directly, which
  also reaches nav points off the flight path. Bit 6 starts in the cockpit
  rather than the full-screen view.
- Action bits 0–5 pick one of 64 controls: arrows, diagonals and roll,
  throttle, afterburner, guns and missiles, the VDU, targeting and weapon
  keys, autopilot, the Alt radio commands, comm replies, the views, nine
  pointer positions, both mouse buttons and the wheel, with Shift, Ctrl and Alt
  held as controls of their own. Bit 6 keeps the controls already held.
- Hold bits 0–5 give 1–64 flight updates and bits 6–7 multiply them by 1, 2,
  4 or 8. The most recent held key repeats every four updates.

`--ticks` (default 600, at most 3600) caps the controlled updates of an input
and sets libFuzzer's `-max_len`. Pause, quit, joystick calibration and the
in-flight computer (`N` with the navigation VDU up) wait for other input and
stay out of the action set. The fuzzer writes small seeds for
every selected mission: a start after zero to three legs, the cockpit start,
and one for each active nav point.

## Running and replaying

The runner prints the build's SHA-256, the libFuzzer command and a replay
command, and logs the session to `out-modern/fuzz/flight-<campaign>-*.log`.
The corpus defaults to `out-modern/fuzz/flight-corpus-<campaign>` and crash
inputs go to `out-modern/fuzz/flight-artifacts-<campaign>`. libFuzzer flags
follow `--`. The game's own console output costs time on every input, so the
runner closes it (`-close_fd_mask=3`, or only stdout in fork mode) unless a
replay or another mask is given; libFuzzer's status lines and sanitizer
reports still appear.

```sh
# One mission, by its zero-based mission number.
python3 bin/runGameTests.py out-modern/fuzz/flight-fuzzer /path/to/WC2 --case so2:3:1

# Keep fuzzing after crashes, timeouts and out-of-memory reports.
make check-fuzz GAME_DATA=/path/to/WC2 \
    FUZZ_ARGS='-- -fork=1 -ignore_crashes=1 -ignore_timeouts=1 -ignore_ooms=1 -max_total_time=3600'

# Replay a saved input; WC2_FLIGHT_TRACE=1 prints the travel and the end state.
WC2_FLIGHT_TRACE=1 python3 bin/runGameTests.py out-modern/fuzz/flight-fuzzer /path/to/WC2 \
    --campaign so1 --replay out-modern/fuzz/flight-artifacts-so1/crash-<hash>
```

One process flies many inputs. Each restarts the clock and the timers kept
against it (input polling, scene frames, the frame throttle), the random
generator, the settings flight keys toggle and the campaign state, and ends
with the campaign loop's own teardown, but the game keeps other state from
mission to mission, as it does in play. A crash that does not replay alone can depend on
earlier inputs; replaying the corpus in order (`--corpus DIR -- -runs=0`) can
reproduce it. Fork mode discards a crashed job's output, so with `-fork` the
runner sends sanitizer reports, crash stacks included, to
`out-modern/fuzz/flight-reports-<campaign>/report.<pid>` instead.

## Failures found by the fuzzer

Every function on these paths matches the retail executable, so each is a
defect of the original game; the native build turns its silent bad reads
into sanitizer reports. All changes are under `SDL_PORT`.

| Failure | Found in | Original behaviour | SDL port change |
| --- | --- | --- | --- |
| Audio mixes a freed sample | `wc2:1:0` seed: guns, then autopilot | With more sounds than voices, stopping a playing sound hands its voice to the loudest waiting sound, which moves to the head of the active list. `ix_system_delete_all_sounds` walks that list first, so the promoted sound survives, still playing a sample `stop_all_sounds` frees next. | Delete the waiting sounds first (`src/ix/system.cpp`). |
| Packet handle table overflows | Every mission with a capital ship, about 50 flights | `load_ship` never marks a capital-ship slot as loaded, so each nav sphere's `load_all_slots` refetches its 41 packets and loses the previous set; `init_intelligence_data` never frees a 12-byte buffer. Their handles fill the 4,096-entry table until the game exits with "qq mem push overflow". | Keep a capital slot that still holds the ship, and free the buffer (`src/mission.c`). |
| Engine exhaust read from a freed packet | `wc2:2:2`, `wc2:6:1` after travel | `new_sphere_shapes` frees the slot of a ship type the next nav does not preload, but such a ship can stay in the sphere and is still drawn; its exhaust records, and the type's for the next `load_all_slots`, point into the freed packet. | `free_ship` clears those pointers (`src/mission.c`). |
| Wing debris from past the slot table | `wc2:12:2` | When such a ship explodes, `Create_explosion_debris` finds no slot for it and takes the wing shape from past the five slots. | Leave the wing out (`src/ship.c`). |
| Capital-ship view freed by an explosion | `so1:4:0` after travel | When no object is free for the explosion, `ShipExplosion` frees the dying capital ship's view packet even when it is one its resource slot preloaded and every ship of the type shares; the leftover loop of `set_up_action_sphere` does the same. The seeded tester's fix below covered `remove_object` and the release loops. | Free only a packet the ship loaded itself (`src/ship.c`, `src/mission.c`). |
| Wingman without an intelligence file | `so1:4:0` (Landis), also possible in `so1:5:0` (Zoe) | A routed ship that `cruise_home` removes far from the player, or that lands at home, keeps its mission record at 0, unlike a routed ship `set_up_action_sphere` removes (state 3). The next set-up of its first nav point brings it back, and if the player's wingman is gone by then, it becomes the wingman. SO1's portrait-21 aces have no `INTEL.021` in any release, so the game exits with a disk error, for example when the player returns to the Olympus after Hobbes died and Landis fled or was sent home. | Record a routed ship as gone when it leaves (`src/brains.c`), and fly an ace without an intelligence file on its ship type's file (`src/mission.c`). |
| Maneuvers aimed at object -1 | `so1:2:3` after travel | `perform_maneuver` sends an inactive target to the line-up and veer-away maneuvers, and a ship can have none; they then use the position before `g_aShipPosition`. | End those maneuvers without a target (`src/fl.c`). |

The fuzzer also found a defect of the port itself. `GetNamedPacketSize`
returns `GetPacketSize`'s `(unsigned int)-1` for a missing packet as a `long`.
That is -1 in retail but 4294967295 on 64-bit macOS and Linux. So when a ship
whose portrait has no `communic.t` file speaks (portrait 10 in `so1:2:1` at
nav 4), `real_vid_transmit`'s `> 0` test passes, and the port loads the
missing subtitle and exits with a disk error. `GetNamedPacketSize` now keeps
retail's -1 (`src/eventmgr.c`).

## Failures found by the seeded tester

The seeded tester that preceded the fuzzer visited each active nav point
with random keys. Its first sweeps (43–42/50 WC2, 17–18/20 SO1, 20/20 SO2)
found the failures below. Every function on these paths matches the retail executable byte for
byte, so each is a defect of the original game that shipped mission data
reaches; the native build turned silent bad reads into sanitizer reports.
Where retail's result is well defined and harmless, the port reproduces it;
otherwise it follows the evident intent, as the WC1 port does for the same
code. All changes are under `SDL_PORT`.

| Failure | Selectors | Original behaviour | SDL port change |
| --- | --- | --- | --- |
| Strike target outside the nav table | `wc2:6:0` nav 2, `wc2:4:3` | A raider whose target no nav point lists uses the target's ship number as a nav index; retail reads entries 10 and 15 of the ten-entry table, inside the mission-ship records that follow. | `init_intelligence_data` copies the same record bytes (`src/mission.c`). |
| Missile defence aims at object -1 | `wc2:6:3` nav 1 | `Mkill_missile` aims at whatever the last AI scan left in `g_nTargetShip`, often -1. | `FindMissileTargetingObject` stores the missile it finds, as in WC1 (`src/logic.c`). |
| Torpedo aimed at object -1 | `wc2:10:0` nav 2, `so1:3:1` | Every torpedo launched while the player is in torpedo-lock view takes the player's saved lock, enemy torpedoes included; it is -1 when none was saved. | The lock applies to the player's own torpedo; a -1 target is measured as retail does (`src/ship.c`). |
| Formation steering on object -1 | `wc2:12:3` nav 1 | A wing leader running `imperial_formation` follows "object -1", built from neighbouring tables. | No formation steering without a leader, as in WC1 (`src/smart.c`). |
| Capital-ship view drawn after it was freed | `so1:2:3`, `so1:5:1`, `wc2:7:0`, `wc2:10:0` | Capital and Broadsword-class ships borrow view packets preloaded by their resource slot. `remove_object` and three release loops free them regardless, under every other ship of that type showing the view; the loops can free one twice. | Free only packets the ship loaded itself (`g_abObjectShapeOwned`) (`src/geom.c`, `src/mission.c`). |
| Audio mixes freed samples | `wc2:4:3`, Saga data | The ix library deactivates a voice without the mixer lock, then `stop_all_sounds` frees its sample while the mixer may still be reading it. | `ix_dspv_clear_active` takes the mixer lock (`src/ix/dspv.cpp`). |
| `ship.v32 #38` disk error | `wc2:8:3` navs 6–7 | The final mission's two "Minefield" navs need `SHIP.V32`, absent from both releases; retail exits if the player comes within 10,000 units of either. Neither is on the flight path, so players rarely do. Confirmed with the original DOS release (see below). | A checksum-guarded data patch empties those two navs (`src/sdl/mission_patches.h`). |
| Mid-flight scene never ends | `so1:4:1` nav 2 | Pressing a key before the rescue scene's `pickup.v00` animation starts leaves that FLIC undrawn, so it never reaches its last frame; the scene then loops without reading input. | During a skip, a FLIC the script reaches ends as if its last frame played (`src/gameflow.c`). |

## Reproducing in the original

The Kilrathi Saga Win32 `WC2.EXE` starts its game main with no arguments,
so it ignores developer options. The DOS `WC2.EXE` still parses them, with
its own letters. After `Origin`, which must come first, it accepts:

- `s<series>`: series.
- `m<mission>`: zero-based mission, or `A`–`D`.
- `l`: launch straight into flight, skipping the title and briefing.
- `as<nav>`: entry nav point.
- `-k` and `-b`: no player damage and no collisions.

`v`, `e` and `t` select VGA, EGA and Tandy graphics. The missing-ship
failure reproduces with the unmodified DOS release in DREAMM:

```sh
cd /path/to/dos/install
dreamm -windowed -launch WC2.EXE Origin v s8 m3 l as6 -k -b
```

It stops with `Error: ship.v32, packet 38` (the developer form of the disk
error, because `Origin` is active).

## Series 13

`MODULE.000` holds a thirteenth base-game series, but no campaign flight
reaches it. `SERIES.S00` sends every final branch there and `GAMEFLOW.S00`
then plays the ending. Its unused nav records hold uninitialized editor bytes
whose wave commands request `ALTINT` files no release ships, so the original
exits with a disk error as each of those flights starts. The fuzzer does not
enumerate series 13; it rejects `wc2:13:*` selectors.

## Harness design

The harness follows `RunSpaceFlight`'s setup and steps its control, simulation
and drawing calls without the normal frame wait, then ends each flight with
`RunSpaceFlight`'s cleanup and the campaign loop's
`ReleaseSpaceflightResources`. Mid-flight scenes receive a Space press every
47 scene frames.

The game runs on a simulated clock. The fuzzer's copy of `src/sdl/input.c`
reads `SDL_GetTicks` and `SDL_Delay` from the harness: each flight update and
each scene frame advance four 1/60-second game ticks, and sleeps on the game
thread, waits that keep pumping and waits that keep polling the clock pass
time of their own. Scenes therefore finish without host time passing, and an
input replays the same way on the same build and data. A wait that never
returns to flight fails after two simulated hours instead of timing out. The
audio mixer and the Kilrathi Saga speech streamer still run on host threads.

Travel along the flight path follows autopilot. A capital ship leaving by the
current nav point's warp point is waited for, as autopilot's "Wait for"
message requires. Wingmen and ships bound for the same point arrive in
formation, and other team members reach their own destination if it is
nearer. The game's own objective checks then register each arrival. Escorted
ships get up to 3,600 updates per input to arrive. Travel does not wait for
enemies to leave, as autopilot would: the player could have fought them first.

A fatal error that calls `exit()` during an input aborts, so libFuzzer keeps
the input. Headless message boxes are printed instead of shown.

## Coverage limits

Travel skips the combat between nav points, and entering a nav point directly
can reach nav points a player would rarely visit, such as the final mission's
minefields. Campaign story conditions, the scenes after a mission and
interactive menus are outside the harness.
