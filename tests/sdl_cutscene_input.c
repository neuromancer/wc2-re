#include "game.h"

static int PushMouseButton(SDL_Window *window, Uint32 type, Uint8 button)
{
    SDL_Event event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.button.windowID = SDL_GetWindowID(window);
    event.button.button = button;
    event.button.state = type == SDL_MOUSEBUTTONDOWN
                             ? SDL_PRESSED : SDL_RELEASED;
    event.button.x = 160;
    event.button.y = 100;
    return SDL_PushEvent(&event) == 1;
}

static int PushSpaceKey(SDL_Window *window, Uint32 type)
{
    SDL_Event event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key.windowID = SDL_GetWindowID(window);
    event.key.state = type == SDL_KEYDOWN ? SDL_PRESSED : SDL_RELEASED;
    event.key.keysym.scancode = SDL_SCANCODE_SPACE;
    event.key.keysym.sym = SDLK_SPACE;
    return SDL_PushEvent(&event) == 1;
}

static int CheckCutsceneAdvance(int expectedAdvance)
{
    SceneFlicObject sprite = {0};
    CutscenePlane plane = {0};
    CutsceneSequence sequence = {0};
    unsigned char script[] = {0x23}; /* Yield after the input check. */
    unsigned char *cursor;
    int result;

    cursor = script;
    g_pCurrentCutsceneSprite_00499c78 = &sprite;
    g_pCurrentCutscenePlane_00499c7c = &plane;
    g_pCurrentCutsceneSequence_00499c80 = &sequence;
    g_bCutsceneSkipFrame_00499c54 = 0;
    g_bCutsceneTextAdvance_005d2ed0 = 1;
    /* Exercise ServiceInputDevices' flush between its two message pumps. */
    g_nNextInputPollTick_0049d6d4 = 0;
    result = RunCutsceneScript(&cursor, 2) == 0 &&
             cursor == script + sizeof(script) &&
             g_bCutsceneSkipFrame_00499c54 == expectedAdvance &&
             g_bCutsceneTextAdvance_005d2ed0 == !expectedAdvance &&
             g_nInputPressCount_0049c258 == 0;
    g_pCurrentCutsceneSprite_00499c78 = 0;
    g_pCurrentCutscenePlane_00499c7c = 0;
    g_pCurrentCutsceneSequence_00499c80 = 0;
    return result;
}

int main(int argumentCount, char **arguments)
{
    const Uint8 buttons[] = {SDL_BUTTON_LEFT, SDL_BUTTON_RIGHT};
    SDL_Window *window;
    int button;
    int result;
    int stage;

    (void)argumentCount;
    (void)arguments;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
        return 1;
    window = SDL_CreateWindow("Cutscene input test", 0, 0, 320, 200, 0);
    if (window == 0) {
        SDL_Quit();
        return 1;
    }
    g_hMainWindow_005d10e0 = (HWND)window;
    g_bKeyboardMouseEnabled_0049be68 = 0;
    g_pfnInputPump_005c840c = 0;
    SdlPumpEvents();
    FlushInputEvents();
    result = 1;

    for (button = 0; button < 2; button++) {
        /* A complete click drained before the script polls still advances
         * once, even though the poll flushes both queued mouse events. */
        stage = 1;
        g_nInputPollPeriod_0049d6d8 = 0;
        if (!PushMouseButton(window, SDL_MOUSEBUTTONDOWN, buttons[button]) ||
            !PushMouseButton(window, SDL_MOUSEBUTTONUP, buttons[button]))
            goto cleanup;
        SdlPumpEvents();
        if (FindQueuedInputEvent(1) == 0 || FindQueuedInputEvent(2) == 0 ||
            !CheckCutsceneAdvance(1) || !CheckCutsceneAdvance(0))
            goto cleanup;

        /* Holding after the first advance and releasing on the next line
         * must not generate a second advance. Keep the release event. */
        stage = 2;
        if (!PushMouseButton(window, SDL_MOUSEBUTTONDOWN, buttons[button]))
            goto cleanup;
        SdlPumpEvents();
        if (!CheckCutsceneAdvance(1) || !CheckCutsceneAdvance(0) ||
            !PushMouseButton(window, SDL_MOUSEBUTTONUP, buttons[button]))
            goto cleanup;
        SdlPumpEvents();
        if (FindQueuedInputEvent(2) == 0 || !CheckCutsceneAdvance(0))
            goto cleanup;

        /* Poll period 1 skips the first message pump, so these events arrive
         * after the flush and exercise the script's queued-input path. */
        stage = 3;
        g_nInputPollPeriod_0049d6d8 = 1;
        if (!PushMouseButton(window, SDL_MOUSEBUTTONDOWN, buttons[button]) ||
            !CheckCutsceneAdvance(1) ||
            !PushMouseButton(window, SDL_MOUSEBUTTONUP, buttons[button]) ||
            !CheckCutsceneAdvance(0))
            goto cleanup;

        /* An unrelated key release cannot cancel a pending mouse advance. */
        stage = 4;
        g_nInputPollPeriod_0049d6d8 = 0;
        if (!PushMouseButton(window, SDL_MOUSEBUTTONDOWN, buttons[button]) ||
            !PushMouseButton(window, SDL_MOUSEBUTTONUP, buttons[button]) ||
            !PushSpaceKey(window, SDL_KEYUP))
            goto cleanup;
        SdlPumpEvents();
        if (!CheckCutsceneAdvance(1) || !CheckCutsceneAdvance(0))
            goto cleanup;
    }

    /* The shared counter also retains a quick keyboard press until the
     * cutscene consumes it; a later release still only clears key state. */
    stage = 5;
    if (!PushSpaceKey(window, SDL_KEYDOWN) ||
        !PushSpaceKey(window, SDL_KEYUP))
        goto cleanup;
    SdlPumpEvents();
    if (FindQueuedInputEvent(5) == 0 ||
        g_abInputKeyState_005c80f0[0x39] != 0 ||
        !CheckCutsceneAdvance(1) || !CheckCutsceneAdvance(0))
        goto cleanup;
    result = 0;

cleanup:
    if (result != 0)
        fprintf(stderr, "Cutscene input failure at stage %d, button %d\n",
                stage, button);
    FlushInputEvents();
    g_hMainWindow_005d10e0 = 0;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}
