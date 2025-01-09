#include <stdint.h>

int main() {
    volatile int a = 30;
    volatile int b = 26;
    volatile int c;
    a = b + 10;
    c = a - 6;

    return 0;
}
