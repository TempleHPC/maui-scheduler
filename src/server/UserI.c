/* HEADER */

#include "moab.h"
#include "msched-proto.h"
 
long   UIDeadLine;
 
int    IgnoreToIteration = 0;
long   IgnoreToTime      = 0;

int MUIJobCtl(msocket_t *,long,char *);
int MUIShow(msocket_t *,long,char *);
int MUIQueueShow(msocket_t *,mxml_t *,mxml_t **);
int MUIJobSetAttr(mjob_t *,int,char *,int,char *,char *,char *,char *);
int MUIResCtl(msocket_t *,long,char *);
int MUIGridCtl(msocket_t *,long,char *);
int MUINodeCtl(msocket_t *,long,char *);
int MUIBal(msocket_t *,long,char *);

int __MUIJobToXML(mjob_t *,mxml_t **,int);
 
int (*Function[])(char *,char *,int,char *,long *) = {
  NULL,
  UIClusterShow,
  NULL,
  SetJobUserPrio,
  UIQueueShow,
  NULL,
  NULL,
  ShowJobHold,
  UIStatShow,
  UIStatClear,
  UIResCreate,
  UIResDestroy,
  UIResShow,
  MUISchedCtl,
  UIDiagnose,
  NULL,
  NULL,
  NULL,
  UIJobGetStart,
  NULL, 
  UIShowGrid,
  ShowBackfillWindow,
  UIShowConfig,
  UIJobShow,
  UINodeShow,
  UIJobStart,
  UIJobCancel,
  UIChangeParameter,
  NULL,
  UIShowEstStartTime,
  NULL,
  NULL };

int (*MCRequestF[])(msocket_t *,long,char *) = {
  NULL,
  NULL,
  NULL,
  MUIGridCtl,
  MUIJobCtl,
  MUINodeCtl,
  MUIResCtl,
  NULL,
  MUIShow,
  NULL,
  NULL };
 
char             CurrentHostName[MAX_MNAME];
 
extern mlog_t    mlog;
 
extern msocket_t   MClS[];

extern const char *MCBType[];
extern const char *MQALType[];
extern const char *MComp[];
extern const char *NodeName[];
extern const char *MService[];
extern const char *MClientCmd[];
extern const char *MHoldType[];
extern const char *MStatType[]; 
extern const char *MClientMode[];
extern const char *MJobAttr[];
extern const char *MFrameAttr[];
extern const char *MNodeAttr[];
 
extern const char *MPolicyMode[];
extern const char *MNAllocPolicy[];
extern const char *MTaskDistributionPolicy[];
extern const char *MBFPolicy[];
extern const char *MBFMPolicy[];
extern const char *MResFlags[];
extern const char *MResPolicy[];
extern const char *MResType[];
extern const char *MResThresholdType[];
extern const char *MCKeyword[];
extern const char *MAMType[];
extern const char *MAMProtocol[];
extern const char *MAMChargePolicy[];
extern const char *MRMType[];
extern const char *MRMSubType[];
extern const char *MRMAuthType[];
extern const char *MGridCtlCmds[];
extern const char *MJobCtlCmds[];
extern const char *MWikiJobAttr[]; 
extern const char *MWikiNodeAttr[];
extern const char *MJobNodeMatchType[];
extern const char *MResourceType[];
extern const char *MNAvailPolicy[];
extern const char *MDefReason[];
extern const char *MPolicyRejection[];
extern const char *MAllocRejType[];
extern const char *MNAccessPolicy[];
extern const char *MJobPrioAccrualPolicyType[]; 
extern const char *MBFPriorityPolicyType[];
extern const char *MNodeLoadPolicyType[];
extern const char *MResourceLimitPolicyType[];
extern const char *MFSPolicyType[];
extern const char *MResSetSelectionType[];
extern const char *MResSetAttrType[];
extern const char *MResSetPrioType[];
extern const char *MPreemptPolicy[];
extern const char *MNodeState[];
extern const char *MJobState[];
extern const char *MXO[]; 
extern const char *MHRObj[];
extern const char *MWeekDay[];
extern const char *MWEEKDAY[];
extern const char *MClusterAttr[];
extern const char *MJobDependType[];
 
extern const char *MJobFlags[];
 
extern mjob_t     *MJob[];
extern mnode_t    *MNode[];
extern mframe_t    MFrame[];
 
extern mqos_t      MQOS[];
extern mrm_t       MRM[];
extern mam_t       MAM[]; 
extern mattrlist_t MAList;
extern msys_t      MSys;
 
extern mrange_t    MRange[];
 
extern int         MAQ[];
extern int         MUIQ[];
extern msched_t    MSched;
extern mstat_t     MStat;
extern msim_t      MSim;
 
extern mgcred_t   *MUser[];
extern mgcred_t    MGroup[];
extern mgcred_t    MAcct[];
extern mclass_t    MClass[];
 
extern mrclass_t   MRClass[];
extern mres_t     *MRes[];
extern sres_t      SRes[];
extern sres_t      OSRes[];
 
extern mpar_t      MPar[];

extern mx_t        X;
 
#include "OUserI.c"

int __MUISSetStatus(msocket_t *, int);





int MUISProcessRequest(

  msocket_t *S,      /* I */
  char      *ErrMsg) /* O */

  {
  char SBuffer[MAX_SBUFFER];

  char tmpAuth[MAX_MLINE];

  int  CIndex;

  long AFlags;

  char *ptr;

  const char *FName = "MUISProcessRequest";

  DBG(3,fUI) DPrint("%s(S,ErrMsg)\n",
    FName);   

  if (S == NULL)
    {
    return(FAILURE);
    }

  /* get command */
 
  if ((ptr = strstr(S->RBuffer,MCKeyword[mckCommand])) == NULL)
    {
    DBG(3,fSOCK) DPrint("ALERT:    cannot locate command\n");

    strcpy(ErrMsg,"cannot locate command"); 
 
    return(FAILURE);
    }
 
  ptr += strlen(MCKeyword[mckCommand]);

  CIndex = MUGetIndex(ptr,MClientCmd,TRUE,mccNONE);
 
  if (MCRequestF[CIndex] == NULL)
    {
    DBG(3,fUI) DPrint("INFO:   service '%10.10s' not supported in %s\n",
      ptr,
      FName);

    snprintf(ErrMsg,MAX_MLINE,"cannot support command '%10.10s'",
      ptr);
 
    return(FAILURE);
    }

  if ((ptr = strstr(S->RBuffer,MCKeyword[mckArgs])) == NULL)
    {
    DBG(3,fSOCK) DPrint("ALERT:    cannot locate command args\n");
 
    strcpy(ErrMsg,"cannot locate command data");
 
    return(FAILURE);
    }

  ptr += strlen(MCKeyword[mckArgs]);

  S->RPtr = ptr;
 
  /* get authentication */
 
  if ((ptr = strstr(S->RBuffer,MCKeyword[mckAuth])) == NULL)
    {
    DBG(3,fSOCK) DPrint("ALERT:    cannot locate authentication\n");

    strcpy(ErrMsg,"cannot locate authentication");
 
    return(FAILURE);
    }
 
  ptr += strlen(MCKeyword[mckAuth]);
 
  MUSScanF(ptr,"%x%s",
    sizeof(tmpAuth),
    tmpAuth);

  ServerGetAuth(tmpAuth,&AFlags);

  S->SBuffer = SBuffer;
  S->SBufSize = sizeof(SBuffer);

  switch(CIndex)
    {
    default:

      {
      int SC;

      MUISMsgClear(S);

      SC = (*MCRequestF[CIndex])(S,AFlags,tmpAuth);

      __MUISSetStatus(S,SC);
 
      S->SBufSize = (long)strlen(S->SBuffer);
 
      MSUSendData(S,MAX_SOCKETWAIT,TRUE,TRUE);
      }  /* END BLOCK */

      break;
    }  /* END switch(CIndex) */

  return(SUCCESS);
  }  /* END MUISProcessRequest() */




int UIProcessClients(
 
  msocket_t *SS,        /* I */
  long       TimeLimit) /* I */
 
  {
  msocket_t  C;
  msocket_t *S;
 
  int    index;
  long   now;
 
  char   HostName[MAX_MNAME];
 
  int    RMDataStageInitiated;
  int    RMDataStaged;
 
  long   RMDataStageTime;
 
  const char *FName = "UIProcessClients";
 
  DBG(8,fUI) DPrint("%s(%d,%ld)\n",
    FName,
    (SS != NULL) ? SS->sd : -1,
    TimeLimit);
 
  if (SS == NULL)
    return(FAILURE);
 
  MUGetTime((mulong *)&now,mtmNONE,NULL);
 
  UIDeadLine = now + TimeLimit;
 
  RMDataStageInitiated = FALSE;
  RMDataStaged         = FALSE;
  RMDataStageTime      = -1;

  while ((now < UIDeadLine) ||
         (MSched.Schedule == FALSE) ||
         (RMDataStaged == FALSE) ||
        ((MSched.RMJobAggregationTime > 0) &&
         (MRM[0].LastSubmissionTime + MSched.RMJobAggregationTime > now)))
    {
    if (MRMCheckEvents() == SUCCESS)
      { 
      /* scheduling event occurred */
 
      UIDeadLine = now;
      }
 
    if (RMDataStageInitiated == FALSE)
      {
      if (((UIDeadLine - now) <= 5) && (X.XRMGetData != (int (*)())0))
        {
        /* initiate data stage 5 seconds early */
 
        if ((*X.XRMGetData)(X.xd,mcmNonBlock) == SUCCESS)
          {
          RMDataStageInitiated = TRUE;
          RMDataStageTime = now;
          }
        }
      else
        {
        /* no prestage available */
 
        RMDataStageInitiated = TRUE;
        RMDataStaged         = TRUE;
        }
      }
    else if (RMDataStaged == FALSE)
      {
      /* if data not staged */
 
      if (X.XRMDataIsStaging != (int (*)())0)
        {
        RMDataStaged = ((*X.XRMDataIsStaging)(X.xd,NULL) == TRUE) ? 
          FALSE : 
          TRUE;
 
        if ((RMDataStaged == FALSE) && ((now - RMDataStageTime) > 120))
          {
          /* data stage is taking too long */
 
          DBG(2,fUI) DPrint("ALERT:    data stage is hung, retrying\n");
 
          if ((*X.XRMGetData)(X.xd,mcmNonBlock) == SUCCESS)
            { 
            RMDataStageInitiated = TRUE;
            RMDataStageTime = now;
            }
          }
        }
      else
        {
        RMDataStaged = TRUE;
        }
      }
 
    if (MSched.Mode == msmSim)
      {
      if (IgnoreToTime != 0)
        {
        if (now < IgnoreToTime)
          {
          return(SUCCESS);
          }
        else
          {
          IgnoreToTime = 0;
          }
        }
 
      if (IgnoreToIteration != 0)
        {
        if (MSched.Iteration < IgnoreToIteration)
          {
          return(SUCCESS);
          }
        else
          {
          IgnoreToIteration = 0;
          }
        }
      }
 
    if (MSched.Schedule == FALSE)
      {
      MUSleep(50000);
      } 
 
    DBG(4,fUI)
      {
      if (!((UIDeadLine - now) % 60))
        {
        DBG(4,fUI) DPrint("INFO:     selecting next client (%ld seconds left)\n",
          (UIDeadLine - now));
        }
      }
    else if ((mlog.Threshold >= 9) && (MSched.CrashMode != TRUE))
      {
      DBG(9,fUI) DPrint("INFO:     selecting next client (%ld seconds left)\n",
        (UIDeadLine - now));
      }
 
    /* accept client connections */
 
    memset(&C,0,sizeof(C));
 
    while (MSUAcceptClient(
             SS,
             &C,
             HostName,
             (1 << TCP)) != FAILURE)
      {
      /* locate index for new client */
 
      for (index = 0;index < MAX_MCLIENT;index++)
        {
        S = &MClS[index];

        if (S->sd <= 0)
          {
          memset(S,0,sizeof(MClS[index]));

          S->sd = C.sd;

          if (C.SocketProtocol != 0)
            S->SocketProtocol = C.SocketProtocol;
          else
            S->SocketProtocol = MSched.DefaultMCSocketProtocol;

          DBG(5,fUI) DPrint("INFO:     client connected at sd %d\n",
            S->sd);
 
          break;
          } 
        }    /* END for (index) */
 
      if (index >= MAX_MCLIENT)
        {
        DBG(2,fUI) DPrint("WARNING:  cannot accept client from '%s' (MaxClient reached)\n",
          HostName);
 
        MSUDisconnect(&C);
        }
      else
        {
        DBG(3,fUI) DPrint("INFO:     client socket from '%s' accepted\n",
          HostName);
        }
      }   /* END while (MSUAcceptClient() != FAILURE) */
 
    if (MSched.CrashMode != TRUE)
      DBG(9,fUI) DPrint("INFO:     all clients connected.  servicing requests\n");
 
    /* service clients */
 
    for (index = 0;index < MAX_MCLIENT;index++)
      {
      S = &MClS[index];

      DBG(11,fUI) DPrint("INFO:     checking read status of client[%d]  (%d)\n",
        index,
        S->sd);
 
      if (S->sd > 0)
        {
        /* read data in socket */
 
        if (UIProcessCommand(S) == FAILURE)
          {
          DBG(6,fUI) DPrint("INFO:     could not service client request %d\n",
            index);
          }

        if (MSched.Shutdown == TRUE)
	  {
	  DBG(1,fUI) DPrint("INFO:     shutting scheduler down (user request)\n");

          MUSleep(1000000);

          MSysShutdown(0);
          }

        MSUFree(S); 
        }
      }    /* END for (index) */
 
    if ((MSched.Mode == msmSim) &&
        (MSched.TimePolicy != mtpReal) &&
        (X.XSleep == (int (*)())0) &&
        (MSched.Schedule == TRUE) &&
        (MSched.CrashMode != TRUE))
      {
      /* resume simulation immediately */
 
      break;
      }
    else
      {
      /* sleep 100 us */
 
      MUSleep(100000);
 
      MUGetTime((mulong *)&now,mtmNONE,NULL);
      }
    }       /* END while (MSched.Time) */
 
  return(SUCCESS);
  }  /* END UIProcessClients() */





int UIUserDiagnose(
 
  char *Buffer,  /* O */
  long *BufSize, /* I/O */
  char *UName,   /* I */
  int   IFlags)  /* I */
 
  {
  int      uindex;
 
  mgcred_t  *U;

  const char *FName = "UIUserDiagnose";
 
  DBG(3,fUI) DPrint("%s(Buffer,BufSize,%s)\n",
    FName,
    (UName != NULL) ? UName : "NULL");
 
  MUStrCpy(Buffer,"Displaying user information...\n",MAX_MBUFFER);
 
  /* create header */
 
  MUStrCat(
    Buffer,
    MUserShow(NULL,NULL,NULL,IFlags),
    MAX_MBUFFER);
 
  for (uindex = 0;uindex < MAX_MUSER + MAX_MHBUF;uindex++)
    {
    U = MUser[uindex];
 
    if ((U == NULL) || (U->Name[0] == '\0') || (U->Name[0] == '\1'))
      continue;

    if (!strcmp(U->Name,ALL))
      continue;
 
    DBG(8,fUI) DPrint("INFO:     checking User[%04d]: %s\n",
      uindex,
      U->Name);
 
    if ((UName != NULL) && strcmp(UName,NONE) && strcmp(UName,U->Name))
      continue;
 
    MUStrCat(
      Buffer,
      MUserShow(U,NULL,NULL,IFlags),
      MAX_MBUFFER);
    }  /* END for (uindex) */
 
  return(SUCCESS);
  }   /* END UIUserDiagnose() */





int UIGroupDiagnose(

  char *Buffer,  /* O */
  long *BufSize, /* I */
  char *GName,   /* I */
  int   IFlags)  /* I */

  {
  int      gindex;

  mgcred_t  *G;

  const char *FName = "UIGroupDiagnose";

  DBG(3,fUI) DPrint("%s(Buffer,BufSize,%s,%d)\n",
    FName,
    (GName != NULL) ? GName : "NULL",
    IFlags);

  MUStrCpy(Buffer,"Displaying group information...\n",MAX_MBUFFER);

  /* create header */

  MUStrCat(
    Buffer,
    MGroupShow(NULL,NULL,NULL,IFlags),
    MAX_MBUFFER);

  for (gindex = 0;gindex < MAX_MGROUP + MAX_MHBUF;gindex++)
    {
    G = &MGroup[gindex];

    if ((G == NULL) || (G->Name[0] == '\0') || (G->Name[0] == '\1'))
      continue;

    if (!strcmp(G->Name,ALL) || !strcmp(G->Name,"NOGROUP"))
      continue;

    DBG(8,fUI) DPrint("INFO:     checking Group[%04d]: %s\n",
      gindex,
      G->Name);

    if ((GName != NULL) && strcmp(GName,NONE) && strcmp(GName,G->Name))
      continue;

    MUStrCat(
      Buffer,
      MGroupShow(G,NULL,NULL,IFlags),
      MAX_MBUFFER);
    }  /* END for (gindex) */

  return(SUCCESS);
  }   /* END UIGroupDiagnose() */





int UIAcctDiagnose(

  char *Buffer,   /* O */
  long *BufSize,  /* I */
  char *AName,    /* I */
  int   IFlags)   /* I */

  {
  int      aindex;

  mgcred_t *A;

  const char *FName = "UIAcctDiagnose";

  DBG(3,fUI) DPrint("%s(Buffer,BufSize,%s,%d)\n",
    FName,
    (AName != NULL) ? AName : "NULL",
    IFlags);

  MUStrCpy(Buffer,"Displaying account information...\n",MAX_MBUFFER);

  /* create header */

  MUStrCat(
    Buffer,
    MAcctShow(NULL,NULL,NULL,IFlags),
    MAX_MBUFFER);

  for (aindex = 0;aindex < MAX_MACCT + MAX_MHBUF;aindex++)
    {
    A = &MAcct[aindex];

    if ((A == NULL) || (A->Name[0] == '\0') || (A->Name[0] == '\1'))
      continue;

    if (!strcmp(A->Name,ALL))
      continue;

    DBG(8,fUI) DPrint("INFO:     checking Acct[%04d]: %s\n",
      aindex,
      A->Name);

    if ((AName != NULL) && strcmp(AName,NONE) && strcmp(AName,A->Name))
      continue;

    MUStrCat(
      Buffer,
      MAcctShow(A,NULL,NULL,IFlags),
      MAX_MBUFFER);
    }  /* END for (aindex) */

  return(SUCCESS);
  }   /* END UIAcctDiagnose() */




int UIJobShow(

  char *RBuffer,  /* I */
  char *SBuffer,  /* O */
  int   AFlags,   /* I */
  char *Auth,     /* I */
  long *SBufSize) /* I/O */
 
  {
  int      Flags;
 
  int      index;
  int      nindex;
  int      rqindex;
  int      tindex;
  int      pindex;
  int      rindex;
  int      sindex;

  int      TaskCount;         
  int      PLevel;      

  mnode_t *N;
  mjob_t  *J;
  mreq_t  *RQ;
  mpar_t  *P;

  char    *ptr;

  char     JobName[MAX_MNAME];
  char     Duration[MAX_MNAME];
  char     ProcLine[MAX_MNAME];
  char     Line[MAX_MLINE]; 
  char     tmpBuffer[MAX_MBUFFER];    
  char     ResList[MAX_MLINE];          
  char     StartTime[MAX_MNAME];
  char     EndTime[MAX_MNAME];
  char     NodeName[MAX_MLINE];

  int      PReason;
  int      Reason[MAX_MREJREASON];
 
  int      ncount;
  int      pcount;
 
  double MinSpeed;
  double PE;
 
  int   TasksAllowed;
  int   IProcs;
 
  mbool_t IsDeferred;
 
  char  MsgBuf[MAX_MBUFFER];

  char *BPtr;
  int   BSpace;

  const char *FName = "UIJobShow";
 
  DBG(2,fUI) DPrint("%s(RBuffer,SBuffer,%d,%s,BufSize)\n",
    FName,
    AFlags,
    Auth);

  MUSScanF(RBuffer,"%d %x%s %d %x%s %x%s",
    &PLevel,
    sizeof(JobName),
    JobName,
    &Flags,
    sizeof(ResList),
    ResList,
    sizeof(NodeName),
    NodeName);
 
  if (MJobFind(JobName,&J,0) != SUCCESS)
    {
    sprintf(SBuffer,"ERROR:  cannot locate job '%s'\n",
      JobName);
 
    DBG(3,fUI) DPrint("INFO:     cannot locate job '%s' in UIJobShow()\n",
      JobName);
 
    return(FAILURE);
    }
 
  /* security check */
 
  if (!(AFlags & ((1 << fAdmin1) | (1 << fAdmin2) | (1 << fAdmin3))))
    {
    if (strcmp(J->Cred.U->Name,Auth) != 0)
      {
      DBG(2,fUI) DPrint("INFO:     user %s is not authorized to check status of job '%s'\n",
        Auth,
        J->Name);
 
      sprintf(SBuffer,"user %s is not authorized to check status of job '%s'\n",
        Auth,
        J->Name);
 
      return(FAILURE);
      }
    } 
 
  SBuffer[0] = '\0';

  BPtr   = SBuffer;
  BSpace = *SBufSize;
 
  RQ = J->Req[0];
 
  /* display job state */
 
  if (Flags & (1 << mcmParse))
    {
    /* FORMAT:      JNAME STATE UNAME GNAME ANAME CNAME  WCLMT  QTIME  STIME  TCNT  NCNT */
 
    MUSNPrintF(&BPtr,&BSpace,"%s=%s;%s=%s;%s=%s;%s=%s;%s=%s;%s=%s;%s=%ld;%s=%ld;%s=%ld;%s=%d;%s=%d;",
      MWikiJobAttr[mwjaName],
      J->Name,
      MWikiJobAttr[mwjaState],
      MJobState[J->State],
      MWikiJobAttr[mwjaUName],
      (J->Cred.U != NULL) ? J->Cred.U->Name : NONE,
      MWikiJobAttr[mwjaGName],
      (J->Cred.G != NULL) ? J->Cred.G->Name : NONE,
      MWikiJobAttr[mwjaAccount],
      (J->Cred.A != NULL) ? J->Cred.A->Name : NONE,
      MWikiJobAttr[mwjaRClass],
      (J->Cred.C != NULL) ? J->Cred.C->Name : NONE,
      MWikiJobAttr[mwjaWCLimit],
      J->WCLimit,
      MWikiJobAttr[mwjaQueueTime],
      J->SubmitTime,
      MWikiJobAttr[mwjaStartTime],
      J->StartTime,
      MWikiJobAttr[mwjaTasks],
      J->Request.TC,
      MWikiJobAttr[mwjaNodes],
      J->Request.NC);

    if (J->Flags != 0)
      {
      Line[0] = '\0'; 
 
      for (index = 0;MJobFlags[index] != NULL;index++)
        {
        if (J->Flags & (1 << index))
          {
          if (Line[0] != '\0')
            strcat(Line,";");
 
          strcat(Line,MJobFlags[index]);
 
          if ((index == mjfAdvReservation) && (J->ResName[0] != '\0'))
            {
            strcat(Line,":");
            strcat(Line,J->ResName);
            }
          }
        }    /* END for (index) */
 
      if (Line[0] == '\0')
        strcpy(Line,NONE);
      }  /* END if (J->Flags != 0) */
 
    MUSNPrintF(&BPtr,&BSpace,"%s=%s;%s=%s;\n",
      MWikiJobAttr[mwjaRFeatures],
      MUMAList(eFeature,RQ->ReqFBM,sizeof(RQ->ReqFBM)),
      MWikiJobAttr[mwjaFlags],
      Line);

    return(SUCCESS);
    }  /* END if (Flags & (1 << mcmParse)) */

  if ((Flags & (1 << mcmVerbose)) && 
      (J->RMJID != NULL) &&
      (strcmp(J->Name,J->RMJID)))
    {
    MUSNPrintF(&BPtr,&BSpace,"checking job %s (RM job '%s')\n\n",
      J->Name,
      J->RMJID);
    }
  else
    {
    MUSNPrintF(&BPtr,&BSpace,"checking job %s\n\n",
      J->Name); 
    }

  DBG(1,fUI) DPrint("job '%s'  State: %11s  EState:  %9s   QueueTime: %26s",
    J->Name,
    MJobState[J->State],
    MJobState[J->EState],
    MULToDString((mulong *)&J->SubmitTime));
 
  if (J->AName != NULL)
    {
    MUSNPrintF(&BPtr,&BSpace,"AName: %s\n",
      J->AName);
    }
 
  MUSNPrintF(&BPtr,&BSpace,"State: %s%s%s\n",
    MAList[eJobState][J->State],
    (J->EState != J->State) ? "  EState: ": "",
    (J->EState != J->State) ? MAList[eJobState][J->EState] : "");

  MUSNPrintF(&BPtr,&BSpace,"Creds:%s%s%s%s%s%s%s%s%s%s\n",
    (J->Cred.U != NULL) ? "  user:" : "",
    (J->Cred.U != NULL) ? J->Cred.U->Name : "",
    (J->Cred.G != NULL) ? "  group:" : "",
    (J->Cred.G != NULL) ? J->Cred.G->Name : "",
    (J->Cred.A != NULL) ? "  account:" : "",
    (J->Cred.A != NULL) ? J->Cred.A->Name : "",
    (J->Cred.C != NULL) ? "  class:" : "",
    (J->Cred.C != NULL) ? J->Cred.C->Name : "",
    (J->Cred.Q != NULL) ? "  qos:" : "",
    (J->Cred.Q != NULL) ? J->Cred.Q->Name : "");

  strcpy(Duration,MULToTString(J->AWallTime));
 
  if (MSched.Mode == msmSim)
    {
    sprintf(Line,"  (Recorded WallTime: %s)",
      MULToTString(J->SimWCTime));
    }
  else
    {
    Line[0] = '\0'; 
    }
 
  MUSNPrintF(&BPtr,&BSpace,"WallTime: %s of %s%s\n",
    Duration,
    MULToTString(J->SpecWCLimit[0]),
    Line);

  if (J->WCLimit != J->SpecWCLimit[0])
    {
    MUSNPrintF(&BPtr,&BSpace,"Adjusted WCLimit: %s\n",
      MULToTString(J->WCLimit));
    }
 
  if (J->SWallTime > 0)
    {
    MUSNPrintF(&BPtr,&BSpace,"Suspended Wall Time: %s\n",
      MULToTString(J->SWallTime));
    }
 
  if (J->SpecWCLimit[2] > 0)
    {
    strcpy(Line,"SpecWCLimit: ");
 
    for (tindex = 0;J->SpecWCLimit[tindex] > 0;tindex++)
      {
      if (tindex > 0)
        strcat(Line,",");
 
      sprintf(temp_str," %ld",
        J->SpecWCLimit[tindex]);
      strcat(Line,temp_str);
      }  /* END for (tindex) */
 
    MUSNPrintF(&BPtr,&BSpace,"%s",
      Line); 
    }
 
  {
  long TQTime;

  char tmpLine[MAX_MNAME];

  if ((J->State == mjsStarting) || (J->State == mjsRunning))
    TQTime = J->StartTime - J->SubmitTime;
  else
    TQTime = MSched.Time - J->SubmitTime;

  strcpy(tmpLine,MULToTString(TQTime));

  MUSNPrintF(&BPtr,&BSpace,"SubmitTime: %s  (Time Queued  Total: %s  Eligible: %s)\n\n",
    MULToDString((mulong *)&J->SubmitTime),
    tmpLine,
    MULToTString(J->EffQueueDuration));
  }  /* END BLOCK */

  if ((J->State == mjsStarting) || (J->State == mjsRunning))
    {
    MUSNPrintF(&BPtr,&BSpace,"StartTime: %s",
      MULToDString((mulong *)&J->StartTime));
    }
 
  if (J->SMinTime != 0)
    {
    MUSNPrintF(&BPtr,&BSpace,"StartDate: %s  %s",
      MULToTString(J->SMinTime - MSched.Time),
      MULToDString((mulong *)&J->SMinTime));
    }
 
  MUSNPrintF(&BPtr,&BSpace,"Total Tasks: %d\n",
    J->Request.TC); 

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];
 
    MUSNPrintF(&BPtr,&BSpace,"\nReq[%d]  TaskCount: %d  Partition: %s\n",
      rqindex,
      RQ->TaskCount,
      MAList[ePartition][RQ->PtIndex]);

    if (RQ->TaskRequestList[2] > 0)
      {
      strcpy(Line,"TaskRequestList: ");
 
      for (tindex = 1;RQ->TaskRequestList[tindex] > 0;tindex++)
        {
        if (tindex > 0)
          strcat(Line,",");
 
        sprintf(temp_str," %d",
          RQ->TaskRequestList[tindex]);
        strcat(Line,temp_str);
        }  /* END for (tindex) */
 
      MUSNPrintF(&BPtr,&BSpace,"%s",
        Line);
      }
 
    if ((J->MReq != NULL) && (J->MReq->Index == rqindex))
      {
      MUSNPrintF(&BPtr,&BSpace,"(MasterReq)\n");
      }
 
    if (RQ->RequiredProcs > 0)
      {
      sprintf(ProcLine,"Procs %s%d  ",
        MComp[RQ->ProcCmp],
        RQ->RequiredProcs); 
      }
    else
      {
      ProcLine[0] = '\0';
      }

    {
    char tmpBuf1[MAX_MNAME];
    char tmpBuf2[MAX_MNAME];
    char tmpBuf3[MAX_MNAME];
 
    MUSNPrintF(&BPtr,&BSpace,"Network: %s  %sMemory %s %s  Disk %s %s  Swap %s %s\n",
      MAList[eNetwork][RQ->Network],
      ProcLine,
      MComp[RQ->MemCmp],
      MULToRSpec((long)RQ->RequiredMemory,mvmMega,tmpBuf1),
      MComp[RQ->DiskCmp],
      MULToRSpec((long)RQ->RequiredDisk,mvmMega,tmpBuf2),
      MComp[RQ->SwapCmp],
      MULToRSpec((long)RQ->RequiredSwap,mvmMega,tmpBuf3));
    }  /* END BLOCK */

    MUSNPrintF(&BPtr,&BSpace,"Opsys: %s  Arch: %s  Features: %s\n",
      MAList[eOpsys][RQ->Opsys],
      MAList[eArch][RQ->Arch],
      MUMAList(eFeature,RQ->ReqFBM,sizeof(RQ->ReqFBM)));

    ptr = MUMAList(eFeature,RQ->PrefFBM,sizeof(RQ->PrefFBM));

    if (strcmp(ptr,NONE) != 0)
      {
      MUSNPrintF(&BPtr,&BSpace,"Preferences: %s\n",
        ptr);
      }

    if (Flags & (1 << mcmVerbose))
      {
      MUSNPrintF(&BPtr,&BSpace,"Exec:  '%s'  ExecSize: %d  ImageSize: %d\n",
        ((J->E.Cmd != NULL) && strcmp(J->E.Cmd,NONE)) ? J->E.Cmd : "",
        J->ExecSize,
        J->ImageSize);
      }
 
    if (RQ->Pool > '\0')
      {
      MUSNPrintF(&BPtr,&BSpace,"Pool:  %d\n",
        (int)(RQ->Pool - 1));
      } 
 
    if ((Flags & (1 << mcmVerbose)) ||
        (RQ->DRes.Procs != 1) ||
        (RQ->DRes.Mem > 0) ||
        (RQ->DRes.Disk > 0))
      {
      MUSNPrintF(&BPtr,&BSpace,"Dedicated Resources Per Task: %s\n",
        MUCResToString(&RQ->DRes,0,0,NULL));
      }
 
    if (((J->State == mjsStarting) || (J->State == mjsRunning)) &&
         (Flags & (1 << mcmVerbose)))
      {
      ptr = MUCResToString(&RQ->URes,0,1,NULL);
 
      if (ptr[0] != '\0')
        {
        MUSNPrintF(&BPtr,&BSpace,"Utilized Resources Per Task:  %s\n",
          ptr);
        }
 
      ptr = MUCResToString(&RQ->LURes,RQ->RMWTime,1,NULL);
 
      if (ptr[0] != '\0')
        {
        MUSNPrintF(&BPtr,&BSpace,"Avg Util Resources Per Task:  %s\n",
          ptr);
        }
 
      ptr = MUCResToString(&RQ->MURes,0,1,NULL);
 
      if (ptr[0] != '\0')
        {
        MUSNPrintF(&BPtr,&BSpace,"Max Util Resources Per Task:  %s\n",
          ptr);
        }
      }    /* END if ((J->State == mjsStarting) || ,,, )) */
 
    if ((J->AWallTime > 0) && (Flags & (1 << mcmVerbose)))
      {
      if (J->MSUtilized > 0.01)
        {
        MUSNPrintF(&BPtr,&BSpace,"Average Utilized Memory: %.2lf MB\n",
          J->MSUtilized / J->AWallTime);
        }
 
      if (J->PSUtilized > 0.01)
        {
        MUSNPrintF(&BPtr,&BSpace,"Average Utilized Procs: %.2lf\n",
          J->PSUtilized / J->AWallTime);
        }
      }
 
    if (((J->State == mjsStarting) || (J->State == mjsRunning)) &&
         (Flags & (1 << mcmVerbose)) &&
         (J->Ckpt != NULL))
      {
      char tmpLine[MAX_MLINE];
 
      strcpy(tmpLine,MULToTString(J->Ckpt->ActiveCPDuration)); 
 
      MUSNPrintF(&BPtr,&BSpace,"Ckpt Walltime:  current: %s   stored: %s\n",
        tmpLine,
        MULToTString(J->Ckpt->StoredCPDuration));
      }
 
    if (J->SystemID != NULL)
      {
      MUSNPrintF(&BPtr,&BSpace,"SystemID:  %s\n",
        J->SystemID);
      }

    if (J->SystemJID != NULL)
      {
      MUSNPrintF(&BPtr,&BSpace,"SystemJID:  %s\n",
        J->SystemJID);
      }
 
    if ((RQ->SetSelection != mrssNONE) ||
        (RQ->SetType != mrstNONE) ||
        (RQ->SetList[0] != NULL))
      {
      MUSNPrintF(&BPtr,&BSpace,"NodeSet=%s:%s:",
        MResSetSelectionType[RQ->SetSelection],
        MResSetAttrType[RQ->SetType]);
 
      for (sindex = 0;RQ->SetList[sindex] != NULL;sindex++)
        {
        if (sindex > 0)
          MUStrNCat(&BPtr,&BSpace,":");
 
        MUStrNCat(&BPtr,&BSpace,RQ->SetList[sindex]);
        }
 
      if (sindex == 0)
        MUStrNCat(&BPtr,&BSpace,NONE);

      MUStrNCat(&BPtr,&BSpace,"\n");
      }  /* END if ((RQ->SetSelection != mrssNONE) || ...) */
 
    if (Flags & (1 << mcmVerbose)) 
      {
      MUSNPrintF(&BPtr,&BSpace,"NodeAccess: %s\n",
	MNAccessPolicy[RQ->NAccessPolicy]);
 
      if (RQ->TasksPerNode > 0)
        {
        MUSNPrintF(&BPtr,&BSpace,"TasksPerNode: %d  ",
          RQ->TasksPerNode);
        }
      }
 
    if (((RQ->NodeCount > 0) &&
        ((RQ->NodeCount * RQ->TasksPerNode) != RQ->TaskCount)) ||
         (Flags & (1 << mcmVerbose)))
      {
      MUSNPrintF(&BPtr,&BSpace,"NodeCount: %d\n",
        RQ->NodeCount);
      }

    if (MJOBISALLOC(J) == TRUE)
      {
      if ((RQ->PtIndex < 1) && (RQ->NodeList[0].N != MSched.GN))
        {
        MUSNPrintF(&BPtr,&BSpace,"ALERT:    partition %d is invalid\n",
          RQ->PtIndex);
        }
 
      MUStrNCat(&BPtr,&BSpace,"Allocated Nodes:\n");
 
      for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
        {
        N = RQ->NodeList[nindex].N;
 
        MUSNPrintF(&BPtr,&BSpace,"[%.20s:%d]",
          N->Name,
          RQ->NodeList[nindex].TC);
 
        if ((nindex % 4) == 3)
          MUStrNCat(&BPtr,&BSpace,"\n");
        }    /* END for (nindex)               */
 
      MUStrNCat(&BPtr,&BSpace,"\n");

      /* show taskmap */

      if (Flags & (1 << mcmVerbose))
        {
        MUStrNCat(&BPtr,&BSpace,"Task Distribution: ");

        for (tindex = 0;J->TaskMap[tindex] != -1;tindex++)
          {
          if (tindex > 10)
            {
            MUSNPrintF(&BPtr,&BSpace,",...");

            break;
            }

          if (tindex > 0)
            {
            MUSNPrintF(&BPtr,&BSpace,",%.20s",
              MNode[J->TaskMap[tindex]]->Name);
            }
          else
            {
            MUSNPrintF(&BPtr,&BSpace,"%.20s",
              MNode[J->TaskMap[tindex]]->Name);
            }
          }  /* END for (tindex) */

        MUStrNCat(&BPtr,&BSpace,"\n");
        }    /* END if (Flags & (1 << mcmVerbose)) */
 
      /* diagnose nodes */
 
      TaskCount = 0;
 
      for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
        {
        N = RQ->NodeList[nindex].N;
 
        TaskCount += RQ->NodeList[nindex].TC;
 
        if ((N->State == mnsDown) ||
           ((N->State == mnsIdle) && (MSched.Time - J->StartTime > 120))) 
          {
          MUSNPrintF(&BPtr,&BSpace,"WARNING:  allocated node %16s is in state %s\n",
            N->Name,
            MNodeState[N->State]);
          }
        }    /* END for (nindex)  */
 
      if ((long)TaskCount != RQ->TaskCount)
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  allocated tasks do not match requested tasks (%d != %d)\n",
          TaskCount,
          RQ->TaskCount);
        }
      }      /* END if (J->State == mjsStarting) */
    }        /* END for (rqindex) */
 
  if (J->ASFunc != NULL)
    {
    (*J->ASFunc)(J,mascShow,NULL,(void **)tmpBuffer);

    MUStrNCat(&BPtr,&BSpace,tmpBuffer);
    }
 
  MUStrNCat(&BPtr,&BSpace,"\n");
 
  if ((J->xd != NULL) && (X.XJobShow != (int (*)())0))
    {
    (*X.XJobShow)(X.xd,J,BPtr);

    BSpace -= strlen(BPtr);
    BPtr   += strlen(BPtr);
    }
 
  MUStrNCat(&BPtr,&BSpace,"\n");

  /* display job executable information */
 
  MUSNPrintF(&BPtr,&BSpace,"IWD: %s  Executable:  %s\n",
    (J->E.IWD != NULL) ? J->E.IWD : NONE,
    (J->E.Cmd != NULL) ? J->E.Cmd : NONE);
 
  MUSNPrintF(&BPtr,&BSpace,"Bypass: %d  StartCount: %d\n",
    J->Bypass,
    J->StartCount);

  if (J->PAL[0] != 0)
    {
    MUSNPrintF(&BPtr,&BSpace,"PartitionMask: %s\n",
      (J->PAL[0] == GLOBAL_PARTITIONLIST) ?
        ALL : MUListAttrs(ePartition,J->PAL[0]));
    }
  else
    {
    MUSNPrintF(&BPtr,&BSpace,"WARNING:  job has invalid partition mask\n",
      J->Name);
    }
 
  if (Flags & (1 << mcmVerbose))
    {
    if ((MRM[1].Type != mrmtNONE) && (J->RM != NULL))
      {
      MUSNPrintF(&BPtr,&BSpace,"ResourceManager[%s]: %s\n",
        J->RM->Name,
        MRMType[J->RM->Type]);
      }
 
    if (J->SystemQueueTime != J->SubmitTime)
      {
      MUSNPrintF(&BPtr,&BSpace,"SystemQueueTime: %20s\n",
        MULToDString((mulong *)&J->SystemQueueTime));
      }
    }
 
  if (J->Flags != 0)
    {
    MUStrNCat(&BPtr,&BSpace,"Flags:      ");
 
    for (index = 0;MJobFlags[index] != NULL;index++)
      {
      if (J->Flags & (1 << index))
        {
        MUSNPrintF(&BPtr,&BSpace," %s",
          MJobFlags[index]);
 
        if ((index == mjfAdvReservation) && (J->ResName[0] != '\0'))
          {
          MUStrNCat(&BPtr,&BSpace,":");
          MUStrNCat(&BPtr,&BSpace,J->ResName);
          }
        }
      }    /* END for (index) */
 
    MUStrNCat(&BPtr,&BSpace,"\n");

    if (J->AttrBM != 0)
      {
      MUStrNCat(&BPtr,&BSpace,"Attr:       ");  

      for (index = 0;index < 32;index++)
        {
        if (J->AttrBM & (1 << index))
          {
          MUSNPrintF(&BPtr,&BSpace," %s",
            MAList[eJFeature][index]);
          }
        }  /* END for (index) */

      MUStrNCat(&BPtr,&BSpace,"\n");        
      }    /* END if (J->AttrBM != 0) */
 
    if ((J->Flags & (1 << mjfHostList)) && (J->ReqHList != NULL))
      {
      if (J->ReqHList[0].N == NULL)
        {
        MUStrNCat(&BPtr,&BSpace,"WARNING:  empty hostlist specified\n");
        }
      else
        {
        MUStrNCat(&BPtr,&BSpace,"HostList: ");
   
        if (J->ReqHLMode == mhlmSubset)
          MUStrNCat(&BPtr,&BSpace," (SUBSET)\n  ");
        else
          MUStrNCat(&BPtr,&BSpace,"\n  ");
 
        for (nindex = 0;J->ReqHList[nindex].N != NULL;nindex++)
          {
          N = J->ReqHList[nindex].N;
 
          MUSNPrintF(&BPtr,&BSpace,"[%.20s:%d]",
            N->Name,
            J->ReqHList[nindex].TC);
 
          if ((nindex % 4) == 3)
            MUStrNCat(&BPtr,&BSpace,"\n  ");
          }  /* END for (nindex) */
        }
      }      /* END if ((J->Flags & (1 << mjfHostList)) && ...) */

    if (J->ExcHList != NULL)
      {
      if (J->ExcHList[0].N == NULL)
        {
        MUStrNCat(&BPtr,&BSpace,"WARNING:  empty hostlist specified\n");
        }
      else
        {
        MUStrNCat(&BPtr,&BSpace,"Exclude HostList:\n");

        for (nindex = 0;J->ExcHList[nindex].N != NULL;nindex++)
          {
          N = J->ExcHList[nindex].N;

          MUSNPrintF(&BPtr,&BSpace,"[%.20s:%d]",
            N->Name,
            J->ExcHList[nindex].TC);

          if ((nindex % 4) == 3)
            MUStrNCat(&BPtr,&BSpace,"\n");
          }  /* END for (nindex) */
        }
      }      /* END if (J->ExcHList != NULL) */
 
    MUStrNCat(&BPtr,&BSpace,"\n");
    } /* END if (J->Flags != 0) */
 
  /* evaluate reservation access */
 
  if (strcmp(ResList,NONE) && (ResList[0] != '\0'))
    {
    char *ptr;
    char *TokPtr;
 
    char *RName;
 
    mres_t *R;
 
    ptr = MUStrTok(ResList,",",&TokPtr);
 
    while (ptr != NULL)
      {
      RName = ptr;
 
      ptr = MUStrTok(NULL,",",&TokPtr);
 
      if (MResFind(RName,&R) == FAILURE)
        {
        MUSNPrintF(&BPtr,&BSpace,"\nNOTE:  cannot locate reservation '%s'\n",
          RName);
 
        continue; 
        }
 
      if (MResCheckJAccess(R,J,J->WCLimit,NULL,NULL) == SUCCESS)
        {
        MUSNPrintF(&BPtr,&BSpace,"\nNOTE:  job granted access to reservation '%s'\n",
          RName);
        }
      else
        {
        MUSNPrintF(&BPtr,&BSpace,"\nNOTE:  job denied access to reservation '%s'\n",
          RName);
        }
      }
    }  /* END if (strcmp(ResList,NONE) && (ResList[0] != '\0')) */
 
  /* display defer info/state discrepancies */
 
  if (J->EState == mjsDeferred)
    {
    MUSNPrintF(&BPtr,&BSpace,"job is deferred.  Reason:  %s  (%s)\n",
      MDefReason[J->HoldReason],
      (J->Message != NULL) ? J->Message : "");
    }
  else if ((J->State != J->EState) &&
          ((J->State != mjsRunning) ||
           (J->EState != mjsStarting)))
    {
    MUSNPrintF(&BPtr,&BSpace,"EState '%s' does not match current state '%s'\n",
      MJobState[J->EState],
      MJobState[J->State]);
    } 

  /* display reservation */
 
  if (J->R != NULL)
    {
    strcpy(StartTime,MULToTString(J->R->StartTime - MSched.Time));
    strcpy(EndTime,MULToTString(J->R->EndTime - MSched.Time));
    strcpy(Duration,MULToTString(J->R->EndTime - J->R->StartTime));
 
    MUSNPrintF(&BPtr,&BSpace,"Reservation '%s' (%s -> %s  Duration: %s)\n",
      J->R->Name,
      StartTime,
      EndTime,
      Duration);
 
    if (((J->State == mjsStarting) || (J->State == mjsRunning)) &&
         (J->StartTime > 0) &&
         (J->StartTime != J->R->StartTime))
      {
      MUSNPrintF(&BPtr,&BSpace,"WARNING:  reservation does not match job start time (%ld != %ld)\n",
        J->R->StartTime,
        J->StartTime);
      }
    }
  else if ((J->State == mjsStarting) || (J->State == mjsRunning))
    {
    MUSNPrintF(&BPtr,&BSpace,"WARNING:  active job has no reservation\n");
    }  /* END else if ((J->State == mjsStarting) || (J->State == mjsRunning)) */

  /* check update time */
 
  if ((MSched.Time - J->ATime) > MAX(300,2 * MSched.RMPollInterval))
    {
    MUSNPrintF(&BPtr,&BSpace,"WARNING:  job has not been detected in %s\n",
      MULToTString(MSched.Time - J->ATime));
    }
 
  /* display job holds */
 
  if (J->Hold != 0)
    {
    int HList[] = { mhBatch, mhSystem, mhUser, mhDefer, -1 };
    int hindex;
  
    int HoldFound = FALSE;
 
    MUStrNCat(&BPtr,&BSpace,"Holds:    ");

    for (hindex = 0;HList[hindex] != -1;hindex++)
      {
      if (J->Hold & (1 << HList[hindex]))
        {
        MUStrNCat(&BPtr,&BSpace,(char *)MHoldType[HList[hindex]]);
        MUStrNCat(&BPtr,&BSpace,"  ");

        HoldFound = TRUE;
        }
      }    /* END for (hindex) */ 
 
    if ((HoldFound == TRUE) && (J->HoldReason != 0))
      {
      MUSNPrintF(&BPtr,&BSpace,"(hold reason:  %s)",
        MDefReason[J->HoldReason]);
      }

    MUStrNCat(&BPtr,&BSpace,"\n");
    }    /* END if (J->Hold != 0) */

  
  /* show job messages */

  if ((J->Message != NULL) && (J->EState != mjsDeferred))
    {
    MUSNPrintF(&BPtr,&BSpace,"Messages:  %s\n",
      J->Message);
    }

  /* display priority info */
 
  if (J->SystemPrio != 0)
    {
    sprintf(Line,"  SystemPriority:  %ld\n",
      J->SystemPrio);
    }
  else
    {
    Line[0] = '\0';
    }

  MJobGetPE(J,&MPar[0],&PE);
 
  MUSNPrintF(&BPtr,&BSpace,"PE:  %.2lf  StartPriority:  %ld%s\n",
    PE,
    J->StartPriority,
    Line);

  if ((J->State == mjsStarting) || (J->State == mjsRunning))
    {
    /* if job is active, no need for feasibility analysis */

    return(SUCCESS);
    }

  /* analyze why job has not started */

  IsDeferred = FALSE;

  RQ = J->Req[0]; /* FIXME:  only analyze primary req */     

  if ((!Flags) & (1 << mcmFuture))
    {
    mbool_t Blocked = FALSE;

    char DValue[MAX_MNAME];
    enum MJobDependEnum DType;
    
    /* evaluate dynamic job state/constraint attributes */

    if ((J->State != mjsIdle) && (J->State != mjsHold))
      {
      MUSNPrintF(&BPtr,&BSpace,"job cannot run  (state, '%s', is not idle)\n",
        MJobState[J->State]);
   
      Blocked = TRUE;
      }
    else
      {
      /* check job expected state */
   
      if ((J->EState != mjsIdle) &&
          (J->EState != mjsHold) &&
          (J->EState != mjsDeferred))
        {
        MUSNPrintF(&BPtr,&BSpace,"job cannot run  (expected state, '%s', is not idle)\n",
          MJobState[J->EState]);
 
        Blocked = TRUE;
        }
      }    /* END else ((J->State != mjsIdle) ...) */ 

    if ((J->SMinTime != 0) && (J->SMinTime > MSched.Time))
      {
      MUSNPrintF(&BPtr,&BSpace,"job cannot run  (start date not reached for %s)\n",
        MULToTString(J->SMinTime - MSched.Time));
 
      Blocked = TRUE;
      }
 
    /* check job holds */
 
    if ((J->Hold != 0) || (J->State == mjsHold))
      {
      MUStrNCat(&BPtr,&BSpace,"job cannot run  (job has hold in place)\n");
 
      Blocked = TRUE;
      }

    /* check job dependencies */
 
    if (MJobCheckDependency(J,&DType,DValue) == FAILURE)
      {
      MUSNPrintF(&BPtr,&BSpace,"job cannot run  (dependency %s %s not met)\n",
        DValue,
        MJobDependType[DType]);
 
      Blocked = TRUE;
      }

    /* check avl proc count */
 
    if ((RQ->DRes.Procs * J->Request.TC) > MPar[0].ARes.Procs)
      {
      MUSNPrintF(&BPtr,&BSpace,"job cannot run  (insufficient idle procs:  %d available)\n",
        MPar[0].ARes.Procs);
 
      Blocked = TRUE;
      }

    if (Blocked == TRUE)
      {
      return(SUCCESS);
      } 
    }      /* END if (!Flags & (1 << mcmFuture)) */

  /* check static state information */

  if (J->EState == mjsDeferred)
    { 
    /* temporarily change EState to allow full feasibility checking */

    IsDeferred = TRUE;
    J->EState  = J->State;
    }

  /* check global configured proc count */
 
  if ((RQ->DRes.Procs * J->Request.TC) > MPar[0].CRes.Procs)
    {
    MUSNPrintF(&BPtr,&BSpace,"job cannot run in any partition  (insufficient total procs:  %d available)\n",
      MPar[0].CRes.Procs);
    }

  for (pindex = 0;pindex < MAX_MPAR;pindex++)
    {
    P = &MPar[pindex];

    if (J->Flags & (1 << mjfSpan))
      {
      if (pindex != 0)
        continue;
      }
    else
      {
      if (pindex == 0)
        continue;
      }
 
    if (P->ConfigNodes == 0)
      {
      continue;
      }
 
    /* can job be selected? */ 

    {
    int     SrcQ[2];
    int     DstQ[2];
    mjob_t *tmpQ[2];
 
    MOQueueInitialize(SrcQ);

    tmpQ[0] = J;
    tmpQ[1] = NULL;
 
    SrcQ[0] =  0;
    SrcQ[1] = -1;

    if ((MQueueSelectAllJobs(
          tmpQ,
          ptHARD,
	  P,
          SrcQ,
          FALSE,
	  FALSE,
          FALSE,
          tmpBuffer) == FAILURE) || (SrcQ[0] == -1))
      {
      MUSNPrintF(&BPtr,&BSpace,"cannot select job %s for partition %s (%s)\n\n",
        J->Name,
        P->Name,
        tmpBuffer);

      continue;
      }

    if ((MQueueSelectJobs(
          SrcQ,
          DstQ,
          ptHARD,
          MAX_MNODE_PER_JOB,
          MAX_MTASK,
          MAX_MTIME,
          P->Index,
          Reason,
          FALSE,
          FALSE) == FAILURE) || (DstQ[0] == -1))
      {
      for (index = 0;index < MAX_MREJREASON;index++)
        {
        if (Reason[index] != 0)
          break;
        }
 
      MUSNPrintF(&BPtr,&BSpace,"cannot select job %s for partition %s (%s)\n\n",
        J->Name,
        P->Name,
        (index != MAX_MREJREASON) ? MAllocRejType[index] : "UNKNOWN");
 
      continue;
      }
    }    /* END BLOCK */

    /* job selected */

    if (MUNumListGetCount(
          J->StartPriority,
          RQ->DRes.PSlot,
          P->CRes.PSlot,
          0,
          NULL) == FAILURE)
      {
      MUSNPrintF(&BPtr,&BSpace,"required classes not configured in partition %s (%s)\n", 
        P->Name,
        MUCAListToString(RQ->DRes.PSlot,P->CRes.PSlot,NULL));
 
      continue;
      }
 
    /* check policies */
 
    if (MJobCheckPolicies(
          J,
          PLevel,
          0,
          P,
          &PReason,
          Line,
          MSched.Time) == FAILURE)
      {
      MUSNPrintF(&BPtr,&BSpace,"job cannot run in partition %s.  (%s)\n",
        P->Name,
        Line);
 
      continue;
      }
 
    ncount = 0;
    pcount = 0;
 
    memset(Reason,0,sizeof(Reason));

    IProcs = 0;
 
    /* check requirements */
 
    MinSpeed = (double)1.0;

    MsgBuf[0] = '\0';
 
    for (index = 0;index < MAX_MNODE;index++)
      {
      N = MNode[index];
 
      if ((N == NULL) || (N->Name[0] == '\0'))
        break;
 
      if (N->Name[0] == '\1')
        continue;
 
      if (N->PtIndex != pindex)
        continue;
 
      if ((NodeName[0] != '\0') &&
           strcmp(NodeName,NONE) &&
           strcmp(NodeName,N->Name))
        {
        continue;
        }
 
      if (((!Flags) & (1 << mcmFuture)) && 
         (((N->State == mnsIdle) || (N->State == mnsActive)) &&
          ((N->EState == mnsIdle) || (N->EState == mnsActive))))
        {
        Reason[marState]++;
 
        if (Flags & (1 << mcmVerbose))
          {
          sprintf(temp_str,"%-24s rejected : %s\n",
            N->Name,
            MAllocRejType[marState]);
          strcat(MsgBuf,temp_str);
          }
        }
      else
        {
        char Affinity;

        IProcs += N->ARes.Procs;

        if (MJobCheckNRes(
              J,
              N,
              RQ,
              (Flags & (1 << mcmFuture)) ? MAX_MTIME : MSched.Time,
              &TasksAllowed,
              MinSpeed,
              &rindex,
              &Affinity,
	      NULL,
              TRUE) == SUCCESS)
          {
          ncount++;
          pcount += TasksAllowed * RQ->DRes.Procs;
 
          if (Flags & (1 << mcmVerbose))
            {
            sprintf(temp_str,"%-24s accepted : %d tasks supported\n",
              N->Name,
              TasksAllowed);
            strcat(MsgBuf,temp_str);
            }
          }
        else
          {
          Reason[rindex]++;
 
          if (Flags & (1 << mcmVerbose))
            {
            sprintf(temp_str,"%-24s rejected : %s\n",
              N->Name,
              MAllocRejType[rindex]); 
            strcat(MsgBuf,temp_str);
            }
          }
        }
      }      /* END for (index) */
 
    if ((!(Flags & (1 << mcmFuture))) && ((long)P->ARes.Procs <
       (RQ->DRes.Procs * J->Request.TC)))
      {
      MUSNPrintF(&BPtr,&BSpace,"job cannot run in partition %s (insufficient idle procs available: %d < %d)\n\n",
        P->Name,
        P->ARes.Procs,
        J->Request.TC * RQ->DRes.Procs);
      }
    else if (pcount < RQ->DRes.Procs * J->Request.TC)
      {
      int rcount;

      MUSNPrintF(&BPtr,&BSpace,"job cannot run in partition %s (idle procs do not meet requirements : %d of %d procs found)\n",
        P->Name, 
        pcount,
        RQ->DRes.Procs * J->Request.TC);
 
      rcount = 0;
 
      MUSNPrintF(&BPtr,&BSpace,"idle procs: %3d  feasible procs: %3d\n",
        IProcs,
        pcount);
 
      for (rindex = 0;rindex < MAX_MREJREASON;rindex++)
        {
        if (Reason[rindex] == 0)
          continue;
 
        if (!(rcount % 5))
          MUStrNCat(&BPtr,&BSpace,"\nRejection Reasons: ");
 
        MUSNPrintF(&BPtr,&BSpace,"[%-13s: %4d]",
          MAllocRejType[rindex],
          Reason[rindex]);
 
        rcount++;
        }
 
      MUStrNCat(&BPtr,&BSpace,"\n\n");
 
      if (Flags & (1 << mcmVerbose))
        {
        /* display detailed rejection reasons */ 
 
        MUStrNCat(&BPtr,&BSpace,"Detailed Node Availability Information:\n\n");
 
        MUStrNCat(&BPtr,&BSpace,MsgBuf);
        }
      }
    else if ((J->Request.NC > 0) && (ncount < J->Request.NC))
      {
      MUSNPrintF(&BPtr,&BSpace,"job cannot run in partition %s (insufficient idle nodes available: %d < %d)\n\n",
        P->Name,
        ncount,
        J->Request.NC);
      }
    else
      {
      MUSNPrintF(&BPtr,&BSpace,"job can run in partition %s (%d procs available.  %d procs required)\n",
        P->Name,
        pcount,
        J->Request.TC * RQ->DRes.Procs);
      }
    }   /* END for (pindex) */
 
  if (IsDeferred == TRUE)
    J->EState = mjsDeferred;
 
  return(SUCCESS);
  }  /* END:  UIJobShow() */





/* <jobctl action={cancel,diagnose,hold,modify,query,resume,start,submit,suspend} job={jobexp} flags={VERBOSE}></jobctl> */

int MUIJobCtl(
 
  msocket_t *S,      /* I */
  long       CFlags, /* I */
  char      *Auth)   /* I */
 
  {
  char Command[MAX_MNAME];
  char JobExp[MAX_MLINE];
  char FlagString[MAX_MLINE];
  char ArgString[MAX_MLINE];

  short JobList[MAX_MJOB];
  int   JobCount;

  char tmpLine[MAX_MLINE];
 
  int  CIndex;
  int  jindex;

  int  rc;

  char Msg[MAX_MLINE];

  mjob_t *J;

  mxml_t *E = NULL;

  const char *FName = "MUIJobCtl";
 
  DBG(2,fUI) DPrint("%s(S,%ld,%s)\n",
    FName,
    CFlags,
    (Auth != NULL) ? Auth : "NULL"); 

  if (S == NULL)
    {
    return(FAILURE);
    }

  MUISMsgClear(S);

  /* TEMP:  force to XML */

  S->WireProtocol = mwpXML;

  switch (S->WireProtocol)
    {
    case mwpXML:

      {
      char *ptr;

      if (((ptr = strchr(S->RPtr,'<')) == NULL) ||
           (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE) ||
           (MXMLGetAttr(E,"action",NULL,Command,sizeof(Command)) == FAILURE) ||
           (MXMLGetAttr(E,"job",NULL,JobExp,sizeof(JobExp)) == FAILURE))
        {
        DBG(3,fUI) DPrint("WARNING:  corrupt command '%100.100s' received\n",
          S->RBuffer);
 
        MUISMsgAdd(S,"ERROR:    corrupt command received\n");

        MXMLDestroyE(&E);

        return(FAILURE);
        }

      MXMLGetAttr(E,"flag",NULL,FlagString,sizeof(FlagString)); 
      MXMLGetAttr(E,"arg",NULL,ArgString,sizeof(ArgString));   
      }  /* END BLOCK */

      break;

    default:
 
      if ((rc = MUSScanF(S->RPtr,"%x%s %x%s %x%s %x%s",
             sizeof(Command),
             Command,
             sizeof(JobExp),
             JobExp,
             sizeof(FlagString),
             FlagString,
             sizeof(ArgString),
             ArgString)) != 4)
        {
        DBG(3,fUI) DPrint("WARNING:  corrupt command '%100.100s' received\n",
          S->RBuffer);

        MUISMsgAdd(S,"ERROR:    corrupt command received\n");
 
        return(FAILURE);
        }

      break;
    }   /* END switch (S->WireProtocol) */

  /* process data */

  if ((CIndex = MUGetIndex(Command,MJobCtlCmds,FALSE,mjcmNONE)) == mjcmNONE)
    {
    DBG(3,fUI) DPrint("WARNING:  unexpected subcommand '%s' received\n",
      Command);

    sprintf(tmpLine,"ERROR:    unexpected subcommand '%s'\n",
      Command);

    MUISMsgAdd(S,tmpLine);
 
    return(FAILURE);
    }

  if (CIndex == mjcmSubmit)
    {
    /* NOTE:  hardcode job submissions to first resource manager */

    if ((rc = MRMSJobSubmit(ArgString,&MRM[0],&J,NULL)) == FAILURE)
      {
      sprintf(tmpLine,"ERROR:    cannot submit job\n");
      }
    else
      {
      sprintf(tmpLine,"INFO:    job %s successfully submitted\n",
        J->Name);
      }
 
    MUISMsgAdd(S,tmpLine);

    return(SUCCESS);
    }

  /* translate job expression to list of jobs */

  /* NOTE:  encapsulate job expression '^(<X>)$' */

  if (MSched.UseJobRegEx == TRUE)
    {
    strcpy(tmpLine,JobExp);
    }
  else
    {
    sprintf(tmpLine,"^(%s)$",
      JobExp);
    }

  S->SBuffer[0] = '\0';

  if ((MUREToList(
        tmpLine,
        mxoJob,
        0,
        JobList,
        &JobCount,
        S->SBuffer) == FAILURE) || (JobCount == 0))
    {
    sprintf(tmpLine,"ERROR:    invalid job expression received (%s)\n",
      JobExp);

    MUISMsgAdd(S,tmpLine);  

    return(FAILURE);
    }

  for (jindex = 0;jindex < JobCount;jindex++) 
    {
    J = MJob[JobList[jindex]];

    /* verify job access */

    if (!(CFlags & ((1 << fAdmin1) | (1 << fAdmin2)))) 
      {
      if (strcmp(J->Cred.U->Name,Auth))
        {
        DBG(2,fUI) DPrint("INFO:     user %s is not authorized to perform operation on job %s\n",
          Auth,
          J->Name);
 
        sprintf(tmpLine,"NOTE:  user %s is not authorized to perform operation on job %s\n",
          Auth,
          J->Name);

        MUISMsgAdd(S,tmpLine);   
 
        continue;
        }
      }    /* END if (!(CFlags & ((1 << fAdmin1) | (1 << fAdmin2)))) */

    switch (CIndex)
      {
      case mjcmCancel:
   
        switch(MSched.Mode)
          {
          case msmSim:
 
            MJobSetState(J,mjsRemoved); 
   
            DBG(3,fUI) DPrint("INFO:     job '%s' cancelled by user %s\n",
              J->Name,
              Auth);
 
            sprintf(tmpLine,"job '%s' cancelled\n",
              J->Name);

            MUISMsgAdd(S,tmpLine);      

            break;

          default:
   
            if (MRMJobCancel(J,"MAUI_INFO:  job cancelled by user\n",NULL) == SUCCESS)
              {
              DBG(3,fUI) DPrint("INFO:     job '%s' cancelled by user %s\n",
                J->Name,
                Auth);
 
              sprintf(tmpLine,"job '%s' cancelled\n",
                J->Name);

              MUISMsgAdd(S,tmpLine);  
              }
            else
              {
              DBG(3,fUI) DPrint("ALERT:    cannot cancel job '%s'\n",
                J->Name);
 
              sprintf(tmpLine,"ERROR:  cannot cancel job '%s'\n",
                J->Name);

              MUISMsgAdd(S,tmpLine);     
              }
  
            break;
          }  /* END switch(MSched.Mode) */
 
        return(SUCCESS);
   
        /*NOTREACHED*/
   
        break;

      case mjcmCheckpoint:

        rc = MUIJobPreempt(J,NULL,Auth,mppCheckpoint,tmpLine);

        MUISMsgAdd(S,tmpLine);

        return(rc);

        /*NOTREACHED*/

        break;
 
      case mjcmModify:

        {
        char tmpAttr[MAX_MLINE];
        char tmpVal[MAX_MLINE];

	int  aindex;

        /* determine attribute type */
      
        if ((MXMLGetAttr(E,"attr",NULL,tmpAttr,sizeof(tmpAttr)) == FAILURE) ||
            (MXMLGetAttr(E,"value",NULL,tmpVal,sizeof(tmpVal)) == FAILURE) ||
	   ((aindex = MUGetIndex(tmpAttr,MJobAttr,FALSE,0)) == 0))
          {
          MXMLDestroyE(&E);

          MUISMsgAdd(S,"cannot parse request");

          return(FAILURE);
          }

        /* set job attributes */

        switch(aindex)
          {
          case mjaHold:
          case mjaQOS:
          case mjaSysPrio:
          case mjaReqCMaxTime:

            MUIJobSetAttr(J,aindex,tmpVal,CFlags,FlagString,ArgString,Auth,Msg);

            MUISMsgAdd(S,Msg);

            break;

          case mjaReqAWDuration:

            /* NYI */

            break;

	  case mjaReqSMinTime:

	    /* NYI */

	    break;

          default:

            /* NO-OP */

            break;
          }  /* END switch(aindex) */
        }    /* END BLOCK */

        break;

      case mjcmQuery:

        {
        char tmpAttr[MAX_MLINE];
        char tmpVal[MAX_MLINE];

        char Msg[MAX_MLINE];

        int  aindex;

        /* NOTE:  process job attributes (ie, uname, starttime, diag, etc) */

        /* determine attribute type */

        if ((MXMLGetAttr(E,"attr",NULL,tmpAttr,sizeof(tmpAttr)) == FAILURE) ||
           ((aindex = MUGetIndex(tmpAttr,MJobAttr,FALSE,0)) == 0))
          {
          MXMLDestroyE(&E);

          MUISMsgAdd(S,"cannot parse request");

          return(FAILURE);
          }

        if (MJobAToString(J,aindex,tmpVal,mdfString) == FAILURE)
          {
          sprintf(Msg,"<job name=\"%s\"><attr name=\"%s\">%s</attr></job>",
            J->Name,
            tmpAttr,
            NONE);
          }
        else
          {
          sprintf(Msg,"<job name=\"%s\"><attr name=\"%s\">%s</attr></job>",
            J->Name,
            tmpAttr,
            tmpVal);
          }
  
        MUISMsgAdd(S,Msg);
        }  /* END BLOCK */

        break;

      case mjcmRequeue:

        rc = MUIJobPreempt(J,NULL,Auth,mppRequeue,tmpLine);

        MUISMsgAdd(S,tmpLine);

        return(rc);

        /*NOTREACHED*/

        break;

      case mjcmResume:

        rc = MUIJobResume(J,NULL,Auth,tmpLine);

        MUISMsgAdd(S,tmpLine);

        return(rc);

        /*NOTREACHED*/

        break;

      case mjcmShow:

        /* NYI */

        break;

      case mjcmSubmit:

        /* NYI */

        break;

      case mjcmSuspend:
   
        rc = MUIJobPreempt(J,NULL,Auth,mppSuspend,tmpLine);
  
        MUISMsgAdd(S,tmpLine);
   
        return(rc);
   
        /*NOTREACHED*/
   
        break;
 
      default:

        /* NO-OP */
 
        break;
      }  /* END switch (cindex) */
    }    /* END for (jindex) */
 
  return(SUCCESS);
  }  /* END MUIJobCtl() */




int MUIResCtl(
 
  msocket_t *S,       /* I */
  long       CFlags,  /* I */
  char      *Auth)    /* I */
 
  {
  char Command[MAX_MNAME];
  char ResExp[MAX_MNAME];

  char FlagString[MAX_MNAME];
  char ArgString[MAX_MNAME];

  short ResList[MAX_MRES];
 
  char tmpLine[MAX_MLINE];
 
  int  CIndex;

  int  rc;
 
  mres_t *R;

  mxml_t *E = NULL;

  int     rindex;
  int     ResCount;
 
  const char *FName = "MUIResCtl";
 
  DBG(2,fUI) DPrint("%s(S,%ld,%s)\n",
    FName,
    CFlags,
    (Auth != NULL) ? Auth : "NULL");
 
  if (S == NULL)
    return(FAILURE);

  /* NOTE:  support create, destroy, modify */

  S->WireProtocol = mwpXML;

  switch (S->WireProtocol)
    {
    case mwpXML:

      {
      char *ptr;

      if (((ptr = strchr(S->RPtr,'<')) == NULL) ||
           (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE) ||
           (MXMLGetAttr(E,"action",NULL,Command,sizeof(Command)) == FAILURE) ||
           (MXMLGetAttr(E,"res",NULL,ResExp,sizeof(ResExp)) == FAILURE))
        {
        DBG(3,fUI) DPrint("WARNING:  corrupt command '%100.100s' received\n",
          S->RBuffer);

        MUISMsgAdd(S,"ERROR:    corrupt command received\n");

        MXMLDestroyE(&E);

        return(FAILURE);
        }

      MXMLGetAttr(E,"flag",NULL,FlagString,sizeof(FlagString));
      MXMLGetAttr(E,"arg",NULL,ArgString,sizeof(ArgString));
      }  /* END BLOCK */

      break;

    default:

      /* not supported */

      MUISMsgAdd(S,"ERROR:    corrupt command received\n");

      MXMLDestroyE(&E);

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(S->WireProtocol) */

  /* process data */

  if ((CIndex = MUGetIndex(Command,MJobCtlCmds,FALSE,mjcmNONE)) == mjcmNONE)
    {
    DBG(3,fUI) DPrint("WARNING:  unexpected subcommand '%s' received\n",
      Command);

    sprintf(tmpLine,"ERROR:    unexpected subcommand '%s'\n",
      Command);

    MUISMsgAdd(S,tmpLine);

    return(FAILURE);
    }

  S->SBuffer[0] = '\0';

  if ((MUREToList(
        tmpLine,
        mxoRsv,
        0,
        ResList,
        &ResCount,
        S->SBuffer) == FAILURE) || (ResCount == 0))
    {
    sprintf(tmpLine,"ERROR:    invalid reservation expression received (%s)\n",
      ResExp);

    MUISMsgAdd(S,tmpLine);

    return(FAILURE);
    }

  for (rindex = 0;rindex < ResCount;rindex++)
    {
    R = MRes[ResList[rindex]];

    /* verify res access */

    if (!(CFlags & ((1 << fAdmin1) | (1 << fAdmin2))))
      {
      if ((R->U != NULL) && strcmp(R->U->Name,Auth))
        {
        DBG(2,fUI) DPrint("INFO:     user %s is not authorized to perform operation on res %s\n",
          Auth,
          R->Name);

        sprintf(tmpLine,"NOTE:  user %s is not authorized to perform operation on res %s\n",
          Auth,
          R->Name);

        MUISMsgAdd(S,tmpLine);

        continue;
        }
      }    /* END if (!(CFlags & ((1 << fAdmin1) | (1 << fAdmin2)))) */

    sprintf(tmpLine,"NOTE:  operation on res %s not supported\n",
      R->Name);

    switch(CIndex)
      {
      case mrcmCreate:

        /* NYI */

        rc = FAILURE;

        MUISMsgAdd(S,tmpLine);

        return(rc);

        /*NOTREACHED*/

        break;

      case mrcmDestroy:

        /* NYI */

        rc = FAILURE;

        MUISMsgAdd(S,tmpLine);

        return(rc);

        /*NOTREACHED*/

        break;

      case mrcmModify:

        /* NYI */

        rc = FAILURE;

        MUISMsgAdd(S,tmpLine);

        return(rc);

        /*NOTREACHED*/

        break;

      default:

        /* NYI */

        MUISMsgAdd(S,tmpLine);

        return(FAILURE);

        /*NOTREACHED*/

        break;
      }  /* END switch(CIndex) */
    }    /* END for (rindex) */
 
  return(SUCCESS);
  }  /* END MUIResCtl() */





int MUIJobPreempt(
 
  mjob_t *J,       /* I */
  char   *Args,
  char   *Auth,
  int     PPolicy,
  char   *Msg)     /* O */
 
  {
  int rc;
  int SC;

  if (Msg == NULL)
    {
    return(FAILURE);
    }

  Msg[0] = '\0';

  if (J == NULL)
    {
    sprintf(Msg,"ERROR:    command failed\n");
 
    return(FAILURE);
    }

  rc = MJobPreempt(J,NULL,PPolicy,Msg,&SC);

  if (Msg[0] == '\0')
    {
    sprintf(Msg,"job %s %s preempted\n",
      J->Name,
      (rc == SUCCESS) ? "successfully" : "cannot be");
    }
 
  return(rc);
  }  /* END MUIJobPreempt() */




int MUIJobResume(

  mjob_t *J,    /* I */
  char   *Args,
  char   *Auth,
  char   *Msg)  /* O */

  {
  int rc;
  int SC;

  if (Msg == NULL)
    {
    return(FAILURE);
    }

  Msg[0] = '\0';

  if (J == NULL)
    {
    sprintf(Msg,"ERROR:    command failed\n");

    return(FAILURE);
    }

  rc = MJobResume(J,Msg,&SC);

  if (Msg[0] == '\0')
    {
    sprintf(Msg,"job %s %s resumed\n",
      J->Name,
      (rc == SUCCESS) ? "successfully" : "cannot be ");
    }

  return(rc);
  }  /* END MUIJobResume() */





int MUIGridCtl(
 
  msocket_t *S,     /* I */
  long       Flags,
  char      *Auth)
 
  {
  char     Command[MAX_MNAME];
  char     ResID[MAX_MNAME];
  char     ResDesc[MAX_MLINE];
 
  int      rc;
 
  int      cindex;
  int      srindex;
  int      dindex;
 
  int      vindex;
 
  enum MHoldReasonEnum      Reason;
 
  char    *ptr;
 
  sres_t  *SR;
  sres_t   tmpSR;
 
  mres_t  *R;
 
  char    *TokPtr;

  const char *FName = "MUIGridCtl";
 
  DBG(2,fUI) DPrint("%s(S,%ld,%s)\n",
    FName,
    Flags,
    (Auth != NULL) ? Auth : "NULL");

  if (S == NULL)
    {
    return(FAILURE);
    }
 
  ResID[0] = '\0';
 
  MUSScanF(S->RPtr,"%x%s",
    sizeof(Command),
    Command);
 
  ptr = S->RPtr + strlen(Command); 
 
  cindex = MUGetIndex(Command,MGridCtlCmds,FALSE,0);
 
  if (cindex == 0)
    {
    DBG(3,fUI) DPrint("WARNING:  unexpected %s command '%s' received\n",
      FName,
      Command);
 
    sprintf(S->SBuffer,"ERROR:    unexpected command '%s'\n",
      Command);
 
    return(FAILURE);
    }
 
  switch (cindex)
    {
    case mcInitialize:
 
      rc = MetaCtlInitialize(ptr,Auth,S->SBuffer);
 
      return(rc);
 
      /*NOTREACHED*/
 
      break;
 
    case mcCommit:
 
      rc = MetaCtlCommit(ptr,Auth,S->SBuffer);
 
      return(rc);
 
      /*NOTREACHED*/
 
      break;
 
    case mcList:
 
      rc = MetaCtlList(ptr,Auth,S->SBuffer);
 
      return(rc);
 
      /*NOTREACHED*/ 
 
      break;
 
    case mcQuery:
 
      rc = MetaCtlQuery(ptr,Auth,S->SBuffer);
 
      return(rc);
 
      /*NOTREACHED*/
 
      break;
 
    case mcRegister:
 
      rc = MetaCtlRegister(ptr,Auth,S->SBuffer);
 
      return(rc);
 
      /*NOTREACHED*/
 
      break;
 
    case mcRemove:
 
      rc = MetaCtlRemove(ptr,Auth,S->SBuffer);
 
      return(rc);
 
      /*NOTREACHED*/
 
      break;
 
    case mcSet:
 
      rc = MetaCtlSet(ptr,Auth,S->SBuffer);
 
      return(rc);
 
      /*NOTREACHED*/
 
      break; 
 
    case mcSubmit:
 
      rc = MetaCtlSubmit(ptr,Auth,S->SBuffer);
 
      return(rc);
 
      /*NOTREACHED*/
 
      break;
 
    case mcModify:
 
      /* parse command line */
 
      /* FORMAT:  <RESID> <PARAMETERS> */
 
      MUSScanF(ptr,"%x%s %x%s",
        sizeof(ResID),
        ResID,
        sizeof(ResDesc),
        ResDesc);
 
      /* locate standing reservation */
 
      for (srindex = 0;srindex < MAX_MSRES;srindex++)
        {
        if (SRes[srindex].Name[0] == '\0')
          continue;
 
        if (!strcmp(SRes[srindex].Name,ResID))
          break;
        }  /* END for (srindex) */
 
      if (srindex == MAX_MSRES)
        {
        DBG(3,fUI) DPrint("WARNING:  cannot locate standing reservation %s\n",
          ResID);
 
        sprintf(S->SBuffer,"ERROR:    cannot locate reservation\n");
 
        return(FAILURE); 
        }
 
      /* make modification to override */
 
      S->SBuffer[0] = '\0';
 
      ptr = MUStrTok(ResDesc,";",&TokPtr);
 
      SR = &OSRes[srindex];
 
      while (ptr != NULL)
        {
        if (!strncmp(ptr,"MAXTIME=",strlen("MAXTIME=")))
          {
          if (!strcmp(ptr + strlen("MAXTIME="),"DEFAULT"))
            {
            MACLClear(SR->ACL,maDuration);
 
            sprintf(temp_str,"MAXTIME set to DEFAULT\n");
            strcat(S->SBuffer,temp_str);
            }
          else
            {
            long MaxTime;
 
            MaxTime = MUTimeFromString(ptr + strlen("MAXTIME="));
 
            MACLSet(
              SR->ACL,
              maDuration,
              &MaxTime,
              mcmpGE,
              nmPositiveAffinity,
              (1 << maclRequired),
              0);
 
            sprintf(temp_str,"MAXTIME set to %ld\n",
              MaxTime);
            strcat(S->SBuffer,temp_str);
            }
          }
        else if (!strncmp(ptr,"DAYS=",strlen("DAYS="))) 
          {
          if (!strcmp(ptr + strlen("DAYS="),"DEFAULT"))
            {
            SR->Days = 0;
 
            sprintf(temp_str,"DAYS set to DEFAULT\n");
            strcat(S->SBuffer,temp_str);
            }
          else
            {
            SR->Days = 0;
 
            for (vindex = 0;MWeekDay[vindex] != NULL;vindex++)
              {
              if ((strstr(ptr + strlen("DAYS="),MWeekDay[vindex]) != NULL) ||
                  (strstr(ptr + strlen("DAYS="),MWEEKDAY[vindex]) != NULL))
                {
                SR->Days |= (1 << vindex);
 
                if (strstr(ptr + strlen("DAYS="),"ALL") != NULL)
                  SR->Days = 255;
                }
              }   /* END for (vindex) */
 
            sprintf(temp_str,"DAYS set to %s\n",
              ptr + strlen("DAYS="));
            strcat(S->SBuffer,temp_str);
            }
          }
        else if (!strncmp(ptr,"STARTTIME=",strlen("STARTTIME=")))
          {
          if (!strcmp(ptr + strlen("STARTTIME="),"DEFAULT"))
            {
            SR->StartTime = 0;
 
            sprintf(temp_str,"STARTTIME set to DEFAULT\n");
            strcat(S->SBuffer,temp_str);
            }
          else
            {
            SR->StartTime = MUTimeFromString(ptr + strlen("STARTTIME=")); 
 
            sprintf(temp_str,"STARTTIME set to %s\n",
              MULToTString(SR->StartTime));
            strcat(S->SBuffer,temp_str);
            }
          }
        else if (!strncmp(ptr,"ENDTIME=",strlen("ENDTIME=")))
          {
          if (!strcmp(ptr + strlen("ENDTIME="),"DEFAULT"))
            {
            SR->EndTime = 0;
 
            sprintf(temp_str,"ENDTIME set to DEFAULT\n");
            strcat(S->SBuffer,temp_str);
            }
          else
            {
            SR->EndTime = MUTimeFromString(ptr + strlen("ENDTIME="));
 
            sprintf(temp_str,"ENDTIME set to %s\n",
              MULToTString(SR->EndTime));
            strcat(S->SBuffer,temp_str);
            }
          }
        else if (!strncmp(ptr,"TASKCOUNT=",strlen("TASKCOUNT=")))
          {
          if (!strcmp(ptr + strlen("TASKCOUNT="),"DEFAULT"))
            {
            SR->TaskCount = 0;
 
            sprintf(temp_str,"TASKCOUNT set to DEFAULT\n");
            strcat(S->SBuffer,temp_str);
            }
          else
            {
            SR->TaskCount = atoi(ptr + strlen("TASKCOUNT="));
 
            sprintf(temp_str,"TASKCOUNT set to %d\n",
              SR->TaskCount);
            strcat(S->SBuffer,temp_str);
            }
          }
        else if (!strncmp(ptr,"CHARGEACCOUNT=",strlen("CHARGEACCOUNT="))) 
          {
          /* flush existing charges */
 
          if ((SR->A != NULL) &&
              (SR->R[0] != NULL) &&
              (SR->R[0]->CIPS > 0.0))
            {
            R = SR->R[0];
 
            if (MAMAllocRDebit(&MAM[0],R,&Reason,NULL) == SUCCESS)
              {
              R->TIPS += R->CIPS;
              R->TAPS += R->CAPS;
 
              R->CIPS = 0.0;
              R->CAPS = 0.0;
              }
            else
              {
              DBG(1,fUI) DPrint("ALERT:    cannot charge %6.2f PS to account %s for reservation %s\n",
                R->CIPS,
                R->A->Name,
                R->Name);
              }
            }
 
          if (!strcmp(ptr + strlen("CHARGEACCOUNT="),"DEFAULT"))
            {
            SR->A = NULL;
 
            sprintf(temp_str,"CHARGEACCOUNT set to DEFAULT\n");
            strcat(S->SBuffer,temp_str);
            }
          else
            {
            MAcctAdd(ptr + strlen("CHARGEACCOUNT="),&SR->A);
 
            sprintf(temp_str,"CHARGEACCOUNT set to %s\n",
              (SR->A != NULL) ? SR->A->Name : NONE);
            strcat(S->SBuffer,temp_str);
            } 
          }    /* END else if () */
 
        ptr = MUStrTok(NULL,";",&TokPtr);
        }  /* END while (ptr != NULL) */
 
      strcat(S->SBuffer,"\nSUCCESS:    changes made\n");
 
      /* release all modified standing reservations */
 
      SR = &SRes[srindex];
 
      for (dindex = 0;dindex < (MAX_SRES_DEPTH - 1);dindex++)
        {
        if (dindex >= SR->Depth)
          break;
 
        if (SRes[srindex].R[dindex] != NULL)
          {
          sprintf(temp_str,"INFO:     reservation %s recycled\n",
            SRes[srindex].R[dindex]->Name);
          strcat(S->SBuffer,temp_str);
 
          MResDestroy(&SRes[srindex].R[dindex]);
 
          MSRGetCurrentValues(&SRes[srindex],&OSRes[srindex],&tmpSR);
 
          MSRSetRes(&tmpSR,srindex,dindex);
 
          while (SRes[srindex].R[dindex]->TaskCount < tmpSR.TaskCount)
            {
            if (MResPreempt(SRes[srindex].R[dindex]) == FAILURE)
              break;
 
            MSRSetRes(&tmpSR,srindex,dindex);
            }  /* END while (SRes[srindex].R[dindex]->TaskCount) */
          }    /* END if (SRes[srindex].R[dindex] != NULL)       */
        }      /* END for (dindex)                               */
 
      DBG(3,fUI) DPrint("INFO:    %s reservations recycled\n",
        ResID);
 
      return(SUCCESS); 
 
      /*NOTREACHED*/
 
      break;
 
    case mcResetStats:

      /* NO-OP */
 
      break;
 
    default:
 
      DBG(3,fUI) DPrint("ALERT:   unexpected command '%s' specified\n",
        Command);
 
      sprintf(S->SBuffer,"\nERROR:    unexpected command '%s' specified (%d : %d)\n",
        Command,
        cindex,
        mcModify);
 
      break;
    }   /* END switch (cindex) */
 
  return(SUCCESS);
  }   /* END MUIGridCtl() */




int MUIBal(

  msocket_t *S,     /* I/O */
  long       Flags, /* I */
  char      *Auth)  /* I */

  {
  char tmpLine[MAX_MLINE];
  char tmpBuf[MAX_MBUFFER];

  int  OIndex;

  int nindex;

  mnode_t *N;
  mnode_t *BestN;

  char *ptr;

  mxml_t *E  = NULL;
  mxml_t *RE = NULL;
  mxml_t *DE = NULL;

  const char *FName = "MUIBal";

  DBG(2,fUI) DPrint("%s(S,%ld,%s)\n",
    FName,
    Flags,
    (Auth != NULL) ? Auth : "NULL");

  if (S == NULL)
    {
    return(FAILURE);
    }

  S->WireProtocol = mwpXML;

  /* extract request */

  if (((ptr = strstr(S->RBuffer,"<Message")) == NULL) ||
       (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE))
    {
    /* cannot parse response */

    return(FAILURE);
    }

  if (MXMLGetChild(E,"Request",NULL,&RE) == FAILURE)
    {
    /* cannot parse response */

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  if (MXMLGetAttr(RE,"object",NULL,tmpLine,0) == FAILURE)
    {
    /* cannot parse response */

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  if ((OIndex = MUGetIndex(tmpLine,MXO,FALSE,mxoNONE)) == mxoNONE)
    {
    /* unexpected object */

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  if (MXMLGetAttr(RE,"action",NULL,tmpLine,0) == FAILURE)
    {
    /* cannot parse response */

    MXMLDestroyE(&E);

    return(FAILURE);
    }

  MXMLDestroyE(&E);

  /* determine best host */

  BestN = NULL;

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if (N == NULL)
      break;

    if (N == (mnode_t *)1)
      continue;

    if ((BestN != NULL) && (N->Load >= BestN->Load))
      continue;

    if (N->CRes.Procs <= 0)
      continue;

    BestN = N;
    }  /* END for (nindex) */

  /* return best host */

  MXMLCreateE(&E,"Reply");
  MXMLCreateE(&RE,"Response");
  MXMLAddE(E,RE);

  MXMLCreateE(&DE,"Data");
  MXMLAddE(RE,DE);

  if (BestN != NULL)
    {
    MXMLSetVal(DE,(void *)BestN->Name,mdfString);
    }

  MXMLToString(E,tmpBuf,MAX_MBUFFER,NULL,TRUE);

  MUISMsgAdd(S,tmpBuf);

  return(SUCCESS);
  }  /* END MUIBal() */




int MUINodeCtl(
 
  msocket_t *S,     /* I/O */
  long       Flags, /* I */
  char      *Auth)  /* I */
 
  {
  char     SubCommand[MAX_MNAME];
  char     Arg[MAX_MLINE];
  char     NodeExp[MAX_MLINE];
 
  int      scindex;
  int      aindex;
  int      vindex;
  int      nindex;

  mnode_t *N;
 
  short    NodeList[MAX_MNODE];
 
  int      rc;
 
  int      NodeCount;
 
  char    *ptr;
 
  char    *TokPtr;

  char     tmpLine[MAX_MLINE];
 
  const char *MNCSCmds[] = {
    NONE,
    "create",
    "destroy",
    "modify",
    NULL };
 
  enum {
    mncsNONE,
    mncsCreate,
    mncsDestroy,
    mncsModify };

  /* node attributes */
 
  const char *MNCMArgs[] = { 
    NONE,
    "resource",
    "state",
    "trace",
    NULL };
 
  enum {
    mncmaNONE,
    mncmaResource,
    mncmaState,
    mncmaTrace };

  const char *FName = "MUINodeCtl";
 
  DBG(2,fUI) DPrint("%s(S,%ld,%s)\n",
    FName,
    Flags,
    (Auth != NULL) ? Auth : "NULL");

  if (S == NULL)
    {
    return(FAILURE);
    }
 
  /* FORMAT:  <CMD> <SUBCOMMAND> <ARG> <NODEEXP>       */
  /*                ^                                  */
  /*     ie:  mnodectl modify state=down node001       */
  /*          mnodectl create trace="X" alpha321       */
  /*          mnodectl modify resource=dog.set:4       */
  /*          mnodectl modify resource=dog.inc:1       */ 
  /*          mnodectl modify resource=cat.dec:2       */
 
  if (MSched.Mode != msmSim)
    {
    /* temporary */
 
    sprintf(S->SBuffer,"ERROR:    nodectl only enabled for simulation mode\n");
 
    return(FAILURE);
    }

  tmpLine[0] = '\0';

  MUISMsgClear(S);

  /* TEMP:  force to XML */

  S->WireProtocol = mwpXML;
 
  rc = MUSScanF(S->RPtr,"%x%s %x%s %x%s",
    sizeof(SubCommand),
    SubCommand,
    sizeof(Arg),
    Arg,
    sizeof(NodeExp),
    NodeExp);
 
  if (rc != 3)
    {
    MUISMsgAdd(S,"ERROR:    corrupt command received\n");

    sprintf(tmpLine,"ERROR:    invalid number of arguments received by command '%s' (%d)\n", 
      S->RBuffer,
      rc);

    MUISMsgAdd(S,tmpLine);
 
    return(FAILURE);
    }
 
  if ((scindex = MUGetIndex(SubCommand,MNCSCmds,FALSE,mncsNONE)) == mncsNONE)
    {
    sprintf(tmpLine,"ERROR:    invalid subcommand received (%s)\n",
      SubCommand);

    MUISMsgAdd(S,tmpLine);
 
    return(FAILURE);
    }

  S->SBuffer[0] = '\0';

  switch (scindex)
    {
    case mncsCreate:

      /* NO-OP */

      break;

    default:

      /* determine node expression list */
 
      if (MUREToList(
            NodeExp,
            mxoNode,
            0,
            NodeList,
            &NodeCount,
            S->SBuffer) == FAILURE)
        {
        sprintf(tmpLine,"ERROR:    invalid node expression received (%s)\n",
          NodeExp);

        MUISMsgAdd(S,tmpLine);
 
        return(FAILURE);
        }

      break;
    }  /* END switch (scindex) */
 
  switch(scindex)
    {
    case mncsCreate:

      /* ARG FORMAT:  {trace=<TRACE_LINE>} */

      ptr = MUStrTok(Arg,"=;",&TokPtr);
 
      if ((aindex = MUGetIndex(ptr,MNCMArgs,FALSE,mncmaNONE)) == mncmaNONE)
        {
        sprintf(tmpLine,"ERROR:    invalid argument received (%s)\n",
          ptr);

        MUISMsgAdd(S,tmpLine);
 
        return(FAILURE);
        }

      ptr = MUStrTok(NULL,"=\"\'",&TokPtr);

      if ((ptr == NULL) || 
          (MSimGetResources(NULL,NULL,ptr) == FAILURE))
        {
        sprintf(tmpLine,"ERROR:    cannot process node create request\n");

        MUISMsgAdd(S,tmpLine);
 
        return(FAILURE);
        }

      sprintf(temp_str,"successfully processed node create request\n");
      strcat(tmpLine,temp_str);
 
      break;

    case mncsDestroy:

      for (nindex = 0;NodeList[nindex] != -1;nindex++)
        {
        char NodeName[MAX_MNAME];

        N = MNode[NodeList[nindex]];

        strcpy(NodeName,N->Name);

        if (MNodeRemove(N) == FAILURE)
          {
          sprintf(temp_str,"ERROR:    cannot remove node '%s'\n",
            NodeName);
          strcat(tmpLine,temp_str);
          }
        else
          {
          sprintf(temp_str,"successfully removed node '%s'\n",
            NodeName);
          strcat(tmpLine,temp_str);
          }
        }  /* END for (nindex) */

      break;

    case mncsModify:
 
      /* ARG FORMAT:  {nodetype|partition|priority|resource|state}={idle|down|draining} */ 

      ptr = MUStrTok(Arg,"=;",&TokPtr);

      while (ptr != NULL)
        { 
        if ((aindex = MUGetIndex(ptr,MNCMArgs,FALSE,mncmaNONE)) == mncmaNONE)
          {
          if (MSched.Mode == msmSim)
            {
            /* allow modification of arbitrary node attribute */

            if ((aindex = MUGetIndex(ptr,MNodeAttr,FALSE,mnaNONE)) == mnaNONE)
              {
              sprintf(temp_str,"ERROR:    invalid argument received (%s)\n",
                ptr);
              strcat(tmpLine,temp_str);

              MUISMsgAdd(S,tmpLine);

              return(FAILURE);
              }

            ptr = MUStrTok(NULL,"=;",&TokPtr);

            for (nindex = 0;NodeList[nindex] != -1;nindex++)
              {
              N = MNode[NodeList[nindex]];

              if (MNodeSetAttr(N,aindex,(void *)ptr,mdfString,mSet) == FAILURE)
                {
                sprintf(temp_str,"ERROR:    cannot set attribute '%s' to '%s' on node '%s'\n",
                  MNodeAttr[aindex],
                  ptr, 
                  N->Name);
                strcat(tmpLine,temp_str);

                MUISMsgAdd(S,tmpLine);

                return(FAILURE);
                }

              sprintf(temp_str,"attribute '%s' set to '%s' on node '%s'\n",
                MNodeAttr[aindex],
                ptr,
                N->Name);
              strcat(tmpLine,temp_str);
              }    /* END for (nindex) */

            ptr = MUStrTok(NULL,"=;",&TokPtr);

            continue;
            }      /* END if (MSched.Mode == msmSim) */

          sprintf(temp_str,"ERROR:    invalid argument received (%s)\n",
            ptr);
          strcat(tmpLine,temp_str);

          MUISMsgAdd(S,tmpLine);
 
          return(FAILURE);
          }  /* END if ((aindex = MUGetIndex()) == mncmaNONE) */
 
        ptr = MUStrTok(NULL,"=;",&TokPtr);
 
        switch(aindex)
          {
          case mncmaResource:

            {
            /* determine resource name, subcommand */

            /* FORMAT:  <RESID>.<SUBC>[.<ARG>] */

            const char *NCModSubC[] = {
              NULL,
              "inc",
              "dec",
              "set",
              NULL };

            enum { 
              ncmodscNONE = 0,
              ncmodscInc,
              ncmodscDec,
              ncmodscSet };

            char  tmpLine[MAX_MLINE];

            char *ptr2;
            char *TokPtr2;

            char *ResID;

            int   cindex;
            int   gindex;

            int   aval;
            int   cval;

            MUStrCpy(tmpLine,ptr,sizeof(tmpLine));

            /* get resource ID */      

            if ((ResID = MUStrTok(ptr,".",&TokPtr2)) == NULL)
              {
              sprintf(temp_str,"ERROR:    no resource ID specified\n");
              strcat(S->SBuffer,temp_str);
 
              return(FAILURE);
              }

            /* get subcommand */

            if ((ptr2 = MUStrTok(NULL,".",&TokPtr2)) == NULL)
              {
              sprintf(temp_str,"ERROR:    no subcommand specified\n");
              strcat(S->SBuffer,temp_str);
 
              return(FAILURE);
              }

            if ((cindex = MUGetIndex(ptr2,NCModSubC,FALSE,ncmodscNONE)) == ncmodscNONE)
              {
              sprintf(temp_str,"ERROR:    invalid modify subcommand specified (%s)\n",
                ptr2);
              strcat(S->SBuffer,temp_str);
 
              return(FAILURE);
              }     

            aval = 0;
            cval = 0;

            if ((ptr = MUStrTok(NULL,".:",&TokPtr2)) != NULL)
              {
              aval = atoi(ptr);

              if ((ptr = MUStrTok(NULL,".:",&TokPtr2)) != NULL)
                {
                cval = atoi(ptr);
                }
              }

            /* loop through all nodes */

            for (nindex = 0;NodeList[nindex] != -1;nindex++)
              {
              N = MNode[NodeList[nindex]];

              /* locate resource */

              gindex = MUMAGetIndex(eGRes,ResID,mAdd);

              if ((gindex == 0) || (gindex >= MAX_MGRES))
                {
                /* generic resource overflow */

                sprintf(temp_str,"ERROR:    generic resource overflow\n");
                strcat(S->SBuffer,temp_str);
 
                return(FAILURE);
                }

              /* adjust resource */

              switch(cindex)
                { 
                case ncmodscInc:
    
                  N->ARes.GRes[gindex].count += MAX(1,aval);
                  N->ARes.GRes[gindex].count = MIN(N->ARes.GRes[gindex].count,N->CRes.GRes[gindex].count);
 
                  break;
             
                case ncmodscDec:
   
                  N->ARes.GRes[gindex].count -= MAX(1,aval);
                  N->ARes.GRes[gindex].count = MAX(N->ARes.GRes[gindex].count,0);
 
                  break;

                case ncmodscSet: 

                  N->ARes.GRes[gindex].count = aval;
                  N->CRes.GRes[gindex].count = MAX(aval,cval);

                  break;

                default:

                  /* NO-OP */

                  break;
                }  /* END switch(cindex) */
              }    /* END for (nindex) */
            }      /* END BLOCK */

            break;

          case mncmaState:
 
            if ((vindex = MUGetIndex(ptr,MNodeState,FALSE,mnsNONE)) == mnsNONE)
              {
              sprintf(temp_str,"ERROR:    invalid node state received (%s)\n",
                ptr);
              strcat(S->SBuffer,temp_str);
 
              return(FAILURE);
              }
 
            /* set node state */
   
            for (nindex = 0;NodeList[nindex] != -1;nindex++)
              {
              MNode[NodeList[nindex]]->NewState = vindex;
              }  /* END for (nindex) */
 
            break;
 
          default:

            /* NO-OP */
 
            break; 
          }  /* END switch(aindex) */

        ptr = MUStrTok(NULL,"=;",&TokPtr);
        }  /* END while (ptr != NULL) */

      MUISMsgAdd(S,tmpLine);
 
      break;
 
    default:

      /* NO-OP */
 
      break;
    }  /* END switch(scindex) */

  return(SUCCESS);
  }  /* END MUINodeCtl() */




int UIQueueDiagnose(
 
  char *Buffer,
  long *BufSize,
  int   PLevel,
  int   PIndex)
 
  {
  /* NOTE:  must be synchronized with MQueueSelectJobs() and MJobCheckPolicies() */

  const char *FName = "UIQueueDiagnose";
 
  DBG(2,fUI) DPrint("%s(Buffer,BufSize,%s,%s)\n",
    FName,
    MPolicyMode[PLevel],
    MAList[ePartition][PIndex]);
 
  if (MQueueDiagnose(MJob,MUIQ,PLevel,&MPar[PIndex],Buffer,*BufSize) == FAILURE)
    {
    sprintf(Buffer,"cannot evaluate blocked jobs\n");
 
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END UIQueueDiagnose() */




int MUISMsgClear(

  msocket_t *S)  /* I (modified) */

  {
  int Align;

  if (S == NULL)
    {
    return(FAILURE);
    }

  if (S->SBuffer != NULL)
    {
    S->SBuffer[0] = '\0';

    sprintf(S->SBuffer,"%s%d ",
      MCKeyword[mckStatusCode],
      scFAILURE);
 
    Align = (int)strlen(S->SBuffer) + (int)strlen(MCKeyword[mckArgs]);
 
    sprintf(temp_str,"%*s%s",
      16 - (Align % 16),
      " ",
      MCKeyword[mckArgs]);
    strcat(S->SBuffer,temp_str);
 
    S->SPtr = &S->SBuffer[strlen(S->SBuffer)];
    }

  switch(S->WireProtocol)
    {
    case mwpXML:

      if (S->SE != NULL)
        MXMLDestroyE((mxml_t **)&S->SE);

      break;
 
    default:

      /* NO-OP */

      break;
    }  /* END switch(S->WireProtocol) */

  return(SUCCESS);
  }  /* END MUISMsgClear() */




int MUISMsgAdd(

  msocket_t *S,   /* I (modified) */
  char      *Msg) /* I */
 
  {
  if (S == NULL)
    {
    return(FAILURE);
    }

  switch(S->WireProtocol)
    {
    case mwpXML:

      {
      if (S->SE == NULL)
        MXMLCreateE((mxml_t **)&S->SE,"SchedResponse");
   
      if ((Msg != NULL) && (Msg[0] != '\0'))
        {
        MXMLSetVal((mxml_t *)S->SE,(void *)Msg,mdfString);
        }
      }    /* END BLOCK */

      break;

    default:

      if (S->SBuffer == NULL)
        {
        MUStrDup(&S->SBuffer,Msg);
        }
      else
        { 
        MUStrCpy(S->SPtr,Msg,S->SBufSize - (S->SPtr - S->SBuffer));
        }

      break;
    }  /* END switch(S->WireProtocol) */

  return(SUCCESS);
  }  /* END MUISMsgAdd() */




int __MUISSetStatus(

  msocket_t *S,   /* I (modified) */
  int        SC)  /* I */

  {
  char *ptr;

  switch(S->WireProtocol)
    {
    case mwpXML:

      { 
      char tmpLine[MAX_MLINE];

      if (S->SE == NULL)
        MXMLCreateE((mxml_t **)&S->SE,"SchedResponse");

      sprintf(tmpLine,"%d",SC);

      MXMLSetAttr((mxml_t *)S->SE,"status",tmpLine,mdfString);
      }  /* END BLOCK */
 
      break;
 
    default:

      ptr = S->SBuffer + strlen(MCKeyword[mckStatusCode]);
 
      *ptr = SC + '0';

      break;
    }  /* END switch(S->WireProtocol) */

  return(SUCCESS);
  }  /* END __MUISSetStatus() */




int UIChangeParameter(
 
  char *RBuffer,  /* I */
  char *Buffer,   /* O */
  int   FLAGS,    /* I */
  char *Auth,     /* I */
  long *BufSize)  /* I */
 
  {
  const char *FName = "UIChangeParameter";

  DBG(2,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);
 
  /* change config parameter */
 
  MCfgAdjustBuffer(&RBuffer,FALSE);
 
  if (MCfgProcessBuffer(RBuffer) == FAILURE)
    {
    DBG(2,fUI) DPrint("WARNING:    config line '%s' cannot be processed\n",
      RBuffer);
 
    sprintf(Buffer,"ERROR:  specified parameters cannot be modified\n");
 
    return(FAILURE);
    }

  /* evaluate cred config */

  MCredLoadConfig(mxoSys,NULL,RBuffer,NULL);
  MCredLoadConfig(mxoQOS,NULL,RBuffer,NULL);
  MCredLoadConfig(mxoUser,NULL,RBuffer,NULL);
  MCredLoadConfig(mxoGroup,NULL,RBuffer,NULL);
  MCredLoadConfig(mxoAcct,NULL,RBuffer,NULL);
  MCredLoadConfig(mxoClass,NULL,RBuffer,NULL);

  /* MNodeLoadConfig(NULL,RBuffer); */
 
  MSched.EnvChanged = TRUE;
 
  DBG(2,fUI) DPrint("INFO:     config line '%s' successfully processed\n",
    RBuffer);
 
  sprintf(Buffer,"parameters modified\n");
 
  return(SUCCESS); 
  }  /* END UIChangeParameter() */




int UIClusterShow(

  char *RBuffer,  /* I */
  char *SBuffer,  /* O */
  int   FLAGS,    /* I */
  char *Auth,     /* I */
  long *SBufSize)  /* I */
 
  {
  int   jindex;
  int   findex;
  int   sindex;
 
  int   nindex;
 
  int   Status = mclsNONE;
 
  mnode_t *N;
  mjob_t  *J;

  mframe_t *F;
 
  int      MaxSlotPerFrame;

  int      DisplayMode = mwpNONE;

  mhost_t *S;

  char    *BPtr;
  int      BSpace;

  const char *FName = "UIClusterShow";
 
  DBG(2,fUI) DPrint("%s(RBuffer,SBuffer,%d,%s,SBufSize)\n",
    FName,
    FLAGS,
    Auth);
 
  /* FORMAT */

  /* SYSTEM TIME=<PRESENTTIME> MAXSLOT=<MAXSLOT>
   * FRAME <NAME> <FINDEX> <ATTR> ...
   * NODE <NAME> <STATE> <FINDEX> <SINDEX> <SCOUNT>
   * ...
   * JOB <NAME> <STATE> <DURATION> :<NODELIST>:
   */

  if (SBuffer == NULL)
    {
    return(FAILURE);
    }

  BPtr   = SBuffer;
  BSpace = *SBufSize;

  BPtr[0] = '\0';

  if (strstr(RBuffer,"XML") != NULL)
    DisplayMode = mwpXML;

  /* determine max slot index */ 
 
  MaxSlotPerFrame = 1;
 
  for (findex = 1;findex < MAX_MFRAME;findex++)
    {
    for (sindex = 1;sindex <= MAX_MSLOTPERFRAME;sindex++)
      {
      S = &MSys[findex][sindex];

      if (S->RMName[0] != '\0')
        {
        MaxSlotPerFrame = MAX(
          MaxSlotPerFrame,
          sindex + MAX(1,S->SlotsUsed) - 1);
        }
      }
    }    /* END for (findex) */
 
  if (MaxSlotPerFrame == 1)
    {
    /* locate MaxSlot info via Node table */
 
    for (nindex = 0;nindex < MAX_MNODE;nindex++)
      {
      N = MNode[nindex];
 
      if ((N == NULL) || (N->Name[0] == '\0'))
        break;
 
      if (N->Name[0] == '\1')
        continue;
 
      MaxSlotPerFrame = MAX(MaxSlotPerFrame,N->SlotIndex + MAX(1,N->SlotsUsed) - 1);
      }  /* END for (nindex) */
    }    /* END if (MaxSlotPerFrame == 1) */

  /* create message header */

  switch(DisplayMode)
    {
    case mwpXML:

      MUSNPrintF(&BPtr,&BSpace,"<%s %s=\"%ld\" %s=\"%d\"/>",
        MXO[mxoCluster],
        MClusterAttr[mcluaPresentTime],
        MSched.Time,
        MClusterAttr[mcluaMaxSlot],
        MaxSlotPerFrame);

      break;

    case mwpAVP:
  
      MUSNPrintF(&BPtr,&BSpace,"%s TIME=%ld MAXSLOT=%d\n",
        MXO[mxoSys],
        MSched.Time,
        MaxSlotPerFrame);

      break;

    default:

      MUSNPrintF(&BPtr,&BSpace,"%s %ld %d\n",
        MXO[mxoSys],
        MSched.Time,
        MaxSlotPerFrame);

      break;
     }  /* END switch(DisplayMode) */
 
  /* format frame information */
 
  for (findex = 0;findex < MAX_MFRAME;findex++)
    {
    F = &MFrame[findex];

    if ((F == NULL) || (F->Name[0] == '\0') || (F->NodeCount <= 0)) 
      continue;

    DBG(5,fUI) DPrint("INFO:     collecting status for frame %s\n",
      F->Name); 

    switch(DisplayMode)
      {
      case mwpXML:

        MUSNPrintF(&BPtr,&BSpace,"<%s %s=\"%d\" %s=\"%s\"/>",
          MXO[mxoFrame],
          MFrameAttr[mfraIndex],
          F->Index,
          MFrameAttr[mfraName],
          F->Name);

        break;

      case mwpAVP:

        /* NYI */

        break;

      default:

        /* FORMAT:      OBJ  NM IN DATA */

        MUSNPrintF(&BPtr,&BSpace,"%s %s %d %s\n",
          MXO[mxoFrame],
          F->Name,
          findex,
	  NONE);

        break;
      }  /* END switch(DisplayMode) */
    }    /* END for (findex) */

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    if (!strcmp(N->Name,"GLOBAL")) 
      continue;

    DBG(5,fUI) DPrint("INFO: checking node %s\n", N->Name);

    /* display failure information */

    if ((N->CRes.Disk > 0) && (N->ARes.Disk <= 0))
      Status |= (1 << probLocalDisk);

    if ((N->CRes.Mem > 0) && (N->ARes.Mem <= 0))
      Status |= (1 << probMemory);

    if ((N->CRes.Swap > 0) && (N->ARes.Swap < MIN_SWAP))
      Status |= (1 << probSwap);

    if (N->RM->Type == mrmtLL)
      {
      if (!(N->Network & MUMAGetBM(eNetwork,"ethernet",mVerify)))
        Status |= (1 << probEthernet);
 
      if (!(N->Network & MUMAGetBM(eNetwork,"hps_ip",mVerify)))
        Status |= (1 << probHPS_IP);
 
      if (!(N->Network & MUMAGetBM(eNetwork,"hps_us",mVerify)) && (N->State == mnsIdle))
        Status |= (1 << probHPS_User);
      }
 
    switch(DisplayMode)
      {
      case mwpXML:

        MUSNPrintF(&BPtr,&BSpace,"<%s %s=\"%s\" %s=\"%s\" %s=\"%d\" %s=\"%d\" %s=\"%d\"/>",
          MXO[mxoNode],
          MNodeAttr[mnaNodeID],
          N->Name,
          MNodeAttr[mnaNodeState],
          MNodeState[N->State],
          MNodeAttr[mnaFrame],
          N->FrameIndex,
          MNodeAttr[mnaSlot],
          N->SlotIndex,
          MNodeAttr[mnaSize],
          MAX(1,N->SlotsUsed));

        break;

      case mwpAVP:

        /* NYI */
 
        break;
 
      default:

        /* FORMAT:  NAME STATE FRAME SLOT SIZE ERROR */
 
        MUSNPrintF(&BPtr,&BSpace,"%s %s %s %d %d %d %s\n",
          MXO[mxoNode],
          N->Name,
          MNodeState[N->State],
          N->FrameIndex,
          N->SlotIndex,
          MAX(1,N->SlotsUsed),
          NONE);
 
        break;
      }  /* END switch(DisplayMode) */
    }    /* END for (nindex) */
 
  DBG(5,fUI) DPrint("INFO:     node state data collected\n");
 
  /* collect job status */
 
  for (jindex = 0;MAQ[jindex] != -1;jindex++)
    {
    J = MJob[MAQ[jindex]];
 
    if ((J == NULL) || (J->Name[0] == '\0') || (J->Name[0] == '\1'))
      continue;

    switch(DisplayMode)
      {
      case mwpXML:

        {
        char tmpLine[MAX_MBUFFER];

        mxml_t *E = NULL;

        if (__MUIJobToXML(J,&E,TRUE) == SUCCESS)
          {
          MXMLToString(E,tmpLine,MAX_MBUFFER,NULL,TRUE);

          MXMLDestroyE(&E);

          MUSNPrintF(&BPtr,&BSpace,"%s",tmpLine,MAX_MBUFFER);
          }
        }  /* END BLOCK */

        break;

      case mwpAVP:

        /* NYI */
 
        break;
 
      default:
 
        MUSNPrintF(&BPtr,&BSpace,"%s %s %d %d %ld %s %s %ld %s ",
          MXO[mxoJob], 
          J->Name,
          MJobGetProcCount(J),
          J->NodeCount,
          J->WCLimit,
          J->Cred.U->Name,
          J->Cred.G->Name,
          J->StartTime,
          MJobState[J->State]);

        DBG(3,fUI) DPrint("INFO:     adding job '%s' info (%d tasks) to buffer\n",
          J->Name,
          J->TaskCount);

        for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
          {
          N = J->NodeList[nindex].N;

          MUSNPrintF(&BPtr,&BSpace,"%d:%d;",
            N->FrameIndex,
            N->SlotIndex);

          DBG(4,fUI) DPrint("INFO:     adding node '%s' of job '%s' to buffer\n",
            N->Name,
            J->Name);
          }  /* END for (nindex) */
 
        break;
      }  /* END switch(DisplayMode) */

    MUSNPrintF(&BPtr,&BSpace,"\n"); 
    }  /* END for (jindex) */

  /* NOTE:  should also display reserved jobs */

  /* NYI */
 
  DBG(5,fUI) DPrint("INFO:     job state data collected (%d bytes)\n",
    (int)strlen(SBuffer));
 
  return(SUCCESS);
  }  /* END UIClusterShow() */




int UIShowConfig(
 
  char *RBuffer,  /* I */
  char *Buffer,   /* O */
  int   FLAGS,    /* I */
  char *Auth,     /* I */
  long *BufSize)  /* I */
 
  {
  int  Vflag;
 
  char ConfigMode[MAX_MNAME];
  char tmpParameter[MAX_MNAME];
 
  int  PIndex;

  const char *FName = "UIShowConfig";
 
  DBG(2,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);
 
  /* set verbose flag */
 
  ConfigMode[0] = '\0';
 
  MUSScanF(RBuffer,"%x%s %x%s",
    sizeof(ConfigMode),
    ConfigMode,
    sizeof(tmpParameter),
    tmpParameter);
 
  PIndex = MUGetIndex(tmpParameter,(const char **)MParam,FALSE,-1);
 
  if (!strcmp(ConfigMode,"VERBOSE"))
    Vflag = 1;
  else
    Vflag = 0; 
 
  ConfigShow(Buffer,PIndex,Vflag);
 
  if (X.XShowConfig != (int (*)())0)
    (*X.XShowConfig)(X.xd,Vflag,Buffer,*BufSize);
 
  return(SUCCESS);
  }  /* END UIShowConfig() */




int UIShowCStats(

  char *CName,   /* I */
  int   CIndex,  /* I */
  char *Buf,     /* O */
  long *BufSize) /* O */

  {
  void *O;
  void *OP;
 
  void   *OE;
  int     OS;

  char   *NameP;

  char tmpLine[MAX_MLINE];

  mxml_t *E;

  const int CAList[] = { mcaID, -1 };
  const int CCList[] = { mxoStats, mxoLimits, mxoFS, -1 }; 

  const char *FName = "UIShowCStats";
 
  DBG(2,fUI) DPrint("%s('%s',%s,Buf,BufSize)\n",
    FName,
    CName,
    MXO[CIndex]);
 
  if ((CName == NULL) || strcmp(CName,NONE))
    {
    /* if CName specified */
 
    DBG(4,fUI) DPrint("INFO:     searching for %s '%s'\n",
      MXO[CIndex],
      CName);

    if (MOGetObject(CIndex,CName,&O,mVerify) == FAILURE)
      {
      DBG(3,fUI) DPrint("INFO:     cannot locate %s %s\n",
        MXO[CIndex],
        CName); 
 
      sprintf(Buf,"cannot locate %s '%s'\n",
        MXO[CIndex],
        CName);
 
      return(FAILURE);
      }
    }   /* END if (CName == NULL) */

  Buf[0] = '\0';

  MSchedStatToString(&MSched,mwpXML,Buf,*BufSize);
 
  MOINITLOOP(&OP,CIndex,&OS,&OE);

  while ((O = MOGetNextObject(&OP,CIndex,OS,OE,&NameP)) != NULL)
    {
    if ((CName != NULL) && strcmp(CName,NONE))
      {
      if (strcmp(CName,NameP))
        continue;
      }
      
    E = NULL;

    MCOToXML(O,CIndex,&E,(int *)CAList,(int *)CCList,0);

    MXMLToString(E,tmpLine,MAX_MBUFFER,NULL,TRUE);
 
    MXMLDestroyE(&E);
 
    strcat(Buf,tmpLine);
    }  /* END while (MOGetNextObject() != NULL) */
 
  return(SUCCESS); 
  }  /* END UIShowCStats() */




int UIStatClear(

  char *RBuffer,
  char *Buffer,
  int   FLAGS,
  char *Auth,
  long *BufSize)
 
  {
  const char *FName = "UIStatClear";
 
  DBG(2,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);
 
  MStatClearUsage(0,(1 << mlActive)|(1 << mlIdle)|(1 << mlSystem),FALSE);
 
  MStat.InitTime = MSched.Time;
 
  sprintf(Buffer,"%ld\n",
    MSched.Time);
 
  DBG(3,fUI) DPrint("INFO:     statistics cleared\n");
 
  if (MSched.Mode == msmNormal)
    {
    MOSSyslog(LOG_INFO,"statistics cleared");
    }
 
  return(SUCCESS);
  }  /* END UIStatClear() */




int UIQueueShow(
 
  char *RBuffer,   /* I */
  char *Buffer,    /* O */
  int   FLAGS,
  char *Auth,
  long *BufSize)
 
  {
  int QueueMode;
 
  char PName[MAX_MNAME];
  char UName[MAX_MNAME];
   
  int rc;

  int Flags;

  mpar_t *P;
 
  const char *FName = "UIQueueShow";
 
  DBG(2,fUI) DPrint("%s(RBuffer,Buffer,%x,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);
 
  MUSScanF(RBuffer,"%d %x%s %d %x%s",
    &QueueMode,
    sizeof(PName),
    PName,
    &Flags,
    sizeof(UName),
    UName);
 
  if (MParFind(PName,&P) == FAILURE)
    {
    sprintf(Buffer,"ERROR:  invalid partition, %s, specified\n",
      PName);
 
    return(FAILURE);
    }
 
  switch(QueueMode)
    {
    case 0:
 
      /* show all queues */ 
 
      rc = UIQueueShowAllJobs(Buffer,BufSize,P,UName);
 
      break;
 
    case 1:
 
      /* show detailed eligible queue */
 
      if (!(FLAGS & ((1 << fAdmin1) | (1 << fAdmin2) | (1 << fAdmin3))))
        {
        sprintf(Buffer,"ERROR:  not authorized to run this command\n");
 
        return(FAILURE);
        }
 
      rc = UIQueueShowEJobs(Buffer,BufSize,P,UName);
 
      break;
 
    case 2:
 
      /* show detailed active queue */
 
      if (!(FLAGS & ((1 << fAdmin1) | (1 << fAdmin2) | (1 << fAdmin3))))
        {
        sprintf(Buffer,"ERROR:  not authorized to run this command\n");
 
        return(FAILURE);
        }
 
      rc = UIQueueShowAJobs(Buffer,BufSize,P,Flags,UName);
 
      break;

    case 3:

      /* show detailed blocked queue */

      /*
      if (!(FLAGS & ((1 << fAdmin1) | (1 << fAdmin2) | (1 << fAdmin3))))
        {
        sprintf(Buffer,"ERROR:  not authorized to run this command\n");
 
        return(FAILURE);
        }
      */

      rc = UIQueueShowBJobs(Buffer,BufSize,P,UName);
 
      break;
  
    default:
 
      rc = FAILURE;
 
      sprintf(Buffer,"ERROR:  unexpected display mode, %d\n",
        QueueMode);
 
      break; 
    }  /* END switch(QueueMode) */
 
  return(rc);
  }  /* END UIQueueShow() */




int UIShowEstStartTime(
 
  char *RBuffer,
  char *Buffer,
  int   FLAGS,
  char *Auth,
  long *BufSize)
 
  {
  const char *FName = "UIShowEstStartTime";

  DBG(2,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);
 
  /* show estimated expected and worst-case job start time */
 
  /* NYI */
 
  return(SUCCESS);
  }  /* END UIShowEstStartTime() */



int UIJobGetStart(
 
  char *RBuffer,
  char *Buffer,
  int   FLAGS,
  char *Auth,
  long *BufSize)
 
  {
  long   Start;
  long   Deadline;
  char   JName[MAX_MNAME];
 
  char   PName[MAX_MNAME];
 
  mnodelist_t MNodeList;

  mpar_t *P; 
  mpar_t *BP;
 
  int    NodeCount;
  int    TaskCount;
 
  int    ReqTaskCount;
  long   ReqDuration;
 
  mjob_t tmpJ;
  mreq_t tmpRQ;
  macl_t tmpACL[MAX_MACL]; 
  macl_t tmpCL[MAX_MACL];
 
  mnalloc_t tmpJNodeList[MAX_MNODE];
  mnalloc_t tmpRQNodeList[MAX_MNODE];
 
  mjob_t *J;
 
  char   *ptr;
  char   *TokPtr;
 
  const char *FName = "UIJobGetStart";
 
  DBG(2,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);
 
  MUSScanF(RBuffer,"%x%s %x%s",
    sizeof(JName),
    JName,
    sizeof(PName),
    PName);

  if (MParFind(PName,&P) == FAILURE)
    P = &MPar[0]; 
 
  if (strchr(JName,'@') == NULL)
    {
    DBG(5,fUI) DPrint("INFO:     attempting to locate job '%s'\n",
      JName);
 
    if (MJobFind(JName,&J,FALSE) == FAILURE)
      {
      DBG(2,fUI) DPrint("WARNING:  cannot locate job '%s'\n",
        JName);
 
      sprintf(Buffer,"ERROR:    cannot locate job '%s'\n",
        JName);
 
      return(FAILURE);
      }
    }
  else
    {
    /* FORMAT:  <PROCCOUNT>@<WALLTIME> */
 
    J = &tmpJ; 
 
    strcpy(J->Name,JName);
 
    ReqTaskCount = 1;
    ReqDuration  = 1;
 
    if ((ptr = MUStrTok(JName,"@",&TokPtr)) != NULL)
      {
      ReqTaskCount = (int)strtol(ptr,NULL,0);
 
      if ((ptr = MUStrTok(NULL,"@",&TokPtr)) != NULL)
        {
        ReqDuration = MUTimeFromString(ptr);
        }
      else
        {
        ReqDuration = 1;
        }
      }
 
    strcpy(JName,J->Name);
 
    memset(&tmpRQ,0,sizeof(tmpRQ));
 
    MJobMkTemp(&tmpJ,&tmpRQ,tmpACL,tmpCL,tmpJNodeList,tmpRQNodeList);
 
    strcpy(J->Name,JName); 
 
    /* allow no credential access */
 
    J->Request.TC = ReqTaskCount;
    J->SpecWCLimit[0] = ReqDuration;
 
    J->Req[0]->TaskCount = ReqTaskCount;
    }
 
  Start = MSched.Time;
 
  if (J->R != NULL)
    {
    Start = J->R->StartTime;
    BP    = &MPar[J->R->PtIndex];
    }
  else
    {
    mpar_t *tmpP;
  
    tmpP = (P->Index > 0) ? P : NULL;

    if (MJobGetEStartTime(
         J,
         &tmpP,
         &NodeCount,
         &TaskCount,
         MNodeList,
         &Start) == FAILURE)
      {
      DBG(2,fUI) DPrint("WARNING:  cannot determine earliest start time for job '%s'\n", 
        J->Name);
 
      sprintf(Buffer,"ERROR:    cannot determine earliest start time for job '%s'\n",
        J->Name);
 
      return(FAILURE);
      }

    BP = tmpP;
    }  /* END else (J->R != NULL) */
 
  /* start time located */
 
  Deadline = Start + J->SpecWCLimit[0];
 
  sprintf(Buffer,"%s %ld %ld %ld %d %s",
    (J->Name[0] != '\0') ? J->Name : NONE,
    MSched.Time,
    Deadline,
    J->SpecWCLimit[0],
    MJobGetProcCount(J),
    BP->Name);
 
  DBG(3,fUI) DPrint("INFO:     earliest start located for job %s in %s on partition %s at %s",
    J->Name,
    MULToTString(Start - MSched.Time),
    BP->Name,
    MULToDString((mulong *)&Deadline)); 
 
  return(SUCCESS);
  }  /* END UIJobGetStart() */




int UIJobStart(
 
  char *RBuffer,
  char *Buffer,
  int   FLAGS,
  char *Auth,
  long *BufSize)
 
  {
  char     JName[MAX_MNAME];

  char     tmp[MAX_MNAME];
  char     SpecNodeList[MAX_MLINE];
  char    *ptr;
 
  char     PName[MAX_MNAME];
 
  int      index;
  int      nindex;
 
  int      nodeindex;
  int      pcount;
 
  int      pindex;
 
  int      ForceLevel;
 
  int      PReason;
 
  nodelist_t  tmpNodeList;
  mnodelist_t MNodeList;
 
  char     NodeMap[MAX_MNODE];
 
  int      NodeCount;
  int      TaskCount; 
 
  int      TC;
  int      PC;
 
  mjob_t   *J;
  mnode_t  *N;
 
  mreq_t   *RQ;
 
  char    *TokPtr;
 
  mpar_t *P;
  mpar_t *PS;
 
  const char *FName = "UIJobStart";
 
  DBG(2,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);
 
  SpecNodeList[0] = '\0';
 
  MUSScanF(RBuffer,"%x%s %x%s %x%s %x%s",
    sizeof(JName),
    JName,
    sizeof(tmp),
    tmp,
    sizeof(PName),
    PName,
    sizeof(SpecNodeList),
    SpecNodeList);
 
  if (MJobFind(JName,&J,0) != SUCCESS)
    {
    sprintf(Buffer,"ERROR:  cannot locate job '%s'\n",
      JName);
 
    DBG(3,fUI) DPrint("INFO:     cannot locate job '%s' in %s()\n",
      FName,
      JName);
 
    return(FAILURE); 
    }

  if (MParFind(PName,&PS) == FAILURE)
    {
    PS = NULL;
    }
 
  ForceLevel = 0;
 
  if (!strcmp(tmp,MClientMode[mcmClear]))
    {
    /* clear host info */
 
    if (!strcmp(SpecNodeList,NONE))
      {
      sprintf(SpecNodeList,"%d",
        J->Request.TC);
      }
 
    if (MRMJobModify(J,"Resource_List","neednodes",SpecNodeList,NULL) == FAILURE)
      {
      sprintf(Buffer,"ERROR:  cannot set hostlist for job '%s' to '%s'\n",
        J->Name,
        SpecNodeList);
 
      return(FAILURE);
      }
    else
      {
      /* reset internal hostlist state */
 
      if (J->SpecFlags & (1 << mjfHostList))
        {
        J->SpecFlags ^= (1 << mjfHostList);
 
        MJobUpdateFlags(J);
 
        if (J->ReqHList != NULL) 
          MUFree((char **)&J->ReqHList);
        }
 
      sprintf(Buffer,"INFO:  successfully set hostlist for job '%s' to '%s'\n",
        J->Name,
        SpecNodeList);
 
      return(SUCCESS);
      }
    }
  else if (!strcmp(tmp,MClientMode[mcmBlock]))
    {
    MUIJobPreempt(J,NULL,Auth,-1,Buffer);
 
    return(SUCCESS);
    }
 
  if (!strcmp(tmp,MClientMode[mcmForce]))
    {
    ForceLevel = 1;
    }
  else if (!strcmp(tmp,MClientMode[mcmForce2]))
    {
    ForceLevel = 2;
    }
 
  /* initialize variables */
 
  RQ = J->Req[0];  /* FIXME:  only one req in UIJobStart */
 
  Buffer[0] = '\0';
 
  memset(NodeMap,nmNone,sizeof(NodeMap));
 
  if (J->State != mjsIdle)
    {
    sprintf(temp_str,"job '%s' is in state '%s'  (state must be idle)\n",
      J->Name,
      MJobState[J->State]); 
    strcat(Buffer,temp_str);
 
    return(SUCCESS);
    }
 
  /* check job expected state */
 
  if (J->EState != mjsIdle)
    {
    sprintf(temp_str,"job '%s' is in expected state '%s' (expected state must be idle)\n",
      J->Name,
      MJobState[J->EState]);
    strcat(Buffer,temp_str);
 
    return(SUCCESS);
    }
 
  /* check proc count */
 
  if ((ForceLevel < 2) || !strcmp(SpecNodeList,NONE))
    {
    if (RQ->DRes.Procs * J->Request.TC > MPar[0].ARes.Procs)
      {
      sprintf(temp_str,"job cannot run  (insufficient idle procs:  %d needed  %d available)\n",
        J->Request.TC * RQ->DRes.Procs,
        MPar[0].ARes.Procs);
      strcat(Buffer,temp_str);
 
      return(SUCCESS);
      }
    }
 
  if (ForceLevel == 0)
    {
    /* check policies */
 
    if (MJobCheckPolicies(
          J,
          ptHARD,
          0,
          (PS != NULL) ? PS : &MPar[0],
          &PReason,
          NULL,
          MSched.Time) == FAILURE)
      {
      sprintf(temp_str,"job cannot run (rejected by policy %s)\n",
        MPolicyRejection[PReason]); 
      strcat(Buffer,temp_str);
 
      return(SUCCESS);
      }
    }
 
  if (!strcmp(SpecNodeList,NONE))
    {
    /* check node requirements */
 
    for (pindex = 0;pindex < MAX_MPAR;pindex++)
      {
      P = &MPar[pindex];
 
      if ((PS != NULL) && (P != PS))
        continue;
 
      if (MUNumListGetCount(
            J->StartPriority,
            RQ->DRes.PSlot,
            P->CRes.PSlot,
            0,
            NULL) == FAILURE)
        {
        continue;
        }
 
      if (ForceLevel < 2)
        {
        if (MJobSelectMNL(
              J,
              P,
              NULL,
              MNodeList,
              NodeMap,
              0) == FAILURE)
          {
          continue;
          }
        }
      else
        { 
        /* ignore policies, reservations, and QOS flags (ie, AdvRes) */
 
        /* get feasible nodes */
 
        if (MReqGetFNL(
              J,
              RQ,
              P,
              NULL,
              tmpNodeList,
              &NodeCount,
              &TaskCount,
              MAX_MTIME,
              0) == FAILURE)
          {
          continue;
          }
 
        memset(NodeMap,nmNone,sizeof(NodeMap));
 
        nindex    = 0;
 
        TaskCount = 0;
        TC        = 0;
 
        /* check node state/estate/resources only, update nodemap */
 
        for (index = 0;index < NodeCount;index++)
          {
          if (tmpNodeList[index].N == NULL)
            break;
 
          N = tmpNodeList[index].N;
 
          if (((N->State != mnsIdle) && (N->State != mnsActive)) ||
              ((N->EState != mnsIdle) && (N->EState != mnsActive)))
            {
            continue;
            } 
 
          TC = MNodeGetTC(N,&N->ARes,&N->CRes,&N->DRes,&RQ->DRes,MSched.Time);
 
          if (TC < 1)
            continue;
 
          TaskCount += TC;
 
          /* add node to list */
 
          MNodeList[0][nindex].N  = N;
          MNodeList[0][nindex].TC = TC;
 
          nindex++;
          }
 
        MNodeList[0][nindex].N = NULL;
 
        if ((TaskCount < J->Request.TC) || (nindex < J->Request.NC))
          {
          continue;
          }
        }    /* END (ForceLevel < 2) */
 
      /* start job in first available partition */
 
      break;
      }      /* END for (pindex) */
 
    if (pindex == MAX_MPAR)
      {
      sprintf(temp_str,"job cannot run (available nodes do not meet requirements in any partition)\n");
      strcat(Buffer,temp_str);
 
      return(SUCCESS);
      }
    }
  else
    {
    /* create specified nodelist */ 
 
    nodeindex = 0;
    pcount    = 0;
 
    ptr = MUStrTok(SpecNodeList,",: \t\n",&TokPtr);
 
    while (ptr != NULL)
      {
      if (MNodeFind(ptr,&N) != SUCCESS)
        {
        char tmpName[MAX_MNAME];

        /* append domain and try again */
 
        sprintf(temp_str,"%s",
          MSched.DefaultDomain);
        strcat(tmpName,temp_str);
 
        if (MNodeFind(tmpName,&N) != SUCCESS)
          {
          sprintf(temp_str,"ERROR:  cannot locate node '%s' in specified nodelist\n",
            ptr);
          strcat(Buffer,temp_str);
 
          return(FAILURE);
          }
        }    /* END if (MNodeFind() != SUCCESS) */
 
      MNodeList[0][nodeindex].N = N;
 
      if (RQ->TasksPerNode > 0)
        MNodeList[0][nodeindex].TC = (short)RQ->TasksPerNode;
      else
        MNodeList[0][nodeindex].TC = 1;
 
      pcount += MNodeList[0][nodeindex].TC;
 
      nodeindex++;
 
      ptr = MUStrTok(NULL,": \t\n",&TokPtr);
      }  /* END while (ptr != NULL) */
 
    if (nodeindex < (int)(RQ->DRes.Procs * J->Request.TC))
      {
      sprintf(temp_str,"ERROR:  incorrect number of procs in hostlist. (%d requested  %d specified)\n",
        J->Request.TC * RQ->DRes.Procs,
        nodeindex);
      strcat(Buffer,temp_str);
 
      return(FAILURE);
      }
 
    MNodeList[0][nodeindex].N = NULL;
    }  /* END else (strcmp(SpecNodeList,NONE)) */
 
  if (MJobAllocMNL(
        J,
        MNodeList,
        NodeMap,
        NULL,
        MPar[RQ->PtIndex].NAllocPolicy,
        MSched.Time) == FAILURE)
    {
    DBG(2,fUI) DPrint("INFO:     cannot allocate nodes for job '%s'\n",
      J->Name);
 
    sprintf(temp_str,"ERROR:    cannot allocate nodes for job '%s'\n",
      J->Name);
    strcat(Buffer,temp_str);
 
    return(SUCCESS);
    }

  PC = RQ->DRes.Procs * J->Request.TC;
 
  if (MJobStart(J) == FAILURE)
    {
    DBG(2,fUI) DPrint("INFO:     job '%s' cannot be started by user %s\n",
      J->Name,
      Auth);
 
    sprintf(temp_str,"job '%s' cannot be started on %d proc%s\n",
      J->Name,
      PC,
      (PC == 1) ? "" : "s");
    strcat(Buffer,temp_str);
 
    return(SUCCESS);
    }
 
  DBG(2,fUI) DPrint("INFO:     job '%s' started by user %s\n",
    J->Name,
    Auth);
 
  sprintf(temp_str,"job '%s' started on %d proc%s\n",
    J->Name,
    PC,
    (PC == 1) ? "" : "s");
  strcat(Buffer,temp_str);
 
  if (MSched.Mode == msmNormal)
    {
    MOSSyslog(LOG_INFO,"job %s started manually",
      J->Name);
    }
 
  return(SUCCESS);
  }  /* END UIJobStart() */




int UIDiagnosePriority(
 
  char   *Buffer,  /* O */
  long   *BufSize, /* I */
  mpar_t *P)       /* I */
 
  {
  int index;
  int JobCount;
 
  double tmpD;
  char  *BPtr;
  int    BSpace;
 
  mjob_t *J;
 
  const char *FName = "UIDiagnosePriority";
 
  DBG(2,fUI) DPrint("%s(Buffer,BufSize,%s)\n",
    FName,
    (P != NULL) ? P->Name : "NULL");
 
  MUSNInit(&BPtr,&BSpace,Buffer,(int)*BufSize);

  MUSNPrintF(&BPtr, &BSpace, "diagnosing job priority information (partition: %s)\n\n",
    P->Name);
 
  /* initialize priority statistics */
 
  MJobGetStartPriority(NULL,P->Index,NULL,1,&BPtr,&BSpace);
 
  JobCount = 0;
 
  for (index = 0;MUIQ[index] != -1;index++)
    {
    J = MJob[MUIQ[index]];

    if ((J == NULL) || (J->Name[0] == '\0') || (J->Name[0] == '\1'))
      continue;
 
    DBG(5,fUI) DPrint("INFO:     diagnosing priority for job '%s'\n",
      J->Name);
 
    MJobGetStartPriority(J,P->Index,&tmpD,0,&BPtr,&BSpace);
 
    J->StartPriority = (long)tmpD;
 
    JobCount++;
    }  /* END for (index) */
 
  if (JobCount > 0)
    {
    MJobGetStartPriority(NULL,P->Index,NULL,2,&BPtr,&BSpace);
    }
  else
    {
    strcpy(Buffer,"no idle jobs in queue\n");
    }
 
  *BufSize = strlen(Buffer);
 
  return(SUCCESS);
  }  /* END UIDiagnosePriority() */




int MParDiagnose(

  char *Buffer,
  long *BufSize,
  char *DiagOpt)

  {
  const char *FName = "MParDiagnose";

  DBG(3,fUI) DPrint("%s(Buffer,BufSize,%s)\n",
    FName,
    DiagOpt);

  MParShow(DiagOpt,Buffer,BufSize,0);

  return(SUCCESS);
  }  /* END MParDiagnose() */




int MUIJobSetAttr(

  mjob_t *J,          /* I */
  int     AIndex,     /* I */
  char   *Val,
  int     CFlags,
  char   *FlagString,
  char   *ArgString,
  char   *AName,
  char   *Msg)

  {
  int JobOwner = FALSE;

  if (J == NULL)
    {
    sprintf(Msg,"ERROR:  internal command failure");

    return(FAILURE);
    }

  if (!strcmp(J->Cred.U->Name,AName))
    {
    JobOwner = TRUE;
    }

  switch (AIndex)
    {
    case mjaHold:

      {
      int hindex;
      int hbm;

      /* get hold type */

      hindex = MUGetIndex(Val,MHoldType,FALSE,mhNONE);

      switch (hindex)
        {
        case mhUser:

          if (!(CFlags & ((1 << fAdmin1) | (1 << fAdmin2))) &&
              (JobOwner == FALSE))
            {
            sprintf(Msg,"ERROR:  request not authorized");

            return(FAILURE);
            }

	  break;

        case mhNONE:

          sprintf(Msg,"ERROR:  invalid hold type (%s) specified",
            Val);

          return(FAILURE);

          /*NOTREACHED*/

          break;

        default:

          if (!(CFlags & ((1 << fAdmin1) | (1 << fAdmin2)))) 
            {
            sprintf(Msg,"ERROR:  request not authorized");

            return(FAILURE);
            }

          break;
        }  /* END switch(hindex) */

      hbm = 1 << hindex;

      MJobSetAttr(
        J,
        mjaHold,
        (void **)&hbm,
        mdfInt,
        (strstr(FlagString,"unset") != NULL) ? mUnset : mSet);

      sprintf(Msg,"job holds adjusted");
      }  /* END BLOCK */

      break;

    case mjaReqCMaxTime:

      {
      long ReqCTime;

      int  Mode;

      if (!(CFlags & ((1 << fAdmin1) | (1 << fAdmin2))) &&
           (JobOwner == FALSE))
        {
        sprintf(Msg,"ERROR:  request not authorized");

        return(FAILURE);
        }

      if (strstr(FlagString,"unset"))
        {
        Mode = mUnset;
        }
      else
        {
        Mode = mSet;
        }
 
      ReqCTime = strtol(Val,NULL,0);

      if (Mode == mSet)
        {
        long BestCTime;

        int  CTimeIsValid = TRUE;

        mnodelist_t MNodeList;

        long        StartTime;

        mpar_t     *P;

        int         NC;
        int         TC;

        /* validate specified time */

        /* NOTE:  active jobs are valid due to preemption */

        /* NOTE:  job QOS should control which jobs allow CMaxTime (NYI) */

        if (J->StartTime > 0)
          BestCTime = J->StartTime + J->SpecWCLimit[0];
        else
          BestCTime = MSched.Time + J->SpecWCLimit[0];

        if (ReqCTime < BestCTime)
          {
          CTimeIsValid = FALSE;
          }
        else
          {
          /* attempt to schedule at requested ctime */

          StartTime = ReqCTime - J->SpecWCLimit[0];
          P         = NULL;

          if (MJobGetEStartTime(
              J,
              &P,
              &NC,
              &TC,
              MNodeList,
              &StartTime) == FAILURE)
            {
            /* cannot reserve nodes at specified time */
            /* attempt to reserve job at earlier time */

            StartTime = MSched.Time;
            P         = NULL;

            if (MJobGetEStartTime(
                 J,
                 &P,
                 &NC,
                 &TC,
                 MNodeList,
                 &StartTime) == FAILURE)
              {
              DBG(2,fUI) DPrint("WARNING:  cannot find earliest start time for job '%s'\n",
                J->Name);

              sprintf(Msg,"ERROR:    cannot find earliest start time for job '%s'\n",
                J->Name);

              CTimeIsValid = FALSE; 
              }
            }
          }    /* END else (ReqCTime < BestCTime) */
  
        if (CTimeIsValid == FALSE)
          {    
          /* cannot meet requested completion time */

          DBG(2,fUI) DPrint("INFO:     requested deadline %s cannot be set for job %s",
            MULToDString((mulong *)&ReqCTime),
            J->Name);

          sprintf(Msg,"requested deadline %s cannot be set for job %s",
            MULToDString((mulong *)&ReqCTime),
            J->Name);

          return(SUCCESS);
          }

        /* create ctime reservation */
 
        if (MResJCreate(J,MNodeList,StartTime,mjrDeadline,NULL) == FAILURE)
          {
          sprintf(Msg,"ERROR:  cannot enforce request completion time deadline %s for job %s",
            MULToDString((mulong *)&ReqCTime),
            J->Name);

          return(SUCCESS);
          }
        }    /* END if (Mode == mSet) */

      MJobSetAttr(
        J,
        AIndex,
        (void **)&ReqCTime,
        mdfLong,
        Mode);
      }  /* END BLOCK */

      break;

    case mjaSysPrio:

      {
      long tmpPrio;

      if (!(CFlags & ((1 << fAdmin1) | (1 << fAdmin2))))
        {
        sprintf(Msg,"ERROR:  not authorized to run this command");

        return(FAILURE);
        }

      tmpPrio = strtol(Val,NULL,0); 

      if ((tmpPrio < 0) || (tmpPrio > 1000))
        {
        strcpy(Msg,"ERROR:  system priority must be in the range 0 - 1000");

        return(FAILURE);
        }

      if (strstr(ArgString,"relative") != NULL)
        tmpPrio += (MAX_PRIO_VAL << 1);
      
      MJobSetAttr(
        J,
        mjaSysPrio,
        (void **)&tmpPrio,
        mdfLong,
        (strstr(FlagString,"unset") != NULL) ? mUnset : mSet);

      sprintf(Msg,"job system priority adjusted");

      if (MSched.Mode == msmNormal)
        {
        MOSSyslog(LOG_INFO,"system prio set to %ld on job %s",
          tmpPrio,
          J->Name);
        }
      }    /* END BLOCK */

      break;

    case mjaQOS:

      {
      mqos_t *Q;

      int QAL[(MAX_MQOS >> 5) + 1];

      if (!(CFlags & ((1 << fAdmin1) | (1 << fAdmin2))) &&
         (JobOwner == FALSE))
        {
        sprintf(Msg,"ERROR:  request not authorized");

        return(FAILURE);
        }

      if (MQOSFind(Val,&Q) == FAILURE)
        {
        sprintf(Msg,"ERROR:  invalid QOS specified");

        return(FAILURE);
        }

      MUBMClear(QAL,MAX_MQOS);

      if (MQOSGetAccess(J,Q,QAL,NULL) == FAILURE)
        {
        DBG(2,fUI) DPrint("INFO:     job %s does not have access to QOS %s (QAL: %s)\n",
          J->Name,
          Q->Name,
          MQOSBMToString(QAL));

        sprintf(Msg,"ERROR:    job %s does not have access to QOS %s (QOSList: %s)\n",
          J->Name,
          Q->Name,
          MQOSBMToString(QAL));

        return(FAILURE);
        }

      /* set both requested and effective QOS */

      MJobSetAttr(
        J,
        mjaQOSReq,
        (void **)Val,
        mdfString,
        mSet);

      MJobSetAttr(
        J,
        mjaQOS,
        (void **)Val,
        mdfString,
        mSet);

      if (MSched.Mode == msmNormal)
        {
        MOSSyslog(LOG_INFO,"QOS changed from %s to %s on job %s",
          J->Cred.Q->Name,
          Q->Name,
          J->Name);
        }

      sprintf(Msg,"job QOS adjusted");
      }  /* END BLOCK */

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(AIndex) */
 
  return(SUCCESS);
  }  /* END MUIJobSetAttr() */




int MUISchedCtl(

  char *RBuffer,
  char *Buffer,
  int   FLAGS,
  char *Auth,
  long *BufSize)

  {
  long  SchedIteration;
  int   SchedMode;
  int   nindex;

  int   PIndex;
  int   VerboseFlag = FALSE;

  char  tmpLine[MAX_MLINE];
  char  Arg[MAX_MLINE];

  char *ptr;

  time_t tmpTime;

  mnode_t *N;

  const char *FName = "MUISchedCtl";

  DBG(2,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);

  MUSScanF(RBuffer,"%d",
    &SchedMode);

  ptr = RBuffer;

  /* FORMAT:  <WS><KEYWORD><WS><ARG> */

  while(isspace(*ptr) && (*ptr != '\0'))
    ptr++;

  while(!isspace(*ptr) && (*ptr != '\0'))
    ptr++;

  while(isspace(*ptr) && (*ptr != '\0'))
    ptr++;

  Arg[0] = '\0';
  SchedIteration = 0;

  if (ptr != '\0')
    {
    int tmpL;

    MUStrCpy(Arg,ptr,sizeof(Arg));

    if (strchr(Arg,':') != NULL)
      {
      tmpL = MUTimeFromString(Arg);

      SchedIteration = tmpL / MSched.RMPollInterval;
      }
    else
      {
      SchedIteration = strtol(Arg,NULL,10);
      }
    }

  if ((SchedMode == msctlStop) || (SchedMode == msctlStep))
    {
    if (strchr(Arg,'S') != NULL)
      {
      if (SchedMode == msctlStop)
        {
        IgnoreToTime = SchedIteration;
        }
      else
        {
        time(&tmpTime);

        IgnoreToTime = (long)tmpTime + SchedIteration;
        }

      sprintf(Buffer,"interface will block for %s\n",
        MULToTString(SchedIteration));

      return(SUCCESS);
      }
    }

  switch(SchedMode)
    {
    case msctlFailure:

      {
      long tmpL;

      tmpL = strtol(Arg,NULL,0);

      if (tmpL <= 0)
        {
        MSim.RMFailureTime = 0;

        sprintf(Buffer,"RM failure mode disabled\n");
        }
      else
        {
        MSim.RMFailureTime = MSched.Time + tmpL;

        sprintf(Buffer,"RM failure mode enabled for %s\n",
          MULToTString(tmpL));
        }
      }

      break;

    case msctlSubmit:

      if (MSimJobSubmit(MSched.Time,NULL,Arg,jtTrace) == SUCCESS)
        {
        sprintf(Buffer,"job successfully submitted\n");
        }
      else
        {
        sprintf(Buffer,"ALERT:  cannot submit job\n");
        }

      break;

    case msctlStop:

      if ((int)SchedIteration > MSched.Iteration)
        {
        DBG(2,fUI) DPrint("INFO:     scheduling will stop in %s at iteration %d\n",
          MULToTString(MSched.RMPollInterval * (SchedIteration - MSched.Iteration)),
          (int)SchedIteration);

        sprintf(Buffer,"scheduling will stop in %s at iteration %d\n",
          MULToTString(MSched.RMPollInterval * (SchedIteration - MSched.Iteration)),
          (int)SchedIteration);

        MSched.Schedule = TRUE;

        MSim.StopIteration = (int)SchedIteration;

        if (strchr(Arg,'I') != NULL)
          IgnoreToIteration = MSim.StopIteration;
        }
      else
        {
        DBG(2,fUI) DPrint("INFO:     scheduling will stop immediately at iteration %d\n",
          MSched.Iteration);

        sprintf(Buffer,"scheduling will stop immediately at iteration %d\n",
          MSched.Iteration);

        MSched.Schedule = FALSE;

        MSim.StopIteration = MSched.Iteration;

        IgnoreToIteration = 0;
        }

      if (MSched.Mode != msmSim)
        MOSSyslog(LOG_INFO,"scheduler stop command received");

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case msctlStep:

      if (SchedIteration > 0)
        {
        DBG(2,fUI) DPrint("INFO:     scheduling will stop in %s at iteration %d\n",
          MULToTString(SchedIteration * MSched.RMPollInterval),
          MSched.Iteration + (int)SchedIteration);

        sprintf(Buffer,"scheduling will stop in %s at iteration %d\n",
          MULToTString(SchedIteration * MSched.RMPollInterval),
          MSched.Iteration + (int)SchedIteration);

        MSched.Schedule = TRUE;

        MSim.StopIteration = MSched.Iteration + SchedIteration;

        if (strchr(Arg,'I') != NULL)
          IgnoreToIteration = MSim.StopIteration;
        }
      else
        {
        DBG(2,fUI) DPrint("INFO:     scheduling will stop in %s at iteration %d\n",
          MULToTString(MSched.RMPollInterval),
          MSched.Iteration + 1);

        sprintf(Buffer,"scheduling will stop in %s at iteration %d\n",
          MULToTString(MSched.RMPollInterval),
          MSched.Iteration + 1);

        MSched.Schedule = TRUE;

        MSim.StopIteration = MSched.Iteration + 1;

        if (strchr(Arg,'I') != NULL)
          IgnoreToIteration = MSim.StopIteration;
        }

      if (MSched.Mode != msmSim)
        MOSSyslog(LOG_INFO,"scheduler step command received");

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case msctlResume:

      MSim.StopIteration = -1;

      MSched.Schedule = TRUE;

      time(&tmpTime);

      UIDeadLine = (long)tmpTime + SchedIteration;

      DBG(2,fUI) DPrint("INFO:     scheduling will resume immediately\n");

      if (MSched.Mode == msmNormal)
        {
        MOSSyslog(LOG_INFO,"scheduler resume command received");
        }

      sprintf(Buffer,"scheduling will resume immediately\n");

      if (strchr(Arg,'I') != NULL)
        IgnoreToIteration = 0;

      *BufSize = strlen(Buffer);

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case msctlReconfig:

      sprintf(Buffer,"ERROR:  dynamic reconfiguration disabled.  restart server to incorporate changes\n");

      *BufSize = strlen(Buffer);

      return(FAILURE);

      /*NOTREACHED*/

      MSched.Reload = TRUE;

      DBG(2,fUI) DPrint("INFO:     scheduler will be reconfigured before next scheduling iteration\n");

      if (MSched.Mode != msmSim)
        MOSSyslog(LOG_NOTICE,"scheduler reconfig command received");

      sprintf(Buffer,"scheduler will be reconfigured before next scheduling iteration\n");

      *BufSize = strlen(Buffer);

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case msctlKill:

      DBG(2,fUI) DPrint("INFO:     scheduler will shutdown immediately\n");

      if (MSched.Mode != msmSim)
        MOSSyslog(LOG_NOTICE,"scheduler kill command received");

      sprintf(Buffer,"scheduler will be shutdown immediately\n");

      MSched.Shutdown = TRUE;

      *BufSize = strlen(Buffer);

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case msctlNodeTable:

      Buffer[0] = '\0';

      for (nindex = 0;nindex < MAX_MNODE;nindex++)
        {
        N = MNode[nindex];

        if ((N == NULL) || (N->Name[0] == '\0'))
          break;

        if (N->Name[0] == '\1')
          continue;

        if (MTraceBuildResource(
              N,
              DEFAULT_RESOURCE_TRACE_VERSION,
              tmpLine,
              sizeof(tmpLine)) == SUCCESS)
          {
          MUStrCat(Buffer,tmpLine,*BufSize);
          MUStrCat(Buffer,"\n",*BufSize);
          }
        }  /* END for (nindex) */

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case msctlModify:

      /* change scheduler parameter */

      ptr = Arg;

      MCfgAdjustBuffer(&ptr,FALSE);

      if (MCfgProcessBuffer(Arg) == FAILURE)
        {
        DBG(2,fUI) DPrint("WARNING:    config line '%s' cannot be processed\n",
          Arg);

        sprintf(Buffer,"ERROR:  specified parameters cannot be modified\n");

        return(FAILURE);
        }

      MSched.EnvChanged = TRUE;

      DBG(2,fUI) DPrint("INFO:     config line '%s' successfully processed\n",
        Arg);

      sprintf(Buffer,"parameters modified\n");

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case msctlList:

      PIndex = MUGetIndex(Arg,(const char **)MParam,FALSE,-1);

      if (strstr(Arg,ALL))
        VerboseFlag = TRUE;

      ConfigShow(Buffer,PIndex,VerboseFlag);

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case msctlInit:

      {
      /* initialize resource manager */

      if (MRM[0].Type != mrmtLSF)
        {
        sprintf(Buffer,"initialize command not supported for RM\n");

        return(SUCCESS);
        }
     
      /* process optional server config */

      /* FORMAT:  <SERVERHOST>[<@<SERVERPORT>] */

      if ((ptr[0] != '\0') && strcmp(ptr,NONE))
        {
        char *TokPtr;
        char *ptr2;

        if ((ptr2 = MUStrTok(ptr,"@",&TokPtr)) != NULL)
          {
          MRMSetAttr(&MRM[0],mrmaHost,(void **)ptr2,mdfString,0);

          if ((ptr2 = MUStrTok(NULL,"@",&TokPtr)) != NULL)
            {
            MRMSetAttr(&MRM[0],mrmaPort,(void **)ptr2,mdfString,0);
            }
          }
        }    /* END BLOCK */

      MRMInitialize();

      sprintf(Buffer,"RM intialized\n");

      return(SUCCESS);
      }  /* END BLOCK */

      /*NOTREACHED*/

      break;

    default:

      DBG(2,fUI) DPrint("WARNING:  received unexpected sched command '%d'\n",
        SchedMode);

      sprintf(Buffer,"ERROR:    unexpected simulation command: '%d'\n",
        SchedMode);

      if (BufSize != NULL)
        *BufSize = strlen(Buffer);

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(SchedMode) */

  return(SUCCESS);
  }  /* END MUISchedCtl() */



int UIDiagnose(

  char *RBuffer,  /* I */
  char *SBuffer,  /* O */
  int   FLAGS,    /* I */
  char *Auth,     /* I */
  long *SBufSize) /* I */

  {
  int  OType;
  char FlagString[MAX_MNAME];

  mpar_t *P;

  char PName[MAX_MNAME];
  char DiagOpt[MAX_MNAME];

  int  IFlags;

  const char *FName = "UIDiagnose";

  if ((SBuffer == NULL) || (RBuffer == NULL))
    {
    return(FAILURE);
    }

  DBG(2,fUI) DPrint("%s(%s,SBuffer,%d,%s,SBufSize)\n",
    FName,
    RBuffer,
    FLAGS,
    Auth);

  MUSScanF(RBuffer,"%d %x%s %x%s %x%s",
    &OType,
    sizeof(FlagString),
    FlagString,
    sizeof(PName),
    PName,
    sizeof(DiagOpt),
    DiagOpt);

  IFlags = (int)strtol(FlagString,NULL,0);

  if (MParFind(PName,&P) == FAILURE)
    {
    P = &MPar[0];
    }

  switch (OType)
    {
    case mxoAcct:

      UIAcctDiagnose(SBuffer,SBufSize,DiagOpt,IFlags);

      break;

    case mxoClass:

      {
      mclass_t *C;

      MClassFind(DiagOpt,&C);

      MClassShow(C,SBuffer,SBufSize,0);
      }  /* END BLOCK */

      break;

    case mxoFrame:

      MFrameShow(DiagOpt,P,SBuffer,(int)*SBufSize,IFlags);

      break;

    case mxoFS:

      {
      int BufSize = MAX_MBUFFER;

      /* DsT Added DiagOpt, so we can limit the output */
      MFSShow(SBuffer,BufSize,DiagOpt,IFlags);

      *SBufSize = strlen(SBuffer);
      }  /* END BLOCK */

      break;

    case mxoGroup:

      UIGroupDiagnose(SBuffer,SBufSize,DiagOpt,IFlags);

      break;

    case mxoJob:

      MUIJobDiagnose(SBuffer,SBufSize,P->Index,DiagOpt,IFlags);

      break;

    case mxoNode:

      UINodeDiagnose(SBuffer,SBufSize,P->Index,DiagOpt,IFlags);

      break;

    case mxoPar:

      MParDiagnose(SBuffer,SBufSize,DiagOpt);

      break;

    case mxoPriority:

      UIDiagnosePriority(SBuffer,SBufSize,P);

      break;

    case mxoQOS:

      MQOSShow(DiagOpt,SBuffer,SBufSize,0);

      break;

    case mxoQueue:

      /* NOTE:  pass PLevel via Flags */

      UIQueueDiagnose(SBuffer,SBufSize,IFlags,P->Index);

      break;

    case mxoRsv:

      UIResDiagnose(SBuffer,SBufSize,P->Index,DiagOpt,IFlags);

      break;

    case mxoRM:

      MRMShow(NULL,SBuffer,(int)*SBufSize,IFlags);

      break;

    case mxoSched:

      MSchedDiag(&MSched,SBuffer,(int)*SBufSize,IFlags);

      break;

    case mxoSRes:

      MSRDiag(NULL,SBuffer,(int)*SBufSize,IFlags);

      break;

    case mxoSys:

      MSysDiagnose(SBuffer,(int)*SBufSize,IFlags);

      break;

    case mxoUser:

      UIUserDiagnose(SBuffer,SBufSize,DiagOpt,IFlags);

      break;

    default:

      DBG(2,fUI) DPrint("ALERT:    unexpected diagnose type, '%s'\n",
        MXO[OType]);

      sprintf(SBuffer,"ERROR:  unexpected diagnose type, '%s'\n",
        MXO[OType]);

      if (SBufSize != NULL)
        *SBufSize = strlen(SBuffer);

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch (OType) */

  if (SBufSize != NULL)
    *SBufSize = strlen(SBuffer);

  return(SUCCESS);
  }  /* END UIDiagnose() */




int UIResCreate(

  char *RBuffer,  /* I */
  char *SBuffer,  /* O */
  int   FLAGS,    /* I */
  char *Auth,     /* I */
  long *SBufSize) /* O */

  {
  long  ClientTime;
  long  StartTime;
  long  EndTime;

  mpar_t *P;

  char  UserList[MAX_MLINE];
  char  GroupList[MAX_MLINE];
  char  AccountList[MAX_MLINE];
  char  QOSList[MAX_MLINE];
  char  ClassList[MAX_MLINE];
  char  FeatureList[MAX_MLINE];
  char  JobFeatureList[MAX_MLINE];

  char  ResID[MAX_MNAME];

  char  ResourceList[MAX_MLINE];

  char  NodeSetString[MAX_MLINE];
  char  FlagString[MAX_MLINE];

  char  Name[MAX_MNAME];

  char  PName[MAX_MNAME];

  int   nindex;
  int   index;

  char  Pattern[MAX_MBUFFER];
  char *PatPtr;
  int   PatSize;

  char *ptr;
  char *TokPtr;

  char  Message[MAX_MLINE];

  short ObjList[MAX_MNODE];

  mnalloc_t NodeList[MAX_MNODE];

  int   MaxTasks;

  int   TaskCount;
  int   NodeCount;

  int   NIndex;
  int   RIndex;

  char  ChargeSpec[MAX_MLINE];

  int   PC;
  int   SPC;

  macl_t  ACL[MAX_MACL];
  char   *ACLList[1];
  mres_t *ResP;

  mgcred_t  *U;
  mgcred_t *G;
  mgcred_t  *A;

  mreq_t tmpRQ;

  unsigned long Flags;

  int    Access;
  int    ResPLevel;

  const char *FName = "UIResCreate";

  DBG(2,fUI) DPrint("%s(%s,SBuffer,%d,%s,SBufSize)\n",
    FName,
    RBuffer,
    FLAGS,
    Auth);

  /* security check - allow access to admin1 or admin2 */

  Access = FALSE;

  ResPLevel = MSched.ResLimitPolicy;

  if ((FLAGS & ((1 << fAdmin1) | (1 << fAdmin2))))
    {
    Access = TRUE;

    ResPLevel = ptOFF;
    }
  else if (MSched.ResCtlPolicy == mrcpAny)
    {
    Access = TRUE;
    }

  if (Access == FALSE)
    {
    DBG(2,fUI) DPrint("INFO:     user %s is not authorized to create reservation\n",
      Auth);

    sprintf(SBuffer,"user %s is not authorized create reservation\n",
      Auth);

    return(FAILURE);
    }


  /* Format:  <CLIENT_PRESENTTIME> <START_TIME> <END_TIME> <PNAME> <ULIST> <GLIST> <ALIST> <CLIST> <QLIST> <RESID> <RLIST> <CANAME> <NODEEXP> <FLIST> <NODESET> <FLAGS> */

  MUSScanF(RBuffer,"%ld %ld %ld %x%s %x%s %x%s %x%s %x%s %x%s %x%s %x%s %x%s %x%s %x%s %x%s %x%s %d %x%s",
    &ClientTime,
    &StartTime,
    &EndTime,
    sizeof(PName),
    PName,
    sizeof(UserList),
    UserList,
    sizeof(GroupList),
    GroupList,
    sizeof(AccountList),
    AccountList,
    sizeof(ClassList),
    ClassList,
    sizeof(QOSList),
    QOSList,
    sizeof(ResID),
    ResID,
    sizeof(ResourceList),
    ResourceList,
    sizeof(ChargeSpec),
    ChargeSpec,
    sizeof(Pattern),
    Pattern,
    sizeof(FeatureList),
    FeatureList,
    sizeof(NodeSetString),
    NodeSetString,
    sizeof(FlagString),
    FlagString,
    &MaxTasks,
    sizeof(JobFeatureList),
    JobFeatureList);

  DBG(5,fUI) DPrint("INFO:     processing %s: CT: %ld  ST: %ld  ET: %ld  PI: %s  UL: %s  GL: %s  AL: %s  QL: %s  RL: %s  AN: %s  P: '%s'  F: '%s'  NS: %s  FL: '%s'\n",
    FName,
    ClientTime,
    StartTime,
    EndTime,
    PName,
    UserList,
    GroupList,
    AccountList,
    QOSList,
    ResourceList,
    ChargeSpec,
    Pattern,
    FeatureList,
    NodeSetString,
    FlagString);

  if (MParFind(PName,&P) == FAILURE)
    P = &MPar[0];

  /* adjust for simulation time discrepancies */

  if (MSched.Mode == msmSim)
    {
    StartTime += (MSched.Time - ClientTime);

    if (EndTime < MAX_MTIME)
      EndTime += (MSched.Time - ClientTime);
    }

  StartTime = MAX(StartTime,MSched.Time);

  if (StartTime >= EndTime)
    {
    DBG(3,fUI) DPrint("WARNING:  cannot create reservation (invalid timeframe)\n");

    sprintf(temp_str,"cannot create reservation (invalid timeframe)\n");
    strcat(SBuffer,temp_str);

    return(FAILURE);
    }

  memset(&tmpRQ,0,sizeof(tmpRQ));

  SBuffer[0] = '\0';

  /* process resource list */

  memset(&tmpRQ.DRes,0,sizeof(tmpRQ.DRes));

  if (!strcmp(ResourceList,NONE))
    {
    tmpRQ.DRes.Procs = -1;
    tmpRQ.DRes.Mem   = 0;
    tmpRQ.DRes.Swap  = 0;
    tmpRQ.DRes.Disk  = 0;
    }
  else
    {
    MUCResFromString(&tmpRQ.DRes,ResourceList);
    }    /* else (!strcmp(ResourceList,NONE)) */

  /* process feature list */

  if (strcmp(FeatureList,NONE) != 0)
    {
    if (strchr(FeatureList,'|'))
      tmpRQ.ReqFMode = tlOR;

    ptr = MUStrTok(FeatureList,":|",&TokPtr);

    while (ptr != NULL)
      {
      MUGetMAttr(eFeature,ptr,mAdd,tmpRQ.ReqFBM,sizeof(tmpRQ.ReqFBM));

      DBG(6,fUI) DPrint("INFO:     feature '%s' added (%s)\n",
        ptr,
        MUMAList(eFeature,tmpRQ.ReqFBM,sizeof(tmpRQ.ReqFBM)));

      ptr = MUStrTok(NULL,":|",&TokPtr);
      }
    }

  /* process node set list */

  if (strcmp(NodeSetString,NONE) != 0)
    {
    /* FORMAT:  <SETSELECTION>:<SETTYPE>[:<SETLIST>] */

    if ((ptr = MUStrTok(NodeSetString,":",&TokPtr)) != NULL)
      {
      tmpRQ.SetSelection = MUGetIndex(ptr,MResSetSelectionType,0,mrssNONE);

      if ((ptr = MUStrTok(NULL,":",&TokPtr)) != NULL)
        {
        tmpRQ.SetType = MUGetIndex(ptr,MResSetAttrType,0,mrstNONE);

        index = 0;

        while ((ptr = MUStrTok(NULL,":,",&TokPtr)) != NULL)
          {
          MUStrDup(&tmpRQ.SetList[index],ptr);

          index++;
          }
        }
      else
        {
        sprintf(SBuffer,"ERROR:    invalid node set expression: '%s'\n",
          NodeSetString);

        return(FAILURE);
        }
      }
    else
      {
      sprintf(SBuffer,"ERROR:    invalid node set expression: '%s'\n",
        NodeSetString);

      return(FAILURE);
      }
    }    /* END if (strcmp(NodeSetString,NONE) != 0) */

  /* set flags */

  Flags = 0;

  if (!strcmp(FlagString,NONE))
    {
    MUBMFromString(FlagString,MResFlags,&Flags);
    }

  if (strstr(Pattern,"TASKS") != NULL)
    {
    /* if TASKS specified */

    if (MResAllocateRE(
          Pattern,
          sizeof(Pattern),
          P->Index,
          NodeList,    /* O */
          &TaskCount,
          StartTime,
          EndTime,
          SBuffer,
          *SBufSize,
          &tmpRQ) == FAILURE)
      {
      sprintf(temp_str,"ERROR:    cannot select requested tasks for '%s'\n",
        Pattern);
      strcat(SBuffer,temp_str);

      return(FAILURE);
      }

    for (nindex = 0;NodeList[nindex].N != NULL;nindex++);

    NodeCount = nindex;
    }
  else
    {
    if (MUREToList(
          Pattern,
          mxoNode,
          P->Index,
          ObjList,
          &NodeCount,
          SBuffer) == FAILURE)
      {
      sprintf(temp_str,"ERROR:    cannot determine nodelist for '%s'\n",
        Pattern);
      strcat(SBuffer,temp_str);

      return(FAILURE);
      }

    TaskCount = 0;

    for (nindex = 0;nindex < NodeCount;nindex++)
      {
      NodeList[nindex].N  = MNode[ObjList[nindex]];
      NodeList[nindex].TC = 1;

      TaskCount++;

      if (MaxTasks > 0)
        {
        if (TaskCount >= MaxTasks)
          {
          nindex++;

          break;
          }
        }
      }    /* END for (nindex) */

    if ((nindex == 1) && (MaxTasks <= 0))
      {
      /* NOTE:  enforce 'customary' behavior */

      MaxTasks = 1;
      }

    NodeList[nindex].N = NULL;

    NodeCount = nindex;
    }  /* END else (strstr(Pattern,"TASKS") != NULL) */

  if (TaskCount > 0)
    {
    /* filter selected nodes */

    NIndex = 0;

    for (nindex = 0;nindex < NodeCount;nindex++)
      {
      if (NodeList[nindex].N == NULL)
        break;

      if ((P->Index != 0) && 
          (NodeList[nindex].N->PtIndex != P->Index))
        {
        continue;
        }

      if (MReqCheckResourceMatch(
            NULL,
            &tmpRQ,
            NodeList[nindex].N,
            &RIndex) == FAILURE)
        {
        continue;
        }

      NodeList[NIndex].N  = NodeList[nindex].N;
      NodeList[NIndex].TC = NodeList[nindex].TC;

      NIndex++;
      }    /* END for (nindex) */

    NodeList[NIndex].N = NULL;

    DBG(4,fUI) DPrint("INFO:     reservation constraints reduced nodelist from %d to %d nodes\n",
      NodeCount,
      NIndex);

    NodeCount = NIndex;
    }

  /* When the feature list is set, and a HOST_REGEX or "ALL"
   * is used, we replace the Pattern with the resulting nodelist.
   * This ensures the reservation is checkpointed properly */

  if ((strcmp(FeatureList,NONE) != 0) && (strstr(Pattern,"TASKS") == NULL))
    {
    PatPtr   = Pattern;
    PatSize = sizeof(Pattern);
    PatPtr[0] = '\0';

    for (nindex = 0;nindex < NodeCount;nindex++)
      {
      if (PatSize < 100)
        {
        DBG(0,fSCHED) DPrint("ERROR:    regex buffer overflow creating reservation '%s'\n", ResID);

        return(FAILURE);
        }

      if (nindex != 0)
        {
        MUSNPrintF(&PatPtr,&PatSize,"|%s",
          NodeList[nindex].N->Name);
        }
      else
        {
        MUSNPrintF(&PatPtr,&PatSize,"%s",
          NodeList[nindex].N->Name);
        }
      }
    }

  /* translate dedicated proc request to per node proc request */

  SPC = tmpRQ.DRes.Procs;

  if (tmpRQ.DRes.Procs == -1)
    {
    for (nindex = 0;NodeList[nindex].N != NULL;nindex++)
      {
      NodeList[nindex].TC = NodeList[nindex].N->CRes.Procs;
      }  /* END for (nindex) */

    tmpRQ.DRes.Procs = 1;
    }

  /* build ACL */

  memset(ACL,0,sizeof(ACL));

  if (strcmp(UserList,NONE) != 0)
    {
    ACLList[0] = UserList;

    MACLLoadConfig(ACL,ACLList,1,maUser);
    }

  if (strcmp(GroupList,NONE) != 0)
    {
    ACLList[0] = GroupList;

    MACLLoadConfig(ACL,ACLList,1,maGroup);
    }

  if (strcmp(AccountList,NONE) != 0)
    {
    ACLList[0] = AccountList;

    MACLLoadConfig(ACL,ACLList,1,maAcct);
    }

  if (strcmp(QOSList,NONE) != 0)
    {
    ACLList[0] = QOSList;

    MACLLoadConfig(ACL,ACLList,1,maQOS);
    }

  if (strcmp(ClassList,NONE) != 0)
    {
    ACLList[0] = ClassList;

    MACLLoadConfig(ACL,ACLList,1,maClass);
    }

  if (strcmp(JobFeatureList,NONE) != 0)
    {
    ACLList[0] = JobFeatureList;

    MACLLoadConfig(ACL,ACLList,1,maJFeature);
    }

  if (strcmp(ResID,NONE))
    {
    strcpy(Name,ResID);
    }
  else if (ACL[0].Type == maNONE)
    {
    strcpy(Name,"SYSTEM");
    }
  else
    {
    strcpy(Name,ACL[0].Name);
    }

  A = NULL;
  G = NULL;
  U = NULL;

  /* charge creds may only be specified by admins */

  if ((FLAGS & ((1 << fAdmin1) | (1 << fAdmin2))))
    {
    if (strcmp(ChargeSpec,NONE))
      {
      /* FORMAT:  ACCOUNT[,GROUP[,USER]] */

      ptr = MUStrTok(ChargeSpec," \n\t:,",&TokPtr);

      if (ptr != NULL)
        {
        MAcctAdd(ptr,&A);

        ptr = MUStrTok(NULL," \n\t:,",&TokPtr);
        }

      if (ptr != NULL)
        {
        MGroupAdd(ptr,&G);

        ptr = MUStrTok(NULL," \n\t:,",&TokPtr);
        }

      if (ptr != NULL)
        {
        MUserAdd(ptr,&U);
        }
      }    /* END if (strcmp(ChargeSpec,NONE)) */
    }      /* END if ((FLAGS & ((1 << fAdmin1) | (1 << fAdmin2)))) */
  else
    {
    /* requesting user is accountable for reservation */

    if ((Auth != NULL) && (MUserAdd(Auth,&U) == SUCCESS))
      {
      int GID;
      
      char *GName;

      mjob_t tmpJ;

      /* determine group and account from user */

      if ((GID = MUGIDFromUID(U->OID)) != -1)
        {
        GName = MUGIDToName(GID);

        MGroupAdd(GName,&G);        
        }

      tmpJ.Cred.U = U;
      tmpJ.Cred.G = G;
      tmpJ.Cred.Q = NULL;

      MJobGetAccount(&tmpJ,&A);      
      }  /* END if ((Auth != NULL) && (MUserAdd(Auth,&U) == SUCCESS)) */
    }    /* END else ((FLAGS & ((1 << fAdmin1) | (1 << fAdmin2)))) */

  /* verify resources exist to cover reservation (NYI) */

  /* allocations? */

  TaskCount = 0;

  for (nindex = 0;NodeList[nindex].N != NULL;nindex++)
    {
    TaskCount += NodeList[nindex].TC;
    }  /* END for (nindex) */

  PC = TaskCount * tmpRQ.DRes.Procs;

  if (ResPLevel != ptOFF)
    {
    mjob_t  tmpJ;

    mjob_t *J;

    long    tmpTime;

    tmpTime = StartTime;

    J = &tmpJ;

    /* populate J */

    strcpy(J->Name,"rescreate");

    J->SpecWCLimit[0]   = EndTime - StartTime;
    J->C.TotalProcCount = PC;
    J->Cred.U           = U;
    J->Cred.A           = A;
    J->Cred.G           = G;
    J->Cred.Q           = NULL;
    J->Cred.C           = NULL;

    if (MPolicyGetEStartTime(
          J,
          P,
          ResPLevel,
          &tmpTime) == FAILURE)
      {
      sprintf(temp_str,"ERROR:    limits prevent reservation creation\n");
      strcat(SBuffer,temp_str);

      return(FAILURE);
      }

    if (tmpTime != StartTime)
      {
      sprintf(temp_str,"ERROR:    cannot reserve requested resources until %s\n",
        MULToTString(tmpTime - MSched.Time));
      strcat(SBuffer,temp_str);

      return(FAILURE);
      }
    }    /* END if (ResPLevel != ptOFF) */

  /* convert dedicated resource usage back */

  if (SPC == -1)
    {
    /* restore dedicated resource usage specification */

    for (nindex = 0;NodeList[nindex].N != NULL;nindex++)
      {
      NodeList[nindex].TC = 1;
      }  /* END for (nindex) */

    tmpRQ.DRes.Procs = -1;
    }  /* END if (SPC == 1) */

  if (MResCreate(
        mrtUser,
        ACL,
        (A != NULL) ? A->Name : NULL,
        Flags,
        NodeList,
        StartTime,
        EndTime,
        NodeCount,
        PC,
        Name,
        &ResP,
        Pattern,
        &tmpRQ.DRes) == FAILURE)
    {
    DBG(3,fUI) DPrint("WARNING:  cannot create reservation for %s on %d node%s\n",
      Name,
      NodeCount,
      (NodeCount == 1) ? "" : "s");

    sprintf(temp_str,"cannot create reservation for '%s' on %d node%s\n",
      Name,
      NodeCount,
      (NodeCount == 1) ? "" : "s");
    strcat(SBuffer,temp_str);

    return(FAILURE);
    }  /* END if (MResCreate() == FAILURE) */

  ResP->Priority = 0;

  if (A != NULL)
    {
    MResSetAttr(ResP,mraAAccount,(void *)A,0,mSet);
    }

  if (G != NULL)
    {
    MResSetAttr(ResP,mraAGroup,(void *)G,0,0);
    }

  if (U != NULL)
    {
    MResSetAttr(ResP,mraAUser,(void *)U,0,0);
    }

  if (MaxTasks > 0)
    {
    MResSetAttr(ResP,mraMaxTasks,(void *)&MaxTasks,mdfInt,0);
    }

  if (ResP->Flags | (1 << mrfPreemptible))
    {
    ResP->Flags ^= (1 << mrfPreemptible);
    }

  MSched.EnvChanged = TRUE;

  DBG(3,fUI) DPrint("INFO:     reservation '%s' created on %d node%s (%d tasks)\n",
    ResP->Name,
    NodeCount,
    (NodeCount == 1) ? "" : "s",
    TaskCount);

  sprintf(SBuffer,"reservation '%s' created on %d node%s (%d tasks)\n",
    ResP->Name,
    NodeCount,
    (NodeCount == 1) ? "" : "s",
    TaskCount);

  for (nindex = 0;nindex < NodeCount;nindex++)
    {
    sprintf(temp_str,"%s:%d\n",
      NodeList[nindex].N->Name,
      NodeList[nindex].TC);
    strcat(SBuffer,temp_str);
    }  /* END for (nindex) */

  sprintf(Message,"RESERVATIONCREATION:  %s USER %s %ld %ld %d %d\n",
    ResP->Name,
    Name,
    StartTime,
    EndTime,
    TaskCount,
    NodeCount);

  MSysRegEvent(Message,0,0,1);

  if (MSched.Mode == msmNormal)
    {
    MOSSyslog(LOG_INFO,"reservation %s created on %d node%s",
      ResP->Name,
      NodeCount,
      (NodeCount == 1) ? "" : "s");
    }

  return(SUCCESS);
  }  /* END UIResCreate() */




int MUIQueueShow(

  msocket_t *S,
  mxml_t    *ReqE,
  mxml_t   **RspE)

  {
  int     jindex;

  int     aindex;
  int     sindex;

  int     IdleJob;
  int     TotalAllocProcs;

  int     SuspQ[MAX_MJOB];

  mjob_t *J;

  mpar_t *P;

  mreq_t *RQ;

  mxml_t *E;

  mxml_t *RE = NULL;
  mxml_t *CE = NULL;
  mxml_t *QE = NULL;
  mxml_t *JE = NULL;

  const char *FName = "MUIQueueShow";

  DBG(2,fUI) DPrint("%s(S,ReqE,RspE)\n",
    FName);

  if ((S == NULL) || (ReqE == NULL) || (RspE == NULL))
    return(FAILURE);

  P = &MPar[0];

  if (*RspE == NULL)
    {
    MUISMsgAdd(S,"ERROR:    internal failure\n");

    return(FAILURE);
    }

  E = *RspE;

  RE = NULL;

  if (MXMLCreateE(&RE,"Response") == FAILURE)
    {
    MUISMsgAdd(S,"ERROR:    internal failure\n");

    return(FAILURE);
    }

  MXMLAddE(E,RE);

  MXMLSetAttr(RE,"object",(void *)MXO[mxoQueue],mdfString);

  /* add cluster stats */

  CE = NULL;

  if (MXMLCreateE(&CE,(char *)MXO[mxoCluster]) == FAILURE)
    {
    MUISMsgAdd(S,"ERROR:    internal failure\n");

    return(FAILURE);
    }

  MXMLAddE(RE,CE);
  
  TotalAllocProcs = 0;

  for (aindex = 0;MAQ[aindex] != -1;aindex++)
    {
    J = MJob[MAQ[aindex]];

    TotalAllocProcs += MJobGetProcCount(J);
    }  /* END for (aindex) */

  /* provide cluster stats */
 
  MXMLSetAttr(CE,"time",(void *)&MSched.Time,mdfLong);
  MXMLSetAttr(CE,"upProcs",(void *)&P->URes.Procs,mdfInt);
  MXMLSetAttr(CE,"idleProcs",(void *)&P->ARes.Procs,mdfInt);
  MXMLSetAttr(CE,"upNodes",(void *)&P->UpNodes,mdfInt);
  MXMLSetAttr(CE,"idleNodes",(void *)&P->IdleNodes,mdfInt);
  MXMLSetAttr(CE,"activeNodes",(void *)&P->ActiveNodes,mdfInt);
  MXMLSetAttr(CE,"allocProcs",(void *)&TotalAllocProcs,mdfInt);

  QE = NULL;

  if (MXMLCreateE(&QE,(char *)MXO[mxoQueue]) == FAILURE)
    {
    MUISMsgAdd(S,"ERROR:    internal failure\n");

    return(FAILURE);
    }

  MXMLAddE(RE,QE);

  MXMLSetAttr(QE,"type","active",mdfString);

  /* locate suspended jobs */

  sindex = 0;

  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    if (J->State == mjsSuspended)
      SuspQ[sindex++] = J->Index;
    }  /* END for (J) */

  SuspQ[sindex] = -1;

  /* list active jobs */

  aindex = 0;
  sindex = 0;

  while ((MAQ[aindex] != -1) || (SuspQ[sindex] != -1))
    {
    if (MAQ[aindex] != -1)
      {
      J = MJob[MAQ[aindex++]];

      if (J->State == mjsSuspended)
        continue;
      }
    else
      {
      J = MJob[SuspQ[sindex++]];
      }

    RQ = J->Req[0];  /* handle only first req of job */

    if ((P->Index > 0) && (RQ->PtIndex != P->Index))
      continue;

    if (__MUIJobToXML(J,&JE,FALSE) == FAILURE)
      continue;

    DBG(3,fUI) DPrint("INFO:     %s:  adding active job[%03d] '%s' to buffer\n",
      FName,
      J->Index,
      J->Name);

    MXMLAddE(QE,JE);
    }  /* END while (while ((MAQ[aindex] != -1) || (SuspQ[sindex] != -1)) */

  /* list eligible jobs */

  QE = NULL;

  if (MXMLCreateE(&QE,(char *)MXO[mxoQueue]) == FAILURE)
    {
    MUISMsgAdd(S,"ERROR:    internal failure\n");

    return(FAILURE);
    }

  MXMLAddE(RE,QE);

  MXMLSetAttr(QE,"type","eligible",mdfString);

  for (jindex = 0;MUIQ[jindex] != -1;jindex++)
    {
    J = MJob[MUIQ[jindex]];

    if ((J == NULL) || (J->Name[0] == '\0') || (J->Name[0] == '\1'))
      continue;

    RQ = J->Req[0];  /* handle only first req of job */

    if ((RQ == NULL) || (J->Cred.U == NULL))
      {
      /* corrupt job record */

      continue;
      }

    if ((P->Index > 0) && (MUBMCheck(P->Index,J->PAL) == FAILURE))
      continue;

    if ((J->State == mjsRemoved) || (J->State == mjsSuspended))
      continue;

    if (__MUIJobToXML(J,&JE,FALSE) == FAILURE)
      continue;

    DBG(3,fUI) DPrint("INFO:     %s:  adding eligible job[%03d] '%s' to buffer\n",
      FName,
      J->Index,
      J->Name);

    MXMLAddE(QE,JE);
    }  /* END for (jindex = 0;UIQ[jindex] != -1;jindex++) */

  /* list blocked jobs */

  QE = NULL;

  if (MXMLCreateE(&QE,(char *)MXO[mxoQueue]) == FAILURE)
    {
    MUISMsgAdd(S,"ERROR:    internal failure\n");

    return(FAILURE);
    }

  MXMLAddE(RE,QE);

  MXMLSetAttr(QE,"type","blocked",mdfString);

  /* list blocked jobs */

  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    if ((P->Index > 0) && (MUBMCheck(P->Index,J->PAL) == FAILURE))
      continue;

    /* continue if active job found */

    if ((J->State == mjsRunning) ||
        (J->State == mjsStarting) ||
        (J->State == mjsSuspended) ||
        (J->IState != mjsNONE))
      {
      continue;
      }

    IdleJob = FALSE;

    for (jindex = 0;MUIQ[jindex] != -1;jindex++)
      {
      if (MUIQ[jindex] == J->Index)
        {
        IdleJob = TRUE;

        break;
        }
      }    /* END for (jindex) */

    if (IdleJob == TRUE)
      continue;

    if (__MUIJobToXML(J,&JE,FALSE) == FAILURE)
      continue;

    DBG(3,fUI) DPrint("INFO:     %s:  adding blocked job[%03d] '%s' to buffer\n",
      FName,
      J->Index,
      J->Name);

    MXMLAddE(QE,JE);
    }  /* END for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next) */

  return(SUCCESS);
  }  /* END MUIQueueShow() */




int __MUIJobToXML(

  mjob_t   *J,
  mxml_t **JEP,
  int       DoShowTaskList)

  {

  long Duration;
  int  Procs;
  
  mxml_t *JE;

  if ((J == NULL) || (JEP == NULL))
    return(FAILURE);

  /* job attributes specified:  
   *  state, wclimit, jobid, user, starttime, submittime, procs, qos 
   *  nodecount, group 
   */

  *JEP = NULL;

  if (MXMLCreateE(JEP,(char *)MXO[mxoJob]) == FAILURE)
    {
    return(FAILURE);
    }

  JE = *JEP;

  if ((MPar[0].UseCPUTime == TRUE) && (J->CPULimit > 0))
    Duration = J->CPULimit;
  else
    Duration = J->WCLimit;

  Procs = MJobGetProcCount(J);

  MXMLSetAttr(JE,(char *)MJobAttr[mjaJobName],(void *)J->Name,mdfString);
  MXMLSetAttr(JE,(char *)MJobAttr[mjaState],(void *)MJobState[J->State],mdfString);

  if (J->Cred.U != NULL)
    MXMLSetAttr(JE,(char *)MJobAttr[mjaUser],(void *)J->Cred.U->Name,mdfString);

  if (J->Cred.G != NULL)
    MXMLSetAttr(JE,(char *)MJobAttr[mjaGroup],(void *)J->Cred.G->Name,mdfString);

  MXMLSetAttr(JE,(char *)MJobAttr[mjaStartTime],(void *)&J->StartTime,mdfLong);
  MXMLSetAttr(JE,(char *)MJobAttr[mjaSubmitTime],(void *)&J->SubmitTime,mdfLong);

  MXMLSetAttr(JE,(char *)MJobAttr[mjaReqAWDuration],(void *)&Duration,mdfLong);
  MXMLSetAttr(JE,(char *)MJobAttr[mjaReqProcs],(void *)&Procs,mdfInt);

  if (J->Request.NC > 0)
    MXMLSetAttr(JE,(char *)MJobAttr[mjaReqNodes],(void *)&J->Request.NC,mdfInt);

  if ((J->Cred.Q != NULL) && (J->Cred.Q->Index != 0))
    MXMLSetAttr(JE,(char *)MJobAttr[mjaQOSReq],(void *)J->Cred.Q->Name,mdfString);

  if (DoShowTaskList == TRUE)
    {
    int nindex;

    mnode_t *N;

    char tmpLine[MAX_MBUFFER];

    tmpLine[0] = '\0';

    for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
      {
      N = J->NodeList[nindex].N;

      sprintf(temp_str,"%d:%d;",
        N->FrameIndex,
        N->SlotIndex);
      strcat(tmpLine,temp_str);

      DBG(4,fUI) DPrint("INFO:     adding node '%s' of job '%s' to buffer\n",
        N->Name,
        J->Name);
      }  /* END for (nindex) */

    if (tmpLine[0] != '\0')
      MXMLSetAttr(JE,(char *)MJobAttr[mjaAllocNodeList],(void *)tmpLine,mdfString);
    }  /* END if (DoShowTaskList == TRUE) */

  return(SUCCESS);
  }  /* END __MUIJobToXML() */

int UIShowUserTasks(

	char 	 *RBuffer,   /* I */
  char   *SBuffer,   /* O */
  long   *SBufSize)
	{
  int     sumOfProcs;


  char    userName[MAX_MLINE];
  char    Line[MAX_MLINE];

  char   *BPtr;
  int     BSpace;

  MUSScanF(RBuffer,"%s",userName);
  BPtr   =  SBuffer;
  BSpace = *SBufSize;

  sumOfProcs = getUserTasks(userName);

  sprintf(Line,"%d\n",sumOfProcs);

  MUStrNCat(&BPtr,&BSpace,Line);

  return(SUCCESS);
	}

int getUserTasks(

	char *userName)
	{
  int     aindex;
  int     sindex;
  int 		sumOfProcs;
  int     SuspQ[MAX_MJOB];

  /* locate suspended jobs */

  sindex = 0;
  sumOfProcs = 0;

  mjob_t *J;
  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    if (J->State == mjsSuspended)
      SuspQ[sindex++] = J->Index;
    }  /* END for (J) */

  SuspQ[sindex] = -1;

  /* Calculate tasks */

  aindex = 0;
  sindex = 0;

  while ((MAQ[aindex] != -1) || (SuspQ[sindex] != -1)) {
    if (MAQ[aindex] != -1) {
      J = MJob[MAQ[aindex++]];

      if (J->State == mjsSuspended)
        continue;
    } else {
      J = MJob[SuspQ[sindex++]];
    }
    if (J->Cred.U != NULL) {
      if (!strcmp(userName, J->Cred.U->Name))
        sumOfProcs = sumOfProcs + MJobGetProcCount(J);
    }
  } /* END while (MAQ[aindex] != -1) */

  return sumOfProcs;
	}




int UIQueueShowAllJobs(

  char   *SBuffer,  /* O */
  long   *SBufSize, /* I */
  mpar_t *P,        /* I */
  char   *UName)    /* I */

  {
  int     jindex;

  int     aindex;
  int     sindex;

  int     IdleJob;
  int     tmpState;
  int     TotalAllocatedProcs;

  int     SuspQ[MAX_MJOB];

  mjob_t *J;

  char    Line[MAX_MLINE];

  long    Limit;

  mreq_t *RQ;

  int     IsTruncated = FALSE;

  char   *BPtr;
  int     BSpace;

  const char *FName = "UIQueueShowAllJobs";

  DBG(2,fUI) DPrint("%s(SBuffer,SBufSize,%s)\n",
    FName,
    (P != NULL) ? P->Name : "NULL");

  if ((SBuffer == NULL) || (P == NULL))
    {
    return(FAILURE);
    }

  BPtr   =  SBuffer;
  BSpace = *SBufSize;

  /* locate suspended jobs */

  sindex = 0;

  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    if (J->State == mjsSuspended)
      SuspQ[sindex++] = J->Index;
    }  /* END for (J) */

  SuspQ[sindex] = -1;

  /* provide general usage stats */

  TotalAllocatedProcs = 0;

  for (aindex = 0;MAQ[aindex] != -1;aindex++)
    {
    J = MJob[MAQ[aindex]];

    TotalAllocatedProcs += MJobGetProcCount(J);
    }  /* END for (aindex) */

  MUSNPrintF(&BPtr,&BSpace,"%ld %d %d %d %d %d %d\n",
    MSched.Time,
    P->URes.Procs,
    P->ARes.Procs,
    P->UpNodes,
    P->IdleNodes,
    P->ActiveNodes,
    TotalAllocatedProcs);

  /* list active jobs */

  aindex = 0;
  sindex = 0;

  while ((MAQ[aindex] != -1) || (SuspQ[sindex] != -1))
    {
    if (MAQ[aindex] != -1)
      {
      J = MJob[MAQ[aindex++]];

      if (J->State == mjsSuspended)
        continue;
      }
    else
      {
      J = MJob[SuspQ[sindex++]];
      }

    /* HvB */

    if ((UName[0] != '\0') && strcmp(J->Cred.U->Name,UName))
      continue;

    RQ = J->Req[0];  /* handle only first req of job */

    if ((P->Index > 0) && (RQ->PtIndex != P->Index))
      continue;

    if (J->MasterJobName != NULL)
      continue;

    DBG(3,fUI) DPrint("INFO:     %s:  adding active job[%03d] '%s' to buffer\n",
      FName,
      J->Index,
      J->Name);

    /* Format:  <JOBNAME> <UNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <STATE> <PRIORITY> */

    if ((MPar[0].UseCPUTime == TRUE) && (J->CPULimit > 0))
      Limit = J->CPULimit;
    else
      Limit = J->WCLimit;

    sprintf(Line,"%16s %8s %8ld %8ld %4d %6ld %1s %2d %3ld\n",
      J->Name,
      (J->Cred.U != NULL) ? J->Cred.U->Name : "-",
      J->StartTime,
      J->SubmitTime,
      MJobGetProcCount(J),
      Limit,
      ((J->Cred.Q != NULL) && (J->Cred.Q->Index != 0)) ? J->Cred.Q->Name : "-",
      (J->IState != mjsNONE) ? J->IState : J->State,
      J->StartPriority);

    MUStrNCat(&BPtr,&BSpace,Line);

    if (BSpace < 3000)
      {
      DBG(1,fUI) DPrint("ALERT:    active job buffer overflow in %s.  (job %d:  buffer size: %ld)\n",
        FName,
        aindex,
        *SBufSize);

      IsTruncated = TRUE;

      break;
      }

    DBG(5,fUI) DPrint("INFO:     line: '%s' (%d)\n",
      Line,
      (int)strlen(SBuffer));
    }  /* END while (MAQ[aindex] != -1) */

  MUStrNCat(&BPtr,&BSpace,"[ENDACTIVE]\n");

  /* list eligible jobs */

  for (jindex = 0;MUIQ[jindex] != -1;jindex++)
    {
    J = MJob[MUIQ[jindex]];
  
    if ((J == NULL) || (J->Name[0] == '\0') || (J->Name[0] == '\1'))
      continue;

    RQ = J->Req[0];  /* handle only first req of job */

    if ((RQ == NULL) || (J->Cred.U == NULL))
      {
      /* corrupt job record */

      continue;
      }

    if ((P->Index > 0) && (MUBMCheck(P->Index,J->PAL) == FAILURE))
      continue;

    if ((J->State == mjsRemoved) || (J->State == mjsSuspended))
      continue;

    if ((UName[0] != '\0') && strcmp(J->Cred.U->Name,UName))
      continue;

    if (J->Hold & (1 << mhBatch))
      tmpState = mjsBatchHold;
    else if (J->Hold & (1 << mhSystem))
      tmpState = mjsSystemHold;
    else if (J->Hold & (1 << mhUser))
      tmpState = mjsUserHold;
    else if (J->EState == mjsDeferred)
      tmpState = mjsDeferred;
    else
      tmpState = (J->IState != mjsNONE) ? J->IState : J->State;

    /* Format:  <JOBNAME> <UNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <STATE> <PRIORITY> */

    sprintf(Line,"%16s %8s %8ld %8ld %4d %6ld %1s %2d %3ld\n",
      J->Name,
      (J->Cred.U != NULL) ? J->Cred.U->Name : "-",
      J->StartTime,
      J->SubmitTime,
      J->Request.TC * RQ->DRes.Procs,
      J->WCLimit,
      ((J->Cred.Q != NULL) && (J->Cred.Q->Index != 0)) ? J->Cred.Q->Name : "-",
      tmpState,
      J->StartPriority);

    MUStrNCat(&BPtr,&BSpace,Line);

    if (BSpace < 3000)
      {
      DBG(1,fUI) DPrint("ERROR:    idle job buffer overflow in %s.  (job %d:  buffer size: %ld)\n",
        FName,
        jindex,
        *SBufSize);

      DBG(2,fUI) DPrint("ALERT:    buffer overflow in %s()\n",
        FName);

      IsTruncated = TRUE;

      break;
      }

    DBG(4,fUI) DPrint("INFO:     line: '%s' (%d)\n",
      Line,
      (int)strlen(SBuffer));
    }  /* END for (jindex) */

  MUStrNCat(&BPtr,&BSpace,"[ENDIDLE]\n");

  /* list blocked jobs */

  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    if ((P->Index > 0) && (MUBMCheck(P->Index,J->PAL) == FAILURE))
      continue;

    /* continue if job has not yet been released in simulation mode */

    if ((MSched.Mode == msmSim) && (J->SMinTime > MSched.Time))
      continue;

    /* continue if active job found */

    if ((J->State == mjsRunning) ||
        (J->State == mjsStarting) ||
        (J->State == mjsSuspended) ||
        (J->IState != mjsNONE))
      {
      continue;
      }

    if ((UName[0] != '\0') && strcmp(J->Cred.U->Name,UName))
      continue;

    IdleJob = FALSE;

    for (jindex = 0;MUIQ[jindex] != -1;jindex++)
      {
      if (MUIQ[jindex] == J->Index)
        {
        IdleJob = TRUE;

        break;
        }
      }    /* END for (jindex) */

    if (IdleJob == TRUE)
      continue;

    if (J->Hold & (1 << mhBatch))
      tmpState = mjsBatchHold;
    else if (J->Hold & (1 << mhSystem))
      tmpState = mjsSystemHold;
    else if (J->Hold & (1 << mhUser))
      tmpState = mjsUserHold;
    else if (J->EState == mjsDeferred)
      tmpState = mjsDeferred;
    else
      tmpState = J->State;

    /* Format:  <JOBNAME> <UNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <STATE> <PRIORITY> */

    sprintf(Line,"%16s %8s %8ld %8ld %4d %6ld %1s %2d %3ld\n",
      J->Name,
      J->Cred.U->Name,
      J->StartTime,
      J->SubmitTime,
      J->Request.TC * J->Req[0]->DRes.Procs,
      J->WCLimit,
      (J->Cred.Q->Index != 0) ? J->Cred.Q->Name : "-",
      tmpState,
      J->StartPriority);

    MUStrNCat(&BPtr,&BSpace,Line);

    DBG(4,fUI) DPrint("INFO:     line: '%s' (%d)\n",
      Line,
      (int)strlen(SBuffer));

    if (BSpace < 3000)
      {
      DBG(2,fUI) DPrint("ALERT:    buffer overflow in %s()\n",
        FName);

      IsTruncated = TRUE;

      break;
      }
    }    /* END for (J) */

  MUStrNCat(&BPtr,&BSpace,"[ENDBLOCKED]\n");

  if (IsTruncated == TRUE)
    {
    MUStrNCat(&BPtr,&BSpace,"NOTE:  output truncated\n\n");
    }

  DBG(2,fUI) DPrint("INFO:     %s buffer size: %d bytes\n",
    FName,
    (int)strlen(SBuffer));

  return(SUCCESS);
  }  /* END UIQueueShowAllJobs() */




int MUIShow(

  msocket_t *S,
  long       CFlags,
  char      *Auth)

  {
  char OString[MAX_MLINE];
  char FlagString[MAX_MLINE];
  char ArgString[MAX_MLINE];

  int  OIndex;

  mxml_t *E = NULL;
  mxml_t *RE = NULL;

  const char *FName = "MUIShow";

  DBG(2,fUI) DPrint("%s(S,%ld,%s)\n",
    FName,
    CFlags,
    (Auth != NULL) ? Auth : "NULL");

  if (S == NULL)
    return(FAILURE);

  MUISMsgClear(S);

  /* TEMP:  force to XML */

  S->WireProtocol = mwpXML;

  switch(S->WireProtocol)
    {
    case mwpXML:

      {
      char *ptr;

      /* FORMAT:  <Message><Request object="queue"/>... */
      /* FORMAT:  <Reply><Response>msg="output truncated\nRM unavailable\n"><queue type="active"><job name="X" state="X" ...></job></queue></Response></Reply> */

      if (((ptr = strchr(S->RPtr,'<')) == NULL) ||
           (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE))
        {
        DBG(3,fUI) DPrint("WARNING:  corrupt command '%100.100s' received\n",
          S->RBuffer);

        MUISMsgAdd(S,"ERROR:    corrupt command received\n");

        MXMLDestroyE(&E);

        return(FAILURE);
        }

      if ((MXMLGetChild(E,"Request",NULL,&RE) == FAILURE) ||
          (MXMLGetAttr(RE,"object",NULL,OString,sizeof(OString)) == FAILURE) ||
         ((OIndex = MUGetIndex(OString,MXO,FALSE,mxoNONE)) == mxoNONE))
        {
        DBG(3,fUI) DPrint("WARNING:  corrupt command '%100.100s' received\n",
          S->RBuffer);

        MUISMsgAdd(S,"ERROR:    corrupt command received\n");

        MXMLDestroyE(&E);

        return(FAILURE);
        }

      MXMLGetAttr(RE,"flag",NULL,FlagString,sizeof(FlagString));
      MXMLGetAttr(RE,"arg",NULL,ArgString,sizeof(ArgString));
      }  /* END BLOCK */

      break;

    default:

      MUISMsgAdd(S,"ERROR:    format not supported\n");

      return(FAILURE);

      break;
    }  /* END switch(S->WireProtocol) */

  /* create parent element */

  if (MXMLCreateE((mxml_t **)&S->SE,"Reply") == FAILURE)
    {
    MUISMsgAdd(S,"ERROR:    cannot create response\n");

    return(FAILURE);
    }

  /* process request */

  switch(OIndex)
    {
    case mxoQueue:

      MUIQueueShow(S,RE,(mxml_t **)&S->SE);

      break;

    default:

      MUISMsgAdd(S,"ERROR:    object not handled\n");

      return(FAILURE);

      break;
    }  /* END switch(OIndex) */

  return(SUCCESS);
  }  /* END MUIShow() */




int MUIJobDiagnose(

  char *Buffer,
  long *BufSize,
  int   PIndex,
  char *DiagOpt,
  int   Flags)

  {
  int jindex;
  int nindex;
  int rqindex;
  int index;

  int pcount;

  mnode_t *N;
  mjob_t  *J;

  int    JobIndex;
  int    NodeCount;

  int    TotalJobCount;
  int    ActiveJobCount;
  int    RunListCount;
  int    LinkJobCount;

  int    Truncated;

  char   QueuedTime[MAX_MNAME];
  char   WCLimit[MAX_MNAME];

  char   MemLine[MAX_MNAME];
  char   DiskLine[MAX_MNAME];
  char   ProcLine[MAX_MNAME];

  char   ClassLine[MAX_MNAME];

  mreq_t *RQ;

  const char *FName = "MUIJobDiagnose";

  DBG(2,fUI) DPrint("%s(Buffer,BufSize,%s,%s,%d)\n",
    FName,
    MAList[ePartition][PIndex],
    DiagOpt,
    Flags);

  /* show main job list */

  DBG(4,fUI) DPrint("INFO:     diagnosing job table (%d job slots)\n",
    MSched.M[mxoJob]);

  Truncated = FALSE;

  sprintf(temp_str,"Diagnosing Jobs\n");
  strcat(Buffer,temp_str);

  /* FORMAT:       NAME STATE PARTI PROCS QOS WCLI RESER MPR USR GRP ACC QTIM NET OPS ARC MEM DSK CLAS FEAT */

  sprintf(Buffer,"%-18s %8.8s %3.3s %4.4s %3s %11s %1.1s %4s %8s %8s %8s %11s %8s %6s %6s %6s %6s %6s %11s %s\n\n",
    "Name",
    "State",
    "Par",
    "Proc",
    "QOS",
    "WCLimit",
    "Reservation",
    "Min",
    "User",
    "Group",
    "Account",
    "QueuedTime",
    "Network",
    "Opsys",
    "Arch",
    "Mem",
    "Disk",
    "Procs",
    "Class",
    "Features");

  TotalJobCount = 0;
  ActiveJobCount = 0;

  for (jindex = 1;jindex < MSched.M[mxoJob];jindex++)
    {
    if ((MJob[jindex] == NULL) || (MJob[jindex] == (mjob_t *)1))
      continue;

    if (MJob[jindex]->Name[0] == '\0')
      continue;

    J = MJob[jindex];

    if (strcmp(DiagOpt,NONE) && strcmp(DiagOpt,J->Name))
      continue;

    RQ = J->Req[0]; /* FIXME */

    if (Truncated == FALSE)
      {
      if ((MPar[0].UseCPUTime == TRUE) && (J->CPULimit > 0))
        strcpy(WCLimit,MULToTString(J->CPULimit));
      else
        strcpy(WCLimit,MULToTString(J->SpecWCLimit[0]));

      strcpy(QueuedTime,MULToTString(MSched.Time - J->SystemQueueTime));

      sprintf(ProcLine,"%s%d",
        MComp[RQ->ProcCmp],
        RQ->RequiredProcs);

      sprintf(MemLine,"%s%d",
        MComp[RQ->MemCmp],
        RQ->RequiredMemory);

      sprintf(DiskLine,"%s%d",
        MComp[RQ->DiskCmp],
        RQ->RequiredDisk);

      MUCAListToString(RQ->DRes.PSlot,NULL,ClassLine);

      if (ClassLine[0] == '\0')
        {
        strcpy(ClassLine,NONE);
        }
      else
        {
        for (index = 0;ClassLine[index] != '\0';index++)
          {
          if ((ClassLine[index] == ' ') && isdigit(ClassLine[index + 1]))
            ClassLine[index] = ':';
          }
        }

      sprintf(temp_str,"%-18s %8.8s %3.3s %4d %3.3s %11s %1d %4d %8.8s %8.8s %8.8s %11s %8s %6s %6s %6s %6s %6s %11s ",
        J->Name,
        MJobState[J->State],
        MAList[ePartition][RQ->PtIndex],
        MJobGetProcCount(J),
        (J->Cred.Q != NULL) ? J->Cred.Q->Name : "-",
        WCLimit,
        J->R ? 1 : 0,
        MJobGetProcCount(J),
        J->Cred.U->Name,
        J->Cred.G->Name,
        (J->Cred.A != NULL) ? J->Cred.A->Name : "-",
        QueuedTime,
        MAList[eNetwork][RQ->Network],
        MAList[eOpsys][RQ->Opsys],
        MAList[eArch][RQ->Arch],
        MemLine,
        DiskLine,
        ProcLine,
        ClassLine);
      strcat(Buffer,temp_str);

      sprintf(temp_str,"%s\n",
        MUMAList(eFeature,RQ->ReqFBM,sizeof(RQ->ReqFBM)));
      strcat(Buffer,temp_str);
      }  /* END if (Truncated == FALSE) */

    TotalJobCount++;

    DBG(5,fUI) DPrint("INFO:     Job[%03d]: '%s'  TotalJobCount: %3d\n",
      jindex,
      MJob[jindex]->Name,
      TotalJobCount);

    /* verify resource utilization */

    if ((J->State == mjsRunning) || (J->State == mjsStarting))
      {
      if ((double)RQ->URes.Procs / 100.0 > (double)RQ->DRes.Procs)
        {
        if (Truncated == FALSE)
          {
          sprintf(temp_str,"WARNING:  job '%s' utilizes more procs than dedicated (%.2lf > %d)\n",
            J->Name,
            RQ->URes.Procs / 100.0,
            RQ->DRes.Procs);
          strcat(Buffer,temp_str);
          }
        }

      if ((RQ->URes.Mem > RQ->DRes.Mem) && (RQ->DRes.Mem > 0))
        {
        if (Truncated == FALSE)
          {
          sprintf(temp_str,"WARNING:  job '%s' utilizes more memory than dedicated (%d > %d)\n",
            J->Name,
            RQ->URes.Mem,
            RQ->DRes.Mem);
          strcat(Buffer,temp_str);
          }
        }

      if ((RQ->LURes.Procs > 0) &&
          (J->PSUtilized > 0.01) &&
         (((J->PSUtilized / RQ->LURes.Procs) > 1.1) || ((J->PSUtilized / RQ->LURes.Procs) < .9)))
        {
        if (Truncated == FALSE)
          {
/*
          sprintf(temp_str,"WARNING:  job '%s' unsynchronized proc usage stats (RM: %ld  Sample: %ld)\n",
            J->Name,
            (long)RQ->LURes.Procs,
            (long)J->PSUtilized);
          strcat(Buffer,temp_str);
 */
          }
        }

      if (((J->AWallTime + J->SWallTime) - (MSched.Time - J->StartTime)) > MSched.RMPollInterval << 1)
        {
        if (Truncated == FALSE)
          {
/*
          sprintf(temp_str,"WARNING:  job '%s' walltime tracking is corrupt (W:%ld + S:%ld) != (P:%ld - S:%ld)\n",
            J->Name,
            J->AWallTime,
            J->SWallTime,
            MSched.Time,
            J->StartTime);
          strcat(Buffer,temp_str);
 */
          }
        }
      }    /* END if ((J->State == mjsRunning) || (J->State == mjsStarting)) */

    /* verify job state consistency */

    if ((J->State != J->EState) &&
        (J->EState != mjsDeferred))
      {
      if (Truncated == FALSE)
        {
        if ((J->State != mjsRunning) || (J->EState != mjsStarting))
          {
          sprintf(temp_str,"WARNING:  job '%s' state '%s' does not match expected state '%s'\n",
            J->Name,
            MJobState[J->State],
            MJobState[J->EState]);
          strcat(Buffer,temp_str);
          }
        }
      }

    /* verify pmask */

    if (J->PAL[0] == 0)
      {
      if (Truncated == FALSE)
        {
        sprintf(temp_str,"WARNING:  job '%s' has invalid pmask\n",
          J->Name);
        strcat(Buffer,temp_str);
        }
      }

    /* if job is active... */

    if ((J->State == mjsRunning) || (J->State == mjsStarting))
      {
      ActiveJobCount++;

      /* check reservation */

      if (J->R == NULL)
        {
        if (Truncated == FALSE)
          {
          sprintf(temp_str,"WARNING:  active job '%s' has no reservation\n",
            J->Name);
          strcat(Buffer,temp_str);
          }
        }

      /* search for active job in MAQ table */

      for (JobIndex = 0;MAQ[JobIndex] != -1;JobIndex++)
        {
        if (MAQ[JobIndex] == jindex)
          break;
        }

      if (MAQ[JobIndex] == -1)
        {
        if (Truncated == FALSE)
          {
          sprintf(temp_str,"WARNING:  active job '%s' is not in MAQ table\n",
            J->Name);
          strcat(Buffer,temp_str);
          }
        }

      /* verify WallClock limit */

      if ((MSched.Time - J->StartTime) > J->WCLimit)
        {
        if (Truncated == FALSE)
          {
          strcpy(WCLimit,MULToTString(J->WCLimit));

          sprintf(temp_str,"WARNING:  job '%s' has exceeded wallclock limit (%s > %s) \n",
            J->Name,
            MULToTString(MSched.Time - J->StartTime),
            WCLimit);
          strcat(Buffer,temp_str);
          }
        }    /* END if MSched.Time */
      }      /* END if J->State    */
    else
      {
      /* check start count on idle jobs */

      if (J->StartCount >= 4)
        {
        if (Truncated == FALSE)
          {
          sprintf(temp_str,"WARNING:  job '%s' has failed to start %d times\n",
            J->Name,
            J->StartCount);
          strcat(Buffer,temp_str);
          }
        }    /* END if J->StartCount */
      }      /* END else if J->State */

    if ((strlen(Buffer) > (*BufSize - 3000)) && (Truncated != TRUE))
      {
      DBG(2,fUI) DPrint("ALERT:    buffer overflow in %s()\n",
        FName);

      Truncated = TRUE;

      strcat(Buffer,"NOTE:  output truncated\n\n");
      }
    }   /* END for jindex */

  if (!strcmp(DiagOpt,NONE))
    {
    /* diagnose linked list consistency */

    DBG(4,fUI) DPrint("INFO:     diagnosing job table link consistency\n");

    LinkJobCount = 0;

    for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
      {
      LinkJobCount++;

      DBG(5,fUI) DPrint("INFO:     Job[%03d]: '%s'  link %3d (next: %p)\n",
        jindex,
        J->Name,
        LinkJobCount,
        J->Next);
      }  /* END for (jindex) */

    /* diagnose MAQ */

    DBG(4,fUI) DPrint("INFO:     diagnosing MAQ table\n");

    if ((Truncated == FALSE) && (Flags & (1 << mcmVerbose)))
      {
      sprintf(temp_str,"\n\nMAQ table (%d job slots)\n",
        MSched.M[mxoJob]);
      strcat(Buffer,temp_str);
      }

    RunListCount = 0;
    Truncated = FALSE;

    for (jindex = 0;MAQ[jindex] != -1;jindex++)
      {
      J = MJob[MAQ[jindex]];

      if ((strlen(Buffer) > (*BufSize - 300)) && (Truncated == FALSE))
        {
        DBG(2,fUI) DPrint("ALERT:    buffer overflow in %s()\n",
          FName);

        Truncated = TRUE;

        strcat(Buffer,"NOTE:  Output Truncated\n\n");
        }

      if ((Truncated == FALSE) && (Flags & (1 << mcmVerbose)))
        {
        sprintf(temp_str,"MAQ[%02d] --> Job[%03d] '%s'\n",
          jindex,
          MAQ[jindex],
          J->Name);
        strcat(Buffer,temp_str);
        }

      /* verify node consistency */

      DBG(4,fUI) DPrint("INFO:     diagnosing node consistency of running Job[%03d] --> [%02d] '%s'\n",
        jindex,
        MAQ[jindex],
        J->Name);

      pcount    = 0;
      NodeCount = 0;

      for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
        {
        RQ = J->Req[rqindex];

        for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
          {
          N = RQ->NodeList[nindex].N;

          if ((RQ->NAccessPolicy == mnacSingleJob) ||
              (RQ->NAccessPolicy == mnacSingleTask) ||
              (RQ->DRes.Procs == -1))
            {
            pcount += N->CRes.Procs;
            }
          else
            {
            pcount += (RQ->NodeList[nindex].TC * RQ->DRes.Procs);
            }

          if (MUBMCheck(N->PtIndex,J->PAL) == FAILURE)
            {
            if (Truncated == FALSE)
              {
              sprintf(temp_str,"WARNING:  job '%s' with partition mask %s has node %s allocated from partition %s\n",
                J->Name,
                (J->PAL[0] == 0) ?
                  ALL : MUListAttrs(ePartition,J->PAL[0]),
                N->Name,
                MAList[ePartition][N->PtIndex]);
              strcat(Buffer,temp_str);
              }
            }

          if ((N->State != mnsBusy) &&
              (N->State != mnsActive) &&
              (N->State != mnsDraining) &&
              (N != MSched.GN) &&
             ((MSched.Time - J->StartTime) > 300))
            {
            if (Truncated == FALSE)
              {
              sprintf(temp_str,"WARNING:  active job '%s' has inactive node %s allocated for %s (node state: '%s')\n",
                J->Name,
                N->Name,
                MULToTString(MSched.Time - J->StartTime),
                MAList[eNodeState][N->State]);
              strcat(Buffer,temp_str);
              }
            }    /* END if N->State != mnsBusy) */
          }      /* END for (nindex)            */

        NodeCount += nindex;
        }        /* END for (rqindex)           */

      if (NodeCount != J->NodeCount)
        {
        if (Truncated == FALSE)
          {
          sprintf(temp_str,"WARNING:  active job '%s' has inconsistent node allocation  (nodes: %d  nodelist size: %d)\n",
            J->Name,
            J->NodeCount,
            NodeCount);
          strcat(Buffer,temp_str);
          }
        }

      if (pcount != MJobGetProcCount(J))
        {
        if (Truncated == FALSE)
          {
          sprintf(temp_str,"WARNING:  active job %s has inconsistent proc allocation  (procs: %d  nodelist procs: %d)\n",
            J->Name,
            MJobGetProcCount(J),
            pcount);
          strcat(Buffer,temp_str);
          }
        }

      RunListCount++;
      }  /* END for (jindex) */

    sprintf(temp_str,"\n\nTotal Jobs: %d  Active Jobs: %d\n",
      TotalJobCount,
      ActiveJobCount);
    strcat(Buffer,temp_str);

    if (TotalJobCount != LinkJobCount)
      {
      sprintf(temp_str,"WARNING:  job table is corrupt (total jobs (%d) != linked jobs (%d))\n",
        TotalJobCount,
        LinkJobCount);
      strcat(Buffer,temp_str);
      }

    if (ActiveJobCount != RunListCount)
      {
      sprintf(temp_str,"WARNING:  active job table is corrupt (active jobs (%d) != active queue size (%d))\n",
        ActiveJobCount,
        RunListCount);
      strcat(Buffer,temp_str);
      }

    /* verify reservations ??? */
    }  /* END if (!strcmp(DiagOpt,NONE)) */

  return(SUCCESS);
  }  /* END MUIJobDiagnose() */




int UIParShowStats(

  char *PName,    /* I */
  char *Buffer,   /* O */
  long *BufSize)  /* I */

  {
  const char *FName = "UIParShowStats";

  DBG(3,fUI) DPrint("%s(%s,Buffer,BufSize)\n",
    FName,
    PName);

  sprintf(Buffer,"ERROR:  partition stats not available\n");

  return(SUCCESS);
  }  /* END UIParShowStats() */




int UINodeShow(

  char *RBuffer,  /* I */
  char *SBuffer,  /* I/O:  output buffer */
  int   FLAGS,    /* I:    command flags */
  char *Auth,     /* I */
  long *SBufSize) /* I */

  {
  char    NodeName[MAX_MNAME << 1];

  mnode_t *N;

  long    Flags;

  const char *FName = "UINodeShow";

  DBG(2,fUI) DPrint("%s(RBuffer,SBuffer,%d,%s,SBufSize)\n",
    FName,
    FLAGS,
    Auth);

  /* FORMAT:  <NODENAME> */

  MUSScanF(RBuffer,"%x%s %ld",
    sizeof(NodeName),
    NodeName,
    &Flags);

  if (!strcmp(NodeName,"DEFAULT"))
    {
    N = &MSched.DefaultN;
    }
  else if (MNodeFind(NodeName,&N) != SUCCESS)
    {
    /* append domain and try again */

    strcat(NodeName,MSched.DefaultDomain);

    if (MNodeFind(NodeName,&N) != SUCCESS)
      {
      sprintf(SBuffer,"ERROR:  cannot locate node '%s'\n",
        NodeName);

      DBG(3,fUI) DPrint("INFO:     cannot locate node '%s' in %s()\n",
        NodeName,
        FName);

      return(FAILURE);
      }
    }    /* END else if (MNodeFind(NodeName,&N) != SUCCESS) */

  SBuffer[0] = '\0';

  MNodeShowState(N,Flags,SBuffer,*SBufSize,mSet);

  if (N != &MSched.DefaultN)
    {
    MNodeShowReservations(N,Flags,SBuffer,*SBufSize,mAdd);

    MNodeDiagnoseState(N,Flags,SBuffer,*SBufSize,mAdd);

    MNodeDiagnoseReservations(N,Flags,SBuffer,*SBufSize,mAdd);
    }  /* END if (N != &MSched.DefaultN) */

  return(SUCCESS);
  }  /* END UINodeShow() */




int UIJobCancel(

  char *RBuffer,   /* I */
  char *SBuffer,   /* O */
  int   FLAGS,
  char *Auth,
  long *SBufSize)

  {
  char  *JobName;
  char  *ptr;
  char  *TokPtr;

  mjob_t *J;

  char   *BPtr;
  int     BSpace;

  const char *FName = "UIJobCancel";

  DBG(2,fUI) DPrint("%s(RBuffer,SBuffer,%d,%s,SBufSize)\n",
    FName,
    FLAGS,
    Auth);

  /* FORMAT:  <JID> [<JID>] ... */

  SBuffer[0] = '\0';

  BPtr   =  SBuffer;
  BSpace = *SBufSize;
  
  if ((ptr = MUStrTok(RBuffer," \t\n",&TokPtr)) == NULL)
    {
    MUSNPrintF(&BPtr,&BSpace,"ERROR:  no jobs specified\n");

    return(SUCCESS);
    }

  /* step through all jobs */

  while (ptr != NULL)
    {
    JobName = ptr;

    ptr = MUStrTok(NULL," \t\n",&TokPtr);

    if (MJobFind(JobName,&J,0) != SUCCESS)
      {
      MUSNPrintF(&BPtr,&BSpace,"ERROR:  cannot locate job '%s'\n",
        JobName);

      continue;
      }

    /* security check */

    if (!(FLAGS & ((1 << fAdmin1) | (1 << fAdmin2))))
      {
      if (strcmp(J->Cred.U->Name,Auth) != 0)
        {
        DBG(2,fUI) DPrint("INFO:     user %s is not authorized to cancel job '%s'\n",
          Auth,
          J->Name);

        MUSNPrintF(&BPtr,&BSpace,"user %s is not authorized to cancel job '%s'\n",
          Auth,
          J->Name);

        continue;
        }
      }

    /* cancel job */

    if (MSched.Mode == msmSim)
      {
      MJobSetState(J,mjsRemoved);

      DBG(3,fUI) DPrint("INFO:     job '%s' cancelled by user %s\n",
        J->Name,
        Auth);

      MUSNPrintF(&BPtr,&BSpace,"job '%s' cancelled\n",
        J->Name);
      }
    else
      {
      /* non-simulation mode */

      if (MRMJobCancel(J,"MAUI_INFO:  job cancelled by user\n",NULL) == SUCCESS)
        {
        DBG(3,fUI) DPrint("INFO:     job '%s' cancelled by user %s\n",
          J->Name,
          Auth);

        MUSNPrintF(&BPtr,&BSpace,"job '%s' cancelled\n",
          J->Name);
        }
      else
        {
        DBG(3,fUI) DPrint("ALERT:    cannot cancel job '%s'\n",
          J->Name);

        MUSNPrintF(&BPtr,&BSpace,"ERROR:  cannot cancel job '%s'\n",
          J->Name);
        }
      }
    }      /* END  while (ptr != NULL) */

  return(SUCCESS);
  }  /* END UIJobCancel() */




int UINodeDiagnose(

  char *SBuffer,  /* O */
  long *SBufSize, /* O */
  int   PIndex,   /* I */
  char *DiagOpt,  /* I */
  int   Flags)    /* I */

  {
  int jindex;
  int rindex;
  int nindex;
  int index;

  mnode_t *N;
  mnode_t *GN;

  mjob_t  *J;

  char     Line[MAX_MLINE];
  char     JobName[MAX_MNAME];

  int JobIndex;
  int NodeIndex;
  int ResIndex;

  int ActiveNodeCount;
  int IdleNodeCount;
  int TotalNodeCount;
  int DownNodeCount;

  int NodeReservationCount;

  int SameStateTime;

  char ClassLine[MAX_MLINE];

  mcres_t TotalCRes;
  mcres_t TotalARes;

  void *D;
  int   (*F)(void *, int, void *, void **);

  char  tmpBuffer[MAX_MBUFFER];

  char *BPtr;
  int   BSpace;

  const char *FName = "UINodeDiagnose";

  DBG(3,fUI) DPrint("%s(SBuffer,BufSize,%s,%s,%d)\n",
    FName,
    MAList[ePartition][PIndex],
    DiagOpt,
    Flags);

  if ((SBuffer == NULL) || (SBufSize == NULL))
    {
    return(FAILURE);
    }

  /* show node list */

  DBG(4,fUI) DPrint("INFO:     diagnosing node table (%d slots)\n",
    MAX_MNODE);

  memset(&TotalCRes,0,sizeof(TotalCRes));
  memset(&TotalARes,0,sizeof(TotalARes));

  SBuffer[0] = '\0';

  BPtr   =  SBuffer;
  BSpace = *SBufSize;

  MUSNPrintF(&BPtr,&BSpace,"diagnosing node table (%d slots)\n",
    MAX_MNODE);

  TotalNodeCount  = 0;
  IdleNodeCount   = 0;
  ActiveNodeCount = 0;
  DownNodeCount   = 0;

  /* create header */

  MUSNPrintF(&BPtr,&BSpace,"%-20s %8s %7s %13s %13s %13s %5s %6s %6s %3.3s %6s %3.3s %-30s %-30s %-22s\n\n",
    "Name",
    "State",
    " Procs ",
    "   Memory    ",
    "    Disk     ",
    "    Swap     ",
    "Speed",
    "Opsys",
    "Arch",
    "Partition",
    "Load",
    "Res",
    "Classes",
    "Network",
    "Features");

  if (MNodeFind(MDEF_GNNAME,&GN) == FAILURE)
    {
    GN = NULL;
    }

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    if (N == GN)
      continue;

    if (strcmp(DiagOpt,NONE) && strcmp(DiagOpt,N->Name))
      continue;

    if ((PIndex != 0) && (PIndex != N->PtIndex))
      continue;

    if ((N->FrameIndex > 0) && (MSys[N->FrameIndex][N->SlotIndex].Attributes & (1 << attrSystem)))
      continue;

    DBG(6,fUI) DPrint("INFO:     diagnosing node %s \n",
      N->Name);

    /* determine reservation count */

    NodeReservationCount = 0;

    for (rindex = 0;rindex < MSched.ResDepth;rindex++)
      {
      if (N->R[rindex] == (mres_t *)1)
        continue;

      if (N->R[rindex] == NULL)
        break;

      NodeReservationCount++;
      }

    /* display node attributes */

    MUSNPrintF(&BPtr,&BSpace,
      (Flags & (1 << mcmVerbose)) ?
        "%-20s %8s %3d:%-3d %6d:%-6d %6d:%-6d %6d:%-6d %5.2f %6s %6s %3s %6.2f %3d " :
        "%-20.20s %8.8s %3d:%-3d %6d:%-6d %6d:%-6d %6d:%-6d %5.2f %6.6s %6.6s %3.3s %6.2f %3.3d ",
      N->Name,
      MAList[eNodeState][N->State],
      MIN(N->ARes.Procs,N->CRes.Procs - N->DRes.Procs),
      N->CRes.Procs,
      MIN(N->ARes.Mem,N->CRes.Mem - N->DRes.Mem),
      N->CRes.Mem,
      MIN(N->ARes.Disk,N->CRes.Disk - N->DRes.Disk),
      N->CRes.Disk,
      MIN(N->ARes.Swap,N->CRes.Swap - N->DRes.Swap),
      N->CRes.Swap,
      N->Speed,
      MAList[eOpsys][N->ActiveOS],
      MAList[eArch][N->Arch],
      MAList[ePartition][N->PtIndex],
      N->Load,
      NodeReservationCount);

    TotalCRes.Procs += N->CRes.Procs;
    TotalARes.Procs += MIN(N->ARes.Procs,N->CRes.Procs - N->DRes.Procs);

    TotalCRes.Mem   += N->CRes.Mem;
    TotalARes.Mem   += MIN(N->ARes.Mem,N->CRes.Mem     - N->DRes.Mem);

    TotalCRes.Disk  += N->CRes.Disk;
    TotalARes.Disk  += MIN(N->ARes.Disk,N->CRes.Disk   - N->DRes.Disk);

    TotalCRes.Swap  += N->CRes.Swap;
    TotalARes.Swap  += MIN(N->ARes.Swap,N->CRes.Swap   - N->DRes.Swap);

    MUCAListToString(N->ARes.PSlot,N->CRes.PSlot,ClassLine);

    sprintf(Line,
      (Flags & (1 << mcmVerbose)) ?
        "%-30s" :
        "%-30.30s",
        (ClassLine[0] != '\0') ? ClassLine : NONE);

    for (index = 0;Line[index] != '\0';index++)
      {
      if ((Line[index] == ' ') && isdigit(Line[index + 1]))
        Line[index] = '_';
      }  /* END for (index) */

    if (Flags & (1 << mcmVerbose))
      {
      MUSNPrintF(&BPtr,&BSpace,"%s %-30s %-20s\n",
        Line,
        MUListAttrs(eNetwork,N->Network),
        MUMAList(eFeature,N->FBM,sizeof(N->FBM)));
      }
    else
      {
      MUSNPrintF(&BPtr,&BSpace,"%s %-30.30s %-20.20s\n",
        Line,
        MUListAttrs(eNetwork,N->Network),
        MUMAList(eFeature,N->FBM,sizeof(N->FBM)));
      }

    /* display diagnostic messages */

    if (N->State == mnsUnknown)
      {
      continue;
      }
    else if ((N->State == mnsActive) ||
             (N->State == mnsIdle) ||
             (N->State == mnsBusy))
      {
      /* check resource utilization */

      if (N->ARes.Procs < (N->CRes.Procs - N->DRes.Procs))
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' has more processors utilized than dedicated (%d > %d)\n",
          N->Name,
          N->CRes.Procs - N->ARes.Procs,
          N->DRes.Procs);
        }

      if ((N->ARes.Mem < (N->CRes.Mem - N->DRes.Mem)) &&
          (N->DRes.Mem > 0))
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' has more memory utilized than dedicated (%d > %d)\n",
          N->Name,
          N->CRes.Mem - N->ARes.Mem,
          N->DRes.Mem);
        }

      if ((N->ARes.Swap < (N->CRes.Swap - N->DRes.Swap - MIN_OS_SWAP_OVERHEAD)) &&
          (N->DRes.Swap > 0))
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' has more swap utilized than dedicated (%d > %d)\n",
          N->Name,
          N->CRes.Swap - N->ARes.Swap,
          N->DRes.Swap);
        }

      if ((N->ARes.Disk < (N->CRes.Disk - N->DRes.Disk)) && (N->DRes.Disk > 0))
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' has more disk utilized than dedicated (%d > %d)\n",
          N->Name,
          N->CRes.Disk - N->ARes.Disk,
          N->DRes.Disk);
        }
      }    /* END else if ((N->State == mnsActive) || (N->State == mnsIdle) || (N->State == mnsBusy)) */

    /* check update time */

    if ((MSched.Time > N->ATime) && ((MSched.Time - N->ATime) > 600))
      {
      MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' has not been updated in %s\n",
        N->Name,
        MULToTString(MSched.Time - N->ATime));
      }

    /* check node memory */

    if ((N->FrameIndex != -1) &&
        (MFrame[N->FrameIndex].Memory > 0) &&
        (N->CRes.Mem != MFrame[N->FrameIndex].Memory))
      {
      MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' memory (%d MB) does not match frame %d memory (%d MB)\n",
        N->Name,
        N->CRes.Mem,
        N->FrameIndex,
        MFrame[N->FrameIndex].Memory);
      }

    /* check node swap space */

    if ((N->CRes.Swap > 0) && (N->ARes.Swap < MIN_SWAP))
      {
      MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' swap space (%d MB) is low\n",
        N->Name,
        N->ARes.Swap);
      }

    /* check state */

    SameStateTime = MSched.Time - N->StateMTime;

    if ((N->State != N->EState) && (SameStateTime > 120))
      {
      if ((N->State == mnsDrained) && (N->State == mnsDraining))
        {
        /* NO-OP */
        }
      else
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' state '%s' does not match expected state '%s'.  sync deadline in %s at %s",
          N->Name,
          MAList[eNodeState][N->State],
          MAList[eNodeState][N->EState],
          MULToTString(N->SyncDeadLine - MSched.Time),
          MULToDString((mulong *)&N->SyncDeadLine));
        }
      }

    JobIndex = -1;

    if (N->State == mnsBusy)
      {
      for (jindex = 0;MAQ[jindex] != -1;jindex++)
        {
        J = MJob[MAQ[jindex]];

        for (NodeIndex = 0;J->NodeList[NodeIndex].N != NULL;NodeIndex++)
          {
          if (J->NodeList[NodeIndex].N == N)
            {
            JobIndex = MAQ[jindex];

            break;
            }
          }

        if (JobIndex != -1)
          break;
        }  /* END for (jindex) */

      if (JobIndex == -1)
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' is busy but not assigned to an active job\n",
          N->Name);

        strcpy(JobName,"[UNKNOWN JOB]");
        }
      else
        {
        strcpy(JobName,MJob[JobIndex]->Name);
        }
      }    /* END if (N->State == mnsBusy) */

    if (N->State == mnsIdle)
      {
      if (N->ARes.Procs != N->CRes.Procs)
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  processor mismatch on idle node %s (%d available  %d configured)\n",
          N->Name,
          N->ARes.Procs,
          N->CRes.Procs);
        }
      }

    /* check disk space */

    if ((N->CRes.Disk > 0) && (N->ARes.Disk <= 0) && (N->State != mnsBusy))
      {
      MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' disk space is full (%d of %d MB available)\n",
        N->Name,
        N->ARes.Disk,
        N->CRes.Disk);
      }

    /* check network adapters */

    if (strcmp(MAList[eArch][N->Arch],"RS6000") == 0)
      {
      if (!(N->Network & MUMAGetBM(eNetwork,"ethernet",mVerify)) && (N->State != mnsDown))
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' is missing ethernet adapter (check JM?)\n",
          N->Name);
        }

      if (!(N->Network & MUMAGetBM(eNetwork,"hps_ip",mVerify)) && (N->State != mnsDown))
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' is missing IP switch adapter (check switch/JM?)\n",
          N->Name);
        }

      if ((N->State != mnsBusy) &&
          (N->State != mnsDraining) &&
          (N->State != mnsDown) &&
          (N->State != mnsReserved) &&
          (SameStateTime > 300) &&
         !(N->Network & MUMAGetBM(eNetwork,"hps_user",mVerify)) &&
         !(N->Network & MUMAGetBM(eNetwork,"hps_us",mVerify)))
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' is not busy and is missing user switch adapter (check switch/runaway processes?)\n",
          N->Name);
        }
      } /* END if (strcmp(MAList[eArch][N->Arch],"RS6000") == 0) */

    /* check reservations */

    for (ResIndex = 0;ResIndex < MSched.ResDepth;ResIndex++)
      {
      if (N->R[ResIndex] == (mres_t *)1)
        continue;

      if (N->R[ResIndex] == NULL)
        break;

      if (N->R[ResIndex]->Name[0] == '\0')
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' has empty reservation pointer  (clearing pointer)\n",
          N->Name);

        N->R[ResIndex] = (mres_t *)1;
        }
      }    /* END for (ResIndex) */

    /* check CPU usage */

    if ((N->State == mnsIdle) && (SameStateTime > 600) && (N->Load > 0.50) )
      {
      MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' has been idle for %s but load is HIGH.  load: %6.3f (check for runaway processes?)\n",
        N->Name,
        MULToTString(SameStateTime),
        N->Load);
      }

    if (N->Load > (N->CRes.Procs * 2))
      {
      MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' has excessive load (state: '%s'  load: %6.3lf)\n",
        N->Name,
        MNodeState[N->State],
        N->Load);
      }

    if (((N->State == mnsBusy) || (N->State == mnsActive)) &&
        (SameStateTime > 900) && ((N->Load / N->CRes.Procs) < 0.20))
      {
      MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' is active for %s but load is LOW.  load: %6.3f (check job %s?)\n",
        N->Name,
        MULToTString(SameStateTime),
        N->Load,
        JobName);
      }

    TotalNodeCount++;

    switch (N->State)
      {
      case mnsIdle:

        IdleNodeCount++;

        break;

      case mnsBusy:
      case mnsActive:
      case mnsDraining:

        ActiveNodeCount++;

        break;

      case mnsReserved:

        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' is reserved by an interactive POE job\n",
          N->Name);

        DownNodeCount++;

        break;

      case mnsDown:
      case mnsDrained:
      case mnsFlush:
      case mnsNONE:

        DownNodeCount++;

        break;

      default:

        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' has unexpected node state (%d)\n",
          N->Name,
          N->State);

        DownNodeCount++;

        break;
      }  /* END switch (N->State) */

    if (BSpace < 1000)
      {
      DBG(2,fUI) DPrint("ALERT:    buffer overflow in %s()\n",
        FName);

      MUStrNCat(&BPtr,&BSpace,"NOTE:     node list truncated\n");

      break;
      }
    }  /* END for (nindex = 0;nindex < MAX_MNODE;nindex++)  */

  /* display cluster summary */

  MUSNPrintF(&BPtr,&BSpace,
    (Flags & (1 << mcmVerbose)) ?
      "%-20s %8s %3d:%-3d %6d:%-6d %6d:%-6d %6d:%-6d\n" :
      "%-20.20s %8.8s %3d:%-3d %6d:%-6d %6d:%-6d %6d:%-6d\n",
    "-----",
    "---",
    TotalARes.Procs,
    TotalCRes.Procs,
    TotalARes.Mem,
    TotalCRes.Mem,
    TotalARes.Disk,
    TotalCRes.Disk,
    TotalARes.Swap,
    TotalCRes.Swap);

  MUSNPrintF(&BPtr,&BSpace,"\nTotal Nodes: %d  (Active: %d  Idle: %d  Down: %d)\n",
    TotalNodeCount,
    ActiveNodeCount,
    IdleNodeCount,
    DownNodeCount);

  /* display non-compute node resources */

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    D = MPar[0].XRes[nindex].Data;

    if (D == NULL)
      break;

    if (D == (void *)1)
      continue;

    F = MPar[0].XRes[nindex].Func;

    if (F == NULL)
      continue;

    if (strcmp(DiagOpt,NONE) && strcmp(DiagOpt,MPar[0].XRes[nindex].Name))
      continue;

    (*F)(&D,mascShow,NULL,(void **)tmpBuffer);

    if (nindex == 0)
      MUStrNCat(&BPtr,&BSpace,"\n\n");

    MUStrNCat(&BPtr,&BSpace,tmpBuffer);
    }  /* END for (nindex) */

  if (GN != NULL)
    {
    /* non default global resources set */

    MUStrNCat(&BPtr,&BSpace,"\n\n");

    MUSNPrintF(&BPtr,&BSpace,"NODE[%s] Config Res  %s\n",
      MDEF_GNNAME,
      MUCResToString(&GN->CRes,0,0,NULL));

    MUSNPrintF(&BPtr,&BSpace,"NODE[%s] Avail Res   %s\n",
      MDEF_GNNAME,
      MUCResToString(&GN->ARes,0,0,NULL));

    MUStrNCat(&BPtr,&BSpace,"\n\n");
    }

  return(SUCCESS);
  }  /* END UINodeDiagnose() */




int UIResShow(

  char *RBuffer,  /* I */
  char *Buffer,   /* O */
  int   FLAGS,    /* I */
  char *Auth,     /* I */
  long *BufSize)  /* I */

  {
  int  ObjectType;
  char Name[MAX_MNAME];

  int  rc;

  char PName[MAX_MNAME];

  mpar_t *P;

  int  Flags;

  const char *FName = "UIResShow";

  DBG(3,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);

  MUSScanF(RBuffer,"%d %x%s %d %x%s",
    &ObjectType,
    sizeof(PName),
    PName,
    &Flags,
    sizeof(Name),
    Name);

  if (MParFind(PName,&P) == FAILURE)
    {
    sprintf(Buffer,"ERROR:  invalid partition, %s, specified\n",
      PName);

    return(FAILURE);
    }

  switch(ObjectType)
    {
    case mxoNode:

      rc = MNodeShowRes(
        NULL,
        Name,
        P,
        Flags,
        mwpXML,
        Buffer,
        *BufSize);

      break;

    case mxoJob:

      rc = UIResList(Auth,FLAGS,Name,P->Index,Flags,Buffer,BufSize);

      break;

    default:

      DBG(0,fUI) DPrint("ERROR:    reservation type '%d' not handled\n",
        ObjectType);

      sprintf(Buffer,"ERROR:    reservation type '%d' not handled\n",
        ObjectType);

      rc = FAILURE;

      break;
    }  /* END switch(ObjectType) */

  return(rc);
  }  /* END UIResShow() */




int UIResList(

  char *Auth,     /* I */
  int   GFlags,
  char *RID,
  int   PIndex,
  int   Flags,
  char *SBuffer,   /* O */
  long *SBufSize)  /* I */

  {
  int  rindex;
  int  aindex;

  int  count;

  mjob_t *J;
  mres_t *R;

  char StartTime[MAX_MNAME];
  char EndTime[MAX_MNAME];
  char Duration[MAX_MNAME];
  char SMinTime[MAX_MNAME];

  time_t tmpTime;

  int Access;

  char *BPtr;
  int   BSpace;

  const char *FName = "UIResList";

  DBG(2,fUI) DPrint("%s(Auth,GFlags,%s,%d,%d,SBuffer,SBufSize)\n",
    FName,
    RID,
    PIndex,
    Flags);

  count = 0;

  BPtr   = SBuffer;
  BSpace = *SBufSize;

  BPtr[0] = '\0';

  MUSNPrintF(&BPtr,&BSpace,"Reservations\n\n");

  MUSNPrintF(&BPtr,&BSpace,"%-18s %5s %1s %11s %11s %11s %9s %s\n\n",
    "ReservationID",
    "Type",
    "S",
    "Start",
    "End",
    "Duration",
    "   N/P   ",
    "StartTime");

  /* add job reservations */

  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    if (J->Name[0] == '\0')
      continue;

    if ((PIndex > 0) && (MUBMCheck(PIndex,J->PAL) == FAILURE))
      continue;

    if ((RID != NULL) && strcmp(RID,NONE) && strcmp(RID,J->Name))
      continue;

    R = J->R;

    if (R == NULL)
      {
      DBG(4,fUI) DPrint("INFO:     job '%s' has no reservation\n",
        J->Name);

      continue;
      }

    strcpy(StartTime,MULToTString(R->StartTime - MSched.Time));
    strcpy(EndTime,MULToTString(R->EndTime - MSched.Time));
    strcpy(Duration,MULToTString(R->EndTime - R->StartTime));

    if (Flags & (1 << mcmVerbose))
      {
      tmpTime = (time_t)R->StartTime;

      strcpy(SMinTime,ctime(&tmpTime));
      }
    else
      {
      strcpy(SMinTime,MULToDString((mulong *)&R->StartTime));
      }

    MUSNPrintF(&BPtr,&BSpace,"%-18s %5s %1c %11s %11s %11s %4d/%-4d %s",
      J->Name,
      MResType[R->Type],
      MJobState[J->State][0],
      StartTime,
      EndTime,
      Duration,
      R->NodeCount,
      R->AllocPC,
      SMinTime);

    count++;

    DBG(5,fUI) DPrint("INFO:     job '%s' added to reservation list\n",
      J->Name);
    }    /* END for (jindex) */

  /* add non-job reservations */

  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    R = MRes[rindex];

    if ((R == NULL) || (R->Name[0] == '\0'))
      break;

    if (R->Name[0] == '\1')
      continue;

    if ((PIndex > 0) && (R->PtIndex != PIndex))
      continue;

    if (R->Type == mrtJob)
      continue;

    if ((RID != NULL) && strcmp(RID,NONE) && strcmp(RID,R->Name))
      continue;

    /* security check */

    Access = FALSE;

    if ((GFlags & ((1 << fAdmin1) | (1 << fAdmin2))))
      {
      Access = TRUE;
      }
    else if ((R->U != NULL) && !strcmp(R->U->Name,Auth))
      {
      Access = TRUE;
      }
    else if (R->Type == mrtUser)
      {
      for (aindex = 0;R->ACL[aindex].Name[0] != '\0';aindex++)
        {
        if ((R->ACL[aindex].Type == maUser) && !strcmp(R->ACL[aindex].Name,Auth))
          {
          Access = TRUE;

          break;
          }
        }  /* END for (aindex) */
      }

    if (Access == FALSE)
      {
      continue;
      }

    /* display non-job reservation */

    strcpy(StartTime,MULToTString(R->StartTime - MSched.Time));
    strcpy(EndTime,MULToTString(R->EndTime - MSched.Time));
    strcpy(Duration,MULToTString(R->EndTime - R->StartTime));

    if (Flags & (1 << mcmVerbose))
      {
      tmpTime = (time_t)R->StartTime;

      strcpy(SMinTime,ctime(&tmpTime));
      }
    else
      {
      strcpy(SMinTime,MULToDString((mulong *)&R->StartTime));
      }

    MUSNPrintF(&BPtr,&BSpace,"%-18s %5s %1c %11s %11s %11s %4d/%-4d %s",
      R->Name,
      MResType[R->Type],
      '-',
      StartTime,
      EndTime,
      Duration,
      R->NodeCount,
      R->AllocPC,
      SMinTime);

    count++;

    DBG(5,fUI) DPrint("INFO:     reservation '%s' added to reservation list\n",
      R->Name);
    }  /* END for (rindex = 0;rindex < MAX_MRES;rindex++) */

  MUSNPrintF(&BPtr,&BSpace,"\n%d %s located\n",
    count,
    (count == 1) ? "reservation" : "reservations");

  return(SUCCESS);
  }  /* END UIResList() */




int ConfigShow(

  char *SBuffer, /* O */
  int   PIndex,  /* I */
  int   Vflag)   /* I */

  {
  int  pindex;
  int  qindex;
  int  cindex;

  mpar_t   *P;
  mqos_t   *Q;
  mclass_t *C;

  char     *ptr;

  ptr = SBuffer;

  ptr[0] = '\0';

  sprintf(temp_str,"# %s version %s (PID: %d)\n",
    MSCHED_NAME,
    MSCHED_VERSION,
    MOSGetPID()
    );
  strcat(ptr,temp_str);

  if (MSched.CrashMode == TRUE)
    {
    sprintf(temp_str,"# CRASHMODE initiated\n\n");
    strcat(ptr,temp_str);
    }

  /* policies */

  for (pindex = 0;pindex < MAX_MPAR;pindex++)
    {
    P = &MPar[pindex];

    if (P->Name[0] == '\0')
      continue;

    MParConfigShow(P,Vflag,PIndex,ptr); /* pass parameter index */

    MPrioConfigShow(Vflag,pindex,ptr);  /* pass partition index */
    }   /* END for (pindex) */

  P = &MPar[0];

  DBG(4,fUI) DPrint("INFO:     policy parameters displayed\n");

  /* standing reservations */

  MSRConfigShow(NULL,Vflag,PIndex,ptr);

  ptr += strlen(ptr);

  MNodeConfigShow(&MSched.DefaultN,Vflag,PIndex,ptr);
  MNodeConfigShow(NULL,Vflag,PIndex,ptr);

  ptr += strlen(ptr);

  MRMOConfigShow(NULL,Vflag,PIndex,ptr);

  ptr += strlen(ptr);

  /* class values */

  MClassConfigShow(NULL,Vflag,ptr);

  for (cindex = 0;cindex < MAX_MCLASS;cindex++)
    {
    C = &MClass[cindex];

    if ((C->Name[0] != '\0') && (C->Name[1] != '\1'))
      MCredConfigShow((void *)C,mxoClass,Vflag,PIndex,ptr);
    }  /* END for (cindex) */

  ptr += strlen(ptr);

  /* QOS values */

  for (qindex = 0;qindex < MAX_MQOS;qindex++)
    {
    Q = &MQOS[qindex];

    if (Q->Name[0] == '\0')
      break;

    MQOSConfigShow(Q,Vflag,PIndex,ptr,MAX_MBUFFER);
    }  /* END for (qindex) */

  ptr += strlen(ptr);

  MServerConfigShow(ptr,PIndex,Vflag);

  ptr += strlen(ptr);

  MSchedOConfigShow(ptr,PIndex,Vflag);
  MSchedConfigShow(&MSched,Vflag,ptr,MAX_MLINE);

  ptr += strlen(ptr);

  DBG(4,fUI) DPrint("INFO:     file/directory parameters displayed\n");

  MAMConfigShow(NULL,Vflag,ptr);

  ptr += strlen(ptr);

  MRMConfigShow(NULL,Vflag,ptr,MAX_MBUFFER);

  ptr += strlen(ptr);

  DBG(4,fUI) DPrint("INFO:     miscellaneous parameters displayed\n");

  /* simulation parameters */

  if ((MSched.Mode == msmSim) || Vflag || (PIndex == -1))
    {
    MSimShow(&MSim,ptr,Vflag);
    }

  return(SUCCESS);
  }  /* END ConfigShow() */




int UIStatShow(

  char *RBuffer,  /* I */
  char *Buffer,   /* O */
  int   FLAGS,
  char *Auth,
  long *BufSize)

  {
  int  ObjectType;
  char ObjectID[MAX_MNAME];

  char PName[MAX_MNAME];

  mpar_t *P;

  int  Flags;
  int  rc;

  /* FORMAT:  <OBJECTTYPE> <OBJECTID> */

  const char *FName = "UIStatShow";

  DBG(3,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);

  MUSScanF(RBuffer,"%d %x%s %x%s %d",
    &ObjectType,
    sizeof(ObjectID),
    ObjectID,
    sizeof(PName),
    PName,
    &Flags);

  if (MParFind(PName,&P) == FAILURE)
    P = &MPar[0];

  switch (ObjectType)
    {
    case mxoUser:
    case mxoGroup:
    case mxoAcct:
    case mxoClass:
    case mxoQOS:

      rc = UIShowCStats(ObjectID,ObjectType,Buffer,BufSize);

      break;

    case mxoPar:

      rc = UIParShowStats(ObjectID,Buffer,BufSize);

      break;

    case mxoNode:

      rc = UINodeStatShow(Flags,Buffer,BufSize);

      break; 

    case mxoSched:

      rc = MSchedStatToString(&MSched,mwpNONE,Buffer,*BufSize);

      break; 

    default:

      sprintf(Buffer,"stat type not handled\n");

      rc = FAILURE;

      break;
    }  /* END switch (ObjectType) */

  return(rc);
  }  /* END UIStatShow() */




int UIResDestroy(

  char *RBuffer,  /* I */
  char *Buffer,   /* O */
  int   FLAGS,    /* I */
  char *Auth,     /* I */
  long *BufSize)  /* I */

  {
  int    aindex;

  int    rtype;

  char   RID[MAX_MNAME];

  mres_t *R;

  char  *ptr;
  char  *TokPtr;

  int    Access;

  const char *FName = "UIResDestroy";

  DBG(2,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);

  Buffer[0] = '\0';

  /* Format:   <RESID>[ <RESID>]...  */

  ptr = MUStrTok(RBuffer," \t\n",&TokPtr);

  while (ptr != NULL)
    {
    MUStrCpy(RID,ptr,sizeof(RID));

    ptr = MUStrTok(NULL," \t\n",&TokPtr);

    /* locate reservation */

    if (MResFind(RID,&R) == FAILURE)
      {
      DBG(3,fUI) DPrint("ALERT:    cannot locate reservation '%s'\n",
        RID);

      sprintf(temp_str,"ERROR:    cannot locate reservation '%s'\n",
        RID);
      strcat(Buffer,temp_str);

      continue;
      }

    /* security check */

    Access = FALSE;

    if ((FLAGS & ((1 << fAdmin1) | (1 << fAdmin2))))
      {
      Access = TRUE;
      }
    else if ((R->U != NULL) && !strcmp(R->U->Name,Auth))
      {
      Access = TRUE;
      }
    else if (R->Type == mrtUser)
      {
      for (aindex = 0;R->ACL[aindex].Name[0] != '\0';aindex++)
        {
        if ((R->ACL[aindex].Type == maUser) && !strcmp(R->ACL[aindex].Name,Auth))
          {
          Access = TRUE;

          break;
          }
        }  /* END for (aindex) */
      }

    if (Access == FALSE)
      {
      DBG(2,fUI) DPrint("INFO:     user %s is not authorized to release reservation '%s'\n",
        Auth,
        RID);

      sprintf(temp_str,"ERROR:     user %s is not authorized to release reservation '%s'\n",
        Auth,
        RID);
      strcat(Buffer,temp_str);

      continue;
      }

    rtype = R->Type;

    MResDestroy(&R);

    DBG(3,fUI) DPrint("INFO:     released %s reservation '%s'\n",
      MResType[rtype],
      RID);

    sprintf(temp_str,"released %s reservation '%s'\n",
      MResType[rtype],
      RID);
    strcat(Buffer,temp_str);

    if (MSched.Mode != msmSim)
      {
      MOSSyslog(LOG_INFO,"%s reservation %s released",
        MResType[rtype],
        RID);
      }
    }    /* END while (ptr) */

  return(SUCCESS);
  }  /* END UIResDestroy() */




int UIQueueShowEJobs(

  char   *SBuffer,   /* O */
  long   *SBufSize,  /* I/O */
  mpar_t *P,         /* I */
  char   *UName)     /* I */

  {
  int      index;

  char     Line[MAX_MLINE];
  mjob_t  *J;
  char     resState;

  mreq_t  *RQ;

  char    *CListPtr;

  char    *BPtr;
  int      BSpace;

  const char *FName = "UIQueueShowEJobs";

  DBG(2,fUI) DPrint("%s(SBuffer,BufSize,%s,%s)\n",
    FName,
    (P != NULL) ? P->Name : "NULL",
    (UName != NULL) ? UName : "NULL");

  if ((P == NULL) || (SBuffer == NULL))
    {
    return(FAILURE);
    }
 
  BPtr   = SBuffer;
  BSpace = *SBufSize;

  MUSNPrintF(&BPtr,&BSpace,"%ld %d\n",
    MSched.Time,
    P->URes.Procs);

  for (index = 0;MUIQ[index] != -1;index++)
    {
    J = MJob[MUIQ[index]];

    if ((J == NULL) || (J->Name[0] == '\0') || (J->Name[0] == '\1'))
      continue;

    if ((P->Index > 0) && (MUBMCheck(P->Index,J->PAL) == FAILURE))
      continue;

    if (J->MasterJobName != NULL)
      continue;

    if ((J->State == mjsStarting) || (J->State == mjsRunning))
      continue;

    if ((UName[0] != '\0') && strcmp(J->Cred.U->Name,UName))
      continue;

    RQ = J->Req[0]; /* FIXME */

    if (P->Index > 0)
      {
      if (MUNumListGetCount(
            J->StartPriority,
            RQ->DRes.PSlot,
            P->CRes.PSlot,
            0,
            NULL) == FAILURE)
        {
        DBG(7,fSCHED) DPrint("INFO:     classes not available for job %s in partition %s (%s)\n",
          J->Name,
          P->Name,
          MUCAListToString(RQ->DRes.PSlot,P->CRes.PSlot,NULL));

        break;
        }

      DBG(4,fUI) DPrint("INFO:     job '%s' can run in partition %s\n",
        J->Name,
        P->Name);
      }

    if (J->R != NULL)
      resState = '*';
    else
      resState = 'X';

    /* Format:  <JOBNAME> <UID> <GID> <QUEUETIME> <MINPROCS> <MEMORY> <CPULIMIT> <PRIORITY> <QOS> <RES-STATE> */

    if (J->Cred.C != NULL)
      CListPtr = J->Cred.C->Name;
    else
      CListPtr = MUCAListToString(RQ->DRes.PSlot,NULL,NULL);

    sprintf(Line,"%s %s %s %ld %d %d %ld %ld %s %c %s\n",
      J->Name,
      J->Cred.U->Name,
      (J->Cred.G != NULL) ? J->Cred.G->Name : NONE,
      J->SystemQueueTime,
      J->Request.TC * J->Req[0]->DRes.Procs,
      J->Request.TC * J->Req[0]->DRes.Mem,
      J->SpecWCLimit[0],
      J->StartPriority,
      ((J->Cred.Q != NULL) && (J->Cred.Q->Index != 0)) ? J->Cred.Q->Name : "-",
      resState,
      (CListPtr[0] != '\0') ? CListPtr : NONE);

    MUSNPrintF(&BPtr,&BSpace,"%s",
      Line);

    if (BSpace <= 300)
      {
      DBG(2,fUI) DPrint("ALERT:    buffer overflow in %s()\n",
        FName);

      MUSNPrintF(&BPtr,&BSpace,"NOTE:  list truncated\n\n");

      break;
      }

    DBG(5,fUI) DPrint("INFO:     job line: '%s' (%d)\n",
      Line,
      (int)strlen(SBuffer));
    }  /* END for (index) */

  DBG(5,fUI) DPrint("INFO:     %s() buffer size: %d bytes\n",
    FName,
    (int)strlen(SBuffer));

  return(SUCCESS);
  }  /* UIQueueShowEJobs() */




/* HvB */

int UIQueueShowBJobs(
 
  char   *SBuffer,   /* O */
  long   *SBufSize,  /* I/O */
  mpar_t *P,         /* I */
  char   *UName)     /* I (optional) */
 
  {
  int      index;
  int      BlockedJob;

  char     Line[MAX_MLINE];
  mjob_t  *J; 

  char    *BPtr;
  int      BSpace;

  int     SrcQ[2];
  mjob_t *tmpQ[2];
  char     tmpBuffer[MAX_MBUFFER];    

  const char *FName = "UIQueueShowBJobs";

  DBG(2,fUI) DPrint("%s(SBuffer,BufSize,%s)\n",
    FName,
    (P != NULL) ? P->Name : "NULL");

  if ((P == NULL) || (SBuffer == NULL))
    {
    return(FAILURE);
    }

  BPtr   = SBuffer;
  BSpace = *SBufSize;

  MUSNPrintF(&BPtr,&BSpace,"%ld %d\n",
    MSched.Time, 0);

  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    BlockedJob = FALSE;

    /* continue if active job found */

    if ((J->State == mjsRunning) ||
        (J->State == mjsStarting) ||
        (J->State == mjsSuspended) ||
        (J->IState != mjsNONE))
      {
      continue;
      }

    if ((UName[0] != '\0') && strcmp(J->Cred.U->Name,UName))
      continue;

    DBG(9,fUI) DPrint("INFO:     Block Reason job %s : %d, %d\n", 
      J->Name, 
      J->BlockReason, 
      J->HoldReason);

    /* We know for sure we have an blocked job */

    if (J->BlockReason > 0)
      {
      BlockedJob = TRUE;
      } /* end if (J->BlockedReason > 0) */
    else
      {
      /* Check if job is in the idle queue, if not job is blocked */

      BlockedJob = TRUE; 

      for (index = 0;MUIQ[index] != -1;index++)
        {
        if (MUIQ[index] == J->Index)
          {
          BlockedJob = FALSE;

          break;
          }
        }  /* END for (index) */
      }

    if (BlockedJob == TRUE)
      {
      MOQueueInitialize(SrcQ); 
      tmpQ[0] = J;
      tmpQ[1] = NULL;

      SrcQ[0] =  0;
      SrcQ[1] = -1;

      if ((MQueueSelectAllJobs(
          tmpQ,
          ptHARD,
          P,
          SrcQ,
          FALSE,
          FALSE,
          FALSE,
          tmpBuffer) == FAILURE) || (SrcQ[0] == -1))
        {
        sprintf(Line,"%s %s %s\n", 
          J->Name, 
          J->Cred.U->Name, 
          tmpBuffer);

        MUSNPrintF(&BPtr,&BSpace,"%s",
          Line);
    
        if (BSpace <= 300)
          {
          DBG(2,fUI) DPrint("ALERT:    buffer overflow in %s()\n", 
            FName);
 
          MUSNPrintF(&BPtr,&BSpace,"NOTE:  list truncated\n\n"); 

          break;
          }
 
        DBG(2,fUI) DPrint("INFO:     job line: '%s' (%d)\n", 
          Line, 
          (int)strlen(SBuffer));
        }
      } /* END if (BlockedJob == TRUE) */
    }   /* END for (J = MJob) */

  return(SUCCESS);
  } /* UIQueueShowBJobs() */




int UIQueueShowAJobs(

  char   *SBuffer,  /* O */
  long   *SBufSize, /* I/O */
  mpar_t *PS,       /* I */
  int     Flags,    /* I */
  char   *UName)    /* I (optional) */

  {
  int      jindex;
  int      nindex;

  long     Limit;

  char     Line[MAX_MLINE];
  mjob_t  *J;

  mreq_t  *RQ;

  int      TotalAProcs = 0;

  mpar_t *P;

  char   *BPtr;
  int     BSpace;

  const char *FName = "UIQueueShowAJobs";

  DBG(2,fUI) DPrint("%s(SBuffer,SBufSize,%s,%d)\n",
    FName,
    PS->Name,
    Flags);

  BPtr   = SBuffer;
  BSpace = (int)*SBufSize;

  BPtr[0] = '\0';

  /* generate summary information */

  TotalAProcs = 0;

  for (jindex = 0;MAQ[jindex] != -1;jindex++)
    {
    J = MJob[MAQ[jindex]];

    if ((J->State == mjsStarting) || (J->State == mjsRunning))
      TotalAProcs += MJobGetProcCount(J);
    }  /* END for (jindex) */

  MUSNPrintF(&BPtr,&BSpace,"%ld %d %d %d %d %d %d\n",
    MSched.Time,
    PS->URes.Procs,
    PS->ARes.Procs,
    PS->UpNodes,
    PS->UpNodes - PS->ActiveNodes,
    PS->URes.Mem,
    TotalAProcs);

  for (jindex = 0;MAQ[jindex] != -1;jindex++)
    {
    J = MJob[MAQ[jindex]];

    RQ = J->Req[0]; /* FIXME */

    if ((PS->Index != 0) && (RQ->PtIndex != PS->Index))
      {
      continue;
      }

    P = &MPar[RQ->PtIndex];

    if (J->MasterJobName != NULL)
      continue;

    if ((UName[0] != '\0') && strcmp(J->Cred.U->Name,UName))
      continue;

    for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++);

    if ((MPar[0].UseCPUTime == TRUE) && (J->CPULimit > 0))
      Limit = J->CPULimit;
    else
      Limit = J->WCLimit;

    /* Format:  <JOBNAME> <JOBSTATE> <UNAME> <GNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <PSDEDICATED> <PSUTILIZED> <PARTITION> <DMEM> <MASTERHOST> <JOBFLAGS> <PRIORITY> */

    sprintf(Line,"%s %s %s %s %ld %ld %d %ld %s %lf %lf %s %d %s %ld %ld %ld\n",
      J->Name,
      MJobState[(J->IState != mjsNONE) ? J->IState : J->State],
      J->Cred.U->Name,
      (J->Cred.G != NULL) ? J->Cred.G->Name : "-",
      J->StartTime,
      J->SystemQueueTime,
      (MSched.DisplayFlags & (1 << dfNodeCentric)) ?
        nindex : MJobGetProcCount(J),
      Limit,
      (J->Cred.Q != NULL) ? J->Cred.Q->Name : "-",
      J->PSDedicated,
      J->PSUtilized,
      P->Name,
      RQ->TaskCount * RQ->DRes.Mem,
      ((J->NodeList != NULL) && (J->NodeList[0].N != NULL) && (J->NodeList[0].N->Name[0] != '\0')) ?
        MNodeAdjustName(J->NodeList[0].N->Name,0) : NONE,
      J->Flags,
      J->AWallTime,
      J->RunPriority);

    MUSNPrintF(&BPtr,&BSpace,"%s",
      Line);

    DBG(5,fUI) DPrint("INFO:     job line: '%s' (%d)\n",
      Line,
      (int)*SBufSize - BSpace);
    }  /* END for (jindex) */

  return(SUCCESS);
  }  /* END UIQueueShowAJobs() */




int UIResDiagnose(

  char *SBuffer,   /* O */
  long *SBufSize,  /* I */
  int   PIndex,    /* I */
  char *DiagOpt,   /* I */
  int   Flags)     /* I */

  {
  int rindex;
  int nindex;

  mnode_t *N;
  mjob_t  *J;

  int   TotalRC;
  int   TotalResStatePC;
  int   TotalResAPC;

  int   ResEnd;
  int   ResLast;

  mres_t *R;

  char  StatLine[MAX_MLINE];

  const char *FName = "UIResDiagnose";

  DBG(3,fUI) DPrint("%s(SBuffer,SBufSize,%s,%s)\n",
    FName,
    MAList[ePartition][PIndex],
    DiagOpt);

  /* check all reservations */

  DBG(4,fUI) DPrint("INFO:     diagnosing reservation table (%d slots)\n",
    MAX_MRES);

  sprintf(temp_str,"Diagnosing Reservations\n");
  strcat(SBuffer,temp_str);

  MResShowState(NULL,Flags,SBuffer,*SBufSize,mAdd);

  TotalRC = 0;

  ResLast = 0;
  ResEnd  = MAX_MRES;

  TotalResAPC     = 0;
  TotalResStatePC = 0;

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    if (N->State == mnsReserved)
      TotalResStatePC += N->CRes.Procs;
    }  /* END for (nindex) */

  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    R = MRes[rindex];

    if ((R == NULL) || (R->Name[0] == '\0'))
      break;

    if (R->Name[0] == '\1')
      continue;

    if ((PIndex > 0) && (PIndex != R->PtIndex))
      continue;

    if (strcmp(DiagOpt,NONE) && strcmp(DiagOpt,R->Name))
      continue;

    /* look for isolated reservations (reservation table corruption) */

    if (R->Name[0] == '\0')
      {
      if (ResEnd == MAX_MRES)
        {
        ResEnd = rindex;
        }

      continue;
      }

    ResLast = rindex;

    DBG(6,fUI) DPrint("INFO:     evaluating MRes[%03d] '%s'\n",
      rindex,
      R->Name);

    MResShowState(R,Flags,SBuffer,*SBufSize,mAdd);

    MResDiagnoseState(
      R,
      Flags,
      SBuffer,
      *SBufSize,
      mAdd);

    TotalRC++;

    StatLine[0] = '\0';

    /* no reservation stats displayed */

    if ((strlen(SBuffer) + strlen(StatLine)) < (*SBufSize - 300))
      {
      strcat(SBuffer,StatLine);
      }
    else
      {
      DBG(2,fUI) DPrint("ALERT:    buffer overflow in %s()\n",
        FName);

      strcat(SBuffer,"NOTE:  list truncated\n\n");

      break;
      }

    if ((R->Type == mrtJob) && (R->J != NULL))
      {
      J = (mjob_t *)R->J;

      if (MJOBISACTIVE(J))
        TotalResAPC += R->AllocPC;
      }
    }  /* END for (rindex) */

  if (!strcmp(DiagOpt,NONE))
    {
    sprintf(temp_str,"\nActive Reserved Processors: %d\n",
      TotalResAPC);
    strcat(SBuffer,temp_str);

    if ((TotalResAPC + TotalResStatePC) != MPar[0].DRes.Procs)
      {
      sprintf(temp_str,"WARNING:  reservation table is corrupt:  active procs reserved does not equal active procs detected (%d != %d)\n",
        TotalResAPC + TotalResStatePC,
        MPar[0].DRes.Procs);
      }

    if (ResLast > ResEnd)
      {
      sprintf(temp_str,"WARNING:  reservation table is corrupt:  empty slot detected (slot: %d) before end of table\n",
        ResEnd);
      strcat(SBuffer,temp_str);
      }

    MResDiagGrid(SBuffer,(int)*SBufSize,0);
    }  /* END if (!strcmp(DiagOpt,NONE) */

  return(SUCCESS);
  }  /* END UIResDiagnose() */


/* END UserI.c */


