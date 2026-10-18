#include <cerrno>
#if EDOM != 33 || ERANGE != 34 || EINVAL != 22
#error cerrno values mismatch in #if
#endif
#if !(EDOM > 0 && ERANGE > 0 && EINVAL > 0)
#error cerrno codes must be positive
#endif
#if EDOM == ERANGE || EDOM == EINVAL || ERANGE == EINVAL
#error cerrno codes must be distinct
#endif
static_assert(EDOM==33 && ERANGE==34 && EINVAL==22);
int main(){errno=0;++errno;int* p=&errno;return (ILIBC_ERRNO==1 && *p==1)?0:1;}
