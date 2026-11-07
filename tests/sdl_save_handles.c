#include "game.h"

static CampaignGlobals campaign;
static CampaignGlobals *testCampaign;
static intptr_t foundHandle;
static intptr_t closedHandle;
static long foundSize;
static short occupied;
static int findCloses;
static int fileOpens;
static int fileCloses;

static void TestLoadCampaign(void)
{
    campaign.wordCount = 4;
    testCampaign = &campaign;
}

static void TestReleasePacket(void **slot)
{
    *slot = 0;
}

static intptr_t TestFindFirst(const char *pattern, struct _finddata_t *data)
{
    if (strcmp(pattern, "savegame.wc2") != 0)
        abort();
    data->size = foundSize;
    return foundHandle;
}

static int TestFindClose(intptr_t handle)
{
    closedHandle = handle;
    findCloses++;
    return 0;
}

static int TestOpen(const char *path, int flags)
{
    (void)path;
    (void)flags;
    fileOpens++;
    return 7;
}

static long TestSeek(int file, long offset, int origin)
{
    (void)file;
    (void)origin;
    return offset;
}

static int TestRead(int file, void *destination, unsigned int size)
{
    (void)file;
    if (size != sizeof(occupied))
        abort();
    memcpy(destination, &occupied, size);
    return size;
}

static int TestClose(int file)
{
    (void)file;
    fileCloses++;
    return 0;
}

/* Compile the shipped implementation with synthetic external dependencies,
 * so no retail campaign packet or save is required. */
#undef _findfirst
#undef _findclose
#undef _open
#undef _lseek
#undef _read
#undef _close
#define _findfirst TestFindFirst
#define _findclose TestFindClose
#define _open TestOpen
#define _lseek TestSeek
#define _read TestRead
#define _close TestClose
#define LoadTemporaryCampaignGlobals TestLoadCampaign
#define ReleasePacketSlot TestReleasePacket
#define g_pCampaignGlobals testCampaign
#define HasSavedPilotCampaign TestHasSavedPilotCampaign
#include "../src/sdl/campaign.c"

static int CheckHandle(intptr_t handle, long size, short saved, int expected)
{
    int result;

    foundHandle = handle;
    foundSize = size;
    occupied = saved;
    closedHandle = -1;
    findCloses = fileOpens = fileCloses = 0;
    result = TestHasSavedPilotCampaign();
    if (result != expected || testCampaign != 0 ||
        findCloses != (handle != -1) ||
        (handle != -1 && closedHandle != handle) ||
        fileOpens != (handle != -1 && size == 8 * 9 + 0x4b6) ||
        fileCloses != fileOpens) {
        fprintf(stderr, "Save find handle lost or closed incorrectly\n");
        return 1;
    }
    return 0;
}

int main(void)
{
    intptr_t highHandle;
    const long validSize = 8 * 9 + 0x4b6;

    highHandle = (intptr_t)0x1234ffff;
#if INTPTR_MAX > INT32_MAX
    /* Catch both the short local and the LLP64 long cast. Low 16 bits are
     * deliberately -1 in the second case, despite a valid host handle. */
    highHandle |= (intptr_t)0x12345678 << 32;
#endif
    return CheckHandle(highHandle - 1, validSize, 1, 1) ||
           CheckHandle(highHandle, validSize, 1, 1) ||
           CheckHandle(0, validSize, 1, 1) ||
           CheckHandle(-1, validSize, 1, 0) ||
           CheckHandle(highHandle, 0, 1, 0) ||
           CheckHandle(highHandle, validSize, 0, 0);
}
