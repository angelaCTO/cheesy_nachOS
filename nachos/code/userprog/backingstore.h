#ifndef BACKINGSTORE_H
#define BACKINGSTORE_H

#include "filesys.h"
#include "translate.h"

class BackingStore {
private:
  int numPages;
  char* filename;
  bool *bstore_array;
  OpenFile *executable;
  FileSystem *file;

public:
  /* Create a backing store file for an AddrSpace */
  BackingStore(int space_id, int file_size, int totalPages);

  ~BackingStore(); /* destructor*/

  /* Write the virtual page referenced by pte to the backing store */
  /* Example invocation: PageOut(&machine->pageTable[virtualPage]) or */
  /*                     PageOut(&space->pageTable[virtualPage]) */
  void PageOut(TranslationEntry *pte, int pteNum);

  /* Read the virtual page referenced by pte from the backing store */
  bool PageIn(TranslationEntry *pte, int pteNum);
};
#endif //BACKINGSTORE_H
