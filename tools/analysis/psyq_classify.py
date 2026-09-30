#!/usr/bin/env python3
"""Classify every SLUS_006.62 EXE function (and the PE.IMG libpress overlay)
as Sony Psy-Q SDK library code vs. game code.

Output: a TSV with columns
    addr  name  library  confidence  evidence  sdk_name

confidence:
  signature  address lies inside a Psy-Q SDK object/function whose byte
             signature matched (vendor/khasinski-parasite-eve-decomp/configs/
             USA/psyq_provenance.json: signature_sha256 vs retail bytes)
  bios-stub  body is a BIOS A0/B0/C0 call vector (`addiu $t2,$zero,0xA0|B0|C0`
             + `jr $t2`) -- the Psy-Q libapi/libc BIOS wrappers
  layout     the oracle decomp's splat layout files it under psyq/<lib>/
  band       inside the SDK link region (0x800718D0..0x80084FE4, all compiled
             by the gcc-2.8.1-style cc1 used for the SDK), filed by the oracle
             under an SDK-shaped subsystem (gpu/cdrom/memcard/pad/gte/math/sys)
             or a gap/tail of one; library inferred from that subsystem or the
             nearest signature/layout neighbour
  overlay    PE.IMG ovl_039F text 0x54..0xF04, the linked libpress (DecDCT /
             VLC) fragment (oracle: "linked PSY-Q libpress/BUILD/VLC fragment")
  -          game code (Square), incl. the AKAO sound driver

This file lists names/addresses only -- no SDK bytes, no SDK source.
Inputs are tracked (vendor/, configs/) plus the local split outputs
(build/generated/disc1_plan.json, asm/disc1, asm/overlays/ovl_039F) and the
user's own retail EXE for the BIOS-stub scan (build/extracted/disc1/SLUS_006.62).
"""
from __future__ import annotations

import argparse
import bisect
import json
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
VENDOR = ROOT / "vendor/khasinski-parasite-eve-decomp/configs/USA"
TEXT_VRAM_BIAS = 0x8000F800  # vram = file offset + bias (0x800 header, t_addr 0x80010000)
SDK_REGION = (0x800718D0, 0x80084FE4)  # first psyq/libgpu (tim) .. first akao/ unit
SUBSYS_LIB = {
    "gpu": "libgpu", "cdrom": "libcd", "memcard": "libcard", "pad": "libpad",
    "gte": "libgte", "math": "libmath", "sys": "libapi", "event": "libapi",
    "save": "libcd",
}


def load_layout():
    rows = []
    pat1 = re.compile(r"\s*- \[(0x[0-9A-Fa-f]+),\s*([.\w]+)(?:,\s*([^\]]+))?\]")
    pat2 = re.compile(r"\s*- \{start: (0x[0-9A-Fa-f]+), type: ([.\w]+), name: ([^,}]+)")
    in_main = False
    for line in (VENDOR / "main.yaml").read_text().splitlines():
        if re.match(r"\s*- name: main\s*$", line):
            in_main = True
        elif re.match(r"\s*- name: ", line) and in_main:
            break
        if not in_main:
            continue
        m = pat1.match(line) or pat2.match(line)
        if m:
            rows.append((int(m.group(1), 16) + TEXT_VRAM_BIAS, m.group(2), (m.group(3) or "").strip()))
    rows.sort()
    return rows


def load_provenance():
    d = json.loads((VENDOR / "psyq_provenance.json").read_text())
    ev = []
    for e in d["evidence"]:
        a = int(e["address"], 16)
        ev.append((a, a + int(e["size"]), e["library"].lower(), e["object"], e["sdk_version"], e.get("labels", [])))
    return ev


def load_syms():
    syms = {}
    for line in (VENDOR / "sym.main.txt").read_text().splitlines():
        m = re.match(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+);", line)
        if m:
            syms.setdefault(int(m.group(2), 16), m.group(1))
    return syms


def load_functions():
    plan = json.loads((ROOT / "build/generated/disc1_plan.json").read_text())
    funcs = []
    for u in plan["units"]:
        if u["kind"] == "c":
            funcs.append((u["vram"], u["name"], u["size"], "c"))
        elif u["kind"] == "asm":
            cur = None
            for line in (ROOT / u["source"]).read_text().splitlines():
                m = re.match(r"glabel (\S+)", line)
                if m:
                    cur = m.group(1)
                    continue
                m = re.match(r"\s*/\* [0-9A-F]+ ([0-9A-F]{8}) ", line)
                if m and cur:
                    funcs.append((int(m.group(1), 16), cur, None, "asm"))
                    cur = None
    funcs.sort()
    # sizes for asm: distance to next function
    out = []
    for i, (a, n, s, k) in enumerate(funcs):
        if s is None:
            s = (funcs[i + 1][0] - a) if i + 1 < len(funcs) else 4
        out.append((a, n, s, k))
    return out


def bios_stub(exe: bytes | None, a: int, size: int):
    if exe is None:
        return None
    off = a - TEXT_VRAM_BIAS
    words = [struct.unpack_from("<I", exe, off + 4 * i)[0] for i in range(min(size // 4, 8))]
    vecs = {0x240A00A0: "A0", 0x240A00B0: "B0", 0x240A00C0: "C0"}
    if len(words) >= 2 and words[0] in vecs and words[1] == 0x01400008:
        fn = (words[2] & 0xFFFF) if (words[2] >> 16) == 0x2409 else None
        return f"{vecs[words[0]]}({fn:#04x})" if fn is not None else vecs[words[0]]
    return None


def classify(exe_path: Path | None):
    layout = load_layout()
    lay_addr = [r[0] for r in layout]
    prov = load_provenance()
    syms = load_syms()
    exe = exe_path.read_bytes() if exe_path and exe_path.exists() else None
    rows = []
    for a, name, size, kind in load_functions():
        lib, conf, evid, sdk = "game", "-", [], syms.get(a, "")
        hits = [p for p in prov if p[0] <= a < p[1]]
        if hits:
            p = hits[0]
            lib, conf = p[2], "signature"
            lbl = [l["name"] for l in p[5] if p[0] + l["offset"] == a]
            evid.append(f"sdk-signature:{p[2].upper()}/{p[3]}@{p[4]}")
            if lbl:
                sdk = lbl[0]
        stub = bios_stub(exe, a, size)
        if stub:
            if conf == "-":
                lib, conf = "libapi", "bios-stub"
            evid.append(f"bios-vector:{stub}")
        i = bisect.bisect_right(lay_addr, a) - 1
        lay = layout[i] if i >= 0 else None
        lname = lay[2] if lay else ""
        if lay:
            evid.append(f"oracle-layout:{lname or lay[1]}")
        if lname.startswith("psyq/"):
            if conf in ("-",):
                lib, conf = lname.split("/")[1], "layout"
        elif conf == "-" and SDK_REGION[0] <= a < SDK_REGION[1]:
            sub = lname.split("/")[0] if lname else ""
            m = re.match(r"main/gap_([a-z]+)_", lname)
            if m:
                sub = m.group(1)
            if sub in SUBSYS_LIB:
                lib = SUBSYS_LIB[sub]
            else:
                # nearest preceding classified SDK neighbour
                lib = next((r[2] for r in reversed(rows) if r[3] in ("signature", "layout", "band")), "psyq?")
            conf = "band"
            evid.append("cc1-2.8.1-sdk-region")
        rows.append([f"0x{a:08X}", name, lib, conf, ";".join(evid), sdk])
    # ovl_039F libpress fragment (text 0x54..0xF04 at vram 0x8010BCF8)
    ovl_base, lo, hi = 0x8010BCF8, 0x8010BCF8 + 0x54, 0x8010BCF8 + 0xF04
    names = set()
    ycfg = ROOT / "configs/USA/overlays/ovl_039F.yaml"
    for line in ycfg.read_text().splitlines():
        m = re.match(r"\s*- \[0x([0-9A-Fa-f]+), c, (\w+)\]", line)
        if m:
            names.add((ovl_base + int(m.group(1), 16), m.group(2)))
    for s in sorted((ROOT / "asm/overlays/ovl_039F").glob("*.s")):
        cur = None
        for line in s.read_text().splitlines():
            m = re.match(r"glabel (\S+)", line)
            if m:
                cur = m.group(1)
                mm = re.match(r"func_([0-9A-F]{8})$", cur)
                if mm:
                    names.add((int(mm.group(1), 16), cur))
    for a, n in sorted(names):
        if lo <= a < hi:
            rows.append([f"ovl_039F:0x{a:08X}", n, "libpress", "overlay",
                         "manifest:ovl_039F 'MDEC / libpress movie module';oracle:sys_reset.yaml 'linked PSY-Q libpress/BUILD/VLC fragment'", ""])
    return rows


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--exe", default=str(ROOT / "build/extracted/disc1/SLUS_006.62"))
    ap.add_argument("--out", action="append", default=[])
    a = ap.parse_args()
    rows = classify(Path(a.exe))
    hdr = "addr\tname\tlibrary\tconfidence\tevidence\tsdk_name\n"
    text = hdr + "".join("\t".join(r) + "\n" for r in rows)
    for o in a.out or ["-"]:
        if o == "-":
            sys.stdout.write(text)
        else:
            Path(o).parent.mkdir(parents=True, exist_ok=True)
            Path(o).write_text(text)
    from collections import Counter
    c = Counter((r[2] if r[3] != "-" else "game", r[3]) for r in rows)
    for k, v in sorted(c.items()):
        print(f"{k[0]:10s} {k[1]:10s} {v}", file=sys.stderr)


if __name__ == "__main__":
    main()
