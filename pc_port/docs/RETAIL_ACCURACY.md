# Retail-accuracy ledger (native port)

This file states exactly where the native port is proven retail-accurate,
what is scripted/adapted, and what blocks full end-to-end accuracy. Every
claim names its evidence. Updated 2026-09-02 (Ubuntu 26.04, GCC 15.2).

## Proven (disc-free, re-runnable)

1. **Translated-function fidelity.** Every translated function on the boot
   path ships with oracle tests that compare the port against retail
   disassembly/EXE bytes (OTC1/EV1/TOK1/DRW1 families, B-series leaves,
   RNG/LZCR/callback units). Full suite:
   `1028 run, 1010 passed, 17 skipped, 1 failed` — the single failure is
   `B54KY_192CE8`, which needs a local disc image
   (`missing local/pe_disc1.path`), not a code defect.
   Evidence: `./pc_port/build/pe-native-tests`.
2. **Run determinism.** Two consecutive
   `--headless --bootstrap-disc --max-frames 1` runs produce byte-identical
   screenshots (`cmp` clean, 230415-byte P6) and byte-identical traces.
   Determinism is a prerequisite for any retail comparison; it holds.
3. **Frame-1 path uses zero adaptations.** The supported boot run invokes 6
   `BOOTSTRAP_RET` providers, 0 `HOST_ADAPTED`, 0 unsupported.
   Evidence: stub summary footer of the frame-1 run.
4. **Strict frontier is mapped, not silent.** On the real disc,
   `--strict-stubs` aborts at exactly one place:
   `func_8007F0C8_completion_selector` called from `func_8007F0C8` —
   the completion selector after a genuine 27-sector CdlReadS queue
   issue (descriptors, sequence, and lane state all real). Anything
   past that is honest stop, never faked output. On the bootstrap
   fixture the frontier is `func_8007F72C` from `func_800698D4`.

## Fixture-scripted on the bootstrap path (translated underneath)

All six providers below are translated retail logic in
`platform/pe_libcd.c` (verified word-for-word against
`asm/disc1/6F684.s`, `71A68.s`, `71150.s`, `72ABC.s`). The
`--bootstrap-disc` fixture branch of `func_800698D4` scripts their
*call sequence* (fixed values, strict aborts at the first one) instead of
executing it — that is fixture behavior, not missing translation:

- `func_8007F72C` (CdReady — the strict-mode gate)
- `func_8007F778`, `func_80082314`, `func_80080C48`
- `DsSearchFile(PEDISC01.IDF)`, `DsSearchFile(PE.IMG)`

The real (non-fixture) mount path executes all six against a memory disc
and is proven by `698D4_real_disc_sequence` / `698D4_second_disc_bits` /
`698D4_no_disc` (`PE_TEST_FILTER=698D4`: 7 passed, 0 failed), including
`D_800B0DCD` bit exactness and the `CdPosToInt` round-trip on `D_800B0DD8`.
The suite's `no_bootstrap_stubs_for_translated` guard pins this: no
translated function may regress to a stub.

## Adapted (documented, bounded)

- **Scratchpad stack handoff** (`func_8001220C_port.c`): retail switches
  `$sp` to scratchpad top and back around the overlay call; the port calls
  natively with trace markers. Call-equivalent, recorded as `HOST_ADAPTED`
  on every run.
- **Disc-wait bound** (`PE_PORT_DISC_WAIT_LIMIT`): without any disc the
  retail mount loop would spin forever, so the port bounds it and exits
  loudly. With a disc (real or fixture) the loop is retail logic and the
  bound is unreachable.
- **E0 delivery pump** (`func_801924F8_port.c`, CDQ2d): retail populates
  streaming slots via DMA-completion interrupts during the E0 poll spin;
  the port has no async interrupts, so the tail fires the installed
  `func_8007C214` completion callback once per poll (the 7ED58
  synchronous-delivery precedent). E0 takes got_frame on the first poll,
  so cadence beyond that is unobservable; the give-up path behind it is
  byte-identical retail logic.
- **Skip-movie New Game** (`func_801909B4_port.c`, `--skip-movie`):
  after the overlay prefix, return selector 1 so `func_8006E9A0(1)`
  publishes `0xA80830C8` without running the 480-frame logo, `FMV001`,
  or the untranslated title menu. Opt-in only; default boot still
  stops at the movie decoder. Not a claim that those scenes ran.
  `func_8006E9A0`'s fade poll honors the host stop and, under
  `--skip-movie` only, forces `CFEE=1` after the first tick so a
  re-armed fade cannot trap the New-Game publish.
- **Pad source** (`PE_Port_SetPadSource`): windowed `port_main` installs
  `HostWindow_PadRaw` so the field tick writes active-low Sony bits to
  `D_800BE9A2` before `func_8003EB04`. Stands in for libpad/`StartPAD`;
  the mapper itself is still the retail digital cut.

## Blocked (needs inputs absent from a clean checkout)

- **Oracle expect-sides** (`tools/rng_oracle.py`, `tools/lzcr_oracle.py`,
  `tools/callback_oracle.py`) require the retail exe extracted from a Disc 1
  image. The port-side dumps (`--rng/lzcr/callback-oracle-dump`) run fine;
  the cross-check cannot run without the image.
- **Real-disc path** (`--disc-image`, FLD1/B54KY disc-gated tests)
  is unverified wherever no image is present — including this checkout.
- **Past-frontier execution** (frame 2+, field entry, theatre) needs the
  untranslated functions; that is decompilation work under the era/docker
  byte-exact gate, not port work.

## Definition of done for "retail accurate end to end"

1. Zero `BOOTSTRAP_RET` on the boot-to-theatre path (strict run clean).
2. Zero `HOST_ADAPTED` invoked on that path.
3. All three oracle cross-checks green against the retail exe.
4. Disc-gated suite green with a local image.
5. Framebuffer/trace equality against hardware or emulator capture
   (capture harness does not exist yet — this is the last missing piece
   even after 1–4; Phase 6E-PRS1 wired the host present path to copy the
   guest DISPENV VRAM window, but no capture comparison exists).
