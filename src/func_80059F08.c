/* Mode-setup selector: arms the D_8009D048 record pointer and its accessor
 * table from either the live D_8009D04C context or func_80052F70, then
 * returns the D_8009D090[arg0] entry.  VRAM 0x80059F08 / file 0x4A708 /
 * size 0xC8.  era_o2_g8: the six state words are gp-relative; D_8009D050 is
 * D_8009D04C + 4 and D_8009D064 is D_8009D058 + 0xC (no symbols of their
 * own).  D_8009D090/D_8009D098 stay absolute indexed arrays. */
extern int D_8009D098[];
extern int D_8009D090[];
extern int D_8009D048;
extern int D_8009D04C;
extern int D_8009D054;
extern int D_8009D058;
extern unsigned char D_800A1F84[];
extern unsigned char D_800C0E48[];
extern unsigned char D_8009D05C[];

extern int func_80052F70(void);

int func_80059F08(unsigned int arg0) {
    if (arg0 < 2) {
        if ((D_8009D098[arg0] != 0) && (D_8009D04C != 0)) {
            D_8009D048 = D_8009D04C;
            D_8009D058 = (int)D_800A1F84;
            *(int *)((char *)&D_8009D058 + 0xC) = 4;
            *(int *)((char *)&D_8009D04C + 4) = D_8009D054;
        } else {
            D_8009D048 = (int)D_800C0E48;
            *(int *)((char *)&D_8009D04C + 4) = func_80052F70();
            D_8009D058 = (int)D_8009D05C;
            *(int *)((char *)&D_8009D058 + 0xC) = 2;
        }
        return D_8009D090[arg0];
    }
    return -1;
}
