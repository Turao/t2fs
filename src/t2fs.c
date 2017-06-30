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
bool mft_info_initialized = false;

char cwdPath[1048] = "/";
FILE2 opened[20]; //opened files

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
    printf("\n tuple %d: \n", i);
    printf("\t logical block %d", tuple.logicalBlockNumber);
    printf("\t [sector %d] \n", sector);
    printf("\t virtual block %d \n", tuple.virtualBlockNumber);
    printf("\t # contiguous blocks %d \n", tuple.numberOfContiguosBlocks);
    printf("\n");

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

}



int identify2 (char *name, int size)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();


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
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;
}



int delete2 (char *filename)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;
}



FILE2 open2 (char *filename)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;
}



int close2 (FILE2 handle)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;
}



int read2 (FILE2 handle, char *buffer, int size)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;
}



int write2 (FILE2 handle, char *buffer, int size)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;
}



int truncate2 (FILE2 handle)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;
}



int seek2 (FILE2 handle, DWORD offset)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;
}



int mkdir2 (char *pathname)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;
}



int rmdir2 (char *pathname)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;

}



DIR2 opendir2 (char *pathname)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;
}



int readdir2 (DIR2 handle, DIRENT2 *dentry)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;
}



int closedir2 (DIR2 handle)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();
  //to-do
  return ERROR;
}