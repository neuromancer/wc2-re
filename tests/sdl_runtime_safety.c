#include "game.h"

int main(int argumentCount, char **arguments)
{
    float thrusterScreenX;
    float thrusterScreenY;
    short collisionPartner = 15;
    short object;
    short ship = 1;
    short target = 2;

    g_asShipSide[0] = SIDE_IMPERIAL;
    g_asShipSide[1] = SIDE_KILRATHI;
    g_acPlayerComponentDamage[5] = 0;
    g_asObjectScreenX[1] = 0;
    g_asObjectScreenY[1] = 0;
    g_nTargetLockCountdown = 0;
    g_nSelectedReleaseWeaponIndex = -1;
    target_locking(1);
    if (g_nTargetLockCountdown != -1)
        return 1;

    /* A fifth VDU mode cannot fit in the four-entry stack.  The native port
     * drops that push and leaves the current mode in place. */
    g_acVduModeStackDepth[1] = 3;
    g_aaiVduModeStack[1][3] = 6;
    push_mode(1, 4);
    if (get_mode(1) != 6 || g_acVduModeStackDepth[1] != 3)
        return 1;

    /* A transmission VDU can be revisited after its speaker has been cleared.
     * WC2 then reads the zero word immediately before the ship-side table. */
    g_nCommDeathSequenceFrame = 0;
    g_bCommSpeechPlaying = 0;
    g_nCommSpeakerObject = -1;
    g_nCommPortraitIndex = -1;
    g_acShipLastAttacker[14] = 0;
    g_acShipLastAttacker[15] = 0;
    vid_transmit();
    if (get_mode(1) != 6)
        return 1;

    g_acShipTarget[ship] = (signed char)target;
    g_asShipManeuver[ship] = MANEUVER_BUZZ_DEBRIS;
    g_acShipSequence[ship] = 2;
    g_aeSpecialManeuver[ship] = SPECIAL_MANEUVER_NONE;
    g_aeObjectClass[target] = OBJECT_CLASS_SHIP;
    g_asShipManeuver[target] = MANEUVER_NONE;
    g_aShipPosition[ship].x = 0;
    g_aShipPosition[ship].y = 0;
    g_aShipPosition[ship].z = 0;
    g_aShipPosition[target].x = 0;
    g_aShipPosition[target].y = 0;
    g_aShipPosition[target].z = 0x10000;
    g_aShipForwardVector[ship].z = 0x100;
    g_aShipForwardVector[target].z = 0x100;
    perform_maneuver(ship);
    if (g_asShipManeuver[ship] != MANEUVER_NONE)
        return 1;

    /* With no target, WC2 reads the zero-filled word immediately before the
     * collision-radius table.  Preserve that result without crossing a
     * native global's sanitizer redzone. */
    g_acShipTarget[ship] = -1;
    g_asShipManeuver[ship] = MANEUVER_THINKING;
    g_asObjectCollisionRadius[ship] = 25;
    g_nTargetFacing = -1;
    SetShipAiScratchWord(0);
    perform_maneuver(ship);
    if (g_asShipManeuver[ship] != MANEUVER_NONE)
        return 1;

    /* An unattributed destruction reads the zero-filled byte immediately
     * before the owner table in WC2, turning attacker -1 into object 0. */
    g_asPilotLevel[target] = 0;
    g_aeSpecialManeuver[target] = SPECIAL_MANEUVER_NONE;
    g_aeObjectClass[target] = OBJECT_CLASS_CAPITAL_SHIP;
    g_abShipEjectionSequenceEnabled[target] = 1;
    g_asShipSide[0] = SIDE_KILRATHI;
    g_asShipSide[target] = SIDE_IMPERIAL;
    g_nInFlightMusicActive = 0;
    g_36_cPilotKillCountValue = 0;
    if (ResolveObjectDestruction(-1, target) != 0 ||
        g_36_cPilotKillCountValue != 1)
        return 1;
    g_asShipSide[0] = SIDE_IMPERIAL;

    /* A base colliding with an asteroid in slot 15 produces a negative mass
     * response and reaches beyond WC2's twelve ship-velocity entries. */
    for (object = 0; object <= SPACE_LAST_MOVING_OBJECT; object++) {
        g_aeObjectClass[object] = OBJECT_CLASS_NULL;
        g_asObjectScreenX[object] = (short)0x8001;
    }
    g_aeObjectClass[0] = OBJECT_CLASS_BASE;
    g_aeObjectClass[collisionPartner] = OBJECT_CLASS_ASTEROID;
    g_acObjectType[0] = 0;
    g_acObjectType[collisionPartner] = 1;
    g_aObjectTypeData[0].radarRadius = 25;
    g_aObjectTypeData[1].radarRadius = 200;
    g_asObjectCollisionRadius[0] = 10;
    g_asObjectCollisionRadius[collisionPartner] = 10;
    zero_vector(&g_aShipPosition[0]);
    zero_vector(&g_aShipPosition[collisionPartner]);
    zero_vector(&g_aShipVelocity[0]);
    zero_vector(&g_aShipVelocity[collisionPartner]);
    g_aShipPosition[collisionPartner].x = 0x100;
    g_acObjectOwner[collisionPartner] = -1;
    g_acLastCollisionObject[0] = -1;
    g_acLastCollisionObject[collisionPartner] = -1;
    g_asShipMaximumVelocity[0] = 1;
    g_bPlayerCollisionEnabled = 1;
    g_nCurrentView = 1;
    object_collision(0);
    if (g_acLastCollisionObject[0] != collisionPartner ||
        g_acLastCollisionObject[collisionPartner] != 0)
        return 1;

    /* Autopilot's view-13 camera draws the running engines as fixed children.
     * Their enhanced positions need the parent's view-space coordinates. */
    for (object = 0; object <= SPACE_LAST_MOVING_OBJECT; object++) {
        g_aeObjectClass[object] = OBJECT_CLASS_NULL;
        g_asObjectScreenX[object] = (short)0x8001;
    }
    init_ijk(EYE_OBJECT);
    zero_vector(&g_aShipPosition[EYE_OBJECT]);
    g_asObjectCollisionRadius[EYE_OBJECT] = 0;
    g_nScreenWidth = 320;
    g_15_nStoredScreenViewCentreX = 160;
    g_37_nViewCenterY = 100;
    g_aeObjectClass[ship] = OBJECT_CLASS_PROJECTILE;
    g_asObjectCollisionRadius[ship] = 0;
    g_asObjectScale[ship] = 0x100;
    g_aShipPosition[ship].x = 0;
    g_aShipPosition[ship].y = 0;
    g_aShipPosition[ship].z = 0x20000;
    transform_objects_to_your_view();
    if (g_aObjectViewPosition[ship].z != 0x20000)
        return 1;

    object = 10;
    g_aeObjectClass[object] = OBJECT_CLASS_FIXED_OBJECT;
    g_asObjectType[object] = OBJECT_DATA_THRUSTERS;
    g_acObjectOwner[object] = (signed char)ship;
    g_asObjectScreenX[object] = 0;
    g_asObjectScreenY[object] = 0;
    g_asObjectDistance[object] = 0;
    g_asObjectScale[object] = 0x100;
    g_asObjectScreenAngle[ship] = 0;
    g_asObjectFlip[ship] = 0;
    reposition_fixed_child_objects();
    SdlGetThrusterScreenPosition(
        object, &thrusterScreenX, &thrusterScreenY);
    if (thrusterScreenX != 160.0f || thrusterScreenY != 100.0f)
        return 1;

    /* Sound teardown precedes cutscene-object teardown.  It must also drop
     * the non-owning mouth-sprite pointer before that object is freed. */
    g_pLinkedCutsceneSprite = (SceneFlicObject *)1;
    g_3_bAfterburnerSfxActive = 1;
    stop_all_sounds();
    if (g_3_bAfterburnerSfxActive != 0 ||
        g_pLinkedCutsceneSprite != 0)
        return 1;

    /* Explicit newlines in a cutscene caption are separate lines, not the
     * end of the string.  The intro location card contains two of them. */
    {
        unsigned char font[2] = {3, 0};
        TextContext context = {0};
        Viewport viewport = {0};

        viewport.left = 4;
        viewport.right = 315;
        context.viewport = &viewport;
        context.cursorY = 7;
        context.font = font;
        SetTextContext(&context);
        DrawTextString("\n\n");
        if (context.cursorX != 4 || context.cursorY != 13)
            return 1;
    }

    /* Retail hides ordinary cinematic text while a speech packet is loaded.
     * The native port keeps that same scripted text visible. */
    {
        unsigned char font[2] = {3, 0};
        char textBuffer[8] = {0};
        const char caption[] = "\n\n";
        TextContext context = {0};
        Viewport viewport = {0};

        viewport.left = 4;
        viewport.right = 315;
        context.viewport = &viewport;
        context.cursorY = 7;
        context.font = font;
        context.text = textBuffer;
        g_2_stCutsceneMessageCtxInfo = context;
        g_10_stCutsceneStringViewportBlock = viewport;
        g_101_stScrViewInfo.pixels = (unsigned char *)1;
        g_12_apszMovieSpeechFiles[0] = "speech.s00";
        g_27_pszCutsceneFormattedText = 0;
        g_nAudioEnabled = 1;
        g_9_bCinematicSpriteTextEnableStatus = 0;

        DrawCutsceneTextAt(-1, -1, 2, caption);
        if (g_16_stCutsceneDrawingTextSettingsInfo.cursorY != 13)
            return 1;
        g_12_apszMovieSpeechFiles[0] = 0;
    }

    /* A delete-on-stop speech object can disappear during sound service. */
    {
        SceneFlicObject speaker = {0};
        int service;

        speaker.currentFrame = 5;
        speaker.waitTicks = 6;
        g_nAudioEnabled = 1;
        g_1_bSpaceFlightActive = 0;
        g_pSpeechSnd = (IxSound *)1;
        g_bSpeechSoundActive = 1;
        g_16_nSpeechCompletionDelay = 0;
        g_bCutsceneSpeechActive = 1;
        g_2_bMovieStringAdvanceState = 1;
        g_pszCutsceneSpeechCursor = (char *)1;
        g_pLinkedCutsceneSprite = &speaker;
        g_nInputPressCount = 0;
        SetCinematicFrameTiming(20.0f);
        ServiceSoundSystem();
        if (g_pSpeechSnd != 0 ||
            g_bSpeechSoundActive != 0 ||
            g_16_nSpeechCompletionDelay != 1 ||
            g_bCutsceneSpeechActive != 0 ||
            g_2_bMovieStringAdvanceState != 0 ||
            g_pszCutsceneSpeechCursor != 0 ||
            speaker.currentFrame != 11 || speaker.waitTicks != 0 ||
            g_17_nFramePeriodMillis != 50) {
            return 1;
        }

        for (service = 0; service < 20; service++)
            ServiceSoundSystem();
        if (g_16_nSpeechCompletionDelay != 21 ||
            g_nInputPressCount != 1 ||
            g_17_nFramePeriodMillis != 14) {
            fprintf(stderr,
                    "speech completion: delay %d, input presses %d, frame period %ld; expected 21, 1, 14\n",
                    g_16_nSpeechCompletionDelay, g_nInputPressCount,
                    g_17_nFramePeriodMillis);
            return 1;
        }
        g_pLinkedCutsceneSprite = 0;
    }

    return 0;
}
