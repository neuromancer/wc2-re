# Producing `code-full/`

`make report` stages `data/full/WC2.ORI.EXE` and creates a stamped
`code-full/` export automatically. `make export-asm` forces a clean
regeneration.

`code-full/` holds the original disassembly that every binary-comp comparison reads. Files
are named by **entry address**, not by symbol:

```
code-full/FUN_0041D0C0.disassembled.txt
    Function: MinShort
    Address: 0x0041D0C0

    MOV CX,word ptr [ESP + 0x8]
    ...
```

Internal branch targets appear as `LAB_%08X:` labels, direct calls as `CALL 0x%08X`, and
in-body jumps as `Jxx      LAB_%08X`.

There are two ways to produce it, and they are complementary.

## 1. `make export-asm` — from the PE, no Ghidra needed

```sh
make export-asm
```

binary-comp disassembles the original executable with Capstone and writes exports for every
function it can identify. By default it takes its targets from the
`/* Function start: 0xADDR */` annotations already present in `src/`, which is exactly what
you want while implementing: annotate a function, run `make export-asm`, and its export
appears.

Useful flags via `EXPORT_ASM_FLAGS`:

```sh
make export-asm EXPORT_ASM_FLAGS=--discover   # also find starts from calls/jumps/prologues
make export-asm EXPORT_ASM_FLAGS=--clean      # drop stale exports first
make export-asm EXPORT_ASM_FLAGS="--map ORIGINAL.map"   # if an original map ever surfaces
```

Limitation: it only knows addresses, so the `Function:` header carries whatever name the
source annotation implies. It has no access to the names in the Ghidra database.

## 2. `ExportToCompile.java` — from Ghidra, with names

The canonical exporter is maintained in binary-comp as
[`ghidra_scripts/ExportToCompile.java`](https://github.com/gg-sl-oss/binary-comp/blob/main/ghidra_scripts/ExportToCompile.java).
Download that script, then run it from Ghidra's **Script Manager** with `WC2.EXE` open. It
exports, for every function:

- `FUN_XXXXXXXX.disassembled.txt` — disassembly, with the Ghidra function name in the header
- `FUN_XXXXXXXX.decompiled.txt` — decompiler output, used by the call checks
- `globals.h` — the global data inventory `verify-globals` compares against
- `strings.txt` — the address-to-string map

Point its output at this repo's `code-full/`.

**This is the export you want for real work**, because it carries the recovered names and
because `verify-globals` / `verify-calls` need `globals.h` and the decompiled text, which
`export-asm` does not produce.

Note: the Ghidra MCP server in this environment has script execution disabled
(`GHIDRA_MCP_ALLOW_SCRIPTS` is unset), so the script has to be run from Ghidra's UI rather
than driven remotely.

## Which comparisons need what

| Command | Needs |
|---|---|
| `make compare-func FUNC=X` | `FUN_<addr>.disassembled.txt`, linked `WC2.EXE` + `WC2.map` |
| `make report` | same, for every annotated function |
| `make order` | same |
| `make verify-calls` | additionally the `.decompiled.txt` exports |
| `make verify-globals` | additionally `code-full/globals.h` |
| `make compare` / `compare-functions` | `data/full/WC2.ORI.EXE` and a linked `WC2.EXE` |
| `make compare-bytes` | both executable files; strict identity including padding and PE/debug metadata |

Everything above also requires the project to **link**, because binary-comp
compares the linked PE and map rather than individual source files.

`make report` includes binary-comp's normalized byte comparison. Its `EXACT`
label masks relocation operands and external call/jump targets; it does not
prove literal file identity. `make compare-bytes` performs that final check
without masking and returns a failure status when any byte or file size differs.

The `BYTES differing/total` label counts differing decoded instruction
encodings, plus one grouped difference for any bytes outside the decoded
instructions. For example, `FillPolygon` reports `BYTES 1/12` even though
its twelve reachable instructions agree: its span also contains the disabled
polygon body. A small numerator is therefore not necessarily a small number
of instructions left to reconstruct.

Function spans at library boundaries can include padding or unmapped thunks.
For example, the restored `IxStreamFile::ix_stream_file_is_reading` at
`0x0046DED9` has 46 instruction bytes, all literally matching, but its mapped
span extends into the import gap. Do not treat that span's reported size
difference as the method's instruction-size difference.

LINK's unmodified symbol map is retained at `out/WC2.link.map`. The build
adds 123 compiler-private data symbols to `WC2.map` using
`bin/appendStaticSymbols.py`. Their linked addresses are recovered from
COFF section offsets, public-symbol anchors, and relocated `DIR32` operands
in the rebuilt image. Address suffixes are never used as linked addresses,
and the script does not read the original executable. This lets
`make verify-globals-data` check all 1,243 serialized globals after grouping
the personnel mouse sample: the 25 that previously lacked map symbols now
compare successfully. This covers declared storage; it does not prove that
all original data has been reconstructed.

The `verify-globals-code` inventory check still needs the Ghidra-generated
`code-full/globals.h`, which is absent from the current automatic export.
The current legacy configuration also points its header scan at the absent
`src/globals.h`; scanning `include/globals.h` explicitly reports declarations
for assembly storage and string literals that the combined C/C++ manifest
does not define. A passing inventory subcommand therefore does not establish
complete global coverage. Use the strict whole-file comparison for the
completion claim.

## Matching checkpoint (2026-11-06)

The current `binary-comp report --bytes` covers 1,677 annotated functions:
1,618 normalized `EXACT`, 18 same-size byte differences, and 41 differing
spans. Its mean mnemonic similarity is 99.93%; 1,660 functions score 100%.
These are different measurements from literal byte identity.

Local-name recovery in `BlitWipeRadialBands` reduces its normalized byte
differences from 90 instruction encodings to four out of 263. Its stack slots
now agree with retail; the remaining four loads select different row pointers
(see [PATTERNS.md](PATTERNS.md)). At that step, all other report rows and
the whole-file positional result stayed unchanged; the function remains
displaced from its retail address.

Declaration ordering scoped to `logic.c` and `turret.c` makes `try2rout`,
`UpdateShipTurretGuns`, `SetTargetCameraEyePosition`, and the trailing
`FinalizeTargetCameraViewHook` span normalized-byte exact. These four rows
were the only report changes at that step. The camera-eye routine lost four
excess code bytes, with four alignment bytes restored at the unit's end,
leaving the whole-image sizes unchanged at that step. No function body,
type, initializer, or storage definition changed.

The DIB change reproduces all nine local slots in `DIBslamReal` and its
row-loop comparison through names and declaration ordering. It now scores
99.28%, with six missing bytes instead of 53 excess bytes. The missing bytes
are two stores of surface-unlock results; those assignments remain absent
under the mapped-body restriction. The trailing `DIBwaitForVerticalBlank`
span changes from four excess padding bytes to one missing padding byte.
Its 36 instruction bytes remain normalized-exact; the current span has
six alignment bytes where retail has seven.
Those were the only normalized report changes at that step.
The smaller code and alignment reduced `.text` by 64 bytes and shifted later
functions, reducing the byte-identical mapped-span count from 586 to 580.
Correcting one function does not necessarily improve a positional metric
while other size differences remain.

The latest local-name recovery covers `ReadTextInput`,
`RunPilotSaveLoadMenu`, and `MainWindowProc`. Their differing instruction
encodings decrease from 115 to 47, 222 to 77, and 220 to 146 respectively
(the instruction-extent comparison in [PATTERNS.md](PATTERNS.md)).
The save/load menu loses 156 excess code bytes, leaving 15. Four fewer
alignment bytes after `RunPilotDatabaseMenu` make its reported span delta
change from -123 to -127 without changing its instructions. These are the
only two changed rows in the full normalized report; `.text` shrinks by
160 bytes in total. The 41 differing spans now sum to +222 bytes, including
the library gaps described above.

The unnormalized `binary-comp exe --functions` comparison finds
580 byte-identical mapped spans and 261 functions at their retail addresses.
The first address drift follows `inflict_damage`: the next function starts
at `0x0041261C`, 17 bytes before retail `0x0041262D`. Span sizes can include
library gaps as described above, so neither the 41 spans nor their net size
delta is a count of source defects.

| Whole-image measurement | Retail | Rebuilt |
| --- | ---: | ---: |
| File bytes | 714,752 | 714,240 |
| `.text` virtual bytes | 576,332 | 575,612 |
| `.data` virtual bytes, including zero-filled storage | 1,332,200 | 1,331,480 |
| Entry point | `0x00473E50` | `0x00473B90` |

`make compare-bytes` still fails. With the link timestamps restored and
the three additional local layouts corrected, it reports 139,718 identical
file positions out of 714,752 (19.5478%). Address
drift changes encoded operands and shifts subsequent bytes, so that number
is not a percentage of reconstruction work completed. Remaining work includes
instruction shape, uninitialized-data layout, library boundary alignment,
and the CodeView/PDB metadata. Passing the declared-global data comparison
does not resolve those differences.

Packing the proven nine-byte personnel mouse sample preserves all function
scores and corrects its internal field offsets, including the unaligned flags
word at `+7`. Allocation and alignment now leave the `.data` virtual size
720 bytes short of retail, down from 736. Its absolute placement and the
unreferenced extent after the sample still need evidence; this is not a
complete reconstruction of the event-manager commons.

The first source-level size discrepancy to investigate is `inflict_damage`
at `0x00411C72` (17 bytes short). The three routines below 90% mnemonic
similarity are `fire_afterburner` at `0x0041693A` (70.45%), `DIBupdate` at
`0x0045E060` (84.40%), and `SetFleetOverviewView` at `0x0044ADE0` (85.27%).
Three expression-shape differences remain in `RunPilotSaveLoadMenu`: our
compiler combines the 38-byte slot header and 96-byte pilot profile into
one addition of 134, while retail emits separate additions of 38 and 96.
The record sizes are already correct; changing them would not fix the source
expression shape. Both value-audit passes now report zero findings after
the local-layout correction, although direct `binary-comp compare` still
shows all three combined additions. The earlier value-audit findings have
disappeared without these instructions being corrected.

`make verify` now completes with exit status zero. Its reports still list
22 functions with call-target diagnostics and 26 with global-access
diagnostics. Those commands do not fail the default verification target on
these findings, and some diagnostics compare source references with
unreachable code or unresolved symbols. Neither the exit status nor the
zero value-audit count establishes zero differences or complete coverage.
