#include "game.h"

int main(int argumentCount, char **arguments)
{
    short objective;

    /* Communications clean up all eight objectives even when the table has
     * no terminating type.  The final condition reads one entry ahead. */
    g_13_stMissionHeaderBlock.entryNavPoint = 0;
    g_13_stMissionHeaderBlock.wingmanMissionShip = -1;
    g_cMissionObjectiveCount = MISSION_OBJECTIVE_COUNT;
    for (objective = 0; objective < MISSION_OBJECTIVE_COUNT; objective++) {
        g_aMissionObjectives[objective].type = 0;
        g_aMissionObjectives[objective].index = 0;
        g_aMissionObjectives[objective].flags = 1;
    }
    cleanup_objectives();
    for (objective = 0; objective < MISSION_OBJECTIVE_COUNT; objective++) {
        if (g_aMissionObjectives[objective].flags != 3)
            return 1;
    }

    /* An explicit terminator must still stop cleanup before later entries,
     * even when those entries have been visited. */
    for (objective = 0; objective < MISSION_OBJECTIVE_COUNT; objective++)
        g_aMissionObjectives[objective].flags = 1;
    g_aMissionObjectives[3].type = -1;
    cleanup_objectives();
    for (objective = 0; objective < MISSION_OBJECTIVE_COUNT; objective++) {
        if (g_aMissionObjectives[objective].flags !=
            (objective < 3 ? 3 : 1))
            return 1;
    }

    return 0;
}
