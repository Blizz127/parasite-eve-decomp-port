#!/usr/bin/env python3
"""Phase 6E-B54E independent oracle: AF54 wait/reissue to AF68.

Verifies the five exclusive wait words and their back-edges against the
SHA-exact executable. Models busy / timeout / complete without importing
production C and without assigning poll=0.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
AF44 = 0x8006AF44
AF4C = 0x8006AF4C
AF54 = 0x8006AF54
AF60 = 0x8006AF60
AF68 = 0x8006AF68
AE04 = 0x8006AE04
AE08 = 0x8006AE08
ADD8 = 0x8006ADD8
POLL = 0x8006E7E8
ISSUE = 0x8006E6A8
NEXT_ISSUE = 0x8006AF88
NEXT_718D0 = 0x8006AFA8
NEXT_30894 = 0x8006B0AC
FONT_FN = 0x800718D0
UNRESOLVED_FN = 0x80030894

WAIT_WORDS = [
    0x0C01B9FA,  # AF54 jal func_8006E7E8
    0x00000000,  # AF58 nop
    0x00409021,  # AF5C addu s2, v0, zero
    0x1640FFA9,  # AF60 bne s2, zero, AE08
    0x00000000,  # AF64 nop
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
    """One retail AF4C/AF54 step. Returns (kind, s0, s2)."""
    if s2 == -1:
        return ("reissue", s0, 1)
    s2 = poll
    if s2 == 0:
        return ("exit", s0, 0)
    if s0 != 0:
        return ("wait", s0, s2)
    return ("texture", s0, s2)


def run_scenarios(data: bytes) -> int:
    n = 0

    # 1. Exclusive 5-word wait window.
    require((AF68 - AF54) // 4 == 5, "5 words")
    for i, want in enumerate(WAIT_WORDS):
        got = load_u32(data, AF54 + i * 4)
        require(got == want, f"word {i} @{AF54 + i * 4:#x} {got:#010x}")
    n += 1

    # 2. jal target and delay nop.
    require(jal_target(load_u32(data, AF54)) == POLL, "jal 6E7E8")
    require(load_u32(data, AF54 + 4) == 0, "jal delay nop")
    n += 1

    # 3. Busy/timeout back-edge lands on AE08, not AF68.
    require(bne_target(AF60, load_u32(data, AF60)) == AE08, "bne AE08")
    require(load_u32(data, AE08) == 0x16000050, "bne s0, AF4C")
    require(bne_target(AE08, load_u32(data, AE08)) == AF4C, "s0!=0 -> AF4C")
    n += 1

    # 4. Timeout reissue is ADD8, canonical s2=1 skips it.
    require(load_u32(data, AF44) == 0x24100001, "s0 = 1")
    require(load_u32(data, AF4C) == 0x1242FFA2, "beq s2, -1, ADD8")
    require(bne_target(AF4C, load_u32(data, AF4C)) == ADD8, "reissue ADD8")
    require(load_u32(data, AE04) == 0x24120001, "s2 = 1 after issue")
    n += 1

    # 5. Canonical locals: first poll, no reissue, 0 exits.
    kind, s0, s2 = wait_step(1, 1, 0)
    require((kind, s0, s2) == ("exit", 1, 0), "canonical first 0 exits")
    n += 1

    # 6. Busy then 0: AE08/AF4C wait, then exit. Not assigned.
    kind, s0, s2 = wait_step(1, 1, 1)
    require((kind, s0, s2) == ("wait", 1, 1), "busy waits")
    kind, s0, s2 = wait_step(s0, s2, 0)
    require((kind, s0, s2) == ("exit", 1, 0), "later 0 exits")
    n += 1

    # 7. Timeout reissues channel 2, then polls again.
    kind, s0, s2 = wait_step(1, 1, -1)
    require((kind, s0, s2) == ("wait", 1, -1), "timeout goes to wait head")
    kind, s0, s2 = wait_step(s0, s2, 99)
    require((kind, s0, s2) == ("reissue", 1, 1), "s2==-1 reissues")
    kind, s0, s2 = wait_step(s0, s2, 0)
    require((kind, s0, s2) == ("exit", 1, 0), "post-reissue 0 exits")
    n += 1

    # 8. AF68 is the exclusive end; later helpers are not this rung.
    require(load_u32(data, AF68) == 0x00008021, "AF68 s0=0")
    require(jal_target(load_u32(data, NEXT_ISSUE)) == ISSUE, "next 6E6A8")
    require(jal_target(load_u32(data, NEXT_718D0)) == FONT_FN, "next 718D0")
    require(jal_target(load_u32(data, NEXT_30894)) == UNRESOLVED_FN,
            "later 30894")
    n += 1

    require(n == 8, "scenario count")
    return n


def main(argv: list[str]) -> int:
    require(len(argv) == 2, "usage: b54e_6ad40_poll_exit_oracle.py executable")
    data = load_exe(pathlib.Path(argv[1]))
    scenarios = run_scenarios(data)
    print(f"B54E oracle: PASS ({scenarios}/{scenarios} scenarios; {argv[1]})")
    print(f"  wait       {AF54:#010x}..{AF68:#010x} (5 words)")
    print("  poll       live func_8006E7E8; s2/poll not assigned")
    print("  exit       0 -> 0x8006af68; 1 -> AE08/AF4C; -1 -> ADD8")
    print("  next       D_800930EE issue via func_8006E6A8 @ 0x8006af88")
    print(f"  unresolved func_80030894 @ {NEXT_30894:#010x}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
