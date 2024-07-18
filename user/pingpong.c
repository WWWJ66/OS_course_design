#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(int argc, int* argv[])
{
  char data = 'B';    //Ҫ���������
  char buf[8];        //���ݻ�����

  int fds_p2c[2];     //�洢�����̵��ӽ��̵��ļ�������������
  int fds_c2p[2];     //�洢�ӽ��̵������̵��ļ�������������

  //�����ܵ�
  if ((pipe(fds_p2c) < 0) || (pipe(fds_c2p) < 0)) {
    //�����ܵ�ʧ�ܣ���ӡ������Ϣ���쳣�˳�
    fprintf(2, "pingpong:pipe() filed\n");
    exit(1);
  }

  //�����ӽ���
  int pid = fork();
  if (pid < 0) {
    //�����ӽ���ʧ�ܣ���ӡ������Ϣ���ر��ļ����������쳣�˳�
    fprintf(2, "pingpong:fork() filed\n");
    close(fds_p2c[0]);
    close(fds_p2c[1]);
    close(fds_c2p[0]);
    close(fds_c2p[1]);
    exit(1);
  }

  if (pid > 0) {
    //������
    close(fds_p2c[0]);
    close(fds_c2p[1]);

    if (write(fds_p2c[1], &data, 1) != 1) {
      //дʧ��
      fprintf(2, "pingpong:write() filed\n");
      exit(1);
    }

    if (read(fds_c2p[0], &buf, 1) != 1) {
      //��ʧ��
      fprintf(2, "pingpong:read() filed\n");
      exit(1);
    }
    
    fprintf(1, "%d: received pong\n", getpid());

    wait(0);
  }
  else if (pid == 0) {
    //�ӽ���
    close(fds_p2c[1]);
    close(fds_c2p[0]);

    if (read(fds_p2c[0], &buf, 1) != 1) {
      //��ʧ��
      fprintf(2, "pingpong:read() filed\n");
      exit(1);
    }

    fprintf(1, "%d: received ping\n", getpid());

    if (write(fds_c2p[1], &buf, 1) != 1) {
      //дʧ��
      fprintf(2, "pingpong:write() filed\n");
      exit(1);
    }
  }

  exit(0);
}
