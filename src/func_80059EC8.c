/* VRAM 0x80059EC8 / file 0x4A6C8 / size 0x40.
 * Two-entry store pair for indices 0/1: remember the argument in
 * D_8009D090[i] and store whether the gp-relative word D_8009D048 differs
 * from the address D_800C0E48 into D_8009D098[i].  The two indexed stores
 * need the ASPSX 2.30 three-word lui/addu/sw-%lo expansion; the array bases
 * stay absolute as incomplete arrays.  era -O2 -G8 + ERA_ASPSX_VER=2.30. */
extern int D_8009D048;
extern int D_8009D090[];
extern int D_8009D098[];
extern int D_800C0E48[];

void func_80059EC8(unsigned int arg0, int arg1) {
    if (arg0 < 2U) {
        D_8009D090[arg0] = arg1;
        D_8009D098[arg0] = (D_8009D048 != (int)D_800C0E48);
    }
}
