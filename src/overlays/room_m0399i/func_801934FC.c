/* room_m0399i (PE.IMG room m0399i chunk 2, VRAM 0x8018EFE8)
 * func_801934FC — blob offset 0x4514, 0x64 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0145i func_801924F8; C re-targeted by symbol address
 * (docs/evidence/room_m0399i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_801937CC();
int func_801934FC(void *o)
{
    void *c = P(o, 0x8);

    SB(o, 0x16) = -1;
    SB(o, 0x17) = -1;
    SB(o, 0x18) = -1;
    SB(o, 0x19) = 3;
    W(o, 0x10) = 0;
    H(o, 0x14) = 0;
    B(o, 0x1A) = 0;
    H(o, 0x32) = 0;
    H(o, 0x36) = 0;
    P(o, 0xC) = func_801937CC;
    W(c, 0x98) |= 0x10002;
    H(c, 0x250) |= 0x400;
    return 0;
}
