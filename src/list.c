#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
 
#include "list.h"

 
bool list_new(List *list, size_t elementSize, freeFunction freeFn)
{
  if(list == NULL) return false;
  if(elementSize <= 0) return false;

  list->logicalLength = 0;
  list->elementSize = elementSize;
  list->head = list->tail = NULL;
  list->freeFn = freeFn;

  return true;
}





// ****************************************************************************
// ****************************************************************************
// Capacity
uint64_t list_size(List *list)
{
  return list->logicalLength;
}


bool list_empty(List *list)
{
  return list->logicalLength == 0 ? true : false;
}





// ****************************************************************************
// ****************************************************************************
// Element access
void list_front(List *list, void *element)
{
  assert(list->head != NULL);
  ListNode *node = list->head;
  memcpy(element, node->data, list->elementSize);
}
 
void list_back(List *list, void *element)
{
  assert(list->tail != NULL);
  ListNode *node = list->tail;
  memcpy(element, node->data, list->elementSize);
}

void list_at(List *list, uint64_t position, void *element)
{
  assert(list->logicalLength > position);

  uint64_t i = 0;
  ListNode *node = list->head;
  while(node != NULL && i < position) {
    node = node->next;
    i++;
  }
  memcpy(element, node->data, list->elementSize);
}





// ****************************************************************************
// ****************************************************************************
// Modifiers
void list_insert(List *list, void *element, insertFunction insertFn)
{
  // TO-DO
  assert(insertFn != NULL);
 
  ListNode *iterator = list->head;
  while(iterator != NULL && !insertFn(iterator->data)) {
    iterator = iterator->next;
  }

  if(iterator == NULL) {
    list_push_back(list, element);
  }
  else if(iterator == list->head) { // first node?
    list_push_front(list, element);
  }
  else if(iterator == list->tail) { // last node?
    list_push_back(list, element);
  }
  else
  {
    ListNode *node = calloc(1, sizeof(ListNode));
    node->data = calloc(1, list->elementSize);
    memcpy(node->data, element, list->elementSize);

    // generic case
    iterator->prev->next = node;
    iterator->prev = node;
  }
}

void list_push_front(List *list, void *element)
{
  ListNode *node = calloc(1, sizeof(ListNode));
  node->data = calloc(1, list->elementSize);
  memcpy(node->data, element, list->elementSize);
  node->next = list->head;
  node->prev = NULL;


  // first node?
  if(list->logicalLength == 0) {
    list->tail = node;
  }
  else {
    list->head->prev = node;
  }
  list->head = node;

  list->logicalLength++;
}



void list_push_back(List *list, void *element)
{
  ListNode *node = calloc(1, sizeof(ListNode));
  node->data = calloc(1, list->elementSize);
  node->next = NULL;
  node->prev = list->tail;

  memcpy(node->data, element, list->elementSize);

  if(list->logicalLength == 0) {
    list->head = list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }

  list->logicalLength++;
}

void* list_pop_front(List *list)
{
  assert(list->head != NULL);
 
  ListNode *node = list->head;
  list->head = node->next;
  if(node->next != NULL) node->next->prev = NULL;
  list->logicalLength--;

  if(list->logicalLength == 0) list->tail = NULL;
  
  void* data = node->data;
  free(node);

  return data;
}


void* list_pop_back(List *list)
{
  assert(list->tail != NULL);

  ListNode *node = list->tail;
  list->tail = node->prev;
  if(node->prev != NULL) node->prev->next = NULL;
  list->logicalLength--;

  if(list->logicalLength == 0) list->head = NULL;

  void* data = node->data;
  free(node);

  return data;
}

void list_destroy(List *list)
{
  ListNode *current;
  while(list->head != NULL) {
    current = list->head;
    list->head = current->next;

    if(list->freeFn) {
      list->freeFn(current->data);
    }
    list->logicalLength--;
    free(current);
  }
}





// *************************************************
// Operations
void* list_remove(List *list, compfunction compFn, void *element)
{
  assert(list != NULL);

  void *data = NULL;

  ListNode *node = list->head;
  while(node != NULL) {
    if(compFn(node->data, element)) {
      data = node->data; //found

      //head
      if(node == list->head) list->head = node->next;

      else if(node->prev != NULL) //!head, has prev
        node->prev->next = node->next;

      //tail
      if(node == list->tail) list->tail = node->prev;
      else if(node->next != NULL) //!tail, has next
        node->next->prev = node->prev;

      list->logicalLength--;

      free(node);
      node = NULL;
    }
    else node = node->next;
  }
  return data;
}




// *************************************************
// Others
void list_for_each(List *list, listIterator iterator)
{
  assert(iterator != NULL);
 
  ListNode *node = list->head;
  bool result = true;
  while(node != NULL && result) {
    result = iterator(node->data);
    node = node->next;
  }
}




void* list_find(List *list, compfunction compFn, void* element)
{ 
  assert(list != NULL);
  ListNode *node = list->head;
  while(node != NULL) {
    if(compFn(node->data, element)) return node->data;
    node = node->next;
  }
  return NULL;
}
