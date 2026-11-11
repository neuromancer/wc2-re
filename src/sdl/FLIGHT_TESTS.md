# Seeded flight crash tests

`bin/checkFlight.c` and `bin/runGameTests.py` adapt the CLI flight tester from
`../wc1-re` to WC2, SO1 and SO2. They use the real SDL event handling, flight
controls, combat simulation, cockpit and original renderer. Player damage,
collisions, enemy AI, weapons and audio remain enabled.

Build and run against an installation containing `GAMEDAT`:

```sh
make check-random GAME_DATA=/path/to/WC2 RANDOM_ARGS='--seed 1 --ticks 600 --jobs 4 --keep-going'
```

DOS and Kilrathi Saga data are accepted; `STREAMS` is copied when present.
The runner copies assets into temporary directories and excludes existing
saves, temporary campaign state, configuration and log files. Each parallel
worker has its own writable copy. The source installation is not written.

The default sweep enumerates 47 base-game mission selectors (series 1–12)
and 20 per installed expansion, then visits their active navigation points.
The ordinary Win32 reconstruction does not include this test code, and every
game fix below is compiled only into the SDL port: the 714,752-byte retail
comparison (`make compare-bytes`) still passes.

## Replaying and selecting tests

Every failed run prints a replay command. Logs remain under a unique
`out-modern/flight-<seed>-*/` directory after temporary assets are removed.
`results.json` records the executable hash, seed, input installation, case,
completion count, error and replay command. `--log-dir` selects a new output
directory explicitly.

```sh
# SO2 only, with three consecutive control seeds.
make check-random GAME_DATA=/path/to/WC2 RANDOM_ARGS='--campaign so2 --seed 1 --runs 3 --jobs 4 --keep-going'

# One mission, using its zero-based CLI mission number.
python3 bin/runGameTests.py out-modern/flight-probe /path/to/WC2 --case wc2:6:0 --seed 1

# Isolate a navigation entry that fails during setup.
python3 bin/runGameTests.py out-modern/flight-probe /path/to/WC2 --case wc2:6:0 --nav 2 --seed 1
```

`--ticks` is a per-navigation-point limit of 1–3600 simulation updates;
landing, death or another mission outcome can end an encounter sooner.
`--timeout` bounds a child process in seconds. Without `--keep-going`, the
runner stops after the first failing worker batch. It returns nonzero on
sanitizer reports, process failures, timeouts and exits without the probe's
completion marker. This last check matters because some original fatal
error paths exit with status zero. Headless message boxes are written to
the child log.

## Current results

Seed 1, 600 updates per active navigation entry, all selectors:

| Data | WC2 | SO1 | SO2 | Completed nav encounters |
| --- | --- | --- | --- | --- |
| DOS | 47/47 passed | 20/20 passed | 20/20 passed | 340 |
| Kilrathi Saga | 47/47 passed | 20/20 passed | 20/20 passed | 340 |

Logs are under `out-modern/flight-dos-fixed/` and
`out-modern/flight-saga-fixed/`. They contain installed-game output and are
not distributed with the source.

## Failures found and fixed

The first sweeps (43–42/50 WC2, 17–18/20 SO1, 20/20 SO2) found the failures
below. Every function on these paths matches the retail executable byte for
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
exits with a disk error as each of those flights starts. The tester does not
enumerate series 13; it rejects `wc2:13:*` selectors.

## Tester design

The driver follows `RunSpaceFlight` setup and steps its control, simulation
and drawing calls without the normal frame wait. It releases and reloads
flight resources between nav encounters. Mid-flight scenes receive a Space
press every 47 scene frames. Pause, quit, autopilot, modal menus (including
the N navigation computer) and interactive campaign progression are outside
the randomized key set.

The probe runs the game on a simulated clock. Its copy of `src/sdl/input.c`
reads `SDL_GetTicks` and `SDL_Delay` from the probe: each flight update and
each scene frame advance four 1/60-second game ticks, and sleeps on the game
thread, waits that keep pumping and waits that keep polling the clock pass
time of their own. Scenes therefore finish without host time passing, and a
seeded run repeats exactly on the same data. A wait that never returns to
flight fails after two simulated hours instead of timing out. The audio
mixer and the Kilrathi Saga speech streamer still run on host threads.

The seed repeats the control choices and seeds the game's random generator.
A single-nav run skips the random calls made by earlier nav encounters, so
replay the full mission first when reproducing a combat failure.

## Coverage limits

Entering nav points directly exercises their data and local combat, not
travel between them, campaign story conditions or every possible outcome.
It can reach nav points a player would rarely visit, such as the final
mission's minefields. A passing seeded run is bounded coverage, not a
full-campaign validation.
