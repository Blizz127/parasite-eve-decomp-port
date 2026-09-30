/* room_m0075i (PE.IMG room m0075i chunk 2, VRAM 0x8018EFE8)
 * func_8018F284 — blob offset 0x29C, 0xC4 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Two lookups gated on func_800C6CE0 and the ctx mode/ready bytes; on -1 call func_8018F348. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F348();
extern char D_80193F20[];
extern char D_80193F50[];
extern char D_80193F68[];
int func_8018F284(void *o)
{
    int r = 0;
    void *c;

    if (func_800C6CE0(o) >= 2) {
        c = P(o, 0x8);
        if (B(c, 0xE) >= 2 || B(P(c, 0x0), 0xAC) != 0) {
            r = func_800C251C(o, D_80193F50);
            r |= func_800C2758(o, D_80193F20, D_80193F68);
        }
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F348(o);
    }
    return 0;
}
