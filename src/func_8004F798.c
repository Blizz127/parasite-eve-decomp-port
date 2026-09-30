/* VRAM 0x8004F798 / file 0x3FF98 / size 0x40. era -O2 -G8.
 * gp base 0x8009CD70; gp+0x1E8 resolves to D_8009CF58. */
extern unsigned char *D_8009CF58;
void func_8005E8A4(int a0, int a1);
int func_8005DCEC(int a0);
int func_8005F27C(int a0);

void func_8004F798(void) {
    func_8005E8A4(4, 4);
    func_8005F27C(func_8005DCEC(D_8009CF58[4] - 1));
}
