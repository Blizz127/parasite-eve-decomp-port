/* room_m0429i — func_801926B4, blob offset 0x36CC, 0x1C4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * Object gate handler (range check -> scratchpad distance test -> player lock + 32-byte pose copy). Levers: second D_8009D254 reload pinned register char *q asm($4) (retail's $a0 base with the 0x10000 mask in $a1; 30 -> 4), third reload a separate local u (4 -> 0). */

typedef struct { int w[8]; } B32;
extern char *D_8009D254;
extern void func_80192D04();
extern int func_800DFE20();
extern void func_80020C74();
extern void func_80192878();
extern void func_80192BC0();

void func_801926B4(char *a0)
{
    char *r = *(char **)(a0 + 8);
    char *s = a0 + 0xC;
    char *v;
    char *p;
    register char *q asm("$4");
    int k;
    char *u;

    v = *(char **)r;
    if (v != 0 && *(int *)(v + 0x10) <= 1000000) {
        **(int **)(a0 + 0x10) = 4;
        func_80192D04();
        return;
    }
    p = D_8009D254;
    if (*(unsigned char *)(p + 0xE) < 0x12) {
        k = *(unsigned short *)(r + 0x16);
        if (k < 4) {
            return;
        }
        if (k < 0xC) {
            *(int *)0x1F800008 = *(int *)(*(char **)(r + 0x238) + 0x174) << 16;
            *(int *)0x1F800010 = *(int *)(*(char **)(r + 0x238) + 0x17C) << 16;
            if (func_800DFE20(p + 0x28, (void *)0x1F800008) >= 0x140) {
                return;
            }
            *(void **)s = func_80192878;
            func_80020C74();
            q = D_8009D254;
            *(char **)(q + 0x1D8) = r + 0x1B4;
            *(short *)(q + 0x1DC) = 4;
            *(short *)(q + 0x1DE) = 0xB;
            *(int *)(q + 0x98) |= 0x10000;
            *(unsigned short *)(q + 0x250) |= 0x400;
            *(B32 *)(q + 0x1E8) = *(B32 *)(s + 0x10);
            s[0x38] = 1;
            u = D_8009D254;
            *(int *)(s + 0x30) = *(int *)(u + 0x28);
            *(int *)(s + 0x34) = *(int *)(u + 0x30);
            return;
        }
    }
    **(int **)(s + 4) = 3;
    *(void **)s = func_80192BC0;
}
