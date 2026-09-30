/*
 * Hand adapters — port_absent lane round 2 (2026-09-24): matched leaves in
 * 0x80038000..0x8005FFFF that the generator reports (guest-layout structs
 * with pointer members, pointer-to-pointer reads, code addresses stored in
 * window records, a void callee whose $v0 is consumed).  Each body follows
 * its matched src/func_XXXXXXXX.c line by line: guest pointers are pe_addr_t,
 * guest pointers held in RAM are loaded with PE_LoadU32, struct fields are
 * accessed at the leaf's own offsets and widths, and a stored code pointer
 * is the callee's retail VMA (what the retail lui/addiu pair loads).
 * Callees with no pc_port implementation are loud boundaries.
 * Tests: pc_port/tests/test_pa_lo2.h.
 */
#include "pe_guest_decomp.h"

/* decomp-derived callees (generated TUs have no header of their own) */
int func_80053F20(int a0);

/* ── save-slot glyph pages (0x800389DC..0x80039898) ──────────────────── */

/* Font record D_80091A1C: bytes +1 D_80091A1D (page key), +2 D_80091A1E
 * (slot), +3 cur (0x80091A1F), +4 D_80091A20 (flag), +0xC data (guest
 * pointer, 0x80091A28) — the leaves' `Font` layout. */
#define LO2_FONT_CUR  0x80091A1Fu
#define LO2_FONT_DATA 0x80091A28u

/* src/func_800389DC.c: read the save-slot block (sectors D_80093176[0..1]
 * from the base D_800B0DD8.base) into D_800B0DD8.buf (+0x94), retrying the
 * issue while it returns -1 and restarting when the poll reports -1; then
 * copy the 328-byte record of `slot` (bytes 1..328 of slot * 328) into
 * D_8009ECD8 +1..+328 (b1..b3, a[24], c, d[100], e[100], f[100] are
 * contiguous in the leaf's Sv layout).  Returns 0. */
int func_800389DC(int slot)
{
    int v;
    int i;
    int k;
    pe_addr_t buf;

    for (;;) {
        while (func_8006E6A8(
                   (int)PE_LoadU32(0x800B0DD8u) + (int)PE_LoadU16(0x80093176u),
                   PE_LoadU32(0x800B0DD8u + 0x94u),
                   (int)PE_LoadU16(0x80093178u) - (int)PE_LoadU16(0x80093176u)) == -1) {
        }
        while ((v = func_8006E7E8()) != 0) {
            if (v == -1)
                break;
        }
        if (v != -1)
            break;
    }
    buf = PE_LoadU32(0x800B0DD8u + 0x94u);
    k = (unsigned char)slot * 328 + 1;
    for (i = 1; i <= 328; i++)
        PE_StoreU8(0x8009ECD8u + (uint32_t)i, PE_LoadU8(buf + (uint32_t)k++));
    return 0;
}

/* cur_attr(): d = data; return d[d[cur + 29] + 4]. */
static unsigned char lo2_cur_attr(void)
{
    pe_addr_t d = PE_LoadU32(LO2_FONT_DATA);

    return PE_LoadU8(d + PE_LoadU8(d + PE_LoadU8(LO2_FONT_CUR) + 29u) + 4u);
}

/* find_slot(q, sel): first i < q[0] with q[1 + i] == sel, else 255. */
static int lo2_find_slot(pe_addr_t q, unsigned char sel)
{
    int i;

    for (i = 0; i < (int)PE_LoadU8(q); i++) {
        if (PE_LoadU8(q + (uint32_t)i + 1u) == sel)
            return i;
    }
    return 255;
}

/* find_pos(cls): p = data + 1; sel = the first i < p[2] with p[3 + i] ==
 * cls (0 when none); then find_slot(p + 27, sel). */
static int lo2_find_pos(unsigned char cls)
{
    pe_addr_t p = PE_LoadU32(LO2_FONT_DATA) + 1u;
    unsigned char sel = 0;
    int i;

    for (i = 0; i < (int)PE_LoadU8(p + 2u); i++) {
        if (PE_LoadU8(p + (uint32_t)i + 3u) == cls) {
            sel = (unsigned char)i;
            i = PE_LoadU8(p + 2u);
        }
    }
    return lo2_find_slot(p + 27u, sel);
}

/* src/func_80039184.c: page back.  Key < 3 -> key = 1, return 255;
 * otherwise key--, slot = D_8009EE22[key], load it, place the cursor on the
 * first slot of class (key % 10 != 0) and return its attribute. */
unsigned char func_80039184(void)
{
    const pe_addr_t k = 0x80091A1Du;

    if (PE_LoadU8(k) < 3u) {
        PE_StoreU8(k, 1u);
        return 255;
    }
    PE_StoreU8(k, (uint8_t)(PE_LoadU8(k) - 1u));
    PE_StoreU8(0x80091A1Eu, PE_LoadU8(0x8009EE22u + PE_LoadU8(k)));
    func_800389DC(PE_LoadU8(0x80091A1Eu));
    PE_StoreU8(LO2_FONT_CUR,
               (uint8_t)lo2_find_pos((unsigned char)(PE_LoadU8(0x80091A1Du) % 10u != 0)));
    return lo2_cur_attr();
}

/* src/func_80039310.c: page forward.  Key >= 70 -> D_80091A20 = 1, return
 * 255; otherwise key++, slot = D_8009EE22[key], D_80091A20 = 0, load it,
 * cursor on the first class-2 slot, return its attribute. */
unsigned char func_80039310(void)
{
    const pe_addr_t k = 0x80091A1Du;

    if (PE_LoadU8(k) >= 70u) {
        PE_StoreU8(0x80091A20u, 1u);
        return 255;
    }
    PE_StoreU8(k, (uint8_t)(PE_LoadU8(k) + 1u));
    PE_StoreU8(0x80091A1Eu, PE_LoadU8(0x8009EE22u + PE_LoadU8(k)));
    PE_StoreU8(0x80091A20u, 0u);
    func_800389DC(PE_LoadU8(0x80091A1Eu));
    PE_StoreU8(LO2_FONT_CUR, (uint8_t)lo2_find_pos(2));
    return lo2_cur_attr();
}

/* src/func_8003944C.c: jump to page `key`.  `key - 2 >= 69` is a signed int
 * compare (keys 0/1 fall through to the `< 2` case): key >= 71 ->
 * D_80091A20 = 1, return 255; key < 2 -> D_80091A1D = 1, return 255.
 * Otherwise load D_8009EE22[key]; cursor class is (key % 10 != 0) when
 * D_80091A20 == 1, else 3; D_80091A20 = 0; return the attribute. */
unsigned char func_8003944C(unsigned char key)
{
    if ((int)key - 2 >= 69) {
        PE_StoreU8(0x80091A20u, 1u);
        return 255;
    }
    if (key < 2) {
        PE_StoreU8(0x80091A1Du, 1u);
        return 255;
    }
    PE_StoreU8(0x80091A1Du, key);
    PE_StoreU8(0x80091A1Eu, PE_LoadU8(0x8009EE22u + key));
    func_800389DC(PE_LoadU8(0x80091A1Eu));
    if (PE_LoadU8(0x80091A20u) == 1u)
        PE_StoreU8(LO2_FONT_CUR,
                   (uint8_t)lo2_find_pos((unsigned char)(PE_LoadU8(0x80091A1Du) % 10u != 0)));
    else
        PE_StoreU8(LO2_FONT_CUR, (uint8_t)lo2_find_pos(3));
    PE_StoreU8(0x80091A20u, 0u);
    return lo2_cur_attr();
}

/* src/func_80039678.c: cursor step.  Mark-class glyphs (attribute < 2 or
 * 24/31/38/45/52/59) with key 20 page forward (func_80039310); attribute 2
 * with key 22 pages back (func_80039184) — both results narrowed to the
 * leaf's unsigned char return.  Otherwise p = data + 28 and: 20 = previous
 * slot with the same column byte p[101 + i], 21 = cur - 1, 22 = next slot
 * with the same column (i < p[0]), 23 = cur + 1 (cur is a byte). */
unsigned char func_80039678(unsigned char key)
{
    unsigned char c = lo2_cur_attr();
    pe_addr_t p;
    unsigned char v;
    int i;

    if ((c < 2 || c == 24 || c == 31 || c == 38 || c == 45 || c == 52 || c == 59) &&
        key == 20)
        return (unsigned char)func_80039310();
    if (c == 2 && key == 22)
        return (unsigned char)func_80039184();
    p = PE_LoadU32(LO2_FONT_DATA) + 28u;
    switch (key) {
    case 20:
        v = PE_LoadU8(p + PE_LoadU8(LO2_FONT_CUR) + 101u);
        for (i = (int)PE_LoadU8(LO2_FONT_CUR) - 1; i >= 0; i--) {
            if (PE_LoadU8(p + (uint32_t)i + 101u) == v) {
                PE_StoreU8(LO2_FONT_CUR, (uint8_t)i);
                i = 0;
            }
        }
        break;
    case 21:
        PE_StoreU8(LO2_FONT_CUR, (uint8_t)(PE_LoadU8(LO2_FONT_CUR) - 1u));
        break;
    case 22:
        v = PE_LoadU8(p + PE_LoadU8(LO2_FONT_CUR) + 101u);
        for (i = (int)PE_LoadU8(LO2_FONT_CUR) + 1; i < (int)PE_LoadU8(p); i++) {
            if (PE_LoadU8(p + (uint32_t)i + 101u) == v) {
                PE_StoreU8(LO2_FONT_CUR, (uint8_t)i);
                i = PE_LoadU8(p);
            }
        }
        break;
    case 23:
        PE_StoreU8(LO2_FONT_CUR, (uint8_t)(PE_LoadU8(LO2_FONT_CUR) + 1u));
        break;
    }
    return lo2_cur_attr();
}

/* src/func_80039898.c: cursor on the first class-3 slot; its attribute. */
unsigned char func_80039898(void)
{
    PE_StoreU8(LO2_FONT_CUR, (uint8_t)lo2_find_pos(3));
    return lo2_cur_attr();
}

/* src/func_8003E0A4.c: v = ((*a0)[2] == 2) ? 3 : 1 — (*a0) is a guest
 * pointer held at a0+0; *(int *)(a0+0x24) = a1; *(short *)(a0+0x28) = v;
 * *(short *)(a0+0x2A) = a2. */
void func_8003E0A4(pe_addr_t a0, int a1, int a2)
{
    int v = (PE_LoadU8(PE_LoadU32(a0) + 2u) == 2u) ? 3 : 1;

    PE_StoreU32(a0 + 0x24u, (uint32_t)a1);
    PE_StoreU16(a0 + 0x28u, (uint16_t)v);
    PE_StoreU16(a0 + 0x2Au, (uint16_t)a2);
}

/* ── menu windows (0x80045FA4..0x8004ECB4) ────────────────────────────── */

/* src/func_80045FA4.c: item detail panel.  Window records (W) have ints at
 * +0x18 (x) / +0x1C (y).  If window (1,7) is not active: anchor to window
 * (1,6) (else (1,11)); the item is func_800556E8(row of (2,7)) when (1,11)
 * is absent and that row is >= 0, else func_80059F08(0); rec =
 * func_8005332C(item); move self by (w.x - self.x, ((rec[20]+1)>>1)*16 -
 * (self.y - 108)).  Otherwise place it at base (56 in battle, else 36) +
 * row*16 (rows < 9) or base - (self.y - 128).  Then draw the two
 * stat pairs D_800A1888[0]+[2] and [1]+[3]: 2-digit field (func_8005FA3C)
 * when < 100 outside battle, else 3-digit (func_8005FB74) clamped to 999. */
void func_80045FA4(pe_addr_t self)
{
    pe_addr_t w;
    pe_addr_t rec;
    int t;
    int base;
    int h;
    int n;
    int dy;

    if (func_800631C0(func_80062A34(1, 7)) == 0) {
        w = func_80062A34(1, 6);
        if (w == 0)
            w = func_80062A34(1, 11);
        if (func_80062A34(1, 11) == 0 &&
            func_80063428(func_80062A34(2, 7)) >= 0)
            t = func_800556E8(func_80063428(func_80062A34(2, 7)));
        else
            t = func_80059F08(0);
        rec = func_8005332C(t);
        func_80063158(self,
                      (int)PE_LoadU32(w + 0x18u) - (int)PE_LoadU32(self + 0x18u),
                      ((PE_LoadU8(rec + 20u) + 1) >> 1) * 16 -
                          ((int)PE_LoadU32(self + 0x1Cu) - 108));
    } else {
        base = PE_LoadU32(0x8009CF0Cu) ? 56 : 36;
        h = (int)PE_LoadU32(self + 0x1Cu);
        if (func_80054288() < 9)
            dy = base + func_80054288() * 16 - h;
        else
            dy = base - (h - 128);
        func_80063158(self, 0, dy);
    }
    func_8005E8A4(4, 4);
    func_8005F5B8(106);
    func_8005E8A4(60, 0);
    func_8005F5B8(107);
    n = (int)PE_LoadU32(0x800A1888u) + (int)PE_LoadU32(0x800A1890u);
    if (n < 100 && PE_LoadU32(0x8009CF0Cu) == 0) {
        func_8005E8A4(-15, 3);
        func_8005FA3C(n);
    } else {
        func_8005E8A4(-20, 3);
        func_8005FB74(n >= 1000 ? 999 : n);
    }
    n = (int)PE_LoadU32(0x800A188Cu) + (int)PE_LoadU32(0x800A1894u);
    if (n < 100 && PE_LoadU32(0x8009CF0Cu) == 0) {
        func_8005E8A4(50, 0);
        func_8005FA3C(n);
    } else {
        func_8005E8A4(45, 0);
        func_8005FB74(n >= 1000 ? 999 : n);
    }
}

/* src/func_8004E704.c: open the equipment screen.  Reuse window (2,50) or
 * create 50 (+0x2C = func_8004ECB4, list +0x30 = func_8004EBCC, 3 items when
 * mode == 0, focus); create 51 (+0x2C func_80044444, +0x40 = 1; list +0x30
 * func_8004EC3C, +0x84 func_80058FEC, +0x44 = -1); D_8009CF0C = mode ? 2 : 1;
 * select the item view (func_800588EC) and give 51 func_80058C4C(mode ?
 * 0x3803FE : 0xF400) items; create 52 (+0x2C func_80044444, nudged by -6
 * when 51's +0x80 was 0; list +0x30 func_8004EC78, +0x84 func_80058FEC,
 * +0x88 func_80050260, +0x44 = -1) with the view length; D_8009CF94 =
 * D_8009CF8C = -1; link 52 +0x78 = 51, 51 +0x7C = 52; install window 19's
 * +0x30 = func_8004C608 unless (1,19) exists; D_8009CF00 = D_8009CEFC = 0;
 * func_8005DE88().
 * The leaf's `n = func_800588EC(mode)` reads a *void* callee's $v0: retail
 * func_800588EC returns with $v0 = D_8009D080 re-read (pe_dis.sh 0x800588EC
 * 0x1BC: 0x80058A84 `lw v0,784(gp)` before `jr ra`), i.e. the view length. */
void func_8004E704(int mode)
{
    pe_addr_t w;
    pe_addr_t l;
    pe_addr_t m;
    int n;
    int first;

    l = func_80062A34(2, 50);
    if (l == 0) {
        w = func_80062D2C(50, 0, 0, 0);
        l = func_8006322C(50, w, w);
        PE_StoreU32(w + 0x2Cu, 0x8004ECB4u);
        PE_StoreU32(l + 0x30u, 0x8004EBCCu);
        if (mode == 0)
            func_800647D0(l, 3);
        func_80062CB8((int)l);
    }
    w = func_80062D2C(51, l, 0, 0);
    l = func_8006322C(51, w, w);
    PE_StoreU32(w + 0x2Cu, 0x80044444u);
    PE_StoreU32(w + 0x40u, 1u);
    PE_StoreU32(l + 0x30u, 0x8004EC3Cu);
    PE_StoreU32(l + 0x84u, 0x80058FECu);
    PE_StoreU32(l + 0x44u, 0xFFFFFFFFu);
    PE_StoreU32(0x8009CF0Cu, mode != 0 ? 2u : 1u);
    func_800588EC(mode);
    n = (int)PE_LoadU32(0x8009D080u);          /* retail $v0, see above */
    func_80051510();
    func_8005B890(0);
    func_800647D0(l, func_80058C4C(mode != 0 ? 0x3803FEu : 0xF400u));
    first = PE_LoadU32(l + 0x80u) == 0;
    w = func_80062D2C(52, l, 0, 0);
    m = func_8006322C(52, w, w);
    PE_StoreU32(w + 0x2Cu, 0x80044444u);
    if (first)
        func_80063158(w, -6, 0);
    PE_StoreU32(m + 0x30u, 0x8004EC78u);
    PE_StoreU32(m + 0x84u, 0x80058FECu);
    PE_StoreU32(m + 0x88u, 0x80050260u);
    PE_StoreU32(m + 0x44u, 0xFFFFFFFFu);
    PE_StoreU32(0x8009CF94u, 0xFFFFFFFFu);
    PE_StoreU32(0x8009CF8Cu, 0xFFFFFFFFu);
    func_800647D0(m, n);
    PE_StoreU32(m + 0x78u, l);
    PE_StoreU32(l + 0x7Cu, m);
    if (func_80062A34(1, 19) == 0)
        PE_StoreU32(func_80062D2C(19, 0, 0, 0) + 0x30u, 0x8004C608u);
    PE_StoreU32(0x8009CF00u, 0u);
    PE_StoreU32(0x8009CEFCu, 0u);
    func_8005DE88();
}

/* src/func_8004E97C.c: leave the equipment screen.  func_80052EC0();
 * D_8009CF0C = 0.  With a pending selection (D_8009CF98 != 0): close
 * windows; if (1,0) is gone recreate the main menu (window 0: +0x2C
 * func_80043DA4, list +0x30 func_8004F838, focus, item count = popcount of
 * the low 9 bits of D_8009CEF0 & (func_8005B89C() ? 0x1F : 0x1EF),
 * D_8009CEFC = 0, D_8009CEF8 = 1, redraw); close 45/24/18; window 27's
 * +0x30 = func_800447F0; create window 1 under the top window (+0x2C
 * func_80044444; list +0x30 func_8004F8D0, focus, +0x84 func_80057C54,
 * +0x88 func_80050260), refresh, D_8009CF94 = D_8009CF8C = -1, item count
 * func_80052F70(), D_8009CF00 = 0; restore the cursor from D_8009CF98 - 1
 * (+0x44 = bit 0, +0x48 = bits 1..7, +0x5C = v >> 8); unless D_8009D008
 * (then just clear it), store D_8009CF9C into D_800C0E46[D_8009CF98] and
 * refresh; D_8009CF98 = 0.  Without a selection: func_800512AC(9, 0). */
void func_8004E97C(void)
{
    pe_addr_t w;
    pe_addr_t l;
    pe_addr_t top;
    pe_addr_t nw;
    int m;
    int n;
    int i;
    int v;

    func_80052EC0();
    PE_StoreU32(0x8009CF0Cu, 0u);
    if (PE_LoadU32(0x8009CF98u) != 0) {
        func_80062F9C();
        if (func_80062A34(1, 0) == 0) {
            w = func_80062D2C(0, 0, 0, 0);
            l = func_8006322C(0, w, w);
            PE_StoreU32(w + 0x2Cu, 0x80043DA4u);
            PE_StoreU32(l + 0x30u, 0x8004F838u);
            func_80062CB8((int)l);
            if (func_8005B89C() != 0)
                m = (int)PE_LoadU32(0x8009CEF0u) & 0x1F;
            else
                m = (int)PE_LoadU32(0x8009CEF0u) & 0x1EF;
            n = 0;
            for (i = 8; i >= 0; i--) {
                n += m & 1;
                m >>= 1;
            }
            func_800647D0(l, n);
            PE_StoreU32(0x8009CEFCu, 0u);
            PE_StoreU32(0x8009CEF8u, 1u);
            func_800439D8();
            func_8004C594();
        }
        func_80062F3C(45);
        func_80062F3C(24);
        func_80062F3C(18);
        top = (pe_addr_t)func_80062CC4();
        PE_StoreU32(func_80062D2C(27, 0, 0, 0) + 0x30u, 0x800447F0u);
        nw = func_80062D2C(1, top, 0, 0);
        l = func_8006322C(1, nw, nw);
        PE_StoreU32(nw + 0x2Cu, 0x80044444u);
        PE_StoreU32(l + 0x30u, 0x8004F8D0u);
        func_80062CB8((int)l);
        PE_StoreU32(l + 0x84u, 0x80057C54u);
        PE_StoreU32(l + 0x88u, 0x80050260u);
        func_80055760();
        PE_StoreU32(0x8009CF94u, 0xFFFFFFFFu);
        PE_StoreU32(0x8009CF8Cu, 0xFFFFFFFFu);
        func_800647D0(l, (int32_t)func_80052F70());
        PE_StoreU32(0x8009CF00u, 0u);
        if (PE_LoadU32(0x8009CF98u) != 0) {
            v = (int)PE_LoadU32(0x8009CF98u) - 1;
            PE_StoreU32(l + 0x44u, (uint32_t)(v & 1));
            PE_StoreU32(l + 0x48u, (uint32_t)((v >> 1) & 0x7F));
            PE_StoreU32(l + 0x5Cu, (uint32_t)(v >> 8));
        }
        if (PE_LoadU32(0x8009D008u) != 0) {
            PE_StoreU32(0x8009D008u, 0u);
        } else {
            PE_StoreU16(0x800C0E46u + PE_LoadU32(0x8009CF98u) * 2u,
                        (uint16_t)PE_LoadU32(0x8009CF9Cu));
            func_80055760();
        }
        PE_StoreU32(0x8009CF98u, 0u);
    } else {
        func_800512AC(9, 0u);
    }
}

/* src/func_8004ECB4.c: equipment top-menu handler (window 50's +0x2C).
 * On confirm (key & 0x10000) act on the selected row of child 0 — row 2
 * in battle-mode 1 counts as 4: 0 focus list 51 (+0x44 = 0); 1 open the
 * sub-window 59 (+0x2C func_800471E4, list +0x30 func_800471BC), refresh
 * 51, D_8009CFB8 = D_8009CF0C; 2 close 51/52, D_8009CF1C = 1, reset the
 * resource buffer, fill D_800A1888[0..1] from func_80053F20 (14/12, 15/13;
 * 999 when the first is set) and [2..3] likewise from func_80053F90 in
 * battle (else 0), message 830; 3 close 51/52, D_8009CF30 = 1, message
 * 894; 4 leave (func_8004E97C) — each ending with func_800525EC().
 * Cancel (key & 0x40) leaves and calls func_80052634.  Returns 1.
 * func_80048918 (message box) has no pc_port implementation: boundary. */
int func_8004ECB4(pe_addr_t self, int key)
{
    pe_addr_t w;
    pe_addr_t l;
    pe_addr_t n;
    int sel;
    int id;

    w = func_80062A20(self, 0);
    if (key & 0x10000) {
        sel = func_80063428(w);
        switch ((PE_LoadU32(0x8009CF0Cu) == 1 && sel == 2) ? 4 : sel) {
        case 0:
            w = func_80062A34(2, 51);
            PE_StoreU32(w + 0x44u, 0u);
            func_80062CB8((int)w);
            func_800525EC();
            break;
        case 1:
            n = func_80062D2C(59, w, 0, 0);
            l = func_8006322C(59, n, n);
            PE_StoreU32(n + 0x2Cu, 0x800471E4u);
            PE_StoreU32(l + 0x30u, 0x800471BCu);
            func_80062CB8((int)l);
            func_800631AC(func_80062A34(1, 51));
            PE_StoreU32(0x8009CFB8u, PE_LoadU32(0x8009CF0Cu));
            break;
        case 2:
            func_80062F3C(51);
            func_80062F3C(52);
            PE_StoreU32(0x8009CF1Cu, 1u);
            func_80052E30(0);
            PE_StoreU32(0x800A1888u, (uint32_t)(func_80053F20(14) ? 999 : func_80053F20(12)));
            PE_StoreU32(0x800A188Cu, (uint32_t)(func_80053F20(15) ? 999 : func_80053F20(13)));
            if (PE_LoadU32(0x8009CF0Cu) != 0) {
                PE_StoreU32(0x800A1890u, (uint32_t)(func_80053F90(14) ? 999 : func_80053F90(12)));
                PE_StoreU32(0x800A1894u, (uint32_t)(func_80053F90(15) ? 999 : func_80053F90(13)));
            } else {
                PE_StoreU32(0x800A1894u, 0u);
                PE_StoreU32(0x800A1890u, 0u);
            }
            id = 830;
            goto show;
        case 3:
            func_80062F3C(51);
            func_80062F3C(52);
            PE_StoreU32(0x8009CF30u, 1u);
            id = 894;
        show:
            (void)PE_D_COMP_BOUNDARY3("func_80048918", 0x80048918u, id, -1, -1);
            func_800525EC();
            break;
        case 4:
            if (key & 0x10000) {
                func_8004E97C();
                func_800525EC();
            }
            break;
        }
    } else if (key & 0x40) {
        func_8004E97C();
        func_80052634();
    }
    return 1;
}

/* src/func_8005DAFC.c: string table at D_800A8028 + D_800A802C (the leaf
 * steps an int pointer back from &D_800A802C): entry a0 of the u16-counted
 * table -> base + (short)base[2 + 2*a0], a guest address; 0 past the end. */
int func_8005DAFC(unsigned int a0)
{
    pe_addr_t base = 0x800A8028u + PE_LoadU32(0x800A802Cu);

    if (a0 < PE_LoadU16(base))
        return (int)(base + (uint32_t)(int)(short)PE_LoadU16(base + (a0 << 1) + 2u));
    return 0;
}
