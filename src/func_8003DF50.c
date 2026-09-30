typedef struct Record16 {
    unsigned short field_00;
    unsigned short field_02;
    unsigned short field_04;
    unsigned short field_06;
    unsigned char pad_08[8];
} Record16;

typedef struct Source {
    unsigned char pad_00[0x18];
    Record16 *records;
} Source;

typedef struct Destination {
    unsigned int field_00;
    unsigned char pad_04[0x20];
    Source *source;
    unsigned char pad_28[4];
    unsigned short field_2C;
    unsigned short field_2E;
    unsigned short field_30;
    unsigned short field_32;
    unsigned char pad_34[0x3C];
    unsigned short field_70;
} Destination;

void func_8003DF50(Destination *dst, Source *source, short index)
{
    dst->field_32 = index;
    dst->field_00 = 0;
    dst->source = source;
    dst->field_70 = source->records[index].field_06;
    dst->field_2C = source->records[index].field_00;
    dst->field_2E = source->records[index].field_02;
    dst->field_30 = source->records[index].field_04 + dst->field_70;
}
