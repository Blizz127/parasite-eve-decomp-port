#!/usr/bin/env python3
"""Phase 6E-B54G independent oracle: B04C wait/reissue to B060.

Verifies the five exclusive second-poll words and their back-edges
against the SHA-exact executable. Models busy / timeout / complete
without importing production C and without assigning poll=0.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
B040 = 0x8006B040
B044 = 0x8006B044
B04C = 0x8006B04C
B054 = 0x8006B054
B058 = 0x8006B058
B060 = 0x8006B060
AF6C = 0x8006AF6C
AF98 = 0x8006AF98
AF9C = 0x8006AF9C
POLL = 0x8006E7E8
ISSUE = 0x8006E6A8
NEXT_ISSUE = 0x8006B080
NEXT_718D0 = 0x8006B0A4
NEXT_30894 = 0x8006B0AC
FONT_FN = 0x800718D0
UNRESOLVED_FN = 0x80030894

WAIT_WORDS = [
    0x0C01B9FA,  # B04C jal func_8006E7E8
    0x00000000,  # B050 nop
    0x00409021,  # B054 addu s2, v0, zero
    0x1640FFD0,  # B058 bne s2, zero, AF9C
    0x00000000,  # B05C nop
]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(addr: int) -> int:
    return addr - 0x80010000 + 0x800


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return (word & 0x3FFFFFF) << 2 | 0x80000000


def bne_target(pc: int, word: int) -> int:
    disp = word & 0xFFFF
    if disp >= 0x8000:
        disp -= 0x10000
    return pc + 4 + disp * 4


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    require(digest == SHA1, f"exe sha1 {digest}")
    return data


def wait_step(s0: int, s2: int, poll: int) -> tuple[str, int, int]:
    """One retail B044/B04C step. Returns (kind, s0, s2)."""
    if s2 == -1:
        return ("reissue", s0, 1)
    s2 = poll
    if s2 == 0:
        return ("exit", s0, 0)
    if s0 != 0:
        return ("wait", s0, s2)
    return ("font", s0, s2)


def run_scenarios(data: bytes) -> int:
    n = 0

    # 1. Exclusive 5-word wait window.
    require((B060 - B04C) // 4 == 5, "5 words")
    for i, want in enumerate(WAIT_WORDS):
        got = load_u32(data, B04C + i * 4)
        require(got == want, f"word {i} @{B04C + i * 4:#x} {got:#010x}")
    n += 1

    # 2. jal target and delay nop.
    require(jal_target(load_u32(data, B04C)) == POLL, "jal 6E7E8")
    require(load_u32(data, B04C + 4) == 0, "jal delay nop")
    n += 1

    # 3. Busy/timeout back-edge lands on AF9C, not B060.
    require(bne_target(B058, load_u32(data, B058)) == AF9C, "bne AF9C")
    require(load_u32(data, AF9C) == 0x16000029, "bne s0, B044")
    require(bne_target(AF9C, load_u32(data, AF9C)) == B044, "s0!=0 -> B044")
    n += 1

    # 4. Timeout reissue is AF6C, canonical s2=1 skips it.
    require(load_u32(data, B040) == 0x24100001, "s0 = 1")
    require(load_u32(data, B044) == 0x1242FFC9, "beq s2, -1, AF6C")
    require(bne_target(B044, load_u32(data, B044)) == AF6C, "reissue AF6C")
    require(load_u32(data, AF98) == 0x24120001, "s2 = 1 after issue")
    n += 1

    # 5. Canonical locals: first poll, no reissue, 0 exits.
    kind, s0, s2 = wait_step(1, 1, 0)
    require((kind, s0, s2) == ("exit", 1, 0), "canonical first 0 exits")
    n += 1

    # 6. Busy then 0: AF9C/B044 wait, then exit. Not assigned.
    kind, s0, s2 = wait_step(1, 1, 1)
    require((kind, s0, s2) == ("wait", 1, 1), "busy waits")
    kind, s0, s2 = wait_step(s0, s2, 0)
    require((kind, s0, s2) == ("exit", 1, 0), "later 0 exits")
    n += 1

    # 7. Timeout reissues D_800930EE, then polls again.
    kind, s0, s2 = wait_step(1, 1, -1)
    require((kind, s0, s2) == ("wait", 1, -1), "timeout goes to wait head")
    kind, s0, s2 = wait_step(s0, s2, 99)
    require((kind, s0, s2) == ("reissue", 1, 1), "s2==-1 reissues")
    kind, s0, s2 = wait_step(s0, s2, 0)
    require((kind, s0, s2) == ("exit", 1, 0), "post-reissue 0 exits")
    n += 1

    # 8. B060 is the exclusive end; later helpers are not this rung.
    require(load_u32(data, B060) == 0x00008021, "B060 s0=0")
    require(jal_target(load_u32(data, NEXT_ISSUE)) == ISSUE, "next 6E6A8")
    require(jal_target(load_u32(data, NEXT_718D0)) == FONT_FN, "next 718D0")
    require(jal_target(load_u32(data, NEXT_30894)) == UNRESOLVED_FN,
            "later 30894")
    n += 1

    require(n == 8, "scenario count")
    return n


def main(argv: list[str]) -> int:
    require(len(argv) == 2, "usage: b54g_6ad40_second_poll_oracle.py executable")
    data = load_exe(pathlib.Path(argv[1]))
    scenarios = run_scenarios(data)
    print(f"B54G oracle: PASS ({scenarios}/{scenarios} scenarios; {argv[1]})")
    print(f"  wait       {B04C:#010x}..{B060:#010x} (5 words)")
    print("  poll       live func_8006E7E8; s2/poll not assigned")
    print("  exit       0 -> 0x8006b060; 1 -> AF9C/B044; -1 -> AF6C")
    print("  parked     6AD40 sequence; D_800930F0 / 30894 not this rung")
    print(f"  unresolved func_80030894 @ {NEXT_30894:#010x}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
