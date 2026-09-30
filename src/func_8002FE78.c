/*
 * func_8002FE78 — opcode 0x59 tagged field reader.
 *
 * VRAM 0x8002FE78 / file 0x20678 / size 0x100 (64 words). Leaf, no frame.
 *
 * tag = arg & 0xFF.  base = *(*D_8009D254).  Range test `sltiu t,0x17`
 * guards a 23-entry switch dispatched through jtbl_80010AC8.  The sentinel
 * -1000 lives in $a2 and is moved to $v0 in the shared epilogue; case bodies
 * just assign $a2 and jump there, so the source keeps a `ret` variable
 * rather than returning directly.
 *
 * Why `ret = x >> n; ret &= m;` for cases 20/21/22 instead of the single
 * expression `ret = (x >> n) & m`: with the compound statement form cc1
 * assigns the shift destination to $a2 (ret) and shares the `& 1` tail
 * between cases 20 and 22, exactly as retail does.  The one-expression form
 * makes cc1 shift in place in $v0 and tail-and into $a2, a five-word
 * register-allocation divergence.  All five words are in these three cases.
 *
 * Retail loads D_8009D254 with `lui $v0,%hi / lw $v0,%lo($v0)` (absolute),
 * so this leaf is -G0.  The switch dispatch is retail's shared rodata pool
 * table, reproduced by MASPSX_THREE_WORD_SYMBOL_STORE=1 (3-word indexed
 * symbol form) + MASPSX_DISPATCH_FOLD=jtbl_80010AC8, which retargets cc1's
 * private $L table to the pool symbol.
 * ROM: asm/disc1/202F8.s @ file 0x20678, 64 words (0x100 bytes).
 */

typedef struct {
    /* 0x00 */ int f00;
    /* 0x04 */ short f04;
    /* 0x06 */ unsigned short f06;
    /* 0x08 */ int f08;
    /* 0x0C */ short f0C;
    /* 0x0E */ unsigned char pad0E[2];
    /* 0x10 */ unsigned short f10;
    /* 0x12 */ unsigned char f12;
    /* 0x13 */ unsigned char pad13[9];
    /* 0x1C */ short f1C;
    /* 0x1E */ unsigned short f1E;
    /* 0x20 */ unsigned short f20;
    /* 0x22 */ unsigned short f22;
    /* 0x24 */ unsigned char pad24[4];
    /* 0x28 */ int f28;
    /* 0x2C */ unsigned char pad2C[0x20];
    /* 0x4C */ unsigned int f4C;
} AyaRec;

extern int *D_8009D254;

int func_8002FE78(unsigned int tag) {
    unsigned char t = tag;
    AyaRec *base = (AyaRec *)*D_8009D254;
    int ret = -1000;

    if (t < 0x17) {
        switch (t) {
        case 0:
            ret = base->f00;
            break;
        case 1:
            ret = base->f04;
            break;
        case 2:
            ret = base->f06;
            break;
        case 3:
            ret = base->f08;
            break;
        case 4:
            ret = base->f0C;
            break;
        case 6:
            ret = base->f10;
            break;
        case 7:
            ret = base->f12;
            break;
        case 10:
            ret = base->f1C;
            break;
        case 11:
            ret = base->f1E;
            break;
        case 12:
            ret = base->f20;
            break;
        case 13:
            ret = base->f28;
            break;
        case 14:
            ret = base->f22;
            break;
        case 20:
            ret = base->f4C >> 9;
            ret &= 1;
            break;
        case 21:
            ret = base->f4C >> 6;
            ret &= 3;
            break;
        case 22:
            ret = base->f4C >> 29;
            ret &= 1;
            break;
        }
    }
    return ret;
}
