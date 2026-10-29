/* Test-only trap for the field-VM differential. The generated handler
 * reaches PE_Decomp_GuestTrapDivZero only for a zero guest divisor; the
 * retail divisor is a byte plus one, so any call fails the case. */
#include <stdlib.h>

void PE_Decomp_GuestTrapDivZero(const char *fn);

void PE_Decomp_GuestTrapDivZero(const char *fn)
{
    (void)fn;
    abort();
}
