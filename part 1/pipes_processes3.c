#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

void executeCommand(const char *command, char *const args[], int inputFd, int outputFd) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process

        // Redirect standard input if inputFd is provided
        if (inputFd != -1) {
            dup2(inputFd, 0);
            close(inputFd);
        }

        // Redirect standard output if outputFd is provided
        if (outputFd != -1) {
            dup2(outputFd, 1);
            close(outputFd);
        }

        // Execute the command
        if (execvp(command, args) == -1) {
            perror("execvp");
            _exit(1);
        }
    } else if (pid == -1) {
        perror("fork");
        _exit(1);
    }
}

int main(int argc, char **argv) {
    int pipefd1[2], pipefd2[2];

    // Make the first pipe (fds go in pipefd1[0] and pipefd1[1])
    if (pipe(pipefd1) == -1) {
        perror("pipe");
        return 1;
    }

    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", argv[1], NULL};
    char *sort_args[] = {"sort", NULL};

    // Fork the first child (P1)
    executeCommand("cat", cat_args, -1, pipefd1[1]);

    // Parent process handles P2 and P3

    // Make the second pipe (fds go in pipefd2[0] and pipefd2[1])
    if (pipe(pipefd2) == -1) {
        perror("pipe");
        return 1;
    }

    // Fork the second child (P2)
    executeCommand("grep", grep_args, pipefd1[0], pipefd2[1]);

    // Parent continues to handle P3

    // Fork the third child (P3)
    executeCommand("sort", sort_args, pipefd2[0], -1);

    // Close unused ends of pipes
    close(pipefd1[0]);
    close(pipefd1[1]);
    close(pipefd2[0]);
    close(pipefd2[1]);

    // Wait for P2 and P3 to finish
    wait(NULL);
    wait(NULL);

    return 0;
}
