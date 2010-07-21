# Makefile for libmcom 
# Copyright 1999 - 2010  Cluster Resources, Inc, All Rights Reserved

   TOP =../..
   BIN =$(TOP)/bin/
   INC =$(TOP)/include/
   SRC =$(TOP)/src/moab
   LIB =$(TOP)/lib/

SCHEDIP=-I$(INC) -I$(MSCHED_HOME)/include 
SHELL  =/bin/sh

  LP   =-L$(LIB) $(SCHEDLP) $(MXLP) $(REGEXLP)
  IP   =$(SCHEDIP) $(REGEXIP) $(MXIP)
  INCS =$(INC)moab.h $(INC)moab-proto.h 
  DEFS =-D$(OPSYS) $(OSDEF) $(MXDEF) $(REGEXDEF) $(SECDEF) $(DAEMONDEF)

LIBMCOM  =libmcom.a
LOBJECTS =MSU.o MSec.o MS3I.o MXML.o MConst.o MTComm.o MComm.o

    LD =$(CC)
    CP =cp
    RM =rm -f

SOURCE = 
CFLAGS = $(OSCCFLAGS)

all:    $(LIBMCOM)

install: all
	mkdir -p $(INST_DIR)/bin $(INST_DIR)/sbin $(INST_DIR)/include $(INST_DIR)/lib

	$(CP) $(LIB)$(LIBMCOM) $(INST_DIR)/lib

uninstall:
	$(RM) $(LIB)$(LIBMCOM)  

.c.o:   $(INCS)
	$(CC) $(IP) $(DEFS) $(CFLAGS) $(SOURCE) -c $<

$(LIBMCOM): $(LOBJECTS)
	ar rcs $(LIB)$(LIBMCOM) $(LOBJECTS) 

clean:
	$(RM) *.o $(LIB)$(LIBMCOM) 

remove:
	$(RM) $(INST_DIR)/lib/$(LIBMCOM)

