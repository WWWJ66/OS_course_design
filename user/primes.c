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

  int fds[2];//�洢�ļ�������������

  //�����ܵ�
  if (pipe(fds) < 0) {
    //�����ܵ�ʧ�ܣ���ӡ������Ϣ���쳣�˳�
    fprintf(2, "primes:pipe() filed\n");
    exit(1);
  }

  //д���ʼ����
  for (int i = 2; i <= num; i++) {
    if (write(fds[1], &i, sizeof(int)) != sizeof(int)) {
      //дʧ�ܣ���ӡ������Ϣ���쳣�˳�
      fprintf(2, "primes:write() filed\n");
      exit(1);
    }
  }

  //�����ӽ���
  int pid = fork();
  if (pid < 0) {
    //�����ӽ���ʧ�ܣ���ӡ������Ϣ���ر��ļ����������쳣�˳�
    fprintf(2, "primes:fork() filed\n");
    close(fds[0]);
    close(fds[1]);
    exit(1);
  }

  if (pid > 0) {
    //������
    close(fds[0]);
    close(fds[1]);
    wait(0);
  }
  else {
    //�ӽ���
    primes(fds);
  }

  //�����˳�
  exit(0);
}

/*
* ���ܣ���ȡ���ĵ�һ������dst��
* ������int l_fds[2]:���洢�ļ�������������
*       int* dst:Ŀ�����
* ����ֵ�������໹����������0�����򷵻�-1
*/
int get_left_first_num(int l_fds[2], int* dst)
{
  if (read(l_fds[0], dst, sizeof(int)) == sizeof(int)) {
    return 0;
  }

  return -1;
}

/*
* ���ܣ����Ҳഫ����Ϣ
* ������int l_fds[2]:���ܵ��洢�ļ�������������
*       int r_fds[2]:�Ҳ�ܵ��洢�ļ�������������
*       int lfn:���ĵ�һ����
* ����ֵ����
*/
void trans(int l_fds[2], int r_fds[2], int lfn)
{
  //������в��ܱ�lfn�������������Ҳ�
  int data;
  while (read(l_fds[0], &data, sizeof(int)) == sizeof(int)) {
    if (data % lfn != 0) {
      if (write(r_fds[1], &data, sizeof(int)) != sizeof(int)) {
        //дʧ�ܣ���ӡ������Ϣ���쳣�˳�
        fprintf(2, "primes:write() filed\n");
        exit(1);
      }
    }
  }
}

/*
* ���ܣ���ȡ����
* ������int l_fds[2]:���洢�ļ�������������
* ����ֵ����
*/
void primes(int l_fds[2])
{
  //�ر����Թܵ���д
  close(l_fds[1]);

  //��ȡ���ĵ�һ����
  int lfn;
  if (get_left_first_num(l_fds, &lfn) != -1) {
    fprintf(1, "prime %d\n", lfn);

    //�����ܵ�
    int r_fds[2];
    if (pipe(r_fds) < 0) {
      //�����ܵ�ʧ�ܣ���ӡ������Ϣ���쳣�˳�
      fprintf(2, "primes:pipe() filed\n");
      exit(1);
    }
    //��������
    trans(l_fds, r_fds, lfn);

    // �رն����Ķ��Լ����Ҳ��д
    close(l_fds[0]);
    close(r_fds[1]);

    //�����ӽ���
    int pid = fork();
    if (pid < 0) {
      //�����ӽ���ʧ�ܣ���ӡ������Ϣ���ر��ļ����������쳣�˳�
      fprintf(2, "primes:fork() filed\n");
      close(r_fds[0]);
      exit(1);
    }

    if (pid > 0) {
      //������
      close(r_fds[0]);
      wait(0);
    }
    else {
      //�ӽ���
      primes(r_fds);
    }
  }

  exit(0);
}
