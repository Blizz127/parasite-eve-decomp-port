/* VRAM 0x8005010C / file 0x4090C / size 0x6C. era -O2 -G8.
 * gp base 0x8009CD70; gp+0x184 resolves to D_8009CEF4. */
extern void *D_8009CEF4;
void func_80050F64(void);
void func_800638D8(void *node, void (*callback)(void));
void func_80062CC4(void);
void func_8005EB58(int value);
void func_8005EB64(int value);
void func_8005E8A4(int a0, int a1);

void func_8005010C(void *node) {
    D_8009CEF4 = node;
    func_800638D8(node, func_80050F64);
    func_80062CC4();
    func_8005EB58(1);
    func_8005EB64(0x68);
    func_8005E8A4(0, 0x10);
    func_8005EB64(0x68);
    func_8005E8A4(0, 0x10);
    func_8005EB64(0x68);
}
