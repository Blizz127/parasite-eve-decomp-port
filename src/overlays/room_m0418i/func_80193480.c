/* room_m0418i — func_80193480, blob offset 0x4498, 0x128 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * randomise 10 phase pairs + probability gate on func_800C2B28(2); lever: *(a2 + i + 0x24) operand order */

extern short D_80199658[];
extern short D_80199590[];
extern int func_80071A54();
extern int *func_800C2B28();

void func_80193480(void *a0, void *a1, unsigned char *a2)
{
    unsigned int i;
    int t;

    for (i = 0; i < 10; i++) {
        D_80199658[i] = func_80071A54() % 4096;
        D_80199590[i] = func_80071A54() % 4096;
        *(a2 + i + 0x24) = 0;
        ((short *)(a2 + 0xC))[i] = 0;
    }
    t = *func_800C2B28(2);
    if (func_80071A54() % 256 < t) {
        *func_800C2B28(1) = 1;
    } else {
        *func_800C2B28(1) = 0;
    }
}
