/*
 * func_80044E14 — slot teardown helper (retail 0x80044E14).
 *
 * VRAM 0x80044E14 / file 0x35614 / size 0x84 (33 words). Head of the
 * 35614.s run; func_80044DCC precedes it.
 *
 * Reads a slot index from arg0+0x24, rebases it by -0x29, then:
 *   1. func_8005E8A4(0, 0xA)  — fade/state setup,
 *   2. func_8005EB58(0),
 *   3. func_8005F594(D_800A1980 + idx * 64),
 *   4. r = D_8009CFA0[idx]; if r != 0: func_8005E8A4(0, 0xE) and
 *      r = func_80062A7C(D_8009CFA0[idx]) (the load is repeated at the
 *      call site, in retail's `lw $a0,0($s0)` slot).
 * Returns r (0 when the table entry is empty).
 *
 * era -O2 -G0. The reload must be spelled as the ARRAY expression
 * `D_8009CFA0[idx]`, not through a live `int *p` local: with the pointer
 * pinned across the calls cc1 keeps the pointer in $s0 and the value in
 * $v0 (matching retail); the pointer-local form instead sinks the table
 * load into $s0 and shifts the epilogue.
 */
extern unsigned char D_800A1980[];
extern int D_8009CFA0[];
extern int func_8005E8A4(int, int);
extern int func_8005EB58(int);
extern int func_8005F594(unsigned char *);
extern int func_80062A7C(int);

int func_80044E14(unsigned char *arg0) {
    int idx = *(int *)(arg0 + 0x24) - 0x29;
    int r;
    func_8005E8A4(0, 0xA);
    func_8005EB58(0);
    func_8005F594(D_800A1980 + (idx << 6));
    r = D_8009CFA0[idx];
    if (r != 0) {
        func_8005E8A4(0, 0xE);
        r = func_80062A7C(D_8009CFA0[idx]);
    }
    return r;
}
