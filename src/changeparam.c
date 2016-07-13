/*
 * changeparam standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

/** Struct for changeparam options */
typedef struct _changeparam_info {
    char *attr;               /**< Attribute name*/
    char **value;              /**< Attribute value */
} changeparam_info_t;

static void free_structs(changeparam_info_t *, client_info_t *);
static int process_args(int, char **, changeparam_info_t *, client_info_t *);
static void print_usage();
static char *buildMsgBuffer(changeparam_info_t );

int main(int argc, char **argv) {

    changeparam_info_t changeparam_info;
    client_info_t client_info;

    char *response, request[MAXBUFFER], *msgBuffer;
    int sd, port;
    long bufSize;
    FILE *f;
    char configDir[MAXLINE];
    char *host;

    memset(&changeparam_info, 0, sizeof(changeparam_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &changeparam_info, &client_info)) {

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

		msgBuffer = buildMsgBuffer(changeparam_info);
		generateBuffer(request, msgBuffer, "changeparam");
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

    free_structs(&changeparam_info, &client_info);

    exit(0);
}

/* combine and save information into a buffer */
char *buildMsgBuffer(changeparam_info_t changeparam_info) {
	char *buffer;
	int i = 0, len = 0;

	/* calculate the length of the whole buffer */
	len += strlen(changeparam_info.attr) + 2;
    while ((changeparam_info.value)[i] != NULL) {
        len += strlen((changeparam_info.value)[i++]) + 2;
    }

	i = 0;
	if ((buffer = (char *) malloc(len + 2)) == NULL) {
		puts("ERROR: cannot allocate memory for buffer");
		return NULL;
	}

	/* build buffer */
	strcpy(buffer, changeparam_info.attr);
	strcat(buffer, " ");
	while ((changeparam_info.value)[i] != NULL) {
		strcat(buffer, (changeparam_info.value)[i++]);
		strcat(buffer, " ");
	}

	return buffer;
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 changeparam_info_t *changeparam_info,
                 client_info_t *client_info)
{
    int c, i;
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
              puts ("Try 'changeparam --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

    /* need at least two arguments */
    if(optind > argc - 2){
        print_usage();
        exit(EXIT_FAILURE);
    }

    /* allocate memory to save the string array from input*/
    changeparam_info->value = (char **) malloc((argc - optind + 1) * sizeof(char *));
    if(!changeparam_info->value){
        puts("ERROR: memory allocation failed");
        exit(EXIT_FAILURE);
    }

    /* copy and save attribute name from input */
    changeparam_info->attr = string_dup(argv[optind++]);

    /* copy and save attribute value from input */
    i = 0;
    while(optind < argc){
        (changeparam_info->value)[i++] = string_dup(argv[optind++]);
    }
    (changeparam_info->value)[i] = NULL;

    return 1;
}

/* free memory */
void free_structs(changeparam_info_t *changeparam_info, client_info_t *client_info) {
    free(changeparam_info->attr);
    free(changeparam_info->value);
    free(client_info->configfile);
    free(client_info->host);
    free(client_info->logfacility);
}

void print_usage()
{
    puts ("\nUsage: changeparam <PARAMETER> <VALUE> [VALUE]...\n\n"
            "Dynamically change the value of any configuration parameter which can be specified in the moab.cfg file.\n"
            "The changes take affect at the beginning of the next scheduling iteration. They are not persistent, only\n"
            "lasting until Moab is shutdown.\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n");
    print_client_usage();
}
