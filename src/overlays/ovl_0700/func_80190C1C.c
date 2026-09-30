/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80190C1C — blob offset 0x1C2C, 0xEC bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80190C1C/REPORT.md).
 * Allocate a pool record in the 7-argument layout (shorts 0..6, words 8/0x38/0x3C, f36 = func_8019959C(a6, a1)) and insert under parent. */

typedef struct Rec2 {
    short id;          /* 0x00 */
    short f02;
    short f04;
    short f06;
    int f08;           /* 0x08 */
    unsigned char pad0C[0x14];
    int f20;           /* 0x20 */
    int f24;
    int f28;
    unsigned char pad2C[0xA];
    short f36;         /* 0x36 */
    int f38;           /* 0x38 */
    int f3C;           /* 0x3C */
    unsigned char pad40[0x2C];
} Rec2;                /* 0x6C */
extern Rec2 D_801E4E00[];
extern short func_801915DC();
extern short func_8019959C();
extern void func_801917BC();
Rec2 *func_80190C1C(void *parent, short a1, short a2, short a3, int a4, int a5, int a6)
{
    short id = func_801915DC();
    Rec2 *r = &D_801E4E00[id];
    r->f06 = a1;
    r->f04 = a2;
    r->f02 = a3;
    r->id = id;
    r->f08 = a6;
    r->f36 = func_8019959C(a6, a1);
    r->f38 = a4;
    r->f3C = a5;
    func_801917BC(r, parent);
    r->f20 = 0;
    r->f24 = 0;
    r->f28 = 0;
    return r;
}
