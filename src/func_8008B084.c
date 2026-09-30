/* VRAM 0x8008B084 / file 0x7B884 / size 0x44. */
extern void func_8008A92C(int, int, int);
void func_8008B084(int *arg0) {
    int a = arg0[1];
    int b = arg0[2];
    arg0[1] = 0x400;
    arg0[2] = 0x1000000;
    arg0[3] = 0x80;
    arg0[4] = 0x7F;
    func_8008A92C((int)arg0, a, b);
}
