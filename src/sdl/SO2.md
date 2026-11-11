# Special Operations 2 SDL integration

SO2 is available in the SDL port through **Start SO2** or `--campaign=so2`.
Support is experimental: automated checks cover every mission start,
navigation entry and default cutscene sequence, but do not establish that a
complete campaign, every objective or every story branch works correctly.
All game-core changes are guarded by `SDL_PORT`; the Win32 reconstruction
remains byte-identical to retail.

## Implemented

- The launcher shows `Start SO2` only when the selected installation contains
  the required expansion files. The original in-game menu is unchanged.
- Bare campaign, mission, alternate-ship and initial-state filenames select
  SO2 resources. Explicit paths retain their original meaning.
- Continue and save slots use `SAVEGAME.SO2`, with `TEMPGLOB.002` for scratch
  state. WC2 and SO1 saves and temporary files remain separate.
- New Game offers to transfer the pilot from a valid SO1 Continue checkpoint,
  falling back to WC2 when SO1 has none. The destination starts at SO2's
  initial campaign state; the source save is never written. Declining keeps
  the normal new-pilot flow. The existing overwrite confirmation protects an
  occupied SO2 Continue checkpoint.
- Navigation reads `SECTOR.002` (Deneb). `INCIDENT.S00` remains shared:
  `GAMEFLOW.S02` explicitly references both it and `INCIDENT.S02`.
- The original `GAMEFLOW.S02` MUS table contains 71 entries: 67 base tracks,
  three from `MUSIC.R01`, and one from `MUSIC.R02`. SDL plays the corresponding
  `MUSIC.A00`, `.A01` and `.A02` sections with OriginFX on DOS and Saga data.
- The Morningstar's Mace uses the original SO2 weapon statistics, unguided
  launch, fuse, blast damage, phase-shield penetration and chained explosions.
  Its nuclear effect uses the original `NUKE.VGA` frames, palette flash and
  hazard-field clearing. No replacement graphics or new controls are added.
- Cutscene test targets accept `CAMPAIGN=so2` for series 1–5, missions 0–3.

## Native corrections

SO2's weapon type 64 is a serialized identifier, not an object-table index.
SDL maps it to the appended Mace record at index 49, and effect type 65 to
index 50. Weapon selection, firing and rendering use these mapped records.

Two original data/engine edge cases also need native bounds checks:

- Series 2 mission 1 gives Maniac comm portrait 36, outside the 32 persistent
  pilot-status flags. An untracked portrait has no recorded death: spawning
  proceeds, and destruction does not write beyond the saved roster. This
  preserves the portrait and the original save layout.
- Series 3 mission 3 can land without a return-home objective. Its lookup
  returns `-1`, so there is no objective flag to update.

Mission numbers above are player-facing; CLI mission selectors start at zero.

## Validation

Checks used isolated data directories with ASAN and UBSAN enabled. Flight
smoke tests disabled player damage and collisions, then ran for 100 frames
or reached an immediate landing/victory transition. Navigation tests entered
each active navigation point directly; they do not simulate travel between
all points or completion of their objectives.

| Check | DOS data | Kilrathi Saga data |
| --- | --- | --- |
| SO2 preflight and postflight cutscene entry points | 40/40 passed | 40/40 passed |
| SO2 direct flight starts | 20/20 passed | 20/20 passed |
| SO2 active navigation entries | 81/81 passed | 81/81 passed |
| Mace selection, firing, ammo, phase shields, blast range and chained detonation | Passed | Passed |
| Mace natural fuse, original explosion rendering and animation cleanup | Passed | Passed |
| SO2 Continue save/load and WC2/SO1 save isolation | Passed | Passed |
| Expansion music tracks 67–70 load and start | Passed | Passed |
| WC2 and SO1 first-mission preflight and flight regressions | 4/4 passed | 4/4 passed |

Additional checks passed:

- 13 pilot-transfer cases: SO1 preference, WC2 fallback, malformed or empty
  checkpoints, unavailable sources, declining, English/Spanish prompts,
  campaign reset, save/load and unchanged source saves. These include the
  existing WC2-to-SO1 transfer path.
- Launcher detection for base-only, SO1-only, SO2-only and combined
  installations; directory changes, saved campaign selection and
  English/Spanish layouts.
- SO1 Strakha cloaking/firing, outbound and return jumps, and the mobile
  objective path-sentinel regression.
- Native CLI/GUI and translation builds, `make verify`, and
  `make compare-bytes`: all 714,752 retail bytes remain identical.

Cutscene checks use the default state at each entry point and automated input
to advance dialogue. They do not cover every conditional branch or prove
visual/audio fidelity. Mace checks exercise focused behavior, including live
flight through the original five-tick launch delay and 200-tick fuse; they
are not a substitute for playing its missions.

Remaining validation includes full campaign progression, alternate outcomes,
objective completion, docking/jump sequences in SO2, save/resume throughout
play, and endings. The existing SO1 anomalies remain tracked in issues
[#8](https://github.com/neuromancer/wc2-re/issues/8) and
[#9](https://github.com/neuromancer/wc2-re/issues/9).

## Assembly evidence

Addresses below refer to the locally installed original Saga `SO2.exe`,
SHA-256 `70175ab4fab5632deeac4d35f2c988255ec8af39b9a761b71554b7bc2a8a587a`.

| Behavior | Original SO2 evidence |
| --- | --- |
| Mace and nuclear-effect records | `0x0049B0C3`, `0x0049B1B6`, 243-byte record stride |
| Nuclear frame sequence | `0x00498020`, frames 0–12 held for two ticks each |
| Weapon/object identifier mapping | `0x0045168F`, `0x0045199B` |
| Shared missile art and nuclear resource loading | `0x00423062` |
| Mace firing, launch and phase-shield handling | `0x00456DBB`, `0x004571CC`, `0x00452C01` |
| Blast range/damage, nuclear conversion and nested explosion traversal | `0x00454E54`, `0x004556B0`, `0x00455758` |
| Flash, hazard clearing, arming and unguided speed | `0x004412D7`, `0x00441395`, `0x004415D1`, `0x00443837` |
| Original out-of-roster pilot access | Caller `0x004380B5`, lookup `0x00437669`, 32-entry allocation `0x0041837B` |

To exercise a Morningstar mission or the previously failing Maniac startup:

```sh
make run-modern-mission CAMPAIGN=so2 SERIES=3 MISSION=1
make run-modern-mission CAMPAIGN=so2 SERIES=2 MISSION=0
```

Set `MODERN_RUN_DIR` to a disposable copy of an installation containing SO2.
For cutscenes, use `make modern-test-cutscenes CAMPAIGN=so2` and repeat with
`CUTSCENE_PHASE=postflight`.
