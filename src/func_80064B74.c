/*
 * func_80064B74 — apply D_800A3060[idx] (4-byte signed rows) to an object:
 * record idx at +0x70 (stored before the null check, as retail's beqz slot
 * shows), set +0x44 from row[0]; when the gp flag D_8009D16C or bit 5 of +0x64
 * is set, clamp row[1] into +0x48 below +0x58, clear +0x44 on the last-frame
 * loop condition, and set +0x5C from row[2].
 *
 * ROM: era gcc-2.7.2-psx -O2 -G8 (D_8009D16C is 0x3FC($gp)).
 */
typedef struct {
    char pad0[0x44];
    int f44;
    int f48;
    char pad4C[0x58 - 0x4C];
    int f58;
    int f5C;
    char pad60[4];
    int f64;
    int f68;
    char pad6C[4];
    int f70;
} Obj;
extern signed char D_800A3060[][4];
extern int D_8009D16C;
void func_80064B74(Obj *o, int idx) {
    signed char *e;
    o->f70 = idx;
    if (o == 0 || idx < 0) return;
    e = D_800A3060[idx];
    o->f44 = e[0];
    if (D_8009D16C == 0 && !(o->f64 & 0x20)) return;
    o->f48 = e[1];
    if (o->f48 >= o->f58) o->f48 = o->f58 - 1;
    if (o->f68 != 0 && o->f44 == 1 && o->f48 == o->f58 - 1) o->f44 = 0;
    o->f5C = e[2];
}
