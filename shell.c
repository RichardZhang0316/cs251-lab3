/*
* Richard Zhang
* CSC 251
* Lab 3
* Mar 21 2024
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

// define the maximum length of a command
#define maxLength 1024
#define MAX_HISTORY 1000

// function declarations go here
void convert(char* input, char** argv);
void controlCDisabler(int signal);
void showHistory();

int main() {
    char input[maxLength];
    // we set the size of argv to 100 to ensure it's long enough
    char* argv[100];
    // the pid of a process
    pid_t pid;
    // the index implying if a command need to be executed concurrently
    int background;
    // the maximum number of commands can be in a pipe is 3. Thus, we define a char array to store them
    char* commands[3];
    // we also want to create two places to save the read & write file that would be used by other commands in a pipe. Thus, we create file descriptors here.
    int fileDescriptors[2];

    // disable control C
    signal(SIGINT, controlCDisabler);

    // the initial prompt
    printf("Welcome to the shell!\n");

    // enter the infinite loop to mimic a shell
    while (1) {
        background = 0;
        // prompt
        printf("shell >> ");
        // get the input
        fgets(input, maxLength, stdin);

        // record the input
        FILE* file = fopen(".myhistory", "a");
        if (file != NULL) {
            fprintf(file, "%s\n", input);
            fclose(file);
        }

        // show history
        if (strcmp(input, "history\n") == 0) {
            showHistory();
            continue;
        }

        // erase history
        if (strcmp(input, "erase history\n") == 0) {
            FILE* file = fopen(".myhistory", "w");
            if (file != NULL) {
                fclose(file);
            }
            continue;
        }

        // the only way to exit the program is writing exit in the command
        if (strcmp(input, "exit\n") == 0) {
            // terminate the program
            return 0;
        }

        // firstly, check if the command need to be executed concurrently
        if (input[strlen(input) - 2] == '&') {
            background = 1;
            // we want to remove the & symbol to ensure the normal execution of the program
            input[strlen(input) - 2] = '\0';
        }

        // split the input, check how many commands are in it, and store them
        int numCommands = 0;
        // split the input using '|'
        char* command = strtok(input, "|");
        while (command != NULL && numCommands < 3) {
            commands[numCommands++] = command;
            command = strtok(NULL, "|");
        }

        // initiate a boolean value to check if this command is the first command in a line of input
        int isFirst = 1;
        // the location of the previous output (or the read file)
        int previousOutputFileLocation = 0;

        // compared to lab 2, we need to introduce a for loop here because there could be more than one command in a line of input
        for (int i = 0; i < numCommands; i++) {
            // convert the command to an array of arguments
            convert(commands[i], argv);

            // if the current command is not the last one in a line of input, we pipe it so that the later command can use its output as its input
            if (i + 1 < numCommands) {
                pipe(fileDescriptors);
            }

            // fork and see is there is an child process
            pid = fork();
            if (pid < 0) {
                // this implies there is an error
                perror("fork() failed");
                exit(1);
            } else if (pid == 0) {
                // this implies this is a child process
                // check if this is the first command in the input. If not, we need to redirect the input file
                if (!isFirst) {
                    // not the first command
                    dup2(previousOutputFileLocation, 0); // redirect the input to the output of the previous command
                    close(previousOutputFileLocation); // no longer needed
                }
                if (i + 1 < numCommands) {
                    // not the last command
                    dup2(fileDescriptors[1], 1); // redirect the output to the output file descriptor
                    close(fileDescriptors[0]); // no longer needed
                    close(fileDescriptors[1]); // no longer needed
                }
                // in this case, we run the execvp() function. In most cases, the first argument in a command is the function to be called, and the rest is the function's arguments, such as flags
                if (execvp(argv[0], argv) < 0) {
                    // this means the execution is failed
                    perror("execvp() failed");
                    exit(2);
                }
            } else {
                // this implies this is the parent process
            
                previousOutputFileLocation = fileDescriptors[0];
                isFirst = 0;
            }   

        }

        if (background == 0) {
            // this means we need to wait for the child process to complete itself.
            // wait for all the child process to complete
            for (int i = 0; i < numCommands; i++) {
                wait(NULL);
            }
        }
   
    }
    return 0;
}

// this function converts a command to an array of arguments
void convert(char* input, char** argv) {
    // we need to traverse the input, so we first set the position index to 0. This means we will starts from 0
    int index = 0;
    char* argument;
    // we want to use space as our delimiter to split a command
    // firstly, we need to get the first argument in a command. The second argument of strtok() shows all the possible delimiter in a command, including space, tab, etc.
    argument = strtok(input, " \t\r\n");
    while (argument != NULL) {
        argv[index] = argument;
        index++;
        // get the next argument from the rest of the command
        argument = strtok(NULL, " \t\r\n");
    }
    // for the last cell of argv, we want to insert NULL to it to mark it's the end of the variable
    argv[index] = NULL;
}

// the function that diable control C
void controlCDisabler(int signal) {
    printf(" I cannot be terminated using ^C. Please type 'exit' if you want to exit.\n");
}

// the function that would show the history of the shell
void showHistory() {
    FILE* file = fopen(".myhistory", "r");
    char line[1024];
    // traverse .myhistory and print every line
    if (file != NULL) {
        while (fgets(line, sizeof(line), file)) {
            printf("%s", line);
        }
    }
}