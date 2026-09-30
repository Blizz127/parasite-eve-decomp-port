/*
 * func_8006DD38 — VRAM 0x8006DD38, size 0x94, file 0x5E538-0x5E5CC.
 *
 * Project (x,y,z) via func_8006DFA8; when D_800B0CE8 is non-zero, queue the
 * index-selected sound from the D_800B0CE8 arena via func_80086608.
 * Returns func_80086608's result, or 0 when the arena is disabled.
 *
 * era -O2 -G0 + ERA_ASPSX_VER=2.30 (3-word lui/addiu/op-%lo expansion).
 * `base` is assigned after the projection call so it is not live across it
 * (the frame stays 0x30); the separate `slot` pointer is what makes cc1 put
 * the arena base in $v1 and the scaled index in $v0.
 */
extern unsigned char D_800B0CE8;

struct SVec {
    short x;
    short y;
    short z;
    int pan;
    int volume;
};

void func_8006DFA8(short *pos, int *pan, int *volume);
int func_80086608(unsigned char *sound, int key, int pan, int volume);

int func_8006DD38(int index, int key, short x, short y, short z) {
    struct SVec s;
    unsigned char *base;
    int result = 0;
    s.x = x;
    s.y = y;
    s.z = z;
    func_8006DFA8(&s.x, &s.pan, &s.volume);
    base = &D_800B0CE8;
    if (*base != 0) {
        unsigned char **slot = (unsigned char **)(base + 0x124);
        result = func_80086608(slot[index], key, s.pan, s.volume);
    }
    return result;
}
