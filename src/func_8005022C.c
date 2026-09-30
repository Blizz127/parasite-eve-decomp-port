extern int D_8009CF58;
extern int D_8009CF20;
extern int D_8009CEF4;
extern void func_800638D8(unsigned char *a, void (*f)(void));
extern void func_80050AD8(void);
void func_8005022C(unsigned char *a0) {
    D_8009CEF4 = (int)a0;
    D_8009CF20 = D_8009CF58;
    func_800638D8(a0, func_80050AD8);
}
