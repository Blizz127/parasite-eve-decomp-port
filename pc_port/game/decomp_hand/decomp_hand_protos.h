/*
 * Aggregator for hand-adapter prototypes in pc_port/game/decomp_hand/.
 * Included by pe_guest_decomp.h so generated TUs (and their compile gate in
 * tools/analysis/gen_decomp_ports.py) see the canonical host signatures.
 * Each porting lane owns its own prototype header; add it here once.
 */
#ifndef PE_DECOMP_HAND_PROTOS_H
#define PE_DECOMP_HAND_PROTOS_H

#include "pe_bios_string.h"          /* BIOS A-function string/memory */
#include "hand_hi_protos.h"          /* VRAM >= 0x80060000 (port_stubs)  */
#if defined(__has_include)
#if __has_include("hand_lo_protos.h")
#include "hand_lo_protos.h"          /* VRAM <  0x80060000 (port_absent) */
#endif
/* port_absent lane, 2026-09-23: the last `absent` leaves, one header per
 * worker so parallel lanes never edit the same file. */
#if __has_include("hand_absent_sdk_protos.h")
#include "hand_absent_sdk_protos.h"   /* 0x80038954..0x80077FFF + 80090AEC/B5C */
#endif
#if __has_include("hand_absent_cd_protos.h")
#include "hand_absent_cd_protos.h"    /* 0x8007D000..0x80084FFF */
#endif
#if __has_include("hand_absent_lo2_protos.h")
#include "hand_absent_lo2_protos.h"   /* 0x80038000..0x8005FFFF, round 2 */
#endif
#if __has_include("hand_absent_hi2_protos.h")
#include "hand_absent_hi2_protos.h"   /* 0x80087000..0x8008FFFF, round 2 */
#endif
#if __has_include("hand_absent_ovl2_protos.h")
#include "hand_absent_ovl2_protos.h"   /* overlay 0x800D0000.., round 2 */
#endif
#if __has_include("hand_absent_sio_protos.h")
#include "hand_absent_sio_protos.h"  /* SIO0 exchange (sio_port.c) */
#endif
#if __has_include("hand_absent_r4_protos.h")
#include "hand_absent_r4_protos.h"   /* round 4 (2026-09-24) */
#endif
#if __has_include("hand_absent_r3_protos.h")
#include "hand_absent_r3_protos.h"   /* round 3 (2026-09-24) */
#endif
#if __has_include("hand_absent_e9_protos.h")
#include "hand_absent_e9_protos.h"    /* E9/E9b guest-pointer leaves */
#endif
#if __has_include("hand_absent_ovl_protos.h")
#include "hand_absent_ovl_protos.h"   /* overlay-resident 0x800C0000.. */
#endif
#if __has_include("hand_port6_protos.h")
#include "hand_port6_protos.h"        /* port lane round 6 (2026-09-27) */
#endif
#if __has_include("hand_port7_protos.h")
#include "hand_port7_protos.h"        /* port lane round 7 (2026-09-27) */
#endif
#if __has_include("hand_akao_protos.h")
#include "hand_akao_protos.h"         /* AKAO driver tick (2026-09-28) */
#endif
#if __has_include("hand_akao_seq_protos.h")
#include "hand_akao_seq_protos.h"     /* AKAO seq start / bank restore (2026-10-07) */
#endif
#endif

#endif /* PE_DECOMP_HAND_PROTOS_H */
