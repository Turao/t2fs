#include <stdio.h>
#include <stdlib.h>

#include "t2fs.h"
#include "utils.h"

int main()
{
	char filename[] = "./definitely_not_a_file";
	// tenta abrir o arquivo (que nao existe)
	if(open2(filename) < 0)
		printf("error while trying to open %s\n", filename);
	else
		printf("%s succesfully opened!\n", filename);

	// tenta fechar o arquivo (que nao existe)
	if(close2(21) < 0)
		printf("error while closing %s\n", filename);
	else
		printf("%s succesfully closed!\n", filename);

	// cria o arquivo
	int file_handle = create2(filename);
	if(file_handle < 0)
		printf("error when creating %s\n", filename);
	else
		printf("%s succesfully created!\n", filename);

	// cria o arquivo (nao consegue, pois este ja existe)
	file_handle = create2(filename);
	if(file_handle < 0)
		printf("error when creating %s\n", filename);
	else
		printf("%s succesfully created!\n", filename);

	// tenta posicionar o arquivo numa posição invalida
	int stream_pointer = seek2(file_handle, -1);
	if(stream_pointer < 0)
		printf("error when setting stream pointer %s\n", filename);
	else
		printf("%s stream pointer now in %d !\n", filename, stream_pointer);

	// usa um handle invalido para o seek
	stream_pointer = seek2(21, 0);
	if(stream_pointer < 0)
		printf("error when setting stream pointer (of non-existent file)\n");
	else
		printf("non-existent file stream pointer now in %d !\n", stream_pointer);

	return SUCCESS;
}