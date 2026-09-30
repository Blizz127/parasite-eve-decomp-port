/* Retail equipment records: 51980..51CC4 and 51E64..5218C.
 * Weapon/armor parameters come from the selected inventory entries, including
 * signed bonuses, integer range/PE curves, and the retail ability bitfields.
 * Authority: asm/disc1/420A8.s and 42664.s; SHA-exact disc1 executable.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_port_compat.h"

/* Reload from the ammo record associated with the equipped weapon. 574A8
 * preserves the retail signed capacity arithmetic and 16-bit quantities. */
int32_t func_800574A8(void)
{
    pe_addr_t item=func_8005332C((int8_t)PE_LoadU8(0x800C0E20u)),ammo;
    unsigned kind=PE_LoadU8(item+6u);
    int32_t capacity=PE_LoadU8(item+9u)+(int16_t)PE_LoadU16(item+0x12u);
    int32_t loaded=PE_LoadU16(item+0xAu),available,amount;
    if (kind>0u && kind<8u) ammo=0x800A1E64u+(kind>4u?kind-5u:0u)*32u;
    else ammo=kind>=19u?0x800A1E64u+(kind-19u)*32u:0x800A1E44u;
    if (capacity>=1000) capacity=999;
    available=PE_LoadU16(ammo+0xAu);
    amount=capacity-loaded<available?capacity-loaded:available;
    PE_StoreU16(ammo+0xAu,(uint16_t)(available-amount));
    PE_StoreU16(item+0xAu,(uint16_t)(loaded+amount));
    return amount;
}

void func_80051510(void)
{
    pe_addr_t aya=PE_LoadU32(0x8009D254u);
    if (aya) {
        pe_addr_t record=PE_LoadU32(aya);
        if (record) {
            PE_StoreU16(0x800C0E08u,PE_LoadU16(record+0xCu));
            if (PE_LoadU32(record+0x68u)) {
                uint32_t saved=func_80052F0C();
                pe_addr_t item;
                func_80052E30(0u);
                item=func_8005332C((int8_t)PE_LoadU8(0x800C0E20u));
                if (item) PE_StoreU16(item+0xAu,(uint16_t)(PE_LoadU32(PE_LoadU32(record+0x68u)+0xCu)&0x3FFu));
                func_80052E30(saved);
            }
        }
    }
}

int32_t func_800518A8(pe_addr_t out)
{
    int32_t amount;
    func_80051510();
    amount=func_800574A8();
    func_80051980(0,out);
    return amount;
}

/* The retail restoring square-root loop uses a signed comparison and wrapped
 * 32-bit shifts. Preserve these even for a negative equipment bonus. */
static uint32_t equipment_root(uint32_t remainder)
{
    uint32_t root = 0u;
    int shift;
    for (shift = 30; shift >= 0; shift -= 2) {
        uint32_t trial = ((root << 2u) + 1u) << shift;
        int take = (int32_t)remainder >= (int32_t)trial;
        root <<= 1u;
        if (take) {
            remainder -= trial;
            root |= 1u;
        }
    }
    return root;
}

static int32_t equipment_parameter(pe_addr_t item, uint32_t base,
                                    uint32_t bonus)
{
    int32_t value = PE_LoadU8(item + base) +
                    (int16_t)PE_LoadU16(item + bonus);
    return value < 1000 ? value : 999;
}

void func_80051980(int32_t unused, pe_addr_t out)
{
    pe_addr_t item;
    uint32_t packed, abilities, category, kind, i;
    int32_t range;
    (void)unused;
    func_80052E30(0u);
    item = func_8005332C((int8_t)PE_LoadU8(0x800C0E20u));
    /* Original weapon initialization dereferences the lookup result even when
     * it is zero. Physical low RAM aliases KSEG0; preserve that read, not a
     * host null dereference or an invented empty-weapon shortcut. */
    if (item < 0x200000u) item |= 0x80000000u;

    PE_StoreU16(out, (uint16_t)equipment_parameter(item, 7u, 0xEu));
    range = equipment_parameter(item, 8u, 0x10u);
    PE_StoreU16(out + 2u, (uint16_t)equipment_root((uint32_t)range * 22500u));
    category = PE_LoadU8(item + 6u);
    PE_StoreU16(out + 6u, (uint16_t)category);
    packed = (PE_LoadU32(out + 0xCu) & 0xFFFFFC00u) |
             (PE_LoadU16(item + 0xAu) & 0x3FFu);
    packed = (packed & 0xFFF003FFu) |
             (((uint32_t)equipment_parameter(item, 9u, 0x12u) & 0x3FFu) << 10u);
    if (category > 0u && category < 8u)
        kind = category > 4u ? category - 4u : 1u;
    else
        kind = category >= 19u ? category - 18u : 0u;
    PE_StoreU32(out + 0xCu, (packed & 0xFFCFFFFFu) | ((kind & 3u) << 20u));
    abilities = 0x11u;
    PE_StoreU32(out + 0x10u, abilities);
    for (i = 0u; i < PE_LoadU8(item + 0x14u); i++) {
        uint32_t command = PE_LoadU8(item + 0x15u + i) & 31u;
        switch (command) {
        case 1u: case 2u: case 3u: case 4u: case 5u:
            abilities = (abilities & ~15u) | (PE_LoadU8(0x800923D0u + command) & 15u);
            break;
        case 6u: case 7u: case 8u:
            abilities = (abilities & ~0xC0u) | ((command - 5u) << 6u);
            break;
        case 9u: abilities |= 0x100u; break;
        case 10u: abilities |= 0x200u; break;
        case 11u: abilities |= 0x400u; break;
        case 12u: abilities |= 0x800u; break;
        case 13u: abilities |= 0x1000u; break;
        case 14u: abilities |= 0x8000u; break;
        case 15u: abilities |= 0x10000u; break;
        case 16u: case 17u:
            abilities = (abilities & ~0x6000u) | ((command - 15u) << 13u);
            break;
        case 18u: abilities |= 0x20000u; break;
        case 19u: case 20u:
            abilities = (abilities & ~0x30u) | ((command - 17u) << 4u);
            break;
        default: break;
        }
        PE_StoreU32(out + 0x10u, abilities);
    }
    if ((abilities & 0x6000u) == 0x2000u)
        PE_StoreU16(out, (uint16_t)((int16_t)PE_LoadU16(out) >> 1));
}

void func_80051E64(pe_addr_t out)
{
    pe_addr_t item;
    PE_StoreU32(out + 4u, 0u);
    func_80052E30(0u);
    item = func_8005332C((int8_t)PE_LoadU8(0x800C0E22u));
    if (item) {
        uint32_t packed = PE_LoadU32(out), abilities = 0u, i;
        int32_t parameter, curve;
        packed = (packed & 0xFFFFFC00u) |
                 ((uint32_t)equipment_parameter(item, 7u, 0xEu) & 0x3FFu);
        packed = (packed & 0xFFF003FFu) |
                 (((uint32_t)equipment_parameter(item, 8u, 0x10u) & 0x3FFu) << 10u);
        parameter = equipment_parameter(item, 9u, 0x12u);
        curve = parameter < 85 ?
            (int32_t)equipment_root((uint32_t)parameter * 3000u) / 10 :
            (249 * parameter / 208 + 402) / 10;
        PE_StoreU32(out, (packed & 0xF00FFFFFu) | (((uint32_t)curve & 255u) << 20u));
        for (i = 0u; i < PE_LoadU8(item + 0x14u); i++) {
            uint32_t command = PE_LoadU8(item + 0x15u + i) & 31u;
            switch (command) {
            case 1u: case 2u: case 3u: case 4u: case 5u:
                abilities |= 1u << (command - 1u); break;
            case 6u: abilities |= 0x4000u; break;
            case 7u: abilities |= 0x8000u; break;
            case 8u: case 9u: case 10u:
                abilities = (abilities & ~0x1E0u) | (1u << (command - 8u + 5u));
                break;
            case 11u: abilities |= 0x800u; break;
            case 12u: abilities |= 0x2000u; break;
            case 13u: abilities |= 0x10000u; break;
            case 14u: abilities |= 0x200u; break;
            case 15u: abilities |= 0x20000u; break;
            case 16u: abilities |= 0x400u; break;
            case 17u: abilities |= 0x1000u; break;
            default: break;
            }
            PE_StoreU32(out + 4u, abilities);
        }
    } else {
        PE_StoreU32(out, PE_LoadU32(out) & 0xF0000000u);
    }
    func_80051CC4();
}
