// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "sysdep.h"
#include <cmath>
#ifdef HOST_SPARC
#include <strings.h>
#endif
extern MemoryManager *manager;
//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *exec) {
  executableFile = exec;
  spaceid = -1;
}


//----------------------------------------------------------------------
// void AddrSpace::Initialize(OpenFile *executable)
//  Loading code
//----------------------------------------------------------------------
int AddrSpace::Initialize() {
	return Initialize(FALSE);
}

int AddrSpace::Initialize(bool readOnly) {
    NoffHeader noffH;
    unsigned int i;

    executableFile->ReadAt((char *)&noffH, sizeof(noffH), 0);
    //int file_length = executableFile->Length();
    //printf("Length of executable file: %d\n", file_length);

    noffHeader = noffH; // STUFF IDK WHAT IM DOING
    if ((noffH.noffMagic != NOFFMAGIC) &&
            (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    // how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
           + UserStackSize; // we need to increase the size
    // to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    printf("Number of Pages: %d\n", numPages);
    printf("Number of PhysPages: %d\n", NumPhysPages);

    /** PROJECT 3: set aside  (3) prevents programs that require too much memory from proceeding
    if ((int)numPages > (manager->NumFreePages())) {
      printf("Not enough memory!\n");
      return -1;
    }
    */

    //ASSERT(numPages <= NumPhysPages);       // check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory

    printf("Initializing address space, num pages %d, size %d\n",
          numPages, size);
    // first, set up the translation
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
      pageTable[i].virtualPage = i;   // for now, virtual page # = phys page #
//      pageTable[i].physicalPage = -1; 
      //pageTable[i].physicalPage = manager->AllocPage(); //remove for PROJECT 3
		  // okay, apparently we can't alloc all the pages we wanted
		  // release all the pages we currently alloced for this, and error out
/** COMMENT OUT FOR PART 3
      if (pageTable[i].physicalPage == -1){
			  for (int inverse = i-1; inverse >=0; inverse--){
				  manager->FreePage(pageTable[inverse].physicalPage);
			  }
			  DEBUG('a',"file too big to alloc, freed progress and errored out");
			  return -1;
		  } */
      pageTable[i].valid = FALSE;
      pageTable[i].use = FALSE;
      pageTable[i].dirty = FALSE;
      pageTable[i].readOnly = readOnly;  // if the code segment was entirely on
      // a separate page, we could set its
      // pages to be read-only
    }
    //virt_page_num = (unsigned) noffH.code.virtualAddr / PageSize;

    /** PROJECT 3: set aside the code that (1) zeros out the physical page frames
     *
    // zero out the entire address space, to zero the unitialized data segment
    // and the stack segment
    for(i = 0; i < numPages; i++) {
      bzero(&machine->mainMemory[pageTable[i].physicalPage * PageSize], PageSize);
    }
    */

    /** PROJECT 3: set aside the code that (2) preloads the address space with the code and data segments from the file
    int codeOffset, codeVirtualAddr, codeSize, codeSizeLoaded;
    int dataOffset, dataVirtualAddr, dataSize, dataSizeLoaded;
    //load in code segments
    if (noffH.code.size > 0) {
      codeSizeLoaded = noffH.code.size;
      codeOffset = noffH.code.inFileAddr;
      codeVirtualAddr = noffH.code.virtualAddr;
      codeSize = 0;
      if ((noffH.code.virtualAddr % PageSize) != 0) {
		    codeSize = PageSize-noffH.code.virtualAddr%PageSize;
        //reset
	      if (codeSize > codeSizeLoaded) {
            codeSize = codeSizeLoaded;
        }
		    codeSizeLoaded = noffH.code.size-codeSize;		
      	executable->ReadAt(&(machine->mainMemory[Translate(codeVirtualAddr)]), codeSize, codeOffset);
		  }

  	  //if address within boundary 
      while (codeSizeLoaded >= PageSize) {
		    codeOffset = codeOffset+codeSize;
		    codeVirtualAddr = codeVirtualAddr+codeSize;
		    codeSize = PageSize;		
		    executable->ReadAt(&(machine->mainMemory[Translate(codeVirtualAddr)]), codeSize, codeOffset);
        codeSizeLoaded = codeSizeLoaded-codeSize;	
      }	
	   
  	  //if address not within boundary
      if (codeSizeLoaded > 0) {
		    codeOffset = codeOffset+codeSize;
		    codeVirtualAddr = codeVirtualAddr+codeSize;
		    codeSize = codeSizeLoaded;				
		    executable->ReadAt(&(machine->mainMemory[Translate(codeVirtualAddr)]), codeSize, codeOffset);
	    }                
    }

	  //load data segments
    if (noffH.initData.size > 0) {
      dataSizeLoaded = noffH.initData.size;
      dataSize = 0;
      dataOffset = noffH.initData.inFileAddr;
      dataVirtualAddr = noffH.initData.virtualAddr;

      if ((noffH.initData.virtualAddr%PageSize) != 0) {    	     
		    dataSize = PageSize-noffH.initData.virtualAddr%PageSize;
        //reset
	      if (dataSize>dataSizeLoaded) {
          dataSize = dataSizeLoaded;
        }
    		dataSizeLoaded = noffH.initData.size-dataSize;		
		    executable->ReadAt(&(machine->mainMemory[Translate(dataVirtualAddr)]), dataSize, dataOffset);
		  }

  	  //if address within boundary 
      while (dataSizeLoaded >= PageSize) {
		    dataOffset = dataOffset+dataSize;
    		dataVirtualAddr = dataVirtualAddr+dataSize;
	    	dataSize = PageSize;		
		    executable->ReadAt(&(machine->mainMemory[Translate(dataVirtualAddr)]), dataSize, dataOffset);
        dataSizeLoaded = dataSizeLoaded-dataSize;	
	    }	
	   
  	  //if address not within boundary
      if (dataSizeLoaded > 0) {
    		dataOffset = dataOffset+dataSize;
		    dataVirtualAddr = dataVirtualAddr+dataSize;
		    dataSize = dataSizeLoaded;
		    executable->ReadAt(&(machine->mainMemory[Translate(dataVirtualAddr)]), dataSize, dataOffset);
    	}         
  }
    */

  return true;
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
  for(unsigned int i=0; i < numPages; i++) {
    if(pageTable[i].valid == TRUE) {
      manager->FreePage(pageTable[i].physicalPage);
    }
  }
  delete [] pageTable;
  delete executableFile;
  delete store;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

//----------------------------------------------------------------------
// AddrSpace::InitializeBackingStore
// Helper function to initialize backing store
//----------------------------------------------------------------------

void AddrSpace::InitializeBackingStore() {
  store = new BackingStore(spaceid, size, numPages);
}

//----------------------------------------------------------------------
// AddrSpace::Translate
// Helper function to convert virtual address 
//----------------------------------------------------------------------

int AddrSpace::Translate(int virtAddr) {
    int physAddr;
    unsigned vpn,offset;
    vpn = (unsigned) virtAddr / PageSize;
    offset = (unsigned) virtAddr % PageSize;
    physAddr = pageTable[vpn].physicalPage * PageSize + offset;
    return physAddr;
}

//----------------------------------------------------------------------
// AddrSpace::PageFaultHandler
// Helper function to page in the faulted page from the executable
// file. Takes in the page number you want to page in and returns 0 or 1
// on failure or success.
//----------------------------------------------------------------------

void AddrSpace::PageFaultHandler(int pageNumber, OpenFile *executable) {
  // allocate a pageframe "on the fly"
  pageTable[pageNumber].physicalPage = manager->AllocPage();
  printf("physical page is: %d\n", pageTable[pageNumber].physicalPage); 
  if (pageTable[pageNumber].physicalPage == -1) {
    int evictedPageNum = Random() % numPages;
    //get new random page if chosen PTE is not valid
    while(pageTable[evictedPageNum].valid == FALSE) {
      evictedPageNum = Random() % numPages;
    }
    printf("Evicting Page %d\n", evictedPageNum);

    if (pageTable[evictedPageNum].dirty == TRUE) {
      // KERNEL MUST SAVE PAGE CONTENTS IN BACKING STORE ON DISK
      store->PageOut(&machine->pageTable[evictedPageNum], evictedPageNum);
    }

    //reset evicted page
    //mark page as invalid
    pageTable[evictedPageNum].valid = FALSE;
    // set dirty bit to false
    pageTable[evictedPageNum].dirty = FALSE;

    manager->FreePage(pageTable[evictedPageNum].physicalPage);

    pageTable[pageNumber].physicalPage = manager->AllocPage();
    printf("Physical Page Being Allocated: %d\n", pageTable[pageNumber].physicalPage);
  }

  // CHECK IF PAGE IS IN BACKING STORE, IF IT IS LOAD FROM BACKING STORE
  bool successfulPageIn = store->PageIn(&machine->pageTable[pageNumber], pageNumber);

  //ELSE LOAD FROM CODE/DATA
  if(!successfulPageIn) {
    int codePageStart = (noffHeader.code.virtualAddr)/PageSize;
    int dataPageStart = (noffHeader.initData.virtualAddr)/PageSize;
    int codePageEnd = (noffHeader.code.virtualAddr + noffHeader.code.size)/PageSize;
    int dataPageEnd = (noffHeader.initData.virtualAddr + noffHeader.initData.size)/PageSize;
    bool segmentsLoaded = false;
    bool counterIncremented = false; 

    if(codePageEnd >= pageNumber && pageNumber >= codePageStart) {
      segmentsLoaded = true;
      stats->numPageIns++; //increment counter
      counterIncremented = true;
      //loading code segment
      if (noffHeader.code.size > 0) {
        //middle page
        if (noffHeader.code.virtualAddr/PageSize == pageNumber){
          executable->ReadAt(&(machine->mainMemory[Translate(noffHeader.code.virtualAddr)]), 
                            PageSize-noffHeader.code.virtualAddr % PageSize,
                            noffHeader.code.inFileAddr);
        }
        else if (codePageEnd > pageNumber){
          executable->ReadAt(&(machine->mainMemory[Translate(PageSize*pageNumber)]),
                            PageSize,
                            noffHeader.code.inFileAddr+(PageSize*pageNumber)-noffHeader.code.virtualAddr);
        }
        else if (codePageEnd == pageNumber){
          executable->ReadAt(&(machine->mainMemory[Translate(PageSize*pageNumber)]),
                            (noffHeader.code.virtualAddr+noffHeader.code.size)%PageSize,
                            noffHeader.code.inFileAddr+(PageSize*pageNumber)-noffHeader.code.virtualAddr);
        }
      }
    }

    if(dataPageEnd >= pageNumber && pageNumber >= dataPageStart) {
      segmentsLoaded = true;
      if(!counterIncremented) {
        stats->numPageIns++; //increment counter
        counterIncremented = true;
      }
      //load data segment
      if (noffHeader.initData.size > 0) {
        //middle page
        if (noffHeader.initData.virtualAddr/PageSize == pageNumber){
          executable->ReadAt(&(machine->mainMemory[Translate(noffHeader.initData.virtualAddr)]),
                            PageSize-noffHeader.initData.virtualAddr%PageSize,
                            noffHeader.initData.inFileAddr);
        }
        else if (dataPageEnd > pageNumber){
          executable->ReadAt(&(machine->mainMemory[Translate(PageSize*pageNumber)]), 
                            PageSize,
                            noffHeader.initData.inFileAddr+(PageSize*pageNumber)-noffHeader.initData.virtualAddr);
        }
        else if (dataPageEnd == pageNumber){
          executable->ReadAt(&(machine->mainMemory[Translate(PageSize*pageNumber)]),
                            (noffHeader.initData.virtualAddr+noffHeader.initData.size)%PageSize,
                            noffHeader.initData.inFileAddr+(PageSize*pageNumber)-noffHeader.initData.virtualAddr);
        }
      }
    }

    //zero fill 
    if(!segmentsLoaded) {
      bzero(&machine->mainMemory[pageTable[pageNumber].physicalPage * PageSize], PageSize);
    }
  }
}
