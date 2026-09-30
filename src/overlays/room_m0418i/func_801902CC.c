/* room_m0418i — func_801902CC, blob offset 0x12E4, 0x194 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * 4-particle spawn around the owner position (rand()%200 jitter, rand()%5, rand()%256); levers: t = load; t -= 100; t += m keeps retail association; m pinned $2. */

extern char *func_800C2B50();
extern int func_80071A54();

#define B(off) (a2 + i)[off]
#define H(off) *(short *)(a2 + i * 2 + (off))
#define V(off) *(short *)(a2 + i * 8 + (off))

void func_801902CC(int a0, int a1, char *a2)
{
    char *r;
    unsigned int i;
    register int m asm("$2");
    int t;

    r = func_800C2B50() + 4;
    a2[0x44] = 4;
    for (i = 0; i < 4; i++) {
        B(0x3C) = 0;
        m = func_80071A54() % 200;
        t = *(int *)(r + 0x14);
        t -= 100;
        t += m;
        V(0) = t;
        V(2) = *(int *)(r + 0x18);
        m = func_80071A54() % 200;
        t = *(int *)(r + 0x1C);
        t -= 100;
        t += m;
        V(4) = t;
        B(0x40) = func_80071A54() % 5 + 1;
        H(0x20) = func_80071A54() % 256 + 0x400;
        B(0x30) = 0x48;
        B(0x34) = 4;
        H(0x28) = 0x40;
        B(0x38) = 0;
    }
}
