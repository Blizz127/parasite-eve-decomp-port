/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_8018FE9C — blob offset 0xeb4, 0x180 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0034i func_8018FC54; C re-targeted by symbol address
 * (docs/evidence/room_m0174i-ports-2026-09-23/REPORT.md). */

extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern short D_800942EC;
extern unsigned char D_80196C7C[];
extern unsigned char D_80196CB8[];
extern int func_8001CAB0();
extern unsigned char *func_800C2B90();

#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define UB(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

void func_8018FE9C(void *a0, unsigned char *a1, unsigned char *o)
{
    unsigned char *p;

    SH(o, 8) += SH(o, 0x18);
    SH(o, 0xA) += SH(o, 0x1A);
    SH(o, 0xC) += SH(o, 0x1C);
    SH(o, 4) += 0x1E;
    if (SH(o, 6) < 0x81) {
        SH(o, 6) += 0xA;
    }
    SB(o, 2)++;
    if (func_8001CAB0(SH(o, 8) << 16, SH(o, 0xC) << 16, D_8009D248, D_8009D1CC)) {
        if ((signed char)(SB(o, 2) % 3) == 0) {
            p = func_800C2B90(a0, 3, D_80196CB8, D_80196C7C);
            if (p) {
                SH(p, 8) = SH(o, 8);
                SH(p, 0xA) = D_800942EC;
                SH(p, 0xC) = SH(o, 0xC);
            }
        }
    }
    if (func_8001CAB0(SH(o, 8) << 16, SH(o, 0xC) << 16, D_8009D248, D_8009D1CC) == 0) {
        a1[1] = 2;
        UB(o, 0) = 0;
    }
}
