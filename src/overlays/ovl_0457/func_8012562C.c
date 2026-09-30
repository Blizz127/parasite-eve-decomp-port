/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_8012562C — blob offset 0x492C, 0x118 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_8012562C/REPORT.md).
 * XA stream start: CdInit, wait for a ready drive (retry search on 0/-1), CdSearchFile(D_80125B48), record start/end sector (D_80172CE8/CEC), reset the stream counters and point the ring D_80172D00 into D_80011610. Loop written with goto (keeps li s0,1 per retry); D_80172CE8 through a pointer-to-volatile; CEC/CF0/CF4/CF8/CFA volatile, D00 not (load-bearing store order). */

typedef struct {
    unsigned char minute, second, sector, track;
} CdlLOC;
typedef struct {
    CdlLOC pos;
    unsigned long size;
    char name[16];
} CdlFILE;
extern char *D_80125B48;
extern int D_80172CE8;
extern volatile int D_80172CEC;
extern volatile int D_80172CF0;
extern volatile int D_80172CF4;
extern volatile unsigned short D_80172CF8;
extern volatile unsigned short D_80172CFA;
extern unsigned char *D_80172D00;
extern unsigned char *D_80011610;
extern unsigned short D_80093166;
extern unsigned short D_80093168;
extern void func_8007EC14();
extern void func_80080CC8();
extern int func_8007F72C();
extern int func_8007F778();
extern void func_80073A44();
extern CdlFILE *func_80081414();
extern int func_80080C48();
int func_8012562C(void)
{
    CdlFILE file;
    CdlFILE *r;
    volatile int *start;
    func_8007EC14();
    func_80080CC8(0);
loop:
    while (func_8007F72C() != 1 || func_8007F778() != 0) {
        func_80073A44(0);
    }
    r = func_80081414(&file, D_80125B48);
    if (r == 0) goto loop;
    if (r == (CdlFILE *)-1) goto loop;
    start = &D_80172CE8;
    *start = func_80080C48(&file.pos);
    D_80172CEC = func_80080C48(&file.pos) + (file.size >> 11);
    D_80172CF0 = *start;
    D_80172CF4 = 0;
    D_80172CF8 = 0;
    D_80172CFA = 0;
    D_80172D00 = D_80011610 + (((D_80093168 - D_80093166) << 11) + 0x34800);
    return 0;
}
