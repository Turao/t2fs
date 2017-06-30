#ifndef __LIST_H
#define __LIST_H

#include <stdbool.h>
#include <stdint.h> // getting rid of machine-dependent type sizes


// inspired by:
// http://pseudomuto.com/development/2013/05/02/implementing-a-generic-linked-list-in-c/
// adapted to a doubly-linked list
// and some nifty tricks
// like taking a 'insert' function to tell us when we need to insert the data

// a common function used to free malloc'd objects
typedef void (*freeFunction)(void *);
  
typedef bool (*listIterator)(void *);

typedef bool (*insertFunction)(void *);

typedef bool (*compfunction)(void *, void *);
 
typedef struct _ListNode {
  void *data;
  struct _ListNode *next, *prev;
} ListNode;
 
typedef struct _List {
  uint64_t logicalLength;
  size_t elementSize; // "an integer capable of holding the largest array index"
  ListNode *head;
  ListNode *tail;
  freeFunction freeFn;
} List;
 

// *************************************************
bool list_new(List *list, size_t elementSize, freeFunction freeFn);


// *************************************************
// Capacity
uint64_t list_size(List *list);
bool list_empty(List *list);


// *************************************************
// Element access
void list_front(List *list, void *element);
void list_back(List *list, void *element);
void list_at(List *list, uint64_t position, void *element);


// *************************************************
// Modifiers
void list_insert(List *list, void *element, insertFunction insertFn);

void list_push_front(List *list, void *element);
void list_push_back(List *list, void *element);

void* list_pop_front(List *list);
void* list_pop_back(List *list);

void list_destroy(List *list);



// *************************************************
// Operations
void* list_remove(List *list, compfunction compFn, void *element);



void list_for_each(List *list, listIterator iterator); //not sure if i'll need something like that
void* list_find(List *list, compfunction compFn, void* element);


// bool swap(TCB_l *a, TCB_l *b); // may not need it

 
#endif