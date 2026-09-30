# Phase 6E-B54A — `func_8006AD40` suffix audit

## Verdict

**B54A PREFIX BOUNDARY IDENTIFIED.**

The accepted B50 cut at retail `0x8006AE50` is no longer the first
unresolved dependency. It sits in the middle of a counted
`func_8006E1C0` loop whose callee is now fully translated. The
canonical post-B53I-D Disc 1 packet has **13** texture entries; only
entry 0 has been issued. The suffix’s first 12 calls are that same
translated helper.

Recommended B54B architecture:

**B. PREFIX TO SMALL RETAIL HELPER**

- Helper: `func_8006E1C0` (already translated, Phase 6E-B51)
- Call site: loop at `0x8006AE44` / `jal` `0x8006AE48`
- New honest cut: **`.L8006AE68` / `0x8006AE68`**
- Scope: finish the remaining 12 iterations, then stop
- Do not take the `D_80091648` walk, `func_8006E498`, the
  `func_8007506C` payload walk, the CD poll, or `func_800718D0`

B54A is read-only. Production source is unchanged. The global frontier
remains `func_8006AD40_prefix_cut` until B54B.

Starting B53I-D commit:

```text
f003e4aa5ecfd17c5b98aa466f8262f44c5d6d37
```

Parent:

```text
175b16a4a78b68ec7515deceb2fd6088b57c4356
```

## Exact cut and suffix range

| | Address | Words | Bytes |
| --- | --- | --- | --- |
| Function | `0x8006AD40` .. `0x8006B35C` | 391 | 1564 (`0x61C`) |
| Proven prefix | `0x8006AD40` .. `0x8006AE50` | 68 | 272 (`0x110`) |
| Suffix | `0x8006AE50` .. `0x8006B35C` | **323** | 1292 (`0x50C`) |
| File offset | `0x5B540` | | |
| Matching EXE SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` | | |

Every 391 literal words, including the 323-word suffix, were checked
against that executable. There are no indirect calls in the suffix. The
only return is `jr $ra` / `nop` at `0x8006B354` / `0x8006B358`.

The last two prefix words are:

```text
0x8006AE48  jal  func_8006E1C0     ; 0x0C01B870
0x8006AE4C   move a1, s4           ; delay
0x8006AE50  lw   v0, 0x28(s3)      ; SUFFIX START
```

## Runtime state at `0x8006AE50`

Captured after a real Disc 1 boot, non-strict so the named cut can
return, with guest RAM still live. Strict mode still fatals at
`func_8006AD40_prefix_cut` from `func_8006AD40`.

Host:

```text
stop_reason = unresolved-boundary
stop_epoch  = 1
main_iters  = 1
presents    = 1
vsyncs      = 0
drawsyncs   = 0
```

Logical MIPS locals reconstructed from the prefix and the live packet:

| Reg | Meaning | Value |
| --- | --- | --- |
| `s0` | current `0x14`-byte entry | `0x8012DF58` (entry 0) |
| `s1` | issued-entry index | `0` (increment is the second suffix word) |
| `s2` | last CD poll / mode | `1` |
| `s3` | metadata = base + `[base+4]` | `0x8012DF18` |
| `s4` | channel-1 buffer | `0x801229A0` |
| `s5` | `&D_800B0CD8` | `0x800B0CD8` |
| `s6` | LBA base `D_800B0DD8` | `0x000003F5` |
| `v0` | `func_8006E1C0` return | `0` |

Guest / subsystem:

```text
D_800B0CD8              0x41004003   bit0 set; CD busy 0x01004000 still set
D_800B0CD8+0x160        0x801229A0   channel-1 / packet base
D_800B0CD8+0x174        0x8012F1A0
D_800B0CD8+0x14C        0x800F34F8
D_800B0CD8+0x16C        0x8012A9A0
D_800B0CD8+0x180        0x8017F1A0
D_800B0CD8+0x188        0x801861A0
base+4                  0x0000B578
header [s3+0x28]        0x0340B5B8   count=13, offset=0xB5B8
entry0                  0x8012DF58
producer / consumer     1 / 1
work marker             1
DMA cb slot 2           0
D_800945E6              0
I_STAT / I_MASK         0 / 0x0009
registered mask         0x0009
source-3 slot           0x80074520
stored / physical DICR  0x00800000 / 0x00800000
DICR edge / flag26      0 / 0
DMA2                    idle; token 2; CHCR 0x00000201
                        MADR 0x8012B8B8 BCR 0x00020010
GPUSTAT                 0x04000000   GP0 idle, GP1 dir=2
VBlank                  0
```

Entry 0 is the accepted two-LoadImage pair:

| | packed | RECT |
| --- | --- | --- |
| image `+8` / `+7` | `0x080B0020` / `h=0x40` | `{704,64,32,64}` |
| CLUT `+10` / `+F` | `0x39040040` / `h=1` | `{256,456,64,1}` |

Real VRAM at the cut (not the synthetic B53I-D fixture pattern):

```text
(704,64)=0x0F0F   (735,127)=0x0F0F
(256,456)=0xFFFF  (319,456)=0x0000
```

These values are evidence. They are not hardcoded into production.

## Suffix control flow

```text
0x8006AE50  lw    header, 0x28(s3)
0x8006AE54  s1++
0x8006AE58  count = header >> 22
0x8006AE5C  if (s1 < count)
0x8006AE60    bne -> 0x8006AE44
0x8006AE64     s0 += 0x14                 ; delay
0x8006AE44  a0 = s0
0x8006AE48  jal func_8006E1C0
0x8006AE4C   a1 = s4
            ; falls into 0x8006AE50 again

0x8006AE68  .L8006AE68                    ; RECOMMENDED B54B CUT
            for (a1 = 0x20; a1 < 0x40; a1 += 0x10)
                pack D_80091648/4A/4C/4E -> 1650/1652
            jal func_8006E498(s4, 0xABADC06C)
            s0 = v0
            while ([s0] != 0)
                jal func_8007506C(s0+4, s0+0xC)
                s0 += [s0] & ~3
            s0 = 1
            if (s2 == -1) goto channel-2 issue (0x8006ADD8)
            jal func_8006E7E8             ; CD poll, do not invent result
            if (s2 != 0) goto 0x8006AE08  ; another packet pass
            ; next CD range D_800930EE / buf+0x180
            jal func_8006E6A8
            if (s0 == 0)
                jal func_800718D0(buf+0x174)   ; FIRST UNRESOLVED FUNCTION
            ... later channels, 6E1C0 again, then SDK shims ...
            D_800B0CD8 &= ~1
            jr ra
```

No suffix instruction touches `0x1F80xxxx`. GPUSTAT, DMA2, DICR, I_STAT,
I_MASK, VRAM, and VBlank are observed only inside already-classified
callees.

## Dependency map

| Callee | Sites | B50 class | B54A class | Notes |
| --- | --- | --- | --- | --- |
| `func_8006E1C0` | AE48, B1DC, B254 | UNRESOLVED | **translated, faithful** | B51; 68 words; issues 1–2 LoadImages |
| `func_8006E498` | AEFC, B118, B12C, B140 | TRANSLATED | **translated, faithful** | B16 key lookup |
| `func_8007506C` | AF18 | UNRESOLVED | **translated, faithful** | B52 LoadImage |
| `func_8006E7E8` | AF54, B04C, B0BC, B154, B20C | TRANSLATED | **translated, faithful** | B16 poll; result is live CD state |
| `func_8006E6A8` | AF88, B080, B0F0, B188 | TRANSLATED | **translated, faithful** | B16 issue |
| `func_800718D0` | AFA8, B0A4 | UNRESOLVED | **unresolved** | 29 words / `0x74`; only callee is `func_8007506C` |
| `func_80030894` | B0AC | UNRESOLVED | **unresolved** | `0xC50` bytes; no port |
| `func_80087024` | B274 | TRANSLATED | host/stream provider | `pe_stream.c` command `0xF1` |
| `func_80074DC0` | B27C | TRANSLATED | host SDK shim | DrawSync |
| `func_80074A44` | B284 | TRANSLATED | host SDK shim | ResetGraph |
| `func_80073A44` | B28C | TRANSLATED | host SDK shim | VSync |
| `func_800755F0` | B2B4 | TRANSLATED | host SDK shim | PutDispEnv |
| `func_80074D28` | B2BC | TRANSLATED | host SDK shim | SetDispMask |

B50-era blockers now resolved on the suffix’s first steps:

- `func_8006E1C0` at `0x8006AE48`
- `func_8007506C` at `0x8006AF18`
- GPU VRAM / GPUSTAT / DMA2 / DICR / I_STAT / I_MASK / callback /
  idle pump / two-token LoadImage — present, and already used by entry 0

Still unresolved, and **not** the next rung:

- `func_800718D0` (small, later)
- `func_80030894` (large, later)
- CD poll *result* after the remaining textures (translated helper, live
  drive state; `D_800B0CD8` still has `0x01004000`)
- Host shims at the function tail are existing class-2 surfaces, not a
  new hardware contract

## Canonical path from the captured state

1. Reload `header=0x0340B5B8`, `s1 = 1`, `1 < 13`.
2. `s0 += 0x14` → entry 1, `jal func_8006E1C0`.
3. Repeat for entries 2..12 (twelve translated calls).
4. `s1 == 13` → fall through to `0x8006AE68`.
   The delay `addiu s0, 0x14` also runs on the untaken back-edge, so
   `$s0` at the new cut is one entry past the last issued slot. It is
   not a live packet pointer; later retail overwrites it.
5. **Recommended stop.**

If B54B later continues past that label (not this rung):

6. Pack `D_80091648` slots `a1=0x20,0x30` into `1650/1652`.
7. `func_8006E498(0x801229A0, 0xABADC06C)` → `0x801229A8`
   (`[0x801229A8]=0x00007F0C ≠ 0`).
8. `func_8007506C` payload walk (translated; more LoadImages).
9. `func_8006E7E8` — **do not invent** a complete poll. Busy bits are
   still set at the current cut.
10. Only if a future capture shows poll `0`: next CD issue, then
    `func_800718D0(0x8012F1A0)` as the first unresolved function.

DMA idle vs busy does not change suffix-local branches. The next
`func_8006E1C0` uses the existing GPU/DMA2 authority. This audit does
not complete those transfers. `func_8001220C` still has only two
explicit completion checkpoints, which already fired for entry 0 in
this iteration. Further completions belong to later host opportunities,
not to invented suffix hardware.

## Hardware-authority audit

| Surface | Suffix-local? | Covered now? |
| --- | --- | --- |
| DMA2 busy/idle | no; inside 7506C / 76C34 / 76EE4 | yes (B53B/H/I) |
| GPUSTAT | no | yes |
| DICR | no | yes |
| I_STAT / I_MASK | no | yes (B53D/I) |
| callback slots | no | yes (B53G) |
| VRAM | no; written by completion of 6E1C0/7506C | yes |
| VBlank | no; later VSync shim only | host shim exists |
| CD busy/poll | only via 6E7E8 after the loop | translated; result not invented |

No new hardware model is required for B54B’s recommended loop.

## Recommended B54B split

Implement only the suffix fragment `0x8006AE50 .. 0x8006AE68`:

```text
header = [s3 + 0x28]
s1 += 1
while (s1 < (header >> 22)) {
    s0 += 0x14
    func_8006E1C0(s0, s4)     /* already translated */
    header = [s3 + 0x28]
    s1 += 1
}
/* stop at 0x8006AE68 */
```

Keep a named unresolved boundary there. Do not translate `func_800718D0`
or `func_80030894`. Do not add a third `func_8001220C` checkpoint. Do
not begin a periodic DMA scheduler.

## Preservation

B54A adds only:

- `pc_port/tools/b54a_6ad40_suffix_oracle.py`
- `pc_port/docs/b54a_func_8006AD40_suffix_audit.md`

No production C. GPU/IRQ/queue/stop-epoch behavior is untouched. The
live frontier remains `func_8006AD40_prefix_cut` at `0x8006AE50` until
B54B.

Independent contract: `pc_port/tools/b54a_6ad40_suffix_oracle.py`
(exact EXE SHA-1, 391 words, delay slots, captured count=13, no
production imports, no automatic DMA/CD evolution).

Measured after this audit (production binaries unchanged):

```text
native suite                         566/566
fresh ASan/UBSan                     566/566
focused D / C / B2 / B1 / H / B53B   8/8, 10/10, 15/15, 8/8, 8/8, 15/15
retained oracle matrix               46/46
B54A suffix oracle                   8/8
B49 normal / sanitizer               PASS / PASS
strict Disc 1 frontier               func_8006AD40_prefix_cut from func_8006AD40
bootstrap strict                     func_8007F72C from func_800698D4
framebuffer SHA-256                  fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
FNV-1a64                             7D860391E1ED6C97
matching EXE SHA-1                   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The B50 oracle's bundled word list is not used as authority here. At least
one prefix-adjacent encoding (`0x8006AEA4`) differs from the executable
(`0x00641825` vs an older `0x00621825`). B54A regenerated all 391 words
from the SHA-exact EXE.
