/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_8018F5F8 — blob offset 0x610, 0x2a8 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80190710; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern unsigned char D_801928F0[];
extern unsigned char D_80192848[];
extern unsigned char *func_800C2B50();
extern unsigned char *func_800C2B90();

#define SH(o, x) (*(short *)((char *)(o) + (x)))

void func_8018F5F8(void *a0, unsigned char *a1, unsigned char *o)
{
    unsigned char *x;
    unsigned char *p;
    unsigned int i;

    x = func_800C2B50();
    if (SH(a1, 2) == 0) {
        p = func_800C2B90(a0, 4, D_801928F0, D_80192848);
        if (p) {
            *(SVECTOR *)p = *(SVECTOR *)o;
        }
        if (SH(a1, 2) == 0) {
            p = func_800C2B90(a0, 3, D_801928F0, D_80192848);
            if (p) {
                *(SVECTOR *)p = *(SVECTOR *)o;
            }
            if (SH(a1, 2) == 0) {
                p = func_800C2B90(a0, 2, D_801928F0, D_80192848);
                if (p) {
                    SH(p, 0x14) = 1;
                    *(SVECTOR *)p = *(SVECTOR *)o;
                }
            }
        }
    }
    if (SH(a1, 2) == 4) {
        p = func_800C2B90(a0, 2, D_801928F0, D_80192848);
        if (p) {
            SH(p, 0x14) = 0;
            *(SVECTOR *)p = *(SVECTOR *)o;
        }
    }
    if (SH(a1, 2) == 6) {
        p = func_800C2B90(a0, 5, D_801928F0, D_80192848);
        if (p) {
            *(SVECTOR *)p = *(SVECTOR *)o;
            if (SH(x, 0x4E) == 0) {
                SH(p, 0xE8) = 8;
                SH(p, 0xEC) = 8;
                for (i = 0; i < SH(p, 0xE8); i++) {
                    SH(p + i * 2, 0x98) = i << 9;
                }
            } else {
                SH(p, 0xE8) = 6;
                SH(p, 0xEC) = 6;
                for (i = 0; i < SH(p, 0xE8); i++) {
                    SH(p + i * 2, 0x98) = i * 0x2A8;
                }
            }
        }
    }
    if (SH(a1, 2) > 0x3C) {
        a1[1] = 2;
    }
}
