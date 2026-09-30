/* room_m0392i (PE.IMG room m0392i chunk 2, VRAM 0x8018EFE8)
 * func_8018F9E0 — blob offset 0x9f8, 0x1dc bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018F838; C re-targeted by symbol address
 * (docs/evidence/room_m0392i-ports-2026-09-23/REPORT.md). */

typedef struct { short x, y, z, pad; } SV;
extern short D_800942EC;
extern char D_801944CC[];
extern char D_80194484[];
extern short *func_800C2B90();

void func_8018F9E0(void *a0, unsigned char *a1, short *a2)
{
    short t;
    short *p;
    short *q;
    unsigned int i;
    short *lim;
    SV *v;
    short *s = a2;

    if (a2[1] >= D_800942EC) {
        t = a2[8];
        if (t > 0x14) {
            a2[8] = t - 0x14;
        } else {
            a1[1] = 2;
        }
        if (((unsigned char *)s)[0x16] == 0) {
            ((unsigned char *)s)[0x16] = 1;
            p = func_800C2B90(a0, 3, D_801944CC, D_80194484);
            if (p != 0) {
                *p = s[0];
                q = p + 1;
                *q = D_800942EC;
                q = p + 2;
                *q = s[2];
            }
            p = func_800C2B90(a0, 2, D_801944CC, D_80194484);
            if (p != 0) {
                *p = s[0];
                q = p + 1;
                *q = D_800942EC;
                q = p + 2;
                *q = s[2];
            }
            v = (SV *)func_800C2B90(a0, 4, D_801944CC, D_80194484);
            if (v != 0) {
                lim = &D_800942EC;
                for (i = 0; i < 6; i++) {
                    v[i + 4].x = s[0];
                    v[i + 4].y = *lim;
                    v[i + 4].z = s[2];
                }
            }
        }
    } else {
        s[0] += s[4];
        s[2] += s[6];
        s[1] += s[10] >> 8;
        s[8] += 0x28;
        s[10] += 0x64;
    }
}
