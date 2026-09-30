/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_80191E1C — blob offset 0x2e34, 0x1c4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80192F34; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

typedef struct { int w[8]; } S32;
extern unsigned char *func_800C2B50();
extern int func_80071A54();

#define UB(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

void func_80191E1C(int a0, int a1, unsigned char *o)
{
    unsigned char *x;
    unsigned int i;
    unsigned int t;

    x = func_800C2B50();
    *(S32 *)o = *(S32 *)(x + 4);
    for (i = 0; i < 4; i++) {
        t = func_80071A54() % 40 + 10;
        func_80071A54();
        SH(o + i * 8, 0x32) = -200;
        SH(o + i * 8, 0x34) = -200;
        SH(o + i * 8, 0x50) = -(t >> 1);
        SH(o + i * 8, 0x52) = -(t >> 1);
        SH(o + i * 8, 0x30) = 0;
        SH(o + i * 2, 0x70) = t << 6;
        SH(o + i * 2, 0x78) = 0xA0;
        switch (i) {
        case 0:
            UB(o + i * 4, 0x20) = 0x9F;
            UB(o + i * 4, 0x21) = 0x8A;
            UB(o + i * 4, 0x22) = 0x89;
            break;
        case 1:
            UB(o + i * 4, 0x21) = 0x4C;
            UB(o + i * 4, 0x20) = 0x40;
            UB(o + i * 4, 0x22) = 6;
            break;
        case 2:
            UB(o + i * 4, 0x20) = 0x12;
            UB(o + i * 4, 0x21) = 0x20;
            UB(o + i * 4, 0x22) = 0x40;
            break;
        case 3:
            UB(o + i * 4, 0x20) = 0x9F;
            UB(o + i * 4, 0x21) = 0x62;
            UB(o + i * 4, 0x22) = 0x89;
            break;
        }
    }
}
