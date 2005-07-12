/* HEADER */

/* contains:                                    *
 *                                              *
 * int MAMInitialize(AS)                        *
 * int MAMAllocJDebit(AM,J,SC,EMsg)             *
 * int MAMAccountGetDefault(UName,AName,RIndex) *
 *
 *                                              */

#include "moab.h"
#include "moab-proto.h"
 
extern mlog_t mlog;
 
extern mnode_t  *MNode[];
extern mframe_t  MFrame[];
extern mpar_t    MPar[];
extern mrm_t     MRM[];
extern mam_t     MAM[];
 
extern mres_t   *MRes[];
extern mgcred_t  MAcct[];
 
extern msched_t  MSched;
extern mjob_t   *MJob[];
extern int       MAQ[];
 
extern const char *MAMType[];
extern const char *MAMState[];
extern const char *MAMOType[];
extern const char *MAMChargePolicy[];
extern const char *MSockProtocol[];
extern const char *MWireProtocol[];
extern const char *MAMAttr[];
extern const char *MAMProtocol[];
extern const char *MCSAlgoType[];
extern const char *MS3CName[];
extern const char *MAMFuncType[];
extern const char *MS3SockProtocol[];
extern const char *MJFActionType[];
extern char       *MS3OName[][MMAX_S3ATTR];
extern char       *MS3JobAttr[][MMAX_S3ATTR];
extern char       *MS3ReqAttr[][MMAX_S3ATTR];

extern char       *MS3ObjName[][MMAX_S3ATTR];
extern char       *MSON[];
extern char       *MSAN[];

#define MAMCfgParam "AMCFG"     

#define MAM_FILEFOOTER "BANKEND\n"

mbool_t MAMTest;

const char *MAMCommands[] = {
  NONE,
  "make_withdrawal",
  NULL };

enum MAMCmdTypes {
  mamcNone = 0,
  mamcJobDebitAlloc };
 

typedef struct
  {
  char *Attr;
  char  Value[MMAX_LINE];
  } mgavplist_t;


/* internal prototypes */

int __MAMStartFunc(mam_t *,int);
int __MAMEndFunc(mam_t *,int);
int MAMGoldDoCommand(mxml_t *,mpsi_t *,enum MHoldReasonEnum *,char *);

/* END internal prototypes */





int MAMInitialize(

  mam_t *AS)  /* I (modified) */

  {
  int aindex;

  mam_t *A;

  const char *FName = "MAMInitialize";

  MDB(3,fAM) MLog("%s(%s)\n",
    FName,
    (AS != NULL) ? AS->Name : "NULL");

  if (getenv(MSCHED_ENVAMTESTVAR) != NULL)
    {
    MDB(1,fAM) MLog("INFO:     AM test mode enabled\n");

    MAMTest = TRUE;
    }
  else
    {
    MAMTest = FALSE;
    }

  if ((MSched.Mode != msmNormal) && (MAMTest == FALSE))
    {
    return(SUCCESS);
    }

  /* NOTE:  must distinguish between multiple AM's */

  for (aindex = 0;aindex < MMAX_AM;aindex++)
    {
    A = &MAM[aindex];

    if (A->Type == mamtNONE) 
      continue;

    if ((AS != NULL) && (A != AS))
      continue;

    /* determine AM version */

    switch(A->Type)
      {
      case mamtQBANK:

        {
        char tmpSBuffer[MMAX_LINE];
        char tmpRBuffer[MMAX_LINE];

        int rc;
        int tmpSC;

        sprintf(tmpSBuffer,"COMMAND=version AUTH=%s",
          MSched.Admin1User[0]);

        rc = MAMQBDoCommand(
          A,
          0,
          tmpSBuffer,
          NULL,
          &tmpSC,
          tmpRBuffer);

        if (rc == SUCCESS)
          {
          /* extract version */

          /* FORMAT:  A.B.C[.D]... -> %02d%02d%02d */

          /* NYI */

          A->State = mrmsActive;
          }  /* END if (rc == SUCCESS) */
        else
          {
          A->State = mrmsDown;
          }
        }    /* END BLOCK */

        break;

      case mamtGOLD:

        {
        /* dynamic version detection not supported */

        /* version can be specified via config file */

        A->P.SocketProtocol = mspHTTP;
        A->P.WireProtocol   = mwpS32;

        A->P.HostName       = A->Host;
        A->P.Port           = A->Port;

        A->P.CSAlgo         = A->CSAlgo;
        A->P.CSKey          = A->CSKey;

        A->P.Timeout        = A->Timeout;

        A->P.Type = mpstAM;

        /* create socket */

        MSUCreate(&A->P.S);

        /* no initialization phase */

        A->State = mrmsActive;
        }  /* END BLOCK */

        break;

      default:

        A->Version = 0;

        break;
      }  /* END switch(A->Type) */

    /* TEMP:  register allocation manager */

    if (MSysDSRegister(
         (char *)MS3CName[mpstAM],
         MRM[0].Name,
         A->Host,
         A->Port,
         NULL,
         (char *)MS3SockProtocol[A->SocketProtocol]) == FAILURE)
      {
      MDB(1,fRM) MLog("ALERT:    cannot register AM with directory service\n");
      }

    if (A->UseDirectoryService == TRUE)
      {
      char HostName[MMAX_NAME];
      int  Port;
      char WireProtocol[MMAX_NAME];
      char SocketProtocol[MMAX_NAME];

      /* obtain interface information */

      if (MSysDSQuery(
           (char *)MS3CName[mpstSC],
           MRM[0].Name,
           HostName,
           &Port,
           WireProtocol,
           SocketProtocol) == SUCCESS)
        {
        /* update active AM interface */

        /* NYI */
        }
      }    /* END if (A->UseDirectoryService == TRUE) */
    }      /* END for (aindex) */

  return(SUCCESS);
  }  /* END MAMInitialize() */





int MAMAllocJDebit(

  mam_t  *A,       /* I */
  mjob_t *J,       /* I */
  enum MHoldReasonEnum *SC, /* O */
  char   *ErrMsg)  /* O (optional) */

  {
  char AMISBuffer[MMAX_LINE << 2];
  char AMIRBuffer[MMAX_LINE << 2];
  char NodeType[MMAX_NAME];
 
  char AccountName[MMAX_NAME];
  char QOSString[MMAX_NAME];

  char JobType[MMAX_NAME];
 
  int  rc;
  int  tmpSC;
 
  mnode_t *N;
 
  double ProcConsumptionRate;
 
  int    ProcCount = 0;
  int    Memory = 0; 

  long   Duration;

  mrm_t *R;

  const char *FName = "MAMAllocJDebit";

  MDB(3,fAM) MLog("%s(A,%s,SC,ErrMsg)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if (SC != NULL)
    *SC = mhrNONE;

  if (ErrMsg != NULL)
    ErrMsg[0] = '\0';

  if ((A == NULL) || (J == NULL))
    {
    return(FAILURE);
    }
 
  if ((A->Type == mamtNONE) ||
     ((MSched.Mode != msmNormal) && (MAMTest != TRUE)))
    {
    return(SUCCESS);
    }

  if (A->State == mrmsDown)
    {
    return(FAILURE);
    }

  R = (J->RM != NULL) ? J->RM : &MRM[0];

  if (((J->Cred.A == NULL) || (!strcmp(J->Cred.A->Name,NONE))) &&
       (MSched.DefaultAccountName[0] == '\0') &&
       (A->Type != mamtQBANK) &&
       (A->Type != mamtGOLD) &&
       (A->Type != mamtFILE))
    {
    MDB(2,fAM) MLog("ALERT:    no account specified for job %s\n",
      J->Name);
 
    return(FAILURE);
    }
 
  if ((J->Cred.A != NULL) && strcmp(J->Cred.A->Name,NONE))
    {
    strcpy(AccountName,J->Cred.A->Name);
 
    if (A->AppendMachineName == TRUE)
      {
      strcat(AccountName,"@");
      strcat(AccountName,R->Name); 
      }
    }
  else
    {
    strcpy(AccountName,MSched.DefaultAccountName);
    }

  strcpy(JobType,"job");

  if (J->CompletionTime > J->StartTime) 
    Duration = (long)J->CompletionTime - J->StartTime;
  else
    Duration = 1;

  /* NOTE:  support QOS dedicated (NYI) */

  if ((J->Cred.Q != NULL) && 
      (J->Cred.Q->Flags & (1 << mqfdedicated)) &&
      (J->NodeList != NULL))
    {
    int nindex;

    ProcCount = 0;

    for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
      {
      ProcCount += J->NodeList[nindex].N->CRes.Procs;
      }  /* END for (nindex) */
    }
  else
    { 
    ProcCount = MAX(1,MJobGetProcCount(J));
    } 

  ProcConsumptionRate = 1.0;
 
  switch (A->ChargePolicy)
    {
    case mamcpDebitAllPE:
    case mamcpDebitSuccessfulPE:

      /* NO-OP */
 
      break;
 
    case mamcpDebitAllCPU:
    case mamcpDebitSuccessfulCPU:
 
      if ((J->PSUtilized > 0.01))
        {
        if (J->Req[0]->LURes.Procs > 0)
          ProcConsumptionRate = (double)J->Req[0]->LURes.Procs / 100.0 / Duration / ProcCount;
        else
          ProcConsumptionRate = (double)J->PSUtilized / Duration / ProcCount;
        }
      else
        {
        ProcConsumptionRate = 1.0;
        }
 
      break;
 
    case mamcpDebitAllWC:
    case mamcpDebitSuccessfulWC:
    default: 

      ProcConsumptionRate = 1.0;
 
      break;
    }  /* END switch(MSched.AMChargePolicy) */
 
  if (MUNLGetMinAVal(J->NodeList,mnaNodeType,NULL,(void **)NodeType) == FAILURE)
    {
    strcpy(NodeType,MDEF_NODETYPE);
    
    if (J->Req[0]->NodeList[0].N != NULL) 
      {
      N = J->Req[0]->NodeList[0].N;

      if ((N->FrameIndex != -1) && (MFrame[N->FrameIndex].NodeType[0] != '\0'))
        strcpy(NodeType,MFrame[N->FrameIndex].NodeType);
      else if (N->NodeType[0] != '\0')
        strcpy(NodeType,N->NodeType);
      }
    }
 
  /* determine QOS */
 
  if ((J->Cred.Q != NULL) && strcmp(J->Cred.Q->Name,DEFAULT))
    {
    sprintf(QOSString,"QOS=%s ",
      J->Cred.Q->Name);
    }
  else
    {
    QOSString[0] = '\0';
    }
 
  switch (A->Type)
    {
    case mamtFILE:
 
      fprintf(A->FP,"%-15s TYPE=job MACHINE=%s ACCOUNT=%s SUBACCOUNT=%s RESOURCE=%d PROCCRATE=%.2lf RESOURCETYPE=%s DURATION=%ld %sREQUESTID=%s NODES=%d\n",
        "WITHDRAWAL",
        R->Name,
        AccountName,
        J->Cred.U->Name,
        ProcCount,
        ProcConsumptionRate,
        NodeType,
        Duration,
        QOSString,
        J->Name,
        J->NodeCount );
 
      fflush(A->FP);
 
      break;
 
    case mamtQBANK:
 
      /* FORMAT:  QOSFACTOR=<FACTORSPEC> NODEFACTOR=<FACTORSPEC> TIMEFACTOR=<FACTORSPEC> */
      /*          <FACTORSPEC> <X>[*<FRACTION>][:<X>[*<FRACTION>]]...                    */
 
      /* PROCCOUNT -> PROCS (QBank 2.9) */
      /* CLASS supported in 2.9 */
 
      sprintf(AMISBuffer,"COMMAND=make_withdrawal AUTH=%s MACHINE=%s%s%s USER=%s WCTIME=%ld PROCCOUNT=%d PROCCRATE=%.2lf %sCLASS=%s NODETYPE=%s JOBID=%s JOBTYPE=job",        
        MSched.Admin1User[0],
        R->Name,
        (AccountName[0] != '\0') ? " ACCOUNT=" : "",
        AccountName,
        J->Cred.U->Name,
        Duration,
        ProcCount,
        ProcConsumptionRate,
        QOSString,
        (J->Cred.C != NULL) ? J->Cred.C->Name : DEFAULT,
        NodeType,
        J->Name);

      rc = MAMQBDoCommand(
             A,
             0,
             AMISBuffer,
             NULL,
             &tmpSC,
             AMIRBuffer);

      break;

    case mamtGOLD:

      {
      mxml_t *JE = NULL;

      char *RspBuf = NULL;

      mxml_t *RE;

      mxml_t *DE;
      mxml_t *AE;

      /* create request string, populate S->SE */

      RE = NULL;

      MXMLCreateE(&RE,MSON[msonRequest]);

      MXMLSetAttr(RE,(char *)MSAN[msanAction],(void *)"Charge",mdfString);

      MS3SetObject(RE,"Job",NULL);

      DE = NULL;

      MXMLCreateE(&DE,MSON[msonData]);

      MXMLAddE(RE,DE);

      JE = NULL;

      MXMLCreateE(&JE,"Job");

      MXMLAddE(DE,JE);

      AE = NULL;
      MXMLCreateE(&AE,"JobId");
      MXMLSetVal(AE,(void *)J->Name,mdfString);
      MXMLAddE(JE,AE);

      if (J->Cred.U != NULL)
        {
        AE = NULL;
        MXMLCreateE(&AE,(char *)MS3JobAttr[A->Version][mjaUser]);
        MXMLSetVal(AE,(void *)J->Cred.U->Name,mdfString);
        MXMLAddE(JE,AE);
        }

      if (J->Cred.A != NULL)
        {
        AE = NULL;
        MXMLCreateE(&AE,(char *)MS3JobAttr[A->Version][mjaAccount]);
        MXMLSetVal(AE,(void *)J->Cred.A->Name,mdfString);
        MXMLAddE(JE,AE);
        }

      if (J->Cred.Q != NULL)
        {
        AE = NULL;
        MXMLCreateE(&AE,(char *)MS3JobAttr[A->Version][mjaQOS]);
        MXMLSetVal(AE,(void *)J->Cred.Q->Name,mdfString);
        MXMLAddE(JE,AE);
        }

      if (R != NULL)
        {
        AE = NULL;
        MXMLCreateE(&AE,"MachineName");
        MXMLSetVal(AE,(void *)R->Name,mdfString);
        MXMLAddE(JE,AE);
        }

      if (ProcCount > 0)
        {
        AE = NULL;
        MXMLCreateE(&AE,(char *)MS3ReqAttr[A->Version][mrqaTCReqMin]);
        MXMLSetVal(AE,(void *)&ProcCount,mdfInt);
        MXMLAddE(JE,AE);
        }

      if (Memory > 0)
        {
        AE = NULL;
        MXMLCreateE(&AE,"Memory");
        MXMLSetVal(AE,(void *)&Memory,mdfInt);
        MXMLAddE(JE,AE);
        }

      if (Duration > 0)
        {
        AE = NULL;
        MXMLCreateE(&AE,(char *)MS3JobAttr[A->Version][mjaAWDuration]);
        MXMLSetVal(AE,(void *)&Duration,mdfLong);
        MXMLAddE(JE,AE);
        }

      if (J->Cred.C != NULL)
        {
        AE = NULL;
        MXMLCreateE(&AE,MS3ReqAttr[A->Version][mrqaReqClass]);
        MXMLSetVal(AE,(void *)&J->Cred.C->Name,mdfString);
        MXMLAddE(JE,AE);
        }

      if (NodeType[0] != '\0')
        {
        AE = NULL;
        MXMLCreateE(&AE,"NodeType");
        MXMLSetVal(AE,(void *)NodeType,mdfString);
        MXMLAddE(JE,AE);
        }

      if (JobType[0] != '\0')
        {
        AE = NULL;
        MXMLCreateE(&AE,"Type");
        MXMLSetVal(AE,(void *)JobType,mdfString);
        MXMLAddE(JE,AE);
        }

      /* submit request */

      {
      enum MHoldReasonEnum RIndex;

      if (MAMGoldDoCommand(RE,&A->P,&RIndex,ErrMsg) == FAILURE)
        {
        MDB(2,fAM) MLog("ALERT:    cannot debit allocation for job\n");

        if (SC != NULL)
          *SC = RIndex;

        if ((A->JFAction == mamjfaNONE) && (RIndex != mhrNoFunds))
          {
          return(SUCCESS);
          }

        return(FAILURE);
        }
      }    /* END BLOCK */

      /* charge successful */

      MSysEMSubmit(
        &MSched.EM,
        (char *)MS3CName[mpstAM],
        "joballoccharge",
        J->Name);

      MUFree(&RspBuf);
      }      /* END BLOCK */

      break;
 
    case mamtNONE:
 
      return(SUCCESS);
 
      /*NOTREACHED*/
 
      break;
 
    default:
 
      return(SUCCESS);
 
      /*NOTREACHED*/ 
 
      break;
    }  /* switch (A->Type) */
 
  return(SUCCESS);
  }  /* END MAMAllocJDebit() */




int MAMAllocRDebit(

  mam_t *AM,       /* I */
  mres_t *R,       /* I */
  enum MHoldReasonEnum *RIndex, /* O (optional) */
  char   *ErrMsg)  /* O */

  {
  char  AMISBuffer[MMAX_LINE << 2];
  char  AMIRBuffer[MMAX_LINE << 2];

  char  NodeType[MMAX_NAME];
 
  char  AName[MMAX_NAME];
  char  SAName[MMAX_NAME];
 
  int   rc;
  int   StatusCode;
  int   tmpSC;
 
  char *ptr;

  long  WCTime;
 
  double PCRate;

  const char *FName = "MAMAllocRDebit";
 
  MDB(3,fAM) MLog("%s(%s,SC,ErrMsg)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");

  if (RIndex != NULL)
    *RIndex = mhrNONE;
 
  if (ErrMsg != NULL)
    ErrMsg[0] = '\0';
 
  if ((AM == NULL) || (R == NULL))
    {
    return(FAILURE);
    }
 
  if ((AM->Type == mamtNONE) ||
     ((MSched.Mode != msmNormal) && (MAMTest != TRUE)))
    {
    return(SUCCESS);
    }

  /* NOTE:  no default accounts for reservations */      
 
  if ((R->A == NULL) || !strcmp(R->A->Name,NONE)) 
    {
    MDB(2,fAM) MLog("ALERT:    no account specified for res %s\n",
      R->Name);

    if (RIndex != NULL) 
      *RIndex = mhrNoFunds;
 
    return(FAILURE);
    }

  /* determine sub account */

  /* FORMAT:  <ACCOUNT> = <SUBACCOUNT>[@<ACCOUNT>] */
 
  if ((ptr = strchr(R->A->Name,'@')) != NULL)
    {
    MUStrCpy(SAName,R->A->Name,MIN(sizeof(SAName),(ptr - R->A->Name + 1)));
 
    MUStrCpy(AName,(ptr + 1),sizeof(AName));
    }
  else
    {
    strcpy(SAName,"KITTY");

    MUStrCpy(AName,R->A->Name,sizeof(AName));
    }
 
  if (AM->AppendMachineName == TRUE)
    {
    strcat(AName,"@");
    strcat(AName,MRM[0].Name);  

    /* FIXME:  assumes single RM per scheduler */
    }
 
  WCTime = MAX(1,(long)((R->CAPS + R->CIPS) / MAX(1,R->AllocPC)));
  PCRate = MIN(1.0,(double)R->CIPS / (R->CAPS + R->CIPS));

  strcpy(NodeType,MDEF_NODETYPE);      

  switch (AM->Type)
    {
    case mamtFILE:
 
      fprintf(AM->FP,"%-15s TYPE=res MACHINE=DEFAULT ACCOUNT=%s SUBACCOUNT=%s RESOURCE=%d RESOURCETYPE=%s PROCCRATE=%.2lf DURATION=%ld QOS=%s REQUESTID=%s NODES=%d\n",
        "WITHDRAWAL",
        AName,
        SAName,
        R->AllocPC,
        NodeType,
        PCRate,
        WCTime,
        DEFAULT,
        R->Name,
        R->NodeCount);
 
      fflush(AM->FP);
 
      break;
 
    case mamtQBANK:

      sprintf(AMISBuffer,"COMMAND=make_withdrawal AUTH=%s MACHINE=%s ACCOUNT=%s USER=%s WCTIME=%ld PROCCRATE=%.2lf PROCCOUNT=%d QOS=%s CLASS=%s NODETYPE=%s JOBID=%s JOBTYPE=res",
        MSched.Admin1User[0],
        MRM[0].Name,
        AName,
        SAName,
        WCTime,
        PCRate,
        R->AllocPC,
        DEFAULT,
        "DEFAULT",
        NodeType,
        R->Name);
 
      rc = MAMQBDoCommand(
        AM,
        0,
        AMISBuffer,
        NULL,
        &StatusCode,
        AMIRBuffer);

      if (rc == FAILURE)
        {
        MDB(1,fAM) MLog("ALERT:    cannot debit account %s for reservation '%s' (AM failure)\n",
          R->A->Name,
          R->Name);

        if (RIndex != NULL) 
          *RIndex = mhrAMFailure;

        if (AM->JFAction != mamjfaNONE) 
          {
          return(FAILURE);
          }

        return(SUCCESS);
        }
 
      if (StatusCode == 0)
        {
        MDB(1,fAM) MLog("ALERT:    cannot debit account %s for reservation '%s' (request refused)\n",
          R->A->Name,
          R->Name);

        if (RIndex != NULL) 
          *RIndex = mhrNoFunds;
 
        return(FAILURE);
        }
 
      break;

    case mamtGOLD:

      {
      mxml_t *E  = NULL;
      mxml_t *C1 = NULL;
      mxml_t *C2 = NULL;
      mxml_t *A  = NULL;
 
      MXMLCreateE(&E,"AllocationManagerRequests");
      MXMLSetAttr(E,"xmlns","http://www.scidac.org/ScalableSystems/AllocationManager",mdfString);
 
      MXMLCreateE(&C1,"createDebitRequest");
      MXMLAddE(E,C1);
 
      MXMLCreateE(&C2,"set");
      MXMLAddE(C1,C2);
 
      MXMLCreateE(&A,"name");
      MXMLSetVal(A,(void *)R->Name,mdfString);
      MXMLAddE(C2,A);
      A = NULL;
 
      if (R->A != NULL)
        {
        MXMLCreateE(&A,"account");
        MXMLSetVal(A,(void *)R->A->Name,mdfString);
        MXMLAddE(C2,A);
        A = NULL;
        }
 
      MXMLToString(E,AMISBuffer,sizeof(AMISBuffer),NULL,TRUE);
 
      rc = MAMQBDoCommand(
        AM,
        0,
        AMISBuffer,
        (void **)&E,
        &tmpSC,
        AMIRBuffer);

      if (E != NULL)
        MXMLDestroyE(&E);

      if (rc == FAILURE)
        {
        if (RIndex != NULL)
          *RIndex = mhrAMFailure;
 
        return(FAILURE);
        }
      }    /* END BLOCK */
 
      return(SUCCESS);
 
      /*NOTREACHED*/
 
      break;

    case mamtNONE:
 
      return(SUCCESS);
 
      /*NOTREACHED*/
 
      break;

    default:
 
      return(SUCCESS);
 
      /*NOTREACHED*/
 
      break;
    }   /* switch (AM->Type) */
 
  return(SUCCESS);
  }  /* END MAMAllocRDebit() */




int MAMAllocJReserve(

  mam_t   *AM,        /* I */
  mjob_t  *J,         /* I */
  mbool_t  TestAlloc, /* I */
  enum MHoldReasonEnum *RIndex, /* O (optional) */
  char    *ErrMsg)    /* O (optional) */

  {
  char    AMISBuffer[MMAX_LINE << 2];
  char    AMIRBuffer[MMAX_LINE];
  char    AccountName[MMAX_NAME];
  char    QOSString[MMAX_NAME];
 
  int     rc;
  int     tmpSC;
 
  char    NodeType[MMAX_NAME];
  long    WCLimit;

  int     ProcCount;
  int     Memory = 0;

  char    JobType[MMAX_NAME];
 
  mnode_t *N;
 
  mpar_t  *P;

  mrm_t   *RM;

  const char *FName = "MAMAllocJReserve";
 
  MDB(3,fAM) MLog("%s(%s,RIndex,ErrMsg)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if (RIndex != NULL)
    *RIndex = mhrNONE;
 
  if (ErrMsg != NULL)
    ErrMsg[0] = '\0';

  if ((AM == NULL) || (J == NULL))
    {
    return(FAILURE);
    }
 
  if ((AM->Type == mamtNONE) || 
     ((MSched.Mode != msmNormal) && (MAMTest != TRUE)))
    {
    return(SUCCESS);
    }

  if (AM->State == mrmsDown)
    {
    return(FAILURE);
    }
 
  if (((J->Cred.A == NULL) || (!strcmp(J->Cred.A->Name,NONE))) &&
       (AM->Type != mamtQBANK) &&
       (AM->Type != mamtGOLD) &&    
       (AM->Type != mamtFILE))
    {
    MDB(2,fAM) MLog("ALERT:    no account specified for job %s\n",
      J->Name);

    if (RIndex != NULL) 
      *RIndex = mhrNoFunds;
 
    return(FAILURE);
    }

  RM = (J->RM != NULL) ? J->RM : &MRM[0];
 
  if ((J->Cred.A != NULL) && strcmp(J->Cred.A->Name,NONE))
    {
    strcpy(AccountName,J->Cred.A->Name);
 
    if (AM->AppendMachineName == TRUE)
      {
      strcat(AccountName,"@");
      strcat(AccountName,MRM[0].Name);  /* FIXME:  assumes single RM per scheduler */
      }
    }
  else
    {
    AccountName[0] = '\0';
    }
 
  N = J->Req[0]->NodeList[0].N;

  P = &MPar[0];

  if (N != NULL)
    {
    if (MUNLGetMinAVal(
          J->NodeList,
          mnaNodeType,
          NULL,
          (void **)NodeType) == FAILURE)
      { 
      if ((N->FrameIndex != -1) && 
          (MFrame[N->FrameIndex].NodeType[0] != '\0'))
        {
        strcpy(NodeType,MFrame[N->FrameIndex].NodeType);
        }
      else if (N->NodeType[0] != '\0')
        {
        strcpy(NodeType,N->NodeType);
        }
      else
        {
        strcpy(NodeType,MDEF_NODETYPE);
        }
      }
    }
  else
    {
    strcpy(NodeType,MDEF_NODETYPE);
    }
 
  /* determine QOS */
 
  if ((J->Cred.Q != NULL) && strcmp(J->Cred.Q->Name,DEFAULT))
    {
    sprintf(QOSString,"QOS=%s ",
      J->Cred.Q->Name);
    }
  else
    {
    QOSString[0] = '\0';
    }

  if ((J->Cred.Q != NULL) &&
      (J->Cred.Q->Flags & (1 << mqfdedicated)) &&
      (J->NodeList != NULL))
    {
    int nindex;

    ProcCount = 0;

    for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
      {
      ProcCount += J->NodeList[nindex].N->CRes.Procs;
      }  /* END for (nindex) */
    }
  else
    {
    ProcCount = MAX(1,MJobGetProcCount(J));
    }

  WCLimit = J->WCLimit;

  if (P->UseMachineSpeed == TRUE)
    {
    WCLimit = (long)((double)WCLimit / N->Speed);
    }
 
  switch (AM->Type)
    {
    case mamtFILE:
 
      fprintf(AM->FP,"%-15s TYPE=job ACCOUNT=%s SUBACCOUNT=%s RESOURCE=%d RESOURCETYPE=%s DURATION=%ld %sREQUESTID=%s NODES=%d\n",
        "RESERVE",
        AccountName,
        J->Cred.U->Name,
        ProcCount,
        NodeType,
        WCLimit,
        QOSString,
        J->Name,
        J->NodeCount);
 
      fflush(AM->FP);
 
      break;
 
    case mamtQBANK:
    
      sprintf(AMISBuffer,"COMMAND=make_reservation AUTH=%s MACHINE=%s%s%s USER=%s WCLIMIT=%ld PROCCOUNT=%d %sCLASS=%s NODETYPE=%s TYPE=%s JOBID=%s JOBTYPE=%s",
        MSched.Admin1User[0],
        RM->Name, 
        (AccountName[0] != '\0') ? " ACCOUNT=" : "",
        AccountName,
        J->Cred.U->Name,
        WCLimit,
        ProcCount,
        QOSString,
        (J->Cred.C != NULL) ? J->Cred.C->Name : DEFAULT,
        NodeType,
        AM->ClientName,
        J->Name,
        (TestAlloc == TRUE) ?  "tempjob" : "job");
 
      rc = MAMQBDoCommand(
        AM,
        0,
        AMISBuffer,
        NULL,
        &tmpSC,
        AMIRBuffer);
 
      if (rc == FAILURE)
        {
        MDB(1,fAM) MLog("ALERT:    cannot create AM reservation for job '%s' (AM failure)\n",
          J->Name);

        if (RIndex != NULL) 
          *RIndex = mhrAMFailure;
 
        if (ErrMsg != NULL)
          strcpy(ErrMsg,"AM transaction failed");

        if (AM->JFAction != mamjfaNONE) 
          {
          return(FAILURE);
          }
        else
          {
          return(SUCCESS);
          }
        }
 
      if (tmpSC == FAILURE)
        {
        MDB(1,fAM) MLog("ALERT:    cannot create AM reservation for job '%s' (request refused)\n",
          J->Name); 

        if (RIndex != NULL) 
          *RIndex = mhrNoFunds;
 
        if (ErrMsg != NULL)
          strcpy(ErrMsg,AMIRBuffer);
 
        return(FAILURE);
        }
 
      break;
 
    case mamtNONE:
 
      return(SUCCESS);

      /*NOTREACHED*/
 
      break;

    case mamtGOLD:

      {
      mxml_t *E  = NULL;
      mxml_t *RE = NULL;
      mxml_t *DE = NULL;
      mxml_t *JE = NULL;
      mxml_t *AE = NULL;

      MXMLCreateE(&E,"Message");

      /*
      MXMLSetAttr(E,"xmlns","http://www.scidac.org/ScalableSystems/AllocationManager",mdfString);
      */

      RE = NULL;

      MXMLCreateE(&RE,"Request");

      MXMLSetAttr(RE,MSAN[msanAction],(void *)"Reserve",mdfString);

      MS3SetObject(RE,MS3ObjName[AM->Version][mxoJob],NULL);

      DE = NULL;

      MXMLCreateE(&DE,MSON[msonData]);

      MXMLAddE(RE,DE);

      JE = NULL;

      MXMLCreateE(&JE,"Job");

      MXMLAddE(DE,JE);

      AE = NULL;
      MXMLCreateE(&AE,"JobId");
      MXMLSetVal(AE,(void *)J->Name,mdfString);
      MXMLAddE(JE,AE);

      if (J->Cred.U != NULL)
        {
        AE = NULL;
        MXMLCreateE(&AE,(char *)MS3JobAttr[AM->Version][mjaUser]);
        MXMLSetVal(AE,(void *)J->Cred.U->Name,mdfString);
        MXMLAddE(JE,AE);
        }

      if (J->Cred.A != NULL)
        {
        AE = NULL;
        MXMLCreateE(&AE,(char *)MS3JobAttr[AM->Version][mjaAccount]);
        MXMLSetVal(AE,(void *)J->Cred.A->Name,mdfString);
        MXMLAddE(JE,AE);
        }

      if (J->Cred.Q != NULL)
        {
        AE = NULL;
        MXMLCreateE(&AE,(char *)MS3JobAttr[AM->Version][mjaQOS]);
        MXMLSetVal(AE,(void *)J->Cred.Q->Name,mdfString);
        MXMLAddE(JE,AE);
        }

      if (RM != NULL)
        {
        AE = NULL;
        MXMLCreateE(&AE,"MachineName");
        MXMLSetVal(AE,(void *)RM->Name,mdfString);
        MXMLAddE(JE,AE);
        }

      if (ProcCount > 0)
        {
        AE = NULL;
        MXMLCreateE(&AE,(char *)MS3ReqAttr[AM->Version][mrqaTCReqMin]);
        MXMLSetVal(AE,(void *)&ProcCount,mdfInt);
        MXMLAddE(JE,AE);
        }

      if (Memory > 0)
        {
        AE = NULL;
        MXMLCreateE(&AE,"Memory");
        MXMLSetVal(AE,(void *)&Memory,mdfInt);
        MXMLAddE(JE,AE);
        }

      if (WCLimit > 0)
        {
        AE = NULL;
        MXMLCreateE(&AE,(char *)MS3JobAttr[AM->Version][mjaAWDuration]);
        MXMLSetVal(AE,(void *)&WCLimit,mdfLong);
        MXMLAddE(JE,AE);
        }

      if (J->Cred.C != NULL)
        {
        AE = NULL;
        MXMLCreateE(&AE,MS3ReqAttr[AM->Version][mrqaReqClass]);
        MXMLSetVal(AE,(void *)&J->Cred.C->Name,mdfString);
        MXMLAddE(JE,AE);
        }

      if (NodeType[0] != '\0')
        {
        AE = NULL;
        MXMLCreateE(&AE,"NodeType");
        MXMLSetVal(AE,(void *)NodeType,mdfString);
        MXMLAddE(JE,AE);
        }

      JobType[0] = '\0';  /* NOTE:  not enabled */

      if (JobType[0] != '\0')
        {
        AE = NULL;
        MXMLCreateE(&AE,"Type");
        MXMLSetVal(AE,(void *)JobType,mdfString);
        MXMLAddE(JE,AE);
        }

      /* submit request */

      if (MAMGoldDoCommand(RE,&AM->P,RIndex,ErrMsg) == FAILURE)
        {
        MDB(2,fAM) MLog("ALERT:    cannot reserve allocation for job\n");

        return(FAILURE);
        }

      /* reserve successful */

      MSysEMSubmit(
        &MSched.EM,
        (char *)MS3CName[mpstAM],
        "joballocreserve",
        J->Name);

      MXMLDestroyE(&E);
      }  /* END BLOCK */

      return(SUCCESS);
 
      /*NOTREACHED*/
 
      break;
       
    default:
 
      return(SUCCESS);

      /*NOTREACHED*/
 
      break;
    }  /* END switch (AM->Type) */
 
  return(SUCCESS);
  }  /* END MAMAllocJReserve() */





int MAMLoadConfig(
 
  char *AMName,  /* I (optional) */
  char *Buf)     /* I (optional) */ 

  {
  char   IndexName[MMAX_NAME];
 
  char   Value[MMAX_LINE];
 
  char  *ptr;
  char  *head;
 
  mam_t *A;
 
  /* FORMAT:  <KEY>=<VAL>[<WS><KEY>=<VAL>]...         */
  /*          <VAL> -> <ATTR>=<VAL>[:<ATTR>=<VAL>]... */
 
  /* load all/specified AM config info */

  head = (Buf != NULL) ? Buf : MSched.ConfigBuffer;
 
  if (head == NULL)
    {
    return(FAILURE);
    }
 
  if ((AMName == NULL) || (AMName[0] == '\0'))
    {
    /* load ALL AM config info */
 
    ptr = head;
 
    IndexName[0] = '\0';
 
    while (MCfgGetSVal(
             head,
             &ptr,
             MAMCfgParam,
             IndexName,
             NULL,
             Value,
             sizeof(Value),
             0,
             NULL) != FAILURE)
      {
      if (MAMFind(IndexName,&A) == FAILURE)
        {
        if (MAMAdd(IndexName,&A) == FAILURE) 
          {
          /* unable to locate/create AM */
 
          IndexName[0] = '\0';
 
          continue;
          }
 
        MAMSetDefaults(A);
        }
 
      /* load AM specific attributes */
 
      MAMProcessConfig(A,Value);

      IndexName[0] = '\0';
      }  /* END while (MCfgGetSVal() != FAILURE) */

    A = &MAM[0];  /* NOTE:  only supports single AM per scheduler */

    if (MAMCheckConfig(A) == FAILURE)
      {
      MAMDestroy(&A);

      /* invalid AM destroyed */

      return(FAILURE);
      }
    }    /* END if ((AMName == NULL) || (AMName[0] == '\0')) */
  else
    {
    /* load specified AM config info */
 
    ptr = MSched.ConfigBuffer;
 
    A = NULL;
 
    while (MCfgGetSVal(
             head,
             &ptr,
             MAMCfgParam,
             AMName,
             NULL,
             Value,
             sizeof(Value),
             0,
             NULL) == SUCCESS)
      {
      if ((A == NULL) &&
          (MAMFind(AMName,&A) == FAILURE))
        {
        if (MAMAdd(AMName,&A) == FAILURE)
          {
          /* unable to add AM */ 
 
          return(FAILURE);
          }
 
        MAMSetDefaults(A);
        }
 
      /* load AM attributes */
 
      MAMProcessConfig(A,Value);
      }  /* END while (MCfgGetSVal() == SUCCESS) */
 
    if (A == NULL)
      {
      return(FAILURE);
      }
 
    if (MAMCheckConfig(A) == FAILURE)
      {
      MAMDestroy(&A);
 
      /* invalid AM destroyed */
 
      return(FAILURE);
      }
    }  /* END else ((AMName == NULL) || (AMName[0] == '\0')) */
 
  return(SUCCESS);
  }  /* END MAMLoadConfig() */
 
 
 
 
int MAMAdd(
 
  char   *AMName,  /* I */
  mam_t **APtr)    /* O */
 
  {
  int     amindex;
 
  mam_t *A; 

  const char *FName = "MAMAdd";
 
  MDB(6,fSTRUCT) MLog("%s(%s,%s)\n",
    FName,
    (AMName != NULL) ? "AMName" : "NULL",
    (APtr != NULL) ? "APtr" : "NULL");
 
  if ((AMName == NULL) || (AMName[0] == '\0'))
    {
    return(FAILURE);
    }
 
  if (APtr != NULL)
    *APtr = NULL;
 
  for (amindex = 0;amindex < MMAX_AM;amindex++)
    {
    A = &MAM[amindex];
 
    if ((A != NULL) && !strcmp(A->Name,AMName))
      {
      /* AM already exists */
 
      if (APtr != NULL)
        *APtr = A;
 
      return(SUCCESS);
      }
 
    if ((A != NULL) &&
        (A->Name[0] != '\0') &&
        (A->Name[0] != '\1'))
      {
      continue;
      }
 
    /* empty slot found */
 
    /* create/initialize new record */
 
    if (A == NULL)
      {
      A = &MAM[amindex];

      if (MAMCreate(AMName,&A) == FAILURE)
        {
        MDB(1,fALL) MLog("ERROR:    cannot alloc memory for AM '%s'\n",
          AMName);
 
        return(FAILURE); 
        }
      }
    else if (A->Name[0] == '\0')
      {
      MUStrCpy(A->Name,AMName,sizeof(A->Name));
      }

    MAMSetDefaults(A);

    MOLoadPvtConfig((void **)A,mxoAM,A->Name,NULL,NULL);

    if (APtr != NULL)
      *APtr = A;
 
    A->Index = amindex;
 
    /* update AM record */
 
    if (MSched.Mode != msmSim)
      MCPRestore(mcpAM,AMName,(void *)A);
 
    MDB(5,fSTRUCT) MLog("INFO:     AM %s added\n",
      AMName);
 
    return(SUCCESS);
    }    /* END for (amindex) */
 
  /* end of table reached */
 
  MDB(1,fSTRUCT) MLog("ALERT:    AM table overflow.  cannot add %s\n",
    AMName);
 
  return(SUCCESS);
  }  /* END MAMAdd() */
 
 
 
 
int MAMCreate(
 
  char   *AMName,  /* I */
  mam_t **AP)      /* O */
 
  {
  mam_t *A;

  if (AP == NULL)
    {
    return(FAILURE);
    }

  A = *AP;
 
  /* use static memory for now */ 
 
  memset(A,0,sizeof(mam_t));
 
  if ((AMName != NULL) && (AMName[0] != '\0'))
    MUStrCpy(A->Name,AMName,sizeof(A->Name));
 
  return(SUCCESS);
  }  /* END MAMCreate() */
 
 
 
 
int MAMFind(
 
  char   *AMName,  /* I */
  mam_t **AP)      /* O */
 
  {
  /* if found, return success with AP pointing to AM */
 
  int    amindex;
  mam_t *A;
 
  if (AP != NULL)
    *AP = NULL;
 
  if ((AMName == NULL) ||
      (AMName[0] == '\0'))
    {
    return(FAILURE);
    }
 
  for (amindex = 0;amindex < MMAX_AM;amindex++)
    {
    A = &MAM[amindex];
 
    if ((A == NULL) || (A->Name[0] == '\0') || (A->Name[0] == '\1'))
      {
      break;
      }
 
    if (strcmp(A->Name,AMName) != 0)
      continue; 
 
    /* AM found */
 
    if (AP != NULL)
      *AP = A;
 
    return(SUCCESS);
    }  /* END for (amindex) */
 
  /* entire table searched */
 
  return(FAILURE);
  }  /* END MAMFind() */
 
 
 
 
int MAMSetDefaults(
 
  mam_t *A)  /* I (modified) */
 
  {
  const char *FName = "MAMSetDefaults";

  MDB(1,fSTRUCT) MLog("%s(%s)\n",
    FName,
    (A != NULL) ? A->Name : "NULL");

  if (A == NULL)
    {
    return(FAILURE);
    }

  switch (A->Type)
    {
    case mamtNONE:

      /* set general defaults */

      A->Port              = MDEF_AMPORT;   
      strcpy(A->Host,MDEF_AMHOST); 
 
      A->Type              = MDEF_AMTYPE;
      A->JFAction          = MDEF_AMJFACTION;   
      A->FlushTime         = 0;
      A->FlushInterval     = MDEF_AMFLUSHINTERVAL;  
      A->Timeout           = MDEF_AMTIMEOUT;
      A->Version           = MDEF_AMVERSION; 
 
      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case mamtGOLD:

      if (A->WireProtocol == mwpNONE)
        A->WireProtocol = mwpXML;

      if (A->SocketProtocol == mspNONE)
        A->SocketProtocol = mspHalfSocket;

      if (A->Version == 0)
        A->Version = msssV3_0;

      break;

    case mamtQBANK:

      if (A->WireProtocol == mwpNONE)
        A->WireProtocol = mwpAVP;

      if (A->SocketProtocol == mspNONE)
        A->SocketProtocol = mspSingleUseTCP;

      if (A->CSAlgo == mcsaNONE)
        A->CSAlgo = mcsaHMAC;

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch (A->Type) */
 
  return(SUCCESS);
  }  /* END MAMSetDefaults() */
 
 
 
 
int MAMProcessConfig(
 
  mam_t *A,     /* I (modified) */
  char  *Value) /* I */
 
  {
  int   aindex;
 
  char *ptr;
  char *TokPtr;
 
  char  ValLine[MMAX_LINE];
  char *ValList[2];
 
  if ((A == NULL) ||
      (Value == NULL) ||
      (Value[0] == '\0'))
    {
    return(FAILURE);
    }
 
  /* process value line */
 
  ptr = MUStrTok(Value," \t\n",&TokPtr);
 
  while(ptr != NULL)
    {
    /* parse name-value pairs */
 
    /* FOAMAT:  <VALUE>[,<VALUE>] */
 
    if (MUGetPair(
          ptr, 
          (const char **)MAMAttr,
          &aindex,
	  NULL,
          TRUE,
          NULL,
          ValLine,
          MMAX_NAME) == FAILURE)
      {
      /* cannot parse value pair */
 
      ptr = MUStrTok(NULL," \t\n",&TokPtr);
 
      continue;
      }
 
    ValList[0] = ValLine;
    ValList[1] = NULL;
 
    switch(aindex)
      {
      case mamaAppendMachineName:

        A->AppendMachineName = MUBoolFromString(ValLine,FALSE);

        break;

      case mamaChargePolicy:

        A->ChargePolicy = 
          MUGetIndex(ValLine,(const char **)MAMChargePolicy,FALSE,MDEF_AMCHARGEPOLICY);

        break;

      case mamaJFAction:

        A->JFAction = MUGetIndex(ValLine,MJFActionType,TRUE,mamjfaNONE);

        break;

      case mamaFallbackAccount:

        MUStrCpy(A->FallbackAccount,ValLine,sizeof(A->FallbackAccount));

        break;

      case mamaFlushInterval:

        /* make interval evenly divisible by MMAX_AMFLUSHINTERVAL */

        {
        long tmpL;

        tmpL = (long)MUTimeFromString(ValLine);

        if (tmpL <= 0)
          {
          A->FlushInterval = MDEF_AMFLUSHINTERVAL;
          }
        else if (tmpL >= MMAX_AMFLUSHINTERVAL)
          {
          A->FlushInterval = MMAX_AMFLUSHINTERVAL;
          }
        else
          {
          tmpL = tmpL % MMAX_AMFLUSHINTERVAL;

          tmpL = MMAX_AMFLUSHINTERVAL / tmpL;

          A->FlushInterval = MMAX_AMFLUSHINTERVAL / tmpL;
          }
        }    /* END BLOCK */

        break;

      case mamaHost:

        {
        char *ptr;
        char *TokPtr;

        /* FORMAT:  [<HOSTNAME>][*] */
 
        ptr = MUStrTok(ValLine,"*",&TokPtr);

        A->UseDirectoryService = FALSE;

        if (ptr == NULL)
          break;

        if (ptr[0] == '*')
          {
          A->UseDirectoryService = TRUE;
          }
        else
          {     
          MUStrCpy(A->Host,ptr,sizeof(A->Host));
          MUStrCpy(A->SpecHost,ptr,sizeof(A->SpecHost));
          }

        ptr = MUStrTok(NULL,"*",&TokPtr);

        if (ptr == NULL)
          break;

        if (ptr[0] == '*')
          {
          A->UseDirectoryService = TRUE;
          }
        else
          {
          MUStrCpy(A->Host,ptr,sizeof(A->Host));
          MUStrCpy(A->SpecHost,ptr,sizeof(A->SpecHost));
          }
        }  /* END BLOCK */

        break;

      case mamaPort:

        A->Port     = (int)strtol(ValLine,NULL,0);
        A->SpecPort = A->Port;

        break;

      case mamaServer:

        {
        char tmpHost[MMAX_LINE];
        char tmpProtocol[MMAX_LINE];

        if (!strcmp(ValLine,"ANY"))
          {
          A->UseDirectoryService = TRUE;
          
          break;
          }

        if (MUURLParse(
              ValLine,
              tmpProtocol,
              tmpHost,
              NULL,
              0,
              &A->Port,
              FALSE) == FAILURE)
          {
          /* cannot parse string */

          break;
          }

        MUStrToUpper(tmpProtocol,NULL,0);

        A->Type = MUGetIndex(tmpProtocol,MAMType,FALSE,0);

        A->SpecPort = A->Port;

        A->UseDirectoryService = FALSE;

        MUStrCpy(A->Host,tmpHost,sizeof(A->Host));
        MUStrCpy(A->SpecHost,tmpHost,sizeof(A->SpecHost));
        }  /* END BLOCK */
 
        break;

      case mamaSocketProtocol:

        A->SocketProtocol = MUGetIndex(ValLine,MSockProtocol,FALSE,0);

        /* NOTE:  if 'HTTP' specified, use 'HTTPClient' */

        break;

      case mamaTimeout:

        A->Timeout = MUTimeFromString(ValLine);

        if (A->Timeout > 1000)
          A->Timeout /= 1000;

        break;

      case mamaType:

        A->Type = MUGetIndex(ValLine,MAMType,FALSE,0);

        break;

      case mamaWireProtocol:

        A->WireProtocol = MUGetIndex(ValLine,MWireProtocol,FALSE,0);

        break;

      default:
 
        MDB(4,fAM) MLog("WARNING:  AM attribute '%s' not handled\n",
          MAMAttr[aindex]);
 
        break;
      }  /* END switch(aindex) */
 
    ptr = MUStrTok(NULL," \t\n",&TokPtr);
    }  /* END while (ptr != NULL) */
 
  return(SUCCESS);
  }  /* END MAMProcessConfig() */
 
 
 
 
int MAMDestroy(
 
  mam_t **A)  /* I (modified) */
 
  {
  if (A == NULL)
    {
    return(SUCCESS);
    }
 
  if (*A == NULL)
    {
    return(SUCCESS);
    }
 
  memset(*A,0,sizeof(mam_t));
 
  return(SUCCESS);
  }  /* END MAMDestroy() */ 
 
 
 
 
int MAMCheckConfig(
 
  mam_t *A)  /* I */
 
  {
  if (A == NULL)
    {
    return(FAILURE);
    }

  MAMSetDefaults(A);
 
  /* NYI */
 
  return(SUCCESS);
  }  /* END MAMCheckConfig() */
 
 
 
 
int MAMShow(
 
  mam_t *APtr,    /* I */
  char  *Buffer,  /* O */
  int    BufSize, /* I */
  int    Mode)    /* I */
 
  {
  int   amindex;

  int   index;
  int   findex;

  mbool_t FailureRecorded;

  char *BPtr;
  int   BSpace;

  mbool_t ShowHeader = TRUE;

  char tmpLine[MMAX_LINE];
 
  mam_t *A;
 
  if (Buffer == NULL)
    {
    return(FAILURE);
    }

  BPtr   = Buffer;
  BSpace = BufSize;

  BPtr[0] = '\0';
 
  /* NOTE:  allow mode to specify verbose, diagnostics, etc */
 
  for (amindex = 0;MAM[amindex].Type != mamtNONE;amindex++)
    {
    A = &MAM[amindex];
 
    if ((APtr != NULL) && (APtr != A))
      continue; 
 
    if (ShowHeader == TRUE)
      {
      ShowHeader = FALSE;
      }

    tmpLine[0] = '\0';

    if (A->Type == mamtNONE)
      continue;

    MUSNPrintF(&BPtr,&BSpace,"AM[%s]  type: '%s'  state: '%s'\n",
      A->Name,
      MAMType[A->Type],
      MAMState[A->State]);

    /* NOTE:  display optional RM version, failures, and fault tolerance config */

    if (A->Version > 0)
      {
      MUSNPrintF(&BPtr,&BSpace,"  Version: '%d'\n",
        A->Version);
      }

    /* NOTE:  stats in ms */

    if (A->RespTotalCount[0] > 0)
      {
      MUSNPrintF(&BPtr,&BSpace,"  AM Performance:  Avg Time: %.2lfs  Max Time:  %.2lfs  (%d samples)\n",
        (double)A->RespTotalTime[0] / A->RespTotalCount[0] / 1000,
        (double)A->RespMaxTime[0] / 1000,
        A->RespTotalCount[0]);
      }

    FailureRecorded = FALSE;

    for (index = 0;index < MMAX_RMFAILURE;index++)
      {
      findex = (index + A->FailIndex) % MMAX_RMFAILURE;

      if (A->FailTime[findex] <= 0)
        continue;

      if (FailureRecorded == FALSE)
        {
        MUSNPrintF(&BPtr,&BSpace,"\nAM[%s] Failures: \n",
          A->Name);

        FailureRecorded = TRUE;
        }

      MUSNPrintF(&BPtr,&BSpace,"  %19.19s  %-15s  '%s'\n",
        MULToDString((mulong *)&A->FailTime[findex]),
        MAMFuncType[A->FailType[findex]],
        (A->FailMsg[findex] != NULL) ? A->FailMsg[findex] : "no msg");
      }  /* END for (index) */

    MUSNPrintF(&BPtr,&BSpace,"\n");
    }  /* END for (amindex) */
 
  return(SUCCESS);
  }  /* END MAMShow() */




int MAMConfigShow(
 
  mam_t *APtr,    /* I */
  int    Mode,    /* I */
  char  *Buffer)  /* O */
 
  {
  int  amindex;
  int  aindex;

  char tmpLine[MMAX_LINE];
  char tmpVal[MMAX_LINE];
 
  mam_t *A;
 
  if (Buffer == NULL)
    {
    return(FAILURE);
    }
 
  /* NOTE:  allow mode to specify verbose, etc */
 
  for (amindex = 0;MAM[amindex].Type != mamtNONE;amindex++)
    {
    A = &MAM[amindex];
 
    if ((APtr != NULL) && (APtr != A))
      continue;
 
    tmpLine[0] = '\0';
 
    if (A->Type == mamtNONE)
      continue;

    sprintf(tmpLine,"AMCFG[%s]",
      A->Name);

    for (aindex = 0;MAMAttr[aindex] != NULL;aindex++)
      {
      tmpVal[0] = '\0';

      switch (aindex)
        {
        case mamaAppendMachineName:

          if (A->AppendMachineName == TRUE)
            strcpy(tmpVal,"TRUE");

          break;

        case mamaChargePolicy:

          if (A->ChargePolicy != 0)
            strcpy(tmpVal,MAMChargePolicy[A->ChargePolicy]);

          break;

        case mamaFallbackAccount:

          if (A->FallbackAccount[0] != '\0')
            strcpy(tmpVal,A->FallbackAccount);

          break;

        case mamaFlushInterval:

          if (A->FlushInterval != 0)
            strcpy(tmpVal,MULToTString(A->FlushInterval));

          break;

        case mamaHost:

          if (A->Host[0] != '\0')
            strcpy(tmpVal,A->Host);

          break;

        case mamaJFAction:

          if (A->JFAction != mamjfaNONE)
            strcpy(tmpVal,MJFActionType[A->JFAction]);

          break;

        case mamaPort:

          if (A->Port != 0)
            sprintf(tmpVal,"%d",A->Port);

          break;

        case mamaTimeout:

          if (A->Timeout != 0)
            strcpy(tmpVal,MULToTString(A->Timeout));

          break;
 
        case mamaType:

          if (A->Type != mamtNONE)
            strcpy(tmpVal,MAMType[A->Type]);

          break;

        default:

          /* not handled */

          break;
        }  /* END switch (aindex) */

      if (tmpVal[0] != '\0')
        {
        sprintf(tmpLine,"%s %s=%s",
          tmpLine,
          MAMAttr[aindex],
          tmpVal);
        }
      }    /* END for (aindex) */

    strcat(Buffer,tmpLine);
    strcat(Buffer,"\n");
    }  /* END for (amindex) */
 
  return(SUCCESS);
  }  /* END MAMConfigShow() */





int MAMActivate(
 
  mam_t *A)  /* I */
 
  {
  char FileName[MMAX_PATH_LEN];
  char PathName[MMAX_PATH_LEN];

  const char *FName = "MAMActivate";
 
  MDB(2,fAM) MLog("%s(%s)\n",
    FName,
    (A != NULL) ? A->Name : "NULL");
 
  if ((A == NULL) || (A->Type == mamtNONE))
    {
    return(SUCCESS);
    }

  MS3Setup(0);
 
  if ((MSched.Mode != msmNormal) && (MAMTest != TRUE))
    {
    return(SUCCESS);
    }
 
  switch (A->Type)
    {
    case mamtQBANK:
 
      /* QBANK requires connection per request */

      strcpy(A->ClientName,MAM_CLIENTTYPE);
 
      break;

    case mamtGOLD:

      /* NO-OP */

      break;
 
    case mamtFILE:
 
      /* open transaction log file */ 
 
      if (A->Host[0] == '\0')
        {
        strcpy(FileName,"am.log");
        }
      else
        {
        strcpy(FileName,A->Host);
        }
 
      if ((FileName[0] != '/') && (FileName[0] != '~'))
        {
        if (MSched.HomeDir[strlen(MSched.HomeDir) - 1] == '/')
          {
          sprintf(PathName,"%s%s",
            MSched.HomeDir,
            FileName);
          }
        else
          {
          sprintf(PathName,"%s/%s",
            MSched.HomeDir,
            FileName);
          }
        }
      else
        {
        strcpy(PathName,FileName);
        }
 
      if ((A->FP = fopen(PathName,"a+")) == NULL)
        {
        MDB(0,fAM) MLog("WARNING:  cannot open AM file '%s', errno: %d (%s)\n",
          PathName,
          errno,
          strerror(errno));
 
        /* dump AM records to logfile */
 
        A->FP = mlog.logfp;
 
        return(FAILURE);
        }
      else
        { 
        fprintf(A->FP,"AMSTART\n");
 
        fflush(A->FP);
        }
 
      MDB(3,fAM) MLog("INFO:     AM type '%s' initialized (file '%s' opened)\n",
        MAMType[mamtFILE],
        PathName);
 
      break;
 
    case mamtNONE:
 
      return(SUCCESS);
 
      /*NOTREACHED*/
 
      break;
 
    default:
 
      return(SUCCESS);
 
      /*NOTREACHED*/
 
      break;
    }  /* END switch (A->Type) */

  /* check fallback account */
 
  if (A->FallbackAccount[0] != '\0')
    {
    /* initialize fallback account */
 
    if (MAcctAdd(A->FallbackAccount,NULL) == FAILURE)
      {
      MDB(1,fAM) MLog("ALERT:    cannot add fallback account '%s'\n",
        A->FallbackAccount);
 
      return(FAILURE);
      }
    }
 
  return(SUCCESS);
  }  /* END MAMActivate() */




int MAMAccountGetDefault(
 
  char *UName,    /* I */
  char *AName,    /* O */
  enum MHoldReasonEnum *RIndex) /* O */
 
  {
  char Line[MMAX_LINE << 2];
  char Response[MMAX_BUFFER << 2];
 
  int  rc;
  int  SC;

  char *ptr;
  char *TokPtr;

  int   uindex;

  mam_t *A;

  static struct {
    char UName[MMAX_NAME];
    char AName[MMAX_NAME];
    } DACache[MMAX_USER];
 
  static mbool_t CacheLoaded = FALSE;

  const char *FName = "MAMAccountGetDefault";

  MDB(3,fAM) MLog("%s(%s,AName,RIndex)\n",
    FName,
    (UName != NULL) ? UName : "NULL");

  if (RIndex != NULL)
    *RIndex = mhrNONE;

  if (AName != NULL)
    AName[0] = '\0';

  if ((UName == NULL) || (AName == NULL))
    {
    return(FAILURE);
    }

  if ((MSched.Mode != msmNormal) && (MAMTest != TRUE))
    {
    return(SUCCESS);
    }

  A = &MAM[0];

  if (A->Type == mamtNONE)
    {
    return(FAILURE);
    }

  if (A->State == mrmsDown)
    {
    return(FAILURE);
    }

  /* check if default account cached */

  if (CacheLoaded == FALSE)
    {
    /* load cache */

    uindex = 0;

    switch (A->Type)
      {
      case mamtQBANK:

        {
        char FS[MMAX_NAME];
        char LS[MMAX_NAME];

        switch (A->Version)
          {
          default:

            strcpy(FS,",|");
            strcpy(LS,"\n");

            break;
          }  /* END switch(A->Version) */

        sprintf(Line,"COMMAND=get_users SHOW=username,defaultaccount AUTH=%s",
          MSched.Admin1User[0]);
 
        if ((MAMQBDoCommand(A,0,Line,NULL,&SC,Response) == SUCCESS) &&
            (SC == SUCCESS))
          {
          /* FORMAT:  {<UNAME>|<ANAME>\n}... */

          /* or */

          /* FORMAT:  {<UNAME>,<ANAME>\n}... */

          ptr = MUStrTok(Response,LS,&TokPtr);

          while (ptr != NULL)
            {
            char *TokPtr2;

            ptr = MUStrTok(ptr,FS,&TokPtr2);

            if (ptr[0] == '\0')
              {
              ptr = MUStrTok(NULL,LS,&TokPtr);

              continue;
              }

            MUStrCpy(DACache[uindex].UName,ptr,sizeof(DACache[uindex].UName));

            if ((ptr = MUStrTok(NULL,FS,&TokPtr2)) != NULL)
              {
              MUStrCpy(DACache[uindex].AName,ptr,sizeof(DACache[uindex].AName));

              uindex++;
              }

            ptr = MUStrTok(NULL,LS,&TokPtr);
            }    /* END while (ptr != NULL) */

          CacheLoaded = TRUE;
          }  /* END if ((MAMQBDoCommand() == SUCCESS) && ...) */
        else
          {
          MDB(1,fAM) MLog("ALERT:    cannot obtain default account list (query failed)\n");
          }
        }    /* END BLOCK */
 
        break;

      case mamtGOLD:

        {
        char *RspBuf = NULL;
        int   rc;

        mxml_t *RE;

        int     CTok;

        mxml_t *DE;
        mxml_t *UE;
        mxml_t *NE;
        mxml_t *AE;
   
        /* create request string, populate S->SE */

        RE = NULL;

        MXMLCreateE(&RE,MSON[msonRequest]);

        MXMLSetAttr(RE,(char *)MSAN[msanAction],(void *)"Query",mdfString);

        MS3SetObject(RE,"User",NULL);

        MS3AddWhere(RE,"Special","False",NULL);

        MS3AddGet(RE,"Name",NULL);
        MS3AddGet(RE,"DefaultProject",NULL);
  
        /* attach XML request to socket */

        if (A->P.S == NULL)
          { 
          return(FAILURE);
          }

        A->P.S->SDE = RE;
 
        rc = MS3DoCommand(&A->P,NULL,&RspBuf,NULL,NULL,NULL);

        MDB(3,fAM) MLog("INFO:     account query response '%s'\n",
          (RspBuf != NULL) ? RspBuf : "NULL");

        /* process response */
      
        DE = (mxml_t *)A->P.S->RDE;

        if (DE == NULL)
          {
          /* data not available */

          MUFree(&RspBuf);

          return(FAILURE);
          }

        /* FORMAT:  <Data><User><Name>X</Name><DefaultProject>Y</DefaultProject></User></Data> */

        CTok = -1;

        while (MXMLGetChild(DE,"User",&CTok,&UE) == SUCCESS)
          {
          if (MXMLGetChild(UE,"Name",NULL,&NE) == FAILURE)
            continue;

          if ((NE->Val == NULL) || (NE->Val[0] == '\0'))
            continue;

          if (!strcmp(NE->Val,"$ANY") ||
              !strcmp(NE->Val,"$NONE"),
              !strcmp(NE->Val,"$MEMBER"),
              !strcmp(NE->Val,"$DEFINED"),
              !strcmp(NE->Val,"$SPECIFIED"))
            {
            continue;
            }

          if (MXMLGetChild(UE,"DefaultProject",NULL,&AE) == FAILURE)
            continue;

          if ((AE->Val == NULL) || (AE->Val[0] == '\0'))
            continue;

          /* populate cache */

          MUStrCpy(DACache[uindex].UName,NE->Val,sizeof(DACache[uindex].UName));

          MUStrCpy(DACache[uindex].AName,AE->Val,sizeof(DACache[uindex].AName));

          uindex++;
          }    /* END while (MXMLGetChild(DE,"User",&CTok,&UE) == SUCCESS) */

        CacheLoaded = TRUE;

        MUFree(&RspBuf);
        }  /* END BLOCK */

        break;

      default:

        /* no am type loaded */

        return(FAILURE);

        break;
      }  /* END switch (A->Type) */

    /* terminate cache */

    DACache[uindex].UName[0] = '\0';
    DACache[uindex].AName[0] = '\0';
    }    /* END if (CacheLoaded == FALSE) */

  if (UName == NULL)
    {
    return(SUCCESS);
    }

  /* search cache */

  for (uindex = 0;uindex < MMAX_USER;uindex++)
    { 
    if (DACache[uindex].UName[0] == '\0')
      break;

    if (strcmp(DACache[uindex].UName,UName))
      continue;
 
    /* match located */

    if (AName != NULL)
      strcpy(AName,DACache[uindex].AName);

    MDB(3,fAM) MLog("INFO:     default account '%s' located for user '%s'\n",
      AName,
      UName);

    return(SUCCESS);
    }  /* END for (uindex) */

  /* NOTE: cache SHOULD contain all user/account mappings */

  if (A->FallbackAccount[0] == '\0')
    {
    return(FAILURE);
    }

  strcpy(AName,A->FallbackAccount);

   MDB(3,fAM) MLog("INFO:     default account '%s' located for user '%s'\n",
    AName,
    UName);

  return(SUCCESS);

  /*NOTREACHED*/
 
  switch (A->Type)
    {
    case mamtFILE:

      if (RIndex != NULL) 
        *RIndex = mhrAMFailure;
 
      return(FAILURE); 
 
      /*NOTREACHED*/
 
      break;
 
    case mamtQBANK:
 
      sprintf(Line,"COMMAND=get_users USER=%s SHOW=defaultaccount AUTH=%s",
        MSched.Admin1User[0],
        UName);
 
      rc = MAMQBDoCommand(
        A,
        0,
        Line,
        NULL,
        &SC,
        Response);
 
      if (rc == FAILURE)
        {
        MDB(1,fAM) MLog("ALERT:    cannot determine default account for user '%s' (AM failure)\n",
          UName);

        if (RIndex != NULL) 
          *RIndex = mhrAMFailure;
 
        return(FAILURE);
        }
 
      if (SC == FAILURE)
        {
        MDB(1,fAM) MLog("ALERT:    cannot determine default account for user '%s' (query failed)\n",
          UName);

        if (RIndex != NULL) 
          *RIndex = mhrAMFailure;
 
        return(FAILURE);
        }

      if (AName != NULL) 
        MUStrCpy(AName,Response,MMAX_NAME);
 
      break; 
 
    case mamtNONE:
 
      return(SUCCESS);
 
      /*NOTREACHED*/
 
      break;
 
    default:
 
      return(SUCCESS);
 
      /*NOTREACHED*/
 
      break;
    }  /* END switch(A->Type) */
 
  return(SUCCESS);
  }  /* END MAMAccountGetDefault() */




int MAMAccountVerify(
 
  char *AName,
  char *UName)
 
  {
  const char *FName = "MAMAccountVerify";

  MDB(2,fAM) MLog("%s(%s,%s)\n",
    FName,
    (AName != NULL) ? AName : "NULL",
    (UName != NULL) ? UName : "NULL");

  /* NYI */
 
  return(SUCCESS);
  }  /* END MAMAccountVerify() */





int MAMQBDoCommand(
 
  mam_t    *A,           /* I */
  int       CmdIndex,    /* I */
  char     *Transaction, /* I */
  void    **E,           /* I (optional) */
  int      *SC,          /* O */
  char     *Response)    /* O (optional) */
 
  {
  msocket_t S;
 
  char *ptr;
 
  char  SBuffer[MMAX_LINE << 2];
  char  Checksum[MMAX_NAME << 2];
  char  Message[MMAX_LINE << 2];
 
  char  tmpHeader[MMAX_NAME];

  const char *FName = "MAMQBDoCommand";
 
  MDB(3,fAM) MLog("%s(%s,%d,%s,E,SC,Response)\n",
    FName,
    (A != NULL) ? A->Name : "NULL",
    CmdIndex,
    Transaction);
 
  if ((A == NULL) ||
     ((A->Type != mamtQBANK) && (A->Type != mamtGOLD)))
    {
    return(FAILURE);
    }
 
  Response[0] = '\0';
  Checksum[0] = '\0';

  __MAMStartFunc(A,0);
 
  MSUInitialize(&S,A->Host,A->Port,0,(1 << msftTCP));

  S.SBuffer = SBuffer;         
  
  if (A->SocketProtocol != 0)
    S.SocketProtocol = A->SocketProtocol;

  if (A->WireProtocol != 0)
    S.WireProtocol   = A->WireProtocol;

  if (E != NULL)
    {
    /* populate E */

    S.SE = (void *)(*(mxml_t **)E);

    E = NULL;
    }
 
  switch(A->WireProtocol)
    {
    case mwpXML:

      strcpy(SBuffer,Transaction);

      break;

    default:
 
      /* build header */ 
 
      sprintf(tmpHeader,"COMMAND=%s AUTH=%s ",
        MSched.Admin1User[0],
        MAMCommands[CmdIndex]);

      MSecGetChecksum(
        Transaction,  
        strlen(Transaction),
        Checksum,
        NULL,
        (A->CSAlgo != 0) ? A->CSAlgo : MSched.DefaultCSAlgo,
        (A->CSKey[0] != '\0') ? A->CSKey : MSched.DefaultCSKey);
 
      sprintf(SBuffer,"%s CHECKSUM=%s\n",
        Transaction,
        Checksum);
 
      break;
    }  /* END switch(A->WireProtocol) */

  S.SBufSize = (long)strlen(SBuffer);         

  switch(A->SocketProtocol)
    {
    case mspHTTP:
    case mspHTTPClient:
    case mspHalfSocket:

      /* connect to server */

      if (MSUConnect(&S,FALSE,NULL) == FAILURE)
        {
        MDB(0,fAM) MLog("ERROR:    cannot connect to %s AM server '%s':%d\n",
          MAMType[A->Type],
          A->Host,
          A->Port);
 
        sprintf(Message,"AMFAILURE:  cannot connect to %s AM server %s:%d (transaction: '%s')\n",
          MAMType[A->Type],
          A->Host,
          A->Port,
          Transaction);
 
        MSysRegEvent(Message,0,0,1);

        __MAMEndFunc(A,0);
 
        return(FAILURE);
        }
 
      /* send data */

      if (MSUSendData(&S,A->Timeout * 1000000,FALSE,FALSE) == FAILURE)
        {
        MDB(0,fAM) MLog("ERROR:    cannot send data to %s AM server '%s':%d\n",
          MAMType[A->Type],
          A->Host,
          A->Port);
 
        sprintf(Message,"AMFAILURE:  cannot send data to %s AM server %s:%d (transaction: '%s')\n",
          MAMType[A->Type],
          A->Host,
          A->Port,
          Transaction);
 
        MSysRegEvent(Message,0,0,1);

        MSUFree(&S);

        __MAMEndFunc(A,0);
 
        return(FAILURE);
        }

      MDB(1,fAM) MLog("INFO:     transaction sent to AM\n");
 
      MDB(3,fAM) MLog("INFO:     message sent: '%s'\n",
        Transaction);

      /* receive data */

      if (MSURecvData(&S,A->Timeout * 1000000,FALSE,NULL,NULL) == FAILURE)
        {
        MDB(0,fAM) MLog("ERROR:    cannot receive data from %s AM server '%s':%d\n",
          MAMType[A->Type],
          A->Host,
          A->Port);
 
        sprintf(Message,"AMFAILURE:  cannot receive data from %s AM server %s:%d (transaction: '%s')\n",
          MAMType[A->Type],
          A->Host,
          A->Port,
          Transaction);
 
        MSysRegEvent(Message,0,0,1);
 
        MSUFree(&S);

        __MAMEndFunc(A,0);
 
        return(FAILURE);
        }
 
      MDB(4,fAM) MLog("INFO:     received message '%s' from server\n",
        S.RBuffer);
 
      MSUDisconnect(&S);
 
      break;
 
    default:

      if (Checksum[0] == '\0')
	{ 
        MSecGetChecksum(
          Transaction,
          strlen(Transaction),
          Checksum,
          NULL,
          (A->CSAlgo != 0) ? A->CSAlgo : MSched.DefaultCSAlgo,
	  (A->CSKey[0] != '\0') ? A->CSKey : MSched.DefaultCSKey);
        }
 
      sprintf(SBuffer,"%s CHECKSUM=%s\n",
        Transaction,
        Checksum);
 
      S.SBufSize = (long)strlen(SBuffer);

      if (MSUConnect(&S,FALSE,NULL) == FAILURE)
        {
        MDB(0,fAM) MLog("ERROR:    cannot connect to %s AM server '%s':%d\n",
          MAMType[A->Type],
          A->Host,
          A->Port);
 
        sprintf(Message,"AMFAILURE:  cannot connect to %s AM server %s:%d (transaction: '%s')\n",
          MAMType[A->Type],
          A->Host,
          A->Port, 
          Transaction);
 
        MSysRegEvent(Message,0,0,1);

        __MAMEndFunc(A,0);

        A->State = mrmsDown;
 
        return(FAILURE);
        }
 
      if (MSUSendData(&S,A->Timeout * 1000000,FALSE,FALSE) == FAILURE)
        {
        MDB(0,fAM) MLog("ERROR:    cannot send data to %s AM server '%s':%d\n",
          MAMType[A->Type],
          A->Host,
          A->Port);
 
        sprintf(Message,"AMFAILURE:  cannot send data to %s AM server %s:%d (transaction: '%s')\n",
          MAMType[A->Type],
          A->Host,
          A->Port,
          Transaction);
 
        MSysRegEvent(Message,0,0,1);
 
        MSUFree(&S);

        __MAMEndFunc(A,0);
 
        return(FAILURE);
        }
      else
        {
        MDB(1,fAM) MLog("INFO:     transaction sent to AM\n");
 
        MDB(3,fAM) MLog("INFO:     message sent: '%s'\n",
          Transaction);
        }
 
      if (MSURecvData(&S,A->Timeout * 1000000,FALSE,NULL,NULL) == FAILURE)
        {
        MDB(0,fAM) MLog("ERROR:    cannot receive data from %s AM server '%s':%d\n",
          MAMType[A->Type],
          A->Host, 
          A->Port);
 
        sprintf(Message,"AMFAILURE:  cannot receive data from %s AM server %s:%d (transaction: '%s')\n",
          MAMType[A->Type],
          A->Host,
          A->Port,
          Transaction);
 
        MSysRegEvent(Message,0,0,1);
 
        MSUFree(&S);

        __MAMEndFunc(A,0);
 
        return(FAILURE);
        }
 
      MDB(4,fAM) MLog("INFO:     received message '%s' from server\n",
        S.RBuffer);
 
      MSUDisconnect(&S);
 
      break;
    }  /* END switch(A->SocketProtocol) */

  switch(A->WireProtocol)
    {
    case mwpXML:
 
      /* no content check */

      strcpy(Response,S.RBuffer);
 
      break;
 
    default:
 
      if ((ptr = strstr(S.RBuffer,"RESULT=")) != NULL)
        {
        ptr += strlen("RESULT=");
 
        strcpy(Response,ptr);
        }
 
      if ((ptr = strstr(S.RBuffer,"STATUSCODE=")) != NULL)
        {
        ptr += strlen("STATUSCODE=");
 
        *SC = (int)strtol(ptr,NULL,0);
 
        if (*SC == 0)
          { 
          MDB(2,fAM) MLog("ALERT:    AM transaction failed.  transaction: '%s'  response: '%s'\n",
            Transaction,
            Response);
 
          sprintf(Message,"AMFAILURE:  transaction failed (transaction: '%s' : response: '%s')\n",
            Transaction,
            Response);
 
          MSysRegEvent(Message,0,0,1);
 
          MSUFree(&S);

          __MAMEndFunc(A,0);
 
          return(SUCCESS);
          }
        }    /* END else ((ptr = strstr(S.RBuffer,"STATUSCODE=")) != NULL) */
      else
        {
        MDB(2,fAM) MLog("ALERT:    cannot determine STATUSCODE for transaction '%s'\n",
          Transaction);
 
        sprintf(Message,"AMFAILURE:  cannot parse reply from AM server %s:%d (transaction: '%s')\n",
          A->Host,
          A->Port,
          Transaction);
 
        MSysRegEvent(Message,0,0,1);
 
        MSUFree(&S);

        __MAMEndFunc(A,0);
 
        return(FAILURE);
        }

      break;
    }  /* END switch(S->WireProtocol) */
 
  MSUFree(&S);

  __MAMEndFunc(A,0);
 
  return(SUCCESS);
  }  /* MAMQBDoCommand() */




int MAMGetChargeRateInfo(
 
  char *Machine)  /* I */
 
  {
  /* NYI */

  return(SUCCESS);
  }  /* END MAMGetChargeRateInfo() */
 
 
 
 
int MAMCacheTransaction(
 
  int     TIndex,  /* I */
  mres_t *R)       /* I */
 
  {
  /* NYI */

  return(SUCCESS);
  }  /* END MAMCacheTransaction() */




int MAMClose(

  mam_t *A)   /* I */
 
  {
  const char *FName = "MAMClose";

  MDB(2,fAM) MLog("%s(%s)\n",
    FName,
    (A != NULL) ? A->Name : "NULL");

  if (A == NULL)
    {
    return(FAILURE);
    }
 
  if ((MSched.Mode != msmNormal) && (MAMTest != TRUE))
    {
    return(SUCCESS);
    }
 
  switch (A->Type)
    {
    case mamtNONE:
 
      return(SUCCESS);
 
      /*NOTREACHED*/
 
      break;

    case mamtFILE:
 
      if (A->FP != NULL)
        {
        fprintf(A->FP,MAM_FILEFOOTER);
 
        fclose(A->FP);
 
        A->FP = NULL;
        }
 
      break;
 
    case mamtQBANK:
 
      /* no close required */
 
      break;
 
    case mamtGOLD:

      /* NYI */
 
      return(SUCCESS); 
 
      /*NOTREACHED*/
 
      break;
 
    default:
 
      return(SUCCESS);
 
      /*NOTREACHED*/
 
      break;
    }  /* END switch (A->Type) */
 
  return(SUCCESS);
  }  /* END MAMClose() */




int MAMProcessOConfig(

  mam_t   *AM,
  int      PIndex,
  int      IVal,
  double   DVal,
 char    *SVal,
 char   **SArray)

  {
  if (AM == NULL)
    {
    return(FAILURE);
    }

  switch(PIndex)
    {
    case pAMAppendMachineName:

      AM->AppendMachineName = MUBoolFromString(SVal,FALSE);

      break;

    case pAMChargePolicy:

      AM->ChargePolicy = MUGetIndex(SVal,(const char **)MAMChargePolicy,0,DEFAULT_MAMCHARGEPOLICY);

      break;

    case pAMDeferOnJobFailure:

      AM->DeferJobOnFailure = MUBoolFromString(SVal,TRUE);

      break;

    case pAMFallbackAccount:

      MUStrCpy(AM->FallbackAccount,SVal,sizeof(AM->FallbackAccount));

      break;

    case pAMFlushInterval:

      /* make interval evenly divisible by MAX_AMFLUSHINTERVAL */

      {
      long tmpL;

      tmpL = (long)MUTimeFromString(SVal);

      if (tmpL <= 0)
        {
        AM->FlushInterval = DEFAULT_MAMFLUSHINTERVAL;
        }
      else if (tmpL >= MAX_MAMFLUSHINTERVAL)
        {
        AM->FlushInterval = MAX_MAMFLUSHINTERVAL;
        }
      else
        {
        tmpL = tmpL % MAX_MAMFLUSHINTERVAL;

        tmpL = MAX_MAMFLUSHINTERVAL / tmpL;

        AM->FlushInterval = MAX_MAMFLUSHINTERVAL / tmpL;
        }
      }    /* END BLOCK */

      break;

    case pAMHost:

      MUStrCpy(AM->Host,SVal,sizeof(AM->Host));

      break;

    case pAMPort:

      AM->Port = IVal;

      break;

    case pAMProtocol:

      AM->WireProtocol = MUGetIndex(SVal,MAMProtocol,0,DEFAULT_MAMWIREPROTOCOL);

      break;

    case pAMTimeout:

      AM->Timeout = MUTimeFromString(SVal);

      if (AM->Timeout > 1000)
        AM->Timeout /= 1000;

      break;

    case pAMType:

      AM->Type = MUGetIndex(SVal,MAMType,0,DEFAULT_MAMTYPE);

      strcpy(AM->Name,SVal);

      break;

    default:

      break;
     }  /* END switch(PIndex) */

  return(SUCCESS);
  }  /* END MAMProcessOConfig() */



int MAMSetAttr(

  mam_t  *A,      /* I (modified) */
  int     AIndex, /* I */
  void  **Value,  /* I */
  int     Format, /* I */
  int     Mode)

  {
  const char *FName = "MAMSetAttr";

  MDB(7,fSCHED) MLog("%s(%s,%d,Value,%d)\n",
    FName,
    (A != NULL) ? A->Name : "NULL",
    AIndex,
    Mode);

  if (A == NULL)
    {
    return(FAILURE);
    }

  switch(AIndex)
    {
    case mamaCSAlgo:

      A->CSAlgo = MUGetIndex((char *)Value,MCSAlgoType,FALSE,A->CSAlgo);

      break;

    case mamaCSKey:

      MUStrCpy(A->CSKey,(char *)Value,MMAX_NAME);

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MAMSetAttr() */




int MAMSyncAlloc(

  mam_t *A,  /* I */
  mrm_t *R)  /* I */

  {
  char Line[MMAX_LINE << 2];
  char Response[MMAX_LINE << 2];
  char JobList[MMAX_LINE << 2];

  char ErrMsg[MMAX_LINE];

  int  rc;
  int  StatusCode;

  int      jindex;

  char    *ptr;
  mjob_t  *J;

  enum MHoldReasonEnum RIndex;

  char    *TokPtr;

  const char *FName = "MAMSysAlloc";

  MDB(3,fAM) MLog("%s(%s,%s)\n",
    FName,
    (A != NULL) ? A->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if ((A == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  if ((MSched.Mode != msmNormal) && (MAMTest != TRUE))
    {
    return(SUCCESS);
    }

  if (A->State == mrmsDown)
    {
    return(FAILURE);
    }

  if (MAQ[0] != -1)
    {
    strcpy(JobList,MJob[MAQ[0]]->Name);

    for (jindex = 1;MAQ[jindex] != -1;jindex++)
      {
      J = MJob[MAQ[jindex]];

      strcat(JobList,":");
      strcat(JobList,J->Name);
      }
    }
  else
    {
    JobList[0] = '\0';
    }

  switch (A->Type)
    {
    case mamtFILE:

      fprintf(A->FP,"%-15s %s\n",
        "INITIALJOBLIST",
        JobList);

      fflush(A->FP);

      break;

    case mamtQBANK:

      sprintf(Line,"COMMAND=sync_reservations AUTH=%s MACHINE=%s TYPE=%s LIST=%s",
        MSched.Admin1User[0],
        R->Name,
        MSCHED_SNAME,
        JobList);

      rc = MAMQBDoCommand(A,0,Line,NULL,&StatusCode,Response);

      if ((rc == FAILURE) || (StatusCode == FAILURE))
        {
        MDB(1,fAM) MLog("ALERT:    cannot sync reservations with AM\n");

        return(FAILURE);
        }

      if ((Response[0] == '\0') || !strcmp(Response,"1"))
        {
        MDB(1,fAM) MLog("INFO:     AM is synchronized\n");

        return(SUCCESS);
        }

      MDB(3,fAM) MLog("INFO:     the following jobs require reservations: '%s'\n",
        Response);

      ptr = MUStrTok(Response,":\n",&TokPtr);

      while (ptr != NULL)
        {
        if (MJobFind(ptr,&J,0) == SUCCESS)
          {
          MAMAllocJReserve(A,J,FALSE,&RIndex,ErrMsg);
          }
        else
          {
          MDB(1,fAM) MLog("ALERT:    cannot locate job '%s' for sync'ing AM alloc\n",
            ptr);
          }

        ptr = MUStrTok(NULL,":\n",&TokPtr);
        }  /* END while (ptr != NULL) */

      break;

    case mamtNONE:

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    default:

      return(SUCCESS);

      /*NOTREACHED*/

      break;
    }  /* END switch(A->Type) */

  return(SUCCESS);
  }  /* END MAMSyncAlloc() */




int MAMAllocResCancel(

  char  *CAName, /* I */
  char  *ReqID,  /* I */
  char  *Msg,    /* I (optional) */
  char  *EMsg,   /* O (optional) */
  enum MHoldReasonEnum *RIndex)  /* O (optional) */

  {
  char  AName[MMAX_NAME];
  char  SAName[MMAX_NAME];

  char  MAMSBuf[MMAX_LINE << 2];
  char  MAMRBuf[MMAX_LINE];

  char  tmpMsg[MMAX_LINE];

  char *ptr;

  int   rc;
  int   SC;

  const char *FName = "MAMAllocResCancel";

  MDB(3,fAM) MLog("%s(%s,%s,RIndex)\n",
    FName,
    (CAName != NULL) ? CAName : "NULL",
    ReqID);

  if (RIndex != NULL)
    *RIndex = mhrNONE;

  if (EMsg != NULL)
    EMsg[0] = '\0';
 
  if ((MSched.Mode != msmNormal) && (MAMTest != TRUE))
    {
    return(SUCCESS);
    }

  if (MAM[0].Type == mamtNONE)
    {
    return(SUCCESS);
    }

  if ((CAName[0] == '\0') || !strcmp(CAName,NONE))
    {
    MDB(2,fAM) MLog("ALERT:    no account specified for request %s\n",
      ReqID);

    return(SUCCESS);
    }

  if ((ptr = strchr(CAName,'@')) != NULL)
    {
    MUStrCpy(SAName,CAName,MIN(sizeof(SAName),ptr - CAName + 1));

    MUStrCpy(AName,(ptr + 1),sizeof(AName));
    }
  else
    {
    MUStrCpy(AName,CAName,sizeof(AName));

    MUStrCpy(SAName,"KITTY",sizeof(SAName));
    }

  if (MAM[0].AppendMachineName == TRUE)
    {
    strcat(AName,"@");
    strcat(AName,MRM[0].Name);  /* FIXME:  assumes single RM per scheduler */
    }

  if ((MAM[0].Version > 290) &&
      (Msg != NULL) &&
      (Msg[0] != '\0'))
    {
    sprintf(tmpMsg," REASON=\"%s\"",
      Msg);
    }
  else
    {
    tmpMsg[0] = '\0';
    }

  switch (MAM[0].Type)
    {
    case mamtFILE:

      fprintf(MAM[0].FP,"%-15s ACCOUNT=%s REQUESTID=%s%s\n",
        "REMOVE",
        AName,
        ReqID,
        tmpMsg);

      fflush(MAM[0].FP);

      break;

    case mamtQBANK:

      sprintf(MAMSBuf,"COMMAND=remove_reservation AUTH=%s ACCOUNT=%s JOBID=%s%s",
        MSched.Admin1User[0],
        AName,
        ReqID,
        tmpMsg);

      rc = MAMQBDoCommand(
        &MAM[0],
        0,
        MAMSBuf,
        NULL,
        &SC,
        MAMRBuf);

      if (rc == FAILURE)
        {
        MDB(1,fAM) MLog("ALERT:    cannot cancel allocation reservation for request '%s' (AM failure)\n",
          ReqID);

        *RIndex = mhrAMFailure;

        if (MAM[0].JFAction != mamjfaNONE)
          {
          return(FAILURE);
          }
        else
          {
          return(SUCCESS);
          }
        }

      if (SC == FAILURE)
        {
        MDB(1,fAM) MLog("ALERT:    cannot cancel allocation reservation for request '%s' (request refused)\n",
          ReqID);

        *RIndex = mhrAMFailure;

        return(FAILURE);
        }

      break;

    case mamtGOLD:

      /* NYI */

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    case mamtNONE:

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    default:

      return(SUCCESS);

      /*NOTREACHED*/

      break;
    }  /* END switch (MAM[0].Type) */

  return(SUCCESS);
  }  /* MAMAllocResCancel() */




int MAMAllocRReserve(

  mam_t *A,          /* I */
  char  *ResName,
  long   StartTime,
  char  *CAName,
  int    ProcCount,
  int    NodeCount,
  long   Duration,
  char  *QOSName,
  char  *NodeType,   /* I */
  enum MHoldReasonEnum *RIndex) /* O */

  {
  char    Line[MMAX_LINE << 2];
  char    Response[MMAX_LINE];

  char    AName[MMAX_NAME];
  char    SAName[MMAX_NAME];

  char   *ptr;
  char   *KittyString = "KITTY";

  int     rc;
  int     StatusCode;

  const char *FName = "MAMAllocRReserve";

  MDB(3,fAM) MLog("%s(%s,%ld,%s,%d,%d,%ld,%s,%s,RIndex)\n",
    FName,
    ResName,
    StartTime,
    CAName,
    ProcCount,
    NodeCount,
    Duration,
    QOSName,
    NodeType);

  if (RIndex != NULL)
    *RIndex = mhrNONE;

  if (A == NULL)
    {
    return(FAILURE);
    }

  if ((MSched.Mode != msmNormal) && (MAMTest != TRUE))
    {
    return(SUCCESS);
    }

  if (A->Type == mamtNONE)
    {
    return(SUCCESS);
    }

  if ((CAName == NULL) || !strcmp(CAName,NONE))
    {
    MDB(2,fAM) MLog("ALERT:    no account specified for reservation %s\n",
      ResName);

    if (RIndex != NULL)
      *RIndex = mhrNoFunds;

    return(FAILURE);
    }

  if ((ptr = strchr(CAName,'@')) != NULL)
    {
    MUStrCpy(SAName,CAName,MIN(ptr - CAName + 1,sizeof(SAName)));
    MUStrCpy(AName,ptr + 1,sizeof(AName));
    }
  else
    {
    MUStrCpy(AName,CAName,sizeof(AName));
    MUStrCpy(SAName,KittyString,sizeof(SAName));
    }

  if (A->AppendMachineName == TRUE)
    {
    MUStrCat(AName,"@",sizeof(AName));
    MUStrCat(AName,MRM[0].Name,sizeof(AName));  /* FIXME:  assumes single RM per scheduler */
    }

  switch (A->Type)
    {
    case mamtFILE:

      fprintf(A->FP,"%-15s TYPE=res ACCOUNT=%s SUBACCOUNT=%s RESOURCE=%d RESOURCETYPE=%s DURATION=%ld QOS=%s REQUESTID=%s NODES=%d\n",
        "RESERVE",
        AName,
        SAName,
        ProcCount,
        NodeType,
        Duration,
        DEFAULT,
        ResName,
        NodeCount);

      fflush(A->FP);

      break;

    case mamtQBANK:

      sprintf(Line,"COMMAND=make_reservation AUTH=%s MACHINE=%s ACCOUNT=%s USER=%s WCLIMIT=%ld PROCCOUNT=%d QOS=%s CLASS=%s NODETYPE=%s TYPE=%s JOBID=%s",
        MSched.Admin1User[0],
        MRM[0].Name,
        AName,
        SAName,
        Duration,
        ProcCount,
        QOSName,
        DEFAULT,
        NodeType,
        MSCHED_SNAME,
        ResName);

      rc = MAMQBDoCommand(A,0,Line,NULL,&StatusCode,Response);

      if (rc == FAILURE)
        {
        MDB(1,fAM) MLog("ALERT:    cannot create AM reservation for reservation '%s' (AM failure)\n",
          ResName);

        if (RIndex != NULL)
          *RIndex = mhrAMFailure;

        if (A->JFAction != mamjfaNONE)
          {
          return(FAILURE);
          }
        else
          {
          return(SUCCESS);
          }
        }

      if (StatusCode == FAILURE)
        {
        MDB(1,fAM) MLog("ALERT:    cannot create AM reservation for reservation '%s' (request refused)\n",
          ResName);

        if (RIndex != NULL)
          *RIndex = mhrNoFunds;

        return(FAILURE);
        }

      break;

    case mamtNONE:

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    default:

      return(SUCCESS);

      /*NOTREACHED*/

      break;
    }  /* END switch (A->Type) */

  return(SUCCESS);
  }   /* END MAMAllocRReserve() */




int MAMShutdown(

  mam_t *A)  /* I */

  {
  if (A == NULL)
    {
    return(FAILURE);
    }

  switch(A->Type)
    {
    case mamtNONE:

      return(SUCCESS);

      /*NOTREACHED*/

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(A->Type) */

  MResChargeAllocation(NULL,1);

  /* NOTE:  temp until AM can unregister itself */

  if (MSysDSUnregister(
       (char *)MS3CName[mpstAM],
       MRM[0].Name,
       MSched.ServerHost,
       MSched.ServerPort,
       NULL,
       NULL) == FAILURE)
    {
    MDB(1,fRM) MLog("ALERT:    cannot unregister with directory service\n");
    }

  MAMClose(A);

  return(SUCCESS);
  }  /* END MAMShutdown() */




int __MAMStartFunc(

  mam_t *A,     /* I */
  int    FType) /* I */

  {
  MUGetMS(NULL,&A->RespStartTime[FType]);

  return(SUCCESS);
  }  /* END __MRMStartFunc() */




int __MAMEndFunc(

  mam_t *A,     /* I */
  int    FType) /* I */

  {
  long NowMS;
  long Interval;

  if (A->RespStartTime[FType] <= 0)
    {
    /* invalid time */

    return(FAILURE);
    }

  MUGetMS(NULL,&NowMS);

  if (NowMS < A->RespStartTime[FType])
    Interval = A->RespStartTime[FType] - NowMS;
  else
    Interval = NowMS - A->RespStartTime[FType];

  A->RespTotalTime[FType] += Interval;
  A->RespMaxTime[FType] = MAX(A->RespMaxTime[FType],Interval);
  A->RespTotalCount[FType]++;

  if (FType != 0)
    {
    A->RespTotalTime[0] += Interval;
    A->RespMaxTime[0] = MAX(A->RespMaxTime[0],Interval);
    A->RespTotalCount[0]++;
    }

  /* reset start time */

  A->RespStartTime[FType] = -1;

  return(SUCCESS);
  }  /* EMD __MRMEndFunc() */



int MAMGoldDoCommand(

  mxml_t *ReqE,                 /* I */
  mpsi_t *P,                    /* I */
  enum MHoldReasonEnum *RIndex, /* O (optional) */
  char   *EMsg)                 /* O (optional) */

  {
  int rc;

  enum MSFC SC;

  char    tmpLine[MMAX_LINE];

  mxml_t *E;
  mxml_t *BE;
  mxml_t *DE;
  mxml_t *RE;

  if (EMsg != NULL)
    EMsg[0] = '\0';

  if (RIndex != NULL)
    *RIndex = mhrNONE;

  if ((ReqE == NULL) || (P == NULL))
    {
    return(FAILURE);
    }

  /* attach XML request to socket */

  P->S->SDE = ReqE;

  rc = MS3DoCommand(P,NULL,NULL,NULL,NULL,NULL);

  MDB(3,fAM) MLog("INFO:     command response '%s'\n",
    (P->S->RBuffer != NULL) ? P->S->RBuffer : "NULL");

  /* process response */

  DE = (mxml_t *)P->S->RDE;

  if (DE == NULL)
    {
    /* data not available */

    MDB(3,fAM) MLog("ALERT:    no job data available\n");
    }
  else
    {
    /* process job data */

    /* NYI */
    }

  /* extract response */

  E = NULL;

  if ((rc == FAILURE) ||
      (MXMLFromString(&E,P->S->RBuffer,NULL,NULL) == FAILURE) ||
      (MXMLGetChild(E,"Body",NULL,&BE) == FAILURE) ||
      (MXMLGetChild(BE,MSON[msonResponse],NULL,&RE) == FAILURE))
    {
    if (RIndex != NULL)
      *RIndex = mhrAMFailure;

    MXMLDestroyE(&E);

    MSUFree(P->S);

    MDB(3,fAM) MLog("ALERT:    cannot extract status\n");

    return(FAILURE);
    }

  MSUFree(P->S);

  if (MS3CheckStatus(RE,&SC,tmpLine) == FAILURE)
    {
    MXMLDestroyE(&E);

    MDB(3,fAM) MLog("ALERT:    failure message '%s' received\n",
      tmpLine);

    if (RIndex != NULL)
      {
      if (!strstr(tmpLine,"Insufficient balance"))
        *RIndex = mhrNoFunds;
      else
        *RIndex = mhrAMFailure;
      }

    return(FAILURE);
    }

  MXMLDestroyE(&E);

  return(SUCCESS);
  }  /* END MAMGoldDoCommand() */

/* END MAM.c */

