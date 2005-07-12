/* */
        
#include "moab.h"
#include "msched-proto.h"

extern char      *StrFrom();

extern mlog_t     mlog;

extern msys_t     MSys;
extern mframe_t   MFrame[];
extern msched_t   MSched;





int SDRGetSystemConfig()

  {
#ifdef __MSDR
  char  List[MAX_MBUFFER];

  char *ptr;

  int   rc;

  int   nindex;
  int   sindex;
  int   findex;

  int   AdapterType;

  char  HostAddress[MAX_MLINE];
  char  HostName[MAX_MLINE];

  char  tmp[MAX_MNAME];

  unsigned long Address;

  struct hostent *hoststruct;

  mhost_t *S;
#endif /* __MSDR */
 
  DBG(2,fSDR) DPrint("SDRGetSystemConfig()\n");

  if (MSched.Mode == msmSim)
    {
    return(SUCCESS);
    }

#ifdef __MSDR             
  if ((rc = SDROpenSession("","","","")) > 1)
    {
    DBG(0,fSDR) DPrint("ERROR:    cannot open SDR session.  rc: %d\n",
      rc);

    return(FAILURE);
    }

  DBG(3,fSDR) DPrint("INFO:     SDR session opened\n");

  memset(List,0,sizeof(List));

  SDRInitObjList(&List);

  SDREmptyObjList(List);

  if ((rc = SDRSetClass(List,"Node")) != 0)
    {
    DBG(1,fSDR) DPrint("ERROR:    SDRSetClass(Node) failed, rc: %d\n",
      rc);

    SDRCloseSession();

    return(FAILURE);
    }

  DBG(3,fSDR) DPrint("INFO:     SDR class set to 'Node'\n");

  if ((rc = SDRAddStringAttrValue(List,"slot_number","")) != 0)
    {
    DBG(1,fSDR) DPrint("ERROR:    SDRAddStringAttrValue(slot_number) failed, rc: %d\n",
      rc);

    SDRCloseSession();

    return(FAILURE);
    }

  if ((rc = SDRAddStringAttrValue(List,"frame_number","")) != 0)
    {
    DBG(1,fSDR) DPrint("ERROR:    SDRAddStringAttrValue(frame_number) failed, rc: %d\n",
      rc);

    SDRCloseSession();

    return(FAILURE);
    }

  if ((rc = SDRAddStringAttrValue(List,"slots_used","")) != 0)
    {
    DBG(1,fSDR) DPrint("ERROR:    SDRAddStringAttrValue(slots_used) failed, rc: %d\n",
      rc);

    SDRCloseSession();

    return(FAILURE);
    }

  DBG(3,fSDR) DPrint("INFO:     requesting SDR 'Node' class data\n");

  if ((rc = SDRGetObjects(List)) != 0)
    {
    DBG(0,fSDR) DPrint("ERROR:    SDRGetObjects() failed, rc: %d\n",
      rc);

    SDREmptyObjList(List);

    SDRCloseSession();

    return(FAILURE);
    }

  DBG(4,fSDR) DPrint("INFO:     SDR 'Node' class data received\n");

  SDRInitLoop(List);

  while(!SDREndOfObjects(List))
    {
    if ((rc = SDRFindAttrInObject(List,"slot_number")) != 0)
      {
      DBG(1,fSDR) DPrint("ERROR:    SDRFindAttrInObject(initial_hostname) failed, rc: %d\n",
        rc);

      SDRNextObject(List);

      continue;
      }

    sindex = IntFrom(List);

    if ((rc = SDRFindAttrInObject(List,"frame_number")) != 0)
      {
      DBG(1,fSDR) DPrint("ERROR:    SDRFindAttrInObject(frame_number) failed, rc: %d\n",
        rc);

      SDRNextObject(List);

      continue;
      }

    findex = IntFrom(List);

    if ((rc = SDRFindAttrInObject(List,"slots_used")) != 0)
      {
      DBG(1,fSDR) DPrint("ERROR:    SDRFindAttrInObject(slots_used) failed, rc: %d\n",
        rc);

      SDRNextObject(List);

      continue;
      }

    if ((findex < MAX_MFRAME) && (sindex <= MAX_MSPSLOT))
      {
      S = &MSys[findex][sindex];

      S->SlotsUsed  = IntFrom(List);
      S->MTime      = MSched.Time;

      MFrameAdd(NULL,&findex,NULL);

      DBG(6,fSDR) DPrint("INFO:     frame: %03d  slot: %03d  slots used: %d\n",
        findex,
        sindex,
        S->SlotsUsed);
      }
    else 
      {
      DBG(1,fSDR) DPrint("ERROR:    invalid frame/slot value returned by SDR (%d/%d)\n",
        findex,
        sindex);
      }

    SDRNextObject(List);
    }

  DBG(3,fSDR) DPrint("INFO:     Node objects read\n");

  SDREmptyObjList(List);

  SDRInitObjList(&List);

  if ((rc = SDRSetClass(List,"Adapter")) != 0)
    {
    DBG(1,fSDR) DPrint("ERROR:    SDRSetClass(Adapter) failed, rc: %d\n",
      rc);

    SDRCloseSession();

    return(FAILURE);
    }

  if ((rc = SDRAddStringAttrValue(List,"slot_number","")) != 0)
    {
    DBG(1,fSDR) DPrint("ERROR:    SDRAddStringAttrValue(slot_number) failed, rc: %d\n",
      rc);

    SDREmptyObjList(List);

    SDRCloseSession();

    return(FAILURE);
    }

  if ((rc = SDRAddStringAttrValue(List,"adapter_type","")) != 0)
    {
    DBG(1,fSDR) DPrint("ERROR:    SDRAddStringAttrValue(adapter_type) failed, rc: %d\n",
      rc);

    SDREmptyObjList(List);

    SDRCloseSession();

    return(FAILURE);
    }

  if ((rc = SDRAddStringAttrValue(List,"netaddr","")) != 0)
    {
    DBG(1,fSDR) DPrint("ERROR:    SDRAddStringAttrValue(netaddr) failed, rc: %d\n",
      rc);

    SDREmptyObjList(List);

    SDRCloseSession();

    return(FAILURE);
    }

  if ((rc = SDRGetObjects(List)) != 0)
    {
    DBG(0,fSDR) DPrint("ERROR:    SDRGetObjects() failed, rc: %d\n",
      rc);

    SDREmptyObjList(List);

    SDRCloseSession();

    return(FAILURE);
    }

  DBG(4,fSDR) DPrint("INFO:     SDR 'Adapter' class data received\n");

  SDRInitLoop(List);

  while(!SDREndOfObjects(List))
    {
    if ((rc = SDRFindAttrInObject(List,"node_number")) != 0)
      {
      DBG(1,fSDR) DPrint("ERROR:    SDRFindAttrInObject(node_number) failed, rc: %d\n",
        rc);

      SDRNextObject(List);

      continue;
      }

    nindex = IntFrom(List);

    sindex = nindex % MAX_MSPSLOT;
    findex = nindex / MAX_MSPSLOT + 1;

    if (sindex == 0)
      {
      sindex = 16;
      findex--;
      }

    if ((findex > MAX_MFRAME) || (sindex > MAX_MSPSLOT))
      {
      DBG(1,fSDR) DPrint("ERROR:    invalid frame/slot value returned by SDR (%d/%d)\n",
        findex,
        sindex);

      SDRNextObject(List);

      continue;
      }

    S = &MSys[findex][sindex];

    S->MTime = MSched.Time;

    DBG(6,fSDR) DPrint("INFO:     configuring adapters for node[%02d][%02d]\n",
      findex,
      sindex);

    if ((rc = SDRFindAttrInObject(List,"adapter_type")) != 0)
      {
      DBG(1,fSDR) DPrint("ERROR:    SDRFindAttrInObject(adapter_type) failed, rc: %d\n",
        rc);

      SDRNextObject(List);

      continue;
      }

    DBG(6,fSDR) DPrint("INFO:     adaptertype: %s\n",
      StrFrom(List));

    if (!strcmp(StrFrom(List),"en0"))
      {
      AdapterType = mnetEN0;
      }
    else if (!strcmp(StrFrom(List),"en1"))
      {
      AdapterType = mnetEN1;
      }
    else if (!strcmp(StrFrom(List),"css0"))
      {
      AdapterType = mnetCSS0;
      }
    else
      {
      SDRNextObject(List);

      continue;
      }

    if ((rc = SDRFindAttrInObject(List,"netaddr")) != 0)
      {
      DBG(1,fSDR) DPrint("ERROR:    SDRFindAttrInObject(netaddr) failed, rc: %d\n",
        rc);

      SDRNextObject(List);

      continue;
      }
    
    strcpy(HostAddress,StrFrom(List));

    if ((Address = inet_addr(HostAddress)) == (unsigned long)-1)
      {
      DBG(1,fSDR) DPrint("ERROR:    cannot determine host address for '%s'\n",
        HostAddress);

      continue;
      }

    DBG(6,fSDR) DPrint("INFO:     address: %s\n",
      HostAddress);

    if ((hoststruct = gethostbyaddr(
          (char *)&Address,
          sizeof(Address),
          AF_INET)) == NULL)
      {
      DBG(2,fSDR) DPrint("ALERT:    cannot determine name for address '%s' (frame: %d slot: %d)\n",
        HostAddress,
        findex,
        sindex);

      SDRNextObject(List);

      continue;
      }

    strcpy(S->NetName[AdapterType],hoststruct->h_name);
    S->NetAddr[AdapterType];

    SDRNextObject(List);

    DBG(6,fSDR) DPrint("INFO:     MSys[%02d][%02d]  css0: %20s  en0: %20s  en1: %s\n",
      findex,
      sindex,
      S->NetName[mnetCSS0],
      S->NetName[mnetEN0],
      S->NetName[mnetEN1]);
    }  /* END while () */

  SDREmptyObjList(List);

  SDRCloseSession();
#endif /* __MSDR */

  return(SUCCESS);
  }  /* END SDRGetSystemConfig() */

/* END SDR.c */

