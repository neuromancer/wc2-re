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
`make verify-globals-data` check all 1,247 serialized globals: the 25 that
previously lacked map symbols now compare successfully. This is coverage
of declared storage, not proof that all original data has been reconstructed.

The `verify-globals-code` inventory check still needs the Ghidra-generated
`code-full/globals.h`, which is absent from the current automatic export.
The current legacy configuration also points its header scan at the absent
`src/globals.h`; scanning `include/globals.h` explicitly reports declarations
for assembly storage and string literals that the combined C/C++ manifest
does not define. A passing inventory subcommand therefore does not establish
complete global coverage. Use the strict whole-file comparison for the
completion claim.

## Matching checkpoint (2026-11-05)

The current `binary-comp report --bytes` covers 1,677 annotated functions:
1,614 normalized `EXACT`, 20 same-size byte differences, and 43 differing
spans. Its mean mnemonic similarity is 99.93%; 1,659 functions score 100%.
These are different measurements from literal byte identity.

The unnormalized `binary-comp exe --functions --no-build` comparison finds
584 byte-identical mapped spans and 261 functions at their retail addresses.
The first address drift follows `inflict_damage`: the next function starts
at `0x0041261C`, 17 bytes before retail `0x0041262D`. Span sizes can include
library gaps as described above, so neither the 43 spans nor their net size
delta is a count of source defects.

| Whole-image measurement | Retail | Rebuilt |
| --- | ---: | ---: |
| File bytes | 714,752 | 714,240 |
| `.text` virtual bytes | 576,332 | 575,836 |
| `.data` virtual bytes, including zero-filled storage | 1,332,200 | 1,331,464 |
| Entry point | `0x00473E50` | `0x00473C70` |

`make compare-bytes` still fails. After restoring the link timestamps it
reports 139,913 identical file positions out of 714,752 (19.575%). Address
drift changes encoded operands and shifts subsequent bytes, so that number
is not a percentage of reconstruction work completed. Remaining work includes
instruction shape, uninitialized-data layout, library boundary alignment,
and the CodeView/PDB metadata. Passing the declared-global data comparison
does not resolve those differences.

The first source-level size discrepancy to investigate is `inflict_damage`
at `0x00411C72` (17 bytes short). The three routines below 90% mnemonic
similarity are `fire_afterburner` at `0x0041693A` (70.45%), `DIBupdate` at
`0x0045E060` (84.40%), and `SetFleetOverviewView` at `0x0044ADE0` (85.27%).
The value audit reports three differences in `RunPilotSaveLoadMenu`: our
compiler combines the 38-byte slot header and 96-byte pilot profile into
one addition of 134, while retail emits separate additions of 38 and 96.
The record sizes are already correct; changing them would not fix the source
expression shape.

`make verify` now completes with exit status zero. Its reports still list
22 functions with call-target diagnostics, 26 with global-access diagnostics,
and the three value differences above. Those commands do not fail the default
verification target on these findings, and some diagnostics compare source
references with unreachable code or unresolved symbols. The exit status is
therefore not a claim of zero differences or complete inventory coverage.
