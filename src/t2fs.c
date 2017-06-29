#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define DEBUG true

#include "t2fs.h"
#include "boot.h"
#include "mft.h"

#include "apidisk.h"
#include "bitmap2.h"
#include "utils.h"

#define DEVELOPERS "Arthur Lenz 218316"
char devs[] = DEVELOPERS;

bool disk_info_initialized = false;
bool mft_info_initialized = false;

char cwdPath[1048] = "/";
FILE2 opened[20]; //opened files


int _logicalBlock_sector(int logicalBlockNumber) {
  return logicalBlockNumber * _bootBlock.blockSize;
}

void test_open_root() {
  DEBUG_PRINT("Reading root dir \n");

  int sector;
  unsigned char buffer[256];
  t2fs_record record[4]; //records per sector

  printf("\n");

  for(int i=0; i<32 && _root_d.tuple[i].atributeType == 1; i++) {
    sector = _logicalBlock_sector(_root_d.tuple[i].logicalBlockNumber);
    printf("tuple %d:\n", i);
    printf("logical block %d\n", _root_d.tuple[i].logicalBlockNumber);
    printf("sector %d\n", sector);
    printf("\n");

    read_sector(sector, buffer);
    memcpy(&record, buffer, sizeof(t2fs_record)*4);

    for(int i=0; i<4; i++)
    {
      if(record[i].TypeVal != TYPEVAL_INVALIDO) {
        printf("name: %s \n", record[i].name);
        printf("typeval: %x \n", record[i].TypeVal);
        printf("block file size: %d \n", record[i].blocksFileSize);
        printf("bytes file size: %d \n", record[i].bytesFileSize);
        printf("mft number: %d \n", record[i].MFTNumber);
        printf("\n");
      }
    }
  }

  //open file1


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