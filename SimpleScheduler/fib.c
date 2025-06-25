#include<stdio.h>
#include<stdlib.h>
unsigned long fib(int n){
    if(n < 2){
        return n;
    }
    else return(fib(n-1)+fib(n-2));
}
int main()
{
    int n = 45;
    printf("nth fib num is %lu\n", fib(n));

}
