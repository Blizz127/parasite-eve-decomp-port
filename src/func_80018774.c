/* VRAM 0x80018774 / file 0x8F74 / size 0x4C. */
extern char *D_8009D2F0;
extern int func_8006F39C(int, char *);
int func_80018774(int arg0) {
    int v = func_8006F39C(*(int *)*(int **)arg0, D_8009D2F0);
    *(int *)*(int **)(arg0 + 4) = v;
    return 1;
}
