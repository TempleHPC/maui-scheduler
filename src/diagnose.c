/*
 * diagnose standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

#define ACCT 1
#define CLASS 3
#define FRAME 6
#define FS 7
#define GROUP 8
#define JOB 9
#define NODE 11
#define PAR 12
#define PRIORITY 13
#define QOS 14
#define QUEUE 15
#define RSV 18
#define RM 19
#define SYS 24
#define USER 25

#define OFF 1
#define SOFT 3
#define HARD 4

#define VERBOSE 4

/** Struct for diagnose options */
typedef struct _diagnose_info {
    int   mode;                /**< Mode */
    int   pType;			   /**< Policy type */
    char *pName;			   /**< Partition name */
    int   flags;			   /**< Flags */
    char *argument;			   /**< Argument */
} diagnose_info_t;

static void free_structs(diagnose_info_t *, client_info_t *);
static int process_args(int, char **, diagnose_info_t *, client_info_t *);
static void print_usage();
static char *buildMsgBuffer(diagnose_info_t );

int main(int argc, char **argv) {

    diagnose_info_t diagnose_info;
    client_info_t client_info;

	char *response, *ptr, *msgBuffer;
	int sd;
	long bufSize;
	char request[MAXBUFFER];

    memset(&diagnose_info, 0, sizeof(diagnose_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &diagnose_info, &client_info)) {

    	if (diagnose_info.pName == NULL) {
    		if ((ptr = getenv(MSCHED_ENVPARVAR)) != NULL) {
    			diagnose_info.pName = string_dup(ptr);
    		} else {
    			diagnose_info.pName = string_dup(GLOBAL_MPARNAME);
    		}
    	}

		get_connection_params(&client_info);

		if (!connectToServer(&sd, client_info.port, client_info.host))
			exit(EXIT_FAILURE);

		msgBuffer = buildMsgBuffer(diagnose_info);
		generateBuffer(request, msgBuffer, "diagnose");
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

		printf("\n%s\n", strstr(response, "ARG=") + strlen("ARG="));

		free(response);

    }

    free_structs(&diagnose_info, &client_info);

    exit(0);
}

/* combine and save information into a buffer */
char *buildMsgBuffer(diagnose_info_t diagnose_info) {
	char *buffer;
	int len = 0;

	if(diagnose_info.argument == NULL)
		diagnose_info.argument = string_dup(NONE);

	/* calculate the length of the whole buffer */
	len += strlen(diagnose_info.pName) + 1;
	len += strlen(diagnose_info.argument) + 1;

	if ((buffer = (char *) malloc(len + 6)) == NULL) {
		puts("ERROR: cannot allocate memory for buffer");
		return NULL;
	}

    if (diagnose_info.mode == 0) {
        print_usage();
        exit(EXIT_FAILURE);
    }

	if (diagnose_info.mode == QUEUE) {
		if (diagnose_info.pType == 0)
			diagnose_info.pType = SOFT;

		sprintf(buffer, "%d %d %s", diagnose_info.mode, diagnose_info.pType,
				diagnose_info.pName);
	} else {
		sprintf(buffer, "%d %d %s %s", diagnose_info.mode, diagnose_info.flags,
				diagnose_info.pName, diagnose_info.argument);
	}

	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 diagnose_info_t *diagnose_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"account",     no_argument,       0, 'A'},
            {"class",       no_argument,       0, 'c'},
            {"fairshare",   no_argument,       0, 'f'},
            {"group",       no_argument,       0, 'g'},
            {"job",         no_argument,       0, 'j'},
            {"frame",       no_argument,       0, 'm'},
            {"node",        no_argument,       0, 'n'},
            {"priority",    no_argument,       0, 'p'},
            {"policylevel", required_argument, 0, 'l'},
            {"queue",       no_argument,       0, 'q'},
            {"qos",         no_argument,       0, 'Q'},
            {"reservation", no_argument,       0, 'r'},
            {"rm",          no_argument,       0, 'R'},
            {"sgrid",       no_argument,       0, 'S'},
            {"partition",   no_argument,       0, 't'},
            {"user",        no_argument,       0, 'u'},
            {"verbose",     no_argument,       0, 'v'},
            {"configfile",  required_argument, 0, 'C'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVAcfgjmnpl:qQrRStuvC:H:P:",
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

          case 'A':
        	  diagnose_info->mode = ACCT;
              break;

          case 'c':
        	  diagnose_info->mode = CLASS;
              break;

          case 'f':
        	  diagnose_info->mode = FS;
              break;

          case 'g':
        	  diagnose_info->mode = GROUP;
              break;

          case 'j':
        	  diagnose_info->mode = JOB;
              break;

          case 'l':
        	  switch (optarg[0]) {
				case 'o':
				case 'O':
					/* o-OFFPOLICY */
					diagnose_info->pType = OFF;
					break;

				case 'h':
				case 'H':
					/* h-HARDPOLICY */
					diagnose_info->pType = HARD;
					break;

				case 's':
				case 'S':
					/* s-SOFTPOLICY */
					diagnose_info->pType = SOFT;
					break;

				default:
		            print_usage();
		            exit(EXIT_FAILURE);
					break;
				}
			  break;

          case 'm':
              diagnose_info->mode = FRAME;
              break;

          case 'n':
              diagnose_info->mode = NODE;
              break;

          case 'p':
              diagnose_info->mode = PRIORITY;
              break;

          case 'q':
              diagnose_info->mode = QUEUE;
              break;

          case 'Q':
              diagnose_info->mode = QOS;
              break;

          case 'r':
              diagnose_info->mode = RSV;
              break;

          case 'R':
              diagnose_info->mode = RM;
              break;

          case 'S':
              diagnose_info->mode = SYS;
              break;

          case 't':
              diagnose_info->mode = PAR;
              break;

          case 'u':
              diagnose_info->mode = USER;
              break;

          case 'v':
        	  diagnose_info->flags |= (1 << VERBOSE);
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
              puts ("Try 'diagnose --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

    /* copy and save argument from input */
	if (optind != argc) {
		diagnose_info->argument = string_dup(argv[optind]);
	}

    return 1;
}

/* free memory */
void free_structs(diagnose_info_t *diagnose_info, client_info_t *client_info) {
    free(diagnose_info->pName);
    free(diagnose_info->argument);
    free(client_info->configfile);
    free(client_info->host);
}

void print_usage()
{
    puts ("\nUsage: diagnose [FLAGS]\n\n"
            "Display information about various aspects of the cluster and the results of internal diagnostic tests.\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n"
    		"\n"
    		"  -A, --account [ACCOUNTID]      provide detailed information about the accounts(aka\n"
    		"                                   projects) or a specified account Maui is currently\n"
    		"                                   tracking\n"
    		"  -c, --class [CLASSID]          provide detailed information about the classes or a\n"
    		"                                   specified class Maui is currently tracking\n"
    		"  -f, --fairshare [OBJECT]       display at a glance information about the fairshare\n"
    		"                                   configuration and historic resource utilization; if\n"
    		"                                   OBJECT is specified (one of: user, group, acct, class, or qos), then\n"
    		"                                   only information for that object type will be displayed \n"
    		"  -g, --group [GROUPID]          provide detailed information about the groups or a\n"
    		"                                   specified group Maui is currently tracking\n"
    		"  -j, --job [JOBID]              provide detailed information about the state of jobs or\n"
    		"                                   a specified job Maui is currently tracking\n"
    		"  -m, --frame [FRAMEID]          provide detailed information about the frames or a\n"
    		"                                   specified frame Maui is currently tracking\n"
    		"  -n, --node [NODEID]            provide detailed information about the state of jobs or\n"
    		"                                   a specified job Maui is currently tracking \n"
    		"  -p, --priority                 display information for priority components actually\n"
    		"                                   utilized\n"
    		"  -q, --queue[-l, --policylevel=POLICYLEVEL]\n"
    		"                                 display information for blocked jobs; if POLICYLEVEL is\n"
    		"                                   specified, then the policylevel will be set\n"
    		"  -Q, --qos                      present information about each QOS maintained by Maui\n"
    		"  -r, --reservation [RESERVATIONID]\n"
    		"                                 allow administrators to look at detailed reservation\n"
    		"                                   information\n"
    		"  -R, --rm                       present information about configured resource managers\n"
    		"  -S, --sgrid                    present information about the scheduler and grid\n"
    		"  -t, --partition [PARTITIONID]  display detailed information about the partitions or a \n"
    		"                                   specified partition Moab is currently tracking\n"
    		"  -u, --user [USERID]            present information about user records or a specified\n"
    		"                                   user record maintained by Maui\n"
    		"  -v, --verbose                  display additional attributes\n");
    print_client_usage();
}
