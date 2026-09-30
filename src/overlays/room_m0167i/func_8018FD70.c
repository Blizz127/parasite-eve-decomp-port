/* room_m0167i (PE.IMG room m0167i chunk 2, VRAM 0x8018EFE8)
 * func_8018FD70 — blob offset 0xD88, 0xF8 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Update 12 drip particles (state, fade cap, fall by +0x20, age counter; done when +0x26 reaches 0). */

typedef struct { short h0, h2, h4, h6; } SV;
typedef struct {
    unsigned char pad0[0x20];
    short f20, f22, f24, f26, f28, f2A;
    signed char state[12];
    signed char age[12];
    short fade[12];
    unsigned char pad5C[4];
    SV v[12];
} Fx;
void func_8018FD70(int a0, unsigned char *q, Fx *p)
{
    unsigned int i;
    short t;

    for (i = 0; i < 12; i++) {
        if (p->state[i] != -1) {
            if (p->state[i] == 1) {
                p->fade[i] += p->f2A;
                t = p->f28;
                if (t < p->fade[i]) {
                    p->fade[i] = t;
                }
            }
            p->v[i].h0 -= p->f20;
            if (p->v[i].h0 < 10) {
                p->state[i] = 1;
            }
        }
        if (++p->age[i] == p->f22) {
            if (--p->f26 == 0) {
                q[1] = 2;
            }
        }
    }
}
