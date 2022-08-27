#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){
    int p[2];
    pipe(p);

    if(fork() == 0){
        // child process
        char s[8];
        read(p[0], s, 1);
        fprintf(1, "%d: received ping\n", getpid());
        write(p[1], s, 1);
        close(p[0]);
        close(p[1]);
    }else{
        char s[8];
        write(p[1], s, 1);
        sleep(5);
        read(p[0], s, 1);
        fprintf(1, "%d: received pong\n", getpid());
        close(p[0]);
        close(p[1]);
    }

    exit(0);
}