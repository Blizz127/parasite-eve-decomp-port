/*
 * func_80066268 - vram 0x80066268, size 0x338. Camera pan step (render/Scene_IsNotBattleMode):
 * modes 1-3 move D_800BCF8C/8E from (F98,F9A) to (F9C,F9E) over F9A2 frames, linear or
 * cosine-eased (flag 8), recentre the view against the room record size, advance time and
 * finish (flags |= 0x80 / 0x84).
 * era: cc1 2.7.2 -O2 -G0 + MASPSX_EXPAND_DIV=1.
 * Levers: f & 8 read before the early-out; dx/dy/t/dur read in retail order; both cos products
 * before the stores; D_800B1624 pointer-to-volatile (two reads); Room struct for the record
 * (struct-vs-scalar aliasing lets rec->h load above the D_800BCF94 store); the recentre
 * terms through int temporaries (a short store of one expression lets the front end narrow
 * the arithmetic to unsigned short, turning retail lh into lhu).
 */
extern unsigned int D_800BCF88;
extern short D_800BCF8C;
extern short D_800BCF8E;
extern short D_800BCF94;
extern short D_800BCF96;
extern short D_800BCF98;
extern short D_800BCF9A;
extern short D_800BCF9C;
extern short D_800BCF9E;
extern short D_800BCFA0;
extern short D_800BCFA2;
extern unsigned char *volatile D_800B1624;
extern unsigned char D_800BCFFD;
extern int func_80077DC4(int);

typedef struct {
    unsigned char pad[0x28];
    unsigned short w;
    unsigned short h;
    unsigned char pad2[8];
} Room;

int func_80066268(void)
{
    unsigned int f;
    int m;
    short dx;
    short dy;
    short dur;
    int c;
    Room *rec;
    unsigned int e;
    short t;

    f = D_800BCF88;
    if (!(f & 0x40)) {
        return -20;
    }
    m = f & 7;
    e = f & 8;
    if ((unsigned int)(m - 1) >= 2 && m != 3) {
        return 0;
    }
    dx = D_800BCF9C - D_800BCF98;
    dy = D_800BCF9E - D_800BCF9A;
    t = D_800BCFA0;
    dur = D_800BCFA2;
    if (e == 0) {
        D_800BCF8C = D_800BCF98 + dx * t / dur;
        D_800BCF8E = D_800BCF9A + dy * t / dur;
    } else {
        int a;
        int b;

        c = func_80077DC4((t << 11) / dur + 0x800) + 0x1000;
        a = dx * c / 0x2000;
        b = dy * c / 0x2000;
        D_800BCF8C = D_800BCF98 + a;
        D_800BCF8E = D_800BCF9A + b;
    }
    rec = (Room *)(D_800B1624 + *(int *)(D_800B1624 + 0x1C)) + D_800BCFFD;
    {
        int a = D_800BCF8C - (short)rec->w / 2;
        int b = D_800BCF8E - (short)rec->h / 2;

        D_800BCF94 = 0xA0 - a;
        D_800BCF96 = 0x70 - b;
    }
    if (m == 1) {
        D_800BCF88 = (D_800BCF88 & ~7) | 2;
    }
    if (++D_800BCFA0 > dur) {
        if ((D_800BCF88 & 7) == 2) {
            D_800BCF88 = (D_800BCF88 & ~7) | 0x84;
        } else {
            D_800BCF88 = (D_800BCF88 & ~7) | 0x80;
        }
    }
    return 0;
}
