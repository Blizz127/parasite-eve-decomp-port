/* room_m0087i (PE.IMG room m0087i chunk 2, VRAM 0x8018EFE8)
 * func_8018F22C — blob offset 0x244, 0x48 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F054; C re-targeted by symbol address
 * (docs/evidence/room_m0087i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern void func_800C2414();
extern char D_801934F8[];
int func_8018F22C(void *o)
{
    if (func_800C6CE0(o) >= 2) {
        func_800C2414(o, D_801934F8);
    }
    return 0;
}
