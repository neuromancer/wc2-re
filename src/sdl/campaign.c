#include "game.h"

int g_nSdlCampaign;
/* SO2.exe's flag at 0x49B2B0: the campaign script set arcade state 4, so a
 * scene has already shown the player's death. */
short g_bSdlScriptedDeath;

/* SO1.exe 0x4509AB, 0x450A36 and 0x450AF9 (identical in SO2.exe): a Confed
 * ship with comm portrait 28, Thrakhath in SO1 series 4 mission 3, is out of
 * contact while it flies to its warp point. The expansions index the tables
 * without checking the ship number; outside them nothing is departing. */
/* SO2.exe 0x4389C5: capital ships and bases load 37 views, but SO2's Ayer's
 * Rock (resource 48) has 62. The 25 beyond the usual set follow the four
 * records in sections 37-40, in sections 41-65. */
int SdlShipViewCount(short resourceType)
{
    return g_nSdlCampaign == 2 && resourceType == 0x30 ? 0x3e : 0x25;
}

int SdlIsDepartingCommContact(short ship)
{
    return g_nSdlCampaign != 0 && ship >= 0 &&
           ship < (short)SDL_arraysize(g_acShipPortrait) &&
           g_acShipPortrait[ship] == 0x1c &&
           g_asShipSide[ship] == SIDE_IMPERIAL &&
           g_asShipMissionType[ship] == MISSION_TYPE_GOTO_WARP;
}

/* Both expansions' STARTGLB files use campaign slot zero, like the base game.
 * Select their files at the host boundary rather than changing the serialized
 * campaign globals or the retail engine's campaign numbering. Only bare game
 * filenames are redirected; explicit paths (including pilot imports) are not.
 * Keep saves and temporary state separate from the base campaign. */
const char *SdlCampaignPath(const char *path)
{
    static const char *const names[][3] = {
        { "campaign.s00", "campaign.s01", "campaign.s02" },
        { "module.000", "module.001", "module.002" },
        { "altships.000", "altships.001", "altships.002" },
        { "startglb.000", "startglb.001", "startglb.002" },
        { "tempglob.000", "tempglob.001", "tempglob.002" },
        { "savegame.wc2", "savegame.so1", "savegame.so2" },
        { "sector.000", "sector.000", "sector.002" }
    };
    size_t index;

    if (g_nSdlCampaign >= 1 && g_nSdlCampaign <= 2 && path != 0) {
        for (index = 0; index < SDL_arraysize(names); index++) {
            if (SDL_strcasecmp(path, names[index][0]) == 0)
                return names[index][g_nSdlCampaign];
        }
    }
    return path;
}

static int SdlPrepareCampaignFiles(int campaign, const char *directory)
{
    static const char *const so1Required[] = {
        "CAMPAIGN.S01", "GAMEFLOW.S01", "MODULE.001", "SERIES.S01",
        "STARTGLB.001", "ALTSHIPS.001", "SHIP.V15", "PCSHIP.V15",
        "MUSIC.A01"
    };
    static const char *const so2Required[] = {
        "CAMPAIGN.S02", "GAMEFLOW.S02", "MODULE.002", "SERIES.S02",
        "STARTGLB.002", "ALTSHIPS.002", "INCIDENT.S02", "SECTOR.002",
        "SHIP.V41", "PCSHIP.V41", "SHIP.V48", "NUKE.VGA",
        "MUSIC.A01", "MUSIC.A02"
    };
    const char *const *required;
    size_t requiredCount;
    size_t index;
    int file;
    unsigned char *initialState;
    size_t initialSize;
    char resolved[4096];
    char path[64];
    char temporary[64];

    if (campaign < 0 || campaign > 2)
        return 0;
    if (campaign != 0) {
        required = campaign == 1 ? so1Required : so2Required;
        requiredCount = campaign == 1 ? SDL_arraysize(so1Required) :
                                       SDL_arraysize(so2Required);
        for (index = 0; index < requiredCount; index++) {
            snprintf(path, sizeof(path), "%s/%s", directory, required[index]);
            file = _open(path, 0x8000);
            if (file < 0) {
                fprintf(stderr, "Special Operations %d requires %s.\n",
                        campaign, path);
                return 0;
            }
            _close(file);
        }
    }
    /* All campaigns need scratch state before the menu checks Continue.
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

int SdlTryImportExpansionPilot(void)
{
    static const char *const sources[] = {
        "./savegame.so1", "./savegame.wc2"
    };
    PilotProfile pilot;
    unsigned short words;
    short occupied;
    int file;
    int slotBytes;
    int reply;
    int source;
    int valid;
    const char *question;
    char signature[36];
    char prompt[52];

    if (g_nSdlCampaign != 1 && g_nSdlCampaign != 2)
        return 0;
    file = _open("startglb.000", 0x8000);
    if (file < 0)
        return 0;
    reply = _read(file, &words, sizeof(words));
    _close(file);
    if (reply != sizeof(words) || words < sizeof(CampaignGlobals) / 2)
        return 0;
    slotBytes = words * 2 + 0x86;
    /* Explicit paths bypass campaign redirection. For SO2, prefer the SO1
     * Continue checkpoint; fall back to WC2 if SO1 has no valid checkpoint. */
    for (source = g_nSdlCampaign == 2 ? 0 : 1; source < 2; source++) {
        file = _open(sources[source], 0x8000);
        if (file < 0)
            continue;
        valid = _filelength(file) == slotBytes * 9 &&
            _lseek(file, slotBytes * 8, SEEK_SET) == slotBytes * 8 &&
            _read(file, &occupied, sizeof(occupied)) == sizeof(occupied) && occupied &&
            _read(file, &pilot, sizeof(pilot)) == sizeof(pilot) &&
            _read(file, signature, sizeof(signature)) == sizeof(signature) &&
            memcmp(signature, "Wing Commander II (c) 1991 Origin", 31) == 0;
        _close(file);
        if (valid && pilot.rank >= 0 && pilot.rank < 5)
            break;
    }
    if (source == 2)
        return 0;
    pilot.firstName[sizeof(pilot.firstName) - 1] = 0;
    pilot.lastName[sizeof(pilot.lastName) - 1] = 0;
    pilot.callsign[sizeof(pilot.callsign) - 1] = 0;
    if (g_nSdlCampaign == 1)
        question = _("Transfer WC2 pilot %s to SO1? (Y/N)");
    else if (source == 0)
        question = _("Transfer SO1 pilot %s to SO2? (Y/N)");
    else
        question = _("Transfer WC2 pilot %s to SO2? (Y/N)");
    /* ShowModalTextPanel also has a 52-byte formatting buffer. */
    snprintf(prompt, sizeof(prompt), question, pilot.callsign);
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
