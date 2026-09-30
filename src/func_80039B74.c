/* VRAM 0x80039B74 / file 0x2A374 / size 0x1B0. */
typedef struct {
    short vx, vy, vz, pad;
} SVec;

typedef struct {
    short x, y, z;
    unsigned char idx;
    unsigned char flags;
} Ent;

typedef struct {
    unsigned char pad0[0x58];
    unsigned char *mats;
    unsigned char pad5C[0x18];
    short h74;
    short h76;
    short h78;
    short pad7A;
    short h7C;
    unsigned char pad7E[0x22];
    Ent ent[2];
} Obj;

extern void func_80039ED4(Obj *, unsigned char *, short);
extern void func_80039D24(Obj *, unsigned char *, short);
extern void func_80079754(SVec *, unsigned char *);

void func_80039B74(Obj *obj, unsigned char *src, int arg) {
    SVec *sv;
    int i;
    int idx;

    sv = (SVec *)0x1F800000;
    if (src == 0) {
        return;
    }
    obj->h7C = *(short *)(src + 0xA);
    obj->h74 = *(short *)(src + 0x4);
    obj->h76 = *(short *)(src + 0x6);
    obj->h78 = *(short *)(src + 0x8);
    if ((src[0] & 3) == 2) {
        func_80039ED4(obj, src, arg);
    } else {
        func_80039D24(obj, src, arg);
    }
    for (i = 0; i < 2; i++) {
        idx = obj->ent[i].idx;
        if (idx > 0) {
            if (obj->ent[i].flags & 0x8) {
                sv[idx].vx = obj->ent[i].x;
            } else if (obj->ent[i].flags & 0x1) {
                sv[idx].vx += obj->ent[i].x;
            }
            if (obj->ent[i].flags & 0x10) {
                sv[idx].vy = obj->ent[i].y;
            } else if (obj->ent[i].flags & 0x2) {
                sv[idx].vy += obj->ent[i].y;
            }
            if (obj->ent[i].flags & 0x20) {
                sv[idx].vz = obj->ent[i].z;
            } else if (obj->ent[i].flags & 0x4) {
                sv[idx].vz += obj->ent[i].z;
            }
            func_80079754((SVec *)((unsigned int)sv | (idx << 3)), obj->mats + idx * 32);
        }
    }
}
