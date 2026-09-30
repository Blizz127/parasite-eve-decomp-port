/* room_m0167i (PE.IMG room m0167i chunk 2, VRAM 0x8018EFE8)
 * func_8018F4F8 — blob offset 0x510, 0x80 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Countdown timer: decrement count; when flag == 1 clear it and, at count 0, reload and call func_800C6C18; flag +1 = 2 when func_800C2B68() == 1. */

typedef struct {
    unsigned char pad[0x28];
    short reload;
    short count;
    short flag;
} Timer;
extern void func_800C6C18();
extern int func_800C2B68();
void func_8018F4F8(int a0, unsigned char *a1, Timer *t)
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
