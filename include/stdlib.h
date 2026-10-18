// stdlib.h
// C compatibility wrapper for <cstdlib>: introduces the 6 function names
// plus size_t into the global namespace with using-declarations
// (ILIBC-STL-001 section 2.2, ILIBC-HDR-001 section 6). Declares nothing of
// its own (W1). The macros EXIT_SUCCESS, EXIT_FAILURE and MB_CUR_MAX come
// from <cstdlib>. The extension API ilibc::_itoa_s and the err versions are
// NOT introduced into the global namespace (W6).
// Must not be used together with ILIBC_HOST_TEST (W5).
#pragma once

#include <cstdlib>

using ILIBC_NS::abs;      using ILIBC_NS::labs;    using ILIBC_NS::atoi;
using ILIBC_NS::atol;     using ILIBC_NS::strtol;  using ILIBC_NS::strtoul;
using ILIBC_NS::size_t;
