/*
*/
        
#ifndef __M_H__
#define __M_H__

#define MSCHED_VERSION "3.2.6p14"
#define MSCHED_NAME    "Maui"
#define MSCHED_SNAME   "maui"

#ifndef __M32COMPAT
#define __M32COMPAT
#endif /* __M32COMPAT */

#ifdef __LOCAL
#  include "msched-local.h"
#endif /* __LOCAL */

#ifdef __MTHREADS
#  include <pthread.h>
#endif

#include "msched-common.h"

#ifdef LIBGEN
#  include <libgen.h>
#else /* LIBGEN */

#ifndef __NT
#ifdef PCRE
#  include <pcreposix.h>
#else /* PCRE */
#  include <regex.h>
#endif /* PCRE */
#endif /* __NT */

#endif /* LIBGEN */

#define DEFAULT_LLBINPATH "/usr/lpp/LoadL/full/bin"

#if defined(__MLL) || defined(__MLL2) || defined(__MLL31)
# define __LLAPI
# include "llapi.h"
#else /* __MLL || __MLL2 || __MLL31 */
# include "msched-llapi.h"
#endif /* __MLL || __MLL2  || __MLL31 */

#if defined(__MLL2) || defined(__MLL31) || defined(__O2K)
#ifndef __SMP
#define __SMP
#endif /* __SMP */
#endif /* __MLL2 || __MLL31 || __O2K */

#define GetLimit(PL,P,PS,PH,Limit)              \
  switch (PL)                                   \
    {                                           \
    case ptSOFT:                                \
      *Limit = ((P == ptON) && (PS > 0)) ?  PS : MAX_VAL; \
      break;                                    \
    case ptHARD:                                \
      *Limit = ((P == ptON) && (PH > 0)) ?  PH : MAX_VAL; \
      break;                                    \
    case ptOFF:                                 \
    default:                                    \
      *Limit = MAX_VAL;                         \
      break;                                    \
    } 

#define MTRAPJOB(TJob,TFunction)                   \
  if (strstr(MSched.MonitoredJob,TJob->Name) &&    \
    ((MSched.MonitoredFunction[0] == '\0') ||      \
      strstr(MSched.MonitoredFunction,TFunction))) \
    MJobTrap(TJob); 

#define MTRAPNODE(TNode,TFunction)                 \
  if (strstr(MSched.MonitoredNode,TNode->Name) &&  \
    ((MSched.MonitoredFunction[0] == '\0') ||      \
      strstr(MSched.MonitoredFunction,TFunction))) \
    MNodeTrap(TNode);

#define MTRAPRES(TRes,TFunction)                   \
  if (strstr(MSched.MonitoredRes,TRes->Name) &&    \
    ((MSched.MonitoredFunction[0] == '\0') ||      \
      strstr(MSched.MonitoredFunction,TFunction))) \
    MResTrap(TRes);
 
/* define trace markers */

#define TRACE_WORKLOAD_VERSION_MARKER          "VERSION"
#define TRACE_RESOURCE_VERSION_MARKER          "VERSION"

#define DEFAULT_WORKLOAD_TRACE_VERSION         230
#define DEFAULT_RESOURCE_TRACE_VERSION         230

#define MCKPT_VERSION                         "322"
#define MCKPT_SVERSIONLIST                    "322"
 
/* temporary fix to handle LL API bug */

#ifndef LL_NOTQUEUED
# define LL_NOTQUEUED                     10
#endif /* LL_NOTQUEUED */

#define DEFAULT_PBSQSUBPATHNAME                "/usr/local/bin/qsub"
 
/* default scheduler values */

#define MASTER_CONFIGFILE                      "/etc/maui.cfg"

#define GLOBAL_MPARNAME                        "ALL"
#define DEFAULT_MPARNAME                       "DEFAULT"
#define DEFAULT_CLASSLIST                      "[DEFAULT:1]"

#define DEFAULT_RES_DEPTH                    24
#define MAX_MRES_DEPTH                      512
#define MMAX_RES_DEPTH                      512

#define DEFAULT_CRDURATION                   30

#define DEFAULT_MLOGLEVEL                     0
#define DEFAULT_MLOGFACILITY                   fALL

#define MDEF_NODETYPE                       "DEFAULT"
#define MDEF_NODEDOWNSTATEDELAYTIME         3600
#define MDEF_NODESETDELAY                   0
#define MDEF_NODESETATTRIBUTE               mrstNONE
#define MDEF_NODESETSELECTIONTYPE           mrssONEOF
#define MDEF_NODESETPRIORITYTYPE            mrspMinLoss
#define MDEF_NODESETTOLERANCE               0.0
#define DEFAULT_RMPOLLINTERVAL                 60
#define DEFAULT_MAXSLEEPITERATION              1
#define DEFAULT_SchedSTEPCOUNT                 60
#define DEFAULT_MACHINECONFIGFILE              ""          

/* server specific config variables */

#define DEFAULT_SchedCONFIGFILE                  "maui.cfg"
#define DEFAULT_SchedLOCKFILE                    "maui.pid"
#define DEFAULT_CHECKPOINTFILE                   "maui.ck"
#define DEFAULT_MLOGFILE                         "maui.log"   
#define MSCHED_ENVDBGVAR                         "MAUIDEBUG"
#define MSCHED_ENVCRASHVAR                       "MAUICRASHMODE"
#define MSCHED_ENVHOMEVAR                        "MAUIHOMEDIR"
#define MSCHED_ENVLOGSTDERRVAR                   "MAUILOGSTDERR"
#define MSCHED_ENVPARVAR                         "MAUIPARTITION"
#define MSCHED_ENVSMPVAR                         "MAUISMP"
#define MSCHED_ENVTESTVAR                        "MAUITEST"
#define MSCHED_ENVCKTESTVAR                      "MAUICKTEST"
#define MSCHED_ENVAMTESTVAR                      "MAUIAMTEST"

#define MSCHED_MSGINFOHEADER                     "MAUI_INFO"
#define MSCHED_MSGERRHEADER                      "MAUI_ERROR"
      
#define DEFAULT_CHECKPOINTINTERVAL          300
#define DEFAULT_CHECKPOINTEXPIRATIONTIME 300000

#define DEFAULT_MRMNETINTERFACE                mnetEN0

#define DEFAULT_MLOGDIR                        "log/"
#define DEFAULT_SchedTOOLSDIR                  "tools/"
#define DEFAULT_MADMINEACTIONPROGRAM           ""
#define DEFAULT_MLOGFILEMAXSIZE         1000000  
#define DEFAULT_MLOGFILEROLLDEPTH             1

#define DEFAULT_CLIENTTIMEOUT          30000000
#define MDEF_CLIENTTIMEOUT             30000000

#define DEFAULT_DOMAIN                         ""

#define DEFAULT_PARTITIONMODE                  ptOFF
#define GLOBAL_PARTITIONLIST                   0xffffff
#define DEFAULT_MPARLIST                       GLOBAL_PARTITIONLIST
#define DEFAULT_PARTITION                      0

#define DEFAULT_RESOURCECOLLECTIONPOLICY       ptOFF
#define DEFAULT_REMOTESTARTCOMMAND             "/usr/bin/rsh"   

#define DEFAULT_RESOURCECOMMAND                "resd"
#define DEFAULT_RESOURCEPORT              40561
#define DEFAULT_RESOURCESAMPLEINTERVAL      300
#define DEFAULT_RESOURCEREPORTWAIT           15
#define DEFAULT_RESOURCEDATAFILE               "/tmp/resd.data"
#define DEFAULT_RESOURCELOCKFILE               "/tmp/resd.lock"

#define DEFAULT_SchedADMIN                     "loadl"
#define DEFAULT_MJOBFBACTIONPROGRAM            ""
#define DEFAULT_DEFERTIME                  3600
#define MAX_RMFAILCOUNT                       3
#define MMAX_RMFAILURE                        3

#define DEFAULT_DEFERCOUNT                   24
#define DEFAULT_DEFERSTARTCOUNT               1
#define DEFAULT_JOBPURGETIME                  0
#define MDEF_NODEPURGETIME                     MAX_MTIME
#define DEFAULT_APIFAILURETHRESHHOLD          6
#define MDEF_NODESYNCDEADLINE               600
#define DEFAULT_JOBSYNCDEADLINE             600
#define DEFAULT_MAXJOBOVERRUN               600                 /* Allow Job to Exceed WCLimit By 10 Minutes */

#define DEFAULT_SRDEPTH                        2
#define DEFAULT_SRTIMELOGIC                    tlOR
#define DEFAULT_SRPRIORITY                     0
#define DEFAULT_SRFLAGS                        0
#define DEFAULT_SRPERIOD                       mpDay

#define DEFAULT_USEMACHINESPEED                FALSE
#define DEFAULT_USESYSTEMQUEUETIME             ptOFF
#define DEFAULT_JOBPRIOACCRUALPOLICY           jpapQueuePolicy
#define DEFAULT_RESOURCEAVAILPOLICY            mrapCombined
#define MDEF_NODELOADPOLICY                    nlpAdjustState
#define MDEF_NODEUNTRACKEDRESOURCEFACTOR       1.2    
#define MDEF_NODEUNTRACKEDPROCFACTOR           1.2
#define DEFAULT_USELOCALMACHINEPRIORITY        ptOFF

/* default policy values */

#define DEFAULT_CBMASK                              (1 << cbResStart) | (1 << cbResCancel)
#define DEFAULT_BACKFILLPOLICY                      bfFIRSTFIT
#define DEFAULT_BACKFILLDEPTH                      0
#define DEFAULT_BACKFILLNODEFACTOR                 0
#define DEFAULT_MAXBACKFILLSCHEDULES           10000

#define DEFAULT_SRSTARTTIME                        0
#define DEFAULT_SRENDTIME                          0
#define DEFAULT_SRDAYS                           255
#define DEFAULT_SRPROCS                           -1
#define DEFAULT_SRTPN                              0
#define MDEF_MINSWAP                               10   /* in MB */
#define DEFAULT_MJOBNODEMATCH                      0

/* system defaults */

#define MDEF_SYSJOBWCLIMIT                   8639999
#define DEFAULT_MAXMETATASKS                        0
#define DEFAULT_MAXJOBSTARTTIME                     MAX_MTIME 
#define DEFAULT_TASKDISTRIBUTIONPOLICY              mtdDefault
#define DEFAULT_MBFMETRIC                           mbfmProcs
#define DEFAULT_JOBSIZEPOLICY                       jsMinProc
#define DEFAULT_RESERVATIONMODE                     resCurrentHighest
#define DEFAULT_RESERVATIONRETRYTIME                0
#define DEFAULT_RESERVATIONTHRESHOLDTYPE            rttNone
#define DEFAULT_RESERVATIONTHRESHOLDVALUE           0
#define DEFAULT_RESERVATIONDEPTH                    1
#define DEFAULT_RESERVATIONQOSLIST                  (mqos_t *)MAX_MQOS


#define MDEF_SYSQAL                                 0
#define MDEF_SYSPAL                                 0
#define MDEF_SYSQDEF                                0
#define MDEF_SYSPDEF                                0

/* default Priority/FairShare values */

#define MDEF_FSFLAGS                               0

#define DEFAULT_DIRECTSPECWEIGHT                   1
#define DEFAULT_UWEIGHT                            0
#define DEFAULT_GWEIGHT                            0
#define DEFAULT_AWEIGHT                            0
#define DEFAULT_CLASSWEIGHT                        0
#define DEFAULT_QOSWEIGHT                          0

#define DEFAULT_SERVICEWEIGHT                      1
#define DEFAULT_QUEUETIMEWEIGHT                    1
#define DEFAULT_XFACTORWEIGHT                      0
#define DEFAULT_XFMINWCLIMIT                     120

#define DEFAULT_FSWEIGHT                           1

#define DEFAULT_UFSWEIGHT                          0
#define DEFAULT_GFSWEIGHT                          0
#define DEFAULT_AFSWEIGHT                          0
#define DEFAULT_QFSWEIGHT                          0
#define DEFAULT_CFSWEIGHT                          0

#define DEFAULT_TARGETWEIGHT                       1
#define DEFAULT_BYPASSWEIGHT                       0

#define DEFAULT_RESOURCEWEIGHT                     1
#define MDEF_NODEWEIGHT                            0
#define DEFAULT_PROCESSORWEIGHT                    0
#define DEFAULT_MEMORYWEIGHT                       0
#define DEFAULT_DISKWEIGHT                         0
#define DEFAULT_SWAPWEIGHT                         0
#define DEFAULT_PEWEIGHT                           0

#define DEFAULT_QOSXFWEIGHT                        0
#define DEFAULT_QOSQTWEIGHT                        0

#define DEFAULT_TARGETXF                         0.0
#define DEFAULT_TARGETQT                           0

#define DEFAULT_MQOSFLAGS                          0
#define DEFAULT_MQOSPRIORITY                       0

#define DEFAULT_FSPOLICY                            fspNONE
#define DEFAULT_FSINTERVAL                     43200
#define DEFAULT_FSDEPTH                            8
#define DEFAULT_FSDECAY                          1.0

/* grid statistics defaults */

#define DEFAULT_MAXTIME                       245760
#define DEFAULT_MINTIME                          120
#define DEFAULT_TIMESCALE                         11

#define DEFAULT_MAXNODE                          512
#define DEFAULT_MINNODE                            1
#define MDEF_NODESCALE                             9

#define DEFAULT_ACCURACYSCALE                     10

#define DEFAULT_TRACEFLAGS                         0

/* default stats values */

#define DEFAULT_MSTATDIR                            "stats/"

#ifndef EMPTY
#define EMPTY                0 
#endif  /* EMPTY */

enum { 
  mfsactNONE = 0,
  mfsactCalc,
  mfsactRotate,
  mfsactWrite };

/* list of job mode values */

#define MAX_MHBUF           768

#define MAX_VAL      2140000000

#define MMAX_JOB           4096
#define MAX_MJOB       MMAX_JOB


#define MAX_MJOB_TRACE     4096

#define MAX_TASK_REQUESTS    32
#define MAX_REQ_TYPE         16
#define MAX_MREQ_PER_JOB      4
#define MMAX_REQ_PER_JOB      4

#define MAX_WORD             32
#define MAX_MLIST            16
#define MAX_MPOOL             7
#define MAX_STATISTICS       16   /* maximum number of statistics to update */
#define MAX_ACCT_ACC          8 

#define MAX_MFRAMECOUNT      64
#define MAX_MSLOTPERFRAME    32
#define MAX_MSRES           128
#define MAX_SRES_DEPTH       64
#define MAX_MRANGE          256
#define MAX_PRIO_VAL 1000000000
#define MAX_ATTEMPTS        500
#define MAX_MADMINUSERS      64
#define MAX_MADMINHOSTS      64
#define MAX_PARMVALS        256
#define MAX_MGRIDTIMES       32
#define MAX_MGRIDSIZES       32
#define MAX_ACCURACY         10
#define MAX_SCALE           128
#define MAX_MWINDOW          16   /* maximum BF windows evaluated */
#define MAX_MRCLASS          16   /* maximum resource classes */
#define MAX_MRM               6
#define MMAX_RM               6
#define MAX_MAM               4
#define MMAX_AM               4
#define MAX_MFRAME           48
#define MAX_SOCKETWAIT  5000000

#define MIN_SWAP             10
#define MIN_OS_SWAP_OVERHEAD 25
#define MIN_OS_CPU_OVERHEAD    0.5
#define MAX_OS_CPU_OVERHEAD    0.0

/* scheduler actions */

enum MSchedActionEnum { 
  mactNONE = 0,
  mactAdminEvent,
  mactJobFB,
  mactMail };

/* flags bit field */
 
enum { fAdmin1 = 0, fAdmin2, fAdmin3, fAdmin4 };

enum ResCtlPolicyEnum {
  mrcpNONE = 0,
  mrcpAdmin,
  mrcpAny };

#define FRAMEMARKER            "FRAME"
#define ATTRMARKER             "ATTR"
#define PARTITIONMARKER        "PM_"
#define QOSDEFAULTMARKER       "QDEF="
#define QOSLISTMARKER          "QLIST="
#define PARTDEFAULTMARKER      "PDEF="
#define PARTLISTMARKER         "PLIST="
#define MASTERJOBMARKER        "MASTERJOB="
#define FSFLAGMARKER           "JOBFLAGS="
#define FSTARGETMARKER         "FSTARGET="
#define PRIORITYMARKER         "PRIORITY="

enum { fspNONE, fspDPS, fspDPES, fspUPS };

enum { jpapNONE = 0, jpapAlways, jpapQueuePolicy, jpapFullPolicy };

enum { nlpNONE = 0, nlpAdjustState, nlpAdjustProcs };

enum { nmNONE = 0, nmExactNodeMatch, nmExactProcMatch };

enum { 
  stNONE = 0, 
  stNormal, 
  stBestEffort, 
  stForce };

/* display flag types */

enum { dfNONE = 0, dfNodeCentric };

/* QOS access list types */

enum { qalOR = 0, qalAND, qalONLY };

enum { dstatWCA = 0 };

/* credential type */

enum { ctUGA = 0, ctACL };
 
enum { tlAND = 0, tlOR };

/* reservation time constraints */

enum { 
  rtcNone = 0, 
  rtcStartOnly, 
  rtcStartLatest, 
  rtcStartEarliest, 
  rtcFeasible };

/* reservation space constraints */

enum { rscNone = 0, rscNodesOnly };

enum MREEnum { mreNONE = 0, mreStart, mreEnd };

/* client modes */

enum MCModeEnum { 
  mcmNONE = 0, 
  mcmRelative, 
  mcmSummary, 
  mcmGrep, 
  mcmVerbose, 
  mcmForce, 
  mcmParse, 
  mcmForce2,
  mcmComplete,
  mcmBlock,
  mcmNonBlock,
  mcmOverlap,
  mcmClear,
  mcmFuture,
  mcmXML };

/* statistics modes */

enum MProfModeEnum {
  mNONE = 0,
  mUseRunTime,
  mSystemQueue,
  mMatrix,
  mTrueXFactor,
  mUseRemoved
  };

enum { resmInclS = 0, resmExclS, resmExclP, resmIgn };


/* distribution types */

enum {
  distRR = 0,
  distBF,
  distCustom
  };
 
/* log facilities */

enum MLogFacilityEnum {
  dfCORE = 0,
  dfSCHED,
  dfSOCK,
  dfUI,
  dfLL,
  dfSDR,
  dfCONFIG,
  dfSTAT,
  dfSIM,
  dfSTRUCT,
  dfFS,
  dfCKPT,
  dfBANK,
  dfRM,
  dfPBS,
  dfWIKI,
  dfSGE,
  dfS3,
  dfLSF,
  dfNAT,
  dfALL };

/* node problems */

enum { 
  probLocalDisk = 0,
  probMemory,
  probEthernet,
  probHPS_IP,
  probHPS_User,
  probSwap
  };  

/* reservation flags */

enum { rfLockTime = 0 };
 
/* status code values */

enum { scFAILURE = 0, scSUCCESS };

/* sched command modes */

enum { 
  msctlKill = 0, 
  msctlResume, 
  msctlReconfig, 
  msctlStep, 
  msctlStop, 
  msctlNodeTable, 
  msctlSubmit,
  msctlList,
  msctlModify,
  msctlFailure,
  msctlInit };

/* FairShare modes */

enum { 
  mfstNONE = 0, 
  mfstTarget, 
  mfstFloor, 
  mfstCeiling, 
  mfstCapRel,
  mfstCapAbs };

/* scheduler modes */

enum { 
  msmNONE = 0, 
  msmNormal, 
  msmProfile,
  msmSim, 
  msmSingleStep, 
  msmTest };

enum SDType {
  msdNONE = 0,
  msdApplication,
  msdResource };

/* Statistics Weighting Modes */

enum { stwJob, stwNode, stwRun, stwRequest, stwPSRun, stwPSRequest };

/* Policy enforcement types */

enum { ptDEFAULT = 0, ptOFF, ptON, ptSOFT, ptHARD, ptEXEMPT, ptQUEUE };

/* Back Fill Metric */

enum {
  mbfmNONE = 0, 
  mbfmProcs,
  mbfmNodes,
  mbfmPS,
  mbfmWalltime };

/* Job Size Approaches */

enum { jsMinProc = 0, jsMaxProc, jsNDMaxProc };

/* BackFill Types */

enum { 
  bfNONE = 0, 
  bfFIRSTFIT, 
  bfBESTFIT, 
  bfGREEDY, 
  bfBESTTIMEFIT, 
  bfLAZY,
  bfPREEMPT };

/* reservation policies */

enum { resDefault = 0, resHighest, resCurrentHighest, resNever };
enum { rttNone = 0, rttBypass, rttQueueTime, rttXFactor };


/* task distribution policies */

enum {
  mtdNONE = 0,
  mtdDefault,
  mtdLocal,
  mtdArbGeo,
  mtdPack,
  mtdRoundRobin };


/* stat types */

enum { 
  stAvgXFactor = 0,
  stMaxXFactor,
  stAvgQTime,
  stAvgBypass,
  stMaxBypass,
  stJobCount,
  stPSRequest,
  stPSRun,
  stWCAccuracy,
  stBFCount,
  stBFPSRun,
  stJobEfficiency,
  stQOSDelivered };

/* BNF objects */

enum {
  bobjNONE = 0,
  bobjJob,
  bobjNode,
  bobjReservation,
  bobjUser,
  bobjGroup,
  bobjAccount,
  bobjPartition,
  bobjScheduler,
  bobjSystem,
  bobjClass,
  bobjQOS,
  bobjFrame,
  bobjQueue
  };

enum {
  mcsNONE = 0,
  mcsLimits,
  mcsFS,
  mcsGeneral,
  mcsUsage };

/* job types */

enum { jobSerial = 0, jobParallel, jobPVM };

/* experienced values */

/* NOTE:  sync w/MAX_MATTR, MAttrType */

enum MExpAttrEnum { 
  eFeature = 0,
  ePartition,
  eQOS,
  eClass,
  eNetwork,
  eOpsys,
  eArch,
  eNodeState,
  eJobState,
  eSysAttr,
  eGRes,
  eJFeature };

enum MExpAttrEnum2 {
  meNFeature = 0,
  mePartition,
  meQOS,
  meClass,
  meNetwork,
  meOpsys,
  meArch,
  meNodeState,
  meJobState,
  meSysAttr,
  meGRes,
  meJFeature };

/* requirements keywords */

enum { 
  kFeature = 0,
  kNetwork,
  kOpsys,
  kArch,
  kMemory,
  kDisk,
  kPool,
  kMachine };

/* net interface types */

#define MAX_MNETTYPE  4

enum { mnetNONE = 0, mnetEN0, mnetEN1, mnetCSS0 };

/* job states */
 
enum MJobStateEnum {
  mjsNONE = 0,
  mjsIdle,
  mjsStarting,
  mjsRunning,
  mjsRemoved,
  mjsCompleted,
  mjsHold,
  mjsDeferred,
  mjsSubmitErr,
  mjsVacated,
  mjsNotRun,
  mjsNotQueued,
  mjsUnknown,
  mjsBatchHold,
  mjsUserHold,
  mjsSystemHold,
  mjsStaging,
  mjsStaged,
  mjsSuspended,
  mjsLost };

/* reasons for policy rejection */

enum MResourceAvailabilityTypes {
  mrapNONE = 0,
  mrapCombined,
  mrapDedicated,
  mrapUtilized };
 
enum {
  prNONE = 0,
  prMaxJobPerUserPolicy,
  prMaxProcPerUserPolicy,
  prMaxNodePerUserPolicy,
  prMaxPSPerUserPolicy,
  prMaxJobQueuedPerUserPolicy,
  prMaxPEPerUserPolicy,
  prMaxJobPerGroupPolicy,
  prMaxNodePerGroupPolicy,
  prMaxPSPerGroupPolicy,
  prMaxJobQueuedPerGroupPolicy,
  prMaxJobPerAccountPolicy,
  prMaxNodePerAccountPolicy,
  prMaxPSPerAccountPolicy,
  prMaxJobQueuedPerAccountPolicy,
  prSystemMaxJobProc,
  prSystemMaxJobTime,
  prSystemMaxJobPS,
  prUnknownUser,
  prUnknownGroup,
  prUnknownAccount };


/* sync with MClientCmd */

enum MClientCmdEnum {
  mccNONE = 0,
  mccBal,
  mccDiagnose,  
  mccGridCtl,    
  mccJobCtl,
  mccNodeCtl,
  mccResCtl,
  mccSchedCtl,
  mccShow,
  mccStat };
 

/* node system attributes */

enum { 
  attrNONE = 0,
  attrJMD, 
  attrBatch, 
  attrInteractive,
  attrLogin,
  attrHPSS,
  attrPIOFS,
  attrSystem,
  attrGateway,
  attrEPrimary
  };

/* frame types */

enum { frTHIN = 0, frWIDE, frHIGH, frLOW };

/* communication simulation types */

enum { comRoundRobin = 0, comBroadcast, comTree, comMasterSlave };

/* Resource Daemon Commands */

enum { resdInitialize = 0, resdSend, resdSendandStop, resdShutDown, resdSync };

extern char *MParam[];

extern const char *MJobState[];
extern const char *MCKeyword[];

/* socket structure used in ResourceDaemon-Scheduler communication */

typedef struct {
  int   PacketType;
  char  PacketID[MAX_MNAME];
  int   PacketSubID;
  char  NodeID[MAX_MNAME];
  char  Buffer[MAX_MLINE + 1];
  int   Status;
  } ResourceSt;

typedef struct {
  char  CollectionID[MAX_MNAME];
  int   CollectionSubID;
  char  Command[MAX_MNAME];
  long  CollectionStart;
  long  CollectionEnd;
  int   SampleCount; 
  int   TotalCSSPackets;
  int   TotalEtherPackets;
  int   TotalNFSReads;
  int   TotalNFSWrites;
  double AvgUserTime;
  int   MaxUserTime;
  double AvgSysTime;
  int   MaxSysTime;
  double AvgWaitTime;
  int   MaxWaitTime;
  double AvgSwapUsage;
  int   MaxSwapUsage;
  double AvgMemUsage;
  int   MaxMemUsage;
  double AvgDiskUsage;
  int   MaxDiskUsage;
  } ResourceDataSt;

typedef struct {
  int   Count;                      /* total jobs in grid                              */
  int   NCount;                     /* nodes allocated to jobs                         */
  int   JobCountSubmitted;          /* jobs submitted                                  */
  int   JobCountSuccessful;         /* jobs successfully completed                     */
  int   QOSMet;                     /* jobs that met requested QOS                     */
  int   RejectionCount;             /* jobs submitted that have been rejected          */
  unsigned long TotalQTS;           /* job queue time                                  */
  unsigned long MaxQTS;
  double TotalQueuedPH;             /* total queued prochours                          */ 
  double TotalRequestTime;          /* total requested job walltime                    */
  double TotalRunTime;              /* total execution job walltime                    */

  /* job-level statistics */

  double PSRequest;                 /* requested procseconds completed                 */
  double PSRequestSubmitted;        /* requested procseconds submitted                 */
  double PSRun;                     /* executed procsecond                             */
  double PSRunSuccessful;           /* successfully completed procseconds              */

  /* iteration-level statistics */

  double PSDedicated;               /* ProcSeconds Dedicated                           */
  double PSUtilized;                /* ProcSeconds Actually Used                       */

  double MSAvail;                   /* MemorySeconds Available                         */
  double MSUtilized;
  double MSDedicated;               /* MemorySeconds Dedicated                         */
  
  double PS2Dedicated;
  double PS2Utilized;

  double JobAcc;                    /* Job Accuracy Sum                                */
  double NJobAcc;                   /* Node Weighed Job Accuracy Sum                   */
  double XFactor;                   /* XFactor Sum                                     */
  double NXFactor;                  /* Node Weighed XFactor Sum                        */
  double PSXFactor;                 /* PS Weighted XFactor Sum                         */
  double MaxXFactor;                /* max XFactor detected                            */
  int    Bypass;                    /* Number of Times Job was Bypassed                */
  int    MaxBypass;                 /* Max Number of Times Job was not Scheduled       */
  int    BFCount;                   /* Number of Jobs Backfilled                       */
  double BFPSRun;                   /* ProcSeconds Backfilled                          */

  short   Accuracy[MAX_ACCURACY];
  dstat_t DStat[8];
  } must_t;


typedef struct {
  char   Name[MAX_MNAME];             /* cred name                     */
  int    Index;
  long   MTime;                       /* modify time                   */

  unsigned long Key;                  /* array hash key                */

  unsigned long OID;

  int    ClassWeight;                 /* NOTE: temp */

  mfs_t  F;                           /* fairness policy config        */
  void  *P;                           /* ??? */

  mcredl_t L;                         /* current resource usage        */                

  must_t Stat;                        /* historical usage statistics   */
  } mgcred_t;


typedef struct {
  char          Name[MAX_MNAME];
  int           Index;
  long          MTime;            /* time record was last updated               */
  long          QTWeight;
  long          XFWeight;
  long          QTTarget;
  double        XFTarget;         /* Targeted XFactor for QOS                   */
  unsigned long Flags;            /* Flags/Exemptions Associated with QOS       */
  char          ResName[16][MAX_MNAME];
  mfs_t         F;
  mcredl_t      L;
  must_t        Stat;             /* Usage Statistics                           */
  } mqos_t;

struct bres_s {
  long           EndTime;
  mcres_t        DRes;
  struct bres_s *Next;
  };

typedef struct bres_s bres_t;

typedef struct {
  char   Name[MAX_MNAME];
  int    Type;
  int  (*Func)(void *,int,void *,void **);             
  void  *Data;
  } xres_t;


typedef struct {
  int  VersionNumber;
  int  ProcVersionNumber;
  char ConfigFile[MAX_PATH_LEN];

  void *JobQO;
  void *JobList;
 
  void *NodeQO;
  void *NodeList;
  } mllmdata_t;

typedef struct {
  char    Name[MAX_MNAME];
 
  long    MTime;
 
  int     CProcs;
  int     CMem;
  int     CSwap;
 
  int     ADisk;
 
  int     ArchIndex;
 
  int     MState;
 
  double  Load;
 
  void   *xd;
  } pbsnodedata_t;

typedef struct {
  int    VersionNumber;
  int    ServerSD;
  long   ServerSDTimeStamp;
  msocket_t SchedS;
  long   SchedSDTimeStamp;
  char   LocalDiskFS[MAX_PATH_LEN];
  char   SubmitExe[MAX_PATH_LEN];

  int    PBS5IsEnabled;  /* boolean */
  int    SSSIsEnabled;   /* boolean */

  pbsnodedata_t D[MAX_MNODE];
 
  long          NodeMTime;
  long          JobMTime;
  long          QueueMTime;

  struct batch_status *NodeList;
  struct batch_status *JobList;
  struct batch_status *QueueList;
  } mpbsmdata_t;

typedef struct {
  int    VersionNumber;
  int    ServerSD;
  long   ServerSDTimeStamp;
  int    SchedSD;
  long   SchedSDTimeStamp;
  char   LocalDiskFS[MAX_PATH_LEN];
  char   SubmitExe[MAX_PATH_LEN];
 
  long   NodeMTime;
  long   JobMTime;
  long   QueueMTime;
 
  void  *NodeList;
  void  *JobList;
  void  *QueueList;
  } msgemdata_t;

typedef struct {
  int    VersionNumber;
  int    ServerSD;
  long   ServerSDTimeStamp;
  int    SchedSD;
  long   SchedSDTimeStamp;
  char   LocalDiskFS[MAX_PATH_LEN];
  char   SubmitExe[MAX_PATH_LEN];
 
  long   NodeMTime;
  long   JobMTime;
  long   QueueMTime;
 
  void  *NodeList;
  void  *JobList;
  void  *QueueList;
  } msssmdata_t;

typedef struct {
  char  *GData;
  } mlsfmdata_t;

enum MRMFlagEnum { mrmfNONE = 0, mrmfJobStageInEnabled, mrmfLocalQueue,      mrmfNoTaskOrdering, mrmfTypeIsExplicit };

#define MAX_MRMSUBTYPE      2
#define MAX_MRMFAILURE      8
#define MAX_MRMFUNC        22

typedef struct {
  char   Name[MAX_MNAME + 1];
  int    Index;

  long   Flags;

  char   APIVersion[MAX_MNAME];
  int    Version;

  enum MRMStateEnum State;

  long   StateMTime;
  int    XState;

  long   LastSubmissionTime;

  /* interface */

  enum MRMTypeEnum    Type;
  enum MRMSubTypeEnum SubType;

  mpsi_t P[MAX_MRMSUBTYPE];     /* peer interface */

  int    AuthType;

  int    NMPort;                /* node monitor port */

  int    EPort;                 /* event port */
  long   EMinTime;              /* min time between processing events */

  int    SpecPort[MAX_MRMSUBTYPE];
  int    SpecNMPort;
  char   SpecServer[MAX_MRMSUBTYPE][MAX_MNAME + 1];

  int    UseDirectoryService;  /* boolean */

  char   DefaultQMDomain[MAX_MNAME];
  char   DefaultQMHost[MAX_MNAME];

  char   SuspendSig[MAX_MNAME];

  int    FailIteration;
  int    FailCount;

  int    WorkloadUpdateIteration;

  long   FailTime[MAX_MRMFAILURE];
  int    FailType[MAX_MRMFAILURE];
  char  *FailMsg[MAX_MRMFAILURE];
  int    FailIndex;

  long   RespTotalTime[MAX_MRMFUNC];
  long   RespMaxTime[MAX_MRMFUNC];
  int    RespTotalCount[MAX_MRMFUNC];
  long   RespStartTime[MAX_MRMFUNC];

  int    JobCount;
  int    NodeCount;

  int    JobCounter;

  int    RMNI;    /* network interface utilized by RM */

  union {
    mllmdata_t  LL;
    mpbsmdata_t PBS;
    msgemdata_t SGE;
    msssmdata_t S3;
    mlsfmdata_t LSF;
    } U;

  void *S;  /* resource manager specific data */

  void *xd;
  } mrm_t;

enum MClassStateEnum {
  mclsNONE = 0,
  mclsActive,    /* accepting jobs, executing jobs */
  mclsDisabled,  /* not accepting jobs, not executing jobs */
  mclsClosed,    /* not accepting jobs, executing jobs */
  mclsStopped }; /* accepting jobs, not executing jobs */

typedef struct {
  char       Name[MAX_MNAME];
  int        Index;
  long       MTime;            /* time record was last updated               */     

  int        NonExeType;

  mfs_t      F;
  mcredl_t   L;
  must_t     Stat;

  int        State;            /* NOT USED */

  int        IsDisabled;       /* queue cannot execute jobs */

  int        DistPolicy;

  char     **NodeList;
  char      *OCNodeName;       /* overcommit node */
  double     OCDProcFactor;    /* dedicated proc factor */

  int        DefFBM[MAX_MATTR >> 5]; /* default feature (BM) */
 
  int        MaxProcPerNode;
  } mclass_t;


enum BFPriorityEnum {
  mbfpNONE = 0,
  mbfpRandom,
  mbfpDuration,
  mbfpHWDuration };

typedef struct {
  unsigned long MinTime;
  unsigned long MaxTime;
  unsigned long TimeStepCount;
  unsigned long TimeStep[MAX_SCALE];
  unsigned long TimeStepSize;
  unsigned long MinNode;
  unsigned long MaxNode;
  unsigned long NodeStepCount;
  unsigned long NodeStep[MAX_SCALE];
  unsigned long NodeStepSize;
  unsigned long AccuracyScale;
  unsigned long AccuracyStep[MAX_SCALE];
  long          BeginTime;
  long          EndTime;
  unsigned long TraceCount;
  } mprofcfg_t;

typedef struct {
  long   InitTime;

  long   SchedRunTime;            /* elapsed time scheduler has been scheduling                    */
  int    IdleJobs;
  int    EligibleJobs;            /* number of jobs eligible for scheduling                        */
  int    ActiveJobs;              /* number of jobs active                                         */
  double TotalPHAvailable;        /* total proc hours available to schedule                        */
  double TotalPHBusy;             /* total proc hours consumed by scheduled jobs                   */
  double SuccessfulPH;            /* proc hours completed successfully                             */
  int    SuccessfulJobsCompleted; /* number of jobs completed successfully                         */
  unsigned long AvgQueuePH;       /* average queue workload                                        */
  int    JobsEvaluated;           /* Total Jobs evaluated for scheduling                           */
  must_t Grid[MAX_MGRIDTIMES][MAX_MGRIDSIZES]; /* stat matrix                                      */
  must_t RTotal[MAX_MGRIDSIZES];   /* row totals                                                   */
  must_t CTotal[MAX_MGRIDSIZES];   /* column totals                                                */
  char   StatDir[MAX_MLINE + 1];  /* Directory For Stat Files                                      */
  double MinEff;                  /* minimum scheduling efficiency                                 */
  int    MinEffIteration;         /* iteration on which the minimum efficiency occurred            */
  double TotalSimComCost;         /* Total Simulated Communication Cost                            */
  int    TotalFSAdjustedFSUsage;  /* Total Decayed FSUsage for FairShare                           */
  int    TotalHeterogeneousJobs;
  int    TotalPreemptJobs;
  double TotalPHPreempted;

  mprofcfg_t P;
  } mstat_t;

/* priority object */
 
enum MPrioComponentEnum {
  mpcNONE = 0,
  mpcServ,      /* service */
  mpcTarg,      /* target service */
  mpcCred,      /* credential */
  mpcAttr,      /* job attribute */
  mpcFS,        /* fairshare */
  mpcRes,       /* resource request */
  mpcUsage      /* resource usage */
  };
 
#define MAX_MPRIOCOMPONENT  8
 
enum MPrioSubComponentEnum {
  mpsNONE = 0,
  mpsSQT,
  mpsSXF,
  mpsSSPV,
  mpsSBP,
  mpsTQT,
  mpsTXF,
  mpsCU,
  mpsCG,
  mpsCA, 
  mpsCQ,
  mpsCC,
  mpsFU,
  mpsFG,
  mpsFA,
  mpsFQ,
  mpsFC,
  mpsAAttr,
  mpsAState,
  mpsRNode,
  mpsRProc,
  mpsRMem,
  mpsRSwap,
  mpsRDisk,
  mpsRPS,
  mpsRPE,
  mpsRWallTime,
  mpsRUProc,
  mpsRUJob,
  mpsUCons,
  mpsURem,
  mpsUPerC
  };
 
#define MAX_MPRIOSUBCOMPONENT  32
 
/* fairshare */
 
typedef struct {
  long   PCW[MAX_MPRIOCOMPONENT];
  long   PCP[MAX_MPRIOCOMPONENT];
 
  long   PSW[MAX_MPRIOSUBCOMPONENT];
  long   PSP[MAX_MPRIOSUBCOMPONENT];
 
  long   PCC[MAX_MPRIOCOMPONENT];
  long   PSC[MAX_MPRIOSUBCOMPONENT]; 
 
  long   XFMinWCLimit;
 
  int    FSPolicy;                /* FairShare Enforceability                                      */
  long   FSInterval;              /* Time Interval Covered by each FS Data File                    */
  int    FSDepth;                 /* Number of FS Time Intervals Considered by FairShare           */
  double FSDecay;                 /* Weighting Factor to Decay Older FS Intervals                  */
  } mfsc_t;


/* par */

typedef struct {
  char   Name[MAX_MNAME + 1];
 
  int    Index;
 
  int    ConfigNodes;
  int    IdleNodes;                 /* any proc is idle                        */
  int    ActiveNodes;               /* any proc is busy                        */
  int    UpNodes;

  mcres_t    CRes;                  /* configured resources                    */
  mcres_t    URes;                  /* up resources                            */
  mcres_t    ARes;                  /* available resources                     */
  mcres_t    DRes;                  /* dedicated resources                     */
  must_t     S;                     /* partition stats                         */
 
  mfsc_t     FSC;
 
  mcredl_t   L;
  mfs_t      F;
 
  int BFPolicy;                     /* backfill policy                         */
  int BFDepth;                      /* depth queue will be searched            */
  int BFMetric;                     /* backfill utilization metric             */
  int BFProcFactor;                 /* factor to encourage use of large jobs   */
  int BFMaxSchedules;               /* maximum schedules to consider           */
  int BFPriorityPolicy;
  int BFChunkSize;
  long BFChunkDuration;

  long BFChunkBlockTime;          /* time at which jobs smaller than BFChunkSize may be considered */
 
  /* system policies */
 
  int MaxMetaTasks;
 
  int MaxJobStartTime;            /* time allowed for nodes to become busy   */
  int NAllocPolicy;               /* algo for allocating nodes               */
  int DistPolicy;
  int JobSizePolicy;              /* policy for determining job size         */
  int JobNodeMatch;
 
  int ResPolicy;                  /* policy for creating/managing reservations */
  int ResRetryTime;               /* time reservations are retried if blocked by system problems */
  int ResTType;                   /* reservation threshold type              */
  int ResTValue;                  /* reservation threshold value             */
  int ResDepth[MAX_MQOS];         /* terminated by '0'                       */
  int ResCount[MAX_MQOS];
  mqos_t *ResQOSList[MAX_MQOS][MAX_MQOS];  /* NULL terminated, MAX_MQOS global */

  /* booleans */
  
  int UseMachineSpeed;
  int UseSystemQueueTime;
  int UseCPUTime;
  int RejectNegPrioJobs;
  int EnableNegJobPriority;        
  int EnableMultiNodeJobs;
  int EnableMultiReqJobs;

  int    JobPrioAccrualPolicy;

  char   NAvailPolicy[MAX_MRESOURCETYPE];

  /* resource limit handling */

  enum MResLimitPolicyEnum  ResourceLimitPolicy[MAX_MRESOURCETYPE];
  enum MResLimitVActionEnum ResourceLimitViolationAction[MAX_MRESOURCETYPE];
  long                      ResourceLimitMaxViolationTime[MAX_MRESOURCETYPE];

  long   AdminMinSTime;

  int    UseLocalMachinePriority;   /* boolean */
  int    NodeLoadPolicy;
  int    NodeSetPolicy;
  int    NodeSetAttribute;
  char  *NodeSetList[MAX_MATTR];    /* list of set delineating job attributes     */
  long   NodeSetDelay;
  double NodeSetTolerance;
  int    NodeSetPriorityType;
  double UntrackedProcFactor;
  int    NodeDownStateDelayTime;
 
  xres_t XRes[MAX_MNODE];

  char  *Message;    /* event messages */
  } mpar_t;
 
/* cred */
 
typedef struct {
  mgcred_t *U;
  mgcred_t *G;
  mgcred_t *A;
  mclass_t *C;
  mqos_t   *Q;
 
  int       CredType;
 
  macl_t   *ACL;
  macl_t   *CL;
 
  long      MTime;
  } mcred_t;

typedef struct {
  char  Name[MAX_MLINE + 1];       /* job ID                 */
  int   Index;                     /* index in mjob_t table  */ 
  } mjobl_t;

typedef struct nodealloc_t {
  short          nodeindex;
  unsigned short taskcount;
  } nodealloc_t;

typedef struct {
  char    Name[MAX_MNAME + 1];   /* name of object reserving nodes             */
  int     Index;

  long    MTime;
  long    CTime;

  char   *RegEx;                 /* host regular expression                    */
  int     PtIndex;                
  int     Type;                  /* reservation type                           */
  int     Mode;                  /* persistant, slide-forward, slide-back, 
                                    slide-any                                  */
  void   *J;
  void   *NL;

  long    StartTime;             /* reservation starttime                      */
  long    EndTime;               /* reservation endtime                        */

  int     MaxTasks;

  int     NodeCount;             /* nodes involved in reservation              */
  int     TaskCount;
  int     ETaskCount;
  int     AllocPC;            
  macl_t  ACL[MAX_MACL];
  macl_t  CL[MAX_MACL];
 
  unsigned long Flags;           /* reservation flags                          */

  double  CIPS;                  /* allocation/utilization stats */
  double  CAPS;
  double  TIPS; 
  double  TAPS;

  char    AccessList[16][MAX_MNAME]; 
  int     AllocResPending;
  long    ExpireTime;
  long    Priority;  
  mcres_t DRes;
  char    CBHost[MAX_MNAME];
  int     CBPort;
  long    CBType;

  char   *SystemID;               /* user or daemon which created reservation */
  char   *Comment;

  mgcred_t *U;                    /* accountable user, group, account */
  mgcred_t *G;                    /* NOTE:  accountable cred same as creation cred */
  mgcred_t *A;        

  char   *XCP;

  int     IsActive;

  void   *O;          /* rsv owner */
  int     OType;

  void   *xd;
  } mres_t;

#define mrsv_t mres_t


typedef struct {
  char          Name[MAX_MNAME];
  int           Period;           /* Day || Week || Infinite */
  int           Type;
  int           Days;
  int           Depth;
  unsigned long StartTime;
  unsigned long EndTime;
  unsigned long WStartTime;
  unsigned long WEndTime;
  macl_t        ACL[MAX_MACL];
  int           TaskCount;
  int           TPN;
  int           MaxIdleTime;      /* idle WC time allowed before reservation is automatically released */
  double        MinLoad;
  char          PName[MAX_MNAME];
  mcres_t       DRes;
  mres_t       *R[MAX_SRES_DEPTH];
  void         *HostList;
  char          HostExpression[MAX_MLINE << 4];
  int           Index;
  mgcred_t     *A;
  int           Flags;    
  double        TotalTime;
  double        IdleTime;
  long          Priority;
  int           FeatureMode;
  int           FeatureMap[MAX_MATTR >> 5];

  void         *O;  /* owner */
  int           OType;
  } sres_t;

#define srsv_t sres_t

typedef struct {
  long       Time;
  short      Type;
  short      Index;
  mcres_t    DRes;  /* total resources dedicated to reservation across all tasks on node */
  } mre_t;

typedef struct {
  long     Time;
  int      Type;
  int      Index;
  mcred_t *C;
  int      NodeCount;
  int      TaskCount;
  long     PS;
  } pe_t;

typedef struct {
  long          MTime;

  char          RMName[MAX_MNAME + 1];
  char          NetName[MAX_MNETTYPE][MAX_MNAME + 1];
  unsigned long NetAddr[MAX_MNETTYPE];

  unsigned long Attributes;
  int           SlotsUsed;
  int           State;
  int           Arch;
  int           Disk;
  int           Classes;
  int           Failures;
  } mhost_t;

typedef mhost_t msys_t[MAX_MFRAME][MAX_MSLOTPERFRAME + 1];

enum ResourceSetTypeEnum { mrstNONE = 0, mrstFeature, mrstMemory, mrstProcSpeed };
enum ResourceSetSelectionEnum { mrssNONE = 0, mrssOneOf, mrssAnyOf, mrssFirstOf };
enum ResourceSetPrioEnum { mrspNONE = 0, mrspBestFit, mrspWorstFit, mrspBestResource, mrspMinLoss };

typedef struct {
  int    NAllocPolicy;
  int    ResOrder[4];
  } mnallocp_t;

typedef struct {
  /* resource:  walltime, procs, nodes, memory, network, etc */
  /* stat:      psutilized, etc */
  /* attr:      credentials, QOS, etc */

  int  TotalProcCount;
  } mjobcache_t;

typedef struct msdata_t {
  char *Location;

  int   IsGlobal;

  char *SrcFileName;
  char *DstFileName;

  char *SrcHostName;
  char *DstHostName;

  long  SrcFileSize;     /* available file size */
  long  DstFileSize;     /* total file size */
  
  long  TStartTime;
  int   TransferRate;  /* KB/s */

  long  ESTime;        /* estimated stage time */
  struct msdata_t *Next;
  } msdata_t;

typedef struct {
  long   CPInterval;

  long   StoredCPDuration; /* duration of walltime checkpointed in previous runs */
  long   ActiveCPDuration; /* duration of walltime checkpointed in current run */

  long   InitialStartTime;
  long   LastCPTime;

  char   UserICPEnabled;
  char   SystemICPEnabled;
  } mjckpt_t;

enum {
  mhlmNONE = 0,
  mhlmSuperset,
  mhlmSubset,
  mhlmExactSet 
  };

#define MAX_MNPCOMP 16

enum MNPrioEnum {
  mnpcNONE = 0,
  mnpcADisk,       /* avl local disk */
  mnpcAMem,        /* avl real memory */
  mnpcAProcs,      /* avl procs */
  mnpcASwap,       /* avl swap */
  mnpcCDisk,       /* cfg local disk */
  mnpcCMem,
  mnpcCProcs,
  mnpcCSwap,
  mnpcJobCount,    /* active jobs on node */
  mnpcLoad,        /* processor load */
  mnpcPref,        /* node is preferred */     
  mnpcPriority,    /* node admin priority */
  mnpcResAffinity, /* reservation affinity */
  mnpcSpeed,       /* processor/machine speed (???) */
  mnpcUsage,       /* percent utilization over time */
  mnpcPLoad };     /* percentage load */

typedef struct {
  double  CW[MAX_MNPCOMP];   /* component weight */
  int     CWIsSet;

  double  SP;                /* static priority  */
  int     SPIsSet;

  double  DP;                /* dynamic priority */
  int     DPIsSet;
  int     DPIteration;
  } mnprio_t;


/* extended load indices */

typedef struct {
  double        PageIn;
  double        PageOut;
  int           LoginCount;
  } mxload_t;


#define MAX_MJOB_PER_NODE  64

typedef struct {
  char   Name[MAX_MNAME + 1]; /* name of node                                */
  int    Index;               /* index in node array                         */

  char  *FullName;            /* fully qualified host name                   */

  long   CTime;               /* time node record was created                */
  long   ATime;               /* time of most recent node information        */
  long   MTime;               /* time node structure was modified            */

  long   ResMTime;
  long   StateMTime;          /* time node changed state                     */

  enum MNodeStateEnum State;  /* node state                                  */
  enum MNodeStateEnum EState; /* expected node state resulting from sched action */
  enum MNodeStateEnum NewState;

  long     SyncDeadLine;      /* time by which state and estate must agree   */

  mcres_t  DRes;              /* dedicated resources (dedicated to consumer) */
  mcres_t  CRes;              /* configured resources                        */
  mcres_t  ARes;              /* available resources                         */
  mcres_t  URes;              /* utilized resources                          */
  mcres_t  SURes;             /* system utilized resources                   */

  int      PtIndex;           /* partition assigned to node                  */

  int      Network;           /* available network interfaces (BM)           */
  int      FBM[MAX_MATTR >> 5]; /* available features (BM)                   */
  int      Attributes;        /* system attributes (BM)                      */

  char     Pool[MAX_MPOOL + 1];
  int      Arch;              /* node hardware architecture                  */
  int      ActiveOS;          /* node operating system                       */

  mres_t **R;                 /* node reservation table                      */
  mre_t   *RE;                /* reservation event table                     */
  short   *RC;                /* reservation count table                     */

  double   Speed;             /* relative processing speed of node           */
  int      ProcSpeed;         /* proc speed in MHz                           */

  double   Load;              /* total node 1 minute load average            */
 
  double   JobLoad;           /* load associated with batch workload         */
  double   ExtLoad;           /* load not associated with batch workload     */
  mxload_t *XLoad;

  double   MaxLoad;           /* max total load allowed                      */

  double   BackLog;           /* avg work required by node                   */

  mulong   STTime;            /* time node was monitored (in 1/100's)        */
  mulong   SUTime;            /* time node was available (in 1/100's)        */
  mulong   SATime;            /* time node was active    (in 1/100's)        */

  int      TaskCount;
  int      EProcCount;

  short    FrameIndex;        /* frame index                                 */
  short    SlotIndex;         /* slot index                                  */
  short    SlotsUsed;

  mrm_t   *RM;

  char     NodeType[MAX_MNAME];

  int      IterationCount;    /* iterations since last RM update        */
  int      Priority;

  int      MaxJCount;
  void   **JList;             /* list of active jobs                    */
  int     *JTC;

  void    *RMData;

  mpu_t    AP;                /* active policy tracking                 */

  char    *NAvailPolicy;      /* node availability policy               */

  int      MaxJobPerUser;     /* TEMP                                   */
  int     *MaxProcPerClass;   /* TEMP                                   */
  int      MaxProcPerUser;
  double   MaxPEPerJob;       /* TEMP                                   */

  char     PrivateQueue;      /* TEMP (boolean)                         */
  char     IsPref;            /* TEMP (boolean)                         */

  mnprio_t *P;                /* node priority parameters */

  char    *Message;           /* event message */

  void    *xd;
  } mnode_t;

typedef struct {
  mnode_t        *N;
  unsigned short  TC;
  } mnalloc_t;

typedef struct {
  mnode_t       *N;
  unsigned short TC;
  double         Prio;
  } mnpri_t;

typedef mnalloc_t mnodelist_t[MAX_MREQ_PER_JOB][MAX_MNODE + 1];
typedef mnalloc_t nodelist_t[MAX_MNODE + 1];


typedef struct {
  int    Index;

  int    RequiredProcs;      /* procs                                       */
  int    ProcCmp;            /* procs comparison                            */
  int    RequiredMemory;     /* real memory (RAM)                           */
  int    MemCmp;             /* memory comparison                           */
  int    RequiredSwap;       /* virtual memory (in MB)                      */
  int    SwapCmp;            /* virtual memory comparison                   */
  int    RequiredDisk;       /* disk space requirement (in MB)              */
  int    DiskCmp;            /* disk comparison                             */
  int    ReqFBM[MAX_MATTR >> 5]; /* required feature (BM)                   */
  int    PrefFBM[MAX_MATTR >> 5]; /* preferred feature (BM)                 */
  int    ReqFMode;           /* required feature mode                       */

  int    SetSelection;       /* one of NONE, ONEOF, ANYOF                   */
  int    SetType;            /* one of NONE, Feature, Memory, ProcSpeed     */
  char  *SetList[MAX_MATTR]; /* list of set delineating job attributes      */
  int    Pool;               /* RM pool index                               */
  int    Network;            /* required network (BM)                       */
  int    Opsys;              /* required OS (BM)                            */
  int    Arch;               /* HW arch (BM)                                */

  int    PtIndex;            /* assigned partition index                    */

  mcres_t  DRes;             /* dedicated resources per task (active)       */
  mcres_t *SDRes;            /* dedicated resources per task (suspended)    */
  mcres_t  URes;             /* utilized resources per task                 */
  mcres_t  LURes;            /* req lifetime utilized resources per task    */
  mcres_t  MURes;            /* maximum utilized resources per task         */

  unsigned long RMWTime;     /* req walltime as reported by RM              */
  unsigned long StatWTime;

  /* index '0' is active, index '1-N' contain requests */

  int    TaskRequestList[MAX_TASK_REQUESTS + 1];

  int    NodeCount;          /* nodes requested */
  int    TaskCount;          /* tasks allocated */

  int    TasksPerNode;
  int    BlockingFactor;
  int    NAccessPolicy;      /* node access policy */
  mnallocp_t *NAllocPolicy;

  int    AdapterAccessMode;
  int    RMIndex;

  enum MJobStateEnum State;
  enum MJobStateEnum EState;

  char   SubJobName[MAX_MNAME];

  mnalloc_t *NodeList;
  } mreq_t;

#define MMAX_JOBRA  16

typedef struct {
  char  *IWD;                /* initial working directory of job             */
  char  *Cmd;                /* job executable                               */
  char  *Input;              /* input                                        */
  char  *Output;             /* output                                       */
  char  *Error;              /* error                                        */
  char  *Args;               /* command line args                            */
  char  *Env;                /* shell environment                            */
  char  *Shell;              /* execution shell                              */
  char  *JobType;            /* job type { ie, parallel, serial, etc }       */

  long   PID;
  long   SID;
  } mjobe_t;
 
typedef struct {
  int TC;
  int NC;
  } mrsrc_t;
 
typedef struct mjob_t {
  char   Name[MAX_MNAME + 1]; /* job ID                                  */
  char  *AName;               /* alternate name (user specified)         */
  char  *RMJID;               /* resource manager job ID                 */

  int    Index;               /* job table index                         */  

  long   CTime;               /* creation time (first RM report)         */
  long   MTime;               /* modification time (any source)          */
  long   ATime;               /* access time (most recent RM report)     */

  long   StateMTime;              

  long   SWallTime;           /* duration job was suspended              */
  long   AWallTime;           /* duration job was executing              */
 
  mjckpt_t *Ckpt;             /* checkpoint structure                    */

  struct mjob_t *Next;      
  struct mjob_t *Prev;   

  mjobcache_t C;
  macl_t      RCL[MAX_MACL]; /* required CL                              */
  mcred_t     Cred;            

  mqos_t     *QReq;

  mres_t     *R;             /* reservation                              */     
  char        ResName[MAX_MNAME];
  char        RAList[MMAX_JOBRA][MAX_MNAME]; /* reservation access list          */

  mrm_t      *RM;

  int         SessionID;

  char     *NeedNodes;

  msdata_t *SIData;         /* stage in data  */
  msdata_t *SOData;         /* stage out data */

  mreq_t *Req[MAX_MREQ_PER_JOB + 1];
  mreq_t *GReq;

  mreq_t *MReq;             /* primary req */

  mjobe_t E;                /* job execution environment               */

  int    NodeCount;         /* nodes assigned                          */
  int    TaskCount;         /* tasks assigned                          */
  int    TasksRequested;    /* tasks requested                         */
  int    NodesRequested;    /* number of nodes requested               */

  mrsrc_t Alloc;
  mrsrc_t Request;

  char  *Geometry;
  char   SpecDistPolicy;
  char   DistPolicy;   

  long   SystemQueueTime;   /* time job was initially eligible to start */
  long   EffQueueDuration;  /* duration of time job was eligible to run */

  mulong SMinTime;          /* effective earliest start time           */
  mulong SpecSMinTime;      /* user specified earliest start time      */
  mulong SysSMinTime;       /* system mandated earliest start time     */
  mulong CMaxTime;          /* user specified latest completion time   */
  mulong TermTime;          /* time by which job must be terminated    */
  mulong RMinTime;          /* earliest 'resume' time for suspend job  */
  mulong SubmitTime;        /* time job was submitted to RM            */
  mulong StartTime;         /* time job began most recent execution    */
  mulong DispatchTime;      /* time job was dispatched by RM           */
  mulong CompletionTime;    /* time job execution completed            */
  mulong LastNotifyTime;
  int    CompletionCode;    /* execution completion code               */
  int    StartCount;        /* number of times job was started         */
  int    DeferCount;        /* number of times job was deferred        */
  int    PreemptCount;      /* number of times job was preempted       */
  int    Bypass;            /* number of times lower prio job was backfilled */       

  int    HoldReason;        /* reason job was deferred/held by scheduler */            
  int    BlockReason;       /* reason job not considered for scheduling */

  long   SyncDeadLine;      /* time by which state and estate must agree */     
  char  *Message;           /* job event message                        */     

  mutime WCLimit;           /* effective walltime limit                 */
  mutime CPULimit;          /* specified CPU limit (per job)            */
  mutime SpecWCLimit[MAX_TASK_REQUESTS + 1];     
  mutime RemainingTime;     /* execution time job has remaining         */
  mutime SimWCTime;         /* total time sim job will consume          */

  int     MinMachineSpeed;  /* (in MHz) */

  int     SpecPAL[MAX_MPALSIZE];              
  int     PAL[MAX_MPALSIZE];              

  int     ImageSize;        /* input data size (in MB) */
  int     ExecSize;         /* executable size (in MB) */

  enum MJobStateEnum State;  /* RM job state                                    */
  enum MJobStateEnum EState; /* expected job state due to scheduler action      */    
  enum MJobStateEnum IState; /* internal job state                              */

  int     Hold;             /* BM { USER  SYS  BATCH }                          */
  int     SuspendType;          

  long    SystemPrio;       /* system admin assigned priority                   */    
  long    UPriority;        /* job priority given by user                       */         
  long    StartPriority;    /* priority of job to start                         */
  long    RunPriority;      /* priority of job to continue running              */

  short   TaskMap[MAX_MTASK_PER_JOB + 1];

  mnalloc_t *NodeList;    /* list of allocated hosts */

  mnalloc_t *ReqHList;    /* required hosts - {sub,super}set */
  mnalloc_t *ExcHList;    /* excluded hosts                  */

  int        ReqHLMode;   /* req hostlist mode               */

  int    RType;
  int    Cluster;           /* step cluster                                     */
  int    Proc;              /* step proc                                        */
  char   SubmitHost[MAX_MNAME]; /* job submission host                          */

  unsigned long IFlags;
  unsigned long Flags;     
  unsigned long SpecFlags; 
  unsigned long SysFlags;  

  unsigned long AttrBM;

  char  *MasterJobName;
  char  *MasterHostName;

  double PSUtilized;        /* procseconds utilized by job                      */
  double PSDedicated;       /* procseconds dedicated to job                     */
  double MSUtilized;
  double MSDedicated;

  char  *RMXString;         /* RM extension string (opaque) */
  char  *RMSubmitString;    /* raw command file */
  int    RMSubmitType;

  char  *SystemID;          /* identifier of the system owner */
  char  *SystemJID;         /* external system identifier */

  mjdepend_t *Depend;

  int    RULVTime;
  int    FeatureMap[1];     

#ifdef __MCPLUS
  int    (*ASFunc)(mjob_t *,int,void *,void *); 
#else
  int    (*ASFunc)();
#endif /* __MCPLUS */

  void  *ASData;

  void  *xd;
  } mjob_t;


typedef struct {
  char Name[MAX_MNAME + 1];
  } node_t;

typedef struct {
  int          RMType;
  mres_t      *R;
  nodealloc_t  NodeList[MAX_MNODE_PER_FRAG + 1];
  short        TaskList[MAX_MTASK_PER_FRAG + 1];   
  } ll_frag_t;

typedef struct {
  int      RMType;
  mres_t  *R;
  } sgi_frag_t;

typedef struct {
  int      RMType;
  mres_t  *R;
  } sge_frag_t;

typedef struct frag_t {
  int      FragType;
  struct frag_t *Next;
  struct frag_t *Prev;
  int      MinTask;
  int      MaxTask;

  union {
    ll_frag_t   LL;
    sgi_frag_t  SGI;
    sge_frag_t  SGE;
    } RM;
  } frag_t;



/* NOTE:  must be sync'd with object table */

#define MDEF_FBPOLLINTERVAL  30
#define MDEF_FBFAILURECOUNT   2

#define MDEF_ADMINACTIONINTERVAL 600

typedef struct {
  char  Name[MAX_MNAME];
  long  StartTime;

  mulong Time;                      /* current epoch time                              */

  char  Version[MAX_MNAME];         /* scheduler version                               */

  struct timeval SchedTime;         /* accurate system time at start of scheduling     */
  long  GreenwichOffset;
  int   Interval;                   /* time transpired (hundredths of a seocnd)        */

  char  Day[MAX_WORD + 1];          /* current local day of the week                   */
  int   DayTime;                    /* current local hour of the day                   */
  long  RMPollInterval;             /* interval between scheduling attempts (seconds)  */
  long  RMJobAggregationTime;

  int   Mode;                       /* mode of scheduler operation                     */
  int   SpecMode;                   /* configured scheduler operation mode             */

  int   StepCount;                  /* number of scheduling steps before shutdown      */

  char  ConfigFile[MAX_MLINE];      /* sched configuration file name                   */

  char *ConfigBuffer;
  char *PvtConfigBuffer;

  char  LogDir[MAX_MLINE];          /* directory for logging                           */
  char  LogFile[MAX_MLINE];         /* log file                                        */
  char  ToolsDir[MAX_MLINE];        /* directory for sched tools                       */
  char  HomeDir[MAX_MLINE];         /* scheduler home directory                        */
  char  LockFile[MAX_MLINE];        /* scheduler lock file                             */
  char  KeyFile[MAX_MLINE];
 
  char  Action[4][MAX_MLINE];       /* response for specified events                   */
  long  ActionInterval;

  int   LogFileMaxSize;             /* maximum size of log file                        */
  int   LogFileRollDepth;

  char  ServerHost[MAX_MNAME];      /* name of scheduler server host                   */
  int   ServerPort;                 /* socket used to communicate with sched client    */

  char  FBServerHost[MAX_MNAME];
  int   FBServerPort;

  long  FBPollInterval;
  int   FBFailureCount;

  int   FBActive;                   /* boolean */

  msocket_t ServerS;
  msocket_t ServerSH;               /* web interface                                   */
  int   DefaultMCSocketProtocol;      
  long  ClientTimeout;     

  char  DefaultCSKey[MAX_MNAME];
  int   DefaultCSAlgo;

  /* SSL interface */

  msocket_t SServerS;

  /* directory server */

  mpsi_t DS;  /* directory service interface */
  mpsi_t EM;  /* event manager interface */

  FILE *statfp;

  char  DefaultDomain[MAX_MNAME];   /* domain appended to unqualified hostnames        */
  char  DefaultQMHost[MAX_MNAME];

  char  Admin1User[MAX_MADMINUSERS + 1][MAX_MNAME]; /* admin usernames                 */
  char  Admin2User[MAX_MADMINUSERS + 1][MAX_MNAME];
  char  Admin3User[MAX_MADMINUSERS + 1][MAX_MNAME];
  char  Admin4User[MAX_MADMINUSERS + 1][MAX_MNAME];
  char  AdminHost[MAX_MADMINHOSTS + 1][MAX_MNAME];  /* hosts allowed admin access      */

  long  DeferTime;                  /* time job will stay deferred                     */
  int   DeferCount;                 /* times job will get deferred before being held   */
  int   DeferStartCount;            /* times job will fail starting before getting deferred */
  long  JobPurgeTime;               /* time job must be missing before getting purged  */
  long  NodePurgeTime;
  int   APIFailureThreshhold;       /* times API can fail before notifying admins      */
  long  NodeSyncDeadline;           /* time in which node must reach expected state    */
  long  JobSyncDeadline;            /* time in which job must reach expected state     */
  long  JobMaxOverrun;              /* time job is allowed to exceed wallclock limit   */
  int   Iteration;                  /* number of scheduling cycles completed           */

  int   Reload;                     /* reload config file at next interval (boolean)   */
  int   Schedule;                   /* scheduling should continue (boolean)            */
  int   Shutdown;
  int   EnvChanged;                 /* scheduling env has changed (boolean)            */

  int   MaxSleepIteration;          /* max iterations scheduler can go without scheduling */
  char  LLConfigFile[MAX_PATH_LEN];

  /* policies */

  enum MNodeAccessPolicyEnum       DefaultNAccessPolicy;

  int   AllocLocalityPolicy;
  int   AllocLocalityGroup;

  int   WCViolAction;

  int   UseJobRegEx;                /* (config boolean) */
  int   SPVJobIsPreemptible;        /* (config boolean) */

  mbool_t EnableEncryption;

  char *Argv[MAX_MARG];

  long  PresentTime;

  char  MonitoredJob[MAX_MLINE];
  char  MonitoredNode[MAX_MLINE];
  char  MonitoredRes[MAX_MLINE];
  char  MonitoredFunction[MAX_MLINE];

  int   ResDepth;
  char  DefaultClassList[MAX_MLINE];

  int   SecureMode;        /* (state boolean) */
  int   DebugMode;         /* (state boolean) */
  int   CrashMode;         /* (state boolean) */

  long  CurrentFSInterval;
  long  DisplayFlags;      /* (BM) */
  long  TraceFlags;        /* (BM) */
  long  RMLoadStart;
  int   NodePollFrequency;
  int   NodePollOffset;
  char  ProcSpeedFeatureHeader[MAX_MNAME];
  int   ProcSpeedFeatureIsVisible;    /* (boolean) */
  char  NodeTypeFeatureHeader[MAX_MNAME];
  int   NodeTypeFeatureIsVisible;     /* (boolean) */
  char  PartitionFeatureHeader[MAX_MNAME];
  int   PartitionFeatureIsVisible;    /* (boolean) */
  int   InitialLoad;

  mgcred_t *DefaultU;
  mgcred_t *DefaultG;
  mgcred_t *DefaultA;
  mqos_t   *DefaultQ;
  mclass_t *DefaultC;
  mnode_t   DefaultN; 
  mjob_t    DefaultJ;

  mnode_t  *GN;

  char   DefaultAccountName[MAX_MNAME];
  int    ReferenceProcSpeed;
  long   PreemptPrioMargin;
  double NodeCPUOverCommitFactor;
  double NodeMemOverCommitFactor;
  int    MaxJobPerIteration;
  long   MinDispatchTime;
  double MaxOSCPULoad;

  int    UseSyslog;      /* config boolean */
  int    SyslogActive;   /* state boolean */
  int    SyslogFacility;
 
  int    ResCtlPolicy;
  int    ResLimitPolicy;
  int    PreemptPolicy;

  int    TimePolicy;
  long   TimeOffset;  /* offset between actual and trace time */
 
  int    ResourcePrefIsActive;  /* state boolean */
#ifdef __SANSI__
  int    PID;
#else /* __SANSI__ */
  pid_t  PID;
#endif /* __SANSI__ */

  int    UID;
  int    GID;

  mpar_t *GP;

  char **IgnQList;

  void *T[mxoLAST];   /* object table   */
  int   S[mxoLAST];   /* size of object */
  int   M[mxoLAST];   /* number of objects in table */
  void *E[mxoLAST];   /* final object */

  void *X;

  msync_t Sync;

  char *Message;      /* general system messages */

  mgrid_t G;

  char *ComputeHost[MAX_MNODE];  /* list of hosts eligible to schedule jobs */
  mnode_t *ComputeN[MAX_MNODE];

  int (*HTTPProcessF)(msocket_t *,char *);
  } msched_t;


typedef struct {
  char   Name[MAX_MNAME + 1];   /* name */

  int    Index;

  long   MTime;
  int    Type;

  int    Memory;
  int    Disk;    
  int    PtIndex;   
  int    NodeCount;
  int    TempSize;
  char   NodeType[MAX_MNAME];
  } mframe_t;

typedef char mattrlist_t[MAX_MLIST][MAX_MATTR][MAX_WORD];



/* structure for passing statistics data to client */

typedef struct {
  char   Name[MAX_MNAME];
  int    ID;
  int    ActiveJobs;
  int    ActiveNodes;
  int    ActivePS;
  int    CompletedJobs;
  int    QueueTimeSum;
  double PSRequest;
  double PSRun;
  double PSDedicated;
  double PSUtilized;
  double MSAvail;
  double MSDedicated;
  double Target;
  double JobAccuracySum;
  double NJobAccuracySum;
  double XFactorSum;
  double PSXFactorSum;
  double MaxXFactor;
  int    BypassSum;
  int    MaxBypass;
  } cstats;

typedef struct {
  long    StartTime;
  long    EndTime;
  int     PIndex;
  int     PIndexCmp;
  int     FIndex;
  int     FIndexCmp;
  int     NIndex;
  int     NIndexCmp;
  int     UID;
  int     UIDCmp;
  int     GID;
  int     GIDCmp;
  char    User[MAX_MNAME];
  int     UserCmp;
  char    Group[MAX_MNAME];
  int     GroupCmp;
  char    EtherName[MAX_MNAME];
  int     EtherNameCmp;
  char    SwitchName[MAX_MNAME];
  int     SwitchNameCmp;
  char    JName[MAX_MNAME];
  int     JNameCmp;
  char    NState[MAX_MNAME];
  int     NStateCmp;
  int     SlotsUsed;
  int     SlotsUsedCmp;
  char    JState[MAX_MNAME];
  int     JStateCmp;
  char    Adapter[MAX_MNAME];
  int     AdapterCmp;
  char    Feature[MAX_MLINE];
  int     FeatureCmp;
  char    Arch[MAX_MNAME];
  int     ArchCmp;
  char    Opsys[MAX_MNAME];
  int     OpsysCmp;
  int     Memory;
  int     MemoryCmp;
  int     Disk;
  int     DiskCmp;
  int     Swap;
  int     SwapCmp;
  } ConstraintSt;

/* requires moab scheduling system */

#include "moab.h"

#endif /* __M_H__ */

