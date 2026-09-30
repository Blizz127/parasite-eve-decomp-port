typedef struct {
    int pad0[13];
    int count;
    int pad38;
    int w;
    int h;
    int cur_col;
    int cur_row;
    int flash_col;
    int flash_row;
    int cols;
    int pad58;
    int base;
    int pad60[5];
    int flags;
    int pad78[5];
    int (*enable)(int);
} Menu;

extern int *D_8009D12C;
extern int D_8009D124;
extern int D_8009D128;
extern int D_8009D10C;
extern int D_800A22B0[];
extern int D_800A2270[];
extern void func_800527C0(int);
extern int func_80073A44(int);
extern void func_80062090(int, int, int);

void func_800634D4(Menu *m, void (*draw)(int), int row, int sel)
{
    int i;
    int idx;
    unsigned int bit;
    int en;
    int hl;
    register int *t asm("$3");
    register int *r asm("$5");
    int t0;
    int t1;

    idx = m->cols * (row + m->base);
    bit = 1 << idx;
    r = D_8009D12C;
    if (r < D_800A22B0) {
        t0 = D_8009D124;
        t1 = D_8009D128;
        t = r + 2;
        D_8009D12C = t;
        r[0] = t0;
        r[1] = t1;
    } else {
        func_800527C0(2);
    }
    D_8009D124 += 2;
    D_8009D128 += 2;
    for (i = 0; i < m->count; i++) {
        en = 1;
        if (m->enable != 0) {
            en = m->enable(idx++);
            m->flags &= ~bit;
            if (en) {
                m->flags |= bit;
            }
            bit <<= 1;
        }
        hl = 0;
        if (en == 0) {
            hl = 1;
        } else if (sel != 0) {
            if (i != m->cur_col || row + m->base != m->cur_row) {
                hl = 1;
            }
        }
        D_8009D10C = hl;
        if (draw != 0) {
            draw(m->count * (m->base + row) + i);
        }
        if ((func_80073A44(-1) & 8) && i == m->flash_col && row + m->base == m->flash_row) {
            int w = m->w;
            int x = *(volatile int *)&D_8009D124;
            int y = *(volatile int *)&D_8009D128;
            int h = *(volatile int *)&m->h;

            D_8009D124 = x - 2;
            D_8009D128 = y - 2;
            func_80062090(w, h, 0);
            D_8009D124 += 2;
            D_8009D128 += 2;
        }
        {
            int w = m->w;
            int x = D_8009D124;
            int y = D_8009D128;

            D_8009D124 = x + w;
            D_8009D128 = y;
        }
    }
    t = D_8009D12C;
    if (D_800A2270 < t) {
        t0 = t[-2];
        t1 = t[-1];
        D_8009D12C = t - 2;
        D_8009D124 = t0;
        D_8009D128 = t1;
    } else {
        func_800527C0(3);
    }
    {
        int h = m->h;
        int y = D_8009D128;
        int x = D_8009D124;

        D_8009D124 = x;
        D_8009D128 = y + h;
    }
}
