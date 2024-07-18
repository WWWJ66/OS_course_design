#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int get_left_first_num(int l_fds[2], int* dst);
void trans(int l_fds[2], int r_fds[2], int lfn);
void primes(int l_fds[2]);


int
main(void)
{
  int num = 35;

  int fds[2];//存储文件描述符的数组

  //创建管道
  if (pipe(fds) < 0) {
    //创建管道失败，打印错误信息，异常退出
    fprintf(2, "primes:pipe() filed\n");
    exit(1);
  }

  //写入初始数据
  for (int i = 2; i <= num; i++) {
    if (write(fds[1], &i, sizeof(int)) != sizeof(int)) {
      //写失败，打印错误信息，异常退出
      fprintf(2, "primes:write() filed\n");
      exit(1);
    }
  }

  //创建子进程
  int pid = fork();
  if (pid < 0) {
    //创建子进程失败，打印错误信息并关闭文件描述符，异常退出
    fprintf(2, "primes:fork() filed\n");
    close(fds[0]);
    close(fds[1]);
    exit(1);
  }

  if (pid > 0) {
    //父进程
    close(fds[0]);
    close(fds[1]);
    wait(0);
  }
  else {
    //子进程
    primes(fds);
  }

  //正常退出
  exit(0);
}

/*
* 功能：获取左侧的第一个数到dst中
* 参数：int l_fds[2]:左侧存储文件描述符的数组
*       int* dst:目标变量
* 返回值：如果左侧还有数，返回0；否则返回-1
*/
int get_left_first_num(int l_fds[2], int* dst)
{
  if (read(l_fds[0], dst, sizeof(int)) == sizeof(int)) {
    return 0;
  }

  return -1;
}

/*
* 功能：向右侧传递信息
* 参数：int l_fds[2]:左侧管道存储文件描述符的数组
*       int r_fds[2]:右侧管道存储文件描述符的数组
*       int lfn:左侧的第一个数
* 返回值：无
*/
void trans(int l_fds[2], int r_fds[2], int lfn)
{
  //将左侧中不能被lfn整除的数传入右侧
  int data;
  while (read(l_fds[0], &data, sizeof(int)) == sizeof(int)) {
    if (data % lfn != 0) {
      if (write(r_fds[1], &data, sizeof(int)) != sizeof(int)) {
        //写失败，打印错误信息，异常退出
        fprintf(2, "primes:write() filed\n");
        exit(1);
      }
    }
  }
}

/*
* 功能：获取素数
* 参数：int l_fds[2]:左侧存储文件描述符的数组
* 返回值：无
*/
void primes(int l_fds[2])
{
  //关闭左侧对管道的写
  close(l_fds[1]);

  //获取左侧的第一个数
  int lfn;
  if (get_left_first_num(l_fds, &lfn) != -1) {
    fprintf(1, "prime %d\n", lfn);

    //创建管道
    int r_fds[2];
    if (pipe(r_fds) < 0) {
      //创建管道失败，打印错误信息，异常退出
      fprintf(2, "primes:pipe() filed\n");
      exit(1);
    }
    //传递数据
    trans(l_fds, r_fds, lfn);

    // 关闭对左侧的读以及对右侧的写
    close(l_fds[0]);
    close(r_fds[1]);

    //创建子进程
    int pid = fork();
    if (pid < 0) {
      //创建子进程失败，打印错误信息并关闭文件描述符，异常退出
      fprintf(2, "primes:fork() filed\n");
      close(r_fds[0]);
      exit(1);
    }

    if (pid > 0) {
      //父进程
      close(r_fds[0]);
      wait(0);
    }
    else {
      //子进程
      primes(r_fds);
    }
  }

  exit(0);
}
