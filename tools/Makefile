# Makefile for maui tools
# Copyright 2016, Temple HPC Team

   TOP =..
   BIN =$(TOP)/bin/
   INC =$(TOP)/include/
   SRC =$(TOP)/src/moab
   LIB =$(TOP)/lib/

SHELL  =/bin/sh

    LD =$(CC)
    CP =cp
    RM =rm -f

SOURCE =
CFLAGS = $(OSCCFLAGS)

all:    fastdep.exe

fastdep.exe : fastdep.c
	$(CC) $(DEFS) $(CFLAGS) $(SOURCE) -o $@ $<

clean:
	$(RM) *.o *.exe

