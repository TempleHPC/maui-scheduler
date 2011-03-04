# Makefile for Maui 3.3.1
# Copyright 1999 - 2010  Cluster Resources, Inc, All Rights Reserved

   TOP =../..
   BIN =$(TOP)/bin
   INC =$(TOP)/include
   SRC =$(TOP)/src/moab
   LIB =$(TOP)/lib

# uncomment for THREADs
#THREADDEF =-D__MTHREADS
#THREADLIB =-lpthread

SCHEDIP=-I$(INC) -I$(TOP)/include 
SHELL  =/bin/sh

  LP   =-L$(LIB) $(SCHEDLP) $(LLLP) $(MXLP) $(REGEXLP) $(PBSLDFLAGS)
  IP   =$(SCHEDIP) $(LLIP) $(SGEIP) $(REGEXIP) $(MXIP) $(PBSCFLAGS)
  INCS =$(INC)moab.h $(INC)msched-proto.h $(INC)msched-common.h
  DEFS =-D$(OPSYS) $(LLDEF) $(SDRDEF) $(PBSDEF) $(OSDEF) $(MXDEF) \
        $(REGEXDEF) $(RMSDEF) $(SECDEF) $(THREADDEF) $(DAEMONDEF)

SERVER   =maui
SCHEDLIB =moab
COMLIB   =mcom

MBINS    =$(SERVER) mclient mprof
LIBS     =-l$(SCHEDLIB) -l$(COMLIB) $(THREADLIB) $(MXLIB) $(LLLIB) $(SDRLIB) \
	  $(REGEXLIB) $(SECLIB) -lm

MCLIENTS =canceljob changeparam checkjob checknode diagnose releasehold \
          releaseres resetstats runjob schedctl sethold setqos setres setspri \
          showbf showconfig showgrid showhold showstart showq showres \
          showstate showstats mschedctl mjobctl mnodectl mdiag mstat mbal

SOBJECTS =Server.o UserI.o SDR.o LLI.o

POBJECTS =mprof.o

    LD =$(CC)
    CP =cp
    LN =ln

SOURCE = 
CFLAGS = $(OSCCFLAGS)

all:    $(MBINS)

install: all
	mkdir -p $(INST_DIR)/bin $(INST_DIR)/sbin $(INST_DIR)/include $(INST_DIR)/lib
	mkdir -p $(MSCHED_HOME)/log $(MSCHED_HOME)/traces $(MSCHED_HOME)/stats $(MSCHED_HOME)/spool $(MSCHED_HOME)/tools

	chmod 1777 $(MSCHED_HOME)/spool

	cp $(BIN)/mclient $(INST_DIR)/bin      
	cp $(BIN)/mprof $(INST_DIR)/bin   
	cp $(BIN)/maui $(INST_DIR)/sbin
	cp $(INC)/moab.h $(INST_DIR)/include
	cp $(LIB)/lib$(SCHEDLIB).a $(INST_DIR)/lib
	cp $(LIB)/lib$(COMLIB).a $(INST_DIR)/lib
     
	(cd $(INST_DIR)/bin; \
	for i in $(MCLIENTS); do \
          if [ ! -f $$i ]; \
            then ln -f mclient $$i || true; \
          fi; \
	done)

	if [ ! -f $(MSCHED_HOME)/maui.cfg ]; \
	  then cp $(TOP)/maui.cfg.dist $(MSCHED_HOME)/maui.cfg; \
	fi

	if [ ! -f $(MSCHED_HOME)/maui-private.cfg ]; \
          then cp $(TOP)/maui-private.cfg.dist $(MSCHED_HOME)/maui-private.cfg; \
        fi

#	(cd ../..;ln -sf etc/maui.cfg;ln -sf etc/maui-private.cfg)

uninstall:
	rm -f $(INST_DIR)/include/moab.h
	
setup:  
	mkdir -p ./bin $(TOP)/include     
	(cd $(TOP);mkdir -p spool stats log traces tools)
	chmod 1777 $(TOP)/spool

.c.o:   $(INCS)
	$(CC) $(IP) $(DEFS) $(CFLAGS) $(SOURCE) -c $<

UserI.o: OUserI.c

$(SERVER): $(SOBJECTS)
	$(LD) $(SOBJECTS) -o $(BIN)/$@ $(LIBS) $(OSLDFLAGS) $(LP)
	chmod 711 $(BIN)/$@ || true

mclient: mclient.o
	$(LD) mclient.o -o $(BIN)/$@ $(OSLDFLAGS) $(LIBS)    $(LP) 
	chmod 751 $(BIN)/$@
	for i in $(MCLIENTS); do \
	  ln -f $(BIN)/$@ $(BIN)/$$i; \
	  chmod 751 $(BIN)/$$i; \
	done

mprof:  $(POBJECTS)
	$(LD) $(POBJECTS) -o $(BIN)/$@ $(OSLDFLAGS) $(LIBS)     $(LP)
	chmod 751 $(BIN)/$@

clean:
	rm -f *.o $(BIN)/maui $(BIN)/mclient $(BIN)/mprof
	cd $(BIN) && rm -f $(MCLIENTS)

remove:
	rm $(INST_DIR)/bin/mclient
	rm $(INST_DIR)/bin/mprof
	rm $(INST_DIR)/sbin/maui
	rm $(INST_DIR)/include/moab.h
