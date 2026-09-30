/* room_m0418i — func_801927A8, blob offset 0x37C0, 0x15C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * 16-slot sprite draw loop via func_800D3114 (colour vectors from func_800C3134); H()/W() per-array index macros. */

typedef struct { unsigned char b[8]; } CV;
extern unsigned char D_80198820[];
extern short D_800F3374;
extern char *func_800C2B50();
extern void func_800C2EAC(), func_800C3098(), func_800C3238(), func_800C3134(), func_800D3114();
extern unsigned short func_80077A64();
extern unsigned short func_80077AA4();

#define H(off) *(short *)(a2 + i * 2 + (off))
#define W(off) *(int *)(a2 + i * 4 + (off))

void func_801927A8(int a0, int a1, char *a2)
{
    CV o1;
    CV o2;
    char *r;
    unsigned int i;
    int c;

    r = func_800C2B50();
    func_800C2EAC(r[0x24]);
    func_800C3098(0x10);
    func_800C3238(2);
    for (i = 0; i < 16; i++) {
        if (H(0x1C0) != 0) {
            func_800C3134(D_80198820, W(0x180), &o1);
            func_800C3134(D_80198820, W(0x180), &o2);
            o2.b[0] = 0;
            o2.b[1] = 0;
            o2.b[2] = 0;
            D_800F3374 = 0;
            c = func_80077A64(1, 0, 0x340, 0x100);
            func_800D3114(a2 + i * 0x18, 2, H(0x240), 0x20, 0x90, 0x51, 0x10, c,
                          func_80077AA4(0, 0x1D9), H(0x200), &o1, &o2, 1);
        }
    }
}
