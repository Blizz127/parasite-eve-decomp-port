/* Host scheduling boundary for the independently validated native ring reader.
 * PE_StrFeed_Pump models drive progress for the host-fed movie path; the pure
 * ring consumer retains its original-code oracle contract unchanged.
 * Evidence: docs/evidence/native-ring-integration-20261103/REPORT.md.
 */
#include <stdint.h>
#include "game_port.h"
#include "pe_plat/cd.h"
#include "pe_str_feed.h"

int pe_plat_cd_st_get_next_host(uint32_t data, uint32_t header)
{
    if (PE_Port_ShouldStop()) return 1;
    PE_StrFeed_Pump();
    if (PE_Port_ShouldStop()) return 1;
    return pe_plat_cd_st_get_next(data, header);
}
