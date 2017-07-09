#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define DEBUG false

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
#define INIT_MFT_INFO() { init_mft_info(); mft_info_initialized = true; INIT_GLOBALS(); }

#define INIT_GLOBALS() { \
  root_directory.dir_descriptor = _root_d; \
  root_directory.record = root_record; \
  strcpy(root_directory.path, "/"); \
  root_directory.parent = NULL; \
  root_directory.current_entry = 0; \
  root_directory.opened = false; \
  cwd = &root_directory; \
}


typedef struct directory {
  descriptor dir_descriptor;
  t2fs_record record;
  char path[1024]; // absolute path
  struct directory *parent;
  int current_entry;
  bool opened;
} directory;

directory opened_dir[20] = {0, 0, 0, false}; // opened directories

t2fs_record root_record = { TYPEVAL_DIRETORIO, "/", 0, 0, 1 };
directory root_directory;
directory *cwd;

FILE2 opened[20]; //opened files


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
    if(!opened_dir[i].opened) return i;
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
  if(path == NULL || record == NULL) return ERROR;
  
  List entries; // lista de entradas
  list_new(&entries, sizeof(t2fs_record), free);

  // pega entradas do diretorio inicial (root ou atual)
  descriptorEntries(cwd->dir_descriptor, &entries);

  printf("************** cd %s  (cwd %s ) ********** \n", path, cwd->path);
  printf("printing (%s) entries \n", cwd->path);
  list_for_each(&entries, print_entry);

  if(strcmp(path, ".") == 0) {
    // diretorio atual. nao fazer nada
    return SUCCESS;
  }

  if(strcmp(path, "..") == 0 && cwd->parent != NULL) {
    // libera o cwd atual e sobe um nivel
    directory *to_be_freed = cwd;
    cwd = cwd->parent;
    free(to_be_freed);
    return SUCCESS;
  }

  // se a entrada pesquisada existe na lista de entradas do diretorio,
  // teremos o record do diretorio/arquivo desejado em record_found
  if(exists(path, &entries, record)) {
    printf("[cd] achou %s\n", path);
    if(record->TypeVal != TYPEVAL_DIRETORIO) return ERROR;
    // encontramos a pasta. criamos um diretorio
    directory *found = calloc(1, sizeof(directory));
    memcpy(&found->record, record, sizeof(t2fs_record));
    get_descriptor(found->record.MFTNumber, &cwd->dir_descriptor);
    found->parent = cwd;
    strcpy(found->path, found->parent->path);
    strcat(found->path, path);

    found->current_entry = 0;
    found->opened = false;

    // finalmente, seta o cwd como a pasta encontrada
    cwd = found;

    return SUCCESS;
  }
  else return ERROR;  
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
  // //to-do

  // temos que descobrir ate onde as pastas ja existem
  // e criar o resto
  t2fs_record rec;
  char *next = strtok(pathname, "/");
  char read[1024] = ""; // o que ja foi lido
  while(cd(next, &rec) != ERROR) {
    strcat(read, next);
    strcat(read, "/");

    next = strtok(NULL, "/");
  }
  if(next == NULL) // leu tudo: acessou todo o pathname
    return ERROR; // caminho invalido porque pasta ja existe

  // char remainer[1024] = "";
  // strcpy(remainer, pathname+strlen(read));
  // printf("remainer %s\n", remainer);

  List entries;
  list_new(&entries, sizeof(t2fs_record), free);

  while(next) {
    descriptorEntries(cwd->dir_descriptor, &entries);
    list_for_each(&entries, print_entry);

    directory new_dir;
    // new_dir.dir_descriptor; // below (1)
    // new_dir.record; // below (2)
    strcpy(new_dir.path, cwd->path);
    strcat(new_dir.path, next);
    strcat(new_dir.path, "/");
    new_dir.parent = cwd;
    new_dir.current_entry = 0;
    new_dir.opened = false;

    // 1.
    // create dir descriptor & write to disk
    int mftNumber = get_free_descriptor(&new_dir.dir_descriptor);
    for(int i=1; i<32; i++)
      new_dir.dir_descriptor.tuple[i].atributeType = -1; // resets descriptor attr types

    new_dir.dir_descriptor.tuple[0].atributeType = 1; // mapeamento
    new_dir.dir_descriptor.tuple[1].atributeType = 0; // fim do encadeamento

    new_dir.dir_descriptor.tuple[0].virtualBlockNumber = 0;
    int free_logical_block = searchBitmap2(0);
    if(free_logical_block <= 0)
      return ERROR; // nao foi possivel achar um bloco livre
    
    new_dir.dir_descriptor.tuple[0].logicalBlockNumber = free_logical_block;
    new_dir.dir_descriptor.tuple[0].numberOfContiguosBlocks = 0;
    setBitmap2(new_dir.dir_descriptor.tuple[0].logicalBlockNumber, 1); // aloca o bloco

    List dir_tuples;
    list_new(&dir_tuples, sizeof(t2fs_4tupla), free);
    write_descriptor(&new_dir.dir_descriptor, &dir_tuples);
    
    // 2.
    // create record of dir
    new_dir.record.TypeVal = TYPEVAL_DIRETORIO;
    strcpy(new_dir.record.name, next);
    new_dir.record.blocksFileSize = 0;
    new_dir.record.bytesFileSize = 0;
    new_dir.record.MFTNumber = mftNumber;

    // printf("newdir record name %s\n", new_dir.record.name);
    // printf("newdir record blocks %d\n", new_dir.record.blocksFileSize);
    // printf("newdir record bytes %d\n", new_dir.record.bytesFileSize);
    // printf("newdir record mft %d\n", new_dir.record.MFTNumber);


    // 3.
    // write record in logical block
    unsigned char buffer[256] = "";
    memcpy(buffer, &new_dir.record, sizeof(t2fs_record));
    write_sector(logicalBlock_sector(free_logical_block), buffer);


    // 4.
    // append dir tuple to current descriptor list of tuples
    t2fs_4tupla dir_t;
    dir_t.atributeType = 1; // mapeamento
    dir_t.virtualBlockNumber = 0;
    dir_t.logicalBlockNumber = free_logical_block;
    dir_t.numberOfContiguosBlocks = 0;

    List cwd_tuples;
    list_new(&cwd_tuples, sizeof(t2fs_4tupla), free);
    descriptor_tuples(cwd->dir_descriptor, &cwd_tuples);
    // updates last tuple, to be a valid vbn-lbn mapped tuple (1),
    // instead of a end tuple (0)
    if(list_size(&cwd_tuples) > 0) {
      t2fs_4tupla *last_tuple = list_pop_back(&cwd_tuples);
      last_tuple->atributeType = 1;
      list_push_back(&cwd_tuples, last_tuple);
    }
    list_push_back(&cwd_tuples, &dir_t);

    t2fs_4tupla end;
    end.atributeType = 0; // fim de encadeamento
    end.virtualBlockNumber = 0;
    end.logicalBlockNumber = 0;
    end.numberOfContiguosBlocks = 0;
    list_push_back(&cwd_tuples, &end);

    write_descriptor(&cwd->dir_descriptor, &cwd_tuples);


    // 6.
    // cd created folder
    t2fs_record dummy_record; // wont need here
    cd(next, &dummy_record);

    next = strtok(NULL, "/");
  }

  return SUCCESS;
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

  if(pathname[0] == '/') { // caminho absoluto
    cwd = &root_directory;
  }

  // tenta acessar o caminho especificado
  // se existir, teremos o diretorio em cwd
  char *next = strtok(pathname, "/");
  directory dir;
  while(next != NULL && cd(next, &dir.record) != ERROR) {
    next = strtok(NULL, "/");
  }
  // testa se o path foi completamente parseado
  if(next != NULL) // se sobrou algo, nao foi
    return ERROR;

  // copiamos o conteudo de cwd para dir
  memcpy(&dir, cwd, sizeof(directory));

  // coloca dir na proxima posicao valida do array de diretorios abertos
  int position = get_valid_dir_handle();

  if(position >= 0 && position < 20) {
    dir.opened = true;
    opened_dir[position] = dir;
    return position;
  }
  else
    // se nao existe nenhuma posicao valida
    // (ja tem 20 diretorios abertos)
    return ERROR;
}



int readdir2 (DIR2 handle, DIRENT2 *dentry)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  // posicao invalida
  // [retorna -2 : nao pode usar o -1 do ERROR]
  // handle fora do range valido
  if(handle < 0 || handle >= 20) return -2;
  // o diretorio nao esta aberto
  if(!opened_dir[handle].opened) return -2;

  directory *dir = &opened_dir[handle];

  List entries;
  list_new(&entries, sizeof(t2fs_record), free);
  descriptorEntries(dir->dir_descriptor, &entries);

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

  // posicao invalida
  // [retorna -2 : nao pode usar o -1 do ERROR]
  // handle fora do range valido
  if(handle < 0 || handle >= 20) return -2;
  // o diretorio nao esta aberto
  if(!opened_dir[handle].opened) return -2;


  // se o handle passado for valido
  // so "liberamos" a vaga, setando beingUsed
  // para falso
  opened_dir[handle].opened = false;


  return SUCCESS;
}