#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


void
find(char* dir, char* fname);

int
main(int argc, char* argv[])
{
  //参数不等于三个，直接报错并异常退出
  if (argc != 3) {
    fprintf(2, "usage: find directory filename \n");
    exit(1);
  }

  //查找指定文件
  find(argv[1], argv[2]);

  //正常退出
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
    //无法打开文件
    fprintf(2, "find: cannot open %s\n", dir);
    return;
  }

  if (fstat(fd, &st) < 0) {
    //无法获取文件状态信息
    fprintf(2, "find: cannot stat%s\n", dir);
    return;
  }

  //参数不正确，报错并异常退出
  if (st.type != T_DIR) {
    fprintf(2, "usage: find directory filename \n");
    return;
  }

  //目录过长
  if (strlen(dir) + 1 + DIRSIZ + 1 > sizeof buf) {
    fprintf(2, "find: directory too long\n");
    return;
  }

  //将目录存储在buf中，在buf末尾添加'/'，并将指针指向buf末尾
  strcpy(buf, dir);
  p = buf + strlen(buf);
  *p++ = '/';

  //循环读取所有的文件条目
  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    //空条目
    if (de.inum == 0) {
      continue;
    }

    //添加路径名称
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    if (stat(buf, &st) < 0) {
      fprintf(2, "find: cannot stat %s\n", buf);
      continue;
    }

    //"."和".."中不递归
    if (st.type == T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..") != 0) {
      find(buf, fname);
    }
    else if (strcmp(fname, p) == 0) {
      fprintf(1, "%s\n", buf);
    }
  }

  close(fd);
}
