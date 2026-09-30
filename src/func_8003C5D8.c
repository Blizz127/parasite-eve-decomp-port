/*
 * Initializes four byte fields from a signed 16-bit scale. A zero scale is
 * replaced with one before the reciprocal byte is computed.
 *
 * Retail: VRAM 0x8003C5D8, file 0x2CDD8, size 0x60.
 */
void func_8003C5D8(unsigned char *state, short scale)
{
    int reciprocal;

    if (scale == 0) {
        scale = 1;
    }

    reciprocal = 128 / scale;
    state[0x8D] = scale;
    state[0x8E] = reciprocal;
    state[0x8F] = reciprocal;
    state[0x93] = reciprocal;
}
