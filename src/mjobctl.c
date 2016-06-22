/*
 * mjobctl standalong client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "msched-version.h"
#include "maui_utils.h"

#define SUCCESS 1
#define FAIL 0

/** Enum for mjobctl actions */
enum mJobCtlCmdEnum {
    mjcmNONE = 0,
    mjcmCancel,
    mjcmCheckpoint,
    mjcmModify,
    mjcmQuery,
    mjcmRequeue,
    mjcmResume,
    mjcmSuspend,
};

/** Struct for mjobctl options */
typedef struct _mjobctl_info {
    int action;
    char *jobid;
    char *attr;
    char *value;
    char *mode;
} mjobctl_info_t;

static void free_structs(mjobctl_info_t *, client_info_t *);
static int process_args(int, char **, mjobctl_info_t *, client_info_t *);
static void print_usage();

int main(int argc, char **argv) {

    int result;
    mjobctl_info_t mjobctl_info;
    client_info_t client_info;

    memset(&mjobctl_info, 0, sizeof(mjobctl_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &mjobctl_info, &client_info)) {

        if (mjobctl_info.action == mjcmCancel) {
            printf("canceling job %s\n", mjobctl_info.jobid);
        }

        if (mjobctl_info.action == mjcmCheckpoint) {
            printf("checkpointing job %s\n", mjobctl_info.jobid);
        }

        if (mjobctl_info.action == mjcmModify) {
            printf("setting job %s attribute %s to %s\n", mjobctl_info.jobid, mjobctl_info.attr, mjobctl_info.value);
        }

        if (mjobctl_info.action == mjcmQuery) {
            printf("querying job %s attribute %s\n", mjobctl_info.jobid, mjobctl_info.attr);
        }

        if (mjobctl_info.action == mjcmResume) {
            printf("resuming job %s\n", mjobctl_info.jobid);
        }

        if (mjobctl_info.action == mjcmRequeue) {
            printf("requeuing job %s\n", mjobctl_info.jobid);
        }

        if (mjobctl_info.action == mjcmSuspend) {
            printf("suspending job %s\n", mjobctl_info.jobid);
        }

        if (client_info.configfile != NULL)
            printf("will use %s as configfile instead of default\n",client_info.configfile);
        if (client_info.loglevel > 0)
            printf("will use %d as loglevel instead of default\n",client_info.loglevel);
        if (client_info.logfacility != NULL)
            printf("will use %s as log facility instead of default\n",client_info.logfacility);
        if (client_info.host != NULL)
            printf("will contact %s as maui server instead of default\n",client_info.host);
        if (client_info.port > 0)
            printf("will use %d as server port instead of default\n",client_info.port);
        if (client_info.keyfile != NULL)
            printf("will use %s as key file instead of default\n",client_info.keyfile);
    }

    free_structs(&mjobctl_info, &client_info);

    exit(0);
}

/*
 processes all the arguments
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
            {"submit",      no_argument,       0, 'S'},
            {"configfile",  required_argument, 0, 'C'},
            {"loglevel",    required_argument, 0, 'D'},
            {"logfacility", required_argument, 0, 'F'},
            {"host",        required_argument, 0, 'H'},
            {"keyfile",     required_argument, 0, 'k'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVcem:o:q:rRsSC:D:F:H:K:P:",
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
              puts ("cancel job: action sets to mjcmCancel");
              mjobctl_info->action = mjcmCancel;
              break;

          case 'e':
              puts ("checkpoint job: action sets to mjcmCheckpoint");
              mjobctl_info->action = mjcmCheckpoint;
              break;

          case 'm':
              if ((optarg == NULL) || (strchr(optarg, '=') == NULL)){
                  printf("Error: no value found in argument\n");
                  return 0;
                  break;
              }

              mjobctl_info->action = mjcmModify;

              /* get attribute field field and value field*/
              char tempStr[128];
              strcpy(tempStr, optarg);
              char *attr = strtok(tempStr,"=");
              char *value = strtok(NULL, "=");
              mjobctl_info->attr = string_dup(attr);
              mjobctl_info->value = string_dup(value);
              mjobctl_info->mode = string_dup("set");
              printf("modify job attribute:  action sets to mjcmModify, attr sets to %s, value sets to %s\n",
                    mjobctl_info->attr, mjobctl_info->value);
              break;

          case 'o':
              mjobctl_info->action = mjcmModify;
              mjobctl_info->attr = string_dup("hold");
              mjobctl_info->value = string_dup(optarg);
              printf("modify job attribute: action sets to mjcmModify, attr sets to %s, value sets to %s\n",
                    mjobctl_info->attr, mjobctl_info->value);
              break;

          case 'q':
              printf ("query job attribute: action sets to mjcmQuery, attr sets to %s\n", optarg);
              mjobctl_info->action = mjcmQuery;
              mjobctl_info->attr = string_dup(optarg);
              break;

          case 'r':
              puts ("resume job: action sets to mjcmResume");
              mjobctl_info->action = mjcmResume;
              break;

          case 'R':
              puts ("requeue job: action sets to mjcmRequeue");
              mjobctl_info->action = mjcmRequeue;
              break;

          case 's':
              puts ("suspend job: action sets to mjcmSuspend");
              mjobctl_info->action = mjcmSuspend;
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

          case 'k':
              printf ("set keyfile to %s\n", optarg);
              client_info->keyfile = string_dup(optarg);
              break;

          case 'P':
              client_info->port = string2int(optarg);
              if (client_info->port != INVALID_STRING)
                  printf ("set port to %s\n", optarg);
              break;

          case '?':
              /* getopt_long already printed an error message. */
              puts ("Try 'showq --help' for more information.");
              return 0;

          default:
              abort();
        }
    }

    if (optind == argc) {
        puts("Error: no jobid found in argument");
        return 0;
    }else if(optind != argc - 1){
        puts("Error: only accept one argument(jobid)");
        return 0;
    }else{
        mjobctl_info->jobid = string_dup(argv[optind]);
    }


    return 1;
}

/* frees memory */
void free_structs(mjobctl_info_t *mjobctl_info, client_info_t *client_info) {
    free(mjobctl_info->attr);
    free(mjobctl_info->jobid);
    free(mjobctl_info->mode);
    free(mjobctl_info->value);
    free(client_info->configfile);
    free(client_info->host);
    free(client_info->keyfile);
    free(client_info->logfacility);
}

void print_usage()
{
    puts ("\nUsage: mjobctl <FLAGS> <JOBID>\n\n"
            "Modify attributes or perform operation on a specified job."
            "\n"
            "  -h, --help                     display this help\n"
            "  -V, --version                  display client version, serverhost, serverport, home\n"
            "                                   directory, build location, build host and build date\n"
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
