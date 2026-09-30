#!/usr/bin/env python3
"""PE.IMG overlay extractor (stdlib only).

PE.IMG is the single overlay-bearing file on both Parasite Eve (USA) discs.
The retail executable describes its layout in three tables; this tool reads
them from the locally extracted EXE, verifies every input hash against
``configs/USA/overlays/manifest.yaml``, and writes the requested overlay bytes
into the git-ignored ``build/extracted/disc1/peimg/`` directory.  Nothing here
is ever committed: the manifest carries hashes and geometry only.

Sub-commands:

  tables            dump the EXE overlay tables (image-load range boundaries,
                    load destinations, room package table) as facts
  extract           write every subsystem overlay listed in the manifest
                    (``--only <id>`` restricts), verify its SHA-1, and record
                    ``<out>/hashes.txt``; idempotent (re-run = re-verify)
  check             verify already-extracted blobs against the manifest
  room <m0418i|slot> [--chunk N]
                    write one room package chunk (default chunk 2, the MIPS
                    bearing chunk that loads at rooms.chunk2_vram) and print
                    its geometry and SHA-1 (recorded facts; the file is ignored)

Every failure (missing PE.IMG, wrong EXE, hash mismatch) exits non-zero with
the exact remedy.  Exit 0 only when every requested blob is present and hashes
as recorded.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
import sys
from pathlib import Path
from typing import Any

_THIS_DIR = Path(__file__).resolve().parent
REPO_ROOT = _THIS_DIR.parents[1]
sys.path.insert(0, str(REPO_ROOT / "tools/build"))

from overlay_targets import (  # noqa: E402
    DEFAULT_MANIFEST,
    TargetError,
    load_manifest,
)

SECTOR = 2048
EXE_VRAM = 0x80010000
EXE_FILE_START = 0x800


class ExtractError(RuntimeError):
    """A prerequisite or hash check failed."""


def sha1_of(path: Path, chunk: int = 1 << 20) -> str:
    digest = hashlib.sha1()
    with path.open("rb") as stream:
        while True:
            block = stream.read(chunk)
            if not block:
                break
            digest.update(block)
    return digest.hexdigest()


def _repo_path(root: Path, path: str | Path) -> Path:
    candidate = Path(path)
    return candidate if candidate.is_absolute() else root / candidate


# ---------------------------------------------------------------------------
# EXE tables
# ---------------------------------------------------------------------------
def exe_offset(vram: int) -> int:
    return vram - EXE_VRAM + EXE_FILE_START


def read_exe(root: Path, manifest: dict[str, Any]) -> bytes:
    path = _repo_path(root, manifest["exe"]["extracted"])
    if not path.is_file():
        raise ExtractError(
            f"missing retail executable {path}; run scripts/extract_us.sh 1 first"
        )
    actual = sha1_of(path)
    if actual != str(manifest["exe"]["sha1"]).lower():
        raise ExtractError(
            f"retail executable SHA-1 {actual} != manifest {manifest['exe']['sha1']}"
        )
    return path.read_bytes()


def range_table(exe: bytes, manifest: dict[str, Any]) -> list[int]:
    base = int(manifest["exe"]["range_table"])
    count = int(manifest["exe"]["range_table_entries"])
    off = exe_offset(base)
    return [struct.unpack_from("<H", exe, off + 2 * i)[0] for i in range(count)]


def dest_table(exe: bytes, manifest: dict[str, Any]) -> list[int]:
    base = int(manifest["exe"]["dest_table"])
    count = int(manifest["exe"]["dest_table_entries"])
    off = exe_offset(base)
    return [struct.unpack_from("<I", exe, off + 4 * i)[0] for i in range(count)]


def room_table(exe: bytes, manifest: dict[str, Any]) -> list[dict[str, Any]]:
    base = int(manifest["exe"]["room_table"])
    count = int(manifest["exe"]["room_table_entries"])
    off = exe_offset(base)
    rooms = []
    for slot in range(count):
        rel, packed = struct.unpack_from("<II", exe, off + 8 * slot)
        sec = (packed & 0xFF, (packed >> 8) & 0xFFF, (packed >> 20) & 0xFFF)
        rooms.append(
            {
                "slot": slot,
                "name": f"m{slot + 1:04d}i",
                "rel": rel,
                "packed": packed,
                "sec": sec,
                "lba": int(manifest["exe"]["room_base_lba"]) + rel,
            }
        )
    return rooms


def cross_check_manifest(exe: bytes, manifest: dict[str, Any]) -> None:
    """Every manifest overlay must agree with the EXE tables it cites."""
    ranges = range_table(exe, manifest)
    dests = dest_table(exe, manifest)
    for ovl_id, entry in manifest["overlays"].items():
        if entry["kind"] != "subsystem":
            continue
        index = int(entry["range_index"])
        if not 0 <= index < len(ranges) - 1:
            raise ExtractError(f"{ovl_id}: range_index {index} out of the EXE table")
        if (ranges[index], ranges[index + 1]) != (entry["sector_start"], entry["sector_end"]):
            raise ExtractError(
                f"{ovl_id}: manifest sectors [0x{entry['sector_start']:X},"
                f"0x{entry['sector_end']:X}) != EXE table entry {index} "
                f"[0x{ranges[index]:X},0x{ranges[index + 1]:X})"
            )
        dest = int(entry["dest_index"])
        if not 0 <= dest < len(dests) or dests[dest] != entry["vram"]:
            raise ExtractError(
                f"{ovl_id}: manifest vram 0x{entry['vram']:08X} != EXE dest table "
                f"entry {dest}"
            )


# ---------------------------------------------------------------------------
# PE.IMG access
# ---------------------------------------------------------------------------
def open_image(root: Path, manifest: dict[str, Any], *, hash_image: bool) -> Path:
    image = manifest["image"]
    path = _repo_path(root, image["extracted"])
    if not path.is_file():
        raise ExtractError(
            f"missing {path}: PE.IMG is not extracted. Run scripts/extract_overlays.sh "
            "(it extracts PE.IMG from the disc image under rom/image/ with "
            "tools/extract/psxiso.py when it is absent)"
        )
    size = path.stat().st_size
    if size != int(image["size"]):
        raise ExtractError(f"{path}: size {size} != manifest {image['size']}")
    if hash_image:
        actual = sha1_of(path)
        if actual != str(image["sha1"]).lower():
            raise ExtractError(f"{path}: SHA-1 {actual} != manifest {image['sha1']}")
    return path


def read_sectors(image: Path, start: int, end: int) -> bytes:
    if not 0 <= start < end:
        raise ExtractError(f"bad sector range [{start},{end})")
    with image.open("rb") as stream:
        stream.seek(start * SECTOR)
        data = stream.read((end - start) * SECTOR)
    if len(data) != (end - start) * SECTOR:
        raise ExtractError(f"short read for sectors [{start},{end}) of {image}")
    return data


def write_blob(path: Path, data: bytes, expected_sha1: str | None) -> tuple[str, bool]:
    """Write ``data`` unless an identical file exists; return (sha1, written)."""
    actual = hashlib.sha1(data).hexdigest()
    if expected_sha1 is not None and actual != expected_sha1.lower():
        raise ExtractError(
            f"{path.name}: extracted bytes hash {actual}, manifest says "
            f"{expected_sha1}; the image or the manifest geometry is wrong — "
            "nothing was written"
        )
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.is_file() and sha1_of(path) == actual:
        return actual, False
    path.write_bytes(data)
    return actual, True


def record_hashes(out_dir: Path, hashes: dict[str, str]) -> Path:
    path = out_dir / "hashes.txt"
    existing: dict[str, str] = {}
    if path.is_file():
        for line in path.read_text(encoding="utf-8").splitlines():
            parts = line.split()
            if len(parts) == 2:
                existing[parts[1]] = parts[0]
    existing.update(hashes)
    path.write_text(
        "".join(f"{digest}  {name}\n" for name, digest in sorted(existing.items())),
        encoding="utf-8",
    )
    return path


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------
def cmd_tables(args: argparse.Namespace, root: Path, manifest: dict[str, Any]) -> int:
    exe = read_exe(root, manifest)
    ranges = range_table(exe, manifest)
    dests = dest_table(exe, manifest)
    print(f"range table {manifest['exe']['range_table']:#x} ({len(ranges)} halfwords):")
    for index, (start, end) in enumerate(zip(ranges, ranges[1:])):
        tag = ""
        for ovl_id, entry in manifest["overlays"].items():
            if entry.get("range_index") == index:
                tag = f"  <- {ovl_id} vram 0x{entry['vram']:08X}"
        print(f"  [{index:2d}] sectors [0x{start:04X},0x{end:04X}) size 0x{(end - start) * SECTOR:X}{tag}")
    print(f"dest table {manifest['exe']['dest_table']:#x}:")
    for index, value in enumerate(dests):
        print(f"  [{index}] 0x{value:08X}")
    rooms = room_table(exe, manifest)
    code_rooms = sum(1 for room in rooms if room["sec"][2] > 0)
    print(
        f"room table {manifest['exe']['room_table']:#x}: {len(rooms)} slots, "
        f"base LBA {manifest['exe']['room_base_lba']}, first {rooms[0]['name']} rel "
        f"{rooms[0]['rel']} sec {rooms[0]['sec']}, last {rooms[-1]['name']} rel "
        f"{rooms[-1]['rel']} sec {rooms[-1]['sec']}, slots with a chunk 2: {code_rooms}"
    )
    cross_check_manifest(exe, manifest)
    print("manifest overlays agree with the EXE tables")
    return 0


def cmd_extract(args: argparse.Namespace, root: Path, manifest: dict[str, Any]) -> int:
    exe = read_exe(root, manifest)
    cross_check_manifest(exe, manifest)
    image = open_image(root, manifest, hash_image=not args.skip_image_hash)
    out_dir = _repo_path(root, args.out or manifest["rooms"]["extract_dir"])
    wanted = manifest["overlays"]
    if args.only:
        missing = [ovl_id for ovl_id in args.only if ovl_id not in wanted]
        if missing:
            raise ExtractError(f"unknown overlay id(s): {', '.join(missing)}")
        wanted = {ovl_id: wanted[ovl_id] for ovl_id in args.only}
    hashes: dict[str, str] = {}
    for ovl_id, entry in wanted.items():
        if entry["kind"] != "subsystem":
            continue
        data = read_sectors(image, int(entry["sector_start"]), int(entry["sector_end"]))
        blob = _repo_path(root, entry["blob"])
        digest, written = write_blob(blob, data, str(entry["sha1"]))
        hashes[blob.name] = digest
        print(
            f"{'wrote' if written else 'ok   '} {blob.relative_to(root)} "
            f"sectors [0x{entry['sector_start']:X},0x{entry['sector_end']:X}) "
            f"size 0x{len(data):X} vram 0x{entry['vram']:08X} sha1 {digest}"
        )
    if not hashes:
        raise ExtractError("no subsystem overlay selected")
    recorded = record_hashes(out_dir, hashes)
    print(f"recorded {len(hashes)} hash(es) in {recorded.relative_to(root)}")
    return 0


def cmd_check(args: argparse.Namespace, root: Path, manifest: dict[str, Any]) -> int:
    failures = 0
    for ovl_id, entry in manifest["overlays"].items():
        blob = _repo_path(root, entry["blob"])
        if args.only and ovl_id not in args.only:
            continue
        if not blob.is_file():
            print(f"MISSING {blob.relative_to(root)} ({ovl_id})")
            failures += 1
            continue
        actual = sha1_of(blob)
        if actual != str(entry["sha1"]).lower() or blob.stat().st_size != int(entry["size"]):
            print(f"MISMATCH {blob.relative_to(root)}: {actual} != {entry['sha1']}")
            failures += 1
        else:
            print(f"ok {blob.relative_to(root)} sha1 {actual}")
    if failures:
        raise ExtractError(f"{failures} overlay blob(s) missing or wrong; run scripts/extract_overlays.sh")
    return 0


def cmd_room(args: argparse.Namespace, root: Path, manifest: dict[str, Any]) -> int:
    exe = read_exe(root, manifest)
    rooms = room_table(exe, manifest)
    key = args.room.lower()
    if key.isdigit():
        slot = int(key)
    elif key.startswith("m") and key.endswith("i") and key[1:-1].isdigit():
        slot = int(key[1:-1]) - 1
    else:
        raise ExtractError(f"room must be m####i or a slot number, got {args.room!r}")
    if not 0 <= slot < len(rooms):
        raise ExtractError(f"room slot {slot} out of range 0..{len(rooms) - 1}")
    room = rooms[slot]
    chunk = args.chunk
    if not 0 <= chunk <= 2:
        raise ExtractError("chunk must be 0, 1 or 2")
    if room["sec"][chunk] == 0:
        raise ExtractError(f"{room['name']} has no chunk {chunk} (sector count 0)")
    image = open_image(root, manifest, hash_image=not args.skip_image_hash)
    # Chunks are contiguous after the package base; PE.IMG offsets are
    # disc LBAs minus PE.IMG's own LBA.
    image_lba = int(manifest["image"]["disc_lba"])
    start = room["lba"] + sum(room["sec"][:chunk]) - image_lba
    end = start + room["sec"][chunk]
    data = read_sectors(image, start, end)
    out_dir = _repo_path(root, args.out or manifest["rooms"]["extract_dir"])
    blob = out_dir / f"room_{room['name']}_c{chunk}.bin"
    digest, written = write_blob(blob, data, None)
    vram = int(manifest["rooms"]["chunk2_vram"]) if chunk == 2 else None
    print(
        f"{'wrote' if written else 'ok   '} {blob.relative_to(root)} slot {slot} "
        f"{room['name']} rel {room['rel']} sec {room['sec']} chunk {chunk} "
        f"PE.IMG sectors [0x{start:X},0x{end:X}) size 0x{len(data):X}"
        + (f" vram 0x{vram:08X}" if vram else "")
        + f" sha1 {digest}"
    )
    record_hashes(out_dir, {blob.name: digest})
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--root", type=Path, default=REPO_ROOT)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument(
        "--skip-image-hash",
        action="store_true",
        help="trust PE.IMG's size only (the full 206 MB SHA-1 takes ~1s)",
    )
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("tables", help="dump the EXE overlay tables")
    extract = sub.add_parser("extract", help="extract subsystem overlays")
    extract.add_argument("--out", type=Path, help="output directory (default: manifest rooms.extract_dir)")
    extract.add_argument("--only", action="append", metavar="ID", help="restrict to this overlay id")
    check = sub.add_parser("check", help="verify extracted overlay blobs")
    check.add_argument("--only", action="append", metavar="ID")
    room = sub.add_parser("room", help="extract one room package chunk")
    room.add_argument("room", help="room name (m0418i) or slot number")
    room.add_argument("--chunk", type=int, default=2)
    room.add_argument("--out", type=Path)
    args = parser.parse_args(argv)
    root = args.root.resolve()
    try:
        manifest = load_manifest(root, args.manifest)
        handler = {
            "tables": cmd_tables,
            "extract": cmd_extract,
            "check": cmd_check,
            "room": cmd_room,
        }[args.command]
        return handler(args, root, manifest)
    except (ExtractError, TargetError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
