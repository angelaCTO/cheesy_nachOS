/** 
 * pagetest_poorlocal.c
 * Test to simulate poor locality memory reference 
 */

#include "syscall.h"

int arr[50] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
               26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 43, 43, 44, 45, 46, 47, 48, 49};

int sumArray() {
  int i, value;
  for(i = 0; i < 50; i++) {
    value += arr[i];
  }
  return value;
}

int main(){
  int i, sum;

  for(i = 0; i < 10; i++) {
    sum += sumArray();
  }

  Exit(sum);
}
