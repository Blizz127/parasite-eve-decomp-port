typedef struct {
    unsigned char pad_00[0x34];
    int multiplier;
    unsigned char pad_38[0x0C];
    int base;
    int factor;
} ValueRecord;

int func_80063428(ValueRecord *record) {
    int result = -1;

    if (record != 0) {
        int base = record->base;

        if (base >= 0) {
            int factor = record->factor;

            if (factor >= 0) {
                result = record->multiplier * factor + base;
            }
        }
    }

    return result;
}
