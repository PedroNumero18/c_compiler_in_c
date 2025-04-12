int sum_squares(int n) {
    int sum = 0;
    int i = 1;
    
    while (i <= n) {
        sum = sum + (i * i);
        i = i + 1;
    }
    
    return sum;
}

int power(int base, int exp) {
    int result = 1;
    while (exp > 0) {
        result = result * base;
        exp = exp - 1;
    }
    return result;
}

int main() {
    int x = 5;
    int squares = sum_squares(x);
    int cube = power(x, 3);
    
    return squares + cube;
}