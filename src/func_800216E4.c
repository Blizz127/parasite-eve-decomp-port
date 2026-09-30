typedef struct {
    void *f0;
    int f4;
    short f8;
    short padA;
} Entry;

extern Entry D_8009E000[];

extern void func_80021850(Entry *a, signed char i, signed char j);

void func_800216E4(Entry *a, signed char lo, signed char hi) {
    signed char i;
    signed char last;

    if (lo >= hi) {
        return;
    }
    func_80021850(a, lo, (lo + hi) / 2);
    last = lo;
    for (i = lo; i <= hi; i++) {
        if (D_8009E000[i].f4 < D_8009E000[lo].f4) {
            func_80021850(a, ++last, i);
        }
    }
    func_80021850(a, lo, last);
    func_800216E4(a, lo, last - 1);
    func_800216E4(a, last + 1, hi);
}
