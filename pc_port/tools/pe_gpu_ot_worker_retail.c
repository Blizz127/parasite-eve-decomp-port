#include <stdint.h>
typedef uint32_t (*Load)(uint32_t);
typedef void (*Store)(uint32_t,uint32_t);
typedef uint32_t (*Call)(uint32_t);
uint32_t draft(uint32_t address,uint32_t count,Load load,Store store,Call call) {
 uint32_t p=load(0x80095870u); store(p,load(p)|0x08000000u);
 p=load(0x8009586cu); store(p,0);
 p=load(0x80095864u); store(p,address+(count<<2)-4u);
 p=load(0x80095868u); store(p,count);
 p=load(0x8009586cu); store(p,0x11000002u);
 call(0x800773d0u);
 for (;;) {
  p=load(0x8009586cu);
  if (!(load(p)&0x01000000u)) return count;
  if (call(0x80077404u)) return 0xffffffffu;
 }
}
