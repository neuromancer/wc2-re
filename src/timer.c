/*
 *  Multimedia frame timer callback.
 *
 *  Retail object 0x40A2D0-0x40A39F (link order; see docs/ORDER.md).
 */
#include "game.h"

/* Function start: 0x40A2D0 */
void CALLBACK FrameTimerCallback(UINT timerId, UINT message,
                                 DWORD user, DWORD first, DWORD second)
{
    (void)timerId;
    (void)message;
    (void)user;
    (void)first;
    (void)second;
    g_nFrameTimerPending_005c844c = 0;
}

/* Function start: 0x40A2E7 */
void SetMultimediaTimerCallback(int period)
{
    int milliseconds = period * 1000 / 60;

    if (period == 0) {
        g_nFrameTimerPending_005c844c = 0;
        if (g_uFrameTimerEventId_005d3bec != 0) {
            timeKillEvent(g_uFrameTimerEventId_005d3bec);
            g_uFrameTimerEventId_005d3bec = 0;
        }
    } else {
        if (g_uFrameTimerEventId_005d3bec != 0) {
            timeKillEvent(g_uFrameTimerEventId_005d3bec);
            g_uFrameTimerEventId_005d3bec = 0;
        }
        g_nFrameTimerPending_005c844c = 1;
        g_uFrameTimerEventId_005d3bec = timeSetEvent(milliseconds, milliseconds,
                                    FrameTimerCallback, 0, 0);
    }
    return;
}
