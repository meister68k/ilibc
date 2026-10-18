// assert.h
// C compatibility wrapper for <cassert> (ILIBC-HDR-001 section 4.5).
// Intentionally has no include guard (H1): the wrapper forwards to <cassert>,
// which redefines assert on every inclusion so that NDEBUG changes are
// reflected. Brings in no names of its own, because assert is a macro and
// therefore needs no using-declaration (W1, W4, section 6.2).
#include <cassert>
