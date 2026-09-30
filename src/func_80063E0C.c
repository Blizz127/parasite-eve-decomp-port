/*
 * func_80063E0C — menu cursor/input handler (VRAM 0x80063E0C, file 0x5460C,
 * 0x9C4). Up/down with scroll clamp and scroll animation, left/right with
 * hand-off to the linked left/right panes (D_8009D15C = focus), confirm via
 * the +0x84 callback (own/left/right selections), cancel restores the saved
 * selection, page up/down through func_800650E0. era gcc-2.7.2 -O2 -G8.
 * The zero-code asm("") statements and the out2 label only stop cc1's
 * cross-jumping where retail kept separate copies; the register pins mirror
 * retail's temporaries.
 */
typedef struct M {
    int pad0[9];
    int id;
    int pad28[3];
    int cols;
    int vis;
    int pad3C;
    int f40;
    int col;
    int row;
    int scol;
    int srow;
    int ncol;
    int nrow;
    int top;
    int anim;
    unsigned int flags;
    int f68;
    int pad6C[3];
    struct M *left;
    struct M *right;
    void *f80;
    int (*cb)(int, int, int, int);
    void (*cb2)(void);
} M;

extern int D_8009D0E8;
extern M *D_8009D15C;
extern int func_8005E038(void);
extern void func_8005267C(void);
extern void func_800525EC(void);
extern void func_800526C4(void);
extern void func_80052634(void);
extern void func_800650E0(void *, int);

static inline int pad(void)
{
    return D_8009D0E8 != 0 ? func_8005E038() : 0;
}

static inline void set_top(M *m, int t)
{
    m->top = t;
    if (t < 0) {
        m->top = 0;
    } else if (m->nrow - m->vis < t) {
        m->top = m->nrow - m->vis;
    }
}

static inline void show_row(M *m)
{
    int row = m->row;

    if (row < m->top) {
        m->top = row;
    } else if (!(row < m->top + m->vis)) {
        m->top = row - m->vis + 1;
    }
}

static inline int idx(M *m)
{
    int r = -1;

    if (m != 0 && m->col >= 0 && m->row >= 0) {
        r = m->cols * m->row + m->col;
    }
    return r;
}

static inline int sel(M *m)
{
    if (m != 0 && m->scol >= 0 && m->srow >= 0) {
        return m->cols * m->srow + m->scol;
    }
    return -1;
}

static inline int pick(M *m, M *o)
{
    int f = 0;

    if (o != 0 && o->scol >= 0) {
        int a = idx(m);
        int b = sel(o);

        if (m->id != o->id || a != b) {
            if (m->cb(m->id, a, o->id, b) != 0) {
                o->scol = -1;
                func_800525EC();
                f = 1;
            } else {
                func_800526C4();
                f = 1;
            }
        }
    }
    return f;
}

static inline int restore(M *m, M *o)
{
    int f = 0;

    if (o != 0 && o->scol >= 0) {
        o->col = o->scol;
        o->scol = -1;
        o->row = o->srow;
        m->col = -1;
        show_row(o);
        D_8009D15C = o;
        f = 1;
    }
    return f;
}

int func_80063E0C(M *m, int key)
{
    int r;
    int t;
    register int u asm("$3");

    r = 0;
    if (pad() & 0x20) {
        goto one;
    }
    if (key & 0x1000) {
        if (m->row > 0) {
            m->row--;
            func_8005267C();
            r = 1;
        } else if (m->flags & 0x10) {
            r = 1;
            m->row = m->nrow - 1;
            func_8005267C();
        }
        if (m->row < m->top && m->anim == 0) {
            t = m->top;
            set_top(m, t - 1);
            if (t != m->top) {
                m->anim = -m->f40 / 2;
            }
        }
        r |= ((m->flags >> 2) ^ 1) & 1;
        goto out;
    }
    if (key & 0x4000) {
        int k1;

        key = 0;
        if (m->col != 0) {
            key = m->f68 != 0;
        }
        k1 = key + 1;
        if (m->row < m->nrow - k1) {
            m->row++;
            func_8005267C();
            r = 1;
        } else if (m->flags & 0x10) {
            m->row = 0;
            func_8005267C();
            r = 1;
        }
        {
            int a = m->top;
            int b = a + m->vis;
            int e = 0;

            if (key) {
                e = m->row == m->nrow - 2;
            }
            if (!(m->row < b - e) && m->anim == 0) {
                set_top(m, a + 1);
                if (a != m->top) {
                    m->anim = m->f40 / 2;
                }
            }
        }
        r |= ((m->flags >> 3) ^ 1) & 1;
        goto out;
    }
    if (key & 0x8000) {
        if (m->col > 0) {
            int v;

            if (m->col == 6) {
                v = 4;
            } else {
                v = m->col - 1;
            }
            m->col = v;
            func_8005267C();
            r = 1;
        } else {
            M *o = m->left;

            if (o != 0) {
                int lim;
                int v;
                int e;

                m->col = -1;
                lim = o->nrow - 1;
                o->col = o->cols - 1;
                v = m->row - m->top + o->top;
                if (v < lim) {
                    lim = v;
                }
                o->row = lim;
                e = 0;
                if (o->f68 != 0) {
                    e = lim == o->nrow - 1;
                }
                r = 1;
                D_8009D15C = o;
                {
                    int e1 = e + 1;

                    o->col = o->cols - e1;
                }
                func_8005267C();
            }
        }
        u = 0;
        if (D_8009D0E8 != 0) {
            register int w asm("$2") = func_8005E038() & 0x5000;

            u = w != 0;
        }
        if (!(m->flags & 1)) {
            goto set1;
        }
        r |= u;
        goto out2;
    }
    if (key & 0x2000) {
        int c = m->col;

        if (c >= 0) {
            int e = 0;
            int e1;

            if (m->row == m->nrow - 1) {
                e = m->f68 != 0;
            }
            e1 = e + 1;
            if (c < m->ncol - e1) {
                int v;

                if (c == 4) {
                    v = 6;
                } else {
                    v = c + 1;
                }
                m->col = v;
            } else {
                M *o = m->right;
                int lim;
                int v;

                if (o == 0) {
                    goto right_done;
                }
                m->col = -1;
                lim = o->nrow - 1;
                o->col = 0;
                v = m->row - m->top + o->top;
                if (v < lim) {
                    lim = v;
                }
                o->row = lim;
                D_8009D15C = o;
            }
            func_8005267C();
            r = 1;
        }
    right_done:
        u = 0;
        if (D_8009D0E8 != 0) {
            register int w asm("$2") = func_8005E038() & 0x5000;

            u = w != 0;
        }
        if (m->flags & 2) {
            goto rtail;
        }
    set1:
        r |= 1;
        goto out;
    }
    if (key & 0x10000) {
        int f;

        if (m->cb == 0) {
            return r;
        }
        if (m->scol >= 0) {
            int a = idx(m);
            int b = sel(m);

            if (a != b) {
                if (m->cb(m->id, a, m->id, b) != 0) {
                    m->scol = -1;
                    func_800525EC();
                    r = 1;
                } else {
                    func_800526C4();
                    r = 1;
                }
            }
        }
        r |= pick(m, m->left);
        f = pick(m, m->right);
        r |= f;
        goto out;
    }
    if (key & 0x40) {
        if (m->scol >= 0) {
            m->col = m->scol;
            m->row = m->srow;
            m->scol = -1;
            show_row(m);
            if (m->cb2 != 0) {
                m->cb2();
            }
            func_80052634();
            r = 1;
        }
        r |= restore(m, m->left);
        u = restore(m, m->right);
    rtail:
        r |= u;
        goto out;
    }
    if (m->f80 != 0 && m == D_8009D15C) {
        if (key & 4) {
            int v = m->row - m->vis;

            if (v < 0) {
                v = 0;
            }
            m->row = v;
            func_800650E0(m->f80, 0x1000);
            return r;
        }
        if (key & 8) {
            int e = 0;
            int e1;
            register int lim asm("$2");
            register int v asm("$4");

            if (m->col == 1) {
                e = m->f68 != 0;
            }
            e1 = e + 1;
            lim = m->nrow - e1;
            v = m->row + m->vis;
            if (lim < v) {
                e = 0;
                if (m->col == 1) {
                    e = m->f68 != 0;
                }
                e1 = e + 1;
                lim = m->nrow - e1;
            } else {
                lim = v;
            }
            m->row = lim;
            func_800650E0(m->f80, 0x4000);
            return r;
        }
        return r;
    }
    if (!(key & 0x20)) {
        goto out;
    }
one:
    r = 1;
out2:
    asm("");
out:
    return r;
}
