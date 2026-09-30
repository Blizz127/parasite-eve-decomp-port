/* room_m0231i (PE.IMG room m0231i chunk 2, VRAM 0x8018EFE8)
 * func_80192290 — blob offset 0x32a8, 0x114 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_80192230; C re-targeted by symbol address
 * (docs/evidence/room_m0231i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void *D_8009D254;
extern int D_800966EC[];
extern short func_800DFF80();
extern void func_801923A4();
void func_80192290(void *o)
{
    int *t;
    int c;

    if (B(D_8009D254, 0xE) < 4) {
        W(D_8009D254, 0x98) &= 0xFFF3FFFF;
        P(o, 0xC) = func_801923A4;
        if (P(o, 0x10) != 0) {
            *(int *)P(o, 0x10) = 2;
        }
        if (H(o, 0x44) == -1) {
            H(o, 0x44) = func_800DFF80((char *)P(o, 0x8) + 0x28, (char *)D_8009D254 + 0x28);
        }
        if (H(o, 0x48) == -1) {
            H(o, 0x48) = H(o, 0x44) + 0x800;
        }
        t = &D_800966EC[H(o, 0x44) & 0xFFF];
        c = ((short *)t)[1];
        H(o, 0x1C) = c;
        H(o, 0x1E) = 0;
        H(o, 0x20) = *t;
        H(o, 0x22) = 0;
        H(o, 0x24) = 0x1000;
        H(o, 0x26) = 0;
        H(o, 0x28) = -H(o, 0x20);
        H(o, 0x2A) = 0;
        H(o, 0x2C) = H(o, 0x1C);
    }
}
