/* Phase 6C: func_80017588 — VRAM 0x80017588, size 0x130, file 0x7D88-0x7EB8.
 * Actor action-relative cursor update. arg0 is a two-pointer descriptor:
 * *arg0[1] is the signed relative offset, *arg0[0] is the code selecting the
 * destination field of D_8009D2F0 (or the D_8009D300 task at +4). Negative
 * offsets store 0; non-negative store D_8009D2F0->0x9C + rel*2. Returns 1.
 * era -O2 -G8. D_8009D2F0 must stay absolute (incomplete array); D_8009D300
 * is the gp+0x590 scalar (0x8009D300). */
typedef struct {
    char pad0[0x9C];
    int  unk9C;
    char padA0[0xFC];
    int  unk19C;
    int  unk1A0;
} Actor;

typedef struct {
    char pad0[4];
    int  unk4;
} Task;

extern Actor *D_8009D2F0[];
extern Task *D_8009D300;

int func_80017588(int **arg0)
{
    int rel = *arg0[1];
    if (rel < 0) {
        switch (*arg0[0]) {
        case 1:
            D_8009D2F0[0]->unk1A0 = 0;
            break;
        case 2:
            D_8009D2F0[0]->unk19C = 0;
            break;
        case 3:
            D_8009D300->unk4 = 0;
            break;
        }
    } else {
        switch (*arg0[0]) {
        case 1:
            D_8009D2F0[0]->unk1A0 = D_8009D2F0[0]->unk9C + rel * 2;
            break;
        case 2:
            D_8009D2F0[0]->unk19C = D_8009D2F0[0]->unk9C + rel * 2;
            break;
        case 3:
            D_8009D300->unk4 = D_8009D2F0[0]->unk9C + rel * 2;
            break;
        }
    }
    return 1;
}
