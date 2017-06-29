#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEBUG true

#include "mft.h"
#include "boot.h"
#include "utils.h"


bool init_mft_info()
{
  DEBUG_PRINT("Initializing mft info \n");
  unsigned char descriptor_buffer[512];

  read_descriptor(0, descriptor_buffer);
  memcpy(&_bitmap_d, descriptor_buffer, sizeof(descriptor));

  read_descriptor(1, descriptor_buffer);
  memcpy(&_root_d, descriptor_buffer, sizeof(descriptor));

  printf("\n");
  return true;
}


int descriptor_sector(int descriptor)
{ 
  return _bootBlock.blockSize + //skips boot sector
  		 ((DESCRIPTOR_SIZE * descriptor) / SECTOR_SIZE); 
}


void read_descriptor(int number, unsigned char buffer[])
{
  int sector = descriptor_sector(number);
  if(read_sector(sector, buffer) != SUCCESS ||
     read_sector(sector+1, &buffer[256]) != SUCCESS) {
      DEBUG_PRINT("Couldn't read descriptor %d [sector %d]", number, sector);
  }
  else DEBUG_PRINT("Descriptor %d loaded from sectors [%d, %d]", number, sector, sector+1);
}