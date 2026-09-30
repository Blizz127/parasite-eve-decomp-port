/* Remaining enemy check: walk D20C body list, call victory if empty.
 * VRAM 0x800292EC / file 0x92EC / size 0x98 (39 words).
 * gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Walks D_8009D20C linked list. For each body: if body != Aya and
 * body+0x00 != 0, sets v1=0 (enemy exists). After walk: if v1==1
 * (no enemies) and Aya HP > 0, calls func_8002F300 (victory/mode 2).
 *
 * ROM: prologue at 0x80029300 (after initial list-load and NULL check).
 * Unusual prologue placement: lui/lw/beq before frame setup.
 */
extern int D_8009D20C;  /* enemy body list head (gp+0x48C) */
extern int D_8009D254;  /* Aya actor pointer (gp+0x534) */
extern int D_8009D278;  /* attack record pointer (gp+0x558) */

extern void func_8002F300(void);  /* victory/mode 2 producer */

void func_800292EC(void) {
    int body = *(int *)&D_8009D20C;
    int no_enemy = 1;

    if (body != 0) {
        int aya = *(int *)&D_8009D254;
        while (body != 0) {
            if (body != aya && *(int *)body != 0)
                no_enemy = 0;
            body = *(int *)(body + 4);
        }
    }

    if ((no_enemy & 0xFF) != 0) {
        int rec = *(int *)&D_8009D278;
        if (*(short *)(rec + 0x0C) > 0)
            func_8002F300();
    }
}
