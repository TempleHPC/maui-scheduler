/* HEADER */

/* Contains:                   *
 *                             */


#include "moab.h"
#include "msched-proto.h"  
 
extern mlog_t       mlog;
extern mnode_t     *MNode[];
extern mjob_t      *MJob[];
extern mqos_t       MQOS[];
extern msched_t     MSched;
extern mframe_t     MFrame[];
extern mpar_t       MPar[];
extern mgcred_t     MClass[];
extern mgcred_t    *MUser[];
extern mgcred_t     MGroup[];
extern mgcred_t     MAcct[];
extern msys_t       MSys;
extern mckpt_t      MCP;
extern mrm_t        MRM[];
extern mstat_t      MStat;
extern mattrlist_t  MAList;
extern m64_t        M64;
 
extern const char *MQALType[];
extern const char *MResourceType[];
extern const char *MNodeState[];
extern const char *MNodeAttr[];
extern const char *MResSetSelectionType[];
extern const char *MResSetAttrType[];
extern const char *MResSetPrioType[];
extern const char *MBFPriorityPolicyType[];
extern const char *MJobPrioAccrualPolicyType[];
extern const char *MNAvailPolicy[];
extern const char *MResourceLimitPolicyType[];
extern const char *MNodeLoadPolicyType[];
extern const char *MPolicyAction[];
extern const char *MBFPolicy[];
extern const char *MJSPolicy[];
extern const char *MJobNodeMatchType[];
extern const char *MBFMPolicy[];
extern const char *MNAllocPolicy[];
extern const char *MResPolicy[];
extern const char *MResThresholdType[];
extern const char *MTaskDistributionPolicy[];
extern const char *MPreemptPolicy[];
extern const char *MFSPolicyType[];




int MJobGetPAL(

  mjob_t   *J,     /* I:  job                                */
  int      *RPAL,  /* I:  requested partition access list    */
  int      *PAL,   /* O:  approved job partition access list */
  mpar_t  **PDef)  /* O:  default partition for job          */

  {
  int tmpPAL[(MAX_MPAR >> 5) + 1];
 
  int  AndMask;
 
  int  pindex;
  int  cindex;
 
  mclass_t *C;

  const char *FName = "MJobGetPAL";
 
  DBG(7,fSTRUCT) DPrint("%s(%s,%s,PAL,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (RPAL != NULL) ? "RPAL" : "NULL",
    (PDef != NULL) ? "PDef" : "NULL");
 
  if ((J == NULL) || (J->Cred.U == NULL) || (J->Cred.G == NULL))
    {
    return(FAILURE);
    }

  C = J->Cred.C;
 
  if (C == NULL)
    {
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
 
  /* determine PAL */

  MUBMCopy(tmpPAL,MPar[0].F.PAL,MAX_MPAR);
 
  if (MPar[0].F.PALType == qalAND)
    {
    AndMask = TRUE;
    }
  else
    {
    /* obtain 'or' list */
 
    if (J->Cred.U->F.PALType == qalOR)
      MUBMOR(tmpPAL,J->Cred.U->F.PAL,MAX_MPAR);
 
    if (J->Cred.G->F.PALType == qalOR)
      MUBMOR(tmpPAL,J->Cred.G->F.PAL,MAX_MPAR);
 
    if ((J->Cred.A != NULL) && (J->Cred.A->F.PALType == qalOR))
      MUBMOR(tmpPAL,J->Cred.A->F.PAL,MAX_MPAR);
 
    if ((C != NULL) && (C->F.PALType == qalOR))
      MUBMOR(tmpPAL,C->F.PAL,MAX_MPAR);

    if ((J->Cred.Q != NULL) && (J->Cred.Q->F.PALType == qalOR))
      MUBMOR(tmpPAL,J->Cred.Q->F.PAL,MAX_MPAR);
    }
 
  /* obtain 'exclusive' list */

  if ((J->Cred.Q != NULL) &&
      (J->Cred.Q->F.PALType == qalAND) &&
      (MUBMIsClear(J->Cred.Q->F.PAL,MAX_MPAR) != SUCCESS))
    {
    if (AndMask != TRUE)
      {
      MUBMCopy(tmpPAL,J->Cred.Q->F.PAL,MAX_MPAR);

      AndMask = TRUE;
      }
    else
      {
      MUBMAND(tmpPAL,J->Cred.Q->F.PAL,MAX_MPAR);
      }
    }
 
  if ((C != NULL) && 
      (C->F.PALType == qalAND) && 
      (MUBMIsClear(C->F.PAL,MAX_MPAR) != SUCCESS)) 
    {
    if (AndMask != TRUE)
      {
      MUBMCopy(tmpPAL,C->F.PAL,MAX_MPAR);
 
      AndMask = TRUE;
      }
    else
      {
      MUBMAND(tmpPAL,C->F.PAL,MAX_MPAR);
      }
    }
 
  if ((J->Cred.A != NULL) && 
      (J->Cred.A->F.PALType == qalAND) && 
      (MUBMIsClear(J->Cred.A->F.PAL,MAX_MPAR) != SUCCESS))
    {
    if (AndMask != TRUE)
      {
      MUBMCopy(tmpPAL,J->Cred.A->F.PAL,MAX_MPAR);
 
      AndMask = TRUE;
      }
    else
      {
      MUBMAND(tmpPAL,J->Cred.A->F.PAL,MAX_MPAR);
      }
    }
 
  if ((J->Cred.G->F.PALType == qalAND) && 
      (MUBMIsClear(J->Cred.G->F.PAL,MAX_MPAR) != SUCCESS))
    {
    if (AndMask != TRUE)
      {
      MUBMCopy(tmpPAL,J->Cred.G->F.PAL,MAX_MPAR);
 
      AndMask = TRUE; 
      }
    else
      {
      MUBMAND(tmpPAL,J->Cred.G->F.PAL,MAX_MPAR);
      }
    }
 
  if ((J->Cred.U->F.PALType == qalAND) &&
      (MUBMIsClear(J->Cred.U->F.PAL,MAX_MPAR) != SUCCESS))                   
    {
    if (AndMask != TRUE)
      {
      MUBMCopy(tmpPAL,J->Cred.U->F.PAL,MAX_MPAR);
 
      AndMask = TRUE;
      }
    else
      {
      MUBMAND(tmpPAL,J->Cred.U->F.PAL,MAX_MPAR);
      }
    }
 
  if ((J->Cred.U->F.PALType == qalONLY) && 
      (MUBMIsClear(J->Cred.U->F.PAL,MAX_MPAR) != SUCCESS)) 
    {
    MUBMCopy(tmpPAL,J->Cred.U->F.PAL,MAX_MPAR);
    }
  else if ((J->Cred.G->F.PALType == qalONLY) &&
           (MUBMIsClear(J->Cred.G->F.PAL,MAX_MPAR) != SUCCESS))      
    {
    MUBMCopy(tmpPAL,J->Cred.G->F.PAL,MAX_MPAR);
    }
  else if ((J->Cred.A != NULL) && (J->Cred.A->F.PALType == qalONLY) &&
           (MUBMIsClear(J->Cred.A->F.PAL,MAX_MPAR) != SUCCESS))      
    {
    MUBMCopy(tmpPAL,J->Cred.A->F.PAL,MAX_MPAR);
    }
  else if ((C != NULL) && (C->F.PALType == qalONLY) &&
           (MUBMIsClear(C->F.PAL,MAX_MPAR) != SUCCESS))      
    {
    MUBMCopy(tmpPAL,C->F.PAL,MAX_MPAR);
    }
  else if ((J->Cred.Q != NULL) && (J->Cred.Q->F.PALType == qalONLY) &&
           (MUBMIsClear(J->Cred.Q->F.PAL,MAX_MPAR) != SUCCESS))
    {
    MUBMCopy(tmpPAL,J->Cred.Q->F.PAL,MAX_MPAR);
    }


  if ((RPAL != NULL) && (RPAL[0] != 0))
    MUBMAND(tmpPAL,RPAL,MAX_MPAR);
 
  if (PAL != NULL)
    MUBMCopy(PAL,tmpPAL,MAX_MPAR);
 
  /* determine allowed partition default */
 
  if (PDef != NULL)
    {
    *PDef = MJobFindDefPart(J, C, tmpPAL);
 
    /* verify access to default partition */
 
    if (!MUBMCheck((*PDef)->Index,tmpPAL))
      {
      *PDef = &MPar[0];
 
      /* locate first legal partition */
 
      for (pindex = 0;pindex < MAX_MPAR;pindex++)
        {
        if (MUBMCheck(pindex,tmpPAL))
          {
          *PDef = &MPar[pindex];
 
          break;
          }
        }    /* END for (pindex) */
      }      /* END if (!(tmpPAL & (1 << PDef->Index))) */
 
    DBG(3,fSTRUCT) DPrint("INFO:     default partition for job %s set to %s (P:%s,U:%s,G:%s,A:%s,C:%s,Q:%s)\n",
      J->Name, 
      (*PDef)->Name,
      ((mpar_t *)MPar[0].F.PDef)->Name,
      ((mpar_t *)J->Cred.U->F.PDef)->Name,
      ((mpar_t *)J->Cred.G->F.PDef)->Name,
      (J->Cred.A == NULL) ? "NULL" : ((mpar_t *)J->Cred.A->F.PDef)->Name,
      (C == NULL) ? "NULL" : ((mpar_t *)C->F.PDef)->Name,
      (J->Cred.Q == NULL) ? "NULL" : ((mpar_t *)J->Cred.Q->F.PDef)->Name);
    }  /* END if (PDef != NULL) */
 
  if ((RPAL != NULL) && (RPAL[0] != 0))
    {
    if (tmpPAL[0] == 0)
      {
      DBG(2,fSTRUCT) DPrint("WARNING:  job %s cannot access requested Partitions '%s'\n",
        J->Name,
        MUListAttrs(ePartition,RPAL[0]));
 
      return(FAILURE);
      }
    }
 
  return(SUCCESS);
  }  /* END MJobGetPAL() */

/*
 * Determines default partition for a job (precedence: U,G,A,C,S,0)
 * 'PAL' is consulted to determine partition access if it is not NULL.
 * 'C' is consulted for the default partition if it is not NULL.
 */
mpar_t *MJobFindDefPart(

  mjob_t   *J,     /* I:  job                                */
  mclass_t *C,     /* I:  job class                          */
  int      *PAL)   /* I:  partition access list              */

  {
  mpar_t   *PDef;

  if ((J->Cred.U->F.PDef != NULL) &&
      (J->Cred.U->F.PDef != &MPar[0]) &&
      (PAL == NULL ||
       MUBMCheck(((mpar_t *)J->Cred.U->F.PDef)->Index,PAL)))
    {
    PDef = (mpar_t  *)J->Cred.U->F.PDef;
    }
  else if ((J->Cred.G->F.PDef != NULL) &&
           (J->Cred.G->F.PDef != &MPar[0]) &&
           (PAL == NULL ||
            MUBMCheck(((mpar_t *)J->Cred.G->F.PDef)->Index,PAL)))
    {
    PDef = (mpar_t  *)J->Cred.G->F.PDef;
    }
  else if ((J->Cred.A != NULL) &&
           (J->Cred.A->F.PDef != NULL) &&
           (J->Cred.A->F.PDef != &MPar[0]) &&
           (PAL == NULL ||
            MUBMCheck(((mpar_t *)J->Cred.A->F.PDef)->Index,PAL)))
    {
    PDef = (mpar_t  *)J->Cred.A->F.PDef;
    }
  else if ((C != NULL) &&
           (C->F.PDef != NULL) &&
           (C->F.PDef != &MPar[0]) &&
           (PAL == NULL ||
            MUBMCheck(((mpar_t *)C->F.PDef)->Index,PAL)))
    {
    PDef = (mpar_t  *)C->F.PDef;
    }
  else if ((J->Cred.Q != NULL) &&
           (J->Cred.Q->F.PDef != NULL) &&
           (J->Cred.Q->F.PDef != &MPar[0]) &&
           (PAL == NULL ||
	    MUBMCheck(((mpar_t *)J->Cred.Q->F.PDef)->Index,PAL)))
    {
    PDef = (mpar_t  *)J->Cred.Q->F.PDef;
    }
  else if ((MPar[0].F.PDef != NULL) &&
           (MPar[0].F.PDef != &MPar[0]))
    {
    PDef = (mpar_t  *)MPar[0].F.PDef;
    }
  else
    {
    PDef = &MPar[MDEF_SYSPDEF];
    }

  return PDef;
  }  /* END MJobFindDefPart() */


int MParFind(
 
  char    *PName,
  mpar_t **PP)
 
  {
  /* If found, return success with P pointing to partition.     */
  /* If not found, return failure with P pointing to            */
  /* first free partition if available, P set to NULL otherwise */
 
  int pindex;

  mpar_t *P;

  if (PP != NULL)
    *PP = NULL;
 
  if ((PName == NULL) ||
      (PName[0] == '\0'))
    {
    return(FAILURE);
    }
 
  for (pindex = 0;pindex < MAX_MPAR;pindex++)
    {
    P = &MPar[pindex];

    if (P->Name[0] == '\0')
      {
      /* 'free' partition slot found */
 
      if (PP != NULL)
        *PP = P;
 
      break;
      }
 
    if (strcmp(P->Name,PName) != 0)
      continue;
 
    /* partition found */

    if (PP != NULL)
      *PP = P;
 
    return(SUCCESS);
    }  /* END for (pindex) */ 
 
  return(FAILURE);
  }  /* END MParFind() */



int MParAdd(
 
  char    *PName,
  mpar_t **PP)
 
  {
  int pindex;

  mpar_t *P;

  char *FName = "MParAdd";
 
  DBG(4,fSTRUCT) DPrint("%s(%s,PPtr)\n",
    FName,
    (PName != NULL) ? PName : "NULL");
 
  if ((PName == NULL) ||
      (PName[0] == '\0'))
    {
    return(FAILURE);
    }

  /* NOTE:  support 'old style' partition indicies */

  if (isdigit(PName[0]))
    {
    pindex = (int)MIN(MAX_MPAR - 1,strtol(PName,NULL,0));

    P = &MPar[pindex];
   
    if (PP != NULL)
      *PP = P;

    return(SUCCESS);
    }

  if (MParFind(PName,&P) == SUCCESS)
    {
    /* partition already exists */

    if (PP != NULL)
      *PP = P;

    return(SUCCESS);
    }

  if (isdigit(PName[0]))
    {
    /* NOTE:  support 'old style' partition indicies */

    pindex = (int)MIN(MAX_MPAR - 1,strtol(PName,NULL,0));

    P = &MPar[pindex];

    if (P->Name[0] > '\1')
      return(SUCCESS);
    }

  if (!strcmp(PName,GLOBAL_MPARNAME))
    {
    P = &MPar[0];

    pindex = 0;
    }
  else if (!strcmp(PName,DEFAULT_MPARNAME))
    {
    P = &MPar[1];

    pindex = 1;
    }
  else 
    {
    for (pindex = 2;pindex < MAX_MPAR;pindex++)
      {
      P = &MPar[pindex];

      if (P->Name[0] <= '\1')
        {
        break;
        }
      }    /* END for (pindex) */

    if (pindex == MAX_MPAR)
      {
      /* partition overflow */

      return(FAILURE);
      }
    }
   
  /* available partition slot located */

  MUFree((char **)&P->L.IP);
  MUFree((char **)&P->L.JP);  
 
  memset(P,0,sizeof(mpar_t));
 
  P->Index = pindex;
 
  MUStrCpy(P->Name,PName,sizeof(P->Name));

  MParInitialize(P,NULL);

  memcpy(MAList[ePartition][pindex],PName,MAX_MNAME);

  if (P->L.IP == NULL)
    P->L.IP = (mpu_t *)calloc(1,sizeof(mpu_t));
 
  if (P->L.JP == NULL)
    P->L.JP = (mpu_t *)calloc(1,sizeof(mpu_t));
 
  if (PP != NULL)
    *PP = P;
 
  return(SUCCESS);
  }  /* END MParAdd() */




int MParInitialize(
 
  mpar_t  *P,     /* I */
  char    *PName) /* I */
 
  {
  int index;

  const char *FName = "MParInitialize";

  DBG(4,fSTRUCT) DPrint("%s(%s,%s)\n",
    FName,
    (P != NULL) ? P->Name : "NULL",
    (PName != NULL) ? PName : "NULL");
 
  if (P == NULL) 
    {
    return(FAILURE);
    }

  if (PName != NULL) 
    memset(P,0,sizeof(mpar_t));

  if (PName != NULL) 
    MUStrCpy(P->Name,PName,sizeof(P->Name));

  for (index = 0;index < MAX_MPAR;index++)
    {
    if (&MPar[index] == P)
      {
      P->Index = index;

      if (PName != NULL)
        MUStrCpy(MAList[ePartition][P->Index],PName,MAX_MNAME);                

      MParSetDefaults(P);

      break;
      }
    }  /* END for (index) */
 
  return(SUCCESS);
  }  /* END MParInitialize() */




int MParSetDefaults(

  mpar_t *P)  /* I */

  {
  int index;

  mfsc_t *F;

  const char *FName = "MParSetDefaults";

  DBG(6,fSTRUCT) DPrint("%s(%s,P)\n",
    FName,
    (P != NULL) ? P->Name : "NULL");

  if (P == NULL)
    {
    return(FAILURE);
    }

  P->BFPolicy             = DEFAULT_BACKFILLPOLICY;
  P->BFDepth              = DEFAULT_BACKFILLDEPTH;
  P->BFProcFactor         = DEFAULT_BACKFILLNODEFACTOR;
  P->BFMaxSchedules       = DEFAULT_MAXBACKFILLSCHEDULES;
 
  P->UseMachineSpeedForFS = DEFAULT_USEMACHINESPEEDFORFS;
  P->UseMachineSpeed      = DEFAULT_USEMACHINESPEED;
  P->UseSystemQueueTime   = DEFAULT_USESYSTEMQUEUETIME;
  P->JobPrioAccrualPolicy = DEFAULT_JOBPRIOACCRUALPOLICY;

  P->NAvailPolicy[0]      = DEFAULT_RESOURCEAVAILPOLICY;

  /*
   * HvB: default is to honour group_list parameter
  */
  P->IgnPbsGroupList      = 0;

  /*
   * HvB: default is to disable secondary group lookups for fairshare
  */
  P->FSSecondaryGroups   = 0;

  for (index = 0;index < MAX_MRESOURCETYPE;index++)
    {
    P->ResourceLimitPolicy[index]          = DEFAULT_MRESOURCELIMITPOLICY;
    P->ResourceLimitViolationAction[index] = DEFAULT_MRESOURCELIMITVIOLATIONACTION;
    }  /* END for (index) */

  P->NodeLoadPolicy       = MDEF_NODELOADPOLICY;
  P->JobNodeMatch         = DEFAULT_MJOBNODEMATCH;
  P->UntrackedProcFactor  = MDEF_NODEUNTRACKEDPROCFACTOR;

  if (P->Index == 0)
    P->F.PAL[0] = (1 << MDEF_SYSPAL);

  /* system policies */
 
  P->MaxMetaTasks         = DEFAULT_MAXMETATASKS;

  P->NodeSetPolicy        = 0;
  P->NodeSetDelay         = MDEF_NODESETDELAY;
  P->NodeSetAttribute     = MDEF_NODESETATTRIBUTE;
  P->NodeSetPriorityType  = mrspMinLoss;
 
  P->MaxJobStartTime      = DEFAULT_MAXJOBSTARTTIME;
  P->RejectNegPrioJobs    = DEFAULT_MREJECTNEGPRIOJOBS; 
  P->EnableNegJobPriority = DEFAULT_MENABLENEGJOBPRIORITY;
  P->EnableMultiNodeJobs  = DEFAULT_MENABLEMULTINODEJOBS;
  P->EnableMultiReqJobs   = DEFAULT_MENABLEMULTIREQJOBS;

  P->NAllocPolicy         = DEFAULT_MNALLOCPOLICY;
  P->DistPolicy           = DEFAULT_TASKDISTRIBUTIONPOLICY;
  P->BFMetric             = DEFAULT_MBFMETRIC;
  P->JobSizePolicy        = DEFAULT_JOBSIZEPOLICY;
 
  P->ResRetryTime         = DEFAULT_RESERVATIONRETRYTIME;
 
  P->ResTType             = DEFAULT_RESERVATIONTHRESHOLDTYPE;
  P->ResTValue            = DEFAULT_RESERVATIONTHRESHOLDVALUE;
 
  P->ResDepth[0]          = DEFAULT_RESERVATIONDEPTH;
  P->ResDepth[1]          = 0;
 
  P->ResQOSList[0][0]     = DEFAULT_RESERVATIONQOSLIST;
  P->ResQOSList[0][1]     = NULL;
 
  P->NodeSetDelay         = MDEF_NODESETDELAY;
 
  P->NodeDownStateDelayTime = MDEF_NODEDOWNSTATEDELAYTIME;
 
  for (index = 1;index < MAX_MQOS;index++)
    {
    P->ResQOSList[index][0] = (mqos_t *)MAX_MQOS;
    }  /* END for (index) */
 
  P->F.PAL[0]             = DEFAULT_MPARLIST;
  P->F.PDef               = &MPar[1];  /* NOTE:  default partition */

  P->F.QDef               = (void *)&MQOS[MDEF_SYSQDEF];
 
  MUBMSet(MDEF_SYSQDEF,P->F.QAL);
 
  P->F.JobFlags           = 0;

  F = &P->FSC;

  if (P->Index != 0)
    {
    /* '-1' indicates use global priorities */

    for (index = 0;index < MAX_MPRIOCOMPONENT;index++)
      {
      F->PCW[index] = -1;
      }  /* END for (index) */

    for (index = 0;index < MAX_MPRIOSUBCOMPONENT;index++)
      {
      F->PSW[index] = -1;
      }  /* END for (index) */

    P->ResPolicy = resDefault;
    }
  else
    {
    P->ResPolicy = DEFAULT_RESERVATIONMODE;

    for (index = 0;index < MAX_MPRIOCOMPONENT;index++)
      {
      F->PCW[index] = 1;
      }  /* END for (index) */

    for (index = 0;index < MAX_MPRIOSUBCOMPONENT;index++)
      {
      F->PSW[index] = 0;
      }  /* END for (index) */

    /* enable service queue time tracking only */

    F->PSW[mpsSQT]   = 1;
    F->XFMinWCLimit  = DEFAULT_XFMINWCLIMIT;
    F->FSPolicy      = DEFAULT_FSPOLICY;
    F->FSInterval    = DEFAULT_FSINTERVAL;
    F->FSDepth       = DEFAULT_FSDEPTH;
    F->FSDecay       = DEFAULT_FSDECAY;
    }  /* END else */

  return(SUCCESS);
  }  /* END MParSetDefaults() */




int MParListBMFromString(
 
  char *PString,
  int   BM[],
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
 
  mpar_t  *P;
 
  if (BM == NULL)
    return(FAILURE);
 
  MUBMClear(BM,MAX_MPAR);
 
  if (PString == NULL)
    return(SUCCESS);
 
  MUStrCpy(tmpLine,PString,sizeof(tmpLine));
 
  /* FORMAT:  PSTRING:   <RANGE>[:<RANGE>]... */
  /*          RANGE:     <VALUE>[-<VALUE>]    */
 
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
 
        if (MParFind(tmpName,&P) == SUCCESS)
          {
          MUBMSet(P->Index,BM);
          }
        else if ((Mode == mAdd) && (P != NULL))
          {
          MParInitialize(P,tmpName);
 
          MUBMSet(P->Index,BM);
          }
        }    /* END for (rindex) */
      }
    else
      {
      /* partition name provided */
 
      /* remove meta characters (handled externally) */
 
      if ((tail = strchr(rtok,'&')) != NULL)
        *tail = '\0';
      else if ((tail = strchr(rtok,'^')) != NULL)
        *tail = '\0';
 
      if (MParFind(rtok,&P) == SUCCESS)
        {
        MUBMSet(P->Index,BM); 
        }
      else if ((Mode == mAdd) && (P != NULL))
        {
        MParInitialize(P,rtok);
 
        MUBMSet(P->Index,BM);
        }
      }
 
    rtok = MUStrTok(NULL,",:",&TokPtr);
    }  /* END while (rtok) */
 
  return(SUCCESS);
  }  /* END MParListBMFromString() */




int MParShow(
 
  char *PName,       /* I */
  char *Buffer,      /* O */
  long *BufSize,     /* I */
  long  DisplayMode) /* I */
 
  {
  int pindex;
  int rindex; 
  int index;

  int URes;
  int ARes;
  int DRes;
  int CRes;

  int HeaderDisplayed;
  
  mgcred_t  *U;
  mgcred_t  *G;
  mgcred_t  *A;
  mgcred_t  *C;

  mpar_t   *P;
 
  char      Line[MAX_MLINE];
 
  char      PList[MAX_MLINE];

  const char *FName = "MParShow";
 
  DBG(2,fUI) DPrint("%s(PName,Buffer,BufSize,DisplayMode)\n",
    FName);
 
  sprintf(Buffer,"Displaying Partition Status\n\n");
 
  MUStrCpy(PList,MParBMToString(MPar[0].F.PAL),sizeof(PList));
 
  sprintf(temp_str,"System Partition Settings:  PList: %s PDef: %s\n\n",
    PList,
    (MPar[0].F.PDef != NULL) ? ((mpar_t  *)MPar[0].F.PDef)->Name : NONE);
  strcat(Buffer,temp_str);

  sprintf(temp_str,"%-20s %8s\n\n",
    "Name",
    "Procs");
  strcat(Buffer,temp_str);
 
  for (pindex = 0;pindex < MAX_MPAR;pindex++)
    {
    P = &MPar[pindex];
 
    if (P->Name[0] == '\0')
      {
      continue;
      }

    if ((pindex == 0) && (MPar[2].ConfigNodes == 0))
      {
      /* ignore global partition if only one partition exists */

      continue;
      }
 
    Line[0] = '\0';
 
    sprintf(temp_str,"%-20s %8d\n",
      (P->Name[0] != '\0') ? P->Name : NONE,
      P->CRes.Procs);
    strcat(Buffer,temp_str);
 
    /* check user access */
 
    Line[0] = '\0';
 
    for (index = 0;index < MAX_MUSER + MAX_MHBUF;index++)
      {
      U = MUser[index];
 
      if ((U == NULL) || (U->Name[0] == '\0') || (U->Name[0] == '\1'))
        continue;
 
      if (MUBMCheck(pindex,U->F.PAL))
        {
        sprintf(temp_str," %s%s",
          U->Name,
          MQALType[U->F.PALType]);
        strcat(Line,temp_str);
        }
      }  /* END for (index) */ 
 
    if (Line[0] != '\0')
      {
      sprintf(temp_str,"  Users:    %s\n",
        Line);
      strcat(Buffer,temp_str);
      }
 
    /* check group access */
 
    Line[0] = '\0';
 
    for (index = 0;index < MAX_MGROUP + MAX_MHBUF;index++)
      {
      G = &MGroup[index];
 
      if ((G->Name[0] == '\0') || (G->Name[0] == '\1'))
        continue;
 
      if (MUBMCheck(pindex,G->F.PAL))
        {
        sprintf(temp_str," %s%s",
          G->Name,
          MQALType[G->F.PALType]);
        strcat(Line,temp_str);
        }
      }  /* END for (index) */
 
    if (Line[0] != '\0')
      {
      sprintf(temp_str,"  Groups:   %s\n",
        Line);
      strcat(Buffer,temp_str);
      }
 
    /* check account access */
 
    Line[0] = '\0';
 
    for (index = 0;index < MAX_MACCT + MAX_MHBUF;index++)
      {
      A = &MAcct[index];
 
      if ((A->Name[0] == '\0') || (A->Name[0] == '\1')) 
        continue;
 
      if (MUBMCheck(pindex,A->F.PAL))
        {
        sprintf(temp_str," %s%s",
          A->Name,
          MQALType[A->F.PALType]);
        strcat(Line,temp_str);
        }
      }  /* END for (index) */
 
    if (Line[0] != '\0')
      {
      sprintf(temp_str,"  Accounts: %s\n",
        Line);
      strcat(Buffer,temp_str);
      }

       /* check class access */

    Line[0] = '\0';

    for (index = 0;index < MAX_MCLASS;index++)
      {
      C = &MClass[index];

      if ((C->Name[0] == '\0') || (C->Name[0] == '\1'))
        continue;

      if (MUBMCheck(pindex,C->F.PAL))
        {
        sprintf(temp_str," %s%s",
          C->Name,
          MQALType[C->F.PALType]);
        strcat(Line,temp_str);
        }
      }  /* END for (index) */

    if (Line[0] != '\0')
      {
      sprintf(temp_str,"  Classes: %s\n",
        Line);
      strcat(Buffer,temp_str);
      }

    if (P->Message != NULL)
      {
      sprintf(temp_str,"  Message: %s\n",
        P->Message);
      strcat(Buffer,temp_str);
      }  /* END if (P->Message) */
    }  /* END for (pindex) */

  /* show resource state */

  sprintf(temp_str,"\n%-12s %10s %10s %7s %10s %7s %10s %7s\n\n",
    "Partition",
    "Configured",
    "Up",
    "U/C",
    "Dedicated",
    "D/U",
    "Active",
    "A/U");
  strcat(Buffer,temp_str);

  for (rindex = mrNode;rindex <= mrDisk;rindex++)
    {
    HeaderDisplayed = FALSE;

    for (pindex = 0;pindex < MAX_MPAR;pindex++)
      {
      P = &MPar[pindex];
 
      if (P->ConfigNodes == 0)
        continue;

      if ((pindex == 0) && (MPar[2].ConfigNodes == 0))
        {
        /* ignore global partition if only one partition exists */

        continue;
        }
 
      if (strcmp(PName,NONE) && strcmp(PName,P->Name))
        continue;

      switch(rindex)
        {
        case mrProc:

          CRes = P->CRes.Procs;
          URes = P->URes.Procs;
          DRes = P->DRes.Procs;
          ARes = P->ARes.Procs;

          break;

        case mrMem:
         
          CRes = P->CRes.Mem;
          URes = P->URes.Mem;
          DRes = P->DRes.Mem;
          ARes = P->ARes.Mem;
 
          break;

        case mrSwap:

          CRes = P->CRes.Swap;
          URes = P->URes.Swap;
          DRes = P->DRes.Swap;
          ARes = P->ARes.Swap;

          break;

        case mrDisk:

          CRes = P->CRes.Disk;
          URes = P->URes.Disk;
          DRes = P->DRes.Disk;
          ARes = P->ARes.Disk;

          break;

        case mrNode:

          CRes = P->ConfigNodes;
          URes = P->UpNodes;
          DRes = P->ActiveNodes;
          ARes = P->UpNodes - P->ActiveNodes;

          break;

        default:

          CRes = 0;
	  URes = 0;
	  DRes = 0;
	  ARes = 0;

          break;
        }  /* END switch (rindex) */

      if (CRes <= 0)
        continue;

      if (HeaderDisplayed == FALSE)
        {
        HeaderDisplayed = TRUE;

        strcat(Buffer,MResourceType[rindex]);
        strcat(Buffer,"----------------------------------------------------------------------------\n");
        }

      sprintf(temp_str,"%-12s %10d %10d %6.2f%c %10d %6.2f%c %10d %6.2f%c\n",
        P->Name,
        CRes,
        URes,
        (double)((CRes > 0) ? URes * 100.0 / CRes : 0.0),
        '%',
        DRes,
        (double)((URes > 0) ? DRes * 100.0 / URes : 0.0),
        '%',
        (URes - ARes),
        (double)((URes > 0) ? (URes - ARes) * 100.0 / URes : 0.0),
        '%');
      strcat(Buffer,temp_str);
      } /* END for (pindex) */
    }    /* END for (rindex) */

  /* display class information */
 
  strcat(Buffer,"\nClass/Queue State\n\n");
 
  sprintf(temp_str,"%12s [<CLASS> <AVAIL>:<UP>]...\n\n",
    " ");
  strcat(Buffer,temp_str);
 
  for (pindex = 0;pindex < MAX_MPAR;pindex++)
    {
    P = &MPar[pindex];
 
    if (P->ConfigNodes == 0)
      continue;

    if ((pindex == 0) && (MPar[2].ConfigNodes == 0))
      {
      /* ignore global partition if only one partition exists */

      continue;
      }
 
    sprintf(temp_str,"%12s %s\n",
      P->Name,
      MUCAListToString(P->ARes.PSlot,P->URes.PSlot,NULL));
    strcat(Buffer,temp_str);
    }   /* END for (pindex) */
 
  return(SUCCESS);
  }  /* END MParShow() */




int MParGetTC( 
 
  mpar_t  *P,     /* I */
  mcres_t *Avl,   /* I */
  mcres_t *Cfg,   /* I */
  mcres_t *Ded,   /* I */
  mcres_t *Req,   /* I */
  long     Time)  /* I */
 
  {
  mcres_t tmpReq;
 
  int        NodeIsDedicated = FALSE;
 
  int        TC;

  const char *FName = "MParGetTC";

  DBG(6,fUI) DPrint("%s(%s,Avl,Cfg,Ded,Req,%ld)\n",
    FName,
    (P != NULL) ? P->Name : "NULL",
    Time);
 
  memcpy(&tmpReq,Req,sizeof(tmpReq));
 
  if (tmpReq.Procs == -1)
    {
    tmpReq.Procs = 1;
    NodeIsDedicated = TRUE;
    }
 
  if (tmpReq.Mem == -1)
    {
    tmpReq.Mem = 1;
    NodeIsDedicated = TRUE;
    }
 
  if (tmpReq.Swap == -1)
    {
    tmpReq.Swap = 1;
    NodeIsDedicated = TRUE;
    }
 
  if (tmpReq.Disk == -1)
    {
    tmpReq.Disk = 1;
    NodeIsDedicated = TRUE; 
    }
 
  TC = MNodeGetTC(NULL,Avl,Cfg,Ded,&tmpReq,Time);
 
  if (NodeIsDedicated == FALSE)
    {
    return(TC);
    }
 
  return(MIN(TC,P->ConfigNodes));
  }  /* END MParGetTC() */




char *MParBMToString(
 
  int *BM)  /* I */
 
  {
  int     pindex;
 
  mpar_t  *P;
 
  static char tmpLine[MAX_MLINE];
 
  tmpLine[0] = '\0';
 
  for (pindex = 1;pindex < MAX_MPAR;pindex++)
    {
    P = &MPar[pindex];    

    if (!(BM[pindex >> M64.INTLBITS] & (1 << (pindex % M64.INTBITS))))
      continue;

    if (P->Name[0] == '\0')
      continue;
 
    if (tmpLine[0] != '\0')
      strcat(tmpLine,":");

    sprintf(temp_str,"%s",
      P->Name);
    strcat(tmpLine,temp_str);
    }  /* END for (pindex) */
 
  return(tmpLine);
  }  /* END MParBMToString() */




int MParProcessOConfig(

  mpar_t *P,      /* I */
  int     PIndex, /* I */
  int     IVal,
  double  DVal,
  char   *SVal,
  char  **SArray)

  {
  int tmpI;

  mpar_t *GP;

  if (P == NULL)
    {
    return(FAILURE);
    }

  GP = &MPar[0];

  switch (PIndex)
    {
    case mcoAdminMinSTime:

      P->AdminMinSTime = MUTimeFromString(SVal);

      break;

    case pBFMetric:

      P->BFMetric = MUGetIndex(SVal,MBFMPolicy,FALSE,P->BFMetric);

      if (MBFMPolicy[P->BFMetric] == NULL)
        {
        DBG(1,fCONFIG) DPrint("ALERT:    invalid %s parameter specified '%s'\n",
          MParam[pBFMetric],
          SVal);
        }

      break;

    case pBFPolicy:

      {
      int index;

      index = MUGetIndex(SVal,(const char **)MBFPolicy,0,bfNONE);

      if (index != bfNONE)
        {
        P->BFPolicy = index;
        }
      else
        {
        if ((MUBoolFromString(SVal,TRUE) == FALSE) || !strcmp(SVal,"NONE"))
          P->BFPolicy = bfNONE;
        }
      }    /* END BLOCK */

      break;

    case pBFDepth:

      P->BFDepth = IVal;

      break;

    case pBFProcFactor:

      P->BFProcFactor = IVal;

      break;

    case pBFMaxSchedules:

      P->BFMaxSchedules = IVal;

      break;

    case pJobNodeMatch:

      P->JobNodeMatch = 0;

      {
      int index;
      int index2;

      for (index = 0;SArray[index] != NULL;index++)
        {
        for (index2 = 0;MJobNodeMatchType[index2] != NULL;index2++)
          {
          if (strstr(SArray[index],MJobNodeMatchType[index2]))
            P->JobNodeMatch |= (1 << index2);
          }
        }
      }    /* END BLOCK */

      break;

    case pJobSizePolicy:

      P->JobSizePolicy = MUGetIndex(SVal,MJSPolicy,FALSE,P->JobSizePolicy);

      if (MJSPolicy[P->JobSizePolicy] == NULL)
        {
        DBG(1,fCONFIG) DPrint("ALERT:    invalid %s parameter specified '%s'\n",
          MParam[pJobSizePolicy],
          SVal);
        }

      break;

    case pMaxJobStartTime:

      P->MaxJobStartTime = MUTimeFromString(SVal);

      break;

    case pNodeAllocationPolicy:

      P->NAllocPolicy = MUGetIndex(SVal,MNAllocPolicy,FALSE,P->NAllocPolicy);

      /* sync GLOBAL and DEFAULT partitions */

      if ((P->Index == 0) &&
         ((MPar[1].NAllocPolicy == mnalNONE2) || (MPar[1].NAllocPolicy == DEFAULT_MNALLOCPOLICY)))
        {
        MPar[1].NAllocPolicy = P->NAllocPolicy;
        }
      else if ((P->Index == 1) &&
         ((MPar[0].NAllocPolicy == mnalNONE2) || (MPar[0].NAllocPolicy == DEFAULT_MNALLOCPOLICY)))
        {
        MPar[0].NAllocPolicy = P->NAllocPolicy;
        }

      break;

    case mcoRejectNegPrioJobs:

      P->RejectNegPrioJobs = MUBoolFromString(SVal,P->RejectNegPrioJobs);

      break;

    case mcoEnableMultiNodeJobs:

      P->EnableMultiNodeJobs = MUBoolFromString(SVal,P->EnableMultiNodeJobs);

      break;

    case mcoEnableMultiReqJobs:

      P->EnableMultiReqJobs = MUBoolFromString(SVal,P->EnableMultiReqJobs);

      break;

    case mcoEnableNegJobPriority:

      P->EnableNegJobPriority = MUBoolFromString(SVal,P->EnableNegJobPriority);

      break;

    case pNodeAvailPolicy:

      /* multi-string */

      {
      /* FORMAT:  <POLICY>[:<RESOURCETYPE>] ... */

      char *ptr;
      char *TokPtr;
      int   pindex;
      int   policyindex;
      int   rindex;

      for (pindex = 0;SArray[pindex] != NULL;pindex++)
        {
        ptr = MUStrTok(SArray[pindex],":",&TokPtr);

        policyindex = MUGetIndex(ptr,MNAvailPolicy,0,0);

        if ((ptr = MUStrTok(NULL,":",&TokPtr)) != NULL)
          {
          rindex = MUGetIndex(ptr,MResourceType,0,0);
          }
        else
          {
          rindex = 0;
          }

        P->NAvailPolicy[rindex] = policyindex;
        }  /* END for (pindex) */
      }

      break;

    case mcoResourceLimitPolicy:

      {
      /* FORMAT:  <RESOURCE>[:<POLICY>[:<ACTION>[:<VTIME>]]] */

      char *ptr;
      char *TokPtr;

      enum MResLimitPolicyEnum  RLPolicy;
      enum MResLimitVActionEnum RLAction;

      int  vindex;
      int  index;

      for (vindex = 0;SArray[vindex] != NULL;vindex++)
        {
        /* determine resource */

        if ((ptr = MUStrTok(SArray[vindex],":",&TokPtr)) == NULL)
          {
          DBG(1,fCONFIG) DPrint("ALERT:    invalid %s parameter specified '%s'\n",
            MParam[mcoResourceLimitPolicy],
            SArray[vindex]);

          continue;
          }

        if ((index = MUGetIndex(ptr,MResourceType,FALSE,mrNONE)) == mrNONE)
          {
          DBG(1,fCONFIG) DPrint("ALERT:    invalid %s parameter specified '%s'\n",
            MParam[mcoResourceLimitPolicy],
            SArray[vindex]);

          continue;
          }

        /* set defaults */

        P->ResourceLimitPolicy[index] = mrlpAlways;
        P->ResourceLimitViolationAction[index] = DEFAULT_MRESOURCELIMITVIOLATIONACTION;

        /* determine policy */

        if ((ptr = MUStrTok(NULL,":",&TokPtr)) == NULL)
          {
          /* no policy specified */

          continue;
          }

        if ((RLPolicy = 
              MUGetIndex(ptr,MResourceLimitPolicyType,FALSE,mrlpNONE)) == mrlpNONE)
          {
          /* invalid policy */

          P->ResourceLimitPolicy[index] = mrlpNONE;

          continue;
          }

        P->ResourceLimitPolicy[index] = RLPolicy;

        /* determine action */

        if ((ptr = MUStrTok(NULL,":",&TokPtr)) == NULL)
          {
          /* no action specified */

          continue;
          }

        if ((RLAction = 
              MUGetIndex(ptr,MPolicyAction,FALSE,mrlaNONE)) == mrlaNONE)
          {
          /* invalid action specified */

          continue;
          }

        P->ResourceLimitViolationAction[index] = RLAction;

        /* determine violation time */

        if ((ptr = MUStrTok(NULL," \t\n",&TokPtr)) == NULL)
          {
          /* no vtime specified */

          continue;
          }

        P->ResourceLimitMaxViolationTime[index] = MUTimeFromString(ptr);
        }  /* END for (vindex) */
      }    /* END BLOCK */

      break;

    case pResPolicy:

      tmpI = MUGetIndex(SVal,MResPolicy,FALSE,0);

      if (tmpI != 0)
        {
        P->ResPolicy = tmpI;
        }
      else
        {
        DBG(1,fCONFIG) DPrint("ALERT:    invalid %s parameter specified '%s'\n",
          MParam[pResPolicy],
          SVal);
        }

      break;

    case pResRetryTime:

      P->ResRetryTime = MUTimeFromString(SVal);

      break;

    case pResThresholdType:

      tmpI = MUGetIndex(SVal,MResThresholdType,FALSE,0);

      if (tmpI != 0)
        {
        P->ResTType = tmpI;
        }
      else
        {
        DBG(1,fCONFIG) DPrint("ALERT:    invalid %s parameter specified '%s'\n",
          MParam[pResThresholdType],
          SVal);
        }
      
      break;

    case pResThresholdValue:

      P->ResTValue = IVal;

      break;

    case pMaxMetaTasks:

      P->MaxMetaTasks = IVal;

      break;

    case pSystemMaxJobProc:

      GP->L.JP->HLimit[mptMaxProc][0] = IVal;
      GP->L.JP->SLimit[mptMaxProc][0] = IVal;

      break;

    case pSystemMaxJobTime:

      GP->L.JP->HLimit[mptMaxWC][0] = MUTimeFromString(SVal);
      GP->L.JP->SLimit[mptMaxWC][0] = GP->L.JP->HLimit[mptMaxWC][0];

      break;

    case pSystemMaxJobPS:

      GP->L.JP->HLimit[mptMaxPS][0] = IVal;
      GP->L.JP->SLimit[mptMaxPS][0] = IVal;

      break;

    case pTaskDistributionPolicy:

      tmpI = MUGetIndex(SVal,MTaskDistributionPolicy,FALSE,0);

      if (tmpI != 0)
        {
        P->DistPolicy = tmpI;
        }
      else
        {
        DBG(1,fCONFIG) DPrint("ALERT:    invalid %s parameter specified '%s'\n",
          MParam[pTaskDistributionPolicy],
          SVal);
        }

      break;

    case pUseSystemQueueTime:

      P->UseSystemQueueTime = MUBoolFromString(SVal,P->UseSystemQueueTime);

      break;

    case pUseCPUTime:

      P->UseCPUTime = MUBoolFromString(SVal,P->UseCPUTime);

      break;

    case pUseMachineSpeedForFS:

      P->UseMachineSpeedForFS = MUBoolFromString(SVal,FALSE);

      break;

    case pUseMachineSpeed:

      P->UseMachineSpeed = MUBoolFromString(SVal,FALSE);

      break;

    case pJobPrioAccrualPolicy:

      P->JobPrioAccrualPolicy = 
        MUGetIndex(SVal,MJobPrioAccrualPolicyType,0,P->JobPrioAccrualPolicy);

      break;

    case pBFPriorityPolicy:

      P->BFPriorityPolicy = MUGetIndex(SVal,MBFPriorityPolicyType,0,P->BFPriorityPolicy);

      break;

    case mcoBFChunkDuration:

      P->BFChunkDuration = MUTimeFromString(SVal);

      if (MSched.Iteration > 0)
        P->BFChunkBlockTime = MSched.Time + P->BFChunkDuration;

      break;

    case mcoBFChunkSize:

      P->BFChunkSize = IVal;

      break;

    case pNodeLoadPolicy:

      P->NodeLoadPolicy = MUGetIndex(SVal,MNodeLoadPolicyType,0,P->NodeLoadPolicy);

      break;

    case pNodeSetPolicy:

      P->NodeSetPolicy = MUGetIndex(SVal,(const char **)MResSetSelectionType,0,P->NodeSetPolicy);

      break;

    case pNodeSetAttribute:

      P->NodeSetAttribute = MUGetIndex(SVal,(const char **)MResSetAttrType,0,P->NodeSetAttribute);

      break;

    case pNodeSetDelay:

      P->NodeSetDelay = MUTimeFromString(SVal);

      break;

    case pNodeSetPriorityType:

      P->NodeSetPriorityType = MUGetIndex(SVal,(const char **)MResSetPrioType,0,mrspNONE);

      break;

    case pNodeSetList:

      {
      int index;

      for (index = 0;index < MAX_MATTR;index++)
        {
        if (SArray[index] == NULL)
          break;

        MUStrDup(&P->NodeSetList[index],SArray[index]);
        }  /* END for (index) */
      }    /* END BLOCK */

      break;

    case pNodeSetTolerance:

      P->NodeSetTolerance = DVal;

      break;

    /* HvB */
    case pIgnPbsGroupList:

      P->IgnPbsGroupList =  MUBoolFromString(SVal,FALSE);

      break;

    /* HvB */
    case pFSSecondaryGroups:

      P->FSSecondaryGroups =  MUBoolFromString(SVal,TRUE);

      break;

    default:

      /* NOT HANDLED */

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(PIndex) */

  return(SUCCESS);
  }  /* END MParProcessOConfig() */




int MParUpdate(

  mpar_t *SP)  /* I (optional) */

  {
  int pindex;
  int nindex;

  mnode_t *N;
  mpar_t  *GP;
  mpar_t  *P;

  const char *FName = "MParUpdate";

  DBG(4,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    (SP != NULL) ? SP->Name : "NULL");

  if (SP == NULL)
    {
    return(FAILURE);
    }

  DBG(4,fSTRUCT) DPrint("INFO:     P[%s]:  Total %d:%d  Up %d:%d  Idle %d:%d  Active %d:%d\n",
    SP->Name,
    SP->ConfigNodes,
    SP->CRes.Procs,
    SP->UpNodes,
    SP->URes.Procs,
    SP->IdleNodes,

    SP->ARes.Procs,
    SP->ActiveNodes,
    SP->DRes.Procs);

  GP = &MPar[0];

  /* clear partition utilization stats */

  for (pindex = 0;pindex < MAX_MPAR;pindex++)
    {
    P = &MPar[pindex];

    if ((SP != GP) && (P != GP) && (P != SP))
      continue;

    if (P->ConfigNodes == 0)
      continue;

    if (SP == GP)
      {
      P->ConfigNodes = 0;

      P->UpNodes     = 0;

      memset(&P->CRes,0,sizeof(P->CRes));
      memset(&P->URes,0,sizeof(P->URes));
      }

    P->IdleNodes   = 0;

    P->ActiveNodes = 0;

    memset(&P->ARes,0,sizeof(P->ARes));
    memset(&P->DRes,0,sizeof(P->DRes));
    }  /* END for (pindex) */

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    if ((N->FrameIndex > 0) &&
        (MSys[N->FrameIndex][N->SlotIndex].Attributes & (1 << attrSystem)))
      {
      continue;
      }

    /* associate node's classes with partition */

    P = &MPar[N->PtIndex];

    if (P == GP)
      {
      if (strcmp(N->Name,"GLOBAL"))
        {
        DBG(1,fSTRUCT) DPrint("ERROR:    node '%s' is not associated with any partition\n",
          N->Name);
        }

      continue;
      }

    if (SP == GP)
      {
      /* global update */

      /* NOTE:  node up status only changes on once per iteration basis */

      P->ConfigNodes++;

      MCResAdd(&P->CRes,&N->CRes,&N->CRes,1,FALSE);
      MCResAdd(&GP->CRes,&N->CRes,&N->CRes,1,FALSE);

      GP->ConfigNodes++;

      if (MNODEISUP(N) == TRUE)
        {
        P->UpNodes++;

        MCResAdd(&P->URes,&N->CRes,&N->CRes,1,FALSE);

        GP->UpNodes++;

        MCResAdd(&GP->URes,&N->CRes,&N->CRes,1,FALSE);
        }
      }   /* END if (SP == GP) */

    if (MNODEISUP(N) == TRUE)
      {
      /* add available node resources to available partition resources */

      MCResAdd(&P->ARes,&N->CRes,&N->ARes,1,FALSE);
      MCResAdd(&GP->ARes,&N->CRes,&N->ARes,1,FALSE);

      /* add dedicated node resources to dedicated partition resources */

      MCResAdd(&P->DRes,&N->CRes,&N->DRes,1,FALSE);
      MCResAdd(&GP->DRes,&N->CRes,&N->DRes,1,FALSE);

      if ((N->ARes.Procs == N->CRes.Procs) && (N->DRes.Procs == 0))
        {
        if ((SP == GP) || (SP == P))
          {
          P->IdleNodes++;
          }

        GP->IdleNodes++;
        }

      /* NOTE:  discrepancies occur when background load is applied */

      if ((N->TaskCount > 0) || (N->State == mnsReserved))
        {
        if ((SP == GP) || (SP == P))
          {
          P->ActiveNodes++;
          }

        GP->ActiveNodes++;
        }
      }    /* END if (MNODEISUP(N) == TRUE) */

    if ((SP == GP) || (SP == P))
      {
      DBG(5,fSTRUCT) DPrint("INFO:     MNode[%s] added to MPar[%s] (%d:%d)\n",

        N->Name,
        MPar[N->PtIndex].Name,
        N->ARes.Procs,
        N->CRes.Procs);
      }
    }      /* END for (nindex) */

  DBG(4,fSTRUCT) DPrint("INFO:     P[%s]:  Total %d:%d  Up %d:%d  Idle %d:%d  Active %d:%d\n",
    SP->Name,
    SP->ConfigNodes,
    SP->CRes.Procs,
    SP->UpNodes,
    SP->URes.Procs,
    SP->IdleNodes,
    SP->ARes.Procs,
    SP->ActiveNodes,
    SP->DRes.Procs);

  return(SUCCESS);
  }  /* END MParUpdate() */





int MParConfigShow(

  mpar_t *P,      /* I */
  int     VFlag,  /* I */
  int     PIndex, /* I (parameter index) */
  char   *Buffer) /* O */

  {
  if ((P == NULL) || (Buffer == NULL))
    {
    return(FAILURE);
    }

  if (P->Index == 0)
    {
    strcat(Buffer,"# global policies\n\n");
    }
  else
    {
    sprintf(temp_str,"\n# partition %s policies\n\n",
      P->Name);
    strcat(Buffer,temp_str);
    }

  if ((P->RejectNegPrioJobs != DEFAULT_MREJECTNEGPRIOJOBS) ||
      (VFlag || (PIndex == -1) || (PIndex == mcoRejectNegPrioJobs)))
    {
    strcat(
      Buffer,
      MUShowSArray(
        MParam[mcoRejectNegPrioJobs],
        P->Index,
        (P->RejectNegPrioJobs == TRUE) ? "TRUE" : "FALSE"));
    }

  if ((P->EnableNegJobPriority != DEFAULT_MENABLENEGJOBPRIORITY) ||
      (VFlag || (PIndex == -1) || (PIndex == mcoEnableNegJobPriority)))
    {
    strcat(
      Buffer,
      MUShowSArray(
        MParam[mcoEnableNegJobPriority],
        P->Index,
        (P->EnableNegJobPriority == TRUE) ? "TRUE" : "FALSE"));
    }

  if ((P->EnableMultiNodeJobs != DEFAULT_MENABLEMULTINODEJOBS) ||
      (VFlag || (PIndex == -1) || (PIndex == mcoEnableMultiNodeJobs)))
    {
    strcat(
      Buffer,
      MUShowSArray(
        MParam[mcoEnableMultiNodeJobs],
        P->Index,
        (P->EnableMultiNodeJobs == TRUE) ? "TRUE" : "FALSE"));
    }

  if ((P->EnableMultiReqJobs != DEFAULT_MENABLEMULTIREQJOBS) ||
      (VFlag || (PIndex == -1) || (PIndex == mcoEnableMultiReqJobs)))
    {
    strcat(
      Buffer,
      MUShowSArray(
        MParam[mcoEnableMultiReqJobs],
        P->Index,
        (P->EnableMultiReqJobs == TRUE) ? "TRUE" : "FALSE"));
    }

  if ((P->BFPriorityPolicy != mbfpNONE) || 
      (VFlag || (PIndex == -1) || (PIndex == pBFPriorityPolicy)))
    {
    strcat(
      Buffer,
      MUShowSArray(
        MParam[pBFPriorityPolicy],
        P->Index,
        (char *)MBFPriorityPolicyType[P->BFPriorityPolicy]));
    }

  if ((P->JobPrioAccrualPolicy != jpapNONE) || 
      (VFlag || (PIndex == -1) || (PIndex == pJobPrioAccrualPolicy)))
    {
    sprintf(temp_str,"%-30s  %s\n",
      MParam[pJobPrioAccrualPolicy],
      (char *)MJobPrioAccrualPolicyType[P->JobPrioAccrualPolicy]);
    strcat(Buffer,temp_str);
    }

  if ((P->NodeLoadPolicy != nlpNONE) || 
      (VFlag || (PIndex == -1) || (PIndex == pNodeLoadPolicy)))
    {
    sprintf(temp_str,"%-30s  %s\n",
      MParam[pNodeLoadPolicy],
      (char *)MNodeLoadPolicyType[P->NodeLoadPolicy]);
    strcat(Buffer,temp_str);
    }

  if (P->Index == 0)
    {
    if ((P->UseMachineSpeedForFS == TRUE) || 
        (VFlag || (PIndex == -1) || (PIndex == pUseMachineSpeedForFS)))
      {
      sprintf(temp_str,"%-30s  %s\n",
        MParam[pUseMachineSpeedForFS],
        (P->UseMachineSpeedForFS == TRUE) ? "TRUE" : "FALSE");
      strcat(Buffer,temp_str);
      }

    if ((P->UseMachineSpeed == TRUE) || 
        (VFlag || (PIndex == -1) || (PIndex == pUseMachineSpeed)))
      {
      sprintf(temp_str,"%-30s  %s\n",
        MParam[pUseMachineSpeed],
        (P->UseMachineSpeed == TRUE) ? "TRUE" : "FALSE");
      strcat(Buffer,temp_str);
      }

    if ((P->UseSystemQueueTime == FALSE) || 
        (VFlag || (PIndex == -1) || (PIndex == pUseSystemQueueTime)))
      {
      sprintf(temp_str,"%-30s  %s\n",
        MParam[pUseSystemQueueTime],
        (P->UseSystemQueueTime == TRUE) ? "TRUE" : "FALSE");
      strcat(Buffer,temp_str);
      }

    if ((P->UseLocalMachinePriority != TRUE) || 
        (VFlag || (PIndex == -1) || (PIndex == pUseLocalMachinePriority)))
      {
      sprintf(temp_str,"%-30s  %s\n",
        MParam[pUseLocalMachinePriority],
        (P->UseLocalMachinePriority == TRUE) ? "TRUE" : "FALSE");
      strcat(Buffer,temp_str);
      }

    if ((P->UntrackedProcFactor > 0.0) || 
        (VFlag || (PIndex == -1) || (PIndex == pNodeUntrackedProcFactor)))
      {
      sprintf(temp_str,"%-30s  %.1lf\n",
        MParam[pNodeUntrackedProcFactor],
        P->UntrackedProcFactor);
      strcat(Buffer,temp_str);
      }
    }    /* END if (P->Index == 0) */

  if ((P->JobNodeMatch != 0) || 
      (VFlag || (PIndex == -1) || (PIndex == pJobNodeMatch)))
    {
    int  index;
    char tmpLine[MAX_MLINE];

    tmpLine[0] = '\0';

    for (index = 0;MJobNodeMatchType[index] != NULL;index++)
      {
      if (P->JobNodeMatch & (1 << index))
        {
        if (tmpLine[0] != '\0')
          strcat(tmpLine," ");

        strcat(tmpLine,MJobNodeMatchType[index]);
        }
      }

    strcat(Buffer,MUShowSArray(MParam[pJobNodeMatch],P->Index,tmpLine));

    strcat(Buffer,"\n");
    }

  if ((P->MaxJobStartTime != MAX_MTIME) ||
      (VFlag || (PIndex == -1) || (PIndex == pMaxJobStartTime)))
    {
    strcat(Buffer,
      MUShowSArray(
	MParam[pMaxJobStartTime],
	P->Index,
	MULToTString(P->MaxJobStartTime)));

    strcat(Buffer,"\n");
    }

  /* system max policies */

  if ((P->MaxMetaTasks != DEFAULT_MAXMETATASKS) || 
      (VFlag || 
      (PIndex == -1) || 
      (PIndex == pMaxMetaTasks)))
    {
    strcat(Buffer,MUShowIArray(MParam[pMaxMetaTasks],P->Index,P->MaxMetaTasks));
    }

  /* node set policies */

  if ((P->NodeSetPolicy != mrssNONE) || (VFlag || (PIndex == -1) ||
      (PIndex == pNodeSetPolicy)))
    {
    strcat(
      Buffer,
      MUShowSArray(
        MParam[pNodeSetPolicy],
        P->Index,
        (char *)MResSetSelectionType[P->NodeSetPolicy]));
    }

  if ((P->NodeSetAttribute != mrssNONE) || (VFlag || (PIndex == -1) ||
      (PIndex == pNodeSetAttribute)))
    {
    strcat(
      Buffer,
      MUShowSArray(
        MParam[pNodeSetAttribute],
        P->Index,
        (char *)MResSetAttrType[P->NodeSetAttribute]));
    }

  if ((P->NodeSetList[0] != NULL) || (VFlag || (PIndex == -1) || (PIndex == pNodeSetList)))
    {
    char tmpLine[MAX_MLINE];
    int  index;

    tmpLine[0] = '\0';

    for (index = 0;P->NodeSetList[index] != NULL;index++)
      {
      sprintf(temp_str,"%s ",
        P->NodeSetList[index]);
      strcat(tmpLine,temp_str);
      }  /* END for (index) */

    strcat(
      Buffer,
      MUShowSArray(
        MParam[pNodeSetList],
        P->Index,
        tmpLine));
    }

  if ((P->NodeSetDelay != MDEF_NODESETDELAY) || (VFlag || (PIndex == -1) ||
      (PIndex == pNodeSetDelay)))
    {
    strcat(
      Buffer,
      MUShowSArray(
        MParam[pNodeSetDelay],
        P->Index,
        (char *)MULToTString(P->NodeSetDelay)));
    }

  if ((P->NodeSetPriorityType != mrspNONE) || (VFlag || (PIndex == -1) ||
      (PIndex == pNodeSetPriorityType)))
    {
    strcat(
      Buffer,
      MUShowSArray(
        MParam[pNodeSetPriorityType],
        P->Index,
        (char *)MResSetPrioType[P->NodeSetPriorityType]));
    }

  if ((P->NodeSetTolerance != MDEF_NODESETTOLERANCE) || (VFlag || (PIndex == -1) ||
      (PIndex == pNodeSetTolerance)))
    {
    strcat(
      Buffer,
      MUShowFArray(
        MParam[pNodeSetTolerance],
        P->Index,
        P->NodeSetTolerance));
    }

  if (P->Index == 0)
    {
    mfsc_t *F;

    int     index;

    F = &P->FSC;

    /* show master partition config */

    strcat(Buffer,"\n");

    /* backfill policies */

    if ((P->BFPolicy != bfNONE) ||
        (VFlag || (PIndex == -1) || (PIndex == pBFPolicy)))
      {
      if ((PIndex == -1) || (PIndex == pBFPolicy) || VFlag)
        strcat(Buffer,MUShowSArray(MParam[pBFPolicy],P->Index,(char *)MBFPolicy[P->BFPolicy]));

      if ((P->BFDepth != DEFAULT_BACKFILLDEPTH) ||
          (VFlag || (PIndex == -1) || (PIndex == pBFDepth)))
        strcat(Buffer,MUShowIArray(MParam[pBFDepth],P->Index,P->BFDepth));

      if ((P->BFProcFactor > 0) ||
          (VFlag || (PIndex == -1) || (PIndex == pBFProcFactor)))
        strcat(Buffer,MUShowIArray(MParam[pBFProcFactor],P->Index,P->BFProcFactor));

      if ((P->BFMaxSchedules != DEFAULT_MAXBACKFILLSCHEDULES) ||
          (VFlag || (PIndex == -1) || (PIndex == pBFMaxSchedules)))
        strcat(Buffer,MUShowIArray(MParam[pBFMaxSchedules],P->Index,P->BFMaxSchedules));

      if ((PIndex == -1) || (PIndex == pBFMetric))
        strcat(Buffer,MUShowSArray(MParam[pBFMetric],P->Index,(char *)MBFMPolicy[P->BFMetric]));

      if (PIndex == -1)
        strcat(Buffer,"\n");
      }  /* END if ((P->BFPolicy != bfNONE) || ... ) */

  if ((P->BFChunkDuration != 0) ||
      (VFlag || (PIndex == -1) || (PIndex == mcoBFChunkDuration)))
    {
    strcat(
      Buffer,
      MUShowSArray(
        MParam[mcoBFChunkDuration],
        P->Index,
        MULToTString(P->BFChunkDuration)));
    }

  if ((P->BFChunkSize != 0) ||
      (VFlag || (PIndex == -1) || (PIndex == mcoBFChunkSize)))
    {
    strcat(
      Buffer,
      MUShowIArray(
        MParam[mcoBFChunkSize],
        P->Index,
        P->BFChunkSize));
    }


    if (VFlag || (PIndex == -1) || (PIndex == pPreemptPolicy))
      {
      strcat(
        Buffer,
        MUShowSArray(
          MParam[pPreemptPolicy],
          P->Index,
          (char *)MPreemptPolicy[MSched.PreemptPolicy]));
      }

    if (VFlag || (PIndex == -1) || (PIndex == mcoAdminMinSTime))
      {
      strcat(
        Buffer,
        MUShowSArray(
          MParam[mcoAdminMinSTime],
          P->Index,
          MULToTString(P->AdminMinSTime)));
      }

    if ((PIndex == mcoResourceLimitPolicy) || VFlag || (PIndex == -1))
      {
      int  index;
      char tmpLine[MAX_MLINE];

      tmpLine[0] = '\0';

      for (index = 1;MResourceType[index] != NULL;index++)
        {
        if (P->ResourceLimitPolicy[index] != mrlpNONE)
          {
          if (P->ResourceLimitMaxViolationTime[index] > 0)
            {
            sprintf(temp_str,"%s:%s:%s:%s ",
              MResourceType[index],
              MResourceLimitPolicyType[P->ResourceLimitPolicy[index]],
              MPolicyAction[P->ResourceLimitViolationAction[index]],
              MULToTString(P->ResourceLimitMaxViolationTime[index]));
            strcat(tmpLine,temp_str);
            }
          else
            {
            sprintf(temp_str,"%s:%s:%s ",
              MResourceType[index],
              MResourceLimitPolicyType[P->ResourceLimitPolicy[index]],
              MPolicyAction[P->ResourceLimitViolationAction[index]]);
            strcat(tmpLine,temp_str);
            }
          }
        }    /* END for (index) */

      strcat(
        Buffer,
        MUShowSArray(
          MParam[mcoResourceLimitPolicy],
          P->Index,
          tmpLine));
      }  /* END if ((PIndex == mcoResourceLimitPolicy) || ...) */

    if ((PIndex == pNodeAvailPolicy) || VFlag || (PIndex == -1))
      {
      int  policyindex;
      int  rindex;

      char tmpLine[MAX_MLINE];

      tmpLine[0] = '\0';

      for (rindex = 0;MResourceType[rindex] != NULL;rindex++)
        {
        policyindex = P->NAvailPolicy[rindex];

        if (policyindex == 0)
          continue;

        if (tmpLine[0] != '\0')
          strcat(tmpLine," ");

        sprintf(temp_str,"%s:%s",
          MNAvailPolicy[policyindex],
          (rindex == 0) ? DEFAULT : MResourceType[rindex]);
        strcat(tmpLine,temp_str);
        }  /* END for (rindex) */

      strcat(
        Buffer,
        MUShowSArray(
          MParam[pNodeAvailPolicy],
          P->Index,
          tmpLine));
      }  /* END if ((PIndex == pNodeAvailPolicy) || ...) */

    /* node allocation policy */

    if ((PIndex == pNodeAllocationPolicy) || VFlag || (PIndex == -1))
      {
      strcat(
        Buffer,
        MUShowSArray(
          MParam[pNodeAllocationPolicy],
          P->Index,
          (char *)MNAllocPolicy[P->NAllocPolicy]));
      }

    /* task distribution policy */

    if (VFlag || (PIndex == -1) || (PIndex == pTaskDistributionPolicy))
      {
      strcat(
        Buffer,
        MUShowSArray(
          MParam[pTaskDistributionPolicy],
          P->Index,
          (char *)MTaskDistributionPolicy[P->DistPolicy]));
      }

    /* reservation policy */

    strcat(
      Buffer,
      MUShowSArray(
        MParam[pResPolicy],
        P->Index,
        (char *)MResPolicy[P->ResPolicy]));

    for (index = 0;index < MAX_MQOS;index++)
      {
      if (!VFlag &&
         ((P->ResDepth[index] == DEFAULT_RESERVATIONDEPTH) ||
          (P->ResDepth[index] == 0)) &&
          (P->ResQOSList[index][0] == (mqos_t *)MAX_MQOS))
        {
        continue;
        }

      if (VFlag || (PIndex == -1) || (PIndex == pResDepth))
        {
        strcat(
          Buffer,
          MUShowIArray(
            MParam[pResDepth],
            P->Index,
            P->ResDepth[index]));
        }

      if (VFlag || (P->ResQOSList[index][0] != (mqos_t *)MAX_MQOS))
        {
        char tmpLine[MAX_MLINE];

        int  qindex;

        if (P->ResQOSList[index][0] == (mqos_t *)MAX_MQOS)
          {
          strcpy(tmpLine,ALL);
          }
        else
          {
          tmpLine[0] = '\0';

          for (qindex = 0;P->ResQOSList[index][qindex] != NULL;qindex++)
            {
            if (P->ResQOSList[index][qindex] == (mqos_t *)MAX_MQOS)
              {
              sprintf(temp_str,"%s ",
                ALL);
              strcat(tmpLine,temp_str);
              }
            else
              {
              sprintf(temp_str,"%s ",
                P->ResQOSList[index][qindex]->Name);
              strcat(tmpLine,temp_str);
              }
            }
          }

        if (VFlag || (PIndex == -1) || (PIndex == pResQOSList))
          {
          strcat(
            Buffer,
            MUShowSArray(
              MParam[pResQOSList],
              index,
              tmpLine));
          }
        }    /* END  if (P->ResQOSList[index][0] != (mqos_t *)MAX_MQOS)) */
      }      /* END for (index) */

    if ((P->ResRetryTime != 0) || (VFlag || (PIndex == -1) || (PIndex == pResRetryTime)))
      {
      strcat(
        Buffer,
        MUShowSArray(
          MParam[pResRetryTime],
          P->Index,
          MULToTString(P->ResRetryTime)));
      }

    if ((P->ResTValue != 0) || (VFlag || (PIndex == -1) || (PIndex == pResThresholdValue)))
      {
      strcat(
        Buffer,
        MUShowSArray(MParam[pResThresholdType],
        P->Index,
        (char *)MResThresholdType[P->ResTType]));

      strcat(Buffer,MUShowIArray(MParam[pResThresholdValue],P->Index,P->ResTValue));
      }

    strcat(Buffer,"\n");

    /* FS policies */

    sprintf(temp_str,"%-30s  %s\n",MParam[pFSPolicy],MFSPolicyType[F->FSPolicy]);
    strcat(Buffer,temp_str);

    sprintf(temp_str,"%-30s  %s%s\n",
      MParam[pFSPolicy],MFSPolicyType[F->FSPolicy],
      (MSched.PercentBasedFS == TRUE) ? "%" : "");
    strcat(Buffer,temp_str);

    if ((F->FSPolicy == fspNONE) || (VFlag || (PIndex == -1) || (PIndex == pFSPolicy)))
      {
      sprintf(temp_str,"%-30s  %s\n",MParam[pFSInterval],MULToTString(F->FSInterval));
      strcat(Buffer,temp_str);
      sprintf(temp_str,"%-30s  %d\n",MParam[pFSDepth],F->FSDepth);
      strcat(Buffer,temp_str);
      sprintf(temp_str,"%-30s  %-6.2f\n",MParam[pFSDecay],F->FSDecay);
      strcat(Buffer,temp_str);

      strcat(Buffer,"\n");
      }

    strcat(Buffer,"\n");
    }  /* END if (P->Index == 0) */

  return(SUCCESS);
  }  /* END MParConfigShow() */




int MParDestroy(

  mpar_t **PP)  /* I */

  {
  mpar_t *P;

  if ((PP == NULL) || (*PP == NULL))
    {
    return(FAILURE);
    }

  P = *PP;

  /* free dynamic attributes */

  MUFree(&P->Message);

  /* NYI */

  return(SUCCESS);
  }  /* END MParDestroy() */



/* END MPar.c */

