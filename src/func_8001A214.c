typedef struct Actor { char pad[0x28]; int x; int y; int z; } Actor;
extern Actor *D_8009D2F0;
extern int func_80077DC4(int);
extern int func_80077CF4(int);
extern int func_8003708C(int, int);
int func_8001A214(int **args)
{
    short angle = 0x1400 - *args[1];
    int radius = *args[0];
    *args[2] = func_8003708C(-radius, func_80077DC4(angle) << 4);
    *args[3] = func_8003708C(-radius, func_80077CF4(angle) << 4);
    *args[2] += D_8009D2F0->x;
    *args[3] += D_8009D2F0->z;
    return 1;
}
