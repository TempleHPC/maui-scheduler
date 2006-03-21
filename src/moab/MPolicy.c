/* HEADER */
        
/* Contains:                                                *
 *   int MQueueSelectJobs(MJL,SJ,PL,MNC,MPC,MWCL,OPI,FR)    *
 *   int MQueueSelectAllJobs(JobList,SelectionMode)         *
 *   int MJobCheckPolicies(J,PL,PT,P,RIndex,Buffer,StartTime) *
 *   int MPolicyGetEStartTime(J,P,PLevel,EarliestTime)      *
 *   int MPolicyAdjustUsage(ResUsage,J,R,LimitType,PU,OIndex,Count,ViolationDetected) *
 *                                                          */

#include "moab.h"
#include "msched-proto.h"  

extern mlog_t      mlog;

extern const char *MPolicyMode[];
extern const char *MPolicyRejection[];
extern const char *MAllocRejType[];
extern const char *MTLimitType[];
extern const char *MXO[];
extern const char *MJobDependType[];

extern msched_t    MSched;
extern mjob_t     *MJob[];
extern mstat_t     MStat;
extern mqos_t      MQOS[];
extern mpar_t      MPar[];
extern mattrlist_t MAList;
extern mrm_t       MRM[];
extern mre_t       MRE[];
extern mres_t     *MRes[];


/* NOTE on limits:

  -1              no limit
   0              not specified (default to no limit)
   1 - INFINITY   limit value

*/

/* NYI:  must handle effqduration */


   

int MQueueSelectJobs(

  int           *SrcQ,          /* I */
  int           *DstQ,          /* O */
  int            PLevel,        /* I */
  int            MaxNC,         /* I */
  int            MaxPC,         /* I */
  unsigned long  MaxWCLimit,    /* I */
  int            OrigPIndex,    /* I */
  int           *FReason,       /* O */
  mbool_t        UpdateStats)   /* I:  (boolean) */

  {
  int      index;
  int      jindex;
  int      sindex;

  mjob_t  *J;

  char     DValue[MAX_MNAME];
  enum MJobDependEnum DType;

  mpar_t  *P;
  mpar_t  *GP;

  long     PS;

  int      LReason[MAX_MREJREASON];
  int      PReason;

  int     *Reason;

  int      PIndex;
  int      PReq;

  mreq_t  *RQ;

  double   PE;

  char     tmpLine[MAX_MLINE];

  const char *FName = "MQueueSelectJobs";

  PIndex = MAX(0,OrigPIndex);

  DBG(3,fSCHED) DPrint("%s(SrcQ,DstQ,%s,%d,%d,%ld,%s,FReason,%s)\n",
    FName,
    MPolicyMode[PLevel],
    MaxNC,
    MaxPC,
    MaxWCLimit,
    (OrigPIndex == -1) ? "EVERY" : MAList[ePartition][PIndex],
    (UpdateStats == TRUE) ? "TRUE" : "FALSE");

  /* system policies only checked in MQueueSelectAllJobs() */

  if (FReason != NULL)
    Reason = FReason;
  else
    Reason = &LReason[0];

  memset(Reason,0,sizeof(LReason));

  if ((SrcQ == NULL) || (DstQ == NULL))
    {
    DBG(1,fSCHED) DPrint("ERROR:    invalid arguments passed to %s()\n",
      FName);
 
    return(FAILURE);
    }

  /* initialize output */

  DstQ[0] = -1;

  if (SrcQ[0] == -1)
    {
    DBG(4,fSCHED) DPrint("INFO:     idle job queue is empty on iteration %d\n",
      MSched.Iteration);

    return(FAILURE);
    }

  P  = &MPar[PIndex];
  GP = &MPar[0];

  if (UpdateStats == TRUE)
    {
    MStat.IdleJobs = 0;
    }

  sindex = 0;

  /* initialize local policies */

  MLocalCheckFairnessPolicy(NULL,MSched.Time,NULL);

  for (jindex = 0;SrcQ[jindex] != -1;jindex++)
    {
    J = MJob[SrcQ[jindex]];

    if (J != NULL)
      {
      DBG(7,fSCHED) DPrint("INFO:     checking job[%d] '%s'\n",
        jindex,
        J->Name);
      }
    else
      {
      DBG(7,fSCHED) DPrint("INFO:     queue job[%d] has been removed\n",
        jindex);

      continue;
      }

    RQ = J->Req[0]; /* FIXME */

    /* if job removed */

    if (J->Name[0] == '\0')
      {
      Reason[marCorruption]++;

      continue;
      }

    if (UpdateStats == TRUE)
      {
      J->BlockReason = 0;

      if (J->State == mjsIdle)
        MStat.IdleJobs++;
      }

    PReq = MJobGetProcCount(J);
    MJobGetPE(J,P,&PE);
    PS   = (long)PReq * J->SpecWCLimit[0];

    /* check partition */

    if (OrigPIndex != -1)
      {
      if ((P->Index == 0) && !(J->Flags & (1 << mjfSpan)))
        {
        /* why?  what does partition '0' mean in partition mode? */

        DBG(3,fSCHED) DPrint("INFO:     job %s not considered for spanning\n",
          J->Name);

        Reason[marPartitionAccess]++;

        continue;
        }
      else if ((P->Index != 0) && (J->Flags & (1 << mjfSpan)))
        {
        DBG(3,fSCHED) DPrint("INFO:     spanning job %s not considered for partition scheduling\n",
          J->Name);

        Reason[marPartitionAccess]++;

        continue;
        }

      if ((P->Index > 0) && (MUBMCheck(P->Index,J->PAL) == FAILURE))
        {
        DBG(7,fSCHED) DPrint("INFO:     job %s not considered for partition %s (allowed %s)\n",
          J->Name,
          P->Name,
          MUListAttrs(ePartition,J->PAL[0]));

        Reason[marPartitionAccess]++;

        continue;
        }
      }   /* END if (OrigPIndex != -1) */

    /* check job state */

    if ((J->State != mjsIdle) && (J->State != mjsSuspended))
      {
      DBG(6,fSCHED) DPrint("INFO:     job %s rejected (job in non-idle state '%s')\n",
        J->Name,
        MJobState[J->State]);

      Reason[marState]++;

      if ((MaxNC == MAX_MNODE) && 
          (MaxWCLimit == MAX_MTIME) && 
          (J->R != NULL))
        {
        if ((J->State != mjsStarting) && (J->State != mjsRunning))
          MResDestroy(&J->R);
        }

      continue;
      }

    /* check if job has been previously scheduled or deferred */

    if ((J->EState != mjsIdle) && (J->EState != mjsSuspended))
      {
      DBG(6,fSCHED) DPrint("INFO:     job %s rejected (job in non-idle expected state: '%s')\n",
        J->Name,
        MJobState[J->EState]);

      Reason[marEState]++;

      if ((MaxNC == MAX_MNODE) && (MaxWCLimit == MAX_MTIME) && (J->R != NULL))
        {
        if ((J->EState != mjsStarting) && (J->EState != mjsRunning))
          MResDestroy(&J->R);
        }

      continue;
      }

    /* check available procs */

    if (PReq > P->CRes.Procs)
      {
      DBG(6,fSCHED) DPrint("INFO:     job %s rejected in partition %s (exceeds configured procs: %d > %d)\n",
        J->Name,
        P->Name,
        PReq,
        P->CRes.Procs);

      Reason[marNodeCount]++;

      if (P->Index <= 0)
        {
        if (J->R != NULL)
          MResDestroy(&J->R);

        if (J->Hold == 0)
          {
          MJobSetHold(
            J,
            (1 << mhDefer),
            MSched.DeferTime,
            mhrNoResources,
            "exceeds partition configured procs");
          }
        }

      continue;
      }

    /* check partition specific limits */

    if (MJobCheckLimits(
          J,
          PLevel,
          P,
          (1 << mlSystem),
          tmpLine) == FAILURE)
      {
      DBG(6,fSCHED) DPrint("INFO:     job %s rejected, partition %s (%s)\n",
        J->Name,
        P->Name,
        tmpLine);

      Reason[marSystemLimits]++;

      if (P->Index <= 0)
        {
        if (J->R != NULL)
          MResDestroy(&J->R);

        MJobSetHold(
          J,
          (1 << mhDefer),
          MSched.DeferTime,
          mhrSystemLimits,
          "exceeds system proc/job limit");
        }

      continue;
      }  /* END if (MJobCheckLimits() == FAILURE) */

    /* check job size */

    if (PReq > MaxPC)
      {
      DBG(6,fSCHED) DPrint("INFO:     job %s rejected in partition %s (exceeds window size: %d > %d)\n",
        J->Name,
        P->Name,
        PReq,
        MaxPC);

      Reason[marNodeCount]++;

      continue;
      }

    /* check job duration */

    if (J->SpecWCLimit[0] > MaxWCLimit)
      {
      DBG(6,fSCHED) DPrint("INFO:     job %s rejected in partition %s (exceeds window time: %ld > %ld)\n",
        J->Name,
        P->Name,
        J->SpecWCLimit[0],
        MaxWCLimit);

      Reason[marTime]++;

      continue;
      }

    /* check partition class support */

    if (P->Index > 0)
      {
      if (MUNumListGetCount(J->StartPriority,RQ->DRes.PSlot,P->CRes.PSlot,0,NULL) == FAILURE)
        {
        DBG(6,fSCHED) DPrint("INFO:     job %s rejected, partition %s (classes not supported '%s')\n",
          J->Name,
          P->Name,
          MUCAListToString(RQ->DRes.PSlot,P->CRes.PSlot,NULL));

        Reason[marClass]++;

        if (J->R != NULL)
          MResDestroy(&J->R);

        continue;
        }
      }      /* END if (PIndex) */

    if (MJobCheckDependency(J,&DType,DValue) == FAILURE)
      {
      DBG(6,fSCHED) DPrint("INFO:     job %s rejected (dependent on job '%s' %s)\n",
        J->Name,
        DValue,
        MJobDependType[DType]);

      if (GP->JobPrioAccrualPolicy == jpapFullPolicy)
        {
        J->SystemQueueTime = MSched.Time;
        }

      Reason[marDepend]++;

      if ((MaxNC == MAX_MNODE) &&
          (MaxWCLimit == MAX_MTIME) &&
          (J->R != NULL))
        {
        MResDestroy(&J->R);
        }

      continue;
      }  /* END if (MJobCheckDependency(J,&JDepend) == FAILURE) */

    /* check partition active job policies */

    if (MJobCheckPolicies(
          J,
          PLevel,
          (1 << mlActive),
          P,   /* NOTE:  may set to &MPar[0] */
          &PReason,
          NULL,
          MAX_MTIME) == FAILURE)
      {
      DBG(6,fSCHED) DPrint("INFO:     job %s rejected, partition %s (policy failure: '%s')\n",
        J->Name,
        P->Name,
        MPolicyRejection[PReason]);

      if (PLevel == ptHARD)
        {
        if (GP->JobPrioAccrualPolicy == jpapFullPolicy)
          {
          J->SystemQueueTime = MSched.Time;
          }
        }

      Reason[marPolicy]++;

      if ((MaxNC == MAX_MNODE) && 
          (MaxWCLimit == MAX_MTIME) && 
          (J->R != NULL))
        {
        MResDestroy(&J->R);
        }

      continue;
      }

    J->Cred.U->MTime = MSched.Time;
    J->Cred.G->MTime = MSched.Time;

    if (J->Cred.A != NULL)
      J->Cred.A->MTime = MSched.Time;

    if (MPar[0].FSC.FSPolicy != fspNONE)
      {
      int OIndex;

      if (MFSCheckCap(NULL,J,P,&OIndex) == FAILURE)
        {
        DBG(5,fSCHED) DPrint("INFO:     job '%s' exceeds %s FS cap\n",
          J->Name,
          (OIndex > 0) ? MXO[OIndex] : "NONE");
 
        if (GP->JobPrioAccrualPolicy == jpapFullPolicy)
          {
          J->SystemQueueTime = MSched.Time;
          }
 
        Reason[marFairShare]++;

        continue;
        }
      }    /* END if (FS[0].FSPolicy != fspNONE) */

    /* NOTE:  idle queue policies handled in MQueueSelectAllJobs() */

    if (MLocalCheckFairnessPolicy(J,MSched.Time,NULL) == FAILURE)
      {
      DBG(6,fSCHED) DPrint("INFO:     job %s rejected, partition %s (violates local fairness policy)\n",
        J->Name,
        P->Name);

      if (GP->JobPrioAccrualPolicy == jpapFullPolicy) 
        {
        J->SystemQueueTime = MSched.Time;
        }

      Reason[marPolicy]++;

      continue;
      }

    /* NOTE:  effective queue duration not yet properly supported */

    J->EffQueueDuration = (MSched.Time > J->SystemQueueTime) ? 
      MSched.Time - J->SystemQueueTime : 0;
 
    /* add job to destination queue */

    DBG(5,fSCHED) DPrint("INFO:     job '%s' added to queue at slot %d\n",
      J->Name,
      sindex);

    DstQ[sindex++] = SrcQ[jindex];
    }  /* END for (jindex) */

  /* terminate list */

  DstQ[sindex] = -1;

  DBG(1,fSCHED)
    {
    DBG(1,fSCHED) DPrint("INFO:     total jobs selected in partition %s: %d/%-d ",
      MAList[ePartition][PIndex],
      sindex,
      jindex);

    for (index = 0;index < MAX_MREJREASON;index++)
      {
      if (Reason[index] != 0)
        {
        fprintf(mlog.logfp,"[%s: %d]",
          MAllocRejType[index],
          Reason[index]);
        }
      }    /* END for (index) */

    fprintf(mlog.logfp,"\n");
    }

  if (sindex == 0)
    return(FAILURE);

  return(SUCCESS);
  }  /* END MQueueSelectJobs() */





int MQueueSelectAllJobs(

  mjob_t **Q,            /* I:  feasible job list */
  int      PLevel,       /* I:  policy level */
  mpar_t  *P,
  int     *JIList,       /* O:  idle job list */
  int      DoPrioritize, /* I:  boolean */
  int      UpdateStats,  /* I:  boolean */
  int      TrackIdle,    /* I:  boolean */
  char    *Msg)          /* O:  optional */

  {
  int      JIndex;
  int      jindex;

  int      Reason[MAX_MREJREASON];
  char     Message[MAX_MLINE];

  int      JobCount;

  mjob_t  *J;

  int      tmpJIList[MAX_MJOB];

  const char *FName = "MQueueSelectAllJobs";

  DBG(3,fSCHED) DPrint("%s(%s,%s,%s,%s,DP,Msg)\n",
    FName,
    (Q != NULL) ? "Q" : "NULL",
    MPolicyMode[PLevel],
    (P != NULL) ? P->Name : "NULL",
    (JIList != NULL) ? "JIList" : "NULL");

  if (JIList == NULL)
    {
    return(FAILURE);
    }

  if ((P == NULL) || 
      (Q == NULL) || 
      ((DoPrioritize == TRUE) && ((Q[0] == NULL) || 
      (Q[0]->Next == NULL) ||
      (Q[0]->Next == Q[0]))))
    {
    if (Msg != NULL)
      strcpy(Msg,"queue is corrupt");

    return(FAILURE);
    }

  /* NOTE:  prioritize all feasible idle/active jobs */
  /*        sort jobs by priority                    */
  /*        apply queue policies                     */
  /*        record eligible jobs in JobList          */
  /*        adjust system queue time                 */

  if (DoPrioritize == TRUE)
    {
    /* prioritize/sort queue */

    JIList[0] = -1;

    MQueuePrioritizeJobs(Q,tmpJIList);
    }
  else
    {
    memcpy(tmpJIList,JIList,sizeof(tmpJIList));
    }

  memset(Reason,0,sizeof(Reason));

  /* initialize throttling policy usage */

  if (UpdateStats == TRUE)
    {
    MStatClearUsage(0,(1 << mlIdle)|(1 << mlActive),FALSE);
    }
  else if (TrackIdle == TRUE)
    {
    /* NOTE:  update idle job stats */

    MStatClearUsage(0,(1 << mlIdle),TRUE);
    }

  JobCount = 0;
  JIndex   = 0;

  /* apply state/queue policies */

  for (jindex = 0;tmpJIList[jindex] != -1;jindex++)
    {
    J = Q[tmpJIList[jindex]];

    if ((J == NULL) || (J->Name[0] == '\0') || (J->Name[0] == '\1'))
      continue;

    JobCount++;

    if (UpdateStats == TRUE)
      {
      J->BlockReason = 0;

      if (J->SysFlags & (1 << mjfSPViolation))
        J->SysFlags ^= (1 << mjfSPViolation);
      }

    /* check job state */

    if ((J->State != mjsIdle) && (J->State != mjsSuspended))
      {
      if (Msg != NULL)
        {
        sprintf(Msg,"non-idle state '%s'",
          MJobState[J->State]);
        }
 
      DBG(6,fSCHED) DPrint("INFO:     job %s rejected (job in non-idle state '%s')\n",
        J->Name,
        MJobState[J->State]);

      Reason[marState]++;

      if ((J->State != mjsStarting) && (J->State != mjsRunning))
        {
        MResDestroy(&J->R);
        }
      else if (J->EState != mjsDeferred)
        {
        /* adjust partition and credential based active use statistics */

        /* determine if job violates 'soft' policies' */
         
        if (UpdateStats == TRUE)
          {
          if (MJobCheckLimits(
                J,
                ptSOFT,
                P,
                (1 << mlActive),
                Message) == FAILURE)
            {
            /* job violates 'soft' active policy */

            J->SysFlags |= (1 << mjfSPViolation);

            MJobUpdateFlags(J);
            }

          MPolicyAdjustUsage(NULL,J,NULL,mlActive,&MPar[0].L.AP,-1,1,NULL);
          MPolicyAdjustUsage(NULL,J,NULL,mlActive,NULL,-1,1,NULL);
          }
        }    /* END else if (J->EState != mjsDeferred) */

      continue;
      }      /* END if (J->State != mjsIdle) */

    /* check if job has been previously scheduled or deferred */

    if ((J->EState != mjsIdle) && (J->EState != mjsSuspended))
      {
      if (Msg != NULL)
        {
        sprintf(Msg,"non-idle expected state '%s'",
          MJobState[J->EState]);
        }

      DBG(6,fSCHED) DPrint("INFO:     job %s rejected (job in non-idle expected state: '%s')\n",
        J->Name,
        MJobState[J->EState]);

      Reason[marEState]++;

      if ((J->EState != mjsStarting) && (J->EState != mjsRunning))
        MResDestroy(&J->R);

      continue;
      }

    /* check user/system/batch hold */
 
    if (J->Hold != 0)
      {
      if (Msg != NULL)
        {
        strcpy(Msg,"job hold active");
        }

      DBG(6,fSCHED) DPrint("INFO:     job %s rejected (hold on job)\n",
        J->Name);
 
      if (MPar[0].JobPrioAccrualPolicy == jpapFullPolicy)
        {
        J->SystemQueueTime = MSched.Time;
        }
 
      Reason[marHold]++;
 
      if (J->R != NULL)
        MResDestroy(&J->R);
 
      continue;
      }

    /* check class state */

    if ((J->Cred.C != NULL) && (J->Cred.C->IsDisabled == TRUE))
      {
      if (Msg != NULL)
        {
        strcpy(Msg,"class not enabled");
        }

      DBG(6,fSCHED) DPrint("INFO:     job %s rejected (class not enabled)\n",
        J->Name);

      if (MPar[0].JobPrioAccrualPolicy == jpapFullPolicy)
        {
        J->SystemQueueTime = MSched.Time;
        }

      Reason[marClass]++;

      if (J->R != NULL)
        MResDestroy(&J->R);

      continue;
      }

    /* check priority */

    if ((MPar[0].RejectNegPrioJobs == TRUE) && (J->StartPriority < 0))
      {
      if (Msg != NULL)
        {
        strcpy(Msg,"negative priority");
        }

      DBG(6,fSCHED) DPrint("INFO:     job %s rejected (negative priority)\n",
        J->Name);

      if (MPar[0].JobPrioAccrualPolicy == jpapFullPolicy)
        {
        J->SystemQueueTime = MSched.Time;
        }

      Reason[marPriority]++;

      if (J->R != NULL)
        MResDestroy(&J->R);

      continue;
      }

    /* check startdate */
 
    if (J->SMinTime > MSched.Time)
      {
      if (Msg != NULL)
        {
        sprintf(Msg,"startdate in '%s'",
          MULToTString(J->SMinTime - MSched.Time));
        }

      DBG(6,fSCHED) DPrint("INFO:     job %s rejected (release time violation: %ld > %ld)\n",
        J->Name,
        J->SMinTime,
        MSched.Time);
 
      Reason[marRelease]++;
 
      if (J->R != NULL)
        MResDestroy(&J->R);
 
      continue;
      }  /* END if (J->SMinTime > MSched.Time) */

    /* check masterjob */
 
    if (J->MasterJobName != NULL)
      {
      if (Msg != NULL)
        {
        sprintf(Msg,"job is subjob to '%s'",
          J->MasterJobName);
        }

      DBG(6,fSCHED) DPrint("INFO:     job %s rejected (job is subjob to '%s')\n",
        J->Name,
        J->MasterJobName);
 
      if (MPar[0].JobPrioAccrualPolicy == jpapFullPolicy)
        {
        J->SystemQueueTime = MSched.Time;
        }
 
      Reason[marRelease]++;
 
      if (J->R != NULL)
        MResDestroy(&J->R);
 
      continue;
      }

    if (UpdateStats == TRUE)
      {
      MStat.IdleJobs++;
      }

    /* check available procs */

    if (MJobGetProcCount(J) > MPar[0].URes.Procs)
      {
      if (Msg != NULL)
        {
        sprintf(Msg,"inadequate procs in system: %d < %d",
          MJobGetProcCount(J),
          MPar[0].URes.Procs);
        }

      DBG(6,fSCHED) DPrint("INFO:     job %s rejected, (exceeds available procs in system: %d > %d)\n",
        J->Name,
        MJobGetProcCount(J),
        MPar[0].URes.Procs);

      Reason[marNodeCount]++;

      if (J->R != NULL)
        MResDestroy(&J->R);

      MJobSetHold(
        J,
        (1 << mhDefer),
        MSched.DeferTime,
        mhrNoResources,
        "exceeds available partition procs");

      continue;
      }

    /* check queue policies */

    /* only non-reserved jobs affected by active/idle/system policies */
 
    if ((J->R == NULL) && (MJobCheckLimits(
          J,
          PLevel,
          P,
          (1 << mlActive) | (1 << mlIdle) | (1 << mlSystem),
          Message) == FAILURE))
      {
      if (Msg != NULL)
        {
        sprintf(Msg,"%s",
          Message);
        }

      DBG(6,fSCHED) DPrint("INFO:     job %s rejected, partition %s (%s)\n",
        J->Name,
        P->Name,
        Message);
 
      if (PLevel == ptHARD)
        {
        if ((MPar[0].JobPrioAccrualPolicy == jpapFullPolicy) ||
            (MPar[0].JobPrioAccrualPolicy == jpapQueuePolicy))
          {
          J->SystemQueueTime = MSched.Time;
          }
        }

      if (UpdateStats == TRUE)
        J->BlockReason = mjneIdlePolicy;
 
      Reason[marPolicy]++;
 
      continue;
      }

    /* adjust partition and credential idle job statistics */

    if ((UpdateStats == TRUE) || (TrackIdle == TRUE))
      {
      MStatAddEJob(J);
      }

    DBG(7,fSCHED) DPrint("INFO:     job[%02d] '%s' added to master list\n",
      JIndex,
      J->Name);

    JIList[JIndex++] = J->Index;
    }  /* END for (jindex) */

  JIList[JIndex] = -1;

  DBG(1,fSCHED)
    {
    int findex;

    DBG(1,fSCHED) DPrint("INFO:     total jobs selected (ALL): %d/%-d ",
      JIndex,
      JobCount);

    for (findex = 0;findex < MAX_MREJREASON;findex++)
      {
      if (Reason[findex] != 0)
        {
        fprintf(mlog.logfp,"[%s: %d]",
          MAllocRejType[findex],
          Reason[findex]);
        }
      }    /* END for (findex) */

    fprintf(mlog.logfp,"\n");
    }

  DBG(4,fSCHED)
    {
    DBG(4,fSCHED) DPrint("INFO:     jobs selected:\n");

    for (jindex = 0;JIList[jindex] != -1;jindex++)
      {
      fprintf(mlog.logfp,"[%03d: %3d]",
        jindex,
        JIList[jindex]);
      }  /* END for (jindex) */

    fprintf(mlog.logfp,"\n");
    }

  return(SUCCESS);
  }  /* END MQueueSelectAllJobs() */





int MJobCheckPolicies(

  mjob_t *J,           /* IN:  job                         */
  int     PLevel,      /* IN:  level of policy enforcement */
  int     PType,
  mpar_t *P,           /* IN:  node partition              */
  int    *RIndex,      /* OUT: reason job was rejected     */
  char   *Buffer,      /* OUT: description of rejection    */
  long    StartTime)   /* IN:  time job will start         */

  {
  const char *FName = "MJobCheckPolicies";

  DBG(5,fSCHED) DPrint("%s(%s,%s,%d,%s,RIndex,%s,%ld)\n",
    FName,
    J->Name,
    MPolicyMode[PLevel],
    PType, 
    (P != NULL) ? P->Name : "NULL",
    (Buffer == NULL) ? "NULL" : "Buffer",
    StartTime);

  if ((J == NULL) || (P == NULL))
    return(FAILURE);

  if (MJobCheckLimits(
        J,
        PLevel,
        P,
        (PType != 0) ? PType : (1 << mlActive)|(1 << mlIdle)|(1 << mlSystem),
        Buffer) == FAILURE)  
    {
    /* job limit rejection */

    if (RIndex != NULL)
      *RIndex = prMaxJobPerUserPolicy;

    return(FAILURE);
    }

  if (Buffer != NULL)
    {
    sprintf(Buffer,"job %s passes all policies in partition %s\n",
      J->Name,
      P->Name);

    DBG(4,fSCHED) DPrint("INFO:     %s",
      Buffer);
    }

  return(SUCCESS);
  }  /* END MJobCheckPolicies() */




int MPolicyCheckLimit(

  int    UsageDelta,  /* I */
  int    PlIndex,     /* I */
  int    PLevel,      /* I */
  int    PtIndex,
  mpu_t *ObjectP,
  mpu_t *DefaultP,
  mpu_t *QOSP,
  int   *LimitP)

  {
  int Usage;
  int Limit;

  /* precedence: QOSP -> ObjectP -> DefaultP */

  if ((QOSP != NULL) && (QOSP->HLimit[PlIndex][PtIndex] != 0))
    {
    /* use QOS */

    Limit = (PLevel == ptSOFT) ? 
      QOSP->SLimit[PlIndex][PtIndex] : 
      QOSP->HLimit[PlIndex][PtIndex];
    }
  else if ((ObjectP != NULL) && (ObjectP->HLimit[PlIndex][PtIndex] != 0))  
    {
    /* use object */

    Limit = (PLevel == ptSOFT) ?
      ObjectP->SLimit[PlIndex][PtIndex] :
      ObjectP->HLimit[PlIndex][PtIndex];
    }
  else if ((DefaultP != NULL) && (DefaultP->HLimit[PlIndex][PtIndex] != 0))  
    {
    /* use default object */

    Limit = (PLevel == ptSOFT) ?
      DefaultP->SLimit[PlIndex][PtIndex] :
      DefaultP->HLimit[PlIndex][PtIndex];
    }
  else
    {
    /* no limit specified */

    Limit = 0;
    }

  Usage = UsageDelta;

  if (ObjectP != NULL)
    Usage += ObjectP->Usage[PlIndex][PtIndex];
 
  if (LimitP != NULL)
    *LimitP = Limit;

  if (Limit > 0)
    {
    switch(PlIndex)
      {
      case mptMinProc:

        if (Usage < Limit)
          {
          return(FAILURE);
          }

        break;

      default:

        if (Usage > Limit)
          {
          return(FAILURE);
          }

        break;
      }  /* END switch(PlIndex) */
    }    /* END if (Limit > 0)  */
 
  return(SUCCESS);
  }  /* END MPolicyCheckLimit() */




int MPolicyCheckJob(

  mjob_t *J,
  long    StartTime,
  int    *RIndex,
  char   *Message)
  
  {
  /* NYI */

  return(SUCCESS);
  }  /* END MPolicyCheckJob() */





int MPolicyInsertEvent(

  mres_t *R)

  {
  /* NYI */

  return(SUCCESS);
  }  /* END PolicyInsertEvent() */




int MPolicyRemoveEvent(

  mres_t *R)

  {
  /* NYI */

  return(SUCCESS);
  }  /* END PolicyRemoveEvent() */





int MPolicyGetEStartTime(

  mjob_t    *J,       /* I */
  mpar_t    *P,       /* I */
  int        PLevel,  /* I */
  long      *Time)    /* O */

  {
  int     rindex;
  int     aindex;

  long    AvailStart;
  long    JobDuration;
 
  mres_t *R;

  mpar_t *GP;
  
  mpu_t   PAvailable[mxoLAST];

  int     PConsumed[MAX_MPOLICY];

  int     oindex;
  int     pindex;

  int     OList[] = { mxoUser, mxoGroup, mxoAcct, mxoQOS, mxoClass, -1 };
  int     PList[] = { mptMaxJob, mptMaxProc, mptMaxPS, mptMaxPE, mptMaxNode, -1 };

  int     ViolationDetected[mxoLAST];

  char    JobAttrs[mxoLAST][MAX_MNAME];

  int     TrackDynamic = FALSE;

  mpu_t  *OP;
  mpu_t  *DP;
  mpu_t  *QP;
 
  struct {
    mres_t *R;
    long    StartTime;
    int     Procs;
    } ActiveR[MAX_MRES];

  int ZList[MAX_MPOLICY][MAX_MPAR];
 
  const char *FName = "MPolicyGetEStartTime";

  DBG(4,fSCHED) DPrint("%s(%s,%s,%s,Time)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (P != NULL) ? P->Name : "NULL",
    MPolicyMode[PLevel]);     

  if ((J == NULL) || (Time == NULL))
    {
    return(FAILURE);
    }

  /* NOTE:  must incorporate two-dimensional throttling policies    */
  /* NOTE:  all reservation EStart estimates based on 'soft' limits */

  GP = &MPar[0];

  JobDuration = J->SpecWCLimit[0];
  AvailStart  = MAX(*Time,MSched.Time);
  *Time       = MAX_MTIME;

  memset(ViolationDetected,FALSE,sizeof(ViolationDetected));

  /* initialize credentials */

  memset(&JobAttrs,0,sizeof(JobAttrs));

  memset(&PAvailable,0,sizeof(PAvailable));             
  memset(&PConsumed,0,sizeof(PConsumed));       

  memset(&ZList,0,sizeof(ZList));

  /* record job consumption */

  PConsumed[mptMaxJob]  = 1;
  PConsumed[mptMaxProc] = MJobGetProcCount(J);
  PConsumed[mptMaxWC]   = J->SpecWCLimit[0];
  PConsumed[mptMaxPS]   = PConsumed[mptMaxProc] * PConsumed[mptMaxWC];

  /* determine all hard limits */             

  if (J->Cred.Q != NULL)
    QP = J->Cred.Q->L.OAP;
  else
    QP = NULL;

  for (oindex = 0;OList[oindex] != -1;oindex++)
    {
    OP = NULL;
    DP = NULL;

    switch(OList[oindex])
      {
      case mxoUser:

        if (J->Cred.U != NULL)
          OP = &J->Cred.U->L.AP;

        if (MSched.DefaultU != NULL)
          DP = &MSched.DefaultU->L.AP;

        break;

      case mxoGroup:

        if (J->Cred.G != NULL)
          OP = &J->Cred.G->L.AP;
 
        if (MSched.DefaultG != NULL)
          DP = &MSched.DefaultG->L.AP;

        break;

      case mxoAcct:

        if (J->Cred.A != NULL)
          OP = &J->Cred.A->L.AP;
 
        if (MSched.DefaultA != NULL)
          DP = &MSched.DefaultA->L.AP;

        break;

      case mxoQOS:

        if (J->Cred.Q != NULL)
          OP = &J->Cred.Q->L.AP;
 
        if (MSched.DefaultQ != NULL)
          DP = &MSched.DefaultQ->L.AP;

        break;

      case mxoClass:

        if (J->Cred.C != NULL)
          OP = &J->Cred.C->L.AP;
 
        if (MSched.DefaultC != NULL)
          DP = &MSched.DefaultC->L.AP;

        break;
      }  /* END switch (OList[oindex]) */

    for (pindex = 0;PList[pindex] != -1;pindex++)
      {
      MPolicyCheckLimit(
        PConsumed[PList[pindex]],
        PList[pindex],
        PLevel,
        0,
        OP,
        DP,
        QP,
        &PAvailable[OList[oindex]].HLimit[PList[pindex]][0]);
     
      /* limit loaded into PAvailable */

      if (PAvailable[OList[oindex]].HLimit[PList[pindex]][0] > 0)
        {
        /* if limit set, reduce limit by job consumption */

        PAvailable[OList[oindex]].Usage[PList[pindex]][0] =
          PAvailable[OList[oindex]].HLimit[PList[pindex]][0] -  
          PConsumed[PList[pindex]];
 
        if ((PList[pindex] == mptMaxPS) || 
            (PList[pindex] == mptMaxWC))
          {
          TrackDynamic = TRUE;
          }
        }
      }  /* END for (pindex) */
    }    /* END for (oindex) */

  if (J->Cred.U != NULL)
    strcpy(JobAttrs[mxoUser],J->Cred.U->Name);

  if (J->Cred.G != NULL)
    strcpy(JobAttrs[mxoGroup],J->Cred.G->Name);          

  if (J->Cred.A != NULL)
    strcpy(JobAttrs[mxoAcct],J->Cred.A->Name);          

  if (J->Cred.Q != NULL)
    strcpy(JobAttrs[mxoQOS],J->Cred.Q->Name);          

  if (J->Cred.C != NULL)
    strcpy(JobAttrs[mxoClass],J->Cred.C->Name);

  if (TrackDynamic == TRUE)
    {
    memset(ActiveR,0,sizeof(ActiveR));        
    }

  /* step through reservation events */

  for (rindex = 0;rindex < (MAX_MRES << 2);rindex++)
    {
    if (MRE[rindex].Type == mreNONE)
      break;

    if (MRE[rindex].Index < 0)
      continue;

    if ((AvailStart > 0) &&
       ((MRE[rindex].Time - AvailStart) >= JobDuration))
      {
      /* adequate slot located */
 
      *Time = AvailStart;

      DBG(4,fSCHED) DPrint("INFO:     adequate policy slot located at time %s for job %s\n",
        MULToTString(AvailStart - MSched.Time),
        J->Name);
 
      return(SUCCESS);
      }

    R = MRes[MRE[rindex].Index];

    if ((R == NULL) || (R->Name[0] == '\0') || (R->Name[0] == '\1'))
      continue;

    if ((R->CL == NULL) || (R->Type != mrtJob))
      continue;

    /* NOTE:  must add support for user reservation tracking based *
     * on accountable credentials */
 
    /* step through CL */

    for (aindex = 0;aindex < MAX_MACL;aindex++)
      {
      if (R->CL[aindex].Name[0] == '\0')
        break;

      if (strcmp(R->CL[aindex].Name,JobAttrs[(int)R->CL[aindex].Type]) != 0)
        continue;

      if (!memcmp(PAvailable[(int)R->CL[aindex].Type].HLimit,ZList,sizeof(ZList)))
        {
        /* no applicable limits set */

        continue;
        }

      if (MRE[rindex].Type == mreStart)
        {
        /* must handle dynamic limits on a per reservation */

        /* NOTE:  decrease PS usage for all jobs not completing at time T by time dT */

        if (TrackDynamic == TRUE)
          {
          /* NYI */
          }

        MPolicyAdjustUsage(
          NULL,
          NULL,
          R,
          mlActive,
          PAvailable,
          R->CL[aindex].Type,
          -1,
          &ViolationDetected[(int)R->CL[aindex].Type]);
        }  /* END if (MRE[rindex].Type == mreStart) */
      else
        {
        /* must handle dynamic procsecond usage tracking */            

        MPolicyAdjustUsage(
          NULL,
          NULL,
          R,
          mlActive,
          PAvailable,
          R->CL[aindex].Type,
          1,
          &ViolationDetected[(int)R->CL[aindex].Type]);
        }  /* END else (MRE[rindex].Type == mreStart) */

      /* do we check each iteration or only when moving to new time? */

      if (memchr(ViolationDetected,TRUE,sizeof(ViolationDetected)))
        {
        /* policy violation detected */              

        AvailStart = 0;
        }
      else 
        {
        /* no policy violation detected */

        if (AvailStart == 0)
          AvailStart = MAX(MSched.Time,MRE[rindex].Time);
        }  /* END else (ViolationDetected == TRUE) */
      }    /* END for (aindex) */
    }      /* END for (rindex) */

  if (AvailStart > 0)
    {
    *Time = AvailStart;

    DBG(4,fSCHED) DPrint("INFO:     policy start time found for job %s in %s\n",
      J->Name,
      MULToTString(AvailStart - MSched.Time));

    return(SUCCESS);
    }
 
  DBG(4,fSCHED) DPrint("INFO:     no policy start time found for job %s\n",
    J->Name);
 
  return(FAILURE);
  }  /* END MPolicyGetEStartTime() */






int MPolicyAdjustUsage(

  int    *ResUsage,          /* IN:     resource usage */
  mjob_t *J,                 /* IN/OUT: job            */
  mres_t *R,                 /* IN:     reservation    */
  int     LimitType,         /* IN:     limit type     */
  mpu_t  *PU,                /* OUT:    policy usage   */
  int     OIndex,            /* IN:     object index   */
  int     Count,             /* IN:     consumer count */
  int    *ViolationDetected) /* OUT:    boolean        */

  {
  int PIndex;

  int PConsumed[MAX_MPOLICY];

  int oindex1;
  int oindex2;

  int pindex;

  mpu_t *tPU[mxoLAST][mxoLAST];

  double PE;

  const char *FName = "MPolicyAdjustUsage";

  DBG(4,fSCHED) DPrint("%s(%s,%s,%s,%s,%s,%s,%d,%s)\n",
    FName,
    (ResUsage != NULL) ? "ResUsage" : "NULL",
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL",
    MTLimitType[LimitType],
    (PU != NULL) ? "PU" : "NULL",
    (OIndex >= 0) ? MXO[OIndex] : ALL,
    Count,
    (ViolationDetected != NULL) ? "ViolationDetected" : "NULL");

  /* adjust all policies */

  PIndex = 0;

  /* handle object consumption */    

  if (ResUsage != NULL)
    {
    memcpy(PConsumed,ResUsage,sizeof(PConsumed));
    }
  else if (J != NULL)
    {
    memset(PConsumed,0,sizeof(PConsumed));        

    if ((J->State == mjsStarting) || (J->State == mjsRunning))
      PConsumed[mptMaxWC] = J->RemainingTime;
    else
      PConsumed[mptMaxWC] = J->WCLimit;

    PConsumed[mptMaxJob]  = 1;    
    PConsumed[mptMaxProc] = MJobGetProcCount(J);
    PConsumed[mptMaxNode] = J->Request.NC;
    PConsumed[mptMaxPS]   = PConsumed[mptMaxProc] * PConsumed[mptMaxWC];

    MJobGetPE(J,&MPar[0],&PE);

    PConsumed[mptMaxPE] = (int)PE;

    if (PU == NULL) 
      PIndex = J->Req[0]->PtIndex;
    }
  else if (R != NULL)
    {
    memset(PConsumed,0,sizeof(PConsumed));           

    PConsumed[mptMaxJob]  = 1;
    PConsumed[mptMaxProc] = R->AllocPC;
    PConsumed[mptMaxNode] = R->NodeCount;
    PConsumed[mptMaxWC]   = R->EndTime - R->StartTime;
    PConsumed[mptMaxPE]   = R->AllocPC;
    PConsumed[mptMaxPS]   = PConsumed[mptMaxProc] * PConsumed[mptMaxWC];       
    }
  else
    {
    return(FAILURE);
    }                       

  /* add consumption state */

  memset(tPU,0,sizeof(tPU));

  if (PU == NULL)
    {
    switch(LimitType)
      {
      case mlIdle:

        if (J->Cred.U != NULL)
          tPU[mxoNONE][mxoUser] = J->Cred.U->L.IP;

        if (J->Cred.G != NULL)
          tPU[mxoNONE][mxoGroup] = J->Cred.G->L.IP;

        if (J->Cred.A != NULL)
          tPU[mxoNONE][mxoAcct] = J->Cred.A->L.IP;

        if (J->Cred.Q != NULL)
          tPU[mxoNONE][mxoQOS] = J->Cred.Q->L.IP;

        if (J->Cred.C != NULL)
          tPU[mxoNONE][mxoClass] = J->Cred.C->L.IP;

        break;

      case mlActive:
      default:
 
        if (J->Cred.U != NULL)
          {
          if ((J->Cred.U->L.APC != NULL) && (J->Cred.C != NULL))
            tPU[mxoClass][mxoUser] = &J->Cred.U->L.APC[J->Cred.C->Index];

          if ((J->Cred.U->L.APQ != NULL) && (J->Cred.Q != NULL))
            tPU[mxoQOS][mxoUser] = &J->Cred.U->L.APQ[J->Cred.Q->Index];

          if ((J->Cred.U->L.APG != NULL) && (J->Cred.G != NULL))
            tPU[mxoGroup][mxoUser] = &J->Cred.U->L.APG[J->Cred.G->Index];

          tPU[mxoNONE][mxoUser] = &J->Cred.U->L.AP;
          }
 
        if (J->Cred.G != NULL)
          {
          tPU[mxoNONE][mxoGroup] = &J->Cred.G->L.AP;

          if ((J->Cred.G->L.APC != NULL) && (J->Cred.C != NULL))
            tPU[mxoClass][mxoGroup] = &J->Cred.G->L.APC[J->Cred.C->Index];

          if ((J->Cred.G->L.APQ != NULL) && (J->Cred.Q != NULL))
            tPU[mxoQOS][mxoGroup] = &J->Cred.G->L.APQ[J->Cred.Q->Index];
          }
 
        if (J->Cred.A != NULL)
          tPU[mxoNONE][mxoAcct] = &J->Cred.A->L.AP;
 
        if (J->Cred.Q != NULL)
          {
          tPU[mxoNONE][mxoQOS] = &J->Cred.Q->L.AP;

          if ((J->Cred.Q->L.APU != NULL) && (J->Cred.U != NULL))
            tPU[mxoUser][mxoQOS] = &J->Cred.Q->L.APU[J->Cred.U->Index];
          }
 
        if (J->Cred.C != NULL)
          {
          tPU[mxoNONE][mxoClass] = &J->Cred.C->L.AP;

          if ((J->Cred.C->L.APU != NULL) && (J->Cred.U != NULL))
            tPU[mxoUser][mxoClass] = &J->Cred.C->L.APU[J->Cred.U->Index];  
          }
 
        break;
      }  /* END switch(PolicyType) */
    }
  else
    {
    tPU[mxoNONE][1] = PU;
    }  /* END else (PU == NULL) */

  if (ViolationDetected != NULL)
    {
    *ViolationDetected = FALSE;
    }

  /* adjust consumption state */

  for (oindex1 = 0;oindex1 < mxoLAST;oindex1++)
    {
    for (oindex2 = 1;oindex2 < mxoLAST;oindex2++)
      {
      if (tPU[oindex1][oindex2] == NULL)
        {
        continue;
        }

      if (((PU == NULL) && (OIndex >= 0) && (OIndex != oindex2)) ||
          ((PU != NULL) && (oindex1 != mxoNONE) && (oindex2 != 1)))
        {
        /* ignore invalid/non-specified limits */

        continue;
        }

      for (pindex = 0;pindex < MAX_MPOLICY;pindex++)
        {
        tPU[oindex1][oindex2]->Usage[pindex][0] += Count * PConsumed[pindex];  
        }

      if (PIndex != 0)
        {
        for (pindex = 0;pindex < MAX_MPOLICY;pindex++)
          {
          tPU[oindex1][oindex2]->Usage[pindex][PIndex] += Count * PConsumed[pindex];
          }
        }

      /* determine if policy violation occurred */

      if (ViolationDetected != NULL)
        {
        for (pindex = 0;pindex < MAX_MPOLICY;pindex++)
          {
          if (tPU[oindex1][oindex2]->Usage[pindex][0] < 0)
            {
            *ViolationDetected = TRUE;

            break;
            }
          }                         

        if ((PIndex > 0) && (*ViolationDetected != TRUE))
          {
          for (pindex = 0;pindex < MAX_MPOLICY;pindex++)
            {
            if (tPU[oindex1][oindex2]->Usage[pindex][PIndex] < 0)
              {
              *ViolationDetected = TRUE;
 
              break;
              }
            }
          }
        }  /* END if (ViolationDetected != NULL) */
      }    /* END for (oindex2) */
    }      /* END for (oindex1) */

  return(SUCCESS);
  }  /* END MPolicyAdjustUsage() */

/* END MPolicy.c */

