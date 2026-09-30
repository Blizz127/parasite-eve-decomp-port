extern unsigned char *D_800942E4; /* arena, stride 0xA0C, ids 0..0xA */
extern unsigned char *D_800942E8; /* arena, stride 0x10C, ids 0xB..0x15 */
extern void **D_800942E0;         /* handler-pointer table by id */
extern unsigned int D_800E10A0[]; /* overlay handler-pointer table */
extern unsigned int D_800B0CD8;   /* flags word */

int func_8006FC18(int idx, int a1, int a2) {
    unsigned char *p;
    int h;

    if ((unsigned int)idx >= 0x16)
        return -0x16;
    if ((unsigned int)idx >= 0xB)
        p = D_800942E8 + (idx - 0xB) * 0x10C;
    else
        p = D_800942E4 + idx * 0xA0C;

    if (p[0] == 0 || p[0] == 6)
        return 0;
    if (a2 == 0 && *(int *)(p + 8) != a1)
        return 0;

    h = p[1];
    if ((unsigned int)h >= 0xC0)
        return -0x17;
    if ((unsigned int)h >= 0x55)
        h = 0x55;
    if (D_800942E0[h] == 0)
        return -0x18;
    if (*(int *)((char *)D_800942E0[h] + 0x14) == 0)
        return -1;

    a2 = (*(int (**)(unsigned char *, int, int))((char *)D_800942E0[h] + 0x14))(
        p, h, a2);

    if ((unsigned int)idx < 0x16) {
        unsigned char *q;
        if ((unsigned int)idx >= 0xB)
            q = D_800942E8 + (idx - 0xB) * 0x10C;
        else
            q = D_800942E4 + idx * 0xA0C;
        if (q[1] == 0x72) {
            unsigned int i;
            for (i = 0x6C; i < 0x73; i++)
                D_800E10A0[i - 0x6C] = 0;
            D_800B0CD8 &= 0xFFFEFFFF;
        }
        q[0] = 0;
        q[1] = 0xFF;
        q[2] = 0xFF;
        q[3] = 0xFF;
        *(int *)(q + 4) = 0;
        *(int *)(q + 8) = 0;
    }
    return a2;
}
