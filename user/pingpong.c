#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(int argc, int* argv[])
{
  char data = 'B';    //要传输的数据
  char buf[8];        //数据缓冲区

  int fds_p2c[2];     //存储父进程到子进程的文件描述符的数组
  int fds_c2p[2];     //存储子进程到父进程的文件描述符的数组

  //创建管道
  if ((pipe(fds_p2c) < 0) || (pipe(fds_c2p) < 0)) {
    //创建管道失败，打印错误信息，异常退出
    fprintf(2, "pingpong:pipe() filed\n");
    exit(1);
  }

  //创建子进程
  int pid = fork();
  if (pid < 0) {
    //创建子进程失败，打印错误信息并关闭文件描述符，异常退出
    fprintf(2, "pingpong:fork() filed\n");
    close(fds_p2c[0]);
    close(fds_p2c[1]);
    close(fds_c2p[0]);
    close(fds_c2p[1]);
    exit(1);
  }

  if (pid > 0) {
    //父进程
    close(fds_p2c[0]);
    close(fds_c2p[1]);

    if (write(fds_p2c[1], &data, 1) != 1) {
      //写失败
      fprintf(2, "pingpong:write() filed\n");
      exit(1);
    }

    if (read(fds_c2p[0], &buf, 1) != 1) {
      //读失败
      fprintf(2, "pingpong:read() filed\n");
      exit(1);
    }
    
    fprintf(1, "%d: received pong\n", getpid());

    wait(0);
  }
  else if (pid == 0) {
    //子进程
    close(fds_p2c[1]);
    close(fds_c2p[0]);

    if (read(fds_p2c[0], &buf, 1) != 1) {
      //读失败
      fprintf(2, "pingpong:read() filed\n");
      exit(1);
    }

    fprintf(1, "%d: received ping\n", getpid());

    if (write(fds_c2p[1], &buf, 1) != 1) {
      //写失败
      fprintf(2, "pingpong:write() filed\n");
      exit(1);
    }
  }

  exit(0);
}
