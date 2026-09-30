/* room_m0350i — func_80194CFC, blob offset 0x5D14, 0x208 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Particle spawner controller; D_8019A7A0 state block as one struct (seed vector + counter + flag) keeps the counter RMW after the particle stores. */

typedef struct { short x, y, z, w; } SV;
typedef struct { SV p; short h8, hA; int wC; short h10, h12, h14, h16; unsigned char b18; } PT;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern struct { SV v; char pad[0x5A]; short n; unsigned char f; } D_8019A7A0;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern void func_801947BC();
extern int func_800CE560();
extern PT *func_800CE610();

int func_80194CFC(int a0)
{
    PT *e;
    unsigned char *p;
    int a;
    int b;

    switch (a0) {
    case 0:
        return func_800CE560(((void **)D_800F33E0)[2], 0x1C, 3, func_801947BC);
    case 1:
        if (D_8019A7A0.f != 0) {
            return 2;
        }
        p = ((unsigned char **)D_800F32D0)[2];
        if (p[0xE] != 0xD) {
            return 0;
        }
        a = *(unsigned short *)(p + 0x16);
        b = *(unsigned short *)(p + 0x1A);
        if (a < 0xF) {
            return 0;
        }
        if (b >= 0xF) {
            return 0;
        }
        e = func_800CE610(((void **)D_800F33E0)[2]);
        e->p = D_8019A7A0.v;
        e->p.w = 0x10;
        e->h8 = 0x200;
        e->hA = *(unsigned short *)(((unsigned char **)D_800F32D0)[2] + 0x3A);
        e->wC = 0;
        e->h10 = 100 - D_8019A7A0.n * 20;
        e->h12 = 0x10;
        e->h16 = 0;
        e->b18 = 0;
        D_8019A7A0.n++;
        break;
    case 2:
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x40;
        break;
    }
    return 0;
}
