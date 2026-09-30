/* room_m0350i — func_80199B0C, blob offset 0xAB24, 0x88 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * actor-list search (D_8009D20C chain) for id bytes +0xC/+0xD with flag 0x10 clear; cursor D_8019A774; lever: guarded for-loop with p=q copy */

extern void *D_8009D20C;
extern unsigned char *D_8019A774;

unsigned char **func_80199B0C(int a0, int a1, int a2)
{
    unsigned char **q = &D_8019A774;
    unsigned char **p;

    *q = D_8009D20C;
    if (*q != 0) {
        for (p = q; *p != 0; *p = *(unsigned char **)(D_8019A774 + 4)) {
            if (D_8019A774[0xC] == a1 && D_8019A774[0xD] == a2 && !(*(int *)(D_8019A774 + 0x98) & 0x10)) break;
        }
    }
    return &D_8019A774;
}
