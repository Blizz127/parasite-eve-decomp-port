/* room_m0392i (PE.IMG room m0392i chunk 2, VRAM 0x8018EFE8)
 * func_8018F278 — blob offset 0x290, 0x48 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F054; C re-targeted by symbol address
 * (docs/evidence/room_m0392i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern void func_800C2414();
extern char D_8019449C[];
int func_8018F278(void *o)
{
    if (func_800C6CE0(o) >= 2) {
        func_800C2414(o, D_8019449C);
    }
    return 0;
}
