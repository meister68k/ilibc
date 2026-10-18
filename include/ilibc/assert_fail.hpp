// ilibc/assert_fail.hpp
// Declaration of the assertion-failure hook (ILIBC-HDR-001 section 4.3).
// The user's program provides the definition; if it does not, the default
// weak symbol from libilibc_hooks.a is used. The function must not return.
#pragma once

extern "C" [[noreturn]] void __ilibc_assert_fail(const char *expr, const char *file, int line) noexcept;
