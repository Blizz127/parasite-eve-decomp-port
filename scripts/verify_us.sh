#!/usr/bin/env bash
# Disc 1 verification entry point. The verifier manifest is generated from
# configs/USA/disc1.yaml; this wrapper must never enumerate spans or leaves.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec python3 "$ROOT/tools/build/disc1_verify.py" "$@"
