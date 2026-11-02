/*
 *  Speech packets and speech cache.
 *
 *  Retail object 0x44F300-0x44FA6F (link order; see docs/ORDER.md).
 */
#include "game.h"

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
            (unsigned int)g_wDigiSampleBytes_0048e0e0, 0x44);
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
    if (packetSize > g_wDigiSampleBytes_0048e0e0)
        return;
    g_nDigiLastBlock_0049bb78 = 0;
    g_wDigiLeft_005d1714 = packetSize;
    g_pDigiData_005d1710 = packet;
    g_wDigiPosition_005d171c = 0;
    g_wDigiPlaying_0049bb60++;
    g_nDmaHalfSelect_0049bb74 = 0;
    if (g_wDmaHalfBytes_0048e0e4 * 2 > g_wDigiLeft_005d1714) {
        if (g_wDmaHalfBytes_0048e0e4 > g_wDigiLeft_005d1714) {
            g_wDmaCountA_0049bb6c = g_wDigiLeft_005d1714;
            g_wDmaCountB_0049bb70 = 0;
        } else {
            g_wDmaCountA_0049bb6c = g_wDmaHalfBytes_0048e0e4;
            g_wDmaCountB_0049bb70 =
                g_wDigiLeft_005d1714 - g_wDmaHalfBytes_0048e0e4;
        }
    } else {
        g_wDmaCountA_0049bb6c = g_wDmaCountB_0049bb70 =
            g_wDmaHalfBytes_0048e0e4;
    }
    src = (void *)IdentityDword((DwordPtr)g_pDigiData_005d1710);
    memcpy(g_pDmaBuffer_005d1718, src, g_wDmaHalfBytes_0048e0e4 * 2);
    g_wDigiLeft_005d1714 -= g_wDmaCountA_0049bb6c + g_wDmaCountB_0049bb70;
    g_wDigiPosition_005d171c += g_wDmaCountA_0049bb6c + g_wDmaCountB_0049bb70;
    return;
}

/* Function start: 0x44F52E */
void LoadAndPlaySpeechPacket(char *fileName, short section)
{
    unsigned int packetSize;

    packetSize = GetNamedPacketSize(fileName, section);
    if (LoadSpeechPacketIntoBuffer(
            fileName, section, g_pDigiSample_005d1720) != 0) {
        PlaySpeechPacketBuffer(
            g_pDigiSample_005d1720, packetSize, 0);
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
    if (g_nEmsIsrBusy_005c841c == 0)
        SoundBlasterOut(0x20, 0x20);
}

/* Function start: 0x44F5B1 */
/* Marks the digitised sample as finished. */
void StopDigiPlayback(void)
{
    g_wDigiPlaying_0049bb60 = 0;
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
    SoundBlasterIn(g_wSoundBlasterPort_0049bb58 + 0xe);
    if (g_nDigiLastBlock_0049bb78 != 0 || g_wDigiPlaying_0049bb60 == 0) {
        StopDigiPlayback();
        SoundBlasterEndOfInterrupt();
        return;
    }
    if (g_wDigiLeft_005d1714 == 0)
        g_nDigiLastBlock_0049bb78++;
    g_wDigiPlaying_0049bb60++;
    if (g_nDmaHalfSelect_0049bb74 == 0) {
        g_nDmaHalfSelect_0049bb74 = 1;
        dmaAddress = g_wDmaAddressB_0049bb68;
        total = g_wDmaCountB_0049bb70;
        offset = 0;
    } else {
        g_nDmaHalfSelect_0049bb74 = 0;
        dmaAddress = g_wDmaAddressA_0049bb64;
        total = g_wDmaCountA_0049bb6c;
        offset = g_wDmaHalfBytes_0048e0e4;
    }
    SoundBlasterOut(0x0a, 5);
    SoundBlasterOut(0x0c, 0);
    SoundBlasterOut(0x0b, 0x49);
    SoundBlasterOut(2, (unsigned char)dmaAddress);
    SoundBlasterOut(2, (unsigned char)(dmaAddress >> 8));
    SoundBlasterOut(0x83, g_ucDmaPage_005d1724);
    SoundBlasterOut(3, (unsigned char)total);
    SoundBlasterOut(3, (unsigned char)(total >> 8));
    SoundBlasterOut(0x0a, 1);
    SoundBlasterWaitWrite();
    SoundBlasterOut(g_wSoundBlasterPort_0049bb58 + 0xc, 0x14);
    SoundBlasterWaitWrite();
    SoundBlasterOut(g_wSoundBlasterPort_0049bb58 + 0xc,
                    (unsigned char)(total - 1));
    SoundBlasterWaitWrite();
    SoundBlasterOut(g_wSoundBlasterPort_0049bb58 + 0xc,
                    (unsigned char)((total - 1) >> 8));
    resolved = (unsigned char *)IdentityDword((DwordPtr)g_pDigiData_005d1710);
    src = g_wDigiPosition_005d171c + resolved;
    dmaHalf = offset + (unsigned char *)g_pDmaBuffer_005d1718;
    if (g_nDmaHalfSelect_0049bb74 == 0)
        slot = &g_wDmaCountB_0049bb70;
    else
        slot = &g_wDmaCountA_0049bb6c;
    *slot = g_wDmaHalfBytes_0048e0e4 < g_wDigiLeft_005d1714 ?
                g_wDmaHalfBytes_0048e0e4 : g_wDigiLeft_005d1714;
    g_wDigiLeft_005d1714 -= *slot;
    g_wDigiPosition_005d171c += *slot;
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

    g_pDigiSample_005d1720 =
        AllocateTaggedMemory((unsigned int)g_wDigiSampleBytes_0048e0e0,
                             0x44);
    if (g_pDigiSample_005d1720 == 0)
        return 1;
    codeAddress = IdentityDword((DwordPtr)g_pDigiSample_005d1720);
    codeLimit = (unsigned int)g_pDigiSample_005d1720 +
        (unsigned int)g_wDigiSampleBytes_0048e0e0;
    if (codeLimit < codeAddress)
        ReportFatalErrorCode("027");
    g_wSoundBlasterPort_0049bb58 =
        (unsigned short)(((unsigned short)port << 4) + 0x200);
    g_wSoundBlasterIrq_0049bb5c = (unsigned short)irq;
    cacheMetadata = AllocateTaggedMemory(0x0c, 0x40);
    if (cacheMetadata == 0)
        return 2;
    g_pDmaBuffer_005d1718 =
        AllocateTaggedMemory(
            (unsigned int)g_wDmaHalfBytes_0048e0e4 * 2 + 0x0f,
            0x42);
    if (g_pDmaBuffer_005d1718 == 0)
        return 1;
    oldBuffer = 0;
    do {
        dmaAddress = (unsigned int)g_pDmaBuffer_005d1718;
        dmaEnd = dmaAddress +
            ((int)((unsigned int)g_wDmaHalfBytes_0048e0e4 * 2) /
             16);
        if ((dmaEnd & 0xfffff000) != (dmaAddress & 0xfffff000)) {
            oldBuffer =
                g_pDmaBuffer_005d1718;
            g_pDmaBuffer_005d1718 =
                AllocateTaggedMemory(
                    (unsigned int)g_wDmaHalfBytes_0048e0e4 * 2 +
                        0x0f,
                    0x42);
            if (g_pDmaBuffer_005d1718 == 0)
                return 1;
            ReleasePacketHandle(oldBuffer);
        } else {
            oldBuffer = 0;
        }
        PumpWindowMessages(0);
    } while (oldBuffer != 0);
    g_ucDmaPage_005d1724 = (unsigned char)(dmaAddress >> 12);
    dmaAddress -= (unsigned int)g_ucDmaPage_005d1724 << 12;
    g_wDmaAddressA_0049bb64 = (unsigned short)(dmaAddress << 4);
    g_wDmaAddressB_0049bb68 =
        (unsigned short)(g_wDmaHalfBytes_0048e0e4 +
                         g_wDmaAddressA_0049bb64);
    return 0;
}

/* Function start: 0x44FA31 */
void ShutdownSoundBlaster(void)
{
    g_wDigiPlaying_0049bb60 = 0;
    FreePacketAndClear(&g_pDmaBuffer_005d1718, 0x40);
    FreePacketAndClear(&g_pDigiSample_005d1720, 0x44);
}
