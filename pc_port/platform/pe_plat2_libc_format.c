/* libc sprintf formatter (retail entry 0x80071A84, public name PE_FormatterFrame, prototype in pe_port_compat.h).
 *
 * PROVENANCE (clean-room, black-box).  Written by the cdrw-e author from:
 *   - the retail-derived black-box vector set cdgen62 (4190 cases: synthetic formats, synthetic %s strings and
 *     synthetic integers; classes tsv / alt / grid / star / chr / str / multi / edge) and its supplement cdgen62b
 *     (1780 cases: the three default flag words swept, %n / %hn / %hhn / %Ld; docs/evidence/native-cdx-20261126/
 *     cdgen62b/REPORT.md), read through their columns only (arguments, return value, ordered loads "L", ordered
 *     stores "S");
 *   - docs/evidence/native-cdx-20261126/cdgen62/REPORT.md, the checker pc_port/tools/pe_cdx_cdgen62_check.c with
 *     pe_cdx_common_canary.h and the run script pe_cdx_cdgen52_61_run.sh (what a case compares);
 *   - the suite's public retail table pc_port/tests/bios_sprintf_retail_vectors.tsv (535 rows; format, argument words,
 *     return value, output) and the public headers pe_plat/libc.h, pe_port_compat.h, pe_guest_ram.h, pe_bios_string.h,
 *     plus platform/pe_guest_ram.c (guest address translation: used only to decide the NULL %s host question);
 *   - the C standard's printf semantics as background.  Where the vectors differ from it the vectors win (listed below).
 * Incidental sightings (disclosed, none used to derive behaviour): three call lines of a game/boot caller of this entry
 * (grep output while looking for registrations: argument shapes only) and the full text of tests/test_formatter_frame.h
 * (a suite test of the previous provider, opened while looking for the registration; it names, but I never opened, the
 * case table it includes).  The length-prefixed '#' %s form was derived from the vectors (cases 3157, 3169, 3181: first
 * byte = length, len+1 bytes read) before that header was opened; its fixture only corroborated it.
 * NOT read: the file this one replaces (pc_port/game/boot/func_80071A84_port.c) or its history, docs/ai_context/
 * FORMATTER_CONTRACT.md, any translated or original code (src/, asm/, vendor/, game/decomp_hand/, game/boot bodies
 * other than the three caller lines above, pe_lib*.c, *_retail.c, the oracle sources under pc_port/tools, retail_*_cases.h,
 * evidence quoting original code).  Nothing below claims how the original was written; it is what the vectors show.
 *
 * Entry contract (the sprintf ABI).  destination, format and the first two argument words arrive in registers; the
 * caller's stack pointer is explicit.  The routine spills the format pointer and the two register arguments into the
 * caller's home area (caller_sp+4, +8, +0xC; the destination word at +0 is not spilled) and reads every argument word
 * from there, so argument k (k = 0 is the first variadic word) lives at caller_sp + 8 + 4k: the two spilled words, then
 * the caller's outgoing stack words from caller_sp+0x10.  The word at caller_sp+4 doubles as the format cursor: it is
 * re-stored as the cursor advances and its final value is observable in the image (vector "S" column).  The saved
 * registers handed in are not needed by any observable behaviour and are ignored.
 *
 * Output is written byte by byte straight into the destination, the closing NUL included; the return value is the
 * number of characters written without that NUL.  There is no capacity limit.
 *
 * What the vectors pin (case classes in brackets):
 *   - Format cursor: after the loop it addresses the terminating NUL; it stops at (does not step over) the character
 *     that ends parsing early.  While parsing a conversion it advances over each consumed character.  [all]
 *   - Conversions d i u x X o c s p and %%; flags - + space # 0 (any order, repeats allowed), width digits or *, '.'
 *     precision digits or * (a lone '.' is precision 0), length letters h and l (any number of them, in any order: %hhd,
 *     %lld and %ll are accepted; h narrows to 16 bits, l is ignored).  [grid, star, edge]
 *   - '*' width: a negative value left-justifies with its magnitude.  '.*' precision: a negative value means no
 *     precision.  The width is read before the precision, both before the value.  [star]
 *   - Any other conversion character (including NUL after a trailing '%') ends the format: the NUL is stored at the
 *     current output position and the characters written so far are returned.  No argument word is consumed.  [edge]
 *   - Integers: precision pads with zeros to that many digits, value 0 with precision 0 prints no digit; '0' pads with
 *     zeros only when there is no precision and no '-'; '+' / ' ' give a sign to signed conversions.  '#': x / X put
 *     "0x" / "0X" before the digits even for the value 0 (alt-form-zero quirk: "%#x" of 0 is "0x0", "%#.0x" is "0x");
 *     o puts a '0' in front when the value is not 0 and the digits do not already start with '0' ("%#.0o" of 0 is empty).
 *   - %p: the argument printed as upper-case hexadecimal with a minimum of eight digits (any precision is replaced), '#'
 *     adds "0p"; the h modifier narrows it to 16 bits.  [grid]
 *   - %c: the low byte of the argument, padded with spaces only (the 0 flag and precision have no effect).  [chr]
 *   - %s without '#': the string length is the count of bytes up to the NUL (the bytes up to and including the NUL are
 *     read); a precision limits the scan and the copy.  Width pads with spaces on the chosen side.  The pointer is read as
 *     given, a NULL pointer too, through the guest memory accessor (no host-side special case).  [str]
 *   - %s with '#': the string is length-prefixed: the first byte is the length, the text starts one byte later and may
 *     contain NULs; a precision still limits the count.  No NUL scan is made.  [str: 93 cases]
 *   - Argument reads: %c loads only the low byte of its argument word (one-byte load in the vectors); every other
 *     conversion loads the whole word.  Words are read when consumed, never ahead.
 *   - Default words (set cdgen62b, 71 triples swept): every conversion starts from the three guest words 0x80094528 /
 *     2C / 30 = initial state word, width and precision (read at each '%').  Digits accumulate onto them (width =
 *     width * 10 + digit, same for the precision after '.'; "%12d" under a default width of 12 is width 1212 and "%.0d"
 *     under a default precision of 3 is precision 30); '*' replaces them.  State word bits: 0 '-', 1 '+', 2 '#', 3 '0',
 *     4 precision given, 5 'h', 6 'l', 7 'L' (6 and 7 have no visible effect), 8..15 a sign character printed by signed
 *     conversions that are not negative and not '+' (the ' ' flag stores 0x20 there).  A precision word counts as
 *     "given" only through bit 4: without it the integer conversions still use it as a minimum digit count, but a zero
 *     value prints a digit, '0' padding is not suppressed and %s is not capped.  With '0' padding active (and bit 4
 *     clear) the minimum digit count is ignored.  '.' sets bit 4 only if the accumulated precision is not negative
 *     (a bare '.' keeps the default); a negative '.*' value clears bit 4 and zeroes the precision.  With bit 4 and a
 *     precision <= 0 a zero value prints no digit.  %s with '#' caps the length with a signed compare and a negative
 *     result moves the output position back (output and return value follow the signed length); %s without '#'
 *     scans at most (unsigned)precision bytes.  Zero words (the EXE image value) give the plain behaviour above.
 *   - %n stores the count so far through the argument pointer: a word, or a halfword with 'h' / 'hh'.  'L' is accepted
 *     as a length letter like 'l'.
 */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_plat/libc.h"
#include "pe_port_compat.h"

/* Per-conversion state word.  The initial value of every conversion is the guest word at 0x80094528 (see the header). */
enum {
    FMT_LEFT   = 1u << 0,   /* '-' */
    FMT_PLUS   = 1u << 1,   /* '+' */
    FMT_ALT    = 1u << 2,   /* '#' */
    FMT_ZERO   = 1u << 3,   /* '0' */
    FMT_PREC   = 1u << 4,   /* a precision was given ('.'): value 0 prints no digit, '0' flag ignored, %s is capped */
    FMT_SHORT  = 1u << 5,   /* 'h' */
    FMT_LONG   = 1u << 6,   /* 'l' (no effect) */
    FMT_LDBL   = 1u << 7,   /* 'L' (no effect) */
    FMT_SIGN_MASK = 0xFF00u /* bits 8..15: the sign character of a signed conversion that is not negative or '+' */
};
#define FMT_SIGN_SPACE 0x2000u
#define FLAG_WORD_ADDR 0x80094528u

/* Sink over the destination plus the walk state of one call. */
typedef struct {
    pe_addr_t out;        /* next destination byte */
    uint32_t count;       /* characters written (the return value) */
    pe_addr_t cursor;     /* format cursor, mirrored at sp+4 */
    pe_addr_t cursor_slot;
    pe_addr_t next_arg;   /* next argument word */
} FormatWalk;

static void put_byte(FormatWalk *w, uint8_t c)
{
    PE_StoreU8(w->out++, c);
    w->count++;
}

static void put_repeat(FormatWalk *w, uint8_t c, int32_t n)
{
    while (n > 0) {
        put_byte(w, c);
        n--;
    }
}

static void set_cursor(FormatWalk *w, pe_addr_t at)
{
    w->cursor = at;
    PE_StoreU32(w->cursor_slot, at);
}

static uint8_t peek_format(const FormatWalk *w)
{
    return PE_LoadU8(w->cursor);
}

static void skip_format(FormatWalk *w)
{
    set_cursor(w, w->cursor + 1u);
}

static uint32_t take_arg(FormatWalk *w)
{
    uint32_t v = PE_LoadU32(w->next_arg);
    w->next_arg += 4u;
    return v;
}

/* %c reads only the low byte of its argument word (the vectors record a one-byte load). */
static uint32_t take_arg_byte(FormatWalk *w)
{
    uint32_t v = PE_LoadU8(w->next_arg);
    w->next_arg += 4u;
    return v;
}

static int is_digit(uint8_t c)
{
    return c >= '0' && c <= '9';
}

/* Pad, copy a run of guest bytes through the byte mover, pad. */
static void emit_text(FormatWalk *w, unsigned flags, int32_t width, pe_addr_t text, int32_t len)
{
    int32_t pad = width - len;
    if (!(flags & FMT_LEFT))
        put_repeat(w, ' ', pad);
    if (len > 0)
        pe_plat_libc_move_bytes(w->out, text, (uint32_t)len);
    /* A negative length (a negative precision from the default word) moves the output position back. */
    w->out += (uint32_t)len;
    w->count += (uint32_t)len;
    if (flags & FMT_LEFT)
        put_repeat(w, ' ', pad);
}

static void emit_char(FormatWalk *w, unsigned flags, int32_t width, uint8_t c)
{
    int32_t pad = width - 1;
    if (!(flags & FMT_LEFT))
        put_repeat(w, ' ', pad);
    put_byte(w, c);
    if (flags & FMT_LEFT)
        put_repeat(w, ' ', pad);
}

static void emit_string(FormatWalk *w, unsigned flags, int32_t width, int32_t prec, uint32_t ptr)
{
    int32_t len = 0;
    if (flags & FMT_ALT) {
        /* Length-prefixed form: the first byte is the length; a precision caps it (signed compare). */
        len = (int32_t)PE_LoadU8(ptr);
        if ((flags & FMT_PREC) && len > prec)
            len = prec;
        emit_text(w, flags, width, ptr + 1u, len);
        return;
    }
    /* NUL scan; with a precision the scan is limited to that many bytes (the count is taken unsigned). */
    if (!(flags & FMT_PREC)) {
        while (PE_LoadU8(ptr + (uint32_t)len) != 0u)
            len++;
    } else {
        while ((uint32_t)len < (uint32_t)prec && PE_LoadU8(ptr + (uint32_t)len) != 0u)
            len++;
    }
    emit_text(w, flags, width, ptr, len);
}

/* One integer conversion.  `signed_conv` selects the sign handling, `base` 8 / 10 / 16, `upper` the hex digit case,
 * `prefix` the two characters put before the digits when '#' is present (0 = none). */
static void emit_integer(FormatWalk *w, unsigned flags, int32_t width, int32_t prec, uint32_t value, int negative,
                         int signed_conv, unsigned base, int upper, const char *prefix, int octal_alt)
{
    char digits[32];
    int nd = 0, np = 0, ns = 0, zeros = 0, i;
    char head[3];
    int32_t total, pad;
    unsigned left = flags & FMT_LEFT;
    int zero_fill;

    if (!(value == 0u && (flags & FMT_PREC) && prec <= 0)) {
        uint32_t v = value;
        do {
            unsigned d = (unsigned)(v % base);
            digits[nd++] = (char)(d < 10u ? '0' + d : (upper ? 'A' : 'a') + (d - 10u));
            v /= base;
        } while (v != 0u);
    }
    if (signed_conv) {
        if (negative) head[ns++] = '-';
        else if (flags & FMT_PLUS) head[ns++] = '+';
        else if (flags & FMT_SIGN_MASK) head[ns++] = (char)((flags & FMT_SIGN_MASK) >> 8);
    }
    if ((flags & FMT_ALT) && prefix) {
        head[ns + np++] = prefix[0];
        head[ns + np++] = prefix[1];
    }
    zero_fill = !left && (flags & FMT_ZERO) && !(flags & FMT_PREC);
    if (prec > nd && !zero_fill) zeros = prec - nd;
    if ((flags & FMT_ALT) && octal_alt && value != 0u && zeros == 0 && !(nd > 0 && digits[nd - 1] == '0')) {
        zeros = 1;
    }
    total = (int32_t)(ns + np + zeros + nd);
    pad = width - total;

    if (!left && !zero_fill)
        put_repeat(w, ' ', pad);
    for (i = 0; i < ns + np; i++)
        put_byte(w, (uint8_t)head[i]);
    if (zero_fill)
        put_repeat(w, '0', pad);
    put_repeat(w, '0', zeros);
    while (nd > 0)
        put_byte(w, (uint8_t)digits[--nd]);
    if (left)
        put_repeat(w, ' ', pad);
}

/* One conversion; the cursor is on the character after '%'.  Returns 0 when the format ends here. */
static int convert(FormatWalk *w)
{
    unsigned flags = PE_LoadU32(FLAG_WORD_ADDR);
    int32_t width = (int32_t)PE_LoadU32(FLAG_WORD_ADDR + 4u);
    int32_t prec = (int32_t)PE_LoadU32(FLAG_WORD_ADDR + 8u);
    int narrow;
    uint8_t c;
    uint32_t arg;

    for (;;) {
        c = peek_format(w);
        if (c == '-') flags |= FMT_LEFT;
        else if (c == '+') flags |= FMT_PLUS;
        else if (c == ' ') flags = (flags & ~FMT_SIGN_MASK) | FMT_SIGN_SPACE;
        else if (c == '#') flags |= FMT_ALT;
        else if (c == '0') flags |= FMT_ZERO;
        else break;
        skip_format(w);
    }
    if (c == '*') {
        width = (int32_t)take_arg(w);
        if (width < 0) {
            flags |= FMT_LEFT;
            width = (int32_t)(0u - (uint32_t)width);
        }
        skip_format(w);
        c = peek_format(w);
    } else {
        while (is_digit(c)) {
            width = (int32_t)((uint32_t)width * 10u + (uint32_t)(c - '0'));
            skip_format(w);
            c = peek_format(w);
        }
    }
    if (c == '.') {
        skip_format(w);
        c = peek_format(w);
        if (c == '*') {
            prec = (int32_t)take_arg(w);
            if (prec < 0) {
                flags &= ~FMT_PREC;
                prec = 0;
            } else {
                flags |= FMT_PREC;
            }
            skip_format(w);
            c = peek_format(w);
        } else {
            while (is_digit(c)) {
                prec = (int32_t)((uint32_t)prec * 10u + (uint32_t)(c - '0'));
                skip_format(w);
                c = peek_format(w);
            }
            /* The precision counts as given only when the accumulated value is not negative. */
            if (prec >= 0)
                flags |= FMT_PREC;
        }
    }
    while (c == 'h' || c == 'l' || c == 'L') {
        flags |= c == 'h' ? FMT_SHORT : c == 'l' ? FMT_LONG : FMT_LDBL;
        skip_format(w);
        c = peek_format(w);
    }
    narrow = (flags & FMT_SHORT) != 0u;

    switch (c) {
    case 'd':
    case 'i': {
        int32_t v;
        arg = take_arg(w);
        v = narrow ? (int32_t)(int16_t)arg : (int32_t)arg;
        emit_integer(w, flags, width, prec, v < 0 ? 0u - (uint32_t)v : (uint32_t)v, v < 0, 1, 10, 0, 0, 0);
        break;
    }
    case 'u':
        arg = take_arg(w);
        emit_integer(w, flags, width, prec, narrow ? (arg & 0xFFFFu) : arg, 0, 0, 10, 0, 0, 0);
        break;
    case 'x':
        arg = take_arg(w);
        emit_integer(w, flags, width, prec, narrow ? (arg & 0xFFFFu) : arg, 0, 0, 16, 0, "0x", 0);
        break;
    case 'X':
        arg = take_arg(w);
        emit_integer(w, flags, width, prec, narrow ? (arg & 0xFFFFu) : arg, 0, 0, 16, 1, "0X", 0);
        break;
    case 'o':
        arg = take_arg(w);
        emit_integer(w, flags, width, prec, narrow ? (arg & 0xFFFFu) : arg, 0, 0, 8, 0, 0, 1);
        break;
    case 'p':
        arg = take_arg(w);
        emit_integer(w, flags | FMT_PREC, width, 8, narrow ? (arg & 0xFFFFu) : arg, 0, 0, 16, 1, "0p", 0);
        break;
    case 'c':
        arg = take_arg_byte(w);
        emit_char(w, flags, width, (uint8_t)arg);
        break;
    case 's':
        arg = take_arg(w);
        emit_string(w, flags, width, prec, arg);
        break;
    case 'n':
        arg = take_arg(w);
        if (narrow)
            PE_StoreU16(arg, (uint16_t)w->count);
        else
            PE_StoreU32(arg, w->count);
        break;
    case '%':
        put_byte(w, '%');
        break;
    default:
        return 0;
    }
    skip_format(w);
    return 1;
}

uint32_t PE_FormatterFrame(pe_addr_t destination, pe_addr_t format, uint32_t arg0, uint32_t arg1, pe_addr_t caller_sp,
                           const uint32_t saved[9])
{
    FormatWalk w;
    (void)saved;
    w.out = destination;
    w.count = 0u;
    w.cursor_slot = caller_sp + 4u;
    w.next_arg = caller_sp + 8u;
    PE_StoreU32(caller_sp + 4u, format);
    PE_StoreU32(caller_sp + 8u, arg0);
    PE_StoreU32(caller_sp + 12u, arg1);
    w.cursor = format;
    for (;;) {
        uint8_t c = peek_format(&w);
        if (c == 0u)
            break;
        skip_format(&w);
        if (c != '%') {
            put_byte(&w, c);
            continue;
        }
        if (!convert(&w))
            break;
    }
    PE_StoreU8(w.out, 0u);
    return w.count;
}
