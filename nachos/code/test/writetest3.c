/*
 * writetest3.c
 * Wrong Console
 */

#include <syscall.h>

int
main ()
{
  char buffer[10];
  int i = 0;
  while (i<10) {
    buffer[i] = 'a';
    i++;
  }
  Write(buffer, 10, ConsoleInput);
  return 0;
}
