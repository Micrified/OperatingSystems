#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

#define NO_READ         0
#define NO_WRITE        1

#define A_WHOAMI        0
#define B_WHOAMI        1

// The size of a page.
int pagesize;

// Page sized buffer of shared memory.
int *shared;

// [Child] File-descriptor to read from when receiving from parent.
int in_fd;

// [Child] File-descriptor to write to when sending stuff to parent.
int out_fd;
 
/*
 *****************************************************************************
 *                          Child Signal Handlers                            *
 *****************************************************************************
*/

/* The call that caused sigsegHandler to fire */
int sigsegReason = NO_READ;

/* SIGSEGV Handler (child friendly) */
void sigsegHandler (int sig, siginfo_t *si, void *ignr) {
    if (sigsegReason == NO_READ) {
        fprintf(stdout, "[ %d ] :: Segfault (no read?) Setting READ protections!\n", getpid());
        mprotect(si->si_addr, pagesize, PROT_READ);
    } else {
        // SEND DATA (which you can read) TO PARENT.
        fprintf(stdout, "[ %d ] :: Segfault (no write?) Setting, Then Sending data!\n", getpid());
        mprotect(si->si_addr, pagesize, PROT_WRITE);
        if (write(out_fd, shared, pagesize) != pagesize) {
            fprintf(stderr, "[ %d ] :: Problem writing data?\n", getpid());
        }
    }
    sigsegReason = 1 - sigsegReason; // Invert reason for next catch.
}

/* Parent Synchronization Interrupt */
void sigsyncHandler (int sig, siginfo_t *si, void *ignr) {
    fprintf(stdout, "[ %d ] :: Sync call. Reading in!\n", getpid());
    mprotect(shared, pagesize, PROT_WRITE); // Temporarily allow writes.
    // READ DATA (from parent pipe) INTO SHARED.
    if (read(in_fd, shared, pagesize) != pagesize) {
        fprintf(stdout, "[ %d ] :: Problem reading data?\n", getpid());
    }
    mprotect(shared, pagesize, PROT_READ); // Re-enable to read-only (assumes its stuck busy waiting).
}

/*
 *****************************************************************************
 *                               Child Routine                               *
 *****************************************************************************
*/

// Set whoami when calling. sharedTurnVariable is just "shared" alias in global.
void pingPong (const int whoami, int *sharedTurnVariable) {
    for (int count = 0; count < 5; count++) {
        while (whoami != *sharedTurnVariable)
            ; // Busy waiting.
        printf(whoami == 0 ? "Ping\n" : "...Pong\n");
        *sharedTurnVariable = 1 - whoami;
    }
    exit(EXIT_SUCCESS);
}

/*
 *****************************************************************************
 *                               Parent Routine                              * 
 *****************************************************************************
*/

/* While the children aren't dead, wait in lockstep 
 * 1. Both a_out and b_out should be non-blocking.
*/
void parentProcess (int a_pid, int b_pid, int a_in, int a_out, int b_in, int b_out) {

    // Check (nonblocking) if children are still going. 
    // waitpid returns zero for childs whose state hasn't changed if WNOHANG specified.
    // I interpret this as meaning they are still open.
    fprintf(stdout, "[Parent %d]: Waiting for %d and %d to finish pingpong!\n", getpid(), a_pid, b_pid);
    while (waitpid(a_pid, NULL, WNOHANG) == 0 && waitpid(b_pid, NULL, WNOHANG) == 0) {

        // Check A. 
        if (read(a_out, shared, pagesize) == pagesize) {
            fprintf(stdout, "[Parent %d] :: Read something from A -> Trying to write now.\n", getpid());
            if (write(b_in, shared, pagesize) == pagesize) {
                kill(b_pid, SIGALRM); // Make B sync.
                fprintf(stdout, "[Parent %d] :: Sent SIGALRM to B!\n", getpid());
            } else {
                fprintf(stdout, "[Parent %d] :: ALERT! Couldn't write A's message to B!\n", getpid());
            }
        }

        // Check B.
        if (read(b_out, shared, pagesize) == pagesize) {
            fprintf(stdout, "[Parent %d] :: Read something from B -> Trying to write now.\n", getpid());
            if (write(a_in, shared, pagesize) == pagesize) {
                kill(a_pid, SIGALRM); // Make A sync.
                fprintf(stdout, "[Parent %d] :: Sent SIGALRM to A!\n", getpid());
            } else {
                fprintf(stdout, "[Parent %d] :: ALERT! Couldn't write B's message to A!\n", getpid());
            }
        }
        //fprintf(stdout, "[Parent %d] :: Looping again -->!\n", getpid());
    }

    exit(EXIT_SUCCESS);
}

/*
 *****************************************************************************
 *                             Setup Routines                                *   
 *****************************************************************************
*/

/* Closes all file-descriptors for children except those given in the parameters */
void childCloseAllExcept (int in_fd_idx, int out_fd_idx, int fds[8]) {
    for (int i = 0; i < 8; i++) {
        if (i == in_fd_idx || i == out_fd_idx) {
            continue;
        } else {
            close(fds[i]);
        }
    }
}

/* Closes all file-descriptors for the parent except those given in the parameters */
void parentCloseAllExcept (int to_a_idx, int from_a_idx, int to_b_idx, int from_b_idx, int fds[8]) {
    for (int i = 0; i < 8; i++) {
        if (i == to_a_idx || i == from_a_idx || i == to_b_idx || i == from_b_idx) {
            continue;
        } else {
            close(fds[i]);
        }
    }
}

/* Sets a file-descriptor to be non-blocking */
void setNonBlocking (int fd) {
    fprintf(stdout, "[Parent %d] :: File-Descriptor %d set to non-blocking!\n", getpid(), fd);
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main (void) {

    /*
     **************************************************************************
     *                          Shared Memory Setup                           *
     **************************************************************************
    */

    // Set the pagesize.
    pagesize = sysconf(_SC_PAGE_SIZE);

    // Allocate shared memory, and align it so that mprotect can work properly.
    if (posix_memalign((void **)&shared, pagesize, 1 * pagesize) != 0) {
        fprintf(stderr, "[Parent %d] :: Aligned memory allocation failed!\n", getpid());
        exit(EXIT_FAILURE);
    }

    // Set the initial value of shared (hopefully write-able at this point).
    *shared = 0;

    // Set initial protection to PROT_READ (read-only).
    if (mprotect(shared, pagesize, PROT_READ) == -1) {
        fprintf(stderr, "[Parent %d] :: Memory protection failed!\n", getpid());
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "[Parent %d] :: Page Setup Done (pagesize = %d)\n", getpid(), pagesize);
    /*
     **************************************************************************
     *                          Shared Signal Handlers                        *
     **************************************************************************
    */
    struct sigaction segHandler;    // Handler for READ/WRITE faults.
    struct sigaction syncHandler;   // Handler for sync order from parent to child.

    // Configure signal-handler args.
    segHandler.sa_flags = syncHandler.sa_flags = SA_SIGINFO;

    // Reset signal-mask.
    sigemptyset(&segHandler.sa_mask); sigemptyset(&syncHandler.sa_mask);

    // Setup signal handlers.
    segHandler.sa_sigaction = sigsegHandler;
    syncHandler.sa_sigaction = sigsyncHandler;

    // Setup signals to be intercepted.
    sigaction(SIGSEGV, &segHandler, NULL); 
    sigaction(SIGALRM, &syncHandler, NULL);

    /*
     **************************************************************************
     *                               Setup pipes                              *
     **************************************************************************
    */

    // The PIDs of A and B.
    int a_pid, b_pid;

    // One bidirectional (2 * 2) pipe for each child (2) to the parent.
    // [0,1] :: Parent -> A :: Parent must close 0, A must close 1.
    // [2,3] :: A -> Parent :: Parent must close 3, A must close 2.
    // [4,5] :: Parent -> B :: Parent must close 4, B must close 5.
    // [6,7] :: B -> Parent :: Parent must close 7, B must close 6.
    int pds[8];
    
    // Create pipes.
    int wasErr = pipe(pds) + pipe(pds + 2) + pipe(pds + 4) + pipe(pds + 6);
    if (wasErr != 0) {
        fprintf(stderr, "Error creating the pipes in parent!\n");
        exit(EXIT_FAILURE);
    }

    // Make sure that pipes that the parent checks (incoming from A and B) are set to not block.
    setNonBlocking(pds[2]); // Reading port on which A will write to the parent
    setNonBlocking(pds[6]); // Reading port on which B will write to the parent

    // A: Fork, Set (I/O), Close irrelevant descriptors. Run pingpong.
    if ((a_pid = fork()) == 0) {
        in_fd = pds[0];
        out_fd = pds[3];
        childCloseAllExcept(0, 3, pds); // Note: uses indices not fds.
        fprintf(stdout, "[Child A: %d] :: Reading from %d and writing to %d!\n", getpid(), in_fd, out_fd);
        pingPong(A_WHOAMI, shared);
    }

    // B: Fork, Set (I/O), Close irrelevant descriptors. Run pingpong.
    if ((b_pid = fork()) == 0) {
        in_fd = pds[4];
        out_fd = pds[7];
        childCloseAllExcept(4, 7, pds);
        fprintf(stdout, "[Child B: %d] :: Reading from %d and writing to %d!\n", getpid(), in_fd, out_fd);
        pingPong(B_WHOAMI, shared);
    }


    // Close all irrelevant desciptors to the parent (to_A, from_a, to_B, from_B).
    parentCloseAllExcept(1,2,5,6, pds); 

    fprintf(stdout, "[Parent %d] :: Closing all descriptors except [%d, %d, %d, %d]!\n", getpid(), pds[1], pds[2], pds[5], pds[6]);

    // Launch parent program.
    parentProcess(a_pid, b_pid, pds[2], pds[1], pds[6], pds[5]);
}
