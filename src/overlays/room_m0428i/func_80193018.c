/* room_m0428i — func_80193018, blob offset 0x4030, 0x2F4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * Effect callback (called through func_800D413C; m0427i's script reaches it
 * too). Mode 1 counts the timer down by the step (returns 1 below 8, setting
 * the actor's state byte to 4 when flagged), grows the ring radius with the
 * story frame counter (cos, capped at 1024) and, while armed, disarms when
 * the player is inside 3/8 of the radius: it sets the player flag 0x4000 and
 * the actor's top bit. Mode 2 draws two rings through func_800D0728 with the
 * colour pair D_8019411C, alpha min(timer, 128).
 * Matching notes: the alpha is a ternary into a short, then widened; the
 * work block is a struct (bytes at +6/+7). */
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
typedef struct { short timer; short step; short size; unsigned char flag; unsigned char armed; } WORK;

extern int *D_800F32D0;
extern int *D_8009D254;
extern int D_800E27EC;
extern CVECTOR D_8019411C[];
extern int func_80077CF4();
extern int func_8005186C();
extern void func_800D0728();

int func_80193018(int mode, WORK *w)
{
    SVECTOR pos;
    SVECTOR rot;
    int *e;
    int *pl;
    unsigned char *b;
    int a;
    int dx;
    int dz;
    short t;
    int v;

    switch (mode) {
    case 1:
        e = (int *)D_800F32D0[2];
        b = ((unsigned char **)e[0])[6];
        if (*b == 1)
            *b = 2;
        w->timer -= w->step;
        if (w->timer < 8) {
            if (w->flag != 0)
                *((unsigned char **)e[0])[6] = 4;
            return 1;
        }
        a = (D_800E27EC + 1) * 24;
        if (a > 1024)
            a = 1024;
        w->size = func_80077CF4(a);
        if (w->armed == 0)
            break;
        pl = D_8009D254;
        dx = (pl[10] - e[10]) >> 16;
        dz = (pl[12] - e[12]) >> 16;
        if (func_8005186C(dx * dx + dz * dz) < (w->size * 3) >> 3) {
            w->armed = 0;
            ((int *)D_8009D254[0])[19] |= 0x4000;
            *(int *)e[0] |= 0x80000000;
        }
        if (w->timer < 64)
            w->armed = 0;
        break;
    case 2:
        e = (int *)D_800F32D0[2];
        t = (w->timer > 128) ? 128 : w->timer;
        v = t;
        if (w->timer > 0) {
            pos.vx = e[127];
            pos.vy = e[128] - 320;
            pos.vz = e[129];
            rot.vx = 1024;
            rot.vy = D_800E27EC << 7;
            rot.vz = 0;
            rot.pad = 1;
            func_800D0728(&pos, 1280, 1536, 16, &rot, w->size, w->size,
                          &D_8019411C[0], &D_8019411C[1], v, 1);
            func_800D0728(&pos, 1536, 1664, 16, &rot, w->size, w->size,
                          &D_8019411C[1], &D_8019411C[0], v, 1);
        }
        break;
    }
    return 0;
}
