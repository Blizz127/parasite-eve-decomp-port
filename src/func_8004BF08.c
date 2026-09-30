/* Phase 5ES: first loop-as-volume leaf (era path). */
extern int D_800A1920[8];
extern int D_800A1940[8];

void func_8004BF08(void) {
    int i;
    int *a;
    int *b;

    i = 0;
    a = D_800A1920;
    b = D_800A1940;
    do {
        *b = 0;
        *a = 0;
        a++;
        i++;
        b++;
    } while (i < 8);
}
