/*
*/

#include "moab.h"
#include "mprof-proto.h"
#include "msched-proto.h"

#define MAX_TRACES 8192
#define MAX_HOUR_SLOTS 15000
#define PMAX_SCALE 128
#define MAX_MEMORY_SIZE 20

#define DEFAULT_BEGINTIME 0
#define DEFAULT_ENDTIME MAX_MTIME

#define PROF_MAX_JOBSIZE 512
#define PROF_TIMEINT_COUNT 1345
#define PROF_TIMEINT_LENGTH 450

#define DEFAULT_TRACE_FLAGS 0

/* Standard Deviation Equation:

  (Sum((xi - X)^2)/(n - 1))^.5

*/

typedef struct {
    time_t FirstQueueTime;
    time_t EarliestQueueTime;
    time_t EarliestStartTime;
    time_t LatestQueueTime;
    time_t LatestStartTime;
    time_t LatestCompletionTime;

    int QueueTimeDistProfile[MAX_HOUR_SLOTS];
    double QueueJobDepthProfile[MAX_HOUR_SLOTS];
    double QueuePSDepthProfile[MAX_HOUR_SLOTS];
    double ActiveJobDepthProfile[MAX_HOUR_SLOTS];
    double ActiveNodeDepthProfile[MAX_HOUR_SLOTS];
    double ActivePSDepthProfile[MAX_HOUR_SLOTS];
    int CompletionTimeDistProfile[MAX_HOUR_SLOTS];
    int TimeQueuedDistProfile[MAX_HOUR_SLOTS];
    int XFactorDistProfile[1001];
    int JobAccuracy[101];
    int TotalAccSum;
    int TotalJobCount;
    double TotalPS;

    int Acc5_99Sum;
    int Acc5_99Count;

    int JobSizeDist[PROF_MAX_JOBSIZE + 1];
    double JobEfficDist[PROF_MAX_JOBSIZE + 1];
    int JobLengthDist[PROF_TIMEINT_COUNT + 1];
    int JobPHDist[1024];

    double PSSizeDist[PROF_MAX_JOBSIZE + 1];
    double PSLengthDist[PROF_TIMEINT_COUNT + 1];
    double PSPHDist[1024];
} mprof_t;

mprof_t SystemProfile;

extern mclass_t MClass[];
extern mrm_t MRM[MAX_MRM];
extern mam_t MAM[MAX_MAM];
extern mre_t MRE[MAX_MRES << 2];
extern mres_t *MRes[MAX_MRES];
extern mrange_t MRange[MAX_MRANGE];
extern mckpt_t MCP;
extern sres_t SRes[];
extern sres_t OSRes[];

mrmfunc_t MRMFunc[MAX_MRMTYPE];

char TraceFile[MAX_MNAME];
char PlotFile[MAX_MNAME];

int PJobCountDistribution();
int PPSRequestDistribution();
int PPSRunDistribution();
int PQueueTimeDistribution();
int PJobLengthAccuracy();
int PXFactor();
int PMaxXFactor();
int ProfileMemoryUsage();
int PAccuracyDistribution();
int PJobEfficiency();
int PQOSSuccessRate();

int (*Function[])() = {PJobCountDistribution, PPSRequestDistribution,
                       PPSRunDistribution,    PQueueTimeDistribution,
                       PJobLengthAccuracy,    PXFactor,
                       PMaxXFactor,           ProfileMemoryUsage,
                       PAccuracyDistribution, PJobEfficiency,
                       PQOSSuccessRate,       NULL};

char *ProfileType[] = {
    "JobCount",      "PSRequest",      "PSRun",    "QueueTime",
    "WCAccuracy",    "XFactor",        "MemUsage", "WCAccuracyD",
    "JobEfficiency", "QOSSuccessRate", 0};

enum {
    prJobCount = 0,
    prPSRequest,
    prPSRun,
    prQueueTime,
    prWCAccuracy,
    prXFactor,
    prMaxXFactor,
    prMemUsage,
    prWCAccuracyD,
    prJobEfficiency,
    prQOSSuccessRate
};

FILE *plotfp;

extern mlog_t mlog;
extern mcfg_t MCfg[];

typedef struct {
    char UserNameList[64][MAX_MNAME];
    char GroupNameList[64][MAX_MNAME];
    char AccountNameList[64][MAX_MNAME];
    char QOSList[MAX_MLINE];
    long StartTime;
    long EndTime;
    char Host[MAX_MNAME];
} mpcons_t;

int ProfileFunction;

extern mattrlist_t MAList;

mpcons_t Profile;

extern mgcred_t *MUser[MAX_MUSER + MAX_MHBUF];
extern mgcred_t MGroup[MAX_MGROUP + MAX_MHBUF];
extern mgcred_t MAcct[MAX_MACCT + MAX_MHBUF];
extern mnode_t *MNode[MAX_MNODE];

extern mstat_t MStat;
extern mqos_t MQOS[MAX_MQOS];

extern msched_t MSched;
msim_t MSim;

char DistFile[MAX_MNAME];

int ProfileMode;

unsigned long JobsatMemorySize[MAX_MEMORY_SIZE];
unsigned long NodesatMemorySize[MAX_MEMORY_SIZE];
double PSRequestatMemorySize[MAX_MEMORY_SIZE];
double PSRunatMemorySize[MAX_MEMORY_SIZE];

int TotalJobs;
unsigned long TotalNodes;
double TotalPSRequest;
double TotalPSRun;

extern mjob_t *MJob[];
extern mframe_t MFrame[MAX_MFRAME];
extern mpar_t MPar[];
extern mjobl_t MJobName[MAX_MJOB + MAX_MHBUF];

extern mx_t X;

extern const char *MNodeState[];

#include "mprof-stub.c"

int main(

    int argc, char *argv[])

{
    mlog.logfp = stderr;

    DBG(2, fCORE) DPrint("main()\n");

    MPInitialize();

    MPReadArgs(argc, argv);

    MSched.Mode = msmProfile;

    MStatProfInitialize(&MStat.P);

    if (TraceFile[0] == '\0') {
        fprintf(stderr, "USAGE ERROR:  (tracefile not specified)\n");

        exit(1);
    }

    MPLoadTrace(TraceFile);

    if (ProfileFunction == -1) {
        if (DistFile[0] != '\0') {
            PQueueDistribution();
        }

        PJobCountDistribution();

        PBFCountDistribution();

        PPSRequestDistribution();

        PPSRunDistribution();

        PQueueTimeDistribution();

        PJobLengthAccuracy();

        PXFactor();

        PMaxXFactor();

        ProfileMemoryUsage();

        PAccuracyDistribution(ProfileMode);

        PJobEfficiency();

        PQOSSuccessRate();
    } else {
        switch (ProfileFunction) {
            case prJobCount:

                PJobCountDistribution();

                break;

            case prPSRequest:

                PPSRequestDistribution();

                break;

            case prPSRun:

                PPSRunDistribution();

                break;

            case prQueueTime:

                PQueueTimeDistribution();

                break;

            case prWCAccuracy:

                PJobLengthAccuracy();

                break;

            case prXFactor:

                PXFactor();

                break;

            case prMaxXFactor:

                PMaxXFactor();

                break;

            case prMemUsage:

                ProfileMemoryUsage();

                break;

            case prWCAccuracyD:

                PAccuracyDistribution(ProfileMode);

                break;

            case prJobEfficiency:

                PJobEfficiency();

                break;

            case prQOSSuccessRate:

                PQOSSuccessRate();

                break;

            default:

                break;
        } /* END switch (ProfileFunction)     */
    }     /* END else (ProfileFunction == -1) */

    /* profile user and group */

    if (ProfileFunction == -1) {
        ProfileUser();

        ProfileGroup();

        ProfileAccount();
    } else {
        switch (ProfileFunction) {
            case prPSRequest:
            case prPSRun:
            case prJobCount:
            case prJobEfficiency:

                ProfileUser();

                ProfileGroup();

                ProfileAccount();

                break;

            default:

                break;
        }
    }

    return (SUCCESS);
} /* END main() */

int MPInitialize()

{
    int index;

    time_t tmpTime;

    const char *FName = "MPInitialize";

    DBG(2, fCORE) DPrint("%s()\n", FName);

    time(&tmpTime);
    MSched.Time = (long)tmpTime;

    MUBuildPList(MCfg, MParam);

    mlog.Threshold = 1;

    /* load state attribute values */

    for (index = 0; MNodeState[index] != NULL; index++)
        strcpy(MAList[eNodeState][index], MNodeState[index]);

    DistFile[0] = '\0';

    /* initialize plot values */

    MStat.P.MaxTime = DEFAULT_MAXTIME;
    MStat.P.MinTime = DEFAULT_MINTIME;
    MStat.P.TimeStepCount = DEFAULT_TIMESCALE;

    MStat.P.MaxNode = PROF_MAX_JOBSIZE;
    MStat.P.MinNode = DEFAULT_MINNODE;
    MStat.P.NodeStepCount = MDEF_NODESCALE;

    MStat.P.AccuracyScale = DEFAULT_ACCURACYSCALE;

    MStat.P.BeginTime = DEFAULT_BEGINTIME;
    MStat.P.EndTime = DEFAULT_ENDTIME;

    ProfileFunction = -1;

    Profile.QOSList[0] = '\0';
    ProfileMode = 0;

    /* set initial profile */

    Profile.UserNameList[0][0] = '\0';
    Profile.GroupNameList[0][0] = '\0';
    Profile.AccountNameList[0][0] = '\0';

    Profile.StartTime = 0;
    Profile.EndTime = MAX_MTIME;

    memset(MUser, 0, sizeof(MUser));
    memset(MGroup, 0, sizeof(MGroup));
    memset(MAcct, 0, sizeof(MAcct));

    memset(JobsatMemorySize, 0, sizeof(JobsatMemorySize));
    memset(NodesatMemorySize, 0, sizeof(NodesatMemorySize));
    memset(PSRequestatMemorySize, 0, sizeof(PSRequestatMemorySize));
    memset(PSRunatMemorySize, 0, sizeof(PSRunatMemorySize));

    memset(&SystemProfile, 0, sizeof(SystemProfile));

    SystemProfile.FirstQueueTime = MAX_MTIME;
    SystemProfile.EarliestQueueTime = MAX_MTIME;
    SystemProfile.EarliestStartTime = MAX_MTIME;

    TotalJobs = 0;
    TotalNodes = 0;
    TotalPSRequest = 0.0;
    TotalPSRun = 0.0;

    MPar[0].FSC.FSPolicy = FALSE;

    return (SUCCESS);
} /* END Initialize() */

int OpenPlotFile(

    char *FileName)

{
    const char *FName = "OpenPlotFile";

    DBG(3, fCORE) DPrint("%s(%s)\n", FName, FileName);

    if ((plotfp = fopen(FileName, "w+")) == NULL) {
        DBG(0, fCORE)
        DPrint("ERROR:    cannot open plotfile '%s', errno: %d\n", FileName,
               errno);

        perror("cannot open file");

        /* dump plot to stderr */

        plotfp = stderr;

        return (FAILURE);
    }

    return (SUCCESS);
} /* END OpenPlotFile() */

int MPReadArgs(

    int argc, char *argv[])

{
    int Flag;
    int index;

    extern int opterr;
    extern int optind;
    extern int optopt;
    extern char *optarg;

    char *ptr;

    const char *FName = "MPReadArgs";

    DBG(2, fCONFIG) DPrint("%s(%d,argv)\n", FName, argc);

    while ((Flag = getopt(argc, argv,
                          "a:A:b:c:d:D:e:fg:hH:mn:N:p:P:qQ:rRs:S:t:T:u:x?")) !=
           -1) {
        switch (Flag) {
            case 'a':

                MStat.P.AccuracyScale = (int)strtol(optarg, NULL, 0);

                if (MStat.P.AccuracyScale > MAX_ACCURACY) {
                    MStat.P.AccuracyScale = MAX_ACCURACY;

                    DBG(2, fCONFIG)
                    DPrint(
                        "ALERT:    accuracy scale reduced to MAXACCURACY "
                        "(%d)\n",
                        MAX_ACCURACY);
                } else if (MStat.P.AccuracyScale == 0) {
                    DBG(2, fCONFIG)
                    DPrint("ERROR:    invalid accuracy scale specified (%s)\n",
                           optarg);

                    exit(1);
                }

                DBG(2, fCONFIG)
                DPrint("INFO:     accuracy scale set to %ld\n",
                       MStat.P.AccuracyScale);

                break;

            case 'A':

                ptr = strtok(optarg, ":");

                index = 0;

                while (ptr != NULL) {
                    strcpy(Profile.AccountNameList[index], ptr);

                    fprintf(stdout, "profiling account '%s'\n", ptr);

                    index++;

                    ptr = strtok(NULL, ":");
                }

                Profile.AccountNameList[index][0] = '\0';

                break;

            case 'b':

                /* begin time */

                if (MUStringToE(optarg, &Profile.StartTime) != SUCCESS) {
                    fprintf(stderr,
                            "ERROR:    invalid BeginTime specified, '%s'\n",
                            optarg);

                    exit(1);
                }

                DBG(2, fCONFIG)
                DPrint("INFO:     BeginTime set to %ld\n", Profile.StartTime);

                break;

            case 'c':

                MStat.P.TraceCount = atoi(optarg);

                DBG(2, fCONFIG)
                DPrint("INFO:     TraceCount set to %ld\n", MStat.P.TraceCount);

                break;

            case 'd':

                strcpy(DistFile, optarg);

                DBG(2, fCONFIG)
                DPrint("INFO:     DistFile set to %s\n", DistFile);

                break;

            case 'D':

                mlog.Threshold = atoi(optarg);

                DBG(2, fCONFIG)
                DPrint("INFO:     LOGLEVEL set to %d\n", mlog.Threshold);

                break;

            case 'e':

                /* end time */

                if (MUStringToE(optarg, &Profile.EndTime) != SUCCESS) {
                    fprintf(stderr,
                            "ERROR:    invalid EndTime specified, '%s'\n",
                            optarg);

                    exit(1);
                }

                DBG(2, fCONFIG)
                DPrint("INFO:     EndTime set to %ld\n", Profile.EndTime);

                break;

            case 'f':

                MSched.TraceFlags |= (1 << tfFixCorruption);

                break;

            case 'g':

                ptr = strtok(optarg, ":");

                index = 0;

                while (ptr != NULL) {
                    strcpy(Profile.GroupNameList[index], ptr);

                    fprintf(stdout, "profiling group '%s'\n", ptr);

                    index++;

                    ptr = strtok(NULL, ":");
                }

                Profile.GroupNameList[index][0] = '\0';

                break;

            case 'H':

                strcpy(Profile.Host, optarg);

                DBG(2, fCONFIG)
                DPrint("INFO:     profile host set to %s\n", Profile.Host);

                break;

            case 'm':

                ProfileMode |= (1 << mMatrix);

                DBG(2, fCONFIG) DPrint("INFO:     matrix mode enabled\n");

                break;

            case 'n':

                MStat.P.NodeStepCount = atoi(optarg);

                if (MStat.P.NodeStepCount > PMAX_SCALE) {
                    MStat.P.NodeStepCount = PMAX_SCALE;

                    fprintf(stderr, "WARNING:  node step count reduced to %d\n",
                            PMAX_SCALE);
                }

                DBG(2, fCONFIG)
                DPrint("INFO:     node step count set to %ld\n",
                       MStat.P.NodeStepCount);

                break;

            case 'N':

                MStat.P.MaxNode = atoi(optarg);

                DBG(2, fCONFIG)
                DPrint("INFO:     max node set to %ld\n", MStat.P.MaxNode);

                break;

            case 'o':

                MStat.P.NodeStepSize = atoi(optarg);

                break;

            case 'p':

                strcpy(PlotFile, optarg);

                DBG(2, fCONFIG)
                DPrint("INFO:     plotfile set to %s\n", PlotFile);

                OpenPlotFile(PlotFile);

                break;

            case 'P':

                for (index = 0; ProfileType[index] != NULL; index++) {
                    if (!strcmp(ProfileType[index], optarg)) break;
                }

                if (ProfileType[index] == NULL) {
                    fprintf(stderr,
                            "ERROR:    invalid profile specified (%s)\n",
                            optarg);

                    fprintf(stderr, "valid profiles:  ");

                    for (index = 0; ProfileType[index] != NULL; index++) {
                        fprintf(stderr, "%s ", ProfileType[index]);
                    }

                    fprintf(stderr, "\n\n");

                    exit(1);
                } else {
                    ProfileFunction = index;
                }

                break;

            case 'q':

                ProfileMode |= (1 << mSystemQueue);

                break;

            case 'Q':

                MUStrCpy(Profile.QOSList, optarg, sizeof(Profile.QOSList));

                break;

            case 'r':

                ProfileMode |= (1 << mUseRunTime);

                break;

            case 'R':

                ProfileMode |= (1 << mUseRemoved);

                DBG(2, fCONFIG) DPrint("INFO:     using removed jobs\n");

                break;

            case 's':

                MStat.P.TimeStepCount = atoi(optarg);

                if (MStat.P.TimeStepCount > PMAX_SCALE) {
                    MStat.P.TimeStepCount = PMAX_SCALE;

                    fprintf(stderr, "WARNING:  time step count reduced to %d\n",
                            PMAX_SCALE);
                }

                DBG(2, fCONFIG)
                DPrint("INFO:     time step count set to %ld\n",
                       MStat.P.TimeStepCount);

                break;

            case 'S':

                MStat.P.TimeStepSize = atoi(optarg);

                break;

            case 't':

                strcpy(TraceFile, optarg);

                DBG(2, fCONFIG)
                DPrint("INFO:     tracefile set to %s\n", TraceFile);

                break;

            case 'T':

                MStat.P.MaxTime = atoi(optarg);

                DBG(2, fCONFIG)
                DPrint("INFO:     max time set to %ld\n", MStat.P.MaxTime);

                break;

            case 'u':

                ptr = strtok(optarg, ":");

                index = 0;

                while (ptr != NULL) {
                    strcpy(Profile.UserNameList[index], ptr);

                    fprintf(stdout, "profiling user '%s'\n", ptr);

                    index++;

                    ptr = strtok(NULL, ":");
                }

                Profile.UserNameList[index][0] = '\0';

                break;

            case 'x':

                ProfileMode |= (1 << mTrueXFactor);

                break;

            case 'h':
            case '?':

                fprintf(stderr, "Usage: %s [FLAGS]\n", argv[0]);

                fprintf(stderr, "          [ -a <ACCURACY_SCALE> ]\n");
                fprintf(stderr, "          [ -A <ACCOUNT>[:<ACCOUNT>]...]\n");
                fprintf(stderr, "          [ -b <BEGIN TIME> ]\n");
                fprintf(stderr, "          [ -c <TRACECOUNT> ]\n");
                fprintf(stderr, "          [ -d <DISTFILE> ]\n");
                fprintf(stderr, "          [ -D <LOGLEVEL> ]\n");
                fprintf(stderr, "          [ -e <END TIME> ]\n");
                fprintf(stderr, "          [ -f ] // FIX CORRUPT TRACE INFO\n");
                fprintf(stderr, "          [ -g <GROUP>[:<GROUP>]...]\n");
                fprintf(stderr, "          [ -h ] // HELP\n");
                fprintf(stderr, "          [ -H <PROFILE_HOST> ]\n");
                fprintf(stderr, "          [ -m ] // ENABLE MATRIX MODE\n");
                fprintf(stderr, "          [ -n <PROC_STEP_COUNT> ]\n");
                fprintf(stderr, "          [ -N <MAX_PROC> ]\n");
                fprintf(stderr, "          [ -o <PROC_STEP_SIZE> ]\n");
                fprintf(stderr, "          [ -p <PLOTFILE> ]\n");
                fprintf(stderr, "          [ -P <PROFILE_CHART_LIST> ]\n");
                fprintf(stderr,
                        "          [ -q ] // USE 'SYSTEMQUEUETIME' FOR XFACTOR "
                        "CALCULATION\n");
                fprintf(stderr, "          [ -Q <QOS> ]\n");
                fprintf(stderr,
                        "          [ -r ] // USE 'RUN' TIME (NOT 'REQUEST' "
                        "TIME)\n");
                fprintf(stderr, "          [ -R ] // INCLUDE REMOVED JOB\n");
                fprintf(stderr, "          [ -s ] <TIME_STEP_COUNT>\n");
                fprintf(stderr, "          [ -S ] <TIME_STEP_SIZE>\n");
                fprintf(stderr, "          [ -t <TRACEFILE> ]\n");
                fprintf(stderr, "          [ -T <MAX_TIME> ]\n");
                fprintf(stderr, "          [ -u <USER>[:<USER>]...]\n");
                fprintf(
                    stderr,
                    "          [ -x ] // USE CANONICAL XFACTOR CALCULATION\n");
                fprintf(stderr, "          [ -? ] \n");

                exit(1);

                break;

            default:

                fprintf(stderr, "WARNING:  unknown flag '%c'\n", Flag);

                exit(1);

                break;
        } /* END switch(Flag) */
    }     /* END while (Flag) */

    return (SUCCESS);
} /* END MPReadArgs() */

int PJobCountDistribution()

{
    char Buffer[MAX_MBUFFER];

    DBG(3, fUI) DPrint("PJobCountDistribution()\n");

    MStatBuildGrid(stJobCount, Buffer, ProfileMode);

    fprintf(stdout, "%s\n", Buffer);

    return (SUCCESS);
} /* END PJobCountDistribution() */

int PBFCountDistribution()

{
    char Buffer[MAX_MBUFFER];

    DBG(3, fUI) DPrint("PBFCountDistribution()\n");

    MStatBuildGrid(stBFCount, Buffer, ProfileMode);

    fprintf(stdout, "%s\n", Buffer);

    return (SUCCESS);
}

int PQueueDistribution()

{
    int start;
    int end;

    int hindex;

    int count;

    double AvgSubmit;
    double AvgQueuedJobs;
    double AvgQueuedPS;

    double AvgActiveJobs;
    double AvgActiveNodes;
    double AvgActivePS;

    double AvgComplJobs;

    double TotalQueueTime;

    double TotalXFactor;

    int ActiveSlots;
    int BackLogSlots;
    int TotalSlots;

    int MaxSubmit;
    int MaxQueuedJobs;
    int MaxQueuedPS;
    int MaxActiveJobs;
    int MaxActiveNodes;
    int MaxActivePS;
    int MaxComplJobs;

    int tindex;
    int nindex;

    int pindex;
    int powerjobs;

    int jcmode;
    int nsmode;

    int jobcount;

    int totaltime;

    int cumjobs;
    double cumns;
    int cumnodes;

    char Time[MAX_MNAME];
    long ETime;

    mprof_t *S;

    FILE *dfp;

    time_t tmpTime1;
    time_t tmpTime2;

    const char *FName = "PQueueDistribution";

    DBG(2, fUI) DPrint("%s()\n", FName);

    AvgSubmit = 0.0;
    AvgQueuedJobs = 0.0;
    AvgQueuedPS = 0.0;

    AvgActiveJobs = 0.0;
    AvgActiveNodes = 0.0;
    AvgActivePS = 0.0;

    AvgComplJobs = 0.0;

    MaxSubmit = 0;
    MaxQueuedJobs = 0;
    MaxQueuedPS = 0;

    MaxActiveJobs = 0;
    MaxActiveNodes = 0;
    MaxActivePS = 0;

    MaxComplJobs = 0;

    count = 0;

    S = &SystemProfile;

    if ((dfp = fopen(DistFile, "w+")) == NULL) {
        DBG(0, fCORE)
        DPrint("ERROR:    cannot open distfile '%s', errno: %d\n", DistFile,
               errno);

        perror("cannot open file");

        /* dump plot to stderr */

        dfp = stderr;

        return (FAILURE);
    }

    /* display header */

    fprintf(dfp, "Total Jobs: %d  Period Profiled:  %6.2f days\n",
            S->TotalJobCount,
            (double)(S->LatestStartTime - S->EarliestStartTime) / 86400);

    fprintf(dfp, "\n");

    tmpTime1 = (time_t)Profile.StartTime;
    tmpTime2 = (time_t)S->EarliestStartTime;

    fprintf(dfp, "First Job Start: %s",
            (Profile.StartTime > 0) ? ctime(&tmpTime1) : ctime(&tmpTime2));

    tmpTime1 = (time_t)Profile.EndTime;
    tmpTime2 = (time_t)S->LatestStartTime;

    fprintf(dfp, "Last Job Start:  %s", (Profile.EndTime < MAX_MTIME)
                                            ? ctime(&tmpTime1)
                                            : ctime(&tmpTime2));

    fprintf(dfp, "\n");

    /* display workload distribution */

    fprintf(dfp, "Job Size Distribution\n");

    jcmode = 0;
    nsmode = 0;

    fprintf(dfp, "\n");

    fprintf(dfp, "%6s  %6s  %6s  %6s  %6s  %6s  %6s\n", "Procs", "Jobs",
            "Effic", "PctJob", "PctPS", "CumJob", "CumPS");

    jcmode = 0;
    nsmode = 0;

    cumnodes = 0;
    cumjobs = 0;
    cumns = 0;

    for (nindex = 1; nindex <= MStat.P.MaxNode; nindex++) {
        cumjobs += S->JobSizeDist[nindex];
        cumns += S->PSSizeDist[nindex];
        cumnodes += nindex * S->JobSizeDist[nindex];

        fprintf(dfp, "%06d  %6d  %6.3f  %6.3f  %6.3f  %6.3f  %6.3f\n", nindex,
                S->JobSizeDist[nindex],
                (S->JobSizeDist[nindex] > 0)
                    ? S->JobEfficDist[nindex] / S->JobSizeDist[nindex] * 100.0
                    : 0.0,
                (double)S->JobSizeDist[nindex] / S->TotalJobCount * 100.0,
                S->PSSizeDist[nindex] / S->TotalPS * 100.0,
                (double)cumjobs / S->TotalJobCount * 100.0,
                cumns / S->TotalPS * 100.0);

        if ((jcmode == 0) && (cumjobs >= S->TotalJobCount / 2))
            jcmode = (nindex + 1);

        if ((nsmode == 0) && (cumns >= S->TotalPS / 2)) nsmode = (nindex + 1);
    } /* END for (nindex) */

    powerjobs = 0;

    for (pindex = 1; (1 << pindex) <= MStat.P.MaxNode; pindex++) {
        powerjobs += S->JobSizeDist[1 << pindex];
    }

    fprintf(dfp, "============\n");

    fprintf(dfp,
            "Total Jobs: %d   Serial: %6.2f%c  Power: %6.2f%c  Size "
            "Mean/Median (Job Count): %6.2f/%d (PH): %6.2f PH/%d nodes\n",
            S->TotalJobCount,
            (double)S->JobSizeDist[1] / S->TotalJobCount * 100.0, '%',
            (double)powerjobs / S->TotalJobCount * 100.0, '%',
            (double)cumnodes / S->TotalJobCount, jcmode,
            S->TotalPS / 3600.0 / S->TotalJobCount, nsmode);

    fprintf(dfp, "\n");

    fprintf(dfp, "Job Length Distribution\n");

    fprintf(dfp, "\n");

    fprintf(dfp, "%6s - %6s  %6s  %6s  %6s  %6s\n", "Start", "End", "Jobs",
            "PctJob", "PctPS", "CumJob");

    jcmode = 0;
    totaltime = 0;

    cumjobs = 0;

    for (tindex = 0; tindex <= MStat.P.MaxTime / PROF_TIMEINT_LENGTH;
         tindex++) {
        if (tindex > PROF_TIMEINT_COUNT) break;

        cumjobs += S->JobLengthDist[tindex];

        fprintf(dfp, "%06.3f - %06.3f  %6d  %6.3f  %6.3f  %6.3f\n",
                (double)tindex / 8.0, (double)(tindex + 1) / 8.0 - .01,
                S->JobLengthDist[tindex],
                (double)S->JobLengthDist[tindex] / S->TotalJobCount * 100.0,
                S->PSLengthDist[tindex] / S->TotalPS * 100.0,
                (double)cumjobs / S->TotalJobCount * 100.0);

        totaltime += S->JobLengthDist[tindex] * (tindex + 1) / 8.0;

        if ((jcmode == 0) && (cumjobs >= S->TotalJobCount / 2))
            jcmode = (tindex + 1);
    } /* END for (tindex) */

    fprintf(dfp, "============\n");

    fprintf(dfp, "Total Jobs: %d   Length Mean/Median:  %6.2f/%.2f hours\n",
            S->TotalJobCount, (double)totaltime / S->TotalJobCount,
            (double)jcmode / 8.0);

    fprintf(dfp, "\n");

    /* display workload accuracy summary */

    start = 1000 - (S->FirstQueueTime - S->EarliestQueueTime) / 3600;

    end = (S->LatestQueueTime - S->FirstQueueTime) / 3600 + 1000;

    fprintf(dfp, "Job Accuracy Distribution\n");

    fprintf(dfp, "\n");

    fprintf(dfp, "%8s %6s %6s %6s\n", "Accuracy", "Count", "PctJob", "CumPct");

    jobcount = 0;

    jcmode = 0;

    for (hindex = 0; hindex <= 100; hindex++) {
        jobcount += S->JobAccuracy[hindex];

        fprintf(dfp, "%8d %6d %6.2f %6.2f\n", hindex, S->JobAccuracy[hindex],
                (double)S->JobAccuracy[hindex] / S->TotalJobCount * 100.0,
                (double)jobcount / S->TotalJobCount * 100.0);

        if ((jcmode == 0) && (jobcount >= S->TotalJobCount / 2))
            jcmode = hindex;
    }

    fprintf(dfp, "============\n");

    fprintf(dfp,
            "Total Jobs: %d  Average WCA: %6.2f    Acc5-99 Jobs: %d  Average "
            "WCA: %6.2f  Median: %d\n",
            S->TotalJobCount, (double)S->TotalAccSum / S->TotalJobCount,
            S->Acc5_99Count, (double)S->Acc5_99Sum / S->Acc5_99Count, jcmode);

    fprintf(dfp, "\n");

    /* display QTime distribution */

    fprintf(dfp, "QueueTime Distribution\n");

    fprintf(dfp, "\n");

    fprintf(dfp, "%8s %6s %6s %6s\n", "QTime", "Count", "PctJob", "CumPct");

    TotalQueueTime = 0.0;

    jobcount = 0;
    jcmode = 0;

    for (hindex = 0; hindex <= 1000; hindex++) {
        jobcount += S->TimeQueuedDistProfile[hindex];

        TotalQueueTime +=
            ((hindex / 5) + .1) * S->TimeQueuedDistProfile[hindex];

        fprintf(
            dfp, "%8.3f %6d %6.2f %6.2f\n", (double)hindex / 5.0,
            S->TimeQueuedDistProfile[hindex],
            (double)S->TimeQueuedDistProfile[hindex] / S->TotalJobCount * 100.0,
            (double)jobcount / S->TotalJobCount * 100.0);

        if ((jcmode == 0) && (jobcount >= S->TotalJobCount / 2))
            jcmode = hindex;
    }

    fprintf(dfp, "============\n");

    fprintf(dfp,
            "Total Jobs: %d  Average QTime (hours): %6.2f  Median: %6.2f\n",
            S->TotalJobCount, (double)TotalQueueTime / S->TotalJobCount,
            (double)jcmode / 5.0);

    fprintf(dfp, "\n");

    /* display XFactor distribution */

    fprintf(dfp, "XFactor Distribution\n");

    fprintf(dfp, "\n");

    fprintf(dfp, "%8s %6s %6s %6s\n\n", "XFactor", "Count", "PctJob", "CumPct");

    TotalXFactor = 0.0;

    jobcount = 0;
    jcmode = 0;

    for (hindex = 0; hindex <= 1000; hindex++) {
        jobcount += S->XFactorDistProfile[hindex];

        TotalXFactor +=
            ((double)(hindex / 10.0) + .05) * S->XFactorDistProfile[hindex];

        fprintf(
            dfp, "%8.3f %6d %6.2f %6.2f\n", (double)hindex / 10.0,
            S->XFactorDistProfile[hindex],
            (double)S->XFactorDistProfile[hindex] / S->TotalJobCount * 100.0,
            (double)jobcount / S->TotalJobCount * 100.0);

        if ((jcmode == 0) && (jobcount >= S->TotalJobCount / 2))
            jcmode = hindex;
    }

    fprintf(dfp, "============\n");

    fprintf(dfp, "Total Jobs: %d  Average XFactor: %6.2f  Median: %6.2f\n",
            S->TotalJobCount, (double)TotalXFactor / S->TotalJobCount,
            (double)jcmode / 10.0);

    fprintf(dfp, "\n");

    /* display time distribution */

    fprintf(dfp, "Time Distribution\n\n");

    fprintf(dfp, "%-19s %6s %6s %15s %6s %6s %15s %6s\n\n", "Time", "Submit",
            "QJobs", "QPS", "RJobs", "RProcs", "RPS", "CmplCt");

    ActiveSlots = 0;
    BackLogSlots = 0;
    TotalSlots = 0;

    for (hindex = start; hindex < end; hindex++) {
        TotalSlots++;

        if (S->ActiveJobDepthProfile[hindex] > 0.0) {
            ActiveSlots++;

            if (S->QueueJobDepthProfile[hindex] > 0.0) BackLogSlots++;
        }

        if ((S->QueueJobDepthProfile[hindex] >= 1.0) ||
            (S->ActiveJobDepthProfile[hindex] >= 1.0)) {
            AvgSubmit += (double)S->QueueTimeDistProfile[hindex];
            AvgQueuedJobs += S->QueueJobDepthProfile[hindex];
            AvgQueuedPS += S->QueuePSDepthProfile[hindex];

            AvgActiveJobs += S->ActiveJobDepthProfile[hindex];
            AvgActiveNodes += S->ActiveNodeDepthProfile[hindex];
            AvgActivePS += S->ActivePSDepthProfile[hindex];

            AvgComplJobs += S->CompletionTimeDistProfile[hindex];

            MaxSubmit = MAX(MaxSubmit, S->QueueTimeDistProfile[hindex]);
            MaxQueuedJobs = MAX(MaxQueuedJobs, S->QueueJobDepthProfile[hindex]);
            MaxQueuedPS = MAX(MaxQueuedPS, S->QueuePSDepthProfile[hindex]);
            MaxActiveJobs =
                MAX(MaxActiveJobs, S->ActiveJobDepthProfile[hindex]);
            MaxActiveNodes =
                MAX(MaxActiveNodes, S->ActiveNodeDepthProfile[hindex]);
            MaxActivePS = MAX(MaxActivePS, S->ActivePSDepthProfile[hindex]);
            MaxComplJobs =
                MAX(MaxComplJobs, S->CompletionTimeDistProfile[hindex]);

            count++;

            ETime = S->FirstQueueTime + ((hindex - 1000) * 3600);

            strcpy(Time, MULToDString(&ETime));

            Time[19] = '\0';

            fprintf(dfp, "%19s %6d %6lu %15.2f %6lu %6.2f %15.2f %6d\n", Time,
                    S->QueueTimeDistProfile[hindex],
                    (unsigned long)S->QueueJobDepthProfile[hindex],
                    S->QueuePSDepthProfile[hindex],
                    (unsigned long)S->ActiveJobDepthProfile[hindex],
                    S->ActiveNodeDepthProfile[hindex],
                    S->ActivePSDepthProfile[hindex],
                    S->CompletionTimeDistProfile[hindex]);
        }
    } /* END for (hindex) */

    fprintf(
        dfp,
        "==================================================================\n");

    fprintf(dfp, "%6s %6s %6s %15s %6s %6s %15s %6s\n", "", "Submit", "QJobs",
            "QPS", "RCnt", "RProcs", "RPS", "CmplCt");

    fprintf(dfp, "\n");

    fprintf(dfp, "%6s %6.2f %6.2f %15.2f %6.2f %6.2f %15.2f %6.2f\n", "Avg",
            AvgSubmit / count, AvgQueuedJobs / count, AvgQueuedPS / count,
            AvgActiveJobs / count, AvgActiveNodes / count, AvgActivePS / count,
            AvgComplJobs / count);

    fprintf(dfp, "%6s %6.2f %6.2f %15.2f %6.2f %6.2f %15.2f %6.2f\n", "Max",
            (double)MaxSubmit, (double)MaxQueuedJobs, (double)MaxQueuedPS,
            (double)MaxActiveJobs, (double)MaxActiveNodes, (double)MaxActivePS,
            (double)MaxComplJobs);

    fprintf(dfp, "\n");

    fprintf(dfp, "Total Slots: %d  Active: %6.2f%s  BackLog: %6.2f%s\n",
            TotalSlots, (double)ActiveSlots / TotalSlots * 100.0, "%",
            (double)BackLogSlots / ActiveSlots * 100.0, "%");

    fprintf(dfp, "\n");

    return (SUCCESS);
} /* END PQueueDistribution() */

int PAccuracyDistribution(

    int Mode)

{
    int timeindex;
    int aindex;

    must_t *C;
    must_t *T;

    char *THeader = "[  %8s  ]";
    char *MTHeader = " %12s  ";

    char *TTotal = "[    %5s   ]";
    char *MTTotal = "     %5s    ";

    char *AVal = "[ %4d  ]";
    char *MAVal = "  %4d   ";

    char *ATitle = "[ %5s ]";
    char *MATitle = "  %5s  ";

    char *Val = "[%7.2f %4d]";
    char *MVal = "     %7.2f  ";

    char *Null = "[ ---------- ]";
    char *MNull = "           0  ";

    char *CTotal = "[%7.2f %4d]";
    char *MCTotal = "     %7.2f  ";

    const char *FName = "PAccuracyDistribution";

    DBG(2, fUI) DPrint("%s(%d)\n", FName, Mode);

    fprintf(stdout, "\n\nAccuracy Distribution (in percent)\n\n");

    fprintf(stdout, (Mode & (1 << mMatrix)) ? MATitle : ATitle, "ACCUR");

    for (timeindex = 0; timeindex < MStat.P.TimeStepCount; timeindex++) {
        fprintf(stdout, (Mode & (1 << mMatrix)) ? MTHeader : THeader,
                MUBStringTime(MStat.P.TimeStep[timeindex]));
    } /* END for (timeindex) */

    fprintf(stdout, (Mode & (1 << mMatrix)) ? MTTotal : TTotal, "TOTAL");

    fprintf(stdout, "\n");

    T = &MPar[0].S;

    for (aindex = 0; aindex < MStat.P.AccuracyScale; aindex++) {
        fprintf(stdout, (Mode & (1 << mMatrix)) ? MAVal : AVal,
                MStat.P.AccuracyStep[aindex]);

        for (timeindex = 0; timeindex < MStat.P.TimeStepCount; timeindex++) {
            C = &MStat.CTotal[timeindex];

            if (C->Accuracy[aindex] != 0) {
                fprintf(stdout, (Mode & (1 << mMatrix)) ? MVal : Val,
                        (double)C->Accuracy[aindex] / C->Count * 100,
                        C->Accuracy[aindex]);
            } else {
                fprintf(stdout, "%s", (Mode & (1 << mMatrix)) ? MNull : Null);
            }
        }

        if (T->Accuracy[aindex] != 0) {
            fprintf(stdout, (Mode & (1 << mMatrix)) ? MVal : Val,
                    (double)T->Accuracy[aindex] / T->Count * 100,
                    T->Accuracy[aindex]);

            fprintf(stdout, "\n");
        } else {
            fprintf(stdout, "%s", (Mode & (1 << mMatrix)) ? MNull : Null);

            fprintf(stdout, "\n");
        }
    }

    fprintf(stdout, (Mode & (1 << mMatrix)) ? MATitle : ATitle, "TOTAL");

    for (timeindex = 0; timeindex < MStat.P.TimeStepCount; timeindex++) {
        C = &MStat.CTotal[timeindex];

        if (C->Count != 0) {
            fprintf(stdout, (Mode & (1 << mMatrix)) ? MCTotal : CTotal,
                    (double)C->TotalRunTime / C->TotalRequestTime * 100,
                    C->Count);
        } else {
            fprintf(stdout, "%s", (Mode & (1 << mMatrix)) ? MNull : Null);
        }
    }

    fprintf(stdout, "\n");

    return (SUCCESS);
} /* END PAccuracyDistribution() */

int PPSRequestDistribution()

{
    char Buffer[MAX_MBUFFER];

    DBG(3, fUI) DPrint("PPSRequestDistribution()\n");

    MStatBuildGrid(stPSRequest, Buffer, ProfileMode);

    fprintf(stdout, "%s\n", Buffer);

    return (SUCCESS);
}

int PPSRunDistribution()

{
    char Buffer[MAX_MBUFFER];

    DBG(3, fUI) DPrint("PPSRunDistribution()\n");

    MStatBuildGrid(stPSRun, Buffer, ProfileMode);

    fprintf(stdout, "%s\n", Buffer);

    return (SUCCESS);
}

int PQueueTimeDistribution()

{
    char Buffer[MAX_MBUFFER];

    DBG(3, fUI) DPrint("PQueueTimeDistribution()\n");

    MStatBuildGrid(stAvgQTime, Buffer, ProfileMode);

    fprintf(stdout, "%s\n", Buffer);

    return (SUCCESS);
}

int PJobLengthAccuracy()

{
    char Buffer[MAX_MBUFFER];

    DBG(2, fUI) DPrint("PJobLengthAccuracy()\n");

    MStatBuildGrid(stWCAccuracy, Buffer, ProfileMode);

    fprintf(stdout, "%s\n", Buffer);

    return (SUCCESS);
}

int PXFactor()

{
    char Buffer[MAX_MBUFFER];

    DBG(2, fUI) DPrint("PXFactor()\n");

    MStatBuildGrid(stAvgXFactor, Buffer, ProfileMode);

    fprintf(stdout, "%s\n", Buffer);

    return (SUCCESS);
}

int PMaxXFactor()

{
    char Buffer[MAX_MBUFFER];

    DBG(2, fUI) DPrint("PMaxXFactor()\n");

    MStatBuildGrid(stMaxXFactor, Buffer, ProfileMode);

    fprintf(stdout, "%s\n", Buffer);

    return (SUCCESS);
}

int PJobEfficiency()

{
    char Buffer[MAX_MBUFFER];

    DBG(2, fUI) DPrint("PJobEfficiency()\n");

    MStatBuildGrid(stJobEfficiency, Buffer, ProfileMode);

    fprintf(stdout, "%s\n", Buffer);

    return (SUCCESS);
}

int PQOSSuccessRate()

{
    char Buffer[MAX_MBUFFER];

    DBG(2, fUI) DPrint("PQOSSuccessRate()\n");

    MStatBuildGrid(stQOSDelivered, Buffer, ProfileMode);

    fprintf(stdout, "%s\n", Buffer);

    return (SUCCESS);
}

int ProfileMemoryUsage()

{
    int MaxMemory = 2048;

    int mindex;

    DBG(2, fUI) DPrint("ProfileMemoryUsage()\n");

    fprintf(stdout, "\n\nMemory Request Distribution\n\n");

    for (mindex = 0; MaxMemory > (32 << (mindex - 1)); mindex++) {
        fprintf(stdout,
                "%4d MB:  %4lu Jobs (%5.2f)  %6lu Procs (%5.2f)  %13.1f "
                "PSRequest (%5.2f)  %13.1f PSRun (%5.2f)\n",
                (32 << mindex), JobsatMemorySize[mindex],
                (double)JobsatMemorySize[mindex] / TotalJobs * 100.0,
                NodesatMemorySize[mindex],
                (double)NodesatMemorySize[mindex] / TotalNodes * 100.0,
                PSRequestatMemorySize[mindex],
                PSRequestatMemorySize[mindex] / TotalPSRequest * 100.0,
                PSRunatMemorySize[mindex],
                PSRunatMemorySize[mindex] / TotalPSRun * 100.0);
    }

    return (SUCCESS);
} /* ENDProfileMemoryUsage() */

int ProfileUser()

{
    int uindex;
    mgcred_t *U;
    must_t *S;
    must_t *T;

    int dindex;
    int aindex;

    double var;
    double *val;
    double mean;

    DBG(2, fUI) DPrint("ProfileUser()\n");

    fprintf(stdout, "\n\nuser job distribution\n");

    fprintf(stdout,
            "\n%10s %10s (PCT) %13s (PCT) %13s (PCT)   %10s %10s %10s %10s "
            "%10s %10s %10s\n\n",
            "Name", "JobCount", "PHRequest", "PHRun", "QueueTime", "AvgRunTime",
            "AvgReqTime", "WCAccuracy", "WCAStdDev", "XFactor", "NodeXF");

    /* NOTE:  must determine number of users in table */

    qsort((void *)&MUser[0], MAX_MUSER + MAX_MHBUF, sizeof(mgcred_t *),
          (int (*)())UPSComp);

    T = &MPar[0].S;

    for (uindex = 0; uindex < MAX_MUSER + MAX_MHBUF; uindex++) {
        U = MUser[uindex];

        if ((U == NULL) || (U->Name[0] == '\0') || (U->Name[0] == '\1'))
            continue;

        if (U->Stat.PSRequest <= 0) continue;

        if (Profile.UserNameList[0][0] != '\0') {
            for (aindex = 0; Profile.UserNameList[aindex][0] != '\0';
                 aindex++) {
                if (!strcmp(U->Name, Profile.UserNameList[aindex])) break;
            }

            if (Profile.UserNameList[aindex][0] == '\0') continue;
        }

        S = &(U->Stat);

        var = 0.0;

        if ((MUDStatIsEnabled(&S->DStat[dstatWCA]) == SUCCESS) &&
            (S->DStat[dstatWCA].Count > 1)) {
            mean = S->JobAcc / S->Count;

            for (dindex = 0; dindex < S->DStat[dstatWCA].Count; dindex++) {
                val = (double *)&S->DStat[dstatWCA]
                          .Data[dindex * S->DStat[dstatWCA].DSize];

                var += pow((*val - mean), 2.0);
            }

            var /= (double)(S->DStat[dstatWCA].Count - 1);

            var = pow(var, 0.5);
        }

        fprintf(stdout,
                "%10s %10d (%05.2f) %11.2f (%05.2f) %11.2f (%05.2f) %10.2f "
                "%10.2f %10.2f %10.3f %10.3f %10.3f %10.3f\n",
                U->Name, S->Count, (double)S->Count / T->Count * 100.0,
                S->PSRequest / 3600.0,
                (double)S->PSRequest / T->PSRequest * 100.0, S->PSRun / 3600.0,
                (double)S->PSRun / T->PSRun * 100.0,
                (double)S->TotalQTS / S->Count / 3600.0,
                (double)S->TotalRunTime / S->Count / 3600.0,
                (double)S->TotalRequestTime / S->Count / 3600.0,
                S->JobAcc / S->Count * 100.0, var * 100.0,
                S->XFactor / S->Count, S->NXFactor / S->NCount);
    } /* END for (uindex) */

    fprintf(stdout, "\n");

    var = 0.0;

    if ((MUDStatIsEnabled(&T->DStat[dstatWCA]) == SUCCESS) &&
        (T->DStat[dstatWCA].Count > 1)) {
        mean = T->JobAcc / T->Count;

        for (dindex = 0; dindex < T->DStat[dstatWCA].Count; dindex++) {
            val = (double *)&T->DStat[dstatWCA]
                      .Data[dindex * T->DStat[dstatWCA].DSize];

            var += pow((*val - mean), 2.0);
        }

        var /= (double)(T->DStat[dstatWCA].Count - 1);

        var = pow(var, 0.5);
    }

    fprintf(stdout,
            "%10s %10d (%05.1f) %11.2f (%05.1f) %11.2f (%05.1f) %10.2f %10.2f "
            "%10.2f %10.3f %10.3f %10.3f %10.3f\n",
            "Total", T->Count, (double)T->Count / T->Count * 100.0,
            T->PSRequest / 3600.0, (double)T->PSRequest / T->PSRequest * 100.0,
            T->PSRun / 3600.0, (double)T->PSRun / T->PSRun * 100.0,
            (double)T->TotalQTS / T->Count / 3600.0,
            (double)T->TotalRunTime / T->Count / 3600.0,
            (double)T->TotalRequestTime / T->Count / 3600.0,
            T->JobAcc / T->Count * 100.0, var * 100.0, T->XFactor / T->Count,
            T->NXFactor / T->NCount);

    return (SUCCESS);
} /* END ProfileUser() */

int ProfileGroup()

{
    int gindex;

    mgcred_t *G;
    must_t *S;
    must_t *T;

    int dindex;
    int aindex;

    double var;
    double *val;
    double mean;

    DBG(2, fUI) DPrint("ProfileGroup()\n");

    fprintf(stdout, "\n\ngroup job distribution\n");

    fprintf(stdout,
            "\n%10s %10s (PCT) %13s (PCT) %13s (PCT)   %10s %10s %10s %10s "
            "%10s %10s %10s\n\n",
            "Name", "JobCount", "PHRequest", "PHRun", "QueueTime", "AvgRunTime",
            "AvgReqTime", "WCAccuracy", "WCAStdDev", "XFactor", "NodeXF");

    /* NOTE:  must determine number of groups in table */

    qsort((void *)&MGroup[0], MAX_MGROUP + MAX_MHBUF, sizeof(mgcred_t),
          (int (*)())GPSComp);

    T = &MPar[0].S;

    for (gindex = 0; MGroup[gindex].Stat.PSRequest > 0; gindex++) {
        G = &MGroup[gindex];

        if (Profile.GroupNameList[0][0] != '\0') {
            for (aindex = 0; Profile.GroupNameList[aindex][0] != '\0';
                 aindex++) {
                if (!strcmp(G->Name, Profile.GroupNameList[aindex])) break;
            }

            if (Profile.GroupNameList[aindex][0] == '\0') continue;
        }

        S = &(G->Stat);

        var = 0.0;

        if ((MUDStatIsEnabled(&S->DStat[dstatWCA]) == SUCCESS) &&
            (S->DStat[dstatWCA].Count > 1)) {
            mean = S->JobAcc / S->Count;

            for (dindex = 0; dindex < S->DStat[dstatWCA].Count; dindex++) {
                val = (double *)&S->DStat[dstatWCA]
                          .Data[dindex * S->DStat[dstatWCA].DSize];

                var += pow((*val - mean), 2.0);
            }

            var /= (double)(S->DStat[dstatWCA].Count - 1);

            var = pow(var, 0.5);
        }

        fprintf(stdout,
                "%10s %10d (%05.2f) %11.2f (%05.2f) %11.2f (%05.2f) %10.2f "
                "%10.2f %10.2f %10.3f %10.3f %10.3f %10.3f\n",
                G->Name, S->Count, (double)S->Count / T->Count * 100.0,
                S->PSRequest / 3600.0,
                (double)S->PSRequest / T->PSRequest * 100.0, S->PSRun / 3600.0,
                (double)S->PSRun / T->PSRun * 100.0,
                (double)S->TotalQTS / S->Count / 3600.0,
                (double)S->TotalRunTime / S->Count / 3600.0,
                (double)S->TotalRequestTime / S->Count / 3600.0,
                S->JobAcc / S->Count * 100.0, var * 100.0,
                S->XFactor / S->Count, S->NXFactor / S->NCount);
    } /* END for (gindex) */

    fprintf(stdout, "\n");

    var = 0.0;

    if ((MUDStatIsEnabled(&T->DStat[dstatWCA]) == SUCCESS) &&
        (T->DStat[dstatWCA].Count > 1)) {
        mean = T->JobAcc / T->Count;

        for (dindex = 0; dindex < T->DStat[dstatWCA].Count; dindex++) {
            val = (double *)&T->DStat[dstatWCA]
                      .Data[dindex * T->DStat[dstatWCA].DSize];

            var += pow((*val - mean), 2.0);
        }

        var /= (double)(T->DStat[dstatWCA].Count - 1);

        var = pow(var, 0.5);
    }

    fprintf(stdout,
            "%10s %10d (%05.1f) %11.2f (%05.1f) %11.2f (%05.1f) %10.2f %10.2f "
            "%10.2f %10.3f %10.3f %10.3f %10.3f\n",
            "Total", T->Count, (double)T->Count / T->Count * 100.0,
            T->PSRequest / 3600.0, (double)T->PSRequest / T->PSRequest * 100.0,
            T->PSRun / 3600.0, (double)T->PSRun / T->PSRun * 100.0,
            (double)T->TotalQTS / T->Count / 3600.0,
            (double)T->TotalRunTime / T->Count / 3600.0,
            (double)T->TotalRequestTime / T->Count / 3600.0,
            T->JobAcc / T->Count * 100.0, var * 100.0, T->XFactor / T->Count,
            T->NXFactor / T->NCount);

    return (SUCCESS);
} /* END ProfileGroup() */

int ProfileAccount()

{
    int aindex;
    mgcred_t *A;
    must_t *S;
    must_t *T;

    int dindex;

    double var;
    double *val;
    double mean;

    DBG(2, fUI) DPrint("ProfileAccount()\n");

    fprintf(stdout, "\n\naccount job distribution\n");

    fprintf(stdout,
            "\n%10s %10s (PCT) %13s (PCT) %13s (PCT)   %10s %10s %10s %10s "
            "%10s %10s %10s\n\n",
            "Name", "JobCount", "PHRequest", "PHRun", "QueueTime", "AvgRunTime",
            "AvgReqTime", "WCAccuracy", "WCAStdDev", "XFactor", "NodeXF");

    /* NOTE:  must determine number of accounts in table */

    qsort((void *)&MAcct[0], MAX_MACCT + MAX_MHBUF, sizeof(mgcred_t),
          (int (*)())APSComp);

    T = &MPar[0].S;

    for (aindex = 0; MAcct[aindex].Stat.PSRequest > 0; aindex++) {
        A = &MAcct[aindex];

        S = &(A->Stat);

        var = 0.0;

        if ((MUDStatIsEnabled(&S->DStat[dstatWCA]) == SUCCESS) &&
            (S->DStat[dstatWCA].Count > 1)) {
            mean = S->JobAcc / S->Count;

            for (dindex = 0; dindex < S->DStat[dstatWCA].Count; dindex++) {
                val = (double *)&S->DStat[dstatWCA]
                          .Data[dindex * S->DStat[dstatWCA].DSize];

                var += pow((*val - mean), 2.0);
            }

            var /= (double)(S->DStat[dstatWCA].Count - 1);

            var = pow(var, 0.5);
        }

        fprintf(stdout,
                "%10s %10d (%05.2f) %11.2f (%05.2f) %11.2f (%05.2f) %10.2f "
                "%10.2f %10.2f %10.3f %10.3f %10.3f %10.3f\n",
                A->Name, S->Count, (double)S->Count / T->Count * 100.0,
                S->PSRequest / 3600.0,
                (double)S->PSRequest / T->PSRequest * 100.0, S->PSRun / 3600.0,
                (double)S->PSRun / T->PSRun * 100.0,
                (double)S->TotalQTS / S->Count / 3600.0,
                (double)S->TotalRunTime / S->Count / 3600.0,
                (double)S->TotalRequestTime / S->Count / 3600.0,
                S->JobAcc / S->Count * 100.0, var * 100.0,
                S->XFactor / S->Count, S->NXFactor / S->NCount);
    } /* END for (uindex) */

    fprintf(stdout, "\n");

    var = 0.0;

    if ((MUDStatIsEnabled(&T->DStat[dstatWCA]) == SUCCESS) &&
        (T->DStat[dstatWCA].Count > 1)) {
        mean = T->JobAcc / T->Count;

        for (dindex = 0; dindex < T->DStat[dstatWCA].Count; dindex++) {
            val = (double *)&T->DStat[dstatWCA]
                      .Data[dindex * T->DStat[dstatWCA].DSize];

            var += pow((*val - mean), 2.0);
        }

        var /= (double)(T->DStat[dstatWCA].Count - 1);

        var = pow(var, 0.5);
    }

    fprintf(stdout,
            "%10s %10d (%05.1f) %11.2f (%05.1f) %11.2f (%05.1f) %10.2f %10.2f "
            "%10.2f %10.3f %10.3f %10.3f %10.3f\n",
            "Total", T->Count, (double)T->Count / T->Count * 100.0,
            T->PSRequest / 3600.0, (double)T->PSRequest / T->PSRequest * 100.0,
            T->PSRun / 3600.0, (double)T->PSRun / T->PSRun * 100.0,
            (double)T->TotalQTS / T->Count / 3600.0,
            (double)T->TotalRunTime / T->Count / 3600.0,
            (double)T->TotalRequestTime / T->Count / 3600.0,
            T->JobAcc / T->Count * 100.0, var * 100.0, T->XFactor / T->Count,
            T->NXFactor / T->NCount);

    return (SUCCESS);
} /* END ProfileAccount() */

/* order high to low */

int UPSComp(mgcred_t **a, mgcred_t **b)

{
    static int tmp;

    if ((*a != NULL) && (*b != NULL))
        tmp = (*b)->Stat.PSRun - (*a)->Stat.PSRun;
    else
        tmp = 0;

    return (tmp);
}

/* order high to low */

int GPSComp(

    mgcred_t *A, mgcred_t *B)

{
    static int tmp;

    tmp = B->Stat.PSRun - A->Stat.PSRun;

    return (tmp);
} /* END GPBSComp() */

/* order high to low */

int APSComp(mgcred_t *a, mgcred_t *b)

{
    static int tmp;

    tmp = b->Stat.PSRun - a->Stat.PSRun;

    return (tmp);
}

int MPLoadTrace(

    char *TraceFile)

{
    int count;

    int aindex;

    int queueindex;
    int startindex;
    int endindex;
    int hindex;

    int rindex;
    int windex;
    int pindex;

    int nhindex;

    char *buf;
    char *ptr;
    char *head;
    char *tail;

    int Version;

    int Offset;

    int Accuracy;
    int RunTime;
    int RunPS;

    mjob_t tmpJ;
    mjob_t *J;

    int mem;
    int mindex;

    int QTime;
    double XFactor;
    double HFraction;

    mprof_t *S;

    mreq_t *RQ;

    int SC;

    int LineCount;

    DBG(2, fSIM) DPrint("LoadTrace(%s)\n", TraceFile);

    S = &SystemProfile;

    if ((buf = MFULoad(TraceFile, 1, macmRead, &count, &SC)) == NULL) {
        DBG(0, fSIM)
        DPrint("ERROR:    cannot open tracefile '%s'\n", TraceFile);

        exit(1);
    }

    Version = DEFAULT_WORKLOAD_TRACE_VERSION;

    /* set head to first line after marker */

    /* load workload traces */

    DBG(3, fSIM)
    DPrint("INFO:     loading workload traces from TraceFile '%s'\n",
           TraceFile);

    count = 0;

    head = buf;

    ptr = head;

    tail = head + strlen(head);

    if (MUDStatIsEnabled(&MPar[0].S.DStat[dstatWCA]) == FAILURE) {
        MUDStatInitialize(&MPar[0].S.DStat[dstatWCA], sizeof(double));
    }

    LineCount = 0;

    while (ptr < tail) {
        LineCount++;
        Offset = 0;

        if (MTraceLoadWorkload(ptr, &Offset, &tmpJ, msmProfile, &Version) ==
            SUCCESS) {
            ptr += Offset;

            J = &tmpJ;

            RQ = J->Req[0]; /* FIXME */

            if ((J->Cred.U->Name[0] == '\0') || (J->Cred.G->Name[0] == '\0')) {
                DBG(1, fSIM)
                DPrint(
                    "ALERT:    cannot determine UName/GName for job '%s'  "
                    "(ignoring job)\n",
                    J->Name);

                continue;
            }

            /* ignore record if job not profiled */

            if ((Profile.Host[0] != '\0') &&
                (strcmp(Profile.Host, J->MasterHostName)))
                continue;

            if ((J->Cred.Q != NULL) && (Profile.QOSList[0] != '\0') &&
                !strstr(Profile.QOSList, J->Cred.Q->Name))
                continue;

            if (Profile.UserNameList[0][0] != '\0') {
                for (aindex = 0; Profile.UserNameList[aindex][0] != '\0';
                     aindex++) {
                    if (!strcmp(J->Cred.U->Name, Profile.UserNameList[aindex]))
                        break;
                }

                if (Profile.UserNameList[aindex][0] == '\0') continue;
            }

            if (Profile.GroupNameList[0][0] != '\0') {
                for (aindex = 0; Profile.GroupNameList[aindex][0] != '\0';
                     aindex++) {
                    if (!strcmp(J->Cred.G->Name, Profile.GroupNameList[aindex]))
                        break;
                }

                if (Profile.GroupNameList[aindex][0] == '\0') continue;
            }

            if ((Profile.AccountNameList[0][0] != '\0') &&
                (J->Cred.A != NULL)) {
                for (aindex = 0; Profile.AccountNameList[aindex][0] != '\0';
                     aindex++) {
                    if (!strcmp(J->Cred.A->Name,
                                Profile.AccountNameList[aindex]))
                        break;
                }

                if (Profile.AccountNameList[aindex][0] == '\0') continue;
            }

            if (Profile.StartTime > J->StartTime) continue;

            if (Profile.EndTime < J->StartTime) continue;

            /* enable detailed wca collection */

            if (MUDStatIsEnabled(&J->Cred.U->Stat.DStat[dstatWCA]) == FAILURE) {
                MUDStatInitialize(&J->Cred.U->Stat.DStat[dstatWCA],
                                  sizeof(double));
            }

            if (MUDStatIsEnabled(&J->Cred.G->Stat.DStat[dstatWCA]) == FAILURE) {
                MUDStatInitialize(&J->Cred.G->Stat.DStat[dstatWCA],
                                  sizeof(double));
            }

            if (J->Cred.A != NULL) {
                if (MUDStatIsEnabled(&J->Cred.A->Stat.DStat[dstatWCA]) ==
                    FAILURE) {
                    MUDStatInitialize(&J->Cred.A->Stat.DStat[dstatWCA],
                                      sizeof(double));
                }
            }

            J->StartTime = J->DispatchTime;

            J->PSDedicated = (double)(J->CompletionTime - J->StartTime) *
                             MJobGetProcCount(J);

            if (((J->State == mjsRemoved) || (J->State == mjsNotRun)) &&
                !(ProfileMode & (1 << mUseRemoved))) {
                MStatUpdateRejectedJobUsage(&tmpJ, ProfileMode);

                continue;
            }

            DBG(6, fSIM) DPrint("INFO:     job '%s' loaded\n", J->Name);

            /* add user/group/account record */

            if (S->FirstQueueTime == MAX_MTIME) {
                S->FirstQueueTime = J->SubmitTime;
                MStat.InitTime = J->SubmitTime;
            }

            if (((long)S->FirstQueueTime - (long)J->SubmitTime) / 3600 > 1000) {
                DBG(0, fSIM)
                DPrint(
                    "ALERT:    traces are not in queuetime order (job '%s' "
                    "queued before first job trace)\n",
                    J->Name);
            }

            RunTime = (J->CompletionTime > J->StartTime)
                          ? J->CompletionTime - J->StartTime
                          : 0;

            RunTime = MIN(RunTime, MStat.P.MaxTime);

            if ((RunTime > J->WCLimit) && (RunTime - J->WCLimit < 600))
                RunTime = J->WCLimit;

            Accuracy = (100 * RunTime) / J->WCLimit;

            if ((Accuracy > 100) || (Accuracy < 0)) {
                DBG(4, fSIM)
                DPrint(
                    "WARNING:  job '%s' exceeded wallclock limit (%d > %ld)\n",
                    J->Name, RunTime, J->WCLimit);

                Accuracy = MAX(Accuracy, 0);
                Accuracy = MIN(Accuracy, 100);
            } else {
                if ((Accuracy >= 5) && (Accuracy < 100)) {
                    S->Acc5_99Sum += Accuracy;
                    S->Acc5_99Count++;
                }
            }

            S->JobAccuracy[Accuracy]++;

            S->TotalAccSum += Accuracy;
            S->TotalJobCount++;

            /* calculate QTime/XFactor distributions */

            QTime = (J->StartTime - J->SubmitTime);

            QTime = MIN(720000, QTime);
            QTime = MAX(0, QTime);

            S->TimeQueuedDistProfile[QTime / 720]++;

            XFactor = (double)(QTime + J->WCLimit) / J->WCLimit;

            XFactor = MIN(100, XFactor);

            S->XFactorDistProfile[(int)(XFactor * 10)]++;

            S->EarliestQueueTime = MIN(S->EarliestQueueTime, J->SubmitTime);
            S->EarliestStartTime = MIN(S->EarliestStartTime, J->StartTime);

            S->LatestQueueTime = MAX(S->LatestQueueTime, J->SubmitTime);
            S->LatestStartTime = MAX(S->LatestStartTime, J->StartTime);
            S->LatestCompletionTime =
                MAX(S->LatestCompletionTime, J->CompletionTime);

            queueindex =
                ((long)J->SubmitTime - (long)S->FirstQueueTime) / 3600 + 1000;
            startindex =
                ((long)J->StartTime - (long)S->FirstQueueTime) / 3600 + 1000;
            endindex =
                ((long)J->CompletionTime - (long)S->FirstQueueTime) / 3600 +
                1000;

            DBG(5, fSIM)
            DPrint("INFO:     job '%s' (%d:%d:%d) FQT: %ld  QT: %ld  ST: %ld\n",
                   J->Name, queueindex, startindex, endindex, S->FirstQueueTime,
                   J->SubmitTime, J->StartTime);

            S->QueueTimeDistProfile[queueindex]++;
            S->CompletionTimeDistProfile[endindex]++;

            /* locate/update start of queued job */

            if (startindex > queueindex)
                HFraction =
                    (double)(1.0 -
                             (double)((J->SubmitTime - S->FirstQueueTime) %
                                      3600) /
                                 3600.0);
            else
                HFraction = (double)(J->StartTime - J->SubmitTime) / 3600.0;

            S->QueueJobDepthProfile[queueindex] += HFraction;
            S->QueuePSDepthProfile[queueindex] +=
                HFraction * (J->WCLimit * MJobGetProcCount(J));

            /* update middle of queued job */

            for (hindex = queueindex + 1; hindex < startindex; hindex++) {
                S->QueueJobDepthProfile[hindex]++;
                S->QueuePSDepthProfile[hindex] +=
                    (double)(J->WCLimit * MJobGetProcCount(J));
            }

            /* locate/update end of queued job */

            if (startindex > queueindex)
                HFraction =
                    (double)((J->StartTime - S->FirstQueueTime) % 3600) /
                    3600.0;
            else
                HFraction = 0.0;

            S->QueueJobDepthProfile[startindex] += HFraction;
            S->QueuePSDepthProfile[startindex] +=
                HFraction * (J->WCLimit * MJobGetProcCount(J));

            /* locate/update start of active job */

            if (endindex > startindex)
                HFraction =
                    (double)(1.0 -
                             (double)((J->StartTime - S->FirstQueueTime) %
                                      3600) /
                                 3600.0);
            else
                HFraction = (double)(J->CompletionTime - J->StartTime) / 3600.0;

            S->ActiveJobDepthProfile[startindex] += HFraction;
            S->ActiveNodeDepthProfile[startindex] +=
                HFraction * MJobGetProcCount(J);
            S->ActivePSDepthProfile[startindex] +=
                HFraction * (J->WCLimit * MJobGetProcCount(J));

            DBG(5, fSIM)
            DPrint("INFO:     job '%s'(%3d/%6ld) dist %4d : %6.2f (%15.2f)\n",
                   J->Name, MJobGetProcCount(J), J->WCLimit, startindex,
                   HFraction * MJobGetProcCount(J),
                   S->ActiveNodeDepthProfile[startindex]);

            /* update middle of active job */

            for (hindex = startindex + 1; hindex < endindex; hindex++) {
                S->ActiveJobDepthProfile[hindex]++;
                S->ActiveNodeDepthProfile[hindex] +=
                    (double)MJobGetProcCount(J);
                S->ActivePSDepthProfile[hindex] +=
                    (double)(J->WCLimit * MJobGetProcCount(J));

                DBG(5, fSIM)
                DPrint(
                    "INFO:     job '%s'(%3d/%6ld) dist %4d : %6.2f (%15.2f)\n",
                    J->Name, MJobGetProcCount(J), J->WCLimit, hindex,
                    (double)MJobGetProcCount(J),
                    S->ActiveNodeDepthProfile[hindex]);
            }

            /* locate/update end of active job */

            if (endindex > startindex)
                HFraction =
                    (double)((J->CompletionTime - S->FirstQueueTime) % 3600) /
                    3600.0;
            else
                HFraction = 0.0;

            S->ActiveJobDepthProfile[endindex] += HFraction;
            S->ActiveNodeDepthProfile[endindex] +=
                HFraction * MJobGetProcCount(J);
            S->ActivePSDepthProfile[endindex] +=
                HFraction * (J->WCLimit * MJobGetProcCount(J));

            DBG(5, fSIM)
            DPrint("INFO:     job '%s'(%3d/%6ld) dist %4d : %6.2f (%15.2f)\n",
                   J->Name, MJobGetProcCount(J), J->WCLimit, endindex,
                   HFraction * MJobGetProcCount(J),
                   S->ActiveNodeDepthProfile[endindex]);

            MStatUpdateCompletedJobUsage(J, msmProfile, ProfileMode);

            /* update distribution data */

            windex = MIN(MStat.P.MaxTime, J->WCLimit / PROF_TIMEINT_LENGTH);
            rindex = MIN(MStat.P.MaxTime, RunTime / PROF_TIMEINT_LENGTH);
            pindex = MIN(MStat.P.MaxNode, MJobGetProcCount(J));

            if (ProfileMode & (1 << mUseRunTime)) {
                RunPS = MJobGetProcCount(J) * J->WCLimit;

                S->JobLengthDist[windex]++;
                S->PSLengthDist[windex] += RunPS;
            } else {
                RunPS = MJobGetProcCount(J) * RunTime;

                S->JobLengthDist[rindex]++;
                S->PSLengthDist[rindex] += RunPS;
            }

            S->TotalPS += RunPS;

            nhindex = MIN(1000, RunPS / 36000);

            S->JobPHDist[nhindex]++;

            S->PSSizeDist[pindex] += RunPS;
            S->JobSizeDist[pindex]++;

            S->JobEfficDist[pindex] +=
                (RunPS > 0.0) ? J->PSUtilized / RunPS : 0.0;

            /* profile memory usage */

            mem = RQ->RequiredMemory;

            switch (RQ->MemCmp) {
                case mcmpGT:

                    mem++;

                    break;

                case mcmpLT:
                case mcmpLE:
                case mcmpNONE:
                default:

                    mem = 0;

                    break;
            } /* END (RQ->MemCmp) */

            for (mindex = 0; mem > (32 << mindex); mindex++)
                ;

            JobsatMemorySize[mindex]++;

            NodesatMemorySize[mindex] += MJobGetProcCount(J);

            PSRequestatMemorySize[mindex] +=
                (double)(MJobGetProcCount(J) * J->WCLimit);

            PSRunatMemorySize[mindex] +=
                (double)(MJobGetProcCount(J) *
                         (J->CompletionTime - J->StartTime));

            TotalJobs++;

            TotalNodes += MJobGetProcCount(J);

            TotalPSRequest += (double)(MJobGetProcCount(J) * J->WCLimit);

            TotalPSRun += (double)MJobGetProcCount(J) *
                          (J->CompletionTime - J->StartTime);

            MDEBUG(4)
            MJobShow(J, 0, NULL);
        } else {
            if (Offset == 0) {
                break;
            } else {
                DBG(2, fSIM)
                DPrint("INFO:     no job on line %20.20s...\n", ptr);

                ptr += Offset;
            }
        } /* END else (LoadWorkloadTrace() == SUCCESS) */

        if ((MStat.P.TraceCount > 0) && (TotalJobs == MStat.P.TraceCount)) {
            DBG(2, fSIM)
            DPrint("INFO:     TraceCount limit (%d) reached\n", TotalJobs);

            break;
        }
    } /* END while (ptr < tail) */

    DBG(3, fSIM)
    DPrint("INFO:     %d job traces loaded from tracefile '%s' (%d lines)\n",
           TotalJobs, TraceFile, LineCount);

    return (SUCCESS);
} /* END MPLoadTrace() */

/* END mprof.c */
