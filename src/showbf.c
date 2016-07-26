/*
 * showbf standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include <locale.h>
#include <grp.h>

#include "msched-version.h"
#include "maui_utils.h"

#define MAX_MTIME 2140000000
#define VERBOSE 4
#define ALL "[ALL]"

/** Struct for showbf options */
typedef struct _showbf_info {
    int   nodeCount;		   /**< Node count */
    int   procCount;		   /**< Proc count */
    int   dMemory;		       /**< Dedicated memory */
    int   memory;		       /**< Memory */
    int   mIndex;			   /**< Memory index */
    int   mode;		           /**< Mode */
    int   flags;		   	   /**< Flags */
	long  duration;            /**< Duration */
    char *pName;               /**< Partition name */
    char *user;			       /**< User */
    char *account;		       /**< Account*/
    char *group;		       /**< Group */
    char *class;		       /**< Class */
    char *QOS;			       /**< QOS name */
    char *featureString;	   /**< Feature string */
} showbf_info_t;

static void free_structs(showbf_info_t *, client_info_t *);
static int process_args(int, char **, showbf_info_t *, client_info_t *);
static void print_usage();
static char *buildMsgBuffer(showbf_info_t );
static long timeFromString(char *);
static int stringToE(char *, long *);
static char *GIDToName(gid_t GID);
static char *UIDToName(uid_t UID);

int main(int argc, char **argv) {

    showbf_info_t showbf_info;
    client_info_t client_info;

	char *response, *msgBuffer, *ptr;
	int sd;
	long bufSize;
	char request[MAXBUFFER];

    memset(&showbf_info, 0, sizeof(showbf_info));
    memset(&client_info, 0, sizeof(client_info));

    showbf_info.account = string_dup("ALL");
    showbf_info.user = string_dup(UIDToName(getuid()));
    showbf_info.group = string_dup(GIDToName(getgid()));

    /* process all the options and arguments */
    if (process_args(argc, argv, &showbf_info, &client_info)) {

    	if (showbf_info.pName == NULL) {
    		if ((ptr = getenv(MSCHED_ENVPARVAR)) != NULL) {
    			showbf_info.pName = string_dup(ptr);
    		} else {
    			showbf_info.pName = string_dup(GLOBAL_MPARNAME);
    		}
    	}

		get_connection_params(&client_info);

		if (!connectToServer(&sd, client_info.port, client_info.host))
			exit(EXIT_FAILURE);

		msgBuffer = buildMsgBuffer(showbf_info);
		generateBuffer(request, msgBuffer, "showbf");
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

	    fprintf(stdout, "\nreservation created\n");

		printf("\n%s\n", strstr(response, "ARG=") + strlen("ARG="));

		free(response);

    }

    free_structs(&showbf_info, &client_info);

    exit(0);
}

char *UIDToName(uid_t UID)
{
    struct passwd *bufptr;
    static char Line[MAXNAME];

    if (UID == ~0U) {
        strcpy(Line, NONE);

        return (Line);
    }

    if ((bufptr = getpwuid(UID)) == NULL) {
        sprintf(Line, "UID%d", UID);
    } else {
        strcpy(Line, bufptr->pw_name);
    }

    return (Line);
}

char *GIDToName(gid_t GID)
{
    struct group *bufptr;

    static char Line[MAXNAME];

    if (GID == ~0U) {
        strcpy(Line, NONE);

        return (Line);
    }

    if ((bufptr = getgrgid(GID)) == NULL) {
        sprintf(Line, "GID%d", GID);
    } else {
        strcpy(Line, bufptr->gr_name);
    }

    return (Line);
}

/* combine and save information into a buffer */
char *buildMsgBuffer(showbf_info_t showbf_info) {

	char *buffer;
	int len = 0;

	const char *MComp[] = {"NC", "<",  "<=", "==", ">=", ">", "<>",
	                       "=",  "!=", "%<", "%!", "%=", NULL};

	if (showbf_info.class == NULL)
		showbf_info.class = string_dup(NONE);
	if (showbf_info.QOS == NULL)
		showbf_info.QOS = string_dup(NONE);
	if (showbf_info.featureString == NULL)
		showbf_info.featureString = string_dup(NONE);

	/* calculate the length of the whole buffer */

	/* plus one for a white space */
	len += strlen(showbf_info.pName) + 1;
	len += strlen(showbf_info.user) + 1;
	len += strlen(showbf_info.account) + 1;
	len += strlen(showbf_info.group) + 1;
	len += strlen(showbf_info.class) + 1;
	len += strlen(showbf_info.QOS) + 1;
	len += strlen(showbf_info.featureString) + 1;

	/* reserve space for numbers */
	if ((buffer = (char *) malloc(len + 30)) == NULL) {
		puts("ERROR: cannot allocate memory for buffer");
		return NULL;
	}

	/* build buffer */
	sprintf(buffer, "%s %s %s %s %ld %d %d %d %d %s %d %d %s %s %s",
			showbf_info.user, showbf_info.group, showbf_info.account,
			showbf_info.pName, showbf_info.duration, showbf_info.nodeCount,
			showbf_info.procCount, showbf_info.dMemory, showbf_info.memory,
			MComp[showbf_info.mIndex], showbf_info.mode, showbf_info.flags,
			showbf_info.class, showbf_info.featureString, showbf_info.QOS);
	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 showbf_info_t *showbf_info,
                 client_info_t *client_info)
{
    int c;
    int index;

    const char *MComp[] = {"NC", "<",  "<=", "==", ">=", ">", "<>",
                           "=",  "!=", "%<", "%!", "%=", NULL};

    enum MCompEnum {
        mcmpNONE = 0,
        mcmpLT,
        mcmpLE,
        mcmpEQ,
        mcmpGE,
        mcmpGT,
        mcmpNE,
        mcmpEQ2,
        mcmpNE2,
        mcmpSSUB,
        mcmpSNE,
        mcmpSEQ
    };

    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"account",     required_argument, 0, 'a'},
            {"all",         required_argument, 0, 'A'},
            {"class",       required_argument, 0, 'c'},
            {"duration",  	required_argument, 0, 'd'},
            {"feature",     required_argument, 0, 'f'},
            {"group",       required_argument, 0, 'g'},
            {"jobfeature",  required_argument, 0, 'j'},
            {"memory",      required_argument, 0, 'm'},
            {"dmemory",     required_argument, 0, 'M'},
            {"nodecount",   required_argument, 0, 'n'},
            {"partition",   required_argument, 0, 'p'},
            {"qos",         required_argument, 0, 'Q'},
            {"proccount",   required_argument, 0, 'r'},
            {"smp",         no_argument,       0, 's'},
            {"user",        required_argument, 0, 'u'},
            {"verbose",     no_argument,       0, 'v'},
            {"configfile",  required_argument, 0, 'C'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

		c = getopt_long(argc, argv,
				"hVa:A:c:d:f:g:j:m:M:n:p:Q:r:su:vC:H:P:", options,
				&option_index);

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
        	  showbf_info->account = string_dup(optarg);
              break;

          case 'A':
        	  showbf_info->account = string_dup(ALL);
        	  showbf_info->group = string_dup(ALL);
        	  showbf_info->user = string_dup(ALL);
              break;

          case 'c':
        	  if (!strcmp(optarg, "ALL"))
        		  showbf_info->class = string_dup(ALL);
        	  else
        		  showbf_info->class = string_dup(optarg);
              break;

          case 'd':
        	  showbf_info->duration = timeFromString(optarg);
              break;

          case 'f':
        	  showbf_info->featureString = string_dup(optarg);
              break;

          case 'g':
        	  showbf_info->group = string_dup(optarg);
              break;

          case 'm':
              if (isdigit(optarg[0])) {
                  /* if no comparison given */

            	  showbf_info->memory = (int)strtol(optarg, NULL, 0);
            	  showbf_info->mIndex = mcmpGE;
              } else {
                  char tmpLine[MAXLINE];

                  /* if comparison given */

                  for (index = 0; ispunct(optarg[index]); index++)
                      ;

                  strncpy(tmpLine, optarg, index);
                  tmpLine[index] = '\0';

                  for (showbf_info->mIndex = 0; MComp[showbf_info->mIndex] != NULL; showbf_info->mIndex++) {
                      if (!strcmp(tmpLine, MComp[showbf_info->mIndex])) break;
                  }

                  if (MComp[showbf_info->mIndex] == NULL) showbf_info->mIndex = 0;

                  sscanf((optarg + index), "%d", &(showbf_info->memory));
              }
              break;

          case 'M':
        	  showbf_info->dMemory = atoi(optarg);
              break;

          case 'n':
        	  showbf_info->nodeCount = atoi(optarg);
              break;

          case 'p':
        	  showbf_info->pName = string_dup(optarg);
              break;

          case 'Q':
        	  showbf_info->QOS = string_dup(optarg);
              break;

          case 'r':
        	  showbf_info->procCount = atoi(optarg);
              break;

          case 's':
        	  showbf_info->mode = 1;
              break;

          case 'v':
        	  showbf_info->flags |= (1 << VERBOSE);
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
              puts ("Try 'showbf --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

    /* no argument accepted */
    if(optind < argc){
        print_usage();
        exit(EXIT_FAILURE);
    }

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
void free_structs(showbf_info_t *showbf_info, client_info_t *client_info) {
    free(showbf_info->pName);
    free(showbf_info->user);
    free(showbf_info->account);
    free(showbf_info->group);
    free(showbf_info->class);
    free(showbf_info->QOS);
    free(showbf_info->featureString);
    free(client_info->configfile);
    free(client_info->host);
}

void print_usage()
{
    puts ("\nUsage: showbf [FLAGS]\n\n"
            "Show what resources are available for immediate use. \n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n"
    		"\n"
    		"  -a, --account=ACCOUNTID        show resource availability information only for the\n"
    		"                                   specified account\n"
    		"  -A, --all                      show resource availability information for all users,\n"
    		"                                   groups, and accounts\n"
    		"  -c, --class=CLASSID            set class\n"
    		"  -d, --duration=VALUE           show resource availability information for the\n"
    		"                                   specified duration in the format [[[DD:]HH:]MM:]SS\n"
    		"  -f, --feature=FEATUREID[:FEATUREID]...\n"
    		"                                 set feature list\n"
    		"  -g, --group=GROUPID            show resource availability information only for the\n"
    		"                                   specified group.\n"
    		"  -j, --jobfeature=JOBFEATUREID[:JOBFEATUREID]...\n"
    		"                                 set jobfeature list\n"
    		"  -m, --memory=VALUE             Allows user to specify the memory requirements for\n"
    		"                                   the backfill nodes of interest\n"
    		"  -M, --dmemory=VALUE            set dedicated memory\n"
    		"  -n, --nodecount=VALUE          show resource availability information for the\n"
    		"                                   specified number of nodes\n"
    		"  -p, --partition=PARTITIONID    show resource availability information only for the\n"
    		"                                   specified partition\n"
    		"  -Q, --qos=QOSID                show resource availability information only for the\n"
    		"                                   specified QOS\n"
    		"  -r, --proccount=VALUE          show resource availability information only for the\n"
    		"                                   specified number of procs\n"
    		"  -s, --smp                      show SMP\n"
    		"  -u, --user=USERID              show resource availability information only for the\n"
    		"                                   specified user\n"
    		"  -v, --verbose                  display additional information\n");
    print_client_usage();
}
