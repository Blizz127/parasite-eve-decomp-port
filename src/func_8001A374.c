/* VRAM 0x8001A374 / file 0xAB74 / size 0x1C. */
extern unsigned char D_800BCFFC;

int func_8001A374(int **arg0) {
    D_800BCFFC = **arg0;
    return 1;
}
