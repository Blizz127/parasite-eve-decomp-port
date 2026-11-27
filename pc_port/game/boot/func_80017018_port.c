/*
 * PE-BTL12 — func_80017018 task VM (translated retail, not matching
 * src/). Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 159 words 0x80017018..0x80017294, SHA-256 0b2a2f69…fcd8.
 * Sole TEXT jal 361F4 @ 0x80036224. One jalr of
 * D_800910A0[word & 0x1FFF].
 *
 *   task = D_8009D300
 *   if (task+8) & 0x50: walk +0x24
 *   actor = D_8009D2F0
 *   if actor+0x98 & 0x1000 and !(task+8 & 0x80): walk
 *   if D1A0 & 0x100 and actor != D254 and !(task+8 & 0x80): walk
 *   if task+0x10 == 0: walk
 *   task+0x10-- ; if still != 0: walk
 *   fetch word at task+0; argc=(word>>13)&0xF; op=word&0x1FFF
 *   kinds in word>>17 (3 bits each; word+4 after 5 args)
 *   kind 0: args[i] = &imm[i]
 *   kind 1: args[i] = actor + 0xAC + imm[i]*4
 *   kind 2: args[i] = D_800A77F0 + imm[i]*4
 *   kind 3: args[i] = D_8009DF70 + imm[i]*4
 *   kind 4: args[i] = D_800B6A80 + imm[i]*4
 *   jalr table[op](args); v0!=0 re-fetch; v0==0 store PC, walk +0x24
 *
 * v0!=0 re-fetches from gp+0x90 (already advanced, or rewritten
 * by op 0). v0==0 stores that PC to task+0 and walks +0x24.
 *
 * Handlers ported here: 0 / 1 / 2 / 0x20 / 0xCE / 0xEA-nop /
 * 0xA / 0x1D / 0x09 / 0x05 / 0x14 / 0x40 / 0x3F / 0xED-2900 /
 * 0xE1 / 0x84 / 0x88 / 0x08 / 0x0B / 0x41 / 0x2E / 0x4E /
 * 0x2F / 0x30 / 0x5E / 0x77 / 0x9B / 0x0C / 0xD9 / 0x24 / 0x11 / 0x86 / 0x04 / 0xAA / 0x65 / 0x82 / 0x9C / 0xAB / 0x1E / 0x79 / 0xC1 / 0x85 / 0xDC / 0x1A / 0x6F / 0x5A / 0xB7 / 0x70 / 0x59 / 0x12 / 0x6A / 0x4B / 0x54 / 0x6B / 0x64 / 0x0E / 0x0D / 0x22 / 0x43 / 0x52 / 0x53 / 0xA6 / 0x2A / 0x87 / 0x31 / 0x94 / 0xC7 / 0xAD / 0xAE / 0xB2 / 0x8B / 0x89 / 0x95 / 0x03 / 0xB8 / 0xC6 / 0x55 / 0xCF / 0x97 / 0x9A / 0x71. 0x1C and 0x1F are the already-ported
 * mailbox leaves. SEW1 adds 0x5C / 0x5D (matching src/ leaves
 * 182C0 / 182E0) and 0x93 (190BC). Any other table slot is an
 * EXPLICIT boundary: the opcode PC is retained in the task and
 * PE_PORT_STOP_UNRESOLVED_BOUNDARY is requested. Retail jalr's the
 * table entry; silently yielding left the task with delay 0 and
 * suspended it forever (observed: Eve's under-stage script at
 * 801A074C op 5D never resumed, so Aya's poll of m19 never ended).
 * Not M2.
 */
#include "pe_plat/disc_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_guestcode.h"

#define GA_D_8009D300 0x8009D300u
#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009CE00 0x8009CE00u
#define GA_D_800910A0 0x800910A0u
/* Full retail VM handlers (decomp_hand), routed for ops 0xED / 0x31. */
int func_80016910(pe_addr_t a);
int func_80017BB4(pe_addr_t a0);
#define GA_D_800A77F0 0x800A77F0u
#define GA_D_8009DF70 0x8009DF70u
#define GA_D_800B6A80 0x800B6A80u
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D2E8 0x8009D2E8u
#define GA_D_8009D28C 0x8009D28Cu
#define GA_OP0        0x80017294u
#define GA_OP1        0x800172BCu
#define GA_OP2        0x800172E0u
#define GA_OP20       0x800172FCu
#define GA_OP1C       0x80017764u
#define GA_OP1F       0x800177ACu
#define GA_OPCE       0x800181CCu
#define GA_OPEA       0x80015DACu
#define GA_OPA        0x800173F4u
#define GA_OP1D       0x80017E20u
#define GA_OP9        0x80012850u
#define GA_OP5        0x8001731Cu
#define GA_OP14       0x80017588u
#define GA_OP40       0x80017D7Cu
#define GA_OP3F       0x80017D5Cu
#define GA_OPED       0x80016910u
#define GA_OPE1       0x8001A374u
#define GA_OP84       0x80018E84u
#define GA_OP88       0x80018F54u
#define GA_OP8        0x8001735Cu
#define GA_OPB        0x80012C20u
#define GA_OP41       0x80017D9Cu
#define GA_OP2E       0x80017AE8u
#define GA_OP4E       0x80017EC4u
#define GA_OP2F       0x80017B34u
#define GA_OP30       0x80017B74u
#define GA_OP5E       0x80014694u
#define GA_OP77       0x80014DA0u
#define GA_OP9B       0x80015240u
#define GA_OPC        0x80012E7Cu
#define GA_OPD9       0x8001A15Cu
#define GA_OP24       0x8001784Cu
#define GA_OP11       0x800130B4u
#define GA_OP86       0x80018EE0u
#define GA_OP4        0x80017988u
#define GA_OPAA       0x80019618u
#define GA_OP65       0x8001856Cu
#define GA_OP82       0x80018E58u
#define GA_OP9C       0x80019410u
#define GA_OPAB       0x80019638u
#define GA_OP1E       0x80019658u
#define GA_OP79       0x80018BECu
#define GA_OPC1       0x80019AC0u
#define GA_OP85       0x80018EB4u
#define GA_OPDC       0x8001A1F0u
#define GA_OP1A       0x800176FCu
#define GA_OP6F       0x80018954u
#define GA_OP5A       0x80018164u
#define GA_OPB7       0x80018A48u
#define GA_OP70       0x8001897Cu
#define GA_OP59       0x80018004u
#define GA_OP12       0x800131E8u
#define GA_OP6A       0x80018774u
#define GA_OP4B       0x80013C34u
#define GA_OP54       0x800143B0u
#define GA_OP6B       0x800187C0u
#define GA_OP64       0x800184ECu
#define GA_OP0E       0x80014228u
#define GA_OP0D       0x80017410u
#define GA_OP22       0x800177C8u
#define GA_OP43       0x80017DE4u
#define GA_OP52       0x80017F88u
#define GA_OP53       0x80017FB0u
#define GA_OPA6       0x80019484u
#define GA_OP28       0x800179F8u
#define GA_OP2A       0x80017A50u
#define GA_OP87       0x80018F0Cu
#define GA_OP31       0x80017BB4u
#define GA_OP94       0x80019154u
#define GA_OPC7       0x80019BE4u
#define GA_OPAD       0x80019748u
#define GA_OPB2       0x80019798u
#define GA_OPAE       0x80019728u
#define GA_OPAC       0x800196E8u
#define GA_OPE3       0x8001A3FCu
#define GA_OP8B       0x80018080u
#define GA_OP89       0x80017FF0u
#define GA_OP95       0x800192B8u
#define GA_OP3        0x80017C54u
#define GA_OPB8       0x80013514u
#define GA_OPC6       0x80013300u
#define GA_OP55       0x800144FCu
#define GA_OPCF       0x80019D24u
/* Host stand-in for ROM sp+16. APPROXIMATION: native has no guest $sp.
 * It must lie in the guest stack area, like retail's frame, and outside
 * every overlay load window.  It used to be 0x80120F80, inside the
 * 0x80120D00 overlay window: opcode 0x35 (func_80014E30) loads the movie
 * overlay there and only then reads **args, so every field movie
 * resolved to id 0 (FMV000) instead of the id the script passed.
 * 0x801FF400..0x801FF4EF is the port stack-scratch block, below every
 * hand scratch window (0x801FF4F0 and up): frame 400..43F, 375E0 list
 * 460, GA_POLY 480..49F and GA_DESC 4A0 (func_8001735C_port.c),
 * GA_EA_SLOT 4B0..4B7 (func_80015DAC_port.c).  PE_VMFRAME_AUDIT=1
 * checks that nothing writes the block between opcodes. */
#define GA_VM_FRAME   0x801FF400u
/* Host stand-in for 17410's stack s16 = -1. APPROXIMATION. */
#define GA_375E0_LIST 0x801FF460u


extern int func_80013300(pe_addr_t args);
extern int func_800144FC(pe_addr_t args);

/* func_80033A2C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80033A2C_port.c (src/func_80033A2C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80019D24: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019D24_port.c (src/func_80019D24.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80017294: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80017294_port.c (src/func_80017294.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800172BC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800172BC_port.c (src/func_800172BC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800172E0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800172E0_port.c (src/func_800172E0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800172FC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800172FC_port.c (src/func_800172FC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8001784C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8001784C_port.c (src/func_8001784C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * PE-BTL32 — opcode 0x04 task-flag walk 17988.
 *
 * 28 words 0x80017988..0x800179F8, SHA-256 2ddb182b…5c0a.
 * D_800910A0[0x04]. Zero jal. Always v0=1.
 * For i in 0..2: walk D2F0+0xA0[i] via +0x24; if task!=D300
 * then task+8 |= 0x10. Live type-0 after 65400 deliver of
 * type-1 0x1C payload 0xFF.
 */
int func_80017988(pe_addr_t args)
{
    unsigned int i;
    pe_addr_t actor;
    pe_addr_t cur;
    pe_addr_t skip;

    (void)args;
    actor = PE_LoadU32(GA_D_8009D2F0);
    skip = PE_LoadU32(GA_D_8009D300);
    for (i = 0u; i < 3u; i++) {
        cur = PE_LoadU32(actor + 0xA0u + i * 4u);
        while (cur != 0u) {
            if (cur != skip)
                PE_StoreU16(cur + 8u,
                            (uint16_t)(PE_LoadU16(cur + 8u) | 0x10u));
            cur = PE_LoadU32(cur + 0x24u);
        }
    }
    return 1;
}

/* func_8001856C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8001856C_port.c (src/func_8001856C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80018E58: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80018E58_port.c (src/func_80018E58.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80019410: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019410_port.c (src/func_80019410.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80019638: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019638_port.c (src/func_80019638.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80019658: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019658_port.c (src/func_80019658.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80018BEC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80018BEC_port.c (src/func_80018BEC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80019AC0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019AC0_port.c (src/func_80019AC0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80018EB4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80018EB4_port.c (src/func_80018EB4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8001A1F0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8001A1F0_port.c (src/func_8001A1F0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * PE-BTL43 — opcode 0x1A RNG write 176FC.
 *
 * 26 words 0x800176FC..0x80017764, SHA-256 ae4816ac…2ab2.
 * D_800910A0[0x1A]. jal 70D6C when *arg1==*arg2, else
 * jal 70DD0(*arg1,*arg2). On normal completion, *arg0 = v0; v0=1.
 * The host 70DD0 currently wraps signed range overflow rather than modeling
 * the original arithmetic exception; see native-random-overflow-20261105.
 * Live type-2 after 0xDC: dest local[0x18], 70DD0(0,0x64).
 */
int func_800176FC(pe_addr_t args)
{
    int32_t lo;
    int32_t hi;
    uint32_t result;

    lo = (int32_t)PE_LoadU32(PE_LoadU32(args + 4u));
    hi = (int32_t)PE_LoadU32(PE_LoadU32(args + 8u));
    if (lo == hi)
        result = func_80070D6C();
    else
        result = (uint32_t)func_80070DD0(lo, hi);
    PE_StoreU32(PE_LoadU32(args), result);
    return 1;
}

/* func_80018954: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80018954_port.c (src/func_80018954.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80018164: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80018164_port.c (src/func_80018164.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80018A48: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80018A48_port.c (src/func_80018A48.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * PE-BTL45 — opcode 0x70 formation wrapper 1897C.
 *
 * 51 words 0x8001897C..0x80018A48, SHA-256 64cd42b0…0ff3.
 * D_800910A0[0x70]. jal 2FA10(D2F0, lbu*0..2, lbu*3, lhu*4,
 * lb*5..8, lbu*9, lbu*10); v0=1.
 * Live (0,0,8,9,cond[1],5,-1,-1,-1,3,15).
 */
int func_8001897C(pe_addr_t args)
{
    func_8002FA10(PE_LoadU32(GA_D_8009D2F0),
                  PE_LoadU8(PE_LoadU32(args)),
                  PE_LoadU8(PE_LoadU32(args + 4u)),
                  PE_LoadU8(PE_LoadU32(args + 8u)),
                  PE_LoadU8(PE_LoadU32(args + 12u)),
                  PE_LoadU16(PE_LoadU32(args + 16u)),
                  (int)(int8_t)PE_LoadU8(PE_LoadU32(args + 20u)),
                  (int)(int8_t)PE_LoadU8(PE_LoadU32(args + 24u)),
                  (int)(int8_t)PE_LoadU8(PE_LoadU32(args + 28u)),
                  (int)(int8_t)PE_LoadU8(PE_LoadU32(args + 32u)),
                  PE_LoadU8(PE_LoadU32(args + 36u)),
                  PE_LoadU8(PE_LoadU32(args + 40u)));
    return 1;
}

/* func_80018004: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80018004_port.c (src/func_80018004.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * PE-BTL48 — opcode 0x6A event-start wrapper 18774.
 *
 * 19 words 0x80018774..0x800187C0, SHA-256 e32a2874…d822.
 * D_800910A0[0x6A]. jal 6F39C(*arg0, D2F0); *arg1=v0; v0=1.
 * Live (0x75, type-2 actor) → local[7].
 */
int func_80018774(pe_addr_t args)
{
    int value;

    value = func_8006F39C(PE_LoadU32(PE_LoadU32(args)),
                          PE_LoadU32(GA_D_8009D2F0));
    PE_StoreU32(PE_LoadU32(args + 4u), (uint32_t)value);
    return 1;
}

/*
 * PE-BTL50 — opcode 0x6B event-tick wrapper 187C0.
 *
 * 22 words 0x800187C0..0x80018818, SHA-256 1e733223…da40.
 * D_800910A0[0x6B]. jal 6F6D4(*arg0, 0, *arg1, *arg2,
 * *arg3, *arg4); v0=1. Live (local[7], 1, 0, 0, 0).
 */
int func_800187C0(pe_addr_t args)
{
    (void)func_8006F6D4(PE_LoadU32(PE_LoadU32(args)),
                        0u,
                        PE_LoadU32(PE_LoadU32(args + 4u)),
                        PE_LoadU32(PE_LoadU32(args + 8u)),
                        PE_LoadU32(PE_LoadU32(args + 12u)),
                        PE_LoadU32(PE_LoadU32(args + 16u)));
    return 1;
}

/* func_800184EC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800184EC_port.c (src/func_800184EC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * PE-BTL54 — opcode 0x0D message-open 17410.
 *
 * 13 words 0x80017410..0x80017444, SHA-256
 * f84d1f42908c9ba5b90a985acaaa36a54538d99d96ad731305bd82571f45518e.
 * D_800910A0[0x0D]. jal 375E0(lh *arg0, 0, &-1); v0=1.
 * Live type-0 FF arm +0x920 id 7 (persist[0]&4==0).
 */
int func_80017410(pe_addr_t args)
{
    PE_StoreU16(GA_375E0_LIST, 0xFFFFu);
    func_800375E0((int)(int16_t)PE_LoadU16(PE_LoadU32(args)), 0u,
                  GA_375E0_LIST);
    return 1;
}

/* Opcode D1: full 13 retail words 80019D84..80019DB8.
 * SHA-256 ccc7772c1178a03639fcbb9b6f3958fd16f58c0d7775664c52ededbab50b45b4.
 * Opens the signed message ID using the configured window geometry.
 * The guest temporary holds the original stack list's -1 terminator.
 */
int func_80019D84(pe_addr_t args)
{
    PE_StoreU16(GA_375E0_LIST, 0xFFFFu);
    func_800375E0((int)(int16_t)PE_LoadU16(PE_LoadU32(args)), 1u,
                  GA_375E0_LIST);
    return 1;
}

/*
 * PE-BTL54 — opcode 0x22 message-poll 177C8.
 *
 * 22 words 0x800177C8..0x80017820, SHA-256 from oracle.
 * D_800910A0[0x22]. jal 37548(lh *arg0). byte0==0 → v0=1.
 * Else CE00-=0xC, D300+0x10=1, v0=0. Live after 0x0D id 7.
 */
/* func_800177C8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800177C8_port.c (src/func_800177C8.c); hand port retired (port3 switch-over C). */

/* func_80037864: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80037864_port.c (src/func_80037864.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_80017DE4(pe_addr_t args)
{
    PE_StoreU32(PE_LoadU32(args), (uint32_t)func_80037864());
    return 1;
}

/* func_80017F88: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80017F88_port.c (src/func_80017F88.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80017FB0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80017FB0_port.c (src/func_80017FB0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80019484: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019484_port.c (src/func_80019484.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * PE-BTL59 — opcode 0x2A bit-or 17A50.
 *
 * 10 words 0x80017A50..0x80017A78, SHA-256
 * 2f58ffe89e4130506d67a6784ea268503a8d0a6dec26bbe01506e2cec926db68.
 * D_800910A0[0x2A]. *arg0 |= (1 << *arg1); v0=1.
 * Live type-0 +0x1150 kinds [4,0] imms [0,4] → scratch[0] |= 0x10.
 * That is bit 4, not the type-6 scratch[0]&4 wait.
 */
int func_80017A50(pe_addr_t args)
{
    pe_addr_t dest;
    uint32_t bit;

    dest = PE_LoadU32(args);
    bit = 1u << (PE_LoadU32(PE_LoadU32(args + 4u)) & 31u);
    PE_StoreU32(dest, PE_LoadU32(dest) | bit);
    return 1;
}

/*
 * PE-BTL96 — opcode 0x28 bit-clear 179F8.
 *
 * 11 words 0x800179F8..0x80017A24, SHA-256
 * cb5775eb…1aeb. D_800910A0[0x28].
 * *arg0 &= ~(1 << *arg1); v0=1. Clear twin of 0x2A.
 * Type-6 +0x1A38 imms [0,2] clears scratch[0] bit 2
 * after +0x1850. Not a first-entry unlock.
 */
int func_800179F8(pe_addr_t args)
{
    pe_addr_t dest;
    uint32_t bit;

    dest = PE_LoadU32(args);
    bit = 1u << (PE_LoadU32(PE_LoadU32(args + 4u)) & 31u);
    PE_StoreU32(dest, PE_LoadU32(dest) & ~bit);
    return 1;
}

/* func_80018F0C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80018F0C_port.c (src/func_80018F0C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80019BE4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019BE4_port.c (src/func_80019BE4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80019748: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019748_port.c (src/func_80019748.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800392EC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800392EC_port.c (src/func_800392EC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_80019798(pe_addr_t args)
{
    PE_StoreU32(PE_LoadU32(args), (uint32_t)func_800392EC() & 0xFFu);
    return 1;
}

/* func_80019728: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019728_port.c (src/func_80019728.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* AC/196E8 and E3/1A3FC, 16 retail words each (9EE8.s, AB74.s).
 * Register halfword-offset lists in the current actor's script and their
 * 16-bit lengths. The stage's battle controller executes these together. */
int func_800196E8(pe_addr_t args)
{
    pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
    uint32_t offset = PE_LoadU32(PE_LoadU32(args));
    PE_StoreU32(0x8009D2F8u, PE_LoadU32(actor + 0x9Cu) + (offset << 1));
    PE_StoreU16(0x8009D264u, (uint16_t)PE_LoadU32(PE_LoadU32(args + 4u)));
    return 1;
}

int func_8001A3FC(pe_addr_t args)
{
    pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
    uint32_t offset = PE_LoadU32(PE_LoadU32(args));
    PE_StoreU32(0x8009D248u, PE_LoadU32(actor + 0x9Cu) + (offset << 1));
    PE_StoreU16(0x8009D1CCu, (uint16_t)PE_LoadU32(PE_LoadU32(args + 4u)));
    return 1;
}

/*
 * PE-BTL61 — opcode 0x8B typed tagged-read 18080.
 *
 * 57 words 0x80018080..0x80018164, SHA-256
 * a896f3b9839e239002929b4239ef244ab7ecc2836c606f2e28ec7712a57bd1c7.
 * *arg0==0: 2FE78(lbu *arg2). Else walk D20C for type==*arg0 and
 * idB==*arg1 with +(0x98)&0x10 clear, then 3010C(actor, lbu *arg2).
 * *arg3 = v0 only on the type-0 or first matching walk hit;
 * exhausted D20C / bit-0x10-only matches skip the store.
 * Handler v0=1. Live type-6 [2,0,0x2C,local].
 */
int func_80018080(pe_addr_t args)
{
    uint32_t type;
    pe_addr_t actor;
    uint8_t tag;
    int value;

    type = PE_LoadU32(PE_LoadU32(args));
    tag = PE_LoadU8(PE_LoadU32(args + 8u));
    if (type == 0u) {
        value = func_8002FE78(tag);
        PE_StoreU32(PE_LoadU32(args + 0xCu), (uint32_t)value);
        return 1;
    }
    actor = PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        if (PE_LoadU8(actor + 0x0Cu) == (uint8_t)type &&
            PE_LoadU8(actor + 0x0Du) ==
                (uint8_t)PE_LoadU32(PE_LoadU32(args + 4u)) &&
            (PE_LoadU32(actor + 0x98u) & 0x10u) == 0u) {
            value = func_8003010C(actor, tag);
            PE_StoreU32(PE_LoadU32(args + 0xCu), (uint32_t)value);
            return 1;
        }
        actor = PE_LoadU32(actor + 4u);
    }
    return 1;
}

/* func_80017FF0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80017FF0_port.c (src/func_80017FF0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800192B8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800192B8_port.c (src/func_800192B8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800192C8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800192C8_port.c (src/func_800192C8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * opcode 0x97 — handler 0x800192DC..0x8001930C (retail 14 words).
 *
 *   800192E4  lw   v0,0(a0)          ; v0  = *args0
 *   800192E8  lui  a1,0x800A
 *   800192EC  lw   a1,-0x2D10(a1)    ; a1  = *(0x8009D2F0)   owner actor
 *   800192F0  lw   a0,0(v0)          ; a0  = **args0          effect index
 *   800192F4  jal  8006FC18
 *   800192F8  addu a2,zero,zero      ; a2  = 0                delay slot
 *   lw ra / li v0,1 / jr ra / addiu sp
 *
 * Live M0374I module 2 (PC 0x801C5D00): args [14]. M0374I is on the M0042I
 * route (docs/ai_context/DAY1_DAY2_TRANSITIONS.md §6), so this closes a real
 * Day-2 park-path opcode. func_8006FC18 is the already-ported effect-destroy
 * leaf (game/boot/func_80020F18_port.c:28); this cut is a pure wrapper.
 * Evidence: docs/evidence/field-vm-opcode-coverage/REPORT.md.
 */
int func_800192DC(pe_addr_t args)
{
    uint32_t index = PE_LoadU32(PE_LoadU32(args));
    (void)func_8006FC18(index, PE_LoadU32(GA_D_8009D2F0), 0u);
    return 1;
}

/*
 * opcode 0x9A — handler 0x800193D8..0x80019410 (retail 14 words).
 *
 *   lw a0,0(a0) ; lw v1,4(a0arg) ; lw a2,8(a0arg)
 *   a0=*a0 ; a1=*v1 ; a2=*a2 ; jal func_80065AD4 ; v0=1
 *
 * Live M0091I module 3 (Day-2 terminal room), two uses args [2,5,0].
 * Evidence: docs/evidence/field-vm-opcode-coverage/REPORT.md.
 */
int func_800193D8(pe_addr_t args)
{
    uint32_t a0 = PE_LoadU32(PE_LoadU32(args));
    uint32_t a1 = PE_LoadU32(PE_LoadU32(args + 4u));
    uint32_t a2 = PE_LoadU32(PE_LoadU32(args + 8u));
    (void)func_80065AD4(a0, a1, a2);
    return 1;
}

/* func_80018A9C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80018A9C_port.c (src/func_80018A9C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * PE-BTL64 — opcode 0x03 camera-request 17C54.
 *
 * 14 words 0x80017C54..0x80017C8C, SHA-256
 * 4d53aeb570e4a1a69243d19089312f97ae255ebe4f0e9d5c15023ab01d9afa24.
 * jal 661EC(lh *arg0, lh *arg1, lhu *arg2, 0); v0=1.
 * Live M0367I type-1 +0x1C8: (0x12B, 0x7C, 1).
 */
int func_80017C54(pe_addr_t args)
{
    int16_t x;
    int16_t y;
    uint16_t z;

    x = (int16_t)PE_LoadU16(PE_LoadU32(args));
    y = (int16_t)PE_LoadU16(PE_LoadU32(args + 4u));
    z = PE_LoadU16(PE_LoadU32(args + 8u));
    func_800661EC((int)x, (int)y, (unsigned int)z, 0u);
    return 1;
}

/* Media waits: retail 18FDC (29 words) and 19060 (19 words).
 * Retry the same instruction next tick while the media owner is active.
 * 190AC is the retail two-word return-1 leaf. */
extern int func_8006EC08(void);
extern int func_8006EBE4(void);

static int pe_script_media_wait(pe_addr_t args, int status_wait)
{
    if (status_wait && (int16_t)func_8006EBE4() ==
            (int32_t)PE_LoadU32(PE_LoadU32(args)))
        return 1;
    if ((uint8_t)func_8006EC08() == 0u)
        return 1;
    PE_StoreU32(GA_D_8009CE00,
                PE_LoadU32(GA_D_8009CE00) - (status_wait ? 12u : 8u));
    PE_StoreU32(PE_LoadU32(GA_D_8009D300) + 0x10u, 1u);
    return 0;
}

/* PC of the opcode being dispatched (set by the main loop) so an
 * unported table slot can retain it instead of advancing past it. */
static pe_addr_t g_pe_17018_opcode_pc;

/* Original 18300..183E8: suspend/resume the actor's three task chains.
 * Access aliases preserve physical RAM addresses, including actor zero. */
static pe_addr_t script_task_ram(pe_addr_t p)
{
    return p < 0x200000u ? p | 0x80000000u : p;
}

int func_80018300(pe_addr_t args)
{
    pe_addr_t current = PE_LoadU32(GA_D_8009D300);
    pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
    (void)args;
    for (unsigned i = 0; i < 3; i++) {
        pe_addr_t task = PE_LoadU32(script_task_ram(actor + 0xA0u + i * 4u));
        while (task) {
            if (task != current)
                PE_StoreU16(script_task_ram(task + 8u),
                            PE_LoadU16(script_task_ram(task + 8u)) | 0x40u);
            task = PE_LoadU32(script_task_ram(task + 0x24u));
        }
    }
    return 1;
}

int func_80018364(pe_addr_t args)
{
    pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
    (void)args;
    for (unsigned i = 0; i < 3; i++) {
        pe_addr_t task = PE_LoadU32(script_task_ram(actor + 0xA0u + i * 4u));
        while (task) {
            unsigned flags = PE_LoadU16(script_task_ram(task + 8u));
            if (flags & 0x40u) {
                pe_addr_t saved = PE_LoadU32(script_task_ram(task + 4u));
                PE_StoreU16(script_task_ram(task + 8u), flags & 0xFFBFu);
                if (saved) {
                    flags = PE_LoadU16(script_task_ram(task + 8u));
                    PE_StoreU32(script_task_ram(task), saved);
                    PE_StoreU32(script_task_ram(task + 0x10u), 1u);
                    PE_StoreU16(script_task_ram(task + 8u), flags & 0xFFDFu);
                }
            }
            task = PE_LoadU32(script_task_ram(task + 0x24u));
        }
    }
    return 1;
}

/* C5/CC model attachment and D4/D5/D6 animation attachment.
 * These original routines can dereference physical RAM zero; canonicalize
 * only those accesses without changing global guest address policy. */
static pe_addr_t attachment_ram(pe_addr_t p)
{
    return p < 0x200000u ? p | 0x80000000u : p;
}

__attribute__((unused)) static int pe_attachment_command(pe_addr_t fn, pe_addr_t args)
{
    pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0), parent, node;
    if (fn == 0x80019170u || fn == 0x80019F04u || fn == 0x80015108u) {
        uint32_t type = PE_LoadU32(PE_LoadU32(args));
        if (!type) parent = PE_LoadU32(GA_D_8009D254);
        else {
            parent = PE_LoadU32(0x8009D20Cu);
            while (parent) {
                if (PE_LoadU8(parent + 0xCu) == type &&
                    PE_LoadU8(parent + 0xDu) == PE_LoadU32(PE_LoadU32(args + 4u)) &&
                    !(PE_LoadU32(parent + 0x98u) & 0x10u)) break;
                parent = PE_LoadU32(parent + 4u);
            }
        }
        if (!parent) return 1;
        if (fn == 0x80015108u) {
            pe_addr_t model = actor + 0x1B4u, source = parent + 0x1B4u, record;
            int32_t index = (int16_t)PE_LoadU16(PE_LoadU32(args + 8u));
            /* Original3DF50 binds an anchor and clears the object pointer,
             * selecting 3A6A8's parent-joint (3E188) transform path. */
            PE_StoreU16(model + 0x32u, (uint16_t)index);
            PE_StoreU32(model, 0u);
            PE_StoreU32(model + 0x24u, source);
            record = attachment_ram(PE_LoadU32(source + 0x18u) + (uint32_t)index * 16u);
            PE_StoreU16(model + 0x70u, PE_LoadU16(record + 6u));
            PE_StoreU16(model + 0x2Cu, PE_LoadU16(record));
            PE_StoreU16(model + 0x2Eu, PE_LoadU16(record + 2u));
            PE_StoreU16(model + 0x30u, (uint16_t)(PE_LoadU16(record + 4u) + PE_LoadU16(model + 0x70u)));
            func_8003A6A8(PE_LoadU32(GA_D_8009D2F0) + 0x1B4u, 0x800B89F8u);
            actor = PE_LoadU32(GA_D_8009D2F0);
            PE_StoreU32(actor + 0x18Cu, parent);
            PE_StoreU32(actor + 0x28u, (uint32_t)PE_LoadU16(actor + 0x254u) << 16);
            PE_StoreU32(actor + 0x2Cu, (uint32_t)PE_LoadU16(actor + 0x256u) << 16);
            PE_StoreU32(actor + 0x30u, (uint32_t)PE_LoadU16(actor + 0x258u) << 16);
            PE_StoreU32(actor + 0x98u, PE_LoadU32(actor + 0x98u) | 0x2000u);
        } else if (fn == 0x80019170u) {
            pe_addr_t model = actor + 0x1B4u;
            uint32_t kind = PE_LoadU8(attachment_ram(PE_LoadU32(model)) + 2u);
            uint16_t joint = PE_LoadU16(PE_LoadU32(args + 8u));
            /* Original3E0A4. Joint stores preserve the signed halfword bits. */
            PE_StoreU32(model + 0x24u, parent + 0x1B4u);
            PE_StoreU16(model + 0x28u, kind == 2u ? 3u : 1u);
            PE_StoreU16(model + 0x2Au, joint);
            PE_StoreU32(PE_LoadU32(GA_D_8009D2F0) + 0x18Cu, parent);
        } else {
            PE_StoreU32(actor + 0x18Cu, parent);
            PE_StoreU32(parent + 0x98u, PE_LoadU32(parent + 0x98u) | 0x100000u);
            PE_StoreU32(actor + 0x98u, PE_LoadU32(actor + 0x98u) | 0x600000u);
        }
        return 1;
    }
    if (fn == 0x80019260u) {
        pe_addr_t model = actor + 0x1B4u;
        pe_addr_t data = attachment_ram(PE_LoadU32(model));
        /* Original3E0D0 clears the link before reading the model kind. */
        PE_StoreU32(model + 0x24u, 0u);
        PE_StoreU16(model + 0x28u, PE_LoadU8(data + 2u) == 2u ? 2u : 0u);
        PE_StoreU32(PE_LoadU32(GA_D_8009D2F0) + 0x18Cu, 0u);
        return 1;
    }
    if (fn == 0x80019FE0u) {
        uint32_t flags = PE_LoadU32(actor + 0x98u);
        node = PE_LoadU32(0x8009D20Cu);
        PE_StoreU32(actor + 0x18Cu, 0u);
        PE_StoreU32(actor + 0x98u, flags & ~0x600000u);
        for (; node; node = PE_LoadU32(node + 4u))
            if (node != actor && PE_LoadU32(node + 0x18Cu) == PE_LoadU32(actor + 0x18Cu))
                return 1;
        /* Original reloads the already-cleared pointer, even on this tail. */
        parent = attachment_ram(PE_LoadU32(PE_LoadU32(GA_D_8009D2F0) + 0x18Cu));
        PE_StoreU32(parent + 0x98u, PE_LoadU32(parent + 0x98u) & ~0x100000u);
        return 1;
    }
    node = PE_LoadU32(0x8009D20Cu);
    PE_StoreU32(actor + 0x98u, PE_LoadU32(actor + 0x98u) & ~0x100000u);
    for (; node; node = PE_LoadU32(node + 4u)) {
        if (PE_LoadU32(node + 0x18Cu) == actor) {
            uint32_t flags = PE_LoadU32(node + 0x98u);
            PE_StoreU32(node + 0x18Cu, 0u);
            PE_StoreU32(node + 0x98u, flags & ~0x600000u);
        }
    }
    return 1;
}

extern int func_80014E30(pe_addr_t a0);

/* src-derived attachment handlers (decomp_hand/absent_lo_actor_port.c,
 * e9_guest_ptr_port.c, absent_lo_state_port.c). */
int func_80015108(pe_addr_t arg0);
int func_80019170(pe_addr_t arg0);
int func_80019F04(pe_addr_t arg0);
int func_80019FE0(void);
int func_8001A064(void);

/* Audit item 26, batch 1: opcode handlers that have generated matched TUs
 * (src/func_*.c) run through the verified guest-code registry, as retail's
 * jalr D_800910A0[op] does, instead of the inline re-implementations below. */
/* Batch 2 (audit item 26): the remaining inline handlers whose src/ leaf the
 * generator cannot emit (guest pointer vectors / structs) were audited line by
 * line against src/: 80018D50, 80018DD4, 80019450, 80018B98, 80018818,
 * 80018864, 80018894, 800199CC, 80017A24 (MIPS sllv low-5 shift), 80017D3C,
 * 80017C8C, 80015648 (fixed: full-width id compares).  80016F10/80014E30 keep
 * their route-only skip shims (HOST_ADAPTED, off on a normal start). */
static const uint32_t k_17018_matched_ops[] = {
    0x80019260u, 0x80017EFCu, 0x80017F20u, 0x8001967Cu, 0x80018F74u, 0x800182A0u, 0x80017AC0u,
    0x80018C58u, 0x80018B68u, 0x80019CECu, 0x800176B8u, 0x800182C0u, 0x800182E0u,
    0x800190BCu, 0x80016FE0u, 0x800176E0u, 0x80017968u, 0x80017928u, 0x80017948u,
    0x80017A78u, 0x80017AA4u, 0x80017CC4u, 0x80017D18u, 0x80017EA4u, 0x80018754u,
    0x80017FDCu, 0x800193B8u, 0x80019298u, 0x80019A9Cu, 0x800199F8u, 0x80017820u,
    0x80018B00u, 0x80018C88u, 0x80018CB8u, 0x80018CF0u, 0x80018D20u, 0x800197D0u,
    0x800197F0u, 0x8001994Cu, 0x8001998Cu, 0x80019D44u, 0x80019C4Cu, 0x80019768u,
};

static int pe_17018_matched_op(pe_addr_t fn)
{
    static int legacy = -1;
    if (legacy < 0) legacy = getenv("PE_VM_INLINE_OPS") != NULL;   /* bisect aid */
    if (legacy) return 0;
    for (unsigned k = 0; k < sizeof k_17018_matched_ops / sizeof k_17018_matched_ops[0]; k++)
        if (k_17018_matched_ops[k] == fn) return PE_GuestCode_Resolve(fn) != NULL;
    return 0;
}

static int pe_17018_dispatch_body(pe_addr_t fn, pe_addr_t args)
{
    if (pe_17018_matched_op(fn))
        return PE_GuestCall("func_80017018", fn, 1u, args, 0u, 0u, 0u);
    if (fn == 0x80018300u) return func_80018300(args);
    if (fn == 0x80018364u) return func_80018364(args);
    /* Attachment opcodes: the src-derived handlers (decomp_hand/, generated
     * 80019260) replace the former inline pe_attachment_command copy
     * (VM batch 3, audited against src/func_{80015108,80019170,80019F04,
     * 80019FE0,8001A064}.c). */
    if (fn == 0x80015108u) return func_80015108(args);
    if (fn == 0x80019170u) return func_80019170(args);
    if (fn == 0x80019F04u) return func_80019F04(args);
    if (fn == 0x80019FE0u) return func_80019FE0();
    if (fn == 0x8001A064u) return func_8001A064();
    if (fn == 0x800192C8u)
        return func_800192C8();
    if (fn == 0x800192DCu)
        return func_800192DC(args);
    if (fn == 0x800193D8u)
        return func_800193D8(args);
    if (fn == 0x80018A9Cu)
        return func_80018A9C(args);
    if (fn == 0x8001930Cu)
        return func_8001930C(args);
    if (fn == 0x80015C7Cu)   /* opcode 0xE9: field message query/close */
        return func_80015C7C(args);
    if (fn == 0x80017EFCu || fn == 0x80017F20u) {
        /* 4F/50: matching leaves pause/resume this actor's animation. */
        pe_addr_t actor=PE_LoadU32(GA_D_8009D2F0);
        uint32_t flags=PE_LoadU32(actor+0x98u);
        PE_StoreU32(actor+0x98u,fn==0x80017EFCu?flags|0x100u:flags&~0x100u);
        return 1;
    }
    if (fn == 0x8001967Cu) { /* 32: matching func_8001967C.c. */
        pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
        PE_StoreU32(actor + 0x98u, PE_LoadU32(actor + 0x98u) & ~0x80u);
        return 1;
    }
    if (fn == 0x80018D50u || fn == 0x80018DD4u) {
        /* STG1: 9550.s, opcodes 80/81 toggle a walkmesh record's bit 7.
         * Flat records are 22 bytes; sloped records are 28 bytes. */
        uint32_t index = PE_LoadU32(PE_LoadU32(args));
        uint32_t stride = PE_LoadU32(0x8009D1D8u) ? 28u : 22u;
        pe_addr_t mesh = PE_LoadU32(0x8009D1FCu);
        pe_addr_t record = PE_LoadU32(mesh + 0x1Cu) + index * stride;
        uint8_t flags = PE_LoadU8(record);
        PE_StoreU8(record, fn == 0x80018D50u ? (uint8_t)(flags | 0x80u)
                                           : (uint8_t)(flags & 0x7Fu));
        return 1;
    }
    if (fn == 0x800136C0u)
        return func_800136C0(args);
    if (fn == 0x80018F74u) { /* 1B: 9774.s, 18F74..18FDC. */
        pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
        int32_t value = (int32_t)PE_LoadU32(PE_LoadU32(args)) >> 4;
        PE_StoreU16(actor + 0x26u, (uint16_t)value);
        if (actor == PE_LoadU32(GA_D_8009D254)) {
            /* Retail wraps the two shifts/add before dividing toward zero. */
            int32_t scaled = (int32_t)((uint32_t)value * 384u);
            PE_StoreU16(0x800BCFFEu, (uint16_t)(scaled / 4096));
        }
        return 1;
    }
    if (fn == 0x80016F10u) {
        /* Optional demo shortcut; ordinary play executes the original
         * two-frame name-entry handshake below. */
        if (PE_Port_SkipOpeningMenu() && D_8009D280 == 0xA8001048u &&
            PE_LoadU32(GA_D_8009D254) != 0u &&
            PE_LoadU32(GA_D_8009D2F0) == PE_LoadU32(GA_D_8009D254) &&
            PE_LoadU32(PE_LoadU32(args)) == 0u) {
            Stub_Record("func_80016F10_skip_opening_menu", "HOST_ADAPTED");
            return 1;
        }
        return func_80016F10(args);
    }
    if (fn == 0x800182A0u) { /* opcode 5B: matching leaf */
        PE_StoreU32(0x800BCF88u, PE_LoadU32(0x800BCF88u) & ~0xC0u);
        return 1;
    }
    if (fn == 0x80017AC0u) { /* opcode 2D: matching leaf */
        PE_StoreU32(GA_D_8009D2E8,
            PE_LoadU32(GA_D_8009D2E8) | PE_LoadU32(PE_LoadU32(args)));
        return 1;
    }
    if (fn == 0x80019450u) { /* opcode 9D: retail 13 words */
        pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
        uint32_t diameter = (uint32_t)((int16_t)PE_LoadU16(actor + 0x224u) * 2);
        uint32_t product = diameter * PE_LoadU32(PE_LoadU32(args));
        PE_StoreU16(PE_LoadU32(actor + 0x1B4u) + 0x14u,
                     (uint16_t)((int32_t)product >> 16));
        return 1;
    }
    if (fn == 0x80018B98u || fn == 0x80018C58u) {
        uint32_t index = PE_LoadU32(PE_LoadU32(args));
        uint32_t value = PE_LoadU32(PE_LoadU32(args + 4u));
        if (fn == 0x80018B98u)
            func_80065954(index, value);
        else
            func_800659C8(index, value);
        return 1;
    }
    if (fn == 0x80018B68u) { /* 6590C: 18-word camera slot setter */
        pe_addr_t container = PE_LoadU32(0x800B1624u);
        pe_addr_t slot = container + PE_LoadU32(container + 0x10u)
            + (PE_LoadU32(PE_LoadU32(args)) << 4);
        uint32_t value = PE_LoadU8(slot + 4u)
            | (PE_LoadU32(PE_LoadU32(args + 4u)) << 16);
        uint8_t flags = PE_LoadU8(slot);
        PE_StoreU16(slot + 0xAu, 0u);
        PE_StoreU8(slot, (uint8_t)(flags | 2u));
        PE_StoreU32(slot + 4u, value);
        return 1;
    }

    if (fn == 0x80019CECu) { /* CD: 19CEC..19D24, adopt animated world position. */
        pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
        PE_StoreU32(actor + 0x28u, (uint32_t)PE_LoadU16(actor + 0x21Cu) << 16);
        PE_StoreU32(actor + 0x30u, (uint32_t)PE_LoadU16(actor + 0x220u) << 16);
        PE_StoreU32(actor + 0x2Cu, (uint32_t)PE_LoadU16(actor + 0x21Eu) << 16);
        return 1;
    }

    if (fn == 0x80014E30u) {
        /* HOST_ADAPTED --skip-movie: opcode 35 loads the movie
         * overlay and calls 1216C4/121C04/1223A8. Skip before its
         * display disable and arena overwrites; retail returns 1. */
        /* PE_ROUTE_PLAY_MOVIES=1 (route harness aid, see port_main.c):
         * the route keeps --skip-movie for boot/title but plays field
         * movies, so each Day-1 movie can be checked end to end. */
        if (PE_Port_SkipMovie() && getenv("PE_ROUTE_PLAY_MOVIES") == NULL) {
            Stub_Record("func_80014E30_skip_movie", "HOST_ADAPTED");
            return 1;
        }
        /* fmv lane: the matched-decomp func_80014E30 (absent_lo_batch3)
         * loads the movie overlay and runs 1216C4/121C04/1223A8; the
         * stream is served by pe_str_feed when the CD device is off.
         * Without a disc / PE.IMG base the overlay read can never
         * complete (retail would spin), so that stays a named cut. */
        if (PE_Disc_GetActive() != NULL && PE_LoadU32(0x800B0DD8u) != 0u)
            return func_80014E30(args);
        Bootstrap_ReturnVoid("func_80014E30", "func_80017018");
        PE_StoreU32(GA_D_8009CE00, PE_LoadU32(GA_D_8009CE00) - 12u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    if (fn == 0x800190ACu)
        return 1;
    if (fn == 0x80018FDCu)
        return pe_script_media_wait(args, 1);
    if (fn == 0x80019060u)
        return pe_script_media_wait(args, 0);

    if (fn == GA_OP0)
        return func_80017294(args);
    if (fn == GA_OP1)
        return func_800172BC();
    if (fn == GA_OP2)
        return func_800172E0(args);
    if (fn == GA_OP20)
        return func_800172FC();
    if (fn == GA_OP1C)
        return func_80017764(args);
    if (fn == GA_OP1F)
        return func_800177AC(args);
    if (fn == GA_OPCE)
        return func_800181CC(args);
    if (fn == GA_OPEA)
        return func_80015DAC_default_cut(args);
    if (fn == GA_OPA)
        return func_800173F4(args);
    if (fn == GA_OP1D)
        return func_80017E20(args);
    if (fn == GA_OP9)
        return func_80012850(args);
    if (fn == GA_OP5)
        return func_8001731C(args);
    if (fn == GA_OP14)
        return func_80017588(args);
    if (fn == GA_OP40)
        return func_80017D7C();
    if (fn == GA_OP3F)
        return func_80017D5C();
    if (fn == GA_OPED)   /* full retail op 0xED (decomp_hand/absent_r3_port.c, src/func_80016910.c) */
        return func_80016910(args);
    if (fn == GA_OPE1)
        return func_8001A374(args);
    if (fn == GA_OP84)
        return func_80018E84(args);
    if (fn == GA_OP88)
        return func_80018F54();
    if (fn == GA_OP8)
        return func_8001735C(args);
    if (fn == GA_OPB)
        return func_80012C20(args);
    if (fn == GA_OP41)
        return func_80017D9C();
    if (fn == GA_OP2E)
        return func_80017AE8(args);
    if (fn == GA_OP4E)
        return func_80017EC4(args);
    if (fn == GA_OP2F)
        return func_80017B34(args);
    if (fn == GA_OP30)
        return func_80017B74();
    if (fn == GA_OP5E)
        return func_80014694(args);
    if (fn == GA_OP77)
        return func_80014DA0(args);
    if (fn == GA_OP9B)
        return func_80015240(args);
    if (fn == GA_OPC)
        return func_80012E7C(args);
    if (fn == GA_OPD9)
        return func_8001A15C(args);
    if (fn == 0x8001A214u)
        return func_8001A214(args);
    if (fn == GA_OP24)
        return func_8001784C(args);
    if (fn == GA_OP11)
        return func_800130B4(args);
    if (fn == GA_OP86)
        return func_80018EE0(args);
    if (fn == GA_OP4)
        return func_80017988(args);
    if (fn == GA_OPAA)
        return func_80019618();
    if (fn == GA_OP65)
        return func_8001856C();
    if (fn == GA_OP82)
        return func_80018E58(args);
    if (fn == GA_OP9C)
        return func_80019410();
    if (fn == GA_OPAB)
        return func_80019638();
    if (fn == GA_OP1E)
        return func_80019658();
    if (fn == GA_OP79)
        return func_80018BEC();
    if (fn == GA_OPC1)
        return func_80019AC0();
    if (fn == GA_OP85)
        return func_80018EB4(args);
    if (fn == GA_OPDC)
        return func_8001A1F0();
    if (fn == GA_OP1A)
        return func_800176FC(args);
    if (fn == GA_OP6F)
        return func_80018954();
    if (fn == 0x8001A390u)
        return func_8001A390(args);
    if (fn == GA_OP5A)
        return func_80018164(args);
    if (fn == GA_OPB7)
        return func_80018A48(args);
    if (fn == 0x800198C4u)
        return func_800198C4(args);
    if (fn == GA_OP70)
        return func_8001897C(args);
    if (fn == GA_OP59)
        return func_80018004(args);
    if (fn == GA_OP12)
        return func_800131E8(args);
    if (fn == 0x800176B8u) { /* op13: matching src/func_800176B8.c */
        pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
        PE_StoreU32(actor + 0x98u, PE_LoadU32(actor + 0x98u) | PE_LoadU32(PE_LoadU32(args)));
        return 1;
    }
    if (fn == GA_OP6A)
        return func_80018774(args);
    if (fn == GA_OP4B)
        return func_80013C34(args);
    if (fn == GA_OP54)
        return func_800143B0(args);
    if (fn == GA_OP6B)
        return func_800187C0(args);
    if (fn == 0x80018818u) {
        (void)func_8006F6D4(PE_LoadU32(PE_LoadU32(args)), 1u,
            PE_LoadU32(PE_LoadU32(args + 4u)), PE_LoadU32(args + 8u),
            PE_LoadU32(args + 12u), PE_LoadU32(args + 16u));
        return 1;
    }
    if (fn == GA_OP64)
        return func_800184EC(args);
    if (fn == GA_OP0E)
        return func_80014228(args);
    if (fn == GA_OP0D)
        return func_80017410(args);
    if (fn == GA_OP22)
        return func_800177C8(args);
    if (fn == GA_OP43)
        return func_80017DE4(args);
    if (fn == GA_OP52)
        return func_80017F88(args);
    if (fn == GA_OP53)
        return func_80017FB0(args);
    if (fn == GA_OPA6)
        return func_80019484(args);
    if (fn == GA_OP2A)
        return func_80017A50(args);
    if (fn == GA_OP28)
        return func_800179F8(args);
    if (fn == GA_OP87)
        return func_80018F0C(args);
    if (fn == GA_OP31)   /* full retail op 0x31 incl. the A9400048 game-over branch (src/func_80017BB4.c) */
        return func_80017BB4(args);
    if (fn == 0x800194B0u) return func_800194B0(args);
    if (fn == 0x80019540u) return func_80019540(args);
    if (fn == 0x80015BACu) return func_80015BAC(args);
    if (fn == GA_OP94)
        return func_80019154(args);
    if (fn == GA_OPC7)
        return func_80019BE4();
    if (fn == GA_OPAD)
        return func_80019748();
    if (fn == GA_OPB2)
        return func_80019798(args);
    if (fn == GA_OPAE)
        return func_80019728();
    if (fn == GA_OPAC)
        return func_800196E8(args);
    if (fn == GA_OPE3)
        return func_8001A3FC(args);
    if (fn == GA_OP8B)
        return func_80018080(args);
    if (fn == GA_OP89)
        return func_80017FF0();
    if (fn == GA_OP95)
        return func_800192B8();
    if (fn == GA_OP3)
        return func_80017C54(args);
    if (fn == GA_OPB8)
        return func_80013514(args);
    if (fn == GA_OPC6)
        return func_80013300(args);
    if (fn == GA_OP55)
        return func_800144FC(args);
    if (fn == GA_OPCF)
        return func_80019D24();
    if (fn == 0x800182C0u) { /* 5C: matching src/func_800182C0.c */
        PE_StoreU32(0x800BCF88u, PE_LoadU32(0x800BCF88u) | 0xC0u);
        return 1;
    }
    if (fn == 0x800182E0u) { /* 5D: matching src/func_800182E0.c */
        PE_StoreU32(PE_LoadU32(GA_D_8009D2F0) + 0x20u,
                    PE_LoadU32(PE_LoadU32(args)));
        return 1;
    }
    if (fn == 0x800190BCu) {
        /* 93: 190BC..19154 (38 words). 3C5D8(actor+0x1B4, (s16)*arg0);
         * actor+0x250 |= 2; when the actor is the camera target D254,
         * also 3C5D8(B0CEC, (s16)*arg0) and B0D88 |= 2. v0=1. */
        pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
        int value = (int)(int16_t)PE_LoadU16(PE_LoadU32(args));
        func_8003C5D8(actor + 0x1B4u, value);
        PE_StoreU16(actor + 0x250u, (uint16_t)(PE_LoadU16(actor + 0x250u) | 2u));
        if (actor == PE_LoadU32(GA_D_8009D254)) {
            func_8003C5D8(0x800B0CECu, value);
            PE_StoreU16(0x800B0D88u, (uint16_t)(PE_LoadU16(0x800B0D88u) | 2u));
        }
        return 1;
    }
    /* SEW2: matching src/ leaves wired by table entry. Each is the retail
     * body from src/func_<addr>.c (era-matched); args are VM_FRAME pointers. */
    if (fn == 0x80016FE0u) { /* F0: 16FE0 — *arg0 = (D2E8 & 1) ? 0 : 1 (EXE words) */
        PE_StoreU32(PE_LoadU32(args), (PE_LoadU32(GA_D_8009D2E8) & 1u) ? 0u : 1u);
        return 1;
    }
    if (fn == 0x800176E0u) { /* 19: *arg0 = u16(task+0xA) */
        PE_StoreU32(PE_LoadU32(args), PE_LoadU16(PE_LoadU32(GA_D_8009D300) + 0xAu));
        return 1;
    }
    if (fn == 0x80017968u) { /* 25: *arg0 = u16(actor+0x24) */
        PE_StoreU32(PE_LoadU32(args), PE_LoadU16(PE_LoadU32(GA_D_8009D2F0) + 0x24u));
        return 1;
    }
    if (fn == 0x80017928u) { /* 26: *arg0 = u8(actor+0xD) */
        PE_StoreU32(PE_LoadU32(args), PE_LoadU8(PE_LoadU32(GA_D_8009D2F0) + 0xDu));
        return 1;
    }
    if (fn == 0x80017948u) { /* 27: u8(actor+0xD) = *arg0 */
        PE_StoreU8(PE_LoadU32(GA_D_8009D2F0) + 0xDu, (uint8_t)PE_LoadU32(PE_LoadU32(args)));
        return 1;
    }
    if (fn == 0x80017A24u) { /* 29: *arg2 = *arg0 & (1 << *arg1) */
        PE_StoreU32(PE_LoadU32(args + 8u), PE_LoadU32(PE_LoadU32(args))
                    & (1u << (PE_LoadU32(PE_LoadU32(args + 4u)) & 31u)));
        return 1;
    }
    if (fn == 0x80017A78u) { /* 2B: D2E8 &= ~*arg0 */
        PE_StoreU32(GA_D_8009D2E8, PE_LoadU32(GA_D_8009D2E8) & ~PE_LoadU32(PE_LoadU32(args)));
        return 1;
    }
    if (fn == 0x80017AA4u) { /* 2C: *arg0 = D2E8 */
        PE_StoreU32(PE_LoadU32(args), PE_LoadU32(GA_D_8009D2E8));
        return 1;
    }
    if (fn == 0x80017D3Cu) { /* 3E: s16(actor+0x224) = upper halfword of *arg0 */
        PE_StoreU16(PE_LoadU32(GA_D_8009D2F0) + 0x224u, PE_LoadU16(PE_LoadU32(args) + 2u));
        return 1;
    }
    if (fn == 0x80017C8Cu) { /* 46: 661EC((s16)*a0, (s16)*a1, (u16)*a2, 8) */
        (void)func_800661EC((int)(int16_t)PE_LoadU16(PE_LoadU32(args)),
                            (int)(int16_t)PE_LoadU16(PE_LoadU32(args + 4u)),
                            (unsigned int)PE_LoadU16(PE_LoadU32(args + 8u)), 8u);
        return 1;
    }
    if (fn == 0x80017CC4u) { /* 47: *arg0 = ((BCF88 & 7) == 4) */
        PE_StoreU32(PE_LoadU32(args), (PE_LoadU32(0x800BCF88u) & 7u) == 4u ? 1u : 0u);
        return 1;
    }
    if (fn == 0x80017D18u) { /* 49: BCF88 = (BCF88 & ~7) | 0x80 */
        PE_StoreU32(0x800BCF88u, (PE_LoadU32(0x800BCF88u) & ~7u) | 0x80u);
        return 1;
    }
    if (fn == 0x80017EA4u) { /* 4D: actor+0x1C = *arg0 */
        PE_StoreU32(PE_LoadU32(GA_D_8009D2F0) + 0x1Cu, PE_LoadU32(PE_LoadU32(args)));
        return 1;
    }
    if (fn == 0x80018754u) { /* 69: D_800A76C4 |= 4 */
        PE_StoreU32(0x800A76C4u, PE_LoadU32(0x800A76C4u) | 4u);
        return 1;
    }
    if (fn == 0x80017FDCu) { /* 8A: D28C = 5 */
        PE_StoreU32(GA_D_8009D28C, 5u);
        return 1;
    }
    if (fn == 0x80017E9Cu || fn == 0x80019050u || fn == 0x80019058u ||
        fn == 0x800190B4u) /* 4C / 8D / 8E / 90: return-1 twins */
        return 1;
    if (fn == 0x800193B8u) { /* 99: 703F4() */
        func_800703F4();
        return 1;
    }
    if (fn == 0x80019298u) { /* CA: u16(actor+0x1E6) = *arg0 */
        PE_StoreU16(PE_LoadU32(GA_D_8009D2F0) + 0x1E6u, (uint16_t)PE_LoadU32(PE_LoadU32(args)));
        return 1;
    }
    {
        /* actor+0x98 flag leaves: 37 |=0x4000, 39 &=~0x4000, 42 &=~0x40,
         * 50 &=~0x100, 78 &=~0x20, B9 &=~1, BA |=1, C2 &=~0x400,
         * C8 &=~0x20000, C9 |=0x20000. */
        uint32_t set = 0u, clear = 0u;
        if (fn == 0x800196A0u) set = 0x4000u;
        else if (fn == 0x800196C4u) clear = 0x4000u;
        else if (fn == 0x80017DC0u) clear = 0x40u;
        else if (fn == 0x80017F20u) clear = 0x100u;
        else if (fn == 0x80018BC8u) clear = 0x20u;
        else if (fn == 0x80019904u) clear = 1u;
        else if (fn == 0x80019928u) set = 1u;
        else if (fn == 0x80019AE4u) clear = 0x400u;
        else if (fn == 0x80019C04u) clear = 0x20000u;
        else if (fn == 0x80019C28u) set = 0x20000u;
        if (set | clear) {
            pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
            PE_StoreU32(actor + 0x98u, (PE_LoadU32(actor + 0x98u) | set) & ~clear);
            return 1;
        }
    }
    if (fn == 0x80019A9Cu || fn == 0x800199F8u) { /* BE &=~8, C0 &=~0x10 on u16 actor+0x250 */
        pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
        uint16_t clear = fn == 0x80019A9Cu ? 8u : 0x10u;
        PE_StoreU16(actor + 0x250u, (uint16_t)(PE_LoadU16(actor + 0x250u) & ~clear));
        return 1;
    }
    /* SEW3: leaves whose callees now live in func_800659F8_port.c. */
    if (fn == 0x80017820u) { /* 23: 3746C((s16)*a0) close message by id */
        func_8003746C((int)(int16_t)PE_LoadU16(PE_LoadU32(args)));
        return 1;
    }
    if (fn == 0x80018B00u) { /* 72: 67678(*a0, *a1) */
        (void)func_80067678(PE_LoadU32(PE_LoadU32(args)), PE_LoadU32(PE_LoadU32(args + 4u)));
        return 1;
    }
    if (fn == 0x80018C88u) { /* 7C: 659F8(*a0, *a1) */
        (void)func_800659F8(PE_LoadU32(PE_LoadU32(args)), PE_LoadU32(PE_LoadU32(args + 4u)));
        return 1;
    }
    if (fn == 0x80018CB8u) { /* 7D: 65A60(*a0, *a1, *a2) */
        (void)func_80065A60(PE_LoadU32(PE_LoadU32(args)), PE_LoadU32(PE_LoadU32(args + 4u)),
                            PE_LoadU32(PE_LoadU32(args + 8u)));
        return 1;
    }
    if (fn == 0x80018CF0u || fn == 0x80018D20u) { /* 7E: 65A9C(*a0, *a1); 7F: 65A9C(*a0, ~*a1) */
        uint32_t bits = PE_LoadU32(PE_LoadU32(args + 4u));
        (void)func_80065A9C(PE_LoadU32(PE_LoadU32(args)), fn == 0x80018D20u ? ~bits : bits);
        return 1;
    }
    if (fn == 0x80015AF0u) return func_80015AF0(args); /* E7 */
    if (fn == 0x800197D0u) { func_800375B4(); return 1; } /* B4 */
    if (fn == 0x800197F0u) { func_800375C4(); return 1; } /* B5 */
    if (fn == 0x8001994Cu || fn == 0x8001998Cu) { /* BB: 676CC(4 args); BC: 67730(4 args) */
        uint32_t a0 = PE_LoadU32(PE_LoadU32(args)), a1 = PE_LoadU32(PE_LoadU32(args + 4u));
        uint32_t a2 = PE_LoadU32(PE_LoadU32(args + 8u)), a3 = PE_LoadU32(PE_LoadU32(args + 12u));
        if (fn == 0x8001994Cu) (void)func_800676CC(a0, a1, a2, a3);
        else                   (void)func_80067730(a0, a1, a2, a3);
        return 1;
    }
    if (fn == 0x80019D84u) return func_80019D84(args); /* D1 */
    if (fn == 0x80019D44u) { /* D0: 37454(4 x u16 args) */
        func_80037454(PE_LoadU16(PE_LoadU32(args)), PE_LoadU16(PE_LoadU32(args + 4u)),
                      PE_LoadU16(PE_LoadU32(args + 8u)), PE_LoadU16(PE_LoadU32(args + 12u)));
        return 1;
    }
    if (fn == 0x80019C4Cu || fn == 0x80015648u) {
        /* CB: 19C4C..19CEC (40 words). Heading from the current actor to
         * the 16.16 point (*a0, *a1): v = 79FB4((z - tz) >> 16,
         * (x - tx) >> 16); v = 0x1400 - v; if (v >= 0x1001) v -= 0x1000;
         * v -= (s16)actor+0x3A; if (v < 0) v += 0x1000; *a2 = v. v0=1.
         * 45: 15648..15790 (82 words): same math toward an ACTOR — *a0 == 0
         * selects D_8009D254, otherwise the first D20C-list actor with
         * +0xC == *a0, +0xD == *a1 and !(+0x98 & 0x10); none -> *a2 = -1. */
        pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
        uint32_t tx, tz;
        int32_t dx, dz, v;
        if (fn == 0x80019C4Cu) {
            tx = PE_LoadU32(PE_LoadU32(args));
            tz = PE_LoadU32(PE_LoadU32(args + 4u));
        } else {
            uint32_t type = PE_LoadU32(PE_LoadU32(args));
            pe_addr_t found;
            if (type == 0u) {
                found = PE_LoadU32(GA_D_8009D254);
            } else {
                uint32_t idb = PE_LoadU32(PE_LoadU32(args + 4u));
                found = PE_LoadU32(GA_D_8009D20C);
                while (found != 0u) {
                    /* src/func_80015648.c compares the bytes with the full
                     * int operands (no truncation). */
                    if ((uint32_t)PE_LoadU8(found + 0x0Cu) == type &&
                        (uint32_t)PE_LoadU8(found + 0x0Du) == idb &&
                        (PE_LoadU32(found + 0x98u) & 0x10u) == 0u)
                        break;
                    found = PE_LoadU32(found + 4u);
                }
            }
            if (found == 0u) {
                PE_StoreU32(PE_LoadU32(args + 8u), 0xFFFFFFFFu);
                return 1;
            }
            tx = PE_LoadU32(found + 0x28u);
            tz = PE_LoadU32(found + 0x30u);
        }
        dx = (int32_t)(PE_LoadU32(actor + 0x28u) - tx) >> 16;
        dz = (int32_t)(PE_LoadU32(actor + 0x30u) - tz) >> 16;
        v = 0x1400 - func_80079FB4(dz, dx);
        if (v >= 0x1001) v -= 0x1000;
        v -= (int32_t)(int16_t)PE_LoadU16(actor + 0x3Au);
        if (v < 0) v += 0x1000;
        PE_StoreU32(PE_LoadU32(args + 8u), (uint32_t)v);
        return 1;
    }
    if (fn == 0x80018864u) { /* 6D: matching C — 6F820(*a0, 0, *a1) write */
        (void)func_8006F820(PE_LoadU32(PE_LoadU32(args)), 0u, PE_LoadU32(PE_LoadU32(args + 4u)));
        return 1;
    }
    if (fn == 0x80018894u) { /* 6E: matching C — 6F820(*a0, 1, a1 pointer) read */
        (void)func_8006F820(PE_LoadU32(PE_LoadU32(args)), 1u, PE_LoadU32(args + 4u));
        return 1;
    }
    if (fn == 0x80019768u) { /* AF: matching C — 1ACE0(actor, (u16)*a0) */
        func_8001ACE0(PE_LoadU32(GA_D_8009D2F0), PE_LoadU16(PE_LoadU32(args)));
        return 1;
    }
    /* SEW8: game/boot/func_80013988_port.c */
    if (fn == 0x8001787Cu) return func_8001787C(args);   /* 18 */
    if (fn == 0x80014BA0u) return func_80014BA0(args);   /* 76 */
    if (fn == 0x80013E84u) return func_80013E84(args);   /* 36 */
    if (fn == 0x80014FD8u) return func_80014FD8(args);   /* 92 */
    if (fn == 0x800155FCu) return func_800155FC(args);   /* A5 */
    if (fn == 0x80019B08u) return func_80019B08();   /* C3 */
    if (fn == 0x80013988u) return func_80013988(args);   /* DB */
    if (fn == 0x80017CE8u) return func_80017CE8(args);   /* 48 */
    if (fn == 0x800199CCu) { /* BF: 199CC (11w) actor+0x250 |= 0x10; u16 actor+0x24E = *a0 */
        pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
        PE_StoreU16(actor + 0x250u, (uint16_t)(PE_LoadU16(actor + 0x250u) | 0x10u));
        PE_StoreU16(actor + 0x24Eu, (uint16_t)PE_LoadU32(PE_LoadU32(args)));
        return 1;
    }
    if (fn == 0x80019DB8u)return func_80019DB8(args);
    if (fn == 0x80019DF4u)return func_80019DF4(args);
    /* Any other handler with a verified host implementation: retail jalr. */
    if (PE_GuestCode_Resolve(fn))
        return PE_GuestCall("func_80017018", fn, 1u, args, 0u, 0u, 0u);
    /* Unported / empty table slot: explicit boundary. Retail would jalr
     * the entry; yielding here left the task suspended (delay 0). Retain
     * the opcode PC so the stop report names the exact script word. */
    fprintf(stderr, "[VM] unported script opcode fn=0x%08X at pc=0x%08X actor=0x%08X\n",
            (unsigned)fn, (unsigned)g_pe_17018_opcode_pc,
            (unsigned)PE_LoadU32(GA_D_8009D2F0));
    PE_StoreU32(GA_D_8009CE00, g_pe_17018_opcode_pc);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}

static int pe_17018_dispatch(pe_addr_t fn,pe_addr_t args)
{
    PE_M34StackOpcode(fn);
    int result=pe_17018_dispatch_body(fn,args);
    PE_M34StackOpcode(0u);
    return result;
}

static void pe_17018_walk_next(void)
{
    pe_addr_t next;

    next = PE_LoadU32(PE_LoadU32(GA_D_8009D300) + 0x24u);
    PE_StoreU32(GA_D_8009D300, next);
}

/* PE_VMFRAME_AUDIT=1 (fmv2 debug aid): prove nothing but this VM writes
 * the host arg-frame window 0x801FF400..0x801FF4EF.  (a) After each
 * dispatch the frame must still hold the decoded pointers (a load that
 * lands on it, as the movie overlay did at 0x80120F80, fails this);
 * (b) between dispatches the whole window must be unchanged. */
#define VMFRAME_WIN_BYTES 0xF0u
static int g_vmframe_audit = -1;
static uint8_t g_vmframe_snap[VMFRAME_WIN_BYTES];
static int g_vmframe_have_snap;
static unsigned long g_vmframe_ops, g_vmframe_bad;
static int vmframe_audit_on(void)
{
    if (g_vmframe_audit < 0)
        g_vmframe_audit = getenv("PE_VMFRAME_AUDIT") != NULL;
    return g_vmframe_audit;
}
static void vmframe_audit_pre(uint32_t op)
{
    if (!vmframe_audit_on() || !g_vmframe_have_snap)
        return;
    if (memcmp(PE_Translate(GA_VM_FRAME, VMFRAME_WIN_BYTES), g_vmframe_snap,
               VMFRAME_WIN_BYTES) != 0) {
        g_vmframe_bad++;
        fprintf(stderr, "[VMFRAME] foreign write between ops (next op %03X) bad=%lu\n",
                (unsigned)op, g_vmframe_bad);
    }
}
static void vmframe_audit_post(uint32_t op, const uint32_t *written)
{
    unsigned i;

    if (!vmframe_audit_on())
        return;
    g_vmframe_ops++;
    for (i = 0; i < 16u; i++)
        if (PE_LoadU32(GA_VM_FRAME + i * 4u) != written[i]) {
            g_vmframe_bad++;
            fprintf(stderr, "[VMFRAME] frame word %u clobbered during op %03X: %08X -> %08X bad=%lu\n",
                    i, (unsigned)op, (unsigned)written[i],
                    (unsigned)PE_LoadU32(GA_VM_FRAME + i * 4u), g_vmframe_bad);
            break;
        }
    memcpy(g_vmframe_snap, PE_Translate(GA_VM_FRAME, VMFRAME_WIN_BYTES),
           VMFRAME_WIN_BYTES);
    g_vmframe_have_snap = 1;
    if ((g_vmframe_ops % 200000u) == 0u)
        fprintf(stderr, "[VMFRAME] ops=%lu bad=%lu\n", g_vmframe_ops, g_vmframe_bad);
}

static void pe_17018_run(void)
{
    if (PE_Port_ShouldStop()) return;
    {
        /* day1 diagnostics (with PE_VM_TRACE): log persist[0]/persist[74]/token
         * whenever persist[0] changes, whatever wrote it. */
        static uint32_t last_p0 = 0xFFFFFFFFu; static int p0_init; static FILE *pf;
        if (!p0_init) { p0_init = 1; if (getenv("PE_VM_TRACE")) pf = stderr; }
        if (getenv("PE_DUMP_AYA")) {
            /* day1 diagnostics: dump Aya's record and weapon on each room change
             * and battle start (compare route vs normal-start New Game). */
            static uint32_t lt; static uint32_t lb;
            uint32_t tok = PE_LoadU32(0x8009D280u), b = PE_LoadU32(0x8009D1A0u) & 2u;
            pe_addr_t rec = PE_LoadU32(0x8009D278u);
            if (rec && (tok != lt || (b && !lb))) {
                pe_addr_t gun = PE_LoadU32(rec + 0x68u);
                fprintf(stderr, "AYAREC tok=%08X battle=%u rec=%08X:", tok, b, rec);
                for (unsigned k = 0; k < 0x70u; k += 4u) fprintf(stderr, " %08X", PE_LoadU32(rec + k));
                fprintf(stderr, "\nAYAGUN gun=%08X:", gun);
                if (PE_RangeIsRam(gun, 0x20u)) for (unsigned k = 0; k < 0x20u; k += 4u) fprintf(stderr, " %08X", PE_LoadU32(gun + k));
                fprintf(stderr, "\n");
            }
            lt = tok; lb = b;
        }
        if (pf && PE_LoadU32(0x800A77F0u) != last_p0) {
            last_p0 = PE_LoadU32(0x800A77F0u);
            fprintf(pf, "P0CHANGE persist0=%08X story=%08X token=%08X\n", last_p0,
                    PE_LoadU32(0x800A7918u), PE_LoadU32(0x8009D280u));
        }
    }
    pe_addr_t task;
    pe_addr_t actor;
    pe_addr_t pc;
    pe_addr_t imm_base;
    uint32_t word;
    uint32_t word2;
    uint32_t kinds;
    uint32_t argc;
    uint32_t op;
    uint32_t delay;
    unsigned int i;
    unsigned int kind;
    int again;
    int first_fetch;
    uint32_t written[16];

    for (;;) {
        task = PE_LoadU32(GA_D_8009D300);
        if (task == 0u)
            return;
        if ((PE_LoadU32(task + 8u) & 0x50u) != 0u) {
            pe_17018_walk_next();
            continue;
        }
        actor = PE_LoadU32(GA_D_8009D2F0);
        if ((PE_LoadU32(actor + 0x98u) & 0x1000u) != 0u &&
            (PE_LoadU16(task + 8u) & 0x80u) == 0u) {
            pe_17018_walk_next();
            continue;
        }
        if ((D_8009D1A0 & 0x100u) != 0u &&
            actor != PE_LoadU32(GA_D_8009D254) &&
            (PE_LoadU16(task + 8u) & 0x80u) == 0u) {
            pe_17018_walk_next();
            continue;
        }
        delay = PE_LoadU32(task + 0x10u);
        if (delay == 0u) {
            pe_17018_walk_next();
            continue;
        }
        delay -= 1u;
        PE_StoreU32(task + 0x10u, delay);
        if (delay != 0u) {
            pe_17018_walk_next();
            continue;
        }

        again = 1;
        first_fetch = 1;
        while (again) {
            if (first_fetch) {
                pc = PE_LoadU32(task);
                PE_StoreU32(GA_D_8009CE00, pc);
                first_fetch = 0;
            } else {
                pc = PE_LoadU32(GA_D_8009CE00);
            }
            word = PE_LoadU32(pc);
            word2 = PE_LoadU32(pc + 4u);
            argc = (word >> 13) & 0xFu;
            op = word & 0x1FFFu;
            kinds = word >> 17;
            imm_base = pc + 8u;
            PE_StoreU32(GA_D_8009CE00, imm_base + argc * 4u);
            vmframe_audit_pre(op);
            /* src/func_80017018.c: args[16] is a stack local that is NOT
             * cleared per opcode -- slots past argc keep the previous
             * opcode's pointers (the frame here persists the same way). */
            for (i = 0; i < 16u; i++)
                written[i] = PE_LoadU32(GA_VM_FRAME + i * 4u);
            for (i = 0; i < argc; i++) {
                kind = kinds & 7u;
                if (kind < 5u) {
                    uint32_t imm = PE_LoadU32(imm_base + i * 4u);
                    pe_addr_t decoded;

                    if (kind == 0u)
                        decoded = imm_base + i * 4u;
                    else if (kind == 1u)
                        decoded = actor + 0xACu + imm * 4u;
                    else if (kind == 2u)
                        decoded = GA_D_800A77F0 + imm * 4u;
                    else if (kind == 3u)
                        decoded = GA_D_8009DF70 + imm * 4u;
                    else
                        decoded = GA_D_800B6A80 + imm * 4u;
                    PE_StoreU32(GA_VM_FRAME + i * 4u, decoded);
                    written[i] = decoded;
                }
                kinds >>= 3;
                if (i == 4u)
                    kinds = word2;
            }
            g_pe_17018_opcode_pc = pc;
            again = pe_17018_dispatch(PE_LoadU32(GA_D_800910A0 + op * 4u),
                                      GA_VM_FRAME);
            if (PE_Port_ShouldStop()) {
                pe_addr_t current = PE_LoadU32(GA_D_8009D300);
                /* Preserve the dispatcher's selected PC, including the
                 * unported-opcode rewind, without walking later tasks. */
                if (current && PE_RangeIsRam(PE_RamCanonical(current, 4u), 4u))
                    PE_StoreU32(current, PE_LoadU32(GA_D_8009CE00));
                return;
            }
            vmframe_audit_post(op, written);
            {
                /* day1 diagnostics: PE_VM_TRACE=path logs every field-VM opcode
                 * (actor key, pc, op, handler, first 3 operands, result). Off by default. */
                static FILE *vt; static int vt_init;
                if (!vt_init) { const char *p = getenv("PE_VM_TRACE"); vt_init = 1; if (p && *p) vt = fopen(p, "w"); }
                if (vt) {
                    uint32_t o[4];
                    for (unsigned k = 0; k < 4u; k++)
                        o[k] = (k < argc && PE_RangeIsRam(written[k], 4u)) ? PE_LoadU32(written[k]) : 0xDEADu;
                    fprintf(vt, "vm tok=%08X key=%02X/%02X anim=%02X pc=%08X op=%03X fn=%08X ops=%X,%X,%X,%X at=%X,%X,%X,%X ret=%d\n",
                            PE_LoadU32(0x8009D280u), PE_LoadU8(actor + 0xCu), PE_LoadU8(actor + 0xDu),
                            PE_LoadU8(actor + 0xEu), pc, op, PE_LoadU32(GA_D_800910A0 + op * 4u),
                            o[0], o[1], o[2], o[3], written[0], written[1], written[2], written[3], again);
                }
            }
            if (again)
                task = PE_LoadU32(GA_D_8009D300);
        }
        /* src: D_8009D300->pc = D_8009CE00 -- the CURRENT task global,
         * which a handler may have changed before returning 0. */
        PE_StoreU32(PE_LoadU32(GA_D_8009D300), PE_LoadU32(GA_D_8009CE00));
        pe_17018_walk_next();
    }
}

void func_80017018(void)
{
    PE_M34StackVm(1);
    pe_17018_run();
    PE_M34StackVm(0);
}
