// ilibc/stdlib.hpp
// The integer-parsing core (detail), the err-returning extensions of
// ILIBC-STL-001 section 2.3, and the non-standard extension ilibc::_itoa_s
// (sections 2.5, 4.5). Internal header, pulled in by <cstdlib>; users must
// not include it directly (ILIBC-HDR-001 section 2.1). Header-only: all
// functions are constexpr inline, no .cpp, no link symbols.
#pragma once

#include <ilibc/config.hpp>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstddef>

namespace ilibc::detail {

// Result of the shared conversion scan (ILIBC-STL-001 section 5.1).
struct parse_result {
    unsigned long magnitude;   // absolute value; indeterminate when overflow is set
    const char   *end;         // one past the last converted digit; input head when none
    bool          negative;    // the sign was '-'
    bool          overflow;    // the magnitude did not fit in unsigned long
};

// Digit value of c; 36 (greater than any valid base) when c is not a digit.
constexpr int digit_value(char c) noexcept {
    if (c >= '0' && c <= '9') { return c - '0'; }
    if (c >= 'a' && c <= 'z') { return c - 'a' + 10; }
    if (c >= 'A' && c <= 'Z') { return c - 'A' + 10; }
    return 36;
}

// Shared scan of leading whitespace, at most one sign, an optional 0x/0
// prefix (base 0 or 16), and the digit run (ILIBC-STL-001 section 4.2).
// Overflow is decided BEFORE the multiply that could wrap (section 5.3):
// once the digit run overflows, the magnitude is frozen but digits keep
// being consumed so that end still lands past the last valid digit.
constexpr parse_result parse_integer(const char *s, int base) noexcept {
    parse_result r = { 0UL, s, false, false };                  // default: no conversion
    if (base < 0 || base == 1 || base > 36) { return r; }       // invalid base: no error (D-L1)

    std::size_t i = 0;
    while (ILIBC_NS::isspace(s[i])) { ++i; }                    // leading whitespace (C locale)
    if (s[i] == '+' || s[i] == '-') { r.negative = (s[i] == '-'); ++i; }

    // Prefix handling: 0x/0X only for base 0 or 16, and only when a hex
    // digit follows; a bare "0x" keeps the "0" as the conversion (4.2).
    if ((base == 0 || base == 16) && s[i] == '0' && (s[i + 1] == 'x' || s[i + 1] == 'X')
        && digit_value(s[i + 2]) < 16) {
        i += 2;
        base = 16;
    } else if (base == 0) {
        base = (s[i] == '0') ? 8 : 10;
    }

    // Digit run: s[i] is read at most up to the terminator (s[i+2] is only
    // read when s[i+1] is 'x'/'X', so the string has at least that byte).
    const unsigned long b     = static_cast<unsigned long>(base);
    const unsigned long limit = ULONG_MAX / b;                  // largest value that can take one more digit
    const unsigned long last  = ULONG_MAX % b;                  // largest last digit at that value
    bool any = false;
    for (;; ++i) {
        const int d = digit_value(s[i]);
        if (d >= base) { break; }
        any = true;
        const unsigned long dv = static_cast<unsigned long>(d);
        if (r.overflow || r.magnitude > limit || (r.magnitude == limit && dv > last)) {
            r.overflow = true;                                  // keep scanning; freeze the value
        } else {
            r.magnitude = r.magnitude * b + dv;
        }
    }
    if (any) { r.end = s + i; }
    return r;
}

}  // namespace ilibc::detail

namespace ilibc {

// strtol with an explicit error sink (ILIBC-STL-001 section 2.3).
// Writes *err only on range overflow; errno is never touched. The negative
// range admits |LONG_MIN| = LONG_MAX + 1 via unsigned negation, so
// "-2147483648" is in range while "2147483648" is ERANGE (sections 5.2, 5.3).
constexpr long strtol(ILIBC_NONNULL const char *nptr, char **endptr, int base, int *err) noexcept {
    const detail::parse_result r = detail::parse_integer(nptr, base);
    if (endptr != nullptr) { *endptr = const_cast<char *>(r.end); }

    constexpr unsigned long max_pos = static_cast<unsigned long>(LONG_MAX);
    const unsigned long limit = r.negative ? max_pos + 1UL : max_pos;  // allow |LONG_MIN| when negative
    if (r.overflow || r.magnitude > limit) {
        if (err != nullptr) { *err = ILIBC_ERANGE; }
        return r.negative ? LONG_MIN : LONG_MAX;
    }
    return r.negative ? static_cast<long>(0UL - r.magnitude)     // negate unsigned, then convert (C++20 two's complement)
                      : static_cast<long>(r.magnitude);
}

// strtoul with an explicit error sink (ILIBC-STL-001 section 2.3). A
// leading '-' negates the magnitude modulo 2^32; only a magnitude larger
// than ULONG_MAX itself is ERANGE (section 4.2 rule 7).
constexpr unsigned long strtoul(ILIBC_NONNULL const char *nptr, char **endptr, int base,
                                int *err) noexcept {
    const detail::parse_result r = detail::parse_integer(nptr, base);
    if (endptr != nullptr) { *endptr = const_cast<char *>(r.end); }

    if (r.overflow) {
        if (err != nullptr) { *err = ILIBC_ERANGE; }
        return ULONG_MAX;
    }
    return r.negative ? 0UL - r.magnitude : r.magnitude;         // unsigned negation
}

// atol with an explicit error sink: identical to strtol base 10 (4.3).
constexpr long atol(ILIBC_NONNULL const char *nptr, int *err) noexcept {
    return ilibc::strtol(nptr, nullptr, 10, err);
}

// atoi with an explicit error sink: the strtol base-10 result narrowed to
// int; identical range on ILP32, enforced by config.hpp (4.3).
constexpr int atoi(ILIBC_NONNULL const char *nptr, int *err) noexcept {
    return static_cast<int>(ilibc::strtol(nptr, nullptr, 10, err));
}

// Safe integer-to-string conversion (ILIBC-STL-001 section 4.5). Argument
// checks are part of the specification (not undefined behaviour) and are
// evaluated in a fixed order: (1) null buffer or zero size leaves the
// buffer untouched, (2) a bad radix empties buffer[0], (3) a too-small
// buffer empties buffer[0] and nothing else. errno is never touched, so
// even failing inputs stay evaluable in constant expressions.
[[nodiscard]] constexpr int _itoa_s(int value, char *buffer, std::size_t size, int radix) noexcept {
    if (buffer == nullptr || size == 0) {                        // 1: argument error; do not touch buffer
        return ILIBC_EINVAL;
    }
    if (radix < 2 || radix > 36) {                               // 2: radix error
        buffer[0] = '\0';
        return ILIBC_EINVAL;
    }

    const bool negative = (radix == 10 && value < 0);            // '-' only for decimal negatives
    unsigned magnitude = static_cast<unsigned>(value);           // two's-complement bit pattern
    if (negative) { magnitude = 0U - magnitude; }                // absolute value; handles INT_MIN
    const unsigned base = static_cast<unsigned>(radix);

    std::size_t digits = 1;                                      // digit count; zero is one digit
    for (unsigned t = magnitude; t >= base; t /= base) { ++digits; }
    const std::size_t length = digits + (negative ? 1U : 0U);    // characters excluding the terminator

    if (size < length + 1) {                                     // 3: buffer too small
        buffer[0] = '\0';
        return ILIBC_ERANGE;
    }

    buffer[length] = '\0';
    const std::size_t first = negative ? 1U : 0U;
    for (std::size_t i = length; i > first; ) {                  // write digits back to front
        --i;
        const unsigned d = magnitude % base;
        buffer[i] = static_cast<char>(d < 10U ? '0' + d : 'a' + (d - 10U));
        magnitude /= base;
    }
    if (negative) { buffer[0] = '-'; }
    return 0;
}

// Array form: the buffer size comes from the array type (section 2.5 #2).
// Only char arrays match; unsigned char / signed char arrays are errors.
template <std::size_t N>
[[nodiscard]] constexpr int _itoa_s(int value, char (&buffer)[N], int radix) noexcept {
    return ilibc::_itoa_s(value, buffer, N, radix);
}

}  // namespace ilibc
