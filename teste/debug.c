#include <stdio.h>
#include <stdlib.h>

#include "t2fs.h"
#include "utils.h"

int main()
{
  char devs[256];

  if(identify2(devs, 256) == ERROR) return ERROR;
  printf("devs: %s\n", devs);

  printf("\n");

  // opendir OK (para root)
  char dirpath[] = "/";
  int dir = opendir2(dirpath);
  printf("dir %d\n", dir);

  // readdir OK (para root)
  DIRENT2 entry;
  while(readdir2(dir, &entry) == 0) {
    printf("entry %s\n", entry.name);
  }
  closedir2(dir);


  char folder[] = "./folder1/folder2/";
  mkdir2(folder);


  // char folder2[] = "./folder2/folder3";
  // mkdir2(folder2);

  return SUCCESS;
}