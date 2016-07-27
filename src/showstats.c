/*
 * showstats standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

#define VERBOSE 4
#define MAXJOB 4096
#define MAX_WORD 32
#define MAX_MUSER 1024
#define MMAX_XMLATTR 64
#define MDEF_XMLICCOUNT 16
#define ID 50
#define MAX_MGROUP 1024
#define MAX_MACCT 1024
#define MAX_MCLASS 16
#define MAX_MQOS 128
#define MAX_ACCURACY 10
#define MAX_MPOLICY 9
#define MAX_MPAR 4
#define MAX_MATTR 128
#define MBNOTSET 2
#define MSET 2
#define USER 25

#define CRYPTHEAD "KGV"

#define mbool_t unsigned char

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
	mstaGCEJobs, /* current eligible jobs */
	mstaGCIJobs, /* current idle jobs */
	mstaGCAJobs, /* current active jobs */
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
	mstaSchedCount
};

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

typedef struct {
    double FSTarget; /* credential fs usage target              */
    int FSMode;      /* fs target type                          */
} mfs_t;

typedef struct {
    int Usage[MAX_MPOLICY][MAX_MPAR];
} mcredl_t;

typedef struct {
    int Count;             /* total jobs in grid                              */
    int NCount;            /* nodes allocated to jobs                         */
    int QOSMet;         /* jobs that met requested QOS                     */
    time_t TotalQTS;    /* job queue time                                  */
    time_t MaxQTS;
    double TotalRequestTime; /* total requested job walltime */
    double TotalRunTime; /* total execution job walltime                    */

    /* job-level statistics */

    double PSRequest; /* requested procseconds completed                 */
    double PSRun; /* executed procsecond                             */

    /* iteration-level statistics */

    double PSDedicated; /* ProcSeconds Dedicated                           */
    double PSUtilized;  /* ProcSeconds Actually Used                       */

    double MSDedicated;

    double MSAvail; /* MemorySeconds Available                         */

    double JobAcc;     /* Job Accuracy Sum                                */
    double NJobAcc;    /* Node Weighed Job Accuracy Sum                   */
    double XFactor;    /* XFactor Sum                                     */
    double NXFactor;   /* Node Weighed XFactor Sum                        */
    double MaxXFactor; /* max XFactor detected                            */
    int Bypass;        /* Number of Times Job was Bypassed                */
    int MaxBypass;     /* Max Number of Times Job was not Scheduled       */
} must_t;

typedef struct {
    char Name[MAXNAME]; /* cred name                     */
    mfs_t F; /* fairness policy config        */
    mcredl_t L; /* current resource usage        */
    must_t Stat; /* historical usage statistics   */
} cred_t;

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

/** Struct for showstats options */
typedef struct _showstats_info {
    int   type;                /**< Type*/
    char *id;                  /**< ID */
    int   flags;			   /**< Flags */
    char *pName;			   /**< Partition name */
} showstats_info_t;

static void free_structs(showstats_info_t *, client_info_t *);
static int process_args(int, char **, showstats_info_t *, client_info_t *);
static void print_usage();
static char *buildMsgBuffer(showstats_info_t *);
static int MStatSetAttr(must_t *, int, void **, int, int);
static int MStatFromXML(must_t *, mxml_t *);
static int MUGetIndex(char *, const char **, int , int );
static int showCStats(int, char *, int , int );
static int MOGetComponent(void *, int ,void **,  int );
static void *MOGetNextObject(void **, int, int, void *, char **);
static int MXMLFromString(mxml_t **, char *, char **, char *);
static int MXMLAddE(mxml_t *, mxml_t *);
static int MXMLSetAttr(mxml_t *E, char *A, void *V, enum MDataFormatEnum Format);
static int MSecDecompress(unsigned char *, unsigned int, unsigned char *, unsigned int,
		unsigned char **, char *);
static int MXMLDestroyE(mxml_t **);
static int MXMLCreateE(mxml_t **, char *);
static int PSDedicatedComp(cred_t *, cred_t *);
static char *MFSTargetToString(double, int);
static int MXMLGetChild(mxml_t *, char *, int *, mxml_t **);
static int MSecComp64BitToBufDecoding(char *, int, char *, int *);
static int MSecEncryption(char *, char *, int);
static int MCredSetAttr(void *, int, int, void **, int, int);
static int MOFromXML(void *, int, mxml_t *);
static int MLimitSetAttr(mcredl_t *, int, void **, int, int);
static int MLimitFromXML(mcredl_t *, mxml_t *);
static int MFSSetAttr(mfs_t *, int, void **, int, int);
static int MFSFromXML(mfs_t *, mxml_t *);
static int MFSTargetFromString(mfs_t *, char *);
static int showSchedulerStatistics(client_info_t *, int, char *);

int main(int argc, char **argv) {

    showstats_info_t showstats_info;
    client_info_t client_info;

	char *response, *msgBuffer, *ptr;
	int sd;
	long bufSize;
	char request[MAXBUFFER];

    memset(&showstats_info, 0, sizeof(showstats_info));
    memset(&client_info, 0, sizeof(client_info));

    showstats_info.type = -1;

    /* process all the options and arguments */
    if (process_args(argc, argv, &showstats_info, &client_info)) {

    	if (showstats_info.pName == NULL) {
    		if ((ptr = getenv(MSCHED_ENVPARVAR)) != NULL) {
    			showstats_info.pName = string_dup(ptr);
    		} else {
    			showstats_info.pName = string_dup(GLOBAL_MPARNAME);
    		}
    	}

		get_connection_params(&client_info);

		if (!connectToServer(&sd, client_info.port, client_info.host))
			exit(EXIT_FAILURE);

		msgBuffer = buildMsgBuffer(&showstats_info);
		generateBuffer(request, msgBuffer, "showstats");
		free(msgBuffer);

		if (!sendPacket(sd, request))
			exit(EXIT_FAILURE);

		if ((bufSize = getMessageSize(sd)) == 0)
			exit(EXIT_FAILURE);

		if ((response = (char *) calloc(bufSize + 1, 1)) == NULL) {
			puts("ERROR: cannot allocate memory for message");
			exit(EXIT_FAILURE);
		}

		/* receive message from server */
		if (!recvPacket(sd, &response, bufSize))
			exit(EXIT_FAILURE);

		ptr = strstr(response, "ARG=") + strlen("ARG=");

		switch (showstats_info.type) {
		        case mxoUser:
		        case mxoGroup:
		        case mxoAcct:
		        case mxoQOS:
		        case mxoClass:

		            showCStats(showstats_info.flags, ptr, strlen(ptr), showstats_info.type);

		            break;


		        case mxoNode:

		            printf("/n%s", ptr);

		            break;

		        case mxoSched:

		            showSchedulerStatistics(&client_info, showstats_info.flags, ptr);

		            break;

		        default:

		            /* NO-OP */

		            break;
		    }

		free(response);

    }

    free_structs(&showstats_info, &client_info);

    exit(0);
}

int showSchedulerStatistics(

		client_info_t *client_info, int flags, char *Buffer)

{
    char *ptr;

    long Time;
    long InitializationTime;
    int SchedRunTime;
    int IdleQueueSize;
    int RunnableJobs;
    int RunningJobs;
    int AvailableNodes;
    int AvailableProcs;
    int AvailableMem;

    int IdleNodes;
    int IdleProcs;
    int IdleMem;

    char TimeString[MAXLINE];
    char temp_str[MAXLINE];

    int TotalJobsCompleted;
    int SuccessfulJobsCompleted;
    double TotalProcHours;
    double DedicatedProcHours;
    double SuccessfulProcHours;
    long QueuePS;
    unsigned long AvgQueuePH;
    double PSRun;

    double MSAvail;

    double WeightedCpuAccuracy;
    double CpuAccuracy;
    double AvgXFactor;
    int Iteration;
    long RMPollInterval;
    int ActiveNodes;
    int BusyProcs;
    int BusyMem;

    double MinEfficiency;
    int MinEffIteration;
    double MaxXFactor;

    double AvgQTime;
    double MaxQTime;

    double AvgBypass;
    int MaxBypass;

    long StartTime;

    double PSDedicated;
    double PSUtilized;
    double MSDedicated;
    double MSUtilized;
    int JobsEvaluated;

    double PreemptPH;
    int PreemptJobs;

    int SMP;

    time_t tmpTime;

    if (getenv(MSCHED_ENVSMPVAR) != NULL)
        SMP = TRUE;
    else
        SMP = FALSE;

    ptr = strtok(Buffer, "\n");

    Time = strtol(ptr, NULL, 10);

    ptr = strtok(NULL, "\n");

    /*          STT INT RT IQ RJ RJ AN AP AM IN IP IM TC SJ TPH DPH SPH MSA MSD
     * QPS AQN WCA CAC PSX IT RPI MEF ME MXF ABP MB AQT MQT PSR PSD PSU MSA MSD
     * JE */

    sscanf(ptr,
           "%ld %ld %d %d %d %d %d %d %d %d %d %d %d %d %lf %lf %lf %lf %lf "
           "%ld %lu %lf %lf %lf %d %ld %lf %d %lf %lf %d %lf %lf %lf %lf %lf "
           "%lf %lf %d %d %lf",
           &StartTime, &InitializationTime,
           &SchedRunTime, /* Time in Hundredth of a Second */
           &IdleQueueSize, &RunnableJobs, &RunningJobs, &AvailableNodes,
           &AvailableProcs, &AvailableMem, &IdleNodes, &IdleProcs, &IdleMem,
           &TotalJobsCompleted, &SuccessfulJobsCompleted, &TotalProcHours,
           &DedicatedProcHours, &SuccessfulProcHours, &MSAvail, &MSDedicated,
           &QueuePS, &AvgQueuePH, &WeightedCpuAccuracy, &CpuAccuracy,
           &AvgXFactor, &Iteration, &RMPollInterval, &MinEfficiency,
           &MinEffIteration, &MaxXFactor, &AvgBypass, &MaxBypass, &AvgQTime,
           &MaxQTime, &PSRun, &PSDedicated, &PSUtilized, &MSDedicated,
           &MSUtilized, &JobsEvaluated, &PreemptJobs, &PreemptPH);

    ActiveNodes = AvailableNodes - IdleNodes;
    BusyProcs = AvailableProcs - IdleProcs;
    BusyMem = AvailableMem - IdleMem;

    fprintf(stdout, "\n");

    if (flags & (1 << VERBOSE)) {
        tmpTime = (time_t)Time;

        if (!strcmp(getConfigVal(client_info,"SERVERMODE"), "SIMULATION")) {
            sprintf(TimeString, "current scheduler time: %s", ctime(&tmpTime));

            TimeString[strlen(TimeString) - 1] = '\0';

            sprintf(temp_str, " (%ld)\n", Time);
            strcat(TimeString, temp_str);

            fprintf(stdout, "%s", TimeString);
        } else {
            fprintf(stdout, "current scheduler time: %s\n", ctime(&tmpTime));
        }
    } /* END if (Flags & (1 << mcmVerbose)) */

    fprintf(stdout, "%s active for   %11s  stats initialized on %s",
            MSCHED_SNAME, timeToString(SchedRunTime / 100),
            getDateString(&InitializationTime));

    if (flags & (1 << VERBOSE)) {
        fprintf(stdout, "statistics for iteration %5d  scheduler started on %s",
                Iteration, getDateString(&StartTime));
    }

    fprintf(stdout, "\n");

    fprintf(stdout, "Eligible/Idle Jobs:            %9d/%-9d (%.3f%c)\n",
            RunnableJobs, IdleQueueSize,
            (IdleQueueSize > 0) ? ((double)RunnableJobs * 100.0 / IdleQueueSize)
                                : 0.0,
            '%');

    fprintf(stdout, "Active Jobs:                   %9d\n", RunningJobs);

    fprintf(stdout, "Successful/Completed Jobs:     %9d/%-9d (%.3f%c)\n",
            SuccessfulJobsCompleted, TotalJobsCompleted,
            (TotalJobsCompleted > 0)
                ? ((double)SuccessfulJobsCompleted * 100.0 / TotalJobsCompleted)
                : 0.0,
            '%');

    if (flags & (1 << VERBOSE)) {
        fprintf(stdout, "Preempt Jobs:                  %9d\n", PreemptJobs);
    }

    fprintf(stdout, "Avg/Max QTime (Hours):         %9.2f/%-9.2f\n",
            AvgQTime / 3600.0, MaxQTime / 3600.0);

    fprintf(stdout, "Avg/Max XFactor:               %9.2f/%-9.2f\n", AvgXFactor,
            MaxXFactor);

    if (flags & (1 << VERBOSE)) {
        fprintf(stdout, "Avg/Max Bypass:                %9.2f/%-9.2f\n",
                AvgBypass, (double)MaxBypass);
    }

    fprintf(stdout, "\n");

    fprintf(stdout, "Dedicated/Total ProcHours:     %9.2f/%-9.2f (%.3f%c)\n",
            DedicatedProcHours, TotalProcHours,
            (TotalProcHours != 0.0)
                ? (DedicatedProcHours * 100.0 / TotalProcHours)
                : 0.0,
            '%');

    if (SMP == TRUE) {
        fprintf(stdout,
                "Dedicated/Total Mem (GBHours): %9.2f/%-9.2f (%.3f%c)\n",
                MSDedicated / 3600.0 / 1024.0, MSAvail / 3600.0 / 1024.0,
                (MSAvail > 0.0) ? (MSDedicated * 100.0 / MSAvail) : 0.0, '%');
    }

    if (flags & (1 << VERBOSE)) {
        fprintf(stdout,
                "Preempt/Dedicated ProcHours:   %9.2f/%-9.2f (%.3f%c)\n",
                PreemptPH, DedicatedProcHours,
                (DedicatedProcHours != 0.0)
                    ? (PreemptPH * 100.0 / DedicatedProcHours)
                    : 0.0,
                '%');
    }

    fprintf(stdout, "\n");

    fprintf(
        stdout, "Current Active/Total Procs:    %9d/%-9d (%.3f%c)\n", BusyProcs,
        AvailableProcs,
        (AvailableProcs > 0) ? (double)BusyProcs * 100.0 / AvailableProcs : 0.0,
        '%');

    if (SMP == TRUE) {
        fprintf(stdout,
                "Current Active/Total Mem (GB): %9.2f/%-9.2f (%.3f%c)\n",
                (double)BusyMem / 1024.0, (double)AvailableMem / 1024.0,
                (double)BusyMem * 100.0 / AvailableMem, '%');
    }

    if ((flags & (1 << VERBOSE)) && (AvailableProcs != AvailableNodes)) {
        fprintf(stdout, "Current Active/Total Nodes:    %9d/%-9d (%.3f%c)\n",
                ActiveNodes, AvailableNodes,
                (double)ActiveNodes * 100.0 / AvailableNodes, '%');
    }

    fprintf(stdout, "\n");

    if (SuccessfulProcHours > 0.0) {
        fprintf(stdout, "Avg WallClock Accuracy:        %8.3f%c\n",
                (TotalJobsCompleted > 0)
                    ? CpuAccuracy / TotalJobsCompleted * 100.0
                    : 0.0,
                '%');

        fprintf(stdout, "Avg Job Proc Efficiency:       %8.3f%c\n",
                (PSDedicated > 0.0) ? PSUtilized / PSDedicated * 100.0 : 0.0,
                '%');
    } else {
        fprintf(stdout, "Avg WallClock Accuracy:         %8s\n", "<N/A>");

        fprintf(stdout, "Avg Job Proc Efficiency:        %8s\n", "<N/A>");
    }

    if (flags & (1 << VERBOSE)) {
        if (MinEffIteration > 0) {
            fprintf(
                stdout,
                "Min System Utilization:        %8.3f%c (on iteration %d)\n",
                MinEfficiency, '%', MinEffIteration);

        } else {
            fprintf(stdout, "Min System Utilization:         %8s\n", "<N/A>");
        }
    }

    if (SuccessfulProcHours != 0) {
        fprintf(
            stdout, "Est/Avg Backlog (Hours):        %8.2f/%-8.2f\n",
            (double)QueuePS * (CpuAccuracy / TotalJobsCompleted) /
                (DedicatedProcHours / TotalProcHours * 3600 * AvailableProcs),
            (double)AvgQueuePH / AvailableProcs);
    } else {
        fprintf(stdout, "Est/Avg Backlog (Hours):        %8s/%-8s\n", "<N/A>",
                "<N/A>");
    }

    return (SUCCESS);
} /* END MCShowSchedulerStatistics() */

int MStatSetAttr(

    must_t *S,    /* I (modified) */
    int AIndex,   /* I */
    void **Value, /* I */
    int Format,   /* I */
    int Mode)     /* I */

{
    if (S == NULL) {
        return (FAILURE);
    }

    switch (AIndex) {
        case mstaTJobCount:

            S->Count = (int)strtol((char *)Value, NULL, 0);

            break;

        case mstaTNJobCount:

            S->NCount = (int)strtol((char *)Value, NULL, 0);

            break;

        case mstaTQueueTime:

            S->TotalQTS = strtol((char *)Value, NULL, 0);

            break;

        case mstaMQueueTime:

            S->MaxQTS = strtol((char *)Value, NULL, 0);

            break;

        case mstaTReqWTime:

            S->TotalRequestTime = strtod((char *)Value, NULL);

            break;

        case mstaTExeWTime:

            S->TotalRunTime = strtod((char *)Value, NULL);

            break;

        case mstaTMSAvl:

            S->MSAvail = strtod((char *)Value, NULL);

            break;

        case mstaTMSDed:

            S->MSDedicated = strtod((char *)Value, NULL);

            break;

        case mstaTPSReq:

            S->PSRequest = strtod((char *)Value, NULL);

            break;

        case mstaTPSExe:

            S->PSRun = strtod((char *)Value, NULL);

            break;

        case mstaTPSDed:

            S->PSDedicated = strtod((char *)Value, NULL);

            break;

        case mstaTPSUtl:

            S->PSUtilized = strtod((char *)Value, NULL);

            break;

        case mstaTJobAcc:

            S->JobAcc = strtod((char *)Value, NULL);

            break;

        case mstaTNJobAcc:

            S->NJobAcc = strtod((char *)Value, NULL);

            break;

        case mstaTXF:

            S->XFactor = strtod((char *)Value, NULL);

            break;

        case mstaTNXF:

            S->NXFactor = strtod((char *)Value, NULL);

            break;

        case mstaMXF:

            S->NXFactor = strtod((char *)Value, NULL);

            break;

        case mstaTBypass:

            S->Bypass = (int)strtol((char *)Value, NULL, 0);

            break;

        case mstaMBypass:

            S->MaxBypass = (int)strtol((char *)Value, NULL, 0);

            break;

        case mstaTQOSAchieved:

            S->QOSMet = (int)strtol((char *)Value, NULL, 0);

            break;

        case mstaInitTime:

            /* NYI */

            break;

        default:

            /* not handled */

            return (FAILURE);

            /*NOTREACHED*/

            break;
    } /* END switch(AIndex) */

    return (SUCCESS);
} /* END MStatSetAttr() */

int MStatFromXML(

    must_t *S, mxml_t *E)

{
    int aindex;
    int saindex;

    const char *MStatAttr[] = {NONE,   "TJC",  "TNJC", "TQT",  "MQT",  "TRT",
                               "TET",  "TMSA", "TMSD", "TPSR", "TPSE", "TPSD",
                               "TPSU", "TJA",  "TNJA", "TXF",  "TNXF", "MXF",
                               "TBP",  "MBP",  "TQM",  NULL};

    if ((S == NULL) || (E == NULL)) {
        return (FAILURE);
    }

    /* NOTE:  do not initialize.  may be overlaying data */

    for (aindex = 0; aindex < E->ACount; aindex++) {
        saindex = MUGetIndex(E->AName[aindex], MStatAttr, FALSE, 0);

        if (saindex == 0) continue;

        MStatSetAttr(S, saindex, (void **)E->AVal[aindex], mdfString, MSET);
    } /* END for (aindex) */

    return (SUCCESS);
} /* END MStatFromXML() */

int MUGetIndex(

    char *Value,          /* I */
    const char **ValList, /* I */
    int AllowSubstring,   /* I (boolean) */
    int DefaultValue)     /* I */

{

    int index;

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
        }
    } /* END for (index) */

    return (DefaultValue);
} /* END MUGetIndex() */

int showCStats(
	int flags,
    char *Buffer, /* I */
    int BufSize,  /* I */
    int OIndex)   /* I */

{
    int OCount;

    double JCPCT;
    double PHRPCT;
    double PHDPCT;
    double PHUPCT;

    char FSTarget[MAX_WORD];
    char Effic[MAX_WORD];

    char MaxXF[MAX_WORD];
    char XF[MAX_WORD];
    char QT[MAX_WORD];
    char JA[MAX_WORD];

    char JCount[MAX_WORD];
    char JCountP[MAX_WORD];
    char PHRQ[MAX_WORD];
    char PHRQP[MAX_WORD];
    char PHUtl[MAX_WORD];
    char PHUtlP[MAX_WORD];
    char PHDed[MAX_WORD];
    char PHDedP[MAX_WORD];

    char *tail;

    mxml_t *E = NULL;

    cred_t *tmpUP[MAX_MUSER];

    cred_t *tmpCred = NULL;

    char *NameP;

    void *O;
    void *OP;

    void *OE;
    int OS;

    mcredl_t *L;
    mfs_t *F;
    must_t *S;
    must_t tmpGS;

	char *mxo[] = { NONE, "acct", "am", "class", "cluster", "cp", "frame",
			"fs", "group", "job", "limits", "node", "par", "priority", "qos",
			"queue", "range", "req", "rsv", "rm", "sched", "sim", "sres",
			"stats", "sys", "user", NULL };

    if ((Buffer[0] == '\0') ||
        (MXMLFromString(&E, Buffer, &tail, NULL) == FAILURE)) {
        fprintf(stdout, "no %s statistics available\n", mxo[OIndex]);

        return (SUCCESS);
    }

    MStatFromXML(&tmpGS, E);

    MXMLDestroyE(&E);

    OCount = 0;

    /* extract cred stats */

	while (MXMLFromString(&E, tail, &tail, NULL) != FAILURE) {

		if (OCount == 0)
			tmpCred = (cred_t *) calloc(MAX_MGROUP, sizeof(cred_t));

		O = &tmpCred[OCount + 1];

		if (OIndex == USER) {
			tmpUP[OCount + 1] = &tmpCred[OCount + 1];
		}

		if (MOFromXML(O, OIndex, E) == FAILURE) {
			continue;
		}

		OCount++;

		MXMLDestroyE(&E);
    } /* END while (MXMLFromString() != FAILURE) */

    tmpUP[OCount + 1] = NULL;

    fprintf(stdout, "statistics initialized %s\n", strstr(Buffer, "[TIME]") + strlen("[TIME]"));

    fprintf(stdout,
            "         |------ Active ------|--------------------------------- "
            "Completed -----------------------------------|\n");

    /*               NAM JOB NOD PH  JOB PCT PHR PCT HRD PCT FST AXF MXF AQH EFF
     * WCA */
    fprintf(
        stdout,
        "%-9s %4s %5s %9s %4s %6s %6s %6s %6s %6s %5s %6s %6s %6s %6s %6s\n",
        mxo[OIndex], "Jobs", "Procs", "ProcHours", "Jobs", "  %  ", "PHReq",
        "  %  ", "PHDed", "  %  ", "FSTgt", "AvgXF", "MaxXF", "AvgQH", "Effic",
        "WCAcc");

    /* sort by dedicated PS */


    OS = 0;

    qsort((void *)&tmpCred[1], OCount, sizeof(cred_t),
          (int (*)())PSDedicatedComp);

    if (OIndex == USER) {
		OP = &tmpUP[0];
		OE = (void *) &tmpUP[OCount + 1];
	} else {
		OP = &tmpCred[0];
		OE = (void *) &tmpCred[OCount + 1];
	}

    while ((O = MOGetNextObject(&OP, OIndex, OS, OE, &NameP)) != NULL) {
        MOGetComponent(O, OIndex, (void *)&S, mxoStats);
        MOGetComponent(O, OIndex, (void *)&F, mxoFS);
        MOGetComponent(O, OIndex, (void *)&L, mxoLimits);
        strcpy(Effic, "------");
        strcpy(FSTarget, "-----");
        strcpy(MaxXF, "------");
        strcpy(XF, "------");
        strcpy(XF, "------");
        strcpy(QT, "------");
        strcpy(JA, "------");
        strcpy(JCount, "----");
        strcpy(JCountP, "------");
        strcpy(PHRQ, "------");
        strcpy(PHRQP, "------");
        strcpy(PHUtl, "------");
        strcpy(PHUtlP, "------");
        strcpy(PHDed, "------");
        strcpy(PHDedP, "------");

        JCPCT = (tmpGS.Count > 0) ? ((double)S->Count / tmpGS.Count * 100.0) : 0.0;
        PHRPCT = (tmpGS.PSRequest > 0)
                     ? ((double)S->PSRequest / tmpGS.PSRequest * 100.0)
                     : 0.0;
        PHDPCT = (tmpGS.PSDedicated > 0.0)
                     ? ((double)S->PSDedicated / tmpGS.PSDedicated * 100.0)
                     : 0.0;
        PHUPCT = (tmpGS.PSUtilized > 0.0)
                     ? ((double)S->PSUtilized / tmpGS.PSUtilized * 100.0)
                     : 0.0;

        if (F->FSTarget > 0.0) {
            sprintf(FSTarget, "%5.5s",
                    MFSTargetToString(F->FSTarget, F->FSMode));
        }

        if (S->PSDedicated > 0.0) {
            sprintf(PHUtl, "%6.1f", (double)S->PSUtilized / 3600.0);
            sprintf(PHDed, "%6.1f", (double)S->PSDedicated / 3600.0);

            sprintf(PHUtlP, "%6.2f", PHUPCT);
            sprintf(PHDedP, "%6.2f", PHDPCT);

            sprintf(Effic, "%6.2f",
                    ((double)S->PSUtilized / S->PSDedicated * 100.0));
        }

        if (S->Count > 0) {
            sprintf(JCount, "%4d", S->Count);
            sprintf(JCountP, "%6.2f", JCPCT);

            sprintf(PHRQ, "%6.1f", (double)S->PSRequest / 3600);
            sprintf(PHRQP, "%6.2f", PHRPCT);

            sprintf(MaxXF, "%6.2f", ((double)S->MaxXFactor));
            sprintf(XF, "%6.2f", MIN(999.99, ((double)S->XFactor / S->Count)));
            sprintf(QT, "%6.2f", ((double)S->TotalQTS / S->Count / 3600));
            sprintf(JA, "%6.2f", (S->JobAcc / S->Count * 100.0));
        }

		if ((L->Usage[mptMaxJob][0] == 0) && (S->PSDedicated == 0.0)
				&& !(flags & (1 << VERBOSE))) {

			continue;
		}

        /*               NAM  ID RJB RND RUNPH JCT JCPCT RPH RQPCT DEDPH DEPCT
         * TARGT AVGXF MAXXF AVGQT EFFIC AVGJA */

        fprintf(stdout,
                "%-9s %4d %5d %9.2f %4s %6s %5s %6s %6s %6s %5s %6s %6s %6s "
                "%6s %6s\n",
                NameP, L->Usage[mptMaxJob][0], L->Usage[mptMaxProc][0],
                (double)L->Usage[mptMaxPS][0] / 3600.0, JCount, JCountP,
                PHRQ, PHRQP, PHDed, PHDedP, FSTarget, XF, MaxXF, QT, Effic, JA);




    } /* END while (MOGetNextObject() != NULL) */

    fprintf(stdout, "\n\n");

    return (SUCCESS);
} /* END MCShowCStats() */

int MOFromXML(

    void *O,    /* I (modified) */
    int OIndex, /* I */
    mxml_t *E)  /* I */

{
    int aindex;
    int cindex;

    int caindex;

    static mfs_t *F;
    static mcredl_t *L;
    static must_t *S;

    static void *CO = NULL;

    const char *MXO[] = {NONE,    "acct",     "am",    "class", "cluster", "cp",
                         "frame", "fs",       "group", "job",   "limits",  "node",
                         "par",   "priority", "qos",   "queue", "range",   "req",
                         "rsv",   "rm",       "sched", "sim",   "sres",    "stats",
                         "sys",   "user",     NULL};

    mxml_t *C;

    if ((O == NULL) || (E == NULL)) {
        return (FAILURE);
    }

    /* NOTE:  do not initialize.  may be overlaying data */

    /* set attributes */

	for (aindex = 0; aindex < E->ACount; aindex++) {

		MCredSetAttr(O, OIndex, ID, (void **) E->AVal[aindex], mdfString,
				MSET);
	}


    if (O != CO) {
        if ((MOGetComponent(O, OIndex, (void *)&F, mxoFS) == FAILURE) ||
            (MOGetComponent(O, OIndex, (void *)&L, mxoLimits) == FAILURE) ||
            (MOGetComponent(O, OIndex, (void *)&S, mxoStats) == FAILURE)) {
            /* cannot get components */

            return (FAILURE);
        }

        CO = 0;
    }

    for (cindex = 0; cindex < E->CCount; cindex++) {
        C = E->C[cindex];

        caindex = MUGetIndex(C->Name, MXO, FALSE, 0);

        if (caindex == 0) continue;

        switch (caindex) {
            case mxoStats:

                MStatFromXML(S, C);

                break;

            case mxoLimits:

                MLimitFromXML(L, C);

                break;

            case mxoFS:

                MFSFromXML(F, C);

                break;

            default:

                break;
        } /* END switch(caindex) */
    }     /* END for (cindex) */

    return (SUCCESS);
} /* END MOFromXML() */

int MFSFromXML(

    mfs_t *F,  /* I (modified) */
    mxml_t *E) /* I */

{
    int aindex;
    int saindex;

    const char *MFSAttr[] = {NONE, "Target", NULL};

    if ((F == NULL) || (E == NULL)) {
        return (FAILURE);
    }

    /* NOTE:  do not initialize.  may be overlaying data */

    for (aindex = 0; aindex < E->ACount; aindex++) {
        saindex = MUGetIndex(E->AName[aindex], MFSAttr, FALSE, 0);

        if (saindex == 0) continue;

        MFSSetAttr(F, saindex, (void **)E->AVal[aindex], mdfString, MSET);
    } /* END for (aindex) */

    return (SUCCESS);
} /* END MFSFromXML() */

int MFSSetAttr(

    mfs_t *F, int AIndex, void **Value, int Format, int Mode)

{
	enum MFSAttrType { mfsaNONE = 0, mfsaTarget };

    if (F == NULL) {
        return (FAILURE);
    }

    switch (AIndex) {
        case mfsaTarget:

            MFSTargetFromString(F, (char *)Value);

            break;

        default:

            /* not handled */

            return (FAILURE);

            /*NOTREACHED*/

            break;
    } /* switch(AIndex) */

    return (SUCCESS);
} /* MFSSetAttr() */

int MFSTargetFromString(

    mfs_t *F,  /* I */
    char *Buf) /* O */

{
    char *tail;

    /* FairShare modes */

	enum {
		mfstNONE = 0, mfstTarget, mfstFloor, mfstCeiling, mfstCapRel, mfstCapAbs
	};

    if ((F == NULL) || (Buf == NULL)) {
        return (FAILURE);
    }

    F->FSTarget = strtod(Buf, &tail);

    if (tail != NULL) {
        if (*tail == '+')
            F->FSMode = mfstFloor;
        else if (*tail == '-')
            F->FSMode = mfstCeiling;
        else if (*tail == '^')
            F->FSMode = mfstCapAbs;
        else if (*tail == '%')
            F->FSMode = mfstCapRel;
        else
            F->FSMode = mfstTarget;
    } else {
        F->FSMode = mfstTarget;
    }

    return (SUCCESS);
} /* END MFSTargetFromString() */

int MLimitFromXML(

    mcredl_t *L, mxml_t *E)

{
    int aindex;
    int saindex;

    const char *MLimitAttr[] = {NONE, "UJobs", "UProcs", "UPS", NULL};

    if ((L == NULL) || (E == NULL)) return (FAILURE);

    /* NOTE:  do not initialize.  may be overlaying data */

    for (aindex = 0; aindex < E->ACount; aindex++) {
        saindex = MUGetIndex(E->AName[aindex], MLimitAttr, FALSE, 0);

        if (saindex == 0) continue;

        MLimitSetAttr(L, saindex, (void **)E->AVal[aindex], mdfString, MSET);
    } /* END for (aindex) */

    return (SUCCESS);
} /* END MStatFromXML() */

int MLimitSetAttr(

    mcredl_t *L, int AIndex, void **Value, int Format, int Mode)

{
    if (L == NULL) return (FAILURE);

    enum MLimitAttrType { mlaNONE = 0, mlaAJobs, mlaAProcs, mlaAPS };

    switch (AIndex) {
        case mlaAJobs:

            L->Usage[mptMaxJob][0] = (int)strtol((char *)Value, NULL, 0);

            break;

        case mlaAProcs:

            L->Usage[mptMaxProc][0] = (int)strtol((char *)Value, NULL, 0);

            break;

        case mlaAPS:

            L->Usage[mptMaxPS][0] = (int)strtol((char *)Value, NULL, 0);

            break;

        default:

            /* not handled */

            return (FAILURE);

            /*NOTREACHED*/

            break;
    } /* switch(AIndex) */

    return (SUCCESS);
} /* MLimitSetAttr() */

int MCredSetAttr(

void *O, /* I (modified) */
int OIndex, /* I */
int AIndex, /* I */
void **Value, /* I */
int Format, /* I */
int Mode) /* I (MSET/mAdd/mClear) */

{
	mcredl_t *L;
	must_t *S;

	if ((O == NULL) || (Value == NULL)) {
		return (FAILURE);
	}

	if (MOGetComponent(O, OIndex, (void *) &S, mxoStats) == FAILURE) {
		/* cannot get components */

		return (FAILURE);
	}

	if (MOGetComponent(O, OIndex, (void *) &L, mxoLimits) == FAILURE) {
		/* cannot get components */

		return (FAILURE);
	}

	strcpy(((cred_t *) O)->Name, (char *) Value);

	return (SUCCESS);
} /* END MCredSetAttr() */

char *MFSTargetToString(

    double FSTarget, /* I */
    int FSMode)      /* I */

{
    const char *MFSTType = "  +-%^";

    static char Line[MAXLINE];

    sprintf(Line, "%.2lf%c", FSTarget, MFSTType[FSMode]);

    return (Line);
} /* END MFSTargetToString() */

int PSDedicatedComp(

    cred_t *a, cred_t *b)

{
    static int tmp;

    tmp = (int)((b->Stat.PSDedicated - a->Stat.PSDedicated) * 1000);

    return (tmp);
}

int MOGetComponent(

    void *O,    /* I */
    int OIndex, /* I */
    void **C,   /* O */
    int CIndex) /* I */

{
    if ((O == NULL) || (C == NULL)) {
        return (FAILURE);
    }

    if (C != NULL) *C = NULL;

    if (CIndex == mxoLimits)
        *C = (void *)&((cred_t *)O)->L;
    else if (CIndex == mxoFS)
        *C = (void *)&((cred_t *)O)->F;
    else if (CIndex == mxoStats)
        *C = (void *)&((cred_t *)O)->Stat;

    if (*C == NULL) {
        return (FAILURE);
    }

    return (SUCCESS);
} /* END MOGetComponent() */

void *MOGetNextObject(

    void **O, /* I */
    int OIndex, int OSize, void *OEnd, char **NameP)

{
    if ((*O >= OEnd) && (OIndex != mxoSched)) {
        *O = NULL;

        return (*O);
    }

    if(OIndex == USER){
        cred_t **UP;

        for (UP = *(cred_t ***)O + 1; UP < (cred_t **)OEnd; UP += 1) {
            if ((*UP != NULL) && ((*UP)->Name[0] > '\1')) {
                *O = (void *)UP;

                if (NameP != NULL) *NameP = (*UP)->Name;

                return ((void *)*UP);
            }
        } /* END for (OP) */
    }else{
        cred_t *c;
        for (c = *(cred_t **)O + 1; c < (cred_t *)OEnd; c += 1) {

            if ((c != NULL) && (c->Name[0] > '\1')) {

                *O = (void *)c;
                if (NameP != NULL) *NameP = c->Name;

                return (*O);
            }
        } /* END for (A) */
    }

    *O = NULL;

    return (*O);
} /* END MOGetNextObject() */

int MXMLFromString(

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

    char tmpNLine[MAXLINE + 1];
    char tmpVLine[MAXBUFFER + 1];

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

        if ((index >= MAXLINE) || (ptr[0] == '\0')) {
            if (EMsg != NULL)
                sprintf(EMsg, "element name is too long - %.10s", tmpNLine);

            return (FAILURE);
        }
    }

    tmpNLine[index] = '\0';

    if ((*EP == NULL) && (MXMLCreateE(EP, tmpNLine) == FAILURE)) {
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

            if (index >= MAXLINE) break;
        }

        tmpNLine[index] = '\0';

        if (*ptr != '\0') ptr++; /* skip '=' */

        if (*ptr != '\0') ptr++; /* skip '"' */

        if (*ptr == '\0') {
            if (EMsg != NULL)
                sprintf(EMsg, "string is corrupt - early termination");

            MXMLDestroyE(EP);

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

            if ((index >= MAXBUFFER) || (*ptr == '\0')) {
                MXMLDestroyE(EP);

                /* locate tail */

                if (Tail != NULL) *Tail = ptr + strlen(ptr);

                if (EMsg != NULL)
                    sprintf(EMsg, "attribute name is too long - %.10s",
                            tmpVLine);

                return (FAILURE);
            }
        }

        tmpVLine[index] = '\0';

        MXMLSetAttr(E, tmpNLine, (void *)tmpVLine, mdfString);

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

            MSecDecompress((unsigned char *)ptr, (unsigned int)len, NULL,
                           (unsigned int)0, (unsigned char **)&tmpBuf, NULL);

            /* process expanded buffer (guaranteed to be a single element */

            if ((MXMLFromString(&C, tmpBuf, NULL, EMsg) == FAILURE) ||
                (MXMLAddE(E, C) == FAILURE)) {
                MXMLDestroyE(EP);

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

            if (index >= MAXBUFFER) break;
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
            char tmpCName[MAXNAME];

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

                if ((index >= MAXLINE) || (ptr2[0] == '\0')) {
                    if (EMsg != NULL)
                        sprintf(EMsg, "element name is too long - %.10s",
                                tmpCName);

                    MXMLDestroyE(EP);

                    return (FAILURE);
                }
            }

            tmpCName[index] = '\0';

            MXMLGetChild(E, tmpCName, NULL, &C);
        }

        if ((MXMLFromString(&C, ptr, &tail, EMsg) == FAILURE) ||
            (MXMLAddE(E, C) == FAILURE)) {
            break;
        }

        ptr = tail;

        if ((ptr == NULL) || (ptr[0] == '\0')) {
            /* XML is corrupt */

            if (Tail != NULL) *Tail = ptr;

            if ((EMsg != NULL) && (EMsg[0] == '\0'))
                strcpy(EMsg, "cannot extract child");

            MXMLDestroyE(EP);

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

            MSecDecompress((unsigned char *)ptr, (unsigned int)len, NULL,
                           (unsigned int)0, (unsigned char **)&tmpBuf, NULL);

            /* process expanded buffer (guaranteed to be a single element */

            if ((MXMLFromString(&C, tmpBuf, NULL, EMsg) == FAILURE) ||
                (MXMLAddE(E, C) == FAILURE)) {
                MXMLDestroyE(EP);

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

            if ((index >= MAXBUFFER) || (*ptr == '\0')) {
                if (EMsg != NULL)
                    sprintf(EMsg, "cannot load value line - %.10s (%s)",
                            tmpVLine,
                            (index >= MAXBUFFER) ? "too long" : "corrupt");

                MXMLDestroyE(EP);

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

        MXMLDestroyE(EP);

        return (FAILURE);
    }

    ptr++; /* ignore '>' */

    if (Tail != NULL) *Tail = ptr;

    return (SUCCESS);
} /* END MXMLFromString() */

int MXMLGetChild(

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
} /* END MXMLGetChild() */

int MXMLAddE(

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
} /* END MXMLAddE() */

int MXMLSetAttr(

    mxml_t *E,                   /* I (modified) */
    char *A,                     /* I */
    void *V,                     /* I */
    enum MDataFormatEnum Format) /* I */

{
    int aindex;
    int iindex;

    int rc;

    char tmpLine[MAXLINE];

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
} /* END MXMLSetAttr() */

int MSecComp64BitToBufDecoding(

    char *IBuf, int IBufLen, char *OBuf, int *OBufLen)

{
    int IIndex = 0;
    int OIndex = 0;

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
} /* MSecComp64BitToBufDecoding() */

int MSecDecompress(

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
            ((OutBuf = (unsigned char *)calloc(MAXBUFFER, 1)) == NULL)) {
            return (FAILURE);
        } else {
            OutBufSize = MAXBUFFER;
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

    MSecComp64BitToBufDecoding((char *)SrcString, (int)SrcSize, (char *)tmpBuf,
                               (int *)&SrcSize);

    if (EKey != NULL) MSecEncryption((char *)tmpBuf, EKey, SrcSize);

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
} /* END MSecDecompress() */

int MSecEncryption(

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
} /* END MSecEncryption() */

int MXMLDestroyE(

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

            MXMLDestroyE(&E->C[index]);
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
} /* END MXMLDestroyE() */

int MXMLCreateE(

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
} /* END MXMLCreateE() */

/* combine and save information into a buffer */
char *buildMsgBuffer(showstats_info_t *showstats_info) {
	char *buffer;
	int len = 0;

	if (showstats_info->type == -1)
		showstats_info->type = mxoSched;

	if (showstats_info->id == NULL)
		showstats_info->id = string_dup(NONE);

	if (showstats_info->type == mxoNode) {
		/* set default EvaluationDepth to MAX_MJOB */

		if (showstats_info->flags < (1 << 1))
			showstats_info->flags |= (MAXJOB << 1);
	}

	/* calculate the length of the whole buffer */

	/* plus one for a white space */
	len += strlen(showstats_info->id) + 1;
	len += strlen(showstats_info->pName) + 1;

	/* reserve extra space for numbers */
	if ((buffer = (char *) malloc(len + 8)) == NULL) {
		puts("ERROR: cannot allocate memory for buffer");
		return NULL;
	}

	/* build buffer */
	sprintf(buffer, "%d %s %s %d", showstats_info->type, showstats_info->id,
			showstats_info->pName, showstats_info->flags);

	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 showstats_info_t *showstats_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"acount",      no_argument,       0, 'a'},
            {"class", 		no_argument,       0, 'c'},
            {"depth",  		required_argument, 0, 'd'},
            {"group",  		no_argument,       0, 'g'},
            {"node", 		no_argument,       0, 'n'},
            {"qos", 		no_argument,       0, 'q'},
			{"scheduler",	no_argument,       0, 's'},
			{"summary", 	no_argument,       0, 'S'},
			{"user", 		no_argument,       0, 'u'},
            {"verbose",     no_argument,       0, 'v'},
            {"configfile",  required_argument, 0, 'C'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVacd:gnqsSuvC:H:P:",
                         options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {

          case 'h':
              print_usage();
              exit(EXIT_SUCCESS);
              break;

          case 'V':
              printf("Maui version %s\n", MSCHED_VERSION);
              exit(EXIT_SUCCESS);
              break;

          case 'a':
        	  showstats_info->type = mxoAcct;
              break;

          case 'c':
        	  showstats_info->type = mxoClass;
              break;

          case 'd':
        	  showstats_info->flags |= atoi(optarg);
              break;

          case 'g':
        	  showstats_info->type = mxoGroup;
              break;

          case 'n':
        	  showstats_info->type = mxoNode;
              break;

          case 'q':
        	  showstats_info->type = mxoQOS;
              break;

          case 's':
        	  showstats_info->type = mxoSched;
              break;

          case 'S':
        	  showstats_info->flags |= 1;
              break;

          case 'u':
        	  showstats_info->type = mxoUser;
              break;

          case 'v':
        	  showstats_info->flags |= (1 << VERBOSE);
              break;

          case 'C':
              printf ("set configfile to %s\n", optarg);
              client_info->configfile = string_dup(optarg);
              break;

          case 'H':
              printf ("set host to %s\n", optarg);
              client_info->host = string_dup(optarg);
              break;

          case 'P':
              client_info->port = string2int(optarg);
              if (client_info->port != INVALID_STRING)
                  printf ("set port to %s\n", optarg);
              break;

          case '?':
              /* getopt_long already printed an error message. */
              puts ("Try 'showstats --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

    /* only accept one argument */
    if(optind < argc - 1){
        print_usage();
        exit(EXIT_FAILURE);
	} else if (optind == argc - 1) {

		/* copy and save job id from input */
		showstats_info->id = string_dup(argv[optind]);
	}
    return 1;
}

/* free memory */
void free_structs(showstats_info_t *showstats_info, client_info_t *client_info) {
    free(showstats_info->id);
    free(showstats_info->pName);
    free(client_info->configfile);
    free(client_info->host);
}

void print_usage()
{
    puts ("\nUsage: showstats [FLAGS]\n\n"
            "Display detailed job state information and diagnostic output for a specified job.\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n"
    		"\n"
    		"  -a, --acount [ACCOUNTID]       display account statistics\n"
    		"  -c, --class [CLASSID]          display class statistics\n"
    		"  -g, --group [GROUPID]          display group statistics\n"
    		"  -n, --node [-d VALUE] [-S]	  display node statistics; use flag 'd' to set evaluation\n"
    		"                                   depth and flag 'S' to only display summary\n"
    		"  -q, --qos [QOSID]              display QoS statistics\n"
    		"  -s, --scheduler [-v]			  display general scheduler statistics; use flag 'v' to\n"
    		"                                   display additional information\n"
    		"  -u, --user [USERID]            display user statistics\n");
    print_client_usage();
}
