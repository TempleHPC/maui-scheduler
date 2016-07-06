/*
 * Collection of utility functions and data structures for maui
 */
#include <time.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#ifndef MAUI_UTILS_H
#define MAUI_UTILS_H

#ifndef __M32COMPAT
#define __M32COMPAT
#endif /* __M32COMPAT */

#define MLog dPrint

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */

#define blk(i)                                                                 \
    (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^ \
                                block->l[(i + 2) & 15] ^ block->l[i & 15],     \
                            1))

#define blk0(i)                                          \
    (block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) | \
                   (rol(block->l[i], 8) & 0x00FF00FF))

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

#define R0(v, w, x, y, z, i)                                     \
    z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); \
    w = rol(w, 30);
#define R1(v, w, x, y, z, i)                                    \
    z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); \
    w = rol(w, 30);
#define R2(v, w, x, y, z, i)                            \
    z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); \
    w = rol(w, 30);
#define R3(v, w, x, y, z, i)                                          \
    z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); \
    w = rol(w, 30);
#define R4(v, w, x, y, z, i)                            \
    z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); \
    w = rol(w, 30);

#define MUINT4 unsigned long
#define MUINT8 unsigned long long
#define mulong unsigned long
#define MBNOTSET 2
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MISSET(B, F) (((B) & (1 << (F))) ? TRUE : FALSE)
#define MAX_MNODE 5120
#define mbool_t unsigned char
#define MAX_MPAR 4
#define MMAX_LINE 1024
#define MAX_MATTR 128
#define MAX_MTASK 4096
#define MAX_MQOS 128
#define SUCCESS 1
#define FAILURE 0
#define TRUE 1
#define FALSE 0
#define NONE "[NONE]"


/* 32 bit values */

#define M32INTBITS 32
#define M32INTLBITS 5
#define M32INTSIZE 4
#define M32INTSHIFT 2

/* 64 bit values */

#define M64INTBITS 64
#define M64INTLBITS 6
#define M64INTSIZE 8
#define M64INTSHIFT 3

#define MOAB_VERSION "3.3.2"
#define MSCHED_SNAME "maui"
#define MBUILD_SKEY    "12971"
#define MSCHED_ENVPARVAR "MAUIPARTITION"
#define GLOBAL_MPARNAME "ALL"
#define CRYPTHEAD "KGV"

#define SHA_BLOCKSIZE 64
#define MDEF_XMLICCOUNT 16
#define MAX_MDESCKSUM_ITERATION 4
#define SOCKETFLAGS (int)0
#define MMAX_XBUFFER 131072
#define SHA_DIGESTSIZE 20
#define MMD5_DIGESTSIZE 0
#define MAX_SCALE 128
#define MAX_MLIST 16
#define MAX_MGRIDTIMES 32
#define MAX_SRES_DEPTH 64
#define MMAX_AMFUNC 20
#define MAX_MGRIDSIZES 32
#define MMAX_SBUFFER 65536
#define MMSG_BUFFER (MMAX_SBUFFER << 5)
#define MCONST_SMALLPACKETSIZE 8192
#define MAX_MBUFFER 65536
#define TCP 1
#define MMAX_BUFFER 65536
#define DEFAULT_CLIENTTIMEOUT 30000000
#define MAX_FILECACHE 32
#define MAX_FBUFFER 16000000
#define MMAX_CLIENT 10
#define MAX_MPRIOSUBCOMPONENT 33
#define MAX_MRESOURCETYPE 6
#define MAX_MPRIOCOMPONENT 8
#define MAX_MREQ_PER_JOB 4
#define MMAX_JOBRA 16
#define MAX_MPOOL 7
#define MAX_MRMFUNC 22
#define MAX_MRMFAILURE 8
#define MAX_MRMSUBTYPE 2
#define MAX_MTASK_PER_JOB MAX_MTASK
#define MAX_MNPCOMP 16
#define MAX_MGRES 4
#define MAX_MCLASS 16
#define MAX_TASK_REQUESTS 32
#define MAX_MACL 32
#define MAX_ACCURACY 10
#define MAX_MFSDEPTH 24
#define MAX_MPALSIZE 1
#define MAX_MQALSIZE 5
#define MAX_MPOLICY 9
#define MAX_MNAME 64
#define MMAX_NAME 64
#define MAX_WORD 32
#define MAX_MLINE 1024
#define MAX_MADMINUSERS 64
#define MAX_MADMINHOSTS 64
#define MAX_PATH_LEN 256
#define MAX_MARG 32
#define MAX_MTIME 2140000000
#define MAX_MCFG 512
#define MMAX_XMLATTR 64
#define FLAG_Compress 0x40
#define FLAG_Copied 0x80

#define MCONST_CKEY "hello"
#define MSCHED_ENVHOMEVAR "MAUIHOMEDIR"
#define MASTER_CONFIGFILE "/etc/maui.cfg"
#define MSCHED_KEYFILE ".moab.key"
#define DEFAULT_SchedCONFIGFILE "maui.cfg"
#define MBUILD_HOMEDIR "/home/dallin/mau/"

#if !defined(DEFAULT_MSERVERHOST)
#define DEFAULT_MSERVERHOST ""
#endif /* !DEFAULT_MSERVERHOST */

#if !defined(DEFAULT_MSERVERPORT)
#define DEFAULT_MSERVERPORT 40559
#endif /* !DEFAULT_MSERVERPORT */

/*
#define MBUILD_DIR     "/home/dallin/maui-scheduler"
#define MBUILD_HOST    "dallin-ThinkCentre-M900"
#define MBUILD_DATE    "Mon Jun 27 15:50:51 EDT 2016"
*/

enum { msmNONE = 0, msmNormal, msmProfile, msmSim, msmSingleStep, msmTest };

enum MTimePolicyEnum { mtpNONE = 0, mtpReal };

enum MTimeModeEnum { mtmNONE = 0, mtmInit, mtmRefresh };

enum MWireProtocolEnum { mwpNONE = 0, mwpAVP, mwpXML, mwpHTML, mwpS32 };

enum MauiObjectSetModeEnum { mosNONE = 0, mVerify, mSet, mAdd, mClear, mUnset };

enum MSockFTypeEnum { msftNONE = 0, msftTCP, msftUDP };

enum MJobNameEnum {
    mjnNONE = 0,
    mjnFullName,
    mjnRMName,
    mjnShortName,
    mjnSpecName
};

/* failure codes */

/* sync w/MFC[] */

enum MSFC {
    msfENone = 0,            /* success */
    msfGWarning = 100,       /* general warning */
    msfEGWireProtocol = 200, /* general wireprotocol/network failure */
    msfEBind = 218,          /* cannot bind socket */
    msfEGConnect = 220,      /* general connection failure */
    msfCannotConnect = 222,  /* cannot connect */
    msfCannotSend = 224,     /* cannot send data */
    msfCannotRecv = 226,     /* cannot receive data */
    msfConnRejected = 230,   /* connection rejected */
    msfETimedOut = 232,      /* connection timed out */
    msfEFraming = 240,       /* general framing failure */
    msfEEOF = 246,           /* unexpected end of file */
    msfEGMessage = 300,      /* general message format error */
    msfENoObject = 311,      /* no object specified in request */
    msfEGSecurity = 400,     /* general security failure */
    msfESecClientSig = 422, /* security - signature creation failed at client */
    msfESecServerAuth = 424, /* security - server auth failure */
    msfESecClientAuth = 442, /* security - client auth failure */
    msfEGEvent = 500,        /* general event failure */
    msfEGServer = 700,       /* general server error */
    msfEGServerBus = 720,    /* general server business logic failure */
    msfEGClient = 800,       /* general client error */
    msfECInternal = 820,     /* client internal error */
    msfECResUnavail = 830,   /* client resource unavailable */
    msfECPolicy = 840,       /* client policy failure */
    msfEGMisc = 900,         /* general miscellaneous error */
    msfUnknownError = 999
}; /* unknown failure */

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
    msonWhere
};

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
    mckCheckSum
};

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
    mjcmTerminate
};

enum MJFActionEnum { mamjfaNONE = 0, mamjfaCancel, mamjfaDefer };

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
    mscBadRequest, /* request is corrupt */
    mscNoData,     /* data not yet available */
    mscLAST
};

enum MResLimitPolicyEnum {
    mrlpNONE = 0,
    mrlpAlways,
    mrlpExtendedViolation,
    mrlpBlockedWorkloadOnly
};

enum MSocketProtocolEnum {
    mspNONE = 0,
    mspSingleUseTCP,
    mspHalfSocket,
    mspHTTPClient,
    mspHTTP,
    mspS3Challenge
};

/* sync w/MCSAlgoType[] */

enum MChecksumAlgoEnum {
    mcsaNONE = 0,
    mcsaDES,
    mcsaHMAC,
    mcsaHMAC64,
    mcsaMD5,
    mcsaPasswd,
    mcsaRemote
};

/* sync w/MS3CName[] */

enum MRMStateEnum { mrmsNONE = 0, mrmsActive, mrmsDown, mrmsCorrupt };

enum MPeerServiceEnum {
    mpstNONE = 0,
    mpstNM, /* system monitor     */
    mpstQM, /* queue manager      */
    mpstSC, /* scheduler          */
    mpstMS, /* meta scheduler     */
    mpstPM, /* process manager    */
    mpstAM, /* allocation manager */
    mpstEM, /* event manager      */
    mpstSD, /* service directory  */
    mpstWWW
}; /* web						  */

enum { dbLOGFILE, dbNOTIFY, dbSYSLOG };

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
    mrmtNative
};

/* res object */

enum MResAttrEnum {
    mraNONE = 0,
    mraAAccount, /* accountable account */
    mraACL,
    mraAGroup, /* accountable group */
    mraAUser,  /* accountable user */
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
    mraXAttr
};

enum MJobAttrEnum {
    mjaNONE = 0,
    mjaAccount,
    mjaAllocNodeList,
    mjaArgs,
    mjaAWDuration, /* active wall time consumed */
    mjaCalendar,
    mjaCmdFile,
    mjaCompletionTime,
    mjaCPULimit,
    mjaDepend,
    mjaDRMJID,
    mjaEEWDuration, /* effective eligible wall duration:  duration job has been
                       eligible for scheduling */
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
    mjaJobID,   /* job batch id */
    mjaJobName, /* user specified job name */
    mjaMasterHost,
    mjaMessages,
    mjaNotification,
    mjaPAL,
    mjaPriority,
    mjaQOS,
    mjaQOSReq,
    mjaReqAWDuration, /* req active walltime duration */
    mjaReqCMaxTime,   /* req latest allowed completion time */
    mjaReqNodes,
    mjaReqProcs,
    mjaReqReservation, /* req reservation */
    mjaReqSMinTime,    /* req earliest start time */
    mjaRMJID,          /* RM job ID */
    mjaRMXString,      /* RM extension string */
    mjaRsvAccess,
    mjaSRMJID,
    mjaStartCount,
    mjaStartTime, /* most recent time job started execution */
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
    mjaUtlProcs
};

enum MNodeAccessPolicyEnum {
    mnacNONE = 0,   /* DEFAULT:  shared */
    mnacShared,     /* any combination of workload allowed */
    mnacSingleJob,  /* peer tasks from a single job may utilize node */
    mnacSingleTask, /* only a single task may utilize node */
    mnacSingleUser /* any number of tasks from the same user may utilize node */
};

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
    svcMBal,
    svcShowTasks
};

enum MResLimitVActionEnum {
    mrlaNONE = 0,
    mrlaCancel,
    mrlaRequeue,
    mrlaSuspend
};

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

enum MJobDependEnum {
    mjdNONE = 0,
    mjdJobStart,
    mjdJobCompletion,
    mjdJobSuccessfulCompletion,
    mjdJobFailedCompletion
};

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
    mjsLost
};

/* status code values */

enum { scFAILURE = 0, scSUCCESS };

enum MDataFormatEnum {
    mdfNONE = 0,
    mdfString,
    mdfInt,
    mdfLong,
    mdfDouble,
    mdfStringArray,
    mdfIntArray,
    mdfLongArray,
    mdfDoubleArray,
    mdfOther,
    mdfLAST
};

enum MAccessModeEnum { macmNONE = 0, macmRead, macmWrite };

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
    mxoLAST
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
    pSimWCScalingPercent, /* sim params */
    pSimInitialQueueDepth,
    pSimWCAccuracy,
    pSimWCAccuracyChange,
    pSimJobSubmissionPolicy,
    pSimNCPolicy,
    pUseMachineSpeed,
    pUseMachineSpeedForFS,
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
    pBFPolicy, /* policy params */
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
    pServWeight, /* NOTE: parameters must sync with prio comp/subcomp enum */
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
    pUExeTimeWeight,
    pURemWeight,
    pUPerCWeight,
    pXFMinWCLimit,
    pServCap, /* NOTE: parameters must sync with prio comp/subcomp enum */
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
    pUExeTimeCap,
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
    pStatDir, /* Stat params */
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
    pOLDFSUWeight, /* params for backwards compatibility */
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
    pOLDRMServer,
    pNAMaxPS,
    pFSSecondaryGroups, /* To enable secondary fairshare group lookups for PBS,
                           HvB */
    pIgnPbsGroupList    /* ignore the -W group_list parameter for PBS, HvB */
};

/* node states */

enum MNodeStateEnum {
    mnsNONE = 0, /* not set */
    mnsNone,     /* set to 'none' by RM */
    mnsDown,
    mnsIdle,
    mnsBusy,
    mnsActive, /* node is executing workload */
    mnsDrained,
    mnsDraining,
    mnsFlush,
    mnsReserved, /* node is reserved (internal) */
    mnsUnknown
}; /* node is up, usage must be determined */

typedef struct {
    char ServerHost[MAX_MNAME];
    int ServerPort;
    long Timeout;

    int Format;

    int SocketProtocol;

    char ServerCSKey[MAX_MNAME];

    enum MChecksumAlgoEnum ServerCSAlgo;

    char ConfigFile[MAX_MLINE];
    char HomeDir[MAX_MLINE];

    char SchedulerMode[MAX_MNAME];

    char BuildDir[MAX_MLINE];
    char BuildHost[MAX_MLINE];
    char BuildDate[MAX_MLINE];

    long DisplayFlags;
} mccfg_t;

/* extended load indices */

typedef struct {
    double PageIn;
    double PageOut;
    int LoginCount;
} mxload_t;

typedef struct {
    char FileName[MAX_PATH_LEN];
    time_t ProgTimeStamp;
    time_t FileTimeStamp;
    char *Buffer;
    int BufSize;
} mfcache_t;

/* structures */

struct attrl {
    struct attrl *next;
    char *name;
    char *resource;
    char *value;
    int op; /* not used */
};

typedef struct {
    mbool_t Is64;
    int INTBITS;
    int INTLBITS;
    int INTSIZE;
    int INTSHIFT;
} m64_t;

typedef struct { char *GData; } mlsfmdata_t;

typedef struct _msgemdata {
    int VersionNumber;
    int ServerSD;
    long ServerSDTimeStamp;
    int SchedSD;
    long SchedSDTimeStamp;
    char LocalDiskFS[MAX_PATH_LEN];
    char SubmitExe[MAX_PATH_LEN];

    long NodeMTime;
    long JobMTime;
    long QueueMTime;

    void *NodeList;
    void *JobList;
    void *QueueList;
} msgemdata_t;

typedef struct {
    char *Name;
    int PIndex;
    int Format;
    int OType;
    void *OPtr;
} mcfg_t;

struct batch_status {
    struct batch_status *next;
    char *name;
    struct attrl *attribs;
    char *text;
};

typedef struct mxml_s {
    char *Name;
    char *Val;

    int ACount;
    int ASize;

    int CCount;
    int CSize;

    char **AName;
    char **AVal;

    struct mxml_s **C;
} mxml_t;

typedef struct _pbsnodedata {
    char Name[MAX_MNAME];

    long MTime;

    int CProcs;
    int CMem;
    int CSwap;

    int ADisk;

    int ArchIndex;

    int MState;

    double Load;

    void *xd;
} pbsnodedata_t;

typedef union {
    unsigned char c[64];
    MUINT4 l[16];
} CHAR64LONG16;

/* server checkpointing object */

typedef struct {
    char CPFileName[MAX_MLINE];

    char *Buffer;

    time_t CPInterval; /* seconds between subsequent checkpoints          */
    time_t CPExpirationTime; /* seconds stale checkpoint data will be kept */
    time_t LastCPTime; /* time of most recent checkpoint                  */

    char DVersion[MAX_MNAME];     /* detected data version */
    char SVersionList[MAX_MLINE]; /* supported versions    */
    char WVersion[MAX_MNAME];     /* write version         */

    FILE *fp;

    char *OBuffer;
} mckpt_t;

typedef struct {
    time_t MinTime;
    time_t MaxTime;
    int TimeStepCount;
    int TimeStep[MAX_SCALE];
    int TimeStepSize;
    int MinNode;
    int MaxNode;
    int NodeStepCount;
    int NodeStep[MAX_SCALE];
    int NodeStepSize;
    int AccuracyScale;
    int AccuracyStep[MAX_SCALE];
    time_t BeginTime;
    time_t EndTime;
    int TraceCount;
} mprofcfg_t;

typedef struct _mllmdata{
    int VersionNumber;
    int ProcVersionNumber;
    char ConfigFile[MAX_PATH_LEN];

    void *JobQO;
    void *JobList;

    void *NodeQO;
    void *NodeList;
} mllmdata_t;

typedef struct {
    int NAllocPolicy;
    int ResOrder[4];
} mnallocp_t;

typedef struct _mnuml {
	int count;
} mnuml_t;

typedef struct _msssmdata {
    int VersionNumber;
    int ServerSD;
    long ServerSDTimeStamp;
    int SchedSD;
    long SchedSDTimeStamp;
    char LocalDiskFS[MAX_PATH_LEN];
    char SubmitExe[MAX_PATH_LEN];

    long NodeMTime;
    long JobMTime;
    long QueueMTime;

    void *NodeList;
    void *JobList;
    void *QueueList;
} msssmdata_t;

/* fairshare */

typedef struct _mfsc {
    long PCW[MAX_MPRIOCOMPONENT];
    long PCP[MAX_MPRIOCOMPONENT];

    long PSW[MAX_MPRIOSUBCOMPONENT];
    long PSP[MAX_MPRIOSUBCOMPONENT];

    long PCC[MAX_MPRIOCOMPONENT];
    long PSC[MAX_MPRIOSUBCOMPONENT];

    time_t XFMinWCLimit;

    int FSPolicy;    /* FairShare Enforceability */
    long FSInterval; /* Time Interval Covered by each FS Data File */
    int FSDepth;     /* Number of FS Time Intervals Considered by FairShare */
    double FSDecay;  /* Weighting Factor to Decay Older FS Intervals */
} mfsc_t;

typedef struct _mnprio{
    double CW[MAX_MNPCOMP]; /* component weight */
    int CWIsSet;

    double SP; /* static priority  */
    int SPIsSet;

    double DP; /* dynamic priority */
    int DPIsSet;
    int DPIteration;
} mnprio_t;

typedef struct _msocket{
    char Name[MMAX_NAME];
    char Host[MMAX_NAME]; /* ??? */
    char RemoteHost[MMAX_NAME];
    int RemotePort;
    char *URI;

    long Flags;
    mulong Timeout;

    enum MSvcEnum SIndex; /* requested service type            */

    int SvcType;     /* requested service type            */
    long StatusCode; /* return code of service request    */
    char *RID;       /* requestor UID                     */

    /* base config */

    long Version;
    int sd;
    char *RBuffer;
    long RBufSize;
    char *SBuffer;

    mulong State;

    mbool_t IsLoaded;
    mbool_t IsNonBlocking;
    mbool_t SBIsDynamic;

    long SBufSize;
    char *RPtr;
    char *SPtr;

    void *SE;  /* xml to send - full (alloc,w/header) */
    void *SDE; /* xml to send (alloc,optional) */

    char *SData; /* text to send (alloc,optional) */

    char *SMsg; /* (alloc) */

    char *ClientName; /* (alloc) */
    void *Cred;       /* possibly a suclcred_t (ptr) */

    void *RE;  /* incoming message - full (alloc) */
    void *RDE; /* incoming message (alloc) */

    /* comm config */

    enum MWireProtocolEnum WireProtocol;
    enum MSocketProtocolEnum SocketProtocol;
    enum MChecksumAlgoEnum CSAlgo;

    mbool_t DoEncrypt;

    char CSKey[MMAX_NAME];
} msocket_t;

typedef struct _mpbsmdata {
    int VersionNumber;
    int ServerSD;
    long ServerSDTimeStamp;
    msocket_t SchedS;
    long SchedSDTimeStamp;
    char LocalDiskFS[MAX_PATH_LEN];
    char SubmitExe[MAX_PATH_LEN];

    int PBS5IsEnabled; /* boolean */
    int SSSIsEnabled;  /* boolean */

    pbsnodedata_t D[MAX_MNODE];

    long NodeMTime;
    long JobMTime;
    long QueueMTime;

    struct batch_status *NodeList;
    struct batch_status *JobList;
    struct batch_status *QueueList;
} mpbsmdata_t;

typedef struct {
    int Procs;
    int Mem;  /* (in MB) */
    int Swap; /* (in MB) */
    int Disk; /* (in MB) */

    mnuml_t PSlot[MAX_MCLASS];
    mnuml_t GRes[MAX_MGRES];
} mcres_t;

typedef struct {
    time_t Time;
    short Type;
    short Index;
    mcres_t DRes; /* total resources dedicated to reservation across all tasks
                     on node */
} mre_t;

/* client object */

typedef struct {
    char Name[MAX_MNAME];
    char CSKey[MAX_MNAME];
} mclient_t;

typedef struct _mjobcache{
    /* resource:  walltime, procs, nodes, memory, network, etc */
    /* stat:      psutilized, etc */
    /* attr:      credentials, QOS, etc */

    int TotalProcCount;
} mjobcache_t;

typedef struct _macl{
    char Name[MAX_MNAME];
    long Value;
    long Flags;
    char Type;
    char Cmp;
    char Affinity;
} macl_t;

typedef struct _mjckpt{
    time_t CPInterval;

    time_t StoredCPDuration; /* duration of walltime checkpointed in previous
                                runs */
    time_t
        ActiveCPDuration; /* duration of walltime checkpointed in current run */

    time_t InitialStartTime;
    time_t LastCPTime;

    char UserICPEnabled;
    char SystemICPEnabled;
} mjckpt_t;

typedef struct {
    MUINT4 state[5];
    MUINT4 count[2];
    unsigned char buffer[64];
} SHA1_CTX;

/* elemental objects */

typedef struct _msync{
    long UpdateTime;
    void *Updates;

    int IsSynced; /* boolean */
} msync_t;

typedef struct _mgrid{
    long MTime;
    int FailureCount;
    int QCount;
    int JCount;
    int RCount;
    int CCount;

    int SIsInitialized;
    int IIsInitialized;

    char *Messages;
} mgrid_t;

typedef struct _mfs {
    unsigned long JobFlags; /* default job flags associated w/cred     */

    long Priority;
    char IsLocalPriority; /* priority is locally specified (boolean) */

    long Overrun; /* duration job may overrun wclimit        */

    void *ADef; /* default account                         */
    int *AAL;   /* account access list                     */

    void *QDef;            /* default QOS                             */
    int QAL[MAX_MQALSIZE]; /* QOS access list                         */
    int QALType;

    void *PDef; /* default partition                       */
    int PAL[MAX_MPALSIZE];
    int PALType;

    double FSTarget; /* credential fs usage target              */
    int FSMode;      /* fs target type                          */

    double FSFactor;              /* effective decayed fs factor             */
    double FSUsage[MAX_MFSDEPTH]; /* FS Usage History                        */

    int IsInitialized; /* (boolean) */
} mfs_t;

typedef struct {
    int Usage[MAX_MPOLICY][MAX_MPAR];
    int SLimit[MAX_MPOLICY][MAX_MPAR];
    int HLimit[MAX_MPOLICY][MAX_MPAR];
} mpu_t;

typedef struct _mcredl {
    mpu_t AP;
    mpu_t *IP;
    mpu_t *JP;

    mpu_t *OAP; /* override active usage policies  */
    mpu_t *OIP; /* override idle usage policies    */
    mpu_t *OJP;

    mpu_t *APU; /* active policy (per user) */
    mpu_t *APC; /* active policy (per class) */
    mpu_t *APG; /* active policy (per group) */
    mpu_t *APQ; /* active policy (per QOS) */

    void *JDef; /* job defaults */
    void *JMax; /* job maximums */
    void *JMin; /* job minimums */
} mcredl_t;

typedef struct _dstat {
    char *Data;
    int DSize;
    int Count;
    int Size;
} dstat_t;

typedef struct _must {
    int Count;             /* total jobs in grid                              */
    int NCount;            /* nodes allocated to jobs                         */
    int JobCountSubmitted; /* jobs submitted                                  */
    int JobCountSuccessful; /* jobs successfully completed */
    int QOSMet;         /* jobs that met requested QOS                     */
    int RejectionCount; /* jobs submitted that have been rejected          */
    time_t TotalQTS;    /* job queue time                                  */
    time_t MaxQTS;
    double TotalQueuedPH; /* total queued prochours                          */
    double TotalRequestTime; /* total requested job walltime */
    double TotalRunTime; /* total execution job walltime                    */

    /* job-level statistics */

    double PSRequest; /* requested procseconds completed                 */
    double PSRequestSubmitted; /* requested procseconds submitted */
    double PSRun; /* executed procsecond                             */
    double PSRunSuccessful; /* successfully completed procseconds */

    /* iteration-level statistics */

    double PSDedicated; /* ProcSeconds Dedicated                           */
    double PSUtilized;  /* ProcSeconds Actually Used                       */

    double MSAvail; /* MemorySeconds Available                         */
    double MSUtilized;
    double MSDedicated; /* MemorySeconds Dedicated                         */

    double PS2Dedicated;
    double PS2Utilized;

    double JobAcc;     /* Job Accuracy Sum                                */
    double NJobAcc;    /* Node Weighed Job Accuracy Sum                   */
    double XFactor;    /* XFactor Sum                                     */
    double NXFactor;   /* Node Weighed XFactor Sum                        */
    double PSXFactor;  /* PS Weighted XFactor Sum                         */
    double MaxXFactor; /* max XFactor detected                            */
    int Bypass;        /* Number of Times Job was Bypassed                */
    int MaxBypass;     /* Max Number of Times Job was not Scheduled       */
    int BFCount;       /* Number of Jobs Backfilled                       */
    double BFPSRun;    /* ProcSeconds Backfilled                          */

    short Accuracy[MAX_ACCURACY];
    dstat_t DStat[8];
} must_t;

typedef struct _xres {
    char Name[MAX_MNAME];
    int Type;
    int (*Func)(void *, int, void *, void **);
    void *Data;
} xres_t;

typedef struct _mgcred{
    char Name[MAX_MNAME]; /* cred name                     */
    int Index;
    long MTime; /* modify time                   */

    unsigned long Key; /* array hash key                */

    unsigned long OID;

    int ClassWeight; /* NOTE: temp */

    mfs_t F; /* fairness policy config        */
    void *P; /* ??? */

    mcredl_t L; /* current resource usage        */

    must_t Stat; /* historical usage statistics   */
} mgcred_t;

typedef struct _mqos{
    char Name[MAX_MNAME];
    int Index;
    long MTime; /* time record was last updated               */
    long QTWeight;
    long XFWeight;
    long QTTarget;
    double XFTarget;     /* Targeted XFactor for QOS                   */
    unsigned long Flags; /* Flags/Exemptions Associated with QOS       */
    char ResName[16][MAX_MNAME];
    mfs_t F;
    mcredl_t L;
    must_t Stat; /* Usage Statistics                           */
} mqos_t;

typedef struct _mclass{
    char Name[MAX_MNAME];
    int Index;
    long MTime; /* time record was last updated               */

    int NonExeType;

    mfs_t F;
    mcredl_t L;
    must_t Stat;

    int State; /* NOT USED */

    int IsDisabled; /* queue cannot execute jobs */

    int DistPolicy;
    int NAPolicy;

    char **NodeList;
    char *OCNodeName;     /* overcommit node */
    double OCDProcFactor; /* dedicated proc factor */

    int DefFBM[MAX_MATTR >> 5]; /* default feature (BM) */

    int MaxProcPerNode;
} mclass_t;

typedef struct _mcred{
    mgcred_t *U;
    mgcred_t *G;
    mgcred_t *A;
    mclass_t *C;
    mqos_t *Q;

    int CredType;

    macl_t *ACL;
    macl_t *CL;

    long MTime;
} mcred_t;

typedef struct _mres{
    char Name[MAX_MNAME + 1]; /* name of object reserving nodes             */
    int Index;

    long MTime;
    long CTime;

    char *RegEx; /* host regular expression                    */
    int PtIndex;
    int Type; /* reservation type                           */
    int Mode; /* persistant, slide-forward, slide-back,
                 slide-any                                  */
    void *J;
    void *NL;

    time_t StartTime; /* reservation starttime                      */
    time_t EndTime;   /* reservation endtime                        */

    int MaxTasks;

    int NodeCount; /* nodes involved in reservation              */
    int TaskCount;
    int ETaskCount;
    int AllocPC;
    macl_t ACL[MAX_MACL];
    macl_t CL[MAX_MACL];

    unsigned long Flags; /* reservation flags                          */

    double CIPS; /* allocation/utilization stats */
    double CAPS;
    double TIPS;
    double TAPS;

    char AccessList[16][MAX_MNAME];
    int AllocResPending;
    long ExpireTime;
    long Priority;
    mcres_t DRes;
    char CBHost[MAX_MNAME];
    int CBPort;
    long CBType;

    char *SystemID; /* user or daemon which created reservation */
    char *Comment;

    mgcred_t *U; /* accountable user, group, account */
    mgcred_t *G; /* NOTE:  accountable cred same as creation cred */
    mgcred_t *A;

    char *XCP;

    int IsActive;

    void *O; /* rsv owner */
    int OType;

    void *xd;
} mres_t;

typedef struct _msdata {
	char *Location;

	int IsGlobal;

	char *SrcFileName;
	char *DstFileName;

	char *SrcHostName;
	char *DstHostName;

	long SrcFileSize; /* available file size */
	long DstFileSize; /* total file size */

	time_t TStartTime;
	int TransferRate; /* KB/s */

	time_t ESTime; /* estimated stage time */
	struct msdata_t *Next;
} msdata_t;

typedef struct _mpsi{
    enum MPeerServiceEnum Type;

    enum MWireProtocolEnum WireProtocol;
    enum MSocketProtocolEnum SocketProtocol;
    enum MChecksumAlgoEnum CSAlgo;

    char *CSKey;

    char *HostName;
    int Port;
    char *Version;

    mulong Timeout;

    void *Data;

    msocket_t *S;
} mpsi_t;

typedef struct _mrm{
    char Name[MAX_MNAME + 1];
    int Index;

    long Flags;

    char APIVersion[MAX_MNAME];
    int Version;

    enum MRMStateEnum State;

    long StateMTime;
    int XState;

    long LastSubmissionTime;

    /* interface */

    enum MRMTypeEnum Type;

    mpsi_t P[MAX_MRMSUBTYPE]; /* peer interface */

    int AuthType;

    int NMPort; /* node monitor port */

    int EPort;     /* event port */
    long EMinTime; /* min time between processing events */

    int SpecPort[MAX_MRMSUBTYPE];
    int SpecNMPort;
    char SpecServer[MAX_MRMSUBTYPE][MAX_MNAME + 1];

    int UseDirectoryService; /* boolean */

    char DefaultQMDomain[MAX_MNAME];
    char DefaultQMHost[MAX_MNAME];

    char SuspendSig[MAX_MNAME];

    int FailIteration;
    int FailCount;

    int WorkloadUpdateIteration;

    time_t FailTime[MAX_MRMFAILURE];
    int FailType[MAX_MRMFAILURE];
    char *FailMsg[MAX_MRMFAILURE];
    int FailIndex;

    time_t RespTotalTime[MAX_MRMFUNC];
    time_t RespMaxTime[MAX_MRMFUNC];
    int RespTotalCount[MAX_MRMFUNC];
    time_t RespStartTime[MAX_MRMFUNC];

    int JobCount;
    int NodeCount;

    int JobCounter;

    int RMNI; /* network interface utilized by RM */

    union {
        mllmdata_t LL;
        mpbsmdata_t PBS;
        msgemdata_t SGE;
        msssmdata_t S3;
        mlsfmdata_t LSF;
    } U;

    void *S; /* resource manager specific data */

    void *xd;

    mbool_t ASyncJobStart; /* asynchronous job start */
} mrm_t;

typedef struct _mnode{
    char Name[MAX_MNAME + 1]; /* name of node                                */
    int Index;                /* index in node array                         */

    char *FullName; /* fully qualified host name                   */

    time_t CTime; /* time node record was created                */
    time_t ATime; /* time of most recent node information        */
    time_t MTime; /* time node structure was modified            */

    time_t ResMTime;
    time_t StateMTime; /* time node changed state                     */

    enum MNodeStateEnum State; /* node state                                  */
    enum MNodeStateEnum
        EState; /* expected node state resulting from sched action */
    enum MNodeStateEnum NewState;

    time_t SyncDeadLine; /* time by which state and estate must agree   */

    mcres_t DRes;  /* dedicated resources (dedicated to consumer) */
    mcres_t CRes;  /* configured resources                        */
    mcres_t ARes;  /* available resources                         */
    mcres_t URes;  /* utilized resources                          */
    mcres_t SURes; /* system utilized resources                   */

    int PtIndex; /* partition assigned to node                  */

    int Network;             /* available network interfaces (BM)           */
    int FBM[MAX_MATTR >> 5]; /* available features (BM)                   */
    int Attributes;          /* system attributes (BM)                      */

    char Pool[MAX_MPOOL + 1];
    int Arch;     /* node hardware architecture                  */
    int ActiveOS; /* node operating system                       */

    mres_t **R; /* node reservation table                      */
    mre_t *RE;  /* reservation event table                     */
    short *RC;  /* reservation count table                     */

    double Speed;  /* relative processing speed of node           */
    int ProcSpeed; /* proc speed in MHz                           */

    double Load; /* total node 1 minute load average            */

    double JobLoad; /* load associated with batch workload         */
    double ExtLoad; /* load not associated with batch workload     */
    mxload_t *XLoad;

    double MaxLoad; /* max total load allowed                      */

    double BackLog; /* avg work required by node                   */

    time_t STTime; /* time node was monitored (in 1/100's)        */
    time_t SUTime; /* time node was available (in 1/100's)        */
    time_t SATime; /* time node was active    (in 1/100's)        */

    int TaskCount;
    int EProcCount;

    short FrameIndex; /* frame index                                 */
    short SlotIndex;  /* slot index                                  */
    short SlotsUsed;

    mrm_t *RM;

    char NodeType[MAX_MNAME];

    int IterationCount; /* iterations since last RM update        */
    int Priority;

    int MaxJCount;
    void **JList; /* list of active jobs                    */
    int *JTC;

    void *RMData;

    mpu_t AP; /* active policy tracking                 */

    char *NAvailPolicy; /* node availability policy               */

    int MaxJobPerUser;    /* TEMP                                   */
    int *MaxProcPerClass; /* TEMP                                   */
    int MaxProcPerUser;
    double MaxPEPerJob; /* TEMP                                   */

    char PrivateQueue; /* TEMP (boolean)                         */
    char IsPref;       /* TEMP (boolean)                         */

    mnprio_t *P; /* node priority parameters */

    char *Message; /* event message */

    void *xd;
} mnode_t;

typedef struct _mnalloc {
    mnode_t *N;
    unsigned short TC;
} mnalloc_t;

typedef struct _mreq {
    int Index;

    int RequiredProcs;  /* procs                                       */
    int ProcCmp;        /* procs comparison                            */
    int RequiredMemory; /* real memory (RAM)                           */
    int MemCmp;         /* memory comparison                           */
    int RequiredSwap;   /* virtual memory (in MB)                      */
    int SwapCmp;        /* virtual memory comparison                   */
    int RequiredDisk;   /* disk space requirement (in MB)              */
    int DiskCmp;        /* disk comparison                             */
    int ReqFBM[MAX_MATTR >> 5];  /* required feature (BM)                   */
    int PrefFBM[MAX_MATTR >> 5]; /* preferred feature (BM)                 */
    int ReqFMode; /* required feature mode                       */

    int SetSelection;         /* one of NONE, ONEOF, ANYOF                   */
    int SetType;              /* one of NONE, Feature, Memory, ProcSpeed     */
    char *SetList[MAX_MATTR]; /* list of set delineating job attributes      */
    int Pool;                 /* RM pool index                               */
    int Network;              /* required network (BM)                       */
    int Opsys;                /* required OS (BM)                            */
    int Arch;                 /* HW arch (BM)                                */

    int PtIndex; /* assigned partition index                    */

    mcres_t DRes;   /* dedicated resources per task (active)       */
    mcres_t *SDRes; /* dedicated resources per task (suspended)    */
    mcres_t URes;   /* utilized resources per task                 */
    mcres_t LURes;  /* req lifetime utilized resources per task    */
    mcres_t MURes;  /* maximum utilized resources per task         */

    time_t RMWTime; /* req walltime as reported by RM              */
    time_t StatWTime;

    /* index '0' is active, index '1-N' contain requests */

    int TaskRequestList[MAX_TASK_REQUESTS + 1];

    int NodeCount; /* nodes requested */
    int TaskCount; /* tasks allocated */

    int TasksPerNode;
    int BlockingFactor;
    int NAccessPolicy; /* node access policy */
    mnallocp_t *NAllocPolicy;

    int AdapterAccessMode;
    int RMIndex;

    enum MJobStateEnum State;
    enum MJobStateEnum EState;

    char SubJobName[MAX_MNAME];

    mnalloc_t *NodeList;
} mreq_t;

typedef struct _mjobe {
    char *IWD;     /* initial working directory of job             */
    char *Cmd;     /* job executable                               */
    char *Input;   /* input                                        */
    char *Output;  /* output                                       */
    char *Error;   /* error                                        */
    char *Args;    /* command line args                            */
    char *Env;     /* shell environment                            */
    char *Shell;   /* execution shell                              */
    char *JobType; /* job type { ie, parallel, serial, etc }       */

    long PID;
    long SID;
} mjobe_t;

typedef struct _mrsrc {
    int TC;
    int NC;
} mrsrc_t;


typedef struct _mjdepend {
    enum MJobDependEnum Type;
    char *Value;
    struct mjdepend_t *Next;
} mjdepend_t;

typedef struct _mjob {
    char Name[MAX_MNAME + 1]; /* job ID                                  */
    char *AName;              /* alternate name (user specified)         */
    char *RMJID;              /* resource manager job ID                 */

    int Index; /* job table index                         */

    time_t CTime; /* creation time (first RM report)         */
    time_t MTime; /* modification time (any source)          */
    time_t ATime; /* access time (most recent RM report)     */

    time_t StateMTime;

    time_t SWallTime; /* duration job was suspended              */
    time_t AWallTime; /* duration job was executing              */

    mjckpt_t *Ckpt; /* checkpoint structure                    */

    struct mjob_t *Next;
    struct mjob_t *Prev;

    mjobcache_t C;
    macl_t RCL[MAX_MACL]; /* required CL                              */
    mcred_t Cred;

    mqos_t *QReq;

    mres_t *R; /* reservation                              */
    char ResName[MAX_MNAME];
    char RAList[MMAX_JOBRA][MAX_MNAME]; /* reservation access list          */

    mrm_t *RM;

    int SessionID;

    char *NeedNodes;

    msdata_t *SIData; /* stage in data  */
    msdata_t *SOData; /* stage out data */

    mreq_t *Req[MAX_MREQ_PER_JOB + 1];
    mreq_t *GReq;

    mreq_t *MReq; /* primary req */

    mjobe_t E; /* job execution environment               */

    int NodeCount;      /* nodes assigned                          */
    int TaskCount;      /* tasks assigned                          */
    int TasksRequested; /* tasks requested                         */
    int NodesRequested; /* number of nodes requested               */

    mrsrc_t Alloc;
    mrsrc_t Request;

    char *Geometry;
    char SpecDistPolicy;
    char DistPolicy;

    time_t SystemQueueTime;  /* time job was initially eligible to start */
    time_t EffQueueDuration; /* duration of time job was eligible to run */

    time_t SMinTime;       /* effective earliest start time           */
    time_t SpecSMinTime;   /* user specified earliest start time      */
    time_t SysSMinTime;    /* system mandated earliest start time     */
    time_t CMaxTime;       /* user specified latest completion time   */
    time_t TermTime;       /* time by which job must be terminated    */
    time_t RMinTime;       /* earliest 'resume' time for suspend job  */
    time_t SubmitTime;     /* time job was submitted to RM            */
    time_t StartTime;      /* time job began most recent execution    */
    time_t DispatchTime;   /* time job was dispatched by RM           */
    time_t CompletionTime; /* time job execution completed            */
    time_t LastNotifyTime;
    int CompletionCode; /* execution completion code               */
    int StartCount;     /* number of times job was started         */
    int DeferCount;     /* number of times job was deferred        */
    int PreemptCount;   /* number of times job was preempted       */
    int Bypass;         /* number of times lower prio job was backfilled */

    int HoldReason;  /* reason job was deferred/held by scheduler */
    int BlockReason; /* reason job not considered for scheduling */

    time_t SyncDeadLine; /* time by which state and estate must agree */
    char *Message;       /* job event message                        */

    time_t WCLimit;  /* effective walltime limit                 */
    time_t CPULimit; /* specified CPU limit (per job)            */
    time_t SpecWCLimit[MAX_TASK_REQUESTS + 1];
    time_t RemainingTime; /* execution time job has remaining         */
    time_t SimWCTime;     /* total time sim job will consume          */

    int MinMachineSpeed; /* (in MHz) */

    int SpecPAL[MAX_MPALSIZE];
    int PAL[MAX_MPALSIZE];

    int ImageSize; /* input data size (in MB) */
    int ExecSize;  /* executable size (in MB) */

    enum MJobStateEnum State;  /* RM job state */
    enum MJobStateEnum EState; /* expected job state due to scheduler action */
    enum MJobStateEnum IState; /* internal job state */

    int Hold; /* BM { USER  SYS  BATCH }                          */
    int SuspendType;

    long SystemPrio;    /* system admin assigned priority                   */
    long UPriority;     /* job priority given by user                       */
    long StartPriority; /* priority of job to start                         */
    long RunPriority;   /* priority of job to continue running              */

    short TaskMap[MAX_MTASK_PER_JOB + 1];

    mnalloc_t *NodeList; /* list of allocated hosts */

    mnalloc_t *ReqHList; /* required hosts - {sub,super}set */
    mnalloc_t *ExcHList; /* excluded hosts                  */

    int ReqHLMode; /* req hostlist mode               */

    int RType;
    int Cluster; /* step cluster                                     */
    int Proc;    /* step proc                                        */
    char SubmitHost[MAX_MNAME]; /* job submission host */

    unsigned long IFlags;
    unsigned long Flags;
    unsigned long SpecFlags;
    unsigned long SysFlags;

    unsigned long AttrBM;

    char *MasterJobName;
    char *MasterHostName;

    double PSUtilized;  /* procseconds utilized by job                      */
    double PSDedicated; /* procseconds dedicated to job                     */
    double MSUtilized;
    double MSDedicated;

    char *RMXString;      /* RM extension string (opaque) */
    char *RMSubmitString; /* raw command file */
    int RMSubmitType;

    char *SystemID;  /* identifier of the system owner */
    char *SystemJID; /* external system identifier */

    mjdepend_t *Depend;

    int RULVTime;
    int FeatureMap[1];

#ifdef __MCPLUS
    int (*ASFunc)(mjob_t *, int, void *, void *);
#else
    int (*ASFunc)();
#endif /* __MCPLUS */

    void *ASData;

    void *xd;
} mjob_t;

typedef struct _mpar {
    char Name[MAX_MNAME + 1];

    int Index;

    int ConfigNodes;
    int IdleNodes;   /* any proc is idle                        */
    int ActiveNodes; /* any proc is busy                        */
    int UpNodes;

    mcres_t CRes; /* configured resources                    */
    mcres_t URes; /* up resources                            */
    mcres_t ARes; /* available resources                     */
    mcres_t DRes; /* dedicated resources                     */
    must_t S;     /* partition stats                         */

    mfsc_t FSC;

    mcredl_t L;
    mfs_t F;

    int BFPolicy;       /* backfill policy                         */
    int BFDepth;        /* depth queue will be searched            */
    int BFMetric;       /* backfill utilization metric             */
    int BFProcFactor;   /* factor to encourage use of large jobs   */
    int BFMaxSchedules; /* maximum schedules to consider           */
    int BFPriorityPolicy;
    int BFChunkSize;
    long BFChunkDuration;

    long BFChunkBlockTime; /* time at which jobs smaller than BFChunkSize may be
                              considered */

    /* system policies */

    int MaxMetaTasks;

    int MaxJobStartTime; /* time allowed for nodes to become busy   */
    int NAllocPolicy;    /* algo for allocating nodes               */
    int DistPolicy;
    int JobSizePolicy; /* policy for determining job size         */
    int JobNodeMatch;

    int ResPolicy;    /* policy for creating/managing reservations */
    int ResRetryTime; /* time reservations are retried if blocked by system
                         problems */
    int ResTType;     /* reservation threshold type              */
    int ResTValue;    /* reservation threshold value             */
    int ResDepth[MAX_MQOS]; /* terminated by '0'                       */
    int ResCount[MAX_MQOS];
    mqos_t *ResQOSList[MAX_MQOS]
                      [MAX_MQOS]; /* NULL terminated, MAX_MQOS global */

    /* booleans */

    int UseMachineSpeed;
    int UseMachineSpeedForFS;
    int UseSystemQueueTime;
    int UseCPUTime;
    int RejectNegPrioJobs;
    int EnableNegJobPriority;
    int EnableMultiNodeJobs;
    int EnableMultiReqJobs;

    int JobPrioAccrualPolicy;

    char NAvailPolicy[MAX_MRESOURCETYPE];

    /* resource limit handling */

    enum MResLimitPolicyEnum ResourceLimitPolicy[MAX_MRESOURCETYPE];
    enum MResLimitVActionEnum ResourceLimitViolationAction[MAX_MRESOURCETYPE];
    long ResourceLimitMaxViolationTime[MAX_MRESOURCETYPE];

    time_t AdminMinSTime;

    int UseLocalMachinePriority; /* boolean */
    int NodeLoadPolicy;
    int NodeSetPolicy;
    int NodeSetAttribute;
    char *NodeSetList[MAX_MATTR]; /* list of set delineating job attributes */
    long NodeSetDelay;
    double NodeSetTolerance;
    int NodeSetPriorityType;
    double UntrackedProcFactor;
    int NodeDownStateDelayTime;

    xres_t XRes[MAX_MNODE];

    char *Message; /* event messages */

    int IgnPbsGroupList; /* Ignore -W group_list parameter for Torque/PBS HvB */
    int FSSecondaryGroups; /* To enable secondary fairshare group lookups for
                              PBS, HvB */

} mpar_t;

typedef struct _msched{
    char Name[MAX_MNAME];
    time_t StartTime;

    time_t Time; /* current epoch time                              */

    char Version[MAX_MNAME]; /* scheduler version */

    struct timeval SchedTime; /* accurate system time at start of scheduling */
    time_t GreenwichOffset;
    int Interval; /* time transpired (hundredths of a seocnd)        */

    char Day[MAX_WORD + 1]; /* current local day of the week */
    int DayTime;           /* current local hour of the day                   */
    time_t RMPollInterval; /* interval between scheduling attempts (seconds)  */
    time_t RMJobAggregationTime;

    int Mode;     /* mode of scheduler operation                     */
    int SpecMode; /* configured scheduler operation mode             */

    int StepCount; /* number of scheduling steps before shutdown      */

    char ConfigFile[MAX_MLINE]; /* sched configuration file name */

    char *ConfigBuffer;
    char *PvtConfigBuffer;

    char LogDir[MAX_MLINE];   /* directory for logging */
    char LogFile[MAX_MLINE];  /* log file */
    char ToolsDir[MAX_MLINE]; /* directory for sched tools */
    char HomeDir[MAX_MLINE];  /* scheduler home directory */
    char LockFile[MAX_MLINE]; /* scheduler lock file */
    char KeyFile[MAX_MLINE];

    char Action[4][MAX_MLINE]; /* response for specified events */
    time_t ActionInterval;

    int LogFileMaxSize; /* maximum size of log file                        */
    int LogFileRollDepth;

    char ServerHost[MAX_MNAME]; /* name of scheduler server host */
    int ServerPort; /* socket used to communicate with sched client    */

    char FBServerHost[MAX_MNAME];
    int FBServerPort;

    time_t FBPollInterval;
    int FBFailureCount;

    int FBActive; /* boolean */

    msocket_t ServerS;
    msocket_t ServerSH; /* web interface                                   */
    int DefaultMCSocketProtocol;
    time_t ClientTimeout;

    char DefaultCSKey[MAX_MNAME];
    int DefaultCSAlgo;

    /* SSL interface */

    msocket_t SServerS;

    /* directory server */

    mpsi_t DS; /* directory service interface */
    mpsi_t EM; /* event manager interface */

    FILE *statfp;

    char
        DefaultDomain[MAX_MNAME]; /* domain appended to unqualified hostnames */
    char DefaultQMHost[MAX_MNAME];

    char Admin1User[MAX_MADMINUSERS + 1][MAX_MNAME]; /* admin usernames */
    char Admin2User[MAX_MADMINUSERS + 1][MAX_MNAME];
    char Admin3User[MAX_MADMINUSERS + 1][MAX_MNAME];
    char Admin4User[MAX_MADMINUSERS + 1][MAX_MNAME];
    char AdminHost[MAX_MADMINHOSTS + 1]
                  [MAX_MNAME]; /* hosts allowed admin access      */

    time_t DeferTime;    /* time job will stay deferred                     */
    int DeferCount;      /* times job will get deferred before being held   */
    int DeferStartCount; /* times job will fail starting before getting deferred
                            */
    time_t JobPurgeTime; /* time job must be missing before getting purged  */
    time_t NodePurgeTime;
    int APIFailureThreshhold; /* times API can fail before notifying admins */
    time_t NodeSyncDeadline;  /* time in which node must reach expected state */
    time_t JobSyncDeadline;   /* time in which job must reach expected state */
    time_t JobMaxOverrun; /* time job is allowed to exceed wallclock limit   */
    int Iteration;        /* number of scheduling cycles completed           */

    int Reload;   /* reload config file at next interval (boolean)   */
    int Schedule; /* scheduling should continue (boolean)            */
    int Shutdown;
    int EnvChanged; /* scheduling env has changed (boolean)            */

    int MaxSleepIteration; /* max iterations scheduler can go without scheduling
                              */
    char LLConfigFile[MAX_PATH_LEN];

    /* policies */

    enum MNodeAccessPolicyEnum DefaultNAccessPolicy;

    int AllocLocalityPolicy;
    int AllocLocalityGroup;

    int WCViolAction;

    int UseJobRegEx;         /* (config boolean) */
    int SPVJobIsPreemptible; /* (config boolean) */

    mbool_t EnableEncryption;
    mbool_t PercentBasedFS;

    char *Argv[MAX_MARG];

    time_t PresentTime;

    char MonitoredJob[MAX_MLINE];
    char MonitoredNode[MAX_MLINE];
    char MonitoredRes[MAX_MLINE];
    char MonitoredFunction[MAX_MLINE];

    int ResDepth;
    char DefaultClassList[MAX_MLINE];

    int SecureMode; /* (state boolean) */
    int DebugMode;  /* (state boolean) */

    time_t CurrentFSInterval;
    long DisplayFlags; /* (BM) */
    long TraceFlags;   /* (BM) */
    time_t RMLoadStart;
    int NodePollFrequency;
    int NodePollOffset;
    char ProcSpeedFeatureHeader[MAX_MNAME];
    int ProcSpeedFeatureIsVisible; /* (boolean) */
    char NodeTypeFeatureHeader[MAX_MNAME];
    int NodeTypeFeatureIsVisible; /* (boolean) */
    char PartitionFeatureHeader[MAX_MNAME];
    int PartitionFeatureIsVisible; /* (boolean) */
    int InitialLoad;
    int NodeAllocMaxPS;

    mgcred_t *DefaultU;
    mgcred_t *DefaultG;
    mgcred_t *DefaultA;
    mqos_t *DefaultQ;
    mclass_t *DefaultC;
    mnode_t DefaultN;
    mjob_t DefaultJ;

    mnode_t *GN;

    char DefaultAccountName[MAX_MNAME];
    int ReferenceProcSpeed;
    long PreemptPrioMargin;
    double NodeCPUOverCommitFactor;
    double NodeMemOverCommitFactor;
    int MaxJobPerIteration;
    time_t MinDispatchTime;
    double MaxOSCPULoad;

    int UseSyslog;    /* config boolean */
    int SyslogActive; /* state boolean */
    int SyslogFacility;

    int ResCtlPolicy;
    int ResLimitPolicy;
    int PreemptPolicy;

    int TimePolicy;
    time_t TimeOffset; /* offset between actual and trace time */

    int ResourcePrefIsActive; /* state boolean */
    pid_t PID;
    uid_t UID;
    gid_t GID;

    mpar_t *GP;

    char **IgnQList;

    void *T[mxoLAST]; /* object table   */
    int S[mxoLAST];   /* size of object */
    int M[mxoLAST];   /* number of objects in table */
    void *E[mxoLAST]; /* final object */

    void *X;

    msync_t Sync;

    char *Message; /* general system messages */

    mgrid_t G;

    char *ComputeHost[MAX_MNODE]; /* list of hosts eligible to schedule jobs */
    mnode_t *ComputeN[MAX_MNODE];

    int (*HTTPProcessF)(msocket_t *, char *);
} msched_t;

/** Struct to collect generic command line flag properties */
typedef struct _client_info {
    char  *configfile;          /**< Name of config file to use */
    int    loglevel;            /**< Log level  */
    char  *logfacility;         /**< Log facility  */
    char  *host;                /**< Host name of Maui server to contact */
    char  *keyfile;             /**< Keyfile location for authentication */
    int    port;                /**< Port number of Maui server to contact */
} client_info_t;

typedef struct {
    time_t StartTime;
    time_t EndTime;
    int TaskCount;
    int NodeCount;
    char Affinity;
} mrange_t;

/** Struct for log info */
typedef struct _mlog{
    FILE *logfp;          		/**< FILE object that identifies the stream */
    int Threshold;
    unsigned long FacilityList;
} mlog_t;

typedef mnalloc_t mnodelist_t[MAX_MREQ_PER_JOB][MAX_MNODE + 1];
typedef mnalloc_t nodelist_t[MAX_MNODE + 1];
typedef char mattrlist_t[MAX_MLIST][MAX_MATTR][MAX_WORD];

typedef struct {
    long InitTime;

    long SchedRunTime; /* elapsed time scheduler has been scheduling */
    int IdleJobs;
    int EligibleJobs;        /* number of jobs eligible for scheduling */
    int ActiveJobs;          /* number of jobs active */
    double TotalPHAvailable; /* total proc hours available to schedule */
    double TotalPHBusy;      /* total proc hours consumed by scheduled jobs */
    double SuccessfulPH;     /* proc hours completed successfully */
    int SuccessfulJobsCompleted; /* number of jobs completed successfully */
    long AvgQueuePH;             /* average queue workload */
    int JobsEvaluated;           /* Total Jobs evaluated for scheduling */
    must_t Grid[MAX_MGRIDTIMES][MAX_MGRIDSIZES]; /* stat matrix */
    must_t RTotal[MAX_MGRIDSIZES];               /* row totals */
    must_t CTotal[MAX_MGRIDSIZES];               /* column totals */
    char StatDir[MAX_MLINE + 1];                 /* Directory For Stat Files */
    double MinEff;       /* minimum scheduling efficiency */
    int MinEffIteration; /* iteration on which the minimum efficiency occurred
                            */
    double TotalSimComCost;     /* Total Simulated Communication Cost */
    int TotalFSAdjustedFSUsage; /* Total Decayed FSUsage for FairShare */
    int TotalHeterogeneousJobs;
    int TotalPreemptJobs;
    double TotalPHPreempted;

    mprofcfg_t P;
} mstat_t;

typedef struct {
    char Name[MAX_MNAME];
    int Period; /* Day || Week || Infinite */
    int Type;
    int Days;
    int Depth;
    time_t StartTime;
    time_t EndTime;
    time_t WStartTime;
    time_t WEndTime;
    macl_t ACL[MAX_MACL];
    int TaskCount;
    int TPN;
    int MaxIdleTime; /* idle WC time allowed before reservation is automatically
                        released */
    double MinLoad;
    char PName[MAX_MNAME];
    mcres_t DRes;
    mres_t *R[MAX_SRES_DEPTH];
    void *HostList;
    char HostExpression[MAX_MLINE << 4];
    int Index;
    mgcred_t *A;
    int Flags;
    double TotalTime;
    double IdleTime;
    long Priority;
    int FeatureMode;
    int FeatureMap[MAX_MATTR >> 5];

    void *O; /* owner */
    int OType;
} sres_t;

typedef struct {
    char Name[MAX_MNAME];
    int Index;

    enum MRMStateEnum State;

    char ClientName[MAX_MNAME];

    /* interface specification */

    enum MWireProtocolEnum WireProtocol;
    enum MSocketProtocolEnum SocketProtocol;

    enum MChecksumAlgoEnum CSAlgo;

    char CSKey[MAX_MNAME];

    char Host[MAX_MNAME]; /* active AM server host                           */
    int Port;             /* active AM service port                          */

    char SpecHost[MAX_MNAME]; /* specified AM server host */
    int SpecPort; /* specified AM service port                       */

    int UseDirectoryService; /* boolean */

    int Type; /* type of AM server                               */
    int Version;

    int Timeout; /* in ms */

    /* policies */

    int ChargePolicy; /* allocation charge policy                        */

    time_t FlushInterval; /* AM flush interval                               */
    time_t FlushTime;

    int DeferJobOnFailure; /* boolean */
    int AppendMachineName; /* boolean */

    char FallbackAccount[MAX_MNAME]; /* account to use if primary account is
                                        unavailable */

    time_t FailTime[MAX_MRMFAILURE];
    int FailType[MAX_MRMFAILURE];
    char *FailMsg[MAX_MRMFAILURE];
    int FailIndex;

    time_t RespTotalTime[MMAX_AMFUNC];
    time_t RespMaxTime[MMAX_AMFUNC];
    int RespTotalCount[MMAX_AMFUNC];
    time_t RespStartTime[MMAX_AMFUNC];

    msocket_t *S;
    FILE *FP;

    mpsi_t P;

    enum MJFActionEnum JFAction;
} mam_t;

typedef struct _mx_t{
    /* base functions */

    int (*JobAllocateResources)(mjob_t *, mnodelist_t, char *, mnodelist_t, int,
                                long int);
    int (*JobCheckPolicies)(mjob_t *, int, int, mpar_t *, int *, char *,
                            long int);
    int (*JobDistributeTasks)(mjob_t *, mrm_t *, mnalloc_t *, short int *);
    int (*JobFind)(char *, mjob_t **, int);
    int (*JobGetFeasibleTasks)(mjob_t *, mreq_t *, mpar_t *, nodelist_t,
                               nodelist_t, int *, int *, long int,
                               long unsigned int);
    int (*JobGetStartPriority)(mjob_t *, int, double *, int, char **, int *);
    int (*JobGetTasks)(mjob_t *, mpar_t *, nodelist_t, mnodelist_t, char *,
                       int);
    int (*JobSetCreds)(mjob_t *, char *, char *, char *);
    int (*JobSetQOS)(mjob_t *, mqos_t *, int);
    int (*JobBuildACL)(mjob_t *);
    int (*JobBuildCL)(mjob_t *);
    char *(*JobNameAdjust)(mjob_t *, char *, mrm_t *, char *, int,
                           enum MJobNameEnum);
    int (*JobStart)(mjob_t *);
    int (*JobValidate)(mjob_t *, char *, int);
    int (*JobDetermineCreds)(mjob_t *);
    int (*QOSGetAccess)(mjob_t *, mqos_t *, int *, mqos_t **);

    int (*QueuePrioritizeJobs)(mjob_t **, int *);
    int (*QueueScheduleJobs)(int *, mpar_t *);

    int (*ReservationCreate)(int, macl_t *, char *, long unsigned int,
                             mnalloc_t *, long int, long int, int, int, char *,
                             mres_t **, char *, mcres_t *);
    int (*ReservationDestroy)(mres_t **);
    int (*ReservationJCreate)(mjob_t *, mnodelist_t, long int, int, mres_t **);
    int (*ResFind)(char *, mres_t **);
    int (*AcctFind)(char *, mgcred_t **);
    int (*JobGetRange)(mjob_t *, mreq_t *, mpar_t *, long int, mrange_t *,
                       mnodelist_t, int *, char *, int, mrange_t *);
    int (*BackFill)(int *, int, mpar_t *);
    int (*DoWikiCommand)(char *, int, long, int, char *, char **, long int *,
                         int *);
    int (*JobGetSNRange)(mjob_t *, mreq_t *, mnode_t *, mrange_t *, int, char *,
                         int *, mrange_t *, mcres_t *, char *);
    int (*LL2ShowError)(int, mjob_t *);
    int (*PBSInitialize)();
    int (*RMCancelJob)(mjob_t *, char *, int *);
    int (*RMJobStart)(mjob_t *, char *, int *);
    int (*SimJobSubmit)(long int, mjob_t **, void *, int);
    int (*SRCreate)(sres_t *, int, int);
    int (*WikiLoadJob)(char *, char *, mjob_t *, short int *, mrm_t *);
    int (*QBankDoTransaction)(mam_t *, int, char *, void **, int *, char *);
    int (*JobSetAttr)(mjob_t *, enum MJobAttrEnum, void **, int, int);
    int (*ResSetAttr)(mres_t *, enum MResAttrEnum, void *, int, int);

    /* extension functions */

    int (*XInitializeMauiInterface)();
    int (*XShowConfig)(void *, int, char *, long);

    int (*XPBSNMGetData)(void *, mnode_t *, mrm_t *);

    int (*XAllocateNodes)();
    int (*XBackfill)();
    int (*XPrioritizeJobs)();
    int (*XDiagnoseJobs)();

    int (*XRMInitialize)(void *, mrm_t *);
    int (*XRMResetState)(void *, mrm_t *);
    int (*XRMVerifyData)(void *, mrm_t *, char *);
    int (*XRMDataIsStaging)(void *, void *);
    int (*XRMGetData)(void *, int);
    int (*XUpdateState)();
    int (*XRMJobSuspend)();
    int (*XRMJobResume)();
    int (*XRMJobSubmit)(void *, char *, mrm_t *, mjob_t **, char *, char *,
                        int *);

    int (*XQueueScheduleJobs)();

    int (*XLLInitialize)();
    int (*XLL2JobLoad)();
    int (*XLL2JobUpdate)();
    int (*XLL2NodeLoad)();
    int (*XLL2NodeUpdate)();

    int (*XJobProcessWikiAttr)(void *, mjob_t *, char *);
    int (*XJobGetStartPriority)(void *, mjob_t *, double *, char *);
    int (*XPBSInitialize)(void *, mrm_t *);
    int (*XPBSJobLoad)();
    int (*XWikiDoCommand)();
    int (*XWikiJobLoad)();

    int (*XMetaStoreCompletedJobInfo)(void *, mjob_t *);

    int (*XAllocMachinePrio)();
    int (*XAllocLoadBased)();

    int (*XUIHandler)(void *, msocket_t *, char *, int);

    int (*XGetClientInfo)(void *, msocket_t *, char *);
    int (*XGetTime)(void *, long *, int);
    int (*XSleep)(void *, long);

    int (*XResInit)();
    int (*XResUpdate)(void *, mres_t *);
    int (*XResDestroy)(void *, mres_t *);
    int (*XResShow)(void *, mres_t *, char *);
    char *(*XResCPCreate)(void *, mres_t *);
    int (*XResCPLoad)(void *, char *, mres_t *);

    int (*XJobPreInit)(void *, mjob_t *);
    int (*XJobPostInit)(void *, mjob_t *, int);
    int (*XJobUpdate)(void *, mjob_t *);
    int (*XJobDestroy)(void *, mjob_t **, int);
    int (*XJobShow)(void *, mjob_t *, char *);
    int (*XJobProcessRMXString)(void *, mjob_t *, char *);
    int (*XJobAllocateResources)(mjob_t *, mnodelist_t, char *, mnodelist_t,
                                 int, long);

    /* base scheduler data */

    mgcred_t **User;
    mgcred_t *Group;
    mgcred_t *Acct;
    mqos_t *MQOS;

    mattrlist_t *AttrList;
    msched_t *Sched;
    long *CREndTime;
    char *CurrentHostName;
    mlog_t *dlog;
    mfsc_t *FS;
    mjob_t **Job;
    mnode_t **MNode;
    mrange_t *MRange;
    mpar_t *MPar;
    time_t *PresentTime;
    sres_t *OSRes;
    mres_t **Res;
    mrm_t *RM;
    mam_t *AM;
    sres_t *SRes;
    mstat_t *Stat;

    /* extension data */

    void *xd;
} mx_t;

extern mlog_t mlog;

#ifndef DBG

#define DBG(X, F) if ((mlog.Threshold >= X) && ((mlog.FacilityList) & F))
#define MDB(X, F) if ((mlog.Threshold >= X) && ((mlog.FacilityList) & F))

#ifdef __MTEST
#define DPrint printf
#endif /* __MTEST */

#define MDEBUG(X) if (mlog.Threshold >= X)

/* log facility values */

#define fCORE (1 << 0)
#define fSCHED (1 << 1)
#define fSOCK (1 << 2)
#define fUI (1 << 3)
#define fLL (1 << 4)
#define fSDR (1 << 5)
#define fCONFIG (1 << 6)
#define fSTAT (1 << 7)
#define fSIM (1 << 8)
#define fSTRUCT (1 << 9)
#define fFS (1 << 10)
#define fCKPT (1 << 11)
#define fAM (1 << 12)
#define fRM (1 << 13)
#define fPBS (1 << 14)
#define fWIKI (1 << 15)
#define fSGE (1 << 16)
#define fS3 (1 << 17)
#define fLSF (1 << 18)
#define fNAT (1 << 19)

#define fALL 0xffffffff
#endif /* DBG */

/** Return copy of string in newly allocated memory. */
extern char *string_dup(const char *);

/** Safely convert a string to an integer */
extern int string2int(const char *);
#define INVALID_STRING ~0+1

/** Print client communication flag usage message */
extern void print_client_usage();

/**	Send formatted output to a stream */
extern int dPrint(char *, ...);

/** Get and save current time or update time*/
extern int getTime(time_t *, enum MTimeModeEnum, msched_t *);

extern int _MCInitialize();

extern int _MSUIPCInitialize();

extern int _MUBuildPList(mcfg_t *, char **);

extern uid_t _MOSGetEUID();

extern uid_t _MOSGetUID();

extern char *_MLogGetTime();

extern int _M64Init(m64_t *);

extern char *_MUUIDToName(uid_t);

extern int _MUStrCpy(char *, char *, int);

extern char *_MFULoad(char *, int, int, int *, int *);

extern int _MUSScanF(char *, char *, ...);

extern int _MUCheckAuthFile(msched_t *, char *, int *, int);

extern char *_MUGIDToName(gid_t);

extern int _MFUGetCachedFile(char *, char **, int *);

extern int _MOSSyslog(int, char *, ...);

extern int _MFUGetAttributes(char *, int *, time_t *, long *, uid_t *, int *,
		int *);

extern int _MFUGetInfo(char *, long *, long *, int *);

extern int _MFUCacheFile(char *, char *, int);

extern int _MCLoadConfig(char *, char *);

extern int _MCfgAdjustBuffer(char **, mbool_t);

extern int _MCfgGetSVal(char *, char **, const char *, char *, int *, char *,
		int, int, char **);

extern int _MCfgGetIVal(char *, char **, const char *, char *, int *, int *,
		char **);

extern int _MUGetIndex(char *, const char **, int, int);

extern long _MUTimeFromString(char *);

extern int _MSchedLoadConfig(char *);

extern int _MUFree(char **);

extern int _MCfgGetVal(char **, const char *, char *, int *, char *, int,
		char **);

extern int _MUStrDup(char **, char *);

extern int _MUStringToE(char *, long *);

extern char *_MUStrTok(char *, char *, char **);

extern int _MSchedSetAttr(msched_t *, int, void **, int, int);

extern int _MUStrCat(char *, char *, int);

extern int _MSchedProcessConfig(msched_t *, char *);

extern int _MUGetPair(char *, const char **, int *, char *, int, int *, char *,
		int);

extern int _MUURLParse(char *, char *, char *, char *, int, int *, mbool_t);

extern int _MUCmpFromString(char *, int *);

extern int __MCLoadEnvironment(char *ParName, char *Host, int *Port);

extern int _MXMLCreateE(mxml_t **, char *);

extern int _MXMLSetAttr(mxml_t *, char *, void *, enum MDataFormatEnum);

extern int _MXMLToString(mxml_t *, char *, int, char **, mbool_t);

extern int _MXMLDestroyE(mxml_t **);

extern int _MSecCompress(unsigned char *, unsigned int, unsigned char *, char *);

extern int _MSecCompressionGetMatch(unsigned char *, unsigned int, unsigned int, int *,
		unsigned int *, int *);

extern int _MSecEncryption(char *, char *, int );

extern int _MSecCompBufTo64BitEncoding(char *, int , char *);

extern int _MSUInitialize(msocket_t *, char *, int, long, long);

extern int _MSUConnect(msocket_t *, mbool_t, char *);

extern int _MSUDisconnect(msocket_t *);

extern int _MSUSelectWrite(int, unsigned long);

extern int _MCSendRequest(msocket_t *);

extern int _MSUSendData(msocket_t *, long, mbool_t, mbool_t);

extern int _MSURecvData(msocket_t *, long, mbool_t, enum MStatusCodeEnum *, char *);

extern int _MXMLExtractE(mxml_t *, mxml_t *, mxml_t **);

extern int _MS3CheckStatus(mxml_t *, enum MSFC *, char *);

extern int __MXMLGetChildCI(mxml_t *, char *, int *, mxml_t **);

extern int _MXMLGetChild(mxml_t *, char *, int *, mxml_t **);

extern int _MXMLFromString(mxml_t **, char *, char **, char *);

extern int _MUISCreateFrame(msocket_t *,mbool_t , mbool_t );

extern int _MUSNInit(char **, int *, char *, int);

extern int _MUSNPrintF(char **, int *, char *, ...);

extern int _MXMLGetAttr(mxml_t *, char *, int *, char *, int);

extern int _MSURecvPacket(int, char **, long, char *, long,
		enum MStatusCodeEnum *);

extern int _MSecGetChecksum(char *, int, char *, char *, enum MChecksumAlgoEnum,
		char *);

extern char *_MUPrintBuffer(char *, int);

extern char *_MUStrStr(char *, char *, int, mbool_t, mbool_t);

extern int _MSecPSDES(unsigned int *, unsigned int *);

extern unsigned short _MSecDoCRC(unsigned short , unsigned char );

extern int _MUReadPipe(char *, char *, int);

extern int _MSecMD5GetDigest(char *, int, char *, int, char *, int);

extern int _MSUSendPacket(int, char *, long, long, enum MStatusCodeEnum *);

extern int _MUStrToUpper(char *, char *, int );

extern int _MSecGetChecksum2(char *, int, char *, int, char *, char *, enum MChecksumAlgoEnum, char *);

extern int _MSecDecompress(unsigned char *, unsigned int, unsigned char *, unsigned int,
		unsigned char **, char *);

extern int _MXMLAddE(mxml_t *, mxml_t *);

extern int _MS3SetStatus(mxml_t *, char *, enum MSFC, char *);

extern int _MXMLToXString(mxml_t *, char **, int *, int, char **,
		mbool_t);

extern int _MXMLSetVal(mxml_t *, void *, enum MDataFormatEnum);

extern int _MSUSelectRead(int, unsigned long);

extern int _MUSleep(long);

extern int _MUStrNCmpCI(char *, char *, int);

extern int _MSecHMACGetDigest(unsigned char *, int, unsigned char *, int, char *, int,
		char *,mbool_t,mbool_t);

extern int _MSecComp64BitToBufDecoding(char *, int, char *, int *);

extern void _MSecSHA1Init(SHA1_CTX *);

extern void _MSecSHA1Init(SHA1_CTX *);

extern void _MSecSHA1Final(unsigned char[], SHA1_CTX *);

extern int _MSecBufTo64BitEncoding(char *, int, char *);

extern int _MSecBufToHexEncoding(char *, int, char *);

extern void _MSecHMACTruncate(char *, char *, int);

extern void _MSecSHA1Update(SHA1_CTX *, unsigned char *, MUINT4 );

extern int _MSecBufTo64BitEncoding(char *, int, char *);

#endif

/* config data */

extern mcfg_t mCfg[];

extern const char *_MSchedMode[];

extern char *_MBool[] ;

extern char *_MCredCfgParm[] ;

extern const char *_MSockProtocol[];

extern char *_MCDisplayType[] ;

extern const char *_MSchedAttr[] ;

extern char *_MComp[];

extern m64_t M64;
extern char *MParam[MAX_MCFG];

extern time_t *STime;

extern int *_MGUSyslogActive; /* boolean */

extern mccfg_t C;

extern mfcache_t MFileCache[MAX_FILECACHE];

extern mjob_t *MJobTraceBuffer;

extern mckpt_t MCP;
/* globals */

extern char *AList[32];

extern long BeginTime;
extern long EndTime ;
extern long Duration;

extern int NodeCount ;
extern int ProcCount ;
extern int TaskCount ;

extern int MIndex;
extern int Memory;
extern int DMemory;

extern int UseXML ;

extern int Mode;

extern int HType;
extern int ObjectType;

extern char ObjectID[MAX_MNAME];

extern char ShowUserName[MAX_MNAME];

extern char UserList[MAX_MLINE];
extern char GroupList[MAX_MLINE];
extern char AccountList[MAX_MLINE];
extern char QOSList[MAX_MLINE];
extern char ClassList[MAX_MLINE];

extern char QLine[MAX_MLINE];
extern char QOSName[MAX_MNAME];

extern char ResourceList[MAX_MLINE];
extern char FeatureString[MAX_MLINE];
extern char JobFeatureString[MAX_MLINE];
extern char NodeSetString[MAX_MLINE];
extern char ClassString[MAX_MLINE];

extern int RType;

extern char NodeRegEx[MAX_MLINE << 2];
extern char JobRegEx[MAX_MLINE << 2];
extern char ResList[MAX_MLINE << 2];
extern char *RegEx;

extern int PType;
extern int BFMode;
extern int DiagMode;
extern int MGridMode;
extern int JobCtlMode;
extern int BalMode;
extern int SchedMode;
extern int PrioMode;
extern char ChargeAccount[MAX_MNAME];
extern char SchedArg[MAX_MLINE];
extern char DiagOpt[MAX_MNAME];

extern long ClientTime;

extern char ParName[MAX_MNAME];
extern int QueueMode;
extern int Flags;
extern char UserName[MAX_MNAME];
extern char Group[MAX_MNAME];
extern char Account[MAX_MNAME];
extern char Type[MAX_MLINE];
extern char RangeList[MAX_MLINE];
extern char ResDesc[MAX_MLINE];
extern char ResFlags[MAX_MLINE];

extern msocket_t Msg;
extern char MsgBuffer[MAX_MLINE << 3];

extern msched_t MSched;

extern const char *_MJobCtlCmds[];

extern const char *_MCKeyword[];

