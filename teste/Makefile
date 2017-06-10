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

all: debug

.PHONY: debug

debug:
	$(CC) $(SRC_DIR)/debug.c -o debug -I$(INC_DIR) -L$(LIB_DIR) -lt2fs -Wall -m32

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~