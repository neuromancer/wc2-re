/*
 *  Loads mission data.
 *
 *  Retail object 0x4401C0-0x44048F (link order; see docs/ORDER.md).
 */
#include "game.h"

#pragma function(strcat)
/* Function start: 0x4401C0 */
void LoadMissionData(short series, short mission)
{
    unsigned char *packet;
    unsigned char *source;
    int position;
    char campaignSlot[4];
    char fileName[13] = "module.00";

    strcat(fileName,
           itoa((int)g_nSelectedCampaignSlot,
                 campaignSlot, 10));

    packet = FetchDiskPacketRetrying(fileName, 0, 0);
    position = series * 0x60;
    position += mission * 0x18;
    source = packet + position;
    DosMemmove(&g_13_stMissionHeaderBlock, source, 0x18);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(fileName, 1, 0);
    position = series * 0xfc8;
    position += mission * 0x3f2;
    source = packet + position;
    DosMemmove(g_aMissionNavPoints, source, 0x3f2);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(fileName, 2, 0);
    position = series * 0x800;
    position += mission * 0x200;
    source = packet + position;
    DosMemmove(g_69_aMissionObjectiveSources, source, 0x200);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(fileName, 3, 0);
    position = series * 0xf00;
    position += mission * 0x3c0;
    source = packet + position;
    DosMemmove(g_aMissionShips, source, 0x3c0);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(fileName, 4, 0);
    position = series * 0xa0;
    position += mission * 0x28;
    source = packet + position;
    DosMemmove(g_5_abMissionAuxBlockBytes, source, 0x28);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(fileName, 5, 0);
    position = series * 0x28;
    source = packet + position;
    DosMemmove(g_abSeriesAuxData, source, 0x28);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(fileName, 6, 0);
    position = series * 0x378;
    position += mission * 0xde;
    source = packet + position;
    DosMemmove(g_abMissionBriefingData, source, 0xde);
    ReleasePacketHandle(packet);
}
