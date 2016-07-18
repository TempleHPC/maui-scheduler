/*
 * mnodectl standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

#define VERBOSE 4
#define AVP 6

/** Struct for mnodectl options */
typedef struct _checknode_info {
	char *subcommand;		   /**< Subcommand */
	char *argument;			   /**< Argument */
    char *nodeid;              /**< Node ID */
} checknode_info_t;

static void free_structs(checknode_info_t *, client_info_t *);
static int process_args(int, char **, checknode_info_t *, client_info_t *);
static void print_usage();
static char *buildMsgBuffer(checknode_info_t );

int main(int argc, char **argv) {

    checknode_info_t checknode_info;
    client_info_t client_info;

    char *response, request[MAXBUFFER], *msgBuffer;
    int sd, port;
    long bufSize;
    FILE *f;
    const char tmpLine[20] = "</SchedResponse>";
    char configDir[MAXLINE];
    char *host, *result, *ptr;

    memset(&checknode_info, 0, sizeof(checknode_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &checknode_info, &client_info)) {

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

		msgBuffer = buildMsgBuffer(checknode_info);
		generateBuffer(request, msgBuffer, "mnodectl");
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

		/* get and print result */
		result = strchr(response, '>');
		ptr = strstr(result, tmpLine);
		*ptr = '\0';

		printf("\n%s\n", ++result);

		free(host);
		free(response);

    }

    free_structs(&checknode_info, &client_info);

    exit(0);
}

/* combine and save information into a buffer */
char *buildMsgBuffer(checknode_info_t checknode_info) {
	char *buffer;
	int len = 0;

	/* calculate the length of the whole buffer */
	len += strlen(checknode_info.nodeid) + 2;
	len += strlen(checknode_info.subcommand) + 2;
	len += strlen(checknode_info.argument) + 2;

	if ((buffer = (char *) malloc(len + 4)) == NULL) {
		puts("ERROR: cannot allocate memory for buffer");
		return NULL;
	}

	/* build buffer */
	sprintf(buffer, "%s %s %s", checknode_info.subcommand,
			checknode_info.argument, checknode_info.nodeid);

	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 checknode_info_t *checknode_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"configfile",  required_argument, 0, 'C'},
            {"loglevel",    required_argument, 0, 'D'},
            {"logfacility", required_argument, 0, 'F'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVC:D:F:H:P:",
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
              puts ("Try 'mnodectl --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

    /* need three arguments */
    if(optind != argc - 3){
        print_usage();
        exit(EXIT_FAILURE);
    }

    /* copy and save node id from input */
    checknode_info->subcommand = string_dup(argv[optind++]);
    checknode_info->argument = string_dup(argv[optind++]);
    checknode_info->nodeid = string_dup(argv[optind++]);

    return 1;
}

/* free memory */
void free_structs(checknode_info_t *checknode_info, client_info_t *client_info) {
    free(checknode_info->nodeid);
    free(checknode_info->subcommand);
    free(checknode_info->argument);
    free(client_info->configfile);
    free(client_info->host);
    free(client_info->logfacility);
}

void print_usage()
{
    puts ("\nUsage: mnodectl [FLAGS] <SUBCOMMAND> <ARG> <NODEID>\n\n"
            "Modify attributes or perform operation on a specified node in simulation mode. SUBCOMMAND can be 'create',\n"
    		"'destroy', or 'modify'. ARG can be 'resource', 'state', or 'trace'.\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n");
    print_client_usage();
}
