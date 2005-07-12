/* HEADER */
 
/* Contains:                                                              *
 *   int MSimJobStart(J)                                                  *
 *   int MSimRMGetInfo()                                                  *
 *   int MSimGetResources(Directory,TraceFile)                            *
 *   int MSimGetWorkload()                                                *
 *   int MSimJobRequeue(J)                                                *
 *                                                                        */

#include "moab.h"
#include "msched-proto.h"  
 
extern mlog_t      mlog;
 
extern msched_t    MSched;
extern mqos_t      MQOS[];
extern mnode_t    *MNode[];
extern mjob_t     *MJob[];
extern mstat_t     MStat;
extern msim_t      MSim;
extern mrm_t       MRM[];
extern mpar_t      MPar[];
extern mframe_t    MFrame[];
extern mjob_t     *MJobTraceBuffer;
extern msys_t      MSys;             
extern mattrlist_t MAList;
extern int         MAQ[];

extern const char *MPolicyMode[];
extern const char *MBFPolicy[];
extern const char *MBFMPolicy[];
extern const char *MNAllocPolicy[];
extern const char *MSimNCPolicy[];
extern const char *MResPolicy[];
extern const char *MSimQPolicy[];
extern const char *MSimFlagType[];
extern const char *MComType[];
extern const char *MJobFlags[];
extern const char *MAllocLocalityPolicy[];
extern const char *MTimePolicy[];

int __MSimJobCustomize(mjob_t *);
int __MSimNodeUpdate(mnode_t *);
int __MSimJobUpdate(mjob_t *);




int MSimJobStart(

  mjob_t *J) /* I */

  {
  int nindex;
  int MinProcSpeed;

  mreq_t *RQ;

  const char *FName = "MSimJobStart";

  DBG(2,fSIM) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if (J == NULL)
    {
    return(FAILURE);
    }

  /* adjust job info */

  RQ = J->Req[0];
 
  if ((J->NodeList != NULL) && (J->NodeList[0].N == NULL))
    {
    memcpy(
      J->NodeList,
      RQ->NodeList,
      sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1));
    }
 
  J->DispatchTime = MSched.Time;

  J->MTime = MSched.Time;
  J->ATime = MSched.Time;
 
  if (MSim.ScaleJobRunTime == TRUE)
    {
    MinProcSpeed = MAX_INT;
 
    for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
      {
      MinProcSpeed = MIN(MinProcSpeed,RQ->NodeList[nindex].N->ProcSpeed);
      }
 
    if ((MinProcSpeed > 0) && (MinProcSpeed < MAX_INT))
      {
      if (MSched.ReferenceProcSpeed > 0)
        {
        J->SimWCTime = (long)(J->SimWCTime * (double)MSched.ReferenceProcSpeed / MinProcSpeed);
        }
      }
    }  /* END if (MSim.ScaleJobRunTime == TRUE) */

  return(SUCCESS);
  }  /* END MSimJobStart() */




int MSimJobModify(

  mjob_t *J,
  char   *Attr,
  char   *SubAttr,
  char   *Value,
  int    *SC)

  {
  /* NYI */

  return(FAILURE);
  }  /* END MSimJobModify() */



int __MSimNodeUpdate(

  mnode_t *N)  /* I */

  {
  double MaxLoad;

  if (N == NULL)
    {
    return(FAILURE);
    }

  /* adjust available node resources */

  if (N->MaxLoad > 0.01)
    MaxLoad = N->MaxLoad;
  else
    MaxLoad = MSched.DefaultN.MaxLoad;

  if (MaxLoad > 0.01)
    {
    double AvailLoad;

    AvailLoad = MAX(0.0,MaxLoad + MIN_OS_CPU_OVERHEAD - (N->Load + N->ExtLoad));

    if (MPar[0].NodeLoadPolicy == nlpAdjustProcs)
      {
      N->ARes.Procs = AvailLoad;

      MNodeAdjustState(N,&N->State);
      }
    else if ((N->Load >= MaxLoad) &&
            ((N->State == mnsIdle) || (N->State == mnsActive)))
      {
      MNodeSetState(N,mnsBusy,0);
      }
    }
  else
    {
    N->ARes.Procs = MIN(
      N->CRes.Procs - N->DRes.Procs,
      N->CRes.Procs - (N->Load + N->ExtLoad));

    N->ARes.Procs = MAX(0,N->ARes.Procs);
    }

  return(SUCCESS);
  }  /* END __MSimNodeUpdate() */




int __MSimJobUpdate(

  mjob_t *J)  /* I */

  {
  if (J->ASFunc != NULL)
    (*J->ASFunc)(J,mascUpdate,J->ASData,NULL);
 
  J->ATime = MSched.Time;
 
  if (J->Ckpt != NULL)
    {
    if ((J->State == mjsStarting) || (J->State == mjsRunning))
      {
      if (MSched.Time - MAX(J->StartTime,J->Ckpt->LastCPTime) > J->Ckpt->CPInterval)
        {
        MJobCheckpoint(J);
        }
      }
    }

  return(SUCCESS);
  }  /* END __MSimJobUpdate() */





int MSimRMGetInfo()
 
  {
  int      nindex;
  int      jindex;
  
  int      OldNodeState;

  long     CTime;
 
  mjob_t  *J;
  mnode_t *N;
 
  mjob_t  *JNext;

  xres_t  *R;

  const char *FName = "MSimRMGetInfo";
 
  DBG(2,fSIM) DPrint("%s()\n",
    FName);
 
  /* update nodes */
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;
 
    DBG(7,fSIM) DPrint("INFO:     updating node %d '%s'\n",
      nindex,
      N->Name);

    OldNodeState = N->State;

    MRMNodePreUpdate(
      N,
      (N->NewState != mnsNONE) ? N->NewState : N->State,
      N->RM);

    __MSimNodeUpdate(N);

    MRMNodePostUpdate(
      N,
      OldNodeState);
    }  /* END for (nindex) */

  /* update non-node resources */

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    R = &MPar[0].XRes[nindex];

    if (R->Name[0] == '\0')
      break;

    if (R->Name[0] == '\1')
      continue;

    if (R->Func != NULL)
      (*R->Func)(&R->Data,mascUpdate,NULL,NULL);
    }  /* END for (nindex) */
 
  /* update jobs */
 
  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = JNext)
    {
    DBG(7,fSIM) DPrint("INFO:     updating job '%s'\n",
      J->Name);

    JNext = J->Next;  /* store next job pointer in case current job is removed */

    MRMJobPreUpdate(J);          

    __MSimJobUpdate(J);
 
    if (J->State == mjsCompleted)
      {
      MSimJobTerminate(J,mjsCompleted);
      }
    else if (J->State == mjsRemoved)
      {
      MSimJobTerminate(J,mjsRemoved);
      }
    else
      { 
      long tmpL;

      MRMJobPostUpdate(
        J,
        J->TaskMap,
        J->State,
        J->RM);

      tmpL = J->StartTime + J->WCLimit + J->SWallTime - MSched.Time;

      if ((J->Ckpt != NULL) && (J->Ckpt->StoredCPDuration > 0))
        {
        tmpL -= J->Ckpt->StoredCPDuration;
        }

      J->RemainingTime = (tmpL > 0) ? tmpL : 0;

      if ((J->State == mjsStarting) || 
          (J->State == mjsRunning) || 
          (J->State == mjsSuspended))
        {
        MJobAddToNL(J,NULL);
        }
      }
    }    /* END for (J = MJob) */
 
  /* complete jobs */
 
  DBG(7,fSIM) DPrint("INFO:     completing jobs\n");
 
  for (jindex = 0;MAQ[jindex] != -1;jindex++)
    {
    if (MAQ[jindex] == 0)
      {
      DBG(0,fSIM) DPrint("ALERT:    corrupt MAQ table entry at index %d\n",
        jindex);
 
      break;
      }
 
    J = MJob[MAQ[jindex]];
 
    DBG(5,fSIM) DPrint("INFO:     check completion of AQ[%02d] job '%18s':  now: %ld  start: %ld  completion: %ld (deadLine: %ld res: %ld)\n",
      jindex,
      J->Name,
      MSched.Time,
      J->StartTime,
      J->StartTime + J->SimWCTime,
      J->StartTime + J->WCLimit,
      (J->R != NULL) ? J->R->EndTime : -1);
 
    MTRAPJOB(J,FName);

    CTime = J->StartTime + J->SWallTime + J->SimWCTime;

    if ((J->Ckpt != NULL) && (J->Ckpt->StoredCPDuration > 0))
      CTime -= J->Ckpt->StoredCPDuration;
 
    if (CTime <= MSched.Time)
      {
      MSimJobTerminate(J,mjsCompleted);
 
      /* just removed a job, (a new job is in current slot) */ 
 
      jindex--;

      continue;
      }

    /* update active job stats */

    MStatUpdateActiveJobUsage(J);     
    }    /* END for (jindex) */
 
  /* submit jobs */
 
  if (MSched.Iteration > 0)
    {
    MSimGetWorkload();
    }
 
  return(SUCCESS);
  }  /* END MSimRMGetInfo() */





int MSimGetWorkload()
 
  {
  mjob_t  *JTrace[MAX_MJOB_TRACE + 1];
 
  int      jindex;
  int      index;

  long     QueueTime;
  long     NewTime;
  long     TraceTime;


  mjob_t  *J;

  const char *FName = "MSimGetWorkload";

  DBG(3,fSCHED) DPrint("%s()\n",
    FName);
 
  NewTime = MSched.Time;
 
  if (MSched.Iteration == -1)
    {
    if ((MJobTraceBuffer = (mjob_t *)calloc(1,sizeof(mjob_t) * MAX_MJOB_TRACE)) == NULL)
      {
      /* cannot allocate workload trace buffer */
 
      fprintf(stderr,"ERROR:     cannot allocate workload trace buffer, %d (%s)\n",
        errno,
        strerror(errno));
 
      exit(0);
      }
    }
 
  /* submit jobs */
 
  jindex = 0;
 
  memset(JTrace,0,sizeof(JTrace));
 
  switch(MSim.JobSubmissionPolicy)
    {
    case msjsTraceSubmit:
 
      if (MSched.Iteration == -1)
        {
        /* establish initial job queue */
 
        index = 0;
 
        QueueTime = 0;
 
        while(index < (int)MSim.InitialQueueDepth)
          {
          if (MSimJobSubmit(-1,&JTrace[jindex],NULL,jtNONE) == FAILURE)
            {
            DBG(3,fSIM) DPrint("INFO:     job traces exhausted\n");

            break;
            }

          DBG(3,fSIM) DPrint("INFO:     submitted job '%s'\n",
            JTrace[jindex]->Name);
 
          index++;
 
          QueueTime = MAX(QueueTime,JTrace[jindex]->SubmitTime);
 
          jindex++;
 
          if (jindex >= MAX_MJOB_TRACE)
            break;
          }    /* END while (index < (int)MSim.InitialQueueDepth) */

        TraceTime = QueueTime;

        if (MSched.TimePolicy == mtpReal)
          MSched.TimeOffset = MSched.Time - TraceTime;
 
        DBG(5,fSIM) DPrint("INFO:     new trace time detected (%s)\n",
          MULToDString((mulong *)&TraceTime));
 
        DBG(2,fSIM) DPrint("INFO:     initial queue depth %d set (%d requested)\n",
          index,
          MSim.InitialQueueDepth);
        }    /* END if (MSched.Iteration == -1) */
      else
        {
        while (MSimJobSubmit(NewTime,&JTrace[jindex],NULL,jtNONE) == SUCCESS)
          {
          DBG(3,fSIM) DPrint("INFO:     job '%s' submitted\n",
            JTrace[jindex]->Name);
 
          jindex++;
 
          if (jindex >= MAX_MJOB_TRACE)
            break;
          }
        }    /* END else (MSched.Iteration == -1) */
 
      break;
 
    case msjsConstantJob:

      if (MSched.TimePolicy != mtpReal) 
        QueueTime = MSim.StartTime;
      else
        QueueTime = 0;
 
      /* adjust idle job statistics */
 
      for (jindex = 1;jindex < MAX_MJOB;jindex++)
        {
        J = MJob[jindex];

        if (J == NULL)
          break;

        if (J->State != mjsIdle)
          continue;

        MPolicyAdjustUsage(NULL,J,NULL,mlIdle,MPar[0].L.IP,-1,1,NULL);
        }   /* END for (jindex) */

      jindex = 0;
 
      while (MPar[0].L.IP->Usage[mptMaxJob][0] < MSim.InitialQueueDepth)
        {
        if (MSimJobSubmit(
              (MSched.Iteration == -1) ? MSched.Time : -1,
              &JTrace[jindex],
              NULL,
              jtNONE) == FAILURE)
          {
          DBG(3,fSIM) DPrint("INFO:     job traces exhausted\n");
 
          break;
          }

        MPolicyAdjustUsage(NULL,JTrace[jindex],NULL,mlIdle,MPar[0].L.IP,-1,1,NULL);     

        DBG(3,fSIM) DPrint("INFO:     job '%s' submitted\n",
          JTrace[jindex]->Name);
 
        if ((MSim.StartTime <= 0) || (MSched.TimePolicy == mtpReal))
          {
          QueueTime = MAX(QueueTime,JTrace[jindex]->SubmitTime);
          }
 
        jindex++;
 
        if (jindex >= MAX_MJOB_TRACE)
          break;
        }   /* END while (MPar[0].IP.Usage[mptMaxJob]) */
 
      if (MSched.Iteration == -1)
        {
        TraceTime = QueueTime;
        }
 
      break;
 
    case msjsConstantPS:

      if (MSched.TimePolicy != mtpReal)
        QueueTime = MSim.StartTime;
      else
        QueueTime = 0;

      /* adjust idle job statistics */
 
      for (jindex = 1;jindex < MAX_MJOB;jindex++)
        {
        J = MJob[jindex];
 
        if (J == NULL)
          break;
 
        if (J->State != mjsIdle)
          continue;
 
        MPolicyAdjustUsage(NULL,J,NULL,mlIdle,MPar[0].L.IP,-1,1,NULL);
        }   /* END for (jindex) */

      jindex = 0;
 
      while ((unsigned long)MPar[0].L.IP->Usage[mptMaxPS][0] <
             MSim.InitialQueueDepth)
        {
        if (MSimJobSubmit(-1,&JTrace[jindex],NULL,jtNONE) == FAILURE)
          {
          DBG(3,fSIM) DPrint("INFO:     job traces exhausted\n");
 
          break;
          }

        MPolicyAdjustUsage(NULL,JTrace[jindex],NULL,mlIdle,MPar[0].L.IP,-1,1,NULL);   

        DBG(3,fSIM) DPrint("INFO:     job '%s' submitted\n",
          JTrace[jindex]->Name);
 
        if ((MSim.StartTime <= 0) || (MSched.TimePolicy == mtpReal))
          QueueTime = MAX(QueueTime,JTrace[jindex]->SubmitTime);
 
        jindex++;
 
        if (jindex >= MAX_MJOB_TRACE)
          break;
        }    /* END while (MStat.QueuePS < MSim.InitialQueueDepth) */
 
      if (MSched.Iteration == -1)
        {
        TraceTime = QueueTime;
        }
 
      break;
 
    default:
 
      DBG(0,fSIM) DPrint("ERROR:    unexpected JobSubmissionPolicy detected (%d)\n",
        MSim.JobSubmissionPolicy);
 
      break;
    }  /* END switch(MSim.JobSubmissionPolicy) */

  if ((MSched.TimePolicy != mtpReal) && (MSched.Iteration == -1))
    { 
    NewTime = TraceTime;

    /* adjust SubmitTime/SystemQueueTime to NewTime */
 
    for (jindex = 0;jindex < MAX_MJOB_TRACE;jindex++)
      {
      J = JTrace[jindex];

      if (J == NULL)
        break;
    
      J->ATime           = NewTime;

      J->SubmitTime      = NewTime;
      J->SystemQueueTime = NewTime;
      }   /* END for (jindex) */
    }     /* END if (MSched.TimePolicy != mtpReal) */
 
  if (NewTime != MSched.Time)
    {
    DBG(1,fSCHED) DPrint("NOTE:     adjusting scheduler time to job submission time (%s)\n",
      MULToTString(NewTime - MSched.Time));

    MResAdjustTime(NewTime - MSched.Time);
 
    MSched.Time = NewTime;
    }
 
  return(SUCCESS);
  }  /* END MSimGetWorkload() */
 
 
 
 
int MSimInitializeWorkload()

  {
  mjob_t      *J;
  mnodelist_t  MNodeList;      
  char         NodeMap[MAX_MNODE];   

  const char *FName = "MSimInitializeWorkload";

  DBG(1,fSCHED) DPrint("%s()\n",
    FName);

  /* start jobs before reservations are in place */

  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    if ((J->Name[0] == '\0') || 
        (J->Name[0] == '\1') || 
        !strcmp(J->Name,DEFAULT)) 
      {
      continue;
      }

    if (!(J->Flags & (1 << mjfPreload)))
      continue;
      
    /* attempt to start jobs */

    if (MJobSelectMNL(
          J,
          &MPar[0],
          NULL,
          MNodeList,
          NodeMap,
          FALSE) == FAILURE)
      {
      DBG(6,fSCHED) DPrint("INFO:     cannot locate %d tasks for preload job '%s'\n",
        J->Request.TC,
        J->Name);

      continue;
      }  /* END:  if (MJobSelectMNL() == FAILURE) */

    if (MJobAllocMNL(
          J,
          MNodeList,
          NodeMap,
          NULL,
          MPar[J->Req[0]->PtIndex].NAllocPolicy,
          MSched.Time) == FAILURE)
      {
      DBG(1,fSCHED) DPrint("ERROR:    cannot allocate nodes to preload job '%s'\n",
        J->Name);

      continue;
      }

    if (MJobStart(J) == FAILURE)
      {
      DBG(1,fSCHED) DPrint("ERROR:    cannot run preload job '%s'\n",
        J->Name);

      continue;
      }

    DBG(1,fSCHED) DPrint("INFO:     preload job '%s' successfully started\n",
      J->Name);
    }  /* END for (J) */

  return(SUCCESS);
  }  /* END MSimInitializeWorkload() */



 
 
int MSimGetResources(
 
  char *Directory,  /* I */
  char *TraceFile,  /* I */
  char *Buffer)     /* I */
 
  {
  int    count;
  int    index;
  int    cindex;
 
  char  *buf;
  char  *ptr;
 
  char  *head;
  char  *tail;
 
  char  *tok;
 
  char   Name[MAX_MLINE];
 
  int    buflen;
 
  int    NodeCount;
  int    ProcCount;
 
  int    Offset;
 
  mnode_t *N;
  mnode_t  tmpN;
 
  int     Version;

  const char *FName = "MSimGetResources";
 
  DBG(2,fSIM) DPrint("%s(%s,%s,%s)\n",
    FName,
    Directory,
    TraceFile,
    (Buffer != NULL) ? "Buffer" : "NULL");
 
  /* setup default RM */
 
  if (MRM[0].Type == mrmtNONE)
    MRMAdd(DEFAULT,NULL);

  if (MRM[0].Type == 0)
    MRM[0].Type = DEFAULT_MRMTYPE;

  if ((Buffer == NULL) || (Buffer[0] == '\0'))
    { 
    /* load resource tracefile */
 
    if (TraceFile[0] == '\0')
      {
      DBG(0,fSIM) DPrint("ERROR:    no resource tracefile specified\n");
 
      fprintf(stderr,"ERROR:    no resource tracefile specified\n");
 
      exit(1);
      }
 
    if ((TraceFile[0] == '/') || (TraceFile[0] == '~'))
      {
      strcpy(Name,TraceFile); 
      }
    else
      {
      if (Directory[strlen(Directory) - 1] == '/')
        {
        sprintf(Name,"%s%s",
          Directory,
          TraceFile);
        }
      else
        {
        sprintf(Name,"%s/%s",
          Directory,
          TraceFile);
        }
      }
 
    if ((buf = MFULoad(Name,1,macmWrite,&count,NULL)) == NULL)
      {
      DBG(0,fSIM) DPrint("ERROR:    cannot open resource tracefile '%s'\n",
        Name);
 
      fprintf(stderr,"ERROR:    cannot open resource tracefile '%s'\n",
        Name);
 
      exit(1);
      }
    }
  else
    {
    buf = Buffer;
    }
 
  Version = DEFAULT_RESOURCE_TRACE_VERSION;
 
  head = buf;
 
  buflen = strlen(head);
  tail   = head + buflen;
 
  ptr = head;
 
  DBG(6,fSIM) DPrint("INFO:     resource buffer size: %d chars\n",
    buflen);
 
  /* load resource traces */
 
  DBG(3,fSIM) DPrint("INFO:     loading resource trace info\n");
 
  NodeCount = 0;
  ProcCount = 0;
 
  Version = DEFAULT_RESOURCE_TRACE_VERSION;
 
  while (ptr < tail)
    {
    if ((MSim.NodeCount != 0) && (NodeCount >= MSim.NodeCount))
      {
      DBG(1,fSIM) DPrint("INFO:     simulation node count reached.  (node count: %d)\n",
        NodeCount);
 
      break;
      }
 
    if (MTraceLoadResource(ptr,&Offset,&tmpN,&Version) == SUCCESS)
      {
      if (MNodeFind(tmpN.Name,&N) == SUCCESS)
        {
        DBG(1,fSIM) DPrint("ALERT:    node '%s' previously detected in tracefile\n",
	  tmpN.Name);
        }
      else
        {
        for (cindex = 0;cindex < MAX_MCLASS;cindex++)
          {
          /* located configured classes */

          if (tmpN.CRes.PSlot[cindex].count > 0)
            break;
          }
 
        if (cindex == MAX_MCLASS)
          {
          DBG(2,fSIM) DPrint("INFO:     non-batch node '%s' trace loaded.  ignored\n",
            tmpN.Name);
          }
        else
          {
          if (MNodeAdd(tmpN.Name,&N) == SUCCESS)
            {
            MNodeCopy(&tmpN,N);

            MNodeLoadConfig(N,NULL);
 
            /* customize node */
 
            DBG(5,fSIM) DPrint("INFO:     customizing MNode[%03d]: '%s'\n",
              N->Index,
              N->Name);
 
            memcpy(&N->ARes,&N->CRes,sizeof(N->ARes));
 
	    MNodeGetLocation(N);

            /* set default partition (overruled below) */
 
            if (MSim.NCPolicy == msncPartitioned)
              {
              for (index = 6;index < MAX_MPAR;index++)
                {
                if (N->CRes.Mem <= (1 << index))
                  break;
                }
 
              N->PtIndex  = index - 5;
              N->CRes.Mem = 1 << index;
              }
            else
              { 
              if (N->PtIndex <= 0)
                N->PtIndex = 1;
              }
 
            if (N->PtIndex == 0)
              {
              /* no specific partition value located */
 
              N->PtIndex = 1;
              }
 
            DBG(5,(fSIM|fSTRUCT)) DPrint("INFO:     host '%s' assigned to frame: %d node: %d partition: %s\n",
              N->Name,
              N->FrameIndex,
              N->SlotIndex,
              MAList[ePartition][N->PtIndex]);
 
            /* determine partition from feature info */
 
            MUStrCpy(
              Name,
              MUMAList(eFeature,N->FBM,sizeof(N->FBM)),
              sizeof(Name));
 
            if ((tok = strstr(Name,PARTITIONMARKER)) != NULL)
              {
              /* partition marker found */
 
              tok += strlen(PARTITIONMARKER);
              tok++;
 
              N->PtIndex = (int)strtol(tok,NULL,0);
 
              DBG(4,fSIM) DPrint("INFO:     node '%s' added to partition %d\n",
                N->Name,
                N->PtIndex);
              }
 
            N->CTime = MSched.Time;
            N->MTime = MSched.Time;
            N->ATime = MSched.Time;
 
            MRMNodePostLoad(N);
 
            DBG(4,fSIM)
              MNodeShow(N);
 
            NodeCount++;
 
            ProcCount += N->CRes.Procs;
            }
          else
            {
            DBG(2,fSIM) DPrint("ALERT:    node buffer overflow.  %d nodes read.  cannot alloc memory for node '%s'\n",
              NodeCount,
              tmpN.Name);

            break; 
            }  /* END else if (MNodeAdd() == SUCCESS) */
          }    /* END else (cindex == MAX_MCLASS)     */
        }      /* END else if (MNodeFind())           */
      }        /* END if (LoadNodeTrace())            */
 
    ptr += Offset;
 
    if (Offset == 0)
      {
      /* last node trace read */
 
      DBG(4,fSIM) DPrint("INFO:     last node reached");
 
      break;
      }
    }        /* END while (ptr < tail)   */
 
  DBG(3,fSIM) DPrint("INFO:     node traces loaded: %d (%d procs)\n",
    NodeCount,
    ProcCount);
 
  DBG(2,fSIM)
    {
    DBG(2,fSIM) DPrint("INFO:     frame summary:  ");
 
    for (index = 0;index < MAX_MFRAME;index++)
      {
      if (MFrame[index].NodeCount > 0)
        {
        fprintf(mlog.logfp,"[%02d: %2d]",
          index,
          MFrame[index].NodeCount);
        }
      }    /* END for (index) */
 
    fprintf(mlog.logfp,"\n");
    }

  if (Buffer == NULL) 
    {
    /* file buffer read */

    free(buf);
    }
 
  return(SUCCESS);
  }  /* END MSimGetResources() */





int MSimJobSuspend(
 
  mjob_t *J)
 
  {
  int      nindex;
  mnode_t *N;
 
  mreq_t  *RQ;

  const char *FName = "MSimJobSuspend";

  DBG(2,fSIM) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");        
 
  if (J == NULL)
    return(FAILURE);
 
  if ((J->State != mjsRunning) && (J->State != mjsStarting))
    return(FAILURE);
 
  RQ = J->Req[0];
 
  for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
    {
    N = J->NodeList[nindex].N;
 
    /* adjust resource usage */

/* 
    MCResAdd(&N->ARes,&N->CRes,&RQ->DRes,J->NodeList[nindex].taskcount,FALSE);

    if (RQ->SDRes != NULL)
      MCResRemove(&N->ARes,&N->CRes,RQ->SDRes,J->NodeList[nindex].taskcount,TRUE);
  
    MCResRemove(&N->DRes,&N->CRes,&RQ->DRes,J->NodeList[nindex].taskcount,TRUE);

    if (RQ->SDRes != NULL)
      MCResAdd(&N->DRes,&N->CRes,RQ->SDRes,J->NodeList[nindex].taskcount,FALSE);
*/
    }  /* END for (nindex) */

 
  MJobSetState(J,mjsSuspended);
  
  return(SUCCESS);
  }  /* END MSimJobSuspend() */
 
 
 
 
 
int MSimJobResume(
 
  mjob_t *J)  /* I */
 
  {
  int     nindex;
  mnode_t *N;
 
  mreq_t  *RQ;

  const char *FName = "MSimJobResume";
 
  DBG(2,fSIM) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");
 
  if (J == NULL)
    {
    return(FAILURE);
    }
 
  if (J->State != mjsSuspended)
    {
    return(FAILURE);
    }
 
  RQ = J->Req[0];
 
  for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
    {
    N = J->NodeList[nindex].N;
 
    /* adjust resource usage */

    if (RQ->SDRes != NULL) 
      MCResRemove(&N->ARes,&N->CRes,RQ->SDRes,J->NodeList[nindex].TC,TRUE);
    else
      MCResRemove(&N->ARes,&N->CRes,&RQ->DRes,J->NodeList[nindex].TC,TRUE);

    if (RQ->SDRes != NULL) 
      MCResAdd(&N->DRes,&N->CRes,RQ->SDRes,J->NodeList[nindex].TC,TRUE);
    else
      MCResAdd(&N->DRes,&N->CRes,&RQ->DRes,J->NodeList[nindex].TC,TRUE);
    }  /* END for (nindex) */

  MJobSetState(J,mjsRunning); 

  if (MResJCreate(J,NULL,MSched.Time,mjrActiveJob,NULL) == FAILURE)
    {
    DBG(0,fSCHED) DPrint("ERROR:    cannot create reservation for job '%s'\n",
      J->Name);
    }

  DBG(2,fSIM) DPrint("INFO:     job %s successfully resumed on %d procs\n",
    J->Name,
    J->Request.TC);
 
  return(SUCCESS);
  }  /* END MSimJobResume() */




int MSimJobCheckpoint(
 
  mjob_t *J)  /* I */
 
  {
  if ((J == NULL) || (J->Ckpt == NULL))
    {
    return(FAILURE);
    }
 
  if ((J->State != mjsRunning) && (J->State != mjsStarting))
    {
    return(FAILURE);
    }
 
  if (J->Ckpt->LastCPTime == 0)
    J->Ckpt->LastCPTime = J->StartTime;

  J->Ckpt->ActiveCPDuration += MSched.Time - J->Ckpt->LastCPTime;
  J->Ckpt->LastCPTime        = MSched.Time;

  return(SUCCESS);
  }  /* END MSimJobCheckpoint() */
 




int MSimJobRequeue(
 
  mjob_t *J)  /* I */
 
  {
  mreq_t  *RQ;

  const char *FName = "MSimJobRequeue";
 
  DBG(2,fSCHED) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");
 
  RQ = J->Req[0];  /* FIXME */

  if (J == NULL)
    {
    return(FAILURE);
    }

  if ((J->State != mjsStarting) && (J->State != mjsRunning))
    {
    DBG(2,fSCHED) DPrint("WARNING:  attemting to requeue non-active job %s\n",
      J->Name);

    return(FAILURE);
    }

  if (!(J->Flags & (1 << mjfRestartable)))
    {
    /* job is not restartable */

    DBG(2,fSCHED) DPrint("WARNING:  attemting to requeue non-restartable job %s (job will be cancelled)\n",
      J->Name);

    return(MSimJobCancel(J));
    }

  if (!(J->Flags & (1 << mjfPreemptee)))
    {
    /* OUCH: temporary for research */

    DBG(2,fSCHED) DPrint("WARNING:  attemting to requeue non-preemptible job %s\n",
      J->Name);

    return(FAILURE);
    }

  if (J->Ckpt != NULL)
    {
    if ((J->Ckpt->StoredCPDuration == 0) || (J->Ckpt->InitialStartTime == 0))
      J->Ckpt->InitialStartTime = J->StartTime;

    J->Ckpt->StoredCPDuration += J->Ckpt->ActiveCPDuration;
    J->Ckpt->ActiveCPDuration  = 0;
    }

  MSimJobTerminate(J,mjsIdle);
 
  return(SUCCESS);
  }  /* END MSimJobRequeue() */




int MJobLocateResources(

  mjob_t    *J,
  int        TaskCount,
  int        JobList[],
  mnalloc_t  NodeList[])

  {
  long value;

  double tmpD;

  int BestValue = 0;

  /* collect idle resources */
  /* preempt jobs as needed */

  if ((J == NULL) || (NodeList == NULL))
    {
    return(FAILURE);
    }

  /* need node name, taskcount, state, availability duration */
 
  /* first pass, collect idle nodes with adequate availability duration */

  /* JobList contains set of jobs which job J can preempt */

  MJobGetRunPriority(J,TaskCount,&tmpD,NULL);

  /* incorporate TaskCount and NodeList info */

  value = (unsigned long)tmpD;

  /* sort jobs by LowestRunPriority * TasksAvailable / TasksPreempted */

  /* NOTE:  only preempt if started job is of greater value than sum of preempted jobs */
 
  BestValue = MAX(value,BestValue);
     
  return(SUCCESS);
  }  /* END MJobLocateResources() */




int MJobSelectPreemptableJobs(

  mjob_t *PreemptorJ,
  mjob_t *PreempteeJ[])

  {
  int pjindex;

  mjob_t *J;

  if ((PreemptorJ == NULL) || (PreempteeJ == NULL))
    {
    return(FAILURE);
    }

  pjindex = 0;

  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    /* if job is active, is preemptible, and has a lower priority */

    if ((J->State != mjsStarting) && (J->State != mjsRunning))
      continue;

    if (!(J->Flags & (1 << mjfPreemptee)))
      continue;

    if (J->RunPriority >= PreemptorJ->StartPriority)
      continue;

    PreempteeJ[pjindex] = J;

    pjindex++;
    }  /* END for (jindex) */

  PreempteeJ[pjindex] = NULL;

  return(SUCCESS);
  }  /* END MJobSelectPreemptableJobs() */





int MJobGetAdjRunPriority(

  mjob_t    *PreemptorJ,
  mjob_t    *PreempteeJ,
  mnalloc_t  FeasNodeList[],
  int       *TaskCount)

  {
  int    nindex;

  long   RunPriority;

  if ((PreemptorJ == NULL) || 
      (PreempteeJ == NULL) || 
      (FeasNodeList == NULL))
    {
    return(FAILURE);
    }

  /* note:  value is proportional to amount of resources freed by preempting job */

  RunPriority = 0;

  /* NOTE:  run priority -> Idle Priority + UsageWeight * 
     (RUWeight * TotalResourcesUsed) +
     (PRWeight * PercentJobRemaining) +
     (RRWeight * TotalResourcesRemaining)
  */

  for (nindex = 0;FeasNodeList[nindex].N != NULL;nindex++)
    {
    /* determine number of tasks provided by suspending job */

    /* NYI */
    }  /* END for (nindex) */

  /* OUTPUT of upper level routine:  priority/task ordered list of jobs */

  return(SUCCESS);
  }  /* END MJobGetAdjRunPriority() */




int MSimJobTerminate(

  mjob_t *J,       /* I */
  int     JSIndex) /* I */

  {
  mnode_t *N;
  mreq_t  *RQ;

  int      rqindex;
  int      nindex;

  int      TC;

  int      OldState;

  int      MinProcSpeed;

  double   NodeLoad;

  const char *FName = "MSimJobTerminate";

  DBG(2,fSCHED) DPrint("%s(%s,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    MJobState[JSIndex]);

  if (J == NULL)
    {
    return(FAILURE);
    }

  OldState  = J->State;

  J->State  = JSIndex;
  J->EState = JSIndex;

  if (J->StartTime < J->SubmitTime)
    {
    DBG(1,fSIM) DPrint("ALERT:    simulation statistics corrupted (negative queue time: %ld)\n",
      (long)J->StartTime - J->SubmitTime);
    }
 
  /* release nodes */
 
  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];

    MinProcSpeed = MAX_INT;
 
    for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
      {
      MinProcSpeed = MIN(MinProcSpeed,RQ->NodeList[nindex].N->ProcSpeed);
      }  /* END for (nindex) */
 
    for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
      {
      if (nindex == MAX_MNODE)
        {
        DBG(0,fSCHED) DPrint("ERROR:    job node buffer overflow in job %s (iteration: %d)\n",
          J->Name,
          MSched.Iteration);
 
        break;
        }

      N = RQ->NodeList[nindex].N;

      /* adjust available/dedicated resources */     
 
      if ((RQ->NAccessPolicy == mnacSingleJob) ||
          (RQ->NAccessPolicy == mnacSingleTask))
        {
        MCResRemove(&N->DRes,&N->CRes,&N->CRes,1,TRUE);

        MCResAdd(&N->ARes,&N->CRes,&N->CRes,1,TRUE);

        NodeLoad = (double)N->CRes.Procs;
        }
      else
        {
        TC = RQ->NodeList[nindex].TC;

        MCResRemove(&N->DRes,&N->CRes,&RQ->DRes,TC,TRUE);         

        MCResAdd(&N->ARes,&N->CRes,&RQ->DRes,TC,TRUE);             

        NodeLoad = (double)TC * RQ->DRes.Procs;
        }

      N->TaskCount -= RQ->NodeList[nindex].TC;

      MNodeAdjustState(N,&N->State);
 
      if (MinProcSpeed > 0)
        {
        /* scale load to account for parallel job synchronization delays */
 
        NodeLoad *= MinProcSpeed / MAX(MinProcSpeed,N->ProcSpeed);
        }
 
      N->Load -= NodeLoad; 
 
      N->MTime = MSched.Time;
      }  /* END for (nindex) */
    }    /* END for (rqindex) */
 
  DBG(3,fSIM) DPrint("INFO:     job %s terminated in state %s.  %3d nodes freed\n",
    J->Name,
    MJobState[J->State],
    J->NodeCount);

  switch(J->State)
    {
    case mjsCompleted:

      J->CompletionTime = MSched.Time;
      J->CompletionCode = 0;

      MJobProcessCompleted(J);

      break;

    case mjsIdle:

      /* NO-OP */

      break;

    default:

      J->CompletionTime = MSched.Time;
      J->CompletionCode = 0;

      if ((OldState == mjsStarting) || (OldState == mjsRunning))
        {
        MJobProcessCompleted(J);
        }
      else
        {
        MJobProcessRemoved(J);
        }

      break;
    }  /* END switch(J->State) */

  if (J->ASFunc != NULL)
    (*J->ASFunc)(J,mascDestroy,NULL,NULL);

  return(SUCCESS);
  }  /* END MSimJobTerminate() */




int MSimJobCancel(

  mjob_t *J)  /* I */

  {
  const char *FName = "MSimJobCancel";

  DBG(3,fSIM) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if (J == NULL)
    {
    return(FAILURE);
    }

  MJobSetState(J,mjsCompleted);
 
  MQueueRemoveAJob(J,stForce);
 
  DBG(3,fSIM) DPrint("INFO:     job '%s' cancelled\n",
    J->Name);

  return(SUCCESS);
  }  /* END MSimJobCancel() */




int MSimJobSubmit(
 
  long     Now,       /* I */
  mjob_t **JPtr,      /* O */
  void    *JobSource, /* I */
  int      JobType)   /* I */
 
  {
  mjob_t *tmpJ;
  mjob_t  tjob;

  mjob_t *J;
 
  static int JobTraceIndex = 0;
  static int JobTraceCount = 0;
 
  static int NoTraces = FALSE;
 
  int        Version;
 
  short      TaskList[MAX_MNODE];

  const char *FName = "MSimJobSubmit";
 
  DBG(4,fSIM) DPrint("%s(%ld,J,JobSource,%d)\n",
    FName,
    Now,
    JobType);

  if (JPtr != NULL)
    *JPtr = NULL;
 
  switch (JobType)
    {
    case jtTrace:
 
      /* load specified trace */
 
      memset(&tjob,0,sizeof(tjob));
 
      Version = DEFAULT_WORKLOAD_TRACE_VERSION;
 
      if (MTraceLoadWorkload(
            (char *)JobSource,
            NULL,
            &tjob,
            MSched.Mode,
            &Version) == FAILURE)
        {
        DBG(4,fSIM) DPrint("INFO:     cannot load job trace\n");
 
        return(FAILURE);
        }

      if (MJobFind(tjob.Name,&tmpJ,0) == SUCCESS)
        {
        DBG(4,fSIM) DPrint("INFO:     existing job %s resubmitted\n",
          tjob.Name);
 
        return(FAILURE);
        }
 
      tmpJ = &tjob;
 
      break;
 
    case jtWiki:
 
      if (MJobFind(((mjob_t *)JobSource)->Name,&tmpJ,0) == SUCCESS)
        {
        DBG(4,fSIM) DPrint("INFO:     existing job %s resubmitted\n",
          ((mjob_t *)JobSource)->Name);
 
        return(FAILURE);
        }
 
      tmpJ = (mjob_t *)JobSource;

      break;
 
    case jtNONE:
    default:
 
      /* load job from workload cache */
 
      if (NoTraces == TRUE)
        {
        /* job queue exhausted */
 
        return(FAILURE);
        }
 
      if (JobTraceIndex >= JobTraceCount)
        {
        if (MSimLoadWorkloadCache(
              MSched.HomeDir,
              MSim.WorkloadTraceFile,
              &JobTraceCount) == FAILURE)
          {
          /* job queue exhausted */
 
          NoTraces = TRUE;
 
          return(FAILURE);
          }
 
        JobTraceIndex = 0;
 
        DBG(4,fSIM) DPrint("INFO:     refreshed job cache with %d job traces\n",
          JobTraceCount);
        }
 
      tmpJ = &MJobTraceBuffer[JobTraceIndex];
 
      if ((Now > 0) && ((tmpJ->SubmitTime + MSched.TimeOffset) > Now))
        {
        /* job release time not reached */
 
        return(FAILURE);
        }
 
      break;
    }   /* END switch (JobType) */

  if (tmpJ->SimWCTime <= 0)
    {
    /* set sim execution time if not specified */

    tmpJ->SimWCTime = tmpJ->SpecWCLimit[0];
    }

  if (tmpJ->Name[0] == '\0')
    {
    /* set job name if not specified */

    MSimJobCreateName(tmpJ->Name,&MRM[0]);
    }

  if ((JobType == jtNONE) && (JobTraceIndex >= JobTraceCount))
    {
    /* job cache is exhausted */

    NoTraces = TRUE;

    return(FAILURE);
    }

  /* process new sim job */
 
  MJobGetName(tmpJ,NULL,&MRM[0],tmpJ->Name,sizeof(tmpJ->Name),mjnShortName);

  MTRAPJOB(tmpJ,FName);

  if (MJobCreate(tmpJ->Name,TRUE,&J) == FAILURE)
    {
    /* cannot create new job */

    return(FAILURE);
    }

  /* migrate new job into local structure */

  if (JPtr != NULL)
    *JPtr = J;

  MJobMove(tmpJ,J);

  TaskList[0] = -1;

  /*
    NOTE:  what should be done here?

    if (JobType != jtNONE)
      free(tmpJ);
  */

  J->ATime = MSched.Time;
 
  if (MRMJobPostLoad(J,TaskList,J->RM) == FAILURE)
    { 
    /* job was removed */

    /* NO-OP */
    }

  if ((Now <= 0) || 
     ((MSched.TimePolicy == mtpReal) && (MSim.JobSubmissionPolicy != msjsTraceSubmit)))
    {
    /* adjust job queue time */

    J->SubmitTime = MSched.Time;
    }

  J->SystemQueueTime = J->SubmitTime;

  /* adjust job record */

  MJobSetState(J,mjsIdle);
 
  J->CTime = J->SubmitTime;
  J->MTime = J->SubmitTime;
  J->ATime = J->SubmitTime;

  if (J->Ckpt == NULL)
    {
    if (MSim.DefaultCheckpointInterval > 0)
      {
      MJobCPCreate(J,&J->Ckpt);
      }
    }
 
  DBG(5,fSIM) DPrint("INFO:     job '%s' submitted\n",
    J->Name);

  DBG(3,fSIM)
    MJobShow(J,0,NULL);

  JobTraceIndex++;
 
  MSim.QueueChanged = TRUE;
 
  return(SUCCESS);
  }  /* END MSimJobSubmit() */




int MSimJobCreateName(

  char  *JName,
  mrm_t *R)

  {
  if ((JName == NULL) || (R == NULL))
    return(FAILURE);

  switch(R->Type)
    {
    case mrmtLL:

      /* FORMAT:  <HOSTID>.<JOBID>.<STEPID> */

      sprintf(JName,"%s.%d.0",
        R->Name,
        R->JobCounter);

      R->JobCounter++;

      break;

    case mrmtPBS:
    default:

      /* FORMAT:  <JOBID>.<SERVER> */

      sprintf(JName,"%d.%s",
        R->JobCounter,
        R->Name);

      R->JobCounter++;

      break;
    }  /* END switch(R->Type) */

  return(SUCCESS);
  }  /* END MSimJobCreateName() */





int MSimLoadWorkloadCache(
 
  char *Directory,
  char *TraceFile,
  int  *TraceCount)
 
  {
  int    JobCount;
  long   EarliestQueueTime;
 
  int    count;
  int    index;
  int    jindex;
 
  int    Found;
 
  char   Name[MAX_MLINE];
 
  char  *buf;
  char  *ptr;
 
  char  *head;
  char  *tail;
 
  int    buflen;
 
  mjob_t *J;
  mjob_t  tmpJ;
 
  int    Version;
 
  int    Offset;

  const char *FName = "MSimLoadWorkloadCache";
 
  DBG(3,fSIM) DPrint("%s(%s,%s,TraceCount)\n",
    FName,
    Directory,
    TraceFile);
 
  if (TraceFile[0] == '\0')
    {
    DBG(0,fSIM) DPrint("ERROR:    no tracefile specified\n");
 
    exit(1);
    }
 
  if ((TraceFile[0] == '/') || (TraceFile[0] == '~'))
    {
    strcpy(Name,TraceFile);
    }
  else
    {
    if (Directory[strlen(Directory) - 1] == '/')
      {
      sprintf(Name,"%s%s",
        Directory,
        TraceFile);
      }
    else
      {
      sprintf(Name,"%s/%s",
        Directory,
        TraceFile); 
      }
    }
 
  if ((buf = MFULoad(Name,1,macmWrite,&count,NULL)) == NULL)
    {
    DBG(0,fSIM) DPrint("ERROR:    cannot open tracefile '%s'\n",
      Name);
 
    exit(1);
    }
 
  Version = DEFAULT_WORKLOAD_TRACE_VERSION;
 
  /* set head to first line after marker */
 
  head = buf + MSim.TraceOffset;
 
  DBG(3,fSIM) DPrint("INFO:     reading workload tracefile at %d byte offset\n",
    MSim.TraceOffset);
 
  /* terminate string at end delimiter */
 
  buflen = strlen(head);
 
  ptr  = head;
  tail = head + buflen;
 
  DBG(6,fSIM) DPrint("INFO:     workload buffer size: %d bytes\n",
    buflen);
 
  /* clear workload cache */
 
  for (jindex = 0;jindex < MAX_MJOB_TRACE;jindex++)
    {
    J = &MJobTraceBuffer[jindex];
 
    if (J->Name[0] == '\0')
      continue;
 
    DBG(3,fSIM) DPrint("INFO:    freeing memory for trace record %d '%s'\n", 
      jindex,
      (J->Name[0] != '\0') ? J->Name : NONE);
 
    MJobDestroy(&J);
    }  /* END for (jindex) */
 
  memset(MJobTraceBuffer,0,sizeof(MJobTraceBuffer[0]) * MAX_MJOB_TRACE);
 
  /* load job traces */
 
  DBG(3,fSIM) DPrint("INFO:     loading job traces\n");
 
  Offset = MSim.TraceOffset;
 
  JobCount = 0;
 
  EarliestQueueTime = MAX_MTIME;
 
  while (ptr < tail)
    {
    if (MTraceLoadWorkload(
          ptr,
          &Offset,
          &tmpJ,
          MSched.Mode,
          &Version) == SUCCESS)
      {
      Found = FALSE;
 
      /* search job trace cache */
 
      for (index = 0;index < JobCount;index++)
        {
        if (!strcmp(tmpJ.Name,MJobTraceBuffer[index].Name))
          {
          Found = TRUE;
 
          break;
          }
        }
 
      if ((Found == TRUE) || (MJobFind(tmpJ.Name,&J,0) == SUCCESS))
        {
        if (Found == TRUE)
          { 
          DBG(1,fSIM) DPrint("WARNING:  job '%s' previously detected in tracefile (MJobTraceBuffer[%d]/JC: %d  IT: %d)\n",
            tmpJ.Name,
            index,
            JobCount,
            MSched.Iteration);
          }
        else
          {
          DBG(1,fSIM) DPrint("WARNING:  job '%s' previously detected in tracefile (Job[%d]/JC: %d  IT: %d)\n",
            tmpJ.Name,
            J->Index,
            JobCount,
            MSched.Iteration);
          }
        }
      else
        {
        J = &MJobTraceBuffer[JobCount];
 
        MJobMove(&tmpJ,J);
 
        __MSimJobCustomize(J);
 
        /* determine earliest job queue time */
 
        EarliestQueueTime = MIN(J->SubmitTime,EarliestQueueTime);
 
        JobCount++;
 
        if (JobCount >= MAX_MJOB_TRACE)
          {
          DBG(1,fSIM) DPrint("INFO:     job cache is full\n");
 
          break;
          }
        }    /* END else if (MJobFind(tmpJ->Name,&J,0) == SUCCESS) */
      }      /* END if (MTraceLoadWorkload() == SUCCESS) */
 
    if (Offset == 0)
      {
      /* corruption detected or buffer empty */
 
      break;
      }
 
    ptr += Offset;
    }        /* END while (ptr < tail) */
 
  if (Offset != 0)
    {
    MSim.TraceOffset += (ptr - head);
    } 
 
  DBG(3,fSIM) DPrint("INFO:     jobs loaded: %d  earliest start: %ld  %s",
    JobCount,
    EarliestQueueTime,
    MULToDString((mulong *)&EarliestQueueTime));
 
  if (TraceCount != NULL)
    *TraceCount = JobCount;
 
  free(buf);
 
  return(SUCCESS);
  }  /* END MSimLoadWorkloadCache() */




int __MSimJobCustomize(

  mjob_t *J)

  {
  mreq_t *RQ;

  /* customize job */
 
  DBG(5,fSIM) DPrint("INFO:     customizing job '%s'\n",
    (J != NULL) ? J->Name : "NULL");

  if (J == NULL)
    return(FAILURE);

  RQ = J->Req[0];  /* NOTE:  only supports single req jobs */
 
  if (MSim.RandomizeJobSize == TRUE)
    {
    int MaxJobSize;

    MaxJobSize = (MPar[1].L.JP->HLimit[mptMaxProc][0] > 0) ?
      MPar[1].L.JP->HLimit[mptMaxProc][0] :
      MPar[0].L.JP->HLimit[mptMaxProc][0];
 
    J->Request.TC = (rand() % MaxJobSize) + 1;
 
    DBG(6,fSIM) DPrint("INFO:     job '%s' tasks adjusted to %d\n",
      J->Name,
      J->Request.TC);
    }
 
  if (MSim.NCPolicy == msncPartitioned)
    {
    int index;

    for (index = 6;index < MAX_MPAR;index++)
      {
      if (RQ->RequiredMemory <= (1 << index))
        break;
      }
 
    RQ->MemCmp = mcmpEQ;
    RQ->RequiredMemory = 1 << index;
    }
 
  /* determine earliest job queue time */

  J->SystemQueueTime = J->SubmitTime;
 
  if (J->SMinTime > 0)
    J->SMinTime = J->SubmitTime;
 
  /* force jobs within WC bounds */

  /* allow sim jobs to violate WC limits */
/*
  if (J->SimWCTime > J->SpecWCLimit[0])
    {
    DBG(4,fSIM) DPrint("INFO:     scaling WC time for job '%s' to fit within WCLimit (%6ld > %6ld)\n",
      J->Name,
      J->WallClockTime,
      J->SpecWCLimit[0]);
 
    J->WallClockTime = J->SpecWCLimit[0];
    }
*/
 
  if (MSim.WCAccuracy != 0.0)
    {
    double AD;

    double Span;

    double Offset;
 
    /* adjust utilized job walltime for average of WCAccuracy */
 
    /* add job duration 'jitter' (bound variability by 2 * DELTA) */

    /* constrain span to 10% */
 
    AD = MAX(0.0,MIN(1.0,MSim.WCAccuracy));

    Span = MIN(AD,1.0 - AD);
    Span = MIN(Span,0.05);
 
    /* adjust utilized job walltime for average of WCAccuracy */

    srand(getpid());

    Offset = (2.0 * rand() / (RAND_MAX + 1.0)) - 1.0;
 
    J->SimWCTime = J->SpecWCLimit[0] * (AD + Span * Offset);
 
    DBG(3,fSIM) DPrint("INFO:     WC Limit accuracy adjusted to %.2lf percent on job %s\n",
      MSim.WCAccuracy,
      J->Name);
    }
  else if (MSim.WCAccuracyChange != 0)
    {
    double tmpTime;

    tmpTime = (double)(J->SpecWCLimit[0] - J->SimWCTime);
 
    J->SpecWCLimit[0] -= (int)((MSim.WCAccuracyChange * tmpTime) / 100);
 
    DBG(8,fSIM) DPrint("INFO:     WC Limit scaled by %.2lf percent to %6ld on job '%s' (acc: %6.2f)\n",
      MSim.WCAccuracyChange * 100.0,
      J->SpecWCLimit[0],
      J->Name,
      (double)J->SimWCTime / J->WCLimit * 100.0);
    }
 
  /* scale job WC limit/runtime by WCScalingPercent */
 
  if ((MSim.WCScalingPercent != 0) && (MSim.WCScalingPercent != 100))
    {
    double tmpTime;

    tmpTime = (double)J->SpecWCLimit[0] * MSim.WCScalingPercent / 100;
 
    J->SpecWCLimit[0] = (long)tmpTime;
 
    tmpTime = (double)J->SimWCTime * MSim.WCScalingPercent / 100;
 
    J->SimWCTime = (long)tmpTime;
 
    DBG(6,fSIM) DPrint("INFO:     job '%s' runtime scaled by %.2lf percent\n",
      J->Name,
      (double)MSim.WCScalingPercent / 100.0);
    }
 
  /* set job mode */
 
  if (MSched.AllocLocalityPolicy != 0)
    {
    J->Flags |= (1 << mjfAllocLocal);
    }

  return(SUCCESS);
  }  /* END __MSimJobCustomize() */





int MSimSummarize()
 
  {
  long   SchedRunTime;
  int    IdleQueueSize;
  int    EligibleJobs;
  int    RunJobs;
  int    UpNodes;
  int    IdleNodes;
  int    TotalJobsCompleted;
  int    SuccessfulJobsCompleted;
  double TotalProcHours;
  double RunningProcHours;
  double SuccessfulProcHours;
  unsigned long QueuePS;
  double WeightedCpuAccuracy;
  double CpuAccuracy;
  double XFactor;
  int    Iteration;
  long   RMPollInterval; 
  double SchedEfficiency;
  int    BusyNodes;
  double MinEff;
  int    MinEffIteration;
 
  double ProcEfficiency;
 
  mpar_t *GP;

  char   tmpBuffer[MAX_MBUFFER];

  const char *FName = "MSimSummarize";
 
  DBG(1,fSIM) DPrint("%s()\n",
    FName);

  GP = &MPar[0];
 
  DBG(1,fSIM) DPrint("INFO:     efficiency:  %3d / %3d  (%.2f percent) [total: %.2f]  jobs: %3d of %3d [total: %d]\n",
    (GP->UpNodes - GP->IdleNodes),
    GP->UpNodes,
    (double)(GP->UpNodes - GP->IdleNodes) / GP->UpNodes * 100.0,
    MStat.TotalPHBusy / MStat.TotalPHAvailable * 100.0,
    MStat.ActiveJobs,
    MStat.EligibleJobs,
    GP->S.Count);
 
  SchedRunTime            = MSched.Iteration * MSched.RMPollInterval * 100;
  IdleQueueSize           = GP->L.IP->Usage[mptMaxJob][0];
  EligibleJobs            = MStat.EligibleJobs;
  RunJobs                 = MStat.ActiveJobs;
  UpNodes                 = GP->UpNodes;
  IdleNodes               = GP->IdleNodes;
  TotalJobsCompleted      = GP->S.Count;
  SuccessfulJobsCompleted = MStat.SuccessfulJobsCompleted;
  TotalProcHours          = MStat.TotalPHAvailable;
  RunningProcHours        = MStat.TotalPHBusy;
  SuccessfulProcHours     = MStat.SuccessfulPH;
  QueuePS                 = GP->L.IP->Usage[mptMaxPS][0];
  WeightedCpuAccuracy     = GP->S.NJobAcc;
  CpuAccuracy             = GP->S.JobAcc;
  XFactor                 = GP->S.XFactor;
  Iteration               = MSched.Iteration;
  RMPollInterval          = MSched.RMPollInterval; 
  MinEff                  = MStat.MinEff;
  MinEffIteration         = MStat.MinEffIteration;
 
  /* show statistics */
 
  BusyNodes = UpNodes - IdleNodes;
 
  fprintf(mlog.logfp,"\n\n");
 
  DBG(0,fSIM) DPrint("SUM:  Statistics for iteration %d: (Poll Interval: %ld) %s\n",
    Iteration,
    RMPollInterval,
    MULToDString(&MSched.Time));
 
  DBG(0,fSIM) DPrint("SUM:  scheduler running for %7ld seconds (%6.2lf hours) initialized on %s\n",
    SchedRunTime / 100,
    (double)SchedRunTime / 360000,
    MULToDString((mulong *)&MStat.InitTime));
 
  DBG(0,fSIM) DPrint("SUM:  active jobs:          %8d  eligible jobs:       %8d  idle jobs:      %5d\n",
    RunJobs,
    EligibleJobs,
    IdleQueueSize);
 
  if (TotalJobsCompleted != 0)
    {
    DBG(0,fSIM) DPrint("SUM:  completed jobs:       %8d  successful jobs:     %8d  avg XFactor:    %7.2f\n",
      TotalJobsCompleted,
      SuccessfulJobsCompleted,
      XFactor / TotalJobsCompleted);
    }
  else
    {
    DBG(0,fSIM) DPrint("SUM:  completed jobs:           NONE\n");
    }
 
  if (TotalProcHours != 0.0) 
    {
    SchedEfficiency = RunningProcHours / TotalProcHours;
 
    DBG(0,fSIM) DPrint("SUM:  available proc hours:%9.2f  running proc hours: %9.2f  efficiency:     %7.3f\n",
      TotalProcHours,
      RunningProcHours,
      SchedEfficiency * 100);

    DBG(0,fSIM) DPrint("SUM:  preempt proc hours:  %9.2f  preempt jobs:      %d         preempt loss:   %7.3f\n",
      (double)MStat.TotalPHPreempted / 3600.0,
      MStat.TotalPreemptJobs,
      (double)MStat.TotalPHPreempted / 3600.0 / TotalProcHours * 100);

    DBG(0,fSIM) DPrint("SUM:        min efficiency: %8.2f  iteration:           %8d\n",
      MinEff,
      MinEffIteration);
    }
  else
    {
    SchedEfficiency = 1.0;
 
    DBG(0,fSIM) DPrint("SUM:  available proc hours:     <N/A>\n");
    }
 
  DBG(0,fSIM) DPrint("SUM:  available nodes:      %8d  busy nodes:          %8d  efficiency:     %7.3f\n",
    UpNodes,
    BusyNodes,
    (double)BusyNodes / UpNodes * 100.0);
 
  if (SuccessfulProcHours != 0.0)
    {
    DBG(0,fSIM) DPrint("SUM:  wallclock accuracy:    %7.3f  weighted wc accuracy: %7.3f\n",
      (double)CpuAccuracy / TotalJobsCompleted * 100.0,
      (double)WeightedCpuAccuracy / SuccessfulProcHours * 100.0 / 36.0);
    }
  else
    {
    DBG(0,fSIM) DPrint("SUM:  WC limit accuracy:       <N/A>\n");
    }
 
  if (TotalProcHours != 0.0)
    {
    DBG(0,fSIM) DPrint("SUM:  est backlog hours:     %7.3lf  avg backlog:         %8.3lf\n",
      (double)SchedEfficiency * (CpuAccuracy / TotalJobsCompleted) * 
      QueuePS / 3600.0 / UpNodes,
      (double)SchedEfficiency * (CpuAccuracy / TotalJobsCompleted) *
      MStat.AvgQueuePH / MSched.Iteration / UpNodes);
    }
  else
    {
    DBG(0,fSIM) DPrint("SUM:  estimated backlog: <N/A>\n");
    }

  tmpBuffer[0] = '\0';

  MSchedStatToString(&MSched,mwpNONE,tmpBuffer,sizeof(tmpBuffer));

  DBG(0,fSIM) DPrint("SUM:  %s\n",
    tmpBuffer);
 
  /* show configuration summary */
 
  DBG(0,fSIM) DPrint("SUM:  scheduler configuration:\n");
 
  DBG(0,fSIM) DPrint("SUM:  backfill policy:      %8s (%d)  BFDepth:            %3d\n",
    MBFPolicy[GP->BFPolicy],
    MStat.JobsEvaluated,
    GP->BFDepth);
 
  DBG(0,fSIM) DPrint("SUM:  allocation:    %15s  wc accuracy:            %5.2lf  wc accuracy change:      %5.2lf\n",
    MNAllocPolicy[GP->NAllocPolicy],
    MSim.WCAccuracy,
    MSim.WCAccuracyChange);
 
  DBG(0,fSIM) DPrint("SUM:  nodes:                %8d  nodeconfig:          %8s  reservation policy:  %14s\n",
    GP->UpNodes,
    MSimNCPolicy[MSim.NCPolicy],
    MResPolicy[GP->ResPolicy]);
 
  DBG(0,fSIM) DPrint("SUM:  queuedepth:          %9d  submitpolicy: %14s\n",
    MSim.InitialQueueDepth,
    MSimQPolicy[MSim.JobSubmissionPolicy]); 
 
  if ((MSched.Mode == msmSim) && (MSim.ComRate != 0.0))
    {
    ProcEfficiency = RunningProcHours * 36.0 / (RunningProcHours * 36.0 + MStat.TotalSimComCost);
 
    DBG(0,fSIM) DPrint("SUM:  processing hours:    %9.2f  comm hours:        %10.2f  proc eff: %7.2f  total efficiency:  %7.2f\n",
      RunningProcHours,
      MStat.TotalSimComCost / 3600,
      ProcEfficiency * 100.0,
      SchedEfficiency * ProcEfficiency * 100.0);
    }
 
  exit(0);

  /*NOTREACHED*/
 
  return(SUCCESS);
  }  /* END MSimSummarize() */




int MSimSetDefaults()

  {
  const char *FName = "MSimSetDefaults";

  DBG(5,fSIM) DPrint("%s()\n",
    FName);

  memset(&MSim,0,sizeof(MSim));  

  /* set default sim values */
 
  MSim.WCScalingPercent     = DEFAULT_MSIMWCSCALINGPERCENT;
  MSim.WCAccuracy           = DEFAULT_MSIMWCACCURACY;
 
  MSim.JobSubmissionPolicy  = DEFAULT_MSIMJSPOLICY;
  MSim.InitialQueueDepth    = DEFAULT_MSIMINITIALQUEUEDEPTH;
 
  MSim.NCPolicy             = DEFAULT_MSIMNCPOLICY;
  MSim.NodeCount            = DEFAULT_MSIMNODECOUNT;
 
  MSim.RandomizeJobSize     = DEFAULT_MSIMRANDOMIZEJOBSIZE;
 
  MSim.Flags                = DEFAULT_MSIMFLAGS;
  MSim.TraceIgnoreJobFlags  = DEFAULT_MSIMTRACEIGNFLAGS;
 
  MSim.StopIteration        = -1;
  MSim.ExitIteration        = -1;
 
  MSim.CommunicationType    = DEFAULT_MSIMCOMMUNICATIONTYPE;
  MSim.IntraFrameCost       = DEFAULT_MSIMINTRAFRAMECOST;
  MSim.InterFrameCost       = DEFAULT_MSIMINTERFRAMECOST;
  MSim.ComRate              = DEFAULT_MSIMCOMRATE;
 
  MSim.AutoShutdown         = DEFAULT_MSIMAUTOSHUTDOWNMODE;
 
  strcpy(MSim.StatFileName,DEFAULT_MSIMSTATFILENAME);
  strcpy(MSim.WorkloadTraceFile,DEFAULT_MSIMWORKLOADTRACEFILE);
  strcpy(MSim.ResourceTraceFile,DEFAULT_MSIMRESOURCETRACEFILE);

  return(SUCCESS);
  }  /* END MSimSetDefaults() */





int MSimInitialize()

  {
  const char *FName = "MSimInitialize";

  mpar_t *GP = &MPar[0];

  DBG(6,fSIM) DPrint("%s()\n",
    FName);

  MSimGetResources(MSched.HomeDir,MSim.ResourceTraceFile,NULL);
 
  MSimGetWorkload();
 
  MSimInitializeWorkload();
 
  MSched.StartTime = MIN(MSched.Time,MSched.StartTime);
  MStat.InitTime   = MIN(MSched.Time,MStat.InitTime);
 
  DBG(1,fSIM) DPrint("INFO:     starting simulation at %ld\n",
    MSched.Time);
 
  DBG(1,fSIM) DPrint("INFO:     Queue Depth:  %8d  Submit Policy:   %15s\n",
    MSim.InitialQueueDepth,
    MSimQPolicy[MSim.JobSubmissionPolicy]);
 
  DBG(1,fSIM) DPrint("INFO:     BF Policy: %5s  Allocation:  %20s  Reservation Policy:  %15s\n",
    MBFPolicy[GP->BFPolicy],
    MNAllocPolicy[GP->NAllocPolicy], 
    MResPolicy[GP->ResPolicy]);
 
  if (GP->BFPolicy != 0)
    {
    DBG(1,fSIM) DPrint("INFO:     BFDepth:                 %8d  BFMetric: %15s\n",
      GP->BFDepth,
      MBFMPolicy[GP->BFMetric]);
    }
 
  DBG(1,fSIM) DPrint("INFO:     Nodes (UP):   %8d  Nodes (Idle):            %8d\n",
    GP->UpNodes,
    GP->IdleNodes);
 
  DBG(1,fSIM) DPrint("INFO:     Procs (UP):   %8d  Procs (Idle):            %8d\n",
    GP->URes.Procs,
    GP->ARes.Procs);
 
  DBG(1,fSIM) DPrint("INFO:     ScaleFactor:  %8d  WC Accuracy:             %8.2lf  WC Accuracy Change:  %15.2lf\n",
    MSim.WCScalingPercent,
    MSim.WCAccuracy,
    MSim.WCAccuracyChange);

  return(SUCCESS);
  }  /* END MSimInitialize() */




int MSimShow(
 
  msim_t *S,
  char   *Buffer,
  int     DMode)
 
  {
  int index;
 
  char tmpLine[MAX_MLINE];
 
  sprintf(Buffer,"%s%-30s  %s\n",Buffer,MParam[pWorkloadTraceFile],S->WorkloadTraceFile);
  sprintf(Buffer,"%s%-30s  %s\n",Buffer,MParam[pResourceTraceFile],S->ResourceTraceFile);
  sprintf(Buffer,"%s%-30s  %s\n",Buffer,MParam[pSimAutoShutdown],MPolicyMode[S->AutoShutdown]);
  sprintf(Buffer,"%s%-30s  %ld\n",Buffer,MParam[pSimStartTime],S->StartTime);
  sprintf(Buffer,"%s%-30s  %s\n",Buffer,MParam[pSimScaleJobRunTime],(S->ScaleJobRunTime == TRUE) ?
    "TRUE" : "FALSE");
 
  tmpLine[0] = '\0';
 
  for (index = 0;MSimFlagType[index] != NULL;index++)
    {
    if (S->Flags & (1 << index))
      {
      if (tmpLine[0] != '\0')
        strcat(tmpLine," ");
 
      strcat(tmpLine,MSimFlagType[index]);
      }
    }   /* END for (index) */
 
  sprintf(Buffer,"%s%-30s  %s\n",Buffer,MParam[pSimFlags],tmpLine);
 
  sprintf(Buffer,"%s%-30s  %s\n",Buffer,MParam[pSimJobSubmissionPolicy],MSimQPolicy[S->JobSubmissionPolicy]);
  sprintf(Buffer,"%s%-30s  %d\n",Buffer,MParam[pSimInitialQueueDepth],S->InitialQueueDepth);
 
  sprintf(Buffer,"%s%-30s  %.2lf\n",Buffer,MParam[pSimWCAccuracy],S->WCAccuracy);
  sprintf(Buffer,"%s%-30s  %.2lf\n",Buffer,MParam[pSimWCAccuracyChange],S->WCAccuracyChange);
  sprintf(Buffer,"%s%-30s  %d\n",Buffer,MParam[pSimNodeCount],S->NodeCount);
  sprintf(Buffer,"%s%-30s  %s\n",Buffer,MParam[pSimNCPolicy],MSimNCPolicy[S->NCPolicy]);
  sprintf(Buffer,"%s%-30s  %d\n",Buffer,MParam[pSimWCScalingPercent],S->WCScalingPercent);
 
  if (S->ComRate != 0.0)
    {
    sprintf(Buffer,"%s%-30s  %4.2f\n",Buffer,MParam[pComRate],S->ComRate);
    sprintf(Buffer,"%s%-30s  %s\n",Buffer,MParam[pCommunicationType],MComType[S->CommunicationType]);
    sprintf(Buffer,"%s%-30s  %4.2f\n",Buffer,MParam[pIntraFrameCost],S->IntraFrameCost);
    sprintf(Buffer,"%s%-30s  %4.2f\n",Buffer,MParam[pInterFrameCost],S->InterFrameCost);
    }
 
  sprintf(Buffer,"%s%-30s  %d\n",Buffer,MParam[pStopIteration],S->StopIteration);
  sprintf(Buffer,"%s%-30s  %d\n",Buffer,MParam[pExitIteration],S->ExitIteration);
 
  DBG(4,fSIM) DPrint("INFO:     simulation parameters displayed\n");
 
  strcat(Buffer,"\n");
 
  return(SUCCESS);
  }  /* END MSimShow() */




int MSimProcessEvents(

  int IQ[])  /* I */

  {
  /* check for simulation exit iteration */
 
  if (MSched.Iteration == MSim.ExitIteration)
    {
    DBG(0,fSCHED) DPrint("INFO:     scheduling complete (%s reached) on iteration %d\n",
      MParam[pExitIteration],
      MSched.Iteration);
 
    MSimSummarize();
    }
 
  /* check for simulation stop iteration */
 
  if (MSched.Iteration == MSim.StopIteration)
    {
    MSim.StopIteration = -1;
 
    MSched.Schedule = FALSE;
    }
 
  /* stop scheduling if queues are empty */
 
  if ((MSim.AutoShutdown == TRUE) &&
      (IQ[0] == -1) &&
      (MAQ[0] == -1))
    {
    DBG(0,fSCHED) DPrint("INFO:     scheduling complete (queues are empty) on iteration %d\n",
      MSched.Iteration);
 
    MSimSummarize();
    }

  /* send 'heartbeat' to log */

  if (!(MSched.Iteration % 1000))
    {
    DBG(0,fSCHED) DPrint("INFO:     iteration %d completed\n",
      MSched.Iteration);
    }

  return(SUCCESS);
  }  /* END MSimProcessEvents() */




int MSimProcessOConfig(

  msim_t *S,
  int     PIndex,
  int     IVal,
  double  DVal,
  char   *SVal,
  char  **SArray)

  {
  if (S == NULL)
    return(FAILURE);

  switch (PIndex)
    {
    case pCommunicationType:

      MUGetIndex(SVal,MComType,FALSE,S->CommunicationType);

      break;

    case pComRate:

      S->ComRate = DVal;

      break;

    case pExitIteration:

      S->ExitIteration = IVal;

      break;

    case pIntraFrameCost:

      S->IntraFrameCost = DVal;

      break;

    case pInterFrameCost:

      S->InterFrameCost = DVal;

      break;

    case pRandomizeJobSize:

      S->RandomizeJobSize = MUBoolFromString(SVal,S->RandomizeJobSize);

      break;

    case pSimNodeCount:

      S->NodeCount = MIN(IVal,MAX_MNODE);

      if (IVal > MAX_MNODE)
        {
        DBG(1,fCONFIG) DPrint("WARNING:  specified nodecount value (%d) is too high  (adjusting to %d)\n",
          IVal,
          MAX_MNODE);
        }

      break;

    case pResourceTraceFile:

      MUStrCpy(S->ResourceTraceFile,SVal,sizeof(S->ResourceTraceFile));

      break;

    case pSimAutoShutdown:

      S->AutoShutdown = MUBoolFromString(SVal,TRUE);

      break;

    case pSimDefaultJobFlags:

      {
      int index;
      
      unsigned long tmpUL;

      S->TraceDefaultJobFlags = 0;

      for (index = 0;SArray[index] != NULL;index++)
        {
        MUBMFromString(SArray[index],MJobFlags,&tmpUL);

        S->TraceDefaultJobFlags |= tmpUL;
        }  /* END for (index) */
      }    /* END BLOCK */

      break;

    case pSimFlags:

      {
      int index;
      int findex;

      S->Flags = 0;

      for (index = 0;SArray[index] != NULL;index++)
        {
        if ((findex = MUGetIndex(SArray[index],MSimFlagType,FALSE,0)) == 0)
          continue;

        S->Flags |= (1 << findex);
        }    /* END for (index) */
      }      /* END BLOCK */

      break;

    case pSimIgnoreJobFlags:

      {
      int index;

      unsigned long tmpUL;

      S->TraceIgnoreJobFlags = 0;

      for (index = 0;SArray[index] != NULL;index++)
        {
        MUBMFromString(SArray[index],MJobFlags,&tmpUL);

        S->TraceIgnoreJobFlags |= tmpUL;
        }  /* END for (index) */
      }    /* END BLOCK */

      break;

    case pSimInitialQueueDepth:

      S->InitialQueueDepth = IVal;

      break;

    case pSimJobSubmissionPolicy:

      if ((S->JobSubmissionPolicy = 
            MUGetIndex(SVal,MSimQPolicy,0,S->JobSubmissionPolicy)) == 0)
        {
        DBG(1,fCONFIG) DPrint("ALERT:    invalid %s parameter specified '%s'\n",
          MParam[pSimJobSubmissionPolicy],
          SVal);
        }

      break;

    case pSimNCPolicy:

      S->NCPolicy = MUGetIndex(SVal,MSimNCPolicy,0,S->NCPolicy);

      break;

    case pSimScaleJobRunTime:

      S->ScaleJobRunTime = MUBoolFromString(SVal,FALSE);

      break;

    case pSimStartTime:

      S->StartTime = MUTimeFromString(SVal);

      break;

    case pSimWCAccuracy:

      S->WCAccuracy = DVal;

      break;

    case pSimWCAccuracyChange:

      S->WCAccuracyChange = DVal;

      break;

    case pSimWCScalingPercent:

      S->WCScalingPercent = IVal;

      break;

    case pStopIteration:

      S->StopIteration = IVal;

      break;

    case pWorkloadTraceFile:

      MUStrCpy(S->WorkloadTraceFile,SVal,sizeof(S->WorkloadTraceFile));

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(PIndex) */

  return(SUCCESS);
  }  /* END MSimProcessOConfig() */

/* END MSim.c */

