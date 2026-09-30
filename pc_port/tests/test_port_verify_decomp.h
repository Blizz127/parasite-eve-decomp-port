/*
 * Port verification against the matching decomp (Phases 5FV–5FZ).
 *
 * Each block takes a guest function that now has byte-exact C in `src/` and
 * checks the port's hand-written translation of the *same* guest function against
 * it.  The port's own arithmetic is already covered elsewhere; what is being
 * pinned here is that the decompiled semantics — which list head, which struct
 * offsets, which constant, which buffer — are the ones the port implements.
 *
 * Cross-references:
 *   src/func_80062F3C.c   <-> pc_port/game/boot/func_80062D2C_port.c (func_80062A34)
 *   src/func_800773D0.c   <-> pc_port/game/boot/func_80076C34_port.c
 *   src/func_8005270C.c   <-> pc_port/game/boot/field_message_port.c
 *   src/func_80052764.c   <-> pc_port/game/boot/battle_reward_port.c
 *   src/func_80052C08.c   <-> pc_port/game/boot/func_8004F910_port.c (via func_80052BCC)
 */

static void test_PORTVERIFY_matched_leaves(void)
{
    TEST("PORTVERIFY_matched_leaves");

    /* func_80062F3C (Phase 5FX): decompiled as a walk of the list rooted at
     * D_8009D154 matching +0x20 == 1 and +0x24 == arg, handing the node to
     * func_8006269C — NULL when the list is empty or nothing matches.  The port
     * spells that walk as func_80062A34(kind, id) + func_8006269C, so the helper
     * must reproduce it exactly, including the NULL-on-miss path. */
    {
        pe_addr_t a = 0x80140000u, b = 0x80140100u, c = 0x80140200u;

        ResetTestState();
        PE_StoreU32(0x8009D154u, a);                                 /* head */
        PE_StoreU32(a + 0u, b); PE_StoreU32(b + 0u, c); PE_StoreU32(c + 0u, 0u);
        PE_StoreU32(a + 0x20u, 2u); PE_StoreU32(a + 0x24u, 7u);      /* wrong kind */
        PE_StoreU32(b + 0x20u, 1u); PE_StoreU32(b + 0x24u, 7u);      /* the hit   */
        PE_StoreU32(c + 0x20u, 1u); PE_StoreU32(c + 0x24u, 9u);      /* later id  */

        ASSERT(func_80062A34(1u, 7u) == b, "5FX: kind 1 + id picks the middle node");
        ASSERT(func_80062A34(1u, 9u) == c, "5FX: walk continues past a kind miss");
        ASSERT(func_80062A34(2u, 7u) == a, "5FX: kind participates in the match");
        ASSERT(func_80062A34(1u, 8u) == 0u, "5FX: no match yields NULL");

        PE_StoreU32(0x8009D154u, 0u);                                /* empty list */
        ASSERT(func_80062A34(1u, 7u) == 0u, "5FX: empty list yields NULL");
    }

    /* func_800773D0 (Phase 5FZ): decompiled as D_80095888 <- clock + 0xF0 with
     * D_8009588C cleared and the deadline left in v0. */
    {
        uint32_t first, second;

        ResetTestState();
        PE_StoreU32(0x80095888u, 0xDEADBEEFu);
        PE_StoreU32(0x8009588Cu, 0x12345678u);

        first = func_800773D0();
        ASSERT(PE_LoadU32(0x80095888u) == first, "5FZ: deadline published at 0x80095888");
        ASSERT(PE_LoadU32(0x8009588Cu) == 0u, "5FZ: poll counter cleared at 0x8009588C");
        ASSERT(first == PE_GPU_VSyncQuery() + 0xF0u, "5FZ: deadline is clock + 0xF0");

        second = func_800773D0();
        ASSERT(PE_LoadU32(0x80095888u) == second, "5FZ: re-arm republishes");
        ASSERT(second >= first, "5FZ: deadline does not go backwards");
    }

    /* func_8005270C / func_80052764 (Phases 5FY / 5FW): publish the field-message
     * fade target in D_8009D01C, then stop it and clear it.  With no sound package
     * the published value is 0. */
    {
        ResetTestState();
        PE_StoreU32(0x800B0E08u, 0u);
        PE_StoreU32(0x8009D01Cu, 0xCAFEF00Du);

        ASSERT(func_8005270C() == 0, "5FY: no package publishes 0");
        ASSERT(PE_LoadU32(0x8009D01Cu) == 0u, "5FY: target overwritten with 0");

        func_80052764();
        ASSERT(PE_LoadU32(0x8009D01Cu) == 0u, "5FW: stop leaves the cleared target");
    }

    /* func_80052C08 (Phase 5FZ): append a 0xFF-terminated byte string.  Retail
     * inlines the copy loop; the port factors it into func_80052BCC, so what is
     * checked is the observable buffer. */
    {
        unsigned i;

        ResetTestState();
        for (i = 0u; i < 8u; i++) PE_StoreU8(0x80141000u + i, 0xFFu);
        PE_StoreU8(0x80141000u, 0x58u);
        PE_StoreU8(0x80141001u, 0xFFu);
        PE_StoreU8(0x80141100u, 0x41u);
        PE_StoreU8(0x80141101u, 0x42u);
        PE_StoreU8(0x80141102u, 0xFFu);
        PE_StoreU8(0x80141103u, 0x00u);

        func_80052C08(0x80141000u, 0x80141100u);

        ASSERT(PE_LoadU8(0x80141000u) == 0x58u, "5FZ: existing byte preserved");
        ASSERT(PE_LoadU8(0x80141001u) == 0x41u, "5FZ: append starts at the terminator");
        ASSERT(PE_LoadU8(0x80141002u) == 0x42u, "5FZ: second byte copied");
        ASSERT(PE_LoadU8(0x80141003u) == 0xFFu, "5FZ: terminator copied");
        ASSERT(PE_LoadU8(0x80141004u) == 0xFFu, "5FZ: copy stops at the terminator");
    }

    /* func_8005E8C4 / func_8005E914 (Phase 5GA): the arena push/pop pair.
     *
     * The decompiled C also calls func_800527C0(2) on push overflow and (3) on pop
     * underflow — argument values that were not previously known.  The retail
     * callee is an empty `jr $ra; nop` stub (src/func_800527C0.c), and the port
     * elides calls to it consistently (see the "pool exhausted (an empty stub)"
     * note in func_8005ED18_port.c), so the elision here is behaviourally nil.
     * What must hold is the cursor arithmetic, the bounds test and the word order. */
    {
        ResetTestState();
        PE_StoreU32(0x8009D12Cu, 0x800A2280u);      /* inside the arena window */
        PE_StoreU32(0x8009D124u, 0x11111111u);
        PE_StoreU32(0x8009D128u, 0x22222222u);

        func_8005E8C4();
        ASSERT(PE_LoadU32(0x8009D12Cu) == 0x800A2288u, "5GA: push advances the cursor by 8");
        ASSERT(PE_LoadU32(0x800A2280u) == 0x11111111u, "5GA: push stores the first word");
        ASSERT(PE_LoadU32(0x800A2284u) == 0x22222222u, "5GA: push stores the second word");

        PE_StoreU32(0x8009D124u, 0u);
        PE_StoreU32(0x8009D128u, 0u);
        func_8005E914();
        ASSERT(PE_LoadU32(0x8009D12Cu) == 0x800A2280u, "5GA: pop steps the cursor back by 8");
        ASSERT(PE_LoadU32(0x8009D124u) == 0x11111111u, "5GA: pop republishes the first word");
        ASSERT(PE_LoadU32(0x8009D128u) == 0x22222222u, "5GA: pop republishes the second word");

        /* Bounds: push at the arena end and pop at the arena start are no-ops. */
        PE_StoreU32(0x8009D12Cu, 0x800A22B0u);
        func_8005E8C4();
        ASSERT(PE_LoadU32(0x8009D12Cu) == 0x800A22B0u, "5GA: push at the arena end does not advance");
        PE_StoreU32(0x8009D12Cu, 0x800A2270u);
        func_8005E914();
        ASSERT(PE_LoadU32(0x8009D12Cu) == 0x800A2270u, "5GA: pop at the arena start does not retreat");
    }

    /* func_800C6EF8 / func_800C6F4C (Phase 5GB): mirror word-array transfers
     * between a mesh record and the fixed buffer D_800E2370.
     *
     * The decompiled C fixes three things the port must match: the source pointer
     * is `record + *(u16 *)(record + 8)` (a byte offset in the +8 halfword), the
     * count is `*(u16 *)(record + 0xA)` and is re-read on *every* iteration, and the
     * transfer is 32-bit word granular.  Retail's `lhu`-then-`slt` comparison and
     * the `blez` guard behave as unsigned for a 16-bit count, so there is no
     * signed/unsigned trap at count >= 0x8000 for either implementation. */
    {
        pe_addr_t mesh = 0x80142000u;       /* record */
        pe_addr_t colors = 0x80142020u;     /* mesh + 0x20 */

        ResetTestState();
        PE_StoreU16(mesh + 8u, 0x20u);      /* byte offset to the colour array */
        PE_StoreU16(mesh + 0xAu, 3u);       /* word count */
        PE_StoreU32(colors + 0u, 0xAAAAAAAAu);
        PE_StoreU32(colors + 4u, 0xBBBBBBBBu);
        PE_StoreU32(colors + 8u, 0xCCCCCCCCu);
        PE_StoreU32(0x800E2370u, 0u);

        func_800C6EF8(mesh);                            /* mesh -> buffer */
        ASSERT(PE_LoadU32(0x800E2370u) == 0xAAAAAAAAu, "5GB: 6EF8 copies word 0 to the buffer");
        ASSERT(PE_LoadU32(0x800E2374u) == 0xBBBBBBBBu, "5GB: 6EF8 copies word 1");
        ASSERT(PE_LoadU32(0x800E2378u) == 0xCCCCCCCCu, "5GB: 6EF8 copies word 2");
        ASSERT(PE_LoadU32(0x800E237Cu) == 0u, "5GB: 6EF8 stops at the count");

        PE_StoreU32(0x800E2370u, 0x11111111u);
        PE_StoreU32(0x800E2374u, 0x22222222u);
        PE_StoreU32(0x800E2378u, 0x33333333u);
        func_800C6F4C(mesh);                            /* buffer -> mesh */
        ASSERT(PE_LoadU32(colors + 0u) == 0x11111111u, "5GB: 6F4C copies word 0 back");
        ASSERT(PE_LoadU32(colors + 4u) == 0x22222222u, "5GB: 6F4C copies word 1 back");
        ASSERT(PE_LoadU32(colors + 8u) == 0x33333333u, "5GB: 6F4C copies word 2 back");

        /* A zero count must transfer nothing in either direction. */
        PE_StoreU16(mesh + 0xAu, 0u);
        PE_StoreU32(colors + 0u, 0xDEADBEEFu);
        PE_StoreU32(0x800E2370u, 0x5A5A5A5Au);
        func_800C6EF8(mesh);
        ASSERT(PE_LoadU32(0x800E2370u) == 0x5A5A5A5Au, "5GB: 6EF8 with count 0 is a no-op");
        func_800C6F4C(mesh);
        ASSERT(PE_LoadU32(colors + 0u) == 0xDEADBEEFu, "5GB: 6F4C with count 0 is a no-op");
    }

    /* func_8005DC9C (src/func_8005DC9C.c; now also called directly by the
     * func_8005D6F4 port's D_8009D0C8 != 0 arm): archive base is
     * &D_800A8028 + D_800A802C; the table sits at base + *(int *)(base+8);
     * an unsigned index >= the u16 count at table+0 returns NULL, otherwise
     * table + (short)entry[index] with entries starting at table+2. */
    {
        const pe_addr_t base = 0x80140400u, table = base + 0x40u;

        ResetTestState();
        PE_StoreU32(0x800A802Cu, base - 0x800A8028u);
        PE_StoreU32(base + 8u, 0x40u);
        PE_StoreU16(table + 0u, 3u);                 /* count */
        PE_StoreU16(table + 2u, 0x0010u);
        PE_StoreU16(table + 4u, (uint16_t)-8);       /* signed entry */
        PE_StoreU16(table + 6u, 0x0020u);

        ASSERT(func_8005DC9C(0u) == table + 0x10u, "DC9C: entry 0 is table-relative");
        ASSERT(func_8005DC9C(1u) == table - 8u, "DC9C: entries are signed shorts");
        ASSERT(func_8005DC9C(2u) == table + 0x20u, "DC9C: last in-range entry");
        ASSERT(func_8005DC9C(3u) == 0u, "DC9C: index == count yields NULL");
        ASSERT(func_8005DC9C(0xFFFFFFFFu) == 0u, "DC9C: bound is unsigned");
    }

    PASS();
}
