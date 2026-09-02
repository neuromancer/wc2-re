/*
 *  Full-screen presentation screens.
 *
 *  Address range 0x437000-0x43cfff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: unbroken run of Blit* screens plus ShowGetReady/Victory/GameOver.
 */
#include "game.h"

#pragma function(memcmp)
#pragma function(memcpy)
#pragma function(strcpy)
#pragma function(strlen)

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

/* Function start: 0x42BDDB */
signed char HasCutsceneMusicNode(CutsceneMusicNode *node)
{
    return node != 0;
}

/* Function start: 0x42BF0C */
void LogChangedCutsceneWorkBuffers(void)
{
    if (memcmp(g_szLastCutsceneWorkBuffer_00499ec0,
               g_pszCutsceneWorkBuffer_005d2ecc,
               strlen(g_szLastCutsceneWorkBuffer_00499ec0)) == 0) {
        if (memcmp(g_szLastCutscenePrintBuffer_00499ed0,
                   g_pszCutscenePrintBuffer_005d2f10,
                   strlen(g_szLastCutscenePrintBuffer_00499ed0)) == 0) {
            return;
        }
    }
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

/* WC2's screens.c declares the memory-status flag one byte wide where the
 * rest of the game declares it as a short.  Only the load and store width
 * differ; the flag itself is 0 or 1. */
#define g_cShowMemoryStatus \
    (*(signed char *)&g_nShowMemoryStatus_0049d784)

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
    short damage;
    short x;
    short y;

    if (g_pCutsceneCockpitPacket_00499c04 != 0) {
        for (damage = 0; damage < 4; damage++) {
            if (g_asCockpitDamageState_005d1ee8[damage] == 1) {
                x = g_aCockpitDamagePositions_0049ae98[damage].x;
                y = (short)(
                    g_aCockpitDamagePositions_0049ae98[damage].y +
                    verticalOffset);
                DrawSpriteDefault(&g_stSecondaryViewBuffer_005d2c90,
                                  x, y,
                                  g_pCutsceneCockpitPacket_00499c04,
                                  damage);
            }
        }
    }
    (void)unused;
}

/* Function start: 0x42C43D */
void RunCutsceneWipeTransition(Viewport *destination, Viewport *source,
                               int wipeType, short duration)
{
    void *workspace;
    signed char complete;

    complete = 0;
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
                    while (complete == 0) {
                        complete = AdvanceViewportWipe(workspace);
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
    while (FindQueuedInputEvent(2) == 0) {
        while (ServiceInputDevices(-1) == 0)
            ;
        ServiceCutsceneRuntimeHook();
        if (FindQueuedInputEvent(2) != 0 ||
            FindQueuedInputEvent(5) != 0)
            break;
    }
}

/* Function start: 0x42C607 */
CutsceneResourceTable *FindActiveCutsceneFileResources(
    CutsceneResourceTable *resources)
{
    while (resources != 0) {
        if (resources->owner == g_nActiveCutsceneResourceLevel_00499d9c)
            return resources;
        resources = resources->next;
    }
    FatalCutsceneError("Missing cutscene file resource level");
    return 0;
}

/* Function start: 0x42C659 */
CutsceneObjectResourceList *FindActiveCutsceneObjectResources(
    CutsceneObjectResourceList *resources)
{
    while (resources != 0) {
        if (resources->owner == g_nActiveCutsceneResourceLevel_00499d9c)
            return resources;
        resources = resources->next;
    }
    FatalCutsceneError("Missing cutscene object resource level");
    return 0;
}

/* Function start: 0x42C6AC */
short RemoveCutsceneMemberIndex(short count, unsigned char *indices,
                                signed char index)
{
    short member;

    for (member = 0; member < count; member++) {
        if (indices[member] == (unsigned char)index) {
            DosMemcpy(indices, indices + member + 1, (size_t)(15 - member));
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
    va_end(arguments);
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

    if (g_cCutsceneVideoMode_00499c48 == 0x13 ||
        g_cCutsceneVideoMode_00499c48 == 0x0d) {
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
}

/* Function start: 0x42C8E1 */
void ConsumeCutscenePalettePacket(short firstColour, short count,
                                  signed char releasePacket)
{
    unsigned char *nextForm;
    unsigned char *chunk;
    unsigned int chunkSize;
    unsigned int formType;
    signed char malformed;

    malformed = 0;
    if (g_pCutscenePalettePacket_005d2e98 != 0) {
        formType = ReadNextSceneForm(
            (unsigned char **)&g_pCutscenePalettePacket_005d2e98,
            &nextForm);
        if (formType == 0x204c4150) {
            chunk = g_pCutscenePalettePacket_005d2e98;
            g_pCutscenePalettePacket_005d2e98 = chunk + 4;
            if (*(unsigned int *)chunk == 0x50414d43) {
                chunkSize = *(unsigned int *)
                    g_pCutscenePalettePacket_005d2e98;
                g_pCutscenePalettePacket_005d2e98 =
                    (unsigned char *)g_pCutscenePalettePacket_005d2e98 + 4;
                SwapSceneChunkSizeEndian((int *)&chunkSize);
                if (chunkSize / 3 != 0x100)
                    malformed++;
            } else {
                malformed++;
            }
            if (malformed != 0)
                FatalCutsceneError("Malformed cutscene palette");
        }
        if (releasePacket != 0)
            ReleasePacketSlot(&g_pCutscenePalettePacket_005d2e98);
    }
}

/* Function start: 0x42CB08 */
void ReleaseCutsceneSoundEffects(short resourceIndex)
{
    CutsceneSoundEffect *previous;
    CutsceneSoundEffect *effect;
    CutsceneSoundEffect *next;

    previous = 0;
    effect = g_pCutsceneSoundEffects_00499c28;
    if (g_nMusicDriverMode_0049be8c == 1 ||
        g_nMusicDriverMode_0049be8c == 2) {
        if (resourceIndex == -1) {
            while (effect != 0) {
                ((void (__cdecl *)(void *, int))FlushSoundEffect)(
                    effect->sound, 0);
                next = effect->next;
                free(effect);
                effect = next;
            }
            g_pCutsceneSoundEffects_00499c28 = 0;
        } else {
            while (effect != 0) {
                if (effect->resourceIndex == resourceIndex) {
                    ((void (__cdecl *)(void *, int))FlushSoundEffect)(
                        effect->sound, 0);
                    if (previous == 0)
                        g_pCutsceneSoundEffects_00499c28 = effect->next;
                    else
                        previous->next = effect->next;
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
}

/* Function start: 0x42CC42 */
void LoadCutsceneSpeechSlot(short resourceIndex, short slot)
{
    CutsceneResourceTable *resources;
    int packetSize;

    if (g_bSpeechCacheEnabled_005c8de8 != 0 &&
        g_bCutsceneViewportPreallocated_00499c4c == 0 &&
        g_bCutsceneSkipAll_00499c58 == 0) {
        resources = FindActiveCutsceneFileResources(
            g_pCutsceneSpeechResources_004928a4);
        while (g_wSpeechCacheState_0049bb60 != 0)
            PumpWindowMessages(0);
        ReleasePacketSlot(&g_apCutsceneSpeechPackets_005d2f80[slot]);
        g_asCutsceneSpeechChannels_005d2d70[slot] = 0;
        g_apszCutsceneSpeechFiles_005d2ee0[slot] =
            GetPackedStringByIndex(
                resources, resources->filenameIndices[resourceIndex]);
        g_asCutsceneSpeechSections_005d2dd0[slot] =
            resources->sectionIndices[resourceIndex];
        packetSize = (int)GetNamedPacketSize(
            g_apszCutsceneSpeechFiles_005d2ee0[slot],
            g_asCutsceneSpeechSections_005d2dd0[slot]);
        if (packetSize < 1) {
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
#pragma function(memcpy)

/* Function start: 0x42BB70 */
void AnimateCutsceneSpeakerMouth(SceneFlicObject *sprite)
{
    signed char duration;
    signed char frame;
    signed char character;
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
    if (g_wSpeechCacheState_0049bb60 != 0 ||
        g_bCutsceneSpeechActive_00499eb8 != 0) {
        if (g_wSpeechCacheState_0049bb60 == 0) {
            g_bCutsceneSpeechActive_00499eb8 =
                (signed char)(g_bCutsceneTextAdvance_005d2ed0 = 0);
            sprite->currentFrame = 11;
            g_pszCutsceneSpeechCursor_00499eb0 = 0;
            return;
        }
        g_bCutsceneSpeechActive_00499eb8 = 1;
    }
    if (g_wSpeechCacheState_0049bb60 != 0)
        speechSpeed = 0;
    sprite->waitStart = g_nInputClock_005c84a8;
    if (g_pszCutsceneSpeechCursor_00499eb0 == 0)
        g_pszCutsceneSpeechCursor_00499eb0 = g_pszCurrentCutsceneText_00499da4;
    character = (signed char)toupper(*g_pszCutsceneSpeechCursor_00499eb0);
    if (character < 0) {
        character = (signed char)(character + 0x80);
        frame = g_acCutsceneMouthFrames_00499db0[character];
        duration = g_acCutsceneMouthDurations_00499e30[character];
    } else if (character < ' ') {
        if (character == 0) {
            g_bCutsceneSpeechActive_00499eb8 =
                (signed char)(g_bCutsceneTextAdvance_005d2ed0 = 0);
            g_pszCutsceneSpeechCursor_00499eb0 = 0;
        }
        frame = -1;
    } else if (character == 'T' &&
               toupper(*g_pszCutsceneSpeechCursor_00499eb0) == 'H') {
        frame = 8;
        duration = 3;
        g_pszCutsceneSpeechCursor_00499eb0++;
    } else {
        frame = g_acCutsceneMouthFrames_00499db0[character];
        duration = g_acCutsceneMouthDurations_00499e30[character];
    }
    if (frame != -1) {
        sprite->currentFrame = frame;
        sprite->waitTicks = (short)(speechSpeed * duration);
#ifdef SDL_PORT
        /* Keep text-only lines readable when no speech controls the pace. */
        if (sprite->waitTicks < CUTSCENE_MOUTH_MIN_TICKS)
            sprite->waitTicks = CUTSCENE_MOUTH_MIN_TICKS;
        SdlTracef("[mouth] ch=%c speed=%d dur=%d wait=%d clock=%d "
                     "period=%ldms cache=%d speech=%d\n",
                     (char)(character >= ' ' ? character : '.'),
                     (int)speechSpeed, (int)duration,
                     (int)sprite->waitTicks, (int)g_nInputClock_005c84a8,
                     g_nFramePeriodMilliseconds_005c343c,
                     (int)g_wSpeechCacheState_0049bb60,
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

/* Function start: 0x42CFDB */
void ReleaseCutsceneMusicNodes(CutsceneMusicNode **head,
                               short resourceIndex)
{
    CutsceneMusicNode *node;
    CutsceneMusicNode *previous;

    node = *head;
    if ((g_nMusicDriverMode_0049be8c == 1 ||
         g_nMusicDriverMode_0049be8c == 2) &&
        HasCutsceneMusicNode(node) != 0) {
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
            g_bSpeechCacheEnabled_005c8de8 == 0)
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
    short savedMemoryStatus;

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
    savedMemoryStatus = g_cShowMemoryStatus;
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
    if (g_pSceneMusicPacket_00499c08 != 0)
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
    g_cShowMemoryStatus = (signed char)savedMemoryStatus;
}

/* Function start: 0x42D81C */
void ExecuteCutsceneScene(CutsceneScene *scene)
{
    SceneFlicObject *savedSprite;
    CutscenePlane *savedPlane;
    short savedOwner;

    savedOwner = g_nActiveCutsceneResourceLevel_00499d9c;
    savedPlane = g_pCurrentCutscenePlane_00499c7c;
    savedSprite = g_pCurrentCutsceneSprite_00499c78;
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
    SceneFlicObject *destinationSprite;
    SceneFlicObject *sourceSprite;

    destinationSprite = g_apSceneObjects_00499c38[
        FindActiveCutsceneObjectResources(
            g_pCutsceneSpriteResources_0049288c)
            ->scriptSymbolIndices[destination]];
    sourceSprite = g_apSceneObjects_00499c38[
        FindActiveCutsceneObjectResources(
            g_pCutsceneSpriteResources_0049288c)
            ->scriptSymbolIndices[source]];
    ReleaseCutsceneSpriteShape(sourceSprite);
    sourceSprite->drawType = destinationSprite->drawType;
    sourceSprite->shape = destinationSprite->shape;
    sourceSprite->baseFrame = destinationSprite->baseFrame;
    sourceSprite->finalFrame = destinationSprite->finalFrame;
    sourceSprite->currentFrame = destinationSprite->currentFrame;
}

/* Function start: 0x42D98C */
void LinkCutsceneSpriteScript(unsigned char destination,
                              unsigned char source)
{
    SceneFlicObject *destinationSprite;
    SceneFlicObject *sourceSprite;

    destinationSprite = g_apSceneObjects_00499c38[
        FindActiveCutsceneObjectResources(
            g_pCutsceneSpriteResources_0049288c)
            ->scriptSymbolIndices[destination]];
    sourceSprite = g_apSceneObjects_00499c38[
        FindActiveCutsceneObjectResources(
            g_pCutsceneSpriteResources_0049288c)
            ->scriptSymbolIndices[source]];
    sourceSprite->linkedOwner = sourceSprite->owner;
    sourceSprite->linkedScript = sourceSprite->scriptStart;
    sourceSprite->scriptCursor = destinationSprite->scriptStart;
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
    if ((g_bCutsceneDrawingEnabled_00499c60 != 0 ||
         g_bCutsceneSkipFrame_00499c54 == 0) &&
        plane->active != 0 && plane->visible != 0 &&
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
    char *output;
    char character;
    unsigned int number[4];

    output = destination;
    if (destination == 0)
        return;
    DosMemset(destination, 0x100, 0);
    while ((character = *source++) != 0) {
        if ((unsigned char)character == 0x40) {
            itoa(g_nCutsceneFormatValue_005d2f0c, (char *)number, 10);
            output = CopyStringAndReturnEnd(output, (char *)number);
        } else if ((unsigned char)character >= 0x85) {
            switch ((unsigned char)character) {
            case 0x85:
                output = CopyStringAndReturnEnd(
                    output, g_szPilotLastName_00499f10);
                break;
            case 0x86:
                output = CopyStringAndReturnEnd(
                    output, g_szPilotCallsign_00499ef8);
                break;
            case 0x87:
                itoa(*((short *)g_pCampaignGlobals_00499c94 + 54),
                     (char *)number, 10);
                output = CopyStringAndReturnEnd(output, (char *)number);
                break;
            case 0x88:
                itoa(*((short *)g_pCampaignGlobals_00499c94 + 55),
                     (char *)number, 10);
                output = CopyStringAndReturnEnd(output, (char *)number);
                break;
            case 0x89:
                output = CopyStringAndReturnEnd(
                    output, g_szPilotFirstName_00499f28);
                break;
            default:
                output++;
                break;
            }
        } else {
            *output++ = character;
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
                    if (g_wSpeechCacheState_0049bb60 == 0 &&
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
            if (g_wSpeechCacheState_0049bb60 == 0 &&
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
        case 0xb0:
            while (g_wSpeechCacheState_0049bb60 != 0)
                PumpWindowMessages(0);
            value = PopCutsceneScriptValue(&stack, stackStorage + 10);
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

/* Function start: 0x434177 */
unsigned short HasSavedPilotCampaign(void)
{
    int campaignBytes;
    struct _finddata_t findData;
    short occupied;
    short findHandle;
    short file;

    LoadTemporaryCampaignGlobals();
    campaignBytes = (unsigned short)g_pCampaignGlobals_00499c94->wordCount * 2;
    ReleasePacketSlot((void **)&g_pCampaignGlobals_00499c94);
    findHandle = (short)_findfirst("savegame.wc2", &findData);
    if (findHandle == -1 ||
        campaignBytes * 9 + 0x4b6 != findData.size)
        return 0;
    _findclose((long)findHandle);
    file = (short)_open("savegame.wc2", 0x8000);
    if (file < 0)
        return 0;
    if (_lseek((int)file, campaignBytes * 8 + 0x430, 0) !=
        campaignBytes * 8 + 0x430) {
        _close((int)file);
        return 0;
    }
    if (_read((int)file, &occupied, 2) != 2) {
        _close((int)file);
        return 0;
    }
    _close((int)file);
    if (occupied == 0)
        return 0;
    return 1;
}

/* Function start: 0x4021A7 */
void RecordCannedSceneBriefingCharacter(signed char character,
                                        signed char pose,
                                        short animationFrame)
{
    CannedSceneBriefingCharacterRecord *record;

    if (g_bHighMemoryBuffersReady_005d2ad8 != 0) {
        if (g_nCannedSceneMode_0049021c == 0) {
            g_dwHighMemoryParagraph_005d3fb4 =
                IdentityDword((DwordPtr)g_pHighMemoryBlockA_004901f8);
            record = (CannedSceneBriefingCharacterRecord *)(
                (unsigned int)(unsigned short)
                    g_nCannedSceneWriteIndex_005d3fa8 +
                g_dwHighMemoryParagraph_005d3fb4);
            record->opcode = 1;
            record->frame = g_nSpaceFrame_00493134;
            record->character = character;
            record->pose = pose;
            record->animationFrame = animationFrame;
            record->endMarker = -1;
            record->nextOffset = 0x29d;
            g_nCannedSceneWriteIndex_005d3fa8 = (short)(
                (unsigned short)g_nCannedSceneWriteIndex_005d3fa8 + 0xa);
            CheckCannedSceneBufferCapacity();
        }
    }
}

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

#pragma function(memset)

/* Function start: 0x433410 */
void PanToScreen(Viewport *source, Viewport *destination)
{
    unsigned char *indices;
    short activeCount;
    short index;
    unsigned short target[3];
    short *transitionPalette;
    short *originalPalette;

    if (g_nSpacePaletteFadeMode_004901e8 == 0x13) {
        indices = AllocateTaggedMemory(256, 0);
        if (indices == 0)
            return;
        memset(indices, 0, 256);
        activeCount = CollectActivePaletteIndices(source, indices, 256);
        originalPalette = AllocateTaggedMemory(
            (unsigned int)(activeCount * 6), 0);
        transitionPalette = AllocateTaggedMemory(
            (unsigned int)(activeCount * 6), 0);
        if (originalPalette == 0 || transitionPalette == 0) {
            ReleasePacketHandle(indices);
            if (originalPalette != 0)
                ReleasePacketHandle(originalPalette);
            if (transitionPalette != 0)
                ReleasePacketHandle(transitionPalette);
            return;
        }
        memset(originalPalette, 0,
               (unsigned int)(activeCount * 6));
        memset(transitionPalette, 0,
               (unsigned int)(activeCount * 6));
        GetPaletteEntry(
            (short)GetViewportPixel(destination,
                                    destination->left,
                                    destination->top),
            target);
        WaitForVerticalBlankThunk();
        for (index = 0; index < activeCount; index++) {
            GetPaletteEntry(
                (short)indices[index],
                (unsigned short *)&originalPalette[index * 3]);
            SetPaletteEntry((short)indices[index], (short *)target);
            *(PaletteRgb *)&transitionPalette[index * 3] =
                *(PaletteRgb *)target;
        }
        CopyViewportContents(source, destination);
        while (StepPaletteTransition(
                   transitionPalette, originalPalette,
                   (short)(activeCount * 3)) != 0) {
            WaitForVerticalBlankThunk();
            for (index = 0; index < activeCount; index++) {
                SetPaletteEntry(
                    (short)indices[index],
                    &transitionPalette[index * 3]);
            }
        }
        ReleasePacketHandle(transitionPalette);
        ReleasePacketHandle(originalPalette);
        ReleasePacketHandle(indices);
    } else {
        CopyViewportContents(source, destination);
    }
}

#pragma intrinsic(memset)

/* Function start: 0x4697A0 (Mac symbol: death_sequence) */
void death_sequence(void)
{
    signed char frame;
    short object;
    void *cockpitBackground;
    void *deathShape;

    deathShape = 0;
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
    for (object = 0; object < 0x46; object++) {
        if (g_asObjectType_00495298[object] == OBJECT_DATA_EJECTED_PILOT)
            remove_object(object);
    }
    g_nResourcePaletteMode_005c57e6 = 1;
    StopMusicUnlessSuppressed();
    if (g_nMemoryConfiguration_005c8dc8 == 1)
        SceneLeaveHook(0x20);
    spacetrack(0x20, 2, 1);
    deathShape = FetchDiskPacketRetrying("pilotanm.vga", 0, 0);
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
                              deathShape, frame);
            dump_buffer_to_screen();
            clear_view_buffer();
            ServiceInputDevices(-1);
            if (g_bSceneEscapeRequested_0049d4b0 == 1)
                break;
        }
        free_view_buffer();
        FreePacketAndClear(&cockpitBackground, 0);
        FreePacketAndClear(&deathShape, 0);
        ServiceInputDevices(-1);
        if (g_bSceneEscapeRequested_0049d4b0 == 1)
            break;
        g_nArcadeState_0049d75c = 0;
        load_all_slots();
        g_nResourcePaletteMode_005c57e6 = 0;
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

#ifndef SDL_PORT

/* Function start: 0x40243C */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Preserve the explicit ES setup used by the hand-written driver glue. */
__declspec(naked) char *CopyRasterDriverName(void *const *callbacks)
{
#ifdef _MSC_VER
#include "screens_copy_raster_driver_name.inc"
#else
    return 0;
#endif
}

/* Function start: 0x402465 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Preserve the explicit ES setup used by the hand-written driver glue. */
__declspec(naked) void InstallRasterDriverCallbacks(
    void *const *callbacks)
{
#ifdef _MSC_VER
#include "screens_install_raster_driver_callbacks.inc"
#else
    return;
#endif
}

/* Function start: 0x402484 */
/* The explicit segment setup and symmetric read primitive at 0x439D63 show
 * that this raster-library pixel writer was hand-written assembly. */
__declspec(naked) unsigned int SetRasterClipPixel(
    RasterClip *clip, int x, int y, int colour)
{
    __asm {
        push ebp
        mov ebp, esp
        add esp, -20h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp + 8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 4]
        inc eax
        mov dword ptr [ebp - 18h], eax
        jle write_invalid_surface
        mov eax, dword ptr [ebx + 8]
        inc eax
        mov ecx, eax
        jle write_invalid_surface
        mov eax, dword ptr [esi + 4]
        mov dword ptr [ebp - 1ch], eax
        cmp eax, 0
        jg write_left_clipped
        mov eax, 0
write_left_clipped:
        mov dword ptr [ebp - 4], eax
        mov eax, dword ptr [esi + 8]
        mov dword ptr [ebp - 20h], eax
        cmp eax, 0
        jg write_top_clipped
        mov eax, 0
write_top_clipped:
        mov dword ptr [ebp - 8], eax
        mov eax, dword ptr [esi + 0ch]
        mov edx, dword ptr [ebp - 18h]
        dec edx
        cmp eax, edx
        jl write_right_clipped
        mov eax, edx
write_right_clipped:
        mov dword ptr [ebp - 0ch], eax
        mov eax, dword ptr [esi + 10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl write_bottom_clipped
        mov eax, edx
write_bottom_clipped:
        mov dword ptr [ebp - 10h], eax
        mov eax, dword ptr [ebp - 0ch]
        cmp eax, dword ptr [ebp - 4]
        jl write_invalid_clip
        mov eax, dword ptr [ebp - 10h]
        cmp eax, dword ptr [ebp - 8]
        jl write_invalid_clip
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp - 14h], eax
        jmp write_point
write_invalid_surface:
        mov eax, 0ffffffffh
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
write_invalid_clip:
        mov eax, 0fffffffeh
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
write_point:
        mov ecx, dword ptr [ebp + 0ch]
        mov ebx, dword ptr [ebp + 10h]
        add ecx, dword ptr [ebp - 1ch]
        add ebx, dword ptr [ebp - 20h]
        cmp ecx, dword ptr [ebp - 4]
        jl write_point_outside
        cmp ecx, dword ptr [ebp - 0ch]
        jg write_point_outside
        cmp ebx, dword ptr [ebp - 8]
        jl write_point_outside
        cmp ebx, dword ptr [ebp - 10h]
        jg write_point_outside
        mov eax, ebx
        imul dword ptr [ebp - 18h]
        add eax, dword ptr [ebp - 14h]
        add eax, ecx
        mov ebx, eax
        xor eax, eax
        mov al, byte ptr [ebx]
        mov dl, byte ptr [ebp + 14h]
        mov byte ptr [ebx], dl
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
write_point_outside:
        mov eax, 0fffffffdh
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
    }
}

/* Function start: 0x40255F */
__declspec(naked) int ReadRasterClipPixel(RasterClip *clip, int x, int y)
{
    __asm {
        push ebp
        mov ebp, esp
        add esp, -20h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp + 8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 4]
        inc eax
        mov dword ptr [ebp - 18h], eax
        jle invalid_surface
        mov eax, dword ptr [ebx + 8]
        inc eax
        mov ecx, eax
        jle invalid_surface
        mov eax, dword ptr [esi + 4]
        mov dword ptr [ebp - 1ch], eax
        cmp eax, 0
        jg left_clipped
        mov eax, 0
left_clipped:
        mov dword ptr [ebp - 4], eax
        mov eax, dword ptr [esi + 8]
        mov dword ptr [ebp - 20h], eax
        cmp eax, 0
        jg top_clipped
        mov eax, 0
top_clipped:
        mov dword ptr [ebp - 8], eax
        mov eax, dword ptr [esi + 0ch]
        mov edx, dword ptr [ebp - 18h]
        dec edx
        cmp eax, edx
        jl right_clipped
        mov eax, edx
right_clipped:
        mov dword ptr [ebp - 0ch], eax
        mov eax, dword ptr [esi + 10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl bottom_clipped
        mov eax, edx
bottom_clipped:
        mov dword ptr [ebp - 10h], eax
        mov eax, dword ptr [ebp - 0ch]
        cmp eax, dword ptr [ebp - 4]
        jl invalid_clip
        mov eax, dword ptr [ebp - 10h]
        cmp eax, dword ptr [ebp - 8]
        jl invalid_clip
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp - 14h], eax
        jmp read_point
invalid_surface:
        mov eax, 0ffffffffh
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
invalid_clip:
        mov eax, 0fffffffeh
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
read_point:
        mov ecx, dword ptr [ebp + 0ch]
        mov ebx, dword ptr [ebp + 10h]
        add ecx, dword ptr [ebp - 1ch]
        add ebx, dword ptr [ebp - 20h]
        cmp ecx, dword ptr [ebp - 4]
        jl point_outside
        cmp ecx, dword ptr [ebp - 0ch]
        jg point_outside
        cmp ebx, dword ptr [ebp - 8]
        jl point_outside
        cmp ebx, dword ptr [ebp - 10h]
        jg point_outside
        mov eax, ebx
        imul dword ptr [ebp - 18h]
        add eax, dword ptr [ebp - 14h]
        add eax, ecx
        mov ebx, eax
        xor eax, eax
        mov al, byte ptr [ebx]
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
point_outside:
        mov eax, 0fffffffdh
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
    }
}

/* Function start: 0x402635 */
/* The segment setup, fixed-point clipping, and unrolled drawing paths identify
 * the original as hand-written raster assembly.  Preserve those paths exactly;
 * mode 0 writes a colour, mode 1 translates through a colour table, and higher
 * modes call the callback passed in the colour argument. */
#pragma optimize("", off)
__declspec(naked) unsigned int DrawClippedLine(
    RasterClip *clip, int x1, int y1, int x2, int y2, int mode, int colour)
{
#ifdef _MSC_VER
#include "screens_draw_clipped_line.inc"
#else
    return 0;
#endif
}
#pragma optimize("", on)

/* Function start: 0x403037 */
/* Original hand-written alternating-pixel rectangle fill used for
 * checkerboard shading inside a clipped raster surface. */
#pragma optimize("", off)
__declspec(naked) int FillRasterClipCheckerboard(
    RasterClip *clip, int left, int top, int right, int bottom,
    unsigned char colour)
{
    __asm {
        push ebp
        mov ebp,esp
        add esp,-0x20
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi,dword ptr [ebp + 0x8]
        mov ebx,dword ptr [esi]
        mov eax,dword ptr [ebx + 0x4]
        inc eax
        mov dword ptr [ebp - 0x18],eax
        jle checker_43a8ba
        mov eax,dword ptr [ebx + 0x8]
        inc eax
        mov ecx,eax
        jle checker_43a8ba
        mov eax,dword ptr [esi + 0x4]
        mov dword ptr [ebp - 0x1c],eax
        cmp eax,0x0
        jg checker_43a86e
        mov eax,0x0
checker_43a86e:
        mov dword ptr [ebp - 0x4],eax
        mov eax,dword ptr [esi + 0x8]
        mov dword ptr [ebp - 0x20],eax
        cmp eax,0x0
        jg checker_43a881
        mov eax,0x0
checker_43a881:
        mov dword ptr [ebp - 0x8],eax
        mov eax,dword ptr [esi + 0xc]
        mov edx,dword ptr [ebp - 0x18]
        dec edx
        cmp eax,edx
        jl checker_43a891
        mov eax,edx
checker_43a891:
        mov dword ptr [ebp - 0xc],eax
        mov eax,dword ptr [esi + 0x10]
        mov edx,ecx
        dec edx
        cmp eax,edx
        jl checker_43a8a0
        mov eax,edx
checker_43a8a0:
        mov dword ptr [ebp - 0x10],eax
        mov eax,dword ptr [ebp - 0xc]
        cmp eax,dword ptr [ebp - 0x4]
        jl checker_43a8c5
        mov eax,dword ptr [ebp - 0x10]
        cmp eax,dword ptr [ebp - 0x8]
        jl checker_43a8c5
        mov eax,dword ptr [ebx]
        mov dword ptr [ebp - 0x14],eax
        jmp checker_43a8d0
checker_43a8ba:
        mov eax,0xffffffff
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
checker_43a8c5:
        mov eax,0xfffffffe
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
checker_43a8d0:
        mov eax,dword ptr [ebp - 0x1c]
        add dword ptr [ebp + 0xc],eax
        add dword ptr [ebp + 0x14],eax
        mov eax,dword ptr [ebp - 0x20]
        add dword ptr [ebp + 0x10],eax
        add dword ptr [ebp + 0x18],eax
        mov eax,dword ptr [ebp - 0x4]
        cmp dword ptr [ebp + 0xc],eax
        jg checker_43a8ed
        mov dword ptr [ebp + 0xc],eax
checker_43a8ed:
        mov eax,dword ptr [ebp - 0x8]
        cmp dword ptr [ebp + 0x10],eax
        jg checker_43a8f8
        mov dword ptr [ebp + 0x10],eax
checker_43a8f8:
        mov eax,dword ptr [ebp - 0xc]
        cmp dword ptr [ebp + 0x14],eax
        jl checker_43a903
        mov dword ptr [ebp + 0x14],eax
checker_43a903:
        mov eax,dword ptr [ebp - 0x10]
        cmp dword ptr [ebp + 0x18],eax
        jl checker_43a90e
        mov dword ptr [ebp + 0x18],eax
checker_43a90e:
        mov ecx,dword ptr [ebp + 0x14]
        sub ecx,dword ptr [ebp + 0xc]
        jl checker_43a969
        inc ecx
        mov eax,dword ptr [ebp + 0x10]
        imul dword ptr [ebp - 0x18]
        add eax,dword ptr [ebp - 0x14]
        add eax,dword ptr [ebp + 0xc]
        mov edi,eax
        mov edx,dword ptr [ebp + 0x18]
        sub edx,dword ptr [ebp + 0x10]
        jl checker_43a969
        mov eax,dword ptr [ebp + 0x1c]
        mov esi,edi
        mov ebx,ecx
        jmp checker_43a93d
checker_43a936:
        add esi,dword ptr [ebp - 0x18]
        mov edi,esi
        mov ecx,ebx
checker_43a93d:
        push edx
        and edx,0x1
        jz checker_43a948
        pop edx
        inc edi
        dec ecx
        jmp checker_43a949
checker_43a948:
        pop edx
checker_43a949:
        mov byte ptr [edi],al
        add edi,0x2
        sub ecx,0x2
        jg checker_43a949
        dec edx
        jns checker_43a936
        xor eax,eax
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
checker_43a969:
        mov eax,0xfffffffc
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
    }
}
#pragma optimize("", on)

/* Function start: 0x403170 */
/* The segment setup, outcode construction, and REP run decoders identify
 * this as an original hand-written clipped raster routine. */
#pragma optimize("", off)
__declspec(naked) int DrawRLEImage(RasterClip *clip, unsigned char *shape,
                                    int frame, int x, int y)
{
    __asm {
        push ebp
        mov ebp,esp
        add esp,-0x50
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi,dword ptr [ebp + 0x8]
        mov ebx,dword ptr [esi]
        mov eax,dword ptr [ebx + 0x4]
        inc eax
        mov dword ptr [ebp - 0x48],eax
        jle plain_43a9f3
        mov eax,dword ptr [ebx + 0x8]
        inc eax
        mov ecx,eax
        jle plain_43a9f3
        mov eax,dword ptr [esi + 0x4]
        mov dword ptr [ebp - 0x4c],eax
        cmp eax,0x0
        jg plain_43a9a7
        mov eax,0x0
plain_43a9a7:
        mov dword ptr [ebp - 0x34],eax
        mov eax,dword ptr [esi + 0x8]
        mov dword ptr [ebp - 0x50],eax
        cmp eax,0x0
        jg plain_43a9ba
        mov eax,0x0
plain_43a9ba:
        mov dword ptr [ebp - 0x38],eax
        mov eax,dword ptr [esi + 0xc]
        mov edx,dword ptr [ebp - 0x48]
        dec edx
        cmp eax,edx
        jl plain_43a9ca
        mov eax,edx
plain_43a9ca:
        mov dword ptr [ebp - 0x3c],eax
        mov eax,dword ptr [esi + 0x10]
        mov edx,ecx
        dec edx
        cmp eax,edx
        jl plain_43a9d9
        mov eax,edx
plain_43a9d9:
        mov dword ptr [ebp - 0x40],eax
        mov eax,dword ptr [ebp - 0x3c]
        cmp eax,dword ptr [ebp - 0x34]
        jl plain_43a9fe
        mov eax,dword ptr [ebp - 0x40]
        cmp eax,dword ptr [ebp - 0x38]
        jl plain_43a9fe
        mov eax,dword ptr [ebx]
        mov dword ptr [ebp - 0x44],eax
        jmp plain_43aa09
plain_43a9f3:
        mov eax,0xffffffff
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
plain_43a9fe:
        mov eax,0xfffffffe
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
plain_43aa09:
        mov eax,dword ptr [ebp - 0x4c]
        add dword ptr [ebp + 0x14],eax
        mov eax,dword ptr [ebp - 0x50]
        add dword ptr [ebp + 0x18],eax
        mov esi,dword ptr [ebp + 0x10]
        shl esi,0x3
        add esi,0x8
        add esi,dword ptr [ebp + 0xc]
        mov esi,dword ptr [esi]
        add esi,dword ptr [ebp + 0xc]
        mov dword ptr [ebp - 0x30],esi
        mov eax,dword ptr [esi + 0x8]
        add eax,dword ptr [ebp + 0x14]
        mov dword ptr [ebp - 0xc],eax
        mov eax,dword ptr [esi + 0xc]
        add eax,dword ptr [ebp + 0x18]
        mov dword ptr [ebp - 0x10],eax
        mov eax,dword ptr [esi + 0x10]
        add eax,dword ptr [ebp + 0x14]
        mov dword ptr [ebp - 0x14],eax
        mov eax,dword ptr [esi + 0x14]
        add eax,dword ptr [ebp + 0x18]
        mov dword ptr [ebp - 0x18],eax
        add esi,0x18
        mov eax,dword ptr [ebp - 0x14]
        cmp eax,dword ptr [ebp - 0xc]
        jl plain_43ad6d
        mov eax,dword ptr [ebp - 0x18]
        cmp eax,dword ptr [ebp - 0x10]
        jl plain_43ad6d
        xor edx,edx
        mov eax,dword ptr [ebp - 0xc]
        sub eax,dword ptr [ebp - 0x34]
        shl eax,0x1
        adc dl,dl
        mov eax,dword ptr [ebp - 0x3c]
        sub eax,dword ptr [ebp - 0xc]
        shl eax,0x1
        adc dl,dl
        mov eax,dword ptr [ebp - 0x10]
        sub eax,dword ptr [ebp - 0x38]
        shl eax,0x1
        adc dl,dl
        mov eax,dword ptr [ebp - 0x40]
        sub eax,dword ptr [ebp - 0x10]
        shl eax,0x1
        adc dl,dl
        mov eax,dword ptr [ebp - 0x14]
        sub eax,dword ptr [ebp - 0x34]
        shl eax,0x1
        adc dh,dh
        mov eax,dword ptr [ebp - 0x3c]
        sub eax,dword ptr [ebp - 0x14]
        shl eax,0x1
        adc dh,dh
        mov eax,dword ptr [ebp - 0x18]
        sub eax,dword ptr [ebp - 0x38]
        shl eax,0x1
        adc dh,dh
        mov eax,dword ptr [ebp - 0x40]
        sub eax,dword ptr [ebp - 0x18]
        shl eax,0x1
        adc dh,dh
        mov dword ptr [ebp - 0x1c],edx
        test dh,dl
        jnz plain_43ad62
        or dl,dh
        jnz plain_43aaf4
        mov esi,dword ptr [ebp + 0x8]
        mov eax,dword ptr [esi + 0x4]
        sub dword ptr [ebp + 0x14],eax
        mov eax,dword ptr [esi + 0x8]
        sub dword ptr [ebp + 0x18],eax
        push dword ptr [ebp - 0x48]
        push dword ptr [ebp + 0x18]
        push dword ptr [ebp + 0x14]
        push dword ptr [ebp - 0x30]
        push dword ptr [ebp + 0x8]
        call DrawRLEImageUnclipped
        add esp,0x14
        jmp plain_43ad5a
plain_43aaf4:
        mov eax,dword ptr [ebp - 0x10]
        imul dword ptr [ebp - 0x48]
        add eax,dword ptr [ebp - 0x44]
        add eax,dword ptr [ebp - 0xc]
        mov edi,eax
        mov ecx,dword ptr [ebp - 0x10]
        mov dword ptr [ebp - 0x20],ecx
        jmp plain_43ab20
plain_43ab0a:
        movzx eax,al
        add esi,eax
        dec esi
plain_43ab10:
        inc esi
plain_43ab11:
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43ab10
        jnz plain_43ab0a
        jc plain_43ab10
        add edi,dword ptr [ebp - 0x48]
        inc ecx
plain_43ab20:
        cmp ecx,dword ptr [ebp - 0x38]
        jl plain_43ab11
        mov dword ptr [ebp - 0x24],edi
        mov dword ptr [ebp - 0x20],ecx
        mov eax,edi
        sub eax,dword ptr [ebp - 0xc]
        add eax,dword ptr [ebp - 0x34]
        mov dword ptr [ebp - 0x28],eax
        mov eax,edi
        sub eax,dword ptr [ebp - 0xc]
        add eax,dword ptr [ebp - 0x3c]
        mov dword ptr [ebp - 0x2c],eax
        jmp plain_43ad4e
plain_43ab46:
        mov eax,dword ptr [ebp - 0x20]
        cmp eax,dword ptr [ebp - 0x40]
        jg plain_43ad5a
        mov edi,dword ptr [ebp - 0x24]
        test dword ptr [ebp - 0x1c],0x8
        jnz plain_43abd3
        test dword ptr [ebp - 0x1c],0x400
        jnz plain_43ac5d
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43ab8b
        jnz plain_43abb4
        jnc plain_43abce
plain_43ab76:
        mov al,byte ptr [esi]
        inc esi
        movzx ecx,al
        add edi,ecx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43ab8b
        jnz plain_43abb4
        jc plain_43ab76
        jnc plain_43abce
plain_43ab8b:
        movzx ecx,al
plain_43ab8e:
        mov al,byte ptr [esi]
        inc esi
        push ecx
        and ecx,0x3
        rep stosb
        mov ah,al
        rol eax,0x8
        mov al,ah
        rol eax,0x8
        mov al,ah
        pop ecx
        shr ecx,0x2
        rep stosd
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43ab8b
        jnc plain_43abce
        jz plain_43ab76
plain_43abb4:
        movzx ecx,al
plain_43abb7:
        push ecx
        and ecx,0x3
        rep movsb
        pop ecx
        shr ecx,0x2
        rep movsd
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43ab8b
        jnz plain_43abb4
        jc plain_43ab76
plain_43abce:
        jmp plain_43ad3f
plain_43abd3:
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43abf3
        jnz plain_43ac24
        jnc plain_43ac58
plain_43abde:
        mov al,byte ptr [esi]
        inc esi
        movzx ecx,al
        add edi,ecx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43abf3
        jnz plain_43ac24
        jc plain_43abde
        jnc plain_43ac58
plain_43abf3:
        movzx ecx,al
        mov eax,dword ptr [ebp - 0x28]
        sub eax,edi
        cmp eax,ecx
        jge plain_43ac16
        or eax,eax
        js plain_43ac07
        add edi,eax
        sub ecx,eax
plain_43ac07:
        test dword ptr [ebp - 0x1c],0x400
        jz plain_43ab8e
        jnz plain_43ac84
plain_43ac16:
        add edi,ecx
        inc esi
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43abf3
        jnc plain_43ac58
        jz plain_43abde
plain_43ac24:
        movzx ecx,al
        mov eax,dword ptr [ebp - 0x28]
        sub eax,edi
        cmp eax,ecx
        jge plain_43ac49
        or eax,eax
        js plain_43ac3a
        add edi,eax
        sub ecx,eax
        add esi,eax
plain_43ac3a:
        test dword ptr [ebp - 0x1c],0x400
        jz plain_43abb7
        jnz plain_43acc7
plain_43ac49:
        add edi,ecx
        add esi,ecx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43abf3
        jnz plain_43ac24
        jc plain_43abde
plain_43ac58:
        jmp plain_43ad3f
plain_43ac5d:
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43ac81
        jnz plain_43acc4
        jnc plain_43acfa
plain_43ac6c:
        mov al,byte ptr [esi]
        inc esi
        movzx ecx,al
        add edi,ecx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43ac81
        jnz plain_43acc4
        jc plain_43ac6c
        jnc plain_43acfa
plain_43ac81:
        movzx ecx,al
plain_43ac84:
        cmp edi,dword ptr [ebp - 0x2c]
        jg plain_43ad1f
        mov eax,edi
        add eax,ecx
        dec eax
        sub eax,dword ptr [ebp - 0x2c]
        cdq
        not edx
        and edx,eax
        sub ecx,edx
        mov al,byte ptr [esi]
        inc esi
        push ecx
        and ecx,0x3
        rep stosb
        mov ah,al
        rol eax,0x8
        mov al,ah
        rol eax,0x8
        mov al,ah
        pop ecx
        shr ecx,0x2
        rep stosd
        add edi,edx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43ac81
        jnc plain_43acfa
        jz plain_43ac6c
plain_43acc4:
        movzx ecx,al
plain_43acc7:
        cmp edi,dword ptr [ebp - 0x2c]
        jg plain_43ad30
        mov eax,edi
        add eax,ecx
        dec eax
        sub eax,dword ptr [ebp - 0x2c]
        cdq
        not edx
        and edx,eax
        sub ecx,edx
        push ecx
        and ecx,0x3
        rep movsb
        pop ecx
        shr ecx,0x2
        rep movsd
        add edi,edx
        add esi,edx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43ac81
        jnz plain_43acc4
        jc plain_43ac6c
plain_43acfa:
        jmp plain_43ad3f
plain_43ad07:
        mov al,byte ptr [esi]
        inc esi
        movzx ecx,al
        add edi,ecx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43ad1c
        jnz plain_43ad2d
        jc plain_43ad07
        jnc plain_43ad3f
plain_43ad1c:
        movzx ecx,al
plain_43ad1f:
        add edi,ecx
        inc esi
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43ad1c
        jnc plain_43ad3f
        jz plain_43ad07
plain_43ad2d:
        movzx ecx,al
plain_43ad30:
        add edi,ecx
        add esi,ecx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja plain_43ad1c
        jnz plain_43ad2d
        jc plain_43ad07
plain_43ad3f:
        mov eax,dword ptr [ebp - 0x48]
        add dword ptr [ebp - 0x24],eax
        add dword ptr [ebp - 0x28],eax
        add dword ptr [ebp - 0x2c],eax
        inc dword ptr [ebp - 0x20]
plain_43ad4e:
        mov eax,dword ptr [ebp - 0x20]
        cmp eax,dword ptr [ebp - 0x18]
        jle plain_43ab46
plain_43ad5a:
        xor eax,eax
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
plain_43ad62:
        mov eax,0xfffffffd
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
plain_43ad6d:
        mov eax,0xfffffffc
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
    }
}
#pragma optimize("", on)

/* Function start: 0x403574 */
/* This is one of the original hand-written raster loops: it establishes ES,
 * decodes each prepared scan line, and uses REP stores/copies for the runs. */
#pragma optimize("", off)
__declspec(naked) int DrawRLEImageUnclipped(
    RasterClip *clip, RLEFrameHeader *frameHeader, int x, int y,
    int strideScratch)
{
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp + 8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi + 4]
        add dword ptr [ebp + 0x10], eax
        mov eax, dword ptr [esi + 8]
        add dword ptr [ebp + 0x14], eax
        mov eax, dword ptr [ebx + 4]
        inc eax
        mov dword ptr [ebp + 0x18], eax
        jle plain_done
        mov esi, dword ptr [ebp + 0xc]
        mov edi, dword ptr [ebx]
        mov eax, dword ptr [esi + 8]
        add eax, dword ptr [ebp + 0x10]
        add edi, eax
        mov eax, dword ptr [esi + 0xc]
        mov ebx, eax
        add eax, dword ptr [ebp + 0x14]
        mul dword ptr [ebp + 0x18]
        add edi, eax
        mov edx, edi
        mov eax, dword ptr [esi + 0x10]
        mov eax, dword ptr [esi + 0x14]
        inc eax
        sub eax, ebx
        mov ebx, eax
        jle plain_done
        add esi, 0x18
plain_next_row:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja plain_solid
        jne plain_literal
        jae plain_row_done
plain_skip:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja plain_solid
        jne plain_literal
        jb plain_skip
        jae plain_row_done
plain_solid:
        movzx ecx, al
        mov al, byte ptr [esi]
        inc esi
        push ecx
        and ecx, 3
        rep stosb
        mov ah, al
        rol eax, 8
        mov al, ah
        rol eax, 8
        mov al, ah
        pop ecx
        shr ecx, 2
        rep stosd
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja plain_solid
        jae plain_row_done
        je plain_skip
plain_literal:
        movzx ecx, al
        push ecx
        and ecx, 3
        rep movsb
        pop ecx
        shr ecx, 2
        rep movsd
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja plain_solid
        jne plain_literal
        jb plain_skip
plain_row_done:
        add edx, dword ptr [ebp + 0x18]
        mov edi, edx
        dec ebx
        jne plain_next_row
plain_done:
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
    }
}
#pragma optimize("", on)

/* Function start: 0x40363B */
/* The segment-register setup and fixed-size REP copy identify this as one of
 * the original hand-written raster support routines. */
__declspec(naked) void SetPaletteTranslationTable(
    const unsigned char *translation)
{
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp + 8]
        mov edi, offset g_abRasterPaletteTranslation_00491b08
        mov ecx, 0x40
        rep movsd
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
    }
}

/* Function start: 0x40365A */
/* Palette-translated counterpart to the original hand-written clipped
 * raster routine above. */
#pragma optimize("", off)
__declspec(naked) int DrawRLEImageColor(RasterClip *clip,
                                         unsigned char *shape,
                                         int frame, int x, int y)
{
    __asm {
        push ebp
        mov ebp,esp
        add esp,-0x50
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi,dword ptr [ebp + 0x8]
        mov ebx,dword ptr [esi]
        mov eax,dword ptr [ebx + 0x4]
        inc eax
        mov dword ptr [ebp - 0x48],eax
        jle color_43aedd
        mov eax,dword ptr [ebx + 0x8]
        inc eax
        mov ecx,eax
        jle color_43aedd
        mov eax,dword ptr [esi + 0x4]
        mov dword ptr [ebp - 0x4c],eax
        cmp eax,0x0
        jg color_43ae91
        mov eax,0x0
color_43ae91:
        mov dword ptr [ebp - 0x34],eax
        mov eax,dword ptr [esi + 0x8]
        mov dword ptr [ebp - 0x50],eax
        cmp eax,0x0
        jg color_43aea4
        mov eax,0x0
color_43aea4:
        mov dword ptr [ebp - 0x38],eax
        mov eax,dword ptr [esi + 0xc]
        mov edx,dword ptr [ebp - 0x48]
        dec edx
        cmp eax,edx
        jl color_43aeb4
        mov eax,edx
color_43aeb4:
        mov dword ptr [ebp - 0x3c],eax
        mov eax,dword ptr [esi + 0x10]
        mov edx,ecx
        dec edx
        cmp eax,edx
        jl color_43aec3
        mov eax,edx
color_43aec3:
        mov dword ptr [ebp - 0x40],eax
        mov eax,dword ptr [ebp - 0x3c]
        cmp eax,dword ptr [ebp - 0x34]
        jl color_43aee8
        mov eax,dword ptr [ebp - 0x40]
        cmp eax,dword ptr [ebp - 0x38]
        jl color_43aee8
        mov eax,dword ptr [ebx]
        mov dword ptr [ebp - 0x44],eax
        jmp color_43aef3
color_43aedd:
        mov eax,0xffffffff
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
color_43aee8:
        mov eax,0xfffffffe
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
color_43aef3:
        mov eax,dword ptr [ebp - 0x4c]
        add dword ptr [ebp + 0x14],eax
        mov eax,dword ptr [ebp - 0x50]
        add dword ptr [ebp + 0x18],eax
        mov esi,dword ptr [ebp + 0x10]
        shl esi,0x3
        add esi,0x8
        add esi,dword ptr [ebp + 0xc]
        mov esi,dword ptr [esi]
        add esi,dword ptr [ebp + 0xc]
        mov dword ptr [ebp - 0x30],esi
        mov eax,dword ptr [esi + 0x8]
        add eax,dword ptr [ebp + 0x14]
        mov dword ptr [ebp - 0xc],eax
        mov eax,dword ptr [esi + 0xc]
        add eax,dword ptr [ebp + 0x18]
        mov dword ptr [ebp - 0x10],eax
        mov eax,dword ptr [esi + 0x10]
        add eax,dword ptr [ebp + 0x14]
        mov dword ptr [ebp - 0x14],eax
        mov eax,dword ptr [esi + 0x14]
        add eax,dword ptr [ebp + 0x18]
        mov dword ptr [ebp - 0x18],eax
        add esi,0x18
        mov eax,dword ptr [ebp - 0x14]
        cmp eax,dword ptr [ebp - 0xc]
        jl color_43b32b
        mov eax,dword ptr [ebp - 0x18]
        cmp eax,dword ptr [ebp - 0x10]
        jl color_43b32b
        xor edx,edx
        mov eax,dword ptr [ebp - 0xc]
        sub eax,dword ptr [ebp - 0x34]
        shl eax,0x1
        adc dl,dl
        mov eax,dword ptr [ebp - 0x3c]
        sub eax,dword ptr [ebp - 0xc]
        shl eax,0x1
        adc dl,dl
        mov eax,dword ptr [ebp - 0x10]
        sub eax,dword ptr [ebp - 0x38]
        shl eax,0x1
        adc dl,dl
        mov eax,dword ptr [ebp - 0x40]
        sub eax,dword ptr [ebp - 0x10]
        shl eax,0x1
        adc dl,dl
        mov eax,dword ptr [ebp - 0x14]
        sub eax,dword ptr [ebp - 0x34]
        shl eax,0x1
        adc dh,dh
        mov eax,dword ptr [ebp - 0x3c]
        sub eax,dword ptr [ebp - 0x14]
        shl eax,0x1
        adc dh,dh
        mov eax,dword ptr [ebp - 0x18]
        sub eax,dword ptr [ebp - 0x38]
        shl eax,0x1
        adc dh,dh
        mov eax,dword ptr [ebp - 0x40]
        sub eax,dword ptr [ebp - 0x18]
        shl eax,0x1
        adc dh,dh
        mov dword ptr [ebp - 0x1c],edx
        test dh,dl
        jnz color_43b320
        or dl,dh
        jnz color_43afde
        mov esi,dword ptr [ebp + 0x8]
        mov eax,dword ptr [esi + 0x4]
        sub dword ptr [ebp + 0x14],eax
        mov eax,dword ptr [esi + 0x8]
        sub dword ptr [ebp + 0x18],eax
        push dword ptr [ebp - 0x48]
        push dword ptr [ebp + 0x18]
        push dword ptr [ebp + 0x14]
        push dword ptr [ebp - 0x30]
        push dword ptr [ebp + 0x8]
        call DrawRLEImageColorUnclipped
        add esp,0x14
        jmp color_43b318
color_43afde:
        mov eax,dword ptr [ebp - 0x10]
        imul dword ptr [ebp - 0x48]
        add eax,dword ptr [ebp - 0x44]
        add eax,dword ptr [ebp - 0xc]
        mov edi,eax
        mov ecx,dword ptr [ebp - 0x10]
        mov dword ptr [ebp - 0x20],ecx
        jmp color_43b00a
color_43aff4:
        movzx eax,al
        add esi,eax
        dec esi
color_43affa:
        inc esi
color_43affb:
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43affa
        jnz color_43aff4
        jc color_43affa
        add edi,dword ptr [ebp - 0x48]
        inc ecx
color_43b00a:
        cmp ecx,dword ptr [ebp - 0x38]
        jl color_43affb
        mov dword ptr [ebp - 0x24],edi
        mov dword ptr [ebp - 0x20],ecx
        mov eax,edi
        sub eax,dword ptr [ebp - 0xc]
        add eax,dword ptr [ebp - 0x34]
        mov dword ptr [ebp - 0x28],eax
        mov eax,edi
        sub eax,dword ptr [ebp - 0xc]
        add eax,dword ptr [ebp - 0x3c]
        mov dword ptr [ebp - 0x2c],eax
        jmp color_43b30c
color_43b030:
        mov eax,dword ptr [ebp - 0x20]
        cmp eax,dword ptr [ebp - 0x40]
        jg color_43b318
        mov edi,dword ptr [ebp - 0x24]
        test dword ptr [ebp - 0x1c],0x8
        jnz color_43b125
        test dword ptr [ebp - 0x1c],0x400
        jnz color_43b1b3
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b081
        jnz color_43b0b2
        jnc color_43b120
color_43b068:
        mov al,byte ptr [esi]
        inc esi
        movzx ecx,al
        add edi,ecx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b081
        jnz color_43b0b2
        jc color_43b068
        jnc color_43b120
color_43b081:
        movzx ecx,al
color_43b084:
        xor eax,eax
        mov al,byte ptr [esi]
        inc esi
        mov al,byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        push ecx
        and ecx,0x3
        rep stosb
        mov ah,al
        rol eax,0x8
        mov al,ah
        rol eax,0x8
        mov al,ah
        pop ecx
        shr ecx,0x2
        rep stosd
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b081
        jnc color_43b120
        jz color_43b068
color_43b0b2:
        movzx ecx,al
color_43b0b5:
        xor eax,eax
        or ecx,ecx
        jz color_43b10d
        cmp ecx,0x4
        jl color_43b0fe
color_43b0c0:
        mov al,byte ptr [esi]
        mov al,byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi],al
        mov al,byte ptr [esi + 0x1]
        mov al,byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi + 0x1],al
        mov al,byte ptr [esi + 0x2]
        mov al,byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi + 0x2],al
        mov al,byte ptr [esi + 0x3]
        mov al,byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi + 0x3],al
        add esi,0x4
        add edi,0x4
        sub ecx,0x4
        jz color_43b10d
        cmp ecx,0x4
        jge color_43b0c0
color_43b0fe:
        mov al,byte ptr [esi]
        mov al,byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi],al
        inc esi
        inc edi
        dec ecx
        jnz color_43b0fe
color_43b10d:
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b081
        jnz color_43b0b2
        jc color_43b068
color_43b120:
        jmp color_43b2fd
color_43b125:
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b145
        jnz color_43b176
        jnc color_43b1ae
color_43b130:
        mov al,byte ptr [esi]
        inc esi
        movzx ecx,al
        add edi,ecx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b145
        jnz color_43b176
        jc color_43b130
        jnc color_43b1ae
color_43b145:
        movzx ecx,al
        mov eax,dword ptr [ebp - 0x28]
        sub eax,edi
        cmp eax,ecx
        jge color_43b168
        or eax,eax
        js color_43b159
        add edi,eax
        sub ecx,eax
color_43b159:
        test dword ptr [ebp - 0x1c],0x400
        jz color_43b084
        jnz color_43b1de
color_43b168:
        add edi,ecx
        inc esi
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b145
        jnc color_43b1ae
        jz color_43b130
color_43b176:
        movzx ecx,al
        mov eax,dword ptr [ebp - 0x28]
        sub eax,edi
        cmp eax,ecx
        jge color_43b19f
        or eax,eax
        js color_43b18c
        add edi,eax
        sub ecx,eax
        add esi,eax
color_43b18c:
        test dword ptr [ebp - 0x1c],0x400
        jz color_43b0b5
        jnz color_43b22d
color_43b19f:
        add edi,ecx
        add esi,ecx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b145
        jnz color_43b176
        jc color_43b130
color_43b1ae:
        jmp color_43b2fd
color_43b1b3:
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b1db
        jnz color_43b22a
        jnc color_43b2b8
color_43b1c2:
        mov al,byte ptr [esi]
        inc esi
        movzx ecx,al
        add edi,ecx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b1db
        jnz color_43b22a
        jc color_43b1c2
        jnc color_43b2b8
color_43b1db:
        movzx ecx,al
color_43b1de:
        cmp edi,dword ptr [ebp - 0x2c]
        jg color_43b2dd
        mov eax,edi
        add eax,ecx
        dec eax
        sub eax,dword ptr [ebp - 0x2c]
        cdq
        not edx
        and edx,eax
        sub ecx,edx
        xor eax,eax
        mov al,byte ptr [esi]
        inc esi
        mov al,byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        push ecx
        and ecx,0x3
        rep stosb
        mov ah,al
        rol eax,0x8
        mov al,ah
        rol eax,0x8
        mov al,ah
        pop ecx
        shr ecx,0x2
        rep stosd
        add edi,edx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b1db
        jnc color_43b2b8
        jz color_43b1c2
color_43b22a:
        movzx ecx,al
color_43b22d:
        cmp edi,dword ptr [ebp - 0x2c]
        jg color_43b2ee
        mov eax,edi
        add eax,ecx
        dec eax
        sub eax,dword ptr [ebp - 0x2c]
        cdq
        not edx
        and edx,eax
        sub ecx,edx
        xor eax,eax
        or ecx,ecx
        jz color_43b29d
        cmp ecx,0x4
        jl color_43b28e
color_43b250:
        mov al,byte ptr [esi]
        mov al,byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi],al
        mov al,byte ptr [esi + 0x1]
        mov al,byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi + 0x1],al
        mov al,byte ptr [esi + 0x2]
        mov al,byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi + 0x2],al
        mov al,byte ptr [esi + 0x3]
        mov al,byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi + 0x3],al
        add esi,0x4
        add edi,0x4
        sub ecx,0x4
        jz color_43b29d
        cmp ecx,0x4
        jge color_43b250
color_43b28e:
        mov al,byte ptr [esi]
        mov al,byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi],al
        inc esi
        inc edi
        dec ecx
        jnz color_43b28e
color_43b29d:
        add edi,edx
        add esi,edx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b1db
        jnz color_43b22a
        jc color_43b1c2
color_43b2b8:
        jmp color_43b2fd
color_43b2c5:
        mov al,byte ptr [esi]
        inc esi
        movzx ecx,al
        add edi,ecx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b2da
        jnz color_43b2eb
        jc color_43b2c5
        jnc color_43b2fd
color_43b2da:
        movzx ecx,al
color_43b2dd:
        add edi,ecx
        inc esi
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b2da
        jnc color_43b2fd
        jz color_43b2c5
color_43b2eb:
        movzx ecx,al
color_43b2ee:
        add edi,ecx
        add esi,ecx
        mov al,byte ptr [esi]
        inc esi
        shr al,0x1
        ja color_43b2da
        jnz color_43b2eb
        jc color_43b2c5
color_43b2fd:
        mov eax,dword ptr [ebp - 0x48]
        add dword ptr [ebp - 0x24],eax
        add dword ptr [ebp - 0x28],eax
        add dword ptr [ebp - 0x2c],eax
        inc dword ptr [ebp - 0x20]
color_43b30c:
        mov eax,dword ptr [ebp - 0x20]
        cmp eax,dword ptr [ebp - 0x18]
        jle color_43b030
color_43b318:
        xor eax,eax
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
color_43b320:
        mov eax,0xfffffffd
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
color_43b32b:
        mov eax,0xfffffffc
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
    }
}
#pragma optimize("", on)

/* Function start: 0x403B32 */
/* Palette-translated counterpart to the original hand-written loop above. */
#pragma optimize("", off)
__declspec(naked) int DrawRLEImageColorUnclipped(
    RasterClip *clip, RLEFrameHeader *frameHeader, int x, int y,
    int strideScratch)
{
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp + 8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi + 4]
        add dword ptr [ebp + 0x10], eax
        mov eax, dword ptr [esi + 8]
        add dword ptr [ebp + 0x14], eax
        mov eax, dword ptr [ebx + 4]
        inc eax
        mov dword ptr [ebp + 0x18], eax
        jle color_done
        mov esi, dword ptr [ebp + 0xc]
        mov edi, dword ptr [ebx]
        mov eax, dword ptr [esi + 8]
        add eax, dword ptr [ebp + 0x10]
        add edi, eax
        mov eax, dword ptr [esi + 0xc]
        mov ebx, eax
        add eax, dword ptr [ebp + 0x14]
        mul dword ptr [ebp + 0x18]
        add edi, eax
        mov edx, edi
        mov eax, dword ptr [esi + 0x10]
        mov eax, dword ptr [esi + 0x14]
        inc eax
        sub eax, ebx
        mov ebx, eax
        jle color_done
        add esi, 0x18
color_next_row:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja color_solid
        jne color_literal
        jae color_row_done
color_skip:
        mov al, byte ptr [esi]
        inc esi
        movzx ecx, al
        add edi, ecx
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja color_solid
        jne color_literal
        jb color_skip
        jae color_row_done
color_solid:
        movzx ecx, al
        xor eax, eax
        mov al, byte ptr [esi]
        inc esi
        mov al, byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        push ecx
        and ecx, 3
        rep stosb
        mov ah, al
        rol eax, 8
        mov al, ah
        rol eax, 8
        mov al, ah
        pop ecx
        shr ecx, 2
        rep stosd
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja color_solid
        jae color_row_done
        je color_skip
color_literal:
        movzx ecx, al
        xor eax, eax
        or ecx, ecx
        je color_literal_done
        cmp ecx, 4
        jl color_literal_tail
color_literal_four:
        mov al, byte ptr [esi]
        mov al, byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi], al
        mov al, byte ptr [esi + 1]
        mov al, byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi + 1], al
        mov al, byte ptr [esi + 2]
        mov al, byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi + 2], al
        mov al, byte ptr [esi + 3]
        mov al, byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi + 3], al
        add esi, 4
        add edi, 4
        sub ecx, 4
        je color_literal_done
        cmp ecx, 4
        jge color_literal_four
color_literal_tail:
        mov al, byte ptr [esi]
        mov al, byte ptr g_abRasterPaletteTranslation_00491b08[eax]
        mov byte ptr [edi], al
        inc esi
        inc edi
        dec ecx
        jne color_literal_tail
color_literal_done:
        mov al, byte ptr [esi]
        inc esi
        shr al, 1
        ja color_solid
        jne color_literal
        jb color_skip
color_row_done:
        add edx, dword ptr [ebp + 0x18]
        mov edi, edx
        dec ebx
        jne color_next_row
color_done:
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
    }
}
#pragma optimize("", on)

/* Function start: 0x403C65 */
/* The retail routine is a hand-written fixed-point scanline texture mapper.
 * It transforms a four-corner workspace, clips both polygon edges, and walks
 * the decoded RLE image with direction-specific source increments. */
#pragma optimize("", off)
__declspec(naked) int RotateRLEImage(
    RasterClip *clip, unsigned char *shape, int frame, int x, int y,
    unsigned char *scratch, unsigned int angleTenths, int scaleX, int scaleY,
    unsigned int flags)
{
#ifdef _MSC_VER
#include "screens_rotate_rle_image.inc"
#else
    return 0;
#endif
}
#pragma optimize("", on)

/* Function start: 0x404811 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* The command-stream walk retains the raster library's explicit ES setup. */
#pragma optimize("", off)
__declspec(naked) unsigned int GetRLEFrameBounds(
    unsigned char *shape, int frame, int x, int y, unsigned int flags,
    int *bounds)
{
#ifdef _MSC_VER
#include "screens_get_rle_frame_bounds.inc"
#else
    return 0;
#endif
}
#pragma optimize("", on)

/* Function start: 0x404989 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* The paired forward/backward scans and explicit segment state belong to the
 * hand-written RLE encoder. */
__declspec(naked) int EncodeRasterClipToRLEFrame(
    RasterClip *clip, unsigned char transparentColour,
    int originX, int originY, unsigned char *output)
{
#ifdef _MSC_VER
#include "screens_encode_raster_clip_to_rle_frame.inc"
#else
    return 0;
#endif
}

/* Function start: 0x404C0C */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Preserve the hand-written RLE command-stream traversal. */
#pragma optimize("", off)
__declspec(naked) unsigned int TranslateRLEFramePalette(
    unsigned char *shape, int frame)
{
#ifdef _MSC_VER
#include "screens_translate_rle_frame_palette.inc"
#else
    return 0;
#endif
}
#pragma optimize("", on)

/* Function start: 0x404C9E */
/* The register-paired run classifier and explicit ES setup identify the
 * original scanline encoder as hand-written raster assembly. */
__declspec(naked) void EncodeRLEScanline(
    int pixelCount, unsigned char transparentColour, int sourceX)
{
    __asm {
        push ebp
        mov ebp, esp
        add esp, -4
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr g_pRLEScanlineStart_004902c1
        mov dword ptr g_pRLEScanCursor_004902c5, esi
        push dword ptr [ebp + 10h]
        push 0
        push 0
        call EmitRLEScanlineRun
        add esp, 0ch
        mov dword ptr [ebp - 4], 5
        mov ecx, dword ptr [ebp + 8]
        or ecx, ecx
        jz encode_finish_line
        mov al, byte ptr [esi]
        inc esi
        dec ecx
        mov ah, al
encode_classify:
        cmp ah, byte ptr [ebp + 0ch]
        jz encode_transparent_run
        mov dword ptr [ebp - 4], 1
        or ecx, ecx
        jz encode_finish_line
        mov al, byte ptr [esi]
        inc esi
        dec ecx
        xor al, ah
        xor ah, al
        or al, al
        jz encode_solid_run
        cmp ah, byte ptr [ebp + 0ch]
        jnz encode_literal_scan
        mov dword ptr g_pRLEScanCursor_004902c5, esi
        push dword ptr [ebp + 10h]
        push 1
        push 1
        call EmitRLEScanlineRun
        add esp, 0ch
        jmp encode_transparent_run
encode_literal_scan:
        or ecx, ecx
        jz encode_finish_line
        mov al, byte ptr [esi]
        inc esi
        dec ecx
        xor al, ah
        xor ah, al
        cmp ah, byte ptr [ebp + 0ch]
        jnz encode_literal_pair
        mov dword ptr g_pRLEScanCursor_004902c5, esi
        push dword ptr [ebp + 10h]
        push 1
        push 1
        call EmitRLEScanlineRun
        add esp, 0ch
        jmp encode_transparent_run
encode_literal_pair:
        or al, al
        jnz encode_literal_scan
        or ecx, ecx
        jz encode_finish_line
        mov al, byte ptr [esi]
        inc esi
        dec ecx
        xor al, ah
        xor ah, al
        cmp ah, byte ptr [ebp + 0ch]
        jnz encode_literal_triplet
        mov dword ptr g_pRLEScanCursor_004902c5, esi
        push dword ptr [ebp + 10h]
        push 1
        push 1
        call EmitRLEScanlineRun
        add esp, 0ch
        jmp encode_transparent_run
encode_literal_triplet:
        or al, al
        jnz encode_literal_scan
        mov dword ptr g_pRLEScanCursor_004902c5, esi
        push dword ptr [ebp + 10h]
        push 3
        push 1
        call EmitRLEScanlineRun
        add esp, 0ch
        mov dword ptr [ebp - 4], 2
encode_solid_run:
        or ecx, ecx
        jz encode_finish_line
        mov al, byte ptr [esi]
        inc esi
        dec ecx
        xor al, ah
        jz encode_solid_run
        xor ah, al
        mov dword ptr g_pRLEScanCursor_004902c5, esi
        push dword ptr [ebp + 10h]
        push 1
        push 2
        call EmitRLEScanlineRun
        add esp, 0ch
        cmp ah, byte ptr [ebp + 0ch]
        jnz encode_classify
        mov dword ptr [ebp - 4], 3
encode_transparent_run:
        or ecx, ecx
        jz encode_finish_line
        mov al, byte ptr [esi]
        inc esi
        dec ecx
        xor al, ah
        jz encode_transparent_run
        xor ah, al
        mov dword ptr g_pRLEScanCursor_004902c5, esi
        push dword ptr [ebp + 10h]
        push 1
        push 3
        call EmitRLEScanlineRun
        add esp, 0ch
        jmp encode_classify
encode_finish_line:
        mov dword ptr g_pRLEScanCursor_004902c5, esi
        push dword ptr [ebp + 10h]
        push 0
        push dword ptr [ebp - 4]
        call EmitRLEScanlineRun
        add esp, 0ch
        push dword ptr [ebp + 10h]
        push 0
        push 4
        call EmitRLEScanlineRun
        add esp, 0ch
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
    }
}

/* Function start: 0x404E27 */
/* Companion command emitter for the hand-written RLE scanline encoder. */
__declspec(naked) void EmitRLEScanlineRun(
    int runType, int trailingCount, int sourceX)
{
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        push eax
        push ecx
        mov esi, dword ptr g_pRLELiteralStart_004902cd
        mov edi, dword ptr g_pRLEOutputCursor_004902c9
        mov eax, dword ptr [ebp + 8]
        cmp eax, 2
        jz emit_solid
        cmp eax, 1
        jz emit_literal
        cmp eax, 3
        jz emit_defer_skip
        cmp eax, 4
        jz emit_end_line
        cmp eax, 0
        jnz emit_done
        xor eax, eax
        mov dword ptr g_nRLEPendingSkip_004902bd, eax
        mov esi, dword ptr g_pRLEScanCursor_004902c5
        mov dword ptr g_pRLELiteralStart_004902cd, esi
        jmp emit_done
emit_solid:
        mov ebx, dword ptr g_nRLEPendingSkip_004902bd
        or ebx, ebx
        jz emit_solid_bounds
emit_solid_skip_chunks:
        mov ecx, ebx
        cmp ecx, 0ffh
        jl emit_solid_skip_ready
        mov ecx, 0ffh
emit_solid_skip_ready:
        sub ebx, ecx
        cmp dword ptr g_pRLEEncodeBuffer_004902b9, 0
        jz emit_solid_skip_advance
        mov al, 1
        mov byte ptr [edi], al
        inc edi
        mov al, cl
        mov byte ptr [edi], al
        inc edi
        jmp emit_solid_skip_input
emit_solid_skip_advance:
        add edi, 2
emit_solid_skip_input:
        add esi, ecx
        or ebx, ebx
        jnz emit_solid_skip_chunks
        mov dword ptr g_nRLEPendingSkip_004902bd, ebx
emit_solid_bounds:
        mov ebx, dword ptr g_pRLEScanCursor_004902c5
        sub ebx, esi
        sub ebx, dword ptr [ebp + 0ch]
        mov eax, dword ptr [ebp + 10h]
        add eax, esi
        sub eax, dword ptr g_pRLEScanlineStart_004902c1
        cmp eax, dword ptr g_nRLEEncodedMinimumX_004902e1
        jge emit_solid_maximum
        mov dword ptr g_nRLEEncodedMinimumX_004902e1, eax
emit_solid_maximum:
        add eax, ebx
        dec eax
        cmp eax, dword ptr g_nRLEEncodedMaximumX_004902e9
        jle emit_solid_chunks
        mov dword ptr g_nRLEEncodedMaximumX_004902e9, eax
        jmp emit_solid_chunks
emit_solid_next_chunk:
        mov ecx, ebx
        cmp ecx, 7fh
        jl emit_solid_chunk_ready
        mov ecx, 7fh
emit_solid_chunk_ready:
        cmp dword ptr g_pRLEEncodeBuffer_004902b9, 0
        jz emit_solid_chunk_advance
        mov al, cl
        add al, al
        mov byte ptr [edi], al
        inc edi
        mov al, byte ptr [esi]
        mov byte ptr [edi], al
        inc edi
        jmp emit_solid_chunk_input
emit_solid_chunk_advance:
        add edi, 2
emit_solid_chunk_input:
        add esi, ecx
        sub ebx, ecx
emit_solid_chunks:
        or ebx, ebx
        jnz emit_solid_next_chunk
        jmp emit_done
emit_literal:
        mov ebx, dword ptr g_nRLEPendingSkip_004902bd
        or ebx, ebx
        jz emit_literal_bounds
emit_literal_skip_chunks:
        mov ecx, ebx
        cmp ecx, 0ffh
        jl emit_literal_skip_ready
        mov ecx, 0ffh
emit_literal_skip_ready:
        sub ebx, ecx
        cmp dword ptr g_pRLEEncodeBuffer_004902b9, 0
        jz emit_literal_skip_advance
        mov al, 1
        mov byte ptr [edi], al
        inc edi
        mov al, cl
        mov byte ptr [edi], al
        inc edi
        jmp emit_literal_skip_input
emit_literal_skip_advance:
        add edi, 2
emit_literal_skip_input:
        add esi, ecx
        or ebx, ebx
        jnz emit_literal_skip_chunks
        mov dword ptr g_nRLEPendingSkip_004902bd, ebx
emit_literal_bounds:
        mov ebx, dword ptr g_pRLEScanCursor_004902c5
        sub ebx, esi
        sub ebx, dword ptr [ebp + 0ch]
        mov eax, dword ptr [ebp + 10h]
        add eax, esi
        sub eax, dword ptr g_pRLEScanlineStart_004902c1
        cmp eax, dword ptr g_nRLEEncodedMinimumX_004902e1
        jge emit_literal_maximum
        mov dword ptr g_nRLEEncodedMinimumX_004902e1, eax
emit_literal_maximum:
        add eax, ebx
        dec eax
        cmp eax, dword ptr g_nRLEEncodedMaximumX_004902e9
        jle emit_literal_chunks
        mov dword ptr g_nRLEEncodedMaximumX_004902e9, eax
        jmp emit_literal_chunks
emit_literal_next_chunk:
        mov ecx, ebx
        cmp ecx, 7fh
        jl emit_literal_chunk_ready
        mov ecx, 7fh
emit_literal_chunk_ready:
        mov edx, ecx
        mov al, cl
        add al, al
        inc al
        cmp dword ptr g_pRLEEncodeBuffer_004902b9, 0
        jz emit_literal_chunk_advance
        mov byte ptr [edi], al
        inc edi
        rep movsb
        jmp emit_literal_chunk_input
emit_literal_chunk_advance:
        inc edi
        add esi, ecx
        add edi, ecx
emit_literal_chunk_input:
        sub ebx, edx
emit_literal_chunks:
        or ebx, ebx
        jnz emit_literal_next_chunk
        jmp emit_done
emit_defer_skip:
        mov ebx, dword ptr g_pRLEScanCursor_004902c5
        sub ebx, esi
        sub ebx, dword ptr [ebp + 0ch]
        mov dword ptr g_nRLEPendingSkip_004902bd, ebx
        jmp emit_done
emit_end_line:
        xor eax, eax
        cmp dword ptr g_pRLEEncodeBuffer_004902b9, 0
        jz emit_end_advance
        mov byte ptr [edi], al
        inc edi
        jmp emit_done
emit_end_advance:
        inc edi
emit_done:
        mov dword ptr g_pRLEOutputCursor_004902c9, edi
        mov dword ptr g_pRLELiteralStart_004902cd, esi
        pop ecx
        pop eax
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
    }
}

/* Function start: 0x405004 */
/* The explicit ES setup and REP stores identify the original as hand-written
 * raster assembly, so preserve that implementation rather than asking the C
 * compiler to synthesize a materially different fill loop. */
__declspec(naked) int FillRasterClip(RasterClip *clip, int colour)
{
    __asm {
        push ebp
        mov ebp, esp
        add esp, -20h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp + 8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 4]
        inc eax
        mov dword ptr [ebp - 18h], eax
        jle invalid_surface
        mov eax, dword ptr [ebx + 8]
        inc eax
        mov ecx, eax
        jle invalid_surface
        mov eax, dword ptr [esi + 4]
        mov dword ptr [ebp - 1ch], eax
        cmp eax, 0
        jg left_clipped
        mov eax, 0
left_clipped:
        mov dword ptr [ebp - 4], eax
        mov eax, dword ptr [esi + 8]
        mov dword ptr [ebp - 20h], eax
        cmp eax, 0
        jg top_clipped
        mov eax, 0
top_clipped:
        mov dword ptr [ebp - 8], eax
        mov eax, dword ptr [esi + 0ch]
        mov edx, dword ptr [ebp - 18h]
        dec edx
        cmp eax, edx
        jl right_clipped
        mov eax, edx
right_clipped:
        mov dword ptr [ebp - 0ch], eax
        mov eax, dword ptr [esi + 10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl bottom_clipped
        mov eax, edx
bottom_clipped:
        mov dword ptr [ebp - 10h], eax
        mov eax, dword ptr [ebp - 0ch]
        cmp eax, dword ptr [ebp - 4]
        jl empty_clip
        mov eax, dword ptr [ebp - 10h]
        cmp eax, dword ptr [ebp - 8]
        jl empty_clip
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp - 14h], eax
        jmp begin_fill
invalid_surface:
        mov eax, -1
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
empty_clip:
        mov eax, -2
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
begin_fill:
        mov eax, dword ptr [ebp - 8]
        imul dword ptr [ebp - 18h]
        add eax, dword ptr [ebp - 14h]
        add eax, dword ptr [ebp - 4]
        mov edi, eax
        mov ebx, dword ptr [ebp - 0ch]
        inc ebx
        sub ebx, dword ptr [ebp - 4]
        mov esi, dword ptr [ebp - 18h]
        sub esi, ebx
        mov al, byte ptr [ebp + 0ch]
        mov ah, al
        shl eax, 10h
        mov al, byte ptr [ebp + 0ch]
        mov ah, al
        mov edx, dword ptr [ebp - 8]
        jmp test_row
fill_row:
        mov ecx, ebx
        and ecx, 3
        rep stosb
        mov ecx, ebx
        shr ecx, 2
        rep stosd
        add edi, esi
        inc edx
test_row:
        cmp edx, dword ptr [ebp - 10h]
        jle fill_row
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
    }
}

/* Function start: 0x4050E3 */
/* This is the companion hand-written clipped blitter.  Direction-flag and
 * segment-register handling are part of its overlap-safe behavior. */
__declspec(naked) int BlitRasterClip(
    RasterClip *source, int sourceX, int sourceY,
    RasterClip *destination, int destinationX,
    int destinationY, unsigned int colour)
{
    __asm {
        push ebp
        mov ebp, esp
        add esp, -9ch
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp + 8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 4]
        inc eax
        mov dword ptr [ebp - 70h], eax
        jle blit_invalid_source
        mov eax, dword ptr [ebx + 8]
        inc eax
        mov ecx, eax
        jle blit_invalid_source
        mov eax, dword ptr [esi + 4]
        mov dword ptr [ebp - 78h], eax
        cmp eax, 0
        jg blit_source_left
        mov eax, 0
blit_source_left:
        mov dword ptr [ebp - 60h], eax
        mov eax, dword ptr [esi + 8]
        mov dword ptr [ebp - 7ch], eax
        cmp eax, 0
        jg blit_source_top
        mov eax, 0
blit_source_top:
        mov dword ptr [ebp - 64h], eax
        mov eax, dword ptr [esi + 0ch]
        mov edx, dword ptr [ebp - 70h]
        dec edx
        cmp eax, edx
        jl blit_source_right
        mov eax, edx
blit_source_right:
        mov dword ptr [ebp - 68h], eax
        mov eax, dword ptr [esi + 10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl blit_source_bottom
        mov eax, edx
blit_source_bottom:
        mov dword ptr [ebp - 6ch], eax
        mov eax, dword ptr [ebp - 68h]
        cmp eax, dword ptr [ebp - 60h]
        jl blit_empty_source
        mov eax, dword ptr [ebp - 6ch]
        cmp eax, dword ptr [ebp - 64h]
        jl blit_empty_source
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp - 74h], eax
        jmp blit_source_ready
blit_invalid_source:
        mov eax, -1
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
blit_empty_source:
        mov eax, -2
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
blit_source_ready:
        mov eax, dword ptr [ebp - 60h]
        mov dword ptr [ebp - 2ch], eax
        mov eax, dword ptr [ebp - 64h]
        mov dword ptr [ebp - 30h], eax
        mov eax, dword ptr [ebp - 68h]
        mov dword ptr [ebp - 34h], eax
        mov eax, dword ptr [ebp - 6ch]
        mov dword ptr [ebp - 38h], eax
        mov eax, dword ptr [ebp - 78h]
        sub dword ptr [ebp - 2ch], eax
        sub dword ptr [ebp - 34h], eax
        mov eax, dword ptr [ebp - 7ch]
        sub dword ptr [ebp - 30h], eax
        sub dword ptr [ebp - 38h], eax
        mov esi, dword ptr [ebp + 14h]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 4]
        inc eax
        mov dword ptr [ebp - 94h], eax
        jle blit_invalid_destination
        mov eax, dword ptr [ebx + 8]
        inc eax
        mov ecx, eax
        jle blit_invalid_destination
        mov eax, dword ptr [esi + 4]
        mov dword ptr [ebp - 98h], eax
        cmp eax, 0
        jg blit_destination_left
        mov eax, 0
blit_destination_left:
        mov dword ptr [ebp - 80h], eax
        mov eax, dword ptr [esi + 8]
        mov dword ptr [ebp - 9ch], eax
        cmp eax, 0
        jg blit_destination_top
        mov eax, 0
blit_destination_top:
        mov dword ptr [ebp - 84h], eax
        mov eax, dword ptr [esi + 0ch]
        mov edx, dword ptr [ebp - 94h]
        dec edx
        cmp eax, edx
        jl blit_destination_right
        mov eax, edx
blit_destination_right:
        mov dword ptr [ebp - 88h], eax
        mov eax, dword ptr [esi + 10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl blit_destination_bottom
        mov eax, edx
blit_destination_bottom:
        mov dword ptr [ebp - 8ch], eax
        mov eax, dword ptr [ebp - 88h]
        cmp eax, dword ptr [ebp - 80h]
        jl blit_empty_destination
        mov eax, dword ptr [ebp - 8ch]
        cmp eax, dword ptr [ebp - 84h]
        jl blit_empty_destination
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp - 90h], eax
        jmp blit_destination_ready
blit_invalid_destination:
        mov eax, -1
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
blit_empty_destination:
        mov eax, -2
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
blit_destination_ready:
        mov eax, dword ptr [ebp - 80h]
        mov dword ptr [ebp - 40h], eax
        mov eax, dword ptr [ebp - 84h]
        mov dword ptr [ebp - 44h], eax
        mov eax, dword ptr [ebp - 88h]
        mov dword ptr [ebp - 48h], eax
        mov eax, dword ptr [ebp - 8ch]
        mov dword ptr [ebp - 4ch], eax
        mov eax, dword ptr [ebp - 98h]
        sub dword ptr [ebp - 40h], eax
        sub dword ptr [ebp - 48h], eax
        mov eax, dword ptr [ebp - 9ch]
        sub dword ptr [ebp - 44h], eax
        sub dword ptr [ebp - 4ch], eax
        mov eax, dword ptr [ebp + 0ch]
        sub eax, dword ptr [ebp + 18h]
        mov dword ptr [ebp - 24h], eax
        mov eax, dword ptr [ebp + 10h]
        sub eax, dword ptr [ebp + 1ch]
        mov dword ptr [ebp - 28h], eax
        mov eax, dword ptr [ebp - 2ch]
        mov edx, dword ptr [ebp - 40h]
        add edx, dword ptr [ebp - 24h]
        cmp eax, edx
        jg blit_copy_left
        mov eax, edx
blit_copy_left:
        mov dword ptr [ebp - 4], eax
        mov eax, dword ptr [ebp - 30h]
        mov edx, dword ptr [ebp - 44h]
        add edx, dword ptr [ebp - 28h]
        cmp eax, edx
        jg blit_copy_top
        mov eax, edx
blit_copy_top:
        mov dword ptr [ebp - 8], eax
        mov eax, dword ptr [ebp - 34h]
        mov edx, dword ptr [ebp - 48h]
        add edx, dword ptr [ebp - 24h]
        cmp eax, edx
        jl blit_copy_right
        mov eax, edx
blit_copy_right:
        mov dword ptr [ebp - 0ch], eax
        mov eax, dword ptr [ebp - 38h]
        mov edx, dword ptr [ebp - 4ch]
        add edx, dword ptr [ebp - 28h]
        cmp eax, edx
        jl blit_copy_bottom
        mov eax, edx
blit_copy_bottom:
        mov dword ptr [ebp - 10h], eax
        mov eax, dword ptr [ebp - 0ch]
        cmp eax, dword ptr [ebp - 4]
        jl blit_no_overlap
        mov eax, dword ptr [ebp - 10h]
        cmp eax, dword ptr [ebp - 8]
        jl blit_no_overlap
        mov eax, dword ptr [ebp - 40h]
        mov edx, dword ptr [ebp - 2ch]
        sub edx, dword ptr [ebp - 24h]
        cmp eax, edx
        jg blit_destination_copy_left
        mov eax, edx
blit_destination_copy_left:
        mov dword ptr [ebp - 14h], eax
        mov eax, dword ptr [ebp - 44h]
        mov edx, dword ptr [ebp - 30h]
        sub edx, dword ptr [ebp - 28h]
        cmp eax, edx
        jg blit_destination_copy_top
        mov eax, edx
blit_destination_copy_top:
        mov dword ptr [ebp - 18h], eax
        mov eax, dword ptr [ebp - 48h]
        mov edx, dword ptr [ebp - 34h]
        sub edx, dword ptr [ebp - 24h]
        cmp eax, edx
        jl blit_destination_copy_right
        mov eax, edx
blit_destination_copy_right:
        mov dword ptr [ebp - 1ch], eax
        mov eax, dword ptr [ebp - 4ch]
        mov edx, dword ptr [ebp - 38h]
        sub edx, dword ptr [ebp - 28h]
        cmp eax, edx
        jl blit_destination_copy_bottom
        mov eax, edx
blit_destination_copy_bottom:
        mov dword ptr [ebp - 20h], eax
        mov eax, dword ptr [ebp - 0ch]
        inc eax
        sub eax, dword ptr [ebp - 4]
        mov dword ptr [ebp - 58h], eax
        mov eax, dword ptr [ebp - 10h]
        inc eax
        sub eax, dword ptr [ebp - 8]
        mov dword ptr [ebp - 54h], eax
        mov eax, dword ptr [ebp - 7ch]
        imul dword ptr [ebp - 70h]
        add eax, dword ptr [ebp - 74h]
        add eax, dword ptr [ebp - 78h]
        mov esi, eax
        mov eax, dword ptr [ebp - 9ch]
        imul dword ptr [ebp - 94h]
        add eax, dword ptr [ebp - 90h]
        add eax, dword ptr [ebp - 98h]
        mov edi, eax
        mov eax, dword ptr [ebp - 8]
        mov ebx, dword ptr [ebp - 18h]
        cmp eax, ebx
        jle blit_bottom_up
        mul dword ptr [ebp - 70h]
        add esi, eax
        mov eax, ebx
        mul dword ptr [ebp - 94h]
        add edi, eax
        mov eax, dword ptr [ebp - 70h]
        mov dword ptr [ebp - 3ch], eax
        mov eax, dword ptr [ebp - 94h]
        mov dword ptr [ebp - 50h], eax
        jmp blit_vertical_ready
blit_bottom_up:
        mov eax, dword ptr [ebp - 10h]
        mul dword ptr [ebp - 70h]
        add esi, eax
        mov eax, dword ptr [ebp - 20h]
        mul dword ptr [ebp - 94h]
        add edi, eax
        mov eax, dword ptr [ebp - 70h]
        neg eax
        mov dword ptr [ebp - 3ch], eax
        mov eax, dword ptr [ebp - 94h]
        neg eax
        mov dword ptr [ebp - 50h], eax
blit_vertical_ready:
        mov ecx, dword ptr [ebp - 58h]
        mov eax, dword ptr [ebp - 4]
        mov ebx, dword ptr [ebp - 14h]
        cmp eax, ebx
        jle blit_right_to_left
        add esi, eax
        add edi, ebx
        sub dword ptr [ebp - 3ch], ecx
        sub dword ptr [ebp - 50h], ecx
        cld
        mov dword ptr [ebp - 5ch], 0
        jmp blit_direction_ready
blit_right_to_left:
        add esi, dword ptr [ebp - 0ch]
        add edi, dword ptr [ebp - 1ch]
        add dword ptr [ebp - 3ch], ecx
        add dword ptr [ebp - 50h], ecx
        std
        mov dword ptr [ebp - 5ch], 3
blit_direction_ready:
        mov eax, dword ptr [ebp + 20h]
        test eax, 0ffffff00h
        jz blit_fill
        mov edx, dword ptr [ebp - 54h]
        mov eax, dword ptr [ebp - 3ch]
        mov ebx, dword ptr [ebp - 50h]
blit_copy_row:
        mov ecx, dword ptr [ebp - 58h]
        and ecx, 3
        rep movsb
        mov ecx, dword ptr [ebp - 58h]
        shr ecx, 2
        sub esi, dword ptr [ebp - 5ch]
        sub edi, dword ptr [ebp - 5ch]
        rep movsd
        add esi, dword ptr [ebp - 5ch]
        add edi, dword ptr [ebp - 5ch]
        add esi, eax
        add edi, ebx
        dec edx
        jnz blit_copy_row
        cld
        jmp blit_success
blit_fill:
        mov dl, al
        mov ah, al
        shl eax, 10h
        mov al, dl
        mov ah, al
        mov edx, dword ptr [ebp - 54h]
        mov ebx, dword ptr [ebp - 50h]
blit_fill_row:
        mov ecx, dword ptr [ebp - 58h]
        and ecx, 3
        rep stosb
        mov ecx, dword ptr [ebp - 58h]
        shr ecx, 2
        sub edi, dword ptr [ebp - 5ch]
        rep stosd
        add edi, dword ptr [ebp - 5ch]
        add edi, ebx
        dec edx
        jnz blit_fill_row
        cld
blit_success:
        xor eax, eax
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
blit_no_overlap:
        mov eax, -3
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0xc9
        ret
    }
}

/* Function start: 0x40547F */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Preserve the raster library's explicit segment setup and wrapped nine-blit
 * decomposition. */
__declspec(naked) int ScrollRasterClipWrapped(
    RasterClip *clip, int deltaX, int deltaY, int mode,
    unsigned int fillOrScratch)
{
#ifdef _MSC_VER
#include "screens_scroll_raster_clip_wrapped.inc"
#else
    return 0;
#endif
}

/* Function start: 0x40567C */
/* The segment setup, explicit clip arithmetic, and symmetric pixel stores
 * identify this as the retail hand-written midpoint ellipse rasterizer. */
#pragma optimize("", off)
__declspec(naked) unsigned int DrawRasterEllipse(
    RasterClip *clip, int x, int y, int horizontalRadius,
    int verticalRadius, int colour)
{
#ifdef _MSC_VER
#include "screens_draw_raster_ellipse.inc"
#else
    return 0;
#endif
}
#pragma optimize("", on)

/* Function start: 0x4059BD */
/* Filled counterpart to the hand-written midpoint ellipse outline routine. */
__declspec(naked) unsigned int FillRasterEllipse(
    RasterClip *clip, int x, int y, int horizontalRadius,
    int verticalRadius, int colour)
{
    __asm {
        push ebp
        mov ebp, esp
        add esp, -54h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        cmp dword ptr [ebp + 14h], 0
        jz fill_ellipse_line
        cmp dword ptr [ebp + 18h], 0
        jnz fill_ellipse_surface
fill_ellipse_line:
        mov eax, dword ptr [ebp + 10h]
        add eax, dword ptr [ebp + 18h]
        mov ebx, dword ptr [ebp + 0ch]
        add ebx, dword ptr [ebp + 14h]
        mov ecx, dword ptr [ebp + 10h]
        sub ecx, dword ptr [ebp + 18h]
        mov edx, dword ptr [ebp + 0ch]
        sub edx, dword ptr [ebp + 14h]
        push dword ptr [ebp + 1ch]
        push 0
        push eax
        push ebx
        push ecx
        push edx
        push dword ptr [ebp + 8]
        call DrawClippedLine
        add esp, 1ch
        jmp fill_ellipse_done
fill_ellipse_surface:
        mov esi, dword ptr [ebp + 8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 4]
        inc eax
        mov dword ptr [ebp - 4ch], eax
        jle fill_ellipse_invalid_surface
        mov eax, dword ptr [ebx + 8]
        inc eax
        mov ecx, eax
        jle fill_ellipse_invalid_surface
        mov eax, dword ptr [esi + 4]
        mov dword ptr [ebp - 50h], eax
        cmp eax, 0
        jg fill_ellipse_left_ready
        mov eax, 0
fill_ellipse_left_ready:
        mov dword ptr [ebp - 38h], eax
        mov eax, dword ptr [esi + 8]
        mov dword ptr [ebp - 54h], eax
        cmp eax, 0
        jg fill_ellipse_top_ready
        mov eax, 0
fill_ellipse_top_ready:
        mov dword ptr [ebp - 3ch], eax
        mov eax, dword ptr [esi + 0ch]
        mov edx, dword ptr [ebp - 4ch]
        dec edx
        cmp eax, edx
        jl fill_ellipse_right_ready
        mov eax, edx
fill_ellipse_right_ready:
        mov dword ptr [ebp - 40h], eax
        mov eax, dword ptr [esi + 10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl fill_ellipse_bottom_ready
        mov eax, edx
fill_ellipse_bottom_ready:
        mov dword ptr [ebp - 44h], eax
        mov eax, dword ptr [ebp - 40h]
        cmp eax, dword ptr [ebp - 38h]
        jl fill_ellipse_empty_clip
        mov eax, dword ptr [ebp - 44h]
        cmp eax, dword ptr [ebp - 3ch]
        jl fill_ellipse_empty_clip
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp - 48h], eax
        jmp fill_ellipse_begin
fill_ellipse_invalid_surface:
        mov eax, -1
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
fill_ellipse_empty_clip:
        mov eax, -2
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
fill_ellipse_begin:
        mov eax, dword ptr [ebp + 1ch]
        mov ah, al
        mov dword ptr [ebp + 1ch], eax
        mov word ptr [ebp + 1eh], ax
        mov eax, dword ptr [ebp - 50h]
        add dword ptr [ebp + 0ch], eax
        mov eax, dword ptr [ebp - 54h]
        add dword ptr [ebp + 10h], eax
        mov eax, dword ptr [ebp + 0ch]
        mov dword ptr [ebp - 4], eax
        mov eax, dword ptr [ebp + 10h]
        mov dword ptr [ebp - 8], eax
        mov dword ptr [ebp - 0ch], 0
        mov eax, dword ptr [ebp + 18h]
        mov dword ptr [ebp - 10h], eax
        mul eax
        mov dword ptr [ebp - 1ch], eax
        shl eax, 1
        mov dword ptr [ebp - 20h], eax
        mov eax, dword ptr [ebp + 14h]
        mul eax
        mov dword ptr [ebp - 14h], eax
        shl eax, 1
        mov dword ptr [ebp - 18h], eax
        mov dword ptr [ebp - 24h], 0
        mov eax, dword ptr [ebp - 18h]
        mul dword ptr [ebp + 18h]
        mov dword ptr [ebp - 28h], eax
        mov eax, dword ptr [ebp - 14h]
        shr eax, 2
        add eax, dword ptr [ebp - 1ch]
        mov dword ptr [ebp - 2ch], eax
        mov eax, dword ptr [ebp - 14h]
        mul dword ptr [ebp + 18h]
        sub dword ptr [ebp - 2ch], eax
        mov ebx, dword ptr [ebp + 18h]
fill_ellipse_region_one_test:
        mov eax, dword ptr [ebp - 24h]
        sub eax, dword ptr [ebp - 28h]
        jns fill_ellipse_region_two_setup
        mov edi, dword ptr [ebp - 4]
        add edi, dword ptr [ebp - 0ch]
        cmp edi, dword ptr [ebp - 38h]
        jl fill_ellipse_region_one_step
        cmp edi, dword ptr [ebp - 40h]
        jl fill_ellipse_region_one_right_ready
        mov edi, dword ptr [ebp - 40h]
fill_ellipse_region_one_right_ready:
        mov dword ptr [ebp - 34h], edi
        mov edi, dword ptr [ebp - 4]
        sub edi, dword ptr [ebp - 0ch]
        cmp edi, dword ptr [ebp - 40h]
        jg fill_ellipse_region_one_step
        cmp edi, dword ptr [ebp - 38h]
        jg fill_ellipse_region_one_left_ready
        mov edi, dword ptr [ebp - 38h]
fill_ellipse_region_one_left_ready:
        mov dword ptr [ebp - 30h], edi
        mov edx, dword ptr [ebp - 8]
        add edx, dword ptr [ebp - 10h]
        cmp edx, dword ptr [ebp - 3ch]
        jl fill_ellipse_region_one_step
        cmp edx, dword ptr [ebp - 44h]
        jg fill_ellipse_region_one_lower
        mov eax, edx
        imul dword ptr [ebp - 4ch]
        add eax, dword ptr [ebp - 48h]
        add eax, edi
        mov edi, eax
        mov ecx, dword ptr [ebp - 34h]
        sub ecx, dword ptr [ebp - 30h]
        inc ecx
        mov eax, dword ptr [ebp + 1ch]
        mov edx, ecx
        and edx, 3
        shr ecx, 2
        rep stosd
        mov ecx, edx
        rep stosb
fill_ellipse_region_one_lower:
        mov edi, dword ptr [ebp - 30h]
        mov edx, dword ptr [ebp - 8]
        sub edx, dword ptr [ebp - 10h]
        cmp edx, dword ptr [ebp - 3ch]
        jl fill_ellipse_region_one_step
        cmp edx, dword ptr [ebp - 44h]
        jg fill_ellipse_region_one_step
        mov eax, edx
        imul dword ptr [ebp - 4ch]
        add eax, dword ptr [ebp - 48h]
        add eax, edi
        mov edi, eax
        mov ecx, dword ptr [ebp - 34h]
        sub ecx, dword ptr [ebp - 30h]
        inc ecx
        mov eax, dword ptr [ebp + 1ch]
        mov edx, ecx
        and edx, 3
        shr ecx, 2
        rep stosd
        mov ecx, edx
        rep stosb
fill_ellipse_region_one_step:
        cmp dword ptr [ebp - 2ch], 0
        js fill_ellipse_region_one_advance_x
        dec dword ptr [ebp - 10h]
        dec ebx
        mov eax, dword ptr [ebp - 28h]
        sub eax, dword ptr [ebp - 18h]
        mov dword ptr [ebp - 28h], eax
        sub dword ptr [ebp - 2ch], eax
fill_ellipse_region_one_advance_x:
        inc dword ptr [ebp - 0ch]
        mov eax, dword ptr [ebp - 24h]
        add eax, dword ptr [ebp - 20h]
        mov dword ptr [ebp - 24h], eax
        add eax, dword ptr [ebp - 1ch]
        add dword ptr [ebp - 2ch], eax
        jmp fill_ellipse_region_one_test
fill_ellipse_region_two_setup:
        mov eax, dword ptr [ebp - 14h]
        sub eax, dword ptr [ebp - 1ch]
        mov edx, eax
        sar eax, 1
        add eax, edx
        sub eax, dword ptr [ebp - 24h]
        sub eax, dword ptr [ebp - 28h]
        sar eax, 1
        add dword ptr [ebp - 2ch], eax
fill_ellipse_region_two:
        mov edi, dword ptr [ebp - 4]
        add edi, dword ptr [ebp - 0ch]
        cmp edi, dword ptr [ebp - 38h]
        jl fill_ellipse_region_two_step
        cmp edi, dword ptr [ebp - 40h]
        jl fill_ellipse_region_two_right_ready
        mov edi, dword ptr [ebp - 40h]
fill_ellipse_region_two_right_ready:
        mov dword ptr [ebp - 34h], edi
        mov edi, dword ptr [ebp - 4]
        sub edi, dword ptr [ebp - 0ch]
        cmp edi, dword ptr [ebp - 40h]
        jg fill_ellipse_region_two_step
        cmp edi, dword ptr [ebp - 38h]
        jg fill_ellipse_region_two_left_ready
        mov edi, dword ptr [ebp - 38h]
fill_ellipse_region_two_left_ready:
        mov dword ptr [ebp - 30h], edi
        mov edx, dword ptr [ebp - 8]
        add edx, dword ptr [ebp - 10h]
        cmp edx, dword ptr [ebp - 3ch]
        jl fill_ellipse_region_two_step
        cmp edx, dword ptr [ebp - 44h]
        jg fill_ellipse_region_two_lower
        mov eax, edx
        imul dword ptr [ebp - 4ch]
        add eax, dword ptr [ebp - 48h]
        add eax, edi
        mov edi, eax
        mov ecx, dword ptr [ebp - 34h]
        sub ecx, dword ptr [ebp - 30h]
        inc ecx
        mov eax, dword ptr [ebp + 1ch]
        mov edx, ecx
        and edx, 3
        shr ecx, 2
        rep stosd
        mov ecx, edx
        rep stosb
fill_ellipse_region_two_lower:
        mov edi, dword ptr [ebp - 30h]
        mov edx, dword ptr [ebp - 8]
        sub edx, dword ptr [ebp - 10h]
        cmp edx, dword ptr [ebp - 3ch]
        jl fill_ellipse_region_two_step
        cmp edx, dword ptr [ebp - 44h]
        jg fill_ellipse_region_two_step
        mov eax, edx
        imul dword ptr [ebp - 4ch]
        add eax, dword ptr [ebp - 48h]
        add eax, edi
        mov edi, eax
        mov ecx, dword ptr [ebp - 34h]
        sub ecx, dword ptr [ebp - 30h]
        inc ecx
        mov eax, dword ptr [ebp + 1ch]
        mov edx, ecx
        and edx, 3
        shr ecx, 2
        rep stosd
        mov ecx, edx
        rep stosb
fill_ellipse_region_two_step:
        cmp dword ptr [ebp - 2ch], 0
        jns fill_ellipse_region_two_advance_y
        inc dword ptr [ebp - 0ch]
        mov eax, dword ptr [ebp - 24h]
        add eax, dword ptr [ebp - 20h]
        mov dword ptr [ebp - 24h], eax
        add dword ptr [ebp - 2ch], eax
fill_ellipse_region_two_advance_y:
        dec dword ptr [ebp - 10h]
        mov eax, dword ptr [ebp - 28h]
        sub eax, dword ptr [ebp - 18h]
        mov dword ptr [ebp - 28h], eax
        sub eax, dword ptr [ebp - 14h]
        sub dword ptr [ebp - 2ch], eax
        dec ebx
        js fill_ellipse_done
        jmp fill_ellipse_region_two
fill_ellipse_done:
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
    }
}

/* Function start: 0x406ACF */
/* Segment preservation and the in-text lookup table identify this as part of
 * the original hand-written raster assembly. */
__declspec(naked) void GetRLETransformTrig(int angleTenths, int *cosine,
                                           int *sine)
{
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov ebx, dword ptr [ebp + 8]
        and ebx, ebx
        jns rle_trig_reduce_high
rle_trig_reduce_low:
        add ebx, 0e10h
        js rle_trig_reduce_low
        jmp rle_trig_reduce_high
rle_trig_subtract_turn:
        sub ebx, 0e10h
rle_trig_reduce_high:
        cmp ebx, 0e10h
        jg rle_trig_subtract_turn
        cmp ebx, 708h
        ja rle_trig_lower_half
        cmp ebx, 384h
        ja rle_trig_second_quadrant
        shl ebx, 2
        mov eax, dword ptr g_anRLEQuarterCosine_00405cbb[ebx]
        neg ebx
        mov edx, dword ptr g_anRLEQuarterCosine_00405cbb[ebx + 0e10h]
        jmp rle_trig_store
rle_trig_second_quadrant:
        neg ebx
        add ebx, 708h
        shl ebx, 2
        mov eax, dword ptr g_anRLEQuarterCosine_00405cbb[ebx]
        neg eax
        neg ebx
        mov edx, dword ptr g_anRLEQuarterCosine_00405cbb[ebx + 0e10h]
        jmp rle_trig_store
rle_trig_lower_half:
        neg ebx
        add ebx, 0e10h
        cmp ebx, 384h
        ja rle_trig_fourth_quadrant
        shl ebx, 2
        mov eax, dword ptr g_anRLEQuarterCosine_00405cbb[ebx]
        neg ebx
        mov edx, dword ptr g_anRLEQuarterCosine_00405cbb[ebx + 0e10h]
        neg edx
        jmp rle_trig_store
rle_trig_fourth_quadrant:
        neg ebx
        add ebx, 708h
        shl ebx, 2
        mov eax, dword ptr g_anRLEQuarterCosine_00405cbb[ebx]
        neg eax
        neg ebx
        mov edx, dword ptr g_anRLEQuarterCosine_00405cbb[ebx + 0e10h]
        neg edx
rle_trig_store:
        mov ebx, dword ptr [ebp + 0ch]
        mov dword ptr [ebx], eax
        mov ebx, dword ptr [ebp + 10h]
        mov dword ptr [ebx], edx
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
    }
}

/* Function start: 0x406B87 */
__declspec(naked) void CalculateRoundedRLEFixedProduct(int left, int right,
                                                       int *result)
{
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov eax, dword ptr [ebp + 8]
        imul dword ptr [ebp + 0ch]
        add eax, 8000h
        adc edx, 0
        mov ax, dx
        ror eax, 10h
        mov edi, dword ptr [ebp + 10h]
        mov dword ptr [edi], eax
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
    }
}

/* Function start: 0x406BAD */
__declspec(naked) void TransformRLEPoint(int *point, int *result,
                                         int *origin,
                                         unsigned int angleTenths,
                                         int scaleX, int scaleY)
{
    __asm {
        push ebp
        mov ebp, esp
        add esp, -20h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        lea eax, [ebp - 8]
        push eax
        lea eax, [ebp - 4]
        push eax
        push dword ptr [ebp + 14h]
        call GetRLETransformTrig
        add esp, 0ch
        mov esi, dword ptr [ebp + 8]
        mov edi, dword ptr [ebp + 10h]
        mov eax, dword ptr [esi]
        sub eax, dword ptr [edi]
        shl eax, 10h
        imul dword ptr [ebp + 18h]
        add eax, 8000h
        adc edx, 0
        mov ebx, edx
        mov eax, ebx
        imul dword ptr [ebp - 4]
        add eax, 8000h
        adc edx, 0
        mov ax, dx
        ror eax, 10h
        mov dword ptr [ebp - 0ch], eax
        mov eax, ebx
        imul dword ptr [ebp - 8]
        add eax, 8000h
        adc edx, 0
        mov ax, dx
        ror eax, 10h
        mov dword ptr [ebp - 14h], eax
        mov eax, dword ptr [esi + 4]
        sub eax, dword ptr [edi + 4]
        shl eax, 10h
        imul dword ptr [ebp + 1ch]
        add eax, 8000h
        adc edx, 0
        mov ecx, edx
        mov eax, ecx
        imul dword ptr [ebp - 4]
        add eax, 8000h
        adc edx, 0
        mov ax, dx
        ror eax, 10h
        mov dword ptr [ebp - 18h], eax
        mov eax, ecx
        imul dword ptr [ebp - 8]
        add eax, 8000h
        adc edx, 0
        mov ax, dx
        ror eax, 10h
        mov dword ptr [ebp - 10h], eax
        mov esi, dword ptr [ebp + 0ch]
        mov edx, dword ptr [ebp - 0ch]
        sub edx, dword ptr [ebp - 10h]
        add edx, dword ptr [edi]
        mov dword ptr [esi], edx
        mov edx, dword ptr [ebp - 18h]
        add edx, dword ptr [ebp - 14h]
        add edx, dword ptr [edi + 4]
        mov dword ptr [esi + 4], edx
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
    }
}

/* Function start: 0x406C74 */
/* The preserved segment register identifies this as another handwritten
 * raster-library accessor.  Offset 8 is the row count used by the raw-frame
 * blitter below this interface. */
__declspec(naked) unsigned int GetRawImageHeight(unsigned char *shape)
{
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp + 8]
        mov eax, dword ptr [esi + 8]
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
    }
}

/* Function start: 0x406C87 */
__declspec(naked) unsigned int GetRawFrameWidth(unsigned char *shape,
                                                 int frame)
{
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov eax, dword ptr [ebp + 0ch]
        shl eax, 2
        add eax, dword ptr [ebp + 8]
        add eax, 10h
        mov esi, dword ptr [eax]
        add esi, dword ptr [ebp + 8]
        mov eax, dword ptr [esi]
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
    }
}

/* Function start: 0x406CA7 */
/* This clipped raw-frame blitter preserves the original hand-written segment
 * setup, REP copies, and XLAT palette-translation path. */
__declspec(naked) int BlitRawFrame(
    RasterClip *clip, int x, int y, unsigned char *shape,
    int frame, unsigned char *translation)
{
    __asm {
        push ebp
        mov ebp, esp
        add esp, -30h
        push ebx
        push esi
        push edi
        push es
        cld
        push ds
        pop es
        mov esi, dword ptr [ebp + 8]
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [ebx + 4]
        inc eax
        mov dword ptr [ebp - 28h], eax
        jle raw_frame_invalid_surface
        mov eax, dword ptr [ebx + 8]
        inc eax
        mov ecx, eax
        jle raw_frame_invalid_surface
        mov eax, dword ptr [esi + 4]
        mov dword ptr [ebp - 2ch], eax
        cmp eax, 0
        jg raw_frame_left_ready
        mov eax, 0
raw_frame_left_ready:
        mov dword ptr [ebp - 14h], eax
        mov eax, dword ptr [esi + 8]
        mov dword ptr [ebp - 30h], eax
        cmp eax, 0
        jg raw_frame_top_ready
        mov eax, 0
raw_frame_top_ready:
        mov dword ptr [ebp - 18h], eax
        mov eax, dword ptr [esi + 0ch]
        mov edx, dword ptr [ebp - 28h]
        dec edx
        cmp eax, edx
        jl raw_frame_right_ready
        mov eax, edx
raw_frame_right_ready:
        mov dword ptr [ebp - 1ch], eax
        mov eax, dword ptr [esi + 10h]
        mov edx, ecx
        dec edx
        cmp eax, edx
        jl raw_frame_bottom_ready
        mov eax, edx
raw_frame_bottom_ready:
        mov dword ptr [ebp - 20h], eax
        mov eax, dword ptr [ebp - 1ch]
        cmp eax, dword ptr [ebp - 14h]
        jl raw_frame_empty_clip
        mov eax, dword ptr [ebp - 20h]
        cmp eax, dword ptr [ebp - 18h]
        jl raw_frame_empty_clip
        mov eax, dword ptr [ebx]
        mov dword ptr [ebp - 24h], eax
        jmp raw_frame_begin
raw_frame_invalid_surface:
        mov eax, -1
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
raw_frame_empty_clip:
        mov eax, -2
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
raw_frame_begin:
        mov eax, dword ptr [ebp - 2ch]
        add dword ptr [ebp + 0ch], eax
        mov eax, dword ptr [ebp - 30h]
        add dword ptr [ebp + 10h], eax
        mov esi, dword ptr [ebp + 14h]
        mov edx, dword ptr [esi + 8]
        mov eax, dword ptr [ebp + 18h]
        shl eax, 2
        add eax, dword ptr [ebp + 14h]
        add eax, 10h
        mov esi, dword ptr [eax]
        add esi, dword ptr [ebp + 14h]
        mov dword ptr [ebp - 8], 0
        mov ecx, dword ptr [esi]
        mov dword ptr [ebp - 4], ecx
        cmp ecx, 0
        jz raw_frame_done
        add esi, 4
        mov edi, dword ptr [ebp + 8]
        mov eax, dword ptr [ebp - 1ch]
        inc eax
        sub eax, ecx
        sub eax, dword ptr [ebp + 0ch]
        jns raw_frame_clip_left
        add ecx, eax
        jle raw_frame_done
raw_frame_clip_left:
        mov eax, dword ptr [ebp + 0ch]
        sub eax, dword ptr [ebp - 14h]
        jns raw_frame_clip_bottom
        add ecx, eax
        jle raw_frame_done
        sub esi, eax
        sub dword ptr [ebp + 0ch], eax
raw_frame_clip_bottom:
        mov eax, dword ptr [ebp - 20h]
        inc eax
        sub eax, edx
        sub eax, dword ptr [ebp + 10h]
        jns raw_frame_clip_top
        add edx, eax
        jle raw_frame_done
raw_frame_clip_top:
        mov eax, dword ptr [ebp + 10h]
        sub eax, dword ptr [ebp - 18h]
        jns raw_frame_position
        add edx, eax
        jle raw_frame_done
        sub dword ptr [ebp + 10h], eax
        imul eax, dword ptr [ebp - 4]
        sub esi, eax
raw_frame_position:
        mov dword ptr [ebp - 10h], edx
        mov eax, dword ptr [ebp + 10h]
        imul dword ptr [ebp - 28h]
        add eax, dword ptr [ebp - 24h]
        add eax, dword ptr [ebp + 0ch]
        mov edi, eax
        mov dword ptr [ebp - 8], ecx
        sub dword ptr [ebp - 4], ecx
        mov eax, dword ptr [ebp - 28h]
        sub eax, ecx
        mov dword ptr [ebp - 0ch], eax
        mov edx, dword ptr [ebp - 10h]
        cmp dword ptr [ebp + 1ch], 0
        jnz raw_frame_translate
raw_frame_copy_row:
        shr ecx, 1
        rep movsw
        adc ecx, 0
        rep movsb
        mov ecx, dword ptr [ebp - 8]
        add esi, dword ptr [ebp - 4]
        add edi, dword ptr [ebp - 0ch]
        dec edx
        jnz raw_frame_copy_row
        mov eax, dword ptr [ebp - 4]
        add eax, dword ptr [ebp - 8]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
raw_frame_translate:
        jecxz raw_frame_done
        mov ebx, dword ptr [ebp + 1ch]
raw_frame_translate_pixel:
        mov al, byte ptr [esi]
        xlatb
        cmp al, 0ffh
        jz raw_frame_skip_translated
        mov byte ptr [edi], al
raw_frame_skip_translated:
        inc esi
        inc edi
        loop raw_frame_translate_pixel
        mov ecx, dword ptr [ebp - 8]
        add esi, dword ptr [ebp - 4]
        add edi, dword ptr [ebp - 0ch]
        dec edx
        jnz raw_frame_translate_pixel
        mov eax, dword ptr [ebp - 4]
        add eax, dword ptr [ebp - 8]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
raw_frame_done:
        mov eax, dword ptr [ebp - 4]
        add eax, dword ptr [ebp - 8]
        pop es
        pop edi
        pop esi
        pop ebx
        leave
        ret
    }
}

/* Function start: 0x406E3A */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Preserve the segment state used by the adjacent hand-written raw blitters. */
__declspec(naked) void BlitSelectedRawFrames(
    RasterClip *clip, int x, int y, unsigned char *shape,
    const unsigned char *frames, unsigned char *translation)
{
#ifdef _MSC_VER
#include "screens_blit_selected_raw_frames.inc"
#else
    return;
#endif
}

/* Function start: 0x406E71 */
/* Shared by the ILBM, PCX, and GIF decoders.  The segment preservation and
 * width-first REP copy identify this as a hand-written scanline blitter. */
__declspec(naked) int BlitRawScanline(RasterClip *clip, int y,
                                      const unsigned char *pixels,
                                      int width)
{
#ifdef _MSC_VER
#include "screens_blit_raw_scanline.inc"
#else
    return 0;
#endif
}

/* Function start: 0x406F80 */
/* The adjacent BMHD/CMAP/BODY identifiers and big-endian chunk length identify
 * this as the common IFF chunk-data locator. */
__declspec(naked) unsigned char *FindIFFChunkData(
    const char *chunkId, const unsigned char *iffData)
{
#ifdef _MSC_VER
#include "screens_find_iff_chunk_data.inc"
#else
    return 0;
#endif
}

/* Function start: 0x406FC2 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Preserve the hand-written ByteRun1 and planar-to-chunky decoder loops. */
__declspec(naked) unsigned int DecodeIFFImage(
    RasterClip *clip, const unsigned char *iffData)
{
#ifdef _MSC_VER
#include "screens_decode_iff_image.inc"
#else
    return 0;
#endif
}

/* Function start: 0x407189 */
__declspec(naked) void CopyILBMPalette(const unsigned char *iffData,
                                       unsigned char *palette)
{
#ifdef _MSC_VER
#include "screens_copy_ilbm_palette.inc"
#else
    return;
#endif
}

/* Function start: 0x4071BA */
__declspec(naked) unsigned int GetILBMImageSize(
    const unsigned char *iffData)
{
#ifdef _MSC_VER
#include "screens_get_ilbm_image_size.inc"
#else
    return 0;
#endif
}

/* Function start: 0x4071E7 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Preserve the hand-written PCX run decoder and its ES scanline writes. */
__declspec(naked) unsigned int DecodePCXImage(
    RasterClip *clip, const unsigned char *pcxData)
{
#ifdef _MSC_VER
#include "screens_decode_pcx_image.inc"
#else
    return 0;
#endif
}

/* Function start: 0x407269 */
/* A PCX file stores its 256 RGB palette entries in the final 768 bytes;
 * raster palette components are converted from eight to six bits here. */
__declspec(naked) void CopyPCXPaletteFromFileTail(
    const unsigned char *fileData, unsigned int fileSize,
    unsigned char *palette)
{
#ifdef _MSC_VER
#include "screens_copy_pcx_palette_from_file_tail.inc"
#else
    return;
#endif
}

/* Function start: 0x407294 */
__declspec(naked) unsigned int GetPCXImageSize(const unsigned char *header)
{
#ifdef _MSC_VER
#include "screens_get_pcx_image_size.inc"
#else
    return 0;
#endif
}

/* Function start: 0x4072BB */
/* These five helpers form the register-calling-convention core of the GIF LZW
 * decoder at 0x0043EC29.  EDI holds its workspace throughout the group. */
__declspec(naked) void ResetGIFLZWDictionary(void)
{
#ifdef _MSC_VER
#include "screens_reset_gif_lzw_dictionary.inc"
#else
    return;
#endif
}

/* Function start: 0x407303 */
__declspec(naked) unsigned int ReadGIFDataSubBlockByte(void)
{
#ifdef _MSC_VER
#include "screens_read_gif_data_sub_block_byte.inc"
#else
    return 0;
#endif
}

/* Function start: 0x40731C */
__declspec(naked) unsigned int ReadGIFLZWCode(void)
{
#ifdef _MSC_VER
#include "screens_read_gif_lzw_code.inc"
#else
    return 0;
#endif
}

/* Function start: 0x407362 */
__declspec(naked) void AppendGIFLZWDictionaryEntry(void)
{
#ifdef _MSC_VER
#include "screens_append_gif_lzw_dictionary_entry.inc"
#else
    return;
#endif
}

/* Function start: 0x4073A8 */
__declspec(naked) void EmitGIFDecodedPixel(void)
{
#ifdef _MSC_VER
#include "screens_emit_gif_decoded_pixel.inc"
#else
    return;
#endif
}

/* Function start: 0x407425 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* The decoder uses the register-convention GIF helpers and explicit segment
 * state of the hand-written raster library. */
__declspec(naked) unsigned int ExpandGIFLZWImage(
    RasterClip *clip, const unsigned char *gifData, void *workspace)
{
#ifdef _MSC_VER
#include "screens_expand_gif_lzw_image.inc"
#else
    return 0;
#endif
}

/* Function start: 0x40763E */
/* GIF global and local color tables use the same packed size field; a local
 * table, when present, replaces the global palette copied first. */
__declspec(naked) void CopyGIFPalette(const unsigned char *gifData,
                                      unsigned char *palette)
{
#ifdef _MSC_VER
#include "screens_copy_gif_palette.inc"
#else
    return;
#endif
}

/* Function start: 0x40769F */
__declspec(naked) unsigned int GetGIFImageSize(
    const unsigned char *gifData)
{
#ifdef _MSC_VER
#include "screens_get_gif_image_size.inc"
#else
    return 0;
#endif
}

/* Function start: 0x4076D7 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Retain the segment-preserving raster-library accessor verbatim. */
__declspec(naked) unsigned int GetRLEFrameDimensions(
    unsigned char *shape, int frame)
{
#ifdef _MSC_VER
#include "screens_get_rle_frame_dimensions.inc"
#else
    return 0;
#endif
}

/* Function start: 0x4076F9 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Retain the segment-preserving raster-library accessor verbatim. */
__declspec(naked) unsigned int GetRLEFrameExtents(
    unsigned char *shape, int frame)
{
#ifdef _MSC_VER
#include "screens_get_rle_frame_extents.inc"
#else
    return 0;
#endif
}

/* Function start: 0x40771C */
/* The ES save with no C-visible use identifies this pair as hand-written
 * raster-library accessors, so retain the original instruction sequence. */
__declspec(naked) unsigned int GetRLEImageSize(unsigned char *shape, int frame)
{
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp + 8]
        add esi, 8
        mov eax, dword ptr [ebp + 0ch]
        shl eax, 3
        add esi, eax
        mov esi, dword ptr [esi]
        add esi, dword ptr [ebp + 8]
        mov eax, dword ptr [esi + 10h]
        sub eax, dword ptr [esi + 8]
        inc eax
        mov ebx, dword ptr [esi + 14h]
        sub ebx, dword ptr [esi + 0ch]
        inc ebx
        shl eax, 10h
        mov ax, bx
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
    }
}

/* Function start: 0x407750 */
__declspec(naked) unsigned int GetRLEImageOrigin(unsigned char *shape,
                                                  int frame)
{
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp + 8]
        add esi, 8
        mov eax, dword ptr [ebp + 0ch]
        shl eax, 3
        add esi, eax
        mov esi, dword ptr [esi]
        add esi, dword ptr [ebp + 8]
        mov eax, dword ptr [esi + 8]
        shl eax, 10h
        mov ax, word ptr [esi + 0ch]
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
    }
}

/* Function start: 0x40777A */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Preserve the hand-written palette-delta stream traversal. */
__declspec(naked) void ApplyRLEFramePalette(
    unsigned char *shape, int frame, unsigned char *palette)
{
#ifdef _MSC_VER
#include "screens_apply_rle_frame_palette.inc"
#else
    return;
#endif
}

/* Function start: 0x4077C5 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Preserve the hand-written palette-delta stream copy. */
__declspec(naked) unsigned int CopyRLEFramePalette(
    unsigned char *shape, int frame, unsigned char *entries)
{
#ifdef _MSC_VER
#include "screens_copy_rle_frame_palette.inc"
#else
    return 0;
#endif
}

/* Function start: 0x40780D */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Preserve the hand-written palette-delta stream copy. */
__declspec(naked) unsigned int SetRLEFramePalette(
    unsigned char *shape, int frame, const unsigned char *entries)
{
#ifdef _MSC_VER
#include "screens_set_rle_frame_palette.inc"
#else
    return 0;
#endif
}

/* Function start: 0x407857 */
__declspec(naked) unsigned int GetRLEFrameCount(const unsigned char *shape)
{
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        push es
        mov esi, dword ptr [ebp + 8]
        mov eax, dword ptr [esi + 4]
        pop es
        pop edi
        pop esi
        pop ebx
        _emit 0c9h
        ret
    }
}

/* Function start: 0x40786A */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Retain the segment-preserving directory scan from the raster library. */
__declspec(naked) int CollectUniqueRLEImageFrames(
    const unsigned char *shape, unsigned int *frames)
{
#ifdef _MSC_VER
#include "screens_collect_unique_rle_image_frames.inc"
#else
    return 0;
#endif
}

/* Function start: 0x4078CC */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Retain the segment-preserving directory scan from the raster library. */
__declspec(naked) int CollectUniqueRLEPaletteFrames(
    const unsigned char *shape, unsigned int *frames)
{
#ifdef _MSC_VER
#include "screens_collect_unique_rle_palette_frames.inc"
#else
    return 0;
#endif
}

/* Function start: 0x407C21 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Preserve the callback-driven, fixed-point palette fade implemented by the
 * hand-written raster library. */
__declspec(naked) void FadeRasterPaletteToPalette(
    RasterSurface *surface, const unsigned char *targetPalette,
    unsigned int duration)
{
#ifdef _MSC_VER
#include "screens_fade_raster_palette_to_palette.inc"
#else
    return;
#endif
}

/* Function start: 0x407DA5 */
/* No inbound reference is known in the shipped executable; this routine is
 * believed unreachable. */
/* Preserve the segment-based colour-set scan used by the raster library. */
__declspec(naked) int CollectRasterClipColours(
    RasterClip *clip, unsigned int *colours)
{
#ifdef _MSC_VER
#include "screens_collect_raster_clip_colours.inc"
#else
    return 0;
#endif
}

#else
#include "screens_portable.inc"
#endif /* !SDL_PORT */
