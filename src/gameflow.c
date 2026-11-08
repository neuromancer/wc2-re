/*
 *  Cutscene engine and game flow.
 *
 *  Retail object 0x42BB70-0x43340F (link order; see docs/ORDER.md).
 */
#define GAMEFLOW_C
#include "game.h"

static char g_szLastCutsceneWorkBuffer[16]; /* 0x00499ec0 */
static char g_szLastCutscenePrintBuffer[12]; /* 0x00499ed0 */

unsigned char g_bMemoryLogToFile = 0; /* 0x00499bf8 */
signed char g_bAutopilotDebugEnabled = 0; /* 0x00499bfc */
signed char g_bRoomTransitionAnimationEnabled = 1; /* 0x00499c00 */
void *g_pCutsceneCockpitPacket = 0; /* 0x00499c04 */
void *g_pSceneMusicPacket = 0; /* 0x00499c08 */
void *g_pCutsceneCockpitPalette = 0; /* 0x00499c0c */
short g_nCutsceneSpritesDrawn = 0; /* 0x00499c10 */
const char *g_apszCutsceneObjectTypeNames[4] = {
    "sprite",
    "plane",
    "sequence",
    "script"
}; /* 0x00499c18 */

#ifdef SDL_PORT
/* Not an original engine flag -- tracks whether any speech clip has
 * actually played yet in the cutscene currently running (reset once per
 * RunLoadedCutscene call, set the first time AnimateCutsceneSpeakerMouth
 * observes g_bSpeechSoundActive go active). Most cutscenes have
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
    (*(signed char *)&g_nShowMemoryStatus)

/* Forward declarations of the script and resource-cache routines defined
 * further down.  Their named parameters also give this unit retail's
 * symbol-key numbering (each named parameter takes one key). */
void ExecuteCutsceneScene(CutsceneScene *scene);
signed char RunCutsceneScript(unsigned char **scriptCursor, short objectType);
char *GetPackedStringByIndex(CutsceneResourceTable *resources, short index);
void RewriteDiskFileGraphicsExtensions(char *fileName);
void *LoadCachedCutsceneResource(CutsceneResourceTable *resources,
                                 short index, int resourceType);
void ReleaseCutsceneSpriteShape(SceneFlicObject *sprite);
void ReleaseLoadedCutsceneResource(CutsceneResourceTable *resources,
                                   short index);
int MapCutsceneSoundEffectNumber(int soundIndex);

#pragma function(memcpy)
/* Function start: 0x42BB70 */
void AnimateCutsceneSpeakerMouth(SceneFlicObject *sprite)
{
    signed char ticks;
    signed char frame;
    signed char speaker;
    signed char speechSpeed;

    speechSpeed = g_cCutsceneSpeechSpeed;
    if (g_2_bMovieStringAdvanceState == 0) {
        g_pszCutsceneSpeechCursor = 0;
        return;
    }
#ifdef SDL_PORT
    /* The two gates below have no original-engine equivalent -- the
     * reference build always animated the mouth straight from text,
     * regardless of audio. Kept port-only so the reference build stays
     * byte-comparable to the original for verification; see the PR
     * discussion this note is based on. */
    if (g_bSpeechSoundActive != 0) {
        g_bCutsceneSpeechAudioSeen = 1;
    } else if (g_bCutsceneSpeechAudioSeen != 0) {
        /* Opcode 0x8a arms text-advance independently of when the paired
         * speech clip (opcode 0xb0) actually starts producing audio --
         * 0xb0's own cache miss falls back to a synchronous blocking disk
         * load (LoadAndPlaySpeechPacket, music.c). Hold the current frame
         * instead of animating from text alone until
         * g_bSpeechSoundActive is set, which happens only once
         * real playback begins (PlayRawSpeechBuffer, sound.c). Only applies
         * once this cutscene is known to have voice audio at all (see
         * g_bCutsceneSpeechAudioSeen) -- most cutscenes have none, and
         * text-driven animation with no audio to wait for is correct for
         * those, same as the original engine always did. */
        return;
    }
    if (g_pSpeechSnd != 0 &&
        ix_sound_is_playing(g_pSpeechSnd) == 0) {
        /* g_bSpeechSoundActive clears only after
         * ServiceSoundSystem's (sound.c) grace period, which exists to
         * decide when to force-advance the script, not whether the mouth
         * should keep moving. Check the sound object's own playing state
         * directly instead, so the mouth stops the instant playback
         * actually stops. */
        return;
    }
#endif
    if (g_bCutsceneSkipFrame != 0 ||
        g_bCutsceneViewportPreallocated != 0) {
        g_bCutsceneSpeechActive =
            (signed char)(g_2_bMovieStringAdvanceState = 0);
        g_pszCutsceneSpeechCursor = 0;
        sprite->currentFrame = 11;
        return;
    }
    if (g_wDigiPlaying != 0 ||
        g_bCutsceneSpeechActive != 0) {
        if (g_wDigiPlaying == 0) {
            g_bCutsceneSpeechActive =
                (signed char)(g_2_bMovieStringAdvanceState = 0);
            sprite->currentFrame = 11;
            g_pszCutsceneSpeechCursor = 0;
            return;
        }
        g_bCutsceneSpeechActive = 1;
    }
    if (g_wDigiPlaying != 0)
        speechSpeed = 0;
    sprite->waitStart = g_196_nInputTimerTicks;
    if (g_pszCutsceneSpeechCursor == 0)
        g_pszCutsceneSpeechCursor = g_pszCurrentCutsceneText;
    speaker = (signed char)toupper(*g_pszCutsceneSpeechCursor);
    if (speaker < 0) {
        speaker = (signed char)(speaker + 0x80);
        frame = g_acCutsceneMouthFrames[speaker];
        ticks = g_acCutsceneMouthDurations[speaker];
    } else if (speaker < ' ') {
        if (speaker == 0) {
            g_bCutsceneSpeechActive =
                (signed char)(g_2_bMovieStringAdvanceState = 0);
            g_pszCutsceneSpeechCursor = 0;
        }
        frame = -1;
    } else if (speaker == 'T' &&
               toupper(*g_pszCutsceneSpeechCursor) == 'H') {
        frame = 8;
        ticks = 3;
        g_pszCutsceneSpeechCursor++;
    } else {
        frame = g_acCutsceneMouthFrames[speaker];
        ticks = g_acCutsceneMouthDurations[speaker];
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
                     (int)sprite->waitTicks, (int)g_196_nInputTimerTicks,
                     g_17_nFramePeriodMillis,
                     (int)g_wDigiPlaying,
                     (int)g_bSpeechSoundActive);
#endif
    } else {
        sprite->currentFrame = 11;
        sprite->waitTicks = (short)(speechSpeed * RandomInRange(2, 4));
    }
    if (g_pszCutsceneSpeechCursor != 0 &&
        *g_pszCutsceneSpeechCursor != 0) {
        g_pszCutsceneSpeechCursor++;
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

    previousFile = g_pMemoryLogFile;
    previousFileMode = g_bMemoryLogToFile;
    g_bMemoryLogToFile = 1;
    g_pMemoryLogFile = file;
    WriteMemoryStateReportHook();
    g_pMemoryLogFile = previousFile;
    g_bMemoryLogToFile = previousFileMode;
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
    if (g_pMemoryLogFile != 0) {
        mainAvail = GetAvailableMainMemory();
        mainLargest = GetLargestMainMemoryBlock();
        farFree = GetAvailableFarMemory();
        farBlock = GetLargestFreeMemoryBlock();
        nearTotal = GetOriginalFreeMemory();
        fprintf(g_pMemoryLogFile,
                "----- Mem State ------\n");
        fprintf(g_pMemoryLogFile,
                "NEAR: %ud\n", nearTotal);
        fprintf(g_pMemoryLogFile,
                "FAR:  %6ld  Largest: %6ld\n",
                mainAvail, mainLargest);
        fprintf(g_pMemoryLogFile,
                "EMS:  %6ld  Largest: %6ld\n",
                farFree, farBlock);
        fprintf(g_pMemoryLogFile,
                "----------------------\n");
    }
}

#pragma function(memcmp, strlen)
/* Function start: 0x42BF0C */
void LogChangedCutsceneWorkBuffers(void)
{
    if (memcmp(g_szLastCutsceneWorkBuffer,
               g_4_pszFilmWorkScratch,
               strlen(g_szLastCutsceneWorkBuffer)) != 0 ||
        memcmp(g_szLastCutscenePrintBuffer,
               g_116_pszFilmPrintStore,
               strlen(g_szLastCutscenePrintBuffer)) != 0) {
        fprintf(g_pMemoryLogFile,
                "Show Work: %.10s...  %.10s...  %.10s...\n",
                g_4_pszFilmWorkScratch,
                g_116_pszFilmPrintStore,
                g_27_pszCutsceneFormattedText);
        memcpy(g_szLastCutsceneWorkBuffer,
               g_4_pszFilmWorkScratch, 10);
        memcpy(g_szLastCutscenePrintBuffer,
               g_116_pszFilmPrintStore, 10);
    }
}

/* Function start: 0x42BFB8 */
void RouteCutsceneViewportToDisplay(void)
{
    unsigned char *firstRow;

    firstRow = 0;
    if (g_stSecondaryViewBufferBlock.pixels != 0 &&
        g_cCutsceneVideoMode == 0x13 &&
        g_bCutsceneViewportPreallocated == 0) {
        firstRow = g_stSecondaryViewBufferBlock.pixels +
            g_stSecondaryViewBufferBlock.rowOffsets[0];
        WriteDetailedMemoryStateReport();
        ReleasePacketSlot(&g_pCutsceneCockpitPalette);
        if (g_pMemoryLogFile != 0)
            fprintf(g_pMemoryLogFile, "Rerouted LZ.\n");
        WriteDetailedMemoryStateReport();
    }
    g_13_pActiveCutscenePixelsAddress = firstRow;
}

/* Function start: 0x42C04B */
void ClearActiveCutscenePixelAlias(void)
{
    unsigned char *firstRow;

    if (g_stSecondaryViewBufferBlock.pixels != 0 &&
        g_cCutsceneVideoMode == 0x13) {
        firstRow = g_stSecondaryViewBufferBlock.pixels +
            g_stSecondaryViewBufferBlock.rowOffsets[0];
        if (g_13_pActiveCutscenePixelsAddress == firstRow)
            g_13_pActiveCutscenePixelsAddress = 0;
    }
}

/* Function start: 0x42C0A2 */
signed char IsCutsceneSpeechLoaded(void)
{
    short slot;

    for (slot = 0; slot < 8; slot++) {
        if (g_12_apszMovieSpeechFiles[slot] != 0)
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

    g_16_stCutsceneDrawingTextSettingsInfo =
        g_2_stCutsceneMessageCtxInfo;
    switch (viewportIndex) {
    case 0:
        g_16_stCutsceneDrawingTextSettingsInfo.viewport =
            &g_15_stSceneFlicScratchViewport;
        break;
    case 1:
        g_16_stCutsceneDrawingTextSettingsInfo.viewport =
            &g_stSecondaryViewBufferBlock;
        break;
    case 2:
        g_16_stCutsceneDrawingTextSettingsInfo.viewport =
            &g_10_stCutsceneStringViewportBlock;
        break;
    case 3:
        g_16_stCutsceneDrawingTextSettingsInfo.viewport =
            &g_9_stCutsceneTextBackingPort;
        break;
    case 4:
        g_16_stCutsceneDrawingTextSettingsInfo.viewport =
            &g_2_stModalSourceWindow;
        break;
    }
    SetTextContext(&g_16_stCutsceneDrawingTextSettingsInfo);
    savedBounds =
        *(ShortRect *)&g_16_stCutsceneDrawingTextSettingsInfo.viewport->left;
    if (g_bCutsceneTextBoundsOverride != 0) {
        g_16_stCutsceneDrawingTextSettingsInfo.viewport->left =
            MaxShort(g_stCutsceneTextBounds.left,
                     g_16_stCutsceneDrawingTextSettingsInfo.viewport->left);
        g_16_stCutsceneDrawingTextSettingsInfo.viewport->top =
            MaxShort(g_stCutsceneTextBounds.top,
                     g_16_stCutsceneDrawingTextSettingsInfo.viewport->top);
        g_16_stCutsceneDrawingTextSettingsInfo.viewport->right =
            MinShort(g_stCutsceneTextBounds.right,
                     g_16_stCutsceneDrawingTextSettingsInfo.viewport->right);
        g_16_stCutsceneDrawingTextSettingsInfo.viewport->bottom =
            MinShort(g_stCutsceneTextBounds.bottom,
                     g_16_stCutsceneDrawingTextSettingsInfo.viewport->bottom);
    }
    if (g_9_bCinematicSpriteTextEnableStatus == 0) {
        if (x > -1) {
            g_29_pCurrentTextContext->cursorX =
                (short)(g_29_pCurrentTextContext->viewport->left + x);
        }
        if (y > -1) {
            g_29_pCurrentTextContext->cursorY =
                (short)(g_29_pCurrentTextContext->viewport->top + y);
        }
    } else {
        g_29_pCurrentTextContext->cursorX =
            (short)(g_29_pCurrentTextContext->viewport->left + x);
        g_29_pCurrentTextContext->cursorY =
            (short)(g_29_pCurrentTextContext->viewport->top + y);
    }
#ifdef SDL_PORT
    if (memcmp(text, "50", 2) != 0) {
#else
    if ((IsCutsceneSpeechLoaded() == 0 ||
         g_27_pszCutsceneFormattedText == text ||
         g_nAudioEnabled == 0) &&
        memcmp(text, "50", 2) != 0) {
#endif
        FormatTextBufferFromStart("%s%P", text);
    }
    g_bCutsceneTextRestorePending = 1;
    SetTextContext(&g_2_stCutsceneMessageCtxInfo);
    *(ShortRect *)&g_16_stCutsceneDrawingTextSettingsInfo.viewport->left =
        savedBounds;
    g_bCutsceneTextBoundsOverride = 0;
}

/* Function start: 0x42C373 */
void LoadCutsceneCockpitOverlay(short scriptValue)
{
    if (g_pCutsceneCockpitPacket == 0) {
        g_pCutsceneCockpitPacket = LoadNamedPacket(
            g_7_szCockpitAssetFilenameText, 8, 0, 0, 0, 1);
    }
    (void)scriptValue;
}

/* Function start: 0x42C3A7 */
void DrawCutsceneCockpitOverlay(short unused, short verticalOffset)
{
    short hits;
    short x;
    short y;

    if (g_pCutsceneCockpitPacket != 0) {
        for (hits = 0; hits < 4; hits++) {
            if (g_69_asCockpitDmgStateList[hits] == 1) {
                x = g_aCockpitDamagePositions[hits].x;
                y = (short)(
                    g_aCockpitDamagePositions[hits].y +
                    verticalOffset);
                DrawSpriteDefault(&g_stSecondaryViewBufferBlock,
                                  x, y,
                                  g_pCutsceneCockpitPacket,
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
    g_bRoomTransitionAnimationEnabled = 1;
    if (g_bCutsceneSkipFrame == 0 &&
        g_bCutsceneViewportPreallocated == 0) {
        if (g_cCutsceneVideoMode == 0x13) {
            if (g_bRoomTransitionAnimationEnabled != 0)
                workspace = malloc(0xce);
            if (workspace == 0) {
                if (g_pMemoryLogFile != 0)
                    fprintf(g_pMemoryLogFile,
                            "Unable to allocate Wipe.\n");
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
                } else if (g_pMemoryLogFile != 0) {
                    fprintf(g_pMemoryLogFile,
                            "Unable to allocate Wipe (1).\n");
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
    if (FindQueuedInputEvent(INPUT_EVENT_BUTTON_UP) == 0) {
        for (;;) {
            if (ServiceInputDevices(-1) != 0) {
                ServiceCutsceneRuntimeHook();
                if (FindQueuedInputEvent(INPUT_EVENT_BUTTON_UP) != 0 ||
                    FindQueuedInputEvent(INPUT_EVENT_KEY_UP) != 0)
                    break;
            }
#ifdef SDL_PORT
            /* See issue #33.  Escape is the one input this loop can
             * never see: it only exits on a queued mouse-up (2) or
             * key-up (5), and once g_bSceneEscapeRequested is
             * set the ServiceInputDevices() poll returns 1 immediately
             * (eventmgr.c) -- so if the Escape key-up event is flushed
             * before this loop looks for it (ServiceInputDevices()
             * flushes the queue on its poll-tick path, and returns 1
             * from it), the loop spins at full CPU with no way out but
             * another keypress.  Adding an exit can only shorten the
             * wait, never block one: the caller returns to the
             * interpreter loop, whose top-of-loop check converts the
             * pending Escape. */
            if (g_bSceneEscapeRequested != 0)
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
        if (resources->owner == g_nActiveCutsceneResourceLevel)
            return resources;
        else
            resources = resources->next;
    }
    FatalCutsceneError("A1");
#ifdef SDL_PORT
    return 0;
#endif
}

/* Function start: 0x42C659 */
CutsceneObjectResourceList *FindActiveCutsceneObjectResources(
    CutsceneObjectResourceList *resources)
{
    while (resources != 0) {
        if (resources->owner == g_nActiveCutsceneResourceLevel)
            return resources;
        else
            resources = resources->next;
    }
    FatalCutsceneError("A2");
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
            DosMemmove(indices, &indices[member + 1], (size_t)(15 - member));
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
    vsprintf(g_4_pszFilmWorkScratch, format, arguments);
#ifdef SDL_PORT
    va_end(arguments);
#endif
    exit_squadron(g_4_pszFilmWorkScratch);
}

/* Function start: 0x42C75D */
void ResetCutsceneSpriteDrawTicks(void)
{
    short index;

    for (index = 0; index < 0x80; index++) {
        if (g_apSceneObjects[index] != 0)
            g_apSceneObjects[index]->tick = 0;
    }
    g_nCutsceneSpritesDrawn = 0;
}

/* Function start: 0x42C7BC */
void PresentCutsceneFrame(Viewport *source, Viewport *destination)
{
    Viewport debugViewport;
    unsigned short debugFrameNumber;

    if (g_cCutsceneVideoMode != 0x13 &&
        g_cCutsceneVideoMode != 0x0d)
        return;
    if (g_bCutsceneDebugOverlayEnabled != 0) {
        debugViewport = g_2_stModalSourceWindow;
        debugViewport.bottom = 0x17;
        g_6_stMovieDebugTextCtx =
            g_2_stCutsceneMessageCtxInfo;
        g_6_stMovieDebugTextCtx.viewport = &debugViewport;
        g_6_stMovieDebugTextCtx.alignment = 0;
        g_6_stMovieDebugTextCtx.cursorY = 0;
        g_6_stMovieDebugTextCtx.cursorX = 0;
        SetTextContext(&g_6_stMovieDebugTextCtx);
        debugFrameNumber = g_wCutsceneDebugFrameNumber++;
        DrawFormattedText(
            "%X%Y%d Delay: %d DrawsCalled: %d * %d Time: %U   ",
            0, 0, debugFrameNumber, g_nCutsceneFrameDelay,
            g_nCutsceneSpritesDrawn,
            g_bMovieFramePresented,
            g_196_nInputTimerTicks);
    }
    if (g_nFrameSkipCountdown == 0) {
        ConsumeCutscenePalettePacket(0, 0x100, 1);
        CopyViewportContents(source, destination);
        ResetCutsceneSpriteDrawTicks();
    }
    SetTextContext(&g_2_stCutsceneMessageCtxInfo);
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
    if (g_26_pFilmRgbPacket == 0)
        return;
    if (ReadNextSceneForm(&g_26_pFilmRgbPacket, &nextChunk) ==
        0x204c4150) {
        chunk = g_26_pFilmRgbPacket;
        g_26_pFilmRgbPacket += 4;
        if (*(unsigned int *)chunk == 0x50414d43) {
            size = *(unsigned int *)g_26_pFilmRgbPacket;
            g_26_pFilmRgbPacket += 4;
            SwapSceneChunkSizeEndian((int *)&size);
            if (size / 3 != 0x100)
                malformed++;
        } else {
            malformed++;
        }
        if (malformed != 0)
            FatalCutsceneError("A3");
    }
    if (releasePacket != 0)
        ReleasePacketSlot((void **)&g_26_pFilmRgbPacket);
}

/* Function start: 0x42C9B0 */
/* Returns the playing sound effect slot of this resource: the third slot
 * found for it is stopped and reused; otherwise 0. */
CutsceneSoundEffect *FindCutsceneSoundEffect(short resourceIndex)
{
    CutsceneSoundEffect *effect;
    short count;

    effect = g_pCutsceneSoundEffects;
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

    if (g_bCutsceneSkipFrame != 0 ||
        g_bCutsceneViewportPreallocated != 0 ||
        g_bCutsceneSkipAll != 0)
        return;
    effect = FindCutsceneSoundEffect(resourceIndex);
    if (effect != 0) {
        effect->sound = sound;
        return;
    }
    effect = (CutsceneSoundEffect *)calloc(1, sizeof(CutsceneSoundEffect));
    if (effect == 0)
        FatalCutsceneError("A5");
    if (g_pMemoryLogFile != 0) {
        fprintf(g_pMemoryLogFile, "Alloc FX: %p free: %d\n",
                effect, (int)(short)GetOriginalFreeMemory());
    }
    effect->resourceIndex = resourceIndex;
    effect->sound = sound;
    effect->next = g_pCutsceneSoundEffects;
    g_pCutsceneSoundEffects = effect;
}

/* Function start: 0x42CB08 */
void ReleaseCutsceneSoundEffects(short resourceIndex)
{
    CutsceneSoundEffect *previous;
    CutsceneSoundEffect *effect;

    previous = 0;
    effect = g_pCutsceneSoundEffects;
    if (g_nMusicDriverMode != 1 && g_nMusicDriverMode != 2)
        return;
    if (resourceIndex == -1) {
        while (effect != 0) {
            ((void (__cdecl *)(void *, int))FlushSoundEffect)(
                effect->sound, 0);
            previous = effect->next;
            free(effect);
            effect = previous;
        }
        g_pCutsceneSoundEffects = 0;
    } else {
        while (effect != 0) {
            if (effect->resourceIndex == resourceIndex) {
                ((void (__cdecl *)(void *, int))FlushSoundEffect)(
                    effect->sound, 0);
                if (previous != 0)
                    previous->next = effect->next;
                else
                    g_pCutsceneSoundEffects = effect->next;
                free(effect);
                if (g_pMemoryLogFile != 0) {
                    fprintf(g_pMemoryLogFile,
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

    if (g_bSpeechEnabledFlag == 0 ||
        g_bCutsceneViewportPreallocated != 0 ||
        g_bCutsceneSkipAll != 0)
        return;
    resourceTable = FindActiveCutsceneFileResources(
        g_pCutsceneSpeechResources);
#ifdef SDL_PORT
    /* See issue #33 (review on PR #34): bailing out here while the
     * cache is still busy must not fall through to releasing/replacing
     * this slot below -- a still-in-flight fetch may be filling it.
     * g_wDigiPlaying is never set nonzero anywhere in
     * this reconstruction today, so this loop cannot actually break
     * early yet -- guarded correctly anyway rather than leaving
     * unsound-if-ever-live code here. */
    while (g_wDigiPlaying != 0) {
        PumpWindowMessages(0);
        if (g_bSceneEscapeRequested != 0)
            return;
    }
#else
    while (g_wDigiPlaying != 0)
        PumpWindowMessages(0);
#endif
    ReleasePacketSlot(&g_3_apSceneSpeechPacketsList[slot]);
    g_1_asCutsceneSpeechChannelsTable[slot] = 0;
    g_12_apszMovieSpeechFiles[slot] =
        GetPackedStringByIndex(
            resourceTable, resourceTable->filenameIndices[resourceIndex]);
    g_19_asCutsceneSpeechSectionsList[slot] =
        resourceTable->sectionIndices[resourceIndex];
    packetSize = (int)GetNamedPacketSize(
        g_12_apszMovieSpeechFiles[slot],
        g_19_asCutsceneSpeechSectionsList[slot]);
    if (packetSize <= 0) {
        g_12_apszMovieSpeechFiles[slot] = 0;
        g_19_asCutsceneSpeechSectionsList[slot] = 0;
    } else {
        g_3_apSceneSpeechPacketsList[slot] =
            LoadSpeechPacketIntoBuffer(
                g_12_apszMovieSpeechFiles[slot],
                g_19_asCutsceneSpeechSectionsList[slot], 0);
        if (g_3_apSceneSpeechPacketsList[slot] != 0) {
            g_1_asCutsceneSpeechChannelsTable[slot] =
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
    if (g_nMusicDriverMode != 1 &&
        g_nMusicDriverMode != 2)
        return;
    if (g_19_bStoredHighMemoryEnabled == 0)
        return;
    if (head == 0)
        return;
    if (HasCutsceneMusicNode(node) == 0) {
        allocated = (CutsceneMusicNode *)calloc(1,
                                                sizeof(CutsceneMusicNode));
        if (allocated == 0)
            FatalCutsceneError("A6");
        if (g_pMemoryLogFile != 0) {
            fprintf(g_pMemoryLogFile,
                    "Alloc Music: %p free: %d\n", allocated,
                    (int)(short)GetOriginalFreeMemory());
        }
        node = allocated;
        node->resourceIndex = (unsigned char)resourceIndex;
        node->packet = LoadCachedCutsceneResource(
            FindActiveCutsceneFileResources(
                g_pCutsceneMusicResources),
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
        if (g_pMemoryLogFile != 0) {
            fprintf(g_pMemoryLogFile,
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
                g_pCutsceneMusicResources),
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
    if (g_nMusicDriverMode != 1 &&
        g_nMusicDriverMode != 2)
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
                    g_pCutsceneMusicResources),
                node->resourceIndex);
            free(node);
            if (g_pMemoryLogFile != 0) {
                fprintf(g_pMemoryLogFile,
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

    node = g_pCutsceneMusicPackets;
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
    if (g_nCutsceneResourceLevel == 0) {
        if (g_bAutopilotDebugEnabled != 0 &&
            g_bSpeechEnabledFlag == 0)
            g_pMemoryLogFile = fopen("logger.$$$", "w");
        g_bCutsceneDrawingEnabled = 0;
        g_bMemoryLogToFile = 1;
        WriteDetailedMemoryStateReport();
        g_bMemoryLogToFile = 0;
        InitializeCinematicTextRenderer();
        g_nCutsceneFrameDelay = g_nCutsceneFrameTick = 0;
        g_28_nCutsceneInitialAvailableMemoryWord =
            GetAvailableMainMemory();
        g_17_nSceneInitialLargestBlockWord =
            GetLargestMainMemoryBlock();
        /* The retail image sets and immediately clears this flag. */
        g_bSceneCockpitLoaded = 1;
        g_bSceneCockpitLoaded = 0;
        if (g_apSceneObjects == 0) {
            g_apSceneObjects = AllocateScenePointerTable(
                0x80, HOST_POINTER_SIZE, 0,
                "Cannot alloc SpritesArray");
        }
        if (g_apCutscenePlanes == 0) {
            g_apCutscenePlanes = AllocateScenePointerTable(
                0x40, HOST_POINTER_SIZE, 0,
                "Cannot alloc PlanesArray");
        }
        if (g_apCutsceneSequences == 0) {
            g_apCutsceneSequences = AllocateScenePointerTable(
                0x100, HOST_POINTER_SIZE, 0,
                "Cannot alloc SequenceArray");
        }
        if (g_apCutsceneScenes == 0) {
            g_apCutsceneScenes = AllocateScenePointerTable(
                0x20, HOST_POINTER_SIZE, 0,
                "Cannot alloc ScriptArray");
        }
        if (g_4_pszFilmWorkScratch == 0) {
            WriteDetailedMemoryStateReport();
            g_4_pszFilmWorkScratch = calloc(0x100, 1);
            if (g_4_pszFilmWorkScratch == 0)
                FatalCutsceneError("A8");
            if (g_pMemoryLogFile != 0)
                fprintf(g_pMemoryLogFile,
                        "Alloc ___workstring: %p free: %d\n",
                        g_4_pszFilmWorkScratch,
                        (int)GetOriginalFreeMemory());
        }
        if (g_116_pszFilmPrintStore == 0) {
            WriteDetailedMemoryStateReport();
            g_116_pszFilmPrintStore = calloc(0x100, 1);
            if (g_116_pszFilmPrintStore == 0)
                FatalCutsceneError("A9");
            if (g_pMemoryLogFile != 0)
                fprintf(g_pMemoryLogFile,
                        "Alloc ___printstring: %p free: %d\n",
                        g_116_pszFilmPrintStore,
                        (int)GetOriginalFreeMemory());
        }
    }
}

/* Function start: 0x42D444 */
void InitializeCutsceneViewports(void)
{
    if (g_15_stSceneFlicScratchViewport.pixels == 0) {
        g_10_stCutsceneStringViewportBlock =
            g_2_stModalSourceWindow;
        g_15_stSceneFlicScratchViewport =
            g_10_stCutsceneStringViewportBlock;
    }
    if (g_2_stCutsceneMessageCtxInfo.viewport == 0) {
        g_2_stCutsceneMessageCtxInfo.viewport =
            &g_9_stCutsceneTextBackingPort;
        g_2_stCutsceneMessageCtxInfo.text =
            g_16_szTextBufferBuffer;
        g_2_stCutsceneMessageCtxInfo.colour = 15;
        g_2_stCutsceneMessageCtxInfo.backgroundColour = 0;
        g_2_stCutsceneMessageCtxInfo.alignment = 2;
        SetTextContext(&g_2_stCutsceneMessageCtxInfo);
    }
}

/* Function start: 0x42D4C1 */
void ReleaseSceneMusicPacket(void)
{
    if (g_pSceneMusicPacket != 0) {
        StopMusic(0);
        /* Track 44's slot of the music table holds the scene music. */
        g_aTuneResources[44].packet =
            (void *)g_4_dwSceneMusicMode;
        ReleasePacketSlot(&g_pSceneMusicPacket);
    }
}

/* Function start: 0x42D4FA */
void ReleaseCutsceneSpeechPackets(void)
{
    short index;

    for (index = 0; index < 8; index++) {
        ReleasePacketSlot(&g_3_apSceneSpeechPacketsList[index]);
        g_12_apszMovieSpeechFiles[index] = 0;
        g_19_asCutsceneSpeechSectionsList[index] = 0;
        g_1_asCutsceneSpeechChannelsTable[index] = 0;
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
     * g_pSpeechSnd alone being non-null -- it does not care
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
    g_pSpeechSnd = 0;
    g_bSpeechSoundActive = 0;
    g_16_nSpeechCompletionDelay = 0;
    g_bCutsceneSpeechAudioSeen = 0;
#endif
    savedTextContext = g_29_pCurrentTextContext;
    savedInputPollPeriod = g_nInputPollPeriod;
    previousMemoryStatus = g_cShowMemoryStatus;
    g_cShowMemoryStatus = 1;
    FlushSoundEffects();
    g_nInputPollPeriod = 1;
#ifdef SDL_PORT
    /* A menu's old polling deadline must not delay cutscene skip input. */
    g_nNextInputPollTick = g_196_nInputTimerTicks;
#endif
    g_cCutsceneVideoMode =
        (signed char)QueryCurrentGraphicsMode();
    ClearCutsceneViewport(&g_2_stModalSourceWindow, 0);
    g_2_stCutsceneMessageCtxInfo.viewport = 0;
    g_47_nCutsceneMemDelta =
        (int)(g_28_nCutsceneInitialAvailableMemoryWord -
              GetAvailableMainMemory());
    g_bCutsceneSkipAll = 0;
    g_bMemoryLogToFile = 1;
    WriteDetailedMemoryStateReport();
    g_bMemoryLogToFile = 0;
    if (g_pMemoryLogFile != 0) {
        fprintf(g_pMemoryLogFile,
                "************************************\n"
                "Initial %ld Overhead %ld\n",
                g_28_nCutsceneInitialAvailableMemoryWord,
                g_47_nCutsceneMemDelta);
    }
    ReleaseSceneMusicPacket();
    ExecuteCutsceneScene(g_apCutsceneScenes[
        *FindActiveCutsceneObjectResources(
            g_pCutsceneSceneResources)->scriptSymbolIndices]);
    g_bCutsceneSkipAll = 0;
    g_bSceneEscapeRequested = 0;
    ReleaseCutsceneSpeechPackets();
    FlushSoundEffects();
    ReleaseSceneFlicPackets();
    ReleaseCutsceneResourceLevel(g_nCutsceneResourceLevel);
    ReleasePacketSlot((void **)&g_apSceneObjects);
    ReleasePacketSlot((void **)&g_apCutscenePlanes);
    ReleasePacketSlot((void **)&g_apCutsceneSequences);
    ReleasePacketSlot((void **)&g_apCutsceneScenes);
    free(g_4_pszFilmWorkScratch);
    if (g_pMemoryLogFile != 0) {
        fprintf(g_pMemoryLogFile,
                "Free ___workstring %p free: %d\n",
                g_4_pszFilmWorkScratch,
                (int)(short)GetOriginalFreeMemory());
    }
    free(g_116_pszFilmPrintStore);
    if (g_pMemoryLogFile != 0) {
        fprintf(g_pMemoryLogFile,
                "Free ___printstring %p free: %d\n",
                g_116_pszFilmPrintStore,
                (int)(short)GetOriginalFreeMemory());
    }
    g_4_pszFilmWorkScratch = 0;
    g_27_pszCutsceneFormattedText = 0;
    g_116_pszFilmPrintStore = 0;
    ClearActiveCutscenePixelAlias();
    ReleaseCutsceneViewport(&g_9_stCutsceneTextBackingPort);
    ReleaseCutsceneViewport(&g_stSecondaryViewBufferBlock);
    if (g_bSceneCockpitLoaded != 0) {
        ReleasePacketSlot(&g_pCutsceneCockpitPalette);
        g_13_pActiveCutscenePixelsAddress = 0;
    }
    g_nInputPollPeriod = savedInputPollPeriod;
    g_29_pCurrentTextContext = savedTextContext;
    ReleasePacketSlot(&g_pCutsceneCockpitPacket);
    if (g_pSceneMusicPacket != 0) {
    }
    StopMusic(0);
    ReleaseCutsceneFileResource(
        0, &g_pCutsceneMusicResources);
    g_pSceneMusicPacket = 0;
    ReleaseCutsceneSoundEffects(-1);
    g_bMemoryLogToFile = 1;
    WriteDetailedMemoryStateReport();
    g_bMemoryLogToFile = 0;
    if (g_pMemoryLogFile != 0)
        fclose(g_pMemoryLogFile);
    g_cShowMemoryStatus = (signed char)previousMemoryStatus;
}

/* Function start: 0x42D81C */
void ExecuteCutsceneScene(CutsceneScene *scene)
{
    SceneFlicObject *savedSprite;
    CutscenePlane *savedPlane;
    short savedOwner;

    savedOwner = g_nActiveCutsceneResourceLevel;
    savedSprite = g_pCurrentCutsceneSprite;
    savedPlane = g_pCurrentCutscenePlane;
    if (scene == 0)
        FatalCutsceneError("A10");
    InitializeCutsceneViewports();
    g_nActiveCutsceneResourceLevel = scene->owner;
    PumpWindowMessages(0);
    if (g_bCutsceneSkipFrame == 0 &&
        g_bCutsceneViewportPreallocated == 0) {
        g_nNextCutsceneFrameClock =
            g_196_nInputTimerTicks + g_nCutsceneFrameDelay;
    }
    if (g_bCutsceneSkipAll == 0)
        RunCutsceneScript(&scene->scriptCursor, 3);
    g_pCurrentCutsceneSprite = savedSprite;
    g_pCurrentCutscenePlane = savedPlane;
    g_nActiveCutsceneResourceLevel = savedOwner;
}

/* Function start: 0x42D8E2 */
void CopyCutsceneSpriteDisplay(unsigned char destination,
                               unsigned char source)
{
    SceneFlicObject *target;
    SceneFlicObject *sourceSprite;

    target = g_apSceneObjects[
        FindActiveCutsceneObjectResources(
            g_pCutsceneSpriteResources)
            ->scriptSymbolIndices[destination]];
    sourceSprite = g_apSceneObjects[
        FindActiveCutsceneObjectResources(
            g_pCutsceneSpriteResources)
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

    target = g_apSceneObjects[
        FindActiveCutsceneObjectResources(
            g_pCutsceneSpriteResources)
            ->scriptSymbolIndices[destination]];
    sourceSprite = g_apSceneObjects[
        FindActiveCutsceneObjectResources(
            g_pCutsceneSpriteResources)
            ->scriptSymbolIndices[source]];
    sourceSprite->linkedOwner = sourceSprite->owner;
    sourceSprite->linkedScript = sourceSprite->scriptStart;
    sourceSprite->scriptCursor = target->scriptStart;
    sourceSprite->scriptStart = sourceSprite->scriptCursor;
    sourceSprite->owner = g_nActiveCutsceneResourceLevel;
}

/* Two sprites a developer could watch: drawing either one resets x first,
 * a store the real x then overwrites. */
extern SceneFlicObject *g_7_pWatchedSceneSpriteA; /* 0x005d2e90 */
extern SceneFlicObject *g_11_pWatchedSceneSpriteB; /* 0x005d2e94 */

/* Function start: 0x42DA25 */
void DrawCutsceneSprite(SceneFlicObject *sprite)
{
    short y;
    short x;
    short rotation;
    short attributes;
    short scale;

    if (sprite == 0)
        FatalCutsceneError("A11");
    if (sprite == g_7_pWatchedSceneSpriteA ||
        sprite == g_11_pWatchedSceneSpriteB)
        x = 0;
    if (g_bCutsceneDrawingEnabled == 0 &&
        g_bCutsceneSkipFrame != 0)
        return;
    if (sprite->active != 0 && sprite->visible != 0 &&
        sprite->shape != 0 && g_bCutsceneViewportPreallocated == 0) {
        x = sprite->x;
        y = sprite->y;
        rotation = sprite->rotation;
        scale = sprite->scale;
        attributes = sprite->drawFlags;
        if (g_pCutsceneSpriteParentPlane != 0) {
            x = (short)(x + g_pCutsceneSpriteParentPlane->x);
            y = (short)(y + g_pCutsceneSpriteParentPlane->y);
            rotation = (short)(rotation +
                g_pCutsceneSpriteParentPlane->rotation);
            scale = (short)(scale +
                g_pCutsceneSpriteParentPlane->scale);
            attributes ^= (short)g_pCutsceneSpriteParentPlane->drawFlags;
        }
        rotation = (short)(rotation % 360);
        if (rotation < 0)
            rotation = (short)(rotation + 360);
        if (scale < 0)
            scale = 0;
        if (scale > 0x4000)
            scale = 0x4000;
        if ((attributes & 1) != 0) {
            if ((attributes & 0x10) != 0) {
                x = (short)(160 - x + 160);
            }
            if ((attributes & 0x20) != 0) {
                y = (short)(100 - y + 100);
            }
            attributes &= ~1;
        }
        g_nCutsceneSpritesDrawn++;
        g_26_pFilmRgbPacket = 0;
        switch (sprite->drawType) {
        case 4:
            if (g_nFrameSkipCountdown == 0) {
                DrawSpriteScaled(&g_stSecondaryViewBufferBlock,
                    x, y, sprite->shape,
                    (short)(sprite->currentFrame -
                            sprite->segmentStartFrame),
                    rotation, scale, attributes);
            }
            sprite->currentFrame++;
            g_26_pFilmRgbPacket =
                (void *)sprite->decoderState;
            AdvanceSceneFlicStream(sprite);
            break;
        case 0:
        case 2:
            if (g_nFrameSkipCountdown == 0) {
                DrawSpriteScaled(&g_stSecondaryViewBufferBlock,
                    x, y, sprite->shape, sprite->baseFrame,
                    rotation, scale, attributes);
                DrawSpriteScaled(&g_stSecondaryViewBufferBlock,
                    x, y, sprite->shape,
                    (short)(sprite->baseFrame + sprite->currentFrame + 1),
                    rotation, scale, attributes);
            }
            sprite->tick++;
            break;
        case 1:
        case 3:
            if (g_nFrameSkipCountdown == 0) {
                DrawSpriteScaled(&g_stSecondaryViewBufferBlock,
                    x, y, sprite->shape,
                    (short)(sprite->baseFrame + sprite->currentFrame),
                    rotation, scale, attributes);
            }
            sprite->tick++;
            break;
        }
    }
}

/* Function start: 0x42DD9F */
void DrawCutscenePlane(CutscenePlane *plane)
{
    short index;

    if (plane == 0)
        FatalCutsceneError("A12");
    if (g_bCutsceneDrawingEnabled == 0 &&
        g_bCutsceneSkipFrame != 0)
        return;
    if (plane->active != 0 && plane->visible != 0 &&
        g_bCutsceneViewportPreallocated == 0) {
        g_pCutsceneSpriteParentPlane = plane;
        for (index = 0; index < plane->spriteCount; index++) {
            DrawCutsceneSprite(g_apSceneObjects[
                plane->spriteIndices[index]]);
        }
    }
}

/* Function start: 0x42DE62 */
void ClearCutsceneViewport(Viewport *viewport, unsigned char colour)
{
    if (g_cCutsceneVideoMode == 13)
        colour = g_abCutscenePaletteTranslation[colour];
    ClearViewport(viewport, colour);
}

/* Function start: 0x42DE9D */
void RestoreCutsceneTextBacking(void)
{
    Viewport destination;
    short height;

    if (g_bCutsceneTextRestorePending != 0) {
        destination = g_10_stCutsceneStringViewportBlock;
        height = (short)(g_10_stCutsceneStringViewportBlock.bottom -
                         g_10_stCutsceneStringViewportBlock.top + 1);
        destination.top =
            (short)((height - g_29_pCurrentTextContext->cursorY) / 4 +
                    g_10_stCutsceneStringViewportBlock.top);
        ClearCutsceneViewport(&g_10_stCutsceneStringViewportBlock,
            g_29_pCurrentTextContext->backgroundColour);
        CopyViewportContents(
            &g_9_stCutsceneTextBackingPort, &destination);
        g_bCutsceneTextRestorePending = 0;
    }
}

/* Function start: 0x42DF32 */
void ClearCutsceneTextViewport(void)
{
    ClearCutsceneViewport(g_29_pCurrentTextContext->viewport,
        g_29_pCurrentTextContext->backgroundColour);
    g_29_pCurrentTextContext->cursorX =
        g_29_pCurrentTextContext->viewport->left;
    g_29_pCurrentTextContext->cursorY =
        g_29_pCurrentTextContext->viewport->top;
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
            itoa(g_117_nCutsceneFormatValueWord, (char *)number, 10);
            out = CopyStringAndReturnEnd(out, (char *)number);
        } else if ((unsigned char)character >= 0x85) {
            switch ((unsigned char)character) {
            case 0x85:
                out = CopyStringAndReturnEnd(
                    out, g_stPilotNameVariables.lastName);
                break;
            case 0x86:
                out = CopyStringAndReturnEnd(
                    out, g_stPilotNameVariables.callsign);
                break;
            case 0x89:
                out = CopyStringAndReturnEnd(
                    out, g_stPilotNameVariables.firstName);
                break;
            case 0x87:
                itoa(*((short *)g_pCampaignGlobals + 54),
                     (char *)number, 10);
                out = CopyStringAndReturnEnd(out, (char *)number);
                break;
            case 0x88:
                itoa(*((short *)g_pCampaignGlobals + 55),
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
    short priorOwner;
    signed char continueSequence;
    signed char textShown;
    short planeIndex;
    CutsceneSequence *priorSequence;

    textShown = 0;
    priorOwner = g_nActiveCutsceneResourceLevel;
    priorSequence = g_pCurrentCutsceneSequence;
    if (sequence == 0)
        FatalCutsceneError("A13");
    g_nActiveCutsceneResourceLevel = sequence->owner;
    g_pCurrentCutsceneSequence = sequence;
    g_bMovieFramePresented = 0;
    do {
        PumpWindowMessages(0);
        if (textShown == 0 && text != 0) {
            textShown++;
            ClearCutsceneTextViewport();
            ExpandCutsceneText(text, g_4_pszFilmWorkScratch);
#ifdef SDL_PORT
            if (memcmp(g_4_pszFilmWorkScratch,
                       "50", 2) != 0) {
#else
            if ((IsCutsceneSpeechLoaded() == 0 ||
                 g_nAudioEnabled == 0) &&
                memcmp(g_4_pszFilmWorkScratch, "50", 2) != 0) {
#endif
                FormatTextBufferFromStart(
                    "%s%P", g_4_pszFilmWorkScratch);
            }
            g_pszCurrentCutsceneText =
                g_4_pszFilmWorkScratch;
            g_bCutsceneTextRestorePending = 1;
        }
        while (g_bCutsceneSkipFrame == 0 &&
               (int)g_nNextCutsceneFrameClock >
                   (int)g_196_nInputTimerTicks) {
            PumpWindowMessages(0);
#ifdef SDL_PORT
            /* See issue #33 (review on PR #34): this sequence-presentation
             * wait is purely clock-driven -- no shared/pending resource is
             * involved, so unlike the speech-cache waits elsewhere there is
             * nothing to leave in an inconsistent state by bailing out
             * early. */
            if (g_bSceneEscapeRequested != 0)
                break;
#endif
        }
        g_nFrameSkipCountdown = 0;
        continueSequence = RunCutsceneScript(&sequence->scriptCursor, 2);
        g_pCurrentCutsceneSequence = sequence;
        if (draw != 0) {
            if (continueSequence != 0 && sequence->planeCount != 0 &&
                g_bMovieFramePresented == 0) {
                for (planeIndex = 0; planeIndex < sequence->planeCount;
                     planeIndex++) {
                    UpdateCutscenePlaneObject(
                        g_apCutscenePlanes[
                            sequence->planeIndices[planeIndex]], 1);
                }
                for (planeIndex = 0; planeIndex < sequence->planeCount;
                     planeIndex++) {
                    DrawCutscenePlane(g_apCutscenePlanes[
                        sequence->planeIndices[planeIndex]]);
                }
                if (g_bCutsceneSkipFrame == 0 &&
                    g_bCutsceneViewportPreallocated == 0) {
                    ConsumeCutscenePalettePacket(0, 0x100, 1);
                    PresentCutsceneFrame(
                        &g_stSecondaryViewBufferBlock,
                        &g_15_stSceneFlicScratchViewport);
                    RestoreCutsceneTextBacking();
                    if (g_nCutsceneFrameDelay != 0) {
                        g_nNextCutsceneFrameClock =
                            g_196_nInputTimerTicks +
                            g_nCutsceneFrameDelay;
                    }
                }
            } else {
                g_bMovieFramePresented = 0;
            }
        } else {
            continueSequence = 0;
        }
    } while (continueSequence != 0);
    g_pCurrentCutsceneSequence = priorSequence;
    g_nActiveCutsceneResourceLevel = priorOwner;
}

/* Function start: 0x42E3CF */
void UpdateCutscenePlaneObject(CutscenePlane *plane,
                               signed char updateChildren)
{
    short savedOwner;
    short blocked;
    CutscenePlane *savedPlane;
    short listSlot;
    short objectIndex;

    savedOwner = g_nActiveCutsceneResourceLevel;
    savedPlane = g_pCurrentCutscenePlane;
    blocked = 0;
    if (plane == 0)
        FatalCutsceneError("A14");
    if (plane->waitTicks != 0) {
        if (g_bCutsceneViewportPreallocated == 0 &&
            g_bCutsceneSkipFrame == 0 &&
            g_196_nInputTimerTicks < plane->waitStart + plane->waitTicks) {
            blocked++;
        } else {
            plane->waitTicks = 0;
        }
    }
    g_pCurrentCutscenePlane = plane;
    g_nActiveCutsceneResourceLevel = plane->owner;
    PumpWindowMessages(0);
    if (blocked == 0)
        RunCutsceneScript(&plane->scriptCursor, 1);
    if (updateChildren != 0) {
        for (listSlot = 0; listSlot < plane->spriteCount; listSlot++) {
            objectIndex = plane->spriteIndices[listSlot];
            UpdateCutsceneSpriteObject(g_apSceneObjects[objectIndex]);
        }
        plane->x = (short)(plane->x + plane->deltaX);
        plane->y = (short)(plane->y + plane->deltaY);
        plane->depth = (short)(plane->depth + plane->deltaDepth);
    }
    g_pCurrentCutscenePlane = savedPlane;
    g_nActiveCutsceneResourceLevel = savedOwner;
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
        if (g_bCutsceneViewportPreallocated == 0 &&
            g_bCutsceneSkipFrame == 0 &&
            g_196_nInputTimerTicks < sprite->waitStart + sprite->waitTicks) {
            waiting++;
        } else {
            sprite->waitTicks = 0;
        }
    }
    if (waiting != 0)
        return;
    PumpWindowMessages(0);
    savedSprite = g_pCurrentCutsceneSprite;
    g_pCurrentCutsceneSprite = sprite;
    if (sprite->field_3 >= 0)
        DispatchCutsceneSpriteHandler(sprite, 0);
    RunCutsceneScript(&sprite->scriptCursor, 0);
    sprite->x = (short)(sprite->x + sprite->deltaX);
    sprite->y = (short)(sprite->y + sprite->deltaY);
    sprite->field_2f = (short)(sprite->field_2f + sprite->deltaDepth);
    g_pCurrentCutsceneSprite = savedSprite;
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
        handler = g_apCutsceneSpriteHandlers[object->field_3];
        handler(object);
        break;
    default:
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
    object->context = g_nSceneFlicContext;
    AdvanceSceneFlicStream(object);
}

/* Function start: 0x42E762 */
void ReleaseSceneFlicPackets(void)
{
    short cacheIndex;
    short objectIndex;
    SceneFlicObject *object;

    for (cacheIndex = 0; cacheIndex < 15U; cacheIndex++) {
        if (g_aSceneFlicCopyTable[cacheIndex].shape != 0) {
            for (objectIndex = 0; objectIndex < 128; objectIndex++) {
                object = g_apSceneObjects[objectIndex];
                if (object != 0 && object->drawType == 4 &&
                    g_aSceneFlicCopyTable[cacheIndex].shape ==
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
            (void **)&g_aSceneFlicCopyTable[cacheIndex].shape);
        ReleasePacketSlot(
            &g_aSceneFlicCopyTable[cacheIndex].auxiliaryAllocation);
    }
}

/* Function start: 0x42E868 */
void AdvanceSceneFlicStream(SceneFlicObject *object)
{
    signed char fetch;
    char filename[68];
    short slot;
    short probe;
    void *retired;

    fetch = 1;
    object->decoderState = 0;
    if (object->segmentEndFrame <= object->currentFrame) {
        if (object->finalFrame <= object->currentFrame) {
            object->active = 0;
            ReleaseSceneFlicPackets();
            object->shape = 0;
        } else {
            if (object->currentFrame != 0) {
                CopyViewportContents(&g_stSecondaryViewBufferBlock,
                                     &g_15_stSceneFlicScratchViewport);
            }
            DosStrcpy(filename, object->filename);
            RewriteDiskFileGraphicsExtensions(filename);
            retired = object->shape;
            if (retired != 0) {
                for (slot = 0; slot < 15U; slot++) {
                    if (g_aSceneFlicCopyTable[slot].shape ==
                        retired) {
                        ReleasePacketSlot(&retired);
                        g_aSceneFlicCopyTable[slot].shape = 0;
                        retired =
                            g_aSceneFlicCopyTable[slot].shape;
                        break;
                    }
                }
            }
            while (fetch != 0) {
                if (g_aSceneFlicCopyTable[0].shape == 0) {
                    object->shape = 0;
                    for (slot = 0; slot < 15U; slot++) {
                        g_aSceneFlicCopyTable[slot]
                            .auxiliaryAllocation = 0;
                        g_aSceneFlicCopyTable[slot].shape =
                            LoadNamedPacket(
                                filename,
                                (short)(object->nextSection + slot),
                                0, 0x0c, 0, 1);
                        if (g_aSceneFlicCopyTable[slot].shape ==
                            0) {
                            if (slot == 0) {
                                OutputDebugPrintf(
                                    "FLIC Failed %s (%d)\n Hit Key to Continue",
                                    filename,
                                    object->nextSection + slot);
                                WaitForCutsceneInputEvent();
                                object->segmentEndFrame = 0;
                                object->currentFrame = object->finalFrame;
                            }
                            fetch = 0;
                            break;
                        } else if (g_pMemoryLogFile != 0) {
                            fprintf(
                                g_pMemoryLogFile,
                                "Loaded Flic %s (%d) at %Fp\n", filename,
                                object->nextSection + slot,
                                g_aSceneFlicCopyTable[slot].shape);
                        }
                    }
                    fetch = 0;
                }
                if (g_aSceneFlicCopyTable[0].shape != 0) {
                    slot = 0;
                    if (object->shape != 0) {
                        while (slot < 15U) {
                            slot++;
                            if (g_aSceneFlicCopyTable[slot - 1]
                                    .shape == object->shape)
                                break;
                        }
                    }
                    if (g_aSceneFlicCopyTable[slot].shape != 0) {
                        object->shape =
                            g_aSceneFlicCopyTable[slot].shape;
                        object->decoderState = 0;
                        g_aSceneFlicCopyTable[slot]
                            .auxiliaryAllocation = 0;
                        object->segmentStartFrame = object->currentFrame;
                        object->segmentEndFrame =
                            (short)(GetShapeFrameCount(object->shape) +
                                    object->currentFrame);
                        object->nextSection++;
                        fetch = 0;
                    } else {
                        for (probe = 0;
                             probe < slot;
                             probe++) {
                            ReleasePacketSlot(
                                (void **)&g_aSceneFlicCopyTable[
                                    probe].shape);
                        }
                        object->shape = 0;
                    }
                }
            }
            CopyViewportContents(&g_15_stSceneFlicScratchViewport,
                                 &g_stSecondaryViewBufferBlock);
        }
    }
}

/* Function start: 0x42EBD0 */
void ReleaseActiveCutscenePacket(CutsceneResourceTable *resources,
                                 void *packet)
{
    short scan;
    short count;
    CutsceneResourceTable *activeResources;
    void **packetSlot;

    activeResources = FindActiveCutsceneFileResources(resources);
    count = activeResources->count;
    packetSlot = activeResources->loadedPackets;
    for (scan = 0; scan < count; scan++, packetSlot++) {
        if (*packetSlot == packet) {
            ReleasePacketSlot(packetSlot);
            if (g_pMemoryLogFile != 0) {
                fprintf(g_pMemoryLogFile,
                        "--- Dumped (reset): %Fp  item: %3d  "
                        "level = %d free = %8lu\n",
                        packet, (int)scan,
                        (int)g_nActiveCutsceneResourceLevel,
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

    savedOwner = g_nActiveCutsceneResourceLevel;
    g_nActiveCutsceneResourceLevel =
        g_1_nCutsceneFontOwnerNumber;
    ReleaseActiveCutscenePacket(
        g_pCutsceneFontResources,
        g_2_stCutsceneMessageCtxInfo.font);
    g_2_stCutsceneMessageCtxInfo.font = 0;
    g_nActiveCutsceneResourceLevel = savedOwner;
}

/* Function start: 0x42ECCB */
void UpdateMap(char *text, short objective)
{
    int deadline;

    ClearCutsceneViewport(&g_2_stModalSourceWindow, 0);
    g_bBriefingMapActive = 1;
    LoadMissionData(g_pCampaignGlobals->series,
                    g_pCampaignGlobals->mission);
    Build_objective_list();
    g_cCurrentObjective = (signed char)objective;
    g_13_pActiveCutscenePixelsAddress = 0;
    BriefingMap_DisplayMap();
    g_bBriefingMapActive = 0;
    RouteCutsceneViewportToDisplay();
    SetTextContext(&g_2_stCutsceneMessageCtxInfo);
    ClearCutsceneTextViewport();
    ExpandCutsceneText((const unsigned char *)text,
                       g_4_pszFilmWorkScratch);
#ifdef SDL_PORT
    if (memcmp(g_4_pszFilmWorkScratch, "50", 2) != 0)
        FormatTextBufferFromStart(
            "%s%P", g_4_pszFilmWorkScratch);
#else
    if (IsCutsceneSpeechLoaded() == 0 || g_nAudioEnabled == 0) {
        if (memcmp(g_4_pszFilmWorkScratch, "50", 2) != 0)
            FormatTextBufferFromStart("%s%P", g_4_pszFilmWorkScratch);
    }
#endif
    g_bCutsceneTextRestorePending = 1;
    RestoreCutsceneTextBacking();
    ServiceInputDevices(-1);
    deadline = g_196_nInputTimerTicks + 600;
    while ((int)g_196_nInputTimerTicks < deadline &&
           g_bCutsceneSkipFrame == 0 &&
           g_bCutsceneViewportPreallocated == 0) {
        if (ServiceInputDevices(-1) != 0) {
            ServiceCutsceneRuntimeHook();
            if (FindQueuedInputEvent(INPUT_EVENT_BUTTON_UP) != 0 ||
                FindQueuedInputEvent(INPUT_EVENT_KEY_UP) != 0)
                break;
        }
    }
    ClearCutsceneViewport(&g_2_stModalSourceWindow, 0);
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
        switch (g_cCutsceneVideoMode) {
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
    unsigned int farMemory;
    char text[256];
    unsigned int largest;
    Viewport viewport;
    unsigned int memory;

    viewport = g_2_stModalSourceWindow;
    if ((g_cCutsceneVideoMode == 0x13 ||
         g_cCutsceneVideoMode == 0x0d) &&
        g_cShowMemoryStatus != 0) {
        if (*message != '~') {
            PresentCutsceneFrame(
                &g_9_stCutsceneTextBackingPort,
                &g_10_stCutsceneStringViewportBlock);
            g_bCutsceneTextRestorePending = 1;
        }
        largest = GetLargestMainMemoryBlock();
        memory = GetAvailableMainMemory();
        farMemory = GetAvailableFarMemory();
        if (*message != '~') {
            sprintf(text,
                    "%s Far: %lu (Lg) = %lu  EMS: %lu\n"
                    "OVR: %lu INITIAL: %lu",
                    message, memory, largest,
                    farMemory,
                    g_47_nCutsceneMemDelta,
                    g_28_nCutsceneInitialAvailableMemoryWord);
        } else {
            strcpy(text, "Game Paused");
        }
        g_16_stCutsceneDrawingTextSettingsInfo =
            g_2_stCutsceneMessageCtxInfo;
        g_16_stCutsceneDrawingTextSettingsInfo.viewport =
            &g_2_stModalSourceWindow;
        SetTextContext(&g_16_stCutsceneDrawingTextSettingsInfo);
        g_29_pCurrentTextContext->cursorX =
            g_29_pCurrentTextContext->viewport->left;
        g_29_pCurrentTextContext->cursorY =
            g_29_pCurrentTextContext->viewport->top;
        FormatTextBufferFromStart("%s%P", text);
        WaitForCutsceneInputEvent();
        g_bCutsceneSkipFrame = 0;
        SetTextContext(&g_2_stCutsceneMessageCtxInfo);
        viewport.top = 0;
        viewport.bottom =
            (short)(g_15_stSceneFlicScratchViewport.top - 1);
        ClearViewport(&viewport, 0);
    }
}

/* Function start: 0x42F0B3 */
void FormatCinematicMemoryStatus(const char *format, ...)
{
    va_list marker;
    char text[256];

    va_start(marker, format);
    vsprintf(text, format, marker);
    DrawCinematicMemoryStatus(text);
    va_end(marker);
}

/* Function start: 0x42F100 */
void ReportCutscenePacketAllocationFailure(const char *filename,
                                           short section,
                                           unsigned int packetSize)
{
    if (g_cCutsceneVideoMode == 0x13) {
        FormatCinematicMemoryStatus(
            "Alloc Failed %s (%d) size=%ld\n",
            filename, section, packetSize);
    }
}

/* Function start: 0x42F135 */
short PopCutsceneScriptValue(short **stack, short *stackBottom)
{
    int current;
    short *top;
    int limit;

    top = *stack;
    current = DosFarPtrToNear(top);
    limit = DosFarPtrToNear(stackBottom);
    if (limit <= current)
        FatalCutsceneError("A18");
    top++;
    *stack = top;
    return top[-1];
}

/* Function start: 0x42F19A */
void PushCutsceneScriptValue(short **stack, short *stackTop,
                             short value)
{
    int current;
    short *cell;
    int topAddress;

    cell = *stack;
    current = DosFarPtrToNear(cell);
    topAddress = DosFarPtrToNear(stackTop);
    if (topAddress >= current)
        FatalCutsceneError("A19");
    cell--;
    *cell = value;
    *stack = cell;
}

/* Function start: 0x42F1FD */
signed char RunCutsceneScript(unsigned char **scriptCursor,
                              short objectType)
{
    unsigned char *scriptBytes;
    short opcode;
    short scriptValueStack[10];
    short secondaryOperand;
    short value;
    short *scriptLocal;
    short baseValue;
    int inputSeen;
    /* Retail reserves 100 unreferenced bytes between topOperand and
     * scriptBytes. The original declaration and purpose are unknown. */
    unsigned char unreferencedFrameStorage[100];
    short *topOperand;
    signed char protectCondition;
    signed char shouldContinue;

    scriptBytes = *scriptCursor;
    protectCondition = 1;
    shouldContinue = 1;
    topOperand = scriptValueStack + 10;
    inputSeen = 0;
    if (scriptBytes == 0)
        FatalCutsceneError(
            "ERROR: Running script for UNLINKED EXTERNAL (%s)",
            g_apszCutsceneObjectTypeNames[objectType]);
    if (g_pCurrentCutsceneSprite == 0) {
        g_pCurrentCutsceneSprite =
            g_apSceneObjects[
                FindActiveCutsceneObjectResources(
                    g_pCutsceneSpriteResources)->
                    scriptSymbolIndices[0]];
    }
    if (g_pCurrentCutscenePlane == 0 &&
        FindActiveCutsceneObjectResources(
            g_pCutscenePlaneResources)->
            scriptSymbolIndices != 0) {
        g_pCurrentCutscenePlane =
            g_apCutscenePlanes[
                FindActiveCutsceneObjectResources(
                    g_pCutscenePlaneResources)->
                    scriptSymbolIndices[0]];
    }
    if (g_pCurrentCutsceneSequence == 0 &&
        FindActiveCutsceneObjectResources(
            g_pCutsceneSequenceResources)->
            scriptSymbolIndices != 0) {
        g_pCurrentCutsceneSequence =
            g_apCutsceneSequences[
                FindActiveCutsceneObjectResources(
                    g_pCutsceneSequenceResources)->
                    scriptSymbolIndices[0]];
    }
    for (;;) {
#ifdef SDL_PORT
        /* See issue #33 (ESC feels slow or dead during cutscenes).
         *
         * The original escape-to-skip conversion is the block further down
         * ("handle_queued_cutscene_input" and the TakeInputPressCount()
         * block below it).  Neither of them reads
         * g_bSceneEscapeRequested on its own: both first require a
         * still-queued key or button press, or a
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
         *   - it clears g_bSceneEscapeRequested itself, so it
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
         * Still gated on g_bCutsceneViewportPreallocated == 0, and
         * that guard is load-bearing rather than cosmetic: preallocated == 1
         * marks the headless state-advance pass RunCampaignGameLoop
         * (brains.c) runs before the chalkboard, where drawing, audio,
         * palette work and nested scene playback are all suppressed.  The
         * player cannot see or hear that pass, so there is nothing there to
         * skip, and cutting it short with skipall would leave the campaign
         * globals it exists to compute half-written.  RunCampaignScript
         * (screen.c) clears the escape flag on the way out of that pass, so
         * a press made during it does not leak into the real cutscene. */
        if (g_bSceneEscapeRequested != 0 &&
            g_bCutsceneViewportPreallocated == 0) {
            SdlTracef("[cutscene] escape converted: objectType=%d "
                      "skipframe=%d skipall=%d clock=%d\n",
                      (int)objectType,
                      (int)g_bCutsceneSkipFrame,
                      (int)g_bCutsceneSkipAll,
                      (int)g_196_nInputTimerTicks);
            ReleaseSceneFlicPackets();
            ReleaseCutsceneSoundEffects(-1);
            ReleaseCutsceneSpeechPackets();
            g_bCutsceneSpeechActive = 0;
            g_bCutsceneSkipFrame = 1;
            g_bCutsceneSkipAll = 1;
            g_bSceneEscapeRequested = 0;
            return 0;
        }
#endif
        if (g_bCutsceneSkipAll != 0)
            g_bCutsceneSkipFrame = 1;
        if (g_bCutsceneSkipFrame == 0 &&
            g_bCutsceneViewportPreallocated == 0) {
            if (ServiceInputDevices(-1) != 0) {
                ServiceCutsceneRuntimeHook();
                g_19_pFilmInputFlag = FindQueuedInputEvent(INPUT_EVENT_KEY_DOWN);
                if (g_19_pFilmInputFlag != 0) {
                    switch (g_19_pFilmInputFlag->status) {
                    case 2:
                        g_cCutsceneSpeechSpeed = 0;
                        goto cutscene_input_processed;
                    case 3:
                        g_cCutsceneSpeechSpeed = 2;
                        goto cutscene_input_processed;
                    case 4:
                        g_cCutsceneSpeechSpeed = 4;
                        goto cutscene_input_processed;
                    case 5:
                        g_cCutsceneSpeechSpeed = 6;
                        goto cutscene_input_processed;
                    case 6:
                        g_cCutsceneSpeechSpeed = 8;
                        goto cutscene_input_processed;
                    case 0x19:
                        WaitForCutsceneInputEvent();
                        if ((g_19_pFilmInputFlag->modifiers &
                             0x1800) == 0) {
                            DrawCinematicMemoryStatus("~");
                        } else {
                            WaitForCutsceneInputEvent();
                        }
                        goto cutscene_input_processed;
                    case 0x11:
                        WaitForCutsceneInputEvent();
                        if ((g_19_pFilmInputFlag->modifiers &
                             0x1800) != 0) {
                            FormatCinematicMemoryStatus(
                                "Series: %d Mission: %c Talk: %d\n",
                                g_pCampaignGlobals->series,
                                g_pCampaignGlobals->mission + 'A',
                                g_pCampaignGlobals->field_08 + 1);
                        }
                        goto cutscene_input_processed;
                    }
                }
                if (g_wDigiPlaying == 0 &&
                    (FindQueuedInputEvent(INPUT_EVENT_KEY_DOWN) != 0 ||
                     FindQueuedInputEvent(INPUT_EVENT_BUTTON_DOWN) != 0)) {
                    ReleaseSceneFlicPackets();
                    ReleaseCutsceneSoundEffects(-1);
                    if (g_bSceneEscapeRequested != 0) {
                        ReleaseCutsceneSpeechPackets();
                        g_bCutsceneSpeechActive = 0;
                        g_bCutsceneSkipFrame = 1;
                        g_bCutsceneSkipAll = 1;
                        g_bSceneEscapeRequested = 0;
                        return 0;
                    }
                    g_2_bMovieStringAdvanceState = 0;
                    g_pszCutsceneSpeechCursor = 0;
                    g_bCutsceneSkipFrame = 1;
                    g_nNextCutsceneFrameClock =
                        g_196_nInputTimerTicks;
                    inputSeen = 1;
                }
            }
cutscene_input_processed:
            if (g_wDigiPlaying == 0 &&
                TakeInputPressCount() != 0 && inputSeen == 0) {
                ReleaseSceneFlicPackets();
                ReleaseCutsceneSoundEffects(-1);
                if (g_bSceneEscapeRequested != 0) {
                    ReleaseCutsceneSpeechPackets();
                    g_bCutsceneSpeechActive = 0;
                    g_bCutsceneSkipFrame = 1;
                    g_bCutsceneSkipAll = 1;
                    g_bSceneEscapeRequested = 0;
                    return 0;
                }
                g_2_bMovieStringAdvanceState = 0;
                g_pszCutsceneSpeechCursor = 0;
                g_bCutsceneSkipFrame = 1;
                g_nNextCutsceneFrameClock =
                    g_196_nInputTimerTicks;
                FlushInputEvents();
            }
            if (objectType == 2 &&
                g_pCurrentCutsceneSequence->waitTicks +
                    g_pCurrentCutsceneSequence->waitStart <
                    (int)g_196_nInputTimerTicks) {
                g_pCurrentCutsceneSequence->waitTicks = 0;
            }
            if (objectType == 2 &&
                g_bCutsceneSkipFrame != 0) {
                g_pCurrentCutsceneSequence->waitTicks = 0;
            }
        } else if (g_bCutsceneSkipFrame != 0) {
            /* The Win32 build retains the DOS release-input branch test. */
        }
        opcode = *scriptBytes++;
        switch (opcode) {
        case 0x07:
            *topOperand = (short)(*topOperand == 0);
            break;
        case 0x05:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand != 0 && value != 0);
            break;
        case 0x06:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand != 0 || value != 0);
            break;
        case 0x00:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand + value);
            break;
        case 0x01:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand - value);
            break;
        case 0x02:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand * value);
            break;
        case 0x03:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand / value);
            break;
        case 0x04:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand % value);
            break;
        case 0x0b:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand & value);
            break;
        case 0x0c:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand | value);
            break;
        case 0x0d:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand ^ value);
            break;
        case 0x09:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
#ifdef SDL_PORT
            *topOperand = (short)(*topOperand << (value & 0x1f));
#else
            *topOperand <<= value;
#endif
            break;
        case 0x0a:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
#ifdef SDL_PORT
            *topOperand = (short)(*topOperand >> (value & 0x1f));
#else
            *topOperand >>= value;
#endif
            break;
        case 0x08:
            secondaryOperand = PopCutsceneScriptValue(
                &topOperand, scriptValueStack + 10);
            baseValue = PopCutsceneScriptValue(
                &topOperand, scriptValueStack + 10);
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                RandomInRange((unsigned short)baseValue,
                              (unsigned short)secondaryOperand));
            break;
        case 0x0f:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            if (protectCondition == 0 &&
                g_bCutsceneSkipFrame != 0 &&
                g_bCutsceneViewportPreallocated == 0) {
                value = 0;
            }
            if (g_bCutsceneSkipAll != 0)
                value = 0;
            if (value == 0) {
                g_2_nCutsceneForkOffsetNumber = *(short *)scriptBytes;
                scriptBytes += g_2_nCutsceneForkOffsetNumber;
            } else {
                scriptBytes += 2;
            }
            break;
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            if (protectCondition == 0 &&
                g_bCutsceneSkipFrame != 0 &&
                g_bCutsceneViewportPreallocated == 0) {
                value = 1;
            }
            if (g_bCutsceneSkipAll != 0)
                value = 0;
            if (value == 1) {
                g_2_nCutsceneForkOffsetNumber = *(short *)scriptBytes;
                scriptBytes += g_2_nCutsceneForkOffsetNumber;
            } else {
                scriptBytes += 2;
            }
            break;
        case 0x0e:
            g_2_nCutsceneForkOffsetNumber = *(short *)scriptBytes;
            scriptBytes += g_2_nCutsceneForkOffsetNumber;
            break;
        case 0x17:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = *topOperand >= value ? 0 : 1;
            break;
        case 0x18:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand > value);
            break;
        case 0x19:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = *topOperand > value ? 0 : 1;
            break;
        case 0x1a:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand >= value);
            break;
        case 0x1b:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand == value);
            break;
        case 0x1c:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            *topOperand = (short)(*topOperand != value);
            break;
        case 0x1d:
            *topOperand = (short)(*topOperand == 0);
            break;
        case 0x1e:
            *topOperand = (short)(*topOperand != 0);
            break;
        case 0x1f:
            g_pCurrentCutsceneSprite =
                g_apSceneObjects[
                    FindActiveCutsceneObjectResources(
                        g_pCutsceneSpriteResources)->
                        scriptSymbolIndices[*scriptBytes++]];
            if (g_pCurrentCutsceneSprite == 0)
                FatalCutsceneError("A21");
            break;
        case 0x20:
            g_pCurrentCutscenePlane =
                g_apCutscenePlanes[
                    FindActiveCutsceneObjectResources(
                        g_pCutscenePlaneResources)->
                        scriptSymbolIndices[*scriptBytes++]];
            if (g_pCurrentCutscenePlane == 0)
                FatalCutsceneError("A22");
            break;
        case 0x21:
            g_pCurrentCutsceneSequence =
                g_apCutsceneSequences[
                    FindActiveCutsceneObjectResources(
                        g_pCutsceneSequenceResources)->
                        scriptSymbolIndices[*scriptBytes++]];
            if (g_pCurrentCutscenePlane == 0)
                FatalCutsceneError("A23");
            break;
        case 0x8a:
            g_2_bMovieStringAdvanceState = 1;
            g_pLinkedCutsceneSprite =
                g_apSceneObjects[
                    FindActiveCutsceneObjectResources(
                        g_pCutsceneSpriteResources)->
                        scriptSymbolIndices[*scriptBytes++]];
            if (g_pLinkedCutsceneSprite == 0)
                FatalCutsceneError("A24");
            g_pLinkedCutsceneSprite->field_3 = 0;
            break;
        case 0x9e:
            g_nSceneFlicContext = *scriptBytes++;
            break;
        case 0x77:
            value = *scriptBytes++;
            g_2_pCutsceneLocalsTemp = AllocateScenePointerTable(
                value, 2, 0, "Cannot Allocate Loacls Data");
            switch (objectType) {
            case 0:
                if (g_pCurrentCutsceneSprite == 0)
                    FatalCutsceneError("A25");
                if (g_pCurrentCutsceneSprite->locals != 0)
                    ReleasePacketSlot((void **)&g_2_pCutsceneLocalsTemp);
                if (g_pCurrentCutsceneSprite->linkedScript == 0)
                    g_pCurrentCutsceneSprite->locals =
                        g_2_pCutsceneLocalsTemp;
                else
                    ReleasePacketSlot((void **)&g_2_pCutsceneLocalsTemp);
                break;
            case 1:
                if (g_pCurrentCutscenePlane == 0)
                    FatalCutsceneError("A26");
                if (g_pCurrentCutscenePlane->locals != 0)
                    ReleasePacketSlot((void **)&g_2_pCutsceneLocalsTemp);
                g_pCurrentCutscenePlane->locals =
                    g_2_pCutsceneLocalsTemp;
                break;
            case 2:
                if (g_pCurrentCutsceneSequence == 0)
                    FatalCutsceneError("A27");
                if (g_pCurrentCutsceneSequence->locals != 0)
                    ReleasePacketSlot((void **)&g_2_pCutsceneLocalsTemp);
                g_pCurrentCutsceneSequence->locals =
                    g_2_pCutsceneLocalsTemp;
                break;
            }
            break;
        case 0x7d:
            WaitForCutsceneInputEvent();
            break;
        case 0x7e: {
            int waitUntil;

            waitUntil = g_196_nInputTimerTicks +
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            while (g_196_nInputTimerTicks < waitUntil &&
                   g_bCutsceneSkipFrame == 0 &&
                   g_bCutsceneViewportPreallocated == 0) {
                if (ServiceInputDevices(-1) != 0) {
                    ServiceCutsceneRuntimeHook();
                    if (FindQueuedInputEvent(INPUT_EVENT_BUTTON_UP) != 0 ||
                        FindQueuedInputEvent(INPUT_EVENT_KEY_UP) != 0) {
                        break;
                    }
                }
            }
            break;
        }
        case 0x7f:
            g_4_nCutsceneResourceScratchValue =
                (short)(signed char)*scriptBytes++;
            if (g_stSecondaryViewBufferBlock.pixels ==
                g_9_stCutsceneTextBackingPort.pixels) {
                g_9_stCutsceneTextBackingPort.pixels = 0;
            }
            g_15_stSceneFlicScratchViewport.left = 0;
            g_15_stSceneFlicScratchViewport.top =
                g_4_nCutsceneResourceScratchValue;
            g_15_stSceneFlicScratchViewport.right = 0x13f;
            g_15_stSceneFlicScratchViewport.bottom =
                (short)(g_4_nCutsceneResourceScratchValue + 0x7d);
            g_10_stCutsceneStringViewportBlock.left = 0x10;
            g_10_stCutsceneStringViewportBlock.top =
                (short)(g_4_nCutsceneResourceScratchValue + 0x7e);
            g_10_stCutsceneStringViewportBlock.right = 0x12f;
            g_10_stCutsceneStringViewportBlock.bottom = 199;
            ReleaseCutsceneViewport(&g_stSecondaryViewBufferBlock);
            g_stSecondaryViewBufferBlock.left =
                g_stSecondaryViewBufferBlock.top = 0;
            g_stSecondaryViewBufferBlock.right = 0x13f;
            g_stSecondaryViewBufferBlock.bottom = 0x7d;
            AllocateCutsceneViewportOrFatal(
                &g_stSecondaryViewBufferBlock, 0, 0);
            RouteCutsceneViewportToDisplay();
            ReleaseCutsceneViewport(
                &g_9_stCutsceneTextBackingPort);
            g_9_stCutsceneTextBackingPort.left =
                g_9_stCutsceneTextBackingPort.top = 0;
            g_9_stCutsceneTextBackingPort.right = 0x11f;
            g_9_stCutsceneTextBackingPort.bottom = 0x24;
            AllocateCutsceneViewportOrFatal(
                &g_9_stCutsceneTextBackingPort, 0, 0);
            break;
        case 0x80:
            g_15_stSceneFlicScratchViewport.left = 0;
            g_15_stSceneFlicScratchViewport.top =
                g_4_nCutsceneResourceScratchValue;
            g_15_stSceneFlicScratchViewport.right = 0x13f;
            g_15_stSceneFlicScratchViewport.bottom = 199;
            g_10_stCutsceneStringViewportBlock.left = 0;
            g_10_stCutsceneStringViewportBlock.top = 0;
            g_10_stCutsceneStringViewportBlock.right = 0x13f;
            g_10_stCutsceneStringViewportBlock.bottom = 199;
            ReleaseCutsceneViewport(&g_stSecondaryViewBufferBlock);
            g_stSecondaryViewBufferBlock.left =
                g_stSecondaryViewBufferBlock.top = 0;
            g_stSecondaryViewBufferBlock.right = 0x13f;
            g_stSecondaryViewBufferBlock.bottom = 199;
            AllocateCutsceneViewportOrFatal(
                &g_stSecondaryViewBufferBlock, 0, 0);
            RouteCutsceneViewportToDisplay();
            ReleaseCutsceneViewport(
                &g_9_stCutsceneTextBackingPort);
            g_9_stCutsceneTextBackingPort =
                g_stSecondaryViewBufferBlock;
            break;
        case 0xb8:
            protectCondition = (signed char)PopCutsceneScriptValue(
                &topOperand, scriptValueStack + 10);
            protectCondition = 1;
            break;
        case 0x22:
            shouldContinue = 0;
            break;
        case 0x23:
            *scriptCursor = scriptBytes;
            return 0;
        case 0x24:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                                    (short)(signed char)*scriptBytes++);
            break;
        case 0x25:
#ifdef SDL_PORT
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                                    *(short *)scriptBytes);
            scriptBytes += 2;
#else
            /* The original word cursor uses MSVC's pointer aliasing. */
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                                    *(*(short **)&scriptBytes)++);
#endif
            break;
        case 0x26:
            if (g_pCampaignGlobals == 0)
                FatalCutsceneError("A28");
            value = 0;
            while (*scriptBytes == 0xff) {
                value = (short)(value + *scriptBytes++);
            }
            value = (short)(value + *scriptBytes++);
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                ((short *)g_pCampaignGlobals)[value]);
            break;
        case 0xae:
            value = *scriptBytes++;
            switch (value) {
            case 0:
                g_stCutsceneTextBounds.left =
                    PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
                break;
            case 1:
                g_stCutsceneTextBounds.top =
                    PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
                break;
            case 2:
                g_stCutsceneTextBounds.right =
                    PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
                break;
            case 3:
                g_stCutsceneTextBounds.bottom =
                    PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
                break;
            }
            g_bCutsceneTextBoundsOverride = 1;
            break;
        case 0xaf:
            g_29_pCurrentTextContext->alignment = *scriptBytes++;
            break;
        case 0x28:
            if (g_pCampaignGlobals == 0)
                FatalCutsceneError("A29");
            value = 0;
            while (*scriptBytes == 0xff) {
                value = (short)(value + *scriptBytes++);
            }
            value = (short)(value + *scriptBytes++);
            ((short *)g_pCampaignGlobals)[value] =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x27:
            if (g_pCampaignGlobals == 0)
                FatalCutsceneError("A30");
            value = *(short *)scriptBytes;
            scriptBytes += 2;
            value = (short)(value + PopCutsceneScriptValue(
                &topOperand, scriptValueStack + 10));
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                ((short *)g_pCampaignGlobals)[value]);
            break;
        case 0x29:
            if (g_pCampaignGlobals == 0)
                FatalCutsceneError("A31");
            value = *(short *)scriptBytes;
            scriptBytes += 2;
            value = (short)(value + PopCutsceneScriptValue(
                &topOperand, scriptValueStack + 10));
            ((short *)g_pCampaignGlobals)[value] =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x2a:
            if (g_pCurrentCutsceneSprite->currentFrame <
                g_pCurrentCutsceneSprite->finalFrame - 1)
                g_pCurrentCutsceneSprite->currentFrame++;
            else
                g_pCurrentCutsceneSprite->currentFrame = 0;
            break;
        case 0x2b:
            if (g_pCurrentCutsceneSprite->currentFrame != 0)
                g_pCurrentCutsceneSprite->currentFrame--;
            else
                g_pCurrentCutsceneSprite->currentFrame =
                    (short)(g_pCurrentCutsceneSprite->finalFrame - 1);
            break;
        case 0x2c:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->active);
            break;
        case 0x2d:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->visible);
            break;
        case 0x2e:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->drawType);
            break;
        case 0x30:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->currentFrame);
            break;
        case 0x31:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->finalFrame);
            break;
        case 0x32:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->waitTicks);
            break;
        case 0x2f:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->baseFrame);
            break;
        case 0x33:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->delayFrames);
            break;
        case 0x34:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->x);
            break;
        case 0x35:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->y);
            break;
        case 0x36:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->field_2f);
            break;
        case 0x37:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->rotation);
            break;
        case 0x38:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->scale);
            break;
        case 0x39:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                (short)g_pCurrentCutsceneSprite->drawFlags);
            break;
        case 0x3d:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->locals[*scriptBytes++]);
            break;
        case 0x3a:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->deltaX);
            break;
        case 0x3b:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->deltaY);
            break;
        case 0x3c:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSprite->deltaDepth);
            break;
        case 0x3e:
            g_pCurrentCutsceneSprite->active = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x41:
            g_pCurrentCutsceneSprite->baseFrame =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x3f:
            g_pCurrentCutsceneSprite->visible = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x40:
            g_pCurrentCutsceneSprite->drawType =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x42:
            g_pCurrentCutsceneSprite->currentFrame =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x43:
            g_pCurrentCutsceneSprite->finalFrame =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x44:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            if (g_bCutsceneSkipFrame != 0)
                value = 0;
#ifdef SDL_PORT
            /* See CUTSCENE_SPRITE_MIN_TICKS: a short script-authored wait
             * relied on the DOS host's own draw speed, not a real-time
             * gate, to look right. */
            if (value > 0 && value < CUTSCENE_SPRITE_MIN_TICKS)
                value = CUTSCENE_SPRITE_MIN_TICKS;
#endif
            g_pCurrentCutsceneSprite->waitTicks = value;
            g_pCurrentCutsceneSprite->waitStart =
                g_196_nInputTimerTicks;
            if (objectType == 0) {
                *scriptCursor = scriptBytes;
                return shouldContinue;
            }
            break;
        case 0x45:
            g_pCurrentCutsceneSprite->delayFrames =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x46:
            g_pCurrentCutsceneSprite->x =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x47:
            g_pCurrentCutsceneSprite->y =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x48:
            g_pCurrentCutsceneSprite->field_2f =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x49:
            g_pCurrentCutsceneSprite->rotation =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x4a:
            g_pCurrentCutsceneSprite->scale =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x4b:
            g_pCurrentCutsceneSprite->drawFlags =
                (unsigned short)PopCutsceneScriptValue(
                    &topOperand, scriptValueStack + 10);
            break;
        case 0x4f:
            g_pCurrentCutsceneSprite->locals[*scriptBytes++] =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x4c:
            g_pCurrentCutsceneSprite->deltaX = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x4d:
            g_pCurrentCutsceneSprite->deltaY = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x4e:
            g_pCurrentCutsceneSprite->deltaDepth = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0xab:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutscenePlane->waitTicks);
            break;
        case 0x50:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutscenePlane->active);
            break;
        case 0x51:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutscenePlane->visible);
            break;
        case 0x52:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutscenePlane->x);
            break;
        case 0x53:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutscenePlane->y);
            break;
        case 0x54:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutscenePlane->depth);
            break;
        case 0x55:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutscenePlane->rotation);
            break;
        case 0x56:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutscenePlane->scale);
            break;
        case 0x58:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                (short)g_pCurrentCutscenePlane->drawFlags);
            break;
        case 0x57:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutscenePlane->locals[*scriptBytes++]);
            break;
        case 0x59:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutscenePlane->deltaX);
            break;
        case 0x5a:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutscenePlane->deltaY);
            break;
        case 0x5b:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutscenePlane->deltaDepth);
            break;
        case 0x5c:
            g_pCurrentCutscenePlane->active = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x5d:
            g_pCurrentCutscenePlane->visible = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x5e:
            g_pCurrentCutscenePlane->x =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x5f:
            g_pCurrentCutscenePlane->y =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x60:
            g_pCurrentCutscenePlane->depth =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x61:
            g_pCurrentCutscenePlane->rotation =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x62:
            g_pCurrentCutscenePlane->scale =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x64:
            g_pCurrentCutscenePlane->drawFlags =
                (unsigned short)PopCutsceneScriptValue(
                    &topOperand, scriptValueStack + 10);
            break;
        case 0x63:
            g_pCurrentCutscenePlane->locals[*scriptBytes++] =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x65:
            g_pCurrentCutscenePlane->deltaX = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x66:
            g_pCurrentCutscenePlane->deltaY = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x67:
            g_pCurrentCutscenePlane->deltaDepth = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0xaa:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            if (g_bCutsceneSkipFrame != 0)
                value = 0;
#ifdef SDL_PORT
            /* See CUTSCENE_SPRITE_MIN_TICKS: a short script-authored wait
             * relied on the DOS host's own draw speed, not a real-time
             * gate, to look right. */
            if (value > 0 && value < CUTSCENE_SPRITE_MIN_TICKS)
                value = CUTSCENE_SPRITE_MIN_TICKS;
#endif
            g_pCurrentCutscenePlane->waitTicks = value;
            g_pCurrentCutscenePlane->waitStart =
                g_196_nInputTimerTicks;
            if (objectType == 1) {
                *scriptCursor = scriptBytes;
                return shouldContinue;
            }
            break;
        case 0x71:
            g_pCurrentCutsceneSequence->locals[*scriptBytes++] =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x70:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSequence->locals[*scriptBytes++]);
            break;
        case 0x68:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSequence->active);
            break;
        case 0x69:
            g_pCurrentCutsceneSequence->active = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x6a:
#ifdef SDL_PORT
            /* Scripts can spin on this timer read without drawing another
             * frame. Keep skip input responsive and yield while it runs. */
            if (g_pCurrentCutsceneSequence->waitTicks > 0 &&
                g_bCutsceneSkipFrame == 0 &&
                g_bCutsceneViewportPreallocated == 0) {
                PumpWindowMessages(0);
                if (g_bSceneEscapeRequested == 0 && g_nInputPressCount == 0)
                    Sleep(1);
            }
#endif
            if (g_bCutsceneSkipFrame != 0)
                g_pCurrentCutsceneSequence->waitTicks = 0;
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_pCurrentCutsceneSequence->waitTicks);
            break;
        case 0x6b:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            if (value != 0 && g_bCutsceneSkipFrame == 0) {
                while (g_196_nInputTimerTicks <
                       g_pCurrentCutsceneSequence->waitStart +
                           g_pCurrentCutsceneSequence->waitTicks) {
                    PumpWindowMessages(0);
#ifdef SDL_PORT
                    /* See issue #33 (ESC feels slow to skip cutscenes):
                     * this loop only pumps the OS message queue, which is
                     * what actually sets g_bSceneEscapeRequested
                     * (sdl/events.c) the instant Escape is pressed -- but
                     * nothing here ever reads that flag. The real
                     * escape-to-skip conversion lives in the outer script
                     * loop above (the queued-input dispatch), which only
                     * runs BETWEEN opcodes.
                     * A long wait (this opcode is script-timed, often
                     * paced to the scene's music/dialogue) blocks that
                     * outer loop from ever running again until the wait
                     * naturally elapses, so Escape sits pending and
                     * ignored for however long is left. Bail out of the
                     * wait itself the instant Escape is pending, the same
                     * way case 0x7e already does for its own wait, and
                     * let the outer loop perform the real conversion on
                     * its very next iteration. */
                    if (g_bSceneEscapeRequested != 0)
                        break;
#endif
                }
            }
            g_pCurrentCutsceneSequence->waitTicks = value;
            g_pCurrentCutsceneSequence->waitStart =
                g_196_nInputTimerTicks;
            break;
        case 0x81:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            if (g_cCutsceneVideoMode == 0x0d)
                value = g_abCutscenePaletteTranslation[value];
            g_2_stCutsceneMessageCtxInfo.colour = (unsigned char)value;
            break;
        case 0x8b:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            if (g_bCutsceneSkipFrame == 0 &&
                g_bCutsceneViewportPreallocated == 0) {
                ClearCutsceneViewport(
                    &g_15_stSceneFlicScratchViewport,
                    (unsigned char)value);
                PanToScreen(&g_stSecondaryViewBufferBlock,
                            &g_15_stSceneFlicScratchViewport);
                MarkDibDirty();
                DIBslamReal();
                CopyViewportContents(&g_stSecondaryViewBufferBlock,
                                     &g_15_stSceneFlicScratchViewport);
            }
            break;
        case 0x8c:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            SaveGamePalette();
            FadeViewportPaletteToColour(
                &g_2_stModalSourceWindow, value, 1);
            ClearCutsceneViewport(
                &g_2_stModalSourceWindow,
                (unsigned char)value);
            MarkDibDirty();
            DIBslamReal();
            RestoreGamePalette();
            break;
        case 0xb3:
            secondaryOperand = PopCutsceneScriptValue(
                &topOperand, scriptValueStack + 10);
            baseValue = PopCutsceneScriptValue(
                &topOperand, scriptValueStack + 10);
            g_117_nCutsceneFormatValueWord =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            g_3_nCutsceneTextIdx = *(short *)scriptBytes;
            scriptBytes += 2;
            g_4_pszFilmSourceText =
                g_pCutsceneTextResources->entries[
                    g_3_nCutsceneTextIdx];
            ExpandCutsceneText(
                (unsigned char *)g_4_pszFilmSourceText,
                g_27_pszCutsceneFormattedText);
            DrawCutsceneTextAt(baseValue, secondaryOperand, 4,
                               g_27_pszCutsceneFormattedText);
            break;
        case 0x6f:
            g_3_nCutsceneTextIdx = *(short *)scriptBytes;
            scriptBytes += 2;
            g_4_pszFilmSourceText =
                g_pCutsceneTextResources->entries[
                    g_3_nCutsceneTextIdx];
            ExpandCutsceneText(
                (unsigned char *)g_4_pszFilmSourceText,
                g_116_pszFilmPrintStore);
            g_pszCurrentCutsceneText =
                g_116_pszFilmPrintStore;
            break;
        case 0xb4:
            g_nCutsceneMusicVolume =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x6e:
            secondaryOperand = PopCutsceneScriptValue(
                &topOperand, scriptValueStack + 10);
            baseValue = PopCutsceneScriptValue(
                &topOperand, scriptValueStack + 10);
            value = *scriptBytes++;
            DrawCutsceneTextAt(baseValue, secondaryOperand, value,
                               g_116_pszFilmPrintStore);
            break;
        case 0xad:
            ReleaseCutsceneSpriteShape(g_apSceneObjects[
                FindActiveCutsceneObjectResources(
                    g_pCutsceneSpriteResources)->
                    scriptSymbolIndices[*scriptBytes++]]);
            break;
        case 0x94:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            switch (*scriptBytes++) {
            case 0:
                ClearCutsceneViewport(
                    &g_15_stSceneFlicScratchViewport,
                    (unsigned char)value);
                break;
            case 1:
                ClearCutsceneViewport(&g_stSecondaryViewBufferBlock,
                                      (unsigned char)value);
                break;
            case 2:
                ClearCutsceneViewport(&g_10_stCutsceneStringViewportBlock,
                                      (unsigned char)value);
                break;
            case 3:
                ClearCutsceneViewport(
                    &g_9_stCutsceneTextBackingPort,
                    (unsigned char)value);
                break;
            case 4:
                ClearCutsceneViewport(&g_2_stModalSourceWindow,
                                      (unsigned char)value);
                break;
            }
            break;
        case 0xa3:
            PushCutsceneScriptValue(
                &topOperand, scriptValueStack,
                (short)(signed char)g_nOriginDevUnlock);
            break;
        case 0x9d:
            value = 0;
            if (g_bCutsceneSkipFrame != 0)
                value = 1;
            if (g_bCutsceneSkipAll != 0)
                value = 2;
            PushCutsceneScriptValue(&topOperand, scriptValueStack, value);
            break;
        case 0xa4:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            switch (value) {
            case -2:
                g_bCutsceneSkipFrame =
                    g_bCutsceneSkipAll = 1;
                break;
            case -1:
                g_bCutsceneSkipFrame = 1;
                break;
            case 0:
                g_bCutsceneSkipFrame =
                    g_bCutsceneSkipAll = 0;
                ServiceInputDevices(-1);
                FlushPendingInputEvents();
                break;
            case 1:
                g_bCutsceneSkipFrame = 0;
                ServiceInputDevices(-1);
                FlushPendingInputEvents();
                break;
            }
            break;
        case 0x9b:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                (short)(g_stSecondaryViewBufferBlock.bottom + 1));
            break;
        case 0x9c:
            PushCutsceneScriptValue(&topOperand, scriptValueStack,
                g_2_bMovieStringAdvanceState);
            break;
        case 0xa2:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            LoadCutsceneSpeechSlot(*scriptBytes++, value);
            CopyViewportContents(&g_15_stSceneFlicScratchViewport,
                                 &g_stSecondaryViewBufferBlock);
            break;
        case 0xb0: {
#ifdef SDL_PORT
            /* See issue #33 (review on PR #34): breaking out here while the
             * cache is still busy must NOT fall through to the "use and
             * clear" logic below as if the fetch had finished -- that would
             * read g_3_apSceneSpeechPacketsList[value] before it is
             * valid and then clear the still-in-flight fetch's own
             * bookkeeping (g_12_apszMovieSpeechFiles[value] etc.),
             * silently dropping it. g_wDigiPlaying is never
             * set nonzero anywhere in this reconstruction today, so this
             * loop cannot actually break early yet -- guarded correctly
             * anyway rather than leaving unsound-if-ever-live code here. */
            signed char cacheStillBusy = 0;
#endif
            while (g_wDigiPlaying != 0) {
                PumpWindowMessages(0);
#ifdef SDL_PORT
                if (g_bSceneEscapeRequested != 0) {
                    cacheStillBusy = 1;
                    break;
                }
#endif
            }
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
#ifdef SDL_PORT
            if (cacheStillBusy)
                break;
#endif
            if (g_3_apSceneSpeechPacketsList[value] != 0 &&
                g_12_apszMovieSpeechFiles[value] != 0) {
                PlaySpeechPacketBuffer(
                    g_3_apSceneSpeechPacketsList[value],
                    g_1_asCutsceneSpeechChannelsTable[value], 1);
            } else if (g_12_apszMovieSpeechFiles[value] != 0) {
                LoadAndPlaySpeechPacket(
                    g_12_apszMovieSpeechFiles[value],
                    g_19_asCutsceneSpeechSectionsList[value]);
                CopyViewportContents(
                    &g_15_stSceneFlicScratchViewport,
                    &g_stSecondaryViewBufferBlock);
                g_3_apSceneSpeechPacketsList[value] = 0;
                g_1_asCutsceneSpeechChannelsTable[value] = 0;
            }
            g_12_apszMovieSpeechFiles[value] = 0;
            g_19_asCutsceneSpeechSectionsList[value] = 0;
            break;
        }
        case 0xa9:
            value = 0;
            if ((g_aObjectTypeData[
                     g_acObjectType[g_nYourWingman]].
                     damageCapacity >> 1) - 1 <=
                g_asObjectDamage[g_nYourWingman]) {
                value++;
            }
            PushCutsceneScriptValue(&topOperand, scriptValueStack, value);
            break;
        case 0x96:
            ReleaseCutsceneFontPacket();
            break;
        case 0x82:
            g_2_stCutsceneMessageCtxInfo.font =
                LoadCachedCutsceneResource(
                    FindActiveCutsceneFileResources(
                        g_pCutsceneFontResources),
                    *scriptBytes++, 1);
            g_9_bCinematicSpriteTextEnableStatus =
                (signed char)*scriptBytes++;
            g_1_nCutsceneFontOwnerNumber =
                g_nActiveCutsceneResourceLevel;
            break;
        case 0x9f:
            g_cCutsceneSoundVolume = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x91:
            value = *scriptBytes++;
            SoundDebugPrintf(
                "VAL=========================================%d\n",
                (int)value);
            if (g_bCutsceneSkipFrame == 0 &&
                g_bCutsceneViewportPreallocated == 0) {
                PlaySfxWaveFileByNumber(
                    MapCutsceneSoundEffectNumber((int)value),
                    -1, 0);
            }
            PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x93:
            value = (signed char)*scriptBytes++;
            ReleaseCutsceneSoundEffects(value);
            break;
        case 0x84:
            value = *scriptBytes++;
            if ((g_nMusicDriverMode == 1 ||
                 g_nMusicDriverMode == 2) &&
                g_bCutsceneViewportPreallocated == 0) {
                FlushSoundEffects();
                ReleasePacketSlot(&g_pCutsceneFxPacket);
                g_pCutsceneFxPacket =
                    LoadCachedCutsceneResource(
                        FindActiveCutsceneFileResources(
                            g_pCutsceneFxResources), value, 3);
                SoundDebugPrintf("LOAD FX val=%d\n", (int)value);
            }
            break;
        case 0x98:
            FlushSoundEffects();
            ReleasePacketSlot(&g_pCutsceneFxPacket);
            break;
        case 0xb2:
            value = *scriptBytes++;
            if ((g_nMusicDriverMode == 1 ||
                 g_nMusicDriverMode == 2) &&
                g_bCutsceneViewportPreallocated == 0) {
                FadeMusic(value);
            }
            break;
        case 0x90:
            if (g_pSceneMusicPacket != 0)
                StopMusic(0);
            value = *scriptBytes++;
            if (g_bCutsceneSkipFrame != 0 ||
                g_bCutsceneViewportPreallocated != 0)
                break;
            g_pSceneMusicPacket = FindLoadedCutsceneMusic(value);
            StartInteractiveMusic(value);
            break;
        case 0x92:
#ifdef SDL_PORT
            StopMusic(*(short *)scriptBytes);
            scriptBytes += 2;
#else
            StopMusic(*(*(short **)&scriptBytes)++);
#endif
            g_pSceneMusicPacket = 0;
            break;
        case 0x85:
            value = *scriptBytes++;
            break;
        case 0x97:
            ReleaseCutsceneMusicNodes(
                &g_pCutsceneMusicPackets, *scriptBytes++);
            break;
        case 0x83:
            if (g_bCutsceneViewportPreallocated != 0) {
                scriptBytes += 3;
            } else {
                unsigned char paletteIndex;
                short colourEntries;
                unsigned char firstColour;

                paletteIndex = *scriptBytes++;
                g_1_pActiveSceneFileResourcesRef =
                    FindActiveCutsceneFileResources(
                        g_pCutscenePaletteResources);
                g_26_pFilmRgbPacket =
                    LoadCachedCutsceneResource(
                        g_1_pActiveSceneFileResourcesRef,
                        paletteIndex, 1);
                firstColour = *scriptBytes++;
                colourEntries = (short)(*scriptBytes++ + 1);
                ConsumeCutscenePalettePacket(firstColour, colourEntries, 0);
                ReleaseLoadedCutsceneResource(
                    g_1_pActiveSceneFileResourcesRef,
                    paletteIndex);
                g_26_pFilmRgbPacket = 0;
                break;
            }
            /* A preallocated viewport falls through to the frame check. */
        case 0x7c:
            if (g_bCutsceneViewportPreallocated != 0 ||
                g_bCutsceneSkipFrame != 0)
                break;
            while (g_196_nInputTimerTicks <
                   g_nNextCutsceneFrameClock) {
                PumpWindowMessages(0);
#ifdef SDL_PORT
                /* See issue #33; same fix as case 0x6b above. This
                 * runs every cutscene frame, so most frames' delay is
                 * short enough not to matter, but a shot with a long
                 * configured frame delay (e.g. paced to music) can
                 * still eat a pending Escape the same way 0x6b did. */
                if (g_bSceneEscapeRequested != 0)
                    break;
#endif
            }
            PresentCutsceneFrame(&g_stSecondaryViewBufferBlock,
                                 &g_15_stSceneFlicScratchViewport);
            if (g_pMemoryLogFile != 0)
                LogChangedCutsceneWorkBuffers();
            RestoreCutsceneTextBacking();
            g_bMovieFramePresented = 1;
            if (g_nCutsceneFrameDelay != 0) {
                g_nNextCutsceneFrameClock =
                    g_196_nInputTimerTicks +
                    g_nCutsceneFrameDelay;
            }
            break;
        case 0x6d:
            if (g_bCutsceneDrawingEnabled == 0 &&
                (g_bCutsceneSkipFrame != 0 ||
                 g_bCutsceneViewportPreallocated != 0)) {
                scriptBytes++;
                break;
            }
            value = *scriptBytes++;
            g_pCutsceneSpriteParentPlane = 0;
            if (value != 0xff) {
                value = FindActiveCutsceneObjectResources(
                    g_pCutsceneSpriteResources)->
                    scriptSymbolIndices[value];
                UpdateCutsceneSpriteObject(g_apSceneObjects[value]);
                DrawCutsceneSprite(g_apSceneObjects[value]);
            } else if (g_pCurrentCutsceneSprite != 0) {
                UpdateCutsceneSpriteObject(g_pCurrentCutsceneSprite);
                DrawCutsceneSprite(g_pCurrentCutsceneSprite);
            }
            g_pCutsceneSpriteParentPlane =
                g_pCurrentCutscenePlane;
            break;
        case 0x6c:
            if (g_bCutsceneDrawingEnabled == 0 &&
                (g_bCutsceneSkipFrame != 0 ||
                 g_bCutsceneViewportPreallocated != 0)) {
                scriptBytes++;
                break;
            }
            value = *scriptBytes++;
            if (value != 0xff) {
                value = FindActiveCutsceneObjectResources(
                    g_pCutscenePlaneResources)->
                    scriptSymbolIndices[value];
                UpdateCutscenePlaneObject(g_apCutscenePlanes[value], 1);
                DrawCutscenePlane(g_apCutscenePlanes[value]);
            } else if (g_pCurrentCutscenePlane != 0) {
                UpdateCutscenePlaneObject(g_pCurrentCutscenePlane, 1);
                DrawCutscenePlane(g_pCurrentCutscenePlane);
            }
            break;
        case 0x86:
            g_bCutsceneDrawingEnabled = (signed char)
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0xb9:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            g_3_nCutsceneTextIdx = *(short *)scriptBytes;
            scriptBytes += 2;
            g_4_pszFilmSourceText =
                g_pCutsceneTextResources->entries[
                    g_3_nCutsceneTextIdx];
            if (g_pMemoryLogFile != 0) {
                fprintf(g_pMemoryLogFile,
                        "%%%%%% ELOG: %Fs = %d\n",
                        g_4_pszFilmSourceText, value);
            }
            break;
        case 0xb1:
            value = *(short *)scriptBytes;
            scriptBytes += 2;
            if (g_pMemoryLogFile != 0) {
                fprintf(g_pMemoryLogFile,
                        "@@@@@@@@@@@@@@@@ Logged: %d\n", value);
                WriteDetailedMemoryStateReport();
            }
            break;
        case 0xa1:
            FormatCinematicMemoryStatus("Report");
            WriteDetailedMemoryStateReport();
            break;
        case 0x95:
            g_cCutsceneTextStyle = (signed char)*scriptBytes++;
            break;
        case 0x89: {
            unsigned char filmNumber;
            CutsceneResourceTable *savedFilmResources;
            void *packet;

            g_nSceneFlicContext = 0x40;
            filmNumber = *scriptBytes++;
            if (g_bCutsceneViewportPreallocated == 0) {
                packet = LoadCachedCutsceneResource(
                    FindActiveCutsceneFileResources(
                        g_pCutsceneFilmResources), filmNumber, 0);
                savedFilmResources = g_pCutsceneFilmResources;
                g_bCutsceneDrawingEnabled = 0;
                g_bCutsceneSkipFrame = 0;
                g_nCutsceneResourceLevel++;
                g_nActiveCutsceneResourceLevel =
                    g_nCutsceneResourceLevel;
                ParseCutsceneContainer(packet);
                WriteMemoryStateReportHook();
                DosMemset(g_4_pszFilmWorkScratch, 4, 0);
                ClearCutsceneTextViewport();
                ClearCutsceneViewport(
                    &g_10_stCutsceneStringViewportBlock, 0);
                ExecuteCutsceneScene(g_apCutsceneScenes[
                    FindActiveCutsceneObjectResources(
                        g_pCutsceneSceneResources)->
                        scriptSymbolIndices[0]]);
                ReleaseCutsceneResourceLevel(
                    g_nCutsceneResourceLevel);
                g_pCutsceneFilmResources = savedFilmResources;
                g_nCutsceneResourceLevel--;
                g_nActiveCutsceneResourceLevel =
                    g_nCutsceneResourceLevel;
                g_bCutsceneSkipFrame = 0;
                ReleaseLoadedCutsceneResource(
                    FindActiveCutsceneFileResources(
                        g_pCutsceneFilmResources), filmNumber);
                g_bCutsceneDrawingEnabled = 0;
            }
            break;
        }
        case 0x88:
            ExecuteCutsceneSequence(
                g_apCutsceneSequences[
                    FindActiveCutsceneObjectResources(
                        g_pCutsceneSequenceResources)->
                        scriptSymbolIndices[*scriptBytes++]],
                0, 1);
            if (g_bCutsceneSkipFrame != 0 && objectType == 3)
                g_bCutsceneSkipFrame = 0;
            break;
        case 0x87:
            g_3_nCutsceneTextIdx = *(short *)scriptBytes;
            scriptBytes += 2;
            g_4_pszFilmSourceText =
                g_pCutsceneTextResources->entries[
                    g_3_nCutsceneTextIdx];
            ExecuteCutsceneSequence(
                g_apCutsceneSequences[
                    FindActiveCutsceneObjectResources(
                        g_pCutsceneSequenceResources)->
                        scriptSymbolIndices[*scriptBytes++]],
                (unsigned char *)g_4_pszFilmSourceText, 1);
            if (g_bCutsceneSkipFrame != 0 &&
                (objectType == 3 || objectType == 2)) {
                g_bCutsceneSkipFrame = 0;
            }
            break;
        case 0x8d:
            g_nCutsceneFrameDelay =
                (unsigned short)PopCutsceneScriptValue(
                    &topOperand, scriptValueStack + 10);
            break;
        case 0x8e:
            g_nCutsceneFrameTick =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x8f:
            g_4_nCutsceneResourceScratchValue = *(short *)scriptBytes;
            scriptBytes += 2;
            /* The original divides by 0x3b (59), not 0x3c (60).
             * g_196_nInputTimerTicks (what this delay is measured against,
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
            g_nCutsceneFrameDelay =
                (unsigned short)(0x3c / g_4_nCutsceneResourceScratchValue);
#else
            g_nCutsceneFrameDelay =
                (unsigned short)(0x3b / g_4_nCutsceneResourceScratchValue);
#endif
            break;
        case 0x75:
            scriptLocal = g_pCurrentCutsceneScene->locals;
            scriptLocal += *scriptBytes++ * 2;
            *scriptLocal =
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            break;
        case 0x74:
            scriptLocal = g_pCurrentCutsceneScene->locals;
            scriptLocal += *scriptBytes++ * 2;
            PushCutsceneScriptValue(&topOperand, scriptValueStack, *scriptLocal);
            break;
        case 0x78:
            if (g_pCurrentCutsceneSprite->linkedScript != 0) {
                g_8_nSavedCutsceneResourceOwnerValue =
                    g_nActiveCutsceneResourceLevel;
            }
            if (g_pCurrentCutsceneSprite == 0) {
                g_pCurrentCutsceneSprite =
                    g_apSceneObjects[0];
            }
            if (g_pCurrentCutsceneSprite == 0)
                FatalCutsceneError("A32");
            g_pCurrentCutsceneSprite->drawType = *scriptBytes++;
            g_pCurrentCutsceneSprite->baseFrame = *scriptBytes++;
            g_pCurrentCutsceneSprite->finalFrame = *scriptBytes++;
            g_pCurrentCutsceneSprite->currentFrame = 0;
            value = 0;
            while (*scriptBytes == 0xff) {
                value = (short)(value + *scriptBytes++);
            }
            value = (short)(value + *scriptBytes++);
            if (g_pCurrentCutsceneSprite->linkedScript != 0) {
                g_nActiveCutsceneResourceLevel =
                    g_pCurrentCutsceneSprite->owner;
            }
            if (g_pCurrentCutsceneSprite->drawType != 4) {
                g_pCurrentCutsceneSprite->shape =
                    LoadCachedCutsceneResource(
                        FindActiveCutsceneFileResources(
                            g_pCutsceneShapeResources), value, 1);
            } else {
                InitializeSceneFlicStream(
                    FindActiveCutsceneFileResources(
                        g_pCutsceneShapeResources),
                    value, g_pCurrentCutsceneSprite);
            }
            if (g_pCurrentCutsceneSprite->linkedScript != 0) {
                g_nActiveCutsceneResourceLevel =
                    g_8_nSavedCutsceneResourceOwnerValue;
            }
            break;
        case 0x99:
            value = 0;
            while (*scriptBytes == 0xff) {
                value = (short)(value + *scriptBytes++);
            }
            value = (short)(value + *scriptBytes++);
            ReleaseLoadedCutsceneResource(
                FindActiveCutsceneFileResources(
                    g_pCutsceneShapeResources), value);
            break;
        case 0x9a:
            g_1_pActiveSceneFileResourcesRef =
                FindActiveCutsceneFileResources(
                g_pCutsceneShapeResources);
            g_4_nCutsceneResourceScratchValue =
                g_1_pActiveSceneFileResourcesRef->count;
            while (g_4_nCutsceneResourceScratchValue-- != 0) {
                ReleaseLoadedCutsceneResource(
                    g_1_pActiveSceneFileResourcesRef,
                    g_4_nCutsceneResourceScratchValue);
            }
            break;
        case 0x7b:
            if (g_bCutsceneViewportPreallocated != 0) {
                scriptBytes += 2;
                break;
            }
            switch ((signed char)*scriptBytes++) {
            case 0:
                if (g_pCurrentCutscenePlane == 0)
                    FatalCutsceneError("A33");
                g_4_nCutsceneResourceScratchValue =
                    FindActiveCutsceneObjectResources(
                        g_pCutsceneSpriteResources)->
                        scriptSymbolIndices[*scriptBytes++];
                g_pCurrentCutscenePlane->spriteCount =
                    RemoveCutsceneMemberIndex(
                        g_pCurrentCutscenePlane->spriteCount,
                        g_pCurrentCutscenePlane->spriteIndices,
                        (signed char)g_4_nCutsceneResourceScratchValue);
                break;
            case 1:
                if (g_pCurrentCutsceneSequence == 0)
                    FatalCutsceneError("A34");
                g_4_nCutsceneResourceScratchValue =
                    FindActiveCutsceneObjectResources(
                        g_pCutscenePlaneResources)->
                        scriptSymbolIndices[*scriptBytes++];
                g_pCurrentCutsceneSequence->planeCount =
                    RemoveCutsceneMemberIndex(
                        g_pCurrentCutsceneSequence->planeCount,
                        g_pCurrentCutsceneSequence->planeIndices,
                        (signed char)g_4_nCutsceneResourceScratchValue);
                break;
            case 2:
            case 3:
                scriptBytes++;
                break;
            case 4:
                if (g_pCurrentCutscenePlane == 0)
                    FatalCutsceneError("A35");
                g_pCurrentCutscenePlane->spriteCount = 0;
                break;
            case 5:
                if (g_pCurrentCutsceneSequence == 0)
                    FatalCutsceneError("A36");
                g_pCurrentCutsceneSequence->planeCount = 0;
                break;
            case 6:
                if (g_pCurrentCutsceneScene == 0)
                    FatalCutsceneError("A37");
                g_pCurrentCutsceneScene->sequenceCount = 0;
                break;
            case 7:
                break;
            }
            break;
        case 0x79:
            if (g_bCutsceneViewportPreallocated != 0) {
                scriptBytes += 2;
                break;
            }
            switch ((signed char)*scriptBytes++) {
            case 0:
                if (g_pCurrentCutscenePlane == 0)
                    FatalCutsceneError("A38");
                g_pCurrentCutscenePlane->spriteIndices[
                    g_pCurrentCutscenePlane->spriteCount++] =
                    (unsigned char)FindActiveCutsceneObjectResources(
                        g_pCutsceneSpriteResources)->
                        scriptSymbolIndices[*scriptBytes++];
                break;
            case 1:
                if (g_pCurrentCutsceneSequence == 0)
                    FatalCutsceneError("A39");
                g_pCurrentCutsceneSequence->planeIndices[
                    g_pCurrentCutsceneSequence->planeCount++] =
                    (unsigned char)FindActiveCutsceneObjectResources(
                        g_pCutscenePlaneResources)->
                        scriptSymbolIndices[*scriptBytes++];
                break;
            case 2:
                if (g_pCurrentCutsceneScene == 0)
                    FatalCutsceneError("A40");
                g_pCurrentCutsceneScene->sequenceIndices[
                    g_pCurrentCutsceneScene->sequenceCount++] =
                    (unsigned char)FindActiveCutsceneObjectResources(
                        g_pCutsceneSequenceResources)->
                        scriptSymbolIndices[*scriptBytes++];
                break;
            case 3:
                scriptBytes++;
                break;
            }
            break;
        case 0xac:
            value = *scriptBytes++;
            CopyCutsceneSpriteDisplay(value, *scriptBytes++);
            break;
        case 0xb7:
            CopyViewportContents(&g_15_stSceneFlicScratchViewport,
                                 &g_stSecondaryViewBufferBlock);
            break;
        case 0xa8:
            value = *scriptBytes++;
            LinkCutsceneSpriteScript(value, *scriptBytes++);
            break;
        case 0x7a:
            switch ((signed char)*scriptBytes++) {
            case 0:
                g_pLinkedCutsceneSprite =
                    g_apSceneObjects[
                    FindActiveCutsceneObjectResources(
                        g_pCutsceneSpriteResources)->
                        scriptSymbolIndices[*scriptBytes++]];
                if (g_pLinkedCutsceneSprite == 0)
                    FatalCutsceneError("A41");
                if (g_pLinkedCutsceneSprite->linkedScript != 0) {
                    g_pLinkedCutsceneSprite->scriptStart =
                        g_pLinkedCutsceneSprite->linkedScript;
                    if (g_pLinkedCutsceneSprite ==
                            g_pCurrentCutsceneSprite &&
                        objectType == 0)
                        FatalCutsceneError("A42");
                    g_pLinkedCutsceneSprite->linkedScript = 0;
                }
                if (g_pLinkedCutsceneSprite ==
                        g_pCurrentCutsceneSprite &&
                    objectType == 0) {
                    scriptBytes =
                        g_pCurrentCutsceneSprite->scriptStart;
                }
                g_pLinkedCutsceneSprite->scriptCursor =
                    g_pLinkedCutsceneSprite->scriptStart;
                g_pLinkedCutsceneSprite->waitTicks =
                    g_pLinkedCutsceneSprite->delayFrames = 0;
                ReleaseCutsceneSpriteShape(
                    g_pLinkedCutsceneSprite);
                g_pLinkedCutsceneSprite->owner =
                    g_pLinkedCutsceneSprite->linkedOwner;
                break;
            case 1:
                g_pLinkedCutscenePlane =
                    g_apCutscenePlanes[
                    FindActiveCutsceneObjectResources(
                        g_pCutscenePlaneResources)->
                        scriptSymbolIndices[*scriptBytes++]];
                if (g_pLinkedCutscenePlane == 0)
                    FatalCutsceneError("A43");
                if (g_pLinkedCutscenePlane ==
                        g_pCurrentCutscenePlane &&
                    objectType == 1) {
                    scriptBytes =
                        g_pCurrentCutscenePlane->scriptStart;
                }
                g_pLinkedCutscenePlane->scriptCursor =
                    g_pLinkedCutscenePlane->scriptStart;
                break;
            case 2:
                g_pLinkedCutsceneSequence =
                    g_apCutsceneSequences[
                    FindActiveCutsceneObjectResources(
                        g_pCutsceneSequenceResources)->
                        scriptSymbolIndices[*scriptBytes++]];
                if (g_pLinkedCutsceneSequence == 0)
                    FatalCutsceneError("A44");
                if (g_pLinkedCutsceneSequence ==
                        g_pCurrentCutsceneSequence &&
                    objectType == 1) {
                    scriptBytes =
                        g_pCurrentCutsceneSequence->scriptStart;
                }
                g_pLinkedCutsceneSequence->scriptCursor =
                    g_pLinkedCutsceneSequence->scriptStart;
                break;
            case 3:
                scriptBytes++;
                break;
            }
            break;
        case 0xa5:
            RunCutsceneWipeTransition(
                &g_15_stSceneFlicScratchViewport,
                &g_stSecondaryViewBufferBlock,
                (int)(signed char)*scriptBytes++,
                PopCutsceneScriptValue(&topOperand, scriptValueStack + 10));
            break;
        case 0xa6:
            value = *scriptBytes++;
            g_3_nCutsceneTextIdx = *(short *)scriptBytes;
            scriptBytes += 2;
            g_4_pszFilmSourceText =
                g_pCutsceneTextResources->entries[
                    g_3_nCutsceneTextIdx];
            if (g_bCutsceneSkipFrame == 0) {
                UpdateMap(g_4_pszFilmSourceText,
                          value);
            }
            break;
        case 0xb6:
            ReleasePacketSlot(&g_pCutsceneCockpitPacket);
            break;
        case 0xb5:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            LoadCutsceneCockpitOverlay(value);
            break;
        case 0xa7:
            value = PopCutsceneScriptValue(&topOperand, scriptValueStack + 10);
            secondaryOperand = PopCutsceneScriptValue(
                &topOperand, scriptValueStack + 10);
            if (g_bCutsceneSkipFrame == 0)
                DrawCutsceneCockpitOverlay(secondaryOperand, value);
            break;
        default:
            FatalCutsceneError("Illegal OpCode: 0x%2x (%d)\n",
                               (unsigned char)opcode, opcode);
            break;
        }
    }
    return 0;
}

/* Keep this callee declaration here: CL 4.1's symbol order controls the
 * following function's comparison operands. */
void *LoadCachedCutsceneResource(CutsceneResourceTable *resources,
                                 short index, int);

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

    if (g_cCutsceneVideoMode == 13) {
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
        if (g_nMusicDriverMode == 2)
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
    long size;
    char filename[132];
    char *packedFilename;
    short sectionIndex;
    void *packet;

    packet = resources->loadedPackets[index];
    retryMusicExtension = 0;
    if (g_pMemoryLogFile != 0 && packet != 0) {
        fprintf(g_pMemoryLogFile,
                "Found: loc: %Fp  level = %d, free = %8lu\n",
                packet, (int)g_nActiveCutsceneResourceLevel,
                GetAvailableMainMemory());
    }
    while (packet == 0) {
        if (g_nSceneFlicContext == 4 &&
            g_19_bStoredHighMemoryEnabled == 0) {
            g_nSceneFlicContext = 0x40;
            break;
        }
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
        sectionIndex = resources->sectionIndices[index];
        packet = LoadNamedPacket(filename, sectionIndex, 0,
            (unsigned short)(g_nSceneFlicContext & 0x7f),
            0, 1);
        if (packet == 0 && g_nPacketError == 8) {
            if (retryMusicExtension != 0) {
                retryMusicExtension = 0;
                RewriteCutsceneMusicExtensionForRetry(filename);
                packet = LoadNamedPacket(filename, sectionIndex, 0,
                    (unsigned short)(
                        g_nSceneFlicContext & 0x7f),
                    0, 1);
                if (packet == 0) {
                    ReportCutscenePacketAllocationFailure(
                        filename, sectionIndex, 0);
                }
            } else {
                ReportCutscenePacketAllocationFailure(
                    filename, sectionIndex, 0);
            }
        } else if (packet == 0 &&
                   (g_nSceneFlicContext & 0x40) != 0) {
            FatalCutsceneError(
                "Unsuccessful load of %s packet %d error = %d",
                filename, sectionIndex, g_nPacketError);
        }
        if (packet == 0) {
            ReportCutscenePacketAllocationFailure(
                filename, sectionIndex,
                GetNamedPacketSize(filename, sectionIndex));
        }
        if (g_pMemoryLogFile != 0) {
            size = GetNamedPacketSize(filename, sectionIndex);
            fprintf(g_pMemoryLogFile,
                    "Loaded: %12s (%03d) size %ld loc: %Fp  "
                    "level = %d, free = %8lu\n",
                    filename, sectionIndex, size, packet,
                    (int)g_nActiveCutsceneResourceLevel,
                    GetAvailableMainMemory());
        }
        g_nSceneFlicContext = 0x40;
        resources->loadedPackets[index] = packet;
        break;
    }
    return packet;
}

/* Function start: 0x433269 */
void ReleaseCutsceneSpriteShape(SceneFlicObject *sprite)
{
    SceneFlicObject *peer;
    void *frames;
    short index;

    frames = sprite->shape;
    g_8_nSavedCutsceneResourceOwnerValue =
        g_nActiveCutsceneResourceLevel;
    if (frames != 0) {
        for (index = 0; index < 0x80; index++) {
            peer = g_apSceneObjects[index];
            if (peer != 0 && peer->shape == frames && peer != sprite) {
                goto shape_is_shared;
            }
        }
    }
    g_nActiveCutsceneResourceLevel = sprite->owner;
    ReleaseActiveCutscenePacket(g_pCutsceneShapeResources, frames);
shape_is_shared:
    g_nActiveCutsceneResourceLevel =
        g_8_nSavedCutsceneResourceOwnerValue;
    sprite->shape = 0;
}

/* Function start: 0x433328 */
void ReleaseLoadedCutsceneResource(CutsceneResourceTable *resources,
                                   short index)
{
    SceneFlicObject *flic;
    short scan;
    void *resource;

    resource = resources->loadedPackets[index];
    ReleasePacketSlot(&resources->loadedPackets[index]);
    if (g_pMemoryLogFile != 0 && resource != 0)
        fprintf(g_pMemoryLogFile,
                "--- Dumped: %Fp  level = %d free = %8lu\n", resource,
                (int)g_nActiveCutsceneResourceLevel,
                GetAvailableMainMemory());
    if (resource != 0) {
        for (scan = 0; scan < 0x80; scan++) {
            flic = g_apSceneObjects[scan];
            if (flic != 0 && flic->shape == resource)
                flic->shape = 0;
        }
    }
}

/* Function start: 0x4333F8 */
int MapCutsceneSoundEffectNumber(int soundIndex)
{
    return soundIndex + 1;
}

CutsceneSoundEffect *g_pCutsceneSoundEffects = 0; /* 0x00499c28 */
signed char g_cCutsceneSoundVolume = 0x40; /* 0x00499c2c */
void *g_pCutsceneFxPacket = 0; /* 0x00499c30 */
CutsceneMusicNode *g_pCutsceneMusicPackets = 0; /* 0x00499c34 */
SceneFlicObject **g_apSceneObjects = 0; /* 0x00499c38 */
CutscenePlane **g_apCutscenePlanes = 0; /* 0x00499c3c */
CutsceneSequence **g_apCutsceneSequences = 0; /* 0x00499c40 */
CutsceneScene **g_apCutsceneScenes = 0; /* 0x00499c44 */
signed char g_cCutsceneVideoMode = 0x13; /* 0x00499c48 */
signed char g_bCutsceneViewportPreallocated = 0; /* 0x00499c4c */
short g_nSceneFlicContext = 0; /* 0x00499c50 */
signed char g_bCutsceneSkipFrame = 0; /* 0x00499c54 */
signed char g_bCutsceneSkipAll = 0; /* 0x00499c58 */
void (*g_apCutsceneSpriteHandlers[1])(SceneFlicObject *sprite) = {
    AnimateCutsceneSpeakerMouth
};
signed char g_bCutsceneDrawingEnabled = 0; /* 0x00499c60 */
SceneFlicObject *g_pLinkedCutsceneSprite = 0; /* 0x00499c64 */
CutscenePlane *g_pLinkedCutscenePlane = 0; /* 0x00499c68 */
CutsceneSequence *g_pLinkedCutsceneSequence = 0; /* 0x00499c6c */
int g_nUnusedCutsceneWord1 = 0; /* 0x00499c70 */
int g_nUnusedCutsceneWord2 = 0; /* 0x00499c74 */
SceneFlicObject *g_pCurrentCutsceneSprite = 0; /* 0x00499c78 */
CutscenePlane *g_pCurrentCutscenePlane = 0; /* 0x00499c7c */
CutsceneSequence *g_pCurrentCutsceneSequence = 0; /* 0x00499c80 */
CutsceneScene *g_pCurrentCutsceneScene = 0; /* 0x00499c84 */
short g_nCutsceneFrameTick = 0; /* 0x00499c88 */
unsigned short g_nCutsceneFrameDelay = 0; /* 0x00499c8c */
int g_nNextCutsceneFrameClock = 0; /* 0x00499c90 */
CampaignGlobals *g_pCampaignGlobals = 0; /* 0x00499c94 */
unsigned char g_abCutscenePaletteTranslation[256] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    7, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 6, 6, 6, 6, 6,
    11, 11, 11, 9, 9, 9, 9, 9, 9, 9, 9, 9, 1, 1, 1, 1,
    15, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    14, 14, 14, 14, 14, 14, 14, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    12, 12, 12, 12, 12, 12, 12, 12, 14, 10, 10, 10, 10, 10, 2, 2,
    14, 14, 14, 14, 14, 14, 14, 14, 6, 6, 6, 6, 6, 6, 6, 6,
    12, 12, 12, 12, 12, 12, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    10, 10, 10, 10, 10, 10, 10, 10, 4, 4, 4, 4, 4, 4, 4, 4,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 9, 9, 9, 9, 1,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8,
    12, 12, 12, 12, 12, 12, 12, 12, 4, 4, 4, 4, 4, 4, 4, 4,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 2, 2, 2, 2, 2, 2,
    7, 7, 7, 7, 8, 8, 8, 8, 12, 12, 12, 12, 4, 4, 4, 4,
    10, 10, 10, 10, 2, 2, 2, 2, 13, 13, 13, 13, 13, 13, 13, 13
}; /* 0x00499c98 */
short g_nCutsceneResourceLevel = 0; /* 0x00499d98 */
short g_nActiveCutsceneResourceLevel = 0; /* 0x00499d9c */
signed char g_bCutsceneTextRestorePending = 0; /* 0x00499da0 */
char *g_pszCurrentCutsceneText = 0; /* 0x00499da4 */
FILE *g_pMemoryLogFile = 0; /* 0x00499da8 */
signed char g_acCutsceneMouthFrames[128] = {
    10, 13, 14, 12, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    3, 5, 5, 1, 5, 9, 5, 5, 1, 2, 5, -1, -1, -1, -1, -1,
    -1, 1, 0, 5, 5, 1, 9, 5, 3, 2, 5, 5, 8, 0, 5, 3,
    0, 6, 5, 5, 5, 4, 9, 6, 5, 5, 5, 1, -1, -1, -1, -1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
}; /* 0x00499db0 */
signed char g_acCutsceneMouthDurations[128] = {
    40, 3, 3, 50, 60, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,
    1, 1, 1, 1, 2, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
}; /* 0x00499e30 */
char *g_pszCutsceneSpeechCursor = 0; /* 0x00499eb0 */
signed char g_cCutsceneSpeechSpeed = 1; /* 0x00499eb4 */
signed char g_bCutsceneSpeechActive = 0; /* 0x00499eb8 */
static char g_szLastCutsceneWorkBuffer[16] = "@@@@@@@@@@"; /* 0x00499ec0 */
static char g_szLastCutscenePrintBuffer[12] = "@@@@@@@@@@"; /* 0x00499ed0 */
signed char g_bCutsceneTextBoundsOverride = 0; /* 0x00499edc */
ShortRect g_stCutsceneTextBounds = { 0, 0, 319, 199 }; /* 0x00499ee0 */
signed char g_bCutsceneDebugOverlayEnabled = 0; /* 0x00499ee8 */
unsigned short g_wCutsceneDebugFrameNumber = 0; /* 0x00499eec */
CutscenePlane *g_pCutsceneSpriteParentPlane = 0; /* 0x00499ef0 */
int g_nUnusedCutsceneWord3 = 0; /* 0x00499ef4 */
PilotNameVariables g_stPilotNameVariables = {
    "CALLSIGN", "PCNAME", "FIRSTNAME"
}; /* 0x00499ef8 */
signed char g_cCutsceneTextStyle = 0; /* 0x00499f40 */

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
signed char g_bSceneCockpitLoaded; /* 0x005d2d66 */
short g_8_nSavedCutsceneResourceOwnerValue; /* 0x005d2d68 */
short g_1_asCutsceneSpeechChannelsTable[8]; /* 0x005d2d70 */
char g_13_nFreeGameflowWord1; /* 0x005d2d80 */ /* never referenced */
Viewport g_10_stCutsceneStringViewportBlock; /* 0x005d2d90 */
CutsceneResourceTable *g_1_pActiveSceneFileResourcesRef; /* 0x005d2da4 */
Viewport g_9_stCutsceneTextBackingPort; /* 0x005d2db0 */
unsigned int g_4_dwSceneMusicMode; /* 0x005d2dc4 */
char *g_27_pszCutsceneFormattedText; /* 0x005d2dc8 */
char *g_4_pszFilmSourceText; /* 0x005d2dcc */
short g_19_asCutsceneSpeechSectionsList[8]; /* 0x005d2dd0 */
signed char g_bMovieFramePresented; /* 0x005d2de0 */
TextContext g_6_stMovieDebugTextCtx; /* 0x005d2df0 */
SceneFlicCacheEntry g_aSceneFlicCopyTable[15]; /* 0x005d2e10 */
unsigned int g_28_nCutsceneInitialAvailableMemoryWord; /* 0x005d2e88 */
short g_2_nCutsceneForkOffsetNumber; /* 0x005d2e8c */
SceneFlicObject *g_7_pWatchedSceneSpriteA; /* 0x005d2e90 */
SceneFlicObject *g_11_pWatchedSceneSpriteB; /* 0x005d2e94 */
unsigned char *g_26_pFilmRgbPacket; /* 0x005d2e98 */
short g_4_nCutsceneResourceScratchValue; /* 0x005d2e9c */
int g_39_nIdleGameflowWord1; /* 0x005d2ea0 */ /* never referenced */
short g_3_nCutsceneTextIdx; /* 0x005d2ea4 */
unsigned int g_17_nSceneInitialLargestBlockWord; /* 0x005d2ea8 */
Viewport g_15_stSceneFlicScratchViewport; /* 0x005d2eb0 */
short g_nEarlierGlobalCount; /* 0x005d2ec4 */
int g_47_nCutsceneMemDelta; /* 0x005d2ec8 */
char *g_4_pszFilmWorkScratch; /* 0x005d2ecc */
short g_2_bMovieStringAdvanceState; /* 0x005d2ed0 */
char *g_12_apszMovieSpeechFiles[8]; /* 0x005d2ee0 */
short *g_2_pCutsceneLocalsTemp; /* 0x005d2f00 */
char g_10_nSpareGameflowWord; /* 0x005d2f04 */ /* never referenced */
InputEventState *g_19_pFilmInputFlag; /* 0x005d2f08 */
short g_117_nCutsceneFormatValueWord; /* 0x005d2f0c */
char *g_116_pszFilmPrintStore; /* 0x005d2f10 */
int g_17_nIdleGameflowWord2[5]; /* 0x005d2f20 */ /* never referenced */
TextContext g_2_stCutsceneMessageCtxInfo; /* 0x005d2f40 */
TextContext g_16_stCutsceneDrawingTextSettingsInfo; /* 0x005d2f60 */
void *g_3_apSceneSpeechPacketsList[8]; /* 0x005d2f80 */
short g_1_nCutsceneFontOwnerNumber; /* 0x005d2fa0 */
char g_68_nUnusedGameflowNumberValue; /* 0x005d2fa2 */ /* never referenced */
short g_8_bUserDestroyedFlag; /* 0x005d2fa4 */
short g_4_nFreeGameflowWord2; /* 0x005d2fa6 */ /* never referenced */
signed char g_36_cPilotKillCountValue; /* 0x005d2fa8 */
