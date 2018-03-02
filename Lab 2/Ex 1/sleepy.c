#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main (void) {

	for (int i = 0; i < 100; i++) {
		sleep(1);
		fprintf(stdout, "Sleeping: %d\n", i);
	}
	return 0;
}
