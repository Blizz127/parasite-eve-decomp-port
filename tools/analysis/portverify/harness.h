#include <stdint.h>
typedef struct { const char *name; int64_t (*gen)(const uint32_t *); int64_t (*hand)(const uint32_t *); const char *kinds; int rk; } PvEntry;
typedef uint32_t pe_addr_t;
extern const PvEntry pv_table[];
typedef signed char s8; typedef unsigned char u8; typedef short s16; typedef unsigned short u16; typedef int s32; typedef unsigned int u32;
