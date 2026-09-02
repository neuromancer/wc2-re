#include "game.h"

#include <string.h>

#define TEST_SPRITE_BYTES 45
#define TEST_SPRITE_HEADER_BYTES (8 + sizeof(unsigned char *))
#define TEST_SPRITE_STORAGE_POINTERS                                      \
    ((TEST_SPRITE_HEADER_BYTES + TEST_SPRITE_BYTES + sizeof(void *) - 1) / \
     sizeof(void *))

static unsigned char *InitializeSolidTestSprite(void *storage,
                                                 unsigned char colour)
{
    unsigned char *cursor;
    unsigned char *shape;
    int frameOffset;
    int signature;
    int row;
    short extent;
    short rowCode;
    short xOffset;
    short yOffset;

    memset(storage, 0, TEST_SPRITE_STORAGE_POINTERS * sizeof(void *));
    shape = (unsigned char *)storage + TEST_SPRITE_HEADER_BYTES;
    signature = 0x6666656a;
    memcpy(shape - 8, &signature, sizeof(signature));
    frameOffset = 8;
    memcpy(shape + 4, &frameOffset, sizeof(frameOffset));
    extent = 1;
    memcpy(shape + 8, &extent, sizeof(extent));
    memcpy(shape + 10, &extent, sizeof(extent));
    memcpy(shape + 12, &extent, sizeof(extent));
    memcpy(shape + 14, &extent, sizeof(extent));

    cursor = shape + 16;
    rowCode = 6;
    xOffset = -1;
    for (row = -1; row <= 1; row++) {
        yOffset = (short)row;
        memcpy(cursor, &rowCode, sizeof(rowCode));
        cursor += sizeof(rowCode);
        memcpy(cursor, &xOffset, sizeof(xOffset));
        cursor += sizeof(xOffset);
        memcpy(cursor, &yOffset, sizeof(yOffset));
        cursor += sizeof(yOffset);
        *cursor++ = colour;
        *cursor++ = colour;
        *cursor++ = colour;
    }
    rowCode = 0;
    memcpy(cursor, &rowCode, sizeof(rowCode));
    return shape;
}

static void ReleasePreparedTestSprite(unsigned char *shape)
{
    unsigned char *preparedShape;

    if (shape == 0)
        return;
    preparedShape = GetPreparedShapeData(shape);
    if (preparedShape != 0)
        ReleasePacketHandle(preparedShape);
}

static int PushKeyboardEvent(SDL_Window *window, Uint32 type,
                             SDL_Scancode scanCode, SDL_Keycode key,
                             SDL_Keymod modifiers, Uint8 repeat)
{
    SDL_Event event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key.windowID = SDL_GetWindowID(window);
    event.key.state = type == SDL_KEYDOWN ? SDL_PRESSED : SDL_RELEASED;
    event.key.repeat = repeat;
    event.key.keysym.scancode = scanCode;
    event.key.keysym.sym = key;
    event.key.keysym.mod = modifiers;
    return SDL_PushEvent(&event) == 1;
}

typedef struct TestRadioHotkey {
    SDL_Scancode scanCode;
    SDL_Keycode key;
    int *flag;
} TestRadioHotkey;

static const TestRadioHotkey g_aTestRadioHotkeys[] = {
    {SDL_SCANCODE_B, SDLK_b, &g_bAltBHotkey_005d1290},
    {SDL_SCANCODE_F, SDLK_f, &g_bAltFHotkey_005d127c},
    {SDL_SCANCODE_A, SDLK_a, &g_bAltAHotkey_005d1294},
    {SDL_SCANCODE_H, SDLK_h, &g_bAltHHotkey_005d128c},
    {SDL_SCANCODE_D, SDLK_d, &g_bAltDHotkey_005d1280},
    {SDL_SCANCODE_T, SDLK_t, &g_bAltTHotkey_005d1298}
};

int main(int argumentCount, char **arguments)
{
    InputEventState input;
    JOYINFO joystickInformation;
    SDL_Event event;
    SDL_JoystickID instanceId;
    SDL_Window *window;
    unsigned char *constellationShape;
    unsigned char *planetShape;
    unsigned char pixels[256];
    unsigned short rowOffsets[17];
    void *constellationStorage[TEST_SPRITE_STORAGE_POINTERS];
    void *planetStorage[TEST_SPRITE_STORAGE_POINTERS];
    int activeSlot;
    int colouredPixelCount;
    int deviceIndex;
    int eventType;
    int index;
    int planetPixelCount;
    int result;
    int stage;

    (void)argumentCount;
    (void)arguments;
    result = 1;
    stage = 1;
    window = 0;
    deviceIndex = -1;
    constellationShape = 0;
    planetShape = 0;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK |
                 SDL_INIT_GAMECONTROLLER) != 0)
        return 1;
    window = SDL_CreateWindow("SDL event test", 0, 0, 640, 400, 0);
    if (window == 0)
        goto cleanup;
    g_hMainWindow_005d10e0 = (HWND)window;

    stage = 2;
    SdlSetMouseGrab(1);
    stage = 20;
    if (SDL_GetWindowMouseGrab(window) != SDL_TRUE)
        goto cleanup;
    memset(&event, 0, sizeof(event));
    event.type = SDL_WINDOWEVENT;
    event.window.windowID = SDL_GetWindowID(window);
    event.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
    if (SDL_PushEvent(&event) != 1)
        goto cleanup;
    SdlPumpEvents();
    stage = 21;
    if (SDL_GetWindowMouseGrab(window) != SDL_FALSE)
        goto cleanup;
    event.window.event = SDL_WINDOWEVENT_FOCUS_GAINED;
    if (SDL_PushEvent(&event) != 1)
        goto cleanup;
    SdlPumpEvents();
    stage = 22;
    if (SDL_GetWindowMouseGrab(window) != SDL_TRUE)
        goto cleanup;
    SdlSuspendMouseGrab();
    SdlSuspendMouseGrab();
    stage = 23;
    if (SDL_GetWindowMouseGrab(window) != SDL_FALSE)
        goto cleanup;
    SdlResumeMouseGrab();
    stage = 24;
    if (SDL_GetWindowMouseGrab(window) != SDL_FALSE)
        goto cleanup;
    SdlResumeMouseGrab();
    stage = 25;
    if (SDL_GetWindowMouseGrab(window) != SDL_TRUE)
        goto cleanup;
    SdlSetMouseGrab(0);

    stage = 3;
    if (SdlTranslateScanCode(SDL_SCANCODE_HOME) != 0x47 ||
        SdlTranslateScanCode(SDL_SCANCODE_DELETE) != 0x53 ||
        SdlTranslateScanCode(SDL_SCANCODE_F12) != 0x58)
        goto cleanup;

    stage = 32;
    g_bSpaceFlightActive_005c586c = 1;
    g_nArcadeState_0049d75c = 0;
    g_nCannedSceneMode_0049021c = 0;
    g_bJumpSequenceActive_004962f0 = 0;
    g_pfnInputPump_005c840c = get_player_input;
    for (index = 0;
         index < (int)(sizeof(g_aTestRadioHotkeys) /
                       sizeof(g_aTestRadioHotkeys[0]));
         index++) {
        *g_aTestRadioHotkeys[index].flag = 0;
        if (!PushKeyboardEvent(window, SDL_KEYDOWN,
                               g_aTestRadioHotkeys[index].scanCode,
                               g_aTestRadioHotkeys[index].key,
                               KMOD_LALT, 0) ||
            !PushKeyboardEvent(window, SDL_KEYUP,
                               g_aTestRadioHotkeys[index].scanCode,
                               g_aTestRadioHotkeys[index].key,
                               KMOD_LALT, 0))
            goto cleanup;
    }
    SdlPumpEvents();
    for (index = 0;
         index < (int)(sizeof(g_aTestRadioHotkeys) /
                       sizeof(g_aTestRadioHotkeys[0]));
         index++) {
        if (*g_aTestRadioHotkeys[index].flag == 0)
            goto cleanup;
        *g_aTestRadioHotkeys[index].flag = 0;
    }

    stage = 33;
    g_bAltDHotkey_005d1280 = 0;
    g_bAltHHotkey_005d128c = 0;
    g_bAltTHotkey_005d1298 = 0;
    if (!PushKeyboardEvent(window, SDL_KEYDOWN, SDL_SCANCODE_D,
                           SDLK_d, KMOD_LALT, 1) ||
        !PushKeyboardEvent(window, SDL_KEYUP, SDL_SCANCODE_D,
                           SDLK_d, KMOD_LALT, 0) ||
        !PushKeyboardEvent(window, SDL_KEYDOWN, SDL_SCANCODE_H,
                           SDLK_h, KMOD_NONE, 0) ||
        !PushKeyboardEvent(window, SDL_KEYUP, SDL_SCANCODE_H,
                           SDLK_h, KMOD_NONE, 0))
        goto cleanup;
    g_bSpaceFlightActive_005c586c = 0;
    if (!PushKeyboardEvent(window, SDL_KEYDOWN, SDL_SCANCODE_T,
                           SDLK_t, KMOD_LALT, 0) ||
        !PushKeyboardEvent(window, SDL_KEYUP, SDL_SCANCODE_T,
                           SDLK_t, KMOD_LALT, 0))
        goto cleanup;
    SdlPumpEvents();
    if (g_bAltDHotkey_005d1280 != 0 ||
        g_bAltHHotkey_005d128c != 0 ||
        g_bAltTHotkey_005d1298 != 0)
        goto cleanup;
    g_bSpaceFlightActive_005c586c = 0;
    g_pfnInputPump_005c840c = 0;

    stage = 30;
    memset(pixels, 0, sizeof(pixels));
    for (index = 0; index < 17; index++)
        rowOffsets[index] = (unsigned short)(index * 16);
    g_stScreenViewport_005d21a0.pixels = pixels;
    g_stScreenViewport_005d21a0.rowOffsets = rowOffsets;
    g_stScreenViewport_005d21a0.left = 0;
    g_stScreenViewport_005d21a0.top = 0;
    g_stScreenViewport_005d21a0.right = 15;
    g_stScreenViewport_005d21a0.bottom = 15;
    SdlDrawViewportStatic(&g_stScreenViewport_005d21a0, 3, 0x5d);
    colouredPixelCount = 0;
    for (index = 0; index < 256; index++) {
        if (pixels[index] == 0x5d)
            colouredPixelCount++;
        else if (pixels[index] != 0)
            goto cleanup;
    }
    if (colouredPixelCount == 0)
        goto cleanup;

    /* Planets carry their own scale and sprite.  Treating one like a star
     * substitutes the constellation sheet, which made WC2's backgrounds
     * disappear and also erased the wrong footprint on the next frame. */
    stage = 31;
    planetShape = InitializeSolidTestSprite(planetStorage, 0x42);
    constellationShape =
        InitializeSolidTestSprite(constellationStorage, 0x17);
    memset(pixels, 0, sizeof(pixels));
    g_stViewBuffer_005d2b00 = g_stScreenViewport_005d21a0;
    g_nScreenWidth_0049d4d8 = 16;
    g_nViewCenterX_005c80d8 = 8;
    g_nViewCenterY_005c80da = 8;
    g_nNavPointerObject_004931b8 = -1;
    g_anSortedObject_005c82c0[0] = 0;
    g_anSortedObject_005c82c0[1] = -1;
    g_asObjectType_00495298[0] = OBJECT_DATA_LASER_CANNON;
    g_aeObjectClass_00495328[0] = OBJECT_CLASS_PLANET;
    g_asObjectScreenX_00493598[0] = 0;
    g_asObjectScreenY_00493628[0] = 0;
    g_asObjectViewFrame_00493508[0] = 0;
    g_asObjectScreenAngle_004936b8[0] = 0;
    g_asObjectScreenScale_00493a58[0] = 0x200;
    g_asObjectFlip_004939c8[0] = 0;
    g_aObjectViewPosition_0059afa0[0].z = 0;
    g_apObjectShape_00493868[0] = planetShape;
    g_pConstellationShape_005d2c4c = constellationShape;
    draw_sorted_objects_to_buffer();
    planetPixelCount = 0;
    for (index = 0; index < 256; index++) {
        if (pixels[index] == 0x42)
            planetPixelCount++;
        else if (pixels[index] == 0x17)
            goto cleanup;
    }
    if (planetPixelCount == 0)
        goto cleanup;
    g_cPrimaryViewBufferColour_0049cb88 = 9;
    intro_drawbackgroundships();
    for (index = 0; index < 256; index++) {
        if (pixels[index] == 0x42)
            goto cleanup;
    }

#if SDL_VERSION_ATLEAST(2, 0, 14)
    stage = 4;
    deviceIndex = SDL_JoystickAttachVirtual(
        SDL_JOYSTICK_TYPE_UNKNOWN, 4, 12, 1);
    if (deviceIndex < 0)
        goto cleanup;
    SDL_JoystickUpdate();
    memset(&joystickInformation, 0, sizeof(joystickInformation));
    if (SdlReadJoystick(0, &joystickInformation) == FALSE &&
        SdlReadJoystick(1, &joystickInformation) == FALSE)
        goto cleanup;
    instanceId = SDL_JoystickGetDeviceInstanceID(deviceIndex);
    if (instanceId < 0)
        goto cleanup;

    g_pfnInputPump_005c840c = get_player_input;
    g_nArcadeState_0049d75c = 0;
    g_acVduModeStackDepth_004934c8[1] = 1;
    g_aaiVduModeStack_00493498[1][0] = 0;
    stage = 5;
    if (!SdlSetJoystickMode("4button-4axis") ||
        !SdlSetJoystickAxesMode("twin-stick-roll"))
        goto cleanup;

    stage = 6;
    activeSlot = 0;
    while (activeSlot < 2) {
        FlushInputEvents();
        g_nActiveInputDevice_005d1726 = (short)activeSlot;
        SdlApplyJoystickFlightControls();
        SdlHandleJoystickButtonEvent(instanceId, 5, 1, 0);
        eventType = GetNextInputEvent(&input);
        if (eventType != 0)
            break;
        activeSlot++;
    }
    if (activeSlot == 2 || eventType != 5 || input.status != 0x11)
        goto cleanup;
    if (GetNextInputEvent(&input) != 4 || input.status != 0x11)
        goto cleanup;

    FlushInputEvents();
    stage = 7;
    g_bSceneEscapeRequested_0049d4b0 = 0;
    SdlHandleJoystickButtonEvent(instanceId, 8, 1, 0);
    if (g_bSceneEscapeRequested_0049d4b0 == 0 ||
        GetNextInputEvent(&input) != 4 || input.status != 1)
        goto cleanup;

    FlushInputEvents();
    stage = 8;
    SdlEndJoystickSpaceflight();
    g_bInputEventQueueEnabled_0049c248 = 1;
    SdlHandleJoystickButtonEvent(instanceId, 3, 1, 0);
    if (GetNextInputEvent(&input) != 5 || input.status != 0x15 ||
        input.value != 'y')
        goto cleanup;
    if (GetNextInputEvent(&input) != 4 || input.status != 0x15 ||
        input.value != 'y')
        goto cleanup;
#endif

    result = 0;

cleanup:
    if (result != 0)
        fprintf(stderr, "SDL event compatibility failure at stage %d\n",
                stage);
    FlushInputEvents();
    g_apObjectShape_00493868[0] = 0;
    g_pConstellationShape_005d2c4c = 0;
    ReleasePreparedTestSprite(planetShape);
    ReleasePreparedTestSprite(constellationShape);
    SdlSetMouseGrab(0);
    SdlShutdownJoysticks();
#if SDL_VERSION_ATLEAST(2, 0, 14)
    if (deviceIndex >= 0)
        SDL_JoystickDetachVirtual(deviceIndex);
#endif
    g_hMainWindow_005d10e0 = 0;
    if (window != 0)
        SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}
