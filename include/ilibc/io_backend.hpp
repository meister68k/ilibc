// ilibc/io_backend.hpp
// I/O backend selection and the write_some/read_some boundaries for
// <cstdio> (ILIBC-IO-001 sections 5.1, 5.2 and 14.5). A backend must be
// selected before the first include of <cstdio>; an unselected or
// reserved-only configuration is a hard #error here, as section 5.2
// requires.
// Internal header: only <cstdio> includes it (section 10), and it must be
// self-contained (ILIBC-HDR-001 H2). No macros are defined (H5), so the
// selection macros are the only configuration surface.
#pragma once

#include <climits>

#if defined(ILIBC_IO_BACKEND_POSIX) && defined(ILIBC_IO_BACKEND_CUSTOM)
// Section 5.1: selecting both is contradictory and never resolved.
#  error "ilibc: ILIBC_IO_BACKEND_POSIX and ILIBC_IO_BACKEND_CUSTOM are mutually exclusive (ILIBC-IO-001 section 5.1)"
#elif defined(ILIBC_IO_BACKEND_CUSTOM)
// Section 5.1: CUSTOM is reserved for a future OS port and intentionally
// unimplemented in the first version.
#  error "ilibc: ILIBC_IO_BACKEND_CUSTOM is not implemented yet (ILIBC-IO-001 section 5.1)"
#elif defined(ILIBC_IO_BACKEND_POSIX) || defined(ILIBC_HOST_IO_TEST)
// POSIX backend (section 5.1): ILIBC_HOST_IO_TEST is an accepted alias of
// ILIBC_IO_BACKEND_POSIX and may be defined together with it. This branch
// is the only place that may reach <unistd.h>.
#  include <unistd.h>

// Host errno cell for error transfer (section 14.5): ::read failures must
// report the host-side error number, but including the host <errno.h>
// would leak macros that collide with ilibc's public <cerrno> names, and
// ilibc's own <errno.h> is an #error stub. The POSIX/E1 libc therefore
// declares its thread-local accessor directly; the declaration matches
// the C++ declaration of the libc (C language linkage, noexcept).
extern "C" int* __errno_location() noexcept;

namespace ilibc::detail {

// Sends up to size bytes and returns the positive count actually written.
// A non-positive ::write result means no progress and is reported as -1
// ("0/positive: sendable" contract, section 5.2); a result above INT_MAX
// cannot be represented as int and also becomes -1. EINTR is not retried
// and write atomicity is not guaranteed. A partial write is a positive
// count; resending the tail is the FD sink's job (section 5.2).
inline int write_some(int fd, const void *buf, unsigned int size) noexcept {
    const ssize_t n = ::write(fd, buf, size);
    if (n <= 0 || n > INT_MAX) {
        return -1;
    }
    return static_cast<int>(n);
}

// Reads up to size bytes (section 14.5): returns a positive byte count,
// 0 for EOF (including a zero-size request), and -1 on error. The
// effective request is capped at INT_MAX so the result always fits in
// int; callers that may read more (fread) loop over partial reads.
// On error the host error number is captured immediately after ::read
// and copied to *err when given; success and EOF never touch *err, and
// err == nullptr discards the report. EINTR is not retried, matching the
// no-automatic-retry rule of section 14.5.
inline int read_some(int fd, void *buf, unsigned int size, int *err) noexcept {
    const unsigned int req =
        (size > static_cast<unsigned int>(INT_MAX))
            ? static_cast<unsigned int>(INT_MAX)
            : size;
    const ssize_t n = ::read(fd, buf, req);
    if (n < 0) {
        if (err != nullptr) {
            *err = *__errno_location();
        }
        return -1;
    }
    // n >= 0 and n <= req <= INT_MAX, so the narrowing is in range.
    return static_cast<int>(n);
}

}  // namespace ilibc::detail
#else
// No backend selected (section 5.1): <cstdio> is unavailable while
// <ilibc/format.hpp> keeps working without any I/O (sections 1, 2.1).
// The wording also satisfies the stub message rule of ILIBC-HDR-001
// section 7.3 S1 for the unselected <cstdio> include.
#  error "ilibc: <cstdio> is not provided (no I/O). Define ILIBC_IO_BACKEND_POSIX (or ILIBC_HOST_IO_TEST) before including <cstdio>. See ILIBC-IO-001 section 5.1."
#endif
