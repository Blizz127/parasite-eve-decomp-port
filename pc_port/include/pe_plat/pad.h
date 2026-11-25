/* libpad PadGetState, PadInfoMode, PadSetAct, PadSetActAlign, PadSetMainMode
 * PadInfoComb and twenty internals (record lookup/busy, command setters,
 * state steps, act-info receive/load, ALIGN handler)
 * -> clean-room host models (platform/pe_plat_pad*.c).  pe_plat_pad_record(port) is the host's controller
 * record lookup (game/boot/pe_controller_sdk_port.c); 0 means the host
 * stopped. */
#ifndef PE_PLAT_PAD_H
#define PE_PLAT_PAD_H
#include <stdint.h>
void pe_plat_pad_initialize_host(void);
#define func_80082534 pe_plat_pad_initialize_host
void pe_plat_pad_records_initialize_host(uint32_t first,uint32_t second);
#define func_800844E4 pe_plat_pad_records_initialize_host
int pe_plat_pad_get_state(uint32_t port);
uint32_t pe_plat_pad_info_mode(uint32_t port, int32_t term, int32_t offs);
void pe_plat_pad_set_act(uint32_t port, uint32_t data, uint32_t count);
int pe_plat_pad_set_act_align(uint32_t port, uint32_t data);
int pe_plat_pad_set_main_mode(uint32_t port, uint32_t mode, uint32_t lock);
uint32_t pe_plat_pad_record(uint32_t port);
int pe_plat_pad_busy(uint32_t record);
/* Small libpad internals (platform/pe_plat_pad_leaf.c). */
uint32_t pe_plat_pad_port_record(int port);
int pe_plat_pad_record_busy(uint32_t record);
int pe_plat_pad_record_port(uint32_t record);
void pe_plat_pad_cmd_shift(uint32_t record);
void pe_plat_pad_cmd_e(uint32_t record);
void pe_plat_pad_cmd_k(uint32_t record);
void pe_plat_pad_cmd_m(uint32_t record);
void pe_plat_pad_cmd_c(uint32_t record, unsigned char arg);
/* libpad text_34 0x80084B78 (pe_plat_pad_gen18.c, clean-room from black-box
 * vectors: pe_cd_leaf_vectors.py cdgen18): card-object state dispatch. */
int pe_plat_pad_card_dispatch(uint32_t obj);
/* libpad padif transfer 0x80084168 (pe_plat_pad_gen23.c, independent
 * re-implementation from black-box vectors: pe_cd_leaf_vectors.py cdgen23). */
int pe_plat_pad_transfer(uint32_t obj);
/* libpad text_770 0x80083014, the card transfer step (pe_plat_pad_gen24.c,
 * independent re-implementation from black-box vectors: cdgen24). */
void pe_plat_pad_transfer_step(uint32_t obj);
/* libpad text_108 0x80084C4C, the pad-record step installed in slot
 * 0x8009B744 (pe_plat_pad_gen26.c, independent re-implementation from
 * black-box vectors: cdgen26). */
void pe_plat_pad_record_step(uint32_t obj);
void pe_plat_pad_cmd_l(uint32_t record, unsigned char arg);
void pe_plat_pad_cmd_f(uint32_t record, unsigned char arg);
void pe_plat_pad_cmd_g(uint32_t record, unsigned char arg);
/* libpad record state helpers (platform/pe_plat_pad_state.c). */
void pe_plat_pad_send_step(uint32_t record);
void pe_plat_pad_query_step(uint32_t record);
int pe_plat_pad_act_size(uint32_t record);
void pe_plat_pad_main_handler(uint32_t record);
void pe_plat_pad_record_init(uint32_t record);
void pe_plat_pad_install_hooks(void);
/* libpad act-info and combined info (platform/pe_plat_pad_actinfo.c). */
int pe_plat_pad_recv_step(uint32_t record);
int pe_plat_pad_align_handler(uint32_t record);
int pe_plat_pad_info_comb(int port, int term, int offs);
int pe_plat_pad_load_act_info(uint32_t record, int arg);
/* libpad RCNT2 wait pair (pe_plat_pad_rc2.c, clean-room from black-box
 * vectors: pc_port/tools/pe_cd_leaf_vectors.py entries 0x80084FC4/0x80084FE4). */
void pe_plat_pad_rc2_wait_set(uint32_t limit);
int pe_plat_pad_rc2_wait_check(void);
/* libpad communication start 0x80082CF0 (pe_plat_pad_gen34.c, clean-room from
 * black-box vectors: pe_cd_leaf_vectors.py cdgen34). */
int pe_plat_pad_start_com(void);
#define func_80082CF0 pe_plat_pad_start_com
/* libpad/libcard SIO0 start 0x80082E00 (pe_plat_pad_gen35.c, clean-room from
 * black-box vectors: pe_cd_leaf_vectors.py cdgen35). */
int pe_plat_pad_sio_start(uint32_t record);
#define func_80082E00 pe_plat_pad_sio_start
#define func_800825C0 pe_plat_pad_get_state
#define func_80082680 pe_plat_pad_info_mode
#define func_80082974 pe_plat_pad_set_act
#define func_800828F4 pe_plat_pad_set_act_align
#define func_80084FC4 pe_plat_pad_rc2_wait_set
#define func_80084FE4 pe_plat_pad_rc2_wait_check
#define func_80084168 pe_plat_pad_transfer
#define func_80083014 pe_plat_pad_transfer_step
#define func_80084C4C pe_plat_pad_record_step
#define func_8008292C pe_plat_pad_set_main_mode
#define func_80084B20 pe_plat_pad_port_record
#define func_80084F8C pe_plat_pad_record_busy
#define func_80084AE8 pe_plat_pad_record_port
#define func_800847A0 pe_plat_pad_cmd_shift
#define func_80083E70 pe_plat_pad_cmd_e
#define func_80083EE4 pe_plat_pad_cmd_k
#define func_80083C20 pe_plat_pad_cmd_m
#define func_80083E50 pe_plat_pad_cmd_c
#define func_80084B78 pe_plat_pad_card_dispatch
#define func_80083E84 pe_plat_pad_cmd_l
#define func_80083EA4 pe_plat_pad_cmd_f
#define func_80083EC4 pe_plat_pad_cmd_g
#define func_800835C0 pe_plat_pad_send_step
#define func_8008389C pe_plat_pad_query_step
#define func_80083790 pe_plat_pad_act_size
#define func_80083D9C pe_plat_pad_main_handler
#define func_80084644 pe_plat_pad_record_init
#define func_80084B44 pe_plat_pad_install_hooks
#define func_80083644 pe_plat_pad_recv_step
#define func_80083C3C pe_plat_pad_align_handler
#define func_8008284C pe_plat_pad_info_comb
#define func_800837C8 pe_plat_pad_load_act_info
/* Canonical guest prototypes (read by the port generators; they expand to
 * the pe_plat declarations above). */
int func_800825C0(uint32_t port);
uint32_t func_80082680(uint32_t port, int32_t term, int32_t offs);
void func_80082974(uint32_t port, uint32_t data, uint32_t count);
int func_800828F4(uint32_t port, uint32_t data);
int func_8008292C(uint32_t port, uint32_t mode, uint32_t lock);
uint32_t func_80084B20(int port);
int func_80084F8C(uint32_t record);
int func_80084AE8(uint32_t record);
void func_800847A0(uint32_t record);
void func_80083E70(uint32_t record);
void func_80083EE4(uint32_t record);
void func_80083C20(uint32_t record);
void func_80083E50(uint32_t record, unsigned char arg);
void func_80083E84(uint32_t record, unsigned char arg);
void func_80083EA4(uint32_t record, unsigned char arg);
void func_80083EC4(uint32_t record, unsigned char arg);
void func_800835C0(uint32_t record);
void func_8008389C(uint32_t record);
int func_80083790(uint32_t record);
void func_80083D9C(uint32_t record);
void func_80084644(uint32_t record);
void func_80084B44(void);
int func_80083644(uint32_t record);
int func_80083C3C(uint32_t record);
int func_8008284C(int port, int term, int offs);
int func_800837C8(uint32_t record, int arg);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_80084FC4(uint32_t limit);
int func_80084FE4(void);
int func_80084B78(uint32_t obj);
int func_80084168(uint32_t obj);
void func_80083014(uint32_t obj);
void func_80084C4C(uint32_t obj);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_80082534(void);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_800844E4(uint32_t first,uint32_t second);
/* Canonical guest-name prototype for the PadStartCom binding. */
int func_80082CF0(void);
/* Canonical guest-name prototype for the binding above. */
int func_80082E00(uint32_t record);
#endif
