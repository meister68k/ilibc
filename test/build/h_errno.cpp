#include <errno.h>
#if EDOM != 33 || ERANGE != 34 || EINVAL != 22
#error errno.h values mismatch in #if
#endif
int main(){errno=0;++errno;int* p=&errno;return (*p==1 && errno==1)?0:1;}
