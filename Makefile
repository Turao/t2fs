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
LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/

all: bin boot mft t2fs lib-t2fs

bin:
	mkdir -p $(BIN_DIR)/

lib-t2fs:
	ar crs $(LIB_DIR)/libt2fs.a $(BIN_DIR)/t2fs.o $(BIN_DIR)/boot.o $(BIN_DIR)/mft.o $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o

t2fs:
	$(CC) -c $(SRC_DIR)/t2fs.c -I$(INC_DIR) -Wall -m32
	mv t2fs.o $(BIN_DIR)

boot:
	$(CC) -c $(SRC_DIR)/boot.c -I$(INC_DIR) -Wall -m32
	mv boot.o $(BIN_DIR)

mft:
	$(CC) -c $(SRC_DIR)/mft.c -I$(INC_DIR) -Wall -m32
	mv mft.o $(BIN_DIR)

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~