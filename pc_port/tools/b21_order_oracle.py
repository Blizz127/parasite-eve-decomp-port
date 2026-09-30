#!/usr/bin/env python3
"""Independent ordered-write oracle for func_80064964."""
from __future__ import annotations

B21_DST = 0x800A3060
B21_LEN = 0x120
FLAGS = (0x800A3078, 0x800A30A0, 0x800A30B0, 0x800A30B8,
         0x800A30C0, 0x800A30C4, 0x800A3124, 0x800A3134)


def expected_writes():
    # A(28h) writes every byte first; the retail sb instructions then write
    # exactly these eight bytes in ROM order.
    return [(address, 0) for address in range(B21_DST, B21_DST + B21_LEN)] + [
        (address, 0xFF) for address in FLAGS
    ]


def final_bytes():
    result = {address: 0 for address in range(B21_DST, B21_DST + B21_LEN)}
    for address in FLAGS:
        result[address] = 0xFF
    return result


def validate(observed) -> None:
    """Raise AssertionError unless an observed write log is exact."""
    expected = expected_writes()
    assert list(observed) == expected, "func_80064964 write order/width mismatch"
    assert observed[:B21_LEN] == [(a, 0) for a in range(B21_DST, B21_DST + B21_LEN)]
    assert observed[B21_LEN:] == [(a, 0xFF) for a in FLAGS]


if __name__ == "__main__":
    print("writes", len(expected_writes()), "first", expected_writes()[0],
          "last", expected_writes()[-1])
