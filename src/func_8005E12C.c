typedef struct Ev {
    struct Ev *next;
    int type;
    int data;
} Ev;

extern Ev *D_8009D0DC;
extern Ev *D_8009D0E0;
extern Ev *D_8009D0E4;
extern int D_8009D0E8;
extern int D_8009D0F0;
extern int D_8009D0F4;
extern int D_8009D0F8;
extern int func_8005E038(void);
extern void func_800527C0(int);

static inline void push_event(int type, int data)
{
    Ev *e = D_8009D0DC;

    if (e != 0) {
        D_8009D0DC = e->next;
        e->next = 0;
        if (D_8009D0E4 != 0) {
            D_8009D0E4->next = e;
        } else {
            if (D_8009D0E0 != 0) {
                func_800527C0(31);
            }
            D_8009D0E0 = e;
        }
        D_8009D0E4 = e;
        e->type = type;
        e->data = data;
    }
}

void func_8005E12C(int a0)
{
    int rep;
    int btn;
    int bits;
    int t;

    rep = 0;
    btn = func_8005E038();
    if (D_8009D0E8 != 0) {
        bits = ~btn & D_8009D0F0;
        if (bits != 0) {
            push_event(4, bits);
        }
        if (D_8009D0F0 != btn) {
            D_8009D0F8 = 16;
        }
        t = D_8009D0F8 - 2;
        D_8009D0F8 = t;
        if (t < 0) {
            if ((a0 != 0 && t < -90) || (t & 3) == 0) {
                D_8009D0F0 = 0;
                rep = 1;
            }
        }
        D_8009D0F4 = (D_8009D0F8 >= -299) ? 1 : 8;
        bits = btn & ~D_8009D0F0;
        if (bits != 0) {
            if (!rep || !(bits & 0x40)) {
                push_event(rep ? 2 : 1, bits);
            }
        }
        D_8009D0F0 = btn;
    } else if (btn == 0) {
        D_8009D0E8 = 1;
    }
}
