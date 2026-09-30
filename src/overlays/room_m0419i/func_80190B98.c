/* room_m0419i (PE.IMG room m0419i chunk 2, VRAM 0x8018EFE8)
 * func_80190B98 — blob offset 0x1bb0, 0xb0 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0075i func_80190B54; C re-targeted by symbol address
 * (docs/evidence/room_m0419i-ports-2026-09-23/REPORT.md). */

extern unsigned char *func_800C2B50();
extern int func_800C5EB0();
extern unsigned char *D_8009D254;

void func_80190B98(void *a0, unsigned char *a1, unsigned char *a2)
{
    short v[4];
    int r[2];
    unsigned char *e;
    int x;
    int t0, t1, t2;

    e = func_800C2B50();
    t0 = *(short *)(D_8009D254 + 0x2A);
    v[0] = t0;
    t1 = *(short *)(D_8009D254 + 0x2A);
    v[1] = t1;
    t2 = *(short *)(D_8009D254 + 0x2A);
    v[2] = t2;
    func_800C5EB0(a2 + 0xAC, v, r);
    x = func_800C5EB0(a2 + 0x78, v, r);
    if (r[0] == 1) {
        *(short *)(e + 0x68) = 1;
    }
    if (x == 1) {
        a1[1] = 2;
    }
}
