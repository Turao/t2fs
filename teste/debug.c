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


  char folder[] = "./folder1/folder2/";
  mkdir2(folder);

  // char root[] = "/";
  // int dir2 = opendir2(root);
  // printf("%s:\n", root);
  // // readdir OK (para root)
  // DIRENT2 entry2;
  // while(readdir2(dir2, &entry2) == 0) {
  //   printf("\t %s\n", entry2.name);
  // }
  // closedir2(dir2);
  // printf("\n");


  char folder1path[] = "/folder1/";
  printf("%s:\n", folder1path);
  int folder1 = opendir2(folder1path);
  // readdir OK (para root)
  DIRENT2 entryf1;
  while(readdir2(folder1, &entryf1) == 0) {
    printf("\t %s\n", entryf1.name);
  }
  closedir2(folder1);
  printf("\n");


  // // opendir OK (para root)
  // char folder2path[] = "/folder1/../folder1/./folder2";
  // printf("%s:\n", folder2path);
  // int folder2 = opendir2(folder2path);
  // // readdir OK (para root)
  // DIRENT2 entryf2;
  // while(readdir2(folder2, &entryf2) == 0) {
  //   printf("\t %s\n", entryf2.name);
  // }
  // closedir2(folder2);
  // printf("\n");


  rmdir2(folder1path);




  return SUCCESS;
}