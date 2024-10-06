# Makefile for Assignment 6 - Processes and Signals

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -pedantic -g

# Targets
TARGET = batch
TESTPROG = testprog

# Default target
all: batch testprog

# Build targets
batch: batch.o
	$(CC) $(CFLAGS) -o batch batch.o

testprog: testprog.o
	$(CC) $(CFLAGS) -o testprog testprog.o

# Compile source files to object files
batch.o: batch.c
	$(CC) $(CFLAGS) -c batch.c -o batch.o

testprog.o: testprog.c
	$(CC) $(CFLAGS) -c testprog.c -o testprog.o

# Debug target
debug: clean batch testprog

# Valgrind target
valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes ./batch $(ARGS)

# GDB target
gdb: batch
	gdb ./batch

# Clean target
clean:
	rm -f batch batch.o testprog testprog.o

# Test target
test: batch testprog
	./batch $(ARGS)

