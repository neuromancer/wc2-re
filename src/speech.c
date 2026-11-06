/*
 *  Speech packets and speech cache.
 *
 *  Retail object 0x44F300-0x44FA6F (link order; see docs/ORDER.md).
 */
#define SPEECH_C
#include "game.h"

const int g_nDigiSampleBytes = 0xfffa; /* 0x0048e0e0 */
const unsigned short g_wDmaHalfBytes = 0x0fa0; /* 0x0048e0e4 */

unsigned short g_wSoundBlasterPort = 0x220; /* 0x0049bb58 */
unsigned short g_wSoundBlasterIrq = 7; /* 0x0049bb5c */
unsigned short g_wDigiPlaying = 0; /* 0x0049bb60 */
unsigned short g_wDmaAddressA = 0; /* 0x0049bb64 */
unsigned short g_wDmaAddressB = 0; /* 0x0049bb68 */
unsigned short g_wDmaCountA = 0; /* 0x0049bb6c */
unsigned short g_wDmaCountB = 0; /* 0x0049bb70 */
int g_nDmaHalfSelect = 0; /* 0x0049bb74 */
short g_nDigiLastBlock = 0; /* 0x0049bb78 */

/* Function start: 0x44F300 */
/* The DOS release wrote the Sound Blaster's I/O ports here; the Win32 port
 * keeps the entry point and does nothing. */
void SoundBlasterOut(unsigned short port, unsigned char value)
{
    return;
}

/* Function start: 0x44F310 */
/* Port read counterpart of SoundBlasterOut; always reads 0. */
unsigned char SoundBlasterIn(unsigned short port)
{
    return 0;
}

/* Function start: 0x44F322 */
/* Waited for the DSP's write buffer to empty under DOS; a no-op on Win32. */
void SoundBlasterWaitWrite(void)
{
    return;
}

/* Function start: 0x44F332 */
void *LoadSpeechPacketIntoBuffer(char *fileName, short section,
                                 void *destination)
{
    void *packet;

    packet = destination;
    if (packet == 0) {
        packet = AllocateTaggedMemory(
            (unsigned short)g_nDigiSampleBytes, 0x44);
    }
    if (packet == 0)
        return 0;
    if (LoadPacketIntoBuffer(fileName, section, packet, 1) == 0)
        return 0;
    return packet;
}

#pragma function(memcpy)
/* Function start: 0x44F3A4 */
/* Plays a loaded speech packet through the Win32 speech player.  The DOS
 * DMA set-up that followed survives behind the early return, without its
 * port writes. */
void PlaySpeechPacketBuffer(void *packet, unsigned short packetSize,
                            int interrupt)
{
    void *src;

    PlayRawSpeechBuffer(packet, packetSize, interrupt);
    return;
    if (packetSize > (unsigned short)g_nDigiSampleBytes)
        return;
    g_nDigiLastBlock = 0;
    g_wDigiLeft = packetSize;
    g_4_pDigiData = packet;
    g_2_wDigiPosition = 0;
    g_wDigiPlaying++;
    g_nDmaHalfSelect = 0;
    if (g_wDmaHalfBytes * 2 > g_wDigiLeft) {
        if (g_wDmaHalfBytes > g_wDigiLeft) {
            g_wDmaCountA = g_wDigiLeft;
            g_wDmaCountB = 0;
        } else {
            g_wDmaCountA = g_wDmaHalfBytes;
            g_wDmaCountB =
                g_wDigiLeft - g_wDmaHalfBytes;
        }
    } else {
        g_wDmaCountA = g_wDmaCountB =
            g_wDmaHalfBytes;
    }
    src = (void *)IdentityDword((DwordPtr)g_4_pDigiData);
    memcpy(g_4_pDmaBufferPointer, src, g_wDmaHalfBytes * 2);
    g_wDigiLeft -= g_wDmaCountA + g_wDmaCountB;
    g_2_wDigiPosition += g_wDmaCountA + g_wDmaCountB;
    return;
}

/* Function start: 0x44F52E */
void LoadAndPlaySpeechPacket(char *fileName, short section)
{
    unsigned int packetSize;

    packetSize = GetNamedPacketSize(fileName, section);
    if (LoadSpeechPacketIntoBuffer(
            fileName, section, g_pDigiSample) != 0) {
        PlaySpeechPacketBuffer(
            g_pDigiSample, packetSize, 0);
    }
}

/* Function start: 0x44F581 */
/* Nothing in the shipped executable calls this.  The DOS end-of-interrupt
 * for the Sound Blaster IRQ, disabled by the Win32 port: it returns before
 * restoring the EMS mapping and acknowledging the interrupt controller. */
void SoundBlasterEndOfInterrupt(void)
{
    return;
    EmsRestoreMapping();
    if (g_105_nEmsIsrBusyValue == 0)
        SoundBlasterOut(0x20, 0x20);
}

/* Function start: 0x44F5B1 */
/* Marks the digitised sample as finished. */
void StopDigiPlayback(void)
{
    g_wDigiPlaying = 0;
}

/* Function start: 0x44F5C5 */
/* Nothing in the shipped executable calls this.  The DOS Sound Blaster DMA
 * interrupt handler, disabled by the Win32 port: it returns before
 * acknowledging the DSP, programming the other DMA half and refilling the
 * half just played from the sample. */
void SoundBlasterDspInterrupt(void)
{
    unsigned char *dmaHalf;
    unsigned char *src;
    unsigned char *resolved;
    unsigned short dmaAddress;
    unsigned short total;
    unsigned short *slot;
    unsigned short offset;

    return;
    EmsSaveMapping();
    SoundBlasterIn(g_wSoundBlasterPort + 0xe);
    if (g_nDigiLastBlock != 0 || g_wDigiPlaying == 0) {
        StopDigiPlayback();
        SoundBlasterEndOfInterrupt();
        return;
    }
    if (g_wDigiLeft == 0)
        g_nDigiLastBlock++;
    g_wDigiPlaying++;
    if (g_nDmaHalfSelect == 0) {
        g_nDmaHalfSelect = 1;
        dmaAddress = g_wDmaAddressB;
        total = g_wDmaCountB;
        offset = 0;
    } else {
        g_nDmaHalfSelect = 0;
        dmaAddress = g_wDmaAddressA;
        total = g_wDmaCountA;
        offset = g_wDmaHalfBytes;
    }
    SoundBlasterOut(0x0a, 5);
    SoundBlasterOut(0x0c, 0);
    SoundBlasterOut(0x0b, 0x49);
    SoundBlasterOut(2, (unsigned char)dmaAddress);
    SoundBlasterOut(2, (unsigned char)(dmaAddress >> 8));
    SoundBlasterOut(0x83, g_1_ucDmaPageValue);
    SoundBlasterOut(3, (unsigned char)total);
    SoundBlasterOut(3, (unsigned char)(total >> 8));
    SoundBlasterOut(0x0a, 1);
    SoundBlasterWaitWrite();
    SoundBlasterOut(g_wSoundBlasterPort + 0xc, 0x14);
    SoundBlasterWaitWrite();
    SoundBlasterOut(g_wSoundBlasterPort + 0xc,
                    (unsigned char)(total - 1));
    SoundBlasterWaitWrite();
    SoundBlasterOut(g_wSoundBlasterPort + 0xc,
                    (unsigned char)((total - 1) >> 8));
    resolved = (unsigned char *)IdentityDword((DwordPtr)g_4_pDigiData);
    src = g_2_wDigiPosition + resolved;
    dmaHalf = offset + (unsigned char *)g_4_pDmaBufferPointer;
    if (g_nDmaHalfSelect == 0)
        slot = &g_wDmaCountB;
    else
        slot = &g_wDmaCountA;
    *slot = g_wDmaHalfBytes < g_wDigiLeft ?
                g_wDmaHalfBytes : g_wDigiLeft;
    g_wDigiLeft -= *slot;
    g_2_wDigiPosition += *slot;
    memcpy(dmaHalf, src, *slot);
    SoundBlasterEndOfInterrupt();
    return;
}

/* Function start: 0x44F84F */
short InitializeSoundBlaster(short port, short irq)
{
    void *cacheMetadata;
    unsigned int codeAddress;
    unsigned int dmaEnd;
    unsigned int codeLimit;
    unsigned int dataSize;
    void *oldBuffer;
    unsigned int dmaAddress;

    g_pDigiSample =
        AllocateTaggedMemory((unsigned short)g_nDigiSampleBytes,
                             0x44);
    if (g_pDigiSample == 0)
        return 1;
    codeAddress = IdentityDword((DwordPtr)g_pDigiSample);
    codeLimit = (unsigned int)g_pDigiSample +
        (unsigned short)g_nDigiSampleBytes;
    if (codeLimit < codeAddress)
        ReportFatalErrorCode("027");
    g_wSoundBlasterPort =
        (unsigned short)(((unsigned short)port << 4) + 0x200);
    g_wSoundBlasterIrq = (unsigned short)irq;
    cacheMetadata = AllocateTaggedMemory(0x0c, 0x40);
    if (cacheMetadata == 0)
        return 2;
    g_4_pDmaBufferPointer =
        AllocateTaggedMemory(
            (unsigned int)g_wDmaHalfBytes * 2 + 0x0f,
            0x42);
    if (g_4_pDmaBufferPointer == 0)
        return 1;
    oldBuffer = 0;
    do {
        dmaAddress = (unsigned int)g_4_pDmaBufferPointer;
        dmaEnd = dmaAddress +
            ((int)((unsigned int)g_wDmaHalfBytes * 2) /
             16);
        if ((dmaEnd >> 12) != (dmaAddress >> 12)) {
            oldBuffer =
                g_4_pDmaBufferPointer;
            g_4_pDmaBufferPointer =
                AllocateTaggedMemory(
                    (unsigned int)g_wDmaHalfBytes * 2 +
                        0x0f,
                    0x42);
            if (g_4_pDmaBufferPointer == 0)
                return 1;
            ReleasePacketHandle(oldBuffer);
        } else {
            oldBuffer = 0;
        }
        PumpWindowMessages(0);
    } while (oldBuffer != 0);
    g_1_ucDmaPageValue = (unsigned char)(dmaAddress >> 12);
    dmaAddress -= (unsigned int)g_1_ucDmaPageValue << 12;
    g_wDmaAddressA = (unsigned short)(dmaAddress << 4);
    g_wDmaAddressB =
        (unsigned short)(g_wDmaHalfBytes +
                         g_wDmaAddressA);
    return 0;
}

/* Function start: 0x44FA31 */
void ShutdownSoundBlaster(void)
{
    g_wDigiPlaying = 0;
    FreePacketAndClear(&g_4_pDmaBufferPointer, 0x40);
    FreePacketAndClear(&g_pDigiSample, 0x44);
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
void *g_4_pDmaBufferPointer; /* 0x005d1718 */
unsigned short g_2_wDigiPosition; /* 0x005d171c */
void *g_pDigiSample; /* 0x005d1720 */
unsigned char g_1_ucDmaPageValue; /* 0x005d1724 */
