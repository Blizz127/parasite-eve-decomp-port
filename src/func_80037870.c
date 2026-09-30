typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short w, h;
} Sprt;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    short w, h;
} Tile;

typedef struct {
    unsigned int tp[2];
    Sprt sp;
} SEnt;

typedef struct {
    unsigned int tp[2];
    Tile t;
} TEnt;

typedef struct {
    unsigned int b0 : 8;
    unsigned int b8 : 8;
    unsigned int b16 : 4;
    unsigned int b20 : 1;
    unsigned int b21 : 1;
    unsigned int b22 : 3;
    unsigned int b25 : 1;
    unsigned int b26 : 6;
} Flags;

typedef struct {
    unsigned char active;
    unsigned char pad1[3];
    unsigned char *f4;
    unsigned char type;
    unsigned char f9;
    unsigned char padA[2];
    Flags fl;
    short val;
    unsigned short pos[4];
    unsigned char dig[5][6];
} Pop;

typedef struct {
    unsigned short pad0[3];
    unsigned short v;
    unsigned short pad8[2];
    unsigned short tpage;
    unsigned short clut;
} Font;

typedef struct {
    unsigned char pre;
    unsigned char adv;
} Adv;

extern Pop D_800BCEA8[];
extern signed char D_8009CEA0;
extern unsigned char D_8009CEA4;
extern unsigned char D_8009CEA8;
extern unsigned char D_8009CEAC;
extern unsigned char D_8009CEB0;
extern unsigned char D_8009CEC4;
extern unsigned char D_8009CEC8;
extern unsigned char D_8009CECC;
extern unsigned char D_8009CE94;
extern int D_8009CED4;
extern unsigned char *D_8009CE90;
extern int D_8009CDDC;
extern unsigned int D_8009D1F4;
extern signed char D_8009169D;
extern unsigned char D_80091694[];
extern Adv D_800916A0[];
extern Font D_80091644[];
extern SEnt D_8009EC70[];
extern TEnt D_8009ECA8[];
extern unsigned int *D_800B0E38[];

extern void func_80077C84(void *, int, int, int);
extern void func_80077C04(Sprt *);
extern int func_80077CB4(void *, void *);
extern void func_800719E4(int) __attribute__((noreturn));
extern void func_80077AC4(void *, void *);
extern void func_8005E894(int, int);
extern void func_80061C34(int, int, int, int);

#define GLYPH(TP, CL)                                   \
    func_80077C84(p, 0, 1, (TP));                       \
    func_80077C04(&p->sp);                              \
    if (func_80077CB4(p, &p->sp)) {                     \
        func_800719E4(-1);                              \
    }                                                   \
    p->sp.x0 = xy[0];                                   \
    p->sp.y0 = xy[1];                                   \
    p->sp.u0 = uv[0];                                   \
    p->sp.v0 = uv[1];                                   \
    p->sp.r0 = D_8009CEA8;                              \
    p->sp.g0 = D_8009CEAC;                              \
    p->sp.b0 = D_8009CEB0;                              \
    p->sp.w = 12;                                       \
    p->sp.h = 12;                                       \
    p->sp.clut = (CL);                                  \
    func_80077AC4(ot, p++);

void func_80037870(void)
{
    unsigned short xy[2];
    unsigned short uv[2];
    unsigned char i;
    unsigned char ctl;
    unsigned char bg;
    unsigned short lh;
    unsigned char adv;
    SEnt *p;

    ctl = 0;
    bg = 0;
    lh = 12;
    p = (SEnt *)D_800B0E38[D_8009CDDC + 3];
    if (D_8009CED4 != 0) {
        func_80077AC4(D_800B0E38[D_8009CDDC] + 2, &D_8009ECA8[D_8009CDDC]);
    }
    for (i = 0; i < 4; i++) {
        unsigned int *ot;
        unsigned char *digits;
        unsigned char stop;
        unsigned short left;
        Pop *rec;
        unsigned char *s;
        Font *font;
        signed char n;
        unsigned char c;
        unsigned short row;
        signed char sel;
        unsigned short *pp;
        Sprt *sq;
        unsigned int w;
        unsigned char *t;

        if (D_800BCEA8[i].active == 0) {
            continue;
        }
        ot = D_800B0E38[D_8009CDDC] + 1;
        stop = 0;
        rec = &D_800BCEA8[i];
        adv = 12;
        if (rec->active == 1) {
            rec->fl.b0 = 0;
            rec->fl.b8 = 0;
            rec->fl.b16 = 0;
            rec->f4 = D_8009CE90;
            for (;;) {
                t = rec->f4;
                if (t[0] == 0xFF || t[0] == 0xF9) {
                    rec->f4 = t + 2;
                    if (t[1] == 0xFE && t[2] == rec->val) {
                        rec->f4 = t + 3;
                        break;
                    }
                }
                rec->f4++;
            }
        }
        left = 20;
        xy[1] = 174;
        xy[0] = 20;
        D_8009CEA8 = D_8009CEAC = D_8009CEB0 = 0x80;
        switch (rec->type) {
        case 3:
            xy[1] = 174;
            xy[0] = 20;
            D_8009CEA8 = D_8009CEC4;
            D_8009CEAC = D_8009CEC8;
            D_8009CEB0 = D_8009CECC;
            break;
        default:
            left = rec->pos[0];
            xy[0] = left;
            if (!rec->fl.b25) {
                xy[1] = rec->pos[1] + 6;
            } else {
                xy[1] = rec->pos[1];
            }
            if (rec->type == 1) {
                func_8005E894(rec->pos[0], rec->pos[1]);
                func_80061C34(rec->pos[2], rec->pos[3], 0, 0);
            }
            break;
        case 0:
            if (rec->f9 != 0 && bg == 0 && D_8009CED4 == 0) {
                bg = 1;
                func_80077AC4(D_800B0E38[D_8009CDDC] + 2, &D_8009ECA8[D_8009CDDC]);
            }
            break;
        }
        s = rec->f4;
        digits = rec->dig[0];
        do {
            font = D_80091644;
            switch (*s) {
            case 0xF7:
                s++;
                xy[0] = left;
                xy[1] = lh + xy[1];
                break;
            case 0xF8:
                if (rec->active == 2 && (D_8009D1F4 & 0x100)) {
                    do {
                    } while (*s++ != 0xF8);
                    rec->f4 = s;
                }
                stop = 1;
                break;
            case 0xF9:
                rec->active = 0;
                stop = 1;
                break;
            case 0xFF:
                if (rec->active == 2 && (D_8009D1F4 & 0x100) && !rec->fl.b25) {
                    rec->active = 0;
                }
                stop = 1;
                break;
            case 0xFA:
                for (n = 0; n < D_8009169D; n++) {
                    c = D_80091694[n];
                    uv[0] = (unsigned char)(c % 21) * 12;
                    uv[1] = (unsigned char)(c / 21) * 12;
                    xy[0] -= D_800916A0[c].pre;
                    GLYPH(font->tpage, font->clut)
                    xy[0] += D_800916A0[c].adv;
                }
                s++;
                break;
            case 0xFB:
                s++;
                switch (*s) {
                case 0:
                case 1:
                case 2:
                case 3:
                    uv[0] = *s * 12 + 64;
                    uv[1] = D_80091644[2].v;
                    GLYPH(D_80091644[2].tpage, D_80091644[2].clut)
                    s++;
                    xy[0] += adv;
                    break;
                case 4:
                    D_8009CEA8 = 0xFF;
                    D_8009CEAC = D_8009CEB0 = 0;
                    s++;
                    break;
                case 5:
                    D_8009CEA8 = D_8009CEAC = D_8009CEB0 = 0x80;
                    s++;
                    break;
                case 6:
                    if (D_8009D1F4 & 0x100) {
                        for (;;) {
                            w = *(unsigned int *)&rec->fl;
                            if (ctl == ((w >> 16) & 0xF)) {
                                s++;
                                *(unsigned int *)&rec->fl = (w & 0xFFF0FFFF) | (((ctl + 1) & 0xF) << 16);
                                stop = 1;
                                continue;
                            }
                            if (*s == 6) {
                                s++;
                            }
                            ctl++;
                            break;
                        }
                    } else {
                        if (ctl == rec->fl.b16) {
                            stop = 1;
                        } else {
                            s++;
                        }
                        ctl++;
                    }
                    break;
                case 7:
                    s++;
                    rec->fl.b0 = *s;
                    if (rec->fl.b8 < rec->fl.b0) {
                        if (ctl == rec->fl.b16) {
                            rec->fl.b8++;
                            stop = 1;
                        } else {
                            s++;
                        }
                        ctl++;
                    } else {
                        for (;;) {
                            if (ctl == rec->fl.b16) {
                                rec->fl.b8 = 0;
                                rec->fl.b0 = *s;
                                s++;
                                rec->fl.b16++;
                                continue;
                            }
                            if (rec->fl.b8 != 0) {
                                s++;
                            }
                            ctl++;
                            break;
                        }
                    }
                    break;
                case 8:
                    for (n = digits[5]; n > 0; n--) {
                        uv[0] = *(digits + n - 1) * 12;
                        uv[1] = 0;
                        GLYPH(font->tpage, font->clut)
                        xy[0] += adv;
                    }
                    s++;
                    digits += 6;
                    break;
                case 9:
                    pp = xy;
                    s++;
                    rec->fl.b21 = 1;
                    rec->fl.b22 = *s & 7;
                    sel = rec->fl.b22;
                    if (D_8009D1F4 & 0x20) {
                        if (++D_8009CEA0 >= sel) {
                            D_8009CEA0 = sel - 1;
                        }
                    }
                    if (D_8009D1F4 & 8) {
                        if (--D_8009CEA0 < 0) {
                            D_8009CEA0 = 0;
                        }
                    }
                    if (D_8009D1F4 & 0x100) {
                        D_8009CEA4 = D_8009CEA0;
                    }
                    s++;
                    sq = &D_8009EC70[D_8009CDDC].sp;
                    sq->x0 = pp[0];
                    sq->y0 = pp[1] + D_8009CEA0 * 12;
                    func_80077AC4(D_800B0E38[D_8009CDDC] + 1, &D_8009EC70[D_8009CDDC]);
                    break;
                default:
                    uv[0] = ((*s + 0xED) % 21) * 12;
                    row = ((*s + 0xED) / 21) * 12;
                    uv[1] = row;
                    if (row >= 241) {
                        uv[1] = row - 252;
                        font++;
                    }
                    GLYPH(font->tpage, font->clut)
                    s++;
                    xy[0] += adv;
                    break;
                }
                break;
            case 0xFC:
                s++;
                uv[0] = ((*s + 0x34) % 21) * 12;
                uv[1] = ((*s + 0x34) / 21) * 12;
                GLYPH(D_80091644[1].tpage, D_80091644[1].clut)
                s++;
                xy[0] += adv;
                break;
            case 0xFD:
                s++;
                uv[0] = ((*s + 0x134) % 21) * 12;
                uv[1] = ((*s + 0x134) / 21) * 12;
                GLYPH(D_80091644[1].tpage, D_80091644[1].clut)
                s++;
                xy[0] += adv;
                break;
            case 0x0F:
                if (D_8009CE94 == 1) {
                    xy[0] -= 7;
                }
            default:
                uv[0] = (unsigned char)(*s % 21) * 12;
                uv[1] = (unsigned char)(*s / 21) * 12;
                xy[0] -= D_800916A0[*s].pre;
                GLYPH(font->tpage, font->clut)
                xy[0] += D_800916A0[*s].adv;
                s++;
                break;
            }
        } while (!stop);
        if (rec->active == 1) {
            rec->active = 2;
        }
    }
}
