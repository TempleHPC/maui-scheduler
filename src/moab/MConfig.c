/* HEADER */
        
/* Contains:                                              *
 * int MCfgAdjustBuffer(Buf,AllowExtension)               *
 * int MCfgGetVal(Buf,Parm,Index,Value,SysTable)          *
 * int MCfgGetIVal(Buf,CurPtr,Parm,IName,Index,Value,SymTable) *
 *                                                        */


#include "moab.h"
#include "msched-proto.h"

extern mlog_t  mlog;

extern msched_t    MSched;
extern mpar_t      MPar[];
extern mcfg_t      MCfg[];
extern mrm_t       MRM[];
extern mam_t       MAM[];
extern msim_t      MSim;




int MCfgAdjustBuffer(

  char     **Buf,            /* I (modified) */
  mbool_t    AllowExtension) /* I */

  {
  char *ptr;

  int   State;

  char  IFile[MAX_MLINE];

  /* change all parameters to upper case */
  /* replace all comments with spaces    */
  /* replace tabs with space             */
  /* replace '"' with space              */
  /* replace unprint chars with space    */
  /* extend '\' lines                    */

  enum { cbPreParm = 0, cbOnParm, cbPreVal, cbOnVal, cbComment };

  const char *FName = "MCfgAdjustBuffer";
 
  DBG(3,fCONFIG) DPrint("%s(Buf)\n",
    FName);

  if ((Buf == NULL) || (*Buf == NULL) || (strlen(*Buf) < 1))
    {
    return(SUCCESS);
    }
 
  ptr = *Buf;

  State = cbPreParm;

  IFile[0] = '\0';

  while (*ptr != '\0')
    {
    /* remove comments */

    if (*ptr == '#')
      {
      State = cbComment;
      }  /* END if (*ptr == '#') */
    else if ((*ptr == '\\') && (State != cbComment))
      {
      *ptr = ' ';

      while(isspace(*ptr))
        {
        *ptr = ' ';

        ptr++;
        }
      }
    else if (*ptr == '\n')
      {
      if ((State == cbComment) && (IFile[0] != '\0'))
        {
        char *IBuf;

        int   blen;

        int   offset;

        /* include file at end */

        /* load file */

        if ((IBuf = MFULoad(IFile,1,macmWrite,NULL,NULL)) == NULL)
          {
          /* cannot load include file */
          }
        else
          {
          blen = strlen(*Buf);

          offset = ptr - *Buf;

          if ((*Buf = (char *)realloc(*Buf,blen + 2 + strlen(IBuf))) == NULL)
            {
            /* NOTE:  memory failure */

            MUFree(&IBuf);

            return(FAILURE);
            }

          /* append file */

	  strcat(*Buf,"\n");
	  strcat(*Buf,IBuf); 

          MUFree(&IBuf);

          ptr = *Buf + offset;
          }

        IFile[0] = '\0';
        }  /* END if ((State == cbComment) && (IFile[0] != '\0')) */

      State = cbPreParm;
      }
    else if ((State == cbComment) || (*ptr == '\\'))
      {
      *ptr = ' ';
      }
    else if (isspace(*ptr) || (*ptr == '='))
      {
      if (*ptr == '=')
        {
        if (State != cbOnVal)
          *ptr = ' ';
        }
      else
        { 
        if (State == cbOnParm)
          State = cbPreVal;

        *ptr = ' ';
        }
      }
    else if (isprint(*ptr))
      {
      if (State == cbPreParm)
        {
        State = cbOnParm;

        /*
        if (isalpha(*ptr))
          *ptr = toupper(*ptr);
        */
        }
      else if (State == cbPreVal)
        {
        State = cbOnVal;
        }
      } 
    else
      {
      *ptr = ' ';
      }

    ptr++;
    }  /* END while (ptr != '\0') */ 

  DBG(5,fCONFIG) DPrint("INFO:     adjusted config Buffer ------\n%s\n------\n",
    *Buf);

  return(SUCCESS);
  }  /* END MCfgAdjustBuffer() */






int MCfgGetVal(

  char  **Buf,
  const char *Parm,
  char   *IName,
  int    *Index,
  char   *Value,
  int     ValSize,
  char  **SymTable)
   
  {
  char *ptr;
  char *tmp;
  char *head;
  char *tail;

  char  IndexName[MAX_MNAME];

  int   iindex;

  const char *FName = "MCfgGetVal";

  DBG(7,fCONFIG) DPrint("%s(Buf,%s,%s,Index,Value,%d,SymTable)\n",
    FName,
    Parm,
    (IName != NULL) ? IName : "NULL",
    ValSize);

  if (Parm == NULL)
    return(FAILURE);

  IndexName[0] = '\0';

  /* FORMAT:  { '\0' || '\n' }[<WS>]<VAR>[<[><WS><INDEX><WS><]>]<WS><VALUE> */

  ptr = *Buf;

  while (ptr != NULL)
    {
    if ((head = strstr(ptr,Parm)) == NULL)
      break;

    ptr = head + strlen(Parm);

    /* look backwards for newline or start of buffer */

    if (head > *Buf)
      tmp = head - 1;
    else
      tmp = *Buf;

    while ((tmp > *Buf) && ((*tmp == ' ') || (*tmp == '\t')))
      tmp--;

    if ((tmp != *Buf) && (*tmp != '\n'))
      continue;

    if ((IName != NULL) && (IName[0] != '\0'))
      {
      /* requested index name specified */

      if (*ptr != '[')
        continue;

      ptr++;

      while (isspace(*ptr))
        ptr++;

      if (strncmp(IName,ptr,strlen(IName)) != 0)
        continue;

      ptr += strlen(IName);

      while (isspace(*ptr))
        ptr++;
       
      if (*ptr != ']')
        continue; 

      ptr++;

      /* requested index found */
      }
    else if (isspace(*ptr))
      {
      /* no index specified */

      if (Index != NULL)
        *Index = 0;
      }
    else
      {
      /* index specified, no specific index requested */

      if (*ptr != '[')
        continue;

      ptr++;

      while (isspace(*ptr))
        ptr++;

      head = ptr;

      while ((!isspace(*ptr)) && (*ptr != ']'))
        ptr++;

      MUStrCpy(IndexName,head,MIN(ptr - head + 1,MAX_MNAME));

      while (isspace(*ptr))
        ptr++;

      if (*ptr != ']')
        continue;

      ptr++;

      if (Index != NULL)
        {
        *Index = (int)strtol(IndexName,&tail,10);

        if (*tail != '\0')
          {
          /* index is symbolic */
  
          if (SymTable == NULL)
            return(FAILURE);

          for (iindex = 0;SymTable[iindex] != NULL;iindex++)
            {
            if (!strcmp(SymTable[iindex],IndexName))
              {
              *Index = iindex;
              break;
              }
            }

          if (SymTable[iindex] == NULL)
            {
            MUStrDup(&SymTable[iindex],IndexName);

            *Index = iindex;
            }
          }
        }
      }    /* END else ... */
   
    while ((*ptr == ' ') || (*ptr == '\t'))
      ptr++;

    if ((tail = strchr(ptr,'\n')) == NULL)
      tail = *Buf + strlen(*Buf);
  
    MUStrCpy(Value,ptr,MIN(tail - ptr + 1,ValSize));

    if ((IName != NULL) && (IName[0] == '\0'))
      MUStrCpy(IName,IndexName,MAX_MNAME);
 
    Value[tail - ptr] = '\0';

    *Buf = tail;

    return(SUCCESS);
    }  /* END while(ptr != NULL) */

  Value[0] = '\0';

  return(FAILURE);
  }  /* END MCfgGetVal() */




 
int MCfgGetIVal(

  char  *Buf,
  char **CurPtr,
  const char *Parm,
  char  *IndexName,
  int   *Index,
  int   *Value,
  char **SymTable)

  {
  char  ValLine[MAX_MLINE];
  char *ptr;

  int   rc;

  const char *FName = "MCfgGetIVal";

  DBG(7,fCONFIG) DPrint("%s(Buf,CurPtr,%s,%s,Index,Value,SymTable)\n",
    FName,
    Parm,
    (IndexName != NULL) ? IndexName : "NULL");

  ptr = Buf;

  if (CurPtr != NULL)
    ptr = MAX(ptr,*CurPtr);

  rc = MCfgGetVal(&ptr,Parm,IndexName,Index,ValLine,sizeof(ValLine),SymTable);

  if (CurPtr != NULL)
    *CurPtr = ptr;

  if (rc == FAILURE)
    return(FAILURE);

  *Value = (int)strtol(ValLine,NULL,0);

  DBG(4,fCONFIG) DPrint("INFO:     %s[%d] set to %d\n",
    Parm,
    (Index != NULL) ? *Index : 0,
    *Value);

  return(SUCCESS);
  }  /* END MCfgGetIVal() */





int MCfgGetDVal(

  char    *Buf,
  char   **CurPtr,
  const char *Parm,
  char    *IndexName,
  int     *Index,
  double  *Value,
  char   **SymTable)

  {
  char  ValLine[MAX_MLINE];
  char *ptr;

  int   rc;

  const char *FName = "MCfgGetDVal";

  DBG(7,fCONFIG) DPrint("%s(Buf,CurPtr,%s,%s,Index,Value,SymTable)\n",
    FName,
    Parm,
    (IndexName != NULL) ? IndexName : "NULL");

  if ((Buf == NULL) || (Value == NULL))
    return(FAILURE);

  ptr = Buf;

  if (CurPtr != NULL)
    ptr = MAX(ptr,*CurPtr);

  rc = MCfgGetVal(&ptr,Parm,IndexName,Index,ValLine,sizeof(ValLine),SymTable);

  if (CurPtr != NULL)
    *CurPtr = ptr;

  if (rc == FAILURE)
    return(FAILURE);

  *Value = strtod(ValLine,NULL);

  DBG(4,fCONFIG) DPrint("INFO:     %s[%d] set to %lf\n",
    Parm,
    (Index != NULL) ? *Index : 0,
    *Value);

  return(SUCCESS);
  }  /* END MCfgGetDVal() */





int MCfgGetSVal(

  char   *Buf,
  char  **CurPtr,
  const char *Parm,
  char   *IndexName,
  int    *Index,
  char   *Value,
  int     ValSize,
  int     Mode,
  char  **SymTable)

  {
  char *ptr;

  int   rc;
  int   index;

  const char *FName = "MCfgGetSVal";

  DBG(7,fCONFIG) DPrint("%s(Buf,CurPtr,%s,%s,Index,Value,SymTable)\n",
    FName,
    Parm,
    (IndexName != NULL) ? IndexName : "NULL");

  if (Value != NULL)
    Value[0] = '\0';

  if (Parm == NULL)
    {
    return(FAILURE);
    }

  ptr = Buf;

  if (CurPtr != NULL)
    ptr = MAX(ptr,*CurPtr);

  rc = MCfgGetVal(&ptr,Parm,IndexName,Index,Value,ValSize,SymTable);

  if (CurPtr != NULL)
    *CurPtr = ptr;

  if (rc == FAILURE)
    {
    return(FAILURE);
    }

  if (Mode == 1)
    {
    /* process only first white space delimited string */

    for (index = 0;Value[index] != '\0';index++)
      {
      if (isspace(Value[index]))
        {
        Value[index] = '\0';

        break;
        }
      }
    }
  else
    {
    /* remove trailing whitespace */
 
    for (index = strlen(Value) - 1;index > 0;index--)
      {
      if (isspace(Value[index]))
        Value[index] = '\0';
      else
        break;
      }  /* END for (index) */
    }

  DBG(4,fCONFIG) DPrint("INFO:     %s[%d] set to %s\n",
    Parm,
    (Index != NULL) ? *Index : 0,
    Value);

  return(SUCCESS);
  }  /* END MCfgGetSVal() */






int MCfgGetSList(

  char  *Buf,
  char **CurPtr,
  const char *Parm,
  char  *IndexName,
  int   *Index,
  int    Length,
  char  *Value,
  char  *SymTable[])

  {
  int   index;
  char  ValLine[MAX_MLINE];

  char *ptr;
  char *TokPtr;

  int   rc;

  const char *FName = "MCfgGetSList";

  DBG(7,fCONFIG) DPrint("%s(Buf,CurPtr,%s,%s,Index,%d,Value,SymTable)\n",
    FName,
    Parm,
    IndexName,
    Length);

  ptr = Buf;

  if (CurPtr != NULL)
    ptr = MAX(ptr,*CurPtr);

  rc = MCfgGetVal(&ptr,Parm,IndexName,Index,ValLine,sizeof(ValLine),SymTable);

  if (rc == FAILURE)
    {
    return(FAILURE);
    }

  index = 0;

  ptr = MUStrTok(ValLine," :;",&TokPtr);

  while (ptr != NULL)
    {
    strncpy(&Value[index * Length],ptr,Length);
    Value[((index + 1) * Length) - 1] = '\0';

    index++;

    ptr = MUStrTok(NULL," :;",&TokPtr);
    }

  Value[index * Length] = '\0';

  for (index = 0;Value[index * Length] != '\0';index++)
    {
    DBG(4,fCONFIG) DPrint("INFO:     %s[%d][%d] set to '%s'\n",
      Parm,
      *Index,
      index,
      &Value[index * Length]);
    }

  return(SUCCESS);
  }  /* END MCfgGetSList() */




int MCfgEnforceConstraints()

  {
  int        pindex;

  int        OList[] = { mxoUser, mxoGroup, mxoAcct, -1 };

  int        oindex;

  mpu_t     *AP;
  mpu_t     *IP;
  mpu_t     *JP;

  int        PtIndex = 0;

  const char *FName = "MCfgEnforceConstraints";

  DBG(4,fCONFIG) DPrint("%s()\n",
    FName);

  if (MSched.ServerHost[0] == '\0')
    {
    DBG(0,fCONFIG) DPrint("ERROR:    parameter '%s' must be specified\n",
      MParam[pServerHost]);

    fprintf(stderr,"ERROR:    parameter '%s' must be specified\n",
      MParam[pServerHost]);

    exit(1);
    }

  for (oindex = 0;OList[oindex] != -1;oindex++)
    {
    AP = NULL;
    IP = NULL;

    switch (OList[oindex])
      {
      case mxoUser:

        if (MSched.DefaultU != NULL)
          {
          AP = &MSched.DefaultU->L.AP;
          IP = MSched.DefaultU->L.IP;
          }

        break;

      case mxoGroup:

        if (MSched.DefaultG != NULL)
          {
          AP = &MSched.DefaultG->L.AP;
          IP = MSched.DefaultG->L.IP;
          }

        break;

      case mxoAcct:

        if (MSched.DefaultA != NULL)
          {
          AP = &MSched.DefaultA->L.AP;
          IP = MSched.DefaultA->L.IP;
          }

        break;

      default:

        /* NOT HANDLED */

        break;
      }  /* END switch(OList[oindex]) */

    if (AP == NULL)
      continue;

    for (pindex = 0;pindex < MAX_MPOLICY;pindex++)
      {
      if (AP != NULL)
        {
        if (AP->SLimit[pindex][PtIndex] == 0)
          {
          AP->SLimit[pindex][PtIndex] = AP->HLimit[pindex][PtIndex];
          }
        else if ((AP->SLimit[pindex][PtIndex] > 0)  &&
                ((AP->HLimit[pindex][PtIndex] < AP->SLimit[pindex][PtIndex]) ||
                 (AP->HLimit[pindex][PtIndex] <= 0)))
          {
          AP->HLimit[pindex][PtIndex] = AP->SLimit[pindex][PtIndex];
          }
        }

      if (IP != NULL)
        {
        if (IP->SLimit[pindex][PtIndex] == 0)
          {
          IP->SLimit[pindex][PtIndex] = IP->HLimit[pindex][PtIndex];
          }
        else if ((IP->SLimit[pindex][PtIndex] > 0)  &&
                ((IP->HLimit[pindex][PtIndex] < IP->SLimit[pindex][PtIndex]) ||
                 (IP->HLimit[pindex][PtIndex] <= 0)))
          {
          IP->HLimit[pindex][PtIndex] = IP->SLimit[pindex][PtIndex];
          }
        }
      }    /* END for (pindex) */
    }      /* END for (oindex) */

  PtIndex = 0;

  JP      = MPar[PtIndex].L.JP;

  for (pindex = 0;pindex < MAX_MPOLICY;pindex++)
    {
    if (JP != NULL)
      {
      if (JP->SLimit[pindex][PtIndex] == 0)
        {
        JP->SLimit[pindex][PtIndex] = JP->HLimit[pindex][PtIndex];
        }
      else if ((JP->SLimit[pindex][PtIndex] > 0)  &&
              ((JP->HLimit[pindex][PtIndex] < JP->SLimit[pindex][PtIndex]) ||
               (JP->HLimit[pindex][PtIndex] <= 0)))
        {
        JP->HLimit[pindex][PtIndex] = JP->SLimit[pindex][PtIndex];
        }
      }
    }    /* END for (pindex) */

  return(SUCCESS);
  }  /* END MCfgEnforceConstraints() */




int MCfgProcessLine(

  int   CIndex,    /* I */
  char *IndexName, /* I (optional) */
  char *Line,      /* I */
  char *Msg)       /* O (optional) */

  {
  int    IVal;
  double DVal;
  char  *SVal;
  char  *SArray[MAX_MINDEX];
  char  *NullString = "";

  char  *ptr;

  int    MIndex;

  char *TokPtr;

  int   PIndex;

  mpar_t      *P  = NULL;
  mam_t       *A  = NULL;
  mrm_t       *R  = NULL;
  sres_t      *SR = NULL;
  mqos_t      *Q  = NULL;

  const char *FName = "MCfgProcessLine";

  PIndex = MCfg[CIndex].PIndex;

  DBG(3,fCONFIG) DPrint("%s(%s,%s,%s)\n",
    FName,
    MParam[PIndex],
    (IndexName != NULL) ? IndexName : "NULL",
    Line);

  switch(MCfg[CIndex].OType)
    {
    case mxoSRes:

      /* verify SR exists */

      /* NOTE: no default SR */

      if (MSRAdd(IndexName,&SR) == FAILURE)
        {
        DBG(2,fCONFIG) DPrint("ALERT:    cannot configure SR[%s]\n",
          IndexName);

        return(FAILURE);
        }

      break;

    case mxoPar:

      if (IndexName[0] == '\0')
        {
        /* default index selects global partition */

        P = &MPar[0];
        }
      else if (MParAdd(IndexName,&P) == FAILURE)
        {
        DBG(2,fCONFIG) DPrint("ALERT:    cannot configure MPar[%s]\n",
          IndexName);

        return(FAILURE);
        }

      break;

    case mxoQOS:

      /* NOTE: no default QOS */

      if (MQOSAdd(IndexName,&Q) == FAILURE)
        {
        DBG(2,fCONFIG) DPrint("ALERT:    cannot configure MQOS[%s]\n",
          IndexName);

        return(FAILURE);
        }

      break;

    case mxoAM:

      if (MAM[0].Name[0] == '\0')
        {
        /* NOTE:  base config only enables primary AM */

        MAMAdd("base",&A);
        }

      break;

    case mxoRM:

      if (IndexName[0] == '\0')
        {
        /* default index selects base RM */

        R = &MRM[0];
        }
      else if (MRMAdd(IndexName,&R) == FAILURE)
        {
        DBG(2,fCONFIG) DPrint("ALERT:    cannot configure MRM[%s]\n",
          IndexName);

        return(FAILURE);
        }
      else
        {
        MRMSetDefaults(R);

        MOLoadPvtConfig((void **)R,mxoRM,NULL,NULL,NULL);
        }

      break;
    }  /* END switch(MCfg[CIndex].OType) */

  /* initialize values */

  IVal   = 0;
  DVal   = 0.0;
  SVal   = NULL;
  MIndex = 0;
  SArray[0] = NULL;

  /* read config values */

  switch(MCfg[CIndex].Format)
    {
    case mdfStringArray:

      /* process string array */

      /* check for first string value */

      if ((SArray[MIndex] = MUStrTok(Line," \t\n",&TokPtr)) != NULL)
        {
        DBG(7,fCONFIG) DPrint("INFO:     adding value[%d] '%s' to string array\n",
          MIndex,
          SArray[MIndex]);

        MIndex++;

        /* load remaining string values */

        while ((SArray[MIndex] = MUStrTok(NULL," \t\n",&TokPtr)) != NULL)
          {
          DBG(7,fCONFIG) DPrint("INFO:     adding value[%d] '%s' to string array\n",
            MIndex,
            SArray[MIndex]);

          MIndex++;
          }
        }
      else
        {
        /* no values located */

        DBG(1,fCONFIG) DPrint("WARNING:  parameter '%s[%s]' is not assigned a value.  using default value\n",
          MParam[PIndex],
          IndexName);
        }

      break;

    case mdfIntArray:

      /* extract integer array */

      /* NYI */

      break;

    case mdfInt:

      DBG(7,fCONFIG) DPrint("INFO:     parsing values for integer array parameter '%s'\n",
        MParam[PIndex]);

      if ((ptr = MUStrTok(Line," \t\n",&TokPtr)) == NULL)
        {
        DBG(1,fCONFIG) DPrint("WARNING:  parameter '%s' is not assigned a value (using default value)\n",
          MParam[PIndex]);

        IVal = -1;
        }
      else
        {
        IVal = (int)strtol(ptr,NULL,0);

        if ((IVal == 0) && (ptr[0] != '0'))
          {
          DBG(1,fCONFIG) DPrint("WARNING:  Parameter[%02d] '%s' value (%s) cannot be read as an integer  (using default value)\n",
            PIndex,
            MParam[PIndex],
            ptr);

          IVal = -1;
          }
        }

      break;

    case mdfDouble:

      /* extract double array */

      if ((ptr = MUStrTok(Line," \t\n",&TokPtr)) == NULL)
        {
        DBG(1,fCONFIG) DPrint("WARNING:  parameter '%s' is not assigned a value  (using default value)\n",
          MParam[PIndex]);

        DVal = -1.0;
        }
      else
        {
        DVal = strtod(ptr,NULL);

        if ((DVal == 0.0) && (ptr[0] != '0'))
          {
          DBG(1,fCONFIG) DPrint("WARNING:  Parameter[%02d] '%s' value (%s) cannot be read as a double  (using default value)\n",
            PIndex,
            MParam[PIndex],
            ptr);

          DVal = -1.0;
          }
        }

      break;

    case mdfDoubleArray:

      /* NYI */

      break;

    case mdfString:

      /* process string value */

      DBG(7,fCONFIG) DPrint("INFO:     parsing value for single string parameter '%s'\n",
        MParam[PIndex]);

      if ((SVal = MUStrTok(Line," \t\n",&TokPtr)) == NULL)
        {
        DBG(1,fCONFIG) DPrint("WARNING:  NULL value specified for parameter[%d] '%s'\n",
          PIndex,
          MParam[PIndex]);

        SVal = NullString;
        }

      break;

    default:

      DBG(0,fCONFIG) DPrint("ERROR:    parameter[%d] '%s' not handled\n",
        PIndex,
        MParam[PIndex]);

      break;
    }  /* END switch (PIndex) */

  if (((MCfg[CIndex].Format == mdfInt)         && (IVal == -1)) ||
      ((MCfg[CIndex].Format == mdfIntArray)    && (IVal == -1)) ||
      ((MCfg[CIndex].Format == mdfDouble)      && (DVal == -1.0)) ||
      ((MCfg[CIndex].Format == mdfDoubleArray) && (DVal == -1.0)) ||
      ((MCfg[CIndex].Format == mdfString)      && (SVal == NULL)) ||
      ((MCfg[CIndex].Format == mdfStringArray) && (SArray[0] == NULL)) ||
      ((MCfg[CIndex].Format == mdfStringArray) && (SArray[0] == NULL)))
    {
    /* invalid parameter specified */

    DBG(0,fCONFIG) DPrint("ALERT:    parameter '%s' has invalid value\n",
      MCfg[CIndex].Name);

    return(SUCCESS);
    }

  /* assign values to parameters */

  switch(MCfg[CIndex].OType)
    {
    case mxoSRes:

      MSRProcessOConfig(SR,PIndex,IVal,DVal,SVal,SArray);

      break;

    case mxoAM:

      MAMProcessOConfig(A,PIndex,IVal,DVal,SVal,SArray);

      break;

    case mxoRM:

      MRMProcessOConfig(R,PIndex,IVal,DVal,SVal,SArray);

      break;

    case mxoQOS:

      MQOSProcessOConfig(Q,PIndex,IVal,DVal,SVal,SArray);

      break;

    case mxoSim:

      MSimProcessOConfig(&MSim,PIndex,IVal,DVal,SVal,SArray);

      break;

    case mxoPar:
    case mxoSched:
    default:
   
      MCfgSetVal(PIndex,IVal,DVal,SVal,SArray,P,IndexName);

      break;
    }  /* END switch(MCfg[CIndex].Type) */

  /* log parameter setting */

  switch(MCfg[CIndex].Format)
    {
    case mdfInt:

      DBG(4,fCONFIG) DPrint("INFO:     parameter '%s' assigned int value %d\n",
        MParam[PIndex],
        IVal);

      break;

    case mdfDouble:

      DBG(4,fCONFIG) DPrint("INFO:     parameter '%s' assigned int value %lf\n",
        MParam[PIndex],
        DVal);

      break;

    case mdfString:

      DBG(4,fCONFIG) DPrint("INFO:     parameter '%s' assigned string value '%s'\n",
        MParam[PIndex],
        SVal);

      break;

    case mdfStringArray:

      {
      char tmpLine[MAX_MLINE];

      int  index;

      tmpLine[0] = '\0';

      for (index = 0;index < MIndex;index++)
        {
        MUStrCat(tmpLine,SArray[index],sizeof(tmpLine));
        MUStrCat(tmpLine," ",sizeof(tmpLine));
        }

      DBG(4,fCONFIG) DPrint("INFO:     parameter '%s' assigned string array '%s'\n",
        MParam[PIndex],
        tmpLine);
      }  /* END BLOCK */

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(MCfg[CIndex].Format) */

  return(SUCCESS);
  }  /* END MCfgProcessLine() */


/* END MConfig.c */

/* HEADER */

/* Contains:                                          *
 *                                                    */
 
 
 
 
#include "moab.h"
#include "msched-proto.h"
 
extern mlog_t     mlog;
 
extern msched_t   MSched;
extern msim_t     MSim;
extern mqos_t     MQOS[];
extern mstat_t    MStat;
extern mprofcfg_t Plot;
extern mcfg_t     MCfg[];
 
extern mframe_t   MFrame[];
extern mpar_t     MPar[];
extern mrm_t      MRM[];
extern mam_t      MAM[];
extern mattrlist_t MAList;
extern sres_t     SRes[];
extern mckpt_t    MCP;
 
extern msys_t     MSys;
 
extern const char *ResThresholdType[];
extern const char *MLogFacilityType[];
extern const char *MCDisplayType[]; 
 
extern const char *NodeAllocationPolicy[];
extern const char *TaskDistributionPolicy[];
extern const char *MPolicyMode[];
extern const char *QPolicy[];
extern const char *NCPolicy[];
extern const char *MBFMPolicy[];
extern const char *JSPolicy[];
extern const char *ResPolicy[];
extern const char *ResCtlPolicy[];
extern const char *MPreemptPolicy[];
extern const char *SchedType[];
extern const char *ComType[];
extern const char *MAMType[];
extern const char *MAMProtocol[];
extern const char *MAMChargePolicy[];
extern const char *MResourceType[];
extern const char *MRMSubType[];
extern const char *RMAuthType[];
extern const char *MNAllocPolicy[];
extern const char *MResFlags[];
 
extern const char *MWeekDay[];
 
extern const char *MQOSFlags[];
extern const char *MSimFlagType[];
extern const char *MJobFlags[];
 
extern const char *MJobNodeMatchType[];
extern const char *MFSPolicyType[];
extern const char *MSRPeriodType[];
extern const char *MSockProtocol[];




/* NOTE:  only handles old-style parameters */

int MCfgProcessBuffer(
 
  char *Buffer)  /* I */
 
  {
  char  *ptr;
  char  *ptr2;
 
  char  *tail;
  char   tmp[MAX_MLINE + 1];
  char   Line[MAX_MLINE + 1];
 
  int    cindex;
  int    CIndex;
 
  int    index;
 
  int    IsValid;
 
  char   IndexName[MAX_MNAME];

  const char *FName = "MCfgProcessBuffer";
 
  DBG(5,fCONFIG) DPrint("%s(%s)\n",
    FName,
    Buffer);
 
  if ((Buffer == NULL) || (Buffer[0] == '\0'))
    {
    return(FAILURE);
    }
 
  /* look for all defined parameters in buffer */
 
  for (cindex = 0;MCfg[cindex].Name != NULL;cindex++)
    {
    DBG(5,fCONFIG) DPrint("INFO:     checking parameter '%s'\n",
      MCfg[cindex].Name);
 
    MUStrCpy(tmp,(char *)MCfg[cindex].Name,sizeof(tmp)); 
 
    ptr = Buffer;
 
    IndexName[0] = '\0';
 
    while ((ptr = strstr(ptr,tmp)) != NULL)
      {
      IsValid = FALSE;
 
      DBG(7,fCONFIG) DPrint("INFO:     checking parameter '%s' (loop)\n",
        MCfg[cindex].Name);
 
      /* verify ptr at start of line */
 
      if ((ptr == Buffer) || (*(ptr - 1) == '\n'))
        {
        ptr2 = &ptr[strlen(tmp)];
 
        switch(*ptr2)
          {
          case '[':
 
            /* determine array parameter */
 
            for (index = 1;index < MAX_MNAME;index++)
              {
              if (isspace(ptr2[index]) || (ptr2[index] == ']'))
                break;
 
              IndexName[index - 1] = ptr2[index];
              }
 
            IndexName[index - 1] = '\0';
 
            DBG(3,fCONFIG) DPrint("INFO:     detected array index '%s'\n",
              IndexName);
 
            IsValid = TRUE;
 
            break; 
 
          case ' ':
          case '\t':
          case '\n':
          case '\0':
 
            IsValid = TRUE;
 
            break;
 
          default:

            /* NO-OP */
 
            break;
          }    /* END switch(ptr[strlen(tmp)])              */
        }      /* if ((ptr == Buffer) || (*(ptr - 1) == '\n')) */
 
      if (IsValid == TRUE)
        {
        DBG(4,fCONFIG) DPrint("INFO:     located parameter '%s'\n",
          MCfg[cindex].Name);
 
        /* move to end of keyword */
 
        while(!isspace(*ptr) && (*ptr != '\0') && (*ptr != '\n'))
          {
          ptr++;
          }
 
        /* remove white space */
 
        while(isspace(*ptr) && (*ptr != '\0') && (*ptr != '\n'))
          {
          ptr++;
          }
 
        if ((tail = strchr(ptr,'\n')) == NULL)
          {
          DBG(3,fCONFIG) DPrint("ALERT:    missing newline termination character\n");
 
          tail = ptr + MAX_MLINE; 
          }
 
        MUStrCpy(Line,ptr,MIN(tail - ptr + 1,sizeof(Line)));

        /* preserve cindex as loop variable */

        CIndex = cindex;
 
        MCfgTranslateBackLevel(&CIndex);
 
        DBG(7,fCONFIG) DPrint("INFO:     value for parameter '%s': '%s'\n",
          MCfg[CIndex].Name,
          Line);
 
        MCfgProcessLine(CIndex,IndexName,Line,NULL);
        }  /* END if (IsValid == TRUE) */
 
      ptr++;
      }  /* END while ((ptr = strstr(ptr,tmp)) != NULL) */
    }    /* for (cindex = 0;...)                        */
 
  MCfgEnforceConstraints();
 
  return(SUCCESS);
  }  /* END MCfgProcessBuffer() */




int MCfgTranslateBackLevel(
 
  int *CIndex)  /* I */
 
  {
  if (CIndex == NULL)
    {
    return(FAILURE);
    }
 
  switch(MCfg[*CIndex].PIndex)
    {
    case pOLDUFSWeight:
    case pOLDFSUWeight:

    
      MCfgGetIndex(pFUWeight,CIndex);
 
      break;
 
    case pOLDGFSWeight:
    case pOLDFSGWeight:

      MCfgGetIndex(pFGWeight,CIndex);     
 
      break;
 
    case pOLDAFSWeight:
    case pOLDFSAWeight:

      MCfgGetIndex(pFAWeight,CIndex);    
 
      break;
 
    case pOLDFSQWeight:

      MCfgGetIndex(pFQWeight,CIndex);      
 
      break;
 
    case pOLDFSCWeight: 

      MCfgGetIndex(pFCWeight,CIndex);       
 
      break;
 
    case pOLDServWeight:

      MCfgGetIndex(pServWeight,CIndex);      
 
      break;
 
    case pOLDDirectSpecWeight:

      MCfgGetIndex(pCredWeight,CIndex);      
 
      break;
 
    case pOLDBankServer:

      MCfgGetIndex(pAMHost,CIndex);      
 
      break;
 
    case pOLDRMServer:

      MCfgGetIndex(pRMHost,CIndex);       
 
      break;

    case pBFType:

      MCfgGetIndex(pBFPolicy,CIndex);      

      break;
 
    default:

      /* NO-OP */
 
      break;
    }  /* END switch(*CIndex) */
 
  return(SUCCESS);
  }  /* END MCfgTranslateBackLevel() */




int MCfgGetIndex(

  int  PIndex, /* I */
  int *CIndex) /* O */

  { 
  int cindex;

  if (CIndex == NULL)
    {
    return(FAILURE);
    }

  for (cindex = 0;cindex < MAX_MCFG;cindex++)
    {
    if (MCfg[cindex].PIndex == PIndex)
      {
      *CIndex = cindex;

      return(SUCCESS);
      }
    }    /* END for (cindex) */
 
  return(FAILURE);
  }  /* END MCfgGetIndex() */




int MCfgSetVal(

  int      PIndex,    /* I */
  int      IVal,
  double   DVal,
  char    *SVal,      /* I */
  char   **SArray,    /* I */
  mpar_t  *P,
  char    *IndexName)
  
  {
  int    MIndex;

  int      val;
  double   valf;
  char    *valp;
  char   **valpa;

  mfsc_t  *F  = NULL;

  const char *FName = "MCfgSetVal";

  DBG(3,fCONFIG) DPrint("%s(%s,IVal,DVal,SVal,SArray,P)\n",
    FName,
    MParam[PIndex]);

  if (P != NULL)
    F = &P->FSC;

  val    = IVal;
  valf   = DVal;
  valp   = SVal;
  valpa  = SArray;

  if (SArray != NULL)
    {
    for (MIndex = 0;SArray[MIndex] != NULL;MIndex++);
    }
  else
    {
    MIndex = 0;
    }

  /* assign values to parameters */        

  switch (PIndex)
    {
    case pResDepth:
    case pParIgnQList:
    case pJobAggregationTime:
    case pLogLevel:
    case pRMPollInterval:
    case pMinDispatchTime:
    case pNodePollFrequency:
    case pMaxSleepIteration:
    case mcoJobFBAction:
    case mcoMailAction:
    case pAdminEAction:
    case pAdminEInterval:
    case pCheckPointFile:
    case pCheckPointInterval:
    case pCheckPointExpirationTime:
    case pDefaultDomain:
    case pDefaultClassList:
    case pServerName:
    case pLogFacility:
    case pLogFileMaxSize:
    case pLogFileRollDepth:
    case pMonitoredJob:
    case pMonitoredNode:
    case pMonitoredRes:
    case pMonitoredFunction:
    case pProcSpeedFeatureHeader:
    case pNodeTypeFeatureHeader:
    case pPartitionFeatureHeader:
    case pNAMaxPS:
    case pNAPolicy:
    case pSchedMode:
    case pClientTimeout:
    case pMCSocketProtocol:
    case pServerHost:
    case pServerPort:
    case pResCtlPolicy:
    case pPreemptPolicy:
    case pDisplayFlags:
    case mcoUseSyslog:
    case mcoDeferTime:
    case pDeferCount:
    case pDeferStartCount:
    case pJobPurgeTime:
    case pNodePurgeTime:
    case pAPIFailureThreshhold:
    case pNodeSyncDeadline:
    case pJobSyncDeadline:
    case pJobMaxOverrun:
    case pMaxJobPerUserPolicy:
    case pMaxJobPerUserCount:
    case pHMaxJobPerUserCount:
    case pMaxNodePerUserPolicy:
    case pMaxNodePerUserCount:
    case pHMaxNodePerUserCount:
    case pMaxProcPerUserPolicy:
    case pMaxProcPerUserCount:
    case pHMaxProcPerUserCount:
    case pMaxPSPerUserPolicy:
    case pMaxPSPerUserCount:
    case pHMaxPSPerUserCount:
    case pMaxJobQueuedPerUserPolicy:
    case pMaxJobQueuedPerUserCount:
    case pHMaxJobQueuedPerUserCount:
    case pMaxJobPerGroupPolicy:
    case pMaxJobPerGroupCount:
    case pHMaxJobPerGroupCount:
    case pMaxNodePerGroupPolicy:
    case pMaxNodePerGroupCount:
    case pHMaxNodePerGroupCount:
    case pMaxPSPerGroupPolicy:
    case pMaxPSPerGroupCount:
    case pHMaxPSPerGroupCount:
    case pMaxJobQueuedPerGroupPolicy:
    case pMaxJobQueuedPerGroupCount:
    case pHMaxJobQueuedPerGroupCount:
    case pMaxJobPerAccountPolicy:
    case pMaxJobPerAccountCount:
    case pHMaxJobPerAccountCount:
    case pMaxNodePerAccountPolicy:
    case pMaxNodePerAccountCount:
    case pHMaxNodePerAccountCount:
    case pMaxPSPerAccountPolicy:
    case pMaxPSPerAccountCount:
    case pHMaxPSPerAccountCount:
    case pMaxJobQueuedPerAccountPolicy:
    case pMaxJobQueuedPerAccountCount:
    case pHMaxJobQueuedPerAccountCount:
    case mcoAllocLocalityPolicy:
    case pMServerHomeDir:
    case pNodeMaxLoad:
    case pNodeCPUOverCommitFactor:
    case pNodeMemOverCommitFactor:
    case pMaxJobPerIteration:
    case mcoDirectoryServer:
    case mcoEventServer:
    case mcoTimePolicy:
    case pSchedLogDir:
    case pSchedLogFile:
    case pSchedStepCount:
    case mcoUseJobRegEx:
    case mcoAdminUsers:
    case mcoAdmin1Users:
    case mcoAdmin2Users:
    case mcoAdmin3Users:
    case mcoAdmin4Users:
    case mcoAdminHosts:
    case mcoComputeHosts:
    case pReservationDepth:
    case pResQOSList:
    case pSchedToolsDir:
    case pSchedLockFile:
    case pStatDir:
    case pPlotMinTime:
    case pPlotMaxTime:
    case pPlotTimeScale:
    case pPlotMinNode:
    case pPlotMaxNode:
    case pPlotNodeScale:
    case pNodeUntrackedProcFactor:

      MSchedProcessOConfig(&MSched,PIndex,val,valf,valp,valpa,IndexName);
      
      break;

    case pNodeAvailPolicy:
    case mcoResourceLimitPolicy:
    case pUseSystemQueueTime:
    case pJobPrioAccrualPolicy:
    case mcoBFChunkDuration:
    case mcoBFChunkSize:
    case pBFPriorityPolicy:
    case pNodeLoadPolicy:
    case pNodeSetPolicy:
    case pNodeSetAttribute:
    case pNodeSetDelay:
    case pNodeSetPriorityType:
    case pNodeSetList:
    case pNodeSetTolerance:
    case pBFPolicy:
    case pBFDepth:
    case pBFProcFactor:
    case pBFMaxSchedules:
    case pJobSizePolicy:
    case pJobNodeMatch:
    case pUseMachineSpeed:
    case pUseMachineSpeedForFS:
    case pNodeAllocationPolicy:
    case pBFMetric:
    case mcoAdminMinSTime:
    case mcoRejectNegPrioJobs:
    case mcoEnableMultiNodeJobs:
    case mcoEnableMultiReqJobs:
    case mcoEnableNegJobPriority:
    case pMaxJobStartTime:
    case pTaskDistributionPolicy:
    case pResPolicy:
    case pResRetryTime:
    case pResThresholdType:
    case pResThresholdValue:
    case pMaxMetaTasks:
    case pSystemMaxJobProc:
    case pSystemMaxJobTime:
    case pSystemMaxJobPS:
    case pIgnPbsGroupList:
    case pFSSecondaryGroups:

      MParProcessOConfig(P,PIndex,val,valf,valp,valpa);
 
      break;

    case pServWeight:
    case pTargWeight:
    case pCredWeight:
    case pFSWeight:
    case pResWeight:
    case pUsageWeight:
    case pSQTWeight:
    case pSXFWeight:
    case pSSPVWeight:
    case pSBPWeight:
    case pTQTWeight:
    case pTXFWeight:
    case pCUWeight:
    case pCGWeight:
    case pCAWeight:
    case pCQWeight:
    case pCCWeight:
    case pFUWeight:
    case pFGWeight:
    case pFAWeight:
    case pFQWeight:
    case pFCWeight:
    case pRNodeWeight:
    case pRProcWeight:
    case pRMemWeight:
    case pRSwapWeight:
    case pRDiskWeight:
    case pRPSWeight:
    case pRPEWeight:
    case pRUProcWeight:
    case pRUJobWeight:
    case pRWallTimeWeight:
    case pUConsWeight:
    case pURemWeight:
    case pUPerCWeight:
    case pUExeTimeWeight:
    case pServCap:
    case pTargCap:
    case pCredCap:
    case pFSCap:
    case pResCap:
    case pUsageCap:
    case pSQTCap:
    case pSXFCap:
    case pSSPVCap:
    case pSBPCap:
    case pTQTCap:
    case pTXFCap:
    case pCUCap:
    case pCGCap:
    case pCACap:
    case pCQCap:
    case pCCCap:
    case pFUCap:
    case pFGCap:
    case pFACap:
    case pFQCap:
    case pFCCap:
    case pRNodeCap:
    case pRProcCap:
    case pRMemCap:
    case pRSwapCap:
    case pRDiskCap:
    case pRPSCap:
    case pRPECap:
    case pRWallTimeCap:
    case pUConsCap:
    case pURemCap:
    case pUPerCCap:
    case pUExeTimeCap:
    case pXFMinWCLimit:
    case pFSPolicy:
    case pFSInterval:
    case pFSDepth:
    case pFSDecay:
 
      MFSProcessOConfig(F,PIndex,val,valf,valp,valpa);
 
      break;

    default:

      DBG(1,fCONFIG) DPrint("ERROR:    unexpected parameter[%d] '%s' detected\n",
        PIndex,
        MParam[PIndex]);

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch (PIndex) */

  return(SUCCESS);
  }  /* END MCfgSetVal() */


/* END MConfig.c */

