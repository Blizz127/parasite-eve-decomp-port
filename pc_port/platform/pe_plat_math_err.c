/* Math-library error reporter (guest 0x800739C4), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * errmath), with no SDK source.  Evidence: docs/evidence/native-errmath-20261031/. */
#include "pe_sdk.h"
#include "pe_plat/libc.h"
#include "pe_guest_ram.h"

/* Last reported error: the code word, then the argument word. */
#define MATH_ERR_CODE_ADDR   0x80094564u
#define MATH_ERR_ARG_ADDR    0x80094568u

/* Codes that also raise a software event.  The event class is shared; the
 * spec differs per code. */
#define MATH_ERR_CODE_A      0x21u
#define MATH_ERR_CODE_B      0x22u
#define MATH_ERR_EVENT_CLASS 0xF4000002u
#define MATH_ERR_SPEC_A      0x301u
#define MATH_ERR_SPEC_B      0x302u

/* Record the error, then deliver the event for codes 0x21 and 0x22 (whole
 * word equality; the event's return value is ignored).  Always returns 0. */
int pe_plat_math_err(int code, int arg)
{
    PE_StoreU32(MATH_ERR_CODE_ADDR, (uint32_t)code);
    PE_StoreU32(MATH_ERR_ARG_ADDR, (uint32_t)arg);
    if ((uint32_t)code == MATH_ERR_CODE_A)
        PE_Event_Deliver(MATH_ERR_EVENT_CLASS, MATH_ERR_SPEC_A);
    else if ((uint32_t)code == MATH_ERR_CODE_B)
        PE_Event_Deliver(MATH_ERR_EVENT_CLASS, MATH_ERR_SPEC_B);
    return 0;
}
