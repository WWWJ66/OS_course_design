#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char* argv[])
{
  int sleep_time;

  //����������������ֱ�ӱ����쳣�˳�
  if (argc != 2) {
    fprintf(2, "usage: sleep pattern\n");
    exit(1);
  }

  //����ͣʱ����ַ���ת��Ϊ����
  sleep_time = atoi(argv[1]);

  //ʹ��ϵͳ����sleep
  sleep(sleep_time);

  //�����˳�
  exit(0);
}
