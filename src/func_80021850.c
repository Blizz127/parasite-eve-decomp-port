typedef signed char s8;

typedef struct {
    unsigned int word0;
    unsigned int word1;
    unsigned int word2;
} Record12;

void func_80021850(Record12 *records, s8 first, s8 second) {
    Record12 temporary = records[first];

    records[first] = records[second];
    records[second] = temporary;
}
