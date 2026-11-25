/* libc qsort over guest memory (guest 0x800723A4), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * qsort), with no SDK source.  Evidence: docs/evidence/native-qsort-20261031/.
 *
 * The comparator is a guest function pointer, called as compare(a, b) with
 * the addresses of two elements; only the sign of its int result is used.
 * Elements are exchanged byte-wise by the swap helper (guest 0x800724F4). */
#include "pe_sdk.h"
#include "pe_guestcode.h"
#include "game_port.h"
#include "pe_guest_ram.h"

#define QSORT_SLOT    "qsort compare"
#define QSORT_CALLER  "func_800723A4"
#define QSORT_ARITY   2u
#define QSORT_PAIR    2u   /* a partition of two elements is compared directly */

/* Call the comparator on guest addresses a, b.  Returns nonzero when the
 * port asked to stop (the caller must unwind at once); *sign gets the int
 * result of the comparator. */
static int qsort_compare(pe_plat_guest_addr compare, uint32_t a, uint32_t b, int *sign)
{
    int ret = 0;

    (void)PE_GuestHook_Call(QSORT_SLOT, QSORT_CALLER, compare, QSORT_ARITY, a, b, 0u, 0u, &ret);
    *sign = ret;
    return PE_Port_ShouldStop() ? 1 : 0;
}

/* Exchange two distinct elements; an element is never exchanged with itself
 * (no stores are made for it). */
static void qsort_swap(uint32_t a, uint32_t b, uint32_t width)
{
    if (a != b)
        func_800724F4(a, b, width);
}

/* Sort n elements at base.  Observed rules (all fixed by the vectors):
 *  - n < 2: nothing happens (no comparator call, no store).
 *  - n == 2: compare(base, base + width); swap the two if the result is > 0.
 *  - otherwise the pivot is element n / 2 (integer division): it is swapped
 *    with element 0 first.  Then for i = 1 .. n-1 in order,
 *    compare(element i, element 0) and, if the result is < 0, the split
 *    point `last` is advanced by one (it starts at 0) and elements `last` and
 *    i are swapped.  After the scan the pivot (element 0) is swapped with
 *    element `last`.
 *  - Then the `last` elements before the pivot are sorted first, and the
 *    n-1-last elements after it second (this order is the comparator call
 *    order seen in the vectors).
 * The second (right-hand) sort is done in the loop, so only left-hand
 * partitions nest on the host stack.  Returns nonzero if the port stopped. */
static int qsort_range(uint32_t base, uint32_t n, uint32_t width, pe_plat_guest_addr compare)
{
    while (n >= 2u) {
        uint32_t i, last;
        int sign;

        if (n == QSORT_PAIR) {
            if (qsort_compare(compare, base, base + width, &sign))
                return 1;
            if (sign > 0)
                qsort_swap(base, base + width, width);
            return 0;
        }
        qsort_swap(base, base + (n / 2u) * width, width);
        last = 0u;
        for (i = 1u; i < n; i++) {
            if (qsort_compare(compare, base + i * width, base, &sign))
                return 1;
            if (sign < 0) {
                last++;
                qsort_swap(base + last * width, base + i * width, width);
            }
        }
        qsort_swap(base, base + last * width, width);
        if (qsort_range(base, last, width, compare))
            return 1;
        base += (last + 1u) * width;
        n -= last + 1u;
    }
    return 0;
}

void pe_plat_qsort(pe_plat_guest_addr base, uint32_t count, uint32_t width, pe_plat_guest_addr compare)
{
    /* Port integration (not original behaviour): once a port stop is pending,
     * guest state must not change (the pivot swap precedes any compare). */
    if (PE_Port_ShouldStop())
        return;
    (void)qsort_range(base, count, width, compare);
}
