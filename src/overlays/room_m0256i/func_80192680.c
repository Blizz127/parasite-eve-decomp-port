/* room_m0256i — func_80192680, blob offset 0x3698, 0x188 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Door/lift easing step: cosine-eased offset over H34 frames (signed div via MASPSX_EXPAND_DIV), phase advance calls func_80192808; explicit v<0 rounding + e local give retail in-place /8192. */

extern short D_800966EE[];
extern void func_80192808();

#define H(off) *(short *)(a0 + (off))
#define UH(off) *(unsigned short *)(a0 + (off))

void func_80192680(unsigned char *a0)
{
    unsigned char *p;
    unsigned char *b;
    int n;
    int v;
    int d;
    int q;
    int e;

    p = *(unsigned char **)(a0 + 8);
    b = a0 + 0xC;
    if (a0[0x1A] == 0) {
        a0[0x1A] = 1;
        H(0x32) = 0;
        H(0x2E) = *(unsigned short *)(b + H(0x1C) * 2 + 0x14);
        H(0x30) = *(unsigned short *)(b + (H(0x1C) << 1) + 0x16) - H(0x2E);
        H(0x34) = *(unsigned short *)(b + H(0x1C) * 2 + 0x1C);
    }
    n = UH(0x32) + 1;
    d = ((short)n << 11) / H(0x34);
    H(0x32) = n;
    v = (-D_800966EE[(d & 0xFFF) * 2] + 0x1000) * H(0x30);
    if (v < 0) {
        v += 0x1FFF;
    }
    e = UH(0x2E);
    p[0x25A] = 3;
    p[0x25B] = 2;
    *(short *)(p + 0x256) = e + (v >> 13);
    if (H(0x32) >= H(0x34)) {
        if (++H(0x1C) >= 3) {
            func_80192808(a0);
        } else {
            a0[0x1A] = 0;
        }
    }
    if (*(unsigned short *)(p + 0x16) >= p[0xF]) {
        *(int *)(p + 0x14) = 0;
    }
}
