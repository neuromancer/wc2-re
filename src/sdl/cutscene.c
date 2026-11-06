#include "game.h"

int g_bSdlCutsceneOnly;

void SdlRunSelectedCampaignCutscene(void)
{
    short savedWingman;
    signed char savedWingmanType;
    short savedWingmanDamage;

    g_nSelectedCampaignSlot = 0;
    ReleaseSpaceflightResources();
    LoadStartingCampaignGlobals(g_nSelectedCampaignSlot);
    if (g_pCampaignGlobals == 0)
        ReportFatalErrorCode("015");

    g_pCampaignGlobals->series =
        g_nDirectSeries;
    g_pCampaignGlobals->mission =
        g_stDirectMissionSelection.mission;
    g_pCampaignGlobals->field_08 = 0;
    g_pCampaignGlobals->arcadeState = 0;
    g_bDeveloperCampaignReady = 0;

    savedWingman = g_nYourWingman;
    if (savedWingman == -1) {
        /* Direct post-flight scenes need the intact wingman state that the
         * skipped mission would normally leave in object slot 1. */
        savedWingmanType = g_acObjectType[1];
        savedWingmanDamage = g_asObjectDamage[1];
        g_nYourWingman = 1;
        g_acObjectType[1] = OBJECT_DATA_LASER_CANNON;
        g_asObjectDamage[1] = 0;
    }

    g_pCutsceneCockpitPalette =
        AllocateScenePointerTable(1, 0x3420, 2, "HB1");
    g_13_pActiveCutscenePixelsAddress =
        g_pCutsceneCockpitPalette;
    g_bCutsceneViewportPreallocated = 1;
    RunCampaignScript(g_nSelectedCampaignSlot);
    g_bCutsceneViewportPreallocated = 0;

    g_pCampaignGlobals->field_08 =
        g_pCampaignGlobals->field_0a;
    fprintf(stderr,
            "Running campaign cutscene: series %d, mission %d, entry %d.\n",
            g_pCampaignGlobals->series,
            g_pCampaignGlobals->mission,
            g_pCampaignGlobals->field_08);
    g_bRoomTransitionAnimationEnabled = 0;
    RunCampaignScript(g_nSelectedCampaignSlot);
    g_pCampaignGlobals->field_08 = 0;

    if (savedWingman == -1) {
        g_acObjectType[1] = savedWingmanType;
        g_asObjectDamage[1] = savedWingmanDamage;
        g_nYourWingman = savedWingman;
    }
    ReleasePacketSlot(&g_pCutsceneCockpitPalette);
    g_13_pActiveCutscenePixelsAddress = 0;
    ReleasePacketSlot((void **)&g_pCampaignGlobals);
    fprintf(stderr, "Cutscene completed; exiting.\n");
}
