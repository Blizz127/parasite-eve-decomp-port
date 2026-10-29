# Park native directed differentials

Run from the repository root after generating the native overlay TUs:

```sh
nice -n 19 python3 tools/analysis/park_native/run_park_diff.py
nice -n 19 python3 tools/analysis/park_native/run_park_diff.py --only 80192CD4 --output-dir build/park-evidence-mesh
```

The runner uses the existing host C compiler (`--cc` overrides its selection),
Python 3.10 or later, and the generator's existing `pycparser` dependency. If
that dependency is already installed outside the active Python environment,
point `PYTHONPATH` at its containing directory before running; this does not
install anything:

```sh
export PYTHONPATH="/existing/path/containing/pycparser${PYTHONPATH:+:$PYTHONPATH}"
nice -n 19 python3 tools/analysis/park_native/run_park_diff.py --output-dir build/park-evidence
```

Use the project's assigned CPU and memory limits. Functions, compiler calls,
and cases run sequentially. The runner compiles temporary oracle harnesses;
it does not build or launch the game. Retail room inputs must already be
available through `gen_overlay_ports.blob_path`, normally under
`build/extracted`. No retail bytes are stored in these providers.

The default target set is exactly these seven new native functions:

| Overlay | Functions | Provider |
| --- | --- | --- |
| `room_m0086i` | `8018F004`, `8018F734`, `801900CC` | `park_inputs_m0086.py` |
| `room_m0089i` | `801924F8`, `80192CD4`, `80193200` | `park_inputs_m0089.py` |
| `room_m0089i` | `80193618` | `park_inputs_m0089_effect.py` |

The runner checks membership in `configs/USA/overlays/nonmatching.yaml` and
discovers each generated filename by its overlay/function provenance. It does
not read `build/park-evidence/generated.txt`. Missing or duplicate generated
candidates are errors. `80190574` belongs to the separate composed fixture
suite and is deliberately excluded; that suite is unchanged.

Evidence defaults to `build/park-evidence`. Relative `--output-dir` paths are
resolved from the repository root. Each function produces a coverage JSON;
`directed-all.json` (or `directed-<selector>.json`) records the completed run.
Records include generated-source, provider, and oracle SHA256 values. The
runner fails on a mismatch, missing input, compilation error, or any skipped
fixture: every attempted fixture must be accepted. It stops at the first
failure and retains the results produced so far.

Coverage includes command/state branches, timer boundaries and negative
values, guest-pointer aliases, texture/alternate-bank paths, and zero-point
trail combinations. Providers document the detailed branch sets. The oracle
compares return values, guest RAM, scratchpad, ordered calls, initialized
callee-visible stack data, and final GTE registers. Callees remain deterministic
mocks: this is caller-level semantic evidence, not full-callee rendering,
room reachability, gameplay, audio, timing, or release-device acceptance.
