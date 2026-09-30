/* VRAM 0x800196E8 / file 0x9EE8 / size 0x40. */
extern int *D_8009D2F0;
extern int D_8009D2F8;
extern short D_8009D264;

int func_800196E8(int **arg0) {
    register int index asm("$3") = *arg0[0];
    D_8009D2F8 = (int)((char *)((void **)D_8009D2F0)[0x9C / 4] + index * 2);
    D_8009D264 = *arg0[1];
    return 1;
}
