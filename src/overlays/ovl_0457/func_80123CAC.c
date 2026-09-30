/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80123CAC — see docs/evidence/ovl_0457-func_80123CAC/REPORT.md. */
extern unsigned short D_80172CAC;
extern unsigned char D_80125B1C[][4];

unsigned char func_80123CAC(unsigned char c)
{
    unsigned short m = D_80172CAC;

    if (m == 2) {
        if ((unsigned char)(c - 'a') < 26) {
            c -= 0x20;
        }
        if ((unsigned char)(c - 'A') < 26) {
            return D_80125B1C[c][2];
        }
        switch (c) {
        case '#':
            return 7;
        case '&':
            return 11;
        case '!':
        case '\'':
        case '.':
        case ':':
            return 3;
        case '(':
        case ')':
        case ',':
        case '-':
        case '/':
            return 4;
        case '@':
            return 9;
        case '?':
            return 6;
        case '~':
        default:
            return 8;
        }
    }
    if ((unsigned char)(c - 'A') < 26) {
        return D_80125B1C[c][m];
    }
    if ((unsigned char)(c - 'a') < 26) {
        return D_80125B1C[c - 0x20][m | 1];
    }
    switch (c) {
    case '!':
    case '.':
    case ':':
        return 2;
    case '\'':
    case ',':
        return 3;
    case '(':
    case ')':
    case '-':
    case '/':
    case '?':
        return 4;
    case '&':
    case '@':
        return 9;
    case '~':
    default:
        return 6;
    }
}
