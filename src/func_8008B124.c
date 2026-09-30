/* VRAM 0x8008B124 / file 0x7B924 / size 0x44. */
extern void func_8008AB1C(int, int, int);
extern void func_8008A92C(int, int, int);
void func_8008B124(int arg0) {
    int p1;
    int p2;
    func_8008AB1C((int)&p1, (int)&p2, *(int *)(arg0 + 4));
    func_8008A92C(arg0, p1, p2);
}
