# Toolchain and compiler-flag derivation

## WC2 target on this branch

Target: `WC2.EXE`, MD5 `6e8bd31e8ad5603f74a9c8ea657d01cd`, 714,752
bytes. Built **1996-09-24 17:41:25 UTC**. Image base `0x00400000`.

WC2's PE header reports linker **3.10**. The selected decomp.me MSVC 4.1
package identifies `CL.EXE` as C/C++ compiler **10.10.6038** and `LINK.EXE` as
incremental linker **3.10.6038**, matching the shipped image's linker stamp.
Both the game core and ix library have the unoptimized debug-build shape, so
this branch uses `/MTd /Od /Oi` throughout.

The PE `TimeDateStamp` and all three `IMAGE_DEBUG_DIRECTORY` timestamps are
`0x32481D45` (1996-09-24 17:41:25 UTC). LINK 3.10 writes the current time;
`bin/stampPeTimestamp.py` restores those four fields after linking. It
validates the PE32 header, the file-backed debug directory, matching link
times, and the zero image checksum before writing. It does not modify code,
data, section sizes, or debug payloads. The separate CodeView identity is
produced by LINK from a seeded PDB, as described below.

LINK's `/OUT:Debug/wc2.exe` reproduces the retail `IMAGE_DEBUG_MISC`
executable-name record, including its 272-byte payload and trailing zeros.
The build links to that path, restores the link timestamps, then copies the
image to the usual `WC2.EXE` target. This changes no code, data, import, or
relocation bytes. The debug payload still starts 512 file bytes before its
retail location because the preceding image remains shorter.

The linker runs with wibo's optional `WIBO_DRIVE_C` mapping rooted at
`out/drive-c` and `/PDB:C:/WCB/wc2/Debug/wc2.pdb`. LINK therefore emits the
retail `C:\WCB\wc2\Debug\wc2.pdb` path directly, with the matching 41-byte
CodeView `NB10` record. The actual PDB lives at
`out/drive-c/WCB/wc2/Debug/wc2.pdb`. No post-link path patch is needed.
Without the mapping, wibo returns every absolute host path with a `Z:`
prefix; its default behavior is unchanged outside the linker invocation.
The Makefile rebuilds wibo when its path implementation or CMake file changes.

Binary-comp confirms that this path change leaves `.text`, `.data`, `.idata`,
`.reloc`, and the MISC/FPO payloads byte-identical to the preceding build.
Only CodeView's size field changes in `.rdata`, from 59 to 41.

`bin/createPdbSeed.c` is a build utility using the actual `MSPDB41.DLL` C API.
It creates a PDB with initial signature `0x32385F33`, then write-opens it
until its age is 21. The Makefile copies that seed before each link. LINK's
write-open advances the age to retail's 22 and emits all 41 CodeView bytes
exactly, including signature, age, and path. The utility verifies the linked
PDB through read-only API calls; neither the PDB nor the CodeView record is
patched. Game objects and all five PE section payloads remain unchanged.

The installed DLL proves the API behavior: `PDBOpenEx` increments an existing
database's age at `0x10309C83`. For a new database, `0x10309C95` loads the
caller-supplied signature and uses the clock only when it is zero;
`0x10309CA9/0x10309CB0` store the signature and initialize age zero.
Microsoft's published [PDB implementation](https://github.com/microsoft/microsoft-pdb/blob/master/PDB/dbi/pdb.cpp)
also documents the initial-signature argument and write-open age increment,
although its newer databases start at age one. The utility checks the legacy
implementation version `19950814` and every expected age.

`make test-pdb-seed` checks creation, age zero, non-mutating verification,
rejection of existing output, wrong identities, and missing/invalid inputs.
Two forced links after independently regenerating the seed produce identical
714,240-byte executables under binary-comp's strict comparison. This removes
PDB relink history as a source of executable nondeterminism.

The remaining scalar COFF/optional-header differences are `SizeOfCode`,
`SizeOfInitializedData`, and `AddressOfEntryPoint`, all consequences of the
unresolved image layout. Section and debug-record offsets still differ.

All 126 FPO records already have the retail sizes, local/parameter counts,
and flags; their start RVAs remain shifted with the CRT code. The FPO payload
has 252 differing bytes, all in those start-address fields.

### WC2 DirectDraw timing initializer unit

The four routines at `0x0045CCB0`-`0x0045CD2B` are two MSVC scalar dynamic
initializer pairs, and their entry points occur in the target's `CRT$XCU`
table. The first initializes the millisecond frame period from the 70 Hz
display-rate global; the second converts that global to the prior frame-rate
float. MSVC 4.1 emits both instruction sequences exactly from ordinary C++
file-scope initializers. They therefore live in `src/dibtime.cpp`, with `/GX`
still disabled and the shared timing globals retaining C linkage.

The controlled remap report separates optimizer and compiler effects:

| Rebuild configuration | Average | Exact | >=90% |
| --- | ---: | ---: | ---: |
| MSVC 4.20 with WC1's optimized core flags | 47.48% | 153 | 176 |
| MSVC 4.20 with `/Od /Oi` | 74.01% | 302 | 460 |
| MSVC 4.1 with `/Od /Oi` | 74.01% | 303 | 461 |

Thus optimization accounts for the material code-generation gap. MSVC 4.1
also supplies the exact linker generation and improves two threshold counts,
although the rounded aggregate is unchanged.

## Historical WC1 predecessor

Target: `WC1.EXE`, MD5 `b20a68b7e45f837e59f7e31bab2e2020`, 518,656 bytes.
Built **1996-09-24 16:33 UTC**. Image base `0x00400000`.

### Toolchain: Microsoft Visual C++ 4.20

| Signal | Value | Reading |
|---|---|---|
| `MajorLinkerVersion.Minor` | 4.20 | link.exe shipped with Visual C++ 4.2 (Aug 1996) |
| Rich header | absent | `DanS` only appears from VC++ 6.0's linker |
| Sections | `.text .rdata .data .idata .reloc` | separate `.idata` = pre-VC5 linker default |
| DOS stub | "This program cannot be run in DOS mode." | MS `winstub`, not Borland/Watcom |
| Imports | no `MSVCRT.DLL`; `GetStringTypeW`, `LCMapStringW`, `RtlUnwind`, `TlsAlloc`, `SetHandleCount` | static MS CRT fingerprint |
| CRT `__FILE__` set | `aw_cmp.c`, `aw_env.c`, `aw_loc.c`, `aw_map.c`, `aw_str.c` | 4.x generation (VC5+ split these into `a_*.c` + `w_*.c`) |
| Startup `entry` @ `0x0044E770` | `_amsg_exit(28)` / `_amsg_exit(16)` on heap/thread init failure | VC 5.0+ uses `fast_error_exit()` here |
| Startup | **no** `__set_app_type()` | introduced in VC++ 5.0's `crt0.c` |
| Startup | `_mtinit`, `tidtable.c` | multithreaded CRT |

The generation (MSVC 4.x) is certain. The point release comes from the 4.20 linker stamp,
corroborated by the Sept 1996 build date.

For contrast: the DOS original (`../releases/dos/WC.EXE`) is **Borland Turbo C++ 1.0** with
VROOMM overlays (`Turbo C++ - Copyright 1990 Borland Intl.`, `Runtime overlay error`). So the
Win32 port is a genuine recompile, not a wrapper.

### WC1 flag derivation

Each flag in the Makefile is backed by an observation. Do not change these casually — the
comparison depends on them.

### `/MTd` — static debug multithreaded CRT

The shipped retail executable links **LIBCMTD**. Present in the image:
`dbgheap.c`, `dbgrpt.c`, `dbgdel.cpp`, `_CrtCheckMemory`, `_CrtIsValidHeapPointer`,
`_CrtMemCheckpoint`, `DAMAGE: after %hs block (#%d) at 0x%08X.`, `Detected memory leaks!`,
`client block at 0x%08X, subtype %x, %u bytes long.`, plus `Microsoft Visual C++ Debug Library`.
The multithreaded half is confirmed by `tidtable.c`, `_mtinit`, and the
`CreateThread`/`TlsAlloc`/`InitializeCriticalSection` imports.

### `/Oy` — omit frame pointers

Almost every game function reads its arguments as `[esp+N]` with no `push ebp; mov ebp,esp`
prologue. Typical recovered prologue: `push esi; push edi; mov si, word [esp+0xC]`.

### NO `/Gf` — string pooling off

Two byte-identical `"DIBsetWholePalette   SetEntries"` literals exist, at `0x0046B6E0` and
`0x0046B71C`, emitted from two different call sites. Identical strings were not merged.

### `/GX` — off by default, enabled for the console owner

Most game-core units have no C++ unwind metadata and use no RTTI type descriptors (`.?AV`,
`.?AU`).  Their SEH is the C `_except_handler3` scope-table form; for example, `entry`'s table
is at `0x00463BE0` (`{prev=-1, filter=0x0044E987, handler=0x0044E9A7}`).

The wrapper at `0x00425B00` is the one proven exception.  Its handler thunk at `0x00425B73`
loads the C++ function-info record at `0x00464B48` (magic `0x19930520`) and jumps to
`__CxxFrameHandler` at `0x0044DA70`; the unwind action at `0x00425B7D` passes the pending
allocation to `operator delete`.  MSVC 4.20 emits this sequence from the console `new`
expression only when the owning `pilot.cpp` unit is compiled with `/GX`.  The flag is therefore
target-specific rather than part of `CFLAGS_CORE`.

### Isolated C++ debug-overlay utility

The game logic remains C, but the Win32 developer-console unit at `0x0041C760`–`0x0041D0BF`
is a proven C++ exception: `0x00425B00` allocates a 0x45C-byte object with `operator new`,
passes it in ECX to the constructor at `0x0041C760`, and `0x00425B90` invokes the ECX
destructor at `0x0041C910` before `operator delete`.  The console's other ECX methods and
callee-cleanup arguments use the same member-function ABI.  The class has no vtable or RTTI;
only its owner needs `/GX`, for the constructor-failure cleanup described above.

### Game-side wave unit compiled as C++

The source unit retained as `src/sound.c` is a second, narrower language exception. Its
game-facing functions keep C linkage, but `playWAVE` (`0x0042B4A0`) and
`PlaySnowStaticSound` (`0x0042B680`) load an `IxSample` or `IxSound` into ECX and call the
library's C++ member functions directly. A C compilation cannot emit those implicit-this
calls without non-original glue. Compiling this unit with `/TP` reproduces both functions at
100.00% similarity while retaining the core optimizer flags and keeping `/GX` disabled.

### Optimizer set `/Og /Oi /Ot /Ob1` — starting point, NOT yet verified

Carried over from the sibling MSVC 4.20 project as a plausible baseline. These still need
per-function confirmation; `make report` similarity is the signal. If a whole class of
functions is systematically off, suspect this line before suspecting the source.

### 16-bit operand density — a source property, not a flag

The core was ported from 16-bit DOS C where `int` was 16 bits. 814 developer functions carry a
measurable `66` operand-size-prefix density (median 0.091 across the game core, 0.000 in the
CRT). This is achieved by declaring variables `short`, **not** by a compiler switch. If your
build emits 32-bit ops where the original used 16-bit, the declared type is wrong.

## Link settings

- `/SUBSYSTEM:WINDOWS`, `/ENTRY:WinMainCRTStartup` — subsystem 2, subsystem version 4.0.
- SectionAlignment `0x1000`, FileAlignment `0x200` (MSVC 4.x defaults).
- Base relocations are **present** in this EXE. Later MSVC linkers made `/FIXED` the default
  for executables; keeping `.reloc` matches the mid-90s behaviour.
- Import libraries needed: `ddraw.lib`, `dsound.lib` from a DirectX 2/3-era SDK (MSVC 4.2
  predates them). `DDRAW.DLL` and `DSOUND.DLL` are bound through the import table, not
  `LoadLibrary`.

Full import set: `KERNEL32` (95), `USER32` (29), `GDI32` (9), `ADVAPI32` (4), `WINMM` (6),
`DSOUND` (2: `DirectSoundCreate`, `DirectSoundEnumerateA`), `DDRAW` (1: `DirectDrawCreate`).
DirectDraw interfaces are then used through COM vtables — e.g. `IDirectDraw2` vtbl `+0x58`
(`WaitForVerticalBlank`), `IDirectDrawPalette` `+0x18` (`SetEntries`), `IDirectDrawSurface`
`+0x7C` (`SetPalette`).
