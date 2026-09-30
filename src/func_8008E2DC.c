/*
 * func_8008E2DC - vram 0x8008E2DC, size 0x20C. Akao sequence scanner (Akao_LookupSampleBankByte):
 * skips non-note opcodes (length table D_8009B7BC, 0xFC sub-opcodes via D_8009B8BC,
 * conditional/relative jumps, loop counters at +0x62[k], k at +0xCE) and returns the next
 * note byte, or 0xA0 when a terminator is reached.
 * era: cc1 2.7.2 -O2 -G0 + MASPSX_DISPATCH_FOLD=jtbl_8001213C + MASPSX_THREE_WORD_SYMBOL_STORE=1
 * (the fold only fires with the three-word knob; 52-entry 0xC9..0xFC table).
 * Levers: goto layout in retail block order; int c with unsigned casts on the note-range tests
 * (retail sltiu there, slti for the 0xFC sub-opcode tests); k-- / k &= 3 in place; lo += hi << 8.
 */
extern unsigned char D_8009B7BC[];
extern unsigned char D_8009B8BC[];
extern unsigned char *D_8009D2C8;

int func_8008E2DC(unsigned char *a0)
{
    unsigned char *p;
    unsigned int k;
    int c;
    int off;
    int lo;
    int hi;

    p = *(unsigned char **)a0;
    k = *(unsigned short *)(a0 + 0xCE);
top:
    c = *p;
redo:
    if ((unsigned int)c < 0x9A) {
        if ((unsigned int)c >= 0x8F) {
            *(unsigned short *)(a0 + 0x82) = 0;
            *(unsigned short *)(a0 + 0x84) &= 0xFFFA;
        }
        return *p;
    }
    if ((unsigned int)c < 0xA0) {
        return 0xA0;
    }
    if (D_8009B7BC[c] != 0) {
        off = D_8009B7BC[c];
        goto skip;
    }
    switch (c) {
    case 0xFC:
        p++;
        c = *p;
        if (D_8009B8BC[c] != 0) {
            off = D_8009B8BC[c];
            goto skip;
        }
        if (c == 7) {
            goto op7;
        }
        if (c < 8) {
            if (c == 6) {
                goto op6;
            }
            goto redo;
        }
        if (c >= 10) {
            goto top;
        }
        p++;
        if (*p == *(unsigned short *)(a0 + 0x62 + k * 2) + 1) {
            p++;
            lo = *p;
            p++;
            hi = *p;
            p++;
            k--;
            k &= 3;
            goto rel16tail;
        }
        p += 3;
        goto top;
    op6:
        p++;
        goto rel16;
    op7:
        p++;
        c = *p;
        p++;
        if (*(unsigned short *)(D_8009D2C8 + 0x56) < (unsigned int)c) {
            goto skip2;
        }
    rel16:
        lo = *p;
        p++;
        hi = *p;
        p++;
    rel16tail:
        lo += hi << 8;
        off = (short)lo;
    skip:
        p += off;
        goto top;
    skip2:
        p += 2;
        goto top;
    case 0xC9:
        p++;
        if (*p == *(unsigned short *)(a0 + 0x62 + k * 2) + 1) {
            p++;
            k--;
            k &= 3;
            goto top;
        }
        goto jumpk;
    case 0xCB:
    case 0xCD:
    case 0xD1:
    case 0xDB:
        p++;
        *(unsigned short *)(a0 + 0x82) = 0;
        *(unsigned short *)(a0 + 0x84) &= 0xFFFA;
        goto top;
    case 0xCC:
    case 0xD0:
        *(unsigned short *)(a0 + 0x84) &= 0xFFFA;
        return 0xA0;
    case 0xCA:
        if (*(int *)(a0 + 0x38) & 0x200000) {
            goto dflt;
        }
    jumpk:
        p = *(unsigned char **)(a0 + 4 + k * 4);
        goto top;
    default:
    dflt:
        *(unsigned short *)(a0 + 0x82) = 0;
        *(unsigned short *)(a0 + 0x84) &= 0xFFFA;
        return 0xA0;
    }
}
