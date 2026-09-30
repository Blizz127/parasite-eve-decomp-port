/* room_m0065i (PE.IMG room m0065i chunk 2, VRAM 0x8018EFE8)
 * func_8018F09C — blob offset 0xb4, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0167i func_8018F28C; C re-targeted by symbol address
 * (docs/evidence/room_m0065i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F128();
extern char D_8018FFF8[];
extern char D_80190010[];
extern char D_8019001C[];
int func_8018F09C(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_80190010);
        r |= func_800C2758(o, D_8018FFF8, D_8019001C);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F128(o);
    }
    return 0;
}
