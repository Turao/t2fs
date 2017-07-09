#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEBUG true

#include "mft.h"
#include "t2fs.h"
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


int get_free_descriptor(descriptor *descriptor)
{
  int mftSize = (_bootBlock.MFTBlocksSize * _bootBlock.blockSize);
  int totalNumberOfDescriptors = mftSize/sizeof(descriptor);
  for(int i=0; i<totalNumberOfDescriptors; i++) {
    get_descriptor(i, descriptor);
    if(descriptor->tuple[0].atributeType == -1) // FREE MFT DESCRIPTOR
      return i;
  }
  return -1;
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


void descriptorEntries(descriptor d, List *entries)
{
  int sector;
  unsigned char buffer[256];
  t2fs_record record[4]; //records per sector

  List valid_tuples;
  list_new(&valid_tuples, sizeof(t2fs_4tupla), free);
  descriptor_tuples(d, &valid_tuples);

  for(int i=0; i<list_size(&valid_tuples); i++) {
    t2fs_4tupla tuple; //iterator
    list_at(&valid_tuples, i, &tuple);

    sector = logicalBlock_sector(tuple.logicalBlockNumber);
    // printf("\n tuple %d: \n", i);
    // printf("\t logical block %d", tuple.logicalBlockNumber);
    // printf("\t [sector %d] \n", sector);
    // printf("\t virtual block %d \n", tuple.virtualBlockNumber);
    // printf("\t # contiguous blocks %d \n", tuple.numberOfContiguosBlocks);
    // printf("\n");

    read_sector(sector, buffer);
    //for each sector, we have 4 records
    // (256 bytes [sector size] / 64 bytes [record size]) = 4
    memcpy(&record, buffer, sizeof(t2fs_record)*4);

    for(int i=0; i<4; i++) {
      if(record[i].TypeVal != TYPEVAL_INVALIDO)
        list_push_back(entries, &record[i]);
    }
  }
}