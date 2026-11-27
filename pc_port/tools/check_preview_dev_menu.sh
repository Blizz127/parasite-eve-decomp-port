#!/usr/bin/env bash
# Regression check for a packaged preview tarball: the launcher's dev-menu default
# (Back+Start / F8 menu) must be the one the build intends, build-info.json must
# declare it, and the PE_DEV_MENU=0 / PE_DEV_MENU=1 environment override must win.
#   pc_port/tools/check_preview_dev_menu.sh <preview.tar.gz> <expected 0|1>
set -euo pipefail
TARBALL="${1:?tarball}"; WANT="${2:?expected 0|1}"
[[ "$WANT" == 0 || "$WANT" == 1 ]] || { echo "FAIL: expected must be 0 or 1" >&2; exit 2; }
T="$(mktemp -d)"; trap 'rm -rf -- "$T"' EXIT
tar -xzf "$TARBALL" -C "$T"
D="$(tar -tzf "$TARBALL" | awk -F/ 'NR==1{print $1}')"   # parasite-eve-day1-preview, or parasite-eve-<tag> for tagged packages
L="$T/$D/parasite-eve-preview"; B="$T/$D/build-info.json"
fail() { echo "FAIL: $*" >&2; exit 1; }
[[ -f "$L" && -f "$B" ]] || fail "launcher or build-info missing in $TARBALL"
line="$(grep -E '^export PE_DEV_MENU=' "$L" || true)"
[[ "$line" == "export PE_DEV_MENU=\"\${PE_DEV_MENU:-$WANT}\"" ]] || fail "launcher default line is '$line', wanted default $WANT"
! grep -rqF "__DEV_MENU" "$T" || fail "unreplaced __DEV_MENU placeholder"
python3 - "$B" "$WANT" <<'PY'
import json, sys
info = json.load(open(sys.argv[1]))
assert info.get("dev_menu_default") == int(sys.argv[2]), ("build-info dev_menu_default", info.get("dev_menu_default"))
PY
# Evaluate the exact export line as the launcher would: unset -> default; explicit 0/1 -> override wins.
got_unset="$(env -i bash -c "$line; printf %s \"\$PE_DEV_MENU\"")"
got_zero="$(env -i PE_DEV_MENU=0 bash -c "$line; printf %s \"\$PE_DEV_MENU\"")"
got_one="$(env -i PE_DEV_MENU=1 bash -c "$line; printf %s \"\$PE_DEV_MENU\"")"
[[ "$got_unset" == "$WANT" ]] || fail "unset env gives $got_unset, wanted $WANT"
[[ "$got_zero" == 0 ]] || fail "PE_DEV_MENU=0 override lost ($got_zero)"
[[ "$got_one" == 1 ]] || fail "PE_DEV_MENU=1 override lost ($got_one)"
echo "OK: $(basename "$TARBALL") dev-menu default $WANT (Back+Start / F8 $([[ $WANT == 1 ]] && echo enabled || echo disabled) unless PE_DEV_MENU overrides); overrides preserved"
