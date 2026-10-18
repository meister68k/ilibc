// errno.h
// C compatibility wrapper for <cerrno>: brings in only the macros and the
// errno variable; declares nothing of its own (ILIBC-HDR-001 section 5.3,
// section 6 W1). Must not be used together with ILIBC_HOST_TEST (W5).
#pragma once

#include <cerrno>
