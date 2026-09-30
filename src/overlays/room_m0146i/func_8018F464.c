/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_8018F464 — blob offset 0x47c, 0x10c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018F470; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

#define P(o, x) (*(void **)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
typedef struct {
    unsigned char pad[0x28];
    short reload;
    short count;
    short flag;
} Timer;
extern int func_800C6CE0();
extern int func_800C2B68();
extern void *D_8009D254;
void func_8018F464(void *o, unsigned char *q, Timer *t)
{
    Timer *u = t;

    if (t->count == 0) {
        if (t->flag == 1 && func_800C6CE0(o) == 3 && *(unsigned char *)P(P(P(o, 0x8), 0x0), 0x18) == 2) {
            t->flag = 0;
            t->count = t->reload;
            W(P(D_8009D254, 0x0), 0x4C) |= 0x4000;
            W(P(P(o, 0x8), 0x0), 0x0) |= 0x80000000;
        }
    } else {
        u->count--;
        u->flag = 0;
    }
    if (func_800C2B68() == 1) {
        q[1] = 2;
    }
}
