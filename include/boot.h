#ifndef __BOOT_H
#define __BOOT_H

#include <stdbool.h>

#include "t2fs.h" //boot block structure
#include "apidisk.h" //read boot sector

t2fs_bootBlock _bootBlock;

bool init_t2fs_bootBlock();

int logicalBlock_sector(int logicalBlockNumber);

#endif // __BOOT_H