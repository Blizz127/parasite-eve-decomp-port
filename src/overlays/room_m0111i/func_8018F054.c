/* room_m0111i (PE.IMG room m0111i chunk 2, VRAM 0x8018EFE8)
 * func_8018F054 — blob offset 0x6c, 0x48 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F054; C re-targeted by symbol address
 * (docs/evidence/room_m0111i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern void func_800C2414();
extern char D_801900AC[];
int func_8018F054(void *o)
{
    if (func_800C6CE0(o) >= 2) {
        func_800C2414(o, D_801900AC);
    }
    return 0;
}
