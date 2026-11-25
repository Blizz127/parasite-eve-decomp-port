/* Host memory-card file operations. These bind BIOS guest entry addresses to
 * the existing clean-room pe_memcard-backed wrappers; behavior is unchanged.
 * Event delivery and the separate first-file model are outside this slice. */
#ifndef PE_PLAT_BIOS_FILE_H
#define PE_PLAT_BIOS_FILE_H
#include <stdint.h>
#ifndef PE_PLAT_GUEST_ADDR_DEFINED
#define PE_PLAT_GUEST_ADDR_DEFINED
typedef uint32_t pe_plat_guest_addr;
#endif
int pe_plat_bios_file_open(pe_plat_guest_addr name, uint32_t mode);
int pe_plat_bios_file_lseek(int fd, int32_t offset, int whence);
int pe_plat_bios_file_read(int fd, pe_plat_guest_addr dst, int32_t length);
int pe_plat_bios_file_write(int fd, pe_plat_guest_addr src, int32_t length);
int pe_plat_bios_file_close(int fd);
int pe_plat_bios_file_format(pe_plat_guest_addr device);
pe_plat_guest_addr pe_plat_bios_file_nextfile(pe_plat_guest_addr dir);
int pe_plat_bios_file_delete(pe_plat_guest_addr name);
/* libcard _card_clear (pe_plat_card_clear.c, clean-room from black-box
 * vectors: pc_port/tools/pe_cd_leaf_vectors.py cardclear). */
int pe_plat_card_clear(uint32_t chan);
/* libcard leaves 0x80082ADC / 0x80082CDC (pe_plat_card_leaf.c, clean-room
 * from black-box vectors: pc_port/tools/pe_cd_leaf_vectors.py ENTRY mode). */
void pe_plat_card_init_timer_callbacks(void);
int pe_plat_card_word_update(void);
/* libcard SIO0 RX-ready poll 0x80083578 (pe_plat_card_sio.c, clean-room
 * from black-box vectors: pc_port/tools/pe_cd_leaf_vectors.py ENTRY mode). */
void pe_plat_card_sio_rx_wait(void);
/* libcard 0x800840DC (pe_plat_card_gen8.c, clean-room from black-box
 * vectors: pc_port/tools/pe_cd_leaf_vectors.py cdgen8). */
int pe_plat_card_840dc(uint32_t a0);
/* libcard InitCARD / StartCARD 0x8007DDD4 / 0x8007DE40 (pe_plat_card_gen13.c,
 * clean-room from black-box vectors: pe_cd_leaf_vectors.py cdgen13). */
void pe_plat_card_init(int a0);
void pe_plat_card_start(void);
/* libcard / libpad slot callers 0x80082B08 / 0x80083DF0 (pe_plat_card_gen14.c,
 * clean-room from black-box vectors: pe_cd_leaf_vectors.py cdgen14). */
int pe_plat_card_slot_poll(void);
int pe_plat_card_obj_reset(uint32_t obj);
/* libcard _copy_memcard_patch 0x8007E4E0 (pe_plat_card_gen15.c, clean-room from
 * black-box vectors: pe_cd_leaf_vectors.py cdgen15). */
void pe_plat_card_copy_patch(void);
/* libcard _dirFailAuto 0x80084EB0 (pe_plat_card_gen17.c, clean-room from
 * black-box vectors: pe_cd_leaf_vectors.py cdgen17). */
void pe_plat_card_dir_fail_auto(uint32_t obj);
/* libapi firstfile 0x800727B4 and its device hook text_19C 0x80072950
 * (pe_plat_api_gen19.c, clean-room from black-box vectors:
 * pe_cd_leaf_vectors.py cdgen19). */
int pe_plat_api_firstfile(uint32_t name, int dir);
int pe_plat_api_firstfile_hook(uint32_t a0, int a1, int a2);
/* libcard card-object methods 0x80083F04 / 0x80083F44 / 0x8008401C
 * (pe_plat_card_gen20.c, clean-room from black-box vectors:
 * pe_cd_leaf_vectors.py cdgen20). */
int pe_plat_card_obj_sync(uint32_t obj);
int pe_plat_card_obj_command(uint32_t obj);
int pe_plat_card_obj_write(uint32_t obj);
/* libcard SIO wait 0x800834E8 (pe_plat_card_gen21.c, clean-room from
 * black-box vectors: pe_cd_leaf_vectors.py cdgen21). */
int pe_plat_card_sio_ack_wait(void);
/* libcard SIO byte exchange 0x800830DC / byte write 0x800832B4
 * (pe_plat_card_gen22.c, clean-room from black-box vectors:
 * pe_cd_leaf_vectors.py cdgen22). */
int pe_plat_card_sio_exchange(uint32_t obj, int byte);
int pe_plat_card_sio_write_byte(uint32_t obj, int byte);
/* libcard MemCard_TimerCallback 0x80082B70 (pe_plat_card_gen25.c,
 * independent re-implementation from black-box vectors: cdgen25). */
int pe_plat_card_timer_callback(void);
/* One iteration of a guest SIO / interrupt wait: advances the port's
 * deterministic clock (pe_plat_sio_tick.c, port glue). */
void pe_plat_sio_poll_tick(void);
#define func_80072734 pe_plat_bios_file_open
#define func_80072744 pe_plat_bios_file_lseek
#define func_80072754 pe_plat_bios_file_read
#define func_80072764 pe_plat_bios_file_write
#define func_80072774 pe_plat_bios_file_close
#define func_80072784 pe_plat_bios_file_format
#define func_80072794 pe_plat_bios_file_nextfile
#define func_800727A4 pe_plat_bios_file_delete
#define func_8007DD74 pe_plat_card_clear
#define func_80082ADC pe_plat_card_init_timer_callbacks
#define func_80082CDC pe_plat_card_word_update
#define func_80083578 pe_plat_card_sio_rx_wait
#define func_800840DC pe_plat_card_840dc
#define func_8007DDD4 pe_plat_card_init
#define func_8007DE40 pe_plat_card_start
#define func_80082B08 pe_plat_card_slot_poll
#define func_80083DF0 pe_plat_card_obj_reset
#define func_8007E4E0 pe_plat_card_copy_patch
#define func_80084EB0 pe_plat_card_dir_fail_auto
#define func_800727B4 pe_plat_api_firstfile
#define func_80072950 pe_plat_api_firstfile_hook
#define func_80083F04 pe_plat_card_obj_sync
#define func_80083F44 pe_plat_card_obj_command
#define func_8008401C pe_plat_card_obj_write
#define func_800834E8 pe_plat_card_sio_ack_wait
#define func_800830DC pe_plat_card_sio_exchange
#define func_800832B4 pe_plat_card_sio_write_byte
#define func_80082B70 pe_plat_card_timer_callback
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_80072734(uint32_t name, uint32_t mode);
int func_80072744(int fd, int32_t offset, int whence);
int func_80072754(int fd, uint32_t dst, int32_t length);
int func_80072764(int fd, uint32_t src, int32_t length);
int func_80072774(int fd);
int func_80072784(uint32_t device);
uint32_t func_80072794(uint32_t dir);
int func_800727A4(uint32_t name);
int func_8007DD74(uint32_t chan);
void func_80082ADC(void);
int func_80082CDC(void);
void func_80083578(void);
int func_800840DC(uint32_t a0);
void func_8007DDD4(int a0);
void func_8007DE40(void);
int func_80082B08(void);
int func_80083DF0(uint32_t obj);
void func_8007E4E0(void);
void func_80084EB0(uint32_t obj);
int func_800727B4(uint32_t name, int dir);
int func_80072950(uint32_t a0, int a1, int a2);
int func_80083F04(uint32_t obj);
int func_80083F44(uint32_t obj);
int func_8008401C(uint32_t obj);
int func_800834E8(void);
int func_800830DC(uint32_t obj, int byte);
int func_800832B4(uint32_t obj, int byte);
int func_80082B70(void);
#endif
