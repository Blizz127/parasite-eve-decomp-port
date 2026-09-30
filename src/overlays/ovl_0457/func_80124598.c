/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80124598 — blob offset 0x3898, 0x94 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80124598/REPORT.md).
 * Emit one 0x1C-byte glyph prim at D_80172CB8 (func_8012462C with the func_80123B1C cell split into u/v nibbles), link it via func_80077AC4, advance the cursor. */

extern unsigned char *D_80172CB8;
extern unsigned char *D_80172CB4;
extern int func_80123B1C(unsigned char);
extern void func_8012462C();
extern void func_80077AC4();
void func_80124598(unsigned short x, unsigned short y, unsigned char c)
{
    unsigned char *p = D_80172CB8;
    int v = func_80123B1C(c);
    func_8012462C(p, x, y, (v & 0xF) << 4, v & 0xF0);
    func_80077AC4(D_80172CB4 + 0x2C, p);
    D_80172CB8 += 0x1C;
}
