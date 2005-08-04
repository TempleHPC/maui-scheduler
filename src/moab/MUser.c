/* HEADER */
 
/* Contains:                                         *
 *  int MUserAdd(UserName,U)                         *
 *  int MUserInitialize(U,UserName)                  *
 *  int MUserFind(U,UName)                           *
 *  int MUserShow(U,Buffer,Mode)                     *
 *                                                   */

#include "moab.h"
#include "msched-proto.h"  

extern mlog_t     mlog;
 
extern msched_t   MSched;
extern mgcred_t   *MUser[];
extern mgcred_t   MGroup[];
extern mckpt_t    MCP;

extern const char *MJobFlags[];
extern const char *MCredAttr[];
extern const char *MXO[];




int MUserLoadCP(

  mgcred_t *US,  /* I (optional) */
  char    *Buf) /* I */

  {
  char    tmpHeader[MAX_MNAME];
  char    UName[MAX_MNAME];
 
  char   *ptr;
 
  mgcred_t *U;
 
  long    CkTime;
 
  mxml_t *E = NULL;
 
  const char *FName = "MUserLoadCP";
 
  DBG(4,fCKPT) DPrint("%s(US,%s)\n",
    FName,
    (Buf != NULL) ? Buf : "NULL"); 
 
  if (Buf == NULL)
    {
    return(FAILURE);
    }
 
  /* FORMAT:  <HEADER> <UID> <CKTIME> <USTRING> */
 
  /* load CP header */
 
  sscanf(Buf,"%s %s %ld",
    tmpHeader,
    UName,
    &CkTime);
 
  if (((long)MSched.Time - CkTime) > MCP.CPExpirationTime)
    return(SUCCESS);
 
  if (US == NULL)
    {
    if (MUserAdd(UName,&U) != SUCCESS)
      {
      DBG(5,fCKPT) DPrint("ALERT:    cannot load CP user '%s'\n",
        UName);
 
      return(FAILURE);
      }
    }
  else
    {
    U = US;
    }
 
  if ((ptr = strchr(Buf,'<')) == NULL)
    {
    return(FAILURE);
    }

  MXMLFromString(&E,ptr,NULL,NULL);
 
  MOFromXML((void *)U,mxoUser,E);
 
  MXMLDestroyE(&E);

  return(SUCCESS);
  }  /* END MUserLoadCP() */





int MUserToString(

  mgcred_t *U,   /* I */
  char    *Buf) /* O */

  {
  const int CPCList[] = {
    mxoStats,
    -1 };
 
  mxml_t *E = NULL;
 
  if ((U == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  Buf[0] = '\0';
 
  MCOToXML((void *)U,mxoUser,&E,NULL,(int *)CPCList,0);
 
  MXMLToString(E,Buf,MAX_MBUFFER,NULL,TRUE);
 
  MXMLDestroyE(&E);
 
  return(SUCCESS);
  }  /* END MUserToString() */





int MUserAdd(
 
  char     *UName,  /* I */
  mgcred_t **UP)     /* O (optional) */
 
  {
  int           uindex;
 
  unsigned long hashkey;
 
  mgcred_t      *U;

  const char *FName = "MUserAdd";
 
  DBG(6,fSTRUCT) DPrint("%s(%s,%s)\n",
    FName,
    (UName != NULL) ? "UName" : "NULL",
    (UP != NULL) ? "UP" : "NULL");

  if ((UName == NULL) || (UName[0] == '\0'))
    {
    return(FAILURE);
    }
 
  if (UP != NULL)
    *UP = NULL;
 
  hashkey = MAX(1,MUGetHash(UName) % MAX_MUSER);
 
  for (uindex = hashkey;uindex < MAX_MUSER + MAX_MHBUF;uindex++)
    {
    U = MUser[uindex];
 
    /* if user already in hash table */

    if ((U != NULL) && 
        (U->Key == hashkey) && 
         !strcmp(U->Name,UName))
      {
      if (UP != NULL)
        *UP = U;
 
      return(SUCCESS);
      }
 
    /* if empty slot found */
 
    if ((U == NULL) || (U->Name[0] == '\0') || (U->Name[0] == '\1'))
      {
      /* setup new record */

      if (U == NULL)
        {
        if (MUserCreate(UName,&MUser[uindex]) == FAILURE)
          {
          DBG(1,fALL) DPrint("ERROR:    cannot alloc memory for user '%s'\n",
            UName);

          return(FAILURE);
          }

        U = MUser[uindex];
        }
     
      if (UP != NULL)
        *UP = U;
 
      U->Key = hashkey;
 
      U->Index = uindex;
 
      MUStrCpy(U->Name,UName,sizeof(U->Name));
 
      if (strcmp(UName,ALL) && strcmp(UName,NONE))
        {
        /* update user record */
 
        MCredAdjustConfig(mxoUser,U);
 
        if (MSched.Mode != msmSim)
          MCPRestore(mcpUser,UName,(void *)U);
 
        U->OID = MUUIDFromName(U->Name);
 
        DBG(5,fSTRUCT) DPrint("INFO:     user %s added\n",
          UName);
        }
      else
        {
        /* no config required for psuedo job */
        }
 
      return(SUCCESS);
      }
    }    /* END for (uindex) */
 
  /* end of table reached */
 
  DBG(1,fSTRUCT) DPrint("ALERT:    user table overflow.  cannot add %s\n",
    UName);
 
  return(FAILURE);
  }  /* END MUserAdd() */




int MUserFind(

  char     *UName,
  mgcred_t **UP)

  {
  /* if found, return success with U pointing to user. */

  int      uindex;
  mgcred_t *U;

  if (UP != NULL)
    *UP = NULL;

  if ((UName == NULL) || 
      (UName[0] == '\0') || 
      (UP == NULL))
    {
    return(FAILURE);
    }

  for (uindex = 0;uindex < MAX_MUSER + MAX_MHBUF;uindex++)
    {
    U = MUser[uindex];

    if ((U == NULL) || (U->Name[0] <= '\1')) 
      {  
      continue;
      }

    if (strcmp(U->Name,UName) != 0)
      continue;

    /* user found */

    *UP = U;

    return(SUCCESS);
    }  /* END for (uindex) */

  /* entire table searched */

  return(FAILURE);
  }  /* END MUserFind() */




int MUserInitialize(

  mgcred_t *U,
  char    *UName)

  {
  if ((U == NULL) || 
      (UName == NULL) || 
      (UName[0] == '\0'))
    {
    return(FAILURE);
    }

  memset(U,0,sizeof(mgcred_t));

  MUStrCpy(U->Name,UName,sizeof(U->Name));

  return(SUCCESS);
  }  /* END MUserInitialize() */




char *MUserShow(

  mgcred_t *U,        /* I */
  char    *SBuffer,  /* O (optional) */
  long    *SBufSize, /* I/O (optional) */
  long     Mode)     /* I */

  {
  static char Line[MAX_MLINE];

  char FlagLine[MAX_MLINE];

  char QALLine[MAX_MLINE];
  char QALChar;

  char ULLine[MAX_MLINE];

  char *Head;

  char *BPtr;
  int   BSpace;

  const char *FName = "MUserShow";

  DBG(3,fUI) DPrint("%s(%s,Buf,%ld,%ld)\n",
    FName,
    (U != NULL) ? U->Name : "NULL",
    (SBufSize != NULL) ? *SBufSize : -1,
    Mode);

  if (SBuffer != NULL)
    {
    BPtr   = SBuffer;
    BSpace = (int)*SBufSize;
    }
  else
    {
    BPtr   = Line;
    BSpace = sizeof(Line);
    }

  Head    = BPtr;
  Head[0] = '\0';

  if (U == NULL)
    {
    /* build header */

    /*            NAME PRI FLAG QDEF QLST * PLST TRG LIMITS */
 
    MUSNPrintF(&BPtr,&BSpace,"%-12s %8s %12s %12s %12s%s %20s %6s %7s\n\n",
      "Name",
      "Priority",
      "Flags",
      "QDef",
      "QOSList",
      "*",
      "PartitionList",
      "Target",
      "Limits"); 
    }
  else
    {
    /* build job info line */

    MUBMToString(U->F.JobFlags,MJobFlags,':',FlagLine,NONE);
 
    MUStrCpy(QALLine,MQOSBMToString(U->F.QAL),sizeof(QALLine));

    if (U->F.QALType == qalAND)
      QALChar = '&';
    else if (U->F.QALType == qalONLY)
      QALChar = '^';
    else
      QALChar = ' ';

    MUStrCpy(
      ULLine,
      MCredShowAttrs(
        &U->L.AP,
        U->L.IP,
        NULL,
        NULL,
        NULL,
        &U->F,
        0,
        (1 << mcsLimits)),
      sizeof(ULLine));        
 
    /*            NAME PRIO FLAG QDEF QLST * PLST FSTARG LIMITS */
 
    MUSNPrintF(&BPtr,&BSpace,"%-12s %8ld %12s %12s %12s%c %20s %6.2lf %7s\n",
      U->Name,
      U->F.Priority,
      FlagLine,
      ((mqos_t *)U->F.QDef) != NULL ?
        ((mqos_t *)U->F.QDef)->Name  :
        NONE,
      (QALLine[0] != '\0') ? QALLine : NONE,
      QALChar,
      (U->F.PAL[0] == 0) ?
        NONE :
        MUListAttrs(ePartition,U->F.PAL[0]),
      U->F.FSTarget,
      (ULLine[0] != '\0') ? ULLine : NONE);

    /* display additional attributes */

    if (U->F.PDef != NULL)
      {
      MUSNPrintF(&BPtr,&BSpace,"  PDEF=%s\n",
        ((mpar_t *)U->F.PDef)->Name);
      }

    if (U->L.APG != NULL)
      {
      int gindex;

      for (gindex = 0;gindex < MAX_MGROUP + MAX_MHBUF;gindex++)
        {
        if (U->L.APG[gindex].SLimit[mptMaxProc][0] > 0)
          {
          MUSNPrintF(&BPtr,&BSpace,"  MAXPROC[GROUP:%s]=%d,%d\n",
            MGroup[gindex].Name,
            U->L.APG[gindex].SLimit[mptMaxProc][0],
            U->L.APG[gindex].HLimit[mptMaxProc][0]);
          }
        }  /* END for (gindex) */
      }    /* END if (G->L.APG != NULL) */
    }  /* END else (U == NULL) */

  return(Head);
  }  /* END MUserShow() */




int MUserCreate(

  char      *UName,  /* I (optional) */
  mgcred_t **UP)     /* O */

  {
  mgcred_t *U;

  if ((UName == NULL) || (UName[0] == '\0') || (UP == NULL))
    {
    return(FAILURE);
    }

  *UP = NULL;

  if ((U = (mgcred_t *)calloc(1,sizeof(mgcred_t))) == NULL)
    {
    DBG(1,fALL) DPrint("ERROR:    cannot alloc memory for user '%s'\n",
      UName);
 
    return(FAILURE);
    }

  /* allocate dynamic memory */

  U->L.IP = (mpu_t *)calloc(1,sizeof(mpu_t));

  *UP = U;

  return(SUCCESS);
  }  /* END MUserCreate() */




int MCOToXML(

  void     *O,       /* I */
  int       OIndex,  /* I */
  mxml_t **EP,      /* O */
  int      *SAList,  /* I (optional) */
  int      *SCList,  /* I (optional) */
  int       Mode)    /* I */

  {
  const int DAList[] = {
    mcaID,
    -1 };

  const int DCList[] = {
    mxoStats,
    -1 };
 
  int  aindex;
  int  cindex;
 
  int *AList;
  int *CList;

  char tmpString[MAX_MLINE];

  mxml_t *XC;

  void *C;
 
  if ((O == NULL) || (EP == NULL))
    {
    return(FAILURE);
    }
 
  if (SAList != NULL)
    AList = SAList;
  else
    AList = (int *)DAList;

  if (SCList != NULL)
    CList = SCList;
  else
    CList = (int *)DCList;

  MXMLCreateE(EP,(char *)MXO[OIndex]);     
 
  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    if ((MCredAToString(O,OIndex,AList[aindex],tmpString,0) == FAILURE) ||
        (tmpString[0] == '\0'))
      {
      continue;
      }
 
    MXMLSetAttr(*EP,(char *)MCredAttr[AList[aindex]],tmpString,mdfString);
    }  /* END for (aindex) */

  for (cindex = 0;CList[cindex] != -1;cindex++)
    {
    XC = NULL;

    if ((MOGetComponent(O,OIndex,&C,CList[cindex]) == FAILURE) ||   
        (MOToXML(C,CList[cindex],&XC) == FAILURE))
      {
      continue;
      }

    MXMLAddE(*EP,XC);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MCOToXML() */




int MUserDestroy(

  mgcred_t **UP)

  {
  mgcred_t *U;

  if ((UP == NULL) || (*UP == NULL))
    {
    return(SUCCESS);
    }

  U = *UP;

  MUFree((char **)U->L.IP);
  MUFree((char **)UP);

  return(FAILURE);
  }  /* END MUserDestroy() */




int MUserFreeTable()

  {
  int uindex;

  for (uindex = 0;uindex < MAX_MUSER + MAX_MHBUF;uindex++)
    {
    if (MUser[uindex] == NULL)
      continue;

    MUserDestroy(&MUser[uindex]);
    }  /* END for (uindex) */

  return(SUCCESS);
  }  /* END MUserFreeTable() */


/* END MUser.c */

