#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define DEBUG true

#include "t2fs.h"
#include "boot.h"
#include "mft.h"
#include "list.h"

#include "apidisk.h"
#include "bitmap2.h"
#include "utils.h"

#define DEVELOPERS \
"Arthur Lenz 218316\n" \
"Guillermo Amaya 217434\n" \
"Rafael Allegretti 215020"

char devs[] = DEVELOPERS;


bool disk_info_initialized = false;
#define INIT_DISK_INFO() {init_t2fs_bootBlock(); disk_info_initialized = true;}
bool mft_info_initialized = false;
#define INIT_MFT_INFO() {init_mft_info(); mft_info_initialized = true;}

char cwdPath[1048] = "/";

FILE2 opened[20]; //opened files

typedef struct directory {
  t2fs_record record;
  List entries;
  int current_entry;
  bool beingUsed;
} directory;

directory opened_dir[20] = {0,0,0,false};

int _logicalBlock_sector(int logicalBlockNumber) {
  return logicalBlockNumber * _bootBlock.blockSize;
}

void _descriptorEntries(descriptor d, List *entries)
{
  int sector;
  unsigned char buffer[256];
  t2fs_record record[4]; //records per sector

  List valid_tuples;
  list_new(&valid_tuples, sizeof(t2fs_4tupla), free);
  descriptor_tuples(d, &valid_tuples);

  for(int i=0; i<list_size(&valid_tuples); i++) {
    t2fs_4tupla tuple; //iterator
    list_at(&valid_tuples, i, &tuple);

    sector = _logicalBlock_sector(tuple.logicalBlockNumber);
    // printf("\n tuple %d: \n", i);
    // printf("\t logical block %d", tuple.logicalBlockNumber);
    // printf("\t [sector %d] \n", sector);
    // printf("\t virtual block %d \n", tuple.virtualBlockNumber);
    // printf("\t # contiguous blocks %d \n", tuple.numberOfContiguosBlocks);
    // printf("\n");

    read_sector(sector, buffer);
    //for each sector, we have 4 records
    // (256 bytes [sector size] / 64 bytes [record size]) = 4
    memcpy(&record, buffer, sizeof(t2fs_record)*4);

    for(int i=0; i<4; i++) {
      if(record[i].TypeVal != TYPEVAL_INVALIDO)
        list_push_back(entries, &record[i]);
    }
  }
}


bool print_entry(void *e) {
  t2fs_record *record = (t2fs_record*) e;
  
  printf("name: %s \n", record->name);
  printf("typeval: %x \n", record->TypeVal);
  printf("block file size: %d \n", record->blocksFileSize);
  printf("bytes file size: %d \n", record->bytesFileSize);
  printf("mft number: %d \n", record->MFTNumber);
  printf("\n");

  return true;
}

bool print_file_data(void *data) {
  t2fs_4tupla *tuple = (t2fs_4tupla*) data;
  if(tuple == NULL) return false;

  // reads the sector, based on the LBN given by the tuple
  unsigned char buffer[256];
  read_sector(_logicalBlock_sector(tuple->logicalBlockNumber), buffer);
  
  // then, just print its content as a string
  printf("%s\n", buffer);
  return true;
}

void read_file(t2fs_record file_r) {
  //looks for the descriptor of the file, in the mft
  descriptor file_d;
  get_descriptor(file_r.MFTNumber, &file_d);

  //get all of its valid (mapped) tuples
  List valid_tuples;
  list_new(&valid_tuples, sizeof(t2fs_4tupla), free);
  descriptor_tuples(file_d, &valid_tuples);

  //for each valid tuple, read the data
  list_for_each(&valid_tuples, print_file_data);
}


bool compare_by_name(void* entry, void *name) { //not tested
  t2fs_record *record = (t2fs_record*) entry;
  if(strcmp(name, record->name) == 0) {
    return true;
  }
  else return false;
}

bool exists(char* name, List *entries, t2fs_record *record) { //not tested
  t2fs_record *found = list_find(entries, compare_by_name, name);
  if(found) {
    memcpy(record, found, sizeof(t2fs_record));
    return true;
  }
  else return false;
}


int get_valid_dir_handle() {
  for(int i=0; i<20; i++) {
    if(!opened_dir[i].beingUsed) return i;
  }
  return ERROR;
}


void test_open_root() {
  DEBUG_PRINT("Reading root dir \n");
  List root_entries;
  list_new(&root_entries, sizeof(t2fs_record), free);
  _descriptorEntries(_root_d, &root_entries);
  list_for_each(&root_entries, print_entry);


  // reads all files within the root dir
  t2fs_record file; //iterator
  for(int i=0; i<list_size(&root_entries); i++) {
    list_at(&root_entries, i, &file);
    read_file(file);
  }

  t2fs_record record_found;
  if(exists("file3", &root_entries, &record_found)) {
    printf("found %s\n", record_found.name);
  }

}



int identify2 (char *name, int size)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();


  test_open_root();


  if(size < sizeof(devs)) return ERROR;
  else
  {
    strncpy(name, devs, sizeof(devs));
    return SUCCESS;
  }
}



FILE2 create2 (char *filename)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();
  //to-do
  return ERROR;
}



int delete2 (char *filename)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();
  //to-do
  return ERROR;
}

int cd(char* path, t2fs_record *record)
{
  printf("path %s\n", path);
  
  List entries; // lista de entradas
  list_new(&entries, sizeof(t2fs_record), free);

  descriptor current_descriptor;
  if(path[0] == '/') //inicia pelo descritor do diretorio root
    current_descriptor = _root_d;
  else
    current_descriptor = cwdDescriptor;

  // pega entradas do diretorio inicial (root ou atual)
  _descriptorEntries(current_descriptor, &entries);

  // vai separando pela '/'
  char* next = strtok(path, "/");
  descriptor parent;
  char parentPath[1024];
  t2fs_record record_found;
  while(next) {
    printf("CWD: %s\n", cwdPath);

    if(strcmp(next, ".") == 0) {
      printf("cd .\n");
      next = strtok(NULL, "/"); // ignore current path (.)
      continue;
    }

    if(strcmp(next, "..") == 0) {
      // olha se esta atualmente no root
      // (nao pode subir se esta no root)
      if(strcmp(cwdPath, "/") == 0) return ERROR;
      else {
        current_descriptor = parent;
        strncpy(cwdPath, parentPath, sizeof(parentPath));
        printf("cd ..\n");
        next = strtok(NULL, "/");
        continue;
      }
    }

    
    // se a entrada pesquisada existe na lista de entradas do diretorio,
    // teremos o record do diretorio/arquivo desejado em record_found
    if(exists(next, &entries, &record_found)) {
      // printf("%s found\n", record_found.name);
      printf("cd %s\n", next);
      
      // atribui o descritor atual como pai
      parent = current_descriptor;
      strncpy(parentPath, cwdPath, sizeof(parentPath));

      // atualiza o current working directory
      strcat(cwdPath, next);
      strcat(cwdPath, "/");

      // e ja busca o descritor do diretorio/arquivo encontrado
      get_descriptor(record_found.MFTNumber, &current_descriptor);

      // pega a proxima parte do filepath
      next = strtok(NULL, "/");
    }
    else {
      printf("%s not found\n", next);
      return ERROR;
    }
  }

  printf("CWD: %s\n", cwdPath);
  memcpy(record, &record_found, sizeof(t2fs_record));

  return SUCCESS;
}


FILE2 open2 (char *filename)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();
  //to-do :: finish tests
  t2fs_record file_r;
  int status = cd(filename, &file_r);
  return status;
}



int close2 (FILE2 handle)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();
  //to-do
  return ERROR;
}



int read2 (FILE2 handle, char *buffer, int size)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();
  //to-do
  return ERROR;
}



int write2 (FILE2 handle, char *buffer, int size)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();
  //to-do
  return ERROR;
}



int truncate2 (FILE2 handle)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();
  //to-do
  return ERROR;
}



int seek2 (FILE2 handle, DWORD offset)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();
  //to-do
  return ERROR;
}



int mkdir2 (char *pathname)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();
  //to-do
  return ERROR;
}



int rmdir2 (char *pathname)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();
  //to-do
  return ERROR;

}



DIR2 opendir2 (char *pathname)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  // tenta acessar o caminho especificado
  // se existir, pega o record do diretorio desejado
  directory dir;
  if(cd(pathname, &dir.record) == SUCCESS) {
    // pega o descritor do diretorio, para buscar
    // as entradas
    descriptor dir_descriptor;
    if(dir.record.MFTNumber == 0) // o root tem o record bugado
      get_descriptor(1, &dir_descriptor);
    else 
      get_descriptor(dir.record.MFTNumber, &dir_descriptor);

    // coloca as entradas validas do diretorio
    // em uma lista
    list_new(&dir.entries, sizeof(t2fs_record), free);
    _descriptorEntries(dir_descriptor, &dir.entries);
    // posiciona o ponteiro de entradas (current entry)
    // na primeira posição valida do diretório
    dir.current_entry = 0;

    // coloca dir na proxima posicao valida do array de diretorios abertos
    int position = get_valid_dir_handle();

    if(position >= 0 && position < 20) {
      dir.beingUsed = true;
      opened_dir[position] = dir;
      return position;
    }
    else
      // se nao existe nenhuma posicao valida
      // (ja tem 20 diretorios abertos)
      return ERROR;
  }


  return ERROR;
}



int readdir2 (DIR2 handle, DIRENT2 *dentry)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();
  //to-do

  // posicao invalida
  // [retorna -2 : nao pode usar o -1 do ERROR]
  // handle fora do range valido
  if(handle < 0 || handle >= 20) return -2;
  // o diretorio nao esta aberto
  if(!opened_dir[handle].beingUsed) return -2;
  
  directory *dir = &opened_dir[handle];
  if(dir->current_entry < list_size(&dir->entries)) {
    t2fs_record entry;
    list_at(&dir->entries, dir->current_entry, &entry);

    strcpy(dentry->name, entry.name);
    dentry->fileType = entry.TypeVal;
    dentry->fileSize = entry.bytesFileSize;

    // avanca uma entrada
    dir->current_entry++;

    return SUCCESS;
  }

  return -1; // END OF FILE
}



int closedir2 (DIR2 handle)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();
  //to-do

  // posicao invalida
  // [retorna -2 : nao pode usar o -1 do ERROR]
  // handle fora do range valido
  if(handle < 0 || handle >= 20) return -2;
  // o diretorio nao esta aberto
  if(!opened_dir[handle].beingUsed) return -2;


  // se o handle passado for valido
  // so "liberamos" a vaga, setando beingUsed
  // para falso
  opened_dir[handle].beingUsed = false;


  return SUCCESS;
}