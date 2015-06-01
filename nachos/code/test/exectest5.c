/* 
 * exectest5.c
 *
 * Simple program to test exec system call.
 * Trying to load a program that doesn't fit into physical memory 
 */

#include "syscall.h"
int main() {
  int status = 1000;   
  status = Exec("../test/sort",0,0,0); // Should lead to an assertion failure 
  Exit(status);
}
