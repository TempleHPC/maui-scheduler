/* HEADER */

#include "moab.h"
#include "msched-proto.h"
 
extern mlog_t mlog;
 
extern mrm_t       MRM[];
extern mam_t       MAM[];

extern const char *MRMState[]; 
extern const char *MRMType[];
extern const char *MRMAuthType[];
extern const char *MRMSubType[];
extern const char *MComp[];
extern const char *MDefReason[];
extern const char *MNodeState[];
extern const char *MRMFuncType[];
extern const char *MSockProtocol[];
extern const char *MWireProtocol[];
extern const char *MRMAttr[];
extern const char *MCSAlgoType[];

extern mattrlist_t MAList;
extern msys_t      MSys;
extern mjob_t     *MJob[];
extern mnode_t    *MNode[];
extern msched_t    MSched;
extern mframe_t    MFrame[];
extern int         MAQ[];
extern mstat_t     MStat;
extern mqos_t      MQOS[];
extern mclass_t    MClass[];
extern mpar_t      MPar[];
extern msim_t      MSim;

extern mrmfunc_t   MRMFunc[];

extern mx_t        X;

#define MRMCfgParam "RMCFG"




int __MRMStartFunc(mrm_t *,int);
int __MRMEndFunc(mrm_t *,int);

#include "MLLI.c"





int MRMLoadModules()

  {
  const char *FName = "MRMLoadModules";

  DBG(2,fRM) DPrint("%s()\n",
    FName);

#if defined(__MLL22) || defined(__MLL31)
  MLLLoadModule(&MRMFunc[mrmtLL]);
#endif /* __MLL22 || __MLL31 */

#ifdef __MPBS
  MPBSLoadModule(&MRMFunc[mrmtPBS]);
#endif /* __MPBS */

#ifdef __MSGE
  MSGELoadModule(&MRMFunc[mrmtSGE]);
#endif /* __MSGE */

  MS3LoadModule(&MRMFunc[mrmtSSS]);

#ifdef __MRMS
  MRMSLoadModule(&MRMFunc[mrmtRMS]);
#endif /* __MRMS */

#ifdef __MLSF
  MLSFLoadModule(&MRMFunc[mrmtLSF]);
#endif /* __MLSF */

  MWikiLoadModule(&MRMFunc[mrmtWiki]);

  return(SUCCESS);
  }  /* END MRMLoadModules() */





int MRMInitialize()
		
  {
  int rmindex;
  int Failure;
  int rc;
  int SC;
 
  mrm_t *R;

  const char *FName = "MRMInitialize";
 
  DBG(2,fRM) DPrint("%s()\n",
    FName);
 
  Failure = FALSE;

  for (rmindex = 0;MRM[rmindex].Type != mrmtNONE;rmindex++)
    {
    R = &MRM[rmindex];

    if (R->Name[0] == '\0')
      {
      sprintf(R->Name,"%s.%d",
        MRMType[R->Type],
        R->Index);
      }
 
    if (MRMFunc[R->Type].RMInitialize == NULL)
      {
      DBG(7,fRM) DPrint("ALERT:    cannot initialize RM (RM '%s' does not support function '%s')\n",
        R->Name,
        MRMFuncType[mrmRMInitialize]);
 
      continue;
      }
 
    MUThread(
      (int (*)(void))*MRMFunc[R->Type].RMInitialize,
      R->P[0].Timeout,
      &rc,
      2,
      NULL,
      R,
      &SC);
 
    if (rc == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot initialize RM (RM '%s' failed in function '%s')\n",
        R->Name,
        MRMFuncType[mrmRMInitialize]);

      MRMSetFailure(R,mrmRMInitialize,"cannot initialize interface");

      Failure = TRUE;
 
      continue;
      }

    R->RMNI          = DEFAULT_MRMNETINTERFACE; 

    R->FailIteration = -1;
    R->State         = mrmsActive;
    }    /* END for (rmindex) */
 
  if (Failure == TRUE)
    {
    return(FAILURE);
    }

  if (X.XRMInitialize != (int (*)())0)
    (*X.XRMInitialize)(X.xd,NULL);

  return(SUCCESS);
  }  /* END MRMInitialize() */
 
 
 
 
int MRMGetInfo()
 
  {
  mjob_t  *J;
 
  int JobCount;
  int NodeCount;

  static int APIFailureCount = 0;

  const char *FName = "MRMGetInfo";

  DBG(2,fRM) DPrint("%s()\n",
    FName);

  MStat.ActiveJobs = 0;         
 
  if (MSched.Mode == msmSim)
    {
    MSimRMGetInfo();
 
    if (MSim.QueueChanged == TRUE)
      {
      MParUpdate(&MPar[0]);
      }
    }
  else 
    {
    /* meanwhile, back in the real world... */
 
    /* clear runtime statistics */
 
    MStat.EligibleJobs = 0;
 
    MAQ[0] = -1;
 
    MClusterClearUsage();
 
    /* load resource data */
 
    if (MSched.Iteration == 0) 
      {
      MSched.InitialLoad = TRUE;
      }
 
    if ((X.XRMGetData != (int (*)())0) && 
       ((*X.XRMGetData)(X.xd,mcmBlock) == FAILURE))
      {
      char tmpLine[MAX_MLINE];

      DBG(0,fRM) DPrint("WARNING:  cannot load resource manager data at time %s\n",
        MULToDString(&MSched.Time));

      APIFailureCount++;
 
      if (APIFailureCount > MSched.APIFailureThreshhold)
        {
        sprintf(tmpLine,"RMFAILURE:  API has failed %d consecutive times on %s",
          MSched.APIFailureThreshhold,
          MULToDString(&MSched.Time));
 
        MSysRegEvent(tmpLine,0,0,1);

        APIFailureCount = 0;
        }
 
      return(FAILURE);
      }

    /* load RM node information */

    if (MRMClusterQuery(&NodeCount,NULL) == SUCCESS)
      {
      DBG(1,fRM) DPrint("INFO:     resources detected: %d\n",
        NodeCount);
      }
    else
      {
      DBG(1,fRM) DPrint("WARNING:  no resources detected\n");
      }
 
    /* load workload data */
 
    if (MRMWorkloadQuery(&JobCount,NULL) == SUCCESS)
      {
      DBG(1,fALL) DPrint("INFO:     jobs detected: %d\n",
        JobCount);
      }
    else
      {
      DBG(1,fALL) DPrint("WARNING:  no workload detected\n");
      }
 
    MSched.InitialLoad = FALSE;

    MStatClearUsage(mxoNode,(1 << mlActive)|(1 << mlIdle),FALSE);
 
    MClusterUpdateNodeState(); 
 
    MParUpdate(&MPar[0]);
    }  /* END else if (MSched.Mode != msmSim) */
 
  /* calculate statistics */
 
  MStat.AvgQueuePH += MPar[0].L.IP->Usage[mptMaxPS][0] / 3600;
 
  DBG(4,fRM)
    {
    DBG(4,fRM) DPrint("INFO:     jobs in queue\n");
 
    for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
      {
      MJobShow(J,0,NULL);
      }
    }

  APIFailureCount = 0;
 
  return(SUCCESS);
  }  /* END MRMGetInfo() */



 
int MRMCheckEvents()
 
  {
  int rmindex;
  int EventFound;
 
  mrm_t *R;

  const char *FName = "MRMCheckEvents";

  DBG(9,fRM) DPrint("%s()\n",
    FName);

  if (MSched.Mode == msmSim)
    {
    return(FAILURE);
    }
 
  EventFound = FALSE;
 
  for (rmindex = 0;MRM[rmindex].Type != mrmtNONE;rmindex++)
    {
    R = &MRM[rmindex];

    if (MRMFunc[R->Type].RMEventQuery == NULL)
      {
      DBG(9,fRM) DPrint("ALERT:    cannot query events on RM (RM '%s' does not support function '%s')\n",
        R->Name,
        MRMFuncType[mrmRMEventQuery]);

      continue;
      }

    if ((*MRMFunc[R->Type].RMEventQuery)(R,NULL) == SUCCESS)
      {
      EventFound = TRUE;

      break;
      }
    }  /* END for (rmindex) */
 
  if (EventFound == FALSE)
    {
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MRMCheckEvents() */




int MRMClusterQuery(

  int *RCount,  /* O */
  int *SC)      /* O */
 
  {
  int rmindex;
  int rc;

  int tmpRCount = 0;
  int TotalRCount;
 
  mrm_t *R;

  const char *FName = "MRMClusterQuery";

  DBG(2,fRM) DPrint("%s()\n",
    FName);
 
  TotalRCount = 0;

  for (rmindex = 0;MRM[rmindex].Type != mrmtNONE;rmindex++)
    {
    R = &MRM[rmindex];

    if (MRMFunc[R->Type].ClusterQuery == NULL)
      {
      DBG(7,fRM) DPrint("ALERT:    cannot load cluster resources on RM (RM '%s' does not support function '%s')\n",
        R->Name,
        MRMFuncType[mrmClusterQuery]);
 
      continue;
      }

    __MRMStartFunc(R,mrmClusterQuery);

    MUThread(
      (int (*)(void))MRMFunc[R->Type].ClusterQuery,
      R->P[0].Timeout,
      &rc,
      4,
      NULL,
      R,
      &tmpRCount,
      NULL,  /* EMsg */
      SC);

    __MRMEndFunc(R,mrmClusterQuery);

    if (rc == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot load cluster resources on RM (RM '%s' failed in function '%s')\n",
        R->Name,
        MRMFuncType[mrmClusterQuery]);

      MRMSetFailure(R,mrmClusterQuery,"cannot get node info");
 
      continue;
      }

    DBG(1,fRM) DPrint("INFO:     %d %s resources detected on RM %s\n",
      tmpRCount,
      MRMType[R->Type],
      R->Name);

    TotalRCount += tmpRCount;
    }    /* END for (rmindex) */

  if (RCount != NULL)
    *RCount = TotalRCount;
 
  if (TotalRCount == 0)
    {
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MRMClusterQuery() */





 
int MRMWorkloadQuery(
 
  int *WCount,  /* O */
  int *SC)      /* O (optional) */
 
  {
  int rmindex;
  int rc;
 
  int tmpWCount;
  int TotalWCount;

  int nindex;
  mnode_t *N;
 
  mrm_t *R;

  const char *FName = "MRMWorkloadQuery";
 
  DBG(2,fRM) DPrint("%s()\n",
    FName);
 
  TotalWCount = 0;

  /* reset dedicated resources */
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];
 
    if ((N == NULL) || (N->Name[0] == '\0'))
      break;
 
    if (N->Name[0] == '\1')
      continue;
 
    memset(&N->DRes,0,sizeof(N->DRes));
    }
 
  for (rmindex = 0;MRM[rmindex].Type != mrmtNONE;rmindex++)
    {
    R = &MRM[rmindex];
 
    if (MRMFunc[R->Type].WorkloadQuery == NULL)
      {
      DBG(7,fRM) DPrint("ALERT:    cannot load cluster workload on RM (RM '%s' does not support function '%s')\n",
        R->Name,
        MRMFuncType[mrmWorkloadQuery]);
 
      continue;
      }

    __MRMStartFunc(R,mrmWorkloadQuery);
 
    MUThread(
      (int (*)(void))MRMFunc[R->Type].WorkloadQuery,
      R->P[0].Timeout,
      &rc,
      3,
      NULL,
      R,
      &tmpWCount,
      SC);

    __MRMEndFunc(R,mrmWorkloadQuery);
 
    if (rc == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot load cluster workload on RM (RM '%s' failed in function '%s')\n",
        R->Name,
        MRMFuncType[mrmWorkloadQuery]);

      MRMSetFailure(R,mrmWorkloadQuery,"cannot get workload info");
 
      continue;
      }
 
    DBG(1,fRM) DPrint("INFO:     %d %s jobs detected on RM %s\n",
      tmpWCount,
      MRMType[R->Type],
      R->Name);
 
    TotalWCount += tmpWCount;
    }    /* END for (rmindex) */
 
  if (WCount != NULL)
    *WCount = TotalWCount;
 
  if (TotalWCount == 0)
    {
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MRMWorkloadQuery() */




int MRMJobStart(

  mjob_t *J,    /* I */
  char   *Msg,  /* O: optional */
  int    *SC)   /* O: optional */

  {
  int rqindex;
  int rmindex;
  int rmcount;

  int rc;

  mrm_t *R;

  int RMList[MAX_MRM];

  char  tmpLine[MAX_MLINE];
  char *MPtr;

  const char *FName = "MRMJobStart";

  DBG(1,fRM) DPrint("%s(%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if (Msg != NULL)
    MPtr = Msg;
  else
    MPtr = tmpLine;

  MPtr[0] = '\0';

  if (J == NULL)
    {
    return(FAILURE);
    }

  switch (MSched.Mode)
    {
    case msmSim:

      return(MSimJobStart(J)); 

      /*NOTREACHED*/

      break;

    case msmTest:

      sprintf(MPtr,"cannot start job - test mode");

      DBG(3,fRM) DPrint("INFO:     cannot start job %s (%s)\n",
        J->Name,
        MPtr);

      if (SC != NULL)
	*SC = mscNoError;
 
      return(FAILURE);

      /*NOTREACHED*/
 
      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(MSched.Mode) */

  memset(RMList,0,sizeof(RMList));
 
  rmcount = 0;
 
  /* determine resource managers used */
 
  for (rqindex = 0;rqindex < MAX_MREQ_PER_JOB;rqindex++)
    {
    if (J->Req[rqindex] == NULL)
      continue;
 
    for (rmindex = 0;rmindex < rmcount;rmindex++)
      {
      if (RMList[rmindex] == J->Req[rqindex]->RMIndex)
        {
        break;
        }
      }
 
    if (rmindex == rmcount)
      {
      RMList[rmindex] = J->Req[rqindex]->RMIndex;
 
      rmcount++;
      }
    }  /* END for (rqindex) */
 
  for (rmindex = 0;rmindex < rmcount;rmindex++)
    {
    R = &MRM[RMList[rmindex]];

    if ((R->FailIteration == MSched.Iteration) &&
        (R->FailCount >= MAX_RMFAILCOUNT))
      {
      sprintf(MPtr,"cannot start job - fail iteration");

      DBG(3,fRM) DPrint("INFO:     cannot start job %s (%s)\n",
        J->Name,
        MPtr);

      if (SC != NULL)
        *SC = mscNoError;

      return(FAILURE);
      }

    if (MRMFunc[R->Type].JobStart == NULL) 
      {
      sprintf(MPtr,"cannot start job - RM '%s' does not support function '%s'",
        R->Name,
        MRMFuncType[mrmJobStart]);    

      DBG(3,fRM) DPrint("ALERT:    cannot start job %s (%s)\n",
        J->Name,
        MPtr);

      if (SC != NULL)
        *SC = mscSysFailure;

      return(FAILURE);
      }

    __MRMStartFunc(R,mrmJobStart);

    MUThread(
      (int (*)(void))MRMFunc[R->Type].JobStart,
      R->P[0].Timeout,
      &rc,
      4,
      NULL,
      J,
      R,
      MPtr,
      SC);

    __MRMEndFunc(R,mrmJobStart);

    if (rc == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot start job %s (RM '%s' failed in function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobStart]);

      MRMSetFailure(R,mrmJobStart,"cannot start job");
 
      return(FAILURE);
      }
    }    /* END for (rmindex) */

  return(SUCCESS);
  }  /* END MRMJobStart() */




int MRMJobCancel(
 
  mjob_t *J,       /* I:  req */
  char   *Message, /* I:  opt */
  int    *SC)      /* IO: opt */
 
  {
  int rqindex;
  int rmindex;
  int rmcount;
 
  int rc;
 
  mrm_t *R;
 
  int RMList[MAX_MRM];

  const char *FName = "MRMJobCancel";
 
  DBG(1,fRM) DPrint("%s(%s,%s,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (Message != NULL) ? Message : "NULL");

  if (J == NULL)
    {
    return(FAILURE);
    }

  switch (MSched.Mode)
    {
    case msmSim:
 
      return(MSimJobCancel(J));
 
      /*NOTREACHED*/
 
      break;
 
    case msmTest:
 
      DBG(3,fRM) DPrint("INFO:     cannot cancel job '%s' (test mode)\n",
        J->Name);
 
      return(FAILURE);
 
      /*NOTREACHED*/
 
      break;
 
    default:

      /* NO-OP */
 
      break;
    }  /* END switch(MSched.Mode) */

  memset(RMList,0,sizeof(RMList));
 
  rmcount = 0;
 
  /* determine resource managers used */
 
  for (rqindex = 0;rqindex < MAX_MREQ_PER_JOB;rqindex++)
    {
    if (J->Req[rqindex] == NULL)
      continue;
 
    for (rmindex = 0;rmindex < rmcount;rmindex++)
      {
      if (RMList[rmindex] == J->Req[rqindex]->RMIndex)
        {
        break;
        }
      }
 
    if (rmindex == rmcount)
      {
      RMList[rmindex] = J->Req[rqindex]->RMIndex;
 
      rmcount++;
      }
    }  /* END for (rqindex) */
 
  for (rmindex = 0;rmindex < rmcount;rmindex++)
    {
    R = &MRM[RMList[rmindex]];

    if (MSched.Iteration == R->FailIteration)
      {
      DBG(3,fRM) DPrint("INFO:     cannot cancel job '%s' (fail iteration)\n",
        J->Name);
 
      return(FAILURE);
      }
 
    if (MRMFunc[R->Type].JobCancel == NULL)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot cancel job %s (RM '%s' does not support function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobCancel]);
 
      return(FAILURE);
      }

    __MRMStartFunc(R,mrmJobCancel);
 
    MUThread(
      (int (*)(void))MRMFunc[R->Type].JobCancel,
      R->P[0].Timeout,
      &rc,
      5,
      NULL,
      J,
      R,
      Message,
      NULL,
      SC);

    __MRMEndFunc(R,mrmJobCancel);
 
    if (rc == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot cancel job %s (RM '%s' failed in function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobCancel]);

      MRMSetFailure(R,mrmJobStart,"cannot cancel job");
 
      return(FAILURE);
      }
    }    /* END for (rmindex) */
 
  return(SUCCESS);
  }  /* END MRMJobCancel() */




int MRMJobGetProximateMNL(

  mjob_t      *J,       /* I:  req */
  mrm_t       *R,
  mnodelist_t  SMNL,
  mnodelist_t  DMNL,
  long         StartTime,
  int          DoExactMatch,
  char        *Message, /* I:  opt */
  int         *SC)      /* IO: opt */

  {
  int rc;

  const char *FName = "MRMJobGetProximateMNL";

  DBG(1,fRM) DPrint("%s(R,%s,SMNL,DMNL,%ld,%d,%s,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    StartTime,
    DoExactMatch,
    (Message != NULL) ? Message : "NULL");

  if ((J == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  switch (MSched.Mode)
    {
    case msmSim:

      return(FAILURE);

      /*NOTREACHED*/

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(MSched.Mode) */

  /* single RM allowed */

  if (MRMFunc[R->Type].JobGetProximateTasks == NULL)
    {
    DBG(3,fRM) DPrint("ALERT:    cannot get proximate tasks for job %s (RM '%s' does not support function '%s')\n",
      J->Name,
      R->Name,
      MRMFuncType[mrmJobGetProximateTasks]);

    return(FAILURE);
    }

  MUThread(
    (int (*)(void))MRMFunc[R->Type].JobGetProximateTasks,
    R->P[0].Timeout,
    &rc,
    8,
    NULL,
    J,
    R,
    SMNL,
    DMNL,
    StartTime,
    DoExactMatch,
    Message,
    SC);

  if (rc == FAILURE)
    {
    DBG(3,fRM) DPrint("ALERT:    cannot get proximate tasks for job %s (RM '%s' failed in function '%s')\n",
      J->Name,
      R->Name,
      MRMFuncType[mrmJobGetProximateTasks]);

    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MRMJobGetProximateTasks() */





int MRMJobModify(

  mjob_t *J,       /* I:  req */
  char   *Attr,    /* I */
  char   *SubAttr, /* I */
  char   *Value,   /* I */
  int    *SC)      /* O: opt */
 
  {
  int rqindex;
  int rmindex;
  int rmcount;
 
  int rc;

  mrm_t *R;
 
  int RMList[MAX_MRM];

  const char *FName = "MRMJobModify";
 
  DBG(1,fRM) DPrint("%s(%s,%s,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (Attr != NULL) ? Attr : "NULL");
 
  if (J == NULL)
    {
    return(FAILURE);
    }
 
  switch (MSched.Mode)
    {
    case msmSim:
 
      return(MSimJobModify(J,Attr,SubAttr,Value,SC));
 
      /*NOTREACHED*/
 
      break;
 
    case msmTest:
 
      DBG(3,fRM) DPrint("INFO:     cannot modify job '%s' (test mode)\n",
        J->Name);
 
      return(FAILURE);
 
      /*NOTREACHED*/
 
      break;
 
    default:

      /* NO-OP */
 
      break;
    }  /* END switch(MSched.Mode) */
 
  memset(RMList,0,sizeof(RMList));
 
  rmcount = 0;
 
  /* determine resource managers used */
 
  for (rqindex = 0;rqindex < MAX_MREQ_PER_JOB;rqindex++)
    {
    if (J->Req[rqindex] == NULL)
      continue;
 
    for (rmindex = 0;rmindex < rmcount;rmindex++)
      {
      if (RMList[rmindex] == J->Req[rqindex]->RMIndex)
        {
        break;
        }
      }
 
    if (rmindex == rmcount)
      {
      RMList[rmindex] = J->Req[rqindex]->RMIndex;
 
      rmcount++;
      }
    }  /* END for (rqindex) */

  for (rmindex = 0;rmindex < rmcount;rmindex++)
    {
    R = &MRM[RMList[rmindex]];
 
    if (MSched.Iteration == R->FailIteration)
      {
      DBG(3,fRM) DPrint("INFO:     cannot modify job '%s' (fail iteration)\n",
        J->Name);
 
      return(FAILURE);
      }
 
    if (MRMFunc[R->Type].JobModify == NULL)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot modify job %s (RM '%s' does not support function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobModify]);
 
      return(FAILURE);
      }
 
    MUThread(
      (int (*)(void))MRMFunc[R->Type].JobModify,
      R->P[0].Timeout,
      &rc,
      7,
      NULL,
      J,
      R,
      Attr,
      SubAttr,
      Value,
      NULL,
      SC);
 
    if (rc == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot modify job %s (RM '%s' failed in function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobModify]);
 
      return(FAILURE);
      }
    }    /* END for (rmindex) */
 
  return(SUCCESS);
  }  /* END MRMJobModify() */




int MRMJobMigrate(

  mjob_t    *J,  /* I */
  mnalloc_t *NL, /* I */
  int       *SC) /* O: (optional) */

  {
  int rqindex;
  int rmindex;
  int rmcount;

  int rc;

  mrm_t *R;

  int RMList[MAX_MRM];

  const char *FName = "MRMJobMigrate";

  DBG(1,fRM) DPrint("%s(%s,%s,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (NL != NULL) ? "NL" : "NULL");

  if (J == NULL)
    {
    return(FAILURE);
    }

  if (MJOBISACTIVE(J) == FALSE)
    {
    return(FAILURE);
    }

  switch (MSched.Mode)
    {
    case msmSim:

      return(FAILURE);

      /*NOTREACHED*/

      break;

    case msmTest:

      DBG(3,fRM) DPrint("INFO:     cannot migrate job '%s' (test mode)\n",
        J->Name);

      return(FAILURE);

      /*NOTREACHED*/

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(MSched.Mode) */

  memset(RMList,0,sizeof(RMList));

  rmcount = 0;

  /* determine resource managers used */

  for (rqindex = 0;rqindex < MAX_MREQ_PER_JOB;rqindex++)
    {
    if (J->Req[rqindex] == NULL)
      continue;

    for (rmindex = 0;rmindex < rmcount;rmindex++)
      {
      if (RMList[rmindex] == J->Req[rqindex]->RMIndex)
        {
        break;
        }
      }    /* END for (rmindex) */

    if (rmindex == rmcount)
      {
      RMList[rmindex] = J->Req[rqindex]->RMIndex;

      rmcount++;
      }
    }  /* END for (rqindex) */

  for (rmindex = 0;rmindex < rmcount;rmindex++)
    {
    R = &MRM[RMList[rmindex]];

    if (MRMFunc[R->Type].JobMigrate == NULL)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot migrate job %s (RM '%s' does not support function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobMigrate]);

      return(FAILURE);
      }

    MUThread(
      (int (*)(void))MRMFunc[R->Type].JobMigrate,
      R->P[0].Timeout,
      &rc,
      5,
      NULL,
      J,
      R,
      NL,
      NULL,
      SC);

    if (rc == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot migrate job %s (RM '%s' failed in function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobMigrate]);

      return(FAILURE);
      }
    }    /* END for (rmindex) */

  return(SUCCESS);
  }  /* END MRMJobMigrate() */




int MRMJobRequeue(
 
  mjob_t  *J,     /* I:  req */
  mjob_t **JPeer, /* I:  opt */
  int     *SC)    /* IO: opt */
 
  {
  int rqindex;
  int rmindex;
  int rmcount;
 
  int rc;
 
  mrm_t *R;
 
  int RMList[MAX_MRM];

  const char *FName = "MRMJobRequeue";
 
  DBG(1,fRM) DPrint("%s(%s,%s,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (JPeer != NULL) ? "JPeer" : "NULL");
 
  if (J == NULL)
    {
    return(FAILURE);
    }

  switch (MSched.Mode)
    {
    case msmSim:
 
      return(MSimJobRequeue(J));
 
      /*NOTREACHED*/
 
      break;
 
    case msmTest:
 
      DBG(3,fRM) DPrint("INFO:     cannot requeue job '%s' (test mode)\n",
        J->Name);
 
      return(FAILURE);
 
      /*NOTREACHED*/
 
      break;
 
    default:
 
      break;
    }  /* END switch(MSched.Mode) */

  memset(RMList,0,sizeof(RMList));
 
  rmcount = 0;
 
  /* determine resource managers used */
 
  for (rqindex = 0;rqindex < MAX_MREQ_PER_JOB;rqindex++)
    {
    if (J->Req[rqindex] == NULL)
      continue;
 
    for (rmindex = 0;rmindex < rmcount;rmindex++)
      {
      if (RMList[rmindex] == J->Req[rqindex]->RMIndex)
        {
        break;
        }
      }
 
    if (rmindex == rmcount)
      {
      RMList[rmindex] = J->Req[rqindex]->RMIndex;
 
      rmcount++;
      }
    }  /* END for (rqindex) */
 
  for (rmindex = 0;rmindex < rmcount;rmindex++)
    {
    R = &MRM[RMList[rmindex]];
 
    if (MRMFunc[R->Type].JobRequeue == NULL)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot requeue job %s (RM '%s' does not support function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobRequeue]);
 
      return(FAILURE);
      }
 
    MUThread(
      (int (*)(void))MRMFunc[R->Type].JobRequeue,
      R->P[0].Timeout,
      &rc,
      5,
      NULL,
      J,
      R,
      JPeer,
      NULL,
      SC);
 
    if (rc == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot requeue job %s (RM '%s' failed in function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobRequeue]);
 
      return(FAILURE);
      }
    }    /* END for (rmindex) */
 
  return(SUCCESS);
  }  /* END MRMJobRequeue() */




int MRMJobSubmit(

  char    *JobDesc,  /* I */
  mrm_t   *R,        /* I */
  mjob_t **J,        /* O */
  char    *JobName,  /* O */
  char    *Msg,      /* O */
  int     *SC)       /* O */

  {
  int rc;

  if (MSched.Mode == msmSim)
    {
    /* return(MSimJobSubmit(MSched.Time,JobDesc,NULL,0)); */

    return(FAILURE);
    }

  MUThread(
    (int (*)(void))MRMFunc[R->Type].JobSubmit,
    R->P[0].Timeout,
    &rc,
    6,
    NULL,
    JobDesc,
    R,
    J,
    JobName,
    Msg,
    SC);

  if (rc == FAILURE)
    {
    DBG(3,fRM) DPrint("ALERT:    cannot submit job (RM '%s' failed in function '%s')\n",
      R->Name,
      MRMFuncType[mrmJobSubmit]);
 
    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MRMJobSubmit() */




int MRMJobSuspend(
 
  mjob_t  *J,     /* I:  req */
  char    *Msg,   /* O   (optional) */
  int     *SC)    /* IO: (optional) */
 
  {
  int rqindex;
  int rmindex;
  int rmcount;
 
  int rc;
 
  mrm_t *R;
 
  int RMList[MAX_MRM];

  const char *FName = "MRMJobSuspend";
 
  DBG(1,fRM) DPrint("%s(%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");
 
  if (J == NULL)
    {
    return(FAILURE);
    }
 
  if (MSched.Mode == msmSim)
    {
    return(MSimJobSuspend(J));
    }
 
  memset(RMList,0,sizeof(RMList));
 
  rmcount = 0;
 
  /* determine resource managers used */
 
  for (rqindex = 0;rqindex < MAX_MREQ_PER_JOB;rqindex++)
    {
    if (J->Req[rqindex] == NULL)
      continue;
 
    for (rmindex = 0;rmindex < rmcount;rmindex++)
      {
      if (RMList[rmindex] == J->Req[rqindex]->RMIndex)
        {
        break;
        }
      }
 
    if (rmindex == rmcount)
      {
      RMList[rmindex] = J->Req[rqindex]->RMIndex;
 
      rmcount++;
      }
    }  /* END for (rqindex) */
 
  for (rmindex = 0;rmindex < rmcount;rmindex++)
    {
    R = &MRM[RMList[rmindex]];
 
    if (MRMFunc[R->Type].JobSuspend == NULL)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot suspend job %s (RM '%s' does not support function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobSuspend]);
 
      return(FAILURE);
      }
 
    MUThread(
      (int (*)(void))MRMFunc[R->Type].JobSuspend,
      R->P[0].Timeout,
      &rc,
      4,
      NULL,
      J,
      R,
      Msg,
      SC);
 
    if (rc == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot suspend job %s (RM '%s' failed in function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobSuspend]);
 
      return(FAILURE);
      }
    }    /* END for (rmindex) */
 
  return(SUCCESS);
  }  /* END MRMJobSuspend() */




int MRMJobCheckpoint(
 
  mjob_t  *J,            /* I:  req */
  int      TerminateJob, /* I:  req */
  char    *Msg,          /* O:  opt */
  int     *SC)           /* O:  opt */
 
  {
  int rqindex;
  int rmindex;
  int rmcount;
 
  int rc;
 
  mrm_t *R;
 
  int RMList[MAX_MRM];

  const char *FName = "MRMJobCheckpoint";
 
  DBG(1,fRM) DPrint("%s(%s,%d,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    TerminateJob);
 
  if (J == NULL)
    {
    return(FAILURE);
    }
 
  if (MSched.Mode == msmSim)
    {
    return(MSimJobCheckpoint(J));
    }
 
  memset(RMList,0,sizeof(RMList));
 
  rmcount = 0;
 
  /* determine resource managers used */
 
  for (rqindex = 0;rqindex < MAX_MREQ_PER_JOB;rqindex++)
    {
    if (J->Req[rqindex] == NULL)
      continue;
 
    for (rmindex = 0;rmindex < rmcount;rmindex++)
      {
      if (RMList[rmindex] == J->Req[rqindex]->RMIndex)
        {
        break;
        }
      }
 
    if (rmindex == rmcount)
      {
      RMList[rmindex] = J->Req[rqindex]->RMIndex;
 
      rmcount++;
      }
    }  /* END for (rqindex) */
 
  for (rmindex = 0;rmindex < rmcount;rmindex++)
    {
    R = &MRM[RMList[rmindex]];
 
    if (MRMFunc[R->Type].JobCheckpoint == NULL)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot checkpoint job %s (RM '%s' does not support function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobCheckpoint]);
 
      return(FAILURE);
      }
 
    MUThread(
      (int (*)(void))MRMFunc[R->Type].JobCheckpoint,
      R->P[0].Timeout,
      &rc,
      5,
      NULL,
      J,
      R,
      TerminateJob,
      Msg,
      SC);
 
    if (rc == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot checkpoint job %s (RM '%s' failed in function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobCheckpoint]);
 
      return(FAILURE);
      }
    }    /* END for (rmindex) */
 
  return(SUCCESS);
  }  /* END MRMJobCheckpoint() */





int MRMJobResume(
 
  mjob_t *J,     /* I:  req */
  char   *Msg,   /* O:  (optional) */
  int    *SC)    /* IO: (optional) */
 
  {
  int rqindex;
  int rmindex;
  int rmcount;

  int nindex;
 
  int rc;
 
  mrm_t   *R;
  mreq_t  *RQ;
  mnode_t *N;
 
  int RMList[MAX_MRM];

  const char *FName = "MRMJobResume";
 
  DBG(1,fRM) DPrint("%s(%s,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");
 
  if (J == NULL)
    {
    return(FAILURE);
    }
 
  if (MSched.Mode == msmSim)
    {
    return(MSimJobResume(J));
    }
 
  memset(RMList,0,sizeof(RMList));
 
  rmcount = 0;
 
  /* determine resource managers used */
 
  for (rqindex = 0;rqindex < MAX_MREQ_PER_JOB;rqindex++)
    {
    if (J->Req[rqindex] == NULL)
      continue;
 
    for (rmindex = 0;rmindex < rmcount;rmindex++)
      {
      if (RMList[rmindex] == J->Req[rqindex]->RMIndex)
        {
        break;
        }
      }
 
    if (rmindex == rmcount)
      {
      RMList[rmindex] = J->Req[rqindex]->RMIndex;
 
      rmcount++;
      }
    }  /* END for (rqindex) */
 
  for (rmindex = 0;rmindex < rmcount;rmindex++)
    {
    R = &MRM[RMList[rmindex]];
 
    if (MRMFunc[R->Type].JobResume == NULL)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot resume job %s (RM '%s' does not support function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobResume]);
 
      return(FAILURE);
      }
 
    MUThread(
      (int (*)(void))MRMFunc[R->Type].JobResume,
      R->P[0].Timeout,
      &rc,
      4,
      NULL,
      J,
      R,
      Msg,
      SC);
 
    if (rc == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot resume job %s (RM '%s' failed in function '%s')\n",
        J->Name,
        R->Name,
        MRMFuncType[mrmJobResume]);
 
      return(FAILURE);
      }
    }    /* END for (rmindex) */

  if (MResJCreate(J,NULL,MSched.Time,mjrActiveJob,NULL) == FAILURE)
    {
    DBG(0,fSCHED) DPrint("ERROR:    cannot create reservation for job '%s'\n",
      J->Name);
    }

  /* adjust per-node resource consumption */

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

  DBG(2,fSIM) DPrint("INFO:     job %s successfully resumed on %d procs\n",
    J->Name,
    J->Request.TC);
 
  return(SUCCESS);
  }  /* END MRMJobResume() */





int MRMLoadConfig(
 
  char *RMName)  /* I */
 
  {
  char    IndexName[MAX_MNAME];
 
  char    Value[MAX_MLINE];
 
  char   *ptr;
 
  mrm_t *R;
 
  /* FORMAT:  <KEY>=<VAL>[<WS><KEY>=<VAL>]...         */
  /*          <VAL> -> <ATTR>=<VAL>[:<ATTR>=<VAL>]... */
 
  /* load all/specified RM config info */
 
  if (MSched.ConfigBuffer == NULL)
    {
    return(FAILURE);
    }
 
  if ((RMName == NULL) || (RMName[0] == '\0'))
    {
    /* load ALL RM config info */
 
    ptr = MSched.ConfigBuffer; 
 
    IndexName[0] = '\0';
 
    while (MCfgGetSVal(
             MSched.ConfigBuffer,
             &ptr,
             MRMCfgParam,
             IndexName,
             NULL,
             Value,
             sizeof(Value),
             0,
             NULL) != FAILURE)
      {
      if (MRMFind(IndexName,&R) == FAILURE)
        {
        if (MRMAdd(IndexName,&R) == FAILURE)
          {
          /* unable to locate/create RM */
 
          IndexName[0] = '\0';
 
          continue;
          }

        MRMSetDefaults(R);

	MOLoadPvtConfig((void **)R,mxoRM,NULL,NULL,NULL);
        }
 
      /* load RM specific attributes */
 
      MRMProcessConfig(R,Value);
 
      IndexName[0] = '\0';
      }  /* END while (MCfgGetSVal() != FAILURE) */
    }    /* END if ((RMName == NULL) || (RMName[0] == '\0')) */
  else
    { 
    /* load specified RM config info */
 
    ptr = MSched.ConfigBuffer;
 
    R = NULL;
 
    while (MCfgGetSVal(
             MSched.ConfigBuffer,
             &ptr,
             MRMCfgParam,
             RMName,
             NULL,
             Value,
             sizeof(Value),
             0,
             NULL) == SUCCESS)
      {
      if ((R == NULL) &&
          (MRMFind(RMName,&R) == FAILURE))
        {
        if (MRMAdd(RMName,&R) == FAILURE)
          {
          /* unable to add standing reservation */
 
          return(FAILURE);
          }  

        MRMSetDefaults(R);

	MOLoadPvtConfig((void **)R,mxoRM,NULL,NULL,NULL);
        }
 
      /* load RM attributes */
 
      MRMProcessConfig(R,Value);
      }  /* END while (MCfgGetSVal() == SUCCESS) */
 
    if (R == NULL)
      { 
      return(FAILURE);
      }
 
    if (MRMCheckConfig(R) == FAILURE)
      {
      MRMDestroy(&R);
 
      /* invalid standing reservation destroyed */
 
      return(FAILURE);
      }
    }  /* END else ((RMName == NULL) || (RMName[0] == '\0')) */
 
  return(SUCCESS);
  }  /* END MRMLoadConfig() */




int MRMAdd(
 
  char   *RMName,  /* I */
  mrm_t **RP)      /* O */
 
  {
  int     rmindex;
 
  mrm_t *R;

  const char *FName = "MRMAdd";
 
  DBG(6,fSTRUCT) DPrint("%s(%s,%s)\n",
    FName,
    (RMName != NULL) ? "RMName" : "NULL",
    (RP != NULL) ? "RP" : "NULL");
 
  if ((RMName == NULL) || (RMName[0] == '\0'))
    {
    return(FAILURE);
    }
 
  if (RP != NULL)
    *RP = NULL;
 
  for (rmindex = 0;rmindex < MAX_MRM;rmindex++)
    {
    R = &MRM[rmindex];
 
    if ((R != NULL) && !strcmp(R->Name,RMName))
      {
      /* RM already exists */
 
      if (RP != NULL)
        *RP = R;
 
      return(SUCCESS);
      }
 
    if ((R != NULL) &&
        (R->Name[0] != '\0') &&
        (R->Name[0] != '\1'))
      {
      continue;
      } 
 
    /* empty slot found */
 
    /* create/initialize new record */
 
    if (R == NULL)
      {
      if (MRMCreate(RMName,&MRM[rmindex]) == FAILURE)
        {
        DBG(1,fALL) DPrint("ERROR:    cannot alloc memory for RM '%s'\n",
          RMName);
 
        return(FAILURE);
        }
 
      R = &MRM[rmindex];
      }
    else if (R->Name[0] == '\0')
      {
      MUStrCpy(R->Name,RMName,sizeof(R->Name));
      }
 
    if (RP != NULL)
      *RP = R;
 
    R->Index = rmindex;
 
    /* update RM record */
 
    if (MSched.Mode != msmSim)
      MCPRestore(mcpRM,RMName,(void *)R);
 
    DBG(5,fSTRUCT) DPrint("INFO:     RM %s added\n",
      RMName);
 
    return(SUCCESS);
    }    /* END for (rmindex) */
 
  /* end of table reached */
 
  DBG(1,fSTRUCT) DPrint("ALERT:    RM table overflow.  cannot add %s\n",
    RMName);
 
  return(SUCCESS); 
  }  /* END MRMAdd() */
 
 
 
 
int MRMCreate(
 
  char  *RMName,
  mrm_t *R)
 
  {
  if (R == NULL)
    {
    return(FAILURE);
    }
 
  /* use static memory for now */
 
  memset(R,0,sizeof(mrm_t));
 
  if ((RMName != NULL) && (RMName[0] != '\0'))
    MUStrCpy(R->Name,RMName,sizeof(R->Name));
 
  return(SUCCESS);
  }  /* END MRMCreate() */




int MRMFind(
 
  char   *RMName,  /* I */
  mrm_t **RP)      /* O (optional) */
 
  {
  /* if found, return success with RP pointing to standing reservation. */
 
  int    rmindex;
  mrm_t *R;
 
  if (RP != NULL)
    *RP = NULL;
 
  if ((RMName == NULL) ||
      (RMName[0] == '\0'))
    {
    return(FAILURE);
    }
 
  for (rmindex = 0;rmindex < MAX_MRM;rmindex++)
    {
    R = &MRM[rmindex];
 
    if ((R == NULL) || (R->Name[0] == '\0') || (R->Name[0] == '\1'))
      {
      break;
      }
 
    if (strcmp(R->Name,RMName) != 0)
      continue;
 
    /* RM found */
 
    if (RP != NULL)
      *RP = R;
 
    return(SUCCESS);
    }  /* END for (rmindex) */
 
  /* entire table searched */ 
 
  return(FAILURE);
  }  /* END MRMFind() */




int MRMSetDefaults(

  mrm_t *R)  /* I */

  {
  if (R == NULL) 
    {
    return(FAILURE);
    }

  if (R->Type == 0)
    {
    /* set general defaults */

    R->Type = DEFAULT_MRMTYPE;
 
    R->AuthType = DEFAULT_MRMAUTHTYPE;

    R->P[0].Timeout  = DEFAULT_MRMTIMEOUT;
    R->P[1].Timeout  = DEFAULT_MRMTIMEOUT;

    R->FailIteration = -1;

    R->JobCounter = 0;
    
    return(SUCCESS);
    }
 
  /* set rm type specific defaults */

  switch(R->Type)
    {
    default:

      /* NO-OP */

      break;
    }  /* END switch(R->Type) */
 
  return(SUCCESS);
  }  /* END MRMSetDefaults() */




int MRMAToString(

  mrm_t *R,      /* I */
  int    AIndex, /* I */ 
  char  *SVal,   /* I */
  int    Mode)   /* I */

  {
  if ((R == NULL) || (SVal == NULL))
    {
    return(FAILURE);
    }

  SVal[0] = '\0';

  switch(AIndex)
    {
    case mrmaAuthType:
 
      if (R->AuthType != 0)
        strcpy(SVal,MRMAuthType[R->AuthType]);
 
      break;
 
    case mrmaConfigFile:
 
      /* NYI */
 
      break;

    case mrmaEPort:

      if (R->EPort > 0)
        {
        sprintf(SVal,"%d",
          R->EPort);
        }

      break;

    case mrmaHost:
 
     if (R->P[0].HostName != NULL)
       strcpy(SVal,R->P[0].HostName);
 
      break;
 
    case mrmaLocalDiskFS:

      /* NYI */
 
      break;

    case mrmaName:

     if (R->Name[0] != '\0')
       strcpy(SVal,R->Name);

      break;
 
    case mrmaNMPort:
 
      if (R->NMPort != 0)
        {
        /* NOTE:  NMPort affect connection to BOTH global node monitor and local node manager */

        sprintf(SVal,"%d",
          R->NMPort);
        }
      
      break;

    case mrmaNMServer:

      if (R->P[1].HostName != NULL)
        strcpy(SVal,R->P[1].HostName);

      break;
 
    case mrmaPort:
 
      if (R->P[0].Port > 0)
        {
        sprintf(SVal,"%d",
          R->P[0].Port);
        }
 
      break;

    case mrmaSocketProtocol:

      if (R->P[0].SocketProtocol != mspNONE)
        strcpy(SVal,MSockProtocol[R->P[0].SocketProtocol]);
 
      break;

    case mrmaSuspendSig:

      if (R->SuspendSig[0] != '\0')
        strcpy(SVal,R->SuspendSig);

      break;

    case mrmaTimeout:
 
      if (R->P[0].Timeout > 0)
        strcpy(SVal,MULToTString(R->P[0].Timeout));
 
      break;
 
    case mrmaType:

      /* FORMAT:  <TYPE>[:<SUBTYPE>] */
 
      if (R->Type != mrmtNONE)
        {
        strcpy(SVal,MRMType[R->Type]);

        if (R->SubType != mrmstNONE)
          {
          sprintf(SVal,"%s:%s",
            SVal,
            MRMSubType[R->SubType]);
          }
        }    /* END if (R->Type != mrmtNONE) */

      break;

    case mrmaVersion:

      if (R->APIVersion[0] != '\0')
        strcpy(SVal,R->APIVersion);

      break;

    case mrmaWireProtocol:

      if (R->P[0].WireProtocol != mwpNONE)         
        strcpy(SVal,MWireProtocol[R->P[0].WireProtocol]);
 
      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MRMAToString() */



 
int MRMProcessConfig(
 
  mrm_t *R,     /* I */
  char  *Value) /* I */
 
  {
  int   aindex;
 
  char *ptr;
  char *TokPtr;
 
  char  ValLine[MAX_MLINE];
  char *ValList[2];
 
  if ((R == NULL) ||
      (Value == NULL) ||
      (Value[0] == '\0'))
    {
    return(FAILURE);
    }
 
  /* process value line */
 
  ptr = MUStrTok(Value," \t\n",&TokPtr);
 
  while(ptr != NULL)
    {
    /* parse name-value pairs */
 
    /* FORMAT:  <VALUE>[,<VALUE>] */
 
    if (MUGetPair(
          ptr,
          (const char **)MRMAttr,
          &aindex,
	  NULL,
          TRUE,
          NULL,
          ValLine,
          MAX_MNAME) == FAILURE) 
      {
      /* cannot parse value pair */
 
      ptr = MUStrTok(NULL," \t\n",&TokPtr);
 
      continue;
      }
 
    ValList[0] = ValLine;
    ValList[1] = NULL;
 
    switch(aindex)
      {       
      case mrmaAuthType:

        R->AuthType = MUGetIndex(ValLine,MRMAuthType,FALSE,0); 

        break;

      case mrmaConfigFile:

        MUStrCpy(R->U.LL.ConfigFile,ValLine,sizeof(R->U.LL.ConfigFile));   

        break;

      case mrmaHost:

        MUStrDup(&R->P[0].HostName,ValLine);      

        break;

      case mrmaLocalDiskFS:

        MUStrCpy(R->U.PBS.LocalDiskFS,ValLine,sizeof(R->U.PBS.LocalDiskFS));    

        break;

      case mrmaMinETime:

        R->EMinTime = MUTimeFromString(ValLine);

        break;

      case mrmaNMPort:

        R->NMPort    = (int)strtol(ValLine,NULL,0);
        R->P[1].Port = (int)strtol(ValLine,NULL,0);

        break;

      case mrmaNMServer:

        MUStrDup(&R->P[1].HostName,ValLine);    

        break;

      case mrmaEPort:

        R->EPort = (int)strtol((char *)Value,NULL,0);

        break;
 
      case mrmaPort:

        R->P[0].Port = (int)strtol(ValLine,NULL,0);          

        break;

      case mrmaSocketProtocol:

        R->P[0].SocketProtocol = MUGetIndex(ValLine,MSockProtocol,FALSE,0);        
        R->P[1].SocketProtocol = R->P[0].SocketProtocol;

        break;

      case mrmaSuspendSig:

        MRMSetAttr(R,aindex,(void **)ValLine,mdfString,mSet);

        break;

      case mrmaTimeout:

        R->P[0].Timeout = MUTimeFromString(ValLine);

        R->P[0].Timeout = MIN(1000,R->P[0].Timeout);
 
        R->P[1].Timeout = R->P[0].Timeout;

        break;

      case mrmaType:

        {
        char *ptr;
        char *TokPtr;

        /* FORMAT:  <TYPE>[:<SUBTYPE>] */

        ptr = MUStrTok(ValLine,":",&TokPtr);
 
        R->Type = MUGetIndex(ptr,MRMType,FALSE,mrmtNONE);
 
        if ((ptr = MUStrTok(NULL,":",&TokPtr)) != NULL)
          {
          R->SubType = MUGetIndex(ptr,MRMSubType,FALSE,mrmstNONE);
          }
        }    /* END BLOCK */

        break;

      case mrmaVersion:

        MUStrCpy(R->APIVersion,ValLine,sizeof(R->APIVersion));

        break;

      case mrmaWireProtocol:

        R->P[0].WireProtocol = MUGetIndex(ValLine,MWireProtocol,FALSE,0);       
        R->P[1].WireProtocol = R->P[0].WireProtocol;

        break;
 
      default:
 
        DBG(4,fRM) DPrint("WARNING:  RM attribute '%s' not handled\n",
          MRMAttr[aindex]);
 
        break;
      }  /* END switch(aindex) */
 
    ptr = MUStrTok(NULL," \t\n",&TokPtr);
    }  /* END while (ptr != NULL) */
 
  return(SUCCESS);
  }  /* END MRMProcessConfig() */




int MRMDestroy(
 
  mrm_t **RP)  /* I */
 
  {
  mrm_t *R;

  if (RP == NULL)
    {
    return(SUCCESS);
    }

  R = *RP;
 
  if (R == NULL)
    {
    return(SUCCESS);
    }

  /* free dynamic data */

  MUFree(&R->P[0].HostName);
  MUFree(&R->P[0].CSKey);

  MUFree(&R->P[1].HostName);
  MUFree(&R->P[1].CSKey);

  /* clear structure */
 
  memset(R,0,sizeof(mrm_t));
 
  return(SUCCESS);
  }  /* END MRMDestroy() */

 


int MRMCheckConfig(

  mrm_t *R)  /* I */

  {
  if (R == NULL)
    {
    return(FAILURE);
    }

  /* NYI */

  return(SUCCESS);
  }  /* END MRMCheckConfig() */



 
int MRMShow(
  
  mrm_t *RP,      /* I */
  char  *Buffer,  /* O */
  int    BufSize, /* I */
  int    Mode)    /* I */

  {
  int rmindex;
  int index;
  int findex;

  int ShowHeader = TRUE;
  int FailureRecorded;

  char *BPtr;
  int   BSpace;

  mrm_t *R;

  if (Buffer == NULL)
    {
    return(FAILURE);
    }

  BPtr   = Buffer;
  BSpace = BufSize;

  BPtr[0] = '\0';

  /* NOTE:  allow mode to specify verbose, diagnostics, etc */

  for (rmindex = 0;MRM[rmindex].Type != mrmtNONE;rmindex++)
    {
    R = &MRM[rmindex];

    if ((RP != NULL)  && (RP != R))
      continue;

    if (ShowHeader == TRUE)
      {
      ShowHeader = FALSE;
      }

    MUSNPrintF(&BPtr,&BSpace,"RM[%s]  type: '%s'  state: '%s'\n",
      R->Name,
      MRMType[R->Type],
      MRMState[R->State]);

    /* NOTE:  display optional RM version, failures, and fault tolerance config */

    if (R->APIVersion[0] != '\0')
      {
      MUSNPrintF(&BPtr,&BSpace,"  Version: '%s'\n",
        R->APIVersion);
      }

    if (R->EPort != 0)
      {
      MUSNPrintF(&BPtr,&BSpace,"  Event Management:  EPORT=%d\n",
        R->EPort);
      }
    else
      {
      MUSNPrintF(&BPtr,&BSpace,"  Event Management:  (event interface disabled)\n",
        R->APIVersion);
      }

    /* display RM specific attributes */

    switch(R->Type)
      {
      case mrmtPBS:

        if (R->U.PBS.PBS5IsEnabled == TRUE)
          {
          MUSNPrintF(&BPtr,&BSpace,"  PBSv5 protocol enabled\n");
          }

        if (R->U.PBS.SSSIsEnabled == TRUE)
          {
          MUSNPrintF(&BPtr,&BSpace,"  SSS protocol enabled\n");
          }

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch(R->Type) */

    /* NOTE:  stats in ms */

    if (R->RespTotalCount[0] > 0)
      {
      MUSNPrintF(&BPtr,&BSpace,"  RM Performance:  Avg Time: %.2lfs  Max Time:  %.2lfs  (%d samples)\n",
        (double)R->RespTotalTime[0] / R->RespTotalCount[0] / 1000,
        (double)R->RespMaxTime[0] / 1000,
        R->RespTotalCount[0]);
      }

    FailureRecorded = FALSE;

    for (index = 0;index < MAX_MRMFAILURE;index++)
      {
      findex = (index + R->FailIndex) % MAX_MRMFAILURE;

      if (R->FailTime[findex] <= 0)
        continue;

      if (FailureRecorded == FALSE)
        {
        MUSNPrintF(&BPtr,&BSpace,"\nRM[%s] Failures: \n",
          R->Name);

        FailureRecorded = TRUE;
        }

      MUSNPrintF(&BPtr,&BSpace,"  %19.19s  %-15s  '%s'\n",
        MULToDString((mulong *)&R->FailTime[findex]),
        MRMFuncType[R->FailType[findex]],
        (R->FailMsg[findex] != NULL) ? R->FailMsg[findex] : "no msg");
      }  /* END for (index) */ 

    MUSNPrintF(&BPtr,&BSpace,"\n");
    }  /* END for (rmindex) */

  /* NOTE:  temp hack.  add AM diagnostics to output */

  MAMShow(NULL,BPtr,BSpace,Mode);

  return(SUCCESS);
  }  /* END MRMShow() */




int MRMConfigShow(
 
  mrm_t *RP,      /* I (optional) */
  int    Mode,    /* I */
  char  *Buffer,  /* O */
  int    BufSize) /* I */

  {
  int  rmindex;
  int  aindex;
 
  char tmpLine[MAX_MLINE];
  char tmpVal[MAX_MLINE];
 
  mrm_t *R;

  char *BPtr;
  int   BSpace;

  const int AList[] = {
    mrmaAuthType,
    mrmaConfigFile,
    mrmaCSAlgo,
    mrmaCSKey,
    mrmaEPort,
    mrmaHost,
    mrmaLocalDiskFS,
    mrmaMinETime,
    mrmaNMPort,
    mrmaNMServer,
    mrmaPort,
    mrmaSocketProtocol,
    mrmaSuspendSig,
    mrmaTimeout,
    mrmaType,
    mrmaVersion,
    mrmaWireProtocol,
    -1 };

  const char *FName = "MRMConfigShow";

  DBG(6,fSTRUCT) DPrint("%s(%s,%d,%s,%d)\n",
    FName,
    (RP != NULL) ? "RP" : "NULL",
    Mode,
    (Buffer != NULL) ? "Buffer" : "NULL",
    BufSize);

  if (Buffer == NULL)
    {
    return(FAILURE);
    }

  BPtr   = Buffer;
  BSpace = BufSize;

  BPtr[0] = '\0';
 
  if (RP == NULL)
    {
    /* display loaded RM modules */

    MUSNPrintF(&BPtr,&BSpace,"# RM MODULES: ");

#if defined(__MLL22) || defined(__MLL31)
    MUSNPrintF(&BPtr,&BSpace,"LL ");
#endif /* __MLL22 || __MLL31 */

#ifdef __MLSF
    MUSNPrintF(&BPtr,&BSpace,"LSF ");
#endif /* __MLSF */

#ifdef __MPBS
    MUSNPrintF(&BPtr,&BSpace,"PBS ");
#endif /* __MPBS */

#ifdef __MRMS
    MUSNPrintF(&BPtr,&BSpace,"RMS ");
#endif /* __MRMS */

#ifdef __MSGE
    MUSNPrintF(&BPtr,&BSpace,"SGE ");
#endif /* __MSGE */

    MUSNPrintF(&BPtr,&BSpace,"SSS ");

    MUSNPrintF(&BPtr,&BSpace,"WIKI ");

    MUSNPrintF(&BPtr,&BSpace,"NATIVE ");

    MUSNPrintF(&BPtr,&BSpace,"\n");
    }  /* END if (RP == NULL) */
 
  /* NOTE:  allow mode to specify verbose, etc */
 
  for (rmindex = 0;rmindex < MAX_MRM;rmindex++)
    {
    R = &MRM[rmindex];
 
    if ((RP != NULL) && (RP != R))
      continue;
 
    tmpLine[0] = '\0';
 
    if (R->Type == mrmtNONE)
      continue;
 
    sprintf(tmpLine,"RMCFG[%s]",
      R->Name);
 
    for (aindex = 0;AList[aindex] != -1;aindex++)
      {
      tmpVal[0] = '\0';
 
      MRMAToString(R,AList[aindex],tmpVal,0);

      if (tmpVal[0] != '\0')
        {
        sprintf(tmpLine,"%s %s=%s",
          tmpLine,
          MRMAttr[AList[aindex]],
          tmpVal);
        }
      }    /* END for (aindex) */
 
    MUSNPrintF(&BPtr,&BSpace,"%s\n",
      tmpLine);
    }  /* END for (rmindex) */
 
  return(SUCCESS);
  }  /* END MRMConfigShow() */




int MRMOConfigShow(
  
  mrm_t *RP,     /* I */
  int    VFlag,  /* I */
  int    PIndex, /* I */
  char  *Buffer) /* O */

  {
  int rmindex;

  mrm_t *R;

  mrm_t ZR;

  if (Buffer == NULL)
    {
    return(FAILURE);
    }

  memset(&ZR,0,sizeof(ZR));

  for (rmindex = 0;rmindex < MAX_MRM;rmindex++)
    {
    R = &MRM[rmindex];

    if (!VFlag && (PIndex != -1) && (PIndex != pRMType))
      continue;

    if (!memcmp(&ZR,R,sizeof(ZR)))
      continue;
 
    if ((R->Type == mrmtNONE) && !VFlag)
      continue;

    if (R->Type == mrmtLL)
      { 
      if ((R->U.LL.ConfigFile[0] != '\0') || (VFlag || (PIndex == -1) || (PIndex == pRMConfigFile)))
        strcat(Buffer,MUShowSArray(MParam[pRMConfigFile],rmindex,R->U.LL.ConfigFile));
      }
 
    if ((VFlag || (PIndex == -1) || (PIndex == pRMAuthType)))
      strcat(Buffer,MUShowSArray(MParam[pRMAuthType],rmindex,(char *)MRMAuthType[R->AuthType]));
 
    strcat(Buffer,"\n");
    }    /* END for (rmindex) */

  return(SUCCESS);
  }  /* END MRMOConfigShow() */




int MRMReqPreLoad(
 
  mreq_t *RQ)  /* I */
 
  {
  if (RQ == NULL)
    {
    return(FAILURE);
    }

  if (RQ->NodeList != NULL)
    RQ->NodeList[0].N = NULL;
 
  RQ->MemCmp     = mcmpGE;
  RQ->DiskCmp    = mcmpGE;
  RQ->SwapCmp    = mcmpGE;
 
  RQ->DRes.Procs = 1;
  RQ->DRes.Mem   = 0;
  RQ->DRes.Disk  = 0;
  RQ->DRes.Swap  = 0;
 
  return(SUCCESS);
  }  /* END MRMReqPreLoad() */




int MRMJobPreUpdate(
 
  mjob_t *J)  /* I */
 
  {
  const char *FName = "MRMJobPreUpdate";
 
  DBG(5,fRM) DPrint("%s(%s)\n",
    FName,
    J->Name);

  /* NO-OP */

  return(SUCCESS);
  }  /* END MRMJobPreUpdate() */




int MRMFinalizeCycle()

  {
  int rc;
  int SC;

  int rmindex;

  mrm_t *R;

  for (rmindex = 0;MRM[rmindex].Type != mrmtNONE;rmindex++)
    {
    R = &MRM[rmindex];

    if (MRMFunc[R->Type].CycleFinalize == NULL)
      {
      DBG(7,fRM) DPrint("INFO:     cannot finalize RM cycle (RM '%s' does not support function '%s')\n",
        R->Name,
        MRMFuncType[mrmCycleFinalize]);

      continue;
      }

    MUThread(
      (int (*)(void))*MRMFunc[R->Type].CycleFinalize,
      R->P[0].Timeout,
      &rc,
      2,
      NULL,
      R,
      &SC);

    if (rc == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot finalize RM cycle (RM '%s' failed in function '%s')\n",
        R->Name,
        MRMFuncType[mrmCycleFinalize]);

      continue;
      }
    }    /* END for (rmindex) */

  return(SUCCESS);
  }  /* END MRMFinalizeCycle() */




int MRMSetAttr(

  mrm_t  *R,      /* I (modified) */
  int     AIndex, /* I */
  void  **Value,  /* I */
  int     Format, /* I */
  int     Mode)   /* I */

  {
  const char *FName = "MRMSetAttr";

  DBG(7,fSCHED) DPrint("%s(%s,%d,Value,%d)\n",
    FName,
    (R != NULL) ? R->Name : "NULL",
    AIndex,
    Mode);

  if (R == NULL)
    {
    return(FAILURE);
    }

  switch(AIndex)
    {
    case mrmaCSAlgo:

      R->P[0].CSAlgo = MUGetIndex((char *)Value,MCSAlgoType,FALSE,R->P[0].CSAlgo);
      R->P[1].CSAlgo = MUGetIndex((char *)Value,MCSAlgoType,FALSE,R->P[1].CSAlgo);

      break;

    case mrmaCSKey:

      {
      char *ptr;
      char *TokPtr;

      /* FORMAT:  <KEY>[,<KEY>] */

      ptr = MUStrTok((char *)Value,",:",&TokPtr);

      if (ptr != NULL)
        {
        MUStrDup(&R->P[0].CSKey,(char *)Value);
   
        ptr = MUStrTok(NULL,",:",&TokPtr);
    
        if (ptr != NULL)
          {
          MUStrDup(&R->P[1].CSKey,(char *)Value);
          }
        }    /* END if (ptr != NULL) */
      }      /* END BLOCK */

      break;

    case mrmaEPort:

      R->EPort = (int)strtol((char *)Value,NULL,0);

      break;

    case mrmaHost:

      if (Value != NULL)
        strcpy(R->P[0].HostName,(char *)Value);

      break;

    case mrmaPort:

      if (Value != NULL)
        {
        if (Format == mdfString)
          {
          R->P[0].Port = (int)strtol((char *)Value,NULL,0);
          }
        else
          {
          R->P[0].Port = *(int *)Value;
          }
        }

      break;

    case mrmaSuspendSig:

      if (Value == NULL)
        break;

      if (Format == mdfString)
        {
        MUStrCpy(R->SuspendSig,(char *)Value,sizeof(R->SuspendSig));
        }
      else
        {
        sprintf(R->SuspendSig,"%d",
          *(int *)Value);
        }

      break;

    default:

      /* attribute not handled */

      return(FAILURE);

      /*NOTREACHED*/

      break;    
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MRMSetAttr() */




int MRMNodePreLoad(

  mnode_t *N,      /* I (modified) */
  int      NState, /* I */
  mrm_t   *R)      /* I */

  {
  const char *FName = "MRMNodePreLoad";

  DBG(5,fRM) DPrint("%s(%s,%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    MAList[eNodeState][NState],
    (R != NULL) ? R->Name : "NULL");

  if (N == NULL)
    {
    return(FAILURE);
    }

  MSched.EnvChanged = TRUE;

  memset(N->FBM,0,sizeof(N->FBM));

  N->Network    = 0;

  N->RM         = R;

  N->State      = NState;

  N->TaskCount  = 0;
  N->EProcCount = 0;

  memset(&N->CRes,0,sizeof(N->CRes));
  memset(&N->DRes,0,sizeof(N->DRes));
  memset(&N->ARes,0,sizeof(N->ARes));

  N->DRes.Procs = -1;
  N->CRes.Procs = -1;

  if (MSched.DefaultN.CRes.Disk > 0)
    N->CRes.Disk = MSched.DefaultN.CRes.Disk;

  N->ARes.Procs = -1;
  N->ARes.Mem   = -1;
  N->ARes.Swap  = -1;
  N->ARes.Disk  = -1;

  N->URes.Procs = -1;

  N->ARes.PSlot[0].count = MAX_INT;

  /* set default partition */

  N->PtIndex         = 1;

  N->EState          = N->State;
  N->SyncDeadLine    = MAX_MTIME;
  N->StateMTime      = MSched.Time;

  N->R[0]            = NULL;

  N->CTime           = MSched.Time;
  N->MTime           = MSched.Time;
  N->ATime           = MSched.Time;

  N->STTime          = 0;
  N->SUTime          = 0;
  N->SATime          = 0;

  return(SUCCESS);
  }  /* END MRMNodePreLoad() */





int MRMNodePostLoad(

  mnode_t *N)  /* I */

  {
  mclass_t *C;

  char      tmpName[MAX_MNAME];

  const char *FName = "MRMNodePostLoad";

  DBG(5,fRM) DPrint("%s(%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL");

  if (N == NULL)
    {
    return(FAILURE);
    }

  /* load node configuration */

  MNodeLoadConfig(N,NULL);

  strcpy(tmpName,MNodeAdjustName(N->Name,1));

  if (strcmp(N->Name,tmpName) != 0)
    MUStrDup(&N->FullName,tmpName);

  if (N->CTime == 0)
    {
    /* initialize node timestamps */

    N->CTime = MSched.Time;
    N->MTime = MSched.Time;
    N->ATime = MSched.Time;
    }

  /* set default values */

  if (N->CRes.Disk == -1)
    {
    if (MSched.DefaultN.CRes.Disk > 0)
      N->CRes.Disk = MSched.DefaultN.CRes.Disk;
    else
      N->CRes.Disk = 1;
    }

  if (N->CRes.Mem == -1)
    {
    if (MSched.DefaultN.CRes.Mem > 0)
      N->CRes.Mem = MSched.DefaultN.CRes.Mem;
    else
      N->CRes.Mem = 1;
    }

  if (N->CRes.Swap == -1)
    {
    if (MSched.DefaultN.CRes.Swap > 0)
      N->CRes.Swap = MSched.DefaultN.CRes.Swap;
    else
      N->CRes.Swap = 1;
    }

  N->CRes.Mem = MAX(N->ARes.Mem,N->CRes.Mem);

  if (N->ARes.Mem == -1)
    N->ARes.Mem = N->CRes.Mem;

  N->CRes.Swap = MAX(N->ARes.Swap,N->CRes.Swap);

  if (N->ARes.Swap == -1)
    N->ARes.Swap = N->CRes.Swap;

  N->CRes.Disk = MAX(N->ARes.Disk,N->CRes.Disk);

  if (N->ARes.Disk == -1)
    N->ARes.Disk = N->CRes.Disk;

  if (N->URes.Procs == -1)
    N->URes.Procs = (int)N->Load;

  if (memchr(N->CRes.PSlot,'\0',sizeof(N->CRes.PSlot)) == NULL)
    {
    MClassAdd(DEFAULT,&C);

    N->CRes.PSlot[C->Index].count = 1;
    N->CRes.PSlot[0].count        = 1;
    }

  /* NOTE: must properly calculate available procs */

  if (N->CRes.Procs == -1)
    N->CRes.Procs = 1;

  if ((MPar[0].NAvailPolicy[0] == mrapUtilized) ||
      (MPar[0].NAvailPolicy[0] == mrapCombined))
    {
    /* use utilized resource info */

    if ((MSched.MaxOSCPULoad > 0.0) &&
        (N->DRes.Procs == 0) &&
        (N->ARes.Procs < N->CRes.Procs))
      {
      N->ARes.Procs = MAX(0,N->ARes.Procs - (int)(MSched.MaxOSCPULoad * N->CRes.Procs));
      }
    }  /* END if (((MPar[0].NAvailPolicy[pindex] == mrapUtilized) ... */

  /* restore checkpoint node information */

  MCPRestore(mcpNode,N->Name,(void *)N);

  /* connect to existing reservations */

  MNodeUpdateResExpression(N);

  if (N->ProcSpeed > 0)
    MSched.ReferenceProcSpeed = MIN(MSched.ReferenceProcSpeed,N->ProcSpeed);

  if (N->Speed <= 0.0)
    N->Speed = 1.0;

  MNodeGetLocation(N);

  if ((N->MaxProcPerClass != NULL) || (MSched.DefaultN.MaxProcPerClass != NULL))
    {
    int cindex;
    int tmpI;

    for (cindex = 0;cindex < MAX_MCLASS;cindex++)
      {
      if (N->CRes.PSlot[cindex].count <= 0)
        continue;

      if ((N->MaxProcPerClass != NULL) && (N->MaxProcPerClass[cindex] > 0))
        {
        tmpI = N->MaxProcPerClass[cindex];
        }
      else if ((MSched.DefaultN.MaxProcPerClass != NULL) && 
               (MSched.DefaultN.MaxProcPerClass[cindex] > 0))
        {
        tmpI = MSched.DefaultN.MaxProcPerClass[cindex];
        }
      else
        {
        continue;
        }

      N->CRes.PSlot[cindex].count = MIN(N->CRes.PSlot[cindex].count,tmpI);
      N->CRes.PSlot[cindex].count = MIN(N->CRes.PSlot[cindex].count,tmpI);
      }    /* END for (cindex) */
    }      /* END if ((N->MaxProcPerClass != NULL) || ...) */

  if ((N->RM == NULL) || (N->RM->Type != mrmtWiki))
    {
    MNodeResetJobSlots(N);
    }

  return(SUCCESS);
  }  /* END MRMNodePostLoad() */




int MRMNodePreUpdate(

  mnode_t *N,       /* I */
  int      NState,  /* I */
  mrm_t   *R)       /* I */

  {
  int     OldState;

  const char *FName = "MRMNodePreUpdate";

  DBG(4,fRM) DPrint("%s(%s,%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    MAList[eNodeState][NState],
    (R != NULL) ? R->Name : "NULL");

  if (N == NULL)
    {
    return(FAILURE);
    }

  /* update node statistics */

  N->STTime += MSched.Interval / 100;

  if ((N->State != mnsDown) &&
      (N->State != mnsNone) &&
      (N->State != mnsFlush) &&
      (N->State != mnsDraining))
    {
    N->SUTime += MSched.Interval / 100;

    if (N->State != mnsIdle)
      {
      N->SATime += MSched.Interval / 100;
      }
    }

  /* update node state */
  
  OldState = N->State;

  N->State = NState;

  /* update node utilization */

  memset(&N->DRes,0,sizeof(N->DRes));

  N->TaskCount      = 0;
  N->EProcCount     = 0;

  if (MSched.Mode == msmSim)
    {
    /* NO-OP */
    }
  else
    {
    N->Load = 0.0;

    memset(&N->ARes,0,sizeof(N->ARes));

    N->ARes.Procs     = -1;
    N->ARes.Mem       = -1;
    N->ARes.Swap      = -1;
    N->ARes.Disk      = -1;
    N->ARes.PSlot[0].count = MAX_INT;
    }

  /* if node state (Busy -> Idle) transition, verify node is Idle */

  if (((N->EState == mnsActive) ||
       (N->EState == mnsBusy)) &&
       (N->State == mnsIdle))
    {
    /* job completed within a single iteration.  synchronize node */

    if (MNodeCheckAllocation(N) != SUCCESS)
      {
      DBG(4,fRM) DPrint("ALERT:    node '%s' expected Busy but is Idle and not allocated\n",
        N->Name);

      N->EState = mnsIdle;
      }
    else
      {
      DBG(4,fRM) DPrint("ALERT:    node '%s' expected Active but is Idle and allocated\n",
        N->Name);
      }
    }

  /* handle node state changes */

  if (N->State != OldState)
    {
    DBG(2,fRM) DPrint("INFO:     node '%s' changed states from %s to %s\n",
      N->Name,
      MAList[eNodeState][OldState],
      MAList[eNodeState][N->State]);

    MSched.EnvChanged = TRUE;

    N->StateMTime = MSched.Time;

    /* handle <ANYSTATE> to 'Down/Drain/Flush' state transitions */

    if ((N->State == mnsDown) ||
        (N->State == mnsDraining) ||
        (N->State == mnsDrained) ||
        (N->State == mnsFlush))
      {
      N->EState       = N->State;
      N->SyncDeadLine = MAX_MTIME;
      }

    /* handle Down/Drain/Flush state to <any state> transitions */

    if ((OldState == mnsDown) ||
        (OldState == mnsDraining) ||
        (OldState == mnsDrained) ||
        (OldState == mnsFlush) ||
        (OldState == mnsReserved))
      {
      N->EState       = N->State;
      N->SyncDeadLine = MAX_MTIME;
      }

    if ((OldState == mnsIdle) && (N->State == mnsBusy))
      {
      if (MSched.Mode != msmTest)
        {
        DBG(1,fRM) DPrint("ALERT:    unexpected node transition on node '%s'  Idle -> Busy\n",
          N->Name);
        }

      N->EState       = N->State;
      N->SyncDeadLine = MAX_MTIME;
      }
    }   /* END if (N->State != OldState) */

  return(SUCCESS);
  }  /* END MRMNodePreUpdate() */





int MRMNodePostUpdate(

  mnode_t *N,         /* I (modified) */
  int      OldState)  /* I */

  {
  const char *FName = "MRMNodePostUpdate";

  DBG(5,fRM) DPrint("%s(%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    MNodeState[OldState]);

  if (N == NULL)
    {
    return(FAILURE);
    }

  N->ATime = MSched.Time;

  /* NOTE:  assume modification on each iteration */

  N->MTime = MSched.Time;
  
  /* set default values */

  N->CRes.Mem = MAX(N->ARes.Mem,N->CRes.Mem);

  if (N->ARes.Mem == -1)
    N->ARes.Mem = N->CRes.Mem;

  /* NOTE: attempt to obtain MAX swap over time */

  N->CRes.Swap = MAX(N->ARes.Swap,N->CRes.Swap);

  if (N->ARes.Swap == -1)
    N->ARes.Swap = N->CRes.Swap;

  if (N->CRes.Disk == -1)
    N->CRes.Disk = 1;

  N->CRes.Disk = MAX(N->ARes.Disk,N->CRes.Disk);

  if (N->ARes.Disk == -1)
    N->ARes.Disk = N->CRes.Disk;

  if (N->CRes.Procs == -1)
    N->CRes.Procs = 1;

  if (N->URes.Procs == -1)
    N->URes.Procs = (int)N->Load;

  /* NOTE:  ARes.Procs and DRes.Procs should be updated in MNodeUpdateState() */

/*
    if (N->ARes.Procs == -1)
      {
      N->ARes.Procs = MAX(0,N->CRes.Procs - N->DRes.Procs);
      }
*/

  if ((MPar[0].NAvailPolicy[0] == mrapUtilized) ||
      (MPar[0].NAvailPolicy[0] == mrapCombined))
    {
    /* use utilized resource information */

    if ((MSched.MaxOSCPULoad > 0.0) &&
        (N->DRes.Procs == 0) &&
        (N->ARes.Procs < N->CRes.Procs))
      {
      N->ARes.Procs = MAX(
        0,
        N->ARes.Procs - (int)(MSched.MaxOSCPULoad * N->CRes.Procs));
      }
    }

  /* unspecifed state, DRes.Procs, and ARes.Procs handled in MNodeUpdateState() */

  if ((N->State != N->EState) && (MSched.Time > N->SyncDeadLine))
    {
    DBG(3,fRM) DPrint("INFO:     synchronizing node '%s' to state '%s' (expected state: '%s')\n",
      N->Name,
      MAList[eNodeState][N->State],
      MAList[eNodeState][N->EState]);

    N->EState = N->State;
    }

  /* handle 'down to non-down' state transitions */

  if (((OldState == mnsDown) || (OldState == mnsDrained)) && 
       (N->State != OldState))
    {
    MNodeUpdateResExpression(N);
    }

  if ((N->MaxProcPerClass != NULL) || 
      (MSched.DefaultN.MaxProcPerClass != NULL))
    {
    int cindex;
    int tmpI;

    for (cindex = 0;cindex < MAX_MCLASS;cindex++)
      {
      if (N->CRes.PSlot[cindex].count <= 0)
        continue;

      if ((N->MaxProcPerClass != NULL) && (N->MaxProcPerClass[cindex] > 0))
        {
        tmpI = N->MaxProcPerClass[cindex];
        }
      else if ((MSched.DefaultN.MaxProcPerClass != NULL) && 
               (MSched.DefaultN.MaxProcPerClass[cindex] > 0))
        {
        tmpI = MSched.DefaultN.MaxProcPerClass[cindex];
        }
      else
        {
        continue;
        }

      N->CRes.PSlot[cindex].count = MIN(N->CRes.PSlot[cindex].count,tmpI);
      }    /* END for (cindex) */
    }      /* END if ((N->MaxProcPerClass != NULL) || ...) */

  if ((N->RM == NULL) || (N->RM->Type != mrmtWiki))
    {
    MNodeResetJobSlots(N);
    }

  return(SUCCESS);
  }  /* END MRMNodePostUpdate() */




int MRMJobPreLoad(

  mjob_t *J,       /* I (modified) */
  char   *JobName, /* I */
  int     RMIndex) /* I */

  {
  const char *FName = "MRMJobPreLoad";

  DBG(5,fRM) DPrint("%s(J,%s,%d)\n",
    FName,
    (JobName != NULL) ? JobName : "NULL",
    RMIndex);

  if (J == NULL)
    {
    return(FAILURE);
    }

  MSched.EnvChanged = TRUE;

  J->RM = &MRM[RMIndex];

  if (JobName != NULL)
    MUStrCpy(J->Name,JobName,sizeof(J->Name));

  J->CTime = MSched.Time;
  J->MTime = MSched.Time;
  J->ATime = MSched.Time;

  /* set defaults */

  J->CMaxTime       = MAX_MTIME;
  J->TermTime       = MAX_MTIME;
  J->Hold           = 0;
  J->SystemPrio     = 0;

  J->SyncDeadLine   = MAX_MTIME;

  J->Bypass         = 0;

  J->AWallTime      = 0;
  J->SWallTime      = 0;

  J->R              = NULL;

  J->PAL[0]         = DEFAULT_MPARLIST;

  J->Request.TC = 1;

  J->State          = mjsIdle;

  J->SpecWCLimit[1] = 0;

  J->QReq           = NULL;

  if (J->NodeList != NULL)
    J->NodeList[0].N = NULL;

  return(SUCCESS);
  }  /* END MRMJobPreLoad() */





int MRMJobPostLoad(

  mjob_t *J,        /* I (modified) */
  short  *TaskList, /* I */
  mrm_t  *R)        /* I */

  {
  int       TC;

  mqos_t   *QDef;
  int       AccountFallback;
  enum MHoldReasonEnum       Reason;

  int       cindex;
  int       rqindex;

  mjob_t   *MJ;
  mreq_t   *RQ;
  mnode_t  *N;
  mgcred_t *A;

  char      ErrMsg[MAX_MLINE];
  char      tmpLine[MAX_MLINE];
  char      tmpFBAccount[MAX_MLINE];

  const char *FName = "MRMJobPostLoad";

  DBG(5,fRM) DPrint("%s(%s,TaskList,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if ((J == NULL) || (R == NULL))
    {
    DBG(1,fRM) DPrint("ERROR:    invalid job/RM pointer passed to %s()\n",
      FName);

    return(FAILURE);
    }

  MTRAPJOB(J,FName);

  /* NOTE:  determine req/def QOS, allocation status, resulting account/QOS, then create reservation */

  if (J->SubmitTime == 0)
    {
    DBG(1,fRM) DPrint("WARNING:  no QueueTime on job '%s'\n",
      J->Name);

    J->SubmitTime = MSched.Time;
    }

  AccountFallback = FALSE;

  J->SystemQueueTime = J->SubmitTime;

  J->EState   = J->State;

  /* set class cred */

  for (cindex = 1;cindex < MAX_MCLASS;cindex++)
    {
    if (J->Req[0]->DRes.PSlot[cindex].count > 0)
      {
      J->Cred.C = &MClass[cindex];

      break;
      }
    }  /* END for (cindex) */

  if (J->Cred.C != NULL)
    {
    if (J->Cred.C->NonExeType == TRUE)
      MJobSetState(J,mjsNotQueued);

    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      RQ = J->Req[0];

      MUBMOR(RQ->ReqFBM,J->Cred.C->DefFBM,MAX_MATTR);

      if (J->Cred.C->NAPolicy != 0)
        RQ->NAccessPolicy = J->Cred.C->NAPolicy;

      }  /* END for (rqindex) */
    }

  /* restore checkpointed job status (QOS/Hold/SPrio/Bypass) */

  MCPRestore(mcpJob,J->Name,(void *)J);

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

  if ((J->SpecWCLimit[1] == 0) || (J->SpecWCLimit[1] == -1))
    {
    DBG(4,fRM) DPrint("WARNING:  wallclock limit not specified for job %s\n",
      J->Name);

    if ((J->Cred.Q != NULL) &&
        (J->Cred.Q->L.JDef != NULL) &&
        (((mjob_t *)J->Cred.Q->L.JDef)->SpecWCLimit[0] > 0))
      {
      J->SpecWCLimit[1] = ((mjob_t *)J->Cred.Q->L.JDef)->SpecWCLimit[0];
      }
    else if ((J->Cred.C != NULL) &&
        (J->Cred.C->L.JDef != NULL) &&
        (((mjob_t *)J->Cred.C->L.JDef)->SpecWCLimit[0] > 0))
      {
      J->SpecWCLimit[1] = ((mjob_t *)J->Cred.C->L.JDef)->SpecWCLimit[0];
      }
    else
      {
      J->SpecWCLimit[1] = MPar[0].L.JP->HLimit[mptMaxWC][0];
      }

    DBG(4,fRM) DPrint("INFO:     wallclock limit set to %s on job %s\n",
      MULToTString(J->SpecWCLimit[1]),
      J->Name);
    }

  J->SpecWCLimit[0] = J->SpecWCLimit[1];

  if ((MPar[0].UseMachineSpeed == FALSE) ||
     ((J->State != mjsRunning) && (J->State != mjsStarting)))
    {
    J->WCLimit = J->SpecWCLimit[0];
    }
  else
    {
    double tmpD;

    MUNLGetMinAVal(J->NodeList,mnaSpeed,NULL,(void *)&tmpD);

    if (tmpD > 0.0)
      J->WCLimit = (long)((double)J->SpecWCLimit[0] / tmpD);
    }

  if (MPar[0].EnableMultiNodeJobs == FALSE)
    {
    J->Req[0]->TasksPerNode = J->Req[0]->TaskCount;
    }

  /* set tasklist/nodelist */

  if (TaskList != NULL)
    {
    int tindex = 0;

    if (J->Req[1] != NULL) 
      {
      for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
        {
        RQ = J->Req[rqindex];

        if (RQ->DRes.Procs == 0)
          {
          /* handle local resources */

          MReqAllocateLocalRes(J,RQ); 
          }
        }  /* END for (rqindex) */

      for (tindex = 0;tindex < MAX_MNODE;tindex++)
        {
        if (TaskList[tindex] == -1)
          break;
        }  /* END for (tindex) */

      MJobGetLocalTL(J,&TaskList[tindex],MAX_MNODE - tindex);
      }  /* END if (J->Req[1] != NULL) */

    if ((J->NodeList == NULL) &&
       ((J->NodeList = (mnalloc_t *)calloc(1,
           sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1))) == NULL))
      {
      DBG(1,fRM) DPrint("ALERT:    cannot allocate memory for job %s nodelist, errno: %d (%s)\n",
        J->Name,
        errno,
        strerror(errno));

      return(FAILURE);
      }

    MUNLFromTL(J->NodeList,TaskList,&J->NodeCount);

    memcpy(J->TaskMap,TaskList,sizeof(J->TaskMap));

    /* tasklist specified for primary req only */

    if (J->Req[0] != NULL)
      {
      memcpy(
        J->Req[0]->NodeList,
        J->NodeList,
        sizeof(mnalloc_t) * tindex);

      J->Req[0]->NodeList[tindex].N = NULL;
      }
    }    /* END if (TaskList != NULL) */

  /* check class constraints */

  if (MJobCheckClassJLimits(
        J,
        J->Cred.C,
        FALSE,
        tmpLine,
        sizeof(tmpLine)) == FAILURE)
    {
    char tmpMsg[MAX_MLINE];

    /* job does not meet class constraints */

    sprintf(tmpMsg,"job violates class configuration '%s'",
      tmpLine);

    MJobSetHold(
      J,
      (1 << mhBatch),
      MSched.DeferTime,
      mhrPolicyViolation,
      tmpLine);

    if (J->Req[0] == NULL)
      {
      /* if sim job removed in set hold */

      return(FAILURE);
      }
    }

  /* check account allocation status */

  if (MAM[0].FallbackAccount[0] != '\0')
    {
    if (MAMAllocJReserve(&MAM[0],J,TRUE,&Reason,ErrMsg) == FAILURE)
      {
      DBG(3,fRM) DPrint("WARNING:  cannot reserve allocation for job '%s', reason: %s\n",
        J->Name,
        MDefReason[Reason]);

      /* Bug in the maui-gold interaction when FALLBACKACCOUNT is enabled 
         neccessitates this double check */

      if (Reason == mhrNoFunds || Reason == mhrAMFailure)
        {
        /* If the fallbackaccount starts with a + then append it to the 
           primary accountname */

        if (MAM[0].FallbackAccount[0] == '+')
          {
          snprintf(tmpFBAccount,sizeof(tmpFBAccount),"%s%s",
            J->Cred.A->Name,
            MAM[0].FallbackAccount + 1);
          }
        else
          {
          strcpy(tmpFBAccount,MAM[0].FallbackAccount);
          }

        DBG(3,fRM) DPrint("WARNING:  insufficient allocation in account '%s' to run job '%s' (attempting fallback account '%s')\n",
          J->Cred.A->Name,
          J->Name,
          tmpFBAccount);

        if (MAcctFind(tmpFBAccount,&A) == SUCCESS)
          {
          J->Cred.A = A;

          AccountFallback = TRUE;
          }
        else
          {
          DBG(2,fRM) DPrint("ALERT:    cannot locate fallback account '%s'\n",
            tmpFBAccount);
          }
        }
      }
    else
      {
      /* adequate allocations available on primary account, release 'test' bank reservation */

#if !defined(__MQBANK29)
      if (MAMAllocResCancel(J->Cred.A->Name,J->Name,"allocation test",NULL,&Reason) == FAILURE)
        {
        DBG(1,fRM) DPrint("ERROR:    cannot cancel allocation reservation for job '%s', reason: %s\n",
          J->Name,
          MDefReason[Reason]);
        }
#endif /* __MQBANK29 */
      }
    }      /* END if (MSched.BankFallbackAccount[0] != '\0') */

  /* reset QOS in case fallback account utilized */

  if ((MQOSGetAccess(J,J->QReq,NULL,&QDef) == FAILURE) ||
      (J->QReq == NULL) ||
      (J->QReq == &MQOS[0]))
    {
    MJobSetQOS(J,QDef,0);

    if ((J->QReq != NULL) && (AccountFallback == FALSE))
      {
      DBG(2,fRM) DPrint("ALERT:    job '%s' cannot use requested QOS %s (deferring/setting QOS to default %s)\n",
        J->Name,
        J->QReq->Name,
        QDef->Name);

      if (MSched.Mode != msmSim)
        {
        /* job cannot run in simulation mode w/o admin intervention */

        MJobSetHold(
          J,
          (1 << mhDefer),
          MSched.DeferTime,
          mhrQOSAccess,
          "job cannot use requested QOS");
        }
      }
    }
  else
    {
    MJobSetQOS(J,J->QReq,0);
    }  /* END else ((MQOSGetAccess(J,J->QReq,NULL,&QDef) == FAILURE) || ... ) */

  MJobBuildCL(J);

  MJobUpdateFlags(J);

  /* determine partition mask */

  if (MJobGetPAL(J,J->SpecPAL,J->PAL,NULL) == FAILURE)
    {
    /* cannot utilize requested PAL */
 
    DBG(2,fRM) DPrint("ALERT:    cannot utilize requested PAL '%x'\n",
      J->SpecPAL[0]);
    }

  if (J->Request.TC != J->Req[0]->TaskRequestList[0])
    {
    J->Request.TC = 0;

    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      RQ = J->Req[rqindex];

      RQ->TaskCount = RQ->TaskRequestList[0];
      J->Request.TC += RQ->TaskRequestList[0];
      }  /* END for (rqindex) */
    }

  if (J->MasterJobName != NULL)
    {
    /* locate/update master job */

    if (MJobFind(J->MasterJobName,&MJ,0) == FAILURE)
      {
      DBG(3,fRM) DPrint("ALERT:    cannot locate master job '%s' for subjob '%s' (cancelling subjob)\n",
        J->MasterJobName,
        J->Name);

      /* cancel subjob */

      MRMJobCancel(J,"MOAB_ERROR:  cannot locate parent job\n",NULL);
      }
    else
      {
      int sjcount = 0;

      for (rqindex = 0;MJ->Req[rqindex] != NULL;rqindex++)
        {
        if (MJ->Req[rqindex]->RMIndex == R->Index)
          {
          strcpy(MJ->Req[rqindex]->SubJobName,J->Name);

          if (MJ->IState == mjsStaging)
            {
            MJ->Req[rqindex]->State = mjsStaged;

            DBG(3,fRM) DPrint("INFO:     located job '%s' subjob[%d] '%s' (RM: %s)\n",
              MJ->Name,
              rqindex,
              J->Name,
              R->Name);
            }
          else
            {
            DBG(3,fRM) DPrint("ALERT:    master job '%s' in state '%s' subjob[%d] '%s' (RM: %s) loaded\n",
              MJ->Name,
              MAList[eJobState][MJ->State],
              rqindex,
              J->Name,
              R->Name);
            }
          }    /* END if (MJ->Req[index].RMIndex == R->Index) */

        if (MJ->Req[rqindex]->State == mjsStaged)
          sjcount++;
        }  /* END  for (rqindex) */

      if (sjcount == rqindex)
        {
        if (MJ->IState == mjsStaging)
          {
          DBG(1,fRM) DPrint("INFO:     all subjobs located.  starting masterjob '%s'\n",
            MJ->Name);

          MJ->IState = mjsStaged;
          }
        }
      }     /* END else if (MJobFind()) */
    }       /* END if (J->MasterJobName != NULL) */
  else      /* (J->MasterJobName != NULL) */
    {
    if ((J->State == mjsIdle) && (J->EState != mjsDeferred))
      {
      /* adjust idle job statistics handled in MQueueSelectAllJobs() */

      /* NO-OP */
      }
    else if (MJOBISALLOC(J) == TRUE)
      {
      double tmpD;

      DBG(4,fRM) DPrint("INFO:     allocated job '%s' detected\n",
        J->Name);

      /* setup allocated node count */

      if (J->TaskCount == 0)
        {
        DBG(2,fRM) DPrint("WARNING:  job '%s' loaded in allocated state with no tasks allocated\n",
          J->Name);

        DBG(2,fRM) DPrint("INFO:     adjusting allocated procs to %d for job '%s'\n",
          MJobGetProcCount(J),
          J->Name);

        J->TaskCount = J->Request.TC;
        }

      /* sanity check start time */

      J->StartTime = MAX(J->StartTime,J->SubmitTime);

      {
      long tmpL;
 
      tmpL = J->StartTime + J->SWallTime + J->WCLimit - MSched.Time;

      J->RemainingTime = (tmpL > 0) ? tmpL : 0;
      }  /* END BLOCK */

      if (J->AWallTime <= 0)
        J->AWallTime = MSched.Time - J->StartTime - J->SWallTime;
      else
        J->AWallTime = MIN(J->AWallTime,MSched.Time - J->StartTime - J->SWallTime);

      /* adjust statistics */

      MStatUpdateSubmitJobUsage(J);

      /* evaluate allocated nodes */

      for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
        {
        int nindex;

        RQ = J->Req[rqindex];

        if (RQ->DRes.Procs == 0)
          {
          MReqAllocateLocalRes(J,RQ);
          }      /* END if (RQ->DRes.Procs == 0) */

        for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
          {
          if (RQ->NodeList[nindex].TC == 0)
            break;

          N  = RQ->NodeList[nindex].N;
          TC = RQ->NodeList[nindex].TC;

          N->TaskCount  += TC;
          N->EProcCount += TC * RQ->DRes.Procs;

          if ((RQ->NAccessPolicy == mnacSingleJob) ||
              (RQ->NAccessPolicy == mnacSingleTask))
            {
            MCResAdd(&N->DRes,&N->CRes,&N->CRes,1,FALSE);
            }
          else
            {
            MCResAdd(&N->DRes,&N->CRes,&RQ->DRes,TC,FALSE);
            }
          }    /* END for (nindex)  */

        /* determine partition */

        if ((RQ->PtIndex == 0) && !(J->Flags & (1 << mjfSpan)))
          {
          if ((RQ->NodeList != NULL) && (RQ->NodeList[0].N != NULL))
            RQ->PtIndex = RQ->NodeList[0].N->PtIndex;
          else
            RQ->PtIndex = 0;
          }
        }      /* END for (rqindex) */

      RQ = J->Req[0];

      if (J->DispatchTime == 0)
        {
        DBG(0,fRM) DPrint("ERROR:    job '%s' loaded in alloc state '%s' with NULL dispatch time\n",
          J->Name,
          MJobState[J->State]);

        J->DispatchTime = MSched.Time;

        /* hack to work around LL API bug */

        J->StartTime = J->DispatchTime;
        }

      MUNLGetMinAVal(J->NodeList,mnaSpeed,NULL,(void **)&tmpD);

      if (tmpD > 0.0)
        {
        J->WCLimit = (int)((double)J->SpecWCLimit[0] / tmpD);
        }

      if ((RQ->NodeList[0].N == NULL) ||
          (RQ->NodeList[0].TC == 0))
        memcpy(RQ->NodeList,J->NodeList,sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1));

      MResJCreate(J,NULL,J->DispatchTime,mjrActiveJob,NULL);

      if (MJOBISACTIVE(J) == TRUE)
        {
        MQueueAddAJob(J);
        }
      } /* END else if (MJOBISALLOC(J) == TRUE) */

    if (J->Req[1] == NULL)
      {
      RQ = J->Req[0];

      if (J->MReq == NULL)
        {
        RQ->TaskCount = J->Request.TC;
        }
      else
        {
        /* if master node requirements specified */

        RQ->TaskCount = J->Request.TC - 1;
        }
      }    /* END if (J->Req[1] == NULL) */
    }      /* END else (J->MasterName != NULL) */

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];

    RQ->TasksPerNode = MIN(RQ->TasksPerNode,RQ->TaskCount);

    if ((RQ->NodeList != NULL) && (RQ->NodeList[0].N != NULL))
      {
      RQ->PtIndex = RQ->NodeList[0].N->PtIndex;
      }
    else
      {
      RQ->PtIndex = 0;
      }
    }    /* END for (rqindex) */

  MJobUpdateResourceCache(J,0);

  if ((MPar[0].L.JP->HLimit[mptMaxWC][0] > 0) &&
      (J->WCLimit > MPar[0].L.JP->HLimit[mptMaxWC][0]))
    {
    MJobSetHold(
      J,
      (1 << mhBatch),
      MSched.DeferTime,
      mhrPolicyViolation,
      "job violates system max wclimit");
    }

  MJobCheckDataReq(J);

  if (X.XJobPostInit != (int (*)())0)
    {
    (*X.XJobPostInit)(X.xd,J,TRUE);
    }

  RQ = J->Req[0];  /* FIXME */

  /* FORMAT:                         N           TR  UN GN    WC   JS   UP    QT NET  OS  AR  MC MEM  DC DSK */

  DBG(1,fRM) DPrint("INFO:     job '%s' loaded: %3d %8s %8s %6ld %10s %3ld %10ld %8s %6s %6s %2s %6d %2s %6d %s %ld\n",
    J->Name,
    J->Request.TC,
    J->Cred.U->Name,
    J->Cred.G->Name,
    J->WCLimit,
    MJobState[J->State],
    J->UPriority,
    J->SubmitTime,
    MAList[eNetwork][RQ->Network],
    MAList[eOpsys][RQ->Opsys],
    MAList[eArch][RQ->Arch],
    MComp[RQ->MemCmp],
    RQ->RequiredMemory,
    MComp[RQ->DiskCmp],
    RQ->RequiredDisk,
    MUMAList(eFeature,RQ->ReqFBM,sizeof(RQ->ReqFBM)),
    J->ATime);

  DBG(6,fRM) DPrint("INFO:     job '%s' size: %d + %d\n",
    J->Name,
    J->ImageSize,
    J->ExecSize);

  if (MSched.Mode == msmSim)
    {
    /* NYI */
    }
  else if (MSched.Mode == msmNormal)
    {
    MOSSyslog(LOG_INFO,"new job %s loaded",
      J->Name);
    }

  return(SUCCESS);
  }  /* END MRMJobPostLoad() */




int MRMJobPostUpdate(

  mjob_t *J,                   /* I (modified) */
  short  *TaskList,            /* I */
  enum MJobStateEnum OldState, /* I */
  mrm_t  *R)                   /* I */

  {
  mreq_t   *RQ;

  int       index = 0;
  int       nindex = 0;
  int       rqindex = 0;
  int       tindex = 0;
  int       jnindex = 0;

  int       TC;

  int       SC;
  long      DeferTime;

  mbool_t   BuildTaskMap;

  int       RQ0TC = 0;
  int       RQ0NC = 0;

  mnode_t  *N;

  char      Message[MAX_MLINE << 4];
  char      tmpLine[MAX_MLINE];

  const char *FName = "MRMJobPostUpdate";

  DBG(5,fRM) DPrint("%s(%s,TaskList,%s,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    MAList[eJobState][OldState],
    (R != NULL) ? R->Name : "NULL");

  MTRAPJOB(J,FName);

  if ((J == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  J->ATime = MSched.Time;

  Message[0] = '\0';

  if (J->Cred.C != NULL)
    {
    if (J->Cred.C->NonExeType == TRUE)
      MJobSetState(J,mjsNotQueued);

    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      RQ = J->Req[0];

      MUBMOR(RQ->ReqFBM,J->Cred.C->DefFBM,MAX_MATTR);
      }  /* END for (rqindex) */
    }

  MJobUpdateFlags(J);

  /* synchronize active job transitions */

  if ((J->EState == mjsStarting) && (J->State == mjsRunning))
    {
    J->EState       = mjsRunning;
    J->SyncDeadLine = MAX_MTIME;
    }

  switch(OldState)
    {
    case mjsSuspended:

      J->SWallTime += (long)(MSched.Interval / 100);

      break;

    case mjsStarting:
    case mjsRunning:

      if (J->AWallTime <= 0)
        {
        J->AWallTime = MAX(0,(long)(MSched.Time - J->StartTime - J->SWallTime));
        }
      else if (MSched.Time > J->StartTime)
        {
        J->AWallTime += (long)(MSched.Interval / 100);
        }

      break;

    case mjsDeferred:

      J->EState       = J->State;
      J->SyncDeadLine = MAX_MTIME;

      break;

    case mjsIdle:
    case mjsNotQueued:
    case mjsHold:
    case mjsSystemHold:

      if ((J->State == mjsIdle) ||
          (J->State == mjsNotQueued) ||
          (J->State == mjsHold) ||
          (J->State == mjsSystemHold))
        {
        if (J->EState != mjsDeferred)
          {
          J->EState       = J->State;
          J->SyncDeadLine = MAX_MTIME;
          }

        if ((J->State != OldState) && (J->State == mjsIdle))
          {
          if (OldState != mjsSystemHold)
            J->SystemQueueTime = MSched.Time;
          }
        }
      else if (J->State == mjsRemoved)
        {
        /* handle cancelled jobs */

        DBG(1,fRM) DPrint("ALERT:   job '%s' was cancelled externally\n",
          J->Name);

        return(SUCCESS);
        }

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch (OldState) */

  /* determine wclimit */

  if (J->SpecWCLimit[1] == 0)
    {
    DBG(4,fRM) DPrint("WARNING:  wallclock limit not specified for job %s\n",
      J->Name);

    if ((J->Cred.Q != NULL) &&
        (J->Cred.Q->L.JDef != NULL) &&
        (((mjob_t *)J->Cred.Q->L.JDef)->SpecWCLimit[0] > 0))
      {
      J->SpecWCLimit[1] = ((mjob_t *)J->Cred.Q->L.JDef)->SpecWCLimit[0];
      }
    else if ((J->Cred.C != NULL) &&
        (J->Cred.C->L.JDef != NULL) &&
        (((mjob_t *)J->Cred.C->L.JDef)->SpecWCLimit[0] > 0))
      {
      J->SpecWCLimit[1] = ((mjob_t *)J->Cred.C->L.JDef)->SpecWCLimit[0];
      }
    else
      {
      J->SpecWCLimit[1] = MDEF_SYSJOBWCLIMIT;
      }
    }
  else if (J->SpecWCLimit[1] == (unsigned long)-1)
    {
    DBG(4,fRM) DPrint("INFO:     wallclock limit set to unlimited to job %s\n",
      J->Name);

    J->SpecWCLimit[1] = MPar[0].L.JP->HLimit[mptMaxWC][0];
    }

  /*
  if (MPar[0].L.JP->HLimit[mptMaxWC][0] > 0)
    J->SpecWCLimit[1] = MIN(J->SpecWCLimit[1],MPar[0].L.JP->HLimit[mptMaxWC][0]);
  */

  /* check class constraints */

  if (MJobCheckClassJLimits(J,J->Cred.C,FALSE,tmpLine,MAX_MLINE) == FAILURE)
    {
    char tmpMsg[MAX_MLINE];

    sprintf(tmpMsg,"job violates class configuration '%s'",
      tmpLine);

    /* job does not meet class constraints */

    MJobSetHold(
      J,
      (1 << mhBatch),
      MSched.DeferTime,
      mhrPolicyViolation,
      tmpLine);
    }

  J->SpecWCLimit[0] = J->SpecWCLimit[1];

  if ((MPar[0].UseMachineSpeed == FALSE) ||
     ((J->State != mjsRunning) && (J->State != mjsStarting)))
    {
    J->WCLimit = J->SpecWCLimit[0];
    }
  else
    {
    double tmpD;

    MUNLGetMinAVal(J->NodeList,mnaSpeed,NULL,(void **)&tmpD);

    if (tmpD > 0.0)
      J->WCLimit = (long)((double)J->SpecWCLimit[0] / tmpD);
    }

  /* adjust active job statistics/state */

  if (MJOBISACTIVE(J))
    {
    J->StartTime = MAX(J->StartTime,J->SubmitTime);

    if ((OldState == mjsHold) ||
        (OldState == mjsIdle) ||
        (OldState == mjsNotRun) ||
        (J->NodeList[0].N == NULL))
      {
      /* build new job list */

      J->NodeList[0].N = NULL;

      J->TaskMap[0] = -1;
      }
    else
      {
      /* check task index ordering */

      if (TaskList != NULL)
        {
        int JobHasMap;

        if (J->TaskMap[0] == -1)
          JobHasMap = FALSE;
        else
          JobHasMap = TRUE;

        RQ0NC = 0;
        RQ0TC = 0;

        if (J->Req[1] != NULL)
          {
          for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
            {
            RQ = J->Req[rqindex];

            if (RQ->DRes.Procs == 0)
              {
              /* handle local resources */

              MReqAllocateLocalRes(J,RQ);
              }
            }  /* END for (rqindex) */

          for (tindex = 0;tindex < MAX_MNODE;tindex++)
            {
            if (TaskList[tindex] == -1)
              break;
            }  /* END for (tindex) */
 
          MJobGetLocalTL(J,&TaskList[tindex],MAX_MNODE - tindex);

          /* NOTE:  must split resources across reqs (NYI) */

          RQ0TC = tindex;
          }  /* END if (J->Req[1] != NULL) */

        for (tindex = 0;TaskList[tindex] != -1;tindex++)
          {
          if (J->TaskMap[tindex] != TaskList[tindex])
            {
            if (JobHasMap == TRUE)
              {
              DBG(1,fRM) DPrint("ALERT:    task %d changed from '%s' to '%s' for active job '%s'\n",
                tindex,
                MNode[J->TaskMap[tindex]]->Name,
                MNode[TaskList[tindex]]->Name,
                J->Name);
              }

            /* rebuild map */

            J->NodeList[0].N = NULL;
            J->Req[0]->NodeList[0].N = NULL;

            break;
            }
          }    /* END for (tindex)             */
        }      /* END if (TaskList != NULL)    */
      }        /* END else (OldState == jHold) */

    if (TaskList != NULL)
      {
      int SNIndex = 0;

      int tmpNC;
      int tmpTC;

      if (J->NodeList[0].N == NULL)
        BuildTaskMap = TRUE;
      else
        BuildTaskMap = FALSE;

      for (tindex = 0;TaskList[tindex] != -1;tindex++)
        {
        N = MNode[TaskList[tindex]];

        if (tindex > MAX_MTASK_PER_JOB)
          {
          DBG(0,fRM) DPrint("WARNING:  job size exceeds internal maximum job size (%d)\n",
            MAX_MTASK_PER_JOB);

          break;
          }

        if (BuildTaskMap == TRUE)
          {
          /* build new node list */

          J->TaskMap[tindex] = TaskList[tindex];

          for (jnindex = 0;J->NodeList[jnindex].N != NULL;jnindex++)
            {
            if (J->NodeList[jnindex].N->Index == TaskList[tindex])
              {
              J->NodeList[jnindex].TC++;

              break;
              }
            }    /* END for (jnindex) */

          if (J->NodeList[jnindex].N == NULL)
            {
            J->NodeList[jnindex].N  = MNode[TaskList[tindex]];
            J->NodeList[jnindex].TC = 1;

            J->NodeList[jnindex + 1].N = NULL;
            }
          }   /* END if (BuildTaskMap == TRUE) */

        DBG(6,fRM) DPrint("INFO:     task %d assigned to job '%s'\n",
          tindex,
          J->Name);

        if (N->State == mnsIdle)
          {
          DBG(3,fRM) DPrint("ALERT:    RM state corruption.  job '%s' has idle node '%s' allocated (node forced to active state)\n",
            J->Name,
            N->Name);

          if (Message[0] == '\0')
            {
            sprintf(Message,"JOBCORRUPTION:  job '%s' has the following idle node(s) allocated:\n",
              J->Name);
            }

          if (strlen(Message) + 100 < sizeof(Message))
            {
            sprintf(Message,"%s '%s' ",
              Message,
              N->Name);
            }

          if (N->DRes.Procs >= N->CRes.Procs)
            N->EState = mnsBusy;
          else
            N->EState = mnsActive;
          }  /* END else if ((OldState == jHold) ... */
        }    /* END for (tindex) */

      /* terminate lists */

      if (BuildTaskMap == TRUE)
        J->TaskMap[tindex] = -1;

      for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
        {
        RQ = J->Req[rqindex + 1];

        tmpNC = 0;
        tmpTC = 0;

        for (nindex = SNIndex;J->NodeList[nindex].TC > 0;nindex++)
          {
          if (J->NodeList[nindex].N == NULL)
            break;

          if ((RQ != NULL) &&
              (J->NodeList[nindex].N == RQ->NodeList[0].N))
            {
            break;
            }

          tmpNC++;
          tmpTC += J->NodeList[nindex].TC;
          }  /* END for (nindex) */

        SNIndex = nindex;

        J->Req[rqindex]->TaskCount = tmpTC;
        J->Req[rqindex]->NodeCount = tmpNC;
        }  /* END for (rqindex) */

      J->NodeList[nindex].N = NULL;

      J->NodeCount = nindex;

      RQ = J->Req[0];  /* if req info is lost */

      if ((RQ->NodeList[0].N == NULL) ||
          (RQ->NodeList[0].TC == 0))
        {
        /* NOTE:  address primary req only */

        memcpy(
          RQ->NodeList,
          J->NodeList,
          sizeof(mnalloc_t) * (RQ->NodeCount + 1));
        }
      }  /* END if (TaskList != NULL) */

    RQ = J->Req[0];

    if (J->StartTime == 0)
      {
      DBG(0,fRM) DPrint("ERROR:    job '%s' updated in active state '%s' with NULL start time\n",
        J->Name,
        MJobState[J->State]);

      J->StartTime    = MSched.Time;
      J->DispatchTime = MSched.Time;
      }

    J->RemainingTime = (J->WCLimit > J->AWallTime) ? 
      J->WCLimit - J->AWallTime : 0;
   
    if (J->TaskCount <= 0)
      {
      DBG(2,fRM) DPrint("WARNING:  job '%s' loaded in active state with no tasks allocated\n",
        J->Name);

      DBG(2,fRM) DPrint("INFO:     adjusting allocated tasks for job '%s'\n",
        J->Name);

      J->TaskCount = J->Request.TC;

      if (J->NodeList != NULL)
        J->NodeList[0].N = NULL;

      if ((RQ != NULL) && (RQ->NodeList != NULL))
        RQ->NodeList[0].N = NULL;
      }  /* END if (J->TaskCount <= 0) */

    MQueueAddAJob(J);

    /* only make reservation on previously active jobs */

    if ((OldState == mjsStarting) || (OldState == mjsRunning))
      {
      RQ = J->Req[0];  /* FIXME:  if req info is lost */

      if ((RQ->NodeList[0].N == NULL) ||
          (RQ->NodeList[0].TC == 0))
        {
        memcpy(
          RQ->NodeList,
          J->NodeList,
          sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1));
        }

      if (J->R != NULL)
        {
        if (MResCheckJobMatch(J,J->R) == FAILURE)
          {
          MResDestroy(&J->R);

          MResJCreate(J,NULL,J->DispatchTime,mjrActiveJob,NULL);
          }
        }
      else
        {
        MResJCreate(J,NULL,J->DispatchTime,mjrActiveJob,NULL);
        }
      }
    }   /* END if (MJOBISACTIVE(J)) */
  else if (J->State == mjsSuspended)
    {
    RQ = J->Req[0];

    if ((RQ->NodeList[0].N == NULL) ||
        (RQ->NodeList[0].TC == 0))
      {
      /* NOTE:  address primary req only */

      memcpy(
        RQ->NodeList,
        J->NodeList,
        sizeof(mnalloc_t) * (RQ->NodeCount + 1));
      }
    }    /* END else if (J->State == mjsSuspended) */

  if (MJOBISACTIVE(J) || MJOBISCOMPLETE(J))
    {
    if (J->Req[1] != NULL)
      {
      int rqindex;

      J->Request.NC = 0;
      J->Request.TC = 0;

      J->TaskCount = 0;
      J->NodeCount = 0;

      for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
        {
        RQ = J->Req[rqindex];

        J->Request.NC += RQ->NodeCount;
        J->Request.TC += RQ->TaskCount;

        if (MJOBISACTIVE(J))
          {
          J->TaskCount += RQ->TaskCount;
          J->NodeCount += RQ->NodeCount;
          }
        }
      }    /* END if (J->Req[1] != NULL) */
    }      /* END if (MJOBISACTIVE(J) || MJOBISCOMPLETE(J)) */

  /* adjust completed/removed/vacated job statistics/state */

  if (MJOBISCOMPLETE(J))
    {
    if (J->StartTime == 0)
      {
      DBG(0,fRM) DPrint("ERROR:    Job[%03d] '%s' updated to completed state '%s' with NULL start time\n",
        J->Index,
        J->Name,
        MJobState[J->State]);

      /* make best guess */

      J->StartTime    = MSched.Time;
      J->DispatchTime = MSched.Time;
      }

    if (J->CompletionTime <= J->StartTime)
      {
      J->CompletionTime = MIN(
        MSched.Time,
        J->StartTime + J->SWallTime + MAX(J->AWallTime,1));
      }

    if (J->TaskCount <= 0)
      J->TaskCount = J->Request.TC;
    }  /* END if (MJOBISCOMPLETE(J)) */

  /* handle rejected jobs */

  if (((J->EState == mjsRunning) ||
       (J->EState == mjsStarting)) &&
      ((J->State == mjsIdle) ||
       (J->State == mjsHold)))
    {
    if (J->Cred.A != NULL)
      {
      if (MAMAllocResCancel(J->Cred.A->Name,J->Name,"job rejected",NULL,(enum MHoldReasonEnum *)&SC) == FAILURE)
        {
        DBG(1,fRM) DPrint("ERROR:    cannot cancel account reservation for job '%s'\n",
          J->Name);
        }
      }

    DeferTime = MSched.DeferTime * (1 + J->StartCount);

    J->NodeCount  = 0;
    J->TaskCount  = 0;
    J->TaskMap[0] = -1;

    DBG(2,fRM) DPrint("ALERT:    job '%s' was in state '%s' but is now in state '%s' (job was rejected)\n",
      J->Name,
      MJobState[J->EState],
      MJobState[J->State]);

    DBG(2,fRM) DPrint("ALERT:    job '%s' is being deferred for %ld seconds\n",
      J->Name,
      DeferTime);

    DBG(4,fRM)
      {
      DBG(4,fRM) DPrint("INFO:     job '%s' nodelist: ",
        J->Name);

      for (index = 0;index < MAX_MNODE_PER_JOB;index++)
        {
        if (J->NodeList[index].N == NULL)
          break;

        fprintf(mlog.logfp,"[%16s]",
          J->NodeList[index].N->Name);
        }  /* END for (index) */

      fprintf(mlog.logfp,"\n");
      }

    MJobSetHold(
      J,
      (1 << mhDefer),
      MSched.DeferTime,
      mhrRMReject,
      "job rejected by RM");
    }  /* END if (((J->EState == mjsRunning) || ... */

  if (MJOBISACTIVE(J))
    {
    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      RQ = J->Req[rqindex];

      for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
        {
        N = RQ->NodeList[nindex].N;

        TC = RQ->NodeList[nindex].TC;

        N->TaskCount  += TC;
        N->EProcCount += TC * RQ->DRes.Procs;

        if ((RQ->NAccessPolicy == mnacSingleJob) ||
            (RQ->NAccessPolicy == mnacSingleTask))
          {
          MCResAdd(&N->DRes,&N->CRes,&N->CRes,1,FALSE);
          }
        else
          {
          MCResAdd(&N->DRes,&N->CRes,&RQ->DRes,TC,FALSE);
          }
        }
      }    /* END for (rqindex) */

    if ((OldState == mjsHold) ||
        (OldState == mjsIdle) ||
        (OldState == mjsNotRun))
      {
      MResJCreate(J,NULL,J->DispatchTime,mjrActiveJob,NULL);
      }
    else
      {
      /* job was in active state on last iteration */

      if ((Message[0] != '\0') &&
          (MSched.Time - J->StartTime > 150))
        {
        /* indicate idle node allocated to active job */

        MSysRegEvent(Message,0,0,1);
        }
      }
    }    /* END   if (MJOBISACTIVE(J)) */

  /* handle external job start */

  if (((OldState == mjsHold) ||
       (OldState == mjsIdle) ||
       (OldState == mjsNotRun)) &&
      ((J->State == mjsRemoved) ||
       (J->State == mjsCompleted) ||
       (J->State == mjsVacated)))
    {
    DBG(4,fRM) DPrint("INFO:     job '%s' state transition (%s --> %s)\n",
      J->Name,
      MJobState[OldState],
      MJobState[J->State]);

    /* if job starts and completes while scheduler is sleeping (cannot happen with x scheduling) */

    if (J->EState != mjsRunning)
      {
      DBG(0,fRM) DPrint("WARNING:  scheduler cannot handle completion in 1 iteration on job '%s'\n",
        J->Name);

      /* set job to active so it can be properly removed */

      if (MAMAllocJReserve(&MAM[0],J,FALSE,(enum MHoldReasonEnum *)&SC,Message) == FAILURE)
        {
        DBG(3,fRM) DPrint("WARNING:  cannot create AM reservation for job '%s'\n",
          J->Name);
        }

      MQueueAddAJob(J);

      if (J->DispatchTime == 0)
        {
        if (J->State == mjsCompleted)
          {
          DBG(0,fRM) DPrint("ERROR:    Job[%03d] '%s' updated to completed state '%s' with NULL dispatch time\n",
            J->Index,
            J->Name,
            MJobState[J->State]);
          }

        J->DispatchTime = MSched.Time;
        }

      RQ = J->Req[0];  /* if req info is lost */

      if ((RQ->NodeList[0].N == NULL) ||
          (RQ->NodeList[0].TC == 0))
        {
        memcpy(RQ->NodeList,J->NodeList,sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1));
        }

      MResJCreate(J,NULL,J->DispatchTime,mjrActiveJob,NULL);

      /* complete job record */

      DBG(1,fRM) DPrint("INFO:     completing job '%s'\n",
        J->Name);
      }
    }     /* END if (((OldState == mjsHold) || ... */

  /* check job synchronization */

  if (J->State != J->EState)
    {
    if (MSched.Time > J->SyncDeadLine)
      {
      /* check if DeferTime is expired */

      if (J->EState == mjsDeferred)
        {
        DBG(2,fRM) DPrint("INFO:     restoring job '%s' from deferred state\n",
          J->Name);

        J->EState = J->State;
        J->SyncDeadLine  = MAX_MTIME;

        if (J->Hold & (1 << mhDefer))
          {
          J->Hold ^= (1 << mhDefer);
          }

        J->SystemQueueTime = MSched.Time;
        }
      }
    }

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];

    if ((RQ->NodeList != NULL) && (RQ->NodeList[0].N != NULL))
      {
      RQ->PtIndex = RQ->NodeList[0].N->PtIndex;
      }
    else
      {
      RQ->PtIndex = 0;
      }
    }    /* END for (rqindex) */

  RQ = J->Req[0];

  /* test mode patch for unexpected state changes */

  if ((J->NodeList[0].N == NULL) &&
      (RQ->NodeList[0].N != NULL))
    {
    memcpy(J->NodeList,RQ->NodeList,sizeof(mnalloc_t) * MAX_MNODE_PER_JOB);
    }

  MJobUpdateResourceCache(J,0);

  return(SUCCESS);
  }  /* END MRMJobPostUpdate() */





int MRMProcessOConfig(

  mrm_t   *R,      /* I (modified) */
  int      PIndex,
  int      IVal,
  double   DVal,
  char    *SVal,
  char   **SArray)

  {
  if (R == NULL)
    {
    return(FAILURE);
    }

  switch (PIndex)
    {
    case pRMPort:

      R->P[0].Port = IVal;

      break;

    case mcoRMEPort:

      R->EPort = IVal;

      break;

    case pRMNMPort:

      R->NMPort = IVal;

      break;

    case pRMTimeout:

      R->P[0].Timeout = MUTimeFromString(SVal);

      if (R->P[0].Timeout > 1000)
        R->P[0].Timeout = 1000;

      R->P[1].Timeout = R->P[0].Timeout;

      break;

    case pRMAuthType:

      R->AuthType = MUGetIndex(SVal,MRMAuthType,FALSE,0);

      break;

    case pRMHost:

      MUStrDup(&R->P[0].HostName,SVal);

      break;

    case pRMName:

      /* NOTE:  disabled */

      /* MUStrCpy(R->Name,SVal,sizeof(R->Name)); */

      break;

    case pRMType:

      {
      char *ptr;
      char *TokPtr;

      /* FORMAT:  <TYPE>[:<SUBTYPE>] */

      ptr = MUStrTok(SVal,":",&TokPtr);

      R->Type = MUGetIndex(ptr,MRMType,FALSE,mrmtNONE);

      if ((ptr = MUStrTok(NULL,":",&TokPtr)) != NULL)
        {
        R->SubType = MUGetIndex(ptr,MRMSubType,FALSE,mrmstNONE);
        }
      }    /* END BLOCK */

      break;

    case pRMConfigFile:

      MUStrCpy(R->U.LL.ConfigFile,SVal,sizeof(R->U.LL.ConfigFile));

      break;

    case pRMLocalDiskFS:

      MUStrCpy(R->U.PBS.LocalDiskFS,SVal,sizeof(R->U.PBS.LocalDiskFS));

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(PIndex) */

  return(SUCCESS);
  }  /* END MRMProcessOConfig() */




int MRMSetFailure(

  mrm_t *R,     /* I */
  int    FType, /* I */
  char  *FMsg)  /* I */

  {
  int findex;

  if (R == NULL)
    {
    return(FAILURE);
    }

  /* record failure */

  R->FailCount++;
  R->FailIteration = MSched.Iteration;

  findex = R->FailIndex;

  R->FailTime[findex] = MSched.Time;
  R->FailType[findex] = FType;

  if (FMsg != NULL)
    MUStrDup(&R->FailMsg[findex],FMsg);
  else
    MUFree(&R->FailMsg[findex]);

  R->FailIndex = (findex + 1) % MAX_MRMFAILURE;

  /* adjust interface state */
  
  return(SUCCESS);
  }  /* END MRMSetFailure() */




int __MRMStartFunc(

  mrm_t *R,     /* I */
  int    FType) /* I */

  {
  MUGetMS(NULL,&R->RespStartTime[FType]);
  
  return(SUCCESS);
  }  /* END __MRMStartFunc() */




int __MRMEndFunc(

  mrm_t *R,     /* I */
  int    FType) /* I */

  {
  long NowMS;
  long Interval;

  if (R->RespStartTime[FType] <= 0)
    {
    /* invalid time */

    return(FAILURE);
    }

  MUGetMS(NULL,&NowMS);

  if (NowMS < R->RespStartTime[FType])
    Interval = R->RespStartTime[FType] - NowMS;
  else
    Interval = NowMS - R->RespStartTime[FType];

  R->RespTotalTime[FType] += Interval;
  R->RespMaxTime[FType] = MAX(R->RespMaxTime[FType],Interval);
  R->RespTotalCount[FType]++;

  if (FType != 0)
    {
    R->RespTotalTime[0] += Interval;
    R->RespMaxTime[0] = MAX(R->RespMaxTime[0],Interval);
    R->RespTotalCount[0]++;
    }

  /* reset start time */

  R->RespStartTime[FType] = -1;

  return(SUCCESS);
  }  /* EMD __MRMEndFunc() */




int MNodeResetJobSlots(

  mnode_t *N)  /* I */
 
  {
  int cindex;
 
  mclass_t *C;

  if ((N->RM != NULL) && (N->RM->Type == mrmtLL))
    {
    /* job slots tracked by RM */

    return(SUCCESS);
    }  

  N->ARes.PSlot[0].count = 0;

  for (cindex = 1;cindex < MAX_MCLASS;cindex++)
    {
    C = &MClass[cindex];

    if ((N->CRes.PSlot[cindex].count > 0) && (C->IsDisabled == FALSE))
      {
      N->ARes.PSlot[cindex].count = N->CRes.PSlot[cindex].count;

      if (C->MaxProcPerNode > 0) 
        N->ARes.PSlot[cindex].count = MIN(N->ARes.PSlot[cindex].count,C->MaxProcPerNode);

      if (N->AP.HLimit[mptMaxProc][0] > 0)
        N->ARes.PSlot[cindex].count = MIN(N->ARes.PSlot[cindex].count,N->AP.HLimit[mptMaxProc][0]);
      }
    else
      {
      N->ARes.PSlot[cindex].count = 0;
      }

    N->ARes.PSlot[0].count += N->ARes.PSlot[cindex].count;
    }  /* END for (cindex) */

  N->ARes.PSlot[0].count = MIN(N->CRes.PSlot[0].count,N->ARes.PSlot[0].count); 

  if (C->MaxProcPerNode > 0)
    N->ARes.PSlot[0].count = MIN(N->ARes.PSlot[0].count,C->MaxProcPerNode);

  if (N->AP.HLimit[mptMaxProc][0] > 0)
    N->ARes.PSlot[0].count = MIN(N->ARes.PSlot[0].count,N->AP.HLimit[mptMaxProc][0]);

  return(SUCCESS);
  }  /* END MNodeResetJobSlots() */



/* END MRM.c */


