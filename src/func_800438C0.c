/* Phase 5EH: arg+return branchy leaf func_800438C0 (era path).
 * VRAM 0x800438C0 / gp+0x180 -> D_8009CEF0. gp base 0x8009CD70.
 * era -O2 -G8: double store must survive; return-0 addu in jr slot (5EA). */
extern int D_8009CEF0;
int func_800438C0(int arg0) {
    int m = arg0 & 0x1FF;
    D_8009CEF0 = m;
    if (m == 0) D_8009CEF0 = 0x1FF;
    return 0;
}
