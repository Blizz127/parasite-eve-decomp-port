/* room_m0418i — func_8018F3C4, blob offset 0x3DC, 0x138 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Object init: func_800C2B40, owner pointer + 32-byte block copy from *(p+0x238), func_8006DC18(0x2E) handle, two sprite descriptor seeds (store order via climb). */

typedef struct { int w[8]; } B32;
extern unsigned char D_80199670, D_80199671, D_80199672, D_80199674, D_80199675, D_80199676;
extern short D_80199678, D_8019967A;
extern unsigned char D_801994A8, D_801994A9, D_801994AA, D_801994AC, D_801994AD, D_801994AE;
extern short D_801994B0, D_801994B2;
extern void func_800C2B40();
extern int func_8006DC18();

void func_8018F3C4(char *a0, int a1, char *a2)
{
    char *p;

    func_800C2B40(a2);
    p = *(char **)(a0 + 8);
    *(char **)a2 = p;
    *(B32 *)(a2 + 4) = **(B32 **)(p + 0x238);
    *(int *)(a2 + 0x24) = func_8006DC18(0x2E);
    D_80199674 = 0x46;
    D_80199675 = 3;
    D_80199678 = -0x1E;
    D_8019967A = 0x80;
    D_80199670 = 0xA0;
    D_80199671 = 0x80;
    D_80199672 = 0xF0;
    D_80199676 = 0;
    D_801994AC = 0x46;
    D_801994AD = 3;
    D_801994B0 = -0x190;
    D_801994B2 = 0x80;
    D_801994A8 = 0xA0;
    D_801994A9 = 0x80;
    D_801994AA = 0xF0;
    D_801994AE = 0;
}
