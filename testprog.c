/* Exits with a given status after a given interval.
 * CSC 357, Assignment 6
 * Given code, Winter '24 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int status = argc < 2 ? EXIT_FAILURE : strtol(argv[1], NULL, 10);
    int delay = argc < 3 ? 1 : strtol(argv[2], NULL, 10);

    sleep(delay);
    printf("Returning %d after %d seconds.\n", status, delay);

    return status;
}
