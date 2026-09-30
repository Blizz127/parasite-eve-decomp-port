/* Indexed record writer with many stack extras.
 * VRAM 0x8002FA10 / file 0x20210 / size 0x94 (37 words).
 * gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 */
void func_8002FA10(unsigned int *a0, unsigned int a1, unsigned int a2, unsigned int a3,
                   unsigned char a4, unsigned short a5, unsigned char a6, unsigned char a7,
                   unsigned char a8, unsigned char a9, unsigned char a10, unsigned char a11) {
    unsigned char i = (unsigned char)a1;
    unsigned char *p = (unsigned char *)(*a0) + (i * 16 + 28);
    unsigned char *q;
    p[0] = 0;
    p[1] = (unsigned char)a2;
    p[2] = (unsigned char)a3;
    p[3] = a4;
    *(unsigned short *)(p + 12) = a5;
    p[14] = a10;
    p[15] = a11;
    q = (unsigned char *)(*a0) + (i * 4);
    q[0x7C] = a6;
    q = (unsigned char *)(*a0) + (i * 4);
    q[0x7D] = a7;
    q = (unsigned char *)(*a0) + (i * 4);
    q[0x7E] = a8;
    q = (unsigned char *)(*a0) + (i * 4);
    q[0x7F] = a9;
}
