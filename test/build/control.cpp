#include <climits>
int main(int argc, char **) {
    const int c = (argc > 1) ? 66 : 65;
    return (c >= 65 && c <= 90 && c != 32) ? 0 : 1;
}
