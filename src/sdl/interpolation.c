/* Spaceflight frame interpolation (issue #69), after wc1-re's.
 *
 * WC2 steps its spaceflight once per frame. RunSpaceFlight waits for its
 * frame timer (g_nSpaceFramePeriod, 50 ms in the port) and DIBslamReal for
 * the frame rate cap (g_fSpaceFlightFrameRate, 20 by default), so flight
 * advances about 18 times a second. This keeps that step and its timing,
 * and fills the wait before the next step with frames drawn between the
 * last two steps: each object is moved and turned part of the way from
 * where the previous step left it, and the view is drawn again and
 * presented. The cockpit around the view keeps the step's own drawing.
 * The autopilot's fly-by and the death camera, which step the simulation
 * in loops of their own paced by the cap alone, are filled in the same way.
 *
 * An in-between frame runs the drawing half of RenderSpaceViewFrame with
 * the state it writes saved and restored, so the simulation never sees it.
 * What RunSpaceFlight drew over the view after it -- update_cockpit's
 * instruments, which the full-screen HUD draws over the view every step,
 * and the HUD message -- is laid over the new view as the step drew it,
 * and the message is drawn again. The mouse-steering pointer is drawn
 * where the mouse is at the time. The frame does not respawn stars or place
 * exhaust, which call rand(), nor advance the missile lock, engage the ITTS
 * or blink the target bracket, and it never loads a capital ship's view.
 * What is shown runs one step behind the simulation. */
#include "game.h"

#include "video_internal.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FLIGHT_SCREEN_PIXELS (SDL_FRAME_WIDTH * SDL_FRAME_HEIGHT)

/* Orientation vectors are Q8; a step turning more than this per component
   (about 22 degrees) is a cut, such as a view change, not motion. */
#define INTERPOLATION_MAX_TURN 96

typedef struct SavedRange {
    void *data;
    size_t size;
} SavedRange;

#define SAVED(name) {(void *)&(name), sizeof(name)}

/* Everything an in-between frame writes: the poses it moves, the slot
   draw_nav_pointer takes or frees, what the view transform, sorting and
   drawing produce, the HUD's lock, lead and bracket state, the HUD
   message's, the cockpit display mode RenderSpaceViewFrame swaps around
   its copy, the mouse-steering pointer, the screen's stale flag, which
   drawing to the screen sets after the step's present has cleared it, and
   the screen viewport, whose top dump_buffer_to_screen moves around its
   copy: the death camera's frames run inside that copy, which presents. */
static const SavedRange g_aSavedFlightState[] = {
    SAVED(g_aeObjectClass), SAVED(g_acObjectOwner), SAVED(g_nNavPointerObject),
    SAVED(g_aShipPosition), SAVED(g_aShipForwardVector),
    SAVED(g_aShipRightVector), SAVED(g_aShipUpVector),
    SAVED(g_asObjectDistance), SAVED(g_asObjectScreenX),
    SAVED(g_asObjectScreenY), SAVED(g_aObjectViewPosition),
    SAVED(g_asObjectScreenScale), SAVED(g_asObjectScreenAngle),
    SAVED(g_asObjectFlip), SAVED(g_asObjectViewFrame), SAVED(g_apObjectShape),
    SAVED(g_asLoadedShipViewFrame), SAVED(g_abObjectShapeOwned),
    SAVED(g_asObjectDrawX), SAVED(g_asObjectDrawY),
    SAVED(g_56_anStoredObjectSortedIndicesData),
    SAVED(g_11_anPlacedObjectDepths),
    SAVED(g_bTargetLockActive), SAVED(g_bTargetLockMode),
    SAVED(g_bMissileLockAcquired), SAVED(g_nTargetLeadIndicatorX),
    SAVED(g_1_nTargetLeadingIndicatorY), SAVED(g_2_cEarlierTargetObjectValue),
    SAVED(g_stCommTargetBracketBounds), SAVED(g_stTargetBracketBounds),
    SAVED(g_nTargetLockMarkerAngle), SAVED(g_nTargetLockMarkerX),
    SAVED(g_36_nTargetLockonMarkerY), SAVED(g_nTargetCameraZoom),
    SAVED(g_nCockpitResetState), SAVED(g_bTargetBracketVisible),
    SAVED(g_pszPendingHudMessage), SAVED(g_pszDisplayedHudMessage),
    SAVED(g_18_nHudMessageColour), SAVED(g_3_cOverlayMessageView),
    SAVED(g_bCaptureHudMessageBackground), SAVED(g_nHudMessageBackgroundDepth),
    SAVED(g_29_pCurrentTextContext), SAVED(g_cLastPilotHandFrame),
    SAVED(g_nCockpitDisplayMode), SAVED(g_bRenderingSpaceFrame),
    SAVED(g_37_stPersonnelMousePointerState), SAVED(g_4_bScreenStale),
    SAVED(g_101_stScrViewInfo)
};

static int g_bFrameInterpolation;
static int g_bFlightFrameDeferred;
static int g_bInterpolatedPass;
static int g_bPreviousPosesValid;
static int g_bStepFrameDrawn;
static int g_bStepDrewObjects;
static DWORD g_dwStepStartTicks;
/* Whether the step is one of RunSpaceFlight's own: those wait for its
   frame timer as well as the frame rate cap, and draw the instruments and
   the pending HUD message over the view. The autopilot's fly-by and the
   death camera wait for the cap alone and draw nothing over the view. */
static int g_bFlightLoopStep;
/* The steps' period as measured, a running average: SDL fires the frame
   timer a little later than its period asks. */
static long g_nMeasuredStepPeriod;
/* The view and followed object each step was drawn with, and those the
   previous poses were drawn with: a change of either is a cut, which the
   eye does not move across. */
static int g_nDrawnView = -1;
static signed char g_cDrawnViewObject = -1;
static int g_nCapturedView = -1;
static signed char g_cCapturedViewObject = -1;
static DWORD g_dwInterpolatedPresentMs = 16;
static unsigned char *g_pSavedFlightState;
static size_t g_nSavedFlightStateSize;

/* The previous step's poses. */
static FixedVector g_aPreviousPositions[SPACE_OBJECT_COUNT];
static FixedVector g_aPreviousVelocities[SPACE_OBJECT_COUNT];
static FixedVector g_aPreviousForward[SPACE_OBJECT_COUNT];
static FixedVector g_aPreviousRight[SPACE_OBJECT_COUNT];
static FixedVector g_aPreviousUp[SPACE_OBJECT_COUNT];
static short g_aePreviousClass[SPACE_OBJECT_COUNT];
static short g_asPreviousType[SPACE_OBJECT_COUNT];
static signed char g_acPreviousShipType[SPACE_OBJECT_COUNT];

/* Child objects' offsets from their parents, as reposition_fixed_child_objects
   read them in the step's own drawing; it leaves them absolute. */
static short g_asChildScreenX[SPACE_OBJECT_COUNT];
static short g_asChildScreenY[SPACE_OBJECT_COUNT];
static short g_asChildDistance[SPACE_OBJECT_COUNT];
static int g_bChildOffsetsValid;

/* How the step turned each object's sprite, for a ship whose view the
   in-between frame does not load. */
static short g_asStepFlip[SPACE_OBJECT_COUNT];
static short g_asStepScreenAngle[SPACE_OBJECT_COUNT];

/* The screen as RenderSpaceViewFrame left it, the screen the step presents,
   and which of its pixels RunSpaceFlight drew in between. */
static unsigned char g_abFlightViewScreen[FLIGHT_SCREEN_PIXELS];
static unsigned char g_abFlightStepScreen[FLIGHT_SCREEN_PIXELS];
static unsigned char g_abFlightOverlayMask[FLIGHT_SCREEN_PIXELS];
static int g_bFlightViewCaptured;

/* The view buffer the step drew its objects over: cleared at the end of
   the step before, unless an explosion's flash filled it since. */
static unsigned char g_abStepViewBuffer[FLIGHT_SCREEN_PIXELS];
static Viewport g_stStepViewBuffer;
static int g_bStepViewBufferValid;

/* Copies the view buffer's rows to or from the step's copy. */
static int CopyStepViewBuffer(int restore)
{
    const Viewport *view = &g_27_stViewScratchInfo;
    unsigned char *row;
    size_t width;
    size_t offset;
    int y;

    if (view->pixels == 0 || view->rowOffsets == 0 || view->left < 0 ||
        view->top < 0 || view->right < view->left ||
        view->bottom < view->top)
        return 0;
    width = (size_t)(view->right - view->left + 1);
    if (width * (size_t)(view->bottom - view->top + 1) >
        sizeof(g_abStepViewBuffer))
        return 0;
    offset = 0;
    for (y = view->top; y <= view->bottom; y++) {
        row = view->pixels + view->rowOffsets[y] + view->left;
        if (restore)
            memcpy(row, g_abStepViewBuffer + offset, width);
        else
            memcpy(g_abStepViewBuffer + offset, row, width);
        offset += width;
    }
    return 1;
}

void SdlSetFrameInterpolation(int enabled)
{
    g_bFrameInterpolation = enabled != 0;
}

int SdlDrawingInterpolatedFrame(void)
{
    return g_bInterpolatedPass;
}

/* RunSpaceFlight, at its start and end, and the death sequence at its end:
   nothing to interpolate from, and no frame left waiting. */
void SdlResetFrameInterpolation(void)
{
    g_bPreviousPosesValid = 0;
    g_bChildOffsetsValid = 0;
    g_bFlightFrameDeferred = 0;
    g_bStepFrameDrawn = 0;
    g_bFlightViewCaptured = 0;
}

/* Before each Update_3Space of a loop that draws spaceflight:
   RunSpaceFlight, the autopilot's fly-by and the death camera. */
void SdlCaptureFlightPoses(void)
{
    long expected;
    long measured;
    DWORD now;
    short obj;

    if (!g_bFrameInterpolation)
        return;
    now = timeGetTime();
    /* A step that took far longer than the timer and cap allow (a load, or
       a menu opened from the controls) says nothing about the cadence. */
    if (g_bPreviousPosesValid && g_bFlightLoopStep) {
        expected = (long)g_nSpaceFramePeriod * 1000 / 60;
        if (expected < g_17_nFramePeriodMillis)
            expected = g_17_nFramePeriodMillis;
        measured = (Sint32)(now - g_dwStepStartTicks);
        if (measured >= expected / 2 && measured <= 2 * expected)
            g_nMeasuredStepPeriod = g_nMeasuredStepPeriod == 0
                ? measured : (g_nMeasuredStepPeriod * 7 + measured) / 8;
    }
    g_dwStepStartTicks = now;
    g_bStepFrameDrawn = 0;
    g_nCapturedView = g_nDrawnView;
    g_cCapturedViewObject = g_cDrawnViewObject;
    for (obj = 0; obj < SPACE_OBJECT_COUNT; obj++) {
        g_aPreviousPositions[obj] = g_aShipPosition[obj];
        g_aPreviousVelocities[obj] = g_aShipVelocity[obj];
        g_aPreviousForward[obj] = g_aShipForwardVector[obj];
        g_aPreviousRight[obj] = g_aShipRightVector[obj];
        g_aPreviousUp[obj] = g_aShipUpVector[obj];
        g_aePreviousClass[obj] = g_aeObjectClass[obj];
        g_asPreviousType[obj] = g_asObjectType[obj];
        g_acPreviousShipType[obj] = g_acObjectType[obj];
    }
    g_bPreviousPosesValid = 1;
}

/* DrawSpaceSceneFrame, just before reposition_fixed_child_objects; whether
   it draws the objects (not in an explosion's first flashes). Nothing has
   been drawn into the view buffer yet. */
void SdlSnapshotFlightChildren(int drawObjects)
{
    if (!g_bFrameInterpolation || g_bInterpolatedPass)
        return;
    memcpy(g_asChildScreenX, g_asObjectScreenX, sizeof(g_asChildScreenX));
    memcpy(g_asChildScreenY, g_asObjectScreenY, sizeof(g_asChildScreenY));
    memcpy(g_asChildDistance, g_asObjectDistance, sizeof(g_asChildDistance));
    g_bChildOffsetsValid = 1;
    g_bStepFrameDrawn = 1;
    g_bStepDrewObjects = drawObjects != 0;
    g_bStepViewBufferValid = CopyStepViewBuffer(0);
    g_stStepViewBuffer = g_27_stViewScratchInfo;
    g_nDrawnView = g_nCurrentView;
    g_cDrawnViewObject = g_cViewObject;
}

/* RunSpaceFlight, after RenderSpaceViewFrame. */
void SdlCaptureFlightView(void)
{
    if (!g_bFrameInterpolation || !g_bStepFrameDrawn ||
        g_18_pDisplayPixelBuffer == 0)
        return;
    memcpy(g_abFlightViewScreen, g_18_pDisplayPixelBuffer,
           sizeof(g_abFlightViewScreen));
    g_bFlightViewCaptured = 1;
}

/* Before the step's frame is presented: when the step drew one,
   DIBslamReal leaves its present to ThrottleFrameAndDrawFps, which
   interpolates up to it. flightLoop tells RunSpaceFlight's own steps, which
   draw over the view after SdlCaptureFlightView, from the fly-by's and the
   death camera's, which draw nothing over it. */
void SdlDeferFlightFrame(int flightLoop)
{
    const unsigned char *screen;
    size_t pixel;

    screen = g_18_pDisplayPixelBuffer;
    g_bFlightFrameDeferred =
        g_bFrameInterpolation && g_bChildOffsetsValid && g_bStepFrameDrawn &&
        (g_bFlightViewCaptured || !flightLoop) && screen != 0;
    g_bFlightLoopStep = flightLoop != 0;
    g_bStepFrameDrawn = 0;
    g_bFlightViewCaptured = 0;
    if (!g_bFlightFrameDeferred)
        return;
    if (!flightLoop) {
        memset(g_abFlightOverlayMask, 0, sizeof(g_abFlightOverlayMask));
        return;
    }
    memcpy(g_abFlightStepScreen, screen, sizeof(g_abFlightStepScreen));
    for (pixel = 0; pixel < FLIGHT_SCREEN_PIXELS; pixel++)
        g_abFlightOverlayMask[pixel] =
            g_abFlightStepScreen[pixel] != g_abFlightViewScreen[pixel];
}

int SdlFlightFrameDeferred(void)
{
    return g_bFlightFrameDeferred;
}

/* get_right_shape, when an object turned to a view other than the one
   loaded for it: an in-between frame keeps the step's view, turned as the
   step turned it, rather than load another. */
int SdlKeepStepShipView(short obj)
{
    if (!g_bInterpolatedPass || obj < 0 || obj >= SPACE_OBJECT_COUNT)
        return 0;
    g_asObjectFlip[obj] = g_asStepFlip[obj];
    g_asObjectScreenAngle[obj] = g_asStepScreenAngle[obj];
    return 1;
}

static int SaveFlightState(void)
{
    size_t index;
    size_t offset;

    if (g_pSavedFlightState == 0) {
        g_nSavedFlightStateSize = 0;
        for (index = 0; index < SDL_arraysize(g_aSavedFlightState); index++)
            g_nSavedFlightStateSize += g_aSavedFlightState[index].size;
        g_pSavedFlightState = malloc(g_nSavedFlightStateSize);
        if (g_pSavedFlightState == 0)
            return 0;
    }
    offset = 0;
    for (index = 0; index < SDL_arraysize(g_aSavedFlightState); index++) {
        memcpy(g_pSavedFlightState + offset, g_aSavedFlightState[index].data,
               g_aSavedFlightState[index].size);
        offset += g_aSavedFlightState[index].size;
    }
    return 1;
}

static void RestoreFlightState(void)
{
    size_t index;
    size_t offset;

    offset = 0;
    for (index = 0; index < SDL_arraysize(g_aSavedFlightState); index++) {
        memcpy(g_aSavedFlightState[index].data, g_pSavedFlightState + offset,
               g_aSavedFlightState[index].size);
        offset += g_aSavedFlightState[index].size;
    }
}

static int Lerp(int previous, int current, int alpha)
{
    return previous + (int)(((int64_t)current - previous) * alpha / 256);
}

static void LerpVector(FixedVector *current, const FixedVector *previous,
                       int alpha)
{
    current->x = Lerp(previous->x, current->x, alpha);
    current->y = Lerp(previous->y, current->y, alpha);
    current->z = Lerp(previous->z, current->z, alpha);
}

static int64_t LargestComponent(const FixedVector *vector)
{
    int64_t x = vector->x < 0 ? -(int64_t)vector->x : vector->x;
    int64_t y = vector->y < 0 ? -(int64_t)vector->y : vector->y;
    int64_t z = vector->z < 0 ? -(int64_t)vector->z : vector->z;

    return x > y ? (x > z ? x : z) : (y > z ? y : z);
}

static int64_t LargestDifference(const FixedVector *current,
                                 const FixedVector *previous)
{
    int64_t x = (int64_t)current->x - previous->x;
    int64_t y = (int64_t)current->y - previous->y;
    int64_t z = (int64_t)current->z - previous->z;

    x = x < 0 ? -x : x;
    y = y < 0 ? -y : y;
    z = z < 0 ? -z : z;
    return x > y ? (x > z ? x : z) : (y > z ? y : z);
}

/* How far an object can move in a step: its velocity before or after. */
static int64_t StepSpeed(int obj)
{
    int64_t speed;

    if (obj < 0 || obj >= SPACE_OBJECT_COUNT)
        return 0;
    speed = LargestComponent(&g_aShipVelocity[obj]);
    if (LargestComponent(&g_aPreviousVelocities[obj]) > speed)
        speed = LargestComponent(&g_aPreviousVelocities[obj]);
    return speed;
}

/* The eye follows the player, or in an external view the ship it watches
   or the target, and some views place it there without giving it a
   velocity of its own. */
static int64_t EyeStepSpeed(void)
{
    const int followed[] = {
        EYE_OBJECT, 0, g_cViewObject, g_nExternalViewShip, g_acShipTarget[0]
    };
    int64_t speed = 0;
    size_t index;

    for (index = 0; index < SDL_arraysize(followed); index++) {
        if (StepSpeed(followed[index]) > speed)
            speed = StepSpeed(followed[index]);
    }
    return speed;
}

/* Moves each object part of the way back to its previous pose. A slot whose
   object changed, a jump its velocity does not explain (a respawn, the
   autopilot or a jump) and a sharp turn (a view change) show the step's own
   pose. Stars and dust are respawned by direction, and children follow
   their parents. */
static void InterpolatePoses(int alpha)
{
    int64_t moved;
    int64_t speed;
    short obj;

    for (obj = 0; obj < SPACE_OBJECT_COUNT; obj++) {
        /* The eye is no object of its own: the view sets it from the
           player's or the camera's pose every step. A change of view, or of
           the object an external view watches, is a cut. */
        if (obj == EYE_OBJECT &&
            (g_nCapturedView != g_nDrawnView ||
             g_cCapturedViewObject != g_cDrawnViewObject))
            continue;
        if (obj != EYE_OBJECT &&
            (g_aeObjectClass[obj] == OBJECT_CLASS_NULL ||
             g_aeObjectClass[obj] != g_aePreviousClass[obj] ||
             g_asObjectType[obj] != g_asPreviousType[obj] ||
             g_acObjectType[obj] != g_acPreviousShipType[obj] ||
             g_aeObjectClass[obj] == OBJECT_CLASS_STAR ||
             g_aeObjectClass[obj] == OBJECT_CLASS_DUST ||
             g_aeObjectClass[obj] == OBJECT_CLASS_FIXED_OBJECT ||
             obj == g_nNavPointerObject))
            continue;
        moved = LargestDifference(&g_aShipPosition[obj],
                                  &g_aPreviousPositions[obj]);
        speed = obj == EYE_OBJECT ? EyeStepSpeed() : StepSpeed(obj);
        if (moved <= 3 * speed + 256)
            LerpVector(&g_aShipPosition[obj], &g_aPreviousPositions[obj],
                       alpha);
        if (LargestDifference(&g_aShipForwardVector[obj],
                              &g_aPreviousForward[obj]) <=
                INTERPOLATION_MAX_TURN &&
            LargestDifference(&g_aShipRightVector[obj],
                              &g_aPreviousRight[obj]) <=
                INTERPOLATION_MAX_TURN &&
            LargestDifference(&g_aShipUpVector[obj], &g_aPreviousUp[obj]) <=
                INTERPOLATION_MAX_TURN) {
            LerpVector(&g_aShipForwardVector[obj], &g_aPreviousForward[obj],
                       alpha);
            LerpVector(&g_aShipRightVector[obj], &g_aPreviousRight[obj],
                       alpha);
            LerpVector(&g_aShipUpVector[obj], &g_aPreviousUp[obj], alpha);
        }
    }
}

/* Where the mouse is now, in the game's 320 by 200 as SdlHandleMouseEvent
   maps it; 0 when it is over no window of the game. */
static int LiveMousePointer(short *x, short *y)
{
    SDL_Window *window;
    int windowX;
    int windowY;
    int logicalX;
    int logicalY;

    SDL_PumpEvents();
    window = SDL_GetMouseFocus();
    if (window == 0)
        return 0;
    SDL_GetMouseState(&windowX, &windowY);
    if (!SdlMapWindowToLogical(window, windowX, windowY, &logicalX, &logicalY))
        return 0;
    *x = (short)SDL_max(0, SDL_min(logicalX, 319));
    *y = (short)SDL_max(0, SDL_min(logicalY, 199));
    return 1;
}

/* The drawing half of RenderSpaceViewFrame at alpha/256 of the way from
   the previous step to this one, under what the step drew after it;
   presented and then undone. */
void SdlDrawInterpolatedFlightFrame(int alpha)
{
    unsigned char *screen;
    int displayMode;
    size_t pixel;
    short hidePlayer;
    short obj;
    short pointerX;
    short pointerY;

    if (!SaveFlightState())
        return;
    /* The copy to the screen presents the frame unless it is part of a
       space frame, as it is in RunSpaceFlight's own drawing. */
    g_bRenderingSpaceFrame = 1;
    /* The mouse-steering pointer follows the mouse between the steps too;
       the steps still read the mouse as before. */
    if (g_nCockpitControlState == 1 && LiveMousePointer(&pointerX, &pointerY)) {
        g_37_stPersonnelMousePointerState.x = pointerX;
        g_37_stPersonnelMousePointerState.y = pointerY;
    }
    if (g_bPreviousPosesValid && alpha < 256)
        InterpolatePoses(alpha);
    memcpy(g_asStepFlip, g_asObjectFlip, sizeof(g_asStepFlip));
    memcpy(g_asStepScreenAngle, g_asObjectScreenAngle,
           sizeof(g_asStepScreenAngle));
    g_bInterpolatedPass = 1;
    transform_objects_to_your_view();
    for (obj = 0; obj < SPACE_OBJECT_COUNT; obj++) {
        if (g_aeObjectClass[obj] != OBJECT_CLASS_FIXED_OBJECT)
            continue;
        g_asObjectScreenX[obj] = g_asChildScreenX[obj];
        g_asObjectScreenY[obj] = g_asChildScreenY[obj];
        g_asObjectDistance[obj] = g_asChildDistance[obj];
    }
    reposition_fixed_child_objects();
    BuildObjectDepthOrder();
    if (g_bStepViewBufferValid &&
        memcmp(&g_stStepViewBuffer, &g_27_stViewScratchInfo,
               sizeof(g_stStepViewBuffer)) == 0)
        CopyStepViewBuffer(1);
    SdlBeginSpaceFrame(
        (const struct ScreenViewportGeometry *)g_15_pScreenViewportGeometry,
        (int)g_1_cStoredViewDisplayModeValue, g_nCockpitDisplayMode > 0,
        (unsigned char)g_cPrimaryViewBufferColour);
    hidePlayer = 0;
    if (g_bJumpSequenceActive != 0 &&
        g_asShipManeuver[0] == MANEUVER_WARPING_OUT) {
        hidePlayer = 1;
        g_aeObjectClass[0] = OBJECT_CLASS_NULL;
    }
    if (g_bStepDrewObjects)
        draw_sorted_objects_to_buffer();
    if (hidePlayer != 0)
        g_aeObjectClass[0] = OBJECT_CLASS_SHIP;
    if (g_nCurrentView == 0 || g_nCurrentView == 4)
        overlay_head_up_display();
    RestoreCockpitExplosionIfVisible();
    displayMode = g_nCockpitDisplayMode;
    if (g_nCockpitDisplayMode > 0)
        g_nCockpitDisplayMode = 9;
    dump_buffer_to_screen();
    g_nCockpitDisplayMode = displayMode;
    if (g_nCurrentView == 0 || g_nCurrentView == 4)
        RestoreTransientCockpitGraphics();
    ClearViewport(&g_27_stViewScratchInfo,
                  (unsigned char)g_cPrimaryViewBufferColour);
    /* The instruments and the message as the step drew them; the message
       is drawn again so that its hi-res letters follow the new view. */
    screen = g_18_pDisplayPixelBuffer;
    for (pixel = 0; pixel < FLIGHT_SCREEN_PIXELS; pixel++) {
        if (g_abFlightOverlayMask[pixel])
            screen[pixel] = g_abFlightStepScreen[pixel];
    }
    if (g_bFlightLoopStep && g_pszPendingHudMessage != 0)
        SetHudTextColour(g_pszPendingHudMessage, g_18_nHudMessageColour);
    g_bInterpolatedPass = 0;
    SdlPresentIndexedFrame(g_18_pDisplayPixelBuffer, g_3_abColourShadow);
    RestoreFlightState();
}

/* ThrottleFrameAndDrawFps, before waiting for the frame rate cap: frames
   until the step is due, each drawn at its expected display time. The step
   is due when both the cap's deadline and RunSpaceFlight's frame timer,
   started with the step, have passed. The last frame ends at the step
   itself, as the game would have presented it. */
void SdlPresentInterpolatedFlightFrames(DWORD deadline, long interval)
{
    long timerInterval;
    long remaining;
    long elapsed;
    DWORD due;
    DWORD start;
    DWORD now;
    int lastAlpha;
    int alpha;
    int frames;

    if (!g_bFlightFrameDeferred || g_bInterpolatedPass)
        return;
    g_bFlightFrameDeferred = 0;
    if (g_bFlightLoopStep) {
        timerInterval = (long)g_nSpaceFramePeriod * 1000 / 60;
        if (interval < timerInterval)
            interval = timerInterval;
        if (g_nMeasuredStepPeriod > interval &&
            g_nMeasuredStepPeriod <= 2 * interval)
            interval = g_nMeasuredStepPeriod;
        due = g_dwStepStartTicks + (DWORD)interval;
        if (deadline != 0 && (Sint32)(deadline - due) > 0)
            due = deadline;
    } else {
        /* The fly-by and the death camera wait for the cap alone. */
        due = deadline != 0 ? deadline : g_dwStepStartTicks + (DWORD)interval;
    }
    frames = 0;
    lastAlpha = 0;
    for (;;) {
        now = timeGetTime();
        /* Signed: a step drawn late is already due. */
        remaining = (Sint32)(due - now);
        if (frames > 0 && remaining < (long)g_dwInterpolatedPresentMs)
            break;
        elapsed = interval - remaining + (long)g_dwInterpolatedPresentMs;
        alpha = interval > 0 ? (int)(elapsed * 256 / interval) : 256;
        /* The first frame guessed its present time from the step before. */
        if (alpha < lastAlpha)
            alpha = lastAlpha;
        if (alpha < 0)
            alpha = 0;
        if (alpha > 256)
            alpha = 256;
        lastAlpha = alpha;
        start = timeGetTime();
        SdlDrawInterpolatedFlightFrame(alpha);
        g_dwInterpolatedPresentMs = timeGetTime() - start;
        if (g_dwInterpolatedPresentMs < 1)
            g_dwInterpolatedPresentMs = 1;
        if (g_dwInterpolatedPresentMs > 50)
            g_dwInterpolatedPresentMs = 50;
        frames++;
        if (alpha >= 256)
            break;
    }
}
