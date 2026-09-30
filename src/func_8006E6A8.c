/*
 * func_8006E6A8 — sector-read issue wrapper. Forwards
 *   (lba, dest, sectors) -> func_8006E6D4(lba, 0, dest, sectors)
 * with a 24-byte frame (ra + outgoing arg dump). dest is kept as
 * unsigned char * because callers pass RAM destinations; the shuffle
 * itself is type-agnostic.
 *
 * ROM: VRAM 0x8006E6A8, file 0x5EEA8, 11 words / 0x2C bytes.
 * Preceded by the jr/nop of the previous function at 0x8006E6A0/0xA4;
 * followed by func_8006E6D4 at 0x8006E6D4.
 */
extern int func_8006E6D4(int lba, int mode, unsigned char *dest, int sectors);

int func_8006E6A8(int lba, unsigned char *dest, int sectors)
{
    return func_8006E6D4(lba, 0, dest, sectors);
}
