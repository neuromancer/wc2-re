#include "game.h"

int main(void)
{
    const short scalingCases[][3] = {
        {0, 5, 1}, {3, 2, 1}, {50, 1, 10},
        {127, 5, 127}, {127, 10, 200}, {200, 5, 200}
    };
    const short invalidMissions[][2] = {
        {-1, 0}, {14, 0}, {0, -1}, {0, 4}, {7, -1}, {7, 4}
    };
    short series;
    short mission;
    short expected;
    short actual;
    unsigned int index;

    /* Distinct loaded values expose a shifted row or column as well as the
     * retail lookup's dependence on adjacency between separate globals. */
    for (index = 0; index < 52; index++)
        g_asDifficultyLevels_004930a8[index] = (short)(20 + index);
    g_nAdaptiveDifficulty_005d3844 = 5;
    for (series = 0; series <= 13; series++) {
        for (mission = 0; mission < 4; mission++) {
            g_nCurrentSeries_005c5870 = series;
            g_nCurrentMission_005c5878 = mission;
            if (series == 0)
                expected = mission == 0 ? 108 : 1;
            else
                expected = (short)(20 + (series - 1) * 4 + mission);
            actual = GetAdaptiveTurnRate();
            if (actual != expected) {
                fprintf(stderr, "series %d mission %d: turn rate %d, expected %d\n",
                        series, mission, actual, expected);
                return 1;
            }
        }
    }

    /* The reported series must still apply adaptive scaling and the retail
     * minimum/maximum rules to the value loaded from difflevl.000. */
    g_nCurrentSeries_005c5870 = 7;
    g_nCurrentMission_005c5878 = 2;
    for (index = 0; index < sizeof(scalingCases) / sizeof(scalingCases[0]);
         index++) {
        g_asDifficultyLevels_004930a8[26] = scalingCases[index][0];
        g_nAdaptiveDifficulty_005d3844 = scalingCases[index][1];
        if (GetAdaptiveTurnRate() != scalingCases[index][2]) {
            fprintf(stderr, "adaptive turn rate scaling case %u failed\n", index);
            return 1;
        }
    }

    g_nAdaptiveDifficulty_005d3844 = 5;
    for (index = 0;
         index < sizeof(invalidMissions) / sizeof(invalidMissions[0]);
         index++) {
        g_nCurrentSeries_005c5870 = invalidMissions[index][0];
        g_nCurrentMission_005c5878 = invalidMissions[index][1];
        if (GetAdaptiveTurnRate() != 1) {
            fprintf(stderr, "invalid mission case %u did not use the minimum\n",
                    index);
            return 1;
        }
    }
    return 0;
}
