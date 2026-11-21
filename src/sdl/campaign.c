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

/* Cloaked ships. A cloaked ship decloaks only when it fires (fire,
 * 0x414DC0), and nothing else ends a cloak: one that stops attacking, or never
 * gets a shot at an evading player, stays invisible for the rest of the
 * mission while autopilot still counts it as near (issue #9). After a minute
 * cloaked it decloaks as firing would, through ResetShipCloakState so that the
 * flight recorder sees it, then waits half a minute before cloaking again.
 * In the flight harness nearly every cloak ends by firing within 400 frames. */
#define SDL_CLOAK_LIMIT_FRAMES 1200
#define SDL_CLOAK_RECHARGE_FRAMES 600

static short g_asSdlCloakRecharge[10];

/* house_keep_objects, each frame, for the AI's ships 1-9. */
void SdlLimitShipCloak(short ship)
{
    if (g_asSdlCloakRecharge[ship] > 0)
        g_asSdlCloakRecharge[ship]--;
    if (g_anShipCloakState[ship] == 1 &&
        g_asShipCloakElapsedFrames[ship] >= SDL_CLOAK_LIMIT_FRAMES) {
        ResetShipCloakState(ship);
        g_asSdlCloakRecharge[ship] = SDL_CLOAK_RECHARGE_FRAMES;
    }
}

int SdlShipCloakRecharged(short ship)
{
    return ship < 0 || ship >= (short)SDL_arraysize(g_asSdlCloakRecharge) ||
           g_asSdlCloakRecharge[ship] == 0;
}

/* Set_up_ship_info, for a new ship in the slot. */
void SdlResetShipCloakRecharge(short ship)
{
    if (ship >= 0 && ship < (short)SDL_arraysize(g_asSdlCloakRecharge))
        g_asSdlCloakRecharge[ship] = 0;
}

/* Objectives that bring the flight path back to a nav point. The objective
 * sweep (update_cockpit) marks any objective the player passes within 1500 m
 * of as visited, current or not, and set_next_destination later skips it. A
 * flight path that leaves a nav point and comes back to it, as SO1 series 1
 * mission 4's return to Rigel's jump point does, so lost its way home on the
 * way out: after Nav1 the nav computer pointed at the other system and
 * autopilot answered "Jump Point" (issue #9). Such a later visible nav-point
 * objective, still ahead on the flight path, with another nav point between it
 * and the earlier visible objective at the same nav point, is not visited by
 * passing it. */
int SdlObjectiveIsPendingReturn(short objective)
{
    short nav;
    int index;
    int earlier;
    int elsewhere;

    if (g_aMissionObjectives[objective].type != 0 ||
        hidden_objective(objective) != 0)
        return 0;
    nav = g_aMissionObjectives[objective].index;
    for (index = g_cCurrentNavPointIndex + 1;
         index < g_cMissionObjectiveCount && g_abFlightPath[index] != -1; index++) {
        if (g_abFlightPath[index] != objective)
            continue;
        elsewhere = 0;
        for (earlier = index - 1; earlier >= 0; earlier--) {
            short other = g_abFlightPath[earlier];

            if (g_aMissionObjectives[other].type != 0 || hidden_objective(other) != 0)
                continue;
            if (g_aMissionObjectives[other].index != nav)
                elsewhere = 1;
            else if (elsewhere)
                return 1;
        }
    }
    return 0;
}

/* Scenes queued for a nav point's fight. A wave command queues a mid-flight
 * scene to play once the last Kilrathi of the sphere is gone, but the scene
 * stayed queued after the player left, and the next sphere cleared of
 * Kilrathi played it: SO2 series 2 mission 3's bomb at Nav3 killed a player
 * who had heeded the warning and then cleared Nav1 (issue #9). The scene now
 * arms only in the sphere that queued it; it stays queued meanwhile, so a
 * player who returns and finishes that fight still gets it, as the missions
 * that end the flight with scene 117 need. */
static short g_nSdlQueuedSceneNavPoint = -1;

/* ProcessMissionWaveCommands, for each scene command. Modes 0-3 and 5 queue
 * the scene; 4 plays it at once and leaves any scene queued before alone. */
void SdlNoteQueuedSceneNavPoint(int mode)
{
    if (mode >= 0 && mode <= 5 && mode != 4)
        g_nSdlQueuedSceneNavPoint = g_nCurrentNavPoint;
}

/* ProcessEnemyWaveCompletion, before arming the queued scene. */
int SdlQueuedSceneIsHere(void)
{
    return g_nSdlQueuedSceneNavPoint == -1 ||
           g_nSdlQueuedSceneNavPoint == g_nCurrentNavPoint;
}

/* new_sphere_shapes frees the resource slots of types the next nav point or
 * wave does not preload, while a ship can still be in the sphere on one: a
 * ship travelling with the player, such as WC2's Kamekh, a survivor of the
 * last wave, or one still dying.  The ship reaches its type only through the
 * slot's index, so it takes the records of whatever type is loaded there
 * next, and get_right_shape loads a capital ship's views from that type's
 * file.  Given get_shape_slot's choice, prefer a free slot that no object
 * uses; keep the choice when every free slot is used. */
short SdlUnclaimedShapeSlot(short slot)
{
    short candidate;
    short obj;

    for (candidate = 0; candidate < 5; candidate++) {
        if (g_aObjectResourceSlots[candidate].resourceType != -1)
            continue;
        for (obj = 0; obj < SPACE_OBJECT_COUNT; obj++) {
            if (g_aeObjectClass[obj] >= OBJECT_CLASS_MISSILE &&
                g_acObjectType[obj] == candidate)
                break;
        }
        if (obj == SPACE_OBJECT_COUNT)
            return candidate;
    }
    return slot;
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
        /* SO1.exe 0x41E9A4 opens sector.001, which DOS data lacks; the
         * Kilrathi Saga copy reads "Vega", but SO1 is set in Enigma. */
        { "sector.000", "sector.000", "sector.002" },
        /* The player's radio lines: SO1.exe and SO2.exe change one taunt. */
        { "communic.you", "communic.yu2", "communic.yu3" }
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
        "MUSIC.A01", "COMMUNIC.YU2"
    };
    static const char *const so2Required[] = {
        "CAMPAIGN.S02", "GAMEFLOW.S02", "MODULE.002", "SERIES.S02",
        "STARTGLB.002", "ALTSHIPS.002", "INCIDENT.S02", "SECTOR.002",
        "SHIP.V41", "PCSHIP.V41", "SHIP.V48", "NUKE.VGA",
        "MUSIC.A01", "MUSIC.A02", "COMMUNIC.YU3", "FIELD.V02"
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
        /* An SO1 pilot is already a colonel, rank 5. */
        if (valid && pilot.rank >= 0 && pilot.rank < PILOT_RANK_COUNT)
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
    /* Both expansions make a transferred pilot a colonel, as they do a new
     * one (SO1.exe 0x404549, SO2.exe 0x466BCD). */
    pilot.rank = 5;
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
