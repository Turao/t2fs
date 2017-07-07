#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEBUG true

#include "mft.h"
#include "boot.h"
#include "list.h"
#include "utils.h"


bool init_mft_info()
{
  DEBUG_PRINT("Initializing mft info \n");
  get_descriptor(0, &_bitmap_d);
  get_descriptor(1, &_root_d);

  cwdDescriptor = _root_d;

  return true;
}


void get_descriptor(int number, descriptor *d)
{
  unsigned char descriptor_buffer[512];
  read_descriptor(number, descriptor_buffer);
  memcpy(d, descriptor_buffer, sizeof(descriptor));
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



void descriptor_tuples(descriptor d, List *tuples)
{
  int i = 0;
  while(i < 32 && d.tuple[i].atributeType > 0) {
    switch(d.tuple[i].atributeType)  {
      case 1: //mapped vnb-lbn 4-tuple
        list_push_back(tuples, &d.tuple[i]);
        i++;
        break;

      case 2: //additional mft descriptor
        get_descriptor(d.tuple[i].virtualBlockNumber, &d);
        i = 0;
        break;

      default: DEBUG_PRINT("Invalid MFT Attribute Type");
    }
  }
}