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

extern Task *D_8009CDFC;
extern unsigned short D_8009D308;

Task *func_80012700(unsigned int entry, Task *parent)
{
    Task *task;
    Task *next;

    task = D_8009CDFC;
    D_8009CDFC = task->next;

    if (parent != 0) {
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

    task->field_0C = 0;
    task->entry = entry;
    task->field_04 = 0;
    task->active = 1;
    task->flags = 0;
    task->serial = D_8009D308++;

    return task;
}
