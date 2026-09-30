typedef struct {
    unsigned char pad_00[0x34];
    int multiplier;
    unsigned char pad_38[0x0C];
    int base;
    int factor;
    unsigned char pad_4C[0x28];
    int flags;
} ValueRecord;

int func_8006346C(ValueRecord *record)
{
    int index = -1;
    int result;

    if (record != 0) {
        int base = record->base;

        if (base >= 0) {
            index = record->factor;
            if (index >= 0) {
                index = record->multiplier * index + base;
            } else {
                index = -1;
            }
        }
    }

    result = -1;
    if (((record->flags >> index) & 1) != 0) {
        result = index;
    }

    return result;
}
