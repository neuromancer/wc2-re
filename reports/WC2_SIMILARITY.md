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
| Exact machine-code matches | 918 |
| Similarity >= 90% | 1407 |
| Similarity < 50% | 0 |
| Average WC2 similarity | 96.54% |
| Median WC2 similarity | 100.00% |
| Report errors / missing exports | 0 |

Low-confidence address assignments and genuinely changed code both depress the aggregate, so the evidence tiers below are more informative than the headline average on its own.

## Similarity by mapping evidence

| Evidence tag | Compared | Average | Median | Exact | >=90% | <50% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `WC1-auto-vt` | 180 | 99.32% | 100.00% | 152 | 179 | 0 |
| `function order; parameter shape; unique BriefingMap_DisplayMap caller; scanner helper anchors; assembly control flow` | 2 | 98.33% | 98.33% | 1 | 2 | 0 |
| `manual-verified` | 28 | 98.19% | 100.00% | 22 | 28 | 0 |
| `WC1-fuzzy-high` | 14 | 97.97% | 99.82% | 7 | 14 | 0 |
| `WC1-order-exact` | 69 | 97.60% | 100.00% | 56 | 64 | 0 |
| `same per-object update-loop caller; collision/vector callee family; 70.78% binary-comp; target 00445087 is capital-ship strike mission case` | 1 | 97.56% | 97.56% | 0 | 1 | 0 |
| `WC2-manual-verified` | 17 | 97.51% | 100.00% | 10 | 16 | 0 |
| `WC1-callgraph-single-low` | 22 | 97.50% | 100.00% | 14 | 20 | 0 |
| `WC1-manual-verified` | 225 | 97.05% | 100.00% | 149 | 212 | 0 |
| `WC2-only` | 339 | 96.98% | 100.00% | 192 | 317 | 0 |
| `WC1-fuzzy-medium` | 57 | 96.52% | 100.00% | 36 | 49 | 0 |
| `WC1-fuzzy-low` | 105 | 96.26% | 100.00% | 56 | 92 | 0 |
| `WC1-callgraph-single` | 16 | 95.97% | 100.00% | 10 | 15 | 0 |
| `WC1-callgraph-propagated` | 198 | 95.72% | 100.00% | 101 | 173 | 0 |
| `WC1-fuzzy-followup` | 146 | 95.09% | 97.87% | 62 | 119 | 0 |
| `WC1-callgraph-transfer` | 8 | 94.24% | 95.03% | 3 | 7 | 0 |
| `WC1-order-semantics-verified` | 24 | 94.16% | 96.22% | 11 | 20 | 0 |
| `WC1-fuzzy-very-low` | 98 | 92.23% | 96.02% | 33 | 73 | 0 |
| `WC1-order-callgraph-verified` | 9 | 92.13% | 90.91% | 3 | 6 | 0 |

## Similarity by source area

| Area | Compared | Average | Median | Exact | >=90% | <50% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| game/core | 1430 | 96.27% | 100.00% | 808 | 1279 | 0 |
| ix audio | 128 | 99.56% | 100.00% | 110 | 128 | 0 |

## Explicit review flags

| Flag | Compared | Average | Median | Exact | >=90% | <50% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `WC1-review-module-mismatch` | 9 | 99.58% | 100.00% | 7 | 9 | 0 |

## Per-file summary

| Source file | Compared | Average | Median | Exact | >=90% | <50% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `src/auto.c` | 5 | 96.91% | 100.00% | 3 | 4 | 0 |
| `src/barracks.c` | 5 | 99.29% | 100.00% | 4 | 5 | 0 |
| `src/brains.c` | 154 | 97.24% | 100.00% | 102 | 140 | 0 |
| `src/cdrom.c` | 5 | 93.08% | 100.00% | 3 | 4 | 0 |
| `src/cmpgn.c` | 11 | 98.91% | 100.00% | 8 | 11 | 0 |
| `src/cockpt.c` | 133 | 96.70% | 100.00% | 85 | 120 | 0 |
| `src/debug.cpp` | 12 | 86.41% | 91.58% | 4 | 6 | 0 |
| `src/dib.c` | 26 | 96.07% | 97.47% | 5 | 24 | 0 |
| `src/disk.c` | 57 | 93.73% | 99.52% | 28 | 43 | 0 |
| `src/eventmgr.c` | 78 | 95.71% | 96.70% | 34 | 72 | 0 |
| `src/geom.c` | 87 | 96.79% | 100.00% | 55 | 79 | 0 |
| `src/gr.c` | 54 | 93.56% | 96.71% | 22 | 46 | 0 |
| `src/hudmsg.c` | 48 | 95.66% | 98.28% | 21 | 42 | 0 |
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
| `src/logic.c` | 144 | 95.90% | 100.00% | 78 | 128 | 0 |
| `src/main.c` | 25 | 96.57% | 100.00% | 17 | 23 | 0 |
| `src/mathfp.c` | 29 | 95.28% | 97.85% | 14 | 24 | 0 |
| `src/mathutil.c` | 4 | 94.24% | 95.00% | 2 | 3 | 0 |
| `src/mono.c` | 11 | 99.69% | 100.00% | 10 | 11 | 0 |
| `src/music.c` | 47 | 94.99% | 100.00% | 27 | 40 | 0 |
| `src/nav.c` | 57 | 95.90% | 100.00% | 30 | 47 | 0 |
| `src/personnel.c` | 20 | 97.80% | 97.56% | 7 | 20 | 0 |
| `src/pilot.cpp` | 21 | 98.67% | 100.00% | 16 | 21 | 0 |
| `src/pload.c` | 10 | 96.82% | 99.10% | 5 | 10 | 0 |
| `src/screen.c` | 58 | 96.31% | 100.00% | 34 | 54 | 0 |
| `src/screens.c` | 111 | 97.58% | 100.00% | 73 | 104 | 0 |
| `src/ship.c` | 43 | 97.19% | 100.00% | 24 | 39 | 0 |
| `src/smart.c` | 26 | 93.69% | 95.22% | 7 | 19 | 0 |
| `src/sound.c` | 19 | 96.22% | 96.77% | 7 | 18 | 0 |
| `src/spc.c` | 23 | 97.16% | 99.49% | 9 | 22 | 0 |
| `src/strdos.c` | 11 | 99.09% | 100.00% | 9 | 11 | 0 |
| `src/sysinput.c` | 23 | 99.77% | 100.00% | 22 | 23 | 0 |
| `src/system.c` | 9 | 93.55% | 100.00% | 5 | 7 | 0 |
| `src/text.c` | 6 | 98.85% | 100.00% | 4 | 6 | 0 |
| `src/winmain.c` | 45 | 97.09% | 100.00% | 27 | 43 | 0 |

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
| `src/main.c` | `house_keep` | `0x427D40` | `0x46604F` | `WC1-manual-verified` | 52.27% |
| `src/killbrd.c` | `DecodeShapeFrame` | `0x440960` | `0x425618` | `WC1-order-exact` | 52.69% |
| `src/brains.c` | `set_up_action_sphere` | `0x40BFF0` | `0x44D35D` | `WC1-manual-verified` | 54.15% |
| `src/logic.c` | `detect_collisions` | `0x4224F0` | `0x4299C9` | `WC1-callgraph-single` | 59.79% |
| `src/disk.c` | `remove_weapon` | `0x41E040` | `0x410715` | `WC1-manual-verified` | 61.38% |
| `src/logic.c` | `free_3Space_objects` | `0x424BE0` | `0x458716` | `WC1-manual-verified` | 61.54% |
| `src/debug.cpp` | `DebugOverlayPrintf` | `0x41CAB0` | `0x45B0BB` | `WC1-callgraph-propagated` | 62.25% |
| `src/logic.c` | `UpdateTargetCameraTracking` | -- | `0x4608E8` | `WC2-only` | 63.23% |
| `src/geom.c` | `init_ijk` | `0x418F60` | `0x40B41F` | `WC1-callgraph-propagated` | 63.24% |
| `src/debug.cpp` | `DebugOverlayConsole::Clear` | `0x41CC00` | `0x45B2E0` | `WC1-order-exact` | 63.41% |
| `src/music.c` | `servicetrack` | `0x42ECB0` | `0x453240` | `WC1-fuzzy-very-low` | 64.00% |
| `src/disk.c` | `FindCutsceneResourceSymbolIndex` | -- | `0x40D8D7` | `WC2-only` | 64.13% |
| `src/music.c` | `DrawTargetRangeReadout` | `0x42DEA0` | `0x43FF40` | `WC1-fuzzy-followup` | 64.58% |
| `src/screen.c` | `ExpandCommMessageTokens` | `0x4315C0` | `0x448136` | `WC1-fuzzy-very-low` | 65.00% |
| `src/gr.c` | `PrepareShapeRLEData` | `0x440D50` | `0x425BF6` | `WC1-fuzzy-very-low` | 65.58% |
| `src/geom.c` | `get_right_shape` | `0x41A610` | `0x40CFF8` | `WC1-callgraph-propagated` | 65.72% |
| `src/screen.c` | `cleanup_objectives` | `0x42EFC0` | `0x421910` | `WC1-fuzzy-followup` | 65.98% |
| `src/cockpt.c` | `flag_reached` | `0x415530` | `0x43AAFF` | `WC1-fuzzy-followup` | 66.01% |
| `src/hudmsg.c` | `select_new_release_weapon` | `0x42AE50` | `0x4616B8` | `WC1-fuzzy-very-low` | 66.21% |
| `src/disk.c` | `LinkCutsceneObjectResources` | -- | `0x40DE5A` | `WC2-only` | 66.38% |
| `src/winmain.c` | `AllocateGuardedMemory` | `0x402BB0` | `0x455466` | `WC1-callgraph-propagated` | 66.67% |
| `src/brains.c` | `Build_objective_list` | `0x40CED0` | `0x44FE9F` | `WC1-fuzzy-very-low` | 66.87% |
| `src/screen.c` | `show_communications_disp` | `0x431290` | `0x447E47` | `WC1-fuzzy-very-low` | 67.19% |
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

## Reproduction

```sh
make wc2-remap-audit
make export-asm
make report
```
