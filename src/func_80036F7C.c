/* VRAM 0x80036F7C / file 0x2777C / size 0x110. */
typedef struct {
    unsigned int flags;
    unsigned int cur;
    unsigned int target;
} Ramp;

extern unsigned int D_8009D1A0;
extern Ramp D_800A76A0[4];
extern signed char func_8006EC08(void);

void func_80036F7C(void) {
    unsigned int i;
    unsigned int f;

    if (D_8009D1A0 & 0x41) {
        return;
    }
    if (func_8006EC08() != 0) {
        return;
    }
    for (i = 0; i < 4; i++) {
        f = D_800A76A0[i].flags;
        if (f & 1) {
            if (!(f & 4)) {
                if (f & 2) {
                    if (f & 5) {
                        (D_800A76A0 + i)->cur = D_800A76A0[i].cur - 1;
                    } else if (D_800A76A0[i].cur > D_800A76A0[i].target) {
                        (D_800A76A0 + i)->cur = D_800A76A0[i].cur - 1;
                    } else {
                        D_800A76A0[i].flags = f | 4;
                    }
                } else {
                    if (f & 5) {
                        (D_800A76A0 + i)->cur = D_800A76A0[i].cur + 1;
                    } else if (D_800A76A0[i].cur < D_800A76A0[i].target) {
                        (D_800A76A0 + i)->cur = D_800A76A0[i].cur + 1;
                    } else {
                        D_800A76A0[i].flags = f | 4;
                    }
                }
            }
        }
    }
}
