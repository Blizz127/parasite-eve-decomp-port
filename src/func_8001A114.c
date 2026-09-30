/* VRAM 0x8001A114 / file 0xA914 / size 0x48. */
extern int func_80077DC4(int);
int func_8001A114(int arg0) {
    int v = func_80077DC4(*(int *)*(int **)arg0);
    *(int *)*(int **)(arg0 + 4) = v << 4;
    return 1;
}
