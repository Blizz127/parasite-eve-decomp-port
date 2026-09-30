/* VRAM 0x80050178 / file 0x40978 / size 0x50. era -O2 -G8.
 * gp base 0x8009CD70; gp+0x184 resolves to D_8009CEF4. */
extern void *D_8009CEF4;
void func_80050FB8(void);
void func_800638D8(void *node, void (*callback)(void));
void func_8005EB58(int value);
void func_8005EB64(int value);
void func_8005E8A4(int a0, int a1);

void func_80050178(void *node) {
    D_8009CEF4 = node;
    func_800638D8(node, func_80050FB8);
    func_8005EB58(1);
    func_8005EB64(0x68);
    func_8005E8A4(0, 0x10);
    func_8005EB64(0x68);
}
