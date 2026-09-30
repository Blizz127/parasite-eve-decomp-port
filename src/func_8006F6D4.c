/*
 * func_8006F6D4 — record query + offset-8 handler invoke (VRAM 0x8006F6D4,
 * file 0x5FED4, 83 words / 0x14C).
 *
 * Resolves the command record for `idx` from the D_800942E4 (ids 0..0xA,
 * stride 0xA0C) / D_800942E8 (ids 0xB..0x15, stride 0x10C) arenas. Failures:
 * -0xA out-of-range id, -0xB handler byte >= 0xC0, -0xC null handler, -1 when
 * the handler's offset-8 slot is null. Otherwise, when a1==1 and a2==0, the
 * record fields at +2, +3 and +4 are written to *a3, *a4, *a5; then the
 * handler's offset-8 function pointer is called with the record and the four
 * args, and its result is returned.
 *
 * The offset-8 pointer expression is written twice (guard + call) so cc1
 * re-loads it after the *a3/*a4/*a5 stores, matching retail's duplicate
 * D_800942E0 load chain. `h` is an int (no re-masking on the 0x55 clamp).
 *
 * ROM: era gcc-2.7.2-psx -O2 -G0.
 */
extern unsigned char *D_800942E4;
extern unsigned char *D_800942E8;
extern void **D_800942E0;

typedef int (*F6D4Fn)(unsigned char *, int, int, int *, int *, int *);

int func_8006F6D4(int idx, int a1, int a2, int *a3, int *a4, int *a5)
{
    unsigned char *p;
    int h;

    if ((unsigned int)idx >= 0x16)
        return -0xA;
    if ((unsigned int)idx >= 0xB)
        p = D_800942E8 + (idx - 0xB) * 0x10C;
    else
        p = D_800942E4 + idx * 0xA0C;

    h = p[1];
    if ((unsigned int)h >= 0xC0)
        return -0xB;
    if ((unsigned int)h >= 0x55)
        h = 0x55;
    if (D_800942E0[h] == 0)
        return -0xC;

    if (*(F6D4Fn *)((unsigned char *)D_800942E0[h] + 8) == 0)
        return -1;

    if (a1 == 1 && a2 == 0) {
        *a3 = p[2];
        *a4 = p[3];
        *a5 = *(int *)(p + 4);
    }
    return (*(F6D4Fn *)((unsigned char *)D_800942E0[h] + 8))(p, a1, a2, a3, a4, a5);
}
