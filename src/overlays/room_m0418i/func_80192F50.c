/* room_m0418i — func_80192F50, blob offset 0x3F68, 0x1C4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * 32-slot spark spawner/updater (rand()%30 spawn gate); counter biv eliminated by loop opt; B()/H() macros; rand result local before the 0x100 store. */

extern int func_80071A54();

#define B(off) (a2 + i)[off]
#define H(off) *(short *)(a2 + i * 2 + (off))

void func_80192F50(int a0, unsigned char *a1, unsigned char *a2)
{
    unsigned int i;

    for (i = 0; i < 32; i++) {
        if (B(0) == 0) {
            if (func_80071A54() % 30 == 0 && *(short *)(a1 + 2) < 0x32) {
                B(0) = 1;
                B(0x20) = 0;
                H(0x40) = func_80071A54() % 4096;
                H(0x80) = 0x7E8;
                H(0xC0) = 0x7E8;
                { int t = func_80071A54(); B(0x100) = 0; B(0x120) = t % 20 + 0x28; }
            }
        } else {
            B(0x20)++;
            H(0x40) += B(0x120);
            H(0xC0) -= B(0x100);
            B(0x100) += 2;
            H(0x80) -= 0x28;
            if (H(0x80) < 0) {
                H(0x80) = 0;
            }
            if (H(0xC0) < 0) {
                B(0) = 0;
            }
        }
    }
    if (*(short *)(a1 + 2) >= 0x79) {
        a1[1] = 2;
    }
}
