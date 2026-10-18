#include <ilibc/format.hpp>
static int eq(const char *a, const char *b) {
    while (*a != 0 && *a == *b) { ++a; ++b; }
    return *a == 0 && *b == 0;
}
int main() {
    char buf[16];
    int n = ilibc::sprintf<"x=%d">(buf, 42);
    int m = ilibc::snprint_f<"abc">(nullptr, 0);
    return (n == 4 && eq(buf, "x=42") && m == 3) ? 0 : 1;
}
