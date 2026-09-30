/*
 * func_80030534 — 2D distance helper. Squares signed dx/dz from two
 * records and forwards the sum to already-C func_8005186C.
 *
 * VRAM 0x80030534 / file 0x20D34 / size 0x50 (20 words). Non-leaf:
 * frame -0x18, $ra only at 0x10.
 *
 * First (dx)^2 is subu/mult with no gap. Second (dz)^2 is subu / nop /
 * mult: the nop is the ASPSX >= 2.30 mflo/mult two-instruction gap
 * (maspsx nop_mflo_mfhi). era -O2 -G0 + --aspsx-version=2.21 leaves
 * that nop out; 2.30 inserts it and is otherwise byte-identical here.
 * Mid-20210 carve: prefix 0xB24, C 0x50, resume 20D84.s.
 */
extern int func_8005186C(int);

int func_80030534(unsigned char *a0, unsigned char *a1) {
    int dx = (int)*(short *)(a0 + 0x268) - (int)*(short *)(a1 + 0x2A);
    int dz = (int)*(short *)(a0 + 0x26C) - (int)*(short *)(a1 + 0x32);
    return func_8005186C(dx * dx + dz * dz);
}
