/*
*/

#ifndef __MPROF_PROTO_H__
#define __MPROF_PROTO_H__

int main(int, char **);
int MPInitialize(void);
int OpenPlotFile(char *);
int MPReadArgs(int, char **);
int PJobCountDistribution(void);
int PBFCountDistribution(void);
int PQueueDistribution(void);
int PAccuracyDistribution(int);
int PNSRequestDistribution(void);
int PNSRunDistribution(void);
int PQueueTimeDistribution(void);
int PJobLengthAccuracy(void);
int PXFactor(void);
int PMaxXFactor(void);
int PJobEfficiency(void);
int PQOSSuccessRate(void);
int ProfileMemoryUsage(void);
int ProfileUser(void);
int ProfileGroup(void);
int ProfileAccount(void);
int UPSComp(mgcred_t **, mgcred_t **);
int GPSComp(mgcred_t *, mgcred_t *);
int APSComp(mgcred_t *, mgcred_t *);
int MPLoadTrace(char *);
int GetComp(char *);

#endif /* __MPROF_PROTO_H__ */
