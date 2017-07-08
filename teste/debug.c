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
  char filepath[] = "./file1/../../file1";

  char dirpath[] = ".";
  int dir = opendir2(dirpath);

  DIRENT2 entry;
  while(readdir2(dir, &entry) == 0) {
  	printf("entry %s\n", entry.name);
    // closedir2(dir);
  }

  return SUCCESS;
}