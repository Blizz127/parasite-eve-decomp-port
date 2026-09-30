/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80190D3C — blob offset 0x1D4C, 0xC8 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80190D3C/REPORT.md).
 * RotMatrix-style (func_800794C4) into the record's MATRIX, copy it to *D_8019BFF0, compose with D_8019CC30, set rot/trans, then func_80197BA0(a1, r->f04). */

typedef struct {
    short m[3][3];
    int t[3];
} MATRIX;
typedef struct {
    short vx, vy, vz, pad;
} SVECTOR;
typedef struct Rec {
    short id;          /* 0x00 */
    short f02;         /* 0x02 */
    int f04;           /* 0x04 */
    MATRIX mat;        /* 0x08 */
    SVECTOR rot;       /* 0x28 */
    short f30;         /* 0x30 */
    short f32;         /* 0x32 */
    unsigned char pad34[0x28];
    short depth;       /* 0x5C */
    short f5E;
    void *parent;      /* 0x60 */
    struct Rec *next;  /* 0x64 */
    struct Rec *prev;  /* 0x68 */
} Rec;                 /* 0x6C */
extern MATRIX *D_8019BFF0;
extern MATRIX D_8019CC30;
extern void func_800794C4();
extern void func_800787D4();
extern void func_80078E94();
extern void func_80078E04();
extern void func_80197BA0();
void func_80190D3C(Rec *r, int a1)
{
    func_800794C4(&r->rot, &r->mat);
    *D_8019BFF0 = r->mat;
    func_800787D4(&D_8019CC30, D_8019BFF0, D_8019BFF0);
    func_80078E94(D_8019BFF0);
    func_80078E04(D_8019BFF0);
    func_80197BA0(a1, r->f04);
}
