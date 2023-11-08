CC=mpicc
CFLAGS= -Wall -Wextra
SFILES= index.c set.c get.c
HFILES= definitions.h index.h set.h get.h
all: kvdb

kvdb: kvdb.c $(SFILES) $(HFILES)
	$(CC) -g $(CFLAGS) $(SFILES) -o $@ $@.c

clean: 
	rm data.bin index.bin kvdb
