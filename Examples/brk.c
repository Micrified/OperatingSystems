/* linux: echo 2 > /proc/sys/vm/overcommit_memory */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MB (1024*1024)

int main (int argc, char *argv[]) {
  char *addr;
  int ok, size=0;
  
  addr = sbrk(0);
  do {
    ok = brk(addr);
    printf ("ok=%d: size=%d MB\n", ok, size);
    size++;
    addr += MB;
  } while (ok == 0);

  return EXIT_SUCCESS;
}
