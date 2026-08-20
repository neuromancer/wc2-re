# Wing Commander II source reconstruction and SDL2 port

This project recreates the source of **Wing Commander II** as shipped in *Wing
Commander: The Kilrathi Saga* (1996). The reconstructed game core is C, the
`ix` audio library is C++, and the reference build uses Microsoft Visual C++
4.1 under [`wibo`](https://github.com/neuromancer/wibo) to reproduce the
original Win32 `WC2.EXE`.

A native SDL2 port is available for Windows, Linux, and macOS. It supports
Kilrathi Saga data and has partial support for the original DOS game data.

No copyrighted game data is included.

## Status

The reconstruction is incomplete: the source compiles and links as `WC2.EXE`,
but significant implementation, mapping, and runtime work remains. `make
report` compares 1,575 functions against the retail executable and averages
98.06% machine-code similarity, which measures reconstruction fidelity, not
gameplay completeness.

The SDL2 port reaches the title screen, the campaign intro, the pilot database,
the pilot save/load menu and space flight, with firing, targeting, the cockpit
displays and music working. A DOS install plays its own AdLib music and
synthesized OriginFX sound effects instead of the Kilrathi Saga streams and
waves. Enhanced rendering is wired but largely untested.

## Download and run the SDL2 port

Download the archive for your platform from
[GitHub Releases](https://github.com/neuromancer/wc2-re/releases). Extract its
contents into an installed Kilrathi Saga or DOS Wing Commander II directory and
keep the bundled runtime libraries beside the executable. Start it with that
directory as the working directory:

```sh
cd /path/to/WC2
./wc2-modern
```

## Build from source

Clone the submodules first:

```sh
git submodule update --init --recursive
```

### SDL2 port

Install a C/C++ compiler plus the SDL2 and LZO2 development packages, then run:

```sh
make -j modern
```

The executable is written to `out-modern/wc2-modern` (or
`out-modern/wc2-modern.exe` on Windows). `make run-modern` launches it with
Kilrathi Saga data in `data/wc2-full`; `make run-modern-dos` uses DOS data in
`data/dos`.

### Reconstructed Win32 build

The default target downloads and checksum-verifies the MSVC 4.1 package, then
builds `WC2.EXE`, `WC2.map`, and compiler-generated assembly under `out/`. The
original executable is not required to compile it:

```sh
make -j
```

To run it, provide a Kilrathi Saga disc image. The Makefile substitutes the
reconstructed executable and launches it under DREAMM:

```sh
make run WC2_ISO=/path/to/kilrathi-saga.iso
```

Use `make run-original` for the retail executable in the same environment, and
`make debug` to start DREAMM's debugger.

## Reconstruction workflow

[`binary-comp`](https://github.com/gg-sl-oss/binary-comp) is required only for
comparison and verification commands:

```sh
python3 -m pip install "binary-comp[all] @ git+https://github.com/gg-sl-oss/binary-comp.git"
make compare-func FUNC=WinMain
make verify
```

These look for the retail executable at `../releases/win32/WC2.EXE`; set
`ORIGINAL_SRC` to point elsewhere.

Contributor references:

- [compiler and flag evidence](docs/COMPILER.md);
- [matching patterns](docs/PATTERNS.md);
- [disassembly export workflow](docs/EXPORT.md);
- [compilation-unit order](docs/ORDER.md);
- [function naming policy](docs/LABELS.md);
- [SDL2 port architecture](docs/SDL2.md); and
- [release process](docs/RELEASING.md).

## License

See [LICENSE](LICENSE). OpenAI Codex and Anthropic Claude were used during the
reconstruction.
