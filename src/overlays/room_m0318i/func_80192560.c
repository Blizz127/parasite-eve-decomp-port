/* room_m0318i — func_80192560, blob offset 0x3578, 0xC0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * find actor by id bytes (+0xD,+0xC) with live model, call func_800CE870; else log; lever: a1 copy pinned $7 */

extern unsigned char *D_8009D20C;
extern unsigned char *D_8009D254;
extern char D_8018F1CC[];
extern void func_800CE870();
extern void func_80071A74();

unsigned char *func_80192560(int a0, int a1)
{
    unsigned char *p;
    register int b asm("$7") = a1;

    for (p = D_8009D20C; p != 0; p = *(unsigned char **)(p + 4)) {
        if (p != D_8009D254 && *(unsigned char **)p != 0
            && *(int *)(*(unsigned char **)p + 0x10) > 0
            && p[0xD] == a0 && p[0xC] == b) {
            func_800CE870(p, 0);
            return p;
        }
    }
    func_80071A74(D_8018F1CC, a0, b);
    return 0;
}
