#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>


void *page;     // The "shared" memory page.
int pagesize;   // The size of a memory page.

void runChild (void) {
    fprintf(stdout, "CHILD (%d) going to write to the page!\n", getpid());
    //sleep(1);
    char *pagePointer = (char *)page;
    pagePointer[0] = 'x';
    fprintf(stdout, "CHILD (%d) done.\n", getpid());
    exit(0);
}

void signalHandler (int signal, siginfo_t *si, void *unused) {

    // Print who I am.
    fprintf(stdout, "I am process %d!\n", getpid());
    
    // Print location of fault.
    fprintf(stdout, "SIGSEGV at 0x%lx\n", (long)si->si_addr);

    // Actually let it write.
    if (mprotect(si->si_addr, pagesize, PROT_WRITE) == -1) {
        fprintf(stderr, "Error: Couldn't grant writing-permissions!\n");   
    }
}


int main (int argc, char *argv[]) {
    // Set the page size.
    pagesize = sysconf(_SC_PAGE_SIZE);

        /*********** Setup the Signal *************/
    // The signal
    struct sigaction mySignal;

    mySignal.sa_flags = SA_SIGINFO;         // Take three args.
    sigemptyset(&mySignal.sa_mask);         // Clear mask for new signal.
    mySignal.sa_sigaction = signalHandler;  // Set callback.
    sigaction(SIGSEGV, &mySignal, NULL);    // Intercept SIGSEGV.

    /*********** Allocate "shared" page. *************/

    // Allocate and align memory along page starting address. Required
    // for use of mprotect (address must be aligned in its parameters).
    if (posix_memalign(&page, pagesize, pagesize) != 0) {
        fprintf(stderr, "Fuck\n");
        exit(EXIT_FAILURE);
    }

    // Protect the page against any writing.
    if (mprotect(page, pagesize, PROT_READ) == -1) {
        fprintf(stderr, "Warning: the page can't be protected!\n");   
    }


    // FORK: If child -> try to write to the page. If parent, setup and handle
    // signal.
    int pid;
    if ((pid = fork()) == 0) {
        runChild();
    }

    // Wait for the child to try to write.
    int status;
    waitpid(pid, &status, 0);

    fprintf(stdout, "PARENT: Done...\n");
    exit(EXIT_SUCCESS);
}