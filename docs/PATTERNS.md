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

A struct member and a plain global differ in tree weight, not only in key: a
member read is `LD(ADD(sym,off))`. Two operands of equal weight compare by key
hash; a member read outweighs a scalar. `fire_computer_graphic_missile`
compares the released weapon's y against `viewport.bottom`, so y has to be a
record member (`ReleaseWeaponPosition`) before any key matters.

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
