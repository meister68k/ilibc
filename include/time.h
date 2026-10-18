// <time.h>  (stub)
#pragma once
#if defined(ILIBC_STUB_PASSTHROUGH)
#  include_next <time.h>
#else
#  error "ilibc: <time.h> is not provided (no clock). See ILIBC-OVR-001 section 4.3."
#endif
