# Sobre as flags:

# std=c99
# ta cheio de for com declaracao dentro
# vide: boa pratica de deixar variaveis perto do uso

# m32
# os objs passados foram compilados em 32bit
# a vm e 32bit
#

# IMPORTANTE:
# PODE DAR PROBLEMA COM ESSA FLAG!!!
# As maquinas dos labs (pelo menos os de estudo) NAO possuem suporte
# para essa flag (nao tem a libc6-dev-i386)
# (E eu como aluno, nao tinha permissao pra instalar)

CFLAGS=-std=c99 -m32


CC=gcc $(CFLAGS)
LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/

all: bin list boot mft t2fs lib-t2fs

bin:
	mkdir -p $(BIN_DIR)/

lib-t2fs:
	ar crs $(LIB_DIR)/libt2fs.a $(BIN_DIR)/t2fs.o $(BIN_DIR)/boot.o $(BIN_DIR)/mft.o $(BIN_DIR)/list.o $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o

t2fs:
	$(CC) -c $(SRC_DIR)/t2fs.c -I$(INC_DIR) -Wall -m32
	mv t2fs.o $(BIN_DIR)

boot:
	$(CC) -c $(SRC_DIR)/boot.c -I$(INC_DIR) -Wall -m32
	mv boot.o $(BIN_DIR)

mft:
	$(CC) -c $(SRC_DIR)/mft.c -I$(INC_DIR) -Wall -m32
	mv mft.o $(BIN_DIR)

list:
	$(CC) -c $(SRC_DIR)/list.c -I$(INC_DIR) -Wall -m32
	mv list.o $(BIN_DIR)

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~