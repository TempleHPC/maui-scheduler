/*
*/
        
#ifndef __MWIKI_H__
#define __MWIKI_H__

#include "msched-common.h"

#define WIKI_MAX_JOB           512
#define WIKI_MAX_NODE          128
#define WIKI_MAX_FRAG_PER_JOB    8
#define WIKI_MAX_NODE_PER_JOB  256

#define DEFAULT_CONFIGFILE       "/etc/rm.cfg"

#define DEFAULT_WMTCPPORT    30002
#define DEFAULT_WMUDPPORT    30003
#define DEFAULT_JMTCPPORT    30004
#define DEFAULT_JMUDPPORT    30005
#define DEFAULT_NMTCPPORT    30006
#define DEFAULT_TMTCPPORT    30007

#define DEFAULT_WMTIMEOUT    10000000
#define DEFAULT_JMTIMEOUT    10000000
#define DEFAULT_NMTIMEOUT    10000000
#define DEFAULT_TMTIMEOUT    10000000

#define DEFAULT_JOBMANCOMMAND  "/usr/local/bin/jobman"
#define DEFAULT_NODEMANCOMMAND "/usr/local/bin/nodeman"
#define DEFAULT_TASKMANCOMMAND "/usr/local/bin/taskman"
#define DEFAULT_RSHCOMMAND     "rsh"

#define DEFAULT_LOCALFILESYSTEM "/tmp"

#define MAX_HIER_CHILD          32
#define MAX_CHILD_PER_LEVEL    128

#define MAX_WIKINODEATTR        22
#define MAX_WIKIJOBATTR         38  

enum { 
  sigAlive = 0
};

enum {
  cmdWMNone = 0,
  cmdWMSubmitJob,
  cmdWMGetJobs,
  cmdWMGetNodes,
  cmdWMUpdateJob,
  cmdWMStartJob,
  cmdWMCancelJob,
  cmdWMSuspendJob,
  cmdWMResumeJob,
  cmdWMCheckPointJob,
  cmdWMMigrateJob,
  cmdWMPurgeJob,
  cmdWMStartTask,
  cmdWMKillTask,
  cmdWMSuspendTask,
  cmdWMResumeTask,
  cmdWMGetTaskStatus,
  cmdWMGetNodeStatus,
  cmdWMGetJobStatus,
  cmdWMUpdateTaskStatus,
  cmdWMUpdateNodeStatus,
  cmdWMUpdateJobStatus,
  cmdWMAllocateNode,
  cmdWMStartNodeman,
  cmdWMSpawnTaskman,
  cmdWMSpawnJobman,
  cmdWMReconfig,
  cmdWMShutdown,
  cmdWMDiagnose,
  cmdWMReparent
  };

enum {
  woNone = 0,
  woNodeStatus,
  woJobStatus
  };

enum {
  wnaNone = 0,
  wnaUpdateTime,
  wnaState,
  wnaOS,
  wnaArch,
  wnaCacheSize,
  wnaL2CacheSize,
  wnaConfMemory,
  wnaAvailMemory,
  wnaConfSwap,
  wnaAvailSwap,
  wnaConfLocalDisk,
  wnaAvailLocalDisk,
  wnaConfProcCount,
  wnaAvailProcCount,
  wnaConfNetworkList,
  wnaAvailNetworkList,
  wnaCPULoad,
  wnaMaxTasks
  };

enum {
  weNone = 0,
  weNoReport,
  weNoParent
  };

enum {
  wsNone = 0,
  wsJob,
  wsNode
  };

typedef struct {
  char  Name[MAX_MNAME];

  long  MTime;

  char  State[MAX_MNAME];

  char  OS[MAX_MNAME];
  char  Arch[MAX_MNAME];

  int   CacheSize;
  int   L2CacheSize;

  int   ConfMemory;
  int   AvailMemory;

  int   ConfSwap;
  int   AvailSwap;

  int   ConfLocalDisk;
  int   AvailLocalDisk;

  int   ConfProcCount;
  int   AvailProcCount;

  char  ConfNetworkList[MAX_MLINE];
  char  AvailNetworkList[MAX_MLINE];

  double CPULoad;

  int   MaxTasks;
  } wiki_node_t;

typedef struct {
  char Arch[MAX_MNAME];
  char OS[MAX_MNAME];
  char Network[MAX_MNAME];
  char Features[MAX_MNAME];
  
  int  ProcCount;
  int  ProcCountCmp;
  int  Memory;
  int  MemCmp;
  int  Disk;
  int  DiskCmp;
  int  Swap;
  int  SwapCmp;

  char IWD[MAX_PATH_LEN];
  char Args[MAX_MNAME];
  char Env[MAX_MNAME];
  char Input[MAX_PATH_LEN];
  char Output[MAX_PATH_LEN];
  char Error[MAX_PATH_LEN];
  char Exec[MAX_PATH_LEN];
  char Shell[MAX_PATH_LEN];
  
  time_t QueueTime;
  time_t StartTime;
  time_t CompletionTime;

  int   Status;

  char  JobMonServer[MAX_MNAME];
  int   JobMonPort;

  char  HostName[MAX_MNODE_PER_FRAG + 1][MAX_MNAME];
  char  HostStatus[MAX_MNODE_PER_FRAG + 1];

  short TaskList[MAX_MTASK_PER_FRAG + 1];

  int   TaskCount;

  char Buf[5 * MAX_MNAME];
  } wiki_frag_t;

typedef struct {
  char Name[MAX_MNAME];

  long MTime;

  int  Status;

  long StateMTime;

  int  Version;

  int  WCLimit;

  int  UID;
  int  GID;
  char Account[MAX_MNAME];

  char PartitionList[MAX_MNAME];

  wiki_frag_t Frag[WIKI_MAX_FRAG_PER_JOB];
  int         FragCount;

  char        NodeList[MAX_MNODE_PER_JOB + 1][MAX_MNAME];
  short       TaskMap[MAX_MTASK_PER_JOB + 1];

  char        JobmanHost[MAX_MNAME];
  int         JobmanPort;
  } wiki_job_t;

typedef struct {
  char Name[MAX_MNAME];

  char Exec[MAX_PATH_LEN];

  char Shell[MAX_PATH_LEN];

  char Args[MAX_MNAME];

  char Input[MAX_PATH_LEN];
  char Output[MAX_PATH_LEN];
  char Error[MAX_PATH_LEN];

  char IWD[MAX_PATH_LEN];

  char Env[MAX_MLINE];

  int  UID;
  int  GID;

  int  TID;
  int  PID;
  int  SID;
} wiki_task_t;

typedef struct {
  char HostName[MAX_MNAME];
  int  MsgType;
  int  Port;
  } wiki_jobstatus_t;

typedef struct {
  char Buf[MAX_MLINE];
  } wiki_stat_t;

enum MWNodeAttrEnum {
  mwnaNONE = 0,
  mwnaUpdateTime,
  mwnaState,
  mwnaOS,
  mwnaArch,
  mwnaCMemory,
  mwnaAMemory,
  mwnaCSwap,
  mwnaASwap,
  mwnaCDisk,
  mwnaADisk,
  mwnaCProc, 
  mwnaAProc,
  mwnaCNet,
  mwnaANet,
  mwnaCRes,
  mwnaARes,
  mwnaCPULoad,
  mwnaCClass,
  mwnaAClass,
  mwnaFeature,
  mwnaPartition,
  mwnaEvent,
  mwnaCurrentTask,
  mwnaMaxTask,
  mwnaSpeed,
  mwnaName
  };

enum MWJobAttrEnum {
  mwjaNONE = 0,
  mwjaUpdateTime,
  mwjaState,
  mwjaWCLimit,
  mwjaTasks,
  mwjaNodes,
  mwjaGeometry,
  mwjaQueueTime,
  mwjaStartDate,
  mwjaStartTime,
  mwjaCompletionTime,
  mwjaUName,
  mwjaGName,
  mwjaAccount,
  mwjaRFeatures,
  mwjaRNetwork,
  mwjaDNetwork,
  mwjaRClass,
  mwjaROpsys,
  mwjaRArch,
  mwjaRMem,
  mwjaRMemCmp,
  mwjaDMem,
  mwjaRDisk,
  mwjaRDiskCmp,
  mwjaDDisk,
  mwjaRSwap,
  mwjaRSwapCmp,
  mwjaDSwap,
  mwjaPartitionList,
  mwjaExec,
  mwjaArgs,
  mwjaIWD,
  mwjaComment,
  mwjaRejectionCount,
  mwjaRejectionMessage,
  mwjaRejectionCode,
  mwjaEvent,
  mwjaTaskList,
  mwjaTaskPerNode,
  mwjaQOS,
  mwjaEndDate,
  mwjaDProcs,
  mwjaHostList,
  mwjaSuspendTime,
  mwjaResAccess,
  mwjaName,
  mwjaEnv,
  mwjaInput,
  mwjaOutput,
  mwjaError,
  mwjaFlags,
  mwjaSubmitString,
  mwjaSubmitType
  };

#endif /* __MWIKI_H__ */

