int func(int n, int j) {
    if (n == j || n >= 0) {
        if (n >= j || n >= 0) {
            if (n >= j || n >= 0) {
                return n;
            } else {
                return j;
            }
        }
        if (n >= j) {
            return 2;
        } else if (n >= j || n == j || n >= 2 && j >= n) {
            return 12;
        }
        return 1;
    }
}

int func2(int n, int j) {
    if (n == j && n <= 0) {
        if (n <= j && n <= 0) {
            if (n <= j && n <= 0) {
                return n;
            } else {
                return j;
            }
        }
        if (n <= j) {
            return 2;
        } else if (n <= j && n == j && n == 2 && j <= n) {
            return 12;
        }
        return 1;
    }
}