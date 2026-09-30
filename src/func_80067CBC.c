/*
 * func_80067CBC — status-word flag setter (VRAM 0x80067CBC, file 0x584BC,
 * 23 words / 0x5C).
 *
 * Retail keeps `&D_800BCF88` in `$a1` for the head load/store pair and
 * rematerializes a fresh base in `$v1` for the tail; the two pinned pointer
 * locals reproduce that split. Head: v = *p; n = v | 0x1000; *p = n; then
 * *p = (n & 0x2000) ? (n & ~0x2000) : (v | 0x3000). Tail: clear 0x8000/0x4000
 * (mask 0xFFFF3FFF) and set 0x4000 through a second base register.
 *
 * ROM: era gcc-2.7.2-psx -O2 -G0.
 */
extern unsigned int D_800BCF88;

int func_80067CBC(void) {
    register unsigned int *p asm("$5");
    register unsigned int *q asm("$3");
    unsigned int v;
    unsigned int n;

    p = &D_800BCF88;
    v = *p;
    n = v | 0x1000u;
    *p = n;
    *p = (n & 0x2000u) ? (n & ~0x2000u) : (v | 0x3000u);
    q = &D_800BCF88;
    *q = (*q & 0xFFFF3FFFu) | 0x4000u;
    return 0;
}
