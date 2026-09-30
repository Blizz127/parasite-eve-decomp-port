/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80123950 — blob offset 0x2C50, 0x1CC bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80123950/REPORT.md).
 * Credits text renderer: walk the D_80125C88 line table (cursor D_80172CB0) from scroll y = D_80172CA4 >> 8, 0x12 px per line, drawing visible lines (y in [-7, 0xE0]) glyph by glyph through D_80172CD0->draw; '\\' centres the rest of the line, TAB advances 14 spaces, ' ' just advances; 0xFF ends. Returns 1 when no line was visible; advances the scroll by D_80172CA8. Levers: a switch (GCC's decision tree gives retail's == 0x20 / slti 0x21 / 9 / 0x5C order) with case '\\' first, unsigned char c, and p[1] indexing in the width loop. */

typedef struct {
    void *f0;
    void *f4;
    void *f8;
    void (*draw)();
} Font;
extern unsigned char *D_80125C88[];
extern unsigned char **D_80172CB0;
extern int D_80172CA4;
extern int D_80172CA8;
extern unsigned short D_80172CAC;
extern Font *D_80172CD0;
extern unsigned short func_80123CAC();
int func_80123950(void)
{
    unsigned char *s;
    unsigned char *p;
    int x;
    int y;
    int w;
    int count;
    unsigned char c;

    D_80172CB0 = D_80125C88;
    s = D_80125C88[0];
    x = 0x18;
    y = D_80172CA4 >> 8;
    count = 0;
    while (*s != 0xFF) {
        if (y >= -7) {
            if (y >= 0xE1) break;
            if (*s != 0) {
                c = *s;
                do {
                    switch (c) {
                    case '\\':
                        w = 0;
                        for (p = s; p[1] != 0; p++) {
                            w += func_80123CAC(p[1]);
                        }
                        x = (unsigned int)(320 - w) >> 1;
                        break;
                    case '\t':
                        x += func_80123CAC(' ') * 14;
                        break;
                    case ' ':
                        goto space;
                    default:
                        D_80172CD0->draw((short)x, (short)y, *s);
                    space:
                        x += func_80123CAC(*s);
                        break;
                    }
                    s++;
                    c = *s;
                } while (c != 0);
            }
            D_80172CAC = 0;
            x = 0x18;
            count++;
        }
        s = *++D_80172CB0;
        y += 0x12;
    }
    D_80172CA4 += D_80172CA8;
    return count == 0;
}
