/* Initialize the fixed 0x800BE9A0 work buffer and advance its service.
 * VRAM 0x8003E944 / file 0x2F144 / size 0x30.
 */
extern void func_800844E4(void *arg0, void *arg1);
extern void func_80082534(void);
extern unsigned char D_800BE9A0[];

void func_8003E944(void) {
    func_800844E4(D_800BE9A0, D_800BE9A0 + 0x22);
    func_80082534();
}
