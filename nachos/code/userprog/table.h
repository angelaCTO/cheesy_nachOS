#ifndef TABLE_H
#define TABLE_H

#include "synch.h"
#include "list.h"

class Table {
private:
    Lock *lock;
    int tableSize;
    void **allocated;

public:
  /* Create a table to hold at most "size" entries. */
  Table(int size);

  /* Destructor */
  ~Table();

  /* Allocate a table slot for "object", returning the "index" of the
  *  allocated entry; otherwise, return -1 if no free slots are available. */
  int Alloc(void *object);

  /* Retrieve the object from table slot at "index", or NULL if that
  * slot has not been allocated. */
  void *Get(int index);

  /* Free the table slot at index. */
  void Release(int index);

  bool IsEmpty();
};

#endif //TABLE_H
