#include <ilibc/format.hpp>
char b[16];
int main(){return ilibc::snprint_f<"a%d">(b,16,1)==2?0:1;}
