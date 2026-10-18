// string.h
// C compatibility wrapper for <cstring>: introduces the 18 function names
// plus size_t into the global namespace with using-declarations
// (ILIBC-STR-001 section 2.3, ILIBC-HDR-001 section 6). Declares nothing of
// its own (H4/W1). NULL comes from <cstddef> via <cstring>.
// Must not be used together with ILIBC_HOST_TEST (ILIBC-HDR-001 W5).
#pragma once

#include <cstring>

using ILIBC_NS::memcpy;   using ILIBC_NS::memmove; using ILIBC_NS::memcmp;
using ILIBC_NS::memchr;   using ILIBC_NS::memset;
using ILIBC_NS::strcpy;   using ILIBC_NS::strncpy; using ILIBC_NS::strcat;
using ILIBC_NS::strncat;  using ILIBC_NS::strcmp;  using ILIBC_NS::strncmp;
using ILIBC_NS::strchr;   using ILIBC_NS::strrchr; using ILIBC_NS::strstr;
using ILIBC_NS::strspn;   using ILIBC_NS::strcspn; using ILIBC_NS::strpbrk;
using ILIBC_NS::strlen;   using ILIBC_NS::size_t;
