#include <stdio.h>
#include <stdlib.h>

#include "t2fs.h"

/*
*   Deseja-se testar a criacao de multiplas pastas dado um caminho 
* (que pode ser tanto relativo quanto absoluto).
*   Na implementacao atual, a funcao mkdir cria todas as pastas nao existentes
* no caminho dado (similar ao comando mkdir -p do unix). 
*   Apos a criacao dos diretorios, realizamos a impressao das entradas de cada 
* pasta criada.
*
* Author: Arthur Lenz
*/
int main()
{
  char pathname[] = "/f1/f2/f3/";

  // cria o diretorio
  if(mkdir2(pathname) != 0)
    printf("error when creating %s\n", pathname);
  else
    printf("%s succesfully created!\n", pathname);

  int handle;
  DIRENT2 entry;

  // volta para o root
  char root[] = "/";
  handle = opendir2(root);
  if(handle < 0)
    printf("error while trying to open %s\n", root);
  else {
    printf("cd: %s\n", root);
    while(readdir2(handle, &entry) == 0) {
      printf("*\t %s\n", entry.name);
    }
    closedir2(handle);
  }


  // entra em f1
  char f1[] = "f1";
  handle = opendir2(f1);
  if(handle < 0)
    printf("error while trying to open %s\n", f1);
  else {
    printf("cd: %s\n", f1);
    while(readdir2(handle, &entry) == 0) {
      printf("*\t %s\n", entry.name);
    }
    closedir2(handle);
  }


  // entra em f2
  char f2[] = "f2";
  handle = opendir2(f2);
  if(handle < 0)
    printf("error while trying to open %s\n", f2);
  else {
    printf("cd: %s\n", f2);
    while(readdir2(handle, &entry) == 0) {
      printf("*\t %s\n", entry.name);
    }
    closedir2(handle);
  }


  // entra em f2
  char f3[] = "f3";
  handle = opendir2(f3);
  if(handle < 0)
    printf("error while trying to open %s\n", f3);
  else {
    printf("cd: %s\n", f3);
    while(readdir2(handle, &entry) == 0) {
      printf("*\t %s\n", entry.name);
    }
    closedir2(handle);
  }


  // sobe dois niveis
  char cdupup[] = "../../";
  handle = opendir2(cdupup);
  if(handle < 0)
    printf("error while trying to open %s\n", cdupup);
  else {
    printf("cd: %s\n", cdupup);
    while(readdir2(handle, &entry) == 0) {
      printf("*\t %s\n", entry.name);
    }
    closedir2(handle);
  }

	return 0;
}