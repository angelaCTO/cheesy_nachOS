// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"
#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array)[0]))

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------
Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------
Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------
void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    while (value == 0) { 			// semaphore not available
        queue->SortedInsert((void *)currentThread, currentThread->getPriority());	// so go to sleep
        currentThread->Sleep();
    }
    value--; 					// semaphore available,
    // consume its value

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------
void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Lock::Lock
//  
//----------------------------------------------------------------------
Lock::Lock(char* debugName) {
	held = 0;
	Q = new List();
  name = debugName;
}

//----------------------------------------------------------------------
// Lock::~Lock
//  
//----------------------------------------------------------------------
Lock::~Lock() {
  ASSERT(!held);
  ASSERT(Q->IsEmpty()); 
  delete Q;
}

//----------------------------------------------------------------------
///DEBUG Lock::Acquire
//	Wait until the lock is FREE, then set it to BUSY
//----------------------------------------------------------------------
void Lock::Acquire() {
	//disable interrupts;
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	if (!this->isHeldByCurrentThread()){
		while (held == 1) { 
			// put this thread (the current thread) on the wait Q for the lock;
			Q->SortedInsert((void *)currentThread, currentThread->getPriority()); // so go to sleep
			currentThread->Sleep();
		}
		held = 1;
		strcpy(threadName,currentThread->getName());
	}
	(void) interrupt->SetLevel(oldLevel); // enable interrupts;
}

//----------------------------------------------------------------------
// Lock::Release()
//	Sets lock to be FREE, waking up a thread waiting in Acquire if
//	necessary
//----------------------------------------------------------------------
void Lock::Release() {
	// disable interrupts;
	Thread *thread = NULL;
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	char arr[] = {'z'};
	DebugInit(arr);
  ASSERT(this->isHeldByCurrentThread());
	
	if (this->isHeldByCurrentThread()){
		// remove a blocked thread from the Q, if there is one;
		if ( !Q->IsEmpty() ) {
			thread = (Thread *)Q->Remove();
		}
		
		// unblock the removed thread;
		if (thread != NULL) {    // make thread ready, consuming the V immediately
			scheduler->ReadyToRun(thread);
		}
		// do some bookkeeping;
		held = 0;
	}
	
	(void) interrupt->SetLevel(oldLevel); // enable interrupts;
}

//----------------------------------------------------------------------
// Lock::isHeldByCurrentThread
//  Returns true if the lock is held by the current thread.
//----------------------------------------------------------------------
bool Lock::isHeldByCurrentThread() {
	
	// figure out size/strlength of threadName
	int threadNameSize = 0;
	for (int i = 0; ; i++){
		if (threadName[i] == '\0'){
			threadNameSize = i;
			break;
		}
	}
	
	// figure out size/strlength of currentThread->getName()
	int threadNameSize2 = 0;
	for (int i = 0; ; i++){
		if ((currentThread->getName())[i] == '\0'){
			threadNameSize2 = i;
			break;
		}
	}
	
	// if the sizes are different, break false early
	if (threadNameSize != threadNameSize2){
		return 0;
	}
	// if any of the elements are different, break false
	for (int i = 0; i < threadNameSize; i++){
		if (threadName[i] != (currentThread->getName())[i]){
			return 0;}
	}
  // if you got here, they are equal
  return held && 1;
}

//----------------------------------------------------------------------
// Condition::Condition
//  
//----------------------------------------------------------------------
Condition::Condition(char* debugName) { 
  waitList = new List;
  name = debugName;
}

//----------------------------------------------------------------------
// Condition::~Condition
//  
//----------------------------------------------------------------------
Condition::~Condition() {
  ASSERT(waitList->IsEmpty()) { 
    delete waitList;
  }
}

//----------------------------------------------------------------------
// Condition::Wait
//	Releases the lock, relinquish the CPU until signaled,
//	then re-acquires the lock
//	[book]Atomically releases the lock and suspends execution of
//	the calling thread, placing the calling thread on the cv's
//	waiting list. Later, when the calling thread is re-enabled,
//	it re-acquires the lock before returning from the wait call.
//----------------------------------------------------------------------
void Condition::Wait(Lock* conditionLock) {
  ASSERT(conditionLock->isHeldByCurrentThread());
  IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts;  
  waitList->SortedInsert(currentThread, currentThread->getPriority()); //Prioritize threads on wait list
  conditionLock->Release();
  char arr[] = {'z'};
  DebugInit(arr);
  DEBUG('z', "Waiting thread: %s\n", currentThread->getName());
  currentThread->Sleep();
  // relinquish the CPU until signaled
  conditionLock->Acquire();
  (void)interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Condition::Signal
//	Wakes up a thread, if there are any waiting on
//	the condition 
//  [book]Takes one thread off the cv's waiting list and marks it as
//  eligible to run. If no threads are on the waiting list, Signal
//  has no effect.
//----------------------------------------------------------------------
void Condition::Signal(Lock* conditionLock) {
  char arr[] = {'z'};
  DebugInit(arr);
  ASSERT(conditionLock->isHeldByCurrentThread());
  IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts;
  if (!waitList->IsEmpty()) {
    Thread *thread = (Thread *)waitList->Remove();
    scheduler->ReadyToRun(thread);
    DEBUG('z', "%s has been unblocked by call to Signal() and is now on the ready queue\n", thread->getName());
  }
  else {
    DEBUG('z', "Signal() is a no-op, nothing to signal\n");          
  }          
  (void)interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Condition::Broadcast
//	Wakes up all threads waiting on the condition 
//  [book]Takes all threads off the cv's waiting list and marks them
//  as eligible to run. If no threads on waiting list, Broadcast
//  has no effect.
//----------------------------------------------------------------------
void Condition::Broadcast(Lock* conditionLock) {
  ASSERT(conditionLock->isHeldByCurrentThread());
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  Thread *thread;
  if(waitList->IsEmpty()) {
    DEBUG('z', "Broadcast() is a no-op, nothing to signal\n");          
  }    
  while((thread = (Thread *)waitList->Remove()) != NULL) {
    scheduler->ReadyToRun(thread); // move thread to ready queue, dequeue it
    DEBUG('z', "%s has been unblocked by call to Broadcast() and is now on the ready queue\n", thread->getName());
  }
  (void)interrupt->SetLevel(oldLevel);
}


//----------------------------------------------------------------------
// Mailbox::Mailbox()
//---------------------------------------------------------------------
Mailbox::Mailbox(char* name) {
  mailboxName = name;
  bufferSet = 0;
  numItems = 0;
  boxLock = new Lock("boxLock");
  bufferLock = new Lock("bufferLock");
  bufferCV = new Condition("bufferCV");
  mailboxCV = new Condition("mailboxCV");
  empty = new Semaphore("empty", 1);
  full = new Semaphore("full", 0);
}

//----------------------------------------------------------------------
// Mailbox::~Mailbox()
//----------------------------------------------------------------------
Mailbox::~Mailbox() {
  delete boxLock;
  delete bufferLock;
  delete bufferCV;
  delete mailboxCV;
  delete empty;
  delete full;
}

//----------------------------------------------------------------------
// Mailbox::Send
//  Atomically waits until Receive is called on the same mailbox and
//  then copies the message into the receive buffer. Once the copy
//  is made, both can return
//----------------------------------------------------------------------
void Mailbox::Send(int message) {
  char arr[] = {'z'};
  DebugInit(arr);
  DEBUG('z',"(Mailbox::Send executing threadName:%s)\n",currentThread->getName());
  boxLock->Acquire();
  numItems++;            // Sender is now present
	if(numItems > 0) {     // if Senders only
    DEBUG('z',"S: Waiting on the Receiver\n");
    mailboxCV->Wait(boxLock);  // wait for Receivers
  }
  else {  // if no items or more Receivers 
    mailboxCV->Signal(boxLock);  // wake up
  }
  boxLock->Release();
  
  empty->P();
  bufferLock->Acquire();
  while(bufferSet) {     // if buffer has been written already
    bufferCV->Wait(bufferLock); //make sure buffer isn't overwritten
  }
  DEBUG('z',"S: Sending message: %d\n", message);
  buffer = message;            // drop off message
  bufferSet = 1;     
  DEBUG('z',"S: Message has been dropped off, waking receiver\n");
  bufferCV->Signal(bufferLock); // waking receiver
  bufferLock->Release(); 
  full->V();
}

//----------------------------------------------------------------------
// Mailbox::Receive
//  Waits until Send is called, at which point the copy is made and
//  both calls return
//----------------------------------------------------------------------
void Mailbox::Receive(int *message){
  char arr[] = {'z'};
  DebugInit(arr);
  DEBUG('z',"(Mailbox::Receive executing threadName:%s)\n",currentThread->getName());
  boxLock->Acquire();
  numItems--;          // Receiver present
  if(numItems < 0) {   // if no Senders
    DEBUG('z',"R: Waiting for Senders\n");
    mailboxCV->Wait(boxLock);  // wait for Senders
  }
  else {               // if nothing present or Senders only
    mailboxCV->Signal(boxLock);
  }
  boxLock->Release();
  
  full->P();
  bufferLock->Acquire();
  while(!bufferSet) {  //if Sender did not drop off message
    DEBUG('z',"R: Waiting for message dropoff\n");
    bufferCV->Wait(bufferLock);  // wait for message
  }
  DEBUG('z',"R: Accepting message from sender\n");
  *message = buffer;             // accept message
  bufferSet = 0;       // buffer can now be overwritten

  //print message that is being received
  DEBUG('z', "#############################################\n");
  DEBUG('z', "MAILBOX\n");
  DEBUG('z', "Message received: %d \n", *message);
  DEBUG('z', "Current Thread Name: %s\n", currentThread->getName());
  DEBUG('z', "#############################################\n");

  bufferCV->Signal(bufferLock); //signal buffer to reset
  bufferLock->Release();
  empty->V(); 
}
