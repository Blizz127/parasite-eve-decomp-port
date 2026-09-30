/*
 * Field-menu Equipment sub-page input handler (installed by func_8004B13C into
 * window+0x2C).
 *
 * Original: [0x8004B394,0x8004B534), 0x1A0 bytes / 104 words, asm/disc1/37CD0.s.
 * This is the input side of the two-list equipment page; the draw side is
 * func_8004B214 (window+0x30) and the list draws are func_8004B534/4B55C.
 *
 *   index  = func_80063428(func_80062A20(node, 0))
 *   shift  = index * 8                      ; byte lane in the packed colour
 *   packed = func_800614A0()                ; D_8009D14C (4 packed colour bytes)
 *
 *   event & 0x1000: lane = (packed >> shift) & 0xFF; lane += 2;
 *                   if (lane >= 0xE9) lane = 0xE8;          ; clamp high
 *                   func_800614AC((packed & ~(0xFF<<shift)) | (lane<<shift));
 *                   func_8005267C(); return 1;
 *   event & 0x4000: lane = (packed >> shift) & 0xFF; lane -= 2;
 *                   if (lane < 0x20) lane = 0x20;           ; clamp low
 *                   func_800614AC(...same compose...);
 *                   func_8005267C(); return 1;
 *   event & 0x10000: list1 = func_80062A20(node, 1);
 *                    sel = func_80063428(list1);
 *                    if (sel == 1) func_800614AC(0x404040);
 *                    else if (sel != 0) {                  ; clear the pair and
 *                        list1+0x44 = 0; list1+0x48 = 0;   ; republish list0
 *                        func_80062CB8(list1);
 *                        func_80062A20(node,0)+0x44 = -1;
 *                    }
 *                    D_800C0E44 = func_800614A0();
 *                    func_80062F1C(node); func_800525EC(); return 1;
 *   event & 0x40:    func_80062F1C(node);
 *                    func_800614AC(D_8009D260);            ; restore the colour
 *                    func_80052634(); return 1;
 *
 * The `~2` clamp bounds are signed slti (lane is 0..255, so lane-2 can go to
 * -2 and lands on 0x20); the `+2` bound is signed slti against 0xE9.
 * func_800614A0/AC are the packed-colour getter/interpolator; both native.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

int func_8004B394(pe_addr_t node, uint32_t event)
{
    int32_t index = func_80063428(func_80062A20(node, 0u));
    unsigned shift = ((unsigned)index * 8u) & 31u;
    uint32_t packed = (uint32_t)func_800614A0();
    uint32_t mask = ~(0xFFu << shift);

    if (event & 0x1000u) {
        uint32_t lane = ((packed >> shift) & 0xFFu) + 2u;
        if (lane >= 0xE9u) lane = 0xE8u;
        func_800614AC((int32_t)((packed & mask) | (lane << shift)));
        func_8005267C();
        return 1;
    }
    if (event & 0x4000u) {
        int32_t lane = (int32_t)((packed >> shift) & 0xFFu) - 2;
        if (lane < 0x20) lane = 0x20;
        func_800614AC((int32_t)((packed & mask) | ((uint32_t)lane << shift)));
        func_8005267C();
        return 1;
    }
    if (event & 0x10000u) {
        pe_addr_t list1 = func_80062A20(node, 1u);
        int32_t selected = func_80063428(list1);

        if (selected == 1) func_800614AC(0x404040);
        else if (selected != 0) {
            PE_StoreU32(list1 + 68u, 0u);
            PE_StoreU32(list1 + 72u, 0u);
            func_80062CB8(list1);
            PE_StoreU32(func_80062A20(node, 0u) + 68u, UINT32_MAX);
        }
        PE_StoreU32(0x800C0E44u, (uint32_t)func_800614A0());
        func_80062F1C(node);
        func_800525EC();
        return 1;
    }
    if (event & 0x40u) {
        func_80062F1C(node);
        func_800614AC((int32_t)PE_LoadU32(0x8009D260u));
        func_80052634();
        return 1;
    }
    return 1;
}
