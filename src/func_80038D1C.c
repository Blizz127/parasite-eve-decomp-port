/* Phase 5ER: byte test-and-clear return leaf (era path). */
extern unsigned char D_80091A20;

int func_80038D1C(void) {
    unsigned char *flag = &D_80091A20;

    if (*flag != 0) {
        *flag = 0;
        return 0;
    }
    return 0xFF;
}
