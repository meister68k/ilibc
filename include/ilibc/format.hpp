// ilibc/format.hpp
// I/O-independent compile-time formatting: the public snprint_f and the
// array-safe sprintf of ILIBC-IO-001 sections 2.1, 3, 4, and 7, plus the
// detail parser, plan, sinks, and emit helpers shared with <cstdio>
// (section 4). Users may include this header directly (ILIBC-HDR-001
// section 2.1); it must stay free of ilibc/io_backend.hpp, <unistd.h>,
// heap, va_arg, and floating point so that it compiles and links without
// any I/O backend (TC-IO-01, TC-IO-17). Header-only: every entity is a
// template or an inline constexpr object, so no .cpp and no link symbols.
#pragma once

#include <ilibc/config.hpp>
#include <ilibc/stdlib.hpp>
#include <climits>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace ilibc::detail {

// Structural string that can be used as a non-type template parameter.
// Deduced from a string literal, terminator byte included, so embedded NULs
// remain visible to the parser (ILIBC-IO-001 section 3).
template <std::size_t N>
struct fixed_string {
    char data[N];

    constexpr fixed_string(const char (&s)[N]) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            data[i] = s[i];
        }
    }

    // Format length without the array terminator
    static constexpr std::size_t size() noexcept { return N - 1; }
};

// What the compile-time parser can reject (ILIBC-IO-001 section 3)
enum class format_err : unsigned char {
    none,
    embedded_nul,   // a NUL byte appears before the end of the literal
    bad_token,      // '%' not followed by 'd', 's', or '%'
};

// One compiled output step
enum class step_kind : unsigned char {
    text,     // literal run, emitted verbatim
    percent,  // "%%": begin points at the first '%', one byte goes out
    dec,      // %d, consumes one int argument
    str,      // %s, consumes one const char*-convertible argument
};

// A step as (kind, offset, length) inside the format literal. begin/len of
// a conversion cover the two format bytes; they are kept for uniformity.
struct format_step {
    step_kind kind;
    std::size_t begin;
    std::size_t len;
};

// Parsed format plan. The step array is a fixed member bounded by
// F.size() + 1 (every step consumes at least one format byte), so the plan
// needs no allocation and stays inside the bounded-resource rules.
template <std::size_t N>
struct format_plan {
    format_step steps[N + 1];
    std::size_t count;
    std::size_t conversions;  // number of %d/%s steps
    format_err err;
};

// Parses F into a plan at compile time. The runtime only walks this plan
// and never scans for '%' (ILIBC-IO-001 section 3). On an error the plan is
// emptied so that the caller's validation stops at the first diagnostic.
template <fixed_string F>
constexpr format_plan<F.size()> parse_plan() noexcept {
    format_plan<F.size()> plan = {};
    std::size_t i     = 0;
    std::size_t text  = 0;  // start of the pending literal run

    while (i < F.size()) {
        const char c = F.data[i];
        if (c == '\0') {                                        // embedded NUL
            plan.err = format_err::embedded_nul;
            plan.count = 0;
            plan.conversions = 0;
            return plan;
        }
        if (c != '%') {
            ++i;
            continue;
        }

        // Flush the literal run that ends at this '%'
        if (i > text) {
            plan.steps[plan.count] = {step_kind::text, text, i - text};
            ++plan.count;
        }
        if (i + 1 >= F.size()) {                                // lone '%'
            plan.err = format_err::bad_token;
            plan.count = 0;
            plan.conversions = 0;
            return plan;
        }

        const char spec = F.data[i + 1];
        step_kind kind;
        if (spec == 'd') {
            kind = step_kind::dec;
            ++plan.conversions;
        } else if (spec == 's') {
            kind = step_kind::str;
            ++plan.conversions;
        } else if (spec == '%') {
            kind = step_kind::percent;
        } else {                                                // %x, %u, flags, ...
            plan.err = format_err::bad_token;
            plan.count = 0;
            plan.conversions = 0;
            return plan;
        }
        plan.steps[plan.count] = {kind, i, 2};
        ++plan.count;
        i += 2;
        text = i;
    }

    // Trailing literal run
    if (F.size() > text) {
        plan.steps[plan.count] = {step_kind::text, text, F.size() - text};
        ++plan.count;
    }
    return plan;
}

// The plan of F as an inline constexpr object; the emission walk reads it
// instead of re-parsing the literal (ILIBC-IO-001 section 3).
template <fixed_string F>
inline constexpr format_plan<F.size()> format_plan_v = parse_plan<F>();

// Sink contract used by the emit helpers: put returns false when the sink
// cannot accept the bytes (counter overflow, ...), which stops the walk.

// Counts output bytes without storing them; sprintf uses it as its first
// pass (ILIBC-IO-001 sections 4 and 7). Fails before n could cross INT_MAX
// so the int return values never wrap.
struct count_sink {
    std::size_t n = 0;

    bool put(const char *, std::size_t len) noexcept {
        if (len > static_cast<std::size_t>(INT_MAX) - n) {
            return false;
        }
        n += len;
        return true;
    }
};

// Fixed caller-provided buffer. pos is the number of stored bytes and is
// capped at cap-1 so the terminator always fits; needed is the length
// before truncation and likewise never crosses INT_MAX (section 4).
struct buffer_sink {
    char *dst;
    std::size_t cap;
    std::size_t pos = 0;
    std::size_t needed = 0;

    constexpr buffer_sink(char *d, std::size_t c) noexcept : dst(d), cap(c) {}

    bool put(const char *p, std::size_t len) noexcept {
        if (len > static_cast<std::size_t>(INT_MAX) - needed) {
            return false;
        }
        if (cap > 0 && pos < cap - 1) {
            const std::size_t room = (cap - 1) - pos;
            const std::size_t copy = (len < room) ? len : room;
            for (std::size_t i = 0; i < copy; ++i) {
                dst[pos + i] = p[i];
            }
            pos += copy;
        }
        needed += len;
        return true;
    }
};

// Emits the n bytes of p to the sink (ILIBC-IO-001 section 4)
template <class Sink>
[[nodiscard]] bool emit_literal(Sink &sink, const char *p, std::size_t n) noexcept {
    return sink.put(p, n);
}

// Formats value as signed decimal through ilibc::_itoa_s (section 4). The
// 12-byte stack buffer holds the worst case "-2147483648" plus the
// terminator, and the terminator itself is not sent to the sink.
template <class Sink>
[[nodiscard]] bool emit_decimal(Sink &sink, int value) noexcept {
    char buf[12];
    if (ilibc::_itoa_s(value, buf, sizeof buf, 10) != 0) {
        return false;  // unreachable: radix 10 always fits in 12 bytes
    }
    std::size_t len = 0;
    while (buf[len] != '\0') {
        ++len;
    }
    return emit_literal(sink, buf, len);
}

// Emits the bytes before the terminator of s (section 4). The string must
// be a valid NUL-terminated object; a null pointer violates the contract
// of %s (section 3).
template <class Sink>
[[nodiscard]] bool emit_string(Sink &sink, const char *s) noexcept {
    ILIBC_ASSERT(s != nullptr);
    std::size_t len = 0;
    while (s[len] != '\0') {
        ++len;
    }
    return emit_literal(sink, s, len);
}

// Defined below; emit_conv continues with it after consuming one argument.
template <fixed_string F, std::size_t I, class Sink, class... Args>
[[nodiscard]] bool emit_steps(Sink &sink, Args &&...args) noexcept;

// Emits the conversion at step I. Kind is a constant expression, and its
// argument is the head of the pack, so the pack shrinks by exactly one per
// conversion and every argument expression is evaluated once by the caller
// (ILIBC-IO-001 section 3). A type that fails validation returns true
// without emitting because check_format has already reported it.
template <fixed_string F, std::size_t I, step_kind Kind, class Sink, class Head, class... Tail>
[[nodiscard]] bool emit_conv(Sink &sink, Head &&head, Tail &&...tail) noexcept {
    bool sent = false;
    if constexpr (Kind == step_kind::dec) {
        if constexpr (std::is_same_v<std::remove_cvref_t<Head>, int>) {
            sent = emit_decimal(sink, static_cast<int>(head));
        } else {
            return true;
        }
    } else {
        if constexpr (std::is_convertible_v<std::remove_cvref_t<Head>, const char *>) {
            sent = emit_string(sink, static_cast<const char *>(head));
        } else {
            return true;
        }
    }
    if (!sent) {
        return false;
    }
    return emit_steps<F, I + 1>(sink, std::forward<Tail>(tail)...);
}

// Walks the plan starting at step I. Every branch is selected by a constant
// expression, so no '%' scan or other format interpretation happens at
// runtime (ILIBC-IO-001 section 3).
template <fixed_string F, std::size_t I, class Sink, class... Args>
[[nodiscard]] bool emit_steps(Sink &sink, Args &&...args) noexcept {
    constexpr const format_plan<F.size()> &plan = format_plan_v<F>;

    if constexpr (I >= plan.count) {
        // check_format proved conversions == sizeof...(Args), so no argument
        // is left over and nothing follows the last step.
        return true;
    } else if constexpr (plan.steps[I].kind == step_kind::text) {
        if (!emit_literal(sink, F.data + plan.steps[I].begin, plan.steps[I].len)) {
            return false;
        }
        return emit_steps<F, I + 1>(sink, std::forward<Args>(args)...);
    } else if constexpr (plan.steps[I].kind == step_kind::percent) {
        if (!emit_literal(sink, F.data + plan.steps[I].begin, 1)) {
            return false;
        }
        return emit_steps<F, I + 1>(sink, std::forward<Args>(args)...);
    } else {
        if constexpr (sizeof...(Args) > 0) {
            return emit_conv<F, I, plan.steps[I].kind>(sink, std::forward<Args>(args)...);
        } else {
            return true;  // mismatch already reported by check_format
        }
    }
}

// Compile-time validation of F against the argument types
// (ILIBC-IO-001 section 3). The four flags form a priority chain (NUL,
// syntax, count, types): a flag below a failing one is forced true so that
// exactly one static_assert in check_format fires.
template <fixed_string F, class... Args>
struct format_check {
    static constexpr format_plan<F.size()> parsed = parse_plan<F>();

    // Whether the I-th argument can back the I-th conversion of its kind
    static constexpr bool types_match() noexcept {
        if constexpr (sizeof...(Args) == 0) {
            return true;
        } else {
            constexpr bool dec_ok[] = {std::is_same_v<std::remove_cvref_t<Args>, int>...};
            constexpr bool str_ok[] = {std::is_convertible_v<std::remove_cvref_t<Args>, const char *>...};
            std::size_t ci = 0;
            for (std::size_t s = 0; s < parsed.count; ++s) {
                const step_kind k = parsed.steps[s].kind;
                if (k == step_kind::dec || k == step_kind::str) {
                    if (k == step_kind::dec ? !dec_ok[ci] : !str_ok[ci]) {
                        return false;
                    }
                    ++ci;
                }
            }
            return true;
        }
    }

    static constexpr bool no_nul = (parsed.err != format_err::embedded_nul);
    static constexpr bool syntax_ok = (parsed.err != format_err::bad_token);
    // An erroneous plan carries no usable counts, so count/type checks are
    // suppressed and the parser's diagnostic stands alone.
    static constexpr bool count_ok = (parsed.err != format_err::none)
                                         ? true
                                         : (parsed.conversions == sizeof...(Args));
    static constexpr bool types_ok = count_ok ? types_match() : true;
};

// Instantiating this function is what reports a bad format; one failing
// flag, one message (ILIBC-IO-001 section 3).
template <fixed_string F, class... Args>
constexpr void check_format() noexcept {
    static_assert(format_check<F, Args...>::no_nul,
                  "ilibc: format string contains an embedded NUL");
    static_assert(format_check<F, Args...>::syntax_ok,
                  "ilibc: invalid format token (only %d, %s, %%)");
    static_assert(format_check<F, Args...>::count_ok,
                  "ilibc: format conversion count does not match arguments");
    static_assert(format_check<F, Args...>::types_ok,
                  "ilibc: %d needs exactly int; %s needs const char*-convertible");
}

}  // namespace ilibc::detail

namespace ilibc {

// Formatted write into a raw buffer (ILIBC-IO-001 sections 2.1, 7).
// capacity > 0 stores at most capacity-1 characters and always NUL-
// terminates; capacity == 0 stores nothing and accepts a null dst. The
// return value is the required length before truncation, or -1 when that
// length would exceed INT_MAX. capacity > 0 with a null dst violates the
// contract, as does overlapping a %s input with the destination. No access
// to write or errno.
template <detail::fixed_string F, class... Args>
int snprint_f(char *dst, std::size_t capacity, Args &&...args) noexcept {
    detail::check_format<F, Args...>();
    ILIBC_ASSERT(capacity == 0 || dst != nullptr);

    detail::buffer_sink sink(dst, capacity);
    const bool sent = detail::emit_steps<F, 0>(sink, std::forward<Args>(args)...);
    if (capacity > 0) {
        dst[sink.pos] = '\0';  // pos never exceeds capacity-1
    }
    if (!sent) {
        return -1;
    }
    return static_cast<int>(sink.needed);
}

// Array-safe sprintf (ILIBC-IO-001 sections 2.1, 7): two passes over the
// compiled plan, first counting with a count_sink, then writing only when
// N holds the required length plus the terminator. Any failure returns -1
// and leaves dst completely unchanged. The argument expressions are
// evaluated once (they are function arguments) even though their values are
// read twice, and the referenced strings must not change between the
// passes. A bare char* never matches the array reference parameter, so the
// pointer form of sprintf does not compile.
template <detail::fixed_string F, std::size_t N, class... Args>
int sprintf(char (&dst)[N], Args &&...args) noexcept {
    detail::check_format<F, Args...>();

    // First pass: measure without touching dst
    detail::count_sink probe;
    if (!detail::emit_steps<F, 0>(probe, std::forward<Args>(args)...)) {
        return -1;  // required length exceeds INT_MAX
    }
    if (probe.n + 1 > N) {
        return -1;  // capacity short; dst stays untouched
    }

    // Second pass: the count already proved the write fits
    detail::buffer_sink sink(&dst[0], N);
    if (!detail::emit_steps<F, 0>(sink, std::forward<Args>(args)...)) {
        return -1;  // unreachable: the first pass accepted this length
    }
    dst[sink.pos] = '\0';
    return static_cast<int>(probe.n);
}

}  // namespace ilibc
