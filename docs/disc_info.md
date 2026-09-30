# Disc information — Parasite Eve (USA / NTSC-U)

Primary target release. All hashes below marked **TODO** must be filled in
during Phase 1 from a user-supplied dump and cross-checked against redump.

## Disc 1 — SLUS-00662 (recorded 2026-07-04 — redump cross-check pending)

All values below were produced by `scripts/extract_us.sh 1` (which runs
`tools/verify/hashfile.py`, `tools/extract/psxiso.py`, and
`tools/analysis/psxexe_info.py`) against a user-supplied bin+cue dump.

### Image

| Field | Value |
| --- | --- |
| Serial | SLUS-00662 |
| Region | NTSC-U |
| Cue layout | single track: `TRACK 01 MODE2/2352` |
| Image size | 495,531,120 bytes (210,685 × 2352-byte sectors) |
| Image CRC-32 | `488641e0` |
| Image MD5 | `efae1df4faf4aadbfd4dc3aa022296cf` |
| Image SHA-1 | `c339455d5b1dae04f77c2ee847d0932adaf2e84b` |
| ISO9660 system id | `PLAYSTATION` |
| ISO9660 volume space | 210,685 sectors (equals image sector count) |

**Redump cross-check: PENDING.** redump.org (disc page
http://redump.org/disc/116/) was unreachable (ECONNREFUSED) on 2026-07-04,
retried same day. The hashes above are computed facts about the local dump;
whether they match redump's Track 1 values is still unverified against
redump itself.

**Independent corroboration (2026-07-04):** an unrelated third-party catalog
(GitHub `portforge/portforge-mediaitems`,
`PS1Rom/Parasite Eve (USA) (Disc 1) · PS1/.mediaitem.json`, found via GitHub
code search for our SHA-1) lists a redump-named dump with identical size,
CRC-32, MD5, and SHA-1. This corroborates that our dump matches the commonly
circulated one, but it is not an authoritative redump verification.

### Main executable — `SLUS_006.62`

Extracted from LBA 24 of the ISO9660 filesystem. `SYSTEM.CNF` confirms it is
the boot executable (contents, CRLF stripped):

```text
BOOT=cdrom:\SLUS_006.62;1
TCB=4
EVENT=16
STACK=801fff00
```

| Field | Value |
| --- | --- |
| Size | 2,025,472 bytes |
| CRC-32 | `7c10c01c` |
| MD5 | `cb095240a2ba358b8fdcbfd4d4f97f04` |
| SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` |
| pc0 (entry point) | `0x80072534` |
| gp0 | `0x00000000` |
| t_addr (load address) | `0x80010000` |
| t_size | `0x1EE000` (2,023,424 = file size − 2048 header ✓) |
| s_addr | `0x801FFFF0` |
| region marker | `Sony Computer Entertainment Inc. for North America area` |

### Filesystem (25 files, from `tools/extract/psxiso.py list`)

```text
    LBA       size  path
     23         60  SYSTEM.CNF;1
     24    2025472  SLUS_006.62;1
   1013  206213120  PE.IMG;1
 101704         40  FMV1/PEDISC01.IDF;1
 101705    3074048  FMV1/FMV000.STR;1
 103206   20955136  FMV1/FMV002.STR;1
 113438   24150016  FMV1/FMV003.STR;1
 125230   12025856  FMV1/FMV004.STR;1
 131102    5259264  FMV1/FMV005.STR;1
 133670   15908864  FMV1/FMV006.STR;1
 141438    4374528  FMV1/FMV006C.STR;1
 143574    9601024  FMV1/FMV007.STR;1
 148262    4718592  FMV1/FMV008.STR;1
 150566    5799936  FMV1/FMV009.STR;1
 153398   10567680  FMV1/FMV010.STR;1
 158558   11763712  FMV1/FMV011.STR;1
 164302    7913472  FMV1/FMV012.STR;1
 168166    8208384  FMV1/FMV013.STR;1
 172174    9256960  FMV1/FMV014.STR;1
 176694    7487488  FMV1/FMV015.STR;1
 180350    2523136  FMV1/FMV016A.STR;1
 181582    2392064  FMV1/FMV016B.STR;1
 182750    9863168  FMV1/FMV017A.STR;1
 187566    4456448  FMV1/FMV017B.STR;1
 189742   42584064  FMV1/FMV001.STR;1
```

Structural observation (verified from the listing above): apart from the
boot files and FMV streams, **all game data lives in a single 206 MB packed
archive `PE.IMG`** starting at LBA 1013. Its internal format is a later
research task (Phase 5+); do not extract or commit its contents.

## Disc 2 — SLUS-00668 (recorded 2026-07-04 — redump cross-check pending)

All values below were produced by `scripts/extract_us.sh all` (same tools as
disc 1) against a user-supplied bin+cue dump, after the fail-loudly fix to
the script.

### Image

| Field | Value |
| --- | --- |
| Serial | SLUS-00668 |
| Region | NTSC-U |
| Cue layout | single track: `TRACK 01 MODE2/2352` |
| Image size | 646,877,616 bytes (275,033 × 2352-byte sectors) |
| Image CRC-32 | `226ee2ee` |
| Image MD5 | `c8e01c1d77baadcb89685eaee839a824` |
| Image SHA-1 | `6dc5b537527aa0d54bdbbd0c14b458083b0743e4` |
| ISO9660 system id | `PLAYSTATION` |
| ISO9660 volume space | 275,033 sectors (equals image sector count) |

**Redump cross-check: PENDING.** redump.org still unreachable (ECONNREFUSED)
on 2026-07-04, retried same day. Same status as disc 1, including the
independent corroboration: the same third-party catalog
(`PS1Rom/Parasite Eve (USA) (Disc 2) · PS1/.mediaitem.json`) lists identical
size, CRC-32, MD5, and SHA-1 for a redump-named disc 2 dump.

### Main executable — `SLUS_006.68`

Extracted from LBA 24. `SYSTEM.CNF` (CRLF stripped) confirms it boots:

```text
BOOT=cdrom:\SLUS_006.68;1
TCB=4
EVENT=16
STACK=801fff00
```

**`SLUS_006.68` is byte-identical to disc 1's `SLUS_006.62`** — same size,
CRC-32, MD5, SHA-1, and PS-X EXE header (see
`docs/reverse_engineering_notes.md` for the entry):

| Field | Value |
| --- | --- |
| Size | 2,025,472 bytes |
| CRC-32 | `7c10c01c` |
| MD5 | `cb095240a2ba358b8fdcbfd4d4f97f04` |
| SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` |
| pc0 / gp0 / t_addr / t_size / s_addr | identical to disc 1 (`0x80072534` / `0x00000000` / `0x80010000` / `0x1EE000` / `0x801FFFF0`) |

### Filesystem (31 files, from `tools/extract/psxiso.py list`)

```text
    LBA       size  path
     23         60  SYSTEM.CNF;1
     24    2025472  SLUS_006.68;1
   1013  206213120  PE.IMG;1
 101705         40  FMV2/PEDISC02.IDF;1
 101706   15826944  FMV2/FMV018.STR;1
 109434    7487488  FMV2/FMV019.STR;1
 113090   11763712  FMV2/FMV020A.STR;1
 118834   23068672  FMV2/FMV020B.STR;1
 130098    4194304  FMV2/FMV021.STR;1
 132146    8306688  FMV2/FMV023A.STR;1
 136202   11370496  FMV2/FMV023B.STR;1
 141754    1392640  FMV2/FMV024A.STR;1
 142434   12320768  FMV2/FMV024B1.STR;1
 148450    1064960  FMV2/FMV024B3.STR;1
 148970    2703360  FMV2/FMV024C.STR;1
 150290    1622016  FMV2/FMV024C2.STR;1
 151082    3424256  FMV2/FMV024E.STR;1
 152754   13877248  FMV2/FMV024F.STR;1
 159530    7684096  FMV2/FMV025.STR;1
 163282    6488064  FMV2/FMV026.STR;1
 166450   10649600  FMV2/FMV027.STR;1
 171650    2523136  FMV2/FMV028.STR;1
 172882    9224192  FMV2/FMV029.STR;1
 177386    6209536  FMV2/FMV030.STR;1
 180418    9732096  FMV2/FMV031.STR;1
 185170    4816896  FMV2/FMV032.STR;1
 187522    5472256  FMV2/FMV033A.STR;1
 190194   10043392  FMV2/FMV033B.STR;1
 195098    4491264  FMV2/FMV034.STR;1
 197291   14827520  FMV2/FMV035.STR;1
 204531  144080896  XASTREAM/CREDITS.XA;1
```

Structural observations (verified from the listings): disc 2 mirrors disc
1's layout — boot files at the same LBAs, `PE.IMG` with the same size
(206,213,120 bytes) at the same LBA (1013), then disc-specific FMV streams
plus a 144 MB `XASTREAM/CREDITS.XA`.

**`PE.IMG` is byte-identical across both discs (verified 2026-07-04):**
extracted from each image with `tools/extract/psxiso.py extract <bin> PE.IMG`
and hashed with `tools/verify/hashfile.py` — both copies are 206,213,120
bytes, CRC-32 `678da5c6`, MD5 `e8e714191694ce3516c56c1f0b3999a9`, SHA-1
`146c0ce7308bf9fdc2ba5a84230e198db0663f3b`. Combined with the byte-identical
boot EXEs, the two discs differ only in FMV/XA streams and volume metadata.

## Other releases (out of scope for now)

- Japan: SLPS-01291 (disc 1), SLPS-01292 (disc 2); Squaresoft Millennium
  Collection and PSone Books reprints exist.
- No PAL release.

## Rules

- Never commit images or extracted files; only record facts (hashes,
  offsets, names) here.
- Every value in this file must state how it was obtained (tool + command).
