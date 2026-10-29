/*
 * func_80069B08 — retail disc-swap screen ("insert disc N"), hand port.
 * VRAM 0x80069B08, 0x5E0 bytes; written from the matched C (src/
 * func_80069B08.c, asm/disc1/58518.s).  Called by the main loop
 * (bootstrap/func_8001220C_port.c) with a0 = wanted disc (1 or 2) when the
 * story word needs the other disc or New Game init runs without disc 2.
 *
 * Flow (retail): read the two message-graphics chunks of PE.IMG
 * (D_800930D8 sector ranges into the D_800B0DD8+0x8C/+0x94 buffers), set up
 * the display, then a per-frame state machine:
 *   0 drive ready and queue empty -> CdlStop (8);  1 wait for it (2: show
 *   "insert disc N" and go to 2; 5/6: retry);  2 wait for the lid to open
 *   (status 0x10);  3 wait for CdReady (1: "reading" message, 180 frames,
 *   then 4; 3: 180 frames, state 6);  4 re-mount (func_800698D4): the wanted
 *   disc's D_800B0DCD bit ends the loop, the wrong disc waits 180 frames and
 *   asks again (9), mount errors show message 5 (7/8);  5-9 timers.
 * Then clear the screen, D_800B0DCD = wanted, wait for CdReady and latch
 * D_800B0DD4.
 *
 * Host pieces (platform/pe_drive_tray.h): the other disc is the user's
 * --disc2-image; the tray opens the lid, swaps the active disc and closes
 * it while state 2 waits, and the CdlStop completes immediately with the CD
 * device disabled (fmv-lane precedent, func_80080DC4).  With the device
 * enabled the retail queue path runs (func_8007EE84).  Without a spare disc
 * for `a0` the screen is a CPU_BOUNDARY STOP, never a silent return.
 * Locals that callees take by guest address (the F418 response buffer and
 * the message list word) live in a guest frame like retail's stack.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "pe_drive_tray.h"
#include "pe_bootstrap.h"
#include "game_port.h"

unsigned char func_8007F788(void);

#define LOC_SIZE 0x18u
#define LOC_RESP 0x00u   /* retail loc.buf18 (F418 response bytes) */
#define LOC_LIST 0x10u   /* retail loc.t20 (message list word) */

static pe_addr_t Packets(void)
{
    return PE_LoadU32(0x800B0E38u + (PE_LoadU32(0x8009CDDCu) << 2));
}

/* One PE.IMG chunk: issue until accepted, then poll; -1 re-issues. */
static int ReadChunk(pe_addr_t range, pe_addr_t dest_slot)
{
    for (;;) {
        int lba = (int)PE_LoadU32(0x800B0DD8u) + PE_LoadU16(range);
        int sectors = PE_LoadU16(range + 2u) - PE_LoadU16(range);
        while (func_8006E6A8(lba, PE_LoadU32(dest_slot), sectors) == -1)
            if (PE_Port_ShouldStop()) return 0;
        for (;;) {
            int v = func_8006E7E8();
            if (v == 0) return 1;
            if (v == -1) break;
            if (PE_Port_ShouldStop()) return 0;
        }
        if (PE_Port_ShouldStop()) return 0;
    }
}

void func_80069B08(int a0)
{
    pe_addr_t loc;
    RECT rect;
    unsigned s2 = 0;
    int s6 = 0;
    uint32_t s7 = 0xFFFFFFFFu;
    int s1 = 0xB4;
    int v1;
    uint32_t bank;

    if (!PE_Tray_CanServe(a0)) {
        PE_Port_DiscSwapBoundary(a0);
        return;
    }
    PE_Tray_Begin(a0);
    loc = PE_GuestFrame_Enter(LOC_SIZE);
    PE_Fill(loc, LOC_SIZE, 0u);
    PE_StoreU16(loc + LOC_LIST, 0xFFFFu);
    func_80073A44(0);
    func_80074D28(0);

    if (!ReadChunk(0x800930D8u, 0x800B0DD8u + 0x8Cu) ||
        !ReadChunk(0x800930DAu, 0x800B0DD8u + 0x94u))
        goto leave;

    func_8003E974();
    func_800371B0(PE_LoadU32(0x800B0E6Cu));
    rect.x = 0; rect.y = 0; rect.w = 0x140; rect.h = 0x1C0;
    func_80074F44(&rect, 0, 0, 1);
    func_800718D0(PE_LoadU32(0x800B0E64u));
    rect.y = 0x100; rect.x = 0x140; rect.w = 0x140; rect.h = 0xE0;
    func_80074DC0(0);
    func_800755F0(0x800BCE80u + PE_LoadU32(0x8009CDDCu) * 20u);
    func_80074D28(1);

    while (!s6) {
        func_800752AC(Packets(), 0x1000);
        switch (s2) {
        case 0:
            if (func_8007F72C() == 1 && func_8007F778() == 0) {
                s7 = PE_Tray_Stop();
                s2 = 1;
            }
            break;
        case 1:
            v1 = func_8007F418(s7, loc + LOC_RESP);
            if (v1 == 2) {
                func_800375E0(a0 == 1 ? 1 : 2, 0, loc + LOC_LIST);
                s2 = 2;
            } else if (v1 >= 5 && v1 < 7) {
                s2 = 0;
            }
            break;
        case 2:
            if (func_8007F788() & 0x10u) s2 = 3;
            break;
        case 3:
            v1 = func_8007F72C();
            if (v1 == 1) {
                func_800374E8();
                func_800375E0(a0 == 1 ? 3 : 4, 0, loc + LOC_LIST);
                s1 = 0xB4;
                s2 = 5;
            } else if (v1 == 3) {
                s1 = 0xB4;
                s2 = 6;
            }
            break;
        case 5:
            if (s1 != 0) s1--;
            else s2 = 4;
            break;
        case 4:
            v1 = func_800698D4();
            if (v1 == -1 || v1 == -2) {
                func_800374E8();
                s1 = 0xB4;
                func_800375E0(5, 0, loc + LOC_LIST);
                s2 = v1 == -1 ? 7 : 8;
            } else if (v1 == 0) {
                uint8_t mounted = PE_LoadU8(0x800B0DCDu);
                if ((a0 == 1 && (mounted & 1u)) || (a0 == 2 && (mounted & 2u))) {
                    s6 = 1;
                } else {
                    s1 = 0xB4;
                    s2 = 9;
                }
            }
            break;
        case 6:
        case 7:
        case 8:
        case 9:
            if (s1 != 0) {
                s1--;
            } else {
                func_800374E8();
                func_800375E0(a0 == 1 ? 1 : 2, 0, loc + LOC_LIST);
                s2 = 2;
            }
            break;
        default:
            break;
        }
        if (PE_Port_ShouldStop()) goto leave;

        func_80037870();
        func_80074DC0(0);
        func_80073A44(0);
        func_80074A44(1);
        bank = PE_LoadU32(0x8009CDDCu);
        func_800755F0(0x800BCE80u + bank * 20u);
        func_80075424(0x800BCDC8u + bank * 92u);
        func_8007512C(&rect, 0, bank != 0 ? 0xE0 : 0);
        func_800753B4(Packets() + 0x3FFCu);
        PE_StoreU32(0x8009CDDCu, bank ^ 1u);
        (void)PE_Tray_Frame(s2 == 2);
        if (PE_Port_ShouldStop()) goto leave;
    }

    func_80073A44(0);
    func_80074D28(0);
    rect.w = 0x140; rect.x = 0; rect.y = 0; rect.h = 0x1C0;
    func_80074F44(&rect, 0, 0, 1);
    func_80074DC0(0);
    PE_StoreU8(0x800B0DCDu, a0 == 1 ? 1u : 2u);
    while (func_8007F72C() != 1) {
        func_80073A44(0);
        if (PE_Port_ShouldStop()) goto leave;
    }
    PE_StoreU16(0x800B0DD4u, (uint16_t)func_8007F7A8());
leave:
    PE_GuestFrame_Leave(loc, LOC_SIZE);
}
