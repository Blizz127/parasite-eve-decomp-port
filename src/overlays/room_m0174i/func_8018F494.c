/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_8018F494 — blob offset 0x4AC, 0x120 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Cache the ctx actor in D_80197408 and copy its model matrix and light block (the player's own when ctx is the player). */

#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int w[8]; } Blk;
extern void *D_80197408;
extern MATRIX D_80196FC8;
extern Blk D_801974E0;
extern void *D_8009D254;
void func_8018F494(void *o)
{
    D_80197408 = P(o, 0x8);
    D_80196FC8 = *(MATRIX *)P(D_80197408, 0x238);
    D_801974E0 = *(Blk *)((char *)P(D_80197408, 0x238) + 0x620);
    if (P(o, 0x8) == D_8009D254) {
        D_801974E0 = *(Blk *)P(D_80197408, 0x238);
    }
}
