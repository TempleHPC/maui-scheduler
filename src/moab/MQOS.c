/* HEADER */
 
/* Contains:                                         *
 *  int MQOSShow(Buffer,BufSize,Flags)               *
 *                                                   */

#include "moab.h"
#include "msched-proto.h"  

extern mlog_t      mlog;
 
extern msched_t    MSched;
extern mpar_t      MPar[];
extern mqos_t      MQOS[];
extern mattrlist_t MAList;
extern mgcred_t   *MUser[];
extern mgcred_t    MGroup[];
extern mgcred_t    MAcct[];
extern mclass_t    MClass[];
extern m64_t       M64;

extern const char *MQOSFlags[];
extern const char *MQALType[];
extern const char *MJobFlags[];

extern const char *MQOSAttr[];

#define MPARM_QOSCFG "QOSCFG"

/* internal prototypes */

int __MQOSSetOverrideLimits(mqos_t *);    






int MQOSFind(

  char    *QName, /* I */
  mqos_t **Q)     /* O (optional) */

  {
  /* If found, return success with Q pointing to QOS.     */
  /* If not found, return failure with Q pointing to      */
  /* first free QOS if available, Q set to NULL otherwise */

  /* NOTE:  QOS's only added, never removed */

  int qindex;

  if (Q != NULL)
    *Q = NULL;

  if ((QName == NULL) || 
      (QName[0] == '\0'))
    {
    return(FAILURE);
    }

  for (qindex = 0;qindex < MAX_MQOS;qindex++)
    {
    if (MQOS[qindex].Name[0] == '\0')
      {
      /* 'free' QOS slot found */

      if (Q != NULL)
        *Q = &MQOS[qindex];

      break;
      }

    if (strcmp(MQOS[qindex].Name,QName) != 0)
      continue;

    /* QOS found */

    if (Q != NULL)
      *Q = &MQOS[qindex];

    return(SUCCESS);
    }  /* END for (qindex) */

  return(FAILURE);
  }  /* END MQOSFind() */




int MQOSLoadConfig(

  char *QName)  /* I */

  {
  mqos_t *Q;

  char   IndexName[MAX_MNAME];
  char   Value[MAX_MNAME];

  char  *ptr;
 
  /* FORMAT:  <KEY>=<VAL>[<WS><KEY>=<VAL>]...         */
  /*          <VAL> -> <ATTR>=<VAL>[:<ATTR>=<VAL>]... */

  const char *FName = "MQOSLoadConfig";

  DBG(4,fCONFIG) DPrint("%s(%s)\n",
    FName,
    (QName != NULL) ? QName : "NULL");

  /* load all/specified QOS config info */

  if (MSched.ConfigBuffer == NULL)
    {
    return(FAILURE);
    }

  if ((QName == NULL) || (QName[0] == '\0'))
    {
    /* load ALL QOS config info */

    ptr = MSched.ConfigBuffer;
 
    IndexName[0] = '\0';
 
    while (MCfgGetSVal(
             MSched.ConfigBuffer,
             &ptr,
             MPARM_QOSCFG,
             IndexName,
             NULL,
             Value,
             sizeof(Value),
             0,
             NULL) != FAILURE)
      {
      if (MQOSFind(IndexName,&Q) == FAILURE)
        {
        if (Q == NULL)
          {
          /* unable to add QOS */

          IndexName[0] = '\0';     

          continue; 
          }
        else
          {
          MQOSAdd(IndexName,&Q);
          }
        }
 
      MQOSProcessConfig(Q,Value);
 
      IndexName[0] = '\0';
      }  /* END while (MCfgGetSVal() != FAILURE) */ 
    }    /* END if ((QName == NULL) || (QName[0] == '\0')) */
  else
    {
    /* load specified QOS config info */

    if (MCfgGetSVal(
          MSched.ConfigBuffer,
          NULL,
          MPARM_QOSCFG,
          QName,
          NULL,
          Value,
          sizeof(Value),
          0,
          NULL) == FAILURE)
      {
      /* cannot locate config info for specified QName */ 

      return(FAILURE);
      }
    
    if (MQOSFind(QName,&Q) == FAILURE)
      {
      if (Q == NULL)
        {
        /* unable to add QOS */

        return(FAILURE);
        }
      else
        {
        MQOSAdd(QName,&Q);
        }
      }

    MQOSProcessConfig(Q,Value);
    }  /* END else ((QName == NULL) || (QName[0] == '\0')) */
   
  return(SUCCESS); 
  }  /* END MQOSLoadConfig() */




int MQOSAdd(

  char    *QName,
  mqos_t **QP)

  {
  int qindex;

  mqos_t *Q;

  const char *FName = "MQOSAdd";

  DBG(3,fCONFIG) DPrint("%s(%s,Q)\n",
    FName,
    (QName != NULL) ? QName : "NULL");

  if ((QName == NULL) ||
      (QName[0] == '\0'))
    {
    return(FAILURE);
    }

  for (qindex = 0;qindex < MAX_MQOS;qindex++)
    {
    Q = &MQOS[qindex];

    if (Q->Name[0] == '\0')
      {
      /* emply QOS slot located */

      MQOSInitialize(Q,QName);

      Q->Index = qindex;

      if (QP != NULL)
        *QP = Q;

      return(SUCCESS);
      }

    if (!strcmp(Q->Name,QName))
      {
      if (QP != NULL)
        *QP = Q;
  
      return(SUCCESS);
      }
    }    /* END for (qindex) */

  if (QP != NULL)
    *QP = NULL;  
 
  return(FAILURE);
  }  /* END MQOSAdd() */





int MQOSInitialize(

  mqos_t *Q,
  char   *QName)

  {
  int index;

  const char *FName = "MQOSInitialize";

  DBG(4,fCONFIG) DPrint("%s(Q,%s)\n",
    FName,
    (QName != NULL) ? QName : "NULL");

  if ((Q == NULL) || 
      (QName == NULL) || 
      (QName[0] == '\0'))
    {
    return(FAILURE);
    }

  memset(Q,0,sizeof(mqos_t));

  MUStrCpy(Q->Name,QName,sizeof(Q->Name));

  Q->QTTarget   = DEFAULT_TARGETQT;
  Q->XFTarget   = DEFAULT_TARGETXF;
 
  Q->Flags      = DEFAULT_MQOSFLAGS;
 
  Q->QTWeight   = DEFAULT_QOSQTWEIGHT;
  Q->XFWeight   = DEFAULT_QOSXFWEIGHT;
 
  Q->F.Priority = DEFAULT_MQOSPRIORITY;

  if (Q->L.OAP == NULL)
    Q->L.OAP = (mpu_t *)calloc(1,sizeof(mpu_t));

  if (Q->L.OIP == NULL)
    Q->L.OIP = (mpu_t *)calloc(1,sizeof(mpu_t));

  if (Q->L.OJP == NULL)
    Q->L.OJP = (mpu_t *)calloc(1,sizeof(mpu_t));

  for (index = 0;index < MAX_MQOS;index++)
    {
    if (&MQOS[index] == Q)
      {
      Q->Index = index;
 
      break;
      }
    }  /* END for (index) */

  return(SUCCESS);
  }  /* END MQOSInitialize() */




int MQOSProcessConfig(

  mqos_t *Q,     /* I */
  char   *Value) /* I */

  {
  char *ptr;
  char *TokPtr;

  int   aindex;

  char  ValLine[MAX_MNAME];

  const char *FName = "MQOSProcessConfig";

  DBG(5,fCONFIG) DPrint("%s(%s,%s)\n",
    FName,
    (Q != NULL) ? Q->Name : "NULL",
    (Value != NULL) ? Value : "NULL");

  if ((Q == NULL) ||
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

    if (MUGetPair(
          ptr,
          (const char **)MQOSAttr,
          &aindex,
	  NULL,
          TRUE,
          NULL,
          ValLine,
          MAX_MNAME) == FAILURE)
      {
      /* cannot parse value pair */

      ptr = MUStrTok(NULL," \t\n",&TokPtr);

      continue;
      }
   
    switch(aindex)
      {
      case mqaQTWeight:

        Q->QTWeight = strtol(ValLine,NULL,0);  

        break;

      case mqaQTTarget:

        Q->QTTarget = strtol(ValLine,NULL,0);           

        break;

      case mqaXFWeight:

        Q->XFWeight = strtol(ValLine,NULL,0);       
 
        break;

      case mqaXFTarget:

        Q->XFTarget = strtod(ValLine,NULL);       

        break;

      case mqaFSTarget:

        Q->F.FSTarget = strtod(ValLine,NULL);
 
        break;

      case mqaFlags:

        /* FORMAT:  FLAGS=A:B:C:D */

        MQOSFlagsFromString(Q,ValLine);

        break;

      default:

        /* not handled */

        return(FAILURE);

        /*NOTREACHED*/

        break;
      }  /* END switch(AIndex) */

    ptr = MUStrTok(NULL," \t\n",&TokPtr);       
    }  /* END while (ptr != NULL) */

  return(SUCCESS);
  }  /* END MQOSProcessConfig() */




int MQOSGetAccess(
 
  mjob_t  *J,     /* I */
  mqos_t  *QReq,  /* I: set to NULL if no request made */
  int     *QAL,   /* O: bitmap of QOS access           */
  mqos_t **QDef)  /* O: default QOS (optional)         */
 
  {
  int  tmpQAL[(MAX_MQOS >> 5) + 1];
  int  ZQAL[(MAX_MQOS >> 5) + 1]; 
 
  int  AndMask;

  int  qindex;
  int  cindex;

  mclass_t *C;

  const char *FName = "MQOSGetAccess";
 
  DBG(7,fSTRUCT) DPrint("%s(%s,%s,QAL,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (QReq != NULL) ? QReq->Name : "NULL",
    (QDef != NULL) ? "QDef" : "NULL");
 
  if ((J == NULL) || (J->Cred.U == NULL) || (J->Cred.G == NULL))
    {
    return(FAILURE);
    }

  memset(ZQAL,0,sizeof(ZQAL));

  if (J->Cred.C != NULL)
    {
    C = J->Cred.C;
    }
  else
    {
    C = NULL;

    for (cindex = 1;cindex < MAX_MCLASS;cindex++)
      {
      if (J->Req[0]->DRes.PSlot[cindex].count > 0)
        {
        MClassFind(MAList[eClass][cindex],&C);

        break;
        }
      }    /* END for (cindex) */
    }
 
  AndMask = FALSE;
 
  /* determine QAL */

  MUBMCopy(tmpQAL,MPar[0].F.QAL,MAX_MQOS); 
 
  if (MPar[0].F.QALType == qalAND)
    {
    AndMask = TRUE;
    }
  else
    {
    /* obtain 'or' list */
 
    if ((J->Cred.U->F.QALType == qalOR) &&
        (memcmp(J->Cred.U->F.QAL,ZQAL,sizeof(ZQAL))))
      {
      MUBMOR(tmpQAL,J->Cred.U->F.QAL,MAX_MQOS); 
      }
    else if ((MSched.DefaultU != NULL) &&
             (MSched.DefaultU->F.QALType == qalOR))
      {
      MUBMOR(tmpQAL,MSched.DefaultU->F.QAL,MAX_MQOS);
      }

    if ((J->Cred.G->F.QALType == qalOR) &&
        (memcmp(J->Cred.G->F.QAL,ZQAL,sizeof(ZQAL))))
      {
      MUBMOR(tmpQAL,J->Cred.G->F.QAL,MAX_MQOS);
      }
    else if ((MSched.DefaultG != NULL) &&
             (MSched.DefaultG->F.QALType == qalOR))
      {
      MUBMOR(tmpQAL,MSched.DefaultG->F.QAL,MAX_MQOS);
      }

    if (J->Cred.A != NULL)
      {
      if ((J->Cred.A->F.QALType == qalOR) &&
          (memcmp(J->Cred.A->F.QAL,ZQAL,sizeof(ZQAL))))
        {
        MUBMOR(tmpQAL,J->Cred.A->F.QAL,MAX_MQOS);
        }
      else if ((MSched.DefaultA != NULL) &&
               (MSched.DefaultA->F.QALType == qalOR))
        {
        MUBMOR(tmpQAL,MSched.DefaultA->F.QAL,MAX_MQOS);
        }
      }

    if (C != NULL)
      {
      if ((C->F.QALType == qalOR) &&
          (memcmp(C->F.QAL,ZQAL,sizeof(ZQAL))))
        {
        MUBMOR(tmpQAL,C->F.QAL,MAX_MQOS);
        }
      else if ((MSched.DefaultC != NULL) &&
               (MSched.DefaultC->F.QALType == qalOR))
        {
        MUBMOR(tmpQAL,MSched.DefaultC->F.QAL,MAX_MQOS);
        }
      }
    }  /* END else (Policy[0].F.QALType == qalAND) */
 
  /* obtain 'exclusive' list */ 

  if ((C != NULL) && (C->F.QALType == qalAND))
    {
    if (AndMask != TRUE)
      {
      MUBMCopy(tmpQAL,C->F.QAL,MAX_MQOS);
 
      AndMask = TRUE;
      }
    else
      {
      MUBMAND(tmpQAL,C->F.QAL,MAX_MQOS);
      }
    }
 
  if ((J->Cred.A != NULL) && (J->Cred.A->F.QALType == qalAND))
    {
    if (AndMask != TRUE)
      {
      MUBMCopy(tmpQAL,J->Cred.A->F.QAL,MAX_MQOS);    
 
      AndMask = TRUE;
      }
    else
      {
      MUBMAND(tmpQAL,J->Cred.A->F.QAL,MAX_MQOS);     
      }
    }
 
  if (J->Cred.G->F.QALType == qalAND)
    {
    if (AndMask != TRUE)
      {
      MUBMCopy(tmpQAL,J->Cred.G->F.QAL,MAX_MQOS);    
 
      AndMask = TRUE;
      }
    else
      {
      MUBMAND(tmpQAL,J->Cred.G->F.QAL,MAX_MQOS);      
      }
    }
 
  if (J->Cred.U->F.QALType == qalAND)
    {
    if (AndMask != TRUE)
      {
      MUBMCopy(tmpQAL,J->Cred.U->F.QAL,MAX_MQOS);    
 
      AndMask = TRUE;
      }
    else
      {
      MUBMAND(tmpQAL,J->Cred.U->F.QAL,MAX_MQOS);     
      }
    } 
 
  if (J->Cred.U->F.QALType == qalONLY)
    {
    MUBMCopy(tmpQAL,J->Cred.U->F.QAL,MAX_MQOS);    
    }
  else if (J->Cred.G->F.QALType == qalONLY)
    {
    MUBMCopy(tmpQAL,J->Cred.G->F.QAL,MAX_MQOS);       
    }
  else if ((J->Cred.A != NULL) && (J->Cred.A->F.QALType == qalONLY))
    {
    MUBMCopy(tmpQAL,J->Cred.A->F.QAL,MAX_MQOS);       
    }
  else if ((C != NULL) && (C->F.QALType == qalONLY))
    {
    MUBMCopy(tmpQAL,C->F.QAL,MAX_MQOS);
    }
 
  if (QAL != NULL)
    MUBMCopy(QAL,tmpQAL,MAX_MQOS);       
 
  /* determine allowed QOS default (precedence: U,G,A,C,S,0) */
 
  if (QDef != NULL)
    {
    if ((J->Cred.U->F.QDef != NULL) && 
        (J->Cred.U->F.QDef != &MQOS[0]) &&
         MUBMCheck(((mqos_t *)J->Cred.U->F.QDef)->Index,tmpQAL))
      {
      *QDef = (mqos_t *)J->Cred.U->F.QDef;
      }
    else if ((MSched.DefaultU != NULL) &&
             (MSched.DefaultU->F.QDef != NULL) &&
             (MSched.DefaultU->F.QDef != &MQOS[0]) &&
              MUBMCheck(((mqos_t *)MSched.DefaultU->F.QDef)->Index,tmpQAL))
      {
      *QDef = (mqos_t *)MSched.DefaultU->F.QDef;
      }
    else if ((J->Cred.G->F.QDef != NULL) &&
             (J->Cred.G->F.QDef != &MQOS[0]) &&     
              MUBMCheck(((mqos_t *)J->Cred.G->F.QDef)->Index,tmpQAL))
      {
      *QDef = (mqos_t *)J->Cred.G->F.QDef;
      }
    else if ((MSched.DefaultG != NULL) &&
             (MSched.DefaultG->F.QDef != NULL) &&
             (MSched.DefaultG->F.QDef != &MQOS[0]) &&
              MUBMCheck(((mqos_t *)MSched.DefaultG->F.QDef)->Index,tmpQAL))
      {
      *QDef = (mqos_t *)MSched.DefaultG->F.QDef;
      }
    else if ((J->Cred.A != NULL) &&
             (J->Cred.A->F.QDef != NULL) &&
             (J->Cred.A->F.QDef != &MQOS[0]) &&       
              MUBMCheck(((mqos_t *)J->Cred.A->F.QDef)->Index,tmpQAL))
      {
      *QDef = (mqos_t *)J->Cred.A->F.QDef;
      }
    else if ((MSched.DefaultA != NULL) &&
             (MSched.DefaultA->F.QDef != NULL) &&
             (MSched.DefaultA->F.QDef != &MQOS[0]) &&
              MUBMCheck(((mqos_t *)MSched.DefaultA->F.QDef)->Index,tmpQAL))
      {
      *QDef = (mqos_t *)MSched.DefaultA->F.QDef;
      }
    else if ((C != NULL) &&
             (C->F.QDef != NULL) &&
             (C->F.QDef != &MQOS[0]) &&
              MUBMCheck(((mqos_t *)C->F.QDef)->Index,tmpQAL))
      {
      *QDef = (mqos_t *)C->F.QDef;
      }
    else if ((MSched.DefaultC != NULL) &&
             (MSched.DefaultC->F.QDef != NULL) &&
             (MSched.DefaultC->F.QDef != &MQOS[0]) &&
              MUBMCheck(((mqos_t *)MSched.DefaultC->F.QDef)->Index,tmpQAL))
      {
      *QDef = (mqos_t *)MSched.DefaultC->F.QDef;
      }
    else if ((MPar[0].F.QDef != NULL) &&
             (MPar[0].F.QDef != &MQOS[0]))
      {
      *QDef = (mqos_t *)MPar[0].F.QDef;
      }
    else 
      {
      *QDef = &MQOS[MDEF_SYSQDEF];
      }
 
    /* verify access to default QOS */
 
    if (!MUBMCheck((*QDef)->Index,tmpQAL))
      {
      *QDef = &MQOS[0];
 
      /* locate first legal QOS */
 
      for (qindex = 0;qindex < MAX_MQOS;qindex++)
        {
        if (MUBMCheck(qindex,tmpQAL))
          {
          *QDef = &MQOS[qindex];
 
          break;
          }
        }    /* END for (qindex) */
      }      /* END if (!(tmpQAL & (1 << QDef->Index))) */

    {
    mqos_t *AQD = NULL;
    mqos_t *CQD = NULL;
 
    if ((J->Cred.A != NULL) && (J->Cred.A->F.QDef != NULL))
      AQD = (mqos_t *)J->Cred.A->F.QDef;
 
    if ((C != NULL) && (C->F.QDef != NULL))
      CQD = (mqos_t *)C->F.QDef;
 
    DBG(3,fSTRUCT) DPrint("INFO:     default QOS for job %s set to %s(%d) (P:%s,U:%s,G:%s,A:%s,C:%s)\n",
      J->Name,
      (*QDef)->Name,
      (*QDef)->Index,
      (MPar[0].F.QDef != NULL)  ? ((mqos_t *)MPar[0].F.QDef)->Name : NONE,
      (J->Cred.U->F.QDef != NULL) ? ((mqos_t *)J->Cred.U->F.QDef)->Name : NONE,
      (J->Cred.G->F.QDef != NULL) ? ((mqos_t *)J->Cred.G->F.QDef)->Name : NONE,
      (AQD != NULL) ? AQD->Name : NONE,
      (CQD != NULL) ? CQD->Name : NONE);
    }  /* END BLOCK */
    }  /* END if (QDef != NULL) */
 
  if (QReq != NULL)
    {
    if (!MUBMCheck(QReq->Index,tmpQAL))
      {
      DBG(2,fSTRUCT) DPrint("WARNING:  job %s cannot access requested QOS[%d] %s\n",
        J->Name,
        QReq->Index,
        QReq->Name);
 
      return(FAILURE);
      }
    } 
 
  return(SUCCESS);
  }  /* END MQOSGetAccess() */




int MQOSListBMFromString(
 
  char *QOSString,
  int  *BM,
  int   Mode)
 
  {
  int   rangestart;
  int   rangeend;
 
  int   rindex;
 
  char  tmpLine[MAX_MLINE];
 
  char *rtok;
  char *tail;

  char  tmpName[MAX_MNAME];
 
  char *TokPtr;

  mqos_t *Q;
 
  if (BM == NULL)
    {
    return(FAILURE);
    }

  MUBMClear(BM,MAX_MQOS); 
 
  if (QOSString == NULL)
    {
    return(SUCCESS);
    }
 
  MUStrCpy(tmpLine,QOSString,sizeof(tmpLine));
 
  /* FORMAT:  QOSSTRING:   <RANGE>[:<RANGE>]... */
  /*          RANGE:       <VALUE>[-<VALUE>]    */
 
  /* NOTE:    The following non-numeric values may appear in the string */
  /*          an should be handled: '&', '^'                            */
 
  rtok = MUStrTok(tmpLine,",:",&TokPtr);
 
  while (rtok != NULL)
    {
    while ((*rtok == '&') || (*rtok == '^'))
      rtok++;
 
    rangestart = strtol(rtok,&tail,10);

    if ((rangestart != 0) || (rtok[0] == '0'))
      { 
      if (*tail == '-')
        rangeend = strtol(tail + 1,&tail,10);
      else
        rangeend = rangestart;

      rangeend = MIN(rangeend,31);
 
      for (rindex = rangestart;rindex <= rangeend;rindex++)
        {
        sprintf(tmpName,"%d",
           rindex);

        if (MQOSFind(tmpName,&Q) == SUCCESS)
          {
          MUBMSet(Q->Index,BM);
          }
        else if ((Mode == mAdd) && (Q != NULL))
          {
          MQOSAdd(tmpName,&Q);

          MUBMSet(Q->Index,BM); 
          }
        }    /* END for (rindex) */
      }
    else
      {
      /* QOS name provided */

      /* remove meta characters (handled externally) */

      if ((tail = strchr(rtok,'&')) != NULL)
        *tail = '\0';
      else if ((tail = strchr(rtok,'^')) != NULL)
        *tail = '\0';

      if (MQOSFind(rtok,&Q) == SUCCESS)
        {
        MUBMSet(Q->Index,BM);  
        }
      else if ((Mode == mAdd) && (Q != NULL))
        {
        MQOSAdd(rtok,&Q);

        MUBMSet(Q->Index,BM);  
        }
      }
       
    rtok = MUStrTok(NULL,",:",&TokPtr);
    }  /* END while (rtok) */
 
  return(SUCCESS);
  }  /* END MQOSListBMFromString() */




int MQOSAddToBM(

  mqos_t *Q,
  int    *BM)

  {
  /* NYI */

  return(SUCCESS);
  }  /* END MQOSAddToBM() */




char *MQOSBMToString(
 
  int *BM)
 
  {
  int     bindex;

  mqos_t *Q;
 
  static char tmpLine[MAX_MLINE];
 
  tmpLine[0] = '\0';

  if (BM == NULL)
    {
    return(tmpLine);
    }

  for (bindex = 0;bindex < MAX_MQOS;bindex++)
    {
    if (!(BM[bindex >> M64.INTLBITS] & (1 << (bindex % M64.INTBITS))))
      continue;

    Q = &MQOS[bindex];
 
    if (tmpLine[0] != '\0')
      strcat(tmpLine,":");

    if (Q->Name[0] != '\0')
      { 
      sprintf(tmpLine,"%s%s",
        tmpLine,
        Q->Name);
      }
    else
      {
      sprintf(tmpLine,"%s%d",
        tmpLine,
        Q->Index);
      }
    }  /* END for (bindex) */
 
  return(tmpLine);
  }  /* END MQOSBMToString() */




int MQOSShow(

  char *QName,        /* I */
  char *SBuffer,      /* O */
  long *SBufSize,     /* I */
  long  DisplayMode)  /* I */
 
  {
  int qindex;
 
  int index;
 
  mqos_t   *Q;
  mgcred_t  *U;
  mgcred_t *G;
  mgcred_t  *A;
  mclass_t *C;
 
  char      QALChar;
  char      Line[MAX_MLINE];
  char      JFlags[MAX_MLINE];
  char      QList[MAX_MLINE];     

  char     *lptr;

  char     *BPtr;
  int       BSpace;

  const char *FName = "MQOSShow";
 
  DBG(2,fUI) DPrint("%s(QName,SBuffer,SBufSize,DisplayMode)\n",
    FName);

  if (SBuffer == NULL)
    {
    return(FAILURE);
    }
 
  BPtr   =  SBuffer;
  BSpace = *SBufSize;

  MUSNPrintF(&BPtr,&BSpace,"QOS Status\n\n");
 
  MUStrCpy(QList,MQOSBMToString(MPar[0].F.QAL),sizeof(QList));
 
  MUSNPrintF(&BPtr,&BSpace,"System QOS Settings:  QList: %s (Def: %s)  Flags: %ld\n\n",
    (QList[0] != '\0') ? QList : NONE,
    ((mqos_t *)MPar[0].F.QDef)->Name,
    MPar[0].F.JobFlags);
 
  MUSNPrintF(&BPtr,&BSpace,"%-20s%c %8s %8s %8s %8s %8s %10s %10s %s\n\n",
    "Name",
    '*',
    "Priority",
    "QTWeight",
    "QTTarget",
    "XFWeight", 
    "XFTarget",
    "QFlags",
    "JobFlags",
    "Limits");
 
  for (qindex = 0;qindex < MAX_MQOS;qindex++)
    {
    Q = &MQOS[qindex];
 
    if ((Q->Name[0] == '\0') &&
        (Q->Flags == 0) &&
        (Q->F.Priority == 0) &&
        (Q->QTWeight == 0) &&
        (Q->XFWeight == 0) &&
        (Q->XFTarget == 0.0) &&
        (Q->QTTarget == 0))
      {
      continue;
      }
 
    MQOSFlagsToString(Q,Line,1); 
 
    if (Q->F.QALType == qalAND)
      QALChar = '&';
    else if (Q->F.QALType == qalONLY)
      QALChar = '^';
    else
      QALChar = ' ';

    lptr = MCredShowAttrs(
      &Q->L.AP,
      Q->L.IP,
      Q->L.OAP,
      Q->L.OIP,
      Q->L.OJP,
      &Q->F,
      0,
      (1 << mcsLimits));

    MUBMToString(Q->F.JobFlags,MJobFlags,':',JFlags,NONE);

    MUSNPrintF(&BPtr,&BSpace,"%-20s%c %8ld %8ld %8ld %8ld %8.2lf %10s %10s %s\n",
      (Q->Name[0] != '\0') ? Q->Name : NONE,
      QALChar,
      Q->F.Priority,
      Q->QTWeight,
      Q->QTTarget,
      Q->XFWeight,
      Q->XFTarget,
      (Line[0] != '\0') ? Line : NONE,
      JFlags,
      ((lptr == NULL) || (*lptr == '\0')) ? NONE : lptr);
 
    /* check user access */
 
    Line[0] = '\0';
 
    for (index = 0;index < MAX_MUSER + MAX_MHBUF;index++)
      {
      U = MUser[index];
 
      if ((U == NULL) || (U->Name[0] == '\0') || (U->Name[0] == '\1'))
        continue;
 
      if (MUBMCheck(qindex,U->F.QAL))
        {
        sprintf(Line,"%s %s%s",
          Line,
          U->Name,
          MQALType[U->F.QALType]);
        }
      }  /* END for (index) */
 
    if (Line[0] != '\0') 
      {
      MUSNPrintF(&BPtr,&BSpace,"  Users:    %s\n",
        Line);
      }
 
    /* check group access */
 
    Line[0] = '\0';
 
    for (index = 0;index < MAX_MGROUP + MAX_MHBUF;index++)
      {
      G = &MGroup[index];
 
      if ((G->Name[0] == '\0') || (G->Name[0] == '\1'))
        continue;
 
      if (MUBMCheck(qindex,G->F.QAL))
        {
        sprintf(Line,"%s %s%s",
          Line,
          G->Name,
          MQALType[G->F.QALType]);
        }
      }  /* END for (index) */
 
    if (Line[0] != '\0')
      {
      MUSNPrintF(&BPtr,&BSpace,"  Groups:   %s\n",
        Line);
      }
 
    /* check account access */
 
    Line[0] = '\0';
 
    for (index = 0;index < MAX_MACCT + MAX_MHBUF;index++)
      {
      A = &MAcct[index];
 
      if ((A->Name[0] == '\0') || (A->Name[0] == '\1'))
        continue; 
 
      if (MUBMCheck(qindex,A->F.QAL))
        {
        sprintf(Line,"%s %s%s",
          Line,
          A->Name,
          MQALType[A->F.QALType]);
        }
      }  /* END for (index) */
 
    if (Line[0] != '\0')
      {
      MUSNPrintF(&BPtr,&BSpace,"  Accounts: %s\n",
        Line);
      }

    /* check class access */

    Line[0] = '\0';

    for (index = 0;index < MAX_MCLASS;index++)
      {
      C = &MClass[index];

      if ((C->Name[0] == '\0') || (C->Name[0] == '\1'))
        continue;

      if (MUBMCheck(qindex,C->F.QAL))
        {
        sprintf(Line,"%s %s%s",
          Line,
          C->Name,
          MQALType[C->F.QALType]);
        }
      }  /* END for (index) */

    if (Line[0] != '\0')
      {
      MUSNPrintF(&BPtr,&BSpace,"  Classes: %s\n",
        Line);
      }
    }  /* END for (qindex) */
 
  return(SUCCESS);
  }  /* END MQOSShow() */




int MQOSConfigShow(

  mqos_t *Q,        /* I */
  int     VFlag,    /* I */
  int     PIndex,   /* I */
  char   *Buffer,   /* O */
  int     BufSize)  /* I */

  {
  if ((Q == NULL) || (Buffer == NULL))
    {
    return(FAILURE);
    }

  /* general credential config */

  if ((Q->Name[0] != '\0') && (Q->Name[1] != '\1'))
      MCredConfigShow((void *)Q,mxoQOS,VFlag,PIndex,Buffer);

  /* QOS specific config */

  if ((Q->F.Priority != 0) || (VFlag || (PIndex == -1) || (PIndex == pQOSPriority)))
    strcat(Buffer,MUShowIArray(MParam[pQOSPriority],Q->Index,Q->F.Priority));

  if ((Q->QTWeight != 0) || (VFlag || (PIndex == -1) || (PIndex == pQOSQTWeight)))
    strcat(Buffer,MUShowIArray(MParam[pQOSQTWeight],Q->Index,Q->QTWeight));

  if ((Q->XFWeight != 0) || (VFlag || (PIndex == -1) || (PIndex == pQOSXFWeight)))
    strcat(Buffer,MUShowIArray(MParam[pQOSXFWeight],Q->Index,Q->XFWeight));

  if ((Q->XFTarget != 0.0) || (VFlag || (PIndex == -1) || (PIndex == pQOSTargetXF)))
    strcat(Buffer,MUShowFArray(MParam[pQOSTargetXF],Q->Index,Q->XFTarget));

  if ((Q->QTTarget != 0) || (VFlag || (PIndex == -1) || (PIndex == pQOSTargetQT)))
    strcat(Buffer,MUShowSArray(MParam[pQOSTargetQT],Q->Index,MULToTString(Q->QTTarget)));

  if ((Q->Flags != 0) || (VFlag || (PIndex == -1) || (PIndex == pQOSFlags)))
    {
    char tmpLine[MAX_MLINE];

    MQOSFlagsToString(Q,tmpLine,0);

    strcat(Buffer,MUShowSArray(MParam[pQOSFlags],Q->Index,tmpLine));
    }     /* END if (Q->Flags != 0) */

  return(SUCCESS);
  }  /* END MQOSConfigShow() */





int __MQOSSetOverrideLimits(
 
  mqos_t *Q) /* I */
 
  {
  int index;
 
  if (Q == NULL)
    {
    return(FAILURE);
    }
 
  for (index = 0;MQOSFlags[index] != NULL;index++)
    {
    if (!(Q->Flags & (1 << index)))
      continue;
 
    switch(index)
      {
      case mqfignJobPerUser:
 
        Q->L.OJP->SLimit[mptMaxJob][0] = -1;
        Q->L.OJP->HLimit[mptMaxJob][0] = -1;

        Q->L.OAP->SLimit[mptMaxJob][0] = -1;
        Q->L.OAP->HLimit[mptMaxJob][0] = -1;
 
        break;
 
      case mqfignNodePerUser:
      case mqfignMaxProc:
 
        Q->L.OJP->SLimit[mptMaxNode][0] = -1;
        Q->L.OJP->HLimit[mptMaxNode][0] = -1;
 
        Q->L.OJP->SLimit[mptMaxProc][0] = -1;
        Q->L.OJP->HLimit[mptMaxProc][0] = -1;

        Q->L.OAP->SLimit[mptMaxNode][0] = -1;
        Q->L.OAP->HLimit[mptMaxNode][0] = -1;
 
        Q->L.OAP->SLimit[mptMaxProc][0] = -1;
        Q->L.OAP->HLimit[mptMaxProc][0] = -1;
 
        break;
 
      case mqfignPSPerUser:
      case mqfignMaxPS: 
 
        Q->L.OJP->SLimit[mptMaxPS][0] = -1;
        Q->L.OJP->HLimit[mptMaxPS][0] = -1;

        Q->L.OAP->SLimit[mptMaxPS][0] = -1;
        Q->L.OAP->HLimit[mptMaxPS][0] = -1;
 
        break;
 
      case mqfignJobQueuedPerUser:
 
        Q->L.OIP->SLimit[mptMaxJob][0] = -1;
        Q->L.OIP->HLimit[mptMaxJob][0] = -1;
 
        break;
      }  /* END switch(index) */
    }    /* END for (index) */
 
  return(SUCCESS);
  }  /* END __MQOSSetOverrideLimits() */




int MQOSProcessOConfig(

  mqos_t *Q,      /* I (modified) */
  int     PIndex, /* I */
  int     IVal,
  double  DVal,
  char   *SVal,
  char  **SArray)

  {
  if (Q == NULL)
    {
    return(FAILURE);
    }

  switch (PIndex)
    {
    case pQOSName:

      /* no longer supported */

      break;

    case pQOSFlags:

      {
      char tmpLine[MAX_MLINE];

      int aindex;

      tmpLine[0] = '\0';

      for (aindex = 0;SArray[aindex] != NULL;aindex++)
        {
        if (SArray[aindex][0] == '\0')
          break;
       
        MUStrCat(tmpLine,SArray[aindex],MAX_MLINE);
        MUStrCat(tmpLine," ",MAX_MLINE); 
        }  /* END for (aindex) */

      MQOSFlagsFromString(Q,tmpLine);
      }  /* END BLOCK */

      break;

    case pQOSPriority:

      Q->F.Priority = IVal;
 
      break;

    case pQOSQTWeight:
 
      Q->QTWeight = IVal;

      break;

    case pQOSTargetQT:
 
      Q->QTTarget = MUTimeFromString(SVal);
 
      break;

    case pQOSTargetXF:

      Q->XFTarget = DVal;
 
      break;

    case pQOSXFWeight:

      Q->XFWeight = IVal;
 
      break;

    default:

      return(FAILURE);
 
      /*NOTREACHED*/

      break;
    }  /* END switch(PIndex) */

  return(SUCCESS);
  }  /* END MQOSProcessOConfig() */




int MQOSAToString(

  mqos_t *Q,      /* I */
  int     AIndex, /* I */
  char   *Buf,    /* O */
  int     Mode)   /* I */

  {
  if ((Q == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  Buf[0] = '\0';

  switch(AIndex)
    {
    case mqaFlags:

      MQOSFlagsToString(Q,Buf,0);

      break;

    default:

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MQOSAToString() */




int MQOSSetAttr(

  mqos_t *Q,       /* I *(modified) */
  int     AIndex,  /* I */
  void  **Value,   /* I */
  int     Format,  /* I */
  int     Mode)    /* I */

  { 
  if (Q == NULL)
    {
    return(FAILURE);
    }

  switch(AIndex)
    {
    case mqaFlags:

      MQOSFlagsFromString(Q,(char *)Value);

      break;

    default:

      /* no handled */

      return(FAILURE);
 
      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MQOSSetAttr() */




int MQOSFlagsFromString(

  mqos_t *Q,   /* I (modified) */
  char   *Buf) /* I */

  {
  int vindex;

  if ((Q == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  Q->Flags = DEFAULT_MQOSFLAGS;
 
  for (vindex = 0;MQOSFlags[vindex] != NULL;vindex++)
    {
    if (strstr(Buf,MQOSFlags[vindex]) != NULL)
      {
      switch(vindex)
        {
        case mqfignJobPerUser:

          if (Q->L.OAP == NULL)
            Q->L.OAP = (mpu_t *)calloc(1,sizeof(mpu_t));

          Q->L.OAP->SLimit[mptMaxJob][0] = -1;
          Q->L.OAP->HLimit[mptMaxJob][0] = -1;

          Q->Flags |= (1 << vindex);

          break;

        case mqfignProcPerUser:

          if (Q->L.OAP == NULL) 
            Q->L.OAP = (mpu_t *)calloc(1,sizeof(mpu_t));

          Q->L.OAP->SLimit[mptMaxProc][0] = -1;
          Q->L.OAP->HLimit[mptMaxProc][0] = -1;

          Q->Flags |= (1 << vindex);

          break;

        case mqfignNodePerUser:

          if (Q->L.OAP == NULL)
            Q->L.OAP = (mpu_t *)calloc(1,sizeof(mpu_t));

          Q->L.OAP->SLimit[mptMaxNode][0] = -1;
          Q->L.OAP->HLimit[mptMaxNode][0] = -1;

          Q->Flags |= (1 << vindex);

          break;

        case mqfignMaxProc: 

          if (Q->L.OJP == NULL)
            Q->L.OJP = (mpu_t *)calloc(1,sizeof(mpu_t));

          Q->L.OJP->SLimit[mptMaxNode][0] = -1;
          Q->L.OJP->HLimit[mptMaxNode][0] = -1;

          Q->L.OJP->SLimit[mptMaxProc][0] = -1;
          Q->L.OJP->HLimit[mptMaxProc][0] = -1;

          Q->Flags |= (1 << vindex);

          break;

        case mqfignPSPerUser:

          if (Q->L.OAP == NULL)
            Q->L.OAP = (mpu_t *)calloc(1,sizeof(mpu_t));

          Q->L.OAP->SLimit[mptMaxPS][0] = -1;
          Q->L.OAP->HLimit[mptMaxPS][0] = -1;

          Q->Flags |= (1 << vindex);

          break;

        case mqfignMaxPS:

          if (Q->L.OJP == NULL)
            Q->L.OJP = (mpu_t *)calloc(1,sizeof(mpu_t));

          Q->L.OJP->SLimit[mptMaxPS][0] = -1;
          Q->L.OJP->HLimit[mptMaxPS][0] = -1;

          Q->Flags |= (1 << vindex);

          break;

        case mqfignJobQueuedPerUser:

          Q->L.OIP->SLimit[mptMaxJob][0] = -1;
          Q->L.OIP->HLimit[mptMaxJob][0] = -1;

          Q->Flags |= (1 << vindex); 

          break;

        case mqfignMaxTime:
        case mqfignSRMaxTime:
        case mqfpreemptor:
        case mqfpreemptee:
        case mqfdedicated:
        case mqfreserved:
        case mqfnobf:
        case mqfNTR:
        case mqfRunNow:
        case mqfPreemptSPV:
        case mqfignHostList:

          Q->Flags |= (1 << vindex);

          break;

        case mqfusereservation:

          {
          char tmpLine[MAX_MLINE];

          char *ptr;
          char *TokPtr;

          /* FORMAT:  RESACCESS=<X>[:<X>]... */

          /* FIXME:  local parsing creating higher layer conflicts */

          Q->Flags |= (1 << vindex);

          sscanf(tmpLine,"%1024s ",
            Buf);

          if (((ptr = MUStrTok(tmpLine,":",&TokPtr)) != NULL) &&
             (((ptr = MUStrTok(NULL,":",&TokPtr)) != NULL)))
            {
            int rindex;

            /* res access list specified */

            for (rindex = 0;rindex < 12;rindex++)
              {
              MUStrCpy(Q->ResName[rindex],ptr,sizeof(Q->ResName[rindex]));

              if ((ptr = MUStrTok(NULL,":",&TokPtr)) == NULL)
                break;
              }  /* END for (rindex) */
            }
          }

          break; 
 
        case mqfignUser:

          Q->Flags |= QFUSER;

          break;

        case mqfignSystem:

          Q->Flags |= QFSYSTEM;

          break;

        case mqfignAll:

          Q->Flags |= QFALL;

          break;
 
        default:

          DBG(2,fCONFIG) DPrint("ALERT:    unexpected QOS flag value detected: %d\n",
            vindex);
 
          break;
        }  /* END switch (vindex) */
      }    /* END if strstr()     */ 
    }      /* END for (vindex)    */
 
  __MQOSSetOverrideLimits(Q);
 
  DBG(2,fCONFIG) DPrint("INFO:     QOS '%s' flags set to %lu\n",
    Q->Name,
    Q->Flags);

  return(SUCCESS);
  }  /* END MQOSFlagsFromString() */




char *MQOSFlagsToString(

  mqos_t *Q,    /* I */
  char   *Buf,  /* O */
  int     Mode) /* I */

  {
  static char Line[MAX_MLINE];

  char SepString[2];

  if ((Q == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  if (Mode == 0)
    strcpy(SepString," ");
  else
    strcpy(SepString,",");

  Line[0] = '\0';

  if ((Q->Flags & QFALL) == QFALL)
    {
    strcpy(Line,MQOSFlags[mqfignAll]);
    }
  else
    {
    /* add user flags */

    if ((Q->Flags & QFUSER) == QFUSER)
      {
      if (Line[0] != '\0')
        strcat(Line,SepString);

      strcat(Line,MQOSFlags[mqfignUser]);
      }
    else
      {
      if (Q->Flags & (1 << mqfignJobPerUser))
        {
        if (Line[0] != '\0')
          strcat(Line,SepString);

        strcat(Line,MQOSFlags[mqfignJobPerUser]);
        }

      if (Q->Flags & (1 << mqfignProcPerUser))
        {
        if (Line[0] != '\0')
          strcat(Line,SepString);

        strcat(Line,MQOSFlags[mqfignProcPerUser]);
        }

      if (Q->Flags & (1 << mqfignNodePerUser))
        {
        if (Line[0] != '\0')
          strcat(Line,SepString);

        strcat(Line,MQOSFlags[mqfignNodePerUser]);
        } 

      if (Q->Flags & (1 << mqfignPSPerUser))
        {
        if (Line[0] != '\0')
          strcat(Line,SepString);

        strcat(Line,MQOSFlags[mqfignPSPerUser]);
        }

      if (Q->Flags & (1 << mqfignJobQueuedPerUser))
        {
        if (Line[0] != '\0')
          strcat(Line,SepString);

        strcat(Line,MQOSFlags[mqfignJobQueuedPerUser]);
        }
      }    /* END else ((Q->Flags & QFUSER) == QFUSER) */

    /* add system flags */

    if ((Q->Flags & QFSYSTEM) == QFSYSTEM)
      {
      if (Line[0] != '\0')
        strcat(Line,SepString);

      strcat(Line,MQOSFlags[mqfignSystem]);
      }
    else
      {
      if (Q->Flags & (1 << mqfignMaxProc))
        {
        if (Line[0] != '\0')
          strcat(Line,SepString);

        strcat(Line,MQOSFlags[mqfignMaxProc]); 
        }

      if (Q->Flags & (1 << mqfignMaxTime))
        {
        if (Line[0] != '\0')
          strcat(Line,SepString);

        strcat(Line,MQOSFlags[mqfignMaxTime]);
        }

      if (Q->Flags & (1 << mqfignMaxPS))
        {
        if (Line[0] != '\0')
          strcat(Line,SepString);

        strcat(Line,MQOSFlags[mqfignMaxPS]);
        }
      }

    /* add SR time ACL flag */

    if (Q->Flags & (1 << mqfignSRMaxTime))
      {
      if (Line[0] != '\0')
        strcat(Line,SepString);

      strcat(Line,MQOSFlags[mqfignSRMaxTime]);
      }
    }   /* END if (MQOSFlags[] = QFALL) */

  /* add non-'fairness policy' flags */

  {
  int findex;

  int FList[] = {
    mqfpreemptor,
    mqfpreemptee,
    mqfdedicated,
    mqfreserved,
    mqfRunNow,
    mqfnobf,
    mqfPreemptSPV,
    mqfignHostList,
    -1 };

  for (findex = 0;FList[findex] != -1;findex++)
    {
    if (Q->Flags & (1 << FList[findex]))
      {
      if (Line[0] != '\0')
        strcat(Line,SepString);

      strcat(Line,MQOSFlags[FList[findex]]);
      } 
    }    /* END for (findex) */
  }

  if (Q->Flags & (1 << mqfusereservation))
    {
    if (Line[0] != '\0')
      strcat(Line,SepString);

    strcat(Line,MQOSFlags[mqfusereservation]);

    if ((Q->ResName != NULL) && (Q->ResName[0][0] != '\0'))
      {
      int rindex;

      /* res access list specified */

      for (rindex = 0;rindex < 12;rindex++)
        {
        if (Q->ResName[rindex][0] == '\0')
          {
          break; 
          }

        sprintf(Line,"%s:%s",
          Line,
          Q->ResName[rindex]);
        }  /* END for (rindex) */
      }
    }

  if (Buf != NULL)
    {
    strcpy(Buf,Line);

    return(Buf);
    }

  return(Line);
  }  /* END MQOSFlagsToString() */




int MQOSConfigLShow(

  mqos_t *Q,      /* I */
  int     VFlag,  /* I */
  int     PIndex, /* I */
  char   *Buffer) /* O */

  {
  int AList[] = {
    -1 };

  int aindex;

  char tmpString[MAX_MLINE];

  if ((Q == NULL) || (Buffer == NULL))
    {
    return(FAILURE);
    }

  /* clear buffer */

  Buffer[0] = '\0';

  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    if ((MQOSAToString(Q,AList[aindex],tmpString,0) == FAILURE) ||
        (tmpString[0] == '\0'))
      {
      continue;
      }

    sprintf(Buffer,"%s %s=%s",
      Buffer,
      MQOSAttr[AList[aindex]],
      tmpString);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MQOSConfigLShow() */




int MQOSDestroy(

  mqos_t **QP)  /* I (modified) */

  {
  mqos_t *Q;

  if ((QP == NULL) || (*QP == NULL))
    {
    return(SUCCESS);
    }

  Q = *QP;

  MUFree((char **)&Q->L.OAP);
  MUFree((char **)&Q->L.OIP);
  MUFree((char **)&Q->L.OJP);

  return(SUCCESS);
  }  /* END MQOSDestroy() */




int MQOSFreeTable()

  {
  int qindex;

  mqos_t *Q;

  for (qindex = 0;qindex < MAX_MQOS;qindex++)
    {
    Q = &MQOS[qindex];

    MQOSDestroy(&Q);
    }  /* END for (qindex) */

  return(SUCCESS);
  }  /* END MQOSFreeTable() */


/* END MQOS.c */

