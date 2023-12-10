#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

void ParentProcess(int *);
void ChildProcess(int *);

int main() {
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        fprintf(stderr, "*** shmget error (server) ***\n");
        exit(1);
    }

    ShmPTR = (int *)shmat(ShmID, NULL, 0);
    if (*ShmPTR == -1) {
        fprintf(stderr, "*** shmat error (server) ***\n");
        exit(1);
    }

    ShmPTR[0] = 0; // BankAccount
    ShmPTR[1] = 0; // Turn

    printf("Server has attached the shared memory...\n");

    printf("Server is about to fork a child process...\n");
    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "*** fork error (server) ***\n");
        exit(1);
    } else if (pid == 0) {
        ChildProcess(ShmPTR);
        exit(0);
    } else {
        ParentProcess(ShmPTR);
        wait(&status);
        printf("Server has detected the completion of its child...\n");
    }

    shmdt((void *)ShmPTR);
    printf("Server has detached its shared memory...\n");
    shmctl(ShmID, IPC_RMID, NULL);
    printf("Server has removed its shared memory...\n");
    printf("Server exits...\n");

    return 0;
}

void ParentProcess(int *SharedMem) {
    for (int i = 0; i < 25; ++i) {
        sleep(rand() % 6);

        int account;
        do {
            while (SharedMem[1] != 0)
                ; // Wait
            account = SharedMem[0];
        } while (account > 100);

        int balance = rand() % 101;

        if (balance % 2 == 0) {
            // Deposit money
            SharedMem[0] += balance;
            printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, SharedMem[0]);
        } else {
            printf("Dear old Dad: Doesn't have any money to give\n");
        }

        SharedMem[1] = 1;
    }
}

void ChildProcess(int *SharedMem) {
    for (int i = 0; i < 25; ++i) {
        sleep(rand() % 6);

        int account;
        do {
            while (SharedMem[1] != 1)
                ; // Wait
            account = SharedMem[0];
        } while (account < 50);

        int balance = rand() % 51;

        if (balance <= account) {
            // Withdraw money
            SharedMem[0] -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, SharedMem[0]);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        SharedMem[1] = 0;
    }
}