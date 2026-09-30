# Disc 1 build authority

`configs/USA/disc1.yaml` is the sole authority for Disc 1 span membership,
order, boundaries, source paths, object paths, linker order, trim sizes, and
verification coverage. Build and verifier entry points must not enumerate
individual leaves.

Compiler behavior that cannot be inferred from a span lives in
`configs/USA/disc1_build_profiles.json`. Its default is era `-O2 -G0`; named
profiles describe only real exceptions such as the modern compiler, `-G8`, or
an opt-in maspsx gate. An override naming a symbol absent from YAML is a hard
error.

`tools/build/disc1_plan.py` validates strict contiguous geometry and emits,
under ignored `build/generated/`:

- `disc1_plan.json` — the complete build plan;
- `disc1_verify_manifest.json` — one entry for every YAML C span;
- `disc1_romorder.ld` — YAML-order primary sections and deterministic
  secondary-section passes.

It also owns the committed
[`generated matching status`](generated/DISC1_MATCHING_STATUS.md). Refresh
that file after a YAML carve with:

```sh
python3 tools/build/disc1_plan.py --write-status
```

No build list or verifier list changes are permitted for a leaf. A normal
carve changes the YAML and adds its semantic C source; a profile assignment is
added only when the default compiler behavior is proven wrong.

Tracked `src/func_*.c` files outside YAML are never inferred to be matching.
They must have an exact entry in
`configs/USA/disc1_nonmatching_sources.json`, with one of the bounded
nonmatching dispositions and an existing evidence path. The public verifier
requires a bijection between that manifest and the extra tracked sources,
cross-checks accepted residuals against their policy inventory, and rejects
file-scope assembly in either population.

## Gates

The artifact-independent gate is safe for public CI and needs no retail data:

```sh
scripts/verify_us.sh --public
python3 tools/build/test_disc1_plan.py
```

The exact gate requires the legally supplied retail executable and generated
split, then builds and verifies every packed C span:

```sh
scripts/split_us.sh
docker run --rm -v "$PWD:/workspace" -w /workspace \
  --user "$(id -u):$(id -g)" pe-mipsel-img:latest bash scripts/build_us.sh
scripts/verify_us.sh
```

The self-hosted CI workflow performs this from a clean checkout without an
actions or Docker cache. Its private runner label is `parasite-eve-retail`; the
runner service must expose a legal executable through `PE_DISC1_EXE_PATH` and
the legal Disc 1 image through `PE_DISC1_BIN_PATH`, and provide Docker, curl,
Git, CMake, Ninja, and Python venv support. The private workflow requires the
full normal and ASan/UBSan native suites with no skips. The public workflow
checks authority, geometry, profiles, generated status, the
source/build/verify bijection, and the artifact-independent native test set.

## Evidence and prioritization

Repeated forwarding-wrapper families may use one family report with a
generated per-span matrix. They do not require one report and one bookkeeping
edit per member; every member still needs its own function-hood and packed-span
evidence.

`tools/build/wrapper_matrix.py` enforces that contract. Its JSON input contains
only the family name, each symbol's function-hood proof, and the
relocation-normalized single-leaf object words. The tool derives boundaries,
word counts, flags, retail words, and packed words from the plan and artifacts;
it refuses the entire batch on the first unequal word. Example input shape:

```json
{
  "schema_version": 1,
  "family": "forwarding callbacks",
  "members": [
    {
      "name": "func_80000000",
      "function_hood": "exact-start callback-table entry at ...",
      "normalized_object_words": ["0x00000000"]
    }
  ]
}
```

Matching count is bookkeeping, not product progress. Candidate ordering should
prefer functions on the active native/port frontier, then reachable callees,
semantic fan-out, and state ownership. Raw word count is only a final
tie-breaker. Native progress is reported separately as production frontier,
implemented retail words, reachable systems, and normal/sanitized test gates.

Native tests that read the legally supplied Disc 1 image are explicitly tagged
`TEST_RETAIL_DISC1` in the test source. Public CI requires every untagged test
to pass and exactly the tagged set to skip; a configured but invalid artifact
still fails. Private full mode permits no skips and requires every test to
pass. The generated [native status](generated/NATIVE_PORT_STATUS.md) derives
both populations from those source-level tags.

`tools/progress/port_priority.py` turns the classified Tier-2 pool into a
generated [native-prioritized queue](generated/NATIVE_CANDIDATE_PRIORITY.md).
It validates the declared frontier against the source/evidence-derived native
metrics, closes the frontier's direct callees, derives a static call graph from
the CMake-linked native sources, and sorts by port relevance before retail
fan-in. Word count is the final tie-breaker. The tool labels static
reachability as a scheduling signal; it does not promote it into runtime
reachability evidence.
