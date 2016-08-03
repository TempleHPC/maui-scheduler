/*
 * setres standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include <locale.h>

#include "msched-version.h"
#include "maui_utils.h"

#define MAX_MTIME 2140000000

/** Struct for setres options */
typedef struct _setres_info {
	long  clientTime;		   /**< Client time */
	long  beginTime;		   /**< Begin time */
	long  endTime;		       /**< End time */
	long  duration;            /**< Duration */
    char *pName;               /**< Partition name */
    char *userList;			   /**< User list */
    char *accountList;		   /**< Account list */
    char *groupList;		   /**< Group list */
    char *classList;		   /**< Class list */
    char *QOSList;			   /**< QOS list */
    char *rName;			   /**< Reservation name */
    char *resourceList;		   /**< Resource list */
    char *chargeAccount;	   /**< Charge list */
    char *featureString;	   /**< Feature string */
    char *nodeSetString;       /**< Node set string*/
    char *flags;			   /**< Flags */
    char *jobFeatureString;	   /**< Job feature string */
    int   taskCount;		   /**< Task count */
    char *nodeid;			   /**< Node ID */
} setres_info_t;

static void free_structs(setres_info_t *, client_info_t *);
static int process_args(int, char **, setres_info_t *, client_info_t *);
static void print_usage();
static char *buildMsgBuffer(setres_info_t *, client_info_t *);
static long timeFromString(char *);
static int stringToE(char *, long *);

int main(int argc, char **argv) {

    setres_info_t setres_info;
    client_info_t client_info;

	char *response, *msgBuffer;
	int sd;
	long bufSize;
	char request[MAXBUFFER];

    memset(&setres_info, 0, sizeof(setres_info));
    memset(&client_info, 0, sizeof(client_info));

    setres_info.endTime = MAX_MTIME;

    /* process all the options and arguments */
    if (process_args(argc, argv, &setres_info, &client_info)) {

		get_connection_params(&client_info);

		if (!connectToServer(&sd, client_info.port, client_info.host)){
			free_structs(&setres_info, &client_info);
			exit(EXIT_FAILURE);
		}

		msgBuffer = buildMsgBuffer(&setres_info, &client_info);
		generateBuffer(request, msgBuffer, "setres");
		free(msgBuffer);

		if (!sendPacket(sd, request)){
			free_structs(&setres_info, &client_info);
			exit(EXIT_FAILURE);
		}

		if ((bufSize = getMessageSize(sd)) == 0){
			free_structs(&setres_info, &client_info);
			exit(EXIT_FAILURE);
		}

		if ((response = (char *) calloc(bufSize + 1, 1)) == NULL) {
	        printError(MEMALLO);
			free_structs(&setres_info, &client_info);
			exit(EXIT_FAILURE);
		}

		/* receive message from server */
		if (!recvPacket(sd, &response, bufSize)){
			free_structs(&setres_info, &client_info);
			exit(EXIT_FAILURE);
		}

	    fprintf(stdout, "\nreservation created\n");

		printf("\n%s\n", strstr(response, "ARG=") + strlen("ARG="));

		free(response);

    }

    free_structs(&setres_info, &client_info);

    exit(0);
}

/* combine and save information into a buffer */
char *buildMsgBuffer(setres_info_t *setres_info, client_info_t *client_info) {

	char *buffer;
	int len = 0;

    time_t tmpT;

    time(&tmpT);
    setres_info->clientTime = (long)tmpT;


	if (setres_info->pName == NULL)
		if ((setres_info->pName = getenv(MSCHED_ENVPARVAR)) == NULL)
			setres_info->pName = string_dup(GLOBAL_MPARNAME);

	if (setres_info->beginTime == 0)
		setres_info->beginTime = setres_info->clientTime;

	if (setres_info->duration != 0)
		setres_info->endTime = setres_info->beginTime + setres_info->duration;
	else if (setres_info->endTime == MAX_MTIME)
		setres_info->endTime = setres_info->beginTime + 100000000;

	if (strlen(setres_info->nodeid) >= (MAXLINE << 2)) {
		fprintf(stderr, "ERROR:    regular expression too long. (%d > %d)\n",
				(int) strlen(setres_info->nodeid), (MAXLINE << 2));

		exit(1);
	}

	if (setres_info->userList == NULL)
		setres_info->userList = string_dup(NONE);
	if (setres_info->accountList == NULL)
		setres_info->accountList = string_dup(NONE);
	if (setres_info->groupList == NULL)
		setres_info->groupList = string_dup(NONE);
	if (setres_info->classList == NULL)
		setres_info->classList = string_dup(NONE);
	if (setres_info->QOSList == NULL)
		setres_info->QOSList = string_dup(NONE);
	if (setres_info->rName == NULL)
		setres_info->rName = string_dup(NONE);
	if (setres_info->resourceList == NULL)
		setres_info->resourceList = string_dup(NONE);
	if (setres_info->chargeAccount == NULL)
		setres_info->chargeAccount = string_dup(NONE);
	if (setres_info->featureString == NULL)
		setres_info->featureString = string_dup(NONE);
	if (setres_info->nodeSetString == NULL)
		setres_info->nodeSetString = string_dup(NONE);
	if (setres_info->flags == NULL)
		setres_info->flags = string_dup(NONE);
	if (setres_info->jobFeatureString == NULL)
		setres_info->jobFeatureString = string_dup(NONE);

	/* calculate the length of the whole buffer */

	/* plus one for a white space */
	len += strlen(setres_info->pName) + 1;
	len += strlen(setres_info->userList) + 1;
	len += strlen(setres_info->accountList) + 1;
	len += strlen(setres_info->groupList) + 1;
	len += strlen(setres_info->classList) + 1;
	len += strlen(setres_info->QOSList) + 1;
	len += strlen(setres_info->rName) + 1;
	len += strlen(setres_info->resourceList) + 1;
	len += strlen(setres_info->chargeAccount) + 1;
	len += strlen(setres_info->featureString) + 1;
	len += strlen(setres_info->nodeSetString) + 1;
	len += strlen(setres_info->flags) + 1;
	len += strlen(setres_info->jobFeatureString) + 1;
	len += strlen(setres_info->nodeid) + 1;

	/* reserve extra space for numbers */
	if ((buffer = (char *) malloc(len + 35)) == NULL) {
        printError(MEMALLO);
		free_structs(setres_info, client_info);
		exit(EXIT_FAILURE);
	}

	/* build buffer */
	sprintf(buffer, "%ld %ld %ld %s %s %s %s %s %s %s %s %s %s %s %s %s %d %s",
			setres_info->clientTime, setres_info->beginTime, setres_info->endTime,
			setres_info->pName, setres_info->userList, setres_info->groupList,
			setres_info->accountList, setres_info->classList, setres_info->QOSList,
			setres_info->rName, setres_info->resourceList,
			setres_info->chargeAccount, setres_info->nodeid,
			setres_info->featureString, setres_info->nodeSetString,
			setres_info->flags, setres_info->taskCount,
			setres_info->jobFeatureString);
	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 setres_info_t *setres_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"account",     required_argument, 0, 'a'},
            {"charge",      required_argument, 0, 'A'},
            {"class",       required_argument, 0, 'c'},
            {"duration",  	required_argument, 0, 'd'},
            {"endtime",     required_argument, 0, 'e'},
            {"feature",     required_argument, 0, 'f'},
            {"group",       required_argument, 0, 'g'},
            {"jobfeature",  required_argument, 0, 'j'},
            {"name",        required_argument, 0, 'n'},
            {"nodesetlist", required_argument, 0, 'N'},
            {"partition",   required_argument, 0, 'p'},
            {"qos",         required_argument, 0, 'Q'},
            {"resource",    required_argument, 0, 'r'},
            {"starttime",   required_argument, 0, 's'},
            {"maxtask",     required_argument, 0, 't'},
            {"user",        required_argument, 0, 'u'},
            {"flags",       required_argument, 0, 'x'},
            {"configfile",  required_argument, 0, 'C'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

		c = getopt_long(argc, argv,
				"hVa:A:c:d:e:f:g:j:n:N:p:Q:r:s:t:u:x:C:H:P:", options,
				&option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {

          case 'h':
              print_usage();
              free_structs(setres_info, client_info);
              exit(EXIT_SUCCESS);
              break;

          case 'V':
              printf("Maui version %s\n", MSCHED_VERSION);
              free_structs(setres_info, client_info);
              exit(EXIT_SUCCESS);
              break;

          case 'a':
        	  setres_info->accountList = string_dup(optarg);
              break;

          case 'A':
        	  setres_info->chargeAccount = string_dup(optarg);
              break;

          case 'c':
        	  setres_info->classList = string_dup(optarg);
              break;

          case 'd':
        	  setres_info->duration = timeFromString(optarg);
              break;

          case 'e':
              if (stringToE(optarg, &(setres_info->endTime)) != SUCCESS) {
				print_usage();

				fprintf(stderr, "ERROR:    invalid endtime specified, '%s'\n",
						optarg);

				free_structs(setres_info, client_info);
				exit(EXIT_FAILURE);
              }
              break;

          case 'f':
        	  setres_info->featureString = string_dup(optarg);
              break;

          case 'g':
        	  setres_info->groupList = string_dup(optarg);
              break;

          case 'j':
        	  setres_info->jobFeatureString = string_dup(optarg);
              break;

          case 'n':
        	  setres_info->rName = string_dup(optarg);
              break;

          case 'N':
        	  setres_info->nodeSetString = string_dup(optarg);
              break;

          case 'p':
        	  setres_info->pName = string_dup(optarg);
              break;

          case 'Q':
        	  setres_info->QOSList = string_dup(optarg);
              break;

          case 'r':
        	  setres_info->resourceList = string_dup(optarg);
              break;

          case 's':
              if (stringToE(optarg, &(setres_info->beginTime)) != SUCCESS) {
				print_usage();

				fprintf(stderr, "ERROR:    invalid starttime specified, '%s'\n",
						optarg);

				free_structs(setres_info, client_info);
				exit(EXIT_FAILURE);
              }
              break;

          case 't':
        	  setres_info->taskCount = (int)strtol(optarg, NULL, 0);;
              break;

          case 'u':
        	  setres_info->userList = string_dup(optarg);
              break;

          case 'x':
        	  setres_info->flags = string_dup(optarg);
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
              puts ("Try 'setres --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

    /* only need one argument */
    if(optind != argc - 1){
        print_usage();
        free_structs(setres_info, client_info);
        exit(EXIT_FAILURE);
    }

    /* copy and save node id from input */
    setres_info->nodeid = string_dup(argv[optind]);

    return 1;
}

long timeFromString(char *TString)
{
    long val;

    char *ptr1;
    char *ptr2;
    char *ptr3;
    char *ptr4;

    char Line[MAXLINE];

    if (TString == NULL) return (0);

    if (!strcmp(TString, "INFINITY")) return (MAX_MTIME);

    if (strchr(TString, ':') == NULL) {
        /* line specified as 'raw' seconds */

        val = strtol(TString, NULL, 0);

        return (val);
    } else if (strchr(TString, '_') != NULL) {
        /* line specified as 'absolute' time */

        stringToE(TString, &val);

        return (val);
    }

    /* line specified in 'military' time */

    strcpy(Line, TString);

    ptr1 = NULL;
    ptr2 = NULL;
    ptr3 = NULL;
    ptr4 = NULL;

    if ((ptr1 = strtok(Line, ":")) != NULL) {
        if ((ptr2 = strtok(NULL, ":")) != NULL) {
            if ((ptr3 = strtok(NULL, ":")) != NULL) {
                ptr4 = strtok(NULL, ":");
            }
        }
    }

    if (ptr1 == NULL) {

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

    return (val);
}

int stringToE(char *TimeLine, long *EpochTime)
{
    char Second[MAXNAME];
    char Minute[MAXNAME];
    char Hour[MAXNAME];
    char Day[MAXNAME];
    char Month[MAXNAME];
    char Year[MAXNAME];
    char TZ[MAXNAME];

    char StringTime[MAXNAME];
    char StringDate[MAXNAME];
    char Line[MAXLINE];

    char *ptr;
    char *tail;

    struct tm Time;
    struct tm *DefaultTime;

    time_t ETime; /* calculated epoch time */
    time_t Now;

    int YearVal;

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

        ETime = Now + timeFromString(ptr + 1);
    } else {
        /* using absolute time */

        /* Format:  HH[:MM[:SS]][_MM[/DD[/YY]]] */

        setlocale(LC_TIME, "en_US.iso88591");

        DefaultTime = localtime(&Now);

        /* copy default values into time structure */

        strcpy(Second, "00");
        strcpy(Minute, "00");
        strftime(Hour, MAXNAME, "%H", DefaultTime);

        strftime(Day, MAXNAME, "%d", DefaultTime);
        strftime(Month, MAXNAME, "%m", DefaultTime);

        strftime(Year, MAXNAME, "%Y", DefaultTime);

        strftime(TZ, MAXNAME, "%Z", DefaultTime);

        if ((tail = strchr(TimeLine, '_')) != NULL) {
            /* time and date specified */

            strncpy(StringTime, TimeLine, (tail - TimeLine));
            StringTime[(tail - TimeLine)] = '\0';

            strcpy(StringDate, (tail + 1));

            /* parse date */

            if ((ptr = strtok(StringDate, "/")) != NULL) {
                strcpy(Month, ptr);

                if ((ptr = strtok(NULL, "/")) != NULL) {
                    strcpy(Day, ptr);

                    if ((ptr = strtok(NULL, "/")) != NULL) {
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

        if ((ptr = strtok(StringTime, ":_")) != NULL) {
            strcpy(Hour, ptr);

            if ((ptr = strtok(NULL, ":_")) != NULL) {
                strcpy(Minute, ptr);

                if ((ptr = strtok(NULL, ":_")) != NULL)
                    strcpy(Second, ptr);
            }
        }

        /* create time string */

        sprintf(Line, "%s:%s:%s %s/%s/%s %s", Hour, Minute, Second, Month, Day,
                Year, TZ);

        /* perform bounds checking */

        if ((atoi(Second) > 59) || (atoi(Minute) > 59) || (atoi(Hour) > 23) ||
            (atoi(Month) > 12) || (atoi(Day) > 31) || (atoi(Year) > 2097)) {

            printf("ERROR: invalid time specified '%s' (bounds exceeded)\n",
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

        if ((ETime = mktime(&Time)) == -1) {
            printf(
                "ERROR: cannot determine epoch time for '%s', errno: %d "
                "(%s)\n",
                Line, errno, strerror(errno));

            return (FAILURE);
        }
    } /* END else (strchr(TimeLine,'+')) */

    *EpochTime = (long)ETime;

    return (SUCCESS);
}

/* free memory */
void free_structs(setres_info_t *setres_info, client_info_t *client_info) {
    free(setres_info->pName);
    free(setres_info->userList);
    free(setres_info->accountList);
    free(setres_info->groupList);
    free(setres_info->classList);
    free(setres_info->QOSList);
    free(setres_info->rName);
    free(setres_info->resourceList);
    free(setres_info->chargeAccount);
    free(setres_info->featureString);
    free(setres_info->nodeSetString);
    free(setres_info->flags);
    free(setres_info->jobFeatureString);
    free(setres_info->nodeid);
    free(client_info->configfile);
    free(client_info->host);
}

void print_usage()
{
    puts ("\nUsage: setres [FLAGS] <NODEID>\n\n"
            "Create reservations on a specified node. Time format:\n"
            "[HH[:MM[:SS]]][_MO[/DD[/YY]]] i.e. 14:30_06/20 or\n"
            "+[[[DD:]HH:]MM:]SS(relative time).\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n"
    		"\n"
    		"  -a, --account=ACCOUNTID[:ACCOUNTID]...\n"
    		"                                 set account list\n"
    		"  -A, --charge=ACCOUNTID[,GROUPID[,USERID]]\n"
    		"                                 charge creds\n"
    		"  -c, --class=CLASSID[:CLASSID]...\n"
    		"                                 set class list\n"
    		"  -d, --duration=VALUE           set duration\n"
    		"  -e, --endtime=VALUE            set endtime\n"
    		"  -f, --feature=FEATUREID[:FEATUREID]...\n"
    		"                                 set feature list\n"
    		"  -g, --group=GROUPID[:GROUPID]...\n"
    		"                                 set group list\n"
    		"  -j, --jobfeature=JOBFEATUREID[:JOBFEATUREID]...\n"
    		"                                 set jobfeature list\n"
    		"  -n, --name=VALUE               set reservation name\n"
    		"  -N, --nodesetlist=SETSELECTION:SETTYPE[:SETLIST]...\n"
    		"                                 set node set list\n"
    		"  -p, --partition=VALUE          set partition\n"
    		"  -Q, --qos=QOSID[:QOSID]...     set QOS list\n"
    		"  -r, --resource RESOURCETYPE=VALUE,[RESOURCETYPE=VALUE]...\n"
    		"                                 set resource list\n"
    		"  -s, --starttime=VALUE          set start time\n"
    		"  -t, --maxtask=VALUE            set max tasks\n"
    		"  -u, --user=USERID[:USERID]...  set user list\n"
    		"  -x, --flags=FLAGID[,FLAGID]... set flags\n");
    print_client_usage();
}
