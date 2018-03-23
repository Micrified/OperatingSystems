#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (void) {
    time_t now = time(NULL);
    time_t utc = utctime(NULL);
    printf("UNX:\t %ld\nUTC:\t %ld\nDIFF:\t %ld\n", (long)now, (long)utc, (long)(utc-now));
    return 0;
}