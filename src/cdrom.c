/*
 *  CD-ROM location and disc-swap prompting.
 *
 *  Address range 0x403100-0x4034ff (provisional -- see docs/ORDER.md).
 *  Boundary evidence: LocateStreamsDirOnDisc..PromptInsertCorrectCd; string band 0x46535C-0x4653FC.
 */
#include "game.h"

/* Function start: 0x456123 */
char *LocateStreamsDirOnDisc(void)
{
#ifdef SDL_PORT
    char current[4096];
    const char *directory;
    int length;

    if (GetCurrentDirectoryA(sizeof(current), current) == 0)
        return 0;
    /* Only the final path component identifies GAMEDAT. Its spelling may
     * use any case, and an ancestor may also contain the word "gamedat". */
    directory = current + strlen(current);
    while (directory > current && directory[-1] != '/' &&
           directory[-1] != '\\')
        directory--;
    length = snprintf(g_szStreamsPath_005b3740,
                      sizeof(g_szStreamsPath_005b3740), "%s%s", current,
                      SDL_strcasecmp(directory, "gamedat") == 0
                          ? "/../streams/" : "/streams/");
    if (length < 0 || (size_t)length >= sizeof(g_szStreamsPath_005b3740))
        return 0;
    if (!SetCurrentDirectoryA(g_szStreamsPath_005b3740))
        return 0;
    if (!SetCurrentDirectoryA(current))
        return 0;
    return g_szStreamsPath_005b3740;
#else
    char current[256];
    char *result;
    char drive;

    result = g_szStreamsPath_005b3740;
    GetCurrentDirectoryA(0xff, current);
    drive = FindCdRomDriveByVolumeLabel("<anydisc>", "\\wc2\\streams\\");
    if (drive != 0) {
        sprintf(result, "%c:%s", drive,
                "\\wc2\\streams\\");
        return result;
    }

    if (strstr(current, "gamedat") != 0)
        sprintf(result, "%s\\..\\%s", current, "streams\\");
    else
        sprintf(result, "%s\\%s", current, "streams\\");
    GetCurrentDirectoryA(0xff, current);
    if (!SetCurrentDirectoryA(result))
        result = 0;
    SetCurrentDirectoryA(current);
    return result;
#endif
}

#pragma function(strcmp)

/* Function start: 0x456236 */
char FindCdRomDriveByVolumeLabel(const char *label,
                                 const char *directory)
{
    char volume[256];
    char root[12];
    char filesystem[64];
    DWORD flags;
    DWORD maximumComponentLength;
    int driveCount;
    char drives[26];
    char scanRoot[12];
    int drive;
    char result;

    driveCount = 0;
    for (drive = 'a'; drive <= 'z'; drive++) {
        sprintf(scanRoot, "%c:\\", (char)drive);
        if (GetDriveTypeA(scanRoot) == DRIVE_CDROM) {
            drives[driveCount] = (char)drive;
            driveCount++;
        }
    }

    result = 0;
    for (drive = 0; drive < driveCount; drive++) {
        sprintf(root, "%c:\\", drives[drive]);
        GetVolumeInformationA(root, volume, 0xff, 0,
                              &maximumComponentLength, &flags,
                              filesystem, sizeof(filesystem));
        if (strcmp(label, "<anydisc>") == 0) {
            if (SetCurrentDirOnDrive(drives[drive], directory) != 0) {
                result = drives[drive];
                break;
            }
        } else if (strcmp(volume, label) == 0) {
            if (SetCurrentDirOnDrive(drives[drive], directory) != 0) {
                result = drives[drive];
                break;
            }
        }
    }
    return result;
}

#pragma intrinsic(strcmp)

/* Function start: 0x4563A3 */
int SetCurrentDirOnDrive(char drive, const char *directory)
{
    char path[256];
    char current[256];
    int result;

    result = 0;
    GetCurrentDirectoryA(0xff, current);
    sprintf(path, "%c:%s", drive, directory);
    if (SetCurrentDirectoryA(path) != 0)
        result = 1;
    SetCurrentDirectoryA(current);
    return result;
}

/* Function start: 0x456443 */
int PromptInsertCorrectCd(void)
{
    int disc;
    char title[18];
    char message[1024];

    disc = 1;
    strcpy(title, "Insert Correct CD");
    sprintf(message,
            "Please place The Kilrathi Saga disc %d into your CD-ROM drive and click OK\n"
            "or click cancel to quit",
            disc);
    while (LocateStreamsDirOnDisc() == 0) {
        if (MessageBoxA(0, message, title,
                        MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
            return 0;
    }
    return 1;
}

/* Function start: 0x45A010 */
short OpenDataFileOrDie(const char *path)
{
    short fd = (short)_open(path, 0x8002);

    g_nPacketError_0049ca90 = (short)errno;
    return fd;
}
