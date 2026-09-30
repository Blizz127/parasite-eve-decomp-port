/* func_80013514 - field-VM handler: turn the camera toward a target POINT.
 * Point-operand twin of func_80013C34 (same turn/step tail).
 * era -O2 -G8, D_8009D2F0 forced absolute. */
extern unsigned char *D_8009D300;
extern int D_8009CE00;
extern unsigned char *D_8009D2F0;

extern int func_80079FB4(int a0, int a1);

int func_80013514(int **a0) {
    unsigned char *st;
    unsigned short fl;
    int px;
    int pz;
    int step;
    int t;
    int cd;
    int res;
    int diff;

    st = D_8009D300;
    fl = *(unsigned short *)(st + 8);
    if ((fl & 0x20) == 0) {
        px = *a0[0];
        pz = *a0[1];
        step = *a0[2];
        *(unsigned short *)(st + 8) = fl | 0x20;
        *(int *)(st + 0x14) = px;
        *(int *)(st + 0x18) = pz;
        *(int *)(st + 0x1C) = step;
    } else {
        px = *(int *)(st + 0x14);
        pz = *(int *)(st + 0x18);
        step = *(int *)(st + 0x1C);
    }

    px = (*(int *)(D_8009D2F0 + 0x28) - px) >> 16;
    pz = (*(int *)(D_8009D2F0 + 0x30) - pz) >> 16;
    if ((px | pz) == 0) {
        return 1;
    }
    t = (0x1400 - func_80079FB4(pz, px)) & 0xFFF;
    cd = *(short *)(D_8009D2F0 + 0x3A);
    res = t;
    if (t != cd) {
        if (cd < t) {
            diff = t - cd;
            if (diff < 0x800) {
                if (step < diff) {
                    res = cd + step;
                }
            } else if (step < diff) {
                res = cd - step;
                if (res < 0) {
                    int x = cd + 0x1000;
                    if (x - t < step) {
                        res = t;
                    }
                }
            }
        } else {
            diff = cd - t;
            if (diff < 0x800) {
                if (step < diff) {
                    res = cd - step;
                }
            } else if (step < diff) {
                res = cd + step;
                if (res > 0x1000) {
                    int x = t + 0x1000;
                    if (x - cd < step) {
                        res = t;
                    }
                }
            }
        }
        res = res & 0xFFF;
        *(short *)(D_8009D2F0 + 0x3A) = res;
        if (res != t) {
            D_8009CE00 = D_8009CE00 - 0x14;
            *(int *)(D_8009D300 + 0x10) = 1;
            return 0;
        }
    }
    *(unsigned short *)(D_8009D300 + 8) = *(unsigned short *)(D_8009D300 + 8) & 0xFFDF;
    return 1;
}
