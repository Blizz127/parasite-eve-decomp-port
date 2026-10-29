/* pe_drive_tray.c — see pe_drive_tray.h. */
#include "pe_drive_tray.h"
#include "pe_guest_ram.h"
#include "pe_sdk.h"
#include "pe_cdreg.h"
#include <stdio.h>

static PE_Disc *g_spare;
static int g_spare_number;
static int g_active_number = 1;
static int g_wanted;
static unsigned g_phase;      /* 0 closed, 1 open */
static unsigned g_frames;
static unsigned g_swaps;

void PE_Tray_Reset(void)
{
    g_spare = NULL; g_spare_number = 0; g_active_number = 1;
    g_wanted = 0; g_phase = 0; g_frames = 0; g_swaps = 0;
}

void PE_Tray_SetSpare(PE_Disc *disc, int number)
{
    g_spare = disc;
    g_spare_number = disc ? number : 0;
}

PE_Disc *PE_Tray_Spare(void) { return g_spare; }
int PE_Tray_SpareNumber(void) { return g_spare_number; }
void PE_Tray_SetActiveNumber(int number) { g_active_number = number; }
int PE_Tray_ActiveNumber(void) { return g_active_number; }
unsigned PE_Tray_Swaps(void) { return g_swaps; }

int PE_Tray_CanServe(int wanted)
{
    return g_spare != NULL && g_spare_number == wanted;
}

void PE_Tray_Begin(int wanted)
{
    g_wanted = wanted; g_phase = 0; g_frames = 0;
}

int PE_Tray_Frame(int waiting_for_lid)
{
    if (!g_spare || g_spare_number != g_wanted) return 0;
    if (g_phase == 0) {
        if (!waiting_for_lid) { g_frames = 0; return 0; }
        if (++g_frames < PE_TRAY_OPEN_DELAY) return 0;
        /* Lid opens: the SDK's shell-open reaction (func_8008068C). */
        PE_StoreU8(0x8009B56Cu, (uint8_t)(PE_LoadU8(0x8009B56Cu) | 0x10u));
        PE_StoreU32(0x8009B574u, 2u);
        g_phase = 1; g_frames = 0;
        fprintf(stderr, "[TRAY] lid open (disc %d out)\n", g_active_number);
        return 0;
    }
    if (++g_frames < PE_TRAY_CLOSE_DELAY) return 0;
    {
        PE_Disc *out = PE_Disc_GetActive();
        int out_number = g_active_number;
        PE_Disc_SetActive(g_spare);
        g_active_number = g_spare_number;
        g_spare = out; g_spare_number = out ? out_number : 0;
    }
    PE_StoreU8(0x8009B56Cu, (uint8_t)(PE_LoadU8(0x8009B56Cu) & ~0x10u));
    PE_StoreU32(0x8009B574u, 1u);
    g_phase = 0; g_frames = 0; g_swaps++;
    fprintf(stderr, "[TRAY] lid closed on disc %d\n", g_active_number);
    return 1;
}

int PE_Tray_InsertNow(int wanted)
{
    PE_Disc *out;
    int out_number;
    if (!PE_Tray_CanServe(wanted)) return 0;
    out = PE_Disc_GetActive();
    out_number = g_active_number;
    PE_Disc_SetActive(g_spare);
    g_active_number = g_spare_number;
    g_spare = out; g_spare_number = out ? out_number : 0;
    g_phase = 0; g_frames = 0; g_swaps++;
    fprintf(stderr, "[TRAY] disc %d inserted (disc %d out)\n", g_active_number, g_spare_number);
    return 1;
}

uint32_t PE_Tray_HostStop(void)
{
    uint32_t sequence = PE_LoadU32(0x8009B53Cu) + 1u;
    pe_addr_t response;
    PE_StoreU32(0x8009B53Cu, sequence);
    if (!sequence) PE_StoreU32(0x8009B53Cu, ++sequence);
    response = PE_GuestFrame_Enter(8u);
    PE_Fill(response, 8u, 0u);              /* status byte: motor off */
    func_8007EB88(sequence, 2u, response);  /* CdlComplete */
    PE_GuestFrame_Leave(response, 8u);
    return sequence;
}

uint32_t PE_Tray_Stop(void)
{
    if (PE_CdReg_DeviceEnabled())
        return func_8007EE84(8u, 0u, 0u, 0xFFFFFFFFu);
    return PE_Tray_HostStop();
}
