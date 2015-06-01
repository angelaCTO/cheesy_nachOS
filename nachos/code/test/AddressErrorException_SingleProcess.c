/* 
 * AddressErrorExceptionHelper.c
 */

#include "syscall.h"

int
main()
{
	int *ptr = -1;
	int val = 0;
	val = *ptr; // should trigger an address error exception
}
