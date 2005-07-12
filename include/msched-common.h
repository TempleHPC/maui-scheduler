/*
*/

#ifndef __M_COMMON_H__
#define __M_COMMON_H__

#ifdef __MTHREADS
#  include <pthread.h>
#endif /* __MTHREADS */


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>

#include <unistd.h>
#include <strings.h>
#include <grp.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define NONE                                   "[NONE]"
#define ALL                                    "[ALL]"
#define NOBODY                                 "[NOBODY]"

#ifndef DEFAULT
# define DEFAULT                                "[DEFAULT]"
#endif /* DEFAULT */

#define MAX_PATH_LEN        256
#define MAX_SBUFFER      262144

#define MAX_MPATH_LEN       256
#define MMAX_PATH_LEN       256

#define MAX_MLINE          1024
#define MMAX_LINE          1024
#define MAX_MBUFFER       65536
#define MMAX_BUFFER       65536
#define MAX_MSBUFFER     262144
#define MAX_MENVVAR          64

#define MAX_USHORT        65532

#define MAX_MARG             32
#define MMAX_ARG             32

#define MAX_TOKEN            64

#define MAX_MTIME    2140000000  
#define MAX_MEFFINF    50000000   /* ~ 1.5 years */

#ifndef MAX_MNODE
# define MAX_MNODE          5120
#endif /* MAX_MNODE */

#define MMAX_NODE           5120

#define MAX_MNODE_PER_JOB   1024
#define MAX_MNODE_PER_FRAG  1024

#ifndef MAX_MTASK
# define MAX_MTASK         4096
#endif /* MAX_MTASK */

#define MAX_MTASK_PER_FRAG  MAX_MTASK
#define MAX_MTASK_PER_JOB   MAX_MTASK
#define MMAX_TASK_PER_JOB   MAX_MTASK


#ifndef MAX_LONG 
# define MAX_LONG              0x7fffffff
#endif /* MAX_LONG */

#ifndef MAX_INT
# define MAX_INT               0x7fffffff           
#endif /* MAX_INT */

#ifndef MAX_CHAR
# define MAX_CHAR              0xff
#endif /* MAX_CHAR */

#define MAX_MCLIENT               10
#define MMAX_CLIENT               10

#define DAY_LEN               86400
#define WEEK_LEN             604800

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef MBNOTSET
#define MBNOTSET 2
#endif /* MBNOTSET */

#ifndef SUCCESS               
#define SUCCESS              1
#endif

#ifndef FAILURE              
#define FAILURE              0
#endif

#define TCP                  1
#define UDP                  2

enum MSockFTypeEnum {
  msftNONE = 0,
  msftTCP,
  msftUDP };

#define MLog DPrint

/* value types */
 
enum {
  cNONE = 0,
  cSString,
  cSNum,
  cSStr,
  cMNum,
  cMStr,
  cSFloat,
  cSIntArray,
  cSFloatArray,
  cMStrArray };

typedef struct {
  mulong StartTime;
  mulong EndTime;
  int    TaskCount;
  int    NodeCount;
  char   Affinity;
  } mrange_t;



#define DSTAT_STEPSIZE  64

typedef struct {
  char *Data;
  int   DSize;
  int   Count;
  int   Size;
  } dstat_t;

typedef struct {
  char    FileName[MAX_PATH_LEN];
  time_t  ProgTimeStamp;
  time_t  FileTimeStamp;
  char   *Buffer;
  int     BufSize;
  } mfcache_t;

typedef struct {
  char    FileName[MAX_PATH_LEN];
  int     Index;
  } mfindex_t;

typedef struct {
  int     Offset;
  char    Key[MAX_MNAME];
  } rindex_t;

#define MAX_MACL  32

/* node accessibility */

enum {
  nmUnavailable = 0,
  nmRequired,
  nmPositiveAffinity,
  nmNeutralAffinity,
  nmNegativeAffinity,
  nmPreemptible,
  nmNone               /* no affinity (ie, no reservation) */
  };

enum MACLEnum {
  maclNONE = 0,
  maclDeny,
  maclXOR,
  maclRequired };

typedef struct {
  char Name[MAX_MNAME];
  long Value;
  long Flags;
  char Type;
  char Cmp;
  char Affinity;
  } macl_t;

#define MAX_FILECACHE        32
#define MAX_FBUFFER    16000000

/* period types */

enum MPeriodEnum { mpNONE = 0, mpDay, mpWeek, mpInfinity };

enum { jtNONE = 0, jtWiki, jtTrace, jtXML };

/* trace flags */

enum MTraceFlagEnum {
  tfNONE,
  tfFixCorruption };

/* client keyword values */

enum MClientKeywordEnum { 
  mckNONE = 0,
  mckStatusCode, 
  mckArgs, 
  mckAuth, 
  mckCommand, 
  mckData, 
  mckClient,
  mckTimeStamp,
  mckCheckSum };

#define MAX_MRESOURCETYPE 6

enum MResourceTypeEnum {
  mrNONE = 0,
  mrNode,
  mrProc,
  mrMem,
  mrSwap,
  mrDisk };


/* object access types */

#define MAX_MATTRTYPE 20
#define MMAX_ATTRTYPE 20

enum MAttrEnum {
  maNONE = 0,
  maJob,
  maNode,
  maRes,
  maUser,
  maGroup,
  maAcct,
  maPartition,
  maScheduler,
  maSystem,
  maClass,
  maQOS,
  maJFeature,
  maDuration,
  maTask,
  maProc,
  maFrame,
  maQueue,
  maCluster };


typedef struct {
  FILE          *logfp;
  int            Threshold;
  unsigned long  FacilityList;
  } mlog_t;

#ifndef DBG   

enum { dbLOGFILE, dbNOTIFY, dbSYSLOG };

#define DBG(X,F) if ((mlog.Threshold >= X) && ((mlog.FacilityList) & F))
#define MDB(X,F) if ((mlog.Threshold >= X) && ((mlog.FacilityList) & F))

#ifdef __MTEST
#define DPrint printf
#endif /* __MTEST */

#define MDEBUG(X) if (mlog.Threshold >= X) 

/* log facility values */

#define fCORE     (1 << 0)
#define fSCHED    (1 << 1)
#define fSOCK     (1 << 2)
#define fUI       (1 << 3)
#define fLL       (1 << 4)
#define fSDR      (1 << 5)
#define fCONFIG   (1 << 6)
#define fSTAT     (1 << 7)
#define fSIM      (1 << 8)
#define fSTRUCT   (1 << 9)
#define fFS       (1 << 10)
#define fCKPT     (1 << 11)
#define fAM       (1 << 12)
#define fRM       (1 << 13)
#define fPBS      (1 << 14)
#define fWIKI     (1 << 15)
#define fSGE      (1 << 16)
#define fS3       (1 << 17)
#define fLSF      (1 << 18)
#define fNAT      (1 << 19)

#define fALL 0xffffffff
#endif /* DBG */

#ifndef MIN
#  define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#  define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

#define RSPMARKER        "RSP="

/* comparision types */

enum MCompEnum { 
  mcmpNONE = 0,
  mcmpLT,
  mcmpLE,
  mcmpEQ,
  mcmpGE,
  mcmpGT,
  mcmpNE,
  mcmpEQ2,
  mcmpNE2,
  mcmpSSUB,
  mcmpSNE,
  mcmpSEQ
  };

enum {
  rmaNONE = 0,
  rmaCheckSum,
  rmaPKI,
  rmaSecurePort
  };

typedef struct
  {
  int count;
  } mnuml_t;
 
typedef struct {
  int Nodes;
  int Memory;
  unsigned long Workload;
  unsigned long InitialWorkload;
  } mrclass_t;

#define MAX_MCLASS  16            
#define MMAX_CLASS  16

#define MAX_MGRES    4    
 
typedef struct {
  int     Procs;
  int     Mem;    /* (in MB) */
  int     Swap;   /* (in MB) */
  int     Disk;   /* (in MB) */

  mnuml_t PSlot[MAX_MCLASS];
  mnuml_t GRes[MAX_MGRES];
  } mcres_t;

/* AM Interface */
 
typedef struct
  {
  const char *Attr;
  char        Value[MAX_MLINE];
  char       *Cmp;
  } mamolist_t;

#define MAX_MAMO  12
 
enum MAMOTypes {
  mamoNONE = 0,
  mamoAccount,
  mamoClass,
  mamoDuration,
  mamoJobID,
  mamoJobType,
  mamoMachine,
  mamoNodeType,
  mamoProcCount,
  mamoProcRate,
  mamoQOS,
  mamoUser
  };

enum {
  pParamNONE = 0,
  mcoBFChunkDuration,
  mcoBFChunkSize,
  mcoComputeHosts,            
  pSchedConfigFile,      
  pSchedMode,
  pSchedStepCount,
  pMServerHomeDir,
  pServerName,
  pSchedLogDir,
  pSchedLogFile,
  pLogLevel,
  pLogFacility,
  pRMPollInterval,
  pDisplayFlags,
  pMaxSleepIteration,
  pSchedToolsDir,
  pSchedLockFile,
  pMachineConfigFile,
  pCheckPointFile,
  pCheckPointInterval,
  pCheckPointExpirationTime,
  pClientCfg,
  pAdminEInterval,
  pAdminEAction,
  pLogFileMaxSize,
  pLogFileRollDepth,
  pServerHost,
  pServerPort,
  pAMType,
  pAMProtocol,
  pAMChargePolicy,
  pAMHost,
  pAMPort, 
  pAMFlushInterval,
  pAMFallbackAccount,
  pAMAppendMachineName,
  pAMDeferOnJobFailure,
  pAMTimeout,
  pDefaultDomain,
  pDefaultQMHost,
  pDefaultClassList,
  pRMPort,
  pRMHost,
  pRMNMPort,
  pRMName,
  pRMType,
  pRMAuthType,
  pRMTimeout,
  pRMConfigFile,
  pRMLocalDiskFS,
  mcoRMEPort,
  pNAPolicy,
  pResourceCollectionPolicy,
  pRemoteStartCommand,
  pResourceCommand,
  pResourcePort,
  pResourceSampleInterval,
  pResourceReportWait,
  pResourceDataFile,
  pResourceLockFile,
  mcoAdminUsers,
  mcoAdmin1Users,
  mcoAdmin2Users,
  mcoAdmin3Users,
  mcoAdmin4Users,
  mcoAdminHosts,
  mcoJobFBAction,
  mcoMailAction,
  mcoDeferTime,
  pDeferCount,
  pDeferStartCount,
  pJobPurgeTime,
  pNodePurgeTime,
  pAPIFailureThreshhold,
  pNodeSyncDeadline,
  pJobSyncDeadline, 
  pJobMaxOverrun,
  pSimWCScalingPercent,           /* sim params */
  pSimInitialQueueDepth,
  pSimWCAccuracy,
  pSimWCAccuracyChange,
  pSimJobSubmissionPolicy,
  pSimNCPolicy,
  pUseMachineSpeed,
  pUseSystemQueueTime,
  pNodeAvailPolicy,
  mcoResourceLimitPolicy,
  mcoAdminMinSTime,
  pJobPrioAccrualPolicy,
  pBFPriorityPolicy,
  pUseLocalMachinePriority,
  pSimNodeCount,
  pSimScaleJobRunTime,
  pSimCheckpointInterval,
  pWorkloadTraceFile,
  pResourceTraceFile,
  pStopIteration,
  pExitIteration,
  pSimAutoShutdown,
  pSimFlags,
  pSimIgnoreJobFlags,
  pSimDefaultJobFlags,
  pSimStartTime,
  pNodeMaxLoad,
  pNodeLoadPolicy,
  pNodeSetPolicy,
  pNodeSetAttribute,
  pNodeSetDelay,
  pNodeSetList,
  pNodeSetTolerance,
  pNodeSetPriorityType,
  pNodeUntrackedProcFactor,
  pNodeDownStateDelayTime,
  mcoAllocLocalityPolicy,
  mcoUseJobRegEx,
  mcoEnableMultiNodeJobs,
  mcoEnableMultiReqJobs,
  pCommunicationType,
  pIntraFrameCost, 
  pInterFrameCost,
  pComRate,
  pRandomizeJobSize,
  pBFPolicy,              /* policy params */
  pBFDepth,
  pBFType,
  pBFProcFactor,
  pBFMaxSchedules,
  pSRName,
  pSRPeriod,
  pSRDays,
  pSRStartTime,
  pSREndTime,
  pSRWStartTime,
  pSRWEndTime,
  pSRDepth,
  pSRResources,
  pSRTPN,
  pSRTaskCount,
  pSRMaxTime,
  pSRIdleTime,
  pSRMinLoad,
  pSRTimeLogic,
  pSRUserList,
  pSRGroupList,
  pSRAccountList,
  pSRQOSList,
  pSRClassList,
  pSRPartition,
  pSRHostList,
  pSRChargeAccount,
  pSRAccess,
  pSRPriority,
  pSRFlags,
  pSRFeatures,
  pSRMinTasks,
  pSRMaxTasks,
  pMaxJobPerUserCount,
  pHMaxJobPerUserCount,
  pMaxJobPerUserPolicy, 
  pMaxProcPerUserCount,
  pHMaxProcPerUserCount,
  pMaxProcPerUserPolicy,
  pMaxNodePerUserCount,
  pHMaxNodePerUserCount,
  pMaxNodePerUserPolicy,
  pMaxPSPerUserCount,
  pHMaxPSPerUserCount,
  pMaxPSPerUserPolicy,
  pMaxJobQueuedPerUserCount,
  pHMaxJobQueuedPerUserCount,
  pMaxJobQueuedPerUserPolicy,
  pMaxPEPerUserCount,
  pHMaxPEPerUserCount,
  pMaxPEPerUserPolicy,
  pMaxJobPerGroupCount,
  pHMaxJobPerGroupCount,
  pMaxJobPerGroupPolicy,
  pMaxNodePerGroupCount,
  pHMaxNodePerGroupCount,
  pMaxNodePerGroupPolicy,
  pMaxPSPerGroupCount,
  pHMaxPSPerGroupCount,
  pMaxPSPerGroupPolicy,
  pMaxJobQueuedPerGroupCount,
  pHMaxJobQueuedPerGroupCount,
  pMaxJobQueuedPerGroupPolicy,
  pMaxJobPerAccountPolicy,
  pMaxJobPerAccountCount,
  pHMaxJobPerAccountCount,
  pMaxNodePerAccountPolicy,
  pMaxNodePerAccountCount,
  pHMaxNodePerAccountCount,
  pMaxPSPerAccountPolicy,
  pMaxPSPerAccountCount,
  pHMaxPSPerAccountCount,
  pMaxJobQueuedPerAccountPolicy,
  pMaxJobQueuedPerAccountCount,
  pHMaxJobQueuedPerAccountCount,
  pMaxMetaTasks, 
  pSystemMaxJobProc,
  pSystemMaxJobTime,
  pSystemMaxJobPS,
  pMaxJobStartTime,
  pNodeAllocationPolicy,
  pTaskDistributionPolicy,
  pBFMetric,
  pJobSizePolicy,
  pJobNodeMatch,
  mcoJobAttrPrioF,
  pResPolicy,
  pResRetryTime,
  pResThresholdValue,
  pResThresholdType,
  pReservationDepth,
  pServWeight,  /* NOTE: parameters must sync with prio comp/subcomp enum */
  pTargWeight,
  pCredWeight,
  pAttrWeight,
  pFSWeight,
  pResWeight,
  pUsageWeight,
  pSQTWeight,
  pSXFWeight,
  pSSPVWeight,
  pSBPWeight,
  pTQTWeight,
  pTXFWeight,
  pCUWeight,
  pCGWeight,
  pCAWeight,
  pCQWeight,
  pCCWeight,
  pFUWeight,
  pFGWeight,
  pFAWeight,
  pFQWeight,
  pFCWeight,
  pAJobAttrWeight,
  pAJobStateWeight,
  pRNodeWeight,
  pRProcWeight,
  pRMemWeight,
  pRSwapWeight,
  pRDiskWeight, 
  pRPSWeight,
  pRPEWeight,
  pRWallTimeWeight,
  pRUProcWeight,
  pRUJobWeight,
  pUConsWeight,
  pURemWeight,
  pUPerCWeight,
  pXFMinWCLimit,
  pServCap,  /* NOTE: parameters must sync with prio comp/subcomp enum */
  pTargCap,
  pCredCap,
  pAttrCap,
  pFSCap,
  pResCap,
  pUsageCap,
  pSQTCap,
  pSXFCap,
  pSSPVCap,
  pSBPCap,
  pTQTCap,
  pTXFCap,
  pCUCap,
  pCGCap,
  pCACap,
  pCQCap,
  pCCCap,
  pFUCap,
  pFGCap,
  pFACap,
  pFQCap,
  pFCCap,
  pAJobAttrCap,
  pAJobStateCap,
  pRNodeCap,
  pRProcCap,
  pRMemCap,
  pRSwapCap,
  pRDiskCap,
  pRPSCap,
  pRPECap,
  pRWallTimeCap,
  pUConsCap,
  pURemCap, 
  pUPerCCap,
  pQOSName,
  pQOSXFWeight,
  pQOSTargetXF,
  pQOSQTWeight,
  pQOSTargetQT,
  pQOSPriority,
  pQOSFlags,
  pResQOSList,
  pFSPolicy,
  pFSConfigFile,
  pFSInterval,
  pFSDepth,
  pFSDecay,
  pFSEnforcement,
  pStatDir,                     /* Stat params */
  pPlotMinTime,
  pPlotMaxTime,
  pPlotTimeScale,
  pPlotMinNode,
  pPlotMaxNode,
  pPlotNodeScale,
  pMonitoredJob,
  pMonitoredNode,
  pMonitoredRes,
  pMonitoredFunction,
  pResDepth,
  pNodePollFrequency,
  pProcSpeedFeatureHeader,
  pNodeTypeFeatureHeader,
  pPartitionFeatureHeader,
  pClientTimeout,
  pMCSocketProtocol,
  pDefaultAccountName,
  pNodeCPUOverCommitFactor,
  pNodeMemOverCommitFactor,
  pMaxJobPerIteration,
  pMinDispatchTime,
  mcoUseSyslog,
  pResCtlPolicy, 
  pPreemptPolicy,
  pJobAggregationTime,
  pUseCPUTime,
  pParIgnQList,
  mcoRejectNegPrioJobs,
  mcoEnableNegJobPriority,
  mcoWCViolAction,
  mcoResLimitPolicy,
  mcoDirectoryServer,
  mcoEventServer,
  mcoTimePolicy,
  pOLDFSUWeight,        /* params for backwards compatibility */
  pOLDFSGWeight,
  pOLDFSAWeight,
  pOLDFSQWeight,
  pOLDFSCWeight,
  pOLDServWeight,
  pOLDQTWeight,
  pOLDXFWeight,
  pOLDUFSWeight,
  pOLDGFSWeight,
  pOLDAFSWeight,
  pOLDDirectSpecWeight,
  pOLDBankServer,
  pOLDRMServer
  };

#endif /* __M_COMMON_H__ */

