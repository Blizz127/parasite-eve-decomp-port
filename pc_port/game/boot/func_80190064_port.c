/*
 * Title-menu pad/update leaf func_80190064.
 *
 * Retail overlay: [0x80190064, 0x80190660), 383 words / 0x5FC.
 * Reads remapped buttons via func_8005E038, walks the title object list at
 * D_801D1370 by type (+0x2C), handles Start intro, cursor motion, and Circle
 * confirm (D_801D1380 = 1001). Edge state lives in D_801D11B8.
 *
 * Disassembly: PE.IMG LBA 1013 + 0x03D2*0x800, load VA 0x8018EFF0.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_port_compat.h"

#define GA_TITLE_LIST   0x801D1370u
#define GA_PAD_PREV     0x801D11B8u
#define GA_TITLE_SELECT 0x801D1380u
#define GA_CB_START     0x801931BCu
#define GA_CB_TYPE6_A   0x801930D8u
#define GA_CB_TYPE6_B   0x8018F958u
#define GA_CB_TYPE6_C   0x8019316Cu

static pe_addr_t title_find_type(unsigned type)
{
    pe_addr_t node = PE_LoadU32(GA_TITLE_LIST);

    while (node != 0u) {
        if (PE_LoadU32(node + 0x2Cu) == type)
            return node;
        node = PE_LoadU32(node);
    }
    return 0u;
}

/* func_8003FFCC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8003FFCC_port.c (src/func_8003FFCC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80190064(void)
{
    uint32_t buttons = func_8005E038();
    pe_addr_t node;
    pe_addr_t other;
    int16_t y;

    if ((buttons & 0x800u) != 0u && (PE_LoadU32(GA_PAD_PREV) & 0x800u) == 0u) {
        node = title_find_type(2u);
        if (node != 0u && (int32_t)PE_LoadU32(node + 0x1Cu) >= 0x81) {
            node = title_find_type(1u);
            if (node != 0u && PE_LoadU32(node + 0x24u) == 0u) {
                PE_StoreU32(node + 0x24u, 1u);
                PE_StoreU32(node + 0x28u, 8u);
                PE_StoreU32(node + 0x14u, GA_CB_START);
                PE_StoreU32(GA_TITLE_SELECT, 0u);
                func_800525EC();
            }
        }
    }

    node = title_find_type(5u);
    if (node == 0u)
        goto done;
    if (PE_LoadU32(node + 0x0Cu) != 0u)
        goto done;

    if (func_80042770(0u) != 0u || func_80042770(1u) != 0u) {
        y = (int16_t)PE_LoadU16(node + 0x06u);
        if (y < 200) {
            PE_StoreU16(node + 0x06u, (uint16_t)(y + 1));
            other = title_find_type(4u);
            if (other != 0u)
                PE_StoreU32(other + 0x20u, 16u);
            PE_StoreU32(GA_TITLE_SELECT, 0u);
        }
        if ((int16_t)PE_LoadU16(node + 0x06u) == 200 && func_8003FFCC() != 0u) {
            other = title_find_type(6u);
            if (other != 0u && PE_LoadU32(other + 0x20u) == 0u) {
                PE_StoreU32(other + 0x0Cu, GA_CB_TYPE6_A);
                PE_StoreU32(other + 0x10u, GA_CB_TYPE6_B);
                PE_StoreU32(other + 0x20u, 20u);
                PE_StoreU32(other + 0x24u, 1u);
            }
        }
    } else {
        y = (int16_t)PE_LoadU16(node + 0x06u);
        if (y >= 181) {
            PE_StoreU16(node + 0x06u, (uint16_t)(y - 1));
            other = title_find_type(7u);
            if (other != 0u) {
                int16_t oy = (int16_t)PE_LoadU16(other + 0x06u);
                if ((int16_t)PE_LoadU16(node + 0x06u) < oy)
                    PE_StoreU16(other + 0x06u, PE_LoadU16(node + 0x06u));
            }
            other = title_find_type(4u);
            if (other != 0u)
                PE_StoreU32(other + 0x20u, (uint32_t)(int32_t)-16);
            PE_StoreU32(GA_TITLE_SELECT, 0u);
        }
    }

    if (func_8003FFCC() == 0u) {
        other = title_find_type(6u);
        if (other != 0u) {
            if (PE_LoadU32(other + 0x20u) == 84u)
                PE_StoreU32(other + 0x0Cu, GA_CB_TYPE6_C);
            if (PE_LoadU32(other + 0x1Cu) != 0u)
                PE_StoreU32(other + 0x24u, (uint32_t)(int32_t)-1);
            {
                pe_addr_t t7 = title_find_type(7u);
                if (t7 != 0u && (int16_t)PE_LoadU16(t7 + 0x06u) == 140)
                    PE_StoreU16(t7 + 0x06u, 160u);
            }
        }
    }

    node = title_find_type(7u);
    if (node == 0u)
        goto done;

    if ((buttons & 0x20u) != 0u) {
        y = (int16_t)PE_LoadU16(node + 0x06u);
        if (y == 160 || y == 180 || y == 200 || y == 140) {
            PE_StoreU32(GA_TITLE_SELECT, 1001u);
            func_800525EC();
        }
    }

    if ((buttons & 0x1000u) != 0u && (PE_LoadU32(GA_PAD_PREV) & 0x1000u) == 0u) {
        y = (int16_t)PE_LoadU16(node + 0x06u);
        other = title_find_type(6u);
        if (other != 0u) {
            if (PE_LoadU32(other + 0x1Cu) == 256u) {
                if (y < 141)
                    goto skip_up;
            } else if (y < 161) {
                goto skip_up;
            }
        }
        PE_StoreU32(GA_TITLE_SELECT, 0u);
        PE_StoreU16(node + 0x06u, (uint16_t)(y - 20));
        func_8005267C();
    }
skip_up:

    if ((buttons & 0x4000u) != 0u && (PE_LoadU32(GA_PAD_PREV) & 0x4000u) == 0u) {
        y = (int16_t)PE_LoadU16(node + 0x06u);
        other = title_find_type(5u);
        if (other != 0u) {
            int16_t limit = (int16_t)PE_LoadU16(other + 0x06u) - 20;
            if (limit < y)
                goto done_pad;
        }
        PE_StoreU32(GA_TITLE_SELECT, 0u);
        PE_StoreU16(node + 0x06u, (uint16_t)(y + 20));
        func_8005267C();
    }

done_pad:
done:
    PE_StoreU32(GA_PAD_PREV, buttons);
}
