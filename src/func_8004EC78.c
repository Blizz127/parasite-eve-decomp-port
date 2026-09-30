extern void func_80052E30(int a);
extern void func_800638D8(unsigned char *a, void (*f)(void));
extern void func_800506E8(void);
void func_8004EC78(unsigned char *a0) {
    func_80052E30(1);
    func_800638D8(a0, func_800506E8);
}
