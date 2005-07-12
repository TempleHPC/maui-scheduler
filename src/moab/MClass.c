/* HEADER */
 
/* Contains:                                         *
 *  int MClassFind(CName,C)                          *
 *  int MClassInitialize(C,CName)                    *
 *                                                   */

#include "moab.h"
#include "msched-proto.h"

extern mlog_t      mlog;
 
extern msched_t    MSched;
extern mclass_t    MClass[];
extern mattrlist_t MAList;
extern mckpt_t     MCP;

extern const char *MJobFlags[];        
extern const char *MClassAttr[];
extern const char *MCredAttr[];
extern const char *MXO[];




int MClassFind(

  char      *CName,  /* I */
  mclass_t **C)      /* O (optional) */

  {
  int cindex;

  const char *FName = "MClassFind";

  DBG(5,fSTRUCT) DPrint("%s(%s,C)\n",
    FName,
    (CName != NULL) ? CName : "NULL");

  if (C != NULL)
    *C = NULL;

  if ((CName == NULL) || 
      (CName[0] == '\0') || 
      (C == NULL))
    {
    return(FAILURE);
    }

  for (cindex = 1;MClass[cindex].Name[0] != '\0';cindex++)
    {
    if (!strcmp(MClass[cindex].Name,CName))
      {
      *C = &MClass[cindex];
 
      return(SUCCESS);
      }

    if (MClass[cindex].Name[0] == '\0')
      {
      *C = &MClass[cindex];
 
      break;
      }
    }    /* END for (cindex) */

  return(FAILURE);
  }  /* END MClassFind() */




int MClassInitialize(

  mclass_t *C,
  char     *CName)

  {
  if ((C == NULL) || 
      (CName == NULL) || 
      (CName[0] == '\0'))
    {
    return(FAILURE);
    }

  /* NYI */

  return(SUCCESS);
  }  /* END MClassInitialize() */




int MClassAdd(

  char      *CName,  /* I */
  mclass_t **C)      /* O */
 
  {
  int cindex;

  if ((CName == NULL) ||
      (CName[0] == '\0'))
    {
    return(FAILURE);
    }

  /* NOTE:  CA to Class mapping prevents use of class slot 0. */
   
  for (cindex = 1;cindex < MAX_MCLASS;cindex++)
    {
    if (MClass[cindex].Name[0] == '\0')
      {
      /* emply class slot located */
 
      memset(&MClass[cindex],0,sizeof(mclass_t));

      MClass[cindex].Index = cindex;

      MClass[cindex].MTime = MSched.Time;
 
      MUStrCpy(MClass[cindex].Name,CName,sizeof(MClass[cindex].Name));

      /* populate attrlist */

      strcpy(MAList[eClass][cindex],CName);
 
      if (C != NULL)
        *C = &MClass[cindex];
 
      return(SUCCESS);
      }
 
    if (!strcmp(MClass[cindex].Name,CName))
      {
      if (C != NULL)
        *C = &MClass[cindex];
 
      return(SUCCESS);
      }
    }    /* END for (cindex) */
 
  if (C != NULL)
    *C = NULL;
 
  return(FAILURE);
  }  /* END MClassAdd() */



int MClassDestroy(

  mclass_t **C)

  {
  if (C == NULL)
    return(SUCCESS);

  /* NYI */

  /* NOTE:  evaluate ramifications of destroying class on index *
   *        based class references */

  return(FAILURE);
  }  /* END MClassDestroy() */




int MClassShow(

  mclass_t *CP,       /* I */
  char     *SBuffer,  /* O */
  long     *SBufSize, /* O */
  int       DMode)    /* I */
 
  {
  int       cindex;
  int       aindex;

  char      tmpLine[MAX_MLINE];
 
  char      tmpQALString[MAX_MLINE];
  char      tmpLString[MAX_MLINE];
 
  int       findex;
 
  char      QALChar;
 
  mclass_t *C;

  int ClAList[] = {
    mclaOCNode,
    mclaOCDProcFactor,
    mclaDefReqFeature,
    mclaMaxProcPerNode,
    mclaWCOverrun,
    -1 };

  int CAList[] = {
    mcaMaxProcPerUser,
    mcaMaxNodePerUser,
    mcaDefWCLimit,
    mcaMaxWCLimit,
    mcaMaxNodePerJob,
    mcaMaxProcPerJob,
    mcaMaxJobPerUser,
    -1 };

  char     *BPtr;
  int       BSpace;

  const char *FName = "MClassShow";

  DBG(3,fUI) DPrint("%s(%s,SBuffer,SBufSize,%d)\n",
    FName,
    (CP != NULL) ? CP->Name : "NULL",
    DMode);

  if (SBuffer == NULL)
    {
    return(FAILURE);
    }

  BPtr   =  SBuffer;
  BSpace = *SBufSize;

  MUSNPrintF(&BPtr,&BSpace,"Class/Queue Status\n\n");
 
  /*                        NAME PRI  FLAG  QDEF  QLST M PLST  FST  Limits */

  MUSNPrintF(&BPtr,&BSpace,"%-14s %-8s %-12s %-12s %12s%c %-20s %-6s %s\n\n",
    "Name",
    "Priority",
    "Flags",
    "QDef",
    "QOSList",
    '*',
    "PartitionList",
    "Target",
    "Limits");
 
  for (cindex = 0;cindex < MAX_MCLASS;cindex++)
    {
    C = &MClass[cindex];
 
    DBG(8,fUI) DPrint("INFO:     checking MClass[%02d]: %s\n",
      cindex,
      C->Name);
 
    if (C->Name[0] == '\0')
      continue;

    if (!strcmp(C->Name,NONE) || !strcmp(C->Name,ALL))
      continue;

    if ((CP != NULL) && (strcmp(CP->Name,NONE) != 0) && (C != CP))
      continue;
 
    tmpLine[0] = '\0';
 
    for (findex = 0;MJobFlags[findex] != NULL;findex++)
      {
      if (!(C->F.JobFlags & (1 << findex)))
        continue;
 
      if (tmpLine[0] != '\0')
        strcat(tmpLine,":");
 
      strcat(tmpLine,MJobFlags[findex]);
      }  /* END for (findex) */
 
    if (C->F.QALType == qalAND)
      QALChar = '&';
    else if (C->F.QALType == qalONLY)
      QALChar = '^';
    else
      QALChar = ' ';
 
    MUStrCpy(tmpQALString,MQOSBMToString(C->F.QAL),sizeof(tmpQALString));

    MUStrCpy(tmpLString,
      MCredShowAttrs(
        &C->L.AP,
        C->L.IP,
        NULL,
        NULL,
        NULL,
        &C->F,
        0,
        (1 << mcsLimits) | (1 << mcsUsage)),
        sizeof(tmpLString));
 
    /*                         NAME PRIO FLAG  QDEF  QLST M PLST  FSTARG Limits */
 
    MUSNPrintF(&BPtr,&BSpace,"%-14s %8ld %-12s %-12s %12s%c %-20s %5.2lf %7s\n",
      C->Name,
      C->F.Priority,
      (tmpLine[0] != '\0') ? tmpLine : NONE,
      ((mqos_t *)C->F.QDef) != NULL ?
        ((mqos_t *)C->F.QDef)->Name :
        NONE,
      (tmpQALString[0] != '\0') ? tmpQALString : NONE,
      QALChar,
      (C->F.PAL[0] == 0) ?
        NONE :
        MUListAttrs(ePartition,C->F.PAL[0]),
      C->F.FSTarget,
      (tmpLString[0] != '\0') ? tmpLString : NONE);

    /* list extended class attributes */

    tmpLine[0] = '\0';

    for (aindex = 0;ClAList[aindex] != -1;aindex++)
      {
      if ((MClassAToString(C,ClAList[aindex],tmpLString,mdfString,0) == SUCCESS) && 
          (tmpLString[0] != '\0') && 
           strcmp(tmpLString,NONE))
        {
        sprintf(tmpLine,"%s  %s=%s",
          tmpLine,
          MClassAttr[ClAList[aindex]],
          tmpLString);
        }
      }    /* END for (aindex) */

    /* list extended cred attributes */

    for (aindex = 0;CAList[aindex] != -1;aindex++)
      {
      if ((MCredAToString((void *)C,mxoClass,CAList[aindex],tmpLString,mdfString) == SUCCESS) &&
          (tmpLString[0] != '\0'))
        {
        sprintf(tmpLine,"%s  %s=%s",
          tmpLine,
          MCredAttr[CAList[aindex]],
          tmpLString);
        }
      }    /* END for (aindex) */

    if (tmpLine[0] != '\0')
      {
      MUStrNCat(&BPtr,&BSpace,tmpLine);
      MUStrNCat(&BPtr,&BSpace,"\n");
      }
    }  /* END for (cindex) */

  return(SUCCESS);
  }  /* END MClassShow() */




int MClassProcessConfig(

  mclass_t *C,     /* I (modified) */
  char     *Value) /* I */

  {
  char *ptr;
  char *TokPtr;

  int   aindex;

  char  ValLine[MAX_MNAME];

  const char *FName = "MClassProcessConfig";

  DBG(5,fCONFIG) DPrint("%s(%s,%s)\n",
    FName,
    (C != NULL) ? C->Name : "NULL",
    (Value != NULL) ? Value : "NULL");

  if ((C == NULL) ||
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
          (const char **)MClassAttr,
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

    /* NOTE:  only allow certain attributes to be set via config */

    switch(aindex)
      {
      case mclaOCNode:
      case mclaDefReqFeature:
      case mclaMaxProcPerNode:
      case mclaWCOverrun:

        MClassSetAttr(C,aindex,(void **)ValLine,mdfString,mSet);

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch(AIndex) */

    ptr = MUStrTok(NULL," \t\n",&TokPtr);
    }  /* END while (ptr != NULL) */

  return(SUCCESS);
  }  /* END MClassProcessConfig() */




int MClassConfigLShow(

  mclass_t *C,
  int       VFlag,
  int       PIndex,
  char     *Buffer)

  {
  int AList[] = {
    mclaOCNode,
    mclaDefReqFeature,
    -1 };

  int aindex;
 
  char tmpString[MAX_MLINE];

  if ((C == NULL) || (Buffer == NULL))
    {
    return(FAILURE);
    }

  /* clear buffer */

  Buffer[0] = '\0';

  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    if ((MClassAToString(C,AList[aindex],tmpString,mfmHuman,0) == FAILURE) ||
        (tmpString[0] == '\0'))
      {
      continue;
      }

    sprintf(Buffer,"%s %s=%s",
      Buffer,
      MClassAttr[AList[aindex]],
      tmpString);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MClassConfigLShow() */




int MClassSetAttr(

  mclass_t *C,      /* I (modified) */
  int       AIndex, /* I */
  void    **Value,  /* I */
  int       Format, /* I */
  int       Mode)   /* I */

  {
  if (C == NULL)
    {
    return(FAILURE);
    }

  switch(AIndex)
    {
    case mclaOCNode:

      MUStrDup(&C->OCNodeName,(char *)Value);

      break;

    case mclaDefReqFeature:

      {
      /* FORMAT:  <FEATURE>[{ \t:,}<FEATURE>]... */

      char *ptr;
      char *TokPtr;

      /* detect default feature requirements */

      ptr = MUStrTok((char *)Value,":",&TokPtr);

      while (ptr != NULL)
        {
        MUGetMAttr(eFeature,ptr,mAdd,C->DefFBM,sizeof(C->DefFBM));

        ptr = MUStrTok(NULL,":",&TokPtr);
        }  /* END while (ptr != NULL) */
      }    /* END BLOCK */

      break;

    case mclaMaxProcPerNode:

      {
      int tmpI;

      if (Format == mdfString)
        tmpI = (int)strtol((char *)Value,NULL,0);
      else
        tmpI = *(int *)Value;

      C->MaxProcPerNode = tmpI;
      }  /* END BLOCK */

      break;

    case mclaOCDProcFactor:

      {
      double tmpD;

      if (Format == mdfString)
        tmpD = (double)strtod((char *)Value,NULL);
      else
        tmpD = *(double *)Value;

      C->OCDProcFactor = tmpD;
      }  /* END BLOCK */

      break;

    case mclaState:

      {
      int tmpI;

      if (Format == mdfString)
        tmpI = (int)strtol((char *)Value,NULL,0);
      else
        tmpI = *(int *)Value;

      C->IsDisabled = (tmpI == mjsRunning) ? TRUE : FALSE;
      }  /* END BLOCK */

      break;

    case mclaWCOverrun:

      {
      int tmpL;

      if (Format == mdfString)
        tmpL = MUTimeFromString((char *)Value);
      else
        tmpL = *(long *)Value;

      C->F.Overrun = tmpL;
      }  /* END BLOCK */
     
    default:

      /* NO-OP */

      return(FAILURE);
   
      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MClassSetAttr() */




int MClassAToString(

  mclass_t *C,      /* I */
  int       AIndex, /* I */
  char     *Buf,    /* O */ 
  int       Format, /* I */
  int       Mode)   /* I */

  {
  if ((C == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  Buf[0] = '\0';

  switch(AIndex)
    {
    case mclaOCNode:

      if (C->OCNodeName != NULL)
        strcpy(Buf,C->OCNodeName);

      break;

    case mclaOCDProcFactor:

      if (C->OCDProcFactor > 0.0)
        sprintf(Buf,"%0.2lf",C->OCDProcFactor);

      break;

    case mclaDefReqFeature:

      strcpy(Buf,MUMAList(eFeature,C->DefFBM,sizeof(C->DefFBM)));

      break;

    case mclaWCOverrun:

      if (C->F.Overrun != 0)
        strcpy(Buf,MULToTString(C->F.Overrun));

      break;

    default:

      /* NO-OP */

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MClassAToString() */




int MClassConfigShow(

  mclass_t *CS,
  int       VFlag,
  char     *Buffer)

  {
  int cindex;
  int aindex;

  mclass_t *C;

  char tmpVal[MAX_MLINE];
  char tmpLine[MAX_MLINE];

  if (Buffer == NULL)
    {
    return(FAILURE);
    }

  for (cindex = 0;cindex < MAX_MCLASS;cindex++)
    {
    C = &MClass[cindex];

    if ((CS != NULL) && (CS != C))
      continue;

    tmpLine[0] = '\0';

    if (C->Name[0] == '\1')
      continue;

    if (C->Name[0] == '\0')
      break;

    sprintf(tmpLine,"CLASSCFG[%s]",
      C->Name);

    for (aindex = 0;MClassAttr[aindex] != NULL;aindex++)
      {
      tmpVal[0] = '\0';

      MClassAToString(C,aindex,tmpVal,mfmHuman,0);

      if (tmpVal[0] != '\0')
        {
        sprintf(tmpLine,"%s %s=%s",
          tmpLine,
          MClassAttr[aindex],
          tmpVal);
        }
      }    /* END for (aindex) */

    strcat(Buffer,tmpLine);
    strcat(Buffer,"\n");
    }  /* END for (cindex) */

  return(SUCCESS);
  }  /* END MClassConfigShow() */




int MClassLoadCP(

  mclass_t *CS,
  char     *Buf)

  {
  char    tmpHeader[MAX_MNAME];
  char    CName[MAX_MNAME];

  char   *ptr;

  mclass_t *C;

  long    CkTime;

  mxml_t *E = NULL;

  const char *FName = "MClassLoadCP";

  DBG(4,fCKPT) DPrint("%s(CS,%s)\n",
    FName,
    (Buf != NULL) ? Buf : "NULL");

  if (Buf == NULL)
    {
    return(FAILURE);
    }

  /* FORMAT:  <HEADER> <GID> <CKTIME> <GSTRING> */

  /* load CP header */

  sscanf(Buf,"%s %s %ld",
    tmpHeader,
    CName,
    &CkTime);

  if (((long)MSched.Time - CkTime) > MCP.CPExpirationTime)
    {
    return(SUCCESS);
    }

  if (CS == NULL)
    {
    if (MClassAdd(CName,&C) != SUCCESS)
      {
      DBG(5,fCKPT) DPrint("ALERT:    cannot load CP class '%s'\n",
        CName);

      return(FAILURE);
      }
    }
  else
    {
    C = CS;
    }

  if ((ptr = strchr(Buf,'<')) == NULL)
    {
    return(FAILURE);
    }

  MXMLCreateE(&E,(char *)MXO[mxoClass]);

  MXMLFromString(&E,ptr,NULL,NULL);

  MOFromXML((void *)C,mxoClass,E);

  MXMLDestroyE(&E);

  return(SUCCESS);
  }  /* END MClassLoadCP() */



/* END MClass.c */

