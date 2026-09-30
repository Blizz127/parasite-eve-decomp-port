/* VRAM 0x800C653C / file 0xB6D3C / size 0x48. */
extern int func_800C62DC(int, int);
int func_800C653C(int arg0, int arg1) {
    int a = func_800C62DC(arg0, arg1);
    return a | func_800C62DC(arg0, arg1 + 8);
}
