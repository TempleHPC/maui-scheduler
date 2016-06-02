/* HEADER */

/*                                                       *
 * Contains:                                             *
 *                                                       *
 *  int MWikiClusterLoadInfo(R,RCount,SC)                *
 *  int MWikiWorkloadQuery(R,JCount,SC)                  *
 *  int MWikiJobStart(J,R,Message,SC)                    *
 *  int MWikiCancelJob(J,R,CMsg,Msg,SC)                  *
 *  int MWikiSuspendJob(J,R)                             *
 *  int MWikiResumeJob(J,R)                              *
 *  int MWikiJobLoad(JobName,AList,WikiJob,J,R)          *
 *  int MWikiUpdateJob(WikiJob,J,RMIndex)                *
 *  int MWikiUpdateJobAttr(Tok,J)                        *
 *  int MWikiNodeLoad(ALine,N,R)                         *
 *  int MWikiNodeUpdate(WikiNode,N)                      *
 *  int MWikiNodeUpdateAttr(Tok,N)                       *
 *  int MGetWikiAttr(OIndex,Name,Status,AttrLine,Start)  *
 *  int MWikiAttrToTaskList(TaskList,JobAttr,TaskCount)  *
 *                                                       */

#include "moab.h"
#include "msched-proto.h"

extern mlog_t      mlog;

extern mrm_t       MRM[];
extern msched_t    MSched;
extern mpar_t      MPar[];
extern mnode_t    *MNode[];
 
extern mattrlist_t MAList;

extern const char *MRMType[];
extern const char *MRMAuthType[];
extern const char *MHRObj[];
extern const char *MCKeyword[];
extern const char *MNodeState[];
extern const char *MCBType[];
extern const char *WMCommandList[];
extern const char *MWikiNodeAttr[];
extern const char *MWikiJobAttr[];
extern const char *MXO[];

extern mx_t        X;

typedef struct {
  char *ClusterInfoBuffer;
  char *WorkloadInfoBuffer;
  } wikirm_t;



/* prototypes */

int MWikiInitialize(mrm_t *,int *);
int MWikiClusterLoadInfo(mrm_t *,int *,char *,int *);
int MWikiJobStart(mjob_t *,mrm_t *,char *,int *);
int MWikiJobCancel(mjob_t *,mrm_t *,char *,char *,int *);
int MWikiJobSuspend(mjob_t *,mrm_t *,char *,int *);
int MWikiJobResume(mjob_t *,mrm_t *,char *,int *); 
int MWikiJobRequeue(mjob_t *,mrm_t *,mjob_t **,char *,int *);
int MWikiWorkloadQuery(mrm_t *,int *,int *);
int MWikiUpdateJob(char *,mjob_t *J,int); 
int MWikiNodeLoad(char *,mnode_t *,mrm_t *); 
int MWikiNodeUpdate(char *,mnode_t *);
int MWikiNodeUpdateAttr(char *,mnode_t *);
int MWikiUpdateJobAttr(char *,mjob_t *);
int MWikiGetAttr(int,char *,int *,char *,char **); 
int MWikiAttrToTaskList(short *,char *,int *);

/* END prototypes */




int MWikiInitialize(

  mrm_t *R,
  int   *SC)

  {
  const char *FName = "MWikiInitialize";

  DBG(1,fWIKI) DPrint("%s(%s,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");
 
  if (R == NULL)
    {
    return(FAILURE);
    }

  if (R->S == NULL)
    {
    R->S = (void *)calloc(1,sizeof(wikirm_t));
 
    /* set default rm specific values */
 
    /* NYI */
    }

  R->FailIteration = -1;

  return(SUCCESS);
  }  /* END MWikiInitialize() */




int MWikiLoadModule(
 
  mrmfunc_t *F)  /* I */
 
  {
  if (F == NULL)
    {
    return(FAILURE);
    }

  F->ClusterQuery   = MWikiClusterLoadInfo; 
  F->JobCancel      = MWikiJobCancel;
  F->JobCheckpoint  = NULL;
  F->JobModify      = NULL;
  F->JobQuery       = NULL;
  F->JobRequeue     = MWikiJobRequeue;
  F->JobResume      = MWikiJobResume;
  F->JobSuspend     = MWikiJobSuspend;
  F->JobStart       = MWikiJobStart;
  F->QueueQuery     = NULL;
  F->ResourceModify = NULL;
  F->ResourceQuery  = NULL;
  F->RMInitialize   = MWikiInitialize;
  F->RMQuery        = NULL;
  F->WorkloadQuery  = MWikiWorkloadQuery;
 
  return(SUCCESS);
  }  /* END MWikiLoadModule() */




int MWikiJobRequeue(

  mjob_t  *J,
  mrm_t   *R,
  mjob_t **JPeer,
  char    *Msg,
  int     *SC)

  {
  /* NYI */

  return(FAILURE);
  }  /* END MWikiJobRequeue */




int MWikiClusterLoadInfo(

  mrm_t *R,
  int   *RCount,
  char  *EMsg,
  int   *SC)

  {
  char   Command[MAX_MLINE];
  char   ArgLine[MAX_MLINE];

  int    tmpSC;

  char  *WikiBuf;
  char **Data;
  long   DataSize;

  char  *ptr;

  char   Name[MAX_MNAME];
  char   AttrLine[MAX_MBUFFER];

  mnode_t *N;

  int    nindex;
  int    ncount;

  int    Status;
  int    OldState;

  long   MTime;

  wikirm_t *S;

  const char *FName = "MWikiClusterLoadInfo";

  DBG(2,fWIKI) DPrint("%s(%s,RCount,EMsg,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");

  if (R == NULL)
    {
    return(FAILURE);
    }

  if (RCount != NULL)
    *RCount = 0;

  MTime = 0;

  S = (wikirm_t *)R->S;

  if ((S == NULL) || (S->ClusterInfoBuffer == NULL))
    {
    sprintf(ArgLine,"%ld:ALL",
      MTime);

    sprintf(Command,"%s%s %s%s",
      MCKeyword[mckCommand],
      WMCommandList[cmdWMGetNodes],
      MCKeyword[mckArgs],
      ArgLine);

    if (MWikiDoCommand(
          R->P[0].HostName,
          R->P[0].Port,
          R->P[0].Timeout * 1000000,
          R->AuthType,
          Command,
          &WikiBuf,
          &DataSize,
          &tmpSC) == FAILURE)
      {
      DBG(2,fWIKI) DPrint("ALERT:    cannot get node list from %s RM\n",
        MRMType[R->Type]);

      return(FAILURE);
      }

    if (tmpSC < 0)
      {
      DBG(2,fWIKI) DPrint("ALERT:    cannot get node list from %s RM\n",
        MRMType[R->Type]);

      return(FAILURE);
      }

    DBG(2,fWIKI) DPrint("INFO:     received node list through %s RM\n",
      MRMType[R->Type]);

    Data = &WikiBuf;
    }  /* END if ((S == NULL) || ... ) */
  else
    {
    Data     = &S->ClusterInfoBuffer;
    DataSize = (long)strlen(S->ClusterInfoBuffer);
    }

  /* load node data */

  if ((ptr = strstr(*Data,MCKeyword[mckArgs])) == NULL)
    {
    DBG(1,fWIKI) DPrint("ALERT:    cannot locate ARG marker in %s()\n",
      FName);

    free(*Data);

    return(FAILURE);
    }
    
  ptr += strlen(MCKeyword[mckArgs]);

  /* FORMAT:  <NODECOUNT>#<NODEID>:<FIELD>=<VALUE>;[<FIELD>=<VALUE>;]...#[<NODEID>:...] */

  ncount = (int)strtol(ptr,NULL,0);

  if (ncount <= 0)
    {
    DBG(1,fWIKI) DPrint("INFO:     no node data sent by %s RM\n",
      MRMType[R->Type]);

    free(*Data);

    return(FAILURE);
    }
  else
    {
    DBG(2,fWIKI) DPrint("INFO:     loading %d node(s)\n",
      ncount);
    }

  if ((ptr = MUStrChr(ptr,'#')) == NULL)
    {
    DBG(1,fWIKI) DPrint("ALERT:    cannot locate start marker for first node in %s()\n",
      FName);

    free(*Data);

    return(FAILURE);
    }

  ptr++;

  for (nindex = 0;nindex < ncount;nindex++)
    {
    if (MWikiGetAttr(mxoNode,Name,&Status,AttrLine,&ptr) == FAILURE)
      {
      DBG(2,fWIKI) DPrint("ALERT:    cannot get wiki state information\n");

      break;
      }

    if (MNodeFind(Name,&N) == SUCCESS)
      {
      /* update existing node */

      OldState = N->State;

      MRMNodePreUpdate(N,Status,R);

      MWikiNodeUpdate(AttrLine,N);

      MRMNodePostUpdate(N,OldState);
      }
    else if (MNodeAdd(Name,&N) == SUCCESS)
      {
      /* if new node, load data */

      MRMNodePreLoad(N,mnsDown,R);

      MWikiNodeLoad(AttrLine,N,R);

      MRMNodePostLoad(N);

      DBG(2,fWIKI)
        MNodeShow(N);
      }
    else
      {
      DBG(1,fWIKI) DPrint("ERROR:    node buffer is full  (ignoring node '%s')\n",
        Name);
      }
    }    /* END for (nindex) */

  if (RCount != NULL)
    *RCount = ncount;

  /* clean up */

  free(*Data);

  return(SUCCESS);
  }  /* END MWikiClusterLoadInfo() */





int MWikiWorkloadQuery(

  mrm_t *R,
  int   *JCount,
  int   *SC)

  {
  char   Command[MAX_MLINE];
  char   ArgLine[MAX_MLINE];

  int    tmpSC;
  char **Data;
  char  *WikiBuf;

  long   DataSize;

  char  *ptr;

  char   Name[MAX_MNAME];
  char   AttrLine[MAX_MBUFFER];
 
  mjob_t *J;

  int    jindex;
  int    jcount;

  int    Status;

  long   MTime;

  short  TaskList[MAX_MTASK_PER_JOB];

  char   Message[MAX_MLINE];

  wikirm_t *S;

  const char *FName = "MWikiWorkloadQuery";

  DBG(2,fWIKI) DPrint("%s(%s,JCount,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");

  if (R == NULL)
    {
    return(FAILURE);
    }

  if (JCount != NULL)
    *JCount = 0;

  S = (wikirm_t *)R->S;

  MTime = 0;

  if ((S == NULL) || (S->WorkloadInfoBuffer == NULL))
    {
    sprintf(ArgLine,"%ld:ALL",
      MTime);

    sprintf(Command,"%s%s %s%s",
      MCKeyword[mckCommand],
      WMCommandList[cmdWMGetJobs],
      MCKeyword[mckArgs],
      ArgLine);

    if (MWikiDoCommand(
          R->P[0].HostName,
          R->P[0].Port,
          R->P[0].Timeout * 1000000,
          R->AuthType,
          Command,
          &WikiBuf,
          &DataSize,
          &tmpSC) == FAILURE)
      {
      DBG(2,fWIKI) DPrint("ALERT:    cannot get job list from %s RM\n",
        MRMType[R->Type]);

      return(FAILURE);
      }

    if (tmpSC < 0)
      {
      DBG(2,fWIKI) DPrint("ALERT:    cannot get job list from %s RM\n",
        MRMType[R->Type]);

      return(FAILURE);
      }

    DBG(2,fWIKI) DPrint("INFO:     received job list through %s RM\n",
      MRMType[R->Type]);

    Data = &WikiBuf;
    }
  else
    {
    Data = &S->WorkloadInfoBuffer;

    DataSize = (long)strlen(S->WorkloadInfoBuffer);
    }

  /* load job data */

  if ((ptr = strstr(*Data,MCKeyword[mckArgs])) == NULL)
    {
    DBG(1,fWIKI) DPrint("ALERT:    cannot locate ARG marker in %s()\n",
      FName);

    free(*Data);
 
    return(FAILURE);
    }

  ptr += strlen(MCKeyword[mckArgs]);

  /* FORMAT:  <JOBCOUNT>#<JOBID>:<FIELD>=<VALUE>;[<FIELD>=<VALUE>;]... */

  jcount = (int)strtol(ptr,NULL,0);

  if (jcount <= 0)
    {
    DBG(1,fWIKI) DPrint("INFO:     no job data sent by %s RM\n",
      MRMType[R->Type]);

    free(*Data);

    return(FAILURE);
    }
  else
    {
    DBG(2,fWIKI) DPrint("INFO:     loading %d job(s)\n",
      jcount);
    }

  if ((ptr = MUStrChr(ptr,'#')) == NULL)
    {
    DBG(1,fWIKI) DPrint("ALERT:    cannot locate start marker for first job in %s()\n",
      FName);

    free(*Data);

    return(FAILURE);
    }

  ptr++;

  for (jindex = 0;jindex < jcount;jindex++)
    {
    if (MWikiGetAttr(mxoJob,Name,&Status,AttrLine,&ptr) == FAILURE)
      break;

    switch(Status)
      {
      case mjsIdle:
      case mjsHold:
      case mjsUnknown:
      case mjsStarting:
      case mjsRunning:
      case mjsSuspended: 
      
        if (MJobFind(Name,&J,0) == SUCCESS)
          {
          /* update existing job */

          MRMJobPreUpdate(J);

          MWikiUpdateJob(AttrLine,J,R->Index);
          }
        else if (MJobCreate(
                  MJobGetName(NULL,Name,R,NULL,0,mjnShortName),TRUE,&J) == SUCCESS)
          {
          /* if new job, load data */

          MRMJobPreLoad(J,Name,R->Index);

          if (MWikiJobLoad(Name,AttrLine,J,TaskList,R) == FAILURE)
            {
            DBG(1,fWIKI) DPrint("ALERT:    cannot load wiki job '%s'\n",
              Name);

            MJobRemove(J);

            continue;
            }

          MRMJobPostLoad(J,TaskList,R);

          DBG(2,fWIKI)
            MJobShow(J,0,NULL);
          }
        else
          {
          DBG(1,fWIKI) DPrint("ERROR:    job buffer is full  (ignoring job '%s')\n",
            Name);
          }

        break;

      case mjsRemoved:
      case mjsCompleted:
      case mjsVacated:

        if (MJobFind(Name,&J,0) == SUCCESS)
          {
          /* if job never ran, remove record.  job cancelled externally */

          if ((J->State != mjsRunning) && (J->State != mjsStarting))
            {
            DBG(1,fWIKI) DPrint("INFO:     job '%s' was cancelled externally\n",
              J->Name);

            /* remove job from joblist */

            MJobRemove(J);

            break;
            }
 
          MRMJobPreUpdate(J);
 
          MWikiUpdateJob(AttrLine,J,R->Index);

          switch(Status)
            {
            case mjsRemoved:
            case mjsVacated:

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

                DBG(3,fWIKI) DPrint("INFO:     job '%s' exceeded wallclock limit %s\n",
                  J->Name,
                  MULToTString(J->WCLimit));

                MJobProcessCompleted(J);
                }

              break;

            case mjsCompleted:

              MJobProcessCompleted(J);

              break;

            default:
  
              /* unexpected job state */
  
              DBG(1,fWIKI) DPrint("WARNING:  unexpected job state (%d) detected for job '%s'\n",
                Status,
                J->Name);

              break;
            }   /* END switch (Status)                        */
          }     /* END if (MJobFind(JobName,&J,0) == SUCCESS) */
        else
          {
          /* ignore job for now */

          DBG(4,fWIKI) DPrint("INFO:     ignoring job '%s'  (state: %s)\n",
            Name,
            MJobState[Status]);
          }

        break;

      default:

        DBG(1,fWIKI) DPrint("WARNING:  job '%s' detected with unexpected state '%d'\n",
          Name,
          Status);

        break;
      }
    }    /* END for jindex */

  /* clean up */

  free(*Data);

  if (JCount != NULL)
    *JCount = jcount;

  return(SUCCESS);
  }  /* END MWikiWorkloadQuery() */





int MWikiJobStart(

  mjob_t *J,
  mrm_t  *R,
  char   *Msg,
  int    *SC)

  {
  char    Command[MAX_MBUFFER];
  char    ArgLine[MAX_MBUFFER];
  char    TaskLine[MAX_MBUFFER];

  int     tindex;
  int     tcount;

  int     tmpSC;

  mnode_t *N;

  char   *Response;

  char    tmpLine[MAX_MLINE];
  char   *MPtr;

  const char *FName = "MWikiJobStart";

  DBG(2,fWIKI) DPrint("%s(%s,%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if (Msg != NULL)
    MPtr = Msg;
  else
    MPtr = tmpLine;

  MPtr[0] = '\0';

  if ((J == NULL) || (R == NULL))
    {
    sprintf(MPtr,"start request is corrupt");

    return(FAILURE);
    }

  TaskLine[0] = '\0';

  tcount = 0;

  for (tindex = 0;tindex < MAX_MTASK_PER_JOB;tindex++)
    {
    if (J->TaskMap[tindex] == -1)
      break;

    N = MNode[J->TaskMap[tindex]];

    if (N->RM == R)
      {
      if (tcount > 0)
        MUStrCat(TaskLine,":",sizeof(TaskLine));

      MUStrCat(TaskLine,N->Name,sizeof(TaskLine));

      tcount++;
      }
    }   /* for (tindex = 0) */

  if (tcount == 0)
    {
    DBG(3,fWIKI) DPrint("ALERT:    no tasks found for job '%s' on RM %d\n",
      J->Name,
      R->Index);

    return(FAILURE);
    }

  sprintf(ArgLine,"%s TASKLIST=%s",
    J->Name,
    TaskLine);

  sprintf(Command,"%s%s %s%s",
    MCKeyword[mckCommand],
    WMCommandList[cmdWMStartJob],
    MCKeyword[mckArgs],
    ArgLine);

  if (MWikiDoCommand(
        R->P[0].HostName,
        R->P[0].Port,
        R->P[0].Timeout * 1000000,
        R->AuthType,
        Command,
        &Response,
        NULL,
        &tmpSC) == FAILURE)
    {
    DBG(2,fWIKI) DPrint("ALERT:    cannot start job '%s' on %s RM on %d procs (command failure)\n",
      J->Name,
      MRMType[R->Type],
      tcount);

    if (SC != NULL)
      *SC = mscRemoteFailure;

    R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  if (tmpSC < 0)
    {
    DBG(2,fWIKI) DPrint("ALERT:    cannot start job '%s' on %s RM on %d procs (rm failure)\n",
      J->Name,
      MRMType[R->Type],
      tcount);

    R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  DBG(2,fWIKI) DPrint("INFO:     job '%s' started through %s RM on %d procs\n",
    J->Name,
    MRMType[R->Type],
    tcount);

  free(Response);

  return(SUCCESS);
  }  /* END MWikiJobStart() */





int MWikiJobCancel(

  mjob_t *J,        /* I */
  mrm_t  *R,        /* I */
  char   *Message,  /* I (opt) */
  char   *Msg,      /* O */
  int    *SC)       /* O */

  {
  char    Command[MAX_MLINE];

  int     tmpSC;
  char   *Response;

  char   CancelType[MAX_MNAME];

  const char *FName = "MWikiJobCancel";

  DBG(2,fWIKI) DPrint("%s(%s,%s,CMsg,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if ((J == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  /* fail in TEST mode */

  if ((MSched.Iteration == R->FailIteration) || (MSched.Mode == msmTest))
    {
    if (MSched.Mode == msmTest)
      {
      DBG(3,fWIKI) DPrint("INFO:     cannot cancel job '%s' (test mode)\n",
        J->Name);

      sleep(2);
      }
    else
      {
      DBG(3,fWIKI) DPrint("INFO:     cannot cancel job '%s' (fail iteration)\n",
        J->Name);
      }

    return(FAILURE);
    }

  if (strstr(Message,"wallclock") != NULL)
    strcpy(CancelType,"WALLCLOCK");
  else
    strcpy(CancelType,"ADMIN");

  sprintf(Command,"%s%s %s%s TYPE=%s",
    MCKeyword[mckCommand],
    WMCommandList[cmdWMCancelJob],
    MCKeyword[mckArgs],
    J->Name,
    CancelType);

  if (MWikiDoCommand(
        R->P[0].HostName,
        R->P[0].Port,
        R->P[0].Timeout * 1000000,
        R->AuthType,
        Command,
        &Response,
        NULL,
        &tmpSC) == FAILURE)
    {
    DBG(2,fWIKI) DPrint("ALERT:    cannot cancel job '%s' through %s RM\n",
      J->Name,
      MRMType[R->Type]);

    R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  if (tmpSC < 0)
    {
    DBG(2,fWIKI) DPrint("ALERT:    cannot cancel job '%s' through %s RM\n",
      J->Name,
      MRMType[R->Type]);

    R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  DBG(2,fWIKI) DPrint("INFO:     job '%s' cancelled through %s RM\n",
    J->Name,
    MRMType[R->Type]);

  free(Response);

  return(SUCCESS);
  }  /* END MWikiCancelJob() */





int MWikiJobSuspend(

  mjob_t *J,
  mrm_t  *R,
  char   *Msg,
  int    *SC)

  {
  char    Command[MAX_MLINE];

  char   *Response;

  int     tmpSC;

  const char *FName = "MWikiJobSuspend";

  DBG(2,fWIKI) DPrint("%s(%s,%s)\n",
    FName,
    J->Name,
    R->Name);

  sprintf(Command,"%s%s %s%s",
    MCKeyword[mckCommand],
    WMCommandList[cmdWMSuspendJob],
    MCKeyword[mckArgs],
    J->Name);

  if (MWikiDoCommand(
        R->P[0].HostName,
        R->P[0].Port,
        R->P[0].Timeout * 1000000,
        R->AuthType,
        Command,
        &Response,
        NULL,
        &tmpSC) == FAILURE)
    {
    DBG(2,fWIKI) DPrint("ALERT:    cannot suspend job '%s' through %s RM\n",
      J->Name,
      MRMType[R->Type]);

    return(FAILURE);
    }

  if (tmpSC < 0)
    {
    DBG(2,fWIKI) DPrint("ALERT:    cannot suspend job '%s' through %s RM\n",
      J->Name,
      MRMType[R->Type]);

    return(FAILURE);
    }

  DBG(2,fWIKI) DPrint("INFO:     job '%s' suspended through %s RM\n",
    J->Name,
    MRMType[R->Type]);

  free(Response);

  return(SUCCESS);
  }  /* END MWikiJobSuspend() */





int MWikiJobResume(

  mjob_t *J,
  mrm_t  *R,
  char   *Msg,
  int    *SC)

  {
  char    Command[MAX_MLINE];

  int     tmpSC;
  char   *Response;

  const char *FName = "MWikiJobResume";

  DBG(2,fWIKI) DPrint("%s(%s,%s)\n",
    FName,
    J->Name,
    R->Name);

  sprintf(Command,"%s%s %s%s",
    MCKeyword[mckCommand],
    WMCommandList[cmdWMResumeJob],
    MCKeyword[mckArgs],
    J->Name);

  if (MWikiDoCommand(
        R->P[0].HostName,
        R->P[0].Port,
        R->P[0].Timeout * 1000000,
        R->AuthType,
        Command,
        &Response,
        NULL,
        &tmpSC) == FAILURE)
    {
    DBG(2,fWIKI) DPrint("ALERT:    cannot resume job '%s' through %s RM\n",
      J->Name,
      MRMType[R->Type]);

    return(FAILURE);
    }

  if (tmpSC < 0)
    {
    DBG(2,fWIKI) DPrint("ALERT:    cannot resume job '%s' through %s RM\n",
      J->Name,
      MRMType[R->Type]);

    return(FAILURE);
    }

  DBG(2,fWIKI) DPrint("INFO:     job '%s' resumed through %s RM\n",
    J->Name,
    MRMType[R->Type]);

  free(Response);

  return(SUCCESS);
  }  /* END MWikiJobResume() */





int MWikiJobLoad(

  char   *JobName,   /* I */
  char   *AList,     /* I */
  mjob_t *J,         /* I (modified) */
  short  *TaskList,  /* O */
  mrm_t  *R)         /* I */

  {
  char    *ptr;
  char    *JobAttr;

  mreq_t  *RQ;

  int      rqindex;
  char     AccountName[MAX_MNAME];
  char    *TokPtr;

  const char *FName = "MWikiJobLoad";

  DBG(2,fWIKI) DPrint("%s(%s,%s,J,TaskList,%s)\n",
    FName,
    JobName,
    AList,
    (R != NULL) ? R->Name : "NULL");

  if ((J == NULL) || (R == NULL))
    {
    DBG(2,fWIKI) DPrint("ALERT:    invalid job/rm pointer specified\n");

    return(FAILURE);
    }

  MUStrCpy(J->Name,JobName,sizeof(J->Name));

  J->CTime = MSched.Time;
  J->MTime = MSched.Time;
  J->ATime = MSched.Time;

  J->RM    = &MRM[R->Index];

  /* NOTE:  if not specified, set wclimit to max allowed */

  J->SpecWCLimit[0] = 0;
  J->SpecWCLimit[1] = 0;

  if (MPar[0].UseMachineSpeed == FALSE)
    {
    J->WCLimit = J->SpecWCLimit[0];
    }

  /* add resource requirements information */

  if (J->Req[0] == NULL)
    {
    MReqCreate(J,NULL,&J->Req[0],FALSE);
    }

  RQ = J->Req[0];  /* convenience variable */

  /* set req defaults */

  MRMReqPreLoad(RQ);

  if (MGroupAdd("NOGROUP",&J->Cred.G) == FAILURE)
    {
    DBG(1,fWIKI) DPrint("ERROR:    cannot add default group for job %s\n",
      J->Name);
    }

  memset(RQ->ReqFBM,0,sizeof(RQ->ReqFBM));

  RQ->RequiredMemory = 0;
  RQ->RequiredSwap   = 0;
  RQ->RequiredDisk   = 0;
  RQ->DiskCmp        = mcmpGE;
  RQ->MemCmp         = mcmpGE;
  RQ->SwapCmp        = mcmpGE;

  RQ->TaskCount      = 1;

  RQ->Opsys    = 0;
  RQ->Arch     = 0;
  RQ->Network  = 0;

  if (TaskList != NULL)
    TaskList[0] = -1;

  RQ->RMIndex  = R->Index;

  if (X.XJobPreInit != (int (*)())0)
    {
    (*X.XJobPreInit)(X.xd,J);
    }

  ptr = AList;

  /* FORMAT:  <FIELD>=<VALUE>;[<FIELD>=<VALUE>;]... */

  JobAttr = MUStrTok(ptr,";\n",&TokPtr);

  while (JobAttr != NULL)
    {
    while (isspace(*JobAttr) && (*JobAttr != '\0'))
      JobAttr++;

    /* MUPurgeEscape(JobAttr); */

    if (!strncmp(JobAttr,"TASKLIST=",strlen("TASKLIST=")))
      {
      JobAttr += strlen("TASKLIST=");

      if (TaskList != NULL)
        MWikiAttrToTaskList(TaskList,JobAttr,&J->TaskCount); 
      }
    else
      {
      MWikiUpdateJobAttr(JobAttr,J);
      }

    JobAttr = MUStrTok(NULL,";\n",&TokPtr);
    }  /* END while (JobAttr != NULL) */

  /* authenticate job */

  if ((J->Cred.U == NULL) || (J->Cred.G == NULL))
    {
    DBG(1,fWIKI) DPrint("ERROR:    cannot authenticate job '%s' (no user/no group)\n",
      J->Name);

    return(FAILURE);
    }

  if (J->Cred.A != NULL)
    strcpy(AccountName,J->Cred.A->Name);
  else
    AccountName[0] = '\0';

  if (MJobSetCreds(
        J,
        J->Cred.U->Name,
        J->Cred.G->Name,
        AccountName) == FAILURE)
    {
    DBG(1,fWIKI) DPrint("ERROR:    cannot authenticate job '%s' (U: %s  G: %s  A: '%s')\n",
      J->Name,
      J->Cred.U->Name,
      J->Cred.G->Name,
      AccountName);

    return(FAILURE);
    }

  if (J->Request.TC == 0)
    {
    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      RQ = J->Req[rqindex];

      J->Request.TC += RQ->TaskCount;
      }  /* END for (rqindex) */

    if ((J->State == mjsStarting) || (J->State == mjsRunning))
      J->TaskCount = J->Request.TC;
 
    if (J->Request.TC == 0)
      {
      DBG(1,fWIKI) DPrint("ERROR:    no taskcount specified for job '%s'\n",
        J->Name);

      return(FAILURE);
      }
    }

  J->EState = J->State;

  J->SystemQueueTime = J->SubmitTime;

  if ((J->Cred.G == NULL) || !strcmp(J->Cred.G->Name,"NOGROUP"))
    {
    int   GID;

    char *ptr;

    if ((GID = MUGIDFromUID(J->Cred.U->OID)) > 0)
      {
      ptr = MUGIDToName(GID);

      if (strncmp(ptr,"GID",strlen("GID")))
        {
        MGroupAdd(ptr,&J->Cred.G);

        DBG(1,fWIKI) DPrint("INFO:     job '%s' assigned default group '%s'\n",
          J->Name,
          MUGIDToName(GID));
        }
      }
    }    /* END if ((J->Cred.G == NULL) || ... ) */

#ifdef __NCSA
  /* hack */

  if (J->Q->Flags & (1 << qfdedicated))
    {
    J->Req[0]->DRes.Procs = -1;
    }
#endif /* __NCSA */

  return(SUCCESS);
  }  /* END MWikiJobLoad() */





int MWikiAttrToTaskList(

  short *TaskList,    /* O */
  char  *AList,       /* I */
  int   *TaskCount)   /* O */

  {
  char *ptr;

  int   tindex;

  mnode_t *N;

  char   *TokPtr;

  const char *FName = "MWikiAttrToTaskList";

  DBG(5,fWIKI) DPrint("%s(TaskList,%s,TaskCount)\n",
    FName,
    AList);

  /* FORMAT:  <HOST>[:<HOST]... */

  ptr = MUStrTok(AList,":;",&TokPtr);

  tindex = 0;

  while (ptr != NULL)
    {
    if (MNodeFind(ptr,&N) == FAILURE)
      {
      DBG(1,fWIKI) DPrint("ALERT:    cannot locate host '%s' for hostlist\n",
        ptr);

      ptr = MUStrTok(NULL,":",&TokPtr);

      continue;
      }

    TaskList[tindex] = N->Index;

    tindex++;

    ptr = MUStrTok(NULL,":",&TokPtr);
    }  /* END while (ptr != NULL) */

  TaskList[tindex] = -1;

  if (TaskCount != NULL)
    *TaskCount = tindex;

  return(SUCCESS);
  }  /* END MWikiAttrToTaskList() */





int MWikiUpdateJob(

  char     AList[],
  mjob_t  *J,
  int      RMIndex)

  {
  char *ptr;
  char *tail;

  short  TaskList[MAX_MTASK_PER_JOB];

  char  JobAttr[MAX_MBUFFER];

  int   OldState;

  const char *FName = "MWikiUpdateJob";

  DBG(2,fWIKI) DPrint("%s(AList,%s,%d)\n",
    FName,
    J->Name,
    RMIndex);

  J->ATime = MSched.Time;

  OldState = J->State;

  ptr = AList;

  TaskList[0] = -1;

  /* FORMAT:  <FIELD>=<VALUE>;[<FIELD>=<VALUE>;]... */

  while ((tail = MUStrChr(ptr,';')) != NULL)
    {
    MUStrCpy(JobAttr,ptr,MIN((long)sizeof(JobAttr),(tail - ptr + 1)));

    MUPurgeEscape(JobAttr);

    if (!strncmp(JobAttr,"TASKLIST=",strlen("TASKLIST=")))
      {
      MWikiAttrToTaskList(
        TaskList,
        &JobAttr[strlen("TASKLIST=")],
        &J->TaskCount);
      }
    else
      {
      MWikiUpdateJobAttr(JobAttr,J);
      }

    ptr = tail + 1;
    }  /* END while() */

  MRMJobPostUpdate(J,TaskList,OldState,&MRM[RMIndex]);

  return(SUCCESS);
  }  /* END MWikiUpdateJob() */




 
int MWikiNodeLoad(

  char        *AString,
  mnode_t     *N,
  mrm_t       *R)

  {
  char *ptr;
  char *tail;

  char  NodeAttr[MAX_MBUFFER];

  time_t tmpTime;

  const char *FName = "MWikiNodeLoad";

  DBG(2,fWIKI) DPrint("%s(AString,%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if (N == NULL)
    {
    return(FAILURE);
    }

  time(&tmpTime);

  N->CTime = (long)tmpTime;
  N->MTime = (long)tmpTime;
  N->ATime = (long)tmpTime;

  N->RM = R;

  N->TaskCount = 0;

  ptr = AString;

  /* FORMAT:  <FIELD>=<VALUE>;[<FIELD>=<VALUE>;]... */

  while (ptr[0] != '\0')
    {
    if ((tail = MUStrChr(ptr,';')) != NULL)
      {
      strncpy(NodeAttr,ptr,MIN(MAX_MBUFFER - 1,tail - ptr));
      NodeAttr[tail - ptr] = '\0';
      }
    else
      {
      MUStrCpy(NodeAttr,ptr,MAX_MBUFFER);
      }

    MUPurgeEscape(NodeAttr);

    MWikiNodeUpdateAttr(NodeAttr,N);

    if (tail == NULL)
      break;

    ptr = tail + 1;
    }  /* END while ((tail = MUStrChr(ptr,';')) != NULL) */

  N->EState       = N->State;

  N->SyncDeadLine = MAX_MTIME;
  N->StateMTime   = MSched.Time;

  N->R[0]         = NULL;

  N->STTime       = 0;
  N->SUTime       = 0;
  N->SATime       = 0;

  if ((N->State != mnsActive) && 
      (N->State != mnsIdle) && 
      (N->State != mnsUnknown))
    {
    N->ARes.Procs = 0;
    }

  DBG(6,fWIKI) DPrint("INFO:     MNode[%03d] '%18s' %9s VM: %8d Mem: %5d Dk: %5d Cl: %s %s\n",
    N->Index,
    N->Name,
    MAList[eNodeState][N->State],
    N->CRes.Swap,
    N->CRes.Mem,
    N->CRes.Disk,
    MUCAListToString(N->CRes.PSlot,NULL,NULL),
    MUMAList(eFeature,N->FBM,sizeof(N->FBM)));

  return(SUCCESS);
  }  /* END MWikiNodeLoad() */





int MWikiNodeUpdate(

  char    *AList,  /* I */
  mnode_t *N)      /* I (modified) */

  {
  char  NodeAttr[MAX_MBUFFER];
  char *ptr;
  char *tail;

  const char *FName = "MWikiNodeUpdate";

  DBG(2,fWIKI) DPrint("%s(AList,%s)\n",
    FName,
    N->Name);

  ptr = AList;

  /* FORMAT:  <FIELD>=<VALUE>[:<FIELD>=<VALUE>]... */
 
  while ((tail = MUStrChr(ptr,';')) != NULL)
    {
    strncpy(NodeAttr,ptr,tail - ptr);
    NodeAttr[tail - ptr] ='\0';

    MUPurgeEscape(NodeAttr);

    MWikiNodeUpdateAttr(NodeAttr,N);

    ptr = tail + 1;
    } 

  return(SUCCESS);
  }  /* END MWikiNodeUpdate() */




int MWikiNodeUpdateAttr(

  char    *Tok,  /* I */
  mnode_t *N)    /* I (modified) */

  {
  char *ptr;
  char *Value;
  char *tail;

  char  Line[MAX_MLINE];

  int   aindex;

  char *TokPtr;

  int   index;

  mpar_t  *P;

  const char *FName = "MWikiNodeUpdateAttr";

  DBG(6,fWIKI) DPrint("%s(%s,%s)\n",
    FName,
    Tok,
    N->Name);

  if ((ptr = MUStrChr(Tok,'=')) == NULL)
    {
    return(FAILURE);
    }

  if ((Tok[0] == 'A') && (isdigit(Tok[1])))
    {
    aindex = (int)strtol(&Tok[1],NULL,0);

    if (aindex > MAX_WIKINODEATTR)
      aindex = MAX_WIKINODEATTR;
    }
  else
    {
    for (aindex = 0;MWikiNodeAttr[aindex] != NULL;aindex++)
      {
      if (!strncmp(MWikiNodeAttr[aindex],Tok,strlen(MWikiNodeAttr[aindex])))
        break;
      }  /* END for (aindex) */
    }

  if (MWikiNodeAttr[aindex] == NULL)
    {
    return(FAILURE);
    }

  ptr++;

  while(!isalnum(*ptr))
    ptr++;

  Value = ptr;

  switch(aindex)
    {
    case mwnaUpdateTime:

      N->ATime = strtol(Value,NULL,0);

      break;

    case mwnaState:

      if (!strcmp(Value,"Draining"))
        N->State = MUMAGetIndex(eNodeState,"Drain",mAdd);
      else
        N->State = MUMAGetIndex(eNodeState,Value,mAdd);

      break;

    case mwnaOS:

      N->ActiveOS = MUMAGetIndex(eOpsys,Value,mAdd);

      break;

    case mwnaArch:

      N->Arch = MUMAGetIndex(eArch,Value,mAdd);

      break;

    case mwnaCMemory:

      N->CRes.Mem = (int)strtol(Value,NULL,0);

      break;

    case mwnaAMemory:

      N->ARes.Mem = (int)strtol(Value,NULL,0);

      break;

    case mwnaCSwap:

      N->CRes.Swap = (int)strtol(Value,NULL,0);

      break;

    case mwnaASwap:

      N->ARes.Swap = (int)strtol(Value,NULL,0);

      break;

    case mwnaCDisk:

      N->CRes.Disk = (int)strtol(Value,NULL,0);

      break;

    case mwnaADisk:

      N->ARes.Disk = (int)strtol(Value,NULL,0);

      break;

    case mwnaCProc:

      N->CRes.Procs = (int)strtol(Value,NULL,0);

      break;

    case mwnaAProc:

      N->ARes.Procs = (int)strtol(Value,NULL,0);

      break;

    case mwnaCNet:

      N->Network = MUMAFromString(eNetwork,Value,mAdd);

      break;

    case mwnaANet:

      N->Network = MUMAFromString(eNetwork,Value,mAdd);

      break;

    case mwnaCRes:

      /* break line into <NAME>,<VALUE> pairs */
      /* VALUE FORMAT:  <VALNAME>[:<COUNT>]   */

      strcpy(Line,Value);

      ptr = MUStrTok(Line,",",&TokPtr);

      while (ptr != NULL)
        {
        if ((tail = strchr(ptr,':')) != NULL)
          {
          *tail = '\0';

          index = MUMAGetIndex(eGRes,ptr,mAdd);

          N->CRes.GRes[index].count = (int)strtol(tail + 1,NULL,0);
          }
        else
          {
          index = MUMAGetIndex(eGRes,ptr,mAdd);             
       
          N->CRes.GRes[index].count = 1;
          }
 
        ptr = MUStrTok(NULL,",",&TokPtr);
        }  /* END while (ptr != NULL) */

      break;

    case mwnaARes:

      /* break line into <NAME>,<VALUE> pairs */

      strcpy(Line,Value);

      ptr = MUStrTok(Line,",",&TokPtr);

      while (ptr != NULL)
        {
        if ((tail = strchr(ptr,':')) != NULL)
          {
          *tail = '\0';
 
          index = MUMAGetIndex(eGRes,ptr,mAdd);
 
          N->ARes.GRes[index].count = (int)strtol(tail + 1,NULL,0);
          }
        else
          {
          index = MUMAGetIndex(eGRes,ptr,mAdd);
 
          N->ARes.GRes[index].count = 1;
          }
 
        ptr = MUStrTok(NULL,",",&TokPtr);
        }  /* END while (ptr != NULL) */          

      break;

    case mwnaCPULoad:

      N->Load = (double)atof(Value);

      break;
 
    case mwnaCClass:

      N->CRes.PSlot[0].count = 0;

      if (strcmp(Value,NONE))
        {
        int cindex;

        MUNumListFromString(N->CRes.PSlot,Value,eClass);

        for (cindex = 1;cindex < MAX_MCLASS;cindex++)
          {
          if (N->CRes.PSlot[cindex].count <= 0)
            continue;

          DBG(6,fWIKI) DPrint("INFO:     configured class '%s'(x%d) added (%s)\n",
            MAList[eClass][cindex],
            N->CRes.PSlot[cindex].count,
            MUCAListToString(N->CRes.PSlot,NULL,NULL));
          }
        }  /* END if (strcmp(Value,NONE)) */

      break;

    case mwnaAClass:

      N->ARes.PSlot[0].count = 0;

      if (strcmp(Value,NONE))
        {
        int cindex;

        MUNumListFromString(N->ARes.PSlot,Value,eClass);

        for (cindex = 1;cindex < MAX_MCLASS;cindex++)
          {
          if (N->ARes.PSlot[cindex].count <= 0)
            continue;

          DBG(6,fWIKI) DPrint("INFO:     available class '%s'(x%d) added (%s)\n",
            MAList[eClass][cindex],
            N->ARes.PSlot[cindex].count,
            MUCAListToString(N->ARes.PSlot,NULL,NULL));
          }  /* END for (cindex) */
        }    /* END if (strcmp(Value,NONE)) */

      break;

    case mwnaFeature:

      /* NOTE:  must incorporate MNodeProcessFeature(N,NFeature[index]); */

      MUMAMAttrFromLine(eFeature,Value,mAdd,N->FBM,sizeof(N->FBM));

      break;

    case mwnaPartition:

      if (MParAdd(Value,&P) == SUCCESS)
        {
        MParAddNode(P,N);
        }

      break;

    case mwnaEvent:

      /* NOT SUPPORTED */

      break;

    case mwnaMaxTask:

      /* NOT IMPLEMENTED */

      break;

    case mwnaSpeed:

      N->Speed = atof(Value);

      break;
 
    default:

      /* unexpected node attribute */

      /* NO-OP */

      break;
    }  /* END switch(aindex) */

  return(SUCCESS);
  }  /* END MWikiNodeUpdateAttr() */




int MWikiUpdateJobAttr(

  char   *Tok,  /* I */
  mjob_t *J)    /* I (modified) */

  {
  char  *ptr;

  char  *Value;
  char  *tok;
  char  *tail;

  mreq_t  *RQ;
  mnode_t *N;
  mqos_t  *QDef;

  int    nindex;
  int    aindex;
  int    rindex;

  int    CIndex;
  int    Count;

  char   HostLine[MAX_MBUFFER];
  char   HostName[MAX_MNAME];
  char   ResLine[MAX_MLINE];

  char  *TokPtr;

  int    tmpI;

  const char *FName = "MWikiUpdateJobAttr";

  DBG(6,fWIKI) DPrint("%s(%s,%s)\n",
    FName,
    Tok,
    J->Name);

  if ((ptr = MUStrChr(Tok,'=')) == NULL)
    {
    return(FAILURE);
    }

  RQ = J->Req[0]; /* FIXME */

  if (*ptr == '\0')
    {
    return(FAILURE);
    }

  if ((Tok[0] == 'A') && (isdigit(Tok[1])))
    {
    aindex = (int)strtol(&Tok[1],NULL,0);

    if (aindex > MAX_WIKIJOBATTR)
      aindex = MAX_WIKIJOBATTR;
    }
  else
    { 
    aindex = MUGetIndex(Tok,MWikiJobAttr,MBNOTSET,0);
    }

  if (aindex == 0)
    {
    if (X.XJobProcessWikiAttr != (int (*)())0) 
      {
      return((*X.XJobProcessWikiAttr)(X.xd,J,Tok)); 
      }

    DBG(1,fWIKI) DPrint("INFO:     unexpected WIKI job attr '%s' detected\n",
      Tok);

    return(FAILURE);
    }

  ptr++;

  while(isspace(*ptr) || (*ptr == '='))
    ptr++;

  Value = ptr;

 if (Value[0] == '\0')
  {
  return(SUCCESS);
  }

  switch(aindex)
    {
    case mwjaName:

      strcpy(J->Name,Value);

      break;

    case mwjaUpdateTime:

      J->MTime = strtol(Value,NULL,0);
      J->ATime = J->MTime;

      break;

    case mwjaState:

      if (strcmp(MJobState[J->State],Value))
        {
        DBG(1,fWIKI) DPrint("INFO:     job '%s' changed states from %s to %s\n",
          J->Name,
          MAList[eJobState][J->State],
          Value);

        J->MTime = MSched.Time;
        }

      J->State = MUMAGetIndex(eJobState,Value,mAdd);

      break;

    case mwjaWCLimit:

      J->SpecWCLimit[1] = strtol(Value,NULL,0);
      J->SpecWCLimit[0] = J->SpecWCLimit[1];
     
      if (MPar[0].UseMachineSpeed == FALSE)
        {
        J->WCLimit = J->SpecWCLimit[0];
        }

      break;

    case mwjaTasks:

      tmpI = (int)strtol(Value,NULL,0);
     
      J->Request.TC = tmpI;
      RQ->TaskCount     = tmpI;

      RQ->TaskRequestList[0] = tmpI;
      RQ->TaskRequestList[1] = tmpI;      
      RQ->TaskRequestList[2] = 0;

      break;

    case mwjaNodes:

      tmpI = (int)strtol(Value,NULL,0);

      J->Request.NC = tmpI;
      RQ->NodeCount     = tmpI;

      break;

    case mwjaGeometry:

      /* NOT IMPLEMENTED */
 
      break;

    case mwjaQueueTime:

      J->SubmitTime = strtol(Value,NULL,10);

      if (J->SubmitTime > MSched.Time)
        {
        DBG(2,fWIKI) DPrint("WARNING:  clock skew detected (queue time for job %s in %s)\n",
          J->Name,
          MULToTString(J->SubmitTime - MSched.Time));

        J->SubmitTime = MSched.Time;
        }

      break;

    case mwjaStartDate:

      {
      long tmpL;

      tmpL = strtol(Value,NULL,10);

      MJobSetAttr(J,mjaReqSMinTime,(void **)&tmpL,mdfLong,mSet);
      }  /* END BLOCK */

      break;

    case mwjaEndDate:

      {
      long tmpL;

      tmpL = strtol(Value,NULL,10);

      MJobSetAttr(J,mjaReqCMaxTime,(void **)&tmpL,mdfLong,mSet);
      }  /* END BLOCK */
 
      break;

    case mwjaStartTime:

      J->StartTime = strtol(Value,NULL,10);

      if (J->StartTime > MSched.Time)
        {
        DBG(2,fWIKI) DPrint("WARNING:  clock skew detected (start time for job %s in %s)\n",
          J->Name,
          MULToTString(J->StartTime - MSched.Time));

        J->StartTime = MSched.Time;
        }

      J->DispatchTime = J->StartTime;

      break;

    case mwjaCompletionTime:

      J->CompletionTime = strtol(Value,NULL,10);

      if (J->CompletionTime > MSched.Time)
        {
        DBG(2,fWIKI) DPrint("WARNING:  clock skew detected (completion time for job %s in %s)\n",
          J->Name,
          MULToTString(J->CompletionTime - MSched.Time));

        J->CompletionTime = MSched.Time;
        }

      break;

    case mwjaUName:

      if (MUserAdd(Value,&J->Cred.U) == FAILURE)
        {
        DBG(1,fWIKI) DPrint("ERROR:    cannot add user for job %s (UserName: %s)\n",
          J->Name,
          Value);
        }
  
      break;

    case mwjaGName:

      if (MGroupAdd(Value,&J->Cred.G) == FAILURE)
        {
        DBG(1,fWIKI) DPrint("ERROR:    cannot add group for job %s (GroupName: %s)\n",
          J->Name,
          Value);
        }

      break;

    case mwjaAccount:

      if (MAcctAdd(Value,&J->Cred.A) == FAILURE)
        {
        DBG(1,fWIKI) DPrint("ERROR:    cannot add account for job %s (Account: %s)\n",
          J->Name,
          Value);
        }

      break;

    case mwjaRFeatures:

      MUMAMAttrFromLine(eFeature,Value,mAdd,RQ->ReqFBM,sizeof(RQ->ReqFBM));

      break;

    case mwjaRNetwork:

      RQ->Network = MUMAGetIndex(eNetwork,Value,mAdd);

      break;

    case mwjaDNetwork:

      /* NOT IMPLEMENTED */

      break;

    case mwjaRClass:

      memset(RQ->DRes.PSlot,0,sizeof(RQ->DRes.PSlot));

      if (strstr(Value,NONE) == NULL)
        {
        /* if Class != '[NONE]' */

        tok = MUStrTok(Value,"[]",&TokPtr);

        do
          {
          if ((tail = strchr(tok,':')) != NULL)
            {
            *tail = '\0';

            Count = (int)strtol(tail + 1,NULL,0);
            }
          else
            {
            Count = 1;
            }

          if ((CIndex = MUMAGetIndex(eClass,tok,mAdd)) == FAILURE)
            {
            DBG(2,fWIKI) DPrint("WARNING:  cannot add class '%s' for job %s\n",
              tok,
              J->Name);

            return(FAILURE);
            }

          RQ->DRes.PSlot[CIndex].count = Count;
          RQ->DRes.PSlot[0].count      += Count;
          }
        while ((tok = MUStrTok(NULL,"[]",&TokPtr)) != NULL);
        }  /* END if (strstr(Value,NONE) == NULL) */

      DBG(1,fWIKI) DPrint("INFO:    classes loaded for job %s : %s\n",
        J->Name,
        MUCAListToString(RQ->DRes.PSlot,NULL,NULL));

      break;

    case mwjaROpsys:

      RQ->Opsys = MUMAGetIndex(eOpsys,Value,mAdd);

      break;

    case mwjaRArch:

      RQ->Arch = MUMAGetIndex(eArch,Value,mAdd);

      break;

    case mwjaRMem:

      RQ->RequiredMemory = (int)strtol(Value,NULL,0);

      break;

    case mwjaRMemCmp:

      RQ->MemCmp = MUCmpFromString(Value,NULL);

      break;

    case mwjaDMem:

      RQ->DRes.Mem = (int)strtol(Value,NULL,0);

      break;

    case mwjaRDisk:

      RQ->RequiredDisk = (int)strtol(Value,NULL,0);

      break;

    case mwjaRDiskCmp:

      RQ->DiskCmp = MUCmpFromString(Value,NULL);

      break;

    case mwjaDDisk:

      RQ->DRes.Disk = (int)strtol(Value,NULL,0); 

      break;

    case mwjaRSwap:

      RQ->RequiredSwap = (int)strtol(Value,NULL,0);

      break;

    case mwjaRSwapCmp:

      RQ->SwapCmp = MUCmpFromString(Value,NULL);

      break;

    case mwjaDSwap:

      RQ->DRes.Swap = (int)strtol(Value,NULL,0);

      break;

    case mwjaPartitionList:

      {
      int tmpI;

      tmpI = MUMAFromString(ePartition,Value,mAdd);

      memcpy(&J->SpecPAL[0],&tmpI,sizeof(J->SpecPAL[0]));
      }  /* END BLOCK */

      break;

    case mwjaExec:

      MUStrDup(&J->E.Cmd,Value);
 
      break;      

    case mwjaArgs:

      MUStrDup(&J->E.Args,Value);
 
      break;      

    case mwjaIWD:
 
      MUStrDup(&J->E.IWD,Value);

      break;

    case mwjaComment:

      /* NOT IMPLEMENTED */

      break;

    case mwjaRejectionCount:

      /* NYI */

      break;

    case mwjaRejectionMessage:

      /* NYI */

      break;

    case mwjaRejectionCode:

      /* NYI */

      break;

    case mwjaEvent:

      /* NYI */

      break;

    case mwjaTaskPerNode:

      J->Req[0]->TasksPerNode = (int)strtol(Value,NULL,0);

      break;

    case mwjaQOS:

      MQOSFind(Value,&J->QReq);

      if ((MQOSGetAccess(J,J->QReq,NULL,&QDef) == FAILURE) ||
          (J->QReq == NULL))
        {
        MJobSetQOS(J,QDef,0);
        }
      else
        {
        MJobSetQOS(J,J->QReq,0);
        }

      break;

    case mwjaDProcs:

      RQ->DRes.Procs = (int)strtol(Value,NULL,0);

      break;

    case mwjaHostList:

      J->SpecFlags |= (1 << mjfHostList);

      strcpy(HostLine,Value);

      ptr = MUStrTok(HostLine,":",&TokPtr);

      nindex = 0;

      if (J->ReqHList == NULL)
        {
        J->ReqHList = (mnalloc_t *)calloc(
          1,
          sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1));
        }

      J->ReqHList[0].N = NULL;

      while (ptr != NULL)
        {
        if (MNodeFind(ptr,&N) == FAILURE)
          {
          /* try full hostname */

          sprintf(HostName,"%s.%s",
            ptr,
            MSched.DefaultDomain);

          if (MNodeFind(HostName,&N) != SUCCESS)
            {
            DBG(1,fWIKI) DPrint("ALERT:    cannot locate node '%s' for job '%s' hostlist\n",
              ptr,
              J->Name);

            N = NULL;
            }
          }

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

      break;

    case mwjaSuspendTime:

      /* NYI */

      break;

    case mwjaResAccess:

      MUStrCpy(ResLine,Value,sizeof(ResLine));

      if ((ptr = MUStrTok(ResLine,":",&TokPtr)) == NULL)
        {
        break;
        }

      if (strstr(Value,"FREE") == NULL)
        {
        /* 'free' resources not available */

        MUStrCpy(J->ResName,ptr,sizeof(J->ResName));

        J->SpecFlags |= (1 << mjfAdvReservation);
        J->SpecFlags |= (1 << mjfByName);
        }

      rindex = 0;

      while (ptr != NULL)
        {
        if (strcmp(ptr,"FREE"))
          {
          MUStrCpy(J->RAList[rindex],ptr,sizeof(J->RAList[0]));

          DBG(4,fWIKI) DPrint("INFO:     job %s granted access to reservation %s\n",
            J->Name,
            J->RAList[rindex]);

          if (rindex == 3)
            break;
          else
            rindex++;
          }

        ptr = MUStrTok(NULL,":",&TokPtr);
        }  /* END while (ptr != NULL) */

      J->RAList[rindex][0] = '\0';
      
      break;

    case mwjaEnv:

      {
      /* must 'unpack' env value */

      MUStringUnpack(Value,Value,MAX_MLINE);

      MUStrDup(&J->E.Env,Value);
      }  /* END BLOCK */

      break;          

    case mwjaInput:

      MUStrDup(&J->E.Input,Value);
 
      break;          

    case mwjaOutput:

      MUStrDup(&J->E.Output,Value);
 
      break;          

    case mwjaError:

      MUStrDup(&J->E.Error,Value);
 
      break;          

    case mwjaSubmitString:

      {
      char tmpLine[MAX_MLINE];

      /* NOTE;  unpack string */

      MUStringUnpack(Value,tmpLine,MAX_MLINE);

      MUStrDup(&J->RMSubmitString,tmpLine);
      }  /* END BLOCK */

      break;

    case mwjaSubmitType:

      J->RMSubmitType = MUGetIndex(Value,MRMType,FALSE,0);

      break;

    default:

      DBG(2,fWIKI) DPrint("INFO:     WIKI keyword '%s'(%d) not handled\n",
        MWikiJobAttr[aindex],
        aindex);
           
      break;
    }  /* END switch (aindex) */

  return(SUCCESS);
  }  /* END MWikiUpdateJobAttr() */





int MWikiGetAttr(

  int    OIndex,
  char  *Name,
  int   *Status,
  char  *Attr,
  char **Start)

  {
  char *ptr;
  char *attr;
  char *tail;

  char  State[MAX_MNAME];

  const char *FName = "MWikiGetAttr";
 
  DBG(4,fWIKI) DPrint("%s(%s,Name,Status,Attr,Start)\n",
    FName,
    MXO[OIndex]);
 
  /* FORMAT:  ID:<FIELD>=<VALUE>;<FIELD>=<VALUE>...{#|'\0'} */

  /* locate ID delimiter */

  if ((ptr = MUStrChr(*Start,':')) == NULL)
    {
    return(FAILURE);
    }

  MUStrCpy(Name,*Start,MIN(ptr - *Start + 1,MAX_MNAME));

  attr = ptr + 1;

  if ((ptr = MUStrChr(attr,'#')) == NULL)
    {
    strcpy(Attr,attr);

    if (Start != NULL)
      *Start = *Start + strlen(*Start);
    }
  else
    {
    MUStrCpy(Attr,attr,MIN(ptr - attr + 1,MAX_MBUFFER));

    if (Start != NULL)
      *Start = ptr + 1;
    }

  switch(OIndex)
    {
    case mxoNode:

      if ((ptr = strstr(Attr,MWikiNodeAttr[mwnaState])) == NULL)
        {
        *Status = mnsNone;
        }
      else
        {
        ptr += strlen(MWikiNodeAttr[mwnaState]);

        while(!isalnum(*ptr))
          ptr++;

        if ((tail = MUStrChr(ptr,';')) != NULL)
          strncpy(State,ptr,tail - ptr);
        else
          strcpy(State,ptr);

        State[tail - ptr] = '\0';

        *Status = MUMAGetIndex(eNodeState,State,mAdd);
        }

      break;

    case mxoJob:

      if ((ptr = strstr(Attr,MWikiJobAttr[mwjaState])) == NULL)
        {
        *Status = mjsNONE;
        }
      else
        {
        ptr += strlen(MWikiJobAttr[mwjaState]);

        while(!isalnum(*ptr))
          ptr++;

        if ((tail = MUStrChr(ptr,';')) != NULL)
          MUStrCpy(State,ptr,MIN(tail - ptr + 1,MAX_MNAME));
        else
          MUStrCpy(State,ptr,MAX_MNAME);

        *Status = MUMAGetIndex(eJobState,State,mAdd);
        }

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(OIndex) */

  return(SUCCESS);
  }  /* END MWikiGetAttr() */





int MWikiDoCommand(

  char  *HostName,
  int    Port,
  long   TO,
  int    Auth,
  char  *Command,
  char **Data,
  long  *DataSize,
  int   *SC)

  {
  msocket_t S;

  char *ptr;

  int   UseCheckSum;

  char *Response;

  const char *FName = "MWikiDoCommand";

  DBG(2,fSCHED) DPrint("%s(%s,%d,%ld,%s,%s,Data,DataSize,SC)\n",
    FName,
    HostName,
    Port,
    TO,
    MRMAuthType[Auth],
    Command);

  if (SC != NULL)
    *SC = -1;

  MSUInitialize(&S,HostName,Port,TO,(1 << TCP));       

  if (Auth == rmaCheckSum)
    UseCheckSum = TRUE;
  else
    UseCheckSum = FALSE;

  if (HostName == NULL)
    {
    /* socket is already open */

    S.sd = Port;  /* HACK:  sd routed in via Port arg */
    }
  else
    {
    if (MSUConnect(&S,FALSE,NULL) == FAILURE)
      {
      DBG(0,fSOCK) DPrint("ERROR:    cannot connect to server %s:%d\n",
        S.RemoteHost,
        S.RemotePort);

      return(FAILURE);
      }
    }

  S.SBuffer      = Command;
  S.SBufSize     = strlen(Command);

  strcpy(S.CSKey,MSched.DefaultCSKey);
  S.CSAlgo       = MSched.DefaultCSAlgo;

  if (MSUSendData(&S,TO,UseCheckSum,FALSE) == FAILURE)
    {
    DBG(0,fSOCK) DPrint("ERROR:    cannot send data to server %s:%d\n",
      HostName,
      Port);

    MSUDisconnect(&S);

    return(FAILURE);
    }
  else
    {
    DBG(1,fRM) DPrint("INFO:     command sent to server\n");

    DBG(3,fRM) DPrint("INFO:     message sent: '%s'\n",
      Command);
    }

  if (Data == NULL)
    {
    /* single sided communication */

    MSUDisconnect(&S);

    return(SUCCESS);
    }

  if (MSURecvData(&S,TO,UseCheckSum,NULL,NULL) == FAILURE)
    {
    DBG(0,fSOCK) DPrint("ERROR:    cannot receive data from server %s:%d\n",
      HostName,
      Port);

    MSUDisconnect(&S);

    return(FAILURE);
    }

  DBG(4,fUI) DPrint("INFO:     received message '%s' from wiki server\n",
    S.RBuffer);

  MSUDisconnect(&S);

  if ((ptr = strstr(S.RBuffer,RSPMARKER)) != NULL)
    {
    ptr += strlen(RSPMARKER);

    Response = ptr;
    }
  else
    {
    Response = NULL;
    }

  if ((ptr = strstr(S.RBuffer,MCKeyword[mckStatusCode])) != NULL)
    {
    ptr += strlen(MCKeyword[mckStatusCode]);

    sscanf(ptr,"%d",
      SC);

    if (*SC < 0)
      {
      DBG(0,fSOCK) DPrint("ERROR:    command '%s'  SC: %d  response: '%s'\n",
        Command,
        *SC,
        (Response == NULL) ? "NONE" : Response);

      MSUFree(&S);

      return(FAILURE);
      }
    }
  else
    {
    DBG(0,fSOCK) DPrint("ERROR:    cannot determine statuscode for command '%s'  SC: %d  response: '%s'\n",
      Command,
      *SC,
      (Response == NULL) ? "NONE" : Response);

    MSUFree(&S);

    return(FAILURE);
    }

  *Data = S.RBuffer;

  if (DataSize != NULL)
    *DataSize = S.RBufSize;

  return(SUCCESS);
  }  /* END MWikiDoCommand() */




int MWikiTestNode(

  char *TString)

  {
  mnode_t *N = NULL;

  if (MNodeCreate(&N) == FAILURE)
    {
    exit(1);
    }

  if (MWikiNodeLoad(TString,N,&MRM[0]) == SUCCESS)
    {
    /* test succeeded */

    exit(0);
    }

  exit(1);

  /*NOTREACHED*/

  return(SUCCESS);
  }  /* END MWikiTestNode() */




int MWikiTestJob(

  char *TString)

  {
  short tmpTaskList[MAX_MNODE];

  mjob_t *J = NULL;

  if (MJobCreate("testjob",TRUE,&J) == FAILURE)
    {
    exit(1);
    }

  if (MWikiJobLoad("testjob",TString,J,tmpTaskList,&MRM[0]) == SUCCESS)
    {
    /* test succeeded */

    exit(0);
    }

  exit(1);

  /*NOTREACHED*/

  return(SUCCESS);
  }  /* END MWikiTestJob() */


/* END MWikiI.c */


