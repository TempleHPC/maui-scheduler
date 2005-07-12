/* HEADER */
 
/*                                           *
 * Contains:                                 *
 *                                           */
 
#include "moab.h"
#include "msched-proto.h"
 
extern msched_t     MSched;
extern mjob_t      *MJob[];
extern mnode_t     *MNode[];
extern mstat_t      MStat;
extern mattrlist_t  MAList;
extern mpar_t       MPar[];
extern mqos_t       MQOS[];
extern mres_t      *MRes[];
 
extern mlog_t       mlog;
 
extern const char  *MPolicyRejection[];
extern const char  *MPolicyMode[];
extern const char  *MComp[];
extern const char  *MNodeState[];
extern const char  *MAllocRejType[];


/* local prototypes */

int __MBFStoreClusterState(short *);
int __MBFRestoreClusterState(short *);
int __MBFReserveNodes(mnalloc_t *);
int __MBFReleaseNodes(short *,mnalloc_t *);





int MBFValue(

  mjob_t       **JobList,
  int            PolicyLevel,
  nodelist_t     NodeList,
  unsigned long  MaxDuration,
  int            NodesAvailable,
  int            ProcsAvailable,
  int            PIndex)

  {
  /* algorithm description */

  /* backfill all jobs normally */

  /* if resources remain, backfill jobs which cannot run due to walltime constraints */

  /* NYI */

  return(SUCCESS);
  }  /* END MBFValue() */





int MBFPreempt(

  mjob_t        *JobList[],
  int            PolicyLevel,
  nodelist_t     NodeList,
  unsigned long  MaxDuration, 
  int            NodesAvailable,
  int            ProcsAvailable,
  mpar_t        *P)

  {
  int           jindex;
  mnodelist_t   MNodeList;

  double        BestBFPriority;
  int           BestJIndex;

  char          NodeMap[MAX_MNODE];
 
  int           CurrentProcCount;
  int           CurrentNodeCount;

  int           ProcsRequested;
  int           NodesRequested;
 
  mjob_t       *J;

  double        JobBFPriority;

  const char *FName = "MBFPreempt";
 
  DBG(2,fSCHED) DPrint("%s(%s,%s,%ld,%d,%d,%s)\n",
    FName,
    (JobList != NULL) ? "JobList" : "NULL",
    (NodeList != NULL) ? "NodeList" : "NULL", 
    MaxDuration,
    NodesAvailable,
    ProcsAvailable,
    P->Name);
 
  CurrentNodeCount = NodesAvailable;
  CurrentProcCount = ProcsAvailable;
 
  while (1)
    {
    BestJIndex     = -1;
    BestBFPriority = 0.0;
 
    for (jindex = 0;JobList[jindex] != NULL;jindex++)
      {
      J = JobList[jindex];

      if (J == (mjob_t *)1)
        continue;
 
      ProcsRequested = MJobGetProcCount(J);
      NodesRequested = J->Request.NC;

      if ((ProcsRequested > CurrentProcCount) ||
          (NodesRequested > CurrentNodeCount))
        {
        /* inadequate resources remaining */

        JobList[jindex] = (mjob_t *)1;

        continue;
        }

      MJobGetBackfillPriority(
        J,
        MaxDuration,
        0,
        &JobBFPriority,
        NULL);

      if (JobBFPriority > BestBFPriority)
        {
        BestBFPriority = JobBFPriority;
        BestJIndex     = jindex;
        }
      }    /* END for (jindex) */

    if (BestJIndex == -1)
      {
      /* no feasible job found */

      return(SUCCESS);
      }

    MStat.JobsEvaluated++;

    J = JobList[BestJIndex];

    if (MJobCheckLimits(
          J,
          PolicyLevel,
          P,
          (1 << mlActive),
          NULL) == FAILURE)
      {
      DBG(4,fSCHED) DPrint("INFO:     job '%s' rejected by active policy\n",
        J->Name);

      JobList[BestJIndex] = (mjob_t *)1;       
 
      continue;
      }    /* END if (MJobCheckLimits() == FAILURE) */
 
    if (MJobSelectMNL(
         J,
         P,
         NodeList,
         MNodeList,
         NodeMap,
         MBNOTSET) == FAILURE)
      {
      DBG(4,fSCHED) DPrint("INFO:     cannot select procs for job '%s'\n",
        J->Name);

      JobList[BestJIndex] = (mjob_t *)1;     
 
      continue;
      }    /* END if (MJobSelectMNL() == FAILURE) */

    if (MJobAllocMNL(
         J,
         MNodeList,
         NodeMap,
         0,
         P->NAllocPolicy,
         MSched.Time) == FAILURE)
      {
      DBG(4,fSCHED) DPrint("INFO:     cannot allocate resources for job '%s'\n",
        J->Name);
 
      JobList[BestJIndex] = (mjob_t *)1;
 
      continue;
      }    /* END if (MJobAllocMNL() == FAILURE) */

    if (MJobStart(J) == FAILURE)
      {
      DBG(4,fSCHED) DPrint("INFO:     cannot start job '%s'\n",
        J->Name);
 
      JobList[BestJIndex] = (mjob_t *)1;
 
      continue;
      }  /* END if (MJobStart(J) == FAILURE) */

    /* job successfully started */

    J->SysFlags |= (1 << mjfBackfill);

    /* if job is backfilled, mark job as preemptible */
 
    J->SpecFlags |= (1 << mjfPreemptee);

    MJobUpdateFlags(J);
 
    MStatUpdateBFUsage(J);

    ProcsRequested = MJobGetProcCount(J);
    NodesRequested = J->Request.NC;

    CurrentNodeCount -= NodesRequested;
    CurrentProcCount -= ProcsRequested;

    JobList[BestJIndex] = (mjob_t *)1;
    }  /* END while (1) */

  return(SUCCESS);
  }  /* END MBFPreempt() */




/* firstfit backfill */
 
int MBFFirstFit(
 
  mjob_t       *BFQueue[],
  int           PolicyLevel,
  nodelist_t    BFNodeList,
  unsigned long Duration,
  int           BFNodeCount,
  int           BFProcCount,
  mpar_t       *P)
 
  {
  int           jindex;
  int           sindex;
 
  mnodelist_t   MNodeList;
  mnodelist_t   BestMNodeList;
 
  mjob_t       *J;
  mreq_t       *RQ;
 
  char          tmpNodeMap[MAX_MNODE];
  char          NodeMap[MAX_MNODE];
  char          BestNodeMap[MAX_MNODE];
 
  int           PC;
  unsigned long SpecWCLimit;
 
  int           BestSIndex;
  int           BestSVal;
 
  int           DefaultTaskCount;
  unsigned long DefaultSpecWCLimit;

  mbool_t       ChunkingEnabled = FALSE;
  mbool_t       ChunkingActive = FALSE;
 
  const char *FName = "MBFFirstFit";
 
  DBG(2,fSCHED) DPrint("%s(BFQueue,%d,BFNodeList,%ld,%d,%d,%s)\n",
    FName,
    PolicyLevel,
    Duration, 
    BFNodeCount,
    BFProcCount,
    P->Name);

  J = NULL;
  PC = 0;

  if ((MPar[0].BFChunkSize > 0) && (MPar[0].BFChunkBlockTime > MSched.Time))
    {
    ChunkingEnabled = TRUE;
    }
  else
    {
    ChunkingEnabled = FALSE;
    }
 
  for (jindex = 0;BFQueue[jindex] != NULL;jindex++)
    {
    if ((ChunkingEnabled == TRUE) && (ChunkingActive == FALSE))
      {
      /* evaluate previous job */

      /* if chunking is enabled and job is chunksize or larger, */
      /* and job is blocked, activate chunking */

      if ((J != NULL) &&
          (MJOBISACTIVE(J) == FALSE) &&
          (PC >= MPar[0].BFChunkSize))
        {
        ChunkingActive = TRUE;
        }
      }

    J = BFQueue[jindex];
 
    RQ = J->Req[0];
 
    MTRAPJOB(J,FName);
 
    if ((J->State != mjsIdle) || (J->EState != mjsIdle))
      {
      continue;
      }
 
    MStat.JobsEvaluated++;
 
    /* try all proc count/wclimit combinations */
 
    BestSIndex = -1;
    BestSVal   = 0;
 
    /* FIXME:  handles only one req */
 
    DefaultSpecWCLimit = J->SpecWCLimit[0];
    DefaultTaskCount   = RQ->TaskRequestList[0];
 
    for (sindex = 0;RQ->TaskRequestList[sindex] > 0;sindex++)
      {
      if ((sindex != 0) &&
          (RQ->TaskRequestList[sindex] == RQ->TaskRequestList[0]))
        {
        /* ignore duplicate shapes */
 
        continue;
        } 
 
      SpecWCLimit = (J->SpecWCLimit[sindex] > 0) ?
        J->SpecWCLimit[sindex] : J->SpecWCLimit[0];
 
      if (sindex != 0)
        MJobUpdateResourceCache(J,sindex);
 
      PC = MJobGetProcCount(J);

      if ((ChunkingActive == TRUE) && (PC < MPar[0].BFChunkSize))
        {
        /* job blocked by chunking */

        DBG(5,fSCHED) DPrint("INFO:     %s:  job %s.%d blocked by chunking (P: %d T: %ld)\n",
          FName,
          J->Name,
          sindex,
          PC,
          SpecWCLimit);

        continue;
        }
 
      DBG(5,fSCHED) DPrint("INFO:     %s:  evaluating job %s.%d (P: %3d T: %ld)\n",
        FName,
        J->Name,
        sindex,
        PC,
        SpecWCLimit);
 
      if (PC > BFProcCount)
        {
        /* NOTE:  re-evaluate/BF may have adjusted initial availability */

        continue;
        }
 
      /* NOTE:  try all shapes, select shape with best job turnaround time */
 
      J->SpecWCLimit[0]      = SpecWCLimit;
      RQ->TaskRequestList[0] = RQ->TaskRequestList[sindex];
      RQ->TaskCount          = RQ->TaskRequestList[sindex];
      J->Request.TC      = RQ->TaskRequestList[sindex];
 
      if (MJobCheckLimits(
            J,
            PolicyLevel,
            P,
            (1 << mlActive),
            NULL) == FAILURE)
        { 
        DBG(8,fSCHED) DPrint("INFO:     job %s.%d does not meet active fairness policies\n",
          J->Name,
          sindex);
 
        continue;
        }
 
      if (MJobSelectMNL(
            J,
            P,
            BFNodeList,
            MNodeList,
            NodeMap,
            MBNOTSET) == FAILURE)
        {
        DBG(8,fSCHED) DPrint("INFO:     cannot locate available resources for job %s.%d\n",
          J->Name,
          sindex);
 
        continue;
        }
 
      memcpy(tmpNodeMap,NodeMap,sizeof(NodeMap));
 
      if (MJobAllocMNL(
            J,
            MNodeList,
            NodeMap,
            0,
            P->NAllocPolicy,
            MSched.Time) == FAILURE)
        {
        continue;
        }
 
      /* job is feasible */
 
      if (PC > BestSVal)
        {
        /* select largest mshape */

        BestSVal   = PC; 
 
        BestSIndex = sindex;
 
        memcpy(BestNodeMap,tmpNodeMap,sizeof(BestNodeMap));
        memcpy(BestMNodeList,MNodeList,sizeof(BestMNodeList));
        }
      }  /* END for (sindex) */
 
    /* FIXME:  only handles one req per job */
 
    MJobUpdateResourceCache(J,0);
 
    J->SpecWCLimit[0]      = DefaultSpecWCLimit;
    RQ->TaskRequestList[0] = DefaultTaskCount;
    J->Request.TC      = DefaultTaskCount;
    RQ->TaskCount          = DefaultTaskCount;
 
    if (BestSIndex >= 0)
      {
      J->SpecWCLimit[0]      = J->SpecWCLimit[BestSIndex];
      RQ->TaskRequestList[0] = RQ->TaskRequestList[BestSIndex];
      RQ->TaskCount          = RQ->TaskRequestList[BestSIndex];
      J->Request.TC      = RQ->TaskRequestList[BestSIndex];
 
      if (sindex > 1)
        {
        if (MJobAllocMNL(
              J,
              BestMNodeList,
              BestNodeMap,
              0,
              P->NAllocPolicy,
              MSched.Time) == FAILURE)
          {
          DBG(0,fSCHED) DPrint("ERROR:    %s:  cannot allocate nodes for job %s.%d\n",
            FName,
            J->Name,
            BestSIndex);
 
          J->SpecWCLimit[0]      = DefaultSpecWCLimit;
          RQ->TaskRequestList[0] = DefaultTaskCount;
          J->Request.TC      = DefaultTaskCount; 
 
          continue;
          }
        }    /* END if (sindex > 1) */
 
      if (MJobStart(J) == FAILURE)
        {
        DBG(0,fSCHED) DPrint("ERROR:    %s:  cannot start job %s.%d\n",
          FName,
          J->Name,
          BestSIndex);
 
        J->SpecWCLimit[0]      = DefaultSpecWCLimit;
        RQ->TaskRequestList[0] = DefaultTaskCount;
        J->Request.TC      = DefaultTaskCount;
        J->TaskCount           = DefaultTaskCount;
        }
      else
        {
        DBG(4,fSCHED) DPrint("INFO:     %s:  job %s.%d started\n",
          FName,
          J->Name,
          BestSIndex);
 
        J->SysFlags |= (1 << mjfBackfill);
 
        MJobUpdateFlags(J);
 
        MStatUpdateBFUsage(J);
        }
      }   /* END if (BestSIndex >= 0) */
    }     /* END for jindex           */
 
  DBG(2,fSCHED) DPrint("INFO:     partition %s nodes/procs available after %s: %d/%d (%d jobs examined)\n",
    P->Name,
    FName,
    P->IdleNodes,
    P->ARes.Procs,
    jindex); 
 
  return(SUCCESS);
  }  /* END MBFFirstFit() */




int __MBFStoreClusterState(
 
  short *NSList)
 
  {
  int nindex;

  mnode_t *N;

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if (N != NULL)    
      NSList[nindex] = N->State;
    else
      NSList[nindex] = 0;
    }  /* END for (nindex) */
 
  return(SUCCESS);
  }  /* END __MBFStoreClusterState() */
 
 
 
 
int __MBFRestoreClusterState(
 
  short *NSList)
 
  {
  int nindex;

  mnode_t *N;

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if (N != NULL)
      N->State = NSList[nindex];
    }  /* END for (nindex) */
 
  return(SUCCESS);
  }  /* END __MBFRestoreClusterState() */ 



 
int __MBFReserveNodes(
 
  mnalloc_t *NL)
 
  {
  int nindex;

  mnode_t *N;

  for (nindex = 0;NL[nindex].N != NULL;nindex++)
    {
    N = NL[nindex].N;

    if (N != NULL)
      N->State = mnsReserved;
    }  /* END for (nindex) */
 
  return(SUCCESS);
  }  /* END __MBFReserveNodes() */
 
 
 
 
 
int __MBFReleaseNodes(
 
  short     *NSList,
  mnalloc_t *NL)
 
  {
  int nindex = 0;

  mnode_t *N;

  for (nindex = 0;NL[nindex].N != NULL;nindex++)
    {
    N = NL[nindex].N;

    if (N != NULL)
      N->State = NSList[NL[nindex].N->Index];
    }  /* END for (nindex) */
 
  return(SUCCESS);
  }  /* END __MBFReleaseNodes() */




int MBFGetWindow(

  int    *BFNodeCount,   /* O:  nodes available                */
  int    *BFTaskCount,   /* O:  tasks available                */
  nodelist_t BFNodeList, /* O:  nodelist available             */
  long   *BFTime,        /* O:  duration available             */
  long    SMinTime,      /* I:  duration required              */
  mpar_t *P,             /* I:  partition                      */
  char   *UName,         /* I:  user name of window requestor  */
  char   *GName,         /* I:  group name of window requestor */
  char   *AName,         /* I:  account of window requestor    */
  int     MemCmp,        /* I:  memory comparison required     */
  int     Memory,        /* I:  memory required on node        */
  unsigned long WCLimit,
  mcres_t *DRes,         /* I:  dedicated resources required per task */
  char   *ClassString,
  char   *FeatureString, /* I */
  char   *QOSName,
  char   *Msg)           /* O:  descriptive message            */

  {
  int  nindex;

  long ETime;

  int  NodeCount;
  int  TaskCount;

  int  RIndex;

  long AvailableTime;

  char Affinity;

  int  TC;

  mjob_t   tmpJ;
  mreq_t   tmpRQ;

  macl_t   tmpCL[MAX_MACL];
  macl_t   tmpACL[MAX_MACL];

  mnalloc_t tmpNodeList[MAX_MNODE + 1];

  mnode_t *N;
  mjob_t  *J;
  mreq_t  *RQ;

  mgcred_t *tmpADef;
  mqos_t   *tmpQDef;
  int       tmpQAL[(MAX_MQOS >> 5) + 1];

  char     BRes[MAX_MNAME];

  char    *ptr;
  char    *TokPtr;

  const char *FName = "MBFGetWindow";

  DBG(3,fSCHED) DPrint("%s(BFNodeCount,BFTaskCount,BFNodeList,BFTime,%ld,%s,%s,%s,%s,'%s %d',%lu,DRes,%s,%s,%s,%s)\n",
    FName,
    SMinTime,
    P->Name,
    UName,
    GName,
    (AName != NULL) ? AName : "NULL",
    MComp[MemCmp],
    Memory,
    WCLimit,
    (ClassString != NULL) ? ClassString : "NULL",
    (FeatureString != NULL) ? FeatureString : "NULL",
    (QOSName != NULL) ? QOSName : "NULL",
    (Msg == NULL) ? "NULL" : "Msg");

  if (SMinTime >= MAX_MTIME)
    {
    return(FAILURE);
    }

  /* build temp job */

  MJobMkTemp(&tmpJ,&tmpRQ,tmpACL,tmpCL,tmpNodeList,NULL);

  J  = &tmpJ;
  RQ = J->Req[0];  /* create single req job */

  strcpy(J->Name,"BFWindow");    

  strcpy(BRes,"UNKNOWN");

  memcpy(&RQ->DRes,DRes,sizeof(DRes));

  if (MJobSetCreds(J,UName,GName,AName) == FAILURE)
    {
    DBG(1,fCORE) DPrint("ALERT:    cannot create authentication for backfill pseudo-job (U: %s  G: %s  A: %s)\n",
      UName,
      GName,
      (AName == NULL) ? "NULL" : AName);

    return(FAILURE);
    }

  J->WCLimit        = WCLimit;
  J->SpecWCLimit[0] = WCLimit;

  MClassFind(
    (ClassString != NULL) ? ClassString : (char *)ALL,
    &J->Cred.C);

  MJobGetAccount(J,&tmpADef);

  if ((AName == NULL) || 
       !strncmp(AName,"ALL",3) ||
       !strncmp(AName,NONE,strlen(NONE))) 
    {
    J->Cred.A = tmpADef;
    }

  MQOSGetAccess(J,NULL,tmpQAL,&tmpQDef);

  if ((QOSName == NULL) || !strncmp(QOSName,NONE,strlen(NONE)))
    {
    J->QReq = tmpQDef;
    }
  else
    {
    MQOSFind(QOSName,&J->QReq);

    if ((J->QReq == NULL) ||
        (J->QReq == &MQOS[0]) || 
        !MUBMCheck(J->QReq->Index,tmpQAL)) 
      {
      J->QReq = tmpQDef;
      }
    }

  MJobSetQOS(J,J->QReq,0);

  MJobBuildCL(J);

  if ((FeatureString == NULL) || !strcmp(FeatureString,NONE))
    {
    memset(RQ->ReqFBM,0,sizeof(RQ->ReqFBM));
    }
  else
    {
    ptr = MUStrTok(FeatureString,":",&TokPtr);

    while (ptr != NULL)
      {
      MUGetMAttr(eFeature,ptr,mAdd,RQ->ReqFBM,sizeof(RQ->ReqFBM));

      DBG(6,fSCHED) DPrint("INFO:     feature '%s' added (%s)\n",
        ptr,
        MUMAList(eFeature,RQ->ReqFBM,sizeof(RQ->ReqFBM)));

      ptr = MUStrTok(NULL,":",&TokPtr);
      }
    }

  RQ->Index        = 0;

  RQ->TaskCount    = DRes->Procs;
  RQ->TasksPerNode = 0;

  ETime            = MAX_MTIME;

  NodeCount        = 0;
  TaskCount        = 0;

  if (Msg != NULL)
    Msg[0] = '\0';

  nindex = 0;

  if (MJobCheckLimits(
        J, 
        ptSOFT, 
        P, 
        (1<<mlActive)|(1<<mlIdle)|(1<<mlSystem),
        NULL) == FAILURE) 
    {
    nindex = MAX_MNODE;
    }

  /* locate all idle nodes with AvailableTime >/< SMinTime */

  for (;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    /* determine node availability */

    if (((N->State != mnsIdle) && (N->State != mnsActive)) ||
        ((N->EState != mnsIdle) && (N->EState != mnsActive)))
      {
      DBG(6,fSCHED) DPrint("INFO:     node %s not considered for backfill (State: %s/EState: %s)\n",
        N->Name,
        MNodeState[N->State],
        MNodeState[N->EState]);
      
      if (Msg != NULL)
        {
        if ((N->State != mnsIdle) && (N->State != mnsActive))
          {
          sprintf(Msg,"%snode %s is unavailable (state '%s')\n",
            Msg,
            N->Name,
            MNodeState[N->State]);
          }
        else
          {
          sprintf(Msg,"%snode %s is unavailable (expected state '%s')\n",
            Msg,
            N->Name,
            MNodeState[N->EState]);
          }
        }

      continue;
      }  /* END if (N->State ...) */

    if ((N->PtIndex != P->Index) && (P->Index != 0))
      {
      DBG(6,fSCHED) DPrint("INFO:     node %s not considered for backfill (invalid partition)\n",
        N->Name);

      if (Msg != NULL)
        sprintf(Msg,"%snode %s is unavailable (partition '%s')\n",
          Msg,
          N->Name,
          MPar[N->PtIndex].Name);

      continue;
      }

    if (MReqCheckResourceMatch(J,RQ,N,&RIndex) == FAILURE)
      {
      DBG(6,fSCHED) DPrint("INFO:     node '%s' rejected.  (%s)\n",
        N->Name,
        MAllocRejType[RIndex]);

      if (Msg != NULL)
        {
        sprintf(Msg,"%snode %s is unavailable (%s)\n",
          Msg,
          N->Name,
          MAllocRejType[RIndex]);
        }

      continue;
      }    /* END if (MReqCheckResourceMatch(J,RQ,N,&RIndex) == FAILURE) */

    /* check reservations */

    MTRAPNODE(N,FName);

    if (MJobCheckNRes(
          J,
          N,
          RQ,
          MSched.Time,
          &TC,
          1.0,
          &RIndex,
          &Affinity,
	  NULL,
          TRUE) == FAILURE)
      {
      /* node does not meet time/resource requirements */

      if (Msg != NULL)
        {
        sprintf(Msg,"%snode %s does not meet requirements (%s)\n",
          Msg,
          N->Name,
          MAllocRejType[RIndex]);
        }

      continue;
      }

    if (MJobGetNRange(
          J,
          RQ,
          N,
          MSched.Time,
          &TC,
          &AvailableTime,
          &Affinity,
          NULL,
          BRes) == FAILURE)
      {
      /* node does not have resources immediately available */

      if (Msg != NULL)
        {
        sprintf(Msg,"%snode %s is blocked immediately\n",
          Msg,
          N->Name);
        }

      continue;
      }

#ifdef __MLONGESTBFWINDOWFIRST
    if (AvailableTime >= SMinTime)
#else /* __MLONGESTBFWINDOWFIRST */
    if (AvailableTime <= SMinTime)
#endif /* __MLONGESTBFWINDOWFIRST */
      {
      if (Msg != NULL)
        {
        if (AvailableTime < (MAX_MTIME >> 1))
          {
          sprintf(Msg,"%snode %s is blocked by reservation %s in %s\n",
            Msg,
            N->Name,
            BRes,
            MULToTString(AvailableTime));
          }
        else
          {
          sprintf(Msg,"%snode %sx%d is available with no timelimit\n",
            Msg,
            N->Name,
            TC);
          }
        }

      continue;
      }
    else
      {
      if (Msg != NULL)
        {
        sprintf(Msg,"%snode %sx%d is available with no timelimit\n",
          Msg,
          N->Name,
          TC);
        }
      }

#ifdef __MLONGESTBFWINDOWFIRST
    if (AvailableTime > ETime)
#else /* __MLONGESTBFWINDOWFIRST */
    if (AvailableTime < ETime)
#endif /* __MLONGESTBFWINDOWFIRST */
      {
      DBG(5,fSCHED) DPrint("INFO:     node %s found with best time %ld\n",
        N->Name,
        AvailableTime);

      ETime = AvailableTime;
      }
    else
      {
      DBG(5,fSCHED) DPrint("INFO:     node %s found with available time %ld\n",
        N->Name,
        AvailableTime);
      }

    if (BFNodeList != NULL)
      {
      BFNodeList[NodeCount].N  = N;
      BFNodeList[NodeCount].TC = TC;

      NodeCount++;
      TaskCount += TC;
      }
    }       /* END for nindex */

  /* locate ETime bounded by reservation max time */

  {
  int rindex;

  mres_t *R;

  long    tmpL;

  for (rindex = 0;MRes[rindex] != NULL;rindex++)
    {
    R = MRes[rindex];

    if ((R->Name[0] == '\0') || (R->Name[0] == '\1'))
      continue;

    if (R->Type == mrtJob)
      continue;

    if (MACLGet(R->ACL,maDuration,(void **)&tmpL,NULL) == FAILURE)
      continue;

#ifdef __MLONGESTBFWINDOWFIRST
    {
    if (tmpL >= SMinTime)
      continue;

    if (tmpL > ETime)
      {
      DBG(5,fSCHED) DPrint("INFO:     reservation %s found with best time %ld\n",
        R->Name,
        tmpL);

      ETime = tmpL;
      }
    }  /* END BLOCK */
#else /* __MLONGESTBFWINDOWFIRST */
    {
    if (tmpL <= SMinTime)
      continue;

    if (tmpL < ETime)
      {
      DBG(5,fSCHED) DPrint("INFO:     reservation %s found with best time %ld\n",
        R->Name,
        tmpL);

      ETime = tmpL;
      }
    }  /* END BLOCK */
#endif /* __MLONGESTBFWINDOWFIRST */
    }  /* END for (rindex) */
  }    /* END BLOCK */

  if (BFTime != NULL)
    *BFTime = ETime;

  if (BFNodeCount != NULL)
    *BFNodeCount = NodeCount;

  if (BFTaskCount != NULL)
    *BFTaskCount = TaskCount;

  if (BFNodeList != NULL)
    {
    BFNodeList[NodeCount].N = NULL;
    }

  DBG(3,fSCHED) DPrint("INFO:     backfill window:  time: %s  nodes: %3d  tasks: %3d  mintime: %5ld (idle nodes: %d)\n",
    MULToTString(*BFTime),
    *BFNodeCount,
    *BFTaskCount,
    SMinTime,
    MPar[0].IdleNodes);

  if ((*BFNodeCount == 0) || (*BFTime == 0))
    {
    /* no backfill window located */

    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MBFGetWindow() */





int MBFBestFit(

  mjob_t       *BFQueue[],
  int           PolicyLevel,
  nodelist_t    BFNodeList,
  unsigned long Duration,
  int           BFNodeCount,
  int           BFProcCount,
  mpar_t       *P)

  {
  int          jindex;
  mnodelist_t  MNodeList;

  unsigned long BFValue;
  unsigned long tmpBFValue;

  mjob_t      *BFJob;
  int          BFIndex;

  char         NodeMap[MAX_MNODE];

  int          APC;
  int          ANC;

  int          PC;

  mjob_t      *LBFQueue[MAX_MJOB];

  mjob_t      *J;

  mpar_t      *GP = &MPar[0];

  const char *FName = "MBFBestFit";

  DBG(2,fSCHED) DPrint("%s(BFQueue,BFNodeList,%ld,%d,%d,%s)\n",
    FName,
    Duration,
    BFNodeCount,
    BFProcCount,
    P->Name);

  memcpy(LBFQueue,BFQueue,sizeof(LBFQueue));

  ANC = BFNodeCount;
  APC = BFProcCount;

  while (1)
    {
    BFJob   = NULL;
    BFValue = 0;
    BFIndex = 0;

    for (jindex = 0;LBFQueue[jindex] != NULL;jindex++)
      {
      if (LBFQueue[jindex] == (mjob_t *)1)
        {
        /* job is not eligible */

        continue;
        }

      J = LBFQueue[jindex];

      PC = MJobGetProcCount(J);

      DBG(3,fSCHED) DPrint("INFO:     attempting BF backfill with job '%15s' (p: %3d t: %ld)\n",
        J->Name,
        PC,
        J->WCLimit);

      if ((J->State != mjsIdle) || (J->EState != mjsIdle))
        {
        LBFQueue[jindex] = (mjob_t *)1;

        continue;
        }

      if (PC > APC)
        {
        LBFQueue[jindex] = (mjob_t *)1;

        continue;
        }

      /* determine job utility metric */

      switch(GP->BFMetric)
        {
        case mbfmPS:

          tmpBFValue = PC * J->WCLimit;

          break;

        case mbfmWalltime:

          tmpBFValue = J->WCLimit;

          break;

        case mbfmProcs:
        default:
 
          tmpBFValue = PC;
 
          break;
        }  /* END switch(GP->BFMetric) */

      if (tmpBFValue <= BFValue)
        {
        /* better job already located */

        continue;
        }

      MStat.JobsEvaluated++;

      if (MJobCheckLimits(
            J,
            PolicyLevel,
            P,
            (1 << mlActive),
            NULL) == FAILURE)
        {
        DBG(8,fSCHED) DPrint("INFO:     job %s does not meet active fairness policies\n",
          J->Name);

        LBFQueue[jindex] = (mjob_t *)1;    
 
        continue;
        }  /* END if (MJobCheckLimits() == FAILURE) */

      if (MJobSelectMNL(
           J,
           P,
           BFNodeList,
           MNodeList,
           NodeMap,
           MBNOTSET) != SUCCESS)
        {
        LBFQueue[jindex] = (mjob_t *)1;    

        DBG(4,fSCHED) DPrint("INFO:     cannot select tasks for job '%s'\n",
          J->Name);

        continue;
        }    /* END if (MJobSelectMNL() != SUCCESS) */

      /* located potential job */

      BFValue = tmpBFValue;
      BFJob   = LBFQueue[jindex];
      BFIndex = jindex;

      DBG(2,fSCHED) DPrint("INFO:     located job '%s' in %s (size: %d duration: %ld)\n",
        J->Name,
        FName,
        PC,
        J->WCLimit);
      }  /* END for (jindex) */

    if (BFJob == NULL)
      {
      /* no eligible jobs found */
 
      DBG(5,fSCHED) DPrint("INFO:     no jobs found to backfill\n");
 
      break;
      }

    J  = BFJob;

    PC = MJobGetProcCount(J);

    LBFQueue[BFIndex] = (mjob_t *)1;

    if (MJobSelectMNL(
         J,
         P,
         BFNodeList,
         MNodeList,
         NodeMap,
         MBNOTSET) == FAILURE)
      {
      DBG(0,fSCHED) DPrint("ERROR:    cannot get tasks selected in %s\n",
        FName);

      continue;
      }

    if (MJobAllocMNL(
          J,
          MNodeList,
          NodeMap,
          0,
          P->NAllocPolicy,
          MSched.Time) == FAILURE)
      {
      DBG(1,fSCHED) DPrint("ERROR:    cannot allocate nodes for job '%s' in %s\n",
        J->Name,
        FName);

      continue;
      }

    if (MJobStart(J) == FAILURE)
      {
      DBG(1,fSCHED) DPrint("ERROR:    cannot start job '%s' in %s\n",
        J->Name,
        FName);

      continue;
      }

    /* job successfully started */

    J->SysFlags |= (1 << mjfBackfill);

    MJobUpdateFlags(J);          

    MStatUpdateBFUsage(J);

    APC -= PC;
    }        /* END while(1) */

  DBG(2,fSCHED) DPrint("INFO:     partition %s nodes/procs available after backfill: %d/%d\n",
    P->Name,
    P->IdleNodes,
    P->ARes.Procs);

  return(SUCCESS);
  }  /* END MBFBestFit() */





int MBFGreedy(

  mjob_t       *BFQueue[],
  int           PolicyLevel,
  nodelist_t    BFNodeList,
  unsigned long Duration,
  int           BFNodeCount,
  int           BFProcCount,
  mpar_t       *P)

  {
  mnodelist_t  MNodeList;

  mjob_t      *BestList[MAX_MJOB];
  int          BestValue;

  mjob_t      *BFList[MAX_MJOB];
  int          BFValue;
  short        BFIndex[MAX_MJOB];        

  mjob_t      *LBFQueue[MAX_MJOB];      

  int          SPC;

  mjob_t      *J;

  int          StartIndex;

  int          index;
  int          sindex;
  int          jindex;

  int          scount;

  int          Failure;

  char         NodeMap[MAX_MNODE];

  nodelist_t   NodeList;

  short        NSList[MAX_MNODE];

  static int   ReservationMisses = 0;

  int          PC;

  mpar_t      *GP;

  const char *FName = "MBFGreedy";

  GP = &MPar[0];

  DBG(1,fSCHED) DPrint("%s(BFQueue,BFNodeList,%ld,%d,%d,%s)\n",
    FName,
    Duration,
    BFNodeCount,
    BFProcCount,
    P->Name);

  /* initialize state */

  StartIndex  = 0;

  BFValue     = 0;
  BFList[0]   = NULL;
  
  BestValue   = 0;
  BestList[0] = NULL;

  sindex = 0;
  scount = 0;

  memset(LBFQueue,0,sizeof(LBFQueue));
  memset(BFIndex,0,sizeof(BFIndex));    

  SPC = 0;

  __MBFStoreClusterState(NSList);

  DBG(4,fSCHED) DPrint("INFO:     greedy backfill procs : %03d  time: %06ld\n",
    BFProcCount,
    Duration);

  DBG(4,fSCHED)
    {
    DBG(4,fSCHED) DPrint("INFO:     backfill jobs: ");

    for (index = 0;LBFQueue[index] != NULL;index++)
      {
      fprintf(mlog.logfp,"[%s]",
        LBFQueue[index]->Name);
      }

    fprintf(mlog.logfp,"\n");
    }

  while(scount++ < GP->BFMaxSchedules)
    {
    BFValue        = 0;
    BFList[sindex] = NULL;

    if ((BestValue > 0) && (BestList[0] == NULL))
      {
      DBG(0,fSCHED) DPrint("ERROR:    BestVal %d achieved but schedule is empty (%p, %p, ...)\n",
        BestValue,
        BestList[0],
        BestList[1]);
  
      __MBFRestoreClusterState(NSList);
 
      return(FAILURE);
      }

    /* locate next job for current schedule */

    for (jindex = StartIndex;LBFQueue[jindex] != NULL;jindex++)
      {
      if (LBFQueue[jindex] == (mjob_t *)1)
        {
        /* job is ineligible */

        continue;
        }

      J = LBFQueue[jindex];

      PC = MJobGetProcCount(J);

      if (scount >= GP->BFMaxSchedules)
        {
        DBG(1,fSCHED) DPrint("ALERT:    max backfill schedules reached in %s (%d) on iteration %d\n",
          FName,
          GP->BFMaxSchedules,
          MSched.Iteration);

        break;
        }

      DBG(5,fSCHED)
        {
        if (sindex > 0)
          {
          DBG(4,fSCHED) DPrint("INFO:     checking  ");

          for (index = 0;index < sindex;index++)
            {
            fprintf(mlog.logfp,"[%s]",
              BFList[index]->Name);
            }  /* END for (index) */

          fprintf(mlog.logfp," (%03d)\n",
            SPC);
          }
        else
          {
          DBG(4,fSCHED) DPrint("INFO:     checking empty list\n");
          }
        
        DBG(4,fSCHED) DPrint("INFO:     checking S[%02d] '%s'  (p: %03d t: %06ld)\n",
          jindex,
          J->Name,
          PC,
          J->WCLimit);
        }

      if ((J->State != mjsIdle) || (J->EState != mjsIdle))
        {
        DBG(6,fSCHED) DPrint("INFO:     job %s is not idle\n",
          J->Name);

        LBFQueue[jindex] = (mjob_t *)1;

        continue;
        }

      if (J->SpecWCLimit[0] > Duration)
        {
        /* NOTE:  check should incorporate machine speed (NYI) */

        LBFQueue[jindex] = (mjob_t *)1;        

        continue;
        }

      if (PC > BFProcCount)
        {
        /* job will not fit in backfill window */

        LBFQueue[jindex] = (mjob_t *)1;        

        continue;
        }

      if ((PC + SPC) > BFProcCount)
        {
        /* job will not fit in existing schedule */

        continue;
        }

      DBG(4,fSCHED) DPrint("INFO:     feasible backfill MJob[%03d] (P: %3d T: %ld)\n",
        J->Index,
        PC,
        J->WCLimit);

      /* NYI:  must reserve and unreserve nodes */

      MStat.JobsEvaluated++;

      if (MJobCheckLimits(
            J,
            PolicyLevel,
            P,
            (1 << mlActive),
            NULL) == FAILURE)
        {
        DBG(8,fSCHED) DPrint("INFO:     job %s does not meet active fairness policies\n",
          J->Name);

        /* NOTE: de-activate when res/release enabled */

        if (sindex == 0)
          LBFQueue[jindex] = (mjob_t *)1;        
 
        continue;
        }  /* END if (MJobCheckLimits() == FAILURE) */

      if (MJobSelectMNL(
            J,
            P,
            BFNodeList,
            MNodeList,
            NodeMap,
            MBNOTSET) == FAILURE)
        {
        /* resources not available with current schedule in place */

        if (sindex == 0)
          LBFQueue[jindex] = (mjob_t *)1;          

        continue;
        }

      if (MJobAllocMNL(
            J,
            MNodeList,
            NodeMap,
            0,
            P->NAllocPolicy,
            MSched.Time) == FAILURE)
        {
        /* cannot allocate resources with current schedule */
 
        if (sindex == 0)
          LBFQueue[jindex] = (mjob_t *)1;
 
        continue;
        }

      /* add job to list */

      BFList[sindex]  = J;
      BFIndex[sindex] = jindex;

      sindex++;

      SPC             += PC;

      sindex++;   

      MJobGetNL(J,(mnalloc_t *)NodeList);

      __MBFReserveNodes((mnalloc_t *)NodeList);

      DBG(6,fSCHED) DPrint("INFO:     reservation added for Job[%03d] '%s'\n",
        sindex,
        J->Name);

      StartIndex = BFIndex[sindex] + 1;

      DBG(4,fSCHED) DPrint("INFO:     located job '%s' for greedy backfill (size: %d length: %ld)\n",
        J->Name,
        PC,
        J->WCLimit);
      }  /* END for (jindex) */

    /* end of selected jobs list reached.  (current schedule complete) */

    /* determine utility of current schedule */

    BFValue = 0;

    switch(GP->BFMetric)
      {
      case mbfmPS:

        for (index = 0;index < sindex;index++)
          {
          BFValue += 
            MJobGetProcCount(BFList[index]) * BFList[index]->WCLimit;
          }

        break;

      case mbfmWalltime:

        for (index = 0;index < sindex;index++)
          {
          BFValue += BFList[index]->WCLimit;
          }

        break;

      case mbfmProcs:
      default:
 
        for (index = 0;index < sindex;index++)
          {
          BFValue +=
            MJobGetProcCount(BFList[index]);
          }
 
        break;
      }  /* END switch(GP->BFMetric) */

    if (BFValue > BestValue)
      {
      /* copy current list to best list */

      for (index = 0;index < sindex;index++)
        {
        BestList[index] = BFList[index];
        }

      BestList[index] = NULL;
       
      BestValue       = BFValue;

      DBG(1,fSCHED)
        {
        DBG(1,fSCHED) DPrint("INFO:     improved list found by greedy in %d searches (utility: %d  procs available: %d)\n",
          scount,
          BestValue,
          BFProcCount - SPC);

        for (index = 0;BestList[index] != NULL;index++)
          {
          J = BestList[index];

          DBG(1,fSCHED) DPrint("INFO:     %02d:  job '%s' procs: %03d time: %06ld\n",
            index,
            J->Name,
            MJobGetProcCount(J),
            J->WCLimit);
          }
        }

      if ((P->BFMetric == mbfmProcs) && (BFProcCount == SPC))
        {
        /* break out if perfect schedule is found */

        break;
        }
      }     /* END if (BFValue > BestValue) */ 

    /* backtrack if at limit */

    if (sindex == 0)
      {
      /* if schedule is empty */

      break;
      }
    else 
      {
      sindex--;

      J = BFList[sindex];

      SPC -= MJobGetProcCount(J);

      /* release reservation */

      DBG(6,fSCHED) DPrint("INFO:     releasing reservation for Job %d '%s'\n",
        sindex,
        J->Name);

      MJobGetNL(J,(mnalloc_t *)NodeList);

      __MBFReleaseNodes(NSList,(mnalloc_t *)NodeList);
      }    /* END else if (sindex == 0) */

    StartIndex = BFIndex[sindex] + 1;
    }   /* END  while(scount++ < GP->BFMaxSchedules) */

  __MBFRestoreClusterState(NSList);

  if (BestValue == 0)
    {
    /* no schedule found */

    DBG(5,fSCHED) DPrint("INFO:     no jobs found to backfill\n");

    return(SUCCESS);
    }

  DBG(3,fSCHED)
    {
    DBG(1,fSCHED) DPrint("INFO:     final list found by greedy in %d searches (%d of %d procs/utility: %d)\n",
      scount,
      SPC,
      BFProcCount,
      BestValue);

    for (index = 0;BestList[index] != NULL;index++)
      {
      J = BestList[index];

      DBG(1,fSCHED) DPrint("INFO:     S[%02d]  job %16s   procs: %03d time: %06ld\n",
        index,
        J->Name,
        MJobGetProcCount(J),
        J->WCLimit);
      }
    }

  /* launch schedule */

  Failure = FALSE;

  for (index = 0;BestList[index] != NULL;index++)
    {
    J = BestList[index];

    if (MJobCheckLimits(
          J,
          PolicyLevel,
          P,
          (1 << mlActive),
          NULL) == FAILURE)
      {
      DBG(1,fSCHED)
        {
        DBG(1,fSCHED) DPrint("ERROR:    scheduling failure %3d in %s (policy violation/no reservation)  iteration: %4d\n",
          ++ReservationMisses,
          FName,
          MSched.Iteration);
 
        MJobShow(J,0,NULL);
        }
 
      Failure = TRUE;
 
      continue;
      }  /* END if (MJobCheckLimits() == FAILURE) */

    if (MJobSelectMNL(
          J,
          P,
          BFNodeList,
          MNodeList,
          NodeMap,
          MBNOTSET) == FAILURE)
      {
      DBG(1,fSCHED)
        {
        DBG(1,fSCHED) DPrint("ERROR:    cannot get tasks in %s on (ERR: %d/no reservation/iteration %d)\n",
          FName,
          ++ReservationMisses,
          MSched.Iteration);
 
        MJobShow(J,0,NULL);
        }
 
      Failure = TRUE;
     
      continue;
      }

    /* job evaluation successful */

    if (MJobAllocMNL(
          J,
          MNodeList,
          NodeMap,
          0,
          P->NAllocPolicy,
          MSched.Time) == FAILURE)
      {
      DBG(1,fSCHED) DPrint("ERROR:    cannot allocate resources to job %s\n",
        J->Name);
 
      Failure = TRUE;

      continue;
      }  /* END if (MJobAllocMNL() == SUCCESS) */

    if (MJobStart(J) == FAILURE)
      {
      DBG(1,fSCHED) DPrint("ERROR:    cannot start job %s in %s()\n",
        FName,
        J->Name);
 
      Failure = TRUE;

      continue;
      }

    /* job successfully started */

    J->SysFlags |= (1 << mjfBackfill);

    MJobUpdateFlags(J);          

    MStatUpdateBFUsage(J);
    }  /* END for (index) */

  /* if reservations fail, use bestfit as fallback */

  if (Failure == TRUE)
    {
    int          tmpNodes;
    int          tmpProcs;
 
    long         tmpTime;

    nodelist_t   tmpNodeList;          

    mcres_t      DRes;          

    memset(&DRes,0,sizeof(DRes));

    DRes.Procs = 1;

    if (MBFGetWindow(
          &tmpNodes,
          &tmpProcs,
          tmpNodeList,
          &tmpTime,
          0,
          P,
          ALL,
          ALL,
          ALL,
          0,
          0,
          1,
          &DRes,
          NULL,
          NULL,
          NULL,
          NULL) == SUCCESS)
      {
      MBFBestFit(
        BFQueue,
        PolicyLevel,
        tmpNodeList,
        tmpTime,
        tmpNodes,
        tmpProcs,
        P);
      }
    }    /* END if (Failure == TRUE) */

  DBG(2,fSCHED) DPrint("INFO:     partition %s nodes/procs available after %s: %d/%d\n",
    P->Name,
    FName,
    P->IdleNodes,
    P->ARes.Procs);

  return(SUCCESS);
  }  /* END MBFGreedy() */




/* END MBF.c */


