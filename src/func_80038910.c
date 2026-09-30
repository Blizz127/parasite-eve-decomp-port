/*
 * func_80038910 — seed a fixed gp-relative record from 6 arguments.
 *
 * VRAM 0x80038910 / file 0x29110 / size 0x30 (12 words).
 * ROM: asm/disc1/28070.s.
 *
 * Args 5 and 6 arrive on the stack (0x10/0x14($sp)); the first four are in
 * $a0..$a3. Every destination is gp-relative (_gp = 0x8009CD70): halfwords at
 * 0x148/0x14C/0x150 and bytes at 0x144/0x154/0x158/0x15C. The literal 1 is
 * materialised into $v1 for the flag byte before the first halfword store.
 */
extern unsigned short D_8009CEB8;   /* 0x148($gp) */
extern unsigned short D_8009CEBC;   /* 0x14C($gp) */
extern unsigned short D_8009CEC0;   /* 0x150($gp) */
extern unsigned char D_8009CEB4;    /* 0x144($gp) */
extern unsigned char D_8009CEC4;    /* 0x154($gp) */
extern unsigned char D_8009CEC8;    /* 0x158($gp) */
extern unsigned char D_8009CECC;    /* 0x15C($gp) */

void func_80038910(unsigned int a0, unsigned int a1, unsigned int a2,
                   unsigned int a3, unsigned char a4, unsigned char a5) {
    D_8009CEB8 = a0;
    D_8009CEB4 = 1;
    D_8009CEBC = a1;
    D_8009CEC0 = a2;
    D_8009CEC4 = a3;
    D_8009CEC8 = a4;
    D_8009CECC = a5;
}
