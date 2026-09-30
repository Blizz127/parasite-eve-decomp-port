/*
 * func_80067678 — VRAM 0x80067678, size 0x54, file 0x57E78-0x57ECC.
 *
 * Sets/clears bit 1 (0x02) of the byte field at record `arg0` (stride 0x38)
 * in the table at *D_800B1624 + 0x14. Returns 0.
 *
 * era -O2 -G0. The `extern ... *volatile` declaration reproduces retail's
 * two independent loads of D_800B1624 (the base and the 0x14 field).
 */
typedef struct {
    char pad[0x14];
    int field_14;
} Header;

extern Header *volatile D_800B1624;

int func_80067678(int arg0, int arg1) {
    unsigned char *p = (unsigned char *)D_800B1624 + D_800B1624->field_14 + arg0 * 56;
    if (arg1 != 0) {
        *p |= 2;
    } else {
        *p &= ~2;
    }
    return 0;
}
