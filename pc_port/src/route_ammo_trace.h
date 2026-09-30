/* Read-only route trace (PE_ROUTE_AMMO_TRACE=1): Aya's weapon/ammo state and
 * enemy HP, printed on change. No guest writes. Self-contained so it can be
 * dropped into older trees for differential runs. Expects g_frame. */
static void RouteAmmoTrace(void)
{
    static int on = -1;
    static uint32_t last_sig = 0xFFFFFFFFu;
    static uint32_t last_ehp[8];
    pe_addr_t rec, gun;
    uint32_t loaded, wid, sig, p0, p1, p2;
    if (on < 0) on = getenv("PE_ROUTE_AMMO_TRACE") != NULL;
    {
        /* Battle-edge log (always on under --route-pad): the anchors for the
         * event-anchored pad clock.  EDGE when D1A0 bit 0x802 or mode changes. */
        static uint32_t lb = 0xFFFFFFFFu, lm = 0xFFFFFFFFu;
        uint32_t b = PE_LoadU32(0x8009D1A0u) & 0x802u, m = PE_LoadU32(0x8009D28Cu);
        if (b != lb || (b && m != lm)) {
            fprintf(stderr, "BATTLE_EDGE %d token=%08X d1a0&802=%X mode=%u\n",
                    g_frame, (unsigned)PE_LoadU32(0x8009D280u), b, m);
        }
        lb = b; lm = m;
    }
    if (!on) return;
    rec = PE_LoadU32(0x8009D278u);
    gun = rec ? PE_LoadU32(rec + 0x68u) : 0u;
    loaded = gun ? (PE_LoadU32(gun + 12u) & 0x3FFu) : 0xFFFu;
    wid = gun ? PE_LoadU16(gun + 6u) : 0xFFFFu;
    p0 = PE_LoadU16(0x800A1E6Eu); p1 = PE_LoadU16(0x800A1E8Eu); p2 = PE_LoadU16(0x800A1EAEu);
    sig = (loaded << 20) ^ (wid << 8) ^ (p0 << 1) ^ (p1 << 11) ^ (p2 << 25) ^ gun;
    if (sig != last_sig) {
        fprintf(stderr, "AMMO %d token=%08X mode=%u gun=%08X wid=%u loaded=%u w0C=%08X "
                "w10=%08X w14=%04X pools=%u,%u,%u d03c=%u hp=%u atb=%u\n",
                g_frame, (unsigned)PE_LoadU32(0x8009D280u), PE_LoadU32(0x8009D28Cu),
                (unsigned)gun, wid, loaded, gun ? PE_LoadU32(gun + 12u) : 0u,
                gun ? PE_LoadU32(gun + 16u) : 0u, gun ? PE_LoadU16(gun + 20u) : 0u,
                p0, p1, p2, PE_LoadU32(0x8009D03Cu),
                rec ? PE_LoadU16(rec + 12u) : 0u, rec ? PE_LoadU16(rec + 16u) : 0u);
        last_sig = sig;
    }
    {
        pe_addr_t aya = PE_LoadU32(0x8009D254u);
        for (unsigned i = 0; i < 8u; i++) {
            pe_addr_t p = PE_LoadU32(0x8009E000u + i * 12u), body;
            uint32_t hp;
            if (!p) { last_ehp[i] = 0; continue; }
            if (p == aya || !PE_RangeIsRam(p, 640u)) continue;
            body = PE_LoadU32(p);
            if (!PE_RangeIsRam(body, 24u)) continue;
            hp = PE_LoadU32(body + 0x10u);
            if (hp != last_ehp[i]) {
                fprintf(stderr, "EHP %d slot=%u type=%u hp=%d (was %d) def8C=%u loaded=%u\n",
                        g_frame, i, PE_LoadU8(p + 12u), (int32_t)hp, (int32_t)last_ehp[i],
                        PE_LoadU32(body + 0x8Cu), loaded);
                last_ehp[i] = hp;
            }
        }
    }
}
