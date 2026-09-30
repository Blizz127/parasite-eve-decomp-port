/*
 * func_8006F9F0 — record-state dispatcher (VRAM 0x8006F9F0, file 0x601F0,
 * 0x228 bytes). id >= 0x16 -> -0x13; state byte in {1,2,4,5} else 0;
 * 4 -> 5; 5 -> reset (0x72 handler also clears the D_800E10A0 voice block
 * and bit 16 of D_800B0CD8); 1 -> 2 then the +0x10 handler call, record
 * word +4 incremented. era gcc-2.7.2 -O2 -G0.
 * Both range tests jump to one join label (the body re-reads p[0] there:
 * the zero-code asm barrier on p stops cse reusing the first load), and the
 * handler call is an if/else assigning r so the -1 arm stays in front.
 */
extern unsigned char *D_800942E4; /* arena, stride 0xA0C, ids 0..0xA */
extern unsigned char *D_800942E8; /* arena, stride 0x10C, ids 0xB..0x15 */
extern void **D_800942E0;         /* handler-pointer table by id */
extern unsigned int D_800E10A0[]; /* voice table */
extern unsigned int D_800B0CD8;   /* flags word */

int func_8006F9F0(int idx)
{
    unsigned char *p;
    int h;
    int r;

    if ((unsigned int)idx >= 0x16)
        return -0x13;
    if ((unsigned int)idx >= 0xB)
        p = D_800942E8 + (idx - 0xB) * 0x10C;
    else
        p = D_800942E4 + idx * 0xA0C;

    if ((unsigned int)(p[0] - 1) < 2)
        goto body;
    if ((unsigned int)(p[0] - 4) < 2)
        goto body;
    return 0;
body:
    {
        asm("" : "=r"(p) : "0"(p));
        if (p[0] == 4) {
            p[0] = 5;
            return 0;
        }
        if (p[0] == 5) {
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
            return 0;
        }
        if (p[0] == 1)
            p[0] = 2;
        h = p[1];
        if ((unsigned int)h >= 0xC0)
            return -0x14;
        if ((unsigned int)h >= 0x55)
            h = 0x55;
        if (D_800942E0[h] == 0)
            return -0x15;
        if (*(int *)((char *)D_800942E0[h] + 0x10) == 0) {
            r = -1;
        } else {
            r = (*(int (**)(unsigned char *))((char *)D_800942E0[h] + 0x10))(p);
            *(int *)(p + 4) += 1;
        }
        return r;
    }
}
