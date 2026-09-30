/* room_m0034i (PE.IMG room m0034i chunk 2, VRAM 0x8018EFE8)
 * func_8018F24C — blob offset 0x264, 0x120 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_8018F494; C re-targeted by symbol address
 * (docs/evidence/room_m0034i-ports-2026-09-23/REPORT.md). */

#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int w[8]; } Blk;
extern void *D_80190048;
extern MATRIX D_80190028;
extern Blk D_80190070;
extern void *D_8009D254;
void func_8018F24C(void *o)
{
    D_80190048 = P(o, 0x8);
    D_80190028 = *(MATRIX *)P(D_80190048, 0x238);
    D_80190070 = *(Blk *)((char *)P(D_80190048, 0x238) + 0x620);
    if (P(o, 0x8) == D_8009D254) {
        D_80190070 = *(Blk *)P(D_80190048, 0x238);
    }
}
