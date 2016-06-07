/* HEADER */

#ifndef __M_PROTO_H__
#define __M_PROTO_H__

#include "moab-proto.h"

int MSDRGetSystemConfig(void);
int __OMCLoadArgs(char, char *, enum MSvcEnum);
int __OMCProcessArgs(int, char **, enum MSvcEnum *);
int OMCShowUsage(enum MSvcEnum);
int UIResCreate(char *, char *, int, char *, long *);
int UIResDestroy(char *, char *, int, char *, long *);
int UIResList(char *, int, char *, int, int, char *, long *);
int UIResShow(char *, char *, int, char *, long *);
int DiagnoseFairShare(char *, long *, int);
int DiagnoseQOS(char *, long *, char *);
int DiagnoseClass(char *, long *, char *);
int ResDiagnose(char *, long *, int, char *, int);
int SetJobHold(char *, char *, int, char *, long *);
int ReleaseJobHold(char *, char *, int, char *, long *);
int ShowJobHold(char *, char *, int, char *, long *);
int UIStatClear(char *, char *, int, char *, long *);
int SetJobSystemPrio(char *, char *, int, char *, long *);
int SetJobUserPrio(char *, char *, int, char *, long *);
int UIJobGetStart(char *, char *, int, char *, long *);
int UIResShowAvail(msocket_t *, int, char *);
int UIShowConfig(char *, char *, int, char *, long *);
int UIJobStart(char *, char *, int, char *, long *);
int UINodeStatShow(int, char *, long *);
int UIChangeParameter(char *, char *, int, char *, long *);
int MigrateJob(char *, char *, int, char *, long *);
int UIShowEstStartTime(char *, char *, int, char *, long *);

#endif /* __M_PROTO_H__ */
