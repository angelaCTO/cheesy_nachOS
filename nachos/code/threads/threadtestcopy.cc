// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "Whale.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;

    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// LockTests and ConditionTests
//----------------------------------------------------------------------
Lock *lock = new Lock("LockTest");
Condition *condition = new Condition("ConditionTest");

void
AcquireLock(int param)
{
  lock->Acquire();
}

void
ReleaseLock(int param) {
  lock->Release();
}

void
DeleteLock(int param) {
  delete lock;
}

// Acquiring the same lock twice
void LockTest1(){
  DEBUG('t', "Entering LockTest1");
  Thread *t1 = new Thread("thread1");
  t1->Fork(AcquireLock, 0);
  t1->Fork(AcquireLock, 0);
}

// Releasing lock that isn't held
void LockTest2() {
  DEBUG('t', "Entering LockTest2");
  Thread *t1 = new Thread("thread1");
  t1->Fork(ReleaseLock, 0);  
}

// Deleting a lock that is held
void LockTest3() {
  DEBUG('t', "Entering LockTest3");
  Thread *t1 = new Thread("thread1");
  t1->Fork(AcquireLock, 0);
  t1->Fork(DeleteLock, 0);
}

void WaitWithoutAcquire(int param) {
  condition->Wait(lock);
}        

// Waiting on a CV without holding a lock
void LockTest4() {
  DEBUG('t', "Entering LockTest4");
  Thread *t1 = new Thread("thread1");
  t1->Fork(WaitWithoutAcquire, 0);
}

void AcquireAndReleaseLock(int param) {
  lock->Acquire();
  lock->Release();
}

//deleting a lock should have no threads on the wait queue
void LockTest5() {
  DEBUG('t', "Entering LockTest5");
  Thread *t1 = new Thread("thread1");
  Thread *t2 = new Thread("thread2");
  t1->Fork(AcquireAndReleaseLock, 0);
  t2->Fork(DeleteLock, 0);
}

void ConditionWait(int param) {
  lock->Acquire();
  condition->Wait(lock); 
}

void ConditionSignal(int param) {
  lock->Acquire();        
  condition->Signal(lock);
  lock->Release();  
}

void SignalWithoutAcquire(int param) {
  condition->Signal(lock);
}
void ConditionBroadcast(int param) {
  lock->Acquire();
  condition->Broadcast(lock);
  lock->Release();
}

void DeleteCondition(int param) {
  delete condition;
}

// Signaling a condition variable wakes only one thread
void ConditionTest1() {
  DEBUG('t', "Entering ConditionTest1");
  Thread *t1 = new Thread("thread1");
  Thread *t2 = new Thread("thread2");
  Thread *t3 = new Thread("thread3");
  t1->Fork(ConditionWait, 0);
  t2->Fork(ConditionWait, 0);
  t3->Fork(ConditionSignal, 0);
}

// Broadcasting a condition wakes up all threads
void ConditionTest2() {
  DEBUG('t', "Entering ConditionTest2");
  Thread *t1 = new Thread("thread1");
  Thread *t2 = new Thread("thread2");
  Thread *t3 = new Thread("thread3");
  Thread *t4 = new Thread("thread4");
  t1->Fork(ConditionWait, 0);
  t2->Fork(ConditionWait, 0);
  t3->Fork(ConditionWait, 0);
  t4->Fork(ConditionBroadcast, 0);
}

// Signaling and broadcasting to a condition variable with no
//       waiters is a no-op and future threads that wait will block
//       (i.e. that signal/broadcast is "lost")
void ConditionTest3() {
  DEBUG('t', "Entering ConditionTest3");
  Thread *t1 = new Thread("thread1");
  Thread *t2 = new Thread("thread2");
  Thread *t3 = new Thread("thread3");
  t1->Fork(ConditionSignal, 0);
  t2->Fork(ConditionBroadcast, 0);
  t3->Fork(ConditionWait, 0);
}

// a thread calling Signal holds the Lock passed in to Signal
void ConditionTest4() {
  DEBUG('t', "Entering ConditionTest4");
  Thread *t1 = new Thread("thread1");
  t1->Fork(SignalWithoutAcquire, 0);
}

// deleting condition variable should have no threads on the wait queue
void ConditionTest5() {
  DEBUG('t', "Entering ConditionTest5");
  Thread *t1 = new Thread("thread1");
  Thread *t2 = new Thread("thread2");
  t1->Fork(ConditionWait, 0);
  t2->Fork(DeleteCondition, 0);
}        



//----------------------------------------------------------------------
// Part 2 - Mailbox
//----------------------------------------------------------------------
Mailbox *mailbox = new Mailbox("mailbox");

void Receive(int param) {
  int *message = new int;
  *message = 0;
  mailbox->Receive(message);
}

void Send(int param) {
  mailbox->Send(param);
}

//Many Receivers, should wait on a Sender
void MailboxTest1() {
  DEBUG('t',"Mailbox Test 1");
  Thread *T1 = new Thread("ONE");
  Thread *T2 = new Thread("TWO");
  Thread *T3 = new Thread("THREE");
  Thread *T4 = new Thread("FOUR");
  T1->Fork(Receive, 0); //should block
  T2->Fork(Receive, 0); //should block
  T3->Fork(Receive, 0); //should block
  T4->Fork(Send, 1); //T1 should return, T4 should return too
                     //SHOULD PRINT 1
}

//Many Senders, should wait on a Receiver
void MailboxTest2() {
  DEBUG('t',"Mailbox Test 2");
  Thread *T1 = new Thread("ONE");
  Thread *T2 = new Thread("TWO");
  Thread *T3 = new Thread("THREE");
  Thread *T4 = new Thread("FOUR");
  T1->Fork(Send, 1); //should block
  T2->Fork(Send, 2); //should block
  T3->Fork(Send, 3); //should block
  T4->Fork(Receive, 0); //T1 should return, T4 should return too
                       //SHOULD PRINT 1
}

//4 Senders, then 4 Receivers, all messages should print
void MailboxTest3() {
  DEBUG('t', "Mailbox Test 3");
  Thread *T1 = new Thread("ONE");
  Thread *T2 = new Thread("TWO");
  Thread *T3 = new Thread("THREE");
  Thread *T4 = new Thread("FOUR");
  Thread *T5 = new Thread("FIVE");
  Thread *T6 = new Thread("SIX");
  Thread *T7 = new Thread("SEVEN");
  Thread *T8 = new Thread("EIGHT");
  T1->Fork(Send, 1); //should block
  T2->Fork(Send, 2); //should block
  T3->Fork(Send, 3); //should block
  T4->Fork(Send, 4); //should block
  T5->Fork(Receive, 0); //should print 1, T1 & T5 should return
  T6->Fork(Receive, 0); //should print 2, T2 & T6 should return
  T7->Fork(Receive, 0); //should print 3, T3 & T7 should return
  T8->Fork(Receive, 0); //should print 4, T4 & T8 should return
}

//4 Receivers, then 4 Senders, all messages should print
void MailboxTest4() {
  DEBUG('t', "Mailbox Test 4");
  Thread *T1 = new Thread("ONE");
  Thread *T2 = new Thread("TWO");
  Thread *T3 = new Thread("THREE");
  Thread *T4 = new Thread("FOUR");
  Thread *T5 = new Thread("FIVE");
  Thread *T6 = new Thread("SIX");
  Thread *T7 = new Thread("SEVEN");
  Thread *T8 = new Thread("EIGHT");
  T1->Fork(Receive, 0);
  T2->Fork(Receive, 0);
  T3->Fork(Receive, 0);
  T4->Fork(Receive, 0);
  T5->Fork(Send, 1);  //should print 1
  T6->Fork(Send, 2);  //should print 2
  T7->Fork(Send, 3);  //should print 3
  T8->Fork(Send, 4);  //should print 4
}

//Senders ONLY, should all block and not print
void MailboxTest5() {
  DEBUG('t', "Mailbox Test 5");
  Thread *T1 = new Thread("ONE");
  Thread *T2 = new Thread("TWO");
  Thread *T3 = new Thread("THREE");
  Thread *T4 = new Thread("FOUR");
  T1->Fork(Send, 1);  //should block
  T2->Fork(Send, 2);  //should block
  T3->Fork(Send, 3);  //should block
  T4->Fork(Send, 4);  //should block
}

//Receivers ONLY, should all block and not print
void MailboxTest6() {
  DEBUG('t', "Mailbox Test 6");
  Thread *T1 = new Thread("ONE");
  Thread *T2 = new Thread("TWO");
  Thread *T3 = new Thread("THREE");
  Thread *T4 = new Thread("FOUR");
  T1->Fork(Receive, 0);
  T2->Fork(Receive, 0);
  T3->Fork(Receive, 0);
  T4->Fork(Receive, 0);
}

//Interwoven Senders and Receivers, as well as
//multiple calls made on same threads
void MailboxTest7() {
  DEBUG('t', "Mailbox Test 7");
  Thread *T1 = new Thread("ONE");
  Thread *T2 = new Thread("TWO");
  Thread *T3 = new Thread("THREE");
  T1->Fork(Send, 1);     //T1 should block
  T2->Fork(Receive, 0);  //T2 receives, should print 1
  T1->Fork(Send, 2);
  T2->Fork(Receive, 0);
  //scheduler->Print();
/*  T1->Fork(Send, 2);     //T1 should block
  T2->Fork(Receive, 0);  //T3 receives, should print 2
  //scheduler->Print();
  T1->Fork(Send, 3);     //T3 sends, should block
 // T3->Fork(Receive, 0);  //should not be legal call
  T2->Fork(Receive, 0);  //T4 receives T3, should print 3
*/  //scheduler->Print();
}

void MailboxTest8() {
  DEBUG('t', "Mailbox Test 8");
  Thread *T1 = new Thread("ONE");
  Thread *T2 = new Thread("TWO");
  Thread *T3 = new Thread("THREE");
  Thread *T4 = new Thread("FOUR");
  Thread *T5 = new Thread("FIVE");
  Thread *T6 = new Thread("SIX");
  T1->Fork(Send, 1);     //T1 should block
  T2->Fork(Receive, 0);  //T2 receives, should print 1
  T3->Fork(Send, 2);     //T1 should block
  T4->Fork(Receive, 0);  //T2 receives, should print 1
  T5->Fork(Send, 3);     //T1 should block
  T6->Fork(Receive, 0);  //T2 receives, should print 1
}

//----------------------------------------------------------------------
// Part 3 - Join
//----------------------------------------------------------------------
void
Joinee()
{
	printf("Joinee START\n");
	int i;
	
	for (i = 0; i < 5; i++) {
		printf("Smell the roses. %d\n",i);
		currentThread->Yield();
	}
	
	currentThread->Yield();
	printf("Done smelling the roses!\n");
	currentThread->Yield();
}
void
Joiner(Thread *joinee)
{
	printf("Joiner START\n");
	
	currentThread->Yield();
	currentThread->Yield();
	
	printf("Waiting for the Joinee to finish executing.\n");
	
	currentThread->Yield();
	currentThread->Yield();
	
	// Note that, in this program, the "joinee" has not finished
	// when the "joiner" calls Join().  You will also need to handle
	// and test for the case when the "joinee" _has_ finished when
	// the "joiner" calls Join().
	
	joinee->Join();
	
	currentThread->Yield();
	currentThread->Yield();
	
	printf("Joinee has finished executing, we can continue.\n");
	
	currentThread->Yield();
	currentThread->Yield();
}

void
ForkerThread()
{
	Thread *joiner = new Thread("joiner", 0);  // will not be joined
	Thread *joinee = new Thread("joinee", 1);  // WILL be joined
	
	// fork off the two threads and let them do their business
	joiner->Fork((VoidFunctionPtr) Joiner, (int) joinee);
	joinee->Fork((VoidFunctionPtr) Joinee, 0);
	
	// this thread is done and can go on its merry way
	printf("Forked off the joiner and joiner threads.\n");
}
//----
void ParentFork(Thread *child){
	child->Join();
	// assert that the thread has now been destroyed
	ASSERT(strcmp(currentThread->getName(),child->getName()) != 0);
}
void ChildFork(){
	// assert that the thread still exists, despite our attempt to destroy it
	//ASSERT(strcmp(currentThread->getName(),child->getName()) == 0);
	currentThread->Yield();
}


// 3.1 should fail DONE
void ParentFork3_1(Thread *child){
	child->Join();
}
void ChildFork3_1(){printf("child ran!\n");}
void Part3_1Test(){
	printf("3.1!\n");
	Thread *parent = new Thread("parent",0);  // will not be joined
	Thread *child = new Thread("child",1);  // WILL be joined
	child->Fork((VoidFunctionPtr)ChildFork3_1,0);
	parent->Fork((VoidFunctionPtr)ParentFork3_1,(int)child);
	printf("exited\n");
}

// 3.2 should see prints (((1)))(((2)))(((3))) in that order
void ParentFork3_2(Thread *child){
	printf("(((1)))\n");
	child->Join();
	printf("(((3)))\n");
}
void ChildFork3_2(){printf("(((2)))\n");}
void Part3_2Test(){
	Thread *parent = new Thread("parent",0);
	Thread *child = new Thread("child",1);
	parent->Fork((VoidFunctionPtr)ParentFork3_2,(int)child);
	child->Fork((VoidFunctionPtr)ChildFork3_2,0);
	printf("exited\n");
}

// 3.3 should succeed and print something like "SUCCESS" GOOD
void ParentFork3_3(Thread *child){
	child->Join();
printf("********************************************************************************\n");
	printf("Test 3.3 SUCCESS\n");
	printf("********************************************************************************\n");
}
void ChildFork3_3(){}
void Part3_3Test(){
	Thread *parent = new Thread("parent",0);
	Thread *child = new Thread("child",1);
	child->Fork((VoidFunctionPtr)ChildFork3_3,0);
	parent->Fork((VoidFunctionPtr)ParentFork3_3,(int)child);
	printf("exited\n");
}

// 3.4 should fail GOOD
void ParentFork3_4(Thread *parent){
	parent->Join();
}
void Part3_4Test(){
	Thread *parent = new Thread("parent",1);  // will not be joined
	parent->Fork((VoidFunctionPtr)ParentFork3_4,(int)parent);
	printf("exited\n");
}

// 3.5 should fail GOOD
void ParentFork3_5(Thread *child){child->Join();}
void ChildFork3_5(){}
void Part3_5Test(){
	Thread *parent = new Thread("parent",0);
	Thread *child = new Thread("child",0);
	parent->Fork((VoidFunctionPtr)ParentFork3_5,(int)child);
	child->Fork((VoidFunctionPtr)ChildFork3_5,0);
	printf("exited\n");
}

// 3.6 should fail GOOD
void ParentFork3_6(Thread *child){child->Join();}
void Part3_6Test(){
	Thread *parent = new Thread("parent",0);
	Thread *child = new Thread("child",1);
	parent->Fork((VoidFunctionPtr)ParentFork3_6,(int)child);
	printf("exited\n");
}
// 3.6.1 should succeed GOOD
void ParentFork3_6_1(Thread *child){child->Join();}
void ChildFork3_6_1(){}
void Part3_6_1Test(){
	Thread *parent = new Thread("parent",0);
	Thread *child = new Thread("child",1);
	parent->Fork((VoidFunctionPtr)ParentFork3_6_1,(int)child);
	child->Fork((VoidFunctionPtr)ChildFork3_6_1,0);
	printf("exited\n");
}

// 3.7 should fail GOOD
void ParentFork3_7(Thread *child){
	printf("(1)");
	child->Join();
	printf("(2)");
	child->Join();
	printf("(3)");
}
void ChildFork3_7(){
	printf("CHILD RAN\n");
}
void Part3_7Test(){
	Thread *parent = new Thread("parent",0);
	Thread *child = new Thread("child",1);
	parent->Fork((VoidFunctionPtr)ParentFork3_7,(int)child);
	child->Fork((VoidFunctionPtr)ChildFork3_7,0);
	printf("exited\n");
}

//----------------------------------------------------------------------
// Part 4 (Priorities) Tests
//----------------------------------------------------------------------

Condition *priorityCV;
Lock *priorityLock;
int i = 0;

void emptyMethod(int param) {
        //DO NOTHING
}

void PriorityTest1() {
  DEBUG('t', "Priority Test 1");
  Thread *t1 = new Thread("thread1");
  Thread *t2 = new Thread("thread2");
  Thread *t3 = new Thread("thread3");
  Thread *t4 = new Thread("thread4");
  Thread *t5 = new Thread("thread5");
  Thread *t6 = new Thread("thread6");
  t1->setPriority(-1);
  t2->setPriority(-8);
  t3->setPriority(6);
  t4->setPriority(10);
  t5->setPriority(8);
  t1->Fork(emptyMethod, 0);
  t2->Fork(emptyMethod, 0);
  t3->Fork(emptyMethod, 0);
  t4->Fork(emptyMethod, 0);
  t5->Fork(emptyMethod, 0);
  t6->Fork(emptyMethod, 0);
  scheduler->Print(); //should print 4, 5, 3, 6, 1, 2
}

void synchronize(int param) {
  priorityLock->Acquire();
  currentThread->setPriority(0);
  printf("Before waiting thread: %s\n", currentThread->getName());
  scheduler->Print();

  while (!i) {
    priorityCV->Wait(priorityLock);
  }

  printf("After Wait call has been finished for thread: %s\n", currentThread->getName());
  scheduler->Print();
  priorityLock->Release();
}

void synchronize2(int param) {
  priorityLock->Acquire();
  i = 1;
  printf("Before signalling in thread: %s\n", currentThread->getName());
  scheduler->Print();

  priorityCV->Signal(priorityLock);
  printf("After signalling in thread: %s\n", currentThread->getName());
  scheduler->Print();
  priorityLock->Release();
}

void PriorityTest2() {
  DEBUG('t', "Priority Test 2");
  
  priorityCV = new Condition("PriorityCV");
  priorityLock = new Lock("PriorityLock");

  Thread *t1 = new Thread("thread1");
  Thread *t2 = new Thread("thread2");
  Thread *t3 = new Thread("thread3");
  Thread *t4 = new Thread("thread4");
  Thread *t5 = new Thread("thread5");
  Thread *t6 = new Thread("thread6");
  Thread *t7 = new Thread("thread7");
  Thread *t8 = new Thread("thread8");
  t1->setPriority(12);
  t2->setPriority(5);
  t3->setPriority(6);
  t4->setPriority(10);
  t5->setPriority(9);
  t6->setPriority(-1);
  t7->setPriority(3);
  t8->setPriority(4);

  t3->Fork(emptyMethod, 0);
  t4->Fork(emptyMethod, 0);
  t1->Fork(synchronize, 0);
  t5->Fork(synchronize, 0);
  t6->Fork(emptyMethod, 0);
  t2->Fork(synchronize2, 0);
  t8->Fork(synchronize2, 0);
  t7->Fork(emptyMethod, 0);
  printf("First print, with all threads forked:\n");
  scheduler->Print(); //should print 1, 4, 5, 3, 2, 8, 7, 6 
}




//----------------------------------------------------------------------
// Part 5 (Whales) Tests
//----------------------------------------------------------------------
Whale *whaleGenerator = new Whale("whaleGenerator");
void makeMales(int trash){
	whaleGenerator->Male();
}

void makeFemales(int trash){
	whaleGenerator->Female();
}

void makeMatchmaker(int trash){
	whaleGenerator->Matchmaker();
}

void printFor5_1(int param) {
  //0 triplets, 3 males, 2 females, 0 MM        
	printf("Triplets made: %d\n",whaleGenerator->getTriplets());
  //subtract values because getMales() getFemales() getMM() returns all whales every created (including matched whales)
	printf("Unmatched Males: %d\n",whaleGenerator->getMales() - whaleGenerator->getTriplets());
	printf("Unmatched Females: %d\n",whaleGenerator->getFemales() - whaleGenerator->getTriplets());
	printf("Unmatched Matchmakers: %d\n",whaleGenerator->getMM() - whaleGenerator->getTriplets());
}

// 5.1
void Part5_1Test(){
	Thread *T1 = new Thread("T1",0);
  Thread *T2 = new Thread("T2", 0);
  Thread *T3 = new Thread("T3", 0);
  Thread *T4 = new Thread("T4", 0);
  Thread *T5 = new Thread("T5", 0);
  Thread *T6 = new Thread("T6", 0);
  T6->setPriority(-1); //set priority to ensure printing after all whale matching has been done

	T1->Fork(makeMales,0);
	T2->Fork(makeMales,0);
	T3->Fork(makeFemales,0);
	T4->Fork(makeFemales,0);
	T5->Fork(makeMales,0);
  T6->Fork(printFor5_1, 0);
}

void printFor5_2(int param) {
  //2 triplets        
	printf("Triplets made: %d\n",whaleGenerator->getTriplets());
  //subtract values because getMales() getFemales() getMM() returns all whales every created (including matched whales)
	printf("Unmatched Males: %d\n",whaleGenerator->getMales() - whaleGenerator->getTriplets());
	printf("Unmatched Females: %d\n",whaleGenerator->getFemales() - whaleGenerator->getTriplets());
	printf("Unmatched Matchmakers: %d\n",whaleGenerator->getMM() - whaleGenerator->getTriplets());
} 

// 5.2
void Part5_2Test(){
	Thread *T1 = new Thread("T1",0);
  Thread *T2 = new Thread("T2", 0);
  Thread *T3 = new Thread("T3", 0);
  Thread *T4 = new Thread("T4", 0);
  Thread *T5 = new Thread("T5", 0);
  Thread *T6 = new Thread("T6", 0);
  Thread *T7 = new Thread("T7", 0);
  T7->setPriority(-1); //set priority to ensure printing after all whale matching has been done

	T1->Fork(makeMales,0);
	T2->Fork(makeFemales,0);
	T3->Fork(makeMatchmaker,0);
	T4->Fork(makeMatchmaker,0);
	T5->Fork(makeFemales,0);
	T6->Fork(makeMales,0);
  T7->Fork(printFor5_2, 0);
}

void printFor5_3(int param) {
  //1 triplet, 1 male, 0 female, 1 matchmaker        
	printf("Triplets made: %d\n",whaleGenerator->getTriplets());
  //subtract values because getMales() getFemales() getMM() returns all whales every created (including matched whales)
	printf("Unmatched Males: %d\n",whaleGenerator->getMales() - whaleGenerator->getTriplets());
	printf("Unmatched Females: %d\n",whaleGenerator->getFemales() - whaleGenerator->getTriplets());
	printf("Unmatched Matchmakers: %d\n",whaleGenerator->getMM() - whaleGenerator->getTriplets());
}

// 5.3
void Part5_3Test(){
	Thread *T1 = new Thread("T1",0);
  Thread *T2 = new Thread("T2", 0);
  Thread *T3 = new Thread("T3", 0);
  Thread *T4 = new Thread("T4", 0);
  Thread *T5 = new Thread("T5", 0);
  Thread *T6 = new Thread("T6", 0);
  T6->setPriority(-1); //set priority to ensure printing after all whale matching has been done

	T1->Fork(makeMales,0);
	T2->Fork(makeMales,0);
	T3->Fork(makeFemales,0);
	T4->Fork(makeMatchmaker,0);
	T5->Fork(makeMatchmaker,0);
  T6->Fork(printFor5_3, 0);
}

void printFor5_4(int param){
  //2 triplets, 1 male, 0 females, 0 matchmakers        
	printf("Triplets made: %d\n",whaleGenerator->getTriplets());
  //subtract values because getMales() getFemales() getMM() returns all whales every created (including matched whales)
	printf("Unmatched Males: %d\n",whaleGenerator->getMales() - whaleGenerator->getTriplets());
	printf("Unmatched Females: %d\n",whaleGenerator->getFemales() - whaleGenerator->getTriplets());
	printf("Unmatched Matchmakers: %d\n",whaleGenerator->getMM() - whaleGenerator->getTriplets());
}

// 5.4
void Part5_4Test(){
	Thread *T1 = new Thread("T1",0);
  Thread *T2 = new Thread("T2", 0);
  Thread *T3 = new Thread("T3", 0);
  Thread *T4 = new Thread("T4", 0);
  Thread *T5 = new Thread("T5", 0);
  Thread *T6 = new Thread("T6", 0);
  Thread *T7 = new Thread("T7", 0);
  Thread *T8 = new Thread("T8", 0);
  T8->setPriority(-1); //set priority to ensure printing after all matching has been done

	T1->Fork(makeMales,0);
	T2->Fork(makeMales,0);
	T3->Fork(makeMales,0);
	T4->Fork(makeFemales,0);
	T5->Fork(makeFemales,0);
	T6->Fork(makeMatchmaker,0);
	T7->Fork(makeMatchmaker,0);
  T8->Fork(printFor5_4, 0);
}

void printFor5_5(int param) {
  //0 triplets, 0 male, 1 female, 6 matchmaker        
	printf("Triplets made: %d\n",whaleGenerator->getTriplets());
  //subtract values because getMales() getFemales() getMM() returns all whales every created (including matched whales)
	printf("Unmatched Males: %d\n",whaleGenerator->getMales() - whaleGenerator->getTriplets());
	printf("Unmatched Females: %d\n",whaleGenerator->getFemales() - whaleGenerator->getTriplets());
	printf("Unmatched Matchmakers: %d\n",whaleGenerator->getMM() - whaleGenerator->getTriplets());
}

// 5.5
void Part5_5Test(){
	Thread *T1 = new Thread("T1",0);
  Thread *T2 = new Thread("T2", 0);
  Thread *T3 = new Thread("T3", 0);
  Thread *T4 = new Thread("T4", 0);
  Thread *T5 = new Thread("T5", 0);
  Thread *T6 = new Thread("T6", 0);
  Thread *T7 = new Thread("T7", 0);
  Thread *T8 = new Thread("T8", 0);
  T8->setPriority(-1);

	T1->Fork(makeMatchmaker,0);
	T2->Fork(makeMatchmaker,0);
	T3->Fork(makeMatchmaker,0);
	T4->Fork(makeMatchmaker,0);
	T5->Fork(makeMatchmaker,0);
	T6->Fork(makeMatchmaker,0);
	T7->Fork(makeFemales,0);
  T8->Fork(printFor5_5, 0);
}


//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------
void
ThreadTest()
{
    switch (testnum) {
    case 1: ThreadTest1(); break;

    //part 1 - Locks and CVs        
    case 2: LockTest1(); break;
    case 3: LockTest2(); break;
    case 4: LockTest3(); break;
    case 5: LockTest4(); break;
    case 6: LockTest5(); break;
    case 7: ConditionTest1(); break;
    case 8: ConditionTest2(); break;
    case 9: ConditionTest3(); break;
    case 10: ConditionTest4(); break;
    case 11: ConditionTest5(); break;

    //part 2 - Mailbox
    case 15: MailboxTest1(); break;
    case 16: MailboxTest2(); break;
    case 17: MailboxTest3(); break;
    case 18: MailboxTest4(); break;
    case 19: MailboxTest5(); break;
    case 20: MailboxTest6(); break;
    case 21: MailboxTest7(); break;
    case 22: MailboxTest8(); break;

    // part 3 - Join
	  case 14: ForkerThread(); break;
	  case 31 :Part3_1Test();  break;
	  case 32 :Part3_2Test();  break;
	  case 33 :Part3_3Test();  break;
	  case 34 :Part3_4Test();  break;
  	case 35 :Part3_5Test();  break;
  	case 36 :Part3_6Test();  break;
  	case 361:Part3_6_1Test();break;
  	case 37 :Part3_7Test();  break;

    // part 4 - priorities
    case 38: PriorityTest1(); break;
    case 39: PriorityTest2(); break;
	
	  // part 5 - whales
	  case 51:Part5_1Test();break;
	  case 52:Part5_2Test();break;
	  case 53:Part5_3Test();break;
	  case 54:Part5_4Test();break;
	  case 55:Part5_5Test();break;
          
	  //default
    default:
        printf("No test specified.\n");
        break;
    }
}

