#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "t2fs.h"


/*
*   Deseja-se criar um arquivo, escrever dados, e entao realizar
*  o fechamento do mesmo sem que ocorra perda dos dados.
*   Apos o fechamento, realizamos novamente uma abertura de arquivo, 
*  seguida da leitura dos dados.
*   Caso nao ocorra uma inconsistencia (esperado), sera impresso o
*  conteudo escrito no inicio do teste pela funcao write2.
*
* Author: Arthur Lenz
*/
int main()
{
  char filename[] = "/sisopI";

  // cria o arquivo
  int handle = create2(filename);
  if(handle < 0)
    printf("error when creating %s\n", filename);
  else
    printf("%s succesfully created!\n", filename);

  // escreve dados no arquivo
  char content[] = "sisopi file content is here!";
  int bytes_written = write2(handle, content, strlen(content));
  if(bytes_written < 0)
    printf("unable to write data. \n");
  else
    printf("data succesfully written!\n");


  // com a escrita, temos o stream pointer
  // apontando para o final do arquivo
  // logo, realizamos um seek ao inicio, para exibir o conteudo
  if(seek2(handle, 0) < 0)
    printf("unable to seek to position 0. \n");
  else
    printf("seek succesful!\n");

  // le o arquivo e joga os dados para
  // um buffer
  char buffer[512] = "";
  int bytes_read = read2(handle, buffer, sizeof(buffer));

  // imprime o conteudo do arquivo lido
  if(bytes_read < 0)
    printf("unable to read %s. \n", filename);
  else
    printf("%s: %s \n", filename, buffer);


  // agora testaremos a consistencia dos dados
  printf("\n now we'll check the consistency of the disk data \n\n");

  // fecha o arquivo
  if(close2(handle) < 0)
    printf("error when closing %s\n", filename);
  else
    printf("%s succesfully closed!\n", filename);


  // abre novamente o arquivo
  handle = open2(filename);
  if(handle < 0)
    printf("error when opening %s\n", filename);
  else
    printf("%s succesfully opened!\n", filename);

  // le novamente o disco
  char anotherBuffer[512] = ""; // outro buffer, comprovando que leu corretamente
  bytes_read = read2(handle, anotherBuffer, sizeof(anotherBuffer));

  // imprime o conteudo do arquivo lido
  if(bytes_read < 0)
    printf("unable to read %s. \n", filename);
  else
    printf("%s: %s\n", filename, anotherBuffer);


	return 0;
}