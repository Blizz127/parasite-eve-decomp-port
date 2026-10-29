/* Check the Windows registry policy with host compilation, not a Win32 build. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#define _WIN32 1
#include "../platform/plat_enhance.c"
int main(void)
{
    int index = pe_enh_option_index("display_scaler");
    const PeEnhOption *option = pe_enh_option(index);
    assert(option && option->nvalues == 2);
    assert(option->values[0] == PE_ENH_SCALER_NEAREST);
    assert(option->values[1] == PE_ENH_SCALER_SHARP_BILINEAR);
    assert(snap(index, PE_ENH_SCALER_FSR1) == PE_ENH_SCALER_SHARP_BILINEAR);
    assert(strcmp(option->names[1], "SHARP BILINEAR") == 0);
    return 0;
}
