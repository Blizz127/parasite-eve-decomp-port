/* VRAM 0x800323C8 / file 0x22BC8 / size 0x214.
 *
 * Place one 28-byte sprite record (8 bytes + a SPRT at +8) of D_8009E928
 * from three 9-entry byte tables selected by a0, then add it to the OT
 * via func_80077AC4. The three initialized local arrays are copied from
 * retail's shared .rodata pool (D_80010DFC / D_80010E10 / D_80010E24):
 * profile era_o2_g0_rodata_fold_800323c8 = -O2 -G0 +
 * MASPSX_RODATA_FOLD=D_80010DFC,D_80010E10,D_80010E24 (maspsx patch 20;
 * the build strips cc1's byte-identical duplicate .rodata). Levers: the
 * SPRT typing (retail keeps &D_8009E928[0].s in $t1), a local index k
 * after the two byte stores, and an unreferenced 16-byte local
 * (`int pad[4]`) for retail's 0x70 frame -- same device as
 * func_800347B4's unused 16-byte local (lead-accepted precedent).
 */
typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short w, h;
} SPRT;
typedef struct { unsigned int pad[2]; SPRT s; } Ent; /* 28 bytes */
typedef struct { short x, y; unsigned char pad[44]; } Base48;
extern Ent D_8009E928[];
extern Base48 D_8009E360[];
extern int D_8009CDDC;
extern int D_800B0E38[];
extern void func_80077AC4(int, void *);

void func_800323C8(signed char a0) {
    unsigned char uv[18] = {0x00,0x00,0x00,0xF8,0xD4,0xF0,0x00,0x00,0x90,0xE8,0x28,0xF8,0xB4,0xEF,0x50,0xF8,0x58,0xE8};
    unsigned char wh[18] = {0x00,0x00,0x28,0x07,0x14,0x07,0x00,0x00,0x40,0x07,0x28,0x07,0x1C,0x07,0x30,0x07,0x38,0x07};
    unsigned char xy[18] = {0x00,0x00,0x14,0x08,0x1E,0x08,0x00,0x00,0x08,0x08,0x14,0x08,0x1A,0x08,0x10,0x08,0x0C,0x08};
    unsigned char *p = uv + a0 * 2;
    unsigned char *r = xy + a0 * 2;
    unsigned char *q;
    int k;
    SPRT *s;
    int pad[4];
    D_8009E928[D_8009CDDC].s.u0 = p[0];
    D_8009E928[D_8009CDDC].s.v0 = p[1];
    q = wh + a0 * 2;
    k = D_8009CDDC;
    s = &D_8009E928[k].s;
    s->w = q[0];
    s->h = q[1];
    s->x0 = D_8009E360[k].x + r[0];
    s->y0 = D_8009E360[k].y + r[1];
    func_80077AC4(D_800B0E38[k] + 16, &D_8009E928[k]);
}
