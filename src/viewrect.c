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
short g_asDifficultyLevels_004930a8[16][4] = {
    50, 200, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127
};
signed char g_cCurrentKey_00493128 = (signed char)0x80;
signed char g_cPreviousKey_0049312c = (signed char)0x80;
unsigned int g_dwCannedSceneSnapshotStart_00493130 = 0;
short g_nSpaceFrame_00493134 = 0;
short g_nRenderedSpaceFrame_00493138 = 0;
signed char g_cViewObject_0049313c = -1;
short g_nEyePitchGoal_00493140 = 0;
short g_nEyeYawGoal_00493144 = 0;
short g_nEyeRollGoal_00493148 = 0;
short g_nEyePitchRate_0049314c = 1;
short g_nEyeYawRate_00493150 = 1;
short g_nEyeRollRate_00493154 = 1;
FixedVector g_vStarFieldMotion_00493158 = { 0 };
FixedVector g_vPreviousStarFieldMotion_00493168 = { 0 };
FixedVector g_vCollisionDelta_00493178 = { 0 };
FixedVector g_vToTarget_00493188 = { 0 };
short g_nFacingToTarget_00493194 = 0;
short g_nTargetFacing_00493198 = 0;
short g_nTargetRange_0049319c = 0;
short g_nTargetShip_004931a0 = 0;
short g_bExpandedShipGraphicsEnabled_004931a4 = 0;
short g_anFlightInput_004931a8[3] = { 0 };
short g_anPreviousFlightInput_004931b0[3] = { 0 };
short g_nNavPointerObject_004931b8 = -1;
short g_nCurrentNavPoint_004931bc = 0;
short g_nCurrentWave_004931c0 = -1;
short g_nLoadedCommPortraitPilot_004931c4 = -1;
int g_nCurrentObjectiveRange_004931c8 = 0;
signed char g_cCurrentObjective_004931cc = -1;
short g_nHazardFieldCount_004931d0 = 0;
HazardField g_aHazardFields_004931d8[8] = { { -1 } };
HazardField *g_pActiveHazardField_00493278 = 0;
short g_bUseEyePositionForHazards_0049327c = 0;
signed char g_abHazardObjects_00493280[0x14] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
signed char g_cMissionObjectiveCount_00493294 = -1;
signed char g_cCurrentNavPointIndex_00493298 = -1;
#ifdef SDL_PORT
signed char g_abFlightPath_004932a0[MISSION_OBJECTIVE_COUNT + 1] = { -1 };
#else
signed char g_abFlightPath_004932a0[MISSION_OBJECTIVE_COUNT] = { -1 };
#endif
#ifdef SDL_PORT
/* The type read at index 8 precedes cleanup_objectives' bounds check; the
 * retail read at 0x49339D falls inside the adjacent object-resource table. */
MissionObjective g_aMissionObjectives_004932a8[MISSION_OBJECTIVE_COUNT + 1] = {
#else
MissionObjective g_aMissionObjectives_004932a8[MISSION_OBJECTIVE_COUNT] = {
#endif
    { 0, 0, 0xff, 7, -1, 0xff, -1 }
};
ObjectResourceSlot g_aObjectResourceSlots_00493398[5] = { { -1 } };
PilotProfile g_stCurrentPilotProfile_00493408 = { 0 };
short g_nExternalViewShip_00493468 = -1;
short g_nYourWingman_0049346c = -1;
signed char g_acPlayerComponentDamage_00493470[9] = { -1 };
short g_bDisplayWingmanTargetData_0049347c = 0;
short g_nPlayerCollisionObject_00493480 = -1;
FixedVector g_vPlayerAcceleration_00493488 = { 0 };
int g_aaiVduModeStack_00493498[3][4] = { 0 };
signed char g_acVduModeStackDepth_004934c8[2] = { -1, 0 };
int g_anLastDrawnVduMode_004934d0[3] = { 0 };
short g_nSelectedGunType_004934dc = -1;
short g_nSelectedReleaseWeaponIndex_004934e0 = -1;
short DAT_004934e4 = 0;
short g_bTargetLockReadoutDirty_004934e8 = 0;
short g_nTargetLockCountdown_004934ec = 0;
short g_nTargetLockMarkerAngle_004934f0 = 0;
signed char g_cTargetDisplayObject_004934f4 = -1;
int g_nUnusedTargetWord_004934f8 = 0;   /* no references in the original */
short g_bTargetLockAcquired_004934fc = 0;
short g_bTargetLockMode_00493500 = 0;
short g_bMissileCameraEnabled_00493504 = 0;
short g_asObjectViewFrame_00493508[SPACE_OBJECT_COUNT] = { 0 };
short g_asObjectScreenX_00493598[SPACE_OBJECT_COUNT] = { 0 };
short g_asObjectScreenY_00493628[SPACE_OBJECT_COUNT] = { 0 };
short g_asObjectScreenAngle_004936b8[SPACE_OBJECT_COUNT] = { 0 };
short g_asObjectDrawX_00493748[SPACE_OBJECT_COUNT] = { 0 };
short g_asObjectDrawY_004937d8[SPACE_OBJECT_COUNT] = { 0 };
unsigned char *g_apObjectShape_00493868[SPACE_OBJECT_COUNT] = { 0 };
signed char g_acObjectType_00493980[SPACE_OBJECT_COUNT] = { 0 };
short g_asObjectFlip_004939c8[SPACE_OBJECT_COUNT] = { 0 };
short g_asObjectScreenScale_00493a58[SPACE_OBJECT_COUNT] = { 0 };
short g_asObjectDistance_00493ae8[SPACE_OBJECT_COUNT] = { 0 };
FixedVector g_aShipRightVector_00493b78[SPACE_OBJECT_COUNT] = { 0 };
FixedVector g_aShipUpVector_00493ec0[SPACE_OBJECT_COUNT] = { 0 };
FixedVector g_aShipForwardVector_00494208[SPACE_OBJECT_COUNT] = { 0 };
FixedVector g_aShipPosition_00494550[SPACE_OBJECT_COUNT] = { 0 };
FixedVector g_aShipVelocity_00494898[SPACE_OBJECT_COUNT] = { 0 };
short g_asObjectCounter_00494be0[SPACE_OBJECT_COUNT] = { 0 };
short g_asObjectAnimationIndex_00494c70[SPACE_OBJECT_COUNT] = { 0 };
signed char g_acObjectAnimationDelay_00494d00[SPACE_OBJECT_COUNT] = { 0 };
signed char g_acObjectCollisionGraceTicks_00494d48[
    SPACE_OBJECT_COUNT] = { 0 };
unsigned short g_asObjectScale_00494d90[SPACE_OBJECT_COUNT] = { 0 };
int g_anShipSpeed_00494e20[SPACE_OBJECT_COUNT] = { 0 };
short g_anObjectPitchRotation_00494f38[SPACE_OBJECT_COUNT] = { 0 };
short g_anObjectYawRotation_00494fc8[SPACE_OBJECT_COUNT] = { 0 };
short g_anObjectRollRotation_00495058[SPACE_OBJECT_COUNT] = { 0 };
short g_asObjectCollisionRadius_004950e8[70] = { 0 };
short g_asObjectDamage_00495178[SPACE_OBJECT_COUNT] = { 0 };
signed char g_acObjectOwner_00495208[SPACE_OBJECT_COUNT] = { -1 };
signed char g_acLastCollisionObject_00495250[SPACE_OBJECT_COUNT] = { -1 };
short g_asObjectType_00495298[SPACE_OBJECT_COUNT] = { 0 };
short g_aeObjectClass_00495328[SPACE_OBJECT_COUNT] = { 0 };
unsigned char *g_apObjectExhaustShape_004953b8[10] = { 0 };
signed char g_acShipNavPoint_004953e0[12] = { -1 };
FixedVector g_aShipDestination_004953f0[10] = { 0 };
ShortVector g_aShipFormationOffset_00495468[10] = { 0 };
short g_anPitchGoal_004954a8[12] = { 0 };
short g_anYawGoal_004954c0[12] = { 0 };
short g_anRollGoal_004954d8[12] = { 0 };
short g_aasShipMaximumShield_004954f0[10][2] = { 0 };
short g_aasShipShield_00495518[10][2] = { 0 };
short g_aasShipArmor_00495540[10][4] = { 0 };
short g_asShipWeaponEnergy_00495590[10] = { 0 };
short g_asShipAfterburnerTimer_004955a8[10] = { 0 };
signed char g_acShipLastAttacker_004955c0[16] = { -1 };
short g_asShipSide_004955d0[12] = { 2 };
short g_asActionCount_004955e8[12] = { 0 };
#ifdef SDL_PORT
short g_aeSpecialManeuver_00495600[SPACE_OBJECT_COUNT] = { 0 };
#else
short g_aeSpecialManeuver_00495600[12] = { 0 };
#endif
signed char g_acTurnRegulator_00495618[16] = { -1 };
signed char g_acTurnInterval_00495628[16] = { -1 };
int g_anShipFuel_00495638[10] = { 0 };
signed char g_acShipExhaustHeat_00495660[16] = { -1 };
signed char g_acShipComputerDamage_00495670[16] = { -1 };
signed char g_acShipCommunicator_00495680[16] = { -1 };
signed char g_acShipDamage_00495690[16] = { -1 };
signed char g_acShipIonDriveDamage_004956a0[16] = { -1 };
unsigned char g_aShipWeapons_004956b0[10][0xA1] = {
    { 0xff, 0, 0, 0, 0, 0xff, 0xff }
};
short g_asShipMissionIndex_00495d00[12] = { 0 };
short g_asLoadedShipViewFrame_00495d18[12] = { 0 };
short g_asShipIntelSlot_00495d30[10] = { 0 };
short g_asShipIntelResourceKey_00495d48[10] = { 0 };
short g_asPilotLevel_00495d60[12] = { 0 };
signed char DAT_00495d78[16] = { -1 };
signed char g_acShipPortrait_00495d88[12] = { -1 };
signed char g_acShipPendingMessage_00495d98[16] = { -1 };
MissionShipRecord *g_apShipMissionRecord_00495da8[10] = { 0 };
short g_asShipWingLeader_00495dd0[12] = { 0 };
short g_asShipMissionType_00495de8[12] = { 0 };
short g_asShipMissionParameter_00495e00[12] = { 0 };
FixedVector g_aShipMissionSpot_00495e18[20] = { 0 };
short g_aeShipObjective_00495f08[12] = { 0 };
signed char g_acShipTarget_00495f20[16] = { -1 };
short g_asShipTactic_00495f30[12] = { 0 };
short g_asShipManeuver_00495f48[12] = { 29 };
signed char g_abShipNavPointIndex_00495f60[16] = { -1 };
short g_asShipMaximumVelocity_00495f70[12] = { 0 };
unsigned int g_anShipAlertFlags_00495f88[10] = { 0 };
signed char g_acShipAlertSource_00495fb0[12] = { -1 };
short g_asShipAlertCountdown_00495fc0[12] = { 0 };
signed char g_abShipTurn_00495fd8[16] = { -1 };
signed char g_acShipSequence_00495fe8[14] = { -1 };
short g_asShipCount_00495ff8[12] = { 0 };
signed char g_acShipCollisionCooldown_00496010[16] = { 0 };
int g_anShipCloakState_00496020[10] = { -1 };
short g_asShipCloakCooldown_00496048[10] = { 0 };
short g_asShipCloakElapsedFrames_00496060[10] = { 0 };
short g_asShipIdentified_00496078[12] = { 1 };
short g_asShipFriendlyFireCooldown_00496090[10] = { 0 };
signed char g_abProjectileCollisionBonus_004960a8[
    SPACE_OBJECT_COUNT] = { 0 };
#ifndef SDL_PORT
short DAT_004960f0[8] = { 0 };
#else
short DAT_004960f0[10] = { 0 };
#endif
signed char g_acShipStress_00496100[16] = { 0 };
signed char g_abPilotEjectionAttempted_00496110[16] = { 0 };
signed char g_abShipEjectionSequenceEnabled_00496120[12] = { 0 };
short g_bEngageAllowed_0049612c = 0;
short g_nAutoEngageTimer_00496130 = -1;
int g_nUnusedWingmanWord_00496134 = 0;   /* no references in the original */
ShipIntelligenceMetadata g_stWingmanIntelligence_00496138 = { 0 };
int g_nMissionMusicTrackOverride_00496144 = 0;
signed char g_acShipList_00496148[16] = { -1 };
short g_asShipListRange_00496158[16] = { 0 };
signed char g_cViableTargetCount_00496178 = 0;
signed char g_acViableTarget_00496180[16] = { -1 };
short g_asViableTargetDistance_00496190[10] = { 0 };
unsigned int g_dwCannedSceneSnapshotEnd_004961a4 = 0;
