### CSC 357 Assignment 6 - Processes and Signals

Purpose:
- This assignment explores process management and signal handling in C by creating a batch processing program. The program executes a sequence of commands as child processes, managing resources and handling signals appropriately.

Ground Rules:
- Code must compile without memory leaks using gcc and valgrind.
- Programs should be compatible with the Cal Poly UNIX servers and only use POSIX-compliant system calls and library functions.

Batch Processing:
- Implement a program named batch that executes multiple commands as child processes, with options to control execution order and handling of errors.
- The program supports the following command-line arguments:
- -n N: Set a maximum of N commands to execute simultaneously (default is to run all commands at once). If N=1, commands execute serially.
- -e: If any command fails (non-zero exit status), all other commands should terminate, including those already running, and remaining commands should not start.
- -v: Verbose mode, where each command is printed to stderr with a + before execution and a - after termination.

Command Execution:
- Commands are provided after a double dash -- and should include any additional arguments to pass directly to each command.
- Commands execute as child processes, and their output should only appear on stdout.
- If a command fails to execute, print an error message using perror and treat it as a failed command.

Signal Handling:
- If the program receives SIGINT, SIGQUIT, or SIGTERM, it must terminate all running commands immediately and exit with EXIT_FAILURE.
- Commands should not ignore these signals; if the user interrupts the program, all processes should halt.

Error Handling:
- If any command or argument is invalid, print a usage message to stderr and exit with EXIT_FAILURE.
- The program should print an error message for commands that fail to start and handle such cases as if they terminated with EXIT_FAILURE.

Key System Calls and Functions:
The assignment suggests using the following:
- fork, execvp: For creating and replacing child processes.
- wait, WIFEXITED, WEXITSTATUS: To check child processes’ exit statuses.
- Ensure processes do not create "fork bombs" by limiting child processes.

Testing:
- The program should be tested to match the expected output exactly. Output comparisons can be done using diff with test files provided.
- Example tests include various combinations of arguments, commands with different exit statuses, and signal handling scenarios.

