/*
 * showtasks standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

/** Struct for showtasks options */
typedef struct _showtasks_info {
    char *username;               /**< Username*/
} showtasks_info_t;

static void free_structs(showtasks_info_t *, client_info_t *);
static int process_args(int, char **, showtasks_info_t *, client_info_t *);
static void print_usage();

int main(int argc, char **argv) {

    showtasks_info_t showtasks_info;
    client_info_t client_info;

    char *response;
    int sd;
    long bufSize;
    char request[MAXBUFFER];

    memset(&showtasks_info, 0, sizeof(showtasks_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &showtasks_info, &client_info)) {

		get_connection_params(&client_info);

		if (!connectToServer(&sd, client_info.port, client_info.host)){
			free_structs(&showtasks_info, &client_info);
			exit(EXIT_FAILURE);
		}

		generateBuffer(request, showtasks_info.username, "showtasks");

		if (!sendPacket(sd, request)){
			free_structs(&showtasks_info, &client_info);
			exit(EXIT_FAILURE);
		}

		if ((bufSize = getMessageSize(sd)) == 0){
			free_structs(&showtasks_info, &client_info);
			exit(EXIT_FAILURE);
		}

		if ((response = (char *) calloc(bufSize + 1, 1)) == NULL) {
			puts("Error: cannot allocate memory for message");
			free_structs(&showtasks_info, &client_info);
			exit(EXIT_FAILURE);
		}

		/* receive message from server */
		if (!recvPacket(sd, &response, bufSize)){
			free_structs(&showtasks_info, &client_info);
			exit(EXIT_FAILURE);
		}

		printf("\nThe number of tasks running for %s is ", showtasks_info.username);
		printf("%s\n", strstr(response, "ARG=") + strlen("ARG="));

		free(response);

    }

    free_structs(&showtasks_info, &client_info);

    exit(0);
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/

int process_args(int argc, char **argv,
                 showtasks_info_t *showtasks_info,
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
              free_structs(showtasks_info, client_info);
              exit(EXIT_SUCCESS);
              break;

          case 'V':
              printf("Maui version %s\n", MSCHED_VERSION);
              free_structs(showtasks_info, client_info);
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
              puts ("Try 'showtasks --help' for more information.");
              return 0;

          default:
              //abort();
              return 0;
        }
    }

    /* only need one argument */
    if(optind != argc - 1){
        print_usage();
        free_structs(showtasks_info, client_info);
        exit(EXIT_FAILURE);
    }

    /* copy and save username from input */
    showtasks_info->username = string_dup(argv[optind]);

    return 1;
}

/* free memory */
void free_structs(showtasks_info_t *showtasks_info, client_info_t *client_info) {
    free(showtasks_info->username);
    free(client_info->configfile);
    free(client_info->host);
}

void print_usage()
{
    puts ("\nUsage: showtasks <USERNAME>\n\n"
            "Query the number of tasks running for a given user.\n"
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n");
    print_client_usage();
}
