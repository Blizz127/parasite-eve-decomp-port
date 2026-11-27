#!/usr/bin/env bash
# usage: pe_cdharness_day6.sh TAG [driver args...]   (Day-6 leg to the credits/ending through the wrapped binary; ASSISTED-DIAGNOSTIC)
# Env: DISC DISC2 (images), CDX_WORK, BIN, PERSIST (persist block for the warp), plus KEY=VALUE via CDH_ENV (space separated, passed with --env).
set -u
ROOT=${ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}
export DISC=${DISC:?} DISC2=${DISC2:?} PE_DISC1_BIN="$DISC" PYTHONDONTWRITEBYTECODE=1
W=${CDX_WORK:?}; B=${BIN:-$ROOT/build/cap/pe-cdh}; tag=$1; shift
O=$W/run-$tag; rm -rf "$O"; mkdir -p "$O/shots" "$O/tmp"; export TMPDIR=$O/tmp
export PE_CDX_CAPTURE=$O/capture.cap
extra=(); for kv in ${CDH_ENV:-}; do extra+=(--env "$kv"); done
echo "start $(date -u +%FT%TZ) pressure: $(grep full /proc/pressure/memory)" > "$O/times.txt"
cd "$ROOT/pc_port/tools"
python3 day6_route_regress.py --bin "$B" --log-dir "$O" --name "$tag" --env PE_ROUTE_MAXHP=999 "${extra[@]}" "$@" --persist "${PERSIST:?}" > "$O/driver.out" 2>&1
rc=$?; echo "end $(date -u +%FT%TZ) rc=$rc" >> "$O/times.txt"; echo "$tag rc=$rc"; tail -3 "$O/driver.out" | cut -c1-300
