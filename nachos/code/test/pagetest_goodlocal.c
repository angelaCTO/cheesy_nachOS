/* 
 * pagetest.c
 *
 * Gobble up memory sequentially
 */

#include "syscall.h"

int arr[50] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
               26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 43, 43, 44, 45, 46, 47, 48, 49};

int repetitiveFunction() {
  int sum;
  sum += arr[0];
  sum += arr[1];
  sum += arr[2];
  return sum;
}

// 1 page = 128 Bytes
// 32 PhysPages (machine.h)
// PhysMem therefore is 4096 Bytes
int main(){
	int i, sum;
  for(i = 0; i < 20; i++) {
    sum += repetitiveFunction();
  }

  Exit(sum);
}
