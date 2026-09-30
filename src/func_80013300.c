/* VRAM 0x80013300 / size 0xE8. Task flag 0x20 command-wait.
 * First visit arms the command via func_8001A680 and yields 0;
 * later visits clear the flag once actor+0x0F is 0 or the
 * +0x14/+0x18 counters say the clip is ready. era -O2 -G8, D_8009D2F0 forced absolute. */

typedef struct Task {
    unsigned int entry;
    unsigned int field_04;
    unsigned short flags;
    unsigned short serial;
    unsigned int field_0C;
    unsigned int active;
} Task;

typedef struct Actor {
    unsigned char pad00[0x0F];
    unsigned char handlerCount;
    unsigned char pad10[0x04];
    unsigned int value;
    unsigned int value2;
    int field_1C;
    unsigned char pad20[0x98 - 0x20];
    unsigned int flags;
} Actor;

extern Actor *D_8009D2F0;
extern Task *D_8009D300;
extern int D_8009CE00;

void func_8001A680(Actor *actor, unsigned int id);

int func_80013300(unsigned short **arg0) {
    Task *task = D_8009D300;
    unsigned short flags = task->flags;
    Actor *actor;

    if ((flags & 0x20) == 0) {
        task->flags = flags | 0x20;
        func_8001A680(D_8009D2F0, **arg0);
        actor = D_8009D2F0;
        actor->flags &= ~0x100u;
    } else {
        actor = D_8009D2F0;
        if (actor->handlerCount != 0) {
            if (actor->field_1C >= 0) {
                unsigned int left = actor->value;
                unsigned int right = actor->value2;

                if (left >= right) {
                    goto yield;
                }
            } else {
                unsigned int right = actor->value;
                unsigned int left = actor->value2;

                if (left >= right) {
                    goto yield;
                }
            }
        }
        task->flags = flags & 0xFFDF;
        return 1;
    }
yield:
    D_8009CE00 -= 12;
    D_8009D300->active = 1;
    return 0;
}
