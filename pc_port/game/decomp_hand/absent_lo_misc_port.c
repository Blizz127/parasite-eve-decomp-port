/*
 * Hand adapters — screen reset, save-header builder and equip-menu exit
 * (port_absent lane, batch 8).  Conventions as in absent_lo_small_port.c.
 */
#include "pe_guest_decomp.h"

/* Hand ports with no header prototype (pc_port/game/boot/func_8003F3C4_port.c). */
void func_8003DFC8(int a0);
void func_800696F0(void);
/* Decomp-derived callee (pc_port/game/decomp/, no header prototype). */
int func_80043474(int a0);

/* src/func_8003F2FC.c: field-screen reset.  With D_800B0CD8 bit 9, clear the
 * 320x448 frame (the RECT is the host ClearImage argument, same short
 * layout); reset drawing, streams and the scene; then set bit 1 and clear
 * bit 11 of D_800B0CD8 (and, when bit 9 was set, bits 9 and 15 as well),
 * D_8009D1A0 = (D_8009D1A0 | 0x40) & ~0x3800. */
void func_8003F2FC(void)
{
    const pe_addr_t s = 0x800B0CD8u;
    unsigned int x;

    if (PE_LoadU32(s) & 0x200u) {
        RECT buf;
        buf.x = 0;
        buf.y = 0;
        buf.w = 0x140;
        buf.h = 0x1C0;
        (void)func_80074F44(&buf, 0u, 0u, 1u);
    }
    (void)func_80074DC0(0);
    func_80087024();
    func_8003DFC8(1);
    func_800696F0();
    x = PE_LoadU32(s) | 2u;
    PE_StoreU32(0x8009D1A0u, (PE_LoadU32(0x8009D1A0u) | 0x40u) & ~0x3800u);
    PE_StoreU32(s, x & ~0x800u);
    if (x & 0x200u)
        PE_StoreU32(s, (PE_LoadU32(s) | 2u) & ~0x8200u);
}

/* src/func_80040210.c: build the 0x44-byte save header at D_8009EE8C:
 * zero it, store slot a0 and the play time split into h/m/s, then the
 * location name (func_8005DE08(-1) for a fixed title, else the room's
 * name by id) and format it with func_8004006C (hand adapter in
 * e9_guest_ptr_port.c) using the format string whose guest address is held
 * in D_8009222C / D_80092228.  Returns the header's guest address. */
/* func_80040210: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80040210_port.c (src/func_80040210.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_800490B0.c: leave the equip menu — copy the 7 selected halfwords
 * (every other entry of D_800A18D8) into D_800C0E28, commit, close the item
 * windows, refocus and, outside battle (D_8009CF0C == 0), reopen the main
 * menu. */
void func_800490B0(void)
{
    pe_addr_t d = 0x800C0E28u;
    pe_addr_t s = 0x800A18D8u;
    int i = 0;

    do {
        PE_StoreU16(d, PE_LoadU16(s));
        s += 4u;
        i++;
        d += 2u;
    } while (i < 7);
    PE_StoreU32(0x800C0E10u, PE_LoadU32(0x8009CF68u));
    func_8005218C();
    func_80062F3C(0xFu);
    func_80062F3C(0xBu);
    func_80062F3C(0xDu);
    func_80062F3C(0x18u);
    func_80062F3C(0x30u);
    if (PE_LoadU32(0x8009CF0Cu) != 0u) {
        func_80062CB8(func_80062A34(2u, 0x32u));
    } else {
        if (func_80062CC4() != 0u)
            func_80063198(PE_LoadU32(func_80062CC4() + 4u));
        func_80063198(func_80062A34(1u, 0x1Bu));
    }
    PE_StoreU32(0x8009CF34u, 0u);
    PE_StoreU32(0x8009CF30u, 0u);
    if (PE_LoadU32(0x8009CF0Cu) == 0u) {
        func_80062CB8(func_80062A34(2u, 0u));
        func_800439D8();
        PE_StoreU32(0x8009CEF8u, 1u);
    }
}

/* src/func_80049354.c: equip-slot list update.  Confirm (bit 16) opens the
 * slot detail (func_8004A570 with the current cell); cancel (bit 6) closes
 * the item windows, refocuses, copies the 7 selected halfwords back
 * (as func_800490B0), commits and posts message 0x37E through func_80048918
 * (port6_port.c). */
int func_80049354(int a0, int a1)
{
    pe_addr_t p;
    pe_addr_t d;
    pe_addr_t s;
    int i;

    p = func_80062A20((pe_addr_t)a0, 0u);
    if ((a1 & 0x10000) != 0) {
        func_8004A570((int)p, func_80063428(p));
        func_800525EC();
    } else if ((a1 & 0x40) != 0) {
        func_80062F3C(0xFu);
        func_80062F3C(0xBu);
        func_80062F3C(0xDu);
        func_80062F3C(0x18u);
        func_80062F3C(0x30u);
        if (PE_LoadU32(0x8009CF0Cu) != 0u) {
            func_80062CB8(func_80062A34(2u, 0x32u));
        } else {
            if (func_80062CC4() != 0u)
                func_80063198(PE_LoadU32(func_80062CC4() + 4u));
            func_80063198(func_80062A34(1u, 0x1Bu));
        }
        PE_StoreU32(0x8009CF34u, 0u);
        d = 0x800C0E28u;
        i = 0;
        s = 0x800A18D8u;
        do {
            PE_StoreU16(d, PE_LoadU16(s));
            s += 4u;
            i++;
            d += 2u;
        } while (i < 7);
        PE_StoreU32(0x800C0E10u, PE_LoadU32(0x8009CF68u));
        func_80048918(0x37E, -1, -1);
        func_80052634();
    }
    return 1;
}

/* src/func_80042264.c: memory-card load verify.  Snapshot the 0x12E4-byte
 * save block D_8009EFD0 into D_800C0DE0, advance the D_800A0ED0 cursor past
 * it, run func_8003FBD8, then take (and zero) the stored 32-bit checksum
 * word and compare it with ~CRC-16/CCITT (poly 0x1021, init 0xFFFF) over the
 * 0x2000 bytes at D_8009EED0.  Match: func_8005C374, D_800A185C = 1, and the
 * 0x54 "loaded" message with func_80042228 as its callback; mismatch: the
 * 0x55/0x56 error pair.  The CRC is carried in 32 bits exactly as the leaf's
 * `int` (only its low 16 bits are compared); func_8003FBD8 is
 * absent_save_port.c. */
void func_80042264(void)
{
    uint32_t stored;
    pe_addr_t p;
    uint32_t crc;
    unsigned short i;
    unsigned short j;
    const pe_addr_t tab = 0x8009EED0u;
    unsigned k;

    PE_StoreU32(0x800A0ED0u, 0x8009EFD0u);
    for (k = 0; k < 0x12E4u; k += 4u)
        PE_StoreU32(0x800C0DE0u + k, PE_LoadU32(0x8009EFD0u + k));
    PE_StoreU32(0x800A0ED0u, PE_LoadU32(0x800A0ED0u) + 0x12E4u);
    func_8003FBD8();   /* absent_save_port.c */
    crc = 0xFFFFu;
    i = 0;
    p = PE_LoadU32(0x800A0ED0u);
    stored = PE_LoadU32(p);
    PE_StoreU32(0x800A0ED0u, PE_LoadU32(0x800A0ED0u) + 4u);
    PE_StoreU32(p, 0u);
    do {
        crc = crc ^ ((uint32_t)PE_LoadU8(tab + i) << 8);
        j = 0;
        do {
            if ((crc & 0x8000u) != 0u)
                crc = (crc << 1) ^ 0x1021u;
            else
                crc = crc << 1;
            j++;
        } while (j < 8u);
        i++;
    } while (i < 0x2000u);
    if ((~crc & 0xFFFFu) == stored) {
        func_8005C374();
        PE_StoreU32(0x800A185Cu, 1u);
        func_8004D9D8();
        func_8004CC50(0x54u, 0u);
        func_8004D024(0x80042228u);
    } else {
        func_8004D9D8();
        func_8004CE28(0x55u, 0x56u);
    }
}

/* Decomp-derived callees used below (pc_port/game/decomp/, no header). */
int func_800438E0(void);
int func_800527B4(void);
int func_80064A48(void);
void func_80063D30(pe_addr_t pe_a0);

/* src/func_8005C25C.c: snapshot the resumable field state into the save
 * block — six party halfwords (0x20 stride from D_800A1E6E) to D_800C1EAC,
 * then D_800C0E44/40, the two 60-tick timers in seconds, the D_800C0DFF /
 * DFD / DFE option bytes, the room name id (D_800C0E3C) and the current
 * location text id (D_800C0E3E, from func_8005D940's $v0). */
/* func_8005C25C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005C25C_port.c (src/func_8005C25C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_8005C374.c: restore what func_8005C25C saved — party halfwords
 * back to D_800A1E6E (0x20 stride), D_800C0E44 / D_800C0E40 to their
 * owners, timers back to ticks (* 60), the option bytes, then reload the
 * room from D_800C0DE0 (or D_800C0DF0 when D_8009D218 is set). */
/* func_8005C374: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005C374_port.c (src/func_8005C374.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* src/func_800491C8.c: equip-slot detail update.  Bit 14: hide this list,
 * show window 2/0x10 and realign windows 1/0xF, 1/0xB, 1/0x2F exactly as
 * func_80048F24; bit 16: open slot detail func_8004A570(p, cell + 5);
 * bit 6: leave the equip menu (func_800490B0). */
int func_800491C8(pe_addr_t a, int f0)
{
    pe_addr_t p;
    pe_addr_t q;
    int v;
    int flags = f0;
    int t;

    p = func_80062A20(a, 0u);
    if (flags & 0x4000) {
        PE_StoreU32(p + 0x44u, 0xFFFFFFFFu);
        p = func_80062A34(2u, 0x10u);
        PE_StoreU32(p + 0x44u, 0u);
        PE_StoreU32(p + 0x48u, 0u);
        func_80063D30(p);
        func_80062CB8(p);
        p = func_80062A34(1u, 0xFu);
        q = func_80062A20(func_80062A34(1u, 0xDu), 0u);
        v = (int)PE_LoadU32(0x8009CF30u);
        t = (int)PE_LoadU32(p + 0x18u);
        if (v == 0) {
            v = (int)PE_LoadU32(q + 0x80u);
            v = (v == 0) ? 0x9C : 0xA2;
        } else {
            v = 0xB0;
        }
        flags = v - t;
        func_80063158(p, flags, 0);
        func_80063198(p);
        p = func_80062A34(1u, 0xBu);
        func_80063158(p, flags, 0);
        func_80063198(p);
        p = func_80062A34(1u, 0x2Fu);
        func_80063158(p, flags, 0);
        func_80063198(p);
        func_8005267C();
    } else if (flags & 0x10000) {
        func_8004A570((int)p, func_80063428(p) + 5);
        func_800525EC();
    } else {
        v = flags & 0x40;
        if (v != 0) {
            func_800490B0();
            func_80052634();
        }
    }
    return 1;
}

/* Decomp-derived callees used below (pc_port/game/decomp/, no header). */
unsigned int func_8006E3D4(pe_addr_t pe_a0);
int func_80038D0C(void);

/* src/func_80015790.c: pick the message token for a talk event and latch it
 * in D_8009D280.  The event kind k comes from func_80039678(**a0) when
 * func_80038D0C() is set, else from func_80038D74() (neither has a pc_port
 * implementation yet: loud boundaries; their return value is the boundary's
 * 0).  k == 0xFF: D_8009CD78 / D_8009CD80 by func_800392EC() < 2 (after
 * resetting the cursor block); k 6/7/8: D_800917E4, or one of its 8-byte
 * variants chosen by func_80070DD0(1, 4) when func_80070DD0(0, 0x64) >= 0x3C;
 * otherwise the (func_800392EC() - 1) / 10 row of the D_8009173C table
 * selects an 8-byte entry of D_8009180C.  The leaf's `int func_8006E3D4()`
 * K&R prototype is called with that one argument (retail passes it in $a0,
 * pinned `asm("$4")`).  When the token equals D_8009D1C4, set D_8009D1A0
 * bit 13 and D_800B0CD8 bit 11. */
int func_80015790(pe_addr_t a0)
{
    int v;
    int k;
    pe_addr_t arg;

    if ((func_80038D0C() & 0xFF) != 0)
        v = func_80039678(PE_LoadU8(PE_LoadU32(a0)));   /* absent_lo2_port.c */
    else
        v = PE_D_COMP_BOUNDARY0("func_80038D74", 0x80038D74u);
    k = v & 0xFF;
    if (k == 0xFF) {
        if (((unsigned int)func_800392EC() & 0xFFu) < 2u) {
            (void)func_80039970();
            arg = 0x8009CD78u;
        } else {
            (void)func_80039970();
            arg = 0x8009CD80u;
        }
        PE_StoreU32(0x8009D280u, func_8006E3D4(arg));
        return 1;
    }
    if (((unsigned int)(v - 6) & 0xFFu) < 2u || k == 8) {
        if ((short)func_80070DD0(0, 0x64) < 0x3C)
            arg = 0x800917E4u;
        else
            arg = 0x800917E4u + ((uint32_t)(int)(short)func_80070DD0(1, 4) << 3);
    } else {
        unsigned int t = ((unsigned int)func_800392EC() - 1u) & 0xFFu;
        unsigned int q = t / 10u;
        unsigned int m;
        t = q & 0xFFu;
        m = t * 24u;
        arg = ((uint32_t)PE_LoadU8(0x8009173Cu + m + (uint32_t)k) << 3) + 0x8009180Cu;
    }
    PE_StoreU32(0x8009D280u, func_8006E3D4(arg));
    if (PE_LoadU32(0x8009D1C4u) == PE_LoadU32(0x8009D280u)) {
        unsigned int x = PE_LoadU32(0x8009D1A0u);
        unsigned int y = PE_LoadU32(0x800B0CD8u);
        PE_StoreU32(0x8009D1A0u, x | 0x2000u);
        PE_StoreU32(0x800B0CD8u, y | 0x800u);
    }
    return 1;
}
