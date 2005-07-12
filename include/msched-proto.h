/*
*/
        
#ifndef __M_PROTO_H__
#define __M_PROTO_H__

#include "moab-proto.h"

int main(int,char **);
int SDRGetSystemConfig(void);
int ServerProcessRequests(void);
int ServerLoadSignalConfig(void);
int ServerShowCopy(void);
int ServerUpdate(void);
int ServerProcessArgs(int,char **,int);
int StringToComp(char *,int *);
int ServerInitializeLog(int,char **);
int ServerGetAuth(char *,long *);
int MServerConfigShow(char *,int,int);
int ServerDemonize(void);
int ServerAuthenticate(void);
int ServerSetCreds(int,int);
int CrashMode(int);
void ServerRestart(int);
int ReloadConfig(int);
int ServerShowUsage(char *);
int UIProcessClients(msocket_t *,long);
int UIFormatShowAllJobs(char *,char *,int); 
int UIFormatHShowAllJobs(char *,char *);
int MUISProcessRequest(msocket_t *,char *);
int UIProcessCommand(msocket_t *);
int UIClusterShow(char *,char *,int,char *,long *);
int UIResCreate(char *,char *,int,char *,long *); 
int UIResDiagnose(char *,long *,int,char *,int); 
int UIResDestroy(char *,char *,int,char *,long *); 
int UIResList(char *,int,char *,int,int,char *,long *);
int UIResShow(char *,char *,int,char *,long *); 
int MUISchedCtl(char *,char *,int,char *,long *);
int UIDiagnose(char *,char *,int,char *,long *); 
int DiagnoseFairShare(char *,long *,int); 
int UIDiagnosePriority(char *,long *,mpar_t *);
int UIQueueDiagnose(char *,long *,int,int); 
int DiagnoseQOS(char *,long *,char *); 
int DiagnoseClass(char *,long *,char *); 
int MUIJobDiagnose(char *,long *,int,char *,int);
int UINodeDiagnose(char *,long *,int,char *,int);
int MParDiagnose(char *,long *,char *);
int UIUserDiagnose(char *,long *,char *,int);
int UIGroupDiagnose(char *,long *,char *,int);
int UIAcctDiagnose(char *,long *,char *,int);
int ResDiagnose(char *,long *,int,char *,int);
int UIQueueShowBJobs(char *,long *,mpar_t *,char *);
int UIQueueShowEJobs(char *,long *,mpar_t *,char *);
int UIQueueShowAJobs(char *,long *,mpar_t *,int, char *);
int UIQueueShowAllJobs(char *,long *,mpar_t *, char *);        
int SetJobHold(char *,char *,int,char *,long *); 
int ReleaseJobHold(char *,char *,int,char *,long *); 
int ShowJobHold(char *,char *,int,char *,long *); 
int UIStatClear(char *,char *,int,char *,long *); 
int UIStatShow(char *,char *,int,char *,long *); 
int SetJobSystemPrio(char *,char *,int,char *,long *); 
int SetJobUserPrio(char *, char *,int,char *,long *); 
int UIJobGetStart(char *,char *,int,char *,long *); 
int UIShowGrid(char *,char *,int,char *,long *); 
int ShowBackfillWindow(char *,char *,int,char *,long *); 
int UIShowConfig(char *,char *,int,char *,long *); 
int UIShowCStats(char *,int,char *,long *); 
int UIJobShow(char *,char *,int,char *,long *);
int UIParShowStats(char *,char *,long *);
int UINodeShow(char *,char *,int,char *,long *);
int UIJobStart(char *,char *,int,char *,long *); 
int MUIJobPreempt(mjob_t *,char *,char *,int,char *);
int MUIJobResume(mjob_t *,char *,char *,char *);
int UIJobCancel(char *,char *,int,char *,long *);
int UINodeStatShow(int,char *,long *);
int UIChangeParameter(char *,char *,int,char *,long *); 
int MigrateJob(char *,char *,int,char *,long *);
int UIShowEstStartTime(char *,char *,int,char *,long *); 
int UIQueueShow(char *,char *,int,char *,long *); 
int ConfigShow(char *,int,int);
int UHProcessRequest(msocket_t *,char *);

#endif /* __M_PROTO_H__ */

