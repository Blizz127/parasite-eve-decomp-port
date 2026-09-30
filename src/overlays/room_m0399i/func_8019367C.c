/* room_m0399i — func_8019367C, blob offset 0x4694, 0x150 bytes. Profile era_o2_g0_expand_div;
 * LINK_EXACT at the target VMA (lane ovl2 2026-09-27). */

typedef struct { int vx, vy, vz; } VEC;
extern int D_800966EC[];
extern void func_80079754();
extern void func_80078CC4();

int func_8019367C(unsigned char *a0)
{
    unsigned char *s = *(unsigned char **)(a0 + 8);
    unsigned char *m;
    int v, t;
    int *b = (int *)0x1F800000;

    (*(void (**)())(a0 + 0xC))(a0);
    m = s + 0x1E8;
    *(int *)(s + 0x1FC) = *(short *)(s + 0x2A);
    *(int *)(s + 0x200) = *(short *)(s + 0x2E);
    *(int *)(s + 0x204) = *(short *)(s + 0x32);
    func_80079754(s + 0x38, m);
    v = *(unsigned short *)(s + 0x26);
    b[2] = b[3] = b[4] = v;
    func_80078CC4(m, b + 2);
    if (*(short *)(a0 + 0x32) != 0) {
        t = ((*(short *)(a0 + 0x34) - *(short *)(a0 + 0x32)) << 11) / *(short *)(a0 + 0x34);
        t = (short)D_800966EC[t & 0xFFF] >> 1;
        b[2] = b[4] = t + 0x1000;
        b[3] = 0x1000 - t;
        func_80078CC4(m, b + 2);
        *(short *)(a0 + 0x32) -= 1;
    }
    return 0;
}
