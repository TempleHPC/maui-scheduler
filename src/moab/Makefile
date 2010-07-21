# Makefile for Moab 2.2.3
# Copyright 1999 - 2010  Cluster Resources Inc, All Rights Reserved

   TOP =../..
   BIN =$(TOP)/bin/
   INC =$(TOP)/include/
   SRC =$(TOP)/src/moab
   LIB =$(TOP)/lib/

# uncomment for THREADs
#THREADDEF =-D__MTHREADS
#THREADLIB =-lpthread

SCHEDIP=-I$(INC) -I$(MSCHED_HOME)/include 
SHELL  =/bin/sh

  LP   =-L$(LIB) $(SCHEDLP) $(LLLP) $(MXLP) $(REGEXLP) $(PBSLDFLAGS)
  IP   =$(SCHEDIP) $(LLIP) $(REGEXIP) $(MXIP) $(PBSCFLAGS)
  INCS =$(INC)moab.h $(INC)msched.h
  DEFS =-D$(OPSYS) $(LLDEF) $(SDRDEF) $(PBSDEF) $(OSDEF) $(MXDEF) $(REGEXDEF) \
        $(SECDEF) $(RMSDEF) $(THREADDEF) $(DAEMONDEF)

MOAB     =libmoab.a
LOBJECTS =MFile.o MConfig.o MQOS.o MLog.o MCred.o MSched.o MAM.o MSR.o MRes.o \
          MUser.o MGroup.o MAcct.o MClass.o MTime.o MNode.o MConst.o MACL.o MStats.o \
          MLocal.o MUtil.o MTrace.o MAppSim.o MQueue.o MLimit.o MXI.o MSim.o \
          MRMSI.o MBF.o MJob.o MPriority.o MPolicy.o MPreempt.o MFS.o MPar.o MRM.o \
          MPBSI.o MWikiI.o MSys.o MCP.o MUI.o $(SSSOBJ)

    LD =$(CC)
    CP =cp
    RM =rm -f

SOURCE =
CFLAGS = $(OSCCFLAGS)

all:    $(MOAB)

install: all
	mkdir -p $(INST_DIR)/bin $(INST_DIR)/sbin $(INST_DIR)/include $(INST_DIR)/lib

	$(CP) $(LIB)$(MOAB) $(INST_DIR)/lib
	$(CP) $(INC)moab.h $(INST_DIR)/include/moab.h 
     
uninstall:
	$(RM) $(INST_DIR)/include/moab.h
	$(RM) $(LIB)$(MOAB)  

.c.o:   $(INCS)
	$(CC) $(IP) $(DEFS) $(CFLAGS) $(SOURCE) -c $<

$(MOAB): $(LOBJECTS)
	ar rcs $(LIB)$(MOAB) $(LOBJECTS) 

clean:
	$(RM) *.o $(LIB)$(MOAB)

remove:
	$(RM) $(INST_DIR)/lib/$(MOAB)

