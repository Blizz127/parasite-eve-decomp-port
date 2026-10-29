/* Focused actual-host dispatch check; links existing runtime archive, no mock. */
#include "pe_guest_ram.h"
#define PE_LIBERATE_FOCUSED_ONLY 1
#include "test_liberate_native.h"
int main(void)
{
    char why[256]={0};PE_RamInit();
    int ok=PE_TestLiberateDispatch(why,sizeof why);
    printf("LIBERATE_NATIVE_DISPATCH %s%s%s\n",ok?"PASS":"FAIL",ok?"":": ",why);
    PE_RamDestroy();return ok?0:1;
}
