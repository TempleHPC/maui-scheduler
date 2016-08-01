/*
 * showstate standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

#define MAX_MFRAMECOUNT 64
#define MAX_MSLOTPERFRAME 32
#define MAXJOB 4096
#define MAX_MFRAME 48
#define MAX_MNETTYPE 4
#define MAX_MJOBINDEX 62

enum { mnetNONE = 0, mnetEN0, mnetEN1, mnetCSS0 };

enum {
    probLocalDisk = 0,
    probMemory,
    probEthernet,
    probHPS_IP,
    probHPS_User,
    probSwap
};

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

/** Struct for showstate options */
typedef struct _showstate_info {
    int xml;               /**< XML flag*/
} showstate_info_t;

/** Struct for host */
typedef struct {
    time_t MTime;

    char RMName[MAXNAME + 1];
    char NetName[MAX_MNETTYPE][MAXNAME + 1];
    unsigned long NetAddr[MAX_MNETTYPE];

    unsigned long Attributes;
    int SlotsUsed;
    int State;
    int Arch;
    int Disk;
    int Classes;
    int Failures;
} mhost_t;

typedef mhost_t msys_t[MAX_MFRAME][MAX_MSLOTPERFRAME + 1];

static void free_structs(client_info_t *);
static int process_args(int, char **, showstate_info_t *, client_info_t *);
static void print_usage();
static int clusterShow(char *, int);
static char *localStrTok(char *, char *, char **);

int main(int argc, char **argv) {

    showstate_info_t showstate_info;
    client_info_t client_info;

    char *response, *ptr;
    int sd;
    long bufSize;
    char request[MAXBUFFER];

    memset(&showstate_info, 0, sizeof(showstate_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &showstate_info, &client_info)) {

		get_connection_params(&client_info);

		if (!connectToServer(&sd, client_info.port, client_info.host)){
			free_structs(&client_info);
			exit(EXIT_FAILURE);
		}

		generateBuffer(request, showstate_info.xml ? "XML" : "DEFAULT",
				"showstate");

		if (!sendPacket(sd, request)){
			free_structs(&client_info);
			exit(EXIT_FAILURE);
		}

		if ((bufSize = getMessageSize(sd)) == 0){
			free_structs(&client_info);
			exit(EXIT_FAILURE);
		}

		if ((response = (char *) calloc(bufSize + 1, 1)) == NULL) {
			puts("Error: cannot allocate memory for message");
			free_structs(&client_info);
			exit(EXIT_FAILURE);
		}

		/* receive message from server */
		if (!recvPacket(sd, &response, bufSize)){
			free_structs(&client_info);
			exit(EXIT_FAILURE);
		}

		/* print result */
		ptr = strstr(response, "ARG=") + strlen("ARG=");

		clusterShow(ptr, showstate_info.xml);

		free(response);

    }

    free_structs(&client_info);

    exit(0);
}

/*
 * extract data from buffer and print out result
 */
int clusterShow(char *buffer, int xml)
{
    char *ptr = NULL;
    char *oPtr = NULL;

    int index;

    msys_t system;

    unsigned char mapj[MAX_MFRAMECOUNT][MAX_MSLOTPERFRAME + 1];

    char tmpLine[MAXLINE];
    char tmpName[MAXNAME];
    char tmpState[MAXNAME];
    char temp_str[MAXLINE];

    char tmpNLString[MAXBUFFER];

    int sChar = 0;

	char jName[MAXJOB][MAXNAME];
	char jUName[MAXJOB][MAXNAME];
	char jGName[MAXJOB][MAXNAME];
	char jState[MAXJOB][MAXNAME];
	long jStartTime[MAXJOB];
	int jNCount[MAXJOB];
	int jPCount[MAXJOB];
	long jDuration[MAXJOB];

	int jindex = 0;
	int findex = 0;
	int sindex = 0;

	char warnings[MAXBUFFER];
	int totalNodes = 0;

	long now = 0;
	int maxSlotPerFrame = 0;
	int maxFrame = 0;

	int slotsUsed = 0;

	char *fPtr = NULL;
	char *sPtr = NULL;
	char *tokPtr = NULL;

	const char *MNodeState[] =
			{ "NONE", "None", "Down", "Idle", "Busy", "Running", "Drained",
					"Drain", "Flush", "Reserved", "Unknown", NULL };

	char JID[] =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz";

    mhost_t *S = NULL;

    warnings[0] = '\0';

    /* FORMAT:                                             */
    /*                                                     */

    if (xml) {
        fprintf(stdout, "%s", buffer);
        exit(0);
    }

    memset(mapj, 255, sizeof(mapj));

    jindex = 0;

    maxFrame = -1;

    ptr = strtok(buffer, "\n");

	while (ptr != NULL) {

		oPtr = ptr;

		ptr = strtok(NULL, "\n");

		/* FORMAT:  <OBJ> [DATA]... */
		if (!strncmp(oPtr, "sys", strlen("sys"))) {

			oPtr += strlen("sys") + 1;

			sscanf(oPtr, "%ld %d", &now, &maxSlotPerFrame);

			if (maxSlotPerFrame <= 0) {
				fprintf(stderr, "ERROR:  invalid frame configuration\n");

				exit(1);
			}
		}

		if (!strncmp(oPtr, "frame", strlen("frame"))) {

			oPtr += strlen("frame") + 1;

			sscanf(oPtr, "%64s %d %1024s", tmpName, &findex, tmpLine);

			findex = MIN(MAX_MFRAME, MAX(0, findex));

			maxFrame = MAX(findex, maxFrame);

			S = &system[findex][0];

			strcpy(S->NetName[mnetEN0], tmpName);

		}

		if (!strncmp(oPtr, "node", strlen("node"))) {

			oPtr += strlen("node") + 1;

			sscanf(oPtr, "%64s %64s %d %d %d %s", tmpName, tmpState, &findex,
					&sindex, &slotsUsed, tmpLine);

			findex = MIN(MAX_MFRAME, MAX(0, findex));
			sindex = MIN(MAX_MSLOTPERFRAME, MAX(1, sindex));

			S = &system[findex][sindex];

			strcpy(S->NetName[mnetEN0], tmpName);
			S->SlotsUsed = slotsUsed;

			for (index = 0; MNodeState[index] != NULL; index++) {
				if (!strcmp(tmpState, MNodeState[index])) {
					break;
				}
			}

			S->State = index;

			/* NOTE:  temporary:  failure is now string */

			S->Failures = strtol(tmpLine, NULL, 0);

			system[findex][0].SlotsUsed = 0;

			system[findex][0].SlotsUsed = MAX(system[findex][0].SlotsUsed,
					sindex + slotsUsed - 1);

			maxSlotPerFrame = MAX(maxSlotPerFrame, system[findex][0].SlotsUsed);

		}

		if (!strncmp(oPtr, "job", strlen("job"))) {

			oPtr += strlen("job") + 1;

			sscanf(oPtr, "%s %d %d %ld %s %s %ld %s %s", jName[jindex],
					&jPCount[jindex], &jNCount[jindex], &jDuration[jindex],
					jUName[jindex], jGName[jindex], &jStartTime[jindex], /* if job is idle and start
					 time set, value is reserve
					 time */
					jState[jindex], tmpNLString);

			fPtr = localStrTok(tmpNLString, " \t\n;:", &tokPtr);

			while (fPtr != NULL) {
				int fVal;
				int sVal;

				fVal = (int) strtol(fPtr, NULL, 0);

				if ((sPtr = localStrTok(NULL, " \t\n;:", &tokPtr)) != NULL)
					sVal = (int) strtol(sPtr, NULL, 0);
				else
					sVal = 0;

				mapj[fVal][sVal] = jindex;

				fPtr = localStrTok(NULL, " \t\n;:", &tokPtr);
			} /* END while (FPtr != NULL) */

			jindex++;

		}
	}

    /* display state */

    fprintf(stdout, "cluster state summary for %s\n\n", getDateString(&now));

    fprintf(stdout, "    %-18s %1s %9s %8s %5s %11s  %19s\n", "JobName", "S",
            "User", "Group", "Procs", "Remaining", "StartTime");

    fprintf(stdout, "    %18.18s %1.1s %9.9s %8.8s %5.5s %11.11s  %19.19s\n\n",
            "------------------------------", "------------------------------",
            "------------------------------", "------------------------------",
            "------------------------------", "------------------------------",
            "------------------------------");

    totalNodes = 0;

    for (index = 0; index < jindex; index++) {
        fprintf(stdout, "(%c) %-18s %1c %9s %8s %5d %11s  %19s",
                JID[index % MAX_MJOBINDEX], jName[index], jState[index][0],
                jUName[index], jGName[index], jPCount[index],
                timeToString(jStartTime[index] + jDuration[index] - now),
				getDateString(&jStartTime[index]));

        totalNodes += jNCount[index];
    } /* END for (index) */

    fprintf(stdout, "\nusage summary:  %d active jobs  %d active nodes\n\n",
            jindex, totalNodes);

    fprintf(stdout, "               %.*s\n", 3 * maxSlotPerFrame,
            "[0][0][0][0][0][0][0][0][0][1][1][1][1][1][1][1][1][1][1][2][2][2]"
            "[2][2][2][2][2][2][2][2][3][3]");

    fprintf(stdout, "               %.*s\n\n", 3 * maxSlotPerFrame,
            "[1][2][3][4][5][6][7][8][9][0][1][2][3][4][5][6][7][8][9][0][1][2]"
            "[3][4][5][6][7][8][9][0][1][2]");

    for (findex = 1; findex < MAX_MFRAME; findex++) {
        int LastSlotUsed;

        if (findex > maxFrame) break;

        S = &system[findex][0];

        if (S->SlotsUsed == 0) continue;

        fprintf(stdout, "Frame %7.7s: ", S->NetName[mnetEN0]);

        LastSlotUsed = 0;

        for (sindex = 1; sindex <= maxSlotPerFrame; sindex++) {
            if (LastSlotUsed >= sindex) continue;

            S = &system[findex][sindex];

            /* add generic warnings */

            /* report non-homogenous memory sizes */

            if (S->Failures & (1 << probMemory)) {
                sprintf(temp_str, "check memory on node %s\n",
                        S->NetName[mnetEN0]);
                strcat(warnings, temp_str);
            }

            if (S->Failures & (1 << probLocalDisk)) {
                sprintf(temp_str, "%scheck local disk on node %s\n", warnings,
                        S->NetName[mnetEN0]);
                strcat(warnings, temp_str);
            }

            if ((S->State == mnsIdle) || (S->State == mnsActive) ||
                (S->State == mnsBusy)) {
                /* report missing ethernet adapter */

                if (S->Failures & (1 << probEthernet)) {
                    sprintf(temp_str, "check ethernet on node %s\n",
                            S->NetName[mnetEN0]);
                    strcat(warnings, temp_str);
                }

                /* report missing HPS_IP adapter */

                if (S->Failures & (1 << probHPS_IP)) {
                    sprintf(temp_str, "check HPS_IP on node %s\n",
                            S->NetName[mnetEN0]);
                    strcat(warnings, temp_str);
                }
            }

            /* report missing HPS_USER adapter */

            if ((S->Failures & (1 << probHPS_User)) && (S->State == mnsIdle)) {
                sprintf(temp_str, "check HPS_USER on node %s (node is idle)\n",
                        S->NetName[mnetEN0]);
                strcat(warnings, temp_str);
            }

            /* if node is associated with a job */

            if (mapj[findex][sindex] != 255) {
                switch (S->State) {
                    case mnsBusy:
                    case mnsActive:
                    case mnsDraining:

                        sChar = JID[mapj[findex][sindex] % MAX_MJOBINDEX];

                        break;

                    case mnsDown:

                        sChar = '*';

                        sprintf(temp_str,
                                "job %s requires node %s (node is down)\n",
                                jName[mapj[findex][sindex]],
                                S->NetName[mnetEN0]);
                        strcat(warnings, temp_str);

                        break;

                    case mnsIdle:
                    case mnsDrained:
                    case mnsFlush:

                        sChar = JID[mapj[findex][sindex] % MAX_MJOBINDEX];

                        sprintf(temp_str,
                                "job %s has allocated node %s (node is in "
                                "state %s)\n",
                                jName[mapj[findex][sindex]],
                                S->NetName[mnetEN0],
                                MNodeState[system[findex][sindex].State]);
                        strcat(warnings, temp_str);

                        break;

                    case mnsUnknown:

                        sChar = '?';

                        break;

                    default:

                        sChar = JID[mapj[findex][sindex] % MAX_MJOBINDEX];

                        break;
                }
            } /* END if (mapj[findex][sindex] != 255) */
            else {
                switch (system[findex][sindex].State) {
                    case mnsDown:

                        sChar = '#';

                        sprintf(temp_str, "node %s is down\n",
                                S->NetName[mnetEN0]);
                        strcat(warnings, temp_str);

                        break;

                    case mnsReserved:
                    case mnsDraining:
                    case mnsDrained:
                    case mnsFlush:

                        sChar = '!';

                        break;

                    case mnsIdle:

                        if (S->Failures & probLocalDisk)
                            sprintf(
                                temp_str,
                                "node %s is idle and local disk space is low\n",
                                S->NetName[mnetEN0]);
                        strcat(warnings, temp_str);

                        sChar = ' ';

                        break;

                    case mnsUnknown:

                        sChar = '?';

                        break;

                    case mnsBusy:

                        sChar = '@';

                        sprintf(temp_str,
                                "node %s has no job scheduled (node is busy)\n",
                                S->NetName[mnetEN0]);
                        strcat(warnings, temp_str);

                        break;

                    case mnsNone:

                        sChar = '~';

                        break;

                    default:

                        sChar = '@';

                        if (S->SlotsUsed > 0) {
                            sprintf(
                                temp_str,
                                "node[%d,%d] %s has unexpected state '%s'\n",
                                findex, sindex, S->NetName[mnetEN0],
                                MNodeState[S->State]);
                            strcat(warnings, temp_str);
                        }

                        break;
                }
            }
            /* determine next populated slot */

            switch (S->SlotsUsed) {
                case 0:

                    /* node not configured */

                    fprintf(stdout, "XXX");

                    break;

                case 1:

                    if (sChar != '~') {
                        fprintf(stdout, "[%c]", sChar);
                    } else {
                        fprintf(stdout, "XXX");
                    }

                    break;

                case 2:

                    if (sChar != '~') {
                        fprintf(stdout, "[ %c  ]", sChar);
                    } else {
                        fprintf(stdout, "XXXXXX");
                    }

                    break;

                case 3:

                    if (sChar != '~') {
                        fprintf(stdout, "[   %c   ]", sChar);
                    } else {
                        fprintf(stdout, "XXXXXXXXX");
                    }

                    break;

                case 4:

                    if (sChar != '~') {
                        fprintf(stdout, "[    %c     ]", sChar);
                    } else {
                        fprintf(stdout, "XXXXXXXXXXXX");
                    }

                    break;

                default:

                    fprintf(stderr,
                            "ERROR:    unexpected slot count (%d) for "
                            "node[%d][%d] '%s'\n",
                            S->SlotsUsed, findex, sindex, S->NetName[mnetEN0]);

                    break;
            }

            LastSlotUsed += MAX(1, S->SlotsUsed);
        }

        fprintf(stdout, "\n");
    }

    fprintf(stdout,
            "\nKey:  [?]:Unknown [*]:Down w/Job [#]:Down [ ]:Idle [@] Busy "
            "w/No Job [!] Drained\n");

    if (warnings[0] != '\0') {
        fprintf(stdout, "\n%s\n", warnings);
    }

    return (1);
}

/*
 * local strtok function to avoid nested strtok being used
 */

char *localStrTok(char *line, char *dList, char **ptr)

{
    char *head = NULL;

    int dindex;

    unsigned char ignchar;

    if (line != NULL) {
        *ptr = line;
    } else if ((ptr != NULL) && (*ptr == NULL)) {
        return 0;
    }

    ignchar = FALSE;

    while (**ptr != '\0') {
        for (dindex = 0; dList[dindex] != '\0'; dindex++) {
            if (**ptr == dList[dindex]) {
                **ptr = '\0';

                (*ptr)++;

                if (head != NULL) {
                    return (head);
                } else {
                    ignchar = TRUE;

                    break;
                }
            }
        }

        if ((ignchar != TRUE) && (**ptr != '\0')) {
            if (head == NULL) head = *ptr;

            (*ptr)++;
        }

        ignchar = FALSE;
    }

    return (head);
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 showstate_info_t *showstate_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"xml",         no_argument,       0, 'x'},
            {"configfile",  required_argument, 0, 'C'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVxC:H:P:",
                         options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {

          case 'h':
              print_usage();
              free_structs(client_info);
              exit(EXIT_SUCCESS);
              break;

          case 'V':
              printf("Maui version %s\n", MSCHED_VERSION);
              free_structs(client_info);
              exit(EXIT_SUCCESS);
              break;

          case 'x':
              showstate_info->xml = TRUE;
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
              puts ("Try 'showstate --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

    /* no arguments accepted */
    if(optind < argc){
        print_usage();
        free_structs(client_info);
        exit(EXIT_FAILURE);
    }

    return 1;
}

/* free memory */
void free_structs(client_info_t *client_info) {
    free(client_info->configfile);
    free(client_info->host);
}

void print_usage()
{
    puts ("\nUsage: showstate [FLAGS]\n\n"
            "Provides a summary of the state of the system.\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n"
            "\n"
            "  -x, --xml                      use xml display mode\n");
    print_client_usage();
}
