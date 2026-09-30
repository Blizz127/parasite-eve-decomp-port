/*
 * Phase 6E-A batch 3 — func_800698D4 (disc mount / PE.IMG search),
 * rewired to the retail sequence (asm/disc1/55430.s:5581).
 *
 * Retail contract:
 *   D_800B0DCD = 0
 *   if CdReady() != 1        → return CdReady status
 *   if queue() != 0          → return 1
 *   r = func_80082314()      → r==1: return 1; r==4: proceed; else return -1
 *   wait drive idle, then:
 *     if DsSearchFile("\FMV1\PEDISC01.IDF;1") found:
 *       wait idle; if DsSearchFile("\PE.IMG;1") found:
 *         D_800B0DD8 = CdPosToInt(fp); D_800B0DCD |= 1
 *     wait idle; if DsSearchFile("\FMV2\PEDISC02.IDF;1") found:
 *       wait idle; if DsSearchFile("\PE.IMG;1") found:
 *         D_800B0DD8 = CdPosToInt(fp); D_800B0DCD |= 2
 *   return D_800B0DCD == 0 ? -2 : 0
 * (DsSearchFile results 0 and -1 are both treated as "not found".)
 *
 * The name strings are the retail rodata bytes at D_80011330/D_80011348/
 * D_80011354 (verified against the retail EXE).  FMV2\PEDISC02.IDF is
 * absent from retail Disc 1, so only bit 1 is set on the real disc.
 *
 * The retail fp is a stack CdlFILE (sp+0x10); the host uses the documented
 * guest scratch address PE_698D4_CDLFILE (0x801FFEC0, 24 bytes), the same
 * pattern as func_8006E834's environment scratch.
 *
 * Wait-idle loops are transcribed verbatim; with the synchronous host
 * drive model the drive is idle after reset and after each completed
 * read, so the bodies never spin.
 *
 * --bootstrap-disc remains an explicit test fixture: it keeps the canned
 * BOOTSTRAP_RET sequence (strict mode still aborts centrally at its first
 * invocation) and reports "mounted" (0), the retail success value.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "stub_registry.h"
#include "pe_bootstrap.h"
#include <string.h>

/* ── Globals ───────────────────────────────────────────────────────── */
/* D_800B0DCD / D_800B0DD8 are guest-RAM lvalue macros (psx_compat.h). */

/* Guest scratch CdlFILE for the DsSearchFile results (documented). */
#define PE_698D4_CDLFILE  0x801FFEC0u

/* Retail rodata name strings (retail EXE file offset 0x1B30). */
static const char PE_NAME_DISC1_IDF[] = "\\FMV1\\PEDISC01.IDF;1"; /* D_80011330 */
static const char PE_NAME_PE_IMG[]    = "\\PE.IMG;1";             /* D_80011348 */
static const char PE_NAME_DISC2_IDF[] = "\\FMV2\\PEDISC02.IDF;1"; /* D_80011354 */

/* Retail wait-idle: while (CdReady()!=1 || queue()!=0) VSync(0). */
static void PE_698D4_WaitIdle(void)
{
    for (;;) {
        if (func_8007F72C() == 1 && func_8007F778() == 0) break;
        func_80073A44(0);
    }
}

int func_800698D4(void)
{
    int s0, r;

    if (g_bootstrap_disc) {
        /* Explicit test fixture (not the default definition of successful
         * real-disc operation).  Returns the retail "mounted" value 0.
         *
         * Seed a minimal valid archive at D_800A8028 so that when
         * func_8006A9E4 cycle B calls func_800527C8 → func_8005D6F4 →
         * func_8005DC4C, the lookup returns a valid guest pointer instead
         * of 0 (which would dereference address 0 and abort).  On real
         * hardware this data arrives from PE.IMG via cycle A of
         * func_8006A9E4; the bootstrap-disc fixture establishes the same
         * precondition directly.
         *
         * Layout: R=0x30, S=0x14, count=120, entry[30] → lone 0xFF
         * record.  Matches the measured Disc 1 USA archive shape and the
         * B26_SeedArchiveDefault test fixture. */
        pe_addr_t arch_hdr  = 0x800A8028u;
        pe_addr_t arch_tbl  = 0x800A806Cu;   /* hdr + R + S */
        pe_addr_t arch_rec  = 0x800A8400u;
        PE_StoreU32(arch_hdr + 4u,  0x30u);              /* R  */
        PE_StoreU32(arch_hdr + 0x34u, 0x14u);            /* S  */
        PE_StoreU16(arch_tbl, (uint16_t)120u);           /* count */
        PE_StoreU16(arch_tbl + 2u + 2u * 30u,
                    (uint16_t)(arch_rec - arch_tbl));    /* entry[30] */
        PE_StoreU8(arch_rec, 0xFFu);                     /* record  */

        D_800B0DCD = 0;
        Bootstrap_ReturnInt("func_8007F72C", "func_800698D4", 1);
        Bootstrap_ReturnInt("func_8007F778", "func_800698D4", 0);
        Bootstrap_ReturnInt("func_80082314", "func_800698D4", 4);
        Bootstrap_ReturnInt("DsSearchFile(PEDISC01.IDF)", "func_800698D4", 1);
        Bootstrap_ReturnInt("DsSearchFile(PE.IMG)", "func_800698D4", 1);
        Bootstrap_ReturnInt("func_80080C48", "func_800698D4", 0);
        D_800B0DD8 = 0;
        D_800B0DCD |= 1;
        D_800B0DCD |= 2;
        return 0;
    }

    D_800B0DCD = 0;
    s0 = func_8007F72C();
    if (s0 != 1) return 1;   /* audit item 19: retail returns 1 (src/func_800698D4.c) */
    if (func_8007F778() != 0) return 1;

    r = func_80082314();
    if (r == 1) return 1;
    if (r != 4) return -1;

    PE_698D4_WaitIdle();
    r = func_80081414(PE_698D4_CDLFILE, PE_NAME_DISC1_IDF);
    if (r != 0 && r != -1) {
        PE_698D4_WaitIdle();
        r = func_80081414(PE_698D4_CDLFILE, PE_NAME_PE_IMG);
        if (r != 0 && r != -1) {
            D_800B0DD8 = (unsigned int)func_80080C48(PE_698D4_CDLFILE);
            D_800B0DCD |= 1;
        }
    }

    PE_698D4_WaitIdle();
    r = func_80081414(PE_698D4_CDLFILE, PE_NAME_DISC2_IDF);
    if (r != 0 && r != -1) {
        PE_698D4_WaitIdle();
        r = func_80081414(PE_698D4_CDLFILE, PE_NAME_PE_IMG);
        if (r != 0 && r != -1) {
            D_800B0DD8 = (unsigned int)func_80080C48(PE_698D4_CDLFILE);
            D_800B0DCD |= 2;
        }
    }

    return (D_800B0DCD == 0) ? -2 : 0;
}
