# Wing Commander II reconstructed source

This is the [reconstructed C and C++ source](https://github.com/neuromancer/wc2-re)
of **Wing Commander II** as shipped in *Wing Commander: The Kilrathi Saga* (1996),
with the SDL2 port's code stripped out. Built with Microsoft Visual C++ 4.1,
it reproduces the original Win32 `WC2.EXE` byte for byte: all **714,752 bytes**,
including code, data, padding and PE/debug metadata.

Every reference build checks the complete executable against the retail SHA-256
and fails on a mismatch:
`f5e133e8eca8eb1a02e21a7d1afbb44d082e384a38d39db58a694371fa583bbe`.

## Source files

Each group lists its files in link order, the order of their code in
`WC2.EXE`.
