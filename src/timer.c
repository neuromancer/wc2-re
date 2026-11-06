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
    g_5_nFrameTimerWaitTicksValue = 0;
}

/* Function start: 0x40A2E7 */
void SetMultimediaTimerCallback(int period)
{
    /* Retail reserves a dword it never touches above the period. */
    int resolution;
    int milliseconds = period * 1000 / 60;

    if (period == 0) {
        g_5_nFrameTimerWaitTicksValue = 0;
        if (g_uFrameTimerEventId != 0) {
            timeKillEvent(g_uFrameTimerEventId);
            g_uFrameTimerEventId = 0;
        }
        return;
    }
    if (g_uFrameTimerEventId != 0) {
        timeKillEvent(g_uFrameTimerEventId);
        g_uFrameTimerEventId = 0;
    }
    g_5_nFrameTimerWaitTicksValue = 1;
    g_uFrameTimerEventId = timeSetEvent(milliseconds, milliseconds,
                                FrameTimerCallback, 0, 0);
    return;
}

/* Linker commons defined here.  LINK lays a unit's commons out by the
 * identifiers' hash keys, so these spellings follow the original addresses. */
unsigned int g_uFrameTimerEventId; /* 0x005d3bec */
