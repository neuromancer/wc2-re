# Matching patterns

The original disassembly and extracted strings are authoritative. Decompiled
code, Ghidra signatures, operational labels, and source from another release
are hints that must be checked against the instructions.

Use `binary-comp` for every similarity decision. Do not substitute a separate
scorer.

## Compilation profiles

Both the game core and the `ix` library are compiled `/Od /Oi` (see
[COMPILER.md](COMPILER.md)). Every function opens with
`PUSH EBP / MOV EBP,ESP / PUSH EBX / PUSH ESI / PUSH EDI`, keeps locals and
expression temporaries on the frame, and leaves through one shared
`POP EDI / POP ESI / POP EBX / LEAVE / RET` epilogue. Because nothing is
optimized away, the statement shape of the source is visible in the code:
every `return`, `break`, `else` and dead statement emits instructions.

## Types and calling conventions

The core descends from 16-bit DOS C. A large amount of state and many
intermediate values are `short`, even though the Win32 compiler's `int` is 32
bits. A 32-bit operation where the original uses an operand-size prefix usually
means the declaration is too wide.

Check each function's terminating instruction and every call site:

- bare `RET` normally indicates `__cdecl`;
- `RET n` indicates callee cleanup such as `__stdcall`;
- C++ `ix` methods receive the implicit object in ECX; and
- loads, sign extension, and how the caller consumes EAX determine the return
  width more reliably than the decompiler.

Do not copy a Ghidra prototype without checking the pushes before each call.
Several recovered calls were initially wrong because the decompiler omitted
arguments after learning an incorrect callee signature.

`BOOLEAN` is supplied by the Win32 headers. Do not redeclare it from the DOS
source with a different underlying type.

## Source shape affects generated code

Equivalent C can produce different instruction order. Match the original
control-flow shape rather than merely its result.

### Returns and branches

An early return may produce a full-register move where assigning to a local and
returning once produces a 16-bit move. Likewise, swapping `if` and `else` arms
changes fall-through direction, especially in `/Od` `ix` code. Follow the
original jumps and epilogue placement.

### Locals in `/Od` code

When the original stores an array element address in a stack slot, declare the
corresponding local pointer and reuse it. Re-indexing the array at every access
is behaviorally equivalent but does not reproduce the unoptimized instruction
stream.

CL 4.1 at `/Od` lays out a function's locals by identifier spelling, not by
declaration or first use. Each local hashes to one of 16 buckets,
`key = (h ^ (h >> 16)) & 0xF`, where `h` is C1's identifier hash
(`h = (h >> 4) + 4 * h + c` over the characters, 32-bit). Slots are handed
out from the top of the frame (`[ebp-4]` first) in ascending key order; locals
with the same key take slots in reverse declaration order. The rule predicted
all 600 random two-local probes. `bin/frameOrder.py name ...` prints the keys
and the resulting order, so a stack layout that differs from the original is
fixed by choosing local names, not by reordering declarations.

`BlitWipeRadialBands` at `0x0044AAE5` is a larger confirmed example: renaming
its 21 existing locals reproduces every slot in the `0x54`-byte frame,
without changing declaration order, types, or expressions. The normalized
`binary-comp` report drops from `BYTES 90/263` to `BYTES 4/263`; every other
function's report is unchanged. The four remaining differences are actual
pointer selections, not allocation: retail loads the lower source-row pointer
at `[ebp-0x54]` and lower destination-row pointer at `[ebp-0x24]` in both
backward-indexed copies. The reconstruction instead loads the upper pointers
at `[ebp-8]` and `[ebp-4]`. Correcting those selections changes the mapped
body's logic and remains separate from the name-only layout recovery.

The same rule applies to the mixed-size locals in `DIBslamReal`
(`0x0045DC33`). Its nine locals occupy a `0xA4`-byte frame: the source and
destination pointers are at `-4/-8`, the source rectangle at `-0x18`, tick
at `-0x1C`, destination rectangle at `-0x2C`, result at `-0x30`, row at
`-0x34`, HDC at `-0x38`, and 108-byte surface description at `-0xA4`.
Renaming eight locals reproduces all those slots and removes 60 bytes of
long stack-displacement encodings. The `/Bk` symbol records confirm that
the locals retain keys 34821–34829 and the following function keeps its
keys; no declaration was added, removed, or reordered.

The DIB-only header order exchanges the height and origin-X declarations,
moving the height key from 29862 to 29858. Its load signature becomes
`0x2E` instead of `0x21`, above the row local's `0x2D`. This reproduces the
retail `MOV` from the local, comparison against the global, and `JLE`,
adding one byte back. The function moves from 53 excess bytes to six missing
bytes at 99.28% mnemonic similarity. The remaining six bytes are the two
original stores of `Unlock` results at `0x0045DEC5` and `0x0045DEE4`; they
require assignments in the mapped body, beyond name and header recovery.

Four local renames in `DIBupdate` (`0x0045E060`) put the source pointer,
destination pointer, and row width at retail `-4/-8/-0x0C`. The decoded
instruction-extent comparison improves from 62/141 to 53/141 differing
encodings, with the same 445 code bytes against retail's 534. All 57 parsed
C++ symbol records retain their keys. The frame remains `0x80` bytes against
retail's `0xA4`: two rectangle locals and the row counter are missing. The
height and result remain 32 bytes above their retail slots, and the surface
description remains 36 bytes above its slot. Renaming cannot supply that
absent storage or its setup.

The DIB header order also exchanges row width and origin Y at keys
29861/29859, keeping the height key at 29858. The width load's signature
becomes `0x2F`, above height's `0x2E`. This restores width-before-height in
`DIBmakeDIB` and `DIBdestroyDIB`, and width-before-top in `DIBupdate`, while
preserving `DIBslamReal`'s row comparison. The two 12-byte copy-size products
and the nine-byte row-offset product now match retail literally. The latter
loses an excess byte, taking `DIBupdate` from 53/141 differing encodings to
51/141 and from 445 code bytes to 444, against retail's 534. All local
symbol records remain identical; the 44 parsed defined-global records
change only those two keys. Linked global addresses are unchanged.

Three more name-only corrections recover the local slots in the text-input,
save/load-menu, and window-procedure frames. The counts below come from
`binary-comp`'s `compare_function_bytes` API over decoded instruction extents,
without trailing alignment. They count differing instruction encodings, not
individual bytes; normalized calls and relocations are still excluded.

| Function | Before | After | Remaining code-size delta |
| --- | ---: | ---: | ---: |
| `ReadTextInput` | 115/268 | 47/268 | +28 bytes |
| `RunPilotSaveLoadMenu` | 222/991 | 77/991 | +15 bytes |
| `MainWindowProc` | 220/568 | 146/568 | +30 bytes |

`ReadTextInput` now places all twelve locals at their retail offsets in the
`0x6C` frame, including the 40-byte text buffer at `-0x30` and viewport at
`-0x50`. `RunPilotSaveLoadMenu` puts its slot and campaign-byte count at
`-4/-8`, ahead of the 280-byte file-search record at `-0x120`. This removes
156 bytes of long stack-displacement encodings. Its ten non-command locals
now match retail; the explicit command local and compiler switch temporary
still leave a `0x144` frame where retail uses `0x140`.

All twelve named locals in `MainWindowProc` now match, including the
character at `-4`, message at `-0x3C`, and paint record at `-0x90`.
Its `0x98` frame remains four bytes shorter than retail's `0x9C`: the retail
`WM_COMMAND` arm uses a switch temporary, while the source uses an `if`.
These edits preserve declaration order, types, literals, and expressions;
the `/Bk` records preserve the local symbol keys. The remaining operand,
control-flow, and frame-size differences require separate body corrections.

Thirteen local renames in `RunPilotDatabaseMenu` (`0x00436A8F`) recover
fourteen corresponding slots throughout the function: the caption buffer
at `-0x50`, first name at `-0x60`, hover command at `-0x64`, result at
`-0x6C`, callsign at `-0x7C`, handled flag at `-0x80`, done flag at `-0x84`,
save record at `-0x88`, saved drive at `-0x8C`, slot index at `-0x90`,
transfer count at `-0x94`, surname at `-0xA4`, text-input result at `-0xAC`,
and file handle at `-0xB0`. The instruction-extent comparison drops from
188/788 to 50/788 differing encodings. Correct long stack displacements add
114 code bytes, reducing the code deficit from 133 to 19 bytes. With two
fewer alignment bytes, the mapped-span deficit falls from 127 to 15.

The `0xB4` frame is still eight bytes shorter than retail's `0xBC` frame.
The reconstruction reuses `choice` at `-0xA8` for both source selection
and the later pilot selection, where retail uses a separate `-0x68` slot.
It also reuses `pilotRecord` at `-0xB4`, where retail uses `-0xB8` after
reopening the selected save. The explicit `commandCode` now occupies
`-0x68`; this is not the retail switch temporary at `-0xBC`. These remaining
slot and control-flow differences need body corrections. No variable was
added, removed, retyped, or reordered; all 84 scalar/parameter and five array
keys in the unit's `/Bk` records are preserved.

The disk-save record's `occupied` byte at `+0x11` must be signed: retail
uses `MOVSX` before its zero test. Correcting `SaveGameDiskRecord` in the
header replaces the unsigned load sequence with that instruction and
reduces the differing encodings further to 48/788. It removes one code
byte, leaving a 20-byte code deficit. This record type has only this reader;
the nonzero test has the same result for every byte value. The record's
`0x33C` size and all C1 keys stay unchanged.

`RunCutsceneScript` (`0x0042F1FD`) has a partial name-only recovery:
renaming `inputHandled` to `inputSeen` moves it from bucket 8 to bucket 1.
Reverse declaration order then puts the input flag at `-4` and the primary
operand `value` at `-8`, matching the retail flag and the operand's dominant
slot. The instruction-extent comparison improves from 1,617/3,672 to
1,519/3,672 differing encodings. All 242 parsed scalar, parameter, and array
symbol keys in the unit are preserved. Code remains 437 bytes short, and
the full normalized report is unchanged.

This does not recover the interpreter's full frame. It remains `0xA8`
bytes instead of retail's `0x130`; retail puts the instruction pointer at
`-0x80` and the ten-word stack at `-0x9C`. The exported instructions do not
explain the gap between the leading slots and those objects. Do not fill it
with guessed storage. The source also reuses `value` for some operations
that use separate retail slots, so `-8` is not correct for every use. A
larger probe placing six leading slots at their apparent retail offsets
regressed the same binary-comp comparison to 1,725/3,672 and was discarded.

Three further renames place `protectCondition` and `shouldContinue` at
retail `-0x10/-0x14` while retaining the input flag at `-4` and primary
operand at `-8`. The instruction pointer becomes `scriptBytes`, at `-0x20`;
its retail `-0x80` slot remains unrecovered. This reduces differing encodings
from 1,519/3,672 to 1,518/3,672 and preserves all 242 parsed symbol keys.
Code and frame sizes remain unchanged. Adding the stack-pointer move to this
variant regressed the comparison to 1,780/3,672 and was rejected.

`SetFleetOverviewView` (`0x0044ADE0`) also has the correct `0x3C` frame size
but initially had misplaced locals. Four renames reproduce all nine slots:
`centre` at `-0x0C`, `cameraDistance` at `-0x10`, `shipSlot` at `-0x14`,
`shipCount` at `-0x18`, `playerRange` at `-0x1C`, `viewOffset` at `-0x28`,
`furthestRange` at `-0x2C`, `range` at `-0x30`, and `axes` at `-0x3C`.
The same instruction-extent comparison drops from 80/292 differing
encodings to 66/292. Code size remains 974 bytes versus retail's 1,044;
the mapped span remains 80 bytes short because its padding also differs.
Declaration order, types, expressions, and C1 symbol keys are preserved.
The remaining differences include retail's 16-bit parameter and counters,
loop control flow, and camera-distance selection. Matching slot offsets
does not establish matching access widths or behavior.

`InflightComputer` (`0x00451FAA`) puts the cockpit shape at `-4`, objective
at `-8`, input record at `-0x1C`, viewport at `-0x30`, done flag at `-0x34`,
saved nav point at `-0x38`, frame at `-0x3C`, cursor at `-0x60`, selected
nav point at `-0x64`, objective flag at `-0x68`, and marker colour at
`-0x6C`. Eight renames recover these offsets, reducing differing instruction
encodings from 62/307 to 36/307. The explicit input-type local still adds
four bytes before the compiler's switch temporary: the frame remains
`0x74` instead of retail's `0x70`. Retail reads the input record at `+6`,
where this function still reads `value` at `+4`; moving the record cannot
correct that field selection. Code remains 22 bytes longer than retail,
with the mapped span reporting +16 because of its alignment boundary.

Nine renames in `player_input` (`0x00466B02`) recover all thirteen named
slots in the original `0x48` frame. The order is X magnitude, Y displacement,
Y magnitude, polled-input flag, buttons, result, event type, 20-byte input
record, mouse pointer, threshold, boost control, saved input state, and X
displacement; the switch temporary is last. Differing encodings decrease
from 163/431 to 82/431. The `/Bk` records preserve all parsed local and
parameter keys in both units, including the functions after `player_input`.

Retail calls the CRT's `abs` at `0x0046F3D0` from `0x00466DCB` and
`0x00466DDC`; `/Oi` was instead inlining those two calls. The unit-scoped
`#pragma function(abs)` before `player_input` restores the calls without
changing its body or C1 symbol keys. Its differing encodings fall further
to 76/431, and mnemonic similarity rises from 93.97% to 95.36%. The eight
added code bytes leave the function nine bytes longer than retail. The
remaining differences include mouse-button branch ordering and the
equivalent but differently encoded clamping comparisons against +/-8
instead of +/-9.

In `HandleSpaceFlightControls` (`0x0046733D`), renaming `inputHandled` to
`inputConsumed` and `control` to `controlHeld` puts the handled-input flag
at retail `-4`, exchanging its slot with the control-key flag. This reduces
differing encodings from 391/1,503 to 388/1,503, with all 68 parsed C1
symbol records retaining their keys. Code remains 14 bytes longer than
retail, and the frame remains `0x60` against `0x84`. The other named locals
still occupy `-8` through `-0x18` rather than retail's `-0x64` through
`-0x74`; names cannot supply the unexplained intervening storage.

Locals declared in a nested block get their slots after every function-scope
local, below them in the frame, whatever their keys; hoist such a local to the
top of the function when the original slot order needs it
(`RetainInputEventsOfType`, `ReleaseInputEventQueue`).

`/Od` gives every declared local a slot even when nothing reads it, so a frame
larger than the slots the code touches is evidence of a dead local in the
original. Several Win32-port routines keep one, sometimes with a lone
initializing store (`PollCampaignChalkboardMenu`'s `done = 0`,
`WaitForInputKey`).

Operand order of commutative operations and comparisons follows the
signatures of C1 symbol keys (see `../wc1-re/docs/COMPILER.md`). Every
identifier, named prototype parameter, label and string literal takes a key,
so adding or removing a local, a `for` loop, an `else`, or a literal shifts the
order decided in every later function of the unit. A struct type declared in a
header shifts every unit; declare a type used by one global next to that
global. C++ units (`/TP`, e.g. `sound.c`) also count unnamed prototype
parameters; C units do not. When a correct structural fix in one function
breaks the operand order of a later one, look first for a neighbouring
function whose local or label count is still wrong; a sweep
(`extern` placeholders inserted at one line) shows the shift each later
function needs.

Shifts are realized with ordinary source, as in WC1 (`docs/WORKAROUNDS.md`
there): a standard `#include` after `game.h`/`ix.h` moves a whole unit
(`usedcols.c` takes `<malloc.h>`, `system.cpp` `<memory.h>`, `dspv.cpp`
`<time.h>`), and a local prototype of a callee costs one key per named
parameter, so a block of them before a function moves that function and
every later one (`logic.c`, `smart.c`, `ship.c`, `nav.c`, `brains.c`,
...). Only the parameters count: the function name is already a symbol
from `functions.h`. A function name's own key still matters when its call is
an operand of a commutative expression (`dot_product`'s three
`MultiplyFixed` calls), so header edits must keep every name's position.

A call that the original made without a prototype in scope shows up as
promoted arguments: every retail caller of `DrawTextAt`, `maintain_formation`
and `intelligence_events` sign-extends its `short` arguments. Declaring the
parameter `int` in `functions.h` (definition keeps `short`; MSVC only warns
C4028) reproduces the push without moving any key; the SDL build keeps the
`short` prototype under `#ifdef SDL_PORT`. A callee with C++ linkage cannot
take this, since the parameter type is part of its decorated name.

A function's return type as its callers see it shows in the extension after
the call: `movsx eax,ax` means `short`, `movzx eax,ax` `unsigned short`
(`GetFontCharWidth`, `ReadWord` return `short`).

A call with no argument pushed at all (`CALL PumpWindowMessages` with no
`PUSH`, no `ADD ESP`) is an unprototyped call made with too few arguments.
Reproduce it through a cast to a no-argument function type and keep the
SDL build on the normal call (`RunTitleScreen`, `FlushPendingInputPresses`).
A caller that pushes a 16-bit value without extending it (`MOV AX,[g] /
PUSH EAX`) saw a 16-bit parameter, even when the definition reads a dword
(`DosMemcpy`'s prototype is `unsigned short`, its definition masks
`size_t`).

In C++ units a renamed local can move later functions' keys. A name already
seen in the unit (`description` is a parameter of `ix_dsp_open_driver`)
appears to add no new key, so re-check the following function after any
rename in `src/ix` (`ix_mixer_thread_proc` and `ix_mixer_service`).

A global that one unit reads as a dword and another as a word (`XOR EAX,EAX /
MOV AX,WORD PTR [g]`) is a 4-byte variable. Declare it with the full width.
The word readers cast it, since `(unsigned short)g` narrows the load itself
(`g_nDigiSampleBytes_0048e0e0`: `comm.c` compares the dword, `speech.c` casts).
The same holds for word and byte: `fltsetup.c` stores and tests
`g_bTargetLockDisplayEnabled_0049afe0` as a word while `cockpt.c` reads
`(signed char)` of it. Where only one site in a unit reads a short global as
a dword (`RunGameApplication` passes `g_nMemoryAdjustmentKb_005c8dda` to
`printf` with `MOV EAX,[g]`), the read is `*(int *)&g` under `#ifndef SDL_PORT`.

In `ix.h` the effect of a declaration change is not a uniform key shift.
Replacing the free `__fastcall` `ix_sound_unlink_from_free_list` with an
`IxSound` member needed two of the system globals' `extern`s moved to where
the free declaration was before `dspv.cpp` and `dsps.cpp` matched again.
After any `ix.h` edit, re-run the whole report rather than one unit.

C++ declarations with `()` and `(void)` denote the same no-argument signature,
but CL 4.1's symbol bookkeeping differs. Restoring
`IxStreamFile::ix_stream_file_is_reading` at `0x0046DED9` with `()` and changing
the adjacent `ix_stream_file_wait` declaration to `()` preserves the existing
ix operand orders; adding `(void)` declarations shifts them. The restored
method's 46 instruction bytes match the retail image literally.

The file-scope prototype immediately before
`IxSample::ix_sample_convert_from_ieee_extended` also determines its byte
assembly order. Replacing the redundant three-parameter `ix_dsp_copy`
declaration with the two-parameter `ldexp` declaration moves the converter's
seven parameter/local keys down by one; `src` moves from 26710 to 26709.
`ldexp` retains the signature and C linkage already provided by `<math.h>`.
All stack offsets remain unchanged, and the preceding AIFF, WAV, and raw
loaders emit identical instructions.

This restores the high mantissa's byte order to 2, 3, 4, 5 and the low
mantissa's order to 8, 7, 6, 9, as retail emits them. The 100-byte pair of
assembly sequences now matches literally, versus 90/100 before. Across the
converter's 116 decoded instructions, normalized differing encodings fall
from ten to zero. Its 378 instruction bytes already had the correct size;
the mapped span remains one byte shorter because it includes the alignment
gap before the streamer library. All 4,512 linker-map addresses remain
unchanged. Removing the redundant prototype without replacing it shifts
the keys by three and leaves eight differing encodings.

Do not apply a whole-function key shift to the AIFF loader merely to fix
its two header-word swaps. Moving the unused WAV record declaration below
the loader fixes those two sequences but reverses a previously correct
dword swap, increasing differing encodings from 29/351 to 30/351. For the
current C2 expression trees, no common key shift satisfies all five
observed word/dword operand orders. The sample-data byte-swap loop also
has a different expression shape. The declaration-move probe is rejected.

`ix_thread_handle_file_chunk` needs both local-name recovery and one
thread-specific declaration exchange. Eleven local renames place all
thirteen declared locals at retail offsets `-4` through `-0x34`, in
declaration order; the compiler temporary remains at `-0x38`. Types,
declaration order, expressions, member names, and string literals are
unchanged. All 44 local/parameter records in the unit retain their C1 keys.
Renames alone reduce differing instruction encodings from 69/307 to 25/307.

Under `IX_THREAD_CPP`, exchanging the reader-directory and disk-chunk
declarations at keys 26348/26359 lowers the disk-chunk load signature from
`0x2A` to `0x20`, below `chunkIndex`'s `0x25`. At retail `0x0046D52A`, C2
then loads the global and compares the local, reproducing the shorter
absolute-load encoding. This removes the extra code byte and makes all
307 decoded instructions normalized-byte exact, at 1,056 bytes. The ten
following thread methods move one byte earlier; the library's end alignment
absorbs that byte, leaving subsequent objects and all section sizes unchanged.

The audit of 100%-mnemonic functions with size differences also finds four
reversed list comparisons in `IxSound::ix_sound_reprioritise`. Merely moving
the waiting-list declaration fixes two and removes two code bytes, but
leaves the active-list comparisons wrong. Their initial load signatures
were `this = 0x22`, active list `= 0x23`, and `playingSound = 0x26`; retail
requires the active-list load before `this` and after `playingSound`.
A global-declaration exchange alone cannot satisfy both; the partial
waiting-list change was rejected.

The complete correction is scoped to `IX_SOUND_CPP`. Seven no-argument
streamer declarations use the equivalent C++ spelling `()` instead of
`(void)`, moving all 19 sound-unit parameter/local keys down seven without
changing any stack offset. `this` becomes key 26429 (load signature `0x2B`)
and `playingSound` becomes 26434 (`0x20`), straddling the unchanged active
list's `0x23`. The waiting-list/system-bits declarations exchange keys
26170/25988, giving the waiting list signature `0x20`, below both `this`
and `waiting` (`0x2E`). This restores all four comparisons.

The same local-key shift would reverse the sample destructor's list
comparison. Exchanging the sample-list/free-sound-head declarations at
keys 26169/25989 preserves that comparison and the start routine's existing
free-list comparisons. Only the priority routine's four load/compare pairs
change in the entire unit. Its differing encodings fall from 32/228 to
zero and its instruction size becomes the retail 811 bytes. All eight
sound-unit functions now have zero normalized instruction differences;
all mapped function bodies are unchanged.

Four following functions move two bytes earlier, and the object's trailing
alignment grows by two bytes. Subsequent object addresses and section sizes
remain unchanged. Removing names from prototype parameters is not an
alternative to the `()` spelling: CL 4.1 still allocates those unnamed C++
parameter symbols, as the seven-parameter voice-helper probe confirms.

Locals take keys in declaration order, but frame slots follow name buckets.
When the locals of a function all have distinct buckets, reordering their
declarations permutes their keys without moving a slot or changing the unit's
key count. `detect_collisions` needed `object` above the parameter `obj` and
`time` above `soonest`; no uniform shift gives both, but declaring `object`
first does. `reposition_fixed_child_objects` orders two `MultiplyFixed` calls
by their last arguments; swapping the `sine`/`cosA` declarations moved the
hash wrap between them. Swapping two adjacent header `extern`s does the same
for globals and moves only those two keys (`g_nCurrentSeries`/
`g_nCurrentMission` for `GetAdaptiveTurnRate`).

Shared declarations can also need a different order in different units.
In `logic.c`, exchanging the wingman and target-lock-readout declarations
moves `g_nYourWingman_0049346c` from key 20742 to 20747 while leaving every
other key unchanged. C2's sign-extended operand signature changes from
`0x4B` to `0x40`, below the `try2rout` parameter's `0x45`; the comparison
then loads the parameter first, as retail does. `LOGIC_C` limits this order
to that unit. Applying a shared-header swap to every unit broke already
matching functions.

`TURRET_C` exchanges the right-vector declaration with player acceleration
and the forward-vector declaration with object distance. This moves the
vector keys from 20966/20968 to 20960/20961 without changing the unit's
total key count or any storage definition. The keys reproduce both
`UpdateShipTurretGuns`' three-product order and `SetTargetCameraEyePosition`'s
choice of register or memory multiply operands. All 20 turret functions
then compare normalized-byte exact, including the four-byte camera-eye
size correction and the trailing alignment span. Moving only the forward
vector fixes the gun routine but breaks camera-eye ordering. These are
verified matching declaration orders, not a claim to have recovered the
original header text; function bodies and all declared types are unchanged.

`SPEECH_C` exchanges `g_wDmaAddressA_0049bb64` and
`g_wDmaHalfBytes_0048e0e4` at keys 20179/20188. This restores the final
addition in `InitializeSoundBlaster` (`0x0044F84F`): half-buffer size loads
into AX before the first DMA address loads into CX. The 25-byte sequence at
retail `0x0044FA0B` now matches literally, versus 19/25 bytes before the
swap, as checked by `binary-comp byte-compare`. The local `/Bk` symbol
stream is byte-identical, and the only instruction changes in the unit's
compiler listing are those two load addresses. Other units retain their
header order. The normalized comparison remains 12/134 differing encodings
because relocation normalization masks the exchanged global operands; the
separate DMA-page comparison still has the wrong expression shape.

The speech-position update in `PlaySpeechPacketBuffer` (`0x0044F3A4`)
also depends on declaration order. Rotating the position/count-A/count-B
declarations gives them keys 20181/20182/21116, respectively, in `SPEECH_C`.
Retail adds count B to count A before adding the position; the earlier
header order made C2 add position to count B first. Only those three load
operands change in the unit's compiler listing, and the local symbol stream
is identical. The first 19 bytes of the sum at retail `0x0044F500` now
match literally, versus 15/19 before. The position's storage remains
`0x2D0` early, so the rest of the update still differs. This code follows
an early return: inspect the linear disassembly as well as the reachable
export when auditing it.

`NEARHEAP_C` exchanges arena-size and heap-base declarations at keys
21245/21246. Retail loads arena size before adding base in all fifteen
end-address sums across eight near-heap functions. The older ordering
reversed those operands; the exchange restores all fifteen sequences.
Local symbol streams are identical, the seven parsed defined-global records
change only those two keys, and instruction sizes and normalized comparisons
stay unchanged. These globals remain `0x2D0` bytes early with the surrounding
storage, so correcting operand order does not establish literal identity
for the near-heap functions.

`MEMFREE_C` uses the same two-key exchange for the end-address additions
in `GetAvailableMainMemory` (`0x00469AD0`) and
`GetLargestMainMemoryBlock` (`0x00469B49`). Both additions survive after
early returns and load size before base in retail. Their local symbol
streams and instruction sizes are unchanged; the only instruction changes
are the two pairs of global operands. Storage placement remains unresolved.

`TITLEMNU_C` exchanges vertical and horizontal joystick-range declarations
at keys 21481/21482. This restores all four scale-product operand orders in
`CalibrateJoystickInteractive` (`0x00418770`) without changing its 1,444
instruction bytes or local symbol stream. Relocation normalization reports
zero differences both before and after this correction. It also masks a
separate statement-order discrepancy: retail checks the left, up, right,
then down scales for zero; the source checks left, right, up, then down.
That discrepancy remains under the mapped-function body restriction.

Equality comparisons can hide the same ordering problem: relocation
normalization masks the two global operands even when C2 loads them in the
opposite order. The following declaration exchanges recover nine retail
comparisons without changing types, expressions, local symbols, or sizes:

| Unit | Comparisons corrected | Exchanged C1 keys |
| --- | --- | --- |
| `spc.c` | canned-scene segment end and space frame | 20332 / 20951 |
| `brains.c` | enemy sighting and current wave | 20191 / 20727 |
| `gameflow.c` | linked/current plane and sequence | 20471 / 20474; 20472 / 20475 |
| `main.c` | pending/displayed HUD text and playback frame counts | 20253 / 20254; 21420 / 21421 |
| `cockpt.c` | displayed/current objective range | 20268 / 21026 |
| `comm.c` | requested/loaded communication portrait | 20413 / 20668 |
| `ix/mixer.cpp` | mixer and primary sound-buffer pointers | 26530 / 26532 |

The core exchanges are scoped to their owning units; the mixer exchanges
its two file-scope extern declarations. Only the eighteen load/compare
operands change in their compiler listings. Eight comparison sequences
now match literally, totaling 103/103 bytes versus 77/103 before. The
remaining playback comparison still references displaced storage.
`check_objectives` (`0x0043B089`) now matches its entire 98-byte mapped span,
including calls, versus 94/98 before.

In `EVENTMGR_C`, exchanging centre-X/origin-X keys 20908/20914 and
centre-Y/origin-Y keys 20909/20260 restores the two additions in
`set_up_screen_viewport` (`0x004648C5`). Retail loads each centre before
its origin. All local symbol records remain identical, and these four
load operands are the unit's only instruction changes. The globals retain
their displaced addresses: the corrected order loses one accidental byte
match in the raw span, from 360/409 to 359/409, despite matching retail's
operand selection. The event-manager storage layout remains unresolved.

Constant array entries and structure fields also expose declaration-order
differences that normalized bytes can hide. Three further exchanges restore
these sequences without changing their expressions:

| Unit | Retail sequence restored | Exchanged C1 keys |
| --- | --- | --- |
| `spc.c` | yaw before pitch in `update_star_field`'s rotation test | 21016 / 20948 |
| `cockpt.c` | roll before pitch in `draw_target_box`'s marker-angle sum | 21065 / 20948 |
| `gameflow.c` | load text-backing pixels, compare secondary-view pixels in `RunCutsceneScript` | 20507 / 21184 |

The two 16-byte rotation sequences now match literally: 32/32 bytes,
versus 26/32 before. The cutscene viewport globals remain displaced, so
their restored operand order does not establish literal identity.

The cutscene frame wait at retail `0x00431A5E` needs both declaration
ordering and a signed deadline. Retail loads the input clock, compares the
next-frame deadline against it, and exits with `JLE`. The prior unsigned
deadline declaration emitted `JAE` with the operands reversed. Declare
`g_nNextCutsceneFrameClock_00499c90` as `int` in both its header and definition;
the other deadline comparison already casts it to `int`.

Within `GAMEFLOW_C`, exchanging the input clock's key 21395 with the
text-bounds override flag's key 20487 lowers the clock's load signature
from `0x2F` to `0x2E`. This puts it below the next-frame deadline's `0x2F`
while preserving `UpdateMap`'s comparison with its local deadline (`0x2E`).
An exchange with the frame-tick key 20477 instead lowers it to `0x26` and
reverses that already-correct comparison; discard that variant. Exchanging
the two clocks directly does nothing because both original load signatures
are `0x2F`.

The final timer sequence matches 15/17 literal bytes, versus 10/17 before;
only the input clock's displaced address differs. Across the three units,
local symbol streams remain identical and only eight load/compare operands
plus the timer branch change. All 4,512 linker-map symbol addresses and all
code sizes remain unchanged. `RunCutsceneScript`'s mnemonic score improves
from 91.20% to 91.23%; its normalized differing-encoding count remains
1,518/3,672 despite the corrected sequence.

A struct member and a plain global differ in tree weight, not only in key: a
member read is `LD(ADD(sym,off))`. Two operands of equal weight compare by key
hash; a member read outweighs a scalar. `fire_computer_graphic_missile`
compares the released weapon's y against `viewport.bottom`, so y has to be a
record member (`ReleaseWeaponPosition`) before any key matters.

`UpdateDamageDisplay` has four reversed coordinate sums: retail loads the
scalar origin before the sprite-position member. The current sign-extended
scalar signatures range from `0x40` to `0x4F` across all sixteen symbol
hashes. The corresponding x-member range is `0x60` to `0x6F`, and the
y-member range is `0x6F` to `0x7E`. The members always outweigh the scalars
by at least `0x11` and `0x20` respectively. Swapping their declarations
cannot correct these current expression trees; the tested swap changes no
instructions and is not retained.

`fire_afterburner` is another weight mismatch. The current product's C2
signature is `0x11E`, below the vector-magnitude call's `0x14B`, so C2
evaluates the vector call first; retail evaluates the product first.
Enumerating all four participating symbol hashes leaves the product at
least two below the vector call. Declaration order and local names alone
therefore cannot reverse these operands with the current expression trees.
An isolated `long`-to-`int` spelling change for the magnitude return type
also leaves the order unchanged. The mapped expression still needs separate
recovery; do not keep sweeping header keys for this case.

`HasSavedPilotCampaign` (`0x00434177`) has a similar limit. Its `lseek` call
has signature `0xA7`, above the offset expression's `0x71`; retail computes
the offset into EBX before calling `lseek`. Across all 256 independent hash
pairs for the function symbol and `stateBytes`, the call ranges from
`0xA0` to `0xAF` and the offset from `0x66` to `0x84`. The call remains at
least `0x1C` higher. The call's last argument is constant zero, so the file
local's key cannot affect its signature. Header declaration ordering and
local spelling cannot reverse these current expression trees; further
header-key sweeps are not warranted.

The signature-logging C2 copy from `../wc1-re/out/c2sig` runs on WC2's `/Bk`
intermediates (copy it and `msvcrt40.dll` into `out/c2trace`). Its log shows
each commutative node's operand signatures, so the needed key change can be
computed from the WC1 formulas instead of swept. A call's signature comes
from its symbol and its last argument only.

A header declaration removed by a merge must be refilled at the same spot, as
in WC1. Merging the two modal-bounds dwords into one `ShortRect` cost one
`extern` in `globals.h` and one prototype parameter name in `functions.h`;
the same two keys placed anywhere else broke 2 to 87 functions. The fillers
used are `extern short DAT_005a86a6` (a real but unreferenced word) and the
`ModalPanelBounds` typedef that the new prototype uses.

`QueuedMouseState` demonstrates a C/C++ difference in this accounting: its
tag, typedef, members, and global declaration consume three keys in the C
units and eight in the C++ units with the current headers. Replacing three
obsolete predecessor declarations at their existing location, and retaining
the next five only for C, preserves all 1,677 function report results.
Counting member declarations as one key each would instead change 119
report rows. The retained declarations allocate no storage.

### Statement shapes that /Od makes visible

Each of these was confirmed against retail code; pick the form the jumps show.

- A `return;` as the last statement emits a `JMP` to the epilogue that sits
  right before it (`E9 00000000`). Empty hooks and many void functions end
  with one; leave it out when the retail code has none.
- A non-void function that the Win32 port left without a `return` falls off
  its end with no `XOR EAX,EAX`. Keep such returns under `#ifdef SDL_PORT`.
- `if (c) return 1; return 0;` jumps straight to the epilogue from both
  arms. `return c;` (a boolean expression) instead makes the `MOV AX,1` arm
  jump to the shared `JMP` after `XOR AX,AX`.
- `if (c) return a; else return b;` adds a second `JMP` after the first
  return (the end of the then-arm). Two consecutive identical `JMP`s mean an
  `else` follows a `return`.
- `if (!c) return;` followed by the body compiles to `Jcc body / JMP end`;
  a body wrapped in `if (c) { ... }` compiles to a single inverted `Jcc`.
- The last `case` of a `switch` (including `default`) normally has no
  `break` in the original; a trailing `break` adds a `JMP` before the
  dispatch block. Case bodies are emitted in source order while the
  dispatch compares in value order, so the order of the call targets in the
  retail bodies gives the source order of the cases.
- `switch (f(x))` stores the value straight into the switch temporary; a
  separate local shows up as an extra frame slot.
- `while ((x = f()) == 0) { ... }` tests at the top with a single `Jcc` to
  the exit; a `do { } while` or `if (...) break;` form adds jumps.
- `return c ? a : b;` jumps to a shared epilogue `JMP` after the `b` arm;
  `if (c) return a; return b;` sends both arms straight to the epilogue
  (`PollKeyboardState`).
- A `break` or `return` inside an `if` whose `else` is the rest of the case
  or loop body leaves no jump over an `else`: write the early exit and drop
  the `else` (`PacketLoad`'s empty-section case, `UntrackFreedHeapBlock`).
- `x += a - b;` evaluates `a - b` first; `x = x + a - b;` starts from `x`
  (`EraseLastTextInputCharacter`).
- When two variables share one stack slot, the original reused one variable
  for both jobs (`ValidateViewportBounds`' allocation scan and top offset).
- A `for` loop jumps over its increment on entry (`JMP` then `INC`); a
  `while` with the increment at the bottom does not. Bounds keep their
  spelling: `i <= 9` compiles to `CMP 9 / JG`, `i < 10` to `CMP 0xA / JGE`.
- A structure assignment from an array element with a constant index copies
  through a computed address; component assignments (`v.x = a[K].x; ...`)
  give the three direct `MOV`s (`UpdateTargetCameraObject`).
- A call's result kept in a register across a second call (`MOV EBX,EAX`)
  is an expression temporary, not a variable: write the calls inside the
  comparison (`ix_streamer_service_audio`).
- A frame-size difference with every touched slot matching means a local
  the original did not have (fold it into the expression) or one it never
  read (see the dead-local note above).
- An early `return;` in front of a disabled DOS body, and `if (0) { ... }`
  debug blocks, keep the dead code in the image. Reproduce the dead body.
- A short argument pushed with `MOVSX` into a callee whose parameter is
  `short` means the original caller had no prototype in scope (K&R
  promotion). Casting the argument does not reproduce it.
- `x <<= 2` on a short local or parameter compiles to `SHL WORD PTR [x],2`;
  `x *= 4` loads, shifts and stores through `EAX`.
- `x == 0 ? a : b` compiles to `CMP EAX,1 / SBB EAX,EAX / AND / ADD`, and
  `!x` to `CMP EAX,1 / SBB EAX,EAX / NEG EAX`; `x < 1` uses `SETL`.
- `if (c) stmt; else ;` keeps a `JMP` over the empty else-arm, and
  `if (c) ;` keeps a test whose `Jcc` lands on the next instruction. Both
  appear in the original (`PeekInputEvent`, `PollSceneHotspotInput`).
- A `for` whose condition and step were transposed in the original
  (`for (i = 0; i++; i < n)`) keeps the post-increment test and evaluates
  the comparison as a discarded statement (`GetViewportIntersection`).
- `for (;;)` and `while (1)` both compile to a body ending in a `JMP` back
  to the top; a block that always leaves through `break` (`Title_Sequence`)
  ends with `JMP exit` followed by that back `JMP`.
- `while (0) { ... }` keeps its body behind `JMP exit` and ends it with a
  second `JMP exit`, not a jump back to the top
  (`InitializeHighMemoryGraphicsBuffers`' disabled DOS retry loop).
  Ghidra does not disassemble such bodies; read the bytes.
- Read the jump table before writing a `switch`. An entry that points at the
  switch end means there is no case for that value. Entries that share a
  lone `JMP end` are explicit empty cases (`case 4: case 5: case 6: break;`).
  Separate `JMP`s mean separate `break`s, and the case bodies appear in source
  order (`RestoreCannedSceneObjectState`, `PollPersonnelMenuInput`).
- An empty `default: break;` as the last label adds one more `JMP end`, but
  the bounds check (`JA`) still targets the switch end
  (`ApplyCannedSceneFrameEvents`).
- A switch whose temporary is the only slot holding the value switches on
  the expression itself (`switch (record->opcode)`); a named copy adds a
  slot.
- A sparse switch dispatch that compares `0xFF` (`JG end / JE end`) before
  the small cases has an explicit empty `case 0xff: break;`. Like an empty
  `default`, its label is never targeted (`ProcessMissionWaveCommands`).
- `f(g++)` copies `g` into a temporary before the increment. A retail
  `INC [g] / MOVSX [g] / DEC EAX / PUSH` sequence is `g++; f(g - 1);`
  (`RunSpaceFlight`). A loop test that copies a counter to a slot, then
  decrements and tests the copy, is `while (n-- > 0)`
  (`ApplyAlternateShipType`).
- `if (x) ;` followed by an unconditional statement is a stray semicolon in
  the original: the test branches to the next instruction
  (`your_internal_damage`'s `pilot_hit` check).
- A `case` whose quadrant check jumps straight to the switch end is written
  `if (...) { ...; break; }` with no `else` (`your_internal_damage`).
- `(*p)[i]` evaluates the index before the pointer; `p[0][i]` loads the
  pointer first (`AllocateFontWorkspace`, `FreeFontWorkspace`).
- `(a & M) != (b & M)` always folds to `((a ^ b) & M) != 0` in this
  compiler; a retail `AND`/`AND`/`CMP` sequence (`InitializeSoundBlaster`)
  comes from some other source form that has not been found yet.
- A `JMP` over a test that nothing reaches is a condition the compiler
  folded to zero: `(g & 0) && ...` (`house_keep_objects`; the DOS source
  has `g_frameCounter & 0`). `!flags & 1` parses as `(!flags) & 1` and
  compiles to `CMP/SBB/NEG/TEST AL,1` (`FinishViewportWipe`).
- The last case of a `switch` that ends in `break;` leaves two `JMP end`
  instructions before the dispatch; with no `break;` there is only the
  implicit one (`capital_ship_intelligence`). Ghidra hides the dead
  second `JMP`; count them in a linear disassembly.
- A loop body with its own declarations gives those locals separate slots
  below the function's own (`ix_mixer_service` relocks into a second set of
  pointers).
- Retail sometimes reuses one local for two unrelated jobs: the DOS `dx`
  and `dy` later hold button states (`KeyboardMousePump`). The frame shows
  this as fewer slots than the reconstruction's locals.
- A `for` loop's step sits before the test (`JMP test / INC / test`); a
  `while` with the increment at the end of the body does not
  (`InitializeCockpitResources`).
- A local the original only assigned is still a slot. The DOS sources often
  name it: `passes = 0x40` in `LoadJoystickCalibrationFile`, `done = 0` in
  `FlushPendingInputPresses`.

### Strings and library calls

Identical string literals were not pooled. Write the literal at each original
use site instead of introducing an alias. Use the matching standard function
when the binary calls or inlines `strcpy`, `memcpy`, or another CRT routine.

### High-level data access

Use typed structure fields and arrays. If a field's purpose is unknown, retain
an offset-based field name until evidence supports a better one. Raw pointer
arithmetic often hides a wrong width and makes comparison harder.

A record copied whole shows the struct-assignment shape: both source dwords
are loaded before the destination address is formed (`MOV EAX,[src]; MOV
ECX,[src+4]; MOV EDX,dst; MOV [EDX],EAX; MOV [EDX+4],ECX`). An inlined
`memcpy` forms the destination first. A record passed by value is pushed as
both loads, then both pushes. `InitializeModalTextPanel` takes its rectangle
as one 8-byte `ShortRect`. It stores the rectangle into the four loose
`Viewport` bounds through a `ShortRect` cast.

## Analysis traps

### Operational labels are not intent

A label such as `ReturnConst...`, `CallThrough...`, or `ScanTbl...` describes
what an earlier analysis observed. It may reflect decompiler folding or an
incorrect signature. Read the exported instructions before writing the body,
then apply [the naming policy](LABELS.md).

### Function annotations must be exact

Operational labels may contain only the low four address digits. Never infer a
full address from the name. A wrong `/* Function start: 0x... */` annotation can
compare a function against an unrelated original and still produce a plausible
score.

Do not place both the phrase `Function start:` and a hexadecimal address in a
prose comment. The exporter can bind that comment to the next definition.
Every real annotation must immediately precede its function.

Run:

```sh
make audit-addresses
```

### Compiler-generated glue

Do not hand-write jump thunks, import stubs, vtable dispatch, exception tables,
or other compiler/linker glue. Express a real forwarding operation in ordinary
C and let the compiler emit the tail jump. `make audit-compiler-glue` rejects
manual replicas.

Do not add C++ exception handling or spell out `__thiscall`. Inline assembly is
allowed only when the original routine itself is strongly evidenced as
hand-written assembly.

## Comparison workflow

```sh
make compare-func FUNC=FunctionName
make report FILTER=source_name
make verify
```

`compare-func` is the normal edit loop. Stop once the function is at least 90%
similar unless a clear, evidence-backed improvement remains. `make verify`
runs the repository-wide expected-zero gates. Export requirements are listed in
[EXPORT.md](EXPORT.md).

Run the reconstructed Win32 executable through DREAMM, not Wine. DREAMM
provides the Windows 95 DirectDraw and DirectSound behavior being reconstructed;
Wine substitutes different host implementations. The default DREAMM settings
use an 8-bit palettized display because the original DirectDraw palette path
depends on it.
