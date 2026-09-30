/* VRAM 0x8003FFCC / file 0x307CC / size 0xA0. */
typedef struct {
    unsigned char state;
    unsigned char pad1[0x28];
    unsigned char f29;
    unsigned char pad2[0x1A];
} CardEntry;

typedef struct {
    unsigned char sel;
    unsigned char status;
    unsigned char pad[0x1A];
    CardEntry e[15];
} CardRec;

extern CardRec D_800A0ED4[2];

int func_8003FFCC(void) {
    CardRec *c;
    CardEntry *e;
    int found;

    found = 0;
    for (c = D_800A0ED4; c < &D_800A0ED4[2] && found == 0; c++) {
        if (c->status == 0xF) {
            for (e = c->e; e < &c->e[15]; e++) {
                found = (e->state == 1) && (e->f29 != 0);
                if (found) {
                    break;
                }
            }
        }
    }
    return found;
}
