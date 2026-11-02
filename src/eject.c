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

    savedInputContext = g_pActiveInputContext_005c8487;
    ejection_sequence((short)(g_nPlayerShipType_00493464 + 0x19), 0);
    if (savedInputContext != 0)
        SetInputViewport(savedInputContext->viewport);
    ConfigureInputPump(1, 0);
    g_nUiInputMode_005c8d3c = 1;
    g_nInputRepeatDelay_005c80d6 = 0x14;
}

/* Function start: 0x438080 */
void ShowPlayerStrandingSequence(void)
{
    ejection_sequence((short)(g_nPlayerShipType_00493464 + 500), 0);
}
