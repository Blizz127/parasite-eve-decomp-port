/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_801934A0 — blob offset 0x44B8, 0x28 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Clear the header word and the 8 flag bytes at +0xB4. */

typedef struct {
    int w0;
    unsigned char pad[0xB0];
    unsigned char flags[8];
} Blk;
void func_801934A0(int a0, int a1, Blk *p)
{
    unsigned int i;

    p->w0 = 0;
    for (i = 0; i < 8; i++) {
        p->flags[i] = 0;
    }
}
