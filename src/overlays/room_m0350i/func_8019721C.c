/* room_m0350i — func_8019721C, blob offset 0x8234, 0x148 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Proximity trigger (abs dx/dz box + sqrt radius vs a1) latching D_8019A86C; shared fail label; struct view for the 0x4C flag RMW lets the D_800F32D0 load hoist. */

extern short D_8019A86C;
extern short D_800942EC;
extern char *D_8009D254;
extern char *D_800F32D0;
extern int func_8005186C();

typedef struct { unsigned int w[0x20]; } WS;

int func_8019721C(char *a0, int a1)
{
    short *t = &D_8019A86C;
    char *p;
    int dx, dz;
    unsigned int *q;

    if (*t != 0) {
        goto fail;
    }
    {
        if (*(short *)(a0 + 0xA) < D_800942EC - 0x200) {
            return 0;
        }
        p = D_8009D254;
        dx = *(int *)(p + 0x1FC) - *(short *)(a0 + 8);
        if (dx < 0) {
            dx = *(short *)(a0 + 8) - *(int *)(p + 0x1FC);
        }
        if (a1 < dx) {
            return 0;
        }
        dz = *(int *)(p + 0x204) - *(short *)(a0 + 0xC);
        if (dz < 0) {
            dz = *(short *)(a0 + 0xC) - *(int *)(p + 0x204);
        }
        if (a1 < dz) {
            return 0;
        }
        if (a1 < func_8005186C(dx * dx + dz * dz)) {
fail:
            return 0;
        }
        {
            ((WS *)*(char **)D_8009D254)->w[0x4C / 4] |= 0x4000;
            q = **(unsigned int ***)(D_800F32D0 + 8);
            if (q != 0) {
                *q |= 0x80000000;
            }
            *t = 0x1E;
            return 1;
        }
    }
    return 0;
}
