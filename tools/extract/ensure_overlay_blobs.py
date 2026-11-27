#!/usr/bin/env python3
"""Make sure every overlay blob the port generators need is extracted.

``tools/analysis/gen_overlay_ports.py`` fingerprints and lowers code from the
retail overlay blobs named by each ``target_path`` in
``configs/USA/overlays/*.yaml`` (``build/extracted/disc1/peimg/ovl_<id>.bin``
and ``room_<id>_c2.bin``).  The standard extraction
(``scripts/extract_us.sh`` + ``scripts/extract_overlays.sh``) leaves some room
blobs out (m0159i and m0437i at the time of writing); they are produced on
demand by ``tools/extract/peimg.py room <id>``.  Without them the generator
emits a *different* port tree than the development tree (stale/extra TUs), so
``scripts/build_pc_port.sh`` runs this tool before generating.

What it does (idempotent; stdlib only):

  * derives the blob list from the overlay yaml set (no hard-coded ids),
  * extracts each missing subsystem overlay with ``peimg.py extract --only``
    and each missing room chunk with ``peimg.py room <id> --chunk 2``,
  * checks every blob against the ``sha1:`` recorded in its yaml,
  * writes only under ``<root>/build/extracted/disc1/peimg/`` (git-ignored).

It never touches the retail disc: its inputs are the already-extracted
``build/extracted/disc1/SLUS_006.62`` and ``build/extracted/disc1/PE.IMG``.
When either is absent (or a hash fails) it exits 1 with the exact remedy:

    scripts/extract_us.sh 1          # needs your disc 1 bin+cue under rom/image/
                                     #   (or PE_IMAGE_DIR=<dir with '*Disc 1*.cue'>)
    scripts/extract_overlays.sh      # extracts PE.IMG from the disc, then overlays

Exit 0 only when every needed blob is present and hashes as recorded.
"""

from __future__ import annotations

import argparse
import hashlib
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
PEIMG_PREFIX = "build/extracted/disc1/peimg/"
TARGET_RE = re.compile(r"^\s*target_path:\s*(\S+)", re.MULTILINE)
SHA1_RE = re.compile(r"^sha1:\s*([0-9a-f]{40})", re.MULTILINE)
ROOM_RE = re.compile(r"^room_(m\d{4}i)_c2\.bin$")
OVL_RE = re.compile(r"^(ovl_[0-9A-Za-z]+)\.bin$")
REMEDY = (
    "  remedy: scripts/extract_us.sh 1   (needs your disc 1 bin+cue under rom/image/,\n"
    "          or PE_IMAGE_DIR=<dir containing a '*Disc 1*.cue'>)\n"
    "          then scripts/extract_overlays.sh"
)


def sha1_of(path: Path) -> str:
    digest = hashlib.sha1()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def needed_blobs(root: Path) -> dict[str, str | None]:
    """blob file name -> committed sha1 (None when the yaml records none)."""
    out: dict[str, str | None] = {}
    for yaml_path in sorted((root / "configs/USA/overlays").glob("*.yaml")):
        if yaml_path.name == "manifest.yaml":
            continue
        text = yaml_path.read_text(encoding="utf-8")
        m = TARGET_RE.search(text)
        if not m:
            continue
        rel = m.group(1).replace("../", "")
        if not rel.startswith(PEIMG_PREFIX):
            continue
        s = SHA1_RE.search(text)
        out[rel[len(PEIMG_PREFIX):]] = s.group(1) if s else None
    return out


def run_peimg(root: Path, *args: str) -> None:
    cmd = [sys.executable, str(root / "tools/extract/peimg.py"), "--root", str(root), *args]
    print("  $ " + " ".join(cmd[1:]), flush=True)
    rc = subprocess.run(cmd).returncode
    if rc != 0:
        raise SystemExit(f"ensure_overlay_blobs: ERROR: {' '.join(args)} exited {rc}\n{REMEDY}")


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", type=Path, default=REPO_ROOT)
    ap.add_argument("--check", action="store_true", help="report only; extract nothing (exit 1 if any blob is missing)")
    args = ap.parse_args(argv)
    root = args.root.resolve()
    need = needed_blobs(root)
    if not need:
        print("ensure_overlay_blobs: ERROR: no overlay yaml with a peimg target_path under "
              f"{root}/configs/USA/overlays", file=sys.stderr)
        return 1
    peimg_dir = root / PEIMG_PREFIX
    missing = sorted(n for n in need if not (peimg_dir / n).is_file())
    print(f"ensure_overlay_blobs: {len(need)} overlay blobs needed, {len(missing)} missing")
    if missing and args.check:
        for n in missing:
            print(f"  missing {PEIMG_PREFIX}{n}", file=sys.stderr)
        return 1
    if missing:
        exe = root / "build/extracted/disc1/SLUS_006.62"
        img = root / "build/extracted/disc1/PE.IMG"
        absent = [str(p.relative_to(root)) for p in (exe, img) if not p.is_file()]
        if absent:
            shown = ", ".join(missing[:4]) + (f", ... ({len(missing)} in all)" if len(missing) > 4 else "")
            print(f"ensure_overlay_blobs: ERROR: cannot extract {shown}; "
                  f"missing input(s): {', '.join(absent)}\n{REMEDY}", file=sys.stderr)
            return 1
        for name in missing:
            room, ovl = ROOM_RE.match(name), OVL_RE.match(name)
            if room:
                run_peimg(root, "room", room.group(1), "--chunk", "2")
            elif ovl:
                run_peimg(root, "extract", "--only", ovl.group(1))
            else:
                print(f"ensure_overlay_blobs: ERROR: do not know how to extract {name}", file=sys.stderr)
                return 1
    bad = []
    for name, want in sorted(need.items()):
        path = peimg_dir / name
        if not path.is_file():
            bad.append(f"{name}: still missing after extraction")
        elif want is not None and sha1_of(path) != want:
            bad.append(f"{name}: sha1 {sha1_of(path)} != recorded {want}")
    if bad:
        print("ensure_overlay_blobs: ERROR: blob verification failed (wrong disc / stale blob?):\n  "
              + "\n  ".join(bad) + f"\n{REMEDY}", file=sys.stderr)
        return 1
    print(f"ensure_overlay_blobs: OK, {len(need)} blobs present"
          + (f" ({len(missing)} extracted)" if missing else "") + ", all match their recorded sha1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
