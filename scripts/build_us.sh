#!/usr/bin/env bash
# Disc 1 matching rebuild entry point.
#
# Span membership, sources, object paths, sizes, linker order, and verifier
# coverage are generated from configs/USA/disc1.yaml. Compiler-only exceptions
# live in configs/USA/disc1_build_profiles.json. Do not add per-leaf commands
# here; update the YAML and, only when needed, a build profile assignment.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec python3 "$ROOT/tools/build/disc1_build.py" "$@"
