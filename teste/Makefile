# Sobre as flags:

# std=gnu99
# usa strdup pra nao estragar a string passada por parametro
# em algumas funcoes
#
# e ta cheio de for com declaracao dentro tambem
# (vide: boa pratica de deixar variaveis perto do uso)

# m32
# os objs passados foram compilados em 32bit
# a vm e 32bit
#

# IMPORTANTE:
# PODE DAR PROBLEMA COM ESSA FLAG!!!
# As maquinas dos labs (pelo menos os de estudo) NAO possuem suporte
# para essa flag (nao tem a libc6-dev-i386)
# (E eu como aluno, nao tinha permissao pra instalar)

CFLAGS=-std=gnu99 -m32

CC=gcc $(CFLAGS)
LIB_DIR=../lib
INC_DIR=../include
BIN_DIR=./
SRC_DIR=./

all: debug shell writing dirception bad_developer_directories bad_developer_files

# forcando que sempre compile, por razoes de: meu computador implicou em nao
# compilar quando existe mudancas no fonte
.PHONY: debug shell writing dirception bad_developer_directories bad_developer_files

debug:
	$(CC) $(SRC_DIR)/debug.c -o debug -I$(INC_DIR) -L$(LIB_DIR) -lt2fs -Wall -m32

shell:
	$(CC) $(SRC_DIR)/shell.c -o shell -I$(INC_DIR) -L$(LIB_DIR) -lt2fs -Wall -m32

writing:
	$(CC) $(SRC_DIR)/writing.c -o writing -I$(INC_DIR) -L$(LIB_DIR) -lt2fs -Wall -m32

dirception:
	$(CC) $(SRC_DIR)/dirception.c -o dirception -I$(INC_DIR) -L$(LIB_DIR) -lt2fs -Wall -m32

bad_developer_directories:
	$(CC) $(SRC_DIR)/bad_developer_directories.c -o bad_developer_directories -I$(INC_DIR) -L$(LIB_DIR) -lt2fs -Wall -m32

bad_developer_files:
	$(CC) $(SRC_DIR)/bad_developer_files.c -o bad_developer_files -I$(INC_DIR) -L$(LIB_DIR) -lt2fs -Wall -m32

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~