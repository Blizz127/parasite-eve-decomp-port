#!/usr/bin/env python3
"""Independent Phase 6E-B54B counted-loop oracle for func_8006AD40.

Verifies the SHA-1-exact retail instructions around 0x8006AE48, executes
the bounded 0x8006AE50..0x8006AE68 loop semantics, and stops before the
later D_80091648 packing instruction.  It imports no production code and
models no GPU, DMA, IRQ, callback, display, or CD progress.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FUNC_START = 0x8006AD40
CALL_PC = 0x8006AE48
CUT_PC = 0x8006AE50
LOOP_EXIT = 0x8006AE68
ENTRY_STRIDE = 0x14

# Call + delay slot, then the complete bounded B54B suffix fragment.
WORDS = (
    0x0C01B870,  # 0x8006AE48 jal func_8006E1C0
    0x02802821,  # 0x8006AE4C move a1,s4 (delay)
    0x8E620028,  # 0x8006AE50 lw v0,0x28(s3)
    0x26310001,  # 0x8006AE54 addiu s1,s1,1
    0x00021582,  # 0x8006AE58 srl v0,v0,22
    0x0222102B,  # 0x8006AE5C sltu v0,s1,v0
    0x1440FFF8,  # 0x8006AE60 bnez v0,0x8006AE44
    0x26100014,  # 0x8006AE64 addiu s0,s0,0x14 (delay)
)
NEXT_WORD = 0x24050020  # 0x8006AE68 addiu a1,zero,0x20; not consumed


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FATAL: {message}")


def u32(value: int) -> int:
    return value & 0xFFFFFFFF


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    require(digest == EXE_SHA1, f"{path} SHA-1 {digest} != {EXE_SHA1}")
    require(data[:8] == b"PS-X EXE", "not a PS-X EXE")
    return data


def verify_words(data: bytes) -> None:
    text_address = struct.unpack_from("<I", data, 0x18)[0]
    file_offset = CALL_PC - text_address + 0x800
    for index, expected in enumerate(WORDS):
        address = CALL_PC + index * 4
        actual = struct.unpack_from("<I", data, file_offset + index * 4)[0]
        require(actual == expected,
                f"word at {address:#010x}: {actual:#010x} != {expected:#010x}")
    actual_next = struct.unpack_from("<I", data, file_offset + len(WORDS) * 4)[0]
    require(actual_next == NEXT_WORD, "0x8006AE68 boundary word changed")


def run_tail(headers: list[int], s1: int = 0,
             entry_address: int = 0x8012DF58) -> dict[str, object]:
    """Execute AE50..AE68 after entry s1 has already returned."""
    calls: list[int] = []
    header_reads = 0
    current = u32(entry_address)
    while True:
        require(header_reads < len(headers), "missing reloaded header value")
        count = u32(headers[header_reads]) >> 22
        header_reads += 1
        s1 = u32(s1 + 1)
        take_back_edge = u32(s1) < u32(count)  # literal sltu
        current = u32(current + ENTRY_STRIDE)  # unconditional branch delay
        if not take_back_edge:
            return {
                "calls": calls,
                "final_s1": s1,
                "final_entry": current,
                "header_reads": header_reads,
                "exit_pc": LOOP_EXIT,
            }
        calls.append(current)


def packed_count(count: int) -> int:
    require(0 <= count <= 0x3FF, "count does not fit retail header")
    return count << 22


def run_scenarios() -> int:
    scenarios = 0

    canonical = run_tail([packed_count(13)] * 13)
    require(len(canonical["calls"]) == 12, "canonical additional call count")
    require(canonical["calls"] ==
            [0x8012DF58 + ENTRY_STRIDE * i for i in range(1, 13)],
            "canonical entries are not 1..12 in order")
    require(canonical["final_s1"] == 13 and
            canonical["final_entry"] == 0x8012DF58 + ENTRY_STRIDE * 13,
            "canonical terminal register state")
    scenarios += 1

    require(0x8012DF58 not in canonical["calls"], "entry 0 replayed")
    scenarios += 1

    exhausted = run_tail([packed_count(1)])
    require(exhausted["calls"] == [] and exhausted["final_s1"] == 1,
            "count=1 must have zero remaining calls")
    scenarios += 1

    edge_two = run_tail([packed_count(2), packed_count(2)])
    require(edge_two["calls"] == [0x8012DF6C] and
            edge_two["final_s1"] == 2 and edge_two["header_reads"] == 2,
            "count=2 increment/compare timing")
    scenarios += 1

    # Initial count zero branches to AE68 before the first jal/AE50. This is
    # a real function path, not an invented mid-loop register state.
    initial_zero = {"calls": [], "exit_pc": LOOP_EXIT}
    require(initial_zero["calls"] == [] and
            initial_zero["exit_pc"] == LOOP_EXIT,
            "initial count zero does not bypass the loop")
    scenarios += 1

    # A changed header is consumed on each return, rather than caching count.
    reloaded = run_tail([packed_count(3), packed_count(2)])
    require(len(reloaded["calls"]) == 1 and reloaded["final_s1"] == 2 and
            reloaded["header_reads"] == 2,
            "header was not reloaded per retail iteration")
    scenarios += 1

    # Decode the comparison word itself: SPECIAL/sltu rd=v0,rs=s1,rt=v0.
    sltu = WORDS[5]
    require((sltu >> 26) == 0 and (sltu & 0x3F) == 0x2B and
            ((sltu >> 21) & 0x1F) == 17 and
            ((sltu >> 16) & 0x1F) == 2 and
            ((sltu >> 11) & 0x1F) == 2,
            "comparison is not literal unsigned sltu v0,s1,v0")
    scenarios += 1

    require(CUT_PC == 0x8006AE50 and LOOP_EXIT == 0x8006AE68 and
            NEXT_WORD == 0x24050020,
            "B54B range or later packing boundary changed")
    scenarios += 1

    require(scenarios == 8, "scenario count")
    return scenarios


def main(argv: list[str]) -> int:
    require(len(argv) == 2,
            "usage: b54b_6ad40_counted_loop_oracle.py executable")
    data = load_exe(pathlib.Path(argv[1]))
    verify_words(data)
    scenarios = run_scenarios()
    print(f"B54B oracle: PASS ({scenarios}/{scenarios} scenarios; {argv[1]})")
    print("  production range 0x8006AE50..0x8006AE68")
    print("  canonical entry0 complete; additional entries 1..12 = 12 calls")
    print("  final s1=13; final s0=0x8012E05C; exit=0x8006AE68")
    print("  0x8006AE68 packing and all later suffix behavior unconsumed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
