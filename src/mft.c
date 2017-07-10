#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEBUG false

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
  memcpy(&d->tuple, descriptor_buffer, sizeof(t2fs_4tupla)*32);
  d->MFTNumber = number;
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

      case 3: //ignore
        i++;
        break;

      default: 
        DEBUG_PRINT("Invalid MFT Attribute Type");
        i++;
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

    if(tuple.atributeType == 3) continue;
    
    sector = logicalBlock_sector(tuple.logicalBlockNumber);
    read_sector(sector, buffer);
    //for each sector, we have 4 records
    // (256 bytes [sector size] / 64 bytes [record size]) = 4
    memcpy(&record, buffer, sizeof(t2fs_record)*4);

    for(int i=0; i<4; i++) {
      if(record[i].TypeVal == TYPEVAL_DIRETORIO ||
         record[i].TypeVal == TYPEVAL_REGULAR)
        list_push_back(entries, &record[i]);
    }
  }
}

int write_descriptor(descriptor *d, List *tuples)
{
  int size = list_size(tuples);
  unsigned char buffer[256];

  if(size == 0) // empty descriptor
  {
    memset(buffer, 0, sizeof(unsigned char)*256);
    write_sector(descriptor_sector(d->MFTNumber), buffer);
    write_sector(descriptor_sector(d->MFTNumber)+1, buffer);
    return SUCCESS;
  }


  for(int i=0; i<size; i++) {
    
    // se encheu o descritor atual, temos que pegar um proximo
    if(i % 32 == 0 && i != 0) {
      // antes, salva o atual
      memcpy(buffer, &d->tuple[0], sizeof(t2fs_4tupla)*16); // only 256 per write
      write_sector(descriptor_sector(d->MFTNumber), buffer);
      memcpy(buffer, &d->tuple[16], sizeof(t2fs_4tupla)*16);
      write_sector(descriptor_sector(d->MFTNumber)+1, buffer);
      
      if(d->tuple[i].atributeType == 2) {// olha se ja tem outro encadeado
        get_descriptor(d->tuple[i].virtualBlockNumber, d);
        continue;
      }
      else {
        // se nao tem, temos que encontrar um descritor mft livre
        int mftNumber = get_free_descriptor(d);
        if(mftNumber == ERROR) return ERROR; // nao existe espaco no mft
        
        d->tuple[i].atributeType = 2; // tupla de encadeamento
        d->tuple[i].virtualBlockNumber = mftNumber;
      }
    }
    else {
      // situacao normal, so sobrescrever a tupla
      list_at(tuples, i, &d->tuple[i]);
    }
  }

  // finalmente, escreve o descritor no disco
  memcpy(buffer, &d->tuple[0], sizeof(t2fs_4tupla)*16); // only 256 per write
  write_sector(descriptor_sector(d->MFTNumber), buffer);
  memcpy(buffer, &d->tuple[16], sizeof(t2fs_4tupla)*16);
  write_sector(descriptor_sector(d->MFTNumber)+1, buffer);

  return SUCCESS;

  // MEU DEUS QUE TRABALHO HORRIVEL
}