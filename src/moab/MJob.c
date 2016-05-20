/* HEADER */
        
/* Contains:                                 *
 *  int MJobDestroy(J)                       *
 *                                           */


#include "moab.h"
#include "msched-proto.h"  


extern mlog_t      mlog;
extern int         MAQ[];
extern mqos_t      MQOS[];
extern msched_t    MSched;
extern mrm_t       MRM[];
extern mam_t       MAM[];
extern mpar_t      MPar[];
extern mstat_t     MStat;
extern mattrlist_t MAList;        
extern mnode_t    *MNode[];
extern mjob_t     *MJob[];
extern mclass_t    MClass[];
extern mrange_t    MRange[MAX_MRANGE];

extern msim_t      MSim;
extern mckpt_t     MCP;
extern mframe_t    MFrame[];
extern msys_t      MSys;

extern mjobl_t     MJobName[];


extern const char *MComp[];
extern const char *MPolicyType[];
extern const char *MDefReason[];
extern const char *MNAccessPolicy[];
extern const char *MNAllocPolicy[];
extern const char *MJobFlags[];
extern const char *MResSetSelectionType[];
extern const char *MResSetAttrType[];
extern const char *MJRType[];
extern const char *MPolicyMode[];
extern const char *MJobAttr[];
extern const char *MReqAttr[];
extern const char *MXO[];
extern const char *MNodeState[];
extern const char *MRMXAttr[];
extern const char *MAllocRejType[];
extern const char *MRMType[];
extern const char *MPreemptPolicy[];
extern const char *MBool[];

extern mx_t X;

typedef struct {
  int Index;
  int Size;
  } _MALPSet;




/* local prototypes */

int __MALPSetCompHToL(_MALPSet *,_MALPSet *);




int MJobCreate(
 
  char    *JName,  /* I: job name */
  mbool_t  AddJob,
  mjob_t **JP)     /* O: job pointer */
 
  {
  int  jindex;
  int  index;
 
  mjob_t *Jtmp;
  mjob_t *J;

  const char *FName = "MJobCreate";
 
  DBG(5,fSTRUCT) DPrint("%s(%s,%s,%s)\n",
    FName,
    JName,
    MBool[AddJob],
    (JP != NULL) ? "JP" : "NULL");

  /* attempt to MJobFind() first ? */
 
  for (index = 1;index < MSched.M[mxoJob];index++)
    {
    J = MJob[index];
 
    /* locate empty job table slot */
 
    if ((J == NULL) || (J == (mjob_t *)1))
      {
      if ((J = (mjob_t *)calloc(1,sizeof(mjob_t))) == NULL)
        {
        DBG(1,fSTRUCT) DPrint("ALERT:    cannot allocate memory for job %s, errno: %d (%s)\n",
          (JName != NULL) ? JName : "NULL",
          errno,
          strerror(errno)); 
 
        return(FAILURE);
        }
 
      MJob[index] = J;
      }

    if (J->Name[0] == '\0')
      {
      memset(J,0,sizeof(mjob_t));
 
      if ((J->NodeList = (mnalloc_t *)calloc(
             1,
             sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1))) == NULL)
        {
        DBG(1,fSTRUCT) DPrint("ALERT:    cannot allocate memory for job %s nodelist, errno: %d (%s)\n",
          (JName != NULL) ? JName : "NULL",
          errno,
          strerror(errno));
 
        return(FAILURE);
        }
 
      /* link job into list */
 
      J->Prev = MJob[0]->Prev;
      J->Next = MJob[0];
 
      MJob[0]->Prev->Next = J;
      MJob[0]->Prev       = J;
 
      J->Index = index;
 
      /* add hash table entry */ 
 
      if (MJobAddHash(JName,index,&jindex) == SUCCESS)
        {
        *JP = J;
 
        DBG(6,fSTRUCT)
          {
          DBG(6,fSTRUCT) DPrint("INFO:     job slot %d allocated to job '%s'\n",
            index,
            JName);
 
          if (MJobFind(JName,&Jtmp,0) == SUCCESS)
            {
            if (Jtmp->Index != index)
              {
              DBG(1,fSTRUCT) DPrint("ERROR:  job table corruption (job index %d !=  hash index %d)\n",
                Jtmp->Index,
                index);
              }
            }
          }
 
        return(SUCCESS);
        }
      else
        {
        DBG(5,fSTRUCT) DPrint("WARNING:  unable to add hash table entry for job '%s'\n",
          JName);
 
        /* undo link */
 
        MJob[0]->Prev = J->Prev;
        J->Prev->Next = 0;
 
        return(FAILURE);
        } 
      }

    MTRAPJOB(J,FName);
    }     /* END for (index = 1;...) */
 
  DBG(1,fALL) DPrint("WARNING:  job buffer overflow (cannot add job '%s')\n",
    (JName != NULL) ? JName : NULL);
 
  return(FAILURE);
  }  /* END MJobCreate() */
 
 
 
 
int MReqCreate(
 
  mjob_t   *J,        /* I */
  mreq_t   *SrcRQ,    /* I (optional)  source requirements */
  mreq_t  **DstRQ,    /* O (optional)  new requirement     */
  mbool_t   DoCreate)

  {
  mreq_t *RQ;
  int     rqindex;

  const char *FName = "MReqCreate";

  DBG(2,fSTRUCT) DPrint("%s(%s,SrcRQ,DstRQ,DoCreate)\n",
    FName,
    (J != NULL) ? J->Name : NULL);

  if (J == NULL)
    {
    return(FAILURE);
    }
 
  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++);
 
  DBG(4,fSTRUCT) DPrint("INFO:     adding requirement at slot %d\n",
    rqindex);
 
  if ((RQ = (mreq_t *)calloc(1,sizeof(mreq_t))) == NULL)
    {
    DBG(1,fSIM) DPrint("ALERT:    cannot allocate memory for requirement %s:%d, errno: %d (%s)\n",
      J->Name,
      rqindex,
      errno, 
      strerror(errno));
 
    return(FAILURE);
    }
 
  if (SrcRQ != NULL)
    memcpy(RQ,SrcRQ,sizeof(mreq_t));
 
  if (RQ->NodeList == NULL)
    {
    if ((RQ->NodeList = (mnalloc_t *)calloc(
           1,
           sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1))) == NULL)
      {
      DBG(1,fSIM) DPrint("ALERT:    cannot allocate memory for hostlist %s:%d, errno: %d (%s)\n",
        J->Name,
        rqindex,
        errno,
        strerror(errno));

      free(RQ);
 
      return(FAILURE);
      }
    }    /* END if (RQ->NodeList == NULL) */
 
  RQ->NodeList[0].N = NULL;
 
  J->Req[rqindex] = RQ;
 
  RQ->Index = rqindex;
 
  if (DstRQ != NULL)
    *DstRQ = RQ;
 
  return(SUCCESS);
  }  /* END MReqCreate() */




int MJobMove(
 
  mjob_t *SrcJ,  /* I */
  mjob_t *DstJ)  /* I (modified) */
 
  {
  char Name[MAX_MNAME];
  int  Index;
 
  mjob_t *Prev;
  mjob_t *Next;
 
  mnalloc_t *NodeList;
  mnalloc_t *ReqHList;
  mnalloc_t *ExcHList;

  const char *FName = "MJobMove";

  DBG(6,fSTRUCT) DPrint("%s(%s,DstJ)\n",
    FName,
    (SrcJ != NULL) ? SrcJ->Name : "NULL");

  if ((SrcJ == NULL) || (DstJ == NULL))
    {
    return(FAILURE);
    }
 
  strcpy(Name,SrcJ->Name);
 
  Index = DstJ->Index;
 
  Prev  = DstJ->Prev;
  Next  = DstJ->Next;
 
  /* copy nodelist pointer */
 
  NodeList = SrcJ->NodeList;
 
  if (DstJ->NodeList != NULL)
    {
    if (NodeList != NULL)
      free(DstJ->NodeList);
    else
      NodeList = DstJ->NodeList; 
    }
 
  ReqHList = SrcJ->ReqHList;
 
  if (DstJ->ReqHList != NULL)
    {
    if (ReqHList != NULL)
      free(DstJ->ReqHList);
    else
      ReqHList = DstJ->ReqHList;
    }

  ExcHList = SrcJ->ExcHList;

  if (DstJ->ExcHList != NULL)
    {
    if (ExcHList != NULL)
      free(DstJ->ExcHList);
    else
      ExcHList = DstJ->ExcHList;
    }
 
  memcpy(DstJ,SrcJ,sizeof(mjob_t));
 
  DstJ->NodeList = NodeList;
  DstJ->ReqHList = ReqHList;
  DstJ->ExcHList = ExcHList;
 
  strcpy(DstJ->Name,Name);
 
  DstJ->Index = Index;
 
  DstJ->Prev  = Prev;
  DstJ->Next  = Next;
 
  DstJ->MReq  = SrcJ->MReq;
 
  /* erase old memory pointers */
 
  memset(SrcJ,0,sizeof(mjob_t));
 
  return(SUCCESS);
  }  /* END MJobMove() */ 




int MJobSetCreds(
 
  mjob_t *J,
  char   *UName,
  char   *GName,
  char   *AName)
 
  {
  const char *FName = "MJobSetCreds";

  DBG(3,fSTRUCT) DPrint("%s(%s,%s,%s,%s)\n",
    FName,
    J->Name,
    UName,
    GName,
    (AName != NULL) ? AName : "NULL");
 
  /* locate/add user record */
 
  if (MUserAdd(UName,&J->Cred.U) == FAILURE)
    {
    DBG(1,fSTRUCT) DPrint("ERROR:    user hash table overflow on job '%s' with user %s\n",
      J->Name,
      UName);
 
    return(FAILURE);
    }
 
  /* locate/add group record */
 
  if (MGroupAdd(GName,&J->Cred.G) == FAILURE) 
    {
    DBG(1,fSTRUCT) DPrint("ERROR:    group hash table overflow on job '%s' with group %s\n",
      J->Name,
      GName);
 
    return(FAILURE);
    }
 
  /* locate/add account record */
 
  if ((AName == NULL) || (AName[0] == '\0'))
    {
    if (MJobGetAccount(J,&J->Cred.A) == FAILURE)
      {
      char Message[MAX_MLINE];

      if (MAM[0].Type != mamtNONE)
        {
        DBG(1,fSTRUCT) DPrint("ERROR:    cannot determine default account for job '%s', user '%s'\n",
          J->Name,
          J->Cred.U->Name);
 
        sprintf(Message,"AMFAILURE:  cannot determine default account for job '%s', user '%s'\n",
          J->Name,
          J->Cred.U->Name);
 
        MSysRegEvent(Message,0,0,1);

        if (MAM[0].DeferJobOnFailure == TRUE)
          {
          MJobSetHold(
            J,
            (1 << mhDefer),
            MSched.DeferTime,
            mhrNoFunds,
            "cannot determine default account");
          }
        }
      }
    }
  else if (MAcctAdd(AName,&J->Cred.A) == FAILURE)
    {
    DBG(1,fSTRUCT) DPrint("ERROR:    account table overflow on job '%s' with account %s\n",
      J->Name,
      AName);
 
    return(FAILURE);
    }
 
  MJobUpdateFlags(J);
 
  return(SUCCESS);
  }  /* END MJobSetCreds() */




int MJobGetAccount(

  mjob_t   *J,  /* I */
  mgcred_t **A)  /* O */

  {
  const char *FName = "MJobGetAccount";

  DBG(7,fSTRUCT) DPrint("%s(%s,A)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if ((J == NULL) || (A == NULL))
    {
    return(FAILURE);
    }

  *A = NULL;

  /* look in cred defaults */

  if ((J->Cred.U != NULL) &&
      (J->Cred.U->F.ADef != NULL))
    {
    *A = J->Cred.U->F.ADef;

    return(SUCCESS);
    }

  if ((J->Cred.G != NULL) &&
      (J->Cred.G->F.ADef != NULL))
    {
    *A = J->Cred.G->F.ADef;
 
    return(SUCCESS);
    }

  if ((J->Cred.Q != NULL) &&
      (J->Cred.Q->F.ADef != NULL))
    {
    *A = J->Cred.Q->F.ADef;
 
    return(SUCCESS);
    }

  if (MAM[0].Type != mamtNONE)
    {
    char tmpAccount[MAX_MNAME];
    enum MHoldReasonEnum  Reason;

    /* get default account number */
 
    if (MAMAccountGetDefault(J->Cred.U->Name,tmpAccount,&Reason) == FAILURE)
      {
      return(FAILURE);
      }

    if ((MAcctFind(tmpAccount,A) == FAILURE) &&
        (MAcctAdd(tmpAccount,A) == FAILURE))
      {
      /* cannot create account structure */

      return(FAILURE);
      }
    }    /* END if (MAM[0].Type != mamtNONE) */

  if ((MSched.DefaultU != NULL) &&
      (MSched.DefaultU->F.ADef != NULL))
    {
    *A = MSched.DefaultU->F.ADef;

    return(SUCCESS);
    }

  if ((MSched.DefaultG != NULL) &&
      (MSched.DefaultG->F.ADef != NULL))
    {
    *A = MSched.DefaultG->F.ADef;

    return(SUCCESS);
    }
 
  return(SUCCESS);
  }  /* END MJobGetAccount() */




char *MJobGetName(

  mjob_t            *J,            /* I (optional) */
  char              *SpecJobName,  /* I (optional) */
  mrm_t             *SpecR,        /* I */
  char              *Buffer,       /* O */
  int                BufLen,       /* I */
  enum MJobNameEnum  NameType)     /* I */

  {
  static char tmpBuffer[MAX_MNAME];

  char  BaseJName[MAX_MNAME];

  int   Len;
  int   RMType;

  char *Head;
  char *ptr;

  mrm_t *R;

  if (SpecJobName != NULL)
    {
    strcpy(BaseJName,SpecJobName);
    }
  else if ((J != NULL) && (J->Name[0] != '\0'))
    {
    strcpy(BaseJName,J->Name);
    }
  else
    {
    if (Buffer != NULL)
      strcpy(Buffer,"NULL");

    return("NULL");
    }

  R = (SpecR != NULL) ? SpecR : &MRM[0];

  RMType = (R->Type != 0) ? R->Type : DEFAULT_MRMTYPE;

  if (Buffer != NULL)
    {
    Head = Buffer;
    Len  = BufLen;
    }
  else
    {
    Head = tmpBuffer;
    Len  = sizeof(tmpBuffer);
    }

  Head[0] = '\0';

  if (R->DefaultQMDomain[0] == '\0')
    {
    /* obtain default domain name */

    switch(RMType)
      {
      case mrmtLL:

        {
        char *ptr;
        char *Head;

        char tmpName[MAX_MNAME];

        /* FORMAT:  <HOST>[.<DOMAIN>].<JOBID>.<STEPID> */

        MUStrCpy(tmpName,BaseJName,MAX_MNAME);

        /* ignore hostname */

        if ((ptr = strchr(tmpName,'.')) == NULL)
          break;

        Head = ptr + 1;

        /* remove job/step id */

        if ((ptr = strrchr(Head,'.')) == NULL)
          break;

        *ptr = '\0';

        if ((ptr = strrchr(Head,'.')) == NULL)
          break;

        *ptr = '\0';

        if (strlen(Head) <= 1) 
          break;

        MUStrCpy(R->DefaultQMDomain,Head,Len);
        }  /* END BLOCK */
 
        break;
 
      case mrmtPBS:

        /* NO-OP */

        break;
 
      default:

        /* NO-OP */
 
        break;
      }  /* END switch(RMType) */

    if (R->DefaultQMDomain[0] == '\0')
      {
      /* domain name could not be determined */

      if (MSched.DefaultDomain[0] != '\0')
        {
        MUStrCpy(R->DefaultQMDomain,&MSched.DefaultDomain[1],sizeof(R->DefaultQMDomain));
        }
      }    /* END if (R->DefaultQMDomain[0] == '\0') */
    }      /* END if (R->DefaultQMDomain[0] == '\0') */

  if (R->DefaultQMHost[0] == '\0')
    {
    switch(RMType)
      {
      case mrmtPBS:

        /* format:  <JOBID>.<QMHOST> */

        if ((ptr = strchr(BaseJName,'.')) != NULL)
          MUStrCpy(R->DefaultQMHost,ptr + 1,Len);
        else
          MUStrCpy(R->DefaultQMHost,MSched.ServerHost,sizeof(R->DefaultQMHost));

        break;

      case mrmtLL:

        /* NO-OP */

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch(RMType) */
    }    /* END if (R->DefaultQMHost[0] == '\0') */

  switch (NameType)
    {
    case mjnShortName: 

      switch(RMType)
        {
        case mrmtLL:

          {
          char *ptr;
          char *TokPtr;

          char  tmpName[MAX_MNAME];

          char *Host;
          char *JobID;
          char *StepID;

          MUStrCpy(tmpName,BaseJName,MAX_MNAME);
        
          ptr = MUStrTok(tmpName,".",&TokPtr);

          /* FORMAT:  <HOST>[.<DOMAIN>].<JOBID>.<STEPID> */

          Host = ptr;

          JobID = NULL;

          StepID = NULL;

          while (ptr != NULL)
            {
            JobID = StepID;

            StepID = ptr;

            ptr = MUStrTok(NULL,".",&TokPtr);
            }  /* END while(ptr != NULL) */
 
          if ((Host == NULL) || (JobID == NULL) || (StepID == NULL))
            {
            MUStrCpy(Head,BaseJName,Len);
            }
          else
            {
            sprintf(Head,"%s.%s.%s",
              Host,
              JobID,
              StepID);
            }
          }    /* END BLOCK */

          break;

        case mrmtPBS:

          /* FORMAT:  <JOBID>[.HOST[.<DOMAIN>]] */

          if ((ptr = strchr(BaseJName,'.')) != NULL)
            MUStrCpy(Head,BaseJName,MIN(ptr - BaseJName + 1,Len));
          else
            MUStrCpy(Head,BaseJName,Len);

          break;

        default:

          /* i.e., WIKI, etc */
 
          /* copy entire name */
 
          MUStrCpy(Head,BaseJName,Len);

          break;
        }  /* END switch(RMType) */
   
      break;

    case mjnSpecName:

      if (J != NULL)
        {
        if (J->AName != NULL)
          {
          MUStrCpy(Head,J->AName,Len);
          }  /* END if (J->AName != NULL) */
        else
          {
          MUStrCpy(Head,J->Name,Len);
          }
        }

      break;

    case mjnFullName: 
    case mjnRMName:
    default:

      if ((J != NULL) && (J->RMJID != NULL))
        {
        MUStrCpy(Head,J->RMJID,Len);

        break;
        }

      switch(RMType)
        {
        case mrmtLL:

          /* FORMAT:  <HOST>[.<DOMAIN>].<JOBID>.<STEPID> */

          if ((R->DefaultQMDomain[0] == '\0') || 
              (strstr(BaseJName,R->DefaultQMDomain) != NULL))
            {
            /* no QM domain or QM domain already in job name */

            MUStrCpy(Head,BaseJName,Len);
            }
          else
            {
            char *ptr;

            if ((ptr = strchr(BaseJName,',')) == NULL)
              {
              MUStrCpy(Head,BaseJName,Len);

              break;
              }

            strncpy(Head,BaseJName,(ptr - BaseJName));

            strcat(Head,R->DefaultQMDomain);

            strcat(Head,ptr);
            }  /* END else (R->DefaultQMDomain[0] == '\0') */

          break;

        case mrmtPBS:
        default:

          /* FORMAT:  <JOBID>.<QMHOST> */

          MUStrCpy(Head,BaseJName,Len);

          /* append domain */
 
          if ((R->DefaultQMHost[0] != '\0') &&
              (strchr(Head,'.') == NULL))
            {
            MUStrCat(Head,".",Len);
            MUStrCat(Head,R->DefaultQMHost,Len);
            }

          break;
        }  /* END switch(RMType) */

      break;
    }  /* END switch(NameType) */
 
  return(Head);
  }  /* END MJobGetName() */




int MJobDestroy(
 
  mjob_t **JP)  /* I */
 
  {
  int    rqindex;
 
  mreq_t *RQ;

  mjob_t *J;

  const char *FName = "MJobDestroy";
 
  DBG(2,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    (JP != NULL) ? (*JP)->Name : "NULL");
 
  if ((JP == NULL) || (*JP == NULL))
    {
    DBG(0,fSTRUCT) DPrint("ERROR:    invalid job pointer specified\n");
 
    return(FAILURE);
    }

  J = *JP;      

  if (X.XJobDestroy != (int (*)())0)
    {
    (*X.XJobDestroy)(X.xd,JP,0);
    }

  /* release job reservations */
 
  if (J->R != NULL) 
    {
    MResDestroy(&J->R);
    }

  /* why was this commented out */
 
  MUFree((char **)&J->Cred.ACL);

  MUFree((char **)&J->Cred.CL);

  MUFree(&J->AName); 
  MUFree(&J->Message);

  MUFree(&J->E.Env);
  MUFree(&J->E.IWD);
  MUFree(&J->E.Input);
  MUFree(&J->E.Output);
  MUFree(&J->E.Error);
  MUFree(&J->E.Args);
  MUFree(&J->E.Cmd);

  MUFree(&J->RMXString);
  MUFree(&J->RMSubmitString);

  MUFree(&J->MasterJobName);
  MUFree(&J->MasterHostName);

  MUFree((char **)&J->ReqHList);
  MUFree((char **)&J->ExcHList);

  MUFree((char **)&J->Depend);

  MUFree(&J->NeedNodes);
  MUFree(&J->Geometry);          

  if (J->SIData != NULL)
    MSDataDestroy(&J->SIData);

  if (J->SOData != NULL)
    MSDataDestroy(&J->SOData);

  if (J->Ckpt != NULL)
    MJobCPDestroy(J,&J->Ckpt);
 
  /* erase req structures */
 
  for (rqindex = 0;rqindex < MAX_MREQ_PER_JOB;rqindex++)
    {
    RQ = J->Req[rqindex];
 
    if (RQ == NULL)
      break;

    MReqDestroy(&J->Req[rqindex]);
    }  /* END for (rqindex) */
 
  MUFree((char **)&J->NodeList);
  MUFree(&J->SystemID); 
  MUFree(&J->RMJID);
 
  /* erase job structure */

  memset(J,0,sizeof(mjob_t));

  if (J->Index > 0)
    { 
    MUFree((char **)JP);

    *JP = NULL;
    }
 
  return(SUCCESS);
  }  /* END MJobDestroy() */




int MReqDestroy(

  mreq_t **RQP)  /* I */

  {
  int index;

  mreq_t *RQ;

  if ((RQP == NULL) || (*RQP == NULL))
    {
    return(SUCCESS);
    }

  RQ = *RQP;

  for (index = 0;RQ->SetList[index] != NULL;index++)
    {
    MUFree((char **)&RQ->SetList[index]);
    }  /* END for (index) */
 
  MUFree((char **)&RQ->NodeList);
  MUFree((char **)&RQ->NAllocPolicy);

  MUFree((char **)RQP);

  return(SUCCESS);
  }  /* END MReqDestroy() */





int MJobGetRunPriority(

  mjob_t *J,        /* I */
  int     PIndex,   /* I */
  double *Priority, /* O */
  char   *Buffer)   /* O (optional) */

  {
  double PercentResUsage;

  long   Duration;
  long   WallTime;             

  long   ResourcesRequired;
  long   ResourcesConsumed;
  long   ResourcesRemaining;

  if ((J == NULL) || 
     ((J->State != mjsStarting) && (J->State != mjsRunning)) || 
      (Priority == NULL))
    {
    return(FAILURE);
    }

  /* NOTE:  allow TotalUsage to be determined by WallTime or CPUTime */
  /*        for now, calculate as dedicated PS                       */

  Duration          = J->WCLimit;
  WallTime          = J->AWallTime;
  ResourcesRequired = MJobGetProcCount(J);

  ResourcesConsumed  = ResourcesRequired * WallTime;
  ResourcesRemaining = ResourcesRequired * (Duration - WallTime);

  PercentResUsage    = WallTime * 100 / MAX(1,Duration - WallTime);
 
  *Priority = MPar[0].FSC.PCW[mpcUsage] * (
     MPar[0].FSC.PSW[mpsUCons] * ResourcesConsumed +
     MPar[0].FSC.PSW[mpsURem]  * ResourcesRemaining +
     MPar[0].FSC.PSW[mpsUPerC] * PercentResUsage
     /* MPar[0].FSC.PSW[mpsUExeTime] * (MSched.Time - J->StartTime) */);

  *Priority += J->StartPriority;

  *Priority += MSched.PreemptPrioMargin;
 
  return(SUCCESS);
  }  /* END MJobGetRunPriority() */




int MJobGetBackfillPriority(

  mjob_t        *J,
  unsigned long  MaxDuration,
  int            PIndex,
  double        *BFPriority,
  char          *Buffer)

  {
  double WCAccuracy;
  long   Duration;

  if ((J == NULL) || (BFPriority == NULL))
    {
    return(FAILURE);
    }

  /* Need probability of success info */
  /* May need idle job priority       */

  Duration = J->WCLimit;

  switch (MPar[0].BFPriorityPolicy)
    { 
    case mbfpDuration:

      *BFPriority = MAX_MTIME - Duration;

      break;

    case mbfpHWDuration:

      MJobGetWCAccuracy(J,&WCAccuracy);

      *BFPriority = (double)MAX_MTIME - WCAccuracy * Duration;

      break;

    case mbfpRandom:
    default:
 
      *BFPriority = 1;
 
      break;
    }  /* END switch(MSched.BFPriorityPolicy) */

  return(SUCCESS);  
  }  /* END MJobGetBackfillPriority() */




double MJobGetWCAccuracy(

  mjob_t *J,
  double *Accuracy)

  {
  if ((J == NULL) || (Accuracy == NULL))
    {
    return(FAILURE);
    }

  if ((J->Cred.U != NULL) && (J->Cred.U->Stat.Count > 0))
    {
    *Accuracy = J->Cred.U->Stat.JobAcc / J->Cred.U->Stat.Count;
    }
  else if ((J->Cred.G != NULL) && (J->Cred.G->Stat.Count > 0))
    {
    *Accuracy = J->Cred.G->Stat.JobAcc / J->Cred.G->Stat.Count;
    }
  else if ((J->Cred.A != NULL) && (J->Cred.A->Stat.Count > 0))
    {
    *Accuracy = J->Cred.A->Stat.JobAcc / J->Cred.A->Stat.Count;
    }
  else if (MPar[0].S.Count > 0)
    {
    *Accuracy = MPar[0].S.JobAcc / MPar[0].S.Count;
    }
  else
    {
    *Accuracy = 1.00;
    }
  
  return(SUCCESS);  
  }  /* END MJobGetWCAccuracy() */




int MJobGetPartitionAccess(

  mjob_t *J)  /* I */

  {
  if (J == NULL)
    {
    return(FAILURE);
    }

  /* incorporate system, user, group, account, qos, and class constraints */

  /* NYI */

  return(SUCCESS);
  }  /* END MJobGetPartitionAccess() */





int MJobUpdateFlags(

  mjob_t *J)  /* I (modified) */

  {
  mreq_t *RQ;

  if (J == NULL)
    {
    return(FAILURE);
    }

  RQ = J->Req[0];

  J->Flags = (J->SpecFlags | 
              J->SysFlags |
            ((J->Cred.U != NULL) ? J->Cred.U->F.JobFlags : 0) |
            ((J->Cred.G != NULL) ? J->Cred.G->F.JobFlags : 0) |
            ((J->Cred.A != NULL) ? J->Cred.A->F.JobFlags : 0) |
            ((J->Cred.C != NULL) ? J->Cred.C->F.JobFlags : 0));

  if (J->Cred.Q != NULL)
    {
    if (J->Cred.Q->Flags & (1 << mqfusereservation))
      J->Flags |= (1 << mjfAdvReservation);

    if (J->Cred.Q->Flags & (1 << mqfpreemptee))
      J->Flags |= (1 << mjfPreemptee);

    if (J->Cred.Q->Flags & (1 << mqfpreemptor))
      J->Flags |= (1 << mjfPreemptor);

    if (J->Cred.Q->Flags & (1 << mqfignHostList))
      J->Flags &= ~(1 << mjfHostList);

    if ((J->Cred.Q->Flags & (1 << mqfPreemptSPV)) &&
        (J->SysFlags & (1 << mjfSPViolation)))
      {
      J->Flags |= (1 << mjfPreemptee);
      }

    if (RQ != NULL)
      {
      if (J->Cred.Q->Flags & (1 << mqfdedicated))
        {
        J->Flags |= (1 << mjfNASingleJob);
        }
      }
    }    /* END if (J->Cred.Q != NULL) */

  /* determine task distribution policy */

  if (J->SpecDistPolicy != 0)
    J->DistPolicy = J->SpecDistPolicy;
  else if ((J->Cred.C != NULL) && (J->Cred.C->DistPolicy != 0))
    J->DistPolicy = J->Cred.C->DistPolicy;

  /* sync job flags and req access policy */

  /* NOTE:  does not allow per req access policy specification */

  if (RQ != NULL)
    {
    if (J->Flags & (1 << mjfNASingleJob))
      {
      RQ->NAccessPolicy = mnacSingleJob;
      }
    else if (J->Flags & (1 << mjfNASingleTask))
      {
      RQ->NAccessPolicy = mnacSingleTask;
      }
    else
      {
      RQ->NAccessPolicy = MSched.DefaultNAccessPolicy;
      }
    }    /* END if (RQ != NULL) */
 
  DBG(7,fSTRUCT) DPrint("INFO:     job flags for job %s: %x, req napolicy=%s\n",
    J->Name,
    (int)J->Flags,
    (RQ != NULL) ? MNAccessPolicy[RQ->NAccessPolicy] : "N/A");

  /* update job attributes */

  if (J->Flags & (1 << mjfPreemptee))
    MJobSetAttr(J,mjaGAttr,(void **)MJobFlags[mjfPreemptee],mdfString,mSet);
  else
    MJobSetAttr(J,mjaGAttr,(void **)MJobFlags[mjfPreemptee],mdfString,mUnset);

  return(SUCCESS);
  }  /* END MJobUpdateFlags() */




int MReqCheckResourceMatch(

  mjob_t  *J,      /* I (optional) */
  mreq_t  *RQ,     /* I */
  mnode_t *N,      /* I */
  int     *RIndex) /* O (optional) */

  {
  char tmpLine[MAX_MLINE]; 

  static mcres_t ZRes;
  static int     Initialized = FALSE;

  int TC;
  int MinTPN;
  int Found;
  int index;

  const char *FName = "MReqCheckResourceMatch";

  DBG(5,fSCHED) DPrint("%s(%s,%d,%s,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (RQ != NULL) ? RQ->Index : -1,
    (N != NULL) ? N->Name : "NULL",
    (RIndex != NULL) ? "RIndex" : "NULL");

  if (RIndex != NULL)
    *RIndex = 0;

  if ((RQ == NULL) || (N == NULL))
    {
    return(FAILURE);
    }

  if (J != NULL)
    {
    /* node rejected if in exc hlist */

    if (J->ExcHList != NULL)
      {
      for (index = 0;J->ExcHList[index].N != NULL;index++)
        {
        if (N == J->ExcHList[index].N)
          {
          DBG(5,fSCHED) DPrint("INFO:     node in excluded hostlist\n");
 
          if (RIndex != NULL)
            *RIndex = marHostList;

          return(FAILURE);
          }
        }    /* END for (index) */
      }      /* END if (J->ExcHList != NULL) */

    /* node is acceptable if it appears in hostlist */
 
    /* NOTE:  must allow super/sub host mask,                                 */
    /*        ie, node missing from hostlist does NOT imply node not feasible */
 
    if ((J->Flags & (1 << mjfHostList)) && 
        (J->ReqHList != NULL) &&
        (RQ->DRes.Procs != 0))
      {
      Found = FALSE;

      for (index = 0;J->ReqHList[index].N != NULL;index++)
        {
        if (N == J->ReqHList[index].N)
          {
          DBG(5,fSCHED) DPrint("INFO:     node in requested hostlist\n");

          Found = TRUE;

          break;
          }
        }    /* END for (index) */

      if ((Found == FALSE) &&  
         ((J->ReqHLMode != mhlmSubset) ||
          (MPar[0].EnableMultiNodeJobs == FALSE)))
        {
        DBG(5,fSCHED) DPrint("INFO:     node is not in specified hostlist\n");
 
        if (RIndex != NULL)
          *RIndex = marHostList;
 
        return(FAILURE);
        }

      if ((Found == FALSE) && (J->ReqHLMode == mhlmExactSet))
        {
        DBG(5,fSCHED) DPrint("INFO:     node is not in specified hostlist\n");

        if (RIndex != NULL)
          *RIndex = marHostList;

        return(FAILURE);
        }
      }    /* END if ((J->Flags & (1 << mjfHostList)) && ... ) */
    }      /* END if (J != NULL) */

  if (Initialized == FALSE)
    {
    memset(&ZRes,0,sizeof(ZRes));   

    Initialized = TRUE;
    }

  /* check OS, node features, configured procs, configured memory, configured classes ... */

  if (RQ->DRes.Procs != 0)
    { 
    /* check opsys */
 
    if ((RQ->Opsys != 0) && (N->ActiveOS != RQ->Opsys))
      {
      DBG(4,fSCHED) DPrint("INFO:     opsys does not match ('%s' needed  '%s' available)\n",
        MAList[eOpsys][RQ->Opsys],
        MAList[eOpsys][N->ActiveOS]);

      if (RIndex != NULL)    
        *RIndex = marOpsys;
 
      return(FAILURE);
      }

    /* check architecture */
 
    if ((RQ->Arch != 0) && (N->Arch != RQ->Arch))
      {
      DBG(5,fSCHED) DPrint("INFO:     architectures do not match (%s needed  %s found)\n",
        MAList[eArch][RQ->Arch],
        MAList[eArch][N->Arch]);

      if (RIndex != NULL)        
        *RIndex = marArch;
 
      return(FAILURE);
      }
    }    /* END if (RQ->DRes.Procs != 0) */
 
  /* check features */

  /* verify all requested features are found */

  if (MAttrSubset(
        N->FBM,
        RQ->ReqFBM,
        sizeof(N->FBM),
        RQ->ReqFMode) != SUCCESS)
    {
    MUStrCpy(
      tmpLine,
      MUMAList(eFeature,N->FBM,sizeof(N->FBM)),
      sizeof(tmpLine));
 
    DBG(5,fSCHED) DPrint("INFO:     inadequate features (%s needed  %s found)\n",
      MUMAList(eFeature,RQ->ReqFBM,sizeof(RQ->ReqFBM)),
      tmpLine);

    if (RIndex != NULL)     
      *RIndex = marFeatures;
 
    return(FAILURE);
    }
 
  /* check configured procs */
 
  if (!MUCompare(N->CRes.Procs,RQ->ProcCmp,RQ->RequiredProcs))
    {
    DBG(5,fSCHED) DPrint("INFO:     inadequate procs (%s %d needed  %d found)\n",
      MComp[RQ->ProcCmp],
      RQ->RequiredProcs,
      N->CRes.Procs);

    if (RIndex != NULL)      
      *RIndex = marCPU;
 
    return(FAILURE);
    }
 
  /* check configured memory */
 
  if (!MUCompare(N->CRes.Mem,RQ->MemCmp,RQ->RequiredMemory))
    {
    DBG(5,fSCHED) DPrint("INFO:     inadequate memory (%s %d needed  %d found)\n",
      MComp[RQ->MemCmp],
      RQ->RequiredMemory,
      N->CRes.Mem);

    if (RIndex != NULL)      
      *RIndex = marMemory;
 
    return(FAILURE);
    }

  if (MUNumListGetCount(
        (J != NULL) ? J->StartPriority : 0,
        RQ->DRes.PSlot,
        N->CRes.PSlot,
        0,
        NULL) == FAILURE)
    {
    DBG(5,fSCHED) DPrint("INFO:     node is missing classes [ALL %d:%d]%s required:configured\n",
      RQ->DRes.PSlot[0].count,
      N->CRes.PSlot[0].count,
      MUCAListToString(RQ->DRes.PSlot,N->CRes.PSlot,NULL));

    if (RIndex != NULL)   
      *RIndex = marClass;
 
    return(FAILURE);
    }

  TC = MNodeGetTC(N,&N->CRes,&N->CRes,&ZRes,&RQ->DRes,MAX_MTIME);      

  if ((TC == 0) || (TC < RQ->TasksPerNode))
    {
    DBG(5,fSCHED) DPrint("INFO:     node supports %d task%c (%d tasks/node required)\n",
      TC,
      (TC == 1) ? ' ' : 's',
      RQ->TasksPerNode);

    if (RIndex != NULL)                                                                    
      *RIndex = marCPU;
 
    return(FAILURE);
    }

  /* check resource pools */

  if (RQ->Pool > 0)
    {
    Found = FALSE;
 
    for (index = 0;index < MAX_MPOOL;index++)
      {
      if (N->Pool[index] == '\0')
        break;

      if (N->Pool[index] == RQ->Pool)
        {
        Found = TRUE;
 
        break;
        }
      }
 
    if (Found == FALSE)
      {
      DBG(3,fSCHED) DPrint("INFO:     pool not found ('%d' needed)\n",
        RQ->Pool - 1);

      if (RIndex != NULL) 
        *RIndex = marPool;
 
      return(FAILURE);
      }
    }  /* END if (RQ->Pool > 0) */

  if ((N->RM != NULL) && (N->RM->Type == mrmtLL))
    {
    /* handle LL geometry */
 
    MinTPN = (RQ->TasksPerNode > 0) ? RQ->TasksPerNode : 1;
 
    if (RQ->NodeCount > 0)
      {
      MinTPN = RQ->TaskCount / RQ->NodeCount;
 
      if (RQ->TaskCount % RQ->NodeCount)
        MinTPN++;
      }
 
    /* handle pre-LL22 geometries */

    if ((RQ->BlockingFactor != 1) &&
       ((TC < (MinTPN - 1)) ||
        (TC == 0) ||
       ((TC < MinTPN) && (RQ->NodeCount > 0) && (RQ->TaskCount % RQ->NodeCount == 0))))
      {
      DBG(5,fSCHED) DPrint("INFO:     node supports %d task%c (%d tasks/node required:LL)\n",
        TC,
        (TC == 1) ? ' ' : 's',
        MinTPN);

      if (RIndex != NULL)
        *RIndex = marCPU;
 
      return(FAILURE);
      }
    }    /* END if ((N->RM != NULL) && (N->RM->Type == mrmtLL)) */

  DBG(5,fSCHED) DPrint("INFO:     node %s can provide resources for job %s:%d\n",
    N->Name,
    (J != NULL) ? J->Name : "NULL",
    RQ->Index);
 
  return(SUCCESS);
  }  /* END MReqCheckResourceMatch() */




int MJobGetProcCount(

  mjob_t *J)  /* I */

  {
  if (J->C.TotalProcCount == 0)
    MJobUpdateResourceCache(J,0);

  return(J->C.TotalProcCount);
  }  /* END MJobGetProcCount() */




int MJobUpdateResourceCache(

  mjob_t *J,      /* I */
  int     SIndex) /* I */

  {
  mreq_t *RQ;

  int rqindex;
  int nindex;

  int proccount;

  if (J == NULL)
    {
    return(FAILURE);
    }

  /* update total proc count         */ 
  /* must handle dedicated resources */

  proccount = 0;

  if ((J->State == mjsStarting) || (J->State == mjsRunning))
    {
    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      RQ = J->Req[rqindex];

      if ((RQ->NAccessPolicy == mnacSingleJob) ||
          (RQ->NAccessPolicy == mnacSingleTask) ||
          (RQ->DRes.Procs == -1))
        {
        /* node resources are dedicated */

        if ((RQ->NodeList == NULL) || (RQ->NodeList[0].N == NULL))
          {
          /* no hostlist specified */

          /* HACK:  assumes all nodes have similar processor configurations */

          if (RQ->NAccessPolicy == mnacSingleJob)
            {
            /* assume packing */

            proccount += RQ->TaskCount * RQ->DRes.Procs;
            }
          else
            {
            /* only one task per node */

            proccount += RQ->TaskCount *
                ((MNode[0] != NULL) ? MNode[0]->CRes.Procs : 1);
            }

          continue;
          }

        /* step through node list */

        for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
          {
          proccount += RQ->NodeList[nindex].N->CRes.Procs;
          }
        }
      else
        {
        /* resources not dedicated */

        proccount += RQ->TaskCount * RQ->DRes.Procs;
        }
      }    /* END for (rqindex) */
    }      /* END if ((J->State == mjsStarting) || ... ) */
  else
    {

    /* final job task mapping not established */

    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      RQ = J->Req[rqindex];

      if ((RQ->NAccessPolicy == mnacSingleJob) ||
          (RQ->NAccessPolicy == mnacSingleTask) ||
          (RQ->DRes.Procs == -1))
        {
        int TC;

        /* node resources are dedicated */

        /* NOTE:  assume homogeneous resources with MNode[0] indicative of procs/node */

        TC = (SIndex > 0) ? RQ->TaskRequestList[SIndex] : RQ->TaskCount;

        if ((RQ->NAccessPolicy == mnacSingleTask) ||
            (RQ->DRes.Procs == -1))
          {
          /* only one task per node */

          proccount += TC * ((MNode[0] != NULL) ? MNode[0]->CRes.Procs : 1);
          }
        else	
          {
          /* assume packing */

          proccount += TC * RQ->DRes.Procs;
          }
        }
      else
        {
        /* resources not dedicated */

        if (SIndex > 0)
          proccount += RQ->TaskRequestList[SIndex] * RQ->DRes.Procs;
        else
          proccount += RQ->TaskCount * RQ->DRes.Procs;
        }
      }    /* END for (rqindex) */
    }      /* END else (J->State) */

  if ((proccount > 0) || (J->Req[0] == NULL))
    {
    J->C.TotalProcCount = proccount;
    }
  else 
    {
    J->C.TotalProcCount = J->Request.TC * 
        ((J->Req[0]->DRes.Procs > 0) ? J->Req[0]->DRes.Procs : 1);
    }

  J->C.TotalProcCount = MAX(1,J->C.TotalProcCount);

  return(SUCCESS);
  }  /* END MJobUpdateResourceCache() */




int MJobClearResourceCache(

  mjob_t *J)  /* I */

  {
  J->C.TotalProcCount = 0;

  return(SUCCESS);
  }  /* END MJobClearResourceCache() */




int MJobRemove(
 
  mjob_t *J)  /* I */
 
  {
  const char *FName = "MJobRemove";

  DBG(2,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    ((J != NULL) && (J != (mjob_t *)1)) ? J->Name : "NULL");
 
  if ((J == NULL) || 
      (J == (mjob_t *)1))
    {
    DBG(0,fSTRUCT) DPrint("ERROR:    invalid job pointer passed to %s()\n",
      FName);
 
    return(SUCCESS);
    }

  if ((J->Name[0] == '\0') ||
      (J->Name[0] == '\1'))
    {
    DBG(0,fSTRUCT) DPrint("INFO:     job previously removed\n");
 
    return(SUCCESS);
    }

  if ((J->State == mjsStarting) || (J->State == mjsRunning))
    {
    /* job process completed? */
    }

  /* release job reservation */
 
  if (J->R != NULL)
    {
    if (MResDestroy(&J->R) == FAILURE)
      {
      DBG(0,fSTRUCT) DPrint("ERROR:    unable to destroy job reservation\n");
      }
    }

  if ((J->Flags & (1 << mjfAdvReservation)) &&
      (J->ResName[0] != '\0'))
    {
    mres_t *R;

    if (MResFind(J->ResName,&R) == SUCCESS)
      {
      if (R->Flags & (1 << mrfSingleUse))
        {
        if (MResDestroy(&R) == FAILURE)
          {
          DBG(0,fSTRUCT) DPrint("ERROR:    unable to destroy job reservation\n");
          }
        }
      }
    }    /* END if ((J->Flags & (1 << mjfAdvReservation)) && ...) */

  if (J->Prev != NULL)
    { 
    /* job is in scheduler job table */

    MJobRemoveFromNL(J,NULL);

    MQueueRemoveAJob(J,stForce);

    /* remove hash table entry */

    MJobRemoveHash(J->Name);

    /* unlink job */

    J->Prev->Next = J->Next;
    J->Next->Prev = J->Prev;
 
    MSim.QueueChanged = TRUE;
    }

  MJobDestroy(&MJob[J->Index]);
 
  return(SUCCESS);
  }  /* END MJobRemove() */




int MQueueRemoveAJob(

  mjob_t *J,     /* I */
  int     Mode)  /* I (stForce, ... ) */

  {
  int jindex;
  int offset;

  if (J == NULL) 
    {
    return(FAILURE);
    }

  if ((Mode != stForce) && 
      (J->State != mjsStarting) && 
      (J->State != mjsRunning))
    {
    return(SUCCESS);
    }

  DBG(6,fSTAT)
    {
    DBG(6,fSTAT) DPrint("INFO:     active jobs: ");
 
    for (jindex = 0;MAQ[jindex] != -1;jindex++)
      {
      fprintf(mlog.logfp,"[%d : %s]",
        MAQ[jindex],
        MJob[MAQ[jindex]]->Name);
      }
 
    fprintf(mlog.logfp,"\n");
    }

  offset = 0;

  for (jindex = 0;MAQ[jindex] != -1;jindex++)
    {
    if (J->Index == MAQ[jindex])
      {
      offset++;
      }
 
    if (offset > 0)
      {
      MAQ[jindex] = MAQ[jindex + offset];
      }
    }    /* END for (jindex) */
 
  MAQ[jindex] = -1;
 
  DBG(6,fCORE)
    {
    DBG(6,fCORE) DPrint("INFO:     active jobs: ");
 
    for (jindex = 0;MAQ[jindex] != -1;jindex++)
      {
      fprintf(mlog.logfp,"[%d : %s]",
        MAQ[jindex],
        MJob[MAQ[jindex]]->Name);
      }  /* END for (jindex) */
 
    fprintf(mlog.logfp,"\n");
    }

  return(SUCCESS);
  }    /* END MQueueRemoveAJob() */




int MJobSetQOS(

  mjob_t *J,    /* I (modified) */
  mqos_t *Q,    /* I */
  int     Mode) /* I */

  {
  if ((J == NULL) || (Q == NULL))
    {
    return(FAILURE);
    }

  if (J->Cred.Q != Q)
    {
    J->Cred.Q = Q;

    MJobUpdateFlags(J);

    MJobBuildCL(J);
    }

  return(SUCCESS);
  }  /* END MJobSetQOS() */




int MJobPreempt(

  mjob_t  *J,              /* I */
  mjob_t **JPeer,          /* I */
  enum MPreemptPolicyEnum PreemptPolicy,  /* I */
  char    *Msg,            /* O */
  int     *SC)             /* O */

  {
  int nindex;
  int TC;
  
  mnode_t *N;
  mreq_t  *RQ;

  int PPolicy;

  const char *FName = "MJobPreempt";

  DBG(4,fSCHED) DPrint("%s(%s,JPeer,%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    MPreemptPolicy[PreemptPolicy]);

  if (J == NULL)
    {
    return(FAILURE);
    }

  PPolicy = PreemptPolicy;

  if (Msg != NULL)
    Msg[0] = '\0';

  /* verify job state */

  if ((J->State != mjsStarting) && (J->State != mjsRunning))
    {
    DBG(4,fSCHED) DPrint("WARNING:  cannot preempt non-active job '%s' (state: '%s' estate: '%s')\n",
      J->Name,
      MJobState[J->State],
      MJobState[J->EState]);

    if (Msg != NULL)
      sprintf(Msg,"cannot preempt non-active job");

    return(FAILURE);
    }

  switch(PPolicy)
    {
    case mppRequeue:

      if (MRMJobRequeue(J,JPeer,SC) == FAILURE)
        {
        /* cannot requeue job */

        return(FAILURE);
        }

      MJobRemoveFromNL(J,NULL);    

      /* remove job from active job list */
 
      MQueueRemoveAJob(J,stForce);

      /* change job state */

      MJobSetState(J,mjsIdle); 
 
      /* release reservation */
 
      if (J->R != NULL)
        {
        MResDestroy(&J->R);

        MResAdjustDRes(J->Name,0);
        }
 
      /* reset job start time */
 
      J->StartTime       = 0;
      J->SystemQueueTime = MSched.Time;
      J->AWallTime       = 0;
      J->SWallTime       = 0;
 
      /* reset job stats */
 
      J->TaskCount = 0;
      J->PSUtilized = 0.0;
 
      /* adjust job mode */
 
      if (J->SysFlags & (1 << mjfBackfill))
        J->SysFlags ^= (1 << mjfBackfill);

      break;

    case mppCheckpoint:
 
      if (MRMJobCheckpoint(J,TRUE,NULL,SC) == FAILURE)
        {
        /* cannot checkpoint job */
 
        return(FAILURE);
        }

      /* NOTE:  checkpoint can either ckpt and resume or ckpt and terminate */

      break;

    case mppSuspend:
 
      if (MRMJobSuspend(J,Msg,SC) == FAILURE)
        {
        /* cannot suspend job */
 
        return(FAILURE);
        }

      MJobSetState(J,mjsSuspended);

      /* NOTE:  to prevent thrashing, by default, suspended jobs should not *
       * resume for MDEF_MINSUSPENDTIME */

      {
      long tmpL;

      tmpL = MSched.Time + MDEF_MINSUSPENDTIME;

      MJobSetAttr(J,mjaSysSMinTime,(void **)&tmpL,mdfLong,mSet);
      }  /* END BLOCK */
 
      if (J->R != NULL)
        {
        MResDestroy(&J->R);

        MResAdjustDRes(J->Name,0);
        }
 
      break;

    case mppOvercommit:

      /* do nothing */

      /* let RM/OS handle all preemption */

      break;

    default:

      /* no policy specified */

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(MSched.PreemptPolicy) */

  RQ = J->Req[0];  /* FIXME */
 
  for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
    {
    N = J->NodeList[nindex].N;
 
    if ((RQ->NAccessPolicy == mnacSingleJob) ||
        (RQ->NAccessPolicy == mnacSingleTask))
      {
      MCResRemove(&N->DRes,&N->CRes,&N->CRes,1,TRUE);

      MCResAdd(&N->ARes,&N->CRes,&N->CRes,1,FALSE);
      }
    else
      {
      TC = J->NodeList[nindex].TC;
 
      MCResRemove(&N->DRes,&N->CRes,&RQ->DRes,TC,TRUE);
 
      MCResAdd(&N->ARes,&N->CRes,&RQ->DRes,TC,FALSE);
      }
 
    /* adjust node state */
 
    if (N->ARes.Procs == 0)
      N->EState = mnsBusy;
    else if (N->ARes.Procs == N->CRes.Procs)
      N->EState = mnsIdle; 
    else
      N->EState = mnsActive;

    if (MSched.Mode == msmSim) 
      N->State = N->EState;
    }  /* END for (nindex) */
 
  /* release allocation manager reservation */
 
  if (J->Cred.A != NULL)
    {
    if (MAMAllocResCancel(J->Cred.A->Name,J->Name,"job suspended",NULL,(enum MHoldReasonEnum *)SC) == FAILURE)
      {
      DBG(1,fRM) DPrint("ERROR:    cannot cancel account reservation for job '%s'\n",
        J->Name);
      }
    }
 
  /* update preemption stats */
 
  MStat.TotalPreemptJobs++;
  MStat.TotalPHPreempted += J->PSDedicated;

  J->PreemptCount ++;
 
  J->PSDedicated = 0.0;
  J->PSUtilized  = 0.0;
  J->MSUtilized  = 0.0;
  J->MSDedicated = 0.0;
 
  if (MSched.Mode == msmSim)
    { 
    if (J->SpecFlags & (1 << mjfPreemptee))
      {
      /* OUCH:  remove after initial research */
 
      J->SpecFlags ^= (1 << mjfPreemptee);
      }
    }
 
  MJobUpdateFlags(J);
 
  return(SUCCESS);
  }  /* END MJobPreempt() */




int MJobResume(

  mjob_t *J,   /* I */
  char   *Msg, /* O (optional) */
  int    *SC)  /* O (optional) */

  {
  const char *FName = "MJobResume";

  DBG(3,fSCHED) DPrint("%s(%s,Msg,SC)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if ((J == NULL) || (J->State != mjsSuspended))
    {
    /* verify job state */

    sprintf(Msg,"cannot resume non-suspended job");

    return(FAILURE);
    }

  if (MRMJobResume(J,Msg,SC) == FAILURE)
    {
    /* cannot resume job */

    return(FAILURE);
    }

  MJobSetState(J,mjsRunning);

  MQueueAddAJob(J);

  return(SUCCESS);
  }  /* END MJobResume() */





int MJobMkTemp(

  mjob_t    *J,           /* I/O (modified) */ 
  mreq_t    *RQ,          /* I */
  macl_t    *ACL,         /* I */
  macl_t    *CL,          /* I */
  mnalloc_t *JNodeList,   /* I */
  mnalloc_t *RQNodeList)  /* I */

  {
  if ((J == NULL) || (RQ == NULL))
    {
    return(FAILURE);
    }

  memset(J,0,sizeof(mjob_t));

  memset(RQ,0,sizeof(mreq_t));

  J->Req[0]      = RQ;     

  RQ->DRes.Procs = 1;

  J->MReq        = NULL;

  RQ->RMIndex    = 0;

  J->Index       = -1;

  J->State       = mjsIdle;

  J->CMaxTime    = MAX_MTIME;
  J->TermTime    = MAX_MTIME;

  J->RM          = &MRM[0];

  if (CL != NULL)
    {
    J->Cred.CL = CL;

    memset(CL,0,sizeof(macl_t) * MAX_MACL);
    }

  if (ACL != NULL)
    {
    J->Cred.ACL = ACL;
 
    memset(ACL,0,sizeof(macl_t) * MAX_MACL);
    }

  if (JNodeList != NULL)
    {
    J->NodeList = JNodeList;

    J->NodeList[0].N = NULL;
    }

  if (RQNodeList != NULL)
    {
    RQ->NodeList = RQNodeList;
 
    RQ->NodeList[0].N = NULL;
    }

  MJobSetAttr(J,mjaPAL,NULL,mdfString,mSet);

  MJobSetQOS(J,&MQOS[0],0);       

  MJobBuildCL(J);       

  return(SUCCESS);
  }  /* END MJobMkTemp() */




int MJobFind(

  char    *SpecJobName, /* I: job name    */
  mjob_t **JP,          /* O: job pointer */
  int      Mode)        /* I: search mode */

  {
  int index;
 
  unsigned long hashkey;

  mjob_t *tmpJ;

# if defined(__MLL) || defined(__MLL2) || defined(__MLL31)
  int count;

  char *split;

# endif /* __MLL || __MLL2 || _MLL31 */

  char QJobID[MAX_MNAME];
  char QHostName[MAX_MNAME];

  char JobName[MAX_MNAME];

  const char *FName = "MJobFind";

  DBG(5,fSTRUCT) DPrint("%s('%s',J,%d)\n",
    FName,
    (SpecJobName != NULL) ? SpecJobName : "NULL",
    Mode);

  if (JP != NULL)
    *JP = NULL;

  if ((SpecJobName == NULL) || (SpecJobName[0] == '\0'))
    {
    return(FAILURE);
    }

  MJobGetName(NULL,SpecJobName,&MRM[0],JobName,sizeof(JobName),mjnShortName);

  /* change to (Mode == 1) to restore */

  QHostName[0] = '\0';

  if (Mode == 1)
    {
#   if defined(__MLL) || defined(__MLL2) || defined(__MLL31)

    {
    char *ptr;

    /* FORMAT:  <SUBMITHOST>[.<DOMAIN>].<JOBID>.<STEPID> */

    count = 0;

    for (ptr = JobName + strlen(JobName) - 1;*ptr != '\0';ptr--)
      {
      if (*ptr == '.')
        {
        count++;

        if (count == 2)
          split = ptr;
        }
      }

    if (count > 2)
      {
      /* fully qualified submit hostname specified */

      MUStrCpy(QJobID,JobName,sizeof(QJobID));
      }
    else
      {
      /* non-fully qualified submit hostname specified */
      /* insert DefaultDomain                          */

      MUStrCpy(QJobID,JobName,MIN(sizeof(QJobID),split - JobName + 1));

      strcat(QJobID,MSched.DefaultDomain);

      strcat(QJobID,split);
      }
    }

#   elif defined(__MPBS)

    {
    char *ptr;

    /* try PBS format name */

    /* FORMAT:  <JOBID>[.<SUBMITHOST>.<DOMAIN>] */

    if ((ptr = strchr(JobName,'.')) != NULL)
      {
      MUStrCpy(QJobID,JobName,MIN(sizeof(QJobID),ptr - JobName + 1));
      MUStrCpy(QHostName,ptr,sizeof(QHostName));
      }
    else
      {
      MUStrCpy(QJobID,JobName,sizeof(QJobID));
      MUStrCpy(QHostName,MSched.DefaultQMHost,sizeof(QHostName));
      }
    }

#   else /* __MPBS */

    MUStrCpy(QJobID,JobName,sizeof(QJobID));

#   endif /* else defined(__MLL) || defined(__MLL2) || defined(__MLL31) */
    }
  else
    {
    MUStrCpy(QJobID,JobName,sizeof(QJobID));
    }

  hashkey = MUGetHash(QJobID) % MAX_MJOB;

  DBG(6,fSTRUCT) DPrint("INFO:     job '%s'  hash %ld\n",
    JobName,
    hashkey);

  for (index = hashkey;index < MSched.M[mxoJob] + MAX_MHBUF;index++)
    {
    /* if name is already in hash table... */

    if (!strcmp(MJobName[index].Name,QJobID))
      {
      tmpJ = MJob[MJobName[index].Index];

      if (JP != NULL)
        *JP = tmpJ;

      if ((tmpJ == NULL) || 
         ((tmpJ->Name[0] != '\0') && (tmpJ->Name[0] != '\1') && strcmp(tmpJ->Name,QJobID)))
        {
        /* hash table is corrupt */

        DBG(7,fSTRUCT) DPrint("WARNING:  job '%s' corruption found at hash[%d] %d '%s' (J->Name: %s)\n",
          QJobID,
          index,
          MJobName[index].Index,
          MJobName[index].Name,
          (tmpJ->Name[0] != '\0') ? tmpJ->Name : "[EMPTY]");

        /* MJobName[index].Name[0] = '\1'; */            

        return(FAILURE);
        }

      DBG(6,fSTRUCT) DPrint("INFO:     job '%s' found at hash[%d] %d '%s' (J->Name: %s)\n",
        QJobID,
        index,
        MJobName[index].Index,
        MJobName[index].Name,
        (tmpJ->Name[0] != '\0') ? tmpJ->Name : "[EMPTY]");

      return(SUCCESS);
      }

    if ((MJobName[index].Name[0] == '\0') &&
        (MJobName[index].Index == 0))
      {
      /* if location is empty... */               

      return(FAILURE);
      }
    }    /* END for (index) */

  /* reached end of table */

  return(FAILURE);
  }  /* END MJobFind() */




int MJobBuildCL(
 
  mjob_t *J)  /* I */
 
  {
  int aindex;
  int cindex;
 
  if (J == NULL)
    {
    return(FAILURE);
    }

  /* NOTE:  ACL info used by both job and job reservations */
 
  if (J->Cred.CL == NULL)
    J->Cred.CL = (macl_t *)calloc(1,sizeof(macl_t) * MAX_MACL);

  if (J->Cred.ACL == NULL)
    J->Cred.ACL = (macl_t *)calloc(1,sizeof(macl_t) * MAX_MACL);      

  if (J->ResName[0] != '\0')
    MACLSet(J->RCL,maRes,J->ResName,mcmpSEQ,nmPositiveAffinity,(1 << maclRequired),0);

  aindex = 0;
 
  /* insert user, group, account, QOS, and class cred info */

  J->Cred.CL[aindex].Type     = maJob;
  MUStrCpy(J->Cred.CL[aindex].Name,J->Name,MAX_MNAME);
  J->Cred.CL[aindex].Cmp      = mcmpSEQ;
  J->Cred.CL[aindex].Affinity = nmPositiveAffinity;
 
  aindex++;
 
  if (J->Cred.U != NULL)
    {
    J->Cred.CL[aindex].Type     = maUser;
    MUStrCpy(J->Cred.CL[aindex].Name,J->Cred.U->Name,MAX_MNAME);
    J->Cred.CL[aindex].Cmp      = mcmpSEQ;  
    J->Cred.CL[aindex].Affinity = nmPositiveAffinity; 

    aindex++;
    }
 
  if (J->Cred.G != NULL)
    {
    J->Cred.CL[aindex].Type     = maGroup;
    MUStrCpy(J->Cred.CL[aindex].Name,J->Cred.G->Name,MAX_MNAME);
    J->Cred.CL[aindex].Cmp      = mcmpSEQ;  
    J->Cred.CL[aindex].Affinity = nmPositiveAffinity;        
 
    aindex++;
    }
 
  if (J->Cred.A != NULL)
    {
    J->Cred.CL[aindex].Type     = maAcct;
    MUStrCpy(J->Cred.CL[aindex].Name,J->Cred.A->Name,MAX_MNAME);
    J->Cred.CL[aindex].Cmp      = mcmpSEQ;  
    J->Cred.CL[aindex].Affinity = nmPositiveAffinity;        
 
    aindex++;
    }
 
  if ((J->Cred.Q != NULL) && (J->Cred.Q->Name[0] != '\0') && (strcmp(J->Cred.Q->Name,DEFAULT)))
    { 
    J->Cred.CL[aindex].Type     = maQOS;
    MUStrCpy(J->Cred.CL[aindex].Name,J->Cred.Q->Name,MAX_MNAME);
    J->Cred.CL[aindex].Cmp      = mcmpSEQ;  
    J->Cred.CL[aindex].Affinity = nmPositiveAffinity;        
 
    aindex++;
    }
 
  if (J->Cred.C != NULL)
    { 
    J->Cred.CL[aindex].Type     = maClass;
    MUStrCpy(J->Cred.CL[aindex].Name,J->Cred.C->Name,MAX_MNAME);
    J->Cred.CL[aindex].Cmp      = mcmpSEQ;  
    J->Cred.CL[aindex].Affinity = nmPositiveAffinity;        
 
    aindex++;
    }
  else if (J->Req[0] != NULL)
    {
    /* potentially multiple class access entries per job */

    for (cindex = 1;cindex < MAX_MCLASS;cindex++)
      {
      if (J->Req[0]->DRes.PSlot[cindex].count > 0)
        {
        J->Cred.CL[aindex].Type     = maClass;
        MUStrCpy(J->Cred.CL[aindex].Name,MAList[eClass][cindex],MAX_MNAME);
        J->Cred.CL[aindex].Cmp      = mcmpSEQ;  
        J->Cred.CL[aindex].Affinity = nmPositiveAffinity;        
 
        aindex++;
        }
      }    /* END for (cindex) */
    }

  /* add reservation access info */

  for (cindex = 0;J->RAList[cindex][0] != '\0';cindex++)
    {
    J->Cred.CL[aindex].Type     = maRes;
    MUStrCpy(J->Cred.CL[aindex].Name,J->RAList[cindex],MAX_MNAME);
    J->Cred.CL[aindex].Cmp      = mcmpSEQ;  
    J->Cred.CL[aindex].Affinity = nmPositiveAffinity;
 
    aindex++;
    }

  if (J->SpecWCLimit[0] > 0)
    {
    J->Cred.CL[aindex].Type     = maDuration;
    J->Cred.CL[aindex].Value    = J->SpecWCLimit[0];
    J->Cred.CL[aindex].Cmp      = mcmpEQ;  
    J->Cred.CL[aindex].Affinity = nmPositiveAffinity;

    aindex++;
    }

  if (J->Request.TC > 0)
    {
    J->Cred.CL[aindex].Type     = maProc;
    J->Cred.CL[aindex].Value    = MJobGetProcCount(J);
    J->Cred.CL[aindex].Cmp      = mcmpEQ;
    J->Cred.CL[aindex].Affinity = nmPositiveAffinity;
 
    aindex++;
    }

  /* add job attr info */

  for (cindex = 0;cindex < 32;cindex++)
    {
    if (J->AttrBM & (1 << cindex))
      {
      J->Cred.CL[aindex].Type     = maJFeature;
      MUStrCpy(J->Cred.CL[aindex].Name,(char *)MAList[eJFeature][cindex],MAX_MNAME);
      J->Cred.CL[aindex].Cmp      = mcmpSEQ;
      J->Cred.CL[aindex].Affinity = nmPositiveAffinity;
 
      aindex++;
      }
    }    /* END for (cindex) */

  J->Cred.CL[aindex].Type = maNONE;
 
  return(SUCCESS);
  }  /* MJobBuildCL() */ 




int MJobCheckLimits(

  mjob_t  *J,       /* I */
  int      PLevel,
  mpar_t  *P,
  int      Mode,
  char    *Message)

  {
  int JUsage[MAX_MPOLICY];      

  const char *MType[] = {
    "active",
    "idle",
    "system",
    NULL };

  int OList1[] = { mxoNONE, mxoUser, -1 };       
  int OList2[] = { mxoUser, mxoGroup, mxoAcct, mxoClass, mxoQOS, mxoPar, -1 };
  int PList[]  = { mptMaxJob, mptMaxProc, mptMaxNode, mptMaxPS, mptMaxPE, mptMaxWC, -1 };
  int MList[]  = { mlActive, mlIdle, mlSystem, -1 };

  int oindex1;
  int oindex2;

  int pindex;
  int mindex;

  int tmpI;

  mpu_t *OP;
  mpu_t *DP;
  mpu_t *QP;

  char *OID;

  double PE;

  /* Modes:  active, system, queue */

  const char *FName = "MJobCheckLimits";

  DBG(7,fSTRUCT) DPrint("%s(%s,%s,P,%d,Message)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    MPolicyMode[PLevel],
    Mode);

  if (J == NULL)
    {
    return(FAILURE);
    }

  if (Message != NULL)
    Message[0] = '\0';

  if (PLevel == ptOFF)
    {
    /* ignore policies */

    return(SUCCESS);
    }

  /* determine resource consumption */
 
  JUsage[mptMaxJob]  = 1;
  JUsage[mptMaxProc] = MJobGetProcCount(J);
  JUsage[mptMaxNode] = J->Request.NC;
  JUsage[mptMaxWC]   = J->WCLimit;

  if ((J->Req[0]->NAccessPolicy == mnacSingleJob) &&
      (MSched.NodeAllocMaxPS == TRUE))
    {
    JUsage[mptMaxPS] = J->NodesRequested * JUsage[mptMaxWC];
    }
  else
    {
    JUsage[mptMaxPS] = JUsage[mptMaxProc] * JUsage[mptMaxWC];
    }

  MJobGetPE(J,&MPar[0],&PE);      

  JUsage[mptMaxPE] = (int)PE;
  
  for (mindex = 0;MList[mindex] != -1;mindex++)
    {
    if (!(Mode & (1 << MList[mindex])))
      continue;

    /* limits: user, group, account, class, override w/QOS */

    if (J->Cred.Q != NULL)
      {
      if (MList[mindex] == mlActive)
        QP = J->Cred.Q->L.OAP;
      else if (MList[mindex] == mlIdle)
        QP = J->Cred.Q->L.OIP;
      else if (MList[mindex] == mlSystem)
        QP = J->Cred.Q->L.OJP;
      else
        QP = NULL;
      }
    else
      {
      QP = NULL;
      }

    for (oindex1 = 0;OList1[oindex1] != -1;oindex1++)
      {
      for (oindex2 = 0;OList2[oindex2] != -1;oindex2++)
        {
        if (((OList2[oindex2] == mxoPar) && (MList[mindex] != mlSystem)) ||
            ((OList2[oindex2] != mxoPar) && (MList[mindex] == mlSystem)))
          {
          continue;
          }    

        if ((OList1[oindex1] != mxoNONE) && (OList2[oindex2] != mxoClass))
          {
          /* TEMP:  only MAXJOB[user,class] enabled */

          continue;
          }
      
        OP = NULL;
        DP = NULL;

        OID = NULL;

        switch(OList2[oindex2])
          {
          case mxoUser:

            if (J->Cred.U != NULL)
              {
              switch(OList1[oindex1])
                {
                case mxoNONE:

                  if (MList[mindex] == mlActive)
                    OP = &J->Cred.G->L.AP;
                  else if (MList[mindex] == mlIdle)
                    OP = J->Cred.G->L.IP;

                  break;

                case mxoGroup:

                  if ((MList[mindex] == mlActive) &&
                      (J->Cred.U->L.APG != NULL) &&
                      (J->Cred.G != NULL))
                    {
                    OP = &J->Cred.U->L.APG[J->Cred.G->Index];
                    }

                  break;

                default:

                  /* NO-OP */

                  break;
                }  /* END switch(OList1[oindex1]) */

              if (MList[mindex] == mlActive)
                OP = &J->Cred.U->L.AP;
              else if (MList[mindex] == mlIdle)
                OP = J->Cred.U->L.IP;

              OID = J->Cred.U->Name;
              }  /* END if (J->Cred.U != NULL) */

            if (MSched.DefaultU != NULL)
              {
              if (MList[mindex] == mlActive)
                DP = &MSched.DefaultU->L.AP;
              else if (MList[mindex] == mlIdle)
                DP = MSched.DefaultU->L.IP;
              }

            break;

          case mxoGroup:

            if (J->Cred.G != NULL)
              {
              switch(OList1[oindex1])
                {
                case mxoNONE:

                  if (MList[mindex] == mlActive)
                    OP = &J->Cred.G->L.AP;
                  else if (MList[mindex] == mlIdle)
                    OP = J->Cred.G->L.IP;

                  break;

                case mxoClass:

                  if ((MList[mindex] == mlActive) &&
                      (J->Cred.G->L.APC != NULL) &&
                      (J->Cred.C != NULL))
                    {
                    OP = &J->Cred.G->L.APC[J->Cred.C->Index];
                    }

                  break;

                case mxoQOS:

                  if ((MList[mindex] == mlActive) &&
                      (J->Cred.G->L.APQ != NULL) &&
                      (J->Cred.Q != NULL))
                    {
                    OP = &J->Cred.G->L.APQ[J->Cred.Q->Index];
                    }

                  break;

                default:

                  /* NO-OP */

                  break;
                }  /* END switch(OList1[oindex1]) */

              OID = J->Cred.G->Name;
              }  /* END if (J->Cred.G != NULL) */

            if (MSched.DefaultG != NULL)
              {
              if (MList[mindex] == mlActive)
                DP = &MSched.DefaultG->L.AP;
              else if (MList[mindex] == mlIdle)
                DP = MSched.DefaultG->L.IP;
              }
 
            break;
 
          case mxoAcct:

            if (J->Cred.A != NULL)
              {
              if (MList[mindex] == mlActive)
                OP = &J->Cred.A->L.AP;
              else if (MList[mindex] == mlIdle)
                OP = J->Cred.A->L.IP;
 
              OID = J->Cred.A->Name;
              }
 
            if (MSched.DefaultA != NULL)
              {
              if (MList[mindex] == mlActive)
                DP = &MSched.DefaultA->L.AP;
              else if (MList[mindex] == mlIdle)
                DP = MSched.DefaultA->L.IP;
              }

            break;

          case mxoQOS:
 
            if (J->Cred.Q != NULL)
              {
              switch(OList1[oindex1])
                {
                case mxoNONE:

                  if (MList[mindex] == mlActive)
                    OP = &J->Cred.Q->L.AP;
                  else if (MList[mindex] == mlIdle)
                    OP = J->Cred.Q->L.IP;

                  break;

                case mxoUser:

                  if ((MList[mindex] == mlActive) &&
                      (J->Cred.Q->L.APU != NULL) &&
                      (J->Cred.U != NULL))
                    {
                    OP = &J->Cred.Q->L.APU[J->Cred.U->Index];
                    }

                  break;

                default:

                  /* NO-OP */

                  break;
                }  /* END switch(OList1[oindex1]) */

              if (MList[mindex] == mlActive)
                OP = &J->Cred.Q->L.AP;
              else if (MList[mindex] == mlIdle)
                OP = J->Cred.Q->L.IP;
 
              OID = J->Cred.Q->Name;
              }
 
            if (MSched.DefaultQ != NULL)
              {
              if (MList[mindex] == mlActive)
                DP = &MSched.DefaultQ->L.AP;
              else if (MList[mindex] == mlIdle)
                DP = MSched.DefaultQ->L.IP;
              }
 
            break;

          case mxoClass:

            if (J->Cred.C != NULL)
              {
              switch(OList1[oindex1])
                {
                case mxoNONE:

                  if (MList[mindex] == mlActive)
                    OP = &J->Cred.C->L.AP;
                  else if (MList[mindex] == mlIdle)
                    OP = J->Cred.C->L.IP;

                  break;

                case mxoUser:

                  if ((MList[mindex] == mlActive) && 
                      (J->Cred.C->L.APU != NULL) &&
                      (J->Cred.U != NULL))
                    {
                    OP = &J->Cred.C->L.APU[J->Cred.U->Index];
                    }

                  break;

                default:

                  /* NO-OP */

                  break;
                }  /* END switch(OList1[oindex1]) */
 
              OID = J->Cred.C->Name;
              }  /* END if (J->Cred.C != NULL) */
 
            if (MSched.DefaultC != NULL)
              {
              if (MList[mindex] == mlActive)
                DP = &MSched.DefaultC->L.AP;
              else if (MList[mindex] == mlIdle)
                DP = MSched.DefaultC->L.IP;
              }

            break;

          case mxoPar:

            if (P != NULL)
              {
              if (MList[mindex] == mlActive)
                OP = &P->L.AP;
              else if (MList[mindex] == mlIdle)
                OP = P->L.IP;
              else if (MList[mindex] == mlSystem)
                OP = P->L.JP;
 
              OID = P->Name;
              }
 
            if (MList[mindex] == mlActive)
              DP = &MPar[0].L.AP;
            else if (MList[mindex] == mlIdle)
              DP = MPar[0].L.IP;
            else if (MList[mindex] == mlSystem)
              DP = MPar[0].L.JP;

            break;         
          }  /* END switch(OList2[oindex2]) */

        if (OP == NULL)
          continue;

        for (pindex = 0;PList[pindex] != -1;pindex++)
          {
          if (MPolicyCheckLimit(
                JUsage[PList[pindex]],
                PList[pindex],
                PLevel,
                0, /* NOTE:  change to P->Index for per partition limits */
                OP,
                DP,
                QP,
                &tmpI) == FAILURE)
            {
            if (Message != NULL)
              {
              sprintf(Message,"job %s violates %s %s %s limit of %d for %s %s %s (R: %d, U: %d)\n",
                J->Name,
                MType[mindex],
                MPolicyMode[PLevel],   
                MPolicyType[PList[pindex]],
                tmpI,
                MXO[OList2[oindex2]],
                (OID != NULL) ? OID : NONE,
                (oindex1 != mxoNONE) ? MXO[OList1[oindex1]] : "",
                JUsage[PList[pindex]],
		OP->Usage[PList[pindex]][0]);
              }

            DBG(3,fSCHED) DPrint("job %s violates %s %s %s limit of %d for %s %s %s (Req: %d, InUse: %d)\n",
              J->Name,
              MType[mindex],
              MPolicyMode[PLevel],
              MPolicyType[PList[pindex]],
              tmpI,
              MXO[OList2[oindex2]],
              (OID != NULL) ? OID : NONE,
              (oindex1 != mxoNONE) ? MXO[OList1[oindex1]] : "",
              JUsage[PList[pindex]],
	      OP->Usage[PList[pindex]][0]);

            if (PLevel == ptHARD && JUsage[PList[pindex]] > tmpI)
              {
              if (J->LastNotifyTime + MSched.ActionInterval < MSched.Time)
                {
                MSysRegEvent(Message,0,0,1);
                J->LastNotifyTime = MSched.Time;
                }
              }

            return(FAILURE);
            }  /* END if (MPolicyCheckLmit() == FAILURE) */
          }    /* END for (pindex) */
        }    /* END for (oindex2) */ 
      }      /* END for (oindex1) */
    }        /* END for (mindex) */

  return(SUCCESS);
  }  /* END MJobCheckLimits() */




int MJobSetHold(
 
  mjob_t               *J,            /* I */
  int                   HoldTypeBM,   /* I (BM) */
  long                  HoldDuration, /* I */
  enum MHoldReasonEnum  HoldReason,   /* I */
  char                 *HoldMsg)      /* I */
 
  {
  char     Line[MAX_MLINE];

  const char *FName = "MJobSetHold";
 
  if (MSched.Mode == msmSim) 
    {
    DBG(0,fSCHED) DPrint("%s(%s,%d,%s,%s,%s)\n",
      FName,
      (J != NULL) ? J->Name : "NULL",
      HoldTypeBM,
      MULToTString(HoldDuration),
      MDefReason[HoldReason],
      (HoldMsg != NULL) ? HoldMsg : "NULL");
    }
  else
    {
    DBG(3,fSCHED) DPrint("%s(%s,%d,%s,%s,%s)\n",
      FName,
      (J != NULL) ? J->Name : "NULL",
      HoldTypeBM,
      MULToTString(HoldDuration),
      MDefReason[HoldReason],
      (HoldMsg != NULL) ? HoldMsg : "NULL");
    }
 
  if ((J == NULL) || (J->Name[0] == '\0'))
    {
    /* job has been removed */
 
    return(SUCCESS);
    }
 
  if ((J->ResName[0] != '\0') && (HoldReason == mhrNoResources))
    {
    DBG(2,fSCHED) DPrint("INFO:     job %s requests reservation %s.  (not deferring)\n",
      J->Name, 
      J->ResName);
 
    return(SUCCESS);
    }

  {
  mrm_t *RM = (J->RM != NULL) ? J->RM : &MRM[0];
 
  if ((RM->FailIteration == MSched.Iteration) &&
      (HoldReason == mhrNoResources))
    {
    DBG(2,fSCHED) DPrint("INFO:     connection to RM %s failed.  Not deferring job %s for reason %s\n",
      RM->Name,
      J->Name,
      MDefReason[HoldReason]);
 
    return(SUCCESS);
    }
  }    /* END BLOCK */

  if ((HoldTypeBM == (1 << mhDefer)) && (MSched.DeferTime <= 0))
    {
    DBG(2,fSCHED) DPrint("INFO:     defer disabled\n");
 
    return(SUCCESS);
    }
 
  if (J->Hold & (1 << mhBatch))
    {
    DBG(2,fSCHED) DPrint("INFO:     job '%s' already on batch hold\n",
      J->Name);
 
    return(SUCCESS);
    }
 
  DBG(2,fSCHED) DPrint("ALERT:    job '%s' cannot run (deferring job for %ld seconds)\n",
    J->Name,
    HoldDuration);
 
  MUStrDup(&J->Message,HoldMsg); 
 
  DBG(2,fSCHED)
    MJobShow(J,0,NULL);
 
  if ((MSched.Mode == msmSim) &&
      (HoldDuration != MSched.RMPollInterval) &&
      (getenv(MSCHED_ENVAMTESTVAR) == NULL))
    {
    DBG(0,fSCHED) DPrint("INFO:     job '%s' removed on iteration %d (cannot run in simulated environment), reason: %s\n",
      J->Name,
      MSched.Iteration,
      MDefReason[HoldReason]);
 
    MStat.EligibleJobs--;
 
    MPolicyAdjustUsage(NULL,J,NULL,mlIdle,MPar[0].L.IP,-1,-1,NULL);
 
    MJobRemove(J);
 
    return(SUCCESS);
    }
 
  if (J->R != NULL)
    MResDestroy(&J->R);

  if (HoldTypeBM == 0)
    J->Hold = 0;
  else 
    J->Hold |= HoldTypeBM;

  J->HoldReason = HoldReason;    

  if (HoldTypeBM & (1 << mhDefer))
    {
    J->EState       = mjsDeferred;
    J->SyncDeadLine = MSched.Time + HoldDuration;

    J->DeferCount++;
    }
 
  /* if job has failed too many times */
 
  if ((MSched.DeferCount > 0) && 
      (J->DeferCount > MSched.DeferCount))
    {
    DBG(1,fSCHED) DPrint("INFO:     batch hold placed on job '%s', reason: '%s'\n",
      J->Name, 
      MDefReason[HoldReason]);
 
    J->Hold |= (1 << mhBatch);
 
    sprintf(Line,"JOBHOLD:  batch hold placed on job '%s'.  defercount: %d  reason: '%s'",
      J->Name,
      J->DeferCount,
      MDefReason[HoldReason]);

    MSysRegEvent(Line,0,0,1); 
    }
  else
    {
    if ((HoldReason == mhrRMReject) || (HoldReason == mhrAPIFailure))
      {
      sprintf(Line,"JOBDEFER:  defer hold placed on job '%s'.   reason: '%s'",
        J->Name,
        MDefReason[HoldReason]);
      
      MSysRegEvent(Line,0,0,1);
      }
    }
  
  return(SUCCESS);
  }  /* END MJobSetHold() */




int MJobTestRMExtension(

  char *XString)  /* I */

  {
  mjob_t tmpJ;

  macl_t    tmpACL[MAX_MACL];
  macl_t    tmpCL[MAX_MACL];

  mreq_t    tmpRQ;

  mnalloc_t tmpNodeList[MAX_MNODE + 1];

  MJobMkTemp(&tmpJ,&tmpRQ,tmpACL,tmpCL,NULL,tmpNodeList);

  if (MJobProcessExtensionString(&tmpJ,XString) == SUCCESS)
    {
    /* test succeeded */

    exit(0);
    }

  exit(1);

  /*NOTREACHED*/

  return(SUCCESS);
  }  /* END MJobTestRMExtension() */




int MJobProcessExtensionString(

  mjob_t *J,        /* I (modified) */
  char   *XString)  /* I */
 
  {
  char   *ptr;
  char   *tail;
  char   *key;
 
  int     index;
  int     aindex;
  int     findex;
  int     nindex;
  int     tindex;
 
  char    tmpLine[MAX_MLINE << 2];
 
  char    MemCmp[3];
 
  mreq_t  *tmpRQPtr;
  mreq_t  *RQ;
 
  mnode_t *N;
 
  char   *Value;
 
  char   *TokPtr;
  char   *TokPtr2;

  const char *FName = "MJobProcessExtensionString";

  DBG(6,fCONFIG) DPrint("%s(%s,%s)\n",
    FName,
    (J != NULL) ? J->Name : NULL,
    (XString != NULL) ? XString : "NULL");

  if (J == NULL)
    {
    return(FAILURE);
    }

  if ((XString == NULL) || (XString[0] == '\0'))
    {
    DBG(6,fCONFIG) DPrint("INFO:     extension string is empty\n");
 
    return(SUCCESS);
    } 

  /* FORMAT:  [<WS>]<ATTR>=<VALUE>[<WS>];... */
  /* FORMAT:  <ATTR>:<VALUE>[;<ATTR>:<VALUE>]... */

  /* NOTE:  handle '=' delimiters for backwards compatibility */

  /* set defaults */
 
  RQ = J->Req[0];

  key = MUStrTok(XString,";\n",&TokPtr);

  /* remove leading whitespace */

  while (isspace(*key) || (*key == '\"') || (*key == '\"'))
    {
    key++;
    }

  while (key != NULL)
    {
    if ((aindex = MUGetIndex(key,MRMXAttr,TRUE,mxaNONE)) == mxaNONE)
      {
      if (X.XJobProcessRMXString != (int (*)())0)
        {
        (*X.XJobProcessRMXString)(X.xd,J,key);
        }

      key = MUStrTok(NULL,";\n",&TokPtr);       

      continue;
      }

    Value = key + strlen(MRMXAttr[aindex]);

    /* remove leading whitespace */

    while (isspace(*Value) || (*Value == '\"') || (*Value == '\"'))
      {
      Value++;
      }

    /* remove separation character */

    while ((*Value == '=') || (*Value == ':'))
      {
      Value++;
      }

    while (isspace(*Value) || (*Value == '\"') || (*Value == '\"'))
      {
      Value++;
      }

    /* remove trailing whitespace */

    for (index = strlen(Value) - 1;index > 0;index--)
      {
      if (!isspace(Value[index]))
        break;

      Value[index] = '\0';
      }  /* END for (index) */

    switch(aindex)
      {
      case mxaAdvRes:

        /* advance reservation flag/value */

        /* FORMAT:  [<RESID>] */

        J->SpecFlags |= (1 << mjfAdvReservation);

        if (Value[0] != '\0')
          MUStrCpy(J->ResName,Value,sizeof(J->ResName));

        break;

      case mxaDDisk:

        /* dedicated disk */

        RQ->DRes.Disk = (int)MURSpecToL(Value,mvmMega,mvmByte);

        if (Value[0] != '0')
          RQ->DRes.Disk = MAX(RQ->DRes.Disk,1);

        DBG(5,fCONFIG) DPrint("INFO:     per task dedicated disk set to %d MB\n",
          RQ->DRes.Disk);

        break;

      case mxaDMem:

        /* dedicated memory */
 
        RQ->DRes.Mem = (int)strtol(Value,NULL,10);
 
        DBG(5,fCONFIG) DPrint("INFO:     per task dedicated memory set to %d MB\n",
          RQ->DRes.Mem);

        break;

      case mxaFlags:

        /* job flags */

        /* FORMAT:  <JOBFLAG>[,<JOBFLAG>]... */

        MUStrCpy(tmpLine,Value,sizeof(tmpLine));

        ptr = MUStrTok(tmpLine,",",&TokPtr2);

        while (ptr != NULL)
          {
          if ((findex = MUGetIndex(ptr,MJobFlags,1,mjfNONE)) == mjfNONE)
            {
            ptr = MUStrTok(NULL,",",&TokPtr2);         

            continue;
            }
     
          J->SpecFlags |= (1 << findex);    
      
          DBG(5,fCONFIG) DPrint("INFO:     flag %s set for job %s\n",
            MJobFlags[findex],
            J->Name);
 
          Value = ptr + strlen(MJobFlags[findex]);

          switch(findex)
            {
            case mjfAdvReservation:
 
              if (Value[0] == ':') 
                {
                sscanf((Value + 1),"%s",
                  J->ResName);
 
                DBG(5,fCONFIG) DPrint("INFO:     reservation %s selected\n",
                  J->ResName);
                }
 
              break;
 
            case mjfNASingleJob:

              RQ->NAccessPolicy = mnacSingleJob;

              J->SpecFlags |= (1 << mjfNASingleJob);
     
              break;

            case mjfNASingleTask:

              RQ->NAccessPolicy = mnacSingleTask;

              J->SpecFlags |= (1 << mjfNASingleTask);

              break;

            default:
 
              /* NO-OP */
 
              break;
            }  /* END switch(findex) */

          ptr = MUStrTok(NULL,",",&TokPtr2);
          }    /* END while (ptr != NULL) */

        MJobUpdateFlags(J);

        break;

      case mxaGeometry:

        /* NYI */

        break;

      case mxaHostList:

        /* HOSTLIST */

        /* FORMAT:  <HOSTNAME>[{,:}<HOSTNAME>]... */

        if (MSched.Mode == msmProfile)
          break;
 
        J->SpecFlags |= (1 << mjfHostList);

        MJobUpdateFlags(J);
 
        if (J->ReqHList == NULL)
          J->ReqHList = (mnalloc_t *)calloc(1,sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1));
 
        MUStrCpy(tmpLine,Value,sizeof(tmpLine));
 
        ptr = MUStrTok(tmpLine,",:",&TokPtr2);
 
        nindex = 0;

        J->ReqHList[nindex].N = NULL;

        while (ptr != NULL)
          {
          if (MNodeFind(ptr,&N) == FAILURE)
            {
            DBG(1,fUI) DPrint("ALERT:    cannot locate node '%s' for job '%s' hostlist\n",
              ptr,
              J->Name);

            ptr = MUStrTok(NULL,",:",&TokPtr2);              

            continue;
            }
 
          if (J->ReqHList[nindex].N == N)
            {
            /* duplicate node detected */

            J->ReqHList[nindex].TC++;
            }
          else
            {
            /* new node found */

            if (J->ReqHList[nindex].N != NULL)
              nindex++;

            J->ReqHList[nindex].N = N;
 
            J->ReqHList[nindex].TC = 1;
            }
 
          ptr = MUStrTok(NULL,",:",&TokPtr2);
          }  /* END while (ptr != NULL) */
 
        J->ReqHList[nindex + 1].N = NULL;

        break;

      case mxaMasterFeature:

        /* FORMAT:  <FEATURE>[{, :}<FEATURE>]... */

        MUStrCpy(tmpLine,Value,sizeof(tmpLine));
 
        ptr = MUStrTok(tmpLine,": ,\t",&TokPtr);
 
        if (J->MReq == NULL)
          {
          if (MReqCreate(J,NULL,&RQ,FALSE) == FAILURE)
            {
            return(FAILURE);
            }

          RQ->RequiredMemory    = 0;
          RQ->MemCmp            = mcmpGE;
 
          memset(RQ->ReqFBM,0,sizeof(RQ->ReqFBM));
 
          RQ->TaskCount         = 1;
 
          memcpy(RQ->DRes.PSlot,J->Req[0]->DRes.PSlot,sizeof(RQ->DRes.PSlot));
 
          RQ->NAccessPolicy     = J->Req[0]->NAccessPolicy;
          RQ->TasksPerNode      = 1;
 
          J->Req[0]->TaskCount -= 1;

          J->Req[0]->TaskCount -= 1;
 
          J->MReq               = RQ;
          }
        else
          {
          RQ = J->MReq;
          }
 
        while (ptr != NULL)
          {
          MUGetMAttr(eFeature,ptr,mAdd,RQ->ReqFBM,sizeof(RQ->ReqFBM));
 
          ptr = MUStrTok(NULL,": \t,",&TokPtr);
          }

        break;

      case mxaMasterMem:

        /* master node memory */

        /* FORMAT:  <CMP><MEMORY> */

        if (J->MReq == NULL)
          {
          if (MReqCreate(J,NULL,&RQ,FALSE) == FAILURE)
            {
            return(FAILURE);
            }

          RQ->TaskCount         = 1;
 
          memcpy(RQ->DRes.PSlot,J->Req[0]->DRes.PSlot,sizeof(RQ->DRes.PSlot));
 
          RQ->NAccessPolicy     = J->Req[0]->NAccessPolicy;

          RQ->TasksPerNode      = 1;
 
          J->Req[0]->TaskCount -= 1;
 
          J->MReq               = RQ;
          }
        else
          {
          RQ = J->MReq;
          }

        /* get memory comparison */
 
        ptr = Value;

        if ((Value[0] == '=') && isdigit(Value[1]))
          {
          RQ->MemCmp = mcmpEQ;
         
          ptr++;
          }
        else
          {
          /* determine memory comparison */
 
          MemCmp[0] = *ptr;
 
          ptr++;
 
          if (*ptr == '=')
            {
            MemCmp[1] = *ptr;
            MemCmp[2] = '\0';
 
            ptr++;
            }
          else
            {
            MemCmp[1] = '\0';
            }

          RQ->MemCmp = MUCmpFromString(MemCmp,NULL);  
          }
 
        RQ->RequiredMemory = atoi(ptr);

        break;

      case mxaNAccessPolicy:

        /* get node access mode */

        RQ->NAccessPolicy = MUGetIndex(Value,MNAccessPolicy,FALSE,MSched.DefaultNAccessPolicy);

        if (RQ->NAccessPolicy == mnacSingleJob)
          {
          J->SpecFlags |= (1 << mjfNASingleJob);
          }
        else if (RQ->NAccessPolicy == mnacSingleTask)
          {
          J->SpecFlags |= (1 << mjfNASingleTask);
          }

        DBG(5,fCONFIG) DPrint("INFO:     job node access policy set to %s\n",
          MNAccessPolicy[RQ->NAccessPolicy]);

        break;

      case mxaNAllocPolicy:

        /* FORMAT:  ??? */

        if ((RQ->NAllocPolicy == NULL) && 
           ((RQ->NAllocPolicy = (mnallocp_t *)calloc(1,sizeof(mnallocp_t))) == NULL))
          {
          /* cannot get memory for NAlloc structure */

          break;
          }
        
        RQ->NAllocPolicy->NAllocPolicy = 
          MUGetIndex(Value,MNAllocPolicy,FALSE,0);
 
        break;

      case mxaNodeSet:

        /* node set info */

        /* FORMAT:  <SETTYPE>:<SETATTR>[:<SETLIST>] */

        if ((ptr = MUStrTok(Value,":",&TokPtr2)) != NULL)
          {
          RQ->SetSelection = MUGetIndex(ptr,MResSetSelectionType,0,mrssNONE);

          if ((ptr = MUStrTok(NULL,":",&TokPtr2)) != NULL)
            {
            RQ->SetType = MUGetIndex(ptr,MResSetAttrType,0,mrstNONE);

            index = 0;

            while ((ptr = MUStrTok(NULL,":,",&TokPtr2)) != NULL)
              {
              MUStrDup(&RQ->SetList[index],ptr);

              index++;
              }
            }
          }

        break;

      case mxaPMask:

        /* partition mask */

        /* FORMAT:  <PARTITION>[:<PARTITION>]... */

        MUStrCpy(tmpLine,Value,sizeof(tmpLine));           
 
        if (strstr(tmpLine,MAList[ePartition][0]) == NULL)
          {
          int tmpI;

          tmpI = MUMAFromString(ePartition,tmpLine,mAdd);

          memcpy(&J->SpecPAL[0],&tmpI,sizeof(J->SpecPAL[0])); 

          /* if non-global pmask specified */

          if (J->SpecPAL[0] == 0)
            {
            DBG(1,fUI) DPrint("ALERT:    NULL pmask set for job '%s'.  (job cannot run)\n",
              J->Name);
            }
 
          DBG(3,fCONFIG)
            {
            MUStrCpy(
              tmpLine,
              MUListAttrs(ePartition,MUMAFromString(ePartition,tmpLine,mVerify)),
              sizeof(tmpLine));
   
            DPrint("INFO:     pmask '%s' specified.  resulting pmask: '%s'\n",
              Value,
              MUListAttrs(ePartition,J->SpecPAL[0]));
            }
          }    /* END if (strstr() == NULL) */

        break;

      case mxaPref:

        /* FORMAT:  PREF(FEATURE:X,FEATURE:Y) */

        /* translate to canonical pref spec */

        {
        char *ptr;
        int   index;

        char  tmpLine[MAX_MLINE];

        index = 0;

        tmpLine[0] = '\0';

        for (ptr = Value;*ptr != '\0';ptr++)
          {
          switch (*ptr)
            {
            case '(':
            case ')':

              /* ignore */
 
              break;

            case ':':
          
              tmpLine[index++] = '=';
              tmpLine[index++] = '=';

              break;

            case ',':

              tmpLine[index++] = '&';
              tmpLine[index++] = '&';

              break;
             
            default:

              tmpLine[index++] = *ptr;

              break;
            }  /* END switch(*ptr) */ 
          }    /* END for (ptr) */

        tmpLine[index] = '\0';

        MReqRResFromString(J,J->Req[0],tmpLine,mrmtWiki,TRUE);
        }  /* END BLOCK */

        DBG(3,fCONFIG) DPrint("INFO:     pref '%s' specified\n",
          Value);

        MSched.ResourcePrefIsActive = TRUE;

        break;

      case mxaQueueJob:

        /* get QUEUEJOB */

        if (MUBoolFromString(Value,TRUE) == FALSE)
          {
          J->SpecFlags |= (1 << mjfNoQueue);

          MJobUpdateFlags(J);

          DBG(4,fUI) DPrint("INFO:     NoQueue flag set for job %s\n",
            J->Name);
          }

        break;

      case mxaQOS:

        /* load QOS */

        if (MQOSFind(Value,&J->QReq) == SUCCESS)
          {
          DBG(3,fCONFIG) DPrint("INFO:     QOS '%s' requested by job %s\n",
            J->QReq->Name,
            J->Name);
          }
        else
          {
          DBG(3,fCONFIG) DPrint("WARNING:  job %s cannot request QOS '%s'\n",
            J->Name,
            Value);
     
          J->QReq = NULL;
          }

        break;

      case mxaSGE:

        /* get SGE request */

        /* FORMAT:  SGE=<WINDOWCOUNT>:<DISPLAY> */

        /* add requirement */

        if (MReqCreate(J,NULL,&tmpRQPtr,FALSE) == FAILURE)
          {
          return(FAILURE);
          }

        index = MUMAGetIndex(eGRes,"SGE",mAdd);

        tmpRQPtr->DRes.GRes[index].count = (unsigned short)strtol(Value,&tail,10);
        tmpRQPtr->TaskCount = 1;

        /* process display name */

        /* NYI */

        break;

      case mxaSID:

        /* get System ID */

        MUStrDup(&J->SystemID,Value);

        if (J->xd != NULL)
          {
          /* note:  set extension job attribute */

          /* NYI */
          }

        DBG(3,fCONFIG) DPrint("INFO:     system ID '%s' set for job %s\n",
          J->SystemID,
          J->Name);

        break;

      case mxaSJID:

        /* get system job ID */

        MUStrDup(&J->SystemJID,Value);

        DBG(3,fCONFIG) DPrint("INFO:     system JID '%s' set for job %s\n",
          J->SystemJID,
          J->Name);

        break;

      case mxaTPN:

        /* get TaskPerNode */
 
        RQ->TasksPerNode = atoi(Value);
 
        DBG(5,fCONFIG) DPrint("INFO:     tasks per node set to %d\n",
          RQ->TasksPerNode);

        break;

      case mxaTRL:

        /* load task request list */

        MUStrCpy(tmpLine,key,sizeof(tmpLine));
 
        /* FORMAT:  TRL=<TASKCOUNT>[:<TASKCOUNT>]... */
 
        ptr = MUStrTok(tmpLine,",",&TokPtr2);

        tindex = 1;
 
        while (ptr != NULL)
          {
          if (strchr(ptr,'-') != NULL)
            {
            }
          else
            {
            RQ->TaskRequestList[tindex] = atoi(ptr);
            tindex++;
            }
 
          ptr = MUStrTok(NULL,",",&TokPtr2);
          }
 
        RQ->TaskRequestList[0] = RQ->TaskRequestList[1];
 
        RQ->TaskRequestList[tindex] = 0;
        J->Request.TC = RQ->TaskRequestList[0];

        DBG(3,fCONFIG) DPrint("INFO:     task request list loaded (%d,%d,...)\n",
          RQ->TaskRequestList[1],
          RQ->TaskRequestList[2]);
 
        break;

      default:

        /* not handled */

        break;
      }  /* END switch(aindex) */

    key = MUStrTok(NULL,";\n",&TokPtr);        
    }  /* END while (key != NULL) */

  return(SUCCESS);
  }  /* END MJobProcessExtensionString() */






int MJobAddHash(
 
  char *JobID,    /* I: job ID */
  int   JobIndex, /* I: index of job in MJob array */
  int  *KIndex)   /* O: index of job in mjobl_t array */
 
  {
  int index;
 
  unsigned long hashkey;
 
  int Found = FALSE;

  const char *FName = "MJobAddHash";
 
  DBG(7,fSTRUCT) DPrint("%s(%s,%d,KIndex)\n",
    FName,
    JobID,
    JobIndex);
 
  /* return FAILURE if no key specified */
 
  if (JobID[0] == '\0')
    {
    return(FAILURE);
    }
 
  hashkey = MUGetHash(JobID) % MAX_MJOB;
 
  for (index = hashkey;index < MSched.M[mxoJob] + MAX_MHBUF;index++)
    {
    /* if name is already in hash table... */
 
    if (!strcmp(MJobName[index].Name,JobID))
      {
      *KIndex = MJobName[index].Index;
 
      return(SUCCESS);
      }
 
    /* if location is empty... */
 
    if (MJobName[index].Name[0] == '\0')
      {
      /* mark first available entry */
 
      if (Found == FALSE)
        {
        Found = TRUE;
 
        *KIndex = index;
        }
 
      /* if deleted item, continue searching */
 
      if (MJobName[index].Index != 0)
        {
        continue;
        }
      else
        {
        MUStrCpy(MJobName[*KIndex].Name,JobID,sizeof(MJobName[*KIndex]));
        MJobName[*KIndex].Index = JobIndex;
 
        DBG(8,fSTRUCT) DPrint("INFO:     job hash for job '%s' (index: %d) created\n",
          JobID,
          JobIndex);
 
        *KIndex = JobIndex;
 
        return(SUCCESS);
        }
      }
    }      /* END for (index) */
 
  /* reached end of table */
 
  DBG(1,fSTRUCT) DPrint("ERROR:  job hash table is FULL.  cannot add MJob[%03d] '%s'\n",
    JobIndex,
    JobID);
 
  return(FAILURE);
  }  /* END MJobAddHash() */
 
 
 
 
 
int MJobRemoveHash(
 
  char *JobName)  /* I */
 
  {
  int index;
 
  unsigned long hashkey;

  const char *FName = "MJobRemoveHash";

  DBG(4,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    JobName);
 
  /* return FAILURE if no key specified */
 
  if (JobName[0] == '\0')
    {
    return(FAILURE);
    }
 
  hashkey = MUGetHash(JobName) % MAX_MJOB;
 
  for (index = hashkey;index < MSched.M[mxoJob] + MAX_MHBUF;index++)
    {
    /* if name is in hash table... */
 
    DBG(6,fSTRUCT) DPrint("INFO:     searching for hash entry at MJobName[%03d] '%s' (JobName: '%s')\n",
      index,
      MJobName[index].Name,
      JobName);
 
    if (!strcmp(MJobName[index].Name,JobName))
      {
      MJobName[index].Name[0] = '\0'; /* Indicate Record is Empty */
      MJobName[index].Index   = 1;    /* Indicate Record was Previously Used */
 
      return(SUCCESS);
      }
 
    if ((MJobName[index].Name[0] == '\0') &&
        (MJobName[index].Index == 0))
      {
      return(FAILURE);
      }
    }    /* END for (index) */
 
  /* reached end of table */
 
  return(FAILURE);
  }  /* END MJobRemoveHash() */




int MJobSetState(

  mjob_t             *J,        /* I (modified) */
  enum MJobStateEnum  NewState) /* I */

  {
  const char *FName = "MJobSetState";

  DBG(6,fSTRUCT) DPrint("%s(%s,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    MJobState[NewState]);

  if (J == NULL)
    {
    return(FAILURE);
    }

  switch (NewState)
    {
    case mjsIdle:

      J->State  = mjsIdle;
      J->EState = mjsIdle;

      J->StateMTime = MSched.Time;

      break;

    case mjsStarting:
    case mjsRunning:

      J->State  = mjsRunning;
      J->EState = mjsRunning;

      J->StateMTime = MSched.Time;

      /* NOTE:  incorporate checkpoint/suspend time */

      if (J->StartTime > 0)
        {
        if (J->AWallTime > 0)
          {
          J->RemainingTime = 
            (J->WCLimit > J->AWallTime) ? (J->WCLimit - J->AWallTime) : 0;
          }
        else
          {
          J->RemainingTime = J->WCLimit + J->SWallTime;

          if (MSched.Time > J->StartTime)
            {
            long tmpL;

            tmpL = J->RemainingTime - (MSched.Time - J->StartTime);

            J->RemainingTime = (tmpL > 0) ? tmpL : 0;
            }
          }
        }
      else
        {
        J->RemainingTime = J->WCLimit;
        }

      break;

    case mjsSuspended:

      J->State  = mjsSuspended;
      J->EState = mjsSuspended;

      J->StateMTime = MSched.Time;     

      break;

    default:

      J->State  = NewState;
      J->EState = NewState;
 
      J->StateMTime = MSched.Time;

      break;
    }  /* END switch(NewState) */
 
  return(SUCCESS);
  }  /* END MJobSetState() */




int MJobGetPE(
 
  mjob_t  *J,     /* I */
  mpar_t  *P,     /* I */
  double  *PEptr) /* O */
 
  {
  int     rqindex;
 
  mreq_t *RQ;

  double  PE; 
  double  TotalPE;
 
  TotalPE = 0.0;

  if ((J == NULL) || (P == NULL))
    {
    if (PEptr != NULL)
      *PEptr = 0.0;
 
    return(FAILURE);
    }
 
  if (P->CRes.Procs == 0)
    {
    if (PEptr != NULL) 
      *PEptr = 0.0;

    return(SUCCESS);
    }
 
  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];
 
    PE = (double)RQ->DRes.Procs / P->CRes.Procs;
 
    if ((RQ->DRes.Mem > 0) && (P->CRes.Mem > 0))
      PE = MAX(PE,(double)RQ->DRes.Mem  / P->CRes.Mem);
 
    if ((RQ->DRes.Disk > 0) && (P->CRes.Disk > 0))
      PE = MAX(PE,(double)RQ->DRes.Disk / P->CRes.Disk);
 
    if ((RQ->DRes.Swap > 0) && (P->CRes.Swap > 0))
      PE = MAX(PE,(double)RQ->DRes.Swap / P->CRes.Swap);
 
    TotalPE += (RQ->TaskCount * PE);
    }  /* END for (rqindex) */
 
  TotalPE *= P->CRes.Procs;

  if (PEptr != NULL)
    *PEptr = TotalPE;
 
  return(SUCCESS);
  }  /* END MJobGetPE() */




int MJobReserve(
 
  mjob_t *J,       /* I (modified) */
  int     ResType) /* I */ 
 
  {
  long          BestTime;
  mpar_t       *BestP;
 
  mnodelist_t   MNodeList;
 
  int           NodeCount;
 
  long          InitialCompletionTime;
  char          IStringTime[MAX_MNAME];
  char          Message[MAX_MLINE];
 
  short         tmpTaskList[MAX_MTASK_PER_JOB + 1];
  mnalloc_t     tmpNodeList[MAX_MNODE_PER_JOB + 1];
 
  mnalloc_t    *tmpNL;
 
  int           sindex;
 
  int           BestSIndex;
  unsigned long BestSTime;
 
  unsigned long DefaultSpecWCLimit;
  int           DefaultTaskCount;

  int           rc;
 
  mreq_t       *RQ;

  const char *FName = "MJobReserve";
 
  DBG(3,fSCHED) DPrint("%s(%s,%s)\n",
    FName,
    J->Name,
    MJRType[ResType]);
 
  MTRAPJOB(J,FName);
 
  if ((J == NULL) || (J->Req[0] == NULL))
    {
    return(FAILURE);
    }
 
  /* if job already reserved, release reservation and try reservation again */
 
  if (J->R != NULL)
    {
    InitialCompletionTime = J->R->EndTime;
 
    strcpy(IStringTime,MULToTString(InitialCompletionTime - MSched.Time));
 
    MResDestroy(&J->R);
    }
  else
    {
    InitialCompletionTime = 0;
    }
 
  /* FIXME:  supports only one req */
 
  RQ = J->Req[0];
 
  DefaultSpecWCLimit = J->SpecWCLimit[0];
  DefaultTaskCount   = RQ->TaskRequestList[0];
 
  BestSIndex = 0;
 
  BestSTime  = MAX_MTIME;
  BestP      = NULL;
 
  for (sindex = 1;RQ->TaskRequestList[sindex] > 0;sindex++)
    {
    /* find earliest completion time for each shape */
 
    BestTime = MAX(MSched.Time,J->SMinTime);
 
    MPolicyGetEStartTime(J,&MPar[0],ptSOFT,&BestTime);
 
    /* FIXME:  single req */ 
 
    J->SpecWCLimit[0]      = J->SpecWCLimit[sindex];
    RQ->TaskRequestList[0] = RQ->TaskRequestList[sindex];
    RQ->TaskCount          = RQ->TaskRequestList[sindex];
    J->Request.TC      = RQ->TaskRequestList[sindex];
 
    if (MJobGetEStartTime(
          J,
          &BestP,
          &NodeCount,
          NULL,
          MNodeList,
          &BestTime) == FAILURE)
      {
      /* job cannot be run on any partition with shape sindex */
 
      sprintf(Message,"cannot create reservation for job '%s'",
        J->Name);
 
      if (InitialCompletionTime > 0)
        {
        DBG(1,fSCHED) DPrint("ALERT:    cannot create reservation (job '%s' previously reserved to start in %s)\n",
          J->Name,
          IStringTime);
 
        sprintf(Message,"%s (job previously reserved to start in %s)\n",
          Message,
          IStringTime);
        }
      else
        {
        DBG(1,fSCHED) DPrint("ALERT:    cannot create new reservation for job %s (shape[%d] %d)\n",
          J->Name,
          sindex,
          RQ->TaskRequestList[sindex]);

        sprintf(temp_str," (intital reservation attempt)\n");
        strcat(Message,temp_str);
        }
      }    /* END if (MJobGetEStartTime() == FAILURE) */
    else
      {
      if (BestTime + J->SpecWCLimit[sindex] < BestSTime)
        {
        BestSTime = BestTime + J->SpecWCLimit[sindex];
        BestSIndex = sindex;
        }
      }
    }      /* END for (sindex) */
 
  J->SpecWCLimit[0]      = DefaultSpecWCLimit;
  J->TaskCount           = DefaultTaskCount;
  RQ->TaskRequestList[0] = DefaultTaskCount;
  RQ->TaskCount          = DefaultTaskCount;
 
  if (BestSTime == MAX_MTIME)
    {
    /* job cannot be run on any partition */
 
    sprintf(Message,"cannot create reservation for job '%s'",
      J->Name);
 
    if (InitialCompletionTime > 0)
      {
      DBG(1,fSCHED) DPrint("ALERT:    cannot create reservation (job '%s' previously reserved to start in %s)\n",
        J->Name,
        IStringTime);
 
      sprintf(temp_str," (job previously reserved to start in %s)\n",
        IStringTime);
      strcat(Message,temp_str);
      }
    else
      {
      DBG(1,fSCHED) DPrint("ALERT:    cannot create new reservation for job %s\n",
        J->Name); 
 
      sprintf(temp_str," (intital reservation attempt)\n");
      strcat(Message,temp_str);
      }
 
    if (MSched.Mode != msmTest)
      {
      MJobSetHold(
        J,
        (1 << mhDefer),
        MSched.DeferTime,
        mhrNoResources,
        Message);
      }
 
    return(FAILURE);
    }  /* END if (BestTime == MAX_MTIME) */
 
  /* FIXME:  does not handle multir-req jobs */
 
  J->SpecWCLimit[0]      = J->SpecWCLimit[BestSIndex];
 
  RQ->TaskRequestList[0] = RQ->TaskRequestList[BestSIndex];
  J->Request.TC      = RQ->TaskRequestList[0];
  RQ->TaskCount          = RQ->TaskRequestList[0];
 
  J->SpecWCLimit[BestSIndex]      = DefaultSpecWCLimit;
  RQ->TaskRequestList[BestSIndex] = DefaultTaskCount;
 
  /* must call 'MJobDistributeTasks()' to get proper task count */

  /* NOTE:  must check locality constraints */

  /* NYI */
 
  if (J->Req[1] == NULL)
    {
    tmpNL = J->Req[0]->NodeList;
 
    RQ->NodeList = (mnalloc_t *)MNodeList[0];
 
    MJobDistributeTasks(
      J,
      &MRM[RQ->RMIndex],
      tmpNodeList,
      tmpTaskList);
 
    memcpy(MNodeList[0],tmpNodeList,sizeof(tmpNodeList));

    rc = MResJCreate(J,MNodeList,BestTime,ResType,NULL);

    RQ->NodeList = tmpNL;
 
    if (rc == FAILURE)
      {
      DBG(1,fSCHED) DPrint("ALERT:    cannot create reservation in %s\n",
        FName);

      return(FAILURE);
      }
    }
  else
    {
    /* NOTE:  need multi-req distribution handling */
 
    MResJCreate(J,MNodeList,BestTime,ResType,NULL);
    }
 
  DBG(2,fSCHED) DPrint("INFO:     job '%s' reserved %d tasks (partition %s) to start in %s on %s (WC: %ld)\n",
    J->Name,
    J->Request.TC,
    BestP->Name,
    MULToTString(BestTime - MSched.Time),
    MULToDString((mulong *)&BestTime),
    J->SpecWCLimit[BestSIndex]);
 
  if ((InitialCompletionTime > 0) &&
      (J->R->EndTime > InitialCompletionTime))
    {
    DBG(1,fSCHED) DPrint("ALERT:    reservation completion for job '%s' delayed from %s to %s\n",
      J->Name,
      IStringTime,
      MULToTString(J->R->EndTime - MSched.Time));
 
    sprintf(Message,"reservation completion for job '%s' delayed from %s to %s\n",
      J->Name,
      IStringTime,
      MULToTString(J->R->EndTime - MSched.Time));
    }
 
  return(SUCCESS);
  }  /* END MJobReserve() */




int MJobTrap(
 
  mjob_t *J)  /* I */
 
  {
  /* NO-OP */

  return(SUCCESS);
  }  /* END MJobTrap() */




int MJobAddAccess(
  
  mjob_t *J,     /* I (modified) */
  char   *RName) /* I */

  {
  int rindex;

  if ((J == NULL) || (RName == NULL))
    {
    return(FAILURE);
    }

  for (rindex = 0;rindex < 16;rindex++)
    {
    if (J->RAList[rindex][0] == '\0')
      {
      strcpy(J->RAList[rindex],RName);  

      return(SUCCESS);
      }
    else if (!strcmp(J->RAList[rindex],RName))
      {
      return(SUCCESS);
      }
    }    /* END for (rindex) */

  return(FAILURE);
  }  /* END MJobAddAccess() */




int MJobSetAttr(

  mjob_t  *J,      /* I (modified) */
  enum MJobAttrEnum AIndex, /* I */
  void   **Value,  /* I */
  int      Format, /* I */
  int      Mode)   /* I */

  {

  const char *FName = "MJobSetAttr";

  DBG(7,fSCHED) DPrint("%s(%s,%s,Value,%d,%d)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    MJobAttr[AIndex],
    Format,
    Mode);

  if (J == NULL)
    {
    return(FAILURE);
    }


  switch(AIndex)
    {
    case mjaAccount:

      if (Value != NULL)
        {
        if (MAcctAdd((char *)Value,&J->Cred.A) == FAILURE)
          {
          DBG(1,fSCHED) DPrint("ERROR:    cannot add account for job %s (Name: %s)\n",
            J->Name,
            (char *)Value);
 
          return(FAILURE);
          }
        }

      break;

    case mjaAllocNodeList:

      /* NYI */

      break;

    case mjaAWDuration:

      {
      long tmpL;

      if (Format == mdfString)
        {
        sscanf((char *)Value,"%ld",
          &tmpL);
        }
      else
        {
        tmpL = *(long *)Value;
        }

      J->AWallTime = tmpL;
      }  /* END BLOCK */

      break;

    case mjaCompletionTime:

      {
      long tmpL;

      if (Format == mdfString)
        {
        sscanf((char *)Value,"%ld",
          &tmpL);
        }
      else
        {
        tmpL = *(long *)Value;
        }

      J->CompletionTime = tmpL;
      }  /* END BLOCK */

    case mjaEEWDuration:

      if (Format == mdfString)
        {
        sscanf((char *)Value,"%ld",
          &J->EffQueueDuration);
        }
      else
        {
        J->EffQueueDuration = *(long *)Value;
        }

      break;

    case mjaExcHList:

      {
      int hindex;

      mnode_t *N;

      if (Value == NULL)
        {
        return(FAILURE);
        }

      switch(Format)
        {
        case mdfString:

          if (MNodeFind((char *)Value,&N) != SUCCESS)
            {
            /* cannot locate requested node */

            return(FAILURE);
            }

          break;

        default:

          N = (mnode_t *)Value;

          break;
        }  /* END switch(Format) */

      /* initialize hostlist */

      if (J->ExcHList == NULL)
        {
        if ((J->ExcHList = (mnalloc_t *)calloc(
              1,
              sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1))) == NULL)
          {
          return(FAILURE);
          }

        J->ExcHList[0].N = NULL;
        }

      if (Mode == mAdd)
        {
        /* locate available slot */

        for (hindex = 0;hindex < MAX_MNODE_PER_JOB;hindex++)
          {
          if (J->ExcHList[hindex].N == N)
            {
            J->ExcHList[hindex].TC++;

            break;
            }

          if (J->ExcHList[hindex].N == NULL)
            {
            J->ExcHList[hindex].N  = N;
            J->ExcHList[hindex].TC = 1;

            J->ExcHList[hindex + 1].N = NULL;

            break;
            }
          }    /* END for (hindex) */
        }      /* END if (Mode == mAdd) */
      else
        {
        hindex = 0;

        J->ExcHList[hindex].N  = N;
        J->ExcHList[hindex].TC = 1;

        J->ExcHList[hindex + 1].N = NULL;
        }  /* END else (Mode == mAdd) */
      }    /* END BLOCK */

      break;

    case mjaExecutable:

      {
      MUStrDup(&J->E.Cmd,(char *)Value);
      }  /* END BLOCK */

    case mjaGAttr:

      {
      char *ptr;
      char *TokPtr;

      int index;

      /* FORMAT:  <ATTR>[=<VALUE>] */    

      ptr = MUStrTok((char *)Value,"=",&TokPtr);
   
      if ((index = MUMAGetBM(eJFeature,ptr,mAdd)) > 0)
        {
        if (Mode == mSet)
          {
          J->AttrBM |= index;

          DBG(3,fSCHED) DPrint("INFO:     attribute '%s' set for job %s\n",
            ptr,
            J->Name);
          }
        else
          {
          /* unset */

          if (J->AttrBM & index)
            J->AttrBM ^= index;

          DBG(8,fSCHED) DPrint("INFO:     attribute '%s' cleared for job %s\n",
            ptr,
            J->Name);
          }
        }
   
      /* NOTE:  <VALUE> not yet handled */
      }  /* END BLOCK */

      break;

    case mjaGroup:

      if ((Value != NULL) && (((char *)Value)[0] != '\0'))
        { 
        if (MGroupAdd((char *)Value,&J->Cred.G) == FAILURE)
          {
          DBG(1,fSCHED) DPrint("ERROR:    cannot add group for job %s (Name: '%s')\n",
            J->Name,
            (char *)Value);
 
          return(FAILURE);
          }
        }
      else
        {
        return(FAILURE);
        }
 
      break;

    case mjaHold:

      {
      int tmpI;

      if (Format == mdfString)
        {
        sscanf((char *)Value,"%d",
          &tmpI);
        }
      else
        {
        tmpI = *(int *)Value;
        }

      if (tmpI == (1 << mhAll))
        {
        tmpI = 
          (1 << mhBatch)  | 
          (1 << mhUser)   | 
          (1 << mhSystem) | 
          (1 << mhDefer);
        }

      if (Mode == mUnset)
        {
        int hindex;

        for (hindex = mhUser;hindex <= mhDefer;hindex++)
          {
          if (tmpI & J->Hold & (1 << hindex))
            J->Hold ^= (1 << hindex);
          }  /* END for (hindex) */
        }
      else if (Mode == mAdd)
        {
        J->Hold |= tmpI;
        }
      else
        {
        J->Hold = tmpI;
        }

      if ((J->EState == mjsDeferred) && !(J->Hold & (1 << mhDefer)))
        J->EState = J->State;
      }    /* END BLOCK */

      break;

    case mjaHostList:

      {
      int hindex;

      char *ptr;

      mnode_t *N;

      if (Value == NULL)
        {
        return(FAILURE);
        }

      switch(Format)
        {
        case mdfString:

          ptr = (char *)Value;

          if (ptr[0] == '\0')
            {
            /* unset existing hostlist */

            J->SpecFlags &= ~(1 << mjfHostList);

            MUFree((char **)&J->ReqHList);

            break;
            }

          if (MNodeFind((char *)Value,&N) != SUCCESS)
            {
            /* cannot locate requested node */

            return(FAILURE);
            }

          break;

        default:

          N = (mnode_t *)Value;

          break;
        }  /* END switch(Format) */

      /* initialize hostlist */

      if (J->ReqHList == NULL)
        {
        if ((J->ReqHList = (mnalloc_t *)calloc(
              1,
              sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1))) == NULL)
          {
          return(FAILURE);
          }
        
        J->ReqHList[0].N = NULL;
        }

      J->SpecFlags |= (1 << mjfHostList);

      if (Mode == mAdd)
        {
        /* locate available slot */

        for (hindex = 0;hindex < MAX_MNODE_PER_JOB;hindex++)
          {
          if (J->ReqHList[hindex].N == N)
            {
            J->ReqHList[hindex].TC++;

            break;
            }

          if (J->ReqHList[hindex].N == NULL)
            {
            J->ReqHList[hindex].N  = N;
            J->ReqHList[hindex].TC = 1;
 
            J->ReqHList[hindex + 1].N = NULL;

            break;
            }
          }    /* END for (hindex) */
        }      /* END if (Mode == mAdd) */
      else
        {
        hindex = 0;

        J->ReqHList[hindex].N  = N;
        J->ReqHList[hindex].TC = 1;

        J->ReqHList[hindex + 1].N = NULL;
        }  /* END else (Mode == mAdd) */
      }    /* END BLOCK */

      break;

    case mjaIWD:

      {
      MUStrDup(&J->E.IWD,(char *)Value);
      }  /* END BLOCK */

      break;

    case mjaJobID:

      MUStrCpy(J->Name,(char *)Value,sizeof(J->Name));

      break;

    case mjaJobName:

      MUStrDup(&J->AName,(char *)Value);

      break;

    case mjaMasterHost:

      /* NYI */

      break;

    case mjaMessages:

      if (Value == NULL)
        {
        MUFree(&J->Message);
        }
      else
        {
        MUStrDup(&J->Message,(char *)Value);
        }

      break;

    case mjaReqNodes:

      {
      int NC;

      if (Value == NULL)
        {
        return(FAILURE);
        }

      if (Format == mdfInt)
        {
        NC = *(int *)Value;
        }
      else
        {
        NC = (int)strtol((char *)Value,NULL,0);
        }

      J->Request.NC = NC;

      if (J->Req[0] != NULL)
        {
        J->Req[0]->NodeCount = NC;
        }
      }    /* END BLOCK */

      break;

    case mjaReqProcs:

      {
      int PC;

      /* NOTE:  temporarily map procs to tasks */

      if (Value == NULL)
        {
        return(FAILURE);
        }

      if (Format == mdfInt)
        {
        PC = *(int *)Value;
        }
      else
        {
        PC = (int)strtol((char *)Value,NULL,0);
        }

      J->Request.TC = PC;

      if (J->Req[0] != NULL)
        {
        J->Req[0]->TaskCount = PC;
        }
      }    /* END BLOCK */

      break;

    case mjaPAL:

      if (Value == NULL)
        memset(J->PAL,0xff,sizeof(J->PAL));
      else
        memcpy(J->PAL,(int *)Value,sizeof(J->PAL));

      break;

    case mjaQOS:

      {
      mqos_t *Q;

      if (MQOSAdd((char *)Value,&Q) == SUCCESS)
        {
        MJobSetQOS(J,Q,0);
        }
      }    /* END BLOCK */

      break;
	    
    case mjaQOSReq:

      MQOSAdd((char *)Value,&J->QReq);

      break;

    case mjaReqAWDuration:

      {
      long tmpL;

      if (Value == NULL)
        break;

      if (Format == mdfLong)
        {
        tmpL = *(long *)Value;
        }
      else
        {
        tmpL = strtol((char *)Value,NULL,0);
        }
       	 
      if (tmpL < 0)
        {
        /* unlimited walltime requested */
		
        tmpL = MPar[0].L.JP->HLimit[mptMaxJob][0];
        }
      else if (tmpL == 0)
        {
        tmpL = MSched.DefaultJ.SpecWCLimit[0];
        }

      J->SpecWCLimit[0] = tmpL;
      J->SpecWCLimit[1] = tmpL;
      }  /* END BLOCK */

      break;

    case mjaReqCMaxTime:

      {
      long tmpL = 0;

      if (Mode == mUnset)
        {
        J->CMaxTime = 0;

        break;
        }

      if ((Format == mdfLong) && (Value != NULL))
        {
        tmpL = *(long *)Value;
        }

      J->CMaxTime = tmpL;
      }  /* END BLOCK */

      break;

    case mjaReqReservation:

      if (Value == NULL)
        {
        if (J->SpecFlags & (1 << mjfAdvReservation))
          J->SpecFlags ^= (1 << mjfAdvReservation);
        }
      else
        {
        J->SpecFlags |= (1 << mjfAdvReservation);     

        MUStrCpy(J->ResName,(char *)Value,sizeof(J->ResName));        
        }

      break;

    case mjaReqSMinTime:

      {
      long tmpL = 0;

      if (Format == mdfString)
        {
        tmpL = MUTimeFromString((char *)Value);
        }
      else if ((Format == mdfLong) && (Value != NULL))
        {
        tmpL = *(long *)Value;
        }

      /* NOTE:  evaluate various modes */

      if (Mode == mSet)
        J->SpecSMinTime = tmpL;
      else
        J->SpecSMinTime = MAX(J->SysSMinTime,tmpL);

      J->SMinTime = MAX(J->SpecSMinTime,J->SysSMinTime);
      }  /* END BLOCK */

      break;

    case mjaRMXString:

      /* RM extension string */

      MUStrDup(&J->RMXString,(char *)Value);

      break;

    case mjaRMJID:

      /* RM job ID */

      MUStrDup(&J->RMJID,(char *)Value);

      break;

    case mjaStartCount:

      if (Format == mdfString)
        {
        sscanf((char *)Value,"%d",
          &J->StartCount);
        }
      else
        {
        J->StartCount = *(int *)Value;
        }
 
      break;

    case mjaState:

      {
      int cindex;

      if (Value == NULL)
        break;

      if (Format == mdfInt)
        {
        cindex = *(int *)Value;
        }
      else
        {
        cindex = MUGetIndex((char *)Value,MJobState,FALSE,0);
        }

      if (cindex != 0)
        MJobSetState(J,cindex);
      }  /* END BLOCK */

      break;

    case mjaStatMSUtl:

      if (Format == mdfString)
        {
        sscanf((char *)Value,"%lf",
          &J->MSUtilized);
        }
      else
        {
        J->MSUtilized = *(double *)Value;
        }
 
      break;

    case mjaStatPSDed:

      if (Format == mdfString)
        {
        sscanf((char *)Value,"%lf",
          &J->PSDedicated);
        }
      else
        {
        J->PSDedicated = *(double *)Value;
        }
 
      break;

    case mjaStatPSUtl:

      if (Format == mdfString)
        {
        sscanf((char *)Value,"%lf",
          &J->PSUtilized);
        }
      else
        {
        J->PSUtilized = *(double *)Value;
        }
 
      break;

    case mjaSubmitTime:

      {
      long tmpL;

      if (Value == NULL)
        break;

      tmpL = 0;

      if (Format == mdfLong)
        {
        tmpL = *(long *)Value;
        }
      else
        {
        tmpL = strtol((char *)Value,NULL,0);
        }

      J->SubmitTime = tmpL;
      }  /* END BLOCK */

      break;

    case mjaSuspendDuration:

      {
      long tmpL;

      if (Value == NULL)
        break;

      tmpL = 0;

      if (Format == mdfLong)
        {
        tmpL = *(long *)Value;
        }
      else
        {
        tmpL = strtol((char *)Value,NULL,0);
        }

      J->SWallTime = tmpL;
      }  /* END BLOCK */

      break;

    case mjaSysPrio:

      if (Mode == mUnset)
        {
        J->SystemPrio = 0;
        
        break;
        }

      if (Format == mdfString)
        {
        sscanf((char *)Value,"%ld",
          &J->SystemPrio);
        }
      else
        { 
        J->SystemPrio = *(long *)Value;
        }
 
      break;

    case mjaSysSMinTime:
      
      {
      long tmpL = 0;

      if (Format == mdfString)
        {
        tmpL = MUTimeFromString((char *)Value);
        }
      else if (Value != NULL)
        {
        tmpL = *(long *)Value;
        }

      if (Mode == mSet)
        J->SysSMinTime = tmpL;
      else
        J->SysSMinTime = MAX(J->SysSMinTime,tmpL);

      J->SMinTime = MAX(J->SpecSMinTime,J->SysSMinTime);

      DBG(5,fSCHED) DPrint("INFO:     system min start time set on job %s for %s\n",
        J->Name,
        MULToTString(J->SysSMinTime - MSched.Time));
      }

      break;

    case mjaUser:

      if (Value != NULL)
        { 
        if (MUserAdd((char *)Value,&J->Cred.U) == FAILURE)
          {
          DBG(1,fSCHED) DPrint("ERROR:    cannot add user for job %s (Name: %s)\n",
            J->Name,
            (char *)Value);
 
          return(FAILURE);
          }
        }
 
      break;

    default:

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MJobSetAttr() */




int MJobToString(

  mjob_t *J,     /* I */
  int    *AList, /* I */
  int     Mode,  /* I */
  char   *Buf)   /* O */

  {
  if ((J == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  switch(Mode)
    {
    default:

      /* NYI */

      break;
    }  /* END switch(Mode) */

  return(SUCCESS);
  }  /* MJobToString() */




int MJobAToString(

  mjob_t *J,      /* I */
  enum MJobAttrEnum AIndex, /* I */
  char   *Buf,    /* O */
  int     Mode)   /* I */

  {
  int BufSize = MAX_MLINE;

  if (Buf == NULL)
    {
    return(FAILURE);
    }

  Buf[0] = '\0';

  if (J == NULL)
    {
    return(FAILURE);
    }

  switch(AIndex)
    {
    case mjaAWDuration:

      sprintf(Buf,"%ld",
        J->AWallTime);

      break;

    case mjaCompletionTime:

      if (J->CompletionTime > 0)
        {
        sprintf(Buf,"%ld",
          J->CMaxTime);
        }

      break;

    case mjaReqCMaxTime:

      if (J->CMaxTime > 0)
        {
        sprintf(Buf,"%ld",
          J->CMaxTime);
        }

      break;

    case mjaEEWDuration:

      sprintf(Buf,"%ld",
        J->EffQueueDuration);

      break;

    case mjaGAttr:

      /* NYI */

      break;

    case mjaHold:

      /* NOTE:  change to human readable */

      sprintf(Buf,"%d",
        J->Hold);

      break;

    case mjaJobID:

      strcpy(Buf,J->Name);

      break;

    case mjaJobName:

      if (J->AName != NULL) 
        strcpy(Buf,J->AName);
 
      break;

    case mjaMessages:

      if (J->Message != NULL)
        {
        MUStrCpy(Buf,J->Message,BufSize);
        }

      break;

    case mjaPAL:

      /* NYI */

      break;

    case mjaQOS:

      if (J->Cred.Q != NULL)
        strcpy(Buf,J->Cred.Q->Name);

      break;
       
    case mjaQOSReq:

      if (J->QReq != NULL)
        strcpy(Buf,J->QReq->Name);        

      break;

    case mjaReqReservation:

      /* NYI */

      break;

    case mjaRMJID:

      /* RM job ID */

      if (J->RMJID != NULL)
        {
        MUStrCpy(Buf,J->RMJID,MAX_MNAME);
        }

      break;

    case mjaStartCount:

      sprintf(Buf,"%d",
        J->StartCount);
      
      break;

    case mjaStatMSUtl:

      sprintf(Buf,"%.3lf",
        J->MSUtilized);

      break;

    case mjaStatPSDed:

      sprintf(Buf,"%.3lf",
        J->PSDedicated);

      break;

    case mjaStatPSUtl:

      sprintf(Buf,"%.3lf",
        J->PSUtilized);

      break;

    case mjaSuspendDuration:

      sprintf(Buf,"%ld",
        J->SWallTime);

      break;

    case mjaSysPrio:

      sprintf(Buf,"%ld",
        J->SystemPrio);

      break;

    case mjaReqSMinTime:

      strcpy(Buf,MULToTString(J->SpecSMinTime));

      break;

    case mjaSysSMinTime:

      strcpy(Buf,MULToTString(J->SysSMinTime));
	
      break;

    default:

      /* not handled */

      return(FAILURE);
 
      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MJobAToString() */





int MReqSetAttr(

  mjob_t  *J,      /* I */
  mreq_t  *RQ,     /* I (modified) */
  enum MReqAttrEnum AIndex, /* I */
  void   **Value,  /* I */
  int      Format, /* I */
  int      Mode)   /* I */
 
  {
  const char *FName = "MReqSetAttr";

  DBG(7,fSCHED) DPrint("%s(%s,RQ,%s,Value,%d,%d)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    MReqAttr[AIndex],
    Format,
    Mode);

  if ((J == NULL) || (RQ == NULL))
    {
    return(FAILURE);
    }

  switch(AIndex)
    {
    case mrqaAllocNodeList:

      if (Format == mdfString)
        {
        /* FORMAT:  <HOST>[{+ \t\n;,}<HOST>] */

        /* NYI */
        }
      else 
        {
        mnalloc_t *NL;
       
        int        nindex;

        NL = (mnalloc_t *)Value;

        if ((RQ->NodeList == NULL) && 
           ((RQ->NodeList = (mnalloc_t *)calloc(1,sizeof(mnalloc_t) * (MAX_MNODE_PER_JOB + 1))) == NULL)) 
          {
          break;
          }

        if (NL == NULL)
          {
          RQ->NodeList[0].N = NULL;
 
          break;
          }
 
        for (nindex = 0;nindex < MAX_MNODE_PER_JOB;nindex++)
          {
          if ((NL[nindex].N == NULL) || (NL[nindex].TC == 0))
            break;

          memcpy(
            &RQ->NodeList[nindex],
            &NL[nindex],
            sizeof(mnalloc_t));
          }  /* END for (nindex) */

        RQ->NodeList[nindex].N  = NULL;
        RQ->NodeList[nindex].TC = 0;
        }  /* END else (Format == mdfString) */

      break;

    case mrqaReqArch:

      if ((RQ->Arch = MUMAGetIndex(eArch,(char *)Value,mAdd)) == FAILURE)
        {
        return(FAILURE);
        }

      break;

    case mrqaReqClass:

      {
      mclass_t *C;
      mclass_t *OC;

      OC = J->Cred.C;

      if (Value != NULL)
        MClassAdd((char *)Value,&C);
      else 
        C = NULL;

      if (C == OC)
        break;

      if (Mode == mSet)
        {
        /* clear old class requirement */

        if (OC != NULL)
          {
          RQ->DRes.PSlot[OC->Index].count = 0;
          RQ->DRes.PSlot[0].count         = 0;

          J->Cred.C = NULL;
          }
        }

      J->Cred.C = C;

      if (C != NULL)
        {
        RQ->DRes.PSlot[C->Index].count = 1;
        RQ->DRes.PSlot[0].count        = 1;

        MUBMOR(RQ->ReqFBM,J->Cred.C->DefFBM,MAX_MATTR);
        }

      MJobUpdateFlags(J);

      if (MPar[2].Name[0] != '\0')
        MJobGetPAL(J,J->SpecPAL,J->PAL,NULL);
      }    /* END BLOCK */

      break;

    case mrqaReqDiskPerTask:

      {
      int tmpI;

      if (Format == mdfInt)
        tmpI = *(int *)Value;
      else
        tmpI = (int)strtol((char *)Value,NULL,0);

      RQ->DRes.Disk = tmpI;
      }  /* END BLOCK */

      break;

    case mrqaReqMemPerTask:

      {
      int tmpI;

      if (Format == mdfInt)
        tmpI = *(int *)Value;
      else
        tmpI = (int)strtol((char *)Value,NULL,0);

      RQ->DRes.Mem = tmpI;
      }  /* END BLOCK */

      break;

    case mrqaReqSwapPerTask:

      /* NYI */

      break;

    case mrqaReqNodeFeature:

      if (Mode == mSet)
        {
        MUBMClear(RQ->ReqFBM,sizeof(RQ->ReqFBM) << 3);
        }

      if (Value != NULL)
        {
        MUGetMAttr(eFeature,(char *)Value,Mode,RQ->ReqFBM,sizeof(RQ->ReqFBM)); 
        }
 
      break;

    case mrqaNCReqMax:

      /* NYI */

      break;

    case mrqaNCReqMin:

      {
      int tmpI;

      if ((Format == mdfInt) && (Value != NULL))
        tmpI = *(int *)Value;
      else
        tmpI = -1;

      if (tmpI != -1)
        {
        J->Request.NC = tmpI;
        RQ->NodeCount     = tmpI;
        }
      }    /* END BLOCK */

      break;

    case mrqaReqOpsys:

      if ((RQ->Opsys = MUMAGetIndex(eOpsys,(char *)Value,mAdd)) == FAILURE)
        {
        return(FAILURE);
        }

      break;

    case mrqaTCReqMax:

      /* NYI */

      break;

    case mrqaTCReqMin:

      {
      int tmpI;

      if (Value == NULL)
        break;

      if (Format == mdfInt)
        tmpI = *(int *)Value;
      else if (Format == mdfLong)
        tmpI = (int)(*(long *)Value);
      else
        tmpI = (int)strtol((char *)Value,NULL,0);

      if (tmpI > 0)
        {
        RQ->TaskRequestList[0] = tmpI;
        RQ->TaskRequestList[1] = tmpI;
        RQ->TaskRequestList[2] = 0;

        J->Request.TC = tmpI;
        RQ->TaskCount     = tmpI;
        }
      }    /* END BLOCK */

      break;

    default:

      /* attribute not supported */

      return(FAILURE);
   
      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MReqSetAttr() */




int MReqAToString(

  mjob_t *J,      /* I */
  mreq_t *RQ,     /* I */
  enum MReqAttrEnum AIndex, /* I */
  char   *Buf,    /* O */
  int     Mode)   /* I */

  {
  if (Buf == NULL)
    {
    return(FAILURE);
    }

  Buf[0] = '\0';

  if ((J == NULL) || (RQ == NULL))
    {
    return(FAILURE);
    }

  switch(AIndex)
    {
    default:

      /* not handled */

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MReqAToString() */




int MJobCheckDataReq(

  mjob_t *J)  /* I */

  {
  long tmpTime;

  if (J == NULL)
    {
    return(FAILURE);
    }

  if (J->SIData == NULL) 
    {
    /* no stage in data requested */

    return(SUCCESS);
    }
 
  if (J->State != mjsIdle)
    {
    /* job is not eligible */

    return(SUCCESS);
    }

  if ((J->SIData->SrcFileSize == 0) ||
      (J->SIData->SrcFileSize > J->SIData->DstFileSize))
    {
    MDataGetEAvailTime(J->SIData,&tmpTime);
    }
  else
    {
    tmpTime = 0;
    }

  MJobSetAttr(J,mjaSysSMinTime,(void **)&tmpTime,mdfLong,mSet);
 
  return(SUCCESS);
  }  /* END MJobCheckDataReq() */





int MSDataDestroy(

  msdata_t **D)  /* I (modified) */

  {
  if ((D == NULL) || (*D == NULL))
    {
    return(SUCCESS);
    }

  MUFree(&(*D)->Location);
  MUFree(&(*D)->SrcFileName);
  MUFree(&(*D)->SrcHostName);         
  MUFree(&(*D)->DstFileName);         
  MUFree(&(*D)->DstHostName);         

  free(*D);

  *D = NULL;

  return(SUCCESS);
  }  /* END MSDataDestroy() */




int MSDataCreate(
 
  msdata_t **D)  /* I (modified) */
 
  {
  if (D == NULL)
    {
    return(FAILURE);
    }

  if (*D != NULL)
    {
    return(SUCCESS);
    }
 
  *D = (msdata_t *)calloc(1,sizeof(msdata_t));
 
  return(SUCCESS);
  }  /* END MSDataDestroy() */




#define MDEF_TRANSFERRATE 2000

int MDataGetEAvailTime(

  msdata_t *D,      /* I */
  long     *ETime)  /* O */

  {
  long DataRemaining;
  long TimeRemaining;

  long TransferRate;

  if (D == NULL)
    {
    return(FAILURE);
    }

  if (D->SrcFileSize == 0)
    MFUGetInfo(D->SrcFileName,NULL,&D->SrcFileSize,NULL);

  if (D->DstFileSize < D->SrcFileSize)
    MFUGetInfo(D->DstFileName,NULL,&D->DstFileSize,NULL);   

  DataRemaining = D->SrcFileSize - D->DstFileSize;

  if (D->TransferRate != 0)
    TransferRate = D->TransferRate;
  else if ((D->TStartTime > 0) && (D->DstFileSize > 0))
    TransferRate = (long)(D->DstFileSize / MAX(1,MSched.Time - D->TStartTime));
  else
    TransferRate = MDEF_TRANSFERRATE;

  TimeRemaining = DataRemaining / MAX(1,TransferRate);
   
  if (ETime != NULL)
    *ETime = MSched.Time + TimeRemaining;
 
  return(SUCCESS);
  }  /* END MDataGetEAvailTime() */





int MSDataStage(

  msdata_t *D)  /* I */

  {
  if (D == NULL)
    {
    return(FAILURE);
    }

  if (D->TStartTime > 0)
    {
    /* stage already initiated */

    return(SUCCESS);
    }

  if (D->SrcHostName != NULL)
    {
    /* input file */

    D->TStartTime = MSched.Time;
    }
  else
    {
    /* output file */
    }

  return(SUCCESS);
  }  /* END MDataStage() */




int MSDataGetLocality(

  char *FileName,  /* I */
  int  *IsGlobal)  /* O (boolean) */

  {
  /* NYI */

  if (IsGlobal != NULL)
    *IsGlobal = TRUE;

  return(SUCCESS);
  }  /* END MSDataGetLocality() */




int MJobCPCreate(

  mjob_t    *J,
  mjckpt_t **C)

  {
  if ((J == NULL) || (C == NULL))
    {
    return(FAILURE);
    }

  *C = calloc(1,sizeof(mjckpt_t));

  return(SUCCESS);
  }  /* END MJobCPCreate() */




int MJobCPDestroy(

  mjob_t    *J,
  mjckpt_t **C)

  {
  if ((J == NULL) || (C == NULL))
    {
    return(FAILURE);
    }

  free(*C);

  *C = NULL; 
 
  return(SUCCESS);
  }  /* END MJobCPDestroy() */




int MJobStart(
 
  mjob_t *J)  /* I */
 
  {
  int      nindex;
 
  int      NodeCount;
  int      TaskCount;
 
  enum MHoldReasonEnum      Reason;
 
  mqos_t  *QDef;
 
  mnode_t *N;
 
  mreq_t  *RQ;
 
  mgcred_t *A; 
 
  int      SC;
  int      TC;
 
  int      rqindex;
 
  int      MinProcSpeed;
 
  double   tmpD;
 
  char     ErrMsg[MAX_MLINE];
 
  double   SumNodeLoad;
  double   NodeLoad;
 
  int      JobIsHetero;
 
  const char *FName = "MJobStart";
 
  DBG(2,fSCHED) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");
 
  if ((J == NULL) || (J->Name[0] == '\0'))
    {
    DBG(2,fSCHED) DPrint("ERROR:    invalid job pointer received\n");
 
    return(FAILURE);
    }

  MTRAPJOB(J,FName);
 
  RQ = J->Req[0];  /* FIXME */
 
  if (MJobDistributeTasks(
       J,
       &MRM[RQ->RMIndex],
       J->NodeList,
       J->TaskMap) == FAILURE)
    { 
    DBG(3,fSCHED) DPrint("WARNING:  cannot distribute allocated tasks for job '%s'\n",
      J->Name);
 
    return(FAILURE);
    }
 
  /* verify adequate allocations exist for account */
 
  if (MAMAllocJReserve(&MAM[0],J,FALSE,&Reason,ErrMsg) == FAILURE)
    {
    DBG(3,fSCHED) DPrint("WARNING:  cannot reserve allocation for job '%s', reason: %s\n",
      J->Name,
      MDefReason[Reason]);
 
    if ((Reason == mhrNoFunds) &&
        (MAM[0].FallbackAccount[0] != '\0') &&
        (MAcctFind(MAM[0].FallbackAccount,&A) == SUCCESS))
      {
      /* insufficient allocations in primary account, try fallback account */
 
      J->Cred.A = A;
 
      if ((MQOSGetAccess(J,J->QReq,NULL,&QDef) == FAILURE) ||
          (J->QReq == NULL))
        {
        MJobSetQOS(J,QDef,0);
 
        if (J->QReq != NULL)
          {
          /* defer job one iteration to allow it to be properly re-prioritized */
 
          sprintf(ErrMsg,"job account adjusted to fallback account");
 
          MJobSetHold( 
            J,
            (1 << mhDefer),
            MSched.RMPollInterval,
            Reason,
            ErrMsg);
          }
        }

      return(FAILURE);
      }
    else if (MSched.Mode != msmTest)
      {
      if ((Reason != mhrAMFailure) || (MAM[0].JFAction != mamjfaNONE))
        {
        /* remove job if JFACTION set to cancel */

        if (MAM[0].JFAction == mamjfaCancel)
          {
          MRMJobCancel(J,"cannot debit job account",NULL);
          }
        else
          {
          /* defer job if set to anything else than NONE or cancel */

          sprintf(ErrMsg,"cannot debit job account");

          MJobSetHold(
            J,
            (1 << mhDefer),
            MSched.DeferTime,
            Reason,
            ErrMsg);
          }

        return(FAILURE);
        }
      }
    }  /* END if (AMReserveJobAllocation()) == FAILURE) */
 
  J->StartCount++;
 
  if (MRMJobStart(J,ErrMsg,&SC) == FAILURE)
    {
    DBG(3,fSCHED) DPrint("WARNING:  cannot start job '%s' through resource manager\n",
      J->Name);
 
    if ((SC == mscRemoteFailure) && 
       ((J->StartCount >= MSched.DeferStartCount) || 
        (J->Flags & (1 << mjfNoQueue))))
      {
      DBG(2,fSCHED) DPrint("ALERT:    job '%s' deferred after %d failed start attempts (API failure on last attempt)\n",
        J->Name,
        J->StartCount);
 
      if (MSched.Mode != msmTest)
        {
        MJobSetHold(
          J,
          (1 << mhDefer),
          MSched.DeferTime,
          mhrAPIFailure,
          ErrMsg);
        }
      }
 
    if (J->Cred.A != NULL)
      {
      if (MAMAllocResCancel(
           J->Cred.A->Name,
           J->Name,
           "job cannot start",
           NULL,
           &Reason) == FAILURE)
        {
        DBG(1,fSCHED) DPrint("ERROR:    cannot cancel allocation reservation for job '%s', reason: %s\n",
          J->Name,
          MDefReason[Reason]);
        }
      }
 
    return(FAILURE);
    } /* END if (MRMJobStart(J,SC) == FAILURE) */ 
 
  J->StartTime     = MSched.Time;
  J->DispatchTime  = MSched.Time;
  J->RemainingTime = J->WCLimit;
 
  /* release all nodes reserved by job */
 
  if (J->R != NULL)
    MResDestroy(&J->R);
 
  /* determine number of nodes selected */
 
  NodeCount = 0;
  TaskCount = 0;
 
  MJobSetState(J,mjsRunning);
 
  /* add job to MAQ list */
 
  MQueueAddAJob(J);
 
  JobIsHetero = FALSE;
 
  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];
 
    if (J->Flags & (1 << mjfSpan))
      {
      RQ->PtIndex = 0;
      }
    else if (RQ->NodeList[0].N != NULL)
      {
      RQ->PtIndex = RQ->NodeList[0].N->PtIndex; 
      }
    else
      {
      RQ->PtIndex = 1;
      }
 
    SumNodeLoad  = 0.0;
 
    /* modify node usage */
 
    MUNLGetMinAVal(RQ->NodeList,mnaProcSpeed,NULL,(void **)&MinProcSpeed);
 
    for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
      {
      N = RQ->NodeList[nindex].N;
 
      NodeCount++;
      TaskCount += RQ->NodeList[nindex].TC;
 
      N->TaskCount += RQ->NodeList[nindex].TC;

      /* adjust available resources, values overwritten on next iteration */
 
      if ((RQ->NAccessPolicy == mnacSingleJob) ||
          (RQ->NAccessPolicy == mnacSingleTask))
        {
        MCResAdd(&N->DRes,&N->CRes,&N->CRes,1,FALSE);

        MCResRemove(&N->ARes,&N->CRes,&N->CRes,1,TRUE);

        NodeLoad = (double)N->CRes.Procs;
        }
      else
        {
        TC = RQ->NodeList[nindex].TC;
 
        MCResAdd(&N->DRes,&N->CRes,&RQ->DRes,TC,FALSE);         
 
        MCResRemove(&N->ARes,&N->CRes,&RQ->DRes,TC,TRUE);

        NodeLoad = (double)TC * RQ->DRes.Procs;
        }
 
      if (MinProcSpeed > 0)
        {
        /* scale load to account for parallel job synchronization delays */
 
        if (MinProcSpeed != N->ProcSpeed)
          JobIsHetero = TRUE;
 
        NodeLoad *= (double)MinProcSpeed / MAX(MinProcSpeed,N->ProcSpeed);
        }
 
      N->Load += NodeLoad;

      SumNodeLoad += NodeLoad;      
 
      /* set expected state */ 
 
      MNodeAdjustState(N,&N->EState);
 
      N->SyncDeadLine = MSched.Time + MSched.NodeSyncDeadline;
 
      N->StateMTime   = MSched.Time;
 
      if ((N->State != mnsIdle) && (N->State != mnsActive))
        {
        DBG(0,fSCHED) DPrint("ERROR:    job %s (TC: %d) started on node MNode[%03d] '%s' which is in state '%s' on iteration %d\n",
          J->Name,
          J->Request.TC,
          N->Index,
          N->Name,
          MAList[eNodeState][N->State],
          MSched.Iteration);
        }
 
      if (MSched.Mode == msmSim)
        {
        MNodeAdjustState(N,&N->State);
        }
      }   /* END for (nindex)  */
 
    if (MSched.Mode == msmSim)
      {
      RQ->URes.Procs = (int)(SumNodeLoad * 100 / TaskCount);
      }
    }     /* END for (rqindex) */
 
  if (JobIsHetero == TRUE)
    MStat.TotalHeterogeneousJobs++;
 
  J->NodeCount       = NodeCount;
  J->TaskCount       = TaskCount;
 
  MUNLGetMinAVal(J->NodeList,mnaSpeed,NULL,(void **)&tmpD);
 
  J->WCLimit = (long)((double)J->SpecWCLimit[0] / tmpD);
 
  /* establish reservations on nodes */
 
  if (MResJCreate(J,NULL,MSched.Time,mjrActiveJob,NULL) == FAILURE)
    {
    DBG(0,fSCHED) DPrint("ERROR:    cannot create reservation for job '%s'\n",
      J->Name);
 
    J->EState = mjsIdle;
 
    return(FAILURE);
    } 
 
  J->SyncDeadLine = MAX_MTIME;
 
  MStatRemoveEJob(J);
 
  RQ = J->Req[0];  /* require single partition per job */
 
  MParUpdate(&MPar[RQ->PtIndex]);
 
  /* job successfully started */
 
  MJobUpdateResourceCache(J,0);
 
  MJobAddToNL(J,NULL);
 
  if (J->ASFunc != NULL)
    {
    char tmpLine[MAX_MLINE];
 
    (*J->ASFunc)(J,mascCreate,NULL,NULL);
 
    sprintf(tmpLine,"STATE=%s",
      MJobState[J->State]);
 
    (*J->ASFunc)(J,mascConfig,tmpLine,NULL);
    }
 
  DBG(2,fSCHED) DPrint("INFO:     starting job '%s'\n",
    J->Name);
 
  DBG(2,fSCHED)
    MJobShow(J,0,NULL);
 
  return(SUCCESS);
  }  /* END MJobStart() */




int MJobShow(

  mjob_t *J,    /* I */
  int     Mode, /* I */
  char   *Buf)  /* O */

  {
  char    tmp[MAX_MNAME];
  mreq_t *RQ;

  const char *FName = "MJobShow";

  DBG(9,fSTRUCT) DPrint("%s(%s,%d,Buf)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    Mode);

  if ((J == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }
 
  RQ = J->Req[0]; /* only show first req for now */
 
  if (RQ == NULL)
    {
    DBG(2,fSTRUCT) DPrint("ALERT:    job %s contains no req info\n",
      J->Name);
 
    return(FAILURE);
    }
 
  if (J->CompletionTime == 0)
    {
    strcpy(tmp,"????????");
    }
  else
    {
    sprintf(tmp,"%08ld",
      J->SimWCTime);
    }
 
  DPrint("[%03d] %16.16s %3d:%3d:%3d(%d) %3.3s %10s(%8s) %8s %8s %10s %s %10.10s %10ld %8s %6s %6s %2s %6d %2s %6d ",
    J->Index,
    J->Name,
    J->Request.TC,
    MJobGetProcCount(J),
    J->Request.NC,
    RQ->TasksPerNode,
    MAList[ePartition][RQ->PtIndex], 
    MULToTString(J->WCLimit),
    tmp,
    (J->Cred.U != NULL) ? J->Cred.U->Name : "NULL",
    (J->Cred.G != NULL) ? J->Cred.G->Name : "NULL",
    MJobState[J->State],
    (J->Cred.Q != NULL) ? J->Cred.Q->Name : "NULL",
    MUCAListToString(RQ->DRes.PSlot,NULL,NULL),
    J->SubmitTime,
    MAList[eNetwork][RQ->Network],
    MAList[eOpsys][RQ->Opsys],
    MAList[eArch][RQ->Arch],
    MComp[RQ->MemCmp],
    RQ->RequiredMemory,
    MComp[RQ->DiskCmp],
    RQ->RequiredDisk);
 
  fprintf(mlog.logfp,"%s",
    MUMAList(eFeature,RQ->ReqFBM,sizeof(RQ->ReqFBM)));
 
  fprintf(mlog.logfp,"\n");

  return(SUCCESS);
  }  /* END MJobShow() */




int MJobGetNL(

  mjob_t    *J,  /* I */
  mnalloc_t *NL) /* O */
 
  {
  int rqindex;
  int nindex;
  int ncount;

  const char *FName = "MJobGetNL";
 
  DBG(5,fSTRUCT) DPrint("%s(%s,NL)\n",
    FName,
    J->Name);
 
  ncount = 0;
 
  for (rqindex = 0;rqindex < MAX_MREQ_PER_JOB;rqindex++)
    {
    if (J->Req[rqindex] == NULL)
      continue; 
 
    for (nindex = 0;J->Req[rqindex]->NodeList[nindex].N != NULL;nindex++)
      {
      if (J->Req[rqindex]->NodeList[nindex].TC == 0)
        break;
 
      NL[ncount].N  = J->Req[rqindex]->NodeList[nindex].N;
      NL[ncount].TC = J->Req[rqindex]->NodeList[nindex].TC;
 
      ncount++;
      }  /* END for (nindex)  */
    }    /* END for (rqindex) */
 
  NL[ncount].N = NULL;
 
  DBG(5,fSTRUCT) DPrint("INFO:     %d nodes found\n",
    ncount);
 
  return(SUCCESS);
  }  /* END JobGetNL() */




int MReqGetPref(

  mreq_t  *RQ,
  mnode_t *N,
  char    *IsPref)

  {
  if ((RQ == NULL) || (N == NULL) || (IsPref == NULL))
    {
    return(FAILURE);
    }

  /* node is 'pref' if 'any' pref features are present */

  if (MAttrSubset(
        N->FBM,
        RQ->PrefFBM,
        sizeof(N->FBM),
        tlOR) == SUCCESS)
    {
    *IsPref = (char)TRUE;
    }
  else
    {
    *IsPref = (char)FALSE;
    }

  return(SUCCESS);
  }  /* END MReqGetPref() */




/* order low to high */
 
int MJobCTimeComp(
 
  int *A,
  int *B)
 
  {
  long tmp;
 
  tmp = (MJob[*A]->StartTime + MJob[*A]->WCLimit) - 
        (MJob[*B]->StartTime + MJob[*B]->WCLimit);
 
  return((int)tmp);
  }  /* END MJobCTimeComp() */




int MJobToXML(

  mjob_t  *J,       /* I */
  mxml_t *E,       /* O */
  int     *SAList)  /* I (optional) */

  {
  int DAList[] = {
    mjaAWDuration,
    mjaEEWDuration,   /* eligible job time */
    mjaHold,
    mjaQOSReq,
    mjaReqReservation,
    mjaStartCount,
    mjaStatMSUtl,
    mjaStatPSDed,
    mjaStatPSUtl,
    mjaSysPrio,
    mjaSysSMinTime,
    -1 };

  int  aindex;

  int *AList;

  char tmpString[MAX_MLINE];

  if ((J == NULL) || (E == NULL))
    {
    return(FAILURE);
    }

  if (SAList != NULL)
    AList = SAList;
  else
    AList = DAList;

  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    if ((MJobAToString(J,AList[aindex],tmpString,0) == FAILURE) ||
        (tmpString[0] == '\0'))
      {
      continue;
      }

    MXMLSetAttr(E,(char *)MJobAttr[AList[aindex]],tmpString,mdfString);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MJobToXML() */




int MJobInitialize(

  mjob_t *J)

  {
  if (J == NULL)
    {
    return(FAILURE);
    }

  memset(J,0,sizeof(mjob_t));

  return(SUCCESS);
  }  /* END MJobInitialize() */





int MJobFromXML(

  mjob_t  *J,
  mxml_t *E)

  {
  int aindex;
  int jaindex;

  if ((J == NULL) || (E == NULL))
    {
    return(FAILURE);
    }

  /* NOTE:  do not initialize.  may be overlaying data */

  for (aindex = 0;aindex < E->ACount;aindex++)
    {
    jaindex = MUGetIndex(E->AName[aindex],MJobAttr,FALSE,0);

    if (jaindex == 0)
      continue;    

    MJobSetAttr(J,jaindex,(void **)E->AVal[aindex],mdfString,mSet);
    }  /* END for (aindex) */
  
  return(SUCCESS);
  }  /* END MJobFromXML() */




int MJobStoreCP(

  mjob_t *J,    /* I */
  char   *Buf)  /* O */

  {
  const int CPAList[] = {
    mjaAWDuration,
    mjaEEWDuration,
    mjaHold,
    mjaMessages,
    mjaPAL,
    mjaQOSReq,
    mjaReqReservation,
    mjaReqSMinTime,
    mjaStartCount,
    mjaStatMSUtl,
    mjaStatPSDed,
    mjaStatPSUtl,
    mjaSuspendDuration,
    mjaSysPrio,
    mjaSysSMinTime,
    mjaUser,
    -1 };

  mxml_t *E = NULL;

  if ((J == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  MXMLCreateE(&E,"job");

  MJobToXML(J,E,(int *)CPAList);

  MXMLToString(E,Buf,MAX_MBUFFER,NULL,TRUE);

  MXMLDestroyE(&E);

  return(SUCCESS);
  }  /* END MJobStoreCP() */




int MJobLoadCP(

  mjob_t *JS,  /* I: OPT */
  char   *Buf) /* I: REQ */

  {
  char     tmpName[MAX_MNAME];
  char     JobName[MAX_MNAME];

  long     CkTime;

  char    *ptr;

  mxml_t *E = NULL;  

  mjob_t   tmpJ;

  mjob_t  *J;

  const char *FName = "MJobLoadCP";

  DBG(4,fCKPT) DPrint("%s(JS,%s)\n",
    FName,
    (Buf != NULL) ? Buf : "NULL");

  if (Buf == NULL)
    {
    return(FAILURE);
    }

  /* FORMAT:  <JOBHEADER> <JOBID> <CKTIME> <JOBSTRING> */

  /* determine job name */
 
  sscanf(Buf,"%s %s %ld",
    tmpName,
    JobName,
    &CkTime);
 
  if (((long)MSched.Time - CkTime) > MCP.CPExpirationTime)
    {
    return(SUCCESS);
    }
 
  if (JS == NULL)
    {
    if (MJobFind(JobName,&J,0) != SUCCESS)
      {
      DBG(5,fCKPT) DPrint("INFO:     job '%s' no longer detected\n",
        JobName);
 
      return(SUCCESS);
      }
    }
  else
    {
    J = &tmpJ;

    MJobInitialize(J); 
    }

  if ((ptr = strchr(Buf,'<')) == NULL)
    {
    return(FAILURE);
    }

  MXMLCreateE(&E,"job");

  MXMLFromString(&E,ptr,NULL,NULL);
 
  MJobFromXML(J,E);
  
  MXMLDestroyE(&E);
 
  if (JS != NULL)
    {
    if (JS->Name[0] == '\0')
      {
      memcpy(JS,J,sizeof(mjob_t));
      }
    else
      {
      /* update CP fields only */
 
      JS->Hold             = J->Hold;
      JS->QReq             = J->QReq;
      JS->StartCount       = J->StartCount;
      JS->SystemPrio       = J->SystemPrio;
 
      JS->PSDedicated      = J->PSDedicated;
      JS->PSUtilized       = J->PSUtilized;
      JS->MSUtilized       = J->MSUtilized;
      JS->AWallTime        = J->AWallTime;
      JS->EffQueueDuration = J->EffQueueDuration;
      JS->SystemQueueTime  = MSched.Time - J->EffQueueDuration;

      JS->Message          = J->Message;
      }
 
    if ((JS->Cred.Q != JS->QReq) && (JS->QReq != NULL))
      {
      mqos_t *QDef;

      if (MQOSGetAccess(JS,JS->QReq,NULL,&QDef) == FAILURE)
        {
        DBG(2,fSTRUCT) DPrint("INFO:     job %s does not have access to checkpointed QOSRequest %s (setting to default %s)\n",
          JS->Name,
          JS->QReq->Name,
          QDef->Name);
        }
      else
        {
        MJobSetQOS(JS,JS->QReq,0);
        }
      }
    }  /* END if (JS != NULL) */
 
  return(SUCCESS);
  }  /* END MJobLoadCP() */




int MJobEval(

  mjob_t *J)  /* I */

  {
  mreq_t *RQ;

  if (J == NULL)
    {
    return(FAILURE);
    }

  if (J->Cred.U == NULL)
    {
    return(FAILURE);
    }

  if (J->Cred.G == NULL)
    {
    return(FAILURE);
    }

  RQ = J->Req[0];

  if (RQ == NULL)
    {
    return(FAILURE);
    }

  if ((RQ->NodeCount > 0) && (RQ->TaskCount > 0) && (RQ->TasksPerNode > 1))
    {
    if (RQ->TasksPerNode * RQ->NodeCount != RQ->TaskCount)
      {
      DBG(2,fSTRUCT) DPrint("INFO:     job '%s' has invalid task layout (TPN:%d * N:%d != T:%d)\n",
        J->Name,
        RQ->TasksPerNode,
        RQ->NodeCount,
        RQ->TaskCount);

      return(FAILURE);
      }
    }

  return(SUCCESS);
  }  /* END MJobEval() */




int MJobGetEStartTime(
 
  mjob_t      *J,          /* I: job                               */
  mpar_t     **PP,         /* I/O: constraining/best partition     */
  int         *NodeCount,  /* O: number of nodes located           */
  int         *TaskCount,  /* O: tasks located                     */
  mnodelist_t  MNodeList,  /* O: list of best nodes to execute job */
  long        *EStartTime) /* I/O: earliest start time possible    */
 
  {
  int    rindex;
  int    pindex;
  int    nindex;
  int    rqindex;
 
  unsigned long JobStartTime;
  unsigned long JobEndTime;
  int           JobTaskCount;
 
  int           TC;
 
  mnodelist_t   tmpMNodeList;
 
  int           tmpNodeCount;
  int           tmpTaskCount;
 
  mreq_t       *RQ;
 
  mrange_t      GRange[MAX_MRANGE];
  mrange_t      TRange[MAX_MRANGE];
 
  int           Type;
 
  mcres_t       DRes;
 
  char          NodeMap[MAX_MNODE];
 
  int           BestTaskCount;
  int           BestNodeCount;
  unsigned long BestStartTime;

  mrange_t      BestRange[MAX_MRANGE];
 
  mpar_t       *P;
  mpar_t       *PS;
  mpar_t       *EP = NULL;    

  int           NAPolicy;

  const char *FName = "MJobGetEStartTime";
 
  DBG(4,fSCHED) DPrint("%s(%s,%s,NodeCount,TaskCount,MNodeList,%ld)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    ((PP != NULL) && (*PP != NULL)) ? (*PP)->Name : "NULL",
    *EStartTime);

  if ((J == NULL) || (J->Req[0] == NULL))
    {
    return(FAILURE);
    }

  if (PP != NULL)
    {
    PS = *PP;
    }
  else
    {
    PS = NULL;
    }
 
  BestTaskCount = 0;
  BestStartTime = MAX_MTIME;
 
  memset(&DRes,0,sizeof(DRes));
  memset(NodeMap,nmUnavailable,sizeof(NodeMap));
 
  if (NodeCount != NULL)
    *NodeCount = 0;
 
  if (TaskCount != 0)
    *TaskCount = 0;

  RQ = J->Req[0];

  NAPolicy = (RQ->NAllocPolicy != NULL) ?
    RQ->NAllocPolicy->NAllocPolicy :
    MPar[RQ->PtIndex].NAllocPolicy;
 
  if (!(J->Flags & (1 << mjfSpan)))
    {
    for (pindex = 1;pindex < MAX_MPAR;pindex++)
      {
      P = &MPar[pindex];

      if ((PS != NULL) && (PS != P))
        continue;
 
      if (P->ConfigNodes == 0)
        continue;
 
      /* check partition access */
 
      if (MUBMCheck(P->Index,J->PAL) == FAILURE)
        {
        DBG(7,fSCHED) DPrint("INFO:     job %s not allowed to run in partition %s (allowed %s)\n", 
          J->Name,
          P->Name,
          MUListAttrs(ePartition,J->PAL[0]));
 
        continue;
        }
 
      Type = (1 << rtcNone);
 
      JobStartTime = *EStartTime;
      JobEndTime   = MAX_MTIME;
      JobTaskCount = 0;
 
      for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
        {
        RQ = J->Req[rqindex];
 
        /* check resource availability */
 
        TC = MParGetTC(P,&P->CRes,&P->CRes,&DRes,&RQ->DRes,MAX_MTIME);
 
        if (TC < J->Request.TC)
          {
          DBG(7,fSCHED) DPrint("INFO:     tasks not available for job %s in partition %s (%d of %d tasks)\n",
            J->Name,
            P->Name,
            TC,
            J->Request.TC);
 
          break;
          }
 
        /* check pslot availability */
 
        if (MUNumListGetCount(
             J->StartPriority,
             RQ->DRes.PSlot,
             P->CRes.PSlot,
             0,
             NULL) == FAILURE)
          {
          DBG(7,fSCHED) DPrint("INFO:     pslots not available for job %s in partition %s (%s)\n",
            J->Name,
            P->Name, 
            MUCAListToString(RQ->DRes.PSlot,P->CRes.PSlot,NULL));
 
          break;
          }
 
        if (MJobGetRange(
              J,
              RQ,
              P,
              JobStartTime,
              GRange,
              NULL,
              &tmpNodeCount,
              NodeMap,
              Type,
              TRange) == FAILURE)
          {
          /* acceptable range could not be located */
 
          JobStartTime = MAX_MTIME;
 
          break;
          }
        else
          {
          JobStartTime  = MAX(JobStartTime,GRange[0].StartTime);
          JobEndTime    = MIN(JobEndTime,GRange[0].EndTime);
          JobTaskCount += GRange[0].TaskCount;
 
          if (JobStartTime > JobEndTime)
            {
            /* located ranges do not overlap */
 
            break;
            }
          }     /* END else ((MJobGetRange() == FAILURE)) */
        }       /* END for (rqindex) */
 
      if (J->Req[rqindex] != NULL) 
        {
        /* ranges not found for all reqs */
 
        continue;
        }
 
      if (JobStartTime < BestStartTime)
        {
        BestStartTime = JobStartTime;
        EP            = P;
        BestTaskCount = JobTaskCount;
 
        if (TRange[0].TaskCount >= J->Request.TC)
          memcpy(BestRange,TRange,sizeof(BestRange));  /* FIXME:  only works for single req jobs */
        else
          memcpy(BestRange,GRange,sizeof(BestRange));
 
        DBG(5,fSCHED) DPrint("INFO:     start time %s found for job %s in partition %s (%ld)\n",
          MULToTString(BestStartTime - MSched.Time),
          J->Name,
          EP->Name,
          *EStartTime);
        }
      }    /* END for (pindex) */

    if (EP != NULL)
      {
      for (rindex = 0;BestRange[rindex].EndTime != 0;rindex++)
        {
        BestTaskCount = 0;
        BestNodeCount = 0;
 
        for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
          {
          if (MJobGetRange(
                J,
                J->Req[rqindex], 
                EP,
                BestRange[rindex].StartTime,
                GRange,
                tmpMNodeList,
                &tmpNodeCount,
                NodeMap,
                (1 << rtcStartEarliest),
                NULL) == FAILURE)
            {
            /* cannot obtain range nodelist */
 
            DBG(2,fSCHED) DPrint("ALERT:    cannot obtain nodelist for job %s in range %d\n",
              J->Name,
              rindex);
 
            continue;
            }
 
          BestNodeCount += tmpNodeCount;
 
          for (nindex = 0;tmpMNodeList[0][nindex].N != NULL;nindex++)
            {
            BestTaskCount += tmpMNodeList[0][nindex].TC;
            }
          }    /* END for (rqindex) */
 
        DBG(2,fSCHED) DPrint("INFO:     located resources for %d tasks (%d) in best partition %s for job %s at time %s\n",
          J->Request.TC,
          BestTaskCount,
          EP->Name,
          J->Name,
          MULToTString(BestStartTime - MSched.Time));

        /* apply locality constraints */

        if ((J->Flags & (1 << mjfAllocLocal)) || 
            (MSched.AllocLocalityPolicy != malpNONE))
          {
          if (MJobProximateMNL(
               J,
               tmpMNodeList,
               tmpMNodeList,
               MAX_MTIME,
               FALSE) == FAILURE)
            {
            DBG(4,fSCHED) DPrint("INFO:     inadequate proximity within available tasks\n");

            return(FAILURE);
            }
          }    /* END if ((J->Flags & (1 << mjfAllocLocal))...) */

        /* impose resource set constraints */

        RQ = J->Req[0];
 
        if (MPar[0].NodeSetDelay > 0)
          {
          int        tmpRSS;
          mnalloc_t *NodeList; 
 
          NodeList = (mnalloc_t *)tmpMNodeList[0];
 
          /* mandatory node set */
 
          tmpRSS = (RQ->SetSelection != mrssNONE) ?
            RQ->SetSelection :
            MPar[0].NodeSetPolicy;
 
          if (MJobSelectResourceSet(
               J,
               RQ,
               (RQ->SetType != mrstNONE) ? RQ->SetType : MPar[0].NodeSetAttribute,
               (tmpRSS != mrssOneOf) ? tmpRSS : mrssFirstOf,
               (RQ->SetList[0] != NULL) ? RQ->SetList : MPar[0].NodeSetList,
               NodeList,
               BestNodeCount) == FAILURE)
            {
            DBG(0,fSCHED) DPrint("WARNING:  nodeset constraints prevent use of tasks for job %s at %s\n",
              J->Name,
              MULToTString(BestRange[rindex].StartTime - MSched.Time));
 
            continue;
            }
          }    /* END if (Policy[0].NodeSetDelay > 0) */

        if (MJobAllocMNL(
              J,
              tmpMNodeList,
              NodeMap,
              MNodeList,
              NAPolicy,
              BestRange[rindex].StartTime) == FAILURE)
          {
          DBG(0,fSCHED) DPrint("WARNING:  cannot allocate tasks for job %s at %s\n",
            J->Name,
            MULToTString(BestRange[rindex].StartTime - MSched.Time));
 
          continue; 
          }
        else
          {
          /* node list located */

          break;
          }
        }    /* END for (rindex) */
 
      if (BestRange[rindex].EndTime == 0)
        {
        DBG(0,fSCHED) DPrint("ERROR:    cannot allocate tasks for job %s at any time\n",
          J->Name);
 
        return(FAILURE);
        }
 
      BestStartTime = BestRange[rindex].StartTime;
 
      tmpTaskCount = 0;
 
      for (nindex = 0;MNodeList[0][nindex].N != NULL;nindex++)
        tmpTaskCount += MNodeList[0][nindex].TC;
 
      if (NodeCount != NULL)
        *NodeCount = nindex;
 
      if (TaskCount != NULL)
        *TaskCount = tmpTaskCount;
 
      *EStartTime = BestStartTime;

      if (PP != NULL) 
        *PP = EP;
 
      return(SUCCESS);
      }  /* END if if (EP != NULL) */
    else
      {
      DBG(2,fSCHED) DPrint("ALERT:    job %s cannot run in any partition\n",
        J->Name); 
 
      return(FAILURE);
      }
    }    /* END if (!(J->Flags & (1 << mjfSpan))) */
 
  /* search for 'spanning' resources */
 
  Type = (1 << rtcFeasible);
 
  JobStartTime = *EStartTime;
  JobEndTime   = MAX_MTIME;
  JobTaskCount = 0;
 
  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];
 
    /* FIXME */
 
    pindex = 0;
 
    P = &MPar[pindex];
 
    /* check resource availability */
 
    TC = MParGetTC(P,&P->CRes,&P->CRes,&DRes,&RQ->DRes,MAX_MTIME);
 
    if (TC < J->Request.TC)
      {
      DBG(7,fSCHED) DPrint("INFO:     resources not available for job %s in partition %s (%d of %d tasks)\n",
        J->Name,
        P->Name,
        TC,
        J->Request.TC);
 
      return(FAILURE);
      }
 
    /* check class availability */ 
 
    if (MUNumListGetCount(J->StartPriority,RQ->DRes.PSlot,P->CRes.PSlot,0,NULL) == FAILURE)
      {
      DBG(7,fSCHED) DPrint("INFO:     classes not available for job %s in partition %s (%s)\n",
        J->Name,
        P->Name,
        MUCAListToString(RQ->DRes.PSlot,P->CRes.PSlot,NULL));
 
      break;
      }
 
    if (MJobGetRange(
          J,
          RQ,
          P,
          JobStartTime,
          GRange,
          NULL,
          &tmpNodeCount,
          NodeMap,
          Type,
          NULL) == FAILURE)
      {
      /* acceptable range could not be located */
 
      DBG(2,fSCHED) DPrint("ALERT:    resources unavailable for spanning job %s:%d\n",
        J->Name,
        rqindex);
 
      return(FAILURE);
      }
    else
      {
      JobStartTime  = MAX(JobStartTime,(unsigned long)GRange[0].StartTime);
      JobEndTime    = MIN(JobEndTime,(unsigned long)GRange[0].EndTime);
      JobTaskCount += GRange[0].TaskCount;
 
      if (JobStartTime > JobEndTime)
        {
        /* located ranges do not overlap */ 
 
        break;
        }
      }
    }   /* END for (rqindex) */
 
  if (J->Req[rqindex] != NULL)
    {
    /* ranges not found for all reqs */
 
    DBG(2,fSCHED) DPrint("ALERT:    resources unavailable for spanning job %s\n",
      J->Name);
 
    return(FAILURE);
    }
 
  for (rindex = 0;BestRange[rindex].EndTime != 0;rindex++)
    {
    /* try to allocate nodes in first acceptable range */
 
    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      RQ = J->Req[rqindex];

      if (MJobGetRange(
            J,
            RQ,
            EP,
            BestRange[rindex].StartTime,
            GRange,
            tmpMNodeList,
            &tmpNodeCount,
            NodeMap,
            (1 << rtcStartEarliest),
            NULL) == FAILURE)
        {
        /* cannot obtain range nodelist */
 
        DBG(2,fSCHED) DPrint("ERROR:    cannot obtain nodelist for spanning job %s in range %d\n",
          J->Name,
          rindex); 
 
        continue;
        }
      }    /* END for (rqindex) */
 
    DBG(2,fSCHED) DPrint("INFO:     located resources for %d tasks for spanning job %s at time %s in range %d\n",
      J->Request.TC,
      J->Name,
      MULToTString(BestRange[rindex].StartTime - MSched.Time),
      rindex);
 
    if (MJobAllocMNL(
          J,
          tmpMNodeList,
          NodeMap,
          MNodeList,
          NAPolicy,
          BestRange[rindex].StartTime) == FAILURE)
      {
      DBG(0,fSCHED) DPrint("ERROR:    cannot allocate tasks for job %s in range %d\n",
        J->Name,
        rindex);
 
      continue;
      }
    else
      {
      break;
      }
    }    /* END for (rindex) */
 
  if (BestRange[rindex].EndTime == 0)
    {
    /* resources could not be allocated in any range */
 
    DBG(0,fSCHED) DPrint("ERROR:    cannot allocate tasks for job %s in any range\n",
      J->Name); 
 
    return(FAILURE);
    }

  if ((BestStartTime >= MAX_MTIME) && (BestRange[0].StartTime < BestStartTime))
    {
    BestStartTime = BestRange[0].StartTime;
    }
 
  tmpTaskCount = 0;
 
  for (nindex = 0;MNodeList[0][nindex].N != NULL;nindex++)
    tmpTaskCount += MNodeList[0][nindex].TC;
 
  if (NodeCount != NULL)
    *NodeCount = nindex;
 
  if (TaskCount != NULL)
    *TaskCount = tmpTaskCount;
 
  *EStartTime = BestStartTime;

  if (PP != NULL) 
    *PP = EP;
 
  return(SUCCESS);
  }  /* END MJobGetEStartTime() */




int MJobSendFB(

  mjob_t *J)

  {
  char   Args[32][MAX_MNAME];
  char  *ArgV[32];

  double xfactor;
 
  int    aindex;
  int    ACount;
 
  mreq_t *RQ;

  const char *FName = "MJobSendFB";

  DBG(2,fCORE) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : NULL);

  if (J == NULL)
    {
    return(FAILURE);
    }

  xfactor = (double)(J->CompletionTime - J->SystemQueueTime) /
                    (J->CompletionTime - J->StartTime);
 
  /* FORMAT:    JN UN EM JS QR QTM STM CTM  XA WCL PRQ MRQ APROC MPROC AME MME */
 
  RQ = J->Req[0];
 
  ACount = 0;
 
  strcpy(Args[ACount++],J->Name);
  strcpy(Args[ACount++],J->Cred.U->Name);
  strcpy(Args[ACount++],NONE);
  strcpy(Args[ACount++],MJobState[J->State]);
  strcpy(Args[ACount++],(J->QReq != NULL) ? J->QReq->Name : NONE);
  sprintf(Args[ACount++],"%ld",J->SubmitTime);
  sprintf(Args[ACount++],"%ld",J->StartTime);
  sprintf(Args[ACount++],"%ld",J->CompletionTime);
  sprintf(Args[ACount++],"%lf",xfactor);
  sprintf(Args[ACount++],"%ld",J->WCLimit);
  sprintf(Args[ACount++],"%d",MJobGetProcCount(J));
  sprintf(Args[ACount++],"%d",RQ->DRes.Mem * RQ->TaskCount);
  sprintf(Args[ACount++],"%lf",
    (double)((J->AWallTime > 0) ? J->PSUtilized / J->AWallTime : 0.0));
  sprintf(Args[ACount++],"%lf",(double)RQ->MURes.Procs / 100.0);
  sprintf(Args[ACount++],"%ld",
    (long)((J->AWallTime > 0) ? RQ->LURes.Mem / J->AWallTime : 0));
  sprintf(Args[ACount++],"%d",RQ->MURes.Mem);
 
  for (aindex = 0;aindex < ACount;aindex++)
    {
    ArgV[aindex + 1] = Args[aindex];
    }
 
  ArgV[aindex + 1] = NULL;

  if (MSysLaunchAction(ArgV,mactJobFB) == SUCCESS)
    {
    DBG(2,fCORE) DPrint("INFO:     job usage sent for job '%s'\n",
      J->Name);
    }

  return(SUCCESS);
  }  /* END MJobSendFB() */




int MJobWriteStats(
 
  mjob_t *J)  /* I */
 
  {
  char Buf[MAX_MBUFFER];

  const char *FName = "MJobWriteStats";
 
  DBG(4,fSTAT) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ?  J->Name : "NULL");

  if (J == NULL)
    {
    return(FAILURE);
    }
 
  if (MSched.statfp == stderr)
    {
    fprintf(MSched.statfp,"STATS:  ");
    }
 
  if (MJobToTString(J,DEFAULT_WORKLOAD_TRACE_VERSION,Buf,sizeof(Buf)) == SUCCESS)
    {
    fprintf(MSched.statfp,"%s",Buf);
 
    fflush(MSched.statfp);
 
    DBG(4,fSTAT) DPrint("INFO:     job stats written for '%s'\n",
      J->Name);
    }
 
  return(SUCCESS);
  }  /* END MJobWriteStats() */




int MJobPReserve(
 
  mjob_t *J,        /* I (modified) */
  int     PIndex,   /* I */
  int    *ResCount, /* O */
  mbool_t *ResCountRej) /* O (optional) */
 
  {
  int GResPolicy;
  int ResPolicy;
 
  int DoReserve;
 
  int XFactor;
 
  int rindex;
  int qindex;
 
  mpar_t *P;
  mpar_t *GP;
 
  const char *FName = "MJobPReserve";
 
  DBG(1,fSCHED) DPrint("%s(%s,%s,%s,ResCountRej)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    MAList[ePartition][PIndex],
    (ResCount != NULL) ? "ResCount" : "NULL");

  if (ResCountRej != NULL)
    *ResCountRej = FALSE;

  if (J == NULL)
    {
    return(FAILURE);
    }
 
  P  = &MPar[PIndex]; 
  GP = &MPar[0];
 
  MTRAPJOB(J,FName);
 
  if ((J->Cred.Q != NULL) && (J->Cred.Q->Flags & (1 << mqfnoreservation)))
    {
    DBG(1,fSCHED) DPrint("INFO:    reservation not allowed for job %s in %s (QOS)\n",
      J->Name,
      FName);

    return(FAILURE);
    }
 
  if (GP->BFPolicy == bfPREEMPT)
    {
    DBG(1,fSCHED) DPrint("INFO:     no priority reservations created (preempt based backfill enabled)\n");

    return(FAILURE);
    }
 
  GResPolicy = (GP->ResPolicy == resDefault) ?
    DEFAULT_RESERVATIONMODE : GP->ResPolicy;
 
  ResPolicy = (P->ResPolicy != resDefault) ?
    P->ResPolicy : GResPolicy;
 
  if ((GP->BFPolicy == bfNONE) ||
      (ResPolicy == resNever))
    {
    /* no reservations required */
 
    /* verify job can run */
 
    if (MJobReserve(J,mjrPriority) == SUCCESS)
      {
      /* release temporary reservation */
 
      MResDestroy(&J->R);
      }
    else
      {
      /* job can never run */
 
      MJobSetHold(
        J,
        (1 << mhDefer), 
        MSched.DeferTime,
        mhrNoResources,
        "cannot create reservation");
      }

    if (ResCountRej != NULL)
      *ResCountRej = TRUE;

    DBG(1,fSCHED) DPrint("INFO:     no priority reservations created (bf/rsv policy)\n");
 
    return(FAILURE);
    }
 
  if (J->R != NULL)
    {
    /* job already has reservation (should be non-priority) */
    DBG(1,fSCHED) DPrint("INFO:     no priority reservations created (existing reservation)\n");
 
    return(FAILURE);
    }
 
  /* reserve nodes for highest priority idle job */
 
  DoReserve = FALSE;
 
  if (GP->BFPolicy != bfNONE)
    {
    switch (ResPolicy)
      {
      case resNever:
 
        /* do not reserve nodes */
 
        DoReserve = FALSE;
 
        break;
 
      case resHighest:
      case resCurrentHighest:
 
        switch (GP->ResTType)
          {
          case rttNone:
 
            DoReserve = TRUE; 
 
            break;
 
          case rttBypass:
 
            if (J->Bypass > GP->ResTValue)
              DoReserve = TRUE;
 
            break;
 
          case rttQueueTime:
 
            if ((MSched.Time - J->SystemQueueTime) >
                 GP->ResTValue)
              {
              DoReserve = TRUE;
              }
 
            break;
 
          case rttXFactor:
 
            XFactor = ((MSched.Time - J->SystemQueueTime) + J->SpecWCLimit[0]) /
                        J->SpecWCLimit[0];
 
            if (XFactor > GP->ResTValue)
              {
              DoReserve = TRUE;
              }
 
            break;
 
          default:
 
            DoReserve = TRUE;
 
            break;
          }  /* END switch (GP->ResTType) */
 
        /* FUTURE WORK: */ 
 
        /* determine cost of previous backfill */
 
        /* could job start if backfill jobs were not running? */
 
        break;
 
      default:
 
        break;
      }  /* END switch (ResPolicy) */
    }    /* if (GP->BFPolicy != bfNONE) */
 
  /* locate QOS group */
 
  for (rindex = 0;rindex < MAX_MQOS;rindex++)
    {
    if (GP->ResDepth[rindex] == 0)
      continue;
 
    for (qindex = 0;qindex < MAX_MQOS;qindex++)
      {
      if (GP->ResQOSList[rindex][qindex] == NULL)
        break;
 
      if ((GP->ResQOSList[rindex][qindex] == J->Cred.Q) ||
          (GP->ResQOSList[rindex][qindex] == (mqos_t *)MAX_MQOS))
        {
        break;
        }
      }  /* END for (qindex) */
 
    if ((GP->ResQOSList[rindex][qindex] == J->Cred.Q) ||
        (GP->ResQOSList[rindex][qindex] == (mqos_t *)MAX_MQOS))
      {
      break;
      }
    }    /* END for (rindex) */
 
  if (rindex == MAX_MQOS)
    { 
    DoReserve = FALSE;
    }
 
  if ((DoReserve == TRUE) &&
      (ResCount[rindex] < GP->ResDepth[rindex]))
    {
    if (MJobReserve(J,mjrPriority) == FAILURE)
      {
      DBG(2,fSCHED) DPrint("WARNING:  cannot reserve priority job '%s'\n",
        ((J != NULL) && (J->Name[0] > 1)) ? J->Name : "NULL");

      if (ResCountRej != NULL)
        *ResCountRej = FALSE;

      return(FAILURE);
      }
    else
      {
      if (J->R != NULL)
        {
        J->R->Priority = J->StartPriority;
 
        J->R->Flags |= (1 << mrfPreemptible);
 
        ResCount[rindex]++;
        }
      else
        {
        DBG(1,fCORE) DPrint("ALERT:    cannot create reservation for priority job %s\n",
          J->Name);
        }
      }
    }
  else
    {
    if (ResCountRej != NULL)
      *ResCountRej = FALSE;

    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MJobPReserve() */



int MJobSetDependency(

  mjob_t              *J,
  enum MJobDependEnum  Type,
  char                *Value)

  {
  mjdepend_t *D;

  if (J == NULL)
    {
    return(FAILURE);
    }

  /* NOTE:  handle only single job dependency per job for now */

  if ((J->Depend == NULL) &&
     ((J->Depend = (mjdepend_t *)calloc(1,sizeof(mjdepend_t))) == NULL))
    {
    return(FAILURE);
    }

  D = J->Depend;

  D->Type = Type;
 
  MUStrDup(&D->Value,Value);
 
  return(SUCCESS);
  }  /* END MJobSetDependency() */




int MJobCheckDependency(
 
  mjob_t              *SJ,    /* I */
  enum MJobDependEnum *Type,  /* O (optional) */
  char                *Value) /* O (optional) */
 
  {
  int   sindex;
  char  Name[MAX_MLINE];
  char  Line[MAX_MLINE];
  char *ptr;
 
  mjob_t *J;

  mjdepend_t *D;
 
  char *TokPtr;

  mrm_t *RM;

  mbool_t  DependSatisfied;
 
  const char *FName = "MJobCheckDependency";
 
  DBG(8,fSCHED) DPrint("%s(%s,Type,%s)\n",
    FName,
    (SJ != NULL) ? SJ->Name : "NULL",
    (Value != NULL) ? "Value" : "NULL");

  if (Value != NULL)
    Value[0] = '\0';

  if (Type != NULL)
    *Type = mjdNONE;

  if (SJ == NULL)
    {
    return(FAILURE);
    }

  D = SJ->Depend;

  /* NOTE:  handle only single job dependency per job for now */

  if ((D == NULL) || (D->Type == mjdNONE) || (D->Value == NULL))
    {
    return(SUCCESS);
    }
 
  RM = (SJ->RM != NULL) ? SJ->RM : &MRM[0];
 
  if (RM->Type == mrmtLL)
    {
    /* enforce job step based prerequisites */
 
    strcpy(Line,SJ->SubmitHost);
 
    ptr = MUStrTok(Line,".",&TokPtr);
 
    for (sindex = 0;sindex < SJ->Proc;sindex++)
      {
      sprintf(Name,"%s.%d.%d", 
        ptr,
        SJ->Cluster,
        sindex);
 
      if (MJobFind(Name,&J,0) == SUCCESS)
        {
        DBG(6,fSCHED) DPrint("INFO:     job '%s' has job '%s' remaining as a prereq step\n",
          SJ->Name,
          Name);
 
        if (Value != NULL)
          strcpy(Value,J->Name);

        /* NOTE:  LL job dependencies no longer default */
 
        /* return(FAILURE); */
        }
      }    /* END for (sindex) */
    }      /* END if (RM->Type) */

  while (D != NULL)
    { 
    DependSatisfied = FALSE;

    /* job dependency specified */
 
    if (MJobFind(D->Value,&J,0) == FAILURE)
      {
      /* cannot locate depend prereq */

      D = D->Next;
 
      continue;
      }
 
    switch(D->Type)
      {
      case mjdJobStart:

        if ((MJOBISACTIVE(J) == TRUE) || 
            (MJOBISCOMPLETE(J) == TRUE)) 
          {
          DependSatisfied = TRUE;
          }
      
        break;
 
      case mjdJobCompletion:

        if (MJOBISCOMPLETE(J) == TRUE)
          {
          DependSatisfied = TRUE;
          }

        break;

      case mjdJobSuccessfulCompletion:

        if (MJOBISSCOMPLETE(J) == TRUE)
          {
          DependSatisfied = TRUE;
          }

        break;

      case mjdJobFailedCompletion:
  
        if (MJOBISFCOMPLETE(J) == TRUE)
          {
          DependSatisfied = TRUE;
          }

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch(D->Type) */

    if (DependSatisfied == FALSE)
      {
      /* dependency not satisfied */
 
      if (Value != NULL)
        strcpy(Value,D->Value);

      if (Type != NULL)
        *Type = D->Type;

      return(FAILURE);
      }  /* END if (DependSatisfied == FALSE) */

    D = D->Next;
    }  /* END while (D != NULL) */

  /* all dependencies satisfied */
 
  return(SUCCESS);
  }  /* END MJobCheckDependency() */




int MJobCheckpoint(

  mjob_t *J)  /* I (modified) */

  {
  int rc;

  if (J == NULL)
    {
    return(FAILURE);
    }

  if (MSched.Mode == msmSim)
    {
    return(MSimJobCheckpoint(J));
    }

  if ((J->Ckpt == NULL) || (J->Ckpt->SystemICPEnabled != TRUE))
    {
    /* job cannot be checkpointed */

    return(FAILURE);
    }

  rc = MRMJobCheckpoint(J,TRUE,NULL,NULL);

  return(rc);
  }  /* END MJobCheckpoint() */




int MJobCheckNRes(

  mjob_t  *J,            /* I: job pointer                 */
  mnode_t *N,            /* I: node pointer                */
  mreq_t  *RQ,           /* I: req                         */
  long     StartTime,    /* I: time job must start         */
  int     *TCAvail,      /* O: tasks allowed at StartTime  */
  double   MinSpeed,     /* I */
  int     *RIndex,       /* O: node rejection index        */
  char    *Affinity,     /* O: affinity status             */
  long    *AvailTime,    /* O: duration available          */
  int      DoFeasibleCheck)

  {
  int MinTPN;

  int TC;

  const char *FName = "MJobCheckNRes";

  DBG(5,fSCHED) DPrint("%s(%s,%s,RQ[%d],%s,TCAvail,%.3f,RIndex,%s,FeasCheck)\n",
    FName,
    J->Name,
    N->Name,
    RQ->Index,
    MULToTString(StartTime - MSched.Time),
    MinSpeed,
    (Affinity == NULL) ? "NULL" : "Affinity");

  *RIndex = 0;

  if (TCAvail != NULL)
    *TCAvail = 0;

  if (Affinity != NULL)
    *Affinity = nmNone;

  if (DoFeasibleCheck == TRUE)
    {
    if (MReqCheckResourceMatch(J,RQ,N,RIndex) == FAILURE)
      {
      return(FAILURE);
      }
    }

  if (StartTime == MSched.Time)
    {
    mcres_t *NA;

    /* check dynamic node attributes */

    if (!(J->Flags & (1 << mjfPreemptor)))
      {
      if (((N->State != mnsIdle) && (N->State != mnsActive)) ||
          ((N->EState != mnsIdle) && (N->EState != mnsActive)))
        {
        DBG(5,fSCHED) DPrint("INFO:     node is in %s state '%s'\n",
          (N->State != mnsIdle) ? "" : "expected",
          (N->State != mnsIdle) ? MNodeState[N->State] : MNodeState[N->EState]);

        *RIndex = marState;

        return(FAILURE);
        }

      if ((RQ->NAccessPolicy == mnacSingleJob) ||
          (RQ->NAccessPolicy == mnacSingleTask))
        {
        if ((N->State != mnsIdle) || (N->EState != mnsIdle))
          {
          DBG(5,fSCHED) DPrint("INFO:     node is in %s state '%s' (Dedicated access required)\n",
            (N->State != mnsIdle) ? "" : "expected",
            (N->State != mnsIdle) ? MNodeState[N->State] : MNodeState[N->EState]);

          *RIndex = marState;

          return(FAILURE);
          }

        /* redundant test */

        if (N->ARes.Procs < N->CRes.Procs)
          {
          DBG(5,fSCHED) DPrint("INFO:     inadequate procs on node '%s' (%d available)\n",
            N->Name,
            N->ARes.Procs);

          *RIndex = marCPU;

          return(FAILURE);
          }
        }    /* END if ((RQ->NAccessPolicy == mnacSingleJob) || ...) */

      NA = &N->ARes;
      }      /* END if (!(J->Flags & (1 << mjfPreemptor))) */
    else
      {
      NA = &N->CRes;
      }

    TC = MNodeGetTC(N,NA,&N->CRes,&N->DRes,&RQ->DRes,StartTime);

    if (TC < MAX(1,RQ->TasksPerNode))
      {
      DBG(5,fSCHED) DPrint("INFO:     node supports %d task%c (%d tasks/node required)\n",
        TC,
        (TC == 1) ? ' ' : 's',
        MAX(1,RQ->TasksPerNode));

      *RIndex = marCPU;

      return(FAILURE);
      }
    else
      {
      DBG(8,fSCHED) DPrint("INFO:     node supports %d task%c (%d tasks/node required)\n",
        TC,
        (TC == 1) ? ' ' : 's',
        MAX(1,RQ->TasksPerNode));
      }

    /* check classes */

    if (MUNumListGetCount(J->StartPriority,RQ->DRes.PSlot,NA->PSlot,0,NULL) == FAILURE)
      {
      DBG(5,fSCHED) DPrint("INFO:     node is missing classes [ALL %d:%d]%s required:available\n",
        RQ->DRes.PSlot[0].count,
        NA->PSlot[0].count,
        MUCAListToString(RQ->DRes.PSlot,NA->PSlot,NULL));

      *RIndex = marClass;
  
      return(FAILURE);
      }

    if ((N->RM != NULL) && (N->RM->Type == mrmtLL))
      {
      /* handle LL geometry */

      MinTPN = (RQ->TasksPerNode > 0) ? RQ->TasksPerNode : 1;

      if (RQ->NodeCount > 0)
        {
        MinTPN = RQ->TaskCount / RQ->NodeCount;
        }

      /* handle pre-LL22 geometries */

      if ((RQ->BlockingFactor != 1) &&
         ((TC < (MinTPN - 1)) ||
         ((TC < MinTPN) && (RQ->TaskCount % RQ->NodeCount == 0))))
        {
        DBG(5,fSCHED) DPrint("INFO:     node supports %d task%c (%d tasks/node required:LL)\n",
          TC,
          (TC == 1) ? ' ' : 's',
          MinTPN);

        *RIndex = marCPU;

        return(FAILURE);
        }
      }    /* END if (RM[N->RMIndex].Type == mrmtLL) */

    /* check adapters */

    if ((RQ->Network != 0) && !(N->Network & (1 << RQ->Network)))
      {
      DBG(5,fSCHED) DPrint("INFO:     inadequate adapters %d (%s needed  %s found)\n",
        RQ->Network,
        MAList[eNetwork][RQ->Network],
        MUListAttrs(eNetwork,N->Network));

      *RIndex = marAdapter;

      return(FAILURE);
      }

    /* check local disk space */

    if (N->CRes.Disk > 0)
      {
      /* NOTE:  only check disk space is disk space is reported */

      if (N->CRes.Disk <= (2 * J->ExecSize))
        {
        DBG(5,fSCHED) DPrint("INFO:     inadequate free space in local filesystem for executable on node '%s' (%d MB available)\n",
          N->Name,
          N->CRes.Disk);

        *RIndex = marDisk;

        return(FAILURE);
        }
      }    /* END if (N->CRes.Disk > 0) */

    if (RQ->DRes.GRes[0].count != 0)
      {
      if (MUNumListGetCount(J->StartPriority,RQ->DRes.GRes,NA->GRes,0,NULL) == FAILURE)
        {
        DBG(3,fSCHED) DPrint("INFO:     generic resources not found (%d needed)\n",
          RQ->DRes.GRes[0].count);

        *RIndex = marFeatures;

        return(FAILURE);
        }
      }
    }   /* END if (StartTime == MSched.Time) */

  if (StartTime < MAX_MTIME)
    {
    /* check dynamic node attributes */

    /* check swap space */

    if ((N->CRes.Swap > 0) && (N->ARes.Swap < MIN_SWAP))
      {
      DBG(5,fSCHED) DPrint("INFO:     inadequate swap on node '%s' (%d MB available)\n",
        N->Name,
        N->ARes.Swap);

      *RIndex = marSwap;

      return(FAILURE);
      }

    /* check requested disk space */

    if (N->FrameIndex != -1)
      {
      if (MSys[N->FrameIndex][N->SlotIndex].Disk > 0)
        {
        if (RQ->RequiredDisk > MSys[N->FrameIndex][N->SlotIndex].Disk)
          {
          DBG(5,fSCHED) DPrint("INFO:     inadequate disk space on node '%s' (%d MB available)\n",
            N->Name,
            MFrame[N->FrameIndex].Disk);

          *RIndex = marDisk;

          return(FAILURE);
          }
        }
      else if (MFrame[N->FrameIndex].Disk > 0)
        {
        if (RQ->RequiredDisk > MFrame[N->FrameIndex].Disk)
          {
          DBG(5,fSCHED) DPrint("INFO:     inadequate disk space on node '%s' (%d MB available)\n",
            N->Name,
            MFrame[N->FrameIndex].Disk);

          *RIndex = marDisk;

          return(FAILURE);
          }
        }
      }      /* if (N->FrameIndex != -1) */

    /* check local disk */

    if (!MUCompare(N->CRes.Disk,RQ->DiskCmp,RQ->RequiredDisk))
      {
      DBG(5,fSCHED) DPrint("INFO:     inadequate disk (%s %d requested  %d found)\n",
        MComp[RQ->DiskCmp],
        RQ->RequiredDisk,
        N->CRes.Disk);

      *RIndex = marDisk;

      return(FAILURE);
      }

    if (MJobCheckNStartTime(
          J,
          RQ,
          N,
          StartTime,
          TCAvail,
          MinSpeed,
          RIndex,
          Affinity,
	  AvailTime) == FAILURE)
      {
      return(FAILURE);
      }
    }
  else
    {
    if (TCAvail != NULL)
      {
      *TCAvail = MNodeGetTC(N,&N->CRes,&N->CRes,&N->DRes,&RQ->DRes,MAX_MTIME);
      }
    }

  if ((TCAvail != NULL) && (*TCAvail == 0))
    {
    return(FAILURE);
    }
 
  if (MLocalJobCheckNRes(J,N,StartTime) == FAILURE)
    {
    DBG(5,fSCHED) DPrint("INFO:     failed local check\n");

    *RIndex = marPolicy;

    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MJobCheckNRes() */




int MJobCheckNStartTime(

  mjob_t   *J,            /* I:   job                          */
  mreq_t   *RQ,
  mnode_t  *N,            /* I:   node                         */
  long      StartTime,    /* I:   job start time               */
  int      *TCAvail,      /* O:   tasks allowed at starttime   */
  double    MinSpeed,
  int      *RIndex,       /* O:   selection failure index      */
  char     *Affinity,     /* O:   affinity of allocation       */
  long     *ATime)        /* O:   availability duration        */

  {
  long AvailableTime;
  int Type;

  long AdjustedWCLimit;

  /* check reservations */

  const char *FName = "MJobCheckNStartTime";

  DBG(5,fSCHED) DPrint("%s(%s,RQ,%s,%s,TasksAllowed,%lf,RIndex,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (N != NULL) ? N->Name : "NULL",
    MULToTString(StartTime - MSched.Time),
    MinSpeed,
    (Affinity != NULL) ? "Affinity" : "NULL");

  if ((J == NULL) || (RQ == NULL) || (N == NULL))
    return(FAILURE);

  AdjustedWCLimit = (long)((double)J->SpecWCLimit[0] / MinSpeed);

  /* check reservation time */

  if (MJobGetNRange(
        J,
        RQ,
        N,
        StartTime,
        TCAvail,
        &AvailableTime,
        Affinity,
        &Type,
        NULL) == FAILURE)
    {
    /* cannot locate feasible time */

    if (RIndex != NULL)
      {
      *RIndex = marTime;
      }

    return(FAILURE);
    }

  if (AvailableTime < MIN(AdjustedWCLimit,8640000)) 
    {
    /* NOTE:  ignore time based violations over 100 days out */

    if (RIndex != NULL)
      {
      *RIndex = marTime;
      }

    return(FAILURE);
    }

  if (ATime != NULL)
    *ATime = AvailableTime;

  return(SUCCESS);
  }  /* END MJobCheckNStartTime() */




/* select feasible nodes */

int MReqGetFNL(

  mjob_t        *J,           /* I:  job                       */
  mreq_t        *RQ,          /* I:  req                       */
  mpar_t        *P,           /* I:  node partition            */
  nodelist_t     SrcNL,       /* I:  eligible nodes (optional) */
  nodelist_t     DstNL,       /* O:  feasible nodes            */
  int           *NC,          /* O:  number of nodes found     */
  int           *TC,          /* O:  number of tasks found     */
  long           StartTime,   /* I:  time job must start       */
  unsigned long  ResMode)     /* I:  0: all  1: best effort    */

  {
  mnode_t *N;

  int  rindex;
  int  nindex;

  int  RIndex;

  int  tc;

  int  TasksAllowed;

  int  tmpRSS;

  mrm_t *RM;

  mnalloc_t tmpNodeList[MAX_MPAR][MAX_MNODE];

  const char *FName = "MReqGetFNL";

  DBG(4,fSCHED) DPrint("%s(%s,%d,%s,%s,DstNL,NC,TC,%ld,%ld)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (RQ != NULL) ? RQ->Index : -1,
    (P != NULL) ? P->Name : "NULL",
    (SrcNL != NULL) ? "SrcNL" : "NULL",
    StartTime,
    ResMode);

  if ((J == NULL) || (RQ == NULL))
    {
    return(FAILURE);
    }

  if (NC != NULL)
    *NC = 0;

  if (TC != NULL)
    *TC = 0;

  /* step through all nodes */

  rindex = 0;

  tc = 0;

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    if (SrcNL != NULL)
      {
      if (SrcNL[nindex].N != NULL)
        N = SrcNL[nindex].N;
      else
        break;
      }
    else
      {
      N = MNode[nindex];

      if ((N == NULL) || (N->Name[0] == '\0'))
        break;

      if (N->Name[0] == '\1')
        continue;

      /* verify requirements are met */

      if (MReqCheckResourceMatch(J,RQ,N,NULL) == FAILURE)
        continue;
      }  /* END (SrcNL != NULL) */

    if ((N->PtIndex != P->Index) &&
        (P->Index != 0) &&
        (N->PtIndex != 0) &&
       !(J->Flags & (1 << mjfSpan)))
      {
      continue;
      }

    if (StartTime != MAX_MTIME)
      {
      if ((N->State != mnsIdle) &&
          (N->State != mnsActive) &&
          (N->State != mnsBusy))
        {
        continue;
        }
      }

    if (SrcNL != NULL)
      {
      /* check exclude list */

      if (J->ExcHList != NULL)
        {
        int hlindex;

        for (hlindex = 0;J->ExcHList[hlindex].N != NULL;hlindex++)
          {
          if (J->ExcHList[hlindex].N->Index == nindex)
            break;
          }  /* END for (hlindex) */

        if (J->ExcHList[hlindex].N != NULL)
          continue;
        }  /* END if (J->ExcHList != NULL) */

      if ((J->Flags & (1 << mjfHostList)) &&
          (J->ReqHList != NULL) &&
         ((J->ReqHLMode != mhlmSubset) ||
          (MPar[0].EnableMultiNodeJobs == FALSE)))
        {
        int hlindex;

        /* check hostlist constraints */

        for (hlindex = 0;J->ReqHList[hlindex].N != NULL;hlindex++)
          {
          if (J->ReqHList[hlindex].N == N)
            break;
          }  /* END for (hlindex) */

        if (J->ReqHList[hlindex].N == NULL)
          continue;
        }
      }    /* END if (SrcNL != NULL) */

    /* check node state */

    if ((N->State == mnsDown) ||
        (N->State == mnsDrained) ||
        (N->State == mnsNone) ||
        (N->State == mnsUnknown))
      {
      DBG(7,fSCHED) DPrint("INFO:     node %s is unavailable (state %s)\n",
        N->Name,
        MNodeState[N->State]);

      continue;
      }

    /* check node policies */

    TasksAllowed = (N->CRes.Procs != 0) ? N->CRes.Procs : 9999;

    if (MNodeCheckPolicies(J,N,MAX_MTIME,&TasksAllowed) == FAILURE)
      {
      DBG(7,fSCHED) DPrint("INFO:     node %s is unavailable (policies)\n",
        N->Name);

      continue;
      }

    if (MPar[0].EnableMultiNodeJobs == FALSE)
      {
      if (TasksAllowed < J->Request.TC)
        {
        DBG(7,fSCHED) DPrint("INFO:     node %s is unavailable (policy/inadequate tasks %d)\n",
          N->Name,
          TasksAllowed);

        continue;
        }
      }
 
    if (MJobCheckNRes(
          J,
          N,
          RQ,
          StartTime,
          &TasksAllowed,
          1.0,
          &RIndex,
          NULL,
	  NULL,
          TRUE) == FAILURE)
      {
      DBG(7,fSCHED) DPrint("INFO:     node %s does not meet job requirements (%s)\n",
        N->Name,
        MAllocRejType[RIndex]);

      continue;
      }

    DBG(6,fSCHED) DPrint("INFO:     node %s added to feasible list (%d tasks)\n",
      N->Name,
      TasksAllowed);

    DstNL[rindex].N  = N;
    DstNL[rindex].TC = TasksAllowed;

    tc += DstNL[rindex].TC;

    rindex++;
    }     /* END for (nindex) */

  DstNL[rindex].N = NULL;

  tmpRSS = (RQ->SetSelection != mrssNONE) ?
    RQ->SetSelection :
    MPar[0].NodeSetPolicy;

  if ((MPar[0].NodeSetDelay == 0) && (tmpRSS != mrssNONE))
    {
    /* best effort node set */

    MJobSelectResourceSet(
      J,
      RQ,
      (RQ->SetType != mrstNONE) ? RQ->SetType : MPar[0].NodeSetAttribute,
      (tmpRSS != mrssFirstOf) ? tmpRSS : mrssOneOf,
      RQ->SetList,
      (mnalloc_t *)DstNL,
      rindex);

    tc = 0;

    for (rindex = 0;DstNL[rindex].N != NULL;rindex++)
      {
      tc += DstNL[rindex].TC;
      }  /* END for (rindex) */
    }    /* END if ((MPar[0].NodeSetDelay == 0) && ...) */

  if (J->RM != NULL)
    RM = J->RM;
  else
    RM = &MRM[0];

  switch(RM->SubType)
    {
    case mrmstRMS:

      {
      int sindex;

      MRMSSelectAdjacentNodes(
        P->Index,
        J->TasksRequested,
        (mnalloc_t *)DstNL,
        tmpNodeList);

      /* merge all feasible slots */

      tc = 0;
      rindex = 0;

      for (sindex = 0;sindex < MAX_MPAR;sindex++)
        {
        if (tmpNodeList[sindex][0].N == NULL)
          break;

        for (nindex = 0;nindex < MAX_MNODE;nindex++)
          {
          if (tmpNodeList[sindex][nindex].N == NULL)
            break;

          memcpy(
            &DstNL[rindex],
            &tmpNodeList[sindex][nindex],
            sizeof(DstNL[0]));

          rindex++;

          tc += tmpNodeList[sindex][nindex].TC;
          }  /* END for (nindex) */
        }    /* END for (sindex) */

      DstNL[rindex].N = NULL;
      }  /* END BLOCK */

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(RM->SubType) */

  DBG(2,fSCHED) DPrint("INFO:     %d feasible tasks found for job %s:%d in partition %s (%d Needed)\n",
    tc,
    J->Name,
    RQ->Index,
    (P != NULL) ? P->Name : "NULL",
    RQ->TaskCount);

  if (NC != NULL)
    *NC = rindex;

  if (TC != NULL)
    *TC = tc;

  if (tc <= 0)
    {
    DBG(7,fSCHED) DPrint("INFO:     inadequate feasible tasks found for job %s:%d\n",
      J->Name,
      RQ->Index);

    return(FAILURE);
    }

  if (!(J->Flags & (1 << mjfBestEffort)))
    {
    if (tc < RQ->TaskCount)
      {
      DBG(2,fSCHED) DPrint("INFO:     inadequate feasible tasks found for job %s:%d in partition %s (%d < %d)\n",
        J->Name,
        RQ->Index,
        (P != NULL) ? P->Name : "NULL",
        tc,
        RQ->TaskCount);

      return(FAILURE);
      }

    if (rindex < RQ->NodeCount)
      {
      DBG(2,fSCHED) DPrint("INFO:     inadequate feasible nodes found for job %s:%d in partition %s (%d < %d)\n",
        J->Name,
        RQ->Index,
        (P != NULL) ? P->Name : "NULL",
        rindex,
        RQ->NodeCount);

      return(FAILURE);
      }
    }    /* END if (!(J->Flags & (1 << mjfBestEffort))) */

  return(SUCCESS);
  }   /* END MReqGetFNL() */




int MJobGetAMNodeList(

  mjob_t      *J,          /* I:  job                             */
  mnodelist_t  SrcMNL,     /* I:  feasible nodes (optional)       */
  mnodelist_t  DstMNL,     /* O:  available nodes                 */
  char         NodeMap[MAX_MNODE], /* O: state of nodes           */
  int         *NC,         /* O:  number of available nodes       */
  int         *TC,         /* O:  number of available procs       */
  long         StartTime)  /* I:  time at which job must start    */

  {
  /* determine nodes available for immediate use at starttime */

  int     nindex;
  int     rqindex;

  int     index;

  mnodelist_t  MtmpNodeList;

  mnalloc_t   *NodeList;
  mnalloc_t   *Feasible;

  mnodelist_t  MNewNodeList;

  char         Affinity;
  int          RIndex;

  int          AvailableTaskCount[MAX_MREQ_PER_JOB];
  int          AvailableNodeCount[MAX_MREQ_PER_JOB];

  mnode_t     *N;

  mreq_t      *RQ;

  double       MinSpeed;

  int          BestEffort;

  int          TasksAllowed;

  const char *FName = "MJobGetAMNodeList";

  DBG(5,fSCHED) DPrint("%s(%s,SrcMNL,DstMNL,NodeMap,NodeCount,TaskCount,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    MULToTString(StartTime - MSched.Time));

  if (J == NULL)
    {
    return(FAILURE);
    }

  /* check time constraints on all nodes */

  if (NC != NULL)
    *NC = 0;

  BestEffort = FALSE;

  if (TC != NULL)
    {
    if (*TC == -1)
      BestEffort = TRUE;

    *TC = 0;
    }

  if (NodeMap != NULL)
    memset(NodeMap,nmUnavailable,sizeof(char) * MAX_MNODE);

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];

    NodeList = (mnalloc_t *)MtmpNodeList[rqindex];
    Feasible = (mnalloc_t *)SrcMNL[rqindex];

    MUNLGetMinAVal(Feasible,mnaSpeed,NULL,(void **)&MinSpeed);

    nindex = 0;

    AvailableTaskCount[rqindex] = 0;
    AvailableNodeCount[rqindex] = 0;

    for (index = 0;Feasible[index].N != NULL;index++)
      {
      N = Feasible[index].N;

      DBG(5,fSCHED) DPrint("INFO:     checking Feasible[%d][%03d]:  '%s'\n",
        rqindex,
        index,
        N->Name);

      if (MJobCheckNStartTime(
            J,
            RQ,
            N,
            StartTime,
            &TasksAllowed,
            MinSpeed,
            &RIndex,
            &Affinity,
	    NULL) == FAILURE)
        {
        /* job cannot start at requested time */

        NodeMap[N->Index] = nmUnavailable;

        continue;
        }

      if (J->Flags & (1 << mjfAdvReservation))
        {
        if ((Affinity == nmPositiveAffinity) ||
            (Affinity == nmNeutralAffinity) ||
            (Affinity == nmNegativeAffinity) ||
            (Affinity == nmRequired))
          {
          DBG(7,fSCHED) DPrint("INFO:     node '%s' added (reserved)\n",
            N->Name);
          }
        else
          {
          DBG(7,fSCHED) DPrint("INFO:     node '%s' rejected (not reserved)\n",
            N->Name);

          NodeMap[N->Index] = nmUnavailable;

          continue;
          }
        }    /* END if (J->Flags & (1 << mjfAdvReservation)) */

      NodeList[nindex].N  = N;
      NodeList[nindex].TC = TasksAllowed;

      NodeMap[NodeList[nindex].N->Index] = Affinity;

      nindex++;

      AvailableTaskCount[rqindex] += TasksAllowed;
      AvailableNodeCount[rqindex] ++;

      DBG(5,fSCHED) DPrint("INFO:     MNode[%03d] '%s' added (%d of %d)\n",
        index,
        N->Name,
        AvailableTaskCount[rqindex],
        RQ->TaskCount);
      }     /* END for (index) */

    /* terminate list */

    NodeList[nindex].N = NULL;

    if (NC != NULL)
      (*NC) += nindex;

    if (TC != NULL)
      (*TC) += AvailableTaskCount[rqindex];

    if (AvailableTaskCount[rqindex] < RQ->TaskCount)
      {
      DBG(3,fSCHED) DPrint("INFO:     inadequate tasks found for job %s:%d (%d < %d) at %s\n",
        J->Name,
        rqindex,
        AvailableTaskCount[rqindex],
        RQ->TaskCount,
        MULToTString(StartTime - MSched.Time));

      if ((BestEffort == TRUE) && (AvailableTaskCount[rqindex] > 0))
        {
        J->Request.TC -= (RQ->TaskCount - AvailableTaskCount[rqindex]);

        RQ->TaskCount = AvailableTaskCount[rqindex];
        }
      else
        {
        return(FAILURE);
        }
      }

    if ((RQ->NodeCount > 0) &&
        (AvailableNodeCount[rqindex] < RQ->NodeCount) &&
        (BestEffort != TRUE))
      {
      DBG(3,fSCHED) DPrint("INFO:     inadequate nodes found for job %s:%d (%d < %d)\n",
        J->Name,
        rqindex,
        AvailableNodeCount[rqindex],
        RQ->NodeCount);

      return(FAILURE);
      }
    }      /* END for (rqindex) */

  DBG(3,fSCHED) DPrint("INFO:     adequate tasks found for all reqs (time %s)\n",
    MULToTString(StartTime - MSched.Time));

  if (J->Flags & (1 << mjfAllocLocal))
    {
    /* check nodelist locality */

    if (MJobProximateMNL(
          J,
          MtmpNodeList,
          MNewNodeList,
          StartTime,
          FALSE) == FAILURE)
      {
      DBG(4,fSCHED) DPrint("INFO:     insufficient proximity in available tasks\n");

      if (NC != NULL)
        *NC = 0;

      NodeList = (mnalloc_t *)MtmpNodeList[0];

      NodeList[0].N = NULL;

      return(FAILURE);
      }
    else
      {
      memcpy(MtmpNodeList,MNewNodeList,sizeof(MNewNodeList));
      }
    }

  if (MJobNLDistribute(J,MtmpNodeList,DstMNL) == FAILURE)
    {
    /* insufficient nodes were found */

    DBG(4,fSCHED)
      {
      for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
        {
        RQ = J->Req[rqindex];

        DBG(4,fSCHED) DPrint("INFO:     tasks found for job %s:%d  %d of %d\n",
          J->Name,
          rqindex,
          AvailableTaskCount[rqindex],
          RQ->TaskCount);
        }
      }

    return(FAILURE);
    }

  DBG(4,fSCHED) DPrint("INFO:     tasks found for job %s (tasks requested: %d)\n",
    J->Name,
    J->Request.TC);

  return(SUCCESS);
  }  /* END MJobGetAMNodeList() */




int MJobProcessRemoved(

  mjob_t *J)  /* I */

  {
  int      nindex;

  int      rqindex;
  int      ccount;

  enum MHoldReasonEnum      Reason;

  char     Line[MAX_MLINE];

  mnode_t *N;

  mjob_t  *MJ;

  mreq_t  *RQ;

  char      Message[MAX_MLINE];

  const char *FName = "MJobProcessRemoved";

  DBG(2,fSTAT) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if (J == NULL)
    {
    return(FAILURE);
    }

  /* active/inactive job was terminated without successfully completing */

  MSched.EnvChanged = TRUE;

  /* debit account if wallclock limit reached */

  if (((unsigned long)(J->CompletionTime - J->StartTime) >= J->WCLimit) ||
       (MAM[0].ChargePolicy == mamcpDebitAllWC) ||
       (MAM[0].ChargePolicy == mamcpDebitAllCPU) ||
       (MAM[0].ChargePolicy == mamcpDebitAllPE))
    {
    if (MAMAllocJDebit(&MAM[0],J,&Reason,Message) == FAILURE)
      {
      DBG(1,fSTAT) DPrint("ERROR:    cannot debit account for job '%s'\n",
        J->Name);

      sprintf(Line,"AMFAILURE:  cannot debit account for job %s, reason %s (%s)\n",
        J->Name,
        MDefReason[Reason],
        Message);

      MSysRegEvent(Line,0,0,1);
      }
    }
  else
    {
    if (J->Cred.A != NULL)
      {
      if (MAMAllocResCancel(J->Cred.A->Name,J->Name,"job removed",NULL,&Reason) == FAILURE)
        {
        DBG(1,fSCHED) DPrint("ERROR:    cannot cancel allocation reservation for job '%s'\n",
          J->Name);
        }
      }
    }

  if (X.XMetaStoreCompletedJobInfo != (int (*)())0)
    {
    (*X.XMetaStoreCompletedJobInfo)(X.xd,J);
    }

  if (MPar[0].BFChunkDuration > 0)
    {
    MPar[0].BFChunkBlockTime = MSched.Time + MPar[0].BFChunkDuration;
    }

  /* create job statistics record */

  MJobWriteStats(J);

  /* adjust reservation */

  if (J->R != NULL)
    {
    if (J->ResName[0] != '\0')
      {
      /* handled in G2 */

      /* NYI */
      }

    if (J->R->Flags & (1 << mrfSingleUse))
      MResDestroy(&J->R);
    }  /* END if (J->R != NULL) */

  /* modify expected state of nodes */

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];

    for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
      {
      N = RQ->NodeList[nindex].N;

      DBG(3,fSTAT) DPrint("INFO:     node '%s' release from job\n",
        N->Name);

      if (nindex == MAX_MNODE_PER_JOB)
        {
        DBG(0,fSTAT) DPrint("ERROR:    node overflow in %s() on job '%s'\n",
          FName,
          J->Name);

        break;
        }

      /* determine new expected state for node */

      if (MNodeCheckAllocation(N) != SUCCESS)
        {
        N->EState = mnsIdle;
        }
      else
        {
        N->EState = mnsActive;
        }

      if (N->EState == N->State)
        N->SyncDeadLine = MAX_MTIME;
      else
        N->SyncDeadLine = MSched.Time + MSched.NodeSyncDeadline;
      }   /* END for (nindex)  */
    }     /* END for (rqindex) */

  /* handle statistics */

  MStatUpdateRejectedJobUsage(J,0);

  /* provide feedback info to user */

  MJobSendFB(J);

  /* modify expected job state */

  J->EState = J->State;

  if (J->MasterJobName != NULL)
    {
    if (MJobFind(J->MasterJobName,&MJ,0) == FAILURE)
      {
      DBG(2,fSTAT) DPrint("WARNING:  cannot locate masterjob '%s' at completion of job '%s'\n",
        J->MasterJobName,
        J->Name);
      }

    ccount = 0;

    for (rqindex = 0;MJ->Req[rqindex] != NULL;rqindex++)
      {
      if (!strcmp(J->Name,MJ->Req[rqindex]->SubJobName))
        {
        MJ->Req[rqindex]->State = mjsCompleted;
        }

      if (MJ->Req[rqindex]->State == mjsCompleted)
        ccount++;
      }

    if (ccount == rqindex)
      {
      /* all subjobs completed.  cancel master job */

      MRMJobCancel(MJ,"MOAB_INFO:  job has completed\n",NULL);
      }
    }     /* END if (J->MasterJobName != NULL) */

  MJobRemove(J);

  return(SUCCESS);
  }  /* END MJobProcessRemoved() */




int MJobProcessCompleted(

  mjob_t *J)  /* I (modified) */

  {
  int      nindex;
  int      rqindex;

  enum MHoldReasonEnum      Reason;

  int      ccount;

  char     Line[MAX_MLINE];

  mnode_t *N;
  mjob_t  *MJ;
  mreq_t  *RQ;

  char     Message[MAX_MLINE];

  const char *FName = "MJobProcessCompleted";

  DBG(2,fSTAT) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if (J == NULL)
    {
    return(FAILURE);
    }

  MSched.EnvChanged = TRUE;

  /* debit account for completed job */

  if (MAMAllocJDebit(&MAM[0],J,&Reason,Message) == FAILURE)
    {
    DBG(1,fSTAT) DPrint("ERROR:    cannot debit account for job '%s'\n",
      J->Name);

    sprintf(Line,"AMFAILURE:  cannot debit allocations for job %s, reason %s (%s)\n",
      J->Name,
      MDefReason[Reason],
      Message);

    MSysRegEvent(Line,0,0,1);
    }

  if (X.XMetaStoreCompletedJobInfo != (int (*)())0)
    {
    (*X.XMetaStoreCompletedJobInfo)(X.xd,J);
    }

  if (MPar[0].BFChunkDuration > 0)
    {
    MPar[0].BFChunkBlockTime = MSched.Time + MPar[0].BFChunkDuration;
    }

  /* create job statistics record */

  MJobWriteStats(J);

  if (X.XJobUpdate != (int (*)())0)
    {
    (*X.XJobUpdate)(X.xd,J);
    }

  /* adjust reservation */

  if (J->R != NULL)
    {
    if (J->ResName[0] != '\0')
      {
      /* handled in G2 */

      /* NYI */
      }
    }

  /* modify node expected state and dedicated resources */

  nindex = 0;

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];

    for (nindex = 0;nindex < MAX_MNODE_PER_JOB;nindex++)
      {
      if (RQ->NodeList[nindex].N == NULL)
        break;

      N = RQ->NodeList[nindex].N;

      DBG(3,fSTAT) DPrint("INFO:     node '%s' returned to idle pool\n",
        N->Name);

      if (nindex == MAX_MNODE_PER_JOB)
        {
        DBG(0,fSTAT) DPrint("ERROR:    job node buffer overflow in %s() on job '%s:%d' (iteration: %d)\n",
          FName,
          J->Name,
          rqindex,
          MSched.Iteration);

        break;
        }

      /* determine new expected state for node */

      if (MNodeCheckAllocation(N) != SUCCESS)
        {
        N->EState = mnsIdle;
        }
      else
        {
        N->EState = mnsActive;
        }

      if (N->EState == N->State)
        N->SyncDeadLine = MAX_MTIME;
      else
        N->SyncDeadLine = MSched.Time + MSched.NodeSyncDeadline;
      }  /* END for (nindex)  */
    }    /* END for (rqindex) */

  if (nindex == MAX_MNODE_PER_JOB)
    {
    DBG(0,fSTAT) DPrint("ERROR:    job node buffer overflow in %s() on job %s:%d (iteration: %d)\n",
      FName,
      J->Name,
      rqindex,
      MSched.Iteration);
    }

  /* handle conglomerate jobs */

  if (J->MasterJobName != NULL)
    {
    if (MJobFind(J->MasterJobName,&MJ,0) == FAILURE)
      {
      DBG(2,fSTAT) DPrint("WARNING:  cannot locate masterjob '%s' at completion of job '%s'\n",
        J->MasterJobName,
        J->Name);
      }

    ccount = 0;

    for (rqindex = 0;MJ->Req[rqindex] != NULL;rqindex++)
      {
      if (!strcmp(J->Name,MJ->Req[rqindex]->SubJobName))
        {
        MJ->Req[rqindex]->State = mjsCompleted;
        }

      if (MJ->Req[rqindex]->State == mjsCompleted)
        ccount++;
      }

    if (ccount == rqindex)
      {
      /* all subjobs completed.  cancel master job */

      MRMJobCancel(MJ,"MOAB_INFO:  job has completed\n",NULL);
      }
    }     /* END if (J->MasterJobName != NULL) */

  /* handle statistics */

  MStatUpdateCompletedJobUsage(J,0,0);

  /* provide feedback info to user */

  MJobSendFB(J);

  /* modify expected job state */

  J->EState = mjsCompleted;

  MJobRemove(J);

  return(SUCCESS);
  }  /* END MJobProcessCompleted() */





int MJobGetNRange(

  mjob_t   *J,              /* IN:  job                              */
  mreq_t   *RQ,             /* IN:  requirement checked              */
  mnode_t  *N,              /* IN:  node                             */
  long      StartTime,      /* IN:  time resources are requested     */
  int      *TasksAvailable, /* OUT: reservations available           */
  long     *WindowTime,     /* OUT: length of time node is available */
  char     *Affinity,       /* OUT: BOOLEAN: reservation affinity    */
  int      *Type,           /* OUT: type of reservation requested    */
  char     *BRes)

  {
  char    tmpAffinity;
  long    RangeTime;

  int MinTasks;
  int MaxTasks;
  int TaskCount;

  int MinTPN;

  mrange_t RRange[MAX_MRANGE];
  mrange_t ARange[MAX_MRANGE];

  int TI;
  int TP;

  int TC;

  int TPNCap;

  mcres_t ZRes;

  const char *FName = "MJobGetNRange";

  DBG(8,fSCHED) DPrint("%s(%s,RQ,%s,%ld,TA,WT,Aff,Type,BRes)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (N != NULL) ? N->Name : "NULL",
    StartTime);

  if ((J == NULL) || (RQ == NULL) || (N == NULL))
    {
    return(FAILURE);
    }

  MinTasks = 0;

  /* find only ranges which start at StartTime */

  RRange[0].StartTime = StartTime;
  RRange[0].EndTime   = StartTime;

  RRange[1].EndTime   = 0;

  if (!(J->Flags & (1 << mjfResMap)) &&
      !(J->Flags & (1 << mjfPreemptor)))
    {
    TC = MNodeGetTC(N,&N->ARes,&N->CRes,&N->DRes,&RQ->DRes,StartTime);
    }
  else
    {
    memset(&ZRes,0,sizeof(ZRes));

    TC = MNodeGetTC(N,&N->CRes,&N->CRes,&ZRes,&RQ->DRes,StartTime);
    }

  MaxTasks = MAX(RQ->TaskCount,RQ->TasksPerNode);
  MaxTasks = MIN(MaxTasks,TC);

  MinTPN = 0;

  if (RQ->TasksPerNode > 0)
    {
    MaxTasks -= MaxTasks % RQ->TasksPerNode;
    MinTPN    = RQ->TasksPerNode;
    }
  else
    {
    if ((N->RM != NULL) && (N->RM->Type == mrmtLL))
      {
      if (RQ->NodeCount > 0)
        {
        MinTPN = RQ->TaskCount / RQ->NodeCount;

        /* pessimistic */

/*
        if (RQ->TaskCount % RQ->NodeCount)
          MinTPN++;
*/
        }
      }
    }

  RangeTime   = 0;
  tmpAffinity = nmNone;

  if (Type != NULL)
    *Type = mrtNONE;

  if (RQ->TasksPerNode > 0)
    TPNCap = RQ->TasksPerNode - 1;
  else
    TPNCap = 0;

  DBG(8,fSCHED) DPrint("INFO:     TC: %d  Maxtasks: %d  MinTasks: %d (TPNCap: %d)\n",
    TC,
    MaxTasks,
    MinTasks,
    TPNCap);

  while (MaxTasks > MAX(MinTasks,TPNCap))
    {
    if (MaxTasks < MinTPN)
      {
      MaxTasks = 0;

      break;
      }

    TaskCount = (MaxTasks + MinTasks + 1) >> 1;

    if (RQ->TasksPerNode > 0)
      {
      TI = MinTasks / RQ->TasksPerNode;

      TP = RQ->TasksPerNode * (TI + 1);

      if (TP > TaskCount)
        TaskCount = TP;
      else
        TaskCount -= (TaskCount % RQ->TasksPerNode);
      }

    RRange[0].TaskCount = TaskCount;

    if (MJobGetSNRange(
          J,
          RQ,
          N,
          RRange,
          1,
          Affinity,
          Type,
          ARange,
          NULL,
          BRes) == FAILURE)
      {
      MaxTasks = TaskCount - 1;

      if (RQ->TasksPerNode > 0)
        MaxTasks -= (MaxTasks % RQ->TasksPerNode);
      }
    else
      {
      MinTasks  = ARange[0].TaskCount;

      RangeTime = MIN(MAX_MTIME,ARange[0].EndTime - ARange[0].StartTime);

      if (Affinity != NULL)
        tmpAffinity = *Affinity;
      }
    }    /* END while(MaxTasks > MinTasks) */

  if (WindowTime != NULL)
    *WindowTime = RangeTime;

  if (Affinity != NULL)
    *Affinity = tmpAffinity;

  if (TasksAvailable != NULL)
    {
    if (RQ->TasksPerNode > 0)
      MinTasks -= (MinTasks % RQ->TasksPerNode);

    *TasksAvailable = MinTasks;
    }

  if ((MaxTasks <= 0) || (RangeTime <= 0))
    {
    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MJobGetNRange() */




/* return up to <RCOUNT> ranges starting within the range <RRANGE> */

int MJobGetSNRange(

  mjob_t     *J,              /* I: job                              */
  mreq_t     *RQ,             /* I: requirement checked              */
  mnode_t    *N,              /* I: node                             */
  mrange_t   *RRange,         /* I: required range for resource availability */
  int         RCount,         /* I: number of ranges requested       */
  char       *Affinity,       /* O: node affinity for job            */
  int        *Type,           /* O: type of reservation requested    */
  mrange_t   *ARange,         /* O: available range list             */
  mcres_t    *DRes,           /* I: dedicated resources required     */
  char       *BRes)           /* O: blocking resid                   */

  {
  /* state variables */

  int eindex;
  int rindex;
  int index;
  int index2;

  long   RangeTime;
  long   JDuration;

  mcres_t AvlRes;
  mcres_t DedRes;
  mcres_t DQRes;

  int     TC;

  mres_t *R;
  int     RC;

  mcres_t *BR;

  /* booleans */

  int    PreRes;       
  int    Same;         
  int    UseDed;      
  int    ActiveRange;

  int    ResourcesAdded;  
  int    ResourcesRemoved; 

  int    PBlock;

  /* temp variables */

  long   Overlap;

  char   WCString[MAX_MNAME];

  long   ResOffset;

  int    IResCount;

  int    tmpTC;

  int    JC;  /* node job count */
  int    UJC; /* 'job per user' count */

  int    MJC;
  int    MUJC;

  mjob_t *tmpJ;

  const char *FName = "MJobGetSNRange";

  DBG(7,fSCHED) DPrint("%s(%s,%d,%s,(%d@%s),%d,%s,Type,ARange,BRes)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (RQ != NULL) ? RQ->Index : -1,
    N->Name,
    RRange[0].TaskCount,
    MULToTString(RRange[0].StartTime - MSched.Time),
    RCount,
    (Affinity == NULL) ? "NULL" : "Affinity");

  if ((J == NULL) || (RQ == NULL) || (N == NULL))
    {
    return(FAILURE);
    }

  MTRAPJOB(J,FName);
  MTRAPNODE(N,FName);

  DBG(8,fSCHED) DPrint("INFO:     attempting to get resources for %s %d * (P: %d  M: %d  S: %d  D: %d)\n",
    J->Name,
    RRange[0].TaskCount,
    RQ->DRes.Procs,
    RQ->DRes.Mem,
    RQ->DRes.Swap,
    RQ->DRes.Disk);

  memset(&ARange[0],0,sizeof(mrange_t));

  {
  char pval;

  UseDed  = FALSE;

  pval = mrapNONE;

  if ((N != NULL) && (N->NAvailPolicy != NULL))
    {
    pval = (N->NAvailPolicy[mrProc] != mrapNONE) ?
      N->NAvailPolicy[mrProc] : N->NAvailPolicy[0];
    }

  if (pval == mrapNONE)
    {
    pval = (MPar[0].NAvailPolicy[mrProc] != mrapNONE) ?
      MPar[0].NAvailPolicy[mrProc] : MPar[0].NAvailPolicy[0];
    }

  /* determine policy impact */

  if ((pval == mrapDedicated) || (pval == mrapCombined))
    {
    UseDed = TRUE;
    }
  }    /* END BLOCK */

  ResOffset = 0;

  /* NOTE:  modifications needed to keep inclusive reservations using
   *             only inclusive resources on SMP nodes
   *               */

  if (Type != NULL)
    *Type = mrtNONE;

  if (Affinity != NULL)
    *Affinity = nmNone;

  ARange[0].EndTime   = 0;
  ARange[0].TaskCount = 0;
  ARange[0].NodeCount = 0;

  DBG(6,fSCHED) MRECheck(N,"MJobGetSNRange-Start",TRUE);

  /* skip irrelevent node events */

  for (eindex = 0;eindex < (MSched.ResDepth << 1);eindex++)
    {
    if (N->RE[eindex].Type == mreNONE)
      break;

    R = N->R[N->RE[eindex].Index];

    if ((R->EndTime > RRange[0].StartTime) ||
        (R->EndTime >= MAX_MTIME - 1))
      break;

    DBG(6,fSCHED) DPrint("INFO:     skipping early MRE[%d] event for '%s' (E: %ld <= S: %ld) : P: %ld\n",
      eindex,
      R->Name,
      R->EndTime,
      RRange[0].StartTime,
      MSched.Time);
    }  /* END for (eindex) */

  JDuration = J->SpecWCLimit[0];

  if (N->RE[eindex].Type == mreNONE)
    {
    /* if no relevent reservations present */

    /* NOTE: node should be idle unless RRange[0].StartTime is in the future */
    /*       MNodeGetTC() should handle future evaluations and ignore ARes   */

    ARange[0].TaskCount =
      MNodeGetTC(N,&N->ARes,&N->CRes,&N->DRes,&RQ->DRes,
        MAX(MSched.Time,RRange[0].StartTime));

    ARange[0].NodeCount = 1;

    DBG(8,fSCHED) DPrint("INFO:     ARange[0] adjusted (TC: %d  S: %ld)\n",
      ARange[0].TaskCount,
      ARange[0].StartTime);

    if (ARange[0].TaskCount < RRange[0].TaskCount)
      {
      mcres_t tmpRes;

      DBG(6,fSCHED) DPrint("INFO:     node %s contains inadequate resources (State: %s)\n",
        N->Name,
        MNodeState[N->State]);

      /* NOTE:  re-calculate N->ARes as (N->CRes - N->DRes) */

      memcpy(&tmpRes,&N->CRes,sizeof(tmpRes));

      MCResRemove(&tmpRes,&N->CRes,&N->DRes,1,FALSE);

      tmpTC = MNodeGetTC(N,&tmpRes,&N->CRes,&N->DRes,&RQ->DRes,
        MAX(MSched.Time,RRange[0].StartTime));

      if (tmpTC >= RRange[0].TaskCount)
        {
        /* experiencing resource race condition */

        /* available resources do not match (cfg - ded) resources */

        /* allow reservation to proceed */

        ResOffset = MSched.RMPollInterval;
        }
      else
        {
        /* resources unavailable regardless of current usage */

        if (Affinity != NULL)
          *Affinity = nmUnavailable;

        return(FAILURE);
        }
      }    /* END if (ARange[0].TaskCount < RRange[0].TaskCount) */

    ARange[0].StartTime = MAX(RRange[0].StartTime,MSched.Time + ResOffset);
    ARange[0].EndTime   = MAX_MTIME;

    /* terminate range list */

    ARange[1].EndTime = 0;

    DBG(8,fSCHED)
      {
      strcpy(WCString,MULToTString(ARange[0].StartTime - MSched.Time));

      DPrint("INFO:     closing ARange[%d] (%s -> %s : %d) (0)\n",
        0,
        WCString,
        MULToTString(ARange[0].EndTime - MSched.Time),
        ARange[0].TaskCount);
      }

    strcpy(WCString,MULToTString(JDuration));

    DBG(5,fSCHED) DPrint("INFO:     node %s supports %d task%s of job %s:%d for %s of %s (no reservation)\n",
      N->Name,
      ARange[0].TaskCount,
      (ARange[0].TaskCount == 1) ? "" : "s",
      J->Name,
      RQ->Index,
      MULToTString(ARange[0].EndTime - ARange[0].StartTime),
      WCString);

    if (DRes != NULL)
      {
      memset(DRes,0,sizeof(mcres_t));

      MCResAdd(DRes,&N->CRes,&N->CRes,1,FALSE);
      }

    if (BRes != NULL)
      strcpy(BRes,"NONE");

    if (J->Flags & (1 << mjfAdvReservation))
      {
      if (Affinity != NULL)
        *Affinity = nmUnavailable;

      DBG(5,fSCHED) DPrint("INFO:     no reservation found for AdvRes job %s\n",
        J->Name);

      return(FAILURE);
      }
    else
      {
      if (ARange[0].TaskCount >= RRange[0].TaskCount)
        {
        return(SUCCESS);
        }
      else
        {
        return(FAILURE);
        }
      }
    }  /* END if (N->RE[eindex].Type == mreNONE) */

  /* initialize reservation state */

  IResCount = 0;

  ActiveRange = FALSE;
  PBlock      = FALSE;
  PreRes      = TRUE;
  rindex      = 0;

  JC = 0;
  UJC = 0;

  if ((J->Req[0]->NAccessPolicy == mnacSingleJob) ||
      (J->Req[0]->NAccessPolicy == mnacSingleTask))
    {
    MJC = 1;
    }
  else if (N->AP.HLimit[mptMaxJob][0] <= 0)
    {
    MJC = MSched.DefaultN.AP.HLimit[mptMaxJob][0];
    }
  else
    {
    MJC = N->AP.HLimit[mptMaxJob][0];
    }

  if (N->MaxJobPerUser <= 0)
    MUJC = MSched.DefaultN.MaxJobPerUser;
  else
    MUJC = N->MaxJobPerUser;

  /* initialize available resources */

  memcpy(&AvlRes,&N->CRes,sizeof(AvlRes));
  memset(&DedRes,0,sizeof(DedRes));

  memset(&DQRes,0,sizeof(DQRes));

  DQRes.Procs = -1;

  tmpTC = RRange[0].TaskCount;

  DBG(8,fSCHED)
    {
    DPrint("INFO:     initial resources: %s\n",
      MUCResToString(&AvlRes,0,0,NULL));

    DPrint("INFO:     requested resources: %s (x%d)\n",
      MUCResToString(&RQ->DRes,0,0,NULL),
      tmpTC);
    }

  /* consume required resources */

  MCResRemove(&AvlRes,&N->CRes,&RQ->DRes,tmpTC,FALSE);

  if (UseDed == TRUE)
    MCResAdd(&DedRes,&N->CRes,&RQ->DRes,tmpTC,FALSE);

  if (MUCResIsNeg(&AvlRes) == SUCCESS)
    {
    /* if inadequate resources are configured */

    DBG(6,fSCHED) DPrint("INFO:     inadequate resources configured\n");

    if (Affinity != NULL)
      *Affinity = nmUnavailable;

    return(FAILURE);
    }

  if (J->Flags & (1 << mjfNASingleJob))
    {
    /* all resources dedicated */

    memset(&AvlRes,0,sizeof(AvlRes));
    }

  if (DRes != NULL)
    {
    /* set up default structures */

    memset(DRes,0,sizeof(mcres_t));

    MCResAdd(DRes,&AvlRes,&N->CRes,1,FALSE);
    }

  ResourcesAdded   = TRUE;
  ResourcesRemoved = TRUE;

  /* step through node events */

  for (;eindex < (MSched.ResDepth << 1);eindex++)
    {
    if (N->RE[eindex].Type == mreNONE)
      break;

    R = N->R[N->RE[eindex].Index];

    if (R->EndTime <= RRange[0].StartTime)
      continue;

    if ((N->RE[eindex].Time > RRange[0].StartTime) &&
        (PreRes == TRUE))
      {
      /* adjust/check requirements at job start */

      DBG(7,fSCHED) DPrint("INFO:     performing starttime check (%d)\n",
        eindex);

      PreRes = FALSE;

      if ((MUCResIsNeg(&AvlRes) == SUCCESS) ||
          (PBlock == TRUE) ||
         ((J->Flags & (1 << mjfAdvReservation)) &&
          (IResCount <= 0)))
        {
        DBG(6,fSCHED) DPrint("INFO:     resources unavailable at time %s\n",
          MULToTString(RRange[0].StartTime - MSched.Time));

        /* no active range can exist if resource unavailable at starttime */

        ActiveRange = FALSE;

        /* 'starttime' range is not active */

        if (N->RE[eindex].Time >= RRange[0].EndTime)
          {
          /* range not active at time '0' and next event is too late to be considered */

          DBG(8,fSCHED) DPrint("INFO:     reservation extends beyond acceptable time range\n");

          break;
          }
        }
      else
        {
        TC = MNodeGetTC(N,&AvlRes,&N->CRes,&DedRes,&RQ->DRes,MSched.Time);

        ARange[rindex].TaskCount = RRange[0].TaskCount + TC;
        ARange[rindex].NodeCount = 1;
        ARange[rindex].StartTime = MAX(RRange[0].StartTime,MSched.Time + ResOffset);

        ActiveRange = TRUE;

        DBG(8,fSCHED) DPrint("INFO:     ARange[%d] started (TC: %d  S: %ld)\n",
          rindex,
          ARange[rindex].TaskCount,
          ARange[rindex].StartTime);

        if (DRes != NULL)
          MUCResGetMin(DRes,DRes,&AvlRes);
        }
      }   /* END if ((N->RE[eindex].Time > RRange[0].StartTime) && ... */

    if ((N->RE[eindex].Time > RRange[0].EndTime) &&
        (ActiveRange == FALSE))
      {
      break;
      }

    /* adjust resources */

    if (N->R[N->RE[eindex].Index]->Type != mrtUser)
      {
      /* NOTE:  DRes represents 'per task' blocked resources */

      BR = &N->R[N->RE[eindex].Index]->DRes;
      RC = N->RC[N->RE[eindex].Index];

      DBG(8,fSCHED) DPrint("INFO:     non-user reservation[%d] '%s'x%d resources %s\n",
        N->RE[eindex].Index,
        N->R[N->RE[eindex].Index]->Name,
        RC,
        MUCResToString(BR,0,0,NULL));
      }
    else
      {
      /* NOTE:  DRes represents 'total' blocked resources */

      BR = &N->RE[eindex].DRes;
      RC = 1;

      DBG(8,fSCHED) DPrint("INFO:     user reservation[%d] '%s'x%d resources %s\n",
        N->RE[eindex].Index,
        N->R[N->RE[eindex].Index]->Name,
        RC,
        MUCResToString(BR,0,0,NULL));
      }

    if (ActiveRange == TRUE)
      {
      /* be generous (ie, JDuration - (R->StartTime - ARange[rindex].StartTime) */

      Overlap =
        MIN(R->EndTime,ARange[rindex].StartTime + JDuration) -
        MAX(R->StartTime,ARange[rindex].StartTime);

      if (Overlap < 0)
        Overlap = R->EndTime - R->StartTime;
      }
    else
      {
      Overlap = R->EndTime - MAX(R->StartTime,RRange[0].StartTime);
      }

    Overlap = MIN(JDuration,Overlap);

    DBG(7,fSCHED) DPrint("INFO:     checking reservation %s %s at time %s (O: %ld)\n",
      N->R[N->RE[eindex].Index]->Name,
      (N->RE[eindex].Type == mreStart) ? "start" : "end",
      MULToTString(N->RE[eindex].Time - MSched.Time),
      Overlap);

    if (N->RE[eindex].Type == mreStart)
      {
      /* reservation start */

      if (MResCheckJAccess(R,J,Overlap,&Same,Affinity) == TRUE)
        {
        DBG(8,fSCHED) DPrint("INFO:     reservations are inclusive\n");

        IResCount++;

        if (J->Flags & (1 << mjfAdvReservation))
          ResourcesAdded = TRUE;
        }
      else
        {
        DBG(8,fSCHED) DPrint("INFO:     reservations are exclusive\n");

        tmpJ = (mjob_t *)R->J;

        if (tmpJ != NULL)
          {
          JC++;

          if (J->Cred.U == tmpJ->Cred.U)
            {
            UJC++;
            }
          }

        ResourcesRemoved = TRUE;

        if ((J->Req[0]->NAccessPolicy == mnacSingleUser) &&
            (tmpJ != NULL) &&
            (tmpJ->Cred.U != NULL) &&
            (J->Cred.U != NULL) &&
            (tmpJ->Cred.U != J->Cred.U) &&
            (strcmp(J->Cred.U->Name,"[ALL]")))
          {
          /* user dedicated resources removed */

          MCResRemove(&AvlRes,&N->CRes,&N->CRes,RC,FALSE);

          if (UseDed == TRUE)
            MCResAdd(&DedRes,&N->CRes,&N->CRes,RC,FALSE);
          }
        else
          {
          MCResRemove(&AvlRes,&N->CRes,BR,RC,FALSE);

          if (UseDed == TRUE)
            MCResAdd(&DedRes,&N->CRes,BR,RC,FALSE);
          }

        DBG(7,fSCHED)
          {
          DPrint("INFO:     removed resources: %s (x%d)\n",
            MUCResToString(BR,0,0,NULL),
            RC);

          DPrint("INFO:     resulting resources: %s\n",
            MUCResToString(&AvlRes,0,0,NULL));
          }

        if (BRes != NULL)
          strcpy(BRes,N->R[N->RE[eindex].Index]->Name);
        }
      }    /* END if (N->RE[eindex].Type == mreStart) */
    else if (N->RE[eindex].Type == mreEnd)
      {
      /* reservation end */

      if (MResCheckJAccess(R,J,Overlap,&Same,Affinity) == TRUE)
        {
        DBG(8,fSCHED) DPrint("INFO:     reservations are inclusive\n");

        IResCount--;

        ResourcesRemoved = TRUE;
        }
      else
        {
        DBG(8,fSCHED) DPrint("INFO:     reservations are exclusive\n");

        tmpJ = (mjob_t *)R->J;

        if (tmpJ != NULL)
          {
          JC--;

          if (J->Cred.U == tmpJ->Cred.U)
            {
            UJC--;
            }
          }

        ResourcesAdded = TRUE;

        if ((J->Req[0]->NAccessPolicy == mnacSingleUser) &&
            (tmpJ != NULL) &&
            (tmpJ->Cred.U != J->Cred.U))
          {
          /* user dedicated resources added */

          MCResAdd(&AvlRes,&N->CRes,&N->CRes,RC,FALSE);

          if (UseDed == TRUE)
            MCResRemove(&DedRes,&N->CRes,&N->CRes,RC,FALSE);
          }
        else
          {
          MCResAdd(&AvlRes,&N->CRes,BR,RC,FALSE);

          if (UseDed == TRUE)
            MCResRemove(&DedRes,&N->CRes,BR,RC,FALSE);
          }

        DBG(7,fSCHED)
          {
          DPrint("INFO:     added resources: %s (x%d)\n",
            MUCResToString(BR,0,0,NULL),
            RC);

          DPrint("INFO:     resulting resources: %s\n",
            MUCResToString(&AvlRes,0,0,NULL));
          }
        }
      }    /* END else (N->RE[eindex].Type == mreStart) */

    /* verify resources once per timestamp */

    if ((N->RE[eindex + 1].Type == mreNONE) ||
        (N->RE[eindex].Time != N->RE[eindex + 1].Time))
      {
      DBG(8,fSCHED) DPrint("INFO:     verifying resource access at %s (%d)\n",
        MULToTString(N->RE[eindex].Time - MSched.Time),
        eindex);

      if (((MJC > 0) && (JC >= MJC)) ||
          ((MUJC > 0) && (UJC >= MUJC)))
        {
        PBlock = TRUE;
        }
      else
        {
        PBlock = FALSE;
        }

      if ((R->StartTime == RRange[0].StartTime) && (PreRes == TRUE))
        {
        /* adjust/check requirements at job start */

        DBG(8,fSCHED) DPrint("INFO:     performing starttime check (%d)\n",
          eindex);

        PreRes = FALSE;

        if (MUCResIsNeg(&AvlRes) == SUCCESS)
          {
          DBG(7,fSCHED) DPrint("INFO:     resources unavailable at time %s\n",
            MULToTString(RRange[0].StartTime - MSched.Time));

          if (N->RE[eindex].Time <= RRange[0].StartTime)
            ActiveRange = FALSE;

          if (N->RE[eindex].Time >= RRange[0].EndTime)
            {
            /* end of required range reached */

            break;
            }
          }
        else if (PBlock == TRUE)
          {
          DBG(7,fSCHED) DPrint("INFO:     policy blocks access at time %s\n",
            MULToTString(RRange[0].StartTime - MSched.Time));

          if (N->RE[eindex].Time <= RRange[0].StartTime)
            ActiveRange = FALSE;

          if (N->RE[eindex].Time >= RRange[0].EndTime)
            {
            /* end of required range reached */

            break;
            }
          }
        else if (!(J->Flags & (1 << mjfAdvReservation)) ||
                  (IResCount > 0))
          {
          /* range is active unless advance reservation is required and not found */

          ActiveRange = TRUE;

          TC = MNodeGetTC(N,&AvlRes,&N->CRes,&DedRes,&RQ->DRes,MSched.Time);

          ARange[rindex].TaskCount = RRange[0].TaskCount + TC;
          ARange[rindex].NodeCount = 1;
          ARange[rindex].StartTime = MAX(RRange[0].StartTime,MSched.Time + ResOffset);

          DBG(8,fSCHED) DPrint("INFO:     ARange[%d] started (TC: %d  S: %ld)\n",
            rindex,
            ARange[rindex].TaskCount,
            ARange[rindex].StartTime);

          if (DRes != NULL)
            MUCResGetMin(DRes,DRes,&AvlRes);
          }
        }   /* END if ((R->StartTime >= RRange[0].StartTime) && ... */

      if ((ResourcesRemoved == TRUE) && (ActiveRange == TRUE))
        {
        ResourcesRemoved = FALSE;
        ResourcesAdded   = FALSE;

        if ((MUCResIsNeg(&AvlRes) == SUCCESS) ||
            (PBlock == TRUE) ||
           ((J->Flags & (1 << mjfAdvReservation)) &&
            (IResCount == 0)))
          {
          /* terminate active range */

          DBG(6,fSCHED) DPrint("INFO:     resources unavailable at time %s during reservation %s %s (%s)\n",
            MULToTString(N->RE[eindex].Time - MSched.Time),
            R->Name,
            (N->RE[eindex].Type == mreStart) ? "start" : "end",
            (PBlock == TRUE) ? "policy" : "resources");

          if (((N->RE[eindex].Time - ARange[rindex].StartTime) >= JDuration) ||
              ((rindex > 0) && (ARange[rindex].StartTime == ARange[rindex - 1].EndTime)))
            {
            /* if reservation is long enough */

            ARange[rindex].EndTime = N->RE[eindex].Time;

            DBG(8,fSCHED)
              {
              strcpy(WCString,MULToTString(ARange[rindex].StartTime - MSched.Time));

              DPrint("INFO:     closing ARange[%d] (%s -> %s : %d) (1)\n",
                rindex,
                WCString,
                MULToTString(ARange[rindex].EndTime - MSched.Time),
                ARange[rindex].TaskCount);
              }

            rindex++;

            if (rindex >= MAX_MRANGE)
              {
              DBG(6,fSCHED) DPrint("ALERT:    range overflow in %s(%s,%s)\n",
                FName,
                J->Name,
                N->Name);

              return(FAILURE);
              }
            }
          else
            {
            DBG(8,fSCHED) DPrint("INFO:     range too short (ignoring)\n");
            }

          ActiveRange = FALSE;

          if ((RCount == rindex) ||
              (N->RE[eindex].Time >= RRange[0].EndTime))
            {
            break;
            }
          }   /* END if (MUCResIsNeg(&AvlRes) || ... */
        else
          {
          /* check new taskcount */

          TC = MNodeGetTC(N,&AvlRes,&N->CRes,&DedRes,&RQ->DRes,MSched.Time);

          if (N->RE[eindex].Time >= RRange[0].StartTime)
            {
            /* if in 'active' time */

            if (((TC + RRange[0].TaskCount) != ARange[rindex].TaskCount) &&
                ((RQ->TaskCount == 0) ||
                 (TC + RRange[0].TaskCount < RQ->TaskCount) ||
                 (TC + RRange[0].TaskCount < ARange[rindex].TaskCount) ||
                 (ARange[rindex].TaskCount < RQ->TaskCount)))
              {
              if (N->RE[eindex].Time > ARange[rindex].StartTime)
                {
                /* create new range */

                ARange[rindex].EndTime = N->RE[eindex].Time;

                DBG(8,fSCHED)
                  {
                  strcpy(WCString,MULToTString(ARange[rindex].StartTime - MSched.Time));

                  DPrint("INFO:     closing ARange[%d] (%s -> %s : %d) (2)\n",
                    rindex,
                    WCString,
                    MULToTString(ARange[rindex].EndTime - MSched.Time),
                    ARange[rindex].TaskCount);
                  }

                rindex++;

                if (rindex >= MAX_MRANGE)
                  {
                  DBG(1,fSCHED) DPrint("ALERT:    range overflow in %s(%s,%s)\n",
                    FName,
                    J->Name,
                    N->Name);

                  return(FAILURE);
                  }
                }    /* END if (N->RE[eindex].Time > ARange[rindex].StartTime) */

              ARange[rindex].StartTime = MAX(N->RE[eindex].Time,MSched.Time + ResOffset);
              ARange[rindex].TaskCount = RRange[0].TaskCount + TC;
              ARange[rindex].NodeCount = 1;

              if (DRes != NULL)
                MUCResGetMin(DRes,DRes,&AvlRes);
              }  /* END if (((TC + RRange[0].TaskCount) != ARange[rindex].TaskCount) */
            }    /* END if (N->RE[eindex].Time >= RRange[0].StartTime) */
          else if ((RRange[0].TaskCount + TC) < ARange[rindex].TaskCount)
            {
            /* in 'pre-active' time */

            DBG(6,fSCHED) DPrint("INFO:     adjusting 'preactive' ARange[%d] taskcount from %d to %d\n",
              rindex,
              ARange[rindex].TaskCount,
              RRange[0].TaskCount + TC);

            ARange[rindex].TaskCount = RRange[0].TaskCount + TC;
            }
          else
            {
            DBG(6,fSCHED) DPrint("INFO:     ARange[%d] taskcount not affected by reservation change\n",
              rindex);
            }
          }   /* END else (MUCResIsNeg(&AvlRes) || ... */
        }
      else if ((ResourcesAdded == TRUE) && (ActiveRange == TRUE))
        {
        int NewRange = FALSE;

        /* adjust taskcount, create new range */

        TC = MNodeGetTC(N,&AvlRes,&N->CRes,&DedRes,&RQ->DRes,MSched.Time);

        if ((TC + RRange[0].TaskCount) > ARange[rindex].TaskCount)
          {
          /* range has grown */

          NewRange = TRUE;
          }
        else if (((TC + RRange[0].TaskCount) < ARange[rindex].TaskCount) &&
            ((RQ->TaskCount == 0) ||
             (TC + RRange[0].TaskCount < RQ->TaskCount) ||
             (ARange[rindex].TaskCount < RQ->TaskCount)))
          {
          /* range is too small */

          NewRange = TRUE;
          }

        if (NewRange == TRUE)
          { 
          ARange[rindex].EndTime = N->RE[eindex].Time;

          DBG(8,fSCHED)
            {
            strcpy(WCString,MULToTString(ARange[rindex].StartTime - MSched.Time));

            DPrint("INFO:     closing ARange[%d] (%s -> %s : %d) (3)\n",
              rindex,
              WCString,
              MULToTString(ARange[rindex].EndTime - MSched.Time),
              ARange[rindex].TaskCount);
            }

          rindex++;

          if (rindex >= MAX_MRANGE)
            {
            DBG(6,fSCHED) DPrint("ALERT:    range overflow in %s(%s,%s)\n",
              FName,
              J->Name,
              N->Name);

            return(FAILURE);
            }

          ARange[rindex].StartTime = MAX(N->RE[eindex].Time,MSched.Time + ResOffset);

          ARange[rindex].TaskCount = RRange[0].TaskCount + TC;

          ARange[rindex].NodeCount = 1;
          }  /* END if (((TC + RRange[0].TaskCount) != ...) */
        }    /* END else if ((ResourcesAdded == TRUE) && (ActiveRange == TRUE)) */
      else if ((ResourcesAdded == TRUE) && (ActiveRange == FALSE))
        {
        ResourcesAdded   = FALSE;
        ResourcesRemoved = FALSE;

        if ((MUCResIsNeg(&AvlRes) == FAILURE) &&
           (!(J->Flags & (1 << mjfAdvReservation)) ||
           (IResCount > 0)) &&
           (PBlock == FALSE))
          {
          /* initiate active range */

          DBG(6,fSCHED) DPrint("INFO:     resources available at time %s during %s %s\n",
            MULToTString(N->RE[eindex].Time - MSched.Time),
            R->Name,
            (N->RE[eindex].Type == mreStart) ? "start" : "end");

          ARange[rindex].StartTime = MAX(MAX(RRange[0].StartTime,N->RE[eindex].Time),MSched.Time + ResOffset);

          /* adjust taskcount */

          TC = MNodeGetTC(N,&AvlRes,&N->CRes,&DedRes,&RQ->DRes,MSched.Time);

          ARange[rindex].TaskCount = RRange[0].TaskCount + TC;

          ARange[rindex].NodeCount = 1;

          DBG(8,fSCHED) DPrint("INFO:     ARange[%d] adjusted (TC: %d  S: %ld)\n",
            rindex,
            ARange[rindex].TaskCount,
            ARange[rindex].StartTime);

          ActiveRange = TRUE;

          if (DRes != NULL)
            {
            MUCResGetMin(DRes,DRes,&AvlRes);
            }
          }   /* END if (MUCResIsNeg())                                */
        }     /* END else if (N->RE[eindex].Type == mreEnd)            */
      }       /* END if (N->RE[eindex].Time != N->RE[eindex + 1].Time) */
    }         /* END for (eindex)                                      */

  if (ActiveRange == TRUE)
    {
    ARange[rindex].EndTime = MAX_MTIME;

    DBG(8,fSCHED)
      {
      strcpy(WCString,MULToTString(ARange[rindex].StartTime - MSched.Time));

      DPrint("INFO:     closing ARange[%d] (%s -> %s : %d) (4)\n",
        rindex,
        WCString,
        MULToTString(ARange[rindex].EndTime - MSched.Time),
        ARange[rindex].TaskCount);
      }

    rindex++;

    if (rindex >= MAX_MRANGE)
      {
      DBG(6,fSCHED) DPrint("ALERT:    range overflow in %s(%s,%s)\n",
        FName,
        J->Name,
        N->Name);

      return(FAILURE);
      }
    }  /* END if (ActiveRange == TRUE) */

  /* terminate range list */

  ARange[rindex].EndTime = 0;

  rindex = 0;

  /* NOTE:  RRange,ARange are availability ranges, not start ranges */

  for (index = 0;ARange[index].EndTime != 0;index++)
    {
    /* enforce time constraints */

    if ((MPar[0].NodeDownStateDelayTime > 0) &&
       ((N->State == mnsDrained) ||
        (N->State == mnsDown) ||
       ((N->State == mnsBusy) && (N->EState == mnsIdle))))
      {
      ARange[index].StartTime = MAX(
        ARange[index].StartTime,
        MIN(ARange[index].EndTime,MSched.Time + MPar[0].NodeDownStateDelayTime));
      }

    if (ARange[index].EndTime < RRange[0].StartTime)
      {
      DBG(6,fSCHED) DPrint("INFO:     ARange[%d] too early for job %s (E: %ld < S: %ld  P: %ld): removing range\n",
        index,
        J->Name,
        ARange[index].EndTime,
        RRange[0].StartTime,
        MSched.Time);

      ARange[index].TaskCount = 0;
      }
    else if (ARange[index].StartTime < RRange[0].StartTime)
      {
      DBG(6,fSCHED) DPrint("INFO:     ARange[%d] too early for job %s (S: %ld < S: %ld): truncating range\n",
        index,
        J->Name,
        ARange[index].StartTime,
        RRange[0].StartTime);

      ARange[index].StartTime = RRange[0].StartTime;
      }

    if (ARange[index].TaskCount < RRange[0].TaskCount)
      {
      DBG(6,fSCHED) DPrint("INFO:     ARange[%d] too small for job %s (%d < %d):  removing range\n",
        index,
        J->Name,
        ARange[index].TaskCount,
        RRange[0].TaskCount);
      }
    else
      {
      /* adequate range found */

      if (index != rindex)
        {
        memcpy(&ARange[rindex],&ARange[index],sizeof(ARange[index]));
        }

      rindex++;
      }
    }    /* END for (index) */

  ARange[rindex].EndTime = 0;

  rindex = 0;

  /* second pass:  remove ranges too short */

  for (index = 0;ARange[index].EndTime != 0;index++)
    {
    /* collapse short ranges */

    RangeTime = ARange[index].EndTime - ARange[index].StartTime;

    if (RangeTime < JDuration)
      {
      /* append prior ranges if immediate */

      for (index2 = index - 1;index2 >= 0;index2--)
        {
        if (ARange[index2].EndTime != ARange[index2 + 1].StartTime)
          break;

        RangeTime += (ARange[index2].EndTime - ARange[index2].StartTime);

        if (RangeTime >= JDuration)
          break;
        }

      if (RangeTime < JDuration)
        {
        /* append later ranges if immediate */

        for (index2 = index + 1;ARange[index2].EndTime != 0;index2++)
          {
          if (ARange[index2 - 1].EndTime != ARange[index2].StartTime)
            break;

          RangeTime += (ARange[index2].EndTime - ARange[index2].StartTime);

          if (RangeTime >= JDuration)
            break;
          }
        }
      }  /* END if (RangeTime < JDuration) */

    if (ARange[index].StartTime > RRange[0].EndTime)
      {
      DBG(6,fSCHED) DPrint("INFO:     ARange[%d] (%ld -> %ld)x%d too late for job %s by %ld\n",
        index,
        ARange[index].StartTime,
        ARange[index].EndTime,
        ARange[index].TaskCount,
        J->Name,
        ARange[index].StartTime - RRange[0].EndTime);

      /* can range be merged with previous range? */

      /* if previous range is too short, merge ranges */

      /* are long ranges or wide ranges sought? */

      if ((rindex > 0) &&
          (ARange[rindex - 1].EndTime == ARange[index].StartTime)
#ifndef __MSEEKLONGRANGE
       && ((ARange[rindex].TaskCount >= ARange[rindex - 1].TaskCount) ||
          ((ARange[rindex - 1].EndTime - ARange[rindex - 1].StartTime) < JDuration))
#endif /* __MSEEKLONGRANGE */
          )
        {
        ARange[rindex - 1].EndTime   = ARange[index].EndTime;
        ARange[rindex - 1].TaskCount =
          MIN(ARange[rindex - 1].TaskCount,ARange[index].TaskCount);
        }

      ARange[index].TaskCount = 0;
      }
    else if ((RRange[0].EndTime > RRange[0].StartTime) &&
             (ARange[index].EndTime > RRange[0].EndTime))
      {
      /* HACK:  allow backfill windows to be located */

      DBG(6,fSCHED) DPrint("INFO:     ARange[%d] too late for job %s (E: %ld > E: %ld  P: %ld): truncating range\n",
        index,
        J->Name,
        ARange[index].EndTime,
        RRange[0].EndTime,
        MSched.Time);

      ARange[index].EndTime = RRange[0].EndTime;
      }

    if (RangeTime < MAX(1,JDuration))
      {
      DBG(6,fSCHED) DPrint("INFO:     ARange[%d] too short for job %s (MR: %ld < W: %ld):  removing range\n",
        index,
        J->Name,
        RangeTime,
        JDuration);

      ARange[index].TaskCount = 0;
      }

    if (ARange[index].TaskCount == 0)
      {
      /* remove range */
      }
    else
      {
      if (index != rindex)
        {
        memcpy(&ARange[rindex],&ARange[index],sizeof(ARange[index]));
        }

      rindex++;
      }
    }  /* END for (index = 0;ARange[index].EndTime != 0;index++) */

  ARange[rindex].EndTime = 0;

  if ((ARange[0].EndTime == 0) || 
      (ARange[0].TaskCount == 0))
    {
    DBG(6,fSCHED) DPrint("INFO:     node %s unavailable for job %s at %s\n",
      N->Name,
      J->Name,
      MULToTString(RRange[0].StartTime - MSched.Time));

    if (Affinity != NULL)
      *Affinity = nmUnavailable;

    return(FAILURE);
    }

  if (J->ResName[0] != '\0')
    {
    if ((Affinity != NULL) &&
       ((*Affinity != nmPositiveAffinity) &&
        (*Affinity != nmNeutralAffinity) &&
        (*Affinity != nmNegativeAffinity)))
      {
      if (Affinity != NULL)
        *Affinity = nmUnavailable;

      return(FAILURE);
      }
    }

  if (Affinity != NULL)
    {
    if ((J->Flags & (1 << mjfHostList)) && (J->ReqHLMode != mhlmSuperset))
      {
      if (J->ReqHLMode != mhlmSubset)
        {
        *Affinity = nmRequired;
        }
      else
        {
        int hindex;

        /* look for match */

        for (hindex = 0;J->ReqHList[hindex].N != NULL;hindex++)
          {
          if (J->ReqHList[hindex].N == N)
            {
            *Affinity = nmRequired;

            break;
            }
          }    /* END for (hindex) */
        }
      }
    }          /* END if (Affinity != NULL) */

  DBG(6,fSCHED)
    {
    for (rindex = 0;ARange[rindex].EndTime != 0;rindex++)
      {
      strcpy(WCString,MULToTString(ARange[rindex].StartTime - MSched.Time));

      DPrint("INFO:     node %s supports %d task%c of job %s:%d for %s at %s\n",
        N->Name,
        ARange[rindex].TaskCount,
        (ARange[rindex].TaskCount == 1) ? ' ' : 's',
        J->Name,
        RQ->Index,
        MULToTString(ARange[rindex].EndTime - ARange[rindex].StartTime),
        WCString);
      }
    }

  /* add required resources to min resource available */

  if (DRes != NULL)
    {
    MCResAdd(DRes,&N->CRes,&RQ->DRes,RRange[0].TaskCount,FALSE);
    }

  return(SUCCESS);
  } /* END MJobGetSNRange() */




int MJobProximateMNL(

  mjob_t      *J,
  mnodelist_t  SMNL,
  mnodelist_t  DMNL,
  long         StartTime,
  int          DoExactMatch)

  {
  int  nindex;
  int  nodeindex;

  int  sindex;
  int  index;

  int  TotalTasks;
  int  TaskCount;

  int  MinSets;

  _MALPSet SetList[MAX_MATTR];

  int  MaxSet;

  int  rc;

  mnodelist_t tmpMNL;

  mnalloc_t *SNL;
  mnalloc_t *DNL;

  mnode_t *N;

  const char *FName = "MJobProximateMNL";

  DBG(2,fSCHED) DPrint("%s(%s,MSNL,MDNL,%ld,%d)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    StartTime,
    DoExactMatch);

  if (J == NULL)
    return(FAILURE);

  if (SMNL == DMNL)
    {
    memcpy(tmpMNL,SMNL,sizeof(tmpMNL));
    }

  switch(MSched.AllocLocalityPolicy)
    {
    case malpRMSpecific:

      rc = MRMJobGetProximateMNL(
        J,
        (J->RM != NULL) ? J->RM : &MRM[0],
        (SMNL != DMNL) ? SMNL : tmpMNL,
        DMNL,
        StartTime,
        DoExactMatch,
        NULL,
        NULL);

      return(rc);

      /*NOTREACHED*/

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(MSched.AllocLocalityPolicy) */

  /* NOTE:  only handles single req! */

  SNL = (mnalloc_t *)SMNL[0];
  DNL = (mnalloc_t *)DMNL[0];

  MaxSet = -1;

  memset(SetList,0,sizeof(SetList));

  for (index = 0;index < MAX_MATTR;index++)
    SetList[index].Index = index;
  
  switch(MSched.AllocLocalityGroup)
    {
    case mxoFrame:
    default:

      for (nindex = 0;SNL[nindex].N != NULL;nindex++)
        {
        N = SNL[nindex].N;
     
        if (N->FrameIndex == -1)
          continue;

        SetList[N->FrameIndex].Size += SNL[nindex].TC;

        MaxSet = MAX(MaxSet,N->FrameIndex);
        }  /* END for (nindex); */

      break;
    }  /* END switch(MSched.AllocLocalityGroup) */

  if ((nindex == 0) || (MaxSet == -1))
    {
    return(FAILURE);
    }

  DBG(2,fSCHED)
    {
    DBG(2,fSCHED) DPrint("INFO:     set summary:  (total nodes: %3d)\n",
      nindex);

    for (sindex = 0;sindex < MAX_MATTR;sindex++)
      {
      if (SetList[sindex].Size)
        {
        fprintf(mlog.logfp,"[%2d : %2d]",
          SetList[sindex].Index,
          SetList[sindex].Size);
        }
      }  /* END for (sindex) */

    fprintf(mlog.logfp,"\n");
    }

  /* sort by set size (H -> L) */

  qsort(
    (void *)SetList,
    MaxSet + 1,
    sizeof(_MALPSet),
    (int(*)(const void *,const void *))__MALPSetCompHToL);

  DBG(4,fSCHED)
    {
    DBG(4,fSCHED) DPrint("INFO:     sorted set summary:\n");

    for (sindex = 0;sindex < MAX_MATTR;sindex++)
      {
      if (SetList[sindex].Size > 0)
        {
        fprintf(mlog.logfp,"[%2d : %2d]",
          SetList[sindex].Index,
          SetList[sindex].Size);
        }  /* END for (sindex) */
      }

    fprintf(mlog.logfp,"\n");
    }

  /* determine minimum set count required by job */

  TotalTasks = 0;

  for (sindex = 0;sindex < MAX_MATTR;sindex++)
    {
    TotalTasks += SetList[sindex].Size;

    if (TotalTasks >= J->Request.TC)
      {
      sindex++;

      break;
      }
    }    /* END for (sindex) */

  MinSets = sindex;

  if (TotalTasks < MJobGetProcCount(J))
    {
    /* NOTE:  inadequate tasks located in all sets */

    DBG(4,fSCHED) DPrint("INFO:     TC available: %d  procs required: %d\n",
      TotalTasks,
      J->Request.TC);

    return(FAILURE);
    }

  DBG(4,fSCHED) DPrint("INFO:     job '%s' requires %d tasks in %d sets\n",
    J->Name,
    J->Request.TC,
    MinSets);

  DBG(4,fSCHED) DPrint("INFO:     adequate tasks found: %d tasks in %d sets\n",
    TotalTasks,
    MinSets);

  /* determine 'optimal' set count */

  {
  /* 
  int OptimalSetCount;
  */

  /* NYI */

  /* NEED INFO ON TASK TO SET MAPPING FOR UNAVAILABLE NODES */

  /* CANNOT ASSUME 'PROC=TASK' OR 'ALL FRAMES SUPPORT JOB' */

  /* fail if current allocation uses more frames than optimal solution */

  /*
  if (MinSets > (OptimalSetCount + MSched.AllocLocalityMargin))
    {
    DBG(4,fSCHED) DPrint("INFO:     inadequate locality found for job '%s' (max sets found %d (O: %d)\n",
      J->Name,
      MinSets,
      OptimalSetCount);

    return(FAILURE);
    }
  */
  }  /* END BLOCK */

  DBG(4,fSCHED) DPrint("INFO:     job '%s' found %d tasks in %d sets (adequate locality)\n",
    J->Name,
    J->Request.TC,
    MinSets);

  /* copy selected nodes into DNL */

  nodeindex = 0;
  TaskCount = 0;

  for (nindex = 0;SNL[nindex].N != NULL;nindex++)
    {
    N = SNL[nindex].N;

    switch(MSched.AllocLocalityGroup)
      {
      case mxoFrame:
      default:

        for (sindex = 0;sindex < MinSets;sindex++)
          {
          if (N->FrameIndex != SetList[sindex].Index)
            continue;

          DBG(4,fSCHED)
            {
            fprintf(mlog.logfp,"[%2d : %8s]",
              nodeindex,
              N->Name);
            }

          DNL[nodeindex].N  = N;
          DNL[nodeindex].TC = SNL[nindex].TC;

          TaskCount += SNL[nindex].TC;
          nodeindex++;

          if (TaskCount >= TotalTasks)
            break;
          }  /* END for (sindex) */
 
        break;
      }  /* END switch(MSched.AllocLocalityGroup) */
    }    /* END for (nindex) */

  DBG(4,fSCHED)
    fprintf(mlog.logfp,"\n");

  if (TaskCount < J->Request.TC)
    {
    DBG(0,fSCHED) DPrint("ERROR:    only obtained %d of %d tasks for job '%s' in %s()\n",
      TaskCount,
      J->Request.TC,
      J->Name,
      FName);

    return(FAILURE);
    }

  DNL[nodeindex].N = NULL;

  DNL = (mnalloc_t *)DMNL[1];

  DNL[0].N = NULL;

  return(SUCCESS);
  } /* END MJobProximateMNL() */




int __MALPSetCompHToL(

  _MALPSet *a,
  _MALPSet *b)

  {
  /* order high to low */

  return(b->Size - a->Size);
  }  /* END __MALPSetCompHToL() */




int MReqRResFromString(

  mjob_t *J,
  mreq_t *RQ,
  char   *ResString,
  int     RMType,
  int     IsPref)

  {
  int   index;

  int   CIndex;
  int   RMIndex;

  char *ptr;
  char *TokPtr;

  char  AName[MAX_MLINE];
  char  ValLine[MAX_MLINE];

  enum MReqRsrcEnum {
    mrrNONE = 0,
    mrrArch,
    mrrADisk,
    mrrAMem,
    mrrAProcs,
    mrrASwap,
    mrrCDisk,
    mrrCMem,
    mrrCProcs,
    mrrCSwap,
    mrrFeature,
    mrrHost,
    mrrNetwork,
    mrrOpSys,
    mrrRMPool };

  #define MAX_MRMRRESTYPE  4

  int RMList[] = { mrmtLL, mrmtLSF, mrmtWiki, -1 };

  typedef struct {
    int   RRIndex;
    char *RName[MAX_MRMRRESTYPE];
    } mrrlist_t;

  /*                  LL              LSF        Wiki        Other1 */

  const mrrlist_t  MRMRRes[] = {
    { mrrNONE,      { NULL,           NULL,      NULL,       NULL }},
    { mrrArch,      { "Arch",         "model",   NULL,       NULL }},
    { mrrADisk,     { NULL,           "tmp",     NULL,       NULL }},
    { mrrAMem,      { NULL,           "mem",     NULL,       NULL }},
    { mrrAProcs,    { NULL,           NULL,      NULL,       NULL }},
    { mrrASwap,     { "Swap",         "swap",    NULL,       NULL }},
    { mrrASwap,     { NULL,           "swp",     NULL,       NULL }},
    { mrrCDisk,     { "Disk",         "maxtmp",  "DISK",     NULL }},
    { mrrCMem,      { "Memory",       "maxmem",  "MEM",      NULL }},
    { mrrCProcs,    { NULL,           "ncpus",   "PROC",     NULL }},
    { mrrCSwap,     { NULL,           "maxswap", "SWAP",     NULL }},
    { mrrFeature,   { "Feature",      NULL,      "FEATURE",  NULL }},
    { mrrHost,      { "Machine",      "hname",   NULL,       NULL }},
    { mrrNetwork,   { "Adapter",      NULL,      NULL,       NULL }},
    { mrrOpSys,     { "OpSys",        "type",    NULL,       NULL }},
    { mrrRMPool,    { "Pool",         NULL,      NULL,       NULL }},
    { 0,            { NULL,           "cpu",     NULL,       NULL }},
    { 0,            { NULL,           "io",      NULL,       NULL }},
    { 0,            { NULL,           "logins",  NULL,       NULL }},
    { 0,            { NULL,           "ls",      NULL,       NULL }},
    { 0,            { NULL,           "idle",    NULL,       NULL }},
    { 0,            { NULL,           "status",  NULL,       NULL }},
    { 0,            { NULL,           "it",      NULL,       NULL }},
    { 0,            { NULL,           "pg",      NULL,       NULL }},
    { 0,            { NULL,           "r15m",    NULL,       NULL }},
    { 0,            { NULL,           "r15s",    NULL,       NULL }},
    { 0,            { NULL,           "ut",      NULL,       NULL }},
    { 0,            { NULL,           "cpuf",    NULL,       NULL }},
    { 0,            { NULL,           "r1m",     NULL,       NULL }},
    { 0,            { NULL,           "ndisks",  NULL,       NULL }},
    { -1,           { NULL,           NULL,      NULL,       NULL }}};

  const char *FName = "MReqRResFromString";

  DBG(5,fSCHED) DPrint("%s(%s,RQ,%s,%d)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (ResString != NULL) ? ResString : "NULL",
    IsPref);

  if ((J == NULL) || (RQ == NULL) || (ResString == NULL))
    {
    return(FAILURE);
    }

  /* locate RM type */

  RMIndex = -1;

  for (index = 0;index < MAX_MRMRRESTYPE;index++)
    {
    if (RMList[index] == RMType)
      {
      RMIndex = index;

      break;
      }
    }  /* END for (index) */

  if (RMIndex == -1)
    {
    /* cannot locate requested RM type */

    RMIndex = mrmtWiki;
    }

  switch(RMType)
    {
    case mrmtLSF:

      /* FORMAT:  expr <REQ>... */
      /* FORMAT:  REQ:  <ATTR>()<CMP><VAL> */

      if (!strcmp(ResString,"expr 1"))
        {
        /* no resource requirements */

        return(SUCCESS);
        }

      if (!strncmp(ResString,"expr",strlen("expr")))
        {
        for (index = 0;index < strlen("expr");index++)
          ResString[index] = ' ';
        }

      while ((ptr = strstr(ResString,"()")) != NULL)
        {
        ptr[0] = ' ';
        ptr[1] = ' ';
        }

      break;
 
    default:

      /* NO-OP */

      break;
    }  /* END switch(RMType) */

  if (IsPref == FALSE)
    {
    /* set defaults */

    memset(RQ->ReqFBM,0,sizeof(RQ->ReqFBM));

    RQ->RequiredMemory = 0;
    RQ->RequiredSwap   = 0;
    RQ->RequiredDisk   = 0;
    RQ->DiskCmp        = mcmpGE;
    RQ->MemCmp         = mcmpGE;

    RQ->Opsys          = 0;
    RQ->Arch           = 0;
    RQ->Network        = 0;
    }  /* END is (IsPref == FALSE) */

  /* parse string by boolean operators */

  /* FORMAT: <REQ>[{ && | || } <REQ> ] ... */

  /* FORMAT: REQ -> [(]<ATTR>[<WS>]<CMP>[<WS>]["]<VAL>["]<WS>[)] */
  /* FORMAT: REQ -> <FEATURE> */

  /* NOTE:  currently do not support 'or' booleans.  all req's 'and'd */

  ptr = MUStrTok(ResString,"&|",&TokPtr);

  while (ptr != NULL)
    {
    if (MUParseComp(ptr,AName,&CIndex,ValLine) == FAILURE)
      {
      DBG(0,fSCHED) DPrint("ALERT:    cannot parse req line for job '%s'\n",
        J->Name);

      ptr = MUStrTok(NULL,"&|",&TokPtr);

      continue;
      }

    /* determine attr type */

    for (index = 0;MRMRRes[index].RRIndex != -1;index++)
      {
      if ((MRMRRes[index].RName[RMIndex] != NULL) &&
          !strcmp(MRMRRes[index].RName[RMIndex],AName))
        {
        break;
        }
      }  /* END for (index) */

    if (MRMRRes[index].RRIndex == 0)
      {
      /* ignore resource request */
 
      continue;
      }

    if (MRMRRes[index].RRIndex == -1)
      {
      if (MUGetMAttr(eFeature,AName,mVerify,RQ->ReqFBM,sizeof(RQ->ReqFBM)) != SUCCESS)
        {
        DBG(0,fSCHED) DPrint("ALERT:    cannot identify resource type '%s' for job '%s'\n",
          AName,
          J->Name);
        }
      else
        {
        DBG(3,fSCHED) DPrint("INFO:     feature request %s added to job %s resource requirements\n",
          AName,
          J->Name);
        }

      ptr = MUStrTok(NULL,"&|",&TokPtr);

      continue;
      }  /* END if (MRMRRes[index].RRIndex == -1) */

    if (IsPref == TRUE)
      {
      DBG(6,fSCHED) DPrint("INFO:     job %s requests resource preference %s %s %s\n",
        J->Name,
        AName,
        MComp[CIndex],
        ValLine);

      switch(MRMRRes[index].RRIndex)
        {
        case mrrFeature:

          if (CIndex == mcmpEQ)
            MUGetMAttr(eFeature,ValLine,mAdd,RQ->PrefFBM,sizeof(RQ->PrefFBM));

          break;

        default:

          /* NO-OP */

          break;
        }  /* END switch(MRMRRes[index].RRIndex) */

      ptr = MUStrTok(NULL,"&|",&TokPtr);

      continue;
      }  /* END if (IsPref == TRUE) */

    DBG(6,fSCHED) DPrint("INFO:     job %s requests resource constraints %s %s %s\n",
      J->Name,
      AName,
      MComp[CIndex],
      ValLine);

    switch(MRMRRes[index].RRIndex)
      {
      case mrrArch:

        if ((ValLine[0] != '\0') && strcmp(ValLine,"any") && (CIndex == mcmpEQ))
          {
          if ((RQ->Arch = MUMAGetIndex(eArch,ValLine,mAdd)) == FAILURE)
            {
            DBG(1,fSCHED) DPrint("WARNING:  job '%s' does not have valid Arch value '%s'\n",
              J->Name,
              ValLine);
            }
          }

        break;

      case mrrADisk:

	RQ->DRes.Disk = (int)strtol(ValLine,NULL,0);

        break;

      case mrrAMem:

        RQ->DRes.Mem = (int)strtol(ValLine,NULL,0);

        break;

      case mrrAProcs:

        RQ->DRes.Procs = (int)strtol(ValLine,NULL,0);

        break;

      case mrrASwap:

        RQ->DRes.Swap = (int)strtol(ValLine,NULL,0);

        break;

      case mrrCDisk:

        RQ->DiskCmp      = CIndex;
        RQ->RequiredDisk = (int)strtol(ValLine,NULL,0) / 1024;

        break;

      case mrrCMem:

        RQ->MemCmp         = CIndex;
        RQ->RequiredMemory = (int)strtol(ValLine,NULL,0);

        break;

      case mrrCProcs:

        RQ->ProcCmp        = CIndex;
        RQ->RequiredProcs  = (int)strtol(ValLine,NULL,0);

        break;

      case mrrCSwap:

        RQ->SwapCmp      = CIndex;
        RQ->RequiredSwap = (int)strtol(ValLine,NULL,0);

        break;

      case mrrFeature:

        if (CIndex == mcmpEQ)
          MUGetMAttr(eFeature,ValLine,mAdd,RQ->ReqFBM,sizeof(RQ->ReqFBM));

        break;

      case mrrHost:

	if (CIndex == mcmpEQ)
          {
          if (MJobSetAttr(
                J,
                mjaHostList,
                (void **)ValLine,
                mdfString,
                mAdd) == FAILURE)
            {
            char tmpLine[MAX_MLINE];

            /* invalid hostlist required */

            sprintf(tmpLine,"invalid hostlist requested '%.64s'",
              ValLine);

            /* set job hold */

            MJobSetHold(
              J,
              (1 << mhDefer),
              MSched.DeferTime,
              mhrNoResources,
              tmpLine);
            }

          J->ReqHLMode = mhlmSubset;
          }
        else if (CIndex == mcmpNE)
          {
          MJobSetAttr(
            J,
            mjaExcHList,
            (void **)ValLine,
            mdfString,
            mAdd);
          }
        else
          {
          /* invalid hostlist requirement */
 
          DBG(1,fSCHED) DPrint("WARNING:  job '%s' detected with invalid host requirment '%s'\n",
            J->Name,
            ValLine);
          }

        break;

      case mrrNetwork:

        RQ->Network = MUMAGetIndex(eNetwork,ValLine,mVerify);

        break;

      case mrrOpSys:

        if ((ValLine[0] != '\0') && strcmp(ValLine,"any") && (CIndex == mcmpEQ))
          {
          if ((RQ->Opsys = MUMAGetIndex(eOpsys,ValLine,mAdd)) == FAILURE)
            {
            DBG(1,fSCHED) DPrint("WARNING:  job '%s' does not have valid Opsys value '%s'\n",
              J->Name,
              ValLine);
            }
          }

        break;

      case mrrRMPool:

        RQ->Pool = (char)(strtol(ValLine,NULL,0) + 1);

        break;

      default:

        /* NO-OP */

        DBG(3,fSCHED) DPrint("WARNING:  resource req type %s not supported in %s()\n",
          AName,
          FName);

        break;
      }  /* END switch(MRMRRes[index].RRIndex) */

    ptr = MUStrTok(NULL,"&|",&TokPtr);
    }  /* END while (ptr != NULL) */

  return(SUCCESS);
  }  /* END MReqRResFromString() */
 



int MJobFreeTable()

  {
  mjob_t *J;     /* I */
  mjob_t *JNext; /* I */

  for (J = MJob[0]->Next;J != MJob[0];J = JNext)
    {
    if (J == NULL)
      break;

    JNext = J->Next;

    MJobDestroy(&J);
    }  /* END for (J) */

  return(SUCCESS);
  }  /* END MJobFreeTable() */




int MJobGetRange(

  mjob_t      *J,             /* I:  job description                                */
  mreq_t      *RQ,            /* I:  job req                                        */
  mpar_t      *P,             /* I:  partition index in which resources must reside */
  long         MinStartTime,  /* I:  earliest possible starttime                    */
  mrange_t    *GRange,        /* O:  array of time ranges located                   */
  mnodelist_t  MAvlNodeList,  /* O:  multi-req list of nodes found                  */
  int         *NodeCount,     /* O:  number of available nodes located              */
  char         NodeMap[MAX_MNODE], /* O: (optional)                                 */
  int          RangeType,     /* I:  (bitmap)                                       */
  mrange_t    *TRange)

  {
  int      TaskCount;
  int      ResIndex[MAX_MNODE];
  mrange_t ARange[MAX_MRANGE];
  mrange_t SRange[MAX_MRANGE];
  nodelist_t NodeList;
  mrange_t RRange[2];
  long     FoundStartTime;
  int      RCount;

  int     AvlTaskCount;
  int     AvlNodeCount;

  int     nindex;
  int     nlindex;

  char    Affinity;

  mnode_t *N;

  const char *FName = "MJobGetRange";

  DBG(5,fSCHED) DPrint("%s(%s,RQ,%s,%s,GRange,%s,NodeMap,%d,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (P != NULL) ? P->Name : "NULL",
    MULToTString(MinStartTime - MSched.Time),
    (MAvlNodeList == NULL) ? "NULL" : "MAvlNodeList",
    RangeType,
    (TRange != NULL) ? "TRange" : "NULL");

  MTRAPJOB(J,FName);

  if ((J == NULL) || (RQ == NULL))
    {
    return(FAILURE);
    }

  memset(ARange,0,sizeof(mrange_t) * 4);

  if (MReqGetFNL(
        J,
        RQ,
        P,
        NULL,
        NodeList,
        NULL,
        &TaskCount,
        MAX_MTIME,
        0) == FAILURE)
    {
    DBG(6,fSCHED) DPrint("INFO:     cannot locate configured resources in %s()\n",
      FName);

    return(FAILURE);
    }

  memset(ResIndex,0,sizeof(ResIndex));

  if (NodeMap != NULL)
    memset(NodeMap,nmUnavailable,sizeof(char) * MAX_MNODE);

  GRange[0].EndTime   = 0;

  RRange[0].StartTime = MinStartTime;
  RRange[0].EndTime   = MAX_MTIME;
  RRange[0].TaskCount = MAX(1,RQ->TasksPerNode);

  RRange[1].EndTime   = 0;

  nlindex      = 0;

  AvlTaskCount = 0;
  AvlNodeCount = 0;

  N = NULL;

  for (nindex = 0;NodeList[nindex].N != NULL;nindex++)
    {
    N = NodeList[nindex].N;

    MTRAPNODE(N,FName);

    if (MJobGetSNRange(
          J,
          RQ,
          N,
          RRange,
          MAX_MRANGE,
          &Affinity,
          NULL,
          ARange,
          NULL,
          NULL) == FAILURE)
      {
      DBG(7,fSCHED) DPrint("INFO:     no reservation time found for job %s on node %s at %s\n",
        J->Name,
        N->Name,
        MULToTString(MinStartTime - MSched.Time));

      if (NodeMap != NULL)
        NodeMap[N->Index] = nmUnavailable;

      continue;
      }

    if (NodeMap != NULL)
      NodeMap[N->Index] = Affinity;

    if (MRangeApplyLocalDistributionConstraints(ARange,J,N) == FAILURE)
      {
      /* no valid ranges available */

      continue;
      }

    if (MRLSFromA(J->SpecWCLimit[0],ARange,SRange) == FAILURE)
      {
      /* no valid ranges available */

      continue;
      }

    if ((SRange[0].EndTime != 0) &&
        (SRange[0].StartTime <= RRange[0].StartTime))
      {
      AvlNodeCount++;

      AvlTaskCount += ARange[0].TaskCount;

      if (MAvlNodeList != NULL)
        {
        MAvlNodeList[RQ->Index][nlindex].N  = NodeList[nindex].N;
        MAvlNodeList[RQ->Index][nlindex].TC = ARange[0].TaskCount;

        nlindex++;

        DBG(7,fSCHED) DPrint("INFO:     node %d '%sx%d' added to nodelist\n",
          AvlNodeCount,
          NodeList[nindex].N->Name,
          ARange[0].TaskCount);
        }
      }
    else
      {
      /* check if merging ranges together helps */

      DBG(7,fSCHED) DPrint("INFO:     reservation does not fit into requested time range on node %s at %s (A[0]: %ld:%ld  R: %ld:%ld)\n",
        N->Name,
        MULToTString(MinStartTime - MSched.Time),
        ARange[0].StartTime,
        ARange[0].EndTime,
        RRange[0].StartTime,
        RRange[0].StartTime + J->WCLimit);

      DBG(7,fSCHED) DPrint("INFO:     reservation does not fit into requested time range on node %s at %ld (A[1]: %ld:%ld  A[2]: %ld:%ld)\n",
        N->Name,
        RRange[0].StartTime - MSched.Time,
        ARange[1].StartTime,
        ARange[1].EndTime,
        ARange[2].StartTime,
        ARange[2].EndTime);
      }

    if (MAvlNodeList == NULL)
      {
      MRLMerge(GRange,SRange,J->Request.TC,&FoundStartTime);

      if (Affinity == nmRequired)
        {
        /* constrain feasible time frames */

        MRLAND(GRange,GRange,SRange);
        }

      if (RangeType & (1 << rtcStartEarliest))
        RRange[0].EndTime = FoundStartTime;
      }  /* END else (MAvlNodeList != NULL) */
    }    /* END for (nindex) */

  if (J->GReq != NULL)
    {
    /* get range of global req vs global node */

    if (MJobGetSNRange(
          J,
          J->GReq,
          MSched.GN,
          RRange,
          MAX_MRANGE,
          &Affinity,
          NULL,
          ARange,
          NULL,
          NULL) == FAILURE)
      {
      DBG(7,fSCHED) DPrint("INFO:     no reservation time found for job %s on node %s at %s\n",
        J->Name,
        (N != NULL) ? N->Name : "NULL",
        MULToTString(MinStartTime - MSched.Time));

      if (NodeMap != NULL)
        NodeMap[N->Index] = nmUnavailable;
      }
    else if (MRangeApplyLocalDistributionConstraints(ARange,J,N) == FAILURE)
      {
      /* no valid ranges available */
      }
    else if (MRLSFromA(J->SpecWCLimit[0],ARange,SRange) == FAILURE)
      {
      /* no valid ranges available */
      }
    else if (MRangeGetIntersection(J,GRange,SRange) == FAILURE)
      {
      /* cannot locate intersection of global range w/standard range */
      }
    else
      {
      /* intersection successfully located */
      }
    }    /* END if (J->GReq != NULL) */

  MRangeApplyGlobalDistributionConstraints(GRange,J,&AvlTaskCount);

  if (NodeCount != NULL)
    *NodeCount = AvlNodeCount;

  if (MAvlNodeList != NULL)
    {
    MAvlNodeList[RQ->Index][nlindex].N = NULL;

    MAvlNodeList[RQ->Index + 1][0].N = NULL;

    if (J->Request.TC > AvlTaskCount)
      {
      DBG(6,fSCHED) DPrint("ALERT:    inadequate tasks located for job %s at %s (%d < %d)\n",
        J->Name,
        MULToTString(MinStartTime - MSched.Time),
        AvlTaskCount,
        J->Request.TC);

      return(FAILURE);
      }

    if ((J->Request.NC > AvlNodeCount) || (AvlNodeCount == 0))
      {
      DBG(6,fSCHED) DPrint("ALERT:    inadequate nodes located for job %s at %s (%d < %d)\n",
        J->Name,
        MULToTString(MinStartTime - MSched.Time),
        AvlNodeCount,
        J->Request.NC);

      return(FAILURE);
      }

    return(SUCCESS);
    }  /* END if (MAvlNodeList != NULL) */

  /* get only feasible ranges */

  if (MPar[0].MaxMetaTasks > 0)
    {
    /* impose meta task limit */

    MRLLimitTC(
      GRange,
      MRange,
      NULL,
      MPar[0].MaxMetaTasks);
    }

  if (TRange != NULL)
    memcpy(TRange,GRange,sizeof(mrange_t) * MAX_MRANGE);

  MJobSelectFRL(J,GRange,RangeType,&RCount);

  if (GRange[0].EndTime == 0)
    {
    DBG(6,fSCHED) DPrint("INFO:     no ranges found\n");

    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MJobGetRange() */




int MSubmitTimeComp(

  mjob_t *a,
  mjob_t *b)

  {
  static int tmp;

  /* order low to high */

  tmp = a->SubmitTime - b->SubmitTime;

  return (tmp);
  }  /* END MSubmitTimeComp() */




int MJobSelectFRL(

  mjob_t   *J,         /* I */
  mrange_t *G,         /* I/O */
  int       RangeType, /* I */
  int      *RCount)    /* O */

  {
  int  rindex;
  int  findex;

  int  TC;

  long OldETime;

  const char *FName = "MJobSelectFRL";

  DBG(5,fSCHED) DPrint("%s(%s,G,%d,RCount)\n",
    FName,
    J->Name,
    RangeType);

  findex   = 0;

  TC       = 0;
  OldETime = 0;

  if (!(RangeType & (1 << rtcFeasible)))
    {
    /* remove ranges without adequate tasks available */

    for (rindex = 0;G[rindex].EndTime != 0;rindex++)
      {
      if ((G[rindex].TaskCount >= J->Request.TC) &&
          (G[rindex].NodeCount >= J->Request.NC))
        {
        /* adequate tasks/nodes available */

        if (findex != rindex)
          memcpy(&G[findex],&G[rindex],sizeof(mrange_t));

        findex++;
        }
      else
        {
        DBG(5,fSCHED) DPrint("INFO:     G[%d] has inadequate resources (T: %d < %d) || (N: %d < %d)\n",
          rindex,
          G[rindex].TaskCount,
          J->Request.TC,
          G[rindex].NodeCount,
          J->Request.NC);
        }
      }    /* END for (rindex) */

    G[findex].EndTime = 0;

    *RCount = findex;

    return(SUCCESS);
    }  /* END if (!(RangeType & (1 << rtcFeasible))) */

  /* remove ranges without adequate tasks/nodes and combine remaining ranges */

  for (rindex = 0;G[rindex].EndTime != 0;rindex++)
    {
    if ((TC != 0) &&
       ((G[rindex].TaskCount < J->Request.TC) ||
        (G[rindex].NodeCount < J->Request.NC) ||
        (G[rindex].StartTime > OldETime)))
      {
      /* terminate existing range */

      G[findex].EndTime = OldETime;
      findex++;

      TC = 0;
      }

    if ((G[rindex].TaskCount >= J->Request.TC) &&
        (G[rindex].NodeCount >= J->Request.NC))
      {
      OldETime = G[rindex].EndTime;

      if (TC == 0)
        {
        /* start new range */

        G[findex].StartTime = G[rindex].StartTime;
        G[findex].TaskCount = J->Request.TC;

        if (J->Request.NC > 0)
          G[findex].NodeCount = J->Request.NC;
        else
          G[findex].NodeCount = G[rindex].NodeCount;

        OldETime = G[rindex].EndTime;
        TC       = J->Request.TC;
        }
      }
    }      /* END for (rindex) */

  if (TC > 0)
    {
    G[findex].EndTime = OldETime;
    findex++;
    }

  G[findex].EndTime = 0;

  *RCount = findex;

  DBG(7,fSCHED)
    {
    for (findex = 0;G[findex].EndTime != 0;findex++)
      {
      DPrint("INFO:     G[%02d]  S: %ld  E: %ld  T: %3d  N: %d\n",
        findex,
        G[findex].StartTime,
        G[findex].EndTime,
        G[findex].TaskCount,
        G[findex].NodeCount);
      }
    }

  return(SUCCESS);
  }  /* MJobSelectFRL() */




int MJobToTString(

  mjob_t  *J,       /* I */
  int      Version, /* I */
  char    *Buffer,  /* O */
  int      BufSize) /* I */

  {
  static int  StatCount = 0;

  int    cindex;
  int    index;

  char  *ptr;
  char   Features[MAX_MLINE];
  char   Classes[MAX_MLINE];

  char   QOSString[MAX_MLINE];

  char   ResString[MAX_MLINE];

  char   ReqHList[MAX_MBUFFER];
  char   AllocHosts[MAX_MBUFFER];

  char   tmpLine[MAX_MLINE];

  mreq_t *RQ[MAX_MREQ_PER_JOB];

  long   tmpStartTime;

  char   *BPtr;
  int     BSpace;

  const char *FName = "MJobToTString";

  DBG(4,fSIM) DPrint("%s(%s,%d,Buf,%d)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    Version,
    BufSize);

  if ((J == NULL) || (Buffer == NULL))
    {
    return(FAILURE);
    }

  BPtr   = Buffer;
  BSpace = BufSize;

  BPtr[0] = '\0';

  RQ[0] = J->Req[0];  /* FIXME */

  ptr = MUMAList(eFeature,RQ[0]->ReqFBM,sizeof(RQ[0]->ReqFBM));

  if (ptr[0] != '\0')
    strcpy(Features,ptr);
  else
    sprintf(Features,"[%s]",NONE);

  switch(Version)
    {
    case 230:

      if ((StatCount++ % 100) == 0)
        {
        /*             NAME NRQ TRQ UID GID CPU STA CLAS QUE DIS STA COM ADA ARC OPS MCMP MEM DCMP DSK FEAT SQU TAS TPN QOS FLG ACC CMD  CC  BYP NSU PAR DPROCS DMEM DDISK DSWAP STARTDATE ENDDATE MNODE RM HOSTLINE RESERVATION RES1 RES2 RES3 */

        MUSNPrintF(&BPtr,&BSpace,"%-20s %3s %3s %8s %9s %7s %9s %11s %9s %9s %9s %9s %9s %6s %6s %-2s %4s %-2s %6s %10s %9s %3s %4s %3s %5s %9s %15s %4s %3s %7s %9s %6s %6s %6s %6s %9s %9s %s %s %s %s %s %s %s\n",
          "# JobID",
          "NRQ",
          "TRQ",
          "UserName",
          "GroupName",
          "WCLimit",
          "JobState",
          "Class",
          "QueueTime",
          "DispatchT",
          "StartTime",
          "CompleteT",
          "Network",
          "Arch",
          "Opsys",
          "MC",
          "Mem",
          "DC",
          "Disk",
          "Features",
          "SQTime",
          "Tas",
          "TPNd",
          "QOS",
          "Flags",
          "Account",
          "Command",
          "Comm",
          "Byp",
          "PSUtil",
          "Partition",
          "DProcs",
          "DMem",
          "DDisk",
          "DSwap",
          "StartDate",
          "EndDate",
          "MNode",
          "RMName",
          "HostList",
          "Reservation",
          "AppType",
          "RES1",
          "RES2"
          );
        }  /* END if ((StatCount++ % 20) == 0) */

      break;

    default:

      DBG(4,fSIM) DPrint("ALERT:    cannot create version %d job trace header\n",
        Version);

      break;
    }  /* END switch(Version) */

  sprintf(ResString,"%7.2f",
    J->PSUtilized);

  tmpStartTime = J->StartTime;

  if (J->Ckpt != NULL)
    tmpStartTime -= J->Ckpt->StoredCPDuration;

  switch(Version)
    {
    case 230:

      Classes[0] = '\0';

      for (cindex = 1;cindex < MAX_MCLASS;cindex++)
        {
        if (RQ[0]->DRes.PSlot[cindex].count > 0)
          {
          sprintf(temp_str,"[%s:%d]",
            MAList[eClass][cindex],
            RQ[0]->DRes.PSlot[cindex].count);
          strcat(Classes, temp_str);
          }
        }    /* END for (cindex) */

      if (Classes[0] == '\0')
        strcpy(Classes,NONE);

      if ((J->Flags & (1 << mjfHostList)) &&
          (J->ReqHList != NULL) &&
          (J->ReqHList[0].N != NULL))
        {
        strcpy(ReqHList,J->ReqHList[0].N->Name);

        for (index = 1;J->ReqHList[index].N != NULL;index++)
          {
          if (J->ReqHList[index].TC <= 0)
            break;

          strcat(ReqHList,":");
          strcat(ReqHList,J->ReqHList[index].N->Name);
          }  /* END for (index) */
        }
      else
        {
        strcpy(ReqHList,NONE);
        }

      AllocHosts[0] = '\0';

      for (index = 0;J->NodeList[index].N != NULL;index++)
        {
        if (J->NodeList[index].TC <= 0)
          break;

        if (AllocHosts[0] != '\0')
          strcat(AllocHosts,":");

        strcat(AllocHosts,J->NodeList[index].N->Name);
        }  /* END for (index) */

      if (AllocHosts[0] == '\0')
        strcpy(AllocHosts,NONE);

      sprintf(QOSString,"%s:%s",
        (J->QReq != NULL) ? J->QReq->Name : NONE,
        (J->Cred.Q != NULL) ? J->Cred.Q->Name : NONE);

      /*            NAME NRQ TRQ UID GID CPU STA CLAS QUET DIST STAT COMT Net ARC OPS MCMP MEM DCMP DSK FEAT SQUT TAS TPN QREQ FLG ACC CMD  CC  BYP NSUTL PAR DPROCS DMEM DDISK DSWAP STARTDATE ENDDATE MNODE RMINDEX HOSTLIST RESID ASNAME RES1 RES2 */

      MUSNPrintF(&BPtr,&BSpace,"%-20s %3d %3d %8s %9s %7ld %9s %11s %9lu %9lu %9lu %9lu %9s %6s %6s %-2s %4dM %-2s %6dM %10s %9lu %3d %4d %s %5s %9s %15s %4s %3d %s %9s %6d %6dM %6dM %6dM %9lu %9lu %s %s %s %s %s %s %s\n",
        J->Name,
        J->Request.NC,
        J->Request.TC,
        (J->Cred.U != NULL) ? J->Cred.U->Name : NONE,
        (J->Cred.G != NULL) ? J->Cred.G->Name : NONE,
        J->SpecWCLimit[0],
        MJobState[J->State],
        Classes,
        (unsigned long)J->SubmitTime,
        (unsigned long)J->DispatchTime,
        (unsigned long)tmpStartTime,
        (unsigned long)J->CompletionTime,
        MAList[eNetwork][RQ[0]->Network],
        MAList[eArch][RQ[0]->Arch],
        MAList[eOpsys][RQ[0]->Opsys],
        MComp[RQ[0]->MemCmp],
        RQ[0]->RequiredMemory,
        MComp[RQ[0]->DiskCmp],
        RQ[0]->RequiredDisk,
        Features,
        (unsigned long)J->SystemQueueTime,
        J->TaskCount,
        RQ[0]->TasksPerNode,
        QOSString,
        MUBMToString(J->SpecFlags,MJobFlags,'[',tmpLine,NONE),
        (J->Cred.A != NULL) ? J->Cred.A->Name : NONE,
        (J->E.Cmd != NULL) ? J->E.Cmd : NONE,
        (J->RMXString != NULL) ? J->RMXString : NONE,
        J->Bypass,
        ResString,
        MAList[ePartition][RQ[0]->PtIndex],
        RQ[0]->DRes.Procs,
        RQ[0]->DRes.Mem,
        RQ[0]->DRes.Disk,
        RQ[0]->DRes.Swap,
        (unsigned long)J->SpecSMinTime,
        (unsigned long)J->CMaxTime,
        AllocHosts,
        (J->RM != NULL) ? J->RM->Name : MRM[0].Name,
        ReqHList,
        (J->ResName[0] != '\0') ? J->ResName : NONE,
        MASGetName(J),
        NONE,
        NONE);

      break;

    default:

      DBG(4,fSIM) DPrint("ALERT:    cannot create version %d job trace record\n",
        Version);

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(Version) */

  return(SUCCESS);
  }  /* END MJobToTString() */




int MJobTestName(

  char *String)  /* I */

  {
  int   rmtindex;

  char *ptr;
  char *TokPtr;

  char  SJobName[MAX_MNAME];
  char  LJobName[MAX_MNAME];

  mrm_t tmpR;

  if ((String == NULL) || ((ptr = MUStrTok(String,":",&TokPtr)) == NULL))
    {
    exit(1);
    }

  /* FORMAT:  <RM>:<JOBID> */

  rmtindex = MUGetIndex(ptr,MRMType,FALSE,0);

  if ((ptr = MUStrTok(NULL,":",&TokPtr)) == NULL)
    {
    exit(1);
    }

  memset(&tmpR,0,sizeof(tmpR));

  tmpR.Type = rmtindex;

  /* make short name */

  MJobGetName(NULL,ptr,&tmpR,SJobName,sizeof(SJobName),mjnShortName);

  /* make long name */

  MJobGetName(NULL,SJobName,&tmpR,LJobName,sizeof(LJobName),mjnRMName);

  fprintf(stderr,"RM: '%s'  Orig: '%s'  S: '%s'  L: '%s'  QMHost: '%s'  DD: '%s'\n",
    MRMType[rmtindex],
    ptr,
    SJobName,
    LJobName,
    tmpR.DefaultQMHost,
    MSched.DefaultDomain);

  if (strcmp(ptr,LJobName))
    {
    /* job names do not match */

    exit(1);
    }
  
  exit(0);

  /*NOTREACHED*/

  return(SUCCESS);
  }  /* END MJobTestName() */




int MJobTestDist()

  {
  mjob_t tmpJ;

  macl_t    tmpACL[MAX_MACL];
  macl_t    tmpCL[MAX_MACL];

  mreq_t    tmpRQ;

  mnalloc_t tmpNodeList[MAX_MNODE + 1];

  int       index;
  char      tmpLine[MAX_MLINE];

  mrm_t     tmpRM;

  MJobMkTemp(&tmpJ,&tmpRQ,tmpACL,tmpCL,NULL,tmpNodeList);

  /* configure RM */

  memset(&tmpRM,0,sizeof(tmpRM));

  strcpy(tmpRM.Name,"PBS");

  tmpRM.Type = mrmtPBS;

  tmpRM.Index = 0;

  /* configure job */

  strcpy(tmpJ.Name,"test");

  tmpJ.DistPolicy = mtdDefault;

  tmpJ.MReq = NULL;

  tmpJ.Geometry = NULL;

  tmpRQ.RMIndex = tmpRM.Index;

  tmpRQ.NodeCount = 16;

  tmpRQ.TaskCount = 16;

  tmpRQ.BlockingFactor = 0;

  /* configure nodes/nodelist */

  for (index = 0;index < 16;index++)
    {
    sprintf(tmpLine,"node%02d",
      index);

    MNodeAdd(tmpLine,&MNode[index]);

    tmpNodeList[index].TC = 2;
    tmpNodeList[index].N  = MNode[index];
    }  /* END for (index) */

  tmpNodeList[index].N = NULL;
  tmpNodeList[index].TC = 0;

  MJobDistributeTasks(&tmpJ,&tmpRM,tmpNodeList,tmpJ.TaskMap);

  if (tmpNodeList[0].TC == 0)
    exit(1);
 
  exit(0);

  /*NOTREACHED*/
 
  return(SUCCESS);
  }  /* END MJobTestDist() */




int MJobCheckClassJLimits(

  mjob_t   *J,         /* I */
  mclass_t *C,         /* I */
  int       AdjustJob, /* I (boolean) */
  char     *Buffer,    /* O (optional) */
  int       BufSize)   /* I (optional) */

  {
  long tmpWCLimit;
  int  tmpNode;
  int  tmpProc;

  mjob_t *JL;

  mclass_t *DefC;

  const char *FName = "MJobCheckClassJLimits";

  DBG(7,fSCHED) DPrint("%s(%s,C,%d,Buffer,BufSize)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    AdjustJob);

  if (Buffer != NULL)
    Buffer[0] = '\0';

  if ((J == NULL) || (C == NULL))
    {
    return(SUCCESS);
    }

  /* if AdjustJob == TRUE, fix job violations */
  /* if AdjustJob == FALSE, return failure on violation */

  /* NOTE:  temporarily map 'proc' to 'task' for class limits */

  DefC = MSched.DefaultC;

  tmpWCLimit = 0;
  tmpNode    = 0;
  tmpProc    = 0;

  if (C->L.JMax != NULL)
    {
    /* check max limits */

    JL = (mjob_t *)C->L.JMax;

    tmpWCLimit = JL->SpecWCLimit[0];
    tmpNode    = JL->Request.NC;
    tmpProc    = JL->Request.TC;
    }

  if ((DefC != NULL) && (DefC->L.JMax != NULL))
    {
    JL = (mjob_t *)DefC->L.JMax;
  
    if (tmpWCLimit == 0)
      tmpWCLimit = JL->SpecWCLimit[0];

    if (tmpNode == 0)
      tmpNode = JL->Request.NC;

    if (tmpProc == 0)
      tmpProc = JL->Request.TC;
    }

  /* NOTE:  need complete method of obtaining 'most constrainted' cred limit (NYI) */

  /* check user wc limits */

  if ((J->Cred.U != NULL) && 
      (J->Cred.U->L.JMax != NULL))
    {
    JL = (mjob_t *)J->Cred.U->L.JMax;

    if (JL->SpecWCLimit[0] > 0)
      {
      tmpWCLimit = (tmpWCLimit > 0) ?
        MIN(tmpWCLimit,JL->SpecWCLimit[0]) : JL->SpecWCLimit[0];
      }
    }  /* END if ((J->Cred.U != NULL) && ...) */

  /* check group wc limits */

  if ((J->Cred.G != NULL) &&
      (J->Cred.G->L.JMax != NULL))
    {
    JL = (mjob_t *)J->Cred.G->L.JMax;

    if (JL->SpecWCLimit[0] > 0)
      {
      tmpWCLimit = (tmpWCLimit > 0) ?
        MIN(tmpWCLimit,JL->SpecWCLimit[0]) : JL->SpecWCLimit[0];
      } 
    }  /* END if ((J->Cred.G != NULL) && ...) */

  if (tmpWCLimit > 0)
    {
    /* check max WC limit */

    if ((J->SpecWCLimit[1] > tmpWCLimit) || (J->SpecWCLimit[1] == -1))
      {
      if (AdjustJob == TRUE)
        {
        MJobSetAttr(
          J,
          mjaReqAWDuration,
          (void **)&tmpWCLimit,
          mdfLong,
          mSet);
        }
      else
        {
        if (Buffer != NULL)
          {
          sprintf(Buffer,"wclimit too high (%ld > %ld)",
            J->SpecWCLimit[1],
            tmpWCLimit);
          }

        return(FAILURE);
        }
      }
    }    /* END if (tmpWCLimit > 0) */

  if (tmpNode > 0)
    {
    /* check max node */

    if (J->Request.NC > tmpNode)
      { 
      if (AdjustJob == TRUE)
        {
        MJobSetAttr(
          J,
          mjaReqNodes,
          (void **)&tmpNode,
          mdfInt,
          mSet);
        }
      else
        {
        if (Buffer != NULL)
          {
          sprintf(Buffer,"nodes too high (%d > %d)",
            J->Request.NC,
            tmpNode);
          }

        return(FAILURE);
        }
      } 
    }    /* END if (tmpNode > 0) */

  if (tmpProc > 0)
    {
    /* check max proc */

    if (J->Request.TC > tmpProc)
      {
      if (AdjustJob == TRUE)
        {
        MJobSetAttr(
          J,
          mjaReqProcs,
          (void **)&tmpProc,
          mdfInt,
          mSet);
        }
      else
        {
        if (Buffer != NULL)
          {
          sprintf(Buffer,"procs too high (%d > %d)",
            J->Request.TC,
            tmpProc);
          }

        return(FAILURE);
        }
      }
    }    /* END if (tmpProc > 0) */

  /* support default class min limits */

  tmpWCLimit = 0;
  tmpNode    = 0;
  tmpProc    = 0;

  if (C->L.JMin != NULL)
    {
    /* check min limits */

    JL = (mjob_t *)C->L.JMin;

    tmpWCLimit = JL->SpecWCLimit[0];
    tmpNode    = JL->Request.NC;
    }

  if ((DefC != NULL) && (DefC->L.JMin != NULL))
    {
    JL = (mjob_t *)DefC->L.JMin;

    if (tmpWCLimit == 0)
      tmpWCLimit = JL->SpecWCLimit[0];

    if (tmpNode == 0)
      tmpNode = JL->Request.NC;
    }

  if (tmpWCLimit > 0)
    {
    /* check min WC limit */

    if ((J->SpecWCLimit[1] < tmpWCLimit) && (J->SpecWCLimit[1] != -1))
      {
      if (AdjustJob == TRUE)
        {
        J->SpecWCLimit[1] = tmpWCLimit;
        }
      else
        {
        if (Buffer != NULL)
          {
          sprintf(Buffer,"wclimit too low (%ld < %ld)",
            J->SpecWCLimit[1],
            tmpWCLimit);
          }

        return(FAILURE);
        }
      }
    }    /* END if (tmpWCLimit > 0) */

  return(SUCCESS);
  }  /* END MJobCheckClassJLimits() */




int MJobValidate(

  mjob_t *J,       /* I */
  char   *Buf,     /* O */
  int     BufSize) /* I */

  {
  char *BPtr;
  int   BSpace;

  char  tmpLine[MAX_MLINE];

  const char *FName = "MJobValidate";

  DBG(6,fSIM) DPrint("%s(%s,Buf,%d)\n",
    FName, 
    (J != NULL) ? J->Name : "NULL",
    BufSize);

  if (J == NULL)
    {
    return(FAILURE);
    }

  if (Buf != NULL)
    {
    BPtr = Buf;

    BPtr[0] = '\0';
    
    BSpace = BufSize;
    }

  if ((J->Cred.C != NULL) && 
      (MJobCheckClassJLimits(J,J->Cred.C,FALSE,tmpLine,sizeof(tmpLine)) == FAILURE))
    {
    if (Buf != NULL)
      {
      MUSNPrintF(&BPtr,&BSpace,"job does not meet constraints of class %s '%s'",
        J->Cred.C->Name,
        tmpLine);
      }

    return(FAILURE);
    }
  
  return(SUCCESS);
  }  /* END MJobValidate() */




int MJobDetermineCreds(

  mjob_t *J)  /* I (modified) */

  {
  const char *FName = "MJobDetermineCreds";

  DBG(7,fSIM) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if (J == NULL)
    {
    return(FAILURE);
    }

  /* get default group */

  if (J->Cred.G == NULL)
    {
    char tmpLine[MAX_MLINE];

    if ((J->Cred.U != NULL) &&
        (MUGNameFromUName(J->Cred.U->Name,tmpLine) == SUCCESS))
      {
      MGroupAdd(tmpLine,&J->Cred.G);
      }
    }

  /* get default account */

  if (J->Cred.A == NULL)
    {
    MJobGetAccount(J,&J->Cred.A);
    }

  /* get default class */

  if (J->Cred.C == NULL)
    {
    if ((MSched.DefaultC != NULL) && 
        (MSched.DefaultC->Name[0] != '\0'))
      {
      MClassFind(MSched.DefaultC->Name,&J->Cred.C);
      }
    }    /* END if (J->Cred.C == NULL) */

  /* get default QOS */

  /* NYI */

  return(SUCCESS);
  }  /* END MJobDetermineCreds() */




int MJobGetLocalTL(

  mjob_t *J,       /* I */
  short  *TL,      /* O */
  int     TLSize)  /* I */

  {
  int TIndex;

  int tindex;
  int rqindex;
  int nindex;

  mreq_t *RQ;

  const char *FName = "MJobGetLocalTL";

  DBG(7,fSIM) DPrint("%s(%s,TL,%d)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    TLSize);

  if ((J == NULL) || (TL == NULL))
    {
    return(FAILURE);
    }

  TIndex = 0;

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];

    if (RQ->DRes.Procs != 0)
      continue;

    for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
      {
      for (tindex = 0;tindex < RQ->NodeList[nindex].TC;tindex++)
        { 
        TL[TIndex++] = RQ->NodeList[nindex].N->Index;

        if (TIndex >= TLSize - 1)
          break;
        }  /* END for (tindex) */
      }    /* END for (nindex) */
    }      /* END for (rqindex) */

  TL[TIndex] = -1;

  return(SUCCESS);
  }  /* END MJobGetLocalTL() */




int MReqAllocateLocalRes(

  mjob_t *J,  /* I */
  mreq_t *RQ) /* I */

  {
  nodelist_t tmpNL;

  const char *FName = "MReqAllocateLocalRes";

  DBG(7,fSIM) DPrint("%s(%s,%d)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (RQ != NULL) ? RQ->Index : -1);

  if ((J == NULL) || (RQ == NULL))
    {
    return(FAILURE);
    }

  if (RQ->DRes.Procs != 0)
    { 
    /* no local resources to allocated */

    return(SUCCESS);
    }

  if (RQ->NodeList[0].N != NULL)
    {
    /* local resources previously allocated */

    return(SUCCESS);
    }

  /* must allocate local resources */

  /* NOTE:  currently allocate first feasible pseudo resource */

  if (MReqGetFNL(
       J,
       RQ,
       &MPar[0],
       NULL, /* I */
       tmpNL,
       NULL,
       NULL,
       MAX_MTIME,
       0) == FAILURE)
    {
    DBG(2,fRM) DPrint("ALERT:    cannot locate feasible pseudo resources for %s:%d\n",
      J->Name,
      RQ->Index);

    return(FAILURE);
    }

  /* NOTE:  only allocate single node per req */

  tmpNL[0].TC = MIN(tmpNL[0].TC,RQ->TaskCount);
  tmpNL[1].N  = NULL;

  MReqSetAttr(
    J,
    RQ,
    mrqaAllocNodeList,
    (void **)tmpNL,
    mdfOther,
    mSet);

  J->Request.NC += 1;
  J->Request.TC += tmpNL[0].TC;

  DBG(2,fRM) DPrint("INFO:     %d of %d tasks allocated for %s:%d from node %s\n",
    RQ->NodeList[0].TC,
    RQ->TaskCount,
    J->Name,
    RQ->Index,
    tmpNL[0].N->Name);

  return(SUCCESS);
  }  /* END MReqAllocateLocalRes() */


/* END MJob.c */


