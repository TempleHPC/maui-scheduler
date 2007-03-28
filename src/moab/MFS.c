/* HEADER */

#include "moab.h"
#include "msched-proto.h"  
 
extern mlog_t      mlog;
 
extern const char *MJobFlags[];
extern const char *MXO[];
extern const char *MXOC[];
extern const char *MFSAttr[];
extern const char *MFSPolicyType[];
 
extern msched_t    MSched;
extern mgcred_t    *MUser[];
extern mgcred_t    MGroup[];
extern mgcred_t     MAcct[];
extern mstat_t     MStat;
extern mattrlist_t MAList;
extern mqos_t      MQOS[];
extern mpar_t      MPar[];
 


int MFSToXML(

  mfs_t    *F,       /* I */
  mxml_t **EP,      /* O */
  int      *SAList)  /* I (optional) */
 
  {
  int DAList[] = {
    mfsaTarget, 
    -1 };

  int  aindex;
 
  int *AList;
 
  char tmpString[MAX_MLINE];
 
  if ((F == NULL) || (EP == NULL))
    {
    return(FAILURE);
    }
 
  if (SAList != NULL)
    AList = SAList;
  else
    AList = DAList;
 
  MXMLCreateE(EP,(char *)MXO[mxoFS]);
 
  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    if ((MFSAToString(F,AList[aindex],tmpString,0) == FAILURE) ||
        (tmpString[0] == '\0'))
      {
      continue;
      }
 
    MXMLSetAttr(*EP,(char *)MFSAttr[AList[aindex]],tmpString,mdfString);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MFSToXML() */




int MFSFromXML(
 
  mfs_t   *F,  /* I (modified) */
  mxml_t *E)  /* I */
 
  {
  int aindex;
  int saindex;
 
  if ((F == NULL) || (E == NULL))
    {
    return(FAILURE);
    }
 
  /* NOTE:  do not initialize.  may be overlaying data */
 
  for (aindex = 0;aindex < E->ACount;aindex++)
    {
    saindex = MUGetIndex(E->AName[aindex],MFSAttr,FALSE,0);
 
    if (saindex == 0)
      continue;
 
    MFSSetAttr(F,saindex,(void **)E->AVal[aindex],mdfString,mSet);
    }  /* END for (aindex) */
 
  return(SUCCESS);
  }  /* END MFSFromXML() */
 
 
 
 
int MFSSetAttr(
 
  mfs_t   *F,
  int      AIndex,
  void   **Value,
  int      Format,
  int      Mode)
 
  { 
  if (F == NULL)
    {
    return(FAILURE);
    }

  switch(AIndex)
    {
    case mfsaTarget:

      MFSTargetFromString(F,(char *)Value);
 
      break;
 
    default:

      /* not handled */

      return(FAILURE);
   
      /*NOTREACHED*/
 
      break;
    }  /* switch(AIndex) */
 
  return(SUCCESS);
  }  /* MFSSetAttr() */




int MFSTargetFromString(

  mfs_t *F,   /* I */
  char  *Buf) /* O */

  {
  char *tail;

  if ((F == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  F->FSTarget = strtod(Buf,&tail);
 
  if (tail != NULL)
    {
    if (*tail == '+')
      F->FSMode = mfstFloor;
    else if (*tail == '-')
      F->FSMode = mfstCeiling;
    else if (*tail == '^')
      F->FSMode = mfstCapAbs;
    else if (*tail == '%')
      F->FSMode = mfstCapRel;
    else
      F->FSMode = mfstTarget;
    }
  else
    {
    F->FSMode = mfstTarget;
    }

  return(SUCCESS);
  }  /* END MFSTargetFromString() */





int MFSAToString(
 
  mfs_t *F,      /* I */
  int    AIndex, /* I */
  char  *Buf,    /* O */
  int    Format) /* I */
 
  {
  if ((F == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }
 
  switch(AIndex)
    {
    case mfsaTarget:
 
      strcpy(Buf,MFSTargetToString(F->FSTarget,F->FSMode));

      break;

    default:

      /* not supported */

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MFSAToString() */




int MFSCheckCap(

  mfs_t   *FSPtr,
  mjob_t  *J,
  mpar_t  *P,
  int     *OIPtr)   /* O (optional) */

  {
  int OList[] = { mxoUser, mxoGroup, mxoAcct, mxoClass, mxoQOS, -1 };
  int oindex;

  double FSUsage;
  double FSReq;

  int    MaxIndex;

  mfs_t *F;

  mfsc_t *FC = &MPar[0].FSC;

  double tmpD;

  mpar_t *GP = &MPar[0];

  switch(FC->FSPolicy)
    {
    case fspDPES:

      MJobGetPE(J,P,&tmpD);
 
      FSReq = (long)tmpD;
 
      break;
 
    case fspUPS:
    case fspDPS:
    default:
 
      FSReq = MJobGetProcCount(J) * J->SpecWCLimit[0];
 
      break;
    }  /* END switch(FC->FSPolicy) */

  if (FSPtr == NULL)
    {
    MaxIndex   = -1;

    if ((J == NULL) || (P == NULL))
      {
      return(FAILURE);
      }

    for (oindex = 0;OList[oindex] != -1;oindex++)
      {
      F = NULL;
 
      switch (OList[oindex])
        {
        case mxoUser:
 
          if (J->Cred.U != NULL)
            F = &J->Cred.U->F;
 
            break;
 
        case mxoGroup:
 
          if (J->Cred.G != NULL)
            F = &J->Cred.G->F;
 
          break;
 
        case mxoAcct:

          if (J->Cred.A != NULL)
            F = &J->Cred.A->F;
 
          break;
 
        case mxoClass:
 
          if (J->Cred.C != NULL)
            F = &J->Cred.C->F;
 
          break;
 
        case mxoQOS:
 
          if (J->Cred.Q != NULL)
            F = &J->Cred.Q->F;
 
          break;

        default:

          /* NO-OP */
 
          break;
        }  /* END switch (OList[oindex]) */
 
      if (F == NULL)
        {
        /* no fairshare object defined for cred */

        continue;
        }

      FSUsage = 0.0;

      switch(F->FSMode)
        {
        case mfstCapAbs:
 
          FSUsage = F->FSUsage[0] + F->FSFactor + FSReq;
 
          break;
 
        case mfstCapRel:
 
          if (GP->F.FSUsage[0] + GP->F.FSFactor > 0)
            {
            FSUsage = (F->FSUsage[0] + F->FSFactor + FSReq) /
                      (GP->F.FSUsage[0] + GP->F.FSFactor) * 100.0;
            }
 
          break;
 
        default:

          /* NO-OP */
 
          break;
        }  /* END switch(F->FSMode) */
 
      if (FSUsage > F->FSTarget)
        {
        if (OIPtr != NULL)
          *OIPtr = MaxIndex;

        return(FAILURE);
        }
      }    /* END for (oindex) */
    }      /* END if (FSPtr == NULL) */
  else
    {
    F = FSPtr;

    FSUsage = 0.0;

    switch(F->FSMode)
      {
      case mfstCapAbs:
 
        FSUsage = F->FSUsage[0] + F->FSFactor + FSReq;
 
        break;
 
      case mfstCapRel:
 
        if (GP->F.FSUsage[0] + GP->F.FSFactor > 0)
          {
          FSUsage = (F->FSUsage[0] + F->FSFactor + FSReq) /
                    (GP->F.FSUsage[0] + GP->F.FSFactor) * 100.0;
          }
 
        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch(F->FSMode) */

    if (FSUsage > F->FSTarget)
      {
      return(FAILURE);
      }
    }    /* END else (FSPtr == NULL) */

  return(SUCCESS);
  }  /* END MFSCheckCap() */




int MFSShutdown(

  mfsc_t *F)  /* I */

  {
  long NewFSInterval;

  if ((F == NULL) || (F->FSPolicy == fspNONE))
    {
    return(SUCCESS);
    }

  /* if FS interval reached */
 
  NewFSInterval = MSched.Time - (MSched.Time % F->FSInterval);
 
  if (NewFSInterval != MSched.CurrentFSInterval)
    {
    if (MSched.CurrentFSInterval != 0)
      MFSUpdateData(F,MSched.CurrentFSInterval,(1 << mfsactRotate));
 
    MSched.CurrentFSInterval = NewFSInterval;
 
    DBG(1,fFS) DPrint("INFO:     FS interval rolled to interval %ld\n",
      MSched.CurrentFSInterval);
    }

  return(SUCCESS);
  }  /* END MFSShutdown() */




int MFSSetDefaults(
 
  mfs_t *F,    /* I (modified) */
  int    Mode) /* I */
 
  {
  if (F == NULL)
    {
    return(FAILURE);
    }
 
  if ((Mode == FALSE) && (F->IsInitialized == TRUE))
    {
    return(SUCCESS);
    }
 
  /* set default values */
 
  F->FSMode           = mfstTarget;
  F->FSTarget         = 0.0;
 
  F->QDef             = &MQOS[0];
 
  MUBMClear(F->QAL,MAX_MQOS);
 
  MUBMSet(((mqos_t *)F->QDef)->Index,F->QAL);
 
  F->PDef             = NULL;

  MUBMClear(F->PAL,MAX_MPAR);

  F->JobFlags         = MDEF_FSFLAGS;
 
  F->Priority         = 0;
 
  F->IsInitialized = TRUE;
 
  return(SUCCESS);
  }  /* END MFSSetDefaults() */




char *MFSTargetToString(
 
  double FSTarget,  /* I */
  int    FSMode)    /* I */
 
  {
  const char *MFSTType = "  +-%^";

  static char Line[MAX_MLINE];
 
  sprintf(Line,"%.2lf%c",
    FSTarget,
    MFSTType[FSMode]);
 
  return(Line);
  }  /* END MFSTargetToString() */




int MFSLoadDataFile(
 
  char *FileName,
  int   FSSlot)
 
  {
  int      count;
 
  char    *buf;
  char     Name[MAX_MNAME];
  char     Type[MAX_MNAME];
 
  double   Value;
  char    *ptr;
  char    *tmp;
 
  char    *head;
  char    *TokPtr;
 
  int      buflen;
 
  int      SC;

  void    *O;

  mfs_t   *F;

  int      oindex;

  int      rc;

  const char *FName = "MFSLoadDataFile";
 
  DBG(2,fFS) DPrint("%s(%s,%d)\n", 
    FName,
    FileName,
    FSSlot);
 
  if ((buf = MFULoad(FileName,1,macmWrite,&count,&SC)) == NULL)
    {
    DBG(1,fFS) DPrint("WARNING:  cannot open FS data file '%s'\n",
      FileName);
 
    return(FAILURE);
    }
 
  buflen = strlen(buf);
  head   = buf;
 
  ptr = MUStrTok(buf,"\n",&TokPtr);
 
  while ((ptr = MUStrTok(head,"\n",&TokPtr)) != NULL)
    {
    head = ptr + strlen(ptr) + 1;
 
    if ((head - buf) >= buflen)
      {
      /* point head to end of buffer */
 
      head = &buf[buflen];
      }
 
    /* ignore comments */
 
    if ((tmp = strchr(ptr,'#')) != NULL)
      *tmp = '\0';
 
    DBG(6,fFS) DPrint("INFO:     parsing FS data line '%s'\n",
      ptr); 
 
    /* load value */
 
    rc = sscanf(ptr,"%32s %32s %lf",
      Type,
      Name,
      &Value);

    if (rc != 3)
      {
      /* ignore line */

      continue;
      }

    if (!strcmp(Type,"TOTAL"))
      {
      oindex = mxoSched;
      }
    else if ((oindex = MUGetIndex(Type,MXO,FALSE,mxoNONE)) == mxoNONE)
      {
      continue;
      }

    if ((MOGetObject(oindex,Name,&O,mAdd) == FAILURE) ||
        (MOGetComponent(O,oindex,(void *)&F,mxoFS) == FAILURE))
      {
      continue;
      }

    /* update value */

    DBG(6,fFS) DPrint("INFO:     %s '%s' FSUsage[%d]: %lf\n",
      MXO[oindex],
      (Name[0] != '\0') ? Name : "NONE",
      FSSlot,
      Value);
 
    F->FSUsage[FSSlot] = Value;
    }  /* END  while ((ptr = MUStrTok(head,"\n",&TokPtr)) != NULL) */
 
  free(buf);
 
  return(SUCCESS);
  }  /* END MFSLoadDataFile() */





int MFSUpdateData(

  mfsc_t *FC,
  int     FSInterval,  /* IN:  FS data interval */
  int     FA)          /* IN:  FS action */

  {
  mpar_t   *GP;

  int      fsindex;

  char     FSFile[MAX_MLINE];

  char    *NameP;

  void    *O;
  void    *OP;

  FILE    *fsfp = NULL;

  const int OList[] = { mxoUser, mxoGroup, mxoAcct, mxoQOS, mxoClass, mxoSched, -1 };
 
  int oindex;

  mfs_t *F;
 
  const char *FName = "MFSUpdateData";

  DBG(2,fFS) DPrint("%s(FC,%d,%x)\n",
    FName,
    FSInterval,
    FA);

  DBG(6,fFS) DPrint("INFO:     mode: %s:%s:%s\n",
    (FA & (1 << mfsactCalc))   ? "calc"  : "",
    (FA & (1 << mfsactRotate)) ? "rotate"  : "",
    (FA & (1 << mfsactWrite))  ? "write" : "");

  GP = &MPar[0];

  if (FA & ((1 << mfsactWrite) | (1 << mfsactRotate)))
    {
    /* open FS data file */

    if (MStat.StatDir[strlen(MStat.StatDir) - 1] == '/')
      {
      sprintf(FSFile,"%sFS.%d",
        MStat.StatDir,
        FSInterval);
      }
    else
      {
      sprintf(FSFile,"%s/FS.%d",
        MStat.StatDir,
        FSInterval);
      }

    if ((fsfp = fopen(FSFile,"w+")) == NULL)
      {
      DBG(0,fFS) DPrint("WARNING:  cannot open FS data file '%s', errno: %d (%s)\n",
        FSFile,
        errno,
        strerror(errno));

      return(FAILURE);
      }
    
    /* write FS file header */

    fprintf(fsfp,"# FS Data File (Duration: %6ld seconds)  Starting: %s\n",
      FC->FSInterval,
      MULToDString(&MSched.Time));
    }  /* END if (FA & ((1 << mfsactWrite) | (1 << mfsactRotate))) */

  for (oindex = 0;OList[oindex] != -1;oindex++)
    {
    int   OS;
    void *OE;

    /* step through all objects */

    MOINITLOOP(&OP,OList[oindex],&OS,&OE);

    DBG(4,fFS) DPrint("INFO:     updating %s fairshare\n",
      MXO[OList[oindex]]);    

    while ((O = MOGetNextObject(&OP,OList[oindex],OS,OE,&NameP)) != NULL)
      {
      if (MOGetComponent(O,OList[oindex],(void *)&F,mxoFS) == FAILURE)
        continue;

      DBG(7,fFS) DPrint("INFO:     updating %s %s\n",
        MXO[OList[oindex]],
        (NameP != NULL) ? NameP : "NONE");

      if ((FA & ((1 << mfsactWrite) | (1 << mfsactRotate))) && (F->FSUsage[0] != 0.0))  
        {
        fprintf(fsfp,"%-10s %15s %12.3f\n",
          MXO[OList[oindex]],
          (NameP != NULL) ? NameP : "NONE",
          F->FSUsage[0]);
        }

      if (FA & (1 << mfsactRotate))
        {
        for (fsindex = 1;fsindex < MAX_MFSDEPTH;fsindex++)
          F->FSUsage[MAX_MFSDEPTH - fsindex] = F->FSUsage[MAX_MFSDEPTH - fsindex - 1];

        F->FSUsage[0] = 0.0;
        }

      if (FA & ((1 << mfsactCalc) | (1 << mfsactRotate)))
        {
        F->FSFactor = MFSCalcFactor(FC,F->FSUsage);
        }
      }    /* END while ((O = MOGetNextObj()) != NULL) */
    }      /* END for (oindex) */

  if (fsfp != NULL)
    fclose(fsfp);

  return(SUCCESS);
  }  /* END MFSUpdateData() */





int MFSInitialize(

  mfsc_t *FC)

  {
  int  StartFSInterval;
  int  CurrentFSInterval;
  char FileName[MAX_MLINE];
  
  int  interval;

  const char *FName = "MFSInitialize";

  DBG(3,fFS) DPrint("%s()\n",
    FName);

  if ((FC == NULL) || (FC->FSPolicy == fspNONE))
    {
    return(SUCCESS);
    }

  CurrentFSInterval = MSched.Time - (MSched.Time % FC->FSInterval);
  StartFSInterval   = CurrentFSInterval - FC->FSDepth * FC->FSInterval;

  for (interval = 0;interval <= MAX_MFSDEPTH;interval++)
    {
    if (interval > FC->FSDepth)
      break;

    if (MStat.StatDir[strlen(MStat.StatDir) - 1] == '/')
      {
      sprintf(FileName,"%sFS.%ld",
        MStat.StatDir,
        StartFSInterval + interval * FC->FSInterval);
      }
    else
      {
      sprintf(FileName,"%s/FS.%ld",
        MStat.StatDir,
        StartFSInterval + interval * FC->FSInterval);
      }
 
    if (MFSLoadDataFile(FileName,FC->FSDepth - interval) == FAILURE)
      {
      DBG(3,fFS) DPrint("WARNING:  cannot load FS file '%s' for slot %d\n",
        FileName,
        FC->FSDepth - interval);
      }
    }    /* END for (interval) */

  MFSUpdateData(FC,0,(1 << mfsactCalc));

  return(SUCCESS);
  }  /* END MFSInitialize() */





double MFSCalcFactor(

  mfsc_t *F,
  double  FSUsage[MAX_MFSDEPTH])  /* IN:  CPU usage history */

  {
  int    cindex;

  static double fsfactor;

  const char *FName = "MFSCalcFactor";

  DBG(7,fFS) DPrint("%s(F,FSUsage)\n",
    FName);

  if ((F == NULL) || (FSUsage == NULL))
    {
    return(FAILURE);
    }

  fsfactor = 0.0;

  for (cindex = 1;cindex < F->FSDepth;cindex++)
    {
    fsfactor += (FSUsage[cindex] * pow(F->FSDecay,cindex));

    DBG(7,fFS) DPrint("INFO:  FSUsage[%d]  %0.2lf\n",
      cindex,
      FSUsage[cindex]);
    }  /* END for (cindex) */

  DBG(7,fFS) DPrint("INFO:  FSFactor: %0.2lf\n",
    fsfactor);

  return(fsfactor);
  }  /* END MFSCalcFactor() */




int MFSProcessOConfig(
 
  mfsc_t *F,       /* I (modified) */
  int     PIndex,  /* I */
  int     IVal,    /* I */
  double  DVal,    /* I */
  char   *SVal,    /* I */
  char  **SArray)  /* I */
 
  {
  char *ptr;

  if (F == NULL)
    {
    return(FAILURE);
    }
 
  switch (PIndex)
    {
    case pFSDecay:
 
      F->FSDecay = DVal;
 
      break;

    case pFSDepth:

      F->FSDepth = MIN(IVal,MAX_MFSDEPTH);
 
      DBG(1,fCONFIG) DPrint("INFO:     %s set to %d\n",
        MParam[PIndex],
        F->FSDepth);
 
      break;

    case pFSInterval:
 
      F->FSInterval = MUTimeFromString(SVal);
 
      break;

    case pFSPolicy:

      if ((ptr = strchr(SVal,'%')) != NULL)
        {
        MSched.PercentBasedFS = TRUE;

        /* patch submitted by Ake Sandgren--assumes that % is always at the end of SVal */

        *ptr = '\0';
        }

      if (MUBoolFromString(SVal,FALSE) == TRUE)
        {
        /* enable backlevel support */ 

        F->FSPolicy = fspDPS;   
        }
      else
        { 
        F->FSPolicy = MUGetIndex(SVal,MFSPolicyType,FALSE,F->FSPolicy);
        }

      break;

    case pServWeight:
    case pTargWeight:
    case pCredWeight:
    case pFSWeight:
    case pResWeight:
    case pUsageWeight:
 
      {
      long  tmpL;
      char *tail;
 
      tmpL = strtol(SVal,&tail,0);
 
      if (*tail == '%')
        {
        F->PCW[PIndex - pServWeight + 1] = 10;
        F->PCP[PIndex - pServWeight + 1] = tmpL;
        }
      else
        {
        F->PCW[PIndex - pServWeight + 1] = tmpL;
        }
      }
 
      break;

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

      F->PSW[PIndex - pSQTWeight + 1] = (long)IVal;

      break;
 
    case pServCap:
    case pTargCap:
    case pCredCap:
    case pFSCap:
    case pResCap:
    case pUsageCap: 
 
      F->PCC[PIndex - pServCap + 1] = (long)IVal;
 
      break;
 
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
 
      F->PSC[PIndex - pSQTCap + 1] = (long)IVal;
 
      break;
 
    case pXFMinWCLimit:
 
      F->XFMinWCLimit = MUTimeFromString(SVal); 
 
      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch (PIndex) */

  return(SUCCESS);
  }  /* END MFSProcessOConfig() */




int MFSShow(

  char *Buf,      /* O */
  int   BufSize,  /* I */
  int   Mode)     /* I */

  {
  char *BPtr;
  int   BSpace;

  int   fsindex;

  const int OList[] = { mxoUser, mxoGroup, mxoAcct, mxoQOS, mxoClass, -1 };

  int   oindex;

  mfs_t  *F;
  mfs_t  *DF;

  int     OS;
  void   *OE;
  void   *O;
  void   *OP;

  char   *NameP;

  double  FSTarget;
  int     FSMode;
  double  FSPercent;

  mpar_t *GP = NULL;

  char    tmpString[MAX_MNAME];
  char    tmpName[MAX_MNAME];

  const char *FName = "MFSShow";

  DBG(3,fFS) DPrint("%s(Buf,%d,%d)\n",
    FName,
    BufSize,
    Mode);

  if (Buf == NULL)
    {
    return(FAILURE);
    }

  GP = &MPar[0];

  BPtr   = Buf;
  BSpace = BufSize;

  BPtr[0] = '\0';

  /* display global config */

  MUSNPrintF(&BPtr,&BSpace,"FairShare Information\n\n");

  MUSNPrintF(&BPtr,&BSpace,"Depth: %d intervals   Interval Length: %s   Decay Rate: %.2f\n\n",
    GP->FSC.FSDepth,
    MULToTString(GP->FSC.FSInterval),
    GP->FSC.FSDecay);

  MUSNPrintF(&BPtr,&BSpace,"FS Policy: %s\n",
    MFSPolicyType[GP->FSC.FSPolicy]);

  MUSNPrintF(&BPtr,&BSpace,"System FS Settings:  Target Usage: %s   Flags: %ld\n\n",
    MFSTargetToString(GP->F.FSTarget,GP->F.FSMode),
    GP->F.JobFlags);

  /* display header */

  MUSNPrintF(&BPtr,&BSpace,"%-14s %7s %7s",
    "FSInterval",
    "  %   ",
    "Target");

  for (fsindex = 0;fsindex < GP->FSC.FSDepth;fsindex++)
    {
    if (GP->F.FSUsage[fsindex] > 0.0)
      {
      MUSNPrintF(&BPtr,&BSpace," %7d",
        fsindex);
      }
    }    /* END for (fsindex) */

  MUSNPrintF(&BPtr,&BSpace,"\n");

  /* display weight line */

  MUSNPrintF(&BPtr,&BSpace,"%-14s %7s %7s",
    "FSWeight",
    "-------",
    "-------");

  for (fsindex = 0;fsindex < GP->FSC.FSDepth;fsindex++)
    {
    if (GP->F.FSUsage[fsindex] > 0.0)
      {
      MUSNPrintF(&BPtr,&BSpace," %7.4f",
        pow(GP->FSC.FSDecay,fsindex));
      }
    }  /* END for (fsindex) */

  MUSNPrintF(&BPtr,&BSpace,"\n");

  /* display total usage line */

  MUSNPrintF(&BPtr,&BSpace,"%-14s %7.2f %7s",
    "TotalUsage",
    100.0,
    "-------");

  for (fsindex = 0;fsindex < GP->FSC.FSDepth;fsindex++)
    {
    if (GP->F.FSUsage[fsindex] > 0.0)
      {
      MUSNPrintF(&BPtr,&BSpace," %7.1f",
        GP->F.FSUsage[fsindex] / 3600.0);
      }
    }    /* END for (fsindex) */

  MUSNPrintF(&BPtr,&BSpace,"\n");

  DBG(6,fUI) DPrint("INFO:     Total FSFactor: %8.2f  FSUsage[0]: %8.2f\n",
    GP->F.FSFactor,
    GP->F.FSUsage[0]);

  /* display all credentials */

  for (oindex = 0;OList[oindex] != -1;oindex++)
    {
    int HeaderDisplayed = FALSE;

    MOINITLOOP(&OP,OList[oindex],&OS,&OE);

    DBG(4,fFS) DPrint("INFO:     updating %s fairshare\n",
      MXO[OList[oindex]]);

    while ((O = MOGetNextObject(&OP,OList[oindex],OS,OE,&NameP)) != NULL)
      {
      if (MOGetComponent(O,OList[oindex],(void *)&F,mxoFS) == FAILURE)
        continue;

      DF = NULL;

      switch(OList[oindex])
        {
        case mxoUser:

          if (MSched.DefaultU != NULL)
            DF = &MSched.DefaultU->F;

          break;

        case mxoGroup:

          if (MSched.DefaultG != NULL)
            DF = &MSched.DefaultG->F;

          break;

        case mxoAcct:

          if (MSched.DefaultA != NULL)
            DF = &MSched.DefaultA->F;

          break;

        case mxoClass:

          if (MSched.DefaultC != NULL)
            DF = &MSched.DefaultC->F;

          break;

        case mxoQOS:

          if (MSched.DefaultQ != NULL)
            DF = &MSched.DefaultQ->F;

          break;

        default:

          /* not supported */

          break;
        }  /* END switch(OList[oindex]) */
        
      if ((NameP == NULL) || 
          (NameP[0] == '\0') || 
           !strcmp(NameP,ALL))
        {
        /* invalid name specified */

        continue;
        }

      if (F->FSTarget > 0.0)
        {
        FSTarget = F->FSTarget;
        FSMode   = F->FSMode;
        }
      else if (DF != NULL)
        {
        FSTarget = DF->FSTarget;
        FSMode   = DF->FSMode;
        }
      else
        {
        FSTarget = 0.0;
        FSMode   = mfstNONE;
        }

      if ((FSTarget == 0.0) &&
          (F->FSFactor == 0.0) &&
          (F->FSUsage[0] == 0.0) &&
         !(Mode & (1 << mcmVerbose)))
        {
        /* no information to report */
 
        continue;
        }

      if (HeaderDisplayed == FALSE)
        {
        MUSNPrintF(&BPtr,&BSpace,"\n%s\n-------------\n",
          MXOC[OList[oindex]]);

        HeaderDisplayed = TRUE;
        }

      strcpy(tmpName,NameP);

      if ((GP->F.FSFactor + GP->F.FSUsage[0]) > 0.0)
        {
        FSPercent = (F->FSFactor + F->FSUsage[0]) / (GP->F.FSFactor + GP->F.FSUsage[0]) * 100.0;

        if ((((FSMode == mfstFloor)   && (FSPercent < FSTarget)) ||
             ((FSMode == mfstCeiling) && (FSPercent > FSTarget)) ||
             ((FSMode == mfstTarget)  && (fabs(FSPercent - FSTarget) > 5.0))) &&
              (FSTarget > 0.0))
          {
          strcat(tmpName,"*");
          }
        }
      else
        {
        FSPercent = 0.0;
        }

      if (FSTarget > 0.0)
        {
        sprintf(tmpString,"%7.7s",
          MFSTargetToString(FSTarget,FSMode));
        }
      else
        {
        strcpy(tmpString,"-------");
        }

      MUSNPrintF(&BPtr,&BSpace,"%-14s %7.2f %7s",
        tmpName,
        FSPercent,
        tmpString);

      DBG(6,fUI) DPrint("INFO:     %s '%s'  FSFactor: %8.2f  FSUsage[0]: %8.2f  FSPercent: %8.2f\n",
        MXO[OList[oindex]],
        NameP,
        F->FSFactor,
        F->FSUsage[0],
        FSPercent);

      for (fsindex = 0;fsindex < GP->FSC.FSDepth;fsindex++)
        {
        if (GP->F.FSUsage[fsindex] > 0.0)
          {
          if (F->FSUsage[fsindex] > 0.0)
            {
            MUSNPrintF(&BPtr,&BSpace," %7.2f",
              F->FSUsage[fsindex] / GP->F.FSUsage[fsindex] * 100.0);
            }
          else
            {
            MUSNPrintF(&BPtr,&BSpace," -------");
            }
          }
        }    /* END for (fsindex) */

      MUSNPrintF(&BPtr,&BSpace,"\n");
      }  /* END while ((O = MOGetNextObject()) != NULL) */
    }    /* END for (oindex) */
  
  return(SUCCESS);
  }  /* END MFSShow() */



/* END MFS.c */

