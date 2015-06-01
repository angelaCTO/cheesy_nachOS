#include "backingstore.h"
#include "system.h"

#include <string>
#include <sstream>
#include <iostream>

/* Create a backing store file for an AddrSpace */
BackingStore::BackingStore(int space_id, int file_size, int totalPages) {
  numPages = totalPages;
  bstore_array = new bool[numPages];
  file = new FileSystem(FALSE); //OR FALSE IDK

  std::stringstream ss;
  ss << space_id;
  std::string str = ss.str();

  std::string name = std::string("bstore") + str; 
  filename = new char[100];
  strcpy(filename, name.c_str());

  file->Create(filename, file_size);
  executable = file->Open(filename);
}
/* Destructor */
BackingStore::~BackingStore() {
  file->Remove(filename);
  delete filename;
  delete bstore_array;
  delete file;
  delete executable;
}

/* Write the virtual page referenced by pte to the backing store */
/* Example invocation: PageOut(&machine->pageTable[virtualPage]) or */
/*                     PageOut(&space->pageTable[virtualPage]) */
void BackingStore::PageOut(TranslationEntry *pte, int pteNum) {
  executable->WriteAt((&(machine->mainMemory[(pte->physicalPage * PageSize)])), PageSize, pte->virtualPage * PageSize);
  stats->numPageOuts++; //increment counter
  bstore_array[pteNum] = true; //set bstore_array to true
}

/* Read the virtual page referenced by pte from the backing store */
bool BackingStore::PageIn(TranslationEntry *pte, int pteNum) {
  if(bstore_array[pteNum]) {
    executable->ReadAt((&(machine->mainMemory[(pte->physicalPage * PageSize)])), PageSize, pte->virtualPage * PageSize);
    stats->numPageIns++; //increment counter
  }
  return bstore_array[pteNum];
}
