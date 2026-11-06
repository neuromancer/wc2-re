/*
 *  Sets a viewport rectangle.
 *
 *  Retail object 0x423C60-0x423C9F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x423C60 */
void SetViewportRect(Viewport *viewport, unsigned short left,
                     unsigned short top, unsigned short right,
                     unsigned short bottom)
{
    viewport->left = (short)left;
    viewport->top = (short)top;
    viewport->right = (short)right;
    viewport->bottom = (short)bottom;
}

/* The shared game state, bracketed by the canned-scene snapshot markers that
 * the recorder saves and restores as one block.  Its position between
 * joycal.c's and sound.c's data places it in this unit, whose only function
 * is SetViewportRect; the original file was most likely a data module. */
short g_asDifficultyLevels[16][4] = {
    50, 200, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127
}; /* 0x004930a8 */
signed char g_cCurrentKey = (signed char)0x80; /* 0x00493128 */
signed char g_cPreviousKey = (signed char)0x80; /* 0x0049312c */
unsigned int g_dwCannedSceneSnapshotStart = 0; /* 0x00493130 */
short g_nSpaceFrame = 0; /* 0x00493134 */
short g_nRenderedSpaceFrame = 0; /* 0x00493138 */
signed char g_cViewObject = -1; /* 0x0049313c */
short g_nEyePitchGoal = 0; /* 0x00493140 */
short g_nEyeYawGoal = 0; /* 0x00493144 */
short g_nEyeRollGoal = 0; /* 0x00493148 */
short g_nEyePitchRate = 1; /* 0x0049314c */
short g_nEyeYawRate = 1; /* 0x00493150 */
short g_nEyeRollRate = 1; /* 0x00493154 */
FixedVector g_vStarFieldMotion = { 0 }; /* 0x00493158 */
FixedVector g_vPreviousStarFieldMotion = { 0 }; /* 0x00493168 */
FixedVector g_vCollisionDelta = { 0 }; /* 0x00493178 */
FixedVector g_vToTarget = { 0 }; /* 0x00493188 */
short g_nFacingToTarget = 0; /* 0x00493194 */
short g_nTargetFacing = 0; /* 0x00493198 */
short g_nTargetRange = 0; /* 0x0049319c */
short g_nTargetShip = 0; /* 0x004931a0 */
short g_bExpandedShipGraphicsEnabled = 0; /* 0x004931a4 */
short g_anFlightInput[3] = { 0 }; /* 0x004931a8 */
short g_anPreviousFlightInput[3] = { 0 }; /* 0x004931b0 */
short g_nNavPointerObject = -1; /* 0x004931b8 */
short g_nCurrentNavPoint = 0; /* 0x004931bc */
short g_nCurrentWave = -1; /* 0x004931c0 */
short g_nLoadedCommPortraitPilot = -1; /* 0x004931c4 */
int g_nCurrentObjectiveRange = 0; /* 0x004931c8 */
signed char g_cCurrentObjective = -1; /* 0x004931cc */
short g_nHazardFieldCount = 0; /* 0x004931d0 */
HazardField g_aHazardFields[8] = { { -1 } }; /* 0x004931d8 */
HazardField *g_pActiveHazardField = 0; /* 0x00493278 */
short g_bUseEyePositionForHazards = 0; /* 0x0049327c */
signed char g_abHazardObjects[0x14] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
}; /* 0x00493280 */
signed char g_cMissionObjectiveCount = -1; /* 0x00493294 */
signed char g_cCurrentNavPointIndex = -1; /* 0x00493298 */
#ifdef SDL_PORT
signed char g_abFlightPath[MISSION_OBJECTIVE_COUNT + 1] = { -1 }; /* 0x004932a0 */
#else
signed char g_abFlightPath[MISSION_OBJECTIVE_COUNT] = { -1 }; /* 0x004932a0 */
#endif
#ifdef SDL_PORT
/* The type read at index 8 precedes cleanup_objectives' bounds check; the
 * retail read at 0x49339D falls inside the adjacent object-resource table. */
MissionObjective g_aMissionObjectives[MISSION_OBJECTIVE_COUNT + 1] = {
#else
MissionObjective g_aMissionObjectives[MISSION_OBJECTIVE_COUNT] = {
#endif
    { 0, 0, 0xff, 7, -1, 0xff, -1 }
}; /* 0x004932a8 */
ObjectResourceSlot g_aObjectResourceSlots[5] = { { -1 } }; /* 0x00493398 */
PilotProfile g_stCurrentPilotProfile = { 0 }; /* 0x00493408 */
short g_nExternalViewShip = -1; /* 0x00493468 */
short g_nYourWingman = -1; /* 0x0049346c */
signed char g_acPlayerComponentDamage[9] = { -1 }; /* 0x00493470 */
short g_bDisplayWingmanTargetData = 0; /* 0x0049347c */
short g_nPlayerCollisionObject = -1; /* 0x00493480 */
FixedVector g_vPlayerAcceleration = { 0 }; /* 0x00493488 */
int g_aaiVduModeStack[3][4] = { 0 }; /* 0x00493498 */
signed char g_acVduModeStackDepth[2] = { -1, 0 }; /* 0x004934c8 */
int g_anLastDrawnVduMode[3] = { 0 }; /* 0x004934d0 */
short g_nSelectedGunType = -1; /* 0x004934dc */
short g_nSelectedReleaseWeaponIndex = -1; /* 0x004934e0 */
short g_nCockpitResetState = 0; /* 0x004934e4 */
short g_bTargetLockReadoutDirty = 0; /* 0x004934e8 */
short g_nTargetLockCountdown = 0; /* 0x004934ec */
short g_nTargetLockMarkerAngle = 0; /* 0x004934f0 */
signed char g_cTargetDisplayObject = -1; /* 0x004934f4 */
int g_nUnusedTargetWord = 0; /* 0x004934f8 */   /* no references in the original */
short g_bTargetLockAcquired = 0; /* 0x004934fc */
short g_bTargetLockMode = 0; /* 0x00493500 */
short g_bMissileCameraEnabled = 0; /* 0x00493504 */
short g_asObjectViewFrame[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00493508 */
short g_asObjectScreenX[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00493598 */
short g_asObjectScreenY[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00493628 */
short g_asObjectScreenAngle[SPACE_OBJECT_COUNT] = { 0 }; /* 0x004936b8 */
short g_asObjectDrawX[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00493748 */
short g_asObjectDrawY[SPACE_OBJECT_COUNT] = { 0 }; /* 0x004937d8 */
unsigned char *g_apObjectShape[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00493868 */
signed char g_acObjectType[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00493980 */
short g_asObjectFlip[SPACE_OBJECT_COUNT] = { 0 }; /* 0x004939c8 */
short g_asObjectScreenScale[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00493a58 */
short g_asObjectDistance[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00493ae8 */
FixedVector g_aShipRightVector[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00493b78 */
FixedVector g_aShipUpVector[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00493ec0 */
FixedVector g_aShipForwardVector[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00494208 */
FixedVector g_aShipPosition[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00494550 */
FixedVector g_aShipVelocity[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00494898 */
short g_asObjectCounter[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00494be0 */
short g_asObjectAnimationIndex[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00494c70 */
signed char g_acObjectAnimationDelay[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00494d00 */
signed char g_acObjectCollisionGraceTicks[
    SPACE_OBJECT_COUNT] = { 0 }; /* 0x00494d48 */
unsigned short g_asObjectScale[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00494d90 */
int g_anShipSpeed[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00494e20 */
short g_anObjectPitchRotation[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00494f38 */
short g_anObjectYawRotation[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00494fc8 */
short g_anObjectRollRotation[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00495058 */
short g_asObjectCollisionRadius[70] = { 0 }; /* 0x004950e8 */
short g_asObjectDamage[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00495178 */
signed char g_acObjectOwner[SPACE_OBJECT_COUNT] = { -1 }; /* 0x00495208 */
signed char g_acLastCollisionObject[SPACE_OBJECT_COUNT] = { -1 }; /* 0x00495250 */
short g_asObjectType[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00495298 */
short g_aeObjectClass[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00495328 */
unsigned char *g_apObjectExhaustShape[10] = { 0 }; /* 0x004953b8 */
signed char g_acShipNavPoint[12] = { -1 }; /* 0x004953e0 */
FixedVector g_aShipDestination[10] = { 0 }; /* 0x004953f0 */
ShortVector g_aShipFormationOffset[10] = { 0 }; /* 0x00495468 */
short g_anPitchGoal[12] = { 0 }; /* 0x004954a8 */
short g_anYawGoal[12] = { 0 }; /* 0x004954c0 */
short g_anRollGoal[12] = { 0 }; /* 0x004954d8 */
short g_aasShipMaximumShield[10][2] = { 0 }; /* 0x004954f0 */
short g_aasShipShield[10][2] = { 0 }; /* 0x00495518 */
short g_aasShipArmor[10][4] = { 0 }; /* 0x00495540 */
short g_asShipWeaponEnergy[10] = { 0 }; /* 0x00495590 */
short g_asShipAfterburnerTimer[10] = { 0 }; /* 0x004955a8 */
signed char g_acShipLastAttacker[16] = { -1 }; /* 0x004955c0 */
short g_asShipSide[12] = { 2 }; /* 0x004955d0 */
short g_asActionCount[12] = { 0 }; /* 0x004955e8 */
#ifdef SDL_PORT
short g_aeSpecialManeuver[SPACE_OBJECT_COUNT] = { 0 }; /* 0x00495600 */
#else
short g_aeSpecialManeuver[12] = { 0 }; /* 0x00495600 */
#endif
signed char g_acTurnRegulator[16] = { -1 }; /* 0x00495618 */
signed char g_acTurnInterval[16] = { -1 }; /* 0x00495628 */
int g_anShipFuel[10] = { 0 }; /* 0x00495638 */
signed char g_acShipExhaustHeat[16] = { -1 }; /* 0x00495660 */
signed char g_acShipComputerDamage[16] = { -1 }; /* 0x00495670 */
signed char g_acShipCommunicator[16] = { -1 }; /* 0x00495680 */
signed char g_acShipDamage[16] = { -1 }; /* 0x00495690 */
signed char g_acShipIonDriveDamage[16] = { -1 }; /* 0x004956a0 */
unsigned char g_aShipWeapons[10][0xA1] = {
    { 0xff, 0, 0, 0, 0, 0xff, 0xff }
}; /* 0x004956b0 */
short g_asShipMissionIndex[12] = { 0 }; /* 0x00495d00 */
short g_asLoadedShipViewFrame[12] = { 0 }; /* 0x00495d18 */
short g_asShipIntelSlot[10] = { 0 }; /* 0x00495d30 */
short g_asShipIntelResourceKey[10] = { 0 }; /* 0x00495d48 */
short g_asPilotLevel[12] = { 0 }; /* 0x00495d60 */
signed char g_acPilotHitPoints[16] = { -1 }; /* 0x00495d78 */
signed char g_acShipPortrait[12] = { -1 }; /* 0x00495d88 */
signed char g_acShipPendingMessage[16] = { -1 }; /* 0x00495d98 */
MissionShipRecord *g_apShipMissionRecord[10] = { 0 }; /* 0x00495da8 */
short g_asShipWingLeader[12] = { 0 }; /* 0x00495dd0 */
short g_asShipMissionType[12] = { 0 }; /* 0x00495de8 */
short g_asShipMissionParameter[12] = { 0 }; /* 0x00495e00 */
FixedVector g_aShipMissionSpot[20] = { 0 }; /* 0x00495e18 */
short g_aeShipObjective[12] = { 0 }; /* 0x00495f08 */
signed char g_acShipTarget[16] = { -1 }; /* 0x00495f20 */
short g_asShipTactic[12] = { 0 }; /* 0x00495f30 */
short g_asShipManeuver[12] = { 29 }; /* 0x00495f48 */
signed char g_abShipNavPointIndex[16] = { -1 }; /* 0x00495f60 */
short g_asShipMaximumVelocity[12] = { 0 }; /* 0x00495f70 */
unsigned int g_anShipAlertFlags[10] = { 0 }; /* 0x00495f88 */
signed char g_acShipAlertSource[12] = { -1 }; /* 0x00495fb0 */
short g_asShipAlertCountdown[12] = { 0 }; /* 0x00495fc0 */
signed char g_abShipTurn[16] = { -1 }; /* 0x00495fd8 */
signed char g_acShipSequence[14] = { -1 }; /* 0x00495fe8 */
short g_asShipCount[12] = { 0 }; /* 0x00495ff8 */
signed char g_acShipCollisionCooldown[16] = { 0 }; /* 0x00496010 */
int g_anShipCloakState[10] = { -1 }; /* 0x00496020 */
short g_asShipCloakCooldown[10] = { 0 }; /* 0x00496048 */
short g_asShipCloakElapsedFrames[10] = { 0 }; /* 0x00496060 */
short g_asShipIdentified[12] = { 1 }; /* 0x00496078 */
short g_asShipFriendlyFireCooldown[10] = { 0 }; /* 0x00496090 */
signed char g_abProjectileCollisionBonus[
    SPACE_OBJECT_COUNT] = { 0 }; /* 0x004960a8 */
#ifndef SDL_PORT
short g_asProjectileEvasionTicks[8] = { 0 }; /* 0x004960f0 */
#else
short g_asProjectileEvasionTicks[10] = { 0 }; /* 0x004960f0 */
#endif
signed char g_acShipStress[16] = { 0 }; /* 0x00496100 */
signed char g_abPilotEjectionAttempted[16] = { 0 }; /* 0x00496110 */
signed char g_abShipEjectionSequenceEnabled[12] = { 0 }; /* 0x00496120 */
short g_bEngageAllowed = 0; /* 0x0049612c */
short g_nAutoEngageTimer = -1; /* 0x00496130 */
int g_nUnusedWingmanWord = 0; /* 0x00496134 */   /* no references in the original */
ShipIntelligenceMetadata g_stWingmanIntelligence = { 0 }; /* 0x00496138 */
int g_nMissionMusicTrackOverride = 0; /* 0x00496144 */
signed char g_acShipList[16] = { -1 }; /* 0x00496148 */
short g_asShipListRange[16] = { 0 }; /* 0x00496158 */
signed char g_cViableTargetCount = 0; /* 0x00496178 */
signed char g_acViableTarget[16] = { -1 }; /* 0x00496180 */
short g_asViableTargetDistance[10] = { 0 }; /* 0x00496190 */
unsigned int g_dwCannedSceneSnapshotEnd = 0; /* 0x004961a4 */
