/* Phase 5ER: word test-and-clear return leaf (era path). */
extern int D_80091A24;

int func_80038D48(void) {
    int *flag = &D_80091A24;

    if (*flag != 0) {
        *flag = 0;
        return 0;
    }
    return 0xFF;
}
