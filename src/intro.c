/*
 *  Title orchestra sequence and victory fireworks.
 *
 *  Retail object 0x407E40-0x40911F (link order; see docs/ORDER.md).
 */
#include "game.h"

typedef struct TitleActorMotion {
    short x;
    short y;
    short velocityX;
    short velocityY;
} TitleActorMotion;

char g_szTitleConductorFrames_00491c68[] =
    "opoqopoqopoqopoqqrstrq";

char *g_apszTitleActorFrames_00491c80[10] = {
    "abcdefghijkaakkkkaaaalllllllmmll",
    "aaaaaaaabcddeeddccffgghhgghhiiih",
    "aaaaaaaaaabbccbbaaccccccccccaccc",
    "aaaabbbcccddeeddccffggaaggffcaaa",
    "aabbbaacccddddddccddeeeeeeddccca",
    "aaaaaabbbaaabbaaaabbbbaabbbbaccc",
    "aabbcdbbbeaaaaaaeeaaaaaaaaaabaaa",
    "abbbaacccdeeaaeeddaaaaaaaaaadacc",
    "aaaaaaaaabbbccbbbbccddddddccaaaa",
    "abccdefgghiijjiihhkkllmmllkkhnnn"
};

TitleActorMotion g_aTitleActorMotion_00491ca8[10] = {
    { 58, 94, -1, 1 },
    { 186, 94, 0, 1 },
    { 278, 94, 1, 1 },
    { 58, 102, -3, 2 },
    { 186, 102, 0, 2 },
    { 278, 102, 3, 2 },
    { 58, 110, -5, 3 },
    { 186, 110, 0, 3 },
    { 278, 110, 5, 3 },
    { 158, 74, 0, 4 }
};

unsigned char *g_pTitleFieldShape_00491cf8;

unsigned char *g_apTitleSections_005d3f70[13];

void *g_pTitleMusic_005d3fa4;

#ifdef SDL_PORT
/* Not an original mechanism. AnimateTitleOrchestra, PlayTitleConductorCue,
 * PushTitleOrchestraAway, RevealTitleLogo and ShowVictoryScreen below
 * each draw through a tight loop gated only by
 * g_nFrameSkipCountdown_0049d760/g_nFrameSkip_0049d764 (or, in
 * ShowVictoryScreen's case, nothing at all) -- a render-skip counter
 * for slow machines (see ReportFramesSkipped, hudmsg.c), not a clock.
 * WaitForInputKey (disk.c) polls input every iteration but never blocks
 * on real elapsed time either. On the original, CPU speed alone kept
 * this roughly watchable; on modern hardware the whole sequence burns
 * through in a handful of milliseconds. The sequence's own exit
 * condition (g_nTitleMusicSequenceStage_0049be94, set from the live
 * OriginFx music playback position, see SdlServiceOriginFxMusic in
 * sdl/music.c) already ties overall duration to real audio time -- this
 * floor only paces the visible per-frame rate within that, which needed
 * its own real-time gate since none existed.
 *
 * The orchestra/logo-reveal sequence (Title_Sequence through
 * RevealTitleLogo) was empirically tuned in-game at 6 ticks (10fps;
 * started at 3, the 20fps rate the rest of the cinematic engine uses --
 * see CUTSCENE_MOUTH_MIN_TICKS, game.h, and opcode 0x7c's own
 * busy-wait, screens.c -- but that measured twice too fast, same
 * correction factor the mouth-pacing floor needed).
 *
 * ShowVictoryScreen's fireworks finale (the ~30-firework simultaneous
 * burst) needed a separate, faster rate -- confirmed against a real-386
 * capture of the DOS original (youtube.com/watch?v=uz78jS1RKOU,
 * ~00:57-01:10): the burst's onset is a single synchronous launch (all
 * fireworks visible within one 33ms video frame, confirming the
 * bulk-seed-29-at-once logic is itself correct, not a reconstruction
 * bug), and the entire burst-to-fully-faded span measured 0.467s
 * (frame-by-frame: first visible at the capture's 19.20s mark, fully
 * clear by 19.67s). The loop needs exactly 11 real draw iterations to
 * finish (fireworks 0-28 complete their 8-frame cycle by iteration 8;
 * firework 29, started 4 iterations late, finishes at iteration 11) --
 * 0.467s / 11 = ~42.5ms/iteration = ~2.55 ticks, rounded to 3.
 *
 * ShowVictoryScreen's FIRST loop (the individually-staggered small
 * fireworks, before the finale) draws at this same normal rate too --
 * a real-386 capture (frame-differencing pass, not spot samples) shows
 * further sparks appearing roughly every ~1-1.3s after the first,
 * becoming continuous in the last ~2.4s (the
 * g_nTitleMusicSequenceStage_0049be94 > 4 guaranteed-spawn phase)
 * until the finale ignites, spanning ~6.2s total -- far more than the
 * ~11 draws (at any per-frame rate) the reference build's own
 * loopCount <= 10 cap allows. Disassembling the real MS-DOS WC2.EXE
 * (D:\MSDOS\WC2\WC2.EXE, separate from and predating this Win32 port)
 * confirms that cap is Win32-specific: DOS's equivalent loop has no
 * iteration limit in music-driven mode at all, looping unconditionally
 * until g_nTitleMusicSequenceStage_0049be94 > 4 AND every firework has
 * finished animating (the same exit condition already used below) --
 * the 10-frame ceiling only exists as DOS's own non-music-driven
 * fallback, exactly matching frameCount below. The reference build
 * keeps the authentic (if abbreviated) Win32 cap; SDL_PORT restores the
 * uncapped DOS original in music-driven mode, deliberately diverging
 * from the Win32 reference here since that cap has no DOS counterpart. */
#define TITLE_FRAME_TICKS 6
#define TITLE_FIREWORKS_TICKS 3
#endif

/* Function start: 0x407E40 */
void Title_Sequence(void)
{
    int waitCount;
    short sectionIndex;

    waitCount = 0;
    g_nUiInputMode_005c8d3c = 0;
    g_pTitleMusic_005d3fa4 =
        FetchDiskPacketRetrying("music.r00", 19, 0);
    g_pTitleFieldShape_00491cf8 =
        FetchDiskPacketRetrying("field.v00", 1, 0);
    for (sectionIndex = 0; sectionIndex < 12; sectionIndex++) {
        g_apTitleSections_005d3f70[sectionIndex] =
            FetchDiskPacketRetrying("title.vga",
                                    (short)(sectionIndex + 1), 0);
    }
    g_pTitleFireworkShape_005c8f58 =
        g_apTitleSections_005d3f70[11];
    g_apTitleSections_005d3f70[12] =
        FetchDiskPacketRetrying("title.vga", 0, 0);
    InitializeConversationViewport();
    g_bSceneEscapeRequested_0049d4b0 = 0;
    while (1) {
        StartMusic(g_pTitleMusic_005d3fa4);
        DrawTitleOrchestra();
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            break;
        AnimateTitleOrchestra();
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            break;
        PlayTitleConductorCue();
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            break;
        PushTitleOrchestraAway();
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            break;
        sectionIndex = RevealTitleLogo();
        if (g_nMusicDriverMode_0049be8c == 2 ||
            g_nMusicDriverMode_0049be8c == 1) {
#ifdef SDL_PORT
            /* waitCount < 3 alone was enough on the original: each
             * WaitForInputKey/ServiceInputDevices call cost real time just
             * from polling hardware, so three iterations were three
             * meaningful waits. On modern hardware (or DOSBox) those calls
             * cost nothing, so this loop used to fall through in well under
             * a millisecond regardless of g_nTitleMusicSequenceStage_
             * 0049be94's real value -- letting ShowVictoryScreen start
             * almost immediately after the logo forms, instead of after the
             * real pause seen on real hardware (~9-10s between the logo
             * settling and the first firework, confirmed against a real-386
             * capture: youtube.com/watch?v=uz78jS1RKOU, ~00:57-01:10). Pace
             * each check for real and raise the iteration cap so it can
             * actually cover that span; g_nTitleMusicSequenceStage_0049be94
             * reaching 3 remains the real exit condition, same as before --
             * this cap is only the safety net against it never doing so. */
            while (g_nTitleMusicSequenceStage_0049be94 < 3 &&
                   waitCount < 200) {
                SdlPaceTitleFrame(TITLE_FRAME_TICKS);
#else
            while (g_nTitleMusicSequenceStage_0049be94 < 3 &&
                   waitCount < 3) {
#endif
                if (WaitForInputKey() != 0)
                    g_bSceneEscapeRequested_0049d4b0 = 1;
                if (g_bSceneEscapeRequested_0049d4b0 != 0)
                    break;
                waitCount++;
            }
        }
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            break;
        ShowVictoryScreen(sectionIndex);
        break;
    }
    StopMusic(0);
    FadeViewportPaletteToColour(
        &g_stModalSourceViewport_005d2c50,
        g_cSecondaryViewBufferColour_0049cb4c, 1);
    ClearViewport(&g_stModalSourceViewport_005d2c50,
                  g_cSecondaryViewBufferColour_0049cb4c);
    RestoreGamePalette();
    for (sectionIndex = 0; sectionIndex < 13; sectionIndex++)
        ReleasePacketHandle(g_apTitleSections_005d3f70[sectionIndex]);
    FreePacketAndClear(&g_pTitleFieldShape_00491cf8, 0);
    ReleasePacketHandle(g_pTitleMusic_005d3fa4);
    ResetScreenClipToFullHeight();
}

/* Function start: 0x40809D */
void DrawTitleSky(short planetY)
{
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                      g_pTitleFieldShape_00491cf8, 0);
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 160, planetY,
                      g_apTitleSections_005d3f70[12], 0);
}

/* Function start: 0x4080DF */
void DrawTitleOrchestra(void)
{
    short actor;

    DrawTitleSky(24);
    for (actor = 0; actor < 10; actor++) {
        DrawSpriteDefault(
            &g_stSecondaryViewBuffer_005d2c90,
            g_aTitleActorMotion_00491ca8[actor].x,
            g_aTitleActorMotion_00491ca8[actor].y,
            g_apTitleSections_005d3f70[actor + 1], 0);
    }
    PanToScreen(&g_stSecondaryViewBuffer_005d2c90,
                &g_stScreenViewport_005d21a0);
}

/* Random-range callee from the event manager. */
unsigned short RandomInRange(unsigned short lo, unsigned short hi);

/* Function start: 0x40815F */
void AnimateTitleOrchestra(void)
{
    int repeat;
    short frameNumber;
    short actorIndex;
    short minimumFrame;
    short totalFrames;

    minimumFrame = 0;
    totalFrames = 0;
    repeat = 0;
    g_nFrameSkipCountdown_0049d760 = 1;
    do {
        for (frameNumber = minimumFrame; frameNumber < 32; frameNumber++) {
            g_nFrameSkipCountdown_0049d760--;
            if (g_nFrameSkipCountdown_0049d760 < 1) {
                g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
#ifdef SDL_PORT
                SdlPaceTitleFrame(TITLE_FRAME_TICKS);
#endif
                DrawTitleSky(24);
                for (actorIndex = 0; actorIndex < 10; actorIndex++) {
                    DrawSpriteDefault(
                        &g_stSecondaryViewBuffer_005d2c90,
                        g_aTitleActorMotion_00491ca8[actorIndex].x,
                        g_aTitleActorMotion_00491ca8[actorIndex].y,
                        g_apTitleSections_005d3f70[actorIndex + 1],
                        (short)(g_apszTitleActorFrames_00491c80[actorIndex]
                                    [frameNumber] - 'a'));
                }
                RefreshMemoryStatusOverlay();
            }
            if (WaitForInputKey() != 0)
                g_bSceneEscapeRequested_0049d4b0 = 1;
            if (g_nTitleMusicSequenceStage_0049be94 >= 1 ||
                g_bSceneEscapeRequested_0049d4b0 != 0)
                return;
            totalFrames++;
        }
        minimumFrame = (short)(
            (unsigned short)RandomInRange(0, 13) + 9);
        for (frameNumber = 31; frameNumber >= minimumFrame; frameNumber--) {
            g_nFrameSkipCountdown_0049d760--;
            if (g_nFrameSkipCountdown_0049d760 < 1) {
                g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
#ifdef SDL_PORT
                SdlPaceTitleFrame(TITLE_FRAME_TICKS);
#endif
                DrawTitleSky(24);
                for (actorIndex = 0; actorIndex < 10; actorIndex++) {
                    DrawSpriteDefault(
                        &g_stSecondaryViewBuffer_005d2c90,
                        g_aTitleActorMotion_00491ca8[actorIndex].x,
                        g_aTitleActorMotion_00491ca8[actorIndex].y,
                        g_apTitleSections_005d3f70[actorIndex + 1],
                        (short)(g_apszTitleActorFrames_00491c80[actorIndex]
                                    [frameNumber] - 'a'));
                }
                RefreshMemoryStatusOverlay();
            }
            if (WaitForInputKey() != 0)
                g_bSceneEscapeRequested_0049d4b0 = 1;
            if (g_nTitleMusicSequenceStage_0049be94 >= 1 ||
                g_bSceneEscapeRequested_0049d4b0 != 0)
                return;
            totalFrames++;
        }
        if ((g_nMusicDriverMode_0049be8c == 0 ||
             g_nMusicDriverMode_0049be8c == 3) &&
            totalFrames > 30)
            return;
        repeat++;
        if (repeat > 1)
            return;
    } while (1);
}

/* Function start: 0x4083E6 */
void PlayTitleConductorCue(void)
{
    int repeat;
    short cueNumber;
    short actor;

    repeat = 0;
    g_nFrameSkipCountdown_0049d760 = 1;
    for (cueNumber = 0; cueNumber < 20; cueNumber++) {
        if (g_szTitleConductorFrames_00491c68[cueNumber] == 'p') {
            if (g_nMusicDriverMode_0049be8c == 2)
                g_nFrameSkipCountdown_0049d760 = 1;
            else if (g_nMusicDriverMode_0049be8c == 1)
                g_nFrameSkipCountdown_0049d760 = 1;
        } else if (g_szTitleConductorFrames_00491c68[cueNumber] == 'q') {
            if (g_nMusicDriverMode_0049be8c == 2)
                g_nFrameSkipCountdown_0049d760 = 1;
            else if (g_nMusicDriverMode_0049be8c == 1)
                g_nFrameSkipCountdown_0049d760 = 1;
        }
        g_nFrameSkipCountdown_0049d760--;
        if (g_nFrameSkipCountdown_0049d760 < 1) {
            g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
#ifdef SDL_PORT
            SdlPaceTitleFrame(TITLE_FRAME_TICKS);
#endif
            DrawTitleSky(24);
            for (actor = 0; actor < 9; actor++) {
                DrawSpriteDefault(
                    &g_stSecondaryViewBuffer_005d2c90,
                    g_aTitleActorMotion_00491ca8[actor].x,
                    g_aTitleActorMotion_00491ca8[actor].y,
                    g_apTitleSections_005d3f70[actor + 1],
                    (short)(g_apszTitleActorFrames_00491c80[actor][31]
                                - 'a'));
            }
            DrawSpriteDefault(
                &g_stSecondaryViewBuffer_005d2c90,
                g_aTitleActorMotion_00491ca8[9].x,
                g_aTitleActorMotion_00491ca8[9].y,
                g_apTitleSections_005d3f70[10],
                (short)(g_szTitleConductorFrames_00491c68[cueNumber] - 'a'));
            RefreshMemoryStatusOverlay();
        }
        if (WaitForInputKey() != 0)
            g_bSceneEscapeRequested_0049d4b0 = 1;
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            return;
    }

    DrawTitleSky(24);
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[0].x,
        g_aTitleActorMotion_00491ca8[0].y,
        g_apTitleSections_005d3f70[1],
        (short)(g_apszTitleActorFrames_00491c80[0][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[1].x,
        g_aTitleActorMotion_00491ca8[1].y,
        g_apTitleSections_005d3f70[2],
        (short)(g_apszTitleActorFrames_00491c80[1][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[2].x,
        g_aTitleActorMotion_00491ca8[2].y,
        g_apTitleSections_005d3f70[3],
        (short)(g_apszTitleActorFrames_00491c80[2][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[3].x,
        g_aTitleActorMotion_00491ca8[3].y,
        g_apTitleSections_005d3f70[4],
        (short)(g_apszTitleActorFrames_00491c80[3][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[4].x,
        g_aTitleActorMotion_00491ca8[4].y,
        g_apTitleSections_005d3f70[5],
        (short)(g_apszTitleActorFrames_00491c80[4][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[5].x,
        g_aTitleActorMotion_00491ca8[5].y,
        g_apTitleSections_005d3f70[6],
        (short)(g_apszTitleActorFrames_00491c80[5][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[6].x,
        g_aTitleActorMotion_00491ca8[6].y,
        g_apTitleSections_005d3f70[7],
        (short)(g_apszTitleActorFrames_00491c80[6][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[7].x,
        g_aTitleActorMotion_00491ca8[7].y,
        g_apTitleSections_005d3f70[8],
        (short)(g_apszTitleActorFrames_00491c80[7][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[8].x,
        g_aTitleActorMotion_00491ca8[8].y,
        g_apTitleSections_005d3f70[9],
        (short)(g_apszTitleActorFrames_00491c80[8][31] - 'a'));
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
        g_aTitleActorMotion_00491ca8[9].x,
        g_aTitleActorMotion_00491ca8[9].y,
        g_apTitleSections_005d3f70[10],
        (short)(g_szTitleConductorFrames_00491c68[20] - 'a'));
    if (g_nMusicDriverMode_0049be8c == 2 ||
        g_nMusicDriverMode_0049be8c == 1) {
        do {
            if (WaitForInputKey() != 0)
                g_bSceneEscapeRequested_0049d4b0 = 1;
            if (g_nTitleMusicSequenceStage_0049be94 >= 2 ||
                g_bSceneEscapeRequested_0049d4b0 != 0) {
                RefreshMemoryStatusOverlay();
                break;
            }
            repeat++;
            if (repeat > 1)
                break;
        } while (1);
    }
}

/* Function start: 0x4087FF */
void DrawScaledTitleLogo(short y, short scale)
{
    short bounds[4];

    GetTransformedShapeBounds(
        &g_stSecondaryViewBuffer_005d2c90, 162, y,
        g_apTitleSections_005d3f70[0], 1, 0, scale, 0, bounds);
    DrawSpriteScaled(&g_stSecondaryViewBuffer_005d2c90, bounds[0], y,
                     g_apTitleSections_005d3f70[0], 0, 0, scale, 0);
    DrawSpriteScaled(&g_stSecondaryViewBuffer_005d2c90, 163, y,
                     g_apTitleSections_005d3f70[0], 1, 0, scale, 0);
    DrawSpriteScaled(&g_stSecondaryViewBuffer_005d2c90,
                     (short)(bounds[2] + 1), y,
                     g_apTitleSections_005d3f70[0], 2, 0, scale, 0);
}

/* Function start: 0x4088A9 */
void PushTitleOrchestraAway(void)
{
    short x;
    short y;
    short actor;
    short step;

    g_nFrameSkipCountdown_0049d760 = 1;
    for (step = 1; step < 120;
         step = (short)(step + step / 4 + 1)) {
        g_nFrameSkipCountdown_0049d760--;
        if (g_nFrameSkipCountdown_0049d760 < 1) {
            g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
#ifdef SDL_PORT
            SdlPaceTitleFrame(TITLE_FRAME_TICKS);
#endif
            DrawTitleSky(24);
            for (actor = 0; actor < 9; actor++) {
                x = (short)(g_aTitleActorMotion_00491ca8[actor].x +
                    g_aTitleActorMotion_00491ca8[actor].velocityX *
                        step);
                y = (short)(g_aTitleActorMotion_00491ca8[actor].y +
                    g_aTitleActorMotion_00491ca8[actor].velocityY *
                        step);
                DrawSpriteScaled(
                    &g_stSecondaryViewBuffer_005d2c90, x, y,
                    g_apTitleSections_005d3f70[actor + 1],
                    (short)(g_apszTitleActorFrames_00491c80[actor][31]
                                - 'a'),
                    0,
                    (short)(g_aTitleActorMotion_00491ca8[actor].velocityY *
                                step * 4 + 0x100),
                    0);
            }
            x = (short)(g_aTitleActorMotion_00491ca8[9].x +
                g_aTitleActorMotion_00491ca8[9].velocityX * step);
            y = (short)(g_aTitleActorMotion_00491ca8[9].y +
                g_aTitleActorMotion_00491ca8[9].velocityY * step);
            DrawSpriteScaled(
                &g_stSecondaryViewBuffer_005d2c90, x, y,
                g_apTitleSections_005d3f70[10],
                (short)(g_szTitleConductorFrames_00491c68[21] - 'a'),
                0,
                (short)(g_aTitleActorMotion_00491ca8[9].velocityY *
                            step * 4 + 0x100),
                0);
            RefreshMemoryStatusOverlay();
        }
        if (WaitForInputKey() != 0)
            g_bSceneEscapeRequested_0049d4b0 = 1;
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            break;
    }
}

/* Function start: 0x408A88 */
short RevealTitleLogo(void)
{
    short scale;
    short distance;
    short logoY;
    short bounds[4];
    short redraw;
    short drawPlanetFirst;
    short scaleDelay;
    short planetY;
    short positionDelay;

    drawPlanetFirst = 0;
    logoY = 59;
    g_nFrameSkipCountdown_0049d760 = 1;
    distance = 5000;
    scaleDelay = 0;
    positionDelay = 0;
    for (; distance >= 1000;
         distance--, scaleDelay--, positionDelay--) {
        redraw = 0;
        if (scaleDelay == 0) {
            redraw = 1;
            planetY = (short)(120000 / (int)distance);
            scaleDelay = 200;
        }
        if (positionDelay == 0) {
            redraw = 1;
            scale = (short)(256000 / (int)distance);
            if (distance > 3000)
                logoY = (short)(logoY - 2);
            else
                logoY = (short)(logoY + 2);
            positionDelay = 100;
        }
        if (redraw != 0) {
            g_nFrameSkipCountdown_0049d760--;
            if (g_nFrameSkipCountdown_0049d760 < 1) {
                g_nFrameSkipCountdown_0049d760 = g_nFrameSkip_0049d764;
#ifdef SDL_PORT
                SdlPaceTitleFrame(TITLE_FRAME_TICKS);
#endif
                DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                                  0, 0,
                                  g_pTitleFieldShape_00491cf8, 0);
                GetTransformedShapeBounds(
                    &g_stSecondaryViewBuffer_005d2c90, 162, logoY,
                    g_apTitleSections_005d3f70[0], 1, 0,
                    scale, 0, bounds);
                if (bounds[3] < planetY)
                    drawPlanetFirst = 1;
                if (drawPlanetFirst == 0) {
                    DrawScaledTitleLogo(logoY, scale);
                    DrawSpriteDefault(
                        &g_stSecondaryViewBuffer_005d2c90,
                        160, planetY,
                        g_apTitleSections_005d3f70[12], 0);
                } else {
                    DrawSpriteDefault(
                        &g_stSecondaryViewBuffer_005d2c90,
                        160, planetY,
                        g_apTitleSections_005d3f70[12], 0);
                    DrawScaledTitleLogo(logoY, scale);
                }
                RefreshMemoryStatusOverlay();
            }
        }
        if (WaitForInputKey() != 0)
            g_bSceneEscapeRequested_0049d4b0 = 1;
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            return logoY;
    }
    DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                      g_pTitleFieldShape_00491cf8, 0);
    DrawScaledTitleLogo(logoY, scale);
    RefreshMemoryStatusOverlay();
    return logoY;
}

/* Function start: 0x408CC8 */
void ShowVictoryScreen(short logoY)
{
    short finishing;
    int loopCount;
    short emptyCount;
    short finalEmptyCount;
    short index;
    short frameCount;
    short spawnFirework;

    finishing = 0;
    frameCount = 0;
    loopCount = 0;
    FlushSoundEffects();
    InitializeFireworks();
    do {
        if (finishing == 0) {
            if (g_nMusicDriverMode_0049be8c == 2 ||
                g_nMusicDriverMode_0049be8c == 1) {
                if (g_nTitleMusicSequenceStage_0049be94 >= 4) {
                    spawnFirework = (short)(
                        (unsigned short)RandomInRange(0, 0) == 0);
                } else {
                    spawnFirework = (short)(
                        (unsigned short)RandomInRange(0, 5) == 0);
                }
            } else {
                spawnFirework = (short)(
                    (unsigned short)RandomInRange(0, 0) == 0);
            }
        }
        if (spawnFirework != 0 && finishing == 0) {
            for (index = 0; index < 5; index++) {
                if (g_aFireworks_005c8df0[index].frame == -1) {
                    g_aFireworks_005c8df0[index].frame = 0;
                    g_aFireworks_005c8df0[index].x =
                        RandomInRange(0, 319);
                    g_aFireworks_005c8df0[index].y =
                        RandomInRange(0, 127);
                    g_aFireworks_005c8df0[index].variant =
                        RandomInRange(0, 2);
                    break;
                }
            }
        }
#ifdef SDL_PORT
        /* Same normal rate as the orchestra sequence -- this loop no
         * longer needs its own rate now that the loopCount cap below
         * is lifted in music-driven mode; real elapsed time comes from
         * running as many draws as it actually takes, not from
         * slowing each individual draw down. */
        SdlPaceTitleFrame(TITLE_FRAME_TICKS);
#endif
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                          g_pTitleFieldShape_00491cf8, 0);
        emptyCount = TheEndFireWorks(
            &g_stSecondaryViewBuffer_005d2c90, 5);
        DrawScaledTitleLogo(logoY, 0x100);
        RefreshMemoryStatusOverlay();
        frameCount++;
        if (finishing == 1 && emptyCount == 5)
            break;
        if (g_nMusicDriverMode_0049be8c == 2 ||
            g_nMusicDriverMode_0049be8c == 1) {
            if (g_nTitleMusicSequenceStage_0049be94 > 4)
                finishing = 1;
        } else if (frameCount > 10) {
            finishing = 1;
        }
        if (WaitForInputKey() != 0)
            g_bSceneEscapeRequested_0049d4b0 = 1;
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            return;
        loopCount++;
#ifdef SDL_PORT
        /* The real MS-DOS WC2.EXE's equivalent loop has no iteration
         * cap at all -- see the comment above SdlPaceTitleFrame.
         * Loop unconditionally here too: the finishing/emptyCount==5
         * break above (DOS's own only real exit) and the escape-key
         * return above are what actually end it. Non-music-driven mode
         * still ends the same way DOS does -- frameCount > 10 sets
         * finishing above, then this same break fires once every
         * firework has finished animating. */
    } while (1);
#else
    } while (loopCount <= 10);
#endif

    for (index = 0; index < 29; index++) {
        g_aFireworks_005c8df0[index].frame = 0;
        g_aFireworks_005c8df0[index].x = RandomInRange(0, 319);
        g_aFireworks_005c8df0[index].y = RandomInRange(0, 127);
        g_aFireworks_005c8df0[index].variant = RandomInRange(0, 2);
    }
    emptyCount = 0;
    finalEmptyCount = 0;
    do {
        if (emptyCount == 30)
            break;
        finalEmptyCount++;
        if (finalEmptyCount == 4) {
            g_aFireworks_005c8df0[29].frame = 0;
            g_aFireworks_005c8df0[29].x = RandomInRange(0, 319);
            g_aFireworks_005c8df0[29].y = RandomInRange(0, 127);
            g_aFireworks_005c8df0[29].variant = RandomInRange(0, 2);
        }
#ifdef SDL_PORT
        SdlPaceTitleFrame(TITLE_FIREWORKS_TICKS);
#endif
        DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90, 0, 0,
                          g_pTitleFieldShape_00491cf8, 0);
        emptyCount = TheEndFireWorks(
            &g_stSecondaryViewBuffer_005d2c90, 30);
        DrawScaledTitleLogo(logoY, 0x100);
        RefreshMemoryStatusOverlay();
        if (WaitForInputKey() != 0)
            g_bSceneEscapeRequested_0049d4b0 = 1;
    } while (g_bSceneEscapeRequested_0049d4b0 == 0);

    for (index = 29; index >= 0; index--) {
        if (g_aFireworks_005c8df0[index].frame != -1) {
            ((void (__cdecl *)(int, short))FlushSoundEffect)(
                g_aFireworks_005c8df0[index].soundHandle, index);
        }
    }
}
