/* */
        
#include "moab.h"
#include "msched-proto.h"


extern mlog_t mlog;

extern mnode_t  *MNode[];
extern msched_t  MSched;
extern mstat_t   MStat;
extern mgcred_t  MAcct[];
extern mrm_t     MRM[];
extern mframe_t  MFrame[];
 
extern mattrlist_t MAList;
extern msys_t    MSys;
 
extern const char *MJobState[];
extern const char *MNodeState[];
extern const char *MLLJobState[];


#if !defined(__MLL) && !defined(__MLL2) && !defined(__MLL31)
#include "__MLLStub.c"
#endif /* !__MLL && !__MLL2  && !__MLL31 */

/* prototypes */

int MLLNodeSetAttrs(mnode_t *,LL_element *);
int LLClusterQuery(mrm_t *,int *,int *);
int MLLJobSuspend(mjob_t *,mrm_t *,char *,int *);
int MLLJobResume(mjob_t *,mrm_t *,char *,int *);
int MLLJobCheckpoint(mjob_t *,mrm_t *,mbool_t,char *,int *);
int MLLGetTL(LL_element *,short *,int *);
int MLLJobStart(mjob_t *,mrm_t *,char *,int *);
int MLLJobCancel(mjob_t *,mrm_t *,char *,char *,int *);
int MLLWorkloadQuery(mrm_t *,int *,int *);
int LLGetClassInfo(mnode_t *,char **,char **);
int MLLJobProcess(LL_element *,LL_element *,LL_element *,char *,mrm_t *,int);
int LL2GetProcList(LL_element *,short *,int *);
int MLLJobLoad(mjob_t *,LL_element *,LL_element *,char *,mrm_t *);
int MLLJobRequeue(mjob_t *,mrm_t *,mjob_t **,char *,int *);
int MLLJobUpdate(mjob_t *,LL_element *,LL_element *,LL_element *,mrm_t *);
int LLShowError(int,mjob_t *);
int LL2ShowError(int,mjob_t *,char *);
int LL2GetNodes(int *,int);
int MLLNodeUpdate(mnode_t *,LL_element *,enum MNodeStateEnum,mrm_t *);
int LLNodeLoad(mnode_t *,LL_element *,int,mrm_t *);
int LLSubmitJob(mjob_t *,int,char *,int *);
int __MLLJobStateToJState(int,int);
int LL2FreeData(mrm_t *);
int LLIInitialize(mrm_t *,int *);
int MLLJobSetAttr(mjob_t *,LL_element *,LL_element *,LL_element *,int);


#include "OLLI.c"




#if defined(__MLL) || defined(__MLL2) || defined(__MLL31)

int MLLLoadModule(
 
  mrmfunc_t *F)  /* I (modified) */
 
  {
  if (F == NULL)
    {
    return(FAILURE);
    }

  F->ClusterQuery   = LLClusterQuery; 
  F->JobCancel      = MLLJobCancel;
  F->JobModify      = NULL;
  F->JobQuery       = NULL;
  F->JobRequeue     = MLLJobRequeue;
  F->JobStart       = MLLJobStart;

#if defined(__MLL31)
  F->JobSuspend     = MLLJobSuspend;
  F->JobResume      = MLLJobResume;
  F->JobCheckpoint  = MLLJobCheckpoint;
#endif /* __MLL31 */

  F->QueueQuery     = NULL;
  F->ResourceModify = NULL;
  F->ResourceQuery  = NULL;
  F->RMInitialize   = LLIInitialize;
  F->RMQuery        = NULL;
  F->WorkloadQuery  = MLLWorkloadQuery;      
  F->RMEventQuery   = NULL;

  return(SUCCESS);
  }  /* END MLLLoadModule() */

#endif /* __MLL || __MLL2 || __MLL31 */





int LLIInitialize(
 
  mrm_t *R,  /* I */
  int   *SC) /* O */
 
  {
  const char *FName = "LLInitialize";

  DBG(1,fLL) DPrint("%s(%s,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");
 
  if (R == NULL)
    {
    return(FAILURE);
    }
 
  R->FailIteration = -1;

#ifdef LL_API_VERSION
  R->Version = LL_API_VERSION;
#endif /* LL_API_VERSION */
 
  return(SUCCESS);
  }  /* END LLIInitialize() */




int MLLJobSuspend(

  mjob_t  *J,   /* I */
  mrm_t   *R,   /* I */
  char    *Msg, /* O */
  int     *SC)  /* O */

  {
  char  tmpJobName[MAX_MLINE];

  const char *FName = "MLLJobSuspend";

  DBG(1,fLL) DPrint("%s(%s,%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if (Msg != NULL)
    Msg[0] = '\0';

  if (J == NULL)
    {
    return(FAILURE);
    }

  /* send signal to job */

  MJobGetName(J,NULL,R,tmpJobName,sizeof(tmpJobName),mjnRMName);

#if defined(__MLL31)
  {
  LL_element *ErrO = NULL;
  int         rc;

  rc = ll_preempt(
    R->Version,
    &ErrO,
    tmpJobName,
    PREEMPT_STEP);

  if (rc != API_OK)
    {
    char *ptr;

    /* log error message */

    ptr = ll_error(&ErrO,0);

    DBG(0,fLL) DPrint("ERROR:    job '%s' cannot be suspended: %s\n",
      J->Name,
      (ptr != NULL) ? ptr : "no message");

    MUFree(&ptr);

    if (R->FailIteration != MSched.Iteration)
      {
      R->FailIteration = MSched.Iteration;
      R->FailCount     = 0;
      }

    return(FAILURE);
    }
  }    /* END BLOCK */
#endif /* __MLL31 */

  /* adjust state */

  MJobSetState(J,mjsSuspended);

  /* release reservation */

  /* adjust stats */

  /* NYI */

  DBG(1,fLL) DPrint("INFO:     job '%s' successfully suspended\n",
    J->Name);

  return(SUCCESS);
  }  /* END MLLJobSuspend() */




int MLLJobResume(

  mjob_t  *J,   /* I */
  mrm_t   *R,   /* I */
  char    *Msg, /* O (optional) */
  int     *SC)  /* O (optional) */

  {
  char  tmpJobName[MAX_MNAME];

  const char *FName = "MLLJobResume";

  DBG(1,fLL) DPrint("%s(%s,%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if (Msg != NULL)
    Msg[0] = '\0';

  if (J == NULL)
    {
    return(FAILURE);
    }

  /* send signal to job */

  MJobGetName(J,NULL,R,tmpJobName,sizeof(tmpJobName),mjnRMName);

#if defined(__MLL31)
  {
  LL_element *ErrO = NULL;
  int         rc;

  rc = ll_preempt(
    R->Version,
    &ErrO,
    tmpJobName,
    RESUME_STEP);

  if (rc != API_OK)
    {
    char *ptr;

    /* log error message */

    ptr = ll_error(&ErrO,0);

    DBG(0,fLL) DPrint("ERROR:    job '%s' cannot be resumed: %s\n",
      J->Name,
      (ptr != NULL) ? ptr : "no message");

    if (R->FailIteration != MSched.Iteration)
      {
      R->FailIteration = MSched.Iteration;
      R->FailCount     = 0;
      }

    MUFree(&ptr);

    return(FAILURE);
    }  /* END if (rc != API_OK) */
  }    /* END BLOCK */
#endif /* __MLL31 */

  /* adjust state */

  MJobSetState(J,mjsRunning);

  /* adjust stats */

  /* NYI */

  DBG(1,fLL) DPrint("INFO:     job '%s' successfully resumed\n",
    J->Name);

  return(SUCCESS);
  }  /* END MLLJobResume() */




int MLLJobCheckpoint(

  mjob_t  *J,            /* I */
  mrm_t   *R,            /* I */
  mbool_t  TerminateJob, /* I */
  char    *Msg,          /* O */
  int     *SC)           /* O */

  {
  char  tmpJobName[MAX_MNAME];

  const char *FName = "MLLJobCheckpoint";

  DBG(1,fLL) DPrint("%s(%s,%s,%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL",
    (TerminateJob == TRUE) ? "TRUE" : "FALSE");

  if (Msg != NULL)
    Msg[0] = '\0';

  if (J == NULL)
    {
    return(FAILURE);
    }

  /* checkpoint job */

  MJobGetName(J,NULL,R,tmpJobName,sizeof(tmpJobName),mjnRMName);

#if defined(__MLL31)
  {
  LL_element   *ErrO = NULL;
  LL_ckpt_info  ci;

  int           rc;

  memset(&ci,0,sizeof(ci));

  ci.version = R->Version;
  ci.step_id = tmpJobName;

  /* NOTE:  LL CKPT_AND_HOLD not implemented */

  if (TerminateJob == TRUE)
    {
    ci.ckptType = CKPT_AND_TERMINATE;
    }
  else
    {
    ci.ckptType = CKPT_AND_CONTINUE;
    }

  /* call should not block while job checkpoints */

  ci.waitType = CKPT_NO_WAIT;

  /* NOTE:  use default 'hung' checkpoint signal */

  /* ci.abort_sig = SIGINT; */

  ci.cp_error_data = NULL;

  /* NOTE: do not set checkpoint timelimits */

  /* ci.soft_limit = 0; */
  /* ci.hard_limit = 0; */

  if ((rc = ll_ckpt(&ci)) != API_OK)
    {
    char *ptr;

    /* log error message */

    ptr = ll_error(&ErrO,0);

    DBG(0,fLL) DPrint("ERROR:    job '%s' cannot be checkpointed: %s\n",
      J->Name,
      (ptr != NULL) ? ptr : "no message");

    if (R->FailIteration != MSched.Iteration)
      {
      R->FailIteration = MSched.Iteration;
      R->FailCount     = 0;
      }

    MUFree(&ptr);

    return(FAILURE);
    }
  }    /* END BLOCK */
#endif /* __MLL31 */

  /* adjust state */

  MJobSetState(J,mjsRunning);

  /* adjust stats */

  /* NYI */


  DBG(1,fLL) DPrint("INFO:     job '%s' successfully resumed\n",
    J->Name);

  return(SUCCESS);
  }  /* END MLLJobCheckpoint() */




int MLLJobRequeue(
 
  mjob_t   *J,
  mrm_t    *R,
  mjob_t **JPeer,
  char     *Msg,
  int      *SC)
 
  {
#if defined(__MLL22) || defined(__MLL31) 
  int rc;
#endif /* __MLL22 || __MLL31 */
 
  int jindex;
  int index;
 
  char *HostList[2];
 
  mnode_t *N;
 
  char  Command[MAX_MLINE];
  char  Buffer[MAX_MLINE];

  const char *FName = "MLLJobRequeue";
 
  DBG(3,fLL) DPrint("%s(%s,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");
 
  if ((J == NULL) || (J->NodeList == NULL) || (J->NodeList[0].N == NULL))
    {
    return(FAILURE);
    }
 
  N = J->NodeList[0].N;
 
  HostList[0] = N->Name;
  HostList[1] = NULL;
 
  if ((MSched.Iteration == R->FailIteration) || (MSched.Mode == msmTest))
    {
    if (MSched.Mode == msmTest)
      {
      DBG(3,fLL) DPrint("INFO:     cannot requeue job '%s' (test mode)\n",
        J->Name);
      }
    else
      {
      DBG(3,fLL) DPrint("INFO:     cannot requeue job '%s' (fail iteration)\n",
        J->Name);
      }
 
    return(FAILURE);
    }
 
  /* verify job is restartable */
 
/*
  if (!(J->Flags & (1 << mjfRestartable)))
    {
    return(FAILURE);
    }
*/
 
  /* verify node is dedicated */
 
  for (jindex = 0;jindex < MAX_MJOB_PER_NODE;jindex++)
    {
    if (N->JList[jindex] == NULL)
      break;

    if (N->JList[jindex] == (mjob_t *)1)
      continue;
 
    if (JPeer == NULL)
      {
      if (N->JList[jindex] != J)
        {
        /* node contains unrelated job */
 
        DBG(3,fLL) DPrint("INFO:     cannot requeue job '%s' (non-dedicated node)\n",
          J->Name);
 
        return(FAILURE);
        }
      }
    else
      {
      for (index = 0;JPeer[index] != NULL;index++)
        {
        if (N->JList[jindex] == JPeer[index])
          break;
        }
 
      if (JPeer[index] == NULL)
        {
        /* node contains unrelated job */
 
        DBG(3,fLL) DPrint("INFO:     cannot requeue job '%s' (non-dedicated node)\n",
          J->Name);
 
        return(FAILURE);
        }
      }
    }  /* END for (jindex) */
 
#if defined(__MLL22) || defined(__MLL31)
 
  DBG(5,fLL) DPrint("INFO:     attempting flush of job %s on node %s\n",
    J->Name,
    HostList[0]);
 
  if ((rc = ll_control(
              LL_CONTROL_VERSION,
              LL_CONTROL_FLUSH,
              HostList,
              NULL,  /* userlist */
              NULL,  /* joblist */
              NULL,  /* classlist */
              0)) != 0)
    {
    LL2ShowError(rc,J,NULL);
 
    return(FAILURE);
    }
 
  DBG(5,fLL) DPrint("INFO:     attempting resume of node %s\n",
    HostList[0]); 
 
  if ((rc = ll_control(
              LL_CONTROL_VERSION,
              LL_CONTROL_RESUME_STARTD,
              HostList,
              NULL,  /* userlist */
              NULL,  /* joblist */
              NULL,  /* classlist */
              0)) != 0)
    {
    LL2ShowError(rc,J,NULL);
 
    return(FAILURE);
    }
 
#else /* __MLL22 || __MLL31 */
 
  sprintf(Command,"%s/llctl -h %s flush startd",
    DEFAULT_LLBINPATH,
    HostList[0]);
 
  if (MUReadPipe(Command,Buffer,sizeof(Buffer)) == FAILURE)
    {
    /* cannot execute command */
 
    return(FAILURE);
    }
 
  DBG(4,fLL) DPrint("INFO:     command '%s' successfully issued.  response '%s' received\n",
    Command,
    Buffer);
 
  sprintf(Command,"%s/llctl -h %s resume startd",
    DEFAULT_LLBINPATH,
    HostList[0]);
 
  if (MUReadPipe(Command,Buffer,sizeof(Buffer)) == FAILURE)
    {
    /* cannot execute command */
 
    return(FAILURE);
    }
 
  DBG(4,fLL) DPrint("INFO:     command '%s' successfully issued.  response '%s' received\n",
    Command,
    Buffer);
 
#endif /* __MLL22 || __MLL31 */
 
  if (J->SysFlags & (1 << mjfBackfill))
    J->SysFlags ^= (1 << mjfBackfill);
 
  MJobUpdateFlags(J);
 
  J->EState = mjsIdle;
 
  return(SUCCESS);
  }  /* END MLLJobRequeue() */




int LL2ShowError(
 
  int     SC,  /* I */
  mjob_t *J,   /* I */
  char   *Buf) /* O (optional) */
 
  {
  char Msg[MAX_MLINE];

  const char *FName = "LL2ShowError";

  DBG(5,fLL) DPrint("%s(%d,J)\n",
    FName,
    SC);

  Msg[0] = '\0';
 
  switch(SC)
    {
    case -1:

      strcpy(Msg,"ERROR:    invalid query element specified\n");
 
      break;
 
    case -2:

      strcpy(Msg,"ERROR:    invalid query daemon specified\n");

      break;
 
    case -3:

      strcpy(Msg,"ERROR:    cannot resolve hostname\n");
 
      break;
 
    case -4:

      strcpy(Msg,"ERROR:    invalid request type specified\n");
 
      break;
 
    case -5:

      strcpy(Msg,"ERROR:    API system error occurred\n");
 
      break;
 
    case -6:
 
      strcpy(Msg,"ERROR:    no objects exist matching request\n");
 
      break;
 
    case -7:

      strcpy(Msg,"ERROR:    API internal error occurred\n");
 
      break;
 
    case -8:

      strcpy(Msg,"ERROR:    job version is incorrect\n");
 
      break;
 
    case -9:

      strcpy(Msg,"ERROR:    job step is not idle\n");
 
      break;
 
    case -10:

      strcpy(Msg,"ERROR:    allocated node is not available to run job\n");

      break;
 
    case -11:

      strcpy(Msg,"ERROR:    allocated node does not have required class initiator available\n");
 
      break; 
 
    case -12:

      strcpy(Msg,"ERROR:    nodes do not meet job requirements\n");  
 
      break;
 
    case -13:

      strcpy(Msg,"ERROR:    incorrect nodecount specified for job\n");
 
      break;
 
    default:
 
      sprintf(Msg,"ERROR:    unexpected error code (%d)\n",
        SC);

      break;
    }  /* END switch(SC) */

  if (Msg[0] != '\0')
    {
    DBG(0,fLL) DPrint(Msg);
 
    if (MSched.Mode == msmNormal)
      MOSSyslog(LOG_ERR,Msg);
    }

  if (Buf != NULL)
    strcpy(Buf,Msg);
 
  return(SUCCESS);
  }  /* END LL2ShowError() */




int LLNodeLoad(

  mnode_t    *N,       /* I (modified) */
  LL_element *Machine, /* I */
  int         NStatus, /* I */
  mrm_t      *R)       /* I */

  {
  const char *FName = "LLNodeLoad";

  DBG(5,fLL) DPrint("%s(%s,Machine,%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    MAList[eNodeState][NStatus],
    (R != NULL) ? R->Name : "NULL");

  if ((N == NULL) || (Machine == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  MTRAPNODE(N,FName);

  /* load standard consumable resources */

  MLLNodeSetAttrs(N,Machine);

  if (N->State == mnsNone)
    {
    DBG(3,fLL) DPrint("WARNING:  node '%s' is unusable in state 'NONE'\n",
      N->Name);
    }

  if ((N->ARes.Disk < 0) && (N->State == mnsIdle))
    {
    DBG(2,fLL) DPrint("WARNING:  idle node %s is unusable  (inadequate disk space in /var)\n",
      N->Name);
    }

  DBG(6,fLL) DPrint("INFO:     MNode[%03d] '%18s' %9s S: %8d M: %5d D: %5d Cl: %s %s\n",
    N->Index,
    N->Name,
    MAList[eNodeState][N->State],
    N->ARes.Swap,
    N->ARes.Mem,
    N->ARes.Disk,
    MUCAListToString(N->ARes.PSlot,N->CRes.PSlot,NULL),
    MUMAList(eFeature,N->FBM,sizeof(N->FBM)));

  return(SUCCESS);
  }  /* END LLNodeLoad() */




int MLLNodeSetAttrs(

  mnode_t    *N,    /* I (modified) */
  LL_element *LLNO) /* I */

  {
  int      tmpI;
  char    *tmpP;
  char   **tmpPA;

  if ((N == NULL) || (LLNO == NULL))
    {
    return(FAILURE);
    }

  /* load base consumables */

  ll_get_data(LLNO,LL_MachineCPUs,&N->CRes.Procs);
  ll_get_data(LLNO,LL_MachineMaxTasks,&N->AP.HLimit[mptMaxProc][0]);

  if (N->AP.HLimit[mptMaxProc][0] >= 1)
    {
    /* NOTE:  constrain configured procs by MaxTask */

    N->CRes.Procs = MAX(N->AP.HLimit[mptMaxProc][0],N->CRes.Procs);
    }

  /* all swap specified in KB */

  ll_get_data(LLNO,LL_MachineVirtualMemory,&tmpI);

  N->ARes.Swap = tmpI / 1024;

  ll_get_data(LLNO,LL_MachineRealMemory,&tmpI);

  /* NOTE:  in LL31, memory specified in MB, all other versions use KB */

#if !defined(__MLL31)
  /* specified in KB */

  N->CRes.Mem = tmpI >> 10;
#else
  N->CRes.Mem = tmpI;
#endif /* !__MLL31 */

#if defined(__MLL22) || defined(__MLL31)
  ll_get_data(LLNO,LL_MachineFreeRealMemory,&tmpI);

#if !defined(__MLL31)
  /* specified in KB */

  N->ARes.Mem = tmpI >> 10;
#else
  N->ARes.Mem = tmpI;
#endif /* !__MLL31 */

#else /* __MLL22 || __MLL31 */
  N->ARes.Mem = N->CRes.Mem;
#endif /* __MLL22 || __MLL31 */

  /* local local disk info */

  /* NOTE:  in LL31, disk specified in MB, all other versions use KB */

  ll_get_data(LLNO,LL_MachineDisk,&tmpI);

#if !defined(__MLL31)
  /* specified in KB */

  N->ARes.Disk = tmpI >> 10;
#else 
  N->ARes.Disk = tmpI;
#endif /* !__MLL31 */

  /* verify bounds */

  N->CRes.Swap = MAX(N->CRes.Swap,N->ARes.Swap);
  N->CRes.Disk = MAX(MMIN_LLCFGDISK,N->ARes.Disk);

  /* load batch pool info */

  {
  int index;

  int  tmpI;
  int *tmpIA = NULL;

  ll_get_data(LLNO,LL_MachinePoolList,&tmpIA);
  ll_get_data(LLNO,LL_MachinePoolListSize,&tmpI);

  for (index = 0;index < MAX_MPOOL;index++)
    {
    if (index >= tmpI)
      break;

    N->Pool[index] = (char)(tmpIA[index] + 1);
    }  /* END for (index) */

  N->Pool[index] = '\0';

  MUFree((char **)&tmpIA);
  }  /* END BLOCK */

  if (ll_get_data(LLNO,LL_MachineOperatingSystem,&tmpP) == 0)
    {
    N->ActiveOS = MUMAGetIndex(eOpsys,tmpP,mAdd);

    free(tmpP);
    }

  if (ll_get_data(LLNO,LL_MachineArchitecture,&tmpP) == 0)
    {
    N->Arch = MUMAGetIndex(eArch,tmpP,mAdd);

    free(tmpP);
    }

  ll_get_data(LLNO,LL_MachineLoadAverage,&N->Load);
  ll_get_data(LLNO,LL_MachineSpeed,&N->Speed);

  /* load configured network adapter info */

  if (ll_get_data(LLNO,LL_MachineAdapterList,&tmpPA) == 0)
    {
    int nindex;

    N->Network = 0;

    for (nindex = 0;tmpPA[nindex] != NULL;nindex++)
      {
      N->Network |= MUMAGetBM(eNetwork,tmpPA[nindex],mAdd);

      free(tmpPA[nindex]);
      }

    free(tmpPA);

    /* NOTE:  load consumable swith adapter info */

    /* NYI */
    }

  /* load node feature info */

  if (ll_get_data(LLNO,LL_MachineFeatureList,&tmpPA) == 0)
    {
    int findex;

    memset(N->FBM,0,sizeof(N->FBM));

    for (findex = 0;tmpPA[findex] != NULL;findex++)
      {

      MNodeProcessFeature(N,tmpPA[findex]);

      free(tmpPA[findex]);
      }

    free(tmpPA);
    }

  /* load class info */

  {
  char **CClass = NULL;
  char **AClass = NULL;

  int rc;

  if ((rc = ll_get_data(LLNO,LL_MachineConfiguredClassList,&CClass)) < 0)
    {
    DBG(1,fLL) DPrint("ALERT:    cannot load in config class info for node %s (rc: %d)\n",
      N->Name,
      rc);

    return(FAILURE);
    }

  if ((rc = ll_get_data(LLNO,LL_MachineAvailableClassList,&AClass)) < 0)
    {
    DBG(1,fLL) DPrint("ALERT:    cannot load avail class info for node %s (rc: %d)\n",
      N->Name,
      rc);

    return(FAILURE);
    }

  LLGetClassInfo(N,CClass,AClass);

  if (CClass != NULL)
    {
    int cindex;

    for (cindex = 0;CClass[cindex] != NULL;cindex++)
      {
      free(CClass[cindex]);
      }  /* END for (cindex) */

    free(CClass);
    }

  if (AClass != NULL)
    {
    int aindex;

    for (aindex = 0;AClass[aindex] != NULL;aindex++)
      free(AClass[aindex]);

    free(AClass);
    }
  }    /* END BLOCK */

#if defined(__MLL22) || defined(__MLL31)
  {
  LL_element *R;
  char       *RName;
  char       *RReqName;
  int         RCfgValue;
  int         RAvlValue;
  int         RReqValue;

  int         rc;

  ll_get_data(LLNO,LL_MachineGetFirstResource,&R);

  RName = NULL;

  while (R != NULL)
    {
    if ((rc = ll_get_data(R,LL_ResourceName,&RName)) < 0)
      {
      DBG(2,fLL) DPrint("WARNING:  cannot get resource name on node %s\n",
        N->Name);

      ll_get_data(LLNO,LL_MachineGetNextResource,&R);

      continue;
      }

    if ((rc = ll_get_data(R,LL_ResourceInitialValue,&RCfgValue)) < 0)
      {
      DBG(2,fLL) DPrint("WARNING:  cannot get resource value for resource %s on node %s\n",
        N->Name,
        RName);

      MUFree(&RName);

      ll_get_data(LLNO,LL_MachineGetNextResource,&R);

      continue;
      }

    if ((rc = ll_get_data(R,LL_ResourceAvailableValue,&RAvlValue)) < 0)
      {
      DBG(2,fLL) DPrint("WARNING:  cannot get resource value for resource %s on node %s\n",
        N->Name,
        RName);

      MUFree(&RName);

      ll_get_data(LLNO,LL_MachineGetNextResource,&R);

      continue;
      }

    if ((rc = ll_get_data(R,LL_ResourceRequirementValue,&RReqValue)) < 0)
      {
      DBG(2,fLL) DPrint("WARNING:  cannot get resource req value on node %s\n",
        N->Name);

      MUFree(&RName);

      ll_get_data(LLNO,LL_MachineGetNextResource,&R);

      continue;
      }

    if ((rc = ll_get_data(R,LL_ResourceRequirementName,&RReqName)) < 0)
      {
      DBG(2,fLL) DPrint("WARNING:  cannot get resource req name on node %s\n",
        N->Name);

      MUFree(&RName);

      ll_get_data(LLNO,LL_MachineGetNextResource,&R);

      continue;
      }

    DBG(2,fLL) DPrint("INFO:     resource %s detected (%d of %d available) : ReqName %s (%d)\n",
      RName,
      RAvlValue,
      RCfgValue,
      RReqName,
      RReqValue);

    if (!strcmp(RName,"ConsumableCpus"))
      {
      N->CRes.Procs = RCfgValue;

      if ((N->State == mnsIdle) || (N->State == mnsActive))
        N->ARes.Procs = RAvlValue;
      }
    else if (!strcmp(RName,"ConsumableVirtualMemory"))
      {
      N->CRes.Swap = RCfgValue;

      if ((N->State == mnsIdle) || (N->State == mnsActive))
        N->ARes.Swap = RAvlValue;
      }
    else if (!strcmp(RName,"ConsumableMemory"))
      {
      N->CRes.Mem = RCfgValue;

      if ((N->State == mnsIdle) || (N->State == mnsActive))
        N->ARes.Mem = RAvlValue;
      }

    MUFree(&RName);

    R = NULL;

    ll_get_data(LLNO,LL_MachineGetNextResource,&R);
    }  /* END while (R != NULL) */
  }    /* END BLOCK */
#endif /* __MLL22 || __MLL31 */

  return(SUCCESS);
  }  /* END MLLNodeSetAttrs() */




int LLClusterQuery(

  mrm_t *R,      /* I */
  int   *RCount, /* O:  number of nodes located */
  int   *SC)     /* O (optional) */

  {
  int rc;

  mnode_t *N;

  char   *NName;
  int     NMemory;
  char   *NState;

  enum MNodeStateEnum NewState;
  enum MNodeStateEnum OldState;

  int     NodeCount;

  LL_element  *qo;
  LL_element  *machine = NULL;

  int tmpSC;

  const char *FName = "LLClusterQuery";

  DBG(1,fLL) DPrint("%s(%s,RCount,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");

  if (RCount != NULL)
    *RCount = 0;

  if (R == NULL)
    {
    DBG(1,fLL) DPrint("ALERT:     invalid RM parameter specified\n");

    return(FAILURE);
    }

  DBG(3,fLL) DPrint("INFO:     getting node information through API\n");

  if (R->U.LL.ConfigFile[0] != '\0')
    MUSetEnv("LOADL_CONFIG",R->U.LL.ConfigFile);

# ifdef __PNNL

  MUSetEnv("LLAPIERRORMSGS","yes");

# endif /* __PNNL */

  if ((qo = ll_query(MACHINES)) == NULL)
    {
    DBG(1,fLL) DPrint("ALERT:    ll_query returns NULL\n");

    return(FAILURE);
    }

  DBG(6,fLL) DPrint("INFO:     ll_query() completed\n");

  if ((rc = ll_set_request(qo,QUERY_ALL,NULL,ALL_DATA)) != 0)
    {
    DBG(6,fLL) DPrint("INFO:     ll_set_request() failed (rc: %d)\n",
      rc);

    return(FAILURE);
    }

  machine = ll_get_objs(qo,LL_CM,NULL,&NodeCount,&tmpSC);

  if (machine == NULL)
    {
    if (tmpSC == -6)
      {
      DBG(2,fLL) DPrint("INFO:     no resources detected\n");

      ll_deallocate(qo);

      return(SUCCESS);
      }

    DBG(1,fLL) DPrint("ALERT:    cannot get node information, SC: %d\n",
      tmpSC);

    LL2ShowError(tmpSC,NULL,NULL);

    ll_deallocate(qo);

    return(FAILURE);
    }

  DBG(2,fLL) DPrint("INFO:     nodes detected: %d\n",
    NodeCount);

  while (machine != NULL)
    {
    ll_get_data(machine,LL_MachineName,&NName);

    ll_get_data(machine,LL_MachineRealMemory,&NMemory);

    ll_get_data(machine,LL_MachineStartdState,&NState);

    NewState = MUMAGetIndex(eNodeState,NState,mAdd);

    free(NState);

    if (RCount != NULL)
      (*RCount)++;

    rc = MNodeFind(NName,&N);

    if ((rc == SUCCESS) && (N->CTime > 0))
      {
      /* if node previously found, update data */

      OldState = N->State;

      MRMNodePreUpdate(N,NewState,R);

      MLLNodeUpdate(N,machine,NewState,R);

      MRMNodePostUpdate(N,OldState);
      }
    else if (NMemory > 0)
      {
      /* new compute node detected */

      if (MNodeAdd(NName,&N) == SUCCESS)
        {
        /* allocate node structure, load data */

        MRMNodePreLoad(N,NewState,R);

        LLNodeLoad(N,machine,NewState,R);

        MRMNodePostLoad(N);
        }
      else
        {
        DBG(1,fLL) DPrint("ERROR:    node buffer is full  (ignoring node '%s')\n",
          NName);
        }
      }
    else
      {
      /* non-startd node detected.  ignore */

      DBG(4,fLL) DPrint("INFO:     non-startd node '%s' detected  (ignoring node)\n",
        NName);
      }  /* END else (rc == SUCCESS ...) */

    free(NName);

    machine = NULL;

    machine = ll_next_obj(qo);
    }    /* END while (machine != NULL) */

  ll_free_objs(qo);

  ll_deallocate(qo);

  return(SUCCESS);
  }  /* END LLClusterQuery() */




int MLLJobLoad(

  mjob_t     *J,       /* I (modified) */
  LL_element *LLJob,   /* I */
  LL_element *LLStep,  /* I */
  char       *JobName, /* ? */
  mrm_t      *R)       /* I */

  {
  LL_element *Cred;
  LL_element *Node;
  LL_element *Task;

  int      nindex;
  int      hindex;

  int      SIndex;

  int      rc;

  mnode_t *N;

  mreq_t  *RQ;

  char    *ptr;

  char  *UName = NULL;
  char  *GName = NULL;
  char  *AName = NULL;

  char  *Comment     = NULL;
  char  *IWD         = NULL;
  char  *Cmd         = NULL;

  char  *ReqPtr      = NULL;
  char  *Class       = NULL;
  char  *SubmitHost  = NULL;

  char **ReqHList;

  int    HoldType;
  mclass_t *C;
  long   tmpL;

  int    LL2Status;

  int    NodeUsage;

  char   Requirements[MAX_MLINE];

  short  TaskList[MAX_MTASK_PER_JOB + 1];

  int    MinInstance;
  int    TaskCount;

  char   Line[MAX_MNAME];

#if defined(__MLL22) || defined(__MLL31)
  LL_element *ResourceReq;
  char       *ReqName;
  int         ReqValue;
  char       *TaskGeometry;
#endif /* __MLL22 || __MLL31 */

  const char *FName = "MLLJobLoad";

  DBG(5,fLL) DPrint("%s(J,LLJob,LLStep,%s,%s)\n",
    FName,
    JobName,
    (R != NULL) ? R->Name : "NULL");

  if ((J == NULL) || (LLJob == NULL) || (LLStep == NULL))
    {
    return(FAILURE);
    }

  /* note:  must handle arbitrary geometry:  load taskmap */

  MTRAPJOB(J,FName);

  /* set LL defaults (Non-RM default only) */

  ll_get_data(LLStep,LL_StepState,&LL2Status);

  J->State = __MLLJobStateToJState(J->State,LL2Status);

  ll_get_data(LLJob,LL_JobCredential,&Cred);

  ll_get_data(Cred,LL_CredentialUserName,&UName);
  ll_get_data(Cred,LL_CredentialGroupName,&GName);
  ll_get_data(LLStep,LL_StepAccountNumber,&AName);

  rc = MJobSetCreds(J,UName,GName,AName);

  MUFree(&UName);
  MUFree(&GName);
  MUFree(&AName);

  if (rc == FAILURE)
    {
    DBG(1,fLL) DPrint("ERROR:    cannot determine creds for job '%s'\n",
      J->Name);

    MJobRemove(J);

    return(FAILURE);
    }

  /* initialize requirement */

  if (MReqCreate(J,NULL,&RQ,FALSE) == FAILURE)
    {
    DBG(1,fLL) DPrint("ALERT:    cannot add requirements to job '%s'\n",
      J->Name);

    return(FAILURE);
    }

  MRMReqPreLoad(RQ);

  ll_get_data(LLStep,LL_StepGetFirstNode,&Node);

  J->TasksRequested = 0;

  J->NodesRequested = 0;

  while (Node != NULL)
    {
    ll_get_data(Node,LL_NodeMinInstances,&MinInstance);

    ll_get_data(Node,LL_NodeInitiatorCount,&TaskCount);

    J->TasksRequested += (TaskCount * MinInstance);

    if (TaskCount > 0)
      J->NodesRequested += MinInstance;

    ll_get_data(LLStep,LL_StepGetNextNode,&Node);
    }  /* END while (Node != NULL) */

#if defined(__MLL22) || defined(__MLL31)
  {
  int TPN = -1;
  int BlockingFactor = -1;

  ll_get_data(LLStep,LL_StepTasksPerNodeRequested,&TPN);

  DBG(1,fLL) DPrint("INFO:     TPN for job %s: %d\n",
    J->Name,
    TPN);

  RQ->TasksPerNode = 1;

  if (TPN > 0)
    {
    RQ->TasksPerNode = TPN;
    }

  ll_get_data(LLStep,LL_StepBlocking,&BlockingFactor);

  DBG(1,fLL) DPrint("INFO:     blocking factor for job %s: %d\n",
    J->Name,
    BlockingFactor);

  if (BlockingFactor > 0)
    {
    RQ->BlockingFactor = BlockingFactor;
    }
  else if (BlockingFactor == -1)
    {
    RQ->BlockingFactor = 1;
    J->NodesRequested = 0;
    }

  /* get arbitrary geometry */

  ll_get_data(LLStep,LL_StepTaskGeometry,&TaskGeometry);

  if (TaskGeometry != NULL)
    {
    DBG(1,fLL) DPrint("INFO:     task geometry for job %s: '%s'\n",
      J->Name,
      TaskGeometry);

    MUStrDup(&J->Geometry,TaskGeometry);

    MUFree(&TaskGeometry);
    }
  }    /* END BLOCK */
#else /* __MLL22 || __MLL31 */

  /* temporary LL2 hack.  TasksPerNode must always be set */

  if ((J->TasksRequested % J->NodesRequested) == 0)
    {
    RQ->TasksPerNode = J->TasksRequested / J->NodesRequested;
    }
  else
    {
    RQ->TasksPerNode = J->TasksRequested / J->NodesRequested + 1;
    }

#endif /* __MLL22 || __MLL31 */

  /* NOTE:  following assumes one req per job */

  RQ->TaskRequestList[0] = J->TasksRequested;
  RQ->TaskRequestList[1] = J->TasksRequested;
  RQ->TaskRequestList[2] = 0;

  RQ->TaskCount = J->TasksRequested;
  RQ->NodeCount = J->NodesRequested;

  DBG(3,fLL) DPrint("INFO:     TotalTasks: %d  Node: %d  TaskPerNode: %d\n",
    RQ->TaskCount,
    RQ->NodeCount,
    RQ->TasksPerNode);

  ll_get_data(LLStep,LL_StepGetFirstNode,&Node);

  ll_get_data(Node,LL_NodeRequirements,&ReqPtr);

  if (ReqPtr != NULL)
    {
    MUStrCpy(Requirements,ReqPtr,sizeof(Requirements));

    MUFree(&ReqPtr);
    }

  ll_get_data(LLStep,LL_StepComment,&Comment);

  MJobSetAttr(J,mjaRMXString,(void **)Comment,mdfString,0);

  MJobProcessExtensionString(J,J->RMXString);

  ll_get_data(LLJob,LL_JobStepType,&J->E.JobType);

  ll_get_data(LLJob,LL_JobSubmitTime,&J->SubmitTime);

  ll_get_data(LLStep,LL_StepStartCount,&J->StartCount);

  ll_get_data(LLStep,LL_StepStartDate,&tmpL);

  MJobSetAttr(J,mjaReqSMinTime,(void **)&tmpL,mdfLong,mSet);

  ll_get_data(LLStep,LL_StepDispatchTime,&J->DispatchTime);

  /* NOTE: job start time not specified in some API versions */

  J->StartTime = J->DispatchTime;

  if ((J->State == mjsCompleted) || (J->State == mjsRemoved))
    {
    ll_get_data(LLStep,LL_StepCompletionDate,&J->CompletionTime);
    ll_get_data(LLStep,LL_StepCompletionCode,&J->CompletionCode);

    if (J->CompletionTime <= J->StartTime)
      J->CompletionTime = MSched.Time;
    }

  ll_get_data(LLStep,LL_StepExecSize,&J->ExecSize);
  ll_get_data(LLStep,LL_StepImageSize,&J->ImageSize);

  ll_get_data(LLStep,LL_StepIwd,&IWD);

  MUStrDup(&J->E.IWD,IWD);

  ll_get_data(LLStep,LL_StepGetFirstNode,&Node);

  Task = NULL;

  if (Node != NULL)
    {
    ll_get_data(Node,LL_NodeGetFirstTask,&Task);
    }

  while (Task != NULL)
    {
    ll_get_data(Task,LL_TaskExecutable,&Cmd);

    MUStrDup(&J->E.Cmd,Cmd);

#if defined(__MLL22) || defined(__MLL31)
    ResourceReq = NULL;

    ll_get_data(Task,LL_TaskGetFirstResourceRequirement,&ResourceReq);

    while (ResourceReq != NULL)
      {
      if ((rc = ll_get_data(ResourceReq,LL_ResourceRequirementName,&ReqName)) < 0)
        {
        DBG(1,fLL) DPrint("WARNING:  cannot get resource req name on job %s\n",
          J->Name);

        ResourceReq = NULL;

        ll_get_data(Task,LL_TaskGetNextResourceRequirement,&ResourceReq);
        }

      if ((rc = ll_get_data(ResourceReq,LL_ResourceRequirementValue,&ReqValue)) < 0)
        {
        DBG(1,fLL) DPrint("WARNING:  cannot get value for resource req %s on job %s\n",
          ReqName,
          J->Name);

        MUFree(&ReqName);

        ResourceReq = NULL;

        ll_get_data(Task,LL_TaskGetNextResourceRequirement,&ResourceReq);
        }

      DBG(1,fLL) DPrint("INFO:     resource req '%s' == %d loaded for job %s\n",
        ReqName,
        ReqValue,
        J->Name);

      if (!strcmp(ReqName,"ConsumableCpus"))
        {
        RQ->DRes.Procs = ReqValue;
        }
      else if (!strcmp(ReqName,"ConsumableVirtualMemory"))
        {
        RQ->DRes.Swap = ReqValue;
        }
      else if (!strcmp(ReqName,"ConsumableMemory"))
        {
        RQ->DRes.Mem = ReqValue;
        }

      MUFree(&ReqName);

      ResourceReq = NULL;

      ll_get_data(Task,LL_TaskGetNextResourceRequirement,&ResourceReq);
      }  /* END while (ResourceReq != NULL) */
#endif /* __MLL22 || __MLL31 */

    ll_get_data(Node,LL_NodeGetNextTask,&Task);
    }  /* END while (Task != NULL) */

  ll_get_data(LLStep,LL_StepWallClockLimitHard,&J->SpecWCLimit[1]);

  J->SpecWCLimit[0] = J->SpecWCLimit[1];
  J->WCLimit        = J->SpecWCLimit[1];

  ll_get_data(LLStep,LL_StepJobClass,&Class);

  if (Class != NULL)
    {
    /* only one class per task */

    MClassAdd(Class,&C);

    RQ->DRes.PSlot[C->Index].count = 1;
    RQ->DRes.PSlot[0].count        = 1;

    MUFree(&Class);
    }

  RQ->NAccessPolicy = MSched.DefaultNAccessPolicy;

  RQ->RMIndex = (R != NULL) ? R->Index : 0;

  MReqRResFromString(
    J,
    RQ,
    Requirements,
    (J->RM != NULL) ? J->RM->Type : MRM[0].Type,
    FALSE);

#if defined(__MLL22) || defined(__MLL31)
  {
  int tmpI;

  ll_get_data(LLStep,LL_StepRestart,&tmpI);

  if (tmpI > 0)
    J->SpecFlags |= (1 << mjfRestartable);
  }  /* END BLOCK */
#else /* __MLL22 || __MLL31 */
  J->SpecFlags |= (1 << mjfRestartable);
#endif /* __MLL22 || __MLL31 */

  ll_get_data(LLStep,LL_StepNodeUsage,&NodeUsage);

  if (NodeUsage == NOT_SHARED)
    {
    J->SpecFlags |= (1 << mjfNASingleJob);
    }

  ll_get_data(LLJob,LL_JobSubmitHost,&SubmitHost);

  MUStrCpy(J->SubmitHost,SubmitHost,sizeof(J->SubmitHost));

  MUFree(&SubmitHost);

  /* NOTE:  must obtain step info from job name */

  strcpy(Line,J->Name);

  if ((ptr = strrchr(Line,'.')) != NULL)
    {
    /* get job step number */

    J->Proc = atoi(ptr + 1);

    *ptr = '\0';

    if ((ptr = strrchr(Line,'.')) != NULL)
      {
      /* get job id number */

      J->Cluster = atoi(ptr + 1);

      *ptr = '\0';

      strcpy(J->SubmitHost,Line);
      }
    }

  if (J->State == mjsHold)
    {
    /* update hold state */

    ll_get_data(LLStep,LL_StepHoldType,&HoldType);

    switch(HoldType)
      {
      case HOLDTYPE_USER:

        J->Hold |= (1 << mhUser);

        break;

      case HOLDTYPE_SYSTEM:

        J->Hold |= (1 << mhSystem);

        break;

      case HOLDTYPE_USERSYS:

        J->Hold |= (1 << mhSystem);
        J->Hold |= (1 << mhUser);

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch(HoldType) */

    DBG(5,fLL) DPrint("INFO:     hold detected on job '%s' (%x)\n",
      J->Name,
      J->Hold);
    }  /* END if (J->State == mjsHold) */

  if ((J->State != mjsRunning) && (J->State != mjsStarting))
    {
    ReqHList = NULL;

    rc = ll_get_data(LLStep,LL_StepHostList,&ReqHList);

    if ((rc == 0) && (ReqHList != NULL) && (ReqHList[0] != NULL))
      {
      nindex = 0;

      J->SpecFlags |= (1 << mjfHostList);

      J->ReqHList = (mnalloc_t *)calloc(1,sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1));

      J->ReqHList[0].N = NULL;

      TaskCount = 0;

      for (hindex = 0;ReqHList[hindex] != NULL;hindex++)
        {
        if (MNodeFind(ReqHList[hindex],&N) == FAILURE)
          {
          DBG(1,fLL) DPrint("ALERT:    cannot locate node '%s' for job '%s' hostlist\n",
            ReqHList[hindex],
            J->Name);

          for (;ReqHList[hindex] != NULL;hindex++)
            MUFree(&ReqHList[hindex]);

          MUFree((char **)&ReqHList);

          return(FAILURE);
          }

        if (J->ReqHList[nindex].N == N)
          {
          J->ReqHList[nindex].TC++;

          DBG(5,fLL) DPrint("INFO:     hostlist node '%s' taskcount increased to %d\n",
            N->Name,
            J->ReqHList[nindex].TC);
          }
        else
          {
          if (J->ReqHList[nindex].N != NULL)
            nindex++;

          J->ReqHList[nindex].N = N;

          J->ReqHList[nindex].TC = 1;

          DBG(5,fLL) DPrint("INFO:     hostlist node '%s' added to job %s\n",
            N->Name,
            J->Name);
          }

        MUFree(&ReqHList[hindex]);

        ReqHList[hindex] = NULL;

        TaskCount++;
        }   /* END for (hindex) */

      J->ReqHList[nindex + 1].N = NULL;

      MUFree((char **)&ReqHList);

      if (TaskCount > J->TasksRequested)
        {
        DBG(1,fLL) DPrint("ALERT:    job %s taskcount less than hostlist tasks (%d < %d) (Adjusting)\n",
          J->Name,
          J->TasksRequested,
          TaskCount);

        J->TasksRequested    = TaskCount;
        J->Req[0]->TaskCount = TaskCount;

        RQ->TaskRequestList[0] = J->TasksRequested;
        RQ->TaskRequestList[1] = J->TasksRequested;
        RQ->TaskRequestList[2] = 0;
        }
      }  /* END if ((ReqHList != NULL) && (ReqHList[0] != NULL)) */
    }    /* END if ((J->State != mjsRunning) && (J->State != mjsStarting)) */

  if (MJOBISALLOC(J) == TRUE)
    {
    MLLGetTL(LLStep,TaskList,&J->TaskCount);
    }
  else
    {
    TaskList[0] = -1;
    }

  /* get job dependencies */

  if ((ptr = strrchr(J->Name,'.')) != NULL)
    {
    char DValue[MAX_MNAME];

    SIndex = strtol(ptr + 1,NULL,0);

    if (SIndex > 0)
      {
      strncpy(DValue,J->Name,ptr - J->Name);

      sprintf(DValue,"%s%d",
        DValue,
        SIndex - 1);

      MJobSetDependency(J,mjdJobCompletion,DValue);
      }
    }

  MRMJobPostLoad(J,TaskList,J->RM);

  return(SUCCESS);
  }  /* END MLLJobLoad() */




int MLLJobCancel(

  mjob_t *J,       /* I */
  mrm_t  *R,       /* I */
  char   *Message, /* I (optional) */
  char   *Msg,     /* O (optional) */
  int    *SC)      /* O (optional) */

  {
  int                   rc;
  LL_terminate_job_info CancelInfo;

  const char *FName = "MLLJobCancel";

  DBG(2,fLL) DPrint("%s(%s,%s,%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL",
    (Message != NULL) ? Message : "NULL");

  if ((J == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  memset(&CancelInfo,0,sizeof(LL_terminate_job_info));

  CancelInfo.version_num      = R->U.LL.ProcVersionNumber;
  CancelInfo.StepId.cluster   = J->Cluster;
  CancelInfo.StepId.proc      = J->Proc;
  CancelInfo.StepId.from_host = J->SubmitHost;
  CancelInfo.msg              = Message;

  DBG(3,fLL) DPrint("INFO:     LL terminate structure initialized\n");

  if (R->U.LL.ConfigFile[0] != '\0')
    MUSetEnv("LOADL_CONFIG",R->U.LL.ConfigFile);

  if ((rc = ll_terminate_job(&CancelInfo)) != API_OK)
    {
    DBG(0,fLL) DPrint("ERROR:    cannot cancel job via LL\n");

    LL2ShowError(rc,J,NULL);

    return(FAILURE);
    }
  else
    {
    DBG(3,fLL) DPrint("INFO:     job '%s' cancelled\n",
      J->Name);

    if (MSched.Mode == msmNormal)
      {
      MOSSyslog(LOG_INFO,"job %s cancelled",
        J->Name);
      }
    }

  return(SUCCESS);
  }  /* END MLLJobCancel() */




int MLLJobStart(

  mjob_t *J,   /* I */
  mrm_t  *R,   /* I */
  char   *Msg, /* O (optional) */
  int    *SC)  /* O (optional) */

  {
  LL_start_job_info  StartInfo;
  char              *TaskList[MAX_MTASK + 1];
  int                rc;

  char               tmpLine[MAX_MLINE];

  const char *FName = "MLLJobStart";

  DBG(4,fLL) DPrint("%s(%s,%s,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if ((J == NULL) || (J->Name[0] == '\0'))
    {
    if (Msg != NULL)
      strcpy(Msg,"cannot start job - invalid job received");

    DBG(0,fLL) DPrint("WARNING:  invalid job passed to %s()\n",
      FName);

    if (SC != NULL)
      *SC = mscBadParam;

    return(FAILURE);
    }

  if (R == NULL)
    {
    return(FAILURE);
    }

  memset(&StartInfo,0,sizeof(LL_start_job_info));

  /* set up tasklist */

  MUTMToHostList(J->TaskMap,TaskList,R);

  StartInfo.nodeList         = TaskList;
  StartInfo.version_num      = R->U.LL.ProcVersionNumber;
  StartInfo.StepId.cluster   = J->Cluster;
  StartInfo.StepId.proc      = J->Proc;
  StartInfo.StepId.from_host = J->SubmitHost;

  DBG(3,fLL) DPrint("INFO:     LL start structure initialized\n");

  MUSetEnv("LOADL_CONFIG",R->U.LL.ConfigFile);

  if ((rc = ll_start_job(&StartInfo)) != API_OK)
    {
    DBG(0,fLL) DPrint("ERROR:    cannot start job '%s'\n",
      J->Name);

    DBG(3,fLL) DPrint("INFO:     version: %d  cluster: %d  proc: %d  fromhost: '%s'\n",
      R->U.LL.ProcVersionNumber,
      J->Cluster,
      J->Proc,
      J->SubmitHost);

    LL2ShowError(rc,J,tmpLine);

    if (Msg != NULL)
      {
      sprintf(Msg,"job start failed with rc %d (%s)",
        rc,
        tmpLine);
      }

    if (rc == API_LL_SCH_ON)
      {
      if (R->FailIteration != MSched.Iteration)
        {
        R->FailIteration = MSched.Iteration;
        R->FailCount     = 0;
        }

      R->FailCount++;
      }

    if (SC != NULL)
      *SC = mscRemoteFailure;

    return(FAILURE);
    }  /* END if ((rc = ll_start_job(&StartInfo)) != API_OK) */

  DBG(3,fLL) DPrint("INFO:     LL job '%s' started\n",
    J->Name);

  if (MSched.Mode == msmNormal)
    {
    MOSSyslog(LOG_INFO,"job %s started",
      J->Name);
    }

  return(SUCCESS);
  }  /* END MLLJobStart() */




int MLLJobProcess(

  LL_element *LLJob,   /* I */
  LL_element *LLStep,  /* I */
  LL_element *LLUsage, /* I (optional) */
  char       *RMJID,   /* I */
  mrm_t      *R,       /* I */
  int         Status)  /* I */

  {
  mjob_t *J;
  char    Message[MAX_MLINE];

  char    tmpJName[MAX_MNAME];

  const char *FName = "MLLJobProcess";

  DBG(4,fLL) DPrint("%s(LLJob,LLStep,LLUsage,%s,%s,%d)\n",
    FName,
    RMJID,
    (R != NULL) ? R->Name : "NULL",
    Status);

  if ((LLJob == NULL) || (LLStep == NULL) || (RMJID == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  MJobGetName(NULL,RMJID,R,tmpJName,sizeof(tmpJName),mjnShortName);

  switch(Status)
    {
    case STATE_IDLE:
    case STATE_PENDING:
    case STATE_STARTING:
    case STATE_RUNNING:
    case STATE_HOLD:
    case STATE_DEFERRED:
    case STATE_UNEXPANDED:
    case STATE_NOTQUEUED:

#if ! defined(__MLL2)
    case STATE_PREEMPTED:
    case STATE_PREEMPT_PENDING:
    case STATE_RESUME_PENDING:
#endif /* END !__MLL2 */

    case STATE_COMPLETE_PENDING:
    case STATE_REJECT_PENDING:
    case STATE_REMOVE_PENDING:
    case STATE_VACATE_PENDING:

      /* pending states:  incomplete information possible */

      /* NOTE:  maintain old state until final state is reached */

      if (MJobFind(tmpJName,&J,0) == SUCCESS)
        {
        switch (Status)
          {
          case STATE_COMPLETE_PENDING:
          case STATE_REJECT_PENDING:
          case STATE_REMOVE_PENDING:
          case STATE_VACATE_PENDING:

            switch (J->State)
              {
              case mjsIdle:

                Status = STATE_IDLE;

                break;

              case mjsRunning:

                Status = STATE_RUNNING;

                break;

              case mjsStarting:

                Status = STATE_STARTING;

                break;

              default:

                Status = STATE_RUNNING;

                break;
              }  /* END switch (J->State) */

            break;

          default:

            /* NO-OP */

            break;
          }      /* END switch (Status) */

        /* if job previously found, update data */

        MRMJobPreUpdate(J);

        MLLJobUpdate(J,LLJob,LLStep,LLUsage,R);
        }  /* END if (MJobFind()) */
      else if (MJobCreate(tmpJName,TRUE,&J) == SUCCESS)
        {
        /* if new job, load data */

        MJobSetAttr(J,mjaRMJID,(void **)RMJID,mdfString,mSet);

        MRMJobPreLoad(J,tmpJName,R->Index);

        MLLJobLoad(J,LLJob,LLStep,tmpJName,R);
        }
      else
        {
        DBG(1,fLL) DPrint("ERROR:    job buffer is full  (ignoring job '%s')\n",
          tmpJName);

        return(FAILURE);
        }

      break;

    case STATE_COMPLETED:
    case STATE_REJECTED:
    case STATE_REMOVED:
    case STATE_VACATED:
    case STATE_CANCELED:
    case STATE_NOTRUN:
    case STATE_TERMINATED:
    case STATE_SUBMISSION_ERR:

      if (MJobFind(tmpJName,&J,0) == SUCCESS)
        {
        /* if job never ran, remove record.  job cancelled externally */

        if ((J->State != mjsRunning) && (J->State != mjsStarting))
          {
          DBG(1,fLL) DPrint("INFO:     job '%s' was cancelled externally\n",
            J->Name);

          /* remove job from joblist */

          MJobRemove(J);

          break;
          }

        /* if job previously found, update job record */

        MRMJobPreUpdate(J);

        MLLJobUpdate(J,LLJob,LLStep,LLUsage,R);

        switch(Status)
          {
          case STATE_REJECT_PENDING:
          case STATE_REMOVE_PENDING:
          case STATE_VACATE_PENDING:
          case STATE_REJECTED:
          case STATE_REMOVED:
          case STATE_VACATED:
          case STATE_CANCELED:
          case STATE_NOTRUN:
          case STATE_TERMINATED:
          case STATE_SUBMISSION_ERR:

            if (MSched.Time < (J->StartTime + J->WCLimit))
              {
              MJobProcessRemoved(J);
              }
            else
              {
              sprintf(Message,"JOBWCVIOLATION:  job '%s' exceeded WC limit %s\n",
                J->Name,
                MULToTString(J->WCLimit));

              MSysRegEvent(Message,0,0,1);

              DBG(3,fLL) DPrint("INFO:     job '%s' exceeded wallclock limit %s\n",
                J->Name,
                MULToTString(J->WCLimit));

              MJobProcessCompleted(J);
              }

            break;

          case STATE_COMPLETED:
          case STATE_COMPLETE_PENDING:

            MJobProcessCompleted(J);

            break;

          default:

            /* unexpected job state */

            DBG(1,fLL) DPrint("WARNING:  unexpected job state (%d) detected for job '%s'\n",
              Status,
              J->Name);

            break;
          }   /* END switch (Status)                        */
        }     /* END if MJobFind(tmpJName,&J,0) == SUCCESS) */
      else
        {
        /* ignore job */

        DBG(4,fLL) DPrint("INFO:     ignoring job '%s'  (state: %s)\n",
          RMJID,
          MJobState[Status + 1]);
        }

      break;

    default:

      DBG(1,fLL) DPrint("WARNING:  job '%s' detected with unexpected state '%d'\n",
        RMJID,
        Status);

      break;
    }  /* END switch(Status) */

  return(SUCCESS);
  }  /* END MLLJobProcess() */




int __MLLJobStateToJState(

  int CurrentJState,  /* I */
  int LLJState)       /* I */

  {
  int JState;

  const char *FName = "__MLLJobStateToJState";

  DBG(3,fLL) DPrint("%s(%s,%s)\n",
    FName,
    MJobState[CurrentJState],
    MLLJobState[LLJState]);

  JState = mjsNONE;

  switch(LLJState)
    {
    case STATE_IDLE:

      JState = mjsIdle;

      /*NOTREACHED*/

      break;

    case STATE_PENDING:
    case STATE_STARTING:

      JState = mjsStarting;

      /*NOTREACHED*/

      break;

    case STATE_RUNNING:

      JState = mjsRunning;

      /*NOTREACHED*/

      break;

    case STATE_COMPLETED:

      JState = mjsCompleted;

      /*NOTREACHED*/

      break;

    case STATE_REMOVED:

      JState = mjsRemoved;

      /*NOTREACHED*/

      break;

    case STATE_REJECT_PENDING:
    case STATE_COMPLETE_PENDING:
    case STATE_VACATE_PENDING:
    case STATE_REMOVE_PENDING:

      if (CurrentJState != mjsNONE)
        JState = CurrentJState;
      else
        JState = mjsCompleted;

      break;

    case STATE_HOLD:

      JState = mjsHold;

      break;

    case STATE_DEFERRED:
    case STATE_REJECTED:

      JState = mjsDeferred;

      break;

    case STATE_UNEXPANDED:
    case STATE_NOTQUEUED:

      JState = mjsNotQueued;

      break;

    case STATE_VACATED:

      JState = mjsVacated;

      break;

    case STATE_NOTRUN:

      JState = mjsNotRun;

      break;

    case STATE_SUBMISSION_ERR:

      JState = mjsSubmitErr;

      break;

    case STATE_TERMINATED:
    case STATE_CANCELED:

      JState = mjsRemoved;

      break;

#if !defined(__MLL2)

    case STATE_PREEMPTED:
    case STATE_PREEMPT_PENDING:
    case STATE_RESUME_PENDING:

      JState = mjsSuspended;

      break;

#endif /* !__MLL2 */

    default:

      JState = mjsNONE;

      break;
    }  /* END switch(Status) */

  return(JState);
  }  /* END __MLLJobStateToJState() */




int MLLWorkloadQuery(

  mrm_t *R,      /* I */
  int   *JCount, /* I */
  int   *SC)     /* I */

  {
  int         rc;

  char       *stepid;

  int         status;

  int         JobCount;

  LL_element *qj;

  LL_element *LLJob;
  LL_element *LLStep;
  LL_element *LLUsage;

  int tmpSC;

  const char *FName = "MLLWorkloadQuery";

  DBG(2,fLL) DPrint("%s(%s,JCount,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");

  if (R == NULL)
    {
    return(FAILURE);
    }

  DBG(3,fLL) DPrint("INFO:     getting job information through LL API version %d\n",
    LL_JOB_VERSION);

  if (JCount != NULL)
    *JCount = 0;

  MUSetEnv("LOADL_CONFIG",R->U.LL.ConfigFile);

  qj = ll_query(JOBS);

  rc = ll_set_request(qj,QUERY_ALL,NULL,ALL_DATA);

  LLJob = ll_get_objs(qj,LL_CM,NULL,&JobCount,&tmpSC);

  if (LLJob == NULL)
    {
    if (tmpSC == -6)
      {
      DBG(2,fLL) DPrint("INFO:     no jobs detected\n");

      ll_deallocate(qj);

      return(SUCCESS);
      }

    DBG(1,fLL) DPrint("ALERT:    cannot get job information, SC: %d\n",
      tmpSC);

    LL2ShowError(tmpSC,NULL,NULL);

    ll_deallocate(qj);

    if (R->FailIteration != MSched.Iteration)
      {
      R->FailIteration = MSched.Iteration;
      R->FailCount     = 0;
      }

    R->FailCount++;

    return(FAILURE);
    }  /* END if (LLJob == NULL) */

  R->U.LL.ProcVersionNumber = LL_PROC_VERSION;
  R->U.LL.VersionNumber     = LL_JOB_VERSION;

  DBG(2,fLL) DPrint("INFO:     jobs detected: %d\n",
    JobCount);

  while (LLJob != NULL)
    {
    ll_get_data(LLJob,LL_JobGetFirstStep,&LLStep);

    while (LLStep != NULL)
      {
      stepid = NULL;

      ll_get_data(LLStep,LL_StepID,&stepid);

      if (stepid != NULL)
        {
        ll_get_data(LLStep,LL_StepState,&status);

        LLUsage = NULL;

        MLLJobProcess(LLJob,LLStep,LLUsage,stepid,R,status);

        free(stepid);
        }  /* END if (stepid != NULL) */

      ll_get_data(LLJob,LL_JobGetNextStep,&LLStep);
      }  /* END while (LLStep != NULL) */

    LLJob = ll_next_obj(qj);
    }  /* END while (LLJob != NULL) */

  ll_free_objs(qj);

  ll_deallocate(qj);

  if (JCount != NULL)
    *JCount = JobCount;

  return(SUCCESS);
  }  /* END MLLWorkloadQuery() */




int MLLJobSetAttr(

  mjob_t     *J,       /* I (modified) */
  LL_element *LLJob,   /* I */
  LL_element *LLStep,  /* I */
  LL_element *LLUsage, /* I */
  int         AIndex)  /* I */

  {
  switch (AIndex)
    {
    case mjaCompletionTime:

      {
      long CompletionTime = 0;
      int  tmpI = 0;

      ll_get_data(LLStep,LL_StepCompletionDate,&CompletionTime);

      /* NOTE:  piggyback completion code */

      ll_get_data(LLStep,LL_StepCompletionCode,&tmpI);

      J->CompletionCode = tmpI;

      if (CompletionTime > J->DispatchTime)
        {
        J->CompletionTime = CompletionTime;
        }
      else
        {
        DBG(1,fLL) DPrint("ALERT:    invalid completion time %s (%ld <= %ld) for job %s in state %s\n",
          MULToTString(CompletionTime - MSched.Time),
          CompletionTime,
          J->DispatchTime,
          J->Name,
          MJobState[J->State]);

        J->CompletionTime = MSched.Time;
        }
      }    /* END BLOCK */

      break;

    case mjaHold:

      {
      int tmpI;

      /* reset RM holds */

      if (J->Hold & (1 << mhUser))
        J->Hold ^= (1 << mhUser);

      if (J->Hold & (1 << mhSystem))
        J->Hold ^= (1 << mhSystem);

      if (J->State == mjsHold)
        {
        /* update hold state */

        ll_get_data(LLStep,LL_StepHoldType,&tmpI);

        switch(tmpI)
          {
          case HOLDTYPE_USER:

            J->Hold |= (1 << mhUser);

            break;

          case HOLDTYPE_SYSTEM:

            J->Hold |= (1 << mhSystem);

            break;

          case HOLDTYPE_USERSYS:

            J->Hold |= (1 << mhSystem);
            J->Hold |= (1 << mhUser);

            break;

          default:

            /* not handled */

            /* NO-OP */

            break;
          }  /* END switch(tmpI) */

        DBG(5,fLL) DPrint("INFO:     hold detected on job '%s' (%x)\n",
          J->Name,
          J->Hold);
        }  /* END if (J->State == mjsHold) */
      }    /* END BLOCK */

      break;

    case mjaStartTime:

      {
      long DispatchTime = 0;

      ll_get_data(LLStep,LL_StepDispatchTime,&DispatchTime);

      if (DispatchTime > J->DispatchTime)
        {
        J->DispatchTime = DispatchTime;
        J->StartTime    = DispatchTime;
        }
      else if (DispatchTime < J->DispatchTime)
        {
        char tmpString[MAX_MNAME];

        strcpy(tmpString,MULToTString(J->DispatchTime - MSched.Time));

        DBG(1,fLL) DPrint("ALERT:    dispatch time for job %s changed from %s to %s (%ld)\n",
          J->Name,
          tmpString,
          MULToTString(DispatchTime - MSched.Time),
          DispatchTime);
        }
      }  /* END BLOCK */

      break;

    case mjaState:

      {
      int tmpI;

      ll_get_data(LLStep,LL_StepState,&tmpI);

      J->State = __MLLJobStateToJState(J->State,tmpI);
      }  /* END BLOCK */

      break;

    case mjaUtlMem:

      if ((J->State != mjsStarting) && (J->State != mjsRunning))
        {
        /* job not active */

        break;
        }

      {
#ifdef __MLL31
#ifndef __INDIANA
      int TC;

      int TotalCPUSnap;
      int TotalMemSnap;

      int64_t MaxMem;
      int64_t CummCPU;

      int TotalMaxMem;
      int TotalCummCPU;

      int MemSnapshot;
      int CPUSnapshot;

      int Num;
      int Err;

      char stepid[MAX_MNAME];

      char *SArray[2] = { NULL, NULL };

      LL_element *qw;
      LL_element *Machine;
      LL_element *LLUsage;

      char *MName;

      if ((qw = ll_query(WLMSTAT)) == NULL)
        {
        /* cannot setup WLM query */

        break;
        }

      MJobAToString(J,mjaRMJID,stepid,0);

      SArray[0] = stepid;

      ll_set_request(qw,QUERY_STEPID,SArray,ALL_DATA);

      ll_get_data(LLStep,LL_StepGetFirstMachine,&Machine);

      TC           = 0;
      TotalCPUSnap = 0;
      TotalMemSnap = 0;

      TotalMaxMem  = 0;
      TotalCummCPU = 0;

      while (Machine)
        {
        if (ll_get_data(Machine,LL_MachineName,&MName) != 0)
          {
          DBG(1,fLL) DPrint("ALERT:    cannot load machine name\n");

          ll_get_data(LLStep,LL_StepGetNextMachine,&Machine);

          continue;
          }

        /* get WLM info from Startd on this machine */

        DBG(1,fLL) DPrint("INFO:     loading usage info from machine '%s'\n",
          MName);

        LLUsage = ll_get_objs(qw,LL_STARTD,MName,&Num,&Err);

        if (LLUsage != NULL)
          {
          ll_get_data(LLUsage,LL_WlmStatMemorySnapshotUsage,&MemSnapshot);
          ll_get_data(LLUsage,LL_WlmStatCpuSnapshotUsage,&CPUSnapshot);

          ll_get_data(LLUsage,LL_WlmStatMemoryHighWater,&MaxMem);
          ll_get_data(LLUsage,LL_WlmStatCpuTotalUsage,&CummCPU);

          DBG(1,fLL) DPrint("INFO:     usage for job %s on host %s: snapmem: %d  snapcpu: %d  himem: %d  totcpu: %d\n",
            J->Name,
            MName,
            MemSnapshot,
            CPUSnapshot,
            (int)MaxMem,
            (int)CummCPU);

          TotalMemSnap += MemSnapshot;
          TotalCPUSnap += CPUSnapshot;

          TotalMaxMem = MAX(TotalMaxMem,MaxMem);

          TotalCummCPU += CummCPU;

          TC++;

          ll_free_objs(qw);
          }  /* END if (LLUsage != NULL) */

        ll_deallocate(qw);

        ll_get_data(LLStep,LL_StepGetNextMachine,&Machine);
        }  /* END while (Machine) */

      if (TC > 0)
        {
        /* NOTE:  snapshot usage appears goofy.  needs work to consume */

        /*
        J->Req[0]->URes.Mem   = TotalMemSnap / TC;
        J->Req[0]->URes.Procs = TotalCPUSnap / TC;
        */

        J->Req[0]->URes.Mem   = TotalMaxMem;
        J->Req[0]->URes.Procs = CummCPU / (MSched.Time - J->StartTime);
        }  /* END if (TC > 0) */

#endif /* !__INDIANA */
#endif /* __MLL31 */
      }  /* END BLOCK */

      break;

    case mjaUtlProcs:

      /* NOTE:  handled via mjaUtlMem */

      /* NO-OP */

      break;

    default:

      /* not handled */

      return(FAILURE);

      /*NOTREACHED*/

      break; 
    }    /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MLLJobSetAttr() */




int MLLJobUpdate(

  mjob_t     *J,       /* I (modified) */
  LL_element *LLJob,   /* I */
  LL_element *LLStep,  /* I */
  LL_element *LLUsage, /* I (optional) */
  mrm_t      *R)       /* I */

  {
  int      OldState;
  int      OldHold;

  short    TaskList[MAX_MTASK_PER_JOB + 1];

  mreq_t  *RQ;

  int      aindex;

  const int AList[] = {
    mjaHold,
    mjaState,
    mjaStartTime,
    mjaUtlMem,
    mjaUtlProcs,
    -1 };

#if defined(__MLL22) || defined(__MLL31)
  LL_element *Node;
  LL_element *Task;
  LL_element *ResourceReq;
  char       *ReqName;
  int         ReqValue;

  int         rc;
#endif /* __MLL22 || __MLL31 */

  const char *FName = "MLLJobUpdate";

  DBG(4,fLL) DPrint("%s(%s,LLJob,LLStep,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if ((J == NULL) || (LLJob == NULL) || (LLStep == NULL))
    {
    return(FAILURE);
    }

  RQ = J->Req[0];

  OldState = J->State;
  OldHold  = J->Hold;

  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    MLLJobSetAttr(J,LLJob,LLStep,LLUsage,AList[aindex]);
    }  /* END for (aindex) */

  if (J->State != OldState)
    {
    DBG(1,fLL) DPrint("INFO:     job '%s' changed states from %s to %s\n",
      J->Name,
      MJobState[OldState],
      MJobState[J->State]);

    J->MTime = MSched.Time;
    }

  if (OldHold != J->Hold)
    J->MTime = MSched.Time;

  ll_get_data(LLStep,LL_StepTaskInstanceCount,&J->TaskCount);

  if ((J->State == mjsRemoved) || (J->State == mjsCompleted))
    {
    MLLJobSetAttr(J,LLJob,LLStep,LLUsage,mjaCompletionTime);
    }

  if (MJOBISALLOC(J) == TRUE)
    {
    MLLGetTL(LLStep,TaskList,&J->TaskCount);
    }
  else
    {
    TaskList[0] = -1;
    }

  /* compare LL messages */

#if defined(__MLL22) || defined(__MLL31)
  {
  char *LLMessages = NULL;

  ll_get_data(LLStep,LL_StepMessages,&LLMessages);

  if ((LLMessages != NULL) && (LLMessages[0] != '\0'))
    {
    DBG(1,fLL) DPrint("ALERT:    llmessage for job %s: '%s'\n",
      J->Name,
      LLMessages);

    free(LLMessages);
    }
  }    /* END BLOCK */

  ll_get_data(LLStep,LL_StepGetFirstNode,&Node);

  Task = NULL;

  if (Node != NULL)
    {
    ll_get_data(Node,LL_NodeGetFirstTask,&Task);
    }

  while (Task != NULL)
    {
    ResourceReq = NULL;

    ll_get_data(Task,LL_TaskGetFirstResourceRequirement,&ResourceReq);

    while (ResourceReq != NULL)
      {
      if ((rc = ll_get_data(ResourceReq,LL_ResourceRequirementName,&ReqName)) < 0)
        {
        DBG(1,fLL) DPrint("WARNING:  cannot get resource req name on job %s\n",
          J->Name);

        ResourceReq = NULL;

        ll_get_data(Task,LL_TaskGetNextResourceRequirement,&ResourceReq);
        }

      if ((rc = ll_get_data(ResourceReq,LL_ResourceRequirementValue,&ReqValue)) < 0)
        {
        DBG(1,fLL) DPrint("WARNING:  cannot get value for resource req %s on job %s\n",
          ReqName,
          J->Name);

        free(ReqName);

        ResourceReq = NULL;

        ll_get_data(Task,LL_TaskGetNextResourceRequirement,&ResourceReq);
        }

      DBG(1,fLL) DPrint("INFO:     resource req '%s' == %d loaded for job %s\n",
        ReqName,
        ReqValue,
        J->Name);

      if (!strcmp(ReqName,"ConsumableCpus"))
        {
        J->Req[0]->DRes.Procs = ReqValue;
        }
      else if (!strcmp(ReqName,"ConsumableVirtualMemory"))
        {
        J->Req[0]->DRes.Swap = ReqValue;
        }
      else if (!strcmp(ReqName,"ConsumableMemory"))
        {
        J->Req[0]->DRes.Mem = ReqValue;
        }

      free(ReqName);

      ResourceReq = NULL;

      ll_get_data(Task,LL_TaskGetNextResourceRequirement,&ResourceReq);
      }  /* END while (ResourceReq != NULL) */

    ll_get_data(Node,LL_NodeGetNextTask,&Task);
    }  /* END while (Task != NULL) */

#endif /* __MLL22 || __MLL31 */

  MRMJobPostUpdate(J,TaskList,OldState,R);

  return(SUCCESS);
  }  /* END MLLJobUpdate() */




int MLLGetTL(

  LL_element *LLStep, /* I */
  short      *TL,     /* O */
  int        *TC)     /* O (optional) */

  {
  LL_element *Machine;
  LL_element *Node;
  char       *MName;

  mnode_t    *N;

  int         tcount;
  int         mindex;
  int         tindex;
  int         nindex;

  int         NodeCount;
  int         TaskCountList[MAX_MNODE];
  int         MinInstanceList[MAX_MNODE];

  int         rc;

  const char *FName = "MLLGetTL";

  DBG(5,fLL) DPrint("%s(LLStep,TL,TC)\n",
    FName);

  if (TL == NULL)
    {
    return(FAILURE);
    }

  nindex = 0;

  ll_get_data(LLStep,LL_StepGetFirstNode,&Node);

  while (Node != NULL)
    {
    ll_get_data(Node,LL_NodeMinInstances,&MinInstanceList[nindex]);

    ll_get_data(Node,LL_NodeInitiatorCount,&TaskCountList[nindex]);

    nindex++;

    ll_get_data(LLStep,LL_StepGetNextNode,&Node);
    }

  ll_get_data(LLStep,LL_StepGetFirstMachine,&Machine);

  if (Machine == NULL)
    {
    DBG(1,fLL) DPrint("ALERT:    cannot get machine object for job\n");

    TL[0] = -1;

    if (TC != NULL)
      *TC = 0;

    return(FAILURE);
    }

  NodeCount = nindex;

  tcount = 0;

  for (nindex = 0;nindex < NodeCount;nindex++)
    {
    for (mindex = 0;mindex < MinInstanceList[nindex];mindex++)
      {
      if ((rc = ll_get_data(Machine,LL_MachineName,&MName)) < 0)
        {
        DBG(1,fLL) DPrint("ALERT:    cannot load hostname %d:%d from LL API for job hostlist (rc: %d)\n",
          nindex,
          mindex,
          rc);

        continue;
        }

      if (MNodeFind(MName,&N) == FAILURE)
        {
        DBG(1,fLL) DPrint("ALERT:    cannot locate host '%s' for job hostlist\n",
          MName);

        continue;
        }

      free(MName);

      for (tindex = 0;tindex < TaskCountList[nindex];tindex++)
        {
        TL[tcount] = N->Index;

        tcount++;
        } /* END for (tindex) */

      ll_get_data(LLStep,LL_StepGetNextMachine,&Machine);
      }   /* END for (mindex) */
    }     /* END for (nindex) */

  TL[tcount] = -1;

  if (TC != NULL)
    *TC = tcount;

  return(SUCCESS);
  }  /* END MLLGetTL() */



int MLLNodeUpdate(

  mnode_t             *N,        /* I (modified) */
  LL_element          *Machine,  /* I */
  enum MNodeStateEnum  State,    /* I */
  mrm_t               *R)        /* I */

  {
  const char *FName = "MLLNodeUpdate";

  DBG(4,fLL) DPrint("%s(%s,%p,%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    Machine,
    MAList[eNodeState][State],
    (R != NULL) ? R->Name : "NULL");

  if ((N == NULL) || (Machine == NULL))
    {
    return(FAILURE);
    }

  MLLNodeSetAttrs(N,Machine);

  if (N->State == mnsNone)
    {
    DBG(3,fLL) DPrint("WARNING:  node '%s' is unusable in state 'NONE'\n",
      N->Name);
    }

  if ((N->ARes.Disk < 0) && (N->State == mnsIdle))
    {
    DBG(2,fLL) DPrint("WARNING:  idle node %s is unusable  (inadequate disk space in /var)\n"
,
      N->Name);
    }

  DBG(5,fLL) DPrint("INFO:     node '%s' %8d %5d %5d %s\n",
    N->Name,
    N->ARes.Swap,
    N->ARes.Mem,
    N->ARes.Disk,
    MUMAList(eFeature,N->FBM,sizeof(N->FBM)));

  return(SUCCESS);
  }  /* END MLLNodeUpdate() */



  
/* END LLI.c */

