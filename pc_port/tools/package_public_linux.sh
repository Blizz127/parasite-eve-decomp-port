#!/usr/bin/env bash
# Package the public Linux x64 release archive: parasite-eve-<tag>-linux-x64.tar.gz (extracting to parasite-eve-<tag>/)
# Layout mirrors the public native-only-e26d98fd archive plus the LICENSES set of the
# private preview packages.  No game data, BIOS or SDK files.  Usage:
#   package_public_linux.sh <gated-binary> <tag> <source-commit> <outdir> <source-note>
set -euo pipefail
BIN=$1; TAG=$2; SRC=$3; OUT=$4; NOTE=${5:-}
W=${PE_PKG_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)}   # source tree holding pc_port/third_party and vendor/
# Reference public launcher (the e26d98fd archive's parasite-eve-preview; its sha256 is checked below).
REF=${PE_PUBLIC_WRAPPER_REF:?set PE_PUBLIC_WRAPPER_REF to the reference launcher wrapper (the parasite-eve-preview script of the previous public archive)}
# Folder and archive are named from the tag (parasite-eve-<tag>); the asset keeps the launcher regex parasite-eve-*-linux-x64*.tar.gz.
NAME=parasite-eve-$TAG; TAR=$NAME-linux-x64.tar.gz
S=$OUT/stage/$NAME; rm -rf "$OUT/stage"; mkdir -p "$S/bin" "$S/LICENSES" "$OUT"
install -m 0755 "$BIN" "$S/bin/parasite-eve-port"
install -m 0644 "$W/pc_port/third_party/SDL_GameControllerDB/gamecontrollerdb.txt" "$S/bin/gamecontrollerdb.txt"
install -m 0644 "$W/pc_port/third_party/SDL_GameControllerDB/LICENSE" "$S/bin/gamecontrollerdb-LICENSE.txt"
install -m 0755 "$REF/parasite-eve-preview" "$S/parasite-eve-preview"
[ "$(sha256sum "$S/parasite-eve-preview" | cut -d' ' -f1)" = f8df2bae8f75ae63c5f55fea37d9b6e9f77e6246a371971eea5b01de33949083 ] || { echo "wrapper differs from the public e26d98fd wrapper"; exit 1; }
# owner/checkpoint packages: dev menu default on (PE_DEV_MENU=1); public packages keep 0
DEV_MENU=${DEV_MENU:-0}
if [ "$DEV_MENU" = 1 ]; then sed -i 's/export PE_DEV_MENU="${PE_DEV_MENU:-0}"/export PE_DEV_MENU="${PE_DEV_MENU:-1}"/' "$S/parasite-eve-preview"; grep -qF 'export PE_DEV_MENU="${PE_DEV_MENU:-1}"' "$S/parasite-eve-preview" || { echo "dev-menu default patch failed"; exit 1; }; fi
install -m 0644 "$W/pc_port/third_party/FidelityFX-FSR-LICENSE.txt" "$S/LICENSES/FidelityFX-FSR-LICENSE.txt"
V=$W/vendor/khasinski-parasite-eve-decomp
install -m 0644 "$V/LICENSE.md" "$S/LICENSES/khasinski-LICENSE.md"
install -m 0644 "$V/THIRD_PARTY_NOTICES.md" "$S/LICENSES/khasinski-THIRD_PARTY_NOTICES.md"
install -m 0644 "$V/PROVENANCE.md" "$S/LICENSES/khasinski-PROVENANCE.md"
# vendor-derived function list: every src file tagged vendor-derived, with linked yes/no from nm
nm --defined-only "$BIN" | awk '{print $3}' | LC_ALL=C sort -u > "$OUT/nm-defined.txt"
( cd "$W" && grep -rl -m1 '^/\* vendor-derived:' src --include=*.c | LC_ALL=C sort ) > "$OUT/vendor-files.txt"
python3 -I - "$OUT/vendor-files.txt" "$OUT/nm-defined.txt" "$W" > "$S/LICENSES/vendor-derived-functions.txt" <<'PY'
import sys, re, os
files, nmf, w = sys.argv[1:]
syms = set(open(nmf).read().split())
tails = set(x.split('__')[-1] for x in syms if '__' in x)
print("# linked\tfunction\tsource file\tupstream (khasinski/parasite-eve-decomp) — see khasinski-PROVENANCE.md / khasinski-LICENSE.md")
n = 0
for rel in open(files).read().split():
    head = open(os.path.join(w, rel), errors='replace').readline().strip()
    m = re.search(r'vendor-derived:\s*(\S+)\s+(\S+)', head)
    up = (m.group(1) + ' ' + m.group(2)) if m else 'khasinski/parasite-eve-decomp'
    fn = os.path.basename(rel)[:-2]
    print(f"{'yes' if (fn in syms or fn in tails) else 'no'}\t{fn}\t{rel}\t{up}"); n += 1
print(f"# {n} vendor-derived source files", file=sys.stderr)
PY
chmod 0644 "$S/LICENSES/vendor-derived-functions.txt"
python3 -I "$W/pc_port/tools/render_preview_readme.py" --template "$W/pc_port/package_readme_preview.txt" --out "$S/README-PREVIEW.txt" --tarball "$TAR" --dir "$NAME" --dev-menu "$DEV_MENU" --default-skip 0 --tag "$TAG" --commit "$SRC" --note "$NOTE"; chmod 0644 "$S/README-PREVIEW.txt"
python3 -I - "$S/build-info.json" "$(sha256sum "$BIN" | cut -d' ' -f1)" "$(sha256sum "$S/parasite-eve-preview" | cut -d' ' -f1)" "$TAG" "$SRC" "$BIN" "$NOTE" <<'PY'
import json, os, sys, datetime
dst, sha, lsha, tag, src, binp, note = sys.argv[1:]
t = datetime.datetime.fromtimestamp(os.stat(binp).st_mtime, datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
json.dump({"schema": 1, "name": "parasite-eve-" + tag, "platform": "linux-x64", "build_id": tag, "source_commit": src,
           "source_note": note, "build_time_utc": t, "binary": {"path": "bin/parasite-eve-port", "sha256": sha},
           "launcher": {"path": "parasite-eve-preview", "sha256": lsha}, "dev_menu_default": int(os.environ.get("DEV_MENU", "0")), "contains_game_data": False}, open(dst, "w"), indent=2); open(dst, "a").write("\n")
PY
chmod 0644 "$S/build-info.json"
(cd "$S" && find . -type f ! -name MANIFEST.txt | LC_ALL=C sort | while read -r f; do printf '%s  %s  %s\n' "$(sha256sum "$f" | cut -d' ' -f1)" "$(stat -c %s "$f")" "${f#./}"; done) > "$S/MANIFEST.txt"; chmod 0644 "$S/MANIFEST.txt"
find "$OUT/stage" -type d -exec chmod 0755 {} +; chmod 0755 "$S/bin/parasite-eve-port" "$S/parasite-eve-preview"
tar --owner=0 --group=0 --numeric-owner --sort=name -C "$OUT/stage" -czf "$OUT/$TAR" "$NAME"
(cd "$OUT" && sha256sum "$TAR" > "SHA256SUMS.linux" && cat SHA256SUMS.linux)
tar -tzvf "$OUT/$TAR" | awk '{print $3, $6}'
bad=$(tar -tzf "$OUT/$TAR" | grep -vE "^$NAME(/|$)" || true); [ -z "$bad" ] || { echo "entry outside $NAME/: $bad"; exit 1; }
echo "PACKAGE $OUT/$TAR"
