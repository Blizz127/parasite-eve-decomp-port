/*
 * func_8003010C — opcode 0x59 slot tagged reader.
 *
 * VRAM 0x8003010C / file 0x2090C / size 0x114 (69 words). Leaf, no frame.
 *
 * slot = *actor.  idx = (tag & 0xFF) - 0x29.  `sltiu idx,0x5A` guards a
 * 90-index switch through the shared rodata pool table jtbl_80010B28;
 * unhandled in-range indices fall to the sentinel -1000 in $v1, moved to
 * $v0 by the shared epilogue, so the source keeps a `ret` variable.
 *
 * Tag 130 is a destructive get: it clears slot+0xCC bit 24 only when the
 * slot's 0x000C0000 mask is fully set.
 *
 * Build: era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 +
 * MASPSX_DISPATCH_FOLD=jtbl_80010B28 (profile era_o2_g0_dispatch_80010b28).
 * ROM: asm/disc1/202F8.s @ file 0x2090C, 69 words (0x114 bytes).
 */

typedef struct {
    /* 0x00 */ unsigned int w00;         /* b03 (byte 0x03) overlaps w00 */
    /* 0x04 */ signed char b04;
    /* 0x05 */ unsigned char gap05[7];
    /* 0x0C */ unsigned short h0C;
    /* 0x0E */ unsigned char gap0E[2];
    /* 0x10 */ int w10;
    /* 0x14 */ unsigned char gap14[8];
    /* 0x1C */ unsigned char b1C;
    /* 0x1D */ unsigned char gap1D[0x6B];
    /* 0x88 */ int w88;
    /* 0x8C */ unsigned short h8C;
    /* 0x8E */ unsigned char gap8E[0x3E];
    /* 0xCC */ unsigned int wCC;
} Slot;

int func_8003010C(Slot **actor, unsigned int tag) {
    Slot *slot = *actor;
    unsigned char t = tag;
    unsigned int idx = t - 0x29;
    int ret = -1000;

    if (idx < 0x5A) {
        switch (idx) {
        case 0:
            ret = slot->b04;
            break;
        case 2:
            ret = slot->h0C;
            break;
        case 3:
            ret = slot->w10;
            if (ret < 0)
                ret = 0;
            break;
        case 7:
            ret = *((unsigned char *)slot
                    + ((slot->w00 >> 17) & 0x70) + 0x1C);
            break;
        case 19:
            ret = slot->w88;
            if (ret < 0)
                ret = 0;
            break;
        case 20:
            ret = slot->h8C;
            break;
        case 36:
            ret = slot->w00 >> 13;
            ret &= 3;
            break;
        case 41:
            ret = ((unsigned char *)slot)[3];
            ret &= 0x3F;
            break;
        case 89: {
            unsigned int cc = slot->wCC;
            if (cc & 0x1000000) {
                if ((slot->w00 & 0xC0000) == 0xC0000) {
                    slot->wCC = cc & 0xFEFFFFFF;
                    ret = 1;
                } else {
                    ret = 0;
                }
            } else {
                ret = 0;
            }
            break;
        }
        }
    }
    return ret;
}
