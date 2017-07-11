#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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


typedef struct file_t {
  descriptor file_descriptor;
  t2fs_record record;
  char *data;
  int stream_position;
  bool opened;
} file_t;

typedef struct directory {
  descriptor dir_descriptor;
  t2fs_record record;
  char path[1024]; // absolute path
  struct directory *parent;
  int current_entry;
  bool opened;
} directory;

directory opened_dirs[20] = {0}; // opened directories

t2fs_record root_record = { TYPEVAL_DIRETORIO, "/", 0, 0, 1 };
directory root_directory;
directory *cwd;

file_t opened_files[20] = {0}; //opened files



/* other functions */
int cd(char* path, t2fs_record *record);
bool print_entry(void *e);
bool print_file_data(void *data);
void read_file(t2fs_record file_r, char *data);
bool compare_record_by_name(void* entry, void *name);
bool exists(char* name, List *entries, t2fs_record *record);
int get_valid_dir_handle();
int get_valid_file_handle();
bool find_by_tuple_record_name_and_invalidate(void *t, void* n);
bool tuple_cleanup(void *t);
int save_file(file_t *f);

bool print_entry(void *e) {
  t2fs_record *record = (t2fs_record*) e;
  
  DEBUG_PRINT("name: %s \n", record->name);
  DEBUG_PRINT("typeval: %x \n", record->TypeVal);
  DEBUG_PRINT("block file size: %d \n", record->blocksFileSize);
  DEBUG_PRINT("bytes file size: %d \n", record->bytesFileSize);
  DEBUG_PRINT("mft number: %d \n", record->MFTNumber);
  DEBUG_PRINT("\n");

  return true;
}


/* Iterative function (to be used with list_for_each)!!
*  Takes a 4tuple, reads its LBN value and prints the content
*  of the block.
*
*  Author: Arthur Lenz
*/
bool print_file_data(void *data) {
  t2fs_4tupla *tuple = (t2fs_4tupla*) data;
  if(tuple == NULL) return false;

  // reads the sector, based on the LBN given by the tuple
  unsigned char buffer[256];
  read_sector(logicalBlock_sector(tuple->logicalBlockNumber), buffer);
  
  // then, just print its content as a string
  DEBUG_PRINT("%s\n", buffer);
  return true;
}



/* Takes a file record and a pointer to a buffer.
*  Reads each one of the blocks mapped by the file
*  and stores the content in the buffer.
*
*  Author: Arthur Lenz
*/
void read_file(t2fs_record file_r, char *data) {
  //looks for the descriptor of the file, in the mft
  descriptor file_d;
  get_descriptor(file_r.MFTNumber, &file_d);

  //get all of its valid (mapped) tuples
  List valid_tuples;
  list_new(&valid_tuples, sizeof(t2fs_4tupla), free);
  descriptor_tuples(file_d, &valid_tuples);

  //for each valid tuple, read the data
  t2fs_4tupla tuple; // iterator
  char buffer[256];
  for(int i=0; i<list_size(&valid_tuples); i++) {
    list_at(&valid_tuples, i, &tuple);
    // reads the sector, based on the LBN given by the tuple
    read_sector(logicalBlock_sector(tuple.logicalBlockNumber), (unsigned char*) buffer);
    strcat(data, buffer);
  }
}



/* Iterative function (to be used with list_for_each)!!
*  Compares a (valid) record with a given name.
*
*  Author: Arthur Lenz
*/
bool compare_record_by_name(void* entry, void *name) {
  // funcao comparadora
  // recebe um ponteiro para o objeto de tipo t2fs_record
  t2fs_record *record = (t2fs_record*) entry;
  if(strcmp(name, record->name) == 0 &&
     record->TypeVal != TYPEVAL_INVALIDO) {
    return true;
  }
  else return false;
}


/* Searches for the record given a name and a 
*  list of entries (of type t2fs_record)
*
*  [output] t2fs_record *record: when found, a copy of the correspondent record
*  [return] a boolean stating if the record exists in the given list
*
*  Author: Arthur Lenz
*/
bool exists(char* name, List *entries, t2fs_record *record) {
  // procura em uma lista de entradas (do tipo t2fs_record)
  // o record de nome desejado
  t2fs_record *found = list_find(entries, compare_record_by_name, name);
  if(found) {
    memcpy(record, found, sizeof(t2fs_record));
    return true;
  }
  else return false;
}




/* Iterates over the opened directories array 
*  looking for a free space
*
*  [return] the free space index, when found
*           -1 if there is no free space available
*
*  Author: Arthur Lenz
*/
int get_valid_dir_handle() {
  // itera sobre o array de diretorios abertos
  // se um estiver livre (i.e. beingUsed = falso)
  // retorna a posicao valida
  for(int i=0; i<20; i++) {
    if(!opened_dirs[i].opened) return i;
  }
  return ERROR;
}



/* Iterates over the opened files array 
*  looking for a free space
*
*  [return] the free space index, when found
*           -1 if there is no free space available
*
*  Author: Arthur Lenz
*/
int get_valid_file_handle() {
  // look at get_valid_dir_handle
  for(int i=0; i<20; i++) {
    if(!opened_files[i].opened) return i;
  }
  return ERROR;
}



/* Copies the name of the developers 
*  in a given buffer (char * name) of a given size
*
*  [return] 0 when copied
*           -1 whenever there's an error
*
*  Author: Arthur Lenz
*/
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



/* Creates a file in a given filepath (char *filename) 
*  (After creating the file, opens it)
*
*  [return] the file handle (position where the information
*           about the file is stored within the opened files array)
*           -1 whenever there's an error
*
*  Author: Arthur Lenz
*/
FILE2 create2 (char *filename)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  char *path = (char*) strdup(filename);
  if(path == NULL) return ERROR;

  if(path[0] == '/') { // caminho absoluto
    cwd = &root_directory;
  }


  // temos que colocar o cwd no caminho do arquivo a ser lido
  t2fs_record record;
  char *next;
  if(strchr(path, '/'))
    next = strtok(path, "/");
  else 
    next = path;
  while(cd(next, &record) != ERROR) {
    next = strtok(NULL, "/");
  }
  if(next == NULL) // leu tudo: usuario deu um caminho apenas com
    return ERROR; // pastas

  char *name = next;

  // olhar se o arquivo ja existe
  List entries;
  list_new(&entries, sizeof(t2fs_record), free);
  get_descriptor(cwd->dir_descriptor.MFTNumber, &cwd->dir_descriptor);
  descriptorEntries(cwd->dir_descriptor, &entries);
  if(exists(name, &entries, &record)) return ERROR; // nao pode criar se ja existe

  // 0.
  // create file structure to hold file descriptor and record
  file_t f;

  // 1.
  // find free mft descriptor to hold the new file's blocks information
  int mftNumber = get_free_descriptor(&f.file_descriptor);
  for(int i=1; i<32; i++)
    f.file_descriptor.tuple[i].atributeType = 0; // resets descriptor attr types

  int free_logical_block = searchBitmap2(0);
  if(free_logical_block <= 0)
    return ERROR; // nao foi possivel achar um bloco livre
  setBitmap2(free_logical_block, 1); // aloca o bloco

  List file_tuples;
  list_new(&file_tuples, sizeof(t2fs_4tupla), free);
  write_descriptor(&f.file_descriptor, &file_tuples);
  

  // 2.
  // create record of file
  f.record.TypeVal = TYPEVAL_REGULAR;
  strcpy(f.record.name, next);
  f.record.blocksFileSize = 0;
  f.record.bytesFileSize = 0;
  f.record.MFTNumber = mftNumber;


  // 3.
  // write record in logical block
  unsigned char buffer[256] = "";
  memcpy(buffer, &f.record, sizeof(t2fs_record));
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

  int handle = open2(f.record.name);
  if(handle == ERROR) return ERROR;
  else 
    return handle;
}



/* Iterative function (to be used with list_for_each)!!
*  Given a 4tuple, frees its logical block associated,
*  clears the memory and sets it to -1 (free mft tuple)
*
*  Author: Arthur Lenz
*/
bool tuple_cleanup(void *t) {
  t2fs_4tupla *tuple = (t2fs_4tupla*) t;
  setBitmap2(tuple->logicalBlockNumber, 0); // free block
  memset(tuple, -1, sizeof(t2fs_4tupla)); // resets
  return true;
}



/* Deletes a file in a given filepath (char *filename) 
*
*  [return] 0 if succeeded
*           -1 whenever there's an error
*
*  Author: Arthur Lenz
*/
int delete2 (char *filename)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  char *path = (char*) strdup(filename);
  if(path == NULL) return ERROR;

  if(path[0] == '/') { // caminho absoluto
    cwd = &root_directory;
  }

  // temos que colocar o cwd no caminho do arquivo a ser lido
  t2fs_record record;
  char *next;
  if(strchr(path, '/'))
    next = strtok(path, "/");
  else 
    next = path;
  while(cd(next, &record) != ERROR) {
    next = strtok(NULL, "/");
  }
  if(next == NULL) // leu tudo: usuario deu um caminho apenas com
    return ERROR; // pastas

  char *name = next;

  // olhar se o arquivo ja existe
  List entries;
  list_new(&entries, sizeof(t2fs_record), free);
  get_descriptor(cwd->dir_descriptor.MFTNumber, &cwd->dir_descriptor);
  descriptorEntries(cwd->dir_descriptor, &entries);
  if(!exists(name, &entries, &record)) return ERROR; // nao pode deletar se nao existe

  
  file_t f;
  f.record = record;
  get_descriptor(f.record.MFTNumber, &f.file_descriptor);
  
  // delete and free all blocks mapped
  // thus, reseting the descriptor
  List tuples;
  list_new(&tuples, sizeof(t2fs_4tupla), free);
  descriptor_tuples(f.file_descriptor, &tuples);
  list_for_each(&tuples, tuple_cleanup);

  // clears the record on the directory descriptor
  List cwd_tuples;
  list_new(&cwd_tuples, sizeof(t2fs_4tupla), free);
  get_descriptor(cwd->dir_descriptor.MFTNumber, 
                 &cwd->dir_descriptor);
  descriptor_tuples(cwd->dir_descriptor, &cwd_tuples);

  // find the tuple to be clear, and do it
  // frees the block (doing that in the find_by_tuple... function)
  // t2fs_4tupla *tuple = 
  list_find(&cwd_tuples, find_by_tuple_record_name_and_invalidate,
            f.record.name);

  write_descriptor(&cwd->dir_descriptor, &cwd_tuples);

  return SUCCESS;
}


/* Changes the current working directory based on a give path (char* path)
*
*  [output] t2fs_record *record: the record of the directory
*                                entered (cwd's record)
*  [return] 0 if succeeded
*           -1 whenever there's an error
*
*  Author: Arthur Lenz
*/
int cd(char* path, t2fs_record *record)
{
  if(path == NULL || record == NULL) return ERROR;
  
  List entries; // lista de entradas
  list_new(&entries, sizeof(t2fs_record), free);

  // pega entradas do diretorio inicial (root ou atual)
  descriptorEntries(cwd->dir_descriptor, &entries);

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
    if(record->TypeVal != TYPEVAL_DIRETORIO) return ERROR;
    // encontramos a pasta. criamos um diretorio
    directory *found = calloc(1, sizeof(directory));
    memcpy(&found->record, record, sizeof(t2fs_record));
    get_descriptor(found->record.MFTNumber, &found->dir_descriptor);
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


/* Opens a file in a given filepath (char *filename) 
*
*  [return] the file handle (position where the information
*           about the file is stored within the opened files array)
*           -1 whenever there's an error
*
*  Author: Arthur Lenz
*/
FILE2 open2 (char *filename)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  char *path = (char*) strdup(filename);
  if(path == NULL) return ERROR;

  if(path[0] == '/') { // caminho absoluto
    cwd = &root_directory;
  }

  // temos que colocar o cwd no caminho do arquivo a ser lido
  t2fs_record record;
  char *next = strtok(path, "/");
  while(cd(next, &record) != ERROR) {
    next = strtok(NULL, "/");
  }
  if(next == NULL) // leu tudo: usuario deu um caminho apenas com
    return ERROR; // pastas

  char *name = next;

  List entries;
  list_new(&entries, sizeof(t2fs_record), free);
  get_descriptor(cwd->dir_descriptor.MFTNumber, &cwd->dir_descriptor);
  descriptorEntries(cwd->dir_descriptor, &entries);
  
  if(!exists(name, &entries, &record)) return ERROR; // arquivo nao existe

  // init file structure
  file_t f;
  f.record = record;
  f.data = (char*) calloc(1, sizeof(char));
  if(f.data == NULL) return ERROR; // calloc problem
  get_descriptor(f.record.MFTNumber, &f.file_descriptor);
  f.stream_position = 0;
  f.opened = true;

  int position = get_valid_file_handle();
  if(position == ERROR) return ERROR;
  opened_files[position] = f;

  read_file(f.record, f.data);

  return position;
}


/* Closes a file given its associated handle 
*
*  [return] 0 if succeded
*           -1 whenever there's an error
*
*  Author: Arthur Lenz
*/
int close2 (FILE2 handle)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  // posicao invalida
  // [retorna -2 : nao pode usar o -1 do ERROR]
  // handle fora do range valido
  if(handle < 0 || handle >= 20) return -2;
  // o diretorio nao esta aberto
  if(!opened_files[handle].opened) return -2;

  // se o handle passado for valido
  // so "liberamos" a vaga, setando beingUsed
  // para falso
  opened_files[handle].opened = false;

  return ERROR;
}



int read2 (FILE2 handle, char *buffer, int size)
{
  if(buffer == NULL) return ERROR;

  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  // valida o handle:

  // posicao invalida
  // [retorna -2 : nao pode usar o -1 do ERROR]
  // handle fora do range valido
  if(handle < 0 || handle >= 20) return -2;
  // o diretorio nao esta aberto
  if(!opened_files[handle].opened) return -2;

  file_t *f = &opened_files[handle];

  int availableBytes = f->record.bytesFileSize - f->stream_position;

  if(availableBytes >= size) { // tem 'size' bytes para escrever no buffer
    strncpy(buffer, f->data+f->stream_position, size);
    f->stream_position += size;
    return size;
  }
  else { // nao tem, devemos copiar o que da e retornar o tamanho
    strncpy(buffer, f->data+f->stream_position, availableBytes);
    f->stream_position = f->record.bytesFileSize;
    return availableBytes;    
  }
}

int save_file(file_t *f)
{
  // limpa todo o mapeamento atual no mft do arquivo

  // pega as tuplas do arquivo mapeado
  List tuples;
  list_new(&tuples, sizeof(t2fs_4tupla), free);
  get_descriptor(f->file_descriptor.MFTNumber, &f->file_descriptor);
  descriptor_tuples(f->file_descriptor, &tuples);
  // para cada tupla:
  // reseta a tupla na memoria e libera os blocos mapeados por ela
  list_for_each(&tuples, tuple_cleanup);

  // escreve um novo mapeamento para os dados
  int blockSize_bytes = _bootBlock.blockSize * SECTOR_SIZE;
  int blocksNeeded = 1 + (f->record.bytesFileSize / blockSize_bytes);

  // para cada bloco necessario, cria uma tupla de mapeamento
  // e escreve o dado no disco
  List new_tuples;
  list_new(&new_tuples, sizeof(t2fs_4tupla), free);
  
  unsigned char internal_buffer[256];
  int internal_pointer = 0;

  for(int i=0; i<blocksNeeded; i++) {
    // cria tupla de mapeamento
    int free_logical_block = searchBitmap2(0);
    
    t2fs_4tupla tuple;
    tuple.atributeType = 1; // mapeamento
    tuple.virtualBlockNumber = i; // numero do bloco (relativo ao arquivo)
    tuple.logicalBlockNumber = free_logical_block;
    tuple.numberOfContiguosBlocks = 0;
    list_push_back(&new_tuples, &tuple);

    // marca o bloco logico como 'em uso'
    if(setBitmap2(free_logical_block, 1) != SUCCESS)
      return ERROR;

    // agora precisamos escrever o bloco
    int sector = logicalBlock_sector(tuple.logicalBlockNumber);
    for(int j=0; j<_bootBlock.blockSize; j++) {     
      // para cada setor dentro do bloco
      // copiamos os 256 bytes (SECTOR_SIZE) respectivos
      memset(internal_buffer, 0, 256); // para evitar lixo
      memcpy(internal_buffer, f->data+internal_pointer, 256);
      internal_pointer += 256;
      if(write_sector(sector+j, internal_buffer) != SUCCESS) return ERROR;
    }
  }

  // por fim, atualiza o descritor mft com as novas
  // tuplas mapeadas
  write_descriptor(&f->file_descriptor, &new_tuples);

  return SUCCESS;
}

int write2 (FILE2 handle, char *buffer, int size)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();


  // valida o handle:
  // posicao invalida
  // [retorna -2 : nao pode usar o -1 do ERROR]
  // handle fora do range valido
  if(handle < 0 || handle >= 20) return -2;
  // o diretorio nao esta aberto
  if(!opened_files[handle].opened) return -2;

  file_t *f = &opened_files[handle];

  
  if(f->stream_position + size <= f->record.bytesFileSize) {
    // sobrescreve (nao ultrapassa o tamanho do original)
    memcpy(f->data+f->stream_position, buffer, size);
  }
  else {
    char *internal_buffer = (char*) calloc(f->stream_position+size+1, 
                                           sizeof(char));
    memcpy(internal_buffer, f->data, f->stream_position);
    memcpy(internal_buffer, buffer, size);
    free(f->data);
    f->data = internal_buffer;
  }
  // atualiza os atributos do arquivo
  f->stream_position += size;
  f->record.bytesFileSize = sizeof(BYTE)*strlen(f->data);

  if(save_file(f) == SUCCESS) return size;
  else return ERROR;
}



int truncate2 (FILE2 handle)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();


  // clears on memory

  // valida o handle:

  // posicao invalida
  // [retorna -2 : nao pode usar o -1 do ERROR]
  // handle fora do range valido
  if(handle < 0 || handle >= 20) return -2;
  // o diretorio nao esta aberto
  if(!opened_files[handle].opened) return -2;

  file_t *f = &opened_files[handle];

  memset(f->data+f->stream_position, 0, 
         sizeof(f->record.bytesFileSize - f->stream_position));

  f->record.bytesFileSize = f->stream_position;

  return save_file(f);
}


/* Sets the position of the stream pointer associated with
*  an opened file, given its handle (FILE2 handle) and offset (DWORD offset) 
*
*  [return] 0 if succeded
*           -1 whenever there's an error
*
*  Author: Arthur Lenz
*/
int seek2 (FILE2 handle, DWORD offset)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  // posicao invalida
  // [retorna -2 : nao pode usar o -1 do ERROR]
  // handle fora do range valido
  if(handle < 0 || handle >= 20) return -2;
  // o diretorio nao esta aberto
  if(!opened_files[handle].opened) return -2;

  file_t *f = &opened_files[handle];

  // valida o offset, checando se
  // atual + offset ultrapassa o tamanho do arquivo
  if(offset > f->record.bytesFileSize) {
    return ERROR;
  }
  else {
    f->stream_position = offset;
    return SUCCESS;
  }
}



/* Creates one or multiple directories given a path
*  Both absolute and relative paths are supported!
*  [example]
*    "/home/" - creates one folder, if home doesn't already exists
*    "/home/desktop" - creates two folders, if they dont exist
*
*  [return] 0 if succeded
*           -1 whenever there's an error
*
*  Author: Arthur Lenz
*/
int mkdir2 (char *pathname)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  char *path = (char*) strdup(pathname);
  if(path == NULL) return ERROR;

  if(path[0] == '/') { // caminho absoluto
    cwd = &root_directory;
  }

  // temos que descobrir ate onde as pastas ja existem
  // e criar o resto
  t2fs_record rec;
  char *next = strtok(path, "/");
  char read[1024] = ""; // o que ja foi lido
  while(cd(next, &rec) != ERROR) {
    strcat(read, next);
    strcat(read, "/");

    next = strtok(NULL, "/");
  }
  if(next == NULL) // leu tudo: acessou todo o path
    return ERROR; // caminho invalido porque pasta ja existe

  List entries;
  list_new(&entries, sizeof(t2fs_record), free);

  while(next) {
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
    // find free mft descriptor to hold the new directory's entries
    int mftNumber = get_free_descriptor(&new_dir.dir_descriptor);
    for(int i=1; i<32; i++)
      new_dir.dir_descriptor.tuple[i].atributeType = 0; // resets descriptor attr types

    int free_logical_block = searchBitmap2(0);
    if(free_logical_block <= 0)
      return ERROR; // nao foi possivel achar um bloco livre
    setBitmap2(free_logical_block, 1); // aloca o bloco

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



bool find_by_tuple_record_name_and_invalidate(void *t, void* n) {
  t2fs_4tupla *tuple = (t2fs_4tupla*) t;
  char *name = (char*) n;

  int sector = logicalBlock_sector(tuple->logicalBlockNumber);
  unsigned char buffer[256];
  t2fs_record record[4]; // 4 records per sector
  read_sector(sector, buffer);
  memcpy(&record, buffer, sizeof(t2fs_record)*4);
  for(int i=0; i<4; i++) {
    if(strcmp(record[i].name, name) == 0 &&
       record[i].TypeVal != TYPEVAL_INVALIDO) { // achou o registro na tupla
      memset(&record[i], 0, sizeof(t2fs_record)); // resets the record
      memcpy(buffer, &record, sizeof(t2fs_record)*4);
      write_sector(sector, buffer);
      tuple->atributeType = 3; // IGNORE
      setBitmap2(tuple->logicalBlockNumber, 0); //frees block
    }
  }
  return false;
}



/* Deletes one or multiple directories given a path
*  Both absolute and relative paths are supported!
*  [example]//
*    "/home/" - creates one folder, if home doesn't already exists
*    "/home/desktop" - creates two folders, if they dont exist
*
*  [return] 0 if succeded
*           -1 whenever there's an error
*
*  Author: Arthur Lenz
*/
int rmdir2 (char *pathname)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  char *path = (char*) strdup(pathname);
  if(path == NULL) return ERROR;
  
  if(path[0] == '/') { // caminho absoluto
    cwd = &root_directory;
  }

  // temos que colocar o cwd no caminho desejado
  t2fs_record rec;
  char *next = strtok(path, "/");
  if(next == NULL) return ERROR; // nao pode remover root
  while(cd(next, &rec) != ERROR) {
    next = strtok(NULL, "/");
  }
  if(next != NULL) // nao leu tudo: usuario deu um caminho com
    return ERROR; // pastas que nao existem

  // 1.
  // free cwd's descriptor (on disk)
  List tuples;
  list_new(&tuples, sizeof(t2fs_4tupla), free);
  get_descriptor(cwd->dir_descriptor.MFTNumber, &cwd->dir_descriptor);
  descriptor_tuples(cwd->dir_descriptor, &tuples);
  list_for_each(&tuples, tuple_cleanup);
  write_descriptor(&cwd->dir_descriptor, &tuples);


  // 2.
  // get cwd's parent descriptor tuples
  List parent_tuples;
  list_new(&parent_tuples, sizeof(t2fs_4tupla), free);
  get_descriptor(cwd->parent->dir_descriptor.MFTNumber, 
                 &cwd->parent->dir_descriptor);
  descriptor_tuples(cwd->parent->dir_descriptor, &parent_tuples);

  // 4.
  // find the tuple to be clear, and do it
  // 5.
  // frees the block (doing that in the find_by_tuple... function)
  list_find(&parent_tuples, 
            find_by_tuple_record_name_and_invalidate,
            cwd->record.name);

  write_descriptor(&cwd->parent->dir_descriptor, &parent_tuples);

  cwd = cwd->parent;
  return SUCCESS;
}



/* Opens a directory given its path (char *pathname) 
*
*  [return] the file handle (position where the information
*           about the directory is stored within the opened directories array)
*           -1 whenever there's an error
*
*  Author: Arthur Lenz
*/
DIR2 opendir2 (char *pathname)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  char *path = (char*) strdup(pathname);
  if(path == NULL) return ERROR;

  if(path[0] == '/') { // caminho absoluto
    cwd = &root_directory;
  }

  // tenta acessar o caminho especificado
  // se existir, teremos o diretorio em cwd
  char *next = strtok(path, "/");
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
    opened_dirs[position] = dir;
    return position;
  }
  else
    // se nao existe nenhuma posicao valida
    // (ja tem 20 diretorios abertos)
    return ERROR;
}


/* Given its associated handle, outputs the information of its entries
*  (both files and sub-directories)
*  Each subsequent call returns the next entry within the directory.
*
*  [output] DIRENTRY *dentry: a DIRENT2 structure containing one entry
*  [return] 0 if succeeded
*           -1 whenever there's an error
*
*  Author: Arthur Lenz
*/
int readdir2 (DIR2 handle, DIRENT2 *dentry)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  // posicao invalida
  // [retorna -2 : nao pode usar o -1 do ERROR]
  // handle fora do range valido
  if(handle < 0 || handle >= 20) return -2;
  // o diretorio nao esta aberto
  if(!opened_dirs[handle].opened) return -2;

  directory *dir = &opened_dirs[handle];

  List entries;
  list_new(&entries, sizeof(t2fs_record), free);
  
  // refresh descriptor (may not be the same as when it was opened)
  get_descriptor(dir->dir_descriptor.MFTNumber, &dir->dir_descriptor);

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



/* Closes a file given its associated handle 
*
*  [return] 0 if succeeded
*           -1 whenever there's an error
*
*  Author: Arthur Lenz
*/
int closedir2 (DIR2 handle)
{
  if(!disk_info_initialized) INIT_DISK_INFO();
  if(!mft_info_initialized) INIT_MFT_INFO();

  // posicao invalida
  // [retorna -2 : nao pode usar o -1 do ERROR]
  // handle fora do range valido
  if(handle < 0 || handle >= 20) return -2;
  // o diretorio nao esta aberto
  if(!opened_dirs[handle].opened) return -2;


  // se o handle passado for valido
  // so "liberamos" a vaga, setando beingUsed
  // para falso
  opened_dirs[handle].opened = false;


  return SUCCESS;
}