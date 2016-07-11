/*
 * showq standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "msched-version.h"
#include "maui_utils.h"

/** Struct for showq options */
typedef struct _showq_info {
    char  *pName;               /**< Partition name */
    char  *username;            /**< User name */
    int   idle;                 /**< Show idle jobs */
    int   running;              /**< Show running jobs */
    int   blocked;              /**< Show blocked jobs */
} showq_info_t;

// local convenience functions

static int process_args(int, char **, showq_info_t *, client_info_t *);
static void print_usage();
static void free_structs(showq_info_t *, client_info_t *);

int main (int argc, char **argv)
{
    showq_info_t showq_info;
    client_info_t client_info;

    memset(&showq_info, 0, sizeof(showq_info));
    memset(&client_info, 0, sizeof(client_info));

    /* process all the options and arguments */
    if (process_args(argc, argv, &showq_info, &client_info)) {

        /* if username has been set, then only print that user's jobs*/
        if ((showq_info.username != NULL) && (strlen(showq_info.username) != 0))
            printf("only printing jobs for user %s\n", showq_info.username);

        if (showq_info.blocked)
            puts("printing information about all jobs in blocked state");

        if (showq_info.idle)
            puts("printing information about all jobs in idle state");

        if (showq_info.running)
            puts("printing information about all jobs in active state");

        /* if no flag has been set, print jobs in all states */
        if (showq_info.running + showq_info.idle + showq_info.blocked < 1) {
            puts( "printing information about all jobs in active, idle and blocked states");
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

    }

    free_structs(&showq_info,&client_info);
    return 0;
}

/*
 processes all the arguments
 returns 1 if the option requires more action to be done
 returns 0 if no more action needs to be done
*/
int process_args(int argc, char **argv,
                 showq_info_t *showq_info,
                 client_info_t *client_info)
{
    int c;
    while (1) {
        struct option options[] = {
            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'V'},
            {"block",       no_argument,       0, 'b'},
            {"idle",        no_argument,       0, 'i'},
            {"partition",   required_argument, 0, 'p'},
            {"running",     no_argument,       0, 'r'},
            {"username",    required_argument, 0, 'u'},
            {"configfile",  required_argument, 0, 'C'},
            {"loglevel",    required_argument, 0, 'D'},
            {"logfacility", required_argument, 0, 'F'},
            {"host",        required_argument, 0, 'H'},
            {"keyfile",     required_argument, 0, 'k'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hVbip:ru:C:D:F:H:k:P:",
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

          case 'b':
              puts ("Show blocked queue: blocked sets to 1\n");
              showq_info->blocked = 1;
              break;

          case 'i':
              puts ("Show idle queue: idle sets to 1\n");
              showq_info->idle = 1;
              break;

          case 'p':
              printf ("set partition to %s\n", optarg);
              showq_info->pName = string_dup(optarg);
              break;

          case 'r':
              puts ("Show running queue: running sets to 1\n");
              showq_info->running = 1;
              break;

          case 'u':
              printf ("set username to %s\n", optarg);
              showq_info->username = string_dup(optarg);
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
              puts ("Try 'showq --help' for more information.\n");
              return 0;

          default:
              abort();
        }
    }

    return 1;
}

/* frees memory*/
void free_structs(showq_info_t *showq_info, client_info_t *client_info){
    free(showq_info->username);
    free(showq_info->pName);
    free(client_info->configfile);
    free(client_info->host);
    free(client_info->logfacility);
}

void print_usage()
{
    puts ("\nUsage: showq [FLAGS]\n\n"
          "Display information about all jobs in active, idle and blocked states.\n"
          "\n"
          "  -h, --help                     display this help\n"
          "  -V, --version                  display client version\n"
          "\n"
          "  -b, --blocked                  display blocked jobs only\n"
          "  -i, --idle                     display idle jobs only\n"
          "  -r, --running                  display active jobs only\n"
          "  -p, --partition=PARTITIONID    display jobs assigned to the specified partition\n"
          "  -u, --username=USERNAMEID      display information about jobs for the specified user\n");
    print_client_usage();
}
