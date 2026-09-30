typedef union {
    unsigned int flags;
    unsigned char bytes[0x100];
} State;

extern State D_800B0CD8;

int func_8006DB48(int index, signed char value0, unsigned char value1,
                  unsigned char value2) {
    State *base = &D_800B0CD8;
    int result = 0;

    base->bytes[index * 2 + 0xDC] = value0;
    base->bytes[index * 2 + 0xDD] = value1;
    base->bytes[index + 0xFE] = value2;

    if (index == 0) {
        base->flags |= 0x40;
    } else if (index == 1) {
        base->flags |= 0x80;
    }

    return result;
}
