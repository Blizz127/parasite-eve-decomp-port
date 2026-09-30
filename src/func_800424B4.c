extern unsigned char D_800A0ED6[];
extern unsigned char D_800A0EF0[];
extern unsigned char D_800A0EF1[];

unsigned char *func_800424B4(int a0, int a1) {
    int rec;
    int off;
    int addr;

    if ((unsigned int)a0 < 2) {
        if (a1 >= 0) {
            rec = a0 * 1048;
            if (a1 < D_800A0ED6[rec]) {
                off = a1 * 68;
                if (D_800A0EF1[off + rec] != 0) {
                    addr = rec + (int)D_800A0EF0;
                    return (unsigned char *)(addr + off);
                }
            }
        }
    }
    return 0;
}
