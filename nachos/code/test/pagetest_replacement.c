/* 
 * pagetest_replacement.c
 * Displays Random Page Replacement policy.
 */

#include "syscall.h"
#define SIZE 1024 

int arr[SIZE];

void initializeArray() {
  int i;
  for(i = 0; i < SIZE; i++) {
    arr[i] = i;
  }
}

// 1 page = 128 Bytes
// 32 PhysPages (machine.h)
// PhysMem therefore is 4096 Bytes
int main(){
  initializeArray();

	int i, sum;
  for(i = 0; i < SIZE; i++) {
    sum += arr[i];
  }
  Exit(sum);
}
