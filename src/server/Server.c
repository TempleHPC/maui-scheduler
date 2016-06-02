/* HEADER */

#define __BASEMODULE
 
#include "moab.h"
#include "msched-proto.h"
 
/* scheduler MODE variables */
 
extern mlog_t mlog;
 
extern msocket_t MClS[];
 
long      CREndTime = 0;
char      CRHostName[MAX_MNAME];

extern mclient_t MClient[MAX_MCLIENT];
extern mjob_t   *MJobTraceBuffer;
extern mjob_t   *MJob[MAX_MJOB];
extern mjobl_t   MJobName[MAX_MJOB + MAX_MHBUF];
extern mnode_t  *MNode[MAX_MNODE];
extern mgcred_t *MUser[MAX_MUSER + MAX_MHBUF];
extern mgcred_t  MGroup[MAX_MGROUP + MAX_MHBUF];
extern mgcred_t  MAcct[MAX_MACCT + MAX_MHBUF];
extern mqos_t    MQOS[MAX_MQOS];
extern msched_t  MSched;
extern mstat_t   MStat;
extern mckpt_t   MCP;
extern mres_t   *MRes[MAX_MRES];         /* terminated with NULL Name          */
extern mre_t     MRE[MAX_MRES << 2];
extern sres_t    SRes[MAX_MSRES];
extern sres_t    OSRes[MAX_MSRES];
extern mclass_t  MClass[MAX_MCLASS];
extern mpar_t    MPar[MAX_MPAR];
extern mrm_t     MRM[MAX_MRM];
extern msim_t    MSim;
extern mam_t     MAM[MAX_MAM];
extern mattrlist_t MAList;
extern mframe_t  MFrame[MAX_MFRAME];
extern int       MUIQ[MAX_MJOB];
extern int       MAQ[MAX_MJOB];          /* terminated by '-1' Value           */
extern mrange_t  MRange[MAX_MRANGE];
extern msys_t    MSys;                   /* cluster layout */

extern mx_t      X;

extern const mcfg_t MCfg[];
 
extern const char *MSimJSPolicy[];
extern const char *MPolicyMode[];
extern const char *MNAllocPolicy[];
extern const char *MResPolicy[];
extern const char *MBFPolicy[];
extern const char *MBFMPolicy[];
extern const char *MSimNCPolicy[];
extern const char *MNodeState[];
extern const char *MPolicyRejection[];
extern const char *MAllocRejType[];
extern const char *MLogFacilityType[];
extern const char *MSchedMode[];
extern const char *MSysAttr[];
extern const char *MJobState[];
extern const char *MJobFlags[];
extern const char *MAMOType[];
extern const char *MSSSCName[];
 
int MGlobalTLock = FALSE;
 
#include "OServer.c"




int main(

  int    ArgC,  /* I */
  char **ArgV)  /* I */

  {
  char     OldDay[MAX_MNAME];

  int      GlobalSQ[MAX_MJOB];
  int      GlobalHQ[MAX_MJOB];

  char    *tmpArgV[1024 + 1];

  int      aindex;
  char    *ptr;
  const char *FName = "main";

#ifdef __MPURIFY
  int Trigger_Purify;
#endif /* __MPURIFY */

  memset(temp_str,0,MMAX_LINE);

  MUGetTime(&MSched.Time,mtmInit,&MSched);

  ServerInitializeLog(ArgC,ArgV);

#ifdef __MPURIFY
  if (Trigger_Purify == 0)
    {
    DBG(3,fCONFIG) DPrint("INFO:     purify triggered\n");
    }
#endif /* __MPURIFY */

  DBG(5,fCORE) DPrint("%s(%d,ArgV)\n",
    FName,
    ArgC);
 
  MSysInitialize(TRUE);

  ServerSetSignalHandlers();

  for (aindex = 1;aindex < ArgC;aindex++)
    {
    if ((ArgV[aindex] == NULL) || (aindex >= 1024))
      break;

    tmpArgV[aindex] = NULL;
    MUStrDup(&tmpArgV[aindex],ArgV[aindex]);
    }
  tmpArgV[aindex] = NULL;

  tmpArgV[0] = NULL;
  if ((ptr=strrchr(ArgV[0],'/')) != NULL) {
    MUStrDup(&tmpArgV[0],ptr+1);
  } else {
    MUStrDup(&tmpArgV[0],ArgV[0]);
  }

  ServerProcessArgs(ArgC,tmpArgV,TRUE);
 
  if (MSysLoadConfig(MSched.HomeDir,MSched.ConfigFile,(1 << mcmForce)) == FAILURE)
    {
    if (MSysLoadConfig(".",MSched.ConfigFile,(1 << mcmForce)) == FAILURE)
      {
      DBG(3,fALL) DPrint("WARNING:  cannot locate configuration file '%s' in '%s' or in current directory\n",
        MSched.ConfigFile,
        MSched.HomeDir);
      }
    }
 
  XInitialize(&X,MSCHED_VERSION,&ArgC,ArgV,MSched.HomeDir);

  MSysDoTest();
 
  ServerProcessArgs(ArgC,tmpArgV,FALSE);

  /* walk to end of ArgV[] because ServerProcessArgs() may free portions of tmpArgV[]
     as side-affect */

  for (aindex = 0;aindex < ArgC;aindex++)
    {
    if (ArgV[aindex] == NULL)
      break;

    MUFree(&tmpArgV[aindex]);
    }
 
  if (MSched.Mode != msmSim)
    {
    ServerDemonize();
 
    ServerAuthenticate();
    }

  SDRGetSystemConfig();

  MSysStartServer(MSched.FBActive);

  while(1)
    {
    MStatInitializeActiveSysUsage();
 
    strcpy(OldDay,MSched.Day);
 
    ServerUpdate();

    DBG(2,fALL) DPrint("INFO:     starting iteration %d\n",
      MSched.Iteration);
 
    MSchedProcessJobs(OldDay,GlobalSQ,GlobalHQ);
 
    MSchedUpdateStats();
 
    if (MSched.Mode != msmSim)
      {
      MQueueCheckStatus();

      MNodeCheckStatus(NULL);
      }
 
    MSysCheck();
 
    MParUpdate(&MPar[0]);
 
    MResCheckStatus(NULL);
 
    DBG(0,fSCHED)
      {
      /* give heartbeat */
 
      if (!(MSched.Iteration % 100) && (MSched.Mode == msmSim))
        {
        fprintf(mlog.logfp,".");
 
        fflush(mlog.logfp);
        }
      }
 
    if (MSched.Mode != msmSim)
      {
      DBG(1,fSCHED) DPrint("INFO:     scheduling complete.  sleeping %ld seconds\n",
        MSched.RMPollInterval);
      }
    else 
      {
      MSimProcessEvents(GlobalHQ);
      }    /* END else (MSched.Mode != msmSim) */
 
    if ((MSched.Iteration % MSched.MaxSleepIteration) == 0)
      {
      MSched.EnvChanged = TRUE;
      }
    else
      {
      MSched.EnvChanged = FALSE;
      }
 
    UIProcessClients(&MSched.ServerS,MSched.RMPollInterval);
    UIProcessClients(&MSched.ServerSH,1);

    ServerProcessRequests();
 
    MOQueueDestroy(MUIQ,TRUE);
    MOQueueDestroy(GlobalSQ,FALSE);
    MOQueueDestroy(GlobalHQ,FALSE);
 
    if ((MSched.Mode == msmSingleStep) && (MSched.StepCount <= 1))
      {
      DBG(4,fSCHED) DPrint("INFO:     scheduler has completed single step scheduling\n");
 
      break;
      }
    else if ((MSched.Mode == msmSingleStep) && (MSched.Iteration >= MSched.StepCount))
      {
      DBG(4,fSCHED) DPrint("INFO:     scheduler has completed multi-step scheduling (%d steps)\n",
        MSched.StepCount);
 
      break;
      } 
 
    fflush(mlog.logfp);
    }  /* END while(1) */
 
  DBG(1,fSCHED) DPrint("INFO:     scheduling completed on %s\n",
    MULToDString(&MSched.Time));
 
  exit(0);
 
  /*NOTREACHED*/
 
  return(0);
  }  /* END main() */




int ServerInitializeLog(
 
  int    ArgC,
  char **ArgV)
 
  {
  char LogFile[MAX_MLINE];
  char tmpLine[MAX_MLINE];
 
  /* set logging defaults */
 
  mlog.logfp        = stderr;
  mlog.Threshold    = DEFAULT_MLOGLEVEL;
  mlog.FacilityList = DEFAULT_MLOGFACILITY;
 
  /* handle command line based logging specification */
 
  strcpy(LogFile,DEFAULT_MLOGFILE);
 
  if (ArgC > 1)
    {
    int aindex = 1;
 
    if ((ArgV[aindex][0] == '-') &&
        (ArgV[aindex][1] == 'L'))
      {
      if (ArgV[aindex][2] != '\0')
        {
        mlog.Threshold = (int)strtol(&ArgV[aindex][2],NULL,10);
 
        aindex++;
        }
      else
        { 
        mlog.Threshold = (int)strtol(ArgV[aindex + 1],NULL,10);
 
        aindex += 2;
        }
 
      DBG(3,fCONFIG) DPrint("INFO:     early LOGLEVEL set to %d\n",
        mlog.Threshold);
      }
 
    if ((ArgC > aindex) &&
        (ArgV[aindex][0] == '-') &&
        (ArgV[aindex][1] == 'l'))
      {
      if (ArgV[aindex][2] != '\0')
        {
        MUStrCpy(LogFile,&ArgV[aindex][2],sizeof(LogFile));
        }
      else
        {
        MUStrCpy(LogFile,ArgV[aindex + 1],sizeof(LogFile));
        }
 
      DBG(3,fCONFIG) DPrint("INFO:     early LOGFILE set to %s\n",
        LogFile);
      }
    }
 
  if ((LogFile[0] != '/') && (LogFile[0] != '~'))
    {
    char *ptr;
 
    if ((ptr = getenv(MSCHED_ENVHOMEVAR)) != NULL)
      {
      MUStrCpy(tmpLine,ptr,sizeof(tmpLine));
 
      sprintf(LogFile,"%s/%s",
        tmpLine, 
        DEFAULT_MLOGDIR);
      }
    else
      {
#ifdef MBUILD_HOMEDIR
      sprintf(LogFile,"%s/%s",
        MBUILD_HOMEDIR,
        DEFAULT_MLOGDIR);
#else /* MBUILD_HOMEDIR */
      strcpy(LogFile,DEFAULT_MLOGDIR);
#endif /* MBUILD_HOMEDIR */
      }
    }
 
  strcpy(tmpLine,DEFAULT_MLOGFILE);
 
  if ((tmpLine[0] != '/') && (tmpLine[0] != '~'))
    {
    if (LogFile[strlen(LogFile) - 1] != '/')
      strcat(LogFile,"/");
 
    MUStrCat(LogFile,DEFAULT_MLOGFILE,sizeof(LogFile));
    }
  else
    {
    MUStrCpy(LogFile,tmpLine,sizeof(LogFile));
    }
 
  MLogInitialize(LogFile,DEFAULT_MLOGFILEMAXSIZE,MSched.Iteration);
 
  return(SUCCESS);
  }  /* END ServerInitializeLog() */




int ServerGetAuth(

  char *AString,
  long *AFlags)

  {
  char *name;
  char *pwd;

  char *TokPtr;

  int   index;

  long  tmpL;

  /* FORMAT:  <USERNAME>[:<PASSWORD>] */

  if (AFlags != NULL)
    *AFlags = 0;

  if ((name = MUStrTok(AString,":",&TokPtr)) == NULL)
    {
    /* no auth string available */

    return(FAILURE);
    }
 
  if ((pwd = MUStrTok(AString,":",&TokPtr)) != NULL)
    {
    /* password string detected */

    /* NYI */
    }

  tmpL = 0;  

  /* determine auth admin level */
 
  for (index = 0;MSched.Admin1User[index][0] != '\0';index++)
    {
    if (!strcmp(MSched.Admin1User[index],name) || 
        !strcmp(MSched.Admin1User[index],"ALL"))
      {
      tmpL |= (1 << fAdmin1);
 
      break;
      }
    }    /* END for (index) */
 
  for (index = 0;MSched.Admin2User[index][0] != '\0';index++)
    {
    if (!strcmp(MSched.Admin2User[index],name) || 
        !strcmp(MSched.Admin2User[index],"ALL"))
      {
      tmpL |= (1 << fAdmin2);
 
      break;
      }
    }    /* END for (index) */
 
  for (index = 0;MSched.Admin3User[index][0] != '\0';index++)
    {
    if (!strcmp(MSched.Admin3User[index],name) || 
        !strcmp(MSched.Admin3User[index],"ALL"))
      {
      tmpL |= (1 << fAdmin3);
 
      break;
      }
    }    /* END for (index) */

  if (AFlags != NULL)
    *AFlags = tmpL;

  return(SUCCESS);
  }  /* END ServerGetAuth() */




int MServerConfigShow(
 
  char *Buffer,   /* O */
  int   PIndex,   /* I */
  int   VFlag)    /* I */

  {
  int  index;

  char Line[MAX_MLINE];

  const char *FName = "MServerConfigShow";

  DBG(1,fCONFIG) DPrint("%s(Buffer,PIndex,VFlag)\n",
    FName);

  if (Buffer == NULL)
    {
    return(FAILURE);
    }

  Buffer[0] = '\0';

  /* display modules */

  sprintf(temp_str,"# SERVER MODULES: ");
  strcat(Buffer,temp_str);

#ifdef __MX
  sprintf(temp_str," MX");
  strcat(Buffer,temp_str);
#endif /* __MX */

#ifdef __MMD5
  sprintf(temp_str," MD5");
  strcat(Buffer,temp_str);
#endif /* __MMD5 */

  sprintf(temp_str,"\n");
  strcat(Buffer,temp_str);

  /* display parameters */

  sprintf(temp_str,"%-30s  %s\n",
    MParam[pSchedMode],
    MSchedMode[MSched.Mode]); 
  strcat(Buffer,temp_str);

  if ((VFlag || (PIndex == -1) || (PIndex == pServerName)))
    {
    sprintf(temp_str,"%-30s  %s\n",MParam[pServerName],MSched.Name);
    strcat(Buffer,temp_str);
    }
 
  sprintf(temp_str,"%-30s  %s\n",MParam[pServerHost],MSched.ServerHost);
  strcat(Buffer,temp_str);
  sprintf(temp_str,"%-30s  %d\n",MParam[pServerPort],MSched.ServerPort);
  strcat(Buffer,temp_str);

  if (MSched.LogFile[0] == '\0')
    {
    sprintf(temp_str,"# NO LOGFILE SPECIFIED\n");
    strcat(Buffer,temp_str);
    sprintf(temp_str,"%-30s  %s\n",MParam[pSchedLogFile],MSched.LogFile);
    strcat(Buffer,temp_str);
    }
  else
    {
    sprintf(temp_str,"%-30s  %s\n",MParam[pSchedLogFile],MSched.LogFile);
    strcat(Buffer,temp_str);
    sprintf(temp_str,"%-30s  %d\n",MParam[pLogFileMaxSize],MSched.LogFileMaxSize);
    strcat(Buffer,temp_str);
    sprintf(temp_str,"%-30s  %d\n",MParam[pLogFileRollDepth],MSched.LogFileRollDepth);
    strcat(Buffer,temp_str);
    }

  sprintf(temp_str,"%-30s  %d\n",MParam[pLogLevel],mlog.Threshold);
  strcat(Buffer,temp_str);
 
  if ((mlog.FacilityList != fALL) || 
      (VFlag || 
      (PIndex == -1) || 
      (PIndex == pLogFacility)))
    {
    Line[0] = '\0';
 
    for (index = 0;MLogFacilityType[index] != NULL;index++)
      {
      if (mlog.FacilityList & (1 << index))
        {
        if (Line[0] != '\0')
          strcat(Line,"|");
 
        strcat(Line,MLogFacilityType[index]);
        }
      }    /* END for (index) */
 
    if (mlog.FacilityList == fALL)
      strcpy(Line,MLogFacilityType[dfALL]);
 
    sprintf(temp_str,"%-30s  %s\n",MParam[pLogFacility],Line);
    strcat(Buffer,temp_str);
    }

  sprintf(temp_str,"%-30s  %s\n",MParam[pMServerHomeDir],MSched.HomeDir);
  strcat(Buffer,temp_str);
  sprintf(temp_str,"%-30s  %s\n",MParam[pSchedToolsDir],MSched.ToolsDir);
  strcat(Buffer,temp_str);
  sprintf(temp_str,"%-30s  %s\n",MParam[pSchedLogDir],MSched.LogDir);
  strcat(Buffer,temp_str);
  sprintf(temp_str,"%-30s  %s\n",MParam[pStatDir],MStat.StatDir);
  strcat(Buffer,temp_str);
 
  sprintf(temp_str,"%-30s  %s\n",MParam[pSchedLockFile],MSched.LockFile);
  strcat(Buffer,temp_str);
  sprintf(temp_str,"%-30s  %s\n",MParam[pSchedConfigFile],MSched.ConfigFile);
  strcat(Buffer,temp_str);

  if ((MSched.Action[mactAdminEvent][0] != '\0') || VFlag)
    { 
    sprintf(temp_str,"%-30s  %s\n",MParam[pAdminEAction],MSched.Action[mactAdminEvent]);
    strcat(Buffer,temp_str);
    }

  if ((MSched.Action[mactJobFB][0] != '\0') || VFlag)
    {
    sprintf(temp_str,"%-30s  %s\n",MParam[mcoJobFBAction],MSched.Action[mactJobFB]);
    strcat(Buffer,temp_str);
    }

  if ((MSched.Action[mactMail][0] != '\0') || VFlag)
    {
    sprintf(temp_str,"%-30s  %s\n",MParam[mcoMailAction],MSched.Action[mactMail]);
    strcat(Buffer,temp_str);
    }
 
  sprintf(temp_str,"%-30s  %s\n",MParam[pCheckPointFile],MCP.CPFileName);
  strcat(Buffer,temp_str);
  sprintf(temp_str,"%-30s  %s\n",MParam[pCheckPointInterval],MULToTString(MCP.CPInterval));
  strcat(Buffer,temp_str);
  sprintf(temp_str,"%-30s  %s\n",MParam[pCheckPointExpirationTime],MULToTString(MCP.CPExpirationTime));
  strcat(Buffer,temp_str);

  if ((MSched.MonitoredJob[0] != '\0') || (VFlag || (PIndex == -1) || (PIndex == pMonitoredJob))){
    sprintf(temp_str,"%-30s  %s\n",MParam[pMonitoredJob],MSched.MonitoredJob);
    strcat(Buffer,temp_str);
  }
 
  if ((MSched.MonitoredNode[0] != '\0') || (VFlag || (PIndex == -1) || (PIndex == pMonitoredNode))){
    sprintf(temp_str,"%-30s  %s\n",MParam[pMonitoredNode],MSched.MonitoredNode);
    strcat(Buffer,temp_str);
  }
 
  if ((MSched.MonitoredFunction[0] != '\0') || (VFlag || (PIndex == -1) || (PIndex == pMonitoredFunction))){
    sprintf(temp_str,"%-30s  %s\n",MParam[pMonitoredFunction],MSched.MonitoredFunction);
    strcat(Buffer,temp_str);
  }

  if ((MSched.ResDepth != DEFAULT_RES_DEPTH) || (VFlag || (PIndex == -1) || (PIndex == pResDepth))){
    sprintf(temp_str,"%-30s  %d\n",MParam[pResDepth],MSched.ResDepth);
    strcat(Buffer,temp_str);
  }

  strcat(Buffer,"\n");      

  return(SUCCESS);
  }  /* END MServerConfigShow() */





int ServerProcessArgs(
 
  int    ArgC,
  char **ArgV,     /* I (potentially modified) */
  int    PreLoad)  /* I (boolean) */
 
  {
  char  Flag;
  int   index;
 
  int   tmpArgC;
 
  char  tmp[MAX_MLINE];
 
  char *OptArg;
  int   OptTok = 1;

  char  AString[MAX_MLINE];
 
  const char *FName = "ServerProcessArgs";
 
  DBG(1,fCONFIG) DPrint("%s(%d,ArgV,%d)\n",
    FName,
    ArgC,
    PreLoad);

  if (PreLoad == TRUE)
    {
    /* preserve all args */
 
    for (index = 0;index < MAX_MARG;index++)
      { 
      if (ArgV[index] == NULL)
        break;
 
      MUStrDup(&MSched.Argv[index],ArgV[index]);
      }  /* END for (index) */
 
    MSched.Argv[index] = NULL;

    strcpy(AString,"C:dhH:v?-:");
    }
  else
    {
    strcpy(AString,"a:Ab:B:c:C:dD:f:hH:i:j:l:L:m:n:N:p:P:r:s:v?-:");
    }

  /* NOTE:  '--' args should be processed in each pass (NYI) */

  tmpArgC = ArgC;
 
  while ((Flag = MUGetOpt(&tmpArgC,ArgV,AString,&OptArg,&OptTok)) != (char) -1)
    {
    switch (Flag)
      {
      case '-':

        {
        char *ptr;

        char *TokPtr = NULL;
        char *Value  = NULL;

        if (OptArg == NULL)
          {
          break;
          }

        if ((ptr = MUStrTok(OptArg,"=",&TokPtr)) != NULL)
          {
          Value = MUStrTok(NULL,"=",&TokPtr);
          }

        if (!strcmp(OptArg,"about"))
          {
          mccfg_t C;

          memset(&C,0,sizeof(C));

          strcpy(C.ServerHost,DEFAULT_MSERVERHOST);
          C.ServerPort = DEFAULT_MSERVERPORT;

          strcpy(C.HomeDir,MBUILD_HOMEDIR);
          strcpy(C.ConfigFile,DEFAULT_SchedCONFIGFILE);

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

          fprintf(stderr,"%s server version %s\n",
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
        else if ((!strcmp(OptArg,"configfile")) && (Value != NULL))
          {
          MUStrCpy(MSched.ConfigFile,Value,sizeof(MSched.ConfigFile));
 
          DBG(3,fCONFIG) DPrint("INFO:     server configfile set to %s\n",
            MSched.ConfigFile);
 
          MSched.SecureMode = FALSE;
          }
        else if (!strcmp(OptArg,"help"))
          {
          ServerShowUsage(ArgV[0]);
 
          exit(0);
          } 
        else if (!strcmp(OptArg,"keyfile"))
          {
          FILE *fp;
          char  tmpS[MAX_MLINE];
 
          if (Value == NULL)
            {
            ServerShowUsage(ArgV[0]);
 
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
 
          fscanf(fp,"%64s",
            tmpS);

          MUStrCpy(MSched.DefaultCSKey,tmpS,MAX_MNAME);     
 
          fclose(fp);
          }
        else if ((!strcmp(OptArg,"loglevel")) && (Value != NULL))
          {
          mlog.Threshold = (int)strtol(Value,NULL,0);
 
          DBG(3,fCONFIG) DPrint("INFO:     log level set to %d\n",
            mlog.Threshold);
 
          break;
          }
        else if ((!strcmp(OptArg,"host")) && (Value != NULL))
          {
          MUStrCpy(MSched.ServerHost,Value,sizeof(MSched.ServerHost));

          DBG(3,fCONFIG) DPrint("INFO:     server host set to %s\n",
            MSched.ServerHost);
          }
        else if ((!strcmp(OptArg,"port")) && (Value != NULL))
          {
          MSched.ServerPort = (int)strtol(Value,NULL,0);
 
          DBG(3,fCONFIG) DPrint("INFO:     server port set to %d\n",
            MSched.ServerPort);
          }
        else if (!strcmp(OptArg,"version"))
          {
          fprintf(stderr,"%s version %s\n",
            MSCHED_NAME,
            MSCHED_VERSION);
 
          ServerShowCopy();
 
          exit(0);
          }
        }    /* END BLOCK */

        break;
 
      case 'a':
 
        MPar[0].NAllocPolicy = MUGetIndex(
          OptArg,
          MNAllocPolicy,
          FALSE,
          MPar[0].NAllocPolicy);
 
        MPar[1].NAllocPolicy = MPar[0].NAllocPolicy;
 
        DBG(3,fCONFIG) DPrint("INFO:     node allocation policy set to %s\n", 
          MNAllocPolicy[MPar[0].NAllocPolicy]);
 
        break;

      case 'A':

        MSched.FBActive = TRUE;

        DBG(3,fCONFIG) DPrint("INFO:     scheduler fallback mode enabled\n");

        break;
 
      case 'b':

        MPar[0].BFPolicy = MUGetIndex(OptArg,MBFPolicy,FALSE,bfNONE);

        DBG(3,fCONFIG) DPrint("INFO:     backfill policy type set to %s\n",
          MBFPolicy[MPar[0].BFPolicy]);
 
        break;

      case 'B':
 
        MPar[0].BFDepth = atoi(OptArg);
 
        DBG(3,fCONFIG) DPrint("INFO:     backfill depth set to %d\n",
          MPar[0].BFDepth);
 
        break;
 
      case 'C':
 
        MUStrCpy(MSched.ConfigFile,OptArg,sizeof(MSched.ConfigFile));
 
        DBG(3,fCONFIG) DPrint("INFO:     configfile set to %s\n",
          MSched.ConfigFile);
 
        MSched.SecureMode = FALSE;
 
        break;
 
      case 'c':

        MPar[0].BFMetric = MUGetIndex(OptArg,MBFMPolicy,FALSE,0);

        DBG(3,fCONFIG) DPrint("INFO:     backfill metric set to %s\n",
          MBFMPolicy[MPar[0].BFMetric]);
 
        break;
 
      case 'd':
 
        /* set 'debug' mode */

        MSched.DebugMode = TRUE;
 
        break;
 
      case 'f':
 
        mlog.FacilityList = strtol(OptArg,NULL,0);
 
        if (mlog.FacilityList == 0)
          {
          for (index = 0;MLogFacilityType[index] != NULL;index++)
            {
            if (strstr(OptArg,MLogFacilityType[index]) != NULL)
              { 
              mlog.FacilityList |= (1 << index);
 
              if (index == dfALL)
                mlog.FacilityList |= fALL;
              }
            }   /* END for (index) */
          }
 
        DBG(3,fCONFIG) DPrint("INFO:     LOGFACILITY set to %ld\n",
          mlog.FacilityList);
 
      case 'h':
 
        ServerShowUsage(ArgV[0]);
 
        exit(0);

        /*NOTREACHED*/
 
        break;
 
      case 'i':
 
        MSched.RMPollInterval = strtol(OptArg,NULL,0);
 
        DBG(3,fCONFIG) DPrint("INFO:     RMPollInterval set to %ld seconds\n",
          MSched.RMPollInterval);
 
        break;
 
      case 'j':
 
        MSched.Mode = msmSim;
 
        MUStrCpy(MSim.WorkloadTraceFile,OptArg,sizeof(MSim.WorkloadTraceFile));
 
        DBG(3,fCONFIG) DPrint("INFO:     simulation mode set.  using workload tracefile '%s'\n",
          MSim.WorkloadTraceFile);
 
        srand(1); 
 
        break;
 
      case 'l':
 
        MUStrCpy(MSched.LogFile,OptArg,sizeof(MSched.LogFile));
 
        DBG(3,fCONFIG) DPrint("INFO:     logfile set to '%s'\n",
          MSched.LogFile);
 
        if ((MSched.LogFile[0] == '/') || (MSched.LogDir[0] == '\0'))
          {
          MUStrCpy(tmp,MSched.LogFile,sizeof(tmp));
          }
        else
          {
          MUStrCpy(tmp,MSched.LogDir,sizeof(tmp));
 
          if (MSched.LogDir[strlen(MSched.LogDir) - 1] != '/')
            strcat(tmp,"/");
 
          MUStrCat(tmp,MSched.LogFile,sizeof(tmp));
          }
 
        MLogInitialize(tmp,-1,MSched.Iteration);
 
        break;

      case 'L':
 
        mlog.Threshold = atoi(OptArg);
 
        DBG(3,fCONFIG) DPrint("INFO:     loglevel set to %d\n",
          mlog.Threshold);
 
        break;
 
      case 'm':
 
        MSched.Mode = MUGetIndex(OptArg,MSchedMode,FALSE,MSched.Mode);
 
        break;
 
      case 'n':
 
        MSim.NodeCount = atoi(OptArg);
 
        DBG(3,fCONFIG) DPrint("INFO:     nodecount set to %d\n",
          MSim.NodeCount); 
 
        break;
 
      case 'N':

        MSim.NCPolicy = MUGetIndex(OptArg,MSimNCPolicy,FALSE,MSim.NCPolicy);

        DBG(3,fCONFIG) DPrint("INFO:     nodeconfiguration set to %s\n",
          MSimNCPolicy[MSim.NCPolicy]);
 
        break;
 
      case 'p':
 
        MSched.ServerPort = (int)strtol(OptArg,NULL,0);
 
        DBG(3,fCONFIG) DPrint("INFO:     server port set to %d\n",
          MSched.ServerPort);
 
        break;
 
      case 'P':

        if ((OptArg == NULL) || (OptArg[0] == '\0'))
          { 
          MSched.Schedule = FALSE;
 
          DBG(3,fCONFIG) DPrint("INFO:     scheduler paused\n");
          }
        else
          {
          /* stop at specified iteration */

          MSched.Schedule = TRUE;

          MSim.StopIteration = (int)strtol(OptArg,NULL,0);
          }
 
        break;
 
      case 'r':
 
        MSched.Mode = msmSim; 
 
        MUStrCpy(MSim.ResourceTraceFile,OptArg,sizeof(MSim.ResourceTraceFile));
 
        DBG(3,fCONFIG) DPrint("INFO:     simulation mode set.  using resource tracefile '%s'\n",
          MSim.ResourceTraceFile);
 
        srand(1);
 
        break;
 
      case 's':
 
        MUStrCpy(MSim.StatFileName,OptArg,sizeof(MSim.StatFileName));
 
        DBG(3,fCONFIG) DPrint("INFO:     simulation statfile set to '%s'\n",
          MSim.StatFileName);
 
        break;
 
      case 'v':
 
        fprintf(stderr,"%s version %s\n",
          MSCHED_NAME,
          MSCHED_VERSION);
 
        ServerShowCopy();
 
        exit(0);
 
        break;
 
      case '?':
 
        ServerShowUsage(ArgV[0]);
 
        exit(0);
 
        break;
 
      default: 
 
        DBG(1,fCONFIG) DPrint("WARNING:  unexpected flag '%c'\n",
          Flag);
 
        break;
      }  /* END switch (Flag)                     */
    }    /* END while ((Flag = MUGetOpt()) != -1) */
 
  return(SUCCESS);
  }  /* END ServerProcessArgs() */




int ServerShowCopy()

  {
  fprintf(stderr,"Copyright 2000-2010 Cluster Resources, Inc, All Rights Reserved\n");
  fprintf(stderr,"This software includes modifications by the Temple HPC Team, Copyright 2016\n");
  fprintf(stderr,"  for the latest release, see https://github.com/TempleHPC/maui-scheduler\n");

  fprintf(stderr,"This software includes the Maui Server Module, Copyright 1996 MHPCC, All Rights Reserved\n");
 
  MUShowCopy();

  return(SUCCESS);
  }  /* END ServerShowCopy() */




int ServerShowUsage(
 
  char *Command)
 
  {
  fprintf(stderr,"Usage:   %s [FLAGS]\n",
    Command);

  fprintf(stderr,"  --configfile=<FILENAME>\n");
  fprintf(stderr,"  --help\n");
  fprintf(stderr,"  --host=<HOSTNAME>\n");
  fprintf(stderr,"  --keyfile=<FILENAME>\n");
  fprintf(stderr,"  --loglevel=<LOGLEVEL>\n");
  fprintf(stderr,"  --port=<SERVERPORT>\n");
  fprintf(stderr,"  --version\n");

  fprintf(stderr,"  [ -a <NODE_ALLOCATION_POLICY> ]\n");
  fprintf(stderr,"  [ -b <BACKFILL_POLICY> ]\n");
  fprintf(stderr,"  [ -B <BACKFILL_DEPTH> ]\n");        
  fprintf(stderr,"  [ -c <BACKFILL_CRITERIA> ]\n");
  fprintf(stderr,"  [ -C <CONFIG_FILE> ]\n");
  fprintf(stderr,"  [ -d <DEBUG_MODE> ] // DISABLE AUTO BACKGROUND\n");
  fprintf(stderr,"  [ -L <LOGLEVEL> ]\n");
  fprintf(stderr,"  [ -f <LOGFACILITY> ]\n");
  fprintf(stderr,"  [ -h ] // HELP\n");
  fprintf(stderr,"  [ -H <SERVER_HOME_DIRECTORY> ]\n");
  fprintf(stderr,"  [ -i <SCHEDULING_INTERVAL> ]\n");
  fprintf(stderr,"  [ -j <WORKLOAD_TRACE_FILE> ]\n");
  fprintf(stderr,"  [ -l <LOG_FILE> ]\n");
  fprintf(stderr,"  [ -L <LOG_LEVEL> ]\n");
  fprintf(stderr,"  [ -m <SCHEDULER_MODE> ]\n");
  fprintf(stderr,"  [ -n <NODE_COUNT> ]\n");
  fprintf(stderr,"  [ -N <NODE_CONFIGURATION> ]\n");
  fprintf(stderr,"  [ -p <SERVICE_PORT> ]\n");
  fprintf(stderr,"  [ -P [<STOPITERATION>]] // PAUSE\n");
  fprintf(stderr,"  [ -r <RESOURCE_TRACE_FILE> ]\n");
  fprintf(stderr,"  [ -s <SIM_STAT_FILE_NAME> ]\n");
  fprintf(stderr,"  [ -w <WALLCLOCK ACCURACY> ]\n");
  fprintf(stderr,"  [ -W <WC_ACCURACY_CHANGE> ]\n");
  fprintf(stderr,"  [ -v ] // VERSION\n");
  fprintf(stderr,"  [ -? ] // HELP\n");
 
  return(SUCCESS);
  }  /* END ServerShowUsage() */




int ServerLoadSignalConfig()

  {
  char *ptr;

  /* NOTE:  CHLD interferes with some RM API's */

  /* signal(SIGCHLD,  (void(*)(int))SIG_IGN); */

/* NOTE: HUP temporarily disabled  *
  signal(SIGHUP,   (void(*)(int))ReloadConfig);
*/
  signal(SIGHUP,   (void(*)(int))SIG_IGN);
  signal(SIGPIPE,  (void(*)(int))SIG_IGN);
 
  if ((ptr = getenv(MSCHED_ENVCRASHVAR)) == NULL)
    {
    /* use default signal handling */
    }
  else if (!strcmp(ptr,"TRAP") || !strcmp(ptr,"trap"))
    {
    signal(SIGSEGV,  (void(*)(int))CrashMode);
    signal(SIGILL,   (void(*)(int))CrashMode);
    }
  else if (!strcmp(ptr,"IGNORE") || !strcmp(ptr,"ignore"))
    {
    signal(SIGSEGV,  (void(*)(int))SIG_IGN);
    signal(SIGILL,   (void(*)(int))SIG_IGN);
    }
  else if (!strcmp(ptr,"DIE") || !strcmp(ptr,"die"))
    {
    signal(SIGSEGV,  (void(*)(int))SIG_DFL);
    signal(SIGILL,   (void(*)(int))SIG_DFL);
    }
  else
    {
    /* unknown signal config */

    signal(SIGSEGV,  (void(*)(int))ServerRestart);
    signal(SIGILL,   (void(*)(int))ServerRestart);
    }

  return(SUCCESS);
  }  /* END ServerLoadSignalConfig() */




int ServerUpdate()

  {
  char            OldDay[MAX_MNAME];

  const char *FName = "ServerUpdate";

  DBG(3,fALL) DPrint("%s()\n",
    FName);

  strcpy(OldDay,MSched.Day);

  /* adjust time/iteration */

  MSysUpdateTime(&MSched);

  MSched.Iteration++;

  /* handle 'pre scheduling' tasks */

  if (strcmp(MSched.Day,OldDay))
    {
    /* starting new day */

    if (MSched.Mode != msmSim)
      {
      /* update statistics */

      MStatOpenFile(MSched.Time);

      fprintf(MSched.statfp,"%s %d\n",
        TRACE_WORKLOAD_VERSION_MARKER,
        DEFAULT_WORKLOAD_TRACE_VERSION);
      }
    }    /* END if (strcmp(MSched.Day,ptr)) */

  return(SUCCESS);
  }  /* END ServerUpdate() */




int ServerProcessRequests()

  {
  mfsc_t *FC;

  const char *FName = "ServerProcessRequests";

  DBG(2,fCORE) DPrint("%s()\n",
    FName);

  FC = &MPar[0].FSC;

  /* reload config file if necessary */

  if ((MSched.Reload == TRUE) && 
     ((MSched.Mode != msmSim) || (getenv(MSCHED_ENVCKTESTVAR) != NULL)))
    {
    int OldIteration;
    int OldStopIteration;

    OldIteration     = MSched.Iteration;
    OldStopIteration = MSim.StopIteration;

    MCPCreate(MCP.CPFileName);

    MCP.LastCPTime = MSched.Time;

    MSysDestroyObjects();

    MSysInitialize(TRUE);

    ServerSetSignalHandlers();

    DBG(1,fCONFIG) DPrint("INFO:     reloading config files\n");

    if (MSysLoadConfig(MSched.HomeDir,MSched.ConfigFile,(1 << mcmForce)) == FAILURE)
      {
      if (MSysLoadConfig(".",MSched.ConfigFile,(1 << mcmForce)) == FAILURE)
        {
        DBG(3,fALL) DPrint("WARNING:  cannot locate configuration file '%s' in '%s' or in current directory\n",
          MSched.ConfigFile,
          MSched.HomeDir);
        }
      }

    MStatProfInitialize(&MStat.P);

#ifdef __MSDR

    if (MSched.Mode != msmSim)
      {
      SDRGetSystemConfig();
      }

#endif /* __MSDR */

    if (FC->FSPolicy != fspNONE)
      {
      MFSInitialize(FC);
      }

    MCPLoad(MCP.CPFileName,mckptResOnly);

    MSched.Iteration   = OldIteration;
    MSim.StopIteration = OldStopIteration;
    }  /* END if ((MSched.Reload == TRUE) && (MSched.Mode != msmSim)) */

  MLogRoll(
    NULL,
    0,
    MSched.Iteration,
    MSched.LogFileRollDepth);

  /* update CP */

  if (((MSched.Time - MCP.LastCPTime) > MCP.CPInterval) &&
      ((MSched.Mode != msmSim) || (getenv(MSCHED_ENVCKTESTVAR) != NULL)))
    {
    MCPCreate(MCP.CPFileName);

    MCP.LastCPTime = MSched.Time;

    if (FC->FSPolicy != fspNONE)
      MFSUpdateData(FC,MSched.CurrentFSInterval,(1 << mfsactWrite));
    }

  if (FC->FSPolicy != fspNONE)
    {
    long NewFSInterval;

    /* if FS interval reached */

    NewFSInterval = MSched.Time - (MSched.Time % FC->FSInterval);

    if (NewFSInterval != MSched.CurrentFSInterval)
      {
      if (MSched.CurrentFSInterval != 0)
        MFSUpdateData(FC,MSched.CurrentFSInterval,(1 << mfsactRotate));

      MSched.CurrentFSInterval = NewFSInterval;

      DBG(1,fFS) DPrint("INFO:     FS rolled to interval %ld\n",
        MSched.CurrentFSInterval);
      }
    }    /* END if (FC->FSPolicy != fspNONE) */

  MResAdjust(NULL,0,0);

  return(SUCCESS);
  }  /* END ServerProcessRequests() */




int ServerSetCreds(

  int UID,  /* I */
  int GID)  /* I */

  {
  /* must set GID first */

  if (MOSSetGID(GID) == -1)
    {
    fprintf(stderr,"ERROR:    cannot change GID to primary admin group '%s'  (GID: %d) %s\n",
      MUGIDToName(GID),
      GID,
      strerror(errno));
    }

  if (MOSSetUID(UID) == -1)
    {
    fprintf(stderr,"ERROR:    cannot change UID to primary admin user '%s'  (UID : %d) %s\n",
      MUUIDToName(UID),
      UID,
      strerror(errno));

    exit(1);
    }

  return(SUCCESS);
  }  /* END ServerSetCreds() */




int ServerAuthenticate()

  {
  int uid;
  int gid;

  int index;

  char  FileName[MAX_MLINE];
  FILE *fp;

  int ValidAdmin;

  const char *FName = "ServerAuthenticate";

  DBG(2,fALL) DPrint("%s()\n",
    FName);

  /* verify scheduler is being run by scheduler admin user */

  ValidAdmin = FALSE;

  uid = MOSGetUID();

  for (index = 0;MSched.Admin1User[index][0] != '\0';index++)
    {
    if (!strcmp(MSched.Admin1User[index],MUUIDToName(uid)))
      {
      ValidAdmin = TRUE;

      break;
      }
    }    /* END for (index) */

  if (ValidAdmin == FALSE)
    {
    fprintf(stderr,"ERROR:    user %s (UID: %d) is not authorized to run this program\n",
      MUUIDToName(uid),
      uid);

    fprintf(stderr,"INFO:     please go away.\n\n");

    exit(1);
    }

  /* run with UID of primary scheduler admin */

  if (MSched.SecureMode == TRUE)
    {
    uid = MUUIDFromName(MSched.Admin1User[0]);
    gid = MUGIDFromUID(uid);

    ServerSetCreds(uid,gid);

    DBG(3,fCORE) DPrint("INFO:     executing scheduler from '%s' under UID %d GID %d\n",
      MSched.HomeDir,
      uid,
      gid);
    }

  /* set up default umask value */

  umask(0077);

  /* prepend HomeDir if necessary */

  if (MSched.Mode != msmTest)
    {
    if ((MSched.LockFile[0] != '~') && (MSched.LockFile[0] != '/'))
      {
      sprintf(FileName,"%s%s",
        MSched.HomeDir,
        MSched.LockFile);
      }
    else
      {
      strcpy(FileName,MSched.LockFile);
      }

    if ((fp = fopen(FileName,"w+")) != NULL)
      {
      fprintf(fp,"%d",
        MOSGetPID());

      fclose(fp);
      }
    else
      {
      DBG(0,fALL) DPrint("ERROR:    cannot update lockfile '%s', errno: %d (%s)\n",
        FileName,
        errno,
        strerror(errno));
      }
    }    /* END if ((fp = fopen(FileName,"w+")) != NULL) */

  return(SUCCESS);
  }  /* END ServerAuthenticate() */



/* END Server.c */

