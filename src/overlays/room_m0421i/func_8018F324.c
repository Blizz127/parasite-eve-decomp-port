/* room_m0421i (PE.IMG room m0421i chunk 2, VRAM 0x8018EFE8)
 * func_8018F324 — blob offset 0x33c, 0x80 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0111i func_8018F2D0; C re-targeted by symbol address
 * (docs/evidence/room_m0421i-ports-2026-09-23/REPORT.md). */

typedef struct {
    unsigned char pad[0x28];
    short reload;
    short count;
    short flag;
} Timer;
extern void func_800C6C18();
extern int func_800C2B68();
void func_8018F324(int a0, unsigned char *a1, Timer *t)
{
    Timer *u = t;

    if (t->count) {
        t->count--;
    }
    if (u->flag == 1) {
        u->flag = 0;
        if (u->count == 0) {
            u->count = u->reload;
            func_800C6C18(a0, u);
        }
    }
    if (func_800C2B68() == 1) {
        a1[1] = 2;
    }
}
