/*
 * func_8006E2D0 — VRAM 0x8006E2D0, size 0x68, file 0x5EAD0-0x5EB38.
 *
 * Expands six 5-bit codes packed in arg1 (bit 27 down to bit 2, stride 5)
 * through the D_800930B4 alphabet, storing one character per byte of arg0
 * and upper-casing a..z (c - 0x20). arg0[6] = 0. era -O2 -G0; the shift
 * count is written `(5 - i) * 5 + 2` so cc1 hoists the constant 5.
 */
extern unsigned char D_800930B4[];

int func_8006E2D0(char *arg0, unsigned int arg1) {
    int i;
    for (i = 0; i < 6; i++) {
        unsigned char c = D_800930B4[(arg1 >> ((5 - i) * 5 + 2)) & 0x1F];
        arg0[i] = c;
        if ((unsigned char)(c - 0x61) < 0x1A)
            arg0[i] = c - 0x20;
    }
    arg0[6] = 0;
    return 0;
}
