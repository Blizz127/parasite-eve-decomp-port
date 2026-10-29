#!/usr/bin/env python3
"""Derive namespaced pc_port TUs from the matched OVERLAY leaves.

Step 1 of the overlay-port mechanism (docs/ai_context/PC_PORT_FROM_DECOMP.md,
"Overlay leaves").  Overlays that share an arena (rooms, ovl_03D2 and
ovl_0700 all load at 0x8018EFxx) reuse the same `func_801xxxxx` names for
different code, so a derived overlay leaf cannot be a plain `func_X`:

  * each matched overlay leaf `src/overlays/<ovl>/func_X.c` becomes
    `pc_port/game/decomp_ovl/<ovl>/func_X_port.c`, defining the host symbol
    `<ovl>__func_X`.  The body stays verbatim: a generated block of
    `#define func_Y <ovl>__func_Y` rename macros (for the leaf itself and every
    same-overlay callee that is also emitted) sits after the shim include;
  * a callee inside the overlay's own VRAM window that is not emitted is a
    loud boundary labelled "<ovl>:func_Y" — it is NEVER bound to a plain-named
    pc_port definition, which may implement another overlay's code at that
    address, unless HAND_OVERLAY_PORTS names the hand port of exactly this
    (overlay, func_Y);
  * an EXE callee (below the overlay windows) binds exactly as an EXE leaf's
    does (canonical / derived host signature, or a loud boundary);
  * a callee in another overlay's window is a loud boundary
    "overlay:func_Y" — retail never links two overlays that share an arena.

Everything else — eligibility (E1–E18), the host adaptation, the E17 compile
gate, pruning and `--verify` — is gen_decomp_ports.py's, reused unchanged.
The banner says `decomp-source-overlay: <ovl>/func_X` so the EXE coverage and
derived-signature scans never mistake an overlay TU for an EXE leaf.

    python3 tools/analysis/gen_overlay_ports.py            # subsystem overlays
    python3 tools/analysis/gen_overlay_ports.py --overlay ovl_0700
    python3 tools/analysis/gen_overlay_ports.py --check --verbose
    python3 tools/analysis/gen_overlay_ports.py --verify
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gen_decomp_ports as g  # noqa: E402
import guest_lower as gl  # noqa: E402

REPO_ROOT = g.REPO_ROOT
OVL_CONFIG_DIR = REPO_ROOT / "configs/USA/overlays"
OVL_SRC_DIR = REPO_ROOT / "src/overlays"
# NON_MATCHING overlay leaves: functionally equivalent C that is not byte-exact.
# They stay `asm` in the overlay YAML (the exact rebuild never sees them); this
# manifest gives their span, their source lives under src/nonmatching/overlays,
# and tools/analysis/e21_diff.py differential-tests each against the retail asm.
NONMATCH_MANIFEST = REPO_ROOT / "configs/USA/overlays/nonmatching.yaml"
NONMATCH_SRC_DIR = REPO_ROOT / "src/nonmatching/overlays"
NONMATCH_RE = re.compile(r"^\s*-\s*\[\s*(\w+)\s*,\s*(\w+)\s*,\s*(0x[0-9A-Fa-f]+)\s*,\s*(0x[0-9A-Fa-f]+)\s*\]",
                         re.MULTILINE)


def nonmatching_leaves() -> dict[str, dict[str, tuple[int, int]]]:
    """{ovl: {name: (file_offset, file_size)}} from the NON_MATCHING manifest."""
    out: dict[str, dict[str, tuple[int, int]]] = {}
    if NONMATCH_MANIFEST.exists():
        for m in NONMATCH_RE.finditer(NONMATCH_MANIFEST.read_text(encoding="utf-8")):
            out.setdefault(m.group(1), {})[m.group(2)] = (int(m.group(3), 16), int(m.group(4), 16))
    return out


def leaf_src(ovl: str, name: str) -> Path:
    """Source of an overlay leaf: the matched C, or the NON_MATCHING C."""
    p = OVL_SRC_DIR / ovl / f"{name}.c"
    if not p.exists() and (NONMATCH_SRC_DIR / ovl / f"{name}.c").exists():
        return NONMATCH_SRC_DIR / ovl / f"{name}.c"
    return p
OUT_ROOT = Path("pc_port/game/decomp_ovl")
SUBSYSTEM_OVERLAYS = ["ovl_0700", "ovl_0457", "ovl_03D2", "ovl_03C9", "ovl_03C5"]

SPAN_RE = re.compile(r"^\s*-\s*\[\s*(0x[0-9A-Fa-f]+)\s*(?:,\s*(\w+)\s*(?:,\s*(\w+))?)?\s*\]",
                     re.MULTILINE)
VRAM_RE = re.compile(r"^\s*vram:\s*(0x[0-9A-Fa-f]+)", re.MULTILINE)
FUNC_RE = re.compile(r"\bfunc_([0-9A-Fa-f]{8})\b")


def symbol(ovl: str, name: str) -> str:
    return f"{ovl}__{name}"


def parse_overlay(ovl: str) -> dict:
    """VRAM window and matched `c` leaves of one overlay config."""
    text = (OVL_CONFIG_DIR / f"{ovl}.yaml").read_text(encoding="utf-8")
    vram_m = VRAM_RE.search(text)
    if not vram_m:
        raise SystemExit(f"gen_overlay_ports: no vram in {ovl}.yaml")
    vram = int(vram_m.group(1), 16)
    spans = [(int(m.group(1), 16), m.group(2), m.group(3))
             for m in SPAN_RE.finditer(text)]
    offsets = sorted({o for o, _, _ in spans})
    leaves: dict[str, dict] = {}
    for off, kind, name in spans:
        if kind != "c" or not name:
            continue
        end = next((o for o in offsets if o > off), None)
        if end is None:
            raise SystemExit(f"gen_overlay_ports: cannot size {ovl}/{name}")
        leaves[name] = {"name": name, "file_offset": off,
                        "file_size": end - off, "vram": vram + off,
                        "words": (end - off) // 4}
    for name, (off, size) in nonmatching_leaves().get(ovl, {}).items():
        if name in leaves:
            raise SystemExit(f"gen_overlay_ports: {ovl}/{name} is both a YAML c leaf and NON_MATCHING")
        leaves[name] = {"name": name, "file_offset": off, "file_size": size,
                        "vram": vram + off, "words": size // 4, "nonmatching": True}
    return {"id": ovl, "vram": vram, "end": vram + offsets[-1], "leaves": leaves}


def all_windows() -> list[tuple[str, int, int]]:
    wins = []
    for path in sorted(OVL_CONFIG_DIR.glob("*.yaml")):
        if path.name == "manifest.yaml":
            continue
        try:
            o = parse_overlay(path.stem)
        except SystemExit:
            continue
        wins.append((o["id"], o["vram"], o["end"]))
    return wins


def in_window(ovl: dict, name: str) -> bool:
    a = int(name[5:], 16)
    return ovl["vram"] <= a < ovl["end"]


def overlay_derived_sigs(ovl: str, out_dir: Path) -> dict[str, dict]:
    """Host signatures of this overlay's already-emitted TUs, keyed by the
    retail name (the rename macro makes callers use `<ovl>__func_X`)."""
    sigs: dict[str, dict] = {}
    if not out_dir.is_dir():
        return sigs
    for path in sorted(out_dir.glob("func_*_port.c")):
        name = path.name[: -len("_port.c")]
        text = path.read_text(encoding="utf-8", errors="replace")
        if f"decomp-source-overlay: {ovl}/{name}" not in text:
            continue
        stripped = g.strip_comments(text)
        sym = symbol(ovl, name)
        m = re.search(rf"^([A-Za-z_][\w \t\*]*?\b){re.escape(name)}\s*\(([^;{{]*?)\)\s*\{{",
                      stripped, re.MULTILINE)
        if m is None:
            continue
        ret = re.sub(r"\s+", " ", m.group(1)).strip()
        params = re.sub(r"\s+", " ", m.group(2)).strip()
        sigs[name] = {"ret": ret, "params": g.split_top(params),
                      "decl": f"{ret} {name}({params or 'void'});", "sym": sym}
    return sigs


# Same-overlay callees no TU of the overlay emits, bound to the one hand port
# that implements THAT overlay's code.  Without an entry here an in-window
# callee stays a loud boundary: the rooms, ovl_03D2 and ovl_0700 reuse the
# 0x8018EFxx addresses for different code, so a plain pc_port name alone does
# not say which overlay it implements.  Identity evidence per entry
# (docs/port/NATIVE_COMPLETION.md, "Intra-overlay boundaries"):
#   movie_overlay_port.c  header SHA-256 5ddd18d8... = the ovl_03C5 blob
#   func_80191B64_port.c  ovl_03D2's data (0x801D11AC, 0x801D0DBD, 0x801D0DE0)
#   func_8018F344_port.c  8018F344..8018F55C SHA-256 f0c1895a... (ACTIVE_HANDOFF)
#   func_80191DE8_port.c  header SHA-256 c51e36c2... = the ovl_0700 blob
#   func_80197BA0_port.c  header SHA-256 8a823f37... = ovl_0700 80197BA0..8019959C
HAND_OVERLAY_PORTS: dict[tuple[str, str], str] = {
    ("ovl_03C5", "func_80121270"): "pc_port/game/boot/movie_overlay_port.c",
    ("ovl_03D2", "func_80191B64"): "pc_port/game/boot/func_80191B64_port.c",
    ("ovl_0700", "func_8018F344"): "pc_port/game/boot/func_8018F344_port.c",
    ("ovl_0700", "func_80193B5C"): "pc_port/game/boot/func_80191DE8_port.c",
    ("ovl_0700", "func_80197BA0"): "pc_port/game/boot/func_80197BA0_port.c",
}


def hand_overlay_sigs(ovl: dict, defs: dict[str, set[str]],
                      emitted: set[str] = frozenset()) -> dict[str, dict]:
    """Host signatures of this overlay's HAND_OVERLAY_PORTS bindings, keyed by
    the retail name.  A callee the overlay emits is skipped (its generated TU
    wins).  A stale binding stops the run: the file no longer holds the only
    plain, non-static definition, or the name is outside the window."""
    out: dict[str, dict] = {}
    for (ov, name), rel in sorted(HAND_OVERLAY_PORTS.items()):
        if ov != ovl["id"] or name in emitted:
            continue
        where = sorted(defs.get(name, ()))
        if not in_window(ovl, name) or where != [rel]:
            raise SystemExit(f"gen_overlay_ports: stale hand binding {ov}:{name} -> {rel} "
                             f"(pc_port defines it in {where or 'no file'})")
        text = g.strip_comments((REPO_ROOT / rel).read_text(encoding="utf-8", errors="replace"))
        m = re.search(rf"^([A-Za-z_][\w \t\*]*?\b){name}\s*\(([^;{{]*?)\)\s*\{{",
                      text, re.MULTILINE)
        if m is None:
            raise SystemExit(f"gen_overlay_ports: no definition of {name} in {rel}")
        ret = re.sub(r"\s+", " ", m.group(1)).strip()
        params = re.sub(r"\s+", " ", m.group(2)).strip()
        out[name] = {"ret": ret, "params": g.split_top(params),
                     "decl": f"{ret} {name}({params or 'void'});"}
    return out


def post_process(ovl: dict, name: str, text: str, local_emitted: set[str],
                 windows: list[tuple[str, int, int]],
                 rename: dict[str, str] | None = None,
                 members: list[str] | None = None) -> str:
    ov = ovl["id"]
    prov = f"decomp-source-overlay: {ov}/{name}"
    for m in members or []:
        if m != ov:
            prov += f"\n * decomp-source-overlay: {m}/{name}"
    text = text.replace(f"decomp-source: {name}", prov, 1)
    text = text.replace(f"evidence: docs/evidence/{name}/REPORT.md",
                        f"evidence: docs/evidence/{ov}-{name}/REPORT.md", 1)
    text = text.replace(f"src/{name}.c", str(leaf_src(ov, name).relative_to(REPO_ROOT)))
    text = text.replace("GENERATED by tools/analysis/gen_decomp_ports.py",
                        "GENERATED by tools/analysis/gen_overlay_ports.py", 1)

    # Loud-boundary labels: overlay-local vs cross-overlay callees.
    def label(m: re.Match) -> str:
        callee = m.group(1)
        if in_window(ovl, callee):
            return f'("{ov}:{callee}"'
        a = int(callee[5:], 16)
        if any(lo <= a < hi for _, lo, hi in windows):
            return f'("overlay:{callee}"'
        return m.group(0)
    text = re.sub(r'\("(func_[0-9A-Fa-f]{8})"', label, text)

    # Rename macros for the leaf and every emitted same-overlay callee named
    # in the TU (boundary callees keep their function-like boundary macro).
    boundary = set(re.findall(r"#define (func_[0-9A-Fa-f]{8})\(\.\.\.\)", text))
    if rename is None:
        rename = {n: symbol(ov, n) for n in (local_emitted | {name})}
    names = sorted({f"func_{h}" for h in FUNC_RE.findall(text)} & set(rename))
    names = [n for n in names if n not in boundary]
    who = ov if not members or len(members) == 1 else ", ".join(members)
    block = ["/* overlay namespace: this TU is " + who + "'s " + name + " */"]
    block += [f"#define {n} {rename[n]}" for n in names]
    inc = '#include "pe_guest_decomp.h"\n'
    i = text.index(inc) + len(inc)
    return text[:i] + "\n".join(block) + "\n" + text[i:]


def run_overlay(ovl: dict, args, windows) -> tuple[int, int, list, dict, dict]:
    ov = ovl["id"]
    out_dir = REPO_ROOT / OUT_ROOT / ov
    src_dir = OVL_SRC_DIR / ov
    written = changed = 0
    drift: list[str] = []
    skipped: dict[str, str] = {}
    uncompilable: dict[str, str] = {}
    local_emitted = {p.name[: -len("_port.c")]
                     for p in out_dir.glob("func_*_port.c")} if out_dir.is_dir() else set()
    total_changed = 0
    for _round in range(8):
        defs = g.pc_port_definitions()
        # Plain pc_port names inside this window may implement another
        # overlay's code: never bind to them (except HAND_OVERLAY_PORTS).
        exe_known = {n for n in defs if not in_window(ovl, n)}
        known = exe_known | local_emitted
        protos = {n: a for n, a in g.canonical_protos().items() if not in_window(ovl, n)}
        sigs = {n: s for n, s in g.canonical_signatures().items() if not in_window(ovl, n)}
        exe_dsigs = {n: s for n, s in g.derived_signatures().items()
                     if not in_window(ovl, n) and n not in protos}
        loc_dsigs = overlay_derived_sigs(ov, out_dir)
        dsigs = {**exe_dsigs, **{n: s for n, s in loc_dsigs.items() if n in local_emitted}}
        hand = hand_overlay_sigs(ovl, defs, local_emitted)
        known |= set(hand)
        dsigs.update(hand)
        for n, s in dsigs.items():
            protos.setdefault(n, len(s["params"]))
            sigs.setdefault(n, {"ret": s["ret"], "params": s["params"]})
        # header `static inline` shims are implementations (gen_decomp_ports)
        for n, s in g.inline_shims().items():
            if not in_window(ovl, n):
                known.add(n)
                protos.setdefault(n, len([p for p in s["params"]
                                          if p.strip() not in ("", "void")]))
                sigs.setdefault(n, s)
        g._ADDR_SIGS.clear()
        g._ADDR_SIGS.update(sigs)
        g._ADDR_KNOWN.clear()
        g._ADDR_KNOWN.update(known)
        shim_macros = g.pc_port_macros()
        # host replacements: the lowering only, as in run_rooms (ovl_0457's
        # credits call ovl_039F's libpress entries, which the port reimplements)
        rnp = retail_nparams()
        repl = {n: s for n, s in host_replacements().items() if not in_window(ovl, n)}
        low_known = known | set(repl)
        low_sigs = {**sigs, **{a: {**b, "retail_nparams": rnp.get(a)} for a, b in repl.items()}}
        low_dsigs = {**dsigs, **repl}

        plans: dict[str, dict] = {}
        lowered: dict[str, str] = {}
        skipped = {}
        for name in sorted(ovl["leaves"]):
            src = leaf_src(ov, name)
            if not src.exists():
                skipped[name] = "no src file"
                continue
            text = src.read_text(encoding="utf-8", errors="replace")
            plan = g.analyze(name, text, ovl["leaves"][name], known, shim_macros,
                             protos, sigs)
            if not plan["eligible"]:
                # E21, as for room code: a pointer-model refusal is lowered to
                # explicit guest-memory C instead (guest_lower.py).
                txt = lowered_tu(name, text, ovl["leaves"][name], plan["reason"],
                                 low_known, low_sigs, low_dsigs)
                if isinstance(txt, str) and txt.startswith("/*"):
                    lowered[name] = txt
                    continue
                skipped[name] = plan["reason"] + (f"; E21: {txt}" if txt else "")
                continue
            stripped = g.strip_comments(text)
            override = [c for c in plan["callees"]
                        if c in dsigs and c != name and
                        not g.same_host_decl(stripped, c, dsigs[c])]
            plan["host_protos"] = [dsigs[c]["decl"] for c in override]
            plan["host_override"] = override
            plan["derived_all"] = sorted(dsigs)
            plans[name] = plan
        if args.check:
            return len(plans) + len(lowered), 0, [], skipped, {}

        rendered = []
        local = set(plans) | set(lowered)
        for name, plan in plans.items():
            txt = g.render(name, plan, ovl["leaves"], known, protos)
            rendered.append((name, post_process(ovl, name, txt, local, windows)))
        for name, txt in lowered.items():
            rendered.append((name, post_process(ovl, name, txt, local, windows)))
        uncompilable = {}
        if not args.no_compile_check:
            cc = g.find_compiler()
            if cc:
                uncompilable = g.compile_check(cc, rendered)
                rendered = [(n, c) for n, c in rendered if n not in uncompilable]
                # A verbatim TU the host compiler rejects gets one E21 try, as
                # in run_rooms (host types on guest pointers are what fails).
                retry = []
                for n in sorted(uncompilable):
                    if n not in plans:
                        continue
                    text = leaf_src(ov, n).read_text(encoding="utf-8", errors="replace")
                    txt = lowered_tu(n, text, ovl["leaves"][n], None, low_known, low_sigs,
                                     low_dsigs)
                    if isinstance(txt, str) and txt.startswith("/*"):
                        retry.append((n, post_process(ovl, n, txt, local, windows)))
                if retry:
                    unc2 = g.compile_check(cc, retry)
                    for n, c in retry:
                        if n not in unc2:
                            rendered.append((n, c))
                            uncompilable.pop(n, None)
        emitted = {n for n, _ in rendered}

        out_dir.mkdir(parents=True, exist_ok=True)
        written = changed = 0
        drift = []
        for name, content in rendered:
            target = out_dir / f"{name}_port.c"
            if target.exists() and target.read_text(encoding="utf-8") == content:
                written += 1
                continue
            if args.verify:
                drift.append(str(target.relative_to(REPO_ROOT)))
                continue
            target.write_text(content, encoding="utf-8")
            written += 1
            changed += 1
        for path in sorted(out_dir.glob("*_port.c")):
            if path.name[: -len("_port.c")] not in emitted:
                if args.verify:
                    drift.append(str(path.relative_to(REPO_ROOT)) + " (orphan)")
                else:
                    path.unlink()
                    changed += 1
        stable = emitted == local_emitted
        local_emitted = emitted
        total_changed += changed       # every pass's writes, not just the last (converged) one
        if args.verify or (changed == 0 and stable):
            break
    return written, total_changed, drift, skipped, uncompilable


# ── rooms: one TU per deduplicated body (step 4) ─────────────────────────
ROOMS_DIR = OUT_ROOT / "rooms"


def room_ids() -> list[str]:
    return [w[0] for w in all_windows() if w[0].startswith("room_")]


def room_groups(rooms: list[dict]) -> dict[tuple[str, str], str]:
    """(room, name) -> group key.  Key = body text refined by the identity of
    same-room callees (iterated to a fixed point); a same-room callee that is
    not a matched leaf is room-specific, so leaves calling it never merge."""
    import hashlib
    body: dict[tuple[str, str], str] = {}
    callees: dict[tuple[str, str], list[tuple]] = {}
    for ovl in rooms:
        r = ovl["id"]
        for n in ovl["leaves"]:
            p = leaf_src(r, n)
            if not p.exists():
                continue
            t = g.strip_comments(p.read_text(encoding="utf-8", errors="replace"))
            body[(r, n)] = hashlib.sha1(re.sub(r"\s+", " ", t).strip().encode()).hexdigest()
            cs = []
            for c, _ in g.call_args(t):
                if c == n:
                    continue
                if c in ovl["leaves"]:
                    cs.append(("L", c))
                elif in_window(ovl, c):
                    cs.append(("A", r, c))
                else:
                    cs.append(("X", c))
            callees[(r, n)] = sorted(set(cs))
    key = dict(body)
    for _ in range(12):
        new = {}
        for k in key:
            r = k[0]
            parts = [key[k]]
            for c in callees[k]:
                parts.append(repr(("L", key.get((r, c[1]), "?"))) if c[0] == "L" else repr(c))
            new[k] = hashlib.sha1("|".join(parts).encode()).hexdigest()
        # refinement only splits groups: stop when the partition stops growing
        done = len(set(new.values())) == len(set(key.values()))
        key = new
        if done:
            break
    return key


def group_symbol(key: str, name: str) -> str:
    return f"room_{key[:8]}__{name}"


# E21: refusal reasons that are about the host/guest pointer model, not about
# the leaf itself.  Such a leaf is lowered to explicit guest-memory C instead
# (tools/analysis/guest_lower.py); anything the lowering cannot express keeps
# the original refusal (plus the lowering's own reason).
LOWER_REASONS = gl.LOWER_REASONS


# Host replacements (`/* host-replacement: func_X */` in pe_port_compat.h):
# gen_decomp_ports owns the parser, the EXE E21 lowering uses it too.
HOST_REPL_RE = g.HOST_REPL_RE
host_replacements = g.host_replacements


_RETAIL_NPARAMS: dict[str, int] | None = None
_DEF_RE = r"\b{}\s*\(((?:[^()]|\([^()]*\))*)\)\s*\{{"


def retail_nparams() -> dict[str, int]:
    """func_* -> parameter count of its matched EXE definition (src/func_*.c),
    the retail authority E21c compares the host prototype against."""
    global _RETAIL_NPARAMS
    if _RETAIL_NPARAMS is None:
        out: dict[str, int] = {}
        for pth in sorted((REPO_ROOT / "src").glob("func_*.c")):
            n = pth.stem
            txt = g.strip_comments(pth.read_text(encoding="utf-8", errors="replace"))
            m = re.search(_DEF_RE.format(re.escape(n)), txt)
            if not m:
                continue
            prm = [x for x in g.split_top(m.group(1)) if x.strip() not in ("", "void")]
            if any(x.strip() == "..." for x in prm):
                continue
            out[n] = len(prm)
        _RETAIL_NPARAMS = out
    return _RETAIL_NPARAMS


def lowered_tu(n: str, text: str, leaf: dict, reason: str | None, known: set[str],
               sigs: dict[str, dict], dsigs: dict[str, dict]) -> str | None:
    """Rendered E21 TU text, the lowering's refusal reason, or None."""
    if reason is not None and not reason.startswith(LOWER_REASONS):
        return None
    try:
        res = gl.lower_leaf(n, text, known, sigs, strip_asm=g.strip_era_asm)
    except gl.LowerError as exc:
        return str(exc)
    protos = []
    for c in res["callees"]:
        sg = dsigs.get(c) or sigs.get(c)
        if sg is None:
            return f"no host prototype for {c}"
        prm = ", ".join(sg["params"]) or "void"
        protos.append(sg.get("decl") or f"{sg['ret']} {c}({prm});")
    return gl.render_lowered(n, leaf, res, protos, f"docs/evidence/{n}/REPORT.md")


def run_rooms(args, windows) -> tuple[int, int, dict, dict]:
    rooms = [parse_overlay(r) for r in room_ids()]
    by_id = {o["id"]: o for o in rooms}
    key = room_groups(rooms)
    groups: dict[str, list[tuple[str, str]]] = {}
    for k, gk in key.items():
        groups.setdefault(gk, []).append(k)
    out_dir = REPO_ROOT / ROOMS_DIR
    existing = {p.name for p in out_dir.glob("*_port.c")} if out_dir.is_dir() else set()
    emitted_keys = {f.split("__")[1][:8] for f in existing if "__" in f}
    skipped: dict[str, str] = {}
    uncompilable: dict[str, str] = {}
    written = changed = 0
    total_changed = 0
    for _round in range(8):
        defs = g.pc_port_definitions()
        canon_sigs = g.canonical_signatures()
        canon_protos = g.canonical_protos()
        exe_dsigs = g.derived_signatures()
        room_dsigs: dict[str, dict] = {}          # group symbol -> sig
        if out_dir.is_dir():
            for pth in out_dir.glob("*_port.c"):
                txt = g.strip_comments(pth.read_text(encoding="utf-8", errors="replace"))
                nm = pth.name.split("__")[0]
                m = re.search(rf"^([A-Za-z_][\w \t\*]*?\b){nm}\s*\(([^;{{]*?)\)\s*\{{", txt, re.MULTILINE)
                if m:
                    ret = re.sub(r"\s+", " ", m.group(1)).strip()
                    prm = re.sub(r"\s+", " ", m.group(2)).strip()
                    room_dsigs[pth.name[:-len("_port.c")]] = {
                        "ret": ret, "params": g.split_top(prm),
                        "decl": f"{ret} {nm}({prm or 'void'});"}
        rendered = []
        plans_emit = 0
        lowered = 0
        verbatim: dict[str, tuple] = {}
        skipped = {}
        for gk, members in sorted(groups.items()):
            r, n = min(members)
            ovl = by_id[r]
            rename = {}
            local_sigs = {}
            for ln in ovl["leaves"]:
                lk = key.get((r, ln))
                if lk and lk[:8] in emitted_keys | {gk[:8]}:
                    rename[ln] = group_symbol(lk, ln)
                    ds = room_dsigs.get(f"{ln}__{lk[:8]}")
                    if ds:
                        local_sigs[ln] = ds
            known = {x for x in defs if not in_window(ovl, x)} | set(rename)
            protos = {a: b for a, b in canon_protos.items() if not in_window(ovl, a)}
            sigs = {a: b for a, b in canon_sigs.items() if not in_window(ovl, a)}
            dsigs = {a: b for a, b in exe_dsigs.items() if not in_window(ovl, a) and a not in protos}
            dsigs.update(local_sigs)
            for a, b in dsigs.items():
                protos.setdefault(a, len(b["params"]))
                sigs.setdefault(a, {"ret": b["ret"], "params": b["params"]})
            # header `static inline` shims (VSync -> HostFB_VSync, ...) are
            # implementations, as gen_decomp_ports already treats them
            for a, b in g.inline_shims().items():
                if not in_window(ovl, a):
                    known.add(a)
                    protos.setdefault(a, len([p for p in b["params"]
                                              if p.strip() not in ("", "void")]))
                    sigs.setdefault(a, b)
            rnp = retail_nparams()
            for a in list(sigs):
                if a in rnp:
                    sigs[a] = {**sigs[a], "retail_nparams": rnp[a]}
            # host replacements: the lowering only (a verbatim TU would call
            # func_X by name, which the port never defines)
            repl = host_replacements()
            low_known = known | set(repl)
            low_sigs = {**sigs, **{a: {**b, "retail_nparams": rnp.get(a)} for a, b in repl.items()}}
            low_dsigs = {**dsigs, **repl}
            g._ADDR_SIGS.clear(); g._ADDR_SIGS.update(sigs)
            g._ADDR_KNOWN.clear(); g._ADDR_KNOWN.update(known)
            text = leaf_src(r, n).read_text(encoding="utf-8", errors="replace")
            plan = g.analyze(n, text, ovl["leaves"][n], known, g.pc_port_macros(), protos, sigs)
            if not plan["eligible"]:
                txt = lowered_tu(n, text, ovl["leaves"][n], plan["reason"], low_known,
                                 low_sigs, low_dsigs)
                if isinstance(txt, str) and txt.startswith("/*"):
                    lowered += 1
                    if args.check:
                        plans_emit += 1
                        continue
                    mem_rooms = sorted({m[0] for m in members})
                    rendered.append((f"{n}__{gk[:8]}", post_process(
                        ovl, n, txt, set(), windows, rename=rename, members=mem_rooms)))
                    continue
                skipped[f"{r}/{n}"] = plan["reason"] + (f"; E21: {txt}" if txt else "")
                continue
            stripped = g.strip_comments(text)
            override = [c for c in plan["callees"] if c in dsigs and c != n
                        and not g.same_host_decl(stripped, c, dsigs[c])]
            plan["host_protos"] = [dsigs[c]["decl"] for c in override]
            plan["host_override"] = override
            plan["derived_all"] = sorted(dsigs)
            plans_emit += 1
            if args.check:
                continue
            txt = g.render(n, plan, ovl["leaves"], known, protos)
            mem_rooms = sorted({m[0] for m in members})
            rendered.append((f"{n}__{gk[:8]}", post_process(
                ovl, n, txt, set(), windows, rename=rename, members=mem_rooms)))
            verbatim[f"{n}__{gk[:8]}"] = (n, text, ovl, rename, mem_rooms, known, sigs, dsigs)
        if args.check:
            return plans_emit, 0, skipped, {}
        uncompilable = {}
        if not args.no_compile_check:
            cc = g.find_compiler()
            if cc:
                uncompilable = g.compile_check(cc, rendered)
                rendered = [(k, c) for k, c in rendered if k not in uncompilable]
                # A verbatim TU the host compiler rejects gets one E21 try.
                retry = []
                for k in sorted(uncompilable):
                    if k not in verbatim:
                        continue
                    n, text, ovl, rename, mem_rooms, known, sigs, dsigs = verbatim[k]
                    txt = lowered_tu(n, text, ovl["leaves"][n], None, known, sigs, dsigs)
                    if isinstance(txt, str) and txt.startswith("/*"):
                        retry.append((k, post_process(ovl, n, txt, set(), windows,
                                                      rename=rename, members=mem_rooms)))
                if retry:
                    unc2 = g.compile_check(cc, retry)
                    for k, c in retry:
                        if k not in unc2:
                            rendered.append((k, c))
                            uncompilable.pop(k, None)
        new_keys = {k.split("__")[1] for k, _ in rendered}
        out_dir.mkdir(parents=True, exist_ok=True)
        changed = written = 0
        names = set()
        for k, content in rendered:
            target = out_dir / f"{k}_port.c"
            names.add(target.name)
            if target.exists() and target.read_text(encoding="utf-8") == content:
                written += 1
                continue
            if args.verify:
                changed += 1
                continue
            target.write_text(content, encoding="utf-8")
            written += 1
            changed += 1
        for pth in out_dir.glob("*_port.c"):
            if pth.name not in names:
                if not args.verify:
                    pth.unlink()
                changed += 1
        stable = new_keys == emitted_keys
        emitted_keys = new_keys
        total_changed += changed       # every pass's writes, not just the last (converged) one
        if args.verify or (changed == 0 and stable):
            break
    return written, total_changed, skipped, uncompilable


# ── guest-code dispatch registry (step 3) ────────────────────────────────
DISPATCH_DIR = OUT_ROOT / "_dispatch"
INT_TYPE_WORDS = {"int", "char", "short", "long", "unsigned", "signed", "pe_addr_t",
                  "uint32_t", "int32_t", "uint16_t", "int16_t", "uint8_t", "int8_t",
                  "uintptr_t", "size_t", "u32", "s32", "u16", "s16", "u8", "s8",
                  "const", "volatile", "register"}


def param_type(p: str) -> str | None:
    """Type of one parameter declaration if it is an integer type, else None."""
    p = p.strip()
    if p in ("", "void"):
        return None
    if "*" in p or "[" in p or "(" in p:
        return None
    words = p.split()
    if len(words) >= 2 and words[-1] not in INT_TYPE_WORDS:
        words = words[:-1]                       # drop the parameter name
    if not words or any(w not in INT_TYPE_WORDS for w in words):
        return None
    return " ".join(w for w in words if w not in ("const", "volatile", "register"))


THUNK_MAX_ARGS = 8   # PE_GUESTCALL_MAX_ARGS: a0-a3 plus four O32 stack arguments


def thunkable(sig: dict) -> list[str] | None:
    """Parameter types of an all-integer host signature (<= 8 params), or None.

    Up to 4 parameters get a PeGuestThunk; 5..8 (O32 stack arguments) get a
    PeGuestThunk8, which PE_GuestCallN calls only when the site passes them."""
    ret = sig["ret"].replace("extern", "").replace("static", "").strip()
    if ret != "void" and (("*" in ret) or any(w not in INT_TYPE_WORDS for w in ret.split())):
        return None
    params = [p for p in sig["params"] if p.strip() not in ("", "void")]
    if len(params) > THUNK_MAX_ARGS:
        return None
    types = [param_type(p) for p in params]
    if any(t is None for t in types):
        return None
    return types


def psyq_addresses() -> set[int]:
    """EXE addresses classified as Psy-Q (configs/USA/psyq_classification.tsv,
    the psyq_port_guard authority)."""
    import psyq_port_guard as pg
    return set(pg.load_classification(REPO_ROOT / "configs/USA/psyq_classification.tsv"))


def fnv1a(data: bytes) -> int:
    h = 0x811C9DC5
    for b in data:
        h ^= b
        h = (h * 0x01000193) & 0xFFFFFFFF
    return h


def closure_fp(ovl: dict, data: bytes, name: str) -> int:
    """FNV-1a over (vma, bytes) of a leaf and every same-room function its
    retail code reaches by absolute `j`/`jal` (transitively). Two registry entries with
    the same closure run byte-identical code all the way down, so either
    generated TU is the same native code for them (pe_guestcode.c)."""
    by_vma = {lf["vram"]: lf for lf in ovl["leaves"].values()}
    lo, hi = ovl["vram"], ovl["vram"] + len(data)
    seen: set[int] = set()
    todo = [ovl["leaves"][name]["vram"]]
    parts = []
    while todo:
        v = todo.pop()
        if v in seen:
            continue
        seen.add(v)
        lf = by_vma.get(v)
        if lf is None:                      # unmatched same-room callee: its address only
            parts.append(v.to_bytes(4, "little"))
            continue
        body = data[lf["file_offset"]:lf["file_offset"] + lf["file_size"]]
        parts.append(v.to_bytes(4, "little") + body)
        for k in range(0, len(body) - 3, 4):
            w = int.from_bytes(body[k:k + 4], "little")
            if w >> 26 in (2, 3):           # j tail calls as well as jal
                t = ((v + k + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
                if lo <= t < hi:
                    todo.append(t)
    return fnv1a(b"".join(sorted(parts)))


def main_checkout(root: Path) -> Path | None:
    """The main checkout of the repository that `root` belongs to.

    A git worktree shares its object store with the main checkout; its own
    `build/` (git-ignored, where extracted blobs live) starts empty.  `git
    rev-parse --git-common-dir` names the shared `.git` directory, whose parent
    is the main checkout.  None when `root` is not in a git repository."""
    try:
        out = subprocess.run(["git", "-C", str(root), "rev-parse", "--git-common-dir"],
                             capture_output=True, text=True, check=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return None
    if not out:
        return None
    common = Path(out)
    if not common.is_absolute():
        common = root / common
    return common.resolve().parent


def blob_roots() -> list[Path]:
    """Checkouts searched for extracted blobs: this one, then the main one."""
    roots = [REPO_ROOT.resolve()]
    main = main_checkout(REPO_ROOT)
    if main is not None and main not in roots:
        roots.append(main)
    return roots


def blob_path(ovl: str, roots: list[Path] | None = None) -> Path | None:
    """The extracted retail blob of `ovl` (its config's `target_path`).

    Looked up in this checkout first and then in the main checkout, so a git
    worktree (whose git-ignored build/ has no extracted data) still finds the
    blobs and the dispatch registry keeps its fingerprints."""
    text = (OVL_CONFIG_DIR / f"{ovl}.yaml").read_text(encoding="utf-8")
    m = re.search(r"^\s*target_path:\s*(\S+)", text, re.MULTILINE)
    if not m:
        return None
    rel = m.group(1)
    cands = [(OVL_CONFIG_DIR / rel).resolve()]   # base_path ../../.. = repo root
    for r in (roots if roots is not None else blob_roots()):
        cands.append((r / rel.replace("../", "")).resolve())
    for p in cands:
        if p.exists():
            return p
    return None


class MissingBlobError(SystemExit):
    """An overlay with registry entries has no extracted blob to fingerprint."""


def require_blobs(missing: dict[str, int], roots: list[Path]) -> None:
    """Fail loudly instead of writing fingerprint 0 for an overlay.

    Overlay code resolves at run time only through its fingerprint (nothing
    declares residency), so a 0 fingerprint silently makes every generated TU
    of that overlay unreachable."""
    if not missing:
        return
    lines = [f"  {ovl}: {n} registry entr{'y' if n == 1 else 'ies'}"
             for ovl, n in sorted(missing.items())]
    raise MissingBlobError(
        "gen_overlay_ports: ERROR: no extracted blob for "
        f"{len(missing)} overlay(s) with registry entries; refusing to write "
        "fingerprint 0 (their generated TUs would never resolve):\n"
        + "\n".join(lines)
        + "\n  searched: " + ", ".join(str(r) for r in roots)
        + "\n  fix: extract them (python3 tools/extract/peimg.py extract / room <id>) "
          "in the main checkout.")


def overlay_facts(ovl: str) -> tuple[str | None, int]:
    """(committed sha1, load size) of an overlay blob."""
    text = (OVL_CONFIG_DIR / f"{ovl}.yaml").read_text(encoding="utf-8")
    sha = re.search(r"^sha1:\s*([0-9a-f]{40})", text, re.MULTILINE)
    size = 0
    man = (OVL_CONFIG_DIR / "manifest.yaml").read_text(encoding="utf-8")
    mm = re.search(rf"^  {re.escape(ovl)}:\n((?:    .*\n)+)", man, re.MULTILINE)
    if mm:
        sm = re.search(r"^\s*size:\s*(0x[0-9A-Fa-f]+)", mm.group(1), re.MULTILINE)
        if sm:
            size = int(sm.group(1), 16)
    if not size:
        cm = re.search(r"size (0x[0-9A-Fa-f]+)", text)
        if cm:
            size = int(cm.group(1), 16)
    return (sha.group(1) if sha else None), size


def static_defined(name: str, files: set[str]) -> bool:
    for f in files:
        t = (REPO_ROOT / f).read_text(encoding="utf-8", errors="replace")
        if re.search(rf"^\s*static\b[^;{{]*\b{re.escape(name)}\s*\(", t, re.MULTILINE):
            return True
    return False


# Behavior-equivalent host callbacks, independently checked against the raw
# retail bodies. These are NOT matched-src leaves and do not affect lowering.
# Evidence: docs/evidence/pe-liberate-native-20261023/REPORT.md.
NATIVE_OVERLAY_CALLBACKS = {
    ("ovl_03C9", "func_801EDC44"): {
        "file": "pc_port/game/boot/func_801EDC44_liberate_native.c",
        "offset": 0x44C, "size": 0x950,
        "sha1": "1d0e972cf9446dc700c82a8597292ced45958ef4",
        "body_sha256": "dbfb30388b1d5ac2ebd9f882f9fff5fee777e089f59fe0e107cf80e381a43e74",
    },
    ("ovl_03C9", "func_801EEBC8"): {
        "file": "pc_port/game/boot/func_801EEBC8_liberate_native.c",
        "offset": 0x13D0, "size": 0x998,
        "sha1": "1d0e972cf9446dc700c82a8597292ced45958ef4",
        "body_sha256": "6abb58634fd7ba34d9af5cdeb419ddfd70e646b7bb5b1fd18d96bda7561d07de",
    },
    # Pool child of func_801F0690 (effect descriptors 0x6E-0x71): mode 0 hands it
    # to func_800CE560, and the effect manager's pool ticks call it.
    # Evidence: docs/evidence/ovl03c9-effect-801F0068-20261024/REPORT.md.
    ("ovl_03C9", "func_801F0068"): {
        "file": "pc_port/game/boot/func_801F0068_effect_native.c",
        "offset": 0x2870, "size": 0x628,
        "sha1": "1d0e972cf9446dc700c82a8597292ced45958ef4",
        "body_sha256": "cef4859722e10424f2da9c40f24630603e19f7aad78e48442570d1ffa22c5d3c",
    },
}


def native_callback_entries(ovl: dict, defs: dict[str, set[str]],
                            data: bytes | None, emitted: set[str]) -> list[dict]:
    """Exact overlay identity + span + sole nonstatic typed definition.

    Missing/stale/changed metadata is a generation failure. A future matched
    conversion must remove its manual entry explicitly; it cannot shadow it.
    Runtime resolution still uses the existing fingerprint-checked registry.
    """
    import hashlib
    entries = []
    for (ov, name), binding in sorted(NATIVE_OVERLAY_CALLBACKS.items()):
        if ov != ovl["id"]:
            continue
        off, size = binding["offset"], binding["size"]
        addr = int(name[5:], 16)
        if name in emitted or name in ovl["leaves"]:
            raise SystemExit(f"gen_overlay_ports: duplicate native callback {ov}:{name}")
        if not in_window(ovl, name) or off < 0 or size <= 0 or (off | size) & 3 or addr != ovl["vram"] + off or addr + size > ovl["end"]:
            raise SystemExit(f"gen_overlay_ports: invalid native callback span {ov}:{name}")
        if data is None or off + size > len(data) or hashlib.sha1(data).hexdigest() != binding["sha1"] or hashlib.sha256(data[off:off + size]).hexdigest() != binding["body_sha256"]:
            raise SystemExit(f"gen_overlay_ports: native callback retail identity mismatch {ov}:{name}")
        rel = binding["file"]
        if defs.get(name) != {rel} or static_defined(name, {rel}):
            raise SystemExit(f"gen_overlay_ports: stale native callback definition {ov}:{name}")
        text = g.strip_comments((REPO_ROOT / rel).read_text(encoding="utf-8"))
        if not re.search(rf"^int {name}\(int32_t mode, pe_addr_t data\)\s*\{{", text, re.MULTILINE):
            raise SystemExit(f"gen_overlay_ports: native callback signature mismatch {ov}:{name}")
        entries.append({"vma": addr, "size": size, "fp": fnv1a(data[off:off + size]),
                        "closure": 0, "symbol": name, "types": ["int32_t", "pe_addr_t"],
                        "ret": "int", "decl": f"int {name}(int32_t mode, pe_addr_t data);"})
    return entries


def render_dispatch(tables: list[dict]) -> str:
    out = ["/*",
           " * GENERATED by tools/analysis/gen_overlay_ports.py — guest-code dispatch",
           " * registry (docs/ai_context/PC_PORT_FROM_DECOMP.md §3b step 3).  One",
           " * uniform thunk per function with an all-integer host signature; tables",
           " * sorted by retail VMA.  Fingerprints are FNV-1a hashes of the retail",
           " * bytes computed from the locally extracted blobs (0 = not available).",
           " */",
           '#include "pe_guest_decomp.h"',
           '#include "pe_plat/cd_stream.h"',
           '#include "pe_callback.h"',
           ""]
    seen_decl: set[str] = set()
    for t in tables:
        for e in t["entries"]:
            if e.get("decl") and e["decl"] not in seen_decl:
                seen_decl.add(e["decl"])
                out.append(e["decl"])
    out.append("")
    n = 0
    for t in tables:
        for e in t["entries"]:
            args = ", ".join(f"({ty})a{i}" for i, ty in enumerate(e["types"]))
            call = f"{e['symbol']}({args})"
            body = (f"{call}; return 0;" if e["ret"] == "void"
                    else f"return (int){call};")
            # The matched C declaration drops the live retail v0 result.
            # The native callback authority returns that previous slot value.
            if t["id"] == "exe" and e["vma"] == 0x80074478:
                body = "return (int)PE_Callback_SetSlot((uint32_t)a0, (pe_addr_t)a1);"
            e["thunk"] = f"pe_gct_{n}"
            if len(e["types"]) <= 4:
                out.append(f"static int pe_gct_{n}(uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)"
                           f" {{ (void)a0; (void)a1; (void)a2; (void)a3; {body} }}")
            else:
                prm = ", ".join(f"uintptr_t a{i}" for i in range(THUNK_MAX_ARGS))
                unused = " ".join(f"(void)a{i};" for i in range(len(e["types"]), THUNK_MAX_ARGS))
                out.append(f"static int pe_gct_{n}({prm}) {{ {unused} {body} }}")
            n += 1
    out.append("")
    for t in tables:
        cname = "pe_gc_" + re.sub(r"\W", "_", t["id"])
        out.append(f"static const PeGuestCodeEntry {cname}_e[] = {{")
        for e in sorted(t["entries"], key=lambda e: e["vma"]):
            if len(e["types"]) <= 4:
                t4, t8, na = e["thunk"], "0", 0
            else:
                t4, t8, na = "0", e["thunk"], len(e["types"])
            out.append(f"    {{0x{e['vma']:08X}u, 0x{e['size']:X}u, 0x{e['fp']:08X}u, "
                       f"{t4}, \"{e['symbol']}\", 0x{e.get('closure', 0):08X}u, {t8}, {na}u}},")
        if not t["entries"]:
            out.append("    {0u, 0u, 0u, 0, 0, 0u, 0, 0u},")
        out.append("};")
        sha = f'"{t["sha1"]}"' if t["sha1"] else "0"
        out.append(f"static const PeGuestCodeTable {cname} = {{\"{t['id']}\", "
                   f"0x{t['lo']:08X}u, 0x{t['hi']:08X}u, {sha}, 0x{t['load_size']:X}u, "
                   f"{cname}_e, {len(t['entries'])}u}};")
    out.append("")
    out.append("static const PeGuestCodeTable *const pe_gc_tables[] = {")
    for t in tables:
        out.append("    &pe_gc_" + re.sub(r"\W", "_", t["id"]) + ",")
    out.append("};")
    out.append("")
    out.append("void PE_GuestCode_InstallGenerated(void)")
    out.append("{")
    out.append(f"    PE_GuestCode_SetRegistry(pe_gc_tables, {len(tables)}u);")
    out.append("}")
    return "\n".join(out) + "\n"


def build_dispatch(args) -> tuple[int, list[str]]:
    """Write pc_port/game/decomp_ovl/_dispatch/zz_dispatch.c."""
    tables: list[dict] = []
    # EXE: generated EXE leaves + canonical-header functions with a
    # (non-static) pc_port definition.
    matched = g.parse_matched()
    exe_hi = max(m["vram"] + m["file_size"] for m in matched.values())
    defs = g.pc_port_definitions()
    canon = g.canonical_signatures()
    dsigs = g.derived_signatures()
    entries = []
    psyq = psyq_addresses()
    for name in sorted(set(defs)):
        a = int(name[5:], 16)
        if not (0x80010000 <= a < exe_hi):
            continue
        sig = canon.get(name) or dsigs.get(name)
        if not sig:
            continue
        types = thunkable(sig)
        if types is None or static_defined(name, defs[name]):
            continue
        if len(types) > 4 and a in psyq:
            # A thunk links its target: a Psy-Q-classified body must not be
            # pulled into the port by the registry (psyq_port_guard); a guest
            # call through a pointer to it stays a loud boundary.
            continue
        decl = None if name in canon else dsigs[name]["decl"]
        size = matched[name]["file_size"] if name in matched else 4
        entries.append({"vma": a, "size": size, "fp": 0, "symbol": name,
                        "types": types, "ret": sig["ret"].strip(), "decl": decl})
    tables.append({"id": "exe", "lo": 0x80010000, "hi": exe_hi, "sha1": None,
                   "load_size": 0, "entries": entries})
    # Overlays: every configured overlay (residency needs the window + facts),
    # with entries for the emitted leaves.
    # room groups: every member room gets an entry pointing at the group TU
    room_entries: dict[str, dict[str, dict]] = {}
    rdir = REPO_ROOT / ROOMS_DIR
    if rdir.is_dir():
        for pth in sorted(rdir.glob("*_port.c")):
            txt = pth.read_text(encoding="utf-8", errors="replace")
            nm, gkey = pth.name[:-len("_port.c")].split("__")
            st = g.strip_comments(txt)
            m = re.search(rf"^([A-Za-z_][\w \t\*]*?\b){nm}\s*\(([^;{{]*?)\)\s*\{{", st, re.MULTILINE)
            if not m:
                continue
            ret = re.sub(r"\s+", " ", m.group(1)).strip()
            prm = re.sub(r"\s+", " ", m.group(2)).strip()
            sig = {"ret": ret, "params": g.split_top(prm), "sym": f"room_{gkey}__{nm}"}
            for rm in re.findall(r"decomp-source-overlay:\s*(\w+)/" + nm, txt):
                room_entries.setdefault(rm, {})[nm] = sig
    roots = blob_roots()
    missing: dict[str, int] = {}
    for ovl_id, lo, hi in all_windows():
        ovl = parse_overlay(ovl_id)
        out_dir = REPO_ROOT / OUT_ROOT / ovl_id
        sigs = overlay_derived_sigs(ovl_id, out_dir)
        sigs.update(room_entries.get(ovl_id, {}))
        blob = blob_path(ovl_id, roots)
        data = blob.read_bytes() if blob else None
        sha, load_size = overlay_facts(ovl_id)
        ents = []
        for name, sig in sorted(sigs.items()):
            types = thunkable(sig)
            if types is None:
                continue
            leaf = ovl["leaves"].get(name)
            if not leaf:
                continue
            if data is None:
                missing[ovl_id] = missing.get(ovl_id, 0) + 1
                continue
            off, size = leaf["file_offset"], leaf["file_size"]
            if off + size > len(data):
                raise SystemExit(f"gen_overlay_ports: ERROR: {ovl_id}/{name} "
                                 f"[{off:#x},+{size:#x}) lies past the end of {blob} "
                                 f"({len(data):#x} bytes)")
            fp = fnv1a(data[off:off + size])
            cl = closure_fp(ovl, data, name) if ovl_id.startswith("room_") else 0
            sym = sig.get("sym") or symbol(ovl_id, name)
            params = ", ".join(sig["params"]) or "void"
            ents.append({"vma": leaf["vram"], "size": leaf["file_size"], "fp": fp, "closure": cl,
                         "symbol": sym, "types": types, "ret": sig["ret"].strip(),
                         "decl": f"{sig['ret']} {sym}({params});"})
        ents.extend(native_callback_entries(ovl, defs, data, set(sigs)))
        tables.append({"id": ovl_id, "lo": lo, "hi": hi, "sha1": sha,
                       "load_size": load_size, "entries": ents})
    require_blobs(missing, roots)
    dropped: list[str] = []
    cc = None if args.no_compile_check else g.find_compiler()
    text = render_dispatch(tables)
    for _ in range(64):
        if cc is None:
            break
        errs = g.compile_check(cc, [("zz_dispatch", text)])
        if not errs:
            break
        msg = errs["zz_dispatch"]
        bad = set(re.findall(r"(\w*func_[0-9A-Fa-f]{8})", msg))
        native = {name for (_ov, name) in NATIVE_OVERLAY_CALLBACKS}
        if bad & native:
            raise SystemExit(f"gen_overlay_ports: native callback registry failed compile check: {msg}")
        if not bad:
            raise SystemExit(f"gen_overlay_ports: dispatch registry does not compile: {msg}")
        for t in tables:
            keep = [e for e in t["entries"] if e["symbol"] not in bad]
            dropped += [e["symbol"] for e in t["entries"] if e["symbol"] in bad]
            t["entries"] = keep
        text = render_dispatch(tables)
    target = REPO_ROOT / DISPATCH_DIR / "zz_dispatch.c"
    stale = REPO_ROOT / DISPATCH_DIR / "zz_dispatch_port.c"
    if stale.exists() and not args.verify:
        stale.unlink()
    if args.verify:
        ok = target.exists() and target.read_text(encoding="utf-8") == text
        return (0 if ok else 1), dropped
    target.parent.mkdir(parents=True, exist_ok=True)
    if not target.exists() or target.read_text(encoding="utf-8") != text:
        target.write_text(text, encoding="utf-8")
    return sum(len(t["entries"]) for t in tables), dropped


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--overlay", action="append", default=[],
                    help="overlay id (repeatable); default: the subsystem overlays")
    ap.add_argument("--rooms", action="store_true",
                    help="only the deduplicated room TUs (default run includes them)")
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--verbose", action="store_true")
    ap.add_argument("--verify", action="store_true")
    ap.add_argument("--no-compile-check", action="store_true")
    args = ap.parse_args()
    ids = args.overlay or SUBSYSTEM_OVERLAYS
    windows = all_windows()
    total_drift = 0
    if not args.overlay or args.rooms:
        written, changed, skipped, unc = run_rooms(args, windows)
        if args.check:
            print(f"gen_overlay_ports: rooms: eligible {written} deduplicated bodies")
        elif args.verify:
            print(f"gen_overlay_ports: rooms: verified {written} TU(s); {changed} drifted")
            total_drift += changed
        else:
            print(f"gen_overlay_ports: rooms: wrote {written} TU(s) to "
                  f"{ROOMS_DIR.as_posix()} ({changed} changed)")
            for nm in sorted(unc):
                print(f"  uncompilable rooms/{nm}: {unc[nm]}")
        if args.verbose:
            for nm in sorted(skipped):
                print(f"  skip {nm}: {skipped[nm]}")
        if args.rooms:
            ids = []
    for ov in ids:
        ovl = parse_overlay(ov)
        written, changed, drift, skipped, unc = run_overlay(ovl, args, windows)
        n = len(ovl["leaves"])
        if args.check:
            print(f"gen_overlay_ports: {ov}: eligible {written}/{n}")
        elif args.verify:
            print(f"gen_overlay_ports: {ov}: verified {written} TU(s); {len(drift)} drifted")
            for d in drift:
                print(f"  stale {d}")
            total_drift += len(drift)
        else:
            print(f"gen_overlay_ports: {ov}: wrote {written}/{n} TU(s) to "
                  f"{(OUT_ROOT / ov).as_posix()} ({changed} changed)")
            for nm in sorted(unc):
                print(f"  uncompilable {ov}/{nm}: {unc[nm]}")
        if args.verbose:
            for nm in sorted(skipped):
                print(f"  skip {ov}/{nm}: {skipped[nm]}")
    if not args.check and not args.overlay:
        n, dropped = build_dispatch(args)
        if args.verify:
            print(f"gen_overlay_ports: dispatch registry "
                  f"{'up to date' if n == 0 else 'STALE'}")
            total_drift += n
        else:
            print(f"gen_overlay_ports: dispatch registry: {n} thunk(s) "
                  f"({len(dropped)} dropped by the compile gate)")
            for d in dropped:
                print(f"  dropped {d}")
    return 1 if (args.verify and total_drift) else 0


if __name__ == "__main__":
    raise SystemExit(main())
