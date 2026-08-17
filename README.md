# Wing Commander II source reconstruction

This project reconstructs the source of **Wing Commander II** as shipped in
*Wing Commander: The Kilrathi Saga* (1996). The target is the Win32
`WC2.EXE`.

The reconstructed game core is C and the `ix` audio library is C++. The
reference build uses Microsoft Visual C++ 4.1 under
[`wibo`](https://github.com/neuromancer/wibo).

No copyrighted game data is included.

## Status

The reconstruction is incomplete. The current source compiles and links as
`WC2.EXE`, but significant implementation, mapping, and runtime work remains.

The SDL2 port inherited with the source does not currently work. This is
expected until the WC2 reconstruction is complete.

`make report` measures machine-code similarity against the retail executable.
The current report compares 1,339 functions:

| Result | Functions |
| --- | ---: |
| Exact matches | 643 |
| Similarity at or above 90% | 959 |
| Similarity below 90% | 380 |

The current average similarity is 90.09%, with no report errors or missing
disassembly exports. Similarity measures reconstruction fidelity, not gameplay
completeness.

## Build

Initialize the `wibo` submodule and build the reconstructed Win32 executable:

```sh
git submodule update --init --recursive
make -j4
```

The Makefile downloads and checksum-verifies the MSVC 4.1 compiler package and
the support files needed to link the executable. The build produces
`WC2.EXE`, `WC2.map`, and compiler-generated assembly under `out/`.

The original game executable is not required to compile the reconstruction.

## Run

A legally obtained Kilrathi Saga disc image is required. The reconstructed
Win32 executable is run through DREAMM:

```sh
make run WC2_ISO=/path/to/kilrathi-saga.iso
```

Run the retail executable in the same environment with:

```sh
make run-original WC2_ISO=/path/to/kilrathi-saga.iso
```

Start DREAMM's debugger with:

```sh
make debug WC2_ISO=/path/to/kilrathi-saga.iso
```

## Reconstruction workflow

[`binary-comp`](https://github.com/gg-sl-oss/binary-comp) is used for
machine-code comparison and verification:

```sh
python3 -m pip install "binary-comp[all] @ git+https://github.com/gg-sl-oss/binary-comp.git"
make wc2-remap-audit
make report
make compare-func FUNC=WinMain
make verify
```

By default, comparison commands look for the retail executable at
`../releases/win32/WC2.EXE`. Set `ORIGINAL_SRC` to use another path:

```sh
make report ORIGINAL_SRC=/path/to/WC2.EXE
```

The address migration manifest is in
[`reports/wc2-address-remap.tsv`](reports/wc2-address-remap.tsv).

Contributor documentation:

- [compiler and flag evidence](docs/COMPILER.md)
- [matching patterns](docs/PATTERNS.md)
- [disassembly export workflow](docs/EXPORT.md)
- [compilation-unit order](docs/ORDER.md)
- [function naming policy](docs/LABELS.md)

## License

See [LICENSE](LICENSE). OpenAI Codex and Anthropic Claude were used during the
reconstruction.
