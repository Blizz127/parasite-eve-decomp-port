/* VRAM 0x800131E8 / size 0x118. Pop a script task off D_8009CDFC,
 * point it at actor+0x9C + (imm << 1), and either link it under
 * D_8009D300 (flags & 3 == 0) or prepend it on actor+0xA8.
 * era -O2 -G8. D_8009D2F0 is an incomplete pointer array so the
 * load stays absolute. */

typedef struct Task {
    unsigned int entry;
    unsigned int field_04;
    unsigned short flags;
    unsigned short serial;
    unsigned int field_0C;
    unsigned int active;
    unsigned char pad_14[0x10];
    struct Task *next;
    struct Task *prev;
} Task;

typedef struct {
    char pad00[0x9C];
    unsigned int unk9C;
    char padA0[0x08];
    Task *listA8;
} Actor;

extern Actor *D_8009D2F0[];
extern Task *D_8009D300;
extern Task *D_8009CDFC;
extern unsigned short D_8009D308;

int func_800131E8(unsigned int **arg0) {
    Task *cur = D_8009D300;

    if ((cur->flags & 3) != 0) {
        unsigned int idx = **(volatile unsigned int **)arg0;
        Actor *actor = D_8009D2F0[0];
        Task *task = D_8009CDFC;
        unsigned short serial = D_8009D308;
        unsigned int base = actor->unk9C;
        Task *saved = task->next;
        Task *head;

        task->prev = 0;
        task->next = 0;
        task->field_0C = 0;
        task->field_04 = 0;
        task->active = 1;
        task->serial = serial;
        task->flags = 0;
        task->entry = (idx << 1) + base;
        head = actor->listA8;
        D_8009D308 = (unsigned short)(serial + 1);
        D_8009CDFC = saved;
        task->next = head;
        if (head != 0) {
            head->prev = task;
        }
        D_8009D2F0[0]->listA8 = task;
    } else {
        Task *parent = cur;
        Task *task = D_8009CDFC;
        unsigned int idx = **(volatile unsigned int **)arg0;
        register Actor *actor asm("$4") = D_8009D2F0[0];
        Task *saved = task->next;
        unsigned int entry;
        unsigned short serial;

        D_8009CDFC = saved;
        entry = (idx << 1) + *(volatile unsigned int *)&actor->unk9C;
        if (parent != 0) {
            Task *next;

            task->prev = parent;
            next = parent->next;
            task->next = next;
            if (next != 0) {
                next->prev = task;
            }
            parent->next = task;
        } else {
            task->prev = 0;
            task->next = 0;
        }
        task->entry = entry;
        serial = D_8009D308;
        task->field_0C = 0;
        task->field_04 = 0;
        task->active = 1;
        task->flags = 0;
        D_8009D308 = (unsigned short)(serial + 1);
        task->serial = serial;
    }
    return 1;
}
