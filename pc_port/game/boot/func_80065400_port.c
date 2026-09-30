/*
 * PE-CH1 — func_80065400: mailbox queue drain (translated retail).
 *
 * Complete retail body (117 words / 0x1D4, exe 0x80065400–0x800655D0,
 * file offset 0x55C00). Words dumped from SHA-1-exact
 * build/disc1.candidate.exe (452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 * yaml places this inside [0x55430, asm] immediately after 653B8.
 * This worktree has no era/asm split, so the leaf cannot yet be a
 * matching src/ unit. Native port follows the 653B8 / 17764 family.
 *
 * Sole jal: func_8003F3C4 @ 0x8003F4E8 (nop delay). Two jal
 * func_80012700 sites (0x80065484 serial arm, 0x8006553C type+id).
 * Spawn is the real PE-CH1 translation in func_80012700_port.c.
 *
 * ROM (gp = 0x8009CD70; 0x44($gp) = D_8009CDB4):
 *
 *   if count == 0: sb 0; return
 *   for i = 0 .. ; (i&0xFF) < lbu count; i++:
 *     rec = D_800A3180 + (i&0xFF)*12
 *     extra_b = lbu rec+4
 *     if extra_b != 0:                    # serial arm
 *       walk D_8009D20C via actor+4
 *       if lhu(actor+0x24)==extra_b:
 *         if lw(actor+0x19C)==0: stop walk
 *         else: deliver; stop walk        # one hit max
 *     else:                               # type+id arm (0x1C path)
 *       walk D_8009D20C via actor+4
 *       if lbu(actor+0x0C)==lhu(rec+0)
 *          and lw(actor+0x19C)!=0
 *          and lbu(actor+0x0D)==lbu(rec+2):
 *         deliver                         # continue walk
 *   sb $zero, 0x44($gp)                   # count=0; records kept
 *
 * deliver (duplicated in ROM at both jal sites):
 *   task = func_80012700(actor+0x19C, 0)
 *   lhu/ori/sh task+0x08 |= 4
 *   sw  rec+8  -> task+0x0C
 *   sw  lbu rec+3 -> task+0x14
 *   old = actor+0xA8
 *   sw  old -> task+0x24                  # even when old==0
 *   if old: sw task -> old+0x28
 *   sw  task -> actor+0xA8
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800A3180 0x800A3180u
#define GA_D_8009CDB4 0x8009CDB4u
#define GA_D_8009D20C 0x8009D20Cu
#define TABLE_STRIDE  0xCu

static void deliver(pe_addr_t actor, pe_addr_t rec)
{
    pe_addr_t task;
    pe_addr_t old;
    unsigned int flags;

    task = func_80012700(PE_LoadU32(actor + 0x19Cu), 0u);
    flags = PE_LoadU16(task + 8u);
    PE_StoreU16(task + 8u, (uint16_t)(flags | 4u));
    PE_StoreU32(task + 0xCu, PE_LoadU32(rec + 8u));
    PE_StoreU32(task + 0x14u, PE_LoadU8(rec + 3u));
    old = PE_LoadU32(actor + 0xA8u);
    PE_StoreU32(task + 0x24u, old);
    if (old != 0u)
        PE_StoreU32(old + 0x28u, task);
    PE_StoreU32(actor + 0xA8u, task);
}

void func_80065400(void)
{
    unsigned int count;
    unsigned int i;

    count = PE_LoadU8(GA_D_8009CDB4);
    if (count != 0u) {
        i = 0u;
        for (;;) {
            pe_addr_t rec;
            unsigned int extra_b;
            pe_addr_t actor;

            rec = GA_D_800A3180 + (i & 0xFFu) * TABLE_STRIDE;
            extra_b = PE_LoadU8(rec + 4u);
            actor = PE_LoadU32(GA_D_8009D20C);

            if (extra_b != 0u) {
                while (actor != 0u) {
                    if (PE_LoadU16(actor + 0x24u) == extra_b) {
                        if (PE_LoadU32(actor + 0x19Cu) != 0u)
                            deliver(actor, rec);
                        break;
                    }
                    actor = PE_LoadU32(actor + 4u);
                }
            } else {
                while (actor != 0u) {
                    if (PE_LoadU8(actor + 0x0Cu) == PE_LoadU16(rec + 0u)
                        && PE_LoadU32(actor + 0x19Cu) != 0u
                        && PE_LoadU8(actor + 0x0Du) == PE_LoadU8(rec + 2u)) {
                        deliver(actor, rec);
                    }
                    actor = PE_LoadU32(actor + 4u);
                }
            }

            i++;
            count = PE_LoadU8(GA_D_8009CDB4);
            if ((i & 0xFFu) >= count)
                break;
        }
    }

    PE_StoreU8(GA_D_8009CDB4, 0u);
}
