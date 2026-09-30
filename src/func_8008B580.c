/* VRAM 0x8008B580 / file 0x7BD80 / size 0x38. */
extern unsigned short D_8009D2A2;
extern int D_8009D2B4;
extern void func_8008D7D0(void);
void func_8008B580(int arg0) {
    unsigned short v = *(unsigned short *)(arg0 + 4);
    D_8009D2A2 = 0;
    D_8009D2B4 = v << 16;
    func_8008D7D0();
}
