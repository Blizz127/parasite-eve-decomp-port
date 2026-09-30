/*
 * func_8008D844 - vram 0x8008D844, size 0x338. Sound fade tick (SPU_StepReverbLoad), every 4th
 * frame: master/reverb fades (D_8009D2A2/D220/D21E counters), the two sequence volume fades
 * through D_8009D2C8 (Seq records), and per-voice pan/volume/pitch fades for the active-voice
 * mask D_800BCD50 over the 0x11C-byte voice records, flagging dirty SPU fields.
 * era: cc1 2.7.2 -O2 -G0 (default profile).
 * Levers: Seq/Voice struct records (struct-vs-scalar aliasing lets the next record load move
 * above the D_8009D2C8 store); the voice walk is a goto loop (a loop.c loop biases the pointer
 * giv by +0xB8); `bit` doubles as the 24-voice counter (retail keeps both in $a2).
 */
extern unsigned short D_8009CDEC;
extern short D_8009D2A2;
extern int D_8009D2B4;
extern int D_8009D284;
extern short D_8009D220;
extern int D_8009D2D0;
extern int D_8009D214;
extern short D_8009D21E;
extern int D_8009D2CC;
extern int D_8009D210;
extern unsigned char D_800B8BB4[];
typedef struct {
    int pitch;
    int pitch_step;
    unsigned char pad8[0x2C];
    unsigned short cnt_pitch;
    unsigned short pad36;
    unsigned short cnt_pan;
    unsigned short vol;
    unsigned short cnt_vol;
    unsigned char pad3E[0x5E];
    short pan;
    short pan_step;
    short vol_step;
    unsigned char padA2[0x16];
    unsigned int flags;
    unsigned char padBC[0x60];
} Voice;

typedef struct {
    int pad0;
    int on;
    int pad8[16];
    int vol;
    int step;
    short cnt;
    short pad52;
    int pad54[5];
} Seq;

extern Seq *D_8009D2C8;
extern unsigned int D_800BCD50;
extern unsigned char D_800BC03C[];
extern unsigned char D_800B8AC0[];
extern unsigned char D_800BA560[];
extern void func_8008D7D0(void);
extern void func_8008AB9C(unsigned char *);

void func_8008D844(void)
{
    int v;
    unsigned int mask;
    unsigned int bit;
    Voice *v2;

    if ((++D_8009CDEC & 3) != 0) {
        return;
    }
    if (D_8009D2A2 != 0) {
        D_8009D2A2--;
        D_8009D2B4 += D_8009D284;
        func_8008D7D0();
    }
    if (D_8009D220 != 0) {
        D_8009D220--;
        D_8009D2D0 += D_8009D214;
    }
    if (D_8009D21E != 0) {
        D_8009D21E--;
        v = D_8009D2CC + D_8009D210;
        if ((v & 0xFF0000) != (D_8009D2CC & 0xFF0000)) {
            unsigned char *q;

            bit = 24;
            q = D_800B8BB4;
            for (; bit != 0; bit--) {
                *(unsigned int *)q |= 0x10;
                q += 0x11C;
            }
        }
        D_8009D2CC = v;
    }
    if (D_8009D2C8->on != 0 && D_8009D2C8->cnt != 0) {
        D_8009D2C8->cnt -= 1;
        v = D_8009D2C8->vol + D_8009D2C8->step;
        if ((v & 0x7F0000) != (D_8009D2C8->vol & 0x7F0000)) {
            func_8008AB9C(D_800B8AC0);
        }
        D_8009D2C8->vol = v;
    }
    D_8009D2C8++;
    if (D_8009D2C8->on != 0 && D_8009D2C8->cnt != 0) {
        D_8009D2C8->cnt -= 1;
        v = D_8009D2C8->vol + D_8009D2C8->step;
        if ((v & 0x7F0000) != (D_8009D2C8->vol & 0x7F0000)) {
            func_8008AB9C(D_800BA560);
        }
        D_8009D2C8->vol = v;
    }
    D_8009D2C8--;
    mask = D_800BCD50;
    if (mask != 0) {
        bit = 0x1000;
        v2 = (Voice *)D_800BC03C;
    loop:
        {
            if (mask & bit) {
                if (v2->cnt_pan != 0) {
                    v2->cnt_pan--;
                    v = v2->pan + v2->pan_step;
                    if ((v & 0xFF00) != (v2->pan & 0xFF00)) {
                        v2->flags |= 3;
                    }
                    v2->pan = v;
                }
                if (v2->cnt_vol != 0) {
                    v2->cnt_vol--;
                    v = v2->vol + v2->vol_step;
                    if ((v & 0xFF00) != (v2->vol & 0xFF00)) {
                        v2->flags |= 3;
                    }
                    v2->vol = v;
                }
                if (v2->cnt_pitch != 0) {
                    v2->cnt_pitch--;
                    v = v2->pitch + v2->pitch_step;
                    if ((v & 0xFF00) != (v2->pitch & 0xFF00)) {
                        v2->flags |= 0x10;
                    }
                    v2->pitch = v;
                }
                mask ^= bit;
            }
            bit <<= 1;
            v2++;
        }
        if (mask != 0) {
            goto loop;
        }
    }
}
