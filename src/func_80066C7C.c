/*
 * func_80066C7C — VRAM 0x80066C7C, size 0x6C, file 0x5747C-0x574E8.
 *
 * Clears the window words D_800BCFE8/EA/EC to zero (after snapshotting the
 * old values into D_800BCFF0/2/4), sets mode byte D_800BCFEE = 6, the signed
 * halfword D_800BCFF6 = arg0 and the zero halfword D_800BCFF8, then returns 0.
 *
 * era -O2 -G0; sibling of func_80066BD8 (same globals, same store shape).
 */
extern unsigned short D_800BCFE8;
extern unsigned short D_800BCFEA;
extern unsigned short D_800BCFEC;
extern unsigned char D_800BCFEE;
extern unsigned short D_800BCFF0;
extern unsigned short D_800BCFF2;
extern unsigned short D_800BCFF4;
extern short D_800BCFF6;
extern unsigned short D_800BCFF8;

int func_80066C7C(short arg0) {
    unsigned short a = D_800BCFE8;
    unsigned short b = D_800BCFEA;
    unsigned short c = D_800BCFEC;
    D_800BCFEE = 6;
    D_800BCFE8 = 0;
    D_800BCFEA = 0;
    D_800BCFEC = 0;
    D_800BCFF6 = arg0;
    D_800BCFF8 = 0;
    D_800BCFF0 = a;
    D_800BCFF2 = b;
    D_800BCFF4 = c;
    return 0;
}
