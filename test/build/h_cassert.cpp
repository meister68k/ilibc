#include <cassert>
constexpr int f(int x){assert(x>=0);return x;}
static_assert(f(1)==1);
int main(){assert(1==1);return 0;}
