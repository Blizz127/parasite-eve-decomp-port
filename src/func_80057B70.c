/*
 * func_80057B70 — per-object update/registration dispatcher (retail 0x80057B70).
 *
 * VRAM 0x80057B70 / file 0x48370 / size 0xA0 (40 words), in 4680C.s.
 *
 * v = func_8005DB44(arg0 + 0xEB). When the gp flag D_8009D028 (gp+0x2B8)
 * is set: s = func_800579D4(arg0, func_800515F8(0)); p = func_80051098()
 * and p[0]=1, p[1]=arg0, p[2]=s; then return func_800512AC(1, &arg0)
 * (the address of a stack copy of arg0). Otherwise, when v[0xE] == 1,
 * return func_80051770(arg0); else return 1.
 *
 * era -O2 -G8: D_8009D028 is at gp+0x2B8 (gp base 0x8009CD70), so the flag
 * must be a small-data access (`lw $v1,0x2B8($gp)`). `-G0` gives a 25-word
 * diff (absolute lui/lw) and also mis-schedules the frame.
 */
extern int D_8009D028;
extern void *func_8005DB44(int);
extern int func_800515F8(int);
extern int func_800579D4(int, int);
extern int *func_80051098(void);
extern int func_800512AC(int, int *);
extern int func_80051770(int);

int func_80057B70(int arg0) {
    unsigned char *v = func_8005DB44(arg0 + 0xEB);
    int s;
    int *p;
    int sp10;
    if (D_8009D028 != 0) {
        s = func_800579D4(arg0, func_800515F8(0));
        p = func_80051098();
        p[0] = 1;
        p[1] = arg0;
        p[2] = s;
        sp10 = arg0;
        return func_800512AC(1, &sp10);
    }
    if (v[0xE] == 1) {
        return func_80051770(arg0);
    }
    return 1;
}
