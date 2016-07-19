/*
 * sethold standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

/** Struct for sethold options */
typedef struct _sethold_info {
    char *jobid;              /**< Job ID */
    char *type;				  /**< Hole type */
} sethold_info_t;

static void free_structs(sethold_info_t *, client_info_t *);
static int process_args(int, char **, sethold_info_t *, client_info_t *);
static void print_usage();
static char *buildMsgBuffer(sethold_info_t );

int main(int argc, char **argv) {

    sethold_info_t sethold_info;
    client_info_t client_info;

	char *response, *result, *ptr, *msgBuffer;
	int sd;
	long bufSize;
	const char tmpLine[20] = "</SchedResponse>";
	char request[MAXBUFFER];

    memset(&sethold_info, 0, sizeof(sethold_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &sethold_info, &client_info)) {

		get_connection_params(&client_info);

		if (!connectToServer(&sd, client_info.port, client_info.host))
			exit(EXIT_FAILURE);

		msgBuffer = buildMsgBuffer(sethold_info);
		generateBuffer(request, msgBuffer, "mjobctl");
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

		printf("\n%s\n\n", ++result);

		free(response);

    }

    free_structs(&sethold_info, &client_info);

    exit(0);
}

/* combine and save information into a buffer */
char *buildMsgBuffer(sethold_info_t sethold_info) {
	char *buffer;
	int len = 0;

	if (sethold_info.type == NULL)
		sethold_info.type = string_dup("All");

	/* calculate the length of the whole buffer */
    len += strlen(sethold_info.jobid);
    len += strlen(sethold_info.type);

	if ((buffer = (char *) malloc(len + 92)) == NULL) {
		puts("ERROR: cannot allocate memory for buffer");
		return NULL;
	}

	/* build buffer */
	sprintf(buffer, "<schedrequest action=\"modify\" attr=\"Hold\" "
			"value=\"%s\" flag=\"set\" job=\"%s\"></schedrequest>\n",
			sethold_info.type, sethold_info.jobid);

	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 sethold_info_t *sethold_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"all",         no_argument,       0, 'a'},
            {"batch",       no_argument,       0, 'b'},
            {"sysrem",      no_argument,       0, 's'},
            {"user",        no_argument,       0, 'u'},
            {"configfile",  required_argument, 0, 'C'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVabsuC:H:P:",
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

          case 'a':
              sethold_info->type = string_dup("All");
              break;

          case 'b':
              sethold_info->type = string_dup("Batch");
              break;

          case 's':
              sethold_info->type = string_dup("System");
              break;

          case 'u':
              sethold_info->type = string_dup("User");
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
              puts ("Try 'sethold --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

    /* need one arguments */
    if(optind != argc - 1){
        print_usage();
        exit(EXIT_FAILURE);
    }

    /* copy and save job id from input */
    sethold_info->jobid= string_dup(argv[optind]);

    return 1;
}

/* free memory */
void free_structs(sethold_info_t *sethold_info, client_info_t *client_info) {
    free(sethold_info->jobid);
    free(sethold_info->type);
    free(client_info->configfile);
    free(client_info->host);
}

void print_usage()
{
    puts ("\nUsage: sethold [FLAGS] <JOBID>\n\n"
            "Set holds on a specified job. The default value of FLAGS is '-A'.\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n"
            "\n"
            "  -a, --all                      set all types of holds\n"
            "  -b, --batch                    set bacth holds\n"
            "  -s, --sysrem                   set system hold\n"
            "  -u, --user                     set user hold\n");
    print_client_usage();
}
