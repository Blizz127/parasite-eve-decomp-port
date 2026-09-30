typedef struct {
    unsigned short dest_type;
    unsigned char dest_id;
    unsigned char payload;
    unsigned int extra;
    unsigned int sender;
} MailboxRecord;

extern MailboxRecord D_800A3180[];
extern unsigned char D_8009CDB4;

void func_800653B8(unsigned int payload, unsigned int dest_id,
                   unsigned int dest_type, unsigned int sender,
                   unsigned int extra) {
    MailboxRecord *record = &D_800A3180[D_8009CDB4];

    record->payload = payload;
    record->dest_id = dest_id;
    record->sender = sender;
    record->dest_type = dest_type;
    record->extra = extra;
    D_8009CDB4++;
}
