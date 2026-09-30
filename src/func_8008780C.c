/* VRAM 0x8008780C / file 0x7800C / size 0x30.
 * SPU voice register 0x1F801C08 + (voice << 4): bit15 from a2>>2 (logical),
 * bits 8-14 from a1, low byte preserved. */
void func_8008780C(int voice, int a1, unsigned int a2) {
    unsigned int base = 0x1F801C08;
    unsigned char *p = (unsigned char *)(base + (voice << 4));
    int hi = (int)((a2 >> 2) << 15);
    int mid = a1 << 8;
    unsigned char low = *p;
    *(unsigned short *)p = (unsigned short)(low | (hi | mid));
}
