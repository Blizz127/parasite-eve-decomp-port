/* room_m0318i — func_801992A4, blob offset 0xA2BC, 0xD8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * event state query twin of m0432i func_80198F54 (threshold *a2) */

typedef struct { unsigned char pad[0x14]; short a, b; } S2368;
extern S2368 *D_800E2368;
extern unsigned char **D_8009D254;
extern int D_800E27EC;

int func_801992A4(int a0, int a1, int *a2)
{
    if (a0 != 1) {
        if (a0 < 2) {
            if (a0 == 0) {
                D_800E2368->a = 0;
                D_800E2368->b = 0;
                *(int *)(*D_8009D254 + 0x4C) &= ~0x10000000;
                return 0;
            }
            return 0;
        }
        return 0;
    }
    if (*(int *)(*D_8009D254 + 0x4C) & 0x10000000) {
        D_800E2368->a = 1;
        D_800E2368->b = 1;
        return 1;
    }
    if (D_800E27EC < *a2) {
        return 0;
    }
    D_800E2368->a = 1;
    D_800E2368->b = 0;
    return 1;
}
