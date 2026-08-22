/*
 *  Music state machine and the streaming music script.
 *
 *  Address range 0x42d000-0x42efff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: the names the routines print -- startMusic,
 *  startIntMusic, queue_next_music, queue_stop, flushFX.
 */
#include "game.h"

/* Function start: 0x44F332 */
void *LoadSpeechPacketIntoBuffer(char *fileName, short section,
                                 void *destination)
{
    void *packet;

    packet = destination;
    if (packet == 0) {
        packet = AllocateTaggedMemory(
            (unsigned int)g_wSpeechCacheCodeBytes_0048e0e0, 0x44);
    }
    if (packet == 0)
        return 0;
    if (LoadPacketIntoBuffer(fileName, section, packet, 1) == 0)
        return 0;
    return packet;
}

/* Function start: 0x44F3A4 */
void PlaySpeechPacketBuffer(void *packet, unsigned int packetSize,
                            int interrupt)
{
    PlayRawSpeechBuffer(packet, packetSize & 0xffff, interrupt);
}

/* Function start: 0x44F52E */
void LoadAndPlaySpeechPacket(char *fileName, short section)
{
    unsigned int packetSize;

    packetSize = GetNamedPacketSize(fileName, section);
    if (LoadSpeechPacketIntoBuffer(
            fileName, section, g_pSpeechCacheCode_005d1720) != 0) {
        PlaySpeechPacketBuffer(
            g_pSpeechCacheCode_005d1720, packetSize, 0);
    }
}

/* Function start: 0x44F84F */
short InitializeSpeechCache(short unitCount, short sizeCode)
{
    void *cacheMetadata;
    unsigned int codeBase;
    unsigned int codeEnd;
    void *discardedDataAllocation;
    unsigned int dataBase;
    unsigned int dataEnd;

    g_pSpeechCacheCode_005d1720 =
        AllocateTaggedMemory((unsigned int)g_wSpeechCacheCodeBytes_0048e0e0,
                             0x44);
    if (g_pSpeechCacheCode_005d1720 == 0)
        return 1;
    codeBase = IdentityDword((DwordPtr)g_pSpeechCacheCode_005d1720);
    codeEnd = (unsigned int)g_pSpeechCacheCode_005d1720 +
        (unsigned int)g_wSpeechCacheCodeBytes_0048e0e0;
    if (codeEnd < codeBase)
        ReportFatalErrorCode("027");
    g_wSpeechCacheBlockBytes_0049bb58 =
        (unsigned short)(((unsigned short)unitCount << 4) + 0x200);
    g_wSpeechCacheSizeCode_0049bb5c = (unsigned short)sizeCode;
    cacheMetadata = AllocateTaggedMemory(0x0c, 0x40);
    if (cacheMetadata == 0)
        return 2;
    g_pSpeechCacheDataAllocation_005d1718 =
        AllocateTaggedMemory(
            (unsigned int)g_wSpeechCacheDataWords_0048e0e4 * 2 + 0x0f,
            0x42);
    if (g_pSpeechCacheDataAllocation_005d1718 == 0)
        return 1;
    do {
        discardedDataAllocation = 0;
        dataBase = (unsigned int)g_pSpeechCacheDataAllocation_005d1718;
        dataEnd = dataBase +
            ((int)((unsigned int)g_wSpeechCacheDataWords_0048e0e4 * 2) /
             16);
        if ((dataEnd & 0xfffff000) != (dataBase & 0xfffff000)) {
            discardedDataAllocation =
                g_pSpeechCacheDataAllocation_005d1718;
            g_pSpeechCacheDataAllocation_005d1718 =
                AllocateTaggedMemory(
                    (unsigned int)g_wSpeechCacheDataWords_0048e0e4 * 2 +
                        0x0f,
                    0x42);
            if (g_pSpeechCacheDataAllocation_005d1718 == 0)
                return 1;
            ReleasePacketHandle(discardedDataAllocation);
        } else {
            discardedDataAllocation = 0;
        }
        PumpWindowMessages(0);
    } while (discardedDataAllocation != 0);
    g_ucSpeechCachePage_005d1724 = (unsigned char)(dataBase >> 12);
    dataBase -= (unsigned int)g_ucSpeechCachePage_005d1724 << 12;
    g_wSpeechCacheSegment_0049bb64 = (unsigned short)(dataBase << 4);
    g_wSpeechCacheEndSegment_0049bb68 =
        (unsigned short)(g_wSpeechCacheDataWords_0048e0e4 +
                         g_wSpeechCacheSegment_0049bb64);
    return 0;
}

/* Function start: 0x44FA31 */
void ShutdownSpeechCache(void)
{
    g_wSpeechCacheState_0049bb60 = 0;
    FreePacketAndClear(&g_pSpeechCacheDataAllocation_005d1718, 0x40);
    FreePacketAndClear(&g_pSpeechCacheCode_005d1720, 0x44);
}

static int g_nActiveMusicStreamMask_0049be9c;
static signed char g_acMusicTrackStreamFlags_0049beb8[72] = {
    6, 6, 6, 6, 6, 6, 10, 10, 10, 10, 10, 10,
    6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0,
    0, 0, 0, 6, 16, 16, 0, 6, 6, 16, 16, 0,
    16, 16, 16, 16, 16, 6, 10, 1, 1, 1, 1, 7,
    1, 1, 1, 1, 1, 1, 1, 6, 6, 6, 1, 16,
    16, 1, 1, 16, 16, 16, 0, 0, 0, 0, 0, 0
};

/* Function start: 0x45A2C0 */
void InitializeFireworks(void)
{
    short i;

    for (i = 0; i < 0x1e; i++)
        g_aFireworks_005c8df0[i].frame = -1;
}

/* Function start: 0x45A300 */
short TheEndFireWorks(Viewport *viewport, short count)
{
    short emptyCount;
    short index;

    emptyCount = 0;
    for (index = (short)(count - 1); index >= 0; index--) {
        if (g_aFireworks_005c8df0[index].frame == -1) {
            emptyCount++;
        } else {
            DrawSpriteDefault(
                viewport, g_aFireworks_005c8df0[index].x,
                g_aFireworks_005c8df0[index].y,
                g_pTitleFireworkShape_005c8f58,
                (short)(g_aFireworks_005c8df0[index].frame +
                        g_aFireworks_005c8df0[index].variant * 8));
            if (g_aFireworks_005c8df0[index].frame++ == 7) {
                g_aFireworks_005c8df0[index].frame = -1;
                ((void (__cdecl *)(int, short))FlushSoundEffectsAndLog)(
                    g_aFireworks_005c8df0[index].soundHandle, index);
            } else if (g_aFireworks_005c8df0[index].frame == 1) {
                PlaySfxWaveFileByNumber(0x25, -1, 0);
            }
        }
    }
    return emptyCount;
}

/* Function start: 0x45A441 */
void InitializeConstellationField(Viewport *viewport, short direction,
                                  short density)
{
    short width;
    short index;
    short randomIndex;
    short height;

    g_pConstellationViewport_005c8f5c = viewport;
    g_nConstellationDirection_0049cb20 = direction;
    width = (short)(viewport->right - viewport->left);
    height = (short)(viewport->bottom - viewport->top);
    g_nConstellationParticleCount_005c9014 =
        (short)(density * 16 / 16);
    index = 0;
    while (index < g_nConstellationParticleCount_005c9014) {
        if (g_nConstellationDirection_0049cb20 != 0) {
            randomIndex = RandomInRange(0, 15);
            g_aConstellationParticles_005c8f60[index].x =
                (short)(g_pConstellationViewport_005c8f5c->left +
                        RandomInRange(0, width));
            g_aConstellationParticles_005c8f60[index].y =
                (short)(g_pConstellationViewport_005c8f5c->top +
                        RandomInRange(0, height));
            g_aConstellationParticles_005c8f60[index].deltaX =
                (short)(g_asConstellationVelocity_0049cae0[randomIndex] *
                        g_nConstellationDirection_0049cb20);
            g_aConstellationParticles_005c8f60[index].frame =
                (short)(g_asConstellationFrame_0049cb00[randomIndex] +
                        RandomInRange(0, 3));
        } else {
            g_aConstellationParticles_005c8f60[index].x =
                (short)(g_pConstellationViewport_005c8f5c->left - 1);
            g_aConstellationParticles_005c8f60[index].frame = 0;
        }
        index++;
    }
}

/* Function start: 0x45A5C7 */
signed char IsConstellationParticleOutsideViewport(
    ConstellationParticle particle)
{
    if (g_pConstellationViewport_005c8f5c->left <= particle.x &&
        g_pConstellationViewport_005c8f5c->right >= particle.x &&
        g_pConstellationViewport_005c8f5c->top <= particle.y &&
        g_pConstellationViewport_005c8f5c->bottom >= particle.y) {
        return 0;
    }
    return 1;
}

/* Function start: 0x45A634 */
void DrawConstellationField(void)
{
    short speed;
    short middleX;
    short width;
    short middleY;
    short index;
    short randomIndex;
    short frame;
    short height;
    int direction;

    height = (short)(g_pConstellationViewport_005c8f5c->bottom -
                     g_pConstellationViewport_005c8f5c->top);
    for (index = 0; index < g_nConstellationParticleCount_005c9014;
         index++) {
        DrawSpriteDefault(g_pConstellationViewport_005c8f5c,
                          g_aConstellationParticles_005c8f60[index].x,
                          g_aConstellationParticles_005c8f60[index].y,
                          g_pConstellationShape_005d2c4c,
                          g_aConstellationParticles_005c8f60[index].frame);
        g_aConstellationParticles_005c8f60[index].x = (short)(
            g_aConstellationParticles_005c8f60[index].x +
            g_aConstellationParticles_005c8f60[index].deltaX);
        g_aConstellationParticles_005c8f60[index].y = (short)(
            g_aConstellationParticles_005c8f60[index].y +
            g_aConstellationParticles_005c8f60[index].deltaY);
        frame = (short)(
            (g_aConstellationParticles_005c8f60[index].frame + 1) % 4);
        g_aConstellationParticles_005c8f60[index].frame = (short)(
            (g_aConstellationParticles_005c8f60[index].frame & 0xfc) +
            frame);
        direction = g_nConstellationDirection_0049cb20;
        switch (direction) {
        case -1:
            if (IsConstellationParticleOutsideViewport(
                    g_aConstellationParticles_005c8f60[index]) != 0) {
                randomIndex = RandomInRange(0, 15);
                speed = g_asConstellationVelocity_0049cae0[randomIndex];
                g_aConstellationParticles_005c8f60[index].x = (short)(
                    g_pConstellationViewport_005c8f5c->right -
                    RandomInRange(0, speed));
                g_aConstellationParticles_005c8f60[index].y = (short)(
                    g_pConstellationViewport_005c8f5c->top +
                    RandomInRange(0, height));
                g_aConstellationParticles_005c8f60[index].deltaX =
                    (short)-speed;
                g_aConstellationParticles_005c8f60[index].deltaY = 0;
                g_aConstellationParticles_005c8f60[index].frame =
                    (short)(g_asConstellationFrame_0049cb00[randomIndex] +
                            RandomInRange(0, 3));
            }
            break;
        case 0:
            if (IsConstellationParticleOutsideViewport(
                    g_aConstellationParticles_005c8f60[index]) != 0) {
                randomIndex = RandomInRange(0, 15);
                speed = g_asConstellationVelocity_0049cae0[randomIndex];
                width = (short)(g_pConstellationViewport_005c8f5c->right -
                                g_pConstellationViewport_005c8f5c->left);
                height = (short)(g_pConstellationViewport_005c8f5c->bottom -
                                 g_pConstellationViewport_005c8f5c->top);
                g_aConstellationParticles_005c8f60[index].x = (short)(
                    g_pConstellationViewport_005c8f5c->left +
                    RandomInRange(0, width));
                g_aConstellationParticles_005c8f60[index].y = (short)(
                    g_pConstellationViewport_005c8f5c->top +
                    RandomInRange(0, height));
                middleX = (short)(
                    (g_pConstellationViewport_005c8f5c->right -
                     g_pConstellationViewport_005c8f5c->left) / 2 +
                    g_pConstellationViewport_005c8f5c->left);
                middleY = (short)(
                    (g_pConstellationViewport_005c8f5c->bottom -
                     g_pConstellationViewport_005c8f5c->top) / 2 +
                    g_pConstellationViewport_005c8f5c->top);
                if (g_aConstellationParticles_005c8f60[index].x < middleX &&
                    g_aConstellationParticles_005c8f60[index].y < middleY) {
                    g_aConstellationParticles_005c8f60[index].deltaX =
                        (short)-speed;
                    g_aConstellationParticles_005c8f60[index].deltaY =
                        (short)-speed;
                } else if (
                    g_aConstellationParticles_005c8f60[index].x < middleX &&
                    g_aConstellationParticles_005c8f60[index].y >= middleY) {
                    g_aConstellationParticles_005c8f60[index].deltaX =
                        (short)-speed;
                    g_aConstellationParticles_005c8f60[index].deltaY = speed;
                } else if (
                    g_aConstellationParticles_005c8f60[index].x >= middleX &&
                    g_aConstellationParticles_005c8f60[index].y < middleY) {
                    g_aConstellationParticles_005c8f60[index].deltaX = speed;
                    g_aConstellationParticles_005c8f60[index].deltaY =
                        (short)-speed;
                } else {
                    g_aConstellationParticles_005c8f60[index].deltaX = speed;
                    g_aConstellationParticles_005c8f60[index].deltaY = speed;
                }
                g_aConstellationParticles_005c8f60[index].frame =
                    (short)(g_asConstellationFrame_0049cb00[randomIndex] +
                            RandomInRange(0, 3));
            }
            break;
        case 1:
            if (IsConstellationParticleOutsideViewport(
                    g_aConstellationParticles_005c8f60[index]) != 0) {
                randomIndex = RandomInRange(0, 15);
                speed = g_asConstellationVelocity_0049cae0[randomIndex];
                g_aConstellationParticles_005c8f60[index].deltaX = speed;
                g_aConstellationParticles_005c8f60[index].deltaY = 0;
                g_aConstellationParticles_005c8f60[index].x = (short)(
                    g_pConstellationViewport_005c8f5c->left +
                    RandomInRange(0, speed));
                g_aConstellationParticles_005c8f60[index].y = (short)(
                    g_pConstellationViewport_005c8f5c->top +
                    RandomInRange(0, height));
                g_aConstellationParticles_005c8f60[index].frame =
                    (short)(g_asConstellationFrame_0049cb00[randomIndex] +
                            RandomInRange(0, 3));
            }
            break;
        }
    }
}

/* Function start: 0x44F010 */
short OpenPacketSection(const char *filename, short section,
                        PacketSectionHandle *handle)
{
    unsigned int sectionEntry;
    unsigned int fileSize;
    unsigned int nextEntry;
    int sectionIndex;
    unsigned int directorySize;
    short compression;
    short sectionCount;
    short file;

    file = OpenDataFileOrDie(filename);
    if (file == -1)
        goto failed;
    if (ReadDataFileAtOffset(file, 0, 4, &fileSize) == 0)
        goto failed;
    if (ReadDataFileAtOffset(file, 4, 4, &directorySize) == 0)
        goto failed;
    sectionCount = (short)(directorySize >> 2) - 1;
    if (section >= sectionCount) {
        g_nPacketError_0049ca90 = 3;
        goto failed;
    }
    sectionIndex = (int)section;
    if (ReadDataFileAtOffset(file, sectionIndex * 4 + 4, 4,
                             &sectionEntry) == 0)
        goto failed;
    handle->finalSection = 0;
    compression = (short)(sectionEntry >> 24);
    if (sectionCount - sectionIndex == 1) {
        handle->finalSection = 1;
        nextEntry = fileSize;
        sectionEntry &= 0x00ffffff;
    } else {
        sectionEntry &= 0x00ffffff;
        if (ReadDataFileAtOffset(file, sectionIndex * 4 + 8, 4,
                                 &nextEntry) == 0)
            goto failed;
        nextEntry &= 0x00ffffff;
    }
    fileSize = nextEntry - sectionEntry;
    if (SeekDataFile(file, sectionEntry, 0) != 0) {
        handle->file = file;
        handle->dataOffset = sectionEntry;
        handle->sectionCount = sectionCount;
        handle->compression = compression;
        handle->dataSize = fileSize;
        handle->position = 0;
        return 1;
    }
failed:
    CloseDataFile((unsigned short)file);
    return 0;
}

/* Function start: 0x44F1C7 */
void CloseDataFileByHandle(unsigned short *handle)
{
    CloseDataFile(*handle);
}

/* Function start: 0x43FAC0 */
void show_target_disp(void)
{
    short *maximumArmor;
    short frame;
    short target;
    short armor;
    Viewport targetViewport;
    short x;
    short y;
    ObjectTypeData *typeData;

    DrawTextAt(&g_stRightVduTextContext_005d2ce0,
               g_stRightVduViewport_005d2b20.left,
               g_stRightVduViewport_005d2b20.top,
               g_szEmptyTargetDisplayText_0049b490, 2);
    if (g_bTargetLockMode_00493500 != 0 &&
        g_bDisplayWingmanTargetData_0049347c == 0) {
        DrawFormattedText(
            g_szTextColourStringColourFormat_0049b4a8,
            (unsigned int)g_abGamePaletteReservedColours_0049cb54[8],
            g_szLockedTarget_0049b494,
            (unsigned int)g_ucDefaultTextColour_0049cb7c);
    } else {
        DrawFormattedText(g_szTextColourStringFormat_0049b4c4,
                          (unsigned int)g_ucDefaultTextColour_0049cb7c,
                          g_szAutoTargetting_0049b4b0);
    }
    target = g_acShipTarget_00495f20[0];
    if (target != -1 && g_anShipCloakState_00496020[target] == 1 &&
        (g_asShipCloakElapsedFrames_00496060[target] >= 40 ||
         (g_asShipCloakElapsedFrames_00496060[target] >= 20 &&
          g_anCloakVisibilityPattern_00492710[
              g_asShipCloakElapsedFrames_00496060[target] - 20] == 1)))
        target = -1;
    if (g_bDisplayWingmanTargetData_0049347c != 0)
        target = g_nYourWingman_0049346c;
    if (target != -1 &&
        (g_aeObjectClass_00495328[target] < OBJECT_CLASS_SHIP ||
         g_aeSpecialManeuver_00495600[target] ==
             SPECIAL_MANEUVER_UNKNOWN_9)) {
        g_acShipTarget_00495f20[0] = -1;
        target = g_acShipTarget_00495f20[0];
        g_bDisplayWingmanTargetData_0049347c = 0;
    }
    g_cTargetDisplayObject_004934f4 = (signed char)target;
    DrawFormattedText(g_szTargetLabel_0049b4cc);
    if (target == -1) {
        DrawFormattedText(g_szNoTarget_0049b4d8);
        return;
    }
    typeData = &g_aObjectTypeData_00496d30[g_acObjectType_00493980[target]];
    if (g_asShipIdentified_00496078[target] != 0) {
        DrawFormattedText(g_szIdentifiedTargetNameFormat_0049b4e0,
                          g_apShipMissionRecord_00495da8[target]->name);
    } else {
        DrawFormattedText(g_szShipTargetNameFormat_0049b4ec,
                          g_szUnknownTarget_0049b4e4);
    }
    DrawFormattedText(g_szRangeLabel_0049b4f0);
    InitializeCockpitReadout(1, &g_stRightVduTextContext_005d2ce0);
    if (g_asObjectScreenX_00493598[target] == -32767 &&
        g_bDisplayWingmanTargetData_0049347c == 0) {
        g_cTargetDisplayObject_004934f4 = -1;
        return;
    }
    if (g_asShipIdentified_00496078[target] == 0)
        return;
    x = (short)(g_stRightVduViewport_005d2b20.left + 0x25);
    y = (short)(g_stRightVduViewport_005d2b20.top + 0x26);
    frame = (short)(g_aasShipShield_00495518[target][1] * 6 /
                    typeData->shieldAft);
    frame = (short)((3 - MinShort(frame, 3)) * 2);
    if (frame < 6) {
        DrawSpriteDefault(&g_stRightVduViewport_005d2b20, x, y,
                          g_pCockpitIndicatorShape_005d2c48, frame);
    }
    targetViewport = g_stRightVduViewport_005d2b20;
    maximumArmor = &typeData->armorFront;
    for (armor = 0; armor < 4; armor++) {
        targetViewport.left =
            (short)(g_aTargetArmorClipRects_0049b470[armor].left + x);
        targetViewport.top =
            (short)(g_aTargetArmorClipRects_0049b470[armor].top + y);
        targetViewport.right =
            (short)(g_aTargetArmorClipRects_0049b470[armor].right + x);
        targetViewport.bottom =
            (short)(g_aTargetArmorClipRects_0049b470[armor].bottom + y);
        if (g_aasShipArmor_00495540[target][armor] >
            (short)(maximumArmor[armor] >> 1)) {
            DrawSpriteDefault(&targetViewport, x, y, typeData->shape, 0);
        } else {
            DrawSpriteDefault(&targetViewport, x, y, typeData->shape, 1);
        }
    }
    DrawSpriteDefault(&g_stRightVduViewport_005d2b20, x, y,
                      typeData->shape, 2);
    frame = (short)(g_aasShipShield_00495518[target][0] * 6 /
                    typeData->shieldFore);
    frame = (short)((3 - MinShort(frame, 3)) * 2);
    if (frame < 6) {
        DrawSpriteDefault(&g_stRightVduViewport_005d2b20, x, y,
                          g_pCockpitIndicatorShape_005d2c48,
                          (short)(frame + 1));
    }
}

#pragma function(strcpy)

/* WC2 reads the manoeuvre slot before it has checked the target, so with
 * nothing targeted it reads the two bytes in front of the array.  What lives
 * there is not the blowing-up marker, so the port answers "no manoeuvre" for
 * an unset target rather than reproduce the overrun. */
#ifdef SDL_PORT
#define SPECIAL_MANEUVER_OF(obj) \
    ((obj) != -1 ? g_aeSpecialManeuver_00495600[obj] \
                 : SPECIAL_MANEUVER_NONE)
#else
#define SPECIAL_MANEUVER_OF(obj) g_aeSpecialManeuver_00495600[obj]
#endif

/* Function start: 0x43FF40 */
void DrawTargetRangeReadout(void)
{
    short target;

    target = g_acShipTarget_00495f20[0];
    if (g_bDisplayWingmanTargetData_0049347c != 0)
        target = g_nYourWingman_0049346c;
    if (SPECIAL_MANEUVER_OF(target) == SPECIAL_MANEUVER_UNKNOWN_9) {
        g_acShipTarget_00495f20[0] = -1;
        g_bDisplayWingmanTargetData_0049347c = 0;
        InvalidateVduMode(1);
        return;
    }
    if (target != -1 &&
        g_aeObjectClass_00495328[target] < OBJECT_CLASS_SHIP) {
        g_acShipTarget_00495f20[0] = -1;
        target = g_acShipTarget_00495f20[0];
    }
    if (g_cTargetDisplayObject_004934f4 != target ||
        g_nRenderedSpaceFrame_00493138 % 8 == 0 ||
        (target != -1 && g_anShipCloakState_00496020[target] == 1 &&
         g_asShipCloakElapsedFrames_00496060[target] < 40)) {
        set_new_vdu(1);
        show_target_disp();
    }
    if (target != -1) {
        if (g_asObjectScreenX_00493598[target] != -32767) {
            if ((unsigned short)g_asObjectDistance_00493ae8[target] >
                    30000 ||
                (unsigned short)g_asObjectDistance_00493ae8[target] < 0) {
                strcpy(g_szTextScratchBuffer_005d1c40,
                       g_szTargetTooFar_0049b4fc);
            } else {
                strcpy(g_szTextScratchBuffer_005d1c40 +
                           strlen(_itoa((unsigned short)
                                            g_asObjectDistance_00493ae8[
                                                target],
                                        g_szTextScratchBuffer_005d1c40, 10)),
                       g_szRangeMetresSuffix_0049b504);
            }
        } else {
            strcpy(g_szTextScratchBuffer_005d1c40,
                   g_szTargetOffscreenRange_0049b508);
        }
        if (g_bDisplayWingmanTargetData_0049347c != 0) {
            strcpy(g_szTextScratchBuffer_005d1c40,
                   g_szTargetDataXmit_0049b510);
        }
        DrawCockpitReadout(1, g_szTextScratchBuffer_005d1c40);
        if (g_nTargetLockCountdown_004934ec == 0) {
            if (g_bTargetLockAcquired_004934fc == 1)
                g_bTargetLockAcquired_004934fc = 0;
        } else if (g_bTargetLockReadoutDirty_004934e8 != 0) {
            EraseCockpitReadoutRegion(
                &g_stRightVduViewport_005d2b20,
                g_stRightVduViewport_005d2b20.left,
                (short)(g_stRightVduViewport_005d2b20.bottom - 6),
                g_stRightVduViewport_005d2b20.right,
                g_stRightVduViewport_005d2b20.bottom,
                g_cSecondaryViewBufferColour_0049cb4c);
            g_bTargetLockReadoutDirty_004934e8 = 0;
        }
    }
}

#pragma intrinsic(strcpy)

/* Function start: 0x45CA50 */
void LogDisplayMode(const char *mode)
{
    SystemDebugPrintf("display_mode == '%s'\n", mode);
    ClearDebugPauseFlags();
    PumpMessagesDuringWait();
    exit(1);
}

/* Function start: 0x45CA80 */
short CalcRectangleArea(const Viewport *viewport)
{
    short height;
    short width;

    if (g_nSpacePaletteFadeMode_004901e8 != 0x13)
        LogDisplayMode("not MCGA");
    height = (short)(viewport->bottom - viewport->top + 1);
    width = (short)(viewport->right - viewport->left + 1);
    return (short)(height * width);
}

/* Function start: 0x45CAD2 */
short AllocateViewport(Viewport *viewport,
                       short clearColour, short flags)
{
    unsigned char *pixels;
    unsigned short top;
    short left;
    unsigned short width;
    unsigned short height;
    unsigned short row;
    short offset;

    height = (unsigned short)(viewport->bottom - viewport->top + 1);
    width = (unsigned short)(viewport->right - viewport->left + 1);
    left = viewport->left;
    top = (unsigned short)viewport->top;
    g_nAllocateViewportCalls_005c8dc4++;
    if (g_nSpacePaletteFadeMode_004901e8 != 0x13)
        LogDisplayMode("not MCGA");
    viewport->allocation = AllocateTaggedMemory(
        (unsigned int)width * height, (unsigned short)(flags + 2));
    g_apViewportAllocations_005d19c0[
        g_nViewportAllocationCount_005d19bc++] = viewport->allocation;
    pixels = viewport->allocation;
    if (pixels == 0)
        return 0;
    viewport->pixels = pixels;
    offset = 0;

    viewport->rowOffsets = AllocateTaggedMemory(
        (top + (unsigned int)height) * 2 + 4, 0);
    if (viewport->rowOffsets == 0) {
        if (g_nSpacePaletteFadeMode_004901e8 != 0x13)
            ReleasePacketHandle(viewport->allocation);
        return 0;
    }

    row = 0;
    while ((unsigned int)height + 2 > row) {
        (&viewport->rowOffsets[top])[row] =
            (unsigned short)(offset - left);
        offset = (short)(offset + width);
        row++;
    }
    if (clearColour != -1)
        ClearViewport(viewport, clearColour);
    return 1;
}

/* Function start: 0x46505E */
short IsSpriteFrameOverlappingRect(const ShortRect *rectangle,
                                   short x, short y,
                                   unsigned char *shape,
                                   short frame)
{
    int rectangleTop;
    int rectangleRight;
    int rectangleBottom;
    int frameLeft;
    int rectangleLeft;
    ShortRect frameBounds;
    int frameTop;
    int frameRight;
    int frameBottom;

    GetShapeFrameBounds(&frameBounds.left, x, y, shape, frame);
    rectangleLeft = rectangle->left;
    rectangleTop = rectangle->top;
    rectangleRight = rectangle->right;
    rectangleBottom = rectangle->bottom;
    frameLeft = frameBounds.left;
    frameTop = frameBounds.top;
    frameRight = frameBounds.right;
    frameBottom = frameBounds.bottom;
    if (((frameLeft <= rectangleLeft && rectangleLeft <= frameRight) ||
         (frameLeft <= rectangleRight && rectangleRight <= frameRight))) {
        if (frameTop <= rectangleTop && rectangleTop <= frameBottom)
            return 1;
        if (frameTop <= rectangleBottom && rectangleBottom <= frameBottom)
            return 1;
    }
    if (((rectangleLeft <= frameLeft && frameLeft <= rectangleRight) ||
         (rectangleLeft <= frameRight && frameRight <= rectangleRight))) {
        if (rectangleTop <= frameTop && frameTop <= rectangleBottom)
            return 1;
        if (rectangleTop <= frameBottom && frameBottom <= rectangleBottom)
            return 1;
    }
    return 0;
}

/* Function start: 0x452450 */
/* Drive the mouse cursor from the numeric keypad when keyboard-mouse mode is
 * on: the arrows and corners move it by the current step, NumLock resets the
 * step and +/- change it, Shift doubles it, and Return and Space stand in for
 * the two buttons. */
void KeyboardMousePump(void)
{
    short deltaX;
    short deltaY;
    short buttons;
    short step;
    short x;
    short y;
    short left;
    short right;
    short top;
    short bottom;
    short wasDown;
    short isDown;

    deltaX = 0;
    deltaY = 0;
    buttons = 0;
    if (g_bKeyboardMouseEnabled_0049be68 == 0)
        return;

    step = g_nInputDeviceMode_005c83e6;
    if (GetShiftKeyState() != 0)
        step = (short)(step + step);
    if (GetNumLockKeyState() != 0)
        g_nInputDeviceMode_005c83e6 = 1;
    if (GetNumpadPlusKeyState() != 0 && g_nInputDeviceMode_005c83e6 < 10)
        g_nInputDeviceMode_005c83e6++;
    if (GetNumpadMinusKeyState() != 0 && g_nInputDeviceMode_005c83e6 > 1)
        g_nInputDeviceMode_005c83e6--;
    if (GetReturnKeyState() != 0)
        buttons |= 4;
    if (GetSpaceKeyState() != 0)
        buttons |= 2;
    if (GetLeftArrowKeyState() != 0)
        deltaX = (short)-step;
    if (GetRightArrowKeyState() != 0)
        deltaX = (short)(step + deltaX);
    if (GetUpArrowKeyState() != 0)
        deltaY = (short)-step;
    if (GetDownArrowKeyState() != 0)
        deltaY = (short)(step + deltaY);
    if (GetHomeKeyState() != 0) {
        deltaX = (short)(deltaX - step);
        deltaY = (short)(deltaY - step);
    }
    if (GetPageUpKeyState() != 0) {
        deltaX = (short)(step + deltaX);
        deltaY = (short)(deltaY - step);
    }
    if (GetEndKeyState() != 0) {
        deltaX = (short)(deltaX - step);
        deltaY = (short)(step + deltaY);
    }
    if (GetPageDownKeyState() != 0) {
        deltaX = (short)(step + deltaX);
        deltaY = (short)(step + deltaY);
    }

    if ((deltaX != 0 || deltaY != 0) &&
        g_stScreenViewport_005d21a0.pixels != 0) {
        x = (short)(g_nQueuedInputX_005c83f0 + deltaX);
        y = (short)(g_nQueuedInputY_005c83f2 + deltaY);
        left = g_stScreenViewport_005d21a0.left;
        right = g_stScreenViewport_005d21a0.right;
        top = g_stScreenViewport_005d21a0.top;
        bottom = g_stScreenViewport_005d21a0.bottom;
        if (left > x)
            x = left;
        else if (right < x)
            x = right;
        if (top > y)
            y = top;
        else if (bottom < y)
            y = bottom;
        g_nQueuedInputX_005c83f0 = x;
        g_nPersonnelMouseX_005c8d00 = g_nQueuedInputX_005c83f0;
        g_nQueuedInputY_005c83f2 = y;
        g_nPersonnelMouseY_005c8d02 = g_nQueuedInputY_005c83f2;
        SetPersonnelCursorPosition(g_nQueuedInputX_005c83f0,
                                   g_nQueuedInputY_005c83f2, 0, 0);
        g_bSuppressNextMouseMove_005c843c = 1;
        SetMousePosition(x, y);
    }

    if (buttons == g_nKeyboardMouseButtons_005b3648)
        return;

    wasDown = (short)(g_nKeyboardMouseButtons_005b3648 & 2);
    isDown = (short)(buttons & 2);
    if (wasDown == 0 && isDown != 0) {
        buttons &= 4;
        QueueInputEvent(1, g_nQueuedInputX_005c83f0,
                        g_nQueuedInputY_005c83f2, 0, 1,
                        buttons, 0, 0, 0);
        SetPersonnelCursorPosition(g_nQueuedInputX_005c83f0,
                                   g_nQueuedInputY_005c83f2, 0, 0);
    } else if (wasDown != 0 && isDown == 0) {
        buttons &= 4;
        QueueInputEvent(2, g_nQueuedInputX_005c83f0,
                        g_nQueuedInputY_005c83f2, 0, 0,
                        buttons, 0, 0, 0);
    }

    wasDown = (short)(g_nKeyboardMouseButtons_005b3648 & 4);
    isDown = (short)(buttons & 4);
    if (wasDown == 0 && isDown != 0) {
        buttons &= 2;
        QueueInputEvent(1, g_nQueuedInputX_005c83f0,
                        g_nQueuedInputY_005c83f2, 0,
                        buttons, 1, 0, 0, 0);
        SetPersonnelCursorPosition(g_nQueuedInputX_005c83f0,
                                   g_nQueuedInputY_005c83f2, 0, 0);
    } else if (wasDown != 0 && isDown == 0) {
        buttons &= 2;
        QueueInputEvent(2, g_nQueuedInputX_005c83f0,
                        g_nQueuedInputY_005c83f2, 0,
                        buttons, 0, 0, 0, 0);
    }
    g_nKeyboardMouseButtons_005b3648 = buttons;
}

/* Function start: 0x452AE5 */
void FadeMusic(int duration)
{
}

/* Function start: 0x452A1B */
void SetMusicOn(short enabled)
{
}

/* Function start: 0x452A00 */
void StopMusicStream(void)
{
    Streamer_stop();
}

/* Function start: 0x452A26 */
void StopMusic(int enabled)
{
    g_nCurrentMusicTrack_0049be98 = -1;
    Streamer_stop();
}

/* Function start: 0x452A40 */
short StartMusic(void *music)
{
    SoundDebugPrintf("startMusic %p", music);
#ifdef SDL_PORT
    SdlStartOriginalTitleMusic();
#endif
    return 0;
}

/* Function start: 0x452A64 */
short StartInteractiveMusic(short track)
{
    SoundDebugPrintf("startIntMusic %d\n", (int)track);
    ProcessMusicScriptCommand(track, 1, 0);
    return track;
}

/* Function start: 0x452A10 */
void SetMusBreakpt(int first, int second)
{
}

/* Function start: 0x452A99 */
unsigned int PaletteFadeHook(void)
{
    return 0;
}

/* Function start: 0x452AAB */
void FlushSoundEffect(void)
{
    SoundDebugPrintf("FLUSH_EFFECT");
    stop_all_sounds();
}

/* Function start: 0x452AC8 */
void FlushSoundEffects(void)
{
    SoundDebugPrintf("FLUSH_EFFECTS");
    stop_all_sounds();
}

/* Function start: 0x452AF0 */
void SceneLeaveHook(void)
{
}

/* Function start: 0x402258 */
void RecordMusicCommandHook(int track, int command, int enabled)
{
}

/* Function start: 0x452B03 */
unsigned short ProcessMusicScriptCommand(int track, int command, int enabled)
{
    int streamFlags;
    int streamerCommand;

    if (track == -1)
        return 0;

    streamFlags = g_acMusicTrackStreamFlags_0049beb8[track];
    SoundDebugPrintf("queue_next_music %d %x\n", track, streamFlags);
    if (track >= 0x40)
        streamerCommand = track - 0x2c;
    else
        streamerCommand = track;

    if (command == 4) {
        SoundDebugPrintf("queue_stop\n");
        StopMusic(enabled);
        g_nCurrentMusicTrack_0049be98 = -1;
        return;
    }

#ifdef SDL_PORT
    /* A DOS install has no streams to queue.  Its music is the MUSIC.A00
     * section that carries this very track number. */
    if (SdlUsingOriginFxMusic()) {
        SdlSetOriginFxMusicTrack(track);
        return 1;
    }
#endif
    if ((streamFlags & g_nActiveMusicStreamMask_0049be9c) == 0) {
        if ((streamFlags & 1) != 0) {
            Streamer_open("gameflow.str");
            g_nActiveMusicStreamMask_0049be9c = 1;
        } else if ((streamFlags & 2) != 0) {
            Streamer_open("spaceflt.str");
            g_nActiveMusicStreamMask_0049be9c = 2;
        } else if ((streamFlags & 0x10) != 0) {
            Streamer_open("gametwo.str");
            g_nActiveMusicStreamMask_0049be9c = 0x10;
        } else {
            SoundDebugPrintf("**** unknown stream for track %d\n", track);
            StopMusic(0);
            return;
        }
        SetStreamerIntensity(0x32);
    } else {
        g_nCurrentMusicTrack_0049be98 = track;
    }

    if (g_nActiveMusicStreamMask_0049be9c == 2) {
        if ((streamFlags & 4) != 0) {
            SetStreamerIntensity(streamerCommand);
        } else if ((streamFlags & 8) != 0) {
            SetStreamerTrigger(streamerCommand);
        } else {
            SoundDebugPrintf(
                "unknown int/trig stream for track %d\n", track);
            exit_squadron("unknown track %d", track);
        }
    } else {
        Streamer_stop();
        ForceStreamerTrigger(streamerCommand);
    }

    if (g_nCannedSceneMode_0049021c == 0)
        RecordMusicCommandHook(track, command, enabled);
    return 1;
}

/* Function start: 0x452CF4 */
void spacetrack(int track, int mode, int enabled)
{
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3)
        ProcessMusicScriptCommand(track, mode, enabled);
}

/* Function start: 0x452D32 */
void StopMusicUnlessSuppressed(void)
{
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3)
        StopMusic(0);
}

/* Function start: 0x452D66 */
unsigned short IsMusicTrackComplete(void)
{
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3 &&
        g_nMusicTrackComplete_0049be88 != 0)
        return 1;
    return 0;
}

/* Function start: 0x452DB0 */
void wait_for_end_of_music(void)
{
    if (g_nMusicDriverMode_0049be8c != 0 &&
        g_nMusicDriverMode_0049be8c != 3) {
        if (g_nWaitForMusicEnabled_0049beac == 0) {
            StopMusic(0);
            return;
        }
        SetMusBreakpt(0, 0);
        while (g_nMusicTrackComplete_0049be88 == 0) {
            if (g_bSceneEscapeRequested_0049d4b0 != 0 ||
                WaitForInputKey() != 0) {
                StopMusic(0);
                return;
            }
        }
    }
}

/* Function start: 0x452E46 */
void new_space_music_changes(short attacker, short victim)
{
    if (g_nInFlightMusicActive_0049bf08 != 0) {
        if (g_asShipSide_004955d0[victim] == SIDE_KILRATHI) {
            if (report_kilrathi_rout(1) == 0) {
                spacetrack(10, 1, 0);
                return;
            }
            if (attacker == 0) {
                if (g_acShipPortrait_00495d88[victim] != -1 ||
                    RandomInRange(0, 3) == 0)
                    spacetrack(9, 3, 0);
                else
                    spacetrack(6, 3, 0);
            }
        } else {
            if (g_nYourWingman_0049346c == victim) {
                spacetrack(8, 3, 0);
                return;
            }
            if (g_asShipSide_004955d0[victim] == SIDE_IMPERIAL) {
                if ((g_asShipMissionType_00495de8[0] ==
                         MISSION_TYPE_DEFEND ||
                     g_asShipMissionType_00495de8[0] ==
                         MISSION_TYPE_ESCORT) &&
                    g_asShipMissionIndex_00495d00[victim] ==
                        g_asShipMissionParameter_00495e00[0]) {
                    spacetrack(11, 3, 0);
                    return;
                }
                spacetrack(7, 3, 0);
            }
        }
    }
}

/* Function start: 0x452F90 */
int changetrack(void)
{
    int track;

    switch (g_asShipMissionType_00495de8[0]) {
    case MISSION_TYPE_ESCORT:
        track = 18;
        break;
    case MISSION_TYPE_STRIKE:
        track = 17;
        break;
    case MISSION_TYPE_DEFEND:
    case MISSION_TYPE_RENDEZVOUS:
        track = 16;
        break;
    default:
        track = 15;
        break;
    }
    if (g_aMissionObjectives_004932a8[
            g_cCurrentObjective_004931cc].type == OBJECTIVE_HOME_BASE) {
        if (triumph(0) != 0) {
            if (g_asShipMissionType_00495de8[0] == MISSION_TYPE_PATROL)
                return 13;
            return 14;
        }
        track = 12;
    }
    return track;
}

/* Function start: 0x453085 */
void gametrack(void)
{
    short level;
    int track;

    track = -1;
    if (g_nInFlightMusicActive_0049bf08 == 0)
        return;
    if (g_nCombatMusicActive_0049bf04 != 0) {
        if ((g_nSpaceFrame_00493134 & 0xf) == 0 ||
            g_nMusicTrackComplete_0049be88 != 0) {
            if (g_nInitialFlightMusicPending_0049bf00 != 0)
                g_nInitialFlightMusicPending_0049bf00 = 0;
            if (FindMissileTargetingObject(0) != 0) {
                track = 3;
            } else if (detect_enemy_tail(0) != -1) {
                track = 1;
            } else if (is_ship_tailing_player_target(0) != 0) {
                track = 2;
            } else {
                level = (short)calculate_damage_level();
                if (level >= 2)
                    track = 4;
                else if (level == 1)
                    track = 5;
                else
                    track = 0;
            }
            if (g_nMissionMusicTrackOverride_00496144 != 0)
                track = g_nMissionMusicTrackOverride_00496144;
            if (report_kilrathi_rout(1) == 0)
                g_nCombatMusicActive_0049bf04 = 0;
        }
    } else if ((g_nSpaceFrame_00493134 & 0xf) == 0 ||
               g_nMusicTrackComplete_0049be88 != 0) {
        if (g_bJumpSequenceActive_004962f0 != 0)
            track = 0x2f;
        else
            track = changetrack();
        if (report_kilrathi_rout(2) != 0)
            g_nCombatMusicActive_0049bf04 = 1;
    }
    spacetrack(track, 1, 0);
}

/* Function start: 0x453240 */
void servicetrack(void)
{
    short volume;
    short object;
    FixedVector travel;
    FixedVector futurePosition;

    if (g_nMusicDriverMode_0049be8c == 0)
        return;
    gametrack();
    if (g_bFlightSoundEffectsEnabled_0049beb0 != 0) {
        for (object = 0; object < SPACE_OBJECT_COUNT;
             object++) {
            if (g_nPassingShipSoundObject_0049bf10 == object &&
                g_aeObjectClass_00495328[object] < OBJECT_CLASS_SHIP)
                g_nPassingShipSoundObject_0049bf10 = -1;
            switch (g_aeObjectClass_00495328[object]) {
            case OBJECT_CLASS_SHIP:
            case OBJECT_CLASS_CAPITAL_SHIP:
            case OBJECT_CLASS_BASE:
                if (g_asObjectScreenX_00493598[object] == -32767)
                    break;
                if ((unsigned short)g_asObjectDistance_00493ae8[object] >= 0x384)
                    break;
                if (g_nPassingShipSoundObject_0049bf10 == -1) {
                    ScaleFixedVector(&g_aShipVelocity_00494898[object], 0x1400,
                                     &travel);
                    AddFixedVectors(&g_aShipPosition_00494550[object], &travel,
                                    &futurePosition);
                    ComputeVectorDelta(
                        &g_aShipPosition_00494550[EYE_OBJECT],
                        &futurePosition, &travel);
                    ComputeVectorDelta(
                        &g_aShipPosition_00494550[EYE_OBJECT],
                        &g_aShipPosition_00494550[object], &futurePosition);
                    if (dot_product(&travel, &futurePosition) < 0xdd) {
                        if (object == 0 && g_bJumpSequenceActive_004962f0 != 0)
                            continue;
                        g_nPassingShipSoundObject_0049bf10 = object;
                        g_nPassingShipSoundCountdown_0049bf14 = 6;
                        volume = 12;
                        g_abSoundEffectDefinitions_0049bf18[0xa] =
                            (unsigned char)volume;
                        *(short *)&g_abSoundEffectDefinitions_0049bf18[0xc] = 2;
                        g_abSoundEffectDefinitions_0049bf18[0xe] = 0x70;
                        if (g_nSpaceFrame_00493134 >
                            g_nPassingShipSoundCooldown_005d156c) {
                            g_nPassingShipSoundCooldown_005d156c =
                                g_nSpaceFrame_00493134 + 24;
                            PlaySfxWaveFileByNumber(2, object, 0);
                        }
                    }
                } else if (g_nPassingShipSoundObject_0049bf10 == object) {
                    g_nPassingShipSoundCountdown_0049bf14--;
                    if (g_nPassingShipSoundCountdown_0049bf14 == 0)
                        g_nPassingShipSoundObject_0049bf10 = -1;
                }
                break;
            }
        }
    }
}

/* Function start: 0x453496 */
void ResetSoundState(void)
{
    FlushSoundEffects();
    g_bAfterburnerSfxActive_005d3864 = 0;
    g_nCriticalDamageWarningSfxHandle_005d1ec0 = 0;
}

/* Function start: 0x4534BA */
void ResetSoundStateForScene(void)
{
    ResetSoundState();
    g_bFlightSoundEffectsEnabled_0049beb0 = 0;
}

/* Function start: 0x4534D3 */
void ResetSoundStateForFlight(void)
{
}

/* Function start: 0x4534FC */
void EnableMusicForScene(void)
{
    g_nWaitForMusicEnabled_0049beac = 1;
    SetMusicOn(1);
}

/* Function start: 0x45351A */
int LogUnknownSoundEffect(const unsigned char *definition,
                          int sourceObject, int pan, int volume,
                          int looping, int priority)
{
    SoundDebugPrintf("Playing SFX <UNKNOWN>\n");
    return 0;
}

/* Function start: 0x45357E */
void FlushSoundEffectsAndLog()
{
    FlushSoundEffect();
    SoundDebugPrintf("flushFX\n");
    return;
}

/* Function start: 0x4535BB */
void sound_effect(signed char soundNumber, short sourceObject, short looping)
{
    short volume;
    short effectHandle = 0;
    int distance;
    FixedVector delta;
    int stereoOffset;
    short sampleNumber;
    short pan;

    SoundDebugPrintf("Playing SFX #%d\n", soundNumber);
    return;
}

/* Function start: 0x45373B */
void PlaySfxWaveFileByNumber(int soundNumber, int sourceObject, int looping)
{
    FixedVector delta;
    int magnitude;
    int distance;

#ifdef SDL_PORT
    /* A DOS install ships no sfx waves; the effect is synthesized from the
     * OriginFX record this number selects. */
    if (SdlUsingOriginFxSoundEffects()) {
        SdlPlayGameSoundEffect(soundNumber, sourceObject, looping);
        return;
    }
#endif
    if (sourceObject != -1) {
        ComputeVectorDelta(&g_aShipPosition_00494550[EYE_OBJECT],
                           &g_aShipPosition_00494550[sourceObject],
                           &delta);
        magnitude = Vector_magnitude(&delta);
        distance = magnitude;
        if (distance > 32000)
            distance = 32000;
        g_aiSoundEffectSourceActive_005d12c0[sourceObject] = 1;
    } else {
        distance = 72000;
    }
    if (distance < 10)
        return;
    soundNumber--;
    SoundDebugPrintf(
        "Playing SFX #%d on obj: %d with volume of %d\n",
        soundNumber, sourceObject, distance);
    sprintf(g_szSfxWavePath_005b3650, "sfx%02i.wav", soundNumber);
    playWAVE(g_szSfxWavePath_005b3650, looping, distance);
}
