extern unsigned char *D_800942E4; /* arena, stride 0xA0C, ids 0..0xA */
extern unsigned char *D_800942E8; /* arena, stride 0x10C, ids 0xB..0x15 */
extern unsigned int D_800E0EF0[]; /* overlay slot array; D_800E10A0 = &[0x6C] */
extern unsigned int D_800B0CD8;   /* flags word */

extern int func_8006FC18(int idx, int a1, int a2);

int func_800702DC(void) {
    int r;
    int i;
    unsigned char *q;

    r = 0;
    for (i = 0; i < 0xB; i++) {
        r = func_8006FC18(i, 0, 1);
        if (r != 0)
            return r;
        if ((unsigned int)i < 0x16) {
            if ((unsigned int)i >= 0xB)
                q = D_800942E8 + (i - 0xB) * 0x10C;
            else
                q = D_800942E4 + i * 0xA0C;
            if (q[1] == 0x72) {
                unsigned int k;
                for (k = 0x6C; k < 0x73; k++)
                    D_800E0EF0[k] = 0;
                D_800B0CD8 &= 0xFFFEFFFF;
            }
            q[0] = 0;
            q[1] = 0xFF;
            q[2] = 0xFF;
            q[3] = 0xFF;
            *(int *)(q + 4) = 0;
            *(int *)(q + 8) = 0;
        }
    }
    return r;
}
