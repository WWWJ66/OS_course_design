#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAX_SIZE 512

int
main(int argc, char* argv[])
{
  //����С��2����ֱ�ӱ����쳣�˳�
  if (argc < 2) {
    fprintf(2, "usage: xargs command [arg ...]\n");
    exit(1);
  }

  //����ӽ���exec�Ĳ���
  char* x_argv[MAXARG];
  for (int i = 1; i < argc; i++) {
    x_argv[i - 1] = argv[i];
  }

  //�ӹܵ��ж�ȡ����
  char ch;
  char temp[MAX_SIZE];
  int i = 0;
  while (read(0, &ch, 1) > 0) {
    //δ����β
    if (ch != '\n') {
      temp[i++] = ch;
    }
    else {
      temp[i] = 0;
      i = 0;

      int pid = fork();
      if (pid < 0) {
        //ֱ�ӱ����쳣�˳�
        fprintf(2, "xargs:fork() failed\n");
        exit(1);
      }

      if (pid > 0) {
        //������
        wait(0);
      }
      else {
        //�ӽ���
        x_argv[argc - 1] = temp;
        if (exec(argv[1], x_argv) < 0) {
          //ֱ�ӱ����쳣�˳�
          fprintf(2, "xargs:exec() failed\n");
          exit(1);
        }
      }
    }
  }

  //�����˳�
  exit(0);
}
