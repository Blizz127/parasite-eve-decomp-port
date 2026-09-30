/* room_m0034i (PE.IMG room m0034i chunk 2, VRAM 0x8018EFE8)
 * func_8018F0E4 — blob offset 0xfc, 0x48 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0186i func_8018FEE8; C re-targeted by symbol address
 * (docs/evidence/room_m0034i-ports-2026-09-23/REPORT.md). */

extern int func_800C6CE0();
extern void func_800C2414();
extern char D_8018FF48[];
int func_8018F0E4(void *o)
{
    if (func_800C6CE0(o) == 3) {
        func_800C2414(o, D_8018FF48);
    }
    return 0;
}
