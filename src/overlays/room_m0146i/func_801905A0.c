/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_801905A0 — blob offset 0x15b8, 0x48 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F054; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern void func_800C2414();
extern char D_80192528[];
int func_801905A0(void *o)
{
    if (func_800C6CE0(o) >= 2) {
        func_800C2414(o, D_80192528);
    }
    return 0;
}
