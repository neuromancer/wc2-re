/*
 *  Player ejection and stranding sequences.
 *
 *  Retail object 0x438020-0x4380AF (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x438020 */
void ShowPlayerEjectionSequence(void)
{
    InputPumpContext *savedInputContext;

    savedInputContext = g_stPersonnelMousePointerState_005c8470.context;
    ejection_sequence((short)(g_stCurrentPilotProfile_00493408.playerShipType + 0x19), 0);
    if (savedInputContext != 0)
        SetInputViewport(savedInputContext->viewport);
    ConfigureInputPump(1, 0);
    g_nInterfaceEventMode_005c8d3c = 1;
    g_nEventRepeatDelayState_005c80d6 = 0x14;
}

/* Function start: 0x438080 */
void ShowPlayerStrandingSequence(void)
{
    ejection_sequence((short)(g_stCurrentPilotProfile_00493408.playerShipType + 500), 0);
}
