/********************************************************************************************
    example: ./a.out "Hi There"
*********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>

/*
//  Parent: reads from P1_READ, writes on P1_WRITE
//  Child:  reads from P2_READ, writes on P2_WRITE
*/
#define P1_READ     0
#define P2_WRITE    1
#define P2_READ     2
#define P1_WRITE    3

// the total number of pipe *pairs* we need
#define NUM_PIPES   2

char *revcase(char *buffer) {
    int i;
    int len = strlen(buffer);
    for (i = 0; i < len; i++) {
        if (isupper(buffer[i]))
            buffer[i] = tolower(buffer[i]);
        else if (islower(buffer[i]))
            buffer[i] = toupper(buffer[i]);
    }
    return buffer;
}

int inputValidation(int argc, char *argv[]) {

    int i;          //Declare counter variable

    bool c = false; //Declare boolean flag using imported <stdbool.h>

    char str[strlen(argv[1])];  //Declare str

    strcpy(str, argv[1]); //copy argument into str

    if (argc != 2) {     // check to see if we have enough arguments to continue
        // Prompt user of correct usage
        fprintf(stderr, "\nUsage: %s <string> or <'string 1, string 2', ..., string n'> for multiple strings\n",
                argv[0]);

        exit(EXIT_FAILURE);    //Exit on improper input

    } else {
        //loop through our string
        for (i = 0; i < strlen(str); i++) {
            //if any any char is a reversible character
            if (isalpha((int) str[i])) {

                c = true; //set the flag to true
            }
        }

        if (c == false) { //If flag is false input does not contain any reversible charachters

            printf("\nSorry, The string you entered did NOT contain any Alphabetical Characters\nRun me again, with at least 1 Alphabetical character\n\n");

            exit(EXIT_FAILURE); //Exit on improper input

        }
        return (0);
    }
}

int main(int argc, char *argv[]) {

    assert(argc > 1);

    int fd[2 * NUM_PIPES];    //Declare int[] of file descriptors

    int len, i;             //Declare length and integer for count

    pid_t pid;              //Declare process id

    char parent[strlen(argv[1])];   //Declare Parent array

    char child[strlen(argv[1])];    //Declare Child array

    if (inputValidation(argc, argv) == 0) /* Check for proper input */

        strcpy(parent, argv[1]);

    // create all the descriptor pairs we need
    for (i = 0; i < NUM_PIPES; ++i) {
        if (pipe(fd + (i * 2)) < 0) {
            perror("Failed to allocate pipes\n");
            exit(EXIT_FAILURE);
        }
    }

    // fork() returns 0 for child process, child-pid for parent process.
    if ((pid = fork()) < 0) {
        perror("Failed to fork process\n");
        return EXIT_FAILURE;
    }

    // if the pid is zero, this is the child process
    if (pid == 0) {
        // Child. Start by closing descriptors we
        //  don't need in this process
        close(fd[P1_READ]);
        close(fd[P1_WRITE]);

        // wait for parent to send us a value
        len = read(fd[P2_READ], &child, len);
        if (len < 0) {
            perror("Child: Failed to read data from pipe\n");
            exit(EXIT_FAILURE);
        } else {

            // report pid to console
            printf("Child: Read the message from Pipe 1: %s. \nChild: Write the modified message to Pipe 2.\n",
                   argv[1]);

            // send the message to toggleString and write it to pipe//
            if (write(fd[P2_WRITE], revcase(child), strlen(child)) < 0) {
                perror("Child: Failed to write response value\n");
                exit(EXIT_FAILURE);
            }
        }

        // finished. close remaining descriptors.
        close(fd[P2_READ]);
        close(fd[P2_WRITE]);

        return EXIT_SUCCESS;
    }

    // Parent. close unneeded descriptors
    close(fd[P2_READ]);
    close(fd[P2_WRITE]);

    // send a value to the child
    printf("Parent: Write the message to Pipe 1.\n");
    if (write(fd[P1_WRITE], argv[1], strlen(argv[1])) != strlen(argv[1])) {
        perror("Parent: Failed to send value to child \n");
        exit(EXIT_FAILURE);
    }

    // now wait for a response
    len = read(fd[P1_READ], &parent, strlen(parent));
    if (len < 0) {
        perror("Parent: failed to read value from pipe\n");
        exit(EXIT_FAILURE);
    } else {
        // report what we received
        printf("Parent: Read the modified message from Pipe 2: %s. \n", revcase(argv[1]));
    }

    // close down remaining descriptors
    close(fd[P1_READ]);
    close(fd[P1_WRITE]);

    // wait for child termination
    wait(NULL);

    return EXIT_SUCCESS;
}