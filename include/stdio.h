// <stdio.h>  (stub)
#pragma once
#if defined(ILIBC_STUB_PASSTHROUGH)
#  include_next <stdio.h>
#else
#  error "ilibc: <stdio.h> is not provided (no I/O). See ILIBC-OVR-001 section 4.3."
#endif
