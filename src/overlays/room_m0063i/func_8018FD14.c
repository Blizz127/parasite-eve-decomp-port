/* room_m0063i (PE.IMG room m0063i chunk 2, VRAM 0x8018EFE8)
 * func_8018FD14 — blob offset 0xd2c, 0x48 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0358i func_80193208; C re-targeted by symbol address
 * (docs/evidence/room_m0063i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_8018FD5C();
void func_8018FD14(void *o)
{
    void *c = P(P(o, 0x8), 0x18C);

    if (*(unsigned short *)((char *)c + 0x1A) >= (unsigned int)(B(c, 0xF) - 1)) {
        func_8018FD5C(o);
    }
}
