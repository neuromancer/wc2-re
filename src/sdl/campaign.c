#include "game.h"

/* The native build selects this implementation instead of narrowing a host
 * find handle to the reconstructed DOS-width local in personnel.c. */
unsigned short HasSavedPilotCampaign(void)
{
    int campaignBytes;
    struct _finddata_t findData;
    short occupied;
    intptr_t findHandle;
    short file;

    LoadTemporaryCampaignGlobals();
    campaignBytes = (unsigned short)g_pCampaignGlobals->wordCount * 2;
    ReleasePacketSlot((void **)&g_pCampaignGlobals);
    findHandle = _findfirst("savegame.wc2", &findData);
    if (findHandle == -1)
        return 0;
    _findclose(findHandle);
    if (campaignBytes * 9 + 0x4b6 != findData.size)
        return 0;
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
