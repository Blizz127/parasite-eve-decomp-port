/* room_m0350i — func_80199E50, blob offset 0xAE68, 0xDC bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * event state handler (0: spawn via func_800CE560, 1: poll + set offset); lever: if-tree with case-1 falling into shared return 0 */

extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern unsigned char D_8019A86E;
extern unsigned char D_8019A854;
extern int func_800CE560();
extern int *func_800CE610();
extern void func_8019A134();

int func_80199E50(int a0)
{
    int *p;

    if (a0 != 1) {
        if (a0 < 2) {
            if (a0 == 0) {
                return func_800CE560(*(void **)(D_800F33E0 + 8), 4, 2, func_8019A134);
            }
            return 0;
        }
    } else {
        if (D_8019A86E != 0) {
            return 2;
        }
        if (D_8019A854 == 0) {
            return 0;
        }
        p = func_800CE610(*(void **)(D_800F33E0 + 8));
        if (p == 0) {
            return 0;
        }
        *p = *(int *)(*(unsigned char **)(D_800F32D0 + 8) + 0x238) + 0xF4;
        D_8019A854 = 0;
    }
    return 0;
}
