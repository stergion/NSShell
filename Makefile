CC=gcc
CFLAGS=-Wall -Wextra -O3

all: 
	$(CC) NSShell.c -o NSShell $(CFLAGS)
	
run_interactive:
	./NSShell

run_batch:
	./NSShell batchFile.bat
	
purge:
	rm NSShell
	rm batchFile.bat