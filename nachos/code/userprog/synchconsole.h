#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "synch.h"
#include "console.h"

class SynchConsole {
private:
  Console* console;
  Semaphore* readAvail;
  Semaphore* writeDone;
  Lock* readLock;
  Lock* writeLock;

public:
  SynchConsole(char* input, char* output); 

  ~SynchConsole(); //destructor

  void WriteDone();
  void ReadAvail();
  void putChar(char c);
  char getChar();
};
#endif //SYNCHCONSOLE_H
