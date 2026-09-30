/* room_m0350i — func_80196C64, blob offset 0x7C7C, 0x1A0 bytes. Flags -O2 -G0 + MASPSX_NARROW_SHIFTED_WORD_LOAD (K3);
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4-less sprite + D_800F3368 block: p = &D_800F3368 through an asm output barrier (s1 base), asm volatile after the first store, pad[2], *(int *)p >> 21 narrowed by K3. */

typedef struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } ST;
extern ST D_800F3368;
extern int D_800E27EC;
extern unsigned short D_800E11E8;
extern unsigned short D_800E1208;
extern unsigned short D_800E2850[];
extern short D_800966EC[];
extern char D_8019A59C[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80196C64(int a0, short *a1)
{
    ST *p;
    int s;
    int t;
    unsigned short us;
    int pad[2];

    if (a0 == 1) {
        if (D_800E27EC >= 8) {
            return 1;
        }
    } else if (a0 == 2) {
        p = &D_800F3368;
        asm("" : "=r"(p) : "0"(p));
        p->a68 = 0x20;
        asm volatile("");
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11E8];
        D_800F3368.a6C = 2;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x20;
        s = *(short *)((char *)D_800966EC + (((D_800E27EC - 1) << 9) & 0x3E00)) * 2 + 0x1000;
        us = func_80077AA4(0, D_800E1208);
        t = D_800E27EC - 1;
        func_800CEE20(a1, 0, (short)s, (short)s, p->a6A * ((t >> 2) & 3) + 0xC0, us, 1,
                      *(int *)((char *)D_800966EC + ((t << 9) & 0x3E00)) >> 21, D_8019A59C);
    }
    return 0;
}
