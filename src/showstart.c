/*
 * showstart standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

/** Struct for showstart options */
typedef struct _showstart_info {
    char *jobid;               /**< Job ID*/
} showstart_info_t;

static void free_structs(showstart_info_t *, client_info_t *);
static int process_args(int, char **, showstart_info_t *, client_info_t *);
static void print_usage();

int main(int argc, char **argv) {

    showstart_info_t showstart_info;
    client_info_t client_info;

	char *response;
	int sd, pc;
	long bufSize, now, deadline, startTime, wCLimit;
	char pName[MAXNAME], request[MAXBUFFER];

    memset(&showstart_info, 0, sizeof(showstart_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &showstart_info, &client_info)) {

		get_connection_params(&client_info);

		if (!connectToServer(&sd, client_info.port, client_info.host))
			exit(EXIT_FAILURE);

		generateBuffer(request, showstart_info.jobid, "showstart");

		if (!sendPacket(sd, request))
			exit(EXIT_FAILURE);

		if ((bufSize = getMessageSize(sd)) == 0)
			exit(EXIT_FAILURE);

		if ((response = (char *) calloc(bufSize + 1, 1)) == NULL) {
			puts("Error: cannot allocate memory for message");
			exit(EXIT_FAILURE);
		}

		/* receive message from server */
		if (!recvPacket(sd, &response, bufSize))
			exit(EXIT_FAILURE);

		/* print result */
		sscanf(strstr(response, "ARG=") + strlen("ARG="),
				"%s %ld %ld %ld %d %s", showstart_info.jobid, &now, &deadline,
				&wCLimit, &pc, pName);

		startTime = deadline - wCLimit;

		fprintf(stdout, "\njob %s requires %d proc%s for %s\n",
				showstart_info.jobid, pc, (pc == 1) ? "" : "s",
				timeToString(wCLimit));

		fprintf(stdout, "Earliest start in      %11s on %s",
				timeToString(startTime - now), getDateString(&startTime));

		fprintf(stdout, "Earliest completion in %11s on %s",
				timeToString(deadline - now), getDateString(&deadline));

		fprintf(stdout, "Best Partition: %s\n\n", pName);

		free(response);

    }

    free_structs(&showstart_info, &client_info);

    exit(0);
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 showstart_info_t *showstart_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"configfile",  required_argument, 0, 'C'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVC:H:P:",
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
              puts ("Try 'showstart --help' for more information.");
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
    showstart_info->jobid = string_dup(argv[optind]);

    return 1;
}

/* free memory */
void free_structs(showstart_info_t *showstart_info, client_info_t *client_info) {
    free(showstart_info->jobid);
    free(client_info->configfile);
    free(client_info->host);
}

void print_usage()
{
    puts ("\nUsage: showstart <JOBID>\n\n"
            "Attempt to determine earliest start time for the specified job and display the start\n"
            "time, completion time, procs required and best partition.\n"
    		"\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n");
    print_client_usage();
}
