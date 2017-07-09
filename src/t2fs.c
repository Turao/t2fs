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
  int current_entry;
  bool beingUsed;
} directory;

directory opened_dir[20] = {0,0,0,false};



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
  read_sector(logicalBlock_sector(tuple->logicalBlockNumber), buffer);
  
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



bool compare_by_name(void* entry, void *name) {
  // funcao comparadora
  // recebe um ponteiro para o objeto de tipo t2fs_record
  t2fs_record *record = (t2fs_record*) entry;
  if(strcmp(name, record->name) == 0) {
    return true;
  }
  else return false;
}



bool exists(char* name, List *entries, t2fs_record *record) {
  // procura em uma lista de entradas (do tipo t2fs_record)
  // o record de nome desejado
  t2fs_record *found = list_find(entries, compare_by_name, name);
  if(found) {
    memcpy(record, found, sizeof(t2fs_record));
    return true;
  }
  else return false;
}



int get_valid_dir_handle() {
  // itera sobre o array de diretorios abertos
  // se um estiver livre (i.e. beingUsed = falso)
  // retorna a posicao valida
  for(int i=0; i<20; i++) {
    if(!opened_dir[i].beingUsed) return i;
  }
  return ERROR;
}



int identify2 (char *name, int size)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

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
  if(path == NULL || record == NULL) return ERROR;
  
  List entries; // lista de entradas
  list_new(&entries, sizeof(t2fs_record), free);

  descriptor current_descriptor;
  if(path[0] == '/') //inicia pelo descritor do diretorio root
    current_descriptor = _root_d;
  else
    current_descriptor = cwdDescriptor;

  // pega entradas do diretorio inicial (root ou atual)
  descriptorEntries(current_descriptor, &entries);

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

  // temos que descobrir ate onde as pastas ja existem
  // e criar o resto
  t2fs_record rec;
  char *next = strtok(pathname, "/");
  char read[1024] = ""; // o que ja foi lido
  char remainer[1024] = ""; // o que falta ser criado
  printf("readz: %s\n", next);
  while(cd(next, &rec) != ERROR) {
    strcat(read, next);
    strcat(read, "/");

    next = strtok(NULL, "/");
    printf("reading %s\n", next);
  }
  strcpy(remainer, pathname+strlen(read));



  char* dir_to_create = strtok(remainer, "/");
  while(dir_to_create) {
    printf("creating... %s\n", dir_to_create);

    // 1.
    // create dir descriptor
    descriptor dir_d;
    int mftNumber = get_free_descriptor(&dir_d);
    dir_d.tuple[0].atributeType = 1;
    dir_d.tuple[1].atributeType = 0;

    dir_d.tuple[0].virtualBlockNumber = 0;
    int free_logical_block = searchBitmap2(0);

    if(free_logical_block <= 0)
      return ERROR; // nao foi possivel achar um bloco livre
    
    dir_d.tuple[0].logicalBlockNumber = free_logical_block;
    dir_d.tuple[0].numberOfContiguosBlocks = 0;

    printf("attr %d\n", dir_d.tuple[0].atributeType);
    printf("vbn %d\n", dir_d.tuple[0].virtualBlockNumber);
    printf("lbn %d\n", dir_d.tuple[0].logicalBlockNumber);
    printf("cont. blocks %d\n", dir_d.tuple[0].numberOfContiguosBlocks);
    // setBitmap2(dir_d.tuple[0].logicalBlockNumber, 1); // aloca o bloco


    // 2.
    // create record of dir
    t2fs_record dir_r;
    dir_r.TypeVal = TYPEVAL_DIRETORIO;
    strcpy(dir_r.name, dir_to_create);
    dir_r.blocksFileSize = 0;
    dir_r.bytesFileSize = 0;
    dir_r.MFTNumber = mftNumber;

    printf("typeval %d\n", dir_r.TypeVal);
    printf("name %s\n", dir_r.name);
    printf("blocksFileSize %d\n", dir_r.blocksFileSize);
    printf("bytesFileSize %d\n", dir_r.bytesFileSize);
    printf("mftNumber %d\n", dir_r.MFTNumber);



    // 3.
    // append record of dir to current descriptor list of entries



    dir_to_create = strtok(NULL, "/");
  }

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

  // pega a lista de entradas, 
  // atraves do descritor do diretorio atual
  descriptor dir_d;
  if(dir->record.MFTNumber == 0) // o root tem o record bugado
      get_descriptor(1, &dir_d);
    else 
      get_descriptor(dir->record.MFTNumber, &dir_d);
  
  List entries;
  list_new(&entries, sizeof(t2fs_record), free);
  descriptorEntries(dir_d, &entries);
  list_for_each(&entries, print_entry);


  // pega a proxima entrada, de acordo com
  // current_entry
  if(dir->current_entry < list_size(&entries)) {
    t2fs_record entry;
    list_at(&entries, dir->current_entry, &entry);

    // seta os atributos de dentry que sera retornado
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