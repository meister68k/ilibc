// ctype.h
// C compatibility wrapper for <cctype>: introduces the 13 names into the
// global namespace with using-declarations (ILIBC-CTP-001 section 2.2,
// ILIBC-HDR-001 section 6). Declares nothing of its own (H4).
// Must not be used together with ILIBC_HOST_TEST (ILIBC-HDR-001 W5).
#pragma once

#include <cctype>

using ILIBC_NS::isalnum;
using ILIBC_NS::isalpha;
using ILIBC_NS::iscntrl;
using ILIBC_NS::isdigit;
using ILIBC_NS::isgraph;
using ILIBC_NS::islower;
using ILIBC_NS::isprint;
using ILIBC_NS::ispunct;
using ILIBC_NS::isspace;
using ILIBC_NS::isupper;
using ILIBC_NS::isxdigit;
using ILIBC_NS::tolower;
using ILIBC_NS::toupper;
