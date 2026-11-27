/* Host seams of the CD stream family (cdx-stream), platform/pe_plat2_cdx_stream_seams.c.
 *
 * Host contracts written clean-room from the port's consumers and tests; no original or translated code
 * read.  The five seams:
 *  - PE_Cd_AttachDevice / PE_Cd_DetachDevice (declared in pe_sdk.h with PeCdAttach): mid-game register-level
 *    attachment of the CD controller model for a consumer that needs the device branch (the first mount,
 *    the credits XA stream);
 *  - PE_Cd_DispatchDataCallback (pe_sdk.h): the address-keyed dispatcher the CD poll/completion hosts use
 *    for the data-ready / status / command-complete callback words;
 *  - pe_plat_cd_stream_auxiliary: the D_800B0CCC stream hook of the stream-assembly handler;
 *  - pe_plat_cd_stream_stop: the named unresolved-boundary stop of the stream handler and the DMA3 issuer.
 * The last two have no declaration in a shared header elsewhere (their callers declare them locally with the
 * same prototypes); this header is the canonical one. */
#ifndef PE_PLAT_CDX_STREAM_SEAMS_H
#define PE_PLAT_CDX_STREAM_SEAMS_H
#include <stdint.h>

/* Call the stream hook word D_800B0CCC (0x800B0CCC) with $a0 = `record`.  Returns 1 when there was nothing to
 * call or the hook ran (the caller checks PE_Port_ShouldStop afterwards, as after any call), 0 when the target
 * was refused (a named CPU_BOUNDARY/REFUSED stop, label "func_8007C564_auxiliary_callback", has been raised). */
int pe_plat_cd_stream_auxiliary(uint32_t record);

/* Raise the named unresolved-boundary stop `name` (strict-stubs mode aborts, as for every named stub). */
void pe_plat_cd_stream_stop(const char *name);
#endif
