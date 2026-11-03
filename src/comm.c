/*
 *  Communications menu and transmissions.
 *
 *  Retail object 0x447300-0x44856F (link order; see docs/ORDER.md).
 */
#include "game.h"

short g_nEnemyTauntCommandBase_0049b76c = 20;
short g_nCommMenuChoiceCount_0049b770 = -1;
short g_nCommMenuReuseMode_0049b774 = 0;
signed char g_cMessageSpeed_0049b778 = 2;
signed char g_cPendingCommMenuAction_0049b77c = 1;
signed char g_bRadioSilence_0049b780 = 0;
signed char g_bVideoImagesSuppressed_0049b784 = 0;
unsigned char *g_pCommPortraitResource_0049b788 = 0;
unsigned char *g_pCommVduFrameResource_0049b78c = 0;
signed char g_cCommMenuRecipient_0049b790 = -1;
short g_nCommSpeakerObject_0049b794 = 0;
short g_nCommSpeakerRating_0049b798 = 0;
short g_nCommPortraitIndex_0049b79c = -1;
short g_bCommSpeechPlaying_0049b7a0 = 0;

/* Function start: 0x447300 */
void LoadCommPortraitResources(short portrait)
{
    if (g_bHighMemoryResourcesEnabled_005c80e4 == 0)
        return;
    FreePacketAndClear(&g_pCommPortraitResource_0049b788, 4);
    g_pCommPortraitResource_0049b788 =
        FetchDiskPacketRetrying("vdu.v00", portrait, 4);
    if (g_pCommVduFrameResource_0049b78c == 0) {
        g_pCommVduFrameResource_0049b78c =
            FetchDiskPacketRetrying("vdu.v00", 0x1c, 4);
    }
}

/* Function start: 0x447369 */
void ResetCommMenuChoices(short reuse)
{
    short choice;

    if (reuse == 0) {
        for (choice = 0; choice < 7; choice++) {
            g_apszCommMenuChoiceTextTable_005d19a0[choice] = 0;
            g_abCommMenuChoiceCommand_005d1948[choice] = -1;
        }
    }
    g_nCommMenuChoiceCount_0049b770 = 0;
    g_nCommMenuReuseMode_0049b774 = reuse;
}

/* Function start: 0x4473D1 */
short IsCommMenuIdle(void)
{
    return g_nCommMenuChoiceCount_0049b770 == 0;
}

/* Function start: 0x4473FC */
void AppendCommMenuChoice(const char *text, short command)
{
    if (g_nCommMenuReuseMode_0049b774 == 1 &&
        (g_apszCommMenuChoiceTextTable_005d19a0[
             g_nCommMenuChoiceCount_0049b770] != text ||
         g_abCommMenuChoiceCommand_005d1948[
             g_nCommMenuChoiceCount_0049b770] != command))
        g_nCommMenuReuseMode_0049b774 = 0;
    g_apszCommMenuChoiceTextTable_005d19a0[
        g_nCommMenuChoiceCount_0049b770] = text;
    g_abCommMenuChoiceCommand_005d1948[
        g_nCommMenuChoiceCount_0049b770] = (signed char)command;
    g_nCommMenuChoiceCount_0049b770++;
}

/* Function start: 0x447479 */
void SendCommMenuChoice(short i)
{
    if (g_apCommunicationTextPacketsTable_005d17c0[i] == 0)
        g_apCommunicationTextPacketsTable_005d17c0[i] =
            LoadPacketAllocated("communic.you", i);
    AppendCommMenuChoice(g_apCommunicationTextPacketsTable_005d17c0[i], i);
}

/* Function start: 0x4474CA */
void OpenCommMenuForTarget(const char *heading, const char *message)
{
    CockpitMessage(message, g_cYellowColour_0049cb58, -1);
    g_pszCommScreenHeading_005d1950 = heading;
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
    g_cPendingCommMenuAction_0049b77c = 1;
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
    return g_nYourWingman_0049346c == -1;
}

/* Function start: 0x44759B */
short have_target(void)
{
    return unactive(g_acShipTarget_00495f20[0]) == 0;
}

/* Function start: 0x4475D3 */
short CanOpenCommMenu(void)
{
    return have_target() || !wingman_dead();
}

/* Function start: 0x44760F */
void SelectCommRecipient(short recipient)
{
    g_cCommMenuRecipient_0049b790 = recipient;
    g_cPendingCommMenuAction_0049b77c = 2;
}

/* Function start: 0x447629 */
void BuildCommunicationRecipientMenu(void)
{
    short target;

    ResetCommMenuChoices(g_nCommMenuReuseMode_0049b774);
    OpenCommMenuForTarget("VID-COM SYSTEM\n\nSend message to?\n\n",
                          "SELECT");
    if (wingman_dead() != 0) {
        SelectCommRecipient(g_acShipTarget_00495f20[0]);
        return;
    }
    if (have_target() == 0 ||
        g_acShipTarget_00495f20[0] == g_nYourWingman_0049346c) {
        SelectCommRecipient(g_nYourWingman_0049346c);
        return;
    }
    AppendCommMenuChoice(
        g_apShipMissionRecord_00495da8[g_nYourWingman_0049346c]->name,
        1);
    target = g_acShipTarget_00495f20[0];
    if (target != -1) {
        if (g_asShipSide_004955d0[target] == SIDE_KILRATHI &&
            g_aeObjectClass_00495328[target] >= OBJECT_CLASS_SHIP) {
            if (g_asShipIdentified_00496078[target] != 0)
                AppendCommMenuChoice("ENEMY TARGET", 2);
            else
                AppendCommMenuChoice(
                    "UNIDENTIFIED", 2);
        } else if (g_asShipSide_004955d0[target] == SIDE_IMPERIAL) {
            if (g_aeObjectClass_00495328[target] >= OBJECT_CLASS_SHIP &&
                g_asShipMissionType_00495de8[target] ==
                    MISSION_TYPE_CANNED_SEQUENCE)
                AppendCommMenuChoice(g_apShipMissionRecord_00495da8[
                                         g_acShipTarget_00495f20[0]]->name,
                                     3);
            if ((g_aeObjectClass_00495328[target] == OBJECT_CLASS_SHIP &&
                 any_enemy(0, 14000) != 0) ||
                g_asShipMissionIndex_00495d00[target] ==
                    g_nHomeOperationShipIndexValue_005d1e22) {
                if (g_asShipIdentified_00496078[target] != 0)
                    AppendCommMenuChoice(g_apShipMissionRecord_00495da8[
                                             g_acShipTarget_00495f20[0]]->name,
                                         3);
                else
                    AppendCommMenuChoice(
                        "UNIDENTIFIED", 3);
            } else if (g_ucPendingEjectionTransition_0049b8ac != 0xff &&
                       g_bEjectionAwaitingCommCommand_0049b8b4 != 0 &&
                       g_asObjectType_00495298[target] >=
                           OBJECT_TYPE_DEBRIS_GLASS) {
                AppendCommMenuChoice(
                    g_apShipMissionRecord_00495da8[
                        g_acShipTarget_00495f20[0]]->name,
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

    if (g_nEnemyCommPilotIndex_005d179e != -1 &&
        g_acShipPortrait_00495d88[g_cCommMenuRecipient_0049b790] ==
            g_nEnemyCommPilotIndex_005d179e &&
        g_asShipSide_004955d0[g_cCommMenuRecipient_0049b790] ==
            SIDE_KILRATHI) {
        g_nEnemyTauntCommandBase_0049b76c =
            (g_nEnemyTauntCommandBase_0049b76c & 1) |
            g_nEnemyCommCommandBase_005d179c;
    } else {
        g_nEnemyTauntCommandBase_0049b76c =
            (g_nEnemyTauntCommandBase_0049b76c & 1) | 20;
    }
    ResetCommMenuChoices(g_nCommMenuReuseMode_0049b774);
    if (g_asShipIdentified_00496078[g_cCommMenuRecipient_0049b790] == 0) {
        SendCommMenuChoice(10);
    } else {
        if (g_cCommMenuRecipient_0049b790 == g_nYourWingman_0049346c ||
            (g_aeObjectClass_00495328[g_cCommMenuRecipient_0049b790] ==
                 OBJECT_CLASS_SHIP &&
             g_asShipSide_004955d0[g_cCommMenuRecipient_0049b790] ==
                 SIDE_IMPERIAL)) {
            if (g_aeShipObjective_00495f08[
                    g_cCommMenuRecipient_0049b790] ==
                    OBJECTIVE_HOLD_FORMATION &&
                any_enemy(0, 14000) != 0)
                SendCommMenuChoice(4);
            if (have_target() != 0 &&
                g_asShipSide_004955d0[g_acShipTarget_00495f20[0]] ==
                    SIDE_KILRATHI)
                SendCommMenuChoice(1);
            for (object = 1; object < 10; object++) {
                if (g_aeObjectClass_00495328[object] ==
                        OBJECT_CLASS_SHIP &&
                    g_acShipTarget_00495f20[object] == 0) {
                    get_facing_range_from_object(0, object);
                    if (g_nFacingToTarget_00493194 < -70 &&
                        g_nTargetFacing_00493198 > 90 &&
                        g_nTargetRange_0049319c < 4000) {
                        SendCommMenuChoice(2);
                        break;
                    }
                }
            }
            if (g_cCommMenuRecipient_0049b790 ==
                g_nYourWingman_0049346c) {
                if (g_nAutoEngageTimer_00496130 != -1)
                    SendCommMenuChoice(5);
                else if (g_aeShipObjective_00495f08[
                             g_nYourWingman_0049346c] !=
                         OBJECTIVE_HOLD_FORMATION)
                    SendCommMenuChoice(6);
            }
            if (g_bRadioSilence_0049b780 != 0)
                SendCommMenuChoice(8);
            else
                SendCommMenuChoice(7);
            SendCommMenuChoice(3);
        }
        if (g_bEjectionAwaitingCommCommand_0049b8b4 != 0 &&
            g_ucPendingEjectionTransition_0049b8ac != 0xff) {
            if (g_asShipSide_004955d0[g_cCommMenuRecipient_0049b790] ==
                    SIDE_IMPERIAL &&
                g_asObjectType_00495298[
                    g_cCommMenuRecipient_0049b790] >=
                    OBJECT_TYPE_DEBRIS_GLASS)
                SendCommMenuChoice(9);
        } else {
            if (g_asShipSide_004955d0[g_cCommMenuRecipient_0049b790] ==
                SIDE_IMPERIAL) {
                if ((g_asShipMissionIndex_00495d00[
                         g_cCommMenuRecipient_0049b790] ==
                         g_nHomeOperationShipIndexValue_005d1e22 ||
                     g_asShipMissionIndex_00495d00[
                         g_cCommMenuRecipient_0049b790] ==
                         g_stMissionHeaderBlock_005d3e70.wingmanMissionShip) &&
                    g_bLandingCommRequestPending_00492fa0 == 0) {
                    if (g_aObjectTypeData_00496d30[
                            g_acObjectType_00493980[
                                g_cCommMenuRecipient_0049b790]]
                            .collisionRadius < 500)
                        SendCommMenuChoice(11);
                    else
                        SendCommMenuChoice(9);
                }
                if (g_asShipMissionType_00495de8[
                        g_cCommMenuRecipient_0049b790] ==
                        MISSION_TYPE_CANNED_SEQUENCE &&
                    g_bLandingCommRequestPending_00492fa0 == 0)
                    SendCommMenuChoice(12);
            }
        }
        if (g_asShipSide_004955d0[g_cCommMenuRecipient_0049b790] ==
            SIDE_KILRATHI) {
            SendCommMenuChoice(g_nEnemyTauntCommandBase_0049b76c);
            SendCommMenuChoice(g_nEnemyTauntCommandBase_0049b76c + 1);
            SendCommMenuChoice(g_nEnemyTauntCommandBase_0049b76c + 2);
        }
    }
    if (IsCommMenuIdle() != 0) {
        CloseCommChoiceMenu();
    } else if (g_nCommMenuChoiceCount_0049b770 < 5) {
        SendCommMenuChoice(0);
    }
    if (IsCommChoiceMenuOpen() != 0) {
        if (g_nCommMenuChoiceCount_0049b770 >= 5)
            strcpy(g_szCommMenuHeadingStore_005d1960,
                   "VID-COM SYSTEM\nTo: ");
        else
            strcpy(g_szCommMenuHeadingStore_005d1960,
                   "VID-COM SYSTEM\n\nTo: ");
        if (g_asShipIdentified_00496078[
                g_cCommMenuRecipient_0049b790] == 0)
            strcat(g_szCommMenuHeadingStore_005d1960,
                   "UNIDENTIFIED");
        else
            strcat(g_szCommMenuHeadingStore_005d1960,
                   g_apShipMissionRecord_00495da8[
                       g_cCommMenuRecipient_0049b790]->name);
        strcat(g_szCommMenuHeadingStore_005d1960,
               "\n");
        OpenCommMenuForTarget(g_szCommMenuHeadingStore_005d1960,
                              "CHOOSE");
    }
}

/* Function start: 0x447D85 */
void RefreshCommunicationMenu(void)
{
    if (IsCommChoiceMenuOpen() != 0) {
        if (g_cPendingCommMenuAction_0049b77c == 1)
            BuildCommunicationRecipientMenu();
        if (g_cPendingCommMenuAction_0049b77c == 2)
            BuildCommunicationCommandMenu();
        if (g_nCommMenuReuseMode_0049b774 == 0)
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
        DrawTextAt(&g_stRightVduTextContextBlock_005d2ce0,
                   g_stRightVduView_005d2b20.left,
                   g_stRightVduView_005d2b20.top,
                   g_pszCommScreenHeading_005d1950, 2);
#ifdef SDL_PORT
        selectedChoice = SdlGetCommunicationMenuSelection();
        normalColour = g_stRightVduTextContextBlock_005d2ce0.colour;
#endif
        for (choice = 0; choice < g_nCommMenuChoiceCount_0049b770;
             choice++) {
#ifdef SDL_PORT
            if ((int)choice == selectedChoice)
                g_stRightVduTextContextBlock_005d2ce0.colour =
                    g_cRedColour_0049cb5c;
#endif
            DrawFormattedText("\n%d.%s",
                              (int)choice + 1,
                              g_apszCommMenuChoiceTextTable_005d19a0[choice]);
#ifdef SDL_PORT
            g_stRightVduTextContextBlock_005d2ce0.colour = normalColour;
#endif
        }
        g_nCommMenuReuseMode_0049b774 = 1;
    }
}

/* Function start: 0x447EF5 */
void Chosen_communicate_option(short choice)
{
    PlaySfxWaveFileByNumber(0x19, -1, 0);
    switch (g_cPendingCommMenuAction_0049b77c) {
    case 0:
        CloseCommChoiceMenu();
        break;
    case 1:
        if (g_abCommMenuChoiceCommand_005d1948[choice] == 0) {
            CloseCommChoiceMenu();
        } else {
            if (g_abCommMenuChoiceCommand_005d1948[choice] == 1)
                SelectCommRecipient(g_nYourWingman_0049346c);
            else
                SelectCommRecipient(g_acShipTarget_00495f20[0]);
            RefreshCommunicationMenu();
        }
        break;
    case 2:
        CloseCommChoiceMenu();
        request(0, (short)g_cCommMenuRecipient_0049b790,
                (short)g_abCommMenuChoiceCommand_005d1948[choice]);
        if (g_abCommMenuChoiceCommand_005d1948[choice] >=
            g_nEnemyTauntCommandBase_0049b76c) {
            g_nEnemyTauntCommandBase_0049b76c =
                (short)(g_nEnemyTauntCommandBase_0049b76c & 0xfe);
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
    if (g_nEnemyTauntCommandBase_0049b76c <= command)
        g_nEnemyTauntCommandBase_0049b76c =
            (short)(g_nEnemyTauntCommandBase_0049b76c & 0xfe);
}

/* Function start: 0x44804A */
void FreeCommDisplayResources(void)
{
    g_nCommSpeakerRating_0049b798 = -1;
    g_nCommSpeakerObject_0049b794 = -1;
    g_nCommPortraitIndex_0049b79c = -1;
}

/* Function start: 0x448070 */
void EndCommSessionWithWingman(void)
{
    if (g_pCommPortraitResource_0049b788 != 0)
        malf_noise(1, 1, g_cDarkGreenColour_0049cb60,
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
    g_pszPendingHudMessage_0049afec = 0;
}

/* Function start: 0x448100 */
void ShowCentredPrompt(char *text, unsigned short arg)
{
    DosStrcpy(g_szOnScreenMessageBuffer_005d1890, text);
    SetHudMessageText(g_szOnScreenMessageBuffer_005d1890,
                      g_bPrimaryViewBufferColour_0049cb50, arg);
}

/* Function start: 0x448136 */
char *ExpandCommMessageTokens(const char *text)
{
    char *dest;
    const char *marker;
    const char *tag;
    short scratchLength;

    g_szTextBufferBuffer_005d1c40[0] = '\0';
    while ((marker = DosStrchr(text, '$')) != 0) {
        dest = DosStrchr(g_szTextBufferBuffer_005d1c40, '\0');
        while (marker != text)
            *dest++ = *text++;
        *dest = '\0';
        marker++;
        tag = marker++;
        switch (*tag) {
        case 'C':
            DosStrcat(g_szTextBufferBuffer_005d1c40,
                      g_stCurrentPilotProfile_00493408.callsign);
            break;
        case 'R':
            DosStrcat(g_szTextBufferBuffer_005d1c40,
                      g_apszPilotRankNames_0049a608[
                          g_stCurrentPilotProfile_00493408.rank]);
            if (g_szTextBufferBuffer_005d1c40[
                    DosStrlen(g_szTextBufferBuffer_005d1c40) - 1] == '.' &&
                *marker == '.') {
                scratchLength = DosStrlen(g_szTextBufferBuffer_005d1c40);
                g_szTextBufferBuffer_005d1c40[scratchLength - 1] = '\0';
            }
            break;
        case 'N':
        case 'P':
            DosStrcat(g_szTextBufferBuffer_005d1c40,
                      g_stCurrentPilotProfile_00493408.lastName);
            break;
        }
        text = marker;
    }
    DosStrcat(g_szTextBufferBuffer_005d1c40, text);
    return g_szTextBufferBuffer_005d1c40;
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
    char text[84];
    char *expanded;
    char indexText[8];

    g_nCommSpeakerObject_0049b794 = obj;
    g_nCommSpeakerRating_0049b798 =
        g_acShipPortrait_00495d88[g_nCommSpeakerObject_0049b794];
    g_nCommPortraitIndex_0049b79c = g_nCommSpeakerRating_0049b798;
    if (g_nCommPortraitIndex_0049b79c == -1)
        return;
    g_bCommSpeechPlaying_0049b7a0 = 0;
    g_nCommDeathSequenceFrame_0049ae84 = 0;
    if (g_bVideoImagesSuppressed_0049b784 == 0) {
        if (g_nCommPortraitIndex_0049b79c !=
            g_nLoadedCommPortraitPilot_004931c4) {
            LoadCommPortraitResources(g_nCommPortraitIndex_0049b79c);
            g_nLoadedCommPortraitPilot_004931c4 =
                g_nCommPortraitIndex_0049b79c;
        }
        if (g_pCommPortraitResource_0049b788 != 0) {
            push_mode(1, 6);
            malf_noise(1, 3, g_cDarkGreenColour_0049cb60,
                       0x17, 1);
            g_nCommPortraitAnimationTickWord_005d1d94 = 0;
            DrawSpriteDefault(&g_stRightVduView_005d2b20,
                              g_stRightVduView_005d2b20.left,
                              g_stRightVduView_005d2b20.top,
                              g_pCommPortraitResource_0049b788, 0);
            DrawSpriteDefault(&g_stRightVduView_005d2b20,
                              g_stRightVduView_005d2b20.left,
                              g_stRightVduView_005d2b20.top,
                              g_pCommPortraitResource_0049b788, 1);
        }
    }
    strcpy(packetFile, "communic.s");
    if (g_nCommPortraitIndex_0049b79c < 10)
        strcat(packetFile, "0");
    itoa((int)g_nCommPortraitIndex_0049b79c, indexText, 10);
    strcat(packetFile, indexText);
    size = GetNamedPacketSize(packetFile, message);
    g_nCommMessageIndex_0049ae88 = message;
    if (g_bSpeechEnabledFlag_005c8de8 != 0 && size > 0 &&
        size < g_nDigiSampleBytes_0048e0e0) {
        DismissHudMessageIfShowing();
#ifdef SDL_PORT
        /* See issue #27: DismissHudMessageIfShowing() only arms the "-1"
         * wait-for-speech sentinel (via set_message_time) when a HUD
         * message was already showing. vid_equiv() only starts a new
         * comm transmission once message_showing() is already false, so
         * on the normal path the sentinel never gets armed here:
         * check_message() then never sees g_nScreenMessageTimeNumber_005d1c32 ==
         * -1 and so never notices g_bSpeechPlaybackDone_004a266c go
         * true, leaving the comm portrait/VDU stuck once the speech line
         * finishes playing. Arm the sentinel unconditionally so playback
         * completion always gets observed. */
        set_message_time(-1);
#endif
        LoadAndPlaySpeechPacket(packetFile, message);
        g_bCommSpeechPlaying_0049b7a0 = 1;
        return;
    }
    /* The sampled line and its subtitle share a packet number: "communic.s"
     * carries the speech, "communic.t" the text. */
    packetFile[9] = 't';
    if (GetNamedPacketSize(packetFile, message) > 0) {
        LoadPacketIntoBuffer(packetFile, message,
                             g_szCommMessageTextText_005d18f0, 0);
        if (g_asShipIdentified_00496078[obj] != 0)
            sprintf(callsign, g_apShipMissionRecord_00495da8[obj]->name);
        else
            sprintf(callsign, "UNKNOWN");
        sprintf(text, "%s: %Fs", callsign,
                g_szCommMessageTextText_005d18f0);
        expanded = ExpandCommMessageTokens(text);
        ShowCentredPrompt(expanded, MeasureMessageWidth(text));
    }
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
char *g_apCommunicationTextPacketsTable_005d17c0[50];
char g_szOnScreenMessageBuffer_005d1890[52];
int g_nUnusedCommWordWord_005d18d0[5]; /* never referenced */
char g_szCommMessageTextText_005d18f0[0x58];
signed char g_abCommMenuChoiceCommand_005d1948[7];
const char *g_pszCommScreenHeading_005d1950;
char g_szCommMenuHeadingStore_005d1960[0x40];
const char *g_apszCommMenuChoiceTextTable_005d19a0[7];
