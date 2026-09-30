#!/usr/bin/env python3
"""Rebuild-target descriptors: the main EXE and the PE.IMG overlays.

The Disc 1 tooling (``disc1_plan.py`` / ``disc1_build.py`` / ``disc1_verify.py``
/ ``disc1_preflight.py``) is parametrized over one ``Target`` instead of the
historical hard-coded EXE geometry.  ``MAIN_TARGET`` reproduces that geometry
exactly (0x800 PS-X header, VRAM 0x80010000, file end 0x1EE800, sources under
``src/``), so the default behaviour of every tool is unchanged; an overlay
target is looked up by id in ``configs/USA/overlays/manifest.yaml``.

The manifest is committed and contains only facts (sector ranges, VRAM, sizes,
SHA-1s); the overlay bytes themselves are extracted locally into git-ignored
``build/extracted/disc1/peimg/`` by ``scripts/extract_overlays.sh``.

This module uses only the standard library on purpose (the public plan gate
must not need PyYAML), so the manifest is restricted to the YAML subset
``parse_simple_yaml`` accepts: nested mappings by two-space indentation, scalar
values (hex/decimal integers, booleans, quoted or bare strings), comments.
``tools/build/test_overlay_plan.py`` cross-checks the reader against PyYAML
when it is installed.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_MANIFEST = Path("configs/USA/overlays/manifest.yaml")
OVERLAY_ID_RE = re.compile(r"^[a-z][A-Za-z0-9_]*$")
SHA1_HEX_RE = re.compile(r"^[0-9a-fA-F]{40}$")


class TargetError(RuntimeError):
    """The target manifest is missing, malformed, or names an unknown id."""


@dataclass(frozen=True)
class Target:
    """Everything the plan/build/verify tools need to know about one image."""

    id: str
    config: str
    profiles: str
    retail: str
    file_start: int
    file_end: int
    vram: int
    has_header: bool
    src_dir: str
    asm_dir: str
    asset_dir: str
    build_dir: str
    candidate: str
    generated_dir: str
    expected_sha1: str | None = None
    description: str = ""
    # Undefined-symbol prefixes the link may derive from their address-named
    # form.  The EXE keeps its historical `D_`-only derivation; overlays also
    # call into the EXE (`func_8001xxxx`) and reference pool tables (`jtbl_`).
    derive_symbol_prefixes: tuple[str, ...] = ("D_",)

    @property
    def is_main(self) -> bool:
        return self.id == "disc1"

    @property
    def load_size(self) -> int:
        return self.file_end - self.file_start

    @property
    def header_source(self) -> str | None:
        return f"{self.asm_dir}/header.s" if self.has_header else None

    @property
    def auto_symbol_files(self) -> tuple[str, ...]:
        """splat's `undefined_*_auto.txt` outputs for this target."""
        if self.is_main:
            return ("undefined_syms_auto.txt", "undefined_funcs_auto.txt")
        return (
            f"{self.asm_dir}/undefined_syms_auto.txt",
            f"{self.asm_dir}/undefined_funcs_auto.txt",
        )

    def serializable(self) -> dict[str, Any]:
        return {
            "id": self.id,
            "config": self.config,
            "profiles": self.profiles,
            "retail": self.retail,
            "file_start": self.file_start,
            "file_end": self.file_end,
            "vram": self.vram,
            "vram_hex": f"0x{self.vram:08X}",
            "has_header": self.has_header,
            "src_dir": self.src_dir,
            "asm_dir": self.asm_dir,
            "asset_dir": self.asset_dir,
            "build_dir": self.build_dir,
            "candidate": self.candidate,
            "generated_dir": self.generated_dir,
            "expected_sha1": self.expected_sha1,
            "description": self.description,
            "derive_symbol_prefixes": list(self.derive_symbol_prefixes),
        }


# The historical Disc 1 executable geometry, verbatim.  Every default in the
# build tools resolves to this descriptor.
MAIN_TARGET = Target(
    id="disc1",
    config="configs/USA/disc1.yaml",
    profiles="configs/USA/disc1_build_profiles.json",
    retail="build/extracted/disc1/SLUS_006.62",
    file_start=0x800,
    file_end=0x1EE800,
    vram=0x80010000,
    has_header=True,
    src_dir="src",
    asm_dir="asm/disc1",
    asset_dir="assets/disc1",
    build_dir="build",
    candidate="build/disc1.candidate.exe",
    generated_dir="build/generated",
    expected_sha1=None,  # taken from the YAML `sha1:` line, as before
    description="Parasite Eve (USA) Disc 1 executable SLUS_006.62",
)


# ---------------------------------------------------------------------------
# Minimal YAML-subset reader (stdlib only)
# ---------------------------------------------------------------------------
_KEY_RE = re.compile(r"^([A-Za-z_][A-Za-z0-9_\-]*):(?:\s+(.*))?$")


def _scalar(raw: str, where: str) -> Any:
    text = raw.strip()
    if not text:
        raise TargetError(f"{where}: empty scalar")
    if len(text) >= 2 and text[0] == text[-1] and text[0] in "\"'":
        return text[1:-1]
    lowered = text.lower()
    if lowered in ("true", "yes"):
        return True
    if lowered in ("false", "no"):
        return False
    if re.fullmatch(r"0[xX][0-9A-Fa-f]+", text):
        return int(text, 16)
    if re.fullmatch(r"-?[0-9]+", text):
        return int(text, 10)
    return text


def _strip_comment(line: str) -> str:
    # A `#` starts a comment only at line start or after whitespace, and never
    # inside a quoted scalar (the manifest keeps quoted values on one line).
    out = []
    quote: str | None = None
    for index, char in enumerate(line):
        if quote:
            out.append(char)
            if char == quote:
                quote = None
            continue
        if char in "\"'":
            quote = char
            out.append(char)
            continue
        if char == "#" and (index == 0 or line[index - 1].isspace()):
            break
        out.append(char)
    return "".join(out).rstrip()


def parse_simple_yaml(text: str, *, where: str = "<manifest>") -> dict[str, Any]:
    """Parse the mapping/scalar YAML subset used by the overlay manifest."""
    root: dict[str, Any] = {}
    # Stack entries: [parent_indent, mapping, key_indent].  Keys indented
    # deeper than parent_indent belong to `mapping`; key_indent pins the
    # indentation of that mapping's keys once the first one is seen.
    stack: list[list[Any]] = [[-1, root, 0]]
    pending: tuple[int, str, dict[str, Any]] | None = None  # (indent, key, owner)
    for number, raw in enumerate(text.splitlines(), 1):
        line = _strip_comment(raw.expandtabs(8))
        if not line.strip():
            continue
        indent = len(line) - len(line.lstrip(" "))
        body = line.strip()
        match = _KEY_RE.match(body)
        if not match:
            raise TargetError(
                f"{where}:{number}: unsupported YAML line (only `key: value` and "
                f"`key:` mappings are allowed): {body!r}"
            )
        key, value = match.group(1), match.group(2)
        if pending is not None:
            pending_indent, pending_key, owner = pending
            pending = None
            if indent <= pending_indent:
                raise TargetError(
                    f"{where}:{number}: key {pending_key!r} has no value and no "
                    "nested mapping"
                )
            child: dict[str, Any] = {}
            owner[pending_key] = child
            stack.append([pending_indent, child, indent])
        while indent <= stack[-1][0]:
            stack.pop()
        _, owner, key_indent = stack[-1]
        if indent != key_indent:
            raise TargetError(
                f"{where}:{number}: inconsistent indentation ({indent} vs {key_indent})"
            )
        if key in owner:
            raise TargetError(f"{where}:{number}: duplicate key {key!r}")
        if value is None or not value.strip():
            owner[key] = None
            pending = (indent, key, owner)
        else:
            owner[key] = _scalar(value, f"{where}:{number}")
    if pending is not None:
        raise TargetError(f"{where}: key {pending[1]!r} has no value and no nested mapping")
    return root


# ---------------------------------------------------------------------------
# Manifest -> Target
# ---------------------------------------------------------------------------
def _repo_path(root: Path, path: Path | str) -> Path:
    candidate = Path(path)
    return candidate if candidate.is_absolute() else root / candidate


def load_manifest(root: Path = REPO_ROOT, manifest: Path | str = DEFAULT_MANIFEST) -> dict[str, Any]:
    path = _repo_path(root, manifest)
    try:
        text = path.read_text(encoding="utf-8")
    except FileNotFoundError as exc:
        raise TargetError(f"missing overlay manifest: {path}") from exc
    data = parse_simple_yaml(text, where=str(path))
    if data.get("schema_version") != 1:
        raise TargetError(f"{path}: schema_version must be 1")
    for section in ("image", "exe", "rooms", "overlays"):
        if not isinstance(data.get(section), dict):
            raise TargetError(f"{path}: `{section}` must be a mapping")
    image = data["image"]
    for key in ("name", "extracted", "size", "sectors", "sha1"):
        if key not in image:
            raise TargetError(f"{path}: image.{key} is required")
    if not SHA1_HEX_RE.match(str(image["sha1"])):
        raise TargetError(f"{path}: image.sha1 must be 40 hex digits")
    if image["size"] != image["sectors"] * 2048:
        raise TargetError(f"{path}: image.size != image.sectors * 2048")
    exe = data["exe"]
    for key in ("extracted", "sha1", "range_table", "dest_table", "room_table",
                "room_table_entries", "room_base_lba"):
        if key not in exe:
            raise TargetError(f"{path}: exe.{key} is required")
    for ovl_id, entry in data["overlays"].items():
        if not OVERLAY_ID_RE.match(ovl_id):
            raise TargetError(f"{path}: overlay id {ovl_id!r} must match {OVERLAY_ID_RE.pattern}")
        if not isinstance(entry, dict):
            raise TargetError(f"{path}: overlays.{ovl_id} must be a mapping")
        for key in ("kind", "sector_start", "sector_end", "size", "vram", "sha1", "blob"):
            if key not in entry:
                raise TargetError(f"{path}: overlays.{ovl_id}.{key} is required")
        if entry["kind"] not in ("subsystem", "room"):
            raise TargetError(f"{path}: overlays.{ovl_id}.kind must be subsystem|room")
        if not SHA1_HEX_RE.match(str(entry["sha1"])):
            raise TargetError(f"{path}: overlays.{ovl_id}.sha1 must be 40 hex digits")
        if entry["size"] != (entry["sector_end"] - entry["sector_start"]) * 2048:
            raise TargetError(
                f"{path}: overlays.{ovl_id}.size 0x{entry['size']:X} != "
                f"(sector_end - sector_start) * 2048"
            )
        if entry["sector_end"] > image["sectors"]:
            raise TargetError(f"{path}: overlays.{ovl_id} ends past the image")
        if "text_start" in entry and "text_end" in entry:
            if not 0 <= entry["text_start"] < entry["text_end"] <= entry["size"]:
                raise TargetError(f"{path}: overlays.{ovl_id} text window is not inside the blob")
        if ("config" in entry) != ("build_profiles" in entry):
            raise TargetError(
                f"{path}: overlays.{ovl_id} needs both `config` and `build_profiles` or neither"
            )
    return data


def overlay_target(ovl_id: str, entry: dict[str, Any]) -> Target:
    if "config" not in entry:
        raise TargetError(
            f"overlay {ovl_id!r} has no splat config yet (add `config` and "
            "`build_profiles` to the manifest entry)"
        )
    return Target(
        id=ovl_id,
        config=str(entry["config"]),
        profiles=str(entry["build_profiles"]),
        retail=str(entry["blob"]),
        file_start=0,
        file_end=int(entry["size"]),
        vram=int(entry["vram"]),
        has_header=False,
        src_dir=f"src/overlays/{ovl_id}",
        asm_dir=f"asm/overlays/{ovl_id}",
        asset_dir=f"assets/overlays/{ovl_id}",
        build_dir=f"build/overlays/{ovl_id}",
        candidate=f"build/overlays/{ovl_id}/candidate.bin",
        generated_dir=f"build/overlays/{ovl_id}/generated",
        expected_sha1=str(entry["sha1"]).lower(),
        description=str(entry.get("description", "")),
        # `.L` too: splat names a lui/addiu address that happens to equal a
        # branch label as that local label (ovl_0457 func_80123CAC indexes a
        # byte table as `.L80125A9C` + (c << 2)); once a C carve splits the
        # referencing and defining asm into different files, the reference
        # must bind by address like the other address-named symbols.
        derive_symbol_prefixes=("D_", "jtbl_", "func_", ".L"),
    )


def load_target(
    name: str | None,
    root: Path = REPO_ROOT,
    manifest: Path | str = DEFAULT_MANIFEST,
) -> Target:
    """Resolve a CLI `--target` value: None/"disc1" -> the EXE, else an overlay."""
    if name in (None, "", MAIN_TARGET.id):
        return MAIN_TARGET
    data = load_manifest(root, manifest)
    entry = data["overlays"].get(name)
    if entry is None:
        known = ", ".join(sorted(data["overlays"]))
        raise TargetError(f"unknown target {name!r}; manifest overlays: {known}")
    return overlay_target(name, entry)


def configured_overlay_ids(root: Path = REPO_ROOT, manifest: Path | str = DEFAULT_MANIFEST) -> list[str]:
    """Overlay ids that carry a splat config (i.e. are rebuild targets)."""
    data = load_manifest(root, manifest)
    return sorted(ovl_id for ovl_id, entry in data["overlays"].items() if "config" in entry)
