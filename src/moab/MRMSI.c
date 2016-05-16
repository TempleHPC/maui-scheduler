/* HEADER */
        
/* Contains:                                   *
 *                                             */

#include "moab.h"
#include "msched-proto.h"  

extern mlog_t    mlog;

extern msched_t  MSched;
extern mclass_t  MClass[];
extern mjob_t   *MJob[];
extern mnode_t  *MNode[];
extern msim_t    MSim;

/* NOTES: */

/* need list of partitions */
/* need nodes w/in partition */
/* need allocation status */
/* can obtain partition info at start up */
/* assumptions:  partition state is static  (for now) */
/*               node usage is dedicated              */
/* load node information from PBS first */
/* do we continue using day and night partitions when scheduler is running? */

#if !defined(__MRMSQUERYCMD)
# define __MRMSQUERYCMD "/usr/bin/rmsquery -u"
#endif /* !__MRMSQUERYCMD */

#if !defined(__MRMSSTARTCMD)
# define __MRMSSTARTCMD "/home2/test/bin/psub"
#endif /* !__MRMSSTARTCMD */

#if !defined(__MRMSCNTLCMD)
# define __MRMSCNTLCMD "/usr/bin/rcontrol"
#endif /* !__MRMSCNTLCMD */



int MRMSIInitialize(mrm_t *,int *);




int MRMSClusterQuery(

  mrm_t *R,
  int   *NCount,
  char  *EMsg,
  int   *SC)

  {
  const char *PartitionQuery = "select name,cpus from partitions where status='running'";

  char tmpBuffer[MAX_MBUFFER];
  char tmpName[MAX_MLINE];

  char tmpCommand[MAX_MLINE];

  char *ptr;
  char *TokPtr;

  mnode_t *N;

  int   ProcCount;

  int   rc;

  int   cindex;
  int   nindex;

  /* NOTE:  translate partition into single SMP node */

  /* cluster usage equivalent to dedicated processors */

  const char *FName = "MRMSClusterQuery";

  DBG(1,fRM) DPrint("%s(R,NC,SC)\n",
    FName);

  if (NCount != NULL)
    *NCount = 0;

  /* FORMAT:  '<NAME> <PROCCOUNT>\n'... */

  if (MSched.Iteration == 0)
    {
    /* load partition info */
 
    sprintf(tmpCommand,"%s \"%s\"",
      __MRMSQUERYCMD,
      PartitionQuery);
 
    rc = MUReadPipe(tmpCommand,tmpBuffer,sizeof(tmpBuffer));
 
    if (rc == FAILURE)
      {
      DBG(0,fPBS) DPrint("ERROR:    cannot load RMS partition information (%s)\n",
        tmpBuffer);
 
      return(FAILURE);
      }

    /* load/initialize node */

    ptr = MUStrTok(tmpBuffer,"\n",&TokPtr);

    while(ptr != NULL)
      {
      MUSScanF(ptr,"%x%s %ld",
        sizeof(tmpName),
        tmpName,
        &ProcCount);

      /* add node for each partition */

      MNodeAdd(tmpName,&N);

      MRMNodePreLoad(N,mnsIdle,R);

      N->State = mnsIdle;

      N->CRes.Procs = ProcCount;
      N->ARes.Procs = ProcCount;

      N->CRes.Mem   = 1000;
      N->ARes.Mem   = 1000;

      N->CRes.Disk  = 1000;
      N->ARes.Disk  = 1000;

      N->ActiveOS = MUMAGetIndex(eOpsys,"DEFAULT",mAdd);

      /* must add support for all queues */

      for (cindex = 2;cindex < MAX_MCLASS;cindex++)
        {
        if (MClass[cindex].Name[0] != '\0')
          {
          N->CRes.PSlot[cindex].count += ProcCount;
          N->CRes.PSlot[0].count      += ProcCount;
          }
        }    /* END for (cindex) */
 
      MRMNodePostLoad(N);

      ptr = MUStrTok(NULL,"\n",&TokPtr);
      }  /* END while(ptr != NULL) */
    }    /* END if (MSched.Iteration == 0) */

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    N->ATime = MSched.Time;

    if (NCount != NULL)
      (*NCount)++;

    }  /* END for (nindex) */

  /* NOTE:  utilized node resources adjusted in MRMSWorkloadQuery() */

  return(SUCCESS);
  }  /* END MRMSClusterQuery() */




int MRMSJobCancel(

  mjob_t *J,
  mrm_t  *R,
  char   *CMsg,
  char   *Msg,
  int    *SC)

  {
  char tmpLine[MAX_MLINE];

  int rc;

  const char *FName = "MRMSJobCancel";

  DBG(1,fRM) DPrint("%s(%s,%s,CMsg,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if ((J == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  /* issue RMS job cancel */

  sprintf(tmpLine,"kill %s",
    J->AName);

  rc = MRMSJobControl(J,tmpLine,NULL,NULL);

  if (rc == SUCCESS)
    {
    MJobSetState(J,mjsCompleted);
    }

  return(SUCCESS);
  }  /* END MRMSJobCancel() */




int MRMSJobStart(
  
  mjob_t *J,
  mrm_t  *R,
  char   *Msg,
  int    *SC)
 
  {
  char tmpCommand[MAX_MLINE];
  char tmpBuffer[5];

  int rc;

  char *ptr;
  char *TokPtr;
  
  int  index;

  /* use qrun -n <PROCCOUNT> -p <PROJECTID> <EXEC> <ARGS> */

  const char *FName = "MRMSJobStart";

  DBG(0,fPBS) DPrint("%s(%s,%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  sprintf(tmpCommand,"%s %d %s %s",
    __MRMSSTARTCMD,
    J->TasksRequested,
    J->E.Cmd,
    (J->E.Args != NULL) ? J->E.Args : "");

  for (index = 1;index < 4;index++)
    {
    /* NOTE:  loop to handle RMS race condition */

    DBG(3,fPBS) DPrint("INFO:     launching RMS job '%s'\n",
      tmpCommand);
 
    rc = MUReadPipe(tmpCommand,tmpBuffer,sizeof(tmpBuffer));
 
    if (rc == FAILURE)
      {
      DBG(0,fPBS) DPrint("ERROR:    cannot start RMS job (%s)\n",
        tmpBuffer);
 
      return(FAILURE);
      }

    DBG(3,fPBS) DPrint("INFO:     received response '%s' from RMS job launch\n",
      tmpBuffer);

    if (isdigit(tmpBuffer[0]))
      {
      /* PID received, job successfully launched */

      break;
      }
    }    /* END for (index) */

  J->StartTime = MSched.Time;
  J->StartCount++;

  /* extract jobid and specify alternate job name */

  if ((ptr = MUStrTok(tmpBuffer," \t\n",&TokPtr)) == NULL)
    {
    /* cannot parse output */

    DBG(0,fPBS) DPrint("ERROR:    cannot parse submit output (%s)\n",
      tmpBuffer);

    return(FAILURE);
    }
    
  MUStrDup(&J->E.Env,ptr);

/*
  if (J->Flags & (1 << mjfPreemptor))
    {
    MRMSJobPrioritize(J,R,NULL,NULL);
    }
*/

  return(SUCCESS);
  }  /* END MRMSJobStart() */




int MRMSJobResume(

  mjob_t *J,
  mrm_t  *R,
  char   *Msg,
  int    *SC)
 
  {
  int rc;

  char tmpLine[MAX_MLINE];

  /* issue RMS job resume */

  sprintf(tmpLine,"resume %s",
    J->AName);
 
  rc = MRMSJobControl(J,tmpLine,NULL,NULL);

  if (rc == SUCCESS)
    {
    MJobSetState(J,mjsStarting);
    }

  return(SUCCESS);
  }  /* END MRMSJobResume() */




int MRMSJobSuspend(

  mjob_t *J,
  mrm_t  *R,
  char   *Msg,
  int    *SC)
 
  {
  int rc;

  char tmpLine[MAX_MLINE];

  /* issue RMS job suspend */

  sprintf(tmpLine,"suspend %s",
    J->AName);
 
  rc = MRMSJobControl(J,tmpLine,NULL,NULL);

  if (rc == SUCCESS)
    {
    MJobSetState(J,mjsSuspended);
    }

  if (SC != NULL)
    *SC = rc;

  return(SUCCESS);
  }  /* END MRMSJobSuspend() */




int MRMSQueueQuery(

  mrm_t *R,
  int   *QCount,
  char  *EMsg,
  int   *SC)
 
  {
  /* no queue info required */

  return(SUCCESS);
  }  /* END MRMSQueueQuery() */




int MRMSWorkloadQuery(

  mrm_t *R,
  int   *JCount,
  int   *SC)
 
  {
  char tmpBuffer[MAX_MBUFFER];
  char tmpCommand[MAX_MLINE];

  char tmpName[MAX_MNAME];
  char tmpString[MAX_MNAME];

  int      nindex;
  int      jindex;

  mnode_t *N;
  mjob_t  *J;

  char    *ptr;

  short    TaskList[MAX_MTASK];
  int      OldState;

  const char *FName = "MRMSWorkloadQuery";

  DBG(0,fPBS) DPrint("%s(%s,JCount,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");

  /* NOTE:  only update state of active jobs */
  /*        do not load any job              */

  if (JCount != NULL)
    *JCount = 0;

  /* clear node usage info */

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    /* clear dedicated/available resources */

    memcpy(&N->ARes,&N->CRes,sizeof(N->ARes));

    memset(&N->DRes,0,sizeof(N->DRes));
    memset(&N->URes,0,sizeof(N->URes));  
    }  /* END for (nindex) */

  /* load active job info */

  /* NOTE:  need to request jobs where status is running OR suspended */

  sprintf(tmpCommand,"%s -v \"select status,pid,name from resources where status='allocated'\"",
    __MRMSQUERYCMD);
 
  if (MUReadPipe(tmpCommand,tmpBuffer,sizeof(tmpBuffer)) == FAILURE)
    {
    DBG(0,fPBS) DPrint("ERROR:    cannot load RMS information (%s)\n",
      tmpBuffer);
 
    return(FAILURE);
    }

  for (jindex = 1;jindex < MAX_MJOB;jindex++)
    {
    J = MJob[jindex];

    if ((J == NULL) || (J->Name[0] == '\0') || (J->Name[0] == '\1'))
      continue;

    if (JCount != NULL)
      (*JCount)++;

    MRMJobPreUpdate(J);

    J->ATime = MSched.Time;

    if ((J->State != mjsRunning) && (J->State != mjsStarting))
      continue;

    /* NOTE: job AName will be determine when job started */

    ptr = NULL;

    if ((J->AName == NULL) && (J->E.Env != NULL))
      {
      /* determine job resource name from PID */

      sprintf(tmpName," %s ",
        J->E.Env);

      if ((ptr = strstr(tmpBuffer,tmpName)) != NULL)
        {
        ptr += strlen(tmpName);

        if (MUSScanF(ptr,"%x%s ",
              sizeof(tmpString),
              tmpString) == 1)
          {
          MUStrDup(&J->AName,tmpString);
          }

        MJobSetState(J,mjsRunning);
        }
        
      }
    else if (J->AName != NULL)
      {
      sprintf(tmpName," %s ",
        J->AName);

      ptr = strstr(tmpBuffer,tmpName);
      }

    if (ptr != NULL)
      {
      int TC;

      /* job is active */

      ptr += strlen(tmpName);

      OldState = J->State;

      J->State = mjsRunning;

      N  = J->NodeList[0].N;
      TC = J->NodeList[0].TC;

      /* MCResAdd(&N->DRes,&N->CRes,&J->Req[0]->DRes,TC,FALSE); */

      MCResAdd(&N->URes,&N->CRes,&J->Req[0]->DRes,TC,FALSE);
      MCResRemove(&N->ARes,&N->CRes,&J->Req[0]->DRes,TC,FALSE);

      MJobAddToNL(J,NULL);

      TaskList[0] = J->NodeList[0].N->Index;
      TaskList[1] = -1;

      MRMJobPostUpdate(J,TaskList,OldState,R);

      DBG(3,fRM) DPrint("INFO:     active job '%s' detected\n",
        J->Name);
      }
    else if (J->State == mjsRunning)
      {
      /* job has completed */

      MJobSetState(J,mjsCompleted);

      DBG(3,fRM) DPrint("INFO:     job '%s' successfully completed\n",
        J->Name);

      MJobProcessCompleted(J);
      }
    else if (J->State == mjsStarting)
      {
      /* NOTE:  job has been launched by scheduler but not appeared in RMS yet */
      }
    }  /* END for (jindex) */

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    /* adjust node state */

    N->ARes.Procs = N->CRes.Procs - N->DRes.Procs;

    /* NOTE:  NodeAdjustState() only 'tightens' resource availability */
    /*        so, free it up                                          */
 
    N->State = mnsIdle;

    MNodeAdjustState(N,&N->State);

    N->EState = N->State;
    }  /* END for (nindex) */
 
  return(SUCCESS);
  }  /* END MRMSWorkloadQuery() */




int MRMSJobSubmit(

  char    *JobDesc,
  mrm_t   *R,
  mjob_t **JPtr,
  int     *SC)
 
  {
  mjob_t   *J;
  mreq_t   *RQ;
  mclass_t *C;

  long Walltime;
  int  ProcCount;

  char ExecString[MAX_MLINE];
  char QueueName[MAX_MLINE];
  char JobName[MAX_MLINE];

  char tmpJobID[MAX_MNAME];

  mqos_t *QDef;

  short TaskList[MAX_MNODE];

  char *JobString;

  char *ptr;
  char *TokPtr;

  time_t T;

  /* FORMAT:  <PROCCOUNT> <WALLTIME> <QUEUENAME> <EXECUTABLE> <JOBNAME> */

  JobString = MUStrTok(JobDesc,"+\n",&TokPtr);

  while (JobString != NULL)
    {
    for (ptr = JobString;*ptr != '\0';ptr++)
      {
      if (*ptr == ',')
        *ptr = ' ';
      }

    MUSScanF(JobString,"%ld %ld %x%s %x%s %x%s",
      &ProcCount,
      &Walltime,
      sizeof(QueueName),
      QueueName,
      sizeof(ExecString),
      ExecString,
      sizeof(JobName),
      JobName);
    
    /* NOTE:  
       - Create Job
       - RMPreload Job
       - Set QueueTime, WallTime, DefaultU, DefaultG, Executable, Args
       - Set State=Idle
    */

    MSimJobCreateName(tmpJobID,R);

    if (MJobCreate(tmpJobID,TRUE,&J) == FAILURE)
      {
      return(FAILURE);
      }

    if (MReqCreate(J,NULL,&RQ,FALSE) == FAILURE)
      {
      DBG(1,fPBS) DPrint("ALERT:    cannot add requirements to job '%s'\n",
        J->Name);
 
      MJobRemove(J);
 
      return(FAILURE);
      }
 
    MRMReqPreLoad(RQ);

    /* if new job, load data */
 
    MRMJobPreLoad(J,tmpJobID,R->Index);

    J->TasksRequested             = ProcCount;
    J->Req[0]->TaskCount          = ProcCount;
    J->Req[0]->TaskRequestList[0] = ProcCount;
    J->Req[0]->TaskRequestList[1] = ProcCount;

    J->WCLimit        = Walltime;
    J->SpecWCLimit[0] = Walltime;
    J->SpecWCLimit[1] = Walltime;

    J->ATime = MSched.Time;

    /* must incorporate queue */

    MUStrDup(&J->E.Cmd,ExecString);

    MUStrDup(&J->E.Args,JobName);

    if (MJobSetCreds(J,"DEFAULT","DEFAULT","DEFAULT") == FAILURE)
      {
      DBG(1,fPBS) DPrint("ERROR:    cannot authenticate job '%s' (U: %s  G: %s  A: '%s')\n",
        J->Name,
        "DEFAULT",
        "DEFAULT",
        "DEFAULT");
 
      MJobRemove(J);
 
      return(FAILURE);
      }

    if (MClassFind(QueueName,&C) == FAILURE)
      {
      DBG(1,fPBS) DPrint("ERROR:    cannot locate requested class '%s' for job '%s'\n",
        QueueName,
        J->Name);
 
      MJobRemove(J);
 
      return(FAILURE);
      }

    RQ->DRes.PSlot[0].count        += 1;
    RQ->DRes.PSlot[C->Index].count += 1;

    if ((MQOSGetAccess(J,J->QReq,NULL,&QDef) == FAILURE) ||
        (J->QReq == NULL))
      {
      MJobSetQOS(J,QDef,0);
      }
    else
      {
      MJobSetQOS(J,J->QReq,0);
      }

    /* job is newly submitted so guaranteed idle */

    J->State = mjsIdle;         

    TaskList[0] = -1;

    MRMJobPostLoad(J,TaskList,R);

    time(&T);
 
    R->LastSubmissionTime = MAX(MSched.Time,(long)T);
 
    DBG(2,fPBS)
      MJobShow(J,0,NULL);

    if (JPtr != NULL)
      *JPtr = J;

    JobString = MUStrTok(NULL,"+\n",&TokPtr);
    }  /* END while (JobString != NULL) */

  return(SUCCESS);
  }  /* END MRMSJobSubmit() */




int MRMSLoadModule(
 
  mrmfunc_t *F)  /* I */
 
  {
  if (F == NULL)
    {
    return(FAILURE);
    }
 
  F->ClusterQuery   = MRMSClusterQuery;
  F->JobCancel      = MRMSJobCancel;
  F->JobModify      = NULL;
  F->JobQuery       = NULL;
  F->JobRequeue     = NULL;
  F->JobResume      = MRMSJobResume;
  F->JobStart       = MRMSJobStart;
  F->JobSuspend     = MRMSJobSuspend;
  F->QueueQuery     = MRMSQueueQuery;
  F->ResourceModify = NULL;
  F->ResourceQuery  = NULL;
  F->RMInitialize   = MRMSIInitialize;
  F->RMQuery        = NULL;
  F->WorkloadQuery  = MRMSWorkloadQuery;
  F->RMEventQuery   = NULL;
 
  return(SUCCESS);
  }  /* END MRMSLoadModule() */




int MRMSIInitialize(
  
  mrm_t *R,
  int   *SC)

  {
  DBG(1,fPBS) DPrint("INFO:     RMS server initialized\n");

  return(SUCCESS);
  }  /* END MRMSIInitialize() */





int MRMSInitialize()

  {
  const char *QueryCommand   = "/bin/rmsquery -u";
  /* const char *PartitionQuery = "select name,configured_nodes,type,cpus,free_cpus from partitions where status='running'"; */
  const char *PartitionQuery = "select name,configured_nodes from partitions where status='running'";
  /* const char *NodeQuery      = "select name from nodes where status='running'"; */


  char tmpCommand[MAX_MLINE];
  char tmpBuffer[MAX_MBUFFER];

  mpar_t  *P;

  int      nindex;
  int      rc;

  char    *ptr;
  char    *TokPtr;

  int      NodeCount;

  char     PartitionName[MAX_MNAME];
  char     HostExpression[MAX_MLINE];

  short    NodeList[MAX_MNODE];

  mnode_t *N;

  /* load partition info */

  sprintf(tmpCommand,"%s \"%s\"",
    QueryCommand,
    PartitionQuery);

  rc = MUReadPipe(tmpCommand,tmpBuffer,sizeof(tmpBuffer));

  if (rc == FAILURE)
    {
    DBG(0,fPBS) DPrint("ERROR:    cannot load RMS information (%s)\n",
      tmpBuffer);

    return(FAILURE);
    }

  /* parse partition info */

  ptr = MUStrTok(tmpBuffer,"\n",&TokPtr);

  while (ptr != NULL)
    {
    /* FORMAT:  Name Type HostList HostList2? Nodes Nodes? ??? CreationDay CreationTime Status ??? AccessType ??? ??? ??? */

    rc = sscanf(ptr,"%s %s",
           PartitionName,
           HostExpression);

    ptr = MUStrTok(NULL,"\n",&TokPtr);
    
    if (rc != 2)
      {
      /* cannot read all needed fields */

      continue;
      }

    MParAdd(PartitionName,&P);

    if (MUREToList(HostExpression,mxoNode,0,NodeList,&NodeCount,NULL) == FAILURE)  
      {
      /* cannot process node expression */

      continue;
      }
 
    for (nindex = 0;NodeList[nindex] != -1;nindex++)
      {
      N = MNode[NodeList[nindex]];

      MParAddNode(P,N);

      N->SlotIndex = nindex;
      }  /* END for (nindex) */
    }    /* END while (ptr != NULL) */ 
   
  return(SUCCESS); 
  }  /* END MRMSInitialize() */




int MJobAllocateContiguous(
 
  mjob_t *J,                /* IN: job allocating nodes                           */
  mreq_t *RQ,               /* IN: req allocating nodes                           */
  mnalloc_t *NodeList,      /* IN: eligible nodes                                 */
  int     RQIndex,          /* IN: index of job req to evaluate                   */
  int    *MinTPN,           /* IN: min tasks per node allowed                     */
  int    *MaxTPN,           /* IN: max tasks per node allowed                     */
  char   *NodeMap,          /* IN: array of node alloc states                     */
  int     AffinityLevel,    /* IN: current reservation affinity level to evaluate */
  int    *NodeIndex,        /* IN/OUT: index of next node to find in BestList     */
  mnalloc_t *BestList[MAX_MREQ_PER_JOB],   /* IN/OUT: list of selected nodes      */
  int    *TaskCount,        /* IN/OUT: total tasks allocated to req               */
  int    *NodeCount)        /* IN/OUT: total nodes allocated to req               */
 
  {
  int nindex;
  int TC;
 
  mnode_t *N;
 
  mnalloc_t MyNodeList[MAX_MNODE];
  int         MyNIndex;

  mnalloc_t tmpNodeList[MAX_MPAR][MAX_MNODE];
 
  /* select first 'RQ->TaskCount' adjacent procs */

  MyNIndex = 0;

  for (nindex = 0;NodeList[nindex].N != NULL;nindex++)
    {
    N  = NodeList[nindex].N;
    TC = NodeList[nindex].TC;
 
    if (NodeMap[N->Index] != AffinityLevel)
      {
      /* node unavailable */ 
 
      continue;
      }
 
    if (TC < MinTPN[RQIndex])
      continue;
 
    TC = MIN(TC,MaxTPN[RQIndex]);
 
    /* add node to private list */
 
    MyNodeList[MyNIndex].TC = TC;
    MyNodeList[MyNIndex].N  = N;

    MyNIndex++;
    }  /* END for (nindex) */

  if (MyNIndex == 0)
    {
    /* no nodes located */
 
    return(FAILURE);
    }
 
  MyNodeList[MyNIndex].N = NULL;

  /* select adjacent nodes */      

  if (MRMSSelectAdjacentNodes(
        NodeList[0].N->PtIndex,
        J->TasksRequested,
        MyNodeList,
        tmpNodeList) == FAILURE)
    {
    /* cannot locate adequate adjacent nodes */
 
    return(FAILURE);
    }
 
  /* NOT IMPLEMENTED */
 
  /* populate BestList with selected nodes */
 
  for (nindex = 0;MyNodeList[nindex].N != NULL;nindex++)
    {
    N  = tmpNodeList[0][nindex].N;
    TC = tmpNodeList[0][nindex].TC;
 
    BestList[RQIndex][NodeIndex[RQIndex]].N  = N;
    BestList[RQIndex][NodeIndex[RQIndex]].TC = TC;
 
    NodeIndex[RQIndex] ++;
    TaskCount[RQIndex] += TC;
    NodeCount[RQIndex] ++; 
 
    /* mark node as used */
 
    NodeMap[N->Index] = nmUnavailable;
 
    if (TaskCount[RQIndex] >= RQ->TaskCount)
      {
      /* all required tasks found */
 
      /* NOTE:  HANDLED BY DIST */
 
      if ((RQ->NodeCount == 0) ||
          (NodeCount[RQIndex] >= RQ->NodeCount))
        {
        /* terminate BestList */
 
        BestList[RQIndex][NodeIndex[RQIndex]].N = NULL;
 
        break;
        }
      }
    }     /* END for (nindex) */
 
  return(SUCCESS);
  }  /* END MJobAllocateContiguous() */




int MRMSSelectAdjacentNodes(

  int         PIndex,
  int         MinTasks,
  mnalloc_t NodeList[],
  mnalloc_t AdjNodeList[][MAX_MNODE])

  {
  int StartIndex;
  int index;
  int rindex;
  int nindex;

  mnalloc_t tmpNodeList[MAX_MNODE];

  int TC;

  mnode_t *N;

  memset(tmpNodeList,0,sizeof(tmpNodeList));

  for (nindex = 0;NodeList[nindex].N != NULL;nindex++)
    {
    N = NodeList[nindex].N;

    if (N->PtIndex != PIndex)
      continue;

    memcpy(&tmpNodeList[MAX(0,N->SlotIndex)],&NodeList[nindex],sizeof(tmpNodeList[0]));
    }  /* END for (nindex) */

  rindex = 0;

  TC = 0;
  StartIndex = 0;

  for (nindex = 0;nindex < MAX_MNODE;nindex++) 
    {
    if (tmpNodeList[nindex].TC > 0)
      {
      if (TC == 0)
        StartIndex = nindex;

      TC += tmpNodeList[nindex].TC;

      continue;
      }
    else if (TC >= MinTasks)
      {
      /* save range */

      for (index = StartIndex;index < nindex;index++)
        {
        memcpy(&AdjNodeList[rindex][index - StartIndex],&tmpNodeList[index],sizeof(tmpNodeList[0]));   
        }

      AdjNodeList[rindex][index - StartIndex].N = NULL;

      rindex++;

      if (rindex >= MAX_MPAR)
        break;
      }

    TC = 0;
    }  /* END for (nindex) */

  AdjNodeList[rindex][0].N = NULL;

  if (rindex == 0)
    return(FAILURE);

  return(SUCCESS);
  }  /* END MRMSSelectAdjacentNodes() */




int MRMSQueryJob(

  mjob_t *J,
  short   TaskList[],
  int    *TaskCount)

  {
  const char *QueryCommand = "/bin/rmsquery -u";

  char Command[MAX_MLINE];
  char tmpBuffer[MAX_MBUFFER];

  char RMSJobID[MAX_MLINE];
  char HostExpression[MAX_MLINE];

  int   nindex;
  int   tlindex;
  int   tindex;

  int   rc;

  int   NodeCount;
  int   PC;

  short NodeList[MAX_MNODE];

  mnode_t *N;

  const char *FName = "MRMSQueryJob";

  DBG(3,fPBS) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if ((J == NULL) || (J->SessionID <= 0))
    {
    return(FAILURE);
    }

  if (TaskCount != NULL)
    *TaskCount = 0;

  /* load RMS job info */

  sprintf(Command,"%s \"select name,hostnames from jobs where session=%d\"",
    QueryCommand,
    J->SessionID);

  if (MUReadPipe(Command,tmpBuffer,sizeof(tmpBuffer)) == FAILURE)
    {
    DBG(0,fPBS) DPrint("ERROR:    cannot load RMS information (%s)\n",
      tmpBuffer);

    return(FAILURE);
    }

  /* parse job node info */

  rc = sscanf(tmpBuffer,"%s %s",
         RMSJobID,
         HostExpression);

  if (rc != 2)
    {
    /* cannot read all needed fields */

    return(FAILURE);
    }

  MUStrDup(&J->AName,RMSJobID);

  /* add allocated hosts to task list */

  tmpBuffer[0] = '\0';

  if (MUREToList(HostExpression,mxoNode,0,NodeList,&NodeCount,tmpBuffer) == FAILURE)
    {
    /* cannot process node expression */

    return(FAILURE);
    }

  tlindex = 0;

  PC = (MNode[0] != NULL) ? MNode[0]->CRes.Procs : 1;

  for (nindex = 0;NodeList[nindex] != -1;nindex++)
    {
    N = MNode[NodeList[nindex]];

    for (tindex = 0;tindex < PC;tindex++)
      {
      TaskList[tlindex] = N->Index;

      tlindex++;
      }
    }  /* END for (nindex) */

  TaskList[tlindex] = -1;

  if (TaskCount != NULL)
    *TaskCount = tlindex;

  return(SUCCESS);
  }  /* END MRMSQueryJob() */




int MRMSJobPrioritize(

  mjob_t *J,
  mrm_t  *R,
  char   *Msg,
  int    *SC)

  {
  char tmpLine[MAX_MLINE];

  /* NYI */

  sprintf(tmpLine,"set resource = %s priority = 100",
    J->AName);

  MRMSJobControl(J,tmpLine,Msg,SC);
  
  return(SUCCESS);
  }  /* END MRMSJobPrioritize() */





int MRMSJobControl(
 
  mjob_t *J,
  char   *Command,
  char   *Msg,
  int    *SC) 
 
  {
  char CmdString[MAX_MLINE];
  char tmpBuffer[MAX_MBUFFER];
 
  int   rc;

  const char *FName = "MRMSJobControl";
 
  DBG(3,fPBS) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");
 
  if ((J == NULL) || (J->AName == NULL))
    {
    return(FAILURE);
    }
 
  /* create RMS job control command */
 
  sprintf(CmdString,"%s %s",
    __MRMSCNTLCMD,
    Command);

  if ((rc = MUReadPipe(CmdString,tmpBuffer,sizeof(tmpBuffer))) == FAILURE)
    {
    DBG(0,fPBS) DPrint("ERROR:    cannot load RMS information (%s)\n",
      tmpBuffer);
 
    return(FAILURE);
    }
 
  /* parse job node info */
 
  if (Msg != NULL)
    strcpy(Msg,tmpBuffer);

  return(SUCCESS);
  }  /* END MRMSJobControl() */

/* END MRMSI.c */

