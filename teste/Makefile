#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#
# 

CC=gcc
LIB_DIR=../lib
INC_DIR=../include
BIN_DIR=./
SRC_DIR=./

all: debug shell writing dirception bad_developer_directories bad_developer_files

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