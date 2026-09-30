/* VRAM 0x8004FDA4 / file 0x405A4 / size 0x44. */
extern int func_80042770(void);
int func_8004FDA4(int arg0) {
    int r = 0;
    if (arg0 == 2 || func_80042770() != 0) {
        r = 1;
    }
    return r;
}
