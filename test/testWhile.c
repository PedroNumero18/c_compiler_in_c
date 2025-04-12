int main(){
    int n = 10; 
    int fact = 1;
    int i = 0;
    while(i < n) {
        fact = fact * i;
        i = i + 1;
    }
    return fact;
}