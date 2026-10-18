// ilibc/config.hpp
// Prerequisites, namespace/asm-label macros, internal assertion, version
// (ILIBC-HDR-001 sections 8.2-8.4).
// Users must not edit this header (H9); configure with -D on the command line.
#pragma once

// (a) language level and compiler
#if __cplusplus < 202002L
#  error "ilibc: C++20 or later is required (C++23 recommended)."
#endif
#if !defined(__GNUC__)
#  error "ilibc: GCC or Clang is required (see ILIBC-OVR-001 D-14)."
#endif

// (b) platform assumptions (ILIBC-OVR-001 section 2.1)
#include <climits>
static_assert(CHAR_BIT == 8, "ilibc: 8-bit char is required");
static_assert(sizeof(int) == 4 && sizeof(long) == 4 && sizeof(void *) == 4,
              "ilibc: ILP32 is required");
static_assert(sizeof(int) == sizeof(long), "ilibc: int and long must have the same size");
static_assert('A' == 0x41 && 'a' == 0x61 && '0' == 0x30, "ilibc: ASCII is required");

// (c) namespace and asm label
#if defined(ILIBC_HOST_TEST)
#  define ILIBC_NS ilibc_under_test
#  define ILIBC_ASM_LABEL(name)
#else
#  define ILIBC_NS std
#  define ILIBC_ASM_LABEL(name) __asm__(name)
#endif
#define ILIBC_NS_BEGIN namespace ILIBC_NS {
#define ILIBC_NS_END   }

// (d) attribute: nullable by default, user may predefine it (e.g. as [[gnu::nonnull]])
#ifndef ILIBC_NONNULL
#  define ILIBC_NONNULL
#endif

// (e) internal assertion (ILIBC-HDR-001 section 4.4)
#if defined(ILIBC_DEBUG)
#  include <ilibc/assert_fail.hpp>
#  define ILIBC_ASSERT(...)                                                    \
     (static_cast<bool>(__VA_ARGS__) ? static_cast<void>(0)                    \
                                     : ::__ilibc_assert_fail(#__VA_ARGS__, __FILE__, __LINE__))
#else
#  define ILIBC_ASSERT(...) (static_cast<void>(0))
#endif

// (f) identification (ILIBC-HDR-001 section 8.3)
#define ILIBC_VERSION_MAJOR 0
#define ILIBC_VERSION_MINOR 1
#define ILIBC_VERSION_PATCH 0
#define ILIBC_VERSION (ILIBC_VERSION_MAJOR * 10000 + ILIBC_VERSION_MINOR * 100 + ILIBC_VERSION_PATCH)
