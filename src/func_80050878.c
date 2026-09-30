/* Phase 6C: func_80050878 — VRAM 0x80050878, size 0x130, file 0x41078-0x411A8.
 * Equipment-page action dispatcher. Indexes the 12-byte rows of D_80092234 by
 * the gp word D_8009CDA8 (0x8009CDA8) times arg0. Case 0 rebuilds the record
 * for D_8009CF04 and pushes the enabled flag; case 2 pushes the "no result"
 * flag of func_80057654; case 3 forwards D_8009CF08. Every path terminates
 * through func_80064C30(func_8005DC4C(action)). era -O2 -G8. */
extern int D_8009CDA8;
extern int D_8009CF04;
extern int D_8009CF08;
extern int D_8009CF0C;
extern void func_8005EB58(int);
extern int func_8005DC4C(int);
extern void func_80064C30(int);
extern int func_80055FE0(int);
extern char *func_8005332C(int);
extern int func_80057654(int);

typedef struct { int v[3]; } Row;
extern Row D_80092234[];

void func_80050878(int arg0)
{
    Row *table = D_80092234;
    int action = table[D_8009CDA8].v[arg0];
    switch (action) {
    case 0: {
        int selected = D_8009CF04;
        int enabled = 0;
        char *rec = func_8005332C(selected);
        if (func_80055FE0(selected))
            enabled = D_8009CF0C != 1 || rec[6] != 10 || rec[0xE] < 4;
        func_8005EB58(!enabled);
        break;
    }
    case 1:
        break;
    case 2:
        func_8005EB58(func_80057654(D_8009CF04) == 0);
        break;
    case 3:
        func_8005EB58(D_8009CF08);
        break;
    }
    func_80064C30(func_8005DC4C(action));
}
