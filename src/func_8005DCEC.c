/* Indexed table lookup with a null fallback (field +0xC).
 * VRAM 0x8005DCEC / file 0x4E4EC / size 0x50. */
extern int D_800A802C;
extern char D_800A8028[];

unsigned char *func_8005DCEC(unsigned int a0) {
    unsigned char *v0 = (unsigned char *)(D_800A802C + (int)D_800A8028);
    unsigned char *v1 = v0 + *(int *)(v0 + 0xC);

    if (!(a0 < *(unsigned short *)v1))
        return 0;
    return v1 + *(short *)(v1 + a0 * 2 + 2);
}
