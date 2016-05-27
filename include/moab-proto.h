/*
*/

#ifdef __MINSURE
int _Insure_mem_info(void *pmem);
int _Insure_ptr_info(void **pptr);
long _Insure_list_allocated_memory(int mode);
#endif /* __MINSURE */

#include "moab.h"
#include "mcom-proto.h"

/* CP object */

int MCPCreate(char *);
int MCPStoreCluster(mckpt_t *,mnode_t **); 
int MCPLoadSched(mckpt_t *,char *,msched_t *);     
int MCPStoreResList(mckpt_t *,mres_t **);
int MCPIsSupported(mckpt_t *,char *);
int MCPLoadSched(mckpt_t *,char *,msched_t *);            
int MCPStoreSRList(mckpt_t *,sres_t *);
int MCPLoadSys(mckpt_t *,char *,msched_t *);
int MCPLoadSysStats(char *);
int MCPLoadStats(char *);
int MCPWriteGridStats(FILE *);
int MCPLoadSR(char *);
int MCPStoreUserList(mckpt_t *,mgcred_t **);
int MCPStoreGroupList(mckpt_t *,mgcred_t *);
int MCPStoreAcctList(mckpt_t *,mgcred_t *);
int MCPStoreObj(FILE *,int,char *,char *);
int MCPLoad(char *,int);
int MCPWriteScheduler(FILE *);
int MCPWriteJobs(FILE *,char *);
int MCPWriteStandingReservations(FILE *,char *);
int MNodeToString(mnode_t *,char *);
int MCPWriteSystemStats(FILE *);
int MCPWriteGridStats(FILE *);
int MCPRestore(int,char *,void *);



/* user object */

int MUserLoadCP(mgcred_t *,char *);
int MUserToXML(mgcred_t *,mxml_t **,int *);
char *MUserShow(mgcred_t *,char *,long *,long);
int MUserInitialize(mgcred_t *,char *);
int MUserFind(char *,mgcred_t **);
int MUserAdd(char *,mgcred_t **);
int MUserCreate(char *,mgcred_t **);
int MUserToString(mgcred_t *,char *);
int MUserDestroy(mgcred_t **);
int MUserFreeTable(void);



/* group object */

char *MGroupShow(mgcred_t *,char *,long *,long);
int MGroupLoadCP(mgcred_t *,char *);    
int MGroupToXML(mgcred_t *,mxml_t **,int *);  
int MGroupInitialize(mgcred_t *,char *);
int MGroupFind(char *,mgcred_t **);
int MGroupAdd(char *,mgcred_t **);
int MGroupToString(mgcred_t *,char *);
int MGroupProcessConfig(mgcred_t *,char *);

/* HvB */
void MGroupGetFSGroups();
int MGroupSecondary(char *, mjob_t *);


/* acct object */

char *MAcctShow(mgcred_t *,char *,long *,long);
int MAcctLoadCP(mgcred_t *,char *);
int MAcctToXML(mgcred_t *,mxml_t **,int *);  
int MAcctInitialize(mgcred_t *,char *);
int MAcctFind(char *,mgcred_t **);
int MAcctAdd(char *,mgcred_t **);
int MAcctToString(mgcred_t *,char *);



/* cred object */

int MCredAToString(void *,int,int,char *,int);    
int MCredSetAttr(void *,int,int,void **,int,int);
int MOFromXML(void *,int,mxml_t *);   
int MOFromString(void *,int,char *);
int MOToXML(void *,int,mxml_t **);
int MOGetComponent(void *,int,void **,int);
int MCOToXML(void *,int,mxml_t **,int *,int *,int);
void *MOGetNextObject(void **,int,int,void *,char **);
int MCredConfigShow(void *,int,int,int,char *);
int MCredConfigLShow(void *,int,int,int,char *);
char *MOGetName(void *,int,char **);
int MCredInitialize(int,void *,char *);
int MCredAdd(int,char *,void **);
int MCredLoadConfig(int,char *,char *,char *);
int MCredAdjustConfig(int,void *);
int MOLoadPvtConfig(void **,int,char *,mpsi_t *,char *);
int MCredSetDefaults(void);
int MOGetObject(int,char *,void **,int);
int MCredProcessConfig(void *,int,char *,mcredl_t *,mfs_t *);
char *MCredShowAttrs(mpu_t *,mpu_t *,mpu_t *,mpu_t *,mpu_t *,mfs_t *,long,long);
int MCredIsMatch(mcred_t *,void *,int);



/* res object */

int MResAllocate(mres_t *,mnalloc_t *);
int MResDeallocateResources(mres_t *);
int MResShowHostList(mres_t *);
int MResGetRID(mres_t *,char *,char *);
int MResLoadCP(mres_t *,char *);
int MResAdjust(mres_t *,long,int);
int MResFromXML(mres_t *,mxml_t *);
int MResSetAttr(mres_t *,enum MResAttrEnum,void *,int,int);
int MResToXML(mres_t *,mxml_t *,int *);
int MResToJob(mres_t *,mjob_t *);
int MResTrap(mres_t *);
int MResPreempt( mres_t *);
int MResCreate(int,macl_t *,char *,unsigned long,mnalloc_t *,long,long,int,int,char *,mres_t **,char *,mcres_t *);
int MResAdjustTime(long);
int MResDestroy(mres_t **);
int MResChargeAllocation(mres_t *,int);
int MResShow(mres_t *);
int MResInitialize(mres_t **,char *);
int MResFind(char *,mres_t **);
int MResUpdateStats(void);
int MResCheckStatus(mres_t *);
int MResJCreate(mjob_t *,mnodelist_t,long,int,mres_t **);
int MResFreeTable(void);
int MResCheckJAccess(mres_t *,mjob_t *,long,int *,char *);
int MResCheckRAccess(mres_t *,mres_t *,long,int *,char *);
int MResAddNode(mres_t *,mnode_t *,int,int);
int MResCheckJobMatch(mjob_t *,mres_t *);
int MResAdjustGResUsage(mres_t *,int);
int MResGetPE(mcres_t *,mpar_t *,double *);
int MResAllocateRE(char *,int,int,mnalloc_t *,int *,long,long,char *,int,mreq_t *);
int MREInsert(mre_t *,long,long,int,mcres_t *,int);
int MRERelease(mre_t *,int,int);
int MRECheck(mnode_t *,char *,int);
int MResShowState(mres_t *,int,char *,int,int);
int MResDiagnoseState(mres_t *,int,char *,int,int);
int MResDiagGrid(char *,int,int);
int MResAdjustDRes(char *,int);
int MResToString(mres_t *,int *,char *,int,int *);
int MNResToXML(mnode_t *,int,mxml_t *,int *);
int MNResToString(mnode_t *,mres_t *,mxml_t **,char *,int);



/* AM object */

int MAMInitialize(mam_t *);
int MAMClose(mam_t *);
int MAMProcessOConfig(mam_t *,int,int,double,char *,char **);
int MAMSetAttr(mam_t *,int,void **,int,int);
int MAMAllocJDebit(mam_t *,mjob_t *,enum MHoldReasonEnum *,char *);
int MAMAllocRDebit(mam_t *,mrsv_t *,enum MHoldReasonEnum *,char *);
int MAMQBDoCommand(mam_t *,int,char *,void **,int *,char *);
int MAMAllocJReserve(mam_t *,mjob_t *,mbool_t,enum MHoldReasonEnum *,char *);
int MAMAllocResCancel(char *,char *,char *,char *,enum MHoldReasonEnum *);
int MAMAccountGetDefault(char *,char *,enum MHoldReasonEnum *);
int MAMLoadConfig(char *,char *);
int MAMAdd(char *,mam_t **);
int MAMShow(mam_t *,char *,int,int);
int MAMConfigShow(mam_t *,int,char *);
int MAMFind(char *,mam_t **);
int MAMDestroy(mam_t **);
int MAMCreate(char *,mam_t **);
int MAMProcessConfig(mam_t *,char *);
int MAMCheckConfig(mam_t *);
int MAMSetDefaults(mam_t *);
int MAMActivate(mam_t *);
int MAMGetChargeRateInfo(char *);
int MAMSyncAlloc(mam_t *,mrm_t *);
int MAMAllocRReserve(mam_t *,char *,long,char *,int,int,long,char *,char *,enum MHoldReasonEnum *);
int MAMIAccountVerify(char *,char *);
int MAMShutdown(mam_t *);



/* par object */

int MParSetDefaults(mpar_t *);
int MParInitialize(mpar_t *,char *);
int MParAdd(char *,mpar_t **);
int MParFind(char *,mpar_t **);
int MParProcessOConfig(mpar_t *,int,int,double,char *,char **);
int MParGetTC(mpar_t *,mcres_t *,mcres_t *,mcres_t *,mcres_t *,long);
int MParAdd(char *,mpar_t **);
int MParAddNode(mpar_t *,mnode_t *);
int MParShow(char *,char *,long *,long);
int MParUpdate(mpar_t *);
int MParConfigShow(mpar_t *,int,int,char *);
int MParListBMFromString(char *,int *,int);
char *MParBMToString(int *);



/* node object */

int MNodeShow(mnode_t *);
int MNodeConfigShow(mnode_t *,int,int,char *);
int MNodeGetPriority(mnode_t *,int,int,double *,long);
int MNodeProcessPrioF(mnode_t *,char *);
int MNodeGetTC(mnode_t *,mcres_t *,mcres_t *,mcres_t *,mcres_t *,long);
int MNodeInitialize(mnode_t *,char *);
int MNodeAdjustState(mnode_t *,enum MNodeStateEnum *);
int MNodeAdjustAvailResources(mnode_t *,double,short,short);
int MNodeSetAttr(mnode_t *,enum MNodeAttrEnum,void **,int,int);
int MNodeSetState(mnode_t *,int,int);
int MNodeEval(mnode_t *);
int MNodeLoadConfig(mnode_t *,char *);
int MNodeProcessConfig(mnode_t *,char *);
int MNodeBuildRE(mnode_t *,mres_t *,int);
int MNodeFind(char *,mnode_t **);
int MNodeAdd(char *,mnode_t **);
int MNodeCreate(mnode_t **);
int MNodeRemove(mnode_t *);
int MNodeDestroy(mnode_t **);
int MNodeCopy(mnode_t *,mnode_t *);
int MNodeTrap(mnode_t *);
int MNodeSetClass(mnode_t *,mclass_t *,char *,int);
char *MNodeAdjustName(char *,int);
int MNodeUpdateResExpression(mnode_t *);
int MClusterUpdateNodeState(void);
int MFrameAdd(char *,int *,mframe_t **);
int MFrameFind(char *,mframe_t **);
int MFrameAddNode(mframe_t *,mnode_t *,int);
int MFrameShow(char *,mpar_t *,char *,int,int);
int MNodeGetLocation(mnode_t *);
int MNodeLocationFromName(mnode_t *,int *,int *);
int MNodeCheckPolicies(mjob_t *,mnode_t *,long,int *);
int MNodeCheckStatus(mnode_t *);
int MNodeResetJobSlots(mnode_t *);
int MNodeGetPreemptList(mjob_t *,mnalloc_t *,mnalloc_t *,mjob_t **,long,int,int,int *,int *);
int MNodeSelectIdleTasks(mjob_t *,mreq_t *,mnalloc_t *,mnodelist_t,int *,int *,char *,int R[MAX_MREQ_PER_JOB][MAX_MREJREASON]);
int MNodeSelectPreemptTasks(mjob_t *,mnalloc_t *,mnodelist_t,int *,int *,char *,int R[MAX_MREQ_PER_JOB][MAX_MREJREASON],long);
int MClusterClearUsage(void);
int MNodeFreeTable(void);
int MNodeProcessFeature(mnode_t *,char *);
int MNodeCheckAllocation(mnode_t *);
int MNodeLoadCP(mnode_t *N,char *Buf);
int MNodeShowState(mnode_t *,int,char *,int,int);
int MNodeDiagnoseState(mnode_t *,int,char *,int,int);
int MNodeShowReservations(mnode_t *,int,char *,int,int);
int MNodeShowRes(mnode_t *,char *,mpar_t *,int,int,char *,int);
int MNodeDiagnoseReservations(mnode_t *,int,char *,int,int);



/* req object */

int MReqDestroy(mreq_t **);
int MReqSetAttr(mjob_t *,mreq_t *,enum MReqAttrEnum,void **,int,int);
int MReqCreate(mjob_t *,mreq_t *,mreq_t **,mbool_t);
int MReqGetFNL(mjob_t *,mreq_t *,mpar_t *,nodelist_t,nodelist_t,int *,int *,long,unsigned long);
int MReqRResFromString(mjob_t *,mreq_t *,char *,int,int);
int MReqGetPref(mreq_t *,mnode_t *,char *);
int MReqAllocateLocalRes(mjob_t *,mreq_t *);
int MReqAToString(mjob_t *,mreq_t *,enum MReqAttrEnum,char *,int);



/* job object */

int MJobFind(char *,mjob_t **,int);
int MJobCreate(char *,mbool_t,mjob_t **);
int MJobMove(mjob_t *,mjob_t *);
int MJobShow(mjob_t *,int,char *);
int MJobReserve(mjob_t *,int);
int MReqCheckResourceMatch(mjob_t *,mreq_t *,mnode_t *,int *);
int MJobUpdateFlags(mjob_t *);
int MJobUpdateResourceCache(mjob_t *,int);
int MJobClearResourceCache(mjob_t *);
int MJobPReserve(mjob_t *,int,int *,mbool_t *);
int MJobProximateMNL(mjob_t *,mnodelist_t,mnodelist_t,long,int);
int MJobCheckPolicies(mjob_t *,int,int,mpar_t *,int *,char *,long);
int MJobSetAttr(mjob_t *,enum MJobAttrEnum,void **,int,int);
int MJobEval(mjob_t *);
int MJobFromXML(mjob_t *,mxml_t *);
int MJobInitialize(mjob_t *);
int MJobToXML(mjob_t *,mxml_t *,int *);
int MJobAttrToString(mjob_t *,int,char *,int);
int MJobSetQOS(mjob_t *,mqos_t *,int);
int MJobSetState(mjob_t *,enum MJobStateEnum);
int MJobPreempt(mjob_t *,mjob_t **,enum MPreemptPolicyEnum,char *,int *);
int MJobResume(mjob_t *,char *,int *);
int MJobGetPAL(mjob_t *,int *,int *,mpar_t **);
mpar_t *MJobFindDefPart(mjob_t *, mclass_t *, int *);
int MJobRemove(mjob_t *);
int MJobGetAccount(mjob_t *,mgcred_t **);
int MJobSetCreds(mjob_t *,char *,char *,char *);
int MJobAllocMNL(mjob_t *,mnodelist_t,char *,mnodelist_t,int,long);
int MJobNLDistribute(mjob_t *,mnodelist_t,mnodelist_t);
int MJobSelectMNL(mjob_t *,mpar_t *,nodelist_t,mnodelist_t,char *,int);
int MJobDistributeTasks(mjob_t *,mrm_t *,mnalloc_t *,short *);
int MJobTrap(mjob_t *);
char *MJobGetName(mjob_t *,char *,mrm_t *,char *,int,enum MJobNameEnum);
int MJobGetStartPriority(mjob_t *,int,double *,int,char **,int *);
int MJobGetRunPriority(mjob_t *,int,double *,char *);
int MJobGetBackfillPriority(mjob_t *,time_t,int,double *,char *);
int MJobGetPartitionAccess(mjob_t *);
int MJobGetAMNodeList(mjob_t *,mnodelist_t,mnodelist_t,char NM[MAX_MNODE],int *,int *,long);
int MJobProcessCompleted(mjob_t *);
int MJobProcessRemoved(mjob_t *);
int MJobAllocatePriority(mjob_t *,mreq_t *,mnalloc_t *,int,int *,int *,char *,int,int *,mnalloc_t *A[MAX_MREQ_PER_JOB],int *,int *,long);
int MJobAllocateFastest(mjob_t *,mreq_t *,mnalloc_t *,int,int *,int *,char *,int,int *,mnalloc_t *A[MAX_MREQ_PER_JOB],int *,int *);
int MJobAllocateBalanced(mjob_t *,mreq_t *,mnalloc_t *,int,int *,int *,char *,int,int *,mnalloc_t *A[MAX_MREQ_PER_JOB],int *,int *);
int MJobAllocateContiguous(mjob_t *,mreq_t *,mnalloc_t *,int,int *,int *,char *,int,int *,mnalloc_t *A[MAX_MREQ_PER_JOB],int *,int *);
int MJobCheckNStartTime(mjob_t *,mreq_t *,mnode_t *,long,int *,double,int *,char *,long *);
int MJobGetNRange(mjob_t *,mreq_t *,mnode_t *,long,int *,long *,char *,int *,char *);
int MJobGetSNRange(mjob_t *,mreq_t *,mnode_t *,mrange_t *,int,char *,int *,mrange_t *,mcres_t *,char *);
int MJobCTimeComp(int *,int *);
int MJobStartPrioComp(int *,int *);
int MRangeApplyLocalDistributionConstraints(mrange_t *,mjob_t *,mnode_t *);
int MRangeApplyGlobalDistributionConstraints(mrange_t *,mjob_t *,int *);
int MRangeGetIntersection(mjob_t *,mrange_t *,mrange_t *);
int MRLMerge(mrange_t *,mrange_t *,int,long *);
int MRLAND(mrange_t *,mrange_t *,mrange_t *);
int MRLANDTest(void);
int MRLLimitTC(mrange_t *,mrange_t *,mrange_t *,int);
int MRLSubtract(mrange_t *,mrange_t *);
int MRLSFromA(long,mrange_t *,mrange_t *);
int MJobSendFB(mjob_t *);
int MJobStart(mjob_t *);
int MJobCheckpoint(mjob_t *);
int MJobSetHold(mjob_t *,int,long,enum MHoldReasonEnum,char *);
int MJobMkTemp(mjob_t *,mreq_t *,macl_t *,macl_t *,mnalloc_t *,mnalloc_t *);
int MJobCheckLimits(mjob_t *,int,mpar_t *,int,char *);
int MJobCheckDataReq(mjob_t *);
int MJobDestroy(mjob_t **);
int MJobSelectPJobList(mjob_t *,int,int,mjob_t **,mnalloc_t *,mjob_t **,int *,int *,nodelist_t **);
int MJobCheckDependency(mjob_t *,enum MJobDependEnum *,char *);
int MJobSetDependency(mjob_t *,enum MJobDependEnum,char *);
int MJobSelectResourceSet(mjob_t *,mreq_t *,int,int,char **,mnalloc_t *,int);
int MJobCheckNRes(mjob_t *,mnode_t *,mreq_t *,long,int *,double,int *,char *,long *,int);
int MJobGetEStartTime(mjob_t *,mpar_t **,int *,int *,mnodelist_t,long *);
int MJobAddToNL(mjob_t *,nodelist_t);
int MJobRemoveFromNL(mjob_t *,nodelist_t);
int MJobFreeTable(void);
int MJobProcessExtensionString(mjob_t *,char *);
int MJobBuildCL(mjob_t *);
int MJobAddHash(char *,int,int *);
int MJobRemoveHash(char *);
int MJobCheckQueuePolicies(mjob_t *,int,mpar_t *,mpar_t *,int *);
int MJobCheckPolicyEvents(mjob_t *,long,int *,char *);
double MJobGetWCAccuracy(mjob_t *,double *);
int MJobGetPE(mjob_t *,mpar_t *,double *);
int MJobWriteStats(mjob_t *J);
int MJobGetNL(mjob_t *,mnalloc_t *);
int MJobDistributeTaskGeometry(mjob_t *,mreq_t *,mnalloc_t *,int *);
int MJobCPCreate(mjob_t *,mjckpt_t **);
int MJobCPDestroy(mjob_t *,mjckpt_t **);
int MJobGetRange(mjob_t *,mreq_t *,mpar_t *,long,mrange_t *,mnodelist_t,int *,char *,int,mrange_t *);
int MJobGetProcCount(mjob_t *);
int MJobGetINL(mjob_t *,mnalloc_t *,mnalloc_t *,int,int *,int *);
int MJobTestRMExtension(char *);
int MJobTestName(char *);
int MJobTestDist(void);
int MJobAddAccess(mjob_t *,char *);
int MJobSelectFRL(mjob_t *,mrange_t *,int,int *);
int MJobToTString(mjob_t *,int,char *,int);
int MJobCheckClassJLimits(mjob_t *,mclass_t *,int,char *,int);
int MJobLoadCP(mjob_t *,char *);
int MJobStoreCP(mjob_t *,char *);
int MJobAToString(mjob_t *,enum MJobAttrEnum,char *,int);
int MJobValidate(mjob_t *,char *,int);
int MJobDetermineCreds(mjob_t *);
int MJobGetLocalTL(mjob_t *,short *,int);



/* queue object */

int MQueueInitialize(mjob_t **,char *);
int MQueueScheduleIJobs(int *,mpar_t *);
int MQueuePrioritizeJobs(mjob_t **,int *);
int MQueueGetBestRQTime(int *,long *);
int MQueueScheduleRJobs(int *);
int MQueueScheduleSJobs(int *);
int MQueueDiagnose(mjob_t **,int *,int,mpar_t *,char *,int);
int MQueueCheckStatus(void);
int MQueueGetRequeueValue(int *,long,long,double *);
int MQueueSelectAllJobs(mjob_t **,int,mpar_t *,int *,int,int,int,char *);
int MQueueSelectJobs(int *,int *,int,int,int,unsigned long,int,int *,mbool_t,mbool_t);
int MQueueAddAJob(mjob_t *);
int MQueueRemoveAJob(mjob_t *,int);
int MQueueBackFill(int *,int,mpar_t *);
int MOQueueInitialize(int *);
int MOQueueDestroy(int *,int);



/* stat object */

int MStatInitialize(mprofcfg_t *);
int MStatSetDefaults(void);
int MStatOpenFile(long);
int MStatShutdown(void);
int MStatToString(must_t *,char *,int *);
int MStatToXML(must_t *,mxml_t **,int *);
int MStatFromString(char *,must_t *);
int MStatFromXML(must_t *,mxml_t *);
int MStatSetAttr(must_t *,int ,void **,int,int);
int MStatAToString(must_t *,int,char *,int);
char *MStatsToString(must_t *,char *);
int MStatClearUsage(int,int,int);
int MStatProfInitialize(mprofcfg_t *);
int MStatUpdateActiveJobUsage(mjob_t *);
int MStatInitializeActiveSysUsage(void);
int MStatUpdateCompletedJobUsage(mjob_t *,int,int);
int MStatUpdateRejectedJobUsage(mjob_t *,int);
int MStatUpdateSubmitJobUsage(mjob_t *);
double MStatCalcCommunicationCost(mjob_t *);
double MStatGetCom(mnode_t *,mnode_t *);
int MStatAddEJob(mjob_t *);
int MStatRemoveEJob(mjob_t *);
int MStatBuildRClass(int,mrclass_t *);
int MStatUpdateBFUsage(mjob_t *);
int MStatBuildGrid(int,char *,int);
char *MStatGetGrid(int,must_t *,must_t *,must_t *,must_t *,int);



/* sys object */

char *MSysToString(msched_t *,char *,int);
int MSysToXML(msched_t *,mxml_t **,int *, int *,int);
int MSysAToString(msched_t *,int,char *,int);
int MSysSetAttr(msched_t *,int,void **,int,int);
int MSysMemCheck(void);
int MSysDoTest(void);
int MSysCheck(void);
int MSysRegEvent(char *,int,long,int);
int MSysRegExtEvent(char *,int,long,int);
int MSysLaunchAction(char **,int);
int MSysLoadConfig(char *,char *,int);
int MSysUpdateTime(msched_t *);
int MSysDSRegister(char *,char *,char *,int,char *,char *);
int MSysDSUnregister(char *,char *,char *,int,char *,char *);
int MSysDSQuery(char *,char *,char *,int *,char *,char *);
int MSysEMSubmit(mpsi_t *,char *,char *,char *);
int MSysEMRegister(mpsi_t *,char *,char *,char *,char *);
int MSysSynchronize(void);
int MSysInitialize(mbool_t);
void MSysShutdown(int);
int MSysDestroyObjects(void);
int MSysDiagnose(char *,int,long);
int MSysStartServer(int);
int M64Init(m64_t *);



/* limit object */

int MLimitToXML(mcredl_t *,mxml_t **,int *);
int MLimitAToString(mcredl_t *,int,char *,int);
int MLimitToString(mcredl_t *,char *);
int MLimitFromXML(mcredl_t *,mxml_t *);
int MLimitSetAttr(mcredl_t *,int ,void **,int,int);           
int MLimitEnforceAll(mpar_t *);



/* fs object */

int MFSToXML(mfs_t *,mxml_t **,int *);
int MFSAToString(mfs_t *,int,char *,int);
int MFSTargetFromString(mfs_t *,char *);
int MFSSetAttr(mfs_t *,int,void **, int,int);
int MFSFromXML(mfs_t *,mxml_t *);
int MFSProcessOConfig(mfsc_t *,int,int,double,char *,char **);
int MFSLoadConfig(void);
int MFSCheckCap(mfs_t *,mjob_t *,mpar_t *,int *);
int MFSSetDefaults(mfs_t *,int);
int MFSLoadDataFile(char *,int);
int MFSUpdateData(mfsc_t *,int,int);
int MFSInitialize(mfsc_t *);
double MFSCalcFactor(mfsc_t *,double *);
char *MFSTargetToString(double,int);
int MFSShutdown(mfsc_t *);
/* Added the DiagOpt, of type char */
int MFSShow(char *,int,char *,int);



/* cfg object */

int MCfgGetIndex(int,int *);
int MCfgTranslateBackLevel(int *);
int MCfgAdjustBuffer(char **,mbool_t);
int MCfgProcessLine(int,char *,char *,char *);
int MCfgEnforceConstraints(void);
int MCfgGetVal(char **,const char *,char *,int *,char *,int,char **);
int MCfgGetIVal(char *,char **,const char *,char *,int *,int *,char **);
int MCfgGetDVal(char *,char **,const char *,char *,int *,double *,char **);
int MCfgGetSVal(char *,char **,const char *,char *,int *,char *,int,int,char **);
int MCfgGetSList(char *,char **,const char *,char *,int *,int,char *,char **);
int MCfgProcessBuffer(char *);
int MCfgSetVal(int,int,double,char *,char **,mpar_t *,char *);



/* qos object */

char *MQOSFlagsToString(mqos_t *,char *,int);
int MQOSFlagsFromString(mqos_t *,char *);
int MQOSSetAttr(mqos_t *,int,void **,int,int);
int MQOSAToString(mqos_t *,int,char *,int);
int MQOSProcessOConfig(mqos_t *,int,int,double,char *,char **);
int MQOSConfigLShow(mqos_t *,int,int,char *);
int MQOSInitialize(mqos_t *,char *);
int MQOSAdd(char *,mqos_t **);
int MQOSFind(char *,mqos_t **);
int MQOSGetAccess(mjob_t *,mqos_t *,int *,mqos_t **);
int MQOSLoadConfig(char *);
int MQOSProcessConfig(mqos_t *,char *);
int MQOSListBMFromString(char *,int *,int);
char *MQOSBMToString(int *);
int MQOSShow(char *,char *,long *,long);
int MQOSDestroy(mqos_t **);
int MQOSFreeTable(void);
int MQOSConfigShow(mqos_t *,int,int,char *,int);



/* class object */

int MClassSetAttr(mclass_t *,int,void **,int,int);
int MClassAToString(mclass_t *,int,char *,int,int);
int MClassProcessConfig(mclass_t *,char *);
int MClassConfigLShow(mclass_t *,int,int,char *);
int MClassInitialize(mclass_t *,char *);
int MClassAdd(char *,mclass_t **);
int MClassFind(char *,mclass_t **);
int MClassConfigShow(mclass_t *,int,char *);
int MClassGetPrio(mjob_t *,long *);
int MClassShow(mclass_t *,char *,long *,int);



/* sres object */

int MSRInitialize(sres_t *,char *);
int MSRFind(char *,sres_t **);
int MSRAdd(char *,sres_t **);
int MSRCreate(char *,sres_t *);
int MSRUpdate(sres_t *);
int MSRDestroy(sres_t **);
int MSRRefresh(void);
int MSRSetAttr(sres_t *,int,void **,int,int);
int MSRAToString(sres_t *,int,char *,int);
int MSRProcessOConfig(sres_t *,int,int,double,char *,char **);
int MSRSelectNodeList(mjob_t *,sres_t *,nodelist_t,int *,long,nodelist_t,unsigned long);
int MSRShow(sres_t *,char *,int,int);
int MSRConfigShow(sres_t *,int,int,char *);
int MSRSetRes(sres_t *,int,int);
int MSRGetAttributes(sres_t *,int,long *,unsigned long *);
int MSRCheckReservation(sres_t *,mres_t *);
int MSRGetCurrentValues(sres_t *,sres_t *,sres_t *);
int MSRBuildHostList(sres_t *);
int MSRLoadConfig(char *);
int MSRCheckConfig(sres_t *);
int MSRProcessConfig(sres_t *,char *);
int MSRToXML(sres_t *,mxml_t *,int *);
int MSRDiag(sres_t *,char *,int,int);
int MSRFromString(sres_t *,char *);
int MSRFromXML(sres_t *,mxml_t *);



/* sched object */

int MSchedProcessOConfig(msched_t *,int,int,double,char *,char **,char *);
int MSchedToString(msched_t *,char *);
int MSchedStatToString(msched_t *,int,char *,int);
int MSchedFromString(msched_t *,char *);
int MSchedSetDefaults(msched_t *);
int MSchedSetAttr(msched_t *,int,void **,int,int);
int MSchedOConfigShow(char *,int,int);
int MPolicyGetEStartTime(mjob_t *,mpar_t *,int,long *);
int MPolicyAdjustUsage(int *,mjob_t *,mres_t *,int,mpu_t *,int,int,int *);
int MPolicyCheckLimit(int,int,int,int,mpu_t *,mpu_t *,mpu_t *,int *);
int MSchedProcessJobs(char *,int *,int *);
int MSchedUpdateStats(void);
int MSchedTest(void);
int MSchedDiag(msched_t *,char *,int,int);
int MSchedProcessConfig(msched_t *,char *);
int MSchedLoadConfig(char *);
int MSchedConfigShow(msched_t *,int,char *,int);
int MSchedAToString(msched_t *,int,char *,int);


/* sim object */

int MSimInitialize(void);
int MSimShow(msim_t *,char *,int);
int MSimProcessEvents(int *);
int MSimSummarize(void);
int MSimProcessOConfig(msim_t *,int,int,double,char *,char **);
int MSimSetDefaults(void);
int MSimRMGetInfo(void);
int MSimMaintainWorkload(void);
int MSimGetWorkload(void);
int MSimInitializeWorkload(void);
int MSimJobSubmit(long,mjob_t **,void *,int);
int MSimJobStart(mjob_t *);
int MSimJobResume(mjob_t *);
int MSimJobModify(mjob_t *,char *,char *,char *,int *);
int MSimJobRequeue(mjob_t *);
int MSimJobSuspend(mjob_t *);
int MSimJobCheckpoint(mjob_t *);
int MSimJobTerminate(mjob_t *,int);
int MSimJobCancel(mjob_t *);
int MSimGetResources(char *,char *,char *);
int MSimLoadWorkloadCache(char *,char *,int *);
int MSimJobCreateName(char *,mrm_t *);



/* RM object */

int MRMInitialize(void);
int MRMClusterQuery(int *,int *);
int MRMWorkloadQuery(int *,int *);
int MRMJobStart(mjob_t *,char *Msg,int *);
int MRMJobCancel(mjob_t *,char *,int *);
int MRMCreate(char *,mrm_t *);
int MRMSetDefaults(mrm_t *);
int MRMOConfigShow(mrm_t *,int,int,char *);
int MRMShow(mrm_t *,char *,int,int);
int MRMFind(char *,mrm_t **);
int MRMDestroy(mrm_t **);
int MRMFinalizeCycle(void);
int MRMProcessConfig(mrm_t *,char *);
int MRMCheckConfig(mrm_t *);
int MRMJobGetProximateMNL(mjob_t *,mrm_t *,mnodelist_t,mnodelist_t,long,int,char *,int *);
int MRMCheckEvents(void);
int MRMJobSubmit(char *,mrm_t *,mjob_t **,char *,char *,int *);
int MRMJobSuspend(mjob_t *,char *,int *);
int MRMJobResume(mjob_t *,char *,int *);
int MRMJobCheckpoint(mjob_t *,int,char *,int *);
int MRMJobMigrate(mjob_t *,mnalloc_t *,int *);
int MRMJobModify(mjob_t *,char *,char *,char *,int *);
int MRMJobPreLoad(mjob_t *,char *,int);
int MRMReqPreLoad(mreq_t *);
int MRMJobPostLoad(mjob_t *,short *,mrm_t *);
int MRMJobPostUpdate(mjob_t *,short *,enum MJobStateEnum,mrm_t *);
int MRMJobStage(mjob_t *);
int MRMJobPreUpdate(mjob_t *);
int MRMJobRequeue(mjob_t *,mjob_t **,int *);
int MRMSetFailure(mrm_t *,int,char *);

int MRMLoadModules(void);

int MPBSLoadModule(mrmfunc_t *);
int MSGELoadModule(mrmfunc_t *);
int MLLLoadModule(mrmfunc_t *);
int MWikiLoadModule(mrmfunc_t *);
int MSSSLoadModule(mrmfunc_t *);
int MLSFLoadModule(mrmfunc_t *);

int MRMProcessOConfig(mrm_t *,int,int,double,char *,char **);
int MRMLoadConfig(char *);
int MRMGetInfo(void);
int MRMAdd(char *,mrm_t **);
int MRMConfigShow(mrm_t *,int,char *,int);
int MRMSetAttr(mrm_t *,int,void **,int,int);
int MRMAToString(mrm_t *,int,char *,int);
int MRMNodePreLoad(mnode_t *,int,mrm_t *);
int MRMNodePostLoad(mnode_t *);
int MRMNodePostUpdate(mnode_t *,int);
int MRMNodePreUpdate(mnode_t *,int,mrm_t *);



/* RMS interface object */

int MRMSInitialize(void);
int MRMSJobAllocateResources(mjob_t *,mreq_t *,mnalloc_t *,int,int *,int *,char *,int,int *,mnalloc_t *A[MAX_MREQ_PER_JOB],int *,int *);
int MRMSSelectAdjacentNodes(int,int,mnalloc_t *,mnalloc_t A[][MAX_MNODE]);
int MRMSQueryJob(mjob_t *,short *,int *);
int MRMSJobControl(mjob_t *,char *,char *,int *);
int MRMSJobSubmit(char *, mrm_t *, mjob_t **, int *);



/* BF object */

int MBFPreempt(mjob_t **,int,nodelist_t,time_t,int,int,mpar_t *);
int MBFFirstFit(mjob_t **,int,nodelist_t,time_t,int,int,mpar_t *);
int MBFBestFit(mjob_t **,int,nodelist_t,time_t,int,int,mpar_t *);
int MBFGreedy(mjob_t **,int,nodelist_t,time_t,int,int,mpar_t *);
int MBFGetWindow(int *,int *,nodelist_t,long *,long,mpar_t *,char *,char *,char *,int,int,unsigned long,mcres_t *,char *,char *,char *,char *);



/* wiki interface object */

int MWikiJobLoad(char *,char *,mjob_t *,short *,mrm_t *);
int MWikiDoCommand(char *,int,long,int,char *,char **,long *,int *);
int MWikiTestNode(char *);
int MWikiTestJob(char *);



/* util object */

int MUThread(int (*)(),long,int *,int,int *,...);
int MUStrCat(char *,char *,int);
int MUStrDup(char **,char *);
int MUMemCCmp(char *Data,char,int);
int MUFree(char **);
int MUCmpFromString(char *,int *);
int MUParseComp(char *,char *,int *,char *);
int MUGetPair(char *,const char **,int *,char *,int,int *,char *,int);
int MUBoolFromString(char *,int);
int MUGetIndex(char *,const char **,int,int);
int MUSScanF(char *,char *,...);
int MUStrCpy(char *,char *,int);
int MUStrToLower(char *);
int MUStrToUpper(char *,char *,int);
char *MUStrChr(char *,char);
int MUGetTime(time_t *,enum MTimeModeEnum,msched_t *);
char *MUPrintBuffer(char *,int);
int MUSleep(long);
int MUNumListGetCount(long,mnuml_t *,mnuml_t *,int,int *);
int MCResAdd(mcres_t *,mcres_t *,mcres_t *,int,int);
int MCResRemove(mcres_t *,mcres_t *,mcres_t *,int,int);
int MUCResIsNeg(mcres_t *);
int MUBuildPList(mcfg_t *,char **);
int MUCResGetMin(mcres_t *,mcres_t *,mcres_t *);
int MUCResGetMax(mcres_t *,mcres_t *,mcres_t *);
int MUNumListFromString(mnuml_t *,char *,int);
char *MUCAListToString(mnuml_t *,mnuml_t *,char *);
char *MUListAttrs(int,int);
int MUShowCopy(void);
int MFULock(char *,char *);
int MUSetEnv(char *,char *);
int MUGetMS(struct timeval *,long *);
time_t MUTimeFromString(char *);
int MUStringToE(char *,long *);
int MUReadPipe(char *,char *,int);
int MUClearChild(int *);
int MUCompare(int,int,int);
char *MULToTString(long);
char *MUStrTok(char *,char *,char **);
char *MUStrTokE(char *,char *,char **);
int MUGetOpt(int *,char **,char *,char **,int *);
long MURSpecToL(char *,enum MValModEnum,enum MValModEnum);
char *MULToRSpec(long,int,char *);
int MUCResFromString(mcres_t *,char *);
char *MUCResToString(mcres_t *,long,int,char *);
int MUStringUnpack(char *,char *,int);
int MUStringPack(char *,char *,int);
int MUStrNCmpL(char *,char *,int);
char *MUStrStrL(char *,char *);
int MUGetTokens(char **,short *,char *,char **);
int MUGetHash2(char *);
int MUDStatInitialize(dstat_t *,int);
int MUDStatIsEnabled(dstat_t *);
int MUDStatAdd(dstat_t *,char *);
int MUNLGetMinAVal(mnalloc_t *,int,mnode_t **,void **);
int MUNLGetMaxAVal(mnalloc_t *,int,mnode_t **,void **);
int MUSNInit(char **,int *,char *,int);
int MUSNPrintF(char **,int *,char *,...);
int MUStrNCat(char **,int *,char *);
int MUSNCat(char **,int *,char *);
char *MUMAList(int,int *,int);
int MUMAGetBM(int,char *,int);
int MUMAGetIndex(int,char *,int);
int MUMAMAttrFromLine(int,char *,int,int *,int);
int MUMAFromList(int,char **,int);
int MUMAFromString(int,char *,int);
char *MUMAToString(int,char,int *,int);
int MUNLCopy(mnalloc_t *,mnodelist_t,int,int);
int MUREToList(char *,int,int,short *,int *,char *);
char *MUUIDToName(uid_t);
char *MUGIDToName(gid_t);
gid_t MUGIDFromUID(uid_t);
uid_t MUUIDFromName(char *);
gid_t MUGIDFromName(char *);
int MUGNameFromUName(char *,char *);
char *MUBListAttrs(int,int);
char *MUSNCTime(long *Time);
int MSubmitTimeComp(mjob_t *,mjob_t *);
char *MUFindEnv(char *,int *);
int MUUnsetEnv(char *);
int MUPurgeEscape(char *);
int MUGetPeriodStart(long,long,int,int,long *);
char *MUBMToString(unsigned long,const char **,char,char *,char *);
int MUBMFromString(char *,const char **,unsigned long *);
int MUBMOR(int *,int *,int);
int MUBMAND(int *,int *,int);
int MUBMIsClear(int *,int);
int MUTMToHostList(short *,char **,mrm_t *);
unsigned long MUGetHash(char *);
int MUGetMAttr(int,char *,int,int *,int);
char *MUShowIArray(const char *,int,int);
char *MUShowLArray(const char *,int,long);
char *MUShowSArray(const char *,int,char *);
char *MUShowFArray(const char *,int,double);
int MUShowSSArray(const char *,char *,char *,char *);
char *MUBStringTime(long);  
char *MUStrStr(char *,char *,int,mbool_t,mbool_t);
int MUStrNCmpCI(char *,char *,int);
char *MUNumListToString(mnuml_t *,mnuml_t *,char *,char *,int);
char *MULToDString(time_t *);
char *MAttrFind(char *,int,int *,int,char **);
int MAttrSubset(int *,int *,int,int);
int MAVPToXML(const char *,const char *,mamolist_t *,mamolist_t *,mamolist_t *,char *,char *);
int MXMLToAVP(const char *,mamolist_t *,char *);
int MOSSyslog(int,char *,...);
pid_t MOSGetPID(void);
int MOSSetGID(gid_t);
int MOSSetUID(uid_t);
uid_t MOSGetUID(void);
uid_t MOSGetEUID(void);
int MOSGetHostName(char *,char *,unsigned long *);
int MOSSyslogInit(msched_t *);
int MUSystemF(char *,int,int *);
int MUCheckAuthFile(msched_t *,char *,int *,int);
char *MUCResRatioToString(mcres_t *,mcres_t *,mcres_t *,int);
int MMovePtr(char **,char **);
int MSDataCreate(msdata_t **);
int MSDataDestroy(msdata_t **);
int MDataGetEAvailTime(msdata_t *,long *);

/* NOTE:  G2 prototype header not included */

int G2XMLCreateE(mxml_t **,char *);
int G2XMLDestroyE(mxml_t **);
int G2XMLSetAttr(mxml_t *,char *,void *,int);
int G2XMLSetVal(mxml_t *,void *,int);
int G2XMLAddE(mxml_t *,mxml_t *);
int G2XMLToString(mxml_t *,char *,int,char **,int);
int G2XMLGetAttr(mxml_t *,char *,int *,char *);
int G2XMLGetChild(mxml_t *,char *,int *,mxml_t **);
int G2XMLFromString(mxml_t **,char *,char **);
int MUIXMLSetStatus(mxml_t *,int,char *,int);
char *MUURLCreate(char *,char *,char *,int,char *,int);
int MUURLParse(char *,char *,char *,char *,int,int *,mbool_t);
int MUHProcessRequest(msocket_t *,char *);
int MUHInitialize(void);



/* log object */

int MLogInitialize(char *,int,int);
int MLogOpen(int);
int MLogRoll(char *,int,int,int);
char *MLogGetTime(void);
void MLogLevelAdjust(int);
int MLogShutdown(void);

#ifndef __MTEST
int DPrint(char *Format, ...);
#endif /* __MTEST */



/* acl management object */

int MACLLoadConfig(macl_t *ACL,char **ACLList,int,int);
int MACLSet(macl_t *,int,void *,int,int,long,int);
int MACLClear(macl_t *ACL,int);
int MACLLoadConfigLine(macl_t *ACL,char *);
int MACLGet(macl_t *,int,void **,int *);
int MACLCheckAccess(macl_t *,macl_t *,char *,int *);
char *MACLListShow(macl_t *,int,int,char *);
char *MACLShow(macl_t,int,int);



/* file util object */

char *MFULoad(char *,int,int,int *,int *);
int MFUGetInfo(char *,long *,long *,int *);
int MFUCacheInitialize(time_t *);
int MFUIndexInitialize(char *, int,int *);
int MFUGetRecord(char *,char *,char **,int *,int *);
int MFUPutRecord(char *,char *,char *,int *);
int MFURemoveRecord(char *,char *,int *);
int MFUGetModifyTime(char *,long *);
int MFUCacheInvalidate(char *);
int MFUGetCurrentIndex(char *,int *,int);
int MFUGetAttributes(char *,int *,long *,long *,uid_t *,int *,int *);
int MFURename(char *,char *);



/* trace object */

int MTraceLoadWorkload(char *,int *,mjob_t *,int,int *);
int MTraceLoadResource(char *,int *,mnode_t *,int *);
int MTraceLoadComputeNode(char *,mnode_t *,int);
int MTraceLoadNetwork(char *,mnode_t *, int);
int MTraceLoadHSM(char *,mnode_t *,int);
int MTraceGetWorkloadVersion(char *,int *);
int MTraceGetResourceVersion(char *,int *);
int MTraceBuildResource(mnode_t *,int,char *,int);



/* ui util object */

int MUISMsgClear(msocket_t *);
int MUISMsgAdd(msocket_t *,char *);



/* client object */

int MClientLoadConfig(void);
int MClientProcessConfig(char *);
int MCDoCommand(char *,int,int,char *,char *);
int MCSendRequest(msocket_t *);



/* local interface */

int MLocalInitialize(void);
int MLocalNodeFilter(mjob_t *,mnode_t *,long);
int MLocalGetNodePriority(mjob_t *,mnode_t *);
int MLocalJobAllocateResources(mjob_t *,mreq_t *,mnalloc_t *,mulong,int,int *,int *,char *,int,int *,mnalloc_t *B[MAX_MREQ_PER_JOB],int *,int *);
int MLocalJobCheckNRes(mjob_t *,mnode_t *,long);
int MLocalJobDistributeTasks(mjob_t *,mrm_t *,mnalloc_t *,short *);
int MLocalNodeInit(mnode_t *);
int MLocalJobInit(mjob_t *);
int MLocalCheckFairnessPolicy(mjob_t *,long,char *);
int MLocalCheckRequirements(mjob_t *,mnode_t *,long);
int MLocalQueueScheduleIJobs(int *,mpar_t *);



/* other */

int MetaCtlInitialize(char *,char *,char *);
int MetaCtlCommit(char *,char *,char *);
int MetaCtlList(char *,char *,char *);
int MetaCtlQuery(char *,char *,char *);
int MetaCtlRegister(char *,char *,char *);
int MetaCtlRemove(char *,char *,char *);
int MetaCtlSet(char *, char *,char *);
int MetaCtlSubmit(char *,char *,char *);
int MetaStoreCompletedJobInfo(mjob_t *);
int MUNLFromTL(mnalloc_t *,short *,int *);



/* app sim interface */

char *MASGetName(void *);
int MASGetDriver(void **,char *,int);
int MGResFind(char *,int,xres_t **);



/* general object */

int MPrioConfigShow(int,int,char *);



/* X prototypes */

int XGetClientInfo(void *,msocket_t *,char *);
int XPBSInitialize(void *,mrm_t *);
int XPBSNMGetData(void *,mnode_t *, mrm_t *);
int XInitialize(mx_t *,char *,int *,char **,char *);
int XLoadClientKeys(void *);
int XUIHandler(void *,msocket_t *,char *,int);
int XUIJobCtl(void);
int XUIMetaCtl(void);
int XUIResCtl(void);
int XShowConfig(void *,char *);
int XRMInitialize(void *,mrm_t *);
int XRMResetState(void *,mrm_t *);
int XRMVerifyData(void *,mrm_t *,char *);
int XRMJobResume(void);
int XRMJobSuspend(void);
int XUpdateState(void);
int XMetaStoreCompletedJobInfo(void);
int XAllocMachinePrio(void);
int XAllocLoadBased(void);
int XJobProcessWikiAttr(void *,mjob_t *,char *);
int XJobDestroy(void *,mjob_t **,int);
int XJobGetStartPriority(void *,mjob_t *, double *);
int XLL2JobLoad(void *,mjob_t *,void *);
int XLL2JobUpdate(void *,mjob_t *,void *);
int XLL2NodeLoad(void *,mnode_t *,void *);
int XLL2NodeUpdate(void *,mnode_t *,void *);
int XQueueScheduleJobs(void *,int *,int);

/* END moab-proto.h */

