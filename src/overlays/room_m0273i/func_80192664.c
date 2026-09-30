/* room_m0273i — func_80192664, blob offset 0x367C, 0x1D4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * Room trigger check (player state, two func_800DFF80 angles, func_800DFE20 distance, player hand-off + 32-byte S32 copy); first try. */

typedef struct { int w[8]; } S32;
extern unsigned char *D_8009D254;
extern void func_80192838();
extern void func_80192D50();
extern int func_800DFF80();
extern int func_800DFE20();
extern void func_80020C74();

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(unsigned char **)((char *)(o) + (x)))

void func_80192664(unsigned char *a0)
{
    unsigned char *o;
    unsigned char *s;
    unsigned char *q;
    int h;
    int a;

    o = P(a0, 8);
    if (B(D_8009D254, 0xE) < 0x12) {
        h = H(o, 0x16);
        if (h < 0xE) {
            return;
        }
        if (h < 0x1E) {
            s = o + 0x1FC;
            *(int *)0x1F800008 = W(P(o, 0x238), 0x594) << 16;
            *(int *)0x1F800010 = W(P(o, 0x238), 0x59C) << 16;
            a = func_800DFF80(s, P(o, 0x238) + 0x594);
            if (((func_800DFF80(s, D_8009D254 + 0x1FC) - a + 0x40) & 0xFFF) >= 0x100) {
                return;
            }
            if (func_800DFE20(D_8009D254 + 0x28, 0x1F800008) >= 0x180) {
                return;
            }
            P(a0, 0xC) = (unsigned char *)func_80192838;
            func_80020C74();
            q = D_8009D254;
            P(q, 0x1D8) = o + 0x1B4;
            H(q, 0x1DC) = 4;
            H(q, 0x1DE) = 0x2C;
            W(q, 0x98) |= 0x10000;
            H(q, 0x250) |= 0x400;
            *(S32 *)(q + 0x1E8) = *(S32 *)(a0 + 0x1C);
            B(a0, 0x4B) = 1;
            W(a0, 0x40) = W(D_8009D254, 0x28);
            W(a0, 0x44) = W(D_8009D254, 0x30);
            return;
        }
    }
    *(int *)P(a0, 0x10) = 3;
    P(a0, 0xC) = (unsigned char *)func_80192D50;
}
