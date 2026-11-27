/*
 * pe_plat CD-ROM location helpers (P-CD lane, tranche 1).
 *
 * PROVENANCE: independent re-implementation from public documentation and
 * black-box retail test vectors.  Inputs used:
 *   - the CD-ROM sector addressing format in the public PS1 hardware
 *     references (psx-spx "CDROM Controller" / "CDROM Disc Images": a
 *     location is minute, second, sector-frame, each BCD, 75 frames per
 *     second, 150-frame (2 s) lead-in before logical sector 0);
 *   - caller side only: the prototypes and call sites in pc_port;
 *   - retail input/output vectors produced by running the retail MIPS
 *     through the test-only oracle (pc_port/tools/pe_movie_stream_helpers_oracle.py + pe_cd_plat_oracle.py
 *     -> tests/retail_*cases.h), without reading the
 *     routine.
 * Translated bodies seen by the implementer: NO.  Incidental exposure: a
 * one-line comment ("return &D_8009AFD0", CdLastPos) and the signature lines
 * of the generated TUs (return type
 * only) -- no function body.  Driver-state byte addresses (accessors below)
 * were located by black-box byte-flip probing of the retail routines.
 * No Psy-Q code.  Not a licence determination.
 *
 * Guest ABI (unchanged for callers; the retail names are bound to these
 * entries by the object-like macros below, so no func_8007A930 /
 * func_8007AA34 symbol is defined by the port):
 *   CdlLOC at guest address `loc`: byte 0 = minute, 1 = second, 2 = frame
 *   (BCD), byte 3 is left untouched.
 */
#ifndef PE_PLAT_CD_H
#define PE_PLAT_CD_H

#include <stdint.h>
#include "pe_plat/cdhost.h"   /* the six CD host entries: 0x8007AAB4 / B010 / B558 / BF44 / C564 / CEAC */


/* Stream setup, including the fifth o32 argument. */
void pe_plat_cd_set_stream(uint32_t mode, int32_t start, int32_t end, uint32_t callback, uint32_t auxiliary);
#define func_8007C304 pe_plat_cd_set_stream

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PE_PLAT_GUEST_ADDR_DEFINED
#define PE_PLAT_GUEST_ADDR_DEFINED
typedef uint32_t pe_plat_guest_addr;
#endif

/* Logical block address -> MSF written to the CdlLOC at `loc` (+150-frame
 * lead-in).  Out-of-range / negative inputs follow C truncating division,
 * exactly as the original does (observed vectors).  Returns `loc`. */
uint32_t pe_plat_cd_int_to_pos(int32_t lba, pe_plat_guest_addr loc);

/* CdlLOC at `loc` (BCD MSF) -> logical block address (minus the lead-in). */
int32_t pe_plat_cd_pos_to_int(pe_plat_guest_addr loc);

/* Driver-state accessors.  The CD driver state lives in guest RAM at fixed
 * addresses that other (still translated) code also reads and writes, so
 * the layout is part of the guest ABI.  Addresses were located by black-box
 * perturbation of the retail routines (flip one byte, observe the return):
 *   status   u8 @ 0x8009AFC4   (CdStatus: last controller status byte)
 *   mode     u8 @ 0x8009AFD4   (CdMode:   last Setmode argument)
 *   lastcom  u8 @ 0x8009AFD5   (CdLastCom: last command issued)
 *   lastpos  CdlLOC @ 0x8009AFD0 (CdLastPos: pointer to the last position)
 * Byte values are returned zero-extended. */
#define PE_PLAT_CD_STATUS_ADDR  0x8009AFC4u
#define PE_PLAT_CD_LASTPOS_ADDR 0x8009AFD0u
#define PE_PLAT_CD_MODE_ADDR    0x8009AFD4u
#define PE_PLAT_CD_LASTCOM_ADDR 0x8009AFD5u

uint8_t  pe_plat_cd_status(void);
uint8_t  pe_plat_cd_mode(void);
uint8_t  pe_plat_cd_last_com(void);
uint32_t pe_plat_cd_last_pos(void);

/* Guest CD debug-level exchange; the shared level remains in guest RAM. */
int32_t pe_plat_cd_set_debug(int32_t level);
int32_t pe_plat_cd_queue_len(void);
uint32_t pe_plat_cd_position_ptr(void);
uint8_t pe_plat_cd_driver_status(void);
int32_t pe_plat_cd_disc_type(void);
int32_t pe_plat_cd_read_enabled(void);
int32_t pe_plat_cd_error_active(void);
int32_t pe_plat_cd_set_ready_mode(int32_t mode);
uint8_t pe_plat_cd_sys_last_command(void);
uint8_t pe_plat_cd_sys_command_mode(void);
uint8_t pe_plat_cd_sys_position_byte4(void);
uint8_t pe_plat_cd_sys_position_byte5(void);
int32_t pe_plat_cd_filename_equal(uint32_t left, uint32_t right);
int32_t pe_plat_cd_ds_mix(uint32_t gains);
int32_t pe_plat_cd_ds_get_sector(uint32_t buffer, int32_t words);
int32_t pe_plat_cd_ds_get_sector2(uint32_t buffer, int32_t words);
int pe_plat_cd_get_sector(unsigned int buffer, unsigned int words);
int pe_plat_cd_get_sector2(int buffer, unsigned int words);
int pe_plat_cd_ready(int mode, uint32_t result);
int pe_plat_cd_poll_sync(uint32_t result);
int pe_plat_cd_poll_ready(uint32_t result);
uint32_t pe_plat_cd_set_sync_callback(uint32_t callback);
uint32_t pe_plat_cd_set_ready_callback(uint32_t callback);
uint32_t pe_plat_cd_set_data_callback(uint32_t callback);
int pe_plat_cd_ds_set_sync_callback(int callback);
int pe_plat_cd_ds_set_ready_callback(int callback);
int pe_plat_cd_ds_set_start_callback(int callback);
/* Generated libcd/libds callers (pe_plat_cd_gen.c, clean-room from
 * black-box vectors: pc_port/tools/pe_cd_leaf_vectors.py cdgen). */
void pe_plat_ds_813e8(void);
void pe_plat_ds_er_clear(void);
void pe_plat_ds_text_ec(unsigned char a0);
void pe_plat_cd_set_mode2_callback(int a0);
void pe_plat_cd_7fb04(void);
void pe_plat_cd_init_intr(void);
int pe_plat_cd_st_get_backloc(int a0);
/* Second batch (pe_plat_cd_gen2.c, clean-room from black-box vectors:
 * pc_port/tools/pe_cd_leaf_vectors.py cdgen2). */
void pe_plat_cd_abort_cmd(void);
int pe_plat_ds_start_ready_system(int a0, int a1);
void pe_plat_ds_end_ready_system(void);
int pe_plat_ds_text_5c4(int a0, pe_plat_guest_addr a1);
/* Third batch (pe_plat_cd_gen3.c / pe_plat_cd_gen4.c, clean-room from
 * black-box vectors: pc_port/tools/pe_cd_leaf_vectors.py cdgen3). */
void pe_plat_ds_flush(void);
void pe_plat_ds_81268(void);
/* Fifth batch (pe_plat_cd_gen5.c, clean-room from black-box vectors:
 * pc_port/tools/pe_cd_leaf_vectors.py cdgen5). */
int pe_plat_cd_try_issue_cmd(uint32_t cmd, uint32_t data);
int pe_plat_cd_7fcbc(int a0, int a1);
pe_plat_guest_addr pe_plat_ds_last_pos(pe_plat_guest_addr dst);
void pe_plat_ds_text_508(unsigned char a0);
void pe_plat_ds_text_490(void);
int pe_plat_ds_ready(int a0);
/* text_130 (pe_plat_cd_gen6.c, clean-room from black-box vectors:
 * pc_port/tools/pe_cd_leaf_vectors.py cdgen6). */
void pe_plat_ds_text_130(unsigned char a0);
/* text_13CC (pe_plat_cd_gen7.c, clean-room from black-box vectors:
 * pc_port/tools/pe_cd_leaf_vectors.py cdgen7). */
void pe_plat_cd_text_13cc(uint32_t status, pe_plat_guest_addr response);
/* libds DS_status 0x800809E0 (pe_plat_ds_gen10.c, clean-room from black-box
 * vectors: pe_cd_leaf_vectors.py cdgen10). */
void pe_plat_ds_status(void);
/* libds 0x80080F98 / 0x8008214C (pe_plat_ds_gen11.c, clean-room from black-box
 * vectors: pe_cd_leaf_vectors.py cdgen11). */
void pe_plat_ds_read_progress(int a0, int a1);
/* libds text_FC 0x80081E70, the DS ready callback (pe_plat_ds_gen28.c,
 * independent re-implementation from black-box vectors: cdgen28). */
void pe_plat_ds_ready_callback(unsigned char intr, uint32_t result);
/* libcd CD_getsector2 0x8007C044: wait for CD data, start the DMA3 sector
 * transfer (pe_plat_cd_gen29.c, independent re-implementation from
 * black-box vectors: cdgen29). Registers go through pe_cdreg. */
int pe_plat_cd_getsector2(uint32_t buffer, uint32_t words);
/* libds DsDataCallback 0x800824F0, DsSystemStatus 0x8007F72C and libcd
 * text_1688 0x8007C13C (pe_plat_cd_gen33.c, independent re-implementations
 * from black-box vectors: cdgen33). */
uint32_t pe_plat_ds_data_callback(uint32_t callback);
int pe_plat_ds_system_status(void);
void pe_plat_cd_intr_poll(void);
/* libds DsGetDiskType 0x80082314 (pe_plat_ds_gen38.c, independent
 * re-implementation from black-box vectors: cdgen38).  Follows the retail
 * command flow, so it needs the CD device model: func_800698D4 attaches it
 * around the first mount (PE_698D4_Mount). */
int pe_plat_ds_get_disk_type(void);
/* libds text_170 0x8007E704, the command-queue flush (pe_plat_ds_gen39.c,
 * independent re-implementation from black-box vectors: cdgen39). */
void pe_plat_ds_text_170(uint32_t status, uint32_t response);
/* libds text_3D0 0x8007E964, the command-complete / error handler
 * (pe_plat_ds_gen39.c, independent re-implementation from black-box vectors:
 * cdgen39). */
void pe_plat_ds_text_3d0(uint32_t status, uint32_t response);
/* libds text_360 0x8007E8F4: pending-command dispatch predicate
 * (pe_plat_ds_gen30.c, independent re-implementation from black-box
 * vectors: cdgen30). */
int pe_plat_ds_text_360(void);
/* libds text_5F4 0x8007EB88: command-queue push (pe_plat_ds_gen31.c,
 * independent re-implementation from black-box vectors: cdgen31). */
void pe_plat_ds_text_5f4(uint32_t sequence, uint32_t status, uint32_t response);
void pe_plat_ds_text_3d8(unsigned char a0, int a1);
/* libds text_904 0x80081D18 (pe_plat_ds_gen16.c, clean-room from black-box
 * vectors: pe_cd_leaf_vectors.py cdgen16). */
int pe_plat_ds_text_904(int a0, int a1, int a2);
/* The retail read poll (800811E4) is the clean-room pe_plat_cdhost_readpoll (pe_plat/cdhost.h; the former names
 * pe_plat_cd_read_poll / pe_plat_cd_read_poll_host are macros onto it). */
void pe_plat_cd_retry_command(void);
/* Initializer retail core and host callback-map adapter (8007F994). */
int pe_plat_cd_initialize_callbacks_retail(void);
int pe_plat_cd_initialize_callbacks(void);
int pe_plat_cd_issue_wrapper(uint32_t command, uint32_t data);
int pe_plat_cd_search_file_host(uint32_t out, const char *path);
uint32_t pe_plat_cd_queue_command_host(uint32_t command,uint32_t param,uint32_t extra,uint32_t callback);
int pe_plat_cd_command_sequence_host(uint32_t mode,uint32_t location,int command,uint32_t extra,uint32_t callback);
int pe_plat_cd_response_query_host(uint32_t sequence,uint32_t response);
int pe_plat_cd_reset_host(void);
int pe_plat_cd_public_init_host(void);
int pe_plat_cd_read_stream_host(uint32_t location, uint32_t mode);
int pe_plat_cd_issue_step_host(int command, uint32_t param, uint32_t response);
int pe_plat_cd_command_wait_host(int command, uint32_t param, uint32_t response);
void pe_plat_cd_store_setloc_hint(uint32_t raw);
/* Independent CD status callback and table-dispatched completion helpers. */
void pe_plat_cd_status_callback(uint32_t status, uint32_t response);
void pe_plat_cd_command_complete(uint32_t status, uint32_t response);
void pe_plat_cd_unexpected_status(uint32_t status, uint32_t response);
void pe_plat_cd_copy4(uint32_t dst, uint32_t src);
void pe_plat_cd_copy8(uint32_t dst, uint32_t src);
unsigned int pe_plat_cd_command_name(unsigned int code);
unsigned int pe_plat_cd_interrupt_name(unsigned int code);
void pe_plat_cd_clear_ring_status(int start, unsigned int count);
void pe_plat_cd_set_handler0(uint32_t callback);
void pe_plat_cd_set_handler1(uint32_t callback);
void pe_plat_cd_set_handler2(uint32_t callback);
void pe_plat_cd_set_handler3(uint32_t callback);
void pe_plat_cd_set_driver_callback(unsigned int callback);
void pe_plat_cd_enable_reads(void);
void pe_plat_cd_set_guarded_state(unsigned int value);
int pe_plat_cd_data_sync(int mode);

#ifdef __cplusplus
}
#endif

/* Retail call-site bindings: the retail routine names resolve to the
 * pe_plat entries above (object-like: declarations, calls and address uses). */
/* libds command-queue leaves and the status-lane getter (pe_plat_ds_leaf.c,
 * clean-room from black-box vectors: pc_port/tools/pe_ds_leaf_vectors.py). */
void pe_plat_ds_cq_clear(uint32_t q);
void pe_plat_ds_cq_pop(void);
uint32_t pe_plat_ds_cq_next_slot(void);
int pe_plat_ds_status_lane(int idx);
/* libcd register and stream-ring leaves (pe_plat_cd_leaf.c, clean-room
 * from black-box vectors: pc_port/tools/pe_cd_leaf_vectors.py). */
void pe_plat_cd_set_vol(uint32_t atv);
int pe_plat_cd_init_vol(void);
void pe_plat_cd_st_free_ring(uint32_t sector);
int pe_plat_cd_st_get_next(uint32_t data, uint32_t header);
/* Host-fed movie path advances the feeder before consuming the native ring. */
int pe_plat_cd_st_get_next_host(uint32_t data, uint32_t header);
#define func_8007C394 pe_plat_cd_st_free_ring
#define func_8007C484 pe_plat_cd_st_get_next_host
void pe_plat_cd_data_ready_callback(void);
#define func_8007C214 pe_plat_cd_data_ready_callback
#define func_8007F994 pe_plat_cd_initialize_callbacks
#define func_8007FCFC pe_plat_cd_issue_wrapper
#define func_80080D5C pe_plat_cd_command_wait_host
#define func_80080DC4 pe_plat_cd_issue_step_host
#define func_80081314 pe_plat_cd_read_stream_host
#define func_8007EC14 pe_plat_cd_public_init_host
#define func_8007ED58 pe_plat_cd_reset_host
#define func_8007F418 pe_plat_cd_response_query_host
#define func_8007F0C8 pe_plat_cd_command_sequence_host
#define func_8007EE84 pe_plat_cd_queue_command_host
#define func_80081414 pe_plat_cd_search_file_host
#define func_800800F4 pe_plat_cd_retry_command
#define func_80080164 pe_plat_cd_status_callback
#define func_80080220 pe_plat_cd_command_complete
#define func_8008068C pe_plat_cd_unexpected_status
void pe_plat_cd_flush(void);
#define func_8007B9EC pe_plat_cd_flush
/* libcd CD_ready 0x8007B290: the BIOS-side CD readiness poll (pe_plat_cd_ready.c,
 * independent re-implementation from black-box vectors: cdgen36).  The wrapper
 * pe_plat_cd_ready above is a different guest function (0x8007A488). */
int pe_plat_cd_ready_poll(uint32_t mode, uint32_t result);
#define func_8007B290 pe_plat_cd_ready_poll
/* libds-block transfer-step gate 0x80081110 (pe_plat_81110.c, independent
 * re-implementation from black-box vectors: cdgen36). */
void pe_plat_ds_81110(void);
int pe_plat_ds_81110_value(void);   /* the same step, returning the value the vectors show in $v0 */
#define func_80081110 pe_plat_ds_81110

#define func_8007E594 pe_plat_ds_cq_clear
#define func_8007E5C4 pe_plat_ds_cq_pop
#define func_8007E6B0 pe_plat_ds_cq_next_slot
#define func_8007FBF0 pe_plat_ds_status_lane
#define func_8007B964 pe_plat_cd_set_vol
#define func_8007BAC0 pe_plat_cd_init_vol
#define func_8007A930 pe_plat_cd_int_to_pos
#define func_8007AA34 pe_plat_cd_pos_to_int
#define func_8007A324 pe_plat_cd_status
#define func_8007A334 pe_plat_cd_mode
#define func_8007A344 pe_plat_cd_last_com
#define func_8007A354 pe_plat_cd_last_pos
#define func_8007A3EC pe_plat_cd_set_debug
#define func_80080CC8 pe_plat_cd_set_debug
#define func_80080B44 pe_plat_cd_int_to_pos
#define func_80080C48 pe_plat_cd_pos_to_int
#define func_8007F778 pe_plat_cd_queue_len
#define func_8007FC28 pe_plat_cd_position_ptr
#define func_8007FC54 pe_plat_cd_driver_status
#define func_8007FCAC pe_plat_cd_disc_type
#define func_8007F7A8 pe_plat_cd_disc_type
#define func_80080940 pe_plat_cd_read_enabled
#define func_8007F788 pe_plat_cd_driver_status
#define func_800822AC pe_plat_cd_error_active
#define func_80081E5C pe_plat_cd_set_ready_mode
#define func_8007FC08 pe_plat_cd_sys_last_command
#define func_8007F7C8 pe_plat_cd_sys_last_command
#define func_8007FC18 pe_plat_cd_sys_command_mode
#define func_8007FC34 pe_plat_cd_sys_position_byte4
#define func_8007FC44 pe_plat_cd_sys_position_byte5
#define func_800816F4 pe_plat_cd_filename_equal
#define func_80080AC4 pe_plat_cd_ds_mix
#define func_80080AE4 pe_plat_cd_ds_get_sector
#define func_80080B04 pe_plat_cd_ds_get_sector2
#define func_8007A88C pe_plat_cd_ds_mix
#define func_8007A8AC pe_plat_cd_get_sector
#define func_8007A8CC pe_plat_cd_get_sector2
#define func_8007A488 pe_plat_cd_ready
#define func_8007FC64 pe_plat_cd_poll_sync
#define func_8007FC88 pe_plat_cd_poll_ready
#define func_8007A4A8 pe_plat_cd_set_sync_callback
#define func_8007A4BC pe_plat_cd_set_ready_callback
#define func_80081254 pe_plat_cd_set_data_callback
#define func_800824B4 pe_plat_cd_ds_set_sync_callback
#define func_800824C8 pe_plat_cd_ds_set_ready_callback
#define func_800824DC pe_plat_cd_ds_set_start_callback
#define func_800813E8 pe_plat_ds_813e8
#define func_800822BC pe_plat_ds_er_clear
#define func_80082400 pe_plat_ds_text_ec
#define func_80080F64 pe_plat_cd_set_mode2_callback
#define func_8007FB04 pe_plat_cd_7fb04
#define func_8007BBB0 pe_plat_cd_init_intr
#define func_8007C2A0 pe_plat_cd_st_get_backloc
#define func_800808BC pe_plat_cd_abort_cmd
#define func_80081D74 pe_plat_ds_start_ready_system
#define func_80081DF8 pe_plat_ds_end_ready_system
#define func_800819D8 pe_plat_ds_text_5c4
#define func_8007F69C pe_plat_ds_flush
#define func_80081268 pe_plat_ds_81268
#define func_8007FB44 pe_plat_cd_try_issue_cmd
#define func_8007FCBC pe_plat_cd_7fcbc
#define func_80080CDC pe_plat_ds_last_pos
#define func_8008227C pe_plat_ds_text_508
#define func_80082204 pe_plat_ds_text_490
#define func_8007F608 pe_plat_ds_ready
#define func_80082444 pe_plat_ds_text_130
#define func_8007F960 pe_plat_cd_text_13cc
#define func_800809E0 pe_plat_ds_status
#define func_80080F98 pe_plat_ds_read_progress
#define func_80081E70 pe_plat_ds_ready_callback
#define func_8007C044 pe_plat_cd_getsector2
#define func_800824F0 pe_plat_ds_data_callback
#define func_8007F72C pe_plat_ds_system_status
#define func_8007C13C pe_plat_cd_intr_poll
#define func_8007E704 pe_plat_ds_text_170
#define func_80082314 pe_plat_ds_get_disk_type
#define func_8007E964 pe_plat_ds_text_3d0
#define func_8007E8F4 pe_plat_ds_text_360
#define func_8007EB88 pe_plat_ds_text_5f4
#define func_8008214C pe_plat_ds_text_3d8
#define func_80081D18 pe_plat_ds_text_904
#define func_80080950 pe_plat_cd_copy4
#define func_80080998 pe_plat_cd_copy8
#define func_8007A400 pe_plat_cd_command_name
#define func_8007A434 pe_plat_cd_interrupt_name
#define func_8007C444 pe_plat_cd_clear_ring_status
#define func_8007FBC0 pe_plat_cd_set_handler0
#define func_8007FBCC pe_plat_cd_set_handler1
#define func_8007FBD8 pe_plat_cd_set_handler2
#define func_8007FBE4 pe_plat_cd_set_handler3
#define func_8007C130 pe_plat_cd_set_driver_callback
#define func_80080930 pe_plat_cd_enable_reads
#define func_800812F4 pe_plat_cd_set_guarded_state
#define func_8007BDDC pe_plat_cd_data_sync
#define func_8007A910 pe_plat_cd_data_sync
#define func_80080B24 pe_plat_cd_data_sync

/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
uint32_t func_8007A930(int32_t lba, uint32_t loc);
int32_t func_8007AA34(uint32_t loc);
uint8_t func_8007A324(void);
uint8_t func_8007A334(void);
uint8_t func_8007A344(void);
uint32_t func_8007A354(void);
int32_t func_8007A3EC(int32_t level);
int32_t func_80080CC8(int32_t level);
uint32_t func_80080B44(int32_t lba, uint32_t loc);
int32_t func_80080C48(uint32_t loc);
int32_t func_8007F778(void);
uint32_t func_8007FC28(void);
uint8_t func_8007FC54(void);
int32_t func_8007FCAC(void);
int32_t func_8007F7A8(void);
int32_t func_80080940(void);
uint8_t func_8007F788(void);
int32_t func_800822AC(void);
int32_t func_80081E5C(int32_t mode);
uint8_t func_8007FC08(void);
uint8_t func_8007F7C8(void);
uint8_t func_8007FC18(void);
uint8_t func_8007FC34(void);
uint8_t func_8007FC44(void);
int32_t func_800816F4(uint32_t left, uint32_t right);
int32_t func_80080AC4(uint32_t gains);
int32_t func_80080AE4(uint32_t buffer, int32_t words);
int32_t func_80080B04(uint32_t buffer, int32_t words);
int32_t func_8007A88C(uint32_t gains);
int func_8007A8AC(unsigned int buffer, unsigned int words);
int func_8007A8CC(int buffer, unsigned int words);
int func_8007A488(int mode, uint32_t result);
int func_8007FC64(uint32_t result);
int func_8007FC88(uint32_t result);
uint32_t func_8007A4A8(uint32_t callback);
uint32_t func_8007A4BC(uint32_t callback);
uint32_t func_80081254(uint32_t callback);
int func_800824B4(int callback);
int func_800824C8(int callback);
int func_800824DC(int callback);
void func_80080950(uint32_t dst, uint32_t src);
void func_80080998(uint32_t dst, uint32_t src);
unsigned int func_8007A400(unsigned int code);
unsigned int func_8007A434(unsigned int code);
void func_8007C444(int start, unsigned int count);
void func_8007FBC0(uint32_t callback);
void func_8007FBCC(uint32_t callback);
void func_8007FBD8(uint32_t callback);
void func_8007FBE4(uint32_t callback);
void func_8007C130(unsigned int callback);
void func_80080930(void);
void func_800812F4(unsigned int value);
int func_8007BDDC(int mode);
int func_8007A910(int mode);
int func_80080B24(int mode);
void func_8007E594(uint32_t q);
void func_8007E5C4(void);
uint32_t func_8007E6B0(void);
int func_8007FBF0(int idx);
void func_8007B964(uint32_t atv);
int func_8007BAC0(void);
void func_800813E8(void);
void func_800822BC(void);
void func_80082400(unsigned char a0);
void func_80080F64(int a0);
void func_8007FB04(void);
void func_8007BBB0(void);
int func_8007C2A0(int a0);
void func_800808BC(void);
int func_80081D74(int a0, int a1);
void func_80081DF8(void);
int func_800819D8(int a0, uint32_t a1);
void func_8007F69C(void);
void func_80081268(void);
int func_8007FB44(uint32_t cmd, uint32_t data);
int func_8007FCBC(int a0, int a1);
uint32_t func_80080CDC(uint32_t dst);
void func_8008227C(unsigned char a0);
void func_80082204(void);
int func_8007F608(int a0);
void func_80082444(unsigned char a0);
void func_8007F960(uint32_t status, uint32_t response);
void func_800809E0(void);
void func_80080F98(int a0, int a1);
void func_80081E70(unsigned char intr, uint32_t result);
void func_8008214C(unsigned char a0, int a1);
int func_80081D18(int a0, int a1, int a2);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_8007C044(uint32_t buffer, uint32_t words);
void func_80081110(void);
int func_8007B290(uint32_t mode, uint32_t result);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_8007E8F4(void);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007EB88(uint32_t sequence, uint32_t status, uint32_t response);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007C304(uint32_t mode, int32_t start, int32_t end, uint32_t callback, uint32_t auxiliary);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007C394(uint32_t sector);
int func_8007C484(uint32_t data, uint32_t header);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007B9EC(void);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007C214(void);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_800811E4(uint32_t fp);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_800800F4(void);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_80080164(uint32_t status, uint32_t response);
void func_80080220(uint32_t status, uint32_t response);
void func_8008068C(uint32_t status, uint32_t response);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_8007F994(void);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_8007FCFC(uint32_t command, uint32_t data);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_80080D5C(int command, uint32_t param, uint32_t response);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_80080DC4(int command, uint32_t param, uint32_t response);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_80081314(uint32_t location, uint32_t mode);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_80081414(uint32_t out, const char *path);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_8007EC14(void);
int func_8007ED58(void);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
uint32_t func_8007EE84(uint32_t command,uint32_t param,uint32_t extra,uint32_t callback);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_8007F0C8(uint32_t mode,uint32_t location,int command,uint32_t extra,uint32_t callback);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_8007F418(uint32_t sequence,uint32_t response);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
uint32_t func_800824F0(uint32_t callback);
int func_8007F72C(void);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007C13C(void);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007E704(uint32_t status, uint32_t response);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007E964(uint32_t status, uint32_t response);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_80082314(void);
#endif
