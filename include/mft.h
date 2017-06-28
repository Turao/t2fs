#ifndef __MFT_H
#define __MFT_H

#include <stdbool.h>

#include "t2fs.h" //mft 4-tuple structure
#include "apidisk.h" //read/write sectors

#define DESCRIPTOR_SIZE 512

typedef struct descriptor {
  // (mft size: 512 bytes / 4-tuple size: 16 bytes) = 32 tuples
  t2fs_4tupla tuple[32];
} descriptor;


descriptor _bitmap_d;
descriptor _root_d;
// descriptor reserved;
// descriptor reserved;

bool init_mft_info();
int descriptor_sector(int descriptor);
void read_descriptor(int number, unsigned char buffer[]);

#endif //__MFT_H