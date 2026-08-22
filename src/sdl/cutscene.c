#include "game.h"

int g_bSdlCutsceneOnly;

void SdlRunSelectedCampaignCutscene(void)
{
    short savedWingman;
    signed char savedWingmanType;
    short savedWingmanDamage;

    g_nSelectedCampaignSlot_005d3bf2 = 0;
    ReleaseSpaceflightResources();
    LoadStartingCampaignGlobals(g_nSelectedCampaignSlot_005d3bf2);
    if (g_pCampaignGlobals_00499c94 == 0)
        ReportFatalErrorCode("015");

    g_pCampaignGlobals_00499c94->series =
        g_nDirectSeries_0049d79c;
    g_pCampaignGlobals_00499c94->mission =
        g_nDirectMission_0049d79a;
    g_pCampaignGlobals_00499c94->field_08 = 0;
    g_pCampaignGlobals_00499c94->arcadeState = 0;
    g_bDeveloperCampaignReady_004926c4 = 0;

    savedWingman = g_nYourWingman_0049346c;
    if (savedWingman == -1) {
        /* Direct post-flight scenes need the intact wingman state that the
         * skipped mission would normally leave in object slot 1. */
        savedWingmanType = g_acObjectType_00493980[1];
        savedWingmanDamage = g_asObjectDamage_00495178[1];
        g_nYourWingman_0049346c = 1;
        g_acObjectType_00493980[1] = OBJECT_DATA_LASER_CANNON;
        g_asObjectDamage_00495178[1] = 0;
    }

    g_pCutsceneCockpitPalette_00499c0c =
        AllocateScenePointerTable(1, 0x3420, 2, "HB1");
    g_pActiveCutscenePixels_005c83dc =
        g_pCutsceneCockpitPalette_00499c0c;
    g_bCutsceneViewportPreallocated_00499c4c = 1;
    RunCampaignScript(g_nSelectedCampaignSlot_005d3bf2);
    g_bCutsceneViewportPreallocated_00499c4c = 0;

    g_pCampaignGlobals_00499c94->field_08 =
        g_pCampaignGlobals_00499c94->field_0a;
    fprintf(stderr,
            "Running campaign cutscene: series %d, mission %d, entry %d.\n",
            g_pCampaignGlobals_00499c94->series,
            g_pCampaignGlobals_00499c94->mission,
            g_pCampaignGlobals_00499c94->field_08);
    g_bRoomTransitionAnimationEnabled_00499c00 = 0;
    RunCampaignScript(g_nSelectedCampaignSlot_005d3bf2);
    g_pCampaignGlobals_00499c94->field_08 = 0;

    if (savedWingman == -1) {
        g_acObjectType_00493980[1] = savedWingmanType;
        g_asObjectDamage_00495178[1] = savedWingmanDamage;
        g_nYourWingman_0049346c = savedWingman;
    }
    ReleasePacketSlot(&g_pCutsceneCockpitPalette_00499c0c);
    g_pActiveCutscenePixels_005c83dc = 0;
    ReleasePacketSlot((void **)&g_pCampaignGlobals_00499c94);
    fprintf(stderr, "Cutscene completed; exiting.\n");
}
