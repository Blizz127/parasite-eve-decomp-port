/* room_m0350i — func_8019569C, blob offset 0x66B4, 0x240 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Bone-anchored emitter controller: [k][i] byte-pair table, 3-word copy loop e[j] = v[j], sin-table word index ((k-7)<<7)&0xF80 (hoisted whole address). */

extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern unsigned char *D_8019A7F8;
extern unsigned char D_8019A804;
extern unsigned char D_8019A4D4[][2][2];
extern int D_800966EC[];
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern void func_80195564();
extern int func_800CE560();
extern short *func_800CE610();

int func_8019569C(int a0)
{
    short *e;
    short *d;
    unsigned char *s;
    unsigned char *src;
    int *v;
    int i;
    int j;
    int k;

    switch (a0) {
    case 0:
        return func_800CE560(((void **)D_800F33E0)[2], 8, 0xA, func_80195564);
    case 1:
        if (D_8019A804 != 0) {
            return 2;
        }
        k = *(unsigned short *)(((unsigned char **)D_800F32D0)[2] + 0x16);
        if ((unsigned int)(k - 7) < 6) {
            for (i = 0; i < 2; i++) {
                s = D_8019A4D4[k][i];
                if (s[0] == 0) {
                    return 0;
                }
                e = func_800CE610(((void **)D_800F33E0)[2]);
                if (e == 0) {
                    return 0;
                }
                if (s[1] != 0) {
                    src = D_8019A7F8;
                } else {
                    src = ((unsigned char **)D_800F32D0)[2];
                }
                v = (int *)(*(char **)(src + 0x238) + s[0] * 32 + 0x14);
                for (j = 0; j < 3; j++) {
                    e[j] = v[j];
                }
                e[3] = D_800966EC[((k - 7) << 7) & 0xF80] + 0x800;
            }
        }
        break;
    case 2:
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11FA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0;
        break;
    }
    return 0;
}
