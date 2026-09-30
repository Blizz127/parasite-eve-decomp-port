/* Reader leaf: pins D_800A1870 as a function pointer by proof (jalr).
 * VRAM 0x80042B6C / file 0x3336C / size 0x5C.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Type evidence (live unit 33338.s):
 *   lw $a0, %lo(D_800A1870); ... jalr $a0
 *   No argument setup after the load; return value unused → void (*)(void).
 *
 * D_800A1874 is referenced only to match this leaf (counter +1 / == 4).
 * It remains WIDTH-ONLY for harvest — not a type pin for 5EG setters.
 */
typedef void (*func_800A1870_t)(void);

extern func_800A1870_t D_800A1870;
extern int D_800A1874;

void func_80042B6C(void) {
    func_800A1870_t cb = D_800A1870;

    if (cb != 0) {
        int c = D_800A1874 + 1;
        D_800A1874 = c;
        if (c == 4) {
            cb();
            D_800A1870 = 0;
            D_800A1874 = 0;
        }
    }
}
