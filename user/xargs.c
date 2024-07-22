#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAX_SIZE 512

int
main(int argc, char* argv[])
{
  //参数小于2个，直接报错，异常退出
  if (argc < 2) {
    fprintf(2, "usage: xargs command [arg ...]\n");
    exit(1);
  }

  //存放子进程exec的参数
  char* x_argv[MAXARG];
  for (int i = 1; i < argc; i++) {
    x_argv[i - 1] = argv[i];
  }

  //从管道中读取数据
  char ch;
  char temp[MAX_SIZE];
  int i = 0;
  while (read(0, &ch, 1) > 0) {
    //未到结尾
    if (ch != '\n') {
      temp[i++] = ch;
    }
    else {
      temp[i] = 0;
      i = 0;

      int pid = fork();
      if (pid < 0) {
        //直接报错，异常退出
        fprintf(2, "xargs:fork() failed\n");
        exit(1);
      }

      if (pid > 0) {
        //父进程
        wait(0);
      }
      else {
        //子进程
        x_argv[argc - 1] = temp;
        if (exec(argv[1], x_argv) < 0) {
          //直接报错，异常退出
          fprintf(2, "xargs:exec() failed\n");
          exit(1);
        }
      }
    }
  }

  //正常退出
  exit(0);
}
