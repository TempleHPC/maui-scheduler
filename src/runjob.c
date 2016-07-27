/*
 * runjob standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

/** Struct for runjob options */
typedef struct _runjob_info {
    char *pName;               /**< Partition name*/
    char *jobid;               /**< Job ID */
    char *mode;			   	   /**< Mode */
    char *nodeid;			   /**< Node ID */
} runjob_info_t;

static void free_structs(runjob_info_t *, client_info_t *);
static int process_args(int, char **, runjob_info_t *, client_info_t *);
static void print_usage();
static char *buildMsgBuffer(runjob_info_t );

int main(int argc, char **argv) {

    runjob_info_t runjob_info;
    client_info_t client_info;

	char *response, *ptr, *msgBuffer;
	int sd;
	long bufSize;
	char request[MAXBUFFER];

    memset(&runjob_info, 0, sizeof(runjob_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &runjob_info, &client_info)) {

    	if (runjob_info.pName == NULL) {
    		if ((ptr = getenv(MSCHED_ENVPARVAR)) != NULL) {
    			runjob_info.pName = string_dup(ptr);
    		} else {
    			runjob_info.pName = string_dup(GLOBAL_MPARNAME);
    		}
    	}

		get_connection_params(&client_info);

		if (!connectToServer(&sd, client_info.port, client_info.host))
			exit(EXIT_FAILURE);

		msgBuffer = buildMsgBuffer(runjob_info);
		generateBuffer(request, msgBuffer, "runjob");
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

    free_structs(&runjob_info, &client_info);

    exit(0);
}

/* combine and save information into a buffer */
char *buildMsgBuffer(runjob_info_t runjob_info) {
	char *buffer;
	int len = 0;

	if(runjob_info.nodeid == NULL)
		runjob_info.nodeid = string_dup(NONE);

	if(runjob_info.mode == NULL)
		runjob_info.mode = string_dup("NONE");

	/* calculate the length of the whole buffer */

	/* plus one for a white space or a 0*/
	len += strlen(runjob_info.jobid) + 1;
	len += strlen(runjob_info.pName) + 1;
	len += strlen(runjob_info.nodeid) + 1;
	len += strlen(runjob_info.mode) + 1;

	if ((buffer = (char *) malloc(len)) == NULL) {
		puts("ERROR: cannot allocate memory for buffer");
		return NULL;
	}

	/* build buffer */
	sprintf(buffer, "%s %s %s %s", runjob_info.jobid, runjob_info.mode,
			runjob_info.pName, runjob_info.nodeid);

	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 runjob_info_t *runjob_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"clear",       no_argument,       0, 'c'},
            {"force", 		no_argument, 	   0, 'f'},
            {"node",  		required_argument, 0, 'n'},
            {"partition",   required_argument, 0, 'p'},
            {"suspend",     no_argument,       0, 's'},
            {"force2",      no_argument,       0, 'x'},
            {"configfile",  required_argument, 0, 'C'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVcfn:p:sxC:H:P:",
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

          case 'c':
        	  runjob_info->mode = string_dup("CLEAR");
              break;

          case 'f':
        	  runjob_info->mode = string_dup("FORCE");
			  break;

          case 'n':
              if (strlen(optarg) >= (MAXLINE << 2)) {
				fprintf(stderr, "ERROR: expression too long. (%d > %d)\n",
						(int)strlen(optarg), (MAXLINE << 2));

				exit(EXIT_FAILURE);
              }

              runjob_info->nodeid = string_dup(optarg);
              break;

          case 'p':
        	  runjob_info->pName = string_dup(optarg);
			  break;

          case 's':
        	  runjob_info->mode = string_dup("BLOCK");
              break;

          case 'x':
        	  runjob_info->mode = string_dup("FORCE2");
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
              puts ("Try 'runjob --help' for more information.");
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
    runjob_info->jobid = string_dup(argv[optind]);

    return 1;
}

/* free memory */
void free_structs(runjob_info_t *runjob_info, client_info_t *client_info) {
    free(runjob_info->jobid);
    free(runjob_info->nodeid);
    free(runjob_info->pName);
    free(runjob_info->mode);
    free(client_info->configfile);
    free(client_info->host);
}

void print_usage()
{
    puts ("\nUsage: runjob [FLAGS] <JOBID>\n\n"
            "Attempt to immediately start the specified job.\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n"
    		"\n"
    		"  -c, --clear                    override hostlist(specify nodes with '-n' flag)\n"
    		"  -f, --force                    attempt to force the job to run, ignoring policies\n"
    		"  -n, --node=NODEID[:NODEID]     attempt to start the job using the specified nodes\n"
    		"  -p, --partition                attempt to start the job in the specified partition\n"
    		"  -s, --suspend                  attempt to suspend the job\n"
    		"  -x, --force2                   attempt to force the job to run, ignoring policies, QoS\n"
    		"                                   flags, and reservations\n");
    print_client_usage();
}
