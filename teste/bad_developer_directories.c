#include <stdio.h>
#include <stdlib.h>

#include "t2fs.h"

int main()
{
	char pathname[] = "./definitely_not_a_folder";

  // tenta abrir o diretorio (que nao existe)
  int dir_handle = opendir2(pathname);
  if(dir_handle < 0)
    printf("error while trying to open %s\n", pathname);
  else
    printf("%s succesfully opened!\n", pathname);

  // tenta fechar o diretorio (que nao existe)
  if(closedir2(21) < 0)
    printf("error while closing %s\n", pathname);
  else
    printf("%s succesfully closed!\n", pathname);

  // tenta remover um diretorio que nao existe
  if(rmdir2(pathname) != 0)
    printf("error when deleting %s\n", pathname);
  else
    printf("%s succesfully deleted!\n", pathname);

  // cria o diretorio
  if(mkdir2(pathname) != 0)
    printf("error when creating %s\n", pathname);
  else
    printf("%s succesfully created!\n", pathname);

  // ja que o cwd passa a apontar para o diretorio criado
  char parent[] = "..";
  opendir2(parent); // sobe pro diretorio pai

  // cria o diretorio (nao consegue, pois este ja existe)
  if(mkdir2(pathname) != 0)
    printf("error when creating %s\n", pathname);
  else
    printf("%s succesfully created!\n", pathname);

  // tenta remover o diretorio root
  char root[] = "/";
  if(rmdir2(root) != 0)
    printf("error when deleting %s\n", root);
  else
    printf("%s succesfully deleting!\n", root);

  
  return 0;
}