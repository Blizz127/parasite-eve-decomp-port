# Phase 6E-B49 — host-loop continuation

## Provenance and pre-edit reproduction

The collision guard ran on branch `phase6e-b-provider-frontier` at required
HEAD `961f19a60a55dd8485ea4f645b637248b35096a9`; `HEAD^` was
`0079a50aefc1d41a809e09019b819e140ccc27e3`. Two status reads were clean.
No process had a cwd inside this checkout, no Git lock or untracked artifact
was present, and no remote branch contained the required HEAD.

The pre-edit real-Disc-1 `--strict-stubs` run was observed under GDB rather
than inferred from source:

1. native `main` entered with stop=0, iterations=0;
2. `func_8001220C` entered;
3. `HostFB_Present` entered from `func_8003E610` -> `func_8003E754` ->
   `func_800755F0`, with stop=0 and presents=0;
4. a hardware watchpoint observed `g_port_stop_requested` change 0 -> 1 at
   the old `host_framebuffer.c:58` assignment;
5. the continuation test at the old `func_8001220C_port.c:115` observed
   stop=1 and iterations=1;
6. `func_8001220C` returned to native `main`;
7. trace events recorded `shutdown_begin` and `shutdown_end`;
8. the process exited 0 with presents=1, iterations=1.

The screenshot SHA-256 was the canonical
`fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb`.

## Retail continuation contract

The executable body in `asm/disc1/2A0C.s` proves that `func_8001220C` is the
long-lived retail loop, with a nested game-state loop:

- startup `func_80072534` calls it once at `0x800725D0` and executes a
  `break 0,1` if it ever returns;
- the outer loop begins at `0x8001224C` and jumps back from `0x800124CC`;
- the nested state loop begins at `0x80012294` and loops from
  `0x800124A8` while bit `0x100` is clear;
- the retail presentation routine returns normally to its callers; no
  presentation count or presentation result controls either loop;
- the outer transition is guest state bit `0x100`, not framebuffer
  presentation.

Therefore presenting a frame is not retail termination. The Phase 6C host
assignment was a host-only divergence.

## Stop-flag and presentation audit

Before B49, the complete `g_port_stop_requested` inventory was:

| Location | Operation | Actual purpose |
|---|---|---|
| `bootstrap/func_8001220C_port.c:44` | definition/reset to 0 | process-start host state |
| `platform/host_framebuffer.c:57-58` | declaration and write to 1 | one-frame bootstrap shortcut |
| `bootstrap/func_8001220C_port.c:99` | read/test | abandon disc wait on host stop |
| `bootstrap/func_8001220C_port.c:115` | read/test | abandon nested state loop on host stop |
| `bootstrap/game_port.h:6` | declaration | shared host state |
| `tests/test_native.c:11777` | test-only definition/reset | satisfy the test link |

There was no window-close writer, test-specific writer, fatal-error writer, or
normal-retail-termination writer. Strict providers and fatal guest-memory
errors terminate independently through the centralized fatal paths.

History identifies the presentation write as classification **A: obsolete
one-frame bootstrap shortcut**. `HostFB_Present` originally only incremented
the presentation count in commit `9c9561b`; commit `4ca5b18` added the global
stop write while routing the new translated main to the first-clear
milestone. The same commit parsed `--max-main-iterations` and
`--stop-after-event` but implemented neither. The older README mentioned
`--max-frames`, but no production parser or policy existed.

After B49, `HostFB_Present` accounts for an admitted host presentation and
reports that event to run control. It never writes the global stop flag. Run
control owns all resets/writes/tests, and the translated main calls the policy
only at its existing host-safe continuation checks.

## Explicit termination policy

- Interactive/default: no artificial frame or iteration limit. Continue
  until window close/Escape, an explicit host request, retail termination, or
  a fatal provider/memory error.
- `--max-frames N`: admit exactly N host framebuffer presentations. Reaching
  N marks the budget pending; further presentations are suppressed until the
  next safe continuation check converts it to a clean frame-limit stop. N=1
  reproduces the historical framebuffer without making presentation an
  unconditional exit side effect.
- `--max-main-iterations N`: allow exactly N starts of the outer retail loop.
  It is useful for outer-loop tests but is not a substitute for a frame budget
  while execution remains inside the nested state loop.
- Windowed runs register `HostWindow_Poll` with run control. Close/Escape is
  converted to `host-quit` at a safe continuation check.
- Strict unresolved providers and fatal guest-memory/provider errors retain
  their immediate fatal exits.

Both numeric budgets reject zero, negative, malformed, and overflowing
values. `PE_Port_RunControlReset` clears stop state, counters, budgets, pending
frame state, quit callback, and stop reason between runs.

## Continuing execution evidence

The production harness is `tools/b49_host_loop_test.py`.

| Run | Exit | Frames | Outer iterations | Framebuffer SHA-256 | Boundary/evidence |
|---|---:|---:|---:|---|---|
| real Disc 1, strict, max-frames=1 | 0 | 1 | 1 | `fb28dc21...dfdb` | historical bounded result |
| repeat of previous run | 0 | 1 | 1 | `fb28dc21...dfdb` | identical trace and screenshot |
| real Disc 1, non-strict, max-frames=2 | 0 | 2 | 1 | `fb28dc21...dfdb` | invokes `func_8006AD40`, then naturally reaches `func_801909B4` |
| real Disc 1, strict, max-frames=2 | 1 | 1 | 1 | not written (fatal) | first boundary `func_8006AD40` from `func_8001220C` |
| bootstrap-disc, strict, max-frames=2 | 1 | 1 | 1 | not written (fatal) | remains `func_8007F72C` from `func_800698D4` |

GDB recorded the new strict boundary with stop=0, frames=1, and outer
iterations=1. Its retail call is `jal func_8006AD40` at `0x800122C4` with a
`nop` delay slot; it is void and has no consumed return.

## `func_801909B4` read-only record

Strict mode does **not** reach `func_801909B4` in B49 because the earlier
`func_8006AD40` boundary is execution's source of truth. A bounded non-strict
run does reach it naturally after the old presentation stop is removed.

Its sole main-loop call is `jal func_801909B4` at `0x800123B4`, caller
`func_8001220C`, with no arguments and a `nop` delay slot. The returned `v0`
is consumed as `a0` in the delay slot of the immediately following
`jal func_8006E9A0` at `0x800123BC`. Its current centralized behavior remains
`Bootstrap_ReturnInt("func_801909B4", "func_8001220C", 0)`. B49 does not
translate it or alter/hardcode that default.

The later two-frame framebuffer remains near-black and has the same hash as
the historical one-frame capture. No rendering or X11 Expose behavior was
changed.

## Verification gates

- normal suite: 451/451;
- fresh ASan/UBSan suite: 451/451;
- normal and sanitizer B49 production harness: pass, including clean bounded
  shutdown and repeated deterministic one-frame runs;
- all 33 retained oracle programs: pass;
- fresh matching build in the documented MIPS toolchain: exact SHA-1
  `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`;
- `git diff --check`: pass.

## B52 retained-harness rerun

B52 does not weaken the B49 frame-budget contract. The retained harness was
rerun against normal and fresh ASan/UBSan builds with identical results:

- deterministic repeated one-frame runs exit cleanly at one frame and one
  main iteration;
- the historical framebuffer remains
  `fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb`;
- the non-strict prefix run stops at the host-safe check with reason
  `unresolved-boundary` before `func_801909B4`;
- continuing strict execution exits 1 at `func_80076C34` from
  `func_8007506C` in both builds;
- bootstrap strict remains `func_8007F72C` from `func_800698D4`.

The current native and fresh sanitizer suites are 468/468. All 33 retained
standalone oracles, the corrected B50 oracle, the B51 oracle, and the new B52
oracle pass.
