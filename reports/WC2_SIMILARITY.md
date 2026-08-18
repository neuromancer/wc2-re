# WC2 address-remap similarity report

This report compares the current WC1 reconstruction against WC2 code at the reviewed WC1→WC2 function destinations. Unmapped functions are excluded rather than compared against arbitrary bytes.

The converted source contains WC2 labels only. Historical WC1 addresses live in the row-level migration manifest so a future `wc2-re` fork does not need a dual-game annotation or build path.

## Coverage and headline result

| Metric | Result |
| --- | ---: |
| WC1 source function markers | 1743 |
| Markers with a WC2 destination | 1455 (83.48%) |
| Explicitly unresolved markers | 288 |
| Functions compared by binary-comp | 1443 |
| Compared / mapped markers | 99.18% |
| Exact machine-code matches | 713 |
| Similarity >= 90% | 1104 |
| Similarity < 50% | 42 |
| Average WC2 similarity | 92.37% |
| Median WC2 similarity | 99.32% |
| Report errors / missing exports | 0 |

Low-confidence address assignments and genuinely changed code both depress the aggregate, so the evidence tiers below are more informative than the headline average on its own.

## Similarity by mapping evidence

| Evidence tag | Compared | Average | Median | Exact | >=90% | <50% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `WC1-auto-vt` | 180 | 99.21% | 100.00% | 151 | 177 | 0 |
| `WC2-manual-verified` | 12 | 97.30% | 100.00% | 7 | 11 | 0 |
| `manual-verified` | 29 | 96.11% | 100.00% | 19 | 28 | 0 |
| `WC2-only` | 230 | 95.91% | 100.00% | 122 | 204 | 0 |
| `WC1-fuzzy-high` | 14 | 95.14% | 98.60% | 6 | 13 | 0 |
| `WC1-callgraph-transfer` | 8 | 94.24% | 95.03% | 3 | 7 | 0 |
| `WC1-manual-verified` | 219 | 93.84% | 100.00% | 122 | 185 | 6 |
| `WC1-fuzzy-low` | 105 | 93.58% | 95.92% | 42 | 76 | 0 |
| `WC1-fuzzy-medium` | 57 | 92.71% | 100.00% | 30 | 41 | 1 |
| `WC1-order-exact` | 69 | 92.69% | 100.00% | 46 | 54 | 1 |
| `WC1-order-semantics-verified` | 24 | 92.09% | 95.84% | 12 | 18 | 0 |
| `WC1-callgraph-single-low` | 22 | 91.91% | 100.00% | 12 | 16 | 1 |
| `WC1-order-callgraph-verified` | 9 | 91.28% | 90.00% | 3 | 5 | 0 |
| `WC1-callgraph-single` | 16 | 90.40% | 98.22% | 8 | 12 | 1 |
| `WC1-fuzzy-followup` | 146 | 87.27% | 91.07% | 39 | 77 | 8 |
| `WC1-callgraph-propagated` | 198 | 86.41% | 95.12% | 68 | 122 | 15 |
| `WC1-fuzzy-very-low` | 102 | 86.02% | 92.08% | 23 | 58 | 7 |
| `same per-object update-loop caller; collision/vector callee family; 70.78% binary-comp; target 00445087 is capital-ship strike mission case` | 1 | 70.90% | 70.90% | 0 | 0 | 0 |
| `function order; parameter shape; unique BriefingMap_DisplayMap caller; scanner helper anchors; assembly control flow` | 2 | 42.67% | 42.67% | 0 | 0 | 2 |

## Similarity by source area

| Area | Compared | Average | Median | Exact | >=90% | <50% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| game/core | 1315 | 91.67% | 97.62% | 603 | 976 | 42 |
| ix audio | 128 | 99.56% | 100.00% | 110 | 128 | 0 |

## Explicit review flags

| Flag | Compared | Average | Median | Exact | >=90% | <50% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `WC1-review-module-mismatch` | 9 | 91.54% | 100.00% | 5 | 7 | 0 |

## Per-file summary

| Source file | Compared | Average | Median | Exact | >=90% | <50% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `src/auto.c` | 5 | 92.92% | 95.35% | 2 | 3 | 0 |
| `src/barracks.c` | 6 | 80.84% | 97.16% | 2 | 4 | 2 |
| `src/brains.c` | 139 | 90.48% | 98.84% | 69 | 103 | 7 |
| `src/cdrom.c` | 5 | 93.08% | 100.00% | 3 | 4 | 0 |
| `src/cmpgn.c` | 8 | 84.67% | 99.22% | 4 | 6 | 2 |
| `src/cockpt.c` | 124 | 88.63% | 96.53% | 57 | 80 | 10 |
| `src/debug.cpp` | 12 | 86.00% | 91.58% | 2 | 6 | 0 |
| `src/dib.c` | 26 | 92.85% | 94.44% | 2 | 20 | 0 |
| `src/disk.c` | 52 | 91.20% | 96.88% | 21 | 36 | 0 |
| `src/eventmgr.c` | 77 | 93.62% | 96.55% | 32 | 67 | 1 |
| `src/geom.c` | 87 | 95.62% | 100.00% | 52 | 74 | 0 |
| `src/gr.c` | 54 | 92.26% | 95.59% | 21 | 39 | 0 |
| `src/hudmsg.c` | 36 | 92.16% | 98.40% | 17 | 28 | 1 |
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
| `src/killbrd.c` | 12 | 91.52% | 98.15% | 6 | 8 | 0 |
| `src/logic.c` | 122 | 91.91% | 95.65% | 48 | 90 | 1 |
| `src/main.c` | 21 | 84.94% | 97.55% | 9 | 12 | 3 |
| `src/mathfp.c` | 28 | 94.14% | 96.43% | 13 | 22 | 0 |
| `src/mathutil.c` | 4 | 94.24% | 95.00% | 2 | 3 | 0 |
| `src/mono.c` | 13 | 91.27% | 100.00% | 10 | 11 | 2 |
| `src/music.c` | 45 | 91.45% | 100.00% | 26 | 32 | 1 |
| `src/nav.c` | 52 | 90.20% | 98.45% | 22 | 32 | 2 |
| `src/personnel.c` | 16 | 93.09% | 96.38% | 4 | 11 | 0 |
| `src/pilot.cpp` | 20 | 97.73% | 100.00% | 15 | 19 | 0 |
| `src/pload.c` | 10 | 92.96% | 98.12% | 4 | 9 | 0 |
| `src/screen.c` | 55 | 89.02% | 95.83% | 21 | 40 | 3 |
| `src/screens.c` | 107 | 96.11% | 100.00% | 71 | 96 | 1 |
| `src/ship.c` | 38 | 83.08% | 91.67% | 12 | 20 | 4 |
| `src/smart.c` | 24 | 91.64% | 92.56% | 4 | 14 | 0 |
| `src/sound.c` | 19 | 95.49% | 97.14% | 8 | 16 | 0 |
| `src/spc.c` | 22 | 81.73% | 87.51% | 3 | 8 | 2 |
| `src/strdos.c` | 11 | 99.09% | 100.00% | 9 | 11 | 0 |
| `src/sysinput.c` | 10 | 99.47% | 100.00% | 9 | 10 | 0 |
| `src/system.c` | 6 | 91.72% | 99.28% | 3 | 4 | 0 |
| `src/text.c` | 6 | 98.67% | 100.00% | 4 | 6 | 0 |
| `src/winmain.c` | 43 | 93.61% | 95.24% | 16 | 32 | 0 |

## Unresolved mapping inventory

| Source file | Count | Functions |
| --- | ---: | --- |
| `src/barracks.c` | 21 | `CreateEmptySaveGameFile`, `EnsureSaveGameFile`, `InitializeBarracksAnimation`, `FreeBarracksMenuLabel`, `SetAwakenBarracksMenuLabel`, `FreeBarracksMenuLabels`, `SaveGame`, `RunWc1TextInputPrompt`, … (+13) |
| `src/brains.c` | 18 | `AnimateScrambleWalk`, `DrawScrambleActor`, `ConfigureScrambleActor`, `DrawScrambleFrame`, `scramble`, `landing`, `funeral_player`, `funeral_wingman`, … (+10) |
| `src/cdrom.c` | 2 | `AllocateFontWorkspace`, `FreeFontWorkspace` |
| `src/cmpgn.c` | 13 | `LoadWc1PaletteTripletsFile`, `RunWc1StrandedSequence`, `ParseFaceAnimation`, `ParseMouthAnimation`, `AddPCName`, `LoadFace`, `LongTalk`, `CloseTalk`, … (+5) |
| `src/cockpt.c` | 3 | `FormatTextBufferFromStart`, `EraseCockpitReadoutAtPosition`, `PlayTargetLockSfx` |
| `src/debug.cpp` | 3 | `DebugOverlayConsole::~DebugOverlayConsole`, `DebugKeyboardHookProc`, `DebugOverlayConsole::SetOpaqueBackground` |
| `src/dib.c` | 2 | `SetWc1SpaceFlightFrameTiming`, `DirectDrawResultToText` |
| `src/disk.c` | 9 | `LoadWc1PacketIntoBuffer`, `SortSignedByteValuesAscending`, `CheckWc1DiskAvailable`, `PromptInsertNumberedDisk`, `GetZeroUnused`, `CheckEscaped`, `WaitForWc1SceneAdvance`, `MoveMenuPointerFromKeyboard`, … (+1) |
| `src/eventmgr.c` | 5 | `SuspendWc1MouseCursor`, `GetWc1VideoReleaseResult`, `Wc1ShutdownHook`, `shrink_vector`, `shrink` |
| `src/geom.c` | 11 | `SeekPacketSection`, `GetMusicDriverPresent`, `intfract_sign`, `SignShort`, `MakeRandomNormalizedVector`, `ConvertShortVectorToFixedVector`, `ship_vs_point`, `ship_vs_ship`, … (+3) |
| `src/gr.c` | 1 | `ClearStreamerTrigger` |
| `src/hudmsg.c` | 16 | `RunWc1KeyAcknowledge`, `GetArcadeBonus`, `FigureArcadeTime`, `DrawWc1ArcadeScorePanel`, `UpdateArcadeScoreDisplay`, `RefreshCockpitStatus`, `RunWc1SpaceFlight`, `UpdateWc1TrainSimMenuCursor`, … (+8) |
| `src/ix/dsp.cpp` | 2 | `ix_dsp_static_initializer`, `ix_dsp_static_initialization_hook` |
| `src/ix/mixer.cpp` | 2 | `ix_mixer_static_initializer`, `ix_mixer_static_initialization_hook` |
| `src/ix/thread.cpp` | 1 | `IxStreamFile::ix_stream_file_is_reading` |
| `src/killbrd.c` | 12 | `CorrectPointers`, `ClearRoomMenuLabel`, `IsRoomMenuLabelEmpty`, `DrawRoomMenuLabel`, `RefreshRoomMenuLabel`, `ClearRoomMenuCursorFrame`, `SelectRoomMenuLabel`, `InitializeRoomMenu`, … (+4) |
| `src/logic.c` | 35 | `find_weapon`, `drop_mine`, `LoadWc1GamePaletteFile`, `InitializeEventManagerResources`, `StartWc1EventManager`, `LoadWc1OriginFxDrivers`, `InitializeWc1DirectionViewFrames`, `LoadLegacySpaceflightResourceSets`, … (+27) |
| `src/main.c` | 4 | `RunWc1GameMain`, `init_player_input`, `SelectPreviousExternalViewObject`, `RunWc1FleetOverviewInput` |
| `src/mathfp.c` | 3 | `ReleaseVideoResourcesHook`, `MeasureShapeFrameStorage`, `ResetTextCursor` |
| `src/mono.c` | 5 | `MeasureScaledIntroTextWidth`, `DrawWc1CenteredScaledIntroText`, `GetLineLength`, `update_canned_sequence`, `ResetStringBuilder` |
| `src/music.c` | 10 | `parse_view_script`, `update_scripted_view`, `initialize_scripted_view`, `DecompressPacketSection`, `GetTargetColourIndex`, `AlignWc1SpriteFrameToRectCorner`, `SetMusicOn`, `SelectFlightMusicTrack`, … (+2) |
| `src/nav.c` | 13 | `NavMapPointInsideReservedArea`, `MergeAdjacentNearHeapBlocks`, `add_statistics`, `FullMissionScore`, `PlayersMissionScore`, `UpdateSeries`, `MoveNewCampaign`, `StartNewCampaign`, … (+5) |
| `src/pilot.cpp` | 24 | `WaitForKeyExceptXOrF12`, `ShowMeanwhileTransition`, `ApplyAnswerTextCipher`, `LoadAnswerPromptAndResponse`, `PromptForAnswerText`, `EraseTextContextBackground`, `DisplayTrainSimHighScoreTable`, `AnimateTrainSimTitle`, … (+16) |
| `src/pload.c` | 1 | `FindActiveSoundEntryBySample` |
| `src/screen.c` | 15 | `ShouldSuspendCursorForRect`, `FrameStartHook`, `CreateCannedSceneObject`, `ShowCampaignVictorySequence`, `ShowTigerClawEscapeScene`, `ShowWc1EndScreen`, `get_face`, `LoadCommPortraitShape`, … (+7) |
| `src/screens.c` | 30 | `DrawCutsceneTextAt`, `AnimateCutsceneSpeakerMouth`, `LoadBriefingRoom`, `ViewWc1Medals`, `AwardCampaignMedal`, `DrawMedalChest`, `DrawMedalLongShot`, `MedalEstablish`, … (+22) |
| `src/ship.c` | 3 | `check_for_lost_control`, `the_creator`, `ResolveWc1ObjectDestruction` |
| `src/smart.c` | 1 | `process_maneuver_node` |
| `src/sound.c` | 7 | `DrawLaunchDoorFrame`, `LaunchPlayerShip`, `ShowCarrierLaunchSequence`, `FxDriverShutdownHook`, `InitializeDiskPromptTextContext`, `LoadInstallDat`, `GetJoystickPresentUnused` |
| `src/strdos.c` | 8 | `CopyFarString`, `GetEventManagerStatus`, `RegisterEventManagerShutdown`, `InitializeEventManager`, `ShutdownEventManager`, `ConfigureEventManagerPointer`, `EventManagerHook`, `SetEventManagerPump` |
| `src/system.c` | 3 | `RunTrainSim`, `LogWc1MemoryUsage`, `GetJoystickButtonEdge` |
| `src/text.c` | 1 | `show_info_disp` |
| `src/winmain.c` | 4 | `RestoreGamePalette`, `easy2see`, `WarpWc1MouseTo`, `GetF1KeyLatch` |

The complete row-level mapping and unresolved list is in `reports/wc2-address-remap.tsv`.
All binary-comp rows, sorted from lowest to highest similarity, are in `reports/wc2-similarity.tsv`.

## Lowest-scoring mapped comparisons

| Source | Function | WC1 | WC2 | Evidence | Similarity |
| --- | --- | ---: | ---: | --- | ---: |
| `src/ship.c` | `inflict_damage` | `0x41E9B0` | `0x411C72` | `WC1-callgraph-propagated` | 36.41% |
| `src/main.c` | `player_input` | `0x4285D0` | `0x466B02` | `WC1-fuzzy-followup` | 37.45% |
| `src/music.c` | `DrawConstellationField` | `0x42D500` | `0x45A634` | `WC1-callgraph-propagated` | 37.66% |
| `src/logic.c` | `InitializeCockpitResources` | `0x4245B0` | `0x4577D7` | `WC1-callgraph-propagated` | 38.63% |
| `src/cmpgn.c` | `ejection_sequence` | `0x4046A0` | `0x44EBCA` | `WC1-callgraph-propagated` | 38.70% |
| `src/screen.c` | `BuildCommunicationCommandMenu` | `0x430FC0` | `0x447890` | `WC1-fuzzy-followup` | 38.91% |
| `src/nav.c` | `PostMission` | `0x40F010` | `0x42BB70` | `WC1-fuzzy-followup` | 39.16% |
| `src/nav.c` | `nav_note` | `0x40DF50` | `0x451756` | `WC1-callgraph-single` | 39.62% |
| `src/brains.c` | `load_ship` | `0x40B9F0` | `0x44BEE5` | `WC1-manual-verified` | 39.96% |
| `src/cockpt.c` | `draw_3d_scanner` | `0x415CE0` | `0x43B7C0` | `function order; parameter shape; unique BriefingMap_DisplayMap caller; scanner helper anchors; assembly control flow` | 40.34% |
| `src/brains.c` | `PlayScrambleHangarScene` | `0x4079C0` | `0x460650` | `WC1-fuzzy-very-low` | 40.36% |
| `src/brains.c` | `free_ship` | `0x40BC70` | `0x44C796` | `WC1-manual-verified` | 40.40% |
| `src/mono.c` | `print_subtitle` | `0x403920` | `0x42C0F1` | `WC1-fuzzy-very-low` | 41.21% |
| `src/main.c` | `get_player_input` | `0x427E40` | `0x4661C2` | `WC1-manual-verified` | 41.28% |
| `src/ship.c` | `fire_turrets` | `0x420AA0` | `0x4159DB` | `WC1-callgraph-propagated` | 41.61% |
| `src/cockpt.c` | `explosion_draw` | `0x417630` | `0x43E8B2` | `WC1-callgraph-propagated` | 42.11% |
| `src/brains.c` | `release_capital_ship_shapes` | `0x40B990` | `0x44BD83` | `WC1-manual-verified` | 42.19% |
| `src/barracks.c` | `GetBunkInfo` | `0x41BB20` | `0x433ED7` | `WC1-fuzzy-very-low` | 42.62% |
| `src/cockpt.c` | `vid_transmit` | `0x417910` | `0x43ECD9` | `WC1-fuzzy-followup` | 43.33% |
| `src/hudmsg.c` | `HandleSpaceFlightControls` | `0x429160` | `0x46733D` | `WC1-fuzzy-followup` | 43.54% |
| `src/screens.c` | `DrawBriefingCharacter` | `0x439150` | `0x4021A7` | `WC1-fuzzy-very-low` | 44.05% |
| `src/cockpt.c` | `print_message_text` | `0x416260` | `0x43C364` | `WC1-manual-verified` | 44.06% |
| `src/cockpt.c` | `overlay_head_up_display` | `0x416AC0` | `0x43D386` | `WC1-callgraph-propagated` | 44.61% |
| `src/cmpgn.c` | `UpdateMap` | `0x405CC0` | `0x42ECCB` | `function order; parameter shape; unique BriefingMap_DisplayMap caller; scanner helper anchors; assembly control flow` | 45.00% |
| `src/cockpt.c` | `update_missile_warning` | `0x417190` | `0x43E1B2` | `WC1-callgraph-propagated` | 45.00% |
| `src/ship.c` | `fire` | `0x4202D0` | `0x414DC0` | `WC1-fuzzy-followup` | 45.68% |
| `src/ship.c` | `send_appropriate_message` | `0x41E900` | `0x411A50` | `WC1-fuzzy-followup` | 45.76% |
| `src/main.c` | `process_player_input` | `0x427F20` | `0x4663A2` | `WC1-fuzzy-very-low` | 46.37% |
| `src/brains.c` | `Set_up_ship_info` | `0x40C5E0` | `0x44DB7E` | `WC1-callgraph-propagated` | 47.04% |
| `src/eventmgr.c` | `TranslatePolledInputEvent` | `0x4355F0` | `0x462625` | `WC1-callgraph-single-low` | 47.14% |
| `src/spc.c` | `set_eye_direction_and_position` | `0x410AF0` | `0x419D01` | `WC1-callgraph-propagated` | 47.24% |
| `src/cockpt.c` | `target_locking` | `0x416120` | `0x43C048` | `WC1-callgraph-propagated` | 47.62% |
| `src/brains.c` | `stationary_intelligence` | `0x40B110` | `0x444EA7` | `WC1-manual-verified` | 47.62% |
| `src/cockpt.c` | `hidden_objective` | `0x4151F0` | `0x43A474` | `WC1-callgraph-propagated` | 47.83% |
| `src/brains.c` | `strike_mission` | `0x40AAC0` | `0x444420` | `WC1-order-exact` | 47.93% |
| `src/barracks.c` | `DrawBarracksBunks` | `0x41BBD0` | `0x46138D` | `WC1-callgraph-propagated` | 48.10% |
| `src/mono.c` | `advance_canned_sequence` | `0x403A80` | `0x4207B0` | `WC1-fuzzy-very-low` | 48.72% |
| `src/screen.c` | `PrintPaletteAllocationMap` | `0x4319B0` | `0x401978` | `WC1-fuzzy-medium` | 48.84% |
| `src/cockpt.c` | `clear_head_up_display` | `0x415A90` | `0x43B29D` | `WC1-fuzzy-very-low` | 48.84% |
| `src/cockpt.c` | `npc_communication` | `0x4174F0` | `0x43E5DA` | `WC1-callgraph-propagated` | 49.02% |

## Reproduction

```sh
make wc2-remap-audit
make export-asm
make report
```
