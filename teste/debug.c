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
  create2("/file3");


  char folder1path[] = "/";
  int folder1;
  DIRENT2 entryf1;
  
  folder1 = opendir2(folder1path);
  if(folder1 >= 0) {
    printf("%s:\n", folder1path);
    while(readdir2(folder1, &entryf1) == 0) {
      printf("*\t %s\n", entryf1.name);
    }
    closedir2(folder1);
    printf("\n");
  }
  else printf("couldnt open %s\n", folder1path);


  return SUCCESS;
}