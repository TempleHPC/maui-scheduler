/*
 * setqos standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

/** Struct for setqos options */
typedef struct _setqos_info {
    char *value;               /**< Attribute name*/
    char *jobid;              /**< Attribute value */
} setqos_info_t;

static void free_structs(setqos_info_t *, client_info_t *);
static int process_args(int, char **, setqos_info_t *, client_info_t *);
static void print_usage();
static char *buildMsgBuffer(setqos_info_t *, client_info_t *);

int main(int argc, char **argv) {

    setqos_info_t setqos_info;
    client_info_t client_info;

	char *response, *ptr, *result, *msgBuffer;
	int sd;
	long bufSize;
	const char tmpLine[20] = "</SchedResponse>";
	char request[MAXBUFFER];

    memset(&setqos_info, 0, sizeof(setqos_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &setqos_info, &client_info)) {

		get_connection_params(&client_info);

		if (!connectToServer(&sd, client_info.port, client_info.host)){
			free_structs(&setqos_info, &client_info);
			exit(EXIT_FAILURE);
		}

		msgBuffer = buildMsgBuffer(&setqos_info, &client_info);
		generateBuffer(request, msgBuffer, "mjobctl");
		free(msgBuffer);

		if (!sendPacket(sd, request)){
			free_structs(&setqos_info, &client_info);
			exit(EXIT_FAILURE);
		}

		if ((bufSize = getMessageSize(sd)) == 0){
			free_structs(&setqos_info, &client_info);
			exit(EXIT_FAILURE);
		}

		if ((response = (char *) calloc(bufSize + 1, 1)) == NULL) {
			puts("ERROR: cannot allocate memory for message");
			free_structs(&setqos_info, &client_info);
			exit(EXIT_FAILURE);
		}

		/* receive message from server */
		if (!recvPacket(sd, &response, bufSize)){
			free_structs(&setqos_info, &client_info);
			exit(EXIT_FAILURE);
		}

		/* get and print result */
		result = strchr(response, '>');
		ptr = strstr(result, tmpLine);
		*ptr = '\0';

		printf("\n%s\n\n", ++result);

		free(response);

    }

    free_structs(&setqos_info, &client_info);

    exit(0);
}

/* combine and save information into a buffer */
char *buildMsgBuffer(setqos_info_t *setqos_info, client_info_t *client_info) {
	char *buffer;
	int len = 0;

	/* calculate the length of the whole buffer */
	len += strlen(setqos_info->value);
    len += strlen(setqos_info->jobid);

	if ((buffer = (char *) malloc(len + 89)) == NULL) {
		puts("ERROR: memory allocation failed");
		free_structs(setqos_info, client_info);
		exit(EXIT_FAILURE);
	}

	/* build buffer */
    sprintf(buffer,
            "<schedrequest action=\"modify\" attr=\"QOS\" value=\"%s\" "
            "flag=\"set\" job=\"%s\"></schedrequest>\n",
			setqos_info->value, setqos_info->jobid);

	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 setqos_info_t *setqos_info,
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
              free_structs(setqos_info, client_info);
              exit(EXIT_SUCCESS);
              break;

          case 'V':
              printf("Maui version %s\n", MSCHED_VERSION);
              free_structs(setqos_info, client_info);
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
              puts ("Try 'setqos --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

    /* only accept two arguments */
    if(optind != argc - 2){
        print_usage();
        free_structs(setqos_info, client_info);
        exit(EXIT_FAILURE);
    }

    /* copy and save attribute name from input */
    setqos_info->value = string_dup(argv[optind++]);

    /* copy and save attribute value from input */
    setqos_info->jobid= string_dup(argv[optind]);

    return 1;
}

/* free memory */
void free_structs(setqos_info_t *setqos_info, client_info_t *client_info) {
    free(setqos_info->value);
    free(setqos_info->jobid);
    free(client_info->configfile);
    free(client_info->host);
}

void print_usage()
{
    puts ("\nUsage: setqos [FLAGS] <QOS> <JOBID>\n\n"
            "Set Quality Of Service for a specified job.\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n");
    print_client_usage();
}