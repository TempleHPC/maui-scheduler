/*
 * mjobctl standalone client program code
 *
 * (c) 2016 Temple HPC Team
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "msched-version.h"
#include "maui_utils.h"



/** Struct for mjobctl options */
typedef struct _mjobctl_info {
    int action;               /**< Action name */
    char *jobid;              /**< Job ID */
    char *attr;               /**< Attribute name*/
    char *value;              /**< Attribute value */
    char *mode;               /**< Mode */
} mjobctl_info_t;

static void free_structs(mjobctl_info_t *, client_info_t *);
static int process_args(int, char **, mjobctl_info_t *, client_info_t *, char *);
static void print_usage();

const char *_MService[] = {NONE,          "showstate",       "setspri",
                          "setupri",     "showq",           "sethold",
                          "releasehold", "showhold",        "showstats",
                          "resetstats",  "setres",          "releaseres",
                          "showres",     "schedctl",        "diagnose",
                          "setdeadline", "releasedeadline", "showdeadline",
                          "showstart",   "setqos",          "showgrid",
                          "showbf",      "showconfig",      "checkjob",
                          "checknode",   "runjob",          "canceljob",
                          "changeparam", "migratejob",      "showstart",
                          "query",       "mjobctl",         "mgridctl",
                          "mnodectl",    "mresctl",         "mschedctl",
                          "mstat",       "mdiag",           "mshow",
                          "mbal",        "showtasks",       NULL};

int main(int argc, char **argv) {

    mjobctl_info_t mjobctl_info;
    client_info_t client_info;

    msocket_t S;

    time_t tmpTime;
    char *ptr;

    char SBuffer[MAX_MBUFFER];

    const char *FName = "main";
    mlog.logfp = stderr;
    DBG(4, fCORE) dPrint("%s(%d,argv)\n", FName, argc);

    dPrint("%s(%d,argv)\n", FName, Mode);

    getTime(&tmpTime, mtmInit, NULL);

    MSched.Time = tmpTime;

    /* set default environment */
    _MCInitialize();

    /* load config file */
    _MCLoadConfig(C.HomeDir, C.ConfigFile);

    /* load environment variables */
    __MCLoadEnvironment(ParName, C.ServerHost, &C.ServerPort);
    memset(&mjobctl_info, 0, sizeof(mjobctl_info));
    memset(&client_info, 0, sizeof(client_info));

    ptr = NULL;

    char MsgBuffer[MAX_MLINE << 3];

    /* process all the options and arguments */
    if (process_args(argc, argv, &mjobctl_info, &client_info, MsgBuffer)) {
        _MSUInitialize(&S, C.ServerHost, C.ServerPort, C.Timeout,
                      (1 << TCP));
        if (C.ServerCSKey[0] != '\0')
            strcpy(S.CSKey, C.ServerCSKey);
        else
            strcpy(S.CSKey, MSched.DefaultCSKey);

        printf("S.CSKey:%s, MSched.DefaultCSKey:%s",S.CSKey, MSched.DefaultCSKey);

        S.CSAlgo = MSched.DefaultCSAlgo;

        S.SocketProtocol = C.SocketProtocol;
        S.SBuffer = SBuffer;
        if (_MSUConnect(&S, FALSE, NULL) == FAILURE) {
            DBG(0, fSOCK)
            dPrint("ERROR:    cannot connect to '%s' port %d\n",
                   S.RemoteHost, S.RemotePort);

            exit(1);
        }
        sprintf(S.SBuffer, "%s%s %s%s %s%s\n", _MCKeyword[mckCommand],
        		_MService[31], _MCKeyword[mckAuth],
                _MUUIDToName(_MOSGetEUID()), _MCKeyword[mckArgs], MsgBuffer);
        printf("S.SBuffer:%s\n", S.SBuffer);
        S.SBufSize = (long)strlen(S.SBuffer);

        S.Timeout = C.Timeout;

        S.WireProtocol = mwpXML;

        /* attempt connection to primary server */
        if (_MCSendRequest(&S) == FAILURE) {
            if (MSched.FBServerHost[0] != '\0') {
                _MUStrCpy(S.RemoteHost, MSched.FBServerHost,
                         sizeof(S.RemoteHost));
                S.RemotePort = MSched.FBServerPort;

                /* attempt connection to secondary server */

                if (_MCSendRequest(&S) == FAILURE) {
                    DBG(0, fUI)
                    dPrint("ERROR:    cannot request service (status)\n");
                	printf("buffer1:%s\n", MsgBuffer);
                    exit(1);
                }
            } else {
                DBG(0, fUI)
                dPrint("ERROR:    cannot request service (status)\n");
            	printf("buffer2:%s\n", MsgBuffer);
                exit(1);
            }
        }

        if (((ptr = strchr(S.RBuffer, '<')) == NULL) ||
                            (_MXMLFromString((mxml_t **)&S.SE, ptr, NULL, NULL) ==
                             FAILURE)) {
                            DBG(0, fUI)
                            dPrint("ERROR:    cannot parse server response (status)\n");

                            exit(1);
        }

        mxml_t *E = (mxml_t *) S.SE;

		/* NOTE:  if query, must display job results */

		if (E->Val != NULL) {
			fprintf(stdout, "\n%s\n", E->Val);
		}

		if (E->CCount > 0) {
			int cindex;

			mxml_t *C;
			mxml_t *GC;

			for (cindex = 0; cindex < E->CCount; cindex++) {
				C = E->C[cindex];

				fprintf(stdout, "%s %s", C->Name, C->AVal[0]);

				if (C->CCount > 0) {
					GC = C->C[0];

					fprintf(stdout, " %s", GC->Val);
				}

				fprintf(stdout, "\n");
			} /* END for (cindex) */
		} /* END if (E->CCount > 0) */

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
                 client_info_t *client_info, char *msgBuffer)
{
    int option_index = 0;

    mxml_t *E = NULL;

    int c;

    _MXMLCreateE(&E, "schedrequest");

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
            {"keyfile",     required_argument, 0, 'k'},
            {"port",        required_argument, 0, 'P'},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, "hVcem:o:q:rRsC:D:F:H:K:P:",
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
              _MXMLSetAttr(E, "action", (void *)_MJobCtlCmds[mjcmCancel], mdfString);
              break;

          case 'e':
              puts ("checkpoint job: action sets to mjcmCheckpoint");
              _MXMLSetAttr(E, "action", (void *)_MJobCtlCmds[mjcmCheckpoint],
                          mdfString);
              break;

          case 'm':
              if ((optarg == NULL) || (strchr(optarg, '=') == NULL)){
                  printf("Error: no value found in argument\n");
                  return 0;
                  break;
              }

              mjobctl_info->action = mjcmModify;

              _MXMLSetAttr(E, "action", (void *)_MJobCtlCmds[mjcmModify],
                          mdfString);


              /* get attribute field and value field*/
              char tempStr[128];
              strcpy(tempStr, optarg);
              char *attr = strtok(tempStr,"=");
              char *value = strtok(NULL, "=");
              _MXMLSetAttr(E, "attr", attr, mdfString);
              _MXMLSetAttr(E, "value", value, mdfString);
              _MXMLSetAttr(E, "mode", "set", mdfString);


              break;

          case 'o':

              _MXMLSetAttr(E, "action", (void *)_MJobCtlCmds[mjcmModify],
                          mdfString);
              _MXMLSetAttr(E, "attr", "hold", mdfString);

              if ((optarg == NULL) || (optarg[0] == '\0'))
                  _MXMLSetAttr(E, "value", "user", mdfString);
              else
                  _MXMLSetAttr(E, "value", optarg, mdfString);

              break;

          case 'q':
              if (optarg == NULL) break;

              _MXMLSetAttr(E, "attr", optarg, mdfString);

              _MXMLSetAttr(E, "action", (void *)_MJobCtlCmds[mjcmQuery],
                          mdfString);
              break;

          case 'r':
              mjobctl_info->action = mjcmResume;
              break;

          case 'R':
              _MXMLSetAttr(E, "action", (void *)_MJobCtlCmds[mjcmResume],
                          mdfString);

              break;

          case 's':
              _MXMLSetAttr(E, "action", (void *)_MJobCtlCmds[mjcmSuspend],
                          mdfString);
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
              puts ("Try 'mjobctl --help' for more information.");
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
        _MXMLSetAttr(E, "job", argv[optind], mdfString);
    }

    _MXMLToString(E, msgBuffer, MAX_MLINE, NULL, TRUE);

    _MXMLDestroyE(&E);

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
