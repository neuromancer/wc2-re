/*
 *  DirectDraw frame-timing startup state.
 *
 *  Address range 0x45CCB0-0x45CD2B (see docs/ORDER.md).
 *  Boundary evidence: the target's CRT initializer table points to the two
 *  scalar initialization pairs in this range immediately before the frame
 *  throttle routine.
 */
#include "game.h"

static int g_nDefaultFrameRate_0049ce88 = 70;
static int g_nDisplayFrameRate_0049ce8c = 70;

long g_nFramePeriodMilliseconds_005c343c =
    (long)(1000.0 / g_nDisplayFrameRate_0049ce8c);
float g_fPreviousFrameRate_005c3440 =
    (float)g_nDisplayFrameRate_0049ce8c;
