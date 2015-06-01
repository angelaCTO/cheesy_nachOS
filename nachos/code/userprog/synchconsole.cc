#include "synchconsole.h"

// SynchConsole *sc = new SynchConsole(/*what is (in, out) ?*/);

static void SynchReadAvail(int c) {
  SynchConsole *sc = (SynchConsole *)c;
  sc->ReadAvail();
}

static void SynchWriteDone(int c) {
  SynchConsole *sc = (SynchConsole *)c;
  sc->WriteDone();
}

SynchConsole::SynchConsole(char* input, char* output) {
  console = new Console(NULL, NULL, (VoidFunctionPtr)SynchReadAvail, 
                        (VoidFunctionPtr)SynchWriteDone, (int)this);
  readAvail = new Semaphore("readAvail", 0);
  writeDone = new Semaphore("writeDone", 0);
  readLock = new Lock("readLock");
  writeLock = new Lock("writeLock");
} 

//destructor
SynchConsole::~SynchConsole() {
  delete readAvail;
  delete writeDone;
  delete readLock;
  delete writeLock;
  delete console;
}

void SynchConsole::ReadAvail() {
  readAvail->V();
}

void SynchConsole::WriteDone() {
  writeDone->V();
}

char SynchConsole::getChar() {
  char value;
  readLock->Acquire();
  readAvail->P();
  value = console->GetChar();
  readLock->Release();
  return value;
}

void SynchConsole::putChar(char c) {
  writeLock->Acquire();
  console->PutChar(c);
  writeDone->P();
  writeLock->Release();
}
