/*
*/

#if !defined(__MOAB_H)
#define __MOAB_H

#include "moab-local.h"
#include "mcom.h"

#define MOAB_VERSION "3.2.6p14"

/* elemental objects */

typedef struct {
  long  UpdateTime;
  void *Updates;

  int   IsSynced; /* boolean */
  } msync_t;

typedef struct {
  long  MTime;
  int   FailureCount;
  int   QCount;  
  int   JCount; 
  int   RCount; 
  int   CCount;

  int   SIsInitialized;  
  int   IIsInitialized; 

  char *Messages;
  } mgrid_t;

typedef struct
  {
  char *Data;     
  char *Owner;    
  char *Source;     
  long  ExpireTime; 
  long  CTime;     
  int Type;

  int   Priority;  
  int   Count;    

  void *Next;    
  } mmb_t;

enum MValModEnum {
  mvmByte = 0,
  mvmWord,
  mvmKilo,
  mvmMega,
  mvmGiga,
  mvmTera };

enum MTimeModeEnum {
  mtmNONE = 0,
  mtmInit,
  mtmRefresh
  };

/* resource limit policies */

enum MResLimitResourceEnum {
  mrlrNONE = (1 << 0),
  mrlrNode = (1 << 1),
  mrlrProc = (1 << 2),
  mrlrMem  = (1 << 3),
  mrlrSwap = (1 << 4),
  mrlrDisk = (1 << 5)
  };

enum MResLimitPolicyEnum { 
  mrlpNONE = 0, 
  mrlpAlways, 
  mrlpExtendedViolation, 
  mrlpBlockedWorkloadOnly };

enum MResLimitVActionEnum { 
  mrlaNONE = 0, 
  mrlaCancel, 
  mrlaRequeue, 
  mrlaSuspend };

enum MActivePolicyTypeEnum {
  mptNONE = 0,
  mptMaxJob,
  mptMaxNode,
  mptMaxPE,
  mptMaxProc,
  mptMaxPS,
  mptMaxWC,
  mptMaxMem,
  mptMinProc
  };

/* NAT RM interface definitions */

#define MNAT_GETLOADCMD  "/usr/bin/vmstat | /usr/bin/tail -1 | /bin/awk '{ print \\$16 }'"
#define MNAT_CANCELCMD   "/bin/kill"
#define MNAT_CANCELSIG   "-15"
#define MNAT_RCMD        "/usr/bin/ssh"

#ifndef mbool_t
# define mbool_t unsigned char
#endif /* mbool_t */

/* 64 bit values */

#define M64INTBITS   64
#define M64INTLBITS  6
#define M64INTSIZE   8
#define M64INTSHIFT  3
#define M64UINT4     unsigned int
#define M64UINT8     unsigned long

/* 32 bit values */

#define M32INTBITS   32
#define M32INTLBITS  5
#define M32INTSIZE   4
#define M32INTSHIFT  2
#define M32UINT4     unsigned long
#define M32UINT8     unsigned long long

#ifdef __M64
#define MINTBITS   64
#define MINTLBITS  6
#define MINTSIZE   8
#define MINTSHIFT  3

#define MUINT4     unsigned int
#define MUINT8     unsigned long
#else /* __M64 */
#define MINTBITS   32
#define MINTLBITS  5
#define MINTSIZE   4
#define MINTSHIFT  2

#define MUINT4     unsigned long
#define MUINT8     unsigned long long
#endif /* __M64 */

typedef struct {
  mbool_t Is64;
  int     INTBITS;
  int     INTLBITS;
  int     INTSIZE;
  int     INTSHIFT;
  } m64_t;

#define MMOVEPTR(SPTR,DPTR)     \
  DPTR = SPTR;SPTR = (char *)0;

#define MAX_MNAME            64
#define MMAX_NAME            64

#ifndef mulong
# define mulong unsigned long
#endif /* mulong */

#ifndef mutime
# define mutime unsigned long
#endif /* mutime */

#ifndef MAX_MPAR
# define MAX_MPAR             4
#endif /* MAX_MPAR */

#define MMAX_PAR  4

#define MAX_MPALSIZE          1

#ifndef MAX_MUSER
# define MAX_MUSER          1024
# define MMAX_USER          1024
#endif /* MAX_MUSER */

#ifndef MAX_MGROUP
# define MAX_MGROUP         1024
#endif /* MAX_MGROUP */

#ifndef MAX_MACCT
# define MAX_MACCT          1024
#endif /* MAX_MACCT */

#ifndef MAX_MINDEX
# define MAX_MINDEX         128
#endif /* MAX_MINDEX */

#ifndef MAX_MQOS
# define MAX_MQOS           128
#endif /* MAX_MQOS */

#define MMAX_QOS  128

#define MAX_MQALSIZE          5

#ifndef MAX_MATTR
# define MAX_MATTR          128
#endif /* MAX_MATTR */

#ifndef MAX_MRES
# define MAX_MRES          1024
#endif /* MAX_MRES */

#ifndef MMAX_RSV
# define MMAX_RSV MAX_MRES
#endif

#ifndef MAX_MFSDEPTH
# define MAX_MFSDEPTH        24
#endif /* MAX_MFSDEPTH */

#define MAX_MPOLICY 9

typedef struct {
  int Usage[MAX_MPOLICY][MAX_MPAR];
  int SLimit[MAX_MPOLICY][MAX_MPAR];
  int HLimit[MAX_MPOLICY][MAX_MPAR];
  } mpu_t;

typedef struct {
  mpu_t   AP;
  mpu_t  *IP;
  mpu_t  *JP;

  mpu_t  *OAP;              /* override active usage policies  */
  mpu_t  *OIP;              /* override idle usage policies    */
  mpu_t  *OJP;

  mpu_t  *APU;              /* active policy (per user) */
  mpu_t  *APC;              /* active policy (per class) */
  mpu_t  *APG;              /* active policy (per group) */
  mpu_t  *APQ;              /* active policy (per QOS) */

  void   *JDef;             /* job defaults */
  void   *JMax;             /* job maximums */
  void   *JMin;             /* job minimums */
  } mcredl_t;

typedef struct {
  unsigned long JobFlags;           /* default job flags associated w/cred     */

  long   Priority;
  char   IsLocalPriority;           /* priority is locally specified (boolean) */

  long   Overrun;                   /* duration job may overrun wclimit        */

  void  *ADef;                      /* default account                         */
  int   *AAL;                       /* account access list                     */

  void  *QDef;                      /* default QOS                             */
  int    QAL[MAX_MQALSIZE];         /* QOS access list                         */
  int    QALType;

  void  *PDef;                      /* default partition                       */
  int    PAL[MAX_MPALSIZE];
  int    PALType;

  double FSTarget;                  /* credential fs usage target              */
  int    FSMode;                    /* fs target type                          */

  double FSFactor;                  /* effective decayed fs factor             */
  double FSUsage[MAX_MFSDEPTH];     /* FS Usage History                        */

  int    IsInitialized;             /* (boolean) */
  } mfs_t;

#define DEFAULT_MRESOURCELIMITPOLICY            mrlpNONE
#define DEFAULT_MRESOURCELIMITRESOURCES         mrlrProcs|mrlrMem
#define DEFAULT_MRESOURCELIMITVIOLATIONACTION   mrlaRequeue

#define MDEF_SPVJOBISPREEMPTIBLE                TRUE

/* node access policy */

#define MDEF_GNNAME  "GLOBAL"

enum MNodeAccessPolicyEnum {
  mnacNONE = 0,      /* DEFAULT:  shared */
  mnacShared,        /* any combination of workload allowed */
  mnacSingleJob,     /* peer tasks from a single job may utilize node */
  mnacSingleTask,    /* only a single task may utilize node */
  mnacSingleUser     /* any number of tasks from the same user may utilize node */
  };

#define DEFAULT_MNACCESSPOLICY mnacShared


enum MJobNameEnum {
  mjnNONE = 0,
  mjnFullName,
  mjnRMName,
  mjnShortName,
  mjnSpecName };

enum MJobDependEnum {
  mjdNONE = 0,
  mjdJobStart,
  mjdJobCompletion,
  mjdJobSuccessfulCompletion,
  mjdJobFailedCompletion };

typedef struct mjdepend_s {
  enum MJobDependEnum Type;
  char               *Value;
  struct mjdepend_s  *Next;
  } mjdepend_t;


/* reservation types */

enum MResType {
  mrtNONE = 0,
  mrtJob,
  mrtUser,
  mrtMeta };

enum MJResSubTypeEnum {
  mjrNone = 0,
  mjrActiveJob,
  mjrPriority,
  mjrQOSReserved,
  mjrDeadline,
  mjrMeta,
  mjrUser };

/* hold reasons */

enum MHoldReasonEnum {
  mhrNONE = 0,
  mhrAdmin,
  mhrNoResources,
  mhrSystemLimits,
  mhrAMFailure,
  mhrNoFunds,
  mhrAPIFailure,
  mhrRMReject,
  mhrPolicyViolation,
  mhrQOSAccess };

/* node states */

enum MNodeStateEnum {
  mnsNONE = 0,  /* not set */
  mnsNone,      /* set to 'none' by RM */
  mnsDown,
  mnsIdle,
  mnsBusy,
  mnsActive,     /* node is executing workload */
  mnsDrained,
  mnsDraining,
  mnsFlush,
  mnsReserved,   /* node is reserved (internal) */
  mnsUnknown };  /* node is up, usage must be determined */

#define DEFAULT_MCSALGO   mcsaDES

/* scheduler services */

enum MSvcEnum {
  svcNONE = 0,
  svcClusterShow,
  svcSetJobSystemPrio,
  svcSetJobUserPrio,
  svcShowQ,
  svcSetJobHold,
  svcReleaseJobHold,
  svcShowJobHold,
  svcShowStats,
  svcResetStats,
  svcResCreate,
  svcResDestroy,
  svcResShow,
  svcSched,
  svcDiagnose,
  svcSetJobDeadline,
  svcReleaseJobDeadline,
  svcShowJobDeadline,
  svcShowEarliestDeadline,
  svcSetJobQOS,
  svcShowGrid,
  svcShowBackfillWindow,
  svcShowConfig,
  svcJobShow,
  svcNodeShow,
  svcRunJob,
  svcCancelJob,
  svcChangeParameter,
  svcMigrateJob,
  svcShowEstimatedStartTime,
  svcBNFQuery,
  svcMJobCtl,
  svcMGridCtl,
  svcMNodeCtl,
  svcMResCtl,
  svcMSchedCtl,
  svcMStat,
  svcMDiagnose,
  svcMShow,
  svcMBal };


typedef struct {
  char  Name[MMAX_NAME];
  char  Host[MMAX_NAME];        /* ??? */
  char  RemoteHost[MMAX_NAME];
  int   RemotePort;
  char *URI;

  long  Flags;
  mulong Timeout;

  enum MSvcEnum SIndex;         /* requested service type            */

  int   SvcType;                /* requested service type            */
  long  StatusCode;             /* return code of service request    */
  char *RID;                    /* requestor UID                     */

  /* base config */

  long  Version;
  int   sd;
  char *RBuffer;               
  long  RBufSize;             
  char *SBuffer;             

  mulong State;

  mbool_t IsLoaded;        
  mbool_t IsNonBlocking;
  mbool_t SBIsDynamic;     

  long  SBufSize;     
  char *RPtr;      
  char *SPtr;     

  void *SE;                     /* xml to send - full (alloc,w/header) */
  void *SDE;                    /* xml to send (alloc,optional) */

  char *SData;                  /* text to send (alloc,optional) */

  char *SMsg;                   /* (alloc) */

  char *ClientName;             /* (alloc) */
  void *Cred;                   /* possibly a suclcred_t (ptr) */

  void *RE;                     /* incoming message - full (alloc) */
  void *RDE;                    /* incoming message (alloc) */

  /* comm config */

  enum MWireProtocolEnum   WireProtocol;
  enum MSocketProtocolEnum SocketProtocol;
  enum MChecksumAlgoEnum   CSAlgo;

  mbool_t                  DoEncrypt;

  char  CSKey[MMAX_NAME];
  } msocket_t;


/* peer service interface */

typedef struct {
  enum MPeerServiceEnum    Type;

  enum MWireProtocolEnum   WireProtocol;
  enum MSocketProtocolEnum SocketProtocol;
  enum MChecksumAlgoEnum   CSAlgo;

  char   *CSKey;

  char   *HostName;
  int     Port;
  char   *Version;

  mulong  Timeout;

  void   *Data;

  msocket_t *S;
  } mpsi_t;

/* NOTE:  sync with MXO[] */

enum MXMLOType {
  mxoNONE = 0,
  mxoAcct,
  mxoAM,
  mxoClass,
  mxoCluster,
  mxoCP,
  mxoFrame,
  mxoFS,
  mxoGroup,
  mxoJob,
  mxoLimits,
  mxoNode,
  mxoPar,
  mxoPriority,
  mxoQOS,
  mxoQueue,
  mxoRange,
  mxoReq,
  mxoRsv,
  mxoRM,
  mxoSched,
  mxoSim,
  mxoSRes,
  mxoStats,
  mxoSys,
  mxoUser,
  mxoLAST };

enum MRMStateEnum {
  mrmsNONE = 0,
  mrmsActive,
  mrmsDown,
  mrmsCorrupt };

enum MRMTypeEnum {
  mrmtNONE = 0,
  mrmtLL,
  mrmtOther,
  mrmtPBS,
  mrmtSGE,
  mrmtSSS,
  mrmtWiki,
  mrmtRMS,
  mrmtLSF,
  mrmtNative };

#define MAX_MRMTYPE 10

enum MJobSearchEnum {
  mjsmBasic = 0,
  mjsmExtended,
  mjsmInternal };

enum MRMSubTypeEnum {
  mrmstNONE = 0,
  mrmstRMS };

#ifndef __M_H
#define __M_H
#endif /* !__M_H */

#ifdef __M_H
#include "msched.h"
#endif /* __M_H */

#if defined (__MMEMTEST)
#ifndef MEMWATCH
# define MEMWATCH
# include "memwatch.h"
#endif /* MEMWATCH */

#ifndef MEMWATCH_STDIO
# define MEMWATCH_STDIO
#endif /* MEMWATCH_STDIO */
#endif /* __MMEMTEST */

#ifndef __M_H
#define __M_H
#endif /* !__M_H */

#ifndef mulong
# define mulong unsigned long
#endif /* mulong */

#ifndef mutime
# define mutime unsigned long
#endif /* mutime */

#if !defined(DEFAULT_MSERVERHOST)
# define DEFAULT_MSERVERHOST              ""
#endif /* !DEFAULT_MSERVERHOST */

#if !defined(DEFAULT_MSERVERPORT)
# define DEFAULT_MSERVERPORT              40559
#endif /* !DEFAULT_MSERVERPORT */

#define DEFAULT_MSERVERMODE               msmNormal

#define DEFAULT_MREJECTNEGPRIOJOBS             FALSE
#define DEFAULT_MENABLENEGJOBPRIORITY          FALSE
#define DEFAULT_MENABLEMULTINODEJOBS           TRUE
#define DEFAULT_MENABLEMULTIREQJOBS            FALSE

#define MAX_MSPSLOT  16

/* scheduling policies */

enum MAllocLocalityPolicyEnum {
  malpNONE = 0,
  malpBestEffort,
  malpForce,
  malpRMSpecific };

enum MLimitTypes {
  mlNONE = 0,
  mlActive,
  mlIdle,
  mlSystem };

enum MJobNonEType {
  mjneNONE = 0,
  mjneIdlePolicy };

enum MEventFlagsEnum {
  mefNONE = 0,
  mefExternal };

enum MTimePolicyEnum {
  mtpNONE = 0,
  mtpReal };


/* server checkpointing object */

typedef struct {
  char  CPFileName[MAX_MLINE];

  char *Buffer;
 
  int   CPInterval;         /* seconds between subsequent checkpoints          */
  int   CPExpirationTime;   /* seconds stale checkpoint data will be kept      */
  mulong LastCPTime;        /* time of most recent checkpoint                  */
 
  char  DVersion[MAX_MNAME];      /* detected data version */
  char  SVersionList[MAX_MLINE];  /* supported versions    */
  char  WVersion[MAX_MNAME];      /* write version         */
 
  FILE *fp;
 
  char *OBuffer;
  } mckpt_t;

/* CP types */

enum MCkPtTypeEnum {
  mcpSched = 0,
  mcpJob,
  mcpRes,
  mcpSRes,
  mcpNode,
  mcpUser,
  mcpGroup,
  mcpAcct,
  mcpTotal,
  mcpRTotal,
  mcpCTotal,
  mcpGTotal,
  mcpSysStats,
  mcpRM,
  mcpAM,
  mcpSys };



/* general objects */

/* reasons for requirements rejection */

/* sync with MAllocRejType */

enum MAllocRejEnum {
  marFeatures = 0,
  marClass,
  marPartition,
  marCPU,
  marMemory,
  marDisk,
  marSwap,
  marAdapter,
  marState,
  marEState,
  marOpsys,
  marArch,
  marRelease,
  marTime,
  marNodeCount,
  marHold,
  marPolicy,
  marLocality,
  marDepend,
  marShortPool,
  marSystemLimits,
  marPartitionAccess,
  marCorruption,
  marFairShare,
  marHostList,
  marPool,
  marPriority };

#define MAX_MREJREASON       27  /* NOTE:  sync with MRejReasonEnum */


enum MAccessModeEnum {
  macmNONE = 0,
  macmRead,
  macmWrite };

/* BNF commands */

enum {
  bnfNone = 0,
  bnfQuery,
  bnfStatus,
  bnfMessage,
  bnfSet
  };

enum MStatusCodeEnum {
  mscNoError = 0,
  mscFuncFailure,
  mscBadParam,      /* invalid parameters */
  mscNoAuth,        /* request not authorized */
  mscNoEnt,         /* entity does not exist */
  mscNoFile,        /* file does not exist */
  mscNoMemory,      /* inadequate memory */
  mscRemoteFailure, /* remote service failed */
  mscSysFailure,    /* system call failed */
  mscTimeout,
  mscBadRequest,    /* request is corrupt */
  mscNoData,        /* data not yet available */
  mscLAST };

/* cred object */

enum MCredAttrType {
  mcaNONE,
  mcaPriority,
  mcaMaxJob,
  mcaMaxNode,
  mcaMaxPE,
  mcaMaxProc,
  mcaMinProc,
  mcaMaxPS,
  mcaMaxWC,        /* max total wclimit */
  mcaMaxMem,
  mcaMaxIJob,
  mcaMaxINode,
  mcaMaxIPE,
  mcaMaxIProc,
  mcaMaxIPS,
  mcaMaxIWC,
  mcaMaxIMem,
  mcaOMaxJob,
  mcaOMaxNode,
  mcaOMaxPE,
  mcaOMaxProc,
  mcaOMaxPS,
  mcaOMaxWC,
  mcaOMaxMem,
  mcaOMaxIJob,
  mcaOMaxINode,
  mcaOMaxIPE,
  mcaOMaxIProc,
  mcaOMaxIPS,
  mcaOMaxIWC,
  mcaOMaxIMem,
  mcaOMaxJNode,
  mcaOMaxJPE,
  mcaOMaxJProc,
  mcaOMaxJPS,
  mcaOMaxJWC,
  mcaOMaxJMem,
  mcaFSTarget,
  mcaQList,
  mcaQDef,
  mcaPList, 
  mcaPDef,
  mcaAList,
  mcaADef,
  mcaJobFlags,
  mcaMaxJobPerUser,
  mcaMaxNodePerUser,
  mcaMaxProcPerUser,
  mcaMaxProcPerNodePerQueue,
  mcaOverrun,
  mcaID,
  mcaDefWCLimit,
  mcaMaxWCLimit,      /* max wclimit per job */
  mcaMaxProcPerNode,
  mcaMaxNodePerJob,
  mcaMaxProcPerJob
  };

enum MOServiceAttrType {
  mosaNONE = 0,
  mosaCSAlgo,
  mosaCSKey,
  mosaHost,
  mosaPort,
  mosaProtocol,
  mosaVersion };


/* group object */

enum MGroupAttrType {
  mgaNONE = 0,
  mgaClassWeight };

/* user object */

enum MUserAttrType {
  muaNONE = 0,
  muaMaxWCLimit };

/* qos object */

enum MQOSAttrType {
  mqaNONE = 0,
  mqaPriority,
  mqaMaxJob,
  mqaMaxProc,
  mqaMaxNode,
  mqaXFWeight,
  mqaQTWeight,
  mqaXFTarget,
  mqaQTTarget,
  mqaFlags,
  mqaFSTarget };


enum MQOSFlagEnum {
  mqfignJobPerUser = 0,
  mqfignProcPerUser,
  mqfignNodePerUser,
  mqfignPSPerUser,
  mqfignJobQueuedPerUser,
  mqfignMaxProc,
  mqfignMaxTime,
  mqfignMaxPS,
  mqfignSRMaxTime,
  mqfignUser,
  mqfignSystem,
  mqfignAll,
  mqfpreemptor,
  mqfpreemptee,
  mqfdedicated,
  mqfreserved,
  mqfusereservation,
  mqfnobf,
  mqfnoreservation,
  mqfrestartpreempt,
  mqfNTR,
  mqfRunNow,
  mqfPreemptSPV,
  mqfignHostList };

#define QFUSER    ((1 << mqfignJobPerUser) | (1 << mqfignNodePerUser) \
		 | (1 << mqfignPSPerUser) | (1 << mqfignJobQueuedPerUser))
#define QFSYSTEM  ((1 << mqfignMaxProc) | (1 << mqfignMaxTime) | (1 << mqfignMaxPS))
#define QFALL     (QFUSER | QFSYSTEM | (1 << mqfignSRMaxTime))



#define MUBMCheck(I,M) (M[(I) >> 5] & (1 << ((I) % 32)))
#define MUBMSet(I,M) ((M[(I) >> 5]) |= (1 << ((I) % 32)))
#define MUBMClear(M,S) memset(M,0,sizeof(int) * ((S >> 5) + 1))
#define MUBMCopy(Dst,Src,S) memcpy(Dst,Src,sizeof(int) * ((S >> 5) + 1))



/* class object */

enum MClassAttrType {
  mclaNONE = 0,
  mclaOCNode,
  mclaDefReqFeature,
  mclaHostList,
  mclaName,
  mclaNAPolicy,
  mclaMaxProcPerNode,
  mclaOCDProcFactor,
  mclaState,
  mclaWCOverrun
  };


/* res object */

enum MResAttrEnum {
  mraNONE = 0,
  mraAAccount,  /* accountable account */
  mraACL,
  mraAGroup,    /* accountable group */
  mraAUser,     /* accountable user */
  mraCreds,
  mraDuration,
  mraEndTime,
  mraFlags,
  mraHostExp,
  mraJState,
  mraMaxTasks,
  mraMessages,
  mraName,
  mraNodeCount,
  mraNodeList,
  mraOwner,
  mraResources,
  mraStartTime,
  mraStatCAPS,
  mraStatCIPS,
  mraStatTAPS,
  mraStatTIPS,
  mraTaskCount,
  mraType,
  mraXAttr };

/* nres object */

enum MNResAttrType {
  mnraNONE = 0,
  mnraDRes,
  mnraEnd,
  mnraName,
  mnraState,
  mnraStart,
  mnraTC,
  mnraType
  };

/* NOTE:  sync with external systems */

enum MResFlagEnum {
  mrfNONE = 0,
  mrfStandingRes,
  mrfSingleUse,
  mrfByName,
  mrfMeta,
  mrfPreemptible,
  mrfTimeFlex,
  mrfSpaceFlex,
  mrfDedicatedNode,     /* only on active reservation on node */
  mrfDedicatedResource, /* reservation does not share reserved resources */
  mrfAdvRes,            /* may only utilize reserved resources */
  mrfForce,             /* force res onto nodes regardless of other res */
  mrfOwnerPreempt };

/* stat object */

enum MStatAttrType {
  mstaNONE = 0,
  mstaTJobCount,
  mstaTNJobCount,
  mstaTQueueTime,
  mstaMQueueTime,
  mstaTReqWTime,
  mstaTExeWTime,
  mstaTMSAvl,
  mstaTMSDed,
  mstaTPSReq,
  mstaTPSExe,
  mstaTPSDed,
  mstaTPSUtl,
  mstaTJobAcc,
  mstaTNJobAcc,
  mstaTXF,
  mstaTNXF,
  mstaMXF,
  mstaTBypass,
  mstaMBypass,
  mstaTQOSAchieved,
  mstaInitTime,  
  mstaGCEJobs,      /* current eligible jobs */
  mstaGCIJobs,      /* current idle jobs */
  mstaGCAJobs,      /* current active jobs */
  mstaGPHAvl,
  mstaGPHUtl,
  mstaGPHSuc,
  mstaGMinEff,
  mstaGMinEffIteration,
  mstaTPHPreempt,
  mstaTJPreempt,
  mstaTJEval,
  mstaTPHQueued, 
  mstaSchedDuration,
  mstaSchedCount };

#define MMAX_JOBATTR 4

typedef struct
  {
  int             AIndex;   /* generic attribute index */
  enum MXMLOType  OType;    /* object type */
  char           *AName[MMAX_JOBATTR]; /* XML attribute name */
  } mobjattr_t;


/* limit object */

enum MLimitAttrType {
  mlaNONE = 0,
  mlaAJobs,
  mlaAProcs,
  mlaAPS
  };


/* rm object */

enum MRMAttrType {
  mrmaNONE = 0,
  mrmaAuthType,
  mrmaConfigFile,
  mrmaCSAlgo,
  mrmaCSKey,
  mrmaEPort,
  mrmaHost,
  mrmaLocalDiskFS,
  mrmaMinETime,
  mrmaName,
  mrmaNMPort,
  mrmaNMServer,
  mrmaPort,
  mrmaSocketProtocol,
  mrmaSuspendSig,
  mrmaTimeout,
  mrmaType,
  mrmaVersion,
  mrmaWireProtocol 
  };

typedef struct
  {
  int   AIndex;
  int   OType;
  char *S02;    /* SSS RM0.2 */
  char *Other;
  } mjobattr_t;


enum {
  mjfNONE = 0,
  mjfAllocLocal,
  mjfBackfill,
  mjfSpan,
  mjfAdvReservation,  
  mjfNoQueue,        
  mjfHostList,
  mjfResMap,         
  mjfSharedResource,  
  mjfByName,
  mjfBestEffort,
  mjfRestartable,
  mjfPreemptee,
  mjfPreemptor,
  mjfNASingleJob,    
  mjfPreload,
  mjfRemote,       
  mjfNASingleTask,
  mjfSPViolation,   
  mjfIgnNodePolicies,
  mjfNoRMStart,
  mjfGlobalQueue,
  mjfIsExiting };

enum {
  mjifNONE = 0,
  mjifIsExiting };

enum MHoldTypeEnum { 
  mhNONE = 0,
  mhUser, 
  mhSystem, 
  mhBatch, 
  mhDefer, 
  mhAll };

/* sync w/MRMFuncType */

enum MRMFuncEnum {
  mrmNONE = 0,
  mrmClusterQuery,
  mrmCycleFinalize,
  mrmJobCancel,
  mrmJobCheckpoint,
  mrmJobGetProximateTasks,
  mrmJobMigrate,
  mrmJobModify,
  mrmJobQuery,
  mrmJobRequeue,
  mrmJobResume,
  mrmJobStart,
  mrmJobSubmit,
  mrmJobSuspend,
  mrmQueueQuery,
  mrmResourceModify,
  mrmResourceQuery,
  mrmRMEventQuery,
  mrmRMGetData,
  mrmRMInitialize,
  mrmRMQuery,
  mrmWorkloadQuery };

typedef struct {
  int (*ClusterQuery)(mrm_t *, int *,char *,int *);
  int (*CycleFinalize)(mrm_t *,int *);
  int (*Initialize)();
  int (*JobCancel)(mjob_t *,mrm_t *,char *,char *,int *);
  int (*JobCheckpoint)(mjob_t *, mrm_t *,mbool_t,char *,int *);
  int (*JobGetProximateTasks)(mjob_t *,mrm_t *,mnodelist_t,mnodelist_t,long,int,char *,int *);
  int (*JobMigrate)(mjob_t *,mrm_t *,mnalloc_t *,char *,int *);
  int (*JobModify)(mjob_t *,mrm_t *,char *,char *,char *,char *,int *);
  int (*JobQuery)();
  int (*JobRequeue)(mjob_t *,mrm_t *,mjob_t **,char *,int *);
  int (*JobResume)(mjob_t *,mrm_t *,char *,int *);
  int (*JobStart)(mjob_t *, mrm_t *, char *, int *);
  int (*JobSubmit)(char *,mrm_t *,mjob_t **,char *,char *,int *);
  int (*JobSuspend)(mjob_t *,mrm_t *,char *,int *);
  int (*QueueQuery)(mrm_t *, int *, int *);
  int (*ResourceModify)();
  int (*ResourceQuery)(mnode_t *,mrm_t *,char *,int *);
  int (*RMEventQuery)(mrm_t *,int *);
  int (*RMGetData)(mrm_t *,int *);
  int (*RMInitialize)(mrm_t *, int *);
  int (*RMQuery)(void);
  int (*WorkloadQuery)(mrm_t *, int *, int *);

  mbool_t IsInitialized;
  } mrmfunc_t;

#define DEFAULT_MRMPORT                        0
#define DEFAULT_MRMSERVER                      ""
#define DEFAULT_MRMTYPE                        mrmtPBS
#define DEFAULT_MRMTIMEOUT                     9
#define DEFAULT_MRMAUTHTYPE                    rmaCheckSum

#define MMAX_AMFUNC  20

/* am object */

enum MJFActionEnum {
  mamjfaNONE = 0,
  mamjfaCancel,
  mamjfaDefer };

#define MDEF_AMJFACTION                mamjfaNONE

typedef struct {
  char  Name[MAX_MNAME];
  int   Index;

  enum MRMStateEnum State;

  char  ClientName[MAX_MNAME];

  /* interface specification */

  enum MWireProtocolEnum   WireProtocol;
  enum MSocketProtocolEnum SocketProtocol;

  enum MChecksumAlgoEnum   CSAlgo;

  char  CSKey[MAX_MNAME];
 
  char  Host[MAX_MNAME];          /* active AM server host                           */
  int   Port;                     /* active AM service port                          */

  char  SpecHost[MAX_MNAME];      /* specified AM server host                        */
  int   SpecPort;                 /* specified AM service port                       */
  
  int   UseDirectoryService;      /* boolean */

  int   Type;                     /* type of AM server                               */
  int   Version;

  int   Timeout;                  /* in ms */

  /* policies */

  int   ChargePolicy;             /* allocation charge policy                        */

  mulong FlushInterval;           /* AM flush interval                               */
  mulong FlushTime;

  int   DeferJobOnFailure;        /* boolean */
  int   AppendMachineName;        /* boolean */

  char  FallbackAccount[MAX_MNAME]; /* account to use if primary account is unavailable */

  long   FailTime[MAX_MRMFAILURE];
  int    FailType[MAX_MRMFAILURE];
  char  *FailMsg[MAX_MRMFAILURE];
  int    FailIndex;

  long   RespTotalTime[MMAX_AMFUNC];
  long   RespMaxTime[MMAX_AMFUNC];
  int    RespTotalCount[MMAX_AMFUNC];
  long   RespStartTime[MMAX_AMFUNC];

  msocket_t *S;
  FILE      *FP;

  mpsi_t     P;

  enum MJFActionEnum JFAction;
  } mam_t;

enum MAMAttrType {
  mamaNONE = 0,
  mamaAppendMachineName,
  mamaChargePolicy,
  mamaConfigFile,
  mamaCSAlgo,
  mamaCSKey,
  mamaDeferJobOnFailure,
  mamaFallbackAccount,
  mamaFlushInterval,
  mamaHost,
  mamaJFAction,
  mamaPort,
  mamaServer,
  mamaSocketProtocol,
  mamaTimeout,
  mamaType,
  mamaWireProtocol 
  };

/* AM types */

enum {
  mamtNONE,
  mamtQBANK,
  mamtRESD,
  mamtFILE,
  mamtGOLD
  };

/* AM consumption policies */

enum {
  mamcpNONE = 0,
  mamcpDebitAllWC,
  mamcpDebitAllCPU,
  mamcpDebitAllPE,
  mamcpDebitSuccessfulWC,
  mamcpDebitSuccessfulCPU,
  mamcpDebitSuccessfulPE
  };

#define DEFAULT_MAMTYPE                        mamtNONE
#define DEFAULT_MAMVERSION                    0
#define DEFAULT_MAMCHARGEPOLICY                mamcpDebitSuccessfulWC
#define DEFAULT_MAMHOST                        ""
#define DEFAULT_MAMPORT                   40560
#define MAX_MAMFLUSHINTERVAL              86400
#define DEFAULT_MAMFLUSHINTERVAL           3600
#define DEFAULT_MAMAUTHTYPE                    rmaCheckSum
#define DEFAULT_MAMWIREPROTOCOL                mwpAVP
#define DEFAULT_MAMSOCKETPROTOCOL              mspSingleUseTCP

#define DEFAULT_MAMDEFERONJOBFAILURE           FALSE
#define DEFAULT_MAMAPPENDMACHINENAME           FALSE

#define DEFAULT_MAMTIMEOUT                    9

#define MMAX_PRIO_VAL                      1000000000

#define MDEF_AMTYPE                        mamtNONE
#define MDEF_AMVERSION                    0
#define MDEF_AMCHARGEPOLICY                mamcpDebitSuccessfulWC
#define MDEF_AMHOST                        ""
#define MDEF_AMPORT                   40560
#define MMAX_AMFLUSHINTERVAL              86400
#define MDEF_AMFLUSHINTERVAL           3600
#define MDEF_AMAUTHTYPE                    rmaCheckSum
#define MDEF_AMWIREPROTOCOL                mwpAVP
#define MDEF_AMSOCKETPROTOCOL              mspSingleUseTCP

#define MDEF_AMDEFERONJOBFAILURE           FALSE
#define MDEF_AMAPPENDMACHINENAME           FALSE

#define MDEF_AMTIMEOUT                    9

#define MCONST_EFFINF                          50000000   /* ~ 1.5 years */


/* sim object */

/* sim flag types */

enum {
  msimfNONE = 0,
  msimfIgnHostList,
  msimfIgnClass,
  msimfIgnQOS,
  msimfIgnMode,
  msimfIgnFeatures,
  msimfIgnFrame,
  msimfIgnAll };

typedef struct {
  char  WorkloadTraceFile[MAX_MLINE + 1]; /* File Containing Workload Traces  */
  char  ResourceTraceFile[MAX_MLINE + 1]; /* File Containing Resource Traces  */

  /* general config */

  int   SimulationInterval;       /* Time Step Used in Simulation             */
  int   WCScalingPercent;         /* Mult. Factor for WC Limit                */
  int   InitialQueueDepth;        /* depth of queue at start time             */
  double WCAccuracy;              /* Accuracy of User Specified Limits        */
  double WCAccuracyChange;        /* Percent Offset to Original WC Accuracy   */
  int   JobSubmissionPolicy;      /* Job submission policy                    */
  int   LocalityMargin;           /* number of frames allowed beyond optimal  */
  int   NCPolicy;                 /* node config policy                       */
  int   NodeCount;                /* Number of Nodes in Simulated System      */
  char  StatFileName[MAX_MLINE];  /* Name of simulation statistics file       */
  mulong DefaultCheckpointInterval;

  mulong StartTime;               /* epoch time to start simulation           */
  int   StopIteration;            /* iteration to schedule before stop        */
  int   ExitIteration;            /* iteration to schedule before exit        */

  long  Flags;
  long  TraceIgnoreJobFlags;
  long  TraceDefaultJobFlags;

  /* config booleans */

  int   ScaleJobRunTime;          /* scale job execution time                 */
  int   AutoShutdown;             /* shutdown when queues are empty           */      
  int   ForceNodeLocality;        /* require node localization                */
  int   RandomizeJobSize;         /* randomize job size distribution          */

  /* general status */

  long  RMFailureTime;
  int   TraceOffset;              /* offset to next trace in tracefile        */
  
  /* status booleans */

  int   QueueChanged;

  /* IPC cost analysis */

  int   CommunicationType;        /* Communication Pattern being Simulated    */
  double IntraFrameCost;          /* Cost of IntraFrame Communication         */
  double InterFrameCost;          /* Cost of InterFrame Communication         */
  double ComRate;                 /* Percent of Instructions that are Comm    */
  } msim_t;

enum MSimNodeConfigPolicyEnum { 
  msncNormal = 0,
  msncHomogeneous,
  msncPartitioned };

enum MSimJobSubmissionPolicyEnum { 
  msjsTraceSubmit = 0,
  msjsConstantJob, 
  msjsConstantPS };

/* default sim values */

#define DEFAULT_MSIMWCSCALINGPERCENT        100
#define DEFAULT_MSIMINITIALQUEUEDEPTH        16
#define DEFAULT_MSIMWCACCURACY              0.0  /* 0 to use trace execution time */
#define DEFAULT_MSIMWCACCURACYCHANGE        0.0
#define DEFAULT_MSIMFLAGS                     0
#define DEFAULT_MSIMTRACEIGNFLAGS              (1 << mjfAllocLocal)
#define DEFAULT_MSIMSTATFILENAME               "simstat.out"
#define DEFAULT_MSIMJSPOLICY                   msjsConstantJob
#define DEFAULT_MSIMNCPOLICY                   msncNormal
#define DEFAULT_MSIMNODECOUNT                 0                 /* 0 Means Use Trace Node Count */
#define DEFAULT_MSCHEDALLOCLOCALITYPOLICY      malpNONE

#define DEFAULT_MSIMCOMMUNICATIONTYPE          comRoundRobin
#define DEFAULT_MSIMINTRAFRAMECOST          0.3
#define DEFAULT_MSIMINTERFRAMECOST          0.3
#define DEFAULT_MSIMCOMRATE                 0.1

#define DEFAULT_MSIMRANDOMIZEJOBSIZE           FALSE
#define DEFAULT_MSIMAUTOSHUTDOWNMODE           TRUE

#define DEFAULT_MSIMWORKLOADTRACEFILE          "workload"
#define DEFAULT_MSIMRESOURCETRACEFILE          "resource"

#define MSCHED_KEYFILE                         ".moab.key"


/* sr object */

enum MSRAttrType {
  msraNone,
  msraAccess,
  msraAccountList,
  msraChargeAccount,
  msraClassList,
  msraDays,
  msraDepth,
  msraEndTime,
  msraFlags,
  msraGroupList,
  msraHostList,
  msraIdleTime,
  msraJobAttrList,
  msraMaxTime,
  msraName,
  msraNodeFeatures,
  msraOwner,
  msraPartition,
  msraPeriod,
  msraPriority,
  msraProcLimit,
  msraQOSList,
  msraResources,
  msraStartTime,
  msraStIdleTime,
  msraStTotalTime,
  msraTaskCount,
  msraTaskLimit,
  msraTimeLimit,
  msraTPN,
  msraUserList,
  msraWEndTime,
  msraWStartTime };



/* fs object */

enum MFSAttrType {
  mfsaNONE = 0,
  mfsaTarget
  };



/* node object */

enum MNodeAttrEnum {
  mnaNONE = 0,
  mnaAccess,
  mnaArch,
  mnaGRes,
  mnaAvlClass,
  mnaAvlMemW,
  mnaAvlProcW,
  mnaCfgClass,
  mnaCfgDisk,
  mnaCfgMem,
  mnaCfgMemW,
  mnaCfgProcW,
  mnaCfgSwap,
  mnaExtLoad,
  mnaFeatures,
  mnaFrame,
  mnaLoad,
  mnaLoadW,
  mnaMaxJob,
  mnaMaxJobPerUser,
  mnaMaxLoad,
  mnaMaxPEPerJob,
  mnaMaxProc,
  mnaMaxProcPerClass,
  mnaMaxProcPerUser,
  mnaNetwork,
  mnaNodeID,
  mnaNodeState,
  mnaNodeType,
  mnaOS,
  mnaPartition,
  mnaPrioF,
  mnaPriority,
  mnaPrioW,
  mnaProcSpeed,
  mnaRADisk,
  mnaRAMem,
  mnaRAProc,
  mnaRASwap,
  mnaRCDisk,
  mnaRCMem,
  mnaRCProc,
  mnaRCSwap,
  mnaResource,
  mnaSize,
  mnaSlot,
  mnaSpeed,
  mnaSpeedW,
  mnaStatATime,
  mnaStatTTime,
  mnaStatUTime,
  mnaTaskCount,
  mnaUsageW
  };



enum MJobAttrEnum {
  mjaNONE = 0,
  mjaAccount,
  mjaAllocNodeList,
  mjaArgs,
  mjaAWDuration,  /* active wall time consumed */
  mjaCalendar,
  mjaCmdFile,
  mjaCompletionTime,
  mjaCPULimit,
  mjaDepend,
  mjaDRMJID,
  mjaEEWDuration, /* effective eligible wall duration:  duration job has been eligible for scheduling */
  mjaEnv,
  mjaExcHList,
  mjaExecutable,
  mjaFlags,
  mjaGAttr,
  mjaGroup,
  mjaHold,
  mjaHostList,
  mjaIsInteractive,
  mjaIsRestartable,
  mjaIsSuspendable,
  mjaIWD,
  mjaJobID,       /* job batch id */
  mjaJobName,     /* user specified job name */
  mjaMasterHost,
  mjaMessages,
  mjaNotification,
  mjaPAL,
  mjaPriority,
  mjaQOS,
  mjaQOSReq,
  mjaReqAWDuration,   /* req active walltime duration */
  mjaReqCMaxTime,     /* req latest allowed completion time */
  mjaReqNodes,
  mjaReqProcs,
  mjaReqReservation,  /* req reservation */
  mjaReqSMinTime,     /* req earliest start time */
  mjaRMJID,           /* RM job ID */
  mjaRMXString,       /* RM extension string */
  mjaRsvAccess,
  mjaSRMJID,
  mjaStartCount,
  mjaStartTime,       /* most recent time job started execution */
  mjaState,
  mjaStatMSUtl,
  mjaStatPSDed,
  mjaStatPSUtl,
  mjaStdErr,
  mjaStdIn,
  mjaStdOut,
  mjaStepID,
  mjaSubmitLanguage,
  mjaSubmitString,
  mjaSubmitTime,
  mjaSuspendDuration, /* duration job has been suspended */
  mjaSysPrio,
  mjaSysSMinTime,
  mjaUser,
  mjaUserPrio,
  mjaUtlMem,
  mjaUtlProcs };

enum MXAttrType {
  mxaNONE,
  mxaAdvRes,
  mxaDistPolicy,
  mxaDMem,
  mxaFlags,
  mxaGeometry,
  mxaHostList,
  mxaMasterFeature,
  mxaMasterMem,
  mxaNAccessPolicy,
  mxaNAllocPolicy,
  mxaNodeSet,
  mxaPMask,
  mxaPref,
  mxaQOS,
  mxaQueueJob,
  mxaSGE,
  mxaSID,
  mxaSJID,
  mxaTPN,
  mxaTRL };

enum MauiAppSimCommandEnum {
  mascNONE,
  mascConfig,
  mascCreate,
  mascDestroy,
  mascQuery,
  mascShow,
  mascUpdate
  };

enum MetaCtlCmdEnum {
  mcNONE = 0,
  mcInitialize,
  mcCommit,
  mcList,
  mcSet,
  mcRemove,
  mcQuery,
  mcSubmit,
  mcModify,
  mcResetStats,
  mcRegister };

enum MJobCtlCmdEnum {
  mjcmNONE = 0,
  mjcmCancel,
  mjcmCheckpoint,
  mjcmDiagnose,
  mjcmModify,
  mjcmQuery,
  mjcmRequeue,
  mjcmResume,
  mjcmShow,
  mjcmStart,
  mjcmSubmit,
  mjcmSuspend,
  mjcmTerminate };

enum MResCtlEnum {
  mrcmNONE = 0,
  mrcmCreate,
  mrcmDestroy,
  mrcmModify,
  mrcmQuery };

enum MBalEnum {
  mccmNONE = 0,
  mccmExecute,
  mccmQuery };


/* req object */

enum MReqAttrEnum {
  mrqaNONE = 0,
  mrqaAllocNodeList,
  mrqaReqArch,
  mrqaReqClass,
  mrqaReqDiskPerTask,
  mrqaReqMemPerTask,
  mrqaReqNodeDisk,
  mrqaReqNodeFeature,
  mrqaReqNodeMem,
  mrqaReqNodeProc,
  mrqaReqNodeSwap,
  mrqaReqOpsys,
  mrqaPartition,
  mrqaReqSwapPerTask,
  mrqaNCReqMax,
  mrqaNCReqMin,
  mrqaTCReqMax,
  mrqaTCReqMin,
  mrqaTPN };



/* sched object */

enum MSchedAttrEnum {
  msaNONE = 0,
  msaCPVersion,
  msaFBServer,
  msaHomeDir,
  msaMode,
  msaName,
  msaServer
  };

enum MClientAttrEnum {
  mcltaNONE = 0,
  mcltaCSAlgo,
  mcltaTimeout,
  mcltaFlags };

enum MWCVioActEnum {
  mwcvaNONE = 0,
  mwcvaCancel,
  mwcvaPreempt
  };

enum MPreemptPolicyEnum {
  mppNONE = 0,
  mppRequeue,
  mppSuspend,
  mppCheckpoint,
  mppOvercommit };

#define DEFAULT_MPREEMPTPOLICY    mppRequeue



/* sys object */

enum MSysAttrType {
  msysaNONE = 0,
  msysaPresentTime,
  msysaStatInitTime,
  msysaSyncTime,
  msysaVersion
  };


/* priority mode */

enum MJobPrioEnum {
  mjpNONE = 0,
  mjpRelative,
  mjpAbsolute };


/* checkpoint modes */

enum MCKPtModeEnum {
  mckptResOnly = 0,
  mckptNonRes
  };


/* cluster object */

enum MClusterAttrType {
  mcluaNONE = 0,
  mcluaMaxSlot,
  mcluaName,
  mcluaPresentTime
  };


/* frame object */

enum MFrameAttrType {
  mfraNONE = 0,
  mfraIndex,
  mfraName
  };

#define DEFAULT_MHSERVER        "supercluster.org"
#define DEFAULT_MHPORT          80
#define DEFAULT_MHSYNCLOCATION  "/maui/syncdir/326/update.html"

#define MAX_MCFG  512

typedef struct {
  char *Name;
  int   PIndex;
  int   Format;
  int   OType;
  void *OPtr;
  } mcfg_t;



/* node allocation policies */

enum MNodeAllocationPolicies {
  mnalNONE2 = 0,
  mnalFirstAvailable,
  mnalLastAvailable,
  mnalMinLocal,
  mnalMinGlobal,
  mnalMinResource,
  mnalMachinePrio,
  mnalCPULoad,
  mnalFirstSet,
  mnalLocal,
  mnalContiguous,
  mnalMaxBalance,
  mnalFastest
  };

#define DEFAULT_MNALLOCPOLICY                       mnalLastAvailable



/* RM objects */

#define MMIN_LLCFGDISK       1   /* (in MB) */



/* client object */

typedef struct {
  char  Name[MAX_MNAME];
  char  CSKey[MAX_MNAME];
  } mclient_t;

typedef struct {
  char  ServerHost[MAX_MNAME];
  int   ServerPort;
  long  Timeout;

  int   Format;

  int   SocketProtocol;

  char  ServerCSKey[MAX_MNAME];

  enum MChecksumAlgoEnum ServerCSAlgo;

  char  ConfigFile[MAX_MLINE];
  char  HomeDir[MAX_MLINE];

  char  SchedulerMode[MAX_MNAME];

  char  BuildDir[MAX_MLINE];
  char  BuildHost[MAX_MLINE];
  char  BuildDate[MAX_MLINE];

  long  DisplayFlags;
  } mccfg_t;



/* extension support */

typedef struct {
  /* base functions */

  int (*JobAllocateResources)(mjob_t *, mnodelist_t, char *, mnodelist_t, int, long int);
  int (*JobCheckPolicies)(mjob_t *,int,int,mpar_t *,int *,char *,long int);
  int (*JobDistributeTasks)(mjob_t *,mrm_t *,mnalloc_t *,short int *);
  int (*JobFind)(char *, mjob_t **, int);
  int (*JobGetFeasibleTasks)(mjob_t *,mreq_t *,mpar_t *,nodelist_t,nodelist_t,int *,int *,long int,long unsigned int);
  int (*JobGetStartPriority)(mjob_t *,int,double *,int,char *);
  int (*JobGetTasks)(mjob_t *,mpar_t *,nodelist_t,mnodelist_t,char *,int);
  int (*JobSetCreds)(mjob_t *, char *, char *, char *);
  int (*JobSetQOS)(mjob_t *, mqos_t *, int);
  int (*JobBuildACL)(mjob_t *);
  int (*JobBuildCL)(mjob_t *);
  char *(*JobNameAdjust)(mjob_t *,char *,mrm_t *,char *,int,enum MJobNameEnum);
  int (*JobStart)(mjob_t *);
  int (*JobValidate)(mjob_t *,char *,int);
  int (*JobDetermineCreds)(mjob_t *);
  int (*QOSGetAccess)(mjob_t *,mqos_t *,int *,mqos_t **);

  int (*QueuePrioritizeJobs)(mjob_t **,int *);
  int (*QueueScheduleJobs)(int *,mpar_t *);

  int (*ReservationCreate)(int,macl_t *,char *,long unsigned int,mnalloc_t *,long int,long int,int,int,char *,mres_t **, char *,mcres_t *);
  int (*ReservationDestroy)(mres_t **);
  int (*ReservationJCreate)(mjob_t *, mnodelist_t, long int, int, mres_t **);
  int (*ResFind)(char *, mres_t **);
  int (*AcctFind)(char *,mgcred_t **);
  int (*JobGetRange)(mjob_t *,mreq_t *,mpar_t *,long int,mrange_t *,mnodelist_t,int *,char *,int,mrange_t *);
  int (*BackFill)(int *, int, mpar_t *);
  int (*DoWikiCommand)(char *,int,long,int,char *,char **,long int *,int *);
  int (*JobGetSNRange)(mjob_t *,mreq_t *,mnode_t *,mrange_t *,int,char *,int *,mrange_t *,mcres_t *,char *);
  int (*LL2ShowError)(int,mjob_t *);
  int (*PBSInitialize)();
  int (*RMCancelJob)(mjob_t *,char *,int *);
  int (*RMJobStart)(mjob_t *,char *,int *);
  int (*SimJobSubmit)(long int,mjob_t **,void *,int);
  int (*SRCreate)(sres_t *, int, int);
  int (*WikiLoadJob)(char *,char *,mjob_t *,short int *,mrm_t *);
  int (*QBankDoTransaction)(mam_t *,int,char *,void **,int *,char *);
  int (*JobSetAttr)(mjob_t *,enum MJobAttrEnum,void **,int,int);
  int (*ResSetAttr)(mres_t *,enum MResAttrEnum,void *,int,int);

  /* extension functions */

  int (*XInitializeMauiInterface)();
  int (*XShowConfig)(void *,int,char *,long);

  int (*XPBSNMGetData)(void *,mnode_t *,mrm_t *);

  int (*XAllocateNodes)();
  int (*XBackfill)();
  int (*XPrioritizeJobs)();
  int (*XDiagnoseJobs)();

  int (*XRMInitialize)(void *,mrm_t *);
  int (*XRMResetState)(void *,mrm_t *);
  int (*XRMVerifyData)(void *,mrm_t *,char *);
  int (*XRMDataIsStaging)(void *,void *);
  int (*XRMGetData)(void *,int);
  int (*XUpdateState)();
  int (*XRMJobSuspend)();
  int (*XRMJobResume)();
  int (*XRMJobSubmit)(void *,char *,mrm_t *,mjob_t **,char *,char *,int *);

  int (*XQueueScheduleJobs)();

  int (*XLLInitialize)();
  int (*XLL2JobLoad)();
  int (*XLL2JobUpdate)();
  int (*XLL2NodeLoad)();
  int (*XLL2NodeUpdate)();

  int (*XJobProcessWikiAttr)(void *,mjob_t *,char *);
  int (*XJobGetStartPriority)(void *,mjob_t *,double *,char *);
  int (*XPBSInitialize)(void *, mrm_t *);
  int (*XPBSJobLoad)();
  int (*XWikiDoCommand)();
  int (*XWikiJobLoad)();

  int (*XMetaStoreCompletedJobInfo)(void *,mjob_t *);

  int (*XAllocMachinePrio)();
  int (*XAllocLoadBased)();

  int (*XUIHandler)(void *,msocket_t *,char *,int);

  int (*XGetClientInfo)(void *,msocket_t *,char *);
  int (*XGetTime)(void *,long *,int);
  int (*XSleep)(void *,long);

  int (*XResInit)();
  int (*XResUpdate)(void *,mres_t *);
  int (*XResDestroy)(void *,mres_t *);
  int (*XResShow)(void *,mres_t *,char *);
  char *(*XResCPCreate)(void *,mres_t *);
  int (*XResCPLoad)(void *,char *,mres_t *);

  int (*XJobPreInit)(void *,mjob_t *);
  int (*XJobPostInit)(void *,mjob_t *,int);
  int (*XJobUpdate)(void *,mjob_t *);
  int (*XJobDestroy)(void *,mjob_t **,int);
  int (*XJobShow)(void *,mjob_t *,char *);
  int (*XJobProcessRMXString)(void *,mjob_t *,char *);
  int (*XJobAllocateResources)(mjob_t *,mnodelist_t,char *,mnodelist_t,int,long);

  /* base scheduler data */

  mgcred_t   **User;
  mgcred_t    *Group;
  mgcred_t    *Acct;
  mqos_t      *MQOS;

  mattrlist_t *AttrList;
  msched_t    *Sched;
  long        *CREndTime;
  char        *CurrentHostName;
  mlog_t      *dlog;
  mfsc_t      *FS;
  mjob_t     **Job;
  mnode_t    **MNode;
  mrange_t    *MRange;
  mpar_t      *MPar;
  mulong      *PresentTime;
  sres_t      *OSRes;
  mres_t     **Res;
  mrm_t       *RM;
  mam_t       *AM;
  sres_t      *SRes;
  mstat_t     *Stat;

  /* extension data */

  void *xd;
  } mx_t;


typedef struct {
  long MTime;
  long ETime;
  long WallTime;

  long WCLimit;

  int  JobSwapLimit;
  int  JobMemLimit;

  int  NCPUs;
  int  NodesRequested;
  int  ProcsRequested;

  long ProcCPULimit;
  long JobCPULimit;

  long UtlJobCPUTime;
  } tpbsa_t;

enum MFormatModeEnum {
  mfmNONE = 0,
  mfmHuman,
  mfmHTTP,
  mfmXML,
  mfmAVP,
  mfmVerbose };

enum MauiObjectSetModeEnum {
  mosNONE = 0,
  mVerify,
  mSet,
  mAdd,
  mClear,
  mUnset };

#define MMAX_NODE_PER_FRAG  1024


/* macros */

#define MOINITLOOP(OP,OI,OS,OE) \
  *(OP)=MSched.T[OI];      \
  *(OS)=MSched.S[OI];      \
  *(OE)=MSched.E[OI];
 
#define MNODEISUP(N) \
  (((N->State == mnsIdle) || (N->State == mnsActive) || (N->State == mnsBusy) \
 || (N->State == mnsDraining) || (N->State == mnsReserved)) ? TRUE : FALSE)

#define MNODEISACTIVE(N) \
  (((N->State == mnsBusy) || (N->State == mnsActive) || (N->State == mnsDraining)) ? TRUE : FALSE)
 
#define MJOBISACTIVE(J) \
  (((J->State == mjsStarting) || (J->State == mjsRunning)) ? TRUE : FALSE)

#define MJOBISALLOC(J) \
  (((J->State == mjsStarting) || (J->State == mjsRunning) || (J->State == mjsSuspended)) ? TRUE : FALSE)

#define MJOBISSUSPEND(J) \
   (((J)->State == mjsSuspended) ? TRUE : FALSE)

#define MJOBISCOMPLETE(J) \
  (((J->State == mjsCompleted) || (J->State == mjsRemoved) || (J->State == mjsVacated)) ? TRUE : FALSE)

#define MJOBISSCOMPLETE(J) \
  (((J->State == mjsCompleted)) ? TRUE : FALSE)

#define MJOBISFCOMPLETE(J) \
  (((J->State == mjsRemoved) || (J->State == mjsVacated)) ? TRUE : FALSE)

#define MISSET(B,F) \
  (((B) & (1 << (F))) ? TRUE : FALSE)

#define MSET(B,F) \
  (B) |= (1 << (F))
                                                                                
#define MUNSET(B,F) \
  (B) &= ~(1 << (F))

#define MMAX_PID 32

/* ended processes */

typedef struct {
  int PID;
  int StatLoc;
  } mpid_t;

/* sync w/MSAN[] */

enum MSSSAttrNameEnum {
  msanNONE = 0,
  msanAction,
  msanArgs,
  msanFlags,
  msanName,
  msanObject,
  msanOp,
  msanOption,
  msanValue,
  msanLAST };


/* sync w/MSON */

enum MSSSObjNameEnum {
  msonNONE = 0,
  msonBody,
  msonData,
  msonEnvelope,
  msonGet,
  msonObject,
  msonRequest,
  msonResponse,
  msonSet,
  msonWhere };

#include "moab-wiki.h"    

#ifndef __MX
#define __MX
#endif /* __MX */

#if !defined(__MX)
#define MAX_SUNAME             64
#define MAX_SULINE           1024
#define MAX_G2XMLATTR          64
#define DEFAULT_G2XMLICCOUNT   16
#else /* !__MX */
#include "mg2.h"
#endif /* !__MX */

#ifdef __M_H      

#define MAM_CLIENTTYPE  "maui"    

#endif /* __M_H */    

#endif /* __MOAB_H */

/* END moab.h */
