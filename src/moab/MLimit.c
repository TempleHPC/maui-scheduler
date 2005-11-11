/* HEADER */
        
/* Contains:                                 *
 *  int MLimitEnforceAll(PLimit)             *
 *                                           */


#include "moab.h"
#include "msched-proto.h"  

extern mlog_t      mlog;

extern msched_t    MSched;
extern mjob_t     *MJob[];
extern mattrlist_t MAList;

extern const char *MResourceType[];
extern const char *MPolicyAction[];





int MLimitEnforceAll(

  mpar_t *P) /* I */

  {
  long    JobWCX;

  mjob_t *J;
  mreq_t *RQ;

  int     ResourceLimitsExceeded;  /* boolean */

  int     VRes = -1;
  int     VLimit = -1;
  int     VVal = -1;

  int     rc;

  char    tmpMsg[MAX_MLINE];

  const char *FName = "MLimitEnforceAll";

  DBG(4,fSCHED) DPrint("%s(%s)\n",
    FName,
    MAList[ePartition][P->Index]);

  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    if ((J->State != mjsStarting) &&
        (J->State != mjsRunning))
      {
      continue;
      }

    /* enforce wallclock limits */

    JobWCX = MSched.JobMaxOverrun;
 
    if ((J->Cred.C != NULL) && (J->Cred.C->F.Overrun > 0))
      JobWCX = J->Cred.C->F.Overrun;
 
    if ((JobWCX >= 0) &&
        (J->WCLimit > 0) &&
        (MSched.Time > J->StartTime) &&
       ((unsigned long)(MSched.Time - J->StartTime) > (J->WCLimit + J->SWallTime + JobWCX)))
      {
      DBG(2,fCORE) DPrint("ALERT:    job '%s' in state '%s' has exceeded its wallclock limit (%ld+S:%ld) by %s (job will be cancelled)\n",
        J->Name,
        MJobState[J->State],
        J->WCLimit,
        J->SWallTime,
        MULToTString((MSched.Time - J->StartTime) - (J->WCLimit + J->SWallTime)));
 
      sprintf(tmpMsg,"JOBWCVIOLATION:  job '%s' in state '%s' has exceeded its wallclock limit (%ld) by %s (job will be cancelled)  job start time: %s",
        J->Name,
        MJobState[J->State],
        J->WCLimit,
        MULToTString(MSched.Time - J->StartTime - J->WCLimit),
        MULToDString(&J->StartTime));
 
      MSysRegEvent(tmpMsg,0,0,1);

      if (MSched.WCViolAction == mwcvaPreempt)
        {
        if (MJobPreempt(J,NULL,-1,NULL,NULL) == FAILURE)
          {
          DBG(2,fCORE) DPrint("ALERT:    cannot preempt job '%s' (job exceeded wallclock limit)\n",
            J->Name);
          }
        }
      else
        {
        if (MRMJobCancel(J,"MOAB_INFO:  job exceeded wallclock limit\n",NULL) == FAILURE)
          {
          /* extend job wallclock by JobWCX */
 
          /* NYI */
          }
        }
      }    /* END if ((JobWCX >= 0) && ...) */

    /* enforce CRes utilization limits */

    ResourceLimitsExceeded = FALSE;

    RQ = J->Req[0];  /* FIXME */

    if ((P->ResourceLimitPolicy[mrProc] != mrlpNONE) &&
        (RQ->URes.Procs > 100 * RQ->DRes.Procs))
      {
      DBG(3,fSCHED) DPrint("INFO:     job %s exceeds requested proc limit (%.2lf > %.2lf)\n",
        J->Name,
        (double)RQ->URes.Procs / 100.0,
        (double)RQ->DRes.Procs);

      VRes   = mrProc;
      VLimit = 100 * RQ->DRes.Procs;
      VVal   = RQ->URes.Procs;

      ResourceLimitsExceeded = TRUE;
      }
    else if ((P->ResourceLimitPolicy[mrMem] != mrlpNONE) &&
        (RQ->DRes.Mem > 0) &&
        (RQ->URes.Mem > RQ->DRes.Mem))
      {
      DBG(3,fSCHED) DPrint("INFO:     job %s exceeds requested memory limit (%d > %d)\n",
        J->Name,
        RQ->URes.Mem,
        RQ->DRes.Mem);

      VRes   = mrMem;
      VLimit = RQ->DRes.Mem;
      VVal   = RQ->URes.Mem;

      ResourceLimitsExceeded = TRUE;
      }
    else if ((P->ResourceLimitPolicy[mrSwap] != mrlpNONE) &&
        (RQ->DRes.Swap > 0) &&		     
        (RQ->URes.Swap > RQ->DRes.Swap))
      {
      DBG(3,fSCHED) DPrint("INFO:     job %s exceeds requested swap limit (%d > %d)\n",
        J->Name,
        RQ->URes.Swap,
        RQ->DRes.Swap);

      VRes   = mrSwap;
      VLimit = RQ->DRes.Swap;
      VVal   = RQ->URes.Swap;

      ResourceLimitsExceeded = TRUE;
      }
    else if ((P->ResourceLimitPolicy[mrDisk] != mrlpNONE) &&
        (RQ->DRes.Disk > 0) &&
        (RQ->URes.Disk > RQ->DRes.Disk))
      {
      DBG(3,fSCHED) DPrint("INFO:     job %s exceeds requested disk limit (%d > %d)\n",
        J->Name,
        RQ->URes.Disk,
        RQ->DRes.Disk);

      VRes   = mrDisk;
      VLimit = RQ->DRes.Disk;
      VVal   = RQ->URes.Disk;

      ResourceLimitsExceeded = TRUE;
      }

    if (ResourceLimitsExceeded == FALSE)
      {
      continue;
      }

    /* job is using more resources than requested */

    J->RULVTime += MSched.Iteration;
  
    switch (P->ResourceLimitPolicy[VRes])
      {
      case mrlpAlways:

        /* check limited resources */

        break;

      case mrlpExtendedViolation:

        /* determine length of violation */

        if (J->RULVTime < P->ResourceLimitMaxViolationTime[VRes])
          {
          /* ignore violation */

          ResourceLimitsExceeded = FALSE;
          }
         
        break;

      case mrlpBlockedWorkloadOnly:

        /* determine if eligible job is blocked */

        /* does job reservation exist at Now + MSched.Iteration for node utilized by job? */

        /* NYI */

        ResourceLimitsExceeded = FALSE;

        break;

      default:

        DBG(1,fSCHED) DPrint("ALERT:    unexpected limit violation policy %d\n",
          P->ResourceLimitPolicy[VRes]);

        ResourceLimitsExceeded = FALSE;

        break;
      }  /* END switch (P->ResourceUtilizationPolicy) */

    if (ResourceLimitsExceeded == FALSE)
      {
      continue;
      }

    /* job violates resource utilization policy */

    sprintf(tmpMsg,"JOBRESVIOLATION:  job '%s' in state '%s' has exceeded %s resource limit (%d > %d) (action %s will be taken)  job start time: %s",
      J->Name,
      MJobState[J->State],
      MResourceType[VRes],
      VVal,
      VLimit,
      MPolicyAction[P->ResourceLimitViolationAction[VRes]],
      MULToDString(&J->StartTime));

    MSysRegEvent(tmpMsg,0,0,1);

    switch(P->ResourceLimitViolationAction[VRes])
      {
      case mrlaRequeue:

        rc = MRMJobRequeue(J,NULL,NULL);

        break;

      case mrlaCancel:

        rc = MRMJobCancel(J,"job violates resource utilization policies",NULL);

        break;

      case mrlaSuspend:

        if ((rc = MRMJobSuspend(J,NULL,NULL)) == SUCCESS)
          {
          J->RMinTime = MSched.Time + P->AdminMinSTime;
          }
         
        break;

      default:

        rc = FAILURE;

        DBG(1,fSCHED) DPrint("ALERT:    unexpected limit violation action %d\n",
          P->ResourceLimitViolationAction[VRes]);

        break;
      }  /* END switch(P->ResourceLimitViolationAction) */

    DBG(1,fSCHED) DPrint("ALERT:    limit violation action %s %s\n",
      MPolicyAction[P->ResourceLimitViolationAction[VRes]],
      (rc == SUCCESS) ? "succeeded" : "failed");
    }    /* END for (jindex) */
 
  return(SUCCESS);
  }  /* END MLimitEnforceAll() */ 

/* END MLimit.c */

