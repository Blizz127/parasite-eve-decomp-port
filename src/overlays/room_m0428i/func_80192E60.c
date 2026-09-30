/* room_m0428i — func_80192E60, blob offset 0x3E78, 0x1B8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * Two-pass func_800D0728 emitter; SV source declared before v[4]; v stores in natural order; min(a1[6], 0x80) ternary per call. */

typedef struct { short x, y, z, w; } SV;
extern int D_800E27EC;
extern unsigned char *D_800F32D0;
extern short D_801941CC;
extern void func_800D0728();

int func_80192E60(int a0, short *a1)
{
    SV s;
    short v[4];
    int *o;

    if (a0 == 1) {
        a1[4] += a1[5];
        a1[5] += 0x30;
        a1[6] -= 0x20;
        if (a1[6] < 0x20) {
            D_801941CC--;
            return 1;
        }
    } else if (a0 == 2) {
        o = *(int **)(D_800F32D0 + 8);
        s.x = o[0x1FC / 4];
        s.y = o[0x200 / 4];
        s.z = o[0x204 / 4];
        v[0] = 0x400;
        v[1] = D_800E27EC << 7;
        v[2] = 0;
        v[3] = 1;
        func_800D0728(&s, 0x300, 0x400, 0x10, v, a1[4], a1[4], a1, &a1[2], a1[6] > 0x80 ? 0x80 : a1[6], 1);
        func_800D0728(&s, 0x400, 0x500, 0x10, v, a1[4], a1[4], &a1[2], a1, a1[6] > 0x80 ? 0x80 : a1[6], 1);
    }
    return 0;
}
