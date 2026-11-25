#include <stdint.h>
typedef void (*Store)(uint32_t,uint32_t);
typedef uint32_t (*Call)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(Store store,Call call)
{
    uint32_t deadline=call(0x80073A44u,0xFFFFFFFFu,0,0,0)+240u;
    store(0x80095888u,deadline);
    store(0x8009588Cu,0);
    return deadline;
}
