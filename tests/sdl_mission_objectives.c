#include "game.h"

int main(int argumentCount, char **arguments)
{
    short objective;

    /* Communications clean up all eight objectives even when the table has
     * no terminating type.  The final condition reads one entry ahead. */
    g_stMissionHeader_005d3e70.entryNavPoint = 0;
    g_stMissionHeader_005d3e70.wingmanMissionShip = -1;
    g_cMissionObjectiveCount_00493294 = MISSION_OBJECTIVE_COUNT;
    for (objective = 0; objective < MISSION_OBJECTIVE_COUNT; objective++) {
        g_aMissionObjectives_004932a8[objective].type = 0;
        g_aMissionObjectives_004932a8[objective].index = 0;
        g_aMissionObjectives_004932a8[objective].flags = 1;
    }
    cleanup_objectives();
    for (objective = 0; objective < MISSION_OBJECTIVE_COUNT; objective++) {
        if (g_aMissionObjectives_004932a8[objective].flags != 3)
            return 1;
    }

    /* An explicit terminator must still stop cleanup before later entries,
     * even when those entries have been visited. */
    for (objective = 0; objective < MISSION_OBJECTIVE_COUNT; objective++)
        g_aMissionObjectives_004932a8[objective].flags = 1;
    g_aMissionObjectives_004932a8[3].type = -1;
    cleanup_objectives();
    for (objective = 0; objective < MISSION_OBJECTIVE_COUNT; objective++) {
        if (g_aMissionObjectives_004932a8[objective].flags !=
            (objective < 3 ? 3 : 1))
            return 1;
    }

    return 0;
}
