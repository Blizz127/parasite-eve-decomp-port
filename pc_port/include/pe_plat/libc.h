/* Independent host implementations of the byte-copy and one-time startup
 * operations. Adapted from the retained WP2a provider a44107c6: its author
 * used public ISO memmove semantics and black-box retail test vectors; its
 * return-value, guest-address ordering and signed count are retail-specific.
 * The startup guard models this image's empty constructor list. This is a
 * provenance statement, not a license determination. */
#ifndef PE_PLAT_LIBC_H
#define PE_PLAT_LIBC_H
#include <stdint.h>
#ifndef PE_PLAT_GUEST_ADDR_DEFINED
#define PE_PLAT_GUEST_ADDR_DEFINED
typedef uint32_t pe_plat_guest_addr;
#endif
pe_plat_guest_addr pe_plat_libc_move_bytes(pe_plat_guest_addr dst, pe_plat_guest_addr src, uint32_t n);
void pe_plat_libsn_main(void);
void pe_plat_snd_init(void);
void pe_plat_swap_bytes(pe_plat_guest_addr a, pe_plat_guest_addr b, uint32_t n);
void pe_plat_fill_bytes(pe_plat_guest_addr dst, uint8_t value, int32_t count);
void pe_plat_clear_words(pe_plat_guest_addr dst, int32_t count);
/* _err_math (pe_plat_math_err.c, clean-room from black-box vectors:
 * pc_port/tools/pe_cd_leaf_vectors.py errmath). */
int pe_plat_math_err(int code, int arg);
/* libc qsort (pe_plat_qsort.c, clean-room from black-box vectors:
 * pc_port/tools/pe_cd_leaf_vectors.py qsort). */
void pe_plat_qsort(pe_plat_guest_addr base, uint32_t count, uint32_t width, pe_plat_guest_addr compare);
#define func_80072334 pe_plat_libc_move_bytes
#define func_800725DC pe_plat_libsn_main
#define func_80036DC8 pe_plat_snd_init
#define func_800724F4 pe_plat_swap_bytes
#define func_80077A28 pe_plat_fill_bytes
#define func_80074330 pe_plat_clear_words
#define func_800739C4 pe_plat_math_err
#define func_800723A4 pe_plat_qsort
#define func_800744A4 pe_plat_clear_words   /* same word clear on every vector */
#define func_8007474C pe_plat_clear_words   /* same word clear on every vector */
/* Canonical guest prototypes (read by the port generators; they expand to
 * the pe_plat declarations above). */
uint32_t func_80072334(uint32_t dst, uint32_t src, uint32_t n);
void func_800725DC(void);
void func_80036DC8(void);
void func_800724F4(uint32_t a, uint32_t b, uint32_t n);
void func_80077A28(uint32_t dst, uint8_t value, int32_t count);
void func_80074330(uint32_t dst, int32_t count);
void func_800744A4(uint32_t dst, int32_t count);
void func_8007474C(uint32_t dst, int32_t count);
int func_800739C4(int code, int arg);
void func_800723A4(uint32_t base, uint32_t count, uint32_t width, uint32_t compare);
#endif
