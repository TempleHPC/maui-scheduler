/* HEADER */

#ifndef __MCLIENT_PROTO_H__
#define __MCLIENT_PROTO_H__

int main(int, char **);

int Initialize(int, char **, int *);
int OMCShowUsage(int);

int MCSendRequest(msocket_t *);
int MCShowCStats(char *, int, int);
int MCClusterShow(char *);
int MCQueueShow(mxml_t *);
int MCShowReservation(char *);
int MCShowReservedNodes(char *);
int MCShowJobReservation(char *);
int MCSchedCtl(char *);
int MCDiagnose(char *);
int MCShowIdle(char *);
int MCShowRun(char *);
int MCShowJobHold(char *);
int MCShowStats(char *, int);
int MCShowSchedulerStatistics(char *);
int MCResetStats(char *);
int MCResCreate(char *);

int MCReleaseReservation(char *);
int MCSetJobHold(char *);
int MCReleaseJobHold(char *);
int MCSetJobSystemPrio(char *);
int MCSetJobUserPrio(char *);
int MCStopScheduling(char *);
int MCResumeScheduling(char *);
int MCSetJobDeadline(char *);
int MCReleaseJobDeadline(char *);
int MCShowJobDeadline(char *);
int MCShowEStart(char *);
int MCSetJobQOS(char *);
int MCShowGrid(char *);
int MCShowBackfillWindow(char *);
int MCShowConfig(char *);
int MCShowUserStats(char *, int);
int MCShowGroupStats(char *, int);
int MCShowAccountStats(char *, int);
int MCRunJob(char *);
int MCCancelJob(char *);
int MCShowNodeStats(char *);
int MCChangeParameter(char *);
int MCMigrateJob(char *);
int MCBShowState(char *);
int MCShowEstimatedStartTime(char *);
int MCShowQueue(char *);
int showTasksPerUser(char *);
int MCShowQ(char *, int);
int LoadConfig(char *, char *);
int PSDedicatedComp(cstats *, cstats *);
int MCUPSDedicatedComp(mgcred_t *, mgcred_t *);
int MCGPSDedicatedComp(mgcred_t *, mgcred_t *);
int MCAPSDedicatedComp(mgcred_t *, mgcred_t *);
int MCQPSDedicatedComp(mqos_t *, mqos_t *);
int MCCPSDedicatedComp(mclass_t *, mclass_t *);

#endif /* __MCLIENT_PROTO_H__ */
