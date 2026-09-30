typedef struct { int p0[5]; int f14; int f18; int f1C; } S1;

extern void func_80078C34(S1 *a0, void *a1, unsigned short *a2);

void func_800C7AE0(unsigned short *a0, S1 *a1, int a2, unsigned short *a3)
{
    unsigned short off;

    off = (a0[0] + a0[1]) * 12 + 0x10 + (a0[2] + a0[3]) * 16;
    func_80078C34(a1, (char *)a0 + off + (a2 & 0xFFFF) * 8, a3);
    a3[0] += a1->f14;
    a3[1] += a1->f18;
    a3[2] += a1->f1C;
}
