#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

#define NO_READ         0
#define NO_WRITE        1

#define A_WHOAMI        0
#define B_WHOAMI        1

// The identity of the child.
int childIdentity;

// The size of a page.
int pagesize;

// Page sized buffer of shared memory.
void *shared;

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
    int bytes;
    if (sigsegReason == NO_READ) {
        //fprintf(stdout, "[ %d ] :: Segfault (no read?) Setting READ protections!\n", getpid());
        if (mprotect(si->si_addr, pagesize, PROT_READ) == -1) {
            fprintf(stderr, "[ %d ] ALERT: Bad mprotect when setting PROT_READ!\n", getpid());   
        }
    } else {
        // SEND DATA (which you can read) TO PARENT.
        //fprintf(stdout, "[ %d ] :: Segfault (no write?) Setting, Then Sending data!\n", getpid());
        if (mprotect(si->si_addr, pagesize, PROT_WRITE) == -1) {
            fprintf(stderr, "[ %d ] :: ALERT: Bad mprotect when setting PROT_WRITE!\n", getpid());
        }

        // Inversion (due to write-signal behavior)
        int *p = shared;
        *p = 1 - childIdentity;


        if ((bytes = write(out_fd, shared, pagesize)) != pagesize) {
            fprintf(stderr, "[ %d ] :: ALERT: Bad write. Bytes = %d\n", getpid(), bytes);
        }
    }
    sigsegReason = 1 - sigsegReason; // Invert reason for next catch.
}

/* Parent Synchronization Interrupt */
void sigsyncHandler (int sig, siginfo_t *si, void *ignr) {
    if (mprotect(shared, pagesize, PROT_WRITE) == -1) {
        fprintf(stderr, "[ %d ] :: Problem setting protections to PROT_WRITE! Reason = \"%s\"\n", getpid(), strerror(errno));
    } 
    // Temporarily allow writes.
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
        mprotect(shared, pagesize, PROT_READ);
        while (whoami != *sharedTurnVariable) {
            sleep(1);
        }
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
void parentProcess (int a_pid, int b_pid, int to_a, int from_a, int to_b, int from_b) {
    int bytes = 0;
    int turn = 0; // Start with waiting on A. 
    // Check (nonblocking) if children are still going. 
    // waitpid returns zero for childs whose state hasn't changed if WNOHANG specified.
    // I interpret this as meaning they are still open.
    while (waitpid(a_pid, NULL, WNOHANG) == 0 && waitpid(b_pid, NULL, WNOHANG) == 0) {

        // Check A. 
        if (turn == 0 && (bytes = read(from_a, shared, pagesize)) == pagesize) {
            //fprintf(stdout, "[Parent %d] :: Read bytes from A. Sending to B!\n", getpid());
            if (write(to_b, shared, pagesize) == pagesize) {
                kill(b_pid, SIGALRM); // Make B sync.
            } else {
                fprintf(stdout, "[Parent %d] :: ALERT! Couldn't write A's message to B!\n", getpid());
            }
        }

        // Check B.
        if (turn == 1 && (bytes = read(from_b, shared, pagesize)) == pagesize) {
            //fprintf(stdout, "[Parent %d] :: Read bytes from B. Sending to A!\n", getpid());
            if (write(to_a, shared, pagesize) == pagesize) {
                kill(a_pid, SIGALRM); // Make A sync.
            } else {
                fprintf(stdout, "[Parent %d] :: ALERT! Couldn't write B's message to A!\n", getpid());
            }
        }

        // Invert turn.
        turn = 1 - turn;

        // Check for read error.
        if (bytes == -1) {
            fprintf(stderr, "[Parent %d] :: Error reading from pipes -> \"%s\"\n", getpid(), strerror(errno));
            exit(EXIT_FAILURE);
        }
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

int main (void) {

    /*
     **************************************************************************
     *                          Shared Memory Setup                           *
     **************************************************************************
    */

    // Set the pagesize.
    pagesize = sysconf(_SC_PAGE_SIZE);

    // Allocate shared memory, and align it so that mprotect can work properly.
    if (posix_memalign(&shared, pagesize, 1 * pagesize) != 0) {
        fprintf(stderr, "[Parent %d] :: Aligned memory allocation failed!\n", getpid());
        exit(EXIT_FAILURE);
    }

    // Set the initial value of shared (hopefully write-able at this point).
    memset(shared, 0, pagesize);

    // Set initial protection to PROT_NONE (no rights).
    if (mprotect(shared, pagesize, PROT_NONE) == -1) {
        fprintf(stderr, "[Parent %d] :: Memory protection failed!\n", getpid());
        exit(EXIT_FAILURE);
    }

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

    /*************************************************************************/


    // A: Fork, Set (I/O), Close irrelevant descriptors. Run pingpong.
    if ((a_pid = fork()) == 0) {
        in_fd = pds[0];
        out_fd = pds[3];
        childCloseAllExcept(0, 3, pds); // Note: uses indices not fds.
        //fprintf(stdout, "[Child A: %d] :: Reading from %d and writing to %d!\n", getpid(), in_fd, out_fd);
        childIdentity = A_WHOAMI;
        pingPong(A_WHOAMI, shared);
    }


    // B: Fork, Set (I/O), Close irrelevant descriptors. Run pingpong.
    if ((b_pid = fork()) == 0) {
        in_fd = pds[4];
        out_fd = pds[7];
        childCloseAllExcept(4, 7, pds);
        //fprintf(stdout, "[Child B: %d] :: Reading from %d and writing to %d!\n", getpid(), in_fd, out_fd);
        childIdentity = B_WHOAMI;
        pingPong(B_WHOAMI, shared);
    }

    /******************* Swap Code Sections if Blocks! **********************/

    /*************************************************************************/

    // Set parent buffer protections.
    if (mprotect(shared, pagesize, PROT_WRITE | PROT_READ) == -1) {
        fprintf(stderr, "[Parent %d] :: Memory protection failed!\n", getpid());
        exit(EXIT_FAILURE);
    }

    // Close all irrelevant desciptors to the parent (to_A, from_A, to_B, from_B).
    parentCloseAllExcept(1,2,5,6, pds); 

    // Delay the parent a bit to let all forks be started.

    // Launch parent program: (int a_pid, int b_pid, int to_a, int from_a, int to_b, int from_b)
    parentProcess(a_pid, b_pid, pds[1], pds[2], pds[5], pds[6]);
}