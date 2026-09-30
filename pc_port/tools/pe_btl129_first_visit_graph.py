#!/usr/bin/env python3
"""PE-BTL129 — first-visit event/task graph and 36448 recovery.

EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
m0005i chunk2 SHA-256 01a64ba3…7e3b.

Does not inject mailbox payloads. Does not poke scratch.
Does not treat file order as task order.
"""
from __future__ import annotations

import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
HDR = 0x800
PE_IMG_LBA = 1013
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048
M0005I_REL = 0x266A
M0005I_PACKED = 0x0600A921
M0005I_CHUNK2 = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
LIST_OFF = 0x202A4
ROOT = pathlib.Path(__file__).resolve().parents[2]

F36448 = 0x80036448
F36448_END = 0x80036DC8
F12700 = 0x80012700
F12774 = 0x80012774
F360B4 = 0x800360B4
F14DA0 = 0x80014DA0
F1CAB0 = 0x8001CAB0

OP_NAMES = {
    0x00: "goto",
    0x02: "yield",
    0x04: "park-sibs",
    0x05: "skip0",
    0x08: "spawn",
    0x09: "alu",
    0x0A: "mov",
    0x0B: "pose",
    0x14: "label",
    0x1C: "mail",
    0x1D: "skip_ne",
    0x1F: "inbox",
    0x20: "park",
    0x2A: "bset",
    0x2E: "cmd",
    0x55: "wait55",
    0x6F: "body",
    0x77: "vol77",
    0x85: "hop85",
    0x89: "wait89",
    0xAE: "ae",
}


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


def jal_word(target: int) -> int:
    return 0x0C000000 | ((target & 0x0FFFFFFF) >> 2)


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def find_disc() -> pathlib.Path:
    pointer = ROOT / "local" / "pe_disc1.path"
    if pointer.is_file():
        path = pathlib.Path(pointer.read_text().strip().splitlines()[0].strip())
        if path.is_file():
            return path
    env = os.environ.get("PE_DISC1_BIN", "").strip()
    if env:
        path = pathlib.Path(env)
        if path.is_file():
            return path
    raise SystemExit("FAIL: missing Disc 1")


def read_form1(disc: pathlib.Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for i in range(nsec):
            fh.seek((lba + i) * SECTOR_RAW + FORM1_OFF)
            chunk = fh.read(FORM1_USER)
            require(len(chunk) == FORM1_USER, f"disc read {lba + i}")
            out.extend(chunk)
    return bytes(out)


def packed_secs(packed: int) -> tuple[int, int, int]:
    return packed & 0xFF, (packed >> 8) & 0xFFF, packed >> 20


def decode_insn(blob: bytes, pc: int) -> dict | None:
    if pc + 8 > len(blob):
        return None
    word, word2 = struct.unpack_from("<II", blob, pc)
    op = word & 0x1FFF
    argc = (word >> 13) & 0xF
    span = 8 + argc * 4
    if pc + span > len(blob):
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
        "word": word,
        "op": op,
        "argc": argc,
        "kinds": kinds,
        "imms": imms,
        "span": span,
    }


def linear_walk(blob: bytes, base: int, end: int) -> list[dict]:
    rows = []
    pc = base
    while pc + 8 <= end:
        insn = decode_insn(blob, pc)
        if insn is None or insn["op"] > 0x1FF:
            break
        insn["rel"] = pc - base
        rows.append(insn)
        pc += insn["span"]
    return rows


def script_base(blob: bytes, list_off: int, typ: int) -> int:
    rel = struct.unpack_from("<I", blob, list_off + 8 + typ * 4)[0]
    return list_off + rel


def load_exe() -> bytes:
    for p in (
        ROOT / "build" / "disc1.candidate.exe",
        ROOT / "build" / "extracted" / "disc1" / "SLUS_006.62",
    ):
        if p.is_file():
            blob = p.read_bytes()
            require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
            return blob
    raise SystemExit("FAIL: missing retail EXE")


def hw(rel_imm: int) -> int:
    return (rel_imm << 1) & 0xFFFFFFFF


def cfg_successors(insn: dict) -> list[int]:
    rel = insn["rel"]
    op = insn["op"]
    imms = insn["imms"]
    nxt = rel + insn["span"]
    if op == 0x00 and imms:
        return [hw(imms[0])]
    if op == 0x20:
        return []
    if op == 0x05 and len(imms) > 1:
        return [nxt, hw(imms[1])]
    if op == 0x1D and len(imms) > 2:
        return [nxt, hw(imms[2])]
    return [nxt]


def reachable(by_rel: dict[int, dict], start: int) -> set[int]:
    seen: set[int] = set()
    stack = [start]
    while stack:
        cur = stack.pop()
        if cur in seen:
            continue
        seen.add(cur)
        insn = by_rel.get(cur)
        if insn is None:
            continue
        for nxt in cfg_successors(insn):
            if nxt not in seen:
                stack.append(nxt)
    return seen


def label_sites(rows: list[dict]) -> list[tuple[int, int, int]]:
    out = []
    for insn in rows:
        if insn["op"] != 0x14 or len(insn["imms"]) < 2:
            continue
        code = insn["imms"][0]
        rel = insn["imms"][1]
        tgt = -1 if rel < 0 else hw(rel)
        out.append((insn["rel"], code, tgt))
    return out


def send_1c(rows: list[dict]) -> list[tuple[int, list[int]]]:
    out = []
    for insn in rows:
        if insn["op"] != 0x1C:
            continue
        out.append((insn["rel"], insn["imms"][:]))
    return out


def loc_key(kind: int, imm: int) -> tuple[int, int]:
    return (kind, imm)


def alu_eval(sub: int, a: int | None, b: int | None) -> int | None:
    if a is None:
        return None
    if sub == 0x07:
        return 1 if a == 0 else 0
    if sub == 0x08:
        return (~a) & 0xFFFFFFFF
    if sub == 0x13:
        return a & 0xFFFFFFFF
    if sub == 0x17:
        return (-a) & 0xFFFFFFFF
    if b is None:
        return None
    sa = a if a < 0x80000000 else a - 0x100000000
    sb = b if b < 0x80000000 else b - 0x100000000
    if sub == 0x00:
        return (a + b) & 0xFFFFFFFF
    if sub == 0x01:
        return (a - b) & 0xFFFFFFFF
    if sub == 0x02:
        return a | b
    if sub == 0x03:
        return a & b
    if sub == 0x04:
        return a ^ b
    if sub == 0x05:
        return 1 if (a or b) else 0
    if sub == 0x06:
        return 1 if (a and b) else 0
    if sub == 0x09:
        return 1 if sb < sa else 0
    if sub == 0x0A:
        return 1 if sa < sb else 0
    if sub == 0x0B:
        return 1 if a == b else 0
    if sub == 0x0C:
        return 1 if sa >= sb else 0
    if sub == 0x0D:
        return 1 if sa <= sb else 0
    if sub == 0x0E:
        return 1 if a != b else 0
    return None


def read_val(env: dict[tuple[int, int], int | None], kind: int, imm: int) -> int | None:
    if kind == 0:
        return imm & 0xFFFFFFFF
    return env.get(loc_key(kind, imm))


def payload_reach(
    by_rel: dict[int, dict],
    start: int,
    payload: int | None,
    persist: dict[int, int] | None = None,
    scratch: dict[int, int] | None = None,
) -> set[int]:
    """CFG under a known mailbox payload / persist / scratch image."""
    persist = persist or {}
    scratch = scratch or {}
    seen: set[tuple[int, tuple[tuple[tuple[int, int], int], ...]]] = set()
    stack: list[tuple[int, dict[tuple[int, int], int | None]]] = []
    env0: dict[tuple[int, int], int | None] = {}
    for k, v in persist.items():
        env0[loc_key(2, k)] = v
    for k, v in scratch.items():
        env0[loc_key(4, k)] = v
    stack.append((start, env0))
    reached: set[int] = set()

    while stack:
        rel, env = stack.pop()
        snap = tuple(sorted((k, v) for k, v in env.items() if v is not None))
        key = (rel, snap)
        if key in seen:
            continue
        seen.add(key)
        reached.add(rel)
        insn = by_rel.get(rel)
        if insn is None:
            continue
        op = insn["op"]
        kinds = insn["kinds"]
        imms = insn["imms"]
        nxt = rel + insn["span"]
        env2 = dict(env)

        if op == 0x1F and kinds:
            dest = loc_key(kinds[0], imms[0])
            env2[dest] = payload
            stack.append((nxt, env2))
            continue
        if op == 0x0A and len(kinds) >= 2:
            dest = loc_key(kinds[0], imms[0])
            env2[dest] = read_val(env, kinds[1], imms[1])
            stack.append((nxt, env2))
            continue
        if op in (0x2A, 0x28) and len(kinds) >= 2:
            dest = loc_key(kinds[0], imms[0])
            bit = read_val(env, kinds[1], imms[1])
            cur = env.get(dest)
            if cur is None:
                cur = 0 if dest[0] in (2, 4) else None
            if bit is not None and cur is not None:
                mask = 1 << (bit & 31)
                env2[dest] = (cur | mask) if op == 0x2A else (cur & ~mask)
            stack.append((nxt, env2))
            continue
        if op == 0x09 and len(kinds) >= 2:
            sub = read_val(env, kinds[0], imms[0])
            dest = loc_key(kinds[1], imms[1])
            a = read_val(env, kinds[2], imms[2]) if len(kinds) > 2 else None
            b = read_val(env, kinds[3], imms[3]) if len(kinds) > 3 else None
            if sub is not None:
                env2[dest] = alu_eval(sub, a, b)
            stack.append((nxt, env2))
            continue
        if op == 0x05 and len(imms) > 1:
            cond = read_val(env, kinds[0], imms[0]) if kinds else None
            skip = hw(imms[1])
            if cond == 0:
                stack.append((skip, env2))
            elif cond is not None:
                stack.append((nxt, env2))
            else:
                stack.append((nxt, env2))
                stack.append((skip, env2))
            continue
        if op == 0x1D and len(imms) > 2:
            left = read_val(env, kinds[0], imms[0]) if kinds else None
            right = read_val(env, kinds[1], imms[1]) if len(kinds) > 1 else None
            skip = hw(imms[2])
            if left is not None and right is not None:
                stack.append((skip if left != right else nxt, env2))
            else:
                stack.append((nxt, env2))
                stack.append((skip, env2))
            continue
        if op == 0x00 and imms:
            stack.append((hw(imms[0]), env2))
            continue
        if op == 0x20:
            continue
        stack.append((nxt, env2))
    return reached


def fmt_mail(imms: list[int]) -> str:
    if len(imms) >= 3:
        return f"0x1C({imms[0]},{imms[1]},0x{imms[2]:02X})"
    return f"0x1C{imms}"


def fmt_insn(insn: dict) -> str:
    op = insn["op"]
    name = OP_NAMES.get(op, f"op{op:02X}")
    extra = ""
    if op == 0x14 and len(insn["imms"]) >= 2:
        code = insn["imms"][0]
        tgt = -1 if insn["imms"][1] < 0 else hw(insn["imms"][1])
        slot = {1: "+0x1A0", 2: "+0x19C", 3: "task+4"}.get(code, f"code{code}")
        extra = f" {slot}=+0x{tgt:X}" if tgt >= 0 else f" {slot}=0"
    elif op == 0x1C:
        extra = " " + fmt_mail(insn["imms"])
    elif op == 0x08 and insn["imms"]:
        extra = f" type {insn['imms'][0]}"
    elif op == 0x09 and len(insn["imms"]) >= 4:
        extra = (
            f" sub={insn['imms'][0]:#x} k={insn['kinds'][:4]} "
            f"imm={insn['imms'][:4]}"
        )
    elif op == 0x77:
        extra = f" argc={insn['argc']}"
    elif op == 0x2A and len(insn["imms"]) >= 2:
        extra = f" [{insn['imms'][0]},{insn['imms'][1]}]"
    return f"+0x{insn['rel']:04X} {name}{extra}"


def prove_36448(exe: bytes) -> None:
    require((F36448_END - F36448) // 4 == 608, "36448 608 words")
    require(load_u32(exe, F36448) == 0x27BDFF88, "36448 addiu -0x78")
    require(jal_target(load_u32(exe, 0x80035C1C)) == F36448, "35558@35C1C jal 36448")
    require(jal_target(load_u32(exe, 0x80035C24)) == F12774, "35558@35C24 jal 12774")
    require(jal_target(load_u32(exe, 0x80035C2C)) == F360B4, "35558@35C2C jal 360B4")
    require(load_u32(exe, 0x80035C10) == 0x30420004, "35C10 andi D1A0 4")
    require(load_u32(exe, 0x80035C14) == 0x14400014, "D1A0&4 skips 36448")

    # Pair walk + 1A0 spawn contract.
    require(load_u32(exe, 0x8003664C) == 0x8E4201A0, "lw s2+0x1A0")
    require(load_u32(exe, 0x800366B8) == 0x8E4401A0, "jal a0 = s2+0x1A0")
    require(jal_target(load_u32(exe, 0x800366BC)) == F12700, "366BC jal 12700")
    require(load_u32(exe, 0x800366D0) == 0x34420001, "task+8 |= 1")
    require(load_u32(exe, 0x80036714) == 0xAE4400A4, "prepend s2+0xA4")
    require(load_u32(exe, 0x80036718) == 0x8E2201A0, "lw s1+0x1A0")
    require(jal_target(load_u32(exe, 0x80036784)) == F12700, "36784 jal 12700")
    require(load_u32(exe, 0x800367E0) == 0xAE2400A4, "prepend s1+0xA4")

    # 1AC-both collision path still 1A0-spawns.
    require(load_u32(exe, 0x8003661C) == 0x8E4201AC, "lw s2+0x1AC")
    require(load_u32(exe, 0x800369F0) == 0x8E4201A0, "367E4 path lw s2+0x1A0")
    require(jal_target(load_u32(exe, 0x80036A60)) == F12700, "36A60 jal 12700")
    require(load_u32(exe, 0x80036AB8) == 0xAE4400A4, "367E4 prepend s2+A4")
    require(load_u32(exe, 0x80036ABC) == 0x8E2201A0, "367E4 lw s1+0x1A0")
    require(jal_target(load_u32(exe, 0x80036B2C)) == F12700, "36B2C jal 12700")
    require(load_u32(exe, 0x80036B84) == 0xAE2400A4, "367E4 prepend s1+A4")

    sites = []
    want = jal_word(F12700)
    for va in range(F36448, F36448_END, 4):
        if load_u32(exe, va) == want:
            sites.append(va)
    require(sites == [0x800366BC, 0x80036784, 0x80036A60, 0x80036B2C],
            f"36448 12700 sites {sites}")

    # 12774 reaps parked slot tasks after 36448.
    require(load_u32(exe, 0x80012798) == 0x8C4400A0, "12774 lw slot A0")
    require(load_u32(exe, 0x800127C8) == 0x30420010, "12774 task+8&0x10")
    require(load_u32(exe, 0x8001282C) == 0x2C420003, "12774 slots 0..2")

    # 0x77 handler is still 14DA0 / 1CAB0.
    require(load_u32(exe, 0x800910A0 + 0x77 * 4) == F14DA0, "table[0x77]")
    require(jal_target(load_u32(exe, 0x80014E04)) == F1CAB0, "14DA0 jal 1CAB0")
    require(load_u32(exe, 0x80014E18) == 0x24020001, "0x77 v0=1")


def load_scripts() -> tuple[bytes, list[dict]]:
    disc = find_disc()
    s0, s1, s2 = packed_secs(M0005I_PACKED)
    chunk2 = read_form1(disc, PE_IMG_LBA + M0005I_REL + s0 + s1, s2)
    require(hashlib.sha256(chunk2).hexdigest() == M0005I_CHUNK2, "chunk2 sha")
    bases = [script_base(chunk2, LIST_OFF, t) for t in range(7)]
    ends = bases[1:] + [0x25014]
    scripts = []
    for typ, base, end in zip(range(7), bases, ends):
        rows = linear_walk(chunk2, base, end)
        scripts.append(
            {
                "type": typ,
                "base": base,
                "end": end,
                "rows": rows,
                "by_rel": {i["rel"]: i for i in rows},
                "labels": label_sites(rows),
                "sends": send_1c(rows),
            }
        )
    return chunk2, scripts


def island_sends(scr: dict, start: int) -> list[tuple[int, list[int]]]:
    seen = reachable(scr["by_rel"], start)
    out = []
    for rel, imms in scr["sends"]:
        if rel in seen:
            out.append((rel, imms))
    return out


def dumps_type3(t3: dict) -> None:
    rows = t3["rows"]
    by = t3["by_rel"]
    require(t3["labels"] == [], "type-3 has no 0x14 mailbox/1A0")
    sites = [i for i in rows if i["op"] == 0x77]
    require(len(sites) == 4, f"type-3 0x77 count {len(sites)}")
    first = sites[0]
    require(first["rel"] == 0x30, "first 0x77 at +0x30")
    from0 = reachable(by, 0)
    require(0x30 in from0, "main task reaches first 0x77")
    require(by[0]["op"] == 0x02, "type-3 starts 0x02")
    require(by[0x0C]["op"] == 0x5E, "then 0x5E pose copy")
    after = first["rel"] + first["span"]
    alu = by.get(after)
    require(alu is not None and alu["op"] == 0x09, "0x77 then ALU")
    require(alu["imms"][:4] == [0x0B, 0, 4, 1], "local[4]==1")
    skip = by.get(after + alu["span"])
    require(skip is not None and skip["op"] == 0x05, "then 0x05")
    miss = hw(skip["imms"][1])
    require(miss == 0xF8, "miss -> second 0x5E/+0xF8")
    hit = after + alu["span"] + skip["span"]
    require(by[0xB4]["op"] == 0x1C and by[0xB4]["imms"][:3] == [0, 0, 0xFE],
            "hit mails type 0 0xFE")
    require(by[0xC8]["op"] == 0x85, "hit then 0x85")
    require(by[0xEC]["op"] == 0x31, "hit then 0x31 dest hop")
    require(0xB4 in reachable(by, hit), "hit arm owns 0xFE")
    require(by[0x3F0]["op"] == 0x00 and hw(by[0x3F0]["imms"][0]) == 0xC,
            "type-3 loops 0x5E/0x77")
    require(not any(imms and imms[0] == 6 for _, imms in t3["sends"]),
            "type-3 has no 0x1C to type 6")
    for rel, imms in t3["sends"]:
        require(imms[:3] == [0, 0, 0xFE], f"type-3 send {rel:#x} {imms}")


def main() -> int:
    exe = load_exe()
    prove_36448(exe)
    _chunk2, scripts = load_scripts()

    print("=== 36448 ===")
    print("  608 words 0x80036448..0x80036DC8")
    print("  35558@35C1C jal 36448; then 12774; then 360B4")
    print("  D1A0&4 skips the three")
    print("  pair walk: +0x1A0 -> 12700 -> slot A4 (bit0, other type/id)")
    print("  four 12700 sites; 1AC-both path still 1A0-spawns")

    print("=== labels 0x14 ===")
    for scr in scripts:
        labs = []
        for rel, code, tgt in scr["labels"]:
            slot = {1: "1A0", 2: "19C", 3: "task+4"}.get(code, str(code))
            labs.append(f"+0x{rel:X} {slot}=+0x{tgt:X}" if tgt >= 0 else f"+0x{rel:X} {slot}=0")
        print(f"  type {scr['type']}: {labs}")

    print("=== 0x1C sends ===")
    for scr in scripts:
        for rel, imms in scr["sends"]:
            print(f"  type {scr['type']} +0x{rel:04X} {fmt_mail(imms)}")

    print("=== +0x1A0 islands ===")
    a4_mail_to_6 = []
    a4_all_sends = []
    type6_has_1a0 = False
    for scr in scripts:
        for rel, code, tgt in scr["labels"]:
            if code != 1:
                continue
            if scr["type"] == 6:
                type6_has_1a0 = True
            if tgt < 0:
                print(f"  type {scr['type']} +0x{rel:X} clears +0x1A0")
                continue
            sends = island_sends(scr, tgt)
            ops = []
            by = scr["by_rel"]
            seen = reachable(by, tgt)
            for r in sorted(seen):
                insn = by[r]
                if insn["op"] in (0x1C, 0x20, 0x1F, 0x55, 0xAE, 0x6F, 0x89, 0x77):
                    ops.append(fmt_insn(insn))
            print(f"  type {scr['type']} +0x1A0=+0x{tgt:X} from +0x{rel:X}")
            print(f"    ops {ops}")
            print(f"    sends {[(hex(r), fmt_mail(m)) for r, m in sends]}")
            for r, m in sends:
                a4_all_sends.append((scr["type"], tgt, r, m))
                if m and m[0] == 6:
                    a4_mail_to_6.append((scr["type"], tgt, r, m))

    require(not type6_has_1a0, "type-6 never writes +0x1A0")
    require(a4_mail_to_6 == [], f"A4 island mailed type 6: {a4_mail_to_6}")

    # Type-5 first 0x14 is code 1 -> +0xF8, which mails type 0 0xFE.
    t5 = scripts[5]
    require(any(code == 1 and tgt == 0xF8 for _, code, tgt in t5["labels"]),
            "type-5 +0x1A0=+0xF8")
    t5_f8 = island_sends(t5, 0xF8)
    require(any(imms[:3] == [0, 0, 0xFE] for _, imms in t5_f8),
            "type-5 A4 island 0x1C(0,0,0xFE)")
    require(not any(imms and imms[0] == 6 for _, imms in t5_f8),
            "type-5 A4 does not mail type 6")

    print("=== mailbox 0x19C islands ===")
    for scr in scripts:
        for rel, code, tgt in scr["labels"]:
            if code != 2 or tgt < 0:
                continue
            sends = island_sends(scr, tgt)
            print(
                f"  type {scr['type']} +0x19C=+0x{tgt:X} "
                f"sends={[fmt_mail(m) for _, m in sends]}"
            )

    dumps_type3(scripts[3])
    print("=== type-3 0x77 ===")
    t3 = scripts[3]
    for insn in t3["rows"]:
        if insn["op"] == 0x77:
            verts = insn["imms"]
            print(f"  {fmt_insn(insn)} verts={verts}")
    print("  handler 14DA0 / 1CAB0; hit -> 0x85; no type-6 mail")

    t0 = scripts[0]
    t2 = scripts[2]
    t6 = scripts[6]
    by0 = t0["by_rel"]
    by2 = t2["by_rel"]
    by6 = t6["by_rel"]

    # Type-0 mailbox payload arms.
    persist0 = {i: 0 for i in range(128)}
    scratch0 = {0: 0}
    t0_mail = 0x618
    require(by0[t0_mail]["op"] == 0x1F, "type-0 mailbox 0x1F")

    print("=== type-0 mailbox payload CFG ===")
    for pay in (0xFF, 0xFE, 0x81, 0x70, 0x7D, 0x7F, 0x00):
        seen = payload_reach(by0, t0_mail, pay, persist0, scratch0)
        hits = [(rel, imms) for rel, imms in t0["sends"] if rel in seen]
        print(f"  payload 0x{pay:02X} -> {[fmt_mail(m) for _, m in hits]}")
        if pay in (0xFF, 0xFE):
            require(0xCAC not in seen, f"payload 0x{pay:02X} reaches +0xCAC")
            require(0x113C not in seen, f"payload 0x{pay:02X} reaches +0x113C")
            require(not any(m and m[0] == 6 for _, m in hits),
                    f"payload 0x{pay:02X} mails type 6")

    # persist==39 still cannot send 0x81 from the new-game 0xFF arm
    # without taking the 0xFF compare.
    persist39 = dict(persist0)
    persist39[0x4A] = 39
    seen39 = payload_reach(by0, t0_mail, 0xFF, persist39, scratch0)
    print(
        "  persist[0x4A]=39 payload 0xFF -> "
        f"{[fmt_mail(m) for r, m in t0['sends'] if r in seen39]}"
    )
    require(0xCAC not in seen39, "persist 39 + 0xFF still misses +0xCAC")

    # What DOES reach +0xCAC / +0x113C? Scan payload 0..0xFF.
    cac_payloads = []
    p70_payloads = []
    p7f_payloads = []
    for pay in range(256):
        seen = payload_reach(by0, t0_mail, pay, persist0, scratch0)
        if 0xCAC in seen:
            cac_payloads.append(pay)
        if 0x113C in seen:
            p70_payloads.append(pay)
        if 0x1128 in seen:
            p7f_payloads.append(pay)
    print(f"  new-game payloads that reach +0xCAC 0x81: {cac_payloads}")
    print(f"  new-game payloads that reach +0x113C 0x70: {p70_payloads}")
    print(f"  new-game payloads that reach +0x1128 0x7F: {p7f_payloads}")
    require(cac_payloads == [], "new-game persist0 never reaches +0xCAC")
    require(p70_payloads == [0x65], "only 0x65 reaches +0x113C")
    require(p7f_payloads == [0x65], "only 0x65 reaches +0x7F")
    seen65 = payload_reach(by0, t0_mail, 0x65, persist0, scratch0)
    require(0x1128 in seen65 and 0x113C in seen65 and 0x1150 in seen65,
            "0x65 -> 0x7F + 0x70 + scratch bit4")
    persist0_bit2 = dict(persist0)
    persist0_bit2[0] = 4
    seen_d = payload_reach(by0, t0_mail, 0x0D, persist0_bit2, scratch0)
    require(0xCAC in seen_d, "persist[0]&4 + payload 0x0D reaches 0x81")
    require(0xCAC not in payload_reach(by0, t0_mail, 0x0D, persist0, scratch0),
            "0x0D without persist[0]&4 misses 0x81")

    # Type-2 mailbox payload arms.
    print("=== type-2 mailbox payload CFG ===")
    t2_mail = 0xA8
    require(by2[t2_mail]["op"] == 0x1F, "type-2 mailbox 0x1F")
    for pay in (0x7D, 0x7A, 0x79, 0x7E, 0x7F, 0x0B, 0x17, 0x1A):
        seen = payload_reach(by2, t2_mail, pay, persist0, scratch0)
        hits = [fmt_mail(m) for r, m in t2["sends"] if r in seen]
        print(f"  payload 0x{pay:02X} body={0x804 in seen} sends={hits}")
    seen7d = payload_reach(by2, t2_mail, 0x7D, persist0, scratch0)
    require(0x534 in seen7d, "0x7D reaches +0x534 0x84")
    require(0x804 not in seen7d, "0x7D does not reach 0x6F")
    seen7f = payload_reach(by2, t2_mail, 0x7F, persist0, scratch0)
    require(0x804 in seen7f, "0x7F reaches 0x6F")
    require(by2[0x804]["op"] == 0x6F, "+0x804 is 0x6F")
    seen7a = payload_reach(by2, t2_mail, 0x7A, persist0, scratch0)
    require(0x5A8 in seen7a, "0x7A reaches +0x5A8 0x7B")
    seen79 = payload_reach(by2, t2_mail, 0x79, persist0, scratch0)
    require(0x624 in seen79, "0x79 reaches +0x624 0x86")
    seen7e = payload_reach(by2, t2_mail, 0x7E, persist0, scratch0)
    require(0x6E4 in seen7e, "0x7E reaches +0x6E4 0x7C")
    for rel in (0x534, 0x5A8, 0x624, 0x6E4, 0x804):
        require(rel in reachable(by2, t2_mail), f"type-2 +0x{rel:X} on mailbox CFG")
        require(rel not in reachable(by2, 0), f"type-2 +0x{rel:X} not on main entry")

    # Type-6 mailbox payload: which incoming payload reaches +0x1850.
    print("=== type-6 mailbox payload CFG ===")
    t6_mail = 0xD08
    require(by6[t6_mail]["op"] == 0x1F, "type-6 mailbox 0x1F")
    require(by6[t6_mail]["kinds"] == [1] and by6[t6_mail]["imms"][0] == 15,
            "type-6 0x1F -> local[15]")
    t6_hits = []
    t6_55 = []
    t6_70_wait = []
    for pay in range(256):
        seen = payload_reach(by6, t6_mail, pay, persist0, scratch0)
        if 0x1850 in seen:
            t6_hits.append(pay)
        if 0xFC8 in seen:
            t6_55.append(pay)
        if 0x1088 in seen:
            t6_70_wait.append(pay)
    print(f"  new-game payloads +0x1850: {[hex(p) for p in t6_hits]}")
    print(f"  new-game payloads +0xFC8 0x55: {[hex(p) for p in t6_55]}")
    print(f"  new-game payloads +0x1088 scratch&0x10 wait: {[hex(p) for p in t6_70_wait]}")
    require(0x81 in t6_55, "0x81 reaches 0x55")
    require(0x81 not in t6_hits, "0x81 new-game does not set scratch bit 2")
    require(0x70 in t6_70_wait, "0x70 waits scratch[0]&0x10")
    seen70_bit4 = payload_reach(by6, t6_mail, 0x70, persist0, {0: 0x10})
    require(0x1100 in seen70_bit4, "0x70 + scratch bit4 reaches 0xAE")
    require(0x1850 in seen70_bit4, "0x70 + scratch bit4 reaches +0x1850")
    require(0x1850 not in reachable(by6, 0), "+0x1850 not on main task")
    require(by6[0xE6C]["imms"][:4] == [0x0B, 0, 15, 0x81], "0x81 compare")
    require(by6[0x100C]["imms"][:4] == [0x0B, 0, 15, 0x70], "0x70 compare")

    # First-visit dest-enter 0x1C census: no dest-enter 0x1C to type 6.
    dest_enter_sends = []
    for scr in scripts:
        seen = reachable(scr["by_rel"], 0)
        for rel, imms in scr["sends"]:
            if rel in seen:
                dest_enter_sends.append((scr["type"], rel, imms))
    print("=== entry-reachable 0x1C (main task, no mailbox) ===")
    for typ, rel, imms in dest_enter_sends:
        print(f"  type {typ} +0x{rel:04X} {fmt_mail(imms)}")
    require(not any(imms and imms[0] == 6 for _, _, imms in dest_enter_sends),
            "no main-task 0x1C to type 6")

    # Type-1 new-game 0xFF is the only first-visit mail.
    t1 = scripts[1]
    t1_entry_sends = [
        (rel, imms) for rel, imms in t1["sends"] if rel in reachable(t1["by_rel"], 0)
    ]
    require(any(imms[:3] == [0, 0, 0xFF] for _, imms in t1_entry_sends),
            "type-1 entry can mail 0xFF")

    print(
        "PASS: 36448 is the 35558@35C1C proximity pair-walk that 12700s "
        "+0x1A0 into A4; type-6 has no +0x1A0; no A4 island mails type 6; "
        "type-3 0x77 is 14DA0/1CAB0 and cannot mail type 6; new-game "
        "type-0 0xFF/0xFE never reach +0xCAC/+0x113C"
    )
    print(
        f"  type-6 +0x1850 payloads={ [hex(p) for p in t6_hits] } "
        f"type-0 +0xCAC payloads={ [hex(p) for p in cac_payloads] } "
        f"type-0 +0x113C payloads={ [hex(p) for p in p70_payloads] } "
        f"type-0 +0x1128 payloads={ [hex(p) for p in p7f_payloads] }"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
