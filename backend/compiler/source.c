int factorial(int n) {
    int result;
    int i;
    int temp;
    int one;
    int zero;
    one = 1;
    zero = 0;

    result = one + zero;
    i = n;
    
    if (n <= one + zero) {
        return result;
    }

    while (i > one * one) {
        temp = result * i;
        result = temp + zero;
        i = i - (one + zero);
    }

    return result;
}

int main() {
    int x;
    int y;
    x = 2 + 3 + 0;
    y = factorial(x);
    return y + 0;
}