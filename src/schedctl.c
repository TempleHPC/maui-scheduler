/*
 * schedctl standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

#define KILL 0
#define RESUME 1
#define STEP 3
#define STOP 4
#define NODETABLE 5
#define SUBMIT 6
#define LIST 7
#define MODIFY 8
#define FAILURE1 9
#define INIT 10

/** Struct for schedctl options */
typedef struct _schedctl_info {
    char *argument;            /**< Argument */
    int   mode;			   	   /**< Mode */
} schedctl_info_t;

static void free_structs(schedctl_info_t *, client_info_t *);
static int process_args(int, char **, schedctl_info_t *, client_info_t *);
static void print_usage();
static char *buildMsgBuffer(schedctl_info_t );

int main(int argc, char **argv) {

    schedctl_info_t schedctl_info;
    client_info_t client_info;

	char *response, *msgBuffer;
	int sd;
	long bufSize;
	char request[MAXBUFFER];

    memset(&schedctl_info, 0, sizeof(schedctl_info));
    memset(&client_info, 0, sizeof(client_info));

    schedctl_info.mode = -1;
    schedctl_info.argument = string_dup("-1");

    /* process all the options and arguments */
    if (process_args(argc, argv, &schedctl_info, &client_info)) {

		get_connection_params(&client_info);

		if (!connectToServer(&sd, client_info.port, client_info.host))
			exit(EXIT_FAILURE);

		msgBuffer = buildMsgBuffer(schedctl_info);
		generateBuffer(request, msgBuffer, "schedctl");
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

    free_structs(&schedctl_info, &client_info);

    exit(0);
}

/* combine and save information into a buffer */
char *buildMsgBuffer(schedctl_info_t schedctl_info) {
	char *buffer;
	int len = 0;

	if (schedctl_info.mode == -1) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	/* calculate the length of the whole buffer */
	len += strlen(schedctl_info.argument) + 1;

	if ((buffer = (char *) malloc(len + 3)) == NULL) {
		puts("ERROR: cannot allocate memory for buffer");
		return NULL;
	}

	/* build buffer */
    sprintf(buffer, "%d %s", schedctl_info.mode, schedctl_info.argument);

	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 schedctl_info_t *schedctl_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"failure",     no_argument,       0, 'f'},
            {"initialize",  no_argument,       0, 'i'},
            {"job",         required_argument, 0, 'j'},
            {"kill",        no_argument, 	   0, 'k'},
            {"list",        no_argument, 	   0, 'l'},
            {"modify",      required_argument, 0, 'm'},
            {"node",        no_argument,	   0, 'n'},
            {"resume",      no_argument,	   0, 'r'},
            {"stop",        no_argument, 	   0, 's'},
            {"step",        no_argument,       0, 'S'},
            {"configfile",  required_argument, 0, 'C'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVfij:klm:nrsSC:H:P:",
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

          case 'f':
        	  schedctl_info->mode = FAILURE1;
              break;

          case 'i':
        	  schedctl_info->mode = INIT;
              break;

          case 'j':
        	  schedctl_info->mode = SUBMIT;
        	  schedctl_info->argument = string_dup(optarg);
              break;

          case 'k':
        	  schedctl_info->mode = KILL;
              break;

          case 'l':
        	  schedctl_info->mode = LIST;
              break;

          case 'm':
        	  schedctl_info->mode = MODIFY;
        	  schedctl_info->argument = string_dup(optarg);
              break;

          case 'n':
        	  schedctl_info->mode = NODETABLE;
              break;

          case 'r':
        	  schedctl_info->mode = RESUME;
              break;

          case 's':
        	  schedctl_info->mode = STOP;
              break;

          case 'S':
        	  schedctl_info->mode = STEP;
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
              puts ("Try 'schedctl --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

	/* only accpet one argument */
	if (optind < argc - 1) {
		print_usage();
		exit(EXIT_FAILURE);
	} else if (optind == argc - 1) {
		/* copy and save argument from input */
		schedctl_info->argument = string_dup(argv[optind]);
	}

    return 1;
}

/* free memory */
void free_structs(schedctl_info_t *schedctl_info, client_info_t *client_info) {
    free(schedctl_info->argument);
    free(client_info->configfile);
    free(client_info->host);
}

void print_usage()
{
    puts ("\nUsage: schedctl [FLAGS]\n\n"
            "Control various aspects of scheduling behavior.\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n"
    		"\n"
    		"  -f, --failure[=DURATION]       enable RM failure mode for DURATION time; disable RM failure mode if\n"
    		"                                   DURATION is less or equal to zero\n"
    		"  -i, --initialize               attempt to initialize RM\n"
			"  -j, --job=JOBID                attempt to submit a job, loading specified trace\n"
			"  -K, --kill                     shutdown the scheduler\n"
			"  -l, --list[=PARAMETERID]       view the current configurable parameters or a specified\n"
			"                                   parameter\n"
			"  -m, --modify <PARAMETER>=<VALUE>\n"
			"                                 modify a specified parameter\n"
			"  -n, --node                     display a node table trace(for use in simulation mode)\n"
			"  -r, --resume                   resume scheduling immediately\n"
			"  -s, --stop[=ITERATION]         stop scheduling at iteration ITERATION or at the completion\n"
			"                                   of the current scheduling iteration if not specified;\n"
			"                                   If ITERATION is followed by the letter 'I', maui will not\n"
			"                                   process client requests until this iteration is reached\n"
			"  -S, --step[=VALUE]             stop scheduling after VALUE iterations or in one more\n"
			"                                   iterations if not specified. If VALUE is followed by\n"
			"                                   the letter 'I', maui will not process client requests\n"
    		"                                   until VALUE more scheduling iterations have been\n"
    		"                                   completed\n");
    print_client_usage();
}
