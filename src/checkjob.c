/*
 * checkjob standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

#define OFF 1
#define SOFT 3
#define HARD 4
#define VERBOSE 4
#define AVP 6

/** Struct for checkjob options */
typedef struct _checkjob_info {
    int   pType;               /**< Policy type*/
    char *jobid;               /**< Job ID */
    int   flags;			   /**< flags */
    char *resid;			   /**< Reservation ID */
    char *nodeid;			   /**< Node ID */
} checkjob_info_t;

static void free_structs(checkjob_info_t *, client_info_t *);
static int process_args(int, char **, checkjob_info_t *, client_info_t *);
static void print_usage();
static char *buildMsgBuffer(checkjob_info_t );

int main(int argc, char **argv) {

    checkjob_info_t checkjob_info;
    client_info_t client_info;

    char *response, request[MAXBUFFER], *msgBuffer;
    int sd, port;
    long bufSize;
    FILE *f;
    char configDir[MAXLINE];
    char *host;

    memset(&checkjob_info, 0, sizeof(checkjob_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &checkjob_info, &client_info)) {

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

		fclose(f);

		if (!connectToServer(&sd, port, host))
			exit(EXIT_FAILURE);

		msgBuffer = buildMsgBuffer(checkjob_info);
		generateBuffer(request, msgBuffer, "checkjob");
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

		printf("\n%s\n", strstr(response, "ARG=") + strlen("ARG="));

		free(host);
		free(response);

    }

    free_structs(&checkjob_info, &client_info);

    exit(0);
}

/* combine and save information into a buffer */
char *buildMsgBuffer(checkjob_info_t checkjob_info) {
	char *buffer;
	int len = 0;

	if(checkjob_info.nodeid == NULL)
		checkjob_info.nodeid = string_dup(NONE);

	if(checkjob_info.resid == NULL)
		checkjob_info.resid = string_dup(NONE);

	/* calculate the length of the whole buffer */
	len += strlen(checkjob_info.jobid) + 2;
	len += strlen(checkjob_info.resid) + 2;
	len += strlen(checkjob_info.nodeid) + 2;

    if (checkjob_info.pType == 0)
    	checkjob_info.pType = SOFT;

	if ((buffer = (char *) malloc(len + 5)) == NULL) {
		puts("ERROR: cannot allocate memory for buffer");
		return NULL;
	}

	/* build buffer */
	sprintf(buffer, "%d %s %d %s %s", checkjob_info.pType,
			checkjob_info.jobid, checkjob_info.flags, checkjob_info.resid,
			checkjob_info.nodeid);

	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 checkjob_info_t *checkjob_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"avp",         no_argument,       0, 'A'},
            {"policylevel", required_argument, 0, 'l'},
            {"node",  		required_argument, 0, 'n'},
            {"reservation", required_argument, 0, 'r'},
            {"verbose",     no_argument,       0, 'v'},
            {"configfile",  required_argument, 0, 'C'},
            {"loglevel",    required_argument, 0, 'D'},
            {"logfacility", required_argument, 0, 'F'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVAl:n:r:vC:D:F:H:P:",
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
        	  checkjob_info->flags |= (1 << AVP);
              break;

          case 'l':
        	  switch (optarg[0]) {
				case 'o':
				case 'O':
					/* o-OFFPOLICY */
					checkjob_info->pType = OFF;
					break;

				case 'h':
				case 'H':
					/* h-HARDPOLICY */
					checkjob_info->pType = HARD;
					break;

				case 's':
				case 'S':
					/* s-SOFTPOLICY */
					checkjob_info->pType = SOFT;
					break;

				default:
		            print_usage();
		            exit(EXIT_FAILURE);
					break;
				}
			  break;

          case 'n':
              if (strlen(optarg) >= (MAXLINE << 2)) {
				fprintf(stderr, "ERROR: expression too long. (%d > %d)\n",
						(int)strlen(optarg), (MAXLINE << 2));

				exit(EXIT_FAILURE);
              }

              checkjob_info->nodeid = string_dup(optarg);
              break;

          case 'r':
        	  checkjob_info->resid = string_dup(optarg);
              break;

          case 'v':
        	  checkjob_info->flags |= (1 << VERBOSE);
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
              puts ("Try 'checkjob --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

    /* only need one argument */
    if(optind != argc - 1){
        print_usage();
        exit(EXIT_FAILURE);
    }

    /* copy and save job id from input */
    checkjob_info->jobid = string_dup(argv[optind]);

    return 1;
}

/* free memory */
void free_structs(checkjob_info_t *checkjob_info, client_info_t *client_info) {
    free(checkjob_info->jobid);
    free(checkjob_info->nodeid);
    free(checkjob_info->resid);
    free(client_info->configfile);
    free(client_info->host);
    free(client_info->logfacility);
}

void print_usage()
{
    puts ("\nUsage: checkjob [FLAGS] <JOBID>\n\n"
            "Display detailed job state information and diagnostic output for a specified job.\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n"
    		"\n"
    		"  -A, --avp                      provide output in the form of parsable Attribute-Value\n"
    		"                                   pairs\n"
    		"  -l, --policylevel=POLICYLEVEL  report job start eligibility subject to specified\n"
    		"                                   throttling policy level. POLICYLEVEL can be 'hard',\n"
    		"                                   'soft', or 'off'\n"
    		"  -n, --node=NODEID              check job access to specified node\n"
    		"  -r, --reservation=RESID        check job access to specified reservation\n"
    		"  -v, --verbose                  display detailed availability information about nodes\n");
    print_client_usage();
}
