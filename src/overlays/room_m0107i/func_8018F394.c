/* room_m0107i (PE.IMG room m0107i chunk 2, VRAM 0x8018EFE8)
 * func_8018F394 — blob offset 0x3ac, 0x10c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0421i func_8018F210; C re-targeted by symbol address
 * (docs/evidence/room_m0107i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { int w[8]; } Blk;
extern void func_800C2B40();
extern int func_8006DC18();
extern int func_8006E498();
extern int D_800B0E64;
extern short D_80193728;
extern short D_8019372A;
extern unsigned char D_80193724;
extern unsigned char D_80193725;
extern unsigned char D_80193720;
extern unsigned char D_80193721;
extern unsigned char D_80193722;
extern unsigned char D_80193726;
extern int D_8019372C;
void func_8018F394(void *o, int a1, void *p)
{
    void *c;

    func_800C2B40(p);
    c = P(o, 0x8);
    P(p, 0x0) = c;
    *(Blk *)((char *)p + 4) = *(Blk *)P(c, 0x238);
    H(p, 0x2A) = 0;
    H(p, 0x2C) = 0;
    H(p, 0x28) = 30;
    W(p, 0x24) = func_8006DC18(35);
    D_80193728 = -300;
    D_8019372A = 128;
    D_80193724 = 0;
    D_80193725 = 0;
    D_80193720 = 128;
    D_80193721 = 128;
    D_80193722 = 128;
    D_80193726 = 0;
    D_8019372C = func_8006E498(D_800B0E64, 0xCB8704);
}
