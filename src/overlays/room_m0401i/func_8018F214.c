/* room_m0401i (PE.IMG m0401i chunk 2, VRAM 0x8018EFE8)
 * func_8018F214 — blob offset 0x22C, 0x2C bytes (two header words precede it: [off-8, bin]).
 * Profile era_o2_g0 (default); LINK_EXACT at the room VMA. Lane ovl 2026-09-27. */

extern void **func_800C22F8(void);
extern char D_8019535C[];

int func_8018F214(void)
{
    *func_800C22F8() = D_8019535C;
    return 0;
}
