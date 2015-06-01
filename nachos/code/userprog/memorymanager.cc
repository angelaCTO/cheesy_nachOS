#include "memorymanager.h"

/* Create a manager to track the allocation of numPages of physical memory.  
 * You will create one by calling the constructor with NumPhysPages as
 * the parameter.  All physical pages start as free, unallocated pages. */
MemoryManager::MemoryManager(int numPages) {
	bitMap = new BitMap(numPages);
  memory_lock = new Lock("memory_lock");
  pageCount = numPages;
}

MemoryManager::~MemoryManager() {
  delete bitMap;
  delete memory_lock;
}

/* Allocate a free page, returning its physical page number or -1
 * if there are no free pages available. */
int MemoryManager::AllocPage() {
  memory_lock->Acquire();
  int pageNum = bitMap->Find();
  if (pageNum > 0) {
    bitMap->Mark(pageNum);
  }
  memory_lock->Release();
  return pageNum;
}

/* Free the physical page and make it available for future allocation. */
void MemoryManager::FreePage(int physPageNum) {
  memory_lock->Acquire();
  bitMap->Clear(physPageNum);
  memory_lock->Release();
}

/* True if the physical page is allocated, false otherwise. */
bool MemoryManager::PageIsAllocated(int physPageNum) {
  memory_lock->Acquire();
  bool isAllocated = bitMap->Test(physPageNum);
  memory_lock->Release();
  return isAllocated;
}

/* Number of pages that can be used */
int MemoryManager::NumFreePages() {
  memory_lock->Acquire();
  int num_count = bitMap->NumClear();
  memory_lock->Release();
  return num_count;
}
