/* gp base 0x8009CD70; gp+0x1E8 resolves to D_8009CF58. */
extern unsigned char *D_8009CF58;

void func_80053648(unsigned char *state);

void func_80051060(void) {
    func_80053648(D_8009CF58);
}
