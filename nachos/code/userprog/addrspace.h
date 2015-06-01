// addrspace.h
//	Data structures to keep track of executing user programs
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "memorymanager.h"
#include "noff.h"
#include "backingstore.h"

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
    //unsigned int virt_page_num; // get the page number
    AddrSpace(OpenFile* exec);	// Create an address space,
    // initializing it with the program
    // stored in the file "executable"

    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
    // before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    int Initialize();
    int Initialize(bool readOnly);
    void InitializeBackingStore();
    int Translate(int virtAddr); //helper to translate
    void PageFaultHandler(int pageNumber, OpenFile *executable); // Pages in faulted pages from executable file
    OpenFile *executableFile; //pointer to open executable file
    TranslationEntry *pageTable;	// Assume linear page table translation
    int spaceid;
    unsigned int size;
    BackingStore *store;

private:
    NoffHeader noffHeader; //pointer to NOFF header
    // for now!
    unsigned int numPages;		// Number of pages in the virtual
    // address space
};

#endif // ADDRSPACE_H
