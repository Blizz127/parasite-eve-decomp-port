/*
 * func_80012E7C — selector-driven triple copy from D_8009D2F0.
 *
 * VRAM 0x80012E7C / file 0x367C / size 0x238 (142 words). Leaf, no frame,
 * no calls.
 *
 * sel = **ctx; if (sel < 7) switch through the shared rodata pool table
 * jtbl_80010080.  Each case reloads *D_8009D2F0 for every one of its three
 * stores (the stores go through ctx[1..3], which may alias the global
 * pointer) and copies three fields into *ctx[1]/[2]/[3]:
 *   case 0: +0x28 +0x2C +0x30      case 1: +0x40 +0x44 +0x48
 *   case 2: +0x68 +0x6C +0x70      case 3: +0x78 +0x7C +0x80
 *   case 4: +0x88 +0x8C +0x90      case 6: +0x58 +0x5C +0x60   (word)
 *   case 5: +0x38 +0x3A +0x3C                                 (signed half)
 * Returns 1.
 *
 * D_8009D2F0 is loaded absolutely (`lui/lw`), so it is declared as an
 * incomplete array; -G0 seeds no gp-relative access.
 *
 * Build: era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 +
 * MASPSX_DISPATCH_FOLD=jtbl_80010080.
 * ROM: asm/disc1/3420.s @ file 0x367C, 142 words (0x238 bytes).
 */

extern unsigned char *D_8009D2F0;

int func_80012E7C(int **ctx) {
    switch (**ctx) {
    case 0:
        *ctx[1] = *(int *)(D_8009D2F0 + 0x28);
        *ctx[2] = *(int *)(D_8009D2F0 + 0x2C);
        *ctx[3] = *(int *)(D_8009D2F0 + 0x30);
        break;
    case 1:
        *ctx[1] = *(int *)(D_8009D2F0 + 0x40);
        *ctx[2] = *(int *)(D_8009D2F0 + 0x44);
        *ctx[3] = *(int *)(D_8009D2F0 + 0x48);
        break;
    case 2:
        *ctx[1] = *(int *)(D_8009D2F0 + 0x68);
        *ctx[2] = *(int *)(D_8009D2F0 + 0x6C);
        *ctx[3] = *(int *)(D_8009D2F0 + 0x70);
        break;
    case 3:
        *ctx[1] = *(int *)(D_8009D2F0 + 0x78);
        *ctx[2] = *(int *)(D_8009D2F0 + 0x7C);
        *ctx[3] = *(int *)(D_8009D2F0 + 0x80);
        break;
    case 4:
        *ctx[1] = *(int *)(D_8009D2F0 + 0x88);
        *ctx[2] = *(int *)(D_8009D2F0 + 0x8C);
        *ctx[3] = *(int *)(D_8009D2F0 + 0x90);
        break;
    case 5:
        *ctx[1] = *(short *)(D_8009D2F0 + 0x38);
        *ctx[2] = *(short *)(D_8009D2F0 + 0x3A);
        *ctx[3] = *(short *)(D_8009D2F0 + 0x3C);
        break;
    case 6:
        *ctx[1] = *(int *)(D_8009D2F0 + 0x58);
        *ctx[2] = *(int *)(D_8009D2F0 + 0x5C);
        *ctx[3] = *(int *)(D_8009D2F0 + 0x60);
        break;
    }
    return 1;
}
