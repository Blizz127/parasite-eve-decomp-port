/* Indexed record field writer: *(base + a1*16 + 28 + {4,8}) = a2,a3.
 * VRAM 0x8002FAD8 / file 0x202D8 / size 0x20 (8 words).
 * gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 */
void func_8002FAD8(unsigned int *a0, unsigned int a1, unsigned int a2, unsigned int a3) {
    unsigned char i = (unsigned char)a1;
    unsigned char *p = (unsigned char *)(*a0) + (i * 16 + 28);
    *(unsigned int *)(p + 4) = a2;
    *(unsigned int *)(p + 8) = a3;
}
