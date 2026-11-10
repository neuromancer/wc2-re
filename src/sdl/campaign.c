#include "game.h"

int g_nSdlCampaign;

/* SO1's STARTGLB.001 uses campaign slot zero, just like the base game.
 * Select its files at the host boundary rather than changing the serialized
 * campaign globals or the retail engine's campaign numbering. Only bare game
 * filenames are redirected; explicit paths (including pilot imports) are not.
 * Keep saves and temporary state separate from the base campaign. */
const char *SdlCampaignPath(const char *path)
{
    static const char *const names[][2] = {
        { "campaign.s00", "campaign.s01" },
        { "module.000", "module.001" },
        { "altships.000", "altships.001" },
        { "startglb.000", "startglb.001" },
        { "tempglob.000", "tempglob.001" },
        { "savegame.wc2", "savegame.so1" }
    };
    size_t index;

    if (g_nSdlCampaign == 1 && path != 0) {
        for (index = 0; index < SDL_arraysize(names); index++) {
            if (SDL_strcasecmp(path, names[index][0]) == 0)
                return names[index][1];
        }
    }
    return path;
}

static int SdlPrepareCampaignFiles(int campaign, const char *directory)
{
    static const char *const required[] = {
        "CAMPAIGN.S01", "GAMEFLOW.S01", "MODULE.001", "SERIES.S01",
        "STARTGLB.001", "ALTSHIPS.001", "SHIP.V15", "PCSHIP.V15",
        "MUSIC.A01"
    };
    size_t index;
    int file;
    unsigned char *initialState;
    size_t initialSize;
    char resolved[4096];
    char path[64];
    char temporary[64];

    if (campaign < 0 || campaign > 1)
        return 0;
    if (campaign == 1) {
        for (index = 0; index < SDL_arraysize(required); index++) {
            snprintf(path, sizeof(path), "%s/%s", directory, required[index]);
            file = _open(path, 0x8000);
            if (file < 0) {
                fprintf(stderr, "Special Operations 1 requires %s.\n",
                        path);
                return 0;
            }
            _close(file);
        }
    }
    /* Both campaigns need scratch state before the menu checks Continue.
     * Explicit paths bypass the active campaign's filename redirection. */
    snprintf(temporary, sizeof(temporary), "%s/TEMPGLOB.%03d", directory, campaign);
    file = _open(temporary, 0x8000);
    if (file >= 0) {
        _close(file);
    } else {
        snprintf(path, sizeof(path), "%s/STARTGLB.%03d", directory, campaign);
        if (!SdlResolvePath(path, resolved, sizeof(resolved)))
            return 0;
        initialState = SDL_LoadFile(resolved, &initialSize);
        if (initialState == 0) {
            fprintf(stderr, "Unable to read %s: %s\n", path, SDL_GetError());
            return 0;
        }
        file = _open(temporary, 0x8501, 0x180);
        if (file < 0 || _write(file, initialState, initialSize) != initialSize) {
            fprintf(stderr, "Unable to initialize %s.\n", temporary);
            if (file >= 0) {
                _close(file);
                _unlink(temporary);
            }
            SDL_free(initialState);
            return 0;
        }
        _close(file);
        SDL_free(initialState);
    }
    return 1;
}

int SdlSelectCampaign(int campaign)
{
    if (!SdlPrepareCampaignFiles(campaign, "GAMEDAT"))
        return 0;
    g_nSdlCampaign = campaign;
    return 1;
}

int SdlConfirmContinueOverwrite(void)
{
    int confirmed;
    int savedCursorDepth;
    int reply;

    confirmed = 0;
    savedCursorDepth = g_nMouseCursorDrawDepth;
    DisableMouseCursorDrawing();
    SdlSuspendMouseGrab();
    if (ShowModalTextPanel(1, _("Overwrite Continue save? (Y/N)")) != 0) {
        /* Consume the press that selected New Game before accepting an
         * answer. Enter and mouse clicks never authorize an overwrite. */
        FlushPendingInputPresses();
        FlushInputEvents();
        do {
            reply = toupper((unsigned char)WaitForQueuedInputPress());
        } while (reply != 'Y' && reply != 'N' && reply != 0x1b);
        confirmed = reply == 'Y';
        FlushPendingInputPresses();
        FlushInputEvents();
        ReleaseModalTextPanel();
    }
    SdlResumeMouseGrab();
    g_nMouseCursorDrawDepth = savedCursorDepth;
    return confirmed;
}

int SdlTryImportSo1Pilot(void)
{
    PilotProfile pilot;
    unsigned short words;
    short occupied;
    int file;
    int slotBytes;
    int reply;
    char signature[36];
    char prompt[128];

    if (g_nSdlCampaign != 1)
        return 0;
    file = _open("startglb.001", 0x8000);
    if (file < 0)
        return 0;
    reply = _read(file, &words, sizeof(words));
    _close(file);
    if (reply != sizeof(words) || words < sizeof(CampaignGlobals) / 2)
        return 0;
    slotBytes = words * 2 + 0x86;
    /* An explicit relative path bypasses SO1's save filename selection. */
    file = _open("./savegame.wc2", 0x8000);
    if (file < 0)
        return 0;
    if (_filelength(file) != slotBytes * 9 ||
        _lseek(file, slotBytes * 8, SEEK_SET) != slotBytes * 8 ||
        _read(file, &occupied, sizeof(occupied)) != sizeof(occupied) || !occupied ||
        _read(file, &pilot, sizeof(pilot)) != sizeof(pilot) ||
        _read(file, signature, sizeof(signature)) != sizeof(signature) ||
        memcmp(signature, "Wing Commander II (c) 1991 Origin", 31) != 0) {
        _close(file);
        return 0;
    }
    _close(file);
    pilot.firstName[sizeof(pilot.firstName) - 1] = 0;
    pilot.lastName[sizeof(pilot.lastName) - 1] = 0;
    pilot.callsign[sizeof(pilot.callsign) - 1] = 0;
    if (pilot.rank < 0 || pilot.rank >= 5)
        return 0;
    snprintf(prompt, sizeof(prompt), _("Transfer WC2 pilot %s to SO1? (Y/N)"), pilot.callsign);
    if (!ShowModalTextPanel(1, "%s", prompt))
        return 0;
    FlushPendingInputPresses();
    FlushInputEvents();
    do {
        reply = toupper((unsigned char)WaitForQueuedInputPress());
    } while (reply != 'Y' && reply != 'N' && reply != 0x1b);
    FlushPendingInputPresses();
    FlushInputEvents();
    ReleaseModalTextPanel();
    if (reply != 'Y')
        return 0;

    LoadStartingCampaignGlobals(0);
    pilot.series = g_pCampaignGlobals->series;
    pilot.mission = g_pCampaignGlobals->mission;
    pilot.missionScore = 0;
    pilot.playerShipType = g_stDefaultPilotProfile.playerShipType;
    g_stCurrentPilotProfile = pilot;
    SDL_strlcpy(g_stPilotNameVariables.firstName, pilot.firstName,
                sizeof(g_stPilotNameVariables.firstName));
    SDL_strlcpy(g_stPilotNameVariables.lastName, pilot.lastName,
                sizeof(g_stPilotNameVariables.lastName));
    SDL_strlcpy(g_stPilotNameVariables.callsign, pilot.callsign,
                sizeof(g_stPilotNameVariables.callsign));
    SaveAndFreeTemporaryCampaignGlobals();
    OpenDiskDataFile(1);
    return 1;
}

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
