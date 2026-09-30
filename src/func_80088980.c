extern int D_8009D2C0;
extern short D_8009B8F8[];
extern short D_8009B9F8;

#define U16(o) (*(unsigned short *)(v + (o)))
#define S16(o) (*(short *)(v + (o)))
#define I32(o) (*(int *)(v + (o)))
#define U32(o) (*(unsigned int *)(v + (o)))
#define U8(o) (*(unsigned char *)(v + (o)))
#define PTR(o) (*(short **)(v + (o)))

void func_80088980(unsigned char *v, unsigned int mask)
{
    int vol;
    unsigned int fl;
    short *t;
    int r;
    int pan;
    int p;
    int k;

    vol = (S16(0x46) * (U16(0x6C) >> 8)) >> 7;
    fl = U32(0x38);
    if (fl & 1) {
        if (--U16(0x8E) == 0) {
            U16(0x8E) = U16(0x8C);
            t = PTR(0x1C);
            if (t[0] == 0 && t[1] == 0) {
                PTR(0x1C) = t + t[2];
            }
            { register int s asm("$4") = *PTR(0x1C)++; r = (U16(0x92) * s) >> 16; }
            if (r != S16(0xE8)) {
                S16(0xE8) = r;
                U32(0xF4) |= 0x10;
                if (r >= 0) {
                    S16(0xE8) = r * 2;
                }
            }
        }
    }
    if (fl & 2) {
        if (--U16(0xA2) == 0) {
            U16(0xA2) = U16(0xA0);
            t = PTR(0x20);
            if (t[0] == 0 && t[1] == 0) {
                PTR(0x20) = t + t[2];
            }
            { register int s asm("$4") = *PTR(0x20)++; register int x asm("$3"); register int y asm("$2") = (vol * (U16(0xA6) >> 8)) << 9; x = y >> 16; r = (x * s) >> 15; }
            if (r != S16(0xEA)) {
                S16(0xEA) = r;
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
            { register int s asm("$4") = *PTR(0x24)++; r = ((U16(0xB4) >> 8) * s) >> 15; }
            if (r != S16(0xEC)) {
                S16(0xEC) = r;
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
        if (!(U32(0x2C) & 0x2000000)) {
            vol = (vol * (signed char)(U16(0xD8) >> 8)) >> 7;
            pan = ((U16(0x76) >> 8) + S16(0xEC)) & 0xFF;
        } else {
            pan = 0x80;
        }
        asm("" : : "r"(pan));
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
    }
    if (fl & 0x10) {
        p = U16(-0x10) + S16(0xE8) + S16(0x36);
        if (!(U32(0x2C) & 0x2000000) && (k = U8(0x3D)) != 0) {
            if (k < 0x80) {
                p += (p * k) >> 7;
            } else {
                p = (p * k) >> 8;
            }
        }
        S16(0x10C) = p & 0x3FFF;
        U32(0xF4) |= 0x10;
    } else if (U32(0xF4) & 0x10) {
        p = I32(0x30) + S16(0xE8) + S16(0x36);
        if (!(U32(0x2C) & 0x2000000) && (k = U8(0x3D)) != 0) {
            if (k < 0x80) {
                p += (p * k) >> 7;
            } else {
                p = (p * k) >> 8;
            }
        }
        S16(0x10C) = p & 0x3FFF;
    }
}
