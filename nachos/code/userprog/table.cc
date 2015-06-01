#include "table.h"

Table::Table(int size) {
  lock = new Lock("Table Lock");
  tableSize = size;
  allocated = (void **)new int[size];
  for(int i = 0; i < tableSize; i++) {
    allocated[i] = NULL;
  }
}

/* Destructor*/
Table::~Table() {
  //delete elements in array before deleting array
  for(int i = 0; i <= tableSize; i++) {
    delete (int *)allocated[i];
  }
  delete lock;
  delete allocated;
}

/* Allocate a table slot for "object", returning the "index" of the
 * allocated entry; otherwise, return -1 if no free slots are available. */
int Table::Alloc(void *object) {
  lock->Acquire();
  int id = -1;
  for (int i = 0; i < tableSize; i++) {
    if (allocated[i] == NULL) {
      allocated[i] = object;
      id = i;
      break;
    }
  }
  lock->Release();
  return id;
}

/* Retrieve the object from table slot at "index", or NULL if that
 * slot has not been allocated. */
void* Table::Get(int index) {
  lock->Acquire();
  void *obj = NULL;
  if(index >= 0 && index < tableSize) {
    obj = allocated[index];
  }
  lock->Release();
  return obj;
}

/* Free the table slot at index. */
void Table::Release(int index) {
  lock->Acquire();
  if(index >= 0 && index < tableSize) {
    allocated[index-1] = NULL;
  }
  lock->Release();
}

bool Table::IsEmpty() {
  lock->Acquire();
  bool empty = true;
  for(int i = 0; i < tableSize; i++) {
    if(allocated[i] != NULL) {
      empty = false;
    }
  }
  lock->Release();
  return empty;
}
