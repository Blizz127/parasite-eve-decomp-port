/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80123B1C — see docs/evidence/ovl_0457-func_80123B1C/REPORT.md. */
extern unsigned short D_80172CAC;

#define PUNCT(i)       \
    switch (c) {       \
    case ':': i++;     \
    case '?': i++;     \
    case '/': i++;     \
    case '.': i++;     \
    case ',': i++;     \
    case '\'': i++;    \
    case '-': i++;     \
    case ')': i++;     \
    case '(': i++;     \
    case '&': i++;     \
    case '#': i++;     \
    case '@': i++;     \
    case '!': i++;     \
    case '~': i++;     \
    }

unsigned char func_80123B1C(unsigned char c)
{
    int i;

    if (D_80172CAC != 0) {
        if ((unsigned char)(c - 'a') < 26) {
            i = c - 0x20;
        } else if ((unsigned char)(c - 'A') < 26) {
            i = c - 'A';
        } else if ((unsigned char)(c - '0') < 10) {
            i = c + 0x3A;
        } else {
            i = -1;
            PUNCT(i)
            i += 0x1A;
        }
    } else {
        if ((unsigned char)(c - 'A') < 26) {
            i = c - 0x19;
        } else if ((unsigned char)(c - 'a') < 26) {
            i = c - 0x1F;
        } else if ((unsigned char)(c - '0') < 10) {
            i = c + 0x3A;
        } else {
            i = -1;
            PUNCT(i)
            i += 0x5C;
        }
    }
    return i;
}
