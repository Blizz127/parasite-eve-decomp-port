/* VRAM 0x8004905C / file 0x3985C / size 0x54.
 * File-menu open: prime the menu cursor, then hand the gp-relative record
 * D_8009CF30 scaled by 0x14 plus 0x48 to func_8005E8A4 and the gp-relative
 * word D_8009CF68 to func_8006055C.  era -O2 -G8. */
extern int D_8009CF30;
extern int D_8009CF68;
extern void func_8005E8A4();
extern void func_8005EB64();
extern void func_8006055C();

void func_8004905C(void) {
    func_8005E8A4(6, 4);
    func_8005EB64(0x93);
    func_8005E8A4(D_8009CF30 * 0x14 + 0x48, 0);
    func_8006055C(D_8009CF68);
}
