/* */
        
#include "mclient.h"
#include "mclient-proto.h"
#include "msched-version.h"

#include "moab.h"
#include "msched-proto.h"

#define MAX_MCARGS  128

extern mattrlist_t MAList;
extern m64_t       M64;

int MCResCreate(char *);
int MCJobShow(char *);
int MCNodeShow(char *);
int MCClusterShow(char *);

int MCJobCtlCreateRequest(char **,int,char *);
int MCResCtlCreateRequest(char **,int,char *);
int MCShowCreateRequest(char **,int,char *);
int MCBalCreateRequest(char **,int,char *);

/* function prototypes */

int MCGridCtl(msocket_t *);
int MCNodeCtl(msocket_t *);
int MCJobCtl(msocket_t *);
int MCResCtl(msocket_t *);
int MCBal(msocket_t *);
int MCShow(msocket_t *);

int __MCLoadEnvironment(char *,char *,int *);
int __MCInitialize();
int __MCLoadArgs(int,char **,int *);
int __OMCProcessArgs(int,char **,int *);
int __MCLoadConfig(char *,char *);
int __MCGetArgList(int,char *);
int MCShowUsage(int);
int MCExtractData(char *,mxml_t **,mxml_t **);

/* END function prototypes */



int (*MCFunction[])() = {
  NULL,
  MCClusterShow,
  MCSetJobSystemPrio,
  MCSetJobUserPrio,
  MCShowQueue,
  MCSetJobHold,
  MCReleaseJobHold,
  MCShowJobHold,
  MCShowStats,
  MCResetStats,
  MCResCreate,
  MCReleaseReservation,
  MCShowReservation,
  MCSchedCtl,
  MCDiagnose,
  MCSetJobDeadline,
  MCReleaseJobDeadline,
  MCShowJobDeadline,
  MCShowEStart,
  MCSetJobQOS,
  MCShowGrid,
  MCShowBackfillWindow,
  MCShowConfig,
  MCJobShow,
  MCNodeShow,
  MCRunJob,
  MCCancelJob,
  MCChangeParameter,
  MCMigrateJob,
  MCShowEstimatedStartTime,
  MCBShowState,
  MCJobCtl,
  MCGridCtl,
  MCNodeCtl,
  MCResCtl,
  NULL,
  NULL,
  NULL,
  MCShow,
  MCBal,
	showTasksPerUser};


extern mlog_t mlog;

extern msocket_t MClS[];
extern mcfg_t    MCfg[];

extern mx_t      X;

mccfg_t   C;

extern mclass_t  MClass[];
extern mgcred_t *MUser[];
extern mgcred_t  MGroup[];
extern mqos_t    MQOS[]; 
extern mgcred_t  MAcct[];     
extern mnode_t  *MNode[];
extern mjob_t   *MJob[];
extern mstat_t   MStat;
extern mpar_t    MPar[];
extern mckpt_t   MCP;
extern mrm_t     MRM[];
extern mjobl_t   MJobName[MAX_MJOB + MAX_MHBUF];

mrmfunc_t MRMFunc[MAX_MRMTYPE];
mjob_t   *MJobTraceBuffer;

extern mres_t   *MRes[];         /* terminated with empty name */
extern mrange_t  MRange[];
extern mre_t     MRE[];
extern sres_t    SRes[];
extern sres_t    OSRes[];
extern mam_t     MAM[];

extern mframe_t  MFrame[];

/* globals */

char      *AList[32];

long       BeginTime = 0;
long       EndTime   = MAX_MTIME;
long       Duration  = 0;

int        NodeCount = 0;
int        ProcCount = 0;
int        TaskCount = 0;

int        MIndex;
int        Memory;
int        DMemory;

int        UseXML = FALSE;

int        Mode = 0;

int        HType;
int        ObjectType;

char       ObjectID[MAX_MNAME];

char       ShowUserName[MAX_MNAME];

char       UserList[MAX_MLINE];
char       GroupList[MAX_MLINE];
char       AccountList[MAX_MLINE];
char       QOSList[MAX_MLINE];
char       ClassList[MAX_MLINE];

char       QLine[MAX_MLINE];
char       QOSName[MAX_MNAME];

char       ResourceList[MAX_MLINE];
char       FeatureString[MAX_MLINE];
char       JobFeatureString[MAX_MLINE];
char       NodeSetString[MAX_MLINE];
char       ClassString[MAX_MLINE];

int        RType;

char       NodeRegEx[MAX_MLINE << 2];
char       JobRegEx[MAX_MLINE << 2];
char       ResList[MAX_MLINE << 2];
char      *RegEx;

int        PType;
int        BFMode;
int        DiagMode;
int        MGridMode;
int        JobCtlMode;
int        BalMode;
int        SchedMode;
int        PrioMode;
char       ChargeAccount[MAX_MNAME];
char       SchedArg[MAX_MLINE];
char       DiagOpt[MAX_MNAME];

long       ClientTime;

char       ParName[MAX_MNAME];
int        QueueMode;
int        Flags;
char       UserName[MAX_MNAME];
char       Group[MAX_MNAME];
char       Account[MAX_MNAME];
char       Type[MAX_MLINE];
char       RangeList[MAX_MLINE];
char       ResDesc[MAX_MLINE];
char       ResFlags[MAX_MLINE];

msocket_t  Msg;
char       MsgBuffer[MAX_MLINE << 3];

extern msched_t   MSched;

/* extern globals */
 
extern const char *MComp[];
extern const char *MService[];
extern const char *MCKeyword[];
extern const char *MHoldType[];
extern const char *MClientMode[];
extern const char *MJobCtlCmds[];
extern const char *MGridCtlCmds[];
extern const char *MResCtlCmds[];
extern const char *MBalCmds[];
extern const char *MNodeState[];
extern const char *MCDisplayType[];
extern const char *MStatType[];
extern const char *MSockProtocol[];
extern const char *MXO[];
extern const char *MJobAttr[];
extern const char *MNResAttr[];
extern const char *MSysAttr[];
extern const char *MResAttr[];
extern const char *MClusterAttr[];
extern const char *MSchedMode[];



/* scheduler user interface client */

#include "omclient.c"
#include "mclient-stub.c"




int main(
 
  int    argc,  /* I */
  char **argv)  /* I */
 
  {
  int                 CIndex;
  char               *ptr;
  int                 scode;
 
  char                SBuffer[MAX_MBUFFER];
 
  time_t              tmpTime;
 
  msocket_t           S;
 
  int                 rc;

  const char *FName = "main";
  memset(temp_str,0,MMAX_LINE);

  mlog.logfp = stderr;
 
  DBG(4,fCORE) DPrint("%s(%d,argv)\n",
    FName,
    argc);
 
  MUGetTime(&tmpTime,mtmInit,NULL);
 
  MSched.Time = tmpTime;
 
  /* load configuration:  precedence = <CMDLINE> > <ENV> > <CFGFILE> > <DFLT> */

  /* set default environment */

  __MCInitialize();

  /* load config file */

  __MCLoadConfig(C.HomeDir,C.ConfigFile);

  /* load environment variables */
 
  __MCLoadEnvironment(ParName,C.ServerHost,&C.ServerPort); 
 
  /* load command line flags */
 
  __MCLoadArgs(argc,argv,&CIndex);
 
  Msg.SIndex = CIndex;
/*
  Msg.RID = MOSGetEUID(); 
 
  DBG(2,fCORE) DPrint("INFO:     service requested by '%s' (UID: %d)\n",
    MUUIDToName(Msg.RID),
    Msg.RID);
*/

  ptr = NULL;
 
  switch(CIndex)
    {
    case svcSched:
    case svcDiagnose:
    case svcClusterShow:
    case svcSetJobSystemPrio:
    case svcSetJobUserPrio:
    case svcShowQ:
    case svcSetJobHold:
    case svcReleaseJobHold:
    case svcShowJobHold:
    case svcShowStats:
    case svcResetStats:
    case svcResCreate:
    case svcResDestroy:
    case svcResShow:
    case svcSetJobDeadline:
    case svcReleaseJobDeadline:
    case svcShowJobDeadline:
    case svcShowEarliestDeadline:
    case svcSetJobQOS:
    case svcShowGrid:
    case svcShowBackfillWindow:
    case svcShowConfig:
    case svcJobShow:
    case svcNodeShow:
    case svcRunJob:
    case svcCancelJob: 
    case svcChangeParameter:
    case svcMigrateJob:
    case svcShowEstimatedStartTime:
    case svcBNFQuery:
    case svcMGridCtl:
    case svcMNodeCtl:
    case svcMJobCtl:
    case svcMShow:
    case svcMResCtl:
    case svcMBal:
    case svcShowTasks:
      MSUInitialize(
        &S,
        C.ServerHost,
        C.ServerPort,
        C.Timeout,
        (1 << TCP));
      if (C.ServerCSKey[0] != '\0')
        strcpy(S.CSKey,C.ServerCSKey);
      else
        strcpy(S.CSKey,MSched.DefaultCSKey);

      S.CSAlgo         = MSched.DefaultCSAlgo;

      S.SocketProtocol = C.SocketProtocol;
      S.SBuffer        = SBuffer;  
      if (MSUConnect(&S,FALSE,NULL) == FAILURE)
        {
        DBG(0,fSOCK) DPrint("ERROR:    cannot connect to '%s' port %d\n",
          S.RemoteHost,
          S.RemotePort);
 
        exit(1);
        }
      sprintf(S.SBuffer,"%s%s %s%s %s%s\n",
        MCKeyword[mckCommand],
        MService[CIndex],
        MCKeyword[mckAuth],
        MUUIDToName(MOSGetEUID()),
        MCKeyword[mckArgs],
        MsgBuffer);
      S.SBufSize = (long)strlen(S.SBuffer);

      S.Timeout = C.Timeout;

      switch(CIndex)
        {
        case svcMJobCtl:
        case svcMShow:
        case svcMNodeCtl:
        case svcMResCtl:
        case svcMBal:

          S.WireProtocol = mwpXML;

          break;

        default:

          /* NO-OP */

          break;
        }  /* END switch(CIndex) */

      /* attempt connection to primary server */
      if (MCSendRequest(&S) == FAILURE)
        {
        if (MSched.FBServerHost[0] != '\0')
          {
          MUStrCpy(S.RemoteHost,MSched.FBServerHost,sizeof(S.RemoteHost));
          S.RemotePort = MSched.FBServerPort;

          /* attempt connection to secondary server */

          if (MCSendRequest(&S) == FAILURE)
            {
            DBG(0,fUI) DPrint("ERROR:    cannot request service (status)\n");

            exit(1);
            }
          }
        else
          {
          DBG(0,fUI) DPrint("ERROR:    cannot request service (status)\n");
 
          exit(1);
          }
        }
      if (S.WireProtocol == mwpXML)
        {
        if (((ptr = strchr(S.RBuffer,'<')) == NULL) ||
             (MXMLFromString((mxml_t **)&S.SE,ptr,NULL,NULL) == FAILURE))
          {
          DBG(0,fUI) DPrint("ERROR:    cannot parse server response (status)\n");

          exit(1);
          }

        scode = scSUCCESS;
        }
      else
        {
        /* locate StatusCode */
 
        if ((ptr = strstr(S.RBuffer,MCKeyword[mckStatusCode])) == NULL)
          {
          DBG(0,fUI) DPrint("ERROR:    cannot parse server response (status)\n");
 
          exit(1);
          }
 
        ptr += strlen(MCKeyword[mckStatusCode]);
 
        scode = (int)strtol(ptr,NULL,0);

        switch(S.SocketProtocol)
          {
          case mspHalfSocket:

            /* NO-OP */

            break;

          default:

            /* locate Data */
 
            if ((ptr = strstr(S.RBuffer,MCKeyword[mckData])) == NULL)
              {
              DBG(0,fUI) DPrint("ERROR:    cannot parse server response (data)\n");
 
              exit(1);
              }

            ptr += strlen(MCKeyword[mckData]);

            break;
          }  /* END switch(S->SocketProtocol) */

        /* locate Args (optional) */

        if (strstr(ptr,MCKeyword[mckArgs]) != NULL)
          {
          ptr = strstr(ptr,MCKeyword[mckArgs]) + strlen(MCKeyword[mckArgs]);
          }			 
				
        S.RPtr = ptr;
        }  /* END else (S.WireProtocol == mwpXML) */
      if (scode == scSUCCESS)
        {
        switch(CIndex)
          {
          case svcMJobCtl:
          case svcMNodeCtl:
          case svcMResCtl:
          case svcMShow:
          case svcMBal:
 
            rc = (*MCFunction[CIndex])(&S);  

            break;

          default:
            rc = (*MCFunction[CIndex])(S.RPtr,S.RBufSize - (ptr - S.RBuffer));

            break;
          }  /* END switch(CIndex) */
 
        MSUFree(&S); 

        if (S.SE != NULL)
          MXMLDestroyE((mxml_t **)&S.SE);
 
        if (rc == SUCCESS)
          exit(0);
        else
          exit(1);
        }
      else
        {
        DBG(0,fUI)
          {
          fprintf(stderr,"ERROR:    '%s' failed\n",
            MService[CIndex]);
          }
 
        /* display message from server */
 
        if (S.RPtr[0] != '\0')
          {
          fprintf(stderr,"%s\n",
            S.RPtr);
          }
 
        MSUFree(&S);
 
        exit(1);
        }
 
      break;
 
    default:
 
      DBG(0,fUI) DPrint("ERROR:    Service[%d] '%s' not implemented\n",
        CIndex,
        MService[CIndex]);
 
      exit(1);
 
      break;
    }  /* END switch(CIndex) */ 
 
  return(SUCCESS);
  }  /* END main() */





int __MCInitialize()
 
  {
  char *ptr;
  char *buf;
 
  int   count;
  int   SC;
 
  char tmpLine[MAX_MLINE];
 
  const char *FName = "__MCInitialize";
 
  DBG(2,fALL) DPrint("%s()\n",
    FName);

  M64Init(&M64);

  MUBuildPList(MCfg,MParam);
 
  strcpy(C.ServerHost,DEFAULT_MSERVERHOST);
  C.ServerPort = DEFAULT_MSERVERPORT;
 
  strcpy(C.HomeDir,MBUILD_HOMEDIR);
  strcpy(C.ConfigFile,DEFAULT_SchedCONFIGFILE);

  MSched.UID = MOSGetEUID();

#if defined(MBUILD_DIR)
  strncpy(C.BuildDir,MBUILD_DIR,sizeof(C.BuildDir));
#else
  strcpy(C.BuildDir,"NA");
#endif /* MBUILD_DIR */

#if defined(MBUILD_HOST)
  strncpy(C.BuildHost,MBUILD_HOST,sizeof(C.BuildHost));
#else
  strcpy(C.BuildHost,"NA");
#endif /* MBUILD_HOST */

#if defined(MBUILD_DATE)
  strncpy(C.BuildDate,MBUILD_DATE,sizeof(C.BuildDate));
#else
  strcpy(C.BuildDate,"NA");
#endif /* MBUILD_DATE */
 
  MSUIPCInitialize(); 
 
  /* setup defaults */
 
  HType        = -1;
 
  ObjectType   = -1;
  RType        = -1;
  PType        = -1;
 
  BFMode       =  0;
  DiagMode     = -1;
  MGridMode    =  0;
  JobCtlMode   =  0;
  BalMode      =  0;
  SchedMode    = -1;
  PrioMode     = -1;
 
  strcpy(ChargeAccount,NONE);
 
  strcpy(DiagOpt,NONE);
 
  strcpy(SchedArg,"-1");
 
  QueueMode    = -1;
 
  Flags        =  0;
 
  Memory       =  0;
  DMemory      =  0; 
 
  MIndex       =  0;
 
  strcpy(UserName,MUUIDToName(MOSGetUID()));
  strcpy(Group,MUGIDToName(getgid()));
  strcpy(Account,"ALL");
 
  strcpy(UserList,NONE);
  strcpy(GroupList,NONE);
  strcpy(AccountList,NONE);
  strcpy(ClassList,NONE);
  strcpy(QOSList,NONE);
  strcpy(ObjectID,NONE);

  ShowUserName[0] = '\0';
 
  strcpy(ResourceList,NONE);
  strcpy(ClassString,NONE);
  strcpy(FeatureString,NONE);
  strcpy(JobFeatureString,NONE);
  strcpy(QOSName,NONE);
  strcpy(NodeSetString,NONE);
  strcpy(ResFlags,NONE);
 
  strcpy(Type,"DEFAULT");
  sprintf(RangeList,"%d,%d",
    0,
    MAX_MTIME);
 
  strcpy(ResDesc,"DEFAULT");
 
  memset(&Msg,0,sizeof(Msg)); 
 
  NodeRegEx[0] = '\0';
  strcpy(ResList,NONE);
   
  /* load environment required for configfile 'bootstrap' */
 
  if ((ptr = getenv(MSCHED_ENVHOMEVAR)) != NULL)
    {
    DBG(4,fCONFIG) DPrint("INFO:     using %s environment variable (value: %s)\n",
      MSCHED_ENVHOMEVAR,
      ptr);
 
    MUStrCpy(C.HomeDir,ptr,sizeof(C.HomeDir));
    }
  else if ((ptr = getenv(MParam[pMServerHomeDir])) != NULL)
    {
    DBG(4,fCONFIG) DPrint("INFO:     using %s environment variable (value: %s)\n",
      MParam[pMServerHomeDir],
      ptr);
 
    MUStrCpy(C.HomeDir,ptr,sizeof(C.HomeDir));
    }
  else
    {
    /* check master configfile */
 
    if ((buf = MFULoad(MASTER_CONFIGFILE,1,macmRead,&count,&SC)) != NULL)
      {
      if (((ptr = strstr(buf,MParam[pMServerHomeDir])) != NULL) ||
          ((ptr = strstr(buf,"MAUIHOMEDIR")) != NULL))
        {
        MUSScanF(ptr,"%x%s %x%s", 
          sizeof(tmpLine),
          tmpLine,
          sizeof(C.HomeDir),
          C.HomeDir);
        }
      }
    }    /* END else (ptr = getenv()) */
 
  if ((ptr = getenv(MParam[pSchedConfigFile])) != NULL)
    {
    DBG(4,fCONFIG) DPrint("INFO:     using %s environment variable (value: %s)\n",
      MParam[pSchedConfigFile],
      ptr);
 
    MUStrCpy(C.ConfigFile,ptr,sizeof(C.ConfigFile));
    }

  {
  int UseAuthFile;

  sprintf(MSched.KeyFile,"%s/%s",
    C.HomeDir,
    MSCHED_KEYFILE);

  MUCheckAuthFile(
    &MSched,
    MSched.DefaultCSKey,
    &UseAuthFile,
    FALSE);

  if (UseAuthFile == TRUE)
    MSched.DefaultCSAlgo = mcsaRemote;
  }  /* END BLOCK */
 
  return(SUCCESS);
  }  /* END __MCInitialize() */




int __MCGetArgList(

  int   CIndex, 
  char *ArgString)

  {
  if (ArgString == NULL)
    {
    return(FAILURE);
    }

  /* set global command line flags */
 
  /* C-CONFIGFILE, D-DEBUG, F-LOGFACILITY, h-HELP, P-PORT, V-VERSION, ?-HELP, --EXTENDEDFLAGS */
 
  strcpy(ArgString,"C:D:F:hP:V?-:");
 
  switch(CIndex)
    {
    case svcResetStats:
    case svcShowGrid:
    case svcSetJobUserPrio:
    case svcSetJobQOS:
    case svcShowJobHold:
    case svcShowEstimatedStartTime:
    case svcResDestroy:
    case svcSetJobDeadline:
    case svcReleaseJobDeadline:
    case svcShowJobDeadline:
    case svcShowEarliestDeadline:
    case svcChangeParameter:
    case svcCancelJob:
    case svcMigrateJob:
 
      /* no command-specific flags */
 
      break;

    case svcSetJobSystemPrio:

      /* r-RELATIVE */

      strcat(ArgString,"r");

      break;

    case svcClusterShow:

      /* x-XML */

      strcat(ArgString,"x");

      break;
 
    case svcBNFQuery: 
 
      /* c-COMMAND */
 
      strcat(ArgString,"c:");
 
      break;
 
    case svcMGridCtl:
 
      /* c-COMMIT l-LIST m-MODIFY q-QUERY r-RELEASE s-SET S-STAGE t-TYPE T-TIME */
 
      strcat(ArgString,"c:lmqrsSt:T:");
 
      break;
 
    case svcMNodeCtl:
 
      /* f: ??? n: ??? p: ??? */
 
      strcat(ArgString,"fn:p:");
 
      break;
 
    case svcMJobCtl:
 
      /* c-CANCEL h-HOLD m-MODIFY p-PRIORITY q-QUERY r-RESUME s-SUBMIT S-SUSPEND */
 
      strcat(ArgString,"cfh:m:n:p:q:r:sS"); 
 
      break;

    case svcMResCtl:

      /* c-CREATE m-MODIFY q-QUERY r-RELEASE */

      strcat(ArgString,"cfm:q:r:");

      break;
 
    case svcRunJob:
 
      /* c-CLEAR, f-FORCE, n-NODELIST, p-PARTITION, s-SUSPEND, x-FORCE2 */
 
      strcat(ArgString,"cfn:p:sx");
 
      break;
 
    case svcShowConfig:
 
      /* v-VERBOSE */
 
      strcat(ArgString,"v");
 
      break;
 
    case svcShowTasks:
    case svcShowQ:
 
      /* a-???, b-BLOCKED, i-IDLE, p-PARTITION, r-RUNNING, u-USERNAME, v-VERBOSE */
 
      strcat(ArgString,"abip:ru:v");
 
      break;
 
    case svcSetJobHold:
    case svcReleaseJobHold: 
 
      /* a-ALL, b-BATCH, s-SYSTEM, u-USER */
 
      strcat(ArgString,"absu");
 
      break;
 
    case svcJobShow:
    case svcNodeShow:
 
      /* A-AVP, l-LEVEL, n-NODE, r-RESERVATION, s-SERIALIZED */
 
      strcat(ArgString,"Al:n:r:v");
 
      break;
 
    case svcDiagnose:
 
      /* a-ACCOUNT, c-class, f-FAIRSHARE, g-GROUP, j-JOB, l-LEVEL, m-fraMe, n-NODE, p-PRIORITY, t-PARTITION, q-QUEUE, Q-QOS, r-RESERVATION, R-RM, s-SR, S-SYS, u-USERs, v-VERBOSE */
 
      strcat(ArgString,"acfgGjl:mnpqQrRsSt:uv");
 
      break;
 
    case svcShowStats:
 
      /* a-ACCOUNT, c-CLASS, d-DEPTH g-GROUP, n-NODE p-PARTITION q-QOS, s-SCHEDULER, S-SUMMARY, u-USER v-VERBOSE */
 
      strcat(ArgString,"a:c:d:g:np:q:sSu:v"); 
 
      break;
 
    case svcResCreate:
 
      /* a-ACCOUNTLIST, A-CHARGEACCOUNT, c-CLASSLIST, d-DURATION, e-ENDTIME, f-FEATURELIST, g-GROUPLIST, j-JOBFEATURE, n-NAME, N-NODESET, p-PARTITION, q-QUEUE, Q-QOS, r-RESOURCES, s-STARTTIME, t-TASKS, u-USERLIST, x-FLAGLIST */
 
      strcat(ArgString,"a:A:c:d:e:f:g:j:n:N:p:q:Q:r:s:t:u:vx:");
 
      break;
 
    case svcResShow:
 
      /* g-GREP, n-NODE, p-PARTITION, r-RELATIVEMODE, s-SUMMARY, v-VERBOSE */
 
      strcat(ArgString,"gnp:rsv");
 
      break;
 
    case svcSched:
 
      /* f-FAILURE, i-INIT, j-JOB, k-KILL, l-LIST, m-MODIFY, n-NODETABLE, r-RESUME, R-RECONFIG, s-STOP, S-STEP, v-VERBOSE */
 
      strcat(ArgString,"f:i:j:klm:nr:Rs:S:vV");
 
      break; 
 
    case svcShowBackfillWindow:
 
      /* A-ALL, a-ACCOUNT, c-CLASS, d-DURATION, f-FEATURE, g-GROUP, m-MEMORY, M-DMEMORY, n-NODES, p-PARTITION,
q-QOS, r-pROC, s-SMP*, u-USER v-VERBOSE */
 
      strcat(ArgString,"Aa:c:d:f:g:m:M:n:p:q:r:Su:vV");
 
      break;
 
    default:
 
      DBG(0,fCONFIG) DPrint("ERROR:  command '%s' args not handled\n",
        MService[CIndex]);
 
      break;
    } /* END switch(CIndex) */
 
  return(SUCCESS);
  }  /* END __MCGetArgList() */





int __MCLoadEnvironment(

  char *ParName,  /* O */
  char *Host,     /* O */
  int  *Port)     /* O */

  {
  char *ptr;

  /* load environment variables */

  /* load partition */
 
  if ((ptr = getenv(MSCHED_ENVPARVAR)) != NULL)
    {
    DBG(4,fCONFIG) DPrint("INFO:     loaded environment variable %s=%s\n",
      MSCHED_ENVPARVAR,
      ptr);
 
    MUStrCpy(ParName,ptr,MAX_MNAME);
    }
  else
    {
    DBG(5,fCONFIG) DPrint("INFO:     partition not set  (using default)\n");
 
    MUStrCpy(ParName,GLOBAL_MPARNAME,MAX_MNAME);
    }
 
  /* get port environment variable */
 
  if ((ptr = getenv(MParam[pServerPort])) != NULL)
    {
    DBG(4,fCONFIG) DPrint("INFO:     loaded environment variable %s=%s)\n",
      MParam[pServerPort],
      ptr);
 
    *Port = (int)strtol(ptr,NULL,0);
    }

  if ((ptr = getenv(MParam[pServerHost])) != NULL)
    {
    DBG(4,fCONFIG) DPrint("INFO:     using %s environment variable (value: %s)\n",
      MParam[pServerHost],
      ptr);
 
    MUStrCpy(Host,ptr,MAX_MNAME);
    }

  return(SUCCESS);
  }  /* END __MCLoadEnvironment() */




int __MCLoadConfig(
 
  char *Directory,  /* I */
  char *ConfigFile) /* I */
 
  {
  char *buf;
  char *ptr;
 
  char  Name[MAX_MLINE + 1];
  char  tmpLine[MAX_MLINE];
 
  int   index;
 
  int   count;
 
  int   SC;

  const char *FName = "__MCLoadConfig";
 
  DBG(2,fUI) DPrint("%s(%s,%s)\n",
    FName,
    Directory,
    ConfigFile);
 
  if ((ConfigFile[0] == '/') || (ConfigFile[0] == '~'))
    {
    strcpy(Name,ConfigFile);
    }
  else
    {
    if (Directory[strlen(Directory) - 1] == '/') 
      sprintf(Name,"%s%s",
        Directory,
        ConfigFile);
    else
      sprintf(Name,"%s/%s",
        Directory,
        ConfigFile);
    }
 
  if ((buf = MFULoad(Name,1,macmRead,&count,&SC)) == NULL)
    {
    DBG(0,fCONFIG) DPrint("WARNING:  cannot open configfile '%s'\n",
      Name);
 
    DBG(0,fCONFIG) DPrint("INFO:     using internal defaults\n");
 
    return(FAILURE);
    }
 
  MCfgAdjustBuffer(&buf,TRUE);

  MSched.ConfigBuffer = buf;
 
  MCfgGetSVal(
    MSched.ConfigBuffer,
    NULL,
    MParam[pServerHost],
    NULL,
    NULL,
    C.ServerHost,
    sizeof(C.ServerHost),
    1,
    NULL);
 
  MCfgGetIVal(
    MSched.ConfigBuffer,
    NULL,
    MParam[pServerPort],
    NULL,
    NULL,
    &C.ServerPort,
    NULL);
 
  MCfgGetSVal(
    MSched.ConfigBuffer,
    NULL,
    MParam[pSchedMode],
    NULL,
    NULL,
    C.SchedulerMode,
    sizeof(C.SchedulerMode),
    1,
    NULL);

  if (MCfgGetSVal(
        MSched.ConfigBuffer,
        NULL,
        MParam[pMCSocketProtocol],
        NULL,
        NULL,
        tmpLine,
        sizeof(tmpLine),
        1,
        NULL) == SUCCESS)
    {
    C.SocketProtocol = MUGetIndex(tmpLine,MSockProtocol,0,mspSingleUseTCP);
    }

  /* get admin4 user */

  if (MCfgGetSVal(
      MSched.ConfigBuffer,
      NULL,
      MParam[mcoAdmin4Users],
      NULL,
      NULL,
      tmpLine,
      sizeof(tmpLine),
      1,
      NULL) == SUCCESS)
    {
    MUStrCpy(MSched.Admin4User[0],tmpLine,MAX_MNAME);
    }
    
  /* get timeout */
 
  if (MCfgGetSVal(
      MSched.ConfigBuffer,
      NULL,
      MParam[pClientTimeout],
      NULL,
      NULL,
      tmpLine,
      sizeof(tmpLine),
      1,
      NULL) == SUCCESS)
    {
    C.Timeout = MUTimeFromString(tmpLine) * 1000000;
 
    if (C.Timeout <= 0)
      C.Timeout = DEFAULT_CLIENTTIMEOUT;
    }
  else
    {
    C.Timeout = DEFAULT_CLIENTTIMEOUT; 
    }
 
  if ((ptr = getenv(MParam[pClientTimeout])) != NULL)
    {
    C.Timeout = MUTimeFromString(ptr) * 1000000;
 
    if (C.Timeout <= 0)
      {
      C.Timeout = DEFAULT_CLIENTTIMEOUT;
      }
    }
 
  MCfgGetSVal(
    MSched.ConfigBuffer,
    NULL,
    MParam[pDisplayFlags],
    NULL,
    NULL,
    tmpLine,
    sizeof(tmpLine),
    0,
    NULL);
 
  C.DisplayFlags = 0;
 
  if (tmpLine[0] != '\0')
    {
    for (index = 0;MCDisplayType[index] != NULL;index++)
      {
      if (strstr(tmpLine,MCDisplayType[index]) != NULL)
        {
        C.DisplayFlags |= (1 << index);
        }
      }    /* END for (index) */
 
    DBG(2,fALL) DPrint("INFO:     %s set to %x\n",
      MParam[pDisplayFlags],
      (int)C.DisplayFlags);
    }  /* END if (tmpLine[0] != '\0') */

  if (MSchedLoadConfig(NULL) == SUCCESS)
    {
    if (MSched.ServerPort > 0)
      C.ServerPort = MSched.ServerPort;

    if (MSched.ServerHost[0] != '\0')
      strcpy(C.ServerHost,MSched.ServerHost);
 
    if (MSched.Mode != 0)
      strcpy(C.SchedulerMode,MSchedMode[MSched.Mode]);
    }  /* END if (MSchedLoadConfig() == SUCCESS) */
 
  return(SUCCESS);
  }  /* END __MCLoadConfig() */





/* command display routines */


int MCJobShow(
 
  char *Buffer)  /* I */
 
  {
  const char *FName = "MCJobShow";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);
 
  fprintf(stdout,"\n\n%s\n",
    Buffer);
 
  if (strstr(Buffer,"ERROR:") != NULL)
    {
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MCJobShow() */
 
 
 
 

int MCNodeShow(
 
  char *Buffer)
 
  {
  DBG(2,fUI) DPrint("MCNodeShow(Buffer)\n");
 
  fprintf(stdout,"\n\n%s\n",
    Buffer);
 
  if (strstr(Buffer,"ERROR:") != NULL)
    return(FAILURE);
 
  return(SUCCESS);
  }  /* END MCNodeShow() */




int MCClusterShow(
 
  char *Buffer)
 
  {
  char *ptr;
  char *OPtr;

  int   index;
  int   oindex;
 
  msys_t System;
 
  char *TokPtr;
  char *TokPtr2;
 
  unsigned char mapj[MAX_MFRAMECOUNT][MAX_MSLOTPERFRAME + 1];

  char  tmpLine[MAX_MLINE]; 
  char  tmpName[MAX_MNAME];
  char  tmpState[MAX_MNAME];
 
  char  tmpNLString[MAX_MBUFFER];
 
  int   SChar;
 
  char  JName[MAX_MJOB][MAX_MNAME];
  char  JUName[MAX_MJOB][MAX_MNAME];
  char  JGName[MAX_MJOB][MAX_MNAME];
  char  JState[MAX_MJOB][MAX_MNAME];
  long  JStartTime[MAX_MJOB];
  int   JNCount[MAX_MJOB];
  int   JPCount[MAX_MJOB];
  long  JDuration[MAX_MJOB];
 
  int   jindex;
  int   findex;
  int   sindex; 
 
  char  Warnings[MAX_MBUFFER];
  int   TotalNodes;
 
  long  Now;
  int   MaxSlotPerFrame;
  int   MaxFrame;

  int   SlotsUsed;
 
  char *FPtr;
  char *SPtr;

  int   Format;

  char  JID[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz";

  mhost_t *S;
 
  #define MAX_MJOBINDEX 62
 
  /* FORMAT:                                             */
  /*                                                     */

  const char *FName = "MCClusterShow";

  DBG(3,fUI) DPrint("%s(Buffer)\n",
    FName);

  if (UseXML == TRUE)
    {
    fprintf(stdout,"%s",
      Buffer);

    exit(0);
    }  /* END if (UseXML == TRUE) */
 
  memset(mapj,255,sizeof(mapj));
 
  Warnings[0] = '\0';

  jindex = 0;

  MaxFrame = -1;

  ptr = MUStrTok(Buffer,"\n",&TokPtr);

  /* NOTE:  determine format (temporary) */

  if (strchr(ptr,'<'))
    Format = mwpXML;
  else if (strchr(ptr,'='))
    Format = mwpAVP;
  else
    Format = mwpNONE;
 
  switch(Format)
    { 
    case mwpXML:

      /* NYI */

      return(FAILURE);

      /*NOTREACHED*/

      break;

    case mwpAVP:

      /* NYI */

      return(FAILURE);

      /*NOTREACHED*/

      break;

    default:
 
      while (ptr != NULL)
        {
        OPtr = ptr;

        ptr = MUStrTok(NULL,"\n",&TokPtr);

	/* FORMAT:  <OBJ> [DATA]... */

        if ((oindex = MUGetIndex(OPtr,MXO,TRUE,0)) == 0)
          {
          /* invalid object type */

          continue;
          }

        OPtr += strlen(MXO[oindex]) + 1;

        switch(oindex)
          {
          case mxoSys:
 
            sscanf(OPtr,"%ld %d",
              &Now,
              &MaxSlotPerFrame);

            if (MaxSlotPerFrame <= 0)
              {
              fprintf(stderr,"ERROR:  invalid frame configuration\n");

              exit(1);
              }
 
            break;
 
          case mxoFrame:
 
            sscanf(OPtr,"%64s %d %1024s",
              tmpName,
              &findex,
              tmpLine);
 
            findex = MIN(MAX_MFRAME,MAX(0,findex));

	    MaxFrame = MAX(findex,MaxFrame);

            S = &System[findex][0]; 
 
            strcpy(S->NetName[mnetEN0],tmpName);
 
            break;
 
          case mxoNode:
 
            sscanf(OPtr,"%64s %64s %d %d %d %s",
              tmpName,
              tmpState,
              &findex,
              &sindex,
              &SlotsUsed,
              tmpLine);
 
            findex = MIN(MAX_MFRAME,MAX(0,findex));
            sindex = MIN(MAX_MSLOTPERFRAME,MAX(1,sindex));

            S = &System[findex][sindex];
 
            strcpy(S->NetName[mnetEN0],tmpName);
            S->SlotsUsed = SlotsUsed; 
 
            S->State = MUGetIndex(tmpState,MNodeState,0,0);

	    /* NOTE:  temporary:  failure is now string */

            S->Failures = strtol(tmpLine,NULL,0);
 
            System[findex][0].SlotsUsed = 
              MAX(System[findex][0].SlotsUsed,sindex + SlotsUsed - 1);
 
            MaxSlotPerFrame = MAX(MaxSlotPerFrame,System[findex][0].SlotsUsed);
 
            break;
 
          case mxoJob:
 
            sscanf(OPtr,"%s %d %d %ld %s %s %ld %s %s",
              JName[jindex],
              &JPCount[jindex],
              &JNCount[jindex],
              &JDuration[jindex],
              JUName[jindex],
              JGName[jindex],
              &JStartTime[jindex],  /* if job is idle and start time set, value is reserve time */
              JState[jindex],
              tmpNLString);
 
            DBG(4,fUI) DPrint("INFO:     loaded job '%s' (D: %6ld  S: %8s) with %d/%d procs/nodes\n",
              JName[jindex],
              JDuration[jindex],
              JState[jindex],
              JPCount[jindex],
              JNCount[jindex]);
 
            FPtr = MUStrTok(tmpNLString," \t\n;:",&TokPtr2);
 
            while (FPtr != NULL)
              {
              int FVal;
              int SVal;

              FVal = (int)strtol(FPtr,NULL,0);

              if ((SPtr = MUStrTok(NULL," \t\n;:",&TokPtr2)) != NULL)
                SVal = (int)strtol(SPtr,NULL,0);
              else
                SVal = 0;
 
              mapj[FVal][SVal] = jindex;
 
              FPtr = MUStrTok(NULL," \t\n;:",&TokPtr2); 
              }  /* END while (FPtr != NULL) */
 
            jindex++;
 
            break;
 
          default:

            /* NO-OP */

            break;
          }  /* END switch(oindex) */
        }    /* END while (ptr != NULL) */

      break;
    }  /* END switch(Format) */
 
  DBG(2,fUI) DPrint("INFO:     job processing completed\n");
 
  /* display state */
 
  fprintf(stdout,"cluster state summary for %s\n\n",
    MULToDString(&Now));
 
  fprintf(stdout,"    %-18s %1s %9s %8s %5s %11s  %19s\n",
    "JobName",
    "S",
    "User",
    "Group",
    "Procs",
    "Remaining",
    "StartTime");
 
  fprintf(stdout,"    %18.18s %1.1s %9.9s %8.8s %5.5s %11.11s  %19.19s\n\n",
    "------------------------------",
    "------------------------------",
    "------------------------------",
    "------------------------------",
    "------------------------------",
    "------------------------------",
    "------------------------------");
 
  TotalNodes = 0;
 
  for (index = 0;index < jindex;index++) 
    {
    fprintf(stdout,"(%c) %-18s %1c %9s %8s %5d %11s  %19s",
      JID[index % MAX_MJOBINDEX],
      JName[index],
      JState[index][0],
      JUName[index],
      JGName[index],
      JPCount[index],
      MULToTString(JStartTime[index] + JDuration[index] - Now),
      MULToDString(&JStartTime[index]));
 
    TotalNodes += JNCount[index];
    }  /* END for (index) */
 
  fprintf(stdout,"\nusage summary:  %d active jobs  %d active nodes\n\n",
    jindex,
    TotalNodes);
 
  fprintf(stdout,"               %.*s\n",
    3 * MaxSlotPerFrame,
    "[0][0][0][0][0][0][0][0][0][1][1][1][1][1][1][1][1][1][1][2][2][2][2][2][2][2][2][2][2][2][3][3]");
 
  fprintf(stdout,"               %.*s\n\n",
    3 * MaxSlotPerFrame,
    "[1][2][3][4][5][6][7][8][9][0][1][2][3][4][5][6][7][8][9][0][1][2][3][4][5][6][7][8][9][0][1][2]");
 
  for (findex = 1;findex < MAX_MFRAME;findex++)
    {
    int LastSlotUsed;

    if (findex > MaxFrame)
      break;

    S = &System[findex][0];
 
    if (S->SlotsUsed == 0)
      continue;
 
    fprintf(stdout,"Frame %7.7s: ",
      S->NetName[mnetEN0]);
 
    LastSlotUsed = 0;
 
    for (sindex = 1;sindex <= MaxSlotPerFrame;sindex++)
      { 
      if (LastSlotUsed >= sindex)
        continue;

      S = &System[findex][sindex];
 
      /* add generic warnings */
 
      /* report non-homogenous memory sizes */
 
      if (S->Failures & (1 << probMemory))
        {
        sprintf(temp_str,"check memory on node %s\n",
          S->NetName[mnetEN0]);
        strcat(Warnings,temp_str);
        }
 
      if (S->Failures & (1 << probLocalDisk))
        {
        sprintf(temp_str,"%scheck local disk on node %s\n",
          Warnings,
          S->NetName[mnetEN0]);
        strcat(Warnings,temp_str);
        }
 
      if ((S->State == mnsIdle) ||
          (S->State == mnsActive) ||
          (S->State == mnsBusy))
        {
        /* report missing ethernet adapter */
 
        if (S->Failures & (1 << probEthernet))
          {
          sprintf(temp_str,"check ethernet on node %s\n",
            S->NetName[mnetEN0]);
          strcat(Warnings,temp_str);
          }
 
        /* report missing HPS_IP adapter */
 
        if (S->Failures & (1 << probHPS_IP))
          {
          sprintf(temp_str,"check HPS_IP on node %s\n",
            S->NetName[mnetEN0]);
          strcat(Warnings,temp_str);
          }
        } 
 
      /* report missing HPS_USER adapter */
 
      if ((S->Failures & (1 << probHPS_User)) &&
         (S->State == mnsIdle))
        {
        sprintf(temp_str,"check HPS_USER on node %s (node is idle)\n",
          S->NetName[mnetEN0]);
        strcat(Warnings,temp_str);
        }
 
      /* if node is associated with a job */
 
      if (mapj[findex][sindex] != 255)
        {
        switch (S->State)
          {
          case mnsBusy:
          case mnsActive:
          case mnsDraining:
 
            SChar = JID[mapj[findex][sindex] % MAX_MJOBINDEX];
 
            break;
 
          case mnsDown:
 
            SChar = '*';
 
            sprintf(temp_str,"job %s requires node %s (node is down)\n",
              JName[mapj[findex][sindex]],
              S->NetName[mnetEN0]);
            strcat(Warnings,temp_str);
 
            break;
 
          case mnsIdle:
          case mnsDrained:
          case mnsFlush:
 
            SChar = JID[mapj[findex][sindex] % MAX_MJOBINDEX]; 
 
            sprintf(temp_str,"job %s has allocated node %s (node is in state %s)\n",
              JName[mapj[findex][sindex]],
              S->NetName[mnetEN0],
              MNodeState[System[findex][sindex].State]);
            strcat(Warnings,temp_str);
 
            break;
 
          case mnsUnknown:
 
            SChar = '?';
 
            break;
 
          default:
 
            SChar = JID[mapj[findex][sindex] % MAX_MJOBINDEX];
 
            break;
          }
        }    /* END if (mapj[findex][sindex] != 255) */
      else
        {
        switch (System[findex][sindex].State)
          {
          case mnsDown:
 
            SChar = '#';
 
            sprintf(temp_str,"node %s is down\n",
              S->NetName[mnetEN0]);
            strcat(Warnings,temp_str);
 
            break;
 
          case mnsReserved:
          case mnsDraining: 
          case mnsDrained:
          case mnsFlush:
 
            SChar = '!';
 
            break;
 
          case mnsIdle:
 
            if (S->Failures & probLocalDisk)
              sprintf(temp_str,"node %s is idle and local disk space is low\n",
                S->NetName[mnetEN0]);
            strcat(Warnings,temp_str);
 
            SChar = ' ';
 
            break;
 
          case mnsUnknown:
 
            SChar = '?';
 
            break;
 
          case mnsBusy:
 
            SChar = '@';
 
            sprintf(temp_str,"node %s has no job scheduled (node is busy)\n",
              S->NetName[mnetEN0]);
            strcat(Warnings,temp_str);
 
            break;
 
          case mnsNone:
 
            SChar = '~';

            break;
 
          default:
 
            SChar = '@';

            if (S->SlotsUsed > 0)
              { 
              sprintf(temp_str,"node[%d,%d] %s has unexpected state '%s'\n",
                findex,
                sindex,
                S->NetName[mnetEN0],  
                MNodeState[S->State]);
              strcat(Warnings,temp_str);
              }
 
            break;
          }  /* END switch (S->State) */
        }    /* END else ... */
 
      /* determine next populated slot */
 
      switch(S->SlotsUsed)
        {
        case 0:

          /* node not configured */

          fprintf(stdout,"XXX");

          break;

        case 1:
 
          if (SChar != '~')
            {
            fprintf(stdout,"[%c]",
              SChar);
            }
          else
            {
            fprintf(stdout,"XXX");
            }
 
          break;
 
        case 2:
 
          if (SChar != '~')
            {
            fprintf(stdout,"[ %c  ]", 
              SChar);
            }
          else
            {
            fprintf(stdout,"XXXXXX");
            }
 
          break;
 
        case 3:
 
          if (SChar != '~')
            {
            fprintf(stdout,"[   %c   ]",
              SChar);
            }
          else
            {
            fprintf(stdout,"XXXXXXXXX");
            }
 
          break;
 
        case 4:
 
          if (SChar != '~')
            {
            fprintf(stdout,"[    %c     ]",
              SChar);
            }
          else
            {
            fprintf(stdout,"XXXXXXXXXXXX");
            }
 
          break;
 
        default:
 
          fprintf(stderr,"ERROR:    unexpected slot count (%d) for node[%d][%d] '%s'\n", 
            S->SlotsUsed,
            findex,
            sindex,
            S->NetName[mnetEN0]);
 
          break;
        }  /* END switch (S->SlotsUsed) */
 
      LastSlotUsed += MAX(1,S->SlotsUsed);
      }    /* END for (sindex) */
 
    fprintf(stdout,"\n");
    }  /* END for (findex) */
 
  fprintf(stdout,"\nKey:  [?]:Unknown [*]:Down w/Job [#]:Down [ ]:Idle [@] Busy w/No Job [!] Drained\n");
 
  if (Warnings[0] != '\0')
    {
    fprintf(stdout,"\n%s\n",
      Warnings);
    }
 
  return(SUCCESS);
  }  /* END MCClusterShow() */





int MCGridCtl(
 
  msocket_t *S)
 
  {
  const char *FName = "MCGridCtl";

  DBG(2,fUI) DPrint("%s(S)\n",
    FName);

  if (S == NULL)
    return(FAILURE);
 
  fprintf(stdout,"\n\n%s\n",
    S->RBuffer);
 
  return(SUCCESS);
  }  /* END MCGridCtl() */
 
 
 
 
int MCNodeCtl(

  msocket_t *S)

  {
  const char *FName = "MCNodeCtl";

  DBG(2,fUI) DPrint("%s(S)\n",
    FName);

  if (S == NULL)
    return(FAILURE);

  switch(S->WireProtocol)
    {
    case mwpXML:

      {
      mxml_t *E = (mxml_t *)S->SE;

      if (E->Val != NULL)
        {
        fprintf(stdout,"\n%s\n",
          E->Val);
        }
      }    /* END BLOCK */

      break;

    default:

      fprintf(stdout,"\n\n%s\n",
        S->RPtr);

      if (strstr(S->RPtr,"ERROR:") != NULL)
        return(FAILURE);

      break;
    }  /* END switch(S->WireProtocol) */

  return(SUCCESS);
  }  /* END MCNodeCtl() */
 

 

int MCBal(

  msocket_t *S)  /* I */

  {
  mxml_t *E  = NULL;
  mxml_t *RE = NULL;
  mxml_t *DE = NULL;

  const char *FName = "MCBal";

  DBG(2,fUI) DPrint("%s(S)\n",
    FName);

  if (S == NULL)
    {
    return(FAILURE);
    }

  /* extract response */

  if ((MXMLGetChild(S->SE,"Reply",NULL,&E) == FAILURE) ||
      (MXMLGetChild(E,"Response",NULL,&RE) == FAILURE) || 
      (MXMLGetChild(RE,"Data",NULL,&DE) == FAILURE))
    {
    fprintf(stderr,"ERROR:  cannot determine best host (parse error)\n");

    return(FAILURE);
    }

  if (DE->Val == NULL) 
    {
    fprintf(stderr,"ERROR:  cannot determine best host (no available host)\n");

    return(FAILURE);
    }

  if (BalMode == mccmExecute)
    {
    /* launch job on ideal node if available */

    AList[0] = strdup(MNAT_RCMD);
    AList[1] = DE->Val;

    if (AList[2] != NULL)
      {
      fprintf(stderr,"\nexecuting command '%s' on host '%s'\n\n",
        AList[2],
        AList[1]);
      }
    else
      {
      fprintf(stderr,"\nlogin to host '%s'\n\n",
        AList[1]);
      }
 
    execv(AList[0],AList);

    exit(0);
    }
  else
    {
    fprintf(stdout,"\n\n%s\n",
      DE->Val);
    }

  return(SUCCESS);
  }  /* END MCBal() */



 
int MCResCtl(
 
  msocket_t *S)  /* I */
 
  {
  const char *FName = "MCResCtl";
 
  DBG(2,fUI) DPrint("%s(S)\n",
    FName);
 
  if (S == NULL)
    {
    return(FAILURE);
    }
 
  fprintf(stdout,"\n\n%s\n",
    S->RBuffer);
 
  return(SUCCESS);
  }  /* END MCResCtl() */



 
int MCJobCtl(
 
  msocket_t *S)  /* I */
 
  {
  const char *FName = "MCJobCtl";

  DBG(2,fUI) DPrint("%s(S)\n",
    FName);

  if (S == NULL)
    {
    return(FAILURE);
    }

  switch(S->WireProtocol)
    {
    case mwpXML:

      {
      mxml_t *E = (mxml_t *)S->SE;

      /* NOTE:  if query, must display job results */

      if (E->Val != NULL)
        {
        fprintf(stdout,"\n%s\n",
          E->Val);
        }
      
      if (E->CCount > 0)
        {
        int cindex;

        mxml_t *C;
        mxml_t *GC;

        for (cindex = 0;cindex < E->CCount;cindex++)
          {
          C = E->C[cindex];

          fprintf(stdout,"%s %s",
            C->Name,
            C->AVal[0]);

          if (C->CCount > 0)
            {
            GC = C->C[0];

            fprintf(stdout," %s",
              GC->Val);
            }

          fprintf(stdout,"\n");
          }    /* END for (cindex) */
        }      /* END if (E->CCount > 0) */
      }        /* END BLOCK */

      break;

    default:

      fprintf(stdout,"\n\n%s\n",
        S->RPtr);

      if (strstr(S->RPtr,"ERROR:") != NULL)
        {
        return(FAILURE);
        }

      break;
    }  /* END switch(S->WireProtocol) */ 
 
  return(SUCCESS);
  }  /* END MCJobCtl() */




int MCShow(

  msocket_t *S)  /* I */

  {
  const char *FName = "MCShow";

  char OString[MAX_MLINE];

  int  OIndex;

  DBG(2,fUI) DPrint("%s(S)\n",
    FName);

  if (S == NULL)
    {
    return(FAILURE);
    }

  switch(S->WireProtocol)
    {
    case mwpXML:

      {
      mxml_t *E = (mxml_t *)S->SE;
      mxml_t *C = NULL;
      mxml_t *RE;

      int      CTok = -1;

      /* FORMAT:  <Reply><Response>...</Response></Reply> */

      if (SchedMode == -2)
        {
        char tmpBuffer[MAX_MBUFFER];

        MXMLToString(E,tmpBuffer,MAX_MBUFFER,NULL,TRUE);

        fprintf(stdout,"\n%s\n",
          tmpBuffer);

        return(SUCCESS);
        }

      /* extract response */
   
      if (MXMLGetChild(E,"Response",NULL,&RE) == FAILURE)
        {
        fprintf(stderr,"ERROR:  invalid response\n");

        return(FAILURE);
        }
 
      /* load messages */

      while (MXMLGetChild(RE,"Message",&CTok,&C) == SUCCESS)
        {
        /* display message */

        if (C->Val != NULL)
          {
          fprintf(stdout,"\n%s\n",
            C->Val);
          }
        }  /* END while (MXMLGetE(RE,"Message",&CTok,&C) == SUCCESS) */

      if (MXMLGetAttr(RE,"object",NULL,OString,sizeof(OString)) == FAILURE) 
        {
        fprintf(stderr,"ERROR:  invalid response\n");

        return(FAILURE);
        }

      OIndex = MUGetIndex(OString,MXO,FALSE,mxoNONE);

      switch(OIndex)
        {
        case mxoCluster:

          /* NYI */

          break;

        case mxoQueue:

          MCQueueShow(RE);

          break;

        default:

          /* NO-OP */

          break;
        }  /* END switch(OIndex) */
      }    /* END BLOCK */

      break;

    default:

      fprintf(stdout,"\n\n%s\n",
        S->RPtr);

      if (strstr(S->RPtr,"ERROR:") != NULL)
        return(FAILURE);

      break;
    }  /* END switch(S->WireProtocol) */

  return(SUCCESS);
  }  /* END MCShow() */




int MCQueueShow(

  mxml_t *E)  /* I */

  {
  mxml_t *CE;
  mxml_t *QE;
  mxml_t *JE;

  int UpProcs = 0;
  int UpNodes = 0;

  int ActiveProcs = 0;
  int ActiveNodes = 0;

  long SchedTime = 0;

  char tmpString[MAX_MNAME];

  char QType[MAX_MNAME];

  char JID[MAX_MNAME];
  char UName[MAX_MNAME];
  char JState[MAX_MNAME];
  char QName[MAX_MNAME];
  
  long StartTime;
  long Duration;

  int  Procs = 0;

  int  CTok = -1;
  int  CTok2;

  int  JCount;
  int  QJCount;

  if (E == NULL)
    return(FAILURE);

  /* load cluster config */

  if (MXMLGetChild(E,(char *)MXO[mxoCluster],&CTok,&CE) == FAILURE)
    {
    return(FAILURE);
    }

  if (MXMLGetAttr(CE,"time",NULL,tmpString,sizeof(tmpString)) == SUCCESS)
    SchedTime = strtol(tmpString,NULL,0);

  if (MXMLGetAttr(CE,"upProcs",NULL,tmpString,sizeof(tmpString)) == SUCCESS)
    UpProcs = (int)strtol(tmpString,NULL,0);

  if (MXMLGetAttr(CE,"upNodes",NULL,tmpString,sizeof(tmpString)) == SUCCESS)
    UpNodes = (int)strtol(tmpString,NULL,0);

  if (MXMLGetAttr(CE,"allocProcs",NULL,tmpString,sizeof(tmpString)) == SUCCESS)
    ActiveProcs = (int)strtol(tmpString,NULL,0);

  if (MXMLGetAttr(CE,"activeNodes",NULL,tmpString,sizeof(tmpString)) == SUCCESS)
    ActiveNodes = (int)strtol(tmpString,NULL,0);

  /* load queue info */

  JCount = 0;

  while (MXMLGetChild(E,(char *)MXO[mxoQueue],&CTok,&QE) != FAILURE)
    {
    if (MXMLGetAttr(QE,"type",NULL,QType,sizeof(QType)) == FAILURE)
      continue;

    /* display queue header */

    fprintf(stdout,"\n%s jobs%.*s\n",
      QType,
      (int)(30 - strlen(QType)),
      "----------------------------");

    if (!strcmp(QType,"active"))
      {
      fprintf(stdout,"%-18s %8s %10s %5s %11s %20s\n\n",
        "JOBNAME",
        "USERNAME",
        "STATE",
        "PROC",
        "REMAINING",
        "STARTTIME");
      }
    else
      {
      fprintf(stdout,"%-18s %8s %10s %5s %11s %20s\n\n",
        "JOBNAME",
        "USERNAME",
        "STATE",
        "PROC",
        "WCLIMIT",
        "QUEUETIME");
      }

    CTok2 = -1;
    QJCount = 0;

    while (MXMLGetChild(QE,(char *)MXO[mxoJob],&CTok2,&JE) != FAILURE)
      {
      if (MXMLGetAttr(JE,(char *)MJobAttr[mjaJobName],NULL,JID,sizeof(JID)) == FAILURE)
        continue;

      if (MXMLGetAttr(JE,(char *)MJobAttr[mjaState],NULL,JState,sizeof(JState)) == FAILURE)
        continue;

      if (MXMLGetAttr(JE,(char *)MJobAttr[mjaUser],NULL,UName,sizeof(UName)) == FAILURE)
        continue;

      if (MXMLGetAttr(JE,(char *)MJobAttr[mjaQOSReq],NULL,QName,sizeof(QName)) == FAILURE)
        strcpy(QName,"-");

      if (MXMLGetAttr(JE,(char *)MJobAttr[mjaStartTime],NULL,tmpString,sizeof(tmpString)) == SUCCESS)
        StartTime = strtol(tmpString,NULL,0);
      else
        StartTime = 0;

      if (MXMLGetAttr(JE,(char *)MJobAttr[mjaReqAWDuration],NULL,tmpString,sizeof(tmpString)) == SUCCESS)
        Duration = strtol(tmpString,NULL,0);
      else
        Duration = 0;

      if (MXMLGetAttr(JE,(char *)MJobAttr[mjaReqProcs],NULL,tmpString,sizeof(tmpString)) == SUCCESS)
        Procs = (int)strtol(tmpString,NULL,0);
      else
        Duration = 1;

      JCount++;
      QJCount++;

      if (!strcmp(QType,"active"))
        {
        fprintf(stdout,"%-18s %8s %10s %5d %11s  %19s",
          JID,
          UName,
          JState,
          Procs,
          MULToTString(Duration - (SchedTime - StartTime)),
          MULToDString(&StartTime));
        }
      else 
        {
        fprintf(stdout,"%-18s %8s %10s %5d %11s  %19s",
          JID,
          UName,
          JState,
          Procs,
          MULToTString(Duration),
          MULToDString(&StartTime));
        }
      }    /* END while (MXMLGetChild() != FAILURE) */

    /* display queue footer */

    sprintf(tmpString,"%d %s job%c   ",
      QJCount,
      QType,
      (QJCount == 1) ? ' ' : 's');

    if (!strcmp(QType,"active"))
      {
      fprintf(stdout,"\n%21s %4d of %4d Processors Active (%.2f%c)\n",
        tmpString,
        ActiveProcs,
        UpProcs,
        (UpProcs > 0) ? (double)ActiveProcs / UpProcs * 100.0 : 0.0,
        '%');

      if (UpProcs != UpNodes)
        {
        fprintf(stdout,"%21s %4d of %4d Nodes Active      (%.2f%c)\n",
          " ",
          ActiveNodes,
          UpNodes,
          (UpNodes > 0) ? (double)ActiveNodes / UpNodes * 100.0 : 0.0,
          '%');
        }
      }
    else
      {
      fprintf(stdout,"\n%s\n",
        tmpString);
      }
    }  /* END while (g2XMLGetChild() != FAILURE) */

  fprintf(stdout,"\nTotal job%s:  %d\n\n",
    (JCount == 1) ? "" : "s",
    JCount);

  return(SUCCESS);
  }  /* END MCQueueShow() */




int MCMigrateJob(
 
  char *Buffer)
 
  {
  const char *FName = "MCMigrateJob";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);
 
  fprintf(stdout,"\n\n%s\n",
    Buffer);
 
  return(SUCCESS);
  }  /* END MCMigrateJob() */
 
 
 
 
 
int MCBShowState(
 
  char *Buffer)  /* I */
 
  {
  const char *FName = "MCBShowState";

  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);
 
  fprintf(stdout,"\n\n%s\n",
    Buffer);
 
  return(SUCCESS);
  }  /* END MCBShowState() */




int MCShowStats(

  char *Buf,     /* I */
  int   BufSize)

  {
  int rc;

  const char *FName = "MCShowStats";

  rc = FAILURE;

  DBG(3,fUI) DPrint("%s(Buf,%d)\n",
    FName,
    BufSize);
 
  switch(ObjectType)
    {
    case mxoUser:
    case mxoGroup:
    case mxoAcct:
    case mxoQOS:
    case mxoClass:

      rc = MCShowCStats(Buf,BufSize,ObjectType);

      break;

    case mxoNode:

      rc = MCShowNodeStats(Buf);
 
      break;
 
    case mxoSched:
 
      rc = MCShowSchedulerStatistics(Buf);
 
      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(ObjectType) */

  return(rc);
  }  /* END MCShowStats() */





int MCShowCStats(
 
  char *Buffer,  /* I */
  int   BufSize, /* I */
  int   OIndex)  /* I */
 
  {
  int    OCount;
 
  double JCPCT;
  double PHRPCT;
  double PHDPCT;
  double PHUPCT;
 
  char  FSTarget[MAX_WORD];
  char  Effic[MAX_WORD];
 
  char  MaxXF[MAX_WORD]; 
  char  XF[MAX_WORD];
  char  QT[MAX_WORD];
  char  JA[MAX_WORD];
 
  char  JCount[MAX_WORD];
  char  JCountP[MAX_WORD];
  char  PHRQ[MAX_WORD];
  char  PHRQP[MAX_WORD];
  char  PHUtl[MAX_WORD];
  char  PHUtlP[MAX_WORD];
  char  PHDed[MAX_WORD];
  char  PHDedP[MAX_WORD];
 
  char *tail;
 
  mxml_t *E = NULL;
 
  mpar_t    tmpGP;

  mgcred_t  *tmpUP[MAX_MUSER]; 

  mgcred_t *tmpU = NULL;
  mgcred_t *tmpG = NULL;
  mgcred_t *tmpA = NULL;
  mqos_t   *tmpQ = NULL;
  mclass_t *tmpC = NULL;

  char     *NameP;

  void     *O;      
  void     *OP;

  void     *OE;
  int       OS;

  mcredl_t *L;
  mfs_t    *F;
  must_t   *S;
  must_t   *GS;
 
  mpar_t   *GP;
 
  const char *FName = "MCShowCStats";
 
  DBG(2,fUI) DPrint("%s(Buffer,%d,%s)\n",
    FName,
    BufSize,
    MXO[OIndex]); 

  GP = &tmpGP;
 
  memset(GP,0,sizeof(mpar_t));

  MSched.GP = GP;        
 
  if ((Buffer[0] == '\0') ||
      (MXMLFromString(&E,Buffer,&tail,NULL) == FAILURE) ||
      (MOFromXML((void *)&MSched,mxoSys,E) == FAILURE))
    {
    fprintf(stdout,"no %s statistics available\n",
      MXO[OIndex]);
 
    return(SUCCESS);
    }

  MStatFromXML((void *)&GP->S,E);

  MXMLDestroyE(&E);

  OCount = 0;
 
  /* extract cred stats */
 
  while (MXMLFromString(&E,tail,&tail,NULL) != FAILURE)
    {
    switch(OIndex)
      {
      case mxoUser:

        if (OCount == 0)     
          tmpU = (mgcred_t *)calloc(MAX_MUSER, sizeof(mgcred_t));
 
        O = &tmpU[OCount + 1];

        tmpUP[OCount + 1] = &tmpU[OCount + 1];    

        break;

      case mxoGroup:

        if (OCount == 0)
          tmpG = (mgcred_t *)calloc(MAX_MGROUP,sizeof(mgcred_t));

        O = &tmpG[OCount + 1];               

        break;

      case mxoAcct:

        if (OCount == 0)
          tmpA = (mgcred_t *)calloc(MAX_MACCT, sizeof(mgcred_t));

        O = &tmpA[OCount + 1];
 
        break;

      case mxoClass:

        if (OCount == 0)
          tmpC = (mclass_t *)calloc(MAX_MCLASS, sizeof(mclass_t));

        O = &tmpC[OCount + 1];
 
        break;

      case mxoQOS:

        if (OCount == 0)
          tmpQ = (mqos_t *)calloc(MAX_MQOS, sizeof(mqos_t));

        O = &tmpQ[OCount + 1];
 
        break;

      default:

        /* no object defined */

        O = NULL;

        break;
      }  /* END switch(OIndex) */

    if (MOFromXML(O,OIndex,E) == FAILURE)
      {
      continue;
      }
 
    OCount++;

    MXMLDestroyE(&E);
    }  /* END while (MXMLFromString() != FAILURE) */

  tmpUP[OCount + 1] = NULL;             
 
  fprintf(stdout,"statistics initialized %s\n",
    MULToDString(&MStat.InitTime)); 
 
  fprintf(stdout,"         |------ Active ------|--------------------------------- Completed -----------------------------------|\n");
 
  /*               NAM JOB NOD PH  JOB PCT PHR PCT HRD PCT FST AXF MXF AQH EFF WCA */
  fprintf(stdout,"%-9s %4s %5s %9s %4s %6s %6s %6s %6s %6s %5s %6s %6s %6s %6s %6s\n",
    MXO[OIndex],
    "Jobs",
    "Procs",
    "ProcHours",
    "Jobs",
    "  %  ",
    "PHReq",
    "  %  ",
    "PHDed",
    "  %  ",
    "FSTgt",
    "AvgXF",
    "MaxXF",
    "AvgQH",
    "Effic",
    "WCAcc");
 
  /* sort by dedicated PS */
 
  DBG(5,fUI) DPrint("INFO:     sorting %d %s\n",
    OCount,
    MXO[OIndex]);

  GS = &GP->S;  

  OS = 0;

  switch(OIndex)
    {
    case mxoUser:
 
      qsort((void *)&tmpU[1],OCount,sizeof(mgcred_t),(int(*)())MCUPSDedicatedComp);

      OP = &tmpUP[0];
      OE = (void *)&tmpUP[OCount + 1];

      break;

    case mxoGroup:

      qsort((void *)&tmpG[1],OCount,sizeof(mgcred_t),(int(*)())MCGPSDedicatedComp);
 
      OP = &tmpG[0];
      OE = (void *)&tmpG[OCount + 1];

      break;

    case mxoAcct:

      qsort((void *)&tmpA[1],OCount,sizeof(mgcred_t),(int(*)())MCAPSDedicatedComp);
 
      OP = &tmpA[0];
      OE = (void *)&tmpA[OCount + 1];

      break;

    case mxoQOS:

     qsort((void *)&tmpQ[1],OCount,sizeof(mqos_t),(int(*)())MCQPSDedicatedComp);
 
      OP = &tmpQ[0];
      OE = (void *)&tmpQ[OCount + 1];

      break;

    case mxoClass:

     qsort((void *)&tmpC[1],OCount,sizeof(mclass_t),(int(*)())MCCPSDedicatedComp);
 
      OP = &tmpC[0];
      OE = (void *)&tmpC[OCount + 1];

      break;

    default:

      return(FAILURE);

      /*NOTRACHED*/

      break;
    }  /* END switch(OIndex) */
 
  while ((O = MOGetNextObject(&OP,OIndex,OS,OE,&NameP)) != NULL)
    { 
    MOGetComponent(O,OIndex,(void *)&S,mxoStats);
    MOGetComponent(O,OIndex,(void *)&F,mxoFS);     
    MOGetComponent(O,OIndex,(void *)&L,mxoLimits);     

    strcpy(Effic,"------");
    strcpy(FSTarget,"-----");
    strcpy(MaxXF,"------");
    strcpy(XF,"------"); 
    strcpy(XF,"------");
    strcpy(QT,"------");
    strcpy(JA,"------");
    strcpy(JCount,"----");
    strcpy(JCountP,"------");
    strcpy(PHRQ,"------");
    strcpy(PHRQP,"------");
    strcpy(PHUtl,"------");
    strcpy(PHUtlP,"------");
    strcpy(PHDed,"------");
    strcpy(PHDedP,"------");
 
    JCPCT  = (GS->Count > 0)         ? ((double)S->Count         / GS->Count * 100.0) : 0.0;
    PHRPCT = (GS->PSRequest > 0)     ? ((double)S->PSRequest     / GS->PSRequest * 100.0) : 0.0;
    PHDPCT = (GS->PSDedicated > 0.0) ? ((double)S->PSDedicated   / GS->PSDedicated * 100.0) : 0.0;
    PHUPCT = (GS->PSUtilized > 0.0)  ? ((double)S->PSUtilized    / GS->PSUtilized * 100.0) : 0.0;
 
    if (F->FSTarget > 0.0)
      {
      sprintf(FSTarget,"%5.5s",
        MFSTargetToString(F->FSTarget,F->FSMode));
      }
 
    if (S->PSDedicated > 0.0)
      {
      sprintf(PHUtl,"%6.1f",(double)S->PSUtilized / 3600.0);
      sprintf(PHDed,"%6.1f",(double)S->PSDedicated / 3600.0);
 
      sprintf(PHUtlP,"%6.2f",PHUPCT);
      sprintf(PHDedP,"%6.2f",PHDPCT);
 
      sprintf(Effic,"%6.2f",((double)S->PSUtilized / S->PSDedicated * 100.0));
      }
 
    if (S->Count > 0)
      {
      sprintf(JCount,"%4d",S->Count);
      sprintf(JCountP,"%6.2f",JCPCT);
 
      sprintf(PHRQ,"%6.1f",(double)S->PSRequest / 3600);
      sprintf(PHRQP,"%6.2f",PHRPCT); 
 
      sprintf(MaxXF,"%6.2f",((double)S->MaxXFactor));
      sprintf(XF,"%6.2f",MIN(999.99,((double)S->XFactor / S->Count)));
      sprintf(QT,"%6.2f",((double)S->TotalQTS / S->Count / 3600));
      sprintf(JA,"%6.2f", (S->JobAcc          / S->Count * 100.0));
      }
 
    if ((L->AP.Usage[mptMaxJob][0] == 0) &&
        (S->PSDedicated == 0.0) &&
       !(Flags & (1 << mcmVerbose)))
      {
      DBG(3,fUI) DPrint("INFO:     ignoring statistics for '%s'\n",
        NameP);
 
      continue;
      }
 
    /*               NAM  ID RJB RND RUNPH JCT JCPCT RPH RQPCT DEDPH DEPCT TARGT AVGXF MAXXF AVGQT EFFIC AVGJA */
 
    fprintf(stdout,"%-9s %4d %5d %9.2f %4s %6s %5s %6s %6s %6s %5s %6s %6s %6s %6s %6s\n",
      NameP,
      L->AP.Usage[mptMaxJob][0],
      L->AP.Usage[mptMaxProc][0],
      (double)L->AP.Usage[mptMaxPS][0] / 3600.0,
      JCount,
      JCountP,
      PHRQ,
      PHRQP,
      PHDed,
      PHDedP,
      FSTarget,
      XF,
      MaxXF,
      QT,
      Effic,
      JA);
    }  /* END while (MOGetNextObject() != NULL) */
 
  fprintf(stdout,"\n\n"); 
 
  return(SUCCESS);
  }  /* END MCShowCStats() */




int MCShowEStart(
 
  char *Buf)  /* I */
 
  {
  char   Name[MAX_MNAME];
 
  long   Now;
  long   Deadline;
  long   StartTime;
  long   WCLimit;

  int    PC;
 
  char   PName[MAX_MNAME];
 
  const char *FName = "MCShowEStart";
 
  DBG(2,fUI) DPrint("%s(Buffer)\n",
    FName);
 
  sscanf(Buf,"%s %ld %ld %ld %d %s",
    Name,
    &Now,
    &Deadline,
    &WCLimit,
    &PC,
    PName);
 
  StartTime = Deadline - WCLimit;
 
  fprintf(stdout,"job %s requires %d proc%s for %s\n",
    Name,
    PC, 
    (PC == 1) ? "" : "s",
    MULToTString(WCLimit));
 
  fprintf(stdout,"Earliest start in      %11s on %s",
    MULToTString(StartTime - Now),
    MULToDString(&StartTime));
 
  fprintf(stdout,"Earliest completion in %11s on %s",
    MULToTString(Deadline - Now),
    MULToDString(&Deadline));
 
  fprintf(stdout,"Best Partition: %s\n\n",
    PName);
 
  return(SUCCESS);
  }  /* END MCShowEStart() */




int __MCLoadArgs(
 
  int    ArgC,    /* I */
  char **ArgV,    /* I */
  int   *CIndex)  /* O */
 
  {
  char  *ptr;
  char  *TokPtr;
 
  char  *Value;
 
  int    Flag;
  char   tmpLine[MAX_MLINE];
 
  char  *Args[MAX_MCARGS];
  int    ArgCount;
 
  int    index;

  int    OptTok; 
  char  *OptArg;
 
  char   OptString[MAX_MLINE];

  const char *MCService[] = {
    "mdiag",
    "mjobctl",
    "mstat",
    "mauictl",
    "mshow",
    NULL };
 
  if ((ArgV == NULL) || (CIndex == NULL))
    {
    return(FAILURE);
    }
 
  /* determine command name */
 
  if ((ptr = strrchr(ArgV[0],'/')) != NULL) 
    MUStrCpy(tmpLine,(ptr + 1),sizeof(tmpLine));
  else
    MUStrCpy(tmpLine,ArgV[0],sizeof(tmpLine));

  if (MUGetIndex(tmpLine,MCService,FALSE,0) != 0)
    {
    *CIndex = MUGetIndex(tmpLine,MService,FALSE,0);  

    DBG(1,fCORE) DPrint("INFO:     requesting command '%s'\n",
      MService[*CIndex]);
    }
  else
    {
    if (!strcmp(tmpLine,"mdiag"))
      *CIndex = svcDiagnose;
    else if (!strcmp(tmpLine,"mstat"))
      *CIndex = svcShowStats;
    else
      *CIndex = MUGetIndex(tmpLine,MService,FALSE,0);
    }

  if (*CIndex == 0)
    {
    fprintf(stderr,"ERROR:    unknown command: '%s'\n",
      tmpLine);
 
    exit(1);
    }

  DBG(1,fCORE) DPrint("INFO:     requesting command '%s'\n",
    MService[*CIndex]);
 
  /* load arg array */
 
  for (index = 0;index <= ArgC;index++)
    {
    Args[index] = ArgV[index];
    }
 
  ArgCount = ArgC;

  switch(*CIndex)
    {
    case svcMJobCtl:

      MCJobCtlCreateRequest(Args,ArgCount,MsgBuffer);

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case svcMResCtl:

      MCResCtlCreateRequest(Args,ArgCount,MsgBuffer);

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case svcMShow:

      MCShowCreateRequest(Args,ArgCount,MsgBuffer);

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case svcMBal:

      MCBalCreateRequest(Args,ArgCount,MsgBuffer);

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(*CIndex) */
 
  __MCGetArgList(*CIndex,OptString);

  Flag = 0;

  OptTok = 1;

  Value = NULL;

  while ((Flag = MUGetOpt(&ArgCount,Args,OptString,&OptArg,&OptTok)) != -1)
    {
    DBG(6,fCONFIG) DPrint("INFO:     processing flag '%c'\n", 
      (char)Flag);

    TokPtr = NULL;
 
    switch ((char)Flag)
      {
      case '-':

        DBG(6,fCONFIG) DPrint("INFO:     received '-' (%s)\n",
          OptArg);

        if ((ptr = MUStrTok(OptArg,"=",&TokPtr)) != NULL)
          {
          Value = MUStrTok(NULL,"=",&TokPtr);
          }

        if (!strcmp(OptArg,"about"))
          {
          fprintf(stderr,"%s client version %s\n",
            MSCHED_SNAME,
            MSCHED_VERSION);

          /* NOTE:  specify:  default configfile, serverhost, serverport */

          fprintf(stderr,"defaults:  server=%s:%d  homedir=%s\n",
            C.ServerHost,
            C.ServerPort,
            C.HomeDir);

          /* NOTE:  specify build location, build date, build host       */

          fprintf(stderr,"build dir:  %s\nbuild host: %s\nbuild date: %s\n",
            C.BuildDir,
            C.BuildHost,
            C.BuildDate);
 
          exit(0);
          }
        else if (!strcmp(OptArg,"configfile"))
          {
          if (Value == NULL)
            {
            MCShowUsage(*CIndex);
 
            exit(1);
            }

          MUStrCpy(C.ConfigFile,OptArg,sizeof(C.ConfigFile));
 
          DBG(2,fCONFIG) DPrint("INFO:     configfile set to '%s'\n",
            C.ConfigFile);
          }
        else if (!strcmp(OptArg,"format"))
          {
          C.Format = mwpXML;
          }
        else if (!strcmp(OptArg,"help"))
          {
          MCShowUsage(*CIndex);
 
          exit(1);
          }
        else if ((!strcmp(OptArg,"host")) && (Value != NULL))
          {
          MUStrCpy(C.ServerHost,Value,sizeof(MSched.ServerHost));
 
          DBG(3,fCONFIG) DPrint("INFO:     server host set to %s\n",
            C.ServerHost);
          }
        else if (!strcmp(OptArg,"keyfile"))
          {
          FILE *fp;

          char tmpKey[MAX_MLINE];

          if (Value == NULL)
            {
            MCShowUsage(*CIndex);
 
            exit(1);
            }

          if ((fp = fopen(Value,"r")) == NULL)
            {
            fprintf(stderr,"ERROR:    cannot open keyfile '%s' errno: %d, (%s)\n",
              Value,
              errno,
              strerror(errno));
          
            exit(1);
            }

          fscanf(fp,"%20s",
            tmpKey);

          strncpy(C.ServerCSKey,tmpKey,MAX_MNAME);

          fclose(fp);
          }
        else if (!strcmp(OptArg,"loglevel"))
          {
          if (Value == NULL)
            {
            MCShowUsage(*CIndex);
   
            exit(1);
            }
 
          mlog.Threshold = (int)strtol(Value,NULL,0);
 
          DBG(2,fCONFIG) DPrint("INFO:     LOGLEVEL set to %d\n",
            mlog.Threshold);
          }
        else if (!strcmp(OptArg,"port"))
          {
          if (Value == NULL)
            {
            MCShowUsage(*CIndex);
 
            exit(1);
            }

          /* P-PORT */
 
          C.ServerPort = (int)strtol(Value,NULL,0);
 
          DBG(2,fCONFIG) DPrint("INFO:     port set to %d\n",
            C.ServerPort);
          }
        else if (!strcmp(OptArg,"version"))
          {
          fprintf(stderr,"%s client version %s\n",
            MSCHED_SNAME,
            MSCHED_VERSION);
 
          exit(0);
          }
 
        break;

      case 'C':

        /* COMPAT:  C-CONFIGFILE */
 
        if (OptArg == NULL)
          {
          MCShowUsage(*CIndex);
 
          exit(1);
          }
 
        MUStrCpy(C.ConfigFile,OptArg,sizeof(C.ConfigFile));
 
        DBG(2,fCONFIG) DPrint("INFO:     ConfigFile set to '%s'\n",
          C.ConfigFile);
 
        break;

      case 'D':

        /* COMPAT:  D-DEBUGLEVEL */
 
        if (OptArg == NULL)
          {
          MCShowUsage(*CIndex);
 
          exit(1);
          }
 
        mlog.Threshold = (int)strtol(OptArg,NULL,0);
 
        DBG(2,fCONFIG) DPrint("INFO:     LOGLEVEL set to %d\n",
          mlog.Threshold);
 
        break;

      case 'P':

        /* COMPAT:  P-PORT */

        if (OptArg == NULL)
          {
          MCShowUsage(*CIndex);
 
          exit(1);
          }
 
        C.ServerPort = (int)strtol(OptArg,NULL,0);
 
        DBG(2,fCONFIG) DPrint("INFO:     port set to %d\n",
          C.ServerPort);
 
        break;

      case '?':
 
        MCShowUsage(*CIndex);
 
        exit(1);
 
        break;

      default:

        /* check compat flags */

        __OMCLoadArgs((char)Flag,OptArg,CIndex);

        break;
      }  /* END switch((char)Flag) */
    }    /* END while ((Flag = MUGetOpt()) != -1) */

  DBG(5,fCONFIG)
    {
    DBG(5,fCONFIG) DPrint("INFO:     flags loaded\n");
 
    DBG(5,fCONFIG) DPrint("INFO:     %d command line args remaining: ",
      ArgCount);
 
    for (index = 0;Args[index] != NULL;index++)
      {
      fprintf(mlog.logfp," '%s'",
        Args[index]);
      }  /* END for (index) */
 
    fprintf(mlog.logfp,"\n");
    }

  __OMCProcessArgs(ArgCount,Args,CIndex);

  return(SUCCESS);
  }    /* END __MCLoadArgs() */



int MCJobSubmitCreateRequest()

  {  
  return(SUCCESS); 
  }  /* END MCJobSubmitCreateRequest() */





int MCProcessGeneralArgs(

  int       CIndex,
  char    **ArgV,
  int      *ArgC,
  mxml_t  *E)
 
  {
  char *OptString = "-:f:v:?";

  char *OptArg;
  int   OptTok;

  char *ptr;
  char *TokPtr;

  char *Value = NULL;

  int Flag;

  if ((ArgV == NULL) || (E == NULL))
    {
    return(FAILURE);
    }
 
  /* process '-v' verbose */
  /* process '-f' format  */

  /* extract global flags */

  OptTok = 1;

  while ((Flag = MUGetOpt(ArgC,ArgV,OptString,&OptArg,&OptTok)) != -1)
    {
    DBG(6,fCONFIG) DPrint("INFO:     processing flag '%c'\n",
      (char)Flag);

    switch ((char)Flag)
      {
      case '-':  

        DBG(6,fCONFIG) DPrint("INFO:     received '-' (%s)\n",
          OptArg);

        if ((ptr = MUStrTok(OptArg,"=",&TokPtr)) != NULL)
          {
          Value = MUStrTok(NULL,"=",&TokPtr);
          }

        if (!strcmp(OptArg,"configfile"))
          {
          if (Value == NULL)
            {
            MCShowUsage(CIndex);

            exit(1);
            }

          MUStrCpy(C.ConfigFile,OptArg,sizeof(C.ConfigFile));

          DBG(2,fCONFIG) DPrint("INFO:     configfile set to '%s'\n",
            C.ConfigFile);
          }
        else if (!strcmp(OptArg,"help"))
          {
          MCShowUsage(CIndex);

          exit(1);
          }
        else if ((!strcmp(OptArg,"host")) && (Value != NULL))
          {
          MUStrCpy(C.ServerHost,Value,sizeof(MSched.ServerHost));

          DBG(3,fCONFIG) DPrint("INFO:     server host set to %s\n",
            C.ServerHost);
          }
        else if (!strcmp(OptArg,"keyfile"))
          {
          FILE *fp;

          char tmpKey[MAX_MLINE];

          if (Value == NULL)
            {
            MCShowUsage(CIndex);

            exit(1);
            }

          if ((fp = fopen(Value,"r")) == NULL)
            {
            fprintf(stderr,"ERROR:    cannot open keyfile '%s' errno: %d, (%s)\n",
              Value,
              errno,
              strerror(errno));

            exit(1);
            }

          fscanf(fp,"%20s",
            tmpKey);

          strncpy(C.ServerCSKey,tmpKey,MAX_MNAME);

          fclose(fp);
          }
        else if (!strcmp(OptArg,"loglevel"))
          {
          if (Value == NULL)
            {
            MCShowUsage(CIndex);

            exit(1);
            }

          mlog.Threshold = (int)strtol(Value,NULL,0);

          DBG(2,fCONFIG) DPrint("INFO:     LOGLEVEL set to %d\n",
            mlog.Threshold);
          }
        else if (!strcmp(OptArg,"port"))
          {
          if (Value == NULL)
            {
            MCShowUsage(CIndex);

            exit(1);
            }

          /* P-PORT */

          C.ServerPort = (int)strtol(Value,NULL,0);

          DBG(2,fCONFIG) DPrint("INFO:     port set to %d\n",
            C.ServerPort);
          }
        else if (!strcmp(OptArg,"version"))
          {
          fprintf(stderr,"%s client version %s\n",
            MSCHED_SNAME,
            MSCHED_VERSION);

          exit(1);
          }

        break;

      case 'f':

	/* log facility list */

        /* NYI */

        break;

      case 'v':

	/* version */

        /* NYI */

        break;
      }  /* END switch((char)Flag) */
    }    /* END while ((Flag = MUGetOpt()) != -1) */
 
  return(SUCCESS);
  }  /* END MCProcessGeneralArgs() */




int MCShowCreateRequest(

  char **ArgV,
  int    ArgC,
  char  *Request)

  {
  int   Flag;

  mxml_t *E = NULL;
  mxml_t *RE = NULL;

  char  OptString[MAX_MLINE];
  char *OptArg;

  int   ArgCount;
  int   OptTok;

  /* FORMAT */

  /* j-JOB q-QUEUE */

  strcpy(OptString,"fj:qn:Rx:");

  /* NOTE:  global command modifiers should be checked and incorporated  (NYI) */
  /*  ie, verbose, format, etc */

  MXMLCreateE(&E,"Message");
  MXMLCreateE(&RE,"Request");

  MXMLSetAttr(RE,"action","query",mdfString);

  MXMLAddE(E,RE);

  ArgCount = ArgC;

  MCProcessGeneralArgs(svcMShow,ArgV,&ArgCount,E);

  OptTok = 1;

  while ((Flag = MUGetOpt(&ArgCount,ArgV,OptString,&OptArg,&OptTok)) != -1)
    {
    DBG(6,fCONFIG) DPrint("INFO:     processing flag '%c'\n",
      (char)Flag);

    switch ((char)Flag)
      {
      case 'j':  /* job <JOBID> */

        /* FORMAT:  -j */

        MXMLSetAttr(RE,"object",(void *)MXO[mxoJob],mdfString);

        break;

      case 'q':  /* queue */

        /* FORMAT:  -q */

        MXMLSetAttr(RE,"object",(void *)MXO[mxoQueue],mdfString);

        break;

      case 'x':  /* direct XML */

        strcpy(Request,OptArg);

        MXMLDestroyE(&E);

        SchedMode = -2;

        return(SUCCESS);

        /*NOTREACHED*/

        break;
     
      case '?':

        MCShowUsage(svcMShow);

        exit(1);

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch (Flag) */
    }    /* END while (Flag = MUGetOpt() != -1) */

  if (MXMLGetAttr(RE,"object",NULL,NULL,0) == FAILURE)
    {
    /* default to queue */

    MXMLSetAttr(RE,"object",(void *)MXO[mxoQueue],mdfString);
    }  /* END if (MXMLGetAttr(E,"object",NULL,NULL,0) == FAILURE) */

  MXMLToString(E,Request,MAX_MLINE,NULL,TRUE);

  MXMLDestroyE(&E);

  return(SUCCESS);
  }  /* END MCShowCreateRequest() */




int MCJobCtlCreateRequest(
  
  char **ArgV,     /* I */
  int    ArgC,     /* I */
  char  *Request)  /* O */

  {
  int   Flag;

  mxml_t *E = NULL;

  char  OptString[MAX_MLINE];
  char *OptArg;

  int   ArgCount;
  int   OptTok;

  /* FORMAT */

  /* c-CANCEL C-CHECKPOINT h-HOLD m-MODIFY p-PRIORITY q-QUERY r-RESUME s-SUSPEND S-SUBMIT */
 
  strcpy(OptString,"cCfh:m:n:p:q:rRsS");
 
  /* NOTE:  global command modifiers should be checked and incorporated  (NYI) */
  /*  ie, verbose, format, etc */

  MXMLCreateE(&E,"schedrequest");

  ArgCount = ArgC;

  MCProcessGeneralArgs(svcMJobCtl,ArgV,&ArgCount,E);

  OptTok = 1;

  while ((Flag = MUGetOpt(&ArgCount,ArgV,OptString,&OptArg,&OptTok)) != -1)
    {
    DBG(6,fCONFIG) DPrint("INFO:     processing flag '%c'\n",
      (char)Flag);
 
    switch ((char)Flag)
      {
      case 'a':  /* arg */

	/* NO-OP */
 
        break;

      case 'c':  /* cancel */

        /* FORMAT:  -c <JOBID> */
 
        MXMLSetAttr(E,"action",(void *)MJobCtlCmds[mjcmCancel],mdfString);

        break;

      case 'C':  /* checkpoint */

        /* FORMAT:  -C <JOBID> */

        MXMLSetAttr(E,"action",(void *)MJobCtlCmds[mjcmCheckpoint],mdfString);

        break;

      case 'h':  /* hold */

        /* FORMAT:  -h <HOLD>[:<HOLD>]... <JOBID> */

        MXMLSetAttr(E,"action",(void *)MJobCtlCmds[mjcmModify],mdfString);
        MXMLSetAttr(E,"attr","hold",mdfString);

        if ((OptArg == NULL) || (OptArg[0] == '\0'))
          MXMLSetAttr(E,"value","user",mdfString);
        else
          MXMLSetAttr(E,"value",OptArg,mdfString);

        break;

      case 'm':  /* modify */

        /* FORMAT:  -m <ATTR>=<VAL> <JOBID> */

        /* <ATTR> = { deadline | hold | hostlist | priority | qos | walltime } */

        MXMLSetAttr(E,"action",(void *)MJobCtlCmds[mjcmModify],mdfString);  

        {
        char *attr;
        char *val;
 
        char *TokPtr;
 
        if ((OptArg == NULL) || (strchr(OptArg,'=') == NULL))
          break;
 
        if (((attr = MUStrTok(OptArg,"= \t",&TokPtr)) == NULL) ||
            ((val = MUStrTok(NULL,"= \t",&TokPtr)) == NULL))
          break;
 
        MXMLSetAttr(E,"attr",attr,mdfString);
        MXMLSetAttr(E,"value",val,mdfString);
	MXMLSetAttr(E,"mode","set",mdfString);
        }  /* END BLOCK */

        break;

      case 'o':  /* option */

        /* FORMAT:  -o <ATTR>=<VAL> */

        {
        char *attr;
        char *val;

        char *TokPtr;

        if ((OptArg == NULL) || (strchr(OptArg,'=') == NULL))
          break;

        if (((attr = MUStrTok(OptArg,"= \t",&TokPtr)) == NULL) ||
            ((val = MUStrTok(NULL,"= \t",&TokPtr)) == NULL))
          break;

        MXMLSetAttr(E,"attr",attr,mdfString);
        MXMLSetAttr(E,"value",val,mdfString);
        }

        break;

      case 'q':  /* query */

        /* FORMAT:  -q [-o <QTYPE>] <JOBID> */

        /* <QTYPE> = { diag | starttime } */

        if (OptArg == NULL)
          break;

        MXMLSetAttr(E,"attr",OptArg,mdfString);

        MXMLSetAttr(E,"action",(void *)MJobCtlCmds[mjcmQuery],mdfString);
       
        break;

      case 'r':  /* resume */

        /* FORMAT:  -r <JOBID> */

        MXMLSetAttr(E,"action",(void *)MJobCtlCmds[mjcmResume],mdfString);      

        break;

      case 'R':  /* requeue */
 
        /* FORMAT:  -R <JOBID> */
 
        MXMLSetAttr(E,"action",(void *)MJobCtlCmds[mjcmRequeue],mdfString);
 
        break;

      case 's':  /* suspend */

        /* FORMAT:  -s <JOBID> */

        MXMLSetAttr(E,"action",(void *)MJobCtlCmds[mjcmSuspend],mdfString);
 
        break;

      case 'S':  /* submit */

        /* FORMAT:  -S <JOBID> */

        MXMLSetAttr(E,"action",(void *)MJobCtlCmds[mjcmSubmit],mdfString);

        break;

      case 't':  /* terminate */

        /* FORMAT:  -t <JOBID> */
 
        MXMLSetAttr(E,"action",(void *)MJobCtlCmds[mjcmTerminate],mdfString);
 
        break;

      case 'x':  /* execute */

        /* FORMAT:  -x [ -o {ignore={state|policy} | nodelist=<NODELIST>}] <JOBID> */

        MXMLSetAttr(E,"action",(void *)MJobCtlCmds[mjcmStart],mdfString);   

        {
        char *attr;
        char *val;
 
        char *TokPtr;
 
        if ((OptArg == NULL) || (strchr(OptArg,'=') == NULL))
          break;
 
        if (((attr = MUStrTok(OptArg,"= \t",&TokPtr)) == NULL) ||
            ((val = MUStrTok(NULL,"= \t",&TokPtr)) == NULL))
          break;
 
        MXMLSetAttr(E,"attr",attr,mdfString);
        MXMLSetAttr(E,"value",val,mdfString);
        }

        break;

      case '?':

        MCShowUsage(svcMJobCtl);

        exit(1);

        break;

      default:

        /* NO-OP */

        break; 
      }  /* END switch (Flag) */
    }    /* END while (Flag = MUGetOpt() != -1) */

  if (ArgCount > 1)
    MXMLSetAttr(E,"job",ArgV[ArgCount - 1],mdfString);

  MXMLToString(E,Request,MAX_MLINE,NULL,TRUE);

  MXMLDestroyE(&E);

  return(SUCCESS);
  }  /* END MCJobCtlCreateRequest() */




int MCBalCreateRequest(

  char **ArgV,     /* I */
  int    ArgC,     /* I */
  char  *Request)  /* O */

  {
  int   Flag;

  mxml_t *E  = NULL;
  mxml_t *RE = NULL;
  mxml_t *SE = NULL;
  mxml_t *WE = NULL;

  char  OptString[MAX_MLINE];
  char *OptArg;

  int   ArgCount;
  int   OptTok;

  /* FORMAT */

  /* NOTE:  global command modifiers should be checked and incorporated  (NYI) */
  /*  ie, verbose, format, etc */

  MXMLCreateE(&E,"Message");
  MXMLCreateE(&RE,"Request");

  MXMLSetAttr(RE,"object",(void *)"cluster",mdfString);

  MXMLAddE(E,RE);

  ArgCount = ArgC;

  MCProcessGeneralArgs(svcMResCtl,ArgV,&ArgCount,E);

  /* first extract command subtype */

  OptTok = 1;

  BalMode = mccmExecute;

  /* q-QUERY r-RUN */

  while ((Flag = MUGetOpt(&ArgCount,ArgV,"qr",&OptArg,&OptTok)) != -1)
    {
    DBG(6,fCONFIG) DPrint("INFO:     processing flag '%c'\n",
      (char)Flag);

    switch(Flag)
      {
      case 'q':

        BalMode = mccmQuery;

        MXMLCreateE(&SE,"Set");
        MXMLAddE(RE,SE);

        break;

      case 'r':

        BalMode = mccmExecute;

        MXMLCreateE(&SE,"Set");
        MXMLAddE(RE,SE);

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch(Flag) */
    }    /* END while(Flag) */

  MXMLSetAttr(RE,"action",(void *)MBalCmds[BalMode],mdfString);

  /* extract subcommand arguments */

  strcpy(OptString,"");

  OptTok = 1;

  while ((Flag = MUGetOpt(&ArgCount,ArgV,OptString,&OptArg,&OptTok)) != -1)
    {
    DBG(6,fCONFIG) DPrint("INFO:     processing flag '%c'\n",
      (char)Flag);

    switch(Flag)
      {
      default:

        /* NO-OP */

        break;
      }  /* END switch (Flag) */
    }    /* END while (Flag = MUGetOpt() != -1) */

  if (ArgCount > 1)
    {
    int index;

    MXMLCreateE(&WE,"Where");
    MXMLAddE(RE,WE);

    for (index = 1;index < ArgCount;index++)
      {
      AList[index + 1] = ArgV[index];

      MXMLSetAttr(WE,"name",(void *)MClusterAttr[mraName],mdfString);
      MXMLSetAttr(WE,"value",(void *)ArgV[index],mdfString);
      }  /* END for (index) */

    AList[index + 2] = NULL;
    }    /* END if (ArgCount > 1) */

  MXMLToString(E,Request,MAX_MLINE,NULL,TRUE);

  MXMLDestroyE(&E);

  return(SUCCESS);
  }  /* END MBalCreateRequest() */




int MCResCtlCreateRequest(

  char **ArgV,     /* I */
  int    ArgC,     /* I */
  char  *Request)  /* O */

  {
  int   Flag;

  int   Mode;

  mxml_t *E  = NULL;
  mxml_t *RE = NULL;
  mxml_t *SE = NULL;
  mxml_t *WE = NULL;

  char  OptString[MAX_MLINE];
  char *OptArg;

  int   ArgCount;
  int   OptTok;

  /* FORMAT */

  /* NOTE:  global command modifiers should be checked and incorporated  (NYI) */
  /*  ie, verbose, format, etc */

  MXMLCreateE(&E,"Message");
  MXMLCreateE(&RE,"Request");

  MXMLSetAttr(RE,"object",(void *)"res",mdfString);

  MXMLAddE(E,RE);

  ArgCount = ArgC;

  MCProcessGeneralArgs(svcMResCtl,ArgV,&ArgCount,E);

  /* first extract command subtype */

  OptTok = 1;

  Mode = -1;

  /* c-CREATE m-MODIFY q-QUERY r-RELEASE */

  while ((Flag = MUGetOpt(&ArgCount,ArgV,"cmq:r",&OptArg,&OptTok)) != -1)
    {
    DBG(6,fCONFIG) DPrint("INFO:     processing flag '%c'\n",
      (char)Flag);

    switch(Flag)
      {
      case 'c':

        Mode = mrcmCreate;

        MXMLSetAttr(RE,"action",(void *)MResCtlCmds[Mode],mdfString);

        MXMLCreateE(&SE,"Set");
        MXMLAddE(RE,SE);

        break;

      case 'm':

        Mode = mrcmModify;

        MXMLSetAttr(RE,"action",(void *)MResCtlCmds[Mode],mdfString);

        MXMLCreateE(&SE,"Set");
        MXMLAddE(RE,SE);

        break;

      case 'q':

        Mode = mrcmQuery;

        MXMLSetAttr(RE,"action",(void *)MResCtlCmds[Mode],mdfString);

        /* set query type (NYI) */

        /* {config|diag|resources} */

        MXMLSetAttr(RE,"attr",(void *)OptArg,mdfString);

        break;

      case 'r':

        Mode = mrcmDestroy;

        MXMLSetAttr(RE,"action",(void *)MResCtlCmds[Mode],mdfString);

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch(Flag) */
    }    /* END while(Flag) */

  if (Mode == -1)
    {
    return(FAILURE);
    }

  /* extract subcommand arguments */

  strcpy(OptString,"a:C:d:e:f:n:R:s:t:");

  OptTok = 1;
  
  while ((Flag = MUGetOpt(&ArgCount,ArgV,OptString,&OptArg,&OptTok)) != -1)
    {
    DBG(6,fCONFIG) DPrint("INFO:     processing flag '%c'\n",
      (char)Flag);

    /* FORMAT:  
        -c -a <ACL> -d <T> -f <FLAG> -s <T> -e <T> -t <COUNT>:<FEATURE> -n <NAME> -R <RES>
        -r <RESID>[<RESID>] ... 
        -m -a <ACL> -d <T> -f <FLAG> -s <T> -e <T> -t <COUNT>:<FEATURE> -n <NAME> -R <RES> <RESID>
        -q <QTYPE> <RESID>[<RESID>] ...
     */

    switch ((char)Flag)
      {
      case 'a':  /* acl */

        if (SE == NULL)
          break;

        MXMLSetAttr(SE,(char *)MResAttr[mraACL],(void *)OptArg,mdfString);

        break;

      case 'd':  /* duration */

        if (SE == NULL)
          break;

        MXMLSetAttr(SE,(char *)MResAttr[mraDuration],(void *)OptArg,mdfString);

        break;

      case 'e':  /* endtime */

        if (SE == NULL)
          break;

        MXMLSetAttr(SE,(char *)MResAttr[mraEndTime],(void *)OptArg,mdfString);

        break;

      case 'f':  /* flags */

        if (SE == NULL)
          break;

        MXMLSetAttr(SE,(char *)MResAttr[mraFlags],(void *)OptArg,mdfString);

        break;

      case 'n':  /* name */

        if (SE == NULL)
          break;

        MXMLSetAttr(SE,(char *)MResAttr[mraName],(void *)OptArg,mdfString);

        break;

      case 'R':  /* resources */

        if (SE == NULL)
          break;

        MXMLSetAttr(SE,(char *)MResAttr[mraResources],(void *)OptArg,mdfString);

        break;

      case 's':  /* starttime */

        if (SE == NULL)
          break;

        MXMLSetAttr(SE,(char *)MResAttr[mraStartTime],(void *)OptArg,mdfString);

        break;

      case 't':  /* tasks */

        if (SE == NULL)
          break;

        MXMLSetAttr(SE,(char *)MResAttr[mraHostExp],(void *)OptArg,mdfString);

        break;

      case '?':

        MCShowUsage(svcMResCtl);

        exit(1);

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch (Flag) */
    }    /* END while (Flag = MUGetOpt() != -1) */

  if (ArgCount > 1)
    {
    int index;

    MXMLCreateE(&WE,"Where");
    MXMLAddE(RE,WE);

    for (index = 0;index < ArgCount;index++)
      {
      MXMLSetAttr(WE,"name",(void *)MResAttr[mraName],mdfString);
      MXMLSetAttr(WE,"value",(void *)ArgV[index],mdfString);
      }  /* END for (index) */
    }    /* END if (ArgCount > 1) */
    
  MXMLToString(E,Request,MAX_MLINE,NULL,TRUE);

  MXMLDestroyE(&E);

  return(SUCCESS);
  }  /* END MCResCtlCreateRequest() */





int __MCShowGFlags()

  {
  fprintf(stderr,"  --about\n");
  fprintf(stderr,"  --configfile=<FILENAME>\n");
  fprintf(stderr,"  --format=<FORMAT>\n");
  fprintf(stderr,"  --help\n");
  fprintf(stderr,"  --host=<SERVERHOSTNAME>\n");
  fprintf(stderr,"  --keyfile=<FILENAME>\n");
  fprintf(stderr,"  --loglevel=<LOGLEVEL>\n");
  fprintf(stderr,"  --port=<SERVERPORT>\n");
  fprintf(stderr,"  --version\n");
  fprintf(stderr,"\n");

  return(SUCCESS);
  }  /* END __MCShowGFlags() */





int MCShowUsage(

  int CIndex)  /* I */

  {
  const char *FName = "MCShowUsage";
 
  DBG(3,fCORE) DPrint("%s(%d)\n",
    FName,
    CIndex);

  /* show flags */

  switch(CIndex)
    {
    case svcClusterShow:
 
      fprintf(stderr,"Usage: %s [FLAGS]\n",
        MService[CIndex]);

      __MCShowGFlags();

      fprintf(stderr,"  -x // XML\n");
 
      break;

    case svcDiagnose:

      fprintf(stderr,"Usage: %s [FLAGS]\n",
        MService[CIndex]);
 
      __MCShowGFlags();
 
      fprintf(stderr,"  -a [ACCOUNTID]\n");
      fprintf(stderr,"  -c [CLASSID]\n");
	  /** DsT added the possible options for f **/
      fprintf(stderr,"  -f [user|group|acct|class|qos] // fairshare\n");
      fprintf(stderr,"  -g [GROUPID]\n");
      fprintf(stderr,"  -j [JOBID]\n");
      fprintf(stderr,"  -m // frame\n");
      fprintf(stderr,"  -n [ -t <PARTITION> ] [NODEID]\n");
      fprintf(stderr,"  -p [ -t <PARTITION> ] // priority\n");
      fprintf(stderr,"  -q [ -l <POLICYLEVEL> ] // queue\n");
      fprintf(stderr,"  -Q [QOSID]\n");
      fprintf(stderr,"  -r [RESID]\n");
      fprintf(stderr,"  -s [SRESID]\n");
      fprintf(stderr,"  -t // partition\n");
      fprintf(stderr,"  -u [USERID]\n");
      fprintf(stderr,"  -v // VERBOSE\n");
 
      break;

    case svcJobShow:

      fprintf(stderr,"Usage: %s [FLAGS] <JOBID>\n",
        MService[CIndex]);
 
      __MCShowGFlags();
 
      fprintf(stderr,"  -A // AVP Mode\n");
      fprintf(stderr,"  -l <POLICYLEVEL>\n");
      fprintf(stderr,"  -n <NODE>\n");
      fprintf(stderr,"  -r <RES_ID>\n");
      fprintf(stderr,"  -v // VERBOSE\n");
 
      break;

    case svcMNodeCtl:

      fprintf(stderr,"Usage: %s [FLAGS] <SUBCOMMAND> <ARG> <NODEEXP>\n",
        MService[CIndex]);
 
      __MCShowGFlags(); 

      fprintf(stderr,"  <SUBCOMMAND> { create | destroy | modify }\n");
 
      break;
 
    case svcMGridCtl:

     fprintf(stderr,"Usage: %s [FLAGS]\n",
        MService[CIndex]);
 
      __MCShowGFlags();
 
      fprintf(stderr,"  -c <RESERVATION>\n");
      fprintf(stderr,"  -l { JOB | RES } <FLAGS> [<OBJECTID>]\n");
      fprintf(stderr,"  -m <RESERVATION> <ATTRLIST>\n");
      fprintf(stderr,"  -q [-r <RANGELIST>] [-t <TYPE>] [<RESOURCE DESCRIPTION>]\n");
      fprintf(stderr,"  -r { JOB | RES } <OBJECTID>\n");
      fprintf(stderr,"  -s [ -t <TYPE>] <RESOURCE DESCRIPTION>\n");
      fprintf(stderr,"  -S <RESOURCE DESCRIPTION>\n");
 
      break;

    case svcMJobCtl:

      fprintf(stderr,"Usage: %s [FLAGS]\n",
        MService[CIndex]);
 
      __MCShowGFlags();

      fprintf(stderr,"  -c <JOBID> // CANCEL\n");
      fprintf(stderr,"  -C <JOBID> // CHECKPOINT\n");
      fprintf(stderr,"  -h <JOBID> // HOLD\n");
      fprintf(stderr,"  -r <JOBID> // RESUME\n");
      fprintf(stderr,"  -R <JOBID> // REQUEUE\n");
      fprintf(stderr,"  -s <JOBID> // SUSPEND\n");
      fprintf(stderr,"  -S <JOBID> // SUBMIT\n");
      fprintf(stderr,"  -x <JOBID> // EXECUTE\n");
 
      break;

    case svcMResCtl:

      fprintf(stderr,"Usage: %s [FLAGS]\n",
        MService[CIndex]);

      __MCShowGFlags();

      fprintf(stderr,"  -c <RESID> // CREATE\n");
      fprintf(stderr,"  -m <RESID> // MODIFY\n");
      fprintf(stderr,"  -q <RESID> // QUERY\n");
      fprintf(stderr,"  -r <RESID> // RELEASE\n");

      break;

    case svcMShow:

      fprintf(stderr,"Usage: %s [FLAGS]\n",
        MService[CIndex]);

      __MCShowGFlags();

      break;

    case svcNodeShow:

      fprintf(stderr,"Usage: %s [FLAGS] <NODEID>\n",
        MService[CIndex]);
 
      __MCShowGFlags();
 
      fprintf(stderr,"  -v // VERBOSE\n");
 
      break;

    case svcResCreate:
 
      fprintf(stderr,"Usage: %s [FLAGS] <RES_EXP>\n",
        MService[CIndex]);

      __MCShowGFlags();      
 
      fprintf(stderr,"  -a <ACCOUNT>[:<ACCOUNT>]...\n");
      fprintf(stderr,"  -A <CHARGEACCOUNT>\n");
      fprintf(stderr,"  -c <CLASS>[:<CLASS>]...\n");

      fprintf(stderr,"  -d <TIME_EXP> // DURATION\n");
      fprintf(stderr,"  -e <TIME_EXP> // ENDTIME\n");
      fprintf(stderr,"  -f <FEATURE>[:<FEATURE>]...\n");
      fprintf(stderr,"  -g <GROUP>[:<GROUP>]...\n");
      fprintf(stderr,"  -j <JOBFEATURE>[:<JOBFEATURE>]...\n");
      fprintf(stderr,"  -n <NAME>\n");
/*
      fprintf(stderr,"  -N <NODESET>\n");
*/
      fprintf(stderr,"  -p <PARTITION>\n");
      fprintf(stderr,"  -q <QOS>[:<QOS>...]\n");
      fprintf(stderr,"  -r <RES_DESC>\n");
      fprintf(stderr,"  -s <TIME_EXP> // STARTTIME\n");
      fprintf(stderr,"  -u <USER>[:<USER>]...\n");
      fprintf(stderr,"  -x <FLAG>[:<FLAG>]...\n");

      fprintf(stderr,"\n"); 
      fprintf(stderr,"(TIME_EXP: [HH[:MM[:SS]]][_MO[/DD[/YY]]] ie 14:30_06/20)\n");
      fprintf(stderr,"(   or     +[[[DD:]HH:]MM:]SS\n");
 
      fprintf(stderr,"(RES_EXP:  ALL | TASKS{==|>=}<TASKCOUNT> | <HOST_REGEX>)\n");
      fprintf(stderr,"(RES_DESC: [PROCS=<X>][;MEM=<X>][;DISK=<X>][;SWAP=<X>;])\n");
 
/*
      fprintf(stderr,"(NODESET: <SETTYPE>:<SETATTR>[:<SETLIST>]...)\n");
*/
 
      break;

    case svcResShow:

      fprintf(stderr,"Usage: %s [FLAGS] [<RESID>]\n",
        MService[CIndex]);
 
      __MCShowGFlags();

      fprintf(stderr,"  -g // GREP-ABLE OUTPUT\n");
      fprintf(stderr,"  -n // NODE INFO\n");        
      fprintf(stderr,"  -r // RELATIVE TIME OUTPUT\n");      
      fprintf(stderr,"  -s // SUMMARY\n");
      fprintf(stderr,"  -v // VERBOSE\n");       
 
      break;

    case svcSched:
 
      fprintf(stderr,"Usage: %s [FLAGS]\n",
        MService[CIndex]);

      __MCShowGFlags();

      fprintf(stderr,"  -f <FAILURE_STATE>\n");
      fprintf(stderr,"  -j <JOB_TRACE>\n");
      fprintf(stderr,"  -k // kill scheduler\n");
      fprintf(stderr,"  -l [<PARAMETER_NAME>] // list parameters\n");
      fprintf(stderr,"  -m <STRING> // modify parameters\n");
      fprintf(stderr,"  -n // generate node trace\n");
      fprintf(stderr,"  -r [<RESUME_TIME>]\n");
/*    fprintf(stderr,"  -R\n"); */
      fprintf(stderr,"  -s [<NUM>] // stop at iteration <NUM>\n");
      fprintf(stderr,"  -S [<NUM>] // stop in <NUM> iterations\n");
 
      break;

    case svcShowBackfillWindow:
 
      fprintf(stderr,"Usage: %s [FLAGS]\n",
        MService[CIndex]);

      __MCShowGFlags(); 
 
      fprintf(stderr,"  -A // ALL\n");
      fprintf(stderr,"  -a <ACCOUNT>\n");
      fprintf(stderr,"  -c <CLASS>\n");
      fprintf(stderr,"  -d <DURATION> // <DURATION> = [[[DD:]HH:]MM:]SS\n");
      fprintf(stderr,"  -f <FEATURE>\n");
      fprintf(stderr,"  -g <GROUP>\n");
      fprintf(stderr,"  -m [<MEMCMP>]<MEMORY> // <MEMCMP> = { = | >= }\n");
      fprintf(stderr,"  -M <DEDICATED MEMORY PER PROC>\n");
      fprintf(stderr,"  -n <NODECOUNT>\n");
      fprintf(stderr,"  -p <PARTITION>\n");
      fprintf(stderr,"  -q <QOS>\n");
      fprintf(stderr,"  -r <PROCCOUNT>\n");
      fprintf(stderr,"  -S // SMP\n");
      fprintf(stderr,"  -u <USER>\n");
 
      break;
    case svcShowTasks:
      fprintf(stderr,"Usage: %s [FLAGS]\n",
        MService[CIndex]);

      __MCShowGFlags();

      fprintf(stderr,"  username // USER\n");

      break;

    case svcShowQ:

      fprintf(stderr,"Usage: %s [FLAGS]\n",
        MService[CIndex]);
 
      __MCShowGFlags();

      fprintf(stderr,"  -b // BLOCKED QUEUE\n");
      fprintf(stderr,"  -i // IDLE QUEUE\n");    
      fprintf(stderr,"  -r // ACTIVE QUEUE\n");    
      fprintf(stderr,"  -u [<USER>]\n");
      fprintf(stderr,"  -v // VERBOSE\n");

      break;

    case svcShowStats:

      fprintf(stderr,"Usage: %s [FLAGS]\n",
        MService[CIndex]);

      fprintf(stderr,"  -a [<ACCOUNT>]\n");
      fprintf(stderr,"  -c [<CLASS>]\n");
      fprintf(stderr,"  -g [<GROUP>]\n");
      fprintf(stderr,"  -h\n");
      fprintf(stderr,"  -q [<QOS>]\n");
      fprintf(stderr,"  -n [ -S ]\n");
      fprintf(stderr,"  -s [ -v ]\n");
      fprintf(stderr,"  -u [<USER>]\n");
 
      __MCShowGFlags();

      break;

    default:

      OMCShowUsage(CIndex);

      break;
    }  /* END switch(CIndex) */

  return(SUCCESS);
  }  /* END MCShowUsage() */




int MCExtractData(

  char     *Buf,  /* I */
  mxml_t **PE,   /* O */
  mxml_t **DE)   /* O (optional) */

  {
  char *ptr;
  
  mxml_t *E = NULL;
  mxml_t *C = NULL;

  if ((Buf == NULL) || (PE == NULL))
    {
    return(FAILURE);
    }

    if (((ptr = strstr(Buf,"<response")) == NULL) ||
       (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE))
    {
    /* cannot parse response */

    fprintf(stderr,"ERROR:    cannot parse server response (corrupt data)\n");

    return(FAILURE);
    }

    /* check status code */

  if (MXMLGetChild(E,"status",NULL,&C) == FAILURE)
    {
    /* cannot locate status code */

    fprintf(stderr,"ERROR:    cannot parse server response (no status)\n");

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  if (strcmp(C->Val,"success"))
    {
    /* command failed */

    char tmpMsg[MAX_MLINE];
    char tmpBuf[MAX_MNAME];

    int  tmpSC;

    if (MXMLGetAttr(C,"message",NULL,tmpMsg,sizeof(tmpMsg)) == FAILURE)
      {
      strcpy(tmpMsg,"N/A");
      }

    if (MXMLGetAttr(C,"code",NULL,tmpBuf,sizeof(tmpBuf)) == SUCCESS)
      {
      tmpSC = (int)strtol(tmpBuf,NULL,0);
      }
    else
      {
      tmpSC = -1;
      }

    fprintf(stderr,"ERROR:    command failed: '%s' (rc: %d)\n",
      tmpMsg,
      tmpSC);

    MXMLDestroyE(&E);

    return(FAILURE);
    }  /* END if (strcmp(C->Val,"success")) */

  if (MXMLGetChild(E,"data",NULL,&C) == FAILURE)
    {
    fprintf(stderr,"ERROR:    cannot parse server response (no data)\n");

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  *PE = E;

  if (DE != NULL)
    {
    *DE = C;
    }  /* END if (DE != NULL) */

  return(SUCCESS);
  }  /* END MCExtractData() */



    
/* function stubs */

int UIQueueShowAllJobs(char *S,long *SS,mpar_t *P, char *U) { return(SUCCESS); }
int UHProcessRequest(msocket_t *S,char *R) { return(SUCCESS); }

/* END function stubs */

/* END mclient.c */

