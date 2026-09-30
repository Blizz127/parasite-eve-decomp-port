/*
 * func_8006F8EC — invoke a command record's offset-0xC handler (VRAM
 * 0x8006F8EC, file 0x600EC, 65 words / 0x104).
 *
 * Resolves the record for `idx` in the D_800942E4 (ids 0..0xA, stride 0xA0C)
 * / D_800942E8 (ids 0xB..0x15, stride 0x10C) arenas. Only records whose
 * state byte (offset 0) is 1 or 2 are dispatchable; other ids return -0x10
 * (out of range), -0x11 (handler byte >= 0xC0) or -0x12 (null handler).
 * The handler byte (offset 1, clamped to 0x55) indexes the D_800942E0
 * pointer table, whose offset-0xC function pointer is called with the
 * record as its argument; a null pointer yields -1.
 *
 * ROM: era gcc-2.7.2-psx -O2 -G0. `h` is an int (retail compares the lbu
 * result directly, with no re-masking on the 0x55 clamp); the final test is
 * written `fn != 0` so cc1 lays the call out as the branch target.
 */
extern unsigned char *D_800942E4;
extern unsigned char *D_800942E8;
extern void **D_800942E0;

int func_8006F8EC(int idx)
{
    unsigned char *p;
    int h;
    int (*fn)(unsigned char *);

    if ((unsigned int)idx >= 0x16)
        return -0x10;

    if ((unsigned int)idx >= 0xB)
        p = D_800942E8 + (idx - 0xB) * 0x10C;
    else
        p = D_800942E4 + idx * 0xA0C;

    if ((unsigned int)(p[0] - 1) >= 2)
        return 0;

    h = p[1];
    if ((unsigned int)h >= 0xC0)
        return -0x11;
    if ((unsigned int)h >= 0x55)
        h = 0x55;
    if (D_800942E0[h] == 0)
        return -0x12;

    fn = *(int (**)(unsigned char *))((unsigned char *)D_800942E0[h] + 0xC);
    if (fn != 0)
        return fn(p);
    return -1;
}
