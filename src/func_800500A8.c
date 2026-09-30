/* VRAM 0x800500A8 / file 0x408A8 / size 0x64. era -O2 -G8.
 * gp base 0x8009CD70; gp+0x1E4 resolves to D_8009CF54. */
extern unsigned int D_8009CF54;
void *func_80062A34(int a, int b);
int func_8005DC4C(int value);
void func_80050F10(void);
void func_800638D8(void *node, void (*callback)(void));

void func_800500A8(void *node) {
    void *entry = func_80062A34(2, 0x17);
    int text;

    if (entry != 0) {
        text = *(int *)((char *)entry + 0x48) + 0x73;
    } else {
        text = 0x75;
    }
    D_8009CF54 = func_8005DC4C(text);
    func_800638D8(node, func_80050F10);
}
