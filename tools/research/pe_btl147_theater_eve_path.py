#!/usr/bin/env python3
"""PE-BTL147 — theater → m0005i path, persist[0] bit-4, type-4 0x0D.

Evidence-only. Does not poke persist/scratch/mailbox. Reconstructs the
retail hop graph immediately preceding m0005i, the persist[0]/[1]/[0x4A]
image those hops leave, the type-4 0x77 volume, and every EXE immediate
that could name m0360i (BTL140: no 0x31 inbound).
"""
from __future__ import annotations

import json
import struct
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

from tools.research.pe_pst0_scan import (  # noqa: E402
    ALPHABET,
    ALU_SUBOPS,
    DISC1_EXE,
    DISC1_EXE_SHA1,
    FIELD_TABLE_OFF,
    FORM1_USER_SIZE,
    LOAD,
    PE_IMG,
    RawMode2Image,
    decode_packed_name,
    decode_script,
    extract_script_from_package,
    find_entry,
    iso_entries,
    parse_field_table,
    sha1_bytes,
)

THEATER = (
    "m0001i",
    "m0002i",
    "m0003i",
    "m0004i",
    "m0005i",
    "m0064i",
    "m0360i",
    "m0367i",
    "m0372i",
    "m0377i",
    "m0378i",
)
M0005_PACKED = 0xA80002C8
M0360_PACKED_NAME = None  # filled at runtime
ALU_NAMES = {k: v[0] for k, v in ALU_SUBOPS.items()}


def encode_packed_name(name: str) -> int:
    token = 0
    for ch, shift in zip(name.lower(), (27, 22, 17, 12, 7, 2)):
        token |= ALPHABET.index(ch) << shift
    return token


def dest_index_from_token(token: int) -> int | None:
    low = token & 0xFFFF
    if low < 0x248 or (low - 0x248) % 0x80:
        return None
    return ((low - 0x248) // 0x80) + 3


def dest_map_from_low(token: int) -> int | None:
    low = token & 0xFFFF
    if (low - 0x48) % 0x80:
        return None
    return (low - 0x48) // 0x80


def fmt_token(token: int) -> dict:
    packed = decode_packed_name(token)
    idx = dest_index_from_token(token)
    map_id = dest_map_from_low(token)
    return {
        "token": f"0x{token:08X}",
        "packed_name": packed,
        "scene_index": idx,
        "scene_name_from_index": f"m{idx + 1:04d}i" if idx is not None else None,
        "map_id_from_low": map_id,
        "scene_name_from_map": f"m{map_id:04d}i" if map_id is not None else None,
    }


def decode_insn(blob: bytes, pc: int, end: int) -> dict | None:
    if pc + 8 > end:
        return None
    word, word2 = struct.unpack_from("<II", blob, pc)
    op = word & 0x1FFF
    argc = (word >> 13) & 0xF
    span = 8 + argc * 4
    if pc + span > end:
        return None
    kinds_bits = word >> 17
    kinds = []
    imms = []
    for i in range(argc):
        if i == 5:
            kinds_bits = word2
        kinds.append(kinds_bits & 7)
        kinds_bits >>= 3
        imms.append(struct.unpack_from("<I", blob, pc + 8 + i * 4)[0])
    return {
        "pc": pc,
        "rel": pc,
        "op": op,
        "argc": argc,
        "kinds": kinds,
        "imms": imms,
        "span": span,
        "word": word,
        "word2": word2,
    }


def walk_module_insns(blob: bytes, start: int, end: int) -> list[dict]:
    rows = []
    pc = start
    while pc + 8 <= end:
        insn = decode_insn(blob, pc, end)
        if insn is None or insn["op"] > 0x1FF:
            break
        insn["rel"] = pc - start
        rows.append(insn)
        pc += insn["span"]
    return rows


def s16(v: int) -> int:
    v &= 0xFFFF
    return v - 0x10000 if v >= 0x8000 else v


def fmt_insn(insn: dict) -> str:
    op = insn["op"]
    kinds = insn["kinds"]
    imms = insn["imms"]
    if op == 0x09 and len(imms) >= 4:
        sub = imms[0]
        return (
            f"+0x{insn['rel']:04X} alu {ALU_NAMES.get(sub, hex(sub))} "
            f"k={kinds[:4]} imm={[hex(x) if isinstance(x, int) else x for x in imms[:4]]}"
        )
    if op == 0x0A and len(imms) >= 2:
        return f"+0x{insn['rel']:04X} mov k={kinds[:2]} imm={[hex(x) for x in imms[:2]]}"
    if op == 0x1C and len(imms) >= 3:
        return f"+0x{insn['rel']:04X} mail 0x1C({imms[0]},{imms[1]},0x{imms[2]:02X})"
    if op == 0x31 and imms:
        tok = fmt_token(imms[0])
        return (
            f"+0x{insn['rel']:04X} dest 0x31 {tok['token']} "
            f"packed={tok['packed_name']} idx={tok['scene_name_from_index']}"
        )
    if op == 0x77:
        verts = []
        for i in range(0, min(8, len(imms)), 2):
            if i + 1 < len(imms) and kinds[i] == 0 and kinds[i + 1] == 0:
                verts.append((s16(imms[i] >> 16) if imms[i] > 0xFFFF else s16(imms[i]),
                              s16(imms[i + 1] >> 16) if imms[i + 1] > 0xFFFF else s16(imms[i + 1])))
        return (
            f"+0x{insn['rel']:04X} vol77 argc={insn['argc']} "
            f"k={kinds} imm={[hex(x) for x in imms]}"
        )
    if op == 0x08 and imms:
        return f"+0x{insn['rel']:04X} spawn type={imms[0]} k={kinds} imm={[hex(x) for x in imms]}"
    if op == 0x05 and len(imms) >= 2:
        skip = (imms[1] << 1) & 0xFFFFFFFF
        return f"+0x{insn['rel']:04X} skip0 cond_k={kinds[0] if kinds else None} -> +0x{skip:X}"
    if op == 0x00 and imms:
        return f"+0x{insn['rel']:04X} goto +0x{(imms[0] << 1) & 0xFFFFFFFF:X}"
    return f"+0x{insn['rel']:04X} op{op:02X} argc={insn['argc']} k={kinds} imm={[hex(x) for x in imms[:6]]}"


def nearby_persist(insns: list[dict], idx: int, window: int = 8) -> list[str]:
    lo = max(0, idx - window)
    hi = min(len(insns), idx + window + 1)
    out = []
    for j in range(lo, hi):
        insn = insns[j]
        if insn["op"] in (0x09, 0x0A, 0x31, 0x1C, 0x77, 0x05, 0x08):
            mark = ">>" if j == idx else "  "
            out.append(mark + " " + fmt_insn(insn))
    return out


def scan_exe_immediates(exe: bytes, wanted: set[int]) -> list[dict]:
    """Find lui/ori, lui/addiu, and raw words for dest-like constants."""
    text_off = 0x800
    text = exe[text_off:]
    hits = []
    # raw word scan (aligned)
    for off in range(0, len(text) - 3, 4):
        w = struct.unpack_from("<I", text, off)[0]
        if w in wanted:
            va = LOAD + off
            hits.append({"kind": "word", "va": f"0x{va:08X}", "value": f"0x{w:08X}"})
    # lui / ori or addiu pairs
    for off in range(0, len(text) - 7, 4):
        w0, w1 = struct.unpack_from("<II", text, off)
        if (w0 >> 26) != 0x0F:  # lui
            continue
        rt0 = (w0 >> 16) & 0x1F
        hi = w0 & 0xFFFF
        op1 = w1 >> 26
        if op1 not in (0x0D, 0x09):  # ori / addiu
            continue
        rs1 = (w1 >> 21) & 0x1F
        rt1 = (w1 >> 16) & 0x1F
        if rs1 != rt0:
            continue
        lo = w1 & 0xFFFF
        if op1 == 0x09 and lo >= 0x8000:
            val = ((hi << 16) + (lo - 0x10000)) & 0xFFFFFFFF
        else:
            val = ((hi << 16) | lo) if op1 == 0x0D else ((hi << 16) + lo) & 0xFFFFFFFF
            if op1 == 0x09:
                val = ((hi << 16) + lo) & 0xFFFFFFFF
        if val in wanted:
            va = LOAD + off
            hits.append(
                {
                    "kind": "lui_pair",
                    "va": f"0x{va:08X}",
                    "pair": "ori" if op1 == 0x0D else "addiu",
                    "rt": rt1,
                    "value": f"0x{val:08X}",
                }
            )
    return hits


def load_scene_scripts(image, pe_entry, field_table, names: set[str] | None = None):
    out = {}
    for rec in field_table:
        if names is not None and rec["name"] not in names:
            continue
        if rec["end"] <= rec["start"]:
            continue
        try:
            blob = image.read_form1_extent(
                pe_entry[1] + rec["start"],
                (rec["end"] - rec["start"]) * FORM1_USER_SIZE,
            )
        except Exception:
            continue
        extracted = extract_script_from_package(blob, rec["meta"])
        if extracted is None:
            continue
        off, script_blob = extracted
        script = decode_script(script_blob)
        out[rec["name"]] = {
            "rec": rec,
            "blob": script_blob,
            "script": script,
            "pkg_off": off,
        }
    return out


def persist_ops_from_insns(insns: list[dict], scene: str, module: int) -> list[dict]:
    rows = []
    for insn in insns:
        op = insn["op"]
        kinds = insn["kinds"]
        imms = insn["imms"]
        if op == 0x0A and len(imms) >= 2 and kinds:
            if kinds[0] == 2:
                rows.append(
                    {
                        "scene": scene,
                        "module": module,
                        "pc": f"+0x{insn['rel']:04X}",
                        "rw": "write",
                        "index": imms[0],
                        "op": "assign",
                        "src_kind": kinds[1] if len(kinds) > 1 else None,
                        "value": imms[1] if len(kinds) > 1 and kinds[1] == 0 else None,
                    }
                )
            if len(kinds) > 1 and kinds[1] == 2:
                rows.append(
                    {
                        "scene": scene,
                        "module": module,
                        "pc": f"+0x{insn['rel']:04X}",
                        "rw": "read",
                        "index": imms[1],
                        "op": "assign_from",
                        "dst_kind": kinds[0],
                        "value": None,
                    }
                )
        if op == 0x09 and len(imms) >= 4:
            sub = imms[0]
            for i, kind in enumerate(kinds[:4]):
                if kind == 2:
                    rows.append(
                        {
                            "scene": scene,
                            "module": module,
                            "pc": f"+0x{insn['rel']:04X}",
                            "rw": "write" if i == 1 else "read",
                            "index": imms[i],
                            "op": ALU_NAMES.get(sub, hex(sub)),
                            "sub": sub,
                            "arg_i": i,
                            "imm_b": imms[3] if len(kinds) > 3 and kinds[3] == 0 else None,
                            "imm_a": imms[2] if len(kinds) > 2 and kinds[2] == 0 else None,
                        }
                    )
    return rows


def main() -> int:
    global M0360_PACKED_NAME
    M0360_PACKED_NAME = encode_packed_name("m0360i")
    m0005_packed = encode_packed_name("m0005i")
    print(f"packed m0005i=0x{m0005_packed:08X} m0360i=0x{M0360_PACKED_NAME:08X}")
    print(f"dest-formula m0005i=0x{M0005_PACKED:08X} m0360i-low-formula=0x{(360 << 7) + 0x48:04X}")

    pointer = ROOT / "local" / "pe_disc1.path"
    disc = Path(pointer.read_text().strip().splitlines()[0].strip())
    out_dir = ROOT / "docs" / "evidence" / "pe-btl147-theater-eve-path"
    out_dir.mkdir(parents=True, exist_ok=True)

    hops_to_m0005 = []
    hops_from_theater = []
    persist_rows = []
    type4_dump = []
    m0372_bit4 = []
    m0004_persist0 = []
    m0360_mod2 = []
    inbound_360_idx = []

    wanted_exe = {
        M0360_PACKED_NAME,
        m0005_packed,
        M0005_PACKED,
        0xA8000000 | ((360 << 7) + 0x48),
        0xA800B448,
        0xA8066048,
        0xA80660C8,
        0xA80830C8,
        0xA8081448,
        0xA8000248,
        0xA8000148,
        0xA80001C8,
        0xA80000C8,
        0xA8002048,  # m0064i from BTL83
    }

    with RawMode2Image(disc) as image:
        entries = iso_entries(image)
        pe_entry = find_entry(entries, PE_IMG)
        exe = image.read_form1_extent(
            find_entry(entries, DISC1_EXE)[1],
            find_entry(entries, DISC1_EXE)[2],
        )
        assert sha1_bytes(exe) == DISC1_EXE_SHA1, sha1_bytes(exe)
        field_table = parse_field_table(exe)
        by_name = {r["name"]: r for r in field_table}

        print("field table theater:")
        for name in THEATER:
            rec = by_name.get(name)
            print(f"  {name}: {rec}")

        # all-scene 0x31 inbound to m0005i / m0360i
        scripts_found = 0
        for rec in field_table:
            if rec["end"] <= rec["start"]:
                continue
            try:
                blob = image.read_form1_extent(
                    pe_entry[1] + rec["start"],
                    (rec["end"] - rec["start"]) * FORM1_USER_SIZE,
                )
            except Exception:
                continue
            extracted = extract_script_from_package(blob, rec["meta"])
            if extracted is None:
                continue
            _off, script_blob = extracted
            script = decode_script(script_blob)
            if not script.get("modules"):
                continue
            scripts_found += 1
            for module in script["modules"]:
                insns = walk_module_insns(script_blob, module["start"], module["end"])
                last_4a = None
                last_p0 = None
                last_p1 = None
                for i, insn in enumerate(insns):
                    if insn["op"] == 0x0A and insn["kinds"] and insn["kinds"][0] == 2:
                        if insn["imms"][0] == 0x4A and len(insn["kinds"]) > 1 and insn["kinds"][1] == 0:
                            last_4a = insn["imms"][1]
                        if insn["imms"][0] == 0 and len(insn["kinds"]) > 1 and insn["kinds"][1] == 0:
                            last_p0 = insn["imms"][1]
                        if insn["imms"][0] == 1 and len(insn["kinds"]) > 1 and insn["kinds"][1] == 0:
                            last_p1 = insn["imms"][1]
                    if insn["op"] != 0x31 or not insn["imms"]:
                        continue
                    token = insn["imms"][0]
                    tok = fmt_token(token)
                    hop = {
                        "from": rec["name"],
                        "from_index": rec["index"],
                        "module": module["index"],
                        "pc": f"+0x{insn['rel']:04X}",
                        **tok,
                        "last_persist_0": last_p0,
                        "last_persist_1": last_p1,
                        "last_persist_4a": last_4a,
                        "context": nearby_persist(insns, i, 6),
                    }
                    if rec["name"] in THEATER:
                        hops_from_theater.append(hop)
                    idx_name = tok["scene_name_from_index"]
                    packed = tok["packed_name"]
                    if (
                        token == M0005_PACKED
                        or packed == "m0005i"
                        or idx_name == "m0005i"
                        or tok["scene_name_from_map"] == "m0005i"
                    ):
                        hops_to_m0005.append(hop)
                    if (
                        token == M0360_PACKED_NAME
                        or packed == "m0360i"
                        or idx_name == "m0360i"
                        or tok["scene_name_from_map"] == "m0360i"
                        or dest_index_from_token(token) == 359
                    ):
                        inbound_360_idx.append(hop)

        theater = load_scene_scripts(image, pe_entry, field_table, set(THEATER))

    # persist ops + type-4 dump on theater scenes
    for name, pack in theater.items():
        blob = pack["blob"]
        script = pack["script"]
        for module in script["modules"]:
            insns = walk_module_insns(blob, module["start"], module["end"])
            persist_rows.extend(persist_ops_from_insns(insns, name, module["index"]))
            if name == "m0005i":
                for insn in insns:
                    if insn["op"] in (0x77, 0x1C, 0x08, 0x5E):
                        type4_dump.append(
                            {
                                "module": module["index"],
                                "line": fmt_insn(insn),
                                "kinds": insn["kinds"],
                                "imms": [int(x) for x in insn["imms"]],
                                "rel": insn["rel"],
                            }
                        )
                # full module 4 dump around 0x77
            if name == "m0004i":
                for row in persist_ops_from_insns(insns, name, module["index"]):
                    if row["index"] == 0:
                        m0004_persist0.append({**row, "line": None})
            if name == "m0372i":
                for insn in insns:
                    if insn["op"] == 0x09 and len(insn["imms"]) >= 4:
                        if 2 in insn["kinds"][:4] and (
                            insn["imms"][3] == 4
                            or insn["imms"][2] == 4
                            or insn["imms"][0] == 3
                        ):
                            m0372_bit4.append(fmt_insn(insn))
            if name == "m0360i" and module["index"] == 2:
                for insn in insns:
                    m0360_mod2.append(fmt_insn(insn))

    # type-4 actor script from m0005i chunk2 list, if present in blob
    m5 = theater.get("m0005i")
    type4_actor = []
    if m5:
        blob = m5["blob"]
        # dump every 0x77 in every module with vertex immediates
        for module in m5["script"]["modules"]:
            insns = walk_module_insns(blob, module["start"], module["end"])
            for i, insn in enumerate(insns):
                if insn["op"] != 0x77:
                    continue
                window = [fmt_insn(x) for x in insns[max(0, i - 4) : i + 12]]
                type4_actor.append(
                    {
                        "module": module["index"],
                        "rel": insn["rel"],
                        "argc": insn["argc"],
                        "kinds": insn["kinds"],
                        "imms_hex": [f"0x{x:X}" for x in insn["imms"]],
                        "s16_imms": [s16(x) for x in insn["imms"]],
                        "hi16": [s16(x >> 16) for x in insn["imms"]],
                        "window": window,
                    }
                )

    exe_path_candidates = [
        ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62",
        ROOT / "build" / "disc1.candidate.exe",
    ]
    exe_blob = None
    for p in exe_path_candidates:
        if p.is_file():
            exe_blob = p.read_bytes()
            break
    exe_hits = scan_exe_immediates(exe_blob, wanted_exe) if exe_blob else []

    # persist[0] bit-4 related on theater
    p0_bit4 = [
        r
        for r in persist_rows
        if r["index"] == 0
        and (
            r.get("imm_b") == 4
            or r.get("imm_a") == 4
            or r.get("value") == 4
            or (isinstance(r.get("value"), int) and r["value"] is not None and (r["value"] & 4))
        )
    ]
    p0_all_theater = [r for r in persist_rows if r["index"] == 0 and r["scene"] in THEATER]
    p4a_theater = [r for r in persist_rows if r["index"] == 0x4A and r["scene"] in THEATER]

    payload = {
        "packed_m0005i": f"0x{m0005_packed:08X}",
        "packed_m0360i": f"0x{M0360_PACKED_NAME:08X}",
        "dest_formula_m0005i": f"0x{M0005_PACKED:08X}",
        "hops_to_m0005i": hops_to_m0005,
        "hops_from_theater": hops_from_theater,
        "inbound_m0360i": inbound_360_idx,
        "persist0_theater": p0_all_theater,
        "persist0_bit4_theater": p0_bit4,
        "persist4a_theater": p4a_theater,
        "m0004i_persist0": m0004_persist0,
        "m0372i_bit4_lines": m0372_bit4,
        "m0360i_module2": m0360_mod2,
        "type4_0x77": type4_actor,
        "m0005i_spawn_mail_vol": type4_dump,
        "exe_dest_immediates": exe_hits,
        "scripts_found": scripts_found,
    }
    (out_dir / "scan.json").write_text(json.dumps(payload, indent=2) + "\n")

    print(f"\nscripts_found={scripts_found}")
    print(f"hops_to_m0005i={len(hops_to_m0005)}")
    for h in hops_to_m0005:
        print(
            f"  {h['from']} mod{h['module']} {h['pc']} token={h['token']} "
            f"packed={h['packed_name']} idx={h['scene_name_from_index']} "
            f"p0={h['last_persist_0']} p1={h['last_persist_1']} p4a={h['last_persist_4a']}"
        )
        for line in h["context"][-8:]:
            print("   ", line)
    print(f"\ninbound_m0360i={len(inbound_360_idx)}")
    for h in inbound_360_idx:
        print(f"  {h}")
    print("\nm0360i module2:")
    for line in m0360_mod2:
        print(f"  {line}")
    print(f"\ntheater hops {len(hops_from_theater)}")
    by_from = defaultdict(list)
    for h in hops_from_theater:
        by_from[h["from"]].append(h)
    for name in THEATER:
        hops = by_from.get(name, [])
        print(f"  {name}: {len(hops)} dests")
        for h in hops:
            print(
                f"    mod{h['module']} {h['pc']} -> {h['token']} "
                f"{h['packed_name']}/{h['scene_name_from_index']} "
                f"p0={h['last_persist_0']} p1={h['last_persist_1']} 4a={h['last_persist_4a']}"
            )
    print("\npersist[0] theater writes/tests:")
    for r in p0_all_theater:
        print(f"  {r}")
    print("\npersist[0] bit4-ish:")
    for r in p0_bit4:
        print(f"  {r}")
    print("\nm0004i persist[0]:")
    for r in m0004_persist0:
        print(f"  {r}")
    print("\nm0372i bit4 lines:")
    for line in m0372_bit4:
        print(f"  {line}")
    print("\ntype-4 / 0x77:")
    for row in type4_actor:
        print(f"  module {row['module']} +0x{row['rel']:04X} argc={row['argc']}")
        print(f"    kinds={row['kinds']}")
        print(f"    imms={row['imms_hex']}")
        print(f"    s16={row['s16_imms']} hi16={row['hi16']}")
        for line in row["window"]:
            print(f"    {line}")
    print("\nEXE dest immediates:")
    for h in exe_hits:
        print(f"  {h}")
    print(f"\nwrote {out_dir / 'scan.json'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
