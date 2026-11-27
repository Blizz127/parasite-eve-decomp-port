/* Original controller queries/configuration used by 8003EB04.
 * Combined source SHA256 fa58951f31f09efabde626872321aeacd180bc41e70fdcafcec1eea1cc446918.
 * The callbacks installed by original 844E4/84B44 run directly; any other
 * hook value goes through the guest-code registry like retail's jalr, and a
 * value with no C there stops at a named boundary instead of inventing
 * state. */
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_guestcode.h"

/* func_80084B20: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80084B20_port.c (src/func_80084B20.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80084F8C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80084F8C_port.c (src/func_80084F8C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* Retail's jalr through D_8009B738 (0x800825D0 and siblings) passes the
 * caller's port in $a0 and dereferences the returned record unchecked.
 * $a1..$a3 still hold the caller's own arguments there; the hooks take one
 * parameter and the host passes 0.  Returns 0 after a stop. */
static pe_addr_t controller_record(uint32_t port)
{
    pe_addr_t hook=PE_LoadU32(0x8009B738u);
    int record=0;
    if(hook==0x80084B20u) return func_80084B20(port);
    if(!PE_GuestHook_Call("D_8009B738_record_hook","libpad",hook,1u,port,
                          0u,0u,0u,&record) || PE_Port_ShouldStop()) return 0;
    if(!record) {
        PE_GuestHook_Refuse("D_8009B738_null_record","libpad",hook,1u,port,
                            0u,0u,0u,"the record hook returned NULL, which "
                            "retail dereferences in low memory");
        return 0;
    }
    return (pe_addr_t)(uint32_t)record;
}
/* D_8009B740's jalr (0x80083BD4 and siblings) passes the record in $a0;
 * the caller's own arguments stay in $a1/$a2 and the host passes 0.
 * Returns busy (1) after a stop, so the callers write nothing. */
static int controller_busy(pe_addr_t record)
{
    pe_addr_t hook=PE_LoadU32(0x8009B740u);
    int busy=1;
    if(hook==0x80084F8Cu) return func_80084F8C(record);
    if(!PE_GuestHook_Call("D_8009B740_busy_hook","libpad",hook,1u,record,
                          0u,0u,0u,&busy) || PE_Port_ShouldStop()) return 1;
    return busy;
}
/* PadGetState (0x800825C0) is the clean-room pe_plat_pad_get_state
 * (platform/pe_plat_pad.c); it reaches the record through this host service. */
uint32_t pe_plat_pad_record(uint32_t port)
{
    return controller_record(port);
}
/* PadInfoMode (0x80082680) is the clean-room pe_plat_pad_info_mode
 * (platform/pe_plat_pad_info.c). */
/* PadSetAct / PadSetActAlign / PadSetMainMode (0x80082974 / 0x800828F4 /
 * 0x8008292C) are the clean-room setters in platform/pe_plat_pad_set.c; the
 * internal 0x80083BB8 / 0x80083D04 they used are no longer linked.  The
 * setters reach the busy check through this host service. */
int pe_plat_pad_busy(uint32_t record)
{
    return controller_busy(record);
}
/* func_800835A4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800835A4_port.c (src/func_800835A4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* Counts 800828F4 entries: the original always reaches 80083BB8, whose ra
 * save is stack residue read by the M0000I menu list (func_801942FC). */
uint32_t PE_ControllerAlignCalls;
