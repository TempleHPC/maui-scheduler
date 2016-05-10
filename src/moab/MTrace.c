/* HEADER */
        
/*                                               *
 * Contains:                                     *
 *                                               *
 * int MTraceLoadWorkload(Buffer,TraceLen,J,Mode,Version) *
 * int MTraceLoadResource(Buffer,TraceLen,N)     *
 * int MTraceGetWorkloadVersion(Buffer,Version)  *
 * int MTraceGetResourceVersion(Buffer,Version)  *
 * int MTraceBuildResource(N,Version,Buf,BufSize) *
 *                                               */





#include "moab.h"
#include "msched-proto.h"

extern mlog_t mlog;

extern msched_t    MSched;
extern msim_t      MSim;
extern mnode_t    *MNode[];
extern mqos_t      MQOS[];
extern mpar_t      MPar[];
extern mrm_t       MRM[];
extern m64_t       M64;

extern mframe_t    MFrame[];

extern mattrlist_t MAList;
extern msys_t      MSys;

extern const char *MComp[];
extern const char *MJobFlags[];
extern const char *MNAccessPolicy[];
extern const char *MResSetSelectionType[];
extern const char *MResSetAttrType[];
extern const char *MJobState[];
   
char NullBuf[MAX_MBUFFER];

/* private prototypes */

extern int MPBSJobSetAttr(mjob_t *,void *,char *,tpbsa_t *,short *,int);
extern int MPBSJobAdjustResources(mjob_t *,tpbsa_t *,mrm_t *);





int MTraceLoadResource( 

  char    *Buffer,   /* I */
  int     *TraceLen, /* I */
  mnode_t *N,        /* O (modified) */
  int     *Version)  /* O (optional) */

  {
  char Line[MAX_MLINE];

  char *ptr;

  char *tail;
  char *head;

  int    rc;

  int    RType;

  enum   { mrtNONE = 0, mrtComputeNode, mrtNetwork, mrtHSM };

  const char *FName = "MTraceLoadResource";

  DBG(5,fSIM) DPrint("%s(Buffer,TraceLen,N,%d)\n",
    FName,
    (Version != NULL) ? *Version : -1);

  if ((Buffer == NULL) || (N == NULL))
    {
    return(FAILURE);
    }

  head = Buffer;

  if ((tail = strchr(head,'\n')) == NULL)
    {
    tail = head + strlen(head);

    if (tail - head < 20)
      {
      if (TraceLen != NULL)
        *TraceLen = 0;

      return(FAILURE);
      }
    }

  if (TraceLen != NULL)
    *TraceLen = (tail - head) + 1;

  for (ptr = head;*ptr != '\0';ptr++)
    {
    /* remove leading whitespace */

    if (!isspace(*ptr))
      break;
    }  /* END for (ptr) */

  if (ptr >= tail)
    {
    return(FAILURE);
    }

  MUStrCpy(Line,ptr,MIN(sizeof(Line),tail - ptr + 1));

  /* eliminate comments */

  if ((ptr = strchr(Line,'#')) != NULL)
    {
    DBG(5,fSIM) DPrint("INFO:     detected trace comment line, '%s'\n",
      Line);

    *ptr = '\0';
    }

  if (strlen(Line) == 0)
    {
    return(FAILURE);
    }

  /* look for version settings */

  if (!strncmp(Line,TRACE_RESOURCE_VERSION_MARKER,strlen(TRACE_RESOURCE_VERSION_MARKER)))
    {
    if (MTraceGetResourceVersion(Line,Version) == FAILURE)
      {
      DBG(0,fSIM) DPrint("ALERT:    cannot determine resource trace version\n");

      /* assume default version */

      if (Version != NULL)
        *Version = DEFAULT_RESOURCE_TRACE_VERSION;
      }
    else
      {
      DBG(3,fSIM) DPrint("INFO:     resource trace version %d detected\n",
        (Version != NULL) ? *Version : -1);
      }

    return(FAILURE);
    }

  DBG(5,fSIM) DPrint("INFO:     parsing line '%s' (len: %d)\n",
    Line,
    *TraceLen);

  {
  int VIndex;

  if (Version != NULL)
    VIndex = *Version;
  else
    VIndex = -1;

  switch (VIndex)
    {
    case 230:
    default:

      if (!strncmp(Line,"COMPUTENODE",strlen("COMPUTENODE")))
        RType = mrtComputeNode;
      else if (!strncmp(Line,"NETWORK",strlen("NETWORK")))
        RType = mrtNetwork;
      else if (!strncmp(Line,"HSM",strlen("HSM")))
        RType = mrtHSM;
      else
        RType = mrtComputeNode;

      break;
    }  /* END switch (VIndex) */

  switch(RType)
    {
    case mrtComputeNode:

      rc = MTraceLoadComputeNode(Line,N,VIndex);

      break;

    case mrtNetwork:

      rc = MTraceLoadNetwork(Line,N,VIndex);

      break;

    case mrtHSM:

      rc = MTraceLoadHSM(Line,N,VIndex);

      break;

    default:

      rc = FAILURE;

      break;
    }  /* END switch (RType) */
  }    /* END BLOCK */

  return(rc);
  }  /* END MTraceLoadResource() */





int MTraceLoadNetwork(
 
  char    *NodeLine,  /* I */
  mnode_t *N,         /* I/O */
  int      Version)   /* I */
 
  {
  char   ResourceType[MAX_MNAME];
  char   EventType[MAX_MNAME];
  char   Name[MAX_MNAME];

  long   ETime;
  char   ResType[MAX_MNAME];
  char   ResData[MAX_MNAME];

  char   tmpLine[MAX_MLINE];

  int    rindex;

  int    rc;

  xres_t *R;

  if ((NodeLine == NULL) || (N == NULL))
    {
    return(FAILURE);
    }

  switch (Version)
    {
    default:

      /* FORMAT:  <RESOURCETYPE> <EVENTTYPE> <EVENTTIME> <NAME> <RESTYPE> <RESDATA> */
 
      rc = MUSScanF(NodeLine,"%x%s %x%s %ld %x%s %x%s %x%s",
         MAX_MNAME,
         ResourceType,
         MAX_MNAME,
         EventType,
         &ETime,
         MAX_MNAME,
         Name,
         MAX_MNAME,
         ResType,
         MAX_MNAME,
         ResData);

      /* locate available X res slot */

      for (rindex = 0;rindex < MAX_MNODE;rindex++)
        {
        R = &MPar[0].XRes[rindex];

        if ((R->Name[0] != '\0') && (R->Name[0] != '\1'))
          {
          continue;
          }

        /* load xres driver */

        MASGetDriver((void **)&R->Func,ResType,msdResource);

        /* create and initialize resource */

        sprintf(tmpLine,"NAME=%s;STATE=ACTIVE;%s",
          Name,
          ResData);

        rc = (*R->Func)(&R->Data,mascCreate,tmpLine,NULL);
    
        MUStrCpy(R->Name,Name,sizeof(R->Name));

        /* FIXME */

        R->Type = msdResource;
      
        break;
        }
  
      break;
    }  /* END switch(Version) */

  return(rc);
  }  /* END MTraceLoadNetwork() */




int MTraceLoadHSM(
 
  char    *NodeLine,  /* I */
  mnode_t *N,         /* I (modified) */
  int      Version)   /* I */
 
  {
  if ((NodeLine == NULL) || (N == NULL))
    {
    return(FAILURE);
    }

  /* NYI */
 
  return(FAILURE);
  }  /* END MTraceLoadHSM() */





int MTraceGetWorkloadVersion(

  char *Buffer,  /* I */
  int  *Version) /* O */

  {
  char  Line[MAX_MLINE];
  char *ptr;

  int   tmpI;

  const char *FName = "MTraceGetWorkloadVersion";

  DBG(3,fSIM) DPrint("%s(Buffer,Version)\n",
    FName);

  if ((Buffer == NULL) || (Version == NULL))
    {
    return(FAILURE);
    }

  if ((ptr = strstr(Buffer,TRACE_WORKLOAD_VERSION_MARKER)) == NULL)
    {
    DBG(1,fSIM) DPrint("ALERT:    cannot locate trace version marker\n");

    *Version = -1;

    return(FAILURE);
    }

  ptr += (strlen(TRACE_WORKLOAD_VERSION_MARKER) + 1);

  sscanf(ptr,"%64s",
    Line);

  tmpI = (int)strtol(Line,NULL,0);

  if (tmpI < 100)
    {
    *Version = -1;

    return(FAILURE);
    }

  *Version = tmpI;
 
  return(SUCCESS);
  }  /* END MTraceGetWorkloadVersion() */





int MTraceGetResourceVersion(

  char *Buffer,  /* I */
  int  *Version) /* O */

  {
  char *ptr;
  char  Line[MAX_MLINE];

  int   tmpI;

  const char *FName = "MTraceGetResourceVersion";

  DBG(3,fSIM) DPrint("%s(Buffer,Version)\n",
    FName);

  if ((Buffer == NULL) || (Version == NULL))
    {
    return(FAILURE);
    }

  if ((ptr = strstr(Buffer,TRACE_RESOURCE_VERSION_MARKER)) == NULL)
    {
    DBG(1,fSIM) DPrint("ALERT:    cannot locate trace version marker\n");

    *Version = -1;

    return(FAILURE);
    }

  ptr += (strlen(TRACE_RESOURCE_VERSION_MARKER) + 1);

  sscanf(ptr,"%64s",
    Line);

  tmpI = (int)strtol(Line,NULL,0);

  if (tmpI < 100)
    {
    *Version = -1;

    return(FAILURE);
    }

  *Version = tmpI;

  return(SUCCESS);
  }  /* END MTraceGetResourceVersion() */





int MTraceBuildResource(

  mnode_t *N,       /* I */
  int      Version, /* I */
  char    *Buf,     /* O */
  int      BufSize) /* I */

  {
  int  cindex;

  char Features[MAX_MLINE];
  char Classes[MAX_MLINE];
  char Networks[MAX_MLINE];

  char *BPtr;
  int   BSpace;

  const char *FName = "MTraceBuildResource";

  DBG(3,fSIM) DPrint("%s(%s,%d,Buf,%d)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    Version,
    BufSize);

  if ((N == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  BPtr   = Buf;
  BSpace = BufSize;

  BPtr[0] = '\0';

  strcpy(Features,MUMAList(eFeature,N->FBM,sizeof(N->FBM)));
  strcpy(Networks,MUListAttrs(eNetwork,N->Network));

  switch(Version)
    {
    case 230:

      Classes[0] = '\0';

      for (cindex = 1;cindex < MAX_MCLASS;cindex++)
        {
        if (N->CRes.PSlot[cindex].count > 0) 
          sprintf(temp_str,"[%s:%d]",
            MAList[eClass][cindex],
            N->CRes.PSlot[cindex].count);
        strcat(Classes,temp_str);
        }  /* END for (cindex) */

      if (Classes[0] == '\0')
        strcpy(Classes,NONE);

      /* FORMAT:  <RESOURCETYPE> <EVENTTYPE> <EVENTTIME> <NAME> <RMINDEX> <SWAP> <MEMORY> <DISK> <MAXPROCS> <SLOTSUSED> <FRAMEINDEX> <SLOTINDEX> <OPSYS> <ARCH> [<FEATURE>]* [<CLASS>]* [<NETWORK>]* <RES1> <RES2> <RES3> <RES4> */

      MUSNPrintF(&BPtr,&BSpace,"%s %s %d %s %s %dM %dM %dM %d %d %d %d %s %s %s %s %s %s %s %s %s",
        "COMPUTENODE",
        "AVAILABLE",
        0,
        N->Name,
        (N->RM != NULL) ? N->RM->Name : NONE,
        N->CRes.Swap,
        N->CRes.Mem,
        N->CRes.Disk,
        N->CRes.Procs,
        N->FrameIndex,
        N->SlotIndex,
        (N->FrameIndex > 0) ? MSys[N->FrameIndex][N->SlotIndex].SlotsUsed : 1,
        MAList[eOpsys][N->ActiveOS],
        MAList[eArch][N->Arch],
        Features,
        Classes,
        Networks,
        NONE,
        NONE,
        NONE,
        NONE
        );

      break;

    default:

      DBG(4,fSIM) DPrint("ALERT:    cannot create version %d resource trace record\n",
        Version);

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }    /* END switch(Version) */

  return(SUCCESS);
  }  /* END MTraceBuildResource() */




int MTraceLoadComputeNode(

  char    *NodeLine,  /* I */
  mnode_t *N,         /* O */
  int      Version)   /* I */

  {
  char *tok;

  char *TokPtr;

  char   tmpLine[MAX_MLINE];
  char   tmpNLine[MAX_MLINE];

  char   ResourceType[MAX_MNAME];
  char   EventType[MAX_MNAME];
  long   ETime;

  char   Name[MAX_MNAME];
  char   OpsysString[MAX_MNAME];
  char   ArchString[MAX_MNAME];
  char   FeatureString[MAX_MLINE];
  char   ClassString[MAX_MLINE];
  char   NetworkString[MAX_MLINE];
  char   GResString[MAX_MLINE];

  char   tmpSwapBuf[MAX_MNAME];
  char   tmpMemBuf[MAX_MNAME];
  char   tmpDiskBuf[MAX_MNAME];

  int    SlotsUsed;

  char   RMName[MAX_MNAME];

  int    FIndex;
  int    SIndex;
  int    cindex;

  int    rc;

  /* set default resource attributes */

  memset(N,0,sizeof(mnode_t));

  N->Load    = 0.0;
  N->Speed   = 1.0;

  N->PtIndex = 0;

  switch(Version)
    {
    case 230:

      if (strchr(NodeLine,';') != NULL)
        {
        sprintf(tmpNLine,"IFS-;%.*s",
          MAX_MLINE,
          NodeLine);
        }
      else
        {
        MUStrCpy(tmpNLine,NodeLine,sizeof(tmpNLine));
        }

      /* FORMAT:  <RESOURCETYPE> <EVENTTYPE> <EVENTTIME> <NAME> <RMINDEX> <SWAP> <MEMORY> <DISK> <MAXPROCS> <SLOTSUSED> <FRAMEINDEX> <SLOTINDEX> <OPSYS> <ARCH> [<FEATURE>]* [<CLASS>]* [<NETWORK>]* [<GRES>]* <RES1> <RES2> <RES3> */

      rc = MUSScanF(tmpNLine,"%x%s %x%s %ld %x%s %x%s %x%s %x%s %x%s %d %d %d %d %x%s %x%s %x%s %x%s %x%s %x%s %x%s %x%s %x%s %x%s",
         MAX_MNAME,
         ResourceType,
         MAX_MNAME,
         EventType,
         &ETime,
         MAX_MNAME,
         Name,
         MAX_MNAME,
         RMName,
         MAX_MNAME,
         tmpSwapBuf,
         MAX_MNAME,
         tmpMemBuf,
         MAX_MNAME,
         tmpDiskBuf,
         &N->CRes.Procs, 
         &FIndex,
         &SIndex,
         &SlotsUsed,
         MAX_MNAME,
         OpsysString,
         MAX_MNAME,
         ArchString,
         MAX_MLINE,
         FeatureString,
         MAX_MLINE,
         ClassString,
         MAX_MLINE,
         NetworkString,
         MAX_MLINE,
         GResString,
         MAX_MLINE,
         tmpLine,            /* RES1 */
         MAX_MLINE,
         tmpLine,            /* RES2 */
         MAX_MLINE,
         tmpLine,            /* RES3 */
         MAX_MLINE,
         tmpLine             /* check for too many lines */
         );

      if ((rc == EOF) || (rc != 21))
        {
        DBG(3,fSIM) DPrint("ALERT:    resource tracefile corruption (%d of %d fields detected) on line '%s'\n",
          rc,
          21,
          NodeLine);

        return(FAILURE);
        }

      /* process node attributes */

      N->CRes.Swap = MURSpecToL(tmpSwapBuf,mvmMega,mvmMega);
      N->CRes.Mem  = MURSpecToL(tmpMemBuf,mvmMega,mvmMega);
      N->CRes.Disk = MURSpecToL(tmpDiskBuf,mvmMega,mvmMega);

      if (!strcmp(RMName,NONE))
        MRMAdd(RMName,&N->RM);
      else
        MRMAdd(DEFAULT,&N->RM);

      /* load Opsys */

      if (strstr(OpsysString,MAList[eOpsys][0]) == NULL)
        N->ActiveOS = MUMAGetIndex(eOpsys,OpsysString,mAdd);

      /* load Arch */

      if (strstr(ArchString,MAList[eArch][0]) == NULL)
        N->Arch = MUMAGetIndex(eArch,ArchString,mAdd);

      memcpy(&N->ARes,&N->CRes,sizeof(N->ARes));

      strcpy(N->Name,MNodeAdjustName(Name,0));

      /* load classes */

      if (MSim.Flags & ((1 << msimfIgnClass) | (1 << msimfIgnAll)))
        {
        mclass_t *C;

        /* ignore specified classes */

        MClassAdd("DEFAULT",&C);

        N->CRes.PSlot[0].count = N->CRes.Procs;
        N->CRes.PSlot[C->Index].count = N->CRes.Procs;
        }
      else
        {
        MUNumListFromString(N->CRes.PSlot,ClassString,eClass);

        for (cindex = 1;cindex < MAX_MCLASS;cindex++)
          {
          if (N->CRes.PSlot[cindex].count > 0)
            MClassAdd(MAList[eClass][cindex],NULL);
          }
        }

      memcpy(N->ARes.PSlot,N->CRes.PSlot,sizeof(N->ARes.PSlot));

      break;

    default:

      DBG(4,fSIM) DPrint("ALERT:    cannot load version %d resource trace record\n",
        Version);

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(Version) */

  if ((MSim.Flags & ((1 << msimfIgnFrame) | (1 << msimfIgnAll))) || 
      (FIndex < 0))
    {
    N->FrameIndex = -1;
    N->SlotIndex  = -1;
    }
  else
    {
    MFrameAddNode(&MFrame[FIndex],N,SIndex);
      
    if ((N->FrameIndex > 0) && (N->FrameIndex < MAX_MFRAME) &&
        (N->SlotIndex > 0)  && (N->SlotIndex <= MAX_MSLOTPERFRAME))
      {
      mhost_t *S;
      int      nindex;
  
      S = &MSys[N->FrameIndex][N->SlotIndex];

      S->SlotsUsed = SlotsUsed;

      for (nindex = 0;nindex < MAX_MNETTYPE;nindex++)
        strcpy(S->NetName[nindex],N->Name);

      if (!strstr(ClassString,"NONE"))
        S->Attributes |= (1 << attrBatch);

      S->MTime = MSched.Time;
      }  /* END if ((N->FrameIndex > 0) ... ) */
    }    /* END else if (MSim.Flags & ((1 << msimfIgnFrame) | ...)) */

  /* initialize statistics */

  N->STTime = 0;
  N->SUTime = 0;
  N->SATime = 0;

  /* load state */

  if (N->CRes.Procs <= 0)
    {
    N->State = mnsNone;
    }
  else
    {
    if (!strcmp(EventType,"AVAILABLE"))
      N->State = mnsIdle;
    else if (!strcmp(EventType,"DEFINED"))
      N->State = mnsDown;
    else if (!strcmp(EventType,"DRAINED"))
      N->State = mnsDrained;
    else
      N->State = mnsNONE;
    }  /* END else (N->CRes.Procs <= 0) */

  N->EState = N->State;

  memcpy(&N->ARes,&N->CRes,sizeof(N->ARes));

  /* load features */

  if (strstr(FeatureString,MAList[eFeature][0]) == NULL)
    {
    tok = MUStrTok(FeatureString,"[]",&TokPtr);

    do
      {
      MNodeProcessFeature(N,tok);

      DBG(6,fSIM) DPrint("INFO:     feature '%s' added (%s)\n",
        tok,
        MUMAList(eFeature,N->FBM,sizeof(N->FBM)));
      }
    while ((tok = MUStrTok(NULL,"[]",&TokPtr)) != NULL);
    }

  /* load network adapters */

  if (strstr(NetworkString,MAList[eNetwork][0]) == NULL)
    {
    tok = MUStrTok(NetworkString,"[]",&TokPtr);

    do
      {
      N->Network |= MUMAGetBM(eNetwork,tok,mAdd);

      DBG(6,fSIM) DPrint("INFO:     network '%s' added (%s)\n",
        tok,
        MUListAttrs(eNetwork,N->Network));
      }
    while ((tok = MUStrTok(NULL,"[]",&TokPtr)) != NULL);
    }

  /* load generic resources */

  if (!strcmp(GResString,NONE) != 0)
    {
    MUNumListFromString(N->CRes.GRes,GResString,eGRes);
    }

  DBG(4,fSIM) DPrint("INFO:     node '%s' F:%d/S:%d  %06d %04d %06d %02d %02d %10s %10s %20s %20s\n",
    N->Name,
    N->FrameIndex,
    N->SlotIndex,
    N->CRes.Swap,
    N->CRes.Mem,
    N->CRes.Disk,
    N->CRes.Procs,
    SlotsUsed,
    OpsysString,
    ArchString,
    FeatureString,
    MUCAListToString(N->CRes.PSlot,NULL,NULL));

  return(SUCCESS);
  }  /* END MTraceLoadComputeNode() */




int MTraceLoadWorkload(

  char     *Buffer,   /* I */
  int      *TraceLen, /* I (optional,modified) */
  mjob_t   *J,        /* O */
  int       Mode,     /* I */
  int      *Version)  /* O */

  {
  char *tail;
  char *head;

  char *ptr;
  char *ptr2;

  char *tok;
  char *TokPtr;

  char Line[MAX_MLINE << 4];
  char tmpLine[MAX_MLINE];

  char MHostName[MAX_MNAME];
  char MReqHList[MAX_MLINE << 2];
  char Reservation[MAX_MNAME];

  char tmpTaskRequest[MAX_MLINE];
  char tmpSpecWCLimit[MAX_MLINE];
  char tmpState[MAX_WORD];
  char tmpOpsys[MAX_WORD];
  char tmpArch[MAX_WORD];
  char tmpNetwork[MAX_WORD];
  char tmpMemCmp[MAX_WORD];
  char tmpDiskCmp[MAX_WORD];
  char tmpReqNFList[MAX_MLINE];
  char tmpClass[MAX_MLINE];
  char tmpRMName[MAX_MLINE];
  char tmpCmd[MAX_MLINE];
  char tmpRMXString[MAX_MLINE >> 2];
  char tmpParName[MAX_MNAME];
  char tmpJFlags[MAX_MLINE];
  char tmpQReq[MAX_WORD];
  char tmpASString[MAX_MLINE];
  char tmpRMemBuf[MAX_MNAME];
  char tmpRDiskBuf[MAX_MNAME];
  char tmpDSwapBuf[MAX_MNAME];
  char tmpDMemBuf[MAX_MNAME];
  char tmpDDiskBuf[MAX_MNAME];
  char UName[MAX_MNAME];
  char GName[MAX_MNAME];
  char AName[MAX_MNAME];

  char SetString[MAX_MLINE];

  unsigned long tmpQTime;
  unsigned long tmpDTime;
  unsigned long tmpSTime;
  unsigned long tmpCTime;

  unsigned long tmpSQTime;


  unsigned long tmpSDate;
  unsigned long tmpEDate;

  int      TPN;
  int      TasksAllocated;
  int      index;
  int      rc;

  int      tindex;
  int      nindex;

  mqos_t  *QDef;

  mreq_t  *RQ[MAX_MREQ_PER_JOB];

  mnode_t *N;

  long     ReqProcSpeed;

  const char *FName = "MTraceLoadWorkload";

  DBG(5,fSIM) DPrint("%s(Buffer,TraceLen,J,%d,%d)\n",
    FName,
    Mode,
    (Version != NULL) ? *Version : -1);

  if (J == NULL)
    {
    DBG(5,fSIM) DPrint("ALERT:    NULL job pointer passed in %s()\n",
      FName);

    return(FAILURE);
    }

  if (Buffer == NULL)
    {
    return(FAILURE);
    }

  if ((tail = strchr(Buffer,'\n')) == NULL)
    {
    /* assume line is '\0' terminated */

    if (TraceLen != NULL)
      tail = Buffer + *TraceLen;
    else
      tail = Buffer + strlen(Buffer);
    }

  head = Buffer;

  MUStrCpy(Line,head,MIN(tail - head + 1,sizeof(Line)));

  DBG(6,fSIM) DPrint("INFO:     parsing trace line '%s'\n",
    Line);

  if (TraceLen != NULL)
    *TraceLen = (tail - head) + 1;

  /* eliminate comments */

  if ((ptr = strchr(Line,'#')) != NULL)
    {
    DBG(5,fSIM) DPrint("INFO:     detected trace comment line, '%s'\n",
      Line);

    *ptr = '\0';
    }

  if (Line[0] == '\0')
    {
    return(FAILURE);
    }

  /* look for VERSION settings */

  if (!strncmp(Line,TRACE_WORKLOAD_VERSION_MARKER,strlen(TRACE_WORKLOAD_VERSION_MARKER)))
    {
    if (MTraceGetWorkloadVersion(Line,Version) == FAILURE)
      {
      DBG(0,fSIM) DPrint("ALERT:    cannot determine workload trace version\n");

      /* assume default version */

      if (Version != NULL)
        *Version = DEFAULT_WORKLOAD_TRACE_VERSION;
      }
    else
      {
      DBG(3,fSIM) DPrint("INFO:     workload trace version %d detected\n",
        (Version != NULL) ? *Version : -1);
      }

    return(FAILURE);
    }  /* END if (!strncmp(Line,TRACE_WORKLOAD_VERSION_MARKER)) */

  /* set default workload attributes */

  memset(J,0,sizeof(mjob_t));

  if (MReqCreate(J,NULL,&J->Req[0],FALSE) == FAILURE)
    {
    DBG(0,fSIM) DPrint("ALERT:    cannot create job req\n");

    return(FAILURE);
    }

  MRMJobPreLoad(J,NULL,0);

  RQ[0]         = J->Req[0];

  RQ[0]->Index  = 0;

  J->StartCount = 0;
  J->R          = NULL;

  switch(*Version)
    {
    case 230:

      /*                Name NR TReq User Grop WClk Stat Clss QUT DST STT CMT netw Arch Opsy MemC RMem DskC RDsk Feat SQT TA TPN QO MO AC CM CC BP PU Part DPrc DMem DDisk DSwap STARTDATE ENDDATE MNODE RM HL RES RES1 RES2 RES3 OVERFLOW */

      rc = sscanf(Line,"%64s %d %64s %64s %64s %64s %64s %64s %lu %lu %lu %lu %64s %64s %64s %64s %64s %64s %64s %64s %lu %d %d %64s %1024s %64s %1024s %1024s %d %lf %64s %d %64s %64s %64s %lu %lu %1024s %64s %1024s %64s %64s %1024s %s %s",
           J->Name,
           &J->Request.NC,
           tmpTaskRequest,
           UName,
           GName,
           tmpSpecWCLimit,
           tmpState,
           tmpClass,
           &tmpQTime,
           &tmpDTime,
           &tmpSTime,
           &tmpCTime,
           tmpNetwork,
           tmpArch,
           tmpOpsys,
           tmpMemCmp,
           tmpRMemBuf,
           tmpDiskCmp,
           tmpRDiskBuf,
           tmpReqNFList,
           &tmpSQTime,             /* SystemQueueTime */
           &TasksAllocated,        /* allocated task count */
           &TPN,
           tmpQReq,
           tmpJFlags,
           AName,
           tmpCmd,
           tmpRMXString,           /* RM extension string */
           &J->Bypass,             /* Bypass          */
           &J->PSUtilized,         /* PSUtilized      */
           tmpParName,             /* partition used  */
           &RQ[0]->DRes.Procs,
           tmpDMemBuf,
           tmpDDiskBuf,
           tmpDSwapBuf,
           &tmpSDate,
           &tmpEDate,
           MHostName,
           tmpRMName,
           MReqHList,
           Reservation,
           SetString,              /* resource sets required by job */
           tmpASString,            /* application simulator name */
           tmpLine,                /* RES1 */
           tmpLine                 /* check for too many fields */
           );

      /* fail if all fields not read in */

      switch (rc)
        {
        case 43:

          strcpy(MHostName,NONE);

          J->RM = &MRM[0];

          break;

        case 44:

          /* canonical 230 trace */

          if (isdigit(tmpRMName[0]))
            J->RM = &MRM[(int)strtol(tmpRMName,NULL,0)];
          else
            J->RM = &MRM[0];

          break;

        default:

          DBG(1,fSIM) DPrint("ALERT:    version %d workload trace is corrupt '%s' (%d of %d fields read) ignoring line\n",
            *Version,
            Line,
            rc,
            44);

          MJobDestroy(&J);

          return(FAILURE);

          /*NOTREACHED*/

          break;
        }  /* END switch(rc) */

      RQ[0]->RequiredMemory = MURSpecToL(tmpRMemBuf,mvmMega,mvmMega);
      RQ[0]->RequiredDisk   = MURSpecToL(tmpRDiskBuf,mvmMega,mvmMega);

      RQ[0]->DRes.Mem       = MURSpecToL(tmpDMemBuf,mvmMega,mvmMega);
      RQ[0]->DRes.Swap      = MURSpecToL(tmpDSwapBuf,mvmMega,mvmMega);
      RQ[0]->DRes.Disk      = MURSpecToL(tmpDDiskBuf,mvmMega,mvmMega);

      if (Mode != msmProfile)
        {
        ptr  = NULL;
        ptr2 = NULL;

        if (strcmp(tmpASString,NONE))
          {
          if ((ptr = MUStrTok(tmpASString,":",&TokPtr)) != NULL)
            {
            ptr2 = MUStrTok(NULL,"|",&TokPtr);
            }
          }

        if (MASGetDriver((void **)&J->ASFunc,ptr,msdApplication) == SUCCESS)
          {
          (*J->ASFunc)(J,mascCreate,ptr2,NULL);
          }
        }    /* END if (Mode != msmProfile) */

      /* obtain job step number from LL based job name */

      /* FORMAT:  <FROM_HOST>.<CLUSTER>.<PROC> */

      MUStrCpy(tmpLine,J->Name,sizeof(tmpLine));

      if ((ptr = MUStrTok(tmpLine,".",&TokPtr)) != NULL)
        {
        MUStrCpy(J->SubmitHost,ptr,sizeof(J->SubmitHost));

        if ((ptr = MUStrTok(NULL,".",&TokPtr)) != NULL)
          {
          J->Cluster = (int)strtol(ptr,NULL,0);

          if ((ptr = MUStrTok(NULL,".",&TokPtr)) != NULL)
            {
            J->Proc = (int)strtol(ptr,NULL,0);
            }
          }
        }

      /* adjust job flags */

      if (Mode == msmProfile)
        {
        MUStrDup(&J->MasterHostName,MHostName);

        /* NOTE:  flags may be specified as string or number */

        if ((J->SpecFlags = strtol(tmpJFlags,NULL,0)) == 0)
          {
          MUBMFromString(tmpJFlags,MJobFlags,&J->SpecFlags);
          }
        }
      else
        {
        /* simulation mode */

        if ((MSim.Flags & (1 << msimfIgnMode)) ||
            (MSim.Flags & (1 << msimfIgnAll)))
          {
          J->SpecFlags = 0;
          }
        else
          {
          /* NOTE:  flags may be specified as string or number */

          if ((J->SpecFlags = strtol(tmpJFlags,NULL,0)) == 0)
            {
            MUBMFromString(tmpJFlags,MJobFlags,&J->SpecFlags);
            }

          if (J->SysFlags & (1 << mjfBackfill))
            J->SysFlags ^= (1 << mjfBackfill);

          if ((J->SpecFlags & (1 << mjfHostList)) &&
              (MSim.Flags & (1 << msimfIgnHostList)))
            {
            J->SpecFlags ^= (1 << mjfHostList);
            }
          }
        }    /* END else (Mode == msmProfile) */

      /* extract arbitrary job attributes */

      /* FORMAT:  JATTR:<ATTR>[=<VALUE>] */

      {
      char *ptr;
      char *TokPtr;

      ptr = MUStrTok(tmpJFlags,"[]",&TokPtr);

      while (ptr != NULL)
        {
        if (!strncmp(ptr,"JATTR:",strlen("JATTR:")))
          {
          MJobSetAttr(J,mjaGAttr,(void **)(ptr + strlen("JATTR:")),0,mSet);
          }

        ptr = MUStrTok(NULL,"[]",&TokPtr);
        }  /* END while (ptr != NULL) */
      }    /* END BLOCK */

      /* set default flags, remove set 'ignore' flags */

      J->SpecFlags |= MSim.TraceDefaultJobFlags;

      for (index = 0;index < M64.INTBITS;index++)
        {
        if (!(MSim.TraceIgnoreJobFlags & (1 << index)))
          continue;

        if (!(J->SpecFlags & (1 << index)))
          continue;

        J->SpecFlags ^= (1 << index);
        }  /* END for (index) */

      /* load task info */

      if (!strncmp(tmpTaskRequest,"PBS=",strlen("PBS=")))
        {
        char tmpLine[MAX_MLINE];

        tpbsa_t tmpA;
        short   TaskList[MAX_MTASK];

        memset(&tmpA,0,sizeof(tmpA));

        sprintf(tmpLine,"Resource_List,nodes,%s",
          &tmpTaskRequest[strlen("PBS=")]);

        MPBSJobSetAttr(J,NULL,tmpLine,&tmpA,TaskList,0);

        MPBSJobAdjustResources(J,&tmpA,J->RM);
        }
      else
        {
        ptr = MUStrTok(tmpTaskRequest,",",&TokPtr);
        tindex = 1;

        while (ptr != NULL)
          {
          if (strchr(ptr,'-') != NULL)
            {
            /* NOTE:  not handled */
            }
          else
            {
            RQ[0]->TaskRequestList[tindex] = atoi(ptr);
            tindex++;
            }

          ptr = MUStrTok(NULL,",",&TokPtr);
          }

        RQ[0]->TaskRequestList[tindex] = 0;
        RQ[0]->TaskRequestList[0]      = RQ[0]->TaskRequestList[1];

        RQ[0]->TaskCount    = RQ[0]->TaskRequestList[0];

        J->Request.TC   = RQ[0]->TaskRequestList[0];

        RQ[0]->TasksPerNode = MAX(0,TPN);

        if (MPar[0].JobNodeMatch & (1 << nmExactNodeMatch))
          {
          RQ[0]->NodeCount  = RQ[0]->TaskCount;
          J->Request.NC = RQ[0]->TaskCount;
          }

        if (MPar[0].JobNodeMatch & (1 << nmExactProcMatch))
          {
          if (RQ[0]->TasksPerNode >= 1)
            {
            RQ[0]->RequiredProcs = RQ[0]->TasksPerNode;
            RQ[0]->ProcCmp       = mcmpEQ;
            }

          if (RQ[0]->TasksPerNode > J->Request.TC)
            {
            RQ[0]->TasksPerNode = 0;
            }
          }
        }    /* END BLOCK */

      /* process WCLimit constraints */

      ptr = MUStrTok(tmpSpecWCLimit,",:",&TokPtr);
      tindex = 1;

      while (ptr != NULL)
        {
        if (strchr(ptr,'-') != NULL)
          {
          /* NOTE:  not handled */
          }
        else
          {
          J->SpecWCLimit[tindex] = strtol(ptr,NULL,0);

          tindex++;
          }

        ptr = MUStrTok(NULL,",:",&TokPtr);
        }  /* END while (ptr != NULL) */

      J->SpecWCLimit[tindex] = 0;
      J->SpecWCLimit[0]      = J->SpecWCLimit[1];

      J->SubmitTime     = tmpQTime;
      J->DispatchTime   = MAX(tmpDTime,tmpSTime);
      J->StartTime      = MAX(tmpDTime,tmpSTime);
      J->CompletionTime = tmpCTime;

      J->SystemQueueTime = tmpSQTime;

      J->SpecSMinTime   = tmpSDate;
      J->CMaxTime       = tmpEDate;

      /* determine required reservation */

      if (strcmp(Reservation,NONE) &&
          strcmp(Reservation,J->Name))
        {
        J->SpecFlags |= (1 << mjfAdvReservation);

        if (strcmp(Reservation,ALL) != 0)
          {
          strcpy(J->ResName,Reservation);
          }
        }

      /* determine job class */

      if ((MSim.Flags & (1 << msimfIgnClass)) ||
          (MSim.Flags & (1 << msimfIgnAll)))
        {
        mclass_t *C;

        MClassAdd("DEFAULT",&C);

        /* ignore specified classes */

        RQ[0]->DRes.PSlot[0].count = 1;
        RQ[0]->DRes.PSlot[C->Index].count = 1;
        }
      else
        {
        int cindex;

        MUNumListFromString(RQ[0]->DRes.PSlot,tmpClass,eClass);

        for (cindex = 1;cindex < MAX_MCLASS;cindex++)
          {
          if (RQ[0]->DRes.PSlot[cindex].count > 0)
            {
            MClassAdd(MAList[eClass][cindex],NULL);
            }
          }  /* END for (cindex) */
        }

      if (Mode != msmProfile)
        {
        if ((strcmp(MReqHList,NONE) != 0) &&
           !(MSim.Flags & (1 << msimfIgnHostList)) &&
           !(MSim.Flags & (1 << msimfIgnAll)))
          {
          J->SpecFlags |= (1 << mjfHostList);

          ptr = MUStrTok(MReqHList,":",&TokPtr);

          nindex = 0;

          if (J->ReqHList == NULL)
            J->ReqHList = (mnalloc_t *)calloc(1,sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1));

          J->ReqHList[0].N = NULL;

          while (ptr != NULL)
            {
            if (MNodeFind(ptr,&N) == FAILURE)
              {
              if (MSched.DefaultDomain[0] != '\0')
                {
                if ((tail = strchr(ptr,'.')) != NULL)
                  {
                  /* try short name */

                  MUStrCpy(MReqHList,ptr,MIN(sizeof(MReqHList),(tail - ptr + 1)));
                  }
                else
                  {
                  /* append default domain */

                  if (MSched.DefaultDomain[0] == '.')
                    {
                    sprintf(MReqHList,"%s%s",
                      ptr,
                      MSched.DefaultDomain);
                    }
                  else
                    {
                    sprintf(MReqHList,"%s.%s",
                      ptr,
                      MSched.DefaultDomain);
                    }
                  }

                if (MNodeFind(MReqHList,&N) != SUCCESS)
                  {
                  DBG(1,fUI) DPrint("ALERT:    cannot locate node '%s' for job '%s' hostlist\n",
                    ptr,
                    J->Name);

                  N = NULL;
                  }
                }
              else
                {
                DBG(1,fUI) DPrint("ALERT:    cannot locate node '%s' for job '%s' hostlist\n",
                  ptr,
                  J->Name);

                N = NULL;
                }
              }  /* END if (MNodeFind() == FAILURE) */

            if (N != NULL)
              {
              if (J->ReqHList[nindex].N == N)
                {
                J->ReqHList[nindex].TC++;
                }
              else
                {
                if (J->ReqHList[nindex].N != NULL)
                  nindex++;

                J->ReqHList[nindex].N = N;

                J->ReqHList[nindex].TC = 1;
                }
              }

            ptr = MUStrTok(NULL,":",&TokPtr);
            }    /* END while (ptr != NULL) */

          J->ReqHList[nindex + 1].N = NULL;
          }      /* END if (strcmp(MReqHList,NONE)) */
        else if ((J->SpecFlags & (1 << mjfHostList)) &&
                 (J->ReqHList == NULL))
          {
          DBG(1,fSIM) DPrint("ALERT:    job %s requests hostlist but has no hostlist specified\n",
            J->Name);

          J->SpecFlags ^= (1 << mjfHostList);
          }
        }    /* END if (MODE != msmProfile) */

      break;

    default:

      DBG(4,fSIM) DPrint("ALERT:    cannot load version %d workload trace record\n",
        *Version);

      MJobDestroy(&J);

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }    /* END switch(*Version) */

  if (!strcmp(AName,NONE) ||
      !strcmp(AName,"0"))
    {
    AName[0] = '\0';
    }

  if ((strcmp(tmpRMXString,NONE) != 0) && (strcmp(tmpRMXString,"0") != 0))
    {
    MJobSetAttr(J,mjaRMXString,(void **)tmpRMXString,mdfString,0);
    }

  if (RQ[0]->DRes.Procs == 0)
    RQ[0]->DRes.Procs = 1;

  if (J->SpecWCLimit[0] == (unsigned long)-1)
    J->SpecWCLimit[0] = MAX_MTIME;

  MUStrDup(&J->E.Cmd,tmpCmd);

  if (Mode == msmSim)
    {
    J->State           = mjsIdle;
    J->EState          = mjsIdle;
    J->Bypass          = 0;
    J->PSUtilized      = 0.0;

    J->SystemQueueTime = J->SubmitTime;
    J->SimWCTime       = MAX(1,J->CompletionTime - J->StartTime);

    J->StartTime       = 0;
    J->DispatchTime    = 0;
    J->CompletionTime  = 0;

    J->WCLimit         = J->SpecWCLimit[0];
    }
  else
    {
    /* profile mode */

    int tmpI;

    tmpI = (int)strtol(tmpParName,NULL,0);

    if (tmpI > 0) 
      {
      RQ[0]->PtIndex = tmpI;
      }
    else
      {
      mpar_t *P;

      MParFind(tmpParName,&P);

      RQ[0]->PtIndex = P->Index;
      }

    J->TaskCount = TasksAllocated;

    J->PSDedicated = MJobGetProcCount(J) * (J->CompletionTime - J->StartTime);

    J->WCLimit = J->SpecWCLimit[0];

    for (index = 0;MJobState[index] != NULL;index++)
      {
      if (!strcmp(tmpState,MJobState[index]))
        {
        J->State  = index;
        J->EState = index;

        break;
        }
      }    /* END for (index) */
    }

  if (J->State == mjsNotRun)
    {
    DBG(3,fSIM) DPrint("ALERT:    ignoring job '%s' (job never ran, state '%s')\n",
      J->Name,
      MJobState[J->State]);

    MJobDestroy(&J);

    return(FAILURE);
    }

  /* check for timestamp corruption */

  if (Mode == msmProfile)
    {
    /* profiler mode */

    if (MSched.TraceFlags & (1 << tfFixCorruption))
      {
      if (J->StartTime < J->DispatchTime)
        {
        DBG(2,fSIM) DPrint("WARNING:  fixing job '%s' with corrupt dispatch/start times (%ld < %ld)\n",
          J->Name,
          J->DispatchTime,
          J->StartTime);

        J->DispatchTime = J->StartTime;
        }

      if (J->SubmitTime > J->StartTime)
        {
        DBG(2,fSIM) DPrint("WARNING:  fixing job '%s' with corrupt queue/start times (%ld > %ld)\n",
          J->Name,
          J->SubmitTime,
          J->StartTime);

        if (J->SystemQueueTime > 0)
          J->SubmitTime = MIN(J->StartTime,J->SystemQueueTime);
        else
          J->SubmitTime = J->StartTime;
        }

      if (J->SystemQueueTime > J->DispatchTime)
        {
        DBG(2,fSIM) DPrint("WARNING:  fixing job '%s' with corrupt system queue/dispatch times (%ld > %ld)\n",
          J->Name,
          J->SystemQueueTime,
          J->DispatchTime);

        J->SystemQueueTime = J->DispatchTime;
        }

      if (J->SystemQueueTime < J->SubmitTime)
        {
        DBG(2,fSIM) DPrint("WARNING:  fixing job '%s' with corrupt system queue/queue time (%ld < %ld)\n",
          J->Name,
          J->SystemQueueTime,
          J->SubmitTime);

        J->SystemQueueTime = J->SubmitTime;
        }
      }    /* END if (MSched.TraceFlags & (1 << tfFixCorruption)) */

    if ((J->SubmitTime > J->DispatchTime) ||
        (J->DispatchTime > J->CompletionTime))
      {
      DBG(1,fSIM) DPrint("ALERT:    ignoring job '%s' with corrupt queue/start/completion times (%ld > %ld > %ld)\n",
        J->Name,
        J->SubmitTime,
        J->DispatchTime,
        J->CompletionTime);

      MJobDestroy(&J);

      return(FAILURE);
      }
    }    /* END if (Mode != msmSim) */

  if (MJobSetCreds(J,UName,GName,AName) == FAILURE)
    {
    DBG(1,fSTRUCT) DPrint("ALERT:    ignoring job '%s' with invalid authentication info (%s:%s:%s)\n",
      J->Name,
      UName,
      GName,
      AName);

    MJobDestroy(&J);

    return(FAILURE);
    }

  /* determine network */

  if (strstr(tmpNetwork,MAList[eNetwork][0]) == NULL)
    {
    RQ[0]->Network = MUMAGetIndex(eNetwork,tmpNetwork,mAdd);
    }

  if (strstr(tmpOpsys,MAList[eOpsys][0]) == NULL)
    {
    if ((RQ[0]->Opsys = MUMAGetIndex(eOpsys,tmpOpsys,mAdd)) == FAILURE)
      {
      DBG(0,fSIM) DPrint("WARNING:  cannot add opsys '%s' for job '%s'\n",
        tmpOpsys,
        J->Name);
      }
    }

  if (strstr(tmpArch,MAList[eArch][0]) == NULL)
    {
    if ((RQ[0]->Arch = MUMAGetIndex(eArch,tmpArch,mAdd)) == FAILURE)
      {
      DBG(0,fSIM) DPrint("WARNING:  cannot add arch '%s' for job '%s'\n",
        tmpArch,
        J->Name);
      }
    }

  /* load feature values */

  if ((MSched.ProcSpeedFeatureHeader[0] != '\0') &&
      (MSched.ReferenceProcSpeed > 0))
    {
    sprintf(tmpLine,"[%s",
      MSched.ProcSpeedFeatureHeader);

    if ((ptr = strstr(tmpReqNFList,tmpLine)) != NULL)
      {
      ReqProcSpeed = strtol(ptr + strlen(tmpLine),NULL,0);

      J->SimWCTime *= (long)((double)ReqProcSpeed / MSched.ReferenceProcSpeed);
      }
    }

  if ((MSched.Mode != msmSim) || !(MSim.Flags & (1 << msimfIgnFeatures)))
    {
    if (!strstr(tmpReqNFList,MAList[eFeature][0]))
      {
      tok = MUStrTok(tmpReqNFList,"[]",&TokPtr);

      do
        {
        MUGetMAttr(eFeature,tok,mAdd,RQ[0]->ReqFBM,sizeof(RQ[0]->ReqFBM));
        }
      while ((tok = MUStrTok(NULL,"[]",&TokPtr)) != NULL);
      }
    }

  RQ[0]->MemCmp    = MUCmpFromString(tmpMemCmp,NULL);
  RQ[0]->DiskCmp   = MUCmpFromString(tmpDiskCmp,NULL);

  RQ[0]->NAccessPolicy = MSched.DefaultNAccessPolicy;

  RQ[0]->TaskCount = J->Request.TC;
  RQ[0]->NodeCount = J->Request.NC;

  MJobProcessExtensionString(J,J->RMXString);

  if ((MSim.Flags & (1 << msimfIgnQOS)))
    {
    tmpQReq[0] = '\0';
    }

  {
  char *ptr;
  char *TokPtr;

  ptr = MUStrTok(tmpQReq,":",&TokPtr);

  if ((ptr != NULL) &&
      (ptr[0] != '\0') &&
       strcmp(ptr,"-1") &&
       strcmp(ptr,"0") &&
       strcmp(ptr,NONE) &&
       strcmp(ptr,DEFAULT))
    {
    if (MQOSAdd(ptr,&J->QReq) == FAILURE)
      {
      /* cannot locate requested QOS */

      MQOSFind(DEFAULT,&J->QReq);
      }

    if (Mode == msmProfile)
      {
      if ((ptr = MUStrTok(NULL,":",&TokPtr)) != NULL)
        {
        MQOSAdd(ptr,&J->Cred.Q);
        }
      }
    }    /* END if ((ptr != NULL) && ...) */
  }      /* END BLOCK */

  if (Mode == msmSim)
    {
    if ((MQOSGetAccess(J,J->QReq,NULL,&QDef) == FAILURE) ||
        (J->QReq == NULL) ||
        (J->QReq == &MQOS[0]))
      {
      MJobSetQOS(J,QDef,0);
      }
    else
      {
      MJobSetQOS(J,J->QReq,0);
      }
    }    /* END if (Mode == msmSim) */

  if ((SetString[0] != '0') && (strcmp(SetString,NONE)))
    {
    /* FORMAT:  ONEOF,FEATURE[,X:Y:Z] */

    if ((ptr = MUStrTok(SetString,",: \t",&TokPtr)) != NULL)
      {
      /* determine selection type */

      RQ[0]->SetSelection = MUGetIndex(ptr,(const char **)MResSetSelectionType,0,mrssNONE);

      if ((ptr = MUStrTok(NULL,",: \t",&TokPtr)) != NULL)
        {
        /* determine set attribute */

        RQ[0]->SetType = MUGetIndex(ptr,(const char **)MResSetAttrType,0,mrstNONE);

        index = 0;

        if ((ptr = MUStrTok(NULL,", \t",&TokPtr)) != NULL)
          {
          /* determine set list */

          while (ptr != NULL)
            {
            MUStrDup(&RQ[0]->SetList[index],ptr);

            index++;

            ptr = MUStrTok(NULL,", \t",&TokPtr);
            }
          }

        RQ[0]->SetList[index] = NULL;
        }  /* END if ((ptr = MUStrTok(NULL)) != NULL) */
      }    /* END if ((ptr = MUStrTok(SetString)) != NULL) */
    }      /* END if ((SetString[0] != '0') && (strcmp(SetString,NONE))) */

  if (MJobEval(J) == FAILURE)
    {
    /* job not properly formed */

    DBG(1,fSTRUCT) DPrint("ALERT:    ignoring job '%s' with corrupt configuration\n",
      J->Name);

    MJobDestroy(&J);

    return(FAILURE);
    }

  DBG(6,fSIM) DPrint("INFO:     job '%s' loaded.  class: %s  opsys: %s  arch: %s\n",
    J->Name,
    MUCAListToString(RQ[0]->DRes.PSlot,NULL,NULL),
    MAList[eOpsys][RQ[0]->Opsys],
    MAList[eArch][RQ[0]->Arch]);

  return(SUCCESS);
  }  /* END MTraceLoadWorkload() */






/* END MTrace.c */



