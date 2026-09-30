#!/usr/bin/env python3
"""Disc cache: retail bytes extracted on demand from the USER'S disc image.

Owner decision 2026-09-28 ("run off the disc"): nothing from the retail game
is committed.  Anything the port, its tests or its tooling needs is read from
the user's own disc image; for convenience it may be extracted into a local,
git-ignored cache that is fully derived from (and keyed by) that image.

Image resolution (first hit wins):
  --image PATH, $PE_DISC1_BIN, local/pe_disc1.path (one line: the .bin path),
  rom/image/*Disc 1*/*.bin
Cache root: $PE_DISC_CACHE_DIR, else <repo>/build/disc-cache (git-ignored).

Layout (deterministic; every file hash-verified against the region table):
  <root>/index.json            image identity stamp -> image sha1
  <root>/<image_sha1>/manifest.json
  <root>/<image_sha1>/SYSTEM.CNF, SLUS_006.62 (or .68), PE.IMG
  <root>/current               text file naming the active <image_sha1>

Commands:
  populate [--image P] [--no-peimg]   validate + extract (idempotent)
  check    [--image P]                validate only (region/disc table)
  path     [--image P]                print the cache dir (exit 1 if absent)
  table                               print the known-disc table

Only stdlib; reuses tools/extract/psxiso.py for ISO9660.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))
import psxiso  # noqa: E402

# Known discs.  Keep in sync with pc_port/platform/pe_disc_check.c
# (tools/extract/test_disc_cache.py cross-checks the two tables).
# exe_sha1 None = recognised serial, no verified dump yet -> unsupported.
KNOWN_DISCS = [
    # region, disc, serial, boot file, exe sha1, PE.IMG sha1, whole-image sha1
    ("USA", 1, "SLUS-00662", "SLUS_006.62", "452fb033f2eaa4b18aa20a5bca60b8125af3a37b",
     "146c0ce7308bf9fdc2ba5a84230e198db0663f3b", "c339455d5b1dae04f77c2ee847d0932adaf2e84b"),
    ("USA", 2, "SLUS-00668", "SLUS_006.68", "452fb033f2eaa4b18aa20a5bca60b8125af3a37b",
     "146c0ce7308bf9fdc2ba5a84230e198db0663f3b", "6dc5b537527aa0d54bdbbd0c14b458083b0743e4"),
    ("JPN", 1, "SLPS-01230", "SLPS_012.30", None, None, None),
    ("JPN", 2, "SLPS-01231", "SLPS_012.31", None, None, None),
]


class DiscError(RuntimeError):
    pass


def sha1_file(path: Path, chunk: int = 1 << 20) -> str:
    h = hashlib.sha1()
    with open(path, "rb") as f:
        while True:
            b = f.read(chunk)
            if not b:
                break
            h.update(b)
    return h.hexdigest()


def cache_root() -> Path:
    env = os.environ.get("PE_DISC_CACHE_DIR")
    return Path(env).expanduser() if env else ROOT / "build/disc-cache"


def resolve_image(explicit: str | None = None) -> Path | None:
    cands: list[str] = []
    if explicit:
        cands.append(explicit)
    env = os.environ.get("PE_DISC1_BIN")
    if env:
        cands.append(env)
    lp = ROOT / "local/pe_disc1.path"
    if lp.exists():
        line = lp.read_text().strip().splitlines()
        if line:
            cands.append(line[0].strip())
    cands.extend(str(p) for p in sorted((ROOT / "rom/image").glob("*Disc 1*/*.bin")))
    for c in cands:
        p = Path(c).expanduser()
        if p.suffix.lower() == ".cue" and p.exists():
            for ln in p.read_text(errors="replace").splitlines():
                ln = ln.strip()
                if ln.upper().startswith("FILE") and '"' in ln:
                    p = p.parent / ln.split('"')[1]
                    break
        if p.is_file():
            return p
    return None


def _stamp(image: Path) -> str:
    st = image.stat()
    return f"{image.resolve()}|{st.st_size}|{st.st_mtime_ns}"


def _load_index() -> dict:
    p = cache_root() / "index.json"
    try:
        return json.loads(p.read_text())
    except (OSError, ValueError):
        return {}


def image_sha1(image: Path) -> str:
    """SHA-1 of the whole image, memoised by (path, size, mtime)."""
    idx = _load_index()
    key = _stamp(image)
    if key in idx:
        return idx[key]
    h = sha1_file(image)
    idx[key] = h
    root = cache_root()
    root.mkdir(parents=True, exist_ok=True)
    tmp = root / "index.json.tmp"
    tmp.write_text(json.dumps(idx, indent=1, sort_keys=True))
    tmp.replace(root / "index.json")
    return h


def identify(image: Path) -> dict:
    """Validate the image and identify it against KNOWN_DISCS.

    Raises DiscError with a user-facing remedy on any problem."""
    try:
        img = psxiso.RawImage(str(image))
    except (OSError, psxiso.IsoError) as exc:
        raise DiscError(f"{image}: not a raw MODE2/2352 BIN image ({exc}). "
                        "Use the .bin of a BIN/CUE dump (not .iso/.chd).") from exc
    try:
        files = {p.split(";")[0].upper(): (lba, size) for p, lba, size in psxiso.walk(img)}
    except Exception as exc:
        raise DiscError(f"{image}: unreadable ISO9660 filesystem ({exc})") from exc
    for region, disc, serial, boot, exe_sha1, peimg_sha1, image_sha1 in KNOWN_DISCS:
        if boot.upper() not in files:
            continue
        if exe_sha1 is None:
            raise DiscError(f"{image}: recognised Parasite Eve {region} Disc {disc} "
                            f"({serial}) but this region is not supported yet "
                            "(only USA SLUS-00662/00668).")
        lba, size = files[boot.upper()]
        data = _read_file(img, lba, size)
        got = hashlib.sha1(data).hexdigest()
        if got != exe_sha1:
            raise DiscError(f"{image}: {boot} SHA-1 {got} != expected {exe_sha1} "
                            f"for {region} Disc {disc} ({serial}); the dump is "
                            "damaged or a different revision.")
        cnf = files.get("SYSTEM.CNF")
        if cnf is None or boot.upper() not in _read_file(img, *cnf).decode("ascii", "replace").upper():
            raise DiscError(f"{image}: SYSTEM.CNF does not boot {boot} ({serial}); "
                            "damaged or modified image")
        if "PE.IMG" not in files:
            raise DiscError(f"{image}: PE.IMG missing (damaged dump?)")
        return {"region": region, "disc": disc, "serial": serial, "boot": boot,
                "exe_sha1": exe_sha1, "peimg_sha1": peimg_sha1,
                "image_sha1": image_sha1, "files": files,
                "img": img}
    raise DiscError(f"{image}: not a Parasite Eve disc (no known boot file; "
                    f"found {sorted(files)[:6]}...)")


def _read_file(img, lba: int, size: int) -> bytes:
    out = bytearray()
    sector = lba
    while len(out) < size:
        out += img.user_data(sector, check_form=True)
        sector += 1
    return bytes(out[:size])


def populate(image: Path, want_peimg: bool = True) -> Path:
    info = identify(image)
    key = image_sha1(image)
    if info["image_sha1"] and key != info["image_sha1"]:
        print(f"WARNING: {image} SHA-1 {key} differs from the reference "
              f"{info['region']} Disc {info['disc']} dump {info['image_sha1']} "
              "(EXE and PE.IMG verified; other tracks may differ)", file=sys.stderr)
    d = cache_root() / key
    d.mkdir(parents=True, exist_ok=True)
    img = info["img"]
    want = [("SYSTEM.CNF", None), (info["boot"], info["exe_sha1"])]
    if want_peimg:
        want.append(("PE.IMG", info["peimg_sha1"]))
    manifest = {"image": str(image.resolve()), "image_sha1": key,
                "region": info["region"], "disc": info["disc"],
                "serial": info["serial"], "files": {}}
    for name, expect in want:
        out = d / name
        lba, size = info["files"][name]
        if not (out.exists() and out.stat().st_size == size
                and (expect is None or sha1_file(out) == expect)):
            tmp = out.with_suffix(out.suffix + ".tmp")
            with open(tmp, "wb") as f:
                sector, left = lba, size
                while left > 0:
                    b = img.user_data(sector, check_form=True)
                    f.write(b[:min(len(b), left)])
                    left -= len(b)
                    sector += 1
            tmp.replace(out)
        got = sha1_file(out)
        if expect is not None and got != expect:
            raise DiscError(f"cache file {out} SHA-1 {got} != {expect}")
        manifest["files"][name] = {"lba": lba, "size": size, "sha1": got}
    # The boot EXE is also exposed as SLUS_006.62 so tools have one name for
    # "the executable" (the USA Disc 1/2 EXEs are byte-identical).
    if info["boot"] != "SLUS_006.62":
        alias = d / "SLUS_006.62"
        if not alias.exists():
            alias.write_bytes((d / info["boot"]).read_bytes())
    (d / "manifest.json").write_text(json.dumps(manifest, indent=1, sort_keys=True) + "\n")
    (cache_root() / "current").write_text(key + "\n")
    return d


def cache_dir_if_ready(image: Path | None = None) -> Path | None:
    """The populated cache dir for the configured image, else None (no I/O
    beyond stat + small json reads; never hashes the image)."""
    image = image or resolve_image()
    root = cache_root()
    key = None
    if image is not None:
        key = _load_index().get(_stamp(image))
    if key is None:
        cur = root / "current"
        if cur.exists():
            key = cur.read_text().strip()
    if not key:
        return None
    d = root / key
    if (d / "manifest.json").exists() and (d / "SLUS_006.62").exists():
        return d
    return None


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="user-disc cache (never committed)")
    ap.add_argument("cmd", choices=["populate", "check", "path", "table"])
    ap.add_argument("--image")
    ap.add_argument("--no-peimg", action="store_true")
    a = ap.parse_args(argv)
    if a.cmd == "table":
        for row in KNOWN_DISCS:
            print("\t".join(str(x) for x in row))
        return 0
    image = resolve_image(a.image)
    if a.cmd == "path":
        d = cache_dir_if_ready(image)
        if d is None:
            print("disc cache not populated: run tools/extract/disc_cache.py populate",
                  file=sys.stderr)
            return 1
        print(d)
        return 0
    if image is None:
        print("ERROR: no disc image configured. Set PE_DISC1_BIN=/path/to/Disc1.bin, "
              "or write the path into local/pe_disc1.path, or pass --image.",
              file=sys.stderr)
        return 1
    try:
        if a.cmd == "check":
            info = identify(image)
            print(f"OK: {image} = Parasite Eve {info['region']} Disc {info['disc']} "
                  f"({info['serial']}), {info['boot']} sha1 {info['exe_sha1']}")
            return 0
        d = populate(image, not a.no_peimg)
        print(d)
        return 0
    except DiscError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
