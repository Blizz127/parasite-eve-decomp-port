/* room_m0273i — func_80194128, blob offset 0x5140, 0x15C bytes. Flags -O2 -G0 + MASPSX_NARROW_SHIFTED_WORD_LOAD (K3);
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite (func_800CEE20); loop counter reused for the table value (s0), *(int *)p >> 21 narrowed by K3. */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern char *D_8009D254;
extern short D_800966EC[];
extern short D_800966EE[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80194128(int a0, short *a1)
{
    short v[4];
    int i;
    int t;
    int k;
    int u;
    register int four asm("$3");

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
    } else if (a0 == 2) {
        for (i = 0; i < 3; i++) {
            v[i] = (*(int **)(D_8009D254 + 0x238))[i + 5];
        }
        four = 4;
        t = D_800E27EC - 1;
        k = D_800F336C;
        i = *(short *)((char *)D_800966EC + ((t << 8) & 0x3F00));
        u = D_800E1204[k];
        if (k == four && D_800F3428 != 0) {
            u += 0xB;
        } else {
            u += 7;
        }
        func_800CEE20(v, a1, 0x2000, i * 3, 5, func_80077AA4(0, u), 1,
                      *(int *)((char *)D_800966EC + ((t << 8) & 0x3F00)) >> 21, 0);
    }
    return 0;
}
