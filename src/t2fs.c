#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#define DEBUG true

#include "t2fs.h"

#include "apidisk.h"
#include "bitmap2.h"
#include "utils.h"



#define DEVELOPERS "Arthur Lenz 218316"
char devs[] = DEVELOPERS;

bool disk_info_initialized = false;

char cwdPath[1048] = "/";


struct t2fs_bootBlock _bootBlock;
bool _init_t2fs_bootBlock()
{
  DEBUG_PRINT("Initializing boot block info \n");
  unsigned char boot_sector[256];
  if(read_sector (0, boot_sector) != SUCCESS) {
    STDERR_INFO("Boot sector init failure");
    return false;
  }

  memcpy(&_bootBlock, boot_sector, sizeof(t2fs_bootBlock));

  DEBUG_PRINT("id: %c%c%c%c \n", _bootBlock.id[0], _bootBlock.id[1],
                            _bootBlock.id[2], _bootBlock.id[3]);
  DEBUG_PRINT("version: %x \n", _bootBlock.version);
  DEBUG_PRINT("blocksize: %x \n", _bootBlock.blockSize);
  DEBUG_PRINT("mft blocksize: %x \n", _bootBlock.MFTBlocksSize);
  DEBUG_PRINT("disk sector size: %x \n", _bootBlock.diskSectorSize);
  return true;

}



int identify2 (char *name, int size)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();

  if(size < sizeof(devs)) return ERROR;
  else
  {
    strncpy(name, devs, sizeof(devs));
    return SUCCESS;
  }
}



FILE2 create2 (char *filename)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;
}



int delete2 (char *filename)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;
}



FILE2 open2 (char *filename)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;
}



int close2 (FILE2 handle)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;
}



int read2 (FILE2 handle, char *buffer, int size)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;
}



int write2 (FILE2 handle, char *buffer, int size)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;
}



int truncate2 (FILE2 handle)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;
}



int seek2 (FILE2 handle, DWORD offset)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;
}



int mkdir2 (char *pathname)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;
}



int rmdir2 (char *pathname)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;

}



DIR2 opendir2 (char *pathname)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;
}



int readdir2 (DIR2 handle, DIRENT2 *dentry)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;
}



int closedir2 (DIR2 handle)
{
  if(!disk_info_initialized) _init_t2fs_bootBlock();
  //to-do
  return ERROR;
}