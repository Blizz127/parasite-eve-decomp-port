/* room_m0350i — func_80199CA4, blob offset 0xACBC, 0x88 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * In-room twin of func_80199B0C (actor-list search, cursor D_8019A7F8); returns D_8019A7A0. */

extern void *D_8009D20C;
extern unsigned char *D_8019A7F8;
extern unsigned char D_8019A7A0[];

unsigned char *func_80199CA4(int a0, int a1, int a2)
{
    unsigned char **q = &D_8019A7F8;
    unsigned char **p;

    *q = D_8009D20C;
    if (*q != 0) {
        for (p = q; *p != 0; *p = *(unsigned char **)(D_8019A7F8 + 4)) {
            if (D_8019A7F8[0xC] == a1 && D_8019A7F8[0xD] == a2 && !(*(int *)(D_8019A7F8 + 0x98) & 0x10)) break;
        }
    }
    return D_8019A7A0;
}
