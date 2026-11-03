/*
 *  Death sequence.
 *
 *  Retail object 0x4697A0-0x469ACF (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_bMissionEndPending_0049da4c = 1;
short g_bDeathSequenceActive_0049da50 = 0;

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
    g_nTrainSimActive_0049d758 = 1;
    g_bMissionEndPending_0049da4c = 0;
    g_bDeathSequenceActive_0049da50 = 1;
    remove_nav_pointer();
    free_cockpit();
    FreePacketAndClear(
        &g_aObjectTypeData_00496d30[OBJECT_DATA_EJECTED_PILOT].shapeSet,
        0);
    for (objIndex = 0; objIndex < 0x46; objIndex++) {
        if (g_asObjectType_00495298[objIndex] == OBJECT_DATA_EJECTED_PILOT)
            remove_object(objIndex);
    }
    g_nResourcePaletteState_005c57e6 = 1;
    StopMusicUnlessSuppressed();
    if (g_nMemoryConfiguration_005c8dc8 == 1)
        SceneLeaveHook(0x20);
    spacetrack(0x20, 2, 1);
    animShape = FetchDiskPacketRetrying("pilotanm.vga", 0, 0);
    cockpitBackground = FetchDiskPacketRetrying(
        g_szCockpitResourceFilename_005d1030, 3, 0);
    PlaySfxWaveFileByNumber(4, -1, 0);
    new_view(10, 0);
    g_bSceneEscapeRequested_0049d4b0 = 0;
    for (;;) {
        g_nFrameSkipCountdown_0049d760 = 1;
        for (frame = 0; frame < 8; frame++) {
            if (frame == 7) {
                ClearViewport(&g_stViewBuffer_005d2b00,
                              g_bPrimaryViewBufferColour_0049cb50);
            } else {
                Draw_3Space_Frame();
                DrawSpriteDefault(&g_stViewBuffer_005d2b00, 0, 0,
                                  cockpitBackground, 0);
            }
            DrawSpriteDefault(&g_stViewBuffer_005d2b00, 160, 199,
                              animShape, frame);
            dump_buffer_to_screen();
            clear_view_buffer();
            ServiceInputDevices(-1);
            if (g_bSceneEscapeRequested_0049d4b0 == 1)
                break;
        }
        free_view_buffer();
        FreePacketAndClear(&cockpitBackground, 0);
        FreePacketAndClear(&animShape, 0);
        ServiceInputDevices(-1);
        if (g_bSceneEscapeRequested_0049d4b0 == 1)
            break;
        g_nArcadeState_0049d75c = 0;
        load_all_slots();
        g_nResourcePaletteState_005c57e6 = 0;
        new_view(5, 0);
        g_nFrameSkipCountdown_0049d760 = 1;
        for (frame = 0; frame < 0x3c; frame++) {
            ServiceInputDevices(-1);
            if (g_bSceneEscapeRequested_0049d4b0 == 1)
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
    g_nArcadeState_0049d75c = 4;
    g_bSceneEscapeRequested_0049d4b0 = 0;
    free_all_slots();
    g_stScreenViewport_005d21a0.top = 0;
    g_stScreenViewport_005d21a0.bottom = 199;
    FadeViewportPaletteToColour(&g_stScreenViewport_005d21a0,
                                g_cSecondaryViewBufferColour_0049cb4c, 1);
    ClearViewport(&g_stScreenViewport_005d21a0,
                  g_cSecondaryViewBufferColour_0049cb4c);
    RestoreGamePalette();
    g_bMissionEndPending_0049da4c = 1;
    g_bDeathSequenceActive_0049da50 = 0;
#ifdef SDL_PORT
    SetDefaultFrameTiming();
#endif
}
