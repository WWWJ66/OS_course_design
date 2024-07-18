#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


void
find(char* dir, char* fname);

int
main(int argc, char* argv[])
{
  //����������������ֱ�ӱ����쳣�˳�
  if (argc != 3) {
    fprintf(2, "usage: find directory filename \n");
    exit(1);
  }

  //����ָ���ļ�
  find(argv[1], argv[2]);

  //�����˳�
  exit(0);
}

void find(char* dir, char* fname)
{
  char buf[512], * p;
  int fd;
  struct dirent de;
  struct stat st;

  fd = open(dir, 0);
  if (fd < 0) {
    //�޷����ļ�
    fprintf(2, "find: cannot open %s\n", dir);
    return;
  }

  if (fstat(fd, &st) < 0) {
    //�޷���ȡ�ļ�״̬��Ϣ
    fprintf(2, "find: cannot stat%s\n", dir);
    return;
  }

  //��������ȷ�������쳣�˳�
  if (st.type != T_DIR) {
    fprintf(2, "usage: find directory filename \n");
    return;
  }

  //Ŀ¼����
  if (strlen(dir) + 1 + DIRSIZ + 1 > sizeof buf) {
    fprintf(2, "find: directory too long\n");
    return;
  }

  //��Ŀ¼�洢��buf�У���bufĩβ���'/'������ָ��ָ��bufĩβ
  strcpy(buf, dir);
  p = buf + strlen(buf);
  *p++ = '/';

  //ѭ����ȡ���е��ļ���Ŀ
  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    //����Ŀ
    if (de.inum == 0) {
      continue;
    }

    //���·������
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    if (stat(buf, &st) < 0) {
      fprintf(2, "find: cannot stat %s\n", buf);
      continue;
    }

    //"."��".."�в��ݹ�
    if (st.type == T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..") != 0) {
      find(buf, fname);
    }
    else if (strcmp(fname, p) == 0) {
      fprintf(1, "%s\n", buf);
    }
  }

  close(fd);
}
