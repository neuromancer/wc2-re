/*
 *  Win32 shell: window creation, message loop, exit paths.
 *
 *  Address range 0x401000-0x402dff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: CreateMainWindow/MainWindowProc/AbortToDesktop cluster; string band 0x465048-0x465354.
 */
#include "wc1.h"

#ifndef WC1_SDL
#pragma function(abs)
#endif

/* Function start: 0x417550 */
void SaveGamePalette(void)
{
    unsigned short index;

    for (index = 0; (short)index < 256; index++) {
        if ((index & 15) == 0)
            WaitForVerticalBlankThunk();
        GetPaletteEntry(index, g_ausPaletteWords_005d3220[index]);
    }
}

/* Function start: 0x4175AD */
void RestoreGamePalette(void)
{
    short index;

    for (index = 0; (short)index < 256; index++) {
        if ((index & 15) == 0)
            WaitForVerticalBlankThunk();
        SetPaletteEntry(index, (short *)g_ausPaletteWords_005d3220[index]);
    }
}

/* Function start: 0x417610 */
short easy2see(short obj)
{
    short bounds[4];
    short x;
    short y;

    if (g_asObjectScreenX_00493598[obj] == (short)0x8001)
        return 0;
    x = (short)(g_asObjectScreenX_00493598[obj] + g_nViewCenterX_005c80d8);
    y = (short)(g_asObjectScreenY_00493628[obj] + g_nViewCenterY_005c80da);
    return GetTransformedShapeBounds(
        &g_stViewBuffer_005d2b00, x, y, g_apObjectShape_00493868[obj],
        g_asObjectViewFrame_00493508[obj],
        g_asObjectScreenAngle_004936b8[obj],
        g_asObjectScreenScale_00493a58[obj],
        g_asObjectFlip_004939c8[obj], bounds);
}

/* Function start: 0x4176D2 */
void make_shard(short asteroid, FixedVector direction)
{
    short fragment;
    short speed;

    fragment = find_vacant_3d_object();
    if (fragment == -1)
        return;
    set_objects_data(fragment, WC2_OBJECT_TYPE_ROCK_CHUNK, asteroid, 0);
    g_asObjectCounter_00494be0[fragment] = 40;
    g_acObjectOwner_00495208[fragment] = (signed char)asteroid;
    SetVectorFixedPoint((unsigned int *)&direction,
                        (short)(g_asObjectCollisionRadius_004950e8[asteroid] >> 1));
    AddFixedVectors(&g_aShipPosition_00494550[asteroid], &direction,
                    &g_aShipPosition_00494550[fragment]);
    g_aShipForwardVector_00494208[fragment] = direction;
    fix_objects_ijk(fragment);
    alter_yaw(signed_random(20), fragment);
    alter_pitch(signed_random(20), fragment);
    g_aShipVelocity_00494898[fragment] =
        g_aShipForwardVector_00494208[fragment];
    speed = (short)(real_velocity(asteroid) + RandomInRange(0, 5));
    SetVectorFixedPoint(
        (unsigned int *)&g_aShipVelocity_00494898[fragment], speed);
}

/* Base flight times and pitch windows for the four forward view bands. */
static const signed char g_acHazardTravelTimeByView_00465048[8] = {
    56, 52, 75, 73, 0, 0, 0, 0
};

static const signed char g_acHazardPitchRange_00465050[8] = {
    -10, 4, -8, 8, -12, 8, -8, 8
};

/* Function start: 0x417838 */
void remove_hazard(signed char hazard)
{
    remove_object((short)hazard);
    g_nActiveHazards_00492e5c =
        MaxShort(0, (short)(g_nActiveHazards_00492e5c - 1));
    RecordCannedSceneObjectEvent((short)hazard, 1);
}

/* Function start: 0x41787A */
void remove_all_hazards(void)
{
    short slot;

    for (slot = 0; slot < 20; slot++) {
        if (g_abHazardObjects_00493280[slot] != -1) {
            remove_hazard(g_abHazardObjects_00493280[slot]);
            g_abHazardObjects_00493280[slot] = -1;
        }
    }
    g_pActiveHazardField_00493278 = 0;
}

/* Function start: 0x4178E5 */
short difficulty(void)
{
    return (short)(abs(25 - (int)g_nHazardReferenceSpeed_00492e58) * 2);
}

/* Function start: 0x417916 */
short asteroid_velocity(void)
{
    return MinShort(20, (short)(RandomBelowOrEqual(7) + 10));
}

/* Function start: 0x417941 */
void skew_randomly(short obj, short allowReverse)
{
    FixedVector saved;

    if (RandomBelow(100) < 50) {
        saved = g_aShipRightVector_00493b78[obj];
        g_aShipRightVector_00493b78[obj] =
            g_aShipForwardVector_00494208[obj];
        g_aShipForwardVector_00494208[obj] =
            g_aShipUpVector_00493ec0[obj];
        g_aShipUpVector_00493ec0[obj] = saved;
    } else {
        saved = g_aShipUpVector_00493ec0[obj];
        g_aShipUpVector_00493ec0[obj] =
            g_aShipForwardVector_00494208[obj];
        g_aShipForwardVector_00494208[obj] =
            g_aShipRightVector_00493b78[obj];
        g_aShipRightVector_00493b78[obj] = saved;
    }
    if (allowReverse != 0 && RandomBelow(100) < 50)
        negate_vector(&g_aShipForwardVector_00494208[obj]);
}

/* Function start: 0x417AD7 */
void align(int *value, short quantum)
{
    *value -= *value % quantum;
}

/* Function start: 0x417AF9 */
void init_hazard(short obj, FixedVector position, short moving)
{
    short type;
    FixedVector vector;
    short hazardMoves;
    short travelTime;
    short speed;

    hazardMoves = moving;
    if (g_pActiveHazardField_00493278->type ==
        WC2_OBJECT_TYPE_ASTEROID_FIELD) {
        type = (short)(WC2_OBJECT_TYPE_ASTEROID1 +
                       RandomBelowOrEqual(5));
    } else {
        type = WC2_OBJECT_TYPE_SPACE_MINE;
    }
    set_objects_data(obj, type, -1, 0);
    g_aShipPosition_00494550[obj] = position;

    if (type == WC2_OBJECT_TYPE_SPACE_MINE) {
        hazardMoves = 0;
        point_at(obj, g_aShipPosition_00494550[0]);
        skew_randomly(obj, 1);
        speed = 2;
        hazardMoves = (short)(speed != 0);
    } else if (hazardMoves != 0) {
        travelTime = (short)(RandomBelowOrEqual(15) +
                             g_cHazardBaseTravelTime_0049af94);
        if (g_bUseEyePositionForHazards_0049327c != 0) {
            travelTime = (short)(travelTime -
                                 RandomBelowOrEqual(difficulty()));
            travelTime = MaxShort(45, travelTime);
        } else {
            travelTime = (short)(travelTime -
                                 RandomBelowOrEqual(difficulty()));
            travelTime = MaxShort(7, travelTime);
        }
        ScaleFixedVector(&g_aShipVelocity_00494898[0],
                         (int)travelTime << 8, &vector);
        AddFixedVectors(&g_aShipPosition_00494550[0], &vector, &vector);
        point_at(obj, vector);
        speed = distance_between_points(
            &vector, &g_aShipPosition_00494550[obj]);
        travelTime = MaxShort(3,
            (short)(travelTime - RandomBelow(5)));
        speed = (short)(speed / travelTime);
    } else {
        point_at(obj, g_pActiveHazardField_00493278->center);
        skew_randomly(obj, 1);
        if (RandomBelow(100) < 20)
            speed = 0;
        else
            speed = asteroid_velocity();
    }
    if (kilrathi_near(0, 16000) != 0)
        speed = 0;
    ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                     (int)speed << 8, &g_aShipVelocity_00494898[obj]);

    if (hazardMoves == 0) {
        int separation;

        if (type == WC2_OBJECT_TYPE_ASTEROID_FIELD)
            separation = 1500;
        else
            separation = RandomBelowOrEqual(1000) << 8;
        ScaleFixedVector(&g_aShipForwardVector_00494208[obj],
                         separation, &vector);
        SubtractFixedVectors(&g_aShipPosition_00494550[obj], &vector,
                             &g_aShipPosition_00494550[obj]);
    }
    if (type == WC2_OBJECT_TYPE_SPACE_MINE) {
        align(&g_aShipPosition_00494550[obj].x, 200);
        align(&g_aShipPosition_00494550[obj].y, 200);
        align(&g_aShipPosition_00494550[obj].z, 200);
    }
    g_acObjectCollisionGraceTicks_00494d48[obj] = 0;
    g_asObjectCounter_00494be0[obj] = 0;
    g_nActiveHazards_00492e5c++;
}

/* Function start: 0x417EA4 */
short near_field(const HazardField *field, const FixedVector *point)
{
    return IsPointWithinRange((FixedVector *)&field->center,
                              (FixedVector *)point,
                              (short)(field->innerRadius + 4300));
}

/* Function start: 0x417ED4 */
short within_field(const HazardField *field, const FixedVector *point)
{
    return IsPointWithinRange((FixedVector *)&field->center,
                              (FixedVector *)point, field->innerRadius);
}

/* Function start: 0x417EFF */
short try_far_spot(FixedVector *spot, short *moving)
{
    short yaw;
    short pitch;
    unsigned short outsideRange;

    copy_frame(0, WC2_SCRATCH_VIEW_OBJECT);
    g_aShipPosition_00494550[WC2_SCRATCH_VIEW_OBJECT] =
        g_aShipPosition_00494550[0];
    pitch = signed_random(20);
    yaw = signed_random(35);
    if (g_nCurrentView_00492fa8 == 0 && g_cCockpitView_0059dab0 <= 3) {
        signed char minimum;
        signed char maximum;

        minimum = g_acHazardPitchRange_00465050[
            g_cCockpitView_0059dab0 * 2];
        if (pitch > minimum &&
            pitch < (maximum = g_acHazardPitchRange_00465050[
                         g_cCockpitView_0059dab0 * 2 + 1]) &&
            abs(yaw) < 19 &&
            RandomBelow(100) < 60)
            *moving = 1;
        else
            *moving = 0;
    } else {
        if (abs(pitch) > 5 && abs(pitch) < 20 &&
            abs(yaw) > 5 && abs(yaw) < 20 && RandomBelow(100) < 30)
            *moving = 1;
        else
            *moving = 0;
    }
    pitch = (short)(pitch + find_ratio(
        -15, 15, g_anObjectPitchRotation_00494f38[0], -150, 150));
    yaw = (short)(yaw + find_ratio(
        -15, 15, g_anObjectYawRotation_00494fc8[0], -150, 150));
    rotate_about_j(yaw,
                   &g_aShipRightVector_00493b78[WC2_SCRATCH_VIEW_OBJECT],
                   &g_aShipForwardVector_00494208[WC2_SCRATCH_VIEW_OBJECT]);
    rotate_about_i(pitch,
                   &g_aShipUpVector_00493ec0[WC2_SCRATCH_VIEW_OBJECT],
                   &g_aShipForwardVector_00494208[WC2_SCRATCH_VIEW_OBJECT]);
    position_relative_ijk(spot, WC2_SCRATCH_VIEW_OBJECT, 0, 0, 3050);
    outsideRange = !(unsigned short)IsPointWithinRange(
        &g_aShipPosition_00494550[0], spot, 3000);
    return outsideRange != 0 &&
           within_field(g_pActiveHazardField_00493278, spot) != 0;
}

/* Function start: 0x418175 */
short rear_sphere(void)
{
    return find_ratio(0, 20, (short)g_nHazardReferenceSpeed_00492e58,
                      4300, 3100);
}

/* Function start: 0x4181A2 */
short ok_hazard_spot(short obj)
{
    int range;
    short result;

    if (g_asObjectScreenX_00493598[obj] == (short)0x8001)
        range = rear_sphere();
    else
        range = 4300;
    result = IsPointWithinRange(&g_aShipPosition_00494550[0],
                                &g_aShipPosition_00494550[obj],
                                (short)range);
    return result;
}

/* Function start: 0x41820B */
short make_hazard(void)
{
    FixedVector spot;
    short moving;
    short obj = find_vacant_3d_object();

    if (obj != -1 && try_far_spot(&spot, &moving) != 0) {
        init_hazard(obj, spot, moving);
        RecordCannedSceneObjectEvent(obj, 0);
    } else
        obj = -1;
    return obj;
}

/* Function start: 0x418288 */
void extra_hazard(short obj)
{
    if (g_aeObjectClass_00495328[obj] == OBJECT_CLASS_DUST)
        g_aeObjectClass_00495328[obj] = OBJECT_CLASS_NULL;
}

/* Function start: 0x4182B6 */
void approach(short obj)
{
    FixedVector vector;

    ScaleFixedVector(&g_aShipVelocity_00494898[0], 20 << 8, &vector);
    AddFixedVectors(&g_aShipPosition_00494550[0], &vector, &vector);
    point_at(obj, vector);
    ScaleFixedVector(&g_aShipForwardVector_00494208[obj], 20 << 8,
                     &vector);
    AddFixedVectors(&g_aShipVelocity_00494898[obj], &vector,
                    &g_aShipVelocity_00494898[obj]);
    if (g_nCannedSceneMode_0049021c == 0)
        RecordCannedSceneObjectEvent(obj, 3);
}

/* Function start: 0x41836E */
void manage_hazard(short obj, short slot)
{
    if (g_nRenderedSpaceFrame_00493138 % 20 != slot)
        return;
    if (ok_hazard_spot(obj) == 0) {
        remove_hazard((signed char)obj);
        return;
    }
    if (g_asObjectType_00495298[obj] == WC2_OBJECT_TYPE_PORCUPINE_MINE &&
        g_asObjectScreenX_00493598[obj] != (short)0x8001 &&
        (unsigned short)g_asObjectDistance_00493ae8[obj] > 1500 &&
        real_velocity(obj) < 20)
        approach(obj);
}

/* Function start: 0x418426 */
void match_ship_to_eye(void)
{
    g_aShipPosition_00494550[0] =
        g_aShipPosition_00494550[WC2_EYE_OBJECT];
    g_nHazardReferenceSpeed_00492e58 = 100;
    g_aShipRightVector_00493b78[0] =
        g_aShipRightVector_00493b78[WC2_EYE_OBJECT];
    g_aShipUpVector_00493ec0[0] =
        g_aShipUpVector_00493ec0[WC2_EYE_OBJECT];
    g_aShipForwardVector_00494208[0] =
        g_aShipForwardVector_00494208[WC2_EYE_OBJECT];
    ScaleFixedVector(&g_aShipForwardVector_00494208[0], 100 << 8,
                     &g_aShipVelocity_00494898[0]);
    g_pActiveHazardField_00493278->center =
        g_aShipPosition_00494550[WC2_EYE_OBJECT];
}

/* Function start: 0x4184F6 */
void update_hazards(void)
{
    short emptySlot;
    short slot;

    emptySlot = -1;

    if (g_bUseEyePositionForHazards_0049327c != 0)
        match_ship_to_eye();
    else
        g_nHazardReferenceSpeed_00492e58 = real_velocity(0);
    for (slot = 0; slot < 20; slot++) {
        if (g_abHazardObjects_00493280[slot] != -1)
            manage_hazard((short)g_abHazardObjects_00493280[slot], slot);
        else
            emptySlot = slot;
    }
    if (emptySlot != -1 &&
        RandomBelowOrEqual(215) <
            (short)g_nHazardReferenceSpeed_00492e58 + 30)
        g_abHazardObjects_00493280[emptySlot] = (signed char)make_hazard();
}

/* Function start: 0x4185CC */
void start_hazard_field(short region)
{
    short slot;

    slot = 0;
    remove_all_hazards();
    g_pActiveHazardField_00493278 = &g_aHazardFields_004931d8[region];
    while (slot++ < 3)
        g_abHazardObjects_00493280[slot] = (signed char)make_hazard();
}

/* Function start: 0x418626 */
void add_hazard_field(enum ObjectType type, FixedVector center,
                      short radius, short density)
{
    HazardField *field;

    if (g_nHazardFieldCount_004931d0 >= 7)
        return;
    field = &g_aHazardFields_004931d8[g_nHazardFieldCount_004931d0];
    field->type = type;
    field->center = center;
    field->outerRadius = radius;
    field->innerRadius = radius;
    field->density = density;
    g_nHazardFieldCount_004931d0++;
}

/* Function start: 0x4186A4 */
void check_hazards(void)
{
    HazardField *field;
    short region;

    if (g_bUseEyePositionForHazards_0049327c != 0)
        return;
    if (g_pActiveHazardField_00493278 == 0) {
        field = g_aHazardFields_004931d8;
        for (region = 0; g_nHazardFieldCount_004931d0 > region;
             region++, field++) {
            if (field != g_pActiveHazardField_00493278 &&
                near_field(field, &g_aShipPosition_00494550[0]) != 0) {
                start_hazard_field(region);
                return;
            }
        }
    } else if (near_field(g_pActiveHazardField_00493278,
                          &g_aShipPosition_00494550[0]) == 0) {
        remove_all_hazards();
    }
}

/* Function start: 0x4199F0 */
void SetPersonnelMousePosition(short x, short y)
{
    g_nPersonnelMouseX_005c8d00 = x;
    g_nPersonnelMouseY_005c8d02 = y;
    g_nQueuedInputX_005c83f0 = x;
    g_nQueuedInputY_005c83f2 = y;
    SetMouseHomePosition(x, y);
}

#pragma function(strcmp)

/* Function start: 0x453C95 */
void CheckLauncherAndConfig(void)
{
#ifdef WC1_SDL
    FILE *config;
    char option[100];
    char resolvedPath[PATH_MAX];

    if (ReadCheaterFlagFromRegistry() != 0) {
        *(unsigned char *)&g_nOriginDevUnlock_0049d774 = 1;
        *(unsigned char *)&g_bPlayerDamageEnabled_0049d77c = 0;
        *(unsigned char *)&g_bPlayerCollisionEnabled_0049d780 = 0;
    }

    if (Wc1SdlResolvePath("WINGCMDR.CFG", resolvedPath,
                          sizeof(resolvedPath)))
        config = fopen(resolvedPath, "rt");
    else
        config = 0;
    if (config != 0) {
        while (fscanf(config, "%s", option) != EOF) {
            char command;

            if (memcmp(option, "$#SAGA.EXE", 11) == 0)
                g_bApplicationControllerActive_0049c25c = 1;
            command = option[0] == '-' ? option[1] : option[0];
            switch (command) {
            case 'b':
                *(unsigned char *)&g_bPlayerCollisionEnabled_0049d780 = 0;
                break;
            case 'c':
                g_bCockpitEnabled_0049c26c = 0;
                break;
            case 'f':
                g_bShowFrameRate_0049c260 = 1;
                break;
            case 'k':
                *(unsigned char *)&g_bPlayerDamageEnabled_0049d77c = 0;
                break;
            case 'q':
                g_bConfigQuickModeEnabled_0049c264 = 0;
                break;
            }
            if (config == 0)
                return;
        }
        fclose(config);
    }
#else
    int scanResult;
    char option[100];
    int controlFile;
    char command;
    FILE *config;

    if (ReadCheaterFlagFromRegistry() != 0) {
        *(unsigned char *)&g_bPlayerDamageEnabled_0049d77c = 0;
        *(unsigned char *)&g_bPlayerCollisionEnabled_0049d780 = 0;
        *(unsigned char *)&g_nOriginDevUnlock_0049d774 = 1;
    }

    controlFile = _open("ctrl.del", O_BINARY);
    if (controlFile != -1) {
        g_bDebugBreakEnabled_0049c238 = 1;
        _close(controlFile);
    }

    config = fopen("WC2.CFG", "rt");
    while (config != 0) {
        scanResult = fscanf(config, "%s", option);
        if (scanResult == -1) {
            fclose(config);
            break;
        }
        if (strcmp("[[log]]", option) == 0)
            g_nDebugFileLoggingEnabled_0049c2d8 = 1;

        command = option[0];
        if (command == '-')
            command = option[1];
        switch (command) {
        case 'b':
            *(unsigned char *)&g_bPlayerCollisionEnabled_0049d780 = 0;
            break;
        case 'k':
            *(unsigned char *)&g_bPlayerDamageEnabled_0049d77c = 0;
            break;
        case 's':
            break;
        case 'f':
            g_bShowFrameRate_0049c260 = 1;
            break;
        case 'q':
            g_bConfigQuickModeEnabled_0049c264 = 0;
            break;
        case 'l':
            g_bSkipCampaignVideo_0049c270 = 1;
            break;
        case 'w':
            g_bUseHardwarePalette_0049c268 = 0;
            break;
        case 'm':
            g_bConfigMemoryOption_0049c2e0 = 1;
            break;
        }
    }
#endif
}

#pragma intrinsic(strcmp)

#ifndef WC1_SDL

/* Function start: 0x453E60 */
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous,
                   LPSTR commandLine, int showCommand)
{
    MEMORYSTATUS memoryStatus;
    RECT clip;
    HANDLE process;

    if (commandLine != 0 && strchr(commandLine, 's') != 0)
        g_nAudioEnabled_0049c244 = 0;
    g_hSingleInstanceSemaphore_005d10e4 =
        CreateSemaphoreA(0, 0, 1, "Wing Commander 2");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxA(0,
                    "Only one instance of Wing Commander 2 for Windows95 may be running at a time",
                    "ATTENTION", MB_ICONERROR);
        exit(0);
    }

    memset(&memoryStatus, 0, sizeof(memoryStatus));
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatus(&memoryStatus);
    if (memoryStatus.dwTotalPhys < 0x800000) {
        MessageBoxA(0,
                    "You must have at leat 8 megs of memory available to play Wing Commander 2 for Windows95",
                    "ATTENTION", MB_ICONERROR);
        exit(0);
    }
    if (memoryStatus.dwTotalPageFile < 0x800000) {
        MessageBoxA(0,
                    "You must have at leat 8 megs of virtual memory available to play Wing Commander 2 for Window95",
                    "ATTENTION", MB_ICONERROR);
        exit(0);
    }

    if (waveOutGetNumDevs() == 0)
        g_nAudioEnabled_0049c244 = 0;
    CheckLauncherAndConfig();
    if (!PromptInsertCorrectCd()) {
        CloseHandle(g_hSingleInstanceSemaphore_005d10e4);
        return 0;
    }
    if (!CreateMainWindow(instance, previous, showCommand))
        return 0;

    g_bWindowInactive_0049c274 = 0;
    process = GetCurrentProcess();
    MonoDebug_install();
    SetPriorityClass(process, HIGH_PRIORITY_CLASS);
    InitializeAudioSystem(g_hMainWindow_005d10e0);
    InitializeAudioStreamer(g_hMainWindow_005d10e0);
    srand((unsigned int)time(0));
    InitGameClockRandomEpoch();
    CreateDebugOverlayConsole(instance, g_hMainWindow_005d10e0, 60, 20);
    g_dwGameStartTime_005d12b4 = (unsigned int)time(0);
    ShowCursor(FALSE);
    g_pfnInputPump_005c840c = 0;
    clip.left = 0;
    clip.top = 0;
    clip.right = 320;
    clip.bottom = 200;
    ClipCursor(&clip);

    _onexit((_onexit_t)AbortToDesktop);
    g_nInputClock_005c84a8 = 0;
    AllocateApplicationScratchBuffer();
    g_bApplicationControllerActive_0049c25c = 1;
    RunGameApplication(0, &g_pEmptyStartupArgumentVector_0049c470);
    g_bApplicationShutdownStarted_0049c23c = 1;
    ReleaseApplicationScratchBuffer();

    ClipCursor(0);
    ShowCursor(TRUE);
    g_dwGameExitTime_005d129c = (unsigned int)time(0);
    DestroyGlobalDebugOverlayConsole();
    ServiceAudioStream();
    DestroyWindow(g_hMainWindow_005d10e0);
    DIBunInstall();
    CloseHandle(g_hSingleInstanceSemaphore_005d10e4);
    return 1;
}

#endif

/* Function start: 0x454103 */
void ShutdownGameWindow(void)
{
    g_bApplicationShutdownStarted_0049c23c = 1;
    g_dwGameExitTime_005d129c = (unsigned int)time(0);
#ifdef WC1_SDL
    {
        SDL_Window *window;

        g_bMainWindowRunning_005d12ac = 0;
        if ((g_dwStreamerState_005c4c38 & 1) != 0)
            ix_streamer_destroy();
        ServiceAudioStream();
        DestroyGlobalDebugOverlayConsole();
        window = (SDL_Window *)g_hMainWindow_005d10e0;
        DIBunInstall();
        Wc1SdlShutdownJoysticks();
        if (window != 0)
            SDL_DestroyWindow(window);
        g_hMainWindow_005d10e0 = 0;
        SDL_Quit();
    }
#else
    ClipCursor(0);
    ShowCursor(TRUE);
    DestroyGlobalDebugOverlayConsole();
    DestroyWindow(g_hMainWindow_005d10e0);
    DIBunInstall();
    ClipCursor(0);
    ShowCursor(TRUE);
    {
        HANDLE process;

        process = GetCurrentProcess();
        SetPriorityClass(process, IDLE_PRIORITY_CLASS);
    }
    CloseHandle(g_hSingleInstanceSemaphore_005d10e4);
#endif
    exit(0);
}

/* Function start: 0x45418B */
void ShowNoticeMessageBox(const char *text)
{
    DIBpositionWindow();
    MessageBoxA(0, text, "NOTICE", 0x10);
    exit(0);
}

/* Function start: 0x45422D */
unsigned int AbortToDesktop(void)
{
#ifndef WC1_SDL
    HANDLE process;

    g_bApplicationShutdownStarted_0049c23c = 1;
    ClipCursor(0);
    ShowCursor(TRUE);
    process = GetCurrentProcess();
    SetPriorityClass(process, IDLE_PRIORITY_CLASS);
#endif
    sprintf(g_abMemoryUsageReport_005d1170,
            "Current: %i\nMax    : %i\nTotal : %i\n",
            g_dwGuardedAllocationBytes_0049c250,
            g_dwGuardedAllocationPeakBytes_0049c254,
            g_dwGuardedAllocationTotalBytes_0049c24c);
    OutputDebugStringA("Memory Info:\n");
    OutputDebugStringA(g_abMemoryUsageReport_005d1170);
#ifndef WC1_SDL
    CloseHandle(g_hSingleInstanceSemaphore_005d10e4);
#endif
    return 0;
}

#ifndef WC1_SDL

/* Function start: 0x4542B7 */
int CreateMainWindow(HINSTANCE instance, HINSTANCE previous,
                     int showCommand)
{
    WNDCLASSA windowClass;

    DAT_005d12b0 = instance;
    if (previous != 0)
        return 0;

    memset(&windowClass, 0, sizeof(windowClass));
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = MainWindowProc;
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadIconA(0, IDI_APPLICATION);
    windowClass.hCursor = 0;
    windowClass.hbrBackground = GetStockObject(BLACK_BRUSH);
    windowClass.lpszMenuName = "Wing Commander";
    windowClass.lpszClassName = "Wing Commander";
    if (RegisterClassA(&windowClass) == 0)
        return 0;

    g_hMainWindow_005d10e0 = CreateWindowExA(0, "Wing Commander", "Wing Commander",
                                   WS_POPUP, 0, 0, 320, 200, 0, 0,
                                   DAT_005d12b0, 0);
    if (g_hMainWindow_005d10e0 == 0) {
        GetLastError();
        return 0;
    }

    DAT_005d1278 = GetDC(g_hMainWindow_005d10e0);
    SetTextColor(DAT_005d1278, RGB(255, 0, 0));
    SetBkColor(DAT_005d1278, RGB(0, 0, 0));
    if (GetDeviceCaps(DAT_005d1278, BITSPIXEL) < 8) {
        MessageBoxA(0,
                    "You must be running with 256, or more, colors to play Wing Commander",
                    "NOTICE", MB_ICONEXCLAMATION);
        DestroyWindow(g_hMainWindow_005d10e0);
        return 0;
    }

    DAT_005a8a34 = SetCursor(0);
    ShowWindow(g_hMainWindow_005d10e0, showCommand);
    UpdateWindow(g_hMainWindow_005d10e0);
    PumpWindowMessages(0);
    PumpWindowMessages(0);
    PumpWindowMessages(0);
    DIBinstall(g_hMainWindow_005d10e0);
    g_bMainWindowRunning_005d12ac = 1;
    return 1;
}

#endif

/* Function start: 0x45445A */
unsigned int PumpWindowMessages(int skipMessages)
{
    MSG message;
    int done;

    done = 0;
    g_nInputClock_005c84a8 = GetTickCount();
    g_nInputClock_005c84a8 -= g_dwGameClockStart_005d12b8;
    g_nInputClock_005c84a8 *= 60;
    g_nInputClock_005c84a8 /= 1000;
    if (skipMessages == 0) {
        if (g_bWindowMessagePumpActive_0049c2e4 != 0)
            return 1;
        g_bWindowMessagePumpActive_0049c2e4 = 1;
        if (g_pfnInputPump_005c840c != 0)
            g_pfnInputPump_005c840c();
        KeyboardMousePump();
#ifdef WC1_SDL
        Wc1SdlPumpEvents();
#else
        done = 0;
        while (done == 0 || g_bWindowInactive_0049c274 != 0) {
            if (g_bWindowInactive_0049c274 != 0) {
                if (GetMessageA(&message, 0, 0, 0) != 0) {
                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                    done = 1;
                } else {
                    LogMemoryUsage();
                    ShutdownGameWindow();
                }
            } else {
                if (PeekMessageA(&message, 0, 0, 0,
                                 PM_NOREMOVE) != 0) {
                    if (GetMessageA(&message, 0, 0, 0) != 0) {
                        TranslateMessage(&message);
                        DispatchMessageA(&message);
                    } else {
                        LogMemoryUsage();
                        ShutdownGameWindow();
                    }
                    done = 0;
                } else {
                    done = 1;
                }
            }
        }
#endif
        g_bWindowMessagePumpActive_0049c2e4 = 0;
    }
    g_nInputClock_005c84a8 = GetTickCount();
    g_nInputClock_005c84a8 -= g_dwGameClockStart_005d12b8;
    g_nInputClock_005c84a8 *= 60;
    g_nInputClock_005c84a8 /= 1000;
    return g_bMainWindowRunning_005d12ac;
}

/* Function start: 0x453C80 */
unsigned int GetF1KeyLatch(void)
{
    return g_bF1KeyDown_0049c240;
}

#ifndef WC1_SDL

/* Function start: 0x454625 */
LRESULT CALLBACK MainWindowProc(HWND window, UINT message,
                                WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paint;
    int mouseEvent;
    unsigned int command;
    unsigned int primaryButton;
    unsigned int secondaryButton;
    unsigned int mouseX;
    unsigned int mouseY;
    unsigned int scanCode;
    MSG characterMessage;
    unsigned int character;
    HANDLE process;
    RECT clip;

    command = (unsigned int)wParam & 0xffff;
    mouseEvent = 0;
    primaryButton = ((unsigned int)wParam & 1) != 0;
    secondaryButton = ((unsigned int)wParam & 2) != 0;
    if (message == WM_MOUSEMOVE || message == WM_RBUTTONUP ||
        message == WM_LBUTTONUP || message == WM_LBUTTONDOWN ||
        message == WM_RBUTTONDOWN) {
        g_nPendingMouseX_0049c2ec = LOWORD(lParam);
        g_nPendingMouseY_0049c2f0 = HIWORD(lParam);
    }
    mouseX = g_nPendingMouseX_0049c2ec;
    mouseY = g_nPendingMouseY_0049c2f0;

    switch (message) {
    case WM_ACTIVATEAPP:
        if (g_bApplicationControllerActive_0049c25c != 0) {
            if (wParam != 0) {
                if (g_bWindowInactive_0049c274 == 1) {
                    clip.left = 0;
                    clip.top = 0;
                    clip.right = 320;
                    clip.bottom = 200;
                    ShowCursor(FALSE);
                    process = GetCurrentProcess();
                    SetPriorityClass(process, HIGH_PRIORITY_CLASS);
                    SetActiveWindow(g_hMainWindow_005d10e0);
                    SetForegroundWindow(g_hMainWindow_005d10e0);
                    DIBreInstall();
                    ClipCursor(&clip);
                    SetCursorPos(g_nSavedWindowCursorX_005d12a0,
                                 g_nSavedWindowCursorY_005d12a4);
                    g_bWindowInactive_0049c274 = 0;
                    WriteDebugString("UNPAUSING...\n");
                }
            } else if (g_bWindowInactive_0049c274 == 0 &&
                       g_bApplicationShutdownStarted_0049c23c == 0) {
                g_nSavedWindowCursorX_005d12a0 = 160;
                g_nSavedWindowCursorY_005d12a4 = 100;
                ClipCursor(0);
                ShowCursor(TRUE);
                process = GetCurrentProcess();
                SetPriorityClass(process, IDLE_PRIORITY_CLASS);
                g_bWindowInactive_0049c274 = 1;
                WriteDebugString("PAUSING...\n");
            }
            return 0;
        }
        /* fall through */
    case WM_ACTIVATE:
        if (g_bApplicationControllerActive_0049c25c != 0) {
            if (((unsigned int)wParam & 0xffff) != 0) {
                if (((unsigned int)wParam >> 16) != 0) {
                    g_bWindowInactive_0049c274 = 1;
                    WriteDebugString("MINIMIZED PAUSING...\n");
                    return 0;
                }
                if (g_bWindowInactive_0049c274 == 1) {
                    clip.left = 0;
                    clip.top = 0;
                    clip.right = 320;
                    clip.bottom = 200;
                    ShowCursor(FALSE);
                    process = GetCurrentProcess();
                    SetPriorityClass(process, HIGH_PRIORITY_CLASS);
                    SetActiveWindow(g_hMainWindow_005d10e0);
                    SetForegroundWindow(g_hMainWindow_005d10e0);
                    DIBreInstall();
                    ClipCursor(&clip);
                    SetCursorPos(g_nSavedWindowCursorX_005d12a0,
                                 g_nSavedWindowCursorY_005d12a4);
                    g_bWindowInactive_0049c274 = 0;
                    WriteDebugString("UNPAUSING...\n");
                }
            } else if (g_bWindowInactive_0049c274 == 0 &&
                       g_bApplicationShutdownStarted_0049c23c == 0) {
                g_nSavedWindowCursorX_005d12a0 = 160;
                g_nSavedWindowCursorY_005d12a4 = 100;
                ClipCursor(0);
                ShowCursor(TRUE);
                process = GetCurrentProcess();
                SetPriorityClass(process, IDLE_PRIORITY_CLASS);
                g_bWindowInactive_0049c274 = 1;
                WriteDebugString("PAUSING...\n");
            }
            return 0;
        }
        /* fall through */
    case WM_SETFOCUS:
        SignalAudioMixerWakeEvent();
        return 0;
    case WM_COMMAND:
        if (command == 3) {
            g_bMainWindowRunning_005d12ac = 0;
            PostQuitMessage(0);
        }
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        g_nInputPressCount_0049c258++;
        QueueInputEvent(1, (unsigned short)mouseX,
                        (unsigned short)mouseY, 0,
                        primaryButton, secondaryButton, 0, 0, 0);
        mouseEvent = 1;
        break;
    case WM_LBUTTONUP:
        g_nInputPressCount_0049c258--;
        QueueInputEvent(2, (unsigned short)mouseX,
                        (unsigned short)mouseY, 0, 1, 0, 0, 0, 0);
        mouseEvent = 1;
        break;
    case WM_RBUTTONUP:
        g_nInputPressCount_0049c258--;
        QueueInputEvent(2, (unsigned short)mouseX,
                        (unsigned short)mouseY, 0, 0, 1, 0, 0, 0);
        mouseEvent = 1;
        break;
    case WM_MOUSEMOVE:
        if (g_bSuppressNextMouseMove_005c843c == 0) {
            QueueInputEvent(3, (unsigned short)mouseX,
                            (unsigned short)mouseY, 0,
                            primaryButton, secondaryButton, 0, 0, 0);
            mouseEvent = 1;
        } else {
            g_bSuppressNextMouseMove_005c843c = 0;
        }
        break;
    case WM_KEYDOWN:
        if (PeekMessageA(&characterMessage, window, WM_CHAR, WM_CHAR,
                         PM_REMOVE) != 0) {
            character = (unsigned int)characterMessage.wParam;
        } else {
            character = 0;
        }
        if (wParam == VK_F1) {
            if (((unsigned long)lParam & 0x40000000) == 0)
                g_bF1KeyDown_0049c240 = 1;
            else
                g_bF1KeyDown_0049c240 = 0;
        }
        scanCode = ((unsigned long)lParam & 0xff0000) >> 16;
        if (scanCode == 1)
            g_bSceneEscapeRequested_0049d4b0 = 1;
        if (((unsigned long)lParam & 0x40000000) == 0 &&
            scanCode != 1) {
            g_nInputPressCount_0049c258++;
        }
        QueueInputEvent(4, (unsigned short)mouseX,
                        (unsigned short)mouseY, (unsigned short)wParam,
                        0, 0, 0, (unsigned short)scanCode,
                        (unsigned short)character);
        SetInputKeyState((int)scanCode, 1);
        break;
    case WM_KEYUP:
        if (PeekMessageA(&characterMessage, window, WM_CHAR, WM_CHAR,
                         PM_REMOVE) != 0) {
            character = (unsigned int)characterMessage.wParam;
        } else {
            character = 0;
        }
        if (wParam == VK_F1)
            g_bF1KeyDown_0049c240 = 0;
        scanCode = ((unsigned long)lParam & 0xff0000) >> 16;
        if (scanCode != 0x81) {
            g_nInputPressCount_0049c258--;
            if (g_nInputPressCount_0049c258 < 0)
                g_nInputPressCount_0049c258 = 0;
        }
        QueueInputEvent(5, (unsigned short)mouseX,
                        (unsigned short)mouseY, (unsigned short)wParam,
                        0, 0, 0, (unsigned short)scanCode,
                        (unsigned short)character);
        SetInputKeyState((int)scanCode, 0);
        break;
    case WM_SYSKEYUP:
        g_bAltAHotkey_005d1294 = 0;
        g_bAltBHotkey_005d1290 = 0;
        g_bJoystickCalibrationHotkey_005d1284 = 0;
        g_bAltTHotkey_005d1298 = 0;
        g_bAltHHotkey_005d128c = 0;
        g_bAltFHotkey_005d127c = 0;
        g_bAltDHotkey_005d1280 = 0;
        g_nLastAltCommandScanCode_005d1274 = 0;
        g_bAltNumpadAddHotkey_005d1270 = 0;
        g_bAltNumpadSubtractHotkey_005d12a8 = 0;
        g_dwSystemKey_005d10a4 = 0;
        break;
    case WM_SYSKEYDOWN:
        g_dwSystemKey_005d10a4 = (unsigned int)wParam;
        switch (wParam) {
        case 'X':
            g_bApplicationShutdownStarted_0049c23c = 1;
            if (((unsigned long)lParam & 0x20000000) != 0)
                PostQuitMessage(0);
            break;
        case 'N':
            ReportSpaceFlightMaxFps(-0.5f);
            break;
        case 'M':
            ReportSpaceFlightMaxFps(0.5f);
            break;
        case 'A':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_bAltAHotkey_005d1294 = 1;
            break;
        case 'B':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_bAltBHotkey_005d1290 = 1;
            break;
        case 'C':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_bJoystickCalibrationHotkey_005d1284 = 1;
            break;
        case 'T':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_bAltTHotkey_005d1298 = 1;
            break;
        case 'H':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_bAltHHotkey_005d128c = 1;
            break;
        case 'F':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_bAltFHotkey_005d127c = 1;
            break;
        case 'D':
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_bAltDHotkey_005d1280 = 1;
            break;
        case VK_ADD:
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_bAltNumpadAddHotkey_005d1270 = 1;
            break;
        case VK_SUBTRACT:
            if (((unsigned long)lParam & 0x20000000) != 0)
                g_bAltNumpadSubtractHotkey_005d12a8 = 1;
            break;
        case 'Z':
            if (((unsigned long)lParam & 0x20000000) != 0 &&
                g_bDebugBreakEnabled_0049c238 != 0) {
                DebugBreak();
            }
            break;
        }
        break;
    case WM_PAINT:
        BeginPaint(g_hMainWindow_005d10e0, &paint);
        EndPaint(g_hMainWindow_005d10e0, &paint);
        break;
    case WM_CLOSE:
    case WM_DESTROY:
        g_bMainWindowRunning_005d12ac = 0;
        ClipCursor(0);
        ShowCursor(TRUE);
        process = GetCurrentProcess();
        SetPriorityClass(process, IDLE_PRIORITY_CLASS);
        PostQuitMessage(0);
        break;
    case WM_QUIT:
        g_bMainWindowRunning_005d12ac = 0;
        ClipCursor(0);
        ShowCursor(TRUE);
        process = GetCurrentProcess();
        SetPriorityClass(process, IDLE_PRIORITY_CLASS);
        g_bApplicationShutdownStarted_0049c23c = 1;
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_SCREENSAVE ||
            (wParam & 0xfff0) == SC_MONITORPOWER)
            return 0;
        break;
    }
    if (mouseEvent != 0) {
        g_stHostMouseMessage_005d10d0.x = mouseX;
        g_stHostMouseMessage_005d10d0.y = mouseY;
        g_stHostMouseMessage_005d10d0.primaryButton = primaryButton;
        g_stHostMouseMessage_005d10d0.secondaryButton = secondaryButton;
    }
    return DefWindowProcA(window, message, wParam, lParam);
}

#endif

/* Function start: 0x4551E7 */
void GetJoystickPosition(unsigned int *x, unsigned int *y,
                         unsigned int *buttons, short joystick,
                         unsigned int fallback)
{
    unsigned int infoIndex;
    unsigned int device;
    JOYINFO sample;

    infoIndex = 0;
    if (joystick != 0) {
        device = 1;
        infoIndex = 1;
    } else {
        device = 0;
        infoIndex = 0;
    }
#ifdef WC1_SDL
    if (Wc1SdlReadJoystick(
            device, &g_aJoystickInfo_005d10b0[infoIndex]) != FALSE) {
#else
    if (joyGetPos(device, &g_aJoystickInfo_005d10b0[infoIndex]) ==
        JOYERR_NOERROR) {
#endif
        *x = g_aJoystickInfo_005d10b0[infoIndex].wXpos;
        *y = g_aJoystickInfo_005d10b0[infoIndex].wYpos;
        *buttons = g_aJoystickInfo_005d10b0[infoIndex].wButtons;
        sample = g_aJoystickInfo_005d10b0[infoIndex];
        if ((sample.wButtons & 1) != 0) {
            if (g_bJoystickPrimaryButtonLatched_0049c2f8 == 0)
                g_nInputPressCount_0049c258++;
            g_bJoystickPrimaryButtonLatched_0049c2f8 = 1;
        } else {
            g_bJoystickPrimaryButtonLatched_0049c2f8 = 0;
        }
        if ((sample.wButtons & 2) != 0) {
            if (g_bJoystickSecondaryButtonLatched_0049c2fc == 0)
                g_nInputPressCount_0049c258++;
            g_bJoystickSecondaryButtonLatched_0049c2fc = 1;
        } else {
            g_bJoystickSecondaryButtonLatched_0049c2fc = 0;
        }
        if (joystick != 0)
            *buttons >>= 2;
        else
            *buttons &= 3;
        return;
    }

    *x = fallback;
    *y = fallback;
    *buttons = fallback;
}

/* Function start: 0x455346 */
short GetJoystickButtons(void)
{
    short buttons;

    buttons = (g_aJoystickInfo_005d10b0[1].wButtons << 2) |
              g_aJoystickInfo_005d10b0[0].wButtons;
    return buttons;
}

/* Function start: 0x45536F */
void GetJoystickDevCaps(short joystick, short *xMin, short *xMax,
                        short *yMin, short *yMax)
{
#ifdef WC1_SDL
    unsigned int hostXMin;
    unsigned int hostXMax;
    unsigned int hostYMin;
    unsigned int hostYMax;
#else
    JOYCAPSA caps;
#endif
    unsigned int device = joystick != 0;

    *xMin = *xMax = *yMin = *yMax = 0;
#ifdef WC1_SDL
    if (Wc1SdlReadJoystickAxisRange(
            device, &hostXMin, &hostXMax, &hostYMin, &hostYMax) == FALSE) {
#else
    if (joyGetDevCapsA(device, &caps, sizeof(caps)) != JOYERR_NOERROR) {
#endif
        return;
    }

#ifdef WC1_SDL
    *xMin = (short)hostXMin;
    *xMax = (short)hostXMax;
    *yMin = (short)hostYMin;
    *yMax = (short)hostYMax;
#else
    *xMin = (short)caps.wXmin;
    *xMax = (short)caps.wXmax;
    *yMin = (short)caps.wYmin;
    *yMax = (short)caps.wYmax;
#endif
}

/* Function start: 0x455427 */
HINSTANCE GetApplicationInstance(void)
{
    return DAT_005d12b0;
}

/* Function start: 0x45543C */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
HWND GetMainWindowHandle(void)
{
    return g_hMainWindow_005d10e0;
}

/* Function start: 0x455451 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
HDC GetMainWindowDeviceContext(void)
{
    return DAT_005d1278;
}

/* Function start: 0x455466 */
void *AllocateGuardedMemory(unsigned int size)
{
    GuardedAllocation *allocation;

    if (g_pGuardedAllocationHead_0049c300 == 0) {
        allocation =
            malloc(sizeof(GuardedAllocation));
        g_pGuardedAllocationHead_0049c300 = allocation;
    } else {
        g_pGuardedAllocationTail_005d10ec->next =
            malloc(sizeof(GuardedAllocation));
        allocation = g_pGuardedAllocationTail_005d10ec->next;
    }
    g_pGuardedAllocationTail_005d10ec = allocation;
    g_pGuardedAllocationTail_005d10ec->next = 0;
    g_pGuardedAllocationTail_005d10ec->size = size;
    g_pGuardedAllocationTail_005d10ec->block = malloc(size + 0x800);

    memset(g_pGuardedAllocationTail_005d10ec->block, 0xab, 0x400);
    memset((unsigned char *)g_pGuardedAllocationTail_005d10ec->block +
               0x400,
           0, size);
    memset((unsigned char *)g_pGuardedAllocationTail_005d10ec->block +
               0x400 + size,
           0xab, 0x400);
    g_dwGuardedAllocationTotalBytes_0049c24c += size;
    g_dwGuardedAllocationBytes_0049c250 += size;
    if (g_dwGuardedAllocationPeakBytes_0049c254 <
        g_dwGuardedAllocationBytes_0049c250) {
        g_dwGuardedAllocationPeakBytes_0049c254 =
            g_dwGuardedAllocationBytes_0049c250;
    }
    return (unsigned char *)g_pGuardedAllocationTail_005d10ec->block +
           0x400;
}

/* Function start: 0x455565 */
void ReportHeapGuardCorruption(void *memory, int count, int overrun)
{
    char text[0x80];

    DIBpositionWindow();
    sprintf(text, "Memory at %p %swritten. Corruption : (%i/%i) %s",
            memory, overrun ? "over" : "under", count, 0x100,
            count > 0x80
                ? (count > 0xc0 ? "EXTREME" : "SEVERE")
                : (count > 0x40 ? "BAD" : "NAUGHTY"));
    MessageBoxA(0, text, "NOTICE", 0x10);
    exit(0);
}

/* Function start: 0x455624 */
void CheckAllGuardedAllocations(void)
{
}

/* Function start: 0x45562F */
int IsFreedHeapBlockTracked(void *memory)
{
    FreedHeapBlock *block;

    block = g_pFreedHeapBlockHead_0049c304;
    while (block != 0) {
        if (block->block == memory)
            return 1;
        if (block->block == (unsigned char *)memory - 8)
            return 1;
        block = block->next;
    }
    return 0;
}

/* Function start: 0x455697 */
int DetectDoubleFree(void *memory)
{
    FreedHeapBlock *block;

    block = g_pFreedHeapBlockHead_0049c304;
    while (block != 0) {
        if (block->block == memory) {
            OutputDebugStringA("DoubleFree Detected\n");
            return 1;
        }
        if (block->block == (unsigned char *)memory - 8) {
            OutputDebugStringA("DoubleFree (pushed) Detected\n");
            return 1;
        }
        block = block->next;
    }
    return 0;
}

/* Function start: 0x455715 */
void TrackFreedHeapBlock(void *memory)
{
    if (g_pFreedHeapBlockHead_0049c304 == 0) {
        g_pFreedHeapBlockHead_0049c304 = malloc(sizeof(FreedHeapBlock));
        g_pFreedHeapBlockTail_0049c308 = g_pFreedHeapBlockHead_0049c304;
    } else {
        g_pFreedHeapBlockTail_0049c308->next = malloc(sizeof(FreedHeapBlock));
        g_pFreedHeapBlockTail_0049c308 = g_pFreedHeapBlockTail_0049c308->next;
    }
    g_pFreedHeapBlockTail_0049c308->next = 0;
    g_pFreedHeapBlockTail_0049c308->block = memory;
}

/* Function start: 0x455882 */
void FreeGuardedAllocation(void *memory)
{
    int found;
#ifdef WC1_SDL
    unsigned char *guard;
    unsigned int guardValue;
#else
    unsigned int *guard;
#endif
    GuardedAllocation *previous;
    GuardedAllocation *allocation;
    int corrupt;
    int i;

    found = 0;
#ifdef WC1_SDL
    guard = (unsigned char *)memory - 0x400;
#else
    guard = (unsigned int *)((unsigned char *)memory - 0x400);
#endif
    previous = 0;
    allocation = g_pGuardedAllocationHead_0049c300;
    while (allocation != 0) {
        if (allocation->block == guard) {
            corrupt = 0;
            for (i = 0; i < 0x100; i++) {
#ifdef WC1_SDL
                memcpy(&guardValue, guard, sizeof(guardValue));
                if (guardValue != 0xabababab)
                    corrupt++;
                guard += sizeof(guardValue);
#else
                if (*guard != 0xabababab)
                    corrupt++;
                guard++;
#endif
            }
            if (corrupt != 0)
                ReportHeapGuardCorruption(memory, corrupt, 0);

            corrupt = 0;
#ifdef WC1_SDL
            guard = (unsigned char *)memory + allocation->size;
#else
            guard =
                (unsigned int *)((unsigned char *)memory + allocation->size);
#endif
            for (i = 0; i < 0x100; i++) {
#ifdef WC1_SDL
                memcpy(&guardValue, guard, sizeof(guardValue));
                if (guardValue != 0xabababab)
                    corrupt++;
                guard += sizeof(guardValue);
#else
                if (*guard != 0xabababab)
                    corrupt++;
                guard++;
#endif
            }
            if (corrupt != 0)
                ReportHeapGuardCorruption(memory, corrupt, 1);

            guard = (unsigned int *)((unsigned char *)memory - 0x400);
            TrackFreedHeapBlock(guard);
            free(guard);
            if (previous != 0)
                previous->next = allocation->next;
            else
                g_pGuardedAllocationHead_0049c300 = allocation->next;
            if (g_pGuardedAllocationTail_005d10ec == allocation) {
                if (previous != 0)
                    g_pGuardedAllocationTail_005d10ec = previous;
                else
                    g_pGuardedAllocationTail_005d10ec = 0;
            }
            free(allocation);
            allocation = 0;
            found = 1;
        }
        if (allocation != 0) {
            previous = allocation;
            allocation = allocation->next;
        }
    }

    guard = (unsigned int *)((unsigned char *)memory - 0x400);
    if (found == 0 && DetectDoubleFree(guard) == 0) {
        sprintf(g_szGuardedHeapDebugMessage_005d10f0,
                "MEM: Unknown memory lost (%p)\n", guard);
        WriteDebugString(g_szGuardedHeapDebugMessage_005d10f0);
    }
}
