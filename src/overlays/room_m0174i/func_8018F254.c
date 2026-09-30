/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_8018F254 — blob offset 0x26C, 0xAC bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Register D_80196CE0 through func_800C22F8 and seed the room light globals; return 0. */

extern int *func_800C22F8();
extern char D_80196CE0[];
extern unsigned char D_80197434;
extern unsigned char D_80197435;
extern unsigned char D_80197454;
extern unsigned char D_80197455;
extern short D_8019745A;
extern short D_80197438;
extern short D_8019743A;
extern unsigned char D_80197436;
extern short D_80197458;
extern unsigned char D_80197450;
extern unsigned char D_80197451;
extern unsigned char D_80197452;
extern unsigned char D_80197456;
int func_8018F254(void)
{
    *func_800C22F8() = (int)D_80196CE0;
    D_80197434 = 32;
    D_80197435 = 3;
    D_80197454 = 43;
    D_80197455 = 2;
    D_8019745A = 128;
    D_80197438 = 0;
    D_8019743A = 0;
    D_80197436 = 0;
    D_80197458 = 0;
    D_80197450 = 128;
    D_80197451 = 128;
    D_80197452 = 128;
    D_80197456 = 0;
    return 0;
}
