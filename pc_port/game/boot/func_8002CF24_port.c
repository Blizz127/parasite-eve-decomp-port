/*
 * PE-BTL2 — func_8002CF24_mode7_cut: D_8009D28C = 7
 *
 * Named cut of the inlined battle-exit store. There is no jal to
 * 0x8002CF24; the two words sit inside the large 0x800299CC-family
 * dispatcher after the victory/exit work at 0x8002CF1C.
 *
 *   0x8002CF24..0x8002CF2C exclusive (2 words / 8 bytes, file 0x1D724).
 *   First excluded word: addiu $v0, 70 at 0x8002CF2C (later UI bytes;
 *   not this cut).
 *
 * Words dumped from SHA-1-exact build/disc1.candidate.exe
 * (452fb033f2eaa4b18aa20a5bca60b8125af3a37b):
 *
 *   0x8002CF24  24020007  addiu $v0, $zero, 7
 *   0x8002CF28  AF82051C  sw    $v0, 0x51C($gp)   ; D_8009D28C
 *
 * $gp = 0x8009CD70; 0x8009CD70+0x51C = 0x8009D28C.
 * Matching leaves only store 0/3/4/5/6/8. This is the ROM writer of 7.
 *
 * Do not invent ATB, menus, damage, PE, AI, HP layout, or a command
 * word. Those stores are not in this two-word window.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D28C 0x8009D28Cu /* gp+0x51C */

void func_8002CF24_mode7_cut(void)
{
    PE_StoreU32(GA_D_8009D28C, 7u);
}
