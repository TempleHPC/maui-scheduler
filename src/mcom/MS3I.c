/* HEADER */
        
/* Contains:                                   *
 *  int MS3DoCommand(P,CmdString,OBuf,OE,SC,Msg) *
 *                                             */

#include "moab.h"
#include "moab-proto.h"  

#define __MS330

/* #include "mg2.h" */

extern mlog_t    mlog;

extern msched_t    MSched;
extern mclass_t    MClass[];
extern mjob_t     *MJob[];
extern mbool_t     MGlobalTLock;
extern mrm_t       MRM[];    
extern mattrlist_t MAList;
extern mnode_t    *MNode[];

extern const char *MJobState[];
extern const char *MNodeState[];
extern const char *MS3JobState[];
extern const char *MS3CName[];
extern const char *MS3SockProtocol[];
extern const char *MClassState[];
extern const char *MS3VName[];
extern const char *MS3Action[];
extern const char *MBool[];

extern char *MSON[];
extern char *MSAN[];

typedef struct {
  msocket_t S;
  } s3handle_t;
 
typedef struct {
  int          SchedSD;
  int          ServerSD;
  s3handle_t   QMH;
  s3handle_t   NMH;
  int          UpdateIteration;
  char        *WorkloadBuffer;
  char        *ResourceBuffer;
  char        *QueueBuffer;
  } s3rm_t;

/* S3 prototypes */

int MS3Initialize(mrm_t *,int *);
int MS3Ping(mrm_t *,enum MStatusCodeEnum *);
int MS3GetData(mrm_t *,mbool_t,int *);
int MS3JobStart(mjob_t *,mrm_t *,char *,enum MStatusCodeEnum *);
int MS3JobRequeue(mjob_t *,mrm_t *,mjob_t **,char *,int *); 
int MS3JobModify(mjob_t *,mrm_t *,char *,char *,char *,int *);
int MS3JobCancel(mjob_t *,mrm_t *,char *,char *,enum MStatusCodeEnum *); 
int MS3JobSubmit(char *,mrm_t *,mjob_t **,long,char *,char *,enum MStatusCodeEnum *);
int MS3JobSuspend(mjob_t *,mrm_t *,char *,int *); 
int MS3JobResume(mjob_t *,mrm_t *,char *,int *); 
int MS3JobCheckpoint(mjob_t *,mrm_t *,mbool_t,char *,int *);
int MS3ClusterQuery(mrm_t *,int *,char *,enum MStatusCodeEnum *);
int MS3WorkloadQuery(mrm_t *,int *,enum MStatusCodeEnum *);
int MS3QueueQuery(mrm_t *,int *,char *,enum MStatusCodeEnum *);
int MS3JobLoad(char *,void *,mjob_t **,short *,mrm_t *);
int MS3JobUpdate(void *,mjob_t **,short *,mrm_t *);
int MS3NodeLoad(mnode_t *,void *,int,mrm_t *);
int MS3NodeUpdate(mnode_t *,void *,int,mrm_t *);
int __MS3JobStepGetState(mxml_t *,mrm_t *,char *,enum MJobStateEnum *); 
int __MS3NodeGetState(void *,mrm_t *,char *,enum MNodeStateEnum *); 
int __MS3QueueGetState(mxml_t *,mrm_t *,char *,int *);
int MS3GetQueueInfo(mnode_t *,char C[][MMAX_NAME],char A[][MMAX_NAME]);
int __MS3GetTaskList(char *,short *,int *,int *);
int __MS3JobSetAttr(mjob_t *,mreq_t *,int,char *,char *,mbool_t,mbool_t,void *);
int __MS3NodeSetAttr(mnode_t *,mrm_t *,char *,char *,void *);
int __MS3QueueSetAttr(mclass_t *,char *,char *);
char *__MS3GetErrMsg(int);
int MS3EncapsulateMessage(char *,char *,mbool_t,char **,long *);
int MS3Setup(int);

/* END S3 prototypes */


/* NOTE:  'NodeList' maps to mjaAllocNodeList in 'Utilized' and mjaHostList in 'Requested' */

/* attribute           object    0.2 attr                  3.0 attr        4.0 attr  */

#ifdef __cplusplus

extern "C" const mobjattr_t MS3JobTable[] =  { 
  { mjaAccount,        mxoJob, { "accountName",            "ProjectId",    NULL, NULL } },
  { mjaAllocNodeList,  mxoJob, { NULL,                     "NodeList",     NULL, NULL } },
  { mjaArgs,           mxoJob, { NULL,                     "Arguments",    NULL, NULL } },
  { mjaAWDuration,     mxoJob, { "wallclockTimeUsed",      "WallDuration", NULL, NULL } },
  { mjaCalendar,       mxoJob, { NULL,                     "Calendar",     NULL, NULL } },
  { mjaCmdFile,        mxoJob, { NULL,                     "Executable",   NULL, NULL } },
  { mjaCompletionTime, mxoJob, { NULL,                     "CompletionTime", NULL, NULL } },  /* change to 'EndTime' */

  { mjaCPULimit,       mxoJob, { NULL,                     "CPULimit",     NULL, NULL } },
  { mjaDepend,         mxoJob, { NULL,                     "Dependency",   NULL, NULL } },
  { mjaEnv,            mxoJob, { NULL,                     "Environment",  NULL, NULL } },
  { mjaFlags,          mxoJob, { NULL,                     "JobFlags",     NULL, NULL } },
  { mjaGroup,          mxoJob, { "groupName",              "GroupId",      NULL, NULL } },
  { mjaHold,           mxoJob, { NULL,                     "Hold",         NULL, NULL } },
  { mjaIsInteractive,  mxoJob, { NULL,                     "IsInteractive", NULL, NULL } },
  { mjaIsRestartable,  mxoJob, { NULL,                     "Restartable",  NULL, NULL } },
  { mjaIsSuspendable,  mxoJob, { NULL,                     "Suspendable",  NULL, NULL } },
  { mjaIWD,            mxoJob, { NULL,                     "InitialWorkingDirectory", NULL, NULL } },
  { mjaJobName,        mxoJob, { "jobName",                "JobName",      NULL, NULL } },
  { mjaJobID,          mxoJob, { "jobId",                  "JobId",        NULL, NULL } },
  { mjaNotification,   mxoJob, { NULL,                     "Notification", NULL, NULL } },
  { mjaPAL,            mxoJob, { NULL,                     "Partition",    NULL, NULL } },
  { mjaQOSReq,         mxoJob, { "qos",                    "QualityOfService", NULL, NULL } },
  { mjaReqAWDuration,  mxoJob, { "wallclockLimit",         "WallDuration", NULL, NULL } },
  { mrqaNCReqMin,      mxoReq, { "requestedNodes",         "NodeCount",    NULL, NULL } },
  { mjaReqReservation, mxoJob, { NULL,                     "RequiredReservation", NULL, NULL } },
  { mjaReqSMinTime,    mxoJob, { "eligibleDate",           "EligibleTime", NULL, NULL } },
  { mjaRMXString,      mxoJob, { "opaqueFlags",            "OpaqueFlags",  NULL, NULL } },   /* replace w/Extension (attrs-> Type={scheduler,grid,am,???} Name=<attr>  5.1.10 of S3 Job Description */
  { mjaRsvAccess,      mxoJob, { NULL,                     "RsvAccess",    NULL, NULL } },
  { mjaStartTime,      mxoJob, { NULL,                     "StartTime",    NULL, NULL } },
  { mjaState,          mxoJob, { "jobState",               "JobState",     NULL, NULL } },
  { mjaStdErr,         mxoJob, { NULL,                     "ErrorFile",    NULL, NULL } },
  { mjaStdIn,          mxoJob, { NULL,                     "InputFile",    NULL, NULL } },
  { mjaStdOut,         mxoJob, { NULL,                     "OutputFile",   NULL, NULL } },
  { mjaStepID,         mxoJob, { NULL,                     "StepId",       NULL, NULL } },
  { mjaStartTime,      mxoJob, { NULL,                     "StartTime",    NULL, NULL } },
  { mjaSubmitLanguage, mxoJob, { NULL,                     "SubmitLanguage", NULL, NULL } },
  { mjaSubmitString,   mxoJob, { NULL,                     "SubmitString", NULL, NULL } },
  { mjaSubmitTime,     mxoJob, { NULL,                     "SubmitTime",   NULL, NULL } },  /* change to SubmissionTime */
  { mjaUser,           mxoJob, { "userName",               "UserId",       NULL, NULL } },
  { mjaUserPrio,       mxoJob, { NULL,                     "Priority",     NULL, NULL } },
  { mrqaReqArch,       mxoReq, { NULL,                     "Architecture", NULL, NULL } },
  { mrqaReqClass,      mxoReq, { "requestedQueue",         "Queue",        NULL, NULL } }, /* FIX */
  { mrqaReqDiskPerTask, mxoReq, { NULL,                    "Disk",         NULL, NULL } },
  { mrqaReqMemPerTask, mxoReq, { NULL,                     "Memory",       NULL, NULL } },
  { mrqaReqNodeDisk,   mxoReq, { NULL,                     "NodeDisk",     NULL, NULL } },
  { mrqaReqNodeFeature, mxoReq,{ NULL,                     "Feature",      NULL, NULL } },
  { mrqaReqNodeMem,    mxoReq, { NULL,                     "NodeMemory",   NULL, NULL } },
  { mrqaReqNodeProc ,  mxoReq, { NULL,                     "NodeProc",     NULL, NULL } },
  { mrqaReqNodeSwap,   mxoReq, { NULL,                     "NodeSwap",     NULL, NULL } },
  { mrqaReqOpsys,      mxoReq, { NULL,                     "OpSys",        NULL, NULL } },
  { mrqaTCReqMin,      mxoReq, { "requestedProcessors",    "Processors",   NULL, NULL } },
  { mrqaTPN,           mxoReq, { NULL,                     "TPN",          NULL, NULL } },
  { -1,                mxoNONE,{ NULL,                     NULL,           NULL, NULL } }};
#else /* __cplusplus */
const mobjattr_t MS3JobTable[] =  { 
  { mjaAccount,        mxoJob, { "accountName",            "ProjectId",    NULL, NULL } },
  { mjaAllocNodeList,  mxoJob, { NULL,                     "NodeList",     NULL, NULL } },
  { mjaArgs,           mxoJob, { NULL,                     "Arguments",    NULL, NULL } },
  { mjaAWDuration,     mxoJob, { "wallclockTimeUsed",      "WallDuration", NULL, NULL } },
  { mjaCalendar,       mxoJob, { NULL,                     "Calendar",     NULL, NULL } },
  { mjaCmdFile,        mxoJob, { NULL,                     "Executable",   NULL, NULL } },
  { mjaCompletionTime, mxoJob, { NULL,                     "CompletionTime", NULL, NULL } },
  { mjaCPULimit,       mxoJob, { NULL,                     "CPULimit",     NULL, NULL } },
  { mjaDepend,         mxoJob, { NULL,                     "Dependency",   NULL, NULL } },
  { mjaEnv,            mxoJob, { NULL,                     "Environment",  NULL, NULL } },
  { mjaFlags,          mxoJob, { NULL,                     "JobFlags",     NULL, NULL } },
  { mjaGroup,          mxoJob, { "groupName",              "GroupId",      NULL, NULL } },
  { mjaHold,           mxoJob, { NULL,                     "Hold",         NULL, NULL } },
  { mjaIsInteractive,  mxoJob, { NULL,                     "IsInteractive", NULL, NULL } },
  { mjaIsRestartable,  mxoJob, { NULL,                     "Restartable",  NULL, NULL } },
  { mjaIsSuspendable,  mxoJob, { NULL,                     "Suspendable",  NULL, NULL } },
  { mjaIsInteractive,  mxoJob, { NULL,                     "IsInteractive", NULL, NULL } },
  { mjaIWD,            mxoJob, { NULL,                     "InitialWorkingDirectory", NULL, NULL } },
  { mjaJobName,        mxoJob, { "jobName",                "JobName",      NULL, NULL } },
  { mjaJobID,          mxoJob, { "jobId",                  "JobId",        NULL, NULL } },
  { mjaNotification,   mxoJob, { NULL,                     "Notification", NULL, NULL } },
  { mjaPAL,            mxoJob, { NULL,                     "Partition",    NULL, NULL } },
  { mjaQOSReq,         mxoJob, { "qos",                    "QualityOfService", NULL, NULL } },
  { mjaReqAWDuration,  mxoJob, { "wallclockLimit",         "WallDuration", NULL, NULL } },
  { mrqaNCReqMin,      mxoReq, { "requestedNodes",         "NodeCount",    NULL, NULL } },
  { mjaReqReservation, mxoJob, { NULL,                     "RequiredReservation", NULL, NULL } },
  { mjaReqSMinTime,    mxoJob, { "eligibleDate",           "EligibleTime", NULL, NULL } },
  { mjaRMXString,      mxoJob, { "opaqueFlags",            "OpaqueFlags",  NULL, NULL } },
  { mjaRsvAccess,      mxoJob, { NULL,                     "RsvAccess",    NULL, NULL } },
  { mjaStartTime,      mxoJob, { NULL,                     "StartTime",    NULL, NULL } },
  { mjaState,          mxoJob, { "jobState",               "JobState",     NULL, NULL } },
  { mjaStdErr,         mxoJob, { NULL,                     "ErrorFile",    NULL, NULL } },
  { mjaStdIn,          mxoJob, { NULL,                     "InputFile",    NULL, NULL } },
  { mjaStdOut,         mxoJob, { NULL,                     "OutputFile",   NULL, NULL } },
  { mjaStepID,         mxoJob, { NULL,                     "StepId",       NULL, NULL } },
  { mjaStartTime,      mxoJob, { NULL,                     "StartTime",    NULL, NULL } },
  { mjaSubmitLanguage, mxoJob, { NULL,                     "SubmitLanguage", NULL, NULL } },
  { mjaSubmitString,   mxoJob, { NULL,                     "SubmitString", NULL, NULL } },
  { mjaSubmitTime,     mxoJob, { NULL,                     "SubmitTime",   NULL, NULL } },
  { mjaUser,           mxoJob, { "userName",               "UserId",       NULL, NULL } },
  { mjaUserPrio,       mxoJob, { NULL,                     "Priority",     NULL, NULL } },
  { mrqaReqArch,       mxoReq, { NULL,                     "Architecture", NULL, NULL } },
  { mrqaReqClass,      mxoReq, { "requestedQueue",         "Queue",        NULL, NULL } }, /* FIX */
  { mrqaReqDiskPerTask, mxoReq, { NULL,                    "Disk",         NULL, NULL } },
  { mrqaReqMemPerTask, mxoReq, { NULL,                     "Memory",       NULL, NULL } },
  { mrqaReqNodeDisk,   mxoReq, { NULL,                     "NodeDisk",     NULL, NULL } },
  { mrqaReqNodeFeature, mxoReq,{ NULL,                     "Feature",      NULL, NULL } },  /* should be <NodeProperties><Feature></Feature><Name></Name><ConsumableResources/></NodeProperties>, see 5.1.9 NOTE:  consumable resources are attributes of node, ie, minswappernode, not to be allocated to job */

  { mrqaReqNodeMem,    mxoReq, { NULL,                     "NodeMemory",   NULL, NULL } },
  { mrqaReqNodeProc ,  mxoReq, { NULL,                     "NodeProc",     NULL, NULL } },
  { mrqaReqNodeSwap,   mxoReq, { NULL,                     "NodeSwap",     NULL, NULL } },
  { mrqaReqOpsys,      mxoReq, { NULL,                     "OpSys",        NULL, NULL } },
  { mrqaTCReqMin,      mxoReq, { "requestedProcessors",    "Processors",   NULL, NULL } },
  { mrqaTPN,           mxoReq, { NULL,                     "TPN",          NULL, NULL } },
  { -1,                mxoNONE,{ NULL,                     NULL,           NULL, NULL } }};
#endif

/* handle multi-level attribute population */

/* attribute          object    0.2 attr         3.0 attr    4.0 attr      xpath */

const mobjattr_t MS3NodeTable[] = {
  { mnaFeatures,    mxoNode,  { "features",      "Features", NULL,         "Features" } },  /* should be NodeProperties, see 5.1.9, may have child elements of Name, Feature, ConsumableResources */
  { mnaNodeID,      mxoNode,  { "nodeId",        "NodeId", NULL,           "NodeId" } },
  { mnaNodeState,   mxoNode,  { "nodeState",     "NodeState", NULL,        "NodeState" } },
  { mnaRCMem,       mxoNode,  { "memTot",        "ConfiguredMemory", NULL, "Configured/Memory" } },
  { mnaRAMem,       mxoNode,  { "memFree",       "AvailableMemory", NULL,  "Available/Memory" } },
  { mnaRCSwap,      mxoNode,  { "swapTot",       "ConfiguredSwap", NULL,   "Configured/Swap" } },
  { mnaRASwap,      mxoNode,  { "swapFree",      "AvailableSwap", NULL,    "Available/Swap" } },
  { mnaRCDisk,      mxoNode,  { "diskTot",       "ConfiguredDisk", NULL,   "Configured/Disk" } },
  { mnaRADisk,      mxoNode,  { "diskFree",      "AvailableDisk", NULL,    "Available/Disk" } },
  { mnaRCProc,      mxoNode,  { "numProcessors", "ConfiguredProcessors", NULL, "Configured/Processors" } }, 
  { mnaLoad,        mxoNode,  { "utlProcessors", "UtilizedProcessors", NULL, "Utilized/Processors" } },

/* { mna???,         mxoNode,  { "nodeType",     "NodeType", NULL, NULL } }, */
/* { mna???,         mxoNode,  { "maxtask",      "VirtualProcessors", NULL, NULL } }, */

  /* NOTE:  percentCPUUsed is available, node load is not */

  { mnaRAProc,      mxoNode,  { "AVLProcCount",  "AvailableProcessors", NULL, "Available/Processors" } },
  { -1,             mxoNONE,  { NULL,            NULL, NULL, NULL } }};

const mobjattr_t MS3NodeRTable[] = {
  { mrDisk,         mxoNode,  { "disk",          "Disk",       NULL,     NULL } },
  { mrMem,          mxoNode,  { "memory",        "Memory",     NULL,     NULL } },
  { mrProc,         mxoNode,  { "processors",    "Processors", NULL,     NULL } },
  { mrSwap,         mxoNode,  { "swap",          "Swap",       NULL,     NULL } },
  { -1,             mxoNONE,  { NULL,            NULL, NULL, NULL } }};


const mobjattr_t MS3QueueTable[] = {
  { mclaName,       mxoClass, { "queueId",       "QueueId", NULL, NULL } },
  { mclaState,      mxoClass, { "queueStarted",  "QueueStarted", NULL, NULL } },
  { mclaHostList,   mxoClass, { "hostList",      "HostList", NULL, NULL } },
  { -1,             mxoNONE,  { NULL,            NULL, NULL, NULL } }};

const mobjattr_t MS3OTable[] = {
  { mxoCluster,     mxoCluster, { "cluster",       "Cluster", NULL, NULL } },
  { mxoJob,         mxoJob,     { "job",           "Job",     NULL, NULL } },
  { mxoNode,        mxoNode,    { "node",          "Node",    NULL, NULL } },
  { mxoQueue,       mxoQueue,   { "queue",         "Queue",   NULL, NULL } },
  { mxoRsv,         mxoRsv,     { "rsv",           "Rsv",     NULL, NULL } },
  { mxoSched,       mxoSched,   { "sched",         "Sched",   NULL, NULL } },
  { mxoRange,       mxoRange,   { "range",         "Range",   NULL, NULL } },
  { mxoNONE,        mxoNONE,    { NULL,            NULL,      NULL, NULL } } };

const mobjattr_t MS3JATable[] = {
  { mrmJobCancel,     mxoNONE,  { "delete",        "Cancel", NULL, NULL } },
  { mrmJobCheckpoint, mxoNONE,  { "checkpoint",    "Checkpoint", NULL, NULL } },
  { mrmJobModify,     mxoNONE,  { "modify",        "Modify", NULL, NULL } },
  { mrmJobQuery,      mxoNONE,  { "query",         "Query", NULL, NULL } },
  { mrmJobRequeue,    mxoNONE,  { "requeue",       "Requeue", NULL, NULL } },
  { mrmJobResume,     mxoNONE,  { "resume",        "Resume", NULL, NULL } },
  { mrmJobStart,      mxoNONE,  { "start",         "Start", NULL, NULL } },
  { mrmJobSubmit,     mxoNONE,  { "submit",        "Submit", NULL, NULL } },
  { mrmJobSuspend,    mxoNONE,  { "suspend",       "Suspend", NULL, NULL } },
  { mrmNONE,          mxoNONE,  { NULL,            NULL, NULL, NULL } } };

/* sync w/ enum MRangeAttrEnum */

const char *MS3RangeAttr[] = {
  NONE,
  "CompletionTime",
  "Cost",
  "Nodes",
  "QOI",
  "ScalingFactor",
  "StartTime",
  "Tasks",
  NULL };

/* sync w/ enum XXX */

const char *MS3RsvAttr[] = {
  NONE,
  "EndTime",
  "Name",
  "StartTime",
  "State",
  NULL };
  
char MQueueErrMsg[MMAX_LINE];
char MWorkloadErrMsg[MMAX_LINE];
char MResourceErrMsg[MMAX_LINE];
  

char *MS3JobAttr[MMAX_S3VERS][MMAX_S3ATTR];
char *MS3ReqAttr[MMAX_S3VERS][MMAX_S3ATTR];
char *MS3NodeAttr[MMAX_S3VERS][MMAX_S3ATTR];
char *MS3NodeRAttr[MMAX_S3VERS][MMAX_S3ATTR];
char *MS3QueueAttr[MMAX_S3ATTR];
char *MS3ObjName[MMAX_S3VERS][MMAX_S3ATTR];
char *MS3JAName[MMAX_S3VERS][MMAX_S3JACTION];

char *MS3Object[MMAX_S3ATTR];


#ifdef __M32COMPAT

#define MJOBREMOVE(J)  MJobRemove(*(J))
#define MJOBPROCESSREMOVED(J)  MJobProcessRemoved(*(J))
#define MJOBPROCESSCOMPLETED(J)  MJobProcessCompleted(*(J))

#define SRM RM

#else /* __M32COMPAT */

#define MJOBREMOVE(J)  MJobRemove(J)
#define MJOBPROCESSREMOVED(J)  MJobProcessRemoved(J)
#define MJOBPROCESSCOMPLETED(J)  MJobProcessCompleted(J)

#endif /* __M32COMPAT */



int MS3Setup(

  int VIndex)  /* I */

  {
  int aindex;
  int vindex;

  static const char *ANone = "[NONE]";

  static mbool_t IsInitialized = FALSE;

  if (IsInitialized == TRUE)
    {
    return(SUCCESS);
    }

  /* NOTE:  populate table with '[NONE]' */

  for (vindex = 0;vindex < MMAX_S3VERS;vindex++)
    {
    for (aindex = 0;aindex < MMAX_S3ATTR;aindex++)
      {
      MS3JobAttr[vindex][aindex] = (char *)ANone;
      MS3ReqAttr[vindex][aindex] = (char *)ANone;

      MS3ObjName[vindex][aindex] = (char *)ANone;

      MS3Object[aindex] = (char *)ANone;

      MS3NodeAttr[vindex][aindex] = (char *)ANone;
      MS3NodeRAttr[vindex][aindex] = (char *)ANone;

      if (vindex > 0)
        continue;
 
      MS3QueueAttr[aindex] = (char *)ANone;
      }  /* END for (aindex) */

    for (aindex = 0;MS3JobTable[aindex].AIndex != -1;aindex++)
      {
      if (MS3JobTable[aindex].OType == mxoJob)
        {
        MS3JobAttr[vindex][MS3JobTable[aindex].AIndex] =
          MS3JobTable[aindex].AName[vindex];
        }
      else
        {
        MS3ReqAttr[vindex][MS3JobTable[aindex].AIndex] =
          MS3JobTable[aindex].AName[vindex];
        }
      }  /* END for (aindex) */

    for (aindex = 0;MS3OTable[aindex].AIndex != mxoNONE;aindex++)
      {
      MS3Object[MS3OTable[aindex].AIndex] = MS3OTable[aindex].AName[VIndex];

      MS3ObjName[vindex][MS3OTable[aindex].AIndex] =
        MS3OTable[aindex].AName[vindex];
      }  /* END for (aindex) */

    for (aindex = 0;MS3JATable[aindex].AIndex != mrmNONE;aindex++)
      {
      MS3JAName[vindex][MS3JATable[aindex].AIndex] =
        MS3JATable[aindex].AName[vindex];
      }  /* END for (aindex) */

    for (aindex = 0;MS3NodeRTable[aindex].AIndex != -1;aindex++)
      {
      if (MS3NodeRTable[aindex].OType == mxoNode)
        {
        MS3NodeRAttr[vindex][MS3NodeRTable[aindex].AIndex] =
          MS3NodeRTable[aindex].AName[vindex];
        }
      else
        {
        /* NO-OP */
        }
      }  /* END for (aindex) */

    for (aindex = 0;MS3NodeTable[aindex].AIndex != -1;aindex++)
      {
      if (MS3NodeTable[aindex].OType == mxoNode)
        {
        MS3NodeAttr[vindex][MS3NodeTable[aindex].AIndex] =
          MS3NodeTable[aindex].AName[vindex];
        }
      else
        {
        /* NO-OP */
        }
      }  /* END for (aindex) */
    }    /* END for (vindex) */

  for (aindex = 0;MS3QueueTable[aindex].AIndex != -1;aindex++)
    {
    if (MS3QueueTable[aindex].OType == mxoClass)
      {
      MS3QueueAttr[MS3QueueTable[aindex].AIndex] =
        MS3QueueTable[aindex].AName[1];  /* NOTE:  '1' is temp hack */
      }
    else
      {
      /* NO-OP */
      }
    }  /* END for (aindex) */

  IsInitialized = TRUE;

  return(SUCCESS);
  }    /* END MS3Setup() */





int MS3LoadModule(
 
  mrmfunc_t *F)  /* I */
 
  {
  if (F == NULL)
    {
    return(FAILURE);
    }
 
  F->ClusterQuery   = MS3ClusterQuery;
  F->JobCancel      = MS3JobCancel;
  F->JobCheckpoint  = MS3JobCheckpoint;
  F->JobModify      = NULL;
  F->JobQuery       = NULL;
  F->JobRequeue     = MS3JobRequeue;
  F->JobResume      = MS3JobResume;
  F->JobStart       = MS3JobStart;
#ifndef __M32COMPAT
  F->JobSubmit      = MS3JobSubmit;
#endif /* __M32COMPAT */
  F->JobSuspend     = MS3JobSuspend;
#ifndef __M32COMPAT
  F->Ping           = MS3Ping;
#endif /* __M32COMPAT */
  F->QueueQuery     = MS3QueueQuery;
  F->ResourceModify = NULL;
  F->ResourceQuery  = NULL;
  F->RMInitialize   = MS3Initialize;
  F->RMQuery        = NULL;
  F->WorkloadQuery  = MS3WorkloadQuery;

  F->IsInitialized = TRUE;
 
  return(SUCCESS);
  }  /* END MS3LoadModule() */




int MS3Ping(

  mrm_t                *R,   /* I */
  enum MStatusCodeEnum *SC)  /* O (optional) */

  {
  int rc;

  static mxml_t *PE = NULL;

  static char tmpLine[MMAX_LINE];

  if (SC != NULL)
    *SC = mscNoError;

  if (R == NULL)
    {
    return(FAILURE);
    }

  if (PE == NULL)
    {
    /* generate minimal ping query */

    MXMLCreateE(&PE,MSON[msonRequest]);

    MXMLSetAttr(PE,MSAN[msanAction],(void *)MS3Action[msssaQuery],mdfString);
    MS3SetObject(PE,MS3ObjName[R->Version][mxoJob],NULL);

    /* specify query constraints */

    MS3AddWhere(PE,"JobClass","active",NULL);

    MXMLToString(PE,tmpLine,sizeof(tmpLine),NULL,TRUE);
    }  /* END if (PE == NULL) */

  rc = MS3DoCommand(
    &R->P[0],
    tmpLine,
    NULL,
    NULL,
    NULL,
    NULL);

  if (rc == FAILURE)
    {
    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MS3Ping() */




int MS3Initialize(

  mrm_t *R,   /* I */
  int   *SC)  /* O (optional) */
 
  {
  s3rm_t *S;

#ifndef __MPROD
  const char *FName = "MS3Initialize";

  MDB(1,fS3) MLog("%s(%s,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");
#endif /* !__MPROD */
 
  if (R == NULL)
    {
    return(FAILURE);
    }
 
  if (R->S == NULL)
    {
    R->S = (void *)calloc(1,sizeof(s3rm_t));
 
    /* set default rm specific values */

    /* N/A */
    }

  R->P[0].Type = mpstQM;
  R->P[0].S    = &((s3rm_t *)R->S)->QMH.S;

  R->P[1].Type = mpstNM;
  R->P[1].S    = &((s3rm_t *)R->S)->NMH.S;

  ((s3rm_t *)(R->S))->UpdateIteration = -1;

  /* set S3_ROOT */

  MUSetEnv("S3_ROOT","/usr/local/sss");

  S = (s3rm_t *)R->S;

  R->Version = MUGetIndex(R->APIVersion,MS3VName,FALSE,msssV3_0);

  MS3Setup(R->Version);
 
  /* register with directory service */

  /* NOTE:  should loop through all supported protocols (NYI) */

  if (!MISSET(R->Flags,mrmfLocalQueue))
    {
    char HostName[MMAX_NAME];
    int  Port;
    char WireProtocol[MMAX_NAME];
    char SocketProtocol[MMAX_NAME];

    if (MSysDSRegister(
         (char *)MS3CName[mpstSC],
         MRM[0].Name,
         MSched.ServerHost,
         MSched.ServerPort,
         NULL,
         (char *)MS3SockProtocol[mspSingleUseTCP]) == FAILURE)
      {
      MDB(1,fRM) MLog("ALERT:    cannot register with directory service\n");
      }

    if (R->UseDirectoryService == TRUE)
      { 
      /* obtain interface information */

      if (MSysDSQuery(
           (char *)MS3CName[mpstQM],
           MRM[0].Name,
           HostName,
           &Port,
           WireProtocol,
           SocketProtocol) == SUCCESS)
        {
        MDB(1,fRM) MLog("INFO:     obtained QM info (H: %s  P: %d  S: %s)\n",
          HostName,
          Port,
          SocketProtocol);

        /* update active QM interface */

        /* NYI */
        }

      if (MSysDSQuery(
           (char *)MS3CName[mpstNM],
           MRM[0].Name,
           HostName,
           &Port,
           WireProtocol,
           SocketProtocol) == SUCCESS)
        {
        /* update active SM interface */

        /* NYI */
        }
      }    /* END if (R->UseDirectoryService == TRUE) */

    /* enable EM service */

    if (MSysDSQuery(
         (char *)MS3CName[mpstEM],
         MRM[0].Name,
         HostName,
         &Port,
         WireProtocol,
         SocketProtocol) == SUCCESS)
      {
      /* update active EM interface */

      if (HostName[0] != '\0')
        MUStrDup(&MSched.EM.HostName,HostName);

      if (Port != -1)
        MSched.EM.Port = Port;
      }

    MSysEMSubmit(
      &MSched.EM,
      (char *)MS3CName[mpstSC],
      "schedstart",
      MSched.Name);

    /* NOTE:  currently ignore EM responses */

    MSysEMRegister(
      &MSched.EM,
      (char *)MS3CName[mpstQM],
      (char *)"job-submitted",
      NULL,
      (char *)"NONE");

    MSysEMRegister(
      &MSched.EM,
      (char *)MS3CName[mpstQM],
      (char *)"job-finished",
      NULL,
      (char *)"NONE");
    }    /* END if (!MISSET(R->Flags,mrmfLocalQueue)) */

  MDB(1,fRM) MLog("INFO:     connected to S3 server\n");

  return(SUCCESS);
  }  /* END MS3Initialize() */




int MS3ProcessEvent(

  mrm_t *R)  /* I */

  {
  int  Cmd;
  int *Cptr;

  int EventReceived     = FALSE;
  int EventsOutstanding = TRUE;

  struct timeval timeout;    
 
  s3rm_t *S;

  msocket_t tmpS;
  msocket_t tmpC;

  if (R == NULL)
    {
    return(FAILURE);
    }

  S = (s3rm_t *)R->S;
 
  if (S->SchedSD == -1)
    {
    MDB(1,fS3) MLog("INFO:     invalid S3 sched socket\n");
   
    return(FAILURE);
    }

  while (EventsOutstanding == TRUE)
    {       
    fd_set fdset;
    int rpp_fd = 0;   

    FD_ZERO(&fdset);

    if (rpp_fd != -1)
      {
      FD_SET(rpp_fd,&fdset);
      }

    FD_SET(S->SchedSD,&fdset);

    timeout.tv_sec  = 0;
    timeout.tv_usec = 10000;   

    if (select(FD_SETSIZE,&fdset,NULL,NULL,&timeout) == -1) 
      {
      MDB(1,fS3) MLog("ALERT:    select failed checking S3 sched socket\n");     

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

      MDB(9,fS3) MLog("ALERT:    no S3 RPP sched socket connections ready\n");
 
      EventsOutstanding = FALSE;        
      }
    else if (!FD_ISSET(S->SchedSD,&fdset))
      {
      /* no connections ready */

      MDB(9,fS3) MLog("INFO:     no S3 sched socket connections ready\n");    

      EventsOutstanding = FALSE;
 
      break;     
      }

    memset(&tmpS,0,sizeof(tmpS));
    tmpS.sd = S->SchedSD;

    memset(&tmpC,0,sizeof(tmpC));         

    if (MSUAcceptClient(&tmpS,&tmpC,NULL,(1 << msftTCP)) == FAILURE)
      {
      /* cannot accept new socket */

      MDB(1,fS3) MLog("ALERT:    cannot accept client on S3 sched socket\n");  

      EventsOutstanding = FALSE;
 
      break;     
      }

    Cptr = &Cmd;

    if (MSURecvPacket(
          tmpS.sd,
          (char **)&Cptr,
          sizeof(int),
          NULL,
          MDEF_CLIENTTIMEOUT,
          NULL) == FAILURE)
      {
      MDB(1,fS3) MLog("ALERT:    cannot read on S3 sched socket\n");  

      close(tmpS.sd);

      EventsOutstanding = FALSE;
 
      break;      
      }

    /* S3 sched command received */

    close(tmpS.sd);

    MDB(4,fS3) MLog("INFO:     S3 command %d received\n",
      Cmd);

    EventReceived = TRUE;         
    }  /* END while (EventsOutstanding == TRUE) */

  if (EventReceived == FALSE)
    {
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MS3ProcessEvent() */ 




int MS3GetData(

  mrm_t   *R,      /* I */
  mbool_t  DoPing, /* I */
  int     *SC)     /* O (optional) */

  {
  int      rc;

  s3rm_t *S;

  mpsi_t  *P;

  mxml_t  *E = NULL;

  mxml_t  *RDE;

  static char *JobReqString = NULL;
  static char *ResReqString = NULL;
  static char *QueueReqString = NULL;

  char tmpString[MMAX_LINE];

#ifndef __MPROD
  const char *FName = "MS3GetData";

  MDB(1,fS3) MLog("%s(%s,%s,SC)\n",
    FName, 
    MBool[DoPing],
    (R != NULL) ? R->Name : "NULL");
#endif /* !__MPROD */

  if ((R == NULL) || (R->S == NULL))
    {
    return(FAILURE);
    }

  RDE = NULL;

  S = (s3rm_t *)R->S;      

  /* NOTE:  get data does not check response 'Status' element */

  if (S->UpdateIteration == MSched.Iteration)
    {
    /* data already updated */

    MDB(1,fS3) MLog("INFO:     S3 data already up to date\n");       

    return(SUCCESS);
    }

  /* clear old data */

  MQueueErrMsg[0]    = '\0';
  MWorkloadErrMsg[0] = '\0';
  MResourceErrMsg[0] = '\0';

  switch (R->Version)
    {
    case msssV3_0:
    case msssV4_0:

      MXMLDestroyE((mxml_t **)&S->WorkloadBuffer);
      MXMLDestroyE((mxml_t **)&S->ResourceBuffer);
      MXMLDestroyE((mxml_t **)&S->QueueBuffer);

      break;

    case msssV0_2:
    default:

      MUFree(&S->WorkloadBuffer);
      MUFree(&S->ResourceBuffer);
      MUFree(&S->QueueBuffer);

      break;
    }  /* END switch (R->Version) */

  /* form S3 XML full resource query */

  if (QueueReqString == NULL)
    {
    /* build queue manager queue status request */

    /* form S3 XML full workload query (NM) */

    if ((R->Version == msssV3_0) || (R->Version == msssV4_0))
      {
      MXMLCreateE(&E,MSON[msonRequest]);

      MXMLSetAttr(
        E,
        MSAN[msanAction],
        (void *)MS3JAName[R->Version][mrmJobQuery],
        mdfString);

      MS3SetObject(E,MS3ObjName[R->Version][mxoQueue],NULL);
      }    /* END if ((R->Version == msssV3_0) || ...) */

    MXMLToString(E,tmpString,sizeof(tmpString),NULL,TRUE);

    MXMLDestroyE(&E);

    QueueReqString = strdup(tmpString);

    /* NOTE:  only query once at start up (temp) */

    MS3DoCommand(
      &R->P[0],
      QueueReqString,
      NULL,
      &RDE,
      NULL,
      MQueueErrMsg);

    if ((R->Version == msssV3_0) || (R->Version == msssV4_0))
      {
      MMovePtr((char **)&RDE,&S->QueueBuffer);

      MDB(3,fS3) MLog("INFO:     data read on queue query\n");

      MSUFree(R->P[0].S);
      }
    else
      {
      MMovePtr(&R->P[0].S->RBuffer,&S->QueueBuffer);
      }
    }    /* END if (QueueReqString == NULL) */

  if (JobReqString == NULL)
    {
    /* form S3 XML full workload query */

    /* NOTE:  request is dependent upon language version */

    if (MISSET(R->Flags,mrmfLocalQueue))
      {
      /* NO-OP */
      }
    else if ((R->Version == msssV3_0) || (R->Version == msssV4_0))
      {
      int aindex;

      const enum MJobAttrEnum JAttrList[] = {
        mjaJobName,
        mjaJobID,
        mjaUser,
        mjaGroup,
        mjaAccount,
        mjaState,
        mjaAWDuration,
        mjaReqSMinTime,
        mjaRMXString,
        mjaAllocNodeList,
        mjaSubmitTime,
        mjaStartTime,
        mjaCompletionTime,
        mjaUserPrio,
        mjaNONE };

      const enum MReqAttrEnum RAttrList[] = {
        mrqaReqClass,
        mrqaNCReqMin,
        mrqaTCReqMin,
        mrqaNONE };

      MXMLCreateE(&E,MSON[msonRequest]);

      MXMLSetAttr(E,MSAN[msanAction],(void *)MS3Action[msssaQuery],mdfString);
      MS3SetObject(E,MS3ObjName[R->Version][mxoJob],NULL);

      /* specify query constraints */

      MS3AddWhere(E,"JobClass","active",NULL);

      /* request attributes */

      for (aindex = 0;JAttrList[aindex] != mjaNONE;aindex++)
        {
        MS3AddGet(E,MS3JobAttr[R->Version][JAttrList[aindex]],NULL);
        }  /* END for (aindex) */

      for (aindex = 0;RAttrList[aindex] != mrqaNONE;aindex++)
        {
        MS3AddGet(E,MS3ReqAttr[R->Version][RAttrList[aindex]],NULL);
        }  /* END for (aindex) */

      /* request high-level node properties attribute */

      MS3AddGet(E,"NodeProperties",NULL);
      }    /* END if ((R->Version == msssV3_0) || ...) */

    MXMLToString(E,tmpString,sizeof(tmpString),NULL,TRUE);

    MXMLDestroyE(&E);   

    JobReqString = strdup(tmpString);
    }    /* END if (JobReqString == NULL) */

  MS3DoCommand(
    &R->P[0],
    JobReqString,
    NULL,
    &RDE,
    NULL,
    MWorkloadErrMsg);

  if ((R->Version == msssV3_0) || (R->Version == msssV4_0))
    {
    MMovePtr((char **)&RDE,&S->WorkloadBuffer);

    MDB(3,fS3) MLog("INFO:     data read on workload query\n");

    MSUFree(R->P[0].S);
    }
  else
    {
    MMovePtr(&R->P[0].S->RBuffer,&S->WorkloadBuffer);
    }

  /* form S3 XML full resource query */

  if (ResReqString == NULL)
    {
    /* build node monitor resource request */

    /* form S3 XML full resource query (NM) */

    if ((R->Version == msssV3_0) || (R->Version == msssV4_0))
      {
      int     aindex;

      enum MNodeAttrEnum NAttrList[] = {
        mnaNodeID,
        mnaNodeState,
        mnaRCProc, 
        mnaRCMem,
        mnaRAMem,
        mnaRCSwap,
        mnaRASwap,
        mnaNONE };

      MXMLCreateE(&E,MSON[msonRequest]);

      MXMLSetAttr(E,MSAN[msanAction],(void *)MS3Action[msssaQuery],mdfString);

      MS3SetObject(E,MS3ObjName[R->Version][mxoNode],NULL);

      for (aindex = 0;NAttrList[aindex] != mnaNONE;aindex++)
        {
        MS3AddGet(E,MS3NodeAttr[MCONST_S3XPATH][NAttrList[aindex]],NULL);
        }  /* END for (aindex) */
      }    /* END if ((R->Version == msssV3_0) || ...) */
 
    MXMLToString(E,tmpString,sizeof(tmpString),NULL,TRUE);
 
    MXMLDestroyE(&E);
 
    ResReqString = strdup(tmpString);
    }  /* END if (ResReqString == NULL) */

  /* if psi 1 host is not set, inherit psi 0 server specification */

  if ((R->P[1].HostName == NULL) && (R->P[0].HostName != NULL))
    MUStrDup(&R->P[1].HostName,R->P[0].HostName);

  P = (R->P[1].Port == 0) ? &R->P[0] : &R->P[1];

  rc = MS3DoCommand(
    P,
    ResReqString,
    NULL,
    &RDE,
    NULL,
    MResourceErrMsg);

  if ((R->Version == msssV3_0) || (R->Version == msssV4_0))
    {
    MMovePtr((char **)&RDE,&S->ResourceBuffer); 

    MDB(3,fS3) MLog("INFO:     data read on cluster query\n");

    MSUFree(P->S);
    }
  else
    {
    MMovePtr(&P->S->RBuffer,&S->ResourceBuffer);
    }

  S->UpdateIteration = MSched.Iteration;     

  MDB(1,fS3) MLog("INFO:     S3 data updated for iteration %d (rc: %d)\n",
    S->UpdateIteration,
    rc);       

  return(SUCCESS);
  }  /* END MS3GetData() */



 
int MS3QueueQuery(

  mrm_t                *R,      /* I */
  int                  *QCount, /* O */
  char                 *EMsg,   /* O (optional,minsize=MMAX_LINE) */
  enum MStatusCodeEnum *SC)     /* O (optional) */

  {
  mclass_t *C;

  char   QID[MMAX_NAME];
  int    Status;

  char  *ptr;

  s3rm_t *S;

  mxml_t *E = NULL;
  mxml_t *RE;
  mxml_t *DE = NULL;
  mxml_t *QE;

  mnode_t *N;

  mnalloc_t *NL;

  int      CTok;

  int      cindex;
  int      nindex;

#ifndef __MPROD
  const char *FName = "MS3QueueQuery";

  MDB(1,fS3) MLog("%s(%s,QCount,EMsg,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");
#endif /* !__MPROD */

  if (QCount != NULL)
    *QCount = 0;

  if (EMsg != NULL)
    EMsg[0] = '\0';

  if (SC != NULL)
    *SC = mscNoError;

  if ((R == NULL) || (R->S == NULL))
    {
    return(FAILURE);
    }

  if (MISSET(R->Flags,mrmfLocalQueue))
    {
    return(SUCCESS);
    }

  S = (s3rm_t *)R->S;

  if (S->UpdateIteration != MSched.Iteration)
    {
    MS3GetData(R,FALSE,NULL);
    }

  /* step through list */

  if (S->QueueBuffer == NULL)
    {
    if (EMsg != NULL)
      strcpy(EMsg,MQueueErrMsg);

    return(FAILURE);
    }

  if ((R->Version == msssV3_0) || (R->Version == msssV4_0))
    {
    DE = (mxml_t *)S->QueueBuffer;
    }
  else if (R->Version == msssV0_2)
    {
    if (((ptr = strstr(S->QueueBuffer,"<Body")) == NULL) ||
         (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE))
      {
      /* cannot parse response */

      MUFree(&S->QueueBuffer);

      return(FAILURE);
      }

    MUFree(&S->QueueBuffer);

    if (MXMLGetChildCI(E,MSON[msonResponse],NULL,&RE) == FAILURE)
      {
      /* cannot parse response */

      MXMLDestroyE(&E);

      return(FAILURE);
      }

    if (MXMLGetChildCI(RE,MSON[msonData],NULL,&DE) == FAILURE)
      {
      /* cannot parse response */

      MXMLDestroyE(&E);

      return(FAILURE);
      }
    }
  else
    {
    /* unexpected version */

    return(FAILURE);
    }

  CTok = -1;

  while(MXMLGetChildCI(DE,MS3ObjName[R->Version][mxoQueue],&CTok,&QE) == SUCCESS)
    {
    QID[0] = '\0';

    if ((__MS3QueueGetState(QE,R,QID,&Status) == FAILURE) ||
        (MClassAdd(QID,&C) == FAILURE))
      {
      /* cannot add class */

      continue;
      }

    if ((Status == mclsDisabled) || (Status == mclsStopped))
      {
      C->IsDisabled = TRUE;
      }

    for (cindex = 0;cindex < QE->CCount;cindex++)
      {
      __MS3QueueSetAttr(C,QE->C[cindex]->Name,QE->C[cindex]->Val);
      }  /* END for (cindex) */

    /* associate queues w/nodes */

    NL = (mnalloc_t *)C->NodeList;

    for (nindex = 0;nindex < MMAX_NODE;nindex++)
      {
      N = MNode[nindex];

      if ((N == NULL) || (N->Name[0] == '\0'))
        break;

      if (N->Name[0] == '\1')
        continue;

      if (NL != NULL)
        {
        int index;

        for (index = 0;NL[index].N != NULL;index++)
          {
          if (NL[index].N == N)
            break;
          }  /* END for (index) */

        if (NL[index].N == NULL)
          {
          /* node not in nodelist */

          continue;
          }
        }

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

    if (QCount != NULL)
      (*QCount)++;
    }    /* END while(MXMLGetChildCI(DE,MS3ObjName[R->Version][mxoQueue],&CTok,&QE) == SUCCESS) */

  MDB(1,fS3) MLog("INFO:     all queues loaded\n");

  return(SUCCESS);
  }  /* END MS3QueueQuery() */



 
int MS3WorkloadQuery(

  mrm_t                *R,       /* I */
  int                  *WCount,  /* O (optional) */
  enum MStatusCodeEnum *SC)      /* O (optional) */

  {
  mjob_t *J;
  
  enum MJobStateEnum Status;

  char   SJobID[MMAX_NAME];
  char   RMJID[MMAX_NAME];

  short  TaskList[MMAX_TASK_PER_JOB + 1];
  char   Message[MMAX_LINE];

  char  *ptr;

  s3rm_t *S;

  mxml_t *E = NULL;
  mxml_t *RE;
  mxml_t *DE;
  mxml_t *JE;

  mxml_t *StepE;
  mxml_t *NextSE;

  int      JCTok;
  int      SCTok;

  const char *FName = "MS3WorkloadQuery";

#ifndef __MPROD
  MDB(1,fS3) MLog("%s(%s,WCount,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");
#endif /* !__MPROD */

  if (WCount != NULL)
    *WCount = 0;

  if (SC != NULL)
    *SC = mscNoError;
 
  if ((R == NULL) || (R->S == NULL))
    {
    return(FAILURE);
    }

  if (!MISSET(R->Flags,mrmfLocalQueue))
    {
    S = (s3rm_t *)R->S;

    if (S->UpdateIteration != MSched.Iteration)
      {
      MS3GetData(R,FALSE,NULL);
      }
 
    if (S->WorkloadBuffer == NULL)
      {
      return(FAILURE);
      }
    }    /* END if (MISSET(R->Flags,mrmfLocalQueue) */

  if (MISSET(R->Flags,mrmfLocalQueue))
    {
    char  **JL;

    mjob_t *J;

    int     jindex;

    JL = (char **)R->U.S3.JobList;

    for (jindex = 0;jindex < MMAX_JOB;jindex++)
      {
      if (JL[jindex] == NULL)
        break;

      if (JL[jindex] == (char *)1)
        continue;

      if (MJobFind(JL[jindex],&J,mjsmExtended) == FAILURE)
        {
        /* NOTE:  If a job has not been found then it is a local queue job *
         *        and must be created here. Job information must come from *
         *        the checkpoint file, if no information is found, destroy job */

        TaskList[0] = -1;

        if (MJobCreate(JL[jindex],TRUE,&J) == FAILURE)
          {
          /* cannot create local job */

          continue;
          }

#ifdef __M32COMPAT
        MRMJobPreLoad(J,JL[jindex],R->Index);
#else /* __M32COMPAT */
        MRMJobPreLoad(J,JL[jindex],R);
#endif /* __M32COMPAT */

        /* migrate JL[jindex] data to J (NYI) */

#ifdef __M32COMPAT
        if (MRMJobPostLoad(J,TaskList,R) == FAILURE)
#else /* __M32COMPAT */
        if (MRMJobPostLoad(J,TaskList,R,NULL) == FAILURE)
#endif /* __M32COMPAT */
          {
          /* Invalid job -- insufficient data recorded in checkpoint file to instantiate job */

          MDB(2,fS3) MLog("ALERT:    invalid job %s in internal queue (removing job)\n",
            J->Name);
         
          /* NOTE:  complete job rather than remove it (NYI) */

          /* add failure message */

#ifdef __M32COMPAT
          MJobRemove(J);
#else /* __M32COMPAT */
          MJobRemove(&J);
#endif /* __M32COMPAT */
        
          continue;
          }

#ifndef __M32COMPAT
        if (MISSET(J->SpecFlags,mjfRsvMap))
          {
          mrsv_t *R;

          if (J->ReqRID == NULL)
            {
            MJobRemove(&J);

            continue;
            }

          if (MRsvFind(J->ReqRID,&R,mraNONE) == FAILURE)
            {
            MJobRemove(&J);

            continue;
            }

          R->J = J;
          }
#endif /* !__M32COMPAT */

        if (J->Request.TC <= 1)
          {
          if (J->Request.NC == 0)
            {
            MDB(2,fS3) MLog("ALERT:    no job task info located for job '%s' (assigning taskcount to 1)\n",
              J->Name);
            }

          if (J->ReqHList != NULL)
            {
            int nc;
            int tc;

            tc = 0;

            for(nc=0;J->ReqHList[nc].N != NULL;nc++)
              {
              tc += J->ReqHList[nc].TC;
              }

            J->Request.TC = MAX(1,tc);
            J->Req[0]->TaskCount = MAX(1,tc);
            J->Req[0]->NodeCount = MAX(1,nc);
            J->Request.NC = MAX(1,nc);

            J->Req[0]->TaskRequestList[0] = MAX(1,tc);
            J->Req[0]->TaskRequestList[1] = MAX(1,tc);
            }
          else
            {
            J->Request.TC = MAX(1,J->Request.NC);
            J->Req[0]->TaskCount = MAX(1,J->Request.NC);
            J->Req[0]->NodeCount = MAX(1,J->Request.NC);
            J->Request.NC = MAX(1,J->Request.NC);

            J->Req[0]->TaskRequestList[0] = MAX(1,J->Request.NC);
            J->Req[0]->TaskRequestList[1] = MAX(1,J->Request.NC);
            }
          }  /* END if (J->Request.TC <= 1) */

        if (WCount != NULL)
          (*WCount)++;

        J->ATime = MSched.Time;
        }
      else if (MISSET(J->Flags,mjfNoRMStart))
        {
        switch (J->State)
          {
          case mjsIdle:
          case mjsStarting:
          case mjsRunning:
          case mjsSuspended:
          case mjsHold:

            MRMJobPreUpdate(J);

            if (MS3JobUpdate(NULL,&J,TaskList,R) == FAILURE)
              {
              MDB(1,fS3) MLog("ALERT:    cannot update S3 job '%s'\n",
                J->Name);

              continue;
              }

            MRMJobPostUpdate(J,TaskList,J->State,R);

            MDB(2,fS3)
              MJobShow(J,0,NULL);

            J->ATime = MSched.Time;

            break;

          case mjsRemoved:
          case mjsCompleted:
          case mjsVacated:

            /* if job never ran, remove record.  job cancelled externally */

            if (MJOBISACTIVE(J) == FALSE)
              {
              MDB(1,fS3) MLog("INFO:     job '%s' was cancelled externally\n",
                J->Name);

              /* remove job from joblist */

              MS3RemoveLocalJob(R,J->Name);

              MJOBREMOVE(&J);

              break;
              }

            MRMJobPreUpdate(J);

            if (MS3JobUpdate(NULL,&J,TaskList,R) == FAILURE)
              {
              MDB(1,fS3) MLog("ALERT:    cannot update S3 job '%s'\n",
                SJobID);

              continue;
              }

            MRMJobPostUpdate(J,TaskList,Status,R);

            MSysEMSubmit(
              &MSched.EM,
              (char *)MS3CName[mpstSC],
              "job-terminated",
              J->Name);

#ifndef __M32COMPAT
            if (MISSET(J->SRM->Flags,mrmfFullCP))
              {
              char FileName[MMAX_PATH_LEN];
  
              sprintf(FileName,"%s/%s.cp",
                MSched.SpoolDir,
                J->Name);

              remove(FileName);
              }
#endif /* !__M32COMPAT */

            switch (Status)
              {
              case mjsRemoved:
              case mjsVacated:

                if (MSched.Time < (J->StartTime + J->WCLimit))
                  {
                  MS3RemoveLocalJob(R,J->Name);

                  MJOBPROCESSREMOVED(&J);
                  }
                else
                  {
                  sprintf(Message,"JOBWCVIOLATION:  job '%s' exceeded WC limit %s\n",
                    J->Name,
                    MULToTString(J->WCLimit));

                  MSysRegEvent(Message,mactNONE,0,1);

                  MDB(3,fS3) MLog("INFO:     job '%s' exceeded wallclock limit %s\n",
                    J->Name,
                    MULToTString(J->WCLimit));

                  MS3RemoveLocalJob(R,J->Name);

                  MJOBPROCESSCOMPLETED(&J);
                  }

                break;

              case mjsCompleted:

                MS3RemoveLocalJob(R,J->Name);

                MJOBPROCESSCOMPLETED(&J);

                break;

              default:

                /* unexpected job state */
 
                MDB(1,fS3) MLog("WARNING:  unexpected job state (%d) detected for job '%s'\n",
                  Status,
                  J->Name);
 
                break;
              }   /* END switch (Status) */

            break;

          default:

            /* NO-OP */
  
            break;
          }     /* END switch (J->State) */
        }       /* END else if (MISSET(J->Flags,mjfNoStart)) */

      /* refresh job */

      J->ATime = MSched.Time;
      }         /* END for (jindex) */
    }
  else if ((R->Version == msssV3_0) || (R->Version == msssV4_0))
    {
    DE = (mxml_t *)S->WorkloadBuffer;

    if (DE == NULL)
      {
      return(FAILURE);
      }
    }
  else if (R->Version == msssV0_2)
    {
    if (((ptr = strstr(S->WorkloadBuffer,"<Body")) == NULL) ||
         (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE))
      {
      /* cannot parse response */
  
      MUFree(&S->WorkloadBuffer);
 
      return(FAILURE);
      }

    MUFree(&S->WorkloadBuffer);

    if (MXMLGetChildCI(E,MSON[msonResponse],NULL,&RE) == FAILURE)
      {
      /* cannot parse response */
 
      MXMLDestroyE(&E);
 
      return(FAILURE);
      }

    if (MXMLGetChildCI(RE,MSON[msonData],NULL,&DE) == FAILURE)
      {
      /* cannot parse response */

      MXMLDestroyE(&E);

      return(FAILURE);
      }
    }
  else
    {
    /* unexpected version */

    return(FAILURE);
    }

  JCTok = -1;

  if (MISSET(R->Flags,mrmfLocalQueue))
    {
    /* NO-OP */
    }
  else while (MXMLGetChildCI(DE,MS3ObjName[R->Version][mxoJob],&JCTok,&JE) == SUCCESS)
    {
    /* load job default config */

    /* NYI */

    SCTok = -1;

    if (MXMLGetChildCI(JE,"Step",&SCTok,&NextSE) == FAILURE)
      {
      /* load single step job */

      NextSE = JE;
      }

    if (NextSE == NULL)
      {
      continue;
      }

    while (NextSE != NULL)
      {
      StepE = NextSE;

      MXMLGetChildCI(JE,"Step",&SCTok,&NextSE);

      /* NYI */

      RMJID[0] = '\0';

      if (__MS3JobStepGetState(StepE,R,RMJID,&Status) == FAILURE)
        {
        /* job object is corrupt */

        MDB(1,fS3) MLog("ALERT:    cannot determine jobid/state in %s (ignoring job)\n",
          FName);

        continue;
        }

      MJobGetName(NULL,RMJID,R,SJobID,MMAX_NAME,mjnShortName);

      if (WCount != NULL)
        (*WCount)++;

      switch (Status)
        {
        case mjsIdle:
        case mjsStarting:
        case mjsRunning:
        case mjsSuspended:
        case mjsHold:
 
          if (MJobFind(SJobID,&J,mjsmBasic) == SUCCESS)
            {
            MRMJobPreUpdate(J);

            if (MS3JobUpdate(StepE,&J,TaskList,R) == FAILURE)
              {
              MDB(1,fS3) MLog("ALERT:    cannot update S3 job '%s'\n",
                SJobID);

              continue;
              }
 
            MRMJobPostUpdate(J,TaskList,Status,R);
            }
          else 
            {
            if (MJobCreate(SJobID,TRUE,&J) == FAILURE)
              {
              MDB(1,fS3) MLog("ERROR:    job buffer is full  (ignoring job '%s')\n",
                SJobID);
 
              break;
              }

            /* if new job, load data */

#ifdef __M32COMPAT
            MRMJobPreLoad(J,SJobID,R->Index);
#else /* __M32COMPAT */
            MRMJobPreLoad(J,SJobID,R);
#endif /* __M32COMPAT */

            MJobSetAttr(J,mjaSRMJID,(void **)RMJID,mdfString,mSet);
            MJobSetAttr(J,mjaDRMJID,(void **)RMJID,mdfString,mSet);
 
            if (MS3JobLoad(SJobID,StepE,&J,TaskList,R) == FAILURE)
              {
              MDB(1,fS3) MLog("ALERT:    cannot load S3 job '%s'\n",
                SJobID);
 
              continue;
              }

#ifdef __M32COMPAT
            MRMJobPostLoad(J,TaskList,R);
#else /* __M32COMPAT */
            MRMJobPostLoad(J,TaskList,R,NULL);
#endif /* __M32COMPAT */

            /* NOTE:  all MJobProcessExtensionString() calls should be moved 
                      to MRMJobPostLoad() */

            if (J->RMXString != NULL)
              {
#ifdef __M32COMPAT
              if (MJobProcessExtensionString(J,J->RMXString) == FAILURE)
#else /* __M32COMPAT */
              if (MJobProcessExtensionString(J,J->RMXString,mxaNONE) == FAILURE)
#endif /* __M32COMPAT */
                {
                MDB(1,fS3) MLog("ALERT:    cannot process extension string for job '%s'\n",
                  SJobID);

                /* NO-OP */
                }
              }
            }  /* END else (MJobFind(SJobID,&J,mjsmBasic) == SUCCESS) */

          MDB(2,fS3)
            MJobShow(J,0,NULL);
 
          break;
 
        case mjsRemoved:
        case mjsCompleted:
        case mjsVacated:
 
          if (MJobFind(SJobID,&J,mjsmBasic) == SUCCESS)
            {
            /* if job never ran, remove record.  job cancelled externally */
 
            if (MJOBISACTIVE(J) == FALSE)
              {
              MDB(1,fS3) MLog("INFO:     job '%s' was cancelled externally\n",
                J->Name);
 
              /* remove job from joblist */
 
              MJOBREMOVE(&J);
 
              break;
              }

            MRMJobPreUpdate(J);
 
            if (MS3JobUpdate(StepE,&J,TaskList,R) == FAILURE)
              {
              MDB(1,fS3) MLog("ALERT:    cannot update S3 job '%s'\n",
                SJobID);

              continue;
              }
 
            MRMJobPostUpdate(J,TaskList,Status,R);

            MSysEMSubmit(
              &MSched.EM,
              (char *)MS3CName[mpstSC],
              "job-terminated",
              J->Name);
 
            switch (Status)
              {
              case mjsRemoved:
              case mjsVacated:
 
                if (MSched.Time < (J->StartTime + J->WCLimit))
                  {
                  MJOBPROCESSREMOVED(&J);
                  }
                else
                  {
                  sprintf(Message,"JOBWCVIOLATION:  job '%s' exceeded WC limit %s\n",
                    J->Name,
                    MULToTString(J->WCLimit));
 
                  MSysRegEvent(Message,mactNONE,0,1);

                  MDB(3,fS3) MLog("INFO:     job '%s' exceeded wallclock limit %s\n",
                    J->Name,
                    MULToTString(J->WCLimit));
 
                  MJOBPROCESSCOMPLETED(&J);
                  }
 
                break;
 
              case mjsCompleted:
 
                MJOBPROCESSCOMPLETED(&J);
 
                break;

              default:
 
                /* unexpected job state */
 
                MDB(1,fS3) MLog("WARNING:  unexpected job state (%d) detected for job '%s'\n",
                  Status,
                  J->Name);
 
                break;
              }   /* END switch (Status)                        */
            }     /* END if (MJobFind(SJobID,&J,mjsmBasic) == SUCCESS) */
          else
            {
            /* ignore job */
 
            MDB(4,fS3) MLog("INFO:     ignoring job '%s'  (state: %s)\n",
              SJobID,
              MJobState[Status]);
            }
 
          break;
 
        default:

          MDB(1,fS3) MLog("WARNING:  job '%s' detected with unexpected state '%d'\n",
            SJobID,
            Status);

          /* NO-OP */
 
          break;
        }  /* END switch (Status) */
      }    /* END while(NextSE != NULL) */

    MDB(1,fS3) MLog("INFO:     all steps loaded for job\n");
    }    /* END while (MXMLGetChildCI(DE,MS3ObjName[R->Version][mxoJob],&JCTok,&JE) == SUCCESS) */

  MDB(1,fS3) MLog("INFO:     all jobs loaded\n");

  MXMLDestroyE(&E);

  /* TEMP:  remove jobs not detected via S3 */

  if (R->FailIteration != MSched.Iteration)
    {
    mjob_t *JNext;
    enum MJobStateEnum OldState;

    for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = JNext)
      {
      JNext = J->Next; /* store next job pointer in case current job is removed */

#ifndef __M32COMPAT
      if ((J->SRM != R) ||
         ((J->SRM == R) && (J->DRM != NULL) && (J->DRM != R)))
        {
        continue;
        }
#endif /* !__M32COMPAT */

      if ((J->ATime > 0) &&
          (MSched.Time - J->ATime > MSched.JobPurgeTime))
        {
        if (MJOBISACTIVE(J) == TRUE)
          {
          MDB(1,fPBS) MLog("INFO:     active S3 job %s has been removed from the queue.  assuming successful completion\n",
            J->Name);

          MRMJobPreUpdate(J);

          /* assume job completed successfully for now */

          OldState          = J->State;

          J->State          = mjsCompleted;

          J->CompletionTime = J->ATime;

          MRMJobPostUpdate(J,NULL,OldState,J->SRM);

          MJOBPROCESSCOMPLETED(&J);
          }
        else
          {
          MDB(1,fPBS) MLog("INFO:     non-active S3 job %s has been removed from the queue.  assuming job was cancelled\n",
            J->Name);

          /* just remove job */

          MJOBREMOVE(&J);
          }
        }
      else if (MISSET(J->Flags,mjfNoRMStart) &&
#ifndef __M32COMPAT
               (!MISSET(J->Flags,mjfRsvMap)) && 
#endif /* __M32COMPAT */
               (J->StartTime != 0) &&
               (J->StartTime + J->WCLimit <= MSched.Time))
        {
        int nindex;
        int TC;

        mnode_t *N;

        mreq_t *RQ = J->Req[0];

        /* assume system job completed successfully */

        MDB(1,fPBS) MLog("INFO:     active S3 job %s has been removed from the queue.  assuming successful completion\n",
          J->Name);

        for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
          {
          N = J->NodeList[nindex].N;

          if ((RQ->NAccessPolicy == mnacSingleJob) ||
              (RQ->NAccessPolicy == mnacSingleTask))
            {
            MCResRemove(&N->DRes,&N->CRes,&N->CRes,1,TRUE);

            MCResAdd(&N->ARes,&N->CRes,&N->CRes,1,FALSE);
            }
          else
            {
            TC = J->NodeList[nindex].TC;

            MCResRemove(&N->DRes,&N->CRes,&RQ->DRes,TC,TRUE);

            MCResAdd(&N->ARes,&N->CRes,&RQ->DRes,TC,FALSE);
            }
          }

        MRMJobPreUpdate(J);

        OldState          = J->State;

        J->State          = mjsCompleted;

        J->CompletionTime = J->ATime;

        MRMJobPostUpdate(J,NULL,OldState,J->SRM);

        MJOBPROCESSCOMPLETED(&J);
        }
      }    /* END for (jindex) */
    }      /* END if (R->FailIteration != MSched.Iteration) */

  return(SUCCESS);
  }  /* END MS3WorkloadQuery() */




int __MS3QueueGetState(

  mxml_t   *QE,      /* I */
  mrm_t    *R,       /* I */
  char     *QID,     /* O */
  int      *Status)  /* O */

  {
  mxml_t *CE;

  if (Status != NULL)
    *Status = mclsNONE;

  if ((QE == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  if ((QID != NULL) && (QID[0] == '\0'))
    {
    /* get queue name */

    if (MXMLGetChildCI(QE,MS3QueueAttr[mclaName],NULL,&CE) == FAILURE)
      {
      return(FAILURE);
      }

    MUStrCpy(QID,CE->Val,MMAX_NAME);
    }  /* END if (QID != NULL) */

  if (Status != NULL)
    {
    if (MXMLGetChildCI(QE,MS3QueueAttr[mclaState],NULL,&CE) == FAILURE)
      {
      /* class is available by default */

      *Status = mclsActive;
      }
    else
      {
      /* 
         mclsActive    accepting jobs, executing jobs 
         mclsDisabled  not accepting jobs, not executing jobs 
         mclsClosed    not accepting jobs, executing jobs 
         mclsStopped   accepting jobs, not executing jobs
      */

      if (MUBoolFromString(CE->Val,FALSE) == TRUE)
        {
        *Status = mclsActive;
        }
      else
        {
        *Status = mclsStopped;
        }
      }
    }    /* END if (Status != NULL) */

  return(SUCCESS);
  }  /* END __MS3QueueGetState() */




int __MS3JobStepGetState(

  mxml_t   *StepE,   /* I */
  mrm_t    *R,       /* I */
  char     *StepID,  /* O */
  enum MJobStateEnum *Status)  /* O */

  {
  mxml_t *XE;

  if (Status != NULL)
    *Status = mjsNONE;

  if ((StepE == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  if ((StepID != NULL) && (StepID[0] == '\0'))
    {
    switch (R->Version)
      {
      case msssV3_0:
      case msssV4_0:
      default:
 
        {
        mxml_t *StepIDE = NULL;

        if (MXMLGetChildCI(
              StepE,
              MS3JobAttr[R->Version][mjaJobID],
              NULL,
              &StepIDE) == SUCCESS)
          {
          MUStrCpy(StepID,StepIDE->Val,MMAX_NAME);
          }
        else
          {
          /* cannot determine step ID */
   
          return(FAILURE);
          }
        }  /* END BLOCK */

        break;

      case msssV0_2:

        /* get job id */

        {
        mxml_t *C;

        if (MXMLGetChildCI(StepE,MS3JobAttr[R->Version][mjaJobID],NULL,&C) == FAILURE)         
          {
          return(FAILURE);
          }

        MUStrCpy(StepID,C->Val,MMAX_NAME);
        }  /* END BLOCK */

        break;
      }  /* END switch (R->Version) */
    }    /* END if (StepID != NULL) */

  if (Status != NULL)
    {
    switch (R->Version)
      {
      case msssV3_0:
      case msssV4_0:

        {
        mxml_t *StateE = NULL;

        if (MXMLGetChildCI(StepE,MS3JobAttr[R->Version][mjaState],NULL,&StateE) == FAILURE)
          {
          return(FAILURE);
          }
    
        MUStrToLower(StateE->Val);
 
        *Status = (enum MJobStateEnum)MUGetIndex(StateE->Val,MS3JobState,FALSE,0);
        }  /* END BLOCK */

        break;

      case msssV0_2:
      default:

        {
        XE = NULL;

        if (MXMLGetChildCI(StepE,MS3JobAttr[R->Version][mjaState],NULL,&XE) == FAILURE)
          {
          mxml_t *C2;

          if (MXMLGetChildCI(StepE,MS3JobAttr[R->Version][mjaAWDuration],NULL,&C2) == SUCCESS)
            { 
            if (!strcmp(C2->Val,"0"))
              *Status = mjsIdle;
            else
              *Status = mjsRunning;
            }
          else
            {
            *Status = mjsNotQueued;
            }
          }
        else
          {
          if ((*Status = (enum MJobStateEnum)MUGetIndex(XE->Val,MJobState,FALSE,0)) == 0)
            {
            *Status = (enum MJobStateEnum)MUGetIndex(XE->Val,MS3JobState,FALSE,0);
            }
          }    /* END else (MXMLGetChildCI() == FAILURE) */
        }      /* END BLOCK */
      }        /* END switch (R->Version) */
    }          /* END if (Status != NULL) */

  return(SUCCESS);
  }  /* END __MS3JobStepGetState() */





int MS3ClusterQuery(
 
  mrm_t                *R,      /* I */
  int                  *RCount, /* O */
  char                 *EMsg,   /* O (optional,minsize=MMAX_LINE) */
  enum MStatusCodeEnum *SC)     /* I */
 
  {
  char  NodeName[MMAX_NAME];

  char *ptr;

  enum MNodeStateEnum Status;
  enum MNodeStateEnum OldState;

  mnode_t *N;

  s3rm_t *S;

  mbool_t  NewNode = FALSE;

  mxml_t  *RE = NULL;
  mxml_t  *DE = NULL;
  mxml_t  *NE = NULL;
  mxml_t  *E  = NULL;

  int      CTok;

  const char *FName = "MS3ClusterQuery";
   
#ifndef __MPROD
  MDB(1,fS3) MLog("%s(%s,RCount,SC)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");
#endif /* !__MPROD */
 
  if (RCount != NULL)
    *RCount = 0;

  if (EMsg != NULL)
    EMsg[0] = '\0';

  if (SC != NULL)
    *SC = mscNoError;
 
  if ((R == NULL) || (R->S == NULL))
    {
    MDB(1,fS3) MLog("ALERT:    invalid parameters passed to %s\n",
      FName);

    if (EMsg != NULL)
      strcpy(EMsg,"invalid parameters");

    return(FAILURE);
    }

  if (MISSET(R->Flags,mrmfLocalQueue))
    {  
    return(SUCCESS);
    }

  S = (s3rm_t *)R->S;

  /* step through list */

  if (S->UpdateIteration != MSched.Iteration)
    {
    MS3GetData(R,FALSE,NULL);
    }

  if (S->ResourceBuffer == NULL) 
    {
    MDB(1,fS3) MLog("ALERT:    no resource data available in %s\n",
      FName);

    if (EMsg != NULL)
      strcpy(EMsg,MResourceErrMsg);

    return(FAILURE);
    }

  if ((R->Version == msssV3_0) || (R->Version == msssV4_0))
    {
    DE = (mxml_t *)S->ResourceBuffer;

    if (DE == NULL)
      {
      MDB(1,fS3) MLog("ALERT:    invalid XML element passed to %s\n",
        FName);

      return(FAILURE);
      }
    }
  else if (R->Version == msssV0_2)
    {
    if (((ptr = strstr(S->ResourceBuffer,"<Body")) == NULL) ||
         (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE))
      {
      /* cannot parse response */

      MUFree(&S->ResourceBuffer);

      MDB(1,fS3) MLog("ALERT:    cannot parse XML passed to %s\n",
        FName);

      return(FAILURE);
      }

    MUFree(&S->ResourceBuffer);

    if (MXMLGetChildCI(E,MSON[msonResponse],NULL,&RE) == FAILURE)
      {
      /* cannot parse response */

      MXMLDestroyE(&E);

      MDB(1,fS3) MLog("ALERT:    cannot locate '%s' element in %s\n",
        MSON[msonResponse],
        FName);

      return(FAILURE);
      }

    if (MXMLGetChildCI(RE,MSON[msonData],NULL,&DE) == FAILURE)
      {
      /* cannot parse response */

      MXMLDestroyE(&E);

      MDB(1,fS3) MLog("ALERT:    cannot locate '%s' element in %s\n",
        MSON[msonData],
        FName);

      return(FAILURE);
      }
    }
  else
    {
    /* unexpected version */

    MDB(1,fS3) MLog("ALERT:    unexpected S3 version '%d' in %s\n",
      R->Version,
      FName);

    return(FAILURE);
    }

  CTok = -1;

  while(MXMLGetChildCI(DE,MS3ObjName[R->Version][mxoNode],&CTok,&NE) == SUCCESS)
    {
    NodeName[0] = '\0';
 
    if (__MS3NodeGetState((void *)NE,R,NodeName,&Status) == FAILURE)
      {
      /* node object is corrupt */
 
      continue;
      }
 
    if (RCount != NULL)
      (*RCount)++;
 
    if (MNodeFind(NodeName,&N) == SUCCESS)
      {
      OldState = N->State;
 
      MRMNodePreUpdate(N,Status,R);
 
      MS3NodeUpdate(N,NE,Status,R);

#ifdef __M32COMPAT
      MRMNodePostUpdate(N,OldState);
#endif /* __M32COMPAT */
      }
    else if (MNodeAdd(NodeName,&N) == SUCCESS)
      {
      NewNode = TRUE;
 
      MRMNodePreLoad(N,Status,R);
 
      MS3NodeLoad(N,NE,Status,R);

#ifdef __M32COMPAT
      MRMNodePostLoad(N);
#endif /* __M32COMPAT */

      MDB(2,fS3)
        MNodeShow(N);
      }
    else
      {
      MDB(1,fS3) MLog("ERROR:    node buffer is full  (ignoring node '%s')\n",
        NodeName);
      }
    }    /* END for (cindex) */

  /* all nodes loaded */

  MXMLDestroyE(&E);

  if (NewNode == TRUE)
    {
    MS3QueueQuery(R,NULL,NULL,NULL);
    }
 
  return(SUCCESS);
  }  /* END MS3ClusterQuery() */





int MS3QueueLoadInfo(
 
  mrm_t   *R,     /* I */
  mnode_t *SpecN) /* I (optional) */

  {
#ifndef __MPROD
  const char *FName = "MS3QueueLoadInfo";

  MDB(1,fS3) MLog("%s(%s,%s)\n",
    FName,
    (R != NULL) ? R->Name : "NULL",
    (SpecN != NULL) ? SpecN->Name : "NULL");
#endif /* !__MPROD */

  if (R == NULL)
    {
    return(FAILURE);
    }

  if ((MSched.Time > (unsigned long)(R->U.S3.ServerSDTimeStamp + 3000)) ||
      (R->U.S3.ServerSD <= 0))
    {
    MS3Initialize(R,NULL);

    if (R->U.S3.ServerSD <= 0)
      {
      /* cannot recover S3 */

      MDB(1,fS3) MLog("ALERT:    cannot re-initialize S3 interface\n");

      return(FAILURE);
      }
    }

  return(SUCCESS);
  }  /* END MS3QueueLoadInfo() */





int __MS3NodeGetState(

  void  *NP,     /* I */
  mrm_t *R,      /* I */
  char  *NodeID, /* O */
  enum MNodeStateEnum *Status) /* O */

  {
  mxml_t *E;

#ifndef __MPROD
  const char *FName = "__MS3NodeGetState";

  MDB(1,fS3) MLog("%s(%s,%s,%s,%s)\n",
    FName,
    (NP != NULL) ? "NP" : "NULL",
    (R != NULL) ? R->Name : "NULL",
    (NodeID != NULL) ? "NodeID" : "NULL",
    (Status != NULL) ? "Status" : "NULL");
#endif /* !__MPROD */

  if ((NP == NULL) || (R == NULL))
    {
    return(FAILURE);
    }
 
  E = (mxml_t *)NP;
 
  if ((NodeID != NULL) && (NodeID[0] == '\0'))
    {
    /* get node name */
 
    mxml_t *C;
 
    if (MXMLGetChildCI(E,MS3NodeAttr[R->Version][mnaNodeID],NULL,&C) == FAILURE)
      {
      return(FAILURE);
      }

    strcpy(NodeID,C->Val); 
    }

  if (Status != NULL)
    {
    mxml_t *C = NULL;

    *Status = mnsNone;

    if (MXMLGetChildCI(E,MS3NodeAttr[R->Version][mnaNodeState],NULL,&C) == FAILURE)
      {
      /* node state not specified */

      *Status = mnsUnknown;      
      }
    else if ((R->Version == msssV3_0) ||
             (R->Version == msssV4_0) ||
             (R->Version == msssV0_2))
      {
      const char *S3NState[] = { NONE, "up", "down", NULL };
      const enum MNodeStateEnum S3NSEq[] = { mnsNone, mnsUnknown, mnsDown, mnsNONE };

      int index;

      /* translate S3 node state to internal node state */

      index = MUGetIndex(C->Val,S3NState,FALSE,mnsNONE);

      *Status = S3NSEq[index];
      }
    else
      {
      const char *S3NState[] = { NONE, "active", "busy", "down", "idle", NULL };
      const enum MNodeStateEnum S3NSEq[] = { mnsNone, mnsActive, mnsBusy, mnsDown, mnsIdle, mnsNONE };

      int index;

      /* translate S3 node state to internal node state */

      index = MUGetIndex(C->Val,S3NState,FALSE,mnsNONE);

      *Status = S3NSEq[index];
      }  /* END else () */

    MDB(1,fS3) MLog("INFO:     S3 node %s set to state %s (%s)\n",
      (NodeID != NULL) ? NodeID : NONE,
      MNodeState[*Status],
      (C != NULL) ? C->Val : "NULL");
    }    /* END if (Status != NULL) */
    
  return(SUCCESS);
  }  /* END __MS3NodeGetState() */





int MS3JobStart(

  mjob_t               *J,   /* I */
  mrm_t                *R,   /* I */
  char                 *Msg, /* O (optional,minsize=MMAX_LINE) */
  enum MStatusCodeEnum *SC)  /* O (optional) */

  {
  char        *Response = NULL;

  char         CmdString[MMAX_BUFFER];

  int          nindex;
  int          tindex;

  char        *MasterHost;

  char        *ptr;

  mnode_t *N;

  mxml_t *E  = NULL;
  mxml_t *RE = NULL;
  mxml_t *DE;
  mxml_t *JE;
  mxml_t *NE;
  mxml_t *C  = NULL;

  char       tmpMsg[MMAX_LINE];
  char       tmpLine[MMAX_LINE];

  enum MSFC  tmpSC;

#ifndef __MPROD
  const char *FName = "MS3JobStart";

  MDB(1,fS3) MLog("%s(%s,%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");
#endif /* __MPROD */

  if (SC != NULL)
    *SC = mscNoError;

  if ((J == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  MasterHost  = NULL;

  if (J->NodeList[0].N == NULL)
    {
    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be started: (empty hostlist)\n",
      J->Name);
 
    return(FAILURE);
    }

  switch (R->Version)
    {
    case msssV3_0:
    case msssV4_0:

      MXMLCreateE(&E,MSON[msonRequest]);
      MXMLSetAttr(E,MSAN[msanAction],(void *)MS3JAName[R->Version][mrmJobStart],mdfString);

      MS3SetObject(E,MS3ObjName[R->Version][mxoJob],NULL);

      DE = NULL;
      MXMLCreateE(&DE,MSON[msonData]);
      MXMLAddE(E,DE);

      JE = NULL;
      MXMLCreateE(&JE,MS3ObjName[R->Version][mxoJob]);
      MXMLAddE(DE,JE);

      C = NULL;
      MXMLCreateE(&C,MS3JobAttr[R->Version][mjaJobID]); 
      MXMLSetVal(C,(void *)J->Name,mdfString);
      MXMLAddE(JE,C);

      C = NULL;
      MXMLCreateE(&C,"NodeList");
      MXMLAddE(JE,C);
   
      for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
        {
        if (J->NodeList[nindex].N->RM != R)
          continue;

        for (tindex = 0;tindex < MAX(1,J->NodeList[nindex].TC);tindex++)
          {
          N = J->NodeList[nindex].N;

          NE = NULL;
          MXMLCreateE(&NE,MS3ObjName[R->Version][mxoNode]);
          MXMLSetVal(NE,(void *)N->Name,mdfString);
          MXMLAddE(C,NE);
          }  /* END for (tindex) */
        }    /* END for (tindex) */

      break;

    default:

      /* not supported */

      return(FAILURE);
    
      /*NOTREACHED*/

      break;
    }      /* END switch (R->Version) */

  MXMLToString(E,CmdString,sizeof(CmdString),NULL,TRUE);
 
  MXMLDestroyE(&E);

  if (MS3DoCommand(
        &R->P[0],
        CmdString,
        &Response,
        NULL,
        NULL,
        Msg) == FAILURE)
    {
    /* job could not be started */

    if (Response != NULL)
      {
      char tmpLine[MMAX_LINE];

      snprintf(tmpLine,sizeof(tmpLine),"cannot start job - %s",
        Response);

      free(Response);

      MUStrDup(&J->Message,tmpLine);
      }

    R->FailIteration = MSched.Iteration;

    if (SC != NULL)
      *SC = mscRemoteFailure;

    MSUFree(R->P[0].S);

    return(FAILURE);
    }

  if (Response != NULL)
    {
    if (((ptr = strstr(Response,"<Body")) == NULL) ||
         (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE))
      {
      /* cannot parse response */
 
      MUFree(&Response);

      R->FailIteration = MSched.Iteration;        

      MSUFree(R->P[0].S);
 
      return(FAILURE);
      }

    MUFree(&Response);     

    if (MXMLGetChildCI(E,MSON[msonResponse],NULL,&RE) == FAILURE)
      {
      /* cannot parse response */
 
      MXMLDestroyE(&E);

      R->FailIteration = MSched.Iteration;  
      }
    }

  /* check status code */

  if (MS3CheckStatus(RE,&tmpSC,tmpMsg) == FAILURE)
    {
    MXMLDestroyE(&E);

    R->FailIteration = MSched.Iteration;      

    snprintf(tmpLine,MMAX_LINE,"cannot start job - RM failure, rc: %d, msg: '%s'",
      tmpSC,
      tmpMsg);

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be started: RM failure (%s)\n",
      J->Name,
      tmpMsg);

    if (Msg != NULL)
      strcpy(Msg,tmpLine);

    MUStrDup(&J->Message,tmpLine);

    MSUFree(R->P[0].S);
 
    return(FAILURE);
    }

  /* job successfully started */

  MXMLDestroyE(&E);      

  /* NOTE: S3 does not provide accurate job start info in many cases */

  J->StartTime    = MSched.Time;
  J->DispatchTime = MSched.Time;

  MSysEMSubmit(
    &MSched.EM,
    (char *)MS3CName[mpstSC],
    (char *)"jobstart",
    (char *)J->Name);

  MDB(1,fS3) MLog("INFO:     job '%s' successfully started\n",
    J->Name);

  return(SUCCESS);
  }  /* END MS3JobStart() */




int MS3JobCancel(

  mjob_t               *J,        /* I */
  mrm_t                *R,        /* I */
  char                 *Message,  /* I */
  char                 *EMsg,     /* O (optional,minsize=MMAX_LINE) */
  enum MStatusCodeEnum *SC)       /* O (optional) */

  {
  mxml_t *E  = NULL;
  mxml_t *RE = NULL;
  mxml_t *CE = NULL;
  mxml_t *DE = NULL;
  mxml_t *JE = NULL;

  char      *ptr;

  char      *Response = NULL; 

  char       CmdString[MMAX_BUFFER];    

  char       tmpMsg[MMAX_LINE];
  char       tmpLine[MMAX_LINE];

  enum MSFC  tmpSC;

#ifndef __MPROD
  const char *FName = "MS3JobCancel";

  MDB(1,fS3) MLog("%s(%s,%s,%s,EMsg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL",
    (Message != NULL) ? Message : "NULL");
#endif /* !__MPROD */

  if (EMsg != NULL)
    EMsg[0] = '\0';

  if (SC != NULL)
    *SC = mscNoError;
 
  if ((J == NULL) || (R == NULL))
    {
    if (EMsg != NULL)
      strcpy(EMsg,"internal error - bad parameters");

    return(FAILURE);
    }

  if (MISSET(J->Flags,mjfNoRMStart))
    {
    MJobSetState(J,mjsRemoved);

    return(SUCCESS);
    }

  switch (R->Version)
    {
    case msssV3_0:
    case msssV4_0:
    default:

      MXMLCreateE(&E,MSON[msonRequest]);
      MXMLSetAttr(E,MSAN[msanAction],(void *)MS3JAName[R->Version][mrmJobCancel],mdfString);

      MS3SetObject(E,MS3ObjName[R->Version][mxoJob],NULL);

      DE = NULL;
      MXMLCreateE(&DE,MSON[msonData]);
      MXMLAddE(E,DE);

      JE = NULL;
      MXMLCreateE(&JE,MS3ObjName[R->Version][mxoJob]);
      MXMLAddE(DE,JE);

      CE = NULL;
      MXMLCreateE(&CE,MS3JobAttr[R->Version][mjaJobID]);
      MXMLSetVal(CE,(void *)J->Name,mdfString);
      MXMLAddE(JE,CE);

      break;
    }  /* END switch (R->Version) */
 
  MXMLToString(E,CmdString,sizeof(CmdString),NULL,TRUE);
 
  MXMLDestroyE(&E);
 
  if (MS3DoCommand(
        &R->P[0],
        CmdString,
        &Response,
        NULL,
        NULL,
        EMsg) == FAILURE)
    {
    /* job could not be cancelled */

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be cancelled (command failed)\n",
      J->Name);
 
    R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  if (((ptr = strstr(Response,"<Body")) == NULL) ||
       (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE))
    {
    /* cannot parse response */

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be cancelled (invalid response)\n",
      J->Name);

    MUFree(&Response);

    if (R->FailIteration == -1)
      R->FailIteration = MSched.Iteration;

    if (EMsg != NULL)
      strcpy(EMsg,"cannot parse response");

    return(FAILURE);
    }

  MUFree(&Response);
 
  if (MXMLGetChildCI(E,MSON[msonResponse],NULL,&RE) == FAILURE)
    {
    /* cannot parse response */

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be cancelled (cannot parse response)\n",
      J->Name);

    MXMLDestroyE(&E);

    if (R->FailIteration == -1)
      R->FailIteration = MSched.Iteration;

    /* why no return(FAILURE) here??? */
    }

  /* check status code */

  if (MS3CheckStatus(RE,&tmpSC,tmpMsg) == FAILURE)
    {
    MXMLDestroyE(&E);

    if (R->FailIteration == -1)
      R->FailIteration = MSched.Iteration;

    snprintf(tmpLine,MMAX_LINE,"cannot cancel job - RM failure, rc: %d, msg: '%s'",
      tmpSC,
      tmpMsg);

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be cancelled: RM failure (%s)\n",
      J->Name,
      tmpMsg);

    if (EMsg != NULL)
      strcpy(EMsg,tmpLine);

    MUStrDup(&J->Message,tmpLine);

    MSUFree(R->P[0].S);

    return(FAILURE);
    }

  /* job successfully cancelled */

  MSysEMSubmit(
    &MSched.EM,
    (char *)MS3CName[mpstSC],
    (char *)"jobcancel",
    (char *)J->Name);

  MDB(1,fS3) MLog("INFO:     job '%s' successfully cancelled\n",
    J->Name);

  return(SUCCESS);
  }  /* END MS3JobCancel() */




int MS3NodeLoad(

  mnode_t   *N,      /* I (update) */
  void      *NP,     /* I */
  int        Status, /* I */
  mrm_t     *R)      /* I */

  {
  int           nindex;
  int           cindex;

  mulong        tmpTime;

  mxml_t      *E;
  mxml_t      *C;

  const char *FName = "MS3NodeLoad";

#ifndef __MPROD
  MDB(2,fS3) MLog("%s(%s,%s,%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    (NP != NULL) ? "NP" : "NULL",
    MNodeState[Status],
    (R != NULL) ? R->Name : "NULL");
#endif /* !__MPROD */

  if ((N == NULL) || (NP == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  MTRAPNODE(N,FName);

  MUGetTime(&tmpTime,mtmNONE,NULL);

  N->CTime = tmpTime;
  N->MTime = tmpTime;
  N->ATime = tmpTime;

  N->RM    = R;

  N->TaskCount = 0;

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

    N->ARes.Swap = MAX(MDEF_MINSWAP,N->ARes.Mem);
    N->CRes.Swap = MAX(MDEF_MINSWAP,N->CRes.Mem);
    }

  N->ActiveOS = MUMAGetIndex(meOpsys,"DEFAULT",mAdd);

  N->Network = MUMAGetBM(meNetwork,"DEFAULT",mAdd);

  N->TaskCount = 0;

  MLocalNodeInit(N);

  E = (mxml_t *)NP;
 
  for (cindex = 0;cindex < E->CCount;cindex++)
    {
    C = E->C[cindex];

    __MS3NodeSetAttr(N,R,C->Name,C->Val,(void *)C);
    }  /* END for (cindex) */

  for (cindex = 0;cindex < MMAX_CLASS;cindex++)
    {
    mclass_t *C;
    mnalloc_t *NL;

    C = &MClass[cindex];

    if ((C == NULL) || (C->Name[0] == '\0'))
      break;

    if (!strcmp(C->Name,"DEFAULT") || !strcmp(C->Name,"ALL"))
      continue;

    if (C->NodeList != NULL)
      {
      NL = (mnalloc_t *)C->NodeList;

      for (nindex = 0;nindex < MMAX_NODE;nindex++)
        {
        if (NL[nindex].N == NULL)
          break;
  
        if (NL[nindex].N == N)
          break; 
        }  /* END for (nindex) */      

      if ((nindex >= MMAX_NODE) || (NL[nindex].N == NULL))
        continue;
      }  /* END if (C->NodeList != NULL) */

    MNodeSetClass(N,C,NULL,mAdd);
    }  /* END for (cindex) */

  MDB(6,fS3) 
    {
    char tmpLine[MMAX_LINE];

#ifdef __M32COMPAT
    MUNumListToString(N->ARes.PSlot,N->CRes.PSlot,NULL,tmpLine,sizeof(tmpLine));
#else /* __M32COMPAT */
    MUNumListToString(N->ARes.PSlot,N->CRes.PSlot,NULL,tmpLine,sizeof(tmpLine),0);
#endif /* __M32COMPAT */

    MLog("INFO:     MNode[%03d] '%18s' %9s VM: %8d Mem: %5d Dk: %5d Cl: %6s %s\n",
      N->Index,
      N->Name,
      MAList[meNodeState][N->State],
      N->CRes.Swap,
      N->CRes.Mem,
      N->CRes.Disk,
      tmpLine,
      MUMAToString(meNFeature,',',N->FBM,sizeof(N->FBM)));

    MLog("INFO:     MNode[%03d] '%18s' C/A/D procs:  %d/%d/%d\n",
      N->Index,
      N->Name,
      N->CRes.Procs,
      N->ARes.Procs,
      N->DRes.Procs);
    }

  return(SUCCESS);
  }  /* END MS3NodeLoad() */






int MS3NodeUpdate(

  mnode_t   *N,      /* I (modified) */
  void      *NP,     /* I */
  int        Status, /* I */
  mrm_t     *R)      /* I */

  {
  int           cindex;

  mulong        tmpTime;

  mxml_t      *E;
  mxml_t      *C;

#ifndef __MPROD
  const char *FName = "MS3NodeUpdate";

  MDB(2,fS3) MLog("%s(%s,%s,%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    (NP != NULL) ? "NP" : "NULL",
    MNodeState[Status],
    (R != NULL) ? R->Name : "NULL");
#endif /* !__MPROD */
 
  if ((N == NULL) || (NP == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  MUGetTime(&tmpTime,mtmNONE,NULL);

  N->MTime   = tmpTime;
  N->ATime   = tmpTime;

  N->RM      = R;

  N->TaskCount = 0;

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

    N->ARes.Swap = MAX(MDEF_MINSWAP,N->ARes.Mem);
    N->CRes.Swap = MAX(MDEF_MINSWAP,N->CRes.Mem);
    }

  /* S3 does not provide pool, opsys, machine speed, or network info */

  N->ActiveOS = MUMAGetIndex(meOpsys,"DEFAULT",mAdd);

  N->Network = MUMAGetBM(meNetwork,"DEFAULT",mAdd);

  N->Load    = 0.0;

  /* get joblist, maxtask, and feature info from S3 server */

  N->TaskCount = 0;

  E = (mxml_t *)NP;

  for (cindex = 0;cindex < E->CCount;cindex++)
    {
    C = E->C[cindex];

    __MS3NodeSetAttr(N,R,C->Name,C->Val,(void *)C);
    }  /* END for (cindex) */
 
  MDB(6,fS3) 
    {
    char tmpLine[MMAX_LINE];

#ifdef __M32COMPAT
    MUNumListToString(N->ARes.PSlot,N->CRes.PSlot,NULL,tmpLine,sizeof(tmpLine));
#else /* __M32COMPAT */
    MUNumListToString(N->ARes.PSlot,N->CRes.PSlot,NULL,tmpLine,sizeof(tmpLine),0);
#endif /* __M32COMPAT */

    MLog("INFO:     MNode[%03d] '%18s' %9s VM: %8d Mem: %5d Dk: %5d Cl: %6s %s\n",
      N->Index,
      N->Name,
      MAList[meNodeState][N->State],
      N->CRes.Swap,
      N->CRes.Mem,
      N->CRes.Disk,
      tmpLine,
      MUMAToString(meNFeature,',',N->FBM,sizeof(N->FBM)));

    MLog("INFO:     MNode[%03d] '%18s' C/A/D procs:  %d/%d/%d\n",
      N->Index,
      N->Name,
      N->CRes.Procs,
      N->ARes.Procs,
      N->DRes.Procs);
    }

  return(SUCCESS);
  }  /* END MS3NodeUpdate() */




int MS3JobLoad(

  char    *JobName,  /* I/O */
  void    *SJP,      /* I (XML) */
  mjob_t **JP,       /* I (modified/freed on failure) */
  short   *TaskList, /* O */
  mrm_t   *R)        /* I */

  {
  int           rqindex;
  mreq_t       *RQ;

  mqos_t       *QDef;

  int           cindex;

  int           CTok;

  long          WallTime;
  long          WCLimit;

  int           NodeCount;
  int           TaskCount;

  mxml_t       *E;
  mxml_t       *CE;

  mbool_t       MultiReqDetected;

  mjob_t       *J;

  char          tmpLine[MMAX_LINE];

  const char *FName = "MS3JobLoad";

#ifndef __MPROD
  MDB(2,fS3) MLog("%s(%s,SJP,%s,TaskList,%s)\n",
    FName,
    (JobName != NULL) ? JobName : "NULL",
    ((JP != NULL) && (*JP != NULL)) ? (*JP)->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");
#endif /* !__MPROD */
 
  if ((SJP == NULL) || (JP == NULL) || (*JP == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  if ((JobName == NULL) && !MISSET(R->Flags,mrmfLocalQueue))
    {
    return(FAILURE);
    }

  /* NOTE:  only assign job id if job successfully processed */

  J = *JP;
 
  MTRAPJOB(J,FName);

  WallTime = 0;
  WCLimit  = 0;

  TaskCount = 1;
  NodeCount = 1;

  /* add resource requirements information */

  if (MReqCreate(J,NULL,&RQ,FALSE) == FAILURE)
    {
    MDB(1,fS3) MLog("ALERT:    cannot add requirements to job '%s'\n",
      J->Name);

    MJOBREMOVE(JP);

    return(FAILURE);
    }

  MRMReqPreLoad(RQ);

  TaskList[0]    = -1;

  J->Request.TC  = 0;

  J->Request.NC  = 0;
  RQ->NodeCount  = 0;

  RQ->DRes.Procs = 1;
  RQ->DRes.Mem   = 0;

  RQ = J->Req[0];  /* FIXME:  only allow one req per job in S3 */

  E = (mxml_t *)SJP;

  /* FORMAT:  

  <Job><JobId>3393</JobId><JobState>idle</JobState><UserId>brett</UserId>
    <Delivered>
      <WallDuration>0</WallDuration>
      <Processors></Processors>
      <NodeCount></NodeCount>
      <SuspendDuration>0</SuspendDuration>
    </Delivered>
    <Requested>
      <WallDuration>300</WallDuration>
      <Processors></Processors>
      <NodeCount></NodeCount>
    </Requested>
    <TaskGroup>
      <Requested>
        <Processors>16</Processors>
        <Memory>1024</Memory>
      </Requested>
      <NodeProperties>
        <Feature>green</Feature>
        <Feature>foo</Feature>
      </NodeProperties>
    </TaskGroup>
    <TaskGroup>
      <Requested>
        <Processors>16</Processors>
      </Requested>
      <NodeProperties>
        <Feature>blue</Feature>
        <Feature>foo</Feature>
      </NodeProperties>
    </TaskGroup>
    <Executable>./mpitest.sh</Executable>
    <InitialWorkingDirectory>/clusters/scl/brett</InitialWorkingDirectory>
    <ProjectId></ProjectId>
    <NodeList></NodeList> 
  </Job>
  */

  /* example 2:

  <Job><JobId>4</JobId><UserId>criguest</UserId><GroupId></GroupId><ProjectId></ProjectId><JobState>idle</JobState>
    <Requested><WallDuration>60</WallDuration></Requested>
    <Delivered><WallDuration>0</WallDuration></Delivered>
    <NodeList></NodeList><StartTime></StartTime>
    <TaskGroup><NodeCount>1</NodeCount><Processors>2</Processors></TaskGroup>
    <TaskGroup><NodeCount>2</NodeCount><Processors>8</Processors></TaskGroup>
    </Job>
  */

  /* NOTE:  if no taskgroups are defined, job level 'requested' attributes are 
            assigned to req 0.  if one or more taskgroups are defined, job level
            'requested' attributes are assigned to all reqs */

  /* make 3 passes */

  MultiReqDetected = FALSE;

  CTok = -1;

  /* pass 1 - identify all task groups */

  while (MXMLGetChild(E,"TaskGroup",&CTok,&CE) == SUCCESS)
    {
    if (MultiReqDetected == FALSE)
      {
      /* initial req already created */

      MultiReqDetected = TRUE;
      }
    else
      {
      /* add req for taskgroup */

      if (MReqCreate(J,NULL,&RQ,FALSE) == FAILURE)
        {
        MDB(1,fS3) MLog("ALERT:    cannot add requirements to job '%s'\n",
          J->Name);

        MJOBREMOVE(JP);

        return(FAILURE);
        }
      }
    }

  /* pass 2 - process all job level attributes and assign to all reqs */

  for (cindex = 0;cindex < E->CCount;cindex++)
    {
    CE = E->C[cindex];

    /* check units */

    if (MXMLGetAttr(CE,"units",NULL,tmpLine,0) == SUCCESS)
      {
      /* NYI */
      }

    if (!strcasecmp(CE->Name,"TaskGroup"))
      {
      /* do not process taskgroup elements in pass 2 */

      continue;
      }

    __MS3JobSetAttr(
      J,
      NULL,
      R->Version,
      CE->Name,
      CE->Val,
      MBNOTSET,
      MBNOTSET,
      CE);
    }  /* END for (cindex) */

  /* pass 3 - process all taskgroup level attributes and assign to specific reqs */

  rqindex = 0;

  for (cindex = 0;cindex < E->CCount;cindex++)
    {
    CE = E->C[cindex];

    if (strcasecmp(CE->Name,"TaskGroup"))
      { 
      /* process only taskgroup elements */

      continue;
      }

    __MS3JobSetAttr(
      J,
      J->Req[rqindex],
      R->Version,
      CE->Name,
      CE->Val,
      MBNOTSET,
      MBNOTSET,
      CE);

    rqindex++;
    }  /* END for (cindex) */

  if (J->Request.TC == 0)
    {
    if (J->Request.NC == 0)
      {
      MDB(2,fS3) MLog("ALERT:    no job task info located for job '%s' (assigning taskcount to 1)\n",
        J->Name);
      }

    if (J->ReqHList != NULL)
      {
      int nc;
      int tc;
     
      tc = 0;
      
      for(nc=0;J->ReqHList[nc].N != NULL;nc++)
        {
        tc += J->ReqHList[nc].TC;
        }

      J->Request.TC = MAX(1,tc);
      RQ->TaskCount = MAX(1,tc);
      RQ->NodeCount = MAX(1,nc);
      J->Request.NC = MAX(1,nc);
                                                                                                                                                       
      J->Req[0]->TaskRequestList[0] = MAX(1,tc);
      J->Req[0]->TaskRequestList[1] = MAX(1,tc);
      }
    else
      {
      J->Request.TC = MAX(1,J->Request.NC);
      RQ->TaskCount = MAX(1,J->Request.NC);
      RQ->NodeCount = MAX(1,J->Request.NC);
      J->Request.NC = MAX(1,J->Request.NC);
                                                                                                                                                       
      J->Req[0]->TaskRequestList[0] = MAX(1,J->Request.NC);
      J->Req[0]->TaskRequestList[1] = MAX(1,J->Request.NC);
      }  /* END if (J->ReqHList != NULL) */
    }    /* END if (J->Request.TC == 0) */

  /* authenticate job */

  if (J->Cred.U == NULL)
    {
    MDB(1,fS3) MLog("ERROR:    cannot authenticate job '%s' (User: %s  Group: %s)\n",
      J->Name,
      (J->Cred.U != NULL) ? J->Cred.U->Name : "NULL",
      (J->Cred.G != NULL) ? J->Cred.G->Name : "NULL");

    MJOBREMOVE(JP);

    return(FAILURE);
    }

  if (MJobSetCreds(
        J,
        J->Cred.U->Name,
        (J->Cred.G != NULL) ? J->Cred.G->Name : (char *)DEFAULT,
        (J->Cred.A != NULL) ? J->Cred.A->Name : NULL) == FAILURE)
    {
    MDB(1,fS3) MLog("ERROR:    cannot authenticate job '%s' (U: %s  G: %s  A: '%s')\n",
      J->Name,
      J->Cred.U->Name,
      (J->Cred.G != NULL) ? J->Cred.G->Name : NONE,
      (J->Cred.A != NULL) ? J->Cred.A->Name : NONE);

    MJOBREMOVE(JP);

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

  /* set defaults for info not specified by S3 */

  J->SubmitTime = MIN(J->SubmitTime,MSched.Time);

  RQ = J->Req[0];

  RQ->RMIndex    = R->Index;

  RQ->TaskRequestList[0] = J->Request.TC;
  RQ->TaskRequestList[1] = J->Request.TC;

  RQ->TaskRequestList[2] = 0;

  /* adjust resource requirement info */
  /* NOTE:  S3 specifies resource requirement info on a 'per node' basis */
 
  J->ExecSize  = 0;
  J->ImageSize = 0;

  RQ->NAccessPolicy = MSched.DefaultNAccessPolicy;

  if ((J->State == mjsStarting) ||
      (J->State == mjsRunning) ||
      (J->State == mjsCompleted) ||
      (J->State == mjsRemoved))
    {
    /* obtain 'per task' statistics */

    /* NO-OP */
    }

  if (JobName == NULL)
    {
    /* set job name */

    /* need rm counter */

    /* NYI */
    }

  MLocalJobInit(J);

  return(SUCCESS);
  }  /* END MS3JobLoad() */




int MS3JobUpdate(

  void      *SJP,      /* I */
  mjob_t   **JP,       /* I (modified/freed on failure) */
  short     *TaskList, /* I */
  mrm_t     *R)        /* I */

  {
  enum MJobStateEnum OldState;

  int           TaskCount;
  int           NodeCount;

  mreq_t        *RQ;

  int           cindex;

  mxml_t       *E;
  mxml_t       *CE;

  char          tmpLine[MMAX_LINE];

  mjob_t       *J;

#ifndef __MPROD
  const char *FName = "MS3JobUpdate";

  MDB(2,fS3) MLog("%s(SJP,%s,TaskList,%s)\n",
    FName,
    ((JP != NULL) && (*JP != NULL)) ? (*JP)->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");
#endif /* !__MPROD */

  if ((JP == NULL) || (*JP == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  J = *JP;

  OldState = J->State;

  TaskList[0] = -1;

  /* get job state: NYI */

  RQ = J->Req[0];

  TaskCount = 1;
  NodeCount = 1;

  if (J->State != OldState)
    {
    MDB(1,fS3) MLog("INFO:     job '%s' changed states from %s to %s\n",
      J->Name,
      MJobState[OldState],
      MJobState[J->State]);

    J->MTime = MSched.Time;
    }

  if (!MISSET(J->Flags,mjfNoRMStart))
    {
    int rqindex;

    E = (mxml_t *)SJP;

    /* make two of three passes (pass one only performed on initial job load */

    /* pass 2 - process all job level attributes and assign to all reqs */

    for (cindex = 0;cindex < E->CCount;cindex++)
      {
      CE = E->C[cindex];

      /* check units */

      if (MXMLGetAttr(CE,"units",NULL,tmpLine,0) == SUCCESS)
        {
        /* NYI */
        }

      if (!strcasecmp(CE->Name,"TaskGroup"))
        {
        /* do not process taskgroup elements in pass 2 */
 
        continue;
        }

      __MS3JobSetAttr(
        J,
        NULL,
        R->Version,
        CE->Name,
        CE->Val,
        MBNOTSET,
        MBNOTSET,
        CE);
      }  /* END for (cindex) */

    /* pass 3 - process all taskgroup level attributes and assign to specific reqs */

    rqindex = 0;

    for (cindex = 0;cindex < E->CCount;cindex++)
      {
      CE = E->C[cindex];

      if (strcasecmp(CE->Name,"TaskGroup"))
        {
        /* process only taskgroup elements */

        continue;
        }

      __MS3JobSetAttr(
        J,
        J->Req[rqindex],
        R->Version,
        CE->Name,
        CE->Val,
        MBNOTSET,
        MBNOTSET,
        CE);

      rqindex++;
      }  /* END for (cindex) */
    }    /* END if (!MISSET(J->Flags,mjfNoRMStart)) */

  if (J->State != OldState)
    {
    MDB(1,fS3) MLog("INFO:     job '%s' changed states from %s to %s\n",
      J->Name,
      MJobState[OldState],
      MJobState[J->State]);

    J->MTime = MSched.Time;
    }

  if ((J->State == mjsCompleted) ||
      (J->State == mjsRemoved) ||
      (J->State == mjsVacated))
    {
    /* NOTE:  no completion time attribute currently supported */

    J->CompletionTime = J->MTime;

    if (J->AWallTime > 0)
      J->CompletionTime = MIN(J->CompletionTime,J->StartTime + J->AWallTime);
    }

  return(SUCCESS);
  }  /* END MS3JobUpdate() */




int __MS3NodeSetAttr(

  mnode_t *N,     /* I (modified) */
  mrm_t   *R,     /* I */
  char    *AName, /* I */
  char    *AVal,  /* I */
  void    *AData) /* I (optional) */
 
  {
  int aindex;

  long tmpL;

  char UData[MMAX_NAME];
  char tmpData[MMAX_NAME];

  if ((N == NULL) || (AName == NULL))
    {
    return(FAILURE);
    }

  if ((aindex = MUGetIndex(AName,(const char **)MS3NodeAttr[R->Version],FALSE,0)) != 0)
    {
    if (AVal == NULL)
      {
      return(SUCCESS);
      }

    switch (aindex)
      {
      case mnaRCProc:
      case mnaRAProc:
      case mnaRCSwap:
      case mnaRASwap:
      case mnaRCDisk:
      case mnaRADisk:
      case mnaRCMem:
      case mnaRAMem:

        tmpL = -1;

        if (AData != NULL)
          {
          /* check units */

          if (MXMLGetAttr((mxml_t *)AData,"units",NULL,UData,sizeof(UData)) == SUCCESS)
            {
            sprintf(tmpData,"%s%s",
              AVal,
              UData);

           /* adjust value by unit information */

           tmpL = MURSpecToL(tmpData,mvmMega,mvmMega);
           }
         }

        if (tmpL == -1)
          tmpL = MURSpecToL(AVal,mvmMega,mvmMega);

      case mnaNodeID:
      case mnaLoad:

        if (strtol(AVal,NULL,0) > 0)
          MNodeSetAttr(N,(enum MNodeAttrEnum)aindex,(void **)AVal,mdfString,mSet);

        break;

      case mnaNodeState:

        if (!strcmp(R->APIVersion,MS3VName[msssV3_0]) ||
            !strcmp(R->APIVersion,MS3VName[msssV4_0]) ||
            !strcmp(R->APIVersion,MS3VName[msssV0_2]))
          {
          const char *S3NState[] = { NONE, "up", "down", NULL };
          const int   S3NSEq[]   = { mnsNone, mnsUnknown, mnsDown, -1 };

          int index;
          int SIndex;

          /* translate S3 node state to internal node state */

          index = MUGetIndex(AVal,S3NState,FALSE,0);

          SIndex = S3NSEq[index];

          MNodeSetAttr(N,mnaNodeState,(void **)&SIndex,mdfInt,mSet);
          }
        else
          {
          const char *S3NState[] = { NONE, "active", "busy", "down", "idle", NULL };
          const int   S3NSEq[]   = { mnsNone, mnsActive, mnsBusy, mnsDown, mnsIdle, -1 };

          int index;
          int SIndex;

          /* translate S3 node state to M node state */

          index = MUGetIndex(AVal,S3NState,FALSE,0);

          SIndex = S3NSEq[index];

          MNodeSetAttr(N,mnaNodeState,(void **)&SIndex,mdfInt,mSet);
          }  /* END else () */

        if (N->State == mnsUnknown)
          {
          N->ARes.Procs = -1;
          }

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch (aindex) */
    }    /* END if (MUGetIndex(AName,(const char **)MS3NodeAttr[R->Version],FALSE,0) != 0) */
  else
    {
    int rindex;

    mxml_t *AE;
    mxml_t *CE;

    mbool_t IsConfigured;

    int CTok;

    /* unrecognized attribute */

    AE = (mxml_t *)AData;

    if (strcmp(AName,"Available"))
      {
      IsConfigured = TRUE;
      }
    else
      {
      IsConfigured = FALSE;
      }

    CTok = -1;

    while (MXMLGetChildCI(AE,NULL,&CTok,&CE) == SUCCESS)
      {
      AVal = CE->Val;

      if ((rindex = MUGetIndex(CE->Name,(const char **)MS3NodeRAttr[R->Version],FALSE,0)) == 0)
        {
        return(FAILURE);
        }

      tmpL = -1;

      /* check units */

      if (MXMLGetAttr((mxml_t *)AData,"units",NULL,UData,sizeof(UData)) == SUCCESS)
        {
        sprintf(tmpData,"%s%s",
          AVal,
          UData);

        /* adjust value by unit information */

         tmpL = MURSpecToL(tmpData,mvmMega,mvmMega);
         }

      if (tmpL == -1)
        tmpL = MURSpecToL(AVal,mvmMega,mvmMega);

      switch (rindex)
        {
        case mrDisk:

          aindex = (IsConfigured == TRUE) ? mnaRCDisk : mnaRADisk;
  
          break;

        case mrMem:

          aindex = (IsConfigured == TRUE) ? mnaRCMem : mnaRAMem;

          break;

        case mrProc:

          aindex = (IsConfigured == TRUE) ? mnaRCProc : mnaRAProc;

          break;

        case mrSwap:

          aindex = (IsConfigured == TRUE) ? mnaRCSwap : mnaRASwap;

          break;
  
        default:

          /* NOT HANDLED */

          return(FAILURE);
       
          /*NOTREACHED*/

          break;
        }  /* END switch (rindex) */

      MNodeSetAttr(N,(enum MNodeAttrEnum)aindex,(void **)AVal,mdfString,mSet);
      }  /* END while (MXMLGetChildCI(CE,NULL,&CTok,&CE) == SUCCCESS) */
    }    /* END else ... */

  return(SUCCESS);
  }  /* END __MS3NodeSetAttr() */




int __MS3QueueSetAttr(

  mclass_t *C,     /* I (modified) */
  char     *AName, /* I */
  char     *AVal)  /* I */

  {
  int aindex;

  if ((C == NULL) || (AName == NULL))
    {
    return(FAILURE);
    }

  if ((aindex = MUGetIndex(AName,(const char **)MS3QueueAttr,FALSE,0)) != 0)
    {
    switch (aindex)
      {
      case mclaHostList:

        {
        char *ptr;
        char *TokPtr;

        mnode_t *N;
        mnalloc_t *NL;

        int   nindex;

        if (C->NodeList == NULL)
          {
          if ((C->NodeList = (void *)calloc(
               1,
               sizeof(mnalloc_t) * MMAX_NODE)) == NULL)
            {
            break;
            }
          }

        NL = (mnalloc_t *)C->NodeList;

        /* build class nodelist */

        nindex = 0;

        /* FORMAT:  <HOST>[,<HOST>]... */

        ptr = MUStrTok(AVal,", \t\n",&TokPtr);

        while (ptr != NULL)
          {
          if (MNodeFind(ptr,&N) == FAILURE)
            {
            ptr = MUStrTok(NULL,", \t\n",&TokPtr);

            continue;
            }

          NL[nindex].N  = N;
          NL[nindex].TC = 1;

          ptr = MUStrTok(NULL,", \t\n",&TokPtr);
          }  /* END while (ptr != NULL) */
        }    /* END BLOCK */

        break;

      case mclaState:

        if (!strcmp(AVal,"true"))
          {
          C->State = mclsActive;

          C->IsDisabled = FALSE;
          }
        else
          {
          C->State = mclsStopped;

          C->IsDisabled = TRUE;
          }

        break;

      default:

        /* attr not supported */

        return(FAILURE);

        /*NOTREACHED*/

        break;
      }  /* END switch (aindex) */
    }

  return(SUCCESS);
  }  /* END __MS3QueueSetAttr() */




/* NOTE:  __MS3JobSetAttr() will process multiple taskgroups and create corresponding
          job reqs */

int __MS3JobSetAttr(

  mjob_t  *J,            /* I (modified) */
  mreq_t  *RQ,           /* I (optional, if not set, attr applies globally) */
  int      VIndex,       /* I */
  char    *AName,        /* I */
  char    *AVal,         /* I */
  mbool_t  IsUtilized,   /* I */
  mbool_t  IsConsumable, /* I */
  void    *E)            /* I (optional) */

  {
  int aindex;
  int rqindex;
  
  if ((J == NULL) || (AName == NULL))
    {
    return(FAILURE);
    }

  if ((aindex = MUGetIndex(AName,(const char **)MS3JobAttr[VIndex],FALSE,0)) != 0)
    {
    if (AVal == NULL)
      {
      if (aindex != mjaAllocNodeList)
        {
        return(SUCCESS);
        }
      }

    switch (aindex)
      {
      case mjaAccount:
      case mjaArgs:
      case mjaCalendar:
      case mjaCmdFile:
      case mjaCPULimit:
      case mjaEnv:
      case mjaFlags:
      case mjaGroup:
      case mjaIsInteractive:
      case mjaIsRestartable:
      case mjaIsSuspendable:
      case mjaIWD:
      case mjaJobName:
      case mjaJobID:
      case mjaNotification:
      case mjaQOSReq:
      case mjaQOS:
      case mjaReqSMinTime:
      case mjaRMXString:
      case mjaRsvAccess:
      case mjaStartTime:
      case mjaSubmitLanguage:
      case mjaSubmitString:
      case mjaSubmitTime:
      case mjaCompletionTime:
      case mjaStdErr:
      case mjaStdIn:
      case mjaStdOut:
      case mjaUser:
      case mjaUserPrio:

        if (AVal == NULL)
          {
          return(SUCCESS);
          }

        MJobSetAttr(J,(enum MJobAttrEnum)aindex,(void **)AVal,mdfString,mSet);

        break;

      case mjaDepend:

        /* NYI */

        break;

      case mjaPAL:

        if (IsUtilized != TRUE)
          {
          MJobSetAttr(J,(enum MJobAttrEnum)aindex,(void **)AVal,mdfString,mSet);
          }

        break;

      case mjaReqProcs:

        if (IsUtilized == TRUE)
          {
          /* utilized procs NYI */

          aindex = mjaNONE;
          }

        MJobSetAttr(J,(enum MJobAttrEnum)aindex,(void **)AVal,mdfString,mSet);

        break;

      case mjaReqAWDuration:
      case mjaAWDuration:

        if (IsUtilized == TRUE)
          {
          aindex = mjaAWDuration;
          }
        else
          {
          aindex = mjaReqAWDuration;
          }

        MJobSetAttr(J,(enum MJobAttrEnum)aindex,(void **)AVal,mdfString,mSet);

        break;

      case mjaReqReservation:

        MJobSetAttr(J,(enum MJobAttrEnum)aindex,(void **)AVal,mdfString,mSet);

        break;

      case mjaAllocNodeList:

        {
        mxml_t *NE;

        char *ptr;

        char tmpBuf[MMAX_BUFFER];

        char *BPtr;
        int   BSpace;

        int   CTok;
 
        /* FORMAT:  <Node>X</Node>... */

        ptr = AVal;

        MUSNInit(&BPtr,&BSpace,tmpBuf,sizeof(tmpBuf));

        /* convert XML to tasklist string */

        CTok = -1;

        while (MXMLGetChildCI((mxml_t *)E,"Node",&CTok,&NE) == SUCCESS)
          {
          if (tmpBuf[0] != '\0')
            MUSNCat(&BPtr,&BSpace,",");

          MUSNCat(&BPtr,&BSpace,NE->Val);
          }  /* END while (MXMLFromString() == SUCCESS) */

        if (tmpBuf[0] != '\0')
          {
          if (IsUtilized == TRUE)
            MJobSetAttr(J,mjaAllocNodeList,(void **)tmpBuf,mdfString,mSet);
          else
            MJobSetAttr(J,mjaHostList,(void **)tmpBuf,mdfString,mSet);
          }
        }  /* END BLOCK */

        break;

      case mjaState:

        switch (VIndex)
          {
          case msssV3_0:
          case msssV4_0:

            J->State = (enum MJobStateEnum)MUGetIndex((char *)AVal,MS3JobState,FALSE,0);

            break;

          case msssV0_2:
          default:

            if ((J->State = (enum MJobStateEnum)MUGetIndex((char *)AVal,MJobState,FALSE,0)) == 0)
              {
              J->State = (enum MJobStateEnum)MUGetIndex((char *)AVal,MS3JobState,FALSE,0);
              }

            break;
          }        /* END switch (VIndex) */

        break;

      default:

        /* attr not supported */

        return(FAILURE);

        /*NOTREACHED*/

        break;
      }  /* END switch (aindex) */
    }
  else if ((aindex = MUGetIndex(AName,(const char **)MS3ReqAttr[VIndex],FALSE,0)) != 0)
    {
    switch (aindex)
      {
      case mrqaNCReqMin:
      case mrqaTCReqMin:

        if (IsUtilized == TRUE)
          {
          /* utilized procs/nodes - NYI */

          aindex = mrqaNONE;

          break;
          }

#ifndef __M32COMPAT
        MSET(J->IFlags,mjifTasksSpecified);
#endif /* __M32COMPAT */

        /* no break - pass through */

      case mrqaReqArch:
      case mrqaReqOpsys:
      case mrqaReqClass:
      case mrqaReqDiskPerTask:
      case mrqaReqMemPerTask:
      case mrqaReqNodeMem:
      case mrqaReqNodeSwap:
      case mrqaReqNodeDisk:
      case mrqaReqNodeProc:
      case mrqaTPN:
      case mrqaReqNodeFeature:

        /* NOTE:  for mrqaReqNodeFeature, support either one feature per 
                  element or multiple features per element */

        for (rqindex = 0;rqindex < MMAX_REQ_PER_JOB;rqindex++)
          {
          if (J->Req[rqindex] == NULL)
            break;

          if ((RQ != NULL) && (RQ != J->Req[rqindex]))
            continue;

          MReqSetAttr(
            J,
            J->Req[rqindex],
            (enum MReqAttrEnum)aindex,
            (void **)AVal,
            mdfString,
            mSet);
          }  /* END for (rqindex) */
           
        break;

      default:

        /* attr not supported */

        return(FAILURE);

        /*NOTREACHED*/

        break;
      }  /* END switch (aindex) */
    }    /* END else if (MUGetIndex() != 0) */
  else
    {
    /* check if contained w/in Requested/Utilized */

    mxml_t *AE;
    mxml_t *CE;

    mbool_t IsUtilized = FALSE;

    mbool_t IsHandled = FALSE;

    int CTok;

    AE = (mxml_t *)E;

    if (!strcmp(AName,"Requested"))
      {
      IsUtilized = FALSE;
      }
    else if (!strcmp(AName,"Delivered"))
      {
      IsUtilized = TRUE;
      }
    else if (!strcmp(AName,"NodeProperties"))
      {
      IsConsumable = FALSE;
      }
    else if (!strcmp(AName,"TaskGroup"))
      {
      /* NO-OP */
      }
    else
      {
      /* unrecognized attribute */

      return(FAILURE);
      }

    CTok = -1;

    while (MXMLGetChildCI(AE,NULL,&CTok,&CE) == SUCCESS)
      {
      char UData[MMAX_NAME];
      char *AVal;

      AVal = CE->Val;

      /* check units */

      if (MXMLGetAttr(CE,"units",NULL,UData,sizeof(UData)) == SUCCESS)
        {
        char tmpData[MMAX_LINE];

        sprintf(tmpData,"%s%s",
          AVal,
          UData);

        /* adjust value by unit information */

        /* NYI */
        }

      __MS3JobSetAttr(
        J,
        RQ,
        (J->SRM != NULL) ? J->SRM->Version : 0,
        CE->Name,
        CE->Val,
        IsUtilized,
        IsConsumable,
        (void *)CE);

      IsHandled = TRUE;
      }  /* END while (MXMLGetChildCI(AE,NULL,&CTok,&CE) == SUCCESS) */

    /* attribute not handled */

    if (IsHandled == FALSE)
      {
      return(FAILURE);
      }
    }  /* END else ((aindex = MUGetIndex(AName)) != 0) */

  return(SUCCESS);
  }  /* END __MS3JobSetAttr() */





int MS3DoCommand(

  mpsi_t  *P,         /* I */
  char    *CmdString, /* I (optional) */
  char   **OBuf,      /* O copy of S->RBuffer or err msg (alloc,optional) */
  mxml_t **ODE,       /* O (optional) */
  int     *SC,        /* O (optional) */
  char    *EMsg)      /* O (optional,minsize=MMAX_LINE) */

  {
  msocket_t *S;

  msocket_t  tmpS;

  char      *ServerHost = NULL;
  int        ServerPort = -1;

  enum MSocketProtocolEnum SocketProtocol = mspNONE;
  enum MWireProtocolEnum   WireProtocol   = mwpNONE;
  enum MChecksumAlgoEnum   CSAlgo         = mcsaNONE;

  enum MStatusCodeEnum     tSC;

  long       Timeout;

  char      *CSKey = NULL;

  mxml_t    *SDE;

  const char *FName = "MS3DoCommand";

#ifndef __MPROD
  MDB(2,fS3) MLog("%s(%s,%s,OBuf,ODE,SC,EMsg)\n",
    FName,
    (P != NULL) ? MS3CName[P->Type] : "NULL",
    (CmdString != NULL) ? CmdString : "NULL");
#endif /* !__MPROD */

  if (EMsg != NULL)
    EMsg[0] = '\0';

  if (SC != NULL)
    *SC = 0;

  if (ODE != NULL)
    *ODE = NULL;

  if ((P == NULL) || 
      (P->Type <= mpstNONE))
    {
    MDB(2,fS3) MLog("ALERT:    invalid peer type in %s\n",
      FName);

    return(FAILURE);
    }

  if ((OBuf != NULL) && (*OBuf != NULL))
    MUFree(OBuf);

  Timeout = (P->Timeout > 0) ? P->Timeout : 1;
  
  switch (P->Type)
    {
    case mpstWWW:

      memset(&tmpS,0,sizeof(tmpS));

      S          = &tmpS;

      ServerHost = P->HostName;

      ServerPort = (P->Port > 0) ? P->Port : 80;

      SocketProtocol = mspHTTPClient;

      CSKey      = NULL;

      break;

    case mpstQM:
    case mpstNM:
    case mpstSD:
    case mpstEM:
    case mpstAM:

      S          = P->S;

      ServerHost = P->HostName;
      ServerPort = P->Port;

      SocketProtocol = P->SocketProtocol;
      WireProtocol   = P->WireProtocol;

      CSKey      = P->CSKey;
      CSAlgo     = P->CSAlgo;

      break;

    default:

      /* invalid service requested */

      if (EMsg != NULL)
        strcpy(EMsg,"invaild service requested");

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch (P->Type) */

  /* free old data objects */

  if ((ServerHost == NULL) || (ServerHost[0] == '\0'))
    {
    /* remote access point not configured */

    MDB(2,fS3) MLog("ALERT:    invalid server host in %s\n",
      FName);

    return(FAILURE);
    }

  if ((P->Type != mpstEM) && (P->Type != mpstWWW))
    {
    char tmpLine[MMAX_LINE];

    sprintf(tmpLine,"%s,%s;",
      MS3CName[mpstSC],
      MS3CName[P->Type]);

    MSysEMSubmit(
      &MSched.EM,
      (char *)MS3CName[mpstSC],
      "comcom",
      tmpLine); 
    }

  if (S != NULL)
    {
    MSUFree(S);
    }

  if (S->sd <= 0)
    {
    /* obtain location/config info from discovery service */

    /* NYI */

    SDE = (mxml_t *)S->SDE;
 
    MSUInitialize(
      S,
      ServerHost,
      ServerPort,
      0,
      (1 << msftTCP));

    S->SDE = SDE;
 
    S->SocketProtocol = SocketProtocol;
    S->WireProtocol   = WireProtocol;

    if (CSKey != NULL)
      strcpy(S->CSKey,CSKey);

    if (CSAlgo != mcsaNONE)
      S->CSAlgo = CSAlgo;

    if (MSUConnect(S,TRUE,NULL) == FAILURE)
      {
      char tmpMsg[MMAX_LINE];

      MDB(0,fS3) MLog("ERROR:    cannot connect to %s server '%s':%d\n",
        MS3CName[P->Type],
        ServerHost,
        ServerPort);
 
      snprintf(tmpMsg,sizeof(tmpMsg),"RMFAILURE:  cannot connect to %s server %s:%d (command: '%s')\n",
        MS3CName[P->Type],
        ServerHost,
        ServerPort,
        (CmdString != NULL) ? CmdString : "<XML>");
 
      MSysRegEvent(tmpMsg,mactNONE,0,1);

      if (EMsg != NULL)
        strcpy(EMsg,tmpMsg);

      switch (P->Type)
        {
        case mpstWWW:

          MSUFree(S);

          break;

        default:

          /* NO-OP */

          break;
        }  /* END switch (P->Type) */

      return(FAILURE);
      }
    }    /* END if (S->sd <= 0) */

  if (P->Data != NULL)
    {
    S->URI = (char *)P->Data;
    }

  if (CmdString != NULL)
    {
    MS3EncapsulateMessage(
      CmdString,
      P->Version,
      TRUE,
      &S->SBuffer,
      &S->SBufSize);
    }

  /* send data */

  if (MSUSendData(S,Timeout * 1000000,FALSE,FALSE) == FAILURE)
    {
    char tmpMsg[MMAX_LINE];

    MSUDisconnect(S);

    MDB(0,fS3) MLog("ERROR:    cannot send data to %s server '%s':%d\n",
      MS3CName[P->Type],
      ServerHost,
      ServerPort);
 
    snprintf(tmpMsg,sizeof(tmpMsg),"FAILURE:  cannot send data to %s server %s:%d (cmd: '%s')\n",
      MS3CName[P->Type],
      ServerHost,
      ServerPort,
      (CmdString != NULL) ? CmdString : "<XML>");
 
    MSysRegEvent(tmpMsg,mactNONE,0,1);

    if (EMsg != NULL)
      strcpy(EMsg,tmpMsg);

    switch (P->Type)
      {
      case mpstWWW:

        MSUFree(S);
 
        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch (P->Type) */

    return(FAILURE);
    }  /* END if (MSUSendData(S,Timeout * 1000000,FALSE,FALSE) == FAILURE) */

  MDB(1,fS3) MLog("INFO:     command sent to server\n");
 
  MDB(3,fS3) MLog("INFO:     message sent: '%s'\n",
    (CmdString != NULL) ? CmdString : "<XML>");

  /* receive data */

  if (MSURecvData(S,Timeout * 1000000,FALSE,&tSC,EMsg) == FAILURE)
    {
    char tmpMsg[MMAX_LINE];

    MSUDisconnect(S);

    if ((OBuf != NULL) && (S->SMsg != NULL))
      {
      MUStrDup(OBuf,S->SMsg);
      }
 
    MDB(0,fS3) MLog("ERROR:    cannot receive response from %s server '%s':%d\n",
      MS3CName[P->Type],
      ServerHost,
      ServerPort);

    snprintf(tmpMsg,sizeof(tmpMsg),"FAILURE:  cannot receive response from %s server %s:%d (cmd: '%s')\n",
      MS3CName[P->Type],
      ServerHost,
      ServerPort,
      (CmdString != NULL) ? CmdString : "<XML>");
 
    MSysRegEvent(tmpMsg,mactNONE,0,1);

    if (EMsg != NULL)
      {
      if (EMsg[0] == '\0')
        strcpy(EMsg,tmpMsg);
      }

    if (SC != NULL)
      *SC = (int)tSC;

    switch (P->Type)
      {
      case mpstWWW:

        MSUFree(S);

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch (P->Type) */

    return(FAILURE);
    }  /* END if (MSURecvData(S,Timeout * 1000000,NULL,FALSE) == FAILURE) */

  if ((P->Type != mpstEM) && (P->Type != mpstWWW))
    {
    char tmpLine[MMAX_LINE];

    sprintf(tmpLine,"%s,%s;",
      MS3CName[P->Type],
      MS3CName[mpstSC]);

    MSysEMSubmit(
      &MSched.EM,
      (char *)MS3CName[mpstSC],
      "comcom",
      tmpLine);
    }

  MDB(1,fS3) MLog("INFO:     response received from server\n");
 
  MDB(3,fS3) MLog("INFO:     response received: '%s'\n",
    S->RBuffer);

  if (OBuf != NULL)  
    *OBuf = strdup(S->RBuffer);

  if (ODE != NULL)
    {
    *ODE = (mxml_t *)S->RDE;

    S->RDE = NULL;
    }

  if ((S->SocketProtocol == mspHalfSocket) ||
      (S->SocketProtocol == mspS3Challenge) ||
      (S->SocketProtocol == mspHTTP) ||
      (S->SocketProtocol == mspHTTPClient))
    {
    /* one time use socket */

    /* NOTE:  disconnecting socket frees xml/string buffer data */

    MSUDisconnect(S);
    }

  switch (P->Type)
    {
    case mpstWWW:

      MSUFree(S);

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch (P->Type) */

  return(SUCCESS);
  }  /* END MS3DoCommand() */




long MS3GetResKVal(

  char *String)  /* I */

  {
  long  val; 
  char *tail;

  char  Mod;

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

  Mod = toupper(*tail);

  if (Mod == 'K')
    {
    return(val);
    }
  else if (Mod == 'M')
    {
    return(val << 10);
    }
  else if (Mod == 'G')
    {
    return(val << 20);
    }
  else if (Mod == 'T')
    {
    return(val << 30);
    }
#ifdef __M64
  else if (Mod == 'P')
    {
    return(val << 40);
    }
  else if (Mod == 'X')
    {
    return(val << 50);
    }
  else if (Mod == 'Z')
    {
    return(val << 60);
    }
#endif /* __M64 */
  else if (Mod == 'b')
    {
    return(val >> 10);
    }

  return(val);
  }  /* END MS3GetResKVal() */




int MS3QueueGetInfo(

  mnode_t *N,
  char     CClass[][MMAX_NAME],
  char     AClass[][MMAX_NAME])

  {
  int cindex;
  int CIndex;

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
    MDB(8,fS3) MLog("INFO:     configured class '%s' specified for node %s\n",
      CClass[cindex],
      N->Name);

    if ((CIndex = MUMAGetIndex(meClass,CClass[cindex],mAdd)) != FAILURE)
      {
      N->CRes.PSlot[CIndex].count++;
      N->CRes.PSlot[0].count++;
      }
    else
      {
      MDB(1,fS3) MLog("ALERT:    cannot add configured class '%s' to node %s\n",
        CClass[cindex],
        N->Name);
      }
    }    /* END for (cindex) */

  /* update available classes */

  for (cindex = 0;AClass[cindex][0] != '\0';cindex++)
    {
    MDB(8,fS3) MLog("INFO:     available class '%s' specified for node %s\n",
      AClass[cindex],
      N->Name);

    if ((CIndex = MUMAGetIndex(meClass,AClass[cindex],mAdd)) != FAILURE)
      {
      if (N->CRes.PSlot[CIndex].count > N->ARes.PSlot[CIndex].count)
        {
        N->ARes.PSlot[CIndex].count++;
        N->ARes.PSlot[0].count++;
        }
      else
        {
        MDB(1,fS3) MLog("ALERT:    class '%s' available but not configured\n",
          AClass[cindex]);
        }
      }
    else
      {
      MDB(1,fS3) MLog("ALERT:    cannot add available class '%s' to node %s\n",
        AClass[cindex],
        N->Name);
      }
    }    /* END for (cindex) */

  ConsumedClasses = 0;

  for (cindex = 1;cindex < MMAX_CLASS;cindex++)
    {
    ConsumedClasses += MAX(
      0,
      N->CRes.PSlot[cindex].count - N->ARes.PSlot[cindex].count);
    }

  if (((N->State == mnsIdle) &&
      (MUNumListGetCount(MMAX_PRIO_VAL,N->CRes.PSlot,N->ARes.PSlot,0,NULL) == FAILURE)) ||
      (MUNumListGetCount(MMAX_PRIO_VAL,N->ARes.PSlot,N->CRes.PSlot,0,NULL) == FAILURE))
    {
    char tmpLine[MMAX_LINE];

    /* S3 corruption */

#ifdef __M32COMPAT
    MUNumListToString(N->ARes.PSlot,N->CRes.PSlot,NULL,tmpLine,sizeof(tmpLine));
#else /* __M32COMPAT */
    MUNumListToString(N->ARes.PSlot,N->CRes.PSlot,NULL,tmpLine,sizeof(tmpLine),0);
#endif /* __M32COMPAT */

    MDB(1,fS3) MLog("ALERT:    %s node %s has class mismatch.  classes: %s\n",
      MAList[meNodeState][N->State],
      N->Name,
      tmpLine);

    if (MSched.Mode == msmNormal)
      {
      MOSSyslog(LOG_NOTICE,"node %s in state %s has class mismatch.  classes: %s\n",
        N->Name,
        MAList[meNodeState][N->State],
        tmpLine);
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
  }  /* END MS3QueueGetInfo() */




int MS3JobRequeue(

  mjob_t  *J,     /* I (modified) */
  mrm_t   *R,     /* I */
  mjob_t **JPeer, /* I (optional) */
  char    *Msg,   /* O (optional,minsize=MMAX_LINE) */
  int     *SC)    /* O */

  {
  char EMsg[MMAX_LINE];

  mxml_t *E  = NULL;
  mxml_t *RE = NULL;
  mxml_t *CE = NULL;
  mxml_t *DE = NULL;
  mxml_t *JE = NULL;

  char *ptr;

  char *Response = NULL;

  char  CmdString[MMAX_BUFFER];

  char tmpJobName[MMAX_NAME];

  enum MSFC tSC;

#ifndef __MPROD
  const char *FName = "MS3JobRequeue";

  MDB(2,fS3) MLog("%s(%s,%s,JPeer,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");
#endif /* !__MPROD */

  if (Msg != NULL)
    Msg[0] = '\0';

  if ((J == NULL) || (R == NULL))
    {
    return(FAILURE);
    }
 
  if (MJOBISACTIVE(J) == FAILURE)
    {
    return(FAILURE);
    }

  MJobGetName(
    J,
    NULL,
    R,
    tmpJobName,
    sizeof(tmpJobName),
    mjnRMName);   

  switch (R->Version)
    {
    case msssV3_0:
    case msssV4_0:

      MXMLCreateE(&E,MSON[msonRequest]);
      MXMLSetAttr(E,MSAN[msanAction],(void *)MS3JAName[R->Version][mrmJobRequeue],mdfString);
      MS3SetObject(E,MS3ObjName[R->Version][mxoJob],NULL);

      DE = NULL;
      MXMLCreateE(&DE,MSON[msonData]);
      MXMLAddE(E,DE);

      JE = NULL;
      MXMLCreateE(&JE,MS3ObjName[R->Version][mxoJob]);
      MXMLAddE(DE,JE);

      CE = NULL;
      MXMLCreateE(&CE,MS3JobAttr[R->Version][mjaJobID]);
      MXMLSetVal(CE,(void *)J->Name,mdfString);
      MXMLAddE(JE,CE);

      break;

    default:

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch (R->Version) */

  MXMLToString(E,CmdString,sizeof(CmdString),NULL,TRUE);

  MXMLDestroyE(&E);

  if (MS3DoCommand(
        &R->P[0],
        CmdString,
        &Response,
        NULL,
        NULL,
        Msg) == FAILURE)
    {
    /* job could not be requeued */

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be requeued (command failed)\n",
      J->Name);

    R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  if (((ptr = strstr(Response,"<Body")) == NULL) ||
       (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE))
    {
    /* cannot parse response */

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be requeued (invalid response)\n",
      J->Name);

    MUFree(&Response);

    if (R->FailIteration == -1)
      R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  MUFree(&Response);

  if (MXMLGetChildCI(E,MSON[msonResponse],NULL,&RE) == FAILURE)
    {
    /* cannot parse response */

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be requeued (cannot parse response)\n",
      J->Name);

    MXMLDestroyE(&E);

    if (R->FailIteration == -1)
      R->FailIteration = MSched.Iteration;
    }

  /* check status code */

  if (MS3CheckStatus(RE,&tSC,EMsg) == FAILURE)
    {
    if (SC != NULL)
      *SC = (int)tSC;

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be requeued (cannot parse command status)\n",
      J->Name);

    MXMLDestroyE(&E);

    if (R->FailIteration == -1)
      R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  if (SC != NULL)
    *SC = (int)tSC;

  /* job successfully requeued */

  MSysEMSubmit(
    &MSched.EM,
    (char *)MS3CName[mpstSC],
    (char *)"jobrequeue",
    (char *)J->Name);

  MDB(1,fS3) MLog("INFO:     job '%s' successfully requeued\n",
    J->Name);

  return(SUCCESS);
  }  /* END MS3JobRequeue() */




char *__MS3GetErrMsg(

  int SD)  /* I */

  {
  static char *ptr;

  ptr = NULL;

  return(ptr);
  }  /* END __MS3GetErrMsg() */




int MS3JobSuspend(

  mjob_t *J,       /* I */
  mrm_t  *R,       /* I */
  char   *EMsg,    /* O (optional) */
  int    *SC)      /* O (optional) */

  {
  mxml_t *E  = NULL;
  mxml_t *RE = NULL;
  mxml_t *CE = NULL;
  mxml_t *DE = NULL;
  mxml_t *JE = NULL;

  char         Message[MMAX_LINE];

  char        *Response = NULL;

  char         CmdString[MMAX_BUFFER];

  char    *ptr;

#ifndef __MPROD  
  const char *FName = "MS3JobSuspend";

  MDB(1,fS3) MLog("%s(%s,%s,%s,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL",
    (EMsg != NULL) ? EMsg : "NULL");
#endif /* !__MPROD */

  if ((J == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  if (MJOBISACTIVE(J) == FALSE)
    {
    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be suspended: (invalid state)\n",
      J->Name);

    return(FAILURE);
    }

  switch (R->Version)
    {
    case msssV3_0:
    case msssV4_0:
    default:

      MXMLCreateE(&E,MSON[msonRequest]);
      MXMLSetAttr(E,MSAN[msanAction],(void *)MS3JAName[R->Version][mrmJobSuspend],mdfString);
   
      MS3SetObject(E,MS3ObjName[R->Version][mxoJob],NULL);

      DE = NULL;
      MXMLCreateE(&DE,MSON[msonData]);
      MXMLAddE(E,DE);

      JE = NULL;
      MXMLCreateE(&JE,MS3ObjName[R->Version][mxoJob]);
      MXMLAddE(DE,JE);

      CE = NULL;
      MXMLCreateE(&CE,MS3JobAttr[R->Version][mjaJobID]);
      MXMLSetVal(CE,(void *)J->Name,mdfString);
      MXMLAddE(JE,CE);

      break;
    }  /* END switch (R->Version) */

  MXMLToString(E,CmdString,sizeof(CmdString),NULL,TRUE);

  MXMLDestroyE(&E);

  if (MS3DoCommand(
        &R->P[0],
        CmdString,
        &Response,
        NULL,
        NULL,
        EMsg) == FAILURE)
    {
    /* job could not be suspended */

    R->FailIteration = MSched.Iteration;

    if (SC != NULL)
      *SC = mscRemoteFailure;

    return(FAILURE);
    }

  if (((ptr = strstr(Response,"<Body")) == NULL) ||
       (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE))
    {
    /* cannot parse response */

    MUFree(&Response);

    R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  MUFree(&Response);

  if (MXMLGetChildCI(E,MSON[msonResponse],NULL,&RE) == FAILURE)
    {
    /* cannot parse response */

    MXMLDestroyE(&E);

    R->FailIteration = MSched.Iteration;
    }

  /* check status code */

  if (MXMLGetChildCI(RE,"Status",NULL,&CE) == FAILURE)
    {
    /* cannot locate status code */

    MXMLDestroyE(&E);

    R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  if (strcmp(CE->Val,"true"))
    {
    mxml_t *tE = NULL;

    /* command failed */

    char tmpMsg[MMAX_LINE];
    int  tmpSC;

    if (MXMLGetChildCI(CE,"Message",NULL,&tE) == SUCCESS)
      {
      MUStrCpy(tmpMsg,tE->Val,sizeof(tmpMsg));
      }
    else
      {
      strcpy(tmpMsg,"NoMessage");
      }

    if (MXMLGetChildCI(RE,"Code",NULL,&tE) == SUCCESS)
      {
      tmpSC = (int)strtol(tE->Val,NULL,0);
      }
    else
      {
      tmpSC = -1;
      }

    snprintf(Message,MMAX_LINE,"cannot suspend job - RM failure, rc: %d, msg: '%s'",
      tmpSC,
      tmpMsg);

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be suspended: RM failure (%s)\n",
      J->Name,
      Message);

    if (EMsg != NULL)
      strcpy(EMsg,Message);

    MUStrDup(&J->Message,Message);

    MXMLDestroyE(&E);

    R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }  /* END if (strcmp(C->Val,"true")) */

  /* job successfully started */

  MXMLDestroyE(&E);

  MSysEMSubmit(
    &MSched.EM,
    (char *)MS3CName[mpstSC],
    (char *)"jobsuspend",
    (char *)J->Name);

  /* adjust state */

  MJobSetState(J,mjsSuspended);

  MDB(1,fS3) MLog("INFO:     job '%s' successfully suspended\n",
    J->Name);

  return(SUCCESS);
  }  /* END MS3JobSuspend() */




int MS3JobResume(

  mjob_t *J,       /* I */
  mrm_t  *R,       /* I */
  char   *EMsg,    /* O (optional) */
  int    *SC)      /* O (optional) */

  {
  char       Message[MMAX_LINE];

  char      *Response = NULL;

  char       CmdString[MMAX_BUFFER];

  char       tmpMsg[MMAX_LINE];
  enum MSFC  tmpSC;

  mxml_t *E  = NULL;
  mxml_t *RE = NULL;
  mxml_t *CE = NULL;
  mxml_t *DE = NULL;
  mxml_t *JE = NULL;

  char *ptr;

#ifndef __MPROD
  const char *FName = "MS3JobResume";

  MDB(1,fS3) MLog("%s(%s,%s,%s,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL",
    (EMsg != NULL) ? EMsg : "NULL");
#endif /* __MPROD */

  if (EMsg != NULL)
    EMsg[0] = '\0';

  if ((J == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  if (MJOBISSUSPEND(J) == FALSE)
    {
    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be resumed: (invalid state)\n",
      J->Name);

    return(FAILURE);
    }

  switch (R->Version)
    {
    case msssV3_0:
    case msssV4_0:
    default:

      MXMLCreateE(&E,MSON[msonRequest]);
      MXMLSetAttr(E,MSAN[msanAction],(void *)MS3JAName[R->Version][mrmJobResume],mdfString);

      MS3SetObject(E,MS3ObjName[R->Version][mxoJob],NULL);

      DE = NULL;
      MXMLCreateE(&DE,MSON[msonData]);
      MXMLAddE(E,DE);

      JE = NULL;
      MXMLCreateE(&JE,MS3ObjName[R->Version][mxoJob]);
      MXMLAddE(DE,JE);

      CE = NULL;
      MXMLCreateE(&CE,MS3JobAttr[R->Version][mjaJobID]);
      MXMLSetVal(CE,(void *)J->Name,mdfString);
      MXMLAddE(JE,CE);

      break;
    }  /* END switch (R->Version) */

  MXMLToString(E,CmdString,sizeof(CmdString),NULL,TRUE);

  MXMLDestroyE(&E);

  if (MS3DoCommand(
        &R->P[0],
        CmdString,
        &Response,
        NULL,
        NULL,
        EMsg) == FAILURE)
    {
    /* job could not be suspended */

    R->FailIteration = MSched.Iteration;

    if (SC != NULL)
      *SC = mscRemoteFailure;

    return(FAILURE);
    }

  if (((ptr = strstr(Response,"<Body")) == NULL) ||
       (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE))
    {
    /* cannot parse response */

    MUFree(&Response);

    R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  MUFree(&Response);

  if (MXMLGetChildCI(E,MSON[msonResponse],NULL,&RE) == FAILURE)
    {
    /* cannot parse response */

    MXMLDestroyE(&E);

    R->FailIteration = MSched.Iteration;
    }

  /* check status code */

  if (MS3CheckStatus(RE,&tmpSC,tmpMsg) == FAILURE)
    {
    /* invalid message status */

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be checkpointed (status failure - %s)\n",
      J->Name,
      (EMsg != NULL) ? EMsg : "N/A");

    MXMLDestroyE(&E);

    if (R->FailIteration == -1)
      R->FailIteration = MSched.Iteration;

    snprintf(Message,MMAX_LINE,"cannot resume job - RM failure, rc: %d, msg: '%s'",
      tmpSC,
      tmpMsg);

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be resumed: RM failure (%s)\n",
      J->Name,
      Message);

    if (EMsg != NULL)
      strcpy(EMsg,Message);

    MUStrDup(&J->Message,Message);

    MXMLDestroyE(&E);

    R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }  /* END if (MS3CheckStatus(RE,&tmpSC,tmpMsg) == FAILURE) */

  /* job successfully resumed */

  MXMLDestroyE(&E);

  MSysEMSubmit(
    &MSched.EM,
    (char *)MS3CName[mpstSC],
    (char *)"jobresume",
    (char *)J->Name);

  /* adjust state */

  MJobSetState(J,mjsSuspended);

  MDB(1,fS3) MLog("INFO:     job '%s' successfully resume\n",
    J->Name);

  return(SUCCESS);
  }  /* END MS3JobResume() */





int MS3JobCheckpoint(

  mjob_t  *J,        /* I */
  mrm_t   *R,        /* I */
  mbool_t  TerminateJobs,  /* I */
  char    *EMsg,     /* O (optional) */
  int     *SC)       /* O (optional) */

  {
  mxml_t *E  = NULL;
  mxml_t *RE = NULL;
  mxml_t *CE = NULL;
  mxml_t *DE = NULL;
  mxml_t *JE = NULL;

  char *ptr;

  char *Response = NULL;

  char  CmdString[MMAX_BUFFER];

  enum MSFC tSC;

#ifndef __MPROD
  const char *FName = "MS3JobCheckpoint";

  MDB(1,fS3) MLog("%s(%s,%s,%s,ErrMsg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL",
    (TerminateJobs == TRUE) ? "TRUE" : "FALSE");
#endif /* !__MPROD */

  if ((J == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  switch (R->Version)
    {
    case msssV3_0:
    case msssV4_0:
    default:

      MXMLCreateE(&E,MSON[msonRequest]);
      MXMLSetAttr(E,MSAN[msanAction],(void *)MS3JAName[R->Version][mrmJobCheckpoint],mdfString);

      MS3SetObject(E,MS3ObjName[R->Version][mxoJob],NULL);

      DE = NULL;
      MXMLCreateE(&DE,MSON[msonData]);
      MXMLAddE(E,DE);

      JE = NULL;
      MXMLCreateE(&JE,MS3ObjName[R->Version][mxoJob]);
      MXMLAddE(DE,JE);

      CE = NULL;
      MXMLCreateE(&CE,MS3JobAttr[R->Version][mjaJobID]);
      MXMLSetVal(CE,(void *)J->Name,mdfString);
      MXMLAddE(JE,CE);

      break;
    }  /* END switch (R->Version) */

  MXMLToString(E,CmdString,sizeof(CmdString),NULL,TRUE);

  MXMLDestroyE(&E);

  if (MS3DoCommand(
        &R->P[0],
        CmdString,
        &Response,
        NULL,
        NULL,
        EMsg) == FAILURE)
    {
    /* job could not be cancelled */

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be checkpointed (command failed)\n",
      J->Name);

    R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  if (((ptr = strstr(Response,"<Body")) == NULL) ||
       (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE))
    {
    /* cannot parse response */

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be checkpointed (invalid response)\n",
      J->Name);

    MUFree(&Response);

    if (R->FailIteration == -1)
      R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  MUFree(&Response);

  if (MXMLGetChildCI(E,MSON[msonResponse],NULL,&RE) == FAILURE)
    {
    /* cannot parse response */

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be checkpointed (cannot parse response)\n",
      J->Name);

    MXMLDestroyE(&E);

    if (R->FailIteration == -1)
      R->FailIteration = MSched.Iteration;
    }

  /* check status code */

  if (MS3CheckStatus(RE,&tSC,EMsg) == FAILURE)
    {
    if (SC != NULL)
      *SC = (int)tSC;

    /* invalid message status */

    MDB(0,fS3) MLog("ERROR:    job '%s' cannot be checkpointed (status failure - %s)\n",
      J->Name,
      (EMsg != NULL) ? EMsg : "N/A");

    MXMLDestroyE(&E);

    if (R->FailIteration == -1)
      R->FailIteration = MSched.Iteration;

    return(FAILURE);
    }

  if (SC != NULL)
    *SC = (int)tSC;

  /* job successfully checkpointed */

  MSysEMSubmit(
    &MSched.EM,
    (char *)MS3CName[mpstSC],
    (char *)"jobcheckpoint",
    (char *)J->Name);

  MDB(1,fS3) MLog("INFO:     job '%s' successfully checkpointed\n",
    J->Name);

  return(SUCCESS);
  }  /* END MS3JobCheckpoint() */





#ifdef MNOT

int __MS3GetTaskList(

  char   TaskString[],
  short  TaskList[],
  int   *TaskCount,
  int   *TPN)

  {
  int     index;
  int     tindex;

  char   *ptr;
  char   *tptr;

  char   *TokPtr;

  mnode_t *N;
  char    tmpHostName[MMAX_NAME];

  int     ppn;

  long    tmpL;
  char   *tail;

  /* FORMAT:  <HOSTNAME>[:ppn=<X>][<HOSTNAME>[:ppn=<X>]]... */

  MDB(5,fS3) MLog("__MS3GetTaskList(%s,%s,%s,%s)\n",
    (TaskString != NULL) ? TaskString : "NULL",
    (TaskList != NULL) ? "TaskList" : "NULL",
    (TaskCount != NULL) ? "TaskCount" : "NULL",
    (TPN != NULL) ? "TPN" : "NULL");

  if ((TaskString == NULL) || (TaskList == NULL))
    {
    return(FAILURE);
    }
 
  ptr = MUStrTok(TaskString,"+ \t",&TokPtr);

  tindex = 0;

  /* NOTE:  only handles one 'ppn' distribution per job */

  ppn = 1;

  while (ptr != NULL)
    {
    ppn = 1;

    strcpy(tmpHostName,ptr);

    if ((tptr = strchr(tmpHostName,'/')) != NULL)
      *tptr = '\0';

    if ((tptr = strchr(tmpHostName,':')) != NULL)
      {
      *tptr = '\0';

      tptr++;

      if (strncmp(tptr,"ppn=",strlen("ppn=")) == 0)
        {
        tptr += strlen("ppn=");

        ppn = strtol(tptr,NULL,10);
        }
      }

    tmpL = strtol(tmpHostName,&tail,0);

    if ((tmpL != 0) && (*tail == '\0'))
      {
      /* nodecount specified */
      }
    else
      { 
      if (MNodeFind(tmpHostName,&N) == FAILURE)
        {
        MDB(1,fS3) MLog("ALERT:    cannot locate host '%s' for job hostlist\n",
          tmpHostName);
        }
      else
        {
        for (index = 0;index < ppn;index++)
          {
          TaskList[tindex] = N->Index;

          tindex++;
          }
        }
      }

    ptr = MUStrTok(NULL,"+ \t",&TokPtr);
    }    /* END while (ptr != NULL) */

  TaskList[tindex] = -1;

  if (TaskCount != NULL)
    *TaskCount = tindex;

  if (TPN != NULL)
    *TPN = ppn;

  MDB(7,fS3) MLog("INFO:     %d task(s) located for job\n",
    tindex);

  if (tindex == 0)
    {
    return(FAILURE);
    }

  return(SUCCESS);
  }    /* END __MS3GetTaskList() */




int MS3JobModify(
 
  mjob_t *J,         /* I (modified) */
  char   *Name,      /* I */
  char   *Resource,  /* I */
  char   *Value)     /* I */
 
  {
  int rc;
 
  char  tmpJName[MMAX_NAME];
  char *ErrMsg;

  if (J == NULL)
    {
    return(FAILURE);
    }
 
  MJobGetName(
    J,
    NULL,
    (J->SRM != NULL) ? J->SRM : &MRM[0],
    tmpJName,
    sizeof(tmpJName),
    mjnRMName);

  /* NYI */
 
  rc = -1;
 
  if (rc != 0)
    {
    ErrMsg = NULL;
 
    MDB(0,fS3) MLog("ERROR:    cannot set job '%s' attr '%s:%s' to '%s' (rc: %d '%s')\n",
      J->Name,
      Name,
      Resource,
      Value,
      rc,
      ErrMsg);
 
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MS3JobModify() */

#endif /* MNOT */




int MS3GetNodeState() { return(SUCCESS); }
int MS3NodeFree() { return(SUCCESS); }


int MS3ProcessQueueInfo() 

  { 
  return(SUCCESS); 
  }  /* END MS3ProcessQueueInfo() */




int MS3EncapsulateMessage(

  char     *DataString, /* I */
  char     *Version,    /* I */
  mbool_t   IsRequest,  /* I */
  char    **SBuf,       /* O */
  long     *SBufSize)   /* I */

  {
  if (SBuf != NULL)
    *SBuf = NULL;

  if ((DataString == NULL) ||
      (SBuf == NULL) ||
      (SBufSize == NULL))
    {
    return(FAILURE);
    }

  if (Version == NULL)
    {
    *SBuf     = DataString;
    *SBufSize = strlen(DataString) + 1;

    return(SUCCESS);
    }

  if (!strcmp(Version,"S32.0"))
    {
    /* NYI */
    }

  return(SUCCESS);
  }  /* END MS3EncapsulateMessage() */




int MS3ExtractMessage(

  char     *DataString, /* I */
  char     *Version,    /* I */
  mbool_t  *IsRequest,
  char    **SBuf,       /* O */
  long     *SBufSize)   /* I */

  {
  if (SBuf != NULL)
    *SBuf = NULL;

  if ((DataString == NULL) ||
      (SBuf == NULL) ||
      (SBufSize == NULL))
    {
    return(FAILURE);
    }

  if (Version == NULL)
    {
    /* FORMAT:  <Message><Response/></Message> */

    *SBuf     = DataString;
    *SBufSize = strlen(DataString) + 1;

    return(SUCCESS);
    }

  if (!strcmp(Version,"S32.0"))
    {
    /* FORMAT:  <Envelope><Request/></Envelope> */

    /* NYI */
    }

  return(SUCCESS);
  }  /* END MS3ExtractMessage() */




#ifndef __M32COMPAT 

int MS3JobSubmit(

  char                 *Data,   /* I (job description) */
  mrm_t                *R,      /* I */
  mjob_t              **JP,     /* O (optional) */
  long                  FlagBM, /* I (enum of XXX) */
  char                 *JobID,  /* O (optional,minsize=MMAX_NAME) */
  char                 *EMsg,   /* O (optional,minsize=MMAX_LINE) */
  enum MStatusCodeEnum *SC)     /* O (optional) */

  {
  mxml_t  *JE;

  s3rm_t *S;

  mjob_t  *J;

  short    TaskList[MMAX_TASK_PER_JOB + 1];

  char     SJobID[MMAX_NAME];
  char    *ptr;

#ifndef __MPROD
  const char *FName = "MS3JobSubmit";

  MDB(1,fS3) MLog("%s(%s,%s,%s,%ld,JobID,%s,SC)\n",
    FName,
    (Data != NULL) ? "Data" : "NULL",
    (R != NULL) ? R->Name : "NULL",
    (JP != NULL) ? "JP" : "NULL",
    FlagBM,
    (EMsg != NULL) ? "EMsg" : "NULL");
#endif /* !__MPROD */

  if (EMsg != NULL)
    EMsg[0] = '\0';

  if ((Data == NULL) || (R == NULL))
    {
    if (EMsg != NULL)
      strcpy(EMsg,"internal error");

    return(FAILURE);
    }

  if (!MISSET(R->Flags,mrmfLocalQueue))
    {
    /* can only submit to local queue */

    if (EMsg != NULL)
      strcpy(EMsg,"invalid resource manager");

    return(FAILURE);
    }

  S = (s3rm_t *)R->S;

  /* parse job */

  JE = NULL;

  if (!MISSET(FlagBM,mjsufTemplateJob))
    {
    if (MXMLFromString(&JE,Data,FALSE,0) == FAILURE)
      {
      /* cannot process job */
 
      if (EMsg != NULL)
        strcpy(EMsg,"cannot parse job");
 
      return(FAILURE);
      }
    }

  /* validate job */

  if (!MISSET(FlagBM,mjsufTemplateJob))
    {
    if (MSched.Name[0] != '\0')
      {
      snprintf(SJobID,sizeof(SJobID),"%s.%d",
        MSched.Name,
        R->JobCounter++);
      }
    else
      {
      snprintf(SJobID,sizeof(SJobID),"Moab.%d",
        R->JobCounter++);
      }
    }
  else
    {
    MUStrCpy(SJobID,((mjob_t *)Data)->Name,sizeof(SJobID));
    }

  if (MJobCreate(SJobID,TRUE,&J) == FAILURE)
    {
    MDB(1,fS3) MLog("ERROR:    job buffer is full  (ignoring job '%s')\n",
      SJobID);

    if (EMsg != NULL)
      strcpy(EMsg,"job buffer is full");

    return(FAILURE);
    }

  if (MISSET(FlagBM,mjsufTemplateJob))
    {
    MRMJobPreLoad(J,SJobID,R);

    MJobMoveAttr((mjob_t *)Data,J,FALSE,NULL,NULL);
    }
  else
    {
    MRMJobPreLoad(J,SJobID,R);
    }

  /* if new job, load data */

  if (MISSET(FlagBM,mjsufGlobalQueue))
    {
    MSET(J->SpecFlags,mjfGlobalQueue);
    }

  MJobSetAttr(J,mjaSRMJID,(void **)SJobID,mdfString,mSet);

  if (!MISSET(FlagBM,mjsufTemplateJob))
    {
    if (MS3JobLoad(SJobID,JE,&J,TaskList,R) == FAILURE)
      {
      MDB(1,fS3) MLog("ALERT:    cannot load S3 job '%s'\n",
        SJobID);

      /* cannot parse/validate job description */

      MXMLDestroyE(&JE);

      MJobDestroy(&J);

      if (EMsg != NULL)
        strcpy(EMsg,"cannot valid job");

      return(FAILURE);
      }
    }
  else
    {
    if (MReqCreate(J,J->Req[0],NULL,FALSE) == FAILURE)
      {
      MDB(1,fS3) MLog("ALERT:    cannot add requirements to job '%s'\n",
        J->Name);

      MJOBREMOVE(JP);

      if (EMsg != NULL)
        strcpy(EMsg,"cannot add job requirements");

      return(FAILURE);
      }
    }

  /* check if tid was passed in to RMXString */

  if ((J->RMXString != NULL) &&
     ((ptr = strstr(J->RMXString,"TID")) != NULL))
    {
    char *TSpec1;
    char *TSpec2;
    char *TSpec3;
    char *TSpec4;
    char *TSpec5;
    char *TSpec6;
    char *TSpec7;
    char *TSpec8;
    char *TSpec9;

    /* TSpec1 -> HostList  mjaAllocNodeList
       TSpec2 -> Duration  mjaReqAWDuration
       TSpec3 -> StartTime mjaReqSMinTime
       TSpec4 -> Flag      ignored (NYI)
       TSpec5 -> OS        ignored (NYI)  
       TSpec9 -> Dep TID List ignored (NYI) */
 
    int TID;

    /* grab tid and overwrite specified values */

    ptr += strlen("tid:");

    TID = strtol(ptr,NULL,0);

    if (MTransFind(
          TID,
          &TSpec1,
          &TSpec2,
          &TSpec3,
          &TSpec4,
          &TSpec5,
          &TSpec6,
          &TSpec7,
          &TSpec8,
          &TSpec9) == SUCCESS)
      {
      if (TSpec1 != NULL)
        {
        int index;

        char tmpBuf[MMAX_BUFFER];

        char *ptr;
        char *TokPtr;

        mreq_t *RQ = J->Req[0];

        MUStrCpy(tmpBuf,TSpec1,sizeof(tmpBuf));

        ptr = MUStrTok(tmpBuf,",",&TokPtr);

        index = 0;

        while (ptr != NULL)
          {
          index++;

          ptr = MUStrTok(NULL,",",&TokPtr);
          }

        MJobSetAttr(J,mjaHostList,(void **)TSpec1,mdfString,mSet);

        MReqSetAttr(J,RQ,mrqaTCReqMin,(void **)&index,mdfInt,mSet);

        J->Request.TC  = index;
        RQ->TaskCount  = index;

        /* grab all processors on each host in list */
        /* NOTE: will not work for non-compute resources FIXME */

        RQ->DRes.Procs = -1;

        RQ->TaskRequestList[0] = J->Request.TC;
        RQ->TaskRequestList[1] = J->Request.TC;
        RQ->TaskRequestList[2] = 0;
        }

      if (TSpec2 != NULL)
        {
        MJobSetAttr(J,mjaReqAWDuration,(void **)TSpec2,mdfString,mSet);
        }

      if (TSpec3 != NULL)
        {
        MJobSetAttr(J,mjaReqSMinTime,(void **)TSpec3,mdfString,mSet);
        }
      }   /* END if (MTransFind()) */
    }     /* END if ((J->RMXString != NULL) && ... */

  if (JE != NULL)
    MXMLDestroyE(&JE);

  if (MRMJobPostLoad(J,TaskList,R,EMsg) == FAILURE)
    {
    MDB(1,fS3) MLog("INFO:     job '%s' is invalid/cannot be processed\n",
      J->Name);

    MJOBREMOVE(JP);

    return(FAILURE);
    }

  if (MISSET(R->Flags,mrmfLocalQueue))
    {
    int rqindex;

    mreq_t *RQ;

    MS3AddLocalJob(R,SJobID);

    J->SRM = R;

    for (rqindex = 0;rqindex < MMAX_REQ_PER_JOB;rqindex++)
      {
      RQ = J->Req[rqindex];

      if (RQ == NULL)
        break;

      RQ->RMIndex = R->Index;
      }  /* END for (rqindex) */
    }    /* END if (MISSET(R->Flags,mrmfLocalQueue)) */

  /* add job to internal queue */

  /* decision to process job immediately or wait for next iteration */
  /* is made at higher level */

  if (JP != NULL)
    *JP = J;

  return(SUCCESS);
  }  /* END MS3JobSubmit() */

#endif /* __M32COMPAT */
 



int MS3AddWhere(

  mxml_t  *E,     /* I (modified) */
  char    *Name,  /* I */
  char    *Value, /* I */
  mxml_t **WEP)   /* O (optional) */

  {
  mxml_t *WE;

  if (WEP != NULL)
    *WEP = NULL;

  if ((E == NULL) || (Name == NULL) || (Value == NULL))
    {
    return(FAILURE);
    }

  WE = NULL;

  MXMLCreateE(&WE,MSON[msonWhere]);
  MXMLSetAttr(WE,MSAN[msanName],(void *)Name,mdfString);

  if (Value != NULL)
    {
#ifdef __MS330
    MXMLSetVal(WE,(void *)Value,mdfString);
#else /* __MS330 */
    MXMLSetAttr(WE,MSAN[msanValue],(void *)Value,mdfString);
#endif /* __MS330 */
    }  /* END if (Value != NULL) */

  MXMLAddE(E,WE);

  if (WEP != NULL)
    *WEP = WE;

  return(SUCCESS);
  }  /* END MS3AddWhere() */





int MS3GetWhere(

  mxml_t  *E,     /* I */
  mxml_t **WEP,   /* O (optional) */
  int     *STok,  /* I/O (optional) */
  char    *Name,  /* O */
  int      NSize, /* I */
  char    *Value, /* O */
  int      VSize) /* I */

  {
  mxml_t *WE;

  if (WEP != NULL)
    *WEP = NULL;

  if (Name != NULL)
    Name[0] = '\0';

  if (Value != NULL)
    Value[0] = '\0';

  if (E == NULL)
    {
    return(FAILURE);
    }

  if (MXMLGetChild(E,MSON[msonWhere],STok,&WE) == FAILURE)
    {
    return(FAILURE);
    }

  MXMLGetAttr(WE,MSAN[msanName],NULL,Name,NSize);

#ifdef __MS330

  if (WE->Val != NULL)
    MUStrCpy(Value,WE->Val,VSize);

#else /* __MS330 */

  MXMLGetAttr(WE,MSAN[msanValue],NULL,Value,VSize);

#endif /* __MS330 */

  if (WEP != NULL)
    *WEP = WE;

  return(SUCCESS);
  }  /* END MS3GetWhere() */





int MS3AddSet(

  mxml_t  *E,     /* I (modified) */
  char    *Name,  /* I */
  char    *Value, /* I */
  mxml_t **SEP)   /* O (optional) */

  {
  mxml_t *SE;

  if (SEP != NULL)
    *SEP = NULL;

  if ((E == NULL) || (Name == NULL))
    {
    return(FAILURE);
    }

  SE = NULL;

  MXMLCreateE(&SE,MSON[msonSet]);
  MXMLSetAttr(SE,MSAN[msanName],(void *)Name,mdfString);

  if (Value != NULL)
    {
#ifdef __MS330
    MXMLSetVal(SE,(void *)Value,mdfString);
#else /* __MS330 */
    MXMLSetAttr(SE,MSAN[msanValue],(void *)Value,mdfString);
#endif /* __MS330 */
    }  /* END if (Value != NULL) */

  MXMLAddE(E,SE);

  if (SEP != NULL)
    {
    *SEP = SE;
    }

  return(SUCCESS);
  }  /* END MS3AddSet() */





int MS3AddGet(

  mxml_t  *E,     /* I (modified) */
  char    *Name,  /* I */
  mxml_t **GEP)   /* O (optional) */

  {
  mxml_t *GE;

  if (GEP != NULL)
    *GEP = NULL;

  if ((E == NULL) || (Name == NULL))
    {
    return(FAILURE);
    }

  GE = NULL;

  MXMLCreateE(&GE,MSON[msonGet]);

  if (Name != NULL)
    {
#ifdef __MS330
    MXMLSetAttr(GE,MSAN[msanName],(void *)Name,mdfString);
#else /* __MS330 */
    MXMLSetAttr(GE,MSAN[msanName],(void *)Name,mdfString);
#endif /* __MS330 */
    }  /* END if (Value != NULL) */

  MXMLAddE(E,GE);

  if (GEP != NULL)
    {
    *GEP = GE;
    }

  return(SUCCESS);
  }  /* END MS3AddGet() */




int MS3GetSet(

  mxml_t  *E,     /* I */
  mxml_t **SEP,   /* O (optional) */
  int     *STok,  /* I/O (optional) */
  char    *Name,  /* O */
  int      NSize, /* I */
  char    *Value, /* O */
  int      VSize) /* I */
 
  {
  mxml_t *SE;

  if (SEP != NULL)
    *SEP = NULL;

  if (Name != NULL)
    Name[0] = '\0';

  if (Value != NULL)
    Value[0] = '\0';

  if (E == NULL)
    {
    return(FAILURE);
    }

  if (MXMLGetChild(E,MSON[msonSet],STok,&SE) == FAILURE)
    {
    return(FAILURE);
    }

  MXMLGetAttr(SE,MSAN[msanName],NULL,Name,NSize);

#ifdef __MS330

  if (SE->Val != NULL)
    MUStrCpy(Value,SE->Val,VSize);

#else /* __MS330 */

  MXMLGetAttr(SE,MSAN[msanValue],NULL,Value,VSize);

#endif /* __MS330 */

  if (SEP != NULL)
    *SEP = SE;
 
  return(SUCCESS);  
  }  /* END MS3GetSet() */





int MS3GetGet(

  mxml_t  *E,     /* I */
  mxml_t **GEP,   /* O (optional) */
  int     *GTok,  /* I/O (optional) */
  char    *Name,  /* O */
  int      NSize) /* I */

  {
  mxml_t *GE;

  if (GEP != NULL)
    *GEP = NULL;

  if (Name != NULL)
    Name[0] = '\0';

  if (E == NULL)
    {
    return(FAILURE);
    }

  if (MXMLGetChild(E,MSON[msonGet],GTok,&GE) == FAILURE)
    {
    return(FAILURE);
    }

#ifdef __MS330

  if (GE->Val != NULL)
    MUStrCpy(Name,GE->Val,NSize);

#else /* __MS330 */

  MXMLGetAttr(GE,MSAN[msanName],NULL,Name,NSize);

#endif /* __MS330 */

  if (GEP != NULL)
    *GEP = GE;

  return(SUCCESS);
  }  /* END MS3GetGet() */






int MS3SetObject(

  mxml_t *E,      /* I */
  char   *OName,  /* I */
  char   *Value)  /* I (optional) */

  {
#ifndef MOPT

  if ((E == NULL) || (OName == NULL))
    {
    return(FAILURE);
    }

#endif /* MOPT */

#ifdef __MS330

  {
  mxml_t *OE = NULL;
 
  MXMLCreateE(&OE,MSON[msonObject]);
  MXMLSetVal(OE,(void *)OName,mdfString);

  MXMLAddE(E,OE);
  }  /* END BLOCK */

#else /* __MS330 */

  MXMLSetAttr(E,MSAN[msanObject],(void *)OName,mdfString);

#endif /* __MS330 */

  return(SUCCESS);
  }  /* END MS3SetObject() */




int MS3GetObject(

  mxml_t *E,     /* I */
  char   *OName) /* O (optional,minsize=MMAX_NAME) */

  {
  if (OName != NULL)
    OName[0] = '\0';

  if (E == NULL)
    {
    return(FAILURE);
    }

#ifdef __MS330

  {
  mxml_t *OE;

  if (MXMLGetChildCI(E,(char *)MSON[msonObject],NULL,&OE) == SUCCESS)
    {
    if (OE->Val != NULL)
      {
      if (OName != NULL)
        MUStrCpy(OName,OE->Val,MMAX_NAME);

      return(SUCCESS);
      }
    }
  }  /* END BLOCK */

#else /* __MS330 */

  if (MXMLGetAttr(E,MSAN[msanObject],NULL,OName,MMAX_NAME) == SUCCESS)
    {
    return(SUCCESS);
    }

#endif /* __MS330 */

  return(FAILURE);
  }  /* END MS3GetObject() */




int MS3InitializeLocalQueue(

  mrm_t *R,      /* I (modified) */
  char  *JList)  /* I (optional) */

  {
  char *ptr;
  char *TokPtr;

  char **JL;

  int   jindex;

  if (R == NULL)
    {
    return(FAILURE);
    }

  if (R->U.S3.JobList == NULL)
    R->U.S3.JobList = calloc(1,sizeof(char *) * (MMAX_JOB + 1));

  if (R->S == NULL)
    R->S = (void *)calloc(1,sizeof(s3rm_t));

  JL = (char **)R->U.S3.JobList;

  if (JList != NULL)
    {
    jindex = 0;

    ptr = MUStrTok(JList,", \t\n",&TokPtr);

    while (ptr != NULL)
      {
      MUStrDup(&JL[jindex],ptr);

      if (jindex >= MMAX_JOB)
        break;
     
      jindex ++;
 
      ptr = MUStrTok(NULL,", \t\n",&TokPtr);
      }  /* END while (ptr != NULL) */

    JL[jindex] = NULL;
    }    /* END if (JList != NULL) */
 
  return(SUCCESS);
  }  /* END MS3InitializeLocalQueue() */




int MS3AddLocalJob(

  mrm_t *R,    /* I (modified) */
  char  *JID)  /* I */

  {
  int   jindex;

  char **JL;

  int   aslot = -1;

  if ((R == NULL) || (R->U.S3.JobList == NULL) || (JID == NULL))
    {
    return(FAILURE);
    }

  JL = (char **)R->U.S3.JobList;

  for (jindex = 0;jindex < MMAX_JOB;jindex++)
    {
    if (JL[jindex] == NULL)
      {
      aslot = jindex;

      break;
      }

    if (JL[jindex] == (char *)1)
      {
      if (aslot == -1)
        {
        aslot = jindex;
        }

      continue;
      }

    if (!strcmp(JID,JL[jindex]))
      {
      return(SUCCESS);
      }
    }  /* END for (jindex) */

  if (aslot == -1)
    {
    return(FAILURE);
    }
  
  if (JL[jindex] == (char *)1)
    JL[jindex] = NULL;
 
  MUStrDup(&JL[aslot],JID);

  return(SUCCESS);
  }  /* END MS3AddLocalJob() */




int MS3RemoveLocalJob(

  mrm_t *R,    /* I (modified) */
  char  *JID)  /* I (optional) */

  {
  int   jindex;

  char **JL;

  if ((R == NULL) || (R->U.S3.JobList == NULL) || (JID == NULL))
    {
    return(FAILURE);
    }

  JL = (char **)R->U.S3.JobList;

  for (jindex = 0;jindex < MMAX_JOB;jindex++)
    {
    if (JL[jindex] == NULL)
      {
      return(SUCCESS);
      }

    if (JL[jindex] == (char *)1)
      {
      continue;
      }

    if (!strcmp(JID,JL[jindex]))
      {
      MUFree(&JL[jindex]);

      JL[jindex] = (char *)1;
      }
    }  /* END for (jindex) */

  return(SUCCESS);
  }  /* END MS3RemoveLocalJob() */




int MS3CheckStatus(

  mxml_t    *E,    /* I */
  enum MSFC *SC,   /* O (optional) */
  char      *EMsg) /* O (optional,minsize=MMAX_LINE) */

  {
  mxml_t *CE;

#ifdef __MS330
  mxml_t *SE;
#endif /* __MS330 */

#ifndef MOPT

  if (E == NULL)
    {
    if (SC != NULL)
      *SC = msfEGMisc;

    if (EMsg != NULL)
      strcpy(EMsg,"internal error");

    return(FAILURE);
    }

#endif /* MOPT */

  /* set defaults */

  if (SC != NULL)
    *SC = msfENone;

  if (EMsg != NULL)
    EMsg[0] = '\0';

#ifdef __MS330

  /* 
     <Status>
       <Value>Success|Warning|Failure</Value>  -- required
       <Code>X</Code>                          -- required
       <Message>X</Message>                    -- optional
     </Status>
  */

  if (MXMLGetChildCI(E,"Status",NULL,&SE) == FAILURE)
    {
    /* cannot locate status element */

    MDB(3,fS3) MLog("ERROR:    cannot locate message status element\n");

    if (SC != NULL)
      *SC = msfEGMessage;

    if (EMsg != NULL)
      strcpy(EMsg,"no status");

    return(FAILURE);
    }

  if ((MXMLGetChildCI(SE,"Message",NULL,&CE) == FAILURE) ||
      (CE->Val == NULL))
    {
    /* cannot locate optional status message */

    MDB(6,fS3) MLog("INFO:     cannot locate status message element\n");
    }
  else
    {
    if (EMsg != NULL)
      MUStrCpy(EMsg,CE->Val,MMAX_LINE);
    }

  if ((MXMLGetChildCI(SE,"Code",NULL,&CE) == FAILURE) ||
      (CE->Val == NULL))
    {
    /* cannot locate status code */

    MDB(3,fS3) MLog("WARNING:  cannot locate status code element\n");

    if (SC != NULL)
      *SC = msfEGMessage;

    if ((EMsg != NULL) && (EMsg[0] == '\0'))
      strcpy(EMsg,"no status code");

    return(FAILURE);
    }

  if (SC != NULL)
    {
    *SC = (enum MSFC)strtol(CE->Val,NULL,0);
    }

  if (MXMLGetChildCI(SE,"Value",NULL,&CE) == FAILURE)
    {
    /* cannot locate status code */

    MDB(3,fS3) MLog("WARNING:  cannot locate status value element\n");

    if ((EMsg != NULL) && (EMsg[0] == '\0'))
      strcpy(EMsg,"no status value");

    return(FAILURE);
    }

  if ((CE->Val == NULL) || !strcmp(CE->Val,"Failure"))
    {
    /* command failed */

    MDB(3,fS3) MLog("WARNING:  request failed\n");

    return(FAILURE);
    }

#else /* __MS330 */

  if (MXMLGetChildCI(E,"Status",NULL,&CE) == FAILURE)
    {
    /* cannot locate status code */

    MDB(3,fS3) MLog("ERROR:    cannot locate message status element\n");

    if (SC != NULL)
      *SC = msfEGMessage;

    if (EMsg != NULL)
      strcpy(EMsg,"no status");

    return(FAILURE);
    }

  if (SC != NULL)
    {
    mxml_t *tE;

    if ((MXMLGetChildCI(CE,"Code",NULL,&tE) == SUCCESS) &&
        (tE->Val != NULL))
      {
      *SC = strtol(tE->Val,NULL,0);
      }
    else
      {
      *SC = msfEGMessage;
      }
    }

  if ((CE->Val == NULL) || strcmp(CE->Val,"true"))
    {
    /* command failed */

    MDB(3,fS3) MLog("ERROR:    request refused\n");

    if ((EMsg != NULL) && (MXMLGetChild(E,"Message",NULL,&CE) == SUCCESS))
      {
      strcpy(EMsg,CE->Val);
      }

    return(FAILURE);
    }

#endif /* !__MS330 */

  return(SUCCESS);
  }  /* END MS3CheckStatus() */




int MS3SetStatus(

  mxml_t *E,      /* I (modified) */
  char   *Value,  /* I */
  enum MSFC Code, /* I */
  char   *Msg)    /* I (optional) */

  {
  mxml_t *SE;
  mxml_t *CE;

  char    tmpLine[MMAX_LINE];

#ifndef MOPT

  if ((E == NULL) || (Value == NULL))
    {
    return(FAILURE);
    }

#endif /* MOPT */

  SE = NULL;

  MXMLCreateE(&SE,"Status");

  MXMLSetVal(
    SE,
    (void *)Value,
    mdfString);

  MXMLAddE(E,SE);

  CE = NULL;

  MXMLCreateE(&CE,"Code");

  sprintf(tmpLine,"%03d",
    Code % 1000);

  MXMLSetVal(
    CE,
    (void *)tmpLine,
    mdfString);

  MXMLAddE(SE,CE);

  if (Msg != NULL)
    {
    CE = NULL;
    MXMLCreateE(&CE,"Message");

    MXMLSetVal(
      CE,
      (void *)Msg,
      mdfString);

    MXMLAddE(SE,CE);
    }  /* END if (Msg != NULL) */

  return(SUCCESS);
  }  /* END MS3SetStatus() */





#ifndef __M32COMPAT

int MS3JobToXML(
                                                                                                                                                             
  mjob_t  *J,        /* I */
  mxml_t **JEP,      /* O */
  enum MJobAttrEnum *SJAList,  /* I (optional) */
  enum MReqAttrEnum *SRQAList, /* I (optional) */
  char    *NullVal)  /* I (optional) */
                                                                                                                                                             
  {
  /* needs to be changed so that the job is converted to S3-XML */
  /* using S3 attributes */

  return(MJobToXML(J,JEP,SJAList,SRQAList,NullVal,FALSE));
  } /* END MS3JobToXML() */

#endif /* __M32COMPAT */

/* END MS3I.c */


