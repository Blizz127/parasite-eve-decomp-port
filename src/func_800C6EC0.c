/* VRAM 0x800C6EC0 / file 0xB76C0 / size 0x18. */
extern unsigned short D_800F346C;
extern unsigned short D_800F3414;

void func_800C6EC0(unsigned int first, unsigned int second) {
    D_800F346C = (unsigned short)first;
    D_800F3414 = (unsigned short)second;
}
