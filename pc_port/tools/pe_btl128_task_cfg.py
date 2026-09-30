#!/usr/bin/env python3
"""PE-BTL128 — m0005i task CFG and retail 3F074 dest-enter order.

EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
m0005i chunk2 SHA-256 01a64ba3…7e3b.

Challenges the control-flow / task model around type-6 +0x190
vs +0x1850. Does not invent a scratch[0] writer.
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


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_target(word: int) -> int:
    return 0x80000000 | ((word & 0x3FFFFFF) << 2)


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
            out += chunk
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


def cfg_successors(insn: dict, blocked_skip: set[int] | None = None) -> list[int]:
    rel = insn["rel"]
    op = insn["op"]
    imms = insn["imms"]
    nxt = rel + insn["span"]
    outs: list[int] = []
    if op == 0x00 and imms:
        outs.append(hw(imms[0]))
        return outs
    if op == 0x05 and len(imms) > 1:
        outs.append(nxt)
        if blocked_skip is None or rel not in blocked_skip:
            outs.append(hw(imms[1]))
        return outs
    if op == 0x1D and len(imms) > 2:
        outs.append(nxt)
        outs.append(hw(imms[2]))
        return outs
    outs.append(nxt)
    return outs


def reachable(
    by_rel: dict[int, dict],
    start: int,
    blocked_skip: set[int] | None = None,
) -> set[int]:
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
        for nxt in cfg_successors(insn, blocked_skip):
            if nxt not in seen:
                stack.append(nxt)
    return seen


def label_sites(rows: list[dict]) -> list[tuple[int, int, int]]:
    """(rel, code, target_rel) for 0x14 with non-negative rel."""
    out = []
    for insn in rows:
        if insn["op"] != 0x14 or len(insn["imms"]) < 2:
            continue
        code = insn["imms"][0]
        rel = insn["imms"][1]
        if rel < 0:
            out.append((insn["rel"], code, -1))
        else:
            out.append((insn["rel"], code, hw(rel)))
    return out


def send_1c(rows: list[dict]) -> list[tuple[int, list[int]]]:
    out = []
    for insn in rows:
        if insn["op"] != 0x1C:
            continue
        out.append((insn["rel"], insn["imms"][:]))
    return out


def kind4_writes(rows: list[dict]) -> list[tuple[int, int, list[int], list[int]]]:
    out = []
    for insn in rows:
        op = insn["op"]
        kinds = insn["kinds"]
        imms = insn["imms"]
        dest_kind4 = False
        if op in (0x0A, 0x2A, 0x28) and kinds and kinds[0] == 4:
            dest_kind4 = True
        if op == 0x09 and len(kinds) > 1 and kinds[1] == 4:
            dest_kind4 = True
        if dest_kind4:
            out.append((insn["rel"], op, kinds, imms))
    return out


def prove_3f074_order(exe: bytes) -> None:
    require(jal_target(load_u32(exe, 0x8003F07C)) == 0x8006B35C, "3F074 jal 6B35C")
    require(jal_target(load_u32(exe, 0x8003F088)) == 0x8006B4F8, "3F074 jal 6B4F8")
    require(jal_target(load_u32(exe, 0x8003F090)) == 0x8006BD68, "3F074 jal 6BD68")
    require(jal_target(load_u32(exe, 0x8003F0B0)) == 0x80034FC4, "3F074 jal 34FC4")
    require(jal_target(load_u32(exe, 0x8003F0B8)) == 0x8001266C, "3F074 jal 1266C")
    require(jal_target(load_u32(exe, 0x8003F204)) == 0x8006BE4C, "3F074 jal 6BE4C")
    require(jal_target(load_u32(exe, 0x8003F20C)) == 0x8006BECC, "3F074 jal 6BECC")
    require(jal_target(load_u32(exe, 0x8003F224)) == 0x8006C4C4, "3F074 jal 6C4C4")
    require(jal_target(load_u32(exe, 0x8003F22C)) == 0x8006C5BC, "3F074 jal 6C5BC")
    require(jal_target(load_u32(exe, 0x8003F23C)) == 0x8001A918, "3F074 jal 1A918")
    require(jal_target(load_u32(exe, 0x8003F274)) == 0x800371B0, "3F074 jal 371B0")
    require(jal_target(load_u32(exe, 0x8003F27C)) == 0x800125E0, "3F074 jal 125E0")
    require(0x8003F0B8 < 0x8003F204 < 0x8003F20C < 0x8003F23C < 0x8003F27C,
            "1266C before 6BECC before 1A918 before 125E0")
    # No branch before 1266C: first beq is 3F0E4.
    require(load_u32(exe, 0x8003F0E4) == 0x10400002, "first beq after 1266C")
    require(load_u32(exe, 0x8003F0A0) == 0xAC22D224, "sw D224=1")
    require(load_u32(exe, 0x8003F0AC) == 0xA422D308, "sh D308=1")


def main() -> int:
    exe = load_exe()
    prove_3f074_order(exe)

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
                "kind4": kind4_writes(rows),
            }
        )

    desc_off = LIST_OFF + struct.unpack_from("<I", chunk2, LIST_OFF)[0]
    require(chunk2[desc_off] == 2, "125E0 count")
    require(chunk2[desc_off + 1] == 1 and chunk2[desc_off + 2] == 0, "desc type 1")
    require(chunk2[desc_off + 3] == 6 and chunk2[desc_off + 4] == 0, "desc type 6")

    t6 = scripts[6]
    t2 = scripts[2]
    t0 = scripts[0]
    t1 = scripts[1]
    by6 = t6["by_rel"]

    require(by6[0x190]["op"] == 0x09, "wait AND")
    require(by6[0x190]["kinds"] == [0, 3, 4, 0], "wait dest cond / scratch[0]")
    require(by6[0x190]["imms"][0] == 3 and by6[0x190]["imms"][2:] == [0, 4],
            "AND scratch[0]&4")
    require(by6[0x1C0]["op"] == 0x05, "wait skip")
    require(hw(by6[0x1C0]["imms"][1]) == 0x1E8, "escape +0x1E8")
    require(by6[0x1D0]["op"] == 0x02, "wait yield")
    require(by6[0x1DC]["op"] == 0x00, "wait goto")
    require(hw(by6[0x1DC]["imms"][0]) == 0x190, "goto +0x190")
    require(by6[0x1850]["op"] == 0x2A, "+0x1850 0x2A")
    require(by6[0x1850]["kinds"][:2] == [4, 0], "+0x1850 kind4")
    require(by6[0x1850]["imms"][:2] == [0, 2], "+0x1850 bit 2")
    require(by6[0x180]["op"] == 0x14, "mailbox label")
    require(by6[0x180]["imms"][:2] == [2, 1668], "code 2 rel 1668")
    require(hw(1668) == 0xD08, "1668<<1 = +0xD08")

    from0 = reachable(by6, 0)
    from0_no_escape = reachable(by6, 0, blocked_skip={0x1C0})
    from_d08 = reachable(by6, 0xD08)
    from_escape = reachable(by6, 0x1E8)

    require(0x190 in from0, "entry reaches wait")
    require(0x1DC in from0, "entry reaches yield")
    require(0x1850 not in from0, "+0x1850 is not on the main-task CFG")
    require(0x1850 not in from0_no_escape, "+0x1850 not on the wait loop")
    require(0x1850 not in from_escape, "+0x1850 is not after the 0x05 escape")
    require(0x1850 in from_d08, "+0x1850 is only on the +0xD08 mailbox island")
    require(0x190 not in from_d08, "mailbox +0xD08 does not re-enter +0x190")
    require(by6[0xFC8]["op"] == 0x55, "+0xFC8 is 0x55")
    require(0xFC8 in from_d08 and 0xFC8 not in from0, "0x55 is mailbox-island only")
    require(by6[0x1100]["op"] == 0xAE, "+0x1100 is 0xAE")
    require(0x1100 in from_d08 and 0x1100 not in from0, "0xAE is mailbox-island only")
    require(by6[0xFAC]["op"] == 0x2A and by6[0xFAC]["imms"][:2] == [0, 3],
            "+0xFAC is 0x2A bit 3, not 0x55")
    require(by6[0x3F8]["op"] == 0x1C and by6[0x3F8]["imms"] == [2, 0, 0x7D],
            "escape path 0x1C(2,0,0x7D)")
    require(0x3F8 in from_escape, "0x7D send is after escape")
    require(0x3F8 not in from0_no_escape, "0x7D send not on the wait loop")
    require(by6[0x40C]["op"] == 0x20, "main task parks after 0x7D")
    require(by6[0xD08]["op"] == 0x1F, "mailbox starts 0x1F")
    require(by6[0xD00]["op"] == 0x20 and 0xD00 not in from0,
            "+0xD00 park is dead linear padding, not a main-task PC")

    labels6 = t6["labels"]
    require(labels6 == [(0x180, 2, 0xD08)], "type-6 only 0x14 is +0x19C=+0xD08")
    require(any(code == 2 and tgt == 0xA8 for _, code, tgt in t2["labels"]),
            "type-2 mailbox +0xA8")
    require(any(code == 2 and tgt == 0x200 for _, code, tgt in t1["labels"]),
            "type-1 mailbox +0x200")
    require(any(code == 2 and tgt == 0x618 for _, code, tgt in t0["labels"]),
            "type-0 mailbox +0x618")

    require((0x3F8, [2, 0, 0x7D]) in t6["sends"], "type-6 0x1C(2,0,0x7D)")

    type6_inbox = []
    for scr in scripts:
        for rel, imms in scr["sends"]:
            if imms and imms[0] == 6:
                type6_inbox.append((scr["type"], rel, imms))
    require(
        type6_inbox
        == [
            (0, 0xCAC, [6, 0, 0x81]),
            (0, 0x113C, [6, 0, 0x70]),
            (2, 0x534, [6, 0, 0x84]),
            (2, 0x5A8, [6, 0, 0x7B]),
            (2, 0x624, [6, 0, 0x86]),
            (2, 0x6E4, [6, 0, 0x7C]),
        ],
        f"type-6 inbox {type6_inbox}",
    )

    by1 = t1["by_rel"]
    require(by1[0x198]["imms"][:4] == [11, 0, 74, 39], "type-1 persist[0x4A]==39")
    require(hw(by1[0x1B0]["imms"][1]) == 0x1CC, "!=39 takes 0x86/0x1C")
    require(by1[0x1C0]["op"] == 0x00 and hw(by1[0x1C0]["imms"][0]) == 0x1EC,
            "==39 skips 0x1C(0,0,0xFF)")
    require(by1[0x1D8]["imms"][:3] == [0, 0, 255], "new-game type-1 mails type-0 0xFF")
    require(by1[0x1F8]["op"] == 0x20, "type-1 parks after 0xFF")
    require(by1[0x12C]["imms"][0] == 2, "type-1 0x08 type 2")

    by0 = t0["by_rel"]
    require(by0[0x618]["op"] == 0x1F, "type-0 mailbox 0x1F")
    require(by0[0x6BC]["imms"][:4] == [11, 0, 4, 255], "0xFF compare")
    require(hw(by0[0x708]["imms"][0]) == 0x2F4, "0xFF goto persist-39 region")
    require(hw(by0[0x30C]["imms"][1]) == 0x608, "persist!=39 parks")
    require(by0[0x608]["op"] == 0x20, "type-0 0x20")
    require(by0[0xCAC]["imms"][:3] == [6, 0, 0x81], "later type-0 0x81 to type 6")
    require(0xCAC > 0x618, "0x81 send is after mailbox entry")

    require((0x1128, [2, 0, 0x7F]) in t0["sends"], "type-0 0x7F")
    require(t2["by_rel"][0x98]["op"] == 0x20, "type-2 parks")
    require(t2["by_rel"][0xA8]["op"] == 0x1F, "type-2 mailbox 0x1F")
    require(t2["by_rel"][0x804]["op"] == 0x6F, "type-2 0x6F")

    bit2 = []
    for scr in scripts:
        for rel, op, kinds, imms in scr["kind4"]:
            if op == 0x2A and imms[:2] == [0, 2]:
                bit2.append((scr["type"], rel))
    require(bit2 == [(6, 0x1850)], f"bit2 {bit2}")

    t1_08 = [i["imms"][0] for i in t1["rows"] if i["op"] == 0x08]
    require(t1_08 == [3, 0, 5, 2, 4], f"type-1 0x08 types {t1_08}")
    require(not any(i["op"] == 0x08 for i in t6["rows"]), "type-6 has no 0x08")

    first_ops = {s["type"]: s["rows"][0]["op"] for s in scripts if s["rows"]}
    require(first_ops[1] == 0xEA, "type-1 starts 0xEA")
    require(first_ops[6] == 0xCE, "type-6 starts 0xCE")

    print(
        "PASS: 3F074 zeros 1266C before 6BECC/1A918/125E0; "
        "type-6 +0x1850 is a +0xD08 mailbox-island PC, not the +0x190 task; "
        "new-game type-1 0xFF wakes type-0 mailbox onto persist!=39 park"
    )
    print(
        f"  from0={len(from0)} from0_no_escape={len(from0_no_escape)} "
        f"from_d08={len(from_d08)} from_escape={len(from_escape)}"
    )
    print(
        f"  type-6 labels={labels6} type-2 labels={t2['labels']} "
        f"type-1 labels={t1['labels']} type-0 labels={t0['labels']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
