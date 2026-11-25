#!/usr/bin/env bash
# Run the strict signature guard over explicit files and retain its complete log.
set -euo pipefail
if (( $# < 2 )); then
    echo "usage: $0 NEW_LOG FILE..." >&2
    exit 2
fi
log=$1; shift
if [[ -e "$log" ]]; then
    echo "guard log already exists: $log" >&2
    exit 2
fi
root=$(cd "$(dirname "$0")/.." && pwd)
tmp=$(mktemp "${log}.tmp.XXXXXX")
trap 'rm -f "$tmp"' EXIT
python3 "$root/tools/analysis/retail_data_guard.py" --no-disc --strict "$@" > "$tmp" 2>&1 || {
    cat "$tmp" >&2
    exit 1
}
if [[ ! -s "$tmp" ]] || ! grep -Fxq "[guard] OK: $# file(s), no retail byte runs" "$tmp"; then
    echo "guard did not emit a non-empty success record for $# file(s)" >&2
    cat "$tmp" >&2
    exit 1
fi
mv "$tmp" "$log"
trap - EXIT
