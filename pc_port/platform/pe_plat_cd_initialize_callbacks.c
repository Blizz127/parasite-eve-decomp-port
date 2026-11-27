/* Independent guest8007F994 from original-code vectors and frozen host traces.
 * Retains exact host callback-map registration and its explicit failure stop. */
#include "pe_guest_ram.h"
#include "pe_sdk.h"
#include "pe_plat/cd_stream.h"
#include "game_port.h"
#include "pe_callback.h"
static int initialize_callbacks(int bind_host)
{
    (void)func_8007BBFC();
    if (PE_Port_ShouldStop()) return 0;
    (void)pe_plat_cd_init_vol();
    if (PE_Port_ShouldStop()) return 0;
    PE_StoreU32(0x800A36A8u,0u);
    PE_StoreU32(0x800A36A4u,0u);
    PE_StoreU32(0x800A36A0u,0u);
    pe_plat_cdxstream_initialize_state();
    pe_plat_cd_set_guarded_state(0u);
    PE_StoreU32(0x8009AFB4u,0x80080164u);
    PE_StoreU32(0x8009AFB8u,0x80080778u);
    if (bind_host && PE_Callback_Bind(0x8007FE24u,func_8007FE24)!=0) {
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    (void)pe_plat_vsync_callbacks(0u,0x8007FE24u);
    PE_StoreU32(0x8009AFD8u,1u);
    PE_StoreU32(0x8009B554u,1u);
    return 1;
}

/* Retail caller evidence stays separate from host callback-map registration. */
int pe_plat_cd_initialize_callbacks_retail(void){return initialize_callbacks(0);}
int pe_plat_cd_initialize_callbacks(void){return initialize_callbacks(1);}
