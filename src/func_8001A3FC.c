/* VRAM 0x8001A3FC / file 0xABFC / size 0x40. */
extern int *D_8009D2F0;
extern int D_8009D248;
extern short D_8009D1CC;

int func_8001A3FC(int **arg0) {
    register int index asm("$3") = *arg0[0];
    D_8009D248 = (int)((char *)((void **)D_8009D2F0)[0x9C / 4] + index * 2);
    D_8009D1CC = *arg0[1];
    return 1;
}
