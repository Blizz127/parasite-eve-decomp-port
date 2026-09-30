/* room_m0174i — func_80190710, blob offset 0x1728, 0x2A8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * Timed spawner via func_800C2B90 (nested ==0 checks, ==4, ==6 with ramp loops); first draft exact. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern unsigned char D_80196E14[];
extern unsigned char D_80196D6C[];
extern unsigned char *func_800C2B50();
extern unsigned char *func_800C2B90();

#define SH(o, x) (*(short *)((char *)(o) + (x)))

void func_80190710(void *a0, unsigned char *a1, unsigned char *o)
{
    unsigned char *x;
    unsigned char *p;
    unsigned int i;

    x = func_800C2B50();
    if (SH(a1, 2) == 0) {
        p = func_800C2B90(a0, 4, D_80196E14, D_80196D6C);
        if (p) {
            *(SVECTOR *)p = *(SVECTOR *)o;
        }
        if (SH(a1, 2) == 0) {
            p = func_800C2B90(a0, 3, D_80196E14, D_80196D6C);
            if (p) {
                *(SVECTOR *)p = *(SVECTOR *)o;
            }
            if (SH(a1, 2) == 0) {
                p = func_800C2B90(a0, 2, D_80196E14, D_80196D6C);
                if (p) {
                    SH(p, 0x14) = 1;
                    *(SVECTOR *)p = *(SVECTOR *)o;
                }
            }
        }
    }
    if (SH(a1, 2) == 4) {
        p = func_800C2B90(a0, 2, D_80196E14, D_80196D6C);
        if (p) {
            SH(p, 0x14) = 0;
            *(SVECTOR *)p = *(SVECTOR *)o;
        }
    }
    if (SH(a1, 2) == 6) {
        p = func_800C2B90(a0, 5, D_80196E14, D_80196D6C);
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
