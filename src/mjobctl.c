/*
 * mjobctl standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include "msched-version.h"
#include "maui_utils.h"

/** Struct for mjobctl options */
typedef struct _mjobctl_info {
    char *action;               /**< Action name */
    char *jobid;              /**< Job ID */
    char *attr;               /**< Attribute name*/
    char *value;              /**< Attribute value */
    char *mode;               /**< Mode */
} mjobctl_info_t;

static void free_structs(mjobctl_info_t *, client_info_t *);
static int process_args(int, char **, mjobctl_info_t *, client_info_t *);
static void print_usage();
static char *buildXML(mjobctl_info_t);

int main(int argc, char **argv) {
    mjobctl_info_t mjobctl_info;
    client_info_t client_info;

    char *response, request[MAXBUFFER], *result, *XMLBuffer;
    int sd, port;
    long bufSize;
    const char tmpLine[20] = "</SchedResponse>";
    FILE *f;
    char configDir[MAXLINE];
    char *ptr, *host;

    memset(&mjobctl_info, 0, sizeof(mjobctl_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
	if (process_args(argc, argv, &mjobctl_info, &client_info)) {

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

		XMLBuffer = buildXML(mjobctl_info);
		generateBuffer(request, XMLBuffer, "mjobctl");
		free(XMLBuffer);

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

		/* get and print result*/
		result = strchr(response, '>');
		ptr = strstr(result, tmpLine);
		*ptr = '\0';

		printf("\n%s\n", ++result);

		free(response);
		free(host);
	}

	free_structs(&mjobctl_info, &client_info);

    exit(0);
}

/*
 process all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/
int process_args(int argc, char **argv,
                 mjobctl_info_t *mjobctl_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {

            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"cancel",      no_argument,       0, 'c'},
            {"checkpoint",  no_argument,       0, 'e'},
            {"modify",      required_argument, 0, 'm'},
            {"hold",        required_argument, 0, 'o'},
            {"query",       required_argument, 0, 'q'},
            {"resume",      no_argument,       0, 'r'},
            {"requeue",     no_argument,       0, 'R'},
            {"suspend",     no_argument,       0, 's'},
            {"configfile",  required_argument, 0, 'C'},
            {"loglevel",    required_argument, 0, 'D'},
            {"logfacility", required_argument, 0, 'F'},
            {"host",        required_argument, 0, 'H'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVcem:o:q:rRsC:D:F:H:P:",
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
              mjobctl_info->action = string_dup("cancel");
              break;

          case 'e':
              mjobctl_info->action = string_dup("checkpoint");
              break;

          case 'm':
              if ((optarg == NULL) || (strchr(optarg, '=') == NULL)){
                  printf("ERROR: no value found in argument\n");
                  return 0;
                  break;
              }

              mjobctl_info->action = string_dup("modify");

              /* get attribute field and value field*/
              char tempStr[128];
              strcpy(tempStr, optarg);
              char *attr = strtok(tempStr,"=");
              char *value = strtok(NULL, "=");
              mjobctl_info->attr = string_dup(attr);
              mjobctl_info->value = string_dup(value);
              mjobctl_info->mode = string_dup("set");
              break;

          case 'o':
              mjobctl_info->action = string_dup("modify");
              mjobctl_info->attr = string_dup("Hold");
              mjobctl_info->value = string_dup(optarg);
              break;

          case 'q':
              mjobctl_info->action = string_dup("query");
              mjobctl_info->attr = string_dup(optarg);
              break;

          case 'r':
              mjobctl_info->action = string_dup("resume");
              break;

          case 'R':
              mjobctl_info->action = string_dup("requeue");
              break;

          case 's':
              mjobctl_info->action = string_dup("suspend");
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
              puts ("Try 'mjobctl --help' for more information.");
              return 0;

          default:
              abort();
        }
    }

    if (optind == argc) {
        puts("ERROR: no jobid found in argument");
        return 0;
    }else if(optind != argc - 1){
        puts("ERROR: only accept one argument(jobid)");
        return 0;
    }else{
        mjobctl_info->jobid = string_dup(argv[optind]);
    }


    return 1;
}

/** build XML buffer
 *
 * This function will take the object passed as argument,
 * build and return the buffer in XML format.
 *
 * @param input object to build buffer
 * @return string buffer.
 */

char *buildXML(mjobctl_info_t mjobctl_info){
	char *XMLBuffer;

	if((XMLBuffer = (char *)malloc(MAXLINE)) == NULL){
		puts("Erro: cannot allocate memory to XML buffer");
		return NULL;
	}

	strcpy(XMLBuffer, "<schedrequest action=\"");
	strcat(XMLBuffer, mjobctl_info.action);
	strcat(XMLBuffer,"\"");

	if(mjobctl_info.attr != NULL){
		strcat(XMLBuffer, " attr=\"");
		strcat(XMLBuffer, mjobctl_info.attr);
		strcat(XMLBuffer,"\"");
	}

	strcat(XMLBuffer, " job=\"");
	strcat(XMLBuffer, mjobctl_info.jobid);
	strcat(XMLBuffer,"\"");

	if(mjobctl_info.mode != NULL){
		strcat(XMLBuffer, " mode=\"");
		strcat(XMLBuffer, mjobctl_info.mode);
		strcat(XMLBuffer,"\"");
	}

	if(mjobctl_info.value != NULL){
		strcat(XMLBuffer, " value=\"");
		strcat(XMLBuffer, mjobctl_info.value);
		strcat(XMLBuffer,"\"");
	}

	strcat(XMLBuffer,"></schedrequest>");

	return XMLBuffer;
}

/* free memory */
void free_structs(mjobctl_info_t *mjobctl_info, client_info_t *client_info) {
    free(mjobctl_info->attr);
    free(mjobctl_info->jobid);
    free(mjobctl_info->mode);
    free(mjobctl_info->value);
    free(client_info->configfile);
    free(client_info->host);
    free(client_info->logfacility);
}

void print_usage()
{
    puts ("\nUsage: mjobctl <FLAGS> <JOBID>\n\n"
            "Modify attributes or perform operation on a specified job."
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version\n"
            "\n"
            "  -c, --cancel                   attempt to cancel a job\n"
            "  -e, --checkpoint               attempt to checkpoint a job\n"
            "  -m, --modify <ATTR>=<VALUE>    attempt to set a specified job attribute\n"
            "  -o, --hold[=HOLDVALUE]         attempt to set 'hold' attribute\n"
            "  -q, --query <ATTR>             query a specified job attribute\n"
            "  -r, --resume                   attempt to resume a job\n"
            "  -R, --requeue                  attempt to requeue a job\n"
            "  -s, --suspend                  attempt to suspend a job\n");
    print_client_usage();
}
