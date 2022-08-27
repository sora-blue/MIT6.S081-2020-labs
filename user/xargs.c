#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

static const int BUF_SIZE = 512;

int main(int argc, char *argv[])
{
    if(argc < 2){
        fprintf(1, "Usage: xargs [command]\n");
        exit(0);
    }
    char *p;
    char buf[BUF_SIZE];
    char *buf_args[MAXARG];
    int i, len, status;


    for(i = 1; i < argc; i++){
        len = strlen(argv[i]);
        if(len >= BUF_SIZE){
            fprintf(2, "xargs: argument too long\n");
            exit(1);
        }
        // mem leak risk but exit-at-once & disposable code
        buf_args[i-1] = (char *)malloc(len+1);
        memmove(buf_args[i-1], argv[i], len);
        buf_args[i-1][len] = 0;
    }
    i = argc - 1;

    for(p = buf; p < BUF_SIZE + buf && i < MAXARG && read(0, p, 1); ){
        // printf("buf: #%s#\n", buf);
        if(*p == ' '){
            len = p - buf;
            if(len > 0){
                buf_args[i] = (char *)malloc(len+1);
                memmove(buf_args[i], buf, len);
                buf_args[i++][len] = 0;
            }
            p = buf;
            continue;
        }
        if(*p == '\n')
        {
            // last argument
            len = p - buf;
            if(len > 0){
                buf_args[i] = (char *)malloc(len+1);
                memmove(buf_args[i], buf, len);
                buf_args[i++][len] = 0;
            }
            p = buf;
            if(fork() == 0){
                // child process
                // int j = 0;
                // for(j = 0; j < i ; j++){
                //     printf("#%s#\n", buf_args[j]);
                // }
                buf_args[i] = 0;
                exec(buf_args[0], buf_args);
                fprintf(2, "xargs: exec failed\n");
                exit(1);
            }else{
                // parent process
                while(wait(&status) != -1){}
                i = argc - 1;
            }
            continue;
        }
        p++;
    }
    if(p >= BUF_SIZE + buf){
        fprintf(2, "xargs: argument too long\n");
        exit(1);
    }
    if(i >= MAXARG){
        fprintf(2, "xargs: too much arguments\n");
        exit(1);
    }
    exit(0);
}
