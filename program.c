#include<stdio.h>

int func(int n, int j) {
    if (n < j || n == 0 ) {
        return n;
    } else {
        return j;
    }
}

int main() {
    printf("%d\n", func(2, 3));
    return 0;
}
