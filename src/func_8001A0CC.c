/* VRAM 0x8001A0CC / file 0xA8CC / size 0x48. */
extern int func_80077CF4(int);
int func_8001A0CC(int arg0) {
    int v = func_80077CF4(*(int *)*(int **)arg0);
    *(int *)*(int **)(arg0 + 4) = v << 4;
    return 1;
}
