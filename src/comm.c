/*
 *  Communications menu and transmissions.
 *
 *  Retail object 0x447300-0x44856F (link order; see docs/ORDER.md).
 */
#define COMM_C
#include "game.h"

short g_nEnemyTauntCommandBase = 20; /* 0x0049b76c */
short g_nCommMenuChoiceCount = -1; /* 0x0049b770 */
short g_nCommMenuReuseMode = 0; /* 0x0049b774 */
signed char g_cMessageSpeed = 2; /* 0x0049b778 */
signed char g_cPendingCommMenuAction = 1; /* 0x0049b77c */
signed char g_bRadioSilence = 0; /* 0x0049b780 */
signed char g_bVideoImagesSuppressed = 0; /* 0x0049b784 */
unsigned char *g_pCommPortraitResource = 0; /* 0x0049b788 */
unsigned char *g_pCommVduFrameResource = 0; /* 0x0049b78c */
signed char g_cCommMenuRecipient = -1; /* 0x0049b790 */
short g_nCommSpeakerObject = 0; /* 0x0049b794 */
short g_nCommSpeakerRating = 0; /* 0x0049b798 */
short g_nCommPortraitIndex = -1; /* 0x0049b79c */
short g_bCommSpeechPlaying = 0; /* 0x0049b7a0 */

/* Function start: 0x447300 */
void LoadCommPortraitResources(short portrait)
{
    if (g_19_bStoredHighMemoryEnabled == 0)
        return;
    FreePacketAndClear(&g_pCommPortraitResource, 4);
#ifdef SDL_PORT
    /* SO1.exe 0x450719 (and SO2.exe): the expansions' portraits come from
     * vdu.v01, Bear's (10) from section 3 and those from 28 on from section
     * portrait - 28. Section 28 of vdu.v00 is the frame drawn around them. */
    if (g_nSdlCampaign != 0 && portrait == 10)
        g_pCommPortraitResource =
            FetchDiskPacketRetrying("vdu.v01", 3, 4);
    else if (g_nSdlCampaign != 0 && portrait > 27)
        g_pCommPortraitResource =
            FetchDiskPacketRetrying("vdu.v01", (short)(portrait - 28), 4);
    else
#endif
    g_pCommPortraitResource =
        FetchDiskPacketRetrying("vdu.v00", portrait, 4);
    if (g_pCommVduFrameResource == 0) {
        g_pCommVduFrameResource =
            FetchDiskPacketRetrying("vdu.v00", 0x1c, 4);
    }
}

/* Function start: 0x447369 */
void ResetCommMenuChoices(short reuse)
{
    short choice;

    if (reuse == 0) {
        for (choice = 0; choice < 7; choice++) {
            g_1_apszCommMenuChoiceTextTable[choice] = 0;
            g_abCommMenuChoiceCommand[choice] = -1;
        }
    }
    g_nCommMenuChoiceCount = 0;
    g_nCommMenuReuseMode = reuse;
}

/* Function start: 0x4473D1 */
short IsCommMenuIdle(void)
{
    return g_nCommMenuChoiceCount == 0;
}

/* Function start: 0x4473FC */
void AppendCommMenuChoice(const char *text, short command)
{
    if (g_nCommMenuReuseMode == 1 &&
        (g_1_apszCommMenuChoiceTextTable[
             g_nCommMenuChoiceCount] != text ||
         g_abCommMenuChoiceCommand[
             g_nCommMenuChoiceCount] != command))
        g_nCommMenuReuseMode = 0;
    g_1_apszCommMenuChoiceTextTable[
        g_nCommMenuChoiceCount] = text;
    g_abCommMenuChoiceCommand[
        g_nCommMenuChoiceCount] = (signed char)command;
    g_nCommMenuChoiceCount++;
}

/* Function start: 0x447479 */
void SendCommMenuChoice(short i)
{
    if (g_48_apCommunicationTextPacketsTable[i] == 0)
        g_48_apCommunicationTextPacketsTable[i] =
            LoadPacketAllocated("communic.you", i);
    AppendCommMenuChoice(g_48_apCommunicationTextPacketsTable[i], i);
}

/* Function start: 0x4474CA */
void OpenCommMenuForTarget(const char *heading, const char *message)
{
    CockpitMessage(message, g_cYellowColour, -1);
    g_pszCommScreenHeading = heading;
}

/* Function start: 0x4474F4 */
short IsCommChoiceMenuOpen(void)
{
    return get_mode(1) == 4;
}

/* Function start: 0x447526 */
void OpenCommRecipientMenu(void)
{
    push_mode(1, 4);
    g_cPendingCommMenuAction = 1;
}

/* Function start: 0x447544 */
void CloseCommChoiceMenu(void)
{
    if (get_mode(1) == 4)
        pop_mode(1);
}

/* Function start: 0x44756F */
short wingman_dead(void)
{
#ifdef SDL_PORT
    /* SO1.exe 0x4509AB: a wingman leaving with comm portrait 28 is out of
     * contact too. */
    if (SdlIsDepartingCommContact(g_nYourWingman))
        return 1;
#endif
    return g_nYourWingman == -1;
}

/* Function start: 0x44759B */
short have_target(void)
{
#ifdef SDL_PORT
    /* SO1.exe 0x450A36: so is such a target. */
    if (SdlIsDepartingCommContact(g_acShipTarget[0]))
        return 0;
#endif
    return unactive(g_acShipTarget[0]) == 0;
}

/* Function start: 0x4475D3 */
short CanOpenCommMenu(void)
{
    return have_target() || !wingman_dead();
}

/* Function start: 0x44760F */
void SelectCommRecipient(short recipient)
{
#ifdef SDL_PORT
    /* SO1.exe 0x450AF9: and the menu cannot select one. */
    if (SdlIsDepartingCommContact(recipient))
        return;
#endif
    g_cCommMenuRecipient = recipient;
    g_cPendingCommMenuAction = 2;
}

/* Function start: 0x447629 */
void BuildCommunicationRecipientMenu(void)
{
    short target;

    ResetCommMenuChoices(g_nCommMenuReuseMode);
    OpenCommMenuForTarget(_("VID-COM SYSTEM\n\nSend message to?\n\n"),
                          _("SELECT"));
    if (wingman_dead() != 0) {
        SelectCommRecipient(g_acShipTarget[0]);
        return;
    }
    if (have_target() == 0 ||
        g_acShipTarget[0] == g_nYourWingman) {
        SelectCommRecipient(g_nYourWingman);
        return;
    }
    AppendCommMenuChoice(
        g_apShipMissionRecord[g_nYourWingman]->name,
        1);
    target = g_acShipTarget[0];
    if (target != -1) {
        if (g_asShipSide[target] == SIDE_KILRATHI &&
            g_aeObjectClass[target] >= OBJECT_CLASS_SHIP) {
            if (g_asShipIdentified[target] != 0)
                AppendCommMenuChoice(_("ENEMY TARGET"), 2);
            else
                AppendCommMenuChoice(
                    _("UNIDENTIFIED"), 2);
        } else if (g_asShipSide[target] == SIDE_IMPERIAL) {
            if (g_aeObjectClass[target] >= OBJECT_CLASS_SHIP &&
                g_asShipMissionType[target] ==
                    MISSION_TYPE_CANNED_SEQUENCE)
                AppendCommMenuChoice(g_apShipMissionRecord[
                                         g_acShipTarget[0]]->name,
                                     3);
            if ((g_aeObjectClass[target] == OBJECT_CLASS_SHIP &&
                 any_enemy(0, 14000) != 0) ||
                g_asShipMissionIndex[target] ==
                    g_6_nHomeOperationShipIndexValue) {
                if (g_asShipIdentified[target] != 0)
                    AppendCommMenuChoice(g_apShipMissionRecord[
                                             g_acShipTarget[0]]->name,
                                         3);
                else
                    AppendCommMenuChoice(
                        _("UNIDENTIFIED"), 3);
            } else if (g_ucPendingEjectionTransition != 0xff &&
                       g_bEjectionAwaitingCommCommand != 0 &&
                       g_asObjectType[target] >=
                           OBJECT_TYPE_DEBRIS_GLASS) {
                AppendCommMenuChoice(
                    g_apShipMissionRecord[
                        g_acShipTarget[0]]->name,
                    3);
            }
        }
    }
    SendCommMenuChoice(0);
}

#pragma function(strcat, strcpy)
/* Function start: 0x447890 */
void BuildCommunicationCommandMenu(void)
{
    short object;

    if (g_nEnemyCommPilotIndex != -1 &&
        g_acShipPortrait[g_cCommMenuRecipient] ==
            g_nEnemyCommPilotIndex &&
        g_asShipSide[g_cCommMenuRecipient] ==
            SIDE_KILRATHI) {
        g_nEnemyTauntCommandBase =
            (g_nEnemyTauntCommandBase & 1) |
            g_1_nEnemyCommCommandBase;
    } else {
        g_nEnemyTauntCommandBase =
            (g_nEnemyTauntCommandBase & 1) | 20;
    }
    ResetCommMenuChoices(g_nCommMenuReuseMode);
    if (g_asShipIdentified[g_cCommMenuRecipient] == 0) {
        SendCommMenuChoice(10);
    } else {
        if (g_cCommMenuRecipient == g_nYourWingman ||
            (g_aeObjectClass[g_cCommMenuRecipient] ==
                 OBJECT_CLASS_SHIP &&
             g_asShipSide[g_cCommMenuRecipient] ==
                 SIDE_IMPERIAL)) {
            if (g_aeShipObjective[
                    g_cCommMenuRecipient] ==
                    OBJECTIVE_HOLD_FORMATION &&
                any_enemy(0, 14000) != 0)
                SendCommMenuChoice(4);
            if (have_target() != 0 &&
                g_asShipSide[g_acShipTarget[0]] ==
                    SIDE_KILRATHI)
                SendCommMenuChoice(1);
            for (object = 1; object < 10; object++) {
                if (g_aeObjectClass[object] ==
                        OBJECT_CLASS_SHIP &&
                    g_acShipTarget[object] == 0) {
                    get_facing_range_from_object(0, object);
                    if (g_nFacingToTarget < -70 &&
                        g_nTargetFacing > 90 &&
                        g_nTargetRange < 4000) {
                        SendCommMenuChoice(2);
                        break;
                    }
                }
            }
            if (g_cCommMenuRecipient ==
                g_nYourWingman) {
                if (g_nAutoEngageTimer != -1)
                    SendCommMenuChoice(5);
                else if (g_aeShipObjective[
                             g_nYourWingman] !=
                         OBJECTIVE_HOLD_FORMATION)
                    SendCommMenuChoice(6);
            }
            if (g_bRadioSilence != 0)
                SendCommMenuChoice(8);
            else
                SendCommMenuChoice(7);
            SendCommMenuChoice(3);
        }
        if (g_bEjectionAwaitingCommCommand != 0 &&
            g_ucPendingEjectionTransition != 0xff) {
            if (g_asShipSide[g_cCommMenuRecipient] ==
                    SIDE_IMPERIAL &&
                g_asObjectType[
                    g_cCommMenuRecipient] >=
                    OBJECT_TYPE_DEBRIS_GLASS)
                SendCommMenuChoice(9);
        } else {
            if (g_asShipSide[g_cCommMenuRecipient] ==
                SIDE_IMPERIAL) {
                if ((g_asShipMissionIndex[
                         g_cCommMenuRecipient] ==
                         g_6_nHomeOperationShipIndexValue ||
                     g_asShipMissionIndex[
                         g_cCommMenuRecipient] ==
                         g_13_stMissionHeaderBlock.wingmanMissionShip) &&
                    g_bLandingCommRequestPending == 0) {
                    if (g_aObjectTypeData[
                            g_acObjectType[
                                g_cCommMenuRecipient]]
                            .collisionRadius < 500)
                        SendCommMenuChoice(11);
                    else
                        SendCommMenuChoice(9);
                }
                if (g_asShipMissionType[
                        g_cCommMenuRecipient] ==
                        MISSION_TYPE_CANNED_SEQUENCE &&
                    g_bLandingCommRequestPending == 0)
                    SendCommMenuChoice(12);
            }
        }
        if (g_asShipSide[g_cCommMenuRecipient] ==
            SIDE_KILRATHI) {
            SendCommMenuChoice(g_nEnemyTauntCommandBase);
            SendCommMenuChoice(g_nEnemyTauntCommandBase + 1);
            SendCommMenuChoice(g_nEnemyTauntCommandBase + 2);
        }
    }
    if (IsCommMenuIdle() != 0) {
        CloseCommChoiceMenu();
    } else if (g_nCommMenuChoiceCount < 5) {
        SendCommMenuChoice(0);
    }
    if (IsCommChoiceMenuOpen() != 0) {
        if (g_nCommMenuChoiceCount >= 5)
            strcpy(g_4_szCommMenuHeadingStore,
                   _("VID-COM SYSTEM\nTo: "));
        else
            strcpy(g_4_szCommMenuHeadingStore,
                   _("VID-COM SYSTEM\n\nTo: "));
        if (g_asShipIdentified[
                g_cCommMenuRecipient] == 0)
            strcat(g_4_szCommMenuHeadingStore,
                   _("UNIDENTIFIED"));
        else
            strcat(g_4_szCommMenuHeadingStore,
                   g_apShipMissionRecord[
                       g_cCommMenuRecipient]->name);
        strcat(g_4_szCommMenuHeadingStore,
               "\n");
        OpenCommMenuForTarget(g_4_szCommMenuHeadingStore,
                              _("CHOOSE"));
    }
}

/* Function start: 0x447D85 */
void RefreshCommunicationMenu(void)
{
    if (IsCommChoiceMenuOpen() != 0) {
        if (g_cPendingCommMenuAction == 1)
            BuildCommunicationRecipientMenu();
        if (g_cPendingCommMenuAction == 2)
            BuildCommunicationCommandMenu();
        if (g_nCommMenuReuseMode == 0)
            InvalidateVduMode(1);
    }
}

/* Function start: 0x447DE3 */
void HandleCommunicationMenuRequest(void)
{
    if (IsCommChoiceMenuOpen() != 0)
        CloseCommChoiceMenu();
    if (message_showing() == 0 &&
        IsCommChoiceMenuOpen() == 0 && CanOpenCommMenu() != 0) {
        OpenCommRecipientMenu();
        ResetCommMenuChoices(0);
        RefreshCommunicationMenu();
    }
}

/* Function start: 0x447E47 */
void show_communications_disp(void)
{
    signed char choice;
#ifdef SDL_PORT
    int selectedChoice;
    unsigned char normalColour;
#endif

    if (IsCommChoiceMenuOpen() == 0)
        HandleCommunicationMenuRequest();
    if (IsCommChoiceMenuOpen() != 0) {
        set_new_vdu(1);
        DrawTextAt(&g_1_stRightVduTextContextBlock,
                   g_4_stRightVduView.left,
                   g_4_stRightVduView.top,
                   g_pszCommScreenHeading, 2);
#ifdef SDL_PORT
        selectedChoice = SdlGetCommunicationMenuSelection();
        normalColour = g_1_stRightVduTextContextBlock.colour;
#endif
        for (choice = 0; choice < g_nCommMenuChoiceCount;
             choice++) {
#ifdef SDL_PORT
            if ((int)choice == selectedChoice)
                g_1_stRightVduTextContextBlock.colour =
                    g_cRedColour;
#endif
            DrawFormattedText("\n%d.%s",
                              (int)choice + 1,
                              g_1_apszCommMenuChoiceTextTable[choice]);
#ifdef SDL_PORT
            g_1_stRightVduTextContextBlock.colour = normalColour;
#endif
        }
        g_nCommMenuReuseMode = 1;
    }
}

/* Function start: 0x447EF5 */
void Chosen_communicate_option(short choice)
{
    PlaySfxWaveFileByNumber(0x19, -1, 0);
    switch (g_cPendingCommMenuAction) {
    case 0:
        CloseCommChoiceMenu();
        break;
    case 1:
        if (g_abCommMenuChoiceCommand[choice] == 0) {
            CloseCommChoiceMenu();
        } else {
            if (g_abCommMenuChoiceCommand[choice] == 1)
                SelectCommRecipient(g_nYourWingman);
            else
                SelectCommRecipient(g_acShipTarget[0]);
            RefreshCommunicationMenu();
        }
        break;
    case 2:
        CloseCommChoiceMenu();
        request(0, (short)g_cCommMenuRecipient,
                (short)g_abCommMenuChoiceCommand[choice]);
        if (g_abCommMenuChoiceCommand[choice] >=
            g_nEnemyTauntCommandBase) {
            g_nEnemyTauntCommandBase =
                (short)(g_nEnemyTauntCommandBase & 0xfe);
        }
        break;
    }
}

short get_mode(short i);
void pop_mode(short i);

/* Function start: 0x448008 */
void IssueQuickCommCommand(short recipient, short command)
{
    request(0, recipient, command);
    if (g_nEnemyTauntCommandBase <= command)
        g_nEnemyTauntCommandBase =
            (short)(g_nEnemyTauntCommandBase & 0xfe);
}

/* Function start: 0x44804A */
void FreeCommDisplayResources(void)
{
    g_nCommSpeakerRating = -1;
    g_nCommSpeakerObject = -1;
    g_nCommPortraitIndex = -1;
}

/* Function start: 0x448070 */
void EndCommSessionWithWingman(void)
{
    if (g_pCommPortraitResource != 0)
        malf_noise(1, 1, g_cDarkGreenColour,
                   23, 1);
    FreeCommDisplayResources();
    if (get_mode(1) == 6)
        pop_mode(1);
}

/* Function start: 0x4480C6 */
void EndCommMenu(void)
{
    RestoreHudMessageBackground();
    clear_message_time();
    if (get_mode(1) == 6)
        EndCommSessionWithWingman();
    g_pszPendingHudMessage = 0;
}

/* Function start: 0x448100 */
void ShowCentredPrompt(char *text, unsigned short arg)
{
    DosStrcpy(g_szOnScreenMessageBuffer, text);
    SetHudMessageText(g_szOnScreenMessageBuffer,
                      g_bPrimaryViewBufferColour, arg);
}

/* Function start: 0x448136 */
char *ExpandCommMessageTokens(const char *text)
{
    char *dest;
    const char *marker;
    const char *tag;
    short scratchLength;

    g_16_szTextBufferBuffer[0] = '\0';
    while ((marker = DosStrchr(text, '$')) != 0) {
        dest = DosStrchr(g_16_szTextBufferBuffer, '\0');
        while (marker != text)
            *dest++ = *text++;
        *dest = '\0';
        marker++;
        tag = marker++;
        switch (*tag) {
        case 'C':
            DosStrcat(g_16_szTextBufferBuffer,
                      g_stCurrentPilotProfile.callsign);
            break;
        case 'R':
            DosStrcat(g_16_szTextBufferBuffer,
                      _(g_apszPilotRankNames[
                          g_stCurrentPilotProfile.rank]));
            if (g_16_szTextBufferBuffer[
                    DosStrlen(g_16_szTextBufferBuffer) - 1] == '.' &&
                *marker == '.') {
                scratchLength = DosStrlen(g_16_szTextBufferBuffer);
                g_16_szTextBufferBuffer[scratchLength - 1] = '\0';
            }
            break;
        case 'N':
        case 'P':
            DosStrcat(g_16_szTextBufferBuffer,
                      g_stCurrentPilotProfile.lastName);
            break;
        }
        text = marker;
    }
    DosStrcat(g_16_szTextBufferBuffer, text);
    return g_16_szTextBufferBuffer;
}

long GetNamedPacketSize(const char *filename, short section);
void LoadAndPlaySpeechPacket(char *fileName, short section);
void *LoadPacketIntoBuffer(char *fileName, short section, void *destination,
                           int registerHandle);
void malf_noise(short vdu, int effect, unsigned short colour, short sound,
                short refresh);

/* Function start: 0x4482CA */
void real_vid_transmit(short obj, short message)
{
    int size;
    char callsign[20];
    char packetFile[16];
#ifdef SDL_PORT
    char text[128]; /* 87-byte translated packet, ship name and separator. */
#else
    char text[84];
#endif
    char *expanded;
    char indexText[8];
#ifdef SDL_PORT
    short commFile;
#endif

    g_nCommSpeakerObject = obj;
    g_nCommSpeakerRating =
        g_acShipPortrait[g_nCommSpeakerObject];
    g_nCommPortraitIndex = g_nCommSpeakerRating;
#ifdef SDL_PORT
    /* SO1.exe 0x4518B8 (and SO2.exe): the expansions' speakers without
     * comm files of their own use other pilots' -- Bear (10) and 30-34
     * file 20, 35 file 21 -- and portrait 14 never transmits. */
    commFile = g_nCommPortraitIndex;
    if (g_nSdlCampaign != 0) {
        if (commFile == 10 || (commFile >= 30 && commFile <= 34))
            commFile = 20;
        else if (commFile == 35)
            commFile = 21;
        if (g_nCommPortraitIndex == 14)
            return;
    }
#endif
    if (g_nCommPortraitIndex == -1)
        return;
    g_bCommSpeechPlaying = 0;
    g_nCommDeathSequenceFrame = 0;
    if (g_bVideoImagesSuppressed == 0) {
        if (g_nCommPortraitIndex !=
            g_nLoadedCommPortraitPilot) {
            LoadCommPortraitResources(g_nCommPortraitIndex);
            g_nLoadedCommPortraitPilot =
                g_nCommPortraitIndex;
        }
        if (g_pCommPortraitResource != 0) {
            push_mode(1, 6);
            malf_noise(1, 3, g_cDarkGreenColour,
                       0x17, 1);
            g_19_nCommPortraitAnimationTickWord = 0;
            DrawSpriteDefault(&g_4_stRightVduView,
                              g_4_stRightVduView.left,
                              g_4_stRightVduView.top,
                              g_pCommPortraitResource, 0);
            DrawSpriteDefault(&g_4_stRightVduView,
                              g_4_stRightVduView.left,
                              g_4_stRightVduView.top,
                              g_pCommPortraitResource, 1);
        }
    }
    strcpy(packetFile, "communic.s");
#ifdef SDL_PORT
    if (commFile < 10)
        strcat(packetFile, "0");
    itoa((int)commFile, indexText, 10);
    strcat(packetFile, indexText);
    /* Paladin's docking replies (messages 0x12 and 0x13) come from
     * palcomm.s08 in the expansions. */
    if (g_nSdlCampaign != 0 && commFile == 8 &&
        (message == 0x12 || message == 0x13))
        strcpy(packetFile, "palcomm.s08");
#else
    if (g_nCommPortraitIndex < 10)
        strcat(packetFile, "0");
    itoa((int)g_nCommPortraitIndex, indexText, 10);
    strcat(packetFile, indexText);
#endif
    size = GetNamedPacketSize(packetFile, message);
    g_nCommMessageIndex = message;
    if (g_bSpeechEnabledFlag != 0 && size > 0 &&
        size < g_nDigiSampleBytes) {
        DismissHudMessageIfShowing();
#ifdef SDL_PORT
        /* See issue #27: DismissHudMessageIfShowing() only arms the "-1"
         * wait-for-speech sentinel (via set_message_time) when a HUD
         * message was already showing. vid_equiv() only starts a new
         * comm transmission once message_showing() is already false, so
         * on the normal path the sentinel never gets armed here:
         * check_message() then never sees g_9_nScreenMessageTimeNumber ==
         * -1 and so never notices g_bSpeechPlaybackDone go
         * true, leaving the comm portrait/VDU stuck once the speech line
         * finishes playing. Arm the sentinel unconditionally so playback
         * completion always gets observed. */
        set_message_time(-1);
#endif
        LoadAndPlaySpeechPacket(packetFile, message);
        g_bCommSpeechPlaying = 1;
        return;
    }
    /* The sampled line and its subtitle share a packet number: "communic.s"
     * carries the speech, "communic.t" the text. */
#ifdef SDL_PORT
    /* The expansions also write the 't' at index 9 of "palcomm.s08",
     * naming a missing "palcomm.st8". Change the extension's letter instead,
     * so PALCOMM.T08's text appears when the reply is not spoken. */
    strrchr(packetFile, '.')[1] = 't';
#else
    packetFile[9] = 't';
#endif
    if (GetNamedPacketSize(packetFile, message) > 0) {
        LoadPacketIntoBuffer(packetFile, message,
                             g_szCommMessageTextText, 0);
        if (g_asShipIdentified[obj] != 0)
            sprintf(callsign, g_apShipMissionRecord[obj]->name);
        else
            sprintf(callsign, _("UNKNOWN"));
#ifdef SDL_PORT
        snprintf(text, sizeof(text), "%s: %s", callsign,
                 g_szCommMessageTextText);
#else
        sprintf(text, "%s: %Fs", callsign,
                g_szCommMessageTextText);
#endif
        expanded = ExpandCommMessageTokens(text);
        ShowCentredPrompt(expanded, MeasureMessageWidth(text));
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
char *g_48_apCommunicationTextPacketsTable[50]; /* 0x005d17c0 */
#ifdef SDL_PORT
char g_szOnScreenMessageBuffer[256];
#else
char g_szOnScreenMessageBuffer[52]; /* 0x005d1890 */
#endif
int g_11_nUnusedCommWordWord[5]; /* 0x005d18d0 */ /* never referenced */
char g_szCommMessageTextText[0x58]; /* 0x005d18f0 */
signed char g_abCommMenuChoiceCommand[7]; /* 0x005d1948 */
const char *g_pszCommScreenHeading; /* 0x005d1950 */
char g_4_szCommMenuHeadingStore[0x40]; /* 0x005d1960 */
const char *g_1_apszCommMenuChoiceTextTable[7]; /* 0x005d19a0 */
