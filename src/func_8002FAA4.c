/* Indexed record field writer with stack extras.
 * VRAM 0x8002FAA4 / file 0x202A4 / size 0x34 (13 words).
 * gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * ROM pulls extra args from 16(sp) (u8) and 20(sp) (u16).
 */
void func_8002FAA4(unsigned int *a0, unsigned int a1, unsigned int a2, unsigned int a3,
                   unsigned char a4, unsigned short a5) {
    unsigned char i = (unsigned char)a1;
    unsigned char *p = (unsigned char *)(*a0) + (i * 16 + 28);
    p[0] = 0;
    p[1] = (unsigned char)a2;
    p[2] = (unsigned char)a3;
    p[3] = a4;
    *(unsigned short *)(p + 12) = a5;
}
