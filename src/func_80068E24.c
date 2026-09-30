/*
 * func_80068E24 - vram 0x80068E24, size 0x328. Screen fade (Render_SetCDDCSlot): for mode 2
 * interpolates the fade TILE colour from->to over `dur` frames, else sets the target colour;
 * links the TILE and a DR_MODE (tpage bits from +0x67) into the current OT slot +3 with
 * PsyQ addPrim (P_TAG bitfields); ends the fade at time >= dur.
 * era: cc1 2.7.2 -O2 -G0 + MASPSX_EXPAND_DIV=1 (profile era_o2_g0_expand_div).
 * Levers: addPrim/setaddr/getaddr through the P_TAG addr:24/len:8 bitfield (the mask/or order);
 * setlen as the P_TAG len byte; the DR_MODE code store addressed as
 * (DR_MODE *)((char *)s->mode + i * 8) (retail addu idx*8 first).
 */
typedef struct {
    unsigned int tag;
    unsigned char r, g, b, code;
    short x, y;
    short w, h;
} TILE;

typedef struct {
    unsigned int tag;
    unsigned int code;
} DR_MODE;

typedef struct {
    unsigned char pad0[0x30];
    TILE tile[2];
    DR_MODE mode[2];
    short to_r;
    short to_g;
    short to_b;
    unsigned char flags;
    unsigned char tpage;
    short from_r;
    short from_g;
    short from_b;
    unsigned short dur;
    unsigned short time;
} Fade;

typedef struct {
    unsigned addr : 24;
    unsigned len : 8;
} P_TAG;

#define setaddr(p, a) (((P_TAG *)(p))->addr = (unsigned int)(a))
#define getaddr(p) (((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)

extern Fade D_800BCF88;
extern int D_8009CDDC;
extern unsigned int *D_800B0E38[];

int func_80068E24(void)
{
    Fade *s;
    int m;
    int f4;
    int d;
    int t;

    s = &D_800BCF88;
    m = s->flags & 3;
    f4 = s->flags & 4;
    if (m == 0) {
        return 0;
    }
    if (m == 2) {
        d = s->dur - 1;
        if (d <= 0) {
            d = 1;
        }
        t = s->time;
        s->tile[D_8009CDDC].r = s->from_r + (s->to_r - s->from_r) * t / d;
        s->tile[D_8009CDDC].g = s->from_g + (s->to_g - s->from_g) * t / d;
        s->tile[D_8009CDDC].b = s->from_b + (s->to_b - s->from_b) * t / d;
    } else {
        s->tile[D_8009CDDC].r = s->to_r;
        s->tile[D_8009CDDC].g = s->to_g;
        s->tile[D_8009CDDC].b = s->to_b;
    }
    addPrim(D_800B0E38[D_8009CDDC] + 3, &s->tile[D_8009CDDC]);
    ((P_TAG *)&s->mode[D_8009CDDC])->len = 1;
    ((DR_MODE *)((char *)s->mode + D_8009CDDC * 8))->code = 0xE1000400 | ((s->tpage & 3) << 5);
    addPrim(D_800B0E38[D_8009CDDC] + 3, &s->mode[D_8009CDDC]);
    if (m == 2) {
        s->time++;
        if (s->time >= s->dur) {
            if (f4) {
                s->flags = 0;
            } else {
                s->flags = 1;
            }
        }
    }
    return 0;
}
