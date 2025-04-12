int main(){
    int n = 10, fact = 1;

    for (int i = 1; i <= n; i++) {
        fact *= i;
    }

    return fact;
}