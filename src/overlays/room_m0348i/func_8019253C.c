/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_8019253C — blob offset 0x3554, 0x30c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80193654; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern SVECTOR D_8018F04C;
extern SVECTOR D_8018F054;
extern SVECTOR D_8018F05C;
extern unsigned char **func_800C2B50();
extern void func_80078C34();
extern int func_80071A54();

#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

void func_8019253C(int a0, unsigned char *a1, unsigned char *o)
{
    unsigned char **x;
    unsigned char *mat;
    SVECTOR a, b, c;
    SVECTOR oa, ob, oc;
    unsigned int i;
    int t;

    x = func_800C2B50();
    mat = (unsigned char *)P(*x, 0x238);
    a = D_8018F04C;
    b = D_8018F054;
    c = D_8018F05C;
    for (i = 0; i < 8; i++) {
        if ((o + i)[0xB4] == 0) {
            if ((short)(SH(a1, 2) % 8) == i && SH(a1, 2) < 0x5A) {
                (o + i)[0xB4] = 1;
                W(o, 0)++;
                func_80078C34(mat + 0xA0, &a, &oa);
                func_80078C34(mat + 0xA0, &b, &ob);
                func_80078C34(mat + 0xA0, &c, &oc);
                SH(o + i * 8, 4) = oa.vx + W(x, 0x38);
                SH(o + i * 8, 6) = oa.vy + W(x, 0x3C);
                SH(o + i * 8, 8) = oa.vz + W(x, 0x40);
                if (!(func_80071A54() & 1)) {
                    *(SVECTOR *)(o + i * 8 + 0x44) = oc;
                } else {
                    *(SVECTOR *)(o + i * 8 + 0x44) = ob;
                }
                SH(o + i * 2, 0x84) = 0x800;
                SH(o + i * 2, 0x94) = 0x80;
                SH(o + i * 2, 0xA4) = 0;
            }
        } else {
            SH(o + i * 8, 4) += SH(o + i * 8, 0x44);
            t = func_80071A54();
            SH(o + i * 8, 8) += SH(o + i * 8, 0x48);
            SH(o + i * 8, 6) -= 0x14;
            SH(o + i * 8, 6) -= t % 20;
            SH(o + i * 2, 0x94) -= 8;
            SH(o + i * 2, 0xA4) += 1;
            if (SH(o + i * 2, 0x94) <= 0) {
                SH(o + i * 2, 0x94) = 0;
                (o + i)[0xB4] = 0;
                W(o, 0)--;
            }
        }
    }
    if (W(o, 0) == 0 && SH(a1, 2) > 0x5A) {
        a1[1] = 2;
    }
}
