/* room_m0141i (PE.IMG room m0141i chunk 2, VRAM 0x8018EFE8)
 * func_8018F0E8 — blob offset 0x100, 0x48 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0034i func_8018F0E4; C re-targeted by symbol address
 * (docs/evidence/room_m0141i-ports-2026-09-23/REPORT.md). */

extern int func_800C6CE0();
extern void func_800C2414();
extern char D_80191FF8[];
int func_8018F0E8(void *o)
{
    if (func_800C6CE0(o) == 3) {
        func_800C2414(o, D_80191FF8);
    }
    return 0;
}
