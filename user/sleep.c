#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char* argv[])
{
  int sleep_time;

  //参数不等于两个，直接报错并异常退出
  if (argc != 2) {
    fprintf(2, "usage: sleep pattern\n");
    exit(1);
  }

  //将暂停时间从字符串转化为整型
  sleep_time = atoi(argv[1]);

  //使用系统调用sleep
  sleep(sleep_time);

  //正常退出
  exit(0);
}
