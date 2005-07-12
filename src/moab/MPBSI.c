/* HEADER */
        
/* Contains:                                    *
 *   int MPBSInitialize(R,SC)                   *
 *   int MPBSWorkloadQuery(R,JCount,SC)         * 
 *   int __MPBSJobGetState(Name,Status,PJob,FlagsP) *
 *   int MPBSClusterQuery(R,RCount,SC)          *
 *   int __MPBSGetNodeState(Name,State,PNode)   *
 *   int MPBSJobStart(J,R,Msg,SC)               *
 *   int MPBSCancelJob(J,Message,R)             *
 *   int MPBSJobMigrate(J,R,NL,Msg,SC)          *
 *   int MPBSJobSubmit(String,R,J,JobName,Msg,SC) *
 *   int MPBSNodeLoad(N,PNode,State,RMIndex)    *
 *   int MPBSQueryMOM(N,R,Msg,SC)               *
 *   int MPBSNodeUpdate(PNode,N,State,RMIndex)  *
 *   int MPBSJobLoad(JobName,PJob,J,TaskList,RMIndex) *
 *   int MPBSJobUpdate(PJob,J,TaskList,RMIndex) *
 *   int MPBSGetClassInfo(N,CClass,AClass)      *
 *   int __MPBSGetTaskList(J,TaskString,TaskList,IsExecList) *
 *   int __MPBSNLToTaskString(mnalloc_t *,mrm_t *,char *,int);
 *   int __MPBSJobChkExecutable(struct batch_status *);
 *   int MPBSLoadQueueInfo(R,SpecN,LoadFull,SC) *
 *                                              */

#ifdef __MPBS21
# define ND_busy       "busy"
# define ATTR_NODE_np  "np"
char *getreq();
#endif /* __MPBS21 */

#include "moab.h"
#include "msched-proto.h"

extern mlog_t    mlog;

extern msched_t  MSched;
extern mclass_t  MClass[];
extern mjob_t   *MJob[];
extern mrm_t     MRM[];    
extern msim_t    MSim;
extern mpar_t    MPar[];
extern mx_t      X;

extern const char *MNodeState[];

#ifndef __MPBS
#include "__MPBSStub.c"
#endif /* __MPBS */

#ifdef __MPBS

#include "pbs_error.h"
#include "pbs_ifl.h"

#ifndef getreq
char *getreq(int);
#endif /* getreq */

extern int pbs_errno; 

extern int get_svrport(const char *,char *,int);
extern int openrm(char *,int);
extern int addreq(int,char *);
extern int closerm(int);
extern int pbs_stagein(int,char *,char *,char *);
extern int pbs_stageout(int,char *,char *,char *);       

/* PBS prototypes */

int MPBSInitialize(mrm_t *,int *SC);
int MPBSProcessEvent(mrm_t *,int *);
int MPBSWorkloadQuery(mrm_t *,int *,int *);
int MPBSClusterQuery(mrm_t *,int *,char *,int *);
int MPBSLoadQueueInfo(mrm_t *,mnode_t *,mbool_t,int *);
int MPBSJobSubmit(char *,mrm_t *,mjob_t **,char *,char *,int *);
int MPBSJobSetAttr(mjob_t *,void *,char *,tpbsa_t *,short *,int);
int MPBSNodeSetAttr(mnode_t *,void *,int);
int MPBSJobAdjustResources(mjob_t *,tpbsa_t *,mrm_t *);
int MPBSJobStart(mjob_t *,mrm_t *,char *,int *);
int MPBSJobRequeue(mjob_t *,mrm_t *,mjob_t **,char *,int *);
int MPBSJobMigrate(mjob_t *,mrm_t *,mnalloc_t *,char *,int *);
int MPBSJobModify(mjob_t *,mrm_t *,char *,char *,char *,char *,int *);
int MPBSJobCancel(mjob_t *,mrm_t *,char *,char *,int *);
int MPBSJobCkpt(mjob_t *,mrm_t *,mbool_t,char *,int *);
int MPBSJobSuspend(mjob_t *,mrm_t *,char *,int *);
int MPBSJobResume(mjob_t *,mrm_t *,char *,int *);
int MPBSJobLoad(char *,struct batch_status *,mjob_t *,short *,int);
int __MPBSSystemQuery(mrm_t *,int *);
int MPBSJobUpdate(struct batch_status *,mjob_t *,short *,int); 
int MPBSNodeLoad(mnode_t *,struct batch_status *,int,mrm_t *); 
int MPBSNodeUpdate(mnode_t *,struct batch_status *,enum MNodeStateEnum,mrm_t *);
int __MPBSJobGetState(struct batch_status *,mrm_t *,char *,enum MJobStateEnum *,mulong *); 
int __MPBSGetNodeState(char *,enum MNodeStateEnum *,struct batch_status *);
int MPBSQueryMOM(mnode_t *,mrm_t *,char *,int *);
int MPBSGetClassInfo(mnode_t *N,char C[][MAX_MNAME],char A[][MAX_MNAME]); 
int __MPBSGetTaskList(mjob_t *,char *,short *,int); 
int __MPBSNLToTaskString(mnalloc_t *,mrm_t *,char *,int);
int __MPBSIGetSSSStatus(mnode_t *,char *); 
long MPBSGetResKVal(char *);
int MPBSQueueQuery(mrm_t *,int *,int *);
int __MPBSJobChkExecutable(struct batch_status *);




/* scheduler globals */

extern mattrlist_t  MAList;

extern mnode_t     *MNode[];
extern mrm_t        MRM[];
extern mrmfunc_t    MRMFunc[];




int MPBSLoadModule(

  mrmfunc_t *F)  /* I (modified) */

  {
  if (F == NULL)
    {
    return(FAILURE);
    }

  F->ClusterQuery   = MPBSClusterQuery;     
  F->JobCancel      = MPBSJobCancel;
  F->JobMigrate     = MPBSJobMigrate;
  F->JobModify      = MPBSJobModify;
  F->JobQuery       = NULL;
  F->JobRequeue     = MPBSJobRequeue;
  F->JobCheckpoint  = MPBSJobCkpt;
  F->JobResume      = MPBSJobResume;
  F->JobStart       = MPBSJobStart;
  F->JobSubmit      = MPBSJobSubmit;
  F->JobSuspend     = MPBSJobSuspend;
  F->QueueQuery     = MPBSQueueQuery;
  F->ResourceModify = NULL;
  F->ResourceQuery  = MPBSQueryMOM;
  F->RMInitialize   = MPBSInitialize;
  F->RMQuery        = NULL;
  F->WorkloadQuery  = MPBSWorkloadQuery;      
  F->RMEventQuery   = MPBSProcessEvent;

  return(SUCCESS);
  }  /* END MPBSLoadModule() */




int MPBSInitialize(

  mrm_t *R,  /* I */
  int   *SC) /* O (optional) */

  {
  int  sd;

  int  PBSEPort;
  char PBSServer[MAX_MNAME];

  msocket_t tmpS;

  static int InitialAttempt = TRUE;

  const char *FName = "MPBSInitialize";

  DBG(1,fPBS) DPrint("%s(%s,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");

  PBSServer[0] = '\0';

  R->P[0].Type = mpstQM;

  if (R->P[0].HostName != NULL)
    {
    strcpy(PBSServer,R->P[0].HostName);
    }

  if (R->P[0].Port > 0)
    {
    sprintf(PBSServer,"%s:%d",
      PBSServer,
      R->P[0].Port);
    }

  if (InitialAttempt == TRUE)
    {
    sleep(1); 

    /* initialize sockets */

    R->U.PBS.ServerSD  = -1;
    R->U.PBS.SchedS.sd = -1;

    InitialAttempt = FALSE;

    if (R->P[0].Timeout != 0)
      {
      char tmpLine[MAX_MLINE];

      sprintf(tmpLine,"%ld",
        R->P[0].Timeout);

      MUSetEnv("PBSAPITIMEOUT",tmpLine);
      }
    }
  else
    {
    /* close active sockets */

    if (R->U.PBS.ServerSD >= 0)
      {
      pbs_disconnect(R->U.PBS.ServerSD);

      R->U.PBS.ServerSD = -1;
      }

    if (R->U.PBS.SchedS.sd > 0)
      {
      close(R->U.PBS.SchedS.sd);

      R->U.PBS.SchedS.sd = -1;
      }
    }  /* END else (InitialAttempt == TRUE) */

  if ((sd = pbs_connect(PBSServer)) < 0) 
    {
    DBG(1,fPBS) DPrint("ERROR:    cannot connect to PBS server '%s'  rc: %d (errno: %d)\n",
      PBSServer,
      sd,
      pbs_errno);

    return(FAILURE);
    }

  R->U.PBS.ServerSD          = sd;
  R->U.PBS.ServerSDTimeStamp = MSched.Time;

  R->FailIteration           = -1;
  R->FailCount               = 0;

  if (R->U.PBS.SubmitExe[0] == '\0')
    {
    strcpy(
      R->U.PBS.SubmitExe,
      DEFAULT_PBSQSUBPATHNAME);
    }

  /* attempt to establish event based interface */

  if (R->EPort > 0)
    {
    PBSEPort = R->EPort; 
    }
  else
    {
    PBSEPort = get_svrport(
      PBS_SCHEDULER_SERVICE_NAME,
      "tcp",
      PBS_SCHEDULER_SERVICE_PORT);     

    R->EPort = PBSEPort;
    }

  MSUInitialize(
    &tmpS,
    NULL,
    PBSEPort,
    MSched.ClientTimeout,
    (1 << TCP));

  if (MSUListen(&tmpS) == FAILURE)
    {
    DBG(1,fPBS) DPrint("WARNING:  cannot connect to PBS scheduler port %d\n",
      PBSEPort);

    R->U.PBS.SchedS.sd = -1;   
    }
  else
    {
    R->U.PBS.SchedS.sd = tmpS.sd;
    }

  R->U.PBS.PBS5IsEnabled = -1;
  R->U.PBS.SSSIsEnabled  = -1;

  switch(R->SubType)
    {
    case mrmstRMS:
 
      MRMSInitialize();
 
      break;
 
    default:

      /* NO-OP */
 
      break;
    }  /* END switch(R->SubType) */

  /* load/update server info */

  __MPBSSystemQuery(R,SC);

  if (X.XPBSInitialize != (int (*)())0)
    (*X.XPBSInitialize)(X.xd,R);

  DBG(1,fPBS) DPrint("INFO:     connected to PBS server %s:%d on sd %d\n",
    (R->P[0].HostName != NULL) ? R->P[0].HostName : "",
    R->P[0].Port,
    R->U.PBS.ServerSD);

  return(SUCCESS);
  }  /* END MPBSInitialize() */




int MPBSProcessEvent(

  mrm_t *R,  /* I */
  int   *SC) /* O (optional) */

  {
  int  PBSCmd;
  int *Iptr;

  int EventReceived     = FALSE;
  int EventsOutstanding = TRUE;

  msocket_t S;
  msocket_t C;

  struct timeval timeout;    

  static struct timeval PrevT = { 0,0 };
  
  if (R == NULL)
    {
    return(FAILURE);
    }

  if (R->U.PBS.SchedS.sd == -1)
    {
    DBG(8,fPBS) DPrint("INFO:     invalid PBS sched socket\n");
   
    return(FAILURE);
    }

  while (EventsOutstanding == TRUE)
    {       
    fd_set fdset;
    extern int rpp_fd;   

    FD_ZERO(&fdset);

    if (rpp_fd != -1)
      {
      FD_SET(rpp_fd,&fdset);
      }

    FD_SET(R->U.PBS.SchedS.sd,&fdset);

    timeout.tv_sec  = 0;
    timeout.tv_usec = 10000;   

    if (select(FD_SETSIZE,&fdset,NULL,NULL,&timeout) == -1) 
      {
      DBG(1,fPBS) DPrint("ALERT:    select failed checking PBS sched socket\n");     

      if (errno != EINTR) 
        {
        /* bad failure */

        EventsOutstanding = FALSE;
 
        break;             
        }

      EventsOutstanding = FALSE;
 
      break;     
      }
 
    if ((rpp_fd != -1) && !FD_ISSET(rpp_fd,&fdset)) 
      {
      /* rpp failure */

      DBG(9,fPBS) DPrint("ALERT:    no PBS RPP sched socket connections ready\n");
 
      EventsOutstanding = FALSE;        
      }
    else if (!FD_ISSET(R->U.PBS.SchedS.sd,&fdset))
      {
      /* no connections ready */

      DBG(9,fPBS) DPrint("INFO:     no PBS sched socket connections ready\n");    

      EventsOutstanding = FALSE;
 
      break;     
      }

    memset(&S,0,sizeof(S));

    S.sd = R->U.PBS.SchedS.sd;

    if (MSUAcceptClient(&S,&C,NULL,(1 << TCP)) == FAILURE)
      {
      /* cannot accept new socket */

      DBG(1,fPBS) DPrint("ALERT:    cannot accept client on PBS sched socket\n");  

      EventsOutstanding = FALSE;
 
      break;     
      }

    Iptr = &PBSCmd;

    if (MSURecvPacket(
          C.sd,
          (char **)&Iptr,
          sizeof(PBSCmd),
	  NULL,
          100000,
          NULL) == FAILURE)
      {
      DBG(1,fPBS) DPrint("ALERT:    cannot read on PBS sched socket\n");  

      close(C.sd);

      EventsOutstanding = FALSE;
 
      break;      
      }

    /* PBS sched command received */

    close(C.sd);

    DBG(4,fPBS) DPrint("INFO:     PBS command %d received\n",
      PBSCmd);

#ifdef NOT
    switch(PBSCmd)
      {
      case SCH_SCHEDULE_NEW:
      case SCH_SCHEDULE_TERM: 

        EventReceived = TRUE;

        break;

      case SCH_ERROR:
      case SCH_SCHEDULE_NULL:
      case SCH_SCHEDULE_TIME:
      case SCH_SCHEDULE_FIRST:
      case SCH_CONFIGURE:
      case SCH_QUIT:
      case SCH_RULESET:
      case SCH_RECYC:

      default: 
 
        /* ignore event */

        break;
      }
#else /* NOT */
    EventReceived = TRUE;         
#endif /* NOT */
    }  /* END while (EventsOutstanding == TRUE) */

  if (EventReceived == FALSE)
    {
    /* check timer */

    if ((R->EMinTime > 0) && (PrevT.tv_sec > 0))
      {
      long NowMS;
      long EMS;

      MUGetMS(NULL,&NowMS);
      MUGetMS(&PrevT,&EMS);
 
      if ((NowMS - EMS) > R->EMinTime)
        {
        /* adequate time has passed between events */

        /* clear timer */

        memset(&PrevT,0,sizeof(PrevT));

        return(SUCCESS); 
        }
      }

    return(FAILURE);
    }  /* END if (EventReceived == FALSE) */

  /* event received, update timer */

  if (R->EMinTime > 0)
    {
    gettimeofday(&PrevT,NULL);

    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MPBSProcessEvent() */ 




          
int MPBSWorkloadQuery(

  mrm_t *R,      /* I */
  int   *JCount, /* I */
  int   *SC)     /* O (modified) */

  {
  struct batch_status *jobs = NULL;
  struct batch_status *cur_job;

  mjob_t *J;
  
  enum MJobStateEnum Status;

  char   RMJID[MAX_MNAME];
  char   SJID[MAX_MNAME];

  char  *ErrMsg;

  short  TaskList[MAX_MTASK_PER_JOB + 1];
  char   Message[MAX_MLINE];

  int    OldState;

  mjob_t *JNext;

  mulong  JobFlagBM;

  const char *FName = "MPBSWorkloadQuery";

  DBG(1,fPBS) DPrint("%s(%s,JCount,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");

  if (R == NULL)
    {
    return(FAILURE);
    }

  if ((MSched.Time > (R->U.PBS.ServerSDTimeStamp + 3000)) ||
      (R->U.PBS.ServerSD <= 0))
    {
    MPBSInitialize(R,NULL);

    if (R->U.PBS.ServerSD <= 0)
      {
      /* cannot recover PBS */

      DBG(1,fPBS) DPrint("ALERT:    cannot re-initialize PBS interface\n");

      R->FailIteration = MSched.Iteration;
      R->FailCount     = MAX_RMFAILCOUNT;

      return(FAILURE);
      }
    }

  if (JCount != NULL)
    *JCount = 0;

  if ((MSim.RMFailureTime >= MSched.Time) ||
      (jobs = pbs_statjob(R->U.PBS.ServerSD,NULL,NULL,NULL)) == NULL)
    {
    if (MSim.RMFailureTime < MSched.Time)
      ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);
    else
      ErrMsg = NULL;

    if (ErrMsg == NULL)
      {
      DBG(3,fPBS) DPrint("INFO:     queue is empty\n");
      }
    else
      {
      DBG(0,fPBS) DPrint("ALERT:    queue is empty or cannot get PBS job info: %s\n",
        ErrMsg);

      R->U.PBS.ServerSDTimeStamp = 0;

      MRMSetFailure(R,mrmWorkloadQuery,"cannot get job info");

      R->FailIteration = MSched.Iteration;
      R->FailCount     = MAX_RMFAILCOUNT;

      if (R->U.PBS.ServerSD > 0)
        {
        pbs_disconnect(R->U.PBS.ServerSD);
  
        R->U.PBS.ServerSD = -1;
        }
      }
    }
  else
    {
    R->WorkloadUpdateIteration = MSched.Iteration;

    for (cur_job = jobs;cur_job != NULL;cur_job = cur_job->next)
      {
      /* ignore jobs that are in route queues */

      if (__MPBSJobChkExecutable(cur_job) == FAILURE)
        {
        DBG(3,fPBS) DPrint("INFO:     ignoring job %s in routing queue\n",
          cur_job->name);

        continue;
        }

      if (JCount != NULL)
        (*JCount)++;

      RMJID[0] = '\0';

      if (__MPBSJobGetState(cur_job,R,RMJID,&Status,&JobFlagBM) == FAILURE)
        break;

      MJobGetName(NULL,RMJID,R,SJID,sizeof(SJID),mjnShortName);

      J = NULL;

      switch (Status)
        {
        case mjsIdle:
        case mjsStarting:
        case mjsRunning:
        case mjsSuspended:
        case mjsHold:

          if (MJobFind(SJID,&J,0) == SUCCESS)
            {
            MRMJobPreUpdate(J);
  
            MPBSJobUpdate(cur_job,J,TaskList,R->Index);
  
            MRMJobPostUpdate(J,TaskList,Status,R);
            }
          else if (MJobCreate(SJID,TRUE,&J) == SUCCESS)
            {
            /* if new job, load data */

            MRMJobPreLoad(J,SJID,R->Index);

            MJobSetAttr(J,mjaRMJID,(void **)RMJID,mdfString,mSet);

            if (MPBSJobLoad(SJID,cur_job,J,TaskList,R->Index) == FAILURE)
              {
              DBG(1,fPBS) DPrint("ALERT:    cannot load PBS job '%s'\n",
                SJID);

              continue;
              }

            MRMJobPostLoad(J,TaskList,R);

            R->LastSubmissionTime = MSched.Time;

            DBG(2,fPBS)
              MJobShow(J,0,NULL);
            }
          else
            {
            DBG(1,fPBS) DPrint("ERROR:    job buffer is full  (ignoring job '%s')\n",
              RMJID);
            }

          break;

        case mjsRemoved:
        case mjsCompleted:
        case mjsVacated:

          if (MJobFind(SJID,&J,0) == SUCCESS)
            {
            /* if job never ran, remove record.  job cancelled externally */
  
            if ((J->State != mjsRunning) && (J->State != mjsStarting))
              {
              DBG(1,fPBS) DPrint("INFO:     job '%s' was cancelled externally\n",
                J->Name);
  
              /* remove job from joblist */
  
              MJobRemove(J);
  
              break;
              }
  
            MRMJobPreUpdate(J);
   
            MPBSJobUpdate(cur_job,J,TaskList,R->Index);
  
            MRMJobPostUpdate(J,TaskList,Status,R);
  
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

                  DBG(3,fPBS) DPrint("INFO:     job '%s' exceeded wallclock limit %s\n",
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
  
                DBG(1,fPBS) DPrint("WARNING:  unexpected job state (%d) detected for job '%s'\n",
                  Status,
                  J->Name);
  
                break;
              }   /* END switch (Status)                     */
            }     /* END if (MJobFind(SJID,&J,0) == SUCCESS) */
          else
            {
            /* ignore job */
  
            DBG(4,fPBS) DPrint("INFO:     ignoring job '%s'  (state: %s)\n",
              RMJID,
              MJobState[Status]);
            }
  
          break;
  
        default:
  
          DBG(1,fPBS) DPrint("WARNING:  job '%s' detected with unexpected state '%d'\n",
            RMJID,
            Status);
  
          break;
        }  /* END switch (Status) */

      if (J != NULL)
        J->IFlags |= JobFlagBM;
      }    /* END for (cur_job)  */

    pbs_statfree(jobs);
    }      /* END else (jobs = pbs_statjob()) */

  /* TEMP:  remove jobs not detected via PBS */

  if ((jobs != NULL) || (R->FailIteration != MSched.Iteration))
    {
    for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = JNext)
      {
      JNext = J->Next; /* store next job pointer in case current job is removed */

      if ((J->ATime > 0) && 
          (MSched.Time - J->ATime > MSched.JobPurgeTime))
        {
        if ((J->State == mjsStarting) || (J->State == mjsRunning))
          {
          DBG(1,fPBS) DPrint("INFO:     active PBS job %s has been removed from the queue.  assuming successful completion\n",
            J->Name);
  
          MRMJobPreUpdate(J);

          /* assume job completed successfully for now */

          OldState          = J->State;

          J->State          = mjsCompleted;

          J->CompletionTime = J->ATime;

          MRMJobPostUpdate(J,NULL,OldState,J->RM);

          MJobProcessCompleted(J);
          }
        else
          {
          DBG(1,fPBS) DPrint("INFO:     non-active PBS job %s has been removed from the queue.  assuming job was cancelled\n",
            J->Name);

          /* just remove job */

          MJobRemove(J);
          }
        }
      }    /* END for (jindex) */
    }      /* END if (jobs != NULL) */

  return(SUCCESS);
  }  /* END MPBSWorkloadQuery() */





int __MPBSJobGetState(

  struct batch_status *PJob,    /* I */
  mrm_t               *R,       /* I */
  char                *JobName, /* O (optional) */
  enum MJobStateEnum  *Status,  
  mulong              *FlagsP) 

  {
  struct attrl *AP;

  *Status = mjsNONE;

  if ((JobName != NULL) && (JobName[0] == '\0'))
    {
    strcpy(JobName,PJob->name);
    }

  if (FlagsP != NULL)
    *FlagsP = 0;

  for (AP = PJob->attribs;AP != NULL;AP = AP->next)
    {
    if (!strcmp(AP->name,ATTR_state))
      {
      switch (AP->value[0])
        {
        case 'Q': /* Queued */
  
          *Status = mjsIdle;

          break;

        case 'R': /* Running */
          
          *Status = mjsRunning;

          break;

        case 'S': /* Suspended */

          *Status = mjsSuspended;

          break;

        case 'T': /* T? */
          
          *Status = mjsStaging;

          break;

        case 'H': /* Hold */
          
          *Status = mjsHold;

          break;

        case 'W':  /* waiting? */
         
          /* job has not reached release time */
 
          *Status = mjsNotQueued;

          break;

        case 'E': 
         
          if (FlagsP != NULL)
            *FlagsP |= (1 << mjifIsExiting);

          *Status = mjsRunning;

          break;

        case 'C': /* completed - TORQUE only */

          *Status = mjsCompleted;
  
          break;

        default:

          /* unexpected job state */

          DBG(1,fPBS) DPrint("ALERT:    unexpected job state '%s' detected for job '%s'\n",
            AP->value,
            PJob->name);

          *Status = mjsNONE;
 
          break;
        }  /* END switch (AP->value[0]) */

      break;
      }    /* END (!strcmp(AP->name,ATTR_state)) */
    }      /* END for (AP) */

  return(SUCCESS);
  }  /* END __MPBSJobGetState() */




int __MPBSSystemQuery(

  mrm_t *R,   /* I */
  int   *SC)  /* O */

  {
  struct batch_status *Server;
  struct batch_status *SP;

  char                *ErrMsg;

  struct attrl        *AP;

  const char *FName = "__MPBSSystemQuery";

  DBG(1,fPBS) DPrint("%s(%s,RCount,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");

  if (R == NULL)
    {
    return(FAILURE);
    }

  if ((MSched.Time > (R->U.PBS.ServerSDTimeStamp + 3000)) ||
      (R->U.PBS.ServerSD <= 0))
    {
    MPBSInitialize(R,NULL);

    if (R->U.PBS.ServerSD <= 0)
      {
      /* cannot recover PBS */

      DBG(1,fPBS) DPrint("ALERT:    cannot re-initialize PBS interface\n");

      R->FailIteration = MSched.Iteration;
      R->FailCount     = MAX_RMFAILCOUNT;

      return(FAILURE);
      }
    }

  if ((MSim.RMFailureTime >= MSched.Time) ||
      (Server = pbs_statserver(R->U.PBS.ServerSD,NULL,NULL)) == NULL)
    {
    if (MSim.RMFailureTime < MSched.Time)
      ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);
    else
      ErrMsg = NULL;

    R->U.PBS.ServerSDTimeStamp = 0;

    R->FailIteration           = MSched.Iteration;
    R->FailCount               = MAX_RMFAILCOUNT;

    DBG(0,fPBS) DPrint("ERROR:    cannot get server info: %s\n",
      (ErrMsg != NULL) ? ErrMsg : "NULL");

    if (R->U.PBS.ServerSD > 0)
      {
      pbs_disconnect(R->U.PBS.ServerSD);

      R->U.PBS.ServerSD = -1;
      }

    return(FAILURE);
    }

  for (SP = Server;SP != NULL;SP = SP->next) 
    {
    for (AP = SP->attribs;AP != NULL;AP = AP->next)
      {
      DBG(6,fPBS) DPrint("INFO:     PBS system attribute '%s'  value: '%s'  (r: %s)\n",
        AP->name,
        (AP->value != NULL) ? AP->value : "NULL",
        (AP->resource != NULL) ? AP->resource : "NULL");

      if (!strcmp(AP->name,"resources_max"))
        {
        if (!strcmp(AP->resource,"ncpus")) 
          {
          MSched.DefaultN.CRes.Procs = (int)strtol(AP->value,NULL,10);

          break;
          }

        /* NYI */
        }
      else if (!strcmp(AP->name,"resources_default"))
        {
        /* NYI */
        }
      else if (!strcmp(AP->name,"resources_available"))
        {
        /* NYI */
        }
      else if (!strcmp(AP->name,"default_queue"))
        {
        if (MSched.DefaultC != NULL)
          {
          MUStrCpy(
            MSched.DefaultC->Name,
            AP->value,
            sizeof(MSched.DefaultC->Name));
          }
        }
      }    /* END for (AP) */
    }      /* END for (SP) */

  return(SUCCESS);
  }  /* END __MPBSSystemQuery() */




int MPBSClusterQuery(

  mrm_t *R,      /* I */
  int   *RCount, /* O */
  char  *EMsg,   /* I (optional) */
  int   *SC)     /* O */

  {
  struct batch_status *nodes;
  char                *ErrMsg;

  char                 Name[MAX_MNAME];
  enum MNodeStateEnum  NewState;

  int                  NewNode;
  enum MNodeStateEnum  OldState;

  struct batch_status *cur_node;
  mnode_t             *N;

  const char *FName = "MPBSClusterQuery";

  DBG(1,fPBS) DPrint("%s(%s,RCount,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");

  if (R == NULL)
    {
    return(FAILURE);
    }

  if ((MSched.Time > (R->U.PBS.ServerSDTimeStamp + 3000)) ||
      (R->U.PBS.ServerSD <= 0))
    {
    MPBSInitialize(R,NULL);

    if (R->U.PBS.ServerSD <= 0)
      {
      /* cannot recover PBS */

      DBG(1,fPBS) DPrint("ALERT:    cannot re-initialize PBS interface\n");

      R->FailIteration = MSched.Iteration;
      R->FailCount     = MAX_RMFAILCOUNT;
 
      return(FAILURE);
      } 
    }

  NewNode = FALSE;

  if ((MSim.RMFailureTime >= MSched.Time) ||
      (nodes = pbs_statnode(R->U.PBS.ServerSD,NULL,NULL,NULL)) == NULL)
    {
    if (MSim.RMFailureTime < MSched.Time)
      ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);
    else
      ErrMsg = NULL;

    R->U.PBS.ServerSDTimeStamp = 0;

    R->FailIteration           = MSched.Iteration;
    R->FailCount               = MAX_RMFAILCOUNT;

    DBG(0,fPBS) DPrint("ERROR:    cannot get node info: %s\n",
      (ErrMsg != NULL) ? ErrMsg : "NULL");

   if (R->U.PBS.ServerSD > 0)
     {
     pbs_disconnect(R->U.PBS.ServerSD);

     R->U.PBS.ServerSD = -1;
     }
 
    return(FAILURE);
    }

  if (RCount != NULL)
    *RCount = 0;

  for (cur_node = nodes;cur_node != NULL;cur_node = cur_node->next)
    {
    if (RCount != NULL)
      (*RCount)++;

    if (__MPBSGetNodeState(Name,&NewState,cur_node) == FAILURE)
      {
      DBG(2,fPBS) DPrint("ALERT:    cannot get PBS node state for node %s\n",
        cur_node->name);

      break;
      }

    if (MNodeFind(Name,&N) == SUCCESS)
      {
      OldState = N->State;

      MRMNodePreUpdate(N,NewState,R);

      MPBSNodeUpdate(N,cur_node,NewState,R);

      MRMNodePostUpdate(N,OldState);
      }
    else if (MNodeAdd(Name,&N) == SUCCESS)
      {
      NewNode = TRUE;

      MRMNodePreLoad(N,NewState,R);

      MPBSNodeLoad(N,cur_node,NewState,R);

      MRMNodePostLoad(N);

      DBG(2,fPBS)
        MNodeShow(N);
      }
    else
      {
      DBG(1,fPBS) DPrint("ERROR:    node buffer is full  (ignoring node '%s')\n",
        Name);
      }
    }    /* END for (cur_node) */

  pbs_statfree(nodes);

  if (NewNode == TRUE)
    {
    MPBSLoadQueueInfo(R,NULL,TRUE,NULL);

    if (R->SubType == mrmstRMS)
      {
#ifdef __MRMS
      MRMSInitialize();
#endif /* __MRMS */
      }
    }

  return(SUCCESS);
  }  /* END MPBSClusterQuery() */




int MPBSQueueQuery(

  mrm_t *R,
  int   *QCount,
  int   *SC)

  {
  int rc;
 
  rc = MPBSLoadQueueInfo(
    R,
    NULL,
    (MSched.Iteration <= 1) ? TRUE : FALSE,
    SC);

  return(rc);
  }  /* END MPBSQueueQuery() */




int MPBSLoadQueueInfo(
 
  mrm_t   *R,        /* I */
  mnode_t *SpecN,    /* I (optional) */
  mbool_t LoadFull, /* I */
  int     *SC)       /* O */

  {
  static struct batch_status *QL = NULL;
  static int    UpdateIteration  = -1;

  char                *ErrMsg;

  struct batch_status *QP;

  struct attrl        *AP;

  int                  nindex;

  mnode_t             *N;

  mclass_t            *C;

  int                  IsGlobal;          /* boolean */
  int                  HostListDetected;  /* boolean */
  int                  ACLHostEnabled;    /* boolean */

  char                *ptr;
  char                *TokPtr;

  int                  QueueStarted;

  const char *FName = "MPBSLoadQueueInfo";

  DBG(1,fPBS) DPrint("%s(%s,%s,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL",
    (SpecN != NULL) ? SpecN->Name : "NULL");

  if (R == NULL)
    {
    return(FAILURE);
    }

  if ((MSched.Time > (R->U.PBS.ServerSDTimeStamp + 3000)) ||
      (R->U.PBS.ServerSD <= 0))
    {
    MPBSInitialize(R,NULL);

    if (R->U.PBS.ServerSD <= 0)
      {
      /* cannot recover PBS */

      DBG(1,fPBS) DPrint("ALERT:    cannot re-initialize PBS interface\n");

      return(FAILURE);
      }
    }

  if (UpdateIteration != MSched.Iteration)
    {
    UpdateIteration = MSched.Iteration;

    if (QL != NULL)
      pbs_statfree(QL);

    if ((MSim.RMFailureTime >= MSched.Time) ||
        (QL = pbs_statque(R->U.PBS.ServerSD,NULL,NULL,NULL)) == NULL)
      {
      if (MSim.RMFailureTime < MSched.Time)
        ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);
      else
        ErrMsg = NULL;

      R->U.PBS.ServerSDTimeStamp = 0;

      DBG(0,fPBS) DPrint("ERROR:    cannot get queue info: %s\n",
        (ErrMsg != NULL) ? ErrMsg : "NULL");

     if (R->U.PBS.ServerSD > 0)
       {
       pbs_disconnect(R->U.PBS.ServerSD);
 
       R->U.PBS.ServerSD = -1;
       }

      return(FAILURE);
      }
    }    /* END if (UpdateIteration != MSched.Iteration) */

  for (QP = QL;QP != NULL;QP = QP->next)
    {
    QueueStarted = FALSE;

    if (MClassAdd(QP->name,&C) == FAILURE)
      continue;

    IsGlobal         = TRUE;
    HostListDetected = FALSE;
    ACLHostEnabled   = FALSE;

    for (AP = QP->attribs;AP != NULL;AP = AP->next)
      {
      /* load limited set of queue attributes (priority, state, limits, ACLs) */

      if (!strcmp(AP->name,ATTR_p))
        {
        /* queue priority */

        int tmpL;

        tmpL = strtol(AP->value,NULL,10);

        if (C->F.IsLocalPriority == FALSE)
          {
          C->F.Priority = tmpL;

          DBG(3,fPBS) DPrint("INFO:     queue '%s' priority set to %ld\n",
            QP->name,
            C->F.Priority);
          }

        continue;
        }

      if (!strcmp(AP->name,ATTR_start))
        {
        /* evaluate queue state */
 
        QueueStarted = MUBoolFromString(AP->value,FALSE);

        if (QueueStarted == FALSE)
          C->IsDisabled = TRUE;
        else
          C->IsDisabled = FALSE;

        DBG(3,fPBS) DPrint("INFO:     queue '%s' started state set to %s\n",
          QP->name,
          AP->value);

        continue;
        }

     if (!strcmp(AP->name,"max_running"))
        {
        /* support max running policy */
   
        MCredSetAttr(
          (void *)C,
          mxoClass,
          mcaMaxJob,
          (void **)AP->value,
          mdfString,
          mSet);
  
        DBG(3,fPBS) DPrint("INFO:     queue '%s' maxrunning set to %s\n",
          QP->name,
          AP->value);
    
        continue;
        }

      if (!strcmp(AP->name,"max_user_run"))
        {
        /* support max active job per user per queue policy */
     
        MCredSetAttr(
          (void *)C,
          mxoClass,
          mcaMaxJobPerUser,
          (void **)AP->value,
          mdfString,
          0);
      
        DBG(3,fPBS) DPrint("INFO:     queue '%s' maxuserrun set to %s\n",
          QP->name,
          AP->value);
       
        continue;
        }
        
      if (LoadFull == FALSE)
        {
        continue;
        }

      if (!strcmp(AP->name,ATTR_aclhten))
        {
        if (MUBoolFromString(AP->value,FALSE) == TRUE)
          {
          ACLHostEnabled = TRUE;
          }
        }
      else if (!strcmp(AP->name,"queue_type"))
        {
        if (strcmp(AP->value,"Execution"))
          {
          /* queue is not execution queue */

          C->NonExeType = TRUE;
          }
        }
      else if (!strcmp(AP->name,ATTR_aclhost))
        {
        /* list of hosts which can submit to queue */

        /* FORMAT:  <HOSTNAME>[{,+}<HOSTNAME>]... */

        HostListDetected = TRUE;

        ptr = MUStrTok(AP->value,",+",&TokPtr);

        while (ptr != NULL)
          {
          if ((SpecN == NULL) || !strcmp(SpecN->Name,ptr))
            {
            if (MNodeFind(ptr,&N) == FAILURE)
              {
              /* cannot find node */
              }
            else
              {
              if (N->CRes.PSlot[C->Index].count == 0)
                {
                int CCount;

                CCount = N->CRes.Procs;

                if ((N->MaxProcPerClass != NULL) && 
                    (N->MaxProcPerClass[C->Index] > 0))
                  {
                  CCount = MIN(CCount,N->MaxProcPerClass[C->Index]);
                  }
                else if ((MSched.DefaultN.MaxProcPerClass != NULL) &&
                         (MSched.DefaultN.MaxProcPerClass[C->Index] > 0))
                  {
                  CCount = MIN(CCount,MSched.DefaultN.MaxProcPerClass[C->Index]);
                  }
	
                N->CRes.PSlot[C->Index].count += CCount;
                N->CRes.PSlot[0].count        += CCount;

                N->ARes.PSlot[C->Index].count += CCount;
                N->ARes.PSlot[0].count        += CCount;
                }
              }  /* END else (MNodeFind() == FAILURE) */
            }    /* END if (SpecN == NULL) || ... ) */

          ptr = MUStrTok(NULL,",+",&TokPtr);
          }      /* END while (ptr != NULL) */
        }
      else if (!strcmp(AP->name,"resources_default"))
        {
        if (!strcmp(AP->resource,"neednodes"))
          {
          char *ptr;

          /* ignore neednodes if used to specify nodecount */

          for (ptr = AP->value;*ptr != '\0';ptr++)
            {
            if (!isdigit(*ptr))
              break;
            }  /* END for (ptr) */

          if (*ptr != '\0')
            {
            /* neednodes node feature specified */

            MClassSetAttr(
              C,
              mclaDefReqFeature,
              (void **)AP->value,
              mdfString,
              mSet);
            }  /* END if (*ptr != '\0') */
          }
        }
      else if (!strcmp(AP->name,"resources_max"))
        {
        if (!strcmp(AP->resource,"nodect"))
          {
          /* NOTE:  PBS 'nodect' constrains job nodes */

          MCredSetAttr(
            (void *)C,
            mxoClass,
            mcaMaxNodePerJob,
            (void **)AP->value,
            mdfString,
            mSet);
          }  /* END if (!strcmp(AP->resource,"nodect")) */
        else if (!strcmp(AP->resource,"ncpus"))
          {
          /* NOTE:  PBS 'ncpus' constrains job procs */

          MCredSetAttr(
            (void *)C,
            mxoClass,
            mcaMaxProcPerJob,
            (void **)AP->value,
            mdfString,
            mSet);
          }  /* END if (!strcmp(AP->resource,"ncpus")) */
        }
      }  /* END for (AP) */

    if ((HostListDetected == TRUE) && (ACLHostEnabled == FALSE))
      {
      DBG(3,fPBS) DPrint("INFO:     class to node mapping enabled for queue '%s'\n",
        QP->name);

      IsGlobal = FALSE;
      }

    if (IsGlobal == TRUE)
      {
      /* add class to all nodes not listed in ACLHosts */

      for (nindex = 0;nindex < MAX_MNODE;nindex++)
        {
        N = MNode[nindex];

	if ((N == NULL) || (N->Name[0] == '\0'))
          break;

        if (N->Name[0] == '\1')
          continue;
	
        if ((SpecN != NULL) && (SpecN != N))
          continue;

        if (N->PrivateQueue == TRUE)
          continue;

        if (N->CRes.PSlot[C->Index].count == 0)
          {
          int CCount;

          CCount = N->CRes.Procs;

          if ((N->MaxProcPerClass != NULL) &&
              (N->MaxProcPerClass[C->Index] > 0))
            {
            CCount = MIN(CCount,N->MaxProcPerClass[C->Index]);
            }

          N->CRes.PSlot[C->Index].count += CCount;
          N->CRes.PSlot[0].count        += CCount;

          if (MClass[C->Index].IsDisabled != TRUE)
            {
            N->ARes.PSlot[C->Index].count += CCount;
            N->ARes.PSlot[0].count        += CCount;
            }
          }
        }  /* END for (nindex) */
      }
    }    /* END for (QP) */

  return(SUCCESS);
  }  /* END MPBSLoadQueueInfo() */





int __MPBSGetNodeState(

  char                *Name,
  enum MNodeStateEnum *State,
  struct batch_status *PNode)

  {
  struct attrl *AP;

  const char *FName = "__MPBSGetNodeState";

  DBG(1,fPBS) DPrint("%s(%s,%s,%s)\n",
    FName,
    (Name != NULL) ? "Name" : "NULL",
    (State != NULL) ? "State" : "NULL",
    (PNode != NULL) ? "PNode" : "NULL");
    
  strcpy(Name,PNode->name);

  if (State == NULL)
    {
    return(FAILURE);
    }

  *State = mnsNone;

  for (AP = PNode->attribs;AP != NULL;AP = AP->next)
    {
    if (!strcmp(AP->name,ATTR_NODE_state))
      {
      if (strstr(AP->value,ND_down) || strstr(AP->value,ND_state_unknown))
        {
        *State = mnsDown;
        }
      else if (strstr(AP->value,ND_offline))
        {
        *State = mnsDrained;
        }
      else if (strstr(AP->value,ND_reserve))
        {
        *State = mnsDrained;
        }
      else if (strstr(AP->value,ND_free))
        {
        *State = mnsIdle;
        }
      else if (strstr(AP->value,ND_job_exclusive))
        {
        *State = mnsBusy;
        }
      else if (strstr(AP->value,ND_busy))
        {
        /* what defines busy? (keyboard idle?) */

        *State = mnsBusy;
        }
      else if (strstr(AP->value,ND_job_sharing))
        {
        *State = mnsActive;
        }
      else
        {
        DBG(1,fPBS) DPrint("ALERT:    cannot map PBS state '%s' to scheduler state on node '%s'\n",
          AP->value,
          Name);
        }
 
      break;
      }  /* END if (!strcmp(AP->name,ATTR_NODE_state)) */
    }    /* END for (AP) */

  if (AP == NULL)
    {
    DBG(1,fPBS) DPrint("ALERT:    PBS node state not specified on node '%s'\n",
      Name);
    }
  else
    {
    DBG(1,fPBS) DPrint("INFO:     PBS node %s set to state %s (%s)\n",
      Name,
      MNodeState[*State],
      AP->value);
    }

  return(SUCCESS);
  }  /* END __MPBSGetNodeState() */






int MPBSJobStart(

  mjob_t *J,   /* I */
  mrm_t  *R,   /* I */
  char   *Msg, /* O (optional) */
  int    *SC)  /* O (optional) */

  {
  int   rc;
  
  static char  HostList[MAX_MBUFFER];
  char         Message[MAX_MLINE];
  char         tmpJobName[MAX_MNAME];

  char        *MasterHost;
  char        *ErrMsg;

  int          JobStartFailed = FALSE;

  const char *FName = "MPBSJobStart";

  DBG(1,fPBS) DPrint("%s(%s,%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if (Msg != NULL)
    Msg[0] = '\0';

  if ((J == NULL) || (R == NULL))
    {
    if (SC != NULL)
      *SC = mscBadParam;

    return(FAILURE);
    }

  MasterHost  = NULL;

  if (R->SubType != mrmstRMS)
    {
    __MPBSNLToTaskString(J->NodeList,R,HostList,sizeof(HostList));

    if (HostList[0] == '\0')
      {
      DBG(0,fPBS) DPrint("ERROR:    job '%s' cannot be started: (empty hostlist)\n",
        J->Name);

      if (Msg != NULL)
        strcpy(Msg,"job cannot be started - empty hostlist");

      if (SC != NULL)
        *SC = mscBadParam;

      return(FAILURE);
      }

    if (MPBSJobModify(J,R,ATTR_l,"neednodes",HostList,NULL,NULL) == FAILURE)
      {
      DBG(0,fPBS) DPrint("ERROR:    cannot set hostlist for job '%s'\n",
        J->Name);

      if (R->FailIteration != MSched.Iteration)
        {
        R->FailIteration = MSched.Iteration;
        R->FailCount     = 0;
        }

      R->FailCount++;

      if (Msg != NULL)
        strcpy(Msg,"job cannot be started - cannot set hostlist");

      if (SC != NULL)
        *SC = mscRemoteFailure;

      return(FAILURE);
      }
    else
      {
      DBG(7,fPBS) DPrint("INFO:     hostlist for job '%s' set to '%s'\n",
        J->Name,
        HostList);
      }
    }
  else
    {
    /* handle RMS */

    static char  RMSNodeSpec[MAX_MLINE];

    /* NOTE:  first field, base node number, is relative to RMS partition  */
    /*        The code below requires a single full partition with PBS     */
    /*        reporting all nodes in partition order                       */

    sprintf(RMSNodeSpec,"%d/%d:%d",
      J->NodeList[0].N->SlotIndex,
      J->Req[0]->NodeCount,
      J->Req[0]->TaskCount);

    if (MPBSJobModify(J,R,ATTR_l,"rmsnodes",RMSNodeSpec,NULL,NULL) == FAILURE)
      {
      DBG(0,fPBS) DPrint("ERROR:    job '%s' cannot set RMS node spec: (RMSNodeSpec: '%s')\n",
        J->Name,
        RMSNodeSpec);

      if (R->FailIteration != MSched.Iteration)
        {
        R->FailIteration = MSched.Iteration;
        R->FailCount     = 0;
        }

      R->FailCount++;

      return(FAILURE);
      }
    else
      {
      DBG(7,fPBS) DPrint("INFO:     RMS node spec for job '%s' set to '%s'\n",
        J->Name,
        RMSNodeSpec);
      }

    if (J->NodeList[0].N->PtIndex > 0)
      {
      if (MPBSJobModify(
            J,
            R,
            ATTR_l,
            "rmspartition",
            MPar[J->NodeList[0].N->PtIndex].Name,NULL,NULL) == FAILURE)
        {
        DBG(0,fPBS) DPrint("ERROR:    job '%s' cannot set RMS node spec: (RMSNodeSpec: '%s')\n",
          J->Name,
          RMSNodeSpec);

        if (R->FailIteration != MSched.Iteration)
          {
          R->FailIteration = MSched.Iteration;
          R->FailCount     = 0;
          }

        R->FailCount++;

        return(FAILURE);
        }
      else
        {
        DBG(7,fPBS) DPrint("INFO:     RMS partition for job '%s' set to '%s'\n",
          J->Name,
          MPar[J->NodeList[0].N->PtIndex].Name);
        }
      }
    }    /* END else (R->SubType != mrmstRMS) */

  /* NOTE:  may want to change to pbs_asyrunjob() */

  /* NOTE:  pbs allows specification of a MasterHost */
  /*   this info must be obtained from job structure */

  MJobGetName(J,NULL,R,tmpJobName,sizeof(tmpJobName),mjnRMName);       

  rc = pbs_runjob(R->U.PBS.ServerSD,tmpJobName,MasterHost,NULL);

  if (rc != 0)
    {
    ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);

    sprintf(Message,"cannot start job - RM failure, rc: %d, msg: '%s'",
      rc,
      (ErrMsg != NULL) ? ErrMsg : "");

    if (Msg != NULL)
      strcpy(Msg,Message);

    MUStrDup(&J->Message,Message);
      
    DBG(0,fPBS) DPrint("ERROR:    job '%s' cannot be started: (rc: %d  errmsg: '%s'  hostlist: '%s')\n",
      J->Name,
      rc,
      ErrMsg,
      HostList);

    JobStartFailed = TRUE;
    }

  if (J->NeedNodes != NULL)
    {
    if (MPBSJobModify(J,R,ATTR_l,"neednodes",J->NeedNodes,NULL,NULL) == FAILURE)
      {
      DBG(7,fPBS) DPrint("WARNING:  cannot reset hostlist for job '%s')\n",
        J->Name);
      }
    else
      {
      DBG(7,fPBS) DPrint("INFO:     hostlist for job '%s' set to '%s'\n",
        J->Name,
        J->NeedNodes);
      }
    }

  if (JobStartFailed == TRUE)
    {
    /* job could not be started */

    if (R->FailIteration != MSched.Iteration)
      {
      R->FailIteration = MSched.Iteration;
      R->FailCount     = 0;
      }
 
    R->FailCount++;

    if (SC != NULL)
      *SC = mscRemoteFailure;
 
    return(FAILURE);
    }

  /* NOTE: PBS does not provide accurate job start info in many cases */

  J->StartTime    = MSched.Time;
  J->DispatchTime = MSched.Time;

  DBG(1,fPBS) DPrint("INFO:     job '%s' successfully started\n",
    J->Name);

  return(SUCCESS);
  }  /* END MPBSJobStart() */





int MPBSJobSuspend(

  mjob_t *J,    /* I (modified) */
  mrm_t  *R,    /* I */
  char   *Msg,  /* O */
  int    *SC)   /* O */

  {
  int rc;

  char *ErrMsg;

  char tmpJobName[MAX_MNAME];

  const char *FName = "MPBSJobSuspend";
 
  DBG(1,fPBS) DPrint("%s(%s,%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");
 
  if (Msg != NULL)
    Msg[0] = '\0';
 
  if (J == NULL)
    {
    return(FAILURE);
    }

  /* send signal to PBS job */

  MJobGetName(J,NULL,R,tmpJobName,sizeof(tmpJobName),mjnRMName);

  /* valid signals:  suspend, resume, <INT>, "SIG<X>" */
 
  rc = pbs_sigjob(
    R->U.PBS.ServerSD,
    tmpJobName,
    (R->SuspendSig[0] != '\0') ? R->SuspendSig : "suspend",
    NULL);

  /* NOTE:  checkpoint with 'pbs_holdjob(connector,job->jobid,"s",NULL);'  */

  if (rc != 0)
    {
    ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);
 
    DBG(0,fPBS) DPrint("ERROR:    job '%s' cannot be suspended: %s\n",
      J->Name,
      ErrMsg);

    if (Msg != NULL)
      {
      sprintf(Msg,"RM Failure %d: '%s'",
        rc,
        ErrMsg);
      }
 
    if (R->FailIteration != MSched.Iteration)
      {
      R->FailIteration = MSched.Iteration;
      R->FailCount     = 0;
      }
 
    return(FAILURE);
    }

  if (R->SubType == mrmstRMS)
    {
    /* issue RMS job suspension */

    MRMSJobControl(J,"suspend",NULL,NULL);
    }

  /* adjust state */

  MJobSetState(J,mjsSuspended);

  /* release reservation */

  /* adjust stats */

  /* NYI */
 
  DBG(1,fPBS) DPrint("INFO:     job '%s' successfully suspended\n",
    J->Name);

  return(SUCCESS);
  }  /* END MPBSJobSuspend() */





int MPBSJobResume(
 
  mjob_t *J,    /* I */
  mrm_t  *R,    /* I */
  char   *Msg,  /* O */
  int    *SC)   /* O */
 
  {
  int rc;

  char *ErrMsg;

  char tmpJobName[MAX_MNAME];
 
  const char *FName = "MPBSJobResume";
 
  DBG(1,fPBS) DPrint("%s(%s,%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");
 
  if (Msg != NULL)
    Msg[0] = '\0';
 
  if (J == NULL)
    {
    return(FAILURE);
    }
 
  /* send signal to PBS job */
 
  MJobGetName(J,NULL,R,tmpJobName,sizeof(tmpJobName),mjnRMName);
 
  rc = pbs_sigjob(
    R->U.PBS.ServerSD,
    tmpJobName,
    "resume",
    NULL);
 
  if (rc != 0)
    {
    ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);
 
    DBG(0,fPBS) DPrint("ERROR:    job '%s' cannot be resumed: %s\n",
      J->Name,
      ErrMsg);

    if (Msg != NULL)
      {
      sprintf(Msg,"RM Failure %d: '%s'",
        rc,
        ErrMsg);
      }
 
    if (R->FailIteration != MSched.Iteration)
      {
      R->FailIteration = MSched.Iteration;
      R->FailCount     = 0;
      }
 
    return(FAILURE);
    }
 
  if (R->SubType == mrmstRMS)
    {
    /* issue RMS job suspension */
 
    MRMSJobControl(J,"resume",NULL,NULL);
    }
 
  /* adjust state */

  MJobSetState(J,mjsRunning); 
 
  /* adjust stats */
 
  /* NYI */
 
  DBG(1,fPBS) DPrint("INFO:     job '%s' successfully resumed\n",
    J->Name);
 
  return(SUCCESS);
  }  /* END MPBSJobResume() */





int MPBSJobCancel(

  mjob_t *J,       /* I */
  mrm_t  *R,       /* I */
  char   *Message, /* I */
  char   *Msg,     /* O */
  int    *SC)      /* O */

  {
  char *ErrMsg;
  int   rc;

  char  tmpJobName[MAX_MNAME];

  const char *FName = "MPBSJobCancel";

  DBG(1,fPBS) DPrint("%s(%s,%s,CMsg,Msg,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL",
    (Message != NULL) ? Message : "NULL");

  if (MSched.PreemptPolicy == mppCheckpoint)
    {
    return(MPBSJobCkpt(J,R,TRUE,Message,SC));
    }

  MJobGetName(J,NULL,R,tmpJobName,sizeof(tmpJobName),mjnRMName);     

  rc = pbs_deljob(R->U.PBS.ServerSD,tmpJobName,Message);

  if (rc != 0)
    {
    ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);

    DBG(0,fPBS) DPrint("ERROR:    job '%s' cannot be cancelled: %s\n",
      J->Name,
      ErrMsg);

    if (R->FailIteration != MSched.Iteration)
      {
      R->FailIteration = MSched.Iteration;
      R->FailCount     = 0;
      }
 
    return(FAILURE);
    }

  DBG(1,fPBS) DPrint("INFO:     job '%s' successfully cancelled\n",
    J->Name);

  return(SUCCESS);
  }  /* END MPBSJobCancel() */




int MPBSNodeLoad(

  mnode_t             *N,        /* I (modified) */
  struct batch_status *PNode,    /* I */
  int                  Status,   /* I */
  mrm_t               *R)        /* I */

  {
  struct attrl *AP;

  int           nindex;
  int           cindex;

  mjob_t       *J;

  char         *ptr;

  char          JobID[MAX_MNAME];

  char          tmpBuffer[MAX_MBUFFER];

  char         *TokPtr;

  mulong        tmpTime;

  const char *FName = "MPBSNodeLoad";

  DBG(2,fPBS) DPrint("%s(%s,%s,%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    (PNode != NULL) ? PNode->name : "NULL",
    MNodeState[Status],
    (R != NULL) ? R->Name : "NULL");

  if ((N == NULL) || (PNode == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  MTRAPNODE(N,FName);

  MUGetTime(&tmpTime,mtmNONE,NULL);

  N->CTime = tmpTime;
  N->MTime = tmpTime;
  N->ATime = tmpTime;

  N->RM = R;

  N->TaskCount = 0;

  /* if SSS PBS patches in place, MOM contact unnecessary */

  if ((Status != mnsDown) && 
      (Status != mnsDrained) && 
      (R->U.PBS.PBS5IsEnabled == -1))
    {
    R->U.PBS.SSSIsEnabled  = FALSE;
    R->U.PBS.PBS5IsEnabled = FALSE;

    for (AP = PNode->attribs;AP != NULL;AP = AP->next)
      {
      /* following are mutually exclusive */

      if (!strcmp(AP->name,"status"))
        {
        R->U.PBS.SSSIsEnabled = TRUE;
        }

      if (!strcmp(AP->name,"resources_assigned"))
        {
        R->U.PBS.PBS5IsEnabled = TRUE;
        }
      }    /* END for (AP) */
    }      /* END if (R->U.PBS.PBS5IsEnabled == -1) */

  if (R->U.PBS.SSSIsEnabled == TRUE)
    {
    /* get node info from 'status' attribute */

    for (AP = PNode->attribs;AP != NULL;AP = AP->next)
      {
      /* following are mutually exclusive */

      if (strcmp(AP->name,"status"))
        continue;

      __MPBSIGetSSSStatus(N,AP->value);

      break;
      }  /* END for (AP) */
    }
  else if (R->U.PBS.PBS5IsEnabled == TRUE)
    {
    for (AP = PNode->attribs;AP != NULL;AP = AP->next)
      {
      MPBSNodeSetAttr(N,(void *)AP,0);
      }  /* END for (AP) */
    }
  else
    {
    /* get node info from MOM */

    /* get CRes.Procs, CRes.Mem, ARes.Disk, Load, Arch, CRes.Swap, and ARes.Swap from MOM */

    if ((Status != mnsDown) && (Status != mnsDrained))
      {
      if (MUThread((int (*)())MPBSQueryMOM,10,NULL,4,NULL,N,R,NULL,NULL) == FAILURE)
        {
        N->State = mnsDown;
        }
      }
    }

  if (N->ARes.Disk > 0)
    {
    if (N->CRes.Disk <= 0)
      N->CRes.Disk = N->ARes.Disk;
    }
  else
    {
    N->ARes.Disk = 1;
    N->CRes.Disk = 1;
    }

  if (N->CRes.Mem > 0)
    {
    if (N->ARes.Mem <= 0)
      N->ARes.Mem = N->CRes.Mem;
    }
  else
    {
    N->ARes.Mem = 1;
    N->CRes.Mem = 1;
    }

  if (N->CRes.Swap > 0)
    {
    if (N->ARes.Swap <= 0)
      N->ARes.Swap = N->CRes.Swap;
    }
  else
    {
    /* virtual memory always at least as large as real memory */

    N->ARes.Swap = MAX(MIN_SWAP,N->ARes.Mem);
    N->CRes.Swap = MAX(MIN_SWAP,N->CRes.Mem);
    }

  /* PBS does not provide pool, opsys, machine speed, or network info */

  N->ActiveOS = MUMAGetIndex(eOpsys,"DEFAULT",mAdd);

  if (N->Network == 0)
    N->Network = MUMAGetBM(eNetwork,"DEFAULT",mAdd);

  N->TaskCount = 0;

  /* get joblist, maxtask, and feature info from PBS server */

  for (AP = PNode->attribs;AP != NULL;AP = AP->next)
    {
    if (!strcmp(AP->name,ATTR_NODE_jobs))
      {
      /* FORMAT:  <JOBID>[,<WS><JOBID>] */

      /* NOTE: if node is space_shared, only single job in list? */

      if (R->SubType == mrmstRMS)
        {
        /* NOTE:  in RMS, the partition base node is reported as running all jobs */
        /* NOTE:  job to node linking must occur elsewhere                        */

        continue;
        }

      MUStrCpy(tmpBuffer,AP->value,sizeof(tmpBuffer));

      DBG(3,fPBS) DPrint("INFO:     node %s has joblist '%s'\n",
        N->Name,
        tmpBuffer);

      ptr = MUStrTok(tmpBuffer,", \t",&TokPtr);

      N->DRes.Procs = 0;

      /* FORMAT:  <JOBID>[/<VP>] */

      while (ptr != NULL)
        {
        char *tail;

        /* job list specified as virtual tasks, '<JOBID>/<TASKID>' */
        /*  or <TASKID>/<JOBID> */

        strtol(ptr,&tail,10);

        if ((tail > ptr) && 
           ((tail[0] == '/') || (tail[0] == '\0')))
          {
          /* FORMAT:  <TASKID>/<JOBID> */

          MUStrCpy(JobID,tail + 1,MAX_MNAME);
          }
        else
          {
          /* FORMAT:  <JOBID>[/<TASKID>] */

          if ((tail = strchr(ptr,'/')) == NULL)
            {
            tail = ptr + strlen(ptr);
            }

          MUStrCpy(JobID,ptr,MIN(MAX_MNAME,tail - ptr + 1));
          }

        N->TaskCount ++;

        if (MJobFind(JobID,&J,0) == SUCCESS)
          {
          N->DRes.Procs += MAX(1,J->Req[0]->DRes.Procs);  /* FIXME */

          if (J->NodeList != NULL)
            {
            for (nindex = 0;nindex < MAX_MNODE_PER_JOB;nindex++)
              {
              if (J->NodeList[nindex].N == N)
                break;
 
              if (J->NodeList[nindex].N == NULL)
                {
                J->NodeList[nindex].N  = N;
                J->NodeList[nindex].TC = 1;
  
                J->NodeList[nindex + 1].N = NULL;

                break;
                }
              }    /* END for (nindex) */
            }
          }        /* END if (MJobFind(ptr,&J,0) == SUCCESS) */
        else
          {
          /* NOTE:  assume 1 proc per task for unknown jobs */

          DBG(2,fPBS) DPrint("INFO:     cannot locate PBS job '%s' (running on node %s)\n",
            JobID,
            N->Name);

          N->DRes.Procs += 1;
          }

        if ((N->State == mnsIdle) || (N->State == mnsActive))
          {
          int UseUtil = FALSE;
          int UseDed  = FALSE;

          int OldState;

          OldState = N->State;

          if ((MPar[0].NAvailPolicy[mrProc] == mrapUtilized) ||
              (MPar[0].NAvailPolicy[mrProc] == mrapCombined)) 
            {
            UseUtil = TRUE;
            }
 
          if ((MPar[0].NAvailPolicy[mrProc] == mrapDedicated) ||
              (MPar[0].NAvailPolicy[mrProc] == mrapCombined)) 
            {
            UseDed = TRUE;
            }

          if (((UseDed == TRUE) && (N->DRes.Procs >= N->CRes.Procs)) ||
              ((UseUtil == TRUE) && (N->URes.Procs >= N->CRes.Procs)))
            {
            N->State = mnsBusy;
            }
          else if (((UseDed == TRUE) && (N->DRes.Procs >= 1)) ||
              ((UseUtil == TRUE) && (N->URes.Procs >= 1)))
            {
            N->State = mnsActive;
            }

          if (N->State != OldState)
            {
            DBG(3,fPBS) DPrint("INFO:     adjusted node %s from state %s to %s (DRes.Procs: %d  CRes.Procs: %d)\n",
              N->Name,
              MNodeState[OldState],
              MNodeState[N->State],
              N->DRes.Procs,
              N->CRes.Procs);
 
            /* force state sync */
 
            N->EState = N->State;
            }
          }    /* END if ((N->State == mnsIdle) || (N->State == mnsActive)) */

        ptr = MUStrTok(NULL,", \t",&TokPtr);
        }          /* END while (ptr != NULL) */
      }
    else if (!strcmp(AP->name,ATTR_NODE_ntype))
      {
      /* what is strict definition of time_shared vs cluster */

      if (!strcmp(AP->value,ND_cluster))
        {
        /* NOTE:  FIXME:  cluster vs time_shared must be handled, but not this way */
/*
        N->AP.HLimit[mptMaxProc][0] = 1;
*/
        }
      }
    else if (!strcmp(AP->name,ATTR_NODE_properties))
      {
      if (R->U.PBS.PBS5IsEnabled == FALSE)
        {
        strcpy(tmpBuffer,AP->value);
       
        ptr = MUStrTok(tmpBuffer,", \t",&TokPtr);

        while (ptr != NULL)
          {
          MNodeProcessFeature(N,ptr);
  
          ptr = MUStrTok(NULL,", \t",&TokPtr);
          } /* END while (ptr != NULL) */
        }
      }   /* END else if (!strcmp()) */
    else if (!strcmp(AP->name,ATTR_NODE_np))
      {
      /* set number of processors */

      N->CRes.Procs = strtol(AP->value,NULL,10);   

      if (MSched.NodeCPUOverCommitFactor > 0.0)
        N->CRes.Procs = (int)(N->CRes.Procs * MSched.NodeCPUOverCommitFactor);
      }
    else if (!strcmp(AP->name,"queue"))
      {
      /* determine queues supported by node */

      MNodeSetClass(N,NULL,AP->value,mSet);

      N->PrivateQueue = TRUE;
      }
    else if (!strcmp(AP->name,"resources_available"))
      {
      if (!strcmp(AP->resource,"mem"))
        {
        N->CRes.Mem = (MPBSGetResKVal(AP->value) >> 10);
   
        N->ARes.Mem = MIN(N->ARes.Mem,N->CRes.Mem);
        }
      else if (!strcmp(AP->resource,"vmem"))
        {
        N->CRes.Swap = (MPBSGetResKVal(AP->value) >> 10);

        N->ARes.Swap = MIN(N->ARes.Swap,N->CRes.Swap);
        }
      }
    }     /* END for (AP) */

  /* TEMP HACK */

  if (N->State == mnsIdle)
    {
    N->ARes.Procs = N->CRes.Procs;
    }
  else if (N->State == mnsActive)
    {
    /* let MNodeUpdateState() calculate ARes.Procs */

    /*
    N->ARes.Procs = MAX(0,N->CRes.Procs - N->DRes.Procs);
    */
    }
  else
    {
    N->ARes.Procs = 0;
    }

  for (cindex = 0;cindex < MAX_MCLASS;cindex++)
    {
    if ((N->State != mnsIdle) && (N->State != mnsActive))
      {
      N->ARes.PSlot[cindex].count = 0;
      }
    else  
      {
      /* TEMP HACK for PBS v2.[12].x/PBS Pro */

      N->ARes.PSlot[cindex].count = MIN(N->ARes.PSlot[cindex].count,N->ARes.Procs);
      }
    }  /* END for (cindex) */

  if (N->AP.HLimit[mptMaxProc][0] > 0)
    N->CRes.PSlot[0].count = N->AP.HLimit[mptMaxProc][0];
  else 
    N->AP.HLimit[mptMaxProc][0] = N->CRes.PSlot[0].count;

  if (N->State == mnsNone)
    {
    DBG(3,fPBS) DPrint("WARNING:  node '%s' is unusable in state 'NONE'\n",
      N->Name);
    }

  if ((N->ARes.Disk < 0) && (N->State == mnsIdle))
    {
    DBG(2,fPBS) DPrint("WARNING:  idle node %s is unusable  (inadequate disk space in /var)\n",
      N->Name);
    }

  MLocalNodeInit(N);

  DBG(6,fPBS) DPrint("INFO:     MNode[%03d] '%18s' %9s VM: %8d Mem: %5d Dk: %5d Cl: %6s %s\n",
    N->Index,
    N->Name,
    MAList[eNodeState][N->State],
    N->CRes.Swap,
    N->CRes.Mem,
    N->CRes.Disk,
    MUCAListToString(N->ARes.PSlot,N->CRes.PSlot,NULL),
    MUMAList(eFeature,N->FBM,sizeof(N->FBM)));

  DBG(6,fPBS) DPrint("INFO:     MNode[%03d] '%18s' C/A/D procs:  %d/%d/%d\n",
    N->Index,
    N->Name,
    N->CRes.Procs,
    N->ARes.Procs,
    N->DRes.Procs);

  return(SUCCESS);
  }  /* END MPBSNodeLoad() */




int MPBSQueryMOM(

  mnode_t *N,   /* I (modified) */
  mrm_t   *R,   /* I */
  char    *Msg, /* O (optional) */
  int     *SC)  /* O (optional) */

  {
  int sd;
  int rindex;

  char *ptr;

  char *Value;
  char *tail;

  double dval;

  int    RCount;

  mulong T;

  char   DiskLine[MAX_MLINE];

  int    TotMem = 0;

  char **RL;

  char *PBSResList[] = {
    "ncpus",              /* number of CPUS */
    "arch",               /* the architecture of the machine */
    "physmem",            /* the amount of physical memory */
    "loadave",            /* the current load average */
#if defined(__AIX43) || defined(__AIX51) || defined(__LINUX) || defined(__IRIX)
    "availmem",
    "totmem",
#endif /* AIX43 || LINUX || IRIX */
    NULL };

  char *PBS5ResList[] = {
    "loadave",            /* the current load average */
    NULL };

  const char *FName = "MPBSQueryMOM";

  DBG(2,fPBS) DPrint("%s(%s,%s,Msg,SC)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if ((N == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  if (MSched.Iteration > 0)
    {
    N->IterationCount++;

    if ((N->IterationCount < MSched.NodePollFrequency) &&
        (N->CTime < MSched.Time) &&
        (N->StateMTime < MSched.Time))
      {
      DBG(6,fPBS) DPrint("INFO:     MOM on host '%s' not updated this iteration (%d < %d)\n",
        N->Name,
        N->IterationCount,
        MSched.NodePollFrequency);

      return(SUCCESS);
      }
    else
      {
      N->IterationCount = 0;
      }
    }
  else if (MSched.NodePollFrequency > 1)
    {
    N->IterationCount = MSched.NodePollOffset;

    MSched.NodePollOffset++;

    MSched.NodePollOffset %= MSched.NodePollFrequency;
    }

  if ((sd = openrm(N->Name,N->RM->NMPort)) < 0)
    {
    DBG(2,fPBS) DPrint("ALERT:    cannot connect to MOM on node '%s', rc: %d\n",
      N->Name,
      sd);

    return(FAILURE);
    }

  RCount = 0;

  RL = (R->U.PBS.PBS5IsEnabled == TRUE) ? PBS5ResList : PBSResList;

  for (rindex = 0;RL[rindex] != NULL;rindex++)
    {
    if (addreq(sd,RL[rindex]) != 0)
      {
      DBG(2,fPBS) DPrint("ALERT:    cannot add req '%s' to MOM on node '%s' (errno: %d:%d)\n",
        RL[rindex],
        N->Name,
        errno,
        pbs_errno);

      closerm(sd);

      return(FAILURE);
      }

    RCount++;
    }    /* END for (rindex) */

  /* determine local disk space */

  if (N->RM->U.PBS.LocalDiskFS[0] != '\0')
    {
    sprintf(DiskLine,"size[fs=%s]",
      N->RM->U.PBS.LocalDiskFS);

    if (addreq(sd,DiskLine) != 0)
      {
      DBG(2,fPBS) DPrint("ALERT:    cannot add req to MOM on node '%s' (errno: %d:%d)\n",
        N->Name,
        errno,
        pbs_errno);

      closerm(sd);

      return(FAILURE);
      }

    RCount++;
    }
  else
    {
    DiskLine[0] = '\0';
    }

  for (rindex = 0;rindex < RCount;rindex++)
    {
    if ((Value = (char *)getreq(sd)) == NULL)
      {
      DBG(2,fPBS) DPrint("ALERT:    cannot get req from MOM on node '%s' (errno: %d:%d)\n",
        N->Name,
        errno,
        pbs_errno);

      break;
      }

    if (!strncmp(Value,"ncpus=",strlen("ncpus=")))
      {  
      ptr = Value + strlen("ncpus=");

      if (*ptr != '?')
        N->CRes.Procs = (int)strtol(ptr,NULL,10);
      }
    else if (!strncmp(Value,"arch=",strlen("arch=")))
      { 
      ptr = Value + strlen("arch=");

      if (*ptr != '?')
        N->Arch = MUMAGetIndex(eArch,ptr,mAdd);
      }
    else if (!strncmp(Value,"physmem=",strlen("physmem=")))
      {  
      ptr = Value + strlen("physmem=");

      if (*ptr != '?')
        N->CRes.Mem = (MPBSGetResKVal(ptr) >> 10);
      }
    else if (!strncmp(Value,"totmem=",strlen("totmem=")))
      { 
      ptr = Value + strlen("totmem=");

      if (*ptr != '?')
        TotMem = (MPBSGetResKVal(ptr) >> 10);
      }
    else if (!strncmp(Value,"availmem=",strlen("availmem=")))
      { 
      ptr = Value + strlen("availmem=");

      if (*ptr != '?')
        N->ARes.Swap = (MPBSGetResKVal(ptr) >> 10);
      }
    else if (!strncmp(Value,"loadave=",strlen("loadave=")))
      {
      ptr = Value + strlen("loadave=");

      if (*ptr != '?')
        {
        dval = strtod(ptr,&tail);

        if (*tail == '\0')
          N->Load = dval;
        }
      }
    else if (!strncmp(Value,DiskLine,strlen(DiskLine)))
      {
      ptr = Value + strlen(DiskLine) + 1;

      if (*ptr != '?')
        N->ARes.Disk = (MPBSGetResKVal(ptr) >> 10);
      }

    free(Value); 
    }    /* END for (rindex) */

  /* NOTE:  PBS totmem = swap + RAM */

  if ((TotMem > 0) && (N->CRes.Mem > 0))
    N->CRes.Swap = TotMem - N->CRes.Mem;

  closerm(sd);

  if (MSched.NodeCPUOverCommitFactor > 0.0)
    {
    N->CRes.Procs = (int)(N->CRes.Procs * MSched.NodeCPUOverCommitFactor);
    }

  if (MSched.NodeMemOverCommitFactor > 0.0)
    {
    /* NOTE:  both real memory and swap overcommitted */

    N->CRes.Mem = (int)(N->CRes.Mem * MSched.NodeMemOverCommitFactor);
    N->ARes.Mem = (int)(N->CRes.Mem * MSched.NodeMemOverCommitFactor);   

    N->CRes.Swap = (int)(N->CRes.Mem * MSched.NodeMemOverCommitFactor);
    N->ARes.Swap = (int)(N->CRes.Mem * MSched.NodeMemOverCommitFactor);

    /* memory factor not applied to node load */
    }

  DBG(6,fPBS)
    {
    MUGetTime(&T,mtmNONE,NULL);

    DPrint("INFO:     MOM info for host '%s' successfully updated (%s)\n",
      N->Name,
      MULToDString(&T));
    }

  return(SUCCESS);
  }  /* END MPBSQueryMOM() */




int MPBSNodeUpdate(

  mnode_t             *N,       /* I (modified) */
  struct batch_status *PNode,   /* I */
  enum MNodeStateEnum  NState,  /* I */
  mrm_t               *R)       /* I */

  {
  struct attrl *AP;

  int           nindex;
  int           cindex;

  mjob_t       *J;

  char         *ptr;

  char          tmpBuffer[MAX_MBUFFER];

  char         *TokPtr;

  mulong        tmpTime;

  int           tmpProcs;
  int           OldState;

  const char *FName = "MPBSNodeUpdate";

  DBG(2,fPBS) DPrint("%s(%s,%s,%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    (PNode != NULL) ? PNode->name : "NULL",
    MNodeState[NState],
    (R != NULL) ? R->Name : "NULL");

  if ((N == NULL) || (PNode == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  MUGetTime(&tmpTime,mtmNONE,NULL);

  N->MTime = (long)tmpTime;
  N->ATime = (long)tmpTime;

  N->RM = R;

  N->TaskCount = 0;

  /* if SSS PBS patches in place, MOM contact unnecessary */

  if ((NState != mnsDown) && 
      (NState != mnsDrained) &&
      (R->U.PBS.SSSIsEnabled == -1))
    {
    R->U.PBS.SSSIsEnabled  = FALSE;
    R->U.PBS.PBS5IsEnabled = FALSE;

    for (AP = PNode->attribs;AP != NULL;AP = AP->next)
      {
      /* following are mutually exclusive */

      if (!strcmp(AP->name,"status"))
        {
        R->U.PBS.SSSIsEnabled = TRUE;

        break;
        }
      else if (!strcmp(AP->name,"resources_assigned"))
        {
        R->U.PBS.PBS5IsEnabled = TRUE;

        break;
        }
      }    /* END for (AP) */
    }      /* END if (R->U.PBS.SSSIsEnabled == -1) */

  if (R->U.PBS.SSSIsEnabled == TRUE)
    {
    for (AP = PNode->attribs;AP != NULL;AP = AP->next)
      {
      /* following are mutually exclusive */

      if (strcmp(AP->name,"status"))
        continue;

      /* get node info from 'status' attribute */

      __MPBSIGetSSSStatus(N,AP->value);

      break;
      }    /* END for (AP) */
    }
  else if (R->U.PBS.PBS5IsEnabled == TRUE)
    {
    for (AP = PNode->attribs;AP != NULL;AP = AP->next)
      {
      MPBSNodeSetAttr(N,(void *)AP,0);
      }  /* END for (AP) */
    }

  /* get node info from MOM */

  /* get CRes.Procs, CRes.Mem, ARes.Disk, Load, Arch, CRes.Swap and ARes.Swap */

  if ((R->U.PBS.SSSIsEnabled != TRUE) && (N->Load <= 0.0))
    {
    if ((NState != mnsDown) && (NState != mnsDrained))
      {
      if (MUThread((int (*)())MPBSQueryMOM,10,NULL,4,NULL,N,R,NULL,NULL) == FAILURE)   
        {
        MUStrDup(&N->Message,"unable to obtain node information from MOM");

        MNodeSetState(N,mnsDown,0);
        }
      }
    }

  /* process updated state info */

  if (N->ARes.Disk > 0)
    {
    if (N->CRes.Disk <= 0)
      N->CRes.Disk = N->ARes.Disk;
    }
  else
    {
    N->ARes.Disk = 1;
    N->CRes.Disk = 1;
    }

  if (N->CRes.Mem > 0)
    {
    if (N->ARes.Mem <= 0)
      N->ARes.Mem = N->CRes.Mem;
    }
  else
    {
    N->ARes.Mem = 1;
    N->CRes.Mem = 1;
    }

  if (N->CRes.Swap > 0)
    {
    if (N->ARes.Swap <= 0)
      N->ARes.Swap = N->CRes.Swap;
    }
  else
    {
    /* virtual memory always at least as large as real memory */

    N->ARes.Swap = MAX(MIN_SWAP,N->ARes.Mem);
    N->CRes.Swap = MAX(MIN_SWAP,N->CRes.Mem);
    }

  /* PBS does not provide pool, opsys, machine speed, or network info */

  N->ActiveOS = MUMAGetIndex(eOpsys,"DEFAULT",mAdd);

  if (N->Network == 0)
    N->Network = MUMAGetBM(eNetwork,"DEFAULT",mAdd);

  /* get joblist, maxtask, and feature info from PBS server */

  N->TaskCount = 0;

  for (AP = PNode->attribs;AP != NULL;AP = AP->next)
    {
    if (!strcmp(AP->name,ATTR_NODE_jobs))
      {
      char  JobID[MAX_MNAME];

      /* NOTE: jobs listed in '<JOBID>[/<TASKID>][,<WS><JOBID>[/<TASKID>]]...' format */

      /* NOTE: if node is space_shared, only single job in list */

      if (R->SubType == mrmstRMS)
        {
        /* NOTE:  in RMS, the partition base node is reported as running all jobs */
        /* NOTE:  job to node linking must occur elsewhere                        */

        continue;
        }

      strcpy(tmpBuffer,AP->value);

      DBG(3,fPBS) DPrint("INFO:     node %s has joblist '%s'\n",
        N->Name,
        tmpBuffer);

      ptr = MUStrTok(tmpBuffer,", \t",&TokPtr);

      N->DRes.Procs = 0;

      while (ptr != NULL)
        {
        char *tail;

        /* job list specified as virtual tasks, '<JOBID>/<TASKID>' */
        /*  or <TASKID>/<JOBID> */

        strtol(ptr,&tail,10);

        if ((tail > ptr) &&
           ((tail[0] == '/') || (tail[0] == '\0')))
          {
          /* FORMAT:  <TASKID>/<JOBID> */

          MUStrCpy(JobID,tail + 1,MAX_MNAME);
          }
        else
          {
          /* FORMAT:  <JOBID>[/<TASKID>] */

          if ((tail = strchr(ptr,'/')) == NULL)
            {
            tail = ptr + strlen(ptr);
            }

          MUStrCpy(JobID,ptr,MIN(MAX_MNAME,tail - ptr + 1));
          }

        N->TaskCount ++;

        if (MJobFind(JobID,&J,0) == SUCCESS)
          {
          if (J->Req[0]->DRes.Procs == -1)
            {
            tmpProcs = N->CRes.Procs;
            }
          else
            {
            tmpProcs = MAX(1,J->Req[0]->DRes.Procs);
            }

          N->DRes.Procs = MIN(N->DRes.Procs + tmpProcs,N->CRes.Procs);

          DBG(3,fPBS) DPrint("INFO:     job %s adds %d processors per task to node %s (%d)\n",
            J->Name,
            J->Req[0]->DRes.Procs,
            N->Name,
            N->DRes.Procs);               

          if (J->NodeList != NULL)
            {
            for (nindex = 0;nindex < MAX_MNODE_PER_JOB;nindex++)
              {
              if (J->NodeList[nindex].N == N)
                break;

              if (J->NodeList[nindex].N == NULL)
                {
                J->NodeList[nindex].N  = N;
                J->NodeList[nindex].TC = 1;

                J->NodeList[nindex + 1].N = NULL;

                break;
                }
              }    /* END for (nindex) */
            }
          }        /* END if (MJobFind(ptr,&J,0) == SUCCESS) */
        else
          {
          /* assume 1 proc per task */

          DBG(2,fPBS) DPrint("ALERT:    cannot locate PBS job '%s' (running on node %s)\n",
            JobID,
            N->Name);

          N->DRes.Procs += 1;
          }

        if ((N->State == mnsIdle) || (N->State == mnsActive))
          {
          int UseUtil = FALSE;
          int UseDed  = FALSE;

          OldState = N->State;

          if ((MPar[0].NAvailPolicy[mrProc] == mrapUtilized) ||
              (MPar[0].NAvailPolicy[mrProc] == mrapCombined))
            {
            UseUtil = TRUE;
            }
 
          if ((MPar[0].NAvailPolicy[mrProc] == mrapDedicated) ||
              (MPar[0].NAvailPolicy[mrProc] == mrapCombined))
            {
            UseDed = TRUE;
            }
 
          if (((UseDed == TRUE) && (N->DRes.Procs >= N->CRes.Procs)) ||
              ((UseUtil == TRUE) && (N->URes.Procs / 100 >= N->CRes.Procs)))
            {
            N->State = mnsBusy;
            }
          else if (((UseDed == TRUE) && (N->DRes.Procs >= 1)) ||
              ((UseUtil == TRUE) && (N->URes.Procs / 100 >= 1)))
            {
            N->State = mnsActive;
            }

          if (N->State != OldState)
            {
            DBG(3,fPBS) DPrint("INFO:     adjusted node %s from state %s to %s (DRes.Procs: %d  CRes.Procs: %d)\n",
              N->Name,
              MNodeState[OldState],
              MNodeState[N->State],
              N->DRes.Procs,
              N->CRes.Procs);
 
            /* force state sync */

            N->EState = N->State;
            }    /* END if ((N->State == mnsIdle) || (N->State == mnsActive)) */
          }      /* END while (ptr != NULL) */

        ptr = MUStrTok(NULL,", \t",&TokPtr);
        }          /* END while (ptr != NULL) */
      }
    else if (!strcmp(AP->name,ATTR_NODE_ntype))
      {
      /* NOTE:  what is strict definition of time_shared vs cluster? */

      if (!strcmp(AP->value,ND_cluster))
        {
        N->AP.HLimit[mptMaxProc][0] = N->CRes.Procs;
        }
      }
    else if (!strcmp(AP->name,ATTR_NODE_properties))
      {
      MUStrCpy(tmpBuffer,AP->value,sizeof(tmpBuffer));

      ptr = MUStrTok(tmpBuffer,", \t",&TokPtr);

      while (ptr != NULL)
        {
        MUGetMAttr(eFeature,ptr,mAdd,N->FBM,sizeof(N->FBM));

        ptr = MUStrTok(NULL,", \t",&TokPtr);
        }
      }
    else if (!strcmp(AP->name,ATTR_NODE_np))
      {
      /* set virtual processors */

      N->CRes.Procs = strtol(AP->value,NULL,10);

      if (MSched.NodeCPUOverCommitFactor > 0.0)
        N->CRes.Procs = (int)(N->CRes.Procs * MSched.NodeCPUOverCommitFactor);
      }
    else if (!strcmp(AP->name,"resources_available"))
      {
      if (!strcmp(AP->resource,"mem"))
        {
        N->CRes.Mem = (MPBSGetResKVal(AP->value) >> 10);
 
        N->ARes.Mem = MIN(N->ARes.Mem,N->CRes.Mem);
        }
      else if (!strcmp(AP->resource,"vmem"))
        {
        N->CRes.Swap = (MPBSGetResKVal(AP->value) >> 10);

        N->ARes.Swap = MIN(N->ARes.Swap,N->CRes.Swap);
        }
      else if (!strcmp(AP->resource,"ncpus"))
        {
        N->CRes.Procs = strtol(AP->value,NULL,10);
        }
      }
    }    /* END for (AP) */

  /* TEMP HACK */

  if (N->State == mnsIdle)
    {
    N->ARes.Procs = N->CRes.Procs;
    }
  else if (N->State == mnsActive)
    {
    /* handled by UpdateNodeState() */

    /*
    N->ARes.Procs = MAX(0,N->CRes.Procs - N->DRes.Procs);
    */
    }
  else
    {
    N->ARes.Procs = 0;
    }

  /* PBS 2.[12].x does not provide class info */

  MPBSLoadQueueInfo(R,N,TRUE,NULL);

  if (N->CRes.PSlot[0].count == 0)
    {
    int cindex;

    strcpy(tmpBuffer,MSched.DefaultClassList);

    MUNumListFromString(N->CRes.PSlot,MSched.DefaultClassList,eClass);

    for (cindex = 1;cindex < MAX_MCLASS;cindex++)
      {
      if (N->CRes.PSlot[cindex].count > 0)
        MClassAdd(MAList[eClass][cindex],NULL);
      }
    }

  memcpy(N->ARes.PSlot,N->CRes.PSlot,sizeof(N->ARes.PSlot));

  for (cindex = 0;cindex < MAX_MCLASS;cindex++)
    {
    if ((N->State != mnsIdle) && (N->State != mnsActive))
      {
      N->ARes.PSlot[cindex].count = 0;
      }
    else if ((N->ARes.Procs > 0) && 
             (N->ARes.PSlot[cindex].count > N->ARes.Procs))
      {
      /* TEMP HACK for PBS v2.1.x */

      N->ARes.PSlot[cindex].count = N->ARes.Procs;
      }
    }  /* END for (cindex) */

  if (N->State == mnsNone)
    {
    DBG(3,fPBS) DPrint("WARNING:  node '%s' is unusable in state 'NONE'\n",
      N->Name);
    }

  if ((N->ARes.Disk < 0) && (N->State == mnsIdle))
    {
    DBG(2,fPBS) DPrint("WARNING:  idle node %s is unusable  (inadequate disk space in /var)\n",
      N->Name);
    }

  if ((N->State != N->EState) && (MSched.Time > N->SyncDeadLine))
    {
    DBG(3,fPBS) DPrint("INFO:     synchronizing node '%s' to expected state '%s' (expected state was '%s')\n",
      N->Name,
      MAList[eNodeState][N->State],
      MAList[eNodeState][N->EState]);

    N->EState = N->State;
    }

  DBG(6,fPBS) DPrint("INFO:     MNode[%03d] '%18s' %9s VM: %8d Mem: %5d Dk: %5d Cl: %6s %s\n",
    N->Index,
    N->Name,
    MAList[eNodeState][N->State],
    N->CRes.Swap,
    N->CRes.Mem,
    N->CRes.Disk,
    MUCAListToString(N->ARes.PSlot,N->CRes.PSlot,NULL),
    MUMAList(eFeature,N->FBM,sizeof(N->FBM)));

  DBG(6,fPBS) DPrint("INFO:     MNode[%03d] '%18s' C/A/D procs:  %d/%d/%d\n",
    N->Index,
    N->Name,
    N->CRes.Procs,
    N->ARes.Procs,
    N->DRes.Procs);

  return(SUCCESS);
  }  /* END MPBSNodeUpdate() */




int MPBSJobLoad(

  char                *JobName,  /* I */
  struct batch_status *PJob,     /* I */
  mjob_t              *J,        /* O (modified) */
  short               *TaskList, /* O */
  int                  RMIndex)  /* I */

  {
  mreq_t       *RQ;

  char          AName[MAX_MNAME];

  mqos_t       *QDef;

  struct attrl *AP;

  tpbsa_t       TA;

  mulong        tmpUL;

  const char *FName = "MPBSJobLoad";

  DBG(2,fPBS) DPrint("%s(%s,%s,J,TaskList,%d)\n",
    FName,
    JobName,
    (PJob != NULL) ? PJob->name : "NULL",
    RMIndex);

  if ((J == NULL) || (PJob == NULL))
    {
    return(FAILURE);
    }

  memset(&TA,0,sizeof(TA));

  if (__MPBSJobGetState(PJob,&MRM[RMIndex],NULL,&J->State,&tmpUL) == FAILURE)
    {
    DBG(1,fPBS) DPrint("ALERT:    cannot get job state info for job '%s'\n",
      J->Name);

    MJobRemove(J);

    return(FAILURE);
    }

  J->IFlags |= tmpUL;

  /* add resource requirements information */
  
  if (MReqCreate(J,NULL,&RQ,FALSE) == FAILURE)
    {
    DBG(1,fPBS) DPrint("ALERT:    cannot add requirements to job '%s'\n",
      J->Name);

    MJobRemove(J);

    return(FAILURE);
    }

  MRMReqPreLoad(RQ);

  TaskList[0] = -1;

  /* get job qtime, comment, uname, gname, account, task/feature request, */
  /*   arch, memory, swap, and disk info from PBS server */

  RQ->NodeCount     = 0;
  RQ->TaskCount     = 0;
  RQ->TasksPerNode  = 0;

  RQ->DRes.Procs    = 1;
  RQ->DRes.Mem      = 0;

  for (AP = PJob->attribs;AP != NULL;AP = AP->next)
    {
    DBG(6,fPBS) DPrint("INFO:     PBS attribute '%s'  value: '%s'  (r: %s)\n",
      AP->name,
      (AP->value != NULL) ? AP->value : "NULL",
      (AP->resource != NULL) ? AP->resource : "NULL");

    MPBSJobSetAttr(J,(void *)AP,NULL,&TA,TaskList,0);
    }    /* END for (AP) */

  if (MRM[RMIndex].SubType == mrmstRMS)
    {
    /* determine RMS job ID and obtain RMS specific info for job */

    if (MJOBISACTIVE(J))
      {
      int TaskCount = 1;

#ifdef __MRMS
      MRMSQueryJob(J,TaskList,&TaskCount);
#endif /* __MRMS */

      J->TasksRequested = TaskCount;
      RQ->TaskCount     = TaskCount;
      }

    if (J->TasksRequested == 0)
      {
      int NC;

      DBG(2,fPBS) DPrint("ALERT:    no job task info located for job '%s' (assigning taskcount to 1)\n",
        J->Name);

      /* assume homegeneous nodes */

      /* assume dedicated nodes */

      NC = (MNode[0] != NULL) ? MNode[0]->CRes.Procs : 1;

      J->TasksRequested = NC;
      RQ->TaskCount     = NC;

      RQ->TasksPerNode  = NC;

      J->NodesRequested = 1;
      RQ->NodeCount     = 1;
      }
    }    /* END if (R->SubType == mrmstRMS) */

  MPBSJobAdjustResources(J,&TA,&MRM[RMIndex]);

  /* validate job */
 
  if (J->TasksRequested == 0)
    {
    DBG(2,fPBS) DPrint("ALERT:    no job task info located for job '%s' (assigning taskcount to 1)\n",
      J->Name);
 
    J->TasksRequested    = 1;
    J->Req[0]->TaskCount = 1;
    }
 
  if ((TA.WCLimit == 0) && (J->CPULimit > 0))
    {
    J->SpecWCLimit[0] = J->CPULimit / J->TasksRequested;
    J->SpecWCLimit[1] = J->SpecWCLimit[0];
    }

  if (J->Cred.G == NULL)
    {
    MGroupAdd(DEFAULT,&J->Cred.G);
    }

  /* authenticate job */

  if ((J->Cred.U == NULL) || (J->Cred.G == NULL))
    {
    DBG(1,fPBS) DPrint("ERROR:    cannot authenticate job '%s' (User: %s  Group: %s)\n",
      J->Name,
      (J->Cred.U != NULL) ? J->Cred.U->Name : "NULL",
      (J->Cred.G != NULL) ? J->Cred.G->Name : "NULL");

    MJobRemove(J);

    return(FAILURE);
    }

  if (J->Cred.A != NULL)
    strcpy(AName,J->Cred.A->Name);
  else
    AName[0] = '\0';

  if (MJobSetCreds(J,J->Cred.U->Name,J->Cred.G->Name,AName) == FAILURE)
    {
    DBG(1,fPBS) DPrint("ERROR:    cannot authenticate job '%s' (U: %s  G: %s  A: '%s')\n",
      J->Name,
      J->Cred.U->Name,
      J->Cred.G->Name,
      AName);

    MJobRemove(J);

    return(FAILURE);
    }

  if ((MQOSGetAccess(J,J->QReq,NULL,&QDef) == FAILURE) ||
      (J->QReq == NULL))
    {
    MJobSetQOS(J,QDef,0);
    }
  else
    {
    MJobSetQOS(J,J->QReq,0);
    }

  /* set defaults for info not specified by PBS */

  /* determine job start time */

  if (MJOBISALLOC(J) && (J->StartTime == 0))
    {
    J->StartTime = MSched.Time;

    if (TA.WallTime > 0)
      J->StartTime -= TA.WallTime;

    /* job start must be later than eligible time */

    if (TA.ETime > 0)
      J->StartTime = MAX(J->StartTime,TA.ETime);

    /* job start must be earlier than most recent modify time */

    if (TA.MTime > 0)
      J->StartTime = MIN(J->StartTime,TA.MTime);

    J->StartTime = MIN(J->StartTime,MSched.Time);     

    J->DispatchTime = J->StartTime;
    }

  J->SubmitTime = MIN(J->SubmitTime,MSched.Time);

  J->ExecSize  = 0;
  J->ImageSize = 0;

  MUStrDup(&J->E.IWD,NONE);
  MUStrDup(&J->E.Cmd,NONE);

  MLocalJobInit(J);

  return(SUCCESS);
  }    /* END MPBSJobLoad() */




int MPBSJobUpdate(

  struct batch_status *PJob,     /* I */
  mjob_t              *J,        /* I/O (modified) */
  short               *TaskList, /* O */
  int                  RMIndex)  /* I */

  {
  int           OldState;

  char          tmpBuffer[MAX_MBUFFER];

  int           TaskCount;
  int           NodeCount;
  int           TPN;
  int           NCPUs;

  struct attrl *AP;

  char         *tail;

  mreq_t        *RQ;

  long          tmpL;

  long          ETime;
  long          MTime;
  long          WallTime;
  long          WCLimit;

  long          CPUTime;
  long          CPUPercent;

  int           MaxJobMem;
  int           MaxJobSwap;

  mulong        tmpUL;

  const char *FName = "MPBSJobUpdate";

  DBG(2,fPBS) DPrint("%s(%s,%s,TaskList,%d)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (PJob != NULL) ? PJob->name : "NULL",
    RMIndex);

  if ((J == NULL) || (PJob == NULL))
    {
    return(FAILURE);
    }

  MTRAPJOB(J,FName);

  OldState = J->State;

  TaskList[0] = -1;

  if (__MPBSJobGetState(PJob,&MRM[RMIndex],NULL,&J->State,&tmpUL) == FAILURE)
    {
    DBG(1,fPBS) DPrint("ALERT:    cannot get job state info for job '%s'\n",
      J->Name);

    return(FAILURE);
    }

  J->IFlags |= tmpUL;

  RQ = J->Req[0];

  TaskCount = 1;
  NodeCount = 1;
  TPN       = 1;
  NCPUs     = 1;

  ETime     = 0;
  MTime     = 0;
  WallTime  = 0;
  WCLimit   = 0;

  CPUTime    = 0;
  CPUPercent = 0;

  MaxJobMem  = 0;
  MaxJobSwap = 0;

  if (J->State != OldState)
    {
    DBG(1,fPBS) DPrint("INFO:     job '%s' changed states from %s to %s\n",
      J->Name,
      MJobState[OldState],
      MJobState[J->State]);

    J->MTime = MSched.Time;
    }

  /* get job qtime, comment, uname, gname, account, task/feature request, */
  /*   arch, memory, swap, and disk info from PBS server */

  for (AP = PJob->attribs;AP != NULL;AP = AP->next)
    {
    if (!strcmp(AP->name,ATTR_qtime))
      {
      /* get queuetime (epochtime) */

      J->SubmitTime = strtol(AP->value,NULL,10);
      }
    else if (!strcmp(AP->name,ATTR_etime))
      {
      /* get runnable time (epochtime) (ie, hold released) */

      ETime = strtol(AP->value,NULL,10);  

      J->SystemQueueTime = MAX(J->SystemQueueTime,ETime);
      }
    else if (!strcmp(AP->name,ATTR_mtime))
      {
      /* get modify time (epochtime) (ie, job started or qaltered) */
 
      MTime = strtol(AP->value,NULL,10);
      }          
    else if (!strcmp(AP->name,"session_id"))
      {
      /* get job session ID */

      J->SessionID = (int)strtol(AP->value,NULL,10);
      }
    else if (!strcmp(AP->name,"exec_host"))
      {
      /* load job node list */

      if ((J->State != mjsStarting) && (J->State != mjsRunning))
        {
        continue;
        }

      strcpy(tmpBuffer,AP->value);

      __MPBSGetTaskList(J,tmpBuffer,TaskList,TRUE);
      }
    else if (!strcmp(AP->name,ATTR_comment))
      {
      /* get comment */

      /* not used */
      }
    else if (!strcmp(AP->name,ATTR_used))
      {
      if (!strcmp(AP->resource,"walltime"))
        {
        WallTime = MPBSGetResKVal(AP->value);  

        DBG(4,fPBS) DPrint("INFO:     walltime for job %s in state %s: %ld (%s)\n",
          J->Name,
          MJobState[J->State],
          WallTime,
          AP->value);
        }
      else if (!strcmp(AP->resource,"cpupercent"))
        {
        if ((J->State == mjsStarting) || (J->State == mjsRunning))
          {
          CPUPercent = strtol(AP->value,NULL,10);
          }
        }
      else if (!strcmp(AP->resource,"mem"))
        {
        if ((J->State == mjsStarting) || (J->State == mjsRunning))
          {
          RQ->URes.Mem = MPBSGetResKVal(AP->value) >> 10;
          }
        }
      else if (!strcmp(AP->resource,"vmem"))
        {
        if ((J->State == mjsStarting) || (J->State == mjsRunning))
          {
          RQ->URes.Swap = MPBSGetResKVal(AP->value) >> 10;
          }
        }
      else if (!strcmp(AP->resource,"cput"))
        {
        if ((J->State == mjsStarting) || (J->State == mjsRunning) || (J->State == mjsCompleted))
          {
          CPUTime = MPBSGetResKVal(AP->value);
          }
        }
      }
    else if (!strcmp(AP->name,ATTR_a))
      {
      long tmpL;

      /* get start date */

      tmpL = strtol(AP->value,&tail,10);

      if ((tail != NULL) && (!isspace(*tail)))
        tmpL = 0;

      MJobSetAttr(J,mjaReqSMinTime,(void **)&tmpL,0,mSet);  
      }
    else if (!strcmp(AP->name,ATTR_l))
      {
      /* required resources may be modified via 'qalter' */

      if (!strcmp(AP->resource,"walltime"))
        {
        WCLimit = MPBSGetResKVal(AP->value);  

        if (WCLimit != J->SpecWCLimit[1])
          {
          if (J->R != NULL)
            {
            /* adjust reservation walltime */

            /* FIXME:  resource increases must be 'approved' */

            if (WCLimit < J->SpecWCLimit[0])
              {
              J->R->EndTime = J->R->StartTime + WCLimit;
              }
            }

          J->SpecWCLimit[0] = WCLimit;
          J->SpecWCLimit[1] = WCLimit;
          }
        }
      else if (!strcmp(AP->resource,"pmem"))
        {
        /* Maximum 'per processor' real memory allowed */

        tmpL = (MPBSGetResKVal(AP->value) >> 10);

        RQ->DRes.Mem = tmpL;

        if (tmpL != RQ->RequiredMemory)
          {
          RQ->RequiredMemory = tmpL;
          RQ->MemCmp         = mcmpGE;
          }
        }
      else if (!strcmp(AP->resource,"pvmem"))
        {
        tmpL = (MPBSGetResKVal(AP->value) >> 10);

        RQ->DRes.Swap = tmpL;

        if (tmpL != RQ->RequiredMemory)
          {
          RQ->RequiredSwap = tmpL;
          RQ->SwapCmp      = mcmpGE;
          }
        }
      else if (!strcmp(AP->resource,"mem"))
        {
        MaxJobMem = (MPBSGetResKVal(AP->value) >> 10);
        }
      else if (!strcmp(AP->resource,"vmem"))
        {
        MaxJobSwap = (MPBSGetResKVal(AP->value) >> 10);
        }
      else if (!strcmp(AP->resource,"file"))
        {
        /* NOTE:  treat file as consumable disk and required disk */

        tmpL = (MPBSGetResKVal(AP->value) >> 10);

        RQ->DRes.Disk = tmpL;

        if (tmpL != RQ->RequiredDisk)
          {
          RQ->RequiredDisk = tmpL;
          RQ->DiskCmp      = mcmpGE;
          }
        }
      else if (!strcmp(AP->resource,"nodect"))
        {
        NodeCount = (int)strtol(AP->value,NULL,10);
 
        J->TasksRequested = MAX(J->TasksRequested,NodeCount);

        if (J->Req[1] == NULL)
          {
          RQ->TaskCount = MAX(RQ->TaskCount,NodeCount);
          }
        }
      else if (!strcmp(AP->resource,"nodes"))
        {
        strcpy(tmpBuffer,AP->value);

        __MPBSGetTaskList(J,tmpBuffer,TaskList,FALSE);
        }            
      else if (!strcmp(AP->resource,"ncpus"))
        {
        int PReq;

        if (!getenv("MAUIIGNNCPUS"))
          {
          NCPUs = (int)strtol(AP->value,NULL,10);

          PReq = RQ->DRes.Procs * J->TasksRequested;

          if (NCPUs != PReq)
            {
            if (J->NodesRequested <= 1)
              {
              J->TasksRequested = 1;
              RQ->TaskCount     = 1;
              J->NodesRequested = 1;

              RQ->DRes.Procs    = NCPUs;
              }
            else
              {
              RQ->DRes.Procs    = 1;

              J->TasksRequested = NCPUs;
              RQ->TaskCount     = NCPUs;
              }

            if (J->R != NULL)
              {
              /* adjust reserved procs */

              /* FIXME:  resource increases must be 'approved' */

              if (NCPUs < J->R->DRes.Procs)
                { 
                J->R->DRes.Procs = NCPUs;
                }
              else
                {
                /* increase proc allocation */

                /* NYI */
                }
              }
            }
          }      /* END if (NCPUs != PReq) */
        }
      else if (!strcmp(AP->resource,"cput"))
        {
        tmpL = MPBSGetResKVal(AP->value);

        if (tmpL != J->CPULimit)
          {
          J->CPULimit = tmpL;
          } 
        }
      else if (!strcmp(AP->resource,"mem"))
        {
        tmpL = (MPBSGetResKVal(AP->value) >> 10);

        if (tmpL != RQ->DRes.Mem)
          {
          RQ->DRes.Mem = tmpL;

          if (J->R != NULL)
            {
            /* adjust reserved memory */

            /* FIXME:  resource increases must be 'approved' */

            if (tmpL < J->R->DRes.Mem)
              {
              J->R->DRes.Mem = tmpL;
              }
            }
          }
        }
      else if (!strcmp(AP->resource,"software"))
        {
        /* NOTE:  old hack (map software to node feature */

        /* MReqSetAttr(J,RQ,mrqaReqNodeFeature,(void **)AP->value,mdfString,mAdd); */

        /* NOTE:  software handled at job load time, no support for dynamic software spec */

        /* Food for further ruminations:

            * software licenses can be either floating or node-locked

            * the above works in the situation of a node-locked license
               for unlimited users; limiting # of concurrent uses could
              be accomplished by forcing users to submit to a specific
               queue/class and limit the number of concurrent jobs in
              that class

            * one can imagine future support looking something like this (from the POV
              of the config file):

              # Node-locked on a single host, unlimited concurrent usage
               SOFTWARECFG[pkg1] HOSTLIST=node01

              # Node-locked on a single host, limited to one concurrent use
               SOFTWARECFG[pkg2] HOSTMAXCOUNT=1 HOSTLIST=node02

               # Floating across several hosts, global maximum on concurrent usage
               SOFTWARECFG[pkg3] MAXCOUNT=5 HOSTLIST=node[1-4][0-9]

              # Floating across several hosts, global and per-host maxima on concurrent usage
              SOFTWARECFG[pkg4] MAXCOUNT=10 HOSTMAXCOUNT=2 HOSTLIST=node[5-8][0-9]

            * this would probably also require support in diagnose ("diagnose -S",
              maybe?)
        */
        }
      else
        {
        /* host     (master node)   not yet supported for PBS */
        }
      }
    else if (!strcmp(AP->name,ATTR_queue))
      {
      MReqSetAttr(J,RQ,mrqaReqClass,(void **)AP->value,mdfString,mSet);
      }
    }    /* END for (AP) */

  if ((WCLimit == 0) && (J->CPULimit > 0))
    {
    J->SpecWCLimit[0] = J->CPULimit / J->TasksRequested;
    J->SpecWCLimit[1] = J->SpecWCLimit[0];
    }

  /* determine job start time */
 
  if (MJOBISALLOC(J) && (J->StartTime <= 0))
    {
    J->StartTime = MSched.Time;
 
    if (WallTime > 0)
      J->StartTime -= WallTime;
 
    if (ETime > 0)
      J->StartTime = MAX(J->StartTime,ETime);
 
    if (MTime > 0)
      J->StartTime = MIN(J->StartTime,MTime);

    J->StartTime = MIN(J->StartTime,MSched.Time);     
 
    J->DispatchTime = J->StartTime;
    }  /* END if (MJOBISALLOC(J) && ...) */                 

  if (MRM[RMIndex].SubType == mrmstRMS)
    {
    /* determine RMS job ID and obtain RMS specific info for job */

    if (MJOBISACTIVE(J))
      {
      int TaskCount = 1;

#ifdef __MRMS
      MRMSQueryJob(J,TaskList,&TaskCount);
#endif /* __MRMS */

      J->TasksRequested = TaskCount;
      RQ->TaskCount     = TaskCount;
      }
    }

  DBG(4,fPBS) DPrint("INFO:     job %s starttime: %ld (%s)  presenttime: %ld  wclimit: %ld  mtime: %ld  etime: %ld  walltime: %ld  state: %s\n",
    J->Name,
    J->StartTime,
    MULToTString(MSched.Time - J->StartTime),
    MSched.Time,
    J->SpecWCLimit[0],
    MTime,
    ETime,
    WallTime,
    MJobState[J->State]);

  /* adjust resource requirement info */
  /* NOTE:  PBS specifies resource requirement info on a 'per node' basis */
 
  if ((TaskCount > 1) && (TPN > 1) && (NodeCount <= 1))
    {
    NodeCount = TaskCount / TPN;
    }

  if ((NodeCount > 1) && (NCPUs > 1))
    RQ->DRes.Procs = 1;

  /* NOTE:  on some systems, PBS is not aware of cputime consumption on non-master nodes */
  /*        on these systems, CPUTime should be scaled 'assuming' master node usage is   */
  /*        identical to usage on all other nodes                                        */

#ifdef __LINUX
  CPUTime *= RQ->TaskCount;
#endif /* __LINUX */

  if (RQ->TaskCount > 0)
    {   
    /* adjust 'per task' statistics */

    if (CPUPercent > 0)
      {
      CPUPercent /= RQ->TaskCount;
      }

    if ((CPUTime > 0) && (WallTime > 0))
      {
      CPUTime = (int)(CPUTime * 100.0 / RQ->TaskCount);
      }
    else
      {
      CPUTime = 1;
      }

    /* adjust 'per task limits */

    if (MaxJobMem > 0)
      {
      /* set job wide dedicated resources */

      RQ->DRes.Mem = MAX(RQ->DRes.Mem,MaxJobMem / RQ->TaskCount);
      }

    if (RQ->URes.Mem > 0)
      {
      RQ->URes.Mem /= RQ->TaskCount;
      }

    if (MaxJobSwap > 0)
      {
      /* set both dedicated resources AND node requirements */

      RQ->DRes.Swap = MAX(RQ->DRes.Swap,MaxJobSwap / RQ->TaskCount);
      }
    }    /* END if (RQ->TaskCount > 0) */

  if (MJOBISACTIVE(J) || MJOBISCOMPLETE(J))  
    {
    J->AWallTime = MAX(J->AWallTime,WallTime);       

    /* CPU time previously adjusted to '100 * per task cpu consumption' */

    /* determine average 'short term' per task load */

    if (CPUPercent > 0)
      {
      RQ->URes.Procs = CPUPercent;
      }

    if ((CPUPercent < 5) && (CPUTime > 0.1) && (WallTime > 0))
      {
      /* PBS does not update 'cpupercent' (RQ->URes.Procs) quickly on some systems */       

      RQ->URes.Procs = MAX(RQ->URes.Procs,CPUTime / WallTime);
      }
    else if ((CPUPercent == 0) && (WallTime > 0))
      {
      RQ->URes.Procs = CPUTime / WallTime;
      }
    else 
      {
      RQ->URes.Procs = CPUTime / MAX(1,MSched.Time - J->StartTime);
      }

    RQ->RMWTime     = WallTime;         
    RQ->LURes.Procs = CPUTime;        

    if ((J->State == mjsCompleted) || (J->State == mjsRemoved))
      {
      if (J->CompletionTime <= J->StartTime)
        {
        J->CompletionTime = MIN(MSched.Time,J->StartTime + WallTime);
        }
      }
    }      /* END if (MJOBISACTIVE(J) || MJOBISCOMPLETE(J)) */

  {
  int rqindex;

  J->NodesRequested = 0;
  J->TasksRequested = 0;

  J->TaskCount = 0;
  J->NodeCount = 0;

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];

    J->NodesRequested += RQ->NodeCount;
    J->TasksRequested += RQ->TaskCount;

    if (MJOBISACTIVE(J))
      {
      J->TaskCount += RQ->TaskCount;
      J->NodeCount += RQ->NodeCount;
      }
    }
  }    /* END BLOCK */

  return(SUCCESS);
  }  /* END MPBSJobUpdate() */






long MPBSGetResKVal(

  char *String)  /* I */

  {
  long  val; 
  char *tail;

  if (String == NULL)
    {
    return(0);
    }

  /* currently drop milliseconds */

  val = strtol(String,&tail,10);

  if (*tail == ':')   /* time resource -> convert to seconds */
    {
    char *tail2;

    val *= 3600;                               /* hours   */

    val += (strtol(tail + 1,&tail2,10) * 60);  /* minutes */

    if (*tail2 == ':')
      val += strtol(tail2 + 1,&tail,10);       /* seconds */

    return(val);
    }

  if (val == 0)
    {
    return(val);
    }
 
  if (*tail != '\0' && *(tail + 1) == 'w')
    {
    /* adjust for word */

    val <<= 3;
    }

  /* NOTE:  round up to nearest MB */

  if (*tail == 'k')
    {
    return(MAX(1024,val));
    }
  else if (*tail == 'm')
    {
    return(MAX(1024,val << 10));
    }
  else if (*tail == 'g')
    {
    return(MAX(1024,val << 20));
    }
  else if (*tail == 't')
    {
    return(MAX(1024,val << 30));
    }
  else if (*tail == 'b')
    {
    return(MAX(1024,val >> 10));
    }
  else if(*tail == 'w')
    {
    return(MAX(1024,val >> 7));
    }

  return(MAX(1024,val));
  }  /* END MPBSGetResKVal() */




int MPBSGetClassInfo(

  mnode_t *N,
  char     CClass[][MAX_MNAME],
  char     AClass[][MAX_MNAME])

  {
  int cindex;
  mclass_t *C;

  int ConsumedClasses;

  memset(N->ARes.PSlot,0,sizeof(N->ARes.PSlot));
  memset(N->CRes.PSlot,0,sizeof(N->CRes.PSlot));

  /* update configured classes */

  if (CClass[0][0] == '\0')
    {
    return(FAILURE);
    }

  for (cindex = 0;CClass[cindex][0] != '\0';cindex++)
    {
    DBG(8,fPBS) DPrint("INFO:     configured class '%s' specified for node %s\n",
      CClass[cindex],
      N->Name);

    if (MClassAdd(CClass[cindex],&C) != FAILURE)
      {
      N->CRes.PSlot[C->Index].count++;
      N->CRes.PSlot[0].count++;
      }
    else
      {
      DBG(1,fPBS) DPrint("ALERT:    cannot add configured class '%s' to node %s\n",
        CClass[cindex],
        N->Name);
      }
    }    /* END for (cindex) */

  /* update available classes */

  for (cindex = 0;AClass[cindex][0] != '\0';cindex++)
    {
    DBG(8,fPBS) DPrint("INFO:     available class '%s' specified for node %s\n",
      AClass[cindex],
      N->Name);

    if (MClassAdd(AClass[cindex],&C) != FAILURE)
      {
      if (N->CRes.PSlot[C->Index].count > N->ARes.PSlot[C->Index].count)
        {
        N->ARes.PSlot[C->Index].count++;
        N->ARes.PSlot[0].count++;
        }
      else
        {
        DBG(1,fPBS) DPrint("ALERT:    class '%s' available but not configured\n",
          AClass[cindex]);
        }
      }
    else
      {
      DBG(1,fPBS) DPrint("ALERT:    cannot add available class '%s' to node %s\n",
        AClass[cindex],
        N->Name);
      }
    }    /* END for (cindex) */

  ConsumedClasses = 0;

  for (cindex = 1;cindex < MAX_MCLASS;cindex++)
    {
    ConsumedClasses += MAX(
      0,
      N->CRes.PSlot[cindex].count - N->ARes.PSlot[cindex].count);
    }  /* END for (cindex) */

  if (((N->State == mnsIdle) &&
      (MUNumListGetCount(MAX_PRIO_VAL,N->CRes.PSlot,N->ARes.PSlot,0,NULL) == FAILURE)) ||
      (MUNumListGetCount(MAX_PRIO_VAL,N->ARes.PSlot,N->CRes.PSlot,0,NULL) == FAILURE))
    {
    /* PBS corruption */

    DBG(1,fPBS) DPrint("ALERT:    %s node %s has class mismatch.  classes: %s\n",
      MAList[eNodeState][N->State],
      N->Name,
      MUCAListToString(N->ARes.PSlot,N->CRes.PSlot,NULL));

    if (MSched.Mode == msmNormal)
      {
      MOSSyslog(LOG_NOTICE,"node %s in state %s has class mismatch.  classes: %s\n",
        N->Name,
        MAList[eNodeState][N->State],
        MUCAListToString(N->ARes.PSlot,N->CRes.PSlot,NULL));
      }
    }

  /* adjust class by max procs */

  if (N->AP.HLimit[mptMaxProc][0] > 0)
    {
    N->CRes.PSlot[0].count =
      MIN(
        N->CRes.PSlot[0].count,
        N->AP.HLimit[mptMaxProc][0]);

    if ((N->CRes.PSlot[0].count - ConsumedClasses) > 0)
      {
      N->ARes.PSlot[0].count =
        MIN(
          N->ARes.PSlot[0].count,
          N->CRes.PSlot[0].count - ConsumedClasses);
      }
    }

  return(SUCCESS);
  }  /* END MPBSGetClassInfo() */




int MPBSJobRequeue(

  mjob_t  *J,     /* I (state modified) */
  mrm_t   *R,     /* I */
  mjob_t **JPeer, /* I */
  char    *Msg,   /* O */
  int     *SC)    /* O */

  {
  int   rc;

  char *ErrMsg;

  char tmpJobName[MAX_MNAME];

  const char *FName = "MPBSJobRequeue";

  DBG(2,fPBS) DPrint("%s(%s,R,JPeer,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");
 
  if ((J == NULL) || 
     ((J->State != mjsStarting) && (J->State != mjsRunning)))
    {
    return(FAILURE);
    }

  MJobGetName(J,NULL,R,tmpJobName,sizeof(tmpJobName),mjnRMName);   

  rc = pbs_rerunjob(R->U.PBS.ServerSD,tmpJobName,NULL);
 
  if (rc != 0)
    {
    ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);
 
    DBG(0,fPBS) DPrint("ERROR:    PBS job '%s' cannot be requeued (rc: %d  '%s')\n",
      J->Name,
      rc,
      ErrMsg);

    if (R->FailIteration != MSched.Iteration)
      {
      R->FailIteration = MSched.Iteration;
      R->FailCount     = 0;
      }
 
    R->FailCount++;
 
    return(FAILURE);
    }

  /* job successfully requeued */

  DBG(7,fPBS) DPrint("INFO:     job '%s' requeued\n",
    J->Name);

  /* restore PBS 'neednodes' value to original value */

  if (MPBSJobModify(J,R,ATTR_l,"neednodes",J->NeedNodes,NULL,NULL) == FAILURE)
    {
    DBG(7,fPBS) DPrint("WARNING:  cannot reset hostlist for job '%s')\n",
      J->Name);
    }
  else
    {
    DBG(7,fPBS) DPrint("INFO:     hostlist for job '%s' set to '%s'\n",
      J->Name,
      (J->NeedNodes != NULL) ? J->NeedNodes : "NULL");
    }

  return(SUCCESS);
  }  /* END MPBSJobRequeue() */




int MPBSJobCkpt(
 
  mjob_t  *J,    /* I (modified) */
  mrm_t   *R,    /* I */
  mbool_t  DoTerminateJob, /* I (boolean) */
  char    *Msg,  /* O (optional) */
  int     *SC)   /* O (optional) */
 
  {
  int   rc;
 
  char *ErrMsg;
 
  char tmpJobName[MAX_MNAME];
 
  const char *FName = "MPBSJobCkpt";
 
  DBG(2,fPBS) DPrint("%s(%s,R,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");
 
  if ((J == NULL) ||
      (R == NULL) ||
     ((J->State != mjsStarting) && (J->State != mjsRunning)))
    {
    return(FAILURE);
    }
 
  MJobGetName(J,NULL,R,tmpJobName,sizeof(tmpJobName),mjnRMName);
 
  rc = pbs_holdjob(R->U.PBS.ServerSD,tmpJobName,"s",NULL);
 
  if (rc != 0)
    {
    ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);
 
    DBG(0,fPBS) DPrint("ERROR:    PBS job '%s' cannot be checkpointed (rc: %d  '%s')\n",
      J->Name,
      rc,
      ErrMsg);
 
    if (R->FailIteration != MSched.Iteration)
      {
      R->FailIteration = MSched.Iteration;
      R->FailCount     = 0;
      }
 
    R->FailCount++;
 
    return(FAILURE);
    }

  rc = pbs_rlsjob(R->U.PBS.ServerSD,tmpJobName,"s",NULL);
 
  if (rc != 0)
    {
    ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);
 
    DBG(0,fPBS) DPrint("ERROR:    PBS job '%s' cannot be released from hold (rc: %d  '%s')\n",
      J->Name,
      rc,
      ErrMsg);
 
    if (R->FailIteration != MSched.Iteration)
      {
      R->FailIteration = MSched.Iteration;
      R->FailCount     = 0;
      }
 
    R->FailCount++;
 
    return(FAILURE);
    }

  /* NOTE:  'DoTerminateJob' flag not supported */

  DBG(7,fPBS) DPrint("INFO:     job '%s' checkpointed\n",
    J->Name);
 
  return(SUCCESS);
  }  /* END MPBSJobCkpt() */





int __MPBSGetTaskList(

  mjob_t *J,            /* I */
  char   *TaskString,   /* I */
  short  *TaskList,     /* O */
  int     IsExecList)   /* I: Boolean */

  {
  int     index;
  int     tindex;

  char   *ptr;
  char   *ptr2;

  char   *host;

  char   *TokPtr;
  char   *TokPtr2;

  mnode_t *N;
  char    tmpHostName[MAX_MNAME];

  int     ppn;

  int     tmpTaskCount;
  char   *tail;

  mreq_t *RQ;
  int     rqindex;

  int     TCSet;
  int     HLSet;
  int     PPNSet;

  short  *TLPtr;

  short   tmpTaskList[MAX_MNODE];

  /* FORMAT:  <HOSTNAME>[:ppn=<X>][<HOSTNAME>[:ppn=<X>]]... */
  /* or       <COUNT>[:ppn=<X>]                             */

  const char *FName = "__MPBSGetTaskList";

  DBG(5,fPBS) DPrint("%s(%s,%s,%s,%d)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (TaskString != NULL) ? TaskString : "NULL",
    (TaskList != NULL) ? "TaskList" : "NULL",
    IsExecList);

  if ((J == NULL) || (TaskString == NULL))
    {
    return(FAILURE);
    }

  ptr = MUStrTok(TaskString,"+ \t",&TokPtr);

  tindex = 0;

  /* NOTE:  only handles one 'ppn' distribution per job */

  ppn = 1;
  rqindex = 0;

  if (TaskList != NULL)
    {
    TLPtr = TaskList;
    }
  else
    {
    TLPtr = tmpTaskList;
    }

  RQ = J->Req[0];

  RQ->TaskCount = 0;

  TCSet = FALSE;
  HLSet = FALSE;
  PPNSet = FALSE;

  /* clear requested node features */

  if ((MJOBISACTIVE(J) == FALSE) && (IsExecList == FALSE))
    {
    MReqSetAttr(
      J,
      RQ,
      mrqaReqNodeFeature,
      (void **)NULL,
      mdfString,
      mSet);
    }

  while (ptr != NULL)
    {
    /* FORMAT:  <COUNT>[ :{[ppn=<X>] | [<FEATURE>]}] */
    /*       or <HOSTNAME>[:ppn=<X>]                 */

    ppn = 1;

    strcpy(tmpHostName,ptr);

    host = MUStrTok(tmpHostName,":",&TokPtr2);  

    /* remove virtual host id */

    if ((tail = strchr(tmpHostName,'/')) != NULL)
      *tail = '\0';

    tmpTaskCount = (int)strtol(tmpHostName,&tail,10);
 
    if ((tmpTaskCount != 0) && (*tail == '\0'))
      {
      if (TCSet == TRUE)
        {
        /* new job req specified */

        rqindex++;

        if (J->Req[rqindex] == NULL)
          {
          MReqCreate(J,NULL,NULL,FALSE);
 
          MRMReqPreLoad(J->Req[rqindex]);
          }

        RQ = J->Req[rqindex];
 
        RQ->TaskCount = 0;

        HLSet = FALSE;
        TCSet = FALSE;
        PPNSet = FALSE;
        }
      }    /* END if ((tmpTaskCount != 0) && (*tail == '\0')) */

    /* check for ppn/feature info */

    ptr2 = MUStrTok(NULL,":",&TokPtr2);

    while (ptr2 != NULL)
      {
      if (strncmp(ptr2,"ppn=",strlen("ppn=")) == 0)
        {
        ppn = (int)strtol(ptr2 + strlen("ppn="),NULL,10);

        PPNSet = TRUE;
        }
      else
        {
        /* feature requirement specified */

        MReqSetAttr(J,RQ,mrqaReqNodeFeature,(void **)ptr2,mdfString,mAdd);
        }

      ptr2 = MUStrTok(NULL,":",&TokPtr2);      
      }

    /* determine hostname/nodecount */

    if ((tmpTaskCount != 0) && (*tail == '\0'))
      {
      /* nodecount specified */

      if (PPNSet == TRUE)
        RQ->TasksPerNode = ppn;

      RQ->TaskCount += tmpTaskCount * MAX(1,RQ->TasksPerNode);

      TCSet = TRUE;
      }
    else
      { 
      if (MNodeFind(tmpHostName,&N) == FAILURE)
        {
        DBG(1,fPBS) DPrint("ALERT:    cannot locate host '%s' for job hostlist\n",
          tmpHostName);

        return(FAILURE);
        }

      /* load task list info */

      for (index = 0;index < ppn;index++)
        {
        TLPtr[tindex] = N->Index;

        tindex++;
        }

      RQ->TaskCount += ppn;   

      HLSet = TRUE;
      }  /* END else ((tmpTaskCount != 0) && (*tail == '\0')) */

    ptr = MUStrTok(NULL,"+ \t",&TokPtr);
    }    /* END while (ptr != NULL) */

  TLPtr[tindex] = -1;

  if ((tindex > 0) && (HLSet == TRUE) && (IsExecList == FALSE))
    {
    /* host list specified */

    if (J->ReqHList == NULL)
      {
      J->ReqHList = (mnalloc_t *)calloc(
        1,
        sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1));
      }

    if ((J->ReqHList == NULL) ||
        (MUNLFromTL(J->ReqHList,TLPtr,NULL) == FAILURE))
      {
      DBG(7,fPBS) DPrint("ALERT:    cannot allocate hostlist for job %s\n",
        J->Name);

      return(FAILURE);
      }

    J->SpecFlags |= (1 << mjfHostList);
  
    DBG(7,fPBS) DPrint("INFO:     required hostlist set for job %s (%s)\n",
      J->Name,
      J->ReqHList[0].N->Name);
    }

  DBG(7,fPBS) DPrint("INFO:     %d host task(s) located for job\n",
    tindex);

  if (rqindex > 0)
    {
    DBG(7,fPBS) DPrint("INFO:     %d task(s) located for job req\n",
      J->Req[0]->TaskCount);
    }

  if (IsExecList == FALSE)
    {
    static mjob_t  *OldJ = NULL;
    static mbool_t  OldJHLSet = FALSE;

    if (HLSet == FALSE)
      {
      /* NOTE:  must handle 'neednodes' specification of hostlist
       *        *  (ie, neednodes specifies hostlist, nodes does not */

      if ((J == OldJ) && (OldJHLSet == FALSE))
        {
        if (J->SpecFlags & (1 << mjfHostList))
          J->SpecFlags ^= (1 << mjfHostList);

        MUFree((char **)&J->ReqHList);
        }
      }  /* END if (HLSet == FALSE) */
    else if (TCSet == TRUE)
      {
      J->ReqHLMode = mhlmSubset;
      }

    if (J == OldJ)
      {
      if (HLSet == TRUE)
        OldJHLSet = TRUE;
      }
    else
      {
      OldJ = J;
      OldJHLSet = HLSet;
      }
    }    /* END if (IsExecList == FALSE) */

  return(SUCCESS);
  }    /* END __MPBSGetTaskList() */




int MPBSJobMigrate(

  mjob_t    *J,    /* I (required) */
  mrm_t     *R,    /* I */
  mnalloc_t *NL,   /* I */
  char      *EMsg, /* O (optional) */
  int       *SC)   /* O (optional) */

  {
  char  tmpHList[MAX_MBUFFER];

  if ((J == NULL) || (R == NULL) || (NL == NULL))
    {
    return(FAILURE);
    }

  if (!(J->Flags & (1 < mjfRestartable)))
    {
    return(FAILURE);
    } 
    
  if (MRMFunc[R->Type].JobCheckpoint != NULL)
    {
    /* checkpoint job */

    if (MPBSJobCkpt(J,R,TRUE,EMsg,SC) == FAILURE)
      {
      return(FAILURE);
      }
    }
  else
    {
    /* requeue job */

    if (MPBSJobRequeue(J,R,NULL,EMsg,SC) == FAILURE)
      {
      return(FAILURE);
      }
    }  /* END else (MRMFunc[R->Type].JobCheckpoint) != NULL) */

  /* create new hostlist */

  __MPBSNLToTaskString(
    NL,
    R,
    tmpHList,
    sizeof(tmpHList));

  /* modify job */

  if (MPBSJobModify(
       J,
       R,
       "Resource_List",
       "neednodes",
       tmpHList,
       EMsg,
       SC) == FAILURE)
    {
    /* clean up */

    return(FAILURE);
    }

  if (MPBSJobStart(J,R,EMsg,SC) == FAILURE)
    {
    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MPBSJobMigrate() */




int MPBSJobModify(
 
  mjob_t *J,        /* I (required) */
  mrm_t  *R,        /* I */ 
  char   *Name,     /* I */
  char   *Resource, /* I */
  char   *Value,    /* I (optional) */
  char   *EMsg,     /* O (optional) */
  int    *SC)       /* O (optional) */
 
  {
  int rc;

  char *RPtr;

  char  tmpLine[MAX_MLINE]; 
  char  tmpJName[MAX_MNAME];
  char *ErrMsg;
 
  struct attrl PBSAttr;

  char *FName = "MPBSJobModify";

  DBG(1,fPBS) DPrint("%s(%s,%s,%s,%s)\n",
    FName,
    (J    != NULL) ? J->Name : "NULL",
    (Name != NULL) ? Name : "NULL",
    "Resource",
    (Value != NULL) ? Value : "NULL");

  if ((J == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  tmpLine[0] = '\0';

  RPtr = (Resource != NULL) ? Resource : tmpLine;
 
  PBSAttr.next     = NULL;
  PBSAttr.name     = Name;
  PBSAttr.resource = RPtr;
  PBSAttr.value    = Value;
  PBSAttr.op       = SET;
 
  MJobGetName(J,NULL,R,tmpJName,sizeof(tmpJName),mjnRMName);
 
  rc = pbs_alterjob(R->U.PBS.ServerSD,tmpJName,&PBSAttr,NULL);
 
  if (rc != 0)
    {
    ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);
 
    DBG(0,fPBS) DPrint("WARNING:  cannot set job '%s' attr '%s:%s' to '%s' (rc: %d '%s')\n",
      tmpJName,
      Name,
      RPtr,
      Value,
      rc,
      ErrMsg);
 
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MPBSJobModify() */




int MPBSJobStageData(

  mjob_t *J,  /* I */
  mrm_t  *R,  /* I */
  int    *SC) /* O (optional) */
 
  {
  int rc;
 
  char  tmpJName[MAX_MNAME];
  char *ErrMsg;

  char *Location;

  const char *FName = "MPBSJobStageData";
 
  DBG(1,fPBS) DPrint("%s(%s,%s,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");
 
  if (J == NULL)
    {
    return(FAILURE);
    }

  if ((J->State == mjsIdle) && (J->SIData != NULL))
    {
    Location = J->SIData->Location;
    }
  else if (((J->State == mjsCompleted) || (J->State == mjsRemoved)) &&
            (J->SOData != NULL))
    {
    Location = J->SOData->Location;
    }
  else
    {
    return(FAILURE);
    }

  MJobGetName(J,NULL,R,tmpJName,sizeof(tmpJName),mjnRMName);
 
  rc = pbs_stagein(R->U.PBS.ServerSD,tmpJName,Location,NULL);
 
  if (rc != 0)
    {
    ErrMsg = pbs_geterrmsg(R->U.PBS.ServerSD);
 
    DBG(0,fPBS) DPrint("ERROR:    cannot stage data for job '%s' location '%s' (rc: %d '%s')\n",
      J->Name,
      Location,
      rc,
      ErrMsg);
 
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MPBSJobStageData() */




int MPBSNodeSetAttr(

  mnode_t *N,    /* I (modified) */
  void    *A,    /* I */
  int      Mode) /* I */

  {
  struct attrl *AP;
  struct attrl  tmpAP;

  if ((N == NULL) || (A == NULL))
    {
    return(FAILURE);
    }

  memset(&tmpAP,0,sizeof(tmpAP));

  if (A != NULL)
    {
    AP = (struct attrl *)A;
    }

  DBG(6,fPBS) DPrint("INFO:     PBS node attribute '%s'  value: '%s'  (r: %s)\n",
    AP->name,
    (AP->value != NULL) ? AP->value : "NULL",
    (AP->resource != NULL) ? AP->resource : "NULL");

  if (!strcmp(AP->name,"resources_available"))
    {
    if (!strcmp(AP->resource,"arch"))
      {
      if (AP->value[0] != '?')
        N->Arch = MUMAGetIndex(eArch,AP->value,mAdd);
      }
    else if (!strcmp(AP->resource,"mem"))
      {
      N->CRes.Mem = (MPBSGetResKVal(AP->value) >> 10);

      N->ARes.Mem = MIN(N->ARes.Mem,N->CRes.Mem);
      }
    else if (!strcmp(AP->resource,"vmem"))
      {
      N->CRes.Swap = (MPBSGetResKVal(AP->value) >> 10);

      N->ARes.Swap = MIN(N->ARes.Swap,N->CRes.Swap);
      }
    else if (!strcmp(AP->resource,"ncpus"))
      {
      N->CRes.Procs = (int)strtol(AP->value,NULL,10);
      }
    }
  else if (!strcmp(AP->name,"license"))
    {
    /* NOTE: value 'l'??? */

    MNodeSetAttr(N,mnaGRes,(void **)AP->value,mdfString,mSet);
    }
  else if (!strcmp(AP->name,"pcpus"))
    {
    /* NYI */

    /* NOTE: value '<NCPUS>'??? */
    }
  else if (!strcmp(AP->name,"properties"))
    {
    char *ptr;
    char *TokPtr;

    ptr = MUStrTok(AP->value,", \t",&TokPtr);

    while (ptr != NULL)
      {
      MNodeProcessFeature(N,ptr);

      ptr = MUStrTok(NULL,", \t",&TokPtr);
      } /* END while (ptr != NULL) */
    }

  /* patch limited data */

  N->CRes.Mem = MAX(N->CRes.Mem,N->ARes.Mem);

  N->ARes.Swap = N->ARes.Mem;
  N->CRes.Swap = N->CRes.Mem;
 
  return(SUCCESS);
  }  /* END MPBSNodeSetAttr() */




int MPBSJobSetAttr(

  mjob_t  *J,        /* I (modified) */
  void    *A,        /* I */
  char    *ValLine,  /* I */
  tpbsa_t *TA,       /* O */
  short   *TaskList, /* O */
  int      Mode)     /* I */

  {
  struct attrl *AP;
  struct attrl  tmpAP;

  char tmpName[MAX_MNAME];
  char tmpResource[MAX_MNAME];
  char tmpValue[MAX_MLINE];

  mrm_t *R;

  mreq_t *RQ;

  if ((J == NULL) || ((A == NULL) && (ValLine == NULL)))
    {
    return(FAILURE);
    }

  memset(&tmpAP,0,sizeof(tmpAP));            

  R = &MRM[J->Req[0]->RMIndex];

  if (A != NULL)
    {
    AP = (struct attrl *)A;
    }
  else
    {
    char *ptr;
    char *TokPtr;

    /* FORMAT:  name,resource,value */

    AP = &tmpAP;

    AP->name     = tmpName;
    AP->resource = tmpResource;
    AP->value    = tmpValue;

    tmpResource[0] = '\0';

    if ((ptr = MUStrTok(ValLine,",",&TokPtr)) != NULL)
      {
      strcpy(AP->name,ptr);

      if ((ptr = MUStrTok(NULL,",",&TokPtr)) != NULL)
        {
        strcpy(AP->resource,ptr);
        }

      if ((ptr = MUStrTok(NULL,",",&TokPtr)) != NULL)
        {
        strcpy(AP->value,ptr);
        }
      }

    if ((AP->name == NULL) || (AP->value == NULL))
      return(FAILURE);
    }  /* END else (A != NULL) */

  RQ = J->Req[0];

  if (!strcmp(AP->name,ATTR_qtime))
    {
    /* get queuetime (epochtime) */
 
    J->SubmitTime = strtol(AP->value,NULL,10);
    }
  else if (!strcmp(AP->name,ATTR_etime))
    {
    /* get eligible time (epochtime) (ie, hold released) */
 
    TA->ETime = strtol(AP->value,NULL,10);
 
    J->SystemQueueTime = MAX(J->SystemQueueTime,TA->ETime);
    }
  else if (!strcmp(AP->name,ATTR_mtime))
    {
    /* get modify time (epochtime) (ie, job started or qalter'd) */
 
    TA->MTime = strtol(AP->value,NULL,10);
    }
  else if (!strcmp(AP->name,"session_id"))
    {
    /* get job session ID */

    J->SessionID = (int)strtol(AP->value,NULL,10);
    }
  else if (!strcmp(AP->name,"exec_host"))
    {
    /* load job node list */

    if (R->SubType != mrmstRMS)
      { 
      DBG(6,fPBS) DPrint("INFO:     processing job %s exechost list '%s'\n",
        J->Name,
        AP->value);
 
      __MPBSGetTaskList(J,AP->value,TaskList,TRUE);
      }
    }
  else if (!strcmp(AP->name,ATTR_r))
    {
    if ((AP->value != NULL) && (MUBoolFromString(AP->value,FALSE) == TRUE))
      J->SpecFlags |= (1 << mjfRestartable);
    }
  else if (!strcmp(AP->name,ATTR_comment))
    {
    /* get comment */

    /* ignore PBS comments for now */
    }
  else if (!strcmp(AP->name,"stagein"))
    {
    /* FORMAT:  <SRCFILE>[@<SRCHOST>]:<DSTFILE> */

    char *ptr;
    char *TokPtr;
 
    MSDataCreate(&J->SIData);
 
    J->SIData->Location = strdup(AP->value);
 
    ptr = MUStrTok(AP->value,":",&TokPtr);

    J->SIData->SrcFileName = strdup(ptr);
    J->SIData->SrcHostName = strdup(ptr);
 
    if (ptr != NULL)
      J->SIData->DstFileName = strdup(ptr);
    }
  else if  (!strcmp(AP->name,"stageout"))
    {
    /* FORMAT:  <SRCFILE>:<DSTFILE>[@<DSTHOST>] */
 
    char *ptr;
 
    MSDataCreate(&J->SOData);
 
    J->SOData->Location = strdup(AP->value);
 
    ptr = AP->value;
 
    J->SOData->SrcFileName = strdup(ptr);
    J->SOData->DstFileName = strdup(ptr);
 
    if (ptr != NULL)
      J->SOData->DstHostName = strdup(ptr);
    }
  else if (!strcmp(AP->name,"x"))
    {
    MJobSetAttr(J,mjaRMXString,(void **)AP->value,mdfString,0);

    if (MJobProcessExtensionString(J,J->RMXString) == FAILURE)
      {
      DBG(1,fPBS) DPrint("ALERT:    cannot process extension line '%s' for job %s (cancelling job)\n",
        (J->RMXString != NULL) ? J->RMXString : "NULL",
        J->Name);
 
      MRMJobCancel(J,"MAUI_ERROR:  cannot process extension line\n",NULL);
 
      MJobRemove(J);
 
      return(FAILURE);
      }
    }
  else if (!strcmp(AP->name,"qos"))
    {
    if (MQOSFind(AP->value,&J->QReq) == SUCCESS)
      {
      DBG(3,fCONFIG) DPrint("INFO:     QOS '%s' requested by job %s\n",
        J->QReq->Name,
        J->Name);
      }
    }
  else if (!strcmp(AP->name,"depend"))
    {
    char *ptr;
    char *TokPtr;

    enum MJobDependEnum DType;
    char *DValue;

    /* FORMAT: <TYPE>:JOBID */
 
    /* <TYPE>: after (job successfully started)           *
     *         afterok (job successfully terminated)      *
     *         afternotok (job terminated unsuccessfully) *
     *         before                                     *
     *         beforeok                                   *
     *         beforenotok                                *
     *         afterany (job terminated)                  */
 
    ptr = MUStrTok(AP->value,": \t\n",&TokPtr);

    DType  = mjdNONE;
    DValue = NULL;

    if ((ptr = MUStrTok(AP->value,": \t\n",&TokPtr)) == NULL)
      {
      return(FAILURE);
      }
 
    if (!strncmp(ptr,"before",strlen("before")))
      {
      /* ignore before dependencies for now */
      }
    else if (!strcmp(ptr,"after"))
      {
      DType = mjdJobStart;
      }
    else
      {
      DType = mjdJobCompletion;
      }
 
    ptr = MUStrTok(NULL,": @\t\n",&TokPtr);

    DValue = ptr;

    MJobSetDependency(J,DType,DValue);
    }
  else if (!strcmp(AP->name,ATTR_euser) ||
           !strcmp(AP->name,ATTR_owner))
    {
    /* get user name */

    char *ptr;
    char *TokPtr;
 
    /* FORMAT:  <USERNAME>[@<SERVERHOST>] */
 
    if ((ptr = MUStrTok(AP->value,"@ \t\n",&TokPtr)) == NULL)
      {
      DBG(1,fPBS) DPrint("ERROR:    cannot add user for job %s (invalid name)\n",
        J->Name);
 
      MJobRemove(J);
 
      return(FAILURE);
      }
 
    if (MUserAdd(ptr,&J->Cred.U) == FAILURE)
      {
      DBG(1,fPBS) DPrint("ERROR:    cannot add user for job %s (Name: %s)\n",
        J->Name,
        ptr);
 
      MJobRemove(J);
 
      return(FAILURE);
      }
    }
  else if (!strcmp(AP->name,ATTR_egroup))
    {
    /* get group name */

    if (MGroupAdd(AP->value,&J->Cred.G) == FAILURE)
      {
      DBG(1,fPBS) DPrint("ERROR:    cannot add group for job %s (Name: %s)\n",
        J->Name,
        AP->value);

      MJobRemove(J);

      return(FAILURE);
      }
    }
  else if (!strcmp(AP->name,ATTR_A))
    {
    /* get account name */

    if (MAcctAdd(AP->value,&J->Cred.A) == FAILURE)
      {
      DBG(1,fPBS) DPrint("ERROR:    cannot add account for job %s (Name: %s)\n",
        J->Name,
        AP->value);
      }
    }
  else if (!strcmp(AP->name,ATTR_used))
    {
    if (!strcmp(AP->resource,"walltime"))
      {
      TA->WallTime = MPBSGetResKVal(AP->value);
 
      RQ->RMWTime = TA->WallTime;
 
      if ((J->State == mjsStarting) || (J->State == mjsRunning))
        {
        J->AWallTime = RQ->RMWTime;
        }
      }
    else if (!strcmp(AP->resource,"cpupercent"))
      {
      if ((J->State == mjsStarting) || (J->State == mjsRunning))
        {
        RQ->URes.Procs = strtol(AP->value,NULL,10);
        }
      }
    else if (!strcmp(AP->resource,"mem"))
      {
      if ((J->State == mjsStarting) || (J->State == mjsRunning))
        {
        RQ->URes.Mem = MPBSGetResKVal(AP->value) >> 10;
        }
      }
    else if (!strcmp(AP->resource,"vmem"))
      {
      if ((J->State == mjsStarting) || (J->State == mjsRunning))
        {
        RQ->URes.Swap = MPBSGetResKVal(AP->value) >> 10;
        }
      }
    else if (!strcmp(AP->resource,"cput"))
      {
      if ((J->State == mjsStarting) ||
          (J->State == mjsRunning) ||
          (J->State == mjsCompleted))
        {
        TA->UtlJobCPUTime = MPBSGetResKVal(AP->value);
        }
      }
    }
  else if (!strcmp(AP->name,ATTR_l))
    {
    /* required resources */

    /* get resource information */

    if (!strcmp(AP->resource,"neednodes"))
      {
      /* record PBS neednodes value */

      MUStrDup(&J->NeedNodes,AP->value);
 
      /* check point or data stage job with specific node requirements */

      /* FORMAT:  <NODENAME>:ppn=<X>[+<NODENAME>:ppn=<X>]... */
 
      /*   or same as 'nodes' if no additional requirements  */
 
      __MPBSGetTaskList(J,AP->value,NULL,FALSE);
      }
    else if (!strcmp(AP->resource,"nodes"))
      {
      /* node specification */
 
      DBG(1,fPBS) DPrint("INFO:     processing node request line '%s'\n",
        AP->value);

      __MPBSGetTaskList(J,AP->value,NULL,FALSE);
 
      if ((J->Req[1] != NULL) && (MPar[0].EnableMultiReqJobs == FALSE))
        {
        /* must sync all req default settings */
 
        DBG(1,fPBS) DPrint("ALERT:    multi-req PBS job submitted (multi-req not allowed)\n");
 
        MJobSetHold(
          J,
          (1 << mhBatch),
          0,
          mhrPolicyViolation,
          "multi-req PBS jobs not allowed");
        }
      }    /* END if (!strcmp(AP->resource,"nodes")) */
    else if (!strcmp(AP->resource,"nodect"))
      {
      TA->NodesRequested = (int)strtol(AP->value,NULL,10);

      J->NodesRequested = TA->NodesRequested;
      J->TasksRequested = MAX(J->TasksRequested,J->NodesRequested);
      }
    else if (!strcmp(AP->resource,"arch"))
      {
      RQ->Arch = MUMAGetIndex(eArch,AP->value,mAdd);
      }
    else if (!strcmp(AP->resource,"pmem"))
      {
      /* NOTE: called 'workingset'? (indiana) */

      /* 'per processor' required memory */

      RQ->DRes.Mem = (MPBSGetResKVal(AP->value) >> 10);
      }
    else if (!strcmp(AP->resource,"pvmem"))
      {
      /* NOTE: called 'pmem'? (indiana) */

      /* 'per processor' required swap */

      RQ->DRes.Swap = (MPBSGetResKVal(AP->value) >> 10);
      }
    else if (!strcmp(AP->resource,"mem"))
      {
      /* 'per job' required memory */

      TA->JobMemLimit = (MPBSGetResKVal(AP->value) >> 10);
      }
    else if (!strcmp(AP->resource,"vmem"))
      {
      /* 'per job' required swap */

      TA->JobSwapLimit = (MPBSGetResKVal(AP->value) >> 10);
      }
    else if (!strcmp(AP->resource,"file"))
      {
      long tmpL;

      /* 'per job' required disk */

      /* NOTE:  treat file as consumable disk and required disk */

      tmpL = (MPBSGetResKVal(AP->value) >> 10);

      RQ->DRes.Disk = tmpL;

      if (tmpL != RQ->RequiredDisk)
        {
        RQ->RequiredDisk = tmpL;
        RQ->DiskCmp      = mcmpGE;
        }
      }
    else if (!strcmp(AP->resource,"walltime"))
      {
      TA->WCLimit = MPBSGetResKVal(AP->value);

      J->SpecWCLimit[0] = TA->WCLimit;
      J->SpecWCLimit[1] = TA->WCLimit;
      }
    else if (!strcmp(AP->resource,"ncpus") && (getenv("MAUIIGNNCPUS") == NULL))
      {
      TA->NCPUs = (int)strtol(AP->value,NULL,10);

      RQ->DRes.Procs = TA->NCPUs;   

      J->NodesRequested = 1;
      J->TasksRequested = 1;
 
      RQ->TaskCount = 1;
      RQ->NodeCount = 1;
      }
    else if (!strcmp(AP->resource,"host"))
      {
      /* used only on O2K systems */

      __MPBSGetTaskList(J,AP->value,NULL,FALSE);
      }
    else if (!strcmp(AP->resource,"cput"))
      {
      /* 'per job' CPU limit */

      TA->JobCPULimit = MPBSGetResKVal(AP->value);

      J->CPULimit = TA->JobCPULimit;
      }
    else if (!strcmp(AP->resource,"pcput"))
      {
      /* 'per processor' CPU limit */

      TA->ProcCPULimit = MPBSGetResKVal(AP->value);
      }
    else if (!strcmp(AP->resource,"rmsnodes"))
      {
      if (R->SubType == mrmstRMS)
        {
        char *ptr;
        char *tptr;

        char *TokPtr;

        DBG(3,fPBS) DPrint("INFO:     processing rmsnode line '%s'\n",
          AP->value);

        /* process RMS nodes info of the format <BASENODEID>/<NODECOUNT>:<TASKCOUNT> */

        if (strchr(AP->value,'/') != NULL)
          {
          /* base node is specified (NOTE:  should only occur if job is already running) */

          /* node layout info is obtained directly from RMS */
          }
        else
          {
          int TPN;
          int TaskCount;
          int NodeCount;

          if ((ptr = MUStrTok(AP->value,":",&TokPtr)) != NULL)
            {
            NodeCount = (int)strtol(ptr,NULL,10);

            /* NOTE:  assume proc/node homogeneous cluster */

            if ((tptr = MUStrTok(NULL,":",&TokPtr)) != NULL)
              {
              TaskCount = (int)strtol(tptr,NULL,10);

              TPN = NodeCount / TaskCount;
              }
            else
              {
              TPN = (MNode[0] != NULL) ? MNode[0]->CRes.Procs : 1;

              TaskCount = NodeCount * TPN;
              }

            J->TasksRequested = TaskCount;
            J->TaskCount      = TaskCount;
            RQ->TaskCount     = TaskCount;

            RQ->TasksPerNode  = TPN;

            RQ->NodeCount     = NodeCount;
            J->NodesRequested = NodeCount;

            TA->NodesRequested = NodeCount;
            }
          else
            {
            /* corrupt RMSNode ptr */
            }
          }
        }
      }
    else if (!strcmp(AP->resource,"rmspartition"))
      {
      if (R->SubType == mrmstRMS)
        {
        mpar_t *P;

        /* process RMS partition */

        if (MParAdd(AP->value,&P) == SUCCESS)
          {
          int tmpI;

          tmpI = MUMAFromString(ePartition,AP->value,mAdd);

          memcpy(&J->SpecPAL[0],&tmpI,sizeof(J->SpecPAL[0]));
          }
        }
      }
    else if (!strcmp(AP->resource,"software"))
      {
      int rqindex;

      int RIndex;

      mreq_t *tmpRQ;

      if ((RIndex = MUMAGetIndex(eGRes,AP->value,mAdd)) == 0)
        {
        /* cannot add support for generic res */

        DBG(1,fPBS) DPrint("ALERT:    cannot add support for GRes software '%s'\n",
          AP->value);
 
        return(FAILURE);
        }

      /* verify software req does not already exist */

      for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
        {
        if (J->Req[rqindex]->DRes.GRes[RIndex].count > 0)
          break;
        }  /* END for (rqindex) */

      if (J->Req[rqindex] != NULL)
        {
        /* software req already added */

        return(SUCCESS);
        }

      /* add software req */

      if (MReqCreate(J,NULL,&tmpRQ,FALSE) == FAILURE)
        {
        DBG(1,fPBS) DPrint("ALERT:    cannot add req to job %s for GRes software '%s'\n",
          J->Name,
          AP->value);

        return(SUCCESS);
        }

      /* NOTE:  PBS currently supports only one license request per job */

      tmpRQ->DRes.GRes[RIndex].count = 1;
      tmpRQ->DRes.GRes[0].count      = 1;
      tmpRQ->TaskCount               = 1;
      tmpRQ->NodeCount               = 1;
 
      /* NOTE:  prior workaround (map software to node feature */

      /* MReqSetAttr(J,RQ,mrqaReqNodeFeature,(void **)AP->value,mdfString,mAdd); */
      }
    else
      {
      /* host     (master node)   not yet supported for PBS */

      /* NO-OP */
      }
    }
  else if (!strcmp(AP->name,ATTR_a))
    {
    long tmpL;

    char *tail;
 
    /* get start date */
 
    tmpL = strtol(AP->value,&tail,10);
 
    if ((tail != NULL) && (!isspace(*tail)))
      tmpL = 0;
 
    MJobSetAttr(J,mjaReqSMinTime,(void **)&tmpL,mdfLong,mSet);
    }
  else if (!strcmp(AP->name,ATTR_queue))
    {
    MReqSetAttr(J,RQ,mrqaReqClass,(void **)AP->value,mdfString,mSet);
    }

  return(SUCCESS);
  }  /* END MPBSJobSetAttr() */




int MPBSJobAdjustResources(

  mjob_t  *J,  /* I/O (modified) */
  tpbsa_t *TA, /* I */
  mrm_t   *R)  /* I */

  {
  mreq_t *RQ;
  int     rqindex;

  if ((J == NULL) || (J->Req[0] == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  RQ = J->Req[0];

  /* handle ncpus/nodect adjustment */

  /* NOTE:  PBSPro 5.2.2 maps 'ncpus' directly to 'ppn' */

  if (TA != NULL)
    {
    if (R->U.PBS.PBS5IsEnabled == TRUE)
      {
      if (TA->NodesRequested > 1)
        {
        TA->NCPUs      = 1;
        RQ->DRes.Procs = 1; 

        if (RQ->NodeCount == 1)
          RQ->NodeCount = 0;
        }
      else if (RQ->TaskCount > 1)
        {
        TA->NCPUs = 1;
        RQ->DRes.Procs = 1;
        }
      }    /* END if (R->U.PBS.PBS5IsEnabled == TRUE) */

    if ((TA->NCPUs > 1) &&
       ((TA->NodesRequested > 1) || (RQ->TasksPerNode > 1)))
      {
      /* multi-node 'ncpu' specification detected */

      RQ->DRes.Procs    = 1;

      RQ->TasksPerNode  = TA->NCPUs;

      RQ->TaskCount     = TA->NCPUs;
      J->TasksRequested = TA->NCPUs;
      }
    }    /* END if (TA != NULL) */

  /* adjust req */

  if (RQ->TaskCount == 0)
    {
    RQ->TaskCount = J->TasksRequested;
    RQ->NodeCount = J->NodesRequested;
    }
 
  /* handle matching policies */
 
  if (MPar[0].JobNodeMatch & (1 << nmExactNodeMatch))
    {
    J->NodesRequested = 0;
    }
 
  J->NodesRequested = 0;
  J->TasksRequested = 0;
 
  J->NodeCount = 0;
  J->TaskCount = 0;

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];

    if (J->ReqHList != NULL)
      {
      /* tasks per node not utilized with hostlists */

      /* NOTE:  must handle issues with mixed hostlist/task spec jobs */

      RQ->TasksPerNode = 0;
      }
    else
      {
      if (MPar[0].JobNodeMatch & (1 << nmExactProcMatch))
        {
        if (RQ->TasksPerNode >= 1)
          {
          RQ->RequiredProcs = RQ->TasksPerNode;
          RQ->ProcCmp       = mcmpEQ;
          }
        }
      }

    RQ->TaskCount = MAX(1,RQ->TaskCount);

    if (TA != NULL)
      {
      if (TA->JobMemLimit > 0)
        RQ->DRes.Mem = MAX(RQ->DRes.Mem,TA->JobMemLimit / RQ->TaskCount);

      if (TA->JobSwapLimit > 0)
        RQ->DRes.Swap = MAX(RQ->DRes.Swap,TA->JobSwapLimit / RQ->TaskCount);
      }  /* END if (TA != NULL) */
 
    if (rqindex > 0)
      {
      if (RQ->DRes.Procs != 0)
        {
        /* NOTE:  currently only support one task type for proc based reqs */

        memcpy(&RQ->DRes,&J->Req[0]->DRes,sizeof(RQ->DRes));
        memcpy(&RQ->URes,&J->Req[0]->URes,sizeof(RQ->URes));

        RQ->Arch    = J->Req[0]->Arch;

        RQ->RequiredMemory = J->Req[0]->RequiredMemory;
        RQ->MemCmp         = J->Req[0]->MemCmp;
        RQ->RequiredSwap   = J->Req[0]->RequiredMemory;
        RQ->SwapCmp        = J->Req[0]->SwapCmp;
        RQ->RequiredDisk   = J->Req[0]->RequiredMemory;
        RQ->DiskCmp        = J->Req[0]->DiskCmp;
        }
 
      RQ->RMWTime = J->Req[0]->RMWTime;
      }  /* if (rqindex > 0) */

    RQ->TasksPerNode = MAX(0,RQ->TasksPerNode);
 
    if ((J->ReqHList == NULL) && (MPar[0].JobNodeMatch & (1 << nmExactNodeMatch)))
      {
      RQ->NodeCount = RQ->TaskCount / MAX(1,RQ->TasksPerNode);
      }

    J->NodesRequested += RQ->NodeCount;
    J->TasksRequested += RQ->TaskCount;
 
    if ((J->State == mjsStarting) || (J->State == mjsRunning))
      {
      J->TaskCount += RQ->TaskCount;
      J->NodeCount += RQ->NodeCount;
      }
 
    RQ->RMIndex = R->Index;
 
    RQ->TaskRequestList[0] = RQ->TaskCount;
    RQ->TaskRequestList[1] = RQ->TaskCount;
    RQ->TaskRequestList[2] = 0;
 
    /* adjust resource requirement info */
    /* NOTE:  PBS specifies resource requirement info on a 'per node' basis */
 
    if ((RQ->TaskCount > 1) &&
        (RQ->TasksPerNode > 1) &&
        (RQ->NodeCount <= 1))
      {
      RQ->NodeCount = MAX(1,RQ->TaskCount / RQ->TasksPerNode);
      }

    if ((RQ->NodeCount > 1) && (RQ->DRes.Procs > 1))
      RQ->DRes.Procs = 1;
 
    if ((RQ->URes.Procs > 0) && (RQ->TaskCount > 0))
      {
      RQ->URes.Procs /= RQ->TaskCount;
      }
 
    RQ->NAccessPolicy = MSched.DefaultNAccessPolicy;
 
    if ((J->State == mjsStarting) ||
        (J->State == mjsRunning) ||
        (J->State == mjsCompleted) ||
        (J->State == mjsRemoved))
      {
      /* obtain 'per task' statistics */

#ifdef __LINUX
      if (TA != NULL)
        TA->UtlJobCPUTime *= RQ->TaskCount;
#endif /* __LINUX */
 
      if (RQ->TaskCount > 0)
        {
        if (RQ->URes.Procs > 0)
          RQ->URes.Procs /= RQ->TaskCount;
 
        if ((TA != NULL) && (TA->UtlJobCPUTime > 0) && (RQ->RMWTime > 0))
          {
          RQ->LURes.Procs = (int)(TA->UtlJobCPUTime * 100.0 / RQ->TaskCount);
          }
        else
          {
          RQ->LURes.Procs = 100;
          }
        }
 
      /* PBS does not update 'cpupercent' (RQ->URes.Procs) quickly on some systems */
 
      if ((RQ->URes.Procs < 5) && (RQ->LURes.Procs > 0.1) && (RQ->RMWTime > 0))
        {
        RQ->URes.Procs = MAX(RQ->URes.Procs,RQ->LURes.Procs / RQ->RMWTime);
        }
      }
    }  /* END for (rqindex) */

  return(SUCCESS);
  }  /* END MPBSJobAdjustResources() */




int MPBSJobSubmit(

  char    *SubmitString,  /* I */
  mrm_t   *R,             /* I */
  mjob_t **J,             /* I */
  char    *JobName,       /* O */
  char    *Msg,           /* O */
  int     *SC)            /* O */

  {
  int rc;

  if (SubmitString == NULL)
    {
    return(FAILURE);
    }

  if (X.XRMJobSubmit == NULL)
    {
    return(FAILURE);
    }

  rc = (*X.XRMJobSubmit)(
    X.xd,
    SubmitString,
    R,
    J,
    JobName,
    Msg,
    SC);
 
  return(rc);
  }  /* END MPBSJobSubmit() */





int __MPBSIGetSSSStatus(

  mnode_t *N,     /* I (modified) */
  char    *SLine) /* I */

  {
  char *ptr;
  char *optr;
  char *tail;

  char *TokPtr;
  char *TokPtr2;

  char *Name;
  char *Value;

  int   TotMem = 0;

  const char *FName = "__MPBSIGetSSSStatus";

  DBG(7,fPBS) DPrint("%s(%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    (SLine != NULL) ? SLine : "NULL");

  if ((N == NULL) || (SLine == NULL))
    {
    return(FAILURE);
    }

  ptr = MUStrTok(SLine,",",&TokPtr);

  while (ptr != NULL)
    {
    optr = ptr;

    ptr = MUStrTok(NULL,",",&TokPtr);  

    /* FORMAT:  <ATTR>=<VALUE>[,<ATTR>=<VALUE>]... */

    if ((Name = MUStrTok(optr,"=",&TokPtr2)) == NULL)
      continue;

    if ((Value = MUStrTok(NULL,"=",&TokPtr2)) == NULL)
      continue;

    if ((Value[0] == '\0') || (Value[0] == '?'))
      continue;

    /* NOTE:  also available:  uname, sessions, nsessions, nusers, size, idletime, resi */

    if (!strcmp(Name,"arch"))
      {
      N->Arch = MUMAGetIndex(eArch,Value,mAdd);
      }
    else if (!strcmp(Name,"totmem"))
      {
      TotMem = (MPBSGetResKVal(Value) >> 10);
      }
    else if (!strcmp(Name,"availmem"))
      {
      N->ARes.Swap = (MPBSGetResKVal(Value) >> 10);
      }
    else if (!strcmp(Name,"physmem"))
      {
      N->CRes.Mem = (MPBSGetResKVal(Value) >> 10);
      }
    else if (!strcmp(Name,"size"))
      {
      char *ptr;
      char *tok;

      ptr = MUStrTok(Value,":",&tok);

      if ((tok != NULL) && (ptr != NULL)) 
        {
        N->ARes.Disk = (MPBSGetResKVal(Value) >> 10);
        N->CRes.Disk = (MPBSGetResKVal(tok) >> 10);
        }
      }
    else if (!strcmp(Name,"ncpus"))
      {
      N->CRes.Procs = (int)strtol(Value,NULL,10);
      }
    else if (!strcmp(Name,"loadave"))
      {
      double tmpD;

      tmpD = strtod(Value,&tail);

      if (*tail == '\0')
        N->Load = tmpD;
      }  /* END else if (!strcmp(Name,"loadave")) */
    else if (!strcmp(Name,"message"))
      {
      if (!strncmp(Value,"ERROR:",strlen("ERROR:")))
        {
        DBG(7,fPBS) DPrint("INFO:     node '%s' marked down - reports internal error '%s'\n",
          N->Name,
          Value + strlen("ERROR:"));

        MNodeSetState(N,mnsDown,0);
        }
      }  /* END else if (!strcmp(Name,"message")) */
    }    /* END while (ptr != NULL) */

  if (MSched.NodeMemOverCommitFactor > 0.0)
    {
    /* NOTE:  both real memory and swap overcommitted */

    N->CRes.Mem = (int)(N->CRes.Mem * MSched.NodeMemOverCommitFactor);
    N->ARes.Mem = (int)(N->ARes.Mem * MSched.NodeMemOverCommitFactor);   

    N->CRes.Swap = (int)(N->CRes.Mem * MSched.NodeMemOverCommitFactor);
    N->ARes.Swap = (int)(N->ARes.Mem * MSched.NodeMemOverCommitFactor);

    /* memory factor not applied to node load */
    }

  /* NOTE:  PBS totmem = swap + RAM */

  if ((TotMem > 0) && (N->CRes.Mem > 0))
    N->CRes.Swap = TotMem - N->CRes.Mem;

  return(SUCCESS);
  }  /* END __MPBSIGetSSSStatus() */




int __MPBSNLToTaskString(

  mnalloc_t *NL,      /* I */
  mrm_t     *R,       /* I */
  char      *TSBuf,   /* O */
  int        BufSize)
 
  {
  int tindex;

  char tmpHostName[MAX_MLINE];

  mnode_t *N;

  if ((NL == NULL) || (TSBuf == NULL))
    {
    return(FAILURE);
    }
 
  TSBuf[0] = '\0';

  /* multiple tasks per node allowed under PBS 2.2 if NP=<X> configured */
  /*   in node file */

  /* FORMAT:  <HOSTNAME>:ppn=<TASKCOUNT>[+<HOSTNAME>:ppn=<TASKCOUNT>]... */

  for (tindex = 0;NL[tindex].N != NULL;tindex++)
    {
    N = NL[tindex].N;

    DBG(7,fPBS) DPrint("INFO:     checking node '%s' for tasklist\n",
      N->Name);

    if (N->RM != R)
      continue;

    if (TSBuf[0] != '\0')
      MUStrCat(TSBuf,"+",BufSize);

#ifdef __NCSA
    {
    char *tail;

    if ((tail = strchr(N->Name,'.')) != NULL)
      {
      MUStrCpy(tmpHostName,N->Name,
        MIN(sizeof(tmpHostName),tail - N->Name));
      }
    else
      {
      MUStrCpy(tmpHostName,N->Name,sizeof(tmpHostName));
      }
    }    /* END BLOCK */
#else /* __NCSA */
    MUStrCpy(tmpHostName,N->Name,sizeof(tmpHostName));
#endif /* __NCSA */

    if (NL[tindex].TC == 1)
      {
      MUStrCat(TSBuf,tmpHostName,BufSize);
      }
    else
      {
      sprintf(TSBuf,"%s%s:ppn=%d",
        TSBuf,
        tmpHostName,
        NL[tindex].TC);
      }
    }  /* END for (tindex) */

  return(SUCCESS);
  }  /* END __MPBSNLToTaskString() */




int __MPBSJobChkExecutable(

  struct batch_status *PJob)  /* I */

  {
  struct attrl *AP;

  mclass_t *C;

  for (AP = PJob->attribs;AP != NULL;AP = AP->next)
    {
    if (!strcmp(AP->name,ATTR_queue))
      break;
    }

  if (AP == NULL)
    {
    return(FAILURE);  /* Odd, this job isn't in a queue?! */
    }

  C = NULL;

  MClassFind(AP->value,&C);

  if (C==NULL)
    {
    return(FAILURE);  /* must be a brand new queue, we'll find it next iteration */
    }

  if (C->NonExeType == TRUE)
    {
    return(FAILURE);  /* the job is in a routing queue */
    }

  return(SUCCESS);
  }  /* END __MPBSJobChkExecutable() */


#else /* __MPBS */

int MPBSGetJobs(int *NumJobs,int RMIndex)

  {
  DBG(0,fPBS) DPrint("ALERT:    %s not enabled for PBS API\n",
    MSCHED_SNAME);

  return(FAILURE);
  }


int MPBSGetNodes(int *NumNodes,int RMIndex)

  {
  DBG(0,fPBS) DPrint("ALERT:    %s not enabled for PBS API\n",
    MSCHED_SNAME);

  return(FAILURE);
  }


int MPBSProcessEvent(mrm_t *R,int *SC)

  {
  DBG(0,fPBS) DPrint("ALERT:    %s not enabled for PBS API\n",
    MSCHED_SNAME);
 
  return(FAILURE);
  }


int MPBSJobSetAttr(
 
  mjob_t  *J,
  void    *A,
  char    *ValLine,
  tpbsa_t *TA,
  short   *TaskList,
  int      Mode)

  {
  DBG(0,fPBS) DPrint("ALERT:    %s not enabled for PBS API\n",
    MSCHED_SNAME);
 
  return(FAILURE);
  }


int MPBSJobAdjustResources(
 
  mjob_t  *J,
  tpbsa_t *TA,
  mrm_t   *R)

  {
  DBG(0,fPBS) DPrint("ALERT:    %s not enabled for PBS API\n",
    MSCHED_SNAME);
 
  return(FAILURE);
  }

#endif /* else __MPBS */


/* END MPBSI.c */




