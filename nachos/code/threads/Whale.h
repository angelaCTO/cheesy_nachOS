#include "synch.h"

class Whale {
	private:
    char* debugName;
    int numMales;
    int numFemales;
    int numMM;
    int tripletsCounter;
    int tripletsMade;
    Lock* whaleLock;
    Condition* maleCV;
    Condition* femaleCV;
    Condition* mmCV;

  public:
		Whale(char *name);
		~Whale();
		void Male();
		void Female();
		void Matchmaker();
    void Done(char* type);
    int getMales();
    int getFemales();
    int getMM();
    int getTriplets();
};
