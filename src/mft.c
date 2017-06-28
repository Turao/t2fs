#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mft.h"
#include "utils.h"


bool init_mft_info()
{
  DEBUG_PRINT("Initializing mft info \n");
  unsigned char descriptor_buffer[512];

  read_descriptor(0, descriptor_buffer);
  memcpy(&_bitmap_d, descriptor_buffer, sizeof(descriptor));

  read_descriptor(1, descriptor_buffer);
  memcpy(&_root_d, descriptor_buffer, sizeof(descriptor));

  printf("descriptor root: \n");
  printf("\t tuple 0: \n");
  printf("\t\t attr: %x \n", _root_d.tuple[0].atributeType);
  printf("\t\t virtual block number: %x \n", _root_d.tuple[0].virtualBlockNumber);
  printf("\t\t logical block number: %x \n", _root_d.tuple[0].logicalBlockNumber);
  printf("\t\t number of contiguous blocks: %x \n", _root_d.tuple[0].numberOfContiguosBlocks);

  return true;
}


int descriptor_sector(int descriptor) 
{ 
  return 1 + ((DESCRIPTOR_SIZE * descriptor) / SECTOR_SIZE); //skip boot sector 
}


void read_descriptor(int number, unsigned char buffer[])
{
  int sector = descriptor_sector(number);
  if(read_sector(sector, buffer) == ERROR ||
     read_sector(sector+1, &buffer[256]) == ERROR) {
      DEBUG_PRINT("Couldn't read descriptor %d [sector %d]", number, sector);
  }
  else DEBUG_PRINT("Descriptor %d loaded from sectors [%d, %d]", number, sector, sector+1);


  for(int i=0; i<512; i++) {
    if(i%16 == 0) printf(" ");
    printf("%x", buffer[i]);
  }
  printf("\n");
}