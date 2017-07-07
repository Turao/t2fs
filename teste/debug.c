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
  open2(filepath);

  return SUCCESS;
}