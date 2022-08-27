#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define false 0
#define true 1

int isPrime(int number){
    if(number < 2) return false;
    int i;
    for(i = 2; i < number ; i++){
        if(number % i == 0)
            return false;
    }
    return true;
}

int childRun(int *p){
        int prime, childP[2], status;
        close(p[1]);
        read(p[0], &prime, sizeof(prime));
        fprintf(1, "prime %d\n", prime);
        while(read(p[0], &prime, sizeof(prime))){
            if(!isPrime(prime)){
                continue;
            }
            pipe(childP);
            if(fork() == 0){
                // grand child
                close(p[0]);
                childRun(childP);
            }else{
                // this process
                close(childP[0]);
                write(childP[1], &prime, sizeof(prime));
                while(read(p[0], &prime, sizeof(prime))){
                    write(childP[1], &prime, sizeof(prime));
                }
                close(childP[1]);
                while(wait(&status) != -1){
                    continue;
                }
            }
            break;
        }
        return 0; 
}

int main(){
    close(0);
    int p[2], i, status;
    pipe(p);
    if(fork() == 0){
        // child process
        childRun(p);
        exit(0);
    }
    // parent process
    close(p[0]);
    for(i = 2 ; i <= 35 ; i++){
        write(p[1], &i, sizeof(i));
    }
    close(p[1]);
    while(wait(&status) != -1){
        continue;
    }
    
    exit(0);
}