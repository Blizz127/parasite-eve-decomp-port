/*
 * Phase 6E-B54K-M — complete func_8006AD40.
 *
 * Full retail body:
 *   391 words / 1564 bytes, exe 0x8006AD40–0x8006B35C (exclusive),
 *   file offset 0x5B540.
 *
 * Implemented body:
 *   391 words / 1564 bytes, exe 0x8006AD40–0x8006B35C (exclusive).
 *   It includes the complete func_80030894 call, the D_800930F0 wait,
 *   the D_800930E0 issue/three-lookups/completion group, the following
 *   D_80093126 issue/E0-entry-walk/completion group, the final +0x188
 *   archive walk, F1/display synchronization, and retail state reset.
 *
 * func_8006E1C0 is TRANSLATED (Phase 6E-B51), and B52 translates its two
 * func_8007506C (Psy-Q LoadImage) wrappers through the read-only validator.
 * B53I-D completes the accepted two-LoadImage lifecycle for entry 0. B54A
 * proves that 0x8006AE50 is mid-loop rather than a dependency boundary, so
 * B54B issues the remaining entries through the same translated helper and
 * stops at the loop exit. If the initial entry count is zero, retail bypasses
 * the loop and reaches that same 0x8006AE68 boundary directly.
 *
 * Classification: 1 — complete translated retail function. All completion polls are
 * consumed live; poll/s2 are not assigned. Host D_8009B6B4
 * collapse is synchronous-provider timing, not planted retail state.
 * B54K-M walks the completed +0x188 archive, issues stream command 0xF1,
 * performs the retail display synchronization, resets the selected state
 * fields, clears D_800B0CD8 bit 0, and returns normally.  No bootstrap
 * boundary remains inside this function.
 */
#include "psx_compat.h"
#include "game_port.h"

#define GA_D_800930EA  0x800930EAu
#define GA_D_800930EC  0x800930ECu
#define GA_D_800930EE  0x800930EEu
#define GA_D_800930F0  0x800930F0u
#define GA_D_800930E0  0x800930E0u
#define GA_D_80093126  0x80093126u
#define GA_D_80091648  0x80091648u
#define GA_D_800B0CD8  0x800B0CD8u
#define GA_D_800B0DD8  0x800B0DD8u

extern int func_8006E6A8(int lba, pe_addr_t dest, int sectors);
extern int func_8006E7E8(void);
extern pe_addr_t func_800718D0(pe_addr_t tim);
extern void func_80030894(void);
extern void func_80087024(void);
extern int func_80074A44(int mode);

/* Shared GetTPage/GetClut pack used by B54D records 2/3 (a1=0x20,0x30)
 * and by the B54C font sites at 0x8006AFF8 / 0x8006B02C (a1=0,0x10). */
static void pack_d80091648(uint32_t record_offset)
{
    uint32_t src_a = PE_LoadU16(GA_D_80091648 + record_offset);
    uint32_t src_b = PE_LoadU16(GA_D_80091648 + record_offset + 2u);
    uint32_t src_c = PE_LoadU16(GA_D_80091648 + record_offset + 4u);
    uint32_t src_d = PE_LoadU16(GA_D_80091648 + record_offset + 6u);
    uint32_t packed1 = ((src_a & 0x3FFu) >> 6) | 0x20u;
    uint32_t packed2;

    packed1 |= (src_b & 0x100u) >> 4;
    packed1 |= (src_b & 0x200u) << 2;
    packed2 = (src_d << 6) | ((src_c >> 4) & 0x3Fu);
    PE_StoreU16(GA_D_80091648 + record_offset + 8u, (uint16_t)packed1);
    PE_StoreU16(GA_D_80091648 + record_offset + 0xAu, (uint16_t)packed2);
}

void PE_func_8006AD40_PackFontRecords(void)
{
    uint32_t record_offset;

    /* 0x8006AFB0..0x8006B038: a1 = 0, 0x10; a1 < 0x20. */
    for (record_offset = 0u; record_offset < 0x20u; record_offset += 0x10u)
        pack_d80091648(record_offset);
}

int func_8006AD40(void)
{
    pe_addr_t lba_base;
    int status;

    /* 0x8006AD6C–0x8006AD7C: guarded zero return. */
    if (!(PE_LoadU32(GA_D_800B0CD8) & 1u))
        return 0;

    lba_base = PE_LoadU32(GA_D_800B0DD8);

    /* 0x8006AD88–0x8006ADD0: channel 1 issue/poll loop. */
    for (;;) {
        do {
            uint32_t start = PE_LoadU16(GA_D_800930EA);
            uint32_t end = PE_LoadU16(GA_D_800930EA + 2u);
            status = func_8006E6A8(
                (int)(lba_base + start),
                PE_LoadU32(GA_D_800B0CD8 + 0x160u),
                (int)(end - start));
        } while (status == -1);

        status = func_8006E7E8();
        if (status == 0)
            break;
        if (status != -1) {
            do {
                status = func_8006E7E8();
            } while (status != 0 && status != -1);
            if (status == 0)
                break;
        }
    }

    /* 0x8006ADD8–0x8006AE00: channel 2 issue loop. */
    do {
        uint32_t start = PE_LoadU16(GA_D_800930EC);
        uint32_t end = PE_LoadU16(GA_D_800930EC + 2u);
        status = func_8006E6A8(
            (int)(lba_base + start),
            PE_LoadU32(GA_D_800B0CD8 + 0x174u),
            (int)(end - start));
    } while (status == -1);

    /* 0x8006AE10–0x8006AE64: complete first counted texture-entry loop.
     * The header is at base + lw(base+4) + 0x28, not base + 0x28. */
    {
        pe_addr_t base = PE_LoadU32(GA_D_800B0CD8 + 0x160u);
        pe_addr_t metadata = base + PE_LoadU32(base + 4u);
        uint32_t header = PE_LoadU32(metadata + 0x28u);
        uint32_t count = header >> 22;
        pe_addr_t entry = base + (header & 0x003FFFFFu);
        uint32_t issued = 0u;

        if (count != 0u) {
            for (;;) {
                uint32_t continue_loop;

                func_8006E1C0(entry, base);        /* 0x8006AE48 */
                header = PE_LoadU32(metadata + 0x28u); /* 0x8006AE50 */
                issued += 1u;                      /* 0x8006AE54 */
                count = header >> 22;              /* 0x8006AE58 */
                continue_loop = issued < count;    /* sltu 0x8006AE5C */
                entry += 0x14u;                    /* delay 0x8006AE64 */
                if (!continue_loop)
                    break;
            }
        }
    }

    /* 0x8006AE68..0x8006AEF4: pack records 2 and 3 only. */
    {
        uint32_t record_offset;

        for (record_offset = 0x20u; record_offset < 0x40u;
             record_offset += 0x10u)
            pack_d80091648(record_offset);
    }

    /* 0x8006AEF8..0x8006AF50: the existing archive lookup and one-or-more
     * size-prefixed LoadImage records. Canonical Disc 1 has exactly one. */
    {
        pe_addr_t base = PE_LoadU32(GA_D_800B0CD8 + 0x160u);
        pe_addr_t record = func_8006E498(base, 0xABADC06Cu);

        while (PE_LoadU32(record) != 0u) {
            RECT rect;
            uint32_t size = PE_LoadU32(record);

            rect.x = (int16_t)PE_LoadU16(record + 4u);
            rect.y = (int16_t)PE_LoadU16(record + 6u);
            rect.w = (int16_t)PE_LoadU16(record + 8u);
            rect.h = (int16_t)PE_LoadU16(record + 0xAu);
            (void)func_8007506C(&rect, record + 0xCu);
            record += size & ~3u;
        }
    }

    /* 0x8006AF44..0x8006AF68: wait/reissue around live func_8006E7E8.
     * Canonical entry locals are s0=1 (AF44) and s2=1 (AE04). AE08 with
     * s0!=0 is the wait head, not the texture loop. Do not assign s2. */
    status = 1;
    for (;;) {
        if (status == -1) {
            /* 0x8006ADD8: reissue the same channel-2 range. */
            do {
                uint32_t start = PE_LoadU16(GA_D_800930EC);
                uint32_t end = PE_LoadU16(GA_D_800930EC + 2u);
                status = func_8006E6A8(
                    (int)(lba_base + start),
                    PE_LoadU32(GA_D_800B0CD8 + 0x174u),
                    (int)(end - start));
            } while (status == -1);
            status = 1; /* 0x8006AE04 */
        }
        status = func_8006E7E8(); /* 0x8006AF54 — live result */
        if (status == 0)
            break;
        /* 0x8006AE08: s0==1 branches to 0x8006AF4C. */
    }

    /* 0x8006AF68..0x8006B04C: issue D_800930EE / dest+0x180, walk the
     * already-complete TIM at dest+0x174, pack records 0 and 1. Canonical
     * s0 is 0 here so 718D0 runs. s2 is set to 1 after a successful issue,
     * so the B044 s2==-1 reissue is not taken. Do not poll at B04C. */
    {
        int s0 = 0; /* 0x8006AF68 */

        do {
            uint32_t start = PE_LoadU16(GA_D_800930EE);
            uint32_t end = PE_LoadU16(GA_D_800930EE + 2u);

            status = func_8006E6A8(
                (int)(lba_base + start),
                PE_LoadU32(GA_D_800B0CD8 + 0x180u),
                (int)(end - start));
        } while (status == -1);

        status = 1; /* 0x8006AF98 */
        if (s0 == 0) {
            (void)func_800718D0(PE_LoadU32(GA_D_800B0CD8 + 0x174u));
            PE_func_8006AD40_PackFontRecords();
            s0 = 1; /* 0x8006B040 */
        }
        /* 0x8006B044 beq s2, -1, AF6C — not taken (status == 1). */
        (void)s0;
    }

    /* 0x8006B04C..0x8006B060: wait/reissue around live func_8006E7E8
     * for the D_800930EE issue. Canonical locals are s0=1 (B040) and
     * s2=1 (AF98). AF9C with s0!=0 is the wait head (B044), not 718D0.
     * Do not assign s2. Do not issue D_800930F0. */
    for (;;) {
        if (status == -1) {
            /* 0x8006AF6C: reissue the same D_800930EE range. */
            do {
                uint32_t start = PE_LoadU16(GA_D_800930EE);
                uint32_t end = PE_LoadU16(GA_D_800930EE + 2u);
                status = func_8006E6A8(
                    (int)(lba_base + start),
                    PE_LoadU32(GA_D_800B0CD8 + 0x180u),
                    (int)(end - start));
            } while (status == -1);
            status = 1; /* 0x8006AF98 */
        }
        status = func_8006E7E8(); /* 0x8006B04C — live result */
        if (status == 0)
            break;
        /* 0x8006AF9C: s0==1 branches to 0x8006B044. */
    }

    /* B54K-A: issue D_800930F0 into dest+0x14C. The busy bits stay armed
     * across the complete 30894 call. 718D0 walks the prior 930EE dest at
     * +0x180 (zero TIM on the prefix path). */
    do {
        uint32_t start = PE_LoadU16(GA_D_800930F0);
        uint32_t end = PE_LoadU16(GA_D_800930F0 + 2u);

        status = func_8006E6A8(
            (int)(lba_base + start),
            PE_LoadU32(GA_D_800B0CD8 + 0x14Cu),
            (int)(end - start));
    } while (status == -1);
    status = 1; /* 0x8006B090 */
    (void)func_800718D0(PE_LoadU32(GA_D_800B0CD8 + 0x180u));

    func_80030894();

    /* B54K-J: 0x8006B0B4..0x8006B0D0. A timeout branches back only to
     * the D_800930F0 issue at 0x8006B064; s0 remains 1, so neither 718D0
     * nor func_80030894 is repeated. Positive polls revisit B0B4, while
     * zero clears s0 at B0D0 and advances to the next descriptor group. */
    for (;;) {
        if (status == -1) {
            do {
                uint32_t start = PE_LoadU16(GA_D_800930F0);
                uint32_t end = PE_LoadU16(GA_D_800930F0 + 2u);

                status = func_8006E6A8(
                    (int)(lba_base + start),
                    PE_LoadU32(GA_D_800B0CD8 + 0x14Cu),
                    (int)(end - start));
            } while (status == -1);
            status = 1; /* 0x8006B090 */
        }
        status = func_8006E7E8(); /* 0x8006B0BC — live result */
        if (status == 0)
            break;
        /* 0x8006B098: s0==1 returns to the B0B4 status gate. */
    }

    /* B54K-K: 0x8006B0D4..0x8006B168. Start the E0 range into +0x16C,
     * resolve three keys from the completed F0 archive at +0x14C exactly
     * once, then consume the E0 completion. A timeout reissues only E0;
     * positive polls retain the lookup-done state and poll again. */
    {
        int lookups_done = 0; /* retail s0 cleared at 0x8006B0D0 */

        do {
            uint32_t start = PE_LoadU16(GA_D_800930E0);
            uint32_t end = PE_LoadU16(GA_D_800930E0 + 2u);

            status = func_8006E6A8(
                (int)(lba_base + start),
                PE_LoadU32(GA_D_800B0CD8 + 0x16Cu),
                (int)(end - start));
        } while (status == -1);
        status = 1; /* 0x8006B100 */

        for (;;) {
            if (!lookups_done) {
                pe_addr_t base = PE_LoadU32(GA_D_800B0CD8 + 0x14Cu);

                lookups_done = 1; /* first jal delay slot, 0x8006B11C */
                PE_StoreU32(GA_D_800B0CD8 + 0x11Cu,
                            func_8006E498(base, 0xC4B5BA04u));
                PE_StoreU32(GA_D_800B0CD8 + 0x120u,
                            func_8006E498(base, 0xCAAD0704u));
                PE_StoreU32(GA_D_800B0CD8 + 0x124u,
                            func_8006E498(base, 0x5EAF6804u));
            }
            if (status == -1) {
                do {
                    uint32_t start = PE_LoadU16(GA_D_800930E0);
                    uint32_t end = PE_LoadU16(GA_D_800930E0 + 2u);

                    status = func_8006E6A8(
                        (int)(lba_base + start),
                        PE_LoadU32(GA_D_800B0CD8 + 0x16Cu),
                        (int)(end - start));
                } while (status == -1);
                status = 1; /* 0x8006B100 */
            }
            status = func_8006E7E8(); /* 0x8006B154 — live result */
            if (status == 0)
                break;
        }
    }

    /* B54K-L: 0x8006B16C..0x8006B21C. Issue the first D_80093126 range
     * into +0x188, walk every 0x14-byte entry in the completed E0 archive
     * at +0x16C once, then consume completion. Timeout reissues only the
     * 3126 range; positive polls skip the already-complete entry walk. */
    {
        int entries_done = 0; /* retail s0 cleared at 0x8006B168 */

        do {
            uint32_t start = PE_LoadU16(GA_D_80093126);
            uint32_t end = PE_LoadU16(GA_D_80093126 + 2u);

            status = func_8006E6A8(
                (int)(lba_base + start),
                PE_LoadU32(GA_D_800B0CD8 + 0x188u),
                (int)(end - start));
        } while (status == -1);
        status = 1; /* 0x8006B198 */

        for (;;) {
            if (!entries_done) {
                pe_addr_t base = PE_LoadU32(GA_D_800B0CD8 + 0x16Cu);
                pe_addr_t metadata = base + PE_LoadU32(base + 4u);
                uint32_t header = PE_LoadU32(metadata + 0x28u);
                uint32_t count = header >> 22;
                pe_addr_t entry = base + (header & 0x003FFFFFu);
                uint32_t i;

                for (i = 0u; i < count; i++, entry += 0x14u)
                    (void)func_8006E1C0(entry, base);
                entries_done = 1; /* 0x8006B1FC */
            }
            if (status == -1) {
                do {
                    uint32_t start = PE_LoadU16(GA_D_80093126);
                    uint32_t end = PE_LoadU16(GA_D_80093126 + 2u);

                    status = func_8006E6A8(
                        (int)(lba_base + start),
                        PE_LoadU32(GA_D_800B0CD8 + 0x188u),
                        (int)(end - start));
                } while (status == -1);
                status = 1; /* 0x8006B198 */
            }
            status = func_8006E7E8(); /* 0x8006B20C — live result */
            if (status == 0)
                break;
        }
    }

    /* B54K-M: 0x8006B220..0x8006B270.  The completed +0x188 archive
     * has the same packed count/offset header and 0x14-byte entry stride
     * as the earlier texture groups.  A zero count bypasses the walk. */
    {
        pe_addr_t base = PE_LoadU32(GA_D_800B0CD8 + 0x188u);
        pe_addr_t metadata = base + PE_LoadU32(base + 4u);
        uint32_t header = PE_LoadU32(metadata + 0x28u);
        uint32_t count = header >> 22;
        pe_addr_t entry = base + (header & 0x003FFFFFu);
        uint32_t i;

        for (i = 0u; i < count; i++, entry += 0x14u)
            (void)func_8006E1C0(entry, base);
    }

    /* 0x8006B274..0x8006B2BC: stream F1 and retail display sync. */
    func_80087024();
    func_80074DC0(0);
    (void)func_80074A44(1);
    func_80073A44(0);
    /* Retail (asm/disc1/5B1E4.s:633-635) indexes the guest pair by
     * D_8009CDDC (*20 via sll/addu/sll), not by 0x800ACDDC: the old
     * host-pointer form read a wild index silently, the guest-address
     * form needs the retail index. */
    func_800755F0(0x800BCE80u + PE_LoadU32(0x8009CDDCu) * 20u);
    func_80074D28(1);

    /* 0x8006B2C0..0x8006B32C: exact-width state reset.  The flag tests
     * are live reloads in retail; bit 0 is cleared last. */
    PE_StoreU16(GA_D_800B0CD8 + 0x06u, 0xFFFFu);
    PE_StoreU8(GA_D_800B0CD8 + 0x0Bu, 0u);
    PE_StoreU8(GA_D_800B0CD8 + 0x0Cu, 0xFFu);
    PE_StoreU8(GA_D_800B0CD8 + 0x09u, 0xFFu);
    PE_StoreU16(GA_D_800B0CD8 + 0xE8u, 0xFFFFu);
    PE_StoreU8(GA_D_800B0CD8 + 0xEBu, 0u);
    PE_StoreU8(GA_D_800B0CD8 + 0xEAu, 0u);
    if ((PE_LoadU32(GA_D_800B0CD8) & 0x40u) == 0u) {
        PE_StoreU8(GA_D_800B0CD8 + 0xDAu, 0xFFu);
        PE_StoreU8(GA_D_800B0CD8 + 0xDDu, 0xFFu);
        PE_StoreU8(GA_D_800B0CD8 + 0xDCu, 0xFFu);
    }
    if ((PE_LoadU32(GA_D_800B0CD8) & 0x80u) == 0u) {
        PE_StoreU8(GA_D_800B0CD8 + 0xDBu, 0xFFu);
        PE_StoreU8(GA_D_800B0CD8 + 0xDFu, 0xFFu);
        PE_StoreU8(GA_D_800B0CD8 + 0xDEu, 0xFFu);
    }
    PE_StoreU32(GA_D_800B0CD8,
                PE_LoadU32(GA_D_800B0CD8) & 0xFFFFFFFEu);
    return 0;
}
