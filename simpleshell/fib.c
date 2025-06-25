#include<stdio.h>
#include<stdlib.h>
unsigned long fib(int n){
    if(n < 2){
        return n;
    }
    else return(fib(n-1)+fib(n-2));
}
int main(int argc, char* argv[])
{
    if(argc != 2){
        printf("Enter the value of n for which you want nth fibonacci number\n");
        exit(1);
    }
    printf("nth fib num is %lu\n", fib(atoi(argv[1])));

}