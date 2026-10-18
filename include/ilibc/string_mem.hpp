// ilibc/string_mem.hpp
// Byte-pointer cores of the five mem* functions and the memchr base body
// (ILIBC-STR-001 sections 2.4, 5.1, 7.2). Internal header, pulled in by
// <cstring>; users must not include it directly (ILIBC-HDR-001 section 2.1).
#pragma once

#include <ilibc/config.hpp>
#include <cstddef>

namespace ilibc::detail {

// Copy the first n bytes from s to d, front to back (section 5.1). Overlapping
// ranges give unspecified results; memmove selects this core or mem_copy_bwd.
constexpr void mem_copy_fwd(unsigned char *d, const unsigned char *s, std::size_t n) noexcept {
    for (std::size_t i = 0; i < n; ++i) { d[i] = s[i]; }
}

// Copy the first n bytes from s to d, back to front (section 5.1). Used when
// d > s so that a source byte is read before any copy overwrites it.
constexpr void mem_copy_bwd(unsigned char *d, const unsigned char *s, std::size_t n) noexcept {
    while (n > 0) { --n; d[n] = s[n]; }
}

// Compare the first n bytes as unsigned char; returns the difference of the
// first differing bytes, 0 when equal or n is 0 (sections 4.4, 5.1).
constexpr int mem_compare(const unsigned char *a, const unsigned char *b, std::size_t n) noexcept {
    for (std::size_t i = 0; i < n; ++i) {
        if (a[i] != b[i]) { return static_cast<int>(a[i]) - static_cast<int>(b[i]); }
    }
    return 0;
}

// Return the address of the first byte equal to c within the first n bytes,
// or nullptr when there is none (n == 0 included) (section 5.1).
constexpr const unsigned char *mem_find(const unsigned char *s, unsigned char c,
                                        std::size_t n) noexcept {
    for (std::size_t i = 0; i < n; ++i) {
        if (s[i] == c) { return s + i; }
    }
    return nullptr;
}

// Set the first n bytes to c (section 5.1).
constexpr void mem_fill(unsigned char *d, unsigned char c, std::size_t n) noexcept {
    for (std::size_t i = 0; i < n; ++i) { d[i] = c; }
}

// Base body of memchr: the asm label fixes the C link name "memchr", so the
// two std:: overloads stay wrappers (ILIBC-STR-001 section 7.2). Defined in
// src/string/memchr.cpp with -fno-tree-loop-distribute-patterns (section 3).
void *memchr_base(const void *s, int c, std::size_t n) noexcept ILIBC_ASM_LABEL("memchr");

}  // namespace ilibc::detail
