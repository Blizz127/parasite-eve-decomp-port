/* room_m0391i (PE.IMG room m0391i chunk 2, VRAM 0x8018EFE8)
 * func_8018F1E8 — blob offset 0x200, 0x2C bytes (five data words precede it:
 * [0x1EC, asm]; shared with m0394i, m0395i). Profile era_o2_g0 (default).
 * Twin of room_m0399i func_8018F1DC with the pointer retargeted: store the
 * D_8019437C pointer through func_800C22F8's result; return 0. */

extern int *func_800C22F8();
extern char D_8019437C[];
int func_8018F1E8(void)
{
    *func_800C22F8() = (int)D_8019437C;
    return 0;
}
