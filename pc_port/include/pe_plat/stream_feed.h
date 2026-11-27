/* CD stream sector feeder: the host-side queue that delivers streamed sectors.
 * Declaration-only header (no function bodies): the port's host model API for the CD stream sector feeder.
 * Declarations were moved unchanged from the port's own host-model header platform/pe_str_feed.h
 * (commit e15e3ee7c); comments describe hardware behaviour as documented publicly (psx-spx)
 * in the project's own words.  No SDK source, tables or disassembly are reproduced here. */
#ifndef PE_PLAT_STREAM_FEED_H
#define PE_PLAT_STREAM_FEED_H

/*
 * HOST_ADAPTED STR stream feeder (fmv lane).
 *
 * When the modeled CD register device (pe_cdreg.c) is disabled, its
 * IRQ-driven sector assembly cannot fill the CD stream ring that the guest's
 * stream reader polls. When a stream is opened with the device disabled, this
 * feeder takes over the *producer* half of the stream ring: it reads raw
 * sectors from the active disc at the 2x drive rate (150 sectors/s = 2.5 per
 * VBlank), assembles STR video chunks into the ring's record and data areas
 * where the guest's stream reader expects them, and hands XA-ADPCM audio
 * sectors to the audio lane's SPU CD input (PE_Spu_CdSector).  Everything
 * downstream (the guest movie players, the variable-length decoder, MDEC,
 * display) is unchanged.
 *
 * Time: the feeder advances emulated VBlanks (HostFB_VSync(0), plus one
 * HostFB_Present refresh when the guest has not presented since the
 * previous VBlank) only while the guest is waiting on the stream, which
 * is when the guest CPU is waiting on the drive.
 */

#include <stdint.h>

#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 1 when the sector at the BCD minute/second/frame location is an STR video or XA audio sector
 * on the active disc (the feeder only takes over real movie streams). */
int  PE_StrFeed_Probe(pe_addr_t bcd_location);
/* Forget all feeder state (host run-control reset). */
void PE_StrFeed_Reset(void);
/* Called when the guest opens a stream while the device is disabled. */
void PE_StrFeed_Open(pe_addr_t bcd_location, uint32_t mode);
/* Called on stream teardown and on the CD pause/stop paths. */
void PE_StrFeed_Close(void);
int  PE_StrFeed_Active(void);
/* Called at the top of each stream poll: make the next frame available in
 * the ring (reading sectors / advancing VBlanks as the drive would). */
void PE_StrFeed_Pump(void);

typedef struct {
    uint32_t start_lba, lba;
    uint32_t sectors, video_sectors, audio_sectors, other_sectors;
    uint32_t frames_published, frames_dropped, vblanks;
    uint32_t last_frame;
} PeStrFeedStats;
void PE_StrFeed_GetStats(PeStrFeedStats *out);

#ifdef __cplusplus
}
#endif

#endif
