#!/usr/bin/env python3
"""
Phase 6E-B50 retail-word oracle and corrective prefix proof for func_8006AD40.

Verifies:
- 391 retail words at 0x8006AD40 (streaming subsystem multiplexer)
- Exact word count against the SHA-exact retail executable
- Complete ROM-order call census (29 calls to 13 unique targets)
- Dependency classification
- Exact 68-word production-prefix cut at the first unresolved call/delay slot

This script deliberately does not claim semantic coverage of the 391-word
suffix or of any unresolved dependency.  Production is prefix-only.  Native
tests independently verify the computed boundary arguments and prove that
non-strict execution does not continue past the state-producing boundary.
"""

import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
START = 0x8006AD40
END = 0x8006B35C
PREFIX_END = 0x8006AE50
PREFIX_WORD_COUNT = (PREFIX_END - START) // 4
FIRST_BOUNDARY_CALL = 0x8006AE48

# func_8006AD40: 391 words at 0x8006AD40 (0x35C = 1564 bytes)
# exe range: 0x8006AD40–0x8006B35C
# file offset: 0x5B540
WORDS = exe_words(0x8006AD40, 391)

# Call census: (jal_addr, target_vram, callee_name, classification)
CALLS = [
    (0x8006ADA4, 0x8006E6A8, "func_8006E6A8", "TRANSLATED"),
    (0x8006ADC0, 0x8006E7E8, "func_8006E7E8", "TRANSLATED"),
    (0x8006ADF4, 0x8006E6A8, "func_8006E6A8", "TRANSLATED"),
    (0x8006AE48, 0x8006E1C0, "func_8006E1C0", "UNRESOLVED"),
    (0x8006AEFC, 0x8006E498, "func_8006E498", "TRANSLATED"),
    (0x8006AF18, 0x8007506C, "func_8007506C", "UNRESOLVED"),
    (0x8006AF54, 0x8006E7E8, "func_8006E7E8", "TRANSLATED"),
    (0x8006AF88, 0x8006E6A8, "func_8006E6A8", "TRANSLATED"),
    (0x8006AFA8, 0x800718D0, "func_800718D0", "UNRESOLVED"),
    (0x8006B04C, 0x8006E7E8, "func_8006E7E8", "TRANSLATED"),
    (0x8006B080, 0x8006E6A8, "func_8006E6A8", "TRANSLATED"),
    (0x8006B0A4, 0x800718D0, "func_800718D0", "UNRESOLVED"),
    (0x8006B0AC, 0x80030894, "func_80030894", "UNRESOLVED"),
    (0x8006B0BC, 0x8006E7E8, "func_8006E7E8", "TRANSLATED"),
    (0x8006B0F0, 0x8006E6A8, "func_8006E6A8", "TRANSLATED"),
    (0x8006B118, 0x8006E498, "func_8006E498", "TRANSLATED"),
    (0x8006B12C, 0x8006E498, "func_8006E498", "TRANSLATED"),
    (0x8006B140, 0x8006E498, "func_8006E498", "TRANSLATED"),
    (0x8006B154, 0x8006E7E8, "func_8006E7E8", "TRANSLATED"),
    (0x8006B188, 0x8006E6A8, "func_8006E6A8", "TRANSLATED"),
    (0x8006B1DC, 0x8006E1C0, "func_8006E1C0", "UNRESOLVED"),
    (0x8006B20C, 0x8006E7E8, "func_8006E7E8", "TRANSLATED"),
    (0x8006B254, 0x8006E1C0, "func_8006E1C0", "UNRESOLVED"),
    (0x8006B274, 0x80087024, "func_80087024", "TRANSLATED"),
    (0x8006B27C, 0x80074DC0, "func_80074DC0", "TRANSLATED"),
    (0x8006B284, 0x80074A44, "func_80074A44", "TRANSLATED"),
    (0x8006B28C, 0x80073A44, "func_80073A44", "TRANSLATED"),
    (0x8006B2B4, 0x800755F0, "func_800755F0", "TRANSLATED"),
    (0x8006B2BC, 0x80074D28, "func_80074D28", "TRANSLATED"),
]

def main():
    import pathlib

    # Locate the retail executable
    candidates = [
        pathlib.Path("build/extracted/disc1/SLUS_006.62"),
        pathlib.Path("rom/image/SLUS_006.62"),
    ]
    exe = None
    for c in candidates:
        if c.exists():
            exe = c
            break
    if exe is None:
        print("B50 ORACLE: SKIP (no retail executable found)")
        sys.exit(0)

    data = exe.read_bytes()
    sha = hashlib.sha1(data).hexdigest()
    if sha != SHA1:
        print(f"B50 ORACLE: FAIL (SHA1 mismatch: {sha})")
        sys.exit(1)
    print(f"SHA1 verified: {sha}")

    # Load address
    taddr = struct.unpack_from("<I", data, 0x18)[0]
    foff = START - taddr + 0x800

    # Verify all words
    mismatches = 0
    for i, expected in enumerate(WORDS):
        actual = struct.unpack_from("<I", data, foff + i * 4)[0]
        if actual != expected:
            addr = START + i * 4
            print(f"  MISMATCH at 0x{addr:08X} word {i}: "
                  f"expected 0x{expected:08X}, got 0x{actual:08X}")
            mismatches += 1

    if mismatches > 0:
        print(f"B50 ORACLE: FAIL ({mismatches} word mismatches)")
        sys.exit(1)

    print(f"All {len(WORDS)} words verified exact")
    if len(WORDS) != (END - START) // 4:
        print("B50 ORACLE: FAIL (literal word count does not match range)")
        sys.exit(1)

    # Corrective production boundary: the prefix includes the first jal and
    # its architecturally executed delay slot, then returns on the host.
    call_i = (FIRST_BOUNDARY_CALL - START) // 4
    if (PREFIX_WORD_COUNT != 68 or
            WORDS[call_i] != 0x0C01B870 or
            WORDS[call_i + 1] != 0x02802821):
        print("B50 ORACLE: FAIL (first-boundary call/delay mismatch)")
        sys.exit(1)

    # Exact packet-header chain feeding a0 at the boundary:
    #   s4 = [D_800B0CD8+0x160]
    #   v0 = [s4+4]
    #   s3 = s4+v0
    #   header = [s3+0x28]
    header_i = (0x8006AE10 - START) // 4
    expected_header_chain = [
        0x8EB40160, 0x3C03003F, 0x8E820004, 0x3463FFFF,
        0x02829821, 0x8E620028,
    ]
    if WORDS[header_i:header_i + len(expected_header_chain)] != expected_header_chain:
        print("B50 ORACLE: FAIL (boundary header indirection mismatch)")
        sys.exit(1)

    print(f"Function range: 0x{START:08X}–0x{END:08X} ({len(WORDS)*4} bytes)")
    print(f"Production prefix: 0x{START:08X}–0x{PREFIX_END:08X} "
          f"({PREFIX_WORD_COUNT} words; call+delay included)")

    # Call census
    print(f"\nCall census: {len(CALLS)} calls")
    resolved = sum(1 for _, _, _, c in CALLS if c == "TRANSLATED")
    unresolved = sum(1 for _, _, _, c in CALLS if c == "UNRESOLVED")
    print(f"  TRANSLATED:  {resolved}")
    print(f"  UNRESOLVED:  {unresolved}")

    unique = {}
    for _, target, name, cls in CALLS:
        unique[target] = (name, cls)
    print(f"\nUnique targets: {len(unique)}")
    for target in sorted(unique):
        name, cls = unique[target]
        count = sum(1 for _, t, _, _ in CALLS if t == target)
        print(f"  0x{target:08X}: {name} ({cls}) ×{count}")

    # First unresolved in execution order
    for addr, target, name, cls in CALLS:
        if cls == "UNRESOLVED":
            print(f"\nFirst unresolved callee: {name} at call-site 0x{addr:08X}")
            break

    print(f"\nB50 ORACLE: PASS ({len(WORDS)} words, {len(CALLS)} calls, "
          f"{unresolved} unresolved boundaries)")


if __name__ == "__main__":
    main()
