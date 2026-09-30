/* VRAM 0x80052894 / file 0x43094 / size 0x30.
 * 12-byte-stride table read divided by 60 (multu 0x88888889 + srl 5). */
extern unsigned int D_800A76A4[];

unsigned int func_80052894(int index) {
    return D_800A76A4[index * 3] / 60u;
}
