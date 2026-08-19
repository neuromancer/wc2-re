# WC2 address-remap similarity report

This report compares the current WC1 reconstruction against WC2 code at the reviewed WC1→WC2 function destinations. Unmapped functions are excluded rather than compared against arbitrary bytes.

The converted source contains WC2 labels only. Historical WC1 addresses live in the row-level migration manifest so a future `wc2-re` fork does not need a dual-game annotation or build path.

## Coverage and headline result

| Metric | Result |
| --- | ---: |
| WC1 source function markers | 1844 |
| Markers with a WC2 destination | 1567 (84.98%) |
| Explicitly unresolved markers | 277 |
| Functions compared by binary-comp | 1558 |
| Compared / mapped markers | 99.43% |
| Exact machine-code matches | 931 |
| Similarity >= 90% | 1424 |
| Similarity < 50% | 0 |
| Average WC2 similarity | 96.89% |
| Median WC2 similarity | 100.00% |
| Report errors / missing exports | 0 |

Low-confidence address assignments and genuinely changed code both depress the aggregate, so the evidence tiers below are more informative than the headline average on its own.

## Similarity by mapping evidence

| Evidence tag | Compared | Average | Median | Exact | >=90% | <50% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `WC1-auto-vt` | 180 | 99.32% | 100.00% | 152 | 179 | 0 |
| `manual-verified` | 28 | 98.19% | 100.00% | 22 | 28 | 0 |
| `WC1-callgraph-single` | 16 | 98.15% | 100.00% | 9 | 16 | 0 |
| `WC1-fuzzy-high` | 14 | 97.97% | 99.82% | 7 | 14 | 0 |
| `function order; parameter shape; unique BriefingMap_DisplayMap caller; scanner helper anchors; assembly control flow` | 2 | 97.83% | 97.83% | 0 | 2 | 0 |
| `WC1-manual-verified` | 225 | 97.80% | 100.00% | 151 | 216 | 0 |
| `WC1-order-exact` | 69 | 97.60% | 100.00% | 56 | 64 | 0 |
| `same per-object update-loop caller; collision/vector callee family; 70.78% binary-comp; target 00445087 is capital-ship strike mission case` | 1 | 97.56% | 97.56% | 0 | 1 | 0 |
| `WC2-manual-verified` | 17 | 97.51% | 100.00% | 10 | 16 | 0 |
| `WC1-callgraph-single-low` | 22 | 97.50% | 100.00% | 14 | 20 | 0 |
| `WC2-only` | 339 | 96.95% | 100.00% | 196 | 316 | 0 |
| `WC1-fuzzy-medium` | 57 | 96.44% | 100.00% | 36 | 49 | 0 |
| `WC1-fuzzy-low` | 105 | 96.27% | 100.00% | 56 | 92 | 0 |
| `WC1-callgraph-propagated` | 198 | 96.18% | 100.00% | 105 | 176 | 0 |
| `WC1-fuzzy-followup` | 146 | 95.88% | 98.42% | 66 | 123 | 0 |
| `WC1-callgraph-transfer` | 8 | 94.24% | 95.03% | 3 | 7 | 0 |
| `WC1-order-semantics-verified` | 24 | 94.20% | 97.12% | 12 | 20 | 0 |
| `WC1-fuzzy-very-low` | 98 | 93.80% | 96.67% | 33 | 79 | 0 |
| `WC1-order-callgraph-verified` | 9 | 92.13% | 90.91% | 3 | 6 | 0 |

## Similarity by source area

| Area | Compared | Average | Median | Exact | >=90% | <50% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| game/core | 1430 | 96.65% | 100.00% | 821 | 1296 | 0 |
| ix audio | 128 | 99.56% | 100.00% | 110 | 128 | 0 |

## Explicit review flags

| Flag | Compared | Average | Median | Exact | >=90% | <50% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `WC1-review-module-mismatch` | 9 | 99.58% | 100.00% | 7 | 9 | 0 |

## Per-file summary

| Source file | Compared | Average | Median | Exact | >=90% | <50% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `src/auto.c` | 5 | 96.91% | 100.00% | 3 | 4 | 0 |
| `src/barracks.c` | 5 | 99.19% | 100.00% | 3 | 5 | 0 |
| `src/brains.c` | 154 | 97.54% | 100.00% | 102 | 141 | 0 |
| `src/cdrom.c` | 5 | 93.08% | 100.00% | 3 | 4 | 0 |
| `src/cmpgn.c` | 11 | 98.81% | 100.00% | 7 | 11 | 0 |
| `src/cockpt.c` | 133 | 96.95% | 100.00% | 85 | 121 | 0 |
| `src/debug.cpp` | 12 | 86.41% | 91.58% | 4 | 6 | 0 |
| `src/dib.c` | 26 | 96.06% | 97.47% | 5 | 24 | 0 |
| `src/disk.c` | 57 | 94.26% | 99.17% | 27 | 44 | 0 |
| `src/eventmgr.c` | 78 | 95.72% | 96.98% | 34 | 72 | 0 |
| `src/geom.c` | 87 | 97.81% | 100.00% | 58 | 82 | 0 |
| `src/gr.c` | 54 | 93.56% | 96.19% | 24 | 46 | 0 |
| `src/hudmsg.c` | 48 | 96.30% | 98.28% | 21 | 43 | 0 |
| `src/ix/dsp.cpp` | 19 | 99.86% | 100.00% | 18 | 19 | 0 |
| `src/ix/dsps.cpp` | 10 | 98.72% | 100.00% | 6 | 10 | 0 |
| `src/ix/dspv.cpp` | 15 | 98.94% | 100.00% | 9 | 15 | 0 |
| `src/ix/ixlog.cpp` | 1 | 100.00% | 100.00% | 1 | 1 | 0 |
| `src/ix/lzo1x.cpp` | 7 | 100.00% | 100.00% | 7 | 7 | 0 |
| `src/ix/mixer.cpp` | 2 | 99.05% | 99.05% | 1 | 2 | 0 |
| `src/ix/sample.cpp` | 4 | 98.72% | 98.73% | 2 | 4 | 0 |
| `src/ix/sound.cpp` | 8 | 99.94% | 100.00% | 7 | 8 | 0 |
| `src/ix/streamer.cpp` | 25 | 99.67% | 100.00% | 23 | 25 | 0 |
| `src/ix/system.cpp` | 27 | 100.00% | 100.00% | 27 | 27 | 0 |
| `src/ix/thread.cpp` | 10 | 99.09% | 100.00% | 9 | 10 | 0 |
| `src/killbrd.c` | 13 | 93.25% | 100.00% | 7 | 10 | 0 |
| `src/logic.c` | 144 | 96.43% | 100.00% | 81 | 130 | 0 |
| `src/main.c` | 25 | 98.33% | 100.00% | 18 | 24 | 0 |
| `src/mathfp.c` | 29 | 95.19% | 96.43% | 13 | 24 | 0 |
| `src/mathutil.c` | 4 | 94.24% | 95.00% | 2 | 3 | 0 |
| `src/mono.c` | 11 | 99.69% | 100.00% | 10 | 11 | 0 |
| `src/music.c` | 47 | 96.72% | 100.00% | 27 | 42 | 0 |
| `src/nav.c` | 57 | 95.96% | 100.00% | 31 | 48 | 0 |
| `src/personnel.c` | 20 | 97.80% | 97.56% | 7 | 20 | 0 |
| `src/pilot.cpp` | 21 | 98.67% | 100.00% | 16 | 21 | 0 |
| `src/pload.c` | 10 | 96.82% | 99.10% | 5 | 10 | 0 |
| `src/screen.c` | 58 | 97.95% | 100.00% | 35 | 57 | 0 |
| `src/screens.c` | 111 | 97.60% | 100.00% | 75 | 104 | 0 |
| `src/ship.c` | 43 | 97.25% | 100.00% | 26 | 39 | 0 |
| `src/smart.c` | 26 | 94.27% | 96.28% | 8 | 20 | 0 |
| `src/sound.c` | 19 | 96.39% | 97.14% | 8 | 18 | 0 |
| `src/spc.c` | 23 | 97.19% | 99.49% | 9 | 22 | 0 |
| `src/strdos.c` | 11 | 99.09% | 100.00% | 9 | 11 | 0 |
| `src/sysinput.c` | 23 | 99.77% | 100.00% | 22 | 23 | 0 |
| `src/system.c` | 9 | 93.55% | 100.00% | 5 | 7 | 0 |
| `src/text.c` | 6 | 98.85% | 100.00% | 4 | 6 | 0 |
| `src/winmain.c` | 45 | 97.08% | 100.00% | 27 | 43 | 0 |

## Unresolved mapping inventory

| Source file | Count | Functions |
| --- | ---: | --- |
| `src/barracks.c` | 22 | `CreateEmptySaveGameFile`, `EnsureSaveGameFile`, `InitializeBarracksAnimation`, `FreeBarracksMenuLabel`, `SetAwakenBarracksMenuLabel`, `FreeBarracksMenuLabels`, `SaveGame`, `RunWc1TextInputPrompt`, … (+14) |
| `src/brains.c` | 15 | `AnimateScrambleWalk`, `DrawScrambleActor`, `ConfigureScrambleActor`, `DrawScrambleFrame`, `scramble`, `landing`, `funeral_player`, `funeral_wingman`, … (+7) |
| `src/cdrom.c` | 2 | `AllocateFontWorkspace`, `FreeFontWorkspace` |
| `src/cmpgn.c` | 13 | `LoadWc1PaletteTripletsFile`, `RunWc1StrandedSequence`, `ParseFaceAnimation`, `ParseMouthAnimation`, `AddPCName`, `LoadFace`, `LongTalk`, `CloseTalk`, … (+5) |
| `src/cockpt.c` | 2 | `EraseCockpitReadoutAtPosition`, `PlayTargetLockSfx` |
| `src/debug.cpp` | 3 | `DebugOverlayConsole::~DebugOverlayConsole`, `DebugKeyboardHookProc`, `DebugOverlayConsole::SetOpaqueBackground` |
| `src/dib.c` | 2 | `SetWc1SpaceFlightFrameTiming`, `DirectDrawResultToText` |
| `src/disk.c` | 9 | `LoadWc1PacketIntoBuffer`, `SortSignedByteValuesAscending`, `CheckWc1DiskAvailable`, `PromptInsertNumberedDisk`, `GetZeroUnused`, `CheckEscaped`, `WaitForWc1SceneAdvance`, `MoveMenuPointerFromKeyboard`, … (+1) |
| `src/eventmgr.c` | 5 | `SuspendWc1MouseCursor`, `GetWc1VideoReleaseResult`, `Wc1ShutdownHook`, `shrink_vector`, `shrink` |
| `src/geom.c` | 11 | `SeekPacketSection`, `GetMusicDriverPresent`, `intfract_sign`, `SignShort`, `MakeRandomNormalizedVector`, `ConvertShortVectorToFixedVector`, `ship_vs_point`, `ship_vs_ship`, … (+3) |
| `src/gr.c` | 2 | `DescribeUnregisteredViewport`, `ClearStreamerTrigger` |
| `src/hudmsg.c` | 15 | `RunWc1KeyAcknowledge`, `GetArcadeBonus`, `FigureArcadeTime`, `DrawWc1ArcadeScorePanel`, `UpdateArcadeScoreDisplay`, `RefreshCockpitStatus`, `RunWc1SpaceFlight`, `UpdateWc1TrainSimMenuCursor`, … (+7) |
| `src/ix/dsp.cpp` | 2 | `ix_dsp_static_initializer`, `ix_dsp_static_initialization_hook` |
| `src/ix/mixer.cpp` | 2 | `ix_mixer_static_initializer`, `ix_mixer_static_initialization_hook` |
| `src/ix/thread.cpp` | 1 | `IxStreamFile::ix_stream_file_is_reading` |
| `src/killbrd.c` | 12 | `CorrectPointers`, `ClearRoomMenuLabel`, `IsRoomMenuLabelEmpty`, `DrawRoomMenuLabel`, `RefreshRoomMenuLabel`, `ClearRoomMenuCursorFrame`, `SelectRoomMenuLabel`, `InitializeRoomMenu`, … (+4) |
| `src/logic.c` | 33 | `drop_mine`, `LoadWc1GamePaletteFile`, `InitializeEventManagerResources`, `StartWc1EventManager`, `LoadWc1OriginFxDrivers`, `InitializeWc1DirectionViewFrames`, `LoadLegacySpaceflightResourceSets`, `GetFxDriverInitResult`, … (+25) |
| `src/main.c` | 3 | `RunWc1GameMain`, `init_player_input`, `SelectPreviousExternalViewObject` |
| `src/mathfp.c` | 2 | `ReleaseVideoResourcesHook`, `ResetTextCursor` |
| `src/mono.c` | 7 | `MeasureScaledIntroTextWidth`, `DrawWc1CenteredScaledIntroText`, `GetLineLength`, `print_subtitle`, `advance_canned_sequence`, `update_canned_sequence`, `ResetStringBuilder` |
| `src/music.c` | 10 | `parse_view_script`, `update_scripted_view`, `initialize_scripted_view`, `DecompressPacketSection`, `GetTargetColourIndex`, `AlignWc1SpriteFrameToRectCorner`, `SetMusicOn`, `SelectFlightMusicTrack`, … (+2) |
| `src/nav.c` | 13 | `NavMapPointInsideReservedArea`, `add_statistics`, `PostMission`, `FullMissionScore`, `PlayersMissionScore`, `UpdateSeries`, `MoveNewCampaign`, `StartNewCampaign`, … (+5) |
| `src/pilot.cpp` | 27 | `WaitForKeyExceptXOrF12`, `ShowMeanwhileTransition`, `ApplyAnswerTextCipher`, `LoadAnswerPromptAndResponse`, `PromptForAnswerText`, `SceneEnterHook`, `EraseTextContextBackground`, `DisplayTrainSimHighScoreTable`, … (+19) |
| `src/pload.c` | 1 | `FindActiveSoundEntryBySample` |
| `src/screen.c` | 12 | `ShouldSuspendCursorForRect`, `FrameStartHook`, `CreateCannedSceneObject`, `ShowCampaignVictorySequence`, `ShowTigerClawEscapeScene`, `ShowWc1EndScreen`, `get_face`, `LoadCommPortraitShape`, … (+4) |
| `src/screens.c` | 28 | `LoadBriefingRoom`, `ViewWc1Medals`, `AwardCampaignMedal`, `DrawMedalChest`, `DrawMedalLongShot`, `MedalEstablish`, `PinMedal`, `DrawMedals`, … (+20) |
| `src/ship.c` | 2 | `the_creator`, `ResolveWc1ObjectDestruction` |
| `src/sound.c` | 7 | `DrawLaunchDoorFrame`, `LaunchPlayerShip`, `ShowCarrierLaunchSequence`, `FxDriverShutdownHook`, `InitializeDiskPromptTextContext`, `LoadInstallDat`, `GetJoystickPresentUnused` |
| `src/strdos.c` | 8 | `CopyFarString`, `GetEventManagerStatus`, `RegisterEventManagerShutdown`, `InitializeEventManager`, `ShutdownEventManager`, `ConfigureEventManagerPointer`, `EventManagerHook`, `SetEventManagerPump` |
| `src/system.c` | 3 | `RunTrainSim`, `LogWc1MemoryUsage`, `GetJoystickButtonEdge` |
| `src/text.c` | 1 | `show_info_disp` |
| `src/winmain.c` | 2 | `easy2see`, `WarpWc1MouseTo` |

The complete row-level mapping and unresolved list is in `reports/wc2-address-remap.tsv`.
All binary-comp rows, sorted from lowest to highest similarity, are in `reports/wc2-similarity.tsv`.

## Lowest-scoring mapped comparisons

| Source | Function | WC1 | WC2 | Evidence | Similarity |
| --- | --- | ---: | ---: | --- | ---: |
| `src/killbrd.c` | `DecodeShapeFrame` | `0x440960` | `0x425618` | `WC1-order-exact` | 52.69% |
| `src/debug.cpp` | `DebugOverlayPrintf` | `0x41CAB0` | `0x45B0BB` | `WC1-callgraph-propagated` | 62.25% |
| `src/logic.c` | `UpdateTargetCameraTracking` | -- | `0x4608E8` | `WC2-only` | 63.23% |
| `src/debug.cpp` | `DebugOverlayConsole::Clear` | `0x41CC00` | `0x45B2E0` | `WC1-order-exact` | 63.41% |
| `src/disk.c` | `FindCutsceneResourceSymbolIndex` | -- | `0x40D8D7` | `WC2-only` | 64.13% |
| `src/gr.c` | `PrepareShapeRLEData` | `0x440D50` | `0x425BF6` | `WC1-fuzzy-very-low` | 65.58% |
| `src/disk.c` | `LinkCutsceneObjectResources` | -- | `0x40DE5A` | `WC2-only` | 66.38% |
| `src/winmain.c` | `AllocateGuardedMemory` | `0x402BB0` | `0x455466` | `WC1-callgraph-propagated` | 66.67% |
| `src/brains.c` | `Build_objective_list` | `0x40CED0` | `0x44FE9F` | `WC1-fuzzy-very-low` | 66.87% |
| `src/system.c` | `ShowMemoryStatusDebug` | `0x4273C0` | `0x437AEC` | `WC1-fuzzy-very-low` | 67.96% |
| `src/gr.c` | `CaptureSpriteBackground` | `0x441450` | `0x426769` | `WC1-callgraph-propagated` | 68.85% |
| `src/hudmsg.c` | `arrive_from_warp` | `0x42A950` | `0x424AEE` | `WC1-fuzzy-followup` | 70.13% |
| `src/screens.c` | `InitializeCutsceneRuntimeResources` | -- | `0x42D227` | `WC2-only` | 70.16% |
| `src/logic.c` | `init_formation_burst` | `0x422B30` | `0x42A39A` | `WC1-fuzzy-very-low` | 70.27% |
| `src/gr.c` | `RestoreSpriteBackground` | `0x441740` | `0x426B96` | `WC1-callgraph-propagated` | 70.65% |
| `src/smart.c` | `intelligence_events` | `0x434A80` | `0x41FF37` | `WC1-callgraph-propagated` | 71.37% |
| `src/cockpt.c` | `ResetScannerContacts` | `0x415A70` | `0x43B258` | `WC1-manual-verified` | 71.43% |
| `src/gr.c` | `fizzle_fade` | `0x442200` | `0x428690` | `WC1-callgraph-propagated` | 71.59% |
| `src/cockpt.c` | `draw_nav_pointer` | `0x4168C0` | `0x43CBFD` | `WC1-fuzzy-very-low` | 71.88% |
| `src/debug.cpp` | `DebugOverlayConsole::Scroll` | `0x41CC50` | `0x45B348` | `WC1-callgraph-propagated` | 72.13% |
| `src/gr.c` | `GetTransformedShapeBounds` | `0x442050` | `0x4283A4` | `WC1-fuzzy-very-low` | 72.20% |
| `src/screens.c` | `ReleaseCutsceneViewport` | -- | `0x42EE86` | `WC2-only` | 72.50% |
| `src/disk.c` | `ReleaseCutsceneObjectResource` | -- | `0x40EAA4` | `WC2-only` | 73.71% |
| `src/screens.c` | `ReleaseLoadedCutsceneResource` | -- | `0x433328` | `WC2-only` | 74.58% |
| `src/nav.c` | `SelectNavObjectiveAtPoint` | `0x40E2B0` | `0x451C5C` | `WC1-fuzzy-very-low` | 74.77% |
| `src/logic.c` | `evaluate_damage` | `0x423C00` | `0x42B985` | `WC1-fuzzy-medium` | 75.00% |
| `src/cockpt.c` | `update_objective_location` | `0x415770` | `0x43AE2F` | `WC1-callgraph-propagated` | 75.00% |
| `src/cdrom.c` | `FindCdRomDriveByVolumeLabel` | `0x403290` | `0x456236` | `WC1-fuzzy-medium` | 75.00% |
| `src/logic.c` | `scan_for_enemy` | `0x422F80` | `0x42A8B5` | `WC1-fuzzy-very-low` | 75.28% |
| `src/nav.c` | `BriefingMap_DisplayMap` | `0x40E210` | `0x451B70` | `WC1-fuzzy-very-low` | 75.41% |
| `src/nav.c` | `PlaceNavMapLabel` | `0x40D2C0` | `0x450458` | `WC1-fuzzy-low` | 75.66% |
| `src/ship.c` | `Create_explosion_debris` | `0x41F800` | `0x413A3B` | `WC1-order-semantics-verified` | 75.81% |
| `src/logic.c` | `build_target_list` | `0x423440` | `0x42AE32` | `WC1-manual-verified` | 76.39% |
| `src/gr.c` | `DrawFontGlyph` | `0x441150` | `0x4261E5` | `WC1-fuzzy-low` | 76.59% |
| `src/cockpt.c` | `check_target` | `0x416FD0` | `0x43DDFC` | `WC1-callgraph-propagated` | 77.16% |
| `src/brains.c` | `cruise_home` | `0x409760` | `0x442770` | `WC1-fuzzy-very-low` | 78.06% |
| `src/disk.c` | `celerate` | `0x41E710` | `0x4117AC` | `WC1-callgraph-propagated` | 80.00% |
| `src/disk.c` | `steady_object` | `0x41E7C0` | `0x4118A9` | `WC1-manual-verified` | 80.00% |
| `src/cockpt.c` | `animate_pilot` | `0x4173C0` | `0x43E43A` | `WC1-fuzzy-followup` | 80.00% |
| `src/mathfp.c` | `RandomBelowOrEqual` | `0x434D50` | `0x4619A1` | `WC1-fuzzy-medium` | 80.00% |

## Reproduction

```sh
make wc2-remap-audit
make export-asm
make report
```
