/*
 *  Cutscene engine and game flow.
 *
 *  Retail object 0x42BB70-0x43340F (link order; see docs/ORDER.md).
 */
#include "game.h"

static char g_szLastCutsceneWorkBuffer_00499ec0[16] = "@@@@@@@@@@";

static char g_szLastCutscenePrintBuffer_00499ed0[16] = "@@@@@@@@@@";

#ifdef SDL_PORT
/* Not an original engine flag -- tracks whether any speech clip has
 * actually played yet in the cutscene currently running (reset once per
 * RunLoadedCutscene call, set the first time AnimateCutsceneSpeakerMouth
 * observes g_bSpeechSoundActive_004a2660 go active). Most cutscenes have
 * no voice audio at all, so AnimateCutsceneSpeakerMouth needs a way to
 * tell "this line's audio hasn't started yet" (wait) apart from "this
 * cutscene has no audio" (animate from text, same as the original always
 * did) -- otherwise every unvoiced line blocks forever. */
static signed char g_bCutsceneSpeechAudioSeen = 0;
#endif

/* WC2's screens.c declares the memory-status flag one byte wide where the
 * rest of the game declares it as a short.  Only the load and store width
 * differ; the flag itself is 0 or 1. */
#define g_cShowMemoryStatus \
    (*(signed char *)&g_nShowMemoryStatus_0049d784)

#pragma function(memcpy)
/* Function start: 0x42BB70 */
void AnimateCutsceneSpeakerMouth(SceneFlicObject *sprite)
{
    signed char ticks;
    signed char frame;
    signed char speaker;
    signed char speechSpeed;

    speechSpeed = g_cCutsceneSpeechSpeed_00499eb4;
    if (g_bCutsceneTextAdvance_005d2ed0 == 0) {
        g_pszCutsceneSpeechCursor_00499eb0 = 0;
        return;
    }
#ifdef SDL_PORT
    /* The two gates below have no original-engine equivalent -- the
     * reference build always animated the mouth straight from text,
     * regardless of audio. Kept port-only so the reference build stays
     * byte-comparable to the original for verification; see the PR
     * discussion this note is based on. */
    if (g_bSpeechSoundActive_004a2660 != 0) {
        g_bCutsceneSpeechAudioSeen = 1;
    } else if (g_bCutsceneSpeechAudioSeen != 0) {
        /* Opcode 0x8a arms text-advance independently of when the paired
         * speech clip (opcode 0xb0) actually starts producing audio --
         * 0xb0's own cache miss falls back to a synchronous blocking disk
         * load (LoadAndPlaySpeechPacket, music.c). Hold the current frame
         * instead of animating from text alone until
         * g_bSpeechSoundActive_004a2660 is set, which happens only once
         * real playback begins (PlayRawSpeechBuffer, sound.c). Only applies
         * once this cutscene is known to have voice audio at all (see
         * g_bCutsceneSpeechAudioSeen) -- most cutscenes have none, and
         * text-driven animation with no audio to wait for is correct for
         * those, same as the original engine always did. */
        return;
    }
    if (g_pSpeechSound_004a2658 != 0 &&
        ix_sound_is_playing(g_pSpeechSound_004a2658) == 0) {
        /* g_bSpeechSoundActive_004a2660 clears only after
         * ServiceSoundSystem's (sound.c) grace period, which exists to
         * decide when to force-advance the script, not whether the mouth
         * should keep moving. Check the sound object's own playing state
         * directly instead, so the mouth stops the instant playback
         * actually stops. */
        return;
    }
#endif
    if (g_bCutsceneSkipFrame_00499c54 != 0 ||
        g_bCutsceneViewportPreallocated_00499c4c != 0) {
        g_bCutsceneSpeechActive_00499eb8 =
            (signed char)(g_bCutsceneTextAdvance_005d2ed0 = 0);
        g_pszCutsceneSpeechCursor_00499eb0 = 0;
        sprite->currentFrame = 11;
        return;
    }
    if (g_wDigiPlaying_0049bb60 != 0 ||
        g_bCutsceneSpeechActive_00499eb8 != 0) {
        if (g_wDigiPlaying_0049bb60 == 0) {
            g_bCutsceneSpeechActive_00499eb8 =
                (signed char)(g_bCutsceneTextAdvance_005d2ed0 = 0);
            sprite->currentFrame = 11;
            g_pszCutsceneSpeechCursor_00499eb0 = 0;
            return;
        }
        g_bCutsceneSpeechActive_00499eb8 = 1;
    }
    if (g_wDigiPlaying_0049bb60 != 0)
        speechSpeed = 0;
    sprite->waitStart = g_nInputClock_005c84a8;
    if (g_pszCutsceneSpeechCursor_00499eb0 == 0)
        g_pszCutsceneSpeechCursor_00499eb0 = g_pszCurrentCutsceneText_00499da4;
    speaker = (signed char)toupper(*g_pszCutsceneSpeechCursor_00499eb0);
    if (speaker < 0) {
        speaker = (signed char)(speaker + 0x80);
        frame = g_acCutsceneMouthFrames_00499db0[speaker];
        ticks = g_acCutsceneMouthDurations_00499e30[speaker];
    } else if (speaker < ' ') {
        if (speaker == 0) {
            g_bCutsceneSpeechActive_00499eb8 =
                (signed char)(g_bCutsceneTextAdvance_005d2ed0 = 0);
            g_pszCutsceneSpeechCursor_00499eb0 = 0;
        }
        frame = -1;
    } else if (speaker == 'T' &&
               toupper(*g_pszCutsceneSpeechCursor_00499eb0) == 'H') {
        frame = 8;
        ticks = 3;
        g_pszCutsceneSpeechCursor_00499eb0++;
    } else {
        frame = g_acCutsceneMouthFrames_00499db0[speaker];
        ticks = g_acCutsceneMouthDurations_00499e30[speaker];
    }
    if (frame != -1) {
        sprite->currentFrame = frame;
        sprite->waitTicks = (short)(speechSpeed * ticks);
#ifdef SDL_PORT
        /* Keep text-only lines readable when no speech controls the pace. */
        if (sprite->waitTicks < CUTSCENE_MOUTH_MIN_TICKS)
            sprite->waitTicks = CUTSCENE_MOUTH_MIN_TICKS;
        SdlTracef("[mouth] ch=%c speed=%d dur=%d wait=%d clock=%d "
                     "period=%ldms cache=%d speech=%d\n",
                     (char)(speaker >= ' ' ? speaker : '.'),
                     (int)speechSpeed, (int)ticks,
                     (int)sprite->waitTicks, (int)g_nInputClock_005c84a8,
                     g_nFramePeriodMilliseconds_005c343c,
                     (int)g_wDigiPlaying_0049bb60,
                     (int)g_bSpeechSoundActive_004a2660);
#endif
    } else {
        sprite->currentFrame = 11;
        sprite->waitTicks = (short)(speechSpeed * RandomInRange(2, 4));
    }
    if (g_pszCutsceneSpeechCursor_00499eb0 != 0 &&
        *g_pszCutsceneSpeechCursor_00499eb0 != 0) {
        g_pszCutsceneSpeechCursor_00499eb0++;
    }
}

/* Function start: 0x42BDDB */
signed char HasCutsceneMusicNode(CutsceneMusicNode *node)
{
    if (node != 0)
        return 1;
    return 0;
}

/* Function start: 0x42BDFE */
void WriteMemoryStateReportHook(void)
{
}

/* Function start: 0x42BE09 */
void LogMemoryStateToFile(FILE *file)
{
    FILE *previousFile;
    unsigned char previousFileMode;

    previousFile = g_pMemoryLogFile_00499da8;
    previousFileMode = g_bMemoryLogToFile_00499bf8;
    g_bMemoryLogToFile_00499bf8 = 1;
    g_pMemoryLogFile_00499da8 = file;
    WriteMemoryStateReportHook();
    g_pMemoryLogFile_00499da8 = previousFile;
    g_bMemoryLogToFile_00499bf8 = previousFileMode;
}

/* Function start: 0x42BE4B */
void WriteDetailedMemoryStateReport(void)
{
    unsigned int mainAvail;
    unsigned int mainLargest;
    unsigned int farFree;
    unsigned int farBlock;
    unsigned short nearTotal;

    WriteMemoryStateReportHook();
    if (g_pMemoryLogFile_00499da8 != 0) {
        mainAvail = GetAvailableMainMemory();
        mainLargest = GetLargestMainMemoryBlock();
        farFree = GetAvailableFarMemory();
        farBlock = GetLargestFreeMemoryBlock();
        nearTotal = GetOriginalFreeMemory();
        fprintf(g_pMemoryLogFile_00499da8,
                "----- Mem State ------\n");
        fprintf(g_pMemoryLogFile_00499da8,
                "NEAR: %ud\n", nearTotal);
        fprintf(g_pMemoryLogFile_00499da8,
                "FAR:  %6ld  Largest: %6ld\n",
                mainAvail, mainLargest);
        fprintf(g_pMemoryLogFile_00499da8,
                "EMS:  %6ld  Largest: %6ld\n",
                farFree, farBlock);
        fprintf(g_pMemoryLogFile_00499da8,
                "----------------------\n");
    }
}

#pragma function(memcmp, strlen)
/* Function start: 0x42BF0C */
void LogChangedCutsceneWorkBuffers(void)
{
    if (memcmp(g_szLastCutsceneWorkBuffer_00499ec0,
               g_pszCutsceneWorkBuffer_005d2ecc,
               strlen(g_szLastCutsceneWorkBuffer_00499ec0)) != 0 ||
        memcmp(g_szLastCutscenePrintBuffer_00499ed0,
               g_pszCutscenePrintBuffer_005d2f10,
               strlen(g_szLastCutscenePrintBuffer_00499ed0)) != 0) {
        fprintf(g_pMemoryLogFile_00499da8,
                "Show Work: %.10s...  %.10s...  %.10s...\n",
                g_pszCutsceneWorkBuffer_005d2ecc,
                g_pszCutscenePrintBuffer_005d2f10,
                g_pszCutsceneFormattedText_005d2dc8);
        memcpy(g_szLastCutsceneWorkBuffer_00499ec0,
               g_pszCutsceneWorkBuffer_005d2ecc, 10);
        memcpy(g_szLastCutscenePrintBuffer_00499ed0,
               g_pszCutscenePrintBuffer_005d2f10, 10);
    }
}

/* Function start: 0x42BFB8 */
void RouteCutsceneViewportToDisplay(void)
{
    unsigned char *firstRow;

    firstRow = 0;
    if (g_stSecondaryViewBuffer_005d2c90.pixels != 0 &&
        g_cCutsceneVideoMode_00499c48 == 0x13 &&
        g_bCutsceneViewportPreallocated_00499c4c == 0) {
        firstRow = g_stSecondaryViewBuffer_005d2c90.pixels +
            g_stSecondaryViewBuffer_005d2c90.rowOffsets[0];
        WriteDetailedMemoryStateReport();
        ReleasePacketSlot(&g_pCutsceneCockpitPalette_00499c0c);
        if (g_pMemoryLogFile_00499da8 != 0)
            fprintf(g_pMemoryLogFile_00499da8, "Rerouted LZ.\n");
        WriteDetailedMemoryStateReport();
    }
    g_pActiveCutscenePixels_005c83dc = firstRow;
}

/* Function start: 0x42C04B */
void ClearActiveCutscenePixelAlias(void)
{
    unsigned char *firstRow;

    if (g_stSecondaryViewBuffer_005d2c90.pixels != 0 &&
        g_cCutsceneVideoMode_00499c48 == 0x13) {
        firstRow = g_stSecondaryViewBuffer_005d2c90.pixels +
            g_stSecondaryViewBuffer_005d2c90.rowOffsets[0];
        if (g_pActiveCutscenePixels_005c83dc == firstRow)
            g_pActiveCutscenePixels_005c83dc = 0;
    }
}

/* Function start: 0x42C0A2 */
signed char IsCutsceneSpeechLoaded(void)
{
    short slot;

    for (slot = 0; slot < 8; slot++) {
        if (g_apszCutsceneSpeechFiles_005d2ee0[slot] != 0)
            return 1;
    }
    return 0;
}

#pragma intrinsic(memcpy)
/* Function start: 0x42C0F1 */
void DrawCutsceneTextAt(short x, short y, short viewportIndex,
                        const char *text)
{
    ShortRect savedBounds;

    g_stCutsceneDrawingTextContext_005d2f60 =
        g_stCutsceneTextContext_005d2f40;
    switch (viewportIndex) {
    case 0:
        g_stCutsceneDrawingTextContext_005d2f60.viewport =
            &g_stSceneFlicScratchViewport_005d2eb0;
        break;
    case 1:
        g_stCutsceneDrawingTextContext_005d2f60.viewport =
            &g_stSecondaryViewBuffer_005d2c90;
        break;
    case 2:
        g_stCutsceneDrawingTextContext_005d2f60.viewport =
            &g_stCutsceneTextViewport_005d2d90;
        break;
    case 3:
        g_stCutsceneDrawingTextContext_005d2f60.viewport =
            &g_stCutsceneTextBackingViewport_005d2db0;
        break;
    case 4:
        g_stCutsceneDrawingTextContext_005d2f60.viewport =
            &g_stModalSourceViewport_005d2c50;
        break;
    }
    SetTextContext(&g_stCutsceneDrawingTextContext_005d2f60);
    memcpy(&savedBounds,
           &g_stCutsceneDrawingTextContext_005d2f60.viewport->left,
           sizeof(savedBounds));
    if (g_bCutsceneTextBoundsOverride_00499edc != 0) {
        g_stCutsceneDrawingTextContext_005d2f60.viewport->left =
            MaxShort(g_nCutsceneTextLeft_00499ee0,
                     g_stCutsceneDrawingTextContext_005d2f60.viewport->left);
        g_stCutsceneDrawingTextContext_005d2f60.viewport->top =
            MaxShort(g_nCutsceneTextTop_00499ee2,
                     g_stCutsceneDrawingTextContext_005d2f60.viewport->top);
        g_stCutsceneDrawingTextContext_005d2f60.viewport->right =
            MinShort(g_nCutsceneTextRight_00499ee4,
                     g_stCutsceneDrawingTextContext_005d2f60.viewport->right);
        g_stCutsceneDrawingTextContext_005d2f60.viewport->bottom =
            MinShort(g_nCutsceneTextBottom_00499ee6,
                     g_stCutsceneDrawingTextContext_005d2f60.viewport->bottom);
    }
    if (g_bCinematicSpriteFontEnabled_005c82a7 == 0) {
        if (x > -1) {
            g_pCurrentTextContext_005c8d1c->cursorX =
                (short)(g_pCurrentTextContext_005c8d1c->viewport->left + x);
        }
        if (y > -1) {
            g_pCurrentTextContext_005c8d1c->cursorY =
                (short)(g_pCurrentTextContext_005c8d1c->viewport->top + y);
        }
    } else {
        g_pCurrentTextContext_005c8d1c->cursorX =
            (short)(g_pCurrentTextContext_005c8d1c->viewport->left + x);
        g_pCurrentTextContext_005c8d1c->cursorY =
            (short)(g_pCurrentTextContext_005c8d1c->viewport->top + y);
    }
#ifdef SDL_PORT
    if (memcmp(text, "50", 2) != 0) {
#else
    if ((IsCutsceneSpeechLoaded() == 0 ||
         g_pszCutsceneFormattedText_005d2dc8 == text ||
         g_nAudioEnabled_0049c244 == 0) &&
        memcmp(text, "50", 2) != 0) {
#endif
        FormatTextBufferFromStart("%s%P", text);
    }
    g_bCutsceneTextRestorePending_00499da0 = 1;
    SetTextContext(&g_stCutsceneTextContext_005d2f40);
    memcpy(&g_stCutsceneDrawingTextContext_005d2f60.viewport->left,
           &savedBounds, sizeof(savedBounds));
    g_bCutsceneTextBoundsOverride_00499edc = 0;
}

/* Function start: 0x42C373 */
void LoadCutsceneCockpitOverlay(int scriptValue)
{
    if (g_pCutsceneCockpitPacket_00499c04 == 0) {
        g_pCutsceneCockpitPacket_00499c04 = LoadNamedPacket(
            g_szCockpitResourceFilename_005d1030, 8, 0, 0, 0, 1);
    }
    (void)scriptValue;
}

/* Function start: 0x42C3A7 */
void DrawCutsceneCockpitOverlay(int unused, short verticalOffset)
{
    short hits;
    short x;
    short y;

    if (g_pCutsceneCockpitPacket_00499c04 != 0) {
        for (hits = 0; hits < 4; hits++) {
            if (g_asCockpitDamageState_005d1ee8[hits] == 1) {
                x = g_aCockpitDamagePositions_0049ae98[hits].x;
                y = (short)(
                    g_aCockpitDamagePositions_0049ae98[hits].y +
                    verticalOffset);
                DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                                  x, y,
                                  g_pCutsceneCockpitPacket_00499c04,
                                  hits);
            }
        }
    }
    (void)unused;
}

/* Function start: 0x42C43D */
void RunCutsceneWipeTransition(Viewport *destination, Viewport *source,
                               int wipeType, short duration)
{
    signed char done;
    void *workspace;

    done = 0;
    workspace = 0;
    g_bRoomTransitionAnimationEnabled_00499c00 = 1;
    if (g_bCutsceneSkipFrame_00499c54 == 0 &&
        g_bCutsceneViewportPreallocated_00499c4c == 0) {
        if (g_cCutsceneVideoMode_00499c48 == 0x13) {
            if (g_bRoomTransitionAnimationEnabled_00499c00 != 0)
                workspace = malloc(0xce);
            if (workspace == 0) {
                if (g_pMemoryLogFile_00499da8 != 0)
                    fprintf(g_pMemoryLogFile_00499da8,
                            "Unable to allocate Wipe ...");
                CopyViewportContents(source, destination);
            } else {
                if (InitializeViewportWipe(source, destination, wipeType,
                                           duration, 1, workspace) != 0) {
                    while (done == 0) {
                        done = AdvanceViewportWipe(workspace);
                        if (wipeType == 8 || wipeType == 9) {
                            MarkDibDirty();
                            DIBslamReal();
                        }
                    }
                } else if (g_pMemoryLogFile_00499da8 != 0) {
                    fprintf(g_pMemoryLogFile_00499da8,
                            "Unable to allocate Wipe 1 ...");
                }
                FinishViewportWipe(workspace);
                free(workspace);
            }
        } else {
            CopyViewportContents(source, destination);
        }
    }
}

/* Function start: 0x42C5A2 */
void WaitForCutsceneInputEvent(void)
{
    if (FindQueuedInputEvent(2) == 0) {
        for (;;) {
            if (ServiceInputDevices(-1) != 0) {
                ServiceCutsceneRuntimeHook();
                if (FindQueuedInputEvent(2) != 0 ||
                    FindQueuedInputEvent(5) != 0)
                    break;
            }
#ifdef SDL_PORT
            /* See issue #33.  Escape is the one input this loop can
             * never see: it only exits on a queued mouse-up (2) or
             * key-up (5), and once g_bSceneEscapeRequested_0049d4b0 is
             * set the ServiceInputDevices() poll returns 1 immediately
             * (eventmgr.c) -- so if the Escape key-up event is flushed
             * before this loop looks for it (ServiceInputDevices()
             * flushes the queue on its poll-tick path, and returns 1
             * from it), the loop spins at full CPU with no way out but
             * another keypress.  Adding an exit can only shorten the
             * wait, never block one: the caller returns to the
             * interpreter loop, whose top-of-loop check converts the
             * pending Escape. */
            if (g_bSceneEscapeRequested_0049d4b0 != 0)
                break;
#endif
        }
    }
}

/* Function start: 0x42C607 */
CutsceneResourceTable *FindActiveCutsceneFileResources(
    CutsceneResourceTable *resources)
{
    while (resources != 0) {
        if (resources->owner == g_nActiveCutsceneResourceLevel_00499d9c)
            return resources;
        else
            resources = resources->next;
    }
    FatalCutsceneError("Missing cutscene file resource level");
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42C659 */
CutsceneObjectResourceList *FindActiveCutsceneObjectResources(
    CutsceneObjectResourceList *resources)
{
    while (resources != 0) {
        if (resources->owner == g_nActiveCutsceneResourceLevel_00499d9c)
            return resources;
        else
            resources = resources->next;
    }
    FatalCutsceneError("Missing cutscene object resource level");
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42C6AC */
short RemoveCutsceneMemberIndex(short count, unsigned char *indices,
                                signed char index)
{
    short member;

    for (member = 0; member < count; member++) {
        if (indices[member] == (unsigned char)index) {
            DosMemcpy(indices, &indices[member + 1], (size_t)(15 - member));
            count--;
        }
    }
    return count;
}

/* Function start: 0x42C725 */
void FatalCutsceneError(const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    vsprintf(g_pszCutsceneWorkBuffer_005d2ecc, format, arguments);
#ifdef SDL_PORT
    va_end(arguments);
#endif
    exit_squadron(g_pszCutsceneWorkBuffer_005d2ecc);
}

/* Function start: 0x42C75D */
void ResetCutsceneSpriteDrawTicks(void)
{
    short index;

    for (index = 0; index < 0x80; index++) {
        if (g_apSceneObjects_00499c38[index] != 0)
            g_apSceneObjects_00499c38[index]->tick = 0;
    }
    g_nCutsceneSpritesDrawn_00499c10 = 0;
}

/* Function start: 0x42C7BC */
void PresentCutsceneFrame(Viewport *source, Viewport *destination)
{
    Viewport debugViewport;
    unsigned short debugFrameNumber;

    if (g_cCutsceneVideoMode_00499c48 != 0x13 &&
        g_cCutsceneVideoMode_00499c48 != 0x0d)
        return;
    if (g_bCutsceneDebugOverlayEnabled_00499ee8 != 0) {
        debugViewport = g_stModalSourceViewport_005d2c50;
        debugViewport.bottom = 0x17;
        g_stCutsceneDebugTextContext_005d2df0 =
            g_stCutsceneTextContext_005d2f40;
        g_stCutsceneDebugTextContext_005d2df0.viewport = &debugViewport;
        g_stCutsceneDebugTextContext_005d2df0.alignment = 0;
        g_stCutsceneDebugTextContext_005d2df0.cursorY = 0;
        g_stCutsceneDebugTextContext_005d2df0.cursorX = 0;
        SetTextContext(&g_stCutsceneDebugTextContext_005d2df0);
        debugFrameNumber = g_wCutsceneDebugFrameNumber_00499eec++;
        DrawFormattedText(
            "%X%Y%d Delay: %d DrawsCalled: %d * %d Time: %U   ",
            0, 0, debugFrameNumber, g_nCutsceneFrameDelay_00499c8c,
            g_nCutsceneSpritesDrawn_00499c10,
            g_bCutsceneFramePresented_005d2de0,
            g_nInputClock_005c84a8);
    }
    if (g_nFrameSkipCountdown_0049d760 == 0) {
        ConsumeCutscenePalettePacket(0, 0x100, 1);
        CopyViewportContents(source, destination);
        ResetCutsceneSpriteDrawTicks();
    }
    SetTextContext(&g_stCutsceneTextContext_005d2f40);
}

/* Function start: 0x42C8E1 */
void ConsumeCutscenePalettePacket(short firstColour, short count,
                                  signed char releasePacket)
{
    signed char malformed;
    unsigned int size;
    unsigned char *nextChunk;
    unsigned char *chunk;

    malformed = 0;
    if (g_pCutscenePalettePacket_005d2e98 == 0)
        return;
    if (ReadNextSceneForm(&g_pCutscenePalettePacket_005d2e98, &nextChunk) ==
        0x204c4150) {
        chunk = g_pCutscenePalettePacket_005d2e98;
        g_pCutscenePalettePacket_005d2e98 += 4;
        if (*(unsigned int *)chunk == 0x50414d43) {
            size = *(unsigned int *)g_pCutscenePalettePacket_005d2e98;
            g_pCutscenePalettePacket_005d2e98 += 4;
            SwapSceneChunkSizeEndian((int *)&size);
            if (size / 3 != 0x100)
                malformed++;
        } else {
            malformed++;
        }
        if (malformed != 0)
            FatalCutsceneError("Malformed cutscene palette");
    }
    if (releasePacket != 0)
        ReleasePacketSlot((void **)&g_pCutscenePalettePacket_005d2e98);
}

/* Function start: 0x42C9B0 */
/* Returns the playing sound effect slot of this resource: the third slot
 * found for it is stopped and reused; otherwise 0. */
CutsceneSoundEffect *FindCutsceneSoundEffect(short resourceIndex)
{
    CutsceneSoundEffect *effect;
    short count;

    effect = g_pCutsceneSoundEffects_00499c28;
    count = 0;
    while (effect != 0) {
        if (effect->resourceIndex == resourceIndex) {
            count++;
            if (count == 3) {
                ((void (__cdecl *)(void *, int))FlushSoundEffect)(
                    effect->sound, 0);
                return effect;
            }
        }
        effect = effect->next;
    }
    return effect;
}

/* Function start: 0x42CA28 */
/* Nothing in the shipped executable calls this.  Records a started sound
 * effect: reuses the slot FindCutsceneSoundEffect returns, or links a new
 * slot at the head of the list. */
void AddCutsceneSoundEffect(void *sound, short resourceIndex)
{
    CutsceneSoundEffect *effect;

    if (g_bCutsceneSkipFrame_00499c54 != 0 ||
        g_bCutsceneViewportPreallocated_00499c4c != 0 ||
        g_bCutsceneSkipAll_00499c58 != 0)
        return;
    effect = FindCutsceneSoundEffect(resourceIndex);
    if (effect != 0) {
        effect->sound = sound;
        return;
    }
    effect = (CutsceneSoundEffect *)calloc(1, sizeof(CutsceneSoundEffect));
    if (effect == 0)
        FatalCutsceneError("A5");
    if (g_pMemoryLogFile_00499da8 != 0) {
        fprintf(g_pMemoryLogFile_00499da8, "Alloc FX: %p free: %d\n",
                effect, (int)(short)GetOriginalFreeMemory());
    }
    effect->resourceIndex = resourceIndex;
    effect->sound = sound;
    effect->next = g_pCutsceneSoundEffects_00499c28;
    g_pCutsceneSoundEffects_00499c28 = effect;
}

/* Function start: 0x42CB08 */
void ReleaseCutsceneSoundEffects(short resourceIndex)
{
    CutsceneSoundEffect *previous;
    CutsceneSoundEffect *effect;

    previous = 0;
    effect = g_pCutsceneSoundEffects_00499c28;
    if (g_nMusicDriverMode_0049be8c != 1 && g_nMusicDriverMode_0049be8c != 2)
        return;
    if (resourceIndex == -1) {
        while (effect != 0) {
            ((void (__cdecl *)(void *, int))FlushSoundEffect)(
                effect->sound, 0);
            previous = effect->next;
            free(effect);
            effect = previous;
        }
        g_pCutsceneSoundEffects_00499c28 = 0;
    } else {
        while (effect != 0) {
            if (effect->resourceIndex == resourceIndex) {
                ((void (__cdecl *)(void *, int))FlushSoundEffect)(
                    effect->sound, 0);
                if (previous != 0)
                    previous->next = effect->next;
                else
                    g_pCutsceneSoundEffects_00499c28 = effect->next;
                free(effect);
                if (g_pMemoryLogFile_00499da8 != 0) {
                    fprintf(g_pMemoryLogFile_00499da8,
                            "Free FX: %p free: %d\n", effect,
                            (int)(short)GetOriginalFreeMemory());
                }
                return;
            }
            previous = effect;
            effect = effect->next;
        }
    }
}

/* Function start: 0x42CC42 */
void LoadCutsceneSpeechSlot(short resourceIndex, short slot)
{
    CutsceneResourceTable *resourceTable;
    int packetSize;

    if (g_bSpeechEnabled_005c8de8 == 0 ||
        g_bCutsceneViewportPreallocated_00499c4c != 0 ||
        g_bCutsceneSkipAll_00499c58 != 0)
        return;
    resourceTable = FindActiveCutsceneFileResources(
        g_pCutsceneSpeechResources_004928a4);
#ifdef SDL_PORT
    /* See issue #33 (review on PR #34): bailing out here while the
     * cache is still busy must not fall through to releasing/replacing
     * this slot below -- a still-in-flight fetch may be filling it.
     * g_wDigiPlaying_0049bb60 is never set nonzero anywhere in
     * this reconstruction today, so this loop cannot actually break
     * early yet -- guarded correctly anyway rather than leaving
     * unsound-if-ever-live code here. */
    while (g_wDigiPlaying_0049bb60 != 0) {
        PumpWindowMessages(0);
        if (g_bSceneEscapeRequested_0049d4b0 != 0)
            return;
    }
#else
    while (g_wDigiPlaying_0049bb60 != 0)
        PumpWindowMessages(0);
#endif
    ReleasePacketSlot(&g_apCutsceneSpeechPackets_005d2f80[slot]);
    g_asCutsceneSpeechChannels_005d2d70[slot] = 0;
    g_apszCutsceneSpeechFiles_005d2ee0[slot] =
        GetPackedStringByIndex(
            resourceTable, resourceTable->filenameIndices[resourceIndex]);
    g_asCutsceneSpeechSections_005d2dd0[slot] =
        resourceTable->sectionIndices[resourceIndex];
    packetSize = (int)GetNamedPacketSize(
        g_apszCutsceneSpeechFiles_005d2ee0[slot],
        g_asCutsceneSpeechSections_005d2dd0[slot]);
    if (packetSize <= 0) {
        g_apszCutsceneSpeechFiles_005d2ee0[slot] = 0;
        g_asCutsceneSpeechSections_005d2dd0[slot] = 0;
    } else {
        g_apCutsceneSpeechPackets_005d2f80[slot] =
            LoadSpeechPacketIntoBuffer(
                g_apszCutsceneSpeechFiles_005d2ee0[slot],
                g_asCutsceneSpeechSections_005d2dd0[slot], 0);
        if (g_apCutsceneSpeechPackets_005d2f80[slot] != 0) {
            g_asCutsceneSpeechChannels_005d2d70[slot] =
                (short)packetSize;
        }
    }
}

/* Function start: 0x42CDB5 */
/* Nothing in the shipped executable calls this.  Makes sure a song is in the
 * loaded-song list: an empty list gets a first node, otherwise a new node is
 * added after the last one.  The list pointer is left at the new node. */
void AddCutsceneMusicNode(CutsceneMusicNode **head, short resourceIndex)
{
    CutsceneMusicNode *node;
    CutsceneMusicNode *nextNode;
    CutsceneMusicNode *allocated;

    node = *head;
    if (g_nMusicDriverMode_0049be8c != 1 &&
        g_nMusicDriverMode_0049be8c != 2)
        return;
    if (g_bHighMemoryResourcesEnabled_005c80e4 == 0)
        return;
    if (head == 0)
        return;
    if (HasCutsceneMusicNode(node) == 0) {
        allocated = (CutsceneMusicNode *)calloc(1,
                                                sizeof(CutsceneMusicNode));
        if (allocated == 0)
            FatalCutsceneError("A6");
        if (g_pMemoryLogFile_00499da8 != 0) {
            fprintf(g_pMemoryLogFile_00499da8,
                    "Alloc Music: %p free: %d\n", allocated,
                    (int)(short)GetOriginalFreeMemory());
        }
        node = allocated;
        node->resourceIndex = (unsigned char)resourceIndex;
        node->packet = LoadCachedCutsceneResource(
            FindActiveCutsceneFileResources(
                g_pCutsceneMusicResources_004928bc),
            resourceIndex, 2);
        *head = node;
        return;
    }
    while (HasCutsceneMusicNode(node->previous) != 0)
        node = node->previous;
    do {
        nextNode = node->next;
        if (node->resourceIndex == resourceIndex)
            return;
        if (HasCutsceneMusicNode(nextNode) != 0)
            node = nextNode;
    } while (HasCutsceneMusicNode(nextNode) != 0);
    allocated = (CutsceneMusicNode *)calloc(1, sizeof(CutsceneMusicNode));
    if (allocated != 0) {
        if (g_pMemoryLogFile_00499da8 != 0) {
            fprintf(g_pMemoryLogFile_00499da8,
                    "Alloc Music: %p free: %d\n", allocated,
                    (int)(short)GetOriginalFreeMemory());
        }
        nextNode = allocated;
        node->next = nextNode;
        *head = nextNode;
        nextNode->previous = node;
        nextNode->resourceIndex = (unsigned char)resourceIndex;
        nextNode->packet = LoadCachedCutsceneResource(
            FindActiveCutsceneFileResources(
                g_pCutsceneMusicResources_004928bc),
            resourceIndex, 2);
    } else {
        FatalCutsceneError("A7");
    }
}

/* Function start: 0x42CFDB */
void ReleaseCutsceneMusicNodes(CutsceneMusicNode **head,
                               short resourceIndex)
{
    CutsceneMusicNode *node;
    CutsceneMusicNode *previous;

    node = *head;
    if (g_nMusicDriverMode_0049be8c != 1 &&
        g_nMusicDriverMode_0049be8c != 2)
        return;
    if (HasCutsceneMusicNode(node) == 0)
        return;
    while (HasCutsceneMusicNode(node->next) != 0)
        node = node->next;
    *head = node;
    while (HasCutsceneMusicNode(node) != 0) {
        previous = node->previous;
        if (node->resourceIndex == resourceIndex ||
            resourceIndex == 0xff) {
            if (HasCutsceneMusicNode(previous) != 0)
                previous->next = node->next;
            if (HasCutsceneMusicNode(node->next) != 0)
                node->next->previous = previous;
            if (*head == node)
                *head = previous;
            ReleaseLoadedCutsceneResource(
                FindActiveCutsceneFileResources(
                    g_pCutsceneMusicResources_004928bc),
                node->resourceIndex);
            free(node);
            if (g_pMemoryLogFile_00499da8 != 0) {
                fprintf(g_pMemoryLogFile_00499da8,
                        "Free Music: %p free: %d\n",
                        node, (int)(short)GetOriginalFreeMemory());
            }
            node = 0;
            if (resourceIndex != 0xff)
                return;
        }
        node = previous;
    }
}

/* Function start: 0x42D181 */
void *FindLoadedCutsceneMusic(short resourceIndex)
{
    CutsceneMusicNode *node;

    node = g_pCutsceneMusicPackets_00499c34;
    if (HasCutsceneMusicNode(node) == 0)
        return 0;
    while (HasCutsceneMusicNode(node->next) != 0)
        node = node->next;
    while (HasCutsceneMusicNode(node) != 0) {
        if (node->resourceIndex == resourceIndex)
            return node->packet;
        node = node->previous;
    }
    return 0;
}

/* Function start: 0x42D227 */
void InitializeCutsceneRuntimeResources(void)
{
    if (g_nCutsceneResourceLevel_00499d98 == 0) {
        if (g_bAutopilotDebugEnabled_00499bfc != 0 &&
            g_bSpeechEnabled_005c8de8 == 0)
            g_pMemoryLogFile_00499da8 = fopen("logger.$$$", "w");
        g_bCutsceneDrawingEnabled_00499c60 = 0;
        g_bMemoryLogToFile_00499bf8 = 1;
        WriteDetailedMemoryStateReport();
        g_bMemoryLogToFile_00499bf8 = 0;
        InitializeCinematicTextRenderer();
        g_nCutsceneFrameDelay_00499c8c = g_nCutsceneFrameTick_00499c88 = 0;
        g_nCutsceneInitialAvailableMemory_005d2e88 =
            GetAvailableMainMemory();
        g_nCutsceneInitialLargestBlock_005d2ea8 =
            GetLargestMainMemoryBlock();
        /* The retail image sets and immediately clears this flag. */
        g_bCutsceneCockpitLoaded_005d2d66 = 1;
        g_bCutsceneCockpitLoaded_005d2d66 = 0;
        if (g_apSceneObjects_00499c38 == 0) {
            g_apSceneObjects_00499c38 = AllocateScenePointerTable(
                0x80, HOST_POINTER_SIZE, 0,
                "Cannot alloc SpritesArray");
        }
        if (g_apCutscenePlanes_00499c3c == 0) {
            g_apCutscenePlanes_00499c3c = AllocateScenePointerTable(
                0x40, HOST_POINTER_SIZE, 0,
                "Cannot alloc PlanesArray");
        }
        if (g_apCutsceneSequences_00499c40 == 0) {
            g_apCutsceneSequences_00499c40 = AllocateScenePointerTable(
                0x100, HOST_POINTER_SIZE, 0,
                "Cannot alloc SequenceArray");
        }
        if (g_apCutsceneScenes_00499c44 == 0) {
            g_apCutsceneScenes_00499c44 = AllocateScenePointerTable(
                0x20, HOST_POINTER_SIZE, 0,
                "Cannot alloc ScriptArray");
        }
        if (g_pszCutsceneWorkBuffer_005d2ecc == 0) {
            WriteDetailedMemoryStateReport();
            g_pszCutsceneWorkBuffer_005d2ecc = calloc(0x100, 1);
            if (g_pszCutsceneWorkBuffer_005d2ecc == 0)
                FatalCutsceneError("A8");
            if (g_pMemoryLogFile_00499da8 != 0)
                fprintf(g_pMemoryLogFile_00499da8,
                        "Alloc ___workstring: %p free: %d\n",
                        g_pszCutsceneWorkBuffer_005d2ecc,
                        (int)GetOriginalFreeMemory());
        }
        if (g_pszCutscenePrintBuffer_005d2f10 == 0) {
            WriteDetailedMemoryStateReport();
            g_pszCutscenePrintBuffer_005d2f10 = calloc(0x100, 1);
            if (g_pszCutscenePrintBuffer_005d2f10 == 0)
                FatalCutsceneError("A9");
            if (g_pMemoryLogFile_00499da8 != 0)
                fprintf(g_pMemoryLogFile_00499da8,
                        "Alloc ___printstring: %p free: %d\n",
                        g_pszCutscenePrintBuffer_005d2f10,
                        (int)GetOriginalFreeMemory());
        }
    }
}

/* Function start: 0x42D444 */
void InitializeCutsceneViewports(void)
{
    if (g_stSceneFlicScratchViewport_005d2eb0.pixels == 0) {
        g_stCutsceneTextViewport_005d2d90 =
            g_stModalSourceViewport_005d2c50;
        g_stSceneFlicScratchViewport_005d2eb0 =
            g_stCutsceneTextViewport_005d2d90;
    }
    if (g_stCutsceneTextContext_005d2f40.viewport == 0) {
        g_stCutsceneTextContext_005d2f40.viewport =
            &g_stCutsceneTextBackingViewport_005d2db0;
        g_stCutsceneTextContext_005d2f40.text =
            g_szTextScratchBuffer_005d1c40;
        g_stCutsceneTextContext_005d2f40.colour = 15;
        g_stCutsceneTextContext_005d2f40.backgroundColour = 0;
        g_stCutsceneTextContext_005d2f40.alignment = 2;
        SetTextContext(&g_stCutsceneTextContext_005d2f40);
    }
}

/* Function start: 0x42D4C1 */
void ReleaseSceneMusicPacket(void)
{
    if (g_pSceneMusicPacket_00499c08 != 0) {
        StopMusic(0);
        g_dwRestoredSceneMusicState_005d14e8 =
            g_dwSceneMusicState_005d2dc4;
        ReleasePacketSlot(&g_pSceneMusicPacket_00499c08);
    }
}

/* Function start: 0x42D4FA */
void ReleaseCutsceneSpeechPackets(void)
{
    short index;

    for (index = 0; index < 8; index++) {
        ReleasePacketSlot(&g_apCutsceneSpeechPackets_005d2f80[index]);
        g_apszCutsceneSpeechFiles_005d2ee0[index] = 0;
        g_asCutsceneSpeechSections_005d2dd0[index] = 0;
        g_asCutsceneSpeechChannels_005d2d70[index] = 0;
    }
}

/* Function start: 0x42D568 */
void RunLoadedCutscene(void)
{
    TextContext *savedTextContext;
    short savedInputPollPeriod;
    short previousMemoryStatus;

#ifdef SDL_PORT
    /* None of these are reset by the previous cutscene's own teardown
     * (ReleaseCutsceneSpeechPackets only clears the precache slot
     * arrays), so a cutscene torn down while its last line was still
     * speaking leaves them stale here. ServiceSoundSystem (sound.c) keys
     * its whole grace-period/mouth-reset/force-advance block on
     * g_pSpeechSound_004a2658 alone being non-null -- it does not care
     * whether THIS cutscene has any speech of its own -- so a stale
     * pointer makes that block fight this cutscene's own (audio-less)
     * mouth animation and force-advance its lines early. Dropping the
     * reference here (not stopping or releasing the sound itself, which
     * risks touching an already-freed object -- see the delete-on-stop
     * comment in ServiceSoundSystem) is enough: if it is still genuinely
     * playing, it finishes on its own and self-deletes via
     * ix_sound_set_delete_on_stop, just no longer tracked here. Also
     * done unconditionally in stop_all_sounds (sound.c) now, but that is
     * not reliably called between every cutscene, so this stays too. Port
     * only, like the mouth-gating logic it protects -- the original never
     * reset any of this here. */
    g_pSpeechSound_004a2658 = 0;
    g_bSpeechSoundActive_004a2660 = 0;
    g_nSpeechCompletionDelay_004a265c = 0;
    g_bCutsceneSpeechAudioSeen = 0;
#endif
    savedTextContext = g_pCurrentTextContext_005c8d1c;
    savedInputPollPeriod = g_nInputPollPeriod_0049d6d8;
    previousMemoryStatus = g_cShowMemoryStatus;
    g_cShowMemoryStatus = 1;
    FlushSoundEffects();
    g_nInputPollPeriod_0049d6d8 = 1;
    g_cCutsceneVideoMode_00499c48 =
        (signed char)QueryCurrentGraphicsMode();
    ClearCutsceneViewport(&g_stModalSourceViewport_005d2c50, 0);
    g_stCutsceneTextContext_005d2f40.viewport = 0;
    g_nCutsceneMemoryDelta_005d2ec8 =
        (int)(g_nCutsceneInitialAvailableMemory_005d2e88 -
              GetAvailableMainMemory());
    g_bCutsceneSkipAll_00499c58 = 0;
    g_bMemoryLogToFile_00499bf8 = 1;
    WriteDetailedMemoryStateReport();
    g_bMemoryLogToFile_00499bf8 = 0;
    if (g_pMemoryLogFile_00499da8 != 0) {
        fprintf(g_pMemoryLogFile_00499da8,
                "************************************\n"
                "Initial %ld Overhead %ld\n",
                g_nCutsceneInitialAvailableMemory_005d2e88,
                g_nCutsceneMemoryDelta_005d2ec8);
    }
    ReleaseSceneMusicPacket();
    ExecuteCutsceneScene(g_apCutsceneScenes_00499c44[
        *FindActiveCutsceneObjectResources(
            g_pCutsceneSceneResources_00492898)->scriptSymbolIndices]);
    g_bCutsceneSkipAll_00499c58 = 0;
    g_bSceneEscapeRequested_0049d4b0 = 0;
    ReleaseCutsceneSpeechPackets();
    FlushSoundEffects();
    ReleaseSceneFlicPackets();
    ReleaseCutsceneResourceLevel(g_nCutsceneResourceLevel_00499d98);
    ReleasePacketSlot((void **)&g_apSceneObjects_00499c38);
    ReleasePacketSlot((void **)&g_apCutscenePlanes_00499c3c);
    ReleasePacketSlot((void **)&g_apCutsceneSequences_00499c40);
    ReleasePacketSlot((void **)&g_apCutsceneScenes_00499c44);
    free(g_pszCutsceneWorkBuffer_005d2ecc);
    if (g_pMemoryLogFile_00499da8 != 0) {
        fprintf(g_pMemoryLogFile_00499da8,
                "Free ___workstring %p free: %d\n",
                g_pszCutsceneWorkBuffer_005d2ecc,
                (int)(short)GetOriginalFreeMemory());
    }
    free(g_pszCutscenePrintBuffer_005d2f10);
    if (g_pMemoryLogFile_00499da8 != 0) {
        fprintf(g_pMemoryLogFile_00499da8,
                "Free ___printstring %p free: %d\n",
                g_pszCutscenePrintBuffer_005d2f10,
                (int)(short)GetOriginalFreeMemory());
    }
    g_pszCutsceneWorkBuffer_005d2ecc = 0;
    g_pszCutscenePrintBuffer_005d2f10 = 0;
    g_pszCutsceneFormattedText_005d2dc8 = 0;
    ClearActiveCutscenePixelAlias();
    ReleaseCutsceneViewport(&g_stCutsceneTextBackingViewport_005d2db0);
    ReleaseCutsceneViewport(&g_stSecondaryViewBuffer_005d2c90);
    if (g_bCutsceneCockpitLoaded_005d2d66 != 0) {
        ReleasePacketSlot(&g_pCutsceneCockpitPalette_00499c0c);
        g_pActiveCutscenePixels_005c83dc = 0;
    }
    g_nInputPollPeriod_0049d6d8 = savedInputPollPeriod;
    g_pCurrentTextContext_005c8d1c = savedTextContext;
    ReleasePacketSlot(&g_pCutsceneCockpitPacket_00499c04);
    if (g_pSceneMusicPacket_00499c08 != 0) {
    }
    StopMusic(0);
    ReleaseCutsceneFileResource(
        0, &g_pCutsceneMusicResources_004928bc);
    g_pSceneMusicPacket_00499c08 = 0;
    ReleaseCutsceneSoundEffects(-1);
    g_bMemoryLogToFile_00499bf8 = 1;
    WriteDetailedMemoryStateReport();
    g_bMemoryLogToFile_00499bf8 = 0;
    if (g_pMemoryLogFile_00499da8 != 0)
        fclose(g_pMemoryLogFile_00499da8);
    g_cShowMemoryStatus = (signed char)previousMemoryStatus;
}

/* Function start: 0x42D81C */
void ExecuteCutsceneScene(CutsceneScene *scene)
{
    SceneFlicObject *savedSprite;
    CutscenePlane *savedPlane;
    short savedOwner;

    savedOwner = g_nActiveCutsceneResourceLevel_00499d9c;
    savedSprite = g_pCurrentCutsceneSprite_00499c78;
    savedPlane = g_pCurrentCutscenePlane_00499c7c;
    if (scene == 0)
        FatalCutsceneError("Cannot execute a null cutscene scene");
    InitializeCutsceneViewports();
    g_nActiveCutsceneResourceLevel_00499d9c = scene->owner;
    PumpWindowMessages(0);
    if (g_bCutsceneSkipFrame_00499c54 == 0 &&
        g_bCutsceneViewportPreallocated_00499c4c == 0) {
        g_nNextCutsceneFrameClock_00499c90 =
            g_nInputClock_005c84a8 + g_nCutsceneFrameDelay_00499c8c;
    }
    if (g_bCutsceneSkipAll_00499c58 == 0)
        RunCutsceneScript(&scene->scriptCursor, 3);
    g_pCurrentCutsceneSprite_00499c78 = savedSprite;
    g_pCurrentCutscenePlane_00499c7c = savedPlane;
    g_nActiveCutsceneResourceLevel_00499d9c = savedOwner;
}

/* Function start: 0x42D8E2 */
void CopyCutsceneSpriteDisplay(unsigned char destination,
                               unsigned char source)
{
    SceneFlicObject *target;
    SceneFlicObject *sourceSprite;

    target = g_apSceneObjects_00499c38[
        FindActiveCutsceneObjectResources(
            g_pCutsceneSpriteResources_0049288c)
            ->scriptSymbolIndices[destination]];
    sourceSprite = g_apSceneObjects_00499c38[
        FindActiveCutsceneObjectResources(
            g_pCutsceneSpriteResources_0049288c)
            ->scriptSymbolIndices[source]];
    ReleaseCutsceneSpriteShape(sourceSprite);
    sourceSprite->drawType = target->drawType;
    sourceSprite->shape = target->shape;
    sourceSprite->baseFrame = target->baseFrame;
    sourceSprite->finalFrame = target->finalFrame;
    sourceSprite->currentFrame = target->currentFrame;
}

/* Function start: 0x42D98C */
void LinkCutsceneSpriteScript(unsigned char destination,
                              unsigned char source)
{
    SceneFlicObject *target;
    SceneFlicObject *sourceSprite;

    target = g_apSceneObjects_00499c38[
        FindActiveCutsceneObjectResources(
            g_pCutsceneSpriteResources_0049288c)
            ->scriptSymbolIndices[destination]];
    sourceSprite = g_apSceneObjects_00499c38[
        FindActiveCutsceneObjectResources(
            g_pCutsceneSpriteResources_0049288c)
            ->scriptSymbolIndices[source]];
    sourceSprite->linkedOwner = sourceSprite->owner;
    sourceSprite->linkedScript = sourceSprite->scriptStart;
    sourceSprite->scriptCursor = target->scriptStart;
    sourceSprite->scriptStart = sourceSprite->scriptCursor;
    sourceSprite->owner = g_nActiveCutsceneResourceLevel_00499d9c;
}

/* Function start: 0x42DA25 */
void DrawCutsceneSprite(SceneFlicObject *sprite)
{
    short y;
    short flags;
    short x;
    short rotation;
    short scale;

    if (sprite == 0)
        FatalCutsceneError("A11");
    if (g_bCutsceneDrawingEnabled_00499c60 == 0 &&
        g_bCutsceneSkipFrame_00499c54 != 0)
        return;
    if (sprite->active != 0 && sprite->visible != 0 &&
        sprite->shape != 0 && g_bCutsceneViewportPreallocated_00499c4c == 0) {
        x = sprite->x;
        y = sprite->y;
        rotation = sprite->rotation;
        scale = sprite->scale;
        flags = sprite->drawFlags;
        if (g_pCutsceneSpriteParentPlane_00499ef0 != 0) {
            x = (short)(x + g_pCutsceneSpriteParentPlane_00499ef0->x);
            y = (short)(y + g_pCutsceneSpriteParentPlane_00499ef0->y);
            rotation = (short)(rotation +
                g_pCutsceneSpriteParentPlane_00499ef0->rotation);
            scale = (short)(scale +
                g_pCutsceneSpriteParentPlane_00499ef0->scale);
            flags ^= g_pCutsceneSpriteParentPlane_00499ef0->drawFlags;
        }
        rotation = (short)(rotation % 360);
        if (rotation < 0)
            rotation = (short)(rotation + 360);
        if (scale < 0)
            scale = 0;
        if (scale > 0x4000)
            scale = 0x4000;
        if ((flags & 1) != 0) {
            if ((flags & 0x10) != 0) {
                x = (short)(160 - x);
                x = (short)(x + 160);
            }
            if ((flags & 0x20) != 0) {
                y = (short)(100 - y);
                y = (short)(y + 100);
            }
            flags &= 0xfffe;
        }
        g_nCutsceneSpritesDrawn_00499c10++;
        g_pCutscenePalettePacket_005d2e98 = 0;
        switch (sprite->drawType) {
        case 0:
        case 2:
            if (g_nFrameSkipCountdown_0049d760 == 0) {
                DrawSpriteScaled(&g_stSecondaryViewBuffer_005d2c90,
                    x, y, sprite->shape, sprite->baseFrame,
                    rotation, scale, flags);
                DrawSpriteScaled(&g_stSecondaryViewBuffer_005d2c90,
                    x, y, sprite->shape,
                    (short)(sprite->baseFrame + sprite->currentFrame + 1),
                    rotation, scale, flags);
            }
            sprite->tick++;
            break;
        case 1:
        case 3:
            if (g_nFrameSkipCountdown_0049d760 == 0) {
                DrawSpriteScaled(&g_stSecondaryViewBuffer_005d2c90,
                    x, y, sprite->shape,
                    (short)(sprite->baseFrame + sprite->currentFrame),
                    rotation, scale, flags);
            }
            sprite->tick++;
            break;
        case 4:
            if (g_nFrameSkipCountdown_0049d760 == 0) {
                DrawSpriteScaled(&g_stSecondaryViewBuffer_005d2c90,
                    x, y, sprite->shape,
                    (short)(sprite->currentFrame -
                            sprite->segmentStartFrame),
                    rotation, scale, flags);
            }
            sprite->currentFrame++;
            g_pCutscenePalettePacket_005d2e98 =
                (void *)sprite->decoderState;
            AdvanceSceneFlicStream(sprite);
            break;
        }
    }
}

/* Function start: 0x42DD9F */
void DrawCutscenePlane(CutscenePlane *plane)
{
    short index;

    if (plane == 0)
        FatalCutsceneError("Cannot draw a null cutscene plane");
    if (g_bCutsceneDrawingEnabled_00499c60 == 0 &&
        g_bCutsceneSkipFrame_00499c54 != 0)
        return;
    if (plane->active != 0 && plane->visible != 0 &&
        g_bCutsceneViewportPreallocated_00499c4c == 0) {
        g_pCutsceneSpriteParentPlane_00499ef0 = plane;
        for (index = 0; index < plane->spriteCount; index++) {
            DrawCutsceneSprite(g_apSceneObjects_00499c38[
                plane->spriteIndices[index]]);
        }
    }
}

/* Function start: 0x42DE62 */
void ClearCutsceneViewport(Viewport *viewport, unsigned char colour)
{
    if (g_cCutsceneVideoMode_00499c48 == 13)
        colour = g_abCutscenePaletteTranslation_00499c98[colour];
    ClearViewport(viewport, colour);
}

/* Function start: 0x42DE9D */
void RestoreCutsceneTextBacking(void)
{
    Viewport destination;
    short height;

    if (g_bCutsceneTextRestorePending_00499da0 != 0) {
        destination = g_stCutsceneTextViewport_005d2d90;
        height = (short)(g_stCutsceneTextViewport_005d2d90.bottom -
                         g_stCutsceneTextViewport_005d2d90.top + 1);
        destination.top =
            (short)((height - g_pCurrentTextContext_005c8d1c->cursorY) / 4 +
                    g_stCutsceneTextViewport_005d2d90.top);
        ClearCutsceneViewport(&g_stCutsceneTextViewport_005d2d90,
            g_pCurrentTextContext_005c8d1c->backgroundColour);
        CopyViewportContents(
            &g_stCutsceneTextBackingViewport_005d2db0, &destination);
        g_bCutsceneTextRestorePending_00499da0 = 0;
    }
}

/* Function start: 0x42DF32 */
void ClearCutsceneTextViewport(void)
{
    ClearCutsceneViewport(g_pCurrentTextContext_005c8d1c->viewport,
        g_pCurrentTextContext_005c8d1c->backgroundColour);
    g_pCurrentTextContext_005c8d1c->cursorX =
        g_pCurrentTextContext_005c8d1c->viewport->left;
    g_pCurrentTextContext_005c8d1c->cursorY =
        g_pCurrentTextContext_005c8d1c->viewport->top;
}

/* Function start: 0x42DF80 */
void ExpandCutsceneText(const unsigned char *source, char *destination)
{
    char character;
    char *out;
    unsigned int number[4];

    out = destination;
    if (out == 0)
        return;
    DosMemset(out, 0x100, 0);
    while ((character = *source++) != 0) {
        if ((unsigned char)character == 0x40) {
            itoa(g_nCutsceneFormatValue_005d2f0c, (char *)number, 10);
            out = CopyStringAndReturnEnd(out, (char *)number);
        } else if ((unsigned char)character >= 0x85) {
            switch ((unsigned char)character) {
            case 0x85:
                out = CopyStringAndReturnEnd(
                    out, g_stPilotNameVariables_00499ef8.lastName);
                break;
            case 0x86:
                out = CopyStringAndReturnEnd(
                    out, g_stPilotNameVariables_00499ef8.callsign);
                break;
            case 0x89:
                out = CopyStringAndReturnEnd(
                    out, g_stPilotNameVariables_00499ef8.firstName);
                break;
            case 0x87:
                itoa(*((short *)g_pCampaignGlobals_00499c94 + 54),
                     (char *)number, 10);
                out = CopyStringAndReturnEnd(out, (char *)number);
                break;
            case 0x88:
                itoa(*((short *)g_pCampaignGlobals_00499c94 + 55),
                     (char *)number, 10);
                out = CopyStringAndReturnEnd(out, (char *)number);
                break;
            default:
                out++;
                break;
            }
        } else {
            *out++ = character;
        }
    }
}

/* Function start: 0x42E12A */
void ExecuteCutsceneSequence(CutsceneSequence *sequence,
                             unsigned char *text, signed char draw)
{
    short savedOwner;
    short planeIndex;
    CutsceneSequence *savedSequence;
    signed char continueSequence;
    signed char textShown;

    textShown = 0;
    savedOwner = g_nActiveCutsceneResourceLevel_00499d9c;
    savedSequence = g_pCurrentCutsceneSequence_00499c80;
    if (sequence == 0)
        FatalCutsceneError("A13");
    g_nActiveCutsceneResourceLevel_00499d9c = sequence->owner;
    g_pCurrentCutsceneSequence_00499c80 = sequence;
    g_bCutsceneFramePresented_005d2de0 = 0;
    do {
        PumpWindowMessages(0);
        if (textShown == 0 && text != 0) {
            textShown++;
            ClearCutsceneTextViewport();
            ExpandCutsceneText(text, g_pszCutsceneWorkBuffer_005d2ecc);
#ifdef SDL_PORT
            if (memcmp(g_pszCutsceneWorkBuffer_005d2ecc,
                       "50", 2) != 0) {
#else
            if ((IsCutsceneSpeechLoaded() == 0 ||
                 g_nAudioEnabled_0049c244 == 0) &&
                memcmp(g_pszCutsceneWorkBuffer_005d2ecc, "50", 2) != 0) {
#endif
                FormatTextBufferFromStart(
                    "%s%P", g_pszCutsceneWorkBuffer_005d2ecc);
            }
            g_pszCurrentCutsceneText_00499da4 =
                g_pszCutsceneWorkBuffer_005d2ecc;
            g_bCutsceneTextRestorePending_00499da0 = 1;
        }
        while (g_bCutsceneSkipFrame_00499c54 == 0 &&
               (int)g_nInputClock_005c84a8 <
                   (int)g_nNextCutsceneFrameClock_00499c90) {
            PumpWindowMessages(0);
#ifdef SDL_PORT
            /* See issue #33 (review on PR #34): this sequence-presentation
             * wait is purely clock-driven -- no shared/pending resource is
             * involved, so unlike the speech-cache waits elsewhere there is
             * nothing to leave in an inconsistent state by bailing out
             * early. */
            if (g_bSceneEscapeRequested_0049d4b0 != 0)
                break;
#endif
        }
        g_nFrameSkipCountdown_0049d760 = 0;
        continueSequence = RunCutsceneScript(&sequence->scriptCursor, 2);
        g_pCurrentCutsceneSequence_00499c80 = sequence;
        if (draw != 0) {
            if (continueSequence == 0 || sequence->planeCount == 0 ||
                g_bCutsceneFramePresented_005d2de0 != 0) {
                g_bCutsceneFramePresented_005d2de0 = 0;
            } else {
                for (planeIndex = 0; planeIndex < sequence->planeCount;
                     planeIndex++) {
                    UpdateCutscenePlaneObject(
                        g_apCutscenePlanes_00499c3c[
                            sequence->planeIndices[planeIndex]], 1);
                }
                for (planeIndex = 0; planeIndex < sequence->planeCount;
                     planeIndex++) {
                    DrawCutscenePlane(g_apCutscenePlanes_00499c3c[
                        sequence->planeIndices[planeIndex]]);
                }
                if (g_bCutsceneSkipFrame_00499c54 == 0 &&
                    g_bCutsceneViewportPreallocated_00499c4c == 0) {
                    ConsumeCutscenePalettePacket(0, 0x100, 1);
                    PresentCutsceneFrame(
                        &g_stSecondaryViewBuffer_005d2c90,
                        &g_stSceneFlicScratchViewport_005d2eb0);
                    RestoreCutsceneTextBacking();
                    if (g_nCutsceneFrameDelay_00499c8c != 0) {
                        g_nNextCutsceneFrameClock_00499c90 =
                            g_nInputClock_005c84a8 +
                            g_nCutsceneFrameDelay_00499c8c;
                    }
                }
            }
        } else {
            continueSequence = 0;
        }
    } while (continueSequence != 0);
    g_pCurrentCutsceneSequence_00499c80 = savedSequence;
    g_nActiveCutsceneResourceLevel_00499d9c = savedOwner;
}

/* Function start: 0x42E3CF */
void UpdateCutscenePlaneObject(CutscenePlane *plane,
                               signed char updateChildren)
{
    CutscenePlane *savedPlane;
    short savedOwner;
    short spriteIndex;
    signed char waiting;

    savedOwner = g_nActiveCutsceneResourceLevel_00499d9c;
    savedPlane = g_pCurrentCutscenePlane_00499c7c;
    waiting = 0;
    if (plane == 0)
        FatalCutsceneError("Cannot update a null cutscene plane");
    if (plane->waitTicks != 0) {
        if (g_bCutsceneViewportPreallocated_00499c4c == 0 &&
            g_bCutsceneSkipFrame_00499c54 == 0 &&
            g_nInputClock_005c84a8 < plane->waitStart + plane->waitTicks) {
            waiting = 1;
        } else {
            plane->waitTicks = 0;
        }
    }
    g_pCurrentCutscenePlane_00499c7c = plane;
    g_nActiveCutsceneResourceLevel_00499d9c = plane->owner;
    PumpWindowMessages(0);
    if (waiting == 0)
        RunCutsceneScript(&plane->scriptCursor, 1);
    if (updateChildren != 0) {
        for (spriteIndex = 0; spriteIndex < plane->spriteCount;
             spriteIndex++) {
            UpdateCutsceneSpriteObject(g_apSceneObjects_00499c38[
                plane->spriteIndices[spriteIndex]]);
        }
        plane->x = (short)(plane->x + plane->deltaX);
        plane->y = (short)(plane->y + plane->deltaY);
        plane->depth = (short)(plane->depth + plane->deltaDepth);
    }
    g_pCurrentCutscenePlane_00499c7c = savedPlane;
    g_nActiveCutsceneResourceLevel_00499d9c = savedOwner;
}

/* Function start: 0x42E553 */
void UpdateCutsceneSpriteObject(SceneFlicObject *sprite)
{
    SceneFlicObject *savedSprite;
    signed char waiting;

    waiting = 0;
    if (sprite == 0)
        FatalCutsceneError("A15");
    if (sprite->delayFrames != 0) {
        sprite->delayFrames--;
        waiting++;
    }
    if (sprite->waitTicks != 0) {
        if (g_bCutsceneViewportPreallocated_00499c4c == 0 &&
            g_bCutsceneSkipFrame_00499c54 == 0 &&
            g_nInputClock_005c84a8 < sprite->waitStart + sprite->waitTicks) {
            waiting++;
        } else {
            sprite->waitTicks = 0;
        }
    }
    if (waiting == 0) {
        PumpWindowMessages(0);
        savedSprite = g_pCurrentCutsceneSprite_00499c78;
        g_pCurrentCutsceneSprite_00499c78 = sprite;
        if (sprite->field_3 >= 0)
            DispatchCutsceneSpriteHandler(sprite, 0);
        RunCutsceneScript(&sprite->scriptCursor, 0);
        sprite->x = (short)(sprite->x + sprite->deltaX);
        sprite->y = (short)(sprite->y + sprite->deltaY);
        sprite->field_2f = (short)(sprite->field_2f + sprite->deltaDepth);
        g_pCurrentCutsceneSprite_00499c78 = savedSprite;
    }
}

/* Function start: 0x42E692 */
void DispatchCutsceneSpriteHandler(SceneFlicObject *sprite,
                                   short handlerType)
{
    SceneFlicObject *object;
    void (*handler)(SceneFlicObject *sprite);

    switch (handlerType) {
    case 0:
        object = sprite;
        handler = g_apCutsceneSpriteHandlers_00499c5c[object->field_3];
        handler(object);
        break;
    case 1:
        break;
    case 2:
        break;
    }
}

void DispatchCutsceneSpriteHandler(SceneFlicObject *sprite, short handlerType);
char *GetPackedStringByIndex(CutsceneResourceTable *resources, short index);

/* Function start: 0x42E6EB */
void InitializeSceneFlicStream(CutsceneResourceTable *resources,
                               short index,
                               SceneFlicObject *object)
{
    object->filename = GetPackedStringByIndex(
        resources, resources->filenameIndices[index]);
    object->nextSection = resources->sectionIndices[index];
    object->segmentStartFrame = 0;
    object->segmentEndFrame = 0;
    object->currentFrame = 0;
    object->context = g_nSceneFlicContext_00499c50;
    AdvanceSceneFlicStream(object);
}

/* Function start: 0x42E762 */
void ReleaseSceneFlicPackets(void)
{
    short cacheIndex;
    short objectIndex;
    SceneFlicObject *object;

    for (cacheIndex = 0; cacheIndex < 15U; cacheIndex++) {
        if (g_aSceneFlicCache_005d2e10[cacheIndex].shape != 0) {
            for (objectIndex = 0; objectIndex < 128; objectIndex++) {
                object = g_apSceneObjects_00499c38[objectIndex];
                if (object != 0 && object->drawType == 4 &&
                    g_aSceneFlicCache_005d2e10[cacheIndex].shape ==
                        object->shape) {
                    object->shape = 0;
                    object->active = 0;
                    break;
                }
            }
        }
    }
    for (cacheIndex = 0; cacheIndex < 15U; cacheIndex++) {
        ReleasePacketSlot(
            (void **)&g_aSceneFlicCache_005d2e10[cacheIndex].shape);
        ReleasePacketSlot(
            &g_aSceneFlicCache_005d2e10[cacheIndex].auxiliaryAllocation);
    }
}

/* Function start: 0x42E868 */
void AdvanceSceneFlicStream(SceneFlicObject *object)
{
    signed char loading;
    char filename[68];
    short cacheIndex;
    short releaseIndex;
    void *oldShape;

    loading = 1;
    object->decoderState = 0;
    if (object->segmentEndFrame <= object->currentFrame) {
        if (object->finalFrame <= object->currentFrame) {
            object->active = 0;
            ReleaseSceneFlicPackets();
            object->shape = 0;
        } else {
            if (object->currentFrame != 0) {
                CopyViewportContents(&g_stSecondaryViewBuffer_005d2c90,
                                     &g_stSceneFlicScratchViewport_005d2eb0);
            }
            DosStrcpy(filename, object->filename);
            RewriteDiskFileGraphicsExtensions(filename);
            oldShape = object->shape;
            if (oldShape != 0) {
                for (cacheIndex = 0; cacheIndex < 15U; cacheIndex++) {
                    if (g_aSceneFlicCache_005d2e10[cacheIndex].shape ==
                        oldShape) {
                        ReleasePacketSlot(&oldShape);
                        g_aSceneFlicCache_005d2e10[cacheIndex].shape = 0;
                        oldShape =
                            g_aSceneFlicCache_005d2e10[cacheIndex].shape;
                        break;
                    }
                }
            }
            while (loading != 0) {
                if (g_aSceneFlicCache_005d2e10[0].shape == 0) {
                    object->shape = 0;
                    for (cacheIndex = 0; cacheIndex < 15U; cacheIndex++) {
                        g_aSceneFlicCache_005d2e10[cacheIndex]
                            .auxiliaryAllocation = 0;
                        g_aSceneFlicCache_005d2e10[cacheIndex].shape =
                            LoadNamedPacket(
                                filename,
                                (short)(object->nextSection + cacheIndex),
                                0, 0x0c, 0, 1);
                        if (g_aSceneFlicCache_005d2e10[cacheIndex].shape ==
                            0) {
                            if (cacheIndex == 0) {
                                OutputDebugPrintf(
                                    "FLIC Failed %s (%d)\n Hit Key to Continue",
                                    filename,
                                    object->nextSection + cacheIndex);
                                WaitForCutsceneInputEvent();
                                object->segmentEndFrame = 0;
                                object->currentFrame = object->finalFrame;
                            }
                            break;
                        }
                        if (g_pMemoryLogFile_00499da8 != 0) {
                            fprintf(
                                g_pMemoryLogFile_00499da8,
                                "Loaded Flic %s (%d) at %Fp\n", filename,
                                object->nextSection + cacheIndex,
                                g_aSceneFlicCache_005d2e10[cacheIndex].shape);
                        }
                    }
                    loading = 0;
                }
                if (g_aSceneFlicCache_005d2e10[0].shape != 0) {
                    cacheIndex = 0;
                    if (object->shape != 0) {
                        while (cacheIndex < 15U) {
                            cacheIndex++;
                            if (g_aSceneFlicCache_005d2e10[cacheIndex - 1]
                                    .shape == object->shape)
                                break;
                        }
                    }
                    if (g_aSceneFlicCache_005d2e10[cacheIndex].shape != 0) {
                        object->shape =
                            g_aSceneFlicCache_005d2e10[cacheIndex].shape;
                        object->decoderState = 0;
                        g_aSceneFlicCache_005d2e10[cacheIndex]
                            .auxiliaryAllocation = 0;
                        object->segmentStartFrame = object->currentFrame;
                        object->segmentEndFrame =
                            (short)(GetShapeFrameCount(object->shape) +
                                    object->currentFrame);
                        object->nextSection++;
                        loading = 0;
                    } else {
                        for (releaseIndex = 0;
                             releaseIndex < cacheIndex;
                             releaseIndex++) {
                            ReleasePacketSlot(
                                (void **)&g_aSceneFlicCache_005d2e10[
                                    releaseIndex].shape);
                        }
                        object->shape = 0;
                    }
                }
            }
            CopyViewportContents(&g_stSceneFlicScratchViewport_005d2eb0,
                                 &g_stSecondaryViewBuffer_005d2c90);
        }
    }
}

/* Function start: 0x42EBD0 */
void ReleaseActiveCutscenePacket(CutsceneResourceTable *resources,
                                 void *packet)
{
    CutsceneResourceTable *activeResources;
    void **packetSlot;
    short resourceCount;
    short index;

    activeResources = FindActiveCutsceneFileResources(resources);
    resourceCount = activeResources->count;
    packetSlot = activeResources->loadedPackets;
    for (index = 0; index < resourceCount; index++, packetSlot++) {
        if (*packetSlot == packet) {
            ReleasePacketSlot(packetSlot);
            if (g_pMemoryLogFile_00499da8 != 0) {
                fprintf(g_pMemoryLogFile_00499da8,
                        "--- Dumped (reset): %Fp  item: %3d  "
                        "level = %d free = %8lu\n",
                        packet, (int)index,
                        (int)g_nActiveCutsceneResourceLevel_00499d9c,
                        GetAvailableMainMemory());
            }
            break;
        }
    }
}

/* Function start: 0x42EC7F */
void ReleaseCutsceneFontPacket(void)
{
    short savedOwner;

    savedOwner = g_nActiveCutsceneResourceLevel_00499d9c;
    g_nActiveCutsceneResourceLevel_00499d9c =
        g_nCutsceneFontOwner_005d2fa0;
    ReleaseActiveCutscenePacket(
        g_pCutsceneFontResources_004928a0,
        g_stCutsceneTextContext_005d2f40.font);
    g_stCutsceneTextContext_005d2f40.font = 0;
    g_nActiveCutsceneResourceLevel_00499d9c = savedOwner;
}

/* Function start: 0x42ECCB */
void UpdateMap(char *text, short objective)
{
    int deadline;

    ClearCutsceneViewport(&g_stModalSourceViewport_005d2c50, 0);
    g_bBriefingMapActive_0049bcb0 = 1;
    LoadMissionData(g_pCampaignGlobals_00499c94->series,
                    g_pCampaignGlobals_00499c94->mission);
    Build_objective_list();
    g_cCurrentObjective_004931cc = (signed char)objective;
    g_pActiveCutscenePixels_005c83dc = 0;
    BriefingMap_DisplayMap();
    g_bBriefingMapActive_0049bcb0 = 0;
    RouteCutsceneViewportToDisplay();
    SetTextContext(&g_stCutsceneTextContext_005d2f40);
    ClearCutsceneTextViewport();
    ExpandCutsceneText((const unsigned char *)text,
                       g_pszCutsceneWorkBuffer_005d2ecc);
#ifdef SDL_PORT
    if (memcmp(g_pszCutsceneWorkBuffer_005d2ecc, "50", 2) != 0)
        FormatTextBufferFromStart(
            "%s%P", g_pszCutsceneWorkBuffer_005d2ecc);
#else
    if (IsCutsceneSpeechLoaded() == 0 || g_nAudioEnabled_0049c244 == 0) {
        if (memcmp(g_pszCutsceneWorkBuffer_005d2ecc, "50", 2) != 0)
            FormatTextBufferFromStart("%s%P", g_pszCutsceneWorkBuffer_005d2ecc);
    }
#endif
    g_bCutsceneTextRestorePending_00499da0 = 1;
    RestoreCutsceneTextBacking();
    ServiceInputDevices(-1);
    deadline = g_nInputClock_005c84a8 + 600;
    while ((int)g_nInputClock_005c84a8 < deadline &&
           g_bCutsceneSkipFrame_00499c54 == 0 &&
           g_bCutsceneViewportPreallocated_00499c4c == 0) {
        if (ServiceInputDevices(-1) != 0) {
            ServiceCutsceneRuntimeHook();
            if (FindQueuedInputEvent(2) != 0 || FindQueuedInputEvent(5) != 0)
                break;
        }
    }
    ClearCutsceneViewport(&g_stModalSourceViewport_005d2c50, 0);
}

/* Function start: 0x42EE4F */
void AllocateCutsceneViewportOrFatal(Viewport *viewport, short colour,
                                     short flags)
{
    if ((short)AllocateViewport(viewport, colour, flags) == 0)
        FatalCutsceneError("A17");
}

/* Function start: 0x42EE86 */
void ReleaseCutsceneViewport(Viewport *viewport)
{
    signed char release;

    release = 1;
    if (viewport->pixels != 0) {
        switch (g_cCutsceneVideoMode_00499c48) {
        case 0x0d:
        case 0x13:
            if (GetDIBPixelBuffer() != viewport->pixels)
                release++;
            break;
        default:
            release++;
            break;
        }
        if (release != 0) {
            free_viewport(viewport);
            viewport->pixels = 0;
        }
    }
}

#pragma function(strcpy)
/* Function start: 0x42EF12 */
void DrawCinematicMemoryStatus(const char *message)
{
    unsigned int availableFarMemory;
    char text[256];
    Viewport viewport;
    unsigned int largestBlock;
    unsigned int availableMemory;

    viewport = g_stModalSourceViewport_005d2c50;
    if ((g_cCutsceneVideoMode_00499c48 == 0x13 ||
         g_cCutsceneVideoMode_00499c48 == 0x0d) &&
        g_cShowMemoryStatus != 0) {
        if (*message != '~') {
            PresentCutsceneFrame(
                &g_stCutsceneTextBackingViewport_005d2db0,
                &g_stCutsceneTextViewport_005d2d90);
            g_bCutsceneTextRestorePending_00499da0 = 1;
        }
        largestBlock = GetLargestMainMemoryBlock();
        availableMemory = GetAvailableMainMemory();
        availableFarMemory = GetAvailableFarMemory();
        if (*message != '~') {
            sprintf(text,
                    "%s Far: %lu (Lg) = %lu  EMS: %lu\n"
                    "OVR: %lu INITIAL: %lu",
                    message, availableMemory, largestBlock,
                    availableFarMemory,
                    g_nCutsceneMemoryDelta_005d2ec8,
                    g_nCutsceneInitialAvailableMemory_005d2e88);
        } else {
            strcpy(text, "Game Paused");
        }
        g_stCutsceneDrawingTextContext_005d2f60 =
            g_stCutsceneTextContext_005d2f40;
        g_stCutsceneDrawingTextContext_005d2f60.viewport =
            &g_stModalSourceViewport_005d2c50;
        SetTextContext(&g_stCutsceneDrawingTextContext_005d2f60);
        g_pCurrentTextContext_005c8d1c->cursorX =
            g_pCurrentTextContext_005c8d1c->viewport->left;
        g_pCurrentTextContext_005c8d1c->cursorY =
            g_pCurrentTextContext_005c8d1c->viewport->top;
        FormatTextBufferFromStart("%s%P", text);
        WaitForCutsceneInputEvent();
        g_bCutsceneSkipFrame_00499c54 = 0;
        SetTextContext(&g_stCutsceneTextContext_005d2f40);
        viewport.top = 0;
        viewport.bottom =
            (short)(g_stSceneFlicScratchViewport_005d2eb0.top - 1);
        ClearViewport(&viewport, 0);
    }
}

/* Function start: 0x42F0B3 */
void FormatCinematicMemoryStatus(const char *format, ...)
{
    va_list arguments;
    char message[256];

    va_start(arguments, format);
    vsprintf(message, format, arguments);
    va_end(arguments);
    DrawCinematicMemoryStatus(message);
}

/* Function start: 0x42F100 */
void ReportCutscenePacketAllocationFailure(const char *filename,
                                           short section,
                                           unsigned int packetSize)
{
    if (g_cCutsceneVideoMode_00499c48 == 0x13) {
        FormatCinematicMemoryStatus(
            "Alloc Failed %s (%d) size=%ld\n",
            filename, section, packetSize);
    }
}

/* Function start: 0x42F135 */
short PopCutsceneScriptValue(short **stack, short *stackBottom)
{
    int stackAddress;
    short *value;
    int bottomAddress;

    value = *stack;
    stackAddress = DosFarPtrToNear(value);
    bottomAddress = DosFarPtrToNear(stackBottom);
    if (bottomAddress <= stackAddress)
        FatalCutsceneError("A18");
    value++;
    *stack = value;
    return value[-1];
}

/* Function start: 0x42F19A */
void PushCutsceneScriptValue(short **stack, short *stackTop,
                             short value)
{
    int stackAddress;
    short *slot;
    int topAddress;

    slot = *stack;
    stackAddress = DosFarPtrToNear(slot);
    topAddress = DosFarPtrToNear(stackTop);
    if (topAddress >= stackAddress)
        FatalCutsceneError("A19");
    slot--;
    *slot = value;
    *stack = slot;
}

/* Function start: 0x42F1FD */
signed char RunCutsceneScript(unsigned char **scriptCursor,
                              short objectType)
{
    unsigned char *instruction;
    unsigned char opcode;
    unsigned char resourceIndex;
    unsigned char memberType;
    CutsceneObjectResourceList *objectResources;
    CutsceneResourceTable *fileResources;
    CutsceneResourceTable *savedFilmResources;
    SceneFlicObject *sprite;
    CutscenePlane *plane;
    CutsceneSequence *sequence;
    short stackStorage[10];
    short *stack;
    short value;
    short otherValue;
    short index;
    short runtimeIndex;
    short waitTicks;
    int inputHandled;
    int waitUntil;
    signed char returnValue;
    signed char branchGuard;
    void *packet;

    instruction = *scriptCursor;
    returnValue = 1;
    branchGuard = 1;
    stack = stackStorage + 10;
    inputHandled = 0;
    if (instruction == 0)
        FatalCutsceneError(
            "ERROR: Running script for UNLINKED EXTERNAL (%s)",
            g_apszCutsceneObjectTypeNames_00499c18[objectType]);
    if (g_pCurrentCutsceneSprite_00499c78 == 0) {
        g_pCurrentCutsceneSprite_00499c78 =
            g_apSceneObjects_00499c38[
                FindActiveCutsceneObjectResources(
                    g_pCutsceneSpriteResources_0049288c)->
                    scriptSymbolIndices[0]];
    }
    if (g_pCurrentCutscenePlane_00499c7c == 0 &&
        FindActiveCutsceneObjectResources(
            g_pCutscenePlaneResources_00492890)->
            scriptSymbolIndices != 0) {
        g_pCurrentCutscenePlane_00499c7c =
            g_apCutscenePlanes_00499c3c[
                FindActiveCutsceneObjectResources(
                    g_pCutscenePlaneResources_00492890)->
                    scriptSymbolIndices[0]];
    }
    if (g_pCurrentCutsceneSequence_00499c80 == 0 &&
        FindActiveCutsceneObjectResources(
            g_pCutsceneSequenceResources_00492894)->
            scriptSymbolIndices != 0) {
        g_pCurrentCutsceneSequence_00499c80 =
            g_apCutsceneSequences_00499c40[
                FindActiveCutsceneObjectResources(
                    g_pCutsceneSequenceResources_00492894)->
                    scriptSymbolIndices[0]];
    }
    for (;;) {
#ifdef SDL_PORT
        /* See issue #33 (ESC feels slow or dead during cutscenes).
         *
         * The original escape-to-skip conversion is the block further down
         * ("handle_queued_cutscene_input" and the TakeInputPressCount()
         * block below it).  Neither of them reads
         * g_bSceneEscapeRequested_0049d4b0 on its own: both first require a
         * still-queued input event (FindQueuedInputEvent(4)/(1)) or a
         * still-pending press count, and only then look at the escape flag.
         * The escape flag is a level that stays set until somebody converts
         * it, but the two things that gate it are one-shot and are routinely
         * destroyed before the conversion ever gets to look at them:
         *
         *   - opcode 0xa4 (cases 0 and 1, the script's own "stop skipping
         *     this line" reset) calls FlushPendingInputEvents(), which frees
         *     the whole queue including the Escape keydown event;
         *   - the non-escape keypress path below calls FlushInputEvents()
         *     for the same reason;
         *   - ServiceInputDevices() (eventmgr.c) itself calls
         *     FlushInputEvents() on its poll-tick path -- and returns 1 from
         *     that path, so the caller believes there is input to look at in
         *     a queue that was just emptied;
         *   - the press count is decremented again by the Escape KEYUP, so
         *     TakeInputPressCount() is back to 0 by the time the loop looks.
         *
         * On top of that both blocks sit inside "skipframe == 0", so while
         * the player is already skipping a line (skipframe == 1, which is
         * exactly when they are most likely to hit Escape) the conversion is
         * not even reached, and by the time the script clears skipframe at
         * the next 0xa4 that same 0xa4 has just flushed the queue.
         *
         * Net effect: Escape gets latched in the flag and then never
         * converted.  The scene does not end; it merely races (all the
         * script-timed waits key off the flag now, see 0x6b/0x7c below)
         * while still doing every load and decode, which is the "many
         * seconds, sometimes needs three presses" behaviour in the issue --
         * pressing Escape again just re-queues an event and re-races the
         * same lottery.
         *
         * Fix: read the level directly, at the top of the interpreter loop,
         * and run the ORIGINAL conversion body verbatim (same calls, same
         * flags, same "return 0", see the identical sequence below).
         *
         * Why this cannot get stuck the way the earlier "force skipframe=1"
         * attempt did:
         *   - it does not set a flag and keep looping.  It performs the
         *     complete transition and returns, so it executes at most once
         *     per RunCutsceneScript activation;
         *   - it clears g_bSceneEscapeRequested_0049d4b0 itself, so it
         *     cannot re-fire;
         *   - it cannot starve the original blocks below: when the flag is
         *     clear this is a no-op, and when it is set the original block
         *     would have produced exactly this state anyway;
         *   - the state it leaves behind (skipall = 1, skipframe = 1,
         *     escape = 0, return 0) is bit-for-bit the state a successful
         *     Escape already produces today, so all the existing unwinding
         *     (ExecuteCutsceneSequence's "while (continueSequence != 0)",
         *     ExecuteCutsceneScene's "if (skipall == 0)", the branch opcodes
         *     forcing value = 0 under skipall) applies unchanged.
         *
         * Still gated on g_bCutsceneViewportPreallocated_00499c4c == 0, and
         * that guard is load-bearing rather than cosmetic: preallocated == 1
         * marks the headless state-advance pass RunCampaignGameLoop
         * (brains.c) runs before the chalkboard, where drawing, audio,
         * palette work and nested scene playback are all suppressed.  The
         * player cannot see or hear that pass, so there is nothing there to
         * skip, and cutting it short with skipall would leave the campaign
         * globals it exists to compute half-written.  RunCampaignScript
         * (screen.c) clears the escape flag on the way out of that pass, so
         * a press made during it does not leak into the real cutscene. */
        if (g_bSceneEscapeRequested_0049d4b0 != 0 &&
            g_bCutsceneViewportPreallocated_00499c4c == 0) {
            SdlTracef("[cutscene] escape converted: objectType=%d "
                      "skipframe=%d skipall=%d clock=%d\n",
                      (int)objectType,
                      (int)g_bCutsceneSkipFrame_00499c54,
                      (int)g_bCutsceneSkipAll_00499c58,
                      (int)g_nInputClock_005c84a8);
            ReleaseSceneFlicPackets();
            ReleaseCutsceneSoundEffects(-1);
            ReleaseCutsceneSpeechPackets();
            g_bCutsceneSpeechActive_00499eb8 = 0;
            g_bCutsceneSkipFrame_00499c54 = 1;
            g_bCutsceneSkipAll_00499c58 = 1;
            g_bSceneEscapeRequested_0049d4b0 = 0;
            return 0;
        }
#endif
        if (g_bCutsceneSkipAll_00499c58 != 0)
            g_bCutsceneSkipFrame_00499c54 = 1;
        if (g_bCutsceneSkipFrame_00499c54 == 0 &&
            g_bCutsceneViewportPreallocated_00499c4c == 0) {
            if (ServiceInputDevices(-1) != 0) {
                ServiceCutsceneRuntimeHook();
                g_pCutsceneInputEvent_005d2f08 = FindQueuedInputEvent(4);
                if (g_pCutsceneInputEvent_005d2f08 != 0) {
                    switch (g_pCutsceneInputEvent_005d2f08->status) {
                    case 2:
                        g_cCutsceneSpeechSpeed_00499eb4 = 0;
                        break;
                    case 3:
                        g_cCutsceneSpeechSpeed_00499eb4 = 2;
                        break;
                    case 4:
                        g_cCutsceneSpeechSpeed_00499eb4 = 4;
                        break;
                    case 5:
                        g_cCutsceneSpeechSpeed_00499eb4 = 6;
                        break;
                    case 6:
                        g_cCutsceneSpeechSpeed_00499eb4 = 8;
                        break;
                    case 0x19:
                        WaitForCutsceneInputEvent();
                        if ((g_pCutsceneInputEvent_005d2f08->modifiers &
                             0x1800) == 0) {
                            DrawCinematicMemoryStatus("~");
                        } else {
                            WaitForCutsceneInputEvent();
                        }
                        break;
                    case 0x11:
                        WaitForCutsceneInputEvent();
                        if ((g_pCutsceneInputEvent_005d2f08->modifiers &
                             0x1800) != 0) {
                            FormatCinematicMemoryStatus(
                                "Series: %d Mission: %c Talk: %d",
                                g_pCampaignGlobals_00499c94->series,
                                g_pCampaignGlobals_00499c94->mission + 'A',
                                g_pCampaignGlobals_00499c94->field_08 + 1);
                        }
                        break;
                    default:
                        goto handle_queued_cutscene_input;
                    }
                } else {
handle_queued_cutscene_input:
                    if (g_wDigiPlaying_0049bb60 == 0 &&
                        (FindQueuedInputEvent(4) != 0 ||
                         FindQueuedInputEvent(1) != 0)) {
                        ReleaseSceneFlicPackets();
                        ReleaseCutsceneSoundEffects(-1);
                        if (g_bSceneEscapeRequested_0049d4b0 != 0) {
                            ReleaseCutsceneSpeechPackets();
                            g_bCutsceneSpeechActive_00499eb8 = 0;
                            g_bCutsceneSkipFrame_00499c54 = 1;
                            g_bCutsceneSkipAll_00499c58 = 1;
                            g_bSceneEscapeRequested_0049d4b0 = 0;
                            return 0;
                        }
                        g_bCutsceneTextAdvance_005d2ed0 = 0;
                        g_pszCutsceneSpeechCursor_00499eb0 = 0;
                        g_bCutsceneSkipFrame_00499c54 = 1;
                        g_nNextCutsceneFrameClock_00499c90 =
                            g_nInputClock_005c84a8;
                        inputHandled = 1;
                    }
                }
            }
            if (g_wDigiPlaying_0049bb60 == 0 &&
                TakeInputPressCount() != 0 && inputHandled == 0) {
                ReleaseSceneFlicPackets();
                ReleaseCutsceneSoundEffects(-1);
                if (g_bSceneEscapeRequested_0049d4b0 != 0) {
                    ReleaseCutsceneSpeechPackets();
                    g_bCutsceneSpeechActive_00499eb8 = 0;
                    g_bCutsceneSkipFrame_00499c54 = 1;
                    g_bCutsceneSkipAll_00499c58 = 1;
                    g_bSceneEscapeRequested_0049d4b0 = 0;
                    return 0;
                }
                g_bCutsceneTextAdvance_005d2ed0 = 0;
                g_pszCutsceneSpeechCursor_00499eb0 = 0;
                g_bCutsceneSkipFrame_00499c54 = 1;
                g_nNextCutsceneFrameClock_00499c90 =
                    g_nInputClock_005c84a8;
                FlushInputEvents();
            }
            if (objectType == 2 &&
                g_pCurrentCutsceneSequence_00499c80->waitTicks +
                    g_pCurrentCutsceneSequence_00499c80->waitStart <
                    (int)g_nInputClock_005c84a8) {
                g_pCurrentCutsceneSequence_00499c80->waitTicks = 0;
            }
            if (objectType == 2 &&
                g_bCutsceneSkipFrame_00499c54 != 0) {
                g_pCurrentCutsceneSequence_00499c80->waitTicks = 0;
            }
        }
        opcode = *instruction++;
        switch (opcode) {
        case 0x07:
            *stack = (short)(*stack == 0);
            break;
        case 0x05:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack != 0 && value != 0);
            break;
        case 0x06:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack != 0 || value != 0);
            break;
        case 0x00:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack + value);
            break;
        case 0x01:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack - value);
            break;
        case 0x02:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack * value);
            break;
        case 0x03:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack / value);
            break;
        case 0x04:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack % value);
            break;
        case 0x0b:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack & value);
            break;
        case 0x0c:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack | value);
            break;
        case 0x0d:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack ^ value);
            break;
        case 0x09:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack << (value & 0x1f));
            break;
        case 0x0a:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack >> (value & 0x1f));
            break;
        case 0x08:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            otherValue = PopCutsceneScriptValue(
                &stack, stackStorage + 10);
            PushCutsceneScriptValue(&stack, stackStorage,
                RandomInRange((unsigned short)otherValue,
                              (unsigned short)value));
            break;
        case 0x0f:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            if (branchGuard == 0 &&
                g_bCutsceneSkipFrame_00499c54 != 0 &&
                g_bCutsceneViewportPreallocated_00499c4c == 0) {
                value = 0;
            }
            if (g_bCutsceneSkipAll_00499c58 != 0)
                value = 0;
            if (value == 0) {
                g_nCutsceneBranchOffset_005d2e8c = *(short *)instruction;
                instruction += g_nCutsceneBranchOffset_005d2e8c;
            } else {
                instruction += 2;
            }
            break;
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            if (branchGuard == 0 &&
                g_bCutsceneSkipFrame_00499c54 != 0 &&
                g_bCutsceneViewportPreallocated_00499c4c == 0) {
                value = 1;
            }
            if (g_bCutsceneSkipAll_00499c58 != 0)
                value = 0;
            if (value == 1) {
                g_nCutsceneBranchOffset_005d2e8c = *(short *)instruction;
                instruction += g_nCutsceneBranchOffset_005d2e8c;
            } else {
                instruction += 2;
            }
            break;
        case 0x0e:
            g_nCutsceneBranchOffset_005d2e8c = *(short *)instruction;
            instruction += g_nCutsceneBranchOffset_005d2e8c;
            break;
        case 0x17:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack < value);
            break;
        case 0x18:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack > value);
            break;
        case 0x19:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack <= value);
            break;
        case 0x1a:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack >= value);
            break;
        case 0x1b:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack == value);
            break;
        case 0x1c:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            *stack = (short)(*stack != value);
            break;
        case 0x1d:
            *stack = (short)(*stack == 0);
            break;
        case 0x1e:
            *stack = (short)(*stack != 0);
            break;
        case 0x1f:
            objectResources = FindActiveCutsceneObjectResources(
                g_pCutsceneSpriteResources_0049288c);
            runtimeIndex = objectResources->scriptSymbolIndices[*instruction++];
            g_pCurrentCutsceneSprite_00499c78 =
                g_apSceneObjects_00499c38[runtimeIndex];
            if (g_pCurrentCutsceneSprite_00499c78 == 0)
                FatalCutsceneError("A21");
            break;
        case 0x20:
            objectResources = FindActiveCutsceneObjectResources(
                g_pCutscenePlaneResources_00492890);
            runtimeIndex = objectResources->scriptSymbolIndices[*instruction++];
            g_pCurrentCutscenePlane_00499c7c =
                g_apCutscenePlanes_00499c3c[runtimeIndex];
            if (g_pCurrentCutscenePlane_00499c7c == 0)
                FatalCutsceneError("A22");
            break;
        case 0x21:
            objectResources = FindActiveCutsceneObjectResources(
                g_pCutsceneSequenceResources_00492894);
            runtimeIndex = objectResources->scriptSymbolIndices[*instruction++];
            g_pCurrentCutsceneSequence_00499c80 =
                g_apCutsceneSequences_00499c40[runtimeIndex];
            if (g_pCurrentCutscenePlane_00499c7c == 0)
                FatalCutsceneError("A23");
            break;
        case 0x8a:
            g_bCutsceneTextAdvance_005d2ed0 = 1;
            resourceIndex = *instruction++;
            objectResources = FindActiveCutsceneObjectResources(
                g_pCutsceneSpriteResources_0049288c);
            g_pLinkedCutsceneSprite_00499c64 =
                g_apSceneObjects_00499c38[
                    objectResources->scriptSymbolIndices[resourceIndex]];
            if (g_pLinkedCutsceneSprite_00499c64 == 0)
                FatalCutsceneError("A24");
            g_pLinkedCutsceneSprite_00499c64->field_3 = 0;
            break;
        case 0x9e:
            g_nSceneFlicContext_00499c50 = *instruction++;
            break;
        case 0x77:
            index = *instruction++;
            g_pCutsceneLocalsScratch_005d2f00 = AllocateScenePointerTable(
                index, 2, 0, "Cannot Allocate Loacls Data");
            switch (objectType) {
            case 0:
                sprite = g_pCurrentCutsceneSprite_00499c78;
                if (sprite == 0)
                    FatalCutsceneError("A25");
                if (sprite->locals != 0)
                    ReleasePacketSlot((void **)&g_pCutsceneLocalsScratch_005d2f00);
                if (sprite->linkedScript == 0)
                    sprite->locals = g_pCutsceneLocalsScratch_005d2f00;
                else
                    ReleasePacketSlot((void **)&g_pCutsceneLocalsScratch_005d2f00);
                break;
            case 1:
                plane = g_pCurrentCutscenePlane_00499c7c;
                if (plane == 0)
                    FatalCutsceneError("A26");
                if (plane->locals != 0)
                    ReleasePacketSlot((void **)&g_pCutsceneLocalsScratch_005d2f00);
                plane->locals = g_pCutsceneLocalsScratch_005d2f00;
                break;
            case 2:
                sequence = g_pCurrentCutsceneSequence_00499c80;
                if (sequence == 0)
                    FatalCutsceneError("A27");
                if (sequence->locals != 0)
                    ReleasePacketSlot((void **)&g_pCutsceneLocalsScratch_005d2f00);
                sequence->locals = g_pCutsceneLocalsScratch_005d2f00;
                break;
            }
            break;
        case 0x7d:
            WaitForCutsceneInputEvent();
            break;
        case 0x7e:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            waitUntil = g_nInputClock_005c84a8 + value;
            while (g_nInputClock_005c84a8 < waitUntil &&
                   g_bCutsceneSkipFrame_00499c54 == 0 &&
                   g_bCutsceneViewportPreallocated_00499c4c == 0) {
                if (ServiceInputDevices(-1) != 0) {
                    ServiceCutsceneRuntimeHook();
                    if (FindQueuedInputEvent(2) != 0 ||
                        FindQueuedInputEvent(5) != 0) {
                        break;
                    }
                }
            }
            break;
        case 0x7f:
            g_nCutsceneResourceScratch_005d2e9c =
                (short)(signed char)*instruction++;
            if (g_stSecondaryViewBuffer_005d2c90.pixels ==
                g_stCutsceneTextBackingViewport_005d2db0.pixels) {
                g_stCutsceneTextBackingViewport_005d2db0.pixels = 0;
            }
            g_stSceneFlicScratchViewport_005d2eb0.left = 0;
            g_stSceneFlicScratchViewport_005d2eb0.top =
                g_nCutsceneResourceScratch_005d2e9c;
            g_stSceneFlicScratchViewport_005d2eb0.right = 0x13f;
            g_stSceneFlicScratchViewport_005d2eb0.bottom =
                (short)(g_nCutsceneResourceScratch_005d2e9c + 0x7d);
            g_stCutsceneTextViewport_005d2d90.left = 0x10;
            g_stCutsceneTextViewport_005d2d90.top =
                (short)(g_nCutsceneResourceScratch_005d2e9c + 0x7e);
            g_stCutsceneTextViewport_005d2d90.right = 0x12f;
            g_stCutsceneTextViewport_005d2d90.bottom = 199;
            ReleaseCutsceneViewport(&g_stSecondaryViewBuffer_005d2c90);
            g_stSecondaryViewBuffer_005d2c90.left =
                g_stSecondaryViewBuffer_005d2c90.top = 0;
            g_stSecondaryViewBuffer_005d2c90.right = 0x13f;
            g_stSecondaryViewBuffer_005d2c90.bottom = 0x7d;
            AllocateCutsceneViewportOrFatal(
                &g_stSecondaryViewBuffer_005d2c90, 0, 0);
            RouteCutsceneViewportToDisplay();
            ReleaseCutsceneViewport(
                &g_stCutsceneTextBackingViewport_005d2db0);
            g_stCutsceneTextBackingViewport_005d2db0.left =
                g_stCutsceneTextBackingViewport_005d2db0.top = 0;
            g_stCutsceneTextBackingViewport_005d2db0.right = 0x11f;
            g_stCutsceneTextBackingViewport_005d2db0.bottom = 0x24;
            AllocateCutsceneViewportOrFatal(
                &g_stCutsceneTextBackingViewport_005d2db0, 0, 0);
            break;
        case 0x80:
            g_stSceneFlicScratchViewport_005d2eb0.left = 0;
            g_stSceneFlicScratchViewport_005d2eb0.top =
                g_nCutsceneResourceScratch_005d2e9c;
            g_stSceneFlicScratchViewport_005d2eb0.right = 0x13f;
            g_stSceneFlicScratchViewport_005d2eb0.bottom = 199;
            g_stCutsceneTextViewport_005d2d90.left = 0;
            g_stCutsceneTextViewport_005d2d90.top = 0;
            g_stCutsceneTextViewport_005d2d90.right = 0x13f;
            g_stCutsceneTextViewport_005d2d90.bottom = 199;
            ReleaseCutsceneViewport(&g_stSecondaryViewBuffer_005d2c90);
            g_stSecondaryViewBuffer_005d2c90.left =
                g_stSecondaryViewBuffer_005d2c90.top = 0;
            g_stSecondaryViewBuffer_005d2c90.right = 0x13f;
            g_stSecondaryViewBuffer_005d2c90.bottom = 199;
            AllocateCutsceneViewportOrFatal(
                &g_stSecondaryViewBuffer_005d2c90, 0, 0);
            RouteCutsceneViewportToDisplay();
            ReleaseCutsceneViewport(
                &g_stCutsceneTextBackingViewport_005d2db0);
            g_stCutsceneTextBackingViewport_005d2db0 =
                g_stSecondaryViewBuffer_005d2c90;
            break;
        case 0xb8:
            PopCutsceneScriptValue(&stack, stackStorage + 10);
            branchGuard = 1;
            break;
        case 0x22:
            returnValue = 0;
            break;
        case 0x23:
            *scriptCursor = instruction;
            return 0;
        case 0x24:
            PushCutsceneScriptValue(&stack, stackStorage,
                                    (short)(signed char)*instruction++);
            break;
        case 0x25:
            PushCutsceneScriptValue(&stack, stackStorage,
                                    *(short *)instruction);
            instruction += 2;
            break;
        case 0x26:
            if (g_pCampaignGlobals_00499c94 == 0)
                FatalCutsceneError("A28");
            index = 0;
            while (*instruction == 0xff) {
                index = (short)(index + *instruction++);
            }
            index = (short)(index + *instruction++);
            PushCutsceneScriptValue(&stack, stackStorage,
                ((short *)g_pCampaignGlobals_00499c94)[index]);
            break;
        case 0xae:
            resourceIndex = *instruction++;
            switch (resourceIndex) {
            case 0:
                g_nCutsceneTextLeft_00499ee0 =
                    PopCutsceneScriptValue(&stack, stackStorage + 10);
                break;
            case 1:
                g_nCutsceneTextTop_00499ee2 =
                    PopCutsceneScriptValue(&stack, stackStorage + 10);
                break;
            case 2:
                g_nCutsceneTextRight_00499ee4 =
                    PopCutsceneScriptValue(&stack, stackStorage + 10);
                break;
            case 3:
                g_nCutsceneTextBottom_00499ee6 =
                    PopCutsceneScriptValue(&stack, stackStorage + 10);
                break;
            }
            g_bCutsceneTextBoundsOverride_00499edc = 1;
            break;
        case 0xaf:
            g_pCurrentTextContext_005c8d1c->alignment = *instruction++;
            break;
        case 0x28:
            if (g_pCampaignGlobals_00499c94 == 0)
                FatalCutsceneError("A29");
            index = 0;
            while (*instruction == 0xff) {
                index = (short)(index + *instruction++);
            }
            index = (short)(index + *instruction++);
            ((short *)g_pCampaignGlobals_00499c94)[index] =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x27:
            if (g_pCampaignGlobals_00499c94 == 0)
                FatalCutsceneError("A30");
            index = *(short *)instruction;
            instruction += 2;
            index = (short)(index + PopCutsceneScriptValue(
                &stack, stackStorage + 10));
            PushCutsceneScriptValue(&stack, stackStorage,
                ((short *)g_pCampaignGlobals_00499c94)[index]);
            break;
        case 0x29:
            if (g_pCampaignGlobals_00499c94 == 0)
                FatalCutsceneError("A31");
            index = *(short *)instruction;
            instruction += 2;
            index = (short)(index + PopCutsceneScriptValue(
                &stack, stackStorage + 10));
            ((short *)g_pCampaignGlobals_00499c94)[index] =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x2a:
            sprite = g_pCurrentCutsceneSprite_00499c78;
            if (sprite->currentFrame < sprite->finalFrame - 1)
                sprite->currentFrame++;
            else
                sprite->currentFrame = 0;
            break;
        case 0x2b:
            sprite = g_pCurrentCutsceneSprite_00499c78;
            if (sprite->currentFrame == 0)
                sprite->currentFrame = (short)(sprite->finalFrame - 1);
            else
                sprite->currentFrame--;
            break;
        case 0x2c:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->active);
            break;
        case 0x2d:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->visible);
            break;
        case 0x2e:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->drawType);
            break;
        case 0x30:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->currentFrame);
            break;
        case 0x31:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->finalFrame);
            break;
        case 0x32:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->waitTicks);
            break;
        case 0x2f:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->baseFrame);
            break;
        case 0x33:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->delayFrames);
            break;
        case 0x34:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->x);
            break;
        case 0x35:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->y);
            break;
        case 0x36:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->field_2f);
            break;
        case 0x37:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->rotation);
            break;
        case 0x38:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->scale);
            break;
        case 0x39:
            PushCutsceneScriptValue(&stack, stackStorage,
                (short)g_pCurrentCutsceneSprite_00499c78->drawFlags);
            break;
        case 0x3d:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->locals[*instruction++]);
            break;
        case 0x3a:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->deltaX);
            break;
        case 0x3b:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->deltaY);
            break;
        case 0x3c:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSprite_00499c78->deltaDepth);
            break;
        case 0x3e:
            g_pCurrentCutsceneSprite_00499c78->active = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x41:
            g_pCurrentCutsceneSprite_00499c78->baseFrame =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x3f:
            g_pCurrentCutsceneSprite_00499c78->visible = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x40:
            g_pCurrentCutsceneSprite_00499c78->drawType =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x42:
            g_pCurrentCutsceneSprite_00499c78->currentFrame =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x43:
            g_pCurrentCutsceneSprite_00499c78->finalFrame =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x44:
            waitTicks = PopCutsceneScriptValue(&stack, stackStorage + 10);
            if (g_bCutsceneSkipFrame_00499c54 != 0)
                waitTicks = 0;
#ifdef SDL_PORT
            /* See CUTSCENE_SPRITE_MIN_TICKS: a short script-authored wait
             * relied on the DOS host's own draw speed, not a real-time
             * gate, to look right. */
            if (waitTicks > 0 && waitTicks < CUTSCENE_SPRITE_MIN_TICKS)
                waitTicks = CUTSCENE_SPRITE_MIN_TICKS;
#endif
            g_pCurrentCutsceneSprite_00499c78->waitTicks = waitTicks;
            g_pCurrentCutsceneSprite_00499c78->waitStart =
                g_nInputClock_005c84a8;
            if (objectType == 0) {
                *scriptCursor = instruction;
                return returnValue;
            }
            break;
        case 0x45:
            g_pCurrentCutsceneSprite_00499c78->delayFrames =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x46:
            g_pCurrentCutsceneSprite_00499c78->x =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x47:
            g_pCurrentCutsceneSprite_00499c78->y =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x48:
            g_pCurrentCutsceneSprite_00499c78->field_2f =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x49:
            g_pCurrentCutsceneSprite_00499c78->rotation =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x4a:
            g_pCurrentCutsceneSprite_00499c78->scale =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x4b:
            g_pCurrentCutsceneSprite_00499c78->drawFlags =
                (unsigned short)PopCutsceneScriptValue(
                    &stack, stackStorage + 10);
            break;
        case 0x4f:
            g_pCurrentCutsceneSprite_00499c78->locals[*instruction++] =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x4c:
            g_pCurrentCutsceneSprite_00499c78->deltaX = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x4d:
            g_pCurrentCutsceneSprite_00499c78->deltaY = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x4e:
            g_pCurrentCutsceneSprite_00499c78->deltaDepth = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0xab:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutscenePlane_00499c7c->waitTicks);
            break;
        case 0x50:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutscenePlane_00499c7c->active);
            break;
        case 0x51:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutscenePlane_00499c7c->visible);
            break;
        case 0x52:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutscenePlane_00499c7c->x);
            break;
        case 0x53:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutscenePlane_00499c7c->y);
            break;
        case 0x54:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutscenePlane_00499c7c->depth);
            break;
        case 0x55:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutscenePlane_00499c7c->rotation);
            break;
        case 0x56:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutscenePlane_00499c7c->scale);
            break;
        case 0x58:
            PushCutsceneScriptValue(&stack, stackStorage,
                (short)g_pCurrentCutscenePlane_00499c7c->drawFlags);
            break;
        case 0x57:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutscenePlane_00499c7c->locals[*instruction++]);
            break;
        case 0x59:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutscenePlane_00499c7c->deltaX);
            break;
        case 0x5a:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutscenePlane_00499c7c->deltaY);
            break;
        case 0x5b:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutscenePlane_00499c7c->deltaDepth);
            break;
        case 0x5c:
            g_pCurrentCutscenePlane_00499c7c->active = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x5d:
            g_pCurrentCutscenePlane_00499c7c->visible = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x5e:
            g_pCurrentCutscenePlane_00499c7c->x =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x5f:
            g_pCurrentCutscenePlane_00499c7c->y =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x60:
            g_pCurrentCutscenePlane_00499c7c->depth =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x61:
            g_pCurrentCutscenePlane_00499c7c->rotation =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x62:
            g_pCurrentCutscenePlane_00499c7c->scale =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x64:
            g_pCurrentCutscenePlane_00499c7c->drawFlags =
                (unsigned short)PopCutsceneScriptValue(
                    &stack, stackStorage + 10);
            break;
        case 0x63:
            g_pCurrentCutscenePlane_00499c7c->locals[*instruction++] =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x65:
            g_pCurrentCutscenePlane_00499c7c->deltaX = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x66:
            g_pCurrentCutscenePlane_00499c7c->deltaY = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x67:
            g_pCurrentCutscenePlane_00499c7c->deltaDepth = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0xaa:
            waitTicks = PopCutsceneScriptValue(&stack, stackStorage + 10);
            if (g_bCutsceneSkipFrame_00499c54 != 0)
                waitTicks = 0;
#ifdef SDL_PORT
            /* See CUTSCENE_SPRITE_MIN_TICKS: a short script-authored wait
             * relied on the DOS host's own draw speed, not a real-time
             * gate, to look right. */
            if (waitTicks > 0 && waitTicks < CUTSCENE_SPRITE_MIN_TICKS)
                waitTicks = CUTSCENE_SPRITE_MIN_TICKS;
#endif
            g_pCurrentCutscenePlane_00499c7c->waitTicks = waitTicks;
            g_pCurrentCutscenePlane_00499c7c->waitStart =
                g_nInputClock_005c84a8;
            if (objectType == 1) {
                *scriptCursor = instruction;
                return returnValue;
            }
            break;
        case 0x71:
            g_pCurrentCutsceneSequence_00499c80->locals[*instruction++] =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x70:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSequence_00499c80->locals[*instruction++]);
            break;
        case 0x68:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSequence_00499c80->active);
            break;
        case 0x69:
            g_pCurrentCutsceneSequence_00499c80->active = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x6a:
            if (g_bCutsceneSkipFrame_00499c54 != 0)
                g_pCurrentCutsceneSequence_00499c80->waitTicks = 0;
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneSequence_00499c80->waitTicks);
            break;
        case 0x6b:
            waitTicks = PopCutsceneScriptValue(&stack, stackStorage + 10);
            if (waitTicks != 0 && g_bCutsceneSkipFrame_00499c54 == 0) {
                while (g_nInputClock_005c84a8 <
                       g_pCurrentCutsceneSequence_00499c80->waitStart +
                           g_pCurrentCutsceneSequence_00499c80->waitTicks) {
                    PumpWindowMessages(0);
#ifdef SDL_PORT
                    /* See issue #33 (ESC feels slow to skip cutscenes):
                     * this loop only pumps the OS message queue, which is
                     * what actually sets g_bSceneEscapeRequested_0049d4b0
                     * (sdl/events.c) the instant Escape is pressed -- but
                     * nothing here ever reads that flag. The real
                     * escape-to-skip conversion lives in the outer script
                     * loop above (case dispatch loop, "handle_queued_
                     * cutscene_input"), which only runs BETWEEN opcodes.
                     * A long wait (this opcode is script-timed, often
                     * paced to the scene's music/dialogue) blocks that
                     * outer loop from ever running again until the wait
                     * naturally elapses, so Escape sits pending and
                     * ignored for however long is left. Bail out of the
                     * wait itself the instant Escape is pending, the same
                     * way case 0x7e already does for its own wait, and
                     * let the outer loop perform the real conversion on
                     * its very next iteration. */
                    if (g_bSceneEscapeRequested_0049d4b0 != 0)
                        break;
#endif
                }
            }
            g_pCurrentCutsceneSequence_00499c80->waitTicks = waitTicks;
            g_pCurrentCutsceneSequence_00499c80->waitStart =
                g_nInputClock_005c84a8;
            break;
        case 0x81:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            if (g_cCutsceneVideoMode_00499c48 == 0x0d)
                value = g_abCutscenePaletteTranslation_00499c98[value];
            g_stCutsceneTextContext_005d2f40.colour = (unsigned char)value;
            break;
        case 0x8b:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            if (g_bCutsceneSkipFrame_00499c54 == 0 &&
                g_bCutsceneViewportPreallocated_00499c4c == 0) {
                ClearCutsceneViewport(
                    &g_stSceneFlicScratchViewport_005d2eb0,
                    (unsigned char)value);
                PanToScreen(&g_stSecondaryViewBuffer_005d2c90,
                            &g_stSceneFlicScratchViewport_005d2eb0);
                MarkDibDirty();
                DIBslamReal();
                CopyViewportContents(&g_stSecondaryViewBuffer_005d2c90,
                                     &g_stSceneFlicScratchViewport_005d2eb0);
            }
            break;
        case 0x8c:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            SaveGamePalette();
            FadeViewportPaletteToColour(
                &g_stModalSourceViewport_005d2c50, value, 1);
            ClearCutsceneViewport(
                &g_stModalSourceViewport_005d2c50,
                (unsigned char)value);
            MarkDibDirty();
            DIBslamReal();
            RestoreGamePalette();
            break;
        case 0xb3:
            otherValue = PopCutsceneScriptValue(
                &stack, stackStorage + 10);
            value = PopCutsceneScriptValue(
                &stack, stackStorage + 10);
            g_nCutsceneFormatValue_005d2f0c =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            g_nCutsceneTextIndex_005d2ea4 = *(short *)instruction;
            instruction += 2;
            g_pszCutsceneSourceText_005d2dcc =
                g_pCutsceneTextResources_0049289c->entries[
                    g_nCutsceneTextIndex_005d2ea4];
            ExpandCutsceneText(
                (unsigned char *)g_pszCutsceneSourceText_005d2dcc,
                g_pszCutsceneFormattedText_005d2dc8);
            DrawCutsceneTextAt(value, otherValue, 4,
                               g_pszCutsceneFormattedText_005d2dc8);
            break;
        case 0x6f:
            g_nCutsceneTextIndex_005d2ea4 = *(short *)instruction;
            instruction += 2;
            g_pszCutsceneSourceText_005d2dcc =
                g_pCutsceneTextResources_0049289c->entries[
                    g_nCutsceneTextIndex_005d2ea4];
            ExpandCutsceneText(
                (unsigned char *)g_pszCutsceneSourceText_005d2dcc,
                g_pszCutscenePrintBuffer_005d2f10);
            g_pszCurrentCutsceneText_00499da4 =
                g_pszCutscenePrintBuffer_005d2f10;
            break;
        case 0xb4:
            g_nCutsceneMusicVolume_0049be90 =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x6e:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            otherValue = PopCutsceneScriptValue(
                &stack, stackStorage + 10);
            resourceIndex = *instruction++;
            DrawCutsceneTextAt(otherValue, value, resourceIndex,
                               g_pszCutscenePrintBuffer_005d2f10);
            break;
        case 0xad:
            resourceIndex = *instruction++;
            objectResources = FindActiveCutsceneObjectResources(
                g_pCutsceneSpriteResources_0049288c);
            ReleaseCutsceneSpriteShape(g_apSceneObjects_00499c38[
                objectResources->scriptSymbolIndices[resourceIndex]]);
            break;
        case 0x94:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            resourceIndex = *instruction++;
            switch (resourceIndex) {
            case 0:
                ClearCutsceneViewport(
                    &g_stSceneFlicScratchViewport_005d2eb0,
                    (unsigned char)value);
                break;
            case 1:
                ClearCutsceneViewport(&g_stSecondaryViewBuffer_005d2c90,
                                      (unsigned char)value);
                break;
            case 2:
                ClearCutsceneViewport(&g_stCutsceneTextViewport_005d2d90,
                                      (unsigned char)value);
                break;
            case 3:
                ClearCutsceneViewport(
                    &g_stCutsceneTextBackingViewport_005d2db0,
                    (unsigned char)value);
                break;
            case 4:
                ClearCutsceneViewport(&g_stModalSourceViewport_005d2c50,
                                      (unsigned char)value);
                break;
            }
            break;
        case 0xa3:
            PushCutsceneScriptValue(
                &stack, stackStorage,
                (short)(signed char)g_nOriginDevUnlock_0049d774);
            break;
        case 0x9d:
            value = 0;
            if (g_bCutsceneSkipFrame_00499c54 != 0)
                value = 1;
            if (g_bCutsceneSkipAll_00499c58 != 0)
                value = 2;
            PushCutsceneScriptValue(&stack, stackStorage, value);
            break;
        case 0xa4:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            switch (value) {
            case 0:
                g_bCutsceneSkipAll_00499c58 = 0;
                g_bCutsceneSkipFrame_00499c54 = 0;
                ServiceInputDevices(-1);
                FlushPendingInputEvents();
                break;
            case 1:
                g_bCutsceneSkipFrame_00499c54 = 0;
                ServiceInputDevices(-1);
                FlushPendingInputEvents();
                break;
            case -2:
                g_bCutsceneSkipAll_00499c58 = 1;
                g_bCutsceneSkipFrame_00499c54 = 1;
                break;
            case -1:
                g_bCutsceneSkipFrame_00499c54 = 1;
                break;
            }
            break;
        case 0x9b:
            PushCutsceneScriptValue(&stack, stackStorage,
                (short)(g_stSecondaryViewBuffer_005d2c90.bottom + 1));
            break;
        case 0x9c:
            PushCutsceneScriptValue(&stack, stackStorage,
                g_bCutsceneTextAdvance_005d2ed0);
            break;
        case 0xa2:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            LoadCutsceneSpeechSlot(*instruction++, value);
            CopyViewportContents(&g_stSceneFlicScratchViewport_005d2eb0,
                                 &g_stSecondaryViewBuffer_005d2c90);
            break;
        case 0xb0: {
#ifdef SDL_PORT
            /* See issue #33 (review on PR #34): breaking out here while the
             * cache is still busy must NOT fall through to the "use and
             * clear" logic below as if the fetch had finished -- that would
             * read g_apCutsceneSpeechPackets_005d2f80[value] before it is
             * valid and then clear the still-in-flight fetch's own
             * bookkeeping (g_apszCutsceneSpeechFiles_005d2ee0[value] etc.),
             * silently dropping it. g_wDigiPlaying_0049bb60 is never
             * set nonzero anywhere in this reconstruction today, so this
             * loop cannot actually break early yet -- guarded correctly
             * anyway rather than leaving unsound-if-ever-live code here. */
            signed char cacheStillBusy = 0;
#endif
            while (g_wDigiPlaying_0049bb60 != 0) {
                PumpWindowMessages(0);
#ifdef SDL_PORT
                if (g_bSceneEscapeRequested_0049d4b0 != 0) {
                    cacheStillBusy = 1;
                    break;
                }
#endif
            }
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
#ifdef SDL_PORT
            if (cacheStillBusy)
                break;
#endif
            if (g_apCutsceneSpeechPackets_005d2f80[value] != 0 &&
                g_apszCutsceneSpeechFiles_005d2ee0[value] != 0) {
                PlaySpeechPacketBuffer(
                    g_apCutsceneSpeechPackets_005d2f80[value],
                    g_asCutsceneSpeechChannels_005d2d70[value], 1);
            } else if (g_apszCutsceneSpeechFiles_005d2ee0[value] != 0) {
                LoadAndPlaySpeechPacket(
                    g_apszCutsceneSpeechFiles_005d2ee0[value],
                    g_asCutsceneSpeechSections_005d2dd0[value]);
                CopyViewportContents(
                    &g_stSceneFlicScratchViewport_005d2eb0,
                    &g_stSecondaryViewBuffer_005d2c90);
                g_apCutsceneSpeechPackets_005d2f80[value] = 0;
                g_asCutsceneSpeechChannels_005d2d70[value] = 0;
            }
            g_apszCutsceneSpeechFiles_005d2ee0[value] = 0;
            g_asCutsceneSpeechSections_005d2dd0[value] = 0;
            break;
        }
        case 0xa9:
            value = 0;
            if ((g_aObjectTypeData_00496d30[
                     g_acObjectType_00493980[g_nYourWingman_0049346c]].
                     damageCapacity >> 1) - 1 <=
                g_asObjectDamage_00495178[g_nYourWingman_0049346c]) {
                value = 1;
            }
            PushCutsceneScriptValue(&stack, stackStorage, value);
            break;
        case 0x96:
            ReleaseCutsceneFontPacket();
            break;
        case 0x82:
            resourceIndex = *instruction++;
            fileResources = FindActiveCutsceneFileResources(
                g_pCutsceneFontResources_004928a0);
            g_stCutsceneTextContext_005d2f40.font =
                LoadCachedCutsceneResource(fileResources, resourceIndex, 1);
            g_bCinematicSpriteFontEnabled_005c82a7 =
                (signed char)*instruction++;
            g_nCutsceneFontOwner_005d2fa0 =
                g_nActiveCutsceneResourceLevel_00499d9c;
            break;
        case 0x9f:
            g_cCutsceneSoundVolume_00499c2c = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x91:
            resourceIndex = *instruction++;
            SoundDebugPrintf(
                "VAL=========================================%d\n",
                (int)resourceIndex);
            if (g_bCutsceneSkipFrame_00499c54 == 0 &&
                g_bCutsceneViewportPreallocated_00499c4c == 0) {
                PlaySfxWaveFileByNumber(
                    MapCutsceneSoundEffectNumber((int)resourceIndex),
                    -1, 0);
            }
            PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x93:
            resourceIndex = *instruction++;
            ReleaseCutsceneSoundEffects((short)(signed char)resourceIndex);
            break;
        case 0x84:
            resourceIndex = *instruction++;
            if ((g_nMusicDriverMode_0049be8c == 1 ||
                 g_nMusicDriverMode_0049be8c == 2) &&
                g_bCutsceneViewportPreallocated_00499c4c == 0) {
                FlushSoundEffects();
                ReleasePacketSlot(&g_pCutsceneFxPacket_00499c30);
                fileResources = FindActiveCutsceneFileResources(
                    g_pCutsceneFxResources_004928b8);
                g_pCutsceneFxPacket_00499c30 =
                    LoadCachedCutsceneResource(
                        fileResources, resourceIndex, 3);
                SoundDebugPrintf("LOAD FX val=%d\n", (int)resourceIndex);
            }
            break;
        case 0x98:
            FlushSoundEffects();
            ReleasePacketSlot(&g_pCutsceneFxPacket_00499c30);
            break;
        case 0xb2:
            resourceIndex = *instruction++;
            if ((g_nMusicDriverMode_0049be8c == 1 ||
                 g_nMusicDriverMode_0049be8c == 2) &&
                g_bCutsceneViewportPreallocated_00499c4c == 0) {
                FadeMusic(resourceIndex);
            }
            break;
        case 0x90:
            if (g_pSceneMusicPacket_00499c08 != 0)
                StopMusic(0);
            resourceIndex = *instruction++;
            if (g_bCutsceneSkipFrame_00499c54 == 0 &&
                g_bCutsceneViewportPreallocated_00499c4c == 0) {
                g_pSceneMusicPacket_00499c08 =
                    FindLoadedCutsceneMusic(resourceIndex);
                StartInteractiveMusic(resourceIndex);
            }
            break;
        case 0x92:
            value = *(short *)instruction;
            instruction += 2;
            StopMusic(value);
            g_pSceneMusicPacket_00499c08 = 0;
            break;
        case 0x85:
            resourceIndex = *instruction++;
            break;
        case 0x97:
            resourceIndex = *instruction++;
            ReleaseCutsceneMusicNodes(
                &g_pCutsceneMusicPackets_00499c34, resourceIndex);
            break;
        case 0x83:
            if (g_bCutsceneViewportPreallocated_00499c4c == 0) {
                resourceIndex = *instruction++;
                g_pActiveCutsceneFileResources_005d2da4 =
                    FindActiveCutsceneFileResources(
                    g_pCutscenePaletteResources_004928b0);
                g_pCutscenePalettePacket_005d2e98 =
                    LoadCachedCutsceneResource(
                        g_pActiveCutsceneFileResources_005d2da4,
                        resourceIndex, 1);
                value = *instruction++;
                otherValue = (short)(*instruction++ + 1);
                ConsumeCutscenePalettePacket(value, otherValue, 0);
                ReleaseLoadedCutsceneResource(
                    g_pActiveCutsceneFileResources_005d2da4,
                    resourceIndex);
                g_pCutscenePalettePacket_005d2e98 = 0;
            } else {
                instruction += 3;
            }
            break;
        case 0x7c:
            if (g_bCutsceneViewportPreallocated_00499c4c == 0 &&
                g_bCutsceneSkipFrame_00499c54 == 0) {
                while (g_nInputClock_005c84a8 <
                       g_nNextCutsceneFrameClock_00499c90) {
                    PumpWindowMessages(0);
#ifdef SDL_PORT
                    /* See issue #33; same fix as case 0x6b above. This
                     * runs every cutscene frame, so most frames' delay is
                     * short enough not to matter, but a shot with a long
                     * configured frame delay (e.g. paced to music) can
                     * still eat a pending Escape the same way 0x6b did. */
                    if (g_bSceneEscapeRequested_0049d4b0 != 0)
                        break;
#endif
                }
                PresentCutsceneFrame(&g_stSecondaryViewBuffer_005d2c90,
                                     &g_stSceneFlicScratchViewport_005d2eb0);
                if (g_pMemoryLogFile_00499da8 != 0)
                    LogChangedCutsceneWorkBuffers();
                RestoreCutsceneTextBacking();
                g_bCutsceneFramePresented_005d2de0 = 1;
                if (g_nCutsceneFrameDelay_00499c8c != 0) {
                    g_nNextCutsceneFrameClock_00499c90 =
                        g_nInputClock_005c84a8 +
                        g_nCutsceneFrameDelay_00499c8c;
                }
            }
            break;
        case 0x6d:
            if (g_bCutsceneDrawingEnabled_00499c60 == 0 &&
                (g_bCutsceneSkipFrame_00499c54 != 0 ||
                 g_bCutsceneViewportPreallocated_00499c4c != 0)) {
                instruction++;
            } else {
                resourceIndex = *instruction++;
                g_pCutsceneSpriteParentPlane_00499ef0 = 0;
                if (resourceIndex == 0xff) {
                    if (g_pCurrentCutsceneSprite_00499c78 != 0) {
                        UpdateCutsceneSpriteObject(
                            g_pCurrentCutsceneSprite_00499c78);
                        DrawCutsceneSprite(g_pCurrentCutsceneSprite_00499c78);
                    }
                } else {
                    objectResources = FindActiveCutsceneObjectResources(
                        g_pCutsceneSpriteResources_0049288c);
                    runtimeIndex = objectResources->scriptSymbolIndices[
                        resourceIndex];
                    UpdateCutsceneSpriteObject(
                        g_apSceneObjects_00499c38[runtimeIndex]);
                    DrawCutsceneSprite(
                        g_apSceneObjects_00499c38[runtimeIndex]);
                }
                g_pCutsceneSpriteParentPlane_00499ef0 =
                    g_pCurrentCutscenePlane_00499c7c;
            }
            break;
        case 0x6c:
            if (g_bCutsceneDrawingEnabled_00499c60 == 0 &&
                (g_bCutsceneSkipFrame_00499c54 != 0 ||
                 g_bCutsceneViewportPreallocated_00499c4c != 0)) {
                instruction++;
            } else {
                resourceIndex = *instruction++;
                if (resourceIndex == 0xff) {
                    if (g_pCurrentCutscenePlane_00499c7c != 0) {
                        UpdateCutscenePlaneObject(
                            g_pCurrentCutscenePlane_00499c7c, 1);
                        DrawCutscenePlane(g_pCurrentCutscenePlane_00499c7c);
                    }
                } else {
                    objectResources = FindActiveCutsceneObjectResources(
                        g_pCutscenePlaneResources_00492890);
                    runtimeIndex = objectResources->scriptSymbolIndices[
                        resourceIndex];
                    UpdateCutscenePlaneObject(
                        g_apCutscenePlanes_00499c3c[runtimeIndex], 1);
                    DrawCutscenePlane(
                        g_apCutscenePlanes_00499c3c[runtimeIndex]);
                }
            }
            break;
        case 0x86:
            g_bCutsceneDrawingEnabled_00499c60 = (signed char)
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0xb9:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            g_nCutsceneTextIndex_005d2ea4 = *(short *)instruction;
            instruction += 2;
            g_pszCutsceneSourceText_005d2dcc =
                g_pCutsceneTextResources_0049289c->entries[
                    g_nCutsceneTextIndex_005d2ea4];
            if (g_pMemoryLogFile_00499da8 != 0) {
                fprintf(g_pMemoryLogFile_00499da8,
                        "%%%%%% ELOG: %Fs = %d\n",
                        g_pszCutsceneSourceText_005d2dcc, value);
            }
            break;
        case 0xb1:
            value = *(short *)instruction;
            instruction += 2;
            if (g_pMemoryLogFile_00499da8 != 0) {
                fprintf(g_pMemoryLogFile_00499da8,
                        "@@@@@@@@@@@@@@@@ Logged: %d\n", value);
                WriteDetailedMemoryStateReport();
            }
            break;
        case 0xa1:
            FormatCinematicMemoryStatus("Report");
            WriteDetailedMemoryStateReport();
            break;
        case 0x95:
            g_cCutsceneTextStyle_00499f40 = (signed char)*instruction++;
            break;
        case 0x89:
            resourceIndex = *instruction++;
            g_nSceneFlicContext_00499c50 = 0x40;
            if (g_bCutsceneViewportPreallocated_00499c4c == 0) {
                fileResources = FindActiveCutsceneFileResources(
                    g_pCutsceneFilmResources_004928b4);
                packet = LoadCachedCutsceneResource(
                    fileResources, resourceIndex, 0);
                savedFilmResources = g_pCutsceneFilmResources_004928b4;
                g_bCutsceneDrawingEnabled_00499c60 = 0;
                g_bCutsceneSkipFrame_00499c54 = 0;
                g_nCutsceneResourceLevel_00499d98++;
                g_nActiveCutsceneResourceLevel_00499d9c =
                    g_nCutsceneResourceLevel_00499d98;
                ParseCutsceneContainer(packet);
                WriteMemoryStateReportHook();
                DosMemset(g_pszCutsceneWorkBuffer_005d2ecc, 4, 0);
                ClearCutsceneTextViewport();
                ClearCutsceneViewport(
                    &g_stCutsceneTextViewport_005d2d90, 0);
                objectResources = FindActiveCutsceneObjectResources(
                    g_pCutsceneSceneResources_00492898);
                ExecuteCutsceneScene(g_apCutsceneScenes_00499c44[
                    objectResources->scriptSymbolIndices[0]]);
                ReleaseCutsceneResourceLevel(
                    g_nCutsceneResourceLevel_00499d98);
                g_nCutsceneResourceLevel_00499d98--;
                g_nActiveCutsceneResourceLevel_00499d9c =
                    g_nCutsceneResourceLevel_00499d98;
                g_bCutsceneSkipFrame_00499c54 = 0;
                g_pCutsceneFilmResources_004928b4 = savedFilmResources;
                fileResources = FindActiveCutsceneFileResources(
                    g_pCutsceneFilmResources_004928b4);
                ReleaseLoadedCutsceneResource(fileResources, resourceIndex);
                g_bCutsceneDrawingEnabled_00499c60 = 0;
            }
            break;
        case 0x88:
            resourceIndex = *instruction++;
            objectResources = FindActiveCutsceneObjectResources(
                g_pCutsceneSequenceResources_00492894);
            ExecuteCutsceneSequence(
                g_apCutsceneSequences_00499c40[
                    objectResources->scriptSymbolIndices[resourceIndex]],
                0, 1);
            if (g_bCutsceneSkipFrame_00499c54 != 0 && objectType == 3)
                g_bCutsceneSkipFrame_00499c54 = 0;
            break;
        case 0x87:
            g_nCutsceneTextIndex_005d2ea4 = *(short *)instruction;
            instruction += 2;
            resourceIndex = *instruction++;
            g_pszCutsceneSourceText_005d2dcc =
                g_pCutsceneTextResources_0049289c->entries[
                    g_nCutsceneTextIndex_005d2ea4];
            objectResources = FindActiveCutsceneObjectResources(
                g_pCutsceneSequenceResources_00492894);
            ExecuteCutsceneSequence(
                g_apCutsceneSequences_00499c40[
                    objectResources->scriptSymbolIndices[resourceIndex]],
                (unsigned char *)g_pszCutsceneSourceText_005d2dcc, 1);
            if (g_bCutsceneSkipFrame_00499c54 != 0 &&
                (objectType == 3 || objectType == 2)) {
                g_bCutsceneSkipFrame_00499c54 = 0;
            }
            break;
        case 0x8d:
            g_nCutsceneFrameDelay_00499c8c =
                (unsigned short)PopCutsceneScriptValue(
                    &stack, stackStorage + 10);
            break;
        case 0x8e:
            g_nCutsceneFrameTick_00499c88 =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x8f:
            value = *(short *)instruction;
            instruction += 2;
            /* The original divides by 0x3b (59), not 0x3c (60).
             * g_nInputClock_005c84a8 (what this delay is measured against,
             * see PumpWindowMessages/winmain.c) ticks in exact 1/60s units
             * -- confirmed independently by CUTSCENE_MOUTH_MIN_TICKS=3
             * meaning exactly 3/60s=50ms=one 20fps frame elsewhere in this
             * file. Converting a requested-fps `value` to a tick delay is
             * 60/value, not 59/value; the sibling opcode 0x8d (above) takes
             * an already-computed tick delay straight from the script with
             * no arithmetic at all, so this is specifically the "set rate
             * by fps" formula and nothing else. With integer division,
             * 59/value truncates to a smaller (i.e. faster) delay than
             * 60/value for nearly every value -- e.g. a scene requesting
             * 20fps gets 59/20=2 ticks/frame (~30fps, 50% too fast)
             * instead of the correct 60/20=3 (20fps). Different requested
             * rates truncate by different amounts, which is consistent
             * with cutscenes running at inconsistent, scene-dependent
             * speeds rather than a single uniform offset. Corrected on the
             * port only -- the reference build keeps the original's own
             * 0x3b so it stays byte-comparable for verification.
             * https://github.com/schlangz/openwc2/blob/main/docs/wc2re_cross_reference.md
             */
#ifdef SDL_PORT
            g_nCutsceneFrameDelay_00499c8c =
                (unsigned short)(0x3c / value);
#else
            g_nCutsceneFrameDelay_00499c8c =
                (unsigned short)(0x3b / value);
#endif
            break;
        case 0x75:
            index = *instruction++;
            g_pCurrentCutsceneScene_00499c84->locals[index * 2] =
                PopCutsceneScriptValue(&stack, stackStorage + 10);
            break;
        case 0x74:
            index = *instruction++;
            PushCutsceneScriptValue(&stack, stackStorage,
                g_pCurrentCutsceneScene_00499c84->locals[index * 2]);
            break;
        case 0x78:
            if (g_pCurrentCutsceneSprite_00499c78->linkedScript != 0) {
                g_nSavedCutsceneResourceOwner_005d2d68 =
                    g_nActiveCutsceneResourceLevel_00499d9c;
            }
            if (g_pCurrentCutsceneSprite_00499c78 == 0) {
                g_pCurrentCutsceneSprite_00499c78 =
                    g_apSceneObjects_00499c38[0];
            }
            if (g_pCurrentCutsceneSprite_00499c78 == 0)
                FatalCutsceneError("A32");
            g_pCurrentCutsceneSprite_00499c78->drawType = *instruction++;
            g_pCurrentCutsceneSprite_00499c78->baseFrame = *instruction++;
            g_pCurrentCutsceneSprite_00499c78->finalFrame = *instruction++;
            g_pCurrentCutsceneSprite_00499c78->currentFrame = 0;
            index = 0;
            while (*instruction == 0xff) {
                index = (short)(index + *instruction++);
            }
            index = (short)(index + *instruction++);
            if (g_pCurrentCutsceneSprite_00499c78->linkedScript != 0) {
                g_nActiveCutsceneResourceLevel_00499d9c =
                    g_pCurrentCutsceneSprite_00499c78->owner;
            }
            if (g_pCurrentCutsceneSprite_00499c78->drawType != 4) {
                fileResources = FindActiveCutsceneFileResources(
                    g_pCutsceneShapeResources_004928a8);
                g_pCurrentCutsceneSprite_00499c78->shape =
                    LoadCachedCutsceneResource(fileResources, index, 1);
            } else {
                InitializeSceneFlicStream(
                    FindActiveCutsceneFileResources(
                        g_pCutsceneShapeResources_004928a8),
                    index, g_pCurrentCutsceneSprite_00499c78);
            }
            if (g_pCurrentCutsceneSprite_00499c78->linkedScript != 0) {
                g_nActiveCutsceneResourceLevel_00499d9c =
                    g_nSavedCutsceneResourceOwner_005d2d68;
            }
            break;
        case 0x99:
            index = 0;
            while (*instruction == 0xff) {
                index = (short)(index + *instruction++);
            }
            index = (short)(index + *instruction++);
            fileResources = FindActiveCutsceneFileResources(
                g_pCutsceneShapeResources_004928a8);
            ReleaseLoadedCutsceneResource(fileResources, index);
            break;
        case 0x9a:
            g_pActiveCutsceneFileResources_005d2da4 =
                FindActiveCutsceneFileResources(
                g_pCutsceneShapeResources_004928a8);
            g_nCutsceneResourceScratch_005d2e9c =
                g_pActiveCutsceneFileResources_005d2da4->count;
            while (g_nCutsceneResourceScratch_005d2e9c-- != 0) {
                ReleaseLoadedCutsceneResource(
                    g_pActiveCutsceneFileResources_005d2da4,
                    g_nCutsceneResourceScratch_005d2e9c);
            }
            break;
        case 0x7b:
            if (g_bCutsceneViewportPreallocated_00499c4c == 0) {
                memberType = *instruction++;
                switch (memberType) {
                case 0:
                    if (g_pCurrentCutscenePlane_00499c7c == 0)
                        FatalCutsceneError("A33");
                    resourceIndex = *instruction++;
                    objectResources = FindActiveCutsceneObjectResources(
                        g_pCutsceneSpriteResources_0049288c);
                    runtimeIndex = objectResources->scriptSymbolIndices[
                        resourceIndex];
                    g_pCurrentCutscenePlane_00499c7c->spriteCount =
                        RemoveCutsceneMemberIndex(
                            g_pCurrentCutscenePlane_00499c7c->spriteCount,
                            g_pCurrentCutscenePlane_00499c7c->spriteIndices,
                            (signed char)runtimeIndex);
                    break;
                case 1:
                    if (g_pCurrentCutsceneSequence_00499c80 == 0)
                        FatalCutsceneError("A34");
                    resourceIndex = *instruction++;
                    objectResources = FindActiveCutsceneObjectResources(
                        g_pCutscenePlaneResources_00492890);
                    runtimeIndex = objectResources->scriptSymbolIndices[
                        resourceIndex];
                    g_pCurrentCutsceneSequence_00499c80->planeCount =
                        RemoveCutsceneMemberIndex(
                            g_pCurrentCutsceneSequence_00499c80->planeCount,
                            g_pCurrentCutsceneSequence_00499c80->planeIndices,
                            (signed char)runtimeIndex);
                    break;
                case 2:
                case 3:
                    instruction++;
                    break;
                case 4:
                    if (g_pCurrentCutscenePlane_00499c7c == 0)
                        FatalCutsceneError("A35");
                    g_pCurrentCutscenePlane_00499c7c->spriteCount = 0;
                    break;
                case 5:
                    if (g_pCurrentCutsceneSequence_00499c80 == 0)
                        FatalCutsceneError("A36");
                    g_pCurrentCutsceneSequence_00499c80->planeCount = 0;
                    break;
                case 6:
                    if (g_pCurrentCutsceneScene_00499c84 == 0)
                        FatalCutsceneError("A37");
                    g_pCurrentCutsceneScene_00499c84->sequenceCount = 0;
                    break;
                case 7:
                    break;
                }
            } else {
                instruction += 2;
            }
            break;
        case 0x79:
            if (g_bCutsceneViewportPreallocated_00499c4c == 0) {
                memberType = *instruction++;
                switch (memberType) {
                case 0:
                    if (g_pCurrentCutscenePlane_00499c7c == 0)
                        FatalCutsceneError("A38");
                    resourceIndex = *instruction++;
                    objectResources = FindActiveCutsceneObjectResources(
                        g_pCutsceneSpriteResources_0049288c);
                    g_pCurrentCutscenePlane_00499c7c->spriteIndices[
                        g_pCurrentCutscenePlane_00499c7c->spriteCount++] =
                        (unsigned char)objectResources->scriptSymbolIndices[
                            resourceIndex];
                    break;
                case 1:
                    if (g_pCurrentCutsceneSequence_00499c80 == 0)
                        FatalCutsceneError("A39");
                    resourceIndex = *instruction++;
                    objectResources = FindActiveCutsceneObjectResources(
                        g_pCutscenePlaneResources_00492890);
                    g_pCurrentCutsceneSequence_00499c80->planeIndices[
                        g_pCurrentCutsceneSequence_00499c80->planeCount++] =
                        (unsigned char)objectResources->scriptSymbolIndices[
                            resourceIndex];
                    break;
                case 2:
                    if (g_pCurrentCutsceneScene_00499c84 == 0)
                        FatalCutsceneError("A40");
                    resourceIndex = *instruction++;
                    objectResources = FindActiveCutsceneObjectResources(
                        g_pCutsceneSequenceResources_00492894);
                    g_pCurrentCutsceneScene_00499c84->sequenceIndices[
                        g_pCurrentCutsceneScene_00499c84->sequenceCount++] =
                        (unsigned char)objectResources->scriptSymbolIndices[
                            resourceIndex];
                    break;
                case 3:
                    instruction++;
                    break;
                }
            } else {
                instruction += 2;
            }
            break;
        case 0xac:
            value = *instruction++;
            otherValue = *instruction++;
            CopyCutsceneSpriteDisplay(value, otherValue);
            break;
        case 0xb7:
            CopyViewportContents(&g_stSceneFlicScratchViewport_005d2eb0,
                                 &g_stSecondaryViewBuffer_005d2c90);
            break;
        case 0xa8:
            value = *instruction++;
            otherValue = *instruction++;
            LinkCutsceneSpriteScript(value, otherValue);
            break;
        case 0x7a:
            memberType = *instruction++;
            switch (memberType) {
            case 0:
                resourceIndex = *instruction++;
                objectResources = FindActiveCutsceneObjectResources(
                    g_pCutsceneSpriteResources_0049288c);
                g_pLinkedCutsceneSprite_00499c64 =
                    g_apSceneObjects_00499c38[
                    objectResources->scriptSymbolIndices[resourceIndex]];
                if (g_pLinkedCutsceneSprite_00499c64 == 0)
                    FatalCutsceneError("A41");
                if (g_pLinkedCutsceneSprite_00499c64->linkedScript != 0) {
                    g_pLinkedCutsceneSprite_00499c64->scriptStart =
                        g_pLinkedCutsceneSprite_00499c64->linkedScript;
                    if (g_pLinkedCutsceneSprite_00499c64 ==
                            g_pCurrentCutsceneSprite_00499c78 &&
                        objectType == 0)
                        FatalCutsceneError("A42");
                    g_pLinkedCutsceneSprite_00499c64->linkedScript = 0;
                }
                if (g_pLinkedCutsceneSprite_00499c64 ==
                        g_pCurrentCutsceneSprite_00499c78 &&
                    objectType == 0) {
                    instruction =
                        g_pCurrentCutsceneSprite_00499c78->scriptStart;
                }
                g_pLinkedCutsceneSprite_00499c64->scriptCursor =
                    g_pLinkedCutsceneSprite_00499c64->scriptStart;
                g_pLinkedCutsceneSprite_00499c64->delayFrames = 0;
                g_pLinkedCutsceneSprite_00499c64->waitTicks = 0;
                ReleaseCutsceneSpriteShape(
                    g_pLinkedCutsceneSprite_00499c64);
                g_pLinkedCutsceneSprite_00499c64->owner =
                    g_pLinkedCutsceneSprite_00499c64->linkedOwner;
                break;
            case 1:
                resourceIndex = *instruction++;
                objectResources = FindActiveCutsceneObjectResources(
                    g_pCutscenePlaneResources_00492890);
                g_pLinkedCutscenePlane_00499c68 =
                    g_apCutscenePlanes_00499c3c[
                    objectResources->scriptSymbolIndices[resourceIndex]];
                if (g_pLinkedCutscenePlane_00499c68 == 0)
                    FatalCutsceneError("A43");
                if (g_pLinkedCutscenePlane_00499c68 ==
                        g_pCurrentCutscenePlane_00499c7c &&
                    objectType == 1) {
                    instruction =
                        g_pCurrentCutscenePlane_00499c7c->scriptStart;
                }
                g_pLinkedCutscenePlane_00499c68->scriptCursor =
                    g_pLinkedCutscenePlane_00499c68->scriptStart;
                break;
            case 2:
                resourceIndex = *instruction++;
                objectResources = FindActiveCutsceneObjectResources(
                    g_pCutsceneSequenceResources_00492894);
                g_pLinkedCutsceneSequence_00499c6c =
                    g_apCutsceneSequences_00499c40[
                    objectResources->scriptSymbolIndices[resourceIndex]];
                if (g_pLinkedCutsceneSequence_00499c6c == 0)
                    FatalCutsceneError("A44");
                if (g_pLinkedCutsceneSequence_00499c6c ==
                        g_pCurrentCutsceneSequence_00499c80 &&
                    objectType == 1) {
                    instruction =
                        g_pCurrentCutsceneSequence_00499c80->scriptStart;
                }
                g_pLinkedCutsceneSequence_00499c6c->scriptCursor =
                    g_pLinkedCutsceneSequence_00499c6c->scriptStart;
                break;
            case 3:
                instruction++;
                break;
            }
            break;
        case 0xa5:
            RunCutsceneWipeTransition(
                &g_stSceneFlicScratchViewport_005d2eb0,
                &g_stSecondaryViewBuffer_005d2c90,
                (int)(signed char)*instruction++,
                PopCutsceneScriptValue(&stack, stackStorage + 10));
            break;
        case 0xa6:
            resourceIndex = *instruction++;
            g_nCutsceneTextIndex_005d2ea4 = *(short *)instruction;
            instruction += 2;
            g_pszCutsceneSourceText_005d2dcc =
                g_pCutsceneTextResources_0049289c->entries[
                    g_nCutsceneTextIndex_005d2ea4];
            if (g_bCutsceneSkipFrame_00499c54 == 0) {
                UpdateMap(g_pszCutsceneSourceText_005d2dcc,
                          resourceIndex);
            }
            break;
        case 0xb6:
            ReleasePacketSlot(&g_pCutsceneCockpitPacket_00499c04);
            break;
        case 0xb5:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            LoadCutsceneCockpitOverlay(value);
            break;
        case 0xa7:
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
            otherValue = PopCutsceneScriptValue(
                &stack, stackStorage + 10);
            if (g_bCutsceneSkipFrame_00499c54 == 0)
                DrawCutsceneCockpitOverlay(otherValue, value);
            break;
        default:
            FatalCutsceneError("Illegal OpCode: 0x%2x (%d)",
                               opcode, opcode);
            break;
        }
    }
}

/* Function start: 0x432DCC */
char *GetPackedStringByIndex(CutsceneResourceTable *resources,
                             short index)
{
    char *text;
    short stringIndex;

    text = resources->packedFilenames;
    for (stringIndex = 0; stringIndex < index; stringIndex++) {
        text = DosStrchr(text, 0);
        text++;
    }
    return text;
}

/* Function start: 0x432E23 */
void RewriteDiskFileGraphicsExtensions(char *fileName)
{
    char *extensionPosition;

    if (g_cCutsceneVideoMode_00499c48 == 13) {
        extensionPosition = DosStrchr(fileName, '.');
#ifdef SDL_PORT
        if (extensionPosition != 0) {
            extensionPosition++;
            if (toupper((int)*extensionPosition) == 'V')
                *extensionPosition = 'E';
        }
#else
        if (extensionPosition++ != 0 &&
            toupper((int)*extensionPosition) == 'V')
            *extensionPosition = 'E';
#endif
    }
}

/* Function start: 0x432E83 */
void RewriteCutsceneMusicExtensionForDriver(char *filename)
{
    char *extension;

    extension = DosStrchr(filename, '.');
    if (extension++ != 0 && *extension != 0) {
        if (g_nMusicDriverMode_0049be8c == 2)
            *extension = 'A';
        else
            *extension = 'R';
    }
}

/* Function start: 0x432EE4 */
void RewriteCutsceneMusicExtensionForRetry(char *filename)
{
    char *extension;

    extension = DosStrchr(filename, '.');
    if (extension++ != 0 && *extension != 0)
        *extension = 'R';
}

/* Function start: 0x432F2A */
void *LoadCachedCutsceneResource(CutsceneResourceTable *resources,
                                 short index, int resourceType)
{
    short retryMusicExtension;
    long packetSize;
    char filename[132];
    char *packedFilename;
    unsigned short section;
    void *packet;

    packet = resources->loadedPackets[index];
    retryMusicExtension = 0;
    if (g_pMemoryLogFile_00499da8 != 0 && packet != 0) {
        fprintf(g_pMemoryLogFile_00499da8,
                "Found: loc: %Fp  level = %d, free = %8lu\n",
                packet, (int)g_nActiveCutsceneResourceLevel_00499d9c,
                GetAvailableMainMemory());
    }
    if (packet == 0) {
        if (g_nSceneFlicContext_00499c50 == 4 &&
            g_bHighMemoryResourcesEnabled_005c80e4 == 0) {
            g_nSceneFlicContext_00499c50 = 0x40;
        } else {
            packedFilename = GetPackedStringByIndex(
                resources, resources->filenameIndices[index]);
            DosStrcpy(filename, packedFilename);
            switch (resourceType) {
            case 1:
                RewriteDiskFileGraphicsExtensions(filename);
                break;
            case 2:
                RewriteCutsceneMusicExtensionForDriver(filename);
                retryMusicExtension = 1;
                break;
            }
            section = resources->sectionIndices[index];
            packet = LoadNamedPacket(filename, section, 0,
                (unsigned short)(g_nSceneFlicContext_00499c50 & 0x7f),
                0, 1);
            if (packet == 0 && g_nPacketError_0049ca90 == 8) {
                if (retryMusicExtension != 0) {
                    retryMusicExtension = 0;
                    RewriteCutsceneMusicExtensionForRetry(filename);
                    packet = LoadNamedPacket(filename, section, 0,
                        (unsigned short)(
                            g_nSceneFlicContext_00499c50 & 0x7f),
                        0, 1);
                    if (packet == 0) {
                        ReportCutscenePacketAllocationFailure(
                            filename, section, 0);
                    }
                } else {
                    ReportCutscenePacketAllocationFailure(
                        filename, section, 0);
                }
            } else if (packet == 0 &&
                       (g_nSceneFlicContext_00499c50 & 0x40) != 0) {
                FatalCutsceneError(
                    "Unsuccessful load of %s packet %d error = %d",
                    filename, section, g_nPacketError_0049ca90);
            }
            if (packet == 0) {
                ReportCutscenePacketAllocationFailure(
                    filename, section,
                    GetNamedPacketSize(filename, section));
            }
            if (g_pMemoryLogFile_00499da8 != 0) {
                packetSize = GetNamedPacketSize(filename, section);
                fprintf(g_pMemoryLogFile_00499da8,
                        "Loaded: %12s (%03d) size %ld loc: %Fp  "
                        "level = %d, free = %8lu\n",
                        filename, section, packetSize, packet,
                        (int)g_nActiveCutsceneResourceLevel_00499d9c,
                        GetAvailableMainMemory());
            }
            g_nSceneFlicContext_00499c50 = 0x40;
            resources->loadedPackets[index] = packet;
        }
    }
    return packet;
}

/* Function start: 0x433269 */
void ReleaseCutsceneSpriteShape(SceneFlicObject *sprite)
{
    SceneFlicObject *other;
    void *shape;
    short index;

    shape = sprite->shape;
    g_nSavedCutsceneResourceOwner_005d2d68 =
        g_nActiveCutsceneResourceLevel_00499d9c;
    if (shape != 0) {
        for (index = 0; index < 0x80; index++) {
            other = g_apSceneObjects_00499c38[index];
            if (other != 0 && other->shape == shape && other != sprite) {
                goto shape_is_shared;
            }
        }
    }
    g_nActiveCutsceneResourceLevel_00499d9c = sprite->owner;
    ReleaseActiveCutscenePacket(g_pCutsceneShapeResources_004928a8, shape);
shape_is_shared:
    g_nActiveCutsceneResourceLevel_00499d9c =
        g_nSavedCutsceneResourceOwner_005d2d68;
    sprite->shape = 0;
}

/* Function start: 0x433328 */
void ReleaseLoadedCutsceneResource(CutsceneResourceTable *resources,
                                   short index)
{
    SceneFlicObject *sprite;
    void *packet;
    short spriteIndex;

    packet = resources->loadedPackets[index];
    ReleasePacketSlot(&resources->loadedPackets[index]);
    if (g_pMemoryLogFile_00499da8 != 0 && packet != 0)
        fprintf(g_pMemoryLogFile_00499da8,
                "--- Dumped: %Fp  level = %d free = %8lu\n", packet,
                (int)g_nActiveCutsceneResourceLevel_00499d9c,
                GetAvailableMainMemory());
    if (packet != 0) {
        for (spriteIndex = 0; spriteIndex < 0x80; spriteIndex++) {
            sprite = g_apSceneObjects_00499c38[spriteIndex];
            if (sprite != 0 && sprite->shape == packet)
                sprite->shape = 0;
        }
    }
}

/* Function start: 0x4333F8 */
int MapCutsceneSoundEffectNumber(int soundIndex)
{
    return soundIndex + 1;
}
