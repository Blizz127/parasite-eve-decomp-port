#!/usr/bin/env bash
# Disassemble a retail function by VRAM.  usage: pe_dis.sh <vram_hex> <size_hex>
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export PATH="$ROOT/tools/mipsel-host/bin:$PATH"
V=$1
SIZE=$2
FOFF=$((V - 0x80010000 + 0x800))
ADJ=$((V - FOFF))
mipsel-linux-gnu-objdump -D -b binary -m mips:isa32 -EL \
  --adjust-vma=$ADJ --start-address=$V --stop-address=$((V + SIZE)) \
  "$ROOT/build/extracted/disc1/SLUS_006.62" 2>/dev/null | \
  sed -n "/^$(printf '%08x' $V)/,\$p"
