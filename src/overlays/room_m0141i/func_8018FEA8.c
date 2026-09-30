/* room_m0141i (PE.IMG room m0141i chunk 2, VRAM 0x8018EFE8)
 * func_8018FEA8 — blob offset 0xEC0, 0x74 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Fill 16 short vectors with the func_800C2B50() position; +0x80 = 2048, +0x82 = 128. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void *func_800C2B50();
void func_8018FEA8(int a0, int a1, void *p)
{
    void *r = func_800C2B50();
    unsigned int i;
    struct { short x, y, z, pad; } *v = p;

    for (i = 0; i < 16; i++) {
        v[i].x = W(r, 0x18);
        v[i].y = W(r, 0x1C);
        v[i].z = W(r, 0x20);
    }
    H(p, 0x82) = 128;
    H(p, 0x80) = 2048;
}
