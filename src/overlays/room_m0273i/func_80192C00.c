/* room_m0273i — func_80192C00, blob offset 0x3C18, 0x150 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Player-state reset + camera seed; D_8009D248 read into a local up front, D_800BCF88 |= via pointer local. */

extern char *D_8009D254;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern unsigned int D_800BCF88;
extern int func_8001CAB0();
extern void func_8001AA78();

#define W(p, o) (*(int *)((char *)(p) + (o)))
#define H(p, o) (*(short *)((char *)(p) + (o)))

void func_80192C00(char *a0, char *a1)
{
    char *p;
    char *q;
    unsigned int *r;
    int k;

    p = D_8009D254;
    k = D_8009D248;
    W(p, 0x98) &= 0xFFFEFFFF;
    H(p, 0x250) &= 0xFBFF;
    W(p, 0x1D8) = 0;
    H(p, 0x1DC) = 0;
    if (func_8001CAB0(W(W(a0, 0x238), 0x594) << 16, W(W(a0, 0x238), 0x59C) << 16, k, D_8009D1CC)) {
        W(D_8009D254, 0x28) = W(W(a0, 0x238), 0x594) << 16;
        W(D_8009D254, 0x30) = W(W(a0, 0x238), 0x59C) << 16;
    } else {
        W(D_8009D254, 0x28) = W(a1, 0x34);
        W(D_8009D254, 0x30) = W(a1, 0x38);
    }
    func_8001AA78(D_8009D254);
    q = D_8009D254;
    W(q, 0x68) = 0;
    W(q, 0x6C) = 0;
    W(q, 0x70) = 0;
    W(q, 0x78) = 0;
    W(q, 0x7C) = 0;
    W(q, 0x80) = 0;
    W(q, 0x40) = W(q, 0x28);
    W(q, 0x44) = W(q, 0x2C);
    W(q, 0x48) = W(q, 0x30);
    r = &D_800BCF88;
    *r |= 0x80;
    a1[0x3F] = 0;
}
