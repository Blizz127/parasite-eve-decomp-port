/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80191C94 — blob offset 0x2CA4, 0x154 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80191C94/REPORT.md).
 * Load PE.IMG ranges 16 (-> D_801D0260) and 18 or 17 (-> D_8019CE10, by D_8019C1F0) with retry-on-error polls. A base pointer local for D_800B0CD8 (LBA word at +0x100) keeps it in $s2. */

extern int D_800B0CD8[];
extern unsigned short D_80093150[];
extern unsigned char D_801D0260;
extern unsigned char D_8019CE10;
extern unsigned char D_8019C1F0;
extern int func_8006E6A8();
extern int func_8006E7E8();

#define LOAD(k, dst, label)                                                   \
    label:                                                                    \
    while (func_8006E6A8(base[0x40] + D_80093150[k], dst,               \
                         D_80093150[(k) + 1] - D_80093150[k]) == -1) {        \
    }                                                                         \
    while ((r = func_8006E7E8()) != 0) {                                      \
        if (r == -1) {                                                        \
            goto label;                                                       \
        }                                                                     \
    }

void func_80191C94(void)
{
    int r;
    int *base = D_800B0CD8;
    LOAD(16, &D_801D0260, l0);
    if (D_8019C1F0 == 0) {
        LOAD(18, &D_8019CE10, l1);
    } else {
        LOAD(17, &D_8019CE10, l2);
    }
}
