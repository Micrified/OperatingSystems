#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>

// Shared variable, size of shared variable.
int *shared;
int sharedSize = sizeof(int);

// Parent-Process PID.
int parentPID;

// Child-Process identity.
int whoami;

// Parent handler: SIGUSR1 & SIGUSR2.
void parentHandler (int signal, siginfo_t *sigInfo, void *unused) {
    if (signal == SIGUSR1) {
        fprintf(stdout, "PARENT [%d] :: SIGUSR1!\n");
    } else {
        fprintf(stdout, "PARENT [%d] :: SIGUSR2!\n");
    }
}

// Child handler: SIGSEGV.
void childHandler (int signal, siginfo_t sigInfo, void *unused) {
    int sig = (whoami == 0) ? SIGUSR1 : SIGUSR2;
    kill(parentPID, sig);
}


int main (void) {

    
}