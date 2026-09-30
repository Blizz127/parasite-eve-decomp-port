# PE-GPU1 — GPU packet-header leaves ported (native)

Base: PE-B54G HEAD. Task: port the five matching-C GPU-header leaves that
block `func_80030894` (788 words, boot GPU-primitive builder), prove each
byte-exact against the matching build, and add a focused oracle. Also classify
the four getters (`GetTPage` / `GetClut` / `SetSemiTrans` / `SetShadeTex`)
listed unresolved by B54D at the same call sites.

## Classification rule

Every one of the nine is a **real outlined ROM function**, not a pure inline
expansion folded into the caller:

- Each has its own standalone code block ending in `jr $ra`
  (`0x03E00008`).
- `func_80030894` reaches each via a distinct `jal` site (verified by scanning
  the 788-word body for `jal` encodings).

Psy-Q `libgpu.h` defines `setPolyF3` / `setPolyFT4` / `setPolyG4` / `setTile`
/ `setSprt` / `GetTPage` / `GetClut` / `SetSemiTrans` / `SetShadeTex` as
header inline macros/functions; the retail compiler outlined each into a
separate ROM function. The matching decomp already has C leaves for the five
SET functions (`src/func_80077B{64,BA4,BC4,C44,C64}.c`); the four getters have
no matching C leaf yet (they remain part of the unresolved `func_80030894`
blob) and are classified only.

## Ported leaves (native)

File: `pc_port/game/boot/func_80077B{64,BA4,BC4,C44,C64}_port.c`.
Signature `void func_80077Bxx(pe_addr_t p)`; `p` is the guest address of the
primitive packet. Each writes exactly two bytes via `PE_StoreU8` and nothing
else (no reads, no other stores, no callees, no hardware). Order preserved:
offset 3 then offset 7. Idempotent.

| symbol | Psy-Q | ROM @ file | byte[3] | byte[7] |
| --- | --- | --- | --- | --- |
| func_80077B64 | SetPolyF3  | 0x80077B64 / 0x68364 | 4  | 32 (0x20) |
| func_80077BA4 | SetPolyFT4 | 0x80077BA4 / 0x683A4 | 9  | 44 (0x2C) |
| func_80077BC4 | SetPolyG4  | 0x80077BC4 / 0x683C4 | 8  | 56 (0x38) |
| func_80077C44 | SetTile    | 0x80077C44 / 0x68444 | 3  | 96 (0x60) |
| func_80077C64 | SetSprt    | 0x80077C64 / 0x68464 | 3  | 64 (0x40) |

ROM pattern (e.g. 0x80077B64), decoded from `build/disc1.candidate.exe`:

```
0x80077B64: 0x24020004   addiu $v0, $zero, 4
0x80077B68: 0xA0820003   sb    $v0, 3($a0)
0x80077B6C: 0x24020020   addiu $v0, $zero, 0x20
0x80077B70: 0x03E00008   jr    $ra
0x80077B74: 0xA0820007   sb    $v0, 7($a0)
```

This matches the matching C leaf exactly; the candidate EXE is the exact
SHA-1 (`452fb033…`), so the native two-byte stores are byte-exact against the
matching build.

## Classified getters (not native-ported)

`func_80077A64` GetTPage, `func_80077AA4` GetClut, `func_80077B04`
SetSemiTrans, `func_80077B34` SetShadeTex — all real outlined functions
(terminate in `jr $ra`; distinct `jal` sites in `func_80030894`). No native
implementation added.

## Verification

- `pc_port/tools/pe_gpu1_header_leaves_oracle.py`: loads the SHA-1-exact
  `build/disc1.candidate.exe`, decodes each ROM word-block, asserts the five
  SET leaves' exact (offset3, offset7) store contract, classifies the four
  getters as real functions, and cross-checks all nine as `jal` callees of
  `func_80030894`. 18 checks pass.
- `test_PEGPU1_header_leaves_byte_exact` (pc_port/tests/test_native.c): calls
  each native leaf on a scratch guest buffer, asserts the identical two bytes,
  asserts no other header bytes are touched, asserts idempotence on repeat,
  and asserts `func_80030894` is not entered. Full native suite: 579/579.
- `func_80030894` is NOT entered; the `func_8006AD40` frontier is NOT advanced.

## Gates

- Matching EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` unchanged (no
  `src/` edit; candidate EXE untouched).
- Native build (`pe-native-tests`) 579/579; new focused test PASS.
