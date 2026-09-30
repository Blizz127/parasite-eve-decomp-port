typedef struct Ev {
    struct Ev *next;
    int type;
    int data;
} Ev;

typedef struct Win {
    int f0;
    struct Win *next;
    unsigned char pad8[0x20];
    int f28;
    int (*handler)(struct Win *, int);
} Win;

extern Ev *D_8009D0DC;
extern Ev *D_8009D0E0;
extern Ev *D_8009D0E4;
extern int D_8009D0EC;
extern Win *func_80062CC4(void);
extern void func_8005E12C(int);

static inline void pop_event(Ev *out)
{
    Ev *e;
    Ev *prev;

    if (D_8009D0E0 != 0) {
        e = D_8009D0E0;
        prev = 0;
        while (e->type == 0) {
            prev = e;
            e = e->next;
            if (e == 0) {
                break;
            }
        }
        if (e != 0) {
            if (prev != 0) {
                prev->next = e->next;
            } else {
                D_8009D0E0 = e->next;
            }
            if (e == D_8009D0E4) {
                D_8009D0E4 = prev;
            }
            e->next = D_8009D0DC;
            D_8009D0DC = e;
            *out = *e;
        } else {
            out->type = 0;
            out->data = 0;
        }
    } else {
        out->type = 0;
        out->data = 0;
    }
}

void func_8005E30C(void)
{
    Win *w;
    Ev ev;
    int a;

    w = func_80062CC4();
    if (D_8009D0EC == 0) {
        func_8005E12C(w->f28);
    }
    pop_event(&ev);
    switch (ev.type) {
    case 1:
    case 2:
        for (; w != 0; w = w->next) {
            a = ev.data;
            if (ev.type == 2) {
                a |= 0x20000;
            }
            if (w->handler(w, a) != 0) {
                break;
            }
        }
        break;
    case 4:
        if (ev.data & 0x20) {
            for (w = func_80062CC4(); w != 0; w = w->next) {
                if (w->handler(w, 0x10000) != 0) {
                    break;
                }
            }
        }
        break;
    }
}
