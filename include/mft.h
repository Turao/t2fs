#ifndef __MFT_H
#define __MFT_H

#include <stdbool.h>

#include "t2fs.h" //mft 4-tuple structure
#include "apidisk.h" //read/write sectors

#include "list.h" //descriptor_tuples

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

/*
[input]
  number: mft descriptor number
[output] 
  descriptor: the correspondent descriptor
*/
void get_descriptor(int number, descriptor *descriptor);


/* returns the descriptor sector in the disk */
int descriptor_sector(int descriptor);

/* stores the descriptor data in a buffer */
void read_descriptor(int number, unsigned char buffer[]);


/*
[input]
  d: mft descriptor
[output] 
  tuples: a list with all valid mapped vnb-lbn 4-tuples 
          taken from the given descriptor
*/
void descriptor_tuples(descriptor d, List *tuples);

#endif //__MFT_H