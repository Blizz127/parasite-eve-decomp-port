/*
 * Phase 6E-B54K-I — func_80030894: complete boot GPU-primitive builder.
 *
 * Full retail body:
 *   788 words / 0xC50 bytes, exe 0x80030894–0x800314E4 (exclusive),
 *   file offset 0x21094, window SHA-256
 *   a4dbd2cf130979a0f5db8ed532d0c559c5b3b90b2c5786e10fe91125311ed6e2.
 *   Sole caller: jal func_8006AD40 @ 0x8006B0AC.  ABI void(void)
 *   (backward liveness fixpoint in the B54J audit; $v0 at exit is a
 *   stale scratch discarded by the caller).
 *
 * Complete native body:
 *   0x80030894..0x800314E4 (788 words) — prologue, the bank record
 *   init at 0x800BE9F0, and the L2(j=0..9) × L3(k=0..3) sprite-array
 *   build at 0x800B01C0, followed by the fixed bank-0 tile/sprite setup
 *   complete L4/L5 loops, fixed G4/sprite records, the complete L6 loop,
 *   the following fixed primitive records, complete L7/L8/L9/L10 loops,
 *   the fixed sprite plus complete descriptor-driven L11 loop, the final
 *   16x16 sprite, and the outer bank loop for i=0,1.  The retail epilogue
 *   restores the frame and returns at 0x800314DC/0x800314E0; the next word
 *   at 0x800314E4 is func_800314E4's independent prologue.
 *
 * Word decode of the implemented window (verified against the
 * SHA-1-exact retail executable 452fb033f2eaa4b18aa20a5bca60b8125af3a37b):
 *
 *   prologue (0x80030894..0x8003090C):
 *     frame 88B; ra@84, fp@80, s7@76 .. s0@48 (sp-relative)
 *     sp+16/17/18 <- sign-extended lb 0x8009CD90+0/+1/+2 (font triple;
 *                    dead in this window — no reader before 0x80030AC4)
 *     fp  <- GetTPage(0,1,0x100,0x1E0) & 0xFFFF = 0x34   (delay slot of
 *            the GetClut jal at 0x800308FC consumes the GetTPage return)
 *     s7  <- 0x80 (vertex byte; first used by B54K-B groups)
 *     sp+32 (half) <- GetClut(0x130,0x1F8) = 0x7E13
 *     sp+24 (byte) <- 0 (outer bank counter i; ++ @0x80031484 and
 *                     test <2 @0x800314A8 live in B54K-B's epilogue)
 *
 *   bank record init (0x80030910..0x80030A14), per bank i (i = 0 here):
 *     s3  <- func_8005DADC(139)  == *(u32*)0x800A8030 + 0x800A8028 + 1112
 *     s0  <- 0x800BE9F0 + i*40  (record table; the audit's corrected
 *            lui 0x800C / addiu -0x1610 resolution)
 *     func_80077BA4(s0)                       SetPolyFT4 header
 *     sb s0+0x0C <- lbu(s3+0)                  u1
 *     sb s0+0x0D <- lbu(s3+1)                  v1
 *     sb s0+0x14 <- (lbu(s3+0)+lbu(s3+4))&0xFF u2
 *     sb s0+0x15 <- lbu(s3+1)                  v2
 *     sb s0+0x1C <- lbu(s3+0)                  u3
 *     sb s0+0x1D <- (lbu(s3+1)+lbu(s3+5))&0xFF v3
 *     sb s0+0x24 <- (lbu(s3+0)+lbu(s3+4))&0xFF u4
 *     sb s0+0x25 <- (lbu(s3+1)+lbu(s3+5))&0xFF v4 (delay slot of the
 *            second GetTPage jal — stores the PRE-call adder value)
 *     sh s0+0x16 <- GetTPage(0,0,0x1C0,0)
 *     sh s0+0x0E <- lhu(s3+2)                  clut id
 *     a1 <- 1 at 0x800309D8 — AFTER the second GetTPage call (which
 *           clobbers a1 to 0) and BEFORE SetSemiTrans, so the retail
 *           argument is abr=1
 *     sh zero -> s0+8,+0xA,+0x10,+0x12,+0x18,+0x1A,+0x20,+0x22
 *     sb zero -> s0+4,+5,+6
 *     func_80077B04(s0, 1)                    SetSemiTrans ON
 *
 *   L2/L3 sprite array (0x80030A18..0x80030AC0):
 *     s5 <- i*1400  (((i*3*4 - i)*16 - i)*8, instruction-exact)
 *     s6 <- 0 (j), s3 <- 0 (k)
 *     L3 body: s1 <- j*140 (((j*8+j)*4-j)*4)
 *              s0 <- k*28  (((k*8-k))*4... == (k*8-k)*4)
 *              a0 <- 0x800B01C0 + s5 + s1 + s0
 *              sp+40 word <- i*12 (v1 save; restored after the call;
 *                              host-stack only, not guest-observable)
 *              func_800370DC(a0, fp)          wrap_sprt twin
 *              sh 0x7E13 -> 0x800B0000 + (s5+s1+s0) + 0x1D6
 *                         == packet + 0x16   clut halfword
 *     L3: k < 4 (sltiu), L2: j < 10 (sltiu), counters &0xFF.
 *
 * All callees are native since B54I/GPU1 (GetTPage, GetClut,
 * SetPolyFT4, SetSemiTrans, func_8005DADC, func_800370DC,
 * func_80077C64, func_80077B64); there is no untranslated boundary inside
 * this body.
 *
 * Classification: 1 — complete translated retail function.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

#define GA_8009CD90      0x8009CD90u
#define GA_RECORD_TABLE  0x800BE9F0u
#define GA_SPRITE_BASE   0x800B01C0u
#define GA_TILE_BASE     0x8009E068u
#define GA_TILE_PACKET   0x8009E098u
#define GA_TILE_STATE    0x800B00E8u
#define GA_SPRT_RECORD   0x800B6920u
#define GA_L4_BASE       0x8009E0F0u
#define GA_L5_BASE       0x8009E1D0u
#define GA_G4_PAIR_BASE  0x800B0130u
#define GA_FIXED_SPRT_A  0x8009E0B8u
#define GA_FIXED_SPRT_B  0x8009E2E8u
#define GA_FIXED_SPRT_C  0x8009E320u
#define GA_L6_BASE       0x8009E358u
#define GA_POST_L6_SPRT  0x8009E460u
#define GA_DIRECT_SPRT   0x8009E498u
#define GA_POLY_F3       0x8009E4D8u
#define GA_L7_BASE       0x8009E3B8u
#define GA_L8_BASE       0x8009E500u
#define GA_POST_L8_SPRT  0x8009E768u
#define GA_L9_BASE       0x8009E7A0u
#define GA_POST_L9_SPRT  0x8009E730u
#define GA_SECOND_SPRT   0x8009E880u
#define GA_L10_BASE      0x8009E8B8u
#define GA_POST_L10_SPRT 0x8009E928u
#define GA_L11_BASE      0x8009E960u
#define GA_FINAL_SPRT    0x8009EC38u

void func_80030894(void)
{
    uint32_t tpage_sprt;   /* s8: sprite tpage mode for wrap_sprt */
    uint16_t clut_id;      /* sp+32 half */
    uint8_t bank = 0u;     /* sp+24 byte: outer bank counter */
    uint8_t tile_colors[3]; /* sp+16..18: entry-time font-byte snapshot */

    /* Retail snapshots these bytes before any call and later reloads them
     * unsigned in L6. Preserve that timing, not merely their final values. */
    tile_colors[0] = PE_LoadU8(GA_8009CD90 + 0u);
    tile_colors[1] = PE_LoadU8(GA_8009CD90 + 1u);
    tile_colors[2] = PE_LoadU8(GA_8009CD90 + 2u);

    /* 0x800308E0..0x8003090C prologue vectors. */
    tpage_sprt = func_80077A64(0u, 1u, 0x100u, 0x1E0u) & 0xFFFFu;
    clut_id = (uint16_t)func_80077AA4(0x130, 0x1F8);

    /* 0x80030910.. bank record init (banks 0 and 1). */
    do {
        pe_addr_t rec = func_8005DADC(139u);            /* s3 */
        pe_addr_t rec_tab = GA_RECORD_TABLE +
                            (pe_addr_t)((uint32_t)bank * 40u); /* s0 */
        uint32_t u0v0_sum;                               /* delay-slot adder */
        uint32_t s5;                                     /* i*1400 */
        uint32_t j, k;

        func_80077BA4(rec_tab);                          /* SetPolyFT4 */

        PE_StoreU8(rec_tab + 0x0Cu, PE_LoadU8(rec + 0u));
        PE_StoreU8(rec_tab + 0x0Du, PE_LoadU8(rec + 1u));
        PE_StoreU8(rec_tab + 0x14u, (uint8_t)(
                   (uint32_t)PE_LoadU8(rec + 0u) + (uint32_t)PE_LoadU8(rec + 4u)));
        PE_StoreU8(rec_tab + 0x15u, PE_LoadU8(rec + 1u));
        PE_StoreU8(rec_tab + 0x1Cu, PE_LoadU8(rec + 0u));
        PE_StoreU8(rec_tab + 0x1Du, (uint8_t)(
                   (uint32_t)PE_LoadU8(rec + 1u) + (uint32_t)PE_LoadU8(rec + 5u)));
        PE_StoreU8(rec_tab + 0x24u, (uint8_t)(
                   (uint32_t)PE_LoadU8(rec + 0u) + (uint32_t)PE_LoadU8(rec + 4u)));
        u0v0_sum = (uint32_t)PE_LoadU8(rec + 1u) +
                   (uint32_t)PE_LoadU8(rec + 5u);
        PE_StoreU8(rec_tab + 0x25u, (uint8_t)u0v0_sum);  /* delay-slot store */

        PE_StoreU16(rec_tab + 0x16u,
                    (uint16_t)func_80077A64(0u, 0u, 0x1C0u, 0u));
        PE_StoreU16(rec_tab + 0x0Eu, PE_LoadU16(rec + 2u));

        /* 0x800309E8..0x80030A0C: zero the xy halfword pairs and the
         * u0/v0/clut-low bytes. */
        PE_StoreU16(rec_tab + 0x08u, 0u);
        PE_StoreU16(rec_tab + 0x0Au, 0u);
        PE_StoreU16(rec_tab + 0x10u, 0u);
        PE_StoreU16(rec_tab + 0x12u, 0u);
        PE_StoreU16(rec_tab + 0x18u, 0u);
        PE_StoreU16(rec_tab + 0x1Au, 0u);
        PE_StoreU16(rec_tab + 0x20u, 0u);
        PE_StoreU16(rec_tab + 0x22u, 0u);
        PE_StoreU8(rec_tab + 0x04u, 0u);
        PE_StoreU8(rec_tab + 0x05u, 0u);
        PE_StoreU8(rec_tab + 0x06u, 0u);
        func_80077B04(rec_tab, 1u);                      /* SetSemiTrans(.,1) */

        /* 0x80030A18..0x80030AC0: L2 (j<10) x L3 (k<4) sprite array. */
        s5 = (uint32_t)bank * 1400u;
        for (j = 0u; j < 10u; j++) {                     /* s6 */
            for (k = 0u; k < 4u; k++) {                  /* s3 */
                uint32_t off = s5 + j * 140u + k * 28u;

                func_800370DC(GA_SPRITE_BASE + off, tpage_sprt);
                /* 0x80030AA0: sh clut -> 0x800B0000 + off + 0x1D6. */
                PE_StoreU16(0x800B0000u + off + 0x1D6u, clut_id);
            }
        }

        /* 0x80030AC4..0x80030C3C: bank-local fixed packet setup before
         * L4.  Every address is the literal retail base plus the decoded
         * bank stride; bank remains zero until the outer-loop epilogue. */
        {
            uint32_t tile_tpage =
                func_80077A64(0u, 0u, 0u, 0u) & 0xFFFFu;
            uint32_t bank24 = (uint32_t)bank * 24u;
            uint32_t bank16 = (uint32_t)bank * 16u;
            uint32_t bank36 = (uint32_t)bank * 36u;
            uint32_t bank28 = (uint32_t)bank * 28u;
            uint32_t bank112 = (uint32_t)bank * 112u;
            uint32_t bank140 = (uint32_t)bank * 140u;
            uint32_t bank72 = (uint32_t)bank * 72u;
            uint32_t bank48 = (uint32_t)bank * 48u;
            uint32_t bank32 = (uint32_t)bank * 32u;
            uint32_t bank20 = (uint32_t)bank * 20u;
            uint32_t bank84 = (uint32_t)bank * 84u;
            uint32_t bank280 = (uint32_t)bank * 280u;
            uint32_t bank56 = (uint32_t)bank * 56u;
            uint32_t bank364 = (uint32_t)bank * 364u;
            pe_addr_t tile = GA_TILE_BASE + bank24;
            pe_addr_t tile_packet = tile + 8u;
            pe_addr_t packet = GA_TILE_PACKET + bank16;
            pe_addr_t state = GA_TILE_STATE + bank36;
            pe_addr_t sprt = GA_SPRT_RECORD + bank28;
            pe_addr_t sprt_tail = sprt + 8u;
            uint32_t slot;

            func_80037140(tile, tile_tpage);
            PE_StoreU8(tile_packet + 4u, 0x30u);
            PE_StoreU8(tile_packet + 5u, 0x30u);
            PE_StoreU8(tile_packet + 6u, 0x30u);
            func_80077B04(tile_packet, 1u);

            func_80077C44(packet);
            PE_StoreU8(packet + 4u, 0x1Du);
            PE_StoreU8(packet + 5u, 0x3Eu);
            PE_StoreU8(packet + 6u, 0x32u);
            PE_StoreU16(packet + 0x0Cu, 0x38u);
            PE_StoreU16(packet + 0x0Eu, 3u);
            /* 30B50 passes s0 (the gauge), not s1 (its background). */
            func_80077BC4(state);

            func_800370DC(sprt, tpage_sprt);
            PE_StoreU8(sprt_tail + 0x0Cu, 0xC8u);
            PE_StoreU8(sprt_tail + 0x0Du, 0xE0u);
            PE_StoreU16(sprt + 0x16u, clut_id);
            PE_StoreU16(sprt_tail + 0x10u, 4u);
            PE_StoreU16(sprt_tail + 0x12u, 8u);

            PE_StoreU8(state + 0x04u, 0u);
            PE_StoreU8(state + 0x05u, 0x46u);
            PE_StoreU8(state + 0x06u, 0x82u);
            PE_StoreU8(state + 0x0Cu, 0x9Fu);
            PE_StoreU8(state + 0x0Du, 0xFFu);
            PE_StoreU8(state + 0x0Eu, 0xF9u);
            PE_StoreU8(state + 0x14u, 0u);
            PE_StoreU8(state + 0x15u, 0x46u);
            PE_StoreU8(state + 0x16u, 0x82u);
            PE_StoreU8(state + 0x1Cu, 0x9Fu);
            PE_StoreU8(state + 0x1Du, 0xFFu);
            PE_StoreU8(state + 0x1Eu, 0xF9u);
            PE_StoreU8(sprt_tail + 4u, 0x9Fu);
            PE_StoreU8(sprt_tail + 5u, 0xFFu);
            PE_StoreU8(sprt_tail + 6u, 0xF9u);

            /* 0x80030C44..0x80030C98: L4, four 28-byte packets. */
            for (slot = 0u; slot < 4u; slot++) {
                uint32_t off = bank112 + slot * 28u;
                pe_addr_t l4 = GA_L4_BASE + off;

                func_800370DC(l4, tpage_sprt);
                PE_StoreU16(l4 + 0x16u, clut_id);
                PE_StoreU16(l4 + 0x18u, 6u);
                PE_StoreU16(l4 + 0x1Au, 10u);
            }

            /* 0x80030C9C..0x80030D1C: L5, five 28-byte packets. */
            for (slot = 0u; slot < 5u; slot++) {
                uint32_t off = bank140 + slot * 28u;
                pe_addr_t l5 = GA_L5_BASE + off;

                func_800370DC(l5, tpage_sprt);
                PE_StoreU16(l5 + 0x16u, clut_id);
                PE_StoreU16(l5 + 0x18u, 6u);
                PE_StoreU16(l5 + 0x1Au, 10u);
            }

            /* 0x80030D20..0x80030F28: two fixed G4 records, three
             * fixed sprite records, and the register setup consumed by
             * L6.  The retail records are unrolled and remain so here. */
            {
                pe_addr_t g4_a = GA_G4_PAIR_BASE + bank72;
                pe_addr_t g4_b = g4_a + 36u;
                pe_addr_t sprt_a = GA_FIXED_SPRT_A + bank28;
                pe_addr_t sprt_b = GA_FIXED_SPRT_B + bank28;
                pe_addr_t sprt_c = GA_FIXED_SPRT_C + bank28;
                pe_addr_t tail;

                func_80077BC4(g4_a);
                func_80077BC4(g4_b);

                PE_StoreU8(g4_a + 0x04u, 0u);
                PE_StoreU8(g4_a + 0x05u, 0x82u);
                PE_StoreU8(g4_a + 0x06u, 0x36u);
                PE_StoreU8(g4_a + 0x0Cu, 0x4Au);
                PE_StoreU8(g4_a + 0x0Du, 0xFFu);
                PE_StoreU8(g4_a + 0x0Eu, 0x3Bu);
                PE_StoreU8(g4_a + 0x14u, 0u);
                PE_StoreU8(g4_a + 0x15u, 0x82u);
                PE_StoreU8(g4_a + 0x16u, 0x36u);
                PE_StoreU8(g4_a + 0x1Cu, 0x4Au);
                PE_StoreU8(g4_a + 0x1Du, 0xFFu);
                PE_StoreU8(g4_a + 0x1Eu, 0x3Bu);

                PE_StoreU8(g4_b + 0x04u, 0xFFu);
                PE_StoreU8(g4_b + 0x05u, 0x3Du);
                PE_StoreU8(g4_b + 0x06u, 0x81u);
                PE_StoreU8(g4_b + 0x0Cu, 0x83u);
                PE_StoreU8(g4_b + 0x0Du, 0x13u);
                PE_StoreU8(g4_b + 0x0Eu, 1u);
                PE_StoreU8(g4_b + 0x14u, 0xFFu);
                PE_StoreU8(g4_b + 0x15u, 0x3Du);
                PE_StoreU8(g4_b + 0x16u, 0x81u);
                PE_StoreU8(g4_b + 0x1Cu, 0x83u);
                PE_StoreU8(g4_b + 0x1Du, 0x13u);
                PE_StoreU8(g4_b + 0x1Eu, 1u);

                func_800370DC(sprt_a, tpage_sprt);
                tail = sprt_a + 8u;
                func_80077B34(tail, 1u);
                PE_StoreU8(tail + 0x04u, 0x80u);
                PE_StoreU8(tail + 0x05u, 0x80u);
                PE_StoreU8(tail + 0x06u, 0x80u);
                PE_StoreU8(tail + 0x0Cu, 0x50u);
                PE_StoreU8(tail + 0x0Du, 0xF4u);
                PE_StoreU16(sprt_a + 0x16u, clut_id);
                PE_StoreU16(tail + 0x10u, 8u);
                PE_StoreU16(tail + 0x12u, 4u);

                func_800370DC(sprt_b, tpage_sprt);
                tail = sprt_b + 8u;
                func_80077B34(tail, 1u);
                PE_StoreU8(tail + 0x04u, 0x80u);
                PE_StoreU8(tail + 0x05u, 0x80u);
                PE_StoreU8(tail + 0x06u, 0x80u);
                PE_StoreU8(tail + 0x0Cu, 0x58u);
                PE_StoreU8(tail + 0x0Du, 0xF4u);
                PE_StoreU16(sprt_b + 0x16u, clut_id);
                PE_StoreU16(tail + 0x10u, 8u);
                PE_StoreU16(tail + 0x12u, 4u);

                func_800370DC(sprt_c, tpage_sprt);
                tail = sprt_c + 8u;
                func_80077B34(tail, 1u);
                PE_StoreU8(tail + 0x04u, 0x80u);
                PE_StoreU8(tail + 0x05u, 0x80u);
                PE_StoreU8(tail + 0x06u, 0x80u);
                PE_StoreU8(tail + 0x0Cu, 0x60u);
                PE_StoreU8(tail + 0x0Du, 0xF4u);
                PE_StoreU16(sprt_c + 0x16u, clut_id);
                PE_StoreU16(tail + 0x10u, 8u);
                PE_StoreU16(tail + 0x12u, 4u);
            }

            /* 0x80030F2C..0x80030F68: L6, three direct SetTile packets.
             * Each packet receives one of the three font bytes loaded by
             * the prologue, replicated across RGB. */
            for (slot = 0u; slot < 3u; slot++) {
                pe_addr_t tile = GA_L6_BASE + bank48 + slot * 16u;
                uint8_t color = tile_colors[slot];

                func_80077C44(tile);
                PE_StoreU8(tile + 0x04u, color);
                PE_StoreU8(tile + 0x05u, color);
                PE_StoreU8(tile + 0x06u, color);
            }

            /* 0x80030F6C..0x8003103C: fixed primitives before L7. */
            {
                pe_addr_t sprt = GA_POST_L6_SPRT + bank28;
                pe_addr_t tail = sprt + 8u;
                pe_addr_t direct_a = GA_DIRECT_SPRT + bank32;
                pe_addr_t direct_b = direct_a + 16u;
                pe_addr_t poly_f3 = GA_POLY_F3 + bank20;

                func_800370DC(sprt, tpage_sprt);
                PE_StoreU8(tail + 0x0Cu, 0xE8u);
                PE_StoreU8(tail + 0x0Du, 0xE0u);
                PE_StoreU16(sprt + 0x16u, clut_id);
                PE_StoreU16(tail + 0x10u, 24u);
                PE_StoreU16(tail + 0x12u, 24u);

                func_80077C64(direct_a);
                func_80077C64(direct_b);
                PE_StoreU8(direct_a + 0x04u, 0xE0u);
                PE_StoreU8(direct_a + 0x05u, 0xE0u);
                PE_StoreU8(direct_a + 0x06u, 0xE0u);
                PE_StoreU8(direct_b + 0x04u, 0x60u);
                PE_StoreU8(direct_b + 0x05u, 0x60u);
                PE_StoreU8(direct_b + 0x06u, 0x60u);
                func_80077B64(poly_f3);
            }

            /* 0x80031040..0x800310A0: L7, three 28-byte sprites. */
            for (slot = 0u; slot < 3u; slot++) {
                pe_addr_t l7 = GA_L7_BASE + bank84 + slot * 28u;

                func_800370DC(l7, tpage_sprt);
                PE_StoreU8(l7 + 0x0Cu, 0x80u);
                PE_StoreU8(l7 + 0x0Du, 0x80u);
                PE_StoreU8(l7 + 0x0Eu, 0x80u);
                PE_StoreU16(l7 + 0x16u, clut_id);
                PE_StoreU16(l7 + 0x18u, 24u);
                PE_StoreU16(l7 + 0x1Au, 8u);
            }

            /* 0x800310A4..0x8003110C: L8, ten 28-byte sprites. */
            for (slot = 0u; slot < 10u; slot++) {
                pe_addr_t l8 = GA_L8_BASE + bank280 + slot * 28u;

                func_800370DC(l8, tpage_sprt);
                PE_StoreU8(l8 + 0x0Cu, 0x80u);
                PE_StoreU8(l8 + 0x0Du, 0x80u);
                PE_StoreU8(l8 + 0x0Eu, 0x80u);
            }

            /* 0x80031110..0x80031188: fixed sprite before L9. */
            {
                pe_addr_t sprt = GA_POST_L8_SPRT + bank28;

                func_800370DC(sprt, tpage_sprt);
                PE_StoreU8(sprt + 0x0Cu, 0x80u);
                PE_StoreU8(sprt + 0x0Du, 0x80u);
                PE_StoreU8(sprt + 0x0Eu, 0x80u);
                PE_StoreU8(sprt + 0x14u, 0x58u);
                PE_StoreU8(sprt + 0x15u, 0xEFu);
                PE_StoreU16(sprt + 0x16u, clut_id);
                PE_StoreU16(sprt + 0x18u, 36u);
                PE_StoreU16(sprt + 0x1Au, 5u);
            }

            /* 0x8003118C..0x800311E8: L9, four 28-byte sprites. */
            for (slot = 0u; slot < 4u; slot++) {
                pe_addr_t l9 = GA_L9_BASE + bank112 + slot * 28u;

                func_800370DC(l9, tpage_sprt);
                PE_StoreU8(l9 + 0x0Cu, 0x80u);
                PE_StoreU8(l9 + 0x0Du, 0x80u);
                PE_StoreU8(l9 + 0x0Eu, 0x80u);
                PE_StoreU16(l9 + 0x16u, clut_id);
                PE_StoreU16(l9 + 0x18u, 6u);
                PE_StoreU16(l9 + 0x1Au, 6u);
            }

            /* 0x800311EC..0x800312C8: two fixed sprites before L10. */
            {
                pe_addr_t first = GA_POST_L9_SPRT + bank28;
                pe_addr_t second = GA_SECOND_SPRT + bank28;
                uint16_t clut_1f9;

                func_800370DC(first, tpage_sprt);
                PE_StoreU8(first + 0x14u, 0x68u);
                PE_StoreU8(first + 0x15u, 0xF4u);
                clut_1f9 = (uint16_t)func_80077AA4(0x130, 0x1F9);
                PE_StoreU16(first + 0x16u, clut_1f9);
                PE_StoreU16(first + 0x18u, 24u);
                PE_StoreU16(first + 0x1Au, 4u);
                PE_StoreU8(first + 0x0Cu, 0x80u);
                PE_StoreU8(first + 0x0Du, 0x80u);
                PE_StoreU8(first + 0x0Eu, 0x80u);

                func_800370DC(second, tpage_sprt);
                PE_StoreU8(second + 0x0Cu, 0x80u);
                PE_StoreU8(second + 0x0Du, 0x80u);
                PE_StoreU8(second + 0x0Eu, 0x80u);
                PE_StoreU8(second + 0x14u, 0x7Cu);
                PE_StoreU8(second + 0x15u, 0xEFu);
                PE_StoreU16(second + 0x16u, clut_id);
                PE_StoreU16(second + 0x18u, 36u);
                PE_StoreU16(second + 0x1Au, 5u);
            }

            /* 0x800312CC..0x8003131C: L10, two 28-byte sprites. */
            for (slot = 0u; slot < 2u; slot++) {
                pe_addr_t l10 = GA_L10_BASE + bank56 + slot * 28u;

                func_800370DC(l10, tpage_sprt);
                PE_StoreU16(l10 + 0x16u, clut_id);
                PE_StoreU16(l10 + 0x18u, 6u);
                PE_StoreU16(l10 + 0x1Au, 6u);
            }

            /* 0x80031320..0x80031390: fixed sprite before L11. */
            {
                pe_addr_t sprt = GA_POST_L10_SPRT + bank28;

                func_800370DC(sprt, tpage_sprt);
                PE_StoreU8(sprt + 0x0Cu, 0x80u);
                PE_StoreU8(sprt + 0x0Du, 0x80u);
                PE_StoreU8(sprt + 0x0Eu, 0u);
                PE_StoreU16(sprt + 0x16u, clut_id);
            }

            /* 0x80031394..0x80031434: L11, 13 descriptor sprites. */
            for (slot = 0u; slot < 13u; slot++) {
                pe_addr_t rec = func_8005DADC(slot + 0x6Au);
                uint32_t mode =
                    func_80077A64(0u, 0u, 0x1C0u, 0u) & 0xFFFFu;
                pe_addr_t l11 = GA_L11_BASE + bank364 + slot * 28u;

                func_800370DC(l11, mode);
                PE_StoreU8(l11 + 0x0Cu, 0x80u);
                PE_StoreU8(l11 + 0x0Du, 0x80u);
                PE_StoreU8(l11 + 0x0Eu, 0x80u);
                PE_StoreU8(l11 + 0x14u, PE_LoadU8(rec + 0u));
                PE_StoreU8(l11 + 0x15u, PE_LoadU8(rec + 1u));
                PE_StoreU16(l11 + 0x16u, PE_LoadU16(rec + 2u));
                PE_StoreU16(l11 + 0x18u, PE_LoadU8(rec + 4u));
                PE_StoreU16(l11 + 0x1Au, PE_LoadU8(rec + 5u));
            }
        }

        /* 0x80031438..0x8003149C: final 16x16 sprite for this bank. */
        {
            uint32_t mode =
                func_80077A64(0u, 0u, 0x1C0u, 0u) & 0xFFFFu;
            pe_addr_t sprt = GA_FINAL_SPRT + (uint32_t)bank * 28u;

            func_800370DC(sprt, mode);
            PE_StoreU16(sprt + 0x18u, 16u);
            PE_StoreU16(sprt + 0x1Au, 16u);
            PE_StoreU8(sprt + 0x0Cu, 0x80u);
            PE_StoreU8(sprt + 0x0Du, 0x80u);
            PE_StoreU8(sprt + 0x0Eu, 0x80u);
        }

        bank = (uint8_t)(bank + 1u);
    } while (bank < 2u);
}
