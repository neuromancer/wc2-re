/*
 *  Death sequence.
 *
 *  Retail object 0x4697A0-0x469ACF (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_bMissionEndPending = 1; /* 0x0049da4c */
short g_bDeathSequenceActive = 0; /* 0x0049da50 */

/* Function start: 0x4697A0 (Mac symbol: death_sequence) */
void death_sequence(void)
{
    short objIndex;
    signed char frame;
    void *cockpitBackground;
    void *animShape;

    animShape = 0;
    cockpitBackground = 0;
#ifdef SDL_PORT
    /* This sequence advances the live 3-space simulation once per frame. */
    SetSpaceFlightFrameTiming();
#endif
    g_nTrainSimActive = 1;
    g_bMissionEndPending = 0;
    g_bDeathSequenceActive = 1;
    remove_nav_pointer();
    free_cockpit();
    FreePacketAndClear(
        &g_aObjectTypeData[OBJECT_DATA_EJECTED_PILOT].shapeSet,
        0);
    for (objIndex = 0; objIndex < 0x46; objIndex++) {
        if (g_asObjectType[objIndex] == OBJECT_DATA_EJECTED_PILOT)
            remove_object(objIndex);
    }
    g_1_nResourcePaletteState = 1;
    StopMusicUnlessSuppressed();
    if (g_98_nMemConfiguration == 1)
        SceneLeaveHook(0x20);
    spacetrack(0x20, 2, 1);
    animShape = FetchDiskPacketRetrying("pilotanm.vga", 0, 0);
    cockpitBackground = FetchDiskPacketRetrying(
        g_7_szCockpitAssetFilenameText, 3, 0);
    PlaySfxWaveFileByNumber(4, -1, 0);
    new_view(10, 0);
    g_bSceneEscapeRequested = 0;
    for (;;) {
        g_nFrameSkipCountdown = 1;
        for (frame = 0; frame < 8; frame++) {
            if (frame == 7) {
                ClearViewport(&g_27_stViewScratchInfo,
                              g_bPrimaryViewBufferColour);
            } else {
                Draw_3Space_Frame();
                DrawSpriteDefault(&g_27_stViewScratchInfo, 0, 0,
                                  cockpitBackground, 0);
            }
            DrawSpriteDefault(&g_27_stViewScratchInfo, 160, 199,
                              animShape, frame);
            dump_buffer_to_screen();
            clear_view_buffer();
            ServiceInputDevices(-1);
            if (g_bSceneEscapeRequested == 1)
                break;
        }
        free_view_buffer();
        FreePacketAndClear(&cockpitBackground, 0);
        FreePacketAndClear(&animShape, 0);
        ServiceInputDevices(-1);
        if (g_bSceneEscapeRequested == 1)
            break;
        g_nArcadeState = 0;
        load_all_slots();
        g_1_nResourcePaletteState = 0;
        new_view(5, 0);
        g_nFrameSkipCountdown = 1;
        for (frame = 0; frame < 0x3c; frame++) {
            ServiceInputDevices(-1);
            if (g_bSceneEscapeRequested == 1)
                break;
            if (Draw_3Space_Frame() != 0)
                dump_buffer_to_screen();
            switch (frame) {
            case 2:
                Explosion(0);
                break;
            }
        }
        break;
    }
    g_nArcadeState = 4;
    g_bSceneEscapeRequested = 0;
    free_all_slots();
    g_101_stScrViewInfo.top = 0;
    g_101_stScrViewInfo.bottom = 199;
    FadeViewportPaletteToColour(&g_101_stScrViewInfo,
                                g_cSecondaryViewBufferColour, 1);
    ClearViewport(&g_101_stScrViewInfo,
                  g_cSecondaryViewBufferColour);
    RestoreGamePalette();
    g_bMissionEndPending = 1;
    g_bDeathSequenceActive = 0;
#ifdef SDL_PORT
    SetDefaultFrameTiming();
#endif
}
