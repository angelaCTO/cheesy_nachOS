#include "Whale.h"

//Constructor
Whale::Whale(char* name) {
  debugName = name;
  numMales = 0;
  numFemales = 0;
  numMM = 0;
  tripletsMade = 0;
  tripletsCounter = 0;
  whaleLock = new Lock("whaleLock");
  maleCV = new Condition("maleCV");
  femaleCV = new Condition("femaleCV");
  mmCV = new Condition("mmCV");
}

Whale::~Whale() {
  delete whaleLock;
  delete maleCV;
  delete femaleCV;
  delete mmCV;
}

void Whale::Male(){
  char arr[] = {'z'};
  DebugInit(arr);

  whaleLock->Acquire();
  numMales++;
  DEBUG('z', "Number of Males has been incremented, values is now %d\n", numMales);
  if(numFemales < numMales || numMM < numMales) { 
      maleCV->Wait(whaleLock); //wait males if females and MM are unmatched 
  }
  else {
    femaleCV->Signal(whaleLock);
    mmCV->Signal(whaleLock);
  }

  Done("Male"); //print matched message
  whaleLock->Release();
}

void Whale::Female() {
  char arr[] = {'z'};
  DebugInit(arr);

  whaleLock->Acquire();
  numFemales++;
  DEBUG('z', "Number of Females has been incremented, values is now %d\n", numFemales);
  if(numMales < numFemales || numMM < numFemales) { 
    femaleCV->Wait(whaleLock); //wait females if males and MM are unmatched
  }
  else {
    maleCV->Signal(whaleLock);
    mmCV->Signal(whaleLock);    
  }

  Done("Female"); //print matched message
  whaleLock->Release();
}

void Whale::Matchmaker(){
  char arr[] = {'z'};
  DebugInit(arr);

  whaleLock->Acquire();
  numMM++;
  DEBUG('z', "Number of MMs has been incremented, values is now %d\n", numMM);
  if(numFemales < numMM || numMales < numMM) {
    mmCV->Wait(whaleLock); //wait MM if males and females are unmatched
  }
  else {
    maleCV->Signal(whaleLock);
    femaleCV->Signal(whaleLock);    
  }

  Done("Matchmaker"); //print matched message
  whaleLock->Release();
}

//called when all whales have been matched
void Whale::Done(char* type) {
  char arr[] = {'z'};
  DebugInit(arr);

  DEBUG('z', "Match has been detected in %s.\n", type);
  tripletsMade++; //total triplets made*3 because Male, Female, MM all call this method if there has been a match
  tripletsCounter = tripletsMade/3; //get the exact number of triplets by dividing by 3
}

//getter method for number of males
int Whale::getMales() {
  return numMales;
}

//getter method for number of females
int Whale::getFemales() {
  return numFemales;
}

//getter method for number of matchmakers
int Whale::getMM() {
  return numMM;
}

int Whale::getTriplets() {
  return tripletsCounter;
}
