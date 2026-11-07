/*
 *  Globals shared by the reconstruction.
 *
 *  Names describe the state; declaration comments retain retail addresses
 *  for binary-comp audits without embedding addresses in identifiers.
 *  Numeric prefixes preserve MSVC's name-hashed allocation order where needed.
 *
 *  These live in one place only because the game-core compilation-unit
 *  boundaries are still unknown; each global belongs in whichever module
 *  defines it, and moves there as docs/ORDER.md is filled in.
 */
#ifndef GAME_GLOBALS_H
#define GAME_GLOBALS_H

extern const int g_anRLEQuarterCosine[901]; /* 0x00405cbb */
extern const unsigned char g_abIffBmhdChunkId[4]; /* 0x00406f74 */
extern const unsigned char g_abIffCmapChunkId[4]; /* 0x00406f78 */
extern const unsigned char g_abIffBodyChunkId[4]; /* 0x00406f7c */
extern short g_nHazardReferenceSpeed; /* 0x00492e58 */
extern short g_nActiveHazards; /* 0x00492e5c */
extern int g_nAudioEnabled; /* 0x0049c244 */
extern WaveTableEntry *g_1_pVoiceWav; /* 0x004a2650 */
extern IxSound *g_pSpeechSnd; /* 0x004a2658 */
extern int g_16_nSpeechCompletionDelay; /* 0x004a265c */
extern int g_bSpeechSoundActive; /* 0x004a2660 */
extern int g_1_bSpaceFlightActive; /* 0x005c586c */
extern int g_nFrameTimingMode; /* 0x0049ceb0 */
extern float g_fSpaceFlightFrameRate; /* 0x0049ceb4 */
extern long g_17_nFramePeriodMillis; /* 0x005c343c */
extern float g_1_fPreviousFrameSpeed; /* 0x005c3440 */
extern DWORD g_dwNextFrameDeadline; /* 0x0049cea4 */
extern long g_lPreviousFrameTick; /* 0x0049ceac */
extern int g_bFrameTimingInitialized; /* 0x0049cebc */
extern int g_nDebugFileLoggingEnabled; /* 0x0049c2d8 */
extern FILE *g_pDebugLogFile; /* 0x0049c2dc */
extern int g_bDebugBreakEnabled; /* 0x0049c238 */
extern int g_bApplicationShutdownStarted; /* 0x0049c23c */
extern int g_bF1KeyDown; /* 0x0049c240 */
extern int g_bInputEventQueueEnabled; /* 0x0049c248 */
extern int g_bApplicationControllerActive; /* 0x0049c25c */
extern int g_bShowFrameRate; /* 0x0049c260 */
extern int g_bConfigQuickModeEnabled; /* 0x0049c264 */
extern int g_bUseHardwarePalette; /* 0x0049c268 */
extern int g_bCockpitEnabled; /* 0x0049c26c */
extern int g_bSkipCampaignVideo; /* 0x0049c270 */
extern int g_bWindowInactive; /* 0x0049c274 */
extern int g_bConfigMemoryOption; /* 0x0049c2e0 */
extern int g_bWindowMessagePumpActive; /* 0x0049c2e4 */
extern int g_nPendingMouseX; /* 0x0049c2ec */
extern int g_nPendingMouseY; /* 0x0049c2f0 */
extern int g_bJoystickPrimaryButtonLatched; /* 0x0049c2f8 */
extern int g_bJoystickSecondaryButtonLatched; /* 0x0049c2fc */
/* The yellow, red and dark green palette colours take the header slots of
 * three names that became literals (WinMain's empty argument vector, nav's
 * label format and the conversation-viewport tag), keeping its symbol count. */
extern unsigned char g_cYellowColour; /* 0x0049cb58 */
extern FreedHeapBlock *g_pFreedHeapBlockHead; /* 0x0049c304 */
extern FreedHeapBlock *g_pFreedHeapBlockTail; /* 0x0049c308 */
extern unsigned int g_dwGuardedAllocationTotalBytes; /* 0x0049c24c */
extern unsigned int g_dwGuardedAllocationBytes; /* 0x0049c250 */
extern unsigned int g_dwGuardedAllocationPeakBytes; /* 0x0049c254 */
extern GuardedAllocation *g_pGuardedAllocationHead; /* 0x0049c300 */
extern short g_nPacketError; /* 0x0049ca90 */
extern short g_bRewritePacketExtensions; /* 0x0049cb48 */
extern unsigned char g_cBlueColour; /* 0x0049cb54 */
extern unsigned char g_abLegacyVideoModeColours[14]; /* 0x0049cb90 */
extern short g_bOriginFxDriverActive; /* 0x0049cbb0 */
extern char g_szInputCursorPacketName[12]; /* 0x0049cba0 */
extern int g_nMinimumMemoryNoMusic; /* 0x0049cc20 */
extern int g_nMinimumMemoryFullMusic; /* 0x0049cc24 */
extern int g_nMinimumMemoryAlternate; /* 0x0049cc28 */
extern unsigned short g_wSoundBlasterPort; /* 0x0049bb58 */
extern unsigned short g_wSoundBlasterIrq; /* 0x0049bb5c */
extern unsigned short g_wDigiPlaying; /* 0x0049bb60 */
/* Speech's DMA-address addition needs these two declaration keys exchanged;
 * other units retain their existing order (docs/PATTERNS.md). */
#ifdef SPEECH_C
extern const unsigned short g_wDmaHalfBytes; /* 0x0048e0e4 */
#else
extern unsigned short g_wDmaAddressA; /* 0x0049bb64 */
#endif
extern unsigned short g_wDmaAddressB; /* 0x0049bb68 */
/* The speech-position sum loads count B, count A, then position. */
#ifdef SPEECH_C
extern unsigned short g_2_wDigiPosition; /* 0x005d171c */
extern unsigned short g_wDmaCountA; /* 0x0049bb6c */
#else
extern unsigned short g_wDmaCountA; /* 0x0049bb6c */
extern unsigned short g_wDmaCountB; /* 0x0049bb70 */
#endif
extern int g_nDmaHalfSelect; /* 0x0049bb74 */
extern short g_nDigiLastBlock; /* 0x0049bb78 */
extern const int g_nJoystickUnavailableSample; /* 0x0048e054 */
extern const int g_nViewportStaticRandomRange; /* 0x0048e060 */
extern const int g_nDigiSampleBytes; /* 0x0048e0e0 */
#ifdef SPEECH_C
extern unsigned short g_wDmaAddressA; /* 0x0049bb64 */
#else
extern const unsigned short g_wDmaHalfBytes; /* 0x0048e0e4 */
#endif
extern short g_nAutopilotFormationShipCount; /* 0x00493060 */
extern unsigned char g_cRedColour; /* 0x0049cb5c */
#ifdef BRAINS_C
extern short g_nCurrentWave; /* 0x004931c0 */
#else
extern short g_nEnemySighting; /* 0x0049b670 */
#endif
extern short g_bHomeObjectiveFlagged; /* 0x0049b674 */
extern void **g_apPacketReferenceGroups[5]; /* 0x0049b898 */
extern short g_anPilotTurnInterval[8]; /* 0x0049b9c0 */
extern short g_nNavMapCoordinateScaling; /* 0x0049bb80 */
extern short g_nNavMapScale; /* 0x0049bb84 */
extern NavMapObjectiveStyle g_aNavMapObjectiveStyles[12]; /* 0x0049bb88 */
extern unsigned char *g_pNavMapShape; /* 0x0049bc48 */
extern unsigned char *g_pNavLocationText; /* 0x0049bc54 */
extern short g_nNavMapLabelCount; /* 0x0049bc4c */
extern short g_nNavMapReservedAreaCount; /* 0x0049bc50 */
extern char *g_apszShipMissionTypeNames[14]; /* 0x0049bc58 */
extern char g_szNoFurtherObjectives[32]; /* 0x0049bc90 */
extern short g_bBriefingMapActive; /* 0x0049bcb0 */
extern char g_szMissionPatrol[8]; /* 0x0049bcb8 */
extern char g_szMissionEscort[8]; /* 0x0049bcc0 */
extern char g_szMissionStrike[8]; /* 0x0049bcc8 */
extern char g_szMissionDefend[8]; /* 0x0049bcd0 */
extern char g_szMissionWingman[8]; /* 0x0049bcd8 */
extern char g_szMissionFlee[8]; /* 0x0049bce0 */
extern char g_szMissionGotoWarp[12]; /* 0x0049bce8 */
extern char g_szMissionWarpArrive[12]; /* 0x0049bcf4 */
extern char g_szMissionRendezvous[12]; /* 0x0049bd00 */
extern char g_szMissionComeHome[12]; /* 0x0049bd0c */
extern char g_szMissionTanker[8]; /* 0x0049bd18 */
extern char g_szMissionTow[4]; /* 0x0049bd20 */
extern char g_szMissionBreakOff[12]; /* 0x0049bd24 */
extern char g_szNavLabelTextFormat[12]; /* 0x0049bd30 */
/* dib.cpp's ten unreferenced words take these header slots, which once
 * named nav.c strings that are now literals. */
extern int g_28_nUnusedFrameSlot; /* 0x005c338c */
extern int g_nUnusedScreenWord1; /* 0x005c3394 */
extern int g_nUnusedScreenWord2; /* 0x005c3398 */
extern int g_4_nUnusedFrameWord; /* 0x005c339c */
extern int g_5_nUnusedDibWord3; /* 0x005c33a0 */
extern int g_nUnusedDibWord4; /* 0x005c3444 */
extern int g_nUnusedDibWord5; /* 0x005c3448 */
extern int g_2_nReservedDibWord; /* 0x005c344c */
extern int g_12_nIdleFrameWord1; /* 0x005c3964 */
extern int g_11_nIdleFrameWord2; /* 0x005c3968 */
extern char g_szNavBlankLine[4]; /* 0x0049bdb0 */
extern char g_szNavTitleFormat[8]; /* 0x0049bdb4 */
extern char g_szNavSectorFormat[12]; /* 0x0049bdbc */
extern char g_szNavSystemFormat[16]; /* 0x0049bdc8 */
extern char g_szNavMissionFormat[8]; /* 0x0049bdd8 */
extern char g_szNavShipFormat[8]; /* 0x0049bde0 */
extern char g_szNavNotesHeading[8]; /* 0x0049bde8 */
extern char g_szNavNoteFormat[4]; /* 0x0049bdf0 */
extern char g_szNavLocationFormat[48]; /* 0x0049bdf4 */
extern char g_szNavViewportName[8]; /* 0x0049be30 */
extern char g_szBriefingNavMapTitle[20]; /* 0x0049be38 */
extern char g_szConfedNavScan[16]; /* 0x0049be4c */
extern int g_bInflightComputerActive; /* 0x0049bcb4 */
extern volatile short g_nNearHeapActive; /* 0x00493044 */
extern volatile short g_nNearHeapMaxDescriptors; /* 0x00493048 */
extern volatile int g_nNearHeapRelocationBytes; /* 0x0049304c */
/* Unreferenced; dead names kept for the header's symbol count. */
extern char g_szCalculating[12]; /* 0x0049b3a0 */
extern char g_szRangeKilometresSuffix[4]; /* 0x0049b3ac */
extern char g_szNoObjective[8]; /* 0x0049b3b0 */
extern char g_szUnknownObjective[8]; /* 0x0049b3b8 */
extern char g_szCompNavigation[16]; /* 0x0049b3c0 */
extern char g_szDestinationFormat[20]; /* 0x0049b3d0 */
extern char g_szNavigationRangeLabel[12]; /* 0x0049b3e4 */
extern char g_szNewObjectivePrompt[20]; /* 0x0049b3f0 */
#ifdef MAIN_C
extern char *g_pszDisplayedHudMessage; /* 0x0049aff0 */
extern char *g_pszPendingHudMessage; /* 0x0049afec */
#else
extern char *g_pszPendingHudMessage; /* 0x0049afec */
extern char *g_pszDisplayedHudMessage; /* 0x0049aff0 */
#endif
extern unsigned char *g_pHudMessageFrameShape; /* 0x0049b288 */
extern unsigned char *g_pHudMessageBackground; /* 0x0049b28c */
extern short g_bCaptureHudMessageBackground; /* 0x0049b290 */
extern short g_nHudMessageBackgroundDepth; /* 0x0049b294 */
extern ShortPoint g_stHudMessageOrigin; /* 0x0049ae90 */
/* Viewport setup adds centres before origins in the event-manager unit. */
#ifdef EVENTMGR_C
extern short g_37_nViewCenterY; /* 0x005c80da */
#else
extern short g_49_nViewportOffsetY; /* 0x005c849c */
#endif
extern signed char g_3_cOverlayMessageView; /* 0x005d1c37 */
extern short g_bDisplayWingmanTargetData; /* 0x0049347c */
extern signed char g_cLastPilotHandFrame; /* 0x0049aff4 */
extern short g_asPilotHandOffsets[34]; /* 0x0049aff8 */
extern short g_nScannerTargetObject; /* 0x0049b07c */
extern Viewport g_stTrainSimRightVduSource; /* 0x0049b2c0 */
extern Viewport g_stTrainSimLeftVduSource; /* 0x0049b2d8 */
#ifdef COCKPT_C
extern int g_nCurrentObjectiveRange; /* 0x004931c8 */
#else
extern int g_nDisplayedObjectiveRange; /* 0x0049b078 */
#endif
extern int g_aiForwardScannerGridRows[78]; /* 0x0049b080 */
extern int g_aiTargetCameraScannerGridRows[50]; /* 0x0049b1b8 */
extern short g_nTargetLockMarkerX; /* 0x0049b298 */
extern int g_nCommPortraitFrame; /* 0x0049b2bc */
extern int g_bForceDamageDisplayRedraw; /* 0x0049b2ec */
/* winmain.c keeps this storage private: it defines WINMAIN_C before including the game
 * headers, so these become its statics at their header positions. */
#ifdef WINMAIN_C
#define WINMAIN_STATIC static
#else
#define WINMAIN_STATIC extern
#endif
WINMAIN_STATIC unsigned char g_13_abUnusedKeyboardArea[0x80]; /* 0x005b3698 */ /* never referenced */
extern char g_szNotEnoughMemoryMessage[]; /* 0x0049cbb8 */
/* Unreferenced: cockpt.c uses literals for these HUD reasons; the dead
 * names keep the header's symbol count. */
/* dib.cpp's two unreferenced words take these two header slots. */
extern int g_nUnusedDibWord1; /* 0x0049cea8 */
extern int g_nUnusedDibWord2; /* 0x0049ceb8 */
extern char g_szAlreadyVisited[16]; /* 0x0049b404 */
extern char g_szObjectiveReached[20]; /* 0x0049b414 */
extern char g_szWaitForFormat[12]; /* 0x0049b428 */
extern ShortRect g_stModalBounds; /* 0x0049ca48 */
/* Unreferenced word.  Declared in the slot the second modal-bounds dword
 * freed, so the header keeps its symbol count (docs/PATTERNS.md). */
extern short g_nUnusedModalWord; /* 0x005a86a6 */
extern ModalTextPanel *g_pModalTextPanel; /* 0x0049ca50 */
extern char *g_pszGameVersion; /* 0x0049b528 */
/* The version text is a literal in version.c; the dead name keeps the
 * header's symbol count. */
extern char g_szGameVersion[9]; /* 0x0049b52c */
extern short g_nPaletteTransitionInitialise; /* 0x0049305c */
extern int g_nDebugOverlayConsoleCount; /* 0x0049cb24 */
extern DWORD g_dwDebugOverlayKey; /* 0x0049cb28 */
extern DWORD g_dwDebugOverlayKeyLatch; /* 0x0049cb2c */
extern HHOOK g_hDebugKeyboardHook; /* 0x0049cb30 */
/* debug.cpp pools its font name, spinner and newline as literals; the dead
 * names keep the header's symbol count. */
extern char g_szDebugOverlayFontName[8]; /* 0x0049cb34 */
extern char g_szDebugOverlaySpinner[5]; /* 0x0049cb3c */
extern char g_szDebugOverlayNewline[4]; /* 0x0049cb44 */
extern short g_aasPlayerDamageSystemTable[5][10]; /* 0x00492d70 */
extern short g_aeShipHitDebrisTypes[4]; /* 0x00492e10 */
extern signed char g_acGunRefireDelay[4]; /* 0x00492e1c */
extern char g_szWeaponDestroyed[20]; /* 0x00492e20 */
extern char g_szFuelTanksHit[16]; /* 0x00492e34 */
extern char g_szComponentFixedFormat[8]; /* 0x00492e44 */
extern char g_szNeedLock[12]; /* 0x00492e4c */
extern unsigned char g_cSecondaryViewBufferColour; /* 0x0049cb4c */
extern int g_bPauseInputActive; /* 0x0049ac9c */
extern unsigned char g_bPrimaryViewBufferColour; /* 0x0049cb50 */
extern unsigned char g_ucHostileCapitalScannerColour; /* 0x0049cb68 */
extern unsigned char g_ucHomeCarrierScannerColour; /* 0x0049cb70 */
extern unsigned char g_ucDefaultTextColour; /* 0x0049cb7c */
extern unsigned char g_ucPrimaryTextColour; /* 0x0049cb64 */
extern unsigned char g_ucPlayerNavLabelColour; /* 0x0049cb74 */
extern unsigned char g_ucNavObjectiveMarkerColour; /* 0x0049cb78 */
extern unsigned char g_ucAsteroidNavMarkerColour; /* 0x0049cb84 */
extern unsigned char g_cPrimaryViewBufferColour; /* 0x0049cb88 */
extern short g_nCockpitBackgroundBytes; /* 0x0049c720 */
extern PacketResourceDescriptor g_aCommon3SpaceResources[11]; /* 0x0049c728 */
extern int g_nCapitalShipViewDistance; /* 0x00492fa4 */
extern short g_bAlternateChaseView; /* 0x00492fac */
extern short g_b3SpaceObjectsActive; /* 0x0049c8ec */
extern short g_bCockpitResourcesActive; /* 0x0049c8e8 */
extern unsigned char *g_pGenericMissileShape; /* 0x0049c8f0 */
extern unsigned char *g_pGenericMissileExhaustShape; /* 0x0049c8f4 */
extern PacketResourceDescriptor g_aMissionResourceDescriptors[8]; /* 0x0049c798 */
extern unsigned char *g_pPilotHandAnimationShapePtr; /* 0x005d2c64 */
extern PacketResourceDescriptor g_aHighMemoryCockpitResources[5]; /* 0x0049c7e8 */
extern PacketResourceDescriptor g_aCockpitCommonShapeResources[5]; /* 0x0049c820 */
extern PacketResourceDescriptor g_aCockpitShipShapeResources[8]; /* 0x0049c858 */
extern PacketResourceDescriptor g_aCockpitViewShapeResources[5]; /* 0x0049c8a8 */
/* Literals in RunPilotDatabaseMenu; dead names kept for the symbol count. */
extern char g_szDefaultPilotName[8]; /* 0x0049ab4c */
extern char g_szDefaultCallsign[12]; /* 0x0049ab54 */
extern int g_nTrainSimActive; /* 0x0049d758 */
extern short g_nCannedSceneMode; /* 0x0049021c */
extern char g_szCannedSceneTapeFile[9]; /* 0x00490208 */
extern short g_nCannedSceneBufferNearCapacityFlag; /* 0x00490214 */
#ifdef SPC_C
extern short g_nSpaceFrame; /* 0x00493134 */
#else
extern short g_nCannedSceneSegmentEndFrame; /* 0x00490218 */
#endif
extern short g_nCannedSceneStateRecordMarker; /* 0x00490220 */
/* taperec.c passes these codes as literals; the declarations hold the
 * header's symbol count (see the damage strings below). */
extern char g_abCannedSceneBufferOverflowCode[4]; /* 0x00490224 */
extern char g_szCannedSceneCreateError[4]; /* 0x00490270 */
extern char g_szCannedSceneWriteOpenError[4]; /* 0x00490274 */
/* A literal in replay.c; dead name kept for the header's symbol count. */
extern char g_szCannedSceneReadOpenError[4]; /* 0x0049b750 */
extern int g_bFullScreenSpaceView; /* 0x0049d718 */
extern int g_nArcadeState; /* 0x0049d75c */
extern short g_nFrameSkipCountdown; /* 0x0049d760 */
extern short g_nFrameSkip; /* 0x0049d764 */
extern short g_nSpaceFramePeriod; /* 0x0049d768 */
extern short g_nCockpitControlState; /* 0x0049d7ac */
extern InputDeviceSample g_stCurrentFlightInput; /* 0x0049d7b0 */
extern short g_wFlightInputButtonState; /* 0x0049d7c0 */
extern int g_bFlightButtonTwoHeld; /* 0x0049d7c8 */
extern int g_bFlightButtonOneHeld; /* 0x0049d7cc */
extern short g_nCockpitControlGoal; /* 0x0049d7d0 */
extern signed char g_bSecondaryMouseButtonHeld; /* 0x0049d7f4 */
extern signed char g_bMouseMoveEventQueued; /* 0x0049d7fc */
extern short g_nMouseYawInput; /* 0x0049d800 */
extern short g_nMousePitchInput; /* 0x0049d804 */
extern int g_bRenderingSpaceFrame; /* 0x0049d858 */
extern short g_bExplosionTraversalIdle; /* 0x00492e18 */
extern short g_nSpaceExplosionFlashActive; /* 0x00492fb4 */
extern short g_bApplyingCollisionDamage; /* 0x00492fb8 */
extern short g_nSpaceBlastFlashIncrement; /* 0x005c5872 */
extern short g_aSpaceExplosionFlashPalette[13][3]; /* 0x0049d808 */
extern short g_bJumpSequenceActive; /* 0x004962f0 */
extern short g_bStarSystemJumpTransition; /* 0x004962ec */
extern short g_nStarSystemJumpDelay; /* 0x004962f4 */
extern int g_1_nSavedPlayerObjectCounter; /* 0x005d2fe0 */
extern int g_nSavedWingmanObjectCounterValue; /* 0x005d2fdc */
extern short g_nOriginDevUnlock; /* 0x0049d774 */
extern short g_bCarrierLandingEnabled; /* 0x0049d778 */
extern short g_bPlayerDamageEnabled; /* 0x0049d77c */
extern short g_bMissionEndPending; /* 0x0049da4c */
extern short g_bDeathSequenceActive; /* 0x0049da50 */
extern short g_bPlayerCollisionEnabled; /* 0x0049d780 */
extern void *g_pMemoryAdjustment; /* 0x0049d788 */
extern signed char g_bParentDirectorySelected; /* 0x0049d78c */
/* The direct-launch flag and mission number are one packed record (the
 * mission sits at 0x49D79A).  C++ units never use it; a struct declaration
 * takes more symbol keys there, so they see the two dead scalar names. */
#ifdef __cplusplus
extern short g_bDirectMissionLaunch; /* 0x0049d798 */
extern short g_nDirectMission; /* 0x0049d79a */
#else
extern struct DirectMissionSelection {
    short launch;
    short mission;
} g_stDirectMissionSelection; /* 0x0049d798 */
#endif
extern short g_nDirectSeries; /* 0x0049d79c */
/* SDL port only: retail reads series 0 from the eight bytes before the
 * difficulty table (the end of joycal.c's literals); this models them.  The
 * declaration also holds the header's symbol count. */
extern short g_aasSeriesMissionTurnRate[1][4]; /* 0x004930a0 */
extern short g_nCurrentLevel; /* 0x005c5878 */
extern short g_nCurSeries; /* 0x005c5870 */
extern short g_asWingmanDamageCredit[10]; /* 0x005d3830 */
extern short g_7_asShipBurstStageCountdown[10]; /* 0x005d3850 */
extern signed char g_10_abExplosionEntityDoneBytes[
    SPACE_OBJECT_COUNT]; /* 0x005d3870 */
extern short g_asPlayerHitsCreditValues[10]; /* 0x005d38c0 */
extern short g_1_asObjectCreationFrameTable[SPACE_OBJECT_COUNT]; /* 0x005d3900 */
extern short g_asMissileProximityFuseValues[16]; /* 0x005d38e0 */
extern short g_asCapitalHitDamageStepTable[10]; /* 0x005d17a0 */
extern short g_2_asCarrierLaunchTimer[10]; /* 0x005d1090 */
extern short g_2_asFlakCooldownTable[16]; /* 0x005d1bd0 */
extern short g_nCarrierLaunchIntervalValue; /* 0x005d108c */
extern short g_nAdaptiveDifficulty; /* 0x005d3844 */
extern short g_asSpikeriCooldown[16]; /* 0x005d1bf0 */
extern short g_asSpikeriLaunchCountdown[10]; /* 0x005d1c10 */
extern signed char g_13_abShipArrivalMessageSent[16]; /* 0x005d1dd0 */
extern short g_bAiMissileFiringEnabled; /* 0x00492d58 */
extern short g_nLastAdaptiveDifficultyChangeFrame; /* 0x00492d60 */
extern short g_bFriendlyFireWarningIssued; /* 0x00492d5c */
extern short g_nObjectType62Index; /* 0x00492d64 */
extern short g_nObjectType63Index; /* 0x00492d68 */
extern short g_nLastFriendlyFireObjectFrame; /* 0x00492d6c */
extern short g_bTargetLockActive; /* 0x0049ae80 */
extern signed char g_bTargetBracketVisible; /* 0x0049b284 */
extern ShortRect g_stTargetBracketBounds; /* 0x0049b2a0 */
extern ShortRect g_stCommTargetBracketBounds; /* 0x0049b2a8 */
extern short g_bMissileLockAcquired; /* 0x0049b2b0 */
extern short g_1_nEnemyCommCommandBase; /* 0x005d179c */
extern short g_nEnemyCommPilotIndex; /* 0x005d179e */
extern int g_11_nMissionResourceBudget; /* 0x005c8de4 */
extern short g_asDifficultyLevels[16][4]; /* 0x004930a8 */
extern short g_asShipIntelSlot[10]; /* 0x00495d30 */
extern short g_asShipIntelResourceKey[10]; /* 0x00495d48 */
extern unsigned char g_aShipIntelligenceData[5][0x51]; /* 0x005d3060 */
extern short g_asShipIntelState[10]; /* 0x0049b678 */
extern signed char g_acShipPortrait[12]; /* 0x00495d88 */
extern short g_asShipTactic[12]; /* 0x00495f30 */
#ifdef COMM_C
extern short g_nCommPortraitIndex; /* 0x0049b79c */
#else
extern short g_nLoadedCommPortraitPilot; /* 0x004931c4 */
#endif
extern int g_nMissionMusicTrackOverride; /* 0x00496144 */
extern short g_nWingmanTargetingMode; /* 0x0049613c */
extern short g_bCockpitDamageFrame0Shown; /* 0x0049b2b4 */
extern short g_bCockpitDamageFrame2Shown; /* 0x0049b2b8 */
extern short g_bFuelGaugeDamaged; /* 0x0049b054 */
extern short g_bForceCriticalDamageWarning; /* 0x0049b058 */
extern short g_nPendingEjectionShip; /* 0x005d1bc4 */
extern short g_bMissionDeathSequencePending; /* 0x0049b720 */
extern short g_bMissionEjectionPodSpawned; /* 0x0049b724 */
/* A literal in replay.c; dead name kept for the header's symbol count. */
extern char g_szEndInflightReplay[20]; /* 0x0049b738 */
extern signed char g_acMissionShipStatusFlagsBytes[8]; /* 0x005d2fb0 */
extern short g_6_bMissionWingmanState; /* 0x005c8dbe */
extern signed char g_acJumpDriveUsedBySystem[5]; /* 0x005d2fe8 */
extern ShortPoint g_aDefaultWeaponDisplayPositions[16]; /* 0x0049afa0 */
extern short *g_pPilotStatus; /* 0x005d2fcc */
extern int g_nShapePrepareTicks; /* 0x005d2fd0 */
extern int g_1_nShapeClipTicksValue; /* 0x005d2fd4 */
extern int g_nShapeDrawTicks; /* 0x005d2fd8 */
extern CampaignGlobals *g_pCampaignGlobals; /* 0x00499c94 */
extern short g_bKilrathiAceGreetingSent; /* 0x00499bf4 */
extern CutsceneObjectResourceList *g_pCutsceneSpriteResources; /* 0x0049288c */
extern CutsceneObjectResourceList *g_pCutscenePlaneResources; /* 0x00492890 */
extern CutsceneObjectResourceList *g_pCutsceneSequenceResources; /* 0x00492894 */
extern CutsceneObjectResourceList *g_pCutsceneSceneResources; /* 0x00492898 */
extern CutsceneTextResource *g_pCutsceneTextResources; /* 0x0049289c */
extern CutsceneResourceTable *g_pCutsceneFontResources; /* 0x004928a0 */
extern CutsceneResourceTable *g_pCutsceneSpeechResources; /* 0x004928a4 */
extern CutsceneResourceTable *g_pCutsceneShapeResources; /* 0x004928a8 */
extern CutsceneResourceTable *g_pCutsceneMouseResources; /* 0x004928ac */
extern CutsceneResourceTable *g_pCutscenePaletteResources; /* 0x004928b0 */
extern CutsceneResourceTable *g_pCutsceneFilmResources; /* 0x004928b4 */
extern CutsceneResourceTable *g_pCutsceneFxResources; /* 0x004928b8 */
extern CutsceneResourceTable *g_pCutsceneMusicResources; /* 0x004928bc */
extern signed char g_cCutsceneVideoMode; /* 0x00499c48 */
extern signed char g_bCutsceneViewportPreallocated; /* 0x00499c4c */
extern SceneFlicObject **g_apSceneObjects; /* 0x00499c38 */
extern CutscenePlane **g_apCutscenePlanes; /* 0x00499c3c */
extern CutsceneSequence **g_apCutsceneSequences; /* 0x00499c40 */
extern CutsceneScene **g_apCutsceneScenes; /* 0x00499c44 */
extern short g_nSceneFlicContext; /* 0x00499c50 */
extern short g_nCutsceneResourceLevel; /* 0x00499d98 */
extern short g_nActiveCutsceneResourceLevel; /* 0x00499d9c */
extern void *g_pCutsceneCockpitPacket; /* 0x00499c04 */
extern void *g_pCutsceneCockpitPalette; /* 0x00499c0c */
extern unsigned char *g_13_pActiveCutscenePixelsAddress; /* 0x005c83dc */
extern short g_nCutsceneSpritesDrawn; /* 0x00499c10 */
extern const char *g_apszCutsceneObjectTypeNames[4]; /* 0x00499c18 */
extern CutsceneSoundEffect *g_pCutsceneSoundEffects; /* 0x00499c28 */
extern signed char g_cCutsceneSoundVolume; /* 0x00499c2c */
extern void *g_pCutsceneFxPacket; /* 0x00499c30 */
extern CutsceneMusicNode *g_pCutsceneMusicPackets; /* 0x00499c34 */
extern signed char g_bCutsceneSkipFrame; /* 0x00499c54 */
extern signed char g_bCutsceneSkipAll; /* 0x00499c58 */
extern void (*g_apCutsceneSpriteHandlers[1])(
    SceneFlicObject *sprite); /* 0x00499c5c */
extern signed char g_bCutsceneDrawingEnabled; /* 0x00499c60 */
extern SceneFlicObject *g_pLinkedCutsceneSprite; /* 0x00499c64 */
#ifdef GAMEFLOW_C
extern CutscenePlane *g_pCurrentCutscenePlane; /* 0x00499c7c */
extern CutsceneSequence *g_pCurrentCutsceneSequence; /* 0x00499c80 */
#else
extern CutscenePlane *g_pLinkedCutscenePlane; /* 0x00499c68 */
extern CutsceneSequence *g_pLinkedCutsceneSequence; /* 0x00499c6c */
#endif
extern SceneFlicObject *g_pCurrentCutsceneSprite; /* 0x00499c78 */
#ifdef GAMEFLOW_C
extern CutscenePlane *g_pLinkedCutscenePlane; /* 0x00499c68 */
extern CutsceneSequence *g_pLinkedCutsceneSequence; /* 0x00499c6c */
#else
extern CutscenePlane *g_pCurrentCutscenePlane; /* 0x00499c7c */
extern CutsceneSequence *g_pCurrentCutsceneSequence; /* 0x00499c80 */
#endif
extern CutsceneScene *g_pCurrentCutsceneScene; /* 0x00499c84 */
extern short g_nCutsceneFrameTick; /* 0x00499c88 */
extern unsigned short g_nCutsceneFrameDelay; /* 0x00499c8c */
extern unsigned char g_abCutscenePaletteTranslation[256]; /* 0x00499c98 */
extern signed char g_bCutsceneTextRestorePending; /* 0x00499da0 */
extern char *g_pszCurrentCutsceneText; /* 0x00499da4 */
extern signed char g_acCutsceneMouthFrames[128]; /* 0x00499db0 */
extern signed char g_acCutsceneMouthDurations[128]; /* 0x00499e30 */
extern char *g_pszCutsceneSpeechCursor; /* 0x00499eb0 */
extern signed char g_cCutsceneSpeechSpeed; /* 0x00499eb4 */
extern signed char g_bCutsceneSpeechActive; /* 0x00499eb8 */
#ifdef GAMEFLOW_C
extern volatile int g_196_nInputTimerTicks; /* 0x005c84a8 */
#else
extern signed char g_bCutsceneTextBoundsOverride; /* 0x00499edc */
#endif
extern int g_nNextCutsceneFrameClock; /* 0x00499c90 */
extern ShortRect g_stCutsceneTextBounds; /* 0x00499ee0 */
/* The three bounds below are fields of g_stCutsceneTextBounds;
 * the dead declarations keep the header's symbol count. */
extern short g_nCutsceneTextTop; /* 0x00499ee2 */
extern short g_nCutsceneTextRight; /* 0x00499ee4 */
extern short g_nCutsceneTextBottom; /* 0x00499ee6 */
extern signed char g_bCutsceneDebugOverlayEnabled; /* 0x00499ee8 */
extern unsigned short g_wCutsceneDebugFrameNumber; /* 0x00499eec */
extern CutscenePlane *g_pCutsceneSpriteParentPlane; /* 0x00499ef0 */
extern signed char g_cCutsceneTextStyle; /* 0x00499f40 */
extern char g_szCutsceneSpriteType[8]; /* 0x00499f44 */
extern char g_szCutscenePlaneType[8]; /* 0x00499f4c */
extern char g_szCutsceneSequenceType[12]; /* 0x00499f54 */
extern char g_szCutsceneScriptType[8]; /* 0x00499f60 */
extern signed char g_bSceneCockpitLoaded; /* 0x005d2d66 */
extern short g_8_nSavedCutsceneResourceOwnerValue; /* 0x005d2d68 */
extern short g_1_asCutsceneSpeechChannelsTable[8]; /* 0x005d2d70 */
extern Viewport g_10_stCutsceneStringViewportBlock; /* 0x005d2d90 */
extern char *g_27_pszCutsceneFormattedText; /* 0x005d2dc8 */
extern CutsceneResourceTable *g_1_pActiveSceneFileResourcesRef; /* 0x005d2da4 */
#ifdef GAMEFLOW_C
extern Viewport g_stSecondaryViewBufferBlock; /* 0x005d2c90 */
#else
extern Viewport g_9_stCutsceneTextBackingPort; /* 0x005d2db0 */
#endif
extern char *g_4_pszFilmSourceText; /* 0x005d2dcc */
extern short g_19_asCutsceneSpeechSectionsList[8]; /* 0x005d2dd0 */
extern signed char g_bMovieFramePresented; /* 0x005d2de0 */
extern TextContext g_6_stMovieDebugTextCtx; /* 0x005d2df0 */
extern unsigned int g_28_nCutsceneInitialAvailableMemoryWord; /* 0x005d2e88 */
extern short g_2_nCutsceneForkOffsetNumber; /* 0x005d2e8c */
extern unsigned char *g_26_pFilmRgbPacket; /* 0x005d2e98 */
extern short g_4_nCutsceneResourceScratchValue; /* 0x005d2e9c */
extern short g_3_nCutsceneTextIdx; /* 0x005d2ea4 */
extern unsigned int g_17_nSceneInitialLargestBlockWord; /* 0x005d2ea8 */
extern int g_47_nCutsceneMemDelta; /* 0x005d2ec8 */
extern char *g_4_pszFilmWorkScratch; /* 0x005d2ecc */
extern short g_2_bMovieStringAdvanceState; /* 0x005d2ed0 */
extern char *g_12_apszMovieSpeechFiles[8]; /* 0x005d2ee0 */
extern short *g_2_pCutsceneLocalsTemp; /* 0x005d2f00 */
extern InputEventState *g_19_pFilmInputFlag; /* 0x005d2f08 */
extern short g_117_nCutsceneFormatValueWord; /* 0x005d2f0c */
extern char *g_116_pszFilmPrintStore; /* 0x005d2f10 */
extern TextContext g_2_stCutsceneMessageCtxInfo; /* 0x005d2f40 */
extern TextContext g_16_stCutsceneDrawingTextSettingsInfo; /* 0x005d2f60 */
extern void *g_3_apSceneSpeechPacketsList[8]; /* 0x005d2f80 */
extern short g_1_nCutsceneFontOwnerNumber; /* 0x005d2fa0 */
extern short g_bSpaceViewBufferEnabled; /* 0x0049d7a4 */
extern int g_nCockpitDisplayMode; /* 0x0049d71c */
extern short g_nShowMemoryStatus; /* 0x0049d784 */
extern int g_bKeyboardMouseEnabled; /* 0x0049be68 */
extern int g_anShipCloakState[10]; /* 0x00496020 */
extern short g_asShipCloakCooldown[10]; /* 0x00496048 */
extern short g_asShipCloakElapsedFrames[10]; /* 0x00496060 */
extern signed char g_cReleaseWeaponDisplayFrame; /* 0x0049b060 */
extern short g_bWingmanTurnRateUnlocked; /* 0x00493040 */
extern short g_bEjectionSequencePending; /* 0x00493058 */
extern short g_bAutopilotSequenceActive; /* 0x00493064 */
extern signed char g_bAutopilotDebugEnabled; /* 0x00499bfc */
extern signed char g_bRoomTransitionAnimationEnabled; /* 0x00499c00 */
extern unsigned char g_bMemoryLogToFile; /* 0x00499bf8 */
extern FILE *g_pMemoryLogFile; /* 0x00499da8 */
/* wipe.c keeps this storage private: it defines WIPE_C before including the
 * game headers, so these become its statics at their header positions and
 * keep the symbol numbering; every other unit sees external declarations. */
#ifdef WIPE_C
#define WIPE_STATIC static
#else
#define WIPE_STATIC extern
#endif
WIPE_STATIC int g_18_nBlendRadialStep; /* 0x005b30f8 */
WIPE_STATIC int g_11_nWipeRadialAccumulator; /* 0x005b3100 */
WIPE_STATIC int g_1_nFadeRadialThreshold; /* 0x005b30fc */
WIPE_STATIC short g_3_asWipeRadialLeadingEdge[200]; /* 0x005b3108 */
WIPE_STATIC short g_asWipeRadialTrailingEdges[200]; /* 0x005b3298 */
WIPE_STATIC short g_5_nWipeMinorExtent; /* 0x005b3428 */
WIPE_STATIC signed char g_1_bWipeWidthAtLeastHeight; /* 0x005b342c */
WIPE_STATIC signed char g_bBlendWidthLessThanHeight; /* 0x005b3430 */
WIPE_STATIC short g_139_nDissolveProgress; /* 0x005b3434 */
WIPE_STATIC short *g_11_pWipeFrontEdges; /* 0x005b3438 */
WIPE_STATIC short *g_9_pWipeRearEdges; /* 0x005b343c */
WIPE_STATIC short g_18_nWipeTotalStages; /* 0x005b3440 */
WIPE_STATIC int g_17_nFadeType; /* 0x005b3444 */
WIPE_STATIC short g_15_nFadeWidth; /* 0x005b3448 */
WIPE_STATIC short g_5_nBlendHeight; /* 0x005b344c */
WIPE_STATIC Viewport g_19_stFadeToView; /* 0x005b3450 */
WIPE_STATIC short g_3_nFadeCurrentHorizontalEdge; /* 0x005b3464 */
WIPE_STATIC short g_nWipeCurrentVerticalEdges; /* 0x005b3468 */
WIPE_STATIC short g_2_nWipePreviousAcrossEdge; /* 0x005b346c */
WIPE_STATIC short g_105_nFadePreviousVerticalEdge; /* 0x005b3470 */
WIPE_STATIC int g_18_nWipeAcrossStride; /* 0x005b3478 */
WIPE_STATIC int g_14_nBlendHorizontalTotal; /* 0x005b3480 */
WIPE_STATIC int g_5_nWipeAcrossThreshold; /* 0x005b347c */
WIPE_STATIC Viewport g_3_stDissolveSourceViewport; /* 0x005b3488 */
WIPE_STATIC Viewport g_26_stWipeTempDestinationViewport; /* 0x005b34a0 */
WIPE_STATIC short g_3_nWipeRoundFlags; /* 0x005b34b4 */
WIPE_STATIC int g_16_nRevealVerticalDelta; /* 0x005b34b8 */
WIPE_STATIC int g_108_nTransitionVertSum; /* 0x005b34c0 */
WIPE_STATIC int g_nRevealVertThreshold; /* 0x005b34bc */
WIPE_STATIC short g_nBlendMaximumStep; /* 0x005b34c4 */
WIPE_STATIC Viewport g_1_stWipeScratchSourcePort; /* 0x005b34c8 */
extern short g_nWipeRadialRowCount; /* 0x005d17b4 */
extern short g_bExpandedShipGraphicsEnabled; /* 0x004931a4 */
extern short g_nExpandedShipMemoryKb; /* 0x0049cbac */
extern signed char g_acExpectedGraphicsModes[5]; /* 0x00493078 */
/* startup.c writes the near heap's descriptor limit at 0x00493048 through
 * g_nNearHeapMaxDescriptors; this second name is only kept for the
 * header's symbol count. */
extern short g_nGraphicsInitializationFlags; /* 0x00493048 */
extern short g_nInputDoubleClickInterval; /* 0x00493050 */
extern short g_nMenuPointerSpeed; /* 0x00493054 */
extern void *g_pHighMemoryBlockA; /* 0x004901f8 */
extern unsigned short g_wHighMemoryBlockBytes; /* 0x004901fc */
extern void *g_pHighMemoryBlockB; /* 0x00490200 */
extern short g_bSpeechResourcesEnabled; /* 0x0049cc2c */
extern short g_bDirectCampaignSelection; /* 0x0049cc74 */
extern short g_bSkipCampaignScenes; /* 0x0049cc78 */
extern short g_bPumpMessagesDuringLoad; /* 0x0049cc7c */
extern short g_bMemoryAdjustmentEnabled; /* 0x0049cc84 */
extern unsigned char g_ucPendingEjectionTransition; /* 0x0049b8ac */
extern short g_bEjectionWaitForEnemyWave; /* 0x0049b8b0 */
extern short g_bEjectionAwaitingCommCommand; /* 0x0049b8b4 */
extern short g_nPendingEjectionSequenceCount; /* 0x0049b8b8 */
extern short g_bEjectionTriggerImmediately; /* 0x0049b8bc */
extern short g_nEjectionSequenceState; /* 0x0049b8c0 */
extern signed char g_cQueuedNpcPortrait; /* 0x0049b8c4 */
extern signed char g_cQueuedNpcMessage; /* 0x0049b8c8 */
extern ShortVector g_aaFormationPositions[5][8]; /* 0x0049b8d0 */
extern short g_nCommDeathSequenceFrame; /* 0x0049ae84 */
extern short g_nCommMessageIndex; /* 0x0049ae88 */
extern ShortPoint g_stWeaponDisplayOffset; /* 0x0049ae8c */
/* The Y offset below is field y of g_stWeaponDisplayOffset; the
 * dead declaration keeps the header's symbol count. */
extern short g_nWeaponDisplayOffsetY; /* 0x0049ae8e */
extern ShortPoint g_aDamageDisplayPositions[9]; /* 0x00490068 */
extern const char *g_apszComponentNames[9]; /* 0x00490090 */
extern unsigned char g_abDamageDisplayFrames[9]; /* 0x004900b8 */
extern const char *g_apszDamageSeverityNames[5]; /* 0x004900c8 */
/* The original damage.c holds these strings as literals (its pointer tables'
 * initializers and use sites), so nothing defines the following names.  The
 * declarations stay because every unit's code depends on how many symbols
 * this header declares: removing them moves operands in about 90 functions.
 * They stand in for original declarations not yet identified. */
extern char g_szIonDrive[12]; /* 0x004900dc */
extern char g_szPowerPlant[12]; /* 0x004900e8 */
extern char g_szShieldGenerator[16]; /* 0x004900f4 */
extern char g_szComputerSystem[16]; /* 0x00490104 */
extern char g_szIntercomUnit[16]; /* 0x00490114 */
extern char g_szTargetTracking[16]; /* 0x00490124 */
extern char g_szAccelerationAbsorbers[16]; /* 0x00490134 */
extern char g_szEjectorSystem[16]; /* 0x00490144 */
extern char g_szRepairSystems[16]; /* 0x00490154 */
extern char g_szDamageOk[4]; /* 0x00490164 */
extern char g_szDamageLight[8]; /* 0x00490168 */
extern char g_szDamageModerate[12]; /* 0x00490170 */
extern char g_szDamageHeavy[8]; /* 0x0049017c */
extern char g_szDamageDestroyed[12]; /* 0x00490184 */
extern char g_szDamageReport[16]; /* 0x00490190 */
extern char g_szNoInternalDamage[20]; /* 0x004901a0 */
extern char g_szDamagedUnitCountFormat[20]; /* 0x004901b4 */
extern char g_szDamageStatusFormat[16]; /* 0x004901c8 */
extern char g_szDamageStatusFormatHighRes[16]; /* 0x004901d8 */
extern short g_nMissionEntryNavOverride; /* 0x0049d790 */
extern short g_asMouseYawThresholds[6]; /* 0x0049d7d8 */
extern short g_asMousePitchThresholds[6]; /* 0x0049d7e8 */
/* A literal in main.c; dead name kept for the header's symbol count. */
extern char g_szSpaceBuffer[16]; /* 0x0049d978 */
extern unsigned char g_cDarkGreenColour; /* 0x0049cb60 */
extern ActiveSoundEntry *g_pActiveSoundHead; /* 0x004961a8 */
extern ActiveSoundEntry *g_pActiveSoundTail; /* 0x004961ac */
extern int g_bAudioSystemInitialized; /* 0x004961b0 */
extern WaveTableEntry *g_pWaveTableHead; /* 0x004961b4 */
extern WaveTableEntry *g_pWaveTableTail; /* 0x004961b8 */
extern char g_szPlayWaveOpenError[36]; /* 0x004961ec */
extern short g_nMusicDriverMode; /* 0x0049be8c */
extern short g_nCutsceneMusicVolume; /* 0x0049be90 */
extern short g_nTitleMusicSequenceStage; /* 0x0049be94 */
/* The two limited-memory music buffers: packed {buffer, state} records, so
 * the second buffer pointer sits at 0x49BEA6.  Only C units use them; in C++
 * a struct declaration takes more symbol keys than in C, so the C++ units
 * see the four dead scalar names instead and keep their numbering. */
#ifdef __cplusplus
extern void *g_pLimitedMusicBufferA; /* 0x0049bea0 */
extern short g_nLimitedMusicBufferAState; /* 0x0049bea4 */
#else
#pragma pack(2)
extern struct LimitedMusicBuffer {
    void *buffer;
    short state;
} g_aLimitedMusicBuffers[2]; /* 0x0049bea0 */
#pragma pack()
#endif
extern void *g_pLimitedMusicBufferB; /* 0x0049bea6 */
extern short g_nLimitedMusicBufferBState; /* 0x0049beaa */
extern signed char g_acMusicResourcePresent[66]; /* 0x0049cc30 */
extern short g_nMusicTrackComplete; /* 0x0049be88 */
extern int g_nCurrentMusicTrack; /* 0x0049be98 */
extern unsigned char g_abSoundEffectDefinitions[0x1b8]; /* 0x0049bf18 */
extern short g_nWaitForMusicEnabled; /* 0x0049beac */
extern short g_bFlightSoundEffectsEnabled; /* 0x0049beb0 */
extern short g_nInitialFlightMusicPending; /* 0x0049bf00 */
extern short g_nCombatMusicActive; /* 0x0049bf04 */
extern short g_nInFlightMusicActive; /* 0x0049bf08 */
extern short g_nPassingShipSoundObject; /* 0x0049bf10 */
extern short g_nPassingShipSoundCountdown; /* 0x0049bf14 */
/* A literal in music.c; dead name kept for the header's symbol count. */
WIPE_STATIC int g_nWipeUnusedValue; /* 0x005b3474 */ /* never referenced */
extern unsigned char g_abTaggedAllocationPrefix[8]; /* 0x0049ca40 */
extern short g_nEnemyTauntCommandBase; /* 0x0049b76c */
extern short g_nCommMenuChoiceCount; /* 0x0049b770 */
extern short g_nCommMenuReuseMode; /* 0x0049b774 */
extern signed char g_cPendingCommMenuAction; /* 0x0049b77c */
extern signed char g_bRadioSilence; /* 0x0049b780 */
extern signed char g_bVideoImagesSuppressed; /* 0x0049b784 */
extern short g_bCommSpeechPlaying; /* 0x0049b7a0 */
extern signed char g_cCommMenuRecipient; /* 0x0049b790 */
extern short g_nCommSpeakerObject; /* 0x0049b794 */
extern short g_nCommSpeakerRating; /* 0x0049b798 */
#ifdef COMM_C
extern short g_nLoadedCommPortraitPilot; /* 0x004931c4 */
#else
extern short g_nCommPortraitIndex; /* 0x0049b79c */
#endif
extern unsigned char *g_pCommPortraitResource; /* 0x0049b788 */
extern unsigned char *g_pCommVduFrameResource; /* 0x0049b78c */
/* comm.c pools these strings as literals; the dead names below keep the
 * header's symbol count. */
extern char g_szSelectCommPrompt[8]; /* 0x0049b7c4 */
extern char g_szCommRecipientHeading[36]; /* 0x0049b7cc */
extern char g_szEnemyTarget[16]; /* 0x0049b7f0 */
extern char g_szUnidentifiedEnemyTarget[16]; /* 0x0049b800 */
extern char g_szUnidentifiedFriendlyTarget[16]; /* 0x0049b810 */
extern char g_szCommCompactHeadingPrefix[20]; /* 0x0049b820 */
extern char g_szCommHeadingPrefix[24]; /* 0x0049b834 */
extern char g_szUnidentifiedCommRecipient[16]; /* 0x0049b84c */
extern char g_szCommHeadingNewline[4]; /* 0x0049b85c */
extern char g_szChooseCommPrompt[8]; /* 0x0049b860 */
extern char g_szCommMenuChoiceFormat[8]; /* 0x0049b868 */
extern short g_19_bStoredHighMemoryEnabled; /* 0x005c80e4 */
extern short g_96_nHighMemoryInitValue; /* 0x005c82a4 */
extern short g_9_nStoredHighMemoryResourceSetupWord; /* 0x005c8458 */
extern short g_18_bEventManagerInitializationState; /* 0x005c80ca */
extern unsigned char g_88_nEventRepeatDelayState; /* 0x005c80d6 */
extern unsigned char g_26_bMouseCursorEnabled; /* 0x005c80e6 */
extern short g_18_nInterfaceEventMode; /* 0x005c8d3c */
extern short g_3_nVoiceBlasterIrqSetting; /* 0x005c8dd8 */
extern short g_5_nStorageAdjustmentKbValue; /* 0x005c8dda */
extern short g_2_bPumpMessagesWhileLoadingState; /* 0x005c8ddc */
extern short g_3_nAudioBlasterPortSettingValue; /* 0x005c8dde */
extern unsigned int g_dwInitialSpareMemory; /* 0x005c8dd0 */
extern short g_14_nCurrentGraphicsModeDetectedValue; /* 0x005c80d2 */
extern int g_98_nMemConfiguration; /* 0x005c8dc8 */
extern int g_8_nMusicMemoryNeed; /* 0x005c8dcc */
extern int g_nAvailableGameMemoryNumber; /* 0x005c8de0 */
extern short g_bSpeechEnabledFlag; /* 0x005c8de8 */
extern const char *g_pszPacketReadOperation; /* 0x00492b0c */
extern const char *g_pszPacketAllocateOperation; /* 0x00492b10 */
extern const char *g_pszPacketLoadErrorFormat; /* 0x00492b14 */
extern char g_szCommSpeakerTextFormat[8]; /* 0x0049b888 */
extern int g_nInputPressCount; /* 0x0049c258 */
extern short g_asConstellationVelocity[16]; /* 0x0049cae0 */
extern short g_asConstellationFrame[16]; /* 0x0049cb00 */
extern short g_nConstellationDirection; /* 0x0049cb20 */
/* No storage: 0x49B46C is the padding before target.c's 8-aligned data.
 * The dead name keeps the header's symbol count. */
extern unsigned short g_wPacketCompressionFormatFlags; /* 0x0049b46c */
extern ShortRect g_aTargetArmorClipRects[4]; /* 0x0049b470 */
extern char g_szEmptyTargetDisplayText[4]; /* 0x0049b490 */
extern char g_szLockedTarget[20]; /* 0x0049b494 */
extern char g_szTextColourStringColourFormat[8]; /* 0x0049b4a8 */
extern char g_szAutoTargetting[20]; /* 0x0049b4b0 */
extern char g_szTextColourStringFormat[8]; /* 0x0049b4c4 */
extern char g_szTargetLabel[12]; /* 0x0049b4cc */
extern char g_szNoTarget[8]; /* 0x0049b4d8 */
extern char g_szIdentifiedTargetNameFormat[4]; /* 0x0049b4e0 */
extern char g_szUnknownTarget[8]; /* 0x0049b4e4 */
extern char g_szShipTargetNameFormat[4]; /* 0x0049b4ec */
extern char g_szRangeLabel[12]; /* 0x0049b4f0 */
extern char g_szTargetTooFar[8]; /* 0x0049b4fc */
extern char g_szRangeMetresSuffix[4]; /* 0x0049b504 */
extern char g_szTargetOffscreenRange[8]; /* 0x0049b508 */
/* main.c keeps this storage private: it defines MAIN_C before including the
 * game headers, so these become its statics at their header positions. */
#ifdef MAIN_C
#define MAIN_STATIC static
#else
#define MAIN_STATIC extern
#endif
MAIN_STATIC int g_nFreeMainWord; /* 0x005c4b18 */ /* never referenced */
extern short g_nSpacePaletteFadeMode; /* 0x004901e8 */
/* dib.cpp defines the DirectDraw GUIDs; this one is missing from the
 * compiler's DirectX 2 ddraw.h (it takes the slot of an older name). */
extern const GUID CLSID_DirectDrawClipper;
extern LPDIRECTDRAW2 g_pDirectDraw2; /* 0x0049ce90 */
#ifdef BRAINS_C
extern short g_nEnemySighting; /* 0x0049b670 */
#else
extern short g_nCurrentWave; /* 0x004931c0 */
#endif
extern short g_bUseEyePositionForHazards; /* 0x0049327c */
extern signed char g_abHazardObjects[0x14]; /* 0x00493280 */
extern short g_nEyePitchRate; /* 0x0049314c */
extern short g_nEyeYawRate; /* 0x00493150 */
extern short g_nEyeRollRate; /* 0x00493154 */
extern signed char g_cCurrentObjective; /* 0x004931cc */
extern int g_nCurrentView; /* 0x00492fa8 */
extern short g_nSavedPlayerTarget; /* 0x0049d460 */
extern short g_nExternalViewShip; /* 0x00493468 */
extern short g_bSavedPlayerTarget; /* 0x0049d464 */
extern short g_asShipTurretFireEnabled[10]; /* 0x0049d470 */
extern short g_nSavedTargetLockCountdown; /* 0x0049d484 */
extern short g_bSavedTargetLockAcquired; /* 0x0049d488 */
extern signed char g_cViewObject; /* 0x0049313c */
/* CL 4.1 orders the wingman comparison by declaration keys.  logic.c
 * needs the wingman key five places later; other units keep this order. */
#ifdef LOGIC_C
extern short g_bTargetLockReadoutDirty; /* 0x004934e8 */
#else
extern short g_nYourWingman; /* 0x0049346c */
#endif
extern short g_nPlayerCollisionObject; /* 0x00493480 */
extern short g_nSelectedGunType; /* 0x004934dc */
extern short g_nSelectedReleaseWeaponIndex; /* 0x004934e0 */
extern short g_nCockpitResetState; /* 0x004934e4 */
#ifdef LOGIC_C
extern short g_nYourWingman; /* 0x0049346c */
#else
extern short g_bTargetLockReadoutDirty; /* 0x004934e8 */
#endif
extern short g_nTargetLockCountdown; /* 0x004934ec */
extern short g_nTargetLockMarkerAngle; /* 0x004934f0 */
extern signed char g_cTargetDisplayObject; /* 0x004934f4 */
extern short g_bTargetLockAcquired; /* 0x004934fc */
extern short g_bMissileCameraEnabled; /* 0x00493504 */
extern short g_bEngageAllowed; /* 0x0049612c */
extern short g_nAutoEngageTimer; /* 0x00496130 */
extern ShipIntelligenceMetadata g_stWingmanIntelligence; /* 0x00496138 */
/* The wingman's behaviour modes are the first fields of the record above;
 * these names stay declared for the header's symbol count. */
extern short g_nWingmanFormationDisobeyMode; /* 0x0049613a */
extern short g_nWingmanEngagementMode; /* 0x0049613e */
extern signed char g_cViableTargetCount; /* 0x00496178 */
extern int g_nAimCameraMode; /* 0x005c8d50 */
extern short g_1_nTargetCameraYawIndicatorX; /* 0x005c8d3e */
extern short g_1_nTargetLensYawIndicatorYWord; /* 0x005c8d40 */
extern short g_asAimCameraYawIndicatorBackground[3]; /* 0x005c8d48 */
extern short g_4_nTargetCameraPitchPipX; /* 0x005c8d6c */
extern short g_nTargetLensPitchIndicatorY; /* 0x005c8d6e */
extern short g_asTargetCameraPitchIndicatorBackgroundTable[3]; /* 0x005c8db0 */
extern short g_nTargetCameraZoom; /* 0x0049d3e4 */
extern short g_nTargetCameraObject; /* 0x0049d338 */
extern int g_nTargetCameraSoundHandle; /* 0x0049d3fc */
extern short g_nTargetCameraEffectObjectA; /* 0x0049d468 */
extern short g_nTargetCameraEffectObjectB; /* 0x0049d46c */
extern FixedVector g_6_vTargetCameraForce; /* 0x005c8d60 */
extern MissionNavPoint g_aMissionNavPoints[MISSION_NAV_POINT_COUNT]; /* 0x00491e98 */
extern MissionShipRecord
    g_aMissionShips[MISSION_SHIP_COUNT]; /* 0x00492290 */
extern signed char g_acDirectionShapeFrame[
    DIRECTION_VIEW_COUNT * DIRECTION_SHAPE_TABLE_COUNT]; /* 0x0049d558 */
extern signed char g_acDirectionShapeFlip[
    DIRECTION_VIEW_COUNT * DIRECTION_SHAPE_TABLE_COUNT]; /* 0x0049d618 */
extern InputEvent *g_pInputEventHead; /* 0x0049d4b4 */
extern InputEvent *g_pInputEventTail; /* 0x0049d4b8 */
extern int g_nInputEventPoolInitialized; /* 0x0049d4bc */
extern int g_nInputDoubleClickDeadline; /* 0x0049d4c0 */
extern int g_nPreviousPrimaryButton; /* 0x0049d4c4 */
extern int g_nPreviousSecondaryButton; /* 0x0049d4c8 */
extern ScreenViewportGeometry g_aScreenViewportGeometry[7]; /* 0x0049d4e8 */
extern short g_asIntelligenceEvent[12]; /* 0x00492fc0 */
extern signed char g_aDefenseManeuversNovice[8]; /* 0x00492fd8 */
extern signed char g_aDefenseManeuversVeteran[8]; /* 0x00492fe0 */
extern signed char g_aDefenseManeuversElite[16]; /* 0x00492fe8 */
extern signed char g_aDefenseManeuversAce[16]; /* 0x00492ff8 */
extern signed char g_aDefenseManeuversBoss[16]; /* 0x00493008 */
extern signed char *g_apDefenseManeuvers[5]; /* 0x00493018 */
extern signed char g_acPilotAggression[8]; /* 0x00493030 */
extern signed char g_acPilotRecovery[8]; /* 0x00493038 */
extern unsigned char *g_pDrawnInputCursorShape; /* 0x0049d4d0 */
extern int g_nMouseCursorDrawDepth; /* 0x0049d4d4 */
extern void *g_apRasterDriverCallbacks[13]; /* 0x00490278 */
extern char g_szRasterDriverName[13]; /* 0x004902ac */
extern unsigned char *g_pRLEEncodeBuffer; /* 0x004902b9 */
extern int g_nRLEPendingSkip; /* 0x004902bd */
extern unsigned char *g_pRLEScanlineStart; /* 0x004902c1 */
extern unsigned char *g_pRLEScanCursor; /* 0x004902c5 */
extern unsigned char *g_pRLEOutputCursor; /* 0x004902c9 */
extern unsigned char *g_pRLELiteralStart; /* 0x004902cd */
extern int g_nRLEEncodedMinimumX; /* 0x004902e1 */
extern int g_nRLEEncodedMinimumY; /* 0x004902e5 */
extern int g_nRLEEncodedMaximumX; /* 0x004902e9 */
extern int g_nRLEEncodedMaximumY; /* 0x004902ed */
extern unsigned char g_abRasterDecodeBuffer[0xd00]; /* 0x004907f1 */
extern unsigned char g_abRasterPaletteUsageScratch[0x300]; /* 0x004914f1 */
extern unsigned char g_abRasterFadeAccumulator[0x300]; /* 0x004917f1 */
extern const unsigned char g_abGifLZWCodeMasks[9]; /* 0x00491af1 */
extern const unsigned char g_abGifInterlaceRowSteps[5]; /* 0x00491afa */
extern const unsigned char g_abGifInterlaceRowStarts[5]; /* 0x00491aff */
extern RasterClip *g_pGifRasterTarget; /* 0x00491b04 */
extern unsigned char g_abRasterPaletteTranslation[256]; /* 0x00491b08 */
extern RLETransformVertex g_aRLETransformVertices[4]; /* 0x00491c08 */
extern int g_anRLESourceSteps[4]; /* 0x00491c58 */
extern const char *g_apszPilotRankNames[5]; /* 0x0049a608 */
/* The rank names are literals in personnel.c's tables; these dead names
 * keep the header's symbol count. */
extern char g_szSecondLieutenantRank[8]; /* 0x0049a6d8 */
extern char g_szFirstLieutenantRank[8]; /* 0x0049a6e0 */
extern char g_szCaptainRank[8]; /* 0x0049a6e8 */
extern char g_szMajorRank[8]; /* 0x0049a6f0 */
extern char g_szLieutenantColonelRank[12]; /* 0x0049a6f8 */
extern char g_szShortSecondLieutenantRank[8]; /* 0x0049a704 */
extern char g_szShortFirstLieutenantRank[8]; /* 0x0049a70c */
extern char g_szShortCaptainRank[8]; /* 0x0049a714 */
extern char g_szShortMajorRank[8]; /* 0x0049a71c */
extern char g_szShortLieutenantColonelRank[8]; /* 0x0049a724 */
extern unsigned char g_abPaletteTranslation[256]; /* 0x00496338 */
extern unsigned short g_awAbsoluteCosine[360]; /* 0x00496438 */
extern unsigned short g_awAbsoluteSine[360]; /* 0x00496708 */
WINMAIN_STATIC int g_nInsertKeyStatus; /* 0x005b3690 */
WINMAIN_STATIC int g_2_nClearedKeyboardState; /* 0x005b3694 */
WINMAIN_STATIC int g_3_nOemPeriodKeyState; /* 0x005b3718 */
WINMAIN_STATIC int g_3_nBeginKeyboardState; /* 0x005b371c */
WINMAIN_STATIC int g_1_nPageDownKeyboardMode; /* 0x005b3720 */
WINMAIN_STATIC int g_nOemCommaKeyState; /* 0x005b3724 */
WINMAIN_STATIC int g_nRightDirKeyboardState; /* 0x005b3728 */
WINMAIN_STATIC int g_18_nEndKeyState; /* 0x005b372c */
WINMAIN_STATIC int g_nClearKeyMode; /* 0x005b3730 */
WINMAIN_STATIC int g_nDeleteKeyState; /* 0x005b3734 */
WINMAIN_STATIC int g_13_nUpDirKeyState; /* 0x005b3738 */
WINMAIN_STATIC int g_15_nDownArrowKeyState; /* 0x005b373c */
#ifdef SDL_PORT
WINMAIN_STATIC char g_9_szStreamsPath[4096]; /* 0x005b3740 */
#else
WINMAIN_STATIC char g_9_szStreamsPath[0x100]; /* 0x005b3740 */
#endif
WINMAIN_STATIC int g_nPgUpwardKeyMode; /* 0x005b3840 */
WINMAIN_STATIC int g_102_nLeftDirKeyboardState; /* 0x005b3844 */
/* datafile.c keeps this storage private: it defines DATAFILE_C before including the game
 * headers, so these become its statics at their header positions. */
#ifdef DATAFILE_C
#define DATAFILE_STATIC static
#else
#define DATAFILE_STATIC extern
#endif
DATAFILE_STATIC char g_1_szReadDataFileFault[0x40]; /* 0x005b3848 */
DATAFILE_STATIC char g_szCreateBlockFileError[0x40]; /* 0x005b3888 */
DATAFILE_STATIC char g_szWriteDataFileError[0x40]; /* 0x005b38c8 */
DATAFILE_STATIC char g_szSeekDataFileFailure[0x48]; /* 0x005b3908 */
/* mono.c keeps this storage private: it defines MONO_C before including the
 * game headers, so these become its statics at their header positions. */
#ifdef MONO_C
#define MONO_STATIC static
#else
#define MONO_STATIC extern
#endif
MONO_STATIC int g_bMonoDebugInstalled; /* 0x005b30e8 */
MONO_STATIC HANDLE g_hMonoDebugDevice; /* 0x005b30ec */
extern WaveTableEntry *g_pSnowStaticClip; /* 0x004a2654 */
extern IxSound *g_9_pSnowStaticAudio; /* 0x004a2664 */
/* kbmouse.c keeps this storage private: it defines KBMOUSE_C before including the game
 * headers, so these become its statics at their header positions. */
#ifdef KBMOUSE_C
#define KBMOUSE_STATIC static
#else
#define KBMOUSE_STATIC extern
#endif
KBMOUSE_STATIC int g_nKeyboardMouseButtons; /* 0x005b3648 */
/* music.c keeps its wave-path buffer private (MUSIC_C); the SDL music
 * adapter shares it, so the port keeps the external declaration. */
#if defined(MUSIC_C) && !defined(SDL_PORT)
#define MUSIC_STATIC static
#else
#define MUSIC_STATIC extern
#endif
MUSIC_STATIC char g_szSfxWavePath[0x40]; /* 0x005b3650 */
/* fl.c keeps this storage private: it defines FL_C before including the
 * game headers, so these become its statics at their header positions. */
#ifdef FL_C
#define FL_STATIC static
#else
#define FL_STATIC extern
#endif
FL_STATIC short g_nShipAiScratchValue; /* 0x005b30f0 */
FL_STATIC unsigned char g_bCurrentManeuverReroll; /* 0x005b30f4 */
extern DebugOverlayConsole *g_pDiagnosticOverlay; /* 0x004a2548 */
extern char g_4_szSystemDebugBuffer[256]; /* 0x004a2550 */
/* DIBslamReal's row loop and the DIB copy-size expressions need this
 * declaration order to reproduce the retail operands (docs/PATTERNS.md). */
#ifdef DIB_C
extern unsigned int g_38_nSurfaceHeight; /* 0x005b3980 */
extern unsigned int g_28_nDisplayRowBytes; /* 0x005b397c */
extern unsigned char *g_18_pDisplayPixelBuffer; /* 0x005b3978 */
extern unsigned int g_101_nDibOffsetY; /* 0x005b3974 */
extern unsigned int g_38_nDibOriginX; /* 0x005b3970 */
#else
extern unsigned int g_38_nDibOriginX; /* 0x005b3970 */
extern unsigned int g_101_nDibOffsetY; /* 0x005b3974 */
extern unsigned char *g_18_pDisplayPixelBuffer; /* 0x005b3978 */
extern unsigned int g_28_nDisplayRowBytes; /* 0x005b397c */
extern unsigned int g_38_nSurfaceHeight; /* 0x005b3980 */
#endif
extern unsigned char g_11_abFrameBackingStore[0xfa00]; /* 0x005b3988 */
extern int g_36_nDibBitsEachPx; /* 0x005c3388 */
extern int g_4_nDibShowCount; /* 0x005c3390 */
extern HWND g_2_hFrameHwnd; /* 0x005c33a4 */
extern unsigned char g_3_abColourShadow[1024]; /* 0x005c3450 */
extern int g_bDibSlamEnabled; /* 0x0049cec0 */
extern char g_18_szSpaceFlightBestFpsMessage[0x80]; /* 0x005c3970 */
/* gr.c keeps its raster and shape scratch storage in statics; these dead
 * "Slot" names hold the header's symbol count. */
/* gr.c keeps this storage private: it defines GR_C before including the
 * game headers, so these become its statics at their header positions. */
#ifdef GR_C
#define GR_STATIC static
#else
#define GR_STATIC extern
#endif
GR_STATIC RasterSurface g_1_stRasterBitmap; /* 0x004a2670 */
GR_STATIC unsigned char g_abShapeTransformScratch[0xfa00]; /* 0x004a2688 */
GR_STATIC RasterClip g_stScreenLimits; /* 0x004b2088 */
GR_STATIC unsigned char g_abSolidColourTranslation[256]; /* 0x004b2710 */
GR_STATIC unsigned char g_13_abShapeRLEScratch[0x100000]; /* 0x004b2810 */
extern unsigned char *g_pMissionCampaignData; /* 0x005988bc */
extern PilotRecord g_aPilotRecords[9]; /* 0x005988d0 */
extern ConstellationObjectDefinition
    *g_pConstellationDefinitions; /* 0x00598a28 */
extern PilotRecord *g_apWingmanPilots[8]; /* 0x00598a30 */
extern unsigned char *g_apRecRoomTextData[3]; /* 0x00598aa0 */
extern char g_16_szTextBufferBuffer[256]; /* 0x005d1c40 */
/* Queued mouse state: the unaligned flags and pointers fix this packed
 * record at 0x005C83F0-0x005C840B in the retail image. */
#pragma pack(push, 1)
typedef struct QueuedMouseState {
    short x;                         /* +0x00 */
    short y;                         /* +0x02 */
    unsigned char primaryButton;      /* +0x04 */
    unsigned char secondaryButton;    /* +0x05 */
    unsigned char field_6;            /* +0x06 */
    unsigned short flags;             /* +0x07 */
    unsigned char *shape;             /* +0x09 */
    short frame;                     /* +0x0D */
    unsigned int field_f;             /* +0x0F */
    Viewport *viewport;               /* +0x13 */
    unsigned int field_17;            /* +0x17 */
    unsigned char shapeChanged;       /* +0x1B */
} QueuedMouseState;
#pragma pack(pop)
extern QueuedMouseState g_16_stEventQueuePointerData; /* 0x005c83f0 */
/* Retain these predecessor declarations only where C1 needs their symbol
 * slots; its C++ front end assigns five more keys to the packed record. */
#ifndef __cplusplus
extern short *g_pMouthAnimationCommands; /* 0x00598af4 */
extern unsigned char *g_pDebriefingSceneData; /* 0x00598afc */
extern unsigned char *g_pBriefingSceneData; /* 0x00598c00 */
extern unsigned char *g_pConversationBackdropShape; /* 0x00598c04 */
extern short g_nConversationMedalIndex; /* 0x00598c08 */
#endif
/* PollMenuInputDevices reads flags at +7, proving this packed prefix.
 * The original record's extent beyond these nine bytes is not yet known. */
#pragma pack(push, 1)
typedef struct PersonnelMouseSample {
    short x;                         /* +0x00 */
    short y;                         /* +0x02 */
    unsigned char primaryButton;      /* +0x04 */
    unsigned char secondaryButton;    /* +0x05 */
    unsigned char field_6;            /* +0x06 */
    unsigned short flags;             /* +0x07 */
} PersonnelMouseSample;
#pragma pack(pop)
/* Its type replaces two obsolete C symbols, or seven C++ symbols. */
#ifndef __cplusplus
extern short g_nConversationTextColour; /* 0x00598c10 */
extern unsigned char *g_pBriefingAnimationShape; /* 0x00598c14 */
extern short *g_pFaceAnimationCommands; /* 0x00598c18 */
extern unsigned char *g_pBriefingBodyShape; /* 0x00598c1c */
extern unsigned char *g_pBriefingPortraitShape; /* 0x00598c24 */
#endif
extern unsigned char *g_pDebriefingTextData; /* 0x00598c28 */
extern unsigned char *g_pBriefingCloseupShape; /* 0x00598c2c */
extern unsigned char *g_pConversationOverlayShape; /* 0x00598c30 */
extern InputEvent g_3_aInputEventArray[0x100]; /* 0x005c5890 */
extern int g_nStreamerAudioPlaying; /* 0x005b2810 */
#ifdef SDL_PORT
extern char g_szStreamerDirectory[4096]; /* 0x005b2818 */
#else
extern char g_szStreamerDirectory[128]; /* 0x005b2818 */
#endif
extern unsigned char g_bInputMode; /* 0x0059a848 */
extern int g_11_anPlacedObjectDepths[SPACE_OBJECT_COUNT]; /* 0x005c8180 */
extern short g_nUnusedInputWord1; /* 0x0059a9f0 */
extern signed char g_1_cStoredViewDisplayModeValue; /* 0x005c82a6 */
extern const ScreenViewportGeometry * volatile
    g_15_pScreenViewportGeometry; /* 0x005c82b0 */
#ifdef EVENTMGR_C
extern short g_11_nViewportLeftOrigin; /* 0x005c849e */
extern short g_49_nViewportOffsetY; /* 0x005c849c */
#else
extern short g_15_nStoredScreenViewCentreX; /* 0x005c80d8 */
extern short g_37_nViewCenterY; /* 0x005c80da */
#endif
extern unsigned char g_77_abKeyboardEventStates[0x80]; /* 0x005c80f0 */
extern MouseCursorState g_37_stPersonnelMousePointerState; /* 0x005c8470 */
extern void (*g_pfnUnusedInputCallback)(void); /* 0x0059ab2c */
extern volatile int g_5_nFrameTimerWaitTicksValue; /* 0x005c844c */
#ifdef EVENTMGR_C
extern short g_15_nStoredScreenViewCentreX; /* 0x005c80d8 */
#else
extern short g_11_nViewportLeftOrigin; /* 0x005c849e */
#endif
extern short g_nUnusedInputWord2; /* 0x0059ab64 */
extern int g_3_anCurrentInputEventEntryAllocation[0x100]; /* 0x005c87e0 */
extern short g_nQueuedInputX; /* 0x005c83f0 */
extern short g_nQueuedInputY; /* 0x005c83f2 */
extern unsigned char g_bQueuedPrimaryButton; /* 0x005c83f4 */
extern unsigned char g_bQueuedSecondaryButton; /* 0x005c83f5 */
extern unsigned short g_wQueuedInputFlags; /* 0x005c83f7 */
extern unsigned char *g_pInputCursorShape; /* 0x005c83f9 */
extern short g_nInputCursorFrame; /* 0x005c83fd */
extern unsigned char g_bInputCursorShapeChanged; /* 0x005c840b */
/* eventmgr.c keeps its found-event record and cursor background private
 * (EVENTMGR_C); the SDL adapters share them, so the port keeps them external. */
#if defined(EVENTMGR_C) && !defined(SDL_PORT)
#define EVENTMGR_STATIC static
#else
#define EVENTMGR_STATIC extern
#endif
EVENTMGR_STATIC unsigned char g_abInputCursorBehind[0x1000]; /* 0x005c3b10 */
extern int g_15_nMouseRenderY; /* 0x005c80a8 */
extern int g_4_nDrawnCursorX; /* 0x005c80ac */
extern unsigned char *g_39_pCurrentCampaignDataOffset; /* 0x005c80c0 */
extern int g_6_bCurrentInputCursorBackgroundCapturedState; /* 0x005c80c4 */
extern int g_10_nSavedPointerY; /* 0x005c8178 */
extern int g_4_nCapturedInputCursorX; /* 0x005c817c */
extern int g_26_bPointerDirtyValue; /* 0x005c8450 */
extern int g_138_nCursorDirtyRightValue; /* 0x005c845c */
extern int g_15_nPointerDirtyBottom; /* 0x005c8460 */
extern int g_148_nPointerDirtyLeft; /* 0x005c85c0 */
extern int g_146_nInputCursorDirtyTop; /* 0x005c85c4 */
EVENTMGR_STATIC InputEventState g_1_stFoundInputEvent; /* 0x005c3af8 */
extern signed char g_9_bCinematicSpriteTextEnableStatus; /* 0x005c82a7 */
extern TextContext *g_29_pCurrentTextContext; /* 0x005c8d1c */
extern void (*g_36_pfnRenderSceneFontCharacterHandler)(char character); /* 0x005c8d30 */
extern void (*g_3_pfnRenderSceneTextStringHandler)(char *text); /* 0x005c8cf4 */
extern short g_27_aasGunCooldown[10][3]; /* 0x005c8d70 */
extern short g_nUnusedInputWord3; /* 0x0059ab34 */
extern int g_56_anStoredObjectSortedIndicesData[SPACE_OBJECT_COUNT]; /* 0x005c82c0 */
extern FixedVector g_aObjectViewPosition[SPACE_OBJECT_COUNT]; /* 0x0059afa0 */
#if defined(SPC_C)
extern short g_anObjectYawRotation[SPACE_OBJECT_COUNT]; /* 0x00494fc8 */
#elif defined(COCKPT_C)
extern short g_anObjectRollRotation[SPACE_OBJECT_COUNT]; /* 0x00495058 */
#else
extern short g_anObjectPitchRotation[SPACE_OBJECT_COUNT]; /* 0x00494f38 */
#endif
extern int g_anShipSpeed[SPACE_OBJECT_COUNT]; /* 0x00494e20 */
extern int g_9_nNextSfx16PlaybackFrame; /* 0x004a2668 */
#ifdef SPC_C
extern short g_nCannedSceneSegmentEndFrame; /* 0x00490218 */
#else
extern short g_nSpaceFrame; /* 0x00493134 */
#endif
extern short g_nCurrentNavPoint; /* 0x004931bc */
extern short g_nCurStellarSystem; /* 0x005d169c */
/* The current pilot profile's score and ship type fields; these names stay
 * declared only for the header's symbol count. */
extern short g_nMissionScore; /* 0x00493462 */
extern short g_nPlayerShipType; /* 0x00493464 */
extern short g_bMissionShipInitializationInProgress; /* 0x0049b890 */
extern signed char g_bLandingCommRequestPending; /* 0x00492fa0 */
extern short g_bTargetLockMode; /* 0x00493500 */
extern short g_nEscapedEnemyCount; /* 0x004962e8 */
/* The turret unit needs these vector keys earlier to reproduce both the
 * gun-origin sums and the camera-eye multiply operands. */
#ifdef TURRET_C
extern FixedVector g_aShipRightVector[SPACE_OBJECT_COUNT]; /* 0x00493b78 */
extern FixedVector g_aShipForwardVector[SPACE_OBJECT_COUNT]; /* 0x00494208 */
#else
extern FixedVector g_vPlayerAcceleration; /* 0x00493488 */
extern short g_asObjectDistance[SPACE_OBJECT_COUNT]; /* 0x00493ae8 */
#endif
extern short g_asObjectDrawX[SPACE_OBJECT_COUNT]; /* 0x00493748 */
extern short g_asObjectDrawY[SPACE_OBJECT_COUNT]; /* 0x004937d8 */
extern short g_bShipResourceReloadInProgress; /* 0x0049b894 */
extern unsigned char *g_apObjectExhaustShape[10]; /* 0x004953b8 */
#ifdef TURRET_C
extern FixedVector g_vPlayerAcceleration; /* 0x00493488 */
#else
extern FixedVector g_aShipRightVector[SPACE_OBJECT_COUNT]; /* 0x00493b78 */
#endif
extern FixedVector g_aShipUpVector[SPACE_OBJECT_COUNT]; /* 0x00493ec0 */
#ifdef TURRET_C
extern short g_asObjectDistance[SPACE_OBJECT_COUNT]; /* 0x00493ae8 */
#else
extern FixedVector g_aShipForwardVector[SPACE_OBJECT_COUNT]; /* 0x00494208 */
#endif
extern HazardField *g_pActiveHazardField; /* 0x00493278 */
extern signed char g_acPlayerComponentDamage[9]; /* 0x00493470 */
extern short g_asShipMaximumVelocity[12]; /* 0x00495f70 */
extern unsigned int g_anShipAlertFlags[10]; /* 0x00495f88 */
extern signed char g_acShipAlertSource[12]; /* 0x00495fb0 */
extern short g_asShipAlertCountdown[12]; /* 0x00495fc0 */
extern signed char g_acObjectType[SPACE_OBJECT_COUNT]; /* 0x00493980 */
extern short g_asObjectAnimationIndex[SPACE_OBJECT_COUNT]; /* 0x00494c70 */
extern signed char g_acObjectAnimationDelay[
    SPACE_OBJECT_COUNT]; /* 0x00494d00 */
#ifdef SDL_PORT
/* Eight objectives plus the terminator Build_objective_list always appends.
 * Retail lets that ninth byte overlap the first objective's map coordinate. */
extern signed char g_abFlightPath[
    MISSION_OBJECTIVE_COUNT + 1]; /* 0x004932a0 */
#else
extern signed char g_abFlightPath[MISSION_OBJECTIVE_COUNT]; /* 0x004932a0 */
#endif
extern FixedVector g_aShipVelocity[SPACE_OBJECT_COUNT]; /* 0x00494898 */
extern short g_anYawGoal[12]; /* 0x004954c0 */
extern short g_asObjectCounter[SPACE_OBJECT_COUNT]; /* 0x00494be0 */
extern short g_nTargetShip; /* 0x004931a0 */
#ifdef SDL_PORT
/* Read and written by object index throughout the game -- objects run to 66 --
 * while the original's array has room for twelve ships and simply runs into
 * the per-ship arrays that follow it in .data.  The port gives it the extent
 * its indices actually need, so those accesses stay inside it; the reference
 * build keeps the original's twelve and its original neighbours. */
extern short g_aeSpecialManeuver[SPACE_OBJECT_COUNT]; /* 0x00495600 */
#else
extern short g_aeSpecialManeuver[12]; /* 0x00495600 */
#endif
extern int g_anShipFuel[10]; /* 0x00495638 */
extern short g_asShipMissionType[12]; /* 0x00495de8 */
extern MissionShipRecord *g_apShipMissionRecord[10]; /* 0x00495da8 */
extern signed char g_acShipPendingMessage[16]; /* 0x00495d98 */
extern short g_asShipMissionIndex[12]; /* 0x00495d00 */
extern short g_asShipMissionParameter[12]; /* 0x00495e00 */
extern FixedVector g_aShipMissionSpot[20]; /* 0x00495e18 */
extern short g_asShipCount[12]; /* 0x00495ff8 */
extern signed char g_acShipDamage[16]; /* 0x00495690 */
extern short g_asViableTargetDistance[10]; /* 0x00496190 */
extern signed char g_cCurrentNavPointIndex; /* 0x00493298 */
extern FixedVector g_aShipPosition[SPACE_OBJECT_COUNT]; /* 0x00494550 */
extern short g_asObjectScreenScale[SPACE_OBJECT_COUNT]; /* 0x00493a58 */
extern short g_asShipAfterburnerTimer[10]; /* 0x004955a8 */
extern signed char g_acShipComputerDamage[16]; /* 0x00495670 */
extern signed char g_cMissionObjectiveCount; /* 0x00493294 */
extern short g_asObjectFlip[SPACE_OBJECT_COUNT]; /* 0x004939c8 */
extern signed char g_acViableTarget[16]; /* 0x00496180 */
extern short g_asActionCount[12]; /* 0x004955e8 */
extern short g_nHazardFieldCount; /* 0x004931d0 */
extern unsigned short g_asObjectScale[SPACE_OBJECT_COUNT]; /* 0x00494d90 */
extern CampaignState g_stCampaignState; /* 0x0059ca50 */
extern unsigned char g_aShipWeapons[10][0xA1]; /* 0x004956b0 */
extern short g_asObjectScreenAngle[SPACE_OBJECT_COUNT]; /* 0x004936b8 */
extern short g_nTargetRange; /* 0x0049319c */
extern short g_asObjectDamage[SPACE_OBJECT_COUNT]; /* 0x00495178 */
extern signed char g_acObjectOwner[SPACE_OBJECT_COUNT]; /* 0x00495208 */
extern short g_asObjectType[SPACE_OBJECT_COUNT]; /* 0x00495298 */
extern signed char g_acShipTarget[16]; /* 0x00495f20 */
extern short g_anPreviousFlightInput[3]; /* 0x004931b0 */
extern short g_nPreviousYawInput; /* 0x004931b2 */
extern short g_nPreviousRollInput; /* 0x004931b4 */
#ifdef SPC_C
extern short g_anObjectPitchRotation[SPACE_OBJECT_COUNT]; /* 0x00494f38 */
#else
extern short g_anObjectYawRotation[SPACE_OBJECT_COUNT]; /* 0x00494fc8 */
#endif
extern signed char g_acTurnRegulator[16]; /* 0x00495618 */
extern short g_asPilotLevel[12]; /* 0x00495d60 */
extern short g_asShipListRange[16]; /* 0x00496158 */
extern signed char g_acPilotHitPoints[16]; /* 0x00495d78 */
/* Original class[] array.  The recovered ObjectClass order explains all of the
 * observed magic values: 0 null, 1 futurion, 10 mine, 11 missile, 12 ship and
 * 13 capital_ship. */
extern short g_aeObjectClass[SPACE_OBJECT_COUNT]; /* 0x00495328 */
extern short g_aeShipObjective[12]; /* 0x00495f08 */
extern short g_asShipIdentified[12]; /* 0x00496078 */
extern short g_asShipFriendlyFireCooldown[10]; /* 0x00496090 */
extern short g_asObjectViewFrame[SPACE_OBJECT_COUNT]; /* 0x00493508 */
#ifdef COCKPT_C
extern int g_nDisplayedObjectiveRange; /* 0x0049b078 */
#else
extern int g_nCurrentObjectiveRange; /* 0x004931c8 */
#endif
extern signed char g_acShipLastAttacker[16]; /* 0x004955c0 */
extern unsigned char *g_apObjectShape[SPACE_OBJECT_COUNT]; /* 0x00493868 */
extern short g_anFlightInput[3]; /* 0x004931a8 */
/* Pitch, yaw and roll are one array in the original; the remaining old names
 * stay declared for the header's symbol count. */
extern short g_nYawInput; /* 0x004931aa */
extern short g_nRollInput; /* 0x004931ac */
extern short g_nFleetOverviewYawVelocity; /* 0x0049d3ec */
extern short g_nFleetOverviewPitchVelocity; /* 0x0049d3f0 */
extern short g_nFleetOverviewYaw; /* 0x0049d3f4 */
extern short g_nFleetOverviewPitch; /* 0x0049d3f8 */
extern short g_asShipWingLeader[12]; /* 0x00495dd0 */
extern short g_aasShipArmor[10][4]; /* 0x00495540 */
extern short g_asShipWeaponEnergy[10]; /* 0x00495590 */
extern signed char g_acShipList[16]; /* 0x00496148 */
extern FixedVector g_vToTarget; /* 0x00493188 */
extern int g_aaiVduModeStack[3][4]; /* 0x00493498 */
extern signed char g_acShipSequence[14]; /* 0x00495fe8 */
extern short g_nTargetFacing; /* 0x00493198 */
extern signed char g_acShipNavPoint[12]; /* 0x004953e0 */
extern FixedVector g_aShipDestination[10]; /* 0x004953f0 */
extern short g_aasShipShield[10][2]; /* 0x00495518 */
extern signed char g_acShipExhaustHeat[16]; /* 0x00495660 */
extern short g_nRenderedSpaceFrame; /* 0x00493138 */
extern int g_6_nPendingAfterburnerSfxFrame; /* 0x005d3820 */
extern signed char g_acShipStress[16]; /* 0x00496100 */
extern signed char g_abPilotEjectionAttempted[16]; /* 0x00496110 */
extern signed char g_abShipEjectionSequenceEnabled[12]; /* 0x00496120 */
extern short g_anRollGoal[12]; /* 0x004954d8 */
extern short g_asShipSide[12]; /* 0x004955d0 */
extern signed char g_acShipCollisionCooldown[16]; /* 0x00496010 */
#ifndef SDL_PORT
extern short g_asProjectileEvasionTicks[8]; /* 0x004960f0 */
#else
/* Projectile-hit evasion timers are indexed by all ten ship slots.  Retail
 * only reserves eight words, so slots 8 and 9 overlap the stress bytes. */
extern short g_asProjectileEvasionTicks[10]; /* 0x004960f0 */
#endif
extern FixedVector g_vCollisionDelta; /* 0x00493178 */
extern signed char g_acLastCollisionObject[SPACE_OBJECT_COUNT]; /* 0x00495250 */
extern signed char g_abProjectileCollisionBonus[
    SPACE_OBJECT_COUNT]; /* 0x004960a8 */
extern short g_aasShipMaximumShield[10][2]; /* 0x004954f0 */
extern short g_asObjectCollisionRadius[SPACE_OBJECT_COUNT]; /* 0x004950e8 */
extern short g_anPitchGoal[12]; /* 0x004954a8 */
extern signed char g_abShipNavPointIndex[16]; /* 0x00495f60 */
extern signed char g_acTurnInterval[16]; /* 0x00495628 */
#ifdef COCKPT_C
extern short g_anObjectPitchRotation[SPACE_OBJECT_COUNT]; /* 0x00494f38 */
#else
extern short g_anObjectRollRotation[SPACE_OBJECT_COUNT]; /* 0x00495058 */
#endif
extern signed char g_abShipTurn[16]; /* 0x00495fd8 */
extern short g_nFacingToTarget; /* 0x00493194 */
extern short g_nNavPointerObject; /* 0x004931b8 */
extern short g_asObjectScreenY[SPACE_OBJECT_COUNT]; /* 0x00493628 */
extern short g_asObjectScreenX[SPACE_OBJECT_COUNT]; /* 0x00493598 */
extern int g_anCloakVisibilityPattern[20]; /* 0x00492710 */
extern int g_abObjectShapeOwned[SPACE_OBJECT_COUNT]; /* 0x00492760 */
extern FixedVector g_vStarFieldMotion; /* 0x00493158 */
extern FixedVector g_vPreviousStarFieldMotion; /* 0x00493168 */
extern short g_nStarFieldJRotationNumber; /* 0x005d320c */
extern short g_1_nStarFieldIRotation; /* 0x005d3210 */
extern HazardField g_aHazardFields[8]; /* 0x004931d8 */
#ifdef SDL_PORT
/* cleanup_objectives reads the next type before checking the eight-entry
 * limit.  Reserve that discarded read without changing the retail loop. */
extern MissionObjective g_aMissionObjectives[
    MISSION_OBJECTIVE_COUNT + 1]; /* 0x004932a8 */
#else
extern MissionObjective g_aMissionObjectives[MISSION_OBJECTIVE_COUNT]; /* 0x004932a8 */
#endif
extern char g_szAutopilotWaitReason[28]; /* 0x0049b2f0 */
extern char *g_pszAutopilotWaitReason; /* 0x0049b050 */
extern short g_asShipManeuver[12]; /* 0x00495f48 */
extern short g_asLoadedShipViewFrame[12]; /* 0x00495d18 */
extern signed char g_acObjectCollisionGraceTicks[
    SPACE_OBJECT_COUNT]; /* 0x00494d48 */
extern ObjectResourceSlot g_aObjectResourceSlots[5]; /* 0x00493398 */
extern signed char g_acShipCommunicator[16]; /* 0x00495680 */
extern signed char g_acShipIonDriveDamage[16]; /* 0x004956a0 */
extern short g_nEyeRollGoal; /* 0x00493148 */
extern short g_nEyeYawGoal; /* 0x00493144 */
extern short g_nEyePitchGoal; /* 0x00493140 */
extern signed char g_acVduModeStackDepth[2]; /* 0x004934c8 */
extern int g_nJoystickCalibrationMinimumY; /* 0x005d2ff0 */
extern int g_2_nJoystickCalibrationMinimumXNumber; /* 0x005d2ff4 */
extern int g_2_nControllerCalibrationMaximumX; /* 0x005d3000 */
extern int g_7_nJoystickTuningTopY; /* 0x005d3004 */
extern char g_szOnScreenMessageBuffer[52]; /* 0x005d1890 */
extern char g_szCommMessageTextText[0x58]; /* 0x005d18f0 */
extern char *g_48_apCommunicationTextPacketsTable[50]; /* 0x005d17c0 */
extern signed char g_abCommMenuChoiceCommand[7]; /* 0x005d1948 */
extern const char *g_pszCommScreenHeading; /* 0x005d1950 */
extern char g_4_szCommMenuHeadingStore[0x40]; /* 0x005d1960 */
extern const char *g_1_apszCommMenuChoiceTextTable[7]; /* 0x005d19a0 */
extern void *g_9_apPacketHandlesTable[0x1000]; /* 0x005c9020 */
extern int g_aiPacketHandleOffsets[0x1000]; /* 0x005cd020 */
extern int g_nPacketHandleCount; /* 0x005d1020 */
extern Viewport *g_pDibScreenViewport; /* 0x005d398c */
extern Viewport *g_19_pFullScreenViewport; /* 0x005d3990 */
extern unsigned short g_2_awDibScreenRowOffsetsData[202]; /* 0x005d39a0 */
extern int g_nDibScreenChunkState; /* 0x005d3b40 */
extern Viewport *g_pDibScreenViewportMirrorRef; /* 0x005d3b44 */
extern int *g_pDibScreenAllocationStatePointer; /* 0x005d3b4c */
extern int g_aiSoundEffectSourceActiveValue[SPACE_OBJECT_COUNT]; /* 0x005d12c0 */
extern MusicResource g_aTuneResources[66]; /* 0x005d13e0 */
extern void *g_4_pDigiData; /* 0x005d1710 */
extern unsigned short g_wDigiLeft; /* 0x005d1714 */
extern void *g_4_pDmaBufferPointer; /* 0x005d1718 */
#ifdef SPEECH_C
extern unsigned short g_wDmaCountB; /* 0x0049bb70 */
#else
extern unsigned short g_2_wDigiPosition; /* 0x005d171c */
#endif
extern void *g_pDigiSample; /* 0x005d1720 */
extern unsigned char g_1_ucDmaPageValue; /* 0x005d1724 */
extern int g_nPassingShipSndCooldown; /* 0x005d156c */
extern short g_10_nAlternateSampleToggle; /* 0x005d1570 */
extern int g_14_anObjectSoundEffects[SPACE_OBJECT_COUNT]; /* 0x005d1580 */
extern int g_nAllocateViewportCalls; /* 0x005c8dc4 */
extern void *g_26_pLibraryCurrentLastPacketAllocation; /* 0x005c80e0 */
extern FireworkState g_16_aFireworks[30]; /* 0x005c8df0 */
extern unsigned char *g_1_pTitleFireworkShape; /* 0x005c8f58 */
extern Viewport *g_pConstellationWindow; /* 0x005c8f5c */
extern ConstellationParticle g_aConstellationParticlesTable[18]; /* 0x005c8f60 */
extern Viewport g_stConstellationViewport; /* 0x005a6b40 */
extern short g_14_nConstellationParticleTotal; /* 0x005c9014 */
extern short g_2_nExternalViewDistanceValue; /* 0x005d320e */
extern Viewport g_101_stScrViewInfo; /* 0x005d21a0 */
extern TextContext g_168_stFlightTextContext; /* 0x005d21c0 */
extern unsigned char *g_66_pCabinHudImage; /* 0x005d21f4 */
extern unsigned char *g_36_apTextGlyphs[4]; /* 0x005d2200 */
extern FixedVector g_9_aDirectionLookRightVector[
    DIRECTION_VIEW_COUNT]; /* 0x005d2210 */
extern FixedVector g_137_aDirectionViewUpVectorArray[
    DIRECTION_VIEW_COUNT]; /* 0x005d2500 */
extern FixedVector g_2_aDirectionViewForwardAxisTable[
    DIRECTION_VIEW_COUNT]; /* 0x005d27f0 */
extern Viewport g_3_stDashViewport; /* 0x005d2160 */
extern Viewport g_9_stCockpitMeterPort; /* 0x005d21e0 */
extern Viewport g_27_stViewScratchInfo; /* 0x005d2b00 */
extern Viewport g_4_stRightVduView; /* 0x005d2b20 */
extern Viewport g_stPilotHandBackgroundViewportBlock; /* 0x005d2b40 */
extern Viewport g_stPilotHandViewportRecord; /* 0x005d2c70 */
extern unsigned char *g_1_pDashWeaponPicture; /* 0x005d2b54 */
extern ShortPoint g_116_aWeaponDisplayPositions[16]; /* 0x005d1de0 */
extern Viewport g_2_stLeftwardVduPortBlock; /* 0x005d2180 */
extern TextContext g_5_stLeftVduStringContextRecord; /* 0x005d2ae0 */
extern TextContext g_1_stRightVduTextContextBlock; /* 0x005d2ce0 */
extern TextContext g_16_stCockpitTextSettingsRecord; /* 0x005d2d00 */
extern unsigned char *g_4_pCockpitPanelShape; /* 0x005d2cd4 */
extern unsigned char *g_pCockpitBackgroundPacket; /* 0x0049a5f0 */
extern unsigned char *g_apCockpitVduOverlayShapes[2]; /* 0x0049a5f8 */
extern unsigned char *g_2_apCockpitViewShapes[4]; /* 0x005d1040 */
extern unsigned char *g_2_apCockpitShipShapes[4]; /* 0x005d1070 */
extern char g_7_szCockpitAssetFilenameText[16]; /* 0x005d1030 */
extern Viewport g_stCockpitBackgroundViewportRecord; /* 0x005d1050 */
extern unsigned char *g_pCockpitViewFrameInfo; /* 0x005d1088 */
extern unsigned char *g_pPendingCockpitDamageShape; /* 0x0049b03c */
extern unsigned char *g_pCockpitExplosionBackground; /* 0x0049b040 */
extern unsigned char *g_pCockpitHudBackground; /* 0x0049b044 */
extern unsigned char *g_pDamageDisplayBackground; /* 0x00490060 */
extern unsigned char *g_4_pScannerBlipUnderlayPointer; /* 0x005d1c2c */
extern unsigned char *g_pReleaseWeaponDisplayBackground; /* 0x0049b05c */
extern signed char g_cReleaseWeaponDisplayFrame; /* 0x0049b060 */
extern signed char g_cReleaseWeaponDisplayTicks; /* 0x0049b064 */
extern signed char g_cReleaseWeaponDisplayState; /* 0x0049b068 */
/* Where the released-weapon sprite drops from; one record, so its y reads as a
 * member (the typedef keeps the header's symbol count of the two words). */
typedef ShortPoint ReleaseWeaponPosition;
extern ReleaseWeaponPosition g_3_stReleaseWeaponReadoutLocationBlock; /* 0x005d1c24 */
extern short g_3_nReleaseWeaponShowKindWord; /* 0x005d1c28 */
extern short g_5_nWeaponDisplayOriginXValue; /* 0x005d4254 */
extern short g_nArmsDisplayOriginY; /* 0x005d4256 */
extern short g_68_nDamageShowTicksNumber; /* 0x005d4250 */
extern char g_12_szDamageStatusText[70]; /* 0x005d4260 */
extern signed char g_cDamageDisplayPart; /* 0x005d42a6 */
extern signed char g_cDamagedPartCount; /* 0x005d42a7 */
extern int g_nDisplayedComponentDamage; /* 0x005d42a8 */
extern signed char g_cDamageDisplayFrame; /* 0x005d42ac */
extern short g_15_nDamageDisplayState; /* 0x005d42ae */
extern ShortPoint g_stHarmSpritePosition; /* 0x005d42b0 */
extern short g_104_nHarmDisplayPhaseValue; /* 0x005d42b4 */
extern Viewport g_6_stConversationTextViewport; /* 0x005d2b60 */
extern char g_6_szDefaultTextBufferText[0xc8]; /* 0x005d2b80 */
extern Viewport g_2_stModalSourceWindow; /* 0x005d2c50 */
extern unsigned char *g_18_pCockpitIndicatorShape; /* 0x005d2c48 */
#ifdef GAMEFLOW_C
extern Viewport g_9_stCutsceneTextBackingPort; /* 0x005d2db0 */
#else
extern Viewport g_stSecondaryViewBufferBlock; /* 0x005d2c90 */
#endif
extern SceneFlicCacheEntry g_aSceneFlicCopyTable[15]; /* 0x005d2e10 */
extern Viewport g_15_stSceneFlicScratchViewport; /* 0x005d2eb0 */
extern short g_9_aasCockpitHitPaletteFades[6][3]; /* 0x005d2cb0 */
extern TextContext g_3_stDefaultStringContext; /* 0x005d2d20 */
extern TextContext g_2_stConversationTextCtx; /* 0x005d2d40 */
extern short g_1_asSpacePaletteFade[3]; /* 0x005d2d60 */
extern unsigned int g_uFrameTimerEventId; /* 0x005d3bec */
extern InputDeviceSample g_10_stPrevFlightControls; /* 0x005c57d0 */
extern char *g_pInitialArguments[31]; /* 0x005c57f0 */
extern Viewport g_stTrainSimTitleDisplayViewport; /* 0x005a7b90 */
extern Viewport g_stTrainSimHighScoreBufferViewport; /* 0x005a7bb0 */
extern TextContext g_stTrainSimTextContext; /* 0x005a7bd0 */
extern Viewport g_stTrainSimHighScoreDisplayViewport; /* 0x005a7bf0 */
extern TextContext g_stTrainSimHighScoreTextContext; /* 0x005a7c10 */
extern HighScoreEntry g_aHighScoreEntries[6]; /* 0x005a7c30 */
extern unsigned char *g_pSceneAnimationPrimaryShape; /* 0x005a7c58 */
extern SceneAnimationObject *g_pSceneAnimationObjects; /* 0x005a7c64 */
extern short g_nSceneAnimationWaitFrames; /* 0x005a7c68 */
extern unsigned char *g_pSceneAnimationDefinitions; /* 0x005a7c6c */
extern unsigned char *g_pSceneAnimationSecondaryShape; /* 0x005a7c70 */
extern signed char g_cCockpitLogicalFile; /* 0x005a7c74 */
extern signed char g_36_cPilotKillCountValue; /* 0x005d2fa8 */
extern short g_8_bUserDestroyedFlag; /* 0x005d2fa4 */
extern short g_5_nCollisionCountdownNumber; /* 0x005d2faa */
extern signed char g_4_acInitialShipKillNumber[8]; /* 0x005d2fc0 */
extern short g_bWingmanKilledThisMission; /* 0x005d2fb8 */
extern short g_nWingmanKillCount; /* 0x005a7cb8 */
extern short g_nLastFoundShip; /* 0x005d2fc8 */
extern unsigned int g_dwOriginalFreeMemory; /* 0x005a7cd8 */
extern unsigned char *g_pMouseCursorResource; /* 0x005a7cdc */
extern int g_3_bAfterburnerSfxActive; /* 0x005d3864 */
extern signed char g_6_bCampaignFlightRunning; /* 0x005c8430 */
extern int g_2_bIgnoreNextCursorMoveFlag; /* 0x005c843c */
extern FixedVector g_2_vNormalizedToTgt; /* 0x005d3bd0 */
extern signed char g_14_cPilotHandFrameCode; /* 0x005d1c30 */
extern short g_9_nScreenMessageTimeNumber; /* 0x005d1c32 */
extern short g_36_nPendingDashDamageValue; /* 0x005d1c34 */
extern int g_bSpeechPlaybackDone; /* 0x004a266c */
extern HudMessageSlot g_12_aHudTextSlotsArray[2]; /* 0x005d1d40 */
extern signed char g_2_cEarlierTargetObjectValue; /* 0x005d1d8e */
extern short g_19_nCommPortraitAnimationTickWord; /* 0x005d1d94 */
extern char g_16_szComponentHitMsg[40]; /* 0x005d1da0 */
extern short g_36_nTargetLockonMarkerY; /* 0x005d1e20 */
extern short g_6_nHomeOperationShipIndexValue; /* 0x005d1e22 */
extern CockpitReadout g_9_aConsoleMetersTable[6]; /* 0x005d1e30 */
extern short g_8_nScopePointerX; /* 0x005d1e6c */
extern short g_9_nScannerCursorYValue; /* 0x005d1e6e */
extern short g_10_asScannerObjectYTable[11]; /* 0x005d1e80 */
extern volatile ShortPoint g_8_stCockpitExplosionPosBlock; /* 0x005d1e98 */
extern short g_15_asScannerObjectX[11]; /* 0x005d1ea0 */
extern short g_47_asScannerBackgroundColourList[10]; /* 0x005d1ed0 */
extern short g_14_asScannerTargetUnderlay[5]; /* 0x005d1d80 */
extern int g_4_nSevereDmgWarningSfxHandle; /* 0x005d1ec0 */
extern unsigned short g_18_nHudMessageColour; /* 0x005d1ef0 */
extern short g_7_nExternalViewTimeout; /* 0x005d3212 */
extern int g_nViewportAllocationCount; /* 0x005d19bc */
extern unsigned char *g_apViewportAllocations[128]; /* 0x005d19c0 */
extern int g_2_nFreeViewportCalls; /* 0x005d1bc0 */
extern short g_nMissionMedalScore; /* 0x005a8116 */
extern short g_nCampaignDataSet; /* 0x005a8118 */
/* Both heap units load arena size before base in their end-address sums. */
#if defined(NEARHEAP_C) || defined(MEMFREE_C)
extern volatile int g_1_nNearHeapBase; /* 0x005d3054 */
extern volatile int g_nNearArenaSize; /* 0x005d3050 */
#else
extern volatile int g_nNearArenaSize; /* 0x005d3050 */
extern volatile int g_1_nNearHeapBase; /* 0x005d3054 */
#endif
extern int g_nNearHeapFirstDescriptor; /* 0x005d3058 */
extern void * volatile g_1_pNearHeapAllocationPtr; /* 0x005d305c */
extern short g_nNavPlotMaxX; /* 0x005d169e */
extern short g_nNavmapMapMinimumX; /* 0x005d16a0 */
extern unsigned short g_awNavObjectiveLabelIndex[8]; /* 0x005d16b0 */
extern short g_nNavmapMapMaximumY; /* 0x005d16c0 */
extern short g_13_nNavmapMapCentreX; /* 0x005d16c2 */
extern short g_2_nNavMapMinimumYValue; /* 0x005d16c4 */
extern TextContext g_stNavMapTextCtx; /* 0x005d16d0 */
extern short g_nNavMapCentreY; /* 0x005d16ec */
extern TextContext g_5_stNavLabelTextContextInfo; /* 0x005d16f0 */
extern CampaignDate g_1_stElapsedCampaignDate; /* 0x005d170c */
extern DiskFileRecord *g_pDiskFileRecords; /* 0x005a7cf0 */
extern Viewport g_stDiskPromptBackgroundViewport; /* 0x005a7d00 */
extern unsigned char g_abDiskPromptDriveState[2]; /* 0x005a7d20 */
extern Viewport g_stDiskPromptViewport; /* 0x005a7d40 */
extern TextContext g_stDiskPromptTextContext; /* 0x005a7d60 */
extern unsigned int g_dwDiskPromptTopLeft; /* 0x005a7d80 */
extern unsigned int g_dwDiskPromptBottomRight; /* 0x005a7d84 */
extern short *g_pPaletteTransitionDirectionRef; /* 0x005d3010 */
extern short *g_pPaletteTransitionDeltaPtr; /* 0x005d3014 */
extern short g_2_nPaletteTransitionMaxDelta; /* 0x005d3018 */
extern short *g_pPaletteTransitionAccumulator; /* 0x005d301c */
extern short g_nPaletteTransitionCountdownNumber; /* 0x005d3020 */
extern CampaignDate *g_pCurrentCampaignDate; /* 0x005a86a8 */
extern CampaignDate *g_4_pElapsedCampaignDatePointer; /* 0x005d3e8c */
extern unsigned char *g_pConversationSpecialShape; /* 0x005a86ec */
extern short g_nFuneralCasketX; /* 0x005a86c8 */
extern short g_nFuneralCasketY; /* 0x005a86ca */
extern ShortPoint g_aFuneralParticles[7]; /* 0x005a86f0 */
extern short g_nFuneralParticleDistance; /* 0x005a8710 */
extern short g_nFuneralForegroundX; /* 0x005a8718 */
extern short g_nFuneralForegroundY; /* 0x005a871a */
extern short g_nFuneralMainScale; /* 0x005a871c */
extern short g_nFuneralRifleFrame; /* 0x005a871e */
extern short g_nFuneralBaseX; /* 0x005a8720 */
extern short g_nFuneralBaseY; /* 0x005a8722 */
extern short g_nFuneralParticleScale; /* 0x005a8728 */
extern short g_nFuneralMainDistance; /* 0x005a8738 */
extern short g_nFuneralGuardFrame; /* 0x005a873c */
extern short g_nTalkingHeadFaceX; /* 0x005a8754 */
extern short g_nTalkingHeadFaceY; /* 0x005a8756 */
extern short g_nTalkingHeadMouthY; /* 0x005a8758 */
extern short g_nTalkingHeadMouthX; /* 0x005a875a */
extern unsigned char g_5_abMissionAuxBlockBytes[0x28]; /* 0x005d3c10 */
extern unsigned char g_abSeriesAuxData[0x28]; /* 0x005d3c40 */
extern MissionObjectiveSource g_69_aMissionObjectiveSources[8]; /* 0x005d3c70 */
extern MissionHeader g_13_stMissionHeaderBlock; /* 0x005d3e70 */
extern unsigned char g_abMissionBriefingData[0xde]; /* 0x005d3e90 */
extern unsigned char *g_pConstellationSprite; /* 0x005d2c4c */
extern unsigned char *g_pIntroFont; /* 0x005a8960 */
extern unsigned int g_69_dwSysKeyValue; /* 0x005d10a4 */
extern JOYINFO g_1_aJoystickInfoArray[2]; /* 0x005d10b0 */
extern GuardedAllocation *g_9_pGuardedAllocationLastRef; /* 0x005d10ec */
extern char g_szGuardedHeapDiagnosticMessage[0x80]; /* 0x005d10f0 */
extern char g_29_abMemoryLoadNotice[0x80]; /* 0x005d1170 */
extern HWND g_10_hMainWindow; /* 0x005d10e0 */
extern HANDLE g_19_hOneProcessLock; /* 0x005d10e4 */
extern int g_39_bAltNumpadAddHotkeySwitch; /* 0x005d1270 */
extern int g_27_nLastAltCommandScanNumberWord; /* 0x005d1274 */
extern int g_2_bAltFHotkeyFlag; /* 0x005d127c */
extern int g_6_bAltDChordState; /* 0x005d1280 */
extern int g_bJoystickCalibrationHotkey; /* 0x005d1284 */
extern HCURSOR g_1_hMainWndCursor; /* 0x005d1288 */
extern int g_2_bAltHHotkey; /* 0x005d128c */
extern int g_4_bMenuBHotkey; /* 0x005d1290 */
extern int g_bMenuAHotkey; /* 0x005d1294 */
extern int g_6_bMenuTHotkeySwitch; /* 0x005d1298 */
extern int g_3_nSavedOwnerCursorX; /* 0x005d12a0 */
extern int g_102_nStoredWindowCursorYWord; /* 0x005d12a4 */
extern int g_bAltNumpadSubtractShortcut; /* 0x005d12a8 */
extern int g_16_bMainWindowRunning; /* 0x005d12ac */
extern HDC g_18_hMainWindowContext; /* 0x005d1278 */
extern HINSTANCE g_7_hAppCopy; /* 0x005d12b0 */
extern unsigned short g_ausPaletteWords[256][3]; /* 0x005d3220 */
extern int g_4_bScreenStale; /* 0x005c395c */
extern int g_nDibDirtyHitTotal; /* 0x005c33a8 */
extern unsigned int g_dwLastDibDirtyReportTime; /* 0x005c39f0 */
extern char g_2_szDibDebugScratch[260]; /* 0x005c3858 */ /* MAX_PATH */
extern int g_8_nDibFlipRealCount; /* 0x005c3960 */
extern unsigned int g_10_dwLastSurfaceSlamRealNoticeTick; /* 0x005c396c */
extern char g_3_szDibSlamRealDebugBuf[256]; /* 0x005c39f8 */
extern const char *g_pszMissileLocked; /* 0x0049b280 */
extern unsigned long g_128_dwGameClockBase; /* 0x005d10e8 */
extern long g_13_lGameClockStartData; /* 0x005d12b8 */
extern unsigned int g_11_dwGameZeroTime; /* 0x005d12b4 */
extern unsigned int g_48_dwGameExitClockValue; /* 0x005d129c */

extern signed char g_4_abCockpitLightMode[7]; /* 0x005d1e70 */
extern signed char g_12_abCockpitLightGoalTable[7]; /* 0x005d1eb8 */
extern short g_69_asCockpitDmgStateList[4]; /* 0x005d1ee8 */
extern ShortPoint g_aCockpitDamagePositions[4]; /* 0x0049ae98 */
extern short g_asCockpitLightX[7]; /* 0x0049aea8 */
extern short g_asCockpitLightY[7]; /* 0x0049aeb8 */
extern signed char g_acCockpitLightOffFrame[7]; /* 0x0049aec8 */
extern signed char g_acCockpitLightOnFrame[7]; /* 0x0049aed0 */
extern CockpitBarDefinition g_aCockpitBarDefinitions[8]; /* 0x0049aed8 */
extern ShortPoint g_aCockpitReadoutOrigins[4]; /* 0x0049af58 */
extern ShortRect g_stLeftVduBounds; /* 0x0049af68 */
extern ShortRect g_stRightVduBounds; /* 0x0049af70 */
extern CockpitScannerGeometry g_stCockpitScannerGeometry; /* 0x0049af78 */
extern ShortRect g_stPilotHandBounds; /* 0x0049af88 */
extern ShortPoint g_stPilotHandOrigin; /* 0x0049af90 */
extern signed char g_cHazardBaseTravelTime; /* 0x0049af94 */
extern short g_nHazardPitchRange; /* 0x0049af98 */
/* fltsetup.c stores and tests a word; cockpt.c reads the low byte. */
extern short g_bTargetLockDisplayEnabled; /* 0x0049afe0 */
extern short g_nTargetLeadIndicatorX; /* 0x0049afe8 */
extern short g_1_nTargetLeadingIndicatorY; /* 0x005d1c2a */
extern char g_szTargetOutOfRange[13]; /* 0x0049b438 */
extern char g_szIttsEngaged[13]; /* 0x0049b448 */
extern unsigned char g_abGunDisplayConfiguration[0xa4]; /* 0x0049d340 */
extern short g_1_nGunShowEnergyPercent; /* 0x005c8d4e */
extern short g_12_nGunDisplayCount; /* 0x005c8dbc */
extern short g_8_nGunDisplaySlot; /* 0x005c8dc0 */
extern int g_9_nTargetCameraOverlayMode; /* 0x005c8db8 */
extern short g_nTargetCameraFrame; /* 0x0049d3e8 */
extern int g_anLastDrawnVduMode[3]; /* 0x004934d0 */
extern unsigned char * volatile g_pCockpitExplosionShape; /* 0x0049b048 */
extern volatile short g_nCockpitExplosionFrame; /* 0x0049b04c */
extern short g_nTargetLockMusicCooldown; /* 0x005d1e78 */
extern unsigned int g_dwStreamerMode; /* 0x005c4c38 */
extern short g_nUnusedInputWord4; /* 0x0059a856 */
/* nav.c keeps this storage private: it defines NAV_C before including the game
 * headers, so these become its statics at their header positions. */
#ifdef NAV_C
#define NAV_STATIC static
#else
#define NAV_STATIC extern
#endif
NAV_STATIC NavMapLabel g_1_aNavMapLabels[20]; /* 0x005b34e0 */
NAV_STATIC ShortRect g_aNavMapExclusionRects[20]; /* 0x005b35a8 */
extern int g_nSfxVolumeSetting; /* 0x0049d74c */
extern int g_nMusicVolumeSetting; /* 0x0049d750 */
extern short g_bViewportDirty; /* 0x0049d76c */
extern int g_anVolumeLevels[11]; /* 0x0049d720 */
extern int g_3_nEventClockScaleValue; /* 0x005c8d24 */
extern short g_asControlsButton1DoubleClickTable[2]; /* 0x005d3030 */
extern short g_3_asControlsButton1ChangedValues[2]; /* 0x005d3034 */
extern short g_9_asInputButton2ChangedList[2]; /* 0x005d3038 */
extern short g_2_asPreviousInputButtonsList[2]; /* 0x005d303c */
extern int g_anControlsButton1PressTime[2]; /* 0x005d3040 */
extern int g_9_anInputButton2PushTime[2]; /* 0x005d3028 */
extern short g_7_asInputButton2DoubleClickValues[2]; /* 0x005d3048 */
extern signed char g_3_bInputPollingGuard; /* 0x005d304c */

extern signed char g_bSceneEscapeRequested; /* 0x0049d4b0 */
extern int g_nNextInputQueueFlushTick; /* 0x0049d4cc */
extern int g_nNextInputPollTick; /* 0x0049d6d4 */
extern short g_nInputPollPeriod; /* 0x0049d6d8 */
extern Viewport *g_pInputViewport; /* 0x005c8403 */
extern void (*g_176_pfnCurrentInputPump)(void); /* 0x005c840c */
extern short g_105_nEmsIsrBusyValue; /* 0x005c841c */
extern InputManagerState *g_18_pEventManager; /* 0x005c8464 */
extern InputPumpContext *g_pActiveInputContext; /* 0x005c8487 */
extern void (*g_59_pfnEventShutdownCallback)(void); /* 0x005c8be0 */
extern short g_86_nCurrentInputTickInterval; /* 0x005c8448 */
extern signed char g_cKeypadMouseSpeedValue; /* 0x005c83e6 */
#ifdef GAMEFLOW_C
extern signed char g_bCutsceneTextBoundsOverride; /* 0x00499edc */
#else
extern volatile int g_196_nInputTimerTicks; /* 0x005c84a8 */
#endif
extern void *g_pApplicationScratchBuffer; /* 0x005c8483 */
extern short g_nUiCursorFrame; /* 0x005c8481 */
extern short g_12_wInputDeviceButtonStatus; /* 0x005c8432 */
extern short g_nSelectedStartingCampaign; /* 0x005d3bf0 */
extern short g_nSelectedCampaignSlot; /* 0x005d3bf2 */
extern DwordPtr g_dwHighMemoryParagraphValue; /* 0x005d3fb4 */
extern short g_78_bHighMemoryBuffersSetState; /* 0x005d2ad8 */
extern short g_128_nShotHotspotLeft; /* 0x005d2120 */
extern short g_27_nSceneHotspotTopValue; /* 0x005d2122 */
extern short g_8_nSceneRegionRightWord; /* 0x005d2124 */
extern short g_15_nSceneAreaLowestWord; /* 0x005d2126 */
extern void *g_13_pStartupStellarPacket; /* 0x005d212c */
extern short g_nScreenWidth; /* 0x0049d4d8 */
extern short g_nScreenHeight; /* 0x0049d4dc */
extern short g_3_nFlightControlCommandValue; /* 0x005d1798 */
extern signed char g_cCurrentKey; /* 0x00493128 */
extern signed char g_cPreviousKey; /* 0x0049312c */
extern unsigned int g_dwCannedSceneSnapshotStart; /* 0x00493130 */
extern void *g_pSceneMusicPacket; /* 0x00499c08 */
extern unsigned int g_4_dwSceneMusicMode; /* 0x005d2dc4 */
/* Dead: 0x5D14E8 is entry 44 of g_aTuneResources; the name keeps
 * the header's symbol count. */
extern unsigned int g_dwRestoredSceneMusicState; /* 0x005d14e8 */
extern short g_3_nCannedSceneWriteIndex; /* 0x005d3fa8 */
extern int g_4_nCannedSceneFileOffset; /* 0x005d3fac */
extern void *g_1_pCannedSceneStateBlockRef; /* 0x005d3fb0 */
#ifdef MAIN_C
extern short g_5_nCannedSceneFrameCount; /* 0x005d404c */
extern short g_nCannedSceneRecordedFrameCountNumber; /* 0x005d3faa */
#else
extern short g_nCannedSceneRecordedFrameCountNumber; /* 0x005d3faa */
extern short g_5_nCannedSceneFrameCount; /* 0x005d404c */
#endif
extern short g_awPaletteEntryAllocation[256]; /* 0x005d4050 */
extern short g_1_asCannedSceneFrameStarts[70]; /* 0x005d3fc0 */

extern PilotProfile g_stDefaultPilotProfile; /* 0x00492660 */
extern short g_bNewPilotCampaignInitialized; /* 0x004926c0 */
extern short g_bDeveloperCampaignReady; /* 0x004926c4 */
extern PilotProfile g_stCurrentPilotProfile; /* 0x00493408 */
/* The text variables that briefing, comm and cutscene text substitute
 * for the player's callsign, last name and first name.  Declared here,
 * beside its only instance, so the type adds no symbols ahead of the
 * other globals. */
typedef struct PilotNameVariables {
    char callsign[24];                /* +0x00 */
    char lastName[24];                /* +0x18 */
    char firstName[24];               /* +0x30 */
} PilotNameVariables;

extern PilotNameVariables g_stPilotNameVariables; /* 0x00499ef8 */
extern short g_1_nResourcePaletteState; /* 0x005c57e6 */
extern short g_3_nOptionMessageColour; /* 0x005c57e8 */
extern short g_nMenuSelectedCol; /* 0x005c5874 */
extern short g_10_nMenuDarkColour; /* 0x005c5876 */
extern int g_nLegacySaveSource; /* 0x0049a600 */
extern short g_nPersonnelMenuHighlight; /* 0x0049a6a0 */
extern void *g_pPersonnelCampaignSnapshot; /* 0x0049a6a4 */
extern unsigned char *g_pPersonnelMenuBackdrop; /* 0x0049a6b8 */
extern short g_sPersonnelPreviousInput; /* 0x0049a6bc */
extern short g_bPersonnelMenuDrawing; /* 0x0049a6c0 */
extern signed char g_bPersonnelPrimaryInputDown; /* 0x0049a6d4 */
extern const char *g_apszSaveGameFileNames[3]; /* 0x0049a6c8 */
extern const char *g_apszShortPilotRankNames[5]; /* 0x0049a620 */
extern void *g_pActiveScenePacket; /* 0x00492654 */
extern short g_nPersonnelCursorX; /* 0x005c8470 */
extern short g_nPersonnelCursorY; /* 0x005c8472 */
MAIN_STATIC int g_bMouseFireButtonLatched; /* 0x005c4b10 */
MAIN_STATIC short g_wSavedInputDeviceButtonState; /* 0x005c4b14 */
extern short g_1_bDiscreteFlightInputWaiting; /* 0x005c57e4 */
extern short g_19_bPolledFlightInputQueued; /* 0x005c587a */
extern int g_10_bPlayerInputActive; /* 0x005c8090 */
extern short g_6_wQueuedButtonsState; /* 0x005c80d4 */
extern PersonnelMouseSample g_6_stPersonnelMouseData; /* 0x005c8d00 */
/* Obsolete field declarations retain C1's numbering; they have no definitions. */
extern short g_nPersonnelMouseY; /* 0x005c8d02 */
extern unsigned char g_bPersonnelPrimaryButton; /* 0x005c8d04 */
extern unsigned char g_bPersonnelSecondaryButton; /* 0x005c8d05 */
extern unsigned short g_wPersonnelInputFlags; /* 0x005c8d07 */
extern int g_bSceneDisplayUpdateActive; /* 0x00499bb8 */
extern short g_1_nCockpitArchiveVariantNumber; /* 0x005c901a */
extern char *g_3_apszPersonnelImportCaptionsTable[8]; /* 0x005d2100 */
extern char g_1_szLegacySaveFolder[0x30]; /* 0x005d2130 */
extern SceneResourceTable g_stSceneZoneTable; /* 0x005d3bf8 */
extern SceneResourceTable g_stSceneTextList; /* 0x005d3c00 */
extern char *g_pszPersonnelFooter; /* 0x00492658 */
extern unsigned char *g_pCampaignChalkboardShape; /* 0x0049ca54 */
extern short g_bDisableChalkboardReplay; /* 0x0049ca58 */
extern short g_8_bSceneBackdropClicked; /* 0x005c9018 */
extern short g_asConstellationObjectIndices[4]; /* 0x0049c8e0 */
extern char g_15_szLegacyGameDataPath[0x400]; /* 0x005b2898 */

extern short g_8_nActiveInputDeviceWord; /* 0x005d1726 */
extern int g_nControllerVerticalDeadZoneWord; /* 0x005d1728 */
extern int g_nJoystickHorizDeadZone; /* 0x005d172c */
extern int g_59_nJoystickUpScaleWord; /* 0x005d173c */
extern int g_12_nControllerLeftScaleValue; /* 0x005d1740 */
extern int g_4_nJoystickMaximumXValue; /* 0x005d1744 */
extern int g_nJoystickMaximumY; /* 0x005d1748 */
extern int g_12_nJoystickMinimumXNumber; /* 0x005d174c */
extern int g_2_nJsMinimumY; /* 0x005d1750 */
extern int g_3_nJoystickRawX; /* 0x005d2ff8 */
extern int g_nStickRawY; /* 0x005d2ffc */
/* Calibration's four scale products depend on these declaration keys. */
#ifdef TITLEMNU_C
extern int g_3_nControllerHorizontalRange; /* 0x005d1758 */
extern int g_nJoystickVerticalRangeNumber; /* 0x005d1754 */
#else
extern int g_nJoystickVerticalRangeNumber; /* 0x005d1754 */
extern int g_3_nControllerHorizontalRange; /* 0x005d1758 */
#endif
extern int g_1_nJsRightScaleWord; /* 0x005d175c */
extern int g_12_nJoystickDownScaleWord; /* 0x005d1760 */
extern int g_2_nJoystickCentreY; /* 0x005d1764 */
extern int g_1_nJoystickCentreXNumber; /* 0x005d1768 */
extern int g_4_nControllerFailureValue; /* 0x005d176c */
extern InputDeviceSample g_aInputDeviceSamplesList[2]; /* 0x005d1780 */
extern short g_nMenuInputRepeatDelay; /* 0x005a8208 */
extern unsigned char *g_1_pCampaignDataBase; /* 0x005d300c */
extern HostMouseMessage g_6_stHostMouseTextBlock; /* 0x005d10d0 */

extern ObjectTypeData g_aObjectTypeData[OBJECT_DATA_COUNT]; /* 0x00496d30 */
extern LPDIRECTDRAWSURFACE g_pPrimarySurface; /* 0x0049ce94 */
extern LPDIRECTDRAWSURFACE g_pSecondarySurface; /* 0x0049ce98 */
extern LPDIRECTDRAWPALETTE g_pDirectDrawPalette; /* 0x0049ce9c */
extern int   g_nDisplayModeCascade; /* 0x0049cea0 */
extern char g_szFrameErrorMessage[0x8c]; /* 0x005c33b0 */
extern char g_59_szFrameFrequencyString[32]; /* 0x005b3950 */
extern float g_fMeasuredFrameRate; /* 0x005c3850 */

extern signed char g_cMessageSpeed; /* 0x0049b778 */
extern unsigned char g_abManeuverRerollChance[50]; /* 0x0049b538 */
extern void (*g_apManeuverHandlers[50])(short, short); /* 0x0049b570 */

extern ShortVector g_aShipFormationOffset[10]; /* 0x00495468 */

/* Ordered name tables read directly out of the image; see include/game_data.h.
 * Both are packed variable-length strings, not fixed-stride arrays, so the
 * game indexes them through a pointer table rather than by multiplication. */

#endif /* GAME_GLOBALS_H */
