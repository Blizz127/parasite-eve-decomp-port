/* ovl_03D2 (PE.IMG subsystem overlay, VRAM 0x8018EFF0)
 * func_80192CE8 — blob offset 0x3CF8, 0x2B0 bytes. Profile era_o2_g0 (default).
 * Movie playback driver: flags D_800B0CD8 bit 9, arms slot D_801D0E04[slot], retries the CD
 * read (func_8006E6A8 / func_8006E7E8) until it starts, sets up the buffers (func_80191FB8) and
 * the sprite (func_801924F8), then pumps frames until the movie ends or is skipped
 * (D_8009D26C & 0x20000004); returns 1 when skipped before frame 1400.
 * Levers (docs/evidence/ovl6-lane-2026-09-27/REPORT.md): the frame loop is a goto loop (a real
 * do/while lets loop.c hoist &D_800B0DBA into $s0); plain unsigned char D_800B0DBA with the
 * frame counter reached as (short *)(p + 2); `x = x - 1` (not x--); separate pointer locals for
 * the two D_800B0CD8 read-modify-writes. */
typedef struct { unsigned char on; unsigned char pad[19]; } Slot;
extern unsigned char D_800B0DBA;
extern unsigned int D_800B0CD8;
extern Slot D_801D0E04[];
extern unsigned short D_8009315E[];
extern int D_800B0DD8;
extern int D_8001160C;
extern int D_80011610;
extern unsigned int D_8009D26C;
extern int D_801D0DE8;
extern int D_801D0DEC;
extern int D_801D0DF0;
extern int D_801D0DF4;
extern int D_801D0DF8;
extern int D_801D0DFC;
extern void func_80074D28();
extern void func_80074DC0();
extern void func_80074A44();
extern int func_8006E6A8();
extern int func_8006E7E8();
extern void func_80072714();
extern void func_800726C4();
extern void func_80072724();
extern int func_80191FB8();
extern void func_801924F8();
extern void func_8003EB04();
extern int func_80192934();
extern void func_800870F0();
extern void func_8010C0D8();
extern void func_8007A2A4();
extern void func_80080DC4();
extern void func_80073A44();
extern void func_80070E54();

int func_80192CE8(int slot)
{
    int buf;
    int ret;
    unsigned int *f;
    unsigned int *g;
    short *q;
    unsigned char *p;
    int v;

    ret = 0;
    f = &D_800B0CD8;
    *f |= 0x200;
    D_801D0E04[slot].on = 1;
    func_80074D28(0);
    func_80074DC0(0);
    func_80074A44(1);
retry:
    while (func_8006E6A8(D_800B0DD8 + D_8009315E[0], D_8001160C, D_8009315E[1] - D_8009315E[0]) == -1) {
    }
    for (;;) {
        v = func_8006E7E8();
        if (v == 0) {
            break;
        }
        if (v == -1) {
            goto retry;
        }
    }
    func_80072714();
    func_800726C4();
    func_80072724();
    buf = D_80011610 + ((D_8009315E[2] - D_8009315E[1]) << 11);
    func_80191FB8(1, &buf);
    func_801924F8((short)slot);
    p = &D_800B0DBA;
    if (*p != 0) {
        q = (short *)(p + 2);
    loop:
        if (*q > 0) {
            func_8003EB04();
            if ((signed char)func_80192934() == 0) {
                D_801D0DE8 = 0;
                D_801D0DEC = 0;
                D_801D0DFC = 0;
                D_801D0DF8 = 0;
                D_801D0DF0 = 0;
                D_801D0DF4 = 0;
                D_800B0DBA = 0;
            } else if (D_8009D26C & 0x20000004) {
                D_800B0DBA = D_800B0DBA - 1;
                func_800870F0(0);
                func_8010C0D8(0);
                func_8007A2A4();
                func_80080DC4(9, 0, 0);
                v = *q;
                D_801D0DE8 = 0;
                D_801D0DEC = 0;
                D_801D0DFC = 0;
                D_801D0DF8 = 0;
                D_801D0DF0 = 0;
                D_801D0DF4 = 0;
                D_800B0DBA = 0;
                if (v < 0x578) {
                    func_80073A44(0);
                    func_80074D28(0);
                    ret = 1;
                }
            }
            func_80070E54();
            if (D_800B0DBA != 0) {
                goto loop;
            }
        }
    }
    g = &D_800B0CD8;
    *g &= ~0x200;
    return ret;
}
