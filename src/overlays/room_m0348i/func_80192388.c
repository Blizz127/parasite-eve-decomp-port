/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_80192388 — blob offset 0x33a0, 0x28 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_801934A0; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

typedef struct {
    int w0;
    unsigned char pad[0xB0];
    unsigned char flags[8];
} Blk;
void func_80192388(int a0, int a1, Blk *p)
{
    unsigned int i;

    p->w0 = 0;
    for (i = 0; i < 8; i++) {
        p->flags[i] = 0;
    }
}
