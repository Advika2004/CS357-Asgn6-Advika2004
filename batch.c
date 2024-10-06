#include "batch.h"


pid_t *PIDARRAY = 0; // array to hold all my PIDs
char ***commandsArray = NULL; // triple star array to hold all the commands
int programCount = 0;
int maxCommands = 128;
int vmode = 0; // indicates that verbose mode should be on
int finalStatus = EXIT_SUCCESS; // Final exit status

void printUsage() {
    fprintf(stderr, "usage: ./batch [-n N] [-e] [-v] -- COMMAND [-- COMMAND ...]\n");
    exit(EXIT_FAILURE);
}

void killThemAll(int signal) {
    for (int i = 0; i < programCount; i++) {
        if (PIDARRAY[i] > 0) { // if my process is still valid and going
            kill(PIDARRAY[i], signal); // kill that process
            waitpid(PIDARRAY[i], NULL, 0); // wait for that process in the parent so that there are no zombies
            if (vmode == 1) { // if I'm in verbose mode, need to print out the stuff that terminates
                fprintf(stderr, "- %s", commandsArray[i][0]); // will print out the - for the termination
                for (int j = 1; commandsArray[i][j] != NULL; j++) {
                    fprintf(stderr, " %s", commandsArray[i][j]); // will go through and print out rest of arguments in command
                }
                fprintf(stderr, "\n"); // will print a new line at the end and will flush buffer
            }
            PIDARRAY[i] = -1; // will indicate that that process has been terminated
        }
    }

    // must free the stuff I am using if I am terminating
    for (int i = 0; i < programCount; i++) {
        if (commandsArray[i] != NULL) {
            free(commandsArray[i]); // free all the commands within the array
        }
    }
    free(commandsArray); // free the array itself
    free(PIDARRAY); // free the array of PIDs
    exit(EXIT_FAILURE); // return with exit failure
}

void signalHandlerPrep(int signum) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = killThemAll;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    if (sigaction(signum, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    int maxArgsAtOnce = -1;
    int errorFound = 0;
    int commandLineIndex = -1;
    int runningJobs = 0;

    if (argc < 2) {
        printUsage();
    }

    // allocating memory for all possible commands and max commands
    commandsArray = malloc(sizeof(char **) * maxCommands);
    PIDARRAY = malloc(sizeof(pid_t) * maxCommands);

    // parsing my arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) { // once I encounter the --, the arguments have started
            commandLineIndex = i + 1; // marks where the command lines start
            break;
        } else if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 >= argc) {
                printUsage();
            }
            maxArgsAtOnce = atoi(argv[++i]);
            if (maxArgsAtOnce < 1) {
                printUsage();
            }
        } else if (strcmp(argv[i], "-e") == 0) {
            errorFound = 1;
        } else if (strcmp(argv[i], "-v") == 0) {
            vmode = 1;
        } else {
            printUsage();
        }
    }

    if (commandLineIndex == -1 || commandLineIndex >= argc) { // if there were no arguments after --
        printUsage();
    }

    // prep these bad boys
    signalHandlerPrep(SIGINT);
    signalHandlerPrep(SIGQUIT);
    signalHandlerPrep(SIGTERM);

    for (int i = commandLineIndex; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {
            programCount++; // counting how many programs I have
            continue;
        }
        // handle resizing the array if needed
        if (programCount >= maxCommands) {
            maxCommands *= 2;
            commandsArray = realloc(commandsArray, sizeof(char **) * maxCommands);
            PIDARRAY = realloc(PIDARRAY, sizeof(pid_t) * maxCommands); // if I run out of room in my arrays, double their size
        }

        int arg_count = 0;
        commandsArray[programCount] = malloc(sizeof(char *) * (argc - commandLineIndex));
        while (i < argc && strcmp(argv[i], "--") != 0) {
            commandsArray[programCount][arg_count] = argv[i];
            arg_count++;
            i++;
        }
        commandsArray[programCount][arg_count] = NULL;
        programCount++;
    }

    if (maxArgsAtOnce == -1) { // if there was no -n specified, then
        maxArgsAtOnce = programCount; // number of args - number of flags = number of programs to run
    }

    for (int commandidx = 0; commandidx < programCount; commandidx++) {
        if (runningJobs >= maxArgsAtOnce) { // if I am greater than the max jobs that are running
            int status;
            pid_t pid = wait(&status); // wait for one of the jobs to be done
            runningJobs--; // decrement the running jobs count
            for (int i = 0; i < programCount; i++) { // for every program
                if (PIDARRAY[i] == pid) { // check if that process is the one that just ended
                    if (vmode) { // if in verbose mode, print out the terminating process
                        fprintf(stderr, "- %s", commandsArray[i][0]);
                        for (int j = 1; commandsArray[i][j] != NULL; j++) {
                            fprintf(stderr, " %s", commandsArray[i][j]);
                        }
                        fprintf(stderr, "\n");
                    }
                    PIDARRAY[i] = -1; // mark this process as handled
                    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
                        finalStatus = EXIT_FAILURE; // update the final exit status
                    }
                    if (errorFound && (pid == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS)) {
                        killThemAll(SIGTERM);
                    }
                    break;
                }
            }
        }

        if (vmode) {
            fprintf(stderr, "+ %s", commandsArray[commandidx][0]); // printing out the command that is running
            for (int i = 1; commandsArray[commandidx][i] != NULL; i++) {
                fprintf(stderr, " %s", commandsArray[commandidx][i]);
            }
            fprintf(stderr, "\n");
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            killThemAll(SIGTERM);
        } else if (pid == 0) {
            // within the child process now
            execvp(commandsArray[commandidx][0], commandsArray[commandidx]); // if replacing the thing fails
            perror(commandsArray[commandidx][0]); // perror
            exit(EXIT_FAILURE);
        } else {
            // else in the parent process
            PIDARRAY[commandidx] = pid; // store the pid into the array
            runningJobs++; // increment how many jobs are going on
        }

        if (runningJobs >= maxArgsAtOnce) {
            int status;
            pid_t pid = wait(&status);
            runningJobs--;
            for (int i = 0; i < programCount; i++) {
                if (PIDARRAY[i] == pid) {
                    if (vmode) {
                        fprintf(stderr, "- %s", commandsArray[i][0]);
                        for (int j = 1; commandsArray[i][j] != NULL; j++) {
                            fprintf(stderr, " %s", commandsArray[i][j]);
                        }
                        fprintf(stderr, "\n");
                    }
                    PIDARRAY[i] = -1; // mark this process as handled
                    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
                        finalStatus = EXIT_FAILURE; // update final exit status
                    }
                    if (errorFound && (pid == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS)) {
                        killThemAll(SIGTERM);
                    }
                    break;
                }
            }
        }
    }

    while (runningJobs > 0) {
        int status;
        pid_t pid = wait(&status);
        runningJobs--;
        for (int i = 0; i < programCount; i++) {
            if (PIDARRAY[i] == pid) {
                if (vmode) {
                    fprintf(stderr, "- %s", commandsArray[i][0]);
                    for (int j = 1; commandsArray[i][j] != NULL; j++) {
                        fprintf(stderr, " %s", commandsArray[i][j]);
                    }
                    fprintf(stderr, "\n");
                }
                PIDARRAY[i] = -1; // mark this process as handled
                if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
                    finalStatus = EXIT_FAILURE; // update final exit status
                }
                if (errorFound && (pid == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS)) {
                    killThemAll(SIGTERM);
                }
                break;
            }
        }
    }

    // Free allocated memory
    for (int i = 0; i < programCount; i++) {
        free(commandsArray[i]);
    }
    free(commandsArray);
    free(PIDARRAY);

    return finalStatus;
}


