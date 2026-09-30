/* room_m0414i (PE.IMG room m0414i chunk 2, VRAM 0x8018EFE8)
 * func_8018F0B4 — blob offset 0xcc, 0x48 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0034i func_8018F0E4; C re-targeted by symbol address
 * (docs/evidence/room_m0414i-ports-2026-09-23/REPORT.md). */

extern int func_800C6CE0();
extern void func_800C2414();
extern char D_80191F94[];
int func_8018F0B4(void *o)
{
    if (func_800C6CE0(o) == 3) {
        func_800C2414(o, D_80191F94);
    }
    return 0;
}
