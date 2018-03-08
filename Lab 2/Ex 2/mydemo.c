#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>

// Here's what happens. 
// You allocated four pages of memory, and aligned it so that they cover exactly four actual memory pages.
// You then set the protections for these four blocks to read-only. 
// When your loop begins to try to write 'a' across all four blocks, it immediately triggers a SIGSEGV.
// This signal gets caught my your handler. Your handler sets the entire page's memory space to allow
// for writing. This works because your signal fires on the very first unwritable address, the start 
// of the page. You then set that address + 1 page size as writable.
//
// The instrution then resumes after the handler returns. It then makes its way up to the next page, and
// also gets caught with the signal handler. The signal handler then grants it writing rights again and off we go.
// Because we only allocated four pages. The signal handler kills the whole program after granting right protection three times.


int pagesize;       // The size of a page of memory!

int signalCount;    // Number of times a signal was caught.

void signalHandler (int signal, siginfo_t *signalInfoStruct, void *unused) {

    // Print out where the fault occurred (the address you weren't allowed to use).
    fprintf(stdout, "Got SIGSEGV at address 0x%lx\n", (long)signalInfoStruct->si_addr);

    // Change protections to PROT_WRITE for the address space given (one page size).
    mprotect(signalInfoStruct->si_addr, pagesize, PROT_WRITE);

    // Increment the number of times you've handled this signal. Stop at three.
    if (++signalCount == 3) {
        exit(EXIT_FAILURE);
    }
}

int main (int argc, char *argv[]) {
    void *buffer;
    char *pointer;
    struct sigaction mySignal;

    /******************************************************/

    // Tell the signal handler to take three arguments.
    mySignal.sa_flags = SA_SIGINFO;

    // Clean out the signal-interception mask.
    sigemptyset(&mySignal.sa_mask);

    // Setup our signal handler.
    mySignal.sa_sigaction = signalHandler;

    // Setup the signal we're intercepting.
    sigaction(SIGSEGV, &mySignal, NULL);

    /******************************************************/

    // Get our page size.
    pagesize = sysconf(_SC_PAGE_SIZE);

    // Allocate some memory, and make sure it starts right on the page address boundary.
    // By default, the protections here are reading and writing protections.
    if (posix_memalign(&buffer, pagesize, 4 * pagesize) != 0) {
        fprintf(stderr, "Fatal error: Aligned memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    // Otherwise, print the start of our aligned memory region!
    fprintf(stdout, "Allocated Region Starts at: 0x%lx\n", (long)buffer);

    // Set memory access protection to READ for the entire allocted memory space.
    // Since we aligned our memory we're allowed to use mprotect this way!
    mprotect(buffer, 4 * pagesize, PROT_READ);

    /******************************************************/

    // Set the pointer to our buffer. And try to overwrite everything in there with byte 'a'.
    for (pointer = buffer; ; *(pointer++) = 'a');

    fprintf(stdout, "Loop Completed!\n");
    return 0;
}