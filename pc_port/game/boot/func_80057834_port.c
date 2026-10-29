/* Game inventory-use adapter based on the existing matched 80057834 lowering.
 * Retains the guest frame and normal ordering while propagating stop before
 * item removal. See native-inventory-use-stop-20261028/REPORT.md. */
#include "pe_guest_decomp.h"
#include "game_port.h"

/* host prototypes of decomp-derived callees (generated TUs) */
void func_80048918(int a0, int a1, int a2);
void func_8004F23C(void);
pe_addr_t func_80051098(void);
void func_800512AC(int cmd, pe_addr_t src);
void func_800516B4(int32_t item);
void func_80055760(void);
int32_t func_80057D30(int32_t index);
pe_addr_t func_8005DB44(unsigned int index);

void func_80057834(int32_t a_i);

static void pe_lowered_func_80057834(pe_addr_t pe_fp, int32_t a_i)
{
    uint32_t v_p;
    uint32_t v_res;
    uint32_t v_r;
    /* local: guest frame +0x0 (4 bytes) */
    int32_t v_v;
    int32_t v_w;
    if ((((((int32_t)(a_i) >= (int32_t)(0))) && (((int32_t)(a_i) < (int32_t)(((int32_t)PE_LoadU32(0x8009D050u)))))))) {
        v_v = (int32_t)(((int16_t)PE_LoadU16((((uint32_t)PE_LoadU32(0x8009D048u)) + (uint32_t)(a_i) * 2u))));
        v_w = (int32_t)(v_v);
        if ((((uint32_t)(((uint32_t)(((int32_t)((uint32_t)(v_v) - (uint32_t)(256)))))) < (uint32_t)(128)))) {
            v_res = (uint32_t)(((uint32_t)(((int32_t)((uint32_t)(((int32_t)((uint32_t)(v_v) << (5)))) + (uint32_t)(((int32_t)(0x800BEEACu))))))));
            goto done;
        }
        if ((((uint32_t)(((uint32_t)(((int32_t)((uint32_t)(v_v) - (uint32_t)(1)))))) < (uint32_t)(255)))) {
            v_res = (uint32_t)(((uint32_t)func_8005DB44(((int32_t)((uint32_t)(v_v) - (uint32_t)(1))))));
            goto done;
        }
        if ((((uint32_t)(((uint32_t)(((int32_t)((uint32_t)(v_w) - (uint32_t)(512)))))) < (uint32_t)(9)))) {
            v_res = (uint32_t)(((uint32_t)(((int32_t)((uint32_t)(((int32_t)((uint32_t)(v_w) << (5)))) + (uint32_t)(((int32_t)(0x8009DE64u))))))));
            goto done;
        }
        v_res = (uint32_t)(0);
        done:;
        v_p = (uint32_t)(v_res);
    } else {
        v_p = (uint32_t)(0);
    }
    if ((((int32_t)(((int32_t)PE_LoadU32(0x8009D028u))) != (int32_t)(0)))) {
        v_r = (uint32_t)(((uint32_t)func_80051098()));
        PE_StoreU32((v_r + (uint32_t)(0) * 4u), (uint32_t)(0));
        PE_StoreU32((v_r + (uint32_t)(1) * 4u), (uint32_t)(((int16_t)PE_LoadU16((((uint32_t)PE_LoadU32(0x8009D048u)) + (uint32_t)(a_i) * 2u)))));
        PE_StoreU32((v_r + (uint32_t)(2) * 4u), (uint32_t)(a_i));
        PE_StoreU32(pe_fp, (uint32_t)(((int16_t)PE_LoadU16((((uint32_t)PE_LoadU32(0x8009D048u)) + (uint32_t)(a_i) * 2u)))));
        (void)(((void)func_800512AC(0, pe_fp)));
        (void)(((void)func_80057D30(a_i)));
        return;
    }
    switch ((int32_t)(((uint8_t)PE_LoadU8((v_p + (uint32_t)(14) * 1u))))) {
        case 1:
            (void)(((void)func_800516B4(((int16_t)PE_LoadU16((((uint32_t)PE_LoadU32(0x8009D048u)) + (uint32_t)(a_i) * 2u))))));
            if (PE_Port_ShouldStop()) return;
            (void)(((void)func_80057D30(a_i)));
            (void)(((void)func_80055760()));
            break;
        case 2:
            (void)(((void)func_8004F23C()));
            break;
        case 4:
        case 5:
        case 6:
            (void)(((void)func_80048918(254, a_i, ((int32_t)((uint32_t)0 - (uint32_t)(1))))));
            break;
        case 12:
        case 13:
        case 14:
            (void)(((void)func_80048918(512, a_i, ((int32_t)((uint32_t)0 - (uint32_t)(1))))));
            break;
    }
}

void func_80057834(int32_t a_i)
{
    const pe_addr_t pe_fp = PE_GuestFrame_Enter(8u);
    pe_lowered_func_80057834(pe_fp, a_i);
    PE_GuestFrame_Leave(pe_fp, 8u);
}
