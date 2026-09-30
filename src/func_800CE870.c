typedef struct PositionSource {
    unsigned char pad_00[0x14];
    int field_14;
    int field_18;
    int field_1C;
} PositionSource;

typedef struct PositionObject {
    unsigned char pad_00[0x28];
    int field_28;
    int field_2C;
    int field_30;
    unsigned char pad_34[0x204];
    PositionSource *source;
} PositionObject;

void func_800CE870(PositionObject *obj, int mode, short *out)
{
    switch (mode) {
    case 0:
        out[0] = obj->source->field_14;
        out[1] = obj->source->field_18;
        out[2] = obj->source->field_1C;
        break;
    case 1:
        out[0] = obj->field_28 >> 16;
        out[1] = obj->field_2C >> 16;
        out[2] = obj->field_30 >> 16;
        break;
    }
}
