/* func_80087AA8 -- Akao per-voice slide / LFO-depth tick (0x80087AA8, 0x4F8).
 * Evidence: docs/evidence/func-80087AA8/REPORT.md
 * era cc1 2.7.2 -O2 -G0 (default profile era_o2_g0), LINK_EXACT.
 */
typedef struct {
    unsigned char pad00[0x1C];
    short * vib_wave; /* 0x1C */
    short * trem_wave; /* 0x20 */
    short * pan_wave; /* 0x24 */
    unsigned char pad28[0x8];
    unsigned int pitch_base; /* 0x30 */
    int pitch; /* 0x34 */
    unsigned int mode; /* 0x38 */
    unsigned char pad3C[0x8];
    int volw; /* 0x44 */
    int volw_step; /* 0x48 */
    int pitch_step; /* 0x4C */
    unsigned char pad50[0xE];
    unsigned short f5E; /* 0x5E */
    unsigned short c60; /* 0x60 */
    unsigned char pad62[0xA];
    unsigned short vol_scale; /* 0x6C */
    unsigned short c6E; /* 0x6E */
    unsigned char pad70[0x2];
    unsigned short c72; /* 0x72 */
    unsigned short c74; /* 0x74 */
    unsigned short pan; /* 0x76 */
    unsigned short c78; /* 0x78 */
    unsigned short c7A; /* 0x7A */
    unsigned char pad7C[0xE];
    unsigned short c8A; /* 0x8A */
    unsigned char pad8C[0x2];
    unsigned short vib_cnt; /* 0x8E */
    unsigned char pad90[0x2];
    unsigned short vib_depth; /* 0x92 */
    unsigned short vdep; /* 0x94 */
    unsigned short c96; /* 0x96 */
    unsigned short vdep_step; /* 0x98 */
    unsigned char pad9A[0x4];
    unsigned short c9E; /* 0x9E */
    unsigned char padA0[0x2];
    unsigned short trem_cnt; /* 0xA2 */
    unsigned char padA4[0x2];
    unsigned short trem_depth; /* 0xA6 */
    unsigned short cA8; /* 0xA8 */
    unsigned short tdep_step; /* 0xAA */
    unsigned char padAC[0x4];
    unsigned short pan_cnt; /* 0xB0 */
    unsigned char padB2[0x2];
    unsigned short pan_depth; /* 0xB4 */
    unsigned short cB6; /* 0xB6 */
    unsigned short pdep_step; /* 0xB8 */
    unsigned short cBA; /* 0xBA */
    unsigned short cBC; /* 0xBC */
    unsigned char padBE[0x16];
    short vol_step; /* 0xD4 */
    unsigned short fD6; /* 0xD6 */
    short expr; /* 0xD8 */
    short expr_step; /* 0xDA */
    short pan_step; /* 0xDC */
    unsigned char padDE[0xA];
    short vib_out; /* 0xE8 */
    short trem_out; /* 0xEA */
    short pan_out; /* 0xEC */
    unsigned char padEE[0x6];
    unsigned int dirty; /* 0xF4 */
} Voice;

typedef struct {
    unsigned char pad00[0x34];
    volatile unsigned int f34;
    unsigned char pad38[4];
    unsigned int f3C;
} Ctl;

extern Ctl *D_8009D2C8;
extern volatile unsigned int D_8009D2C4;
extern void func_80089960();
extern void func_80089CF0();

void func_80087AA8(Voice *v, unsigned int mask) {
    int n;
    unsigned int d;
    short *p;

    if (v->c72) {
        v->c72--;
        n = v->volw + v->volw_step;
        if ((n & 0xFFE00000) != (v->volw & 0xFFE00000)) {
            v->dirty |= 3;
        }
        v->volw = n;
    }
    if (v->c60) {
        v->c60--;
        v->f5E += v->fD6;
        v->dirty |= 3;
    }
    if (v->c6E) {
        v->c6E--;
        n = v->vol_scale + v->vol_step;
        if ((n & 0x7F00) != (v->vol_scale & 0x7F00)) {
            v->dirty |= 3;
        }
        v->vol_scale = n;
    }
    if (v->c74) {
        v->c74--;
        n = v->expr + v->expr_step;
        if ((v->mode & 0x100) && (n & 0xFF00) != (v->expr & 0xFF00)) {
            v->dirty |= 3;
        }
        v->expr = n;
    }
    if (v->c78) {
        v->c78--;
        n = v->pan + v->pan_step;
        if ((n & 0xFF00) != (v->pan & 0xFF00)) {
            v->dirty |= 3;
        }
        v->pan = n;
    }
    if (v->c8A) {
        v->c8A--;
    }
    if (v->c9E) {
        v->c9E--;
    }
    if (v->cBA) {
        if (--v->cBA == 0) {
            D_8009D2C8->f34 ^= mask;
            D_8009D2C4 |= 0x10;
            func_80089960();
        }
    }
    if (v->cBC) {
        if (--v->cBC == 0) {
            D_8009D2C8->f3C ^= mask;
            func_80089CF0();
        }
    }
    if (v->c96) {
        unsigned int b;
        v->c96--;
        v->vdep += v->vdep_step;
        d = (v->vdep & 0x7F00) >> 8;
        if (v->vdep & 0x8000) {
            b = (d * v->pitch_base) >> 7;
        } else {
            b = (d * ((v->pitch_base * 15) >> 8)) >> 7;
        }
        v->vib_depth = b;
        if (v->c8A == 0 && v->vib_cnt != 1) {
            p = v->vib_wave;
            if (p[0] == 0 && p[1] == 0) {
                p += p[2];
            }
            n = (v->vib_depth * *p) >> 16;
            if (n != v->vib_out) {
                v->vib_out = n;
                v->dirty |= 0x10;
                if (n >= 0) {
                    v->vib_out = n << 1;
                }
            }
        }
    }
    if (v->cA8) {
        v->cA8--;
        v->trem_depth += v->tdep_step;
        if (v->c9E == 0 && v->trem_cnt != 1) {
            p = v->trem_wave;
            if (p[0] == 0 && p[1] == 0) {
                p += p[2];
            }
            n = (short)((((((short *)v)[0x23] * (v->vol_scale >> 8)) >> 7) * (v->trem_depth >> 8)) >> 7);
            n = (n * *p) >> 15;
            if (n != v->trem_out) {
                v->trem_out = n;
                v->dirty |= 3;
            }
        }
    }
    if (v->cB6) {
        v->cB6--;
        v->pan_depth += v->pdep_step;
        if (v->pan_cnt != 1) {
            p = v->pan_wave;
            if (p[0] == 0 && p[1] == 0) {
                p += p[2];
            }
            n = ((v->pan_depth >> 8) * *p) >> 15;
            if (n != v->pan_out) {
                v->pan_out = n;
                v->dirty |= 3;
            }
        }
    }
    if (v->c7A) {
        v->c7A--;
        n = v->pitch + v->pitch_step;
        if ((n & 0xFFFF0000) != (v->pitch & 0xFFFF0000)) {
            v->dirty |= 0x10;
        }
        v->pitch = n;
    }
}
