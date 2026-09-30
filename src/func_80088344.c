extern int D_8009D2C0;
extern unsigned char *D_8009D2C8;
extern unsigned char D_8009D2CE;
extern short D_8009B8F8[];
extern short D_8009B9F8;
extern unsigned char D_800B8AC0[];

#define U16(o) (*(unsigned short *)(v + (o)))
#define S16(o) (*(short *)(v + (o)))
#define I32(o) (*(int *)(v + (o)))
#define U32(o) (*(unsigned int *)(v + (o)))
#define PTR(o) (*(short **)(v + (o)))

void func_80088344(unsigned char *v, unsigned int mask)
{
    int vol;
    unsigned int fl;
    short *t;
    int r;
    int pan;
    int p;
    unsigned char *w;

    vol = (S16(0x46) * (U16(0x6C) >> 8)) >> 7;
    fl = U32(0x38);
    if (fl & 1) {
        if (U16(0x8A) == 0 && --U16(0x8E) == 0) {
            U16(0x8E) = U16(0x8C);
            t = PTR(0x1C);
            if (t[0] == 0 && t[1] == 0) {
                PTR(0x1C) = t + t[2];
            }
            { int s = *PTR(0x1C)++; pan = (U16(0x92) * s) >> 16; }
            if (pan != S16(0xE8)) {
                S16(0xE8) = pan;
                U32(0xF4) |= 0x10;
                if (pan >= 0) {
                    S16(0xE8) = pan * 2;
                }
            }
        }
    }
    if (fl & 2) {
        if (U16(0x9E) == 0 && --U16(0xA2) == 0) {
            U16(0xA2) = U16(0xA0);
            t = PTR(0x20);
            if (t[0] == 0 && t[1] == 0) {
                PTR(0x20) = t + t[2];
            }
            pan = ((vol * (U16(0xA6) >> 8)) << 9) >> 16;
            pan = (pan * *PTR(0x20)++) >> 15;
            if (pan != S16(0xEA)) {
                S16(0xEA) = pan;
                U32(0xF4) |= 3;
            }
        }
    }
    if (fl & 4) {
        if (--U16(0xB0) == 0) {
            U16(0xB0) = U16(0xAE);
            t = PTR(0x24);
            if (t[0] == 0 && t[1] == 0) {
                PTR(0x24) = t + t[2];
            }
            pan = ((U16(0xB4) >> 8) * *PTR(0x24)++) >> 15;
            if (pan != S16(0xEC)) {
                S16(0xEC) = pan;
                U32(0xF4) |= 3;
            }
        }
    }
    if (fl & 0x20) {
        vol = ((short)(U16(-0x10) << 1) * (U16(0x6C) >> 8)) >> 7;
        U32(0xF4) |= 3;
    }
    if (U32(0xF4) & 3) {
        vol += S16(0xEA);
        vol = (vol * (*(unsigned short *)(D_8009D2C8 + 0x4A) & 0x7F)) >> 7;
        pan = ((U16(0x76) >> 8) + S16(0xEC)) & 0xFF;
        switch (D_8009D2C0) {
        case 1:
            S16(0x118) = (vol * D_8009B8F8[pan]) >> 15;
            S16(0x11A) = (vol * D_8009B8F8[pan ^ 0xFF]) >> 15;
            break;
        case 4:
            S16(0x118) = (vol * D_8009B8F8[pan]) >> 15;
            r = (vol * D_8009B8F8[pan ^ 0xFF]) >> 15;
            S16(0x11A) = r;
            if (mask & 0xAAAAAA) {
                S16(0x11A) = ~r;
            } else {
                S16(0x118) = ~U16(0x118);
            }
            break;
        default:
            S16(0x11A) = (unsigned int)(vol * D_8009B9F8) >> 15;
            S16(0x118) = S16(0x11A);
            break;
        }
        if (fl & 0x800) {
            w = D_800B8AC0 + U16(0x5C) * 0x11C;
            *(short *)(w + 0x118) = S16(0x118);
            *(short *)(w + 0x11A) = S16(0x11A);
            vol = U16(0x5E) >> 1;
            switch (D_8009D2C0) {
            case 1:
                S16(0x118) = (vol * D_8009B8F8[pan]) >> 15;
                S16(0x11A) = (vol * D_8009B8F8[pan ^ 0xFF]) >> 15;
                break;
            case 4:
                S16(0x118) = (vol * D_8009B8F8[pan]) >> 15;
                r = (vol * D_8009B8F8[pan ^ 0xFF]) >> 15;
                S16(0x11A) = r;
                if (mask & 0xAAAAAA) {
                    S16(0x11A) = ~r;
                } else {
                    S16(0x118) = ~U16(0x118);
                }
                break;
            default:
                S16(0x11A) = (unsigned int)(vol * D_8009B9F8) >> 15;
                S16(0x118) = S16(0x11A);
                break;
            }
            *(short *)(w + 0x118) -= S16(0x118);
            *(short *)(w + 0x11A) -= S16(0x11A);
        }
    }
    if (fl & 0x10) {
        p = U16(-0x10) + S16(0xE8) + S16(0x36);
        if ((pan = D_8009D2CE) != 0) {
            if (pan < 0x80) {
                p += (p * pan) >> 7;
            } else {
                p = (p * pan) >> 8;
            }
        }
        S16(0x10C) = p & 0x3FFF;
        U32(0xF4) |= 0x10;
    } else if (U32(0xF4) & 0x10) {
        p = I32(0x30) + S16(0xE8) + S16(0x36);
        if ((pan = D_8009D2CE) != 0) {
            if (pan < 0x80) {
                p += (p * pan) >> 7;
            } else {
                p = (p * pan) >> 8;
            }
        }
        S16(0x10C) = p & 0x3FFF;
    }
}
