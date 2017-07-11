#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEBUG false

#include "boot.h"
#include "utils.h"



/* Reads the information within the boot block
*  and initializes its associated global structure
*
*  Author: Arthur Lenz
*/
bool init_t2fs_bootBlock()
{
  DEBUG_PRINT("Initializing boot block info \n");
  unsigned char boot_sector[SECTOR_SIZE];
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



/* Given a logical block number, outputs its initial
*  sector
*
*  Author: Arthur Lenz
*/
int logicalBlock_sector(int logicalBlockNumber)
{
  return logicalBlockNumber * _bootBlock.blockSize;
}