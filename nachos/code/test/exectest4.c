/* 
 * exectest4.c
 *
 * Simple program to test exec system call.
 * Specifies a filename that DNE 
 */

#include "syscall.h"
int main() {
  int result = 1000;
	result = Exec("../test/DNE_File",0,0,0);
	Exit(result);
}
