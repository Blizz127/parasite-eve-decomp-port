/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80192740 — blob offset 0x3750, 0xC0 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80192740/REPORT.md).
 * Link the double-buffered 0x24-byte prim into ot[0xFFF] (PSY-Q addPrim via P_TAG bitfields) when D_8019C1F0 == 1, then step D_8019C02C through func_80193B5C. The != ternary order is load-bearing (3 words). */

typedef struct {
    unsigned addr : 24;
    unsigned len : 8;
} P_TAG;
typedef struct {
    int f00;
    unsigned int *ot;
} DB;
extern unsigned char D_8019C1F0;
extern DB *D_8019C9C0;
extern DB D_8019C1F8;
extern unsigned char D_801EA598[2][0x24];
extern short D_8019C02C;
extern short func_80193B5C();

#define PRIM() ((D_8019C9C0 != &D_8019C1F8) ? D_801EA598[1] : D_801EA598[0])

void func_80192740(void)
{
    if (D_8019C1F0 == 1) {
        ((P_TAG *)PRIM())->addr = ((P_TAG *)&D_8019C9C0->ot[0xFFF])->addr;
        ((P_TAG *)&D_8019C9C0->ot[0xFFF])->addr = (unsigned int)PRIM();
    }
    if (D_8019C02C != 0) {
        D_8019C02C = func_80193B5C(D_8019C02C);
    }
}
