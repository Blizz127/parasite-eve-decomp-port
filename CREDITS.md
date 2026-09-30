# Credits

## Parasite Eve decompilation by khasinski

Part of the decompiled code in `src/` was imported from
**khasinski (Chris Hasinski)**'s decompilation of the same retail binary:
[khasinski/parasite-eve-decomp](https://github.com/khasinski/parasite-eve-decomp),
at commit `3067919e`. Each imported function keeps a header naming the
source, and every file is listed in [UPSTREAM_FILES.md](UPSTREAM_FILES.md).
That project has not chosen a license, so its code is **not** relicensed
here. It remains its authors' work and is included with attribution. The
owner is re-deriving those functions independently from the retail binary.
The prebuilt release is compiled from the full tree and so contains that
code.

## Structural reference

The [Parasite Eve 2 decompilation](https://github.com/GabeRealB/parasite-eve-2-decomp)
(GabeRealB) was studied as a reference for project layout and tooling. No
source or configuration was copied from it.

## Decompilation, port, tools and documentation

Written by [Blizz127](https://github.com/Blizz127), with AI coding
assistants (Claude, Codex and others) under the owner's direction.

## Tools

| Tool | Use | License |
|---|---|---|
| [splat](https://github.com/ethteck/splat) | Splitting the binaries | MIT |
| [spimdisasm](https://github.com/Decompollaborate/spimdisasm) | Disassembly | MIT |
| [maspsx](https://github.com/mkst/maspsx) (Mark Street) | PSX assembler wrapper. A patched copy is in `tools/era/maspsx/`, with its MIT `LICENSE` | MIT |
| [decompals/old-gcc](https://github.com/decompals/old-gcc) | GCC 2.7.2 / 2.8.1 era compilers for the matching build (fetched by `scripts/setup_era.sh`, not included) | GPL |
| [objdiff](https://github.com/encounter/objdiff) | Object diffing | Apache-2.0 / MIT |
| GNU binutils (mipsel) | Assembler and linker | GPL |

None of these tools is redistributed here, except the patched maspsx copy
named above.

## Libraries

| Component | Use | License |
|---|---|---|
| [PsyCross](https://github.com/OpenDriver2/PsyCross) | Fetched by `scripts/fetch_psycross.sh` for later integration; not linked yet | MIT |
| libX11, libpulse-simple | Window and sound, loaded at runtime from the system | MIT, LGPL-2.1 |

See [pc_port/THIRD_PARTY.md](pc_port/THIRD_PARTY.md) for details.
