/*
 * PE-BTL48 — func_8006F39C event start plus 0x55 helpers
 * (translated retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 6F39C: 206 words 0x8006F39C..0x8006F6D4, SHA-256 ee236f21…8337.
 * CE49C: 23 words 0x800CE49C..0x800CE4F8, SHA-256 65cc4cd5…63bc.
 * D4620: 30 words 0x800D4620..0x800D4698, SHA-256 52e49e7d…10a5.
 *
 * Opcode 0x6A / 18774 jals 6F39C(*arg0, D2F0). Live imm 0x75
 * remaps to table code 0x55 (D_800942E0[0x55] = 0x800E13D4,
 * +4 = 0x800D4620). Finds a free 0xA0C slot in *D_800942E4
 * (6914C / 6A8D4 B0E60). CE49C(slot, 0x20) is the 0x55 extra;
 * EXE D_800E1044[0x20] is 0 so that leaf returns -1 without
 * writing +0x8C. D4620 then inits the slot. Return is the
 * slot index.
 *
 * Codes 0x6C..0x72 CD/XA prelude is not this cut (live 0x75
 * skips it). Unknown jalr targets are not invented.
 */
#include "psx_compat.h"
#include "pe_guestcode.h"
#include "pe_port_compat.h"
#include "game_port.h"

#define GA_D_800B0CD8 0x800B0CD8u
#define GA_D_800B0DD8 0x800B0DD8u
#define GA_D_80093162 0x80093162u
#define GA_D_80011618 0x80011618u
#define GA_D_800E10A0 0x800E10A0u
#define GA_D_800942E0 0x800942E0u
#define GA_D_800942E4 0x800942E4u
#define GA_D_800942E8 0x800942E8u
#define GA_D_800E1044 0x800E1044u
#define GA_FN_D4620   0x800D4620u

extern int func_8006E6A8(int lba, pe_addr_t dest, int sectors);
extern int func_8006E7E8(void);
extern int func_80072714(void);
extern void func_800726C4(void);
extern void func_80072724(void);

/* Overlay handler descriptors installed into D_800E10A0[0..6] for the CD/XA
 * prelude ids 0x6C..0x72 (matched src/func_8006F39C.c). */
static const pe_addr_t overlay_handlers[7] = {
    0x801F1BD8u, 0x801F1C58u, 0x801F1D00u, 0x801F1D8Cu,
    0x801F1E18u, 0x801F1EA4u, 0x801F1EF0u
};

/* The matched leaf's 0x6C..0x72 prelude: when the overlay-handler flag is
 * clear, stream the D_80093162 offset/size pair from the current mount base
 * (D_800B0DD8) into D_80011618, polling with a -1 restart, then install the
 * seven handlers and raise bit 0x10000. */
static void otag_install_overlay_handlers(void)
{
    int r;
    int i;

    if ((PE_LoadU32(GA_D_800B0CD8) & 0x10000u) != 0u)
        return;

retry:
    do {
        r = func_8006E6A8(
            PE_LoadU32(GA_D_800B0DD8) + (int)PE_LoadU16(GA_D_80093162),
            PE_LoadU32(GA_D_80011618),
            (int)PE_LoadU16(GA_D_80093162 + 2u) -
                (int)PE_LoadU16(GA_D_80093162));
    } while (r == -1);
    for (;;) {
        r = func_8006E7E8();
        if (r == 0)
            break;
        if (r == -1)
            goto retry;
    }
    (void)func_80072714();
    func_800726C4();
    func_80072724();
    for (i = 0; i < 7; i++)
        PE_StoreU32(GA_D_800E10A0 + (pe_addr_t)i * 4u, overlay_handlers[i]);
    PE_StoreU32(GA_D_800B0CD8,
                PE_LoadU32(GA_D_800B0CD8) | 0x10000u);
}

void func_800D4620(pe_addr_t slot)
{
    pe_addr_t a1;
    pe_addr_t a0;
    pe_addr_t a2;
    int i;

    a2 = slot + 0x0Cu;
    PE_StoreU8(slot + 0x02u, 1u);
    PE_StoreU8(slot + 0x03u, 0u);
    PE_StoreU16(slot + 0x1Au, 0u);
    PE_StoreU16(slot + 0x1Cu, 0u);
    PE_StoreU32(slot + 0x10u, slot + 0x90u);
    PE_StoreU8(slot + 0x18u, 0u);
    PE_StoreU8(slot + 0x19u, 0u);
    a1 = slot + 0x18u;
    for (i = 6; i >= 0; i--) {
        PE_StoreU16(a1 + 0x12u, 0u);
        a1 -= 2u;
    }
    a1 = a2 + 0x20u;
    a0 = a2 + 0x24u;
    for (i = 0; i < 8; i++) {
        PE_StoreU16(a1, 0xFFFFu);
        PE_StoreU16(a0 - 2u, 0u);
        PE_StoreU32(a0, 0u);
        a0 += 0x0Cu;
        a1 += 0x0Cu;
    }
}

int func_800CE49C(pe_addr_t slot, unsigned int extra)
{
    pe_addr_t rec;

    rec = PE_LoadU32(GA_D_800E1044 + extra * 4u);
    if (rec == 0u)
        return -1;
    PE_StoreU32(slot + 0x8Cu, rec);
    PE_StoreU32(slot + 0x0Cu, PE_LoadU32(rec + 0x34u));
    return 0;
}

int func_8006F39C(unsigned int code, pe_addr_t userdata)
{
    unsigned int orig;
    unsigned int extra;
    pe_addr_t table;
    pe_addr_t entry;
    pe_addr_t fn;
    pe_addr_t pool;
    pe_addr_t slot;
    unsigned int stride;
    int index;
    int i;
    uint8_t used;

    orig = code;
    extra = 0u;
    if (code >= 0xC0u)
        return -7;

    if (code >= 0x6Cu && code <= 0x72u)
        otag_install_overlay_handlers();

    PE_M34StackConstruct(code);
    (void)func_8006914C(0);

    if (code >= 0x55u) {
        extra = code - 0x55u;
        code = 0x55u;
    }

    table = PE_LoadU32(GA_D_800942E0);
    entry = PE_LoadU32(table + code * 4u);
    if (entry == 0u)
        return -8;
    fn = PE_LoadU32(entry + 4u);
    if (fn == 0u)
        return -1;

    if (code >= 0x46u && code < 0x55u) {
        pool = PE_LoadU32(GA_D_800942E8);
        stride = 0x10Cu;
    } else {
        pool = PE_LoadU32(GA_D_800942E4);
        stride = 0xA0Cu;
    }

    index = -1;
    slot = pool;
    for (i = 0; i < 11; i++) {
        used = PE_LoadU8(slot);
        if (used == 0u) {
            index = i;
            break;
        }
        slot += stride;
    }
    if (index < 0)
        return -3;

    slot = pool + (pe_addr_t)index * stride;
    PE_StoreU8(slot, 1u);
    PE_StoreU8(slot + 1u, (uint8_t)orig);
    PE_StoreU8(slot + 2u, 0u);
    PE_StoreU8(slot + 3u, 0u);
    PE_StoreU32(slot + 4u, 0u);
    PE_StoreU32(slot + 8u, userdata);
    if (code == 0x55u)
        (void)func_800CE49C(slot, extra);
    if (fn == 0x8018EFFCu && PE_MirrorOverlay())
        (void)PE_MirrorInit(slot);
    else if (fn == GA_FN_D4620)
        func_800D4620(slot);
    else if (fn == 0x800C9A70u)
        (void)func_800C9A70(slot);
    else if (fn == 0x800CD728u)
        (void)func_800CD728(slot);
    else if (fn == 0x8018F00Cu && PE_M34BossEffectOverlay())
        (void)PE_M34BossEffectInit(slot);
    else if (fn == 0x8018F06Cu && PE_M0348iEffectOverlay())
        (void)PE_M0348iEffectInit(slot);
    else if (fn == 0x800CE084u)
        (void)func_800CE084(slot);
    else if (fn == 0x8019151Cu && PE_M28MovementOverlay())
        (void)PE_M28MovementInit(slot);
    else if (fn == 0x8019150Cu && PE_M32MovementOverlay())
        (void)PE_M32MovementInit(slot);
    else if (PE_GuestCode_Resolve(fn))
        /* Generated overlay / EXE TU at that address (pe_guestcode.h). */
        (void)PE_GuestCall("@8006F39C:init", fn, 1u, (uintptr_t)slot, 0, 0, 0);
    else if (g_pe_strict_effect_stack) {
        fprintf(stderr, "[func_8006F39C] unresolved init fn=%08X code=%02X token=%08X slot=%08X\n",
                fn, code, (unsigned)PE_LoadU32(0x8009D280u), slot);
        Bootstrap_ReturnVoid("func_8006F39C_constructor", "func_8006F39C");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    } else {
        /* DAY1 SHIM (docs/ai_context/DAY1_FIDELITY_GAPS.md, "func_8006F39C
         * unresolved init"): the record is allocated exactly as retail but
         * its overlay init callback has no native port; skip it and keep
         * running.  Logged once per distinct fn. */
        static pe_addr_t seen[16]; static unsigned nseen;
        unsigned k;
        for (k = 0; k < nseen && seen[k] != fn; k++) {}
        if (k == nseen && nseen < 16u) {
            seen[nseen++] = fn;
            fprintf(stderr, "[DAY1_SHIM] func_8006F39C: init fn=%08X code=%02X token=%08X slot=%08X not ported; skipped\n",
                    fn, code, (unsigned)PE_LoadU32(0x8009D280u), slot);
        }
    }
    return index + ((code >= 0x46u && code < 0x55u) ? 11 : 0);
}
