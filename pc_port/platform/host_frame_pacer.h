#ifndef HOST_FRAME_PACER_H
#define HOST_FRAME_PACER_H
#include <stdint.h>

/* Host presentation clock: 60000/1001 Hz. The fraction prevents accumulated
 * rounding error; guest VBlank queries and deterministic tests never sleep. */
typedef struct {
    uint64_t deadline_ns;
    unsigned fraction;
    int started;
} HostFramePacer;

static inline uint64_t HostFramePacer_Deadline(HostFramePacer *p, uint64_t now)
{
    if (!p->started) {
        p->deadline_ns=now;
        p->fraction=0u;
        p->started=1;
    }
    p->deadline_ns+=UINT64_C(16683333);
    p->fraction+=20000u;
    if (p->fraction>=60000u) {p->deadline_ns++;p->fraction-=60000u;}
    /* Preserve ordinary scheduling jitter, but discard the backlog after
     * a slow frame, debugger pause or suspended window. */
    if (now>p->deadline_ns && now-p->deadline_ns>=UINT64_C(16683334)) {
        p->deadline_ns=now;
        p->fraction=0u;
    }
    return p->deadline_ns;
}
#endif
