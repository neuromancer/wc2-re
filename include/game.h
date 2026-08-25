/*
 *  ============================================================================
 *  |                            WINGLEADER                                    |
 *  |                  The 3D space combat simulator                           |
 *  |                                                                          |
 *  |          A game of interstellar fighter conflict.                        |
 *  |                                                                          |
 *  |          (c)1989,1990 Chris Roberts.  All rights reserved.               |
 *  ============================================================================
 *
 *  Shared declarations for the Wing Commander (Kilrathi Saga, Win32)
 *  reconstruction.  The banner above is reproduced from the leaked original
 *  main-module header; the module name in that file was
 *  "MAIN GAME C SOURCE MODULE (Main loop etc.)".
 *
 *  The core is C, ported from 16-bit DOS sources.  `int` was 16 bits in the
 *  original, so nearly all game state is `short` here.  Using `int` where the
 *  original used a 16-bit type produces 32-bit operations and breaks the
 *  instruction comparison -- see AGENTS.md.
 */
#ifndef GAME_H
#define GAME_H

#ifdef SDL_PORT
#include "sdl_port.h"
#endif

#ifndef ANALYSIS_BUILD
#ifndef SDL_PORT
#include <windows.h>
#include <ddraw.h>
#include <mmsystem.h>
#include <conio.h>
#include <direct.h>
#include <io.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#endif

/* The original stores addresses in dwords -- IdentityDword is its identity
 * helper for them, and the high-memory paragraph is one such address.  A dword
 * is exactly a pointer on Win32; on LP64 it is half of one, so anything that
 * has to hold an address takes this alias instead and keeps the reference
 * build's `unsigned int` untouched. */
#ifdef SDL_PORT
typedef uintptr_t DwordPtr;
#else
typedef unsigned int DwordPtr;
#endif

/* The in-flight replay snapshots 0x493130-0x4961A4 -- a contiguous 12404-byte
 * span of the original's .data -- in single copies, and restores it the same
 * way.  The reconstruction declares that span as several hundred separate C
 * objects, and no linker is going to lay them out in the original's order, so
 * the copies would move unrelated memory in either direction.  The port leaves
 * the replay recording alone rather than write the wrong bytes to disk and
 * read them back over its own globals; restoring it needs the whole span laid
 * out contiguously first. */
/* The span runs from g_dwCannedSceneSnapshotStart_00493130 to the end of
 * g_asViableTargetDistance_00496190, the last global before the sound lists
 * at 0x4961A8.  Spelled as its width rather than as the difference of the two
 * addresses because the reconstruction does not place them that far apart. */
#define CANNED_SCENE_SNAPSHOT_BYTES 0x3074u

/* Loading an object-type record.  The record on disk has the original's
 * layout: 0xF3 bytes, with the animation, shapeSet and shape pointers taking
 * four bytes each.  The reference build's struct matches that exactly, so the
 * packet is read straight into it.  On LP64 the same struct is wider and every
 * field past `animation` sits four bytes late -- weaponLoadout among them,
 * which is why a ship loaded with no guns -- so the port unpacks instead.  The
 * three pointer fields are junk in the packet and every caller overwrites them
 * immediately, so neither path preserves them. */
#ifdef SDL_PORT
#define LOAD_OBJECT_TYPE_RECORD(fileName, section, record) \
    SdlLoadObjectTypeRecord((fileName), (section), (record))
#else
#define LOAD_OBJECT_TYPE_RECORD(fileName, section, record) \
    LoadPacketIntoBuffer((fileName), (section), (record), 0)
#endif

/* One mouth position per cinematic frame is what the original animates.
 * Nearly every entry in the mouth-duration table is a single 60Hz tick, so
 * without a floor the clock gate only bites at 60 positions a second --
 * the port has to name a cadence instead of inheriting one from however
 * fast the host happens to draw, or the mouth runs the line out well
 * before the speech does. Was 3 ticks (one frame at 20fps, the rate
 * PlayRawSpeechBuffer sets while speech is playing) -- correct for
 * voiced lines, where actual audio length is the real pacing reference
 * regardless of this floor's exact value, but unvoiced lines have
 * nothing else limiting their pace, and 3 measured twice too fast there.
 * 6 ticks (10 positions/sec) is the current best empirical value for the
 * unvoiced case; not derived from a DOS trace. */
#define CUTSCENE_MOUTH_MIN_TICKS 6

/* Same class of bug as CUTSCENE_MOUTH_MIN_TICKS above, but for the general
 * sprite/plane wait opcodes (0x44, 0xaa) that every cutscene object -- not
 * just the speaker's mouth -- uses to pace its own script. A script-authored
 * wait of a tick or two relied on the DOS host's own draw speed to look
 * right; the port's clock gate can resolve it dozens of times a second.
 * Starting from the mouth's proven value as a first guess, not tuned
 * separately yet. */
#define CUTSCENE_SPRITE_MIN_TICKS 6

/* Marks a routine that deliberately indexes out of one global and into the one
 * that follows it.  The original's data layout is what makes those reads land
 * where they are meant to, and the reconstruction reproduces that layout
 * exactly, so the sanitizers have to be told the crossing is the point. */
#ifdef SDL_PORT
#define CROSSES_GLOBALS \
    __attribute__((no_sanitize("address", "array-bounds")))
#else
#define CROSSES_GLOBALS
#endif

/* Tables of host pointers -- scene and cutscene resources, packet reference
 * groups -- are sized four bytes an element throughout, because that is what a
 * pointer was.  On LP64 the same table has to be twice as wide or every entry
 * past the first runs off the end of the allocation. */
#ifdef SDL_PORT
#define HOST_POINTER_SIZE ((short)sizeof(void *))
#else
#define HOST_POINTER_SIZE 4
#endif

#ifdef SDL_PORT
/* Two things the reconstruction inherits from MSVC have to be corrected on the
 * way to a modern C library, and both of them corrupt a diagnostic exactly
 * when something has already gone wrong.
 *
 * The game formats into the original's fixed-size buffers, and the messages
 * that carry a whole file path run past them; on the original that scribbled
 * over dead stack, here it smashes the frame.  __builtin_object_size bounds
 * each one by the size the compiler can prove, and yields (size_t)-1 when it
 * cannot, which leaves the call behaving exactly as it did before.
 *
 * The formats themselves also still carry MSVC's far-pointer modifiers
 * ("%Fs", "%Fp").  Those are a no-op in a flat model, but clang reads the F as
 * a conversion and consumes an argument for it, shifting every value after it.
 * The shims strip the modifier and hand the rest to the C library. */
#define printf   SdlPrintf
#define fprintf  SdlFprintf
#define sprintf(buffer, ...) \
    SdlSnprintf((buffer), __builtin_object_size((buffer), 1), __VA_ARGS__)
#define vsprintf(buffer, format, arguments) \
    SdlVsnprintf((buffer), __builtin_object_size((buffer), 1), (format), \
                    (arguments))
#endif

/* Degrees are the angular unit throughout the game core (the constant lives at
 * DAT_004631b0 in the original); the trig shims convert on the way in. */
#define DEG2RAD 0.017453292519943295

/* The DirectDraw back end uses this release-and-clear shape for its surfaces. */
#define COM_RELEASE(surface) \
    do { if ((surface) != 0) { IDirectDrawSurface_Release(surface); (surface) = 0; } } while (0)

/* --------------------------------------------------------------------------
 * Original 16-bit-era type spellings.
 * -------------------------------------------------------------------------- */
typedef short          INT16;
typedef unsigned short UINT16;
typedef unsigned char  UINT8;
typedef signed char    INT8;

typedef struct IxSound IxSound;
typedef struct IxSample IxSample;

/* The game-side wave unit inspects the leading public fields and invokes the
 * selected methods declared here.  The IX implementation owns the complete
 * C++ object layout. */
struct IxSample {
    unsigned int flags;
#ifdef __cplusplus
    void ix_sample_set_looping(int enabled);
    int ix_sample_load_wav(void *data, int bytes);
    int ix_sample_load_raw(void *data, int bytes, int frequency,
                           int bitsPerSample, int channels);
#endif
};

struct IxSound {
    unsigned int flags;
    IxSample *sample;
#ifdef __cplusplus
    void ix_system_sound_set_volume(unsigned short volume);
    void ix_system_sound_set_pan(unsigned short pan);
    void ix_sound_set_delete_on_stop(int enabled);
#endif
};

/* The DOS rasteriser passes this record to every drawing primitive.  Its
 * offsets are fixed by the accesses in the 0x00440C00-0x00441A8F block. */
typedef struct Viewport {
    unsigned char *pixels;          /* +0x00 */
    unsigned short *rowOffsets;     /* +0x04 */
    short left;                     /* +0x08 */
    short top;                      /* +0x0A */
    short right;                    /* +0x0C */
    short bottom;                   /* +0x0E */
    unsigned char *allocation;      /* +0x10 */
} Viewport;

#ifdef SDL_PORT
/* The original rasterisers form this pointer before applying their vertical
 * clip. Avoid an otherwise unused out-of-range table read under sanitizers. */
#define SPRITE_ROW_OFFSET(viewport, row) \
    ((row) < (viewport)->top || (row) > (viewport)->bottom \
         ? 0 \
         : (viewport)->rowOffsets[row])
#else
#define SPRITE_ROW_OFFSET(viewport, row) viewport->rowOffsets[row]
#endif

/* The event manager snapshots these 28 bytes with seven MOVSD operations.
 * The unaligned pointer fields and reserved spans are fixed by the original
 * addresses at 0x0059AB10-0x0059AB2B. */
#pragma pack(push, 1)
typedef struct MouseCursorState {
    volatile short x;                    /* +0x00 */
    volatile short y;                    /* +0x02 */
    unsigned char primaryButton;         /* +0x04 */
    unsigned char secondaryButton;       /* +0x05 */
    unsigned char reserved;              /* +0x06 */
    unsigned short flags;                /* +0x07 */
    unsigned char * volatile shape;      /* +0x09 */
    unsigned short frame;                /* +0x0D */
    unsigned int reservedAfterFrame;     /* +0x0F */
    Viewport * volatile viewport;        /* +0x13 */
    unsigned int reservedAfterViewport;  /* +0x17 */
    unsigned char shapeChanged;          /* +0x1B */
} MouseCursorState;

typedef struct HostMouseMessage {
    int x;
    int y;
    int primaryButton;
    int secondaryButton;
} HostMouseMessage;

#pragma pack(pop)

#ifndef SDL_PORT
typedef char MouseCursorState_size_must_be_0x1c[
    sizeof(MouseCursorState) == 0x1c ? 1 : -1];
#endif

/* The event manager keeps a fixed pool of doubly-linked input records. */
typedef struct InputEvent {
    short type;                       /* +0x00 */
    short x;                          /* +0x02 */
    short y;                          /* +0x04 */
    short value;                      /* +0x06 */
    unsigned int modifiers;           /* +0x08 */
    unsigned int timestamp;           /* +0x0C */
    short primaryButton;              /* +0x10 */
    short secondaryButton;            /* +0x12 */
    unsigned int field_14;             /* +0x14 */
    unsigned int field_18;             /* +0x18 */
    unsigned int status;               /* +0x1C */
    struct InputEvent *next;           /* +0x20 */
    struct InputEvent *previous;       /* +0x24 */
} InputEvent;

/* WC2 widens the event type and aligns the public event record.  value and
 * status are also written together as a 32-bit value at several call sites. */
typedef struct InputEventState {
    int type;                         /* +0x00 */
    unsigned short value;             /* +0x04 */
    short status;                     /* +0x06 */
    unsigned int timestamp;           /* +0x08 */
    unsigned short modifiers;         /* +0x0C */
    short x;                          /* +0x0E */
    short y;                          /* +0x10 */
} InputEventState;

/* One sampled joystick position and its button mask. */
typedef struct InputDeviceSample {
    int x;
    int y;
    unsigned int buttons;
} InputDeviceSample;

/* Linear 8-bit raster target and its active clipping rectangle. */
typedef struct RasterSurface {
    unsigned char *pixels;
    int maximumX;
    int maximumY;
    int field_C;
    int field_10;
} RasterSurface;

typedef struct RasterClip {
    RasterSurface *surface;
    int left;
    int top;
    int right;
    int bottom;
} RasterClip;

/* One transformed corner used by the hand-written RLE scan converter.  The
 * third word is deliberately unused; the retail workspace advances through
 * these records with a 20-byte stride. */
typedef struct RLETransformVertex {
    int destinationX;
    int destinationY;
    int reserved;
    int sourceX;
    int sourceY;
} RLETransformVertex;

/* Cached shape frames use the renderer's row-oriented RLE representation.
 * The four signed bounds are relative to the shape origin; the byte stream
 * for the first row begins immediately after this 24-byte header. */
typedef struct RLEFrameHeader {
    short height;
    short width;
    short topExtent;
    short leftExtent;
    int left;
    int top;
    int right;
    int bottom;
}
#ifdef SDL_PORT
__attribute__((packed))
#endif
RLEFrameHeader;

/* One scratch glyph bitmap used while drawing the packed game fonts.  The
 * renderer stores a pointer-to-pointer to this record in TextContext because
 * the DOS implementation could move the backing allocation. */
typedef struct FontWorkspace {
    int width;
    int height;
    unsigned char *pixels;
} FontWorkspace;

/* Packed text renderer state.  The Win32 port retained the DOS byte layout:
 * the draw colour is at +0x0C, the optional text pointer at +0x0E and the
 * horizontal alignment byte at +0x16. */
#pragma pack(push, 1)
typedef struct TextContext {
    Viewport *viewport;              /* +0x00 */
    short cursorX;                   /* +0x04 */
    short cursorY;                   /* +0x06 */
    unsigned char *font;             /* +0x08 */
    unsigned char colour;            /* +0x0C */
    unsigned char backgroundColour;  /* +0x0D */
    char *text;                      /* +0x0E */
    char *textCursor;                /* +0x12 */
    signed char alignment;           /* +0x16 */
    FontWorkspace **fontWorkspace;   /* +0x17 */
} TextContext;

/* One of six numeric cockpit readouts.  The packed ten-byte stride is used
 * directly by the original renderer at 0x00438454-0x004385B3. */
typedef struct CockpitReadout {
    TextContext *context;           /* +0x00 */
    short x;                        /* +0x04 */
    short y;                        /* +0x06 */
    short previousRight;            /* +0x08 */
} CockpitReadout;

/* Saved background and text state for the centred modal message panel.  The
 * unaligned Viewports are inherited from the packed DOS layout. */
typedef struct ModalTextPanel {
    TextContext context;              /* +0x00 */
    Viewport savedBackground;         /* +0x1B */
    Viewport viewport;                /* +0x2F */
    TextContext *previousContext;     /* +0x43 */
    short left;                       /* +0x47 */
    short top;                        /* +0x49 */
    short right;                      /* +0x4B */
    short bottom;                     /* +0x4D */
} ModalTextPanel;

/* One flashing cockpit/VDU message.  Two adjacent records begin at
 * 0x005D1D40; the unaligned text pointer at +0x0D is intentional. */
typedef struct HudMessageSlot {
    TextContext *context;            /* +0x00 */
    short x;                         /* +0x04 */
    short y;                         /* +0x06 */
    short colour;                    /* +0x08 */
    short drawColour;                /* +0x0A */
    signed char flashCount;          /* +0x0C */
    const char *text;                /* +0x0D */
} HudMessageSlot;
#pragma pack(pop)

#ifndef SDL_PORT
typedef char CockpitReadout_size_must_be_0x0a[
    sizeof(CockpitReadout) == 0x0a ? 1 : -1];
#endif

/* Runtime wave-cache node.  The name pointer and next link are established by
 * the allocation/free paths at 0x0042B1F0 and 0x0042B300; the sample pointer
 * at +0x08 is established by playWAVE. */
typedef struct WaveTableEntry {
    char *name;
    int field_4;
    IxSample *sample;
    int field_c;
    struct WaveTableEntry *next;
} WaveTableEntry;

/* One transient IX sound and its link in the game-side active-sound list. */
typedef struct ActiveSoundEntry {
    IxSound *sound;
    struct ActiveSoundEntry *next;
} ActiveSoundEntry;

/* Metadata for allocations surrounded by the 0x400-byte 0xAB guard regions
 * checked by ReportHeapGuardCorruption. */
typedef struct GuardedAllocation {
    void *block;
    unsigned int size;
    struct GuardedAllocation *next;
} GuardedAllocation;

/* One pointer retained by the debug heap after the underlying block is freed. */
typedef struct FreedHeapBlock {
    void *block;
    struct FreedHeapBlock *next;
} FreedHeapBlock;

/* Win32 developer console.  The constructor/destructor and member-call ABI in
 * the shipped image identify this small utility as C++ even though the game
 * itself remains C.  The 0x400-byte formatting area fixes the complete 0x45C
 * object layout used by the 0x0041C760 debug-overlay unit. */
typedef struct DebugOverlayConsole {
    int columns;                         /* +0x000 */
    int rows;                            /* +0x004 */
    int cursorColumn;                    /* +0x008 */
    int cursorRow;                       /* +0x00C */
    HWND window;                         /* +0x010 */
    char formatBuffer[0x400];            /* +0x014 */
    char *textBuffer;                    /* +0x414 */
    char *dirtyLines;                    /* +0x418 */
    int characterWidth;                  /* +0x41C */
    int characterHeight;                 /* +0x420 */
    int busyWait;                        /* +0x424 */
    int reverseVideo;                    /* +0x428 */
    COLORREF textColor;                  /* +0x42C */
    COLORREF backgroundColor;            /* +0x430 */
    int backgroundMode;                  /* +0x434 */
    int field_438;                       /* +0x438 */
    int field_43c;                       /* +0x43C */
    int field_440;                       /* +0x440 */
    int field_444;                       /* +0x444 */
    int field_448;                       /* +0x448 */
    HANDLE mutex;                        /* +0x44C */
    int animationState;                  /* +0x450 */
    int spinnerIndex;                    /* +0x454 */
    char *spinnerCharacters;             /* +0x458 */
#ifdef __cplusplus
    DebugOverlayConsole(HINSTANCE module, HWND targetWindow,
                        int columnCount, int rowCount, int waitMode);
    ~DebugOverlayConsole(void);
    void Clear(void);
    void Scroll(void);
    void DrawPendingLines(void);
    char WaitForKey(void);
    void EnableReverseVideo(void);
    void DisableReverseVideo(void);
    void SetOverlayTextColor(int red, int green, int blue);
    void SetOverlayBackgroundColor(int red, int green, int blue);
    void SetTransparentBackground(void);
    void SetTextPosition(int row, int column);
    void SetCursorPosition(int column, int row);
    void Reset(void);
    unsigned char TakeStepFlag(void);
    void ClearPauseFlags(void);
#endif
} DebugOverlayConsole;

/*
 * The DOS source spelled `BOOLEAN window_colored = FALSE;`, but <windows.h>
 * already provides BOOLEAN (as BYTE) and MSVC 4.2 rejects a redefinition with a
 * different base type.  The Win32 port therefore has to be using the windows.h
 * one, so do not redeclare it here.  TRUE/FALSE likewise come from windows.h.
 */
#if defined(ANALYSIS_BUILD) && !defined(SDL_PORT)
typedef unsigned char BOOLEAN;
#ifndef TRUE
#define FALSE 0
#define TRUE  1
#endif
#endif

/* --------------------------------------------------------------------------
 * Enumerations named by the leaked main-module source.  The tag names and the
 * first enumerator of each are original:
 *     GAME_MODE   Game_Mode   = Player;
 *     GAME_STATUS Game_Status = ALIVE;
 * The remaining enumerators are NOT yet recovered -- do not invent values that
 * the assembly has not confirmed.
 * -------------------------------------------------------------------------- */
typedef enum {
    Player = 0
    /* TODO: remaining GAME_MODE enumerators unknown */
} GAME_MODE;

typedef enum {
    ALIVE = 0
    /* TODO: remaining GAME_STATUS enumerators unknown */
} GAME_STATUS;

/* --------------------------------------------------------------------------
 * Shared utility layer.  These were recovered by ranking the call graph by
 * fan-in; the addresses are the originals.
 * -------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* Variadic printers cannot be generated mechanically, so they live here. */
void ShowOnScreenMessage(short duration, const char *fmt, ...);
void SoundDebugPrintf(const char *fmt, ...);   /* 0x437946 */
void SystemDebugPrintf(const char *fmt, ...);  /* 0x40FDAD */

#ifndef ANALYSIS_BUILD
#include "game_data.h"
#include "globals.h"
#include "functions.h"
#include "externs.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* GAME_H */
