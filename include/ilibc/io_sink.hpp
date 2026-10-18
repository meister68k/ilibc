// ilibc/io_sink.hpp
// Shared FD sink for <cstdio> (ILIBC-IO-001 sections 4, 5.2, 8).
// Format emission (fd_sink) and the plain string writers (fd_write_n)
// share the partial-write resend loop; the format path additionally caps
// the running count at INT_MAX so an int return value can never wrap.
// Header-only: every entity is inline, so no ilibc.a object is added
// (section 10). Self-contained (ILIBC-HDR-001 H2).
#pragma once

#include <ilibc/io_backend.hpp>
#include <climits>
#include <cstddef>

namespace ilibc::detail {

// Length of the NUL-terminated string s. Kept local so <cstdio> does not
// depend on <cstring>; the pointed-to object must be a valid NUL-
// terminated region (ILIBC-IO-001 section 8).
inline std::size_t cstr_len(const char *s) noexcept {
    std::size_t n = 0;
    while (s[n] != '\0') {
        ++n;
    }
    return n;
}

// Writes at most len bytes from buf to fd, resending the unwritten tail
// after every partial write, and returns how many bytes actually reached
// the backend. Stops at the first cannot-send (write_some <= 0) without
// retrying (ILIBC-IO-001 section 5.2); len == 0 touches neither fd nor
// buf and returns 0. This is the common resend loop of section 5.2, so
// the format sink and the plain writers behave identically.
inline std::size_t fd_write_n(int fd, const void *buf, std::size_t len) noexcept {
    const char *p = static_cast<const char *>(buf);
    std::size_t off = 0;
    while (off < len) {
        const std::size_t rest = len - off;
        // write_some takes unsigned int; capping each request at INT_MAX
        // keeps any single positive result representable as int (5.2)
        const std::size_t chunk =
            (rest > static_cast<std::size_t>(INT_MAX)) ? static_cast<std::size_t>(INT_MAX) : rest;
        const int w = write_some(fd, p + off, static_cast<unsigned int>(chunk));
        if (w <= 0) {
            break;  // cannot send: no progress, fail with what was sent
        }
        off += static_cast<std::size_t>(w);
    }
    return off;
}

// Format sink over an fd (ILIBC-IO-001 sections 4, 6): put accepts a
// chunk only when the whole chunk was written and the running count
// stays <= INT_MAX, so print_f can report -1 instead of an overflowing
// int. n counts accepted bytes and is meaningful only while every put
// has returned true.
struct fd_sink {
    int fd;
    std::size_t n = 0;

    explicit fd_sink(int f) noexcept : fd(f) {}

    bool put(const char *p, std::size_t len) noexcept {
        if (len > static_cast<std::size_t>(INT_MAX) - n) {
            return false;  // the int return value would overflow
        }
        if (fd_write_n(fd, p, len) != len) {
            return false;  // short write: the backend cannot send
        }
        n += len;
        return true;
    }
};

}  // namespace ilibc::detail
