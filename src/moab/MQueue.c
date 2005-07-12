/* HEADER */
        
/* Contains:                                 *
 *  int MQueueInitialize(Q,QueueName)        *
 *                                           */


#include "moab.h"
#include "msched-proto.h"  

extern mlog_t      mlog;

extern mjob_t     *MJob[];
extern mnode_t    *MNode[];
extern mpar_t      MPar[];
extern mrm_t       MRM[];

extern mstat_t     MStat;
extern mattrlist_t MAList;
extern msched_t    MSched;
extern int         MAQ[];

extern const char *MHoldType[];    
extern const char *MPolicyMode[];
extern const char *MDefReason[];
extern const char *MAllocRejType[];
extern const char *MJRType[];
extern const char *MBFPolicy[];
extern const char *MXO[];
extern const char *MJobDependType[];




int MQueueInitialize(

  mjob_t **Q,     /* I (modified) */
  char    *QName) /* I */

  {
  if ((Q == NULL) || (QName == NULL))
    {
    return(FAILURE);
    }

  if ((*Q = (mjob_t *)calloc(1,sizeof(mjob_t))) == NULL)
    {
    DBG(1,fSTRUCT) DPrint("ALERT:    cannot allocate memory for queue %s, errno: %d (%s)\n",
      (QName != NULL) ? QName : "NULL",
      errno,
      strerror(errno));
 
    return(FAILURE);
    }
 
  MUStrCpy((*Q)->Name,QName,sizeof((*Q)->Name));

  (*Q)->Next = *Q;
  (*Q)->Prev = *Q;

  return(SUCCESS);
  }  /* END MQueueInitialize() */




int MQueuePrioritizeJobs(

  mjob_t **Q,         /* I: list of jobs to be prioritized (optional)   */
  int     *JobIndex)  /* O: hi-low priority sorted array of job indexes */ 

  {
  double  tmpD;
  int     jindex;

  long    MaxIdleStartPriority = 0;

  mjob_t *J;

  if (JobIndex != NULL)
    jindex = 0;

  if ((Q == NULL) ||
      (Q[0] == NULL) ||
      (Q[0]->Next == NULL) ||
      (Q[0]->Next == Q[0]))
    {
    /* no queue specified, search full job table */

    for (jindex = 0;JobIndex[jindex] != -1;jindex++)
      {
      J = MJob[JobIndex[jindex]];

      MJobGetStartPriority(J,0,&tmpD,0,NULL);
 
      J->StartPriority = (unsigned long)tmpD;

      if ((J->State == mjsStarting) || (J->State == mjsRunning))
        {
        MJobGetRunPriority(J,0,&tmpD,NULL);
 
        J->RunPriority = (unsigned long)tmpD;
        }
      else 
        {
        MaxIdleStartPriority = MAX(MaxIdleStartPriority,J->StartPriority);  
        } 
      }
    }
  else
    {
    jindex = 0;

    for (J = Q[0]->Next;J != Q[0];J = J->Next)
      {
      MJobGetStartPriority(J,0,&tmpD,0,NULL);

      J->StartPriority = (unsigned long)tmpD;

      if ((J->State == mjsStarting) || (J->State == mjsRunning))
        {
        MJobGetRunPriority(J,0,&tmpD,NULL);

        J->RunPriority = (unsigned long)tmpD;
        }
      else
        {
        MaxIdleStartPriority = MAX(MaxIdleStartPriority,J->StartPriority);
        }

      if (JobIndex != NULL)
        JobIndex[jindex++] = J->Index;
      }
    }  /* END for (J) */

  if (JobIndex != NULL)
    {
    if (jindex > 1)
      {      
      qsort(
        (void *)&JobIndex[0],
        jindex,
        sizeof(int),
        (int(*)(const void *,const void *))MJobStartPrioComp);
      }

    JobIndex[jindex] = -1;          

#if !defined(__MALWAYSPREEMPT)
    /* adjust preemption backfill status */
 
    for (jindex = 0;JobIndex[jindex] != -1;jindex++)
      {
      J = MJob[JobIndex[jindex]];

      if (((J->State == mjsStarting) || (J->State == mjsRunning)) && 
           (J->StartPriority >= MaxIdleStartPriority) &&
           (J->Flags & (1 << mjfBackfill)) &&
           (J->Flags & (1 << mjfPreemptee)))
        {
        J->SpecFlags ^= (1 << mjfPreemptee);
       
        MJobUpdateFlags(J);
 
        DBG(2,fCORE) DPrint("INFO:     backfill job '%s' no longer preemptible (%ld > %ld)\n",
          J->Name,
          J->StartPriority,
          MaxIdleStartPriority);
        }
      }
#endif /* !defined(__MALWAYSPREEMPT) */
    }    /* END if (JobIndex != NULL) */

  return(SUCCESS);
  }  /* END MQueuePrioritizeJobs() */




int MOQueueInitialize(

  int *Queue)  /* I */

  {
  if (Queue == NULL)
    {
    return(FAILURE);
    }

  Queue[0] = -1;
  
  return(SUCCESS);
  }  /* END MOQueueInitialize() */




int MOQueueDestroy(

  int *Queue,        /* I (modified) */
  int  ModifyStats)  /* I (boolean) */

  {
  int jindex;

  mjob_t *J;

  if (Queue == NULL)
    {
    return(FAILURE);
    }

  if (ModifyStats == TRUE)
    {
    /* adjust idle job stats */            

    for (jindex = 0;Queue[jindex] != -1;jindex++)
      {
      J = MJob[Queue[jindex]];

      if ((J != NULL) && (J->State == mjsIdle))
        {
        /* remove idle job from object stats */

        MPolicyAdjustUsage(NULL,J,NULL,mlIdle,MPar[0].L.IP,-1,-1,NULL);             
        MPolicyAdjustUsage(NULL,J,NULL,mlIdle,NULL,-1,-1,NULL); 

        MStat.IdleJobs--;
        }

      Queue[jindex] = -1;
      }  /* END for (jindex) */
    }    /* END if (ModifyStats == TRUE) */

  Queue[0] = -1;

  return(SUCCESS);
  }  /* END MOQueueDestroy() */




int MQueueBackFill(
 
  int     *BFQueue,
  int      PLevel,
  mpar_t  *P)
 
  {
  int BFNodeCount;
  int BFProcCount;
 
  long BFTime;
 
  long OBFTime;
 
  nodelist_t BFNodeList;
 
  int     tmpQ[MAX_MJOB];
 
  mjob_t *JList[MAX_MJOB];

  mjob_t *J;
 
  int    jindex;
  int    index;
 
  long AdjBFTime;

  double tmpD;
 
  mcres_t DRes;

  const char *FName = "MQueueBackFill";
 
  DBG(1,fSCHED) DPrint("%s(BFQueue,%s,%s)\n",
    FName,
    MPolicyMode[PLevel],
    (P != NULL) ? P->Name : "NULL");

  if (P == NULL)
    {
    return(FAILURE);
    }
 
  memset(&DRes,0,sizeof(DRes));
 
  DRes.Procs = 1;
 
  /* backfill partition */ 
 
#ifdef __MLONGESTBFWINDOWFIRST
  OBFTime = MAX_MTIME;
#else /* __MLONGESTBFWINDOWFIRST */
  OBFTime = 0;
#endif /* __MLONGESTBFWINDOWFIRST */
 
  /* process normal backfill */
 
  while (MBFGetWindow(
           &BFNodeCount,
           &BFProcCount,
           BFNodeList,
           &BFTime,
           OBFTime,
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
    DBG(3,fSCHED) DPrint("INFO:     backfill window obtained [%d nodes/%d procs : %s]\n",
      BFNodeCount,
      BFProcCount,
      MULToTString(BFTime));
 
    OBFTime = BFTime;

    MUNLGetMaxAVal((mnalloc_t *)BFNodeList,mnaSpeed,NULL,(void **)&tmpD);
 
    AdjBFTime = (int)((double)BFTime * tmpD);
 
    MOQueueInitialize(tmpQ);
 
    if (MQueueSelectJobs(
          BFQueue, 
          tmpQ,
          PLevel,
          BFNodeCount,
          BFProcCount,
          AdjBFTime,
          P->Index,
          NULL,
          FALSE) == FAILURE)
      {
      DBG(5,fSCHED) DPrint("INFO:     no jobs meet BF window criteria in partition %s\n",
        P->Name);
 
      MOQueueDestroy(tmpQ,FALSE);
 
      continue;
      }   /* END if (MQueueSelectJobs() == FAILURE) */

    /* remove ineligible jobs: reserved jobs, nobf QOS jobs, etc */

    if (tmpQ[0] == -1)
      {
      MOQueueDestroy(tmpQ,FALSE);            

      continue;
      }

    index = 0;

    for (jindex = 0;tmpQ[jindex] != -1;jindex++)
      {
      J = MJob[tmpQ[jindex]];

      if (J->R != NULL)
        {
        /* do not backfill reserved jobs */

        continue;
        }

      if ((J->Cred.Q != NULL) && (J->Cred.Q->Flags & (1 << mqfnobf)))
        {
        /* job QOS disables backfill */

        continue;
        }
 
      if (index != jindex)
        tmpQ[index] = tmpQ[jindex];
 
      JList[index] = J;

      index++;

      if ((MPar[0].BFDepth > 0) &&
          (index >= MPar[0].BFDepth))
        {
        break;
        }
      }  /* END for (jindex) */

    /* terminate queues */

    tmpQ[index]  = -1; 
    JList[index] = NULL;
 
    switch(MPar[0].BFPolicy)
      {
      case bfPREEMPT:
 
        MBFPreempt(
          JList,
          PLevel,
          BFNodeList,
          BFTime,
          BFNodeCount,
          BFProcCount,
          P); 
 
        break;
 
      case bfFIRSTFIT:

        MBFFirstFit(
          JList,
          PLevel,
          BFNodeList,
          BFTime,
          BFNodeCount,
          BFProcCount,
          P);
 
        break;
 
      case bfBESTFIT:

        MBFBestFit(
          JList,
          PLevel,
          BFNodeList,
          BFTime,
          BFNodeCount,
          BFProcCount,
          P);

        break;
 
      case bfGREEDY:

        MBFGreedy(
          JList,
          PLevel,
          BFNodeList,
          BFTime,
          BFNodeCount,
          BFProcCount,
          P); 
 
        break;

      case bfNONE:

        return(SUCCESS);

        /*NOTREACHED*/

        break;
 
      default:

        DBG(0,fSCHED) DPrint("ERROR:    unexpected backfill policy %d (using %s)\n",
          MPar[0].BFPolicy,
          MBFPolicy[DEFAULT_BACKFILLPOLICY]);

        MPar[0].BFPolicy = DEFAULT_BACKFILLPOLICY; 
 
        MBFFirstFit(
          JList,
          PLevel,
          BFNodeList,
          BFTime,
          BFNodeCount,
          BFProcCount,
          P);

        break;
      } /* END switch (MPar[0].BFPolicy)  */
 
    MOQueueDestroy(tmpQ,FALSE);
    }   /* END while (MBFGetWindow() == SUCCESS) */
 
  return(SUCCESS);
  }  /* END MQueueBackFill() */




int MQueueScheduleIJobs(
 
  int     *Q,  /* I */
  mpar_t  *P)  /* I */
 
  {
  int      jindex;
 
  mnodelist_t MNodeList;
  mnodelist_t tmpMNodeList;
 
  int      index;
  int      sindex;
 
  mjob_t  *J;
  mreq_t  *RQ;

  mbool_t  ResCountRej;

  int      rqindex;
 
  int      SchedCount;
 
  char     NodeMap[MAX_MNODE];
 
  int      PResCount[MAX_MQOS];
 
  int      BestSIndex;
  int      BestSValue;
 
  int      IdleJobFound;
 
  int      rindex;
  int      qindex;
 
  long     tmpL;
 
  int      AllowPreemption;
 
  unsigned long DefaultSpecWCLimit;

  int DefaultTaskCount[MAX_MREQ_PER_JOB];

  int      NAPolicy;

  const char *FName = "MQueueScheduleIJobs";
 
  DBG(2,fSCHED) DPrint("%s(Q,%s)\n",
    FName,
    (P != NULL) ? P->Name : NULL);

  if ((P == NULL) || (Q == NULL))
    {
    return(FAILURE);
    }

  if (MLocalQueueScheduleIJobs(Q,P) == SUCCESS)
    {
    return(SUCCESS);
    }
 
  /*
     NOTE: 
 
     need a new job metric.  must find schedule with best completion
     times for most highest priority jobs.  NOTE:  this will significantly
     affect current resource weightings.  Do we weight by job size, ie
     V = P * TC * f(Tc)?
 
   */
 
  if (Q[0] == -1)
    {
    DBG(2,fSCHED) DPrint("INFO:     no jobs in queue\n");
 
    return(FAILURE);
    }
 
  IdleJobFound = FALSE;
 
  SchedCount = 0;

  memset(PResCount,0,sizeof(PResCount));
 
  /* attempt to start all feasible jobs */
  /* jobs listed in priority FIFO order */
  /* reserved jobs included in list     */
 
  for (jindex = 0;Q[jindex] != -1;jindex++)
    {
    J = MJob[Q[jindex]];
 
    MTRAPJOB(J,FName);
 
    DBG(7,fSCHED) DPrint("INFO:     checking job '%s'\n",
      J->Name);
 
    if ((J->EState == mjsStarting) ||
        (J->State == mjsStarting) ||
        (J->EState == mjsRunning) ||
        (J->State == mjsRunning) ||
        (J->State == mjsSuspended))
      {
      /* continue if job started in MQueueScheduleRJobs() */
 
      DBG(7,fSCHED) DPrint("INFO:     job '%s' not considered for prio scheduling (State/EState)\n", 
        J->Name);
 
      continue;
      }
 
    if ((J->R != NULL) &&
       ((J->R->PtIndex != P->Index) || (J->R->StartTime > MSched.Time)))
      {
      /* job already has reservation */
 
      DBG(7,fSCHED) DPrint("INFO:     job '%s' not considered for prio scheduling (existing reservation)\n",
        J->Name);
 
      /* locate QOS group */
 
      for (rindex = 0;rindex < MAX_MQOS;rindex++)
        {
        if (MPar[0].ResDepth[rindex] == 0)
          continue;
 
        for (qindex = 0;qindex < MAX_MQOS;qindex++)
          {
          if (MPar[0].ResQOSList[rindex][qindex] == NULL)
            break;
 
          if ((MPar[0].ResQOSList[rindex][qindex] == J->Cred.Q) ||
              (MPar[0].ResQOSList[rindex][qindex] == (mqos_t *)MAX_MQOS))
            {
            break;
            }
          }  /* END for (qindex) */
 
        if ((MPar[0].ResQOSList[rindex][qindex] == J->Cred.Q) ||
            (MPar[0].ResQOSList[rindex][qindex] == (mqos_t *)MAX_MQOS))
          {
          break;
          }
        }    /* END for (rindex) */
 
      if (rindex != MAX_MQOS)
        { 
        PResCount[rindex]++;
        }
 
      continue;
      }  /* END if ((J->R != NULL) && ...) */
 
    if (IdleJobFound == TRUE)
      {
      DBG(7,fSCHED) DPrint("INFO:     job '%s' not considered for prio scheduling (Idle Job Reached)\n",
        J->Name);
 
      /* reserve job if eligible */
 
      if (MJobCheckLimits(
            J,
            ptSOFT,
            P,
            (1 << mlActive),
            NULL) == FAILURE)
        {
        /* continue if jobs started on this iteration    */
        /* cause this job to now violate active policies */
 
        DBG(5,fSCHED) DPrint("INFO:     skipping job '%s'  (job now violates active policy)\n",
          J->Name);
 
        continue;
        }
 
      if ((MJobPReserve(J,P->Index,PResCount,NULL) != SUCCESS) ||
          (J->R == NULL) ||
          (J->R->StartTime > MSched.Time))
        {
        /* reservation made for idle job */
  
        continue;
        }

      /* job reservation created for immediate use (schedule job) */
      }  /* END if (IdleJobFound == TRUE) */
 
    MStat.JobsEvaluated++;
 
    BestSValue = 0;
    BestSIndex = -1; 
 
    NAPolicy = (J->Req[0]->NAllocPolicy != NULL) ?
      J->Req[0]->NAllocPolicy->NAllocPolicy :
      MPar[J->Req[0]->PtIndex].NAllocPolicy;
 
    DefaultSpecWCLimit = J->SpecWCLimit[0];
  
    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      DefaultTaskCount[rqindex] = J->Req[rqindex]->TaskRequestList[0];
      }  /* END for (rqindex) */
 
    for (sindex = 1;J->Req[0]->TaskRequestList[sindex] > 0;sindex++)
      {
      J->SpecWCLimit[0] = J->SpecWCLimit[sindex];

      J->Request.TC = 0;

      for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
        {
        RQ = J->Req[rqindex];

        RQ->TaskRequestList[0]  = RQ->TaskRequestList[sindex];
        RQ->TaskCount           = RQ->TaskRequestList[sindex];
        J->Request.TC      += RQ->TaskRequestList[sindex];
        }  /* END for (rqindex) */
 
      /* is the best strategy widest fit?  ie, local greedy? */
 
      MJobUpdateResourceCache(J,sindex);
 
      if (MJobCheckLimits(
            J,
            ptSOFT,
            P,
            (1 << mlActive),
            NULL) == FAILURE)
        {
        /* continue if jobs started on this iteration    */
        /* cause this job to now violate active policies */
 
        DBG(5,fSCHED) DPrint("INFO:     skipping job '%s'  (job violates active policy')\n",
          J->Name);
 
        continue;
        }
 
      DBG(4,fSCHED) DPrint("INFO:     checking job %s(%d)  state: %s (ex: %s)\n",
        J->Name,
        sindex,
        MJobState[J->State],
        MJobState[J->EState]);
 
      /* select resources for job */ 
 
      if (MPar[0].BFPolicy == bfPREEMPT)
        {
        /* all priority jobs are preemptors */
 
        AllowPreemption = TRUE;
        }
      else
        {
        AllowPreemption = MBNOTSET;
        }
 
      if (MJobSelectMNL(
            J,
            P,
            NULL,
            MNodeList,
            NodeMap,
            AllowPreemption) == FAILURE)
        {
        DBG(6,fSCHED) DPrint("INFO:     cannot locate %d tasks for job '%s' in partition %s\n",
          J->Request.TC,
          J->Name,
          P->Name);
 
        continue;
        }  /* END:  if (MJobSelectMNL() == FAILURE) */
 
      BestSValue = 1;
      BestSIndex = sindex;
      }    /* END for (sindex) */
 
    J->SpecWCLimit[0] = DefaultSpecWCLimit;
    J->Request.TC = 0;

    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      RQ = J->Req[rqindex];

      RQ->TaskRequestList[0]  = DefaultTaskCount[rqindex]; 
      RQ->TaskCount           = DefaultTaskCount[rqindex];
      J->Request.TC      += DefaultTaskCount[rqindex];
      }  /* END for (rqindex) */
 
    MJobUpdateResourceCache(J,0);
 
    if (BestSValue <= 0)
      {
      /* no available slot located */

      if (MJobPReserve(J,P->Index,PResCount,&ResCountRej) == SUCCESS)
        {
        continue;
        }
      else
        {
        /* skip job */
 
        if (!(J->Flags & (1 << mjfAdvReservation)))
          {
          IdleJobFound = TRUE;
          }
 
        continue;
        }
      }    /* END:  if (BestSValue <= 0) */
 
    /* adequate nodes found */
 
    J->SpecWCLimit[0]      = J->SpecWCLimit[BestSIndex];
    J->Request.TC      = 0;

    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      RQ = J->Req[rqindex];

      RQ->TaskRequestList[0]  = RQ->TaskRequestList[BestSIndex];
      RQ->TaskCount           = RQ->TaskRequestList[BestSIndex];
      J->Request.TC      += RQ->TaskRequestList[BestSIndex];

      RQ->TaskRequestList[BestSIndex] = DefaultTaskCount[rqindex];
      }
 
    J->SpecWCLimit[BestSIndex]      = DefaultSpecWCLimit;
 
    if (BestSIndex > 1)
      MJobUpdateResourceCache(J,BestSIndex);
 
    if (MPar[0].NodeSetDelay > 0)
      memcpy(tmpMNodeList,MNodeList,sizeof(tmpMNodeList));
 
    if (MJobAllocMNL( 
          J,
          MNodeList,
          NodeMap,
          NULL,
          NAPolicy,
          MSched.Time) == FAILURE)
      {
      DBG(1,fSCHED) DPrint("ERROR:    cannot allocate nodes to job '%s' in partition %s\n",
        J->Name,
        P->Name);
 
      tmpL = MSched.Time + 1;
 
      MJobSetAttr(J,mjaSysSMinTime,(void **)&tmpL,0,mAdd);
 
      if ((MJobPReserve(J,P->Index,PResCount,&ResCountRej) == SUCCESS) ||
          (ResCountRej == FALSE))
        {
        continue;
        }
      else
        {
        /* skip job */
 
        IdleJobFound = TRUE;
 
        continue;
        }
      }    /* END if (NodesAllocated == FALSE) */
 
    if (MJobStart(J) == FAILURE)
      {
      DBG(1,fSCHED) DPrint("ERROR:    cannot start job '%s' in partition %s\n",
        J->Name,
        P->Name);
 
      tmpL = MSched.Time + 1;
 
      MJobSetAttr(J,mjaSysSMinTime,(void **)&tmpL,0,mAdd);
 
      if (MJobPReserve(J,P->Index,PResCount,&ResCountRej) == SUCCESS)
        { 
        continue;
        }
      else
        {
        /* skip job */
 
        IdleJobFound = TRUE;
 
        continue;
        }
      }    /* END if (MJobStart() == FAILURE) */
 
    if ((MPar[0].BFPolicy == bfPREEMPT) && (IdleJobFound == TRUE))
      {
      /* job was backfilled */
 
      J->SpecFlags |= (1 << mjfPreemptee);
 
      MJobUpdateFlags(J);
      }
 
    SchedCount++;
    }      /* END for jindex */
 
  if (SchedCount != 0)
    {
    DBG(2,fSCHED) DPrint("INFO:     %d jobs started on iteration %d\n",
      SchedCount,
      MSched.Iteration);
    }
 
  DBG(2,fSCHED)
    {
    fprintf(mlog.logfp,"Active Jobs------\n");
 
    for (index = 0;MAQ[index] != -1;index++)
      MJobShow(MJob[MAQ[index]],0,NULL);
 
    fprintf(mlog.logfp,"------------------\n");
    }
 
  DBG(2,fSCHED) DPrint("INFO:     resources available after scheduling: N: %d  P: %d\n", 
    MPar[0].IdleNodes,
    MPar[0].ARes.Procs);
 
  return(SUCCESS);
  }  /* END MQueueScheduleIJobs() */





int MQueueDiagnose(

  mjob_t **FullQ,     /* I */
  int     *NBJobList, 
  int      PLevel,
  mpar_t  *PS,
  char    *Buffer)

  {
  int       pindex;
  int       index;
  int       jindex;
 
  mjob_t   *J;

  char      DValue[MAX_MNAME];
  enum      MJobDependEnum DType;
 
  char      Message[MAX_MLINE];
 
  int       RIndex;
  mbool_t   IsBlocked;
 
  mpar_t   *P;
  mpar_t   *GP;
 
  mreq_t    *RQ;

  const char *FName = "MQueueDiagnose";
 
  if ((FullQ == NULL) || 
      (Buffer == NULL) ||
      (PS == NULL))
    {
    return(FAILURE);
    }

  /* NOTE:  must be synchronized with MQueueSelectJobs() and MJobCheckPolicies() */
 
  sprintf(Buffer,"%sDiagnosing blocked jobs (policylevel %s  partition %s)\n\n",
    Buffer,
    MPolicyMode[PLevel],
    PS->Name);
 
  if ((FullQ[0]->Next == NULL) || (FullQ[0]->Next == FullQ[0]))
    {
    DBG(4,fUI) DPrint("no blocked jobs found in %s()\n",
      FName);
 
    sprintf(Buffer,"no blocked jobs found\n");
 
    return(FAILURE);
    }
 
  MLocalCheckFairnessPolicy(NULL,MSched.Time,NULL);
 
  for (J = FullQ[0]->Next;J != FullQ[0];J = J->Next)
    {
    if ((PS->Index > 0) && (MUBMCheck(PS->Index,J->PAL) == FAILURE))
      continue;
 
    RQ = J->Req[0]; /* FIXME */
 
    if ((J->State == mjsStarting) || (J->State == mjsRunning))
      {
      continue;
      } 
 
    DBG(4,fUI) DPrint("INFO:     checking job '%s' QOS: %s  (QFlags: %lu)\n",
      J->Name,
      (J->Cred.Q != NULL) ? J->Cred.Q->Name : "NULL",
      (J->Cred.Q != NULL) ? J->Cred.Q->Flags : 0);

    /* look for job in NBJobList */

    for (jindex = 0;jindex < MAX_MJOB + MAX_MHBUF;jindex++)
      {
      if (NBJobList[jindex] == -1)
        break;

      if (J->Index == NBJobList[jindex])
        break;
      }  /* END for (jindex) */

    if (J->Index == NBJobList[jindex])
      {
      /* job is not blocked */

      continue;
      }

    IsBlocked = FALSE;

    /* check job state */
 
    if ((J->State != mjsIdle) && (J->State != mjsHold))
      {
      sprintf(Buffer,"%sjob %-20s has non-idle state (state: '%s')\n",
        Buffer,
        J->Name,
        MJobState[J->State]);
 
      continue;
      }
 
    /* check user/system/batch hold */
 
    if (J->Hold != 0)
      {
      sprintf(Buffer,"%sjob %-20s has the following hold(s) in place: ",
        Buffer,
        J->Name);
 
      for (index = 0;MHoldType[index] != NULL;index++)
        {
        if (J->Hold & (1 << index))
          {
          strcat(Buffer," ");
          strcat(Buffer,MHoldType[index]);
          }
        }    /* END for (index) */
 
      strcat(Buffer,"\n"); 
 
      IsBlocked = TRUE;
      }
 
    /* check if job has been previously scheduled or deferred */
 
    if (J->EState != mjsIdle)
      {
      sprintf(Buffer,"%sjob %-20s has non-idle expected state (expected state: %s)\n",
        Buffer,
        J->Name,
        MJobState[J->EState]);
 
      IsBlocked = TRUE;
      }
 
    /* check release time */
 
    if (J->SMinTime > MSched.Time)
      {
      sprintf(Buffer,"%sjob %-20s has not reached its start date (%s to startdate)\n",
        Buffer,
        J->Name,
        MULToTString(J->SMinTime - MSched.Time));
 
      IsBlocked = TRUE;
      }
 
    /* check job dependencies */
 
    if (MJobCheckDependency(J,&DType,DValue) == FAILURE)
      {
      sprintf(Buffer,"%sjob %s requires %s of job '%s')\n",
        Buffer,
        J->Name, 
        MJobDependType[DType],
        DValue);
 
      IsBlocked = TRUE;
      }  /* END if (MJobCheckDependency(J,&DType,DValue) == FAILURE) */

    if (MLocalCheckFairnessPolicy(J,MSched.Time,NULL) == FAILURE)
      {
      DBG(6,fSCHED) DPrint("INFO:     job %s rejected, partition %s (violates local fairness policy)\n",
        J->Name,
        PS->Name);
 
      sprintf(Buffer,"%sjob %-20s would violate 'local' fairness policies\n",
        Buffer,
        J->Name);
 
      IsBlocked = TRUE;
      }
 
    GP = &MPar[0];
 
    /* determine all partitions in which job can run */

    for (pindex = 1;pindex < MAX_MPAR;pindex++)
      {
      P = &MPar[pindex];
 
      if (P->ConfigNodes == 0)
        continue;
 
      if (MUNumListGetCount(
            J->StartPriority,
            RQ->DRes.PSlot,
            P->CRes.PSlot,
            0,
            NULL) == FAILURE)
        {
        /* required classes not configured in partition */
 
        sprintf(Buffer,"%sjob %-20s requires classes not configured in partition %s (%s)\n",
          Buffer,
          J->Name,
          P->Name,
          MUCAListToString(RQ->DRes.PSlot,P->CRes.PSlot,NULL));

        IsBlocked = TRUE;
 
        continue;
        }
 
      if (MUBMCheck(P->Index,J->PAL) == FAILURE)
        { 
        sprintf(Buffer,"%sjob %-20s not allowed to run in partition %s  (partitions allowed: %s)\n",
          Buffer,
          J->Name,
          P->Name,
          MUListAttrs(ePartition,J->PAL[0]));

        IsBlocked = TRUE;
 
        continue;
        }
 
      /* check job limits and other policies */
 
      if (MJobCheckPolicies(
            J,
            PLevel,
            0,
            P,
            &RIndex,
            Message,
            MSched.Time) == FAILURE)
        {
        strcat(Buffer,Message);

        IsBlocked = TRUE;
 
        continue;
        }
 
      if (GP->FSC.FSPolicy != fspNONE)
        {
        int OIndex;
 
        if (MFSCheckCap(NULL,J,P,&OIndex) == FAILURE)
          {
          DBG(5,fSCHED) DPrint("INFO:     job '%s' exceeds %s FS cap\n",
            J->Name,
            MXO[OIndex]);
 
          sprintf(Buffer,"%sjob %-20s would violate %s FS cap in partition %s\n",
            Buffer,
            J->Name,
            MXO[OIndex],
            P->Name);

          IsBlocked = TRUE;
 
          continue;
          }
        }    /* END if (GP->FSC.FSPolicy != fspNONE) */
      }      /* END for (pindex) */

    if (IsBlocked == FALSE) 
      {
      switch(J->BlockReason)
        {
        case mjneIdlePolicy:

          sprintf(Buffer,"%sjob %-20s is blocked by idle job policy\n",
            Buffer,
            J->Name);

          break;

        default:

          if ((PLevel == ptHARD) || (PLevel == ptOFF))
            {
            sprintf(Buffer,"%sjob %-20s is not blocked at this policy level\n",
              Buffer,
              J->Name);
            }

          break;
        }  /* END switch(J->BlockReason) */
      }
    }     /* END for (jindex) */

  return(SUCCESS);
  }  /* END MQueueDiagnose() */




int MQueueScheduleSJobs(

  int *Q)  /* I */

  {
  int         jindex;

  mjob_t     *J;

  mnodelist_t MNodeList;
  char        NodeMap[MAX_MNODE];

  int         AllowPreemption;

  const char *FName = "MQueueScheduleSJobs";

  DBG(5,fSCHED) DPrint("%s(Q)\n",
    FName);

  if (Q == NULL)
    {
    return(FAILURE);
    }

  /* locate suspended jobs */

  AllowPreemption = FALSE;

  for (jindex = 0;Q[jindex] != -1;jindex++)
    {
    J = MJob[Q[jindex]];

    if (J->State != mjsSuspended)
      continue;

    if (J->RMinTime > MSched.Time)
      continue;

    if (MJobSelectMNL(
          J,
          &MPar[J->Req[0]->PtIndex],
          J->NodeList,
          MNodeList,
          NodeMap,
          AllowPreemption) == FAILURE)
      {
      /* job nodes not available */

      continue;
      }

    /* all nodes support job */

    MJobResume(J,NULL,NULL);
    }  /* END for (jindex) */

  return(SUCCESS);
  }  /* END MQueueScheduleSJobs() */





int MQueueCheckStatus()
 
  {
  int index;
  int ReasonList[MAX_MREJREASON];
 
  int SrcQ[2];
  int tmpQ[2];
 
  enum MHoldReasonEnum Reason;
 
  mjob_t  *J;
 
  mjob_t  *JNext;
 
  char DeferMessage[MAX_MLINE];

  long Delta;
 
  const char *FName = "MQueueCheckStatus";
 
  DBG(3,fSTRUCT) DPrint("%s()\n",
    FName);
 
  /* purge expired jobs */
 
  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = JNext)
    {
    JNext = J->Next;
 
    DBG(7,fSTRUCT) DPrint("INFO:     checking purge criteria for job '%s'\n",
      J->Name);

    if ((J->RM != NULL) && (J->RM->WorkloadUpdateIteration != MSched.Iteration))
      {
      continue;
      }

    Delta = (long)MSched.Time - J->ATime;
 
    if (Delta > MAX(MAX(15,MSched.RMPollInterval),MSched.JobPurgeTime))
      {
      DBG(2,fSTRUCT) DPrint("WARNING:  job '%s' no longer detected (%ld > %ld)\n",
        J->Name,
        Delta,
        MAX(MAX(15,MSched.RMPollInterval),MSched.JobPurgeTime)); 
 
      DBG(2,fSTRUCT) DPrint("ALERT:    purging job '%s'\n",
        J->Name);

      if (((J->State == mjsStarting) || (J->State == mjsRunning)) &&
           (J->CompletionTime <= 0))
        {
        J->CompletionTime = MSched.Time;
        }
 
      if (J->Cred.A != NULL)
        {
        if (MAMAllocResCancel(
              J->Cred.A->Name,
              J->Name,
              "job purged",
              NULL,
              &Reason) == FAILURE)
          {
          DBG(1,fSCHED) DPrint("ERROR:    cannot cancel allocation reservation for job '%s'\n",
            J->Name);
          }
        }
 
      /* remove job from job list */

      if ((J->State == mjsStarting) || (J->State == mjsRunning))
        {
        MJobProcessCompleted(J);
        }
 
      MJobRemove(J);
      }  /* END if ((MSched.Time - J->ATime) > ...) */
 
    if ((J->State == mjsIdle) || (J->State == mjsDeferred) || (J->State == mjsHold))
      {
      if (J->Flags & (1 << mjfNoQueue))
        {
        DBG(2,fSTRUCT) DPrint("INFO:     cancelling Non-Queue job '%s'\n",
          J->Name);
 
        if (J->Cred.A != NULL)
          {
          if (MAMAllocResCancel(J->Cred.A->Name,J->Name,"interactive job purged",NULL,&Reason) == FAILURE)
            {
            DBG(1,fSCHED) DPrint("ERROR:    cannot cancel allocation reservation for job '%s'\n",
              J->Name);
            }
          }
 
        /* cancel job */
 
        if (J->EState == mjsDeferred) 
          {
          sprintf(DeferMessage,"SCHED_INFO:  job cannot run.  Reason: %s\n",
            MDefReason[J->HoldReason]);
 
          MRMJobCancel(J,DeferMessage,NULL);
          }
        else
          {
          MOQueueInitialize(SrcQ);
 
          SrcQ[0] = J->Index;
          SrcQ[1] = -1;
 
          memset(ReasonList,0,sizeof(ReasonList));
 
          if (MQueueSelectJobs(
                SrcQ,
                tmpQ,
                ptHARD,
                MAX_MNODE_PER_JOB,
                MAX_MTASK,
                MAX_MTIME,
                -1,
                ReasonList,
                FALSE) == FAILURE)
            {
            strcpy(DeferMessage,"SCHED_INFO:  job cannot run.  Reason: cannot select job\n");
            }
          else if (tmpQ[0] == -1)
            {
            for (index = 0;index < MAX_MREJREASON;index++)
              {
              if (ReasonList[index] != 0)
                break;
              }
 
            if (index != MAX_MREJREASON) 
              {
              sprintf(DeferMessage,"SCHED_INFO:  job cannot run.  Reason: %s\n",
                MAllocRejType[index]);
              }
            else
              {
              strcpy(DeferMessage,"SCHED_INFO:  job cannot run.  Reason: policy violation\n");
              }
            }
          else
            {
            strcpy(DeferMessage,"SCHED_INFO:  insufficient resources to run job\n");
            }
 
          MRMJobCancel(J,DeferMessage,NULL);
          }  /* END else (J->EState == mjsDeferred)    */
        }    /* END if (J->Flags & (1 << mjfNoQueue))  */
      }      /* END if ((J->State == mjsIdle)...       */
    }        /* END for (jindex)                       */
 
  /* purge subjobs */
 
  /* NYI */
 
  return(SUCCESS);
  }  /* END MQueueCheckStatus() */




int MQueueAddAJob(
 
  mjob_t *J)  /* I */
 
  {
  int      jindex;

  const char *FName = "MQueueAddAJob";
 
  DBG(4,fSCHED) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");
 
  if (J == NULL)
    {
    return(FAILURE);
    }
 
  /* MQueueAddAJob() requires J->StartTime */
 
  /* find open MAQ slot */
 
  for (jindex = 0;MAQ[jindex] != -1;jindex++)
    {
    if (J->Index == MAQ[jindex])
      {
      /* job previously added */
 
      return(SUCCESS);
      }
    }    /* END for (jindex) */
 
  MAQ[jindex]     = J->Index;
  MAQ[jindex + 1] = -1;
 
  /* sort active jobs in earliest completion time first order */
 
  qsort(
    (void *)&MAQ[0],
    (jindex + 1),
    sizeof(int),
    (int(*)(const void *,const void *))MJobCTimeComp);
 
  MStatUpdateActiveJobUsage(J); 
 
  /* if job is started by scheduler or is found already running */
 
  if ((J->EState == mjsIdle) || (J->CTime == MSched.Time))
    {
    /* new job detected */
 
    /* if job started by scheduler set EState to Starting.      */
    /* if job discovered in active state, set to whatever is detected. */
 
    if (MSched.Mode == msmSim)
      {
      J->EState = mjsRunning;
      }
    else
      {
      if (J->State == mjsIdle)
        J->EState = mjsStarting;
      else
        J->EState = J->State;
      }
    }
  else
    {
    DBG(6,fSCHED) DPrint("INFO:     previously detected active Job[%03d] '%s' added to MAQ\n",
      J->Index,
      J->Name);
    }
 
  DBG(5,fSCHED)
    {
    DBG(5,fSCHED) DPrint("INFO:     job '%s' added to MAQ at slot %d\n",
      J->Name,
      jindex);
 
    DBG(5,fSCHED) DPrint("INFO:     MAQ: "); 
 
    for (jindex = 0;MAQ[jindex] != -1;jindex++)
      {
      fprintf(mlog.logfp,"[%d : %s : %ld]",
        MAQ[jindex],
        MJob[MAQ[jindex]]->Name,
        MJob[MAQ[jindex]]->StartTime + MJob[MAQ[jindex]]->WCLimit - MSched.Time);
      }  /* END for (jindex) */
 
    fprintf(mlog.logfp,"\n");
    }
 
  return(SUCCESS);
  }  /* END MQueueAddAJob() */




int MQueueScheduleRJobs(
 
  int *Q)  /* I:  prioritized list of feasible jobs */
 
  {
  int      index;
  int      jindex;
 
  mnodelist_t MNodeList;
 
  int      SchedCount;
 
  int      PIndex;
 
  char     NodeMap[MAX_MNODE];
 
  int      HPSAdapterCheck;
  int      RMNodeStateCheck;
 
  int      HPSAdapterDelayCount;
  int      RMNodeStateDelayCount; 
 
  mjob_t  *J;
  mnode_t *N;
  mreq_t  *RQ;
 
  mres_t  *R;
 
  int      rqindex;
 
  int      rindex;
  int      qindex;
 
  int      HighPriority[MAX_MPAR][MAX_MQOS];
 
  int      GResPolicy;
  int      ResPolicy;
 
  long     tmpL;

  int      NAPolicy;
 
  const char *FName = "MQueueScheduleRJobs";
 
  DBG(2,fSCHED) DPrint("%s(Q)\n",
    FName);

  if (Q == NULL)
    {
    return(FAILURE);
    }
 
  memset(HighPriority,0,sizeof(HighPriority));
 
  GResPolicy = (MPar[0].ResPolicy == resDefault) ?
    DEFAULT_RESERVATIONMODE :
    MPar[0].ResPolicy;
 
  /* check all feasible jobs */
 
  SchedCount = 0;
 
  for (jindex = 0;Q[jindex] != -1;jindex++)
    {
    J = MJob[Q[jindex]];
 
    if ((J == NULL) || (J->State != mjsIdle))
      continue;
 
    MTRAPJOB(J,FName); 
 
    DBG(5,fSCHED) DPrint("INFO:     checking job %s in %s()\n",
      J->Name,
      FName);
 
    /* bind job to specified reservation */
 
    if (J->ResName[0] != '\0')
      {
      if (MResFind(J->ResName,&R) == SUCCESS)
        {
        /* non-preemptible */
 
        if (J->R != NULL)
          MResDestroy(&J->R);
 
        if (MJobReserve(J,mjrUser) == SUCCESS)
          {
          J->R->Priority = J->StartPriority;
          }
        }
      }
 
    PIndex = 0;
 
    if (J->R != NULL)
      {
      PIndex = J->R->PtIndex;
 
      ResPolicy = (MPar[PIndex].ResPolicy != resDefault) ?
        MPar[PIndex].ResPolicy :
        GResPolicy;
      }
    else
      {
      ResPolicy = GResPolicy;
      }
 
    /* create new reservations */
 
    if (J->Cred.Q->Flags & (1 << mqfreserved)) 
      {
      if (J->R != NULL)
        MResDestroy(&J->R);
 
      /* non-preemptible */
 
      if (MJobReserve(J,mjrQOSReserved) == FAILURE)
        {
        DBG(1,fSCHED) DPrint("ALERT:    cannot create %s reservation for job %s\n",
          MJRType[mjrQOSReserved],
          J->Name);
 
        continue;
        }
      }
    else if ((J->CMaxTime != MAX_MTIME) && (J->CMaxTime >= MSched.Time))
      {
      if (J->R != NULL)
        MResDestroy(&J->R);
 
      /* non-preemptible */
 
      if (MJobReserve(J,mjrDeadline) == FAILURE)
        {
        DBG(1,fSCHED) DPrint("ALERT:    cannot create %s reservation for job %s\n",
          MJRType[mjrDeadline],
          J->Name);
 
        continue;
        }
      }
    else if (J->RType == mjrPriority)
      {
      /* locate QOS group */
 
      for (rindex = 0;rindex < MAX_MQOS;rindex++)
        {
        if (MPar[0].ResDepth[rindex] == 0)
          continue;
 
        for (qindex = 0;qindex < MAX_MQOS;qindex++) 
          {
          if (MPar[0].ResQOSList[rindex][qindex] == NULL)
            break;
 
          if ((MPar[0].ResQOSList[rindex][qindex] == J->Cred.Q) ||
              (MPar[0].ResQOSList[rindex][qindex] == (mqos_t *)MAX_MQOS))
            {
            break;
            }
          }  /* END for (qindex) */
 
        if ((MPar[0].ResQOSList[rindex][qindex] == J->Cred.Q) ||
            (MPar[0].ResQOSList[rindex][qindex] == (mqos_t *)MAX_MQOS))
          {
          break;
          }
        }    /* END for (rindex) */
 
      if (rindex == MAX_MQOS)
        {
        /* ERROR:  cannot locate qos reservation group */
 
        DBG(2,fSCHED) DPrint("ALERT:    cannot locate QOS group for reserved job %s (QOS: %s) in %s()\n",
          J->Name,
          J->Cred.Q->Name,
          FName);
 
        if (J->R != NULL)
          MResDestroy(&J->R);
 
        continue;
        }
 
      HighPriority[PIndex][rindex]++;
 
      /* release all 'current highest' reservations */
      /* attempt 'slide-forward' on other priority reservations */
 
      if (J->R != NULL)
        {
        switch (ResPolicy) 
          {
          case resCurrentHighest:
 
            /* release all reservations */

            DBG(6,fSCHED) DPrint("INFO:     releasing reservation '%s' for res policy CurrentHighest\n",
              J->R->Name);
 
            MResDestroy(&J->R);
 
            /* recreate top 'ResDepth' reservations                                 */
            /* appropriate reservations will be re created in MQueueScheduleIJobs() */
 
/*
            if (HighPriority[PIndex][rindex] <=
                MPar[0].ResDepth[rindex])
              {
              if (MJobReserve(J,mjrPriority) == SUCCESS)
                {
                J->R->Priority  = J->StartPriority;
 
                J->R->Flags    |= (1 << mrfPreemptible);
                }
              }
*/
 
            break;
 
          case resHighest:
 
            /* all 'Highest' reservations should be slid forward regardless */
            /* of ResDepth                                          */
 
            if (MJobReserve(J,mjrPriority) == SUCCESS)
              {
              J->R->Priority  = J->StartPriority;
 
              J->R->Flags    |= (1 << mrfPreemptible);
              }
 
            break;
 
          case resNever:
          default: 
 
            MResDestroy(&J->R);
 
            break;
          }  /* END switch (ResPolicy)  */
        }    /* END if (J->R != NULL) */
      }      /* END if (J->RType == mjrPriority) */
 
    /* continue if no reservation */
 
    if (J->R == NULL)
      {
      continue;
      }
 
    /* if the time to schedule has arrived... */
 
    if ((MSched.Time >= J->R->StartTime) && (J->State == mjsIdle))
      {
      DBG(2,fSCHED) DPrint("INFO:     located job '%s' reserved to start %30s",
        J->Name,
        MULToDString((mulong *)&J->R->StartTime));
 
      /* get reservation partition information */
 
      PIndex = J->R->PtIndex;
 
      if (MJobCheckLimits(
            J,
            ptSOFT,
            &MPar[PIndex],
            (1 << mlActive),
            NULL) == FAILURE)
        {
        if (strcmp(J->ResName,J->R->Name) != 0)
          MResDestroy(&J->R);
 
        if (MSched.Mode != msmTest)
          {
          if (MSched.Mode == msmNormal)
            { 
            MOSSyslog(LOG_NOTICE,"cannot run reserved job '%s'.  job violates active policy",
              J->Name);
            }
 
          DBG(1,fSCHED) DPrint("ALERT:    cannot run reserved job '%s'.  (job violates active policy)\n",
            J->Name);
 
          MJobSetHold(
            J,
            (1 << mhDefer),
            MSched.DeferTime,
            mhrPolicyViolation,
            "reserved job violates active policy");
          }
 
        continue;
        }  /* END if (MJobCheckLimits() == FAILURE) */
 
      if (MJobSelectMNL(
            J,
            &MPar[PIndex],
            NULL,
            MNodeList,
            NodeMap,
            TRUE) == FAILURE)
        {
        /* not enough nodes to run job */
 
        DBG(2,fSCHED) DPrint("ALERT:    insufficient nodes to run reserved job '%s' on iteration %d\n",
          J->Name,
          MSched.Iteration);
 
        /* check if LL node state/adapter is source of problem */
 
        HPSAdapterCheck  = FALSE;
 
        RMNodeStateCheck = TRUE;
 
        for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
          {
          RQ = J->Req[rqindex]; 
 
          if (MRM[RQ->RMIndex].Type == mrmtLL)
            {
            if (RQ->Network == MUMAGetIndex(eNetwork,"hps_user",mVerify))
              {
              HPSAdapterCheck = TRUE;
 
              break;
              }
            }
          }    /* END for (rqindex) */
 
        HPSAdapterDelayCount = 0;
 
        RMNodeStateDelayCount = 0;
 
        for (index = 0;index < MAX_MNODE;index++)
          {
          N = MNode[index];

          if ((N == NULL) || (N->Name[0] == '\0'))
            break;
 
          if (N->Name[0] == '\1')
            continue;
 
          if ((N->PtIndex != PIndex) &&
              (PIndex != 0) &&
             !(J->Flags & (1 << mjfSpan)))
            {
            continue;
            }
 
          if ((N->EState == mnsIdle) ||
              (N->EState == mnsActive))
            {
            if (((N->RM->Type == mrmtLL) ||
                 (N->RM->Type == mrmtPBS)) &&
                 (RMNodeStateCheck == TRUE))
              {
              /* add dedicated check for running jobs */
 
              if (N->State == mnsBusy)
                { 
                DBG(6,fSCHED) DPrint("INFO:     node '%s' is not available for scheduling.  (state: %s  exp: %s)\n",
                  N->Name,
                  MAList[eNodeState][N->State],
                  MAList[eNodeState][N->EState]);
 
                RMNodeStateDelayCount++;
                }
              }
            }
 
          if ((N->RM->Type == mrmtLL) &&
               (HPSAdapterCheck == TRUE))
            {
            if (!(N->Network & MUMAGetBM(eNetwork,"hps_user",mVerify)))
              {
              DBG(6,fSCHED) DPrint("INFO:     LL node '%s' is not available for scheduling.  (missing 'hps_user' adapter)\n",
                N->Name);
 
              HPSAdapterDelayCount++;
              }
            }
          }     /* END for (index = 0;MNode[index].Name[0];index++) */
 
        if (((MSched.Time - J->R->StartTime) <
              MPar[0].ResRetryTime) &&
            ((HPSAdapterDelayCount > 0) ||
             (RMNodeStateDelayCount > 0)))
          {
          /* need to extend reservation properly */
 
          if (MSched.Mode == msmNormal)
            {
            MOSSyslog(LOG_NOTICE,"extending reservation for job %s, A: %d PS: %d",
              J->Name,
              HPSAdapterDelayCount,
              RMNodeStateDelayCount);
            }
 
          tmpL = MSched.Time + 1; 
 
          MJobSetAttr(J,mjaSysSMinTime,(void **)&tmpL,0,mAdd);
 
          DBG(4,fSCHED) DPrint("INFO:     reservation for job '%s' blocked by RM race condition.  (retrying reservation)\n",
            J->Name);
 
          if (MJobReserve(J,J->RType) == SUCCESS)
            {
            DBG(4,fSCHED) DPrint("INFO:     reservation for job '%s' recreated for time %s\n",
              J->Name,
              MULToTString(J->R->StartTime - MSched.Time));
            }
          else
            {
            DBG(4,fSCHED) DPrint("ALERT:    cannot create slide-back reservation for job '%s'\n",
              J->Name);
            }
          }  /* END if (MSched.Time...) */
        else
          {
          DBG(2,fSCHED) DPrint("ALERT:    insufficient nodes to run reserved job '%s'  (reservation released)\n",
            J->Name);
 
          if (strcmp(J->ResName,J->R->Name) != 0)
            MResDestroy(&J->R);
 
          if (MSched.Mode != msmTest)
            {
            if (MSched.Mode == msmNormal)
              {
              MOSSyslog(LOG_NOTICE,"reserved job %s cannot run.  deferring",
                J->Name);
              }
 
            MJobSetHold(
              J,
              (1 << mhDefer),
              MSched.DeferTime,
              mhrNoResources, 
              "insufficient resources to start reserved job");
            }
          }  /* END else (MSched.Time - J->R->StartTime) */
 
        continue;
        }  /* END if (MJobSelectMNL() == FAILURE) */

      RQ = J->Req[0];

      NAPolicy = (RQ->NAllocPolicy != NULL) ?
        RQ->NAllocPolicy->NAllocPolicy :
        MPar[RQ->PtIndex].NAllocPolicy;
 
      if (MJobAllocMNL(
            J,
            MNodeList,
            NodeMap,
            NULL,
            NAPolicy,
            MSched.Time) == SUCCESS)
        {
        if (MJobStart(J) == SUCCESS)
          {
          SchedCount++;
 
          DBG(2,fSCHED) DPrint("INFO:     reserved job '%s' started\n",
            J->Name);
          }
        else
          {
          DBG(1,fSCHED) DPrint("ALERT:    cannot run reserved job '%s'\n",
            J->Name);
          }
        }
      else
        {
        DBG(1,fSCHED) DPrint("ERROR:    cannot allocate nodes for job '%s'\n",
          J->Name);
        }
      }    /* END if (if (MSched.Time >= J->R->StartTime)) */
    }      /* END for (jindex)   */
 
  /* increment bypass value */
 
  if (SchedCount != 0)
    {
    DBG(2,fSCHED) DPrint("INFO:     reserved jobs scheduled: %d\n", 
      SchedCount);
    }
 
  return(SUCCESS);
  }    /* END MQueueScheduleRJobs() */


/* END MQueue.c */


