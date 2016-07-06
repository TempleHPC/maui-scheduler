#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <syslog.h>
#include <errno.h>
#include <sys/stat.h>
#include <locale.h>
#include <math.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#include "maui_utils.h"

static void __MSecSHA1Transform(MUINT4[], unsigned char[]);

int _MSUClientCount = 0; /* keeps a count of number of connected clients */

char tmpSBuf[MMSG_BUFFER]; /* NOTE: global to avoid compiler-specific stack
                              failures */
mx_t X;

/* config data */

mcfg_t mCfg[] = {
    {NONE, pParamNONE, mdfString, -1, NULL},
    {"LOGLEVEL", pLogLevel, mdfInt, mxoSched, NULL},
    {"LOGDIR", pSchedLogDir, mdfString, mxoSched, NULL},
    {"LOGFACILITY", pLogFacility, mdfStringArray, mxoSched, NULL},
    {"LOGFILEMAXSIZE", pLogFileMaxSize, mdfInt, mxoSched, NULL},
    {"LOGFILE", pSchedLogFile, mdfString, mxoSched, NULL},
    {"LOGFILEROLLDEPTH", pLogFileRollDepth, mdfInt, mxoSched, NULL},
    {"ACCOUNTCAP", pCACap, mdfInt, mxoPar, NULL},
    {"ACCOUNTWEIGHT", pCAWeight, mdfInt, mxoPar, NULL},
    {"ADMIN1", mcoAdmin1Users, mdfStringArray, mxoSched, NULL},
    {"ADMIN2", mcoAdmin2Users, mdfStringArray, mxoSched, NULL},
    {"ADMIN3", mcoAdmin3Users, mdfStringArray, mxoSched, NULL},
    {"ADMIN4", mcoAdmin4Users, mdfStringArray, mxoSched, NULL},
    {"ADMINHOSTS", mcoAdminHosts, mdfStringArray, mxoSched, NULL},
    {"ADMINS", mcoAdminUsers, mdfStringArray, mxoSched, NULL},
    {"AFSWEIGHT", pOLDAFSWeight, mdfInt, mxoPar, NULL},
    {"APIFAILURETHRESHHOLD", pAPIFailureThreshhold, mdfInt, mxoSched, NULL},
    {"ATTRWEIGHT", pAttrWeight, mdfString, mxoPar, NULL},
    {"ATTRCAP", pAttrCap, mdfString, mxoPar, NULL},
    {"ATTRSTATEWEIGHT", pAJobStateWeight, mdfString, mxoPar, NULL},
    {"ATTRATTRWEIGHT", pAJobAttrWeight, mdfString, mxoPar, NULL},
    {"ATTRSTATECAP", pAJobStateCap, mdfString, mxoPar, NULL},
    {"ATTRATTRCAP", pAJobAttrCap, mdfString, mxoPar, NULL},
    {"BACKFILLDEPTH", pBFDepth, mdfInt, mxoPar, NULL},
    {"BACKFILLMAXSCHEDULES", pBFMaxSchedules, mdfInt, mxoPar, NULL},
    {"BACKFILLMETRIC", pBFMetric, mdfString, mxoPar, NULL},
    {"BACKFILLPOLICY", pBFPolicy, mdfString, mxoPar, NULL},
    {"BACKFILLPROCFACTOR", pBFProcFactor, mdfInt, mxoPar, NULL},
    {"BACKFILLTYPE", pBFType, mdfString, mxoPar, NULL},
    {"BANKAPPENDMACHINENAME", pAMAppendMachineName, mdfString, mxoAM, NULL},
    {"BANKCHARGEPOLICY", pAMChargePolicy, mdfString, mxoAM, NULL},
    {"BANKDEFERONJOBFAILURE", pAMDeferOnJobFailure, mdfString, mxoAM, NULL},
    {"BANKFALLBACKACCOUNT", pAMFallbackAccount, mdfString, mxoAM, NULL},
    {"BANKFLUSHINTERVAL", pAMFlushInterval, mdfString, mxoAM, NULL},
    {"BANKHOST", pAMHost, mdfString, mxoAM, NULL},
    {"BANKPORT", pAMPort, mdfInt, mxoAM, NULL},
    {"BANKPROTOCOL", pAMProtocol, mdfString, mxoAM, NULL},
    {"BANKSERVER", pOLDBankServer, mdfString, mxoAM, NULL},
    {"BANKTIMEOUT", pAMTimeout, mdfString, mxoAM, NULL},
    {"BANKTYPE", pAMType, mdfString, mxoAM, NULL},
    {"BFCHUNKDURATION", mcoBFChunkDuration, mdfString, mxoPar, NULL},
    {"BFCHUNKSIZE", mcoBFChunkSize, mdfInt, mxoPar, NULL},
    {"BFPRIORITYPOLICY", pBFPriorityPolicy, mdfString, mxoPar, NULL},
    {"BYPASSCAP", pSBPCap, mdfInt, mxoPar, NULL},
    {"BYPASSWEIGHT", pSBPWeight, mdfInt, mxoPar, NULL},
    {"CHECKPOINTEXPIRATIONTIME", pCheckPointExpirationTime, mdfString, mxoSched,
     NULL},
    {"CHECKPOINTFILE", pCheckPointFile, mdfString, mxoSched, NULL},
    {"CHECKPOINTINTERVAL", pCheckPointInterval, mdfString, mxoSched, NULL},
    {"CLASSCAP", pCCCap, mdfInt, mxoPar, NULL},
    {"CLASSWEIGHT", pCCWeight, mdfInt, mxoPar, NULL},
    {"CLIENTCFG", pClientCfg, mdfString, mxoSched, NULL},
    {"CLIENTTIMEOUT", pClientTimeout, mdfString, mxoSched, NULL},
    {"COMINTERFRAMECOST", pInterFrameCost, mdfDouble, mxoSim, NULL},
    {"COMINTRAFRAMECOST", pIntraFrameCost, mdfDouble, mxoSim, NULL},
    {"COMPUTEHOSTS", mcoComputeHosts, mdfStringArray, mxoSched, NULL},
    {"CONSUMEDCAP", pUConsCap, mdfInt, mxoPar, NULL},
    {"CONSUMEDWEIGHT", pUConsWeight, mdfInt, mxoPar, NULL},
    {"CREDCAP", pCredCap, mdfInt, mxoPar, NULL},
    {"CREDWEIGHT", pCredWeight, mdfString, mxoPar, NULL},
    {"DEFAULTACCOUNT", pDefaultAccountName, mdfString, mxoSched, NULL},
    {"DEFAULTCLASSLIST", pDefaultClassList, mdfStringArray, mxoSched, NULL},
    {"DEFAULTDOMAIN", pDefaultDomain, mdfString, mxoSched, NULL},
    {"DEFAULTQMHOST", pDefaultQMHost, mdfString, mxoSched, NULL},
    {"DEFERCOUNT", pDeferCount, mdfInt, mxoSched, NULL},
    {"DEFERSTARTCOUNT", pDeferStartCount, mdfInt, mxoSched, NULL},
    {"DEFERTIME", mcoDeferTime, mdfString, mxoSched, NULL},
    {"DIRECTORYSERVER", mcoDirectoryServer, mdfString, mxoSched, NULL},
    {"DIRECTSPECWEIGHT", pOLDDirectSpecWeight, mdfInt, mxoPar, NULL},
    {"DISKCAP", pRDiskCap, mdfInt, mxoPar, NULL},
    {"DISKWEIGHT", pRDiskWeight, mdfInt, mxoPar, NULL},
    {"DISPLAYFLAGS", pDisplayFlags, mdfStringArray, mxoSched, NULL},
    {"ENABLEMULTINODEJOBS", mcoEnableMultiNodeJobs, mdfString, mxoPar, NULL},
    {"ENABLEMULTIREQJOBS", mcoEnableMultiReqJobs, mdfString, mxoPar, NULL},
    {"ENABLENEGJOBPRIORITY", mcoEnableNegJobPriority, mdfString, mxoPar, NULL},
    {"EVENTSERVER", mcoEventServer, mdfString, mxoSched, NULL},
    {"FEATURENODETYPEHEADER", pNodeTypeFeatureHeader, mdfString, mxoSched,
     NULL},
    {"FEATUREPARTITIONHEADER", pPartitionFeatureHeader, mdfString, mxoSched,
     NULL},
    {"FEATUREPROCSPEEDHEADER", pProcSpeedFeatureHeader, mdfString, mxoSched,
     NULL},
    {"FEEDBACKPROGRAM", mcoJobFBAction, mdfString, mxoSched, NULL},
    {"ALLOCLOCALITYPOLICY", mcoAllocLocalityPolicy, mdfString, mxoSched, NULL},
    {"FSACCOUNTCAP", pFACap, mdfInt, mxoPar, NULL},
    {"FSACCOUNTWEIGHT", pFAWeight, mdfInt, mxoPar, NULL},
    {"FSACCOUNTWEIGHT", pOLDFSAWeight, mdfInt, mxoPar, NULL},
    {"FSCAP", pFSCap, mdfInt, mxoPar, NULL},
    {"FSCLASSCAP", pFCCap, mdfInt, mxoPar, NULL},
    {"FSCLASSWEIGHT", pFCWeight, mdfInt, mxoPar, NULL},
    {"FSCLASSWEIGHT", pOLDFSCWeight, mdfInt, mxoPar, NULL},
    {"FSCONFIGFILE", pFSConfigFile, mdfString, mxoPar, NULL},
    {"FSDECAY", pFSDecay, mdfDouble, mxoPar, NULL},
    {"FSDEPTH", pFSDepth, mdfInt, mxoPar, NULL},
    {"FSENFORCEMENT", pFSEnforcement, mdfString, mxoPar, NULL},
    {"FSGROUPCAP", pFGCap, mdfInt, mxoPar, NULL},
    {"FSGROUPWEIGHT", pFGWeight, mdfInt, mxoPar, NULL},
    {"FSGROUPWEIGHT", pOLDFSGWeight, mdfInt, mxoPar, NULL},
    {"FSINTERVAL", pFSInterval, mdfString, mxoPar, NULL},
    {"FSPOLICY", pFSPolicy, mdfString, mxoPar, NULL},
    {"FSQOSCAP", pFQCap, mdfInt, mxoPar, NULL},
    {"FSQOSWEIGHT", pFQWeight, mdfInt, mxoPar, NULL},
    {"FSQOSWEIGHT", pOLDFSQWeight, mdfInt, mxoPar, NULL},
    {"FSSECONDARYGROUPS", pFSSecondaryGroups, mdfString, mxoPar, NULL},
    {"FSUSERCAP", pFUCap, mdfInt, mxoPar, NULL},
    {"FSUSERWEIGHT", pFUWeight, mdfInt, mxoPar, NULL},
    {"FSUSERWEIGHT", pOLDFSUWeight, mdfInt, mxoPar, NULL},
    {"FSWEIGHT", pFSWeight, mdfString, mxoPar, NULL},
    {"GFSWEIGHT", pOLDGFSWeight, mdfInt, mxoPar, NULL},
    {"GROUPCAP", pCGCap, mdfInt, mxoPar, NULL},
    {"GROUPWEIGHT", pCGWeight, mdfInt, mxoPar, NULL},
    {"IGNPBSGROUPLIST", pIgnPbsGroupList, mdfString, mxoPar, NULL},
    {"JOBAGGREGATIONTIME", pJobAggregationTime, mdfString, mxoSched, NULL},
    {"JOBMAXOVERRUN", pJobMaxOverrun, mdfString, mxoSched, NULL},
    {"JOBMAXSTARTTIME", pMaxJobStartTime, mdfString, mxoPar, NULL},
    {"JOBNODEMATCHPOLICY", pJobNodeMatch, mdfStringArray, mxoPar, NULL},
    {"JOBPRIOACCRUALPOLICY", pJobPrioAccrualPolicy, mdfString, mxoPar, NULL},
    {"JOBPRIOF", mcoJobAttrPrioF, mdfString, mxoPar, NULL},
    {"JOBPURGETIME", pJobPurgeTime, mdfString, mxoSched, NULL},
    {"JOBSIZEPOLICY", pJobSizePolicy, mdfString, mxoPar, NULL},
    {"JOBSYNCTIME", pJobSyncDeadline, mdfString, mxoSched, NULL},
    {"LOCKFILE", pSchedLockFile, mdfString, mxoSched, NULL},
    {"MACHINECONFIGFILE", pMachineConfigFile, mdfString, mxoSched, NULL},
    {"MAILPROGRAM", mcoMailAction, mdfString, mxoSched, NULL},
    {"MAXJOBPERACCOUNTCOUNT", pHMaxJobPerAccountCount, mdfInt, mxoSched, NULL},
    {"MAXJOBPERACCOUNTPOLICY", pMaxJobPerAccountPolicy, mdfString, mxoSched,
     NULL},
    {"MAXJOBPERGROUPCOUNT", pHMaxJobPerGroupCount, mdfInt, mxoSched, NULL},
    {"MAXJOBPERGROUPPOLICY", pMaxJobPerGroupPolicy, mdfString, mxoSched, NULL},
    {"MAXJOBPERITERATION", pMaxJobPerIteration, mdfInt, mxoSched, NULL},
    {"MAXJOBPERUSERCOUNT", pHMaxJobPerUserCount, mdfInt, mxoSched, NULL},
    {"MAXJOBPERUSERPOLICY", pMaxJobPerUserPolicy, mdfString, mxoSched, NULL},
    {"MAXJOBQUEUEDPERACCOUNTCOUNT", pHMaxJobQueuedPerAccountCount, mdfInt,
     mxoSched, NULL},
    {"MAXJOBQUEUEDPERACCOUNTPOLICY", pMaxJobQueuedPerAccountPolicy, mdfString,
     mxoSched, NULL},
    {"MAXJOBQUEUEDPERGROUPCOUNT", pHMaxJobQueuedPerGroupCount, mdfInt, mxoSched,
     NULL},
    {"MAXJOBQUEUEDPERGROUPPOLICY", pMaxJobQueuedPerGroupPolicy, mdfString,
     mxoSched, NULL},
    {"MAXJOBQUEUEDPERUSERCOUNT", pHMaxJobQueuedPerUserCount, mdfInt, mxoSched,
     NULL},
    {"MAXJOBQUEUEDPERUSERPOLICY", pMaxJobQueuedPerUserPolicy, mdfString,
     mxoSched, NULL},
    {"MAXNODEPERUSERCOUNT", pHMaxNodePerUserCount, mdfInt, mxoSched, NULL},
    {"MAXNODEPERUSERPOLICY", pMaxNodePerUserPolicy, mdfString, mxoSched, NULL},
    {"MAXPEPERUSERCOUNT", pHMaxPEPerUserCount, mdfInt, mxoSched, NULL},
    {"MAXPEPERUSERPOLICY", pMaxPEPerUserPolicy, mdfString, mxoSched, NULL},
    {"MAXPROCPERACCOUNTCOUNT", pHMaxNodePerAccountCount, mdfInt, mxoSched,
     NULL},
    {"MAXPROCPERACCOUNTPOLICY", pMaxNodePerAccountPolicy, mdfString, mxoSched,
     NULL},
    {"MAXPROCPERGROUPCOUNT", pHMaxNodePerGroupCount, mdfInt, mxoSched, NULL},
    {"MAXPROCPERGROUPPOLICY", pMaxNodePerGroupPolicy, mdfString, mxoSched,
     NULL},
    {"MAXPROCPERUSERCOUNT", pHMaxProcPerUserCount, mdfInt, mxoSched, NULL},
    {"MAXPROCPERUSERPOLICY", pMaxProcPerUserPolicy, mdfString, mxoSched, NULL},
    {"MAXPROCSECONDPERACCOUNTCOUNT", pHMaxPSPerAccountCount, mdfInt, mxoSched,
     NULL},
    {"MAXPROCSECONDPERACCOUNTPOLICY", pMaxPSPerAccountPolicy, mdfString,
     mxoSched, NULL},
    {"MAXPROCSECONDPERGROUPCOUNT", pHMaxPSPerGroupCount, mdfInt, mxoSched,
     NULL},
    {"MAXPROCSECONDPERGROUPPOLICY", pMaxPSPerGroupPolicy, mdfString, mxoSched,
     NULL},
    {"MAXPROCSECONDPERUSERCOUNT", pHMaxPSPerUserCount, mdfInt, mxoSched, NULL},
    {"MAXPROCSECONDPERUSERPOLICY", pMaxPSPerUserPolicy, mdfString, mxoSched,
     NULL},
    {"MAXSLEEPITERATION", pMaxSleepIteration, mdfInt, mxoSched, NULL},
    {"MCSOCKETPROTOCOL", pMCSocketProtocol, mdfString, mxoSched, NULL},
    {"MEMCAP", pRMemCap, mdfInt, mxoPar, NULL},
    {"MEMWEIGHT", pRMemWeight, mdfInt, mxoPar, NULL},
    {"METAMAXTASKS", pMaxMetaTasks, mdfInt, mxoSched, NULL},
    {"MINADMINSTIME", mcoAdminMinSTime, mdfString, mxoSched, NULL},
    {"MINDISPATCHTIME", pMinDispatchTime, mdfString, mxoSched, NULL},
    {"NODEACCESSPOLICY", pNAPolicy, mdfString, mxoSched, NULL},
    {"NODEALLOCMAXPS", pNAMaxPS, mdfString, mxoSched, NULL},
    {"NODEALLOCATIONPOLICY", pNodeAllocationPolicy, mdfString, mxoPar, NULL},
    {"NODECAP", pRNodeCap, mdfInt, mxoPar, NULL},
    {"NODECPUOVERCOMMITFACTOR", pNodeCPUOverCommitFactor, mdfDouble, mxoSched,
     NULL},
    {"NODEDOWNSTATEDELAYTIME", pNodeDownStateDelayTime, mdfString, mxoSched,
     NULL},
    {"NODELOADPOLICY", pNodeLoadPolicy, mdfString, mxoPar, NULL},
    {"NODEMAXLOAD", pNodeMaxLoad, mdfDouble, mxoSched, NULL},
    {"NODEMEMOVERCOMMITFACTOR", pNodeMemOverCommitFactor, mdfDouble, mxoSched,
     NULL},
    {"NODEPOLLFREQUENCY", pNodePollFrequency, mdfInt, mxoSched, NULL},
    {"NODEPURGETIME", pNodePurgeTime, mdfString, mxoSched, NULL},
    {"NODESETATTRIBUTE", pNodeSetAttribute, mdfString, mxoPar, NULL},
    {"NODESETDELAY", pNodeSetDelay, mdfString, mxoPar, NULL},
    {"NODESETLIST", pNodeSetList, mdfStringArray, mxoPar, NULL},
    {"NODESETPOLICY", pNodeSetPolicy, mdfString, mxoPar, NULL},
    {"NODESETPRIORITYTYPE", pNodeSetPriorityType, mdfString, mxoPar, NULL},
    {"NODESETTOLERANCE", pNodeSetTolerance, mdfDouble, mxoPar, NULL},
    {"NODESYNCTIME", pNodeSyncDeadline, mdfString, mxoSched, NULL},
    {"NODEUNTRACKEDLOADFACTOR", pNodeUntrackedProcFactor, mdfDouble, mxoSched,
     NULL},
    {"NODEWEIGHT", pRNodeWeight, mdfInt, mxoPar, NULL},
    {"NOTIFICATIONINTERVAL", pAdminEInterval, mdfString, mxoSched, NULL},
    {"NOTIFICATIONPROGRAM", pAdminEAction, mdfString, mxoSched, NULL},
    {"PARIGNQUEUELIST", pParIgnQList, mdfStringArray, mxoSched, NULL},
    {"PECAP", pRPECap, mdfInt, mxoPar, NULL},
    {"PERCENTCAP", pUPerCCap, mdfInt, mxoPar, NULL},
    {"PERCENTWEIGHT", pUPerCWeight, mdfInt, mxoPar, NULL},
    {"PEWEIGHT", pRPEWeight, mdfInt, mxoPar, NULL},
    {"PLOTMAXPROC", pPlotMaxNode, mdfInt, mxoSched, NULL},
    {"PLOTMAXTIME", pPlotMaxTime, mdfString, mxoSched, NULL},
    {"PLOTMINPROC", pPlotMinNode, mdfInt, mxoSched, NULL},
    {"PLOTMINTIME", pPlotMinTime, mdfString, mxoSched, NULL},
    {"PLOTPROCSCALE", pPlotNodeScale, mdfInt, mxoSched, NULL},
    {"PLOTTIMESCALE", pPlotTimeScale, mdfInt, mxoSched, NULL},
    {"PREEMPTPOLICY", pPreemptPolicy, mdfString, mxoSched, NULL},
    {"PROCCAP", pRProcCap, mdfInt, mxoPar, NULL},
    {"PROCWEIGHT", pRProcWeight, mdfInt, mxoPar, NULL},
    {"PSCAP", pRPSCap, mdfInt, mxoPar, NULL},
    {"PSWEIGHT", pRPSWeight, mdfInt, mxoPar, NULL},
    {"QOSCAP", pCQCap, mdfInt, mxoPar, NULL},
    {"QOSFLAGS", pQOSFlags, mdfStringArray, mxoQOS, NULL},
    {"QOSNAME", pQOSName, mdfString, mxoQOS, NULL},
    {"QOSPRIORITY", pQOSPriority, mdfInt, mxoQOS, NULL},
    {"QOSQTWEIGHT", pQOSQTWeight, mdfInt, mxoQOS, NULL},
    {"QOSTARGETQT", pQOSTargetQT, mdfInt, mxoQOS, NULL},
    {"QOSTARGETXF", pQOSTargetXF, mdfDouble, mxoQOS, NULL},
    {"QOSWEIGHT", pCQWeight, mdfInt, mxoPar, NULL},
    {"QOSXFWEIGHT", pQOSXFWeight, mdfInt, mxoQOS, NULL},
    {"QUEUETIMECAP", pSQTCap, mdfInt, mxoPar, NULL},
    {"QUEUETIMETARGETWEIGHT", pOLDQTWeight, mdfInt, mxoPar, NULL},
    {"QUEUETIMEWEIGHT", pSQTWeight, mdfInt, mxoPar, NULL},
    {"REJECTNEGPRIOJOBS", mcoRejectNegPrioJobs, mdfString, mxoPar, NULL},
    {"REMAININGCAP", pURemCap, mdfInt, mxoPar, NULL},
    {"REMAININGWEIGHT", pURemWeight, mdfInt, mxoPar, NULL},
    {"REMOTESTARTCOMMAND", pRemoteStartCommand, mdfString, mxoSched, NULL},
    {"RESCAP", pResCap, mdfInt, mxoPar, NULL},
    {"RESCTLPOLICY", pResCtlPolicy, mdfString, mxoSched, NULL},
    {"RESDEPTH", pResDepth, mdfInt, mxoSched, NULL},
    {"RESERVATIONDEPTH", pReservationDepth, mdfInt, mxoSched, NULL},
    {"RESERVATIONPOLICY", pResPolicy, mdfString, mxoPar, NULL},
    {"RESERVATIONQOSLIST", pResQOSList, mdfStringArray, mxoSched, NULL},
    {"RESERVATIONRETRYTIME", pResRetryTime, mdfString, mxoPar, NULL},
    {"RESERVATIONTHRESHOLDTYPE", pResThresholdType, mdfString, mxoPar, NULL},
    {"RESERVATIONTHRESHOLDVALUE", pResThresholdValue, mdfInt, mxoPar, NULL},
    {"RESLIMITPOLICY", mcoResLimitPolicy, mdfString, mxoSched, NULL},
    {"NODEAVAILABILITYPOLICY", pNodeAvailPolicy, mdfStringArray, mxoPar, NULL},
    {"RESOURCECOLLECTIONPOLICY", pResourceCollectionPolicy, mdfString, mxoSched,
     NULL},
    {"RESOURCEDAEMON", pResourceCommand, mdfString, mxoSched, NULL},
    {"RESOURCEDATAFILE", pResourceDataFile, mdfString, mxoSched, NULL},
    {"RESOURCELIMITPOLICY", mcoResourceLimitPolicy, mdfStringArray, mxoPar,
     NULL},
    {"RESOURCELOCKFILE", pResourceLockFile, mdfString, mxoSched, NULL},
    {"RESOURCEPORT", pResourcePort, mdfInt, mxoSched, NULL},
    {"RESOURCEREPORTWAIT", pResourceReportWait, mdfInt, mxoSched, NULL},
    {"RESOURCESAMPLEINTERVAL", pResourceSampleInterval, mdfInt, mxoSched, NULL},
    {"RESWEIGHT", pResWeight, mdfString, mxoPar, NULL},
    {"RMAUTHTYPE", pRMAuthType, mdfString, mxoRM, NULL},
    {"RMCONFIGFILE", pRMConfigFile, mdfString, mxoRM, NULL},
    {"RMEPORT", mcoRMEPort, mdfInt, mxoRM, NULL},
    {"RMHOST", pRMHost, mdfString, mxoRM, NULL},
    {"RMLOCALDISKFS", pRMLocalDiskFS, mdfString, mxoRM, NULL},
    {"RMNAME", pRMName, mdfString, mxoRM, NULL},
    {"RMNMPORT", pRMNMPort, mdfInt, mxoRM, NULL},
    {"RMPOLLINTERVAL", pRMPollInterval, mdfString, mxoSched, NULL},
    {"RMPORT", pRMPort, mdfInt, mxoRM, NULL},
    {"RMSERVER", pOLDRMServer, mdfString, mxoRM, NULL},
    {"RMTIMEOUT", pRMTimeout, mdfString, mxoRM, NULL},
    {"RMTYPE", pRMType, mdfString, mxoRM, NULL},
    {"SERVERCONFIGFILE", pSchedConfigFile, mdfString, mxoSched, NULL},
    {"SERVERHOMEDIR", pMServerHomeDir, mdfString, mxoSched, NULL},
    {"SERVERHOST", pServerHost, mdfString, mxoSched, NULL},
    {"SERVERMODE", pSchedMode, mdfString, mxoSched, NULL},
    {"SERVERNAME", pServerName, mdfString, mxoSched, NULL},
    {"SERVERPORT", pServerPort, mdfInt, mxoSched, NULL},
    {"SERVICETARGETWEIGHT", pOLDServWeight, mdfInt, mxoPar, NULL},
    {"SERVICEWCAP", pServCap, mdfInt, mxoPar, NULL},
    {"SERVICEWEIGHT", pServWeight, mdfString, mxoPar, NULL},
    {"SIMAUTOSHUTDOWN", pSimAutoShutdown, mdfString, mxoSim, NULL},
    {"SIMCHECKPOINTINTERVAL", pSimCheckpointInterval, mdfString, mxoSim, NULL},
    {"SIMCOMRATE", pComRate, mdfDouble, mxoSim, NULL},
    {"SIMCOMTYPE", pCommunicationType, mdfString, mxoSim, NULL},
    {"SIMDEFAULTJOBFLAGS", pSimDefaultJobFlags, mdfStringArray, mxoSim, NULL},
    {"SIMEXITITERATION", pExitIteration, mdfInt, mxoSim, NULL},
    {"SIMFLAGS", pSimFlags, mdfStringArray, mxoSim, NULL},
    {"SIMIGNOREJOBFLAGS", pSimIgnoreJobFlags, mdfStringArray, mxoSim, NULL},
    {"SIMINITIALQUEUEDEPTH", pSimInitialQueueDepth, mdfInt, mxoSim, NULL},
    {"SIMJOBSUBMISSIONPOLICY", pSimJobSubmissionPolicy, mdfString, mxoSim,
     NULL},
    {"SIMNODECONFIGURATION", pSimNCPolicy, mdfString, mxoSim, NULL},
    {"SIMNODECOUNT", pSimNodeCount, mdfInt, mxoSim, NULL},
    {"SIMRANDOMIZEJOBSIZE", pRandomizeJobSize, mdfString, mxoSim, NULL},
    {"SIMRESOURCETRACEFILE", pResourceTraceFile, mdfString, mxoSim, NULL},
    {"SIMSCALEJOBRUNTIME", pSimScaleJobRunTime, mdfString, mxoSim, NULL},
    {"SIMSTARTTIME", pSimStartTime, mdfString, mxoSim, NULL},
    {"SIMSTOPITERATION", pStopIteration, mdfInt, mxoSim, NULL},
    {"SIMTIMEPOLICY", mcoTimePolicy, mdfString, mxoSim, NULL},
    {"SIMWCACCURACYCHANGE", pSimWCAccuracyChange, mdfDouble, mxoSim, NULL},
    {"SIMWCACCURACY", pSimWCAccuracy, mdfDouble, mxoSim, NULL},
    {"SIMWCSCALINGPERCENT", pSimWCScalingPercent, mdfInt, mxoSim, NULL},
    {"SIMWORKLOADTRACEFILE", pWorkloadTraceFile, mdfString, mxoSim, NULL},
    {"SMAXJOBPERACCOUNTCOUNT", pMaxJobPerAccountCount, mdfInt, mxoSched, NULL},
    {"SMAXJOBPERGROUPCOUNT", pMaxJobPerGroupCount, mdfInt, mxoSched, NULL},
    {"SMAXJOBPERUSERCOUNT", pMaxJobPerUserCount, mdfInt, mxoSched, NULL},
    {"SMAXJOBQUEUEDPERACCOUNTCOUNT", pMaxJobQueuedPerAccountCount, mdfInt,
     mxoSched, NULL},
    {"SMAXJOBQUEUEDPERGROUPCOUNT", pMaxJobQueuedPerGroupCount, mdfInt, mxoSched,
     NULL},
    {"SMAXJOBQUEUEDPERUSERCOUNT", pMaxJobQueuedPerUserCount, mdfInt, mxoSched,
     NULL},
    {"SMAXNODEPERUSERCOUNT", pMaxNodePerUserCount, mdfInt, mxoSched, NULL},
    {"SMAXPEPERUSERCOUNT", pMaxPEPerUserCount, mdfInt, mxoSched, NULL},
    {"SMAXPROCPERACCOUNTCOUNT", pMaxNodePerAccountCount, mdfInt, mxoSched,
     NULL},
    {"SMAXPROCPERGROUPCOUNT", pMaxNodePerGroupCount, mdfInt, mxoSched, NULL},
    {"SMAXPROCPERUSERCOUNT", pMaxProcPerUserCount, mdfInt, mxoSched, NULL},
    {"SMAXPROCSECONDPERACCOUNTCOUNT", pMaxPSPerAccountCount, mdfInt, mxoSched,
     NULL},
    {"SMAXPROCSECONDPERGROUPCOUNT", pMaxPSPerGroupCount, mdfInt, mxoSched,
     NULL},
    {"SMAXPROCSECONDPERUSERCOUNT", pMaxPSPerUserCount, mdfInt, mxoSched, NULL},
    {"SPVIOLATIONCAP", pSSPVCap, mdfInt, mxoPar, NULL},
    {"SPVIOLATIONWEIGHT", pSSPVWeight, mdfInt, mxoPar, NULL},
    {"SRACCESS", pSRAccess, mdfString, mxoSRes, NULL},
    {"SRACCOUNTLIST", pSRAccountList, mdfStringArray, mxoSRes, NULL},
    {"SRCHARGEACCOUNT", pSRChargeAccount, mdfString, mxoSRes, NULL},
    {"SRCLASSLIST", pSRClassList, mdfStringArray, mxoSRes, NULL},
    {"SRDAYS", pSRDays, mdfStringArray, mxoSRes, NULL},
    {"SRDEPTH", pSRDepth, mdfInt, mxoSRes, NULL},
    {"SRENDTIME", pSREndTime, mdfString, mxoSRes, NULL},
    {"SRFEATURES", pSRFeatures, mdfStringArray, mxoSRes, NULL},
    {"SRFLAGS", pSRFlags, mdfStringArray, mxoSRes, NULL},
    {"SRGROUPLIST", pSRGroupList, mdfStringArray, mxoSRes, NULL},
    {"SRHOSTLIST", pSRHostList, mdfStringArray, mxoSRes, NULL},
    {"SRIDLETIME", pSRIdleTime, mdfString, mxoSRes, NULL},
    {"SRMAXTASKS", pSRMaxTasks, mdfInt, mxoSRes, NULL},
    {"SRMAXTIME", pSRMaxTime, mdfString, mxoSRes, NULL},
    {"SRMINLOAD", pSRMinLoad, mdfString, mxoSRes, NULL},
    {"SRMINTASKS", pSRMinTasks, mdfInt, mxoSRes, NULL},
    {"SRNAME", pSRName, mdfString, mxoSRes, NULL},
    {"SRPARTITION", pSRPartition, mdfString, mxoSRes, NULL},
    {"SRPERIOD", pSRPeriod, mdfString, mxoSRes, NULL},
    {"SRPRIORITY", pSRPriority, mdfInt, mxoSRes, NULL},
    {"SRQOSLIST", pSRQOSList, mdfStringArray, mxoSRes, NULL},
    {"SRRESOURCES", pSRResources, mdfString, mxoSRes, NULL},
    {"SRSTARTTIME", pSRStartTime, mdfString, mxoSRes, NULL},
    {"SRTASKCOUNT", pSRTaskCount, mdfInt, mxoSRes, NULL},
    {"SRTIMELOGIC", pSRTimeLogic, mdfString, mxoSRes, NULL},
    {"SRTPN", pSRTPN, mdfInt, mxoSRes, NULL},
    {"SRUSERLIST", pSRUserList, mdfStringArray, mxoSRes, NULL},
    {"SRWENDTIME", pSRWEndTime, mdfString, mxoSRes, NULL},
    {"SRWSTARTTIME", pSRWStartTime, mdfString, mxoSRes, NULL},
    {"STATDIR", pStatDir, mdfString, mxoSched, NULL},
    {"STEPCOUNT", pSchedStepCount, mdfInt, mxoSched, NULL},
    {"SWAPCAP", pRSwapCap, mdfInt, mxoPar, NULL},
    {"SWAPWEIGHT", pRSwapWeight, mdfInt, mxoPar, NULL},
    {"SYSTEMMAXJOBWALLTIME", pSystemMaxJobTime, mdfString, mxoPar, NULL},
    {"SYSTEMMAXPROCPERJOB", pSystemMaxJobProc, mdfInt, mxoPar, NULL},
    {"SYSTEMMAXPROCSECONDPERJOB", pSystemMaxJobPS, mdfInt, mxoPar, NULL},
    {"TARGETCAP", pTargCap, mdfInt, mxoPar, NULL},
    {"TARGETQUEUETIMECAP", pTQTCap, mdfInt, mxoPar, NULL},
    {"TARGETQUEUETIMEWEIGHT", pTQTWeight, mdfInt, mxoPar, NULL},
    {"TARGETWEIGHT", pTargWeight, mdfString, mxoPar, NULL},
    {"TARGETXFACTORCAP", pTXFCap, mdfInt, mxoPar, NULL},
    {"TARGETXFACTORWEIGHT", pTXFWeight, mdfInt, mxoPar, NULL},
    {"TASKDISTRIBUTIONPOLICY", pTaskDistributionPolicy, mdfString, mxoPar,
     NULL},
    {"TOOLSDIR", pSchedToolsDir, mdfString, mxoSched, NULL},
    {"TRAPFUNCTION", pMonitoredFunction, mdfString, mxoSched, NULL},
    {"TRAPJOB", pMonitoredJob, mdfString, mxoSched, NULL},
    {"TRAPNODE", pMonitoredNode, mdfString, mxoSched, NULL},
    {"TRAPRES", pMonitoredRes, mdfString, mxoSched, NULL},
    {"UFSWEIGHT", pOLDUFSWeight, mdfInt, mxoPar, NULL},
    {"UJOBWEIGHT", pRUJobWeight, mdfInt, mxoPar, NULL},
    {"UPROCWEIGHT", pRUProcWeight, mdfInt, mxoPar, NULL},
    {"USAGEEXECUTIONTIMECAP", pUExeTimeCap, mdfInt, mxoPar, NULL},
    {"USAGEEXECUTIONTIMEWEIGHT", pUExeTimeWeight, mdfInt, mxoPar, NULL},
    {"USAGECAP", pUsageCap, mdfInt, mxoPar, NULL},
    {"USAGEWEIGHT", pUsageWeight, mdfString, mxoPar, NULL},
    {"USECPUTIME", pUseCPUTime, mdfString, mxoPar, NULL},
    {"USEJOBREGEX", mcoUseJobRegEx, mdfString, mxoSched, NULL},
    {"USELOCALMACHINEPRIORITY", pUseLocalMachinePriority, mdfString, mxoSched,
     NULL},
    {"USEMACHINESPEED", pUseMachineSpeed, mdfString, mxoSched, NULL},
    {"USEMACHINESPEEDFORFS", pUseMachineSpeedForFS, mdfString, mxoSched, NULL},
    {"USERCAP", pCUCap, mdfInt, mxoPar, NULL},
    {"USERWEIGHT", pCUWeight, mdfInt, mxoPar, NULL},
    {"USESYSLOG", mcoUseSyslog, mdfString, mxoSched, NULL},
    {"USESYSTEMQUEUETIME", pUseSystemQueueTime, mdfString, mxoPar, NULL},
    {"WALLTIMECAP", pRWallTimeCap, mdfInt, mxoPar, NULL},
    {"WALLTIMEWEIGHT", pRWallTimeWeight, mdfInt, mxoPar, NULL},
    {"WCVIOLATIONACTION", mcoWCViolAction, mdfString, mxoSched, NULL},
    {"XFACTORCAP", pSXFCap, mdfInt, mxoPar, NULL},
    {"XFACTORTARGETWEIGHT", pOLDXFWeight, mdfInt, mxoPar, NULL},
    {"XFACTORWEIGHT", pSXFWeight, mdfInt, mxoPar, NULL},
    {"XFMINWCLIMIT", pXFMinWCLimit, mdfString, mxoPar, NULL},
    {NULL, -1, -1, -1, NULL}};

const char *_MSchedMode[] = {NONE,         "NORMAL", "PROFILE", "SIMULATION",
                            "SINGLESTEP", "TEST",   NULL};

char *_MBool[] = {"FALSE", "TRUE", "NOTSET", NULL};

char *_MSON[] = {NONE,  "Body",   "Data",    "Envelope",
                      "Get", "Object", "Request", "Response",
                      "Set", "Where",  NULL};

static char CDList[256] = {
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 62, 00, 00, 00, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 00, 00, 00, 00, 00, 00, 0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 00, 00, 00, 00,
    00, 00, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00,
};

char *_MCredCfgParm[] = {
    NULL,     "ACCOUNTCFG", NULL, "CLASSCFG", NULL,      NULL,     NULL,
    NULL,     "GROUPCFG",   NULL, NULL,       "NODECFG", "PARCFG", NULL,
    "QOSCFG", "QUEUECFG",   NULL, NULL,       NULL,      NULL,     "SCHEDCFG",
    NULL,     NULL,         NULL, "SYSCFG",   "USERCFG", NULL};

const char *_MSockProtocol[] = {
    NONE, "SUTCP", "SSS-HALF", "HTTPCLIENT", "HTTP", "SSS-CHALLENGE", NULL};

char *_MCDisplayType[] = {"NONE", "NODECENTRIC", NULL};

const char *_MSchedAttr[] = {NONE,   "CPVERSION", "FBSERVER", "HOMEDIR",
                            "MODE", "NAME",      "SERVER",   NULL};

char *_MComp[] = {"NC", "<",  "<=", "==", ">=", ">", "<>",
                       "=",  "!=", "%<", "%!", "%=", NULL};

const char *_MJobCtlCmds[] = {NONE,        "cancel", "checkpoint", "diagnose",
                            "modify",    "query",  "requeue",    "resume",
                            "show",      "start",  "submit",     "suspend",
                            "terminate", NULL};

/* sync w/enum MChecksumAlgoEnum */

const char *_MCSAlgoType[] = {NONE,  "DES",    "HMAC",   "HMAC64",
                             "MD5", "PASSWD", "REMOTE", NULL};

static char CList[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char *_MCKeyword[] = {NONE,  "SC=",     "ARG=", "AUTH=", "CMD=",
                           "DT=", "CLIENT=", "TS=",  "CK=",   NULL};

/* global buffer for string manipulations.
   has to be kept reusable at all time. */
char temp_str[MMAX_LINE];

m64_t M64;
char *MParam[MAX_MCFG];

time_t *STime;

int *_MGUSyslogActive = NULL; /* boolean */

mccfg_t C;

mfcache_t MFileCache[MAX_FILECACHE];

mjob_t *MJobTraceBuffer;

mckpt_t MCP;
/* globals */

char *AList[32];

long BeginTime = 0;
long EndTime = MAX_MTIME;
long Duration = 0;

int NodeCount = 0;
int ProcCount = 0;
int TaskCount = 0;

int MIndex;
int Memory;
int DMemory;

int UseXML = FALSE;

int Mode = 0;

int HType;
int ObjectType;

char ObjectID[MAX_MNAME];

char ShowUserName[MAX_MNAME];

char UserList[MAX_MLINE];
char GroupList[MAX_MLINE];
char AccountList[MAX_MLINE];
char QOSList[MAX_MLINE];
char ClassList[MAX_MLINE];

char QLine[MAX_MLINE];
char QOSName[MAX_MNAME];

char ResourceList[MAX_MLINE];
char FeatureString[MAX_MLINE];
char JobFeatureString[MAX_MLINE];
char NodeSetString[MAX_MLINE];
char ClassString[MAX_MLINE];

int RType;

char NodeRegEx[MAX_MLINE << 2];
char JobRegEx[MAX_MLINE << 2];
char ResList[MAX_MLINE << 2];
char *RegEx;

int PType;
int BFMode;
int DiagMode;
int MGridMode;
int JobCtlMode;
int BalMode;
int SchedMode;
int PrioMode;
char ChargeAccount[MAX_MNAME];
char SchedArg[MAX_MLINE];
char DiagOpt[MAX_MNAME];

long ClientTime;

char ParName[MAX_MNAME];
int QueueMode;
int Flags;
char UserName[MAX_MNAME];
char Group[MAX_MNAME];
char Account[MAX_MNAME];
char Type[MAX_MLINE];
char RangeList[MAX_MLINE];
char ResDesc[MAX_MLINE];
char ResFlags[MAX_MLINE];

msocket_t Msg;
char MsgBuffer[MAX_MLINE << 3];

msched_t MSched;

/** Duplicate a string
 *
 * This function will take the string passed as argument
 * create a copy of it in newly allocated memory. The new
 * memory is allocated with malloc() to a suitable size.
 * The function returns a pointer to that copy which can
 * be passed to free() in order to return the allocated
 * memory to the system.
 *
 * @param input string to be copied
 * @return pointer to the new storage with the copy of
 * the input string or NULL if the call to malloc failed.
 */

char *string_dup(const char *input){

    char *output;
    int len;

    len = strlen(input) + 1;
    output = (char *) malloc(len);
    if (output) strcpy(output,input);
    return output;
}

/** Convert string to integer
 *
 * This function will take the string passed as argument,
 * check if it is a valid number and then convert it to
 * and integer. If it is not a number the value INVALID_STRING
 * is returned.
 *
 * @param input string to be converted
 * @return value of string converted to integer or INVALID_STRING.
 */

int string2int(const char *input){

    int i,n;

    if (input == NULL) return INVALID_STRING;

    n = strlen(input);
    if (n == 0) return INVALID_STRING;

    for (i = 0; i < n; i++) {
        if (isdigit(input[i]) || input[i] == '-' || input[i] == '+')
            continue;
        return INVALID_STRING;
    }

    return atoi(input);
}

/** Print common help message for flags related to client/server
 * communication between the command line tools and the maui server. */

void print_client_usage()
{
    puts("  -C, --configfile=FILENAME      set configfile\n"
         "  -D, --loglevel=LOGLEVEL        set loglevel\n"
         "  -F, --logfacility=LOGFACILITY  set logfacility\n"
         "  -H, --host=SERVERHOSTNAME      set serverhost\n"
         "  -k, --keyfile=FILENAME         set server keyfile\n"
         "  -P, --port=SERVERPORT          set serverport\n");
}

/**	Send formatted output to a stream
 *
 *  This function will take the string, FILE pointer and list of
 *  arguments passed as arguments and send formatted output to a stream
 *  which is declared by the FILE pointer.
 *
 *  @param1 formatted input string
 *  @param2 input stream to be sent
 *  @return SUCCESS
 */

int dPrint(

    char *Format, /* I */
    ...)

{
    va_list Args;

    if (mlog.logfp != NULL) {
        if (mlog.logfp != stderr) {
            fprintf(mlog.logfp, "%s", _MLogGetTime());
        }

        va_start(Args, Format);

        vfprintf(mlog.logfp, Format, Args);

        va_end(Args);
    } /* END if (mlog.logfp != NULL) */

    return (SUCCESS);
} /* END dPrint() */

char *_MLogGetTime()

{
    time_t epoch_time = 0;
    struct tm *present_time;
    static char line[MAX_MLINE];
    static time_t now = 0;

    getTime(&epoch_time, mtmNONE, NULL);

    if (epoch_time != now) {
        now = epoch_time;

        if ((present_time = localtime(&epoch_time)) != NULL) {
            sprintf(line, "%2.2d/%2.2d %2.2d:%2.2d:%2.2d ",
                    present_time->tm_mon + 1, present_time->tm_mday,
                    present_time->tm_hour, present_time->tm_min,
                    present_time->tm_sec);
        } else {
            sprintf(line, "%2.2d/%2.2d %2.2d:%2.2d:%2.2d ", 0, 0, 0, 0, 0);
        }
    } /* END if (epoch_time != now) */

    return (line);
} /* END MLogGetTime() */

/** Get and save current time or update time
 *
 * 	this function will take the object time_t, time mode integer
 * 	and object msched_t passed as arguments, check if the scheduler
 * 	is in simulation mode and then get or update the time. If it is
 * 	not in simulation mode, the current time will be saved.
 *
 * 	@param1 object time_t to be saved
 * 	@param2 time mode to be used in simulation mode
 * 	@param3 object msched_t to be checked if in simulation mode
 * 	@return SUCCESS or FAILURE
 */

int getTime(time_t *Time, enum MTimeModeEnum RefreshMode, msched_t *S)

{
	if (Time == NULL) {
		return (FAILURE);
	}

	if (((S != NULL) && (S->TimePolicy != mtpNONE))
			|| (RefreshMode == mtmRefresh)) {
		*Time = MIN(*Time, MAX_MTIME);
	}

	/* simulation mode */
	if (S != NULL) {

		if ((S->Mode == msmSim) && (S->TimePolicy != mtpReal)) {
			switch (RefreshMode) {

			case mtmRefresh:

				/* refresh */

				*Time += S->RMPollInterval;

				break;

			case mtmInit:

				/* load real time */

				time(Time);

				break;

			default:

				/* no action necessary */

				break;
			} /* END switch(RefreshMode) */

			return (SUCCESS);
		} /* END if ((S->Mode == msmSim) && ...) */
	} /* END if (S != NULL) */

	/* load real time */

	time(Time);

	return (SUCCESS);
} /* END MUGetTime() */

int _MCInitialize()

{
    char *ptr;
    char *buf;

    int count;
    int SC;

    char tmpLine[MAX_MLINE];

    const char *FName = "__MCInitialize";

    DBG(2, fALL) dPrint("%s()\n", FName);

    _M64Init(&M64);

    _MUBuildPList(mCfg, MParam);

    strcpy(C.ServerHost, DEFAULT_MSERVERHOST);
    C.ServerPort = DEFAULT_MSERVERPORT;

    strcpy(C.HomeDir, MBUILD_HOMEDIR);
    strcpy(C.ConfigFile, DEFAULT_SchedCONFIGFILE);

    MSched.UID = _MOSGetEUID();

#if defined(MBUILD_DIR)
    strncpy(C.BuildDir, MBUILD_DIR, sizeof(C.BuildDir));
#else
    strcpy(C.BuildDir, "NA");
#endif /* MBUILD_DIR */

#if defined(MBUILD_HOST)
    strncpy(C.BuildHost, MBUILD_HOST, sizeof(C.BuildHost));
#else
    strcpy(C.BuildHost, "NA");
#endif /* MBUILD_HOST */

#if defined(MBUILD_DATE)
    strncpy(C.BuildDate, MBUILD_DATE, sizeof(C.BuildDate));
#else
    strcpy(C.BuildDate, "NA");
#endif /* MBUILD_DATE */

    /* setup defaults */

    HType = -1;

    ObjectType = -1;
    RType = -1;
    PType = -1;

    BFMode = 0;
    DiagMode = -1;
    MGridMode = 0;
    JobCtlMode = 0;
    BalMode = 0;
    SchedMode = -1;
    PrioMode = -1;

    strcpy(ChargeAccount, NONE);

    strcpy(DiagOpt, NONE);

    strcpy(SchedArg, "-1");

    QueueMode = -1;

    Flags = 0;

    Memory = 0;
    DMemory = 0;

    MIndex = 0;

    strcpy(UserName, _MUUIDToName(_MOSGetUID()));
    strcpy(Group, _MUGIDToName(getgid()));
    strcpy(Account, "ALL");

    strcpy(UserList, NONE);
    strcpy(GroupList, NONE);
    strcpy(AccountList, NONE);
    strcpy(ClassList, NONE);
    strcpy(QOSList, NONE);
    strcpy(ObjectID, NONE);

    ShowUserName[0] = '\0';

    strcpy(ResourceList, NONE);
    strcpy(ClassString, NONE);
    strcpy(FeatureString, NONE);
    strcpy(JobFeatureString, NONE);
    strcpy(QOSName, NONE);
    strcpy(NodeSetString, NONE);
    strcpy(ResFlags, NONE);

    strcpy(Type, "DEFAULT");
    sprintf(RangeList, "%d,%d", 0, MAX_MTIME);

    strcpy(ResDesc, "DEFAULT");

    memset(&Msg, 0, sizeof(Msg));

    NodeRegEx[0] = '\0';
    strcpy(ResList, NONE);

    /* load environment required for configfile 'bootstrap' */

    if ((ptr = getenv(MSCHED_ENVHOMEVAR)) != NULL) {
        DBG(4, fCONFIG)
        dPrint("INFO:     using %s environment variable (value: %s)\n",
               MSCHED_ENVHOMEVAR, ptr);

        _MUStrCpy(C.HomeDir, ptr, sizeof(C.HomeDir));
    } else if ((ptr = getenv(MParam[pMServerHomeDir])) != NULL) {
        DBG(4, fCONFIG)
        dPrint("INFO:     using %s environment variable (value: %s)\n",
               MParam[pMServerHomeDir], ptr);

        _MUStrCpy(C.HomeDir, ptr, sizeof(C.HomeDir));
    } else {
        /* check master configfile */

        if ((buf = _MFULoad(MASTER_CONFIGFILE, 1, macmRead, &count, &SC)) !=
            NULL) {
            if (((ptr = strstr(buf, MParam[pMServerHomeDir])) != NULL) ||
                ((ptr = strstr(buf, "MAUIHOMEDIR")) != NULL)) {
                _MUSScanF(ptr, "%x%s %x%s", sizeof(tmpLine), tmpLine,
                         sizeof(C.HomeDir), C.HomeDir);
            }
        }
    } /* END else (ptr = getenv()) */

    if ((ptr = getenv(MParam[pSchedConfigFile])) != NULL) {
        DBG(4, fCONFIG)
        dPrint("INFO:     using %s environment variable (value: %s)\n",
               MParam[pSchedConfigFile], ptr);

        _MUStrCpy(C.ConfigFile, ptr, sizeof(C.ConfigFile));
    }

    {
        int UseAuthFile;

        sprintf(MSched.KeyFile, "%s/%s", C.HomeDir, MSCHED_KEYFILE);

        _MUCheckAuthFile(&MSched, MSched.DefaultCSKey, &UseAuthFile, FALSE);

        if (UseAuthFile == TRUE) MSched.DefaultCSAlgo = mcsaRemote;
    } /* END BLOCK */

    return (SUCCESS);
} /* END __MCInitialize() */

uid_t _MOSGetUID()

{
    return (getuid());
} /* END MOSGetUID() */

char *_MUUIDToName(

    uid_t UID) /* I */

{
    struct passwd *bufptr;
    static char Line[MAX_MNAME];

    const char *FName = "_MUUIDToName";

    DBG(10, fSTRUCT) dPrint("%s(%d)\n", FName, UID);

    if (UID == ~0U) {
        strcpy(Line, NONE);

        return (Line);
    }

    if ((bufptr = getpwuid(UID)) == NULL) {
        sprintf(Line, "UID%d", UID);
    } else {
        strcpy(Line, bufptr->pw_name);
    }

    return (Line);
} /* END _MUUIDToName() */


int _M64Init(

    m64_t *M) /* I (modified) */

{
    if (sizeof(int) == 4) {
        /* 32 bit operation */

        M->Is64 = FALSE;

        M->INTBITS = M32INTBITS;
        M->INTLBITS = M32INTLBITS;
        M->INTSIZE = M32INTSIZE;
        M->INTSHIFT = M32INTSHIFT;
    } else {
        /* 64 bit operation */

        M->Is64 = TRUE;

        M->INTBITS = M64INTBITS;
        M->INTLBITS = M64INTLBITS;
        M->INTSIZE = M64INTSIZE;
        M->INTSHIFT = M64INTSHIFT;
    }

    MDB(5, fSTRUCT)
    MLog("INFO:     64Bit enabled: %s  UINT4[%d]  UINT8[%d]\n", _MBool[M->Is64],
         sizeof(MUINT4), sizeof(MUINT8));

    return (SUCCESS);
} /* END M64Init() */

int _MUBuildPList(

    mcfg_t *C, char **PList)

{
    int cindex;

    if ((C == NULL) || (PList == NULL)) {
        return (FAILURE);
    }

    for (cindex = 0; C[cindex].Name != NULL; cindex++) {
        PList[C[cindex].PIndex] = C[cindex].Name;
    }

    return (SUCCESS);
} /* END MUBuildPList() */

uid_t _MOSGetEUID()

{
    return (geteuid());
} /* END MOSGetEUID() */

char *_MUGIDToName(

    gid_t GID) /* I */

{
    struct group *bufptr;

    static char Line[MAX_MNAME];

    const char *FName = "MUGIDToName";

    DBG(10, fSTRUCT) dPrint("%s(%d)\n", FName, GID);

    if (GID == ~0U) {
        strcpy(Line, NONE);

        return (Line);
    }

    if ((bufptr = getgrgid(GID)) == NULL) {
        sprintf(Line, "GID%d", GID);
    } else {
        strcpy(Line, bufptr->gr_name);
    }

    return (Line);
} /* END MUGIDToName() */

int _MUStrCpy(

    char *Dst,  /* I */
    char *Src,  /* O */
    int Length) /* I */

{
    int index;

    if ((Dst == NULL) || (Src == NULL) || (Length == 0)) {
        return (FAILURE);
    }

    if (Length == -1) Length = MAX_MNAME;

    for (index = 0; index < Length; index++) {
        if (Src[index] == '\0') break;

        Dst[index] = Src[index];
    } /* END for (index) */

    if (index >= Length)
        Dst[Length - 1] = '\0';
    else
        Dst[index] = '\0';

    return (SUCCESS);
} /* END _MUStrCpy() */

char *_MFULoad(

    char *FileName, int BlockSize, int AccessMode, int *BlockCount, int *SC)

{
    FILE *dfp = NULL;

    char *ptr;
    char *buf;
    int BufSize;

    int count;

    int ReadCount;

    struct stat sbuf;

    const char *FName = "MFULoad";

    DBG(5, fCORE)
    dPrint("%s(%s,%d,%s,BlockCount,SC)\n", FName,
           (FileName != NULL) ? FileName : "NULL", BlockSize,
           (AccessMode == macmRead) ? "READ" : "WRITE");

    if (SC != NULL) *SC = mscNoError;

    /* check if file is cached */

    if ((FileName != NULL) && (FileName[0] != '\0') &&
        (_MFUGetCachedFile(FileName, &buf, &BufSize) == SUCCESS)) {
        if (BlockCount != NULL) *BlockCount = BufSize / BlockSize;

        if (AccessMode == macmRead) {
            /* use cached data */

            return (buf);
        } else {
            /* copy cached data */
            if ((ptr = (char *)calloc(BufSize + 1, 1)) == NULL) {
                _MOSSyslog(LOG_ERR,
                          "cannot calloc buffer for file '%s', errno: %d (%s)",
                          FileName, errno, strerror(errno));

                DBG(2, fCORE)
                dPrint(
                    "ERROR:    cannot calloc file buffer for file '%s', errno: "
                    "%d (%s)\n",
                    FileName, errno, strerror(errno));

                if (SC != NULL) *SC = mscNoEnt;

                return (NULL);
            } else {
                /* fill calloc'd buffer */

                memcpy(ptr, buf, BufSize + 1);

                DBG(9, fCORE)
                dPrint("INFO:     cached file data copied from %p to %p\n", buf,
                       ptr);

                return (ptr);
            }
        } /* else (AccessMode == macmRead) */

        /* always return */
    } /* END if (__MFUGetCachedFile() == SUCCESS) */

    /* initialize block size to legal value if not previously set */

    if (BlockSize < 1) BlockSize = 1;

    if (FileName != NULL) {
        if (stat(FileName, &sbuf) == -1) {
            DBG(2, fCORE)
            dPrint("ERROR:    cannot stat file '%s', errno: %d (%s)\n",
                   FileName, errno, strerror(errno));

            if (SC != NULL) *SC = mscNoEnt;

            return (NULL);
        }

        ReadCount = sbuf.st_size / BlockSize;

        if ((dfp = fopen(FileName, "r")) == NULL) {
            DBG(2, fCORE)
            dPrint("ERROR:    cannot open file '%s', errno: %d (%s)\n",
                   FileName, errno, strerror(errno));

            if (SC != NULL) *SC = mscNoEnt;

            return (NULL);
        }
    } else {
        dfp = stdin;

        ReadCount = MAX_FBUFFER / BlockSize;
    }

    BufSize = ReadCount * BlockSize;

    DBG(5, fCORE)
    dPrint("INFO:     new file '%s' opened with %d bytes (ReadCount = %d)\n",
           FileName, BufSize, ReadCount);
    if ((ptr = (char *)calloc(BufSize + 1, 1)) == NULL) {
        _MOSSyslog(LOG_ERR,
                  "cannot calloc %d byte buffer for file '%s', errno: %d (%s)",
                  BufSize, FileName, errno, strerror(errno));

        DBG(2, fCORE)
        dPrint(
            "ERROR:    cannot calloc file buffer for file '%s', errno: %d "
            "(%s)\n",
            FileName, errno, strerror(errno));

        if (SC != NULL) *SC = mscNoMemory;

        if (dfp != stdin) fclose(dfp);

        return (NULL);
    }

    if ((count = (int)fread(ptr, ReadCount, BlockSize, dfp)) < 0) {
        _MOSSyslog(LOG_ERR, "cannot read file '%s', errno: %d (%s)", FileName,
                  errno, strerror(errno));

        DBG(2, fCORE)
        dPrint("ERROR:    cannot read file '%s', errno: %d (%s)\n", FileName,
               errno, strerror(errno));

        free(ptr);

        if (SC != NULL) *SC = mscNoEnt;

        if (dfp != stdin) fclose(dfp);

        return (NULL);
    }

    if (dfp != stdin) fclose(dfp);

    if (BlockCount != NULL) *BlockCount = count;

    ptr[BufSize] = '\0';

    _MFUCacheFile(FileName, ptr, BufSize);

    if (AccessMode == macmRead) {
        /* use cached data */

        free(ptr);

        if (_MFUGetCachedFile(FileName, &buf, &BufSize) == SUCCESS) {
            if (BlockCount != NULL) *BlockCount = BufSize / BlockSize;

            return (buf);
        } else {
            if (SC != NULL) *SC = mscNoEnt;

            return (NULL);
        }
    } /* END if (AccessMode == macmRead) */

    return (ptr);
} /* END MFULoad() */

int _MOSSyslog(

    int Facility, /* I */
    char *Format, /* I */
    ...)          /* I */

{
    va_list Args;

    if ((_MGUSyslogActive == NULL) || (*_MGUSyslogActive == FALSE)) {
        return (SUCCESS);
    }

    va_start(Args, Format);

    syslog(Facility, Format, Args);

    va_end(Args);

    return (SUCCESS);
} /* END MOSSyslog() */

int _MFUCacheFile(

    char *FileName, /* I */
    char *Buffer,   /* I */
    int BufSize)    /* I */

{
    int index;

    time_t now;

    const char *FName = "__MFUCacheFile";

    DBG(5, fSTRUCT) dPrint("%s(%s,Buffer,%d)\n", FName, FileName, BufSize);

    /* look for existing cache entry */

    for (index = 0; index < MAX_FILECACHE; index++) {
        if (MFileCache[index].FileName[0] == '\0') continue;

        if (strcmp(MFileCache[index].FileName, FileName)) continue;

        if (MFileCache[index].Buffer != NULL) {
            free(MFileCache[index].Buffer);

            MFileCache[index].Buffer = NULL;
        }
        if ((MFileCache[index].Buffer = (char *)calloc(BufSize + 1, 1)) ==
            NULL) {
            _MOSSyslog(
                LOG_ERR,
                "cannot calloc cache buffer for file '%s', errno: %d (%s)",
                FileName, errno, strerror(errno));

            DBG(2, fSTRUCT)
            dPrint(
                "ERROR:    cannot calloc file buffer for file '%s', errno: %d "
                "(%s)\n",
                FileName, errno, strerror(errno));

            return (FAILURE);
        }

        memcpy(MFileCache[index].Buffer, Buffer, BufSize);

        MFileCache[index].Buffer[BufSize] = '\0';

        MFileCache[index].BufSize = BufSize;

        MFileCache[index].ProgTimeStamp = (STime != NULL) ? *STime : 0;

        time(&now);

        MFileCache[index].FileTimeStamp = now;

        DBG(5, fSTRUCT)
        dPrint("INFO:     file '%s' cached in slot %d (%d bytes)\n", FileName,
               index, BufSize);

        return (SUCCESS);
    } /* END for (index) */

    /* create new cache entry */

    for (index = 0; index < MAX_FILECACHE; index++) {
        if (MFileCache[index].FileName[0] == '\0') {
            /* calloc space for buffer */

            if ((MFileCache[index].Buffer = (char *)calloc(BufSize + 1, 1)) ==
                NULL) {
                _MOSSyslog(
                    LOG_ERR,
                    "cannot calloc cache buffer for file '%s', errno: %d (%s)",
                    FileName, errno, strerror(errno));

                DBG(2, fSTRUCT)
                dPrint(
                    "ERROR:    cannot calloc cache buffer for file '%s', "
                    "errno: %d (%s)\n",
                    FileName, errno, strerror(errno));

                return (FAILURE);
            }

            strcpy(MFileCache[index].FileName, FileName);

            memcpy(MFileCache[index].Buffer, Buffer, BufSize);

            MFileCache[index].Buffer[BufSize] = '\0';

            MFileCache[index].BufSize = BufSize;

            MFileCache[index].ProgTimeStamp = (STime != NULL) ? *STime : 0;

            time(&now);

            MFileCache[index].FileTimeStamp = now;

            DBG(5, fSTRUCT)
            dPrint("INFO:     new file '%s' cached in slot %d (%d bytes)\n",
                   FileName, index, BufSize);

            return (SUCCESS);
        }
    } /* END for index */

    DBG(1, fSTRUCT)
    dPrint(
        "ALERT:    file cache overflow while attempting to cache file '%s'\n",
        FileName);

    return (FAILURE);
} /* END __MFUCacheFile() */

int _MFUGetCachedFile(

    char *FileName, /* I */
    char **Buffer,  /* O */
    int *BufSize)   /* I */

{
    int index;
    long mtime;

    const char *FName = "_MFUGetCachedFile";

    DBG(5, fSTRUCT)
    dPrint("%s(%s,Buffer,BufSize)\n", FName,
           (FileName != NULL) ? FileName : "NULL");

    if (Buffer == NULL) {
        return (FAILURE);
    }

    *Buffer = NULL;

    if ((FileName == NULL) || (FileName[0] == '\0')) {
        return (FAILURE);
    }

    for (index = 0; index < MAX_FILECACHE; index++) {
        if (MFileCache[index].FileName[0] == '\0') continue;

        if (strcmp(MFileCache[index].FileName, FileName)) continue;

        /* cache entry located */

        if (MFileCache[index].ProgTimeStamp == ((STime != NULL) ? *STime : 0)) {
            if (MFileCache[index].Buffer == NULL) {
                DBG(5, fSTRUCT)
                dPrint("ALERT:    filecache buffer is empty for file '%s'\n",
                       FileName);

                return (FAILURE);
            }

            *Buffer = MFileCache[index].Buffer;
            *BufSize = MFileCache[index].BufSize;

            DBG(5, fSTRUCT)
            dPrint("INFO:     cached file '%s' located in slot %d (%d Bytes)\n",
                   FileName, index, (int)strlen(*Buffer));

            return (SUCCESS);
        }

        if (_MFUGetInfo(FileName, &mtime, NULL, NULL) == FAILURE) {
            DBG(5, fSTRUCT)
            dPrint("ALERT:    cannot determine modify time for file '%s'\n",
                   FileName);

            return (FAILURE);
        }

        if (mtime > MFileCache[index].FileTimeStamp) {
            DBG(5, fSTRUCT)
            dPrint("INFO:     file '%s' has been modified\n", FileName);

            return (FAILURE);
        }

        /* file cache is current */

        *Buffer = MFileCache[index].Buffer;
        *BufSize = MFileCache[index].BufSize;

        return (SUCCESS);
    } /* END for (index) */

    /* file cache not found */

    DBG(6, fCORE) dPrint("INFO:     file '%s' not cached\n", FileName);

    return (FAILURE);
} /* END __MFUGetCachedFile() */

int _MUSScanF(

    char *StringBuffer, /* I */
    char *Format,       /* I */
    ...)

{
    char *fptr;
    char *sptr;

    char *tail;

    char *tmpS;
    long *tmpL;
    int *tmpI;

    char IFSList[MAX_MNAME];

    long size;
    long length;

    va_list VA;

    int ArgCount;

    ArgCount = 0;

    /* FORMAT:  "%x%s %ld %d" */

    if (StringBuffer == NULL) {
        return (FAILURE);
    }

    if (Format == NULL) {
        return (FAILURE);
    }

    sptr = StringBuffer;

    if (!strncmp(sptr, "IFS-", strlen("IFS-"))) {
        sptr += strlen("IFS-");

        IFSList[0] = *sptr;

        IFSList[1] = '\0';

        sptr++;
    } else {
        strcpy(IFSList, " \t\n");
    }

    va_start(VA, Format);

    size = MAX_MNAME;

    for (fptr = Format; *fptr != '\0'; fptr++) {
        if (*fptr == '%') {
            fptr++;

            /* remove IFS chars */

            while (strchr(IFSList, sptr[0]) && (sptr[0] != '\0')) sptr++;

            switch (*fptr) {
                case 'd':

                    /* read integer */

                    tmpI = va_arg(VA, int *);

                    if (tmpI != NULL) *tmpI = (int)strtol(sptr, &tail, 10);

                    sptr = tail;

                    while (!strchr(IFSList, sptr[0]) && (sptr[0] != '\0'))
                        sptr++;

                    ArgCount++;

                    break;

                case 'l':

                    tmpL = va_arg(VA, long *);

                    if (tmpL != NULL) *tmpL = strtol(sptr, &tail, 10);

                    sptr = tail;

                    while (!strchr(IFSList, sptr[0]) && (sptr[0] != '\0'))
                        sptr++;

                    ArgCount++;

                    break;

                case 's':

                    if (size == 0) {
                        return (FAILURE);
                    }

                    tmpS = va_arg(VA, char *);
                    tmpS[0] = '\0';

                    length = 0;

                    while ((sptr[0] != '\0') && strchr(IFSList, sptr[0]))
                        sptr++;

                    while (length < (size - 1)) {
                        if (*sptr == '\0') break;

                        if (strchr(IFSList, sptr[0])) break;

                        if (tmpS != NULL) tmpS[length] = *sptr;

                        length++;

                        sptr++;
                    }

                    if (tmpS != NULL) tmpS[length] = '\0';

                    if (length > 0) ArgCount++;

                    break;

                case 'x':

                    size = va_arg(VA, int);

                    break;

                default:

                    break;
            } /* END switch(*fptr) */
        }     /* END if (*fptr == '%') */
    }         /* END for (fptr = Format,*fptr != '\0';fptr++) */

    va_end(VA);

    return (ArgCount);
} /* END MUSScanf() */

int _MFUGetInfo(

    char *FileName, long *ModifyTime, long *FileSize, int *IsExe)

{
    struct stat sbuf;

    if (stat(FileName, &sbuf) == -1) {
        DBG(2, fCORE)
        dPrint("INFO:     cannot stat file '%s', errno: %d (%s)\n", FileName,
               errno, strerror(errno));

        return (FAILURE);
    }

    if (ModifyTime != NULL) *ModifyTime = (long)sbuf.st_mtime;

    if (FileSize != NULL) *FileSize = (long)sbuf.st_size;

    if (IsExe != NULL) {
        if (sbuf.st_mode & S_IXUSR)
            *IsExe = TRUE;
        else
            *IsExe = FALSE;
    }

    return (SUCCESS);
} /* END MFUGetInfo() */

int _MUCheckAuthFile(

    msched_t *S,     /* I (optional) */
    char *KeyBuf,    /* O (optional) */
    int *UseKeyFile, /* O (optional) */
    int IsServer)    /* I (boolean) */

{
    char tmpFileName[MAX_MLINE];

    int AuthFileOK = FALSE;
    int IsPrivate;

    uid_t UID;

    char *ptr;

    /* determine file name */

    if ((ptr = getenv("MAUTH_FILE")) != NULL) {
        _MUStrCpy(tmpFileName, ptr, sizeof(tmpFileName));
    } else if (S != NULL) {
        if (S->KeyFile[0] != '\0') {
            _MUStrCpy(tmpFileName, S->KeyFile, sizeof(tmpFileName));
        } else {
            sprintf(tmpFileName, "%s/%s", S->HomeDir, MSCHED_KEYFILE);
        }
    } else {
        strcpy(tmpFileName, MSCHED_KEYFILE);
    }

    /* check existence */

    if (_MFUGetAttributes(tmpFileName, NULL, NULL, NULL, &UID, &IsPrivate,
                         NULL) == SUCCESS) {
        if (IsPrivate == TRUE) {
            if (IsServer == TRUE) {
                if ((S != NULL) && (UID == S->UID)) {
                    AuthFileOK = TRUE;
                }
            } else {
                AuthFileOK = TRUE;
            }
        }
    }

    if (AuthFileOK == FALSE) {
        if (UseKeyFile != NULL) *UseKeyFile = FALSE;

        if (KeyBuf != NULL) strncpy(KeyBuf, MBUILD_SKEY, MAX_MNAME);

        return (SUCCESS);
    }

    if (UseKeyFile != NULL) *UseKeyFile = TRUE;

    if ((IsServer == TRUE) && (KeyBuf != NULL)) {
        char *ptr;

        int i;

        /* load key file */

        if ((ptr = _MFULoad(tmpFileName, 1, macmRead, NULL, NULL)) == NULL) {
            /* cannot load data */

            return (FAILURE);
        }

        _MUStrCpy(KeyBuf, ptr, MAX_MNAME);

        for (i = strlen(KeyBuf) - 1; i > 0; i--) {
            if (!isspace(KeyBuf[i])) break;

            KeyBuf[i] = '\0';
        } /* END for (i) */
    }     /* END if ((IsServer == TRUE) && (KeyBuf != NULL)) */

    return (SUCCESS);
} /* END MUCheckAuthFile() */

int _MFUGetAttributes(

    char *PathName, /* I */
    int *Perm,      /* O */
    time_t *MTime,  /* O */
    long *Size,     /* O */
    uid_t *UID,     /* O */
    int *IsPrivate, /* O */
    int *IsExe)     /* O */

{
    struct stat S;

    if (PathName == NULL) {
        return (FAILURE);
    }

    if (stat(PathName, &S) == -1) {
        return (FAILURE);
    }

    if (MTime != NULL) *MTime = S.st_mtime;

    if (Size != NULL) *Size = S.st_size;

    if (Perm != NULL) *Perm = (int)S.st_mode;

    if (UID != NULL) *UID = S.st_uid;

    if (IsPrivate != NULL) {
        if (S.st_mode & (S_IRWXG | S_IRWXO))
            *IsPrivate = FALSE;
        else
            *IsPrivate = TRUE;
    }

    if (IsExe != NULL) {
        if (S.st_mode & S_IXUSR)
            *IsExe = TRUE;
        else
            *IsExe = FALSE;
    }

    return (SUCCESS);
} /* END MFUGetAttributes() */

int _MCLoadConfig(

    char *Directory,  /* I */
    char *ConfigFile) /* I */

{
    char *buf;
    char *ptr;

    char Name[MAX_MLINE + 1];
    char tmpLine[MAX_MLINE];

    int index;

    int count;

    int SC;

    const char *FName = "__MCLoadConfig";

    DBG(2, fUI) dPrint("%s(%s,%s)\n", FName, Directory, ConfigFile);

    if ((ConfigFile[0] == '/') || (ConfigFile[0] == '~')) {
        strcpy(Name, ConfigFile);
    } else {
        if (Directory[strlen(Directory) - 1] == '/')
            sprintf(Name, "%s%s", Directory, ConfigFile);
        else
            sprintf(Name, "%s/%s", Directory, ConfigFile);
    }

    if ((buf = _MFULoad(Name, 1, macmRead, &count, &SC)) == NULL) {
        DBG(0, fCONFIG) dPrint("WARNING:  cannot open configfile '%s'\n", Name);

        DBG(0, fCONFIG) dPrint("INFO:     using internal defaults\n");

        return (FAILURE);
    }

    _MCfgAdjustBuffer(&buf, TRUE);

    MSched.ConfigBuffer = buf;

    _MCfgGetSVal(MSched.ConfigBuffer, NULL, MParam[pServerHost], NULL, NULL,
                C.ServerHost, sizeof(C.ServerHost), 1, NULL);
    _MCfgGetIVal(MSched.ConfigBuffer, NULL, MParam[pServerPort], NULL, NULL,
                &C.ServerPort, NULL);
    _MCfgGetSVal(MSched.ConfigBuffer, NULL, MParam[pSchedMode], NULL, NULL,
                C.SchedulerMode, sizeof(C.SchedulerMode), 1, NULL);

    if (_MCfgGetSVal(MSched.ConfigBuffer, NULL, MParam[pMCSocketProtocol], NULL,
                    NULL, tmpLine, sizeof(tmpLine), 1, NULL) == SUCCESS) {
        C.SocketProtocol =
            _MUGetIndex(tmpLine, _MSockProtocol, 0, mspSingleUseTCP);
    }

    /* get admin4 user */

    if (_MCfgGetSVal(MSched.ConfigBuffer, NULL, MParam[mcoAdmin4Users], NULL,
                    NULL, tmpLine, sizeof(tmpLine), 1, NULL) == SUCCESS) {
        _MUStrCpy(MSched.Admin4User[0], tmpLine, MAX_MNAME);
    }

    /* get timeout */

    if (_MCfgGetSVal(MSched.ConfigBuffer, NULL, MParam[pClientTimeout], NULL,
                    NULL, tmpLine, sizeof(tmpLine), 1, NULL) == SUCCESS) {
        C.Timeout = _MUTimeFromString(tmpLine) * 1000000;

        if (C.Timeout <= 0) C.Timeout = DEFAULT_CLIENTTIMEOUT;
    } else {
        C.Timeout = DEFAULT_CLIENTTIMEOUT;
    }

    if ((ptr = getenv(MParam[pClientTimeout])) != NULL) {
        C.Timeout = _MUTimeFromString(ptr) * 1000000;

        if (C.Timeout <= 0) {
            C.Timeout = DEFAULT_CLIENTTIMEOUT;
        }
    }

    _MCfgGetSVal(MSched.ConfigBuffer, NULL, MParam[pDisplayFlags], NULL, NULL,
                tmpLine, sizeof(tmpLine), 0, NULL);

    C.DisplayFlags = 0;

    if (tmpLine[0] != '\0') {
        for (index = 0; _MCDisplayType[index] != NULL; index++) {
            if (strstr(tmpLine, _MCDisplayType[index]) != NULL) {
                C.DisplayFlags |= (1 << index);
            }
        } /* END for (index) */

        DBG(2, fALL)
        dPrint("INFO:     %s set to %x\n", MParam[pDisplayFlags],
               (int)C.DisplayFlags);
    } /* END if (tmpLine[0] != '\0') */

    if (_MSchedLoadConfig(NULL) == SUCCESS) {
        if (MSched.ServerPort > 0) C.ServerPort = MSched.ServerPort;

        if (MSched.ServerHost[0] != '\0')
            strcpy(C.ServerHost, MSched.ServerHost);

        if (MSched.Mode != 0) strcpy(C.SchedulerMode, _MSchedMode[MSched.Mode]);
    } /* END if (MSchedLoadConfig() == SUCCESS) */

    return (SUCCESS);
} /* END __MCLoadConfig() */

int _MCfgAdjustBuffer(

    char **Buf,             /* I (modified) */
    mbool_t AllowExtension) /* I */

{
    char *ptr;

    int State;

    char IFile[MAX_MLINE];

    /* change all parameters to upper case */
    /* replace all comments with spaces    */
    /* replace tabs with space             */
    /* replace '"' with space              */
    /* replace unprint chars with space    */
    /* extend '\' lines                    */

    enum { cbPreParm = 0, cbOnParm, cbPreVal, cbOnVal, cbComment };

    const char *FName = "_MCfgAdjustBuffer";

    DBG(3, fCONFIG) dPrint("%s(Buf)\n", FName);

    if ((Buf == NULL) || (*Buf == NULL) || (strlen(*Buf) < 1)) {
        return (SUCCESS);
    }

    ptr = *Buf;

    State = cbPreParm;

    IFile[0] = '\0';

    while (*ptr != '\0') {
        /* remove comments */

        if (*ptr == '#') {
            if (AllowExtension == TRUE) {
                /* look for include */

                if (!strncmp(ptr, "#INCLUDE", strlen("#INCLUDE"))) {
                    /* FORMAT:  #INCLUDE <FILENAME> */

                    _MUSScanF(ptr + strlen("#INCLUDE"), "%x%s", MMAX_LINE,
                             IFile);
                }
            }

            State = cbComment;
        } /* END if (*ptr == '#') */
        else if ((*ptr == '\\') && (State != cbComment)) {
            *ptr = ' ';

            while (isspace(*ptr)) {
                *ptr = ' ';

                ptr++;
            }
        } else if (*ptr == '\n') {
            if ((State == cbComment) && (IFile[0] != '\0')) {
                char *IBuf;

                int blen;

                int offset;

                /* include file at end */

                /* load file */

                if ((IBuf = _MFULoad(IFile, 1, macmWrite, NULL, NULL)) == NULL) {
                    /* cannot load include file */
                } else {
                    blen = strlen(*Buf);

                    offset = ptr - *Buf;

                    if ((*Buf = (char *)realloc(
                             *Buf, blen + 2 + strlen(IBuf))) == NULL) {
                        /* NOTE:  memory failure */

                        _MUFree(&IBuf);

                        return (FAILURE);
                    }

                    /* append file */

                    strcat(*Buf, "\n");
                    strcat(*Buf, IBuf);

                    _MUFree(&IBuf);

                    ptr = *Buf + offset;
                }

                IFile[0] = '\0';
            } /* END if ((State == cbComment) && (IFile[0] != '\0')) */

            State = cbPreParm;
        } else if ((State == cbComment) || (*ptr == '\\')) {
            *ptr = ' ';
        } else if (isspace(*ptr) || (*ptr == '=')) {
            if (*ptr == '=') {
                if (State != cbOnVal) *ptr = ' ';
            } else {
                if (State == cbOnParm) State = cbPreVal;

                *ptr = ' ';
            }
        } else if (isprint(*ptr)) {
            if (State == cbPreParm) {
                State = cbOnParm;

                /*
                if (isalpha(*ptr))
                  *ptr = toupper(*ptr);
                */
            } else if (State == cbPreVal) {
                State = cbOnVal;
            }
        } else {
            *ptr = ' ';
        }

        ptr++;
    } /* END while (ptr != '\0') */

    DBG(5, fCONFIG)
    dPrint("INFO:     adjusted config Buffer ------\n%s\n------\n", *Buf);

    return (SUCCESS);
} /* END _MCfgAdjustBuffer() */

int _MCfgGetSVal(

    char *Buf, char **CurPtr, const char *Parm, char *IndexName, int *Index,
    char *Value, int ValSize, int Mode, char **SymTable)

{
    char *ptr;

    int rc;
    int index;

    const char *FName = "_MCfgGetSVal";

    DBG(7, fCONFIG)
    dPrint("%s(Buf,CurPtr,%s,%s,Index,Value,SymTable)\n", FName, Parm,
           (IndexName != NULL) ? IndexName : "NULL");

    if (Value != NULL) Value[0] = '\0';

    if (Parm == NULL) {
        return (FAILURE);
    }

    ptr = Buf;

    if (CurPtr != NULL) ptr = MAX(ptr, *CurPtr);

    rc = _MCfgGetVal(&ptr, Parm, IndexName, Index, Value, ValSize, SymTable);

    if (CurPtr != NULL) *CurPtr = ptr;

    if (rc == FAILURE) {
        return (FAILURE);
    }

    if (Mode == 1) {
        /* process only first white space delimited string */

        for (index = 0; Value[index] != '\0'; index++) {
            if (isspace(Value[index])) {
                Value[index] = '\0';

                break;
            }
        }
    } else {
        /* remove trailing whitespace */

        for (index = strlen(Value) - 1; index > 0; index--) {
            if (isspace(Value[index]))
                Value[index] = '\0';
            else
                break;
        } /* END for (index) */
    }

    DBG(4, fCONFIG)
    dPrint("INFO:     %s[%d] set to %s\n", Parm, (Index != NULL) ? *Index : 0,
           Value);

    return (SUCCESS);
} /* END _MCfgGetSVal() */

int _MCfgGetIVal(

    char *Buf, char **CurPtr, const char *Parm, char *IndexName, int *Index,
    int *Value, char **SymTable)

{
    char ValLine[MAX_MLINE];
    char *ptr;

    int rc;

    const char *FName = "_MCfgGetIVal";

    DBG(7, fCONFIG)
    dPrint("%s(Buf,CurPtr,%s,%s,Index,Value,SymTable)\n", FName, Parm,
           (IndexName != NULL) ? IndexName : "NULL");

    ptr = Buf;

    if (CurPtr != NULL) ptr = MAX(ptr, *CurPtr);

    rc = _MCfgGetVal(&ptr, Parm, IndexName, Index, ValLine, sizeof(ValLine),
                    SymTable);

    if (CurPtr != NULL) *CurPtr = ptr;

    if (rc == FAILURE) return (FAILURE);

    *Value = (int)strtol(ValLine, NULL, 0);

    DBG(4, fCONFIG)
    dPrint("INFO:     %s[%d] set to %d\n", Parm, (Index != NULL) ? *Index : 0,
           *Value);

    return (SUCCESS);
} /* END _MCfgGetIVal() */

int _MUGetIndex(

    char *Value,          /* I */
    const char **ValList, /* I */
    int AllowSubstring,   /* I (boolean) */
    int DefaultValue)     /* I */

{
    const char *FName = "_MUGetIndex";

    int index;

    DBG(3, fSTRUCT)
    dPrint("%s(%s,%s,%d)\n", FName, (Value != NULL) ? Value : "NULL",
           (ValList != NULL) ? "ValList" : "NULL", DefaultValue);

    if (ValList == NULL) {
        return (DefaultValue);
    }

    if (Value == NULL) {
        return (DefaultValue);
    }

    for (index = 0; ValList[index] != NULL; index++) {
        if ((AllowSubstring == FALSE) && (!strcmp(Value, ValList[index]))) {
            return (index);
        } else if ((AllowSubstring == TRUE) &&
                   (!strncmp(Value, ValList[index], strlen(ValList[index])))) {
            return (index);
        } else if (AllowSubstring == MBNOTSET) {
            int len = strlen(ValList[index]);

            if (!strncmp(Value, ValList[index], len) &&
                (strchr(" \t\n=<>,:;|", Value[len]) || (Value[len] == '\0'))) {
                return (index);
            }
        }
    } /* END for (index) */

    return (DefaultValue);
} /* END _MUGetIndex() */

long _MUTimeFromString(

    char *TString)

{
    long val;

    char *ptr1;
    char *ptr2;
    char *ptr3;
    char *ptr4;

    char *TokPtr;

    char Line[MAX_MLINE];

    const char *FName = "_MUTimeFromString";

    DBG(2, fCONFIG)
    dPrint("%s(%s)\n", FName, (TString != NULL) ? TString : "NULL");

    if (TString == NULL) return (0);

    if (!strcmp(TString, "INFINITY")) return (MAX_MTIME);

    if (strchr(TString, ':') == NULL) {
        /* line specified as 'raw' seconds */

        val = strtol(TString, NULL, 0);

        DBG(4, fCONFIG)
        dPrint("INFO:     string '%s' specified as seconds\n", TString);

        return (val);
    } else if (strchr(TString, '_') != NULL) {
        /* line specified as 'absolute' time */

        _MUStringToE(TString, &val);

        DBG(4, fCONFIG)
        dPrint("INFO:     string '%s' specified as absolute time\n", TString);

        return (val);
    }

    /* line specified in 'military' time */

    _MUStrCpy(Line, TString, sizeof(Line));

    ptr1 = NULL;
    ptr2 = NULL;
    ptr3 = NULL;
    ptr4 = NULL;

    if ((ptr1 = _MUStrTok(Line, ":", &TokPtr)) != NULL) {
        if ((ptr2 = _MUStrTok(NULL, ":", &TokPtr)) != NULL) {
            if ((ptr3 = _MUStrTok(NULL, ":", &TokPtr)) != NULL) {
                ptr4 = _MUStrTok(NULL, ":", &TokPtr);
            }
        }
    }

    if (ptr1 == NULL) {
        DBG(4, fCONFIG) dPrint("INFO:     cannot read string '%s'\n", TString);

        return (0);
    }

    if (ptr4 == NULL) {
        /* adjust from HH:MM:SS to DD:HH:MM:SS notation */

        ptr4 = ptr3;
        ptr3 = ptr2;
        ptr2 = ptr1;
        ptr1 = NULL;
    }

    val = (((ptr1 != NULL) ? atoi(ptr1) : 0) * 86400) +
          (((ptr2 != NULL) ? atoi(ptr2) : 0) * 3600) +
          (((ptr3 != NULL) ? atoi(ptr3) : 0) * 60) +
          (((ptr4 != NULL) ? atoi(ptr4) : 0) * 1);

    DBG(4, fCONFIG) dPrint("INFO:     string '%s' -> %ld\n", TString, val);

    return (val);
} /* END _MUTimeFromString() */

int _MSchedLoadConfig(

    char *Buf) /* I (optional) */

{
    char IndexName[MAX_MNAME];

    char Value[MAX_MLINE];

    char *ptr;
    char *head;

    /* FORMAT:  <KEY>=<VAL>[<WS><KEY>=<VAL>]...         */
    /*          <VAL> -> <ATTR>=<VAL>[:<ATTR>=<VAL>]... */

    /* load all/specified AM config info */

    head = (Buf != NULL) ? Buf : MSched.ConfigBuffer;

    if (head == NULL) {
        return (FAILURE);
    }

    /* load all sched config info */

    ptr = head;

    IndexName[0] = '\0';

    while (_MCfgGetSVal(head, &ptr, _MCredCfgParm[mxoSched], IndexName, NULL,
                       Value, sizeof(Value), 0, NULL) != FAILURE) {
        if (IndexName[0] != '\0') {
            /* set scheduler name */

            _MSchedSetAttr(&MSched, msaName, (void *)IndexName, mdfString, mSet);
        }

        /* load sys specific attributes */

        _MSchedProcessConfig(&MSched, Value);

        IndexName[0] = '\0';
    } /* END while (_MCfgGetSVal() != FAILURE) */

    return (SUCCESS);
} /* END MSchedLoadConfig() */

int _MUFree(

    char **Ptr) /* I */

{
    if ((Ptr == NULL) || (*Ptr == NULL)) {
        return (SUCCESS);
    }

    free(*Ptr);

    *Ptr = NULL;

    return (SUCCESS);
} /* END _MUFree() */

int _MCfgGetVal(

    char **Buf, const char *Parm, char *IName, int *Index, char *Value,
    int ValSize, char **SymTable)

{
    char *ptr;
    char *tmp;
    char *head;
    char *tail;

    char IndexName[MAX_MNAME];

    int iindex;

    const char *FName = "_MCfgGetVal";

    DBG(7, fCONFIG)
    dPrint("%s(Buf,%s,%s,Index,Value,%d,SymTable)\n", FName, Parm,
           (IName != NULL) ? IName : "NULL", ValSize);

    if (Parm == NULL) return (FAILURE);

    IndexName[0] = '\0';

    /* FORMAT:  { '\0' || '\n' }[<WS>]<VAR>[<[><WS><INDEX><WS><]>]<WS><VALUE> */

    ptr = *Buf;

    while (ptr != NULL) {
        if ((head = strstr(ptr, Parm)) == NULL) break;
        ptr = head + strlen(Parm);

        /* look backwards for newline or start of buffer */

        if (head > *Buf)
            tmp = head - 1;
        else
            tmp = *Buf;

        while ((tmp > *Buf) && ((*tmp == ' ') || (*tmp == '\t'))) tmp--;

        if ((tmp != *Buf) && (*tmp != '\n')) continue;

        if ((IName != NULL) && (IName[0] != '\0')) {
            /* requested index name specified */

            if (*ptr != '[') continue;

            ptr++;

            while (isspace(*ptr)) ptr++;

            if (strncmp(IName, ptr, strlen(IName)) != 0) continue;

            ptr += strlen(IName);

            while (isspace(*ptr)) ptr++;

            if (*ptr != ']') continue;

            ptr++;

            /* requested index found */
        } else if (isspace(*ptr)) {
            /* no index specified */

            if (Index != NULL) *Index = 0;
        } else {
            /* index specified, no specific index requested */

            if (*ptr != '[') continue;

            ptr++;

            while (isspace(*ptr)) ptr++;

            head = ptr;

            while ((!isspace(*ptr)) && (*ptr != ']')) ptr++;

            _MUStrCpy(IndexName, head, MIN(ptr - head + 1, MAX_MNAME));

            while (isspace(*ptr)) ptr++;

            if (*ptr != ']') continue;

            ptr++;

            if (Index != NULL) {
                *Index = (int)strtol(IndexName, &tail, 10);

                if (*tail != '\0') {
                    /* index is symbolic */

                    if (SymTable == NULL) return (FAILURE);

                    for (iindex = 0; SymTable[iindex] != NULL; iindex++) {
                        if (!strcmp(SymTable[iindex], IndexName)) {
                            *Index = iindex;
                            break;
                        }
                    }

                    if (SymTable[iindex] == NULL) {
                        _MUStrDup(&SymTable[iindex], IndexName);

                        *Index = iindex;
                    }
                }
            }
        } /* END else ... */

        while ((*ptr == ' ') || (*ptr == '\t')) ptr++;

        if ((tail = strchr(ptr, '\n')) == NULL) tail = *Buf + strlen(*Buf);

        _MUStrCpy(Value, ptr, MIN(tail - ptr + 1, ValSize));

        if ((IName != NULL) && (IName[0] == '\0'))
            _MUStrCpy(IName, IndexName, MAX_MNAME);

        Value[tail - ptr] = '\0';

        *Buf = tail;

        return (SUCCESS);
    } /* END while(ptr != NULL) */

    Value[0] = '\0';

    return (FAILURE);
} /* END _MCfgGetVal() */

int _MUStrDup(

    char **Dst, char *Src)

{
    if (Dst == NULL) {
        return (FAILURE);
    }

    if ((*Dst != NULL) && (Src != NULL) && (Src[0] == (*Dst)[0]) &&
        (!strcmp(Src, *Dst))) {
        /* strings are identical */

        return (SUCCESS);
    }

    _MUFree(Dst);

    if ((Src != NULL) && (Src[0] != '\0')) *Dst = strdup(Src);

    return (SUCCESS);
} /* END _MUStrDup() */

int _MUStringToE(

    char *TimeLine, long *EpochTime)

{
    char Second[MAX_MNAME];
    char Minute[MAX_MNAME];
    char Hour[MAX_MNAME];
    char Day[MAX_MNAME];
    char Month[MAX_MNAME];
    char Year[MAX_MNAME];
    char TZ[MAX_MNAME];

    char StringTime[MAX_MNAME];
    char StringDate[MAX_MNAME];
    char Line[MAX_MLINE];

    char *ptr;
    char *tail;

    struct tm Time;
    struct tm *DefaultTime;

    time_t ETime; /* calculated epoch time */
    time_t Now;

    int YearVal;

    char *TokPtr;

    const char *FName = "_MUStringToE";

    DBG(2, fCONFIG) dPrint("%s(%s,EpochTime)\n", FName, TimeLine);

    time(&Now);

    /* check 'NOW' keyword */

    if (!strcmp(TimeLine, "NOW")) {
        *EpochTime = (long)Now;

        return (SUCCESS);
    }

    /* check 'OFF' keyword */

    if (!strcmp(TimeLine, "OFF")) {
        *EpochTime = MAX_MTIME;

        return (SUCCESS);
    }

    if ((ptr = strchr(TimeLine, '+')) != NULL) {
        /* using relative time */

        /* Format [ +d<DAYS> ][ +h<HOURS> ][ +m<MINUTES> ][ +s<SECONDS> ] */

        ETime = Now + _MUTimeFromString(ptr + 1);
    } else {
        /* using absolute time */

        /* Format:  HH[:MM[:SS]][_MM[/DD[/YY]]] */

        setlocale(LC_TIME, "en_US.iso88591");

        DefaultTime = localtime(&Now);

        /* copy default values into time structure */

        strcpy(Second, "00");
        strcpy(Minute, "00");
        strftime(Hour, MAX_MNAME, "%H", DefaultTime);

        strftime(Day, MAX_MNAME, "%d", DefaultTime);
        strftime(Month, MAX_MNAME, "%m", DefaultTime);

        strftime(Year, MAX_MNAME, "%Y", DefaultTime);

        strftime(TZ, MAX_MNAME, "%Z", DefaultTime);

        if ((tail = strchr(TimeLine, '_')) != NULL) {
            /* time and date specified */

            strncpy(StringTime, TimeLine, (tail - TimeLine));
            StringTime[(tail - TimeLine)] = '\0';

            strcpy(StringDate, (tail + 1));

            DBG(7, fCONFIG)
            dPrint("INFO:     time: '%s'  date: '%s'\n", StringTime,
                   StringDate);

            /* parse date */

            if ((ptr = _MUStrTok(StringDate, "/", &TokPtr)) != NULL) {
                strcpy(Month, ptr);

                if ((ptr = _MUStrTok(NULL, "/", &TokPtr)) != NULL) {
                    strcpy(Day, ptr);

                    if ((ptr = _MUStrTok(NULL, "/", &TokPtr)) != NULL) {
                        YearVal = atoi(ptr);

                        if (YearVal < 97) {
                            sprintf(Year, "%d", YearVal + 2000);
                        } else if (YearVal < 1900) {
                            sprintf(Year, "%d", YearVal + 1900);
                        } else {
                            sprintf(Year, "%d", YearVal);
                        }
                    }
                }
            }
        } else {
            strcpy(StringTime, TimeLine);
        }

        /* parse time */

        if ((ptr = _MUStrTok(StringTime, ":_", &TokPtr)) != NULL) {
            strcpy(Hour, ptr);

            if ((ptr = _MUStrTok(NULL, ":_", &TokPtr)) != NULL) {
                strcpy(Minute, ptr);

                if ((ptr = _MUStrTok(NULL, ":_", &TokPtr)) != NULL)
                    strcpy(Second, ptr);
            }
        }

        /* create time string */

        sprintf(Line, "%s:%s:%s %s/%s/%s %s", Hour, Minute, Second, Month, Day,
                Year, TZ);

        /* perform bounds checking */

        if ((atoi(Second) > 59) || (atoi(Minute) > 59) || (atoi(Hour) > 23) ||
            (atoi(Month) > 12) || (atoi(Day) > 31) || (atoi(Year) > 2097)) {
            DBG(1, fCONFIG)
            dPrint("ERROR:    invalid time specified '%s' (bounds exceeded)\n",
                   Line);

            return (FAILURE);
        }

        memset(&Time, 0, sizeof(Time));

        Time.tm_hour = atoi(Hour);
        Time.tm_min = atoi(Minute);
        Time.tm_sec = atoi(Second);
        Time.tm_mon = atoi(Month) - 1;
        Time.tm_mday = atoi(Day);
        Time.tm_year = atoi(Year) - 1900;

        /* adjust for TZ */

        Time.tm_isdst = -1;

        /* place current time into tm structure */

        DBG(5, fCONFIG) dPrint("INFO:     generated time line: '%s'\n", Line);

        /* strptime(Line,"%T %m/%d/%Y %Z",&Time); */

        if ((ETime = mktime(&Time)) == -1) {
            DBG(5, fCONFIG)
            dPrint(
                "ERROR:    cannot determine epoch time for '%s', errno: %d "
                "(%s)\n",
                Line, errno, strerror(errno));

            return (FAILURE);
        }
    } /* END else (strchr(TimeLine,'+')) */

    DBG(3, fCONFIG)
    dPrint("INFO:     current   epoch:  %lu  time:  %s\n", (unsigned long)Now,
           ctime(&Now));

    DBG(3, fCONFIG)
    dPrint("INFO:     calculated epoch: %lu  time:  %s\n", (unsigned long)ETime,
           ctime(&ETime));

    *EpochTime = (long)ETime;

    return (SUCCESS);
} /* END _MUStringToE() */

char *_MUStrTok(

    char *Line,  /* I (optional) */
    char *DList, /* I */
    char **Ptr)  /* O */

{
    char *Head = NULL;

    int dindex;

    mbool_t ignchar;

    if (Line != NULL) {
        *Ptr = Line;
    } else if ((Ptr != NULL) && (*Ptr == NULL)) {
        return (FAILURE);
    }

    ignchar = FALSE;

    while (**Ptr != '\0') {
        for (dindex = 0; DList[dindex] != '\0'; dindex++) {
            if (**Ptr == DList[dindex]) {
                **Ptr = '\0';

                (*Ptr)++;

                if (Head != NULL) {
                    return (Head);
                } else {
                    ignchar = TRUE;

                    break;
                }
            }
        } /* END for (dindex) */

        if ((ignchar != TRUE) && (**Ptr != '\0')) {
            if (Head == NULL) Head = *Ptr;

            (*Ptr)++;
        }

        ignchar = FALSE;
    } /* END while (**Ptr != '\0') */

    return (Head);
} /* END _MUStrTok() */

int _MSchedSetAttr(

    msched_t *S, /* I (modified) */
    int AIndex,  /* I */
    void **AVal, /* I */
    int Format,  /* I */
    int Mode)    /* I */

{
    if ((S == NULL) || (AVal == NULL)) {
        return (FAILURE);
    }

    switch (AIndex) {
        case msaCPVersion:

        {
            char *ptr;

            if ((ptr = strrchr((char *)AVal, '.')) != NULL) {
                *ptr = '\0';
            }

            strcpy(MCP.DVersion, (char *)AVal);
        }

        break;

        case msaHomeDir:

            _MUStrCpy(S->HomeDir, (char *)AVal, sizeof(S->HomeDir));

            /* append '/' if necessary */

            if (S->HomeDir[strlen(S->HomeDir) - 1] != '/')
                _MUStrCat(S->HomeDir, "/", sizeof(S->HomeDir));

            /* set CWD to home directory */

            if (chdir(S->HomeDir) == -1) {
                perror("cannot change directory");

                DBG(0, fALL)
                dPrint(
                    "ERROR:    cannot change directory to '%s', errno: %d "
                    "(%s)\n",
                    S->HomeDir, errno, strerror(errno));
            }

            break;

        case msaName:

            _MUStrCpy(S->Name, (char *)AVal, sizeof(S->Name));

            break;

        default:

            /* NO-OP */

            break;
    } /* END switch(AIndex) */

    return (SUCCESS);
} /* END MSchedSetAttr() */

int _MUStrCat(

    char *Dst, char *Src, int DstSize)

{
    int index;
    int DEnd;

    if ((Dst == NULL) || (DstSize <= 0)) {
        return (FAILURE);
    }

    if ((Src == NULL) || (Src[0] == '\0')) {
        return (SUCCESS);
    }

    DEnd = MIN((int)strlen(Dst), DstSize);

    for (index = 0; index < DstSize - DEnd; index++) {
        if (Src[index] == '\0') break;

        Dst[DEnd + index] = Src[index];
    } /* END for (index) */

    Dst[MIN(DstSize - 1, DEnd + index)] = '\0';

    return (SUCCESS);
} /* END _MUStrCat() */

int _MSchedProcessConfig(

    msched_t *S, /* I (modified) */
    char *Value) /* I */

{
    int aindex;

    char *ptr;
    char *TokPtr;

    char ValLine[MAX_MLINE];

    if ((S == NULL) || (Value == NULL) || (Value[0] == '\0')) {
        return (FAILURE);
    }

    /* process value line */

    ptr = _MUStrTok(Value, " \t\n", &TokPtr);

    while (ptr != NULL) {
        /* parse name-value pairs */

        /* FOAMAT:  <VALUE>[,<VALUE>] */

        if (_MUGetPair(ptr, (const char **)_MSchedAttr, &aindex, NULL, TRUE, NULL,
                      ValLine, MAX_MNAME) == FAILURE) {
            /* cannot parse value pair */

            ptr = _MUStrTok(NULL, " \t\n", &TokPtr);

            continue;
        }

        switch (aindex) {
            case msaFBServer:

                _MUURLParse(ValLine, NULL, S->FBServerHost, NULL, 0,
                           &S->FBServerPort, TRUE);

                break;

            case msaServer:

                _MUURLParse(ValLine, NULL, S->ServerHost, NULL, 0,
                           &S->ServerPort, TRUE);

                break;

            case msaMode:

                S->Mode = _MUGetIndex(ValLine, _MSchedMode, FALSE, S->Mode);
                S->SpecMode = S->Mode;

                break;

            default:

                DBG(4, fAM)
                dPrint("WARNING:  sys attribute '%s' not handled\n",
                       _MSchedAttr[aindex]);

                break;
        } /* END switch(aindex) */

        ptr = _MUStrTok(NULL, " \t\n", &TokPtr);
    } /* END while (ptr != NULL) */

    return (SUCCESS);
} /* END _MSchedProcessConfig() */

int _MUGetPair(

    char *String,          /* I */
    const char **AttrName, /* I */
    int *AttrIndex,        /* O */
    char *AttrArray,       /* O (optional) */
    int CmpRelative,       /* I (boolean) */
    int *CmpMode,          /* O (optional) */
    char *ValLine,         /* O */
    int ValSize)           /* I */

{
    char *ptr;

    char tmpLine[MAX_MNAME + 1];
    int index;
    int CIndex;

    int vindex;

    int SQCount;
    int DQCount;

    if ((String == NULL) || (String[0] == '\0') || (AttrName == NULL) ||
        (AttrName[0] == NULL) || (AttrIndex == NULL) || (ValLine == NULL) ||
        (ValSize <= 0)) {
        return (FAILURE);
    }

    /* FORMAT:  [<WS>]<ATTRIBUTE>[\[<INDEX>\]][<WS>]<CMP>[<WS>]<VAL>[<WS>] */
    /* FORMAT:  <CMP>: =,==,+=,-= */

    *AttrIndex = 0;

    if (AttrArray != NULL) AttrArray[0] = '\0';

    ptr = String;

    /* remove leading WS */

    while (isspace(ptr[0])) ptr++;

    /* load attribute */

    for (index = 0; index < MAX_MNAME; index++) {
        if (ptr[index] == '\0') break;

        if (CmpRelative == TRUE) {
            if (isspace(ptr[index]) || (ptr[index] == '=') ||
                (ptr[index] == '+') || (ptr[index] == '-')) {
                break;
            }
        } else {
            if (isspace(ptr[index]) || (ptr[index] == '=') ||
                (ptr[index] == '>') || (ptr[index] == '<')) {
                break;
            }
        }

        if (ptr[index] == '[') {
            int aindex2 = 0;

            /* attr index located */

            tmpLine[index] = '\0';

            for (index = index + 1; index < MAX_MNAME; index++) {
                if ((ptr[index] == ']') || (ptr[index] == '\0')) break;

                if (AttrArray != NULL) {
                    AttrArray[aindex2] = ptr[index];

                    aindex2++;
                }
            } /* END for (index) */

            if (AttrArray != NULL) AttrArray[aindex2] = '\0';

            index++;

            break;
        } /* END if (ptr[index] == '[') */

        tmpLine[index] = ptr[index];
    } /* END for (index) */

    tmpLine[index] = '\0';
    ptr += index;

    if ((*AttrIndex = _MUGetIndex(tmpLine, AttrName, FALSE, 0)) == 0) {
        /* cannot process attr name */

        *AttrIndex = -1;

        return (FAILURE);
    }

    /* remove whitespace */

    while (isspace(ptr[0])) ptr++;

    if (CmpRelative == TRUE) {
        if ((ptr[0] != '=') && (ptr[0] != '+') && (ptr[0] != '-')) {
            return (FAILURE);
        }

        switch (ptr[0]) {
            case '+':

                CIndex = 1;

                break;

            case '-':

                CIndex = -1;

                break;

            default:

                CIndex = 0;

                break;
        } /* END switch(ptr[0]) */

        ptr += 1;
    } else {
        if ((ptr[0] != '=') && (ptr[0] != '<') && (ptr[0] != '>')) {
            return (FAILURE);
        }

        CIndex = _MUCmpFromString(ptr, &index);

        ptr += index;
    }

    if (CmpMode != NULL) *CmpMode = CIndex;

    if (ptr[0] == '=') {
        ptr++;
    }

    /* remove whitespace */

    while (isspace(ptr[0])) ptr++;

    /* load value */

    SQCount = 0;
    DQCount = 0;

    vindex = 0;

    for (index = 0; index < ValSize - 1; index++) {
        if (ptr[index] == '\'') {
            SQCount++;

            continue;
        } else if (ptr[index] == '\"') {
            DQCount++;

            continue;
        }

        if ((!(SQCount % 2) && !(DQCount % 2) && isspace(ptr[index])) ||
            (ptr[index] == '\0')) {
            break;
        }

        ValLine[vindex++] = ptr[index];
    } /* END for (index) */

    ValLine[vindex] = '\0';

    return (SUCCESS);
} /* END _MUGetPair() */

int _MUURLParse(

    char *URL,            /* I (modified) */
    char *Protocol,       /* I (optional) */
    char *HostName,       /* I */
    char *Directory,      /* I (optional) */
    int DirSize,          /* I */
    int *Port,            /* I */
    mbool_t DoInitialize) /* I (boolean) */

{
    char *head;
    char *tail;

    if (URL == NULL) {
        return (FAILURE);
    }

    if (DoInitialize == TRUE) {
        if (Protocol != NULL) Protocol[0] = '\0';

        if (HostName != NULL) HostName[0] = '\0';

        if (Directory != NULL) Directory[0] = '\0';

        if (Port != NULL) *Port = 0;
    } /* END if (DoInitialize == TRUE) */

    /* FORMAT:  [<PROTO>://]<HOST>[:<PORT>][/<DIR>] */

    head = URL;

    if ((tail = strstr(head, "://"))) {
        if (Protocol != NULL)
            _MUStrCpy(Protocol, head, MIN(tail - head + 1, MAX_MNAME));

        head = tail + strlen("://");
    }

    if (!(tail = strchr(head, '/')) && !(tail = strchr(head, ':'))) {
        tail = head + strlen(head);
    }

    if (HostName != NULL)
        _MUStrCpy(HostName, head, MIN(tail - head + 1, MAX_MNAME));

    head = tail;

    if (*head == ':') {
        /* extract port */

        if (Port != NULL) *Port = (int)strtol(head + 1, &tail, 0);

        head = tail;
    }

    if (*head == '/') {
        /* extract directory */

        tail = head + strlen(head);

        if (Directory != NULL)
            _MUStrCpy(Directory, head, MIN(tail - head + 1, DirSize));

        head = tail;
    }

    return (SUCCESS);
} /* END _MUURLParse() */

int _MUCmpFromString(

    char *Line, int *Size)

{
    int index;
    int CIndex;
    int Len;

    const char *FName = "_MUCmpFromString";

    DBG(9, fSTRUCT) dPrint("%s(%s,Size)\n", FName, Line);

    CIndex = 0;

    if (Size != NULL) *Size = 0;

    for (index = 0; _MComp[index] != '\0'; index++) {
        Len = strlen(_MComp[index]);

        if (strncmp(Line, _MComp[index], Len) != 0) continue;

        if (Len == 2) {
            if (Size != NULL) *Size = Len;

            return (index);
        }

        CIndex = index;

        if (Size != NULL) *Size = Len;
    } /* END for (index) */

    if (CIndex == mcmpEQ2)
        CIndex = mcmpEQ;
    else if (CIndex == mcmpNE2)
        CIndex = mcmpNE;

    return (CIndex);
} /* END _MUCmpFromString() */

int __MCLoadEnvironment(

    char *ParName, /* O */
    char *Host,    /* O */
    int *Port)     /* O */

{
    char *ptr;

    /* load environment variables */

    /* load partition */

    if ((ptr = getenv(MSCHED_ENVPARVAR)) != NULL) {
        DBG(4, fCONFIG)
        dPrint("INFO:     loaded environment variable %s=%s\n",
               MSCHED_ENVPARVAR, ptr);

        _MUStrCpy(ParName, ptr, MAX_MNAME);
    } else {
        DBG(5, fCONFIG)
        dPrint("INFO:     partition not set  (using default)\n");

        _MUStrCpy(ParName, GLOBAL_MPARNAME, MAX_MNAME);
    }

    /* get port environment variable */

    if ((ptr = getenv(MParam[pServerPort])) != NULL) {
        DBG(4, fCONFIG)
        dPrint("INFO:     loaded environment variable %s=%s)\n",
               MParam[pServerPort], ptr);

        *Port = (int)strtol(ptr, NULL, 0);
    }

    if ((ptr = getenv(MParam[pServerHost])) != NULL) {
        DBG(4, fCONFIG)
        dPrint("INFO:     using %s environment variable (value: %s)\n",
               MParam[pServerHost], ptr);

        _MUStrCpy(Host, ptr, MAX_MNAME);
    }

    return (SUCCESS);
} /* END __MCLoadEnvironment() */

int _MXMLCreateE(

    mxml_t **E, /* O */
    char *Name) /* I (optional) */

{
    /* NOTE:  should 'Name' be mandatory? */

    if (E == NULL) {
        return (FAILURE);
    }

    if ((*E = (mxml_t *)calloc(1, sizeof(mxml_t))) == NULL) {
        return (FAILURE);
    }

    if ((Name != NULL) && (Name[0] != '\0')) (*E)->Name = strdup(Name);

    return (SUCCESS);
} /* END _MXMLCreateE() */

int _MXMLSetAttr(

    mxml_t *E,                   /* I (modified) */
    char *A,                     /* I */
    void *V,                     /* I */
    enum MDataFormatEnum Format) /* I */

{
    int aindex;
    int iindex;

    int rc;

    char tmpLine[MMAX_LINE];

    char *ptr;

    /* NOTE:  overwrite existing attr if found */

    if ((E == NULL) || (A == NULL)) {
        return (FAILURE);
    }

    if (V != NULL) {
        switch (Format) {
            case mdfString:
            default:

                ptr = (char *)V;

                break;

            case mdfInt:

                sprintf(tmpLine, "%d", *(int *)V);

                ptr = tmpLine;

                break;

            case mdfLong:

                sprintf(tmpLine, "%ld", *(long *)V);

                ptr = tmpLine;

                break;

            case mdfDouble:

                sprintf(tmpLine, "%f", *(double *)V);

                ptr = tmpLine;

                break;
        } /* END switch(Format) */
    } else {
        tmpLine[0] = '\0';

        ptr = tmpLine;
    }

    /* initialize attribute table */

    if (E->AName == NULL) {
        E->AName = (char **)calloc(1, sizeof(char *) * MMAX_XMLATTR);
        E->AVal = (char **)calloc(1, sizeof(char *) * MMAX_XMLATTR);

        if ((E->AName == NULL) || (E->AVal == NULL)) {
            return (FAILURE);
        }

        E->ASize = MMAX_XMLATTR;
        E->ACount = 0;
    }

    /* insert in alphabetical order */

    /* overwrite existing attribute if found */

    iindex = 0;
    rc = 0;

    for (aindex = 0; aindex < E->ACount; aindex++) {
        rc = strcmp(E->AName[aindex], A);

        if (rc > 0) break;

        if (rc == 0) {
            iindex = aindex;

            break;
        }

        iindex = aindex + 1;
    } /* END for (aindex) */

    if (aindex >= E->ACount) {
        iindex = aindex;

        if (aindex >= E->ASize) {
            /* allocate memory */

            E->AName = (char **)realloc(
                E->AName, sizeof(char *) * MAX(16, E->ASize << 1));
            E->AVal = (char **)realloc(E->AVal,
                                       sizeof(char *) * MAX(16, E->ASize << 1));

            if ((E->AVal == NULL) || (E->AName == NULL)) {
                E->ASize = 0;

                return (FAILURE);
            }

            E->ASize <<= 1;
        }
    } /* END if (aindex >= E->ACount) */

    if ((ptr == NULL) && (aindex >= E->ACount)) {
        /* no action required for empty attribute */

        return (SUCCESS);
    }

    /* prepare insertion point */

    if (rc != 0) {
        for (aindex = E->ACount - 1; aindex >= iindex; aindex--) {
            E->AVal[aindex + 1] = E->AVal[aindex];
            E->AName[aindex + 1] = E->AName[aindex];
        } /* END for (aindex) */

        E->AVal[aindex + 1] = NULL;
        E->AName[aindex + 1] = NULL;
    } /* END if (rc != 0) */

    if ((iindex < E->ACount) && (E->AVal[iindex] != NULL))
        free(E->AVal[iindex]);

    E->AVal[iindex] = strdup((ptr != NULL) ? ptr : "");

    if ((rc != 0) || (E->AName[iindex] == NULL)) {
        E->AName[iindex] = strdup(A);

        E->ACount++;
    }

    return (SUCCESS);
} /* END _MXMLSetAttr() */

int _MXMLToString(

    mxml_t *E,          /* I */
    char *Buf,          /* O */
    int BufSize,        /* I */
    char **Tail,        /* O */
    mbool_t NoCompress) /* I (compress Data element) */

{
    char NewLine[MMAX_BUFFER];

    int index;

    int BSpace;

    char *BPtr;

    char *tail;

    int len;

    if (Buf != NULL) {
        Buf[0] = '\0';
    }

    if ((E == NULL) || (Buf == NULL)) {
        return (FAILURE);
    }

    if (BufSize < MMAX_NAME) {
        return (FAILURE);
    }

    BPtr = Buf;
    BSpace = BufSize;

    /* display header */

    BPtr[0] = '<';

    BPtr++;
    BSpace--;

    if (E->Name != NULL) {
        len = strlen(E->Name);

        if (len >= BSpace) {
            /* insufficient space */

            BPtr[0] = '\0';

            return (FAILURE);
        }

        strcpy(BPtr, E->Name);

        BSpace -= len;
        BPtr += len;
    } else {
        strcpy(BPtr, "NA");

        len = strlen("NA");

        BPtr += len;
        BSpace -= len;
    }

    /* display attributes */

    for (index = 0; index < E->ACount; index++) {
        /* FORMAT:  <NAME>="<VAL>" */

        BPtr[0] = ' ';

        BPtr++;
        BSpace--;

        len = strlen(E->AName[index]);

        if (len >= BSpace) {
            /* insufficient space */

            BPtr[0] = '\0';

            return (FAILURE);
        }

        strcpy(BPtr, E->AName[index]);

        BSpace -= len;
        BPtr += len;

        BPtr[0] = '=';

        BPtr++;
        BSpace--;

        BPtr[0] = '"';

        BPtr++;
        BSpace--;

        if ((strchr(E->AVal[index], '<') != NULL) ||
            (strchr(E->AVal[index], '>') != NULL)) {
            /* must replace '<' with '&lt;' and '>' with '&gt;' */

            int index1;
            int index2 = 0;

            len = strlen(E->AVal[index]);

            for (index1 = 0; index1 < len; index1++) {
                if (E->AVal[index][index1] == '<') {
                    NewLine[index2++] = '&';
                    NewLine[index2++] = 'l';
                    NewLine[index2++] = 't';
                    NewLine[index2++] = ';';
                } else if (E->AVal[index][index1] == '>') {
                    NewLine[index2++] = '&';
                    NewLine[index2++] = 'g';
                    NewLine[index2++] = 't';
                    NewLine[index2++] = ';';
                } else {
                    NewLine[index2++] = E->AVal[index][index1];
                }
            } /* END for (index1) */

            NewLine[index2] = '\0';

            len = strlen(NewLine);

            if (len >= BSpace) {
                /* insufficient space */

                BPtr[0] = '\0';

                return (FAILURE);
            }

            strcpy(BPtr, NewLine);
        } /* END if (strchr(E->AVal,'<')...) */
        else {
            len = strlen(E->AVal[index]);

            if (len >= BSpace) {
                /* insufficient space */

                BPtr[0] = '\0';

                return (FAILURE);
            }

            strcpy(BPtr, E->AVal[index]);
        } /* END else */

        BSpace -= len;
        BPtr += len;

        BPtr[0] = '"';

        BPtr++;
        BSpace--;
    } /* END for (index) */

    BPtr[0] = '>';

    BPtr++;
    BSpace--;

    if (E->Val != NULL) {
        len = strlen(E->Val);

        if (len >= BSpace) {
            /* insufficient space */

            BPtr[0] = '\0';

            return (FAILURE);
        }

        strcpy(BPtr, E->Val);

        BSpace -= len;
        BPtr += len;
    }

    /* display children */

    for (index = 0; index < E->CCount; index++) {
        if (E->C[index] == NULL) continue;

        if (_MXMLToString(E->C[index], BPtr, BSpace, &tail, NoCompress) ==
            FAILURE) {
            return (FAILURE);
        }

        len = strlen(BPtr);

        BSpace -= len;
        BPtr += len;
    } /* END for (index) */

    /* display footer */

    if (E->Name != NULL) {
        len = strlen(E->Name);
    } else {
        len = strlen("NA");
    }

    if (BSpace < len + 4) {
        BPtr[0] = '\0';

        return (FAILURE);
    }

    BPtr[0] = '<';

    BPtr++;
    BSpace--;

    BPtr[0] = '/';

    BPtr++;
    BSpace--;

    if (E->Name != NULL) {
        strcpy(BPtr, E->Name);
    } else {
        strcpy(BPtr, "NA");
    }

    BSpace -= len;
    BPtr += len;

    BPtr[0] = '>';

    BPtr++;
    BSpace--;

    /* terminate string */

    BPtr[0] = '\0';

    if (Tail != NULL) *Tail = BPtr;

    if ((NoCompress == FALSE) && (strlen(Buf) > (MMAX_BUFFER >> 1))) {
        if ((E->Name != NULL) && !strcmp(E->Name, "Data")) {
            /* attempt to compress in place */

            if (_MSecCompress((unsigned char *)Buf, strlen(Buf), NULL,
                             MCONST_CKEY) == FAILURE) {
                return (FAILURE);
            }
        }
    }

    return (SUCCESS);
} /* END _MXMLToString() */

int _MXMLDestroyE(

    mxml_t **EP) /* I (modified) */

{
    int index;

    mxml_t *E;

    if (EP == NULL) {
        return (FAILURE);
    }

    E = *EP;

    if (E == NULL) {
        return (SUCCESS);
    }

    if (E->C != NULL) {
        /* destroy children */

        for (index = 0; index < E->CCount; index++) {
            if (E->C[index] == NULL) continue;

            _MXMLDestroyE(&E->C[index]);
        } /* END for (index) */

        free(E->C);
    } /* END if (E->C != NULL) */

    /* free attributes */

    if (E->AName != NULL) {
        for (index = 0; index < E->ACount; index++) {
            if (E->AName[index] == NULL) break;

            free(E->AName[index]);

            if ((E->AVal != NULL) && (E->AVal[index] != NULL))
                free(E->AVal[index]);
        } /* END for (index) */

        if (E->AVal != NULL) {
            free(E->AVal);
        }

        if (E->AName != NULL) {
            free(E->AName);
        }
    } /* END if (E->AName != NULL) */

    /* free name */

    if (E->Name != NULL) free(E->Name);

    if (E->Val != NULL) free(E->Val);

    free(E);

    *EP = NULL;

    return (SUCCESS);
} /* END _MXMLDestroyE() */

int _MSecCompress(

    unsigned char *SrcString, /* I */
    unsigned int SrcSize,     /* I */
    unsigned char *DstString, /* O (optional,if NULL, populate SrcString) */
    char *EKey) /* I (optional - enables encryption when present) */

{
    static char *LocalDstString = NULL;
    static int LocalDstSize = 0;

    char *Dest;

    int Hash[4096];

    int NewLength;

    unsigned int Key;
    unsigned int Size;
    unsigned int Pos;
    unsigned int Command = 0;
    unsigned int X = 0;
    unsigned int Y = 9;
    unsigned int Z = 3;

    unsigned char Bit = 0;

#ifndef __MPROD
    const char *FName = "_MSecCompress";

    MDB(2, fSTRUCT)
    MLog("%s(SrcString,%d,DstString,%s)\n", FName, SrcSize,
         (EKey != NULL) ? "EKey" : "NULL");
#endif /* !__MPROD */

    NewLength = (int)(ceil(SrcSize / 3.0) * 4) + 1;

    MDB(7, fSTRUCT)
    MLog("INFO:     compressing data (Original Size - %d)\n", SrcSize);

    NewLength = (int)(ceil(SrcSize / 3.0) * 4) + 1;

    if (DstString == NULL) {
        /* use static local buffer */

        if (LocalDstString == NULL) {
            LocalDstString = (char *)calloc(NewLength, 1);

            LocalDstSize = NewLength;
        } else if (LocalDstSize < NewLength) {
            LocalDstString = (char *)realloc(LocalDstString, NewLength);

            LocalDstSize = NewLength;
        }

        Dest = LocalDstString;
    } else {
        Dest = (char *)DstString;
    }

    for (Key = 0; Key < 4096; Key++) Hash[Key] = -1;

    Dest[0] = FLAG_Compress;

    /* Y = 9, initialize Dest up Y */

    Dest[1] = '\0';
    Dest[2] = '\0';
    Dest[3] = '\0';
    Dest[4] = '\0';
    Dest[5] = '\0';
    Dest[6] = '\0';
    Dest[7] = '\0';
    Dest[8] = '\0';
    Dest[9] = '\0';

    for (; (X < SrcSize) && (Y <= SrcSize);) {
        if (Bit > 15) {
            Dest[Z++] = (Command >> 8) & 0x00ff;
            Dest[Z] = Command & 0x00ff;
            Z = Y;
            Bit = 0;
            Y += 2;
        }

        for (Size = 1; (SrcString[X] == SrcString[X + Size]) &&
                       (Size < 0x0fff) && (X + Size < SrcSize);
             Size++)
            ;

        if (Size >= 16) {
            Dest[Y++] = 0;
            Dest[Y++] = ((Size - 16) >> 8) & 0x00ff;
            Dest[Y++] = (Size - 16) & 0x00ff;
            Dest[Y++] = SrcString[X];
            X += Size;
            Command = (Command << 1) + 1;
        } else if (_MSecCompressionGetMatch(SrcString, X, SrcSize, Hash, &Size,
                                           (int *)&Pos)) {
            Key = ((X - Pos) << 4) + (Size - 3);
            Dest[Y++] = (Key >> 8) & 0x00ff;
            Dest[Y++] = Key & 0x00ff;
            X += Size;
            Command = (Command << 1) + 1;
        } else {
            Dest[Y++] = SrcString[X++];
            Command = (Command << 1);
        }

        Bit++;
    } /* END for (X) */

    if (Dest == LocalDstString) {
        /* pad end of data */

        Dest[Y] = '\0';
        Dest[Y + 1] = '\0';
    }

    Command <<= (16 - Bit);

    Dest[Z++] = (Command >> 8) & 0x00ff;
    Dest[Z] = Command & 0x00ff;

    if (Y > SrcSize) {
        memmove(Dest + 1, SrcString, SrcSize);
        Dest[0] = FLAG_Copied;

        return (SrcSize + 1);
    }

    if (EKey != NULL) {
        _MSecEncryption(Dest, EKey, Y);
    }

    _MSecCompBufTo64BitEncoding(
        Dest, Y, (DstString == NULL) ? (char *)SrcString : (char *)DstString);

    MDB(7, fSTRUCT)
    MLog("INFO:     compressed data size - %d\n",
         (DstString == NULL) ? strlen((char *)SrcString)
                             : strlen((char *)DstString));

    return (SUCCESS);
} /* END _MSecCompress() */

int _MSecCompressionGetMatch(

    unsigned char *Source, unsigned int X, unsigned int SourceSize, int *Hash,
    unsigned int *Size, int *Pos) /* O */

{
    unsigned int HashValue =
        (40543L * ((((Source[X] << 4) ^ Source[X + 1]) << 4) ^ Source[X + 2]) >>
         4) &
        0xfff;

    *Pos = Hash[HashValue];

    Hash[HashValue] = X;

    if ((*Pos != -1) && ((X - *Pos) < 4096)) {
        for (*Size = 0;
             ((*Size < 18) && (Source[X + *Size] == Source[*Pos + *Size]) &&
              ((X + *Size) < SourceSize));
             (*Size)++)
            ;

        return (*Size >= 3);
    }

    return (FALSE);
} /* END _MSecCompressionGetMatch() */

int _MSecEncryption(

    char *SrcString, char *Key, int SrcSize)

{
    int r;
    char *cp_val;
    char *cp_key;
    char result;

    r = 0;

    if ((SrcString != NULL) && (Key != NULL)) {
        cp_val = SrcString;
        cp_key = Key;

        while (r < SrcSize) {
            result = *cp_val ^ *cp_key;
            *cp_val = result;
            cp_val++;
            cp_key++;
            if (*cp_key == '\0') cp_key = Key;
            r++;
        }
    }

    return (SUCCESS);
} /* END _MSecEncryption() */

int _MSecCompBufTo64BitEncoding(

    char *IBuf, int IBufLen, char *OBuf) /* size = X */

{
    int IIndex = 0;
    int OIndex = 0;

    if ((OBuf == NULL) || (IBuf == NULL)) {
        return (FAILURE);
    }

    do {
        OBuf[OIndex++] = CList[(IBuf[IIndex] >> 2) & 0x3f];

        OBuf[OIndex++] = CList[((IBuf[IIndex] << 4) & 0x30) |
                               ((IBuf[IIndex + 1] >> 4) & 0x0f)];

        if (IIndex + 1 < IBufLen)
            OBuf[OIndex++] = CList[((IBuf[IIndex + 1] << 2) & 0x3c) |
                                   ((IBuf[IIndex + 2] >> 6) & 0x03)];
        else
            OBuf[OIndex++] = '=';

        if (IIndex + 2 < IBufLen)
            OBuf[OIndex++] = CList[((IBuf[IIndex + 2]) & 0x3f)];
        else
            OBuf[OIndex++] = '=';

        IIndex += 3;
    } while (IIndex < IBufLen);

    OBuf[OIndex++] = '\0';

    return (SUCCESS);
} /* _MSecCompBufTo64BitEncoding() */

int _MSUInitialize(

    msocket_t *S,    /* I (modified) */
    char *RHostName, /* I (optional) */
    int RPort,       /* I */
    long CTimeout,   /* I */
    long SFlags)     /* I */

{
    if (S == NULL) {
        return (FAILURE);
    }

    memset(S, 0, sizeof(msocket_t));

    S->sd = -1;

    /* version is unknown, known versions are > 0 */

    S->Version = -1;

    _MUStrCpy(S->RemoteHost, RHostName, sizeof(S->RemoteHost));

    S->RemotePort = RPort;

    S->Timeout = CTimeout;

    S->Flags = SFlags;

    return (SUCCESS);
} /* END _MSUInitialize() */

int _MSUConnect(

    msocket_t *S,  /* I (modified) */
    mbool_t Force, /* I */
    char *EMsg)    /* O (optional,minsize=MMAX_LINE) */

{
    struct sockaddr_in s_sockaddr;
    struct hostent *s_hostent;
    struct in_addr in;

    int flags;

    char *hptr = "localhost";

#ifndef __MPROD
    const char *FName = "_MSUConnect";

    MDB(4, fSOCK)
    MLog("%s(%s,%s,EMsg)\n", FName, (S != NULL) ? "S" : "NULL", _MBool[Force]);
#endif /* !__MPROD */

    if (EMsg != NULL) EMsg[0] = '\0';

    if (S == NULL) {
        if (EMsg != NULL) strcpy(EMsg, "invalid parameter");

        return (FAILURE);
    }

    if (S->sd >= 0) {
        if (Force == FALSE) {
            /* connection already established */

            return (SUCCESS);
        }

        _MSUDisconnect(S);
    }

    S->sd = -1;

    memset(&s_sockaddr, 0, sizeof(s_sockaddr));

    if ((S->RemoteHost[0] != '\0')) {
        hptr = S->RemoteHost;
    }

    if (inet_aton(hptr, &in) == 0) {
        if ((s_hostent = gethostbyname(hptr)) == (struct hostent *)NULL) {
            MDB(1, fSOCK)
            MLog(
                "ERROR:    cannot resolve IP address from hostname '%s', "
                "errno: %d (%s)\n",
                hptr, errno, strerror(errno));

            if (EMsg != NULL) strcpy(EMsg, "cannot resolve address");

            return (FAILURE);
        }

        memcpy(&s_sockaddr.sin_addr, s_hostent->h_addr, s_hostent->h_length);
    } else {
        memcpy(&s_sockaddr.sin_addr, &in.s_addr, sizeof(s_sockaddr.sin_addr));
    }

    MDB(5, fSOCK)
    MLog("INFO:     trying to connect to %s (Port: %d)\n",
         inet_ntoa(s_sockaddr.sin_addr), S->RemotePort);

    s_sockaddr.sin_family = AF_INET;

    s_sockaddr.sin_port = htons(S->RemotePort);

    if ((S->sd = socket(PF_INET,
                        MISSET(S->Flags, msftTCP) ? SOCK_STREAM : SOCK_DGRAM,
                        0)) < 0) {
        MDB(1, fSOCK)
        MLog("ERROR:    cannot create socket, errno: %d (%s)\n", errno,
             strerror(errno));

        if (EMsg != NULL) strcpy(EMsg, "cannot create socket");

        return (FAILURE);
    }

    fcntl(S->sd, F_SETFD, 1);

    if (MISSET(S->Flags, msftTCP) && (S->Timeout > 0) &&
        (S->SocketProtocol != mspHalfSocket) &&
        (S->SocketProtocol != mspS3Challenge)) {
        /* enable non-blocking mode on client */

        if ((flags = fcntl(S->sd, F_GETFL, 0)) == -1) {
            MDB(1, fSOCK)
            MLog(
                "WARNING:  cannot get socket attribute values, errno: %d "
                "(%s)\n",
                errno, strerror(errno));

            close(S->sd);

            if (EMsg != NULL) strcpy(EMsg, "cannot enable non-blocking mode");

            return (FAILURE);
        }

        if (fcntl(S->sd, F_SETFL, (flags | O_NDELAY)) == -1) {
            MDB(0, fSOCK)
            MLog(
                "WARNING:  cannot set socket NDELAY attribute, errno: %d "
                "(%s)\n",
                errno, strerror(errno));

            close(S->sd);

            if (EMsg != NULL) strcpy(EMsg, "cannot enable non-blocking mode");

            return (FAILURE);
        }

        MDB(5, fSOCK) MLog("INFO:     non-blocking mode established\n");
    }

    if (connect(S->sd, (struct sockaddr *)&s_sockaddr, sizeof(s_sockaddr)) ==
        -1) {
        if ((errno == EINPROGRESS) && (S->Timeout > 0)) {
            /* wait if non-blocking */

            if (_MSUSelectWrite(S->sd, S->Timeout) == FAILURE) {
                /* connect() has taken too long */

                MDB(4, fSOCK)
                MLog(
                    "ERROR:    cannot connect to server '%s' on port %d, "
                    "errno: %d (%s)\n",
                    hptr, S->RemotePort, errno, strerror(errno));

                close(S->sd);

                if (EMsg != NULL) strcpy(EMsg, "cannot establish connection");

                return (FAILURE);
            }
        } else {
            MDB(4, fSOCK)
            MLog(
                "ERROR:    cannot connect to server '%s' on port %d, errno: %d "
                "(%s)\n",
                (hptr != NULL) ? hptr : "NULL", S->RemotePort, errno,
                strerror(errno));

            close(S->sd);

            if (EMsg != NULL) {
                sprintf(EMsg, "cannot establish connection - %s",
                        strerror(errno));
            }

            return (FAILURE);
        } /* END if (errno == EINPROGRESS) && ...) */
    }     /* END if (connect() == -1) */

    MDB(5, fSOCK)
    MLog("INFO:     successful connect to %s server (sd: %d)\n",
         MISSET(S->Flags, msftTCP) ? "TCP" : "UDP", S->sd);

    return (SUCCESS);
} /* END _MSUConnect() */

int _MSUDisconnect(

    msocket_t *S) /* I */

{
#ifndef __MPROD
    const char *FName = "_MSUDisconnect";

    MDB(2, fSOCK) MLog("%s(%s)\n", FName, (S != NULL) ? "S" : "NULL");
#endif /* !__MPROD */

    if (S == NULL) {
        return (SUCCESS);
    }

    if (S->sd <= 0) {
        return (SUCCESS);
    }

    if ((S->SocketProtocol == mspHalfSocket) ||
        (S->SocketProtocol == mspS3Challenge)) {
        /* delay required for half socket connections to allow data to be
         * transmitted */

        /* NOTE:  temporary hack approach */

        /* sleep(1); */
    }

    close(S->sd);

    S->sd = -1;

    _MSUClientCount--;

    return (SUCCESS);
} /* END _MSUDisconnect() */

int _MSUSelectWrite(

    int sd,                  /* I */
    unsigned long TimeLimit) /* I */

{
    struct timeval TimeOut;
    int numfds;

    fd_set wset;

    const char *FName = "_MSUSelectWrite";

    MDB(7, fSOCK) MLog("%s(%d,%lu)\n", FName, sd, TimeLimit);

    FD_ZERO(&wset);

    FD_SET(sd, &wset);

    TimeOut.tv_sec = TimeLimit / 1000000;
    TimeOut.tv_usec = TimeLimit % 1000000;

    numfds = sd;

    if (select(numfds + 1, NULL, &wset, NULL, &TimeOut) > 0) {
        if (FD_ISSET(sd, &wset)) {
            return (SUCCESS);
        }

    } /* END if (select() > 0) */

    return (FAILURE);
} /* END _MSUSelectWrite() */

int _MCSendRequest(

    msocket_t *S) /* I */

{
    const char *FName = "_MCSendRequest";

    DBG(2, fUI) dPrint("%s(%s)\n", FName, (S != NULL) ? "S" : "NULL");
    if (S == NULL) {
        return (FAILURE);
    }
    if (S->SBufSize == 0) S->SBufSize = (long)strlen(S->SBuffer);
    if (_MSUSendData(S, S->Timeout, TRUE, FALSE) == FAILURE) {
        DBG(0, fSOCK)
        dPrint(
            "ERROR:    cannot send request to server %s:%d (server may not be "
            "running)\n",
            S->RemoteHost, S->RemotePort);

        _MSUDisconnect(S);

        return (FAILURE);
    } else {
        DBG(1, fUI) dPrint("INFO:     message sent to server\n");

        DBG(3, fUI)
        dPrint("INFO:     message sent: '%s'\n",
               (S->SBuffer != NULL) ? S->SBuffer : "NULL");
    }
    if (_MSURecvData(S, S->Timeout, TRUE, NULL, NULL) == FAILURE) {
        fprintf(stderr, "ERROR:    lost connection to server\n");

        return (FAILURE);
    }
    DBG(3, fUI) dPrint("INFO:     message received\n");

    DBG(4, fUI)
    dPrint("INFO:     received message '%s' from server\n", S->RBuffer);

    return (SUCCESS);
} /* END _MCSendRequest() */

int _MSUSendData(

    msocket_t *S,              /* I */
    long TimeLimit,            /* I */
    mbool_t DoSocketLayerAuth, /* I */
    mbool_t IsResponse)        /* I */

{
    char TSLine[MMAX_LINE];
    char CKLine[MMAX_LINE];
    char SHeader[MMAX_LINE + MCONST_SMALLPACKETSIZE];

    char CKSum[MMAX_LINE];

    /* NOTE:  RH7.x compiler fails on full local MMAX_SBUFFER tmpSBuf */

    time_t Now;

    long PacketSize;

    char *sptr = NULL;

    enum MStatusCodeEnum SC;

    const char *FName = "_MSUSendData";

#ifndef __MPROD
    MDB(2, fSOCK)
    MLog("%s(%s,%ld,%s,%s)\n", FName, (S != NULL) ? "S" : "NULL", TimeLimit,
         (DoSocketLayerAuth == TRUE) ? "TRUE" : "FALSE",
         (IsResponse == TRUE) ? "TRUE" : "FALSE");
#endif /* !__MPROD */

    /* initialize */
    TSLine[0] = '\0';
    CKLine[0] = '\0';
    SHeader[0] = '\0';

    tmpSBuf[0] = '\0'; /* tmpSBuf is global */

    switch (S->WireProtocol) {
        case mwpS32:

            /* create message framing */

            _MUISCreateFrame(S, TRUE, IsResponse);

            _MXMLToString((mxml_t *)S->SE, tmpSBuf, sizeof(tmpSBuf), NULL,
                         !MSched.EnableEncryption);

            sptr = S->SBuffer;

            S->SBuffer = tmpSBuf;
            S->SBufSize = strlen(tmpSBuf);

            DoSocketLayerAuth = FALSE;

            break;

        case mwpXML:

            if (S->SE != NULL) {
                char tmpStatus[MMAX_LINE];

                int HeadSize;
                int Align;

                /* package string */

                switch (S->SocketProtocol) {
                    case mspHTTP:
                    case mspHTTPClient:

                    {
                        int len;

                        char *BPtr;
                        int BSpace;

                        /* mxml_t *tE = NULL; */

                        BPtr = tmpSBuf;
                        BSpace = sizeof(tmpSBuf);

                        /* build XML header */

                        /* NOTE:  S->E appended to end of header (tE ignored) */

                        len = snprintf(
                            BPtr, BSpace,
                            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");

                        if (len >= 0) {
                            BPtr += len;
                            BSpace -= len;
                        }

                        /*
                        MXMLCreateE(&tE,"Message");

                        MXMLSetAttr(
                          tE,
                          "xmlns",
                          "http://www.scidac.org/ScalableSystems/AllocationManager",
                          mdfString);

                        MXMLSetAttr(
                          tE,
                          "xmlns:xsi",
                          "http://www.w3.org/2001/XMLSchema-instance",
                          mdfString);

                        MXMLSetAttr(
                          tE,
                          "xsi:schemaLocation",
                          "http://www.scidac.org/ScalableSystems/AllocationManager
                        am.xsd",
                          mdfString);

                        _MXMLAddE(tE,S->E);
                        */

                        _MXMLToString((mxml_t *)S->SE, BPtr, BSpace, NULL, TRUE);

                        /*
                        _MXMLDestroyE(&tE);
                        S->SE = NULL;
                        */

                        _MXMLDestroyE((mxml_t **)&S->SE);
                    } /* END BLOCK */

                    break;

                    default:

                    {
                        char *BPtr;
                        int BSpace;

                        sptr = S->SBuffer;

                        S->SBuffer = tmpSBuf;

                        _MUSNInit(&BPtr, &BSpace, tmpSBuf, sizeof(tmpSBuf));

                        _MUSNPrintF(&BPtr, &BSpace, "%s%d ",
                                   _MCKeyword[mckStatusCode], scSUCCESS);

                        Align = (int)strlen(tmpSBuf) +
                                (int)strlen(_MCKeyword[mckArgs]);

                        _MUSNPrintF(&BPtr, &BSpace, "%*s%s", 16 - (Align % 16),
                                   " ", _MCKeyword[mckData]);

                        HeadSize = (int)strlen(S->SBuffer);

                        _MXMLToString((mxml_t *)S->SE, S->SBuffer + HeadSize,
                                     sizeof(tmpSBuf), NULL, TRUE);

                        if (_MXMLGetAttr((mxml_t *)S->SE, "status", NULL,
                                        tmpStatus,
                                        sizeof(tmpStatus)) == SUCCESS) {
                            char *ptr;

                            ptr = S->SBuffer + strlen(_MCKeyword[mckStatusCode]);

                            *ptr = tmpStatus[0];
                        }
                    }

                    break;
                } /* END switch(S->SocketProtocol) */

                S->SBufSize = strlen(S->SBuffer) + 1;
            } /* END if (S->SE != NULL) */

            break;

        default:

            /* no where else is this set until here */

            if (S->SBuffer != NULL) {
                S->SBufSize = strlen(S->SBuffer);
            } else {
                S->SBufSize = 0;

                /* FAIL and exit below */
            }

            break;
    } /* END switch (S->WireProtocol) */

    /* initialize connection/build header */

    if (S->SBuffer == NULL) {
        MDB(2, fSOCK) MLog("ALERT:    empty message in %s\n", FName);

        return (FAILURE);
    }

    SHeader[0] = '\0';

    switch (S->SocketProtocol) {
        case mspHalfSocket:

            /* NO-OP */

            break;

        case mspS3Challenge:

        {
            char *ptr;

            char tmpChallenge[MMAX_LINE];
            char tmpResponse[MMAX_LINE];

            char tmpLine[MMAX_LINE];

            /* NOTE:  assume previously connected */

            if (S->CSKey[0] != '\0') {
                /* must receive challenge to generate header */

                /* NOTE:  read to first '\n' */

                ptr = tmpChallenge;

                if (_MSURecvPacket(S->sd, &ptr, MMAX_LINE, "\n",
                                  MAX(1000000, TimeLimit), NULL) == FAILURE) {
                    MDB(1, fSOCK)
                    MLog("ALERT:    cannot read half socket data\n");

                    return (FAILURE);
                }

                /* remove '\n' */

                ptr[strlen(ptr) - 1] = '\0';

                if (_MSecGetChecksum(tmpChallenge, strlen(tmpChallenge),
                                    tmpResponse, NULL, mcsaMD5,
                                    S->CSKey) == FAILURE) {
                    MDB(1, fSOCK) MLog("ALERT:    cannot determine digest\n");

                    return (FAILURE);
                }

                /* FORMAT:  <DIGEST>\n */

                sprintf(tmpLine, "%s\n", tmpResponse);

                if (_MSUSendPacket(S->sd, tmpLine, strlen(tmpLine),
                                  MAX(1000000, TimeLimit), &SC) == FAILURE) {
                    MDB(1, fSOCK) MLog("ALERT:    cannot send packet data\n");

                    return (FAILURE);
                }

                /* receive response */

                if (_MSURecvPacket(S->sd, &ptr, 1, NULL, MAX(1000000, TimeLimit),
                                  NULL) == FAILURE) {
                    MDB(1, fSOCK)
                    MLog("ALERT:    cannot read half socket data\n");

                    return (FAILURE);
                }

                if (ptr[0] != '1') {
                    MDB(1, fSOCK)
                    MLog("ALERT:    invalid challenge response '%c'\n", ptr[0]);

                    return (FAILURE);
                }
            } /* END if (S->CSKey[0] != '\0') */

            /* FORMAT:  <BYTECOUNT> <MESSAGE> */

            sprintf(SHeader, "%d ", (int)strlen(S->SBuffer));
        } /* END BLOCK */

        break;

        case mspHTTPClient:
        case mspHTTP:

        {
            if (S->URI != NULL) {
                /* FORMAT:  'GET <URI> %s\r\n\r\n' */

                sprintf(SHeader, "GET %s %s\r\n\r\n", S->URI, "HTTP/1.0");
            } else {
                char tmpBuf[MMAX_NAME];

                /* FORMAT:  'POST /SSSRMAP3 HTTP/1.1\r\nContent-Type: text/xml;
                 * '  */
                /*          'charset="UTF-8"\r\nContent-Length:_<LENGTH>\r\n\r\n'
                 */

                sprintf(tmpBuf, "%x", (unsigned int)strlen(S->SBuffer));

                _MUStrToUpper(tmpBuf, NULL, 0);

                /* remove 'Connection: close' */

                sprintf(SHeader,
                        "POST /%s %s\r\nContent-Type: %s; "
                        "charset=\"utf-8\"\r\nTransfer-Encoding: "
                        "%s;\r\n\r\n%s\r\n",
                        "SSSRMAP3", "HTTP/1.1", "text/xml", "chunked", tmpBuf);
            }

            /* append chunk terminator */

            /* perform bounds checking (NYI) */

            S->SBuffer[S->SBufSize] = '0';
            S->SBuffer[S->SBufSize++] = '\r';
            S->SBuffer[S->SBufSize++] = '\n';
            S->SBuffer[S->SBufSize++] = '\0';

        } /* END BLOCK */

        break;

        default:

            /* FORMAT:
             * <SIZE><CHAR>CK=<CKSUM><WS>TS=<TS><WS>ID=<ID><WS>[CLIENT=<CLIENT><WS>]DT=<MESSAGE>
             */

            CKLine[0] = '\0';

            SHeader[sizeof(SHeader) - 1] = '\0';

            if (DoSocketLayerAuth == TRUE) {
                char tmpStr[MMAX_BUFFER];

                time(&Now);

                sprintf(tmpStr, "%s%ld %s%s", _MCKeyword[mckTimeStamp],
                        (long)Now, _MCKeyword[mckAuth],
                        _MUUIDToName(_MOSGetEUID()));

                if (S->Name[0] != '\0') {
                    sprintf(temp_str, " %s%s", _MCKeyword[mckClient], S->Name);
                    strcat(TSLine, temp_str);
                }

                sprintf(TSLine, "%s %s", tmpStr, _MCKeyword[mckData]);
                printf("TSLine:%s, S->SBuffer:%s,S->CSAlgo:%d,S->CSKey:%s\n",TSLine, S->SBuffer,S->CSAlgo,S->CSKey);
                _MSecGetChecksum2(
                    TSLine, strlen(TSLine), S->SBuffer,
                    strlen(S->SBuffer), /* NOTE:  was S->SBufSize */
                    CKSum, NULL, S->CSAlgo, S->CSKey);

                sprintf(CKLine, "%s%s %s", _MCKeyword[mckCheckSum], CKSum,
                        TSLine);
                printf("CKSum:%s\n",CKSum);
            } /* END if (DoSocketLayerAuth == TRUE) */

            printf("CKLine:%s\n",CKLine);

            PacketSize = S->SBufSize;

            if (isprint(S->SBuffer[0])) {
                /* check for binary data - FIXME */

                PacketSize = strlen(S->SBuffer);
            }

            sprintf(SHeader, "%08ld\n%s", PacketSize + (long)strlen(CKLine),
                    CKLine);

            break;
    } /* END switch (S->SocketProtocol) */

    MDB(7, fSOCK) MLog("INFO:     header created '%s'\n", SHeader);

    /* send data */

    switch (S->SocketProtocol) {
        case mspS3Challenge:
        case mspHalfSocket:

            if (SHeader[0] != '\0') {
                MDB(1, fSOCK)
                MLog("ALERT:    cannot send packet header '%s'\n", SHeader);

                if (_MSUSendPacket(S->sd, SHeader, strlen(SHeader),
                                  MAX(1000000, TimeLimit), &SC) == FAILURE) {
                    MDB(1, fSOCK)
                    MLog("ALERT:    cannot send packet header '%s'\n", SHeader);

                    return (FAILURE);
                }
            } /* END if (SHeader[0] != '\0') */

            if (_MSUSendPacket(S->sd, S->SBuffer, S->SBufSize,
                              MAX(1000000, TimeLimit), &SC) == FAILURE) {
                MDB(1, fSOCK) MLog("ALERT:    cannot send packet data\n");

                return (FAILURE);
            }

            if (shutdown(S->sd, SHUT_WR) == -1) {
                MDB(1, fSOCK)
                MLog("ALERT:    cannot close send connections (%d : %s)\n",
                     errno, strerror(errno));

                if (errno != ENOTCONN) {
                    /* cannot properly close connection */

                    /* NOTE:  cannot be certain data was successfully
                     * transferred */

                    return (FAILURE);
                }
            }

            break;

        case mspHTTP:
        default:

            PacketSize = (long)strlen(SHeader);

            if (S->SBufSize <= MCONST_SMALLPACKETSIZE) {
                memcpy(SHeader + PacketSize, S->SBuffer, S->SBufSize);

                PacketSize += S->SBufSize;

                MDB(6, fSOCK)
                MLog("INFO:     sending short packet '%.512s'\n",
                     _MUPrintBuffer(SHeader, PacketSize));
            }

            if (_MSUSendPacket(S->sd, SHeader, PacketSize, TimeLimit, &SC) ==
                FAILURE) {
                MDB(1, fSOCK)
                MLog("ALERT:    cannot send packet header info, '%.128s'\n",
                     SHeader);

                if (SC == mscNoEnt)
                    S->StatusCode = msfConnRejected;
                else
                    S->StatusCode = msfEGWireProtocol;

                return (FAILURE);
            }

            if (S->SBufSize > MCONST_SMALLPACKETSIZE) {
                if (_MSUSendPacket(S->sd, S->SBuffer, S->SBufSize, TimeLimit,
                                  &SC) == FAILURE) {
                    MDB(1, fSOCK) MLog("ALERT:    cannot send packet data\n");

                    return (FAILURE);
                }
            }

            break;
    } /* END switch (S->SocketProtocol) */

    /* clean up data */

    switch (S->WireProtocol) {
        case mwpS32:

            S->SBuffer = sptr;

            break;

        case mwpXML:

            if (S->SE != NULL) {
                _MXMLDestroyE((mxml_t **)&S->SE);
            }

            S->SBuffer = sptr;

            break;

        default:

            /* NO-OP */

            break;
    } /* END switch (S->WireProtocol) */

    return (SUCCESS);
} /* END _MSUSendData() */

int _MSURecvData(

    msocket_t *S,             /* I (modified) */
    long TimeLimit,           /* I */
    mbool_t DoAuthenticate,   /* I */
    enum MStatusCodeEnum *SC, /* O (optional) */
    char *EMsg)               /* O (optional,minsize=MMAX_LINE) */

{
    char tmpLine[MMAX_LINE];

    long TSVal;

    char CKLine[MMAX_LINE];
    char CKSum[MMAX_LINE];

    char *ptr;
    char *ptr2;

    char *dptr;

    time_t Now;

    char TMarker[MMAX_NAME];

/* read data from socket.  socket is NOT closed */

#ifndef __MPROD
    const char *FName = "_MSURecvData";

    MDB(2, fSOCK)
    MLog("%s(%s,%ld,%s,SC,EMsg)\n", FName, (S != NULL) ? "S" : "NULL",
         TimeLimit, _MBool[DoAuthenticate]);
#endif /* !__MPROD */

    /* initialize */

    if (SC != NULL) *SC = mscNoError;

    if (EMsg != NULL) EMsg[0] = '\0';

    if (S == NULL) {
        MDB(1, fSOCK) MLog("ALERT:    invalid socket pointer received\n");

        if (EMsg != NULL) strcpy(EMsg, "invalid socket received");

        if (SC != NULL) *SC = mscBadParam;

        return (FAILURE);
    }

    if (S->sd <= 0) {
        MDB(1, fSOCK) MLog("ALERT:    socket is closed\n");

        if (EMsg != NULL) strcpy(EMsg, "socket is closed");

        if (SC != NULL) *SC = mscNoEnt;

        return (FAILURE);
    }

    S->RBuffer = NULL;

    TMarker[0] = '\0';
    switch (S->SocketProtocol) {
        case mspS3Challenge:

        {
            char tmpLine[MMAX_LINE];

            char *ptr;

            /* read bytecount */

            ptr = tmpLine;

            memset(tmpLine, 0, MMAX_LINE);

            if (_MSURecvPacket(S->sd, &ptr, 0, " ", TimeLimit, SC) == FAILURE) {
                MDB(1, fSOCK) MLog("ALERT:    cannot read bytecount\n");

                if (EMsg != NULL) strcpy(EMsg, "cannot read byte count");

                if (SC != NULL) *SC = mscNoEnt;

                return (FAILURE);
            }

            /* terminate buffer */

            /* NYI */

            S->RBufSize = strtol(ptr, NULL, 10);

            if ((S->RBufSize > MMSG_BUFFER) || (S->RBufSize <= 0)) {
                /* reject empty messages and potential denial of service attacks
                 */
                /* allow packets between 1 and 2MB bytes */

                MDB(1, fSOCK)
                MLog("ALERT:    invalid packet size (%ld)\n", S->RBufSize);

                if (EMsg != NULL) {
                    sprintf(EMsg, "invalid packet size requested - %ld bytes",
                            S->RBufSize);
                }

                if (SC != NULL) *SC = mscNoMemory;

                return (FAILURE);
            }
            if ((S->RBuffer = (char *)calloc(S->RBufSize + 1, 1)) == NULL) {
                MDB(1, fSOCK)
                MLog(
                    "ERROR:    cannot allocate buffer space (%ld bytes "
                    "requested)  errno: %d (%s)\n",
                    S->RBufSize, errno, strerror(errno));

                if (EMsg != NULL) {
                    sprintf(EMsg, "cannot allocate %ld bytes for message",
                            S->RBufSize);
                }

                if (SC != NULL) *SC = mscNoMemory;

                return (FAILURE);
            }

            if (_MSURecvPacket(S->sd, &S->RBuffer, S->RBufSize, NULL,
                              MAX(TimeLimit, 1000000), NULL) == FAILURE) {
                MDB(1, fSOCK)
                MLog("ALERT:    cannot receive packet (%ld bytes requested)\n",
                     S->RBufSize);

                _MUFree(&S->RBuffer);

                if (EMsg != NULL) {
                    sprintf(EMsg, "cannot receive %ld bytes for message",
                            S->RBufSize);
                }

                return (FAILURE);
            }
        } /* END BLOCK */

        break;

        case mspHalfSocket:

            if (S->RBuffer == NULL) {
                /* allocate large receive space */

                S->RBufSize = MMAX_BUFFER << 4;
                if ((S->RBuffer = (char *)calloc(S->RBufSize + 1, 1)) == NULL) {
                    MDB(1, fSOCK)
                    MLog(
                        "ERROR:    cannot allocate buffer space (%ld bytes "
                        "requested)  errno: %d (%s)\n",
                        S->RBufSize, errno, strerror(errno));

                    if (EMsg != NULL) {
                        sprintf(EMsg, "cannot allocate %ld bytes for message",
                                S->RBufSize);
                    }

                    return (FAILURE);
                }
            }

            if (_MSURecvPacket(S->sd, &S->RBuffer, 0, NULL, TimeLimit, NULL) ==
                FAILURE) {
                MDB(1, fSOCK) MLog("ALERT:    cannot read half socket data\n");

                if (EMsg != NULL) {
                    sprintf(EMsg, "cannot receive %ld bytes for message",
                            S->RBufSize);
                }

                return (FAILURE);
            }

            break;

        case mspHTTP:
        case mspHTTPClient:

            /* FORMAT:  'Content-Length: ' or 'Transfer-Encoding' marker */

            {
                ptr = tmpLine;

                /* load HTTP header */

                if (_MSURecvPacket(S->sd, &ptr, sizeof(tmpLine), "\r\n\r\n",
                                  MAX(TimeLimit, 1000000), NULL) == FAILURE) {
                    MDB(1, fSOCK) MLog("ALERT:    cannot load HTTP header\n");

                    if (EMsg != NULL)
                        strcpy(EMsg, "cannot read message header");

                    if (SC != NULL) *SC = mscNoEnt;

                    return (FAILURE);
                }

                S->RBufSize = 0;

                if ((ptr2 = _MUStrStr(ptr, "transfer-encoding:", 0, TRUE,
                                     FALSE)) != NULL) {
                    ptr2 += strlen("transfer-encoding:") + 1;

                    /* check if chunked */

                    if (_MUStrStr(ptr, "chunked", 0, TRUE, FALSE) != NULL) {
                        /* read initial chunk size */

                        if (_MSURecvPacket(S->sd, &ptr2, sizeof(tmpLine), "\r\n",
                                          MAX(TimeLimit, 1000000),
                                          NULL) == FAILURE) {
                            MDB(1, fSOCK)
                            MLog("ALERT:    cannot load HTTP chunk size\n");

                            if (EMsg != NULL)
                                strcpy(EMsg, "cannot load initial chunk size");

                            if (SC != NULL) *SC = mscNoMemory;

                            return (FAILURE);
                        }

                        /* NOTE:  chunks are in hex */

                        if ((S->RBufSize = strtol(ptr2, NULL, 16)) <= 0) {
                            /* invalid packet length located */

                            MDB(1, fSOCK)
                            MLog("ALERT:    cannot determine packet size\n");

                            if (EMsg != NULL) {
                                sprintf(
                                    EMsg,
                                    "cannot parse initial chunk size - %.16s",
                                    ptr2);
                            }

                            return (FAILURE);
                        }
                    }
                } /* if ((ptr2 = _MUStrStr(ptr)) != NULL) */

                if (S->RBufSize != 0) {
                    /* chunk length already located */

                    /* NO-OP */
                } else if (((ptr2 = _MUStrStr(ptr, "content-length:", 0, TRUE,
                                             FALSE)) != NULL) ||
                           ((ptr2 = _MUStrStr(ptr, "content-length:", 0, TRUE,
                                             FALSE)) != NULL)) {
                    /* extract packet length */

                    ptr2 += strlen("content-length:") + 1;

                    if ((S->RBufSize = strtol(ptr2, NULL, 10)) <= 0) {
                        /* invalid packet length located */

                        MDB(1, fSOCK)
                        MLog("ALERT:    cannot determine packet size\n");

                        if (EMsg != NULL) {
                            sprintf(EMsg,
                                    "invalid http packet size received - %.16s",
                                    ptr2);
                        }

                        return (FAILURE);
                    }
                } else {
                    MDB(6, fSOCK)
                    MLog("NOTE:     packet length not specified (%.32s)\n",
                         ptr);

                    strcpy(TMarker, "</html>");

                    /* create 'adequate' buffer */

                    S->RBufSize = MMAX_BUFFER;
                }

                if (S->RBuffer == NULL) {
                    /* allocate receive space */
                    if ((S->RBuffer = (char *)calloc(S->RBufSize + 1, 1)) ==
                        NULL) {
                        MDB(1, fSOCK)
                        MLog(
                            "ERROR:    cannot allocate buffer space (%ld bytes "
                            "requested)  errno: %d (%s)\n",
                            S->RBufSize, errno, strerror(errno));

                        if (EMsg != NULL) {
                            sprintf(EMsg,
                                    "cannot allocate %d bytes for message",
                                    (int)S->RBufSize);
                        }

                        if (SC != NULL) *SC = mscNoMemory;

                        return (FAILURE);
                    }
                }

                /* read data */

                if (_MSURecvPacket(S->sd, &S->RBuffer, S->RBufSize,
                                  (TMarker[0] != '\0') ? TMarker : NULL,
                                  MAX(TimeLimit, 1000000), NULL) == FAILURE) {
                    MDB(1, fSOCK) MLog("ALERT:    cannot read HTTP data\n");

                    if (EMsg != NULL) strcpy(EMsg, "cannot load HTTP data");

                    if (SC != NULL) *SC = mscNoEnt;

                    return (FAILURE);
                }

                /* terminate buffer */

                S->RBuffer[S->RBufSize] = '\0';

                /* NOTE:  no timestamp, version, or checksum */
            } /* END BLOCK */

            break;

        default:

            ptr = tmpLine;

            ptr[0] = '\0';

            if (TimeLimit != 0) {
                /* allow polling */
                /* TODO: place this logic in other areas? */

                TimeLimit = MAX(TimeLimit, 1000000);
            }

            if (_MSURecvPacket(S->sd, &ptr, 9 * sizeof(char), NULL, TimeLimit,
                              SC) == FAILURE) {
                MDB(1, fSOCK) MLog("ALERT:    cannot determine packet size\n");

                if (EMsg != NULL) strcpy(EMsg, "cannot load packet size");

                if ((SC != NULL) && (*SC != mscNoData)) *SC = mscNoEnt;
                return (FAILURE);
            }

            if (!strncmp(tmpLine, "GET ", strlen("GET "))) {
                if (MSched.HTTPProcessF == NULL) {
                    /* HTTP processing not supported */

                    if (EMsg != NULL)
                        strcpy(EMsg, "http processing not supported");
                    return (FAILURE);
                }

                (*MSched.HTTPProcessF)(S, tmpLine);

                if (shutdown(S->sd, SHUT_WR) == -1) {
                    MDB(1, fSOCK)
                    MLog("ALERT:    cannot close send connections (%d : %s)\n",
                         errno, strerror(errno));

                    if (errno != ENOTCONN) {
                        /* cannot properly close connection */

                        /* NOTE:  cannot be certain data was successfully
                         * transferred */

                        if (EMsg != NULL)
                            strcpy(EMsg, "cannot close connection");
                        return (FAILURE);
                    }
                }

                _MSUDisconnect(S);

                /* NOTE:  return failure to prevent additional processing (temp)
                 */

                if (EMsg != NULL) strcpy(EMsg, "socket is closed");
                return (FAILURE);
            } /* END if (!strncmp(tmpLine,"GET ",strlen("GET "))) */

            tmpLine[8] = '\0';

            /* NOTE:  some strtol() routines fail on zero pad */

            sscanf(tmpLine, "%ld", &S->RBufSize);

            if ((S->RBufSize > (MMAX_BUFFER << 5)) || (S->RBufSize <= 0)) {
                /* reject empty messages and potential denial of service attacks
                 */
                /* allow packets between 1 and 2MB bytes */

                MDB(1, fSOCK)
                MLog("ALERT:    invalid packet size (%ld)\n", S->RBufSize);

                if (EMsg != NULL) strcpy(EMsg, "packet size is invalid");

                if (SC != NULL) *SC = mscNoMemory;
                return (FAILURE);
            }
            if ((S->RBuffer = (char *)calloc(S->RBufSize + 1, 1)) == NULL) {
                MDB(1, fSOCK)
                MLog(
                    "ERROR:    cannot allocate buffer space (%ld bytes "
                    "requested)  errno: %d (%s)\n",
                    S->RBufSize, errno, strerror(errno));

                if (EMsg != NULL)
                    strcpy(EMsg, "cannot allocate memory for message");

                if (SC != NULL) *SC = mscNoMemory;
                return (FAILURE);
            }

            if (_MSURecvPacket(S->sd, &S->RBuffer, S->RBufSize, NULL,
                              MAX(TimeLimit, 1000000), NULL) == FAILURE) {
                MDB(1, fSOCK)
                MLog("ALERT:    cannot receive packet (%ld bytes requested)\n",
                     S->RBufSize);

                _MUFree(&S->RBuffer);

                if (EMsg != NULL) strcpy(EMsg, "cannot read message");

                if (SC != NULL) *SC = mscNoEnt;
                return (FAILURE);
            }

            S->RBuffer[S->RBufSize] = '\0';

            break;
    } /* switch (S->SocketProtocol) */

    if (S->WireProtocol == mwpNONE) {
        /* determine wire protocol */

        if (strstr(S->RBuffer, "<Envelope") != NULL) {
            S->WireProtocol = mwpS32;

            /* set default algorithm */

            S->CSAlgo = MSched.DefaultCSAlgo;
        }
    } /* END if (S->WireProtocol == mwpNONE) */

    /* adjust state */

    switch (S->WireProtocol) {
        case mwpS32:

            /* no socket level authentication required */

            DoAuthenticate = FALSE;

            break;

        default:

            /* NO-OP */

            break;
    } /* END switch (S->WireProtocol) */

    /* authenticate message */

    if (DoAuthenticate == TRUE) {
        switch (S->SocketProtocol) {
            case mspS3Challenge:
            case mspHalfSocket:
            case mspHTTP:
            case mspHTTPClient:

                /* NO-OP */

                break;

            default:

                /* verify packet */

                /* NOTE:  client marker must not be in args */

                /* locate data marker */

                if ((dptr = strstr(S->RBuffer, _MCKeyword[mckData])) == NULL) {
                    MDB(3, fSOCK)
                    MLog("ALERT:    cannot locate command data (%.60s)\n",
                         S->RBuffer);

                    _MUFree(&S->RBuffer);

                    if (EMsg != NULL)
                        strcpy(EMsg, "cannot locate command data");

                    return (FAILURE);
                }

                if ((ptr = strstr(S->RBuffer, _MCKeyword[mckArgs])) != NULL) {
                    /* arg marker located */

                    dptr = ptr;
                }

                /* set defaults */

                S->Version = 0;

                strcpy(S->Name, NONE);

                if (S->CSKey[0] == '\0') strcpy(S->CSKey, MSched.DefaultCSKey);

                S->CSAlgo = MSched.DefaultCSAlgo;

                /* extract client name */

                if (((ptr = strstr(S->RBuffer, _MCKeyword[mckClient])) !=
                     NULL) &&
                    (ptr < dptr)) {
                    ptr += strlen(_MCKeyword[mckClient]);

                    if ((X.XGetClientInfo !=
                         (int (*)(void *, msocket_t *, char *))0) &&
                        ((*X.XGetClientInfo)(X.xd, S, ptr) == SUCCESS)) {
                        /* NOTE:  enable logging only during unit testing */

                        /*
                        MDB(1,fSOCK) MLog("INFO:     using checksum seed '%s'
                        for client '%s'\n",
                          S->SKey,
                          S->Name);
                        */
                    } else {
                        /* use default client detection */

                        for (ptr2 = ptr; (ptr2 - ptr) < MMAX_NAME; ptr2++) {
                            if ((*ptr2 == '\0') || (*ptr2 == ':') ||
                                isspace(*ptr2)) {
#ifdef __M32COMPAT
                                extern mclient_t MClient[];

                                mclient_t *C;

                                int index;

                                _MUStrCpy(
                                    S->Name, ptr,
                                    MIN((long)sizeof(S->Name), ptr2 - ptr + 1));

                                for (index = 0; index < MMAX_CLIENT; index++) {
                                    C = &MClient[index];

                                    if (C->Name[0] == '\0') break;

                                    if (C->Name[0] == '\1') continue;

                                    if (!strcmp(S->Name, C->Name)) {
                                        strcpy(S->CSKey, C->CSKey);

                                        break;
                                    }
                                } /* END for (index) */
#else                             /* __M32COMPAT */
                                mpsi_t *P;

                                _MUStrCpy(S->Name, ptr, MIN((int)sizeof(S->Name),
                                                           ptr2 - ptr + 1));

                                if (MPeerFind(S->Name, &P, FALSE) == SUCCESS) {
                                    if (P->CSKey != NULL)
                                        strcpy(S->CSKey, P->CSKey);
                                }
#endif                            /* __M32COMPAT */

                                if (*ptr2 == ':') {
                                    ptr2++;

                                    S->Version = strtol(ptr2, NULL, 10);
                                }

                                break;
                            }
                        } /* END for (ptr2) */
                    }     /* END else (X.XGetClientInfo != NULL) */
                } /* END if (((ptr = strstr(S->RBuffer,_MCKeyword[mckClient])) !=
                     NULL)... */

                /* get checksum */

                if ((ptr = strstr(S->RBuffer, _MCKeyword[mckCheckSum])) ==
                    NULL) {
                    MDB(1, fSOCK)
                    MLog("ALERT:    cannot locate checksum '%s'\n",
                         _MUPrintBuffer(S->RBuffer, S->RBufSize));

                    free(S->RBuffer);

                    S->RBuffer = NULL;

                    if (EMsg != NULL) strcpy(EMsg, "cannot locate checksum");

                    if (SC != NULL) *SC = mscNoAuth;

                    return (FAILURE);
                }

                ptr += strlen(_MCKeyword[mckCheckSum]);

                _MUStrCpy(CKLine, ptr, sizeof(CKLine));

                for (ptr2 = &CKLine[0]; *ptr2 != '\0'; ptr2++) {
                    if (isspace(*ptr2)) {
                        *ptr2 = '\0';

                        break;
                    }
                } /* END for (ptr2) */

                ptr += strlen(CKLine);

                if ((ptr = strstr(ptr, _MCKeyword[mckTimeStamp])) == NULL) {
                    MDB(1, fSOCK) MLog("ALERT:    cannot locate timestamp\n");

                    _MUFree(&S->RBuffer);

                    if (EMsg != NULL) strcpy(EMsg, "cannot locate timestamp");

                    if (SC != NULL) *SC = mscNoAuth;

                    return (FAILURE);
                }

                /* verify checksum */

                if (S->CSAlgo != mcsaNONE) {
                    _MSecGetChecksum(ptr, strlen(ptr), CKSum, NULL, S->CSAlgo,
                                    S->CSKey);

                    if (strcmp(CKSum, CKLine) != 0) {
                        MDB(1, fSOCK)
                        MLog(
                            "ALERT:    checksum does not match (%s:%s)  "
                            "request '%.120s'\n",
                            CKSum, CKLine, ptr);

#ifdef __M32COMPAT
                        if (strcmp(MSched.Admin4User[0], "ALL"))
#else  /* __M32COMPAT */
                        if (strcmp(MSched.Admin[4].UName[0], "ALL"))
#endif /* __M32COMPAT */
                        {
                            _MUFree(&S->RBuffer);

                            if (EMsg != NULL)
                                strcpy(EMsg, "invalid message authentication");

                            if (SC != NULL) *SC = mscNoAuth;

                            return (FAILURE);
                        }
                    } /* END if (strcmp(CKSum,CKLine) != 0) */
                }     /* END if (S->CSKey != NULL) */

                /* get timestamp */

                ptr += strlen(_MCKeyword[mckTimeStamp]);

                TSVal = strtol(ptr, NULL, 10);

                /* locate data */

                if ((ptr = strstr(ptr, _MCKeyword[mckData])) == NULL) {
                    MDB(1, fSOCK) MLog("ALERT:    cannot locate data\n");

                    _MUFree(&S->RBuffer);

                    if (EMsg != NULL)
                        strcpy(EMsg, "cannot locate message data");

                    return (FAILURE);
                }

                /* verify timestamp */

                time(&Now);

                if ((((long)Now - TSVal) > 3600) ||
                    (((long)Now - TSVal) < -3600)) {
                    MDB(1, fSOCK)
                    MLog("ALERT:    timestamp does not match (%lu:%lu)\n",
                         (unsigned long)Now, (unsigned long)TSVal);

                    _MUFree(&S->RBuffer);

                    if (EMsg != NULL)
                        strcpy(EMsg, "invalid timestamp detected");

                    if (SC != NULL) *SC = mscNoAuth;

                    return (FAILURE);
                }

                break;
        } /* END switch(S->SocketProtocol) */
    }     /* END if (DoAuthenticate == TRUE) */

    /* validate message */

    switch (S->WireProtocol) {
        case mwpS32:

        {
            mxml_t *EE = NULL;
            mxml_t *SE;
            mxml_t *BE;
            mxml_t *RE;
            mxml_t *DE;

            char AName[MMAX_LINE]; /* actor name */
            char tmpLine[MMAX_LINE];
            char tEMsg[MMAX_LINE];

            /* validate envelope */

            if (_MXMLFromString(&EE, S->RBuffer, NULL, tEMsg) == FAILURE) {
                MDB(1, fSOCK)
                MLog(
                    "ALERT:    invalid socket request received (cannot process "
                    "XML - %s)\n",
                    tEMsg);

                _MUFree(&S->RBuffer);

                if (EMsg != NULL) strcpy(EMsg, "cannot parse XML data");

                return (FAILURE);
            }

            S->RE = (void *)EE;

            if (_MXMLGetChild(EE, "Body", NULL, &BE) == FAILURE) {
                _MXMLDestroyE((mxml_t **)&S->RE);

                MDB(1, fSOCK)
                MLog(
                    "ALERT:    invalid socket request received (cannot locate "
                    "body)\n");

                _MUFree(&S->RBuffer);

                if (EMsg != NULL) strcpy(EMsg, "cannot locate message body");

                return (FAILURE);
            }

            /* NOTE: 'should' be deprecated - actor located in Request element
             * (S3 3.0) */

            if (_MXMLGetAttr(BE, "actor", NULL, AName, sizeof(AName)) ==
                SUCCESS) {
                _MUStrDup(&S->RID, AName);
            }

            if (_MXMLGetAttr(EE, "type", NULL, tmpLine, sizeof(tmpLine)) ==
                SUCCESS) {
                if (!strcasecmp(tmpLine, "nonblocking")) {
                    S->IsNonBlocking = TRUE;
                }
            }

            if (S->CSAlgo != mcsaNONE) {
                if (_MXMLGetChild(EE, "Signature", NULL, &SE) == SUCCESS) {
                    mxml_t *DVE;
                    mxml_t *SVE;

                    char *BString;
                    char TChar;
                    char *tail;

                    char tmpLine[MMAX_LINE];

                    mpsi_t *P = NULL;

                    /* process signature */

                    _MXMLGetChild(SE, "DigestValue", NULL, &DVE);
                    _MXMLGetChild(SE, "SignatureValue", NULL, &SVE);

                    tmpLine[0] = '\0';

                    /* extract body string */

                    if ((BString = strstr(S->RBuffer, "<Body")) != NULL) {
                        if ((tail = strstr(BString, "</Body>")) != NULL) {
                            tail += strlen("</Body>");

                            TChar = *tail;
                            *tail = '\0';

                            if (S->CSKey[0] == '\0') {
/* determine key from actor */

#ifdef __M32COMPAT
                                strcpy(S->CSKey, MSched.DefaultCSKey);
#else  /* __M32COMPAT */
                                if ((MPeerFind(AName, &P, FALSE) == SUCCESS) &&
                                    (P->CSKey != NULL)) {
                                    strcpy(S->CSKey, P->CSKey);
                                } else if ((MPeerFind(S->RemoteHost, &P,
                                                      TRUE) == SUCCESS) &&
                                           (P->CSKey != NULL)) {
                                    /* try to look-up using hostname */

                                    strcpy(S->CSKey, P->CSKey);

                                    /* add peer to auth */

                                    MSchedAddAdmin(AName, P->RIndex);
                                } else {
                                    /* use default */

                                    strcpy(S->CSKey, MSched.DefaultCSKey);
                                }
#endif /* __M32COMPAT */
                            }

                            _MSecGetChecksum(BString, strlen(BString), tmpLine,
                                            NULL, mcsaHMAC64, S->CSKey);

                            *tail = TChar;
                        }
                    } else {
                        /* FAILURE: message contains no body */

                        _MXMLDestroyE((mxml_t **)&S->RE);

                        MDB(1, fSOCK)
                        MLog(
                            "ALERT:    invalid socket request received (cannot "
                            "locate body marker)\n");

                        _MUFree(&S->RBuffer);

                        if (EMsg != NULL)
                            strcpy(EMsg, "cannot locate message body");

                        if (SC != NULL) *SC = mscBadRequest;

                        return (FAILURE);
                    }

                    if (strcmp(SVE->Val, tmpLine)) {
                        /* signatures do not match */

                        /* NOTE:  if checksum does not match, attempt to locate
                         * error message from server */

                        if (_MXMLGetChild(BE, _MSON[msonResponse], NULL, &RE) ==
                            SUCCESS) {
                            char tmpLine[MMAX_LINE];

                            enum MSFC tSC;

                            if (_MS3CheckStatus(RE, &tSC, tmpLine) == FAILURE) {
                                if (tmpLine[0] != '\0')
                                    _MUStrDup(&S->SMsg, tmpLine);

                                S->StatusCode = (long)tSC;

                                if (EMsg != NULL) {
                                    snprintf(EMsg, MMAX_LINE,
                                             "remote server rejected request, "
                                             "message '%s'",
                                             tmpLine);
                                }
                            }
                        }

                        _MXMLDestroyE((mxml_t **)&S->RE);

                        MDB(1, fSOCK)
                        MLog("ALERT:    signatures do not match\n");

                        _MUFree(&S->RBuffer);

                        if ((EMsg != NULL) && (EMsg[0] == '\0')) {
                            /* attempt to locate client */

                            if (P == NULL) {
                                char *ptr;

                                if (!strncasecmp(AName, "peer:",
                                                 strlen("peer:"))) {
                                    ptr = AName + strlen("peer:");

                                    sprintf(EMsg, "unknown client '%.32s'",
                                            ptr);
                                } else {
                                    sprintf(EMsg,
                                            "invalid key for client '%.32s'",
                                            AName);
                                }
                            } else {
                                strcpy(EMsg, "invalid key for client");
                            }
                        } /* END if ((EMsg != NULL) && ...) */

                        if (SC != NULL) *SC = mscNoAuth;

                        return (FAILURE);
                    } /* if (strcmp(SVE->Val,tmpLine)) */
                } /* END if (_MXMLGetChild(EE,"Signature",NULL,&SE) == SUCCESS)
                     */
                else {
                    /* cannot locate signature element */

                    /* reject message? */

                    /* NYI */
                }
            } else {
                /* no authentication algorithm specified */

                /* NYI */
            }

            /* NOTE:  get next object of either request or response type */

            if (_MXMLGetChild(BE, _MSON[msonRequest], NULL, &RE) == SUCCESS) {
                _MXMLExtractE((mxml_t *)BE, (mxml_t *)RE, (mxml_t **)&S->RDE);

                S->RPtr = NULL;

                if (S->RID == NULL) {
                    /* NOTE:  S->RID should be populated before checksum call is
                     * made */

                    if (_MXMLGetAttr(RE, "actor", NULL, AName, sizeof(AName)) ==
                        SUCCESS) {
                        _MUStrDup(&S->RID, AName);
                    }
                }
            } else if (_MXMLGetChild(BE, _MSON[msonResponse], NULL, &RE) ==
                       SUCCESS) {
                char tmpLine[MMAX_LINE];

                enum MSFC tSC;

                /* load status information if provided */

                if (_MS3CheckStatus(RE, &tSC, tmpLine) == FAILURE) {
                    if (tmpLine[0] != '\0') _MUStrDup(&S->SMsg, tmpLine);
                }

                S->StatusCode = (int)tSC;

                if (_MXMLGetChild(RE, _MSON[msonData], NULL, &DE) == FAILURE) {
                    /* response message received */

                    if (S->StatusCode != 0) {
                        MDB(1, fSOCK)
                        MLog(
                            "ALERT:    request failed with status code %03ld "
                            "(%s)\n",
                            S->StatusCode, (S->SMsg != NULL) ? S->SMsg : "");

                        if (SC != NULL)
                            *SC = (enum MStatusCodeEnum)S->StatusCode;

                        MDB(7, fSOCK)
                        MLog("INFO:     failed request message '%.256s'\n",
                             (S->RBuffer != NULL) ? S->RBuffer : "NULL");

                        _MXMLDestroyE((mxml_t **)&S->RE);

                        _MUFree(&S->RBuffer);

                        if (EMsg != NULL) {
                            if (S->SMsg == NULL) {
                                sprintf(EMsg,
                                        "server rejected request with status "
                                        "code %ld",
                                        S->StatusCode);
                            } else {
                                snprintf(EMsg, MMAX_LINE,
                                         "server rejected request with status "
                                         "code %ld - %s",
                                         S->StatusCode, S->SMsg);
                            }
                        } /* END if (EMsg != NULL) */

                        return (FAILURE);
                    }

                    S->RDE = NULL;
                    S->RPtr = NULL;

                    MDB(1, fSOCK)
                    MLog("INFO:     successfully received socket response\n");
                } else {
                    _MXMLExtractE((mxml_t *)RE, (mxml_t *)DE,
                                 (mxml_t **)&S->RDE);

                    S->RPtr = DE->Val;
                }
            } /* END else if (_MXMLGetChild(BE) == SUCCESS) */
            else {
                _MXMLDestroyE((mxml_t **)&S->RE);

                MDB(1, fSOCK)
                MLog("ALERT:    invalid socket message received\n");

                _MUFree(&S->RBuffer);

                if (EMsg != NULL) strcpy(EMsg, "invalid message type received");

                return (FAILURE);
            } /* END else */
        }     /* END BLOCK */

        break;

        default:

            /* NYI */

            break;
    } /* END switch (S->WireProtocol) */

    S->IsLoaded = TRUE;

    return (SUCCESS);
} /* END _MSURecvData() */

int _MXMLExtractE(

    mxml_t *E,   /* I */
    mxml_t *C,   /* I */
    mxml_t **CP) /* O (optional) */

{
    int cindex;

    if ((E == NULL) || (C == NULL)) {
        return (FAILURE);
    }

    for (cindex = 0; cindex < E->CCount; cindex++) {
        if (C != E->C[cindex]) {
            if (_MXMLExtractE(E->C[cindex], C, CP) == SUCCESS) {
                return (SUCCESS);
            }

            continue;
        }

        if (CP != NULL) *CP = E->C[cindex];

        E->C[cindex] = NULL;

        return (SUCCESS);
    } /* END for (cindex) */

    return (FAILURE);
} /* _MXMLExtractE() */

int _MS3CheckStatus(

    mxml_t *E,     /* I */
    enum MSFC *SC, /* O (optional) */
    char *EMsg)    /* O (optional,minsize=MMAX_LINE) */

{
    mxml_t *CE;

#ifdef __MS330
    mxml_t *SE;
#endif /* __MS330 */

#ifndef MOPT

    if (E == NULL) {
        if (SC != NULL) *SC = msfEGMisc;

        if (EMsg != NULL) strcpy(EMsg, "internal error");

        return (FAILURE);
    }

#endif /* MOPT */

    /* set defaults */

    if (SC != NULL) *SC = msfENone;

    if (EMsg != NULL) EMsg[0] = '\0';

#ifdef __MS330

    /*
       <Status>
         <Value>Success|Warning|Failure</Value>  -- required
         <Code>X</Code>                          -- required
         <Message>X</Message>                    -- optional
       </Status>
    */

    if (__MXMLGetChildCI(E, "Status", NULL, &SE) == FAILURE) {
        /* cannot locate status element */

        MDB(3, fS3) MLog("ERROR:    cannot locate message status element\n");

        if (SC != NULL) *SC = msfEGMessage;

        if (EMsg != NULL) strcpy(EMsg, "no status");

        return (FAILURE);
    }

    if ((__MXMLGetChildCI(SE, "Message", NULL, &CE) == FAILURE) ||
        (CE->Val == NULL)) {
        /* cannot locate optional status message */

        MDB(6, fS3) MLog("INFO:     cannot locate status message element\n");
    } else {
        if (EMsg != NULL) MUStrCpy(EMsg, CE->Val, MMAX_LINE);
    }

    if ((__MXMLGetChildCI(SE, "Code", NULL, &CE) == FAILURE) ||
        (CE->Val == NULL)) {
        /* cannot locate status code */

        MDB(3, fS3) MLog("WARNING:  cannot locate status code element\n");

        if (SC != NULL) *SC = msfEGMessage;

        if ((EMsg != NULL) && (EMsg[0] == '\0')) strcpy(EMsg, "no status code");

        return (FAILURE);
    }

    if (SC != NULL) {
        *SC = (enum MSFC)strtol(CE->Val, NULL, 0);
    }

    if (__MXMLGetChildCI(SE, "Value", NULL, &CE) == FAILURE) {
        /* cannot locate status code */

        MDB(3, fS3) MLog("WARNING:  cannot locate status value element\n");

        if ((EMsg != NULL) && (EMsg[0] == '\0'))
            strcpy(EMsg, "no status value");

        return (FAILURE);
    }

    if ((CE->Val == NULL) || !strcmp(CE->Val, "Failure")) {
        /* command failed */

        MDB(3, fS3) MLog("WARNING:  request failed\n");

        return (FAILURE);
    }

#else /* __MS330 */

    if (__MXMLGetChildCI(E, "Status", NULL, &CE) == FAILURE) {
        /* cannot locate status code */

        MDB(3, fS3) MLog("ERROR:    cannot locate message status element\n");

        if (SC != NULL) *SC = msfEGMessage;

        if (EMsg != NULL) strcpy(EMsg, "no status");

        return (FAILURE);
    }

    if (SC != NULL) {
        mxml_t *tE;

        if ((__MXMLGetChildCI(CE, "Code", NULL, &tE) == SUCCESS) &&
            (tE->Val != NULL)) {
            *SC = strtol(tE->Val, NULL, 0);
        } else {
            *SC = msfEGMessage;
        }
    }

    if ((CE->Val == NULL) || strcmp(CE->Val, "true")) {
        /* command failed */

        MDB(3, fS3) MLog("ERROR:    request refused\n");

        if ((EMsg != NULL) &&
            (_MXMLGetChild(E, "Message", NULL, &CE) == SUCCESS)) {
            strcpy(EMsg, CE->Val);
        }

        return (FAILURE);
    }

#endif /* !__MS330 */

    return (SUCCESS);
} /* END _MS3CheckStatus() */

int __MXMLGetChildCI(

    mxml_t *E,   /* I */
    char *CName, /* I (optional) */
    int *CTok,   /* I (optional) */
    mxml_t **CP) /* O (optional) */

{
    int cindex;
    int cstart;

    int SLen;

    if (CP != NULL) *CP = NULL;

#ifndef __MOPT
    if (E == NULL) {
        return (FAILURE);
    }
#endif /* __MOPT */

    if (CTok != NULL)
        cstart = *CTok;
    else
        cstart = -1;

    if (CName != NULL)
        SLen = strlen(CName) + 1;
    else
        SLen = 0;

    for (cindex = cstart + 1; cindex < E->CCount; cindex++) {
        if (E->C[cindex] == NULL) continue;

        if ((CName == NULL) || !strncasecmp(CName, E->C[cindex]->Name, SLen)) {
            if (CP != NULL) *CP = E->C[cindex];

            if (CTok != NULL) *CTok = cindex;

            return (SUCCESS);
        }
    } /* END for (cindex) */

    return (FAILURE);
} /* END __MXMLGetChildCI() */

int _MXMLGetChild(

    mxml_t *E,   /* I */
    char *CName, /* I (optional) */
    int *CTok,   /* I (optional) */
    mxml_t **C)  /* O */

{
    int cindex;
    int cstart;

    if (C != NULL) *C = NULL;

    if ((E == NULL) || (C == NULL)) {
        return (FAILURE);
    }

    if (CTok != NULL)
        cstart = *CTok;
    else
        cstart = -1;

    for (cindex = cstart + 1; cindex < E->CCount; cindex++) {
        if (E->C[cindex] == NULL) continue;

        if ((CName == NULL) || !strcmp(CName, E->C[cindex]->Name)) {
            *C = E->C[cindex];

            if (CTok != NULL) *CTok = cindex;

            return (SUCCESS);
        }
    } /* END for (cindex) */

    return (FAILURE);
} /* END _MXMLGetChild() */

int _MXMLFromString(

    mxml_t **EP,     /* O (populate or create - will be freed on failure) */
    char *XMLString, /* I */
    char **Tail,     /* O (optional) */
    char *EMsg)      /* O (optional) */

{
    mxml_t *E;
    char *ptr;

    char *tail;

    int index;

    mbool_t ElementIsClosed = FALSE;

    mbool_t DoAppend = FALSE;

    char tmpNLine[MMAX_LINE + 1];
    char tmpVLine[MMAX_XBUFFER + 1];

    if (EP != NULL) *EP = NULL;

    if (EMsg != NULL) EMsg[0] = '\0';

    if ((XMLString == NULL) || (EP == NULL)) {
        if (EMsg != NULL) strcpy(EMsg, "invalid arguments");

        return (FAILURE);
    }

    if ((ptr = strchr(XMLString, '<')) == NULL) {
        if (EMsg != NULL) strcpy(EMsg, "no XML in string");

        return (FAILURE);
    }

    if (ptr[1] == '/') {
        /* located tail marker */

        if (EMsg != NULL) strcpy(EMsg, "premature termination marker");

        return (FAILURE);
    }

    /* NOTE:  should support append/overlay parameter (NYI) */

    /* ignore 'meta' elements */

    while ((ptr[1] == '?') || (ptr[1] == '!')) {
        ptr++;

        /* ignore 'meta' elements */

        if (*ptr == '?') {
            ptr++;

            if ((ptr = strstr(ptr, "?>")) == NULL) {
                /* cannot locate end of meta element */

                return (FAILURE);
            }

            if ((ptr = strchr(ptr, '<')) == NULL) {
                /* cannot locate next element */

                if (EMsg != NULL) strcpy(EMsg, "cannot locate post-meta XML");

                return (FAILURE);
            }
        } /* END if (*ptr == '?') */

        /* ignore 'comment' element */

        if (!strncmp(ptr, "!--", 3)) {
            ptr += 3;

            if ((ptr = strstr(ptr, "-->")) == NULL) {
                /* cannot locate end of comment element */

                if (EMsg != NULL)
                    strcpy(EMsg, "cannot locate comment termination marker");

                return (FAILURE);
            }

            if ((ptr = strchr(ptr, '<')) == NULL) {
                /* cannot locate next element */

                if (EMsg != NULL)
                    strcpy(EMsg, "cannot locate post-comment XML");

                return (FAILURE);
            }
        } /* END if (!strncmp(ptr,"!--",3)) */
        else if (ptr[1] == '!') {
            char *ptr2;

            ptr++;
            ptr++;

            while (*ptr != '\0') {
                if (strchr("<[>", *ptr) != NULL) {
                    break;
                }

                ptr++;
            }

            if ((ptr2 = strchr("<[>", *ptr)) == NULL) {
                /* cannot locate end element */

                if (EMsg != NULL)
                    strcpy(EMsg, "cannot locate post-comment XML");

                return (FAILURE);
            }

            switch (*ptr2) {
                case '[':

                    ptr = strstr(ptr, "]>");

                    break;

                default:

                    /* NYI */

                    return (FAILURE);

                    /* NOTREACHED */

                    break;
            }

            if ((ptr = strchr(ptr, '<')) == NULL) {
                /* cannot locate next element */

                if (EMsg != NULL)
                    strcpy(EMsg, "cannot locate post-comment XML");

                return (FAILURE);
            }
        } /* END if (*ptr == '!') */
    }     /* END while ((ptr[1] == '?') || (ptr[1] == '!')) */

    /* remove whitespace */

    while (isspace(*ptr)) ptr++;

    /* extract root element */

    if (*ptr != '<') {
        /* cannot located start of element */

        if (EMsg != NULL) strcpy(EMsg, "cannot locate start of root element");

        return (FAILURE);
    }

    ptr++; /* ignore '<' */

    index = 0;

    while ((*ptr != ' ') && (*ptr != '>')) {
        if ((ptr[0] == '/') && (ptr[1] == '>')) {
            ElementIsClosed = TRUE;

            break;
        }

        tmpNLine[index++] = *(ptr++);

        if ((index >= MMAX_LINE) || (ptr[0] == '\0')) {
            if (EMsg != NULL)
                sprintf(EMsg, "element name is too long - %.10s", tmpNLine);

            return (FAILURE);
        }
    }

    tmpNLine[index] = '\0';

    if ((*EP == NULL) && (_MXMLCreateE(EP, tmpNLine) == FAILURE)) {
        if (EMsg != NULL)
            sprintf(EMsg, "cannot create XML element '%s'", tmpNLine);

        return (FAILURE);
    }

    E = *EP;

    if ((E->ACount > 0) || (E->CCount > 0)) {
        DoAppend = TRUE;
    }

    if (ElementIsClosed == TRUE) {
        ptr += 2; /* skip '/>' */

        if (Tail != NULL) *Tail = ptr;

        return (SUCCESS);
    }

    while (*ptr == ' ') ptr++;

    while (*ptr != '>') {
        /* extract attributes */

        /* FORMAT:  <ATTR>="<VAL>" */

        index = 0;

        while ((*ptr != '=') && (*ptr != '\0')) {
            tmpNLine[index++] = *(ptr++);

            if (index >= MMAX_LINE) break;
        }

        tmpNLine[index] = '\0';

        if (*ptr != '\0') ptr++; /* skip '=' */

        if (*ptr != '\0') ptr++; /* skip '"' */

        if (*ptr == '\0') {
            if (EMsg != NULL)
                sprintf(EMsg, "string is corrupt - early termination");

            _MXMLDestroyE(EP);

            return (FAILURE);
        }

        index = 0;

        while ((*ptr != '"') || ((ptr > XMLString) && (*(ptr - 1) == '\\'))) {
            if (((*ptr == '&') && (*(ptr + 1) == 'l') && (*(ptr + 2) == 't') &&
                 (*(ptr + 3) == ';'))) {
                tmpVLine[index++] = '<';

                ptr += 4;
            } else if (((*ptr == '&') && (*(ptr + 1) == 'g') &&
                        (*(ptr + 2) == 't') && (*(ptr + 3) == ';'))) {
                tmpVLine[index++] = '>';

                ptr += 4;
            } else {
                tmpVLine[index++] = *(ptr++);
            }

            if ((index >= MMAX_XBUFFER) || (*ptr == '\0')) {
                _MXMLDestroyE(EP);

                /* locate tail */

                if (Tail != NULL) *Tail = ptr + strlen(ptr);

                if (EMsg != NULL)
                    sprintf(EMsg, "attribute name is too long - %.10s",
                            tmpVLine);

                return (FAILURE);
            }
        }

        tmpVLine[index] = '\0';

        _MXMLSetAttr(E, tmpNLine, (void *)tmpVLine, mdfString);

        ptr++; /* ignore '"' */

        while (*ptr == ' ') ptr++;

        if ((ptr[0] == '/') && (ptr[1] == '>')) {
            /* element terminator reached */

            ptr += 2; /* skip '/>' */

            if (Tail != NULL) *Tail = ptr;

            return (SUCCESS);
        }
    } /* END while (*ptr != '>') */

    ptr++; /* ignore '>' */

    /* skip whitespace */

    while (isspace(*ptr)) ptr++;

    /* NOTE:  value can occur before, after, or spread amongst children */

    /* extract value */

    if (*ptr != '<') {
        char *ptr2;

        index = 0;

        if (!strncmp(ptr, CRYPTHEAD, strlen(CRYPTHEAD))) {
            char *tail;
            int len;

            mxml_t *C;

            char *tmpBuf;

            /* compressed data detected, '<' symbol guaranteed to not be part of
             * string */

            tail = strchr(ptr, '<');

            /* determine size of value */

            len = tail - ptr;

            /* uncompress data */

            tmpBuf = NULL;

            /* NOTE:  CRYPTHEAD header indicates encryption/compression */

            _MSecDecompress((unsigned char *)ptr, (unsigned int)len, NULL,
                           (unsigned int)0, (unsigned char **)&tmpBuf, NULL);

            /* process expanded buffer (guaranteed to be a single element */

            if ((_MXMLFromString(&C, tmpBuf, NULL, EMsg) == FAILURE) ||
                (_MXMLAddE(E, C) == FAILURE)) {
                _MXMLDestroyE(EP);

                if ((EMsg != NULL) && (EMsg[0] == '\0')) {
                    strcpy(EMsg, "cannot add child element");
                }

                return (FAILURE);
            }

            tmpBuf = NULL;

            /* move pointer to end of compressed data */

            ptr = tail;
        } /* END if (!strncmp(ptr,CRYPTHEAD,strlen(CRYPTHEAD))) */

        while (*ptr != '<') {
            tmpVLine[index++] = *(ptr++);

            if (index >= MMAX_XBUFFER) break;
        }

        tmpVLine[index] = '\0';

        E->Val = strdup(tmpVLine);

        /* restore '<' symbols */

        for (ptr2 = strchr(E->Val, (char)14); ptr2 != NULL;
             ptr2 = strchr(ptr2, (char)14))
            *ptr2 = '<';
    } /* END if (*ptr != '<') */

    /* extract children */

    while (ptr[1] != '/') {
        mxml_t *C;

        C = NULL;

        if (DoAppend == TRUE) {
            char *ptr2;
            char tmpCName[MMAX_NAME];

            int index;

            /* FORMAT:  <NAME>... */

            /* locate name */

            ptr2 = ptr + 1; /* ignore '<' */

            index = 0;

            while ((*ptr2 != ' ') && (*ptr2 != '>')) {
                if ((ptr2[0] == '/') && (ptr2[1] == '>')) {
                    break;
                }

                tmpCName[index++] = *(ptr2++);

                if ((index >= MMAX_LINE) || (ptr2[0] == '\0')) {
                    if (EMsg != NULL)
                        sprintf(EMsg, "element name is too long - %.10s",
                                tmpCName);

                    _MXMLDestroyE(EP);

                    return (FAILURE);
                }
            }

            tmpCName[index] = '\0';

            _MXMLGetChild(E, tmpCName, NULL, &C);
        }

        if ((_MXMLFromString(&C, ptr, &tail, EMsg) == FAILURE) ||
            (_MXMLAddE(E, C) == FAILURE)) {
            break;
        }

        ptr = tail;

        if ((ptr == NULL) || (ptr[0] == '\0')) {
            /* XML is corrupt */

            if (Tail != NULL) *Tail = ptr;

            if ((EMsg != NULL) && (EMsg[0] == '\0'))
                strcpy(EMsg, "cannot extract child");

            _MXMLDestroyE(EP);

            return (FAILURE);
        }
    } /* END while (ptr[1] != '/') */

    /* ignore whitespace */

    while (isspace(*ptr)) ptr++;

    /* value may follow children */

    if (E->Val == NULL) {
        if (!strncmp(ptr, CRYPTHEAD, strlen(CRYPTHEAD))) {
            char *tail;
            int len;

            mxml_t *C;

            char *tmpBuf;

            /* compressed data detected, '<' symbol guaranteed to not be part of
             * string */

            tail = strchr(ptr, '<');

            /* determine size of value */

            len = tail - ptr;

            /* uncompress data */

            tmpBuf = NULL;

            /* NOTE:  CRYPTHEAD header indicates encryption/compression */

            _MSecDecompress((unsigned char *)ptr, (unsigned int)len, NULL,
                           (unsigned int)0, (unsigned char **)&tmpBuf, NULL);

            /* process expanded buffer (guaranteed to be a single element */

            if ((_MXMLFromString(&C, tmpBuf, NULL, EMsg) == FAILURE) ||
                (_MXMLAddE(E, C) == FAILURE)) {
                _MXMLDestroyE(EP);

                if ((EMsg != NULL) && (EMsg[0] == '\0')) {
                    strcpy(EMsg, "cannot add child element");
                }

                return (FAILURE);
            }

            tmpBuf = NULL;

            /* move pointer to end of compressed data */

            ptr = tail;
        } /* END if (!strncmp(ptr,CRYPTHEAD,strlen(CRYPTHEAD))) */

        index = 0;

        while (*ptr != '<') {
            tmpVLine[index++] = *(ptr++);

            if ((index >= MMAX_XBUFFER) || (*ptr == '\0')) {
                if (EMsg != NULL)
                    sprintf(EMsg, "cannot load value line - %.10s (%s)",
                            tmpVLine,
                            (index >= MMAX_XBUFFER) ? "too long" : "corrupt");

                _MXMLDestroyE(EP);

                return (FAILURE);
            }

            if ((*ptr == '/') && (ptr[1] == '>')) break;
        }

        tmpVLine[index] = '\0';

        if (index > 0) E->Val = strdup(tmpVLine);
    } /* END if (tmpVLine[0] == '\0') */

    /* process tail */

    if (*ptr == '/') {
        /* process '/>' */

        ptr++; /* ignore '/' */
    } else if (*ptr != '\0') {
        /* NOTE: corrupt XML string may move ptr beyond string terminator */

        ptr++; /* ignore '<' */

        ptr++; /* ignore '/' */

        ptr += strlen(E->Name);
    }

    if (*ptr == '\0') {
        if (EMsg != NULL) sprintf(EMsg, "xml tail is corrupt");

        _MXMLDestroyE(EP);

        return (FAILURE);
    }

    ptr++; /* ignore '>' */

    if (Tail != NULL) *Tail = ptr;

    return (SUCCESS);
} /* END _MXMLFromString() */

int _MUISCreateFrame(

    msocket_t *S,       /* I (modified) */
    mbool_t DoCompress, /* I */
    mbool_t IsResponse) /* I */

{
    const char *FName = "_MUISCreateFrame";

    if (S == NULL) {
        return (FAILURE);
    }

    switch (S->WireProtocol) {
        case mwpS32:

        {
            mxml_t *E = NULL;
            mxml_t *tE;
            mxml_t *BE;
            mxml_t *RE;
            mxml_t *SE = NULL;

            char Signature[MMAX_LINE];
            char Digest[MMAX_LINE];

            int BufSize;

            char *tmpBuf = NULL;

            /* create message framing */

            if (S->SE != NULL) {
                /* frame previously created */

                return (SUCCESS);
            }

            _MXMLCreateE(&E, _MSON[msonEnvelope]);

            _MXMLSetAttr(E, "count", (void *)"1", mdfString);
            _MXMLSetAttr(E, "name", (void *)MSCHED_SNAME, mdfString);

            if ((S->IsNonBlocking == TRUE) && (IsResponse == FALSE)) {
                _MXMLSetAttr(E, "type", (void *)"nonblocking", mdfString);
            }

            _MXMLSetAttr(E, "version", (void *)MOAB_VERSION, mdfString);
            _MXMLSetAttr(E, "component", (void *)"ClusterScheduler", mdfString);

            if (S->CSAlgo != mcsaNONE) {
                SE = NULL;

                _MXMLCreateE(&SE, "Signature");
                _MXMLAddE(E, SE);

                /* NOTE:  signature currently not populated */
            }

            BE = NULL;

            _MXMLCreateE(&BE, _MSON[msonBody]);

            /* NOTE:  actor not needed in body for S3 3.0 */

            if (S->ClientName != NULL)
                _MXMLSetAttr(BE, "actor", (void *)S->ClientName, mdfString);
            else
                _MXMLSetAttr(BE, "actor", (void *)_MUUIDToName(_MOSGetEUID()),
                            mdfString);

            _MXMLAddE(E, BE);

            S->SE = (void *)E;

            if (IsResponse == TRUE) {
                tE = NULL;

                _MXMLCreateE(&tE, _MSON[msonResponse]);

                _MXMLAddE(BE, tE);

                /* add status information */

                /* get first 'Body' child (ignore signature) */

                if ((_MXMLGetChild((mxml_t *)S->SE, _MSON[msonBody], NULL, &BE) ==
                     FAILURE) ||
                    (_MXMLGetChild(BE, NULL, NULL, &RE) == FAILURE)) {
                    return (FAILURE);
                }

                if (_MS3SetStatus(RE, (S->StatusCode == 0) ? (char *)"Success"
                                                          : (char *)"Failure",
                                 (enum MSFC)(S->StatusCode % 1000),
                                 S->SMsg) == FAILURE) {
                    return (FAILURE);
                }

                /* add data */

                if (S->SDE == NULL) {
                    _MXMLCreateE((mxml_t **)&S->SDE, _MSON[msonData]);

                    if ((DoCompress == TRUE) && (S->SPtr != NULL)) {
                        if (_MSecCompress((unsigned char *)S->SPtr, /* I/O */
                                         strlen(S->SPtr), NULL,
                                         MCONST_CKEY) == FAILURE) {
                            MDB(0, fSOCK)
                            MLog("WARNING:  cannot compress data in %s\n",
                                 FName);

                            return (FAILURE);
                        }
                    } else {
                        /* NOTE:  handle proper freeing of send data */

                        ((mxml_t *)S->SDE)->Val = S->SPtr;

                        /* should S->SPtr be set to null?  should S->SBuffer be
                         * freed? */
                    } /* END if ((DoCompress == TRUE) && ...) */
                }

                _MXMLAddE(RE, (mxml_t *)S->SDE);

                S->SDE = NULL;
            } /* END if (IsResponse == TRUE) */
            else {
                /* message is request */

                /* add data */

                if (S->SDE != NULL) {
                    RE = (mxml_t *)S->SDE;

                    S->SDE = NULL;
                } else {
                    RE = NULL;

                    _MXMLFromString(&RE, S->SBuffer, NULL, NULL);
                }

                if (S->ClientName != NULL)
                    _MXMLSetAttr(RE, "actor", (void *)S->ClientName, mdfString);
                else
                    _MXMLSetAttr(RE, "actor", (void *)_MUUIDToName(_MOSGetEUID()),
                                mdfString);

                _MXMLAddE(BE, RE);
            } /* END else (IsResponse == TRUE) */

            /* NOTE:  do not check exit status, only checksum first 64K */

            _MXMLToXString(BE, &tmpBuf, &BufSize, MMAX_BUFFER << 5, NULL, TRUE);

            if (S->DoEncrypt == TRUE) {
                /* encrypt body element */

                /* NYI */
            }

            /* generate checksum on first 64k body element */

            if (SE != NULL) {
                _MSecGetChecksum(tmpBuf, strlen(tmpBuf), Signature, Digest,
                                mcsaHMAC64, S->CSKey);

                tE = NULL;
                _MXMLCreateE(&tE, "DigestValue");

                _MXMLSetVal(tE, (void *)Digest, mdfString);

                _MXMLAddE(SE, tE);

                tE = NULL;
                _MXMLCreateE(&tE, "SignatureValue");

                _MXMLSetVal(tE, (void *)Signature, mdfString);

                _MXMLAddE(SE, tE);
            } /* END if (SE != NULL) */

            _MUFree(&tmpBuf);
        } /* END BLOCK */

        break;

        case mwpXML:

            /* NO-OP */

            break;

        default:

            /* NO-OP */

            break;
    } /* END switch(S->WireProtocol) */

    return (SUCCESS);
} /* END _MUISCreateFrame() */

int _MUSNInit(

    char **BPtr,    /* O (modified) */
    int *BSpace,    /* O */
    char *SrcBuf,   /* I */
    int SrcBufSize) /* I */

{
    if ((BPtr == NULL) || (BSpace == NULL) || (SrcBuf == NULL)) {
        return (FAILURE);
    }

    *BPtr = SrcBuf;

    *BSpace = SrcBufSize;

    (*BPtr)[0] = '\0';

    return (SUCCESS);
} /* END _MUSNInit() */

int _MUSNPrintF(

    char **BPtr, int *BSpace, char *Format, ...)

{
    int len;
    const char *FName = "_MUSNPrintF";
    va_list Args;

    if ((BPtr == NULL) || (BSpace == NULL) || (Format == NULL) ||
        (*BSpace <= 0)) {
        DBG(4, fCORE) dPrint("ALERT:    Memory Error in %s\n", FName);
        return (FAILURE);
    }

    va_start(Args, Format);

    len = vsnprintf(*BPtr, *BSpace, Format, Args);

    va_end(Args);

    if (len <= 0) {
        DBG(4, fCORE) dPrint("ALERT:    vsnprintf Error in %s\n", FName);
        return (FAILURE);
    }

#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1
    /* XXX: The following is true for glibc > 2.1 */

    /* if vsnprintf returns the same value as size or greater */
    /* then the output is truncated. see man vsnprintf. */
    if (len == *BSpace || len > *BSpace) {
        DBG(1, fCORE)
        dPrint("ALERT:    Possible vsnprintf truncation in %s\n", FName);
        return (FAILURE);
    }
#endif

    *BPtr += len;
    *BSpace -= len;

    return (SUCCESS);
} /* END MUSNPrintf() */

int _MXMLGetAttr(

    mxml_t *E,   /* I */
    char *AName, /* I/O */
    int *ATok,   /* I (optional) */
    char *AVal,  /* O (optional) */
    int VSize)   /* I */

{
    /* NOTE:  set AName to empty string to get Name */

    int aindex;
    int astart;

    int EVSize;

    if (AVal != NULL) AVal[0] = '\0';

    if (E == NULL) {
        return (FAILURE);
    }

    EVSize = (VSize > 0) ? VSize : MMAX_LINE;

    if (ATok != NULL)
        astart = *ATok;
    else
        astart = -1;

    for (aindex = astart + 1; aindex < E->ACount; aindex++) {
        if ((AName == NULL) || (AName[0] == '\0') ||
            !strcmp(AName, E->AName[aindex])) {
            if ((AName != NULL) && (AName[0] == '\0')) {
                strncpy(AName, E->AName[aindex], MMAX_NAME);
                AName[MMAX_NAME - 1] = '\0';
            }

            if (AVal != NULL) {
                strncpy(AVal, E->AVal[aindex], EVSize);
                AVal[EVSize - 1] = '\0';
            }

            if (ATok != NULL) *ATok = aindex;

            return (SUCCESS);
        }
    } /* END for (aindex) */

    return (FAILURE);
} /* END _MXMLGetAttr() */

int _MSURecvPacket(

    int sd,                   /* I */
    char **BufP,              /* O (alloc if NULL ptr passed) */
    long BufSize,             /* I (optional) */
    char *Pattern,            /* I (optional) */
    long TimeOut,             /* I (in us) */
    enum MStatusCodeEnum *SC) /* O (optional) */

{
    long count;

    int rc;
    int len;

    int MaxBuf;

    char *ptr;

#ifndef __MPROD
    const char *FName = "_MSURecvPacket";

    MDB(3, fSOCK)
    MLog("%s(%d,BufP,%ld,%s,%ld,SC)\n", FName, sd, BufSize,
         (Pattern != NULL) ? Pattern : "NULL", TimeOut);

#endif /* !__MPROD */

    if (BufP == NULL) {
        return (FAILURE);
    }

    count = 0;

    MaxBuf = (BufSize > 0) ? BufSize : (MMAX_BUFFER << 4);

    if ((BufSize == 0) || (*BufP == NULL) || (Pattern != NULL)) {
        char *ptr;
        int ReadSize;

        if (*BufP == NULL) {
            if ((*(char **)BufP = (char *)calloc(1, MaxBuf)) == NULL) {
                /* cannot allocate memory */
                return (FAILURE);
            }
        }

        if (Pattern != NULL) {
            ReadSize = 1;

            len = strlen(Pattern);
        } else {
            ReadSize = MaxBuf;

            len = 0;
        }

        ptr = *(char **)BufP;

        while (TRUE) {
            if (_MSUSelectRead(sd, TimeOut) == FAILURE) {
                MDB(2, fSOCK)
                MLog(
                    "WARNING:  cannot receive message within %1.6lf second "
                    "timeout (aborting)\n",
                    (double)TimeOut / 1000000);

                if (SC != NULL) {
                    *SC = mscNoData;
                }
                return (FAILURE);
            }

            rc = recv(sd, &ptr[count], ReadSize, SOCKETFLAGS);

            if (rc == 0) {
/* select indicated data was available, but recv() returned nothing */

/* sleep and try again */

#ifdef __M32COMPAT
                _MUSleep(TimeOut);
#else  /* __M32COMPAT */
                _MUSleep(TimeOut, FALSE);
#endif /* __M32COMPAT */

                rc = recv(sd, &ptr[count], ReadSize, SOCKETFLAGS);

                if (rc == 0) {
                    MDB(2, fSOCK)
                    MLog(
                        "WARNING:  cannot receive message within %1.6lf second "
                        "timeout (no data/aborting)\n",
                        (double)TimeOut / 1000000);
                    return (FAILURE);
                }
            }

            if (rc == -1) {
                if (errno == EAGAIN) continue;

                /* socket recv call failed */

                break;
            }

            if (count >= MaxBuf) {
                /* buffer size reached */

                break;
            }

            /* NOTE:  precludes binary data */

            if ((count > 0) && (ptr[count - 1] == '\0')) {
                /* end of string located */

                break;
            }

            count += rc;

            if ((Pattern != NULL) && (count >= len) &&
                (_MUStrNCmpCI(Pattern, ptr + count - len, len) == SUCCESS)) {
                /* termination pattern located */

                break;
            }
        } /* END while (TRUE) */

        ptr[count] = '\0';
    } /* END if ((BufSize == 0) || ... ) */
    else {
        time_t Start;
        time_t Now;

        ptr = *BufP;

        time(&Start);
        Now = Start;

        while (count < BufSize) {
            if (_MSUSelectRead(sd, TimeOut) == FAILURE) {
                MDB(2, fSOCK)
                MLog(
                    "WARNING:  cannot receive message within %1.6lf second "
                    "timeout (aborting)\n",
                    (double)TimeOut / 1000000);

                if (SC != NULL) {
                    *SC = mscNoData;
                }
                return (FAILURE);
            }

            rc = recv(sd, (ptr + count), (BufSize - count), SOCKETFLAGS);
            printf("ptr + count:%s\n",ptr + count);

            if (rc > 0) {
                count += rc;
                continue;
            }

            time(&Now);

            if (((long)Now - (long)Start) >= (TimeOut / 1000000)) {
                MDB(2, fSOCK)
                MLog(
                    "WARNING:  cannot receive message within %1.6lf second "
                    "timeout (aborting)\n",
                    (double)TimeOut / 1000000);
                return (FAILURE);
            }

            if (rc < 0) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                    /* continue if packet partially read */

                    MDB(0, fSOCK)
                    MLog(
                        "ERROR:    socket blocked (select() indicated socket "
                        "was available)\n");

                    continue;
                }

                if ((errno == ECONNRESET) || (errno == EINTR)) {
                    MDB(0, fSOCK)
                    MLog("INFO:     client has disconnected, errno: %d (%s)\n",
                         errno, strerror(errno));
                    return (FAILURE);
                }

                MDB(6, fSOCK)
                MLog("WARNING:  cannot read client socket, errno: %d (%s)\n",
                     errno, strerror(errno));
                return (FAILURE);
            }

            if ((rc == 0) && (count == 0)) {
                /* fail if no packet detected */
                return (FAILURE);
            }

        } /* END while (count < BufSize) */

        MDB(6, fSOCK)
        MLog("INFO:     %ld of %ld bytes read from sd %d\n", count, BufSize,
             sd);
    } /* END else (BufSize == 0) */

    MDB(8, fSOCK)
    MLog("INFO:     message '%s' read\n", _MUPrintBuffer(*BufP, count));

    /* NOTE:  return SUCCESS if buffer full even if entire message not loaded */

    return (SUCCESS);
} /* END _MSURecvPacket() */

int _MSecGetChecksum(

    char *Buf,                   /* I */
    int BufSize,                 /* I */
    char *Checksum,              /* O */
    char *Digest,                /* O (optional) */
    enum MChecksumAlgoEnum Algo, /* I */
    char *CSKeyString)           /* I */

{
#ifndef __MPROD
    const char *FName = "_MSecGetChecksum";

    MDB(5, fSTRUCT)
    MLog("%s(Buf,%d,Checksum,%s,CSKey)\n", FName, BufSize, _MCSAlgoType[Algo]);
#endif /* !__MPROD */

    if ((Buf == NULL) || (Checksum == NULL) || (CSKeyString == NULL)) {
        return (FAILURE);
    }

    switch (Algo) {
        case mcsaHMAC:

        {
            _MSecHMACGetDigest((unsigned char *)CSKeyString, strlen(CSKeyString),
                              (unsigned char *)Buf, strlen(Buf), Checksum,
                              SHA_DIGESTSIZE, NULL, FALSE, FALSE);
        } /* END BLOCK */

        break;

        case mcsaHMAC64:

        {
            _MSecHMACGetDigest((unsigned char *)CSKeyString, strlen(CSKeyString),
                              (unsigned char *)Buf, strlen(Buf), Checksum,
                              SHA_DIGESTSIZE, Digest, TRUE, TRUE);
        } /* END BLOCK */

        break;

        case mcsaMD5:

        {
            _MSecMD5GetDigest(CSKeyString, strlen(CSKeyString), Buf, strlen(Buf),
                             Checksum, MMD5_DIGESTSIZE);
        } /* END BLOCK */

        break;

        case mcsaRemote:

        {
            char CmdLine[MMAX_LINE];

            sprintf(CmdLine, "mauth \"%s\"", Buf);

#ifndef __M32COMPAT
            if (_MUReadPipe(CmdLine, NULL, Checksum, MMAX_LINE) == FAILURE) {
                return (FAILURE);
            }
#endif    /* __M32COMPAT */
        } /* END BLOCK */

        break;

        case mcsaDES:
        default:

        {
            unsigned int crc;
            unsigned int lword;
            unsigned int rword;

            int index;

            unsigned int SKey;

            SKey = (unsigned int)strtoul(CSKeyString, NULL, 0);

            crc = 0;

            for (index = 0; index < BufSize; index++) {
                crc =
                    (unsigned int)_MSecDoCRC((unsigned short)crc, Buf[index]);
            }

            lword = crc;
            rword = SKey;

            _MSecPSDES(&lword, &rword);

            Checksum[0] = '\0';

            sprintf(Checksum, "%08x%08x", (int)lword, (int)rword);
        } /* END BLOCK */

        break;
    } /* END switch(Algo) */

    /* NOTE:  enable logging only for unit testing.  MUST DISABLE FOR PRODUCTION
     */

    /*
       MDB(8,fSTRUCT) MLog("INFO:     checksum '%s' calculated for %d byte
       buffer '%s' using seed %x\n",
         Checksum,
         BufSize,
         Buf,
         SKey);
    */

    return (SUCCESS);
} /* END _MSecGetChecksum() */

char *_MUPrintBuffer(

    char *Buf,   /* I */
    int BufSize) /* I */

{
    int bindex;
    int lindex;

    int bcount;

    static char Line[MAX_MLINE + 1];

    lindex = 0;

    bcount = (MAX_MLINE < BufSize) ? MAX_MLINE : BufSize;

    for (bindex = 0; bindex < bcount; bindex++) {
        if (!isprint(Buf[bindex]) && !isspace(Buf[bindex]))
            break;
        else
            Line[lindex++] = Buf[bindex];
    } /* END for (bindex) */

    Line[lindex] = '\0';

    return (Line);
} /* END _MUPrintBuffer() */

char *_MUStrStr(

    char *String,              /* I */
    char *Pattern,             /* I */
    int TIndex,                /* I (optional) */
    mbool_t IsCaseInsensitive, /* I */
    mbool_t GetLast)           /* I */

{
    int sindex;
    int pindex;

    int slen;
    int plen;

    char sc;
    char pc;

    if ((String == NULL) || (Pattern == NULL)) {
        return (NULL);
    }

    /* TIndex is starting tail index when GetLast is true */

    slen = strlen(String);

    if (GetLast == FALSE) {
        for (sindex = 0; sindex < slen; sindex++) {
            for (pindex = 0; Pattern[pindex] != '\0'; pindex++) {
                sc = String[sindex + pindex];
                pc = Pattern[pindex];

                if ((IsCaseInsensitive == TRUE) && (isalpha(sc))) {
                    if (tolower(sc) != tolower(pc)) break;
                } else if (sc != pc) {
                    break;
                }
            } /* END for (pindex) */

            if (Pattern[pindex] == '\0') {
                return (&String[sindex]);
            }
        } /* END for (sindex) */
    } else {
        /* GetLast == TRUE */

        plen = strlen(Pattern);

        sindex = (TIndex > 0) ? TIndex - plen : slen - plen;

        for (; sindex >= 0; sindex--) {
            for (pindex = 0; Pattern[pindex] != '\0'; pindex++) {
                sc = String[sindex + pindex];
                pc = Pattern[pindex];

                if ((IsCaseInsensitive == TRUE) && (isalpha(sc))) {
                    if (tolower(sc) != tolower(pc)) break;
                } else if (sc != pc) {
                    break;
                }
            } /* END for (pindex) */

            if (Pattern[pindex] == '\0') {
                return (&String[sindex]);
            }
        } /* END for (sindex) */
    }     /* END else (GetLast == TRUE) */

    return (NULL);
} /* END _MUStrStr() */

int _MSecPSDES(

    unsigned int *lword, unsigned int *irword)

{
    int index;

    unsigned int ia;
    unsigned int ib;
    unsigned int iswap;
    unsigned int itmph;
    unsigned int itmpl;

    static unsigned int c1[MAX_MDESCKSUM_ITERATION] = {
        (unsigned int)0xcba4e531, (unsigned int)0x537158eb,
        (unsigned int)0x145cdc3c, (unsigned int)0x0d3fdeb2};

    static unsigned int c2[MAX_MDESCKSUM_ITERATION] = {
        (unsigned int)0x12be4590, (unsigned int)0xab54ce58,
        (unsigned int)0x6954c7a6, (unsigned int)0x15a2ca46};

    itmph = 0;
    itmpl = 0;

    for (index = 0; index < MAX_MDESCKSUM_ITERATION; index++) {
        iswap = *irword;

        ia = iswap ^ c1[index];

        itmpl = ia & 0xffff;
        itmph = ia >> 16;

        ib = (itmpl * itmpl) + ~(itmph * itmph);
        ia = (ib >> 16) | ((ib & 0xffff) << 16);

        *irword = (*lword) ^ ((ia ^ c2[index]) + (itmpl * itmph));

        *lword = iswap;
    }

    return (SUCCESS);
} /* END _MSecPSDES() */

unsigned short _MSecDoCRC(

    unsigned short crc, unsigned char val)

{
    int index;

    unsigned int ans;

    ans = (crc ^ val << 8);

    for (index = 0; index < 8; index++) {
        if (ans & 0x8000)
            ans = (ans << 1) ^ 4129;
        else
            ans <<= 1;
    } /* END for (index) */

    return ((unsigned short)ans);
} /* END _MSecDoCRC() */

int _MUReadPipe(

    char *Command, /* I */
    char *Buffer,  /* O */
    int BufSize)   /* I */

{
    FILE *fp;
    int rc;

    const char *FName = "_MUReadPipe";

    DBG(5, fSOCK)
    dPrint("%s(%s,Buffer,%d)\n", FName, (Command != NULL) ? Command : "NULL",
           BufSize);

    if ((Command == NULL) || (Buffer == NULL)) {
        return (FAILURE);
    }

    if ((fp = popen(Command, "r")) == NULL) {
        DBG(0, fSOCK)
        dPrint("ERROR:    cannot open pipe on command '%s', errno: %d (%s)\n",
               Command, errno, strerror(errno));

        return (FAILURE);
    }

    if ((rc = fread(Buffer, 1, BufSize, fp)) == -1) {
        DBG(0, fSOCK)
        dPrint("ERROR:    cannot read pipe on command '%s', errno: %d (%s)\n",
               Command, errno, strerror(errno));

        pclose(fp);

        return (FAILURE);
    }

    /* terminate buffer */

    Buffer[rc] = '\0';

    DBG(5, fSOCK) dPrint("INFO:     pipe(%s) -> '%s'\n", Command, Buffer);

    pclose(fp);

    return (SUCCESS);
} /* END _MUReadPipe() */

int _MSecMD5GetDigest(

    char *CSKeyString,  /* I */
    int CSKeyLen,       /* I */
    char *DataString,   /* I */
    int DataLen,        /* I */
    char *CSString,     /* O */
    int MaxCSStringLen) /* O */

{
    int index;

    char tmpBuf[MMAX_BUFFER];

    char MD5Sum[MMD5_DIGESTSIZE + 2];

    char *ptr = NULL;

#ifndef __MPROD
    const char *FName = "_MSecMD5GetDigest";

    MDB(2, fSTRUCT)
    MLog("%s(CSKeyString,CSKeyLen,DString,DLen,CSString,MaxCSLen)\n", FName);
#endif /* !__MPROD */

    if ((CSKeyString == NULL) || (DataString == NULL) || (CSString == NULL)) {
        return (FAILURE);
    }

    /* NOTE:  MD5(SrcBuf,strlen(SrcBuf),tmpLine); */

    CSString[0] = '\0';

    sprintf(tmpBuf, "%s%s", DataString, CSKeyString);

#ifdef __MMD5
    ptr = MD5((unsigned char *)tmpBuf, (unsigned long)strlen(tmpBuf), MD5Sum);
#endif /* __MMD5 */

    if (ptr == NULL) {
        return (FAILURE);
    }

    for (index = 0; index < MMD5_DIGESTSIZE; index++) {
        sprintf(temp_str, "%02x", (unsigned char)MD5Sum[index]);
        strcat(CSString, temp_str);
    } /* END for (index) */

    strcat(CSString, "\n");

    return (SUCCESS);
} /* END _MSecMD5GetDigest() */

int _MSUSendPacket(

    int sd,                   /* I */
    char *Buf,                /* I */
    long BufSize,             /* I */
    long TimeOut,             /* I */
    enum MStatusCodeEnum *SC) /* O (optional) */

{
    int rc;
    int count;

/*  extern int errno; */

#ifndef __MPROD
    const char *FName = "_MSUSendPacket";

    MDB(5, fSOCK) MLog("%s(%d,Buf,%ld,%ld,SC)\n", FName, sd, BufSize, TimeOut);
#endif /* !__MPROD */

    if (SC != NULL) *SC = mscNoError;

    MDB(9, fSOCK)
    MLog("INFO:     sending packet '%s'\n", _MUPrintBuffer(Buf, BufSize));

    count = 0;

    TimeOut = MAX(50000, TimeOut);

    while (count < BufSize) {
        if (_MSUSelectWrite(sd, TimeOut) == FAILURE) {
            MDB(2, fSOCK)
            MLog(
                "WARNING:  cannot send message within %1.6f second timeout "
                "(aborting)\n",
                (double)TimeOut / 1000000);

            return (FAILURE);
        }

        printf("Buf + count:%s\n",Buf + count);
        if ((rc = send(sd, (Buf + count), (BufSize - count), SOCKETFLAGS)) <
            0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                /* should never occur with select */

                MDB(0, fSOCK)
                MLog(
                    "ERROR:    socket blocked (select() indicated socket was "
                    "available)\n");

                continue;
            }

            MDB(1, fSOCK)
            MLog("WARNING:  cannot send packet, errno: %d (%s)\n", errno,
                 strerror(errno));

            rc = errno;

            if (SC != NULL) {
                if (errno == ECONNREFUSED) {
                    /* no service available at remote port */

                    *SC = mscNoEnt;
                } else {
                    /* default connection failure */

                    *SC = mscNoEnt;
                }
            }

            return (FAILURE);
        } else if (rc == 0) {
            /* should never occur with select */

            MDB(1, fSOCK)
            MLog(
                "ERROR:    no data sent (select() indicated socket was "
                "available)\n");

            continue;
        } else {
            MDB(2, fSOCK) {
                MDB(4, fSOCK) {
                    MDB(4, fSOCK)
                    MLog("INFO:     packet sent (%d bytes of %ld)\n", rc,
                         BufSize);
                }
                else {
                    if (rc != 0) {
                        MDB(0, fSOCK)
                        MLog("INFO:     packet sent (%d bytes of %ld)\n", rc,
                             BufSize);
                    }
                }
            }
        } /* END else ... */

        if (rc > 0) count += rc;
    } /* END while (count < BufSize) */
    return (SUCCESS);
} /* END _MSUSendPacket() */

int _MUStrToUpper(

    char *String, /* I (modified) */
    char *OBuf,   /* O (optional) */
    int BufSize)  /* I */

{
    int sindex;
    char *ptr;

    if (String == NULL) {
        return (SUCCESS);
    }

    ptr = (OBuf != NULL) ? OBuf : String;

    for (sindex = 0; String[sindex] != '\0'; sindex++) {
        if ((BufSize > 0) && (sindex >= (BufSize - 1))) break;

        ptr[sindex] = toupper(String[sindex]);
    } /* END for (sindex) */

    ptr[sindex] = '\0';

    return (SUCCESS);
} /* END _MUStrToUpper() */

int _MSecGetChecksum2(

    char *Buf1,                  /* I */
    int BufSize1,                /* I */
    char *Buf2,                  /* I */
    int BufSize2,                /* I */
    char *Checksum,              /* O */
    char *Digest,                /* O (optional) */
    enum MChecksumAlgoEnum Algo, /* I */
    char *CSKeyString)           /* I */

{
#ifndef __MPROD
    const char *FName = "_MSecGetChecksum2";

    MDB(5, fSTRUCT)
    MLog("%s(Buf1,%d,Buf2,%d,Checksum,%s,CSKey)\n", FName, BufSize1, BufSize2,
         _MCSAlgoType[Algo]);
#endif /* !__MPROD */

    if ((Buf1 == NULL) || (Buf2 == NULL) || (Checksum == NULL)) {
        return (FAILURE);
    }

    switch (Algo) {
        case mcsaHMAC:

        {
            char *ptr;

            /* NOTE:  merge header and data */

            ptr = (char *)calloc(BufSize1 + BufSize2, 1);

            strcpy(ptr, Buf1);
            strcpy(ptr + BufSize1, Buf2);

            _MSecHMACGetDigest((unsigned char *)CSKeyString, strlen(CSKeyString),
                              (unsigned char *)ptr, BufSize1 + BufSize2,
                              Checksum, SHA_DIGESTSIZE, NULL, FALSE, FALSE);

            free(ptr);
        } /* END BLOCK */

        break;

        case mcsaHMAC64:

        {
            char *ptr;

            /* NOTE:  merge header and data */

            ptr = (char *)calloc(BufSize1 + BufSize2, 1);

            strcpy(ptr, Buf1);
            strcpy(ptr + BufSize1, Buf2);

            _MSecHMACGetDigest((unsigned char *)CSKeyString, strlen(CSKeyString),
                              (unsigned char *)ptr, strlen(ptr), Checksum,
                              SHA_DIGESTSIZE, Digest, TRUE, TRUE);

            free(ptr);
        } /* END BLOCK */

        break;

        case mcsaMD5:

        {
            char *ptr;

            /* NOTE:  merge header and data */

            ptr = (char *)calloc(BufSize1 + BufSize2, 1);

            strcpy(ptr, Buf1);
            strcpy(ptr + BufSize1, Buf2);

            _MSecMD5GetDigest(CSKeyString, strlen(CSKeyString), ptr,
                             BufSize1 + BufSize2, Checksum, MMD5_DIGESTSIZE);

            free(ptr);
        } /* END BLOCK */

        break;

        case mcsaDES:
        default:

        {
            unsigned int crc;
            unsigned int lword;
            unsigned int rword;

            unsigned int Seed;

            int index;

            Seed = (unsigned int)strtoul(CSKeyString, NULL, 0);

            crc = 0;

            for (index = 0; index < BufSize1; index++) {
                crc =
                    (unsigned int)_MSecDoCRC((unsigned short)crc, Buf1[index]);
            } /* END for (index) */

            for (index = 0; index < BufSize2; index++) {
                crc =
                    (unsigned int)_MSecDoCRC((unsigned short)crc, Buf2[index]);
            } /* END for (index) */

            lword = crc;
            rword = Seed;

            _MSecPSDES(&lword, &rword);

            sprintf(Checksum, "%08x%08x", (int)lword, (int)rword);
        } /* END BLOCK */

        break;
    } /* END switch(Algo) */

    return (SUCCESS);
} /* END _MSecGetChecksum2() */

int _MSecDecompress(

    unsigned char *SrcString, /* I */
    unsigned int SrcSize,     /* I */
    unsigned char *SDstBuf,   /* I (optional) */
    unsigned int SDstSize,    /* I (required if DstBuf set) */
    unsigned char **DDstBuf,  /* O (optional) */
    char *EKey) /* I (optional - encryption enabled when present) */

{
    static unsigned char *tmpBuf = NULL;
    static unsigned char *OutBuf = NULL;

    static int OutBufSize = 0;

    unsigned int DstSize;

    unsigned int NewLength;

    unsigned int X = 9;
    unsigned int Y = 0;
    unsigned int Pos;
    unsigned int Size;
    unsigned int K;
    int Command;

    unsigned char *DstString;

    unsigned char Bit = 16;

    if ((SrcString == NULL) || (SrcSize <= 0)) {
        return (FAILURE);
    }

    if ((SDstBuf != NULL) && (SDstSize > 0)) {
        DstString = SDstBuf;
        DstSize = SDstSize;
    } else if (DDstBuf != NULL) {
        if ((OutBuf == NULL) &&
            ((OutBuf = (unsigned char *)calloc(MMAX_BUFFER, 1)) == NULL)) {
            return (FAILURE);
        } else {
            OutBufSize = MMAX_BUFFER;
        }

        *DDstBuf = NULL;

        DstString = OutBuf;
        DstSize = OutBufSize;
    } else {
        return (FAILURE);
    }

    DstString[0] = '\0';

    NewLength = SrcSize;

    if (tmpBuf == NULL) {
        if ((tmpBuf = (unsigned char *)calloc(NewLength, 1)) == NULL) {
            return (FAILURE);
        }
    } else {
        if (sizeof(tmpBuf) < NewLength) {
            if ((tmpBuf = (unsigned char *)realloc(tmpBuf, NewLength)) ==
                NULL) {
                return (FAILURE);
            }
        }
    }

    tmpBuf[0] = '\0';

    _MSecComp64BitToBufDecoding((char *)SrcString, (int)SrcSize, (char *)tmpBuf,
                               (int *)&SrcSize);

    if (EKey != NULL) _MSecEncryption((char *)tmpBuf, EKey, SrcSize);

    Command = (tmpBuf[3] << 8) + tmpBuf[4];

    /*
      if (tmpBuf[0] == FLAG_Copied)
        {
        for (Y = 1; Y < SrcSize; DstString[Y-1] = tmpBuf[Y++]);

        return(SrcSize-1);
        }
    */

    for (; X < SrcSize;) {
        if (Bit == 0) {
            Command = (tmpBuf[X++] << 8);

            Command += tmpBuf[X++];
            Bit = 16;
        }

        if (Command & 0x8000) {
            Pos = (tmpBuf[X++] << 4);
            Pos += (tmpBuf[X] >> 4);

            if (Pos) {
                Size = (tmpBuf[X++] & 0x0f) + 3;

                if ((Y + Size) > DstSize) {
                    /* bounds check */

                    if (DDstBuf != NULL) {
                        DstSize <<= 1;

                        if ((OutBuf = (unsigned char *)realloc(
                                 OutBuf, DstSize)) == NULL) {
                            return (FAILURE);
                        }

                        DstString = OutBuf;
                    } else {
                        return (FAILURE);
                    }
                }

                for (K = 0; K < Size; K++) {
                    DstString[Y + K] = DstString[Y - Pos + K];
                }

                Y += Size;
            } else {
                Size = (tmpBuf[X++] << 8);
                Size += tmpBuf[X++] + 16;

                if ((Y + Size) > DstSize) {
                    /* bounds check */

                    if (DDstBuf != NULL) {
                        DstSize <<= 1;

                        if ((OutBuf = (unsigned char *)realloc(
                                 OutBuf, DstSize)) == NULL) {
                            return (FAILURE);
                        }

                        DstString = OutBuf;
                    } else {
                        return (FAILURE);
                    }
                }

                for (K = 0; K < Size; DstString[Y + K++] = tmpBuf[X])
                    ;

                X++;
                Y += Size;
            }
        } else {
            if (Y >= DstSize) {
                /* bounds check */

                if (DDstBuf != NULL) {
                    DstSize <<= 1;

                    if ((OutBuf = (unsigned char *)realloc(OutBuf, DstSize)) ==
                        NULL) {
                        return (FAILURE);
                    }

                    DstString = OutBuf;
                } else {
                    return (FAILURE);
                }
            }

            DstString[Y++] = tmpBuf[X++];
        } /* END else () */

        Command <<= 1;
        Bit--;
    } /* END for() */

    /* terminate buffer */

    if (Y < DstSize) OutBuf[Y] = '\0';

    if (DDstBuf != NULL) {
        *DDstBuf = OutBuf;
    }

    return (SUCCESS);
} /* END _MSecDecompress() */

int _MXMLAddE(

    mxml_t *E, /* I (modified) */
    mxml_t *C) /* I (required) */

{
    if ((E == NULL) || (C == NULL)) {
        return (FAILURE);
    }

    if (E->CCount >= E->CSize) {
        if (E->C == NULL) {
            E->C = (mxml_t **)calloc(1, sizeof(mxml_t *) * MDEF_XMLICCOUNT);

            E->CSize = MDEF_XMLICCOUNT;
        } else {
            E->C = (mxml_t **)realloc(
                E->C, sizeof(mxml_t *) * MAX(16, E->CSize << 1));

            E->CSize <<= 1;
        }

        if (E->C == NULL) {
            /* cannot alloc memory */

            return (FAILURE);
        } /* END if (E->C == NULL) */
    }     /* END if (E->CCount >= E->CSize) */

    E->C[E->CCount] = C;

    E->CCount++;

    return (SUCCESS);
} /* END _MXMLAddE() */

int _MS3SetStatus(

    mxml_t *E,      /* I (modified) */
    char *Value,    /* I */
    enum MSFC Code, /* I */
    char *Msg)      /* I (optional) */

{
    mxml_t *SE;
    mxml_t *CE;

    char tmpLine[MMAX_LINE];

#ifndef MOPT

    if ((E == NULL) || (Value == NULL)) {
        return (FAILURE);
    }

#endif /* MOPT */

    SE = NULL;

    _MXMLCreateE(&SE, "Status");

    _MXMLSetVal(SE, (void *)Value, mdfString);

    _MXMLAddE(E, SE);

    CE = NULL;

    _MXMLCreateE(&CE, "Code");

    sprintf(tmpLine, "%03d", Code % 1000);

    _MXMLSetVal(CE, (void *)tmpLine, mdfString);

    _MXMLAddE(SE, CE);

    if (Msg != NULL) {
        CE = NULL;
        _MXMLCreateE(&CE, "Message");

        _MXMLSetVal(CE, (void *)Msg, mdfString);

        _MXMLAddE(SE, CE);
    } /* END if (Msg != NULL) */

    return (SUCCESS);
} /* END _MS3SetStatus() */

int _MXMLToXString(

    mxml_t *E,          /* I */
    char **Buf,         /* O (populated/modified) */
    int *BufSize,       /* I/O */
    int MaxBufSize,     /* I */
    char **Tail,        /* O */
    mbool_t NoCompress) /* I */

{
    int NewSize;

    /* NOTE:  MXMLToString() only fails due to lack of space */

    if ((E == NULL) || (Buf == NULL)) {
        return (FAILURE);
    }

    /* allocate initial memory if required */

    if (*Buf == NULL) {
        NewSize = MMAX_BUFFER;
        if ((*Buf = (char *)calloc(NewSize, 1)) == NULL) {
            /* cannot allocate buffer */

            return (FAILURE);
        }

        if (BufSize != NULL) *BufSize = MMAX_BUFFER;
    } else {
        if (BufSize != NULL) {
            NewSize = *BufSize;
        } else {
            return (FAILURE);
        }
    }

    while (_MXMLToString(E, *Buf, NewSize, Tail, NoCompress) == FAILURE) {
        if (NewSize >= MaxBufSize) {
            return (FAILURE);
        }

        NewSize = MIN(NewSize << 1, MaxBufSize);

        if ((*Buf = (char *)realloc(*Buf, NewSize)) == NULL) {
            /* cannot allocate buffer */

            return (FAILURE);
        }

        if (BufSize != NULL) *BufSize = NewSize;
    } /* END while (MXMLToString() == FAILURE) */

    return (SUCCESS);
} /* END _MXMLToXString() */

int _MXMLSetVal(

    mxml_t *E,                   /* I (modified) */
    void *V,                     /* I */
    enum MDataFormatEnum Format) /* I */

{
    char tmpLine[MMAX_LINE];
    char *ptr;

    if ((E == NULL) || (V == NULL)) {
        return (FAILURE);
    }

    if (E->Val != NULL) {
        free(E->Val);

        E->Val = NULL;
    }

    switch (Format) {
        case mdfString:
        default:

            ptr = (char *)V;

            break;

        case mdfInt:

            sprintf(tmpLine, "%d", *(int *)V);

            ptr = tmpLine;

            break;

        case mdfLong:

            sprintf(tmpLine, "%ld", *(long *)V);

            ptr = tmpLine;

            break;

        case mdfDouble:

            sprintf(tmpLine, "%f", *(double *)V);

            ptr = tmpLine;

            break;
    } /* END switch(Format) */

    E->Val = strdup(ptr);

    /* strip '<' symbols  NOTE:  ignore '<' symbol in attrs */

    /* NOTE:  must replace temp hack 14 w/ &lt; */

    for (ptr = strchr(E->Val, '<'); ptr != NULL; ptr = strchr(ptr, '<'))
        *ptr = (char)14;

    return (SUCCESS);
} /* END _MXMLSetVal() */

int _MSUSelectRead(

    int sd,                  /* I */
    unsigned long TimeLimit) /* I (in us) */

{
    struct timeval TimeOut;
    int numfds;

    fd_set rset;

    const char *FName = "_MSUSelectRead";

    MDB(7, fSOCK) MLog("%s(%d,%lu)\n", FName, sd, TimeLimit);

    FD_ZERO(&rset);

    FD_SET(sd, &rset);

    TimeOut.tv_sec = TimeLimit / 1000000;
    TimeOut.tv_usec = TimeLimit % 1000000;

    numfds = sd;

    if (select(numfds + 1, &rset, NULL, NULL, &TimeOut) > 0) {
        if (FD_ISSET(sd, &rset)) {
            return (SUCCESS);
        }

        MDB(2, fSOCK) MLog("_MSUSelectRead-FD is not set\n");
    } /* END if (select() > 0) */

    MDB(2, fSOCK) MLog("_MSUSelectRead-select failed\n");

    return (FAILURE);
} /* END _MSUSelectRead() */

int _MUSleep(

    long SleepDuration) /* I (in us) */

{
    struct timeval timeout;

    if ((X.XSleep != (int (*)())0) && (MSched.TimePolicy != mtpReal)) {
        (*X.XSleep)(X.xd, SleepDuration);
    } else {
        timeout.tv_sec = SleepDuration / 1000000;
        timeout.tv_usec = SleepDuration % 1000000;

        select(0, (fd_set *)NULL, (fd_set *)NULL, (fd_set *)NULL, &timeout);
    }

    return (SUCCESS);
} /* END _MUSleep() */

int _MUStrNCmpCI(

    char *String,  /* I */
    char *Pattern, /* I */
    int SStrLen)   /* I (optional) */

{
    int StrLen;

    int pindex;

    if ((String == NULL) || (Pattern == NULL)) {
        return (FAILURE);
    }

    StrLen = (SStrLen > 0) ? SStrLen : MMAX_BUFFER;

    for (pindex = 0; pindex < StrLen; pindex++) {
        if (Pattern[pindex] == '\0') {
            if (String[pindex] != '\0') {
                return (FAILURE);
            }

            break;
        }

        if (tolower(String[pindex]) != tolower(Pattern[pindex])) {
            return (FAILURE);
        }
    } /* END for (pindex) */

    return (SUCCESS);
} /* END _MUStrNCmpCI() */

int _MSecHMACGetDigest(

    unsigned char *CSKeyString, /* I */
    int CSKeyLen,               /* I */
    unsigned char *DataString,  /* I */
    int DataLen,                /* I */
    char *CSString,             /* O */
    int MaxCSStringLen,         /* I */
    char *DigestString,         /* O (optional) */
    mbool_t TwoPass,            /* I */
    mbool_t Use64BitEncoding)   /* I */

{
    SHA1_CTX ictx;
    SHA1_CTX octx;

    unsigned char isha[SHA_DIGESTSIZE];
    unsigned char osha[SHA_DIGESTSIZE];
    unsigned char key[SHA_DIGESTSIZE];
    unsigned char buf[SHA_BLOCKSIZE];
    unsigned char osha2[SHA_DIGESTSIZE];

    int index;

    char tmpBuf[SHA_DIGESTSIZE << 1];

    unsigned char *DPtr;

    int DLen;

#ifndef __MPROD
    const char *FName = "_MSecHMACGetDigest";

    MDB(6, fSTRUCT)
    MLog("%s(%s,%d,%s,%d,CSString,%d,DigestString,%s,%s)\n", FName,
         (CSKeyString != NULL) ? (char *)CSKeyString : "NULL", CSKeyLen,
         (DataString != NULL) ? (char *)DataString : "NULL", DataLen,
         MaxCSStringLen, _MBool[TwoPass], _MBool[Use64BitEncoding]);
#endif /* !__MPROD */

    /* create hash */

    if (TwoPass == TRUE) {
        SHA1_CTX tctx;

        _MSecSHA1Init(&tctx);
        _MSecSHA1Update(&tctx, DataString, DataLen);
        _MSecSHA1Final(osha2, &tctx);

        if (DigestString != NULL) {
            /* encode digest */

            if (Use64BitEncoding == TRUE) {
                _MSecBufTo64BitEncoding((char *)osha2, SHA_DIGESTSIZE,
                                       DigestString);
            } else {
                _MSecBufToHexEncoding((char *)osha2, SHA_DIGESTSIZE,
                                     DigestString);
            }
        } /* END if (DigestValue != NULL) */

        DPtr = osha2;
        DLen = SHA_DIGESTSIZE;
    } /* END if (CSKeyString == NULL) */
    else {
        DPtr = DataString;
        DLen = DataLen;
    }

    if (CSKeyLen > SHA_BLOCKSIZE) {
        SHA1_CTX tctx;

        _MSecSHA1Init(&tctx);
        _MSecSHA1Update(&tctx, CSKeyString, CSKeyLen);
        _MSecSHA1Final(key, &tctx);

        CSKeyString = key;
        CSKeyLen = SHA_DIGESTSIZE;
    }

    /**** inner digest ****/

    _MSecSHA1Init(&ictx);

    /* pad the key for inner digest */

    for (index = 0; index < CSKeyLen; index++) {
        buf[index] = CSKeyString[index] ^ 0x36;
    }

    for (index = CSKeyLen; index < SHA_BLOCKSIZE; index++) {
        buf[index] = 0x36;
    }

    _MSecSHA1Update(&ictx, buf, SHA_BLOCKSIZE);

    _MSecSHA1Update(&ictx, DPtr, DLen);

    _MSecSHA1Final(isha, &ictx);

    /**** outer digest ****/

    _MSecSHA1Init(&octx);

    /* pad the key for outer digest */

    for (index = 0; index < CSKeyLen; index++) {
        buf[index] = CSKeyString[index] ^ 0x5C;
    }

    for (index = CSKeyLen; index < SHA_BLOCKSIZE; index++) {
        buf[index] = 0x5C;
    }

    _MSecSHA1Update(&octx, buf, SHA_BLOCKSIZE);
    _MSecSHA1Update(&octx, isha, SHA_DIGESTSIZE);

    _MSecSHA1Final(osha, &octx);

    /* truncate and encode digest */

    _MSecHMACTruncate((char *)osha, tmpBuf,
                       MIN(SHA_DIGESTSIZE, MaxCSStringLen));

    if (Use64BitEncoding == TRUE) {
        _MSecBufTo64BitEncoding(tmpBuf, MIN(SHA_DIGESTSIZE, MaxCSStringLen),
                               CSString);
    } else {
        _MSecBufToHexEncoding(tmpBuf, MIN(SHA_DIGESTSIZE, MaxCSStringLen),
                             CSString);
    }

    return (SUCCESS);
} /* END _MSecHMACGetDigest() */

int _MSecComp64BitToBufDecoding(

    char *IBuf, int IBufLen, char *OBuf, int *OBufLen)

{
    int IIndex = 0;
    int OIndex = 0;

    if ((OBuf == NULL) || (IBuf == NULL)) {
        return (FAILURE);
    }

    do {
        OBuf[OIndex++] = ((CDList[(int)IBuf[IIndex]] << 2) & 0xfc) |
                         ((CDList[(int)IBuf[IIndex + 1]] >> 4) & 0x03);

        OBuf[OIndex++] = ((CDList[(int)IBuf[IIndex + 1]] << 4) & 0xf0) |
                         ((CDList[(int)IBuf[IIndex + 2]] >> 2) & 0x0f);

        OBuf[OIndex++] = ((CDList[(int)IBuf[IIndex + 2]] << 6) & 0xc0) |
                         ((CDList[(int)IBuf[IIndex + 3]]));

        IIndex += 4;
    } while (IIndex < IBufLen);

    if (OBufLen != NULL) *OBufLen = OIndex;

    return (SUCCESS);
} /* _MSecComp64BitToBufDecoding() */

void _MSecSHA1Init(

    SHA1_CTX *context) /* O */

{
#ifndef __MPROD
    const char *FName = "_MSecSHA1Init";

    MDB(9, fSTRUCT) MLog("%s(context)\n", FName);
#endif /* !__MPROD */

    /* initialize new context */

    /* SHA1 initialization constants */

    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;

    return;
} /* END _MSecSHA1Init() */

void _MSecSHA1Update(

    SHA1_CTX *Context, unsigned char *Data, MUINT4 DataLen)

{
    MUINT4 i, j;

#ifndef __MPROD
    const char *FName = "_MSecSHA1Update";

    MDB(10, fSTRUCT) MLog("%s(Context,Data,DataLen)\n", FName);
#endif /* !__MPROD */

    j = (Context->count[0] >> 3) & 63;

    if ((Context->count[0] += DataLen << 3) < (DataLen << 3))
        Context->count[1]++;

    Context->count[1] += (DataLen >> 29);

    if ((j + DataLen) > 63) {
        i = 64 - j;

        memcpy(&Context->buffer[j], Data, i);

        __MSecSHA1Transform(Context->state, Context->buffer);

        for (; i + 63 < DataLen; i += 64) {
            __MSecSHA1Transform(Context->state, &Data[i]);
        } /* END for() */

        j = 0;
    } else {
        i = 0;
    }

    memcpy(&Context->buffer[j], &Data[i], DataLen - i);

    return;
} /* END _MSecSHA1Update() */

void _MSecSHA1Final(

    unsigned char digest[20], SHA1_CTX *context)

{
    /* add padding and return the message digest. */

    MUINT4 i;
    unsigned char finalcount[8];

#ifndef __MPROD
    const char *FName = "_MSecSHA1Final";

    MDB(9, fSTRUCT) MLog("%s(digest,context)\n", FName);
#endif /* !__MPROD */

    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)] >>
                                         ((3 - (i & 3)) * 8)) &
                                        255); /* Endian independent */
    }                                         /* END for (i) */

    _MSecSHA1Update(context, (unsigned char *)"\200", 1);

    while ((context->count[0] & 504) != 448) {
        _MSecSHA1Update(context, (unsigned char *)"\0", 1);
    }

    _MSecSHA1Update(context, finalcount, 8);

    for (i = 0; i < 20; i++) {
        digest[i] =
            (unsigned char)((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) &
                            255);
    } /* END for (i) */

    /* reset variables */

    i = 0;

    memset(context->buffer, 0, 64);
    memset(context->state, 0, 20);
    memset(context->count, 0, 8);
    memset(finalcount, 0, 8);

    return;
} /* END _MSecSHA1Final() */

int _MSecBufTo64BitEncoding(

    char *IBuf,  /* I */
    int IBufLen, /* I */
    char *OBuf)  /* O */

{
    int index;
    int cindex;

    int block;
    int slack;
    int end;

    char tmpBuf[5];

    short val;

    if ((IBuf == NULL) || (OBuf == NULL)) {
        return (SUCCESS);
    }

    tmpBuf[4] = '\0';

    OBuf[0] = '\0';

    for (index = 0; index < IBufLen; index += 3) {
        block = 0;
        slack = IBufLen - index - 1;
        end = MIN(2, slack);

        for (cindex = 0; cindex <= end; cindex++) {
            block +=
                ((unsigned char)IBuf[index + cindex] << (8 * (2 - cindex)));
        } /* END for (cindex) */

        for (cindex = 0; cindex < 4; cindex++) {
            val = (block >> (6 * (3 - cindex))) & 0x003f;

            tmpBuf[cindex] = CList[val];
        } /* END for (cindex) */

        if (slack < 2) {
            tmpBuf[3] = '=';

            if (slack < 1) tmpBuf[2] = '=';
        }

        strcat(OBuf, tmpBuf);
    } /* END for (index) */

    return (SUCCESS);
} /* END _MSecBufTo64BitEncoding() */

int _MSecBufToHexEncoding(

    char *IBuf,  /* I */
    int IBufLen, /* I */
    char *OBuf)  /* O */

{
    int index;

    if ((IBuf == NULL) || (OBuf == NULL)) {
        return (SUCCESS);
    }

    OBuf[0] = '\0';

    for (index = 0; index < IBufLen; index++) {
        sprintf(temp_str, "%02x", (unsigned char)IBuf[index]);
        strcat(OBuf, temp_str);
    } /* END for (index) */

    return (SUCCESS);
} /* END _MSecBufToHexEncoding() */

void _MSecHMACTruncate(

    char *d1, /* data to be truncated */
    char *d2, /* truncated data */
    int len)  /* length in bytes to keep */

{
    int i;

#ifndef __MPROD
    const char *FName = "_MSecHMACTruncate";

    MDB(9, fSTRUCT) MLog("%s(d1,d2,%d)\n", FName, len);
#endif /* !__MPROD */

    for (i = 0; i < len; i++) {
        d2[i] = d1[i];
    } /* END for (i) */

    return;
} /* END _MSecHMACTruncate() */

static void __MSecSHA1Transform(

    MUINT4 state[5],          /* I */
    unsigned char buffer[64]) /* I */

{
    /* hash a single 512-bit block. This is the core of the algorithm. */

    MUINT4 a, b, c, d, e;

    unsigned char workspace[64];

    CHAR64LONG16 *block = (CHAR64LONG16 *)workspace;

#ifndef __BMPROD
    const char *FName = "__MSecSHA1Transform";

    MDB(9, fSTRUCT) MLog("%s(context)\n", FName);
#endif /* !__BMPROD */

    memcpy(block, buffer, 64);

    /* copy context->state[] to working vars */

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    /* 4 rounds of 20 operations each. Loop unrolled. */

    R0(a, b, c, d, e, 0);
    R0(e, a, b, c, d, 1);
    R0(d, e, a, b, c, 2);
    R0(c, d, e, a, b, 3);
    R0(b, c, d, e, a, 4);
    R0(a, b, c, d, e, 5);
    R0(e, a, b, c, d, 6);
    R0(d, e, a, b, c, 7);
    R0(c, d, e, a, b, 8);
    R0(b, c, d, e, a, 9);
    R0(a, b, c, d, e, 10);
    R0(e, a, b, c, d, 11);
    R0(d, e, a, b, c, 12);
    R0(c, d, e, a, b, 13);
    R0(b, c, d, e, a, 14);
    R0(a, b, c, d, e, 15);
    R1(e, a, b, c, d, 16);
    R1(d, e, a, b, c, 17);
    R1(c, d, e, a, b, 18);
    R1(b, c, d, e, a, 19);
    R2(a, b, c, d, e, 20);
    R2(e, a, b, c, d, 21);
    R2(d, e, a, b, c, 22);
    R2(c, d, e, a, b, 23);
    R2(b, c, d, e, a, 24);
    R2(a, b, c, d, e, 25);
    R2(e, a, b, c, d, 26);
    R2(d, e, a, b, c, 27);
    R2(c, d, e, a, b, 28);
    R2(b, c, d, e, a, 29);
    R2(a, b, c, d, e, 30);
    R2(e, a, b, c, d, 31);
    R2(d, e, a, b, c, 32);
    R2(c, d, e, a, b, 33);
    R2(b, c, d, e, a, 34);
    R2(a, b, c, d, e, 35);
    R2(e, a, b, c, d, 36);
    R2(d, e, a, b, c, 37);
    R2(c, d, e, a, b, 38);
    R2(b, c, d, e, a, 39);
    R3(a, b, c, d, e, 40);
    R3(e, a, b, c, d, 41);
    R3(d, e, a, b, c, 42);
    R3(c, d, e, a, b, 43);
    R3(b, c, d, e, a, 44);
    R3(a, b, c, d, e, 45);
    R3(e, a, b, c, d, 46);
    R3(d, e, a, b, c, 47);
    R3(c, d, e, a, b, 48);
    R3(b, c, d, e, a, 49);
    R3(a, b, c, d, e, 50);
    R3(e, a, b, c, d, 51);
    R3(d, e, a, b, c, 52);
    R3(c, d, e, a, b, 53);
    R3(b, c, d, e, a, 54);
    R3(a, b, c, d, e, 55);
    R3(e, a, b, c, d, 56);
    R3(d, e, a, b, c, 57);
    R3(c, d, e, a, b, 58);
    R3(b, c, d, e, a, 59);
    R4(a, b, c, d, e, 60);
    R4(e, a, b, c, d, 61);
    R4(d, e, a, b, c, 62);
    R4(c, d, e, a, b, 63);
    R4(b, c, d, e, a, 64);
    R4(a, b, c, d, e, 65);
    R4(e, a, b, c, d, 66);
    R4(d, e, a, b, c, 67);
    R4(c, d, e, a, b, 68);
    R4(b, c, d, e, a, 69);
    R4(a, b, c, d, e, 70);
    R4(e, a, b, c, d, 71);
    R4(d, e, a, b, c, 72);
    R4(c, d, e, a, b, 73);
    R4(b, c, d, e, a, 74);
    R4(a, b, c, d, e, 75);
    R4(e, a, b, c, d, 76);
    R4(d, e, a, b, c, 77);
    R4(c, d, e, a, b, 78);
    R4(b, c, d, e, a, 79);

    /* add the working vars back into context.state[] */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;

    /* clear variables */

    a = b = c = d = e = 0;

    memset(block, 0, 64);

    return;
} /* END __MSecSHA1Transform() */
