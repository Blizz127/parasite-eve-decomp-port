typedef struct {
    unsigned char pad0[0x26];
    unsigned short n;
} Obj;

extern int D_8009CDB8;
extern int D_8009CDBC;
extern int D_8009CDC0;
extern int D_8009CDC4;

#define EX(e) ((e) >> 22)
#define EY(e) (((e) >> 12) & 0x3FF)

int func_80068710(Obj *o, unsigned int *it, unsigned char dir, unsigned char wrap)
{
    int i;
    int bx;
    int best;
    int step;
    unsigned char found;
    unsigned char more;
    int x;
    int by;
    int prev;
    int sel;

    bx = 0x7FFF;
    best = 0;
    sel = 0;
    if (wrap) {
        by = 0x7FFF;
        if (!dir) {
            by = -1;
            bx = -1;
            for (i = 0; i < o->n; i++) {
                if (by < (int)EY(it[i])) {
                    by = EY(it[i]);
                }
            }
            for (i = 0; i < o->n; i++) {
                if (EY(it[i]) == by) {
                    if (bx < (int)EX(it[i])) {
                        bx = EX(it[i]);
                        sel = i;
                    }
                }
            }
            prev = D_8009CDBC;
            D_8009CDBC = sel;
            D_8009CDB8 = prev;
            D_8009CDC0 = EX(it[sel]);
            D_8009CDC4 = EY(it[sel]);
            return sel;
        } else {
            for (i = 0; i < o->n; i++) {
                if ((int)EY(it[i]) < by) {
                    by = EY(it[i]);
                }
            }
            for (i = 0; i < o->n; i++) {
                if (EY(it[i]) == by) {
                    if ((int)EX(it[i]) < bx) {
                        bx = EX(it[i]);
                        best = i;
                    }
                }
            }
        }
        prev = D_8009CDBC;
        D_8009CDBC = best;
        D_8009CDB8 = prev;
        D_8009CDC0 = EX(it[best]);
        D_8009CDC4 = EY(it[best]);
        return best;
    }
    step = 16;
    if (!dir) {
        step = -16;
    }
    found = 0;
    more = 0;
    for (i = 0; i < o->n; i++) {
        x = EX(it[i]);
        if (x == D_8009CDC0 + step) {
            if (EY(it[i]) == D_8009CDC4) {
                prev = D_8009CDBC;
                D_8009CDBC = i;
                D_8009CDB8 = prev;
                D_8009CDC0 = EX(it[i]);
                D_8009CDC4 = EY(it[i]);
                return i;
            }
        } else if (EY(it[i]) == D_8009CDC4 && D_8009CDC0 < x) {
            more = 1;
        }
    }
    if (!found && more == 1) {
        if (!dir) {
            bx = 0;
            for (i = 0; i < o->n; i++) {
                if (EY(it[i]) == D_8009CDC4 && (int)EX(it[i]) < D_8009CDC0 && bx < (int)EX(it[i])) {
                    bx = EX(it[i]);
                    best = i;
                    found = 1;
                }
            }
        } else {
            bx = 0x7FFF;
            for (i = 0; i < o->n; i++) {
                if (EY(it[i]) == D_8009CDC4 && D_8009CDC0 < (int)EX(it[i]) && (int)EX(it[i]) < bx) {
                    bx = EX(it[i]);
                    best = i;
                    found = 1;
                }
            }
        }
    }
    if (found == 1) {
        goto done;
    }
    found = 0;
    if (!dir) {
        bx = -1;
        for (i = 0; i < o->n; i++) {
            if (EY(it[i]) == D_8009CDC4 + step && bx < (int)EX(it[i])) {
                bx = EX(it[i]);
                best = i;
                found = 1;
            }
        }
    } else {
        bx = 0x7FFF;
        for (i = 0; i < o->n; i++) {
            if (EY(it[i]) == D_8009CDC4 + step && (int)EX(it[i]) < bx) {
                bx = EX(it[i]);
                best = i;
                found = 1;
            }
        }
    }
    if (!found) {
        return func_80068710(o, it, dir, 1);
    }
done:
    prev = D_8009CDBC;
    D_8009CDBC = best;
    D_8009CDB8 = prev;
    D_8009CDC0 = EX(it[best]);
    D_8009CDC4 = EY(it[best]);
    return best;
}
