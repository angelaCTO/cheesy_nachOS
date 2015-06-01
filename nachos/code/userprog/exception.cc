// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#define ERROR 0  

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synchconsole.h"
#include "table.h"
#include "addrspace.h"

extern Table *table;
extern SynchConsole *synchconsole;
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

// Helper function to pass into Fork()
void create_new_process(int i) {
  currentThread->space->InitRegisters();
  currentThread->space->SaveState();
  currentThread->space->RestoreState();
  machine->Run();
}

// Helper function to increase PC
void increase_program_counter() {
  machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
  machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
  machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
}

// SYSCALL EXEC
SpaceId syscall_exec() {
	int addr = machine->ReadRegister(4);
  char *filename = new char[100];
  for (int j = 0; j < 100; j++) {
    filename[j] = '\0';
  }
  int val;
  int i = 0;
  while (val != 0) {
    machine->ReadMem(addr, 1, &val);
    filename[i] = (char) val;
    i++;
    addr++;
  } 

  if(filename[i] == '\0') {
    DEBUG('a',"Filename is null terminated\n");
  }
  DEBUG('a',"filename: %s\n",filename);
  
  SpaceId space_id = 0; //if there is an error
	OpenFile *execute = fileSystem->Open(filename);

	if (execute == NULL) {
		printf("File could not be opened.\n");
	  machine->WriteRegister(2, 0);
    return 0; // Can't open file
  }

	AddrSpace *addr_space = new AddrSpace(execute);
	int success = addr_space->Initialize();
	if (success <= 0) {
		printf("Address space could not be initialized.\n");
    delete execute;
	  machine->WriteRegister(2, 0);
    return 0;
	}

	Thread *new_thread = new Thread("new thread");
	new_thread->space = addr_space;
	space_id = table->Alloc((void *) new_thread);

	new_thread->setPID(space_id);
  addr_space->spaceid = space_id;
  addr_space->InitializeBackingStore();
	// KEEP THE FILE OPEN
  //delete execute; //close file
	new_thread->Fork(create_new_process, 0);
	currentThread->Yield();
	machine->WriteRegister(2, space_id);
	return space_id;
}

// SYSCALL EXIT
void syscall_exit (int status) {
	printf("Calling syscall_exit with status: %d\n",status);
	if (table->IsEmpty()) {
		DEBUG('a',"Table is empty, calling interrupt->Halt()\n");
		interrupt->Halt();
	}
	int pid = currentThread->getPID();
	DEBUG('a',"currentThread->getPID() returns %d\n",pid);
	table->Release(pid);
	currentThread->setPID(0);
	currentThread->space->~AddrSpace(); //deleting addrspace of thread
	currentThread->Finish();
}

// HELPER FUNCTION TO READ THE BUFFER
int read_buffer(int address, char *out_buffer, int bytes)
{
  int ch;
  for (int i=0; i < bytes; i++){
    if (!machine->ReadMem(address, 1, &ch)) {
      if (!machine->ReadMem(address, 1, &ch)) {
        return 0;
      }
    }
    *out_buffer++ = (char) ch;
    address++;
   }
  return 1;
}

//SYSCALL WRITE
/** Write system call for console write.
 */
// reg2 - if entry: syscall_id; on exit: syscall_return
// reg4,5,6,7 = arg1,2,3,4
void syscall_write() {
    int addr = machine->ReadRegister(4);    
    int size = machine->ReadRegister(5);    //number of bytes to write
    int id = machine->ReadRegister(6); //file descriptor
    
    // if size to write is less than 0, ERROR
    if (size < 0) {
      printf("Can't write negative bytes!\n");
      return;
    }
    //convert id to char array
    char *buffer = new char[size];
  
    switch (id) {
        case ConsoleOutput: 
          if(!read_buffer(addr, buffer, size)) {
            printf("ReadMem has triggered a page fault!\n");
            break;
          }
          for (int i = 0; i < size; i++) {  
	          synchconsole->putChar(buffer[i]);
          }
          break;
        default:
          // if ConsoleInput or anything else, ERROR
          printf("Cannot write!\n");
          break;
    }
    delete buffer;
    return;
}

// HELPER FUNCTION TO WRITE TO BUFFER
int write_buffer(char *buff, int address, int bytes)
{
  for (int i = 0; i < bytes; i++) {
    if(!machine->WriteMem(address, 1, (int)*buff)) {  //byte by byte
      if(!machine->WriteMem(address, 1, (int)*buff)) {
        return 0;
      }
    }
    buff++;
    address++;
  }
  return 1;
}

// SYSCALL READ
int syscall_read () {
  int buffer = machine->ReadRegister(4);
  int size = machine->ReadRegister(5);
  int id = machine->ReadRegister(6);
  
  // If the size to read is less than 0, ERROR
  if (size < 0) {
    printf("Error, negative size to read.\n");
	    machine->WriteRegister(2, 0);
    return 0;
  }
  char *data = new char[size];  // max char size

  switch(id) {
    case ConsoleInput:
      for (int i = 0; i<size; i++) {
        data[i] = synchconsole->getChar();
      }
      if(!write_buffer(data, buffer, size)) {
        printf("WriteMem has triggered a page fault!\n");
        machine->WriteRegister(2, 0);
        return 0;
      }
	    machine->WriteRegister(2, size);
      break;
    default:
      printf("Cannot read!\n");
	    machine->WriteRegister(2, 0);
      return 0; 
  }
  delete data;
  return 1;
}


void ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);
  switch (which) {
    case SyscallException:
		  switch(type) {
			  case SC_Exec:
          syscall_exec();
          increase_program_counter();
          break;
			  case SC_Exit:
          syscall_exit(machine->ReadRegister(4));
          increase_program_counter();
          break;
			  case SC_Open:
			  	break;
			  case SC_Read:
          syscall_read();
          increase_program_counter();
				  break;
			  case SC_Write:
				  syscall_write();
          increase_program_counter();
				  break;
			  case SC_Close:
				  break;
			  case SC_Halt:
				  DEBUG('a', "Shutdown, initiated by user program.\n");
				  interrupt->Halt();
				  break;
			  default:
				  printf("Unexpected syscall  %d\n", type);
				  ASSERT(false);
				  break;
		  }
      break;
    case PageFaultException: {
      //PROJECT 3: prepare requested page on demand
      stats->numPageFaults++; //increment page fault counter
      int fault_addr = machine->ReadRegister(39); // BadVAddReg is 39 from machine.h
      int page_num = fault_addr / PageSize;
      printf("\n------------------------------\n");
      printf("Page number: %d\n", page_num);
      currentThread->space->PageFaultHandler(page_num, currentThread->space->executableFile); // something like this
      currentThread->space->pageTable[page_num].valid = TRUE;
      printf("******************************\nStats Output:\n");
      stats->Print();
      printf("==============================\n");
    }
      break;
    case ReadOnlyException:
      printf("Write attempted to page marked \"read-only\"\n");
      syscall_exit(machine->ReadRegister(4));
      increase_program_counter();
      break;
    case BusErrorException:
      printf("Translation resulted in an invalid physical address\n");
      syscall_exit(machine->ReadRegister(4));
      increase_program_counter();
      break;
		case AddressErrorException:
      printf("Unaligned reference or one that was beyond the end of the address space\n");
      syscall_exit(machine->ReadRegister(4));
      increase_program_counter();
      break;
		case OverflowException:
      printf("Integer overflow in add or sub\n");
      syscall_exit(machine->ReadRegister(4));
      increase_program_counter();
      break;
		case IllegalInstrException:
      printf("Unimplemented or reserved instruction\n");
      syscall_exit(machine->ReadRegister(4));
      increase_program_counter();
      break;
		case NumExceptionTypes:	
      syscall_exit(machine->ReadRegister(4));
      increase_program_counter();
      break;
		case NoException:   // Apparently this is not needed
      printf("Everything ok!\n");
      break;
		default:
			printf("Unexpected user mode exception %d\n", which);
      break;
	}
}

