#include <cstdlib>
static_assert(EXIT_SUCCESS==0 && EXIT_FAILURE==1 && MB_CUR_MAX==1);
int main(){return (ILIBC_NS::abs(-1)==1 && std::atoi("42")==42 && std::strtol("10",nullptr,10)==10L)?0:1;}
