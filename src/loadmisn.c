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
           itoa((int)g_nSelectedCampaignSlot_005d3bf2,
                 campaignSlot, 10));

    packet = FetchDiskPacketRetrying(fileName, 0, 0);
    position = series * 0x60;
    position += mission * 0x18;
    source = packet + position;
    DosMemcpy(&g_stMissionHeader_005d3e70, source, 0x18);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(fileName, 1, 0);
    position = series * 0xfc8;
    position += mission * 0x3f2;
    source = packet + position;
    DosMemcpy(g_aMissionNavPoints_00491e98, source, 0x3f2);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(fileName, 2, 0);
    position = series * 0x800;
    position += mission * 0x200;
    source = packet + position;
    DosMemcpy(g_aMissionObjectiveSources_005d3c70, source, 0x200);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(fileName, 3, 0);
    position = series * 0xf00;
    position += mission * 0x3c0;
    source = packet + position;
    DosMemcpy(g_aMissionShips_00492290, source, 0x3c0);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(fileName, 4, 0);
    position = series * 0xa0;
    position += mission * 0x28;
    source = packet + position;
    DosMemcpy(g_abMissionAuxData_005d3c10, source, 0x28);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(fileName, 5, 0);
    position = series * 0x28;
    source = packet + position;
    DosMemcpy(g_abSeriesAuxData_005d3c40, source, 0x28);
    ReleasePacketHandle(packet);

    packet = FetchDiskPacketRetrying(fileName, 6, 0);
    position = series * 0x378;
    position += mission * 0xde;
    source = packet + position;
    DosMemcpy(g_abMissionBriefingData_005d3e90, source, 0xde);
    ReleasePacketHandle(packet);
}
