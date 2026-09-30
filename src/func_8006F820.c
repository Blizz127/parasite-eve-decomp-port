/*
 * func_8006F820 — record state get/set by id (VRAM 0x8006F820, file 0x5FF20,
 * 51 words / 0xCC).
 *
 * Same two-arena id map as func_8006F2C4: ids 0x00..0x0A in D_800942E4
 * (stride 0xA0C), ids 0x0B..0x15 in D_800942E8 (stride 0x10C). The handler
 * byte at record+1 indexes the D_800942E0 pointer table; a null handler
 * fails with -0xF, an out-of-range id with -0xD. mode==0 writes the low
 * byte (only if arg < 6), otherwise arg is an out-pointer that receives the
 * byte. Always returns the current byte.
 *
 * `arg` is intentionally typed `int` and cast for the store because the
 * retail code uses $a2 as an integer in the set path and as an address in
 * the get path; `mode` selects which meaning applies.
 *
 * ROM: era gcc-2.7.2-psx -O2 -G0. The source keeps the `mode == 0` test
 * with the get-arm as the `else` so cc1 lays the store block out as the
 * branch target (bnez $a1), matching retail.
 */
extern unsigned char *D_800942E4;
extern unsigned char *D_800942E8;
extern void **D_800942E0;

int func_8006F820(int idx, int mode, int arg)
{
    unsigned char *p;
    int h;

    if ((unsigned int)idx >= 0x16)
        return -0xD;

    if ((unsigned int)idx >= 0xB)
        p = D_800942E8 + (idx - 0xB) * 0x10C;
    else
        p = D_800942E4 + idx * 0xA0C;

    h = p[1];
    if ((unsigned int)h >= 0x55)
        h = 0x55;
    if (D_800942E0[h] == 0)
        return -0xF;

    if (mode == 0) {
        if ((unsigned int)arg < 6)
            p[0] = arg;
    } else {
        *(int *)arg = p[0];
    }
    return p[0];
}
