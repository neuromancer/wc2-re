# Compilation-unit order

MSVC emits functions in source order, and the linker concatenates object files
in command-line order. Address order is therefore the primary evidence for
source ownership and link order.

`src/map` is the address-sorted inventory of source functions with a mapped
retail destination. Unresolved functions are omitted until their destinations
are established.

The physical source and object order still preserves the predecessor
reconstruction while retail compilation-unit ownership is recovered. Do not
mechanically reorder the files from low-confidence transfer rows: `make sort`
is expected to report out-of-order retail labels during this migration. The
sections below record the historical predecessor boundaries as provenance, not
current placement claims.

## Proven retail boundaries

| Reconstructed source | Range | Evidence |
| --- | --- | --- |
| `src/dibtime.cpp` | `0x0045CCB0`-`0x0045CD2B` | Two scalar dynamic initializer pairs referenced by consecutive `CRT$XCU` entries; immediately precedes `ThrottleFrameAndDrawFps` |

The timing unit is linked between `src/music.c` and `src/screen.c`, matching
the target sequence from `AllocateViewport` through the initializers to the
frame throttle.

## Image layout and link line

```text
0x00401000  game core (command-line objects)
0x00469C80  ix sound library: system.cpp, sound.cpp, sample.cpp
0x0046BD50  ix streamer library: streamer.cpp, thread.cpp (stream files last)
0x0046DF08  import thunks of the first search pass (DDRAW ... ADVAPI32)
0x0046E0E0  LIBCMTD members the game needs, in first-reference order
0x00473E50  wincrt0.obj, then the CRT members only the CRT and ix need
0x00489990  ix dsp library: dsp, dspv, dsps, mixer, dspmix (assembled)
0x0048C290  ix LZO library: lzo1x.cpp
0x0048C400  ix file library: ixfile.cpp
0x0048C58A  import thunks of the second search pass (DSOUND, KERNEL32)
0x0048C760  CRT members reached through OLDNAMES (getch, strupr, flength),
            then those the file module needs (fseek, ftell, fread, fwrite)
```

The final source-written function in `thread.cpp` is
`IxStreamFile::ix_stream_file_is_reading` at `0x0046DED9`; its return is at
`0x0046DF06`, immediately before the first-pass import thunks.

The source-file order above does not by itself prove separate compilation
units. Retail has no padding between `system.cpp` and `sound.cpp`, between
`sound.cpp` and `sample.cpp`, or between `streamer.cpp` and `thread.cpp`.
Our individual CL 4.1 objects request 16-byte `.text` alignment, adding
padding at those boundaries. Isolated `/G3`, `/Gy`, and `/Zp1` builds all
retain that alignment; `/Gy` additionally splits every function into its
own 16-byte-aligned COMDAT and is inconsistent with the observed layout.

Compiling the three sound sources together and the two streamer sources
together is a candidate for recovering those boundaries. The initial
include-based probe compiles and links, but only 65 of the 75 functions
retain exact normalized instruction encodings, versus 74 in the separate
build. Changed declaration context and compiler pragmas must be understood
before adopting it. The production build remains separate; do not infer
original object boundaries solely from diagnostic `__FILE__` paths.

LINK 3.10 pulls each library member when its symbol is first needed: by each
command-line object in turn, then the entry point, then each pulled member,
and it searches the libraries again on later passes.  The layout above
therefore records the link line, which the Makefile reproduces:

| Evidence in the original | Link-line choice |
| --- | --- |
| MISC and CodeView debug records (`Debug/wc2.exe`, `C:\WCB\wc2\Debug\wc2.pdb`), 126 FPO records, a thunk for every import | `/DEBUG /INCREMENTAL:NO`, which keeps unreferenced thunks |
| `wincrt0.obj` follows the CRT members the game references | No `/ENTRY` |
| ix sound and streamer members precede the first-pass thunks; `ldexp`, needed by `sample.cpp`, follows `wincrt0.obj` | Those two ix libraries precede the import libraries |
| The dsp, LZO and file members follow the whole first-pass CRT | Their libraries follow `libcmtd.lib oldnames.lib` |
| Import descriptors run DDRAW, WINMM, KERNEL32, USER32, GDI32, ADVAPI32, DSOUND | Only dsp members need DirectSound, so its imports come from the second pass |
| `ix_dspv_mix` starts at `0x0048BFF8`, directly after `mixer.cpp` | `dspmix.asm` is a four-byte-aligned member of its own, as in WC1 |

The CRT's first-pass order also records which names the game called.  The
DOS-era names (`itoa`, `ltoa`, `ultoa`, `strupr`, `filelength`, and `unlink`,
`open`, `write`, `close` and `lseek` in specific units) resolve through
OLDNAMES, so their members are pulled later than a direct reference would put
them.  `smart.c` and `fl.c` call `labs`, a separate member from the `abs` the
rest of the game calls.

`dspv.cpp` and `dsps.cpp` each begin with an empty `$E2`/`$E1` initializer
pair in the original (`0x0048A210`, `0x0048A9F0`), registered in `CRT$XCU`.
VC++ emits them for these global record arrays when their types declare a
copy-assignment operator. `IxVoice` and `IxStream` now have those declarations
in `ix.h`; no assignment implementation or handwritten initializer is needed.
The compiler listings contain both `$E2`/`$E1` pairs and their `CRT$XCU`
references.

## Historical predecessor game-core working split

The core is grouped into short subsystem-named files following address order.
The windows below are placement guides, not claims that every edge is an
original object boundary. A nested source unit can be proven even when its
containing file boundary remains provisional.

| File | Working address window | Main evidence |
| --- | --- | --- |
| `src/winmain.c` | `0x401000`–`0x402DFF` | Win32 entry, window, message loop, shutdown |
| `src/sysinput.c` | `0x402E00`–`0x4030FF` | Win32 input and clock wrappers |
| `src/cdrom.c` | `0x403100`–`0x4034FF` | CD location and disc prompts |
| `src/mono.c` | `0x403500`–`0x403E4F` | MONODEBG and adjacent host routines |
| `src/auto.c` | `0x403E50`–`0x40460F` | Exact Mac `auto` symbol run |
| `src/cmpgn.c` | `0x404610`–`0x40609F` | Campaign/briefing Mac symbols |
| `src/brains.c` | `0x4060A0`–`0x40CFFF` | Mac `fl`/`brain` symbols and recovered `BRAINS.C` |
| `src/nav.c` | `0x40D000`–`0x40FFFF` | Nav drawing and location strings |
| `src/spc.c` | `0x410000`–`0x412FFF` | Space simulation and Mac `spc` symbols |
| `src/cockpt.c` | `0x413000`–`0x417FFF` | Mac `cockpt` symbols |
| `src/geom.c` | `0x418000`–`0x41AD4F` | Geometry and modal panel sequence |
| `src/barracks.c` | `0x41ADA0`–`0x41C75F` | Save/load and barracks sequence |
| `src/debug.cpp` | `0x41C760`–`0x41D0BF` | Proven C++ debug-console object |
| `src/mathutil.c` | `0x41D000`–`0x41D24F` | Integer min/max cluster |
| `src/disk.c` | `0x41D250`–`0x41EFFF` | Disk files and packet retry paths |
| `src/personnel.c` | Retail `0x428C35`, `0x433AD0`–`0x436A8F`, `0x459BC8` | Personnel database and previous-game pilot transfer flow |
| `src/ship.c` | `0x41F000`–`0x420FFF` | Ship damage, explosions, and weapons |
| `src/logic.c` | `0x421000`–`0x424FFF` | Mac `logic` unit and adjacent mission logic |
| `src/pilot.cpp` | `0x425000`–`0x426FFF` | Pilot/TrainSim flow and C++ console owner |
| `src/system.c` | `0x427000`–`0x4274DF` | Process-level reporting and exit |
| `src/main.c` | `0x4274E0`–`0x427FFF` | Recovered WINGLEADER main module |
| `src/hudmsg.c` | `0x428000`–`0x42AFFF` | HUD messages plus Mac `targ` and `select` units |
| `src/pload.c` | `0x42B000`–`0x42B3FF` | Proven `Library\Source\Pload.c` path |
| `src/sound.c` | `0x42B400`–`0x42CFFF` | Wave playback, settings, and `INSTALL.DAT` |
| `src/music.c` | `0x42D000`–`0x42EFFF` | Self-naming music diagnostics |
| `src/screen.c` | `0x42F000`–`0x431FFF` | Screen scopes, prompts, and comm menus |
| `src/dib.c` | `0x432000`–`0x43390F` | Proven DirectDraw `DIB*` unit |
| `src/text.c` | `0x433690`–`0x433ABF` | The retail build adds its cinematic sprite-font renderer before the exact Mac `show_info_disp` symbol |
| `src/smart.c` | `0x433AC0`–`0x434CCF` | Proven Mac `smart` symbol run |
| `src/mathfp.c` | `0x434CD0`–`0x4353FF` | Random and floating-point helpers |
| `src/strdos.c` | `0x435400`–`0x4355EF` | DOS-width string/memory shims and setup calls |
| `src/eventmgr.c` | `0x4355F0`–`0x436FFF` | Event queue, pointer, keyboard, and timers |
| `src/screens.c` | `0x437000`–`0x43F5FF` | Full-screen scenes and raster support |
| `src/killbrd.c` | `0x43F600`–`0x440BFF` | Kill board, conversations, save flags |
| `src/gr.c` | `0x440C00`–`0x44269F` | Raster primitives, effects, and game audio wrappers |

`src/globals.c` is temporary storage for globals whose original owner or
declaration order is not yet proven. It is not an original compilation unit.

## Strong boundary anchors

- `src/pload.c` identifies itself through the string
  `Library\Source\Pload.c PacketLoad`.
- The retail build adds `ShutdownNearHeap` at `0x00420B12` directly between
  `InitializeNearHeap` and `AllocateNearHeapBlockFromEnd`; its seven globals
  are the same private band used by the surrounding near-heap routines in
  `src/nav.c`.
- `LoadSpaceflightResources` is at `0x0045C35C`, between the direction-view
  setup and music initialization in `src/logic.c`. Its three `LoadShapeSet`
  calls, chained debris-shape assignments, and `LoadOriginFxDrivers` caller
  disprove the earlier low-confidence `0x00420B12` transfer.
- `src/dib.c` and `src/music.c` identify their routines in diagnostics.
- The debug overlay's constructor, destructor, ECX methods, and allocation
  sites prove `src/debug.cpp` as a C++ unit.
- Macintosh symbols establish the nested `auto`, `fl`, `brain`, `logic`,
  `targ`, `select`, and `smart` runs after their Win32 bodies are checked.
- With string pooling disabled, each object's string literals form a local
  band. Code/string address pairs are useful boundary evidence where enough
  literals survive.

## Global ownership

Globals referenced by several units are declared in `include/globals.h`.
Compilation-unit-private declarations stay in their owner. Definitions move
out of `src/globals.c` only when both owner and declaration order are supported
by the original image.

`bin/collectGlobalDefinitions.py` produces `out/globals-audit.c` so verification
can inspect distributed definitions as one manifest. Use `make globals-data`
before and after moving a global band.

The public event-manager state occupies `0x005C80A8`-`0x005C8D3D`, after
main's event pool and before the turret block. Its definitions, including the
two pause-input arrays used by main, belong to `eventmgr.c`. Moving this band
out of the provisional `globals.c` lets all 21 main commons land at their
retail addresses. The event-manager block still needs its internal symbol
order and packed records recovered; declaration order alone does not order
LINK's commons.

The drawn-cursor Y/X pair now occupies retail `0x005C80A8/0x005C80AC`.
Four global renames place `g_nMouseRenderY_005c80a8` and
`g_nDrawnCursorX_005c80ac` first in this unit's common-symbol order, in hash
buckets 4 and 36. `g_cScreenViewMode_005c82a6` and
`g_wPendingButtonMask_005c80d4` move to buckets 189 and 169 so they no longer
precede that pair. All four preserve their C1 symbol keys; definitions,
types, and function-body expressions are unchanged. Nine global addresses
move, all within this band, and the allocation's total extent is unchanged.
Only the leading coordinate pair is now at retail addresses.

The current 53 common allocations cover every direct assembly address and
every relocated pointer target found in the original within
`0x005C80A8`-`0x005C8D3D`. The remaining gaps, including
`0x005C80B0`-`0x005C80BF`, have no such references. This does not prove that
the gaps are unused or establish their declarations: calculated accesses
and unreferenced storage remain possible. Do not fill them with guessed
globals merely to reach the next known address.

The observed array extents do not account for those gaps. `ClearInputKeyState`
(`0x004642D6`) clears `0x80` bytes; `AllocateInputEvent` (`0x00462890`)
clears `0x400` slot-use bytes and scans `0x100` entries.
`BuildObjectDepthOrder` (`0x0046436E`) clears `0x118` bytes and limits both
object-order arrays to `0x46` entries. The pause-state loop in
`HandleSpaceFlightControls` stops at `0x100`, matching both 256-byte arrays.
These accesses support the current declarations, not larger arrays inferred
from their neighbours' addresses.

The current map localizes the 720-byte uninitialized-data deficit to this
event-manager band: annotated globals before it have retail addresses;
after its two leading turret words and their alignment, subsequent bands
remain uniformly `0x2D0` bytes early. This is not evidence for adding
720 bytes of storage without declarations. The apparent movie-text/cache
overlap reported later in `gameflow.c` is unrelated: the FLIC lookup at
`0x0042E868` increments its index before using displacement `0x005D2E08`.
Indices 1 through 15 access cache entries at `0x005D2E10` through
`0x005D2E80`, leaving the preceding 27-byte text context untouched.

`InflightComputer` saves and restores 33 bytes at `0x005C8470` (eight `MOVSD`s
and one `MOVSB`). This is the personnel cursor record: x/y are at `+0/+2`,
the shape pointer at `+0x0D`, frame at `+0x11`, scratch pointer at `+0x13`, and
input context at `+0x17`. Those fields share the record's storage. The older
28-byte snapshot at `0x0059AB10` was inherited from WC1 and is not its WC2
destination.

The queued mouse record at `0x005C83F0` is 28 packed bytes. The event and
cursor routines access x/y at `+0/+2`, button bytes at `+4/+5`, flags at
`+7`, shape at `+9`, frame at `+0x0D`, viewport at `+0x13`, and the shape
change flag at `+0x1B`. In particular, the unaligned word and pointer
addresses cannot be reproduced by separately allocated commons. These nine
globals now share `QueuedMouseState` storage; the unreferenced gaps retain
offset-based field names. This fixes their relative layout, while placement
of the record within the event-manager band remains unresolved.

The personnel mouse sample at `0x005C8D00` has the same first nine bytes.
`PollMenuInputDevices` reads its flags as a word at `+7`, which cannot be
represented by an independently aligned short global. `PersonnelMouseSample`
now groups the two coordinates, two button bytes, intervening byte, and flags
into that packed prefix. No observed access proves its full extent; do not
infer another 19 bytes solely from the next known global at `0x005C8D1C`.

Within an object's commons, MSVC's identifier hash determines emission
order: compute `h = (h >> 4) + 4*h + character` with 32-bit wrapping, then
`(h ^ (h >> 16)) & 0x3FF`. The event-manager map follows ascending hash
buckets. This differs from the sequential C1 symbol keys that affect
instruction selection. Recovering spelling and allocation sizes must
therefore preserve both constraints; merely sorting declarations is
insufficient. Unreferenced gaps in this band still need evidence before
their storage can be reconstructed.

The remaining 78 predecessor commons formerly in `globals.c` had no
relocation references from any reference-build object, including data
tables; because they were uninitialized leaves, reachability was already at
a fixpoint. Seventy-seven definitions were removed and the eye-space cache
was retained under `SDL_PORT`. Reference builds now allocate no storage in
`globals.c`. Shared-header declarations remain where needed to preserve
MSVC's existing symbol keys during further recovery.

## Recovering another boundary

1. Run `make order` and inspect adjacent entries in `src/map`.
2. Compare callers, private globals, strings, and cross-release symbol order.
3. Do not promote a boundary based on subsystem similarity alone.
4. Update this file and `SRCS_ORDERED_CORE` together once the evidence is
   sufficient.

A wrong object boundary shifts every later rebuilt address, so uncertain edges
remain explicitly provisional.
