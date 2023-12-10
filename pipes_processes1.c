#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>

int main()
{
    // We use two pipes
    // First pipe to send input string from parent
    // Second pipe to send concatenated string from child

    int fd1[2];  // Used to store two ends of the first pipe
    int fd2[2];  // Used to store two ends of the second pipe

    char fixed_str[] = "howard.edu";
    char input_str[100];
    pid_t p;

    if (pipe(fd1) == -1 || pipe(fd2) == -1)
    {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }

    printf("Enter a string to concatenate:");
    scanf("%s", input_str);

    p = fork();

    if (p < 0)
    {
        fprintf(stderr, "fork Failed");
        return 1;
    }

    // Parent process
    else if (p > 0)
    {
        close(fd1[0]);  // Close reading end of the first pipe
        close(fd2[1]);  // Close writing end of the second pipe

        // Write input string and close writing end of the first pipe.
        write(fd1[1], input_str, strlen(input_str) + 1);

        // Wait for child to send a string
        wait(NULL);

        // Read the concatenated string from the second pipe
        char concat_str[100];
        read(fd2[0], concat_str, 100);

        printf("Concatenated string: %s\n", concat_str);

        // Close both ends of pipes
        close(fd1[1]);
        close(fd2[0]);
    }

    // Child process
    else
    {
        close(fd1[1]);  // Close writing end of the first pipe
        close(fd2[0]);  // Close reading end of the second pipe

        // Read a string using the first pipe
        char concat_str[100];
        read(fd1[0], concat_str, 100);

        // Concatenate a fixed string with it
        int k = strlen(concat_str);
        int i;
        for (i = 0; i < strlen(fixed_str); i++)
            concat_str[k++] = fixed_str[i];

        concat_str[k] = '\0';   // String ends with '\0'

        // Write the concatenated string to the second pipe
        write(fd2[1], concat_str, strlen(concat_str) + 1);

        // Close both ends of pipes
        close(fd1[0]);
        close(fd2[1]);

        exit(0);
    }

    return 0;
}
