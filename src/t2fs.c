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



int identify2 (char *name, int size)
{
  if(!disk_info_initialized) init_t2fs_bootBlock();
  if(!mft_info_initialized) init_mft_info();

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