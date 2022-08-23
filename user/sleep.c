#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int num;

  if(argc < 2){
    fprintf(2, "Usage: sleep time\n");
    exit(1);
  }

  num = atoi(argv[1]);
  sleep(num * 10);
  exit(0);
}