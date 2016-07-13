/*
 * showq standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

#define IDLE 1
#define RUNNING 2
#define BLOCKED 3
#define SPVIOLATION 18
#define BACKFILL 2
#define PREEMPTEE 12

/** Struct for showq options */
typedef struct _showq_info {
    char  *pName;               /**< Partition name */
    char  *username;            /**< User name */
    int   idle;                 /**< Show idle jobs */
    int   running;              /**< Show running jobs */
    int   blocked;              /**< Show blocked jobs */
} showq_info_t;

// local convenience functions

static int process_args(int, char **, showq_info_t *, client_info_t *);
static void print_usage();
static void free_structs(showq_info_t *, client_info_t *);
static char *buildMsgBuffer(showq_info_t);
static int showAQueue(char *);
static int showIQueue(char *, char *);
static int showRQueue(char *, char *, int );
static int showBQueue(char *);

int main (int argc, char **argv)
{
    showq_info_t showq_info;
    client_info_t client_info;

    char *response, request[MAXBUFFER], *msgBuffer;
    int sd, port, displayFlags;
    long bufSize;;
    FILE *f;
    char configDir[MAXLINE];
    char *host, *ptr;

    memset(&showq_info, 0, sizeof(showq_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &showq_info, &client_info)) {

    	if (showq_info.pName == NULL) {
    		if ((ptr = getenv(MSCHED_ENVPARVAR)) != NULL) {
    			showq_info.pName = string_dup(ptr);
    		} else {
    			showq_info.pName = string_dup(GLOBAL_MPARNAME);
    		}
    	}

		/* get config file directory and open it*/
		strcpy(configDir, MBUILD_HOMEDIR);
		if (client_info.configfile != NULL) {
			printf("will use %s as configfile instead of default\n",
					client_info.configfile);
			strcat(configDir, client_info.configfile);
		} else {
			strcat(configDir, CONFIGFILE);
		}
		if ((f = fopen(configDir, "rb")) == NULL) {
			puts("ERROR: cannot locate config file");
			exit(EXIT_FAILURE);
		}

		if (client_info.host != NULL) {
			printf("will contact %s as maui server instead of default\n",
					client_info.host);
			host = client_info.host;
		} else {
			host = getConfigVal(f, "SERVERHOST");
		}

		if (client_info.port > 0) {
			printf("will use %d as server port instead of default\n",
					client_info.port);
			port = client_info.port;
		} else {
			port = atoi(getConfigVal(f, "SERVERPORT"));
		}

		if((ptr = getConfigVal(f, "DISPLAYFLAGS")) != NULL){
			if (strstr(ptr, "NODECENTRIC") != NULL){
				displayFlags = TRUE;
			}
		}
		free(ptr);

		fclose(f);

		if (!connectToServer(&sd, port, host))
			exit(EXIT_FAILURE);

		msgBuffer = buildMsgBuffer(showq_info);
		generateBuffer(request, msgBuffer, "showq");
		free(msgBuffer);

		if (!sendPacket(sd, request))
			exit(EXIT_FAILURE);

		if ((bufSize = getMessageSize(sd)) == 0)
			exit(EXIT_FAILURE);

		if ((response = (char *) calloc(bufSize + 1, 1)) == NULL) {
			puts("ERROR: cannot allocate memory for message");
			exit(EXIT_FAILURE);
		}

		/* receive message from server*/
		if (!recvPacket(sd, &response, bufSize))
			exit(EXIT_FAILURE);

        /* if no flag has been set, print jobs in all states */
		if (showq_info.running + showq_info.idle + showq_info.blocked < 1) {
			showAQueue(response);
		} else {
			if (showq_info.idle) {
				puts("idle jobs:");
				showIQueue(response, showq_info.pName);
			}

			if (showq_info.running){
				if (showq_info.idle)
					puts("");
				puts("running jobs:");
				showRQueue(response, showq_info.pName, displayFlags);
			}

			if (showq_info.blocked){
				if (showq_info.idle || showq_info.running)
					puts("");
				puts("blocked jobs:");
				showBQueue(response);
			}
		}

		free(host);
		free(response);
    }

    free_structs(&showq_info,&client_info);

    return 0;
}

/* show all queue */
int showAQueue(char *buffer) {
	char *ptr, name[MAXNAME], tmpQOS[MAXNAME], UserName[MAXNAME], tmp[MAXLINE];
	long stime, qtime, now, wCLimit;
	int procs, count, priority, state;
	int upProcs, idleProcs, upNodes, idleNodes, activeNodes;
	int busyNodes, busyProcs, acount, icount, ncount, rc;
	const char *jobState[] = { "NONE", "Idle", "Starting", "Running" };

	buffer = strstr(buffer, "ARG=") + strlen("ARG=");

	/* get present time */
	rc = sscanf(buffer, "%ld %d %d %d %d %d %d\n", &now, &upProcs, &idleProcs,
			&upNodes, &idleNodes, &activeNodes, &busyProcs);

	busyNodes = activeNodes;
	busyProcs = MIN(upProcs, busyProcs);

	ptr = strtok(buffer, "\n");

	/* display list of active jobs */
	fprintf(stdout, "ACTIVE JOBS--------------------\n");
	fprintf(stdout, "%-18s %8s %10s %5s %11s %20s\n\n", "JOBNAME", "USERNAME",
			"STATE", "PROC", "REMAINING", "STARTTIME");

	/* read all active jobs */
	acount = 0;

	while ((ptr = strtok(NULL, "\n")) != NULL) {
		if (!strcmp(ptr, "[ENDACTIVE]"))
			break;

		acount++;
		count++;

		/* Format:  <JOBNAME> <USERNAME> <START TIME> <QUEUE TIME> <PROCS>
		 * <CPULIMIT> <QOS> <STATE> <PRIO> */
		rc = sscanf(ptr, "%s %s %ld %ld %d %ld %s %d %d", name, UserName,
				&stime, &qtime, &procs, &wCLimit, tmpQOS, &state, &priority);

		if (rc != 9)
			continue;

		/* display job */
		fprintf(stdout, "%-18s %8s %10s %5d %11s  %19s", name, UserName,
				jobState[state], procs, timeToString(wCLimit - (now - stime)),
				getDateString(&stime));
	}

	sprintf(tmp, "%d Active Job%c   ", acount, (acount == 1) ? ' ' : 's');

	fprintf(stdout, "\n%21s %4d of %4d Processors Active (%.2f%c)\n", tmp,
			busyProcs, upProcs,
			(upProcs > 0) ? (double) busyProcs / upProcs * 100.0 : 0.0, '%');

	if ((upNodes > 0) && (upProcs != upNodes)) {
		fprintf(stdout, "%21s %4d of %4d Nodes Active      (%.2f%c)\n", " ",
				busyNodes, upNodes,
				(upNodes > 0) ? (double) busyNodes / upNodes * 100.0 : 0.0,
				'%');
	}

	/* display list of idle jobs */
	fprintf(stdout, "\nIDLE JOBS----------------------\n");

	fprintf(stdout, "%-18s %8s %10s %5s %11s %20s\n\n", "JOBNAME", "USERNAME",
			"STATE", "PROC", "WCLIMIT", "QUEUETIME");

	/* read all idle jobs */
	icount = 0;

	while ((ptr = strtok(NULL, "\n")) != NULL) {
		if (!strcmp(ptr, "[ENDIDLE]"))
			break;

		count++;
		icount++;

		/* Format:  <JOBNAME> <USERNAME> <START TIME> <QUEUE TIME> <PROCS>
		 * <CPULIMIT> <QOS> <STATE> <PRIO> */
		rc = sscanf(ptr, "%s %s %ld %ld %d %ld %s %d %d", name, UserName,
				&stime, &qtime, &procs, &wCLimit, tmpQOS, &state, &priority);

		/* display job */
		if (rc != 9)
			continue;

		fprintf(stdout, "%-18s %8s %10s %5d %11s  %19s", name, UserName,
				jobState[state], procs, timeToString(wCLimit),
				getDateString(&qtime));
	}

	fprintf(stdout, "\n%d Idle Job%c\n", icount, (icount == 1) ? ' ' : 's');

	/* display list of non-queued jobs */
	fprintf(stdout, "\nBLOCKED JOBS----------------\n");

	fprintf(stdout, "%-18s %8s %10s %5s %11s %20s\n\n", "JOBNAME", "USERNAME",
			"STATE", "PROC", "WCLIMIT", "QUEUETIME");

	/* read all blocked jobs */
	ncount = 0;

	while ((ptr = strtok(NULL, "\n")) != NULL) {
		if (!strcmp(ptr, "[ENDBLOCKED]")) {
			break;
		}

		count++;
		ncount++;

		/* Format:  <JOBNAME> <USERNAME> <START TIME> <QUEUE TIME> <PROCS>
		 * <CPULIMIT> <QOS> <STATE> <PRIORITY>  */
		rc = sscanf(ptr, "%s %s %ld %ld %d %ld %s %d %d", name, UserName,
				&stime, &qtime, &procs, &wCLimit, tmpQOS, &state, &priority);

		if (rc != 9)
			continue;

		/* display job */
		fprintf(stdout, "%-18s %8s %10s %5d %11s  %19s", name, UserName,
				(state > 0) ? jobState[state] : "-", procs,
				timeToString(wCLimit), getDateString(&qtime));
	}

	fprintf(stdout,
			"\nTotal Jobs: %d   Active Jobs: %d   Idle Jobs: %d   Blocked "
					"Jobs: %d\n", count, acount, icount, ncount);

	while ((ptr = strtok(NULL, "\n")) != NULL) {
		fprintf(stdout, "\n%s\n", ptr);
	}

	return 1;
}

/* show idle queue*/
int showIQueue(char *msgBuffer, char *parName) {
	char *ptr, *buffer, name[MAXNAME], userName[MAXNAME], groupName[MAXNAME];
	long qtime, startTime;
	int minprocs, dmemory, wCLimit, count, priority, totalProcs, SMP;
	char tmpQOS[MAXNAME], jobState[MAXNAME], jobClass[MAXNAME],
			SMPLine[MAXNAME];
	double totalLoad, xfactor;

	if (getenv(MSCHED_ENVSMPVAR) != NULL)
		SMP = TRUE;
	else
		SMP = FALSE;

	buffer = (char *) malloc(strlen(msgBuffer) + 1);
	strcpy(buffer,msgBuffer);
	buffer = strstr(buffer, "ARG=") + strlen("ARG=");

	SMPLine[0] = '\0';

	/* display prioritized list of idle jobs */
	if (strcmp(parName, GLOBAL_MPARNAME) != 0) {
		fprintf(stdout, "Partition: %s\n", parName);
	}

	fprintf(stdout, "%18s %11s %8s %2s %9s %8s %6s %s%11s %9s %20s\n\n",
			"JobName", "Priority", "XFactor", "Q", "User", "Group", "Procs",
			(SMP == TRUE) ? "Memory " : "", "WCLimit", "Class",
			"SystemQueueTime");

	count = 0;

	/* get current time */
	sscanf(buffer, "%ld %d\n", &startTime, &totalProcs);

	ptr = strtok(buffer, "\n");

	totalLoad = 0.0;

	while ((ptr = strtok(NULL, "\n")) != NULL) {
		if (strstr(ptr, "Truncated")) {
			fprintf(stderr,
					"WARNING:  idle job list is too long  (list truncated)\n");

			break;
		}

		if (!strcmp(ptr, "[BLOCKED]") || !strcmp(ptr, "[RUNNING]"))
			break;

		count++;

		/* Format:  <JOBNAME> <USERNAME> <GROUPNAME> <QUEUETIME> <MINPROCS>
		 * <CPULIMIT> <PRIORITY> <QOS> <JOB-STATE> */
		sscanf(ptr, "%s %s %s %ld %d %d %d %d %s %s %s", name, userName,
				groupName, &qtime, &minprocs, &dmemory, &wCLimit, &priority,
				tmpQOS, jobState, jobClass);

		if (jobState[0] == 'X')
			jobState[0] = ' ';

		totalLoad += (double) wCLimit * minprocs;

		xfactor = (double) (startTime - qtime + wCLimit) / wCLimit;

		if (SMP == TRUE)
			sprintf(SMPLine, " %6d", dmemory);

		/* display job */
		fprintf(stdout, "%18s%c%11d %8.1f %2.2s %9s %8s %6d%s%12s %9s %21s",
				name, jobState[0], priority, xfactor, tmpQOS, userName,
				groupName, minprocs, SMPLine, timeToString(wCLimit), jobClass,
				getDateString(&qtime));
	}

	fprintf(stdout,
			"\nJobs: %d  Total Backlog:  %.2f ProcHours  (%.2f Hours)\n", count,
			totalLoad / 3600.0, (double) totalLoad / 3600.0 / totalProcs);

	return 1;
}

/* show blocked queue */
int showBQueue(char *msgBuffer) {

	char *ptr, *buffer, name[MAXNAME], reason[MAXBUFFER], userName[MAXNAME];
    long startTime;
    int totalProcs;

	fprintf(stdout, "%18s %9s %20s\n\n", "JobName", "User", "Reason");

	buffer = (char *) malloc(strlen(msgBuffer) + 1);
	strcpy(buffer, msgBuffer);
	buffer = strstr(buffer, "[BLOCKED]\n") + strlen("[BLOCKED]\n");

    sscanf(buffer, "%ld %d\n", &startTime, &totalProcs);

	strtok(buffer, "\n");

	while ((ptr = strtok(NULL, "\n")) != NULL) {
		if (strstr(ptr, "Truncated")) {
			fprintf(stderr,
					"WARNING:  blocked job list is too long  (list truncated)\n");

			break;
		}

		/* Format:  <JOBNAME> <USERNAME> <DUMMY1> <DUMMY2> <REASON> */
		sscanf(ptr, "%s %s %*s %*s %[^\n]s", name, userName, reason);

		/* display job */
		fprintf(stdout, "%18s %9s %s\n", name, userName, reason);
	}

	return 1;
}

/* show running queue */
int showRQueue(char *msgBuffer, char *parName, int displayFlags) {
	char *ptr, *buffer, partition[MAXNAME];
	char name[MAXNAME], tmpQOS[MAXNAME], effic[MAXNAME];
	long stime, qtime, aWallTime, wCLimit, now, jFlags, priority;
	int procs, count, rc, upProcs, idleProcs, busyProcs, SMP;
	double xfactor, psdedicated, psutilized;
	int upNodes, idleNodes, busyNodes, dMemory, dedicatedMemory,
			configuredMemory;
	char jState[MAXNAME], userName[MAXNAME], groupName[MAXNAME];
	char mHostName[MAXNAME], SMPLine[MAXLINE], jobMarker;

	if (getenv(MSCHED_ENVSMPVAR) != NULL)
		SMP = TRUE;
	else
		SMP = FALSE;

	SMPLine[0] = '\0';

	buffer = (char *) malloc(strlen(msgBuffer) + 1);
	strcpy(buffer, msgBuffer);
	buffer = strstr(buffer, "[RUNNING]\n") + strlen("[RUNNING]\n");

	/* get general state */
	sscanf(buffer, "%ld %d %d %d %d %d %d\n", &now, &upProcs, &idleProcs,
			&upNodes, &idleNodes, &configuredMemory, &busyProcs);

	busyNodes = upNodes - idleNodes;

	busyProcs = MIN(upProcs, busyProcs);

	/* display partition */
	if (strcmp(parName, GLOBAL_MPARNAME) != 0) {
		fprintf(stdout, "partition: %s\n", parName);
	}

	/* display list of active jobs */
	fprintf(stdout,
			"%18s%c %1s %3s %6s %8s %2s %9s %8s %8s %5s %s%11s  %19s\n\n",
			"JobName", ' ', "S", "Par", "Effic", "XFactor", "Q", "User",
			"Group", "MHost", displayFlags ? "Nodes" : "Procs",
			(SMP == TRUE) ? "Memory " : "", "Remaining", "StartTime");

	count = 0;

	ptr = strtok(buffer, "\n");

	dedicatedMemory = 0;

	/*  read all active jobs */
	while ((ptr = strtok(NULL, "\n")) != NULL) {
		if (!strcmp(ptr, "[BLOCKED]"))
			break;

		count++;

		/* Format:  <JOBNAME> <STATE> <USERNAME> <GROUPNAME> <START TIME> <QUEUE
		 * TIME> <PROCS> <CPULIMIT> <QOS> <PSDEDICATED> <PSUTILIZED> <PARTITION>
		 */
		rc = sscanf(ptr,
				"%s %s %s %s %ld %ld %d %ld %s %lf %lf %s %d %s %ld %ld %ld",
				name, jState, userName, groupName, &stime, &qtime, &procs,
				&wCLimit, tmpQOS, &psdedicated, &psutilized, partition,
				&dMemory, mHostName, &jFlags, &aWallTime, &priority);

		if (rc != 17) {
			fprintf(stderr, "ALERT:  job data is corrupt (%d fields: '%s')\n",
					rc, ptr);

			continue;
		}

		dedicatedMemory += dMemory;

		xfactor = (double) ((stime - qtime) + wCLimit) / wCLimit;

		if (psdedicated > 0.0)
			sprintf(effic, "%6.2f", (double) psutilized / psdedicated * 100.0);
		else
			strcpy(effic, "------");

		if (SMP == TRUE)
			sprintf(SMPLine, "%6d ", dMemory);

		/* display job */
		if (jFlags & (1 << SPVIOLATION)) {
			jobMarker = '_';
		} else if (jFlags & (1 << BACKFILL)) {
			if (jFlags & (1 << PREEMPTEE)) {
				jobMarker = '*';
			} else {
				jobMarker = '+';
			}
		} else if (jFlags & (1 << PREEMPTEE)) {
			jobMarker = '-';
		} else {
			jobMarker = ' ';
		}

		fprintf(stdout,
				"%18s%c %c %3.3s %6s %8.1f %2.2s %9s %8s %8s %5d %s%11s  %19s",
				name, jobMarker, jState[0], partition, effic, xfactor, tmpQOS,
				userName, groupName, mHostName, procs, SMPLine,
				timeToString(wCLimit - aWallTime), getDateString(&stime));
	}

	if (displayFlags) {
		fprintf(stdout, "\n%3d Jobs   %5d of %5d Nodes Active (%.2f%c)\n",
				count, busyNodes, upNodes, (double) busyNodes / upNodes * 100.0,
				'%');
	} else {
		fprintf(stdout, "\n%3d Jobs   %5d of %5d Processors Active (%.2f%c)\n",
				count, busyProcs, upProcs,
				(upProcs > 0) ? (double) busyProcs / upProcs * 100.0 : 0.0,
				'%');
	}

	if ((SMP == TRUE) && (configuredMemory > 0)) {
		fprintf(stdout, "           %5d of %5d MB Memory in Use  (%.2f%c)\n",
				dedicatedMemory, configuredMemory,
				(double) dedicatedMemory / configuredMemory * 100.0, '%');
	}

	return 1;
}

/* combine and save information into a buffer */
char *buildMsgBuffer(showq_info_t showq_info) {
	char *buffer;
	int queueMode = 0;

	if ((buffer = (char *) malloc(24)) == NULL) {
		puts("ERROR: cannot allocate memory for buffer");
		return NULL;
	}

	if (showq_info.blocked)
		queueMode |= (1 << BLOCKED);

	if (showq_info.running)
		queueMode |= (1 << RUNNING);

	if (showq_info.idle)
		queueMode |= (1 << IDLE);

	if (showq_info.username == NULL) {
		showq_info.username = strdup("");
	}

	sprintf(buffer, "%d %s %d %s", queueMode, showq_info.pName, 0,
			showq_info.username);

	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 showq_info_t *showq_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {
            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"block",       no_argument,       0, 'b'},
            {"idle",        no_argument,       0, 'i'},
            {"partition",   required_argument, 0, 'p'},
            {"running",     no_argument,       0, 'r'},
            {"username",    required_argument, 0, 'u'},
            {"configfile",  required_argument, 0, 'C'},
            {"loglevel",    required_argument, 0, 'D'},
            {"logfacility", required_argument, 0, 'F'},
            {"host",        required_argument, 0, 'H'},
            {"keyfile",     required_argument, 0, 'k'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVbip:ru:C:D:F:H:k:P:",
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

          case 'b':
              showq_info->blocked = 1;
              break;

          case 'i':
              showq_info->idle = 1;
              break;

          case 'p':
              showq_info->pName = string_dup(optarg);
              break;

          case 'r':
              showq_info->running = 1;
              break;

          case 'u':
              showq_info->username = string_dup(optarg);
              break;

          case 'C':
              printf ("set configfile to %s\n", optarg);
              client_info->configfile = string_dup(optarg);
              break;

          case 'D':
              client_info->loglevel = string2int(optarg);

              if (client_info->loglevel != INVALID_STRING)
                  printf ("set loglevel to %s\n", optarg);
              break;

          case 'F':
              printf ("set logfacility to %s\n", optarg);
              client_info->logfacility = string_dup(optarg);
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
              puts ("Try 'showq --help' for more information.\n");
              return 0;

          default:
              abort();
        }
    }

    return 1;
}

/* free memory */
void free_structs(showq_info_t *showq_info, client_info_t *client_info){
    free(showq_info->username);
    free(showq_info->pName);
    free(client_info->configfile);
    free(client_info->host);
    free(client_info->logfacility);
}

void print_usage()
{
    puts ("\nUsage: showq [FLAGS]\n\n"
          "Display information about all jobs in active, idle and blocked states.\n"
          "\n"
          "  -h, --help                     display this help\n"
          "  -V, --version                  display client version\n"
          "\n"
          "  -b, --blocked                  display blocked jobs only\n"
          "  -i, --idle                     display idle jobs only\n"
          "  -r, --running                  display active jobs only\n"
          "  -p, --partition=PARTITIONID    display jobs assigned to the specified partition\n"
          "  -u, --username=USERNAMEID      display information about jobs for the specified user\n");
    print_client_usage();
}
