#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

static const int BUF_SIZE = 512;

char *
filename(char *path)
{
    char *p;

    for(p=path+strlen(path); p >= path && *p != '/'; p--){
        
    }
    return p + (*p == '/');
}

int find(char *path, char *target, char *buf){
    char *p;
    int fd, len;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return 1;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        return 1;
    }

    switch(st.type){
        case T_FILE:
            if(!strcmp(filename(path), target)){
                fprintf(1, "%s\n", path);
            }
            break;
        case T_DIR:
            strcpy(buf, path);
            p = buf + strlen(buf);
            p[0] = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)){
                if(de.inum == 0)
                    continue;
                if(de.name[0] == '.'){
                    continue;
                }
                if(strlen(buf) + 1 + strlen(de.name) > BUF_SIZE){
                    fprintf(1, "find: path too long\n");
                    continue;
                }
                len = strlen(de.name);
                memmove(p+1, de.name, len);
                p[len+1] = 0;
                find(buf, target, buf);
            }
            break;
    }
    close(fd);
    return 0;
}

int main(int argc, char *argv[])
{
    char buf[BUF_SIZE];
    close(0);
    if(argc < 3){
        fprintf(1, "Usage: find [directory] [filename]\n");
        exit(0);
    }
    exit(find(argv[1], argv[2], buf));
}
