/*
 * func_8003E610 — boot display/graphics bring-up (Boot Rung 1, called by
 * main directly after func_8006A64C). Straight-line dispatcher: ten calls
 * with immediate args, no branches, no loops, no $gp, no globals.
 * 0x140/0xE0 = 320x224 — the display resolution.
 * ROM: asm/disc1/2E7D8.s @ file 0x2EE10, 28 words (0x70 bytes), frame 0x18.
 */

extern void func_80073C94(void);
extern void func_8003E754(int w, int h);
extern void func_8007D054(void);
extern void func_80077F7C(void);
extern void func_80079004(int a, int b);
extern void func_80079024(int a);
extern void func_800409B4(void);
extern void func_8003E944(void);
extern void func_8007EC14(void);
extern void func_80080CC8(int a);

void func_8003E610(void) {
    func_80073C94();
    func_8003E754(0x140, 0xE0); /* 320x224 display */
    func_8007D054();
    func_80077F7C();
    func_80079004(0xA0, 0x70);
    func_80079024(0xF0);
    func_800409B4();
    func_8003E944();
    func_8007EC14();
    func_80080CC8(0);
}
