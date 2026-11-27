/* No-op GTE provider for the gte60 checker controls: every function does
 * nothing and reads return 0, so every checker must FAIL against it.
 *   cc -std=gnu99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/game -Ipc_port/tools pc_port/tools/pe_gte60_X_check.c pc_port/tools/pe_gte60_noop_stub.c -o check_noop
 * (X = a, b or c; batch a also needs pe_plat/gte.h's binding, defined here.) */
#include "pe_sdk.h"
PeGteState g_pe_gte;
uint32_t pe_plat_gte_translate_matrix(uint32_t m, uint32_t v) { (void)v; return m; }
uint32_t PE_GTE_LZCR(uint32_t v) { (void)v; return 0; }
void PE_GTE_SetLZCS(uint32_t v) { (void)v; }
void PE_GTE_LoadRT(pe_addr_t m) { (void)m; }
void PE_GTE_LoadRT33(pe_addr_t m) { (void)m; }
void PE_GTE_LoadLCM(pe_addr_t m) { (void)m; }
void PE_GTE_LoadLLM_halfs(const int16_t *h) { (void)h; }
void PE_GTE_SetIR(int16_t a, int16_t b, int16_t c) { (void)a; (void)b; (void)c; }
void PE_GTE_SetV0(int16_t a, int16_t b, int16_t c) { (void)a; (void)b; (void)c; }
void PE_GTE_SetV1(int16_t a, int16_t b, int16_t c) { (void)a; (void)b; (void)c; }
void PE_GTE_SetV2(int16_t a, int16_t b, int16_t c) { (void)a; (void)b; (void)c; }
void PE_GTE_SetRGBC(uint32_t v) { (void)v; }
void PE_GTE_SetBK(int32_t a, int32_t b, int32_t c) { (void)a; (void)b; (void)c; }
void PE_GTE_MVMVA(uint32_t c) { (void)c; }
void PE_GTE_GPF(int sf, int lm) { (void)sf; (void)lm; }
void PE_GTE_GPL(int sf, int lm) { (void)sf; (void)lm; }
void PE_GTE_OP(int sf, int lm) { (void)sf; (void)lm; }
void PE_GTE_NCCT(void) {}
void PE_GTE_RTPS_coordinates(uint32_t *xy, uint32_t *z) { *xy = 0; *z = 0; }
void PE_GTE_RTPT_coordinates(uint32_t xy[3], uint32_t z[3]) { int i; for (i = 0; i < 3; i++) xy[i] = z[i] = 0; }
void PE_GTE_AVSZ3(const uint32_t z[3]) { (void)z; }
void PE_GTE_AVSZ4(const uint32_t z[4]) { (void)z; }
int32_t PE_GTE_NCLIP(void) { return 0; }
uint32_t PE_GTE_ReadData(uint32_t r) { (void)r; return 0; }
void PE_GTE_WriteData(uint32_t r, uint32_t v) { (void)r; (void)v; }
uint32_t PE_GTE_ReadCtrl(uint32_t r) { (void)r; return 0; }
void PE_GTE_WriteCtrl(uint32_t r, uint32_t v) { (void)r; (void)v; }
void PE_GTE_Command(uint32_t c) { (void)c; }
