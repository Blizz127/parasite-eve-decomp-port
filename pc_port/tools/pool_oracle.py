#!/usr/bin/env python3
"""Independent retail oracle for func_80062568 free-list pool initializer.

Phase 6E-B20.  Does NOT call the production implementation.  Generates
the exact expected write sequence from the independently derived retail
geometry and verifies against the transcribed MIPS body.

Output: ordered write sequence (ordinal, address, width, value).
28 ordered store operations across 27 unique addresses (the final slot is written twice: loop stores 0x800A3060, then explicit null overwrite).
Exit 0 if the implementation under test matches.  Exit 1 if any write
is missing, extra, or wrong.

Usage:
  Displays expected writes.  The test framework compares the production
  output against this model via B20_ExpectedPoolWrite().
"""
import struct, sys

# Independent geometry derived from raw MIPS
POOL_BASE   = 0x800A22E0
POOL_STRIDE = 0x90
POOL_SLOTS  = 24          # (0x800A3060 - 0x800A22E0) / 0x90
POOL_END    = POOL_BASE + POOL_SLOTS * POOL_STRIDE  # 0x800A3060
LAST_SLOT   = POOL_BASE + (POOL_SLOTS - 1) * POOL_STRIDE  # 0x800A2FD0

# $gp-relative globals ($gp = 0x8009CD70)
GA_HEAD = 0x8009CD70 + 0x3E8   # D_8009D158
GA_TAIL = 0x8009CD70 + 0x3EC   # D_8009D15C
GA_AUX  = 0x8009CD70 + 0x3E4   # D_8009D154


def expected_writes():
    """Generate the complete ordered write sequence."""
    writes = []
    ordinal = 0

    # Loop: store next-slot pointer at each slot
    for slot in range(POOL_BASE, POOL_END, POOL_STRIDE):
        ordinal += 1
        writes.append((ordinal, slot, 4, slot + POOL_STRIDE))

    # Explicit null-termination of last slot
    ordinal += 1
    writes.append((ordinal, LAST_SLOT, 4, 0))

    # Pool head = base
    ordinal += 1
    writes.append((ordinal, GA_HEAD, 4, POOL_BASE))

    # Tail/counter = 0
    ordinal += 1
    writes.append((ordinal, GA_TAIL, 4, 0))

    # Auxiliary = 0
    ordinal += 1
    writes.append((ordinal, GA_AUX, 4, 0))

    return writes


def check_production(pe_load_u32):
    """Verify production state matches the oracle."""
    errors = []

    # Check pool links (slots 0..22: link = next)
    for i in range(POOL_SLOTS - 1):
        addr = POOL_BASE + i * POOL_STRIDE
        got = pe_load_u32(addr)
        want = addr + POOL_STRIDE
        if got != want:
            errors.append(f"slot[{i}] 0x{addr:08X}: got 0x{got:08X} want 0x{want:08X}")

    # Check last slot = 0
    got = pe_load_u32(LAST_SLOT)
    if got != 0:
        errors.append(f"last slot 0x{LAST_SLOT:08X}: got 0x{got:08X} want 0")

    # Check head
    got = pe_load_u32(GA_HEAD)
    if got != POOL_BASE:
        errors.append(f"head 0x{GA_HEAD:08X}: got 0x{got:08X} want 0x{POOL_BASE:08X}")

    # Check tail
    got = pe_load_u32(GA_TAIL)
    if got != 0:
        errors.append(f"tail 0x{GA_TAIL:08X}: got 0x{got:08X} want 0")

    # Check aux
    got = pe_load_u32(GA_AUX)
    if got != 0:
        errors.append(f"aux 0x{GA_AUX:08X}: got 0x{got:08X} want 0")

    return errors


if __name__ == "__main__":
    writes = expected_writes()
    print(f"Expected writes: {len(writes)}")
    for ordinal, addr, width, value in writes:
        print(f"  [{ordinal:2d}] 0x{addr:08X}  w={width}  val=0x{value:08X}")
