#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

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
  unsigned char boot_sector[256];
  if(read_sector (0, boot_sector) != SUCCESS) {
    STDERR_INFO("Boot sector init failure");
    return false;
  }

  _bootBlock.id = {boot_sector[3], boot_sector[2], boot_sector[1], boot_sector[0]};
  // _bootBlock.version;
  // _bootBlock.blocksize;
  // _bootBlock.MFTBlocksSize;
  // _bootBlock.diskSectorSize;

  printf("id: %s\n", _bootBlock.id);

}



int identify2 (char *name, int size)
{
  if(size < sizeof(devs)) return ERROR;
  else
  {
    strncpy(name, devs, sizeof(devs));
    return SUCCESS;
  }
}



FILE2 create2 (char *filename)
{
  //to-do
}



int delete2 (char *filename)
{
  //to-do
}



FILE2 open2 (char *filename)
{
  //to-do
}



int close2 (FILE2 handle)
{
  //to-do
}



int read2 (FILE2 handle, char *buffer, int size)
{
  //to-do
}



int write2 (FILE2 handle, char *buffer, int size)
{
  //to-do
}



int truncate2 (FILE2 handle)
{
  //to-do
}



int seek2 (FILE2 handle, DWORD offset)
{
  //to-do
}



int mkdir2 (char *pathname)
{
  //to-do
}



int rmdir2 (char *pathname)
{
  //to-do
}



DIR2 opendir2 (char *pathname)
{
  //to-do
}



int readdir2 (DIR2 handle, DIRENT2 *dentry)
{
  //to-do
}



int closedir2 (DIR2 handle)
{
  //to-do
}