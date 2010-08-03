/* HEADER */

#include "moab.h"
#include "msched-proto.h"  
 
/*
#define __MSEEKLONGRANGE 1
*/
 
extern msched_t    MSched;
extern mnode_t    *MNode[];
extern mstat_t     MStat;
extern mpar_t      MPar[];
extern mattrlist_t MAList;
extern mres_t     *MRes[];
extern sres_t      SRes[];
extern sres_t      OSRes[];

extern mre_t       MRE[]; 
 
extern mrm_t       MRM[];
extern mam_t       MAM[];
extern mckpt_t     MCP;
 
extern mqos_t      MQOS[];
extern mclass_t    MClass[];
 
extern const char *MResType[];
extern const char *MJRType[];
extern const char *MCBType[];
extern const char *MNodeState[];
extern const char *MResAttr[];
extern const char *MResFlags[];
extern const char *MWeekDay[];
extern const char *MNResAttr[];
extern const char *MAttrO[];
extern const char *MXO[];
extern const char *MXOC[];
 
extern mlog_t      mlog;
 
extern mrange_t    MRange[MAX_MRANGE];
extern mx_t        X;




int MResCreate(
 
  int           Type,      /* I */
  macl_t       *ACL,       /* I */
  char         *CAName,
  unsigned long Flags,
  mnalloc_t    *NodeList,  /* I */
  long          StartTime,
  long          EndTime,
  int           NodeCount, /* I */
  int           ProcCount, /* I */
  char         *RBaseName,
  mres_t      **ResP,      /* O */
  char         *RegEx,     /* I */
  mcres_t      *DRes)      /* I */
 
  {
  int     rindex;
 
  mres_t  *R;
 
  char    Message[MAX_MLINE];
 
  enum MHoldReasonEnum Reason;
 
  long    WallTime;
 
  const char *FName = "MResCreate";
 
  DBG(3,fSTRUCT) DPrint("%s(%s,ACL,%s,%ld,NodeList,%ld,%ld,%d,%d,%s,ResP,'%s',DRes)\n",
    FName, 
    MResType[Type],
    (CAName != NULL) ?
      CAName :
      "NULL",
    Flags,
    StartTime,
    EndTime,
    NodeCount,
    ProcCount,
    RBaseName,
    (RegEx != NULL) ? RegEx : "NULL");
 
  if (ResP != NULL)
    *ResP = NULL;

  /* perform sanity check */

  if (StartTime >= EndTime)
    {
    /* invalid timeframe */

    DBG(0,fSTRUCT) DPrint("ERROR:    invalid timeframe for reservation ID '%s' (starttime %ld >= endtime %ld)\n",
         ((ACL != NULL) && (ACL[0].Name[0] != '\0')) ?
         ACL[0].Name :
         "SYSTEM",
         StartTime,
         EndTime);

    return(FAILURE);
    }

  if (MSched.Time > MCONST_EFFINF)
    {
    StartTime = MAX(StartTime,MSched.Time - MCONST_EFFINF);
    }

  /* find first available reservation slot */
 
  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    R = MRes[rindex];
 
    if ((R == NULL) || (R->Name[0] == '\0') || (R->Name[0] == '\1'))
      break;
    }  /* END for (rindex) */
 
  if (rindex == MAX_MRES)
    {
    DBG(1,fSTRUCT) DPrint("ERROR:    reservation overflow on reservation type %s for ID '%s'\n",
      MResType[Type],
      ((ACL != NULL) && (ACL[0].Name[0] != '\0')) ?
          ACL[0].Name :
          "SYSTEM");
 
    return(FAILURE);
    } 
 
  MResInitialize(&MRes[rindex],NULL);
 
  R = MRes[rindex];
 
  R->Index = rindex;
 
  MResSetAttr(R,mraACL,(void *)ACL,0,0);
 
  MACLSet(R->CL,maRes,RBaseName,mcmpSEQ,nmNeutralAffinity,0,0);
  MACLSet(R->ACL,maRes,RBaseName,mcmpSEQ,nmNeutralAffinity,0,0);
 
  if ((NodeCount > 0) || (Flags & (1 << mrfStandingRes)))
    {
    /* is reservation is populated or is standing reservation */

    MResGetRID(R,RBaseName,R->Name);
    }
  else
    {
    /* use base/restore checkpointed name */
 
    strcpy(R->Name,RBaseName);
    }
 
  MTRAPRES(R,FName);
 
  /* check allocation */
 
  if ((CAName != NULL) &&
      (CAName[0] != '\0') &&
       strcmp(CAName,NONE))
    {
    MAcctAdd(CAName,&R->A);

    /* prevent checkpointed rsv's from double creating AM reservations */
 
    if (MSched.Iteration >= 0) 
      {
      if (StartTime > MSched.Time)
        WallTime = MIN(MAM[0].FlushInterval,EndTime - StartTime);
      else
        WallTime = MIN(MAM[0].FlushTime,EndTime) - MSched.Time;
 
      if (MAMAllocRReserve(
            &MAM[0],
            R->Name,
            StartTime,
            CAName,
            ProcCount,
            NodeCount,
            WallTime,
            0,
            MDEF_NODETYPE,
            &Reason) == FAILURE)
        {
        DBG(1,fSCHED) DPrint("ALERT:    cannot reserve allocation for %d procs for reservation %s\n",
          ProcCount,
          R->Name);
 
        MResDestroy(&R);
 
        return(FAILURE);
        }
      }
    else
      {
      R->AllocResPending = TRUE;
      }
    }
 
  MResSetAttr(R,mraType,(void **)&Type,0,0);
 
  R->StartTime  = StartTime;
  R->EndTime    = MIN(EndTime,MAX_MTIME - 1); 
 
  MREInsert(MRE,R->StartTime,R->EndTime,rindex,DRes,(MAX_MRES << 2));
 
  MResSetAttr(R,mraFlags,(void **)&Flags,0,0);
  MResSetAttr(R,mraHostExp,(void **)RegEx,mdfString,mSet);
 
  if ((NodeList != NULL) && (NodeList[0].N != NULL))
    {
    R->PtIndex = NodeList[0].N->PtIndex;
    }
  else
    {
    R->PtIndex = -1;
    }
 
  if (DRes != NULL)
    {
    memcpy(&R->DRes,DRes,sizeof(R->DRes));
    }
  else
    {
    R->DRes.Procs = -1;
    R->DRes.Mem   = -1;
    R->DRes.Disk  = -1;
    R->DRes.Swap  = -1;
    }

  if (NodeList != NULL)
    {
    if (MResAllocate(R,NodeList) == FAILURE)
      {
      MResDestroy(&R);

      return(FAILURE);
      }
    }    /* END if (NodeList != NULL) */
 
  if ((Type != mrtJob) && !(R->Flags & (1 << mrfStandingRes)))
    {
    sprintf(Message,"RESERVATIONCREATED:  %s %s %s %ld %ld %ld %d\n",
      R->Name, 
      MResType[Type],
      RBaseName,
      MSched.Time,
      StartTime,
      EndTime,
      R->NodeCount);
 
    MSysRegEvent(Message,0,0,1);
    }
 
  if (ResP != NULL)
    *ResP = R;
 
  if (R->Flags & (1 << mrfMeta))
    MResAdjustGResUsage(R,1);

  R->CTime = MSched.Time;
  R->MTime = MSched.Time;
 
  return(SUCCESS);
  }  /* END MResCreate() */




int MResAllocate(

  mres_t    *R,        /* I (modified) */
  mnalloc_t *NodeList) /* I */

  {
  int nindex;

  int TC;
  
  mnode_t *N;

  const char *FName = "MResAllocate";

  DBG(7,fSTRUCT) DPrint("%s(%s,NodeList)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");

  if ((R == NULL) || (NodeList == NULL))
    {
    return(FAILURE);
    }

  R->NodeCount = 0;
  R->TaskCount = 0;
  R->AllocPC = 0;

  /* link nodes to reservation */

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    if ((NodeList == NULL) || (NodeList[nindex].N == NULL))
      break;

    N = NodeList[nindex].N;

    MTRAPNODE(N,FName);

    TC = NodeList[nindex].TC;

    R->NodeCount ++;
    R->TaskCount ++;
    R->AllocPC += (R->DRes.Procs == -1) ?
      N->CRes.Procs :
      (TC * R->DRes.Procs);

    if (MResAddNode(R,N,TC,0) == FAILURE)
      {
      return(FAILURE);
      }

    if (N->PtIndex != R->PtIndex)
      R->PtIndex = 0;
    }    /* END for (nindex) */

  if (nindex == MAX_MNODE)
    {
    DBG(1,fSTRUCT) DPrint("ERROR:    node overflow in %s\n",
      FName);
    }

  return(SUCCESS);
  }  /* END MResAllocate() */




int MResAdjust(

  mres_t *RS,         /* I (optional) */
  long    StartTime,  /* I (optional) */
  int     TaskCount)  /* I (optional) */

  {
  mres_t  *R;
 
  int      rindex;
  long     BestStartTime;

  long     Delta;

  char     NodeMap[MAX_MNODE];

  mrange_t GRange[MAX_MRANGE];

  mnodelist_t MNodeList;

  mjob_t  tmpJ;
  mreq_t  tmpRQ;

  macl_t   tmpCL[MAX_MACL];
  macl_t   tmpACL[MAX_MACL];

  mnalloc_t tmpNodeList[MAX_MNODE + 1];

  mjob_t *J;

  const char *FName = "MResAdjust";

  DBG(2,fSCHED) DPrint("%s(%s,%ld,%d)\n",
    FName,
    (RS != NULL) ? RS->Name : "NULL",
    StartTime,
    TaskCount);

  J = &tmpJ;

  MJobMkTemp(J,&tmpRQ,tmpACL,tmpCL,tmpNodeList,NULL);
 
  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    R = MRes[rindex];
 
    if ((R == NULL) || (R->Name[0] == '\0'))
      break;

    if (R->Name[0] == '\1')
      continue;
 
    if (RS == NULL)
      {
      if (!(R->Flags & (1 << mrfMeta)) ||
          !(R->Flags & (1 << mrfTimeFlex)))
        {
        /* reservation is not adjustable */
 
        continue;
        }
 
      if (R->StartTime <= MSched.Time)
        {
        /* reservation is already active */
   
        continue;
        }
      }
    else
      {
      if (R != RS)
        {
        /* reservation not specified */

        continue;
        }
      }    /* END else (RS == NULL) */
 
    DBG(1,fCORE) DPrint("INFO:     evaluating reservation %s\n",
      R->Name);

    /* maintain existing reservation */
 
    MResToJob(R,J);

    MJobAddAccess(J,R->Name);

    if (StartTime > 0)
      BestStartTime = StartTime;
    else
      BestStartTime = R->StartTime;

    if (TaskCount > 0)
      MReqSetAttr(J,J->Req[0],mrqaTCReqMin,(void **)&TaskCount,mdfInt,mSet);
 
    if (MJobGetRange(
         J,
         J->Req[0],
         &MPar[0],
         MSched.Time,
         GRange,
         NULL,
         NULL,
         NodeMap,
         (1 << rtcStartEarliest),
         NULL) == FAILURE)
      {
      /* cannot locate available resources */

      continue;
      }

    if (StartTime > 0)
      {
      if (GRange[0].StartTime != StartTime)
        {
        DBG(2,fSCHED) DPrint("INFO:     cannot obtain desired starttime (%ld != %ld)\n",
          GRange[0].StartTime,
          StartTime);

        return(FAILURE);
        }
      }
    else if (GRange[0].StartTime >= BestStartTime)
      {
      /* new reservation is not earlier */

      continue;
      }

    BestStartTime = GRange[0].StartTime;

    /* select resources */

    if (MJobGetRange(
         J,
         J->Req[0],
         &MPar[0],
         BestStartTime,
         GRange,
         MNodeList,
         NULL,
         NodeMap,
         0,
         NULL) == FAILURE)
      {
      /* cannot locate new resources */

      continue;
      }

    /* NOTE:  must adjust global MRE table */

    MResDeallocateResources(R);

    MRERelease(MRE,R->Index,(MAX_MRES << 2));

    Delta = R->StartTime - BestStartTime;
    
    R->StartTime -= Delta;
    R->EndTime   -= Delta;
 
    MREInsert(
      MRE,
      R->StartTime,
      R->EndTime,
      R->Index,
      &R->DRes,
      (MAX_MRES << 2));

    if (MResAllocate(R,(mnalloc_t *)MNodeList[0]) == FAILURE)
      {
      DBG(1,fCORE) DPrint("ALERT:    cannot adjust reservation %s (reservation empty)\n",
        R->Name);

      continue;
      }
    }  /* END for (rindex) */

  return(SUCCESS);
  }  /* END MResAdjust() */




int MResSetAttr(

  mres_t *R,      /* I (modified) */
  enum MResAttrEnum AIndex, /* I */
  void   *AVal,   /* I */
  int     Format, /* I */
  int     Mode)   /* I */

  {
  int      nindex;
  int      sindex;

  mnode_t *N;

  long     tmpStartTime;
  long     tmpDuration = -1;

  mre_t   *RE;

  if (R == NULL)
    {
    return(FAILURE);
    }

  /* NOTE:  Mode:  0: ==   -1: -=    +1: += */

  switch(AIndex)
    {
    case mraACL:

      if (AVal == NULL)
        {
        R->ACL[0].Type = maNONE;        
        }
      else if (Format != mdfString)
        {
        memcpy(R->ACL,AVal,sizeof(R->ACL));         
        }
      else if (strcmp((char *)AVal,NONE) != 0)
        {
        MACLLoadConfigLine(R->ACL,(char *)AVal);
        }    /* END if (strcmp((char *)AVal,NONE) != 0) */
 
      break;

    case mraAAccount:

      if (Format == mdfString)
        {
        if (strcmp((char *)AVal,NONE))
          {
          if (MAcctAdd((char *)AVal,&R->A) == FAILURE)
            {
            DBG(1,fCKPT) DPrint("ALERT:    cannot add account %s for reservation %s\n",
              (char *)AVal,
              R->Name);
 
            return(FAILURE);
            }
          }
        }
      else
        {
        R->A = (mgcred_t *)AVal;
        }

      break;

    case mraAGroup:

      if (Format == mdfString)
        {
        if (strcmp((char *)AVal,NONE))
          {
          if (MGroupAdd((char *)AVal,&R->G) == FAILURE)
            {
            DBG(1,fCKPT) DPrint("ALERT:    cannot add group %s for reservation %s\n",
              (char *)AVal,
              R->Name);
 
            return(FAILURE);
            }
          }
        }
      else
        {
        R->G = (mgcred_t *)AVal;
        }

      break;

    case mraAUser:

      if (Format == mdfString)
        {
        if (strcmp((char *)AVal,NONE))
          {
          if (MUserAdd((char *)AVal,&R->U) == FAILURE)
            {
            DBG(1,fCKPT) DPrint("ALERT:    cannot add user %s for reservation %s\n",
              (char *)AVal,
              R->Name);
 
            return(FAILURE);
            }
          }
        }
      else
        {
        R->U = (mgcred_t *)AVal;
        }

      break;

    case mraFlags:

      if (Format == mdfString)
        R->Flags = strtol((char *)AVal,NULL,0);
      else
        R->Flags = *(long *)AVal;

      break;

    case mraHostExp:

      if ((AVal == NULL) || !strcmp((char *)AVal,NONE))
        {
        MUFree(&R->RegEx);

        break;
        }

      if ((R->RegEx == NULL) &&
         ((R->RegEx = (char *)calloc(1,strlen((char *)AVal) + 1)) == NULL))
        {
        break;
        } 
  
      MUStrCpy(R->RegEx,(char *)AVal,strlen((char *)AVal) + 1);
 
      break;

    case mraMaxTasks:

      if (Format == mdfString)
        R->MaxTasks = (int)strtol((char *)AVal,NULL,0);
      else
        R->MaxTasks = *(int *)AVal;

      break;
 
    case mraName:
 
      MUStrCpy(R->Name,(char *)AVal,sizeof(R->Name)); 
 
      break;

    case mraNodeCount:

      if (Format == mdfString)
        R->NodeCount = (int)strtol((char *)AVal,NULL,0);
      else
        R->NodeCount = *(int *)AVal;

      break;
 
    case mraNodeList:

      {
      int         NIndex;
      int         nindex;

      int         NCount;

      mnode_t    *N;
      mnode_t    *NPtr;

      mnalloc_t *NL;

      if (AVal == NULL)
        return(FAILURE);

      /* assume ordered list */

      NL = (mnalloc_t *)AVal;

      NIndex = 0;

      if (NL[0].N == NULL)
        {
        NPtr   = NULL;
        NCount = 0;
        }
      else
        {
        NPtr   = NL[0].N;
        NCount = NL[0].TC;
        }
       
      /* locate nodes */

      for (nindex = 0;nindex < MAX_MNODE;nindex++)
        {
        N = MNode[nindex];

        if ((N == NULL) || (N->Name[0] == '\0'))
          break;
 
        if (N->Name[0] == '\1')
          continue;

        switch(Mode)
          {
          case -1:

            if (NPtr == N)
              MResAddNode(R,N,NCount,Mode);      

            break;

          case 0:  

            if (NPtr == N)
              MResAddNode(R,N,NCount,Mode);
            else
              MResAddNode(R,N,0,Mode);

            break;

          case 1:
       
            if (NPtr == N)
              MResAddNode(R,N,NCount,Mode);

            break;
          }  /* END switch(Mode) */

        if (N == NPtr)
          {
          NIndex++;

          if (NL[NIndex].N == NULL)
            break;
 
          NPtr   = NL[NIndex].N;
          NCount = NL[NIndex].TC;
          }
        }    /* END for (nindex) */
      }      /* END BLOCK */
 
      break;

     case mraResources:

      if (Format == mdfString) 
        {
        memset(&R->DRes,0,sizeof(R->DRes));

        R->DRes.Procs = -1;

        MUCResFromString(&R->DRes,(char *)AVal);
        }
      else
        {
        memcpy(&R->DRes,(void *)AVal,sizeof(R->DRes));
        }
 
      break;

    case mraEndTime:
    case mraDuration:
    case mraStartTime:

      {
      int StartForward = FALSE;
      int EndForward   = FALSE;

      long     tmpStartTime = 0;
      long     tmpDuration  = 0;

      /* adjust rsv timeframe */

      if (AIndex == mraEndTime)
        {
        if (Format == mdfString)
          R->EndTime = strtol((char *)AVal,NULL,0);
        else
          R->EndTime = *(long *)AVal;

        tmpStartTime = R->StartTime;
        }
      else if (AIndex == mraStartTime)
        {
        if (Format == mdfString)
          tmpStartTime = strtol((char *)AVal,NULL,0);
        else
          tmpStartTime = *(long *)AVal;

        if ((R->StartTime > 0) && (R->EndTime > 0))
          tmpDuration = R->EndTime - R->StartTime;
        }
      else  /* set duration */
        {
        tmpStartTime = R->StartTime;

        if (Format == mdfString)
          tmpDuration = strtol((char *)AVal,NULL,0);
        else
          tmpDuration  = *(long *)AVal;
        }

      if (R->StartTime > 0)
        {
        if (tmpStartTime < R->StartTime)
          {
          StartForward = TRUE;
          EndForward   = TRUE;
          }

        if (tmpDuration < R->EndTime - R->StartTime)
          {
          EndForward = TRUE;
          }
        }

      R->StartTime = tmpStartTime;

      if (tmpDuration > 0)
        R->EndTime = tmpStartTime + tmpDuration;

      /* change all RE objects */

      for (nindex = 0;nindex < MAX_MNODE;nindex++)
        {
        N = MNode[nindex];

	if ((N == NULL) || (N->Name[0] == '\0'))
          break;

        if (N->Name[0] == '\1')
	  continue;

        for (sindex = 0;sindex < MSched.ResDepth << 1;sindex++)
          {
          RE = &N->RE[sindex];

          if (RE == (mre_t *)1)
            continue;

          if (RE == NULL)
            break;

          if (N->R[RE->Index] != R)
            continue;

          /* adjust times of RE */

          /* NYI */

          /* if node contains multiple container reservations, node RE table must be rebuilt */

          MNodeBuildRE(N,R,FALSE);
          }
        }  /* END for (nindex) */
      }    /* END BLOCK */

      break;

    case mraStatCAPS:

      if (Format == mdfString)
        R->CAPS = strtod((char *)AVal,NULL);
      else
        R->CAPS = *(double *)AVal;

      break;

    case mraStatCIPS:

      if (Format == mdfString)
        R->CIPS = strtod((char *)AVal,NULL);
      else
        R->CIPS = *(double *)AVal;

      break;

    case mraStatTAPS:

      if (Format == mdfString)
        R->TAPS = strtod((char *)AVal,NULL);
      else
        R->TAPS = *(double *)AVal;

      break;

    case mraStatTIPS:

      if (Format == mdfString)
        R->TIPS = strtod((char *)AVal,NULL);
      else
        R->TIPS = *(double *)AVal;

      break;

    case mraType:

      if (Format == mdfString)
        R->Type = (int)strtol((char *)AVal,NULL,0);
      else
        R->Type = *(int *)AVal;

      break;

    default:

      /* not handled */

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MResSetAttr() */




int MResAToString(

  mres_t *R,        /* I */
  int     AIndex,   /* I */
  char   *Buf,      /* O */
  int     SBufSize, /* I (optional) */
  int     Mode)     /* I */

  {
  int BufSize;

  if (Buf == NULL)
    {
    return(FAILURE);
    }

  BufSize = (SBufSize > 0) ? SBufSize : MMAX_LINE;

  Buf[0] = '\0';

  if (R == NULL)
    {
    return(FAILURE);
    }

  switch(AIndex)
    {
    case mraAAccount:  /* accountable account */

      if (R->A != NULL) 
        strcpy(Buf,R->A->Name);

      break;

    case mraACL:

      MACLListShow(R->ACL,-1,(1 << mfmAVP)|(1 << mfmVerbose),Buf); 

      break;

    case mraAGroup:    /* accountable group */

      if (R->G != NULL)
        strcpy(Buf,R->G->Name);

      break;

    case mraAUser:     /* accountable user */

      if (R->U != NULL)
        strcpy(Buf,R->U->Name);

      break;

    case mraCreds:

      {
      /* FORMAT:  <CRED>=<VAL>[,<CRED>=<VAL>]... */

      int aindex;

      if (R->CL == NULL)
        break;

      for (aindex = 0;R->CL[aindex].Type != maNONE;aindex++)
        {
        if (aindex != 0)
          strcat(Buf,",");

        switch(R->CL[aindex].Cmp)
          {
          case mcmpSEQ:
          case mcmpSSUB:
          case mcmpSNE:

            sprintf(Buf,"%s%s=%s",
              Buf,
              MAttrO[(int)R->CL[aindex].Type],
              R->CL[aindex].Name);

            break;

          default:

            sprintf(Buf,"%s%s=%ld",
              Buf,
              MAttrO[(int)R->CL[aindex].Type],
              R->CL[aindex].Value);
 
            break;
          }  /* END switch(R->CL[aindex].Cmp) */
        }    /* END for (aindex) */
      }      /* END BLOCK */

      break;

    case mraDuration:

      sprintf(Buf,"%ld",
        R->EndTime - R->StartTime);

      break;

    case mraEndTime:

      sprintf(Buf,"%ld",
        R->EndTime);

      break;

    case mraFlags:

      sprintf(Buf,"%ld",
        R->Flags);

      break;

    case mraHostExp:

      if (R->RegEx != NULL)
        {
        MUStrCpy(Buf,R->RegEx,BufSize);
        }

      break;
  
    case mraJState:

      {
      if ((R->Type != mrtJob) || (R->J == NULL))
        break;

      strcpy(Buf,MJobState[((mjob_t *)R->J)->State]);
      }  /* END BLOCK */

      break;

    case mraMaxTasks:

      sprintf(Buf,"%d",
        R->MaxTasks);

      break;

    case mraName:

      if (R->Name[0] != '\0')
        strcpy(Buf,R->Name);

      break;

    case mraNodeCount:

      sprintf(Buf,"%d",
        R->NodeCount);

      break;

    case mraNodeList:

      /* NYI */

      break;

    case mraOwner:

      if (R->O != NULL)
        {
        sprintf(Buf,"%s:%s",
          MXOC[R->OType],
          MOGetName(R->O,R->OType,NULL));
        }

      break;

    case mraResources:

      MUCResToString(&R->DRes,0,2,Buf);      

      break;

    case mraStartTime:

      sprintf(Buf,"%ld",
        R->StartTime);
 
      break;

    case mraStatCAPS:

      sprintf(Buf,"%.2lf",
        R->CAPS);

      break;

    case mraStatCIPS:

      sprintf(Buf,"%.2lf",
        R->CIPS);
 
      break;

    case mraStatTAPS:

      sprintf(Buf,"%.2lf",
        R->TAPS);

      break;

    case mraStatTIPS:

      sprintf(Buf,"%.2lf",
        R->TIPS);

      break;

    case mraTaskCount:

      /* NYI */

      break;

    case mraType:

      sprintf(Buf,"%d",
        R->Type);

      break;

    case mraXAttr:

      if (X.XResCPCreate != (char *(*)())0) 
        strcpy(Buf,(*X.XResCPCreate)(X.xd,R));

      break;

    default:

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MResAToString() */




int MResToXML(
 
  mres_t  *R,      /* I */
  mxml_t *E,      /* O */
  int     *SAList) /* I (optional) */
 
  {
  const int DAList[] = {
    mraName,
    mraType,
    mraStartTime,
    mraEndTime,
    mraNodeCount,
    mraACL,
    mraFlags,
    mraAAccount,
    mraAGroup,
    mraAUser,
    mraStatCAPS,
    mraStatCIPS,
    mraStatTAPS,
    mraStatTIPS,
    mraResources,
    mraHostExp,
    mraMaxTasks,
    mraXAttr,
    -1 };
 
  int  aindex;
 
  int *AList;
 
  char tmpString[MAX_MBUFFER];
 
  if ((R == NULL) || (E == NULL))
    {
    return(FAILURE);
    }
 
  if (SAList != NULL)
    AList = SAList;
  else
    AList = (int *)DAList;
 
  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    if ((MResAToString(R,AList[aindex],tmpString,sizeof(tmpString),0) == FAILURE) ||
        (tmpString[0] == '\0'))
      {
      continue;
      }
 
    MXMLSetAttr(E,(char *)MResAttr[AList[aindex]],tmpString,mdfString);
    }  /* END for (aindex) */
 
  return(SUCCESS);
  }  /* END MResToXML() */




int MResLoadCP(

  mres_t *RS,  /* I (optional) */
  char   *Buf) /* I */

  {
  char     tmpName[MAX_MNAME];
  char     ResID[MAX_MNAME];
 
  long     CkTime;
 
  char    *ptr;
 
  mxml_t *E = NULL;
 
  mres_t  *R;

  mres_t  *ResP;

  mres_t   tmpR;

  const char *FName = "MResLoadCP";
 
  DBG(4,fCKPT) DPrint("%s(RS,%s)\n",
    FName,
    (Buf != NULL) ? Buf : "NULL");
 
  if (Buf == NULL)
    {
    return(FAILURE);
    }
 
  /* FORMAT:  <RESHEADER> <RESID> <CKTIME> <RESSTRING> */
 
  /* determine res ID */
 
  sscanf(Buf,"%s %s %ld",
    tmpName,
    ResID,
    &CkTime);
 
  if (((long)MSched.Time - CkTime) > MCP.CPExpirationTime)
    {
    return(SUCCESS);
    }

  if ((ptr = strchr(Buf,'<')) == NULL)
    {
    return(FAILURE);
    }
 
  if (MXMLFromString(&E,ptr,NULL,NULL) == FAILURE)
    {
    return(FAILURE);
    }
 
  if (RS == NULL)
    {
    if (MResFind(ResID,&R) != SUCCESS)
      {
      R = &tmpR;

      MResInitialize(&R,NULL);         
      }
    }
  else
    {
    R = RS;
    }
 
  MResFromXML(R,E);

  MXMLDestroyE(&E);        
 
  if (R == &tmpR)
    {
    if (MResCreate(
         R->Type,
         R->ACL,
         (R->A != NULL) ? R->A->Name : "",
         R->Flags,
         /* R->NodeList, */ NULL,
         R->StartTime,
         R->EndTime,
         0,
         0,
         R->Name,
         &ResP,
         R->RegEx,
         &R->DRes) == FAILURE)
      {
      /* cannot create reservation */
 
      return(FAILURE);
      }

    if (R->XCP != NULL)
      {
      if (X.XResCPLoad != (int (*)())0)
        {
        (*X.XResCPLoad)(X.xd,R->XCP,ResP);
        }

      MUFree(&R->XCP);
      }

    ResP->CAPS = R->CAPS;
    ResP->CIPS = R->CIPS;
 
    ResP->TAPS = R->TAPS;
    ResP->TIPS = R->TIPS;

    if (R->A != NULL)
      MResSetAttr(ResP,mraAAccount,(void *)R->A,0,0);
 
    if (R->G != NULL)
      MResSetAttr(ResP,mraAGroup,(void *)R->G,0,0);
 
    if (R->U != NULL)
      MResSetAttr(ResP,mraAUser,(void *)R->U,0,0);

    if (R->MaxTasks > 0)
      MResSetAttr(ResP,mraMaxTasks,(void *)&R->MaxTasks,mdfInt,0);
    }  /* END if (RS == &tmpR) */
 
  return(SUCCESS);
  }  /* END MResLoadCP() */
 



int MResFromXML(

  mres_t  *R,  /* I (modified) */
  mxml_t *E)  /* I */

  {
  int aindex;
  int raindex;

  if ((R == NULL) || (E == NULL))
    {
    return(FAILURE);
    }

  /* NOTE:  do not initialize.  may be overlaying data */
 
  for (aindex = 0;aindex < E->ACount;aindex++)
    {
    raindex = MUGetIndex(E->AName[aindex],MResAttr,FALSE,0);
 
    if (raindex == 0)
      continue;
 
    MResSetAttr(R,raindex,(void **)E->AVal[aindex],mdfString,mSet);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MResFromXML() */



 
int MResTrap(
 
  mres_t *R)
 
  {
  /* NO-OP */

  return(SUCCESS);
  }  /* END MResTrap() */




int MResFind(
 
  char    *ResID,
  mres_t **RV)
 
  {
  int     rindex;
  mres_t *R;

  const char *FName = "MResFind";
 
  DBG(8,fCORE) DPrint("%s(%s,Res)\n",
    FName,
    ResID);
 
  /* locate reservation */
 
  R = NULL;
 
  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    R = MRes[rindex];
 
    if ((R == NULL) || (R->Name[0] == '\0'))
      {
      DBG(8,fUI) DPrint("INFO:     cannot locate reservation '%s'\n",
        ResID);

      return(FAILURE);
      }

    if (R->Name[0] == '\1')
      continue;

    if (!strcmp(ResID,R->Name))
      break;
    }  /* END for (rindex) */
 
  if (rindex == MAX_MRES)
    {
    DBG(8,fUI) DPrint("INFO:     cannot locate reservation '%s' (end of list)\n",
      ResID);
 
    return(FAILURE);
    }
 
  *RV = R; 
 
  if (R == NULL)
    return(FAILURE);
 
  return(SUCCESS);
  }  /* END MResFind() */




int MResBuildACL(

  mres_t *R)   /* I */

  {
  int aindex;

  if (R == NULL)
    {
    return(FAILURE);
    }

  aindex = 0;

  if (R->Name != NULL)
    {
    R->CL[aindex].Type     = maRes;
    strcpy(R->CL[aindex].Name,R->Name);
    R->CL[aindex].Cmp      = mcmpSEQ;    
    R->CL[aindex].Affinity = nmNeutralAffinity;

    aindex++;
    }

  if (R->EndTime > 0)
    {
    R->CL[aindex].Type     = maDuration;
    R->CL[aindex].Value    = R->EndTime - R->StartTime;
    R->CL[aindex].Cmp      = mcmpEQ;     
    R->CL[aindex].Affinity = nmNeutralAffinity;

    aindex++;
    }

  if (R->Flags & (1 << mrfByName))
    {
    R->CL[aindex].Type     = maRes;
    R->CL[aindex].Name[0]  = '\0';
    R->CL[aindex].Cmp      = mcmpSEQ;      
    R->CL[aindex].Affinity = nmNeutralAffinity;
    R->CL[aindex].Flags    |= (1 << maclRequired);

    aindex++;
    }

  /* accountable credentials also provide reservation creds */

  if (R->U != NULL)
    {
    R->CL[aindex].Type     = maUser;
    strcpy(R->CL[aindex].Name,R->U->Name);
    R->CL[aindex].Value    = 0;
    R->CL[aindex].Cmp      = mcmpSEQ;
    R->CL[aindex].Affinity = nmNeutralAffinity;

    aindex++;
    }

  if (R->G != NULL)
    {
    R->CL[aindex].Type     = maGroup;
    strcpy(R->CL[aindex].Name,R->G->Name);
    R->CL[aindex].Value    = 0;
    R->CL[aindex].Cmp      = mcmpSEQ;
    R->CL[aindex].Affinity = nmNeutralAffinity;

    aindex++;
    }

  if (R->A != NULL)
    {
    R->CL[aindex].Type     = maAcct;
    strcpy(R->CL[aindex].Name,R->A->Name);
    R->CL[aindex].Value    = 0;
    R->CL[aindex].Cmp      = mcmpSEQ;
    R->CL[aindex].Affinity = nmNeutralAffinity;

    aindex++;
    }

  R->CL[aindex].Type = maNONE;

  aindex = 0;

  /* reservation ACL includes job, user, group, account, QoS, or class names */
 
  return(SUCCESS);
  }  /* END MResBuildACL() */





int MResGetNRange(

  mres_t   *RPtr,      /* I: reservation requesting access */
  mnode_t  *N,         /* I: node possessing resources     */
  long      Duration,  /* I: minimum duration              */
  mrange_t *RRange,    /* I: required timeframe            */
  mrange_t *ARange,    /* O: availability range            */
  mcres_t  *DRes,
  char     *BRName)    /* O: blocking reservation name     */

  {
  int eindex;
  int rindex;
  int index;
  int index2;
 
  long   RangeTime;
 
  mcres_t CurRes;
  mcres_t DedRes;
  mcres_t DQRes;
 
  int     TC;
 
  mres_t *R;
  int     RC;
 
  mcres_t *BR;

  long    Overlap;

  int    PreRes;
  int    Same;
 
  int    ActiveRange;
 
  char   WCString[MAX_MNAME];
 
  int    ResBuffer;
 
  int    ResourcesAdded;
  int    ResourcesRemoved;
 
  int    InclusiveReservationCount;
 
  int    tmpTC;

  const char *FName = "MResGetNRange";

  DBG(7,fSCHED) DPrint("%s(%s,%s,%ld,RRange,ARange,BRName)\n",
    FName,
    (RPtr != NULL) ? RPtr->Name : "NULL",
    (N != NULL) ? N->Name : "NULL",
    Duration);

  if ((RPtr == NULL) || (N == NULL))
    {
    return(FAILURE);
    }

  MTRAPRES(RPtr,FName);
  MTRAPNODE(N,FName);
 
  memset(&ARange[0],0,sizeof(mrange_t));

#ifdef __MRESBUFFER
  ResBuffer = MSched.RMPollInterval; 
#else /* __MRESBUFFER */
  ResBuffer = 0;
#endif /* __MRESBUFFER */

  /* initialize available range */
 
  ARange[0].EndTime   = 0;
  ARange[0].TaskCount = 0;
  ARange[0].NodeCount = 0;

  R = NULL;

  /* locate first relevant event */

  for (eindex = 0;eindex < (MSched.ResDepth << 1);eindex++)
    {
    if (N->RE[eindex].Type == mreNONE)
      break;
 
    R = N->R[N->RE[eindex].Index];
 
    if ((R->EndTime > RRange[0].StartTime) ||
        (R->EndTime >= MAX_MTIME - 1))
      break;
 
    DBG(6,fSCHED) DPrint("INFO:     ignoring early RE[%d] event for '%s' (E: %ld <= S: %ld) : P: %ld\n",
      eindex,
      R->Name,
      R->EndTime,
      RRange[0].StartTime,
      MSched.Time);
    }  /* END for (eindex) */

  if (N->RE[eindex].Type == mreNONE)
    {
    /* if no relevant reservations present */
 
    ARange[0].TaskCount =
      MNodeGetTC(N,&N->ARes,&N->CRes,&N->DRes,&RPtr->DRes,
        MAX(MSched.Time,RRange[0].StartTime));
 
    ARange[0].NodeCount = 1;

    ARange[0].StartTime = RRange[0].StartTime + ResBuffer;
    ARange[0].EndTime   = MAX_MTIME;

    ARange[0].Affinity  = nmNeutralAffinity;
 
    /* terminate range list */
 
    ARange[1].EndTime = 0;

    if (ARange[0].TaskCount < RRange[0].TaskCount)
      {
      DBG(6,fSCHED) DPrint("INFO:     node %s contains inadequate resources (State: %s)\n",
        N->Name,
        MNodeState[N->State]);
 
      tmpTC = MNodeGetTC(N,&N->ARes,&N->CRes,&N->DRes,&RPtr->DRes,MAX_MTIME);

      /* fail unless resbuffer in use */

      if ((tmpTC < RRange[0].TaskCount) && (ResBuffer > 0))
        {
        /* experiencing resource race condition */
        }
      else
        {
        ARange[0].Affinity = nmUnavailable;
 
        return(FAILURE);
        }
      }

    DBG(8,fSCHED)
      {
      strcpy(WCString,MULToTString(ARange[0].StartTime - MSched.Time));
 
      DPrint("INFO:     closing ARange[%d] (%s -> %s : %d) (0)\n",
        0, 
        WCString,
        MULToTString(ARange[0].EndTime - MSched.Time),
        ARange[0].TaskCount);
      }
 
    strcpy(WCString,MULToTString(R->EndTime - R->StartTime));
 
    DBG(5,fSCHED) DPrint("INFO:     node %s supports %d task%s of res %s for %s of %s (no RE)\n",
      N->Name,
      ARange[0].TaskCount,
      (ARange[0].TaskCount == 1) ? "" : "s",
      R->Name,
      MULToTString(ARange[0].EndTime - ARange[0].StartTime),
      WCString);

    if (DRes != NULL)
      {
      memset(DRes,0,sizeof(mcres_t));
 
      MCResAdd(DRes,&N->CRes,&N->CRes,1,FALSE);
      }
 
    if (RPtr->Flags & (1 << mrfAdvRes))
      {
      ARange[0].Affinity = nmUnavailable;
 
      DBG(5,fSCHED) DPrint("INFO:     no reservation found for AdvRes res %s\n",
        RPtr->Name);
 
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
 
  InclusiveReservationCount = 0;
 
  ActiveRange = FALSE;
 
  PreRes = TRUE;
  rindex = 0;
 
  /* initialize available resources */
 
  memcpy(&CurRes,&N->CRes,sizeof(CurRes));
  memset(&DedRes,0,sizeof(DedRes));
 
  memset(&DQRes,0,sizeof(DQRes));
 
  DQRes.Procs = -1;
 
  DBG(8,fSCHED)
    {
    DPrint("INFO:     initial resources: %s\n",
      MUCResToString(&CurRes,0,0,NULL));
 
    DPrint("INFO:     requested resources: %s (x%d)\n",
      MUCResToString(&RPtr->DRes,0,0,NULL),
      RRange[0].TaskCount);
    }
 
  /* consume required resources */
 
  MCResRemove(&CurRes,&N->CRes,&RPtr->DRes,RRange[0].TaskCount,FALSE);
 
  if (MUCResIsNeg(&CurRes) == SUCCESS)
    { 
    /* if inadequate resources are configured */
 
    DBG(6,fSCHED) DPrint("INFO:     inadequate resources configured\n");
 
    ARange[0].Affinity = nmUnavailable;
 
    return(FAILURE);
    }
 
  if (RPtr->Flags & (1 << mrfDedicatedNode))
    {
    /* all resources dedicated */
 
    memset(&CurRes,0,sizeof(CurRes));
    }
 
  if (DRes != NULL)
    {
    /* set up default structures */
 
    memset(DRes,0,sizeof(mcres_t));
 
    MCResAdd(DRes,&CurRes,&N->CRes,1,FALSE);
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
 
      if ((MUCResIsNeg(&CurRes) == SUCCESS) ||
         ((RPtr->Flags & (1 << mrfAdvRes)) &&
          (InclusiveReservationCount <= 0)))
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
        TC = MNodeGetTC(N,&CurRes,&N->CRes,&DedRes,&RPtr->DRes,MSched.Time);
 
        ARange[rindex].TaskCount = RRange[0].TaskCount + TC;
        ARange[rindex].NodeCount = 1;
        ARange[rindex].StartTime = RRange[0].StartTime + ResBuffer; 
 
        ActiveRange = TRUE;
 
        if (DRes != NULL)
          MUCResGetMin(DRes,DRes,&CurRes);
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
      /* NOTE:  BR represents 'per task' resources dedicated */

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
      /* NOTE:  BR represents total resources dedicated */

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
      /* be generous (ie, Duration - (R->StartTime - ARange[rindex].StartTime) */
 
      Overlap =
        MIN(R->EndTime,ARange[rindex].StartTime + Duration) -
        MAX(R->StartTime,ARange[rindex].StartTime);
 
      if (Overlap < 0)
        Overlap = MIN(R->EndTime - R->StartTime,Duration);
      }
    else
      {
      Overlap = MIN(
        R->EndTime - MAX(R->StartTime,RRange[0].StartTime),
        Duration);
      }

    Overlap = MIN(Duration,Overlap);
 
    DBG(7,fSCHED) DPrint("INFO:     checking reservation %s %s at time %s (O: %ld)\n",
      N->R[N->RE[eindex].Index]->Name,
      (N->RE[eindex].Type == mreStart) ? "start" : "end",
      MULToTString(N->RE[eindex].Time - MSched.Time),
      Overlap);
 
    if (N->RE[eindex].Type == mreStart)
      {
      /* reservation start */
 
      if (MResCheckRAccess(R,RPtr,Overlap,&Same,&ARange[0].Affinity) == TRUE)
        {
        DBG(8,fSCHED) DPrint("INFO:     reservations are inclusive\n");
 
        InclusiveReservationCount++;
 
        if (RPtr->Flags & (1 << mrfAdvRes))
          ResourcesAdded = TRUE; 
        }
      else
        {
        DBG(8,fSCHED) DPrint("INFO:     reservations are exclusive\n");
 
        ResourcesRemoved = TRUE;
 
        MCResRemove(&CurRes,&N->CRes,BR,RC,FALSE);
 
        DBG(7,fSCHED)
          {
          DPrint("INFO:     removed resources: %s (x%d)\n",
            MUCResToString(BR,0,0,NULL),
            RC);
 
          DPrint("INFO:     resulting resources: %s\n",
            MUCResToString(&CurRes,0,0,NULL));
          }
 
        if (BRName != NULL)
          strcpy(BRName,N->R[N->RE[eindex].Index]->Name);
        }
      }    /* END if (N->RE[eindex].Type == mreStart) */
    else if (N->RE[eindex].Type == mreEnd)
      {
      /* reservation end */
 
      if (MResCheckRAccess(R,RPtr,Overlap,&Same,&ARange[0].Affinity) == TRUE)
        {
        DBG(8,fSCHED) DPrint("INFO:     reservations are inclusive\n");
 
        InclusiveReservationCount--;
 
        ResourcesRemoved = TRUE;
        }
      else
        {
        DBG(8,fSCHED) DPrint("INFO:     reservations are exclusive\n");
 
        ResourcesAdded = TRUE; 
 
        MCResAdd(&CurRes,&N->CRes,BR,RC,FALSE);
 
        DBG(7,fSCHED)
          {
          DPrint("INFO:     added resources: %s (x%d)\n",
            MUCResToString(BR,0,0,NULL),
            RC);
 
          DPrint("INFO:     resulting resources: %s\n",
            MUCResToString(&CurRes,0,0,NULL));
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
 
      if ((R->StartTime == RRange[0].StartTime) && (PreRes == TRUE))
        {
        /* adjust/check requirements at job start */
 
        DBG(8,fSCHED) DPrint("INFO:     performing starttime check (%d)\n",
          eindex);
 
        PreRes = FALSE;
 
        if (MUCResIsNeg(&CurRes) == SUCCESS)
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
        else if (!(RPtr->Flags & (1 << mrfAdvRes)) ||
                (InclusiveReservationCount > 0))
          {
          /* range is active unless advance reservation is required and not found */
 
          ActiveRange = TRUE;
 
          TC = MNodeGetTC(N,&CurRes,&N->CRes,&DedRes,&RPtr->DRes,MSched.Time);
 
          ARange[rindex].TaskCount = RRange[0].TaskCount + TC;
          ARange[rindex].NodeCount = 1;
          ARange[rindex].StartTime = RRange[0].StartTime + ResBuffer;
 
          if (DRes != NULL)
            MUCResGetMin(DRes,DRes,&CurRes);
          }
        }   /* END if ((R->StartTime >= RRange[0].StartTime) && ... */
 
      if ((ResourcesRemoved == TRUE) && (ActiveRange == TRUE))
        {
        ResourcesRemoved = FALSE;
        ResourcesAdded   = FALSE;
 
        if ((MUCResIsNeg(&CurRes) == SUCCESS) ||
           ((RPtr->Flags & (1 << mrfAdvRes)) &&
            (InclusiveReservationCount == 0)))
          {
          /* terminate active range */
 
          DBG(6,fSCHED) DPrint("INFO:     resources unavailable at time %s during reservation %s %s\n",
            MULToTString(N->RE[eindex].Time - MSched.Time),
            R->Name,
            (N->RE[eindex].Type == mreStart) ? "start" : "end"); 
 
          if (((N->RE[eindex].Time - ARange[rindex].StartTime) >= Duration) ||
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
                RPtr->Name,
                N->Name);
 
              return(FAILURE);
              }
            }
          else
            {
            DBG(8,fSCHED) DPrint("INFO:     range too short (ignoring)\n");
            }
 
          ActiveRange = FALSE;
 
          if (N->RE[eindex].Time >= RRange[0].EndTime)
            { 
            break;
            }
          }   /* END if (MUCResIsNeg(&CurRes) || ... */
        else
          {
          /* check new taskcount */
 
          TC = MNodeGetTC(N,&CurRes,&N->CRes,&DedRes,&RPtr->DRes,MSched.Time);
 
          if (N->RE[eindex].Time >= RRange[0].StartTime)
            {
            /* if in 'active' time */
 
            if (((TC + RRange[0].TaskCount) != ARange[rindex].TaskCount) &&
                ((RPtr->TaskCount == 0) ||
                 (TC + RRange[0].TaskCount < RPtr->TaskCount) ||
                 (TC + RRange[rindex].TaskCount < ARange[rindex].TaskCount) ||
                 (ARange[rindex].TaskCount < RPtr->TaskCount)))
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
                    RPtr->Name, 
                    N->Name);
 
                  return(FAILURE);
                  }
                }    /* END if (N->RE[eindex].Time > ARange[rindex].StartTime) */
 
              ARange[rindex].StartTime = N->RE[eindex].Time + ResBuffer;
              ARange[rindex].TaskCount = RRange[0].TaskCount + TC;
              ARange[rindex].NodeCount = 1;
 
              if (DRes != NULL)
                MUCResGetMin(DRes,DRes,&CurRes);
              }  /* END if (((TC + ...))) */
            }
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
          }   /* END else (MUCResIsNeg(&CurRes) || ... */
        }
      else if ((ResourcesAdded == TRUE) && (ActiveRange == TRUE))
        {
        /* adjust taskcount, create new range */
 
        TC = MNodeGetTC(N,&CurRes,&N->CRes,&DedRes,&RPtr->DRes,MSched.Time);
 
        if (((TC + RRange[0].TaskCount) != ARange[rindex].TaskCount) && 
            ((RPtr->TaskCount == 0) ||
             (TC + RRange[0].TaskCount < RPtr->TaskCount) ||
             (ARange[rindex].TaskCount < RPtr->TaskCount)))
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
              RPtr->Name,
              N->Name);
 
            return(FAILURE);
            }
 
          ARange[rindex].StartTime = N->RE[eindex].Time + ResBuffer;
 
          ARange[rindex].TaskCount = RRange[0].TaskCount + TC;
 
          ARange[rindex].NodeCount = 1;
          }
        }
      else if ((ResourcesAdded == TRUE) && (ActiveRange == FALSE))
        {
        ResourcesAdded   = FALSE;
        ResourcesRemoved = FALSE; 
 
        if ((MUCResIsNeg(&CurRes) == FAILURE) &&
           (!(RPtr->Flags & (1 << mrfAdvRes)) ||
           (InclusiveReservationCount > 0)))
          {
          /* initiate active range */
 
          DBG(6,fSCHED) DPrint("INFO:     resources available at time %s during %s %s\n",
            MULToTString(N->RE[eindex].Time - MSched.Time),
            R->Name,
            (N->RE[eindex].Type == mreStart) ? "start" : "end");
 
          ARange[rindex].StartTime = MAX(RRange[0].StartTime,N->RE[eindex].Time) + ResBuffer;
 
          /* adjust taskcount */
 
          TC = MNodeGetTC(N,&CurRes,&N->CRes,&DedRes,&RPtr->DRes,MSched.Time);
 
          ARange[rindex].TaskCount = RRange[0].TaskCount + TC;
 
          ARange[rindex].NodeCount = 1;
 
          ActiveRange = TRUE;
 
          if (DRes != NULL)
            {
            MUCResGetMin(DRes,DRes,&CurRes);
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
        RPtr->Name,
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
        RPtr->Name,
        ARange[index].EndTime,
        RRange[0].StartTime,
        MSched.Time);
 
      ARange[index].TaskCount = 0;
      }
    else if (ARange[index].StartTime < RRange[0].StartTime)
      {
      DBG(6,fSCHED) DPrint("INFO:     ARange[%d] too early for job %s (S: %ld < S: %ld): truncating range\n",
        index,
        RPtr->Name,
        ARange[index].StartTime,
        RRange[0].StartTime);
 
      ARange[index].StartTime = RRange[0].StartTime;
      }
 
    if (ARange[index].TaskCount < RRange[0].TaskCount)
      {
      DBG(6,fSCHED) DPrint("INFO:     ARange[%d] too small for job %s (%d < %d):  removing range\n",
        index,
        RPtr->Name,
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
 
    if (RangeTime < Duration)
      {
      /* append prior ranges if immediate */
 
      for (index2 = index - 1;index2 >= 0;index2--)
        {
        if (ARange[index2].EndTime != ARange[index2 + 1].StartTime)
          break;
 
        RangeTime += (ARange[index2].EndTime - ARange[index2].StartTime);
 
        if (RangeTime >= Duration)
          break;
        }
 
      if (RangeTime < Duration)
        {
        /* append later ranges if immediate */
 
        for (index2 = index + 1;ARange[index2].EndTime != 0;index2++)
          {
          if (ARange[index2 - 1].EndTime != ARange[index2].StartTime)
            break; 
 
          RangeTime += (ARange[index2].EndTime - ARange[index2].StartTime);
 
          if (RangeTime >= Duration)
            break;
          }
        }
      }  /* END if (RangeTime < Duration) */
 
    if (ARange[index].StartTime > RRange[0].EndTime)
      {
      DBG(6,fSCHED) DPrint("INFO:     ARange[%d] (%ld -> %ld)x%d too late for job %s by %ld\n",
        index,
        ARange[index].StartTime,
        ARange[index].EndTime,
        ARange[index].TaskCount,
        RPtr->Name,
        ARange[index].StartTime - RRange[0].EndTime);
 
      /* can range be merged with previous range? */
 
      /* are long ranges or wide ranges sought? */
 
      if ((rindex > 0) &&
          (ARange[rindex - 1].EndTime == ARange[index].StartTime)
#ifndef __MSEEKLONGRANGE
       && (ARange[rindex].TaskCount >= ARange[rindex - 1].TaskCount)
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
        RPtr->Name,
        ARange[index].EndTime,
        RRange[0].EndTime,
        MSched.Time);
 
      ARange[index].EndTime = RRange[0].EndTime;
      }
 
    if (RangeTime < MAX(1,Duration))
      {
      DBG(6,fSCHED) DPrint("INFO:     ARange[%d] too short for job %s (MR: %ld < W: %ld):  removing range\n",
        index,
        RPtr->Name,
        RangeTime,
        Duration);
 
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
 
  if ((ARange[0].EndTime == 0) || (ARange[0].TaskCount == 0))
    {
    DBG(6,fSCHED) DPrint("INFO:     node %s unavailable for job %s at %s\n",
      N->Name,
      RPtr->Name,
      MULToTString(RRange[0].StartTime - MSched.Time));
 
    ARange[0].Affinity = nmUnavailable;
 
    return(FAILURE);
    }

  DBG(6,fSCHED)
    {
    for (rindex = 0;ARange[rindex].EndTime != 0;rindex++)
      {
      strcpy(WCString,MULToTString(ARange[rindex].StartTime - MSched.Time));
 
      DPrint("INFO:     node %s supports %d task%c of job %s for %s at %s\n",
        N->Name,
        ARange[rindex].TaskCount,
        (ARange[rindex].TaskCount == 1) ? ' ' : 's',
        RPtr->Name,
        MULToTString(ARange[rindex].EndTime - ARange[rindex].StartTime),
        WCString); 
      }
    }
 
  /* add required resources to min resource available */
 
  if (DRes != NULL)
    {
    MCResAdd(DRes,&N->CRes,&R->DRes,RRange[0].TaskCount,FALSE);
    }
 
  return(SUCCESS);
  }  /* END MResGetNRange() */ 






int MResToJob(

  mres_t *R,  /* I */
  mjob_t *J)  /* O */

  {
  if ((R == NULL) || (J == NULL))
    {
    return(FAILURE);
    }

  strcpy(J->Name,R->Name);

  /* copy creds */

  if (J->Cred.CL != NULL)
    memcpy(J->Cred.CL,R->CL,sizeof(macl_t) * MAX_MACL);

  /* clear res access list */

  J->RAList[0][0] = '\0';

  J->Request.TC = R->TaskCount;
  J->WCLimit        = R->EndTime - R->StartTime;
  J->SpecWCLimit[0] = J->WCLimit;

  /* NOTE:  support single req reservations */

  if (J->Req[0] != NULL)
    {
    mreq_t *RQ;

    RQ = J->Req[0];

    memcpy(&RQ->DRes,&R->DRes,sizeof(mcres_t));
    }  /* END if (J->Req[0] != NULL) */

  return(SUCCESS);
  }  /* END MResToJob() */




int MResAdjustTime(

  long Delta)
 
  {
  int rindex;
  int nindex;
  int reindex;
 
  mnode_t *N;
  mres_t  *R;

  const char *FName = "MResAdjustTime";

  DBG(4,fSTRUCT) DPrint("%s(%ld)\n",
    FName,
    Delta);
 
  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    R = MRes[rindex];

    if ((R == NULL) || (R->Name[0] == '\0'))
      break;

    if (R->Name[0] == '\1')
      continue;

    R->StartTime = MIN(R->StartTime + Delta,MAX_MTIME);
    R->EndTime   = MIN(R->EndTime + Delta,MAX_MTIME);
    }      /* END for (rindex) */
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;
 
    for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
      {
      if (N->RE[reindex].Type == mreNONE)
        break;
 
      N->RE[reindex].Time = MIN(N->RE[reindex].Time + Delta,MAX_MTIME);
      }  /* END for (reindex) */
    }    /* END for (nindex) */
 
  return(SUCCESS);
  }  /* END MResAdjustTime() */





int MResAddNode(
 
  mres_t  *R,         /* I (modified) */
  mnode_t *N,         /* I */
  int      TaskCount, /* I */
  int      Mode)      /* I */
 
  {
  int sindex;
  int FreeSlot;

  mcres_t DRes;

  const char *FName = "MResAddNode";
 
  DBG(5,fSTRUCT) DPrint("%s(%s,%s,%d,%d)\n",
    FName,
    (R != NULL) ? R->Name : "NULL",
    (N != NULL) ? N->Name : "NULL",
    TaskCount,
    Mode);

  if ((R == NULL) || (N == NULL))
    {
    return(FAILURE);
    }

  R->MTime    = MSched.Time; 
  N->ResMTime = MSched.Time;
 
  /* find matching/available reservation slot */

  FreeSlot = -1;
 
  for (sindex = 0;sindex < MSched.ResDepth;sindex++)
    {
    if (N->R[sindex] == NULL)
      {
      /* end of list located */

      break;
      }

    if ((FreeSlot == -1) && (N->R[sindex] == (mres_t *)1))
      {
      /* free slot located */

      FreeSlot = sindex;
      }

    if (N->R[sindex] == R)
      {
      /* specified reservation located */

      break;
      }
    }    /* END for (sindex) */

  if (FreeSlot != -1)
    {
    sindex = FreeSlot;
    }
  else if (sindex == MSched.ResDepth)
    {
    DBG(2,fSTRUCT) DPrint("WARNING:  reservation overflow on node %s (%d >= %d) - increase %s\n",
      N->Name,
      sindex,
      MSched.ResDepth,
      MParam[pResDepth]);

    return(FAILURE);
    }

  DBG(6,fSCHED)
    {
    MRECheck(N,"MResAddNode-Start",TRUE);
    }

  memcpy(&DRes,&R->DRes,sizeof(DRes));

  if ((R->Type == mrtUser) && (TaskCount > 1))
    {
    /* NOTE:  system reservations have DRes set to 'total' blocked resources */

    MCResAdd(&DRes,&N->CRes,&R->DRes,(TaskCount - 1),TRUE);

    TaskCount = 1;
    }

  switch(Mode)
    {
    case -1:  /* remove node */

      if (N->R[sindex] == R)
        {
        if ((TaskCount == -1) || (TaskCount >= N->RC[sindex]))
          {
          N->R[sindex] = (mres_t *)1;  

          /* remove RE */

          MRERelease(N->RE,sindex,MSched.ResDepth << 1);
          }
        else
          {
          N->RC[sindex] -= TaskCount;
          }
        }
      else
        {
        /* reservation does not exist on node */

        /* no action required */
        }

      break;

    case 0:

      if (N->R[sindex] == R)
        {
        /* set reservation taskcount */

        N->RC[sindex] = TaskCount;
        }
      else
        {
        /* create new reservation entry */

        N->R[sindex] = R;

        N->RC[sindex] = TaskCount;

        MREInsert(N->RE,R->StartTime,R->EndTime,sindex,&DRes,MSched.ResDepth << 1);       
        }

      break;

    case 1:

      if (N->R[sindex] == R)
        {
        /* set reservation taskcount */
 
        N->RC[sindex] += TaskCount;
        }
      else
        {
        /* create new reservation entry */
 
        N->R[sindex] = R;
 
        N->RC[sindex] = TaskCount;
 
        MREInsert(N->RE,R->StartTime,R->EndTime,sindex,&DRes,MSched.ResDepth << 1);
        }

      break;
    }  /* END switch(Mode) */
 
  DBG(6,fSCHED)
    {
    MRECheck(N,"MResAddNode-End",TRUE);
    }
 
  return(SUCCESS);
  }  /* END MResAddNode() */




int MNodeBuildRE(

  mnode_t *N,        /* I:  required */
  mres_t  *RPtr,     /* I:  optional */ 
  int      Rebuild)  /* I:  boolean  */
 
  {
  mres_t  *JR;
  mres_t  *R;
 
  mjob_t  *J;
 
  int     JRC;
  int     Overlap;
 
  int     crindex;
 
  int     jrindex1;
  int     jrindex2;
 
  int     oreindex;
  int     nreindex;
  int     creindex;
  int     jreindex;
  int     treindex;
 
  int     ResFound;  /* (boolean) */
 
  long    InitialJStartTime;
  long    CRStartTime;
  long    CREndTime;
  long    CROverlap;
  long    JRETime;
 
  short   CRC[MAX_MRES_DEPTH];
  short   CRI[MAX_MRES_DEPTH];
  mre_t   mCRE[MAX_MRES_DEPTH];
  short   CRESI[MAX_MRES_DEPTH];
  short   CREEI[MAX_MRES_DEPTH];
 
  mre_t  *CRE;  /* container reservation events */
  mre_t  *ORE;  /* old reservation events */
  mre_t  *NRE;  /* new reservation events */
  mre_t  *JRE;  /* job reservation events */
 
  mcres_t BR;
  mcres_t IBR;  /* blocked resources */
  mcres_t ZRes; /* empty CRes structure */
 
  int        OPC;
  int        NPC;

  const char *FName = "MNodeBuildRE";
 
  DBG(4,fCORE) DPrint("%s(%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    (RPtr != NULL) ? RPtr->Name : "NULL");

  if (N == NULL)
    {
    return(FAILURE);
    }

  /* must calculate BRes for each 'container' reservation at         */
  /* each 'job' event boundary.  collapse container reservations     */
  /* where possible using memcmp.  Insert all container reservation  */
  /* events at BRes calculation completion                           */

  /* NOTE:  new RE table consists of                                 *
   *          job reservation events                                 *
   *          container reservations events                          *
   *        in case of multiple containers                           *
   *          jobs extract resources from all inclusive reservations *
   *        in case of multiple competing containers                 *
   *          available resources assigned to reservations in priority order */

  /* if Rebuild is set, rebuild full RE table                        *
   * otherwise, build only events occuring in R timeframe            */

  memset(&ZRes,0,sizeof(ZRes));
 
  CRE = (mre_t *)malloc(sizeof(mre_t) * ((MAX_MRES_DEPTH << 1) + 1));
  ORE = (mre_t *)malloc(sizeof(mre_t) * ((MAX_MRES_DEPTH << 1) + 1));
  NRE = (mre_t *)malloc(sizeof(mre_t) * ((MAX_MRES_DEPTH << 1) + 1));
  JRE = (mre_t *)malloc(sizeof(mre_t) * ((MAX_MRES_DEPTH << 1) + 1));
 
  CRI[0] = -1;
 
  ResFound = FALSE;
 
  jreindex = 0;
 
  JRETime  = 0;
 
  for (nreindex = 0;N->RE[nreindex].Type != mreNONE;nreindex++)
    {
    if (nreindex >= (MSched.ResDepth << 1))
      break;
 
    if (N->R[N->RE[nreindex].Index]->Type == mrtJob)
      {
      JRETime = N->RE[nreindex].Time;
 
      /* job reservation located */
 
      memcpy(&JRE[jreindex],&N->RE[nreindex],sizeof(JRE[0]));
 
      if ((ResFound == FALSE) && (RPtr == N->R[N->RE[nreindex].Index]))
        {
        MTRAPRES(RPtr,FName);
 
        ResFound = TRUE;
        }
 
      jreindex++;
      }
    else
      {
      /* record each unique container reservation */ 
 
      for (crindex = 0;crindex < MSched.ResDepth;crindex++)
        {
        if (N->RE[nreindex].Index == CRI[crindex])
          {
          /* verify job res event matches CR split start time */
          /* if not, both events should block same (MAX) resources */
 
          if (JRETime != N->RE[nreindex].Time)
            {
            /* look for matching job re entries */
 
            for (treindex = nreindex + 1;N->RE[treindex].Type != mreNONE;treindex++)
              {
              if (N->RE[treindex].Time != N->RE[nreindex].Time)
                break;
 
              if (N->R[N->RE[nreindex].Index]->Type == mrtJob)
                {
                JRETime = N->RE[nreindex].Time;
 
                break;
                }
              }    /* END for (treindex) */
            }      /* END if (JRETime != N->RE[nreindex].Time) */
 
          if (JRETime != N->RE[nreindex].Time)
            {
            /* no matching job res event found */
 
            /* must update both start and end events */
 
            if (N->RE[nreindex].Type == mreStart)
              {
              OPC = (mCRE[crindex].DRes.Procs != -1) ? mCRE[crindex].DRes.Procs : N->CRes.Procs;
              NPC = (N->RE[nreindex].DRes.Procs != -1) ? N->RE[nreindex].DRes.Procs : N->CRes.Procs;
 
              if (OPC > NPC)
                {
                /* if previous event tuple is larger, locate outstanding end event */
 
                for (treindex = nreindex + 1;N->RE[treindex].Type != mreNONE;treindex++)
                  {
                  if ((N->RE[treindex].Index != N->RE[nreindex].Index) ||
                      (N->RE[treindex].Type != mreEnd))
                    {
                    continue;
                    }
 
                  /* end event found */

                  break;
                  }  /* END for (treindex) */
 
                memcpy(&N->RE[nreindex].DRes,&mCRE[crindex].DRes,sizeof(N->RE[0].DRes));
                memcpy(&N->RE[treindex].DRes,&mCRE[crindex].DRes,sizeof(N->RE[0].DRes));
 
                if ((N->RE[CRESI[crindex]].DRes.Procs == -1) || (N->RE[CREEI[crindex]].DRes.Procs == -1))
                  {
                  fprintf(stderr,"OUCH:  negative blocked resources detected\n");
                  }
                }
              else if (OPC < NPC)
                {
                memcpy(&mCRE[crindex].DRes,&N->RE[nreindex].DRes,sizeof(mCRE[crindex].DRes));

                memcpy(&N->RE[CRESI[crindex]].DRes,&mCRE[crindex].DRes,sizeof(N->RE[0].DRes));
                memcpy(&N->RE[CREEI[crindex]].DRes,&mCRE[crindex].DRes,sizeof(N->RE[0].DRes));
 
                if ((N->RE[CRESI[crindex]].DRes.Procs == -1) || (N->RE[CREEI[crindex]].DRes.Procs == -1))
                  {
                  fprintf(stderr,"OUCH:  negative blocked resources detected\n");
                  }
                }
 
              CRESI[crindex] = nreindex;
              }
            else
              {
              CREEI[crindex] = nreindex;
              }
            }    /* END if (JRETime != N->RE[nreindex].Time) */
 
          break;
          }      /* END if (N->RE[nreindex].Index == CRI[crindex]) */
 
        if (CRI[crindex] == -1)
          {
          CRI[crindex]   = N->RE[nreindex].Index;
          CRC[crindex]   = N->RC[N->RE[nreindex].Index];
          CRESI[crindex] = nreindex;
 
          memcpy(&mCRE[crindex],&N->RE[nreindex],sizeof(mCRE[crindex]));
 
          CRI[crindex + 1] = -1;
 
          break;
          }
        }    /* END for (crindex)               */
      }      /* END else (R[N->RE[nreindex]...) */
    }        /* END for (nreindex)              */
 
  JRE[jreindex].Type = mreNONE;
  JRE[jreindex].Index = 0;
 
  InitialJStartTime = (jreindex > 0) ?
    N->R[JRE[0].Index]->StartTime :
    MAX_MTIME;
 
  if (((RPtr != NULL) && (ResFound == FALSE)) ||
       (jreindex == 0))
    {
    /* requested reservation not found on node */
 
    /* issue, must update reservavation utilization in MResDestroy()
       situations where job RE's have just been removed.  ie,
       job RE impact on inclusive reservations must be handled
    */
 
    if (Rebuild == FALSE)
      { 
      return(SUCCESS);
      }
    }
 
  MTRAPNODE(N,FName);
 
  DBG(6,fSCHED)
    {
    MRECheck(N,"MNodeBuildRE-Start",TRUE);
    }
 
  memcpy(ORE,JRE,sizeof(ORE[0]) * (jreindex + 1));
 
  DBG(6,fSTRUCT) DPrint("INFO:     updating cr reservations for res '%s'\n",
    (RPtr != NULL) ? RPtr->Name : "NULL");
 
  /* CR: Container Reservation */
 
  for (crindex = 0;CRI[crindex] != -1;crindex++)
    {
    creindex = 0;
 
    memset(CRE,0,sizeof(mre_t) * (MAX_MRES_DEPTH << 1));
 
    R = N->R[CRI[crindex]];
 
    CRStartTime = MAX(MSched.Time,R->StartTime);
 
    CREndTime   = R->EndTime;
 
    /* blocked resources =~ dedicated resource per task * task count */
 
    memset(&IBR,0,sizeof(BR));

    /* IBR is total blocked resources */
 
    MCResAdd(&IBR,&N->CRes,&R->DRes,CRC[crindex],FALSE);

    if (CRStartTime < InitialJStartTime)
      {
      CRE[creindex].Index = CRI[crindex];
      CRE[creindex].Time  = CRStartTime;
      memcpy(&CRE[creindex].DRes,&IBR,sizeof(IBR));
      CRE[creindex].Type  = mreStart;
 
      creindex++;
 
      CRE[creindex].Index = CRI[crindex];
      CRE[creindex].Time  = MIN(CREndTime,InitialJStartTime);
      memcpy(&CRE[creindex].DRes,&IBR,sizeof(IBR));
      CRE[creindex].Type  = mreEnd;
 
      CRStartTime = InitialJStartTime;
 
      creindex++;
      }  /* END if (CRStartTime < InitialJStartTime) */
 
    for (jrindex1 = 0;JRE[jrindex1].Type != mreNONE;jrindex1++)
      {
      /* locate smallest event interval */
 
      for (jrindex2 = 0;JRE[jrindex2].Type != mreNONE;jrindex2++)
        {
        if (JRE[jrindex2].Time > CRStartTime)
          {
          CREndTime = MIN(CREndTime,JRE[jrindex2].Time);
 
          break;
          }
        }    /* END for (jrindex2) */ 
 
      Overlap =
        MIN(CREndTime,R->EndTime) -
        MAX(CRStartTime,R->StartTime);
 
      if (Overlap <= 0)
        {
        if (CRStartTime >= R->EndTime)
          break;
        else
          continue;
        }
 
      memcpy(&BR,&IBR,sizeof(BR));

      for (jrindex2 = 0;JRE[jrindex2].Type != mreNONE;jrindex2++)
        {
        if (JRE[jrindex2].Type != mreStart)
          continue;
 
        JR = N->R[JRE[jrindex2].Index];
 
        if (JRE[jrindex2].Time >= CREndTime)
          break;
 
        Overlap =
          MIN(CREndTime,JR->EndTime) -
          MAX(CRStartTime,JR->StartTime);
 
        if (Overlap <= 0)
          {
          if (JR->StartTime > CREndTime)
            break;
          else
            continue;
          }
 
        JRC = N->RC[JRE[jrindex2].Index];
 
        J = (mjob_t *)JR->J;
 
        /* if job reservation overlaps container reservation component */
 
        switch(R->Type)
          {
          case mrtUser:
 
            CROverlap =
              MIN(R->EndTime,JR->EndTime) -
              MAX(R->StartTime,JR->StartTime);
 
            if (MResCheckJAccess(R,J,CROverlap,NULL,NULL) == TRUE)
              {
              if (JRE[jrindex2].Type == mreStart)
                { 
                /* remove dedicated job resources from container blocked resources */

                MCResRemove(&BR,&N->CRes,&JR->DRes,JRC,FALSE);
                }
              else if (JRE[jrindex2].Type == mreEnd)
                {
                /* remove dedicated job resources from container blocked resources */
 
                MCResAdd(&BR,&N->CRes,&JR->DRes,JRC,FALSE);
                }
              }
 
            break;

          default:

            /* NO-OP */
 
            break;
          }     /* END switch (CR->Type) */
        }       /* END for (jrindex2)    */
 
      if (CRStartTime == R->EndTime)
        break;
 
      MUCResGetMax(&BR,&BR,&ZRes);
 
      if ((creindex > 0) &&
          (memcmp(&BR,&CRE[creindex - 1].DRes,sizeof(BR)) == 0))
        {
        /* merge ranges */

        CRE[creindex - 1].Time = CREndTime;
        }
      else
        {
        /* create new range */
 
        CRE[creindex].Index = CRI[crindex];
        CRE[creindex].Time  = CRStartTime;
        memcpy(&CRE[creindex].DRes,&BR,sizeof(CRE[0].DRes));
        CRE[creindex].Type  = mreStart;
 
        creindex++;
 
        CRE[creindex].Index = CRI[crindex];
        CRE[creindex].Time  = CREndTime;
        memcpy(&CRE[creindex].DRes,&BR,sizeof(CRE[0].DRes));
        CRE[creindex].Type  = mreEnd;
 
        creindex++;
        }
 
      if (CREndTime >= R->EndTime)
        break;
 
      /* advance start/end time for new CRes */
 
      CRStartTime = CREndTime;
      CREndTime   = R->EndTime;
      }         /* END for (jrindex1)    */ 
 
    if (creindex == 0)
      {
      /* no job overlap, use original CR events */
 
      CRE[creindex].Index = CRI[crindex];
      CRE[creindex].Time  = CRStartTime;
      memcpy(&CRE[creindex].DRes,&IBR,sizeof(CRE[0].DRes));
      CRE[creindex].Type  = mreStart;
 
      creindex++;
 
      CRE[creindex].Index = CRI[crindex];
      CRE[creindex].Time  = CREndTime;
      memcpy(&CRE[creindex].DRes,&IBR,sizeof(CRE[0].DRes));
      CRE[creindex].Type  = mreEnd;
 
      creindex++;
      }
 
    /* merge CR events with JR events */
 
    CRE[creindex].Type = mreNONE;
 
    creindex = 0;
    oreindex = 0;
    nreindex = 0;
 
    while ((CRE[creindex].Type != mreNONE) ||
           (ORE[oreindex].Type != mreNONE))
      {
      if ((creindex >= (MSched.ResDepth << 1)) ||
          (oreindex >= (MSched.ResDepth << 1)) ||
          (nreindex >= (MSched.ResDepth << 1)))
        {
        DBG(1,fSTRUCT) DPrint("ALERT:    node reservation event overflow (N: %d  C: %d  O: %d) - increase %s\n",
          nreindex,
          creindex,
          oreindex,
          MParam[pResDepth]);
 
        break;
        }
 
      if ((ORE[oreindex].Type == mreNONE) ||
          ((CRE[creindex].Type != mreNONE) &&
           (CRE[creindex].Time < ORE[oreindex].Time)))
        {
        memcpy(&NRE[nreindex],&CRE[creindex],sizeof(NRE[0]));
 
        nreindex++;
 
        creindex++;
        }
      else
        {
        memcpy(&NRE[nreindex],&ORE[oreindex],sizeof(NRE[0]));
 
        nreindex++;

        oreindex++;
        }
      }   /* END while ((CRE[creindex].Type != mreNONE) || (ORE[oreindex].Type != mreNONE)) */ 
 
    memcpy(ORE,NRE,(sizeof(ORE[0]) * nreindex));
 
    ORE[nreindex].Type = mreNONE;
    }     /* END for (crindex)     */
 
  memcpy(N->RE,ORE,sizeof(N->RE[0]) * (nreindex + 1));
 
  /* perform sanity check on RE table */
 
  CRStartTime = 0;
 
  for (nreindex = 0;N->RE[nreindex].Type != mreNONE;nreindex++)
    {
    if (nreindex >= (MSched.ResDepth << 1))
      break;
 
    DBG(6,fSTRUCT) DPrint("INFO:     N[%s]->RE[%02d] (%s %s)\n",
      N->Name,
      nreindex,
      N->R[N->RE[nreindex].Index]->Name,
      MULToTString(N->RE[nreindex].Time - MSched.Time));
 
    if (CRStartTime > N->RE[nreindex].Time)
      {
      DBG(2,fSTRUCT) DPrint("ALERT:    node %s RE table is corrupt.  RE[%d] '%s' at %s is out of time order\n",
        N->Name,
        nreindex,
        N->R[N->RE[nreindex].Index]->Name,
        MULToTString(N->RE[nreindex].Time - MSched.Time));
      }
 
    CRStartTime = N->RE[nreindex].Time;
    }   /* END for (reindex) */
 
  DBG(6,fSCHED)
    {
    MRECheck(N,"MNodeBuildRE-End",TRUE);
    }
 
  free(ORE);
  free(CRE);
  free(NRE);
  free(JRE);
 
  return(SUCCESS);
  }  /* END MNodeBuildRE() */




int MResCheckRAccess(

  mres_t *R,        /* I:  existing reservation */
  mres_t *ReqR,     /* I:  requesting reservation */
  long    Overlap,  /* I:  duration of overlap */
  int    *Same,     /* O:  boolean:  reservations are identical */
  char   *Affinity) /* O:  affinity of access */
 
  {
  int InclusiveReservation;
 
  const char *FName = "MResCheckRAccess";
 
  /* R:  existing job or user reservation */
 
  /* attributes of a META reservation */
 
  /*   container reservation          */
  /*   owned by user                  */
  /*   cannot be moved in time        */
  /*   queries should be allowed to request META res <X> or free resources */
 
  DBG(8,fSTRUCT) DPrint("%s(%s,%s,%ld,Same,Affinity)\n",
    FName,
    (R != NULL) ? R->Name : "NULL",
    (ReqR != NULL) ? ReqR->Name : "NULL",
    Overlap);

  if ((R == NULL) || (ReqR == NULL))
    {
    return(FAILURE);
    }

  if ((R->ExpireTime > 0) && (R->ExpireTime != MAX_MTIME))
    {
    return(FALSE);
    }
 
  if (Same != NULL) 
    *Same = FALSE;

  if ((R->Flags & (1 << mrfDedicatedResource)) || 
      (ReqR->Flags & (1 << mrfDedicatedResource)))
    {
    /* exclusive reservations */
 
    DBG(8,fSTRUCT) DPrint("INFO:     exclusive (two job reservations)\n");
 
    return(FALSE);
    }
 
  MACLCheckAccess(R->ACL,ReqR->CL,Affinity,&InclusiveReservation);

  return(InclusiveReservation);
  }  /* END MResCheckRAccess() */



int MResGetFeasibleTasks(

  mres_t   *R,
  mrange_t *ARange)

  {
  if (R == NULL)
    return(FAILURE);

  return(SUCCESS);
  }  /* MResGetFeasibleTasks() */




int MResUpdateStats()

  {
  mres_t  *R;
  mnode_t *N;
 
  int     nindex;
  int     reindex;
  int     rindex;
 
  int     RC;
  int     PC;
 
  int     BlockedProcs;
 
  mcres_t *BR;

  double  IPS;
 
  double  interval;
  double  fsusage;

  const char *FName = "MResUpdateStats";

  DBG(3,fSTRUCT) DPrint("%s()\n",
    FName);
 
  interval = (double)MSched.Interval / 100.0;
 
  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    R = MRes[rindex];
 
    if ((R == NULL) || (R->Name[0] == '\0'))
      break;
 
    if (R->Name[0] == '\1')
      continue;
 
    if (MSched.Time < R->StartTime)
      continue;
 
    if (MSched.Time > R->EndTime)
      continue; 

    if (R->IsActive == FALSE)
      {
      /* reservation is now active */

      R->IsActive = TRUE;
      }  /* END if (R->IsActive == FALSE) */
 
    if (X.XResUpdate != (int (*)())0)
      {
      (*X.XResUpdate)(X.xd,R);
      }
    }      /* END for (rindex) */
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];
 
    if ((N == NULL) || (N->Name[0] == '\0'))
      break;
 
    if (N->Name[0] == '\1')
      continue;
 
    MTRAPNODE(N,FName);
 
    for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
      {
      if (N->RE[reindex].Type == mreNONE)
        break;
 
      if (N->RE[reindex].Time > MSched.Time)
        break;
 
      if (N->RE[reindex].Type != mreStart)
        continue;
 
      R = N->R[N->RE[reindex].Index];
 
      if (R->Type == mrtJob)
        continue;
 
      if (MSched.Time > R->EndTime)
        continue; 
 
      RC = N->RC[N->RE[reindex].Index];
 
      PC = RC * ((R->DRes.Procs != -1) ? R->DRes.Procs : N->CRes.Procs);
 
      BR = &N->RE[reindex].DRes;  /* total resources dedicated */
 
      BlockedProcs = (BR->Procs == -1) ? N->CRes.Procs : BR->Procs;
 
      DBG(9,fSTRUCT) DPrint("INFO:     checking utilization for MNode[%03d] '%s', slot %d\n",
        nindex,
        N->Name,
        N->RE[reindex].Index);
 
      DBG(5,fSTRUCT) DPrint("INFO:     updating usage stats for reservation %s on node '%s'\n",
        R->Name,
        N->Name);

      IPS = (interval * BlockedProcs);
 
      R->CAPS += (interval * (PC - BlockedProcs));
      R->CIPS += IPS;

      switch(MPar[0].FSC.FSPolicy)
        {
        case fspDPES:

          {
          double PEC;

          MResGetPE(BR,&MPar[0],&PEC);
 
          fsusage = interval * PEC;
          }
 
          break;
 
        case fspUPS:
 
          fsusage = 0.0;
 
          break;
 
        case fspDPS:
        default:
 
          fsusage = IPS;
 
          break;
        }  /* END switch(FS[0].FSPolicy) */

      if (R->A != NULL)
        R->A->F.FSUsage[0] += fsusage;

      if (R->G != NULL)
        R->G->F.FSUsage[0] += fsusage;

      if (R->U != NULL)
        R->U->F.FSUsage[0] += fsusage;
      }    /* END for reindex */
    }      /* END for nindex  */
 
  return(SUCCESS);
  }  /* END MResUpdateStats() */




int MResChargeAllocation(

  mres_t *RS,
  int     Mode)
 
  {
  int rindex;
  enum MHoldReasonEnum Reason;
 
  mres_t *R;
 
  long WallTime;

  const char *FName = "MResChargeAllocation";
 
  DBG(3,fSTAT) DPrint("%s(%s,%d)\n",
    FName,
    (RS != NULL) ? RS->Name : "NULL",
    Mode);
 
  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    R = MRes[rindex];

    if ((R == NULL) || (R->Name[0] == '\0'))
      break;

    if (R->Name[0] == '\1')
      continue;

    if ((RS != NULL) && (R != RS))
      continue;
 
    if (R->Type == mrtJob)
      continue;
 
    if ((R->A == NULL) || !strcmp(R->A->Name,NONE))
      {
      continue;
      }
 
    if (R->AllocResPending == TRUE)
      {
      if (R->StartTime > MSched.Time)
        WallTime = MIN(MAM[0].FlushInterval,R->EndTime - R->StartTime);
      else
        WallTime = MIN(MAM[0].FlushTime,R->EndTime) - MSched.Time;
 
      if (MAMAllocRReserve(
            &MAM[0],
            R->Name,
            MSched.Time,
            R->A->Name,
            R->AllocPC,
            R->NodeCount,
            WallTime,
            0, 
            MDEF_NODETYPE,
            &Reason) == FAILURE)
        {
        DBG(1,fSCHED) DPrint("ALERT:    cannot reserve allocation for %d procs for reservation %s\n",
          R->AllocPC,
          R->Name);
 
        if (Reason == mhrNoFunds)
          MResDestroy(&R);
 
        continue;
        }
 
      R->AllocResPending = FALSE;
 
      continue;
      }
 
    if (R->StartTime > MSched.Time)
      {
      /* reservation in future */
 
      continue;
      }
 
    if (R->CIPS <= 0.0)
      {
      if (Mode == 2)
        {
        if (MAMAllocResCancel(
              (R->A != NULL) ? R->A->Name : NONE,
              R->Name,
              "reservation released",
              NULL,
              &Reason) == FAILURE)
          {
          DBG(1,fSTAT) DPrint("ALERT:    cannot cancel allocation reservation for reservation %s\n",
            R->Name);
          }
        }
      else if (MAMAllocRReserve(
            &MAM[0],
            R->Name,
            MSched.Time,
	    (R->A != NULL) ? R->A->Name : NONE, 
            R->AllocPC,
            R->NodeCount,
            MAX(1,MIN(R->EndTime,MAM[0].FlushTime) - MAX(MSched.Time,R->StartTime)),
            0,
            MDEF_NODETYPE,
            &Reason) == FAILURE)
        {
        DBG(3,fUI) DPrint("WARNING:  cannot reserve allocation for %s on %d proc%s\n",
          R->Name,
          R->AllocPC,
          (R->AllocPC == 1) ? "" : "s");
 
        if (Reason == mhrNoFunds)
          { 
          MResDestroy(&R);
          }
        }
 
      continue;
      }  /* END if (R->CIPS <= 0.0) */
 
    if (MAMAllocRDebit(&MAM[0],R,&Reason,NULL) == SUCCESS)
      {
      R->TIPS += R->CIPS;
      R->TAPS += R->CAPS;
 
      R->CIPS = 0.0;
      R->CAPS = 0.0;
 
      /* create new allocation reservation */
 
      if (Mode == 0)
        {
        if (MAMAllocRReserve(
              &MAM[0],
              R->Name,
              MSched.Time,
	      (R->A != NULL) ? R->A->Name : NONE, 
              R->AllocPC, 
              R->NodeCount,
              MAX(1,MIN(R->EndTime,MAM[0].FlushTime) - MAX(MSched.Time,R->StartTime)),
              0,
              MDEF_NODETYPE,
              &Reason) == FAILURE)
          {
          DBG(3,fUI) DPrint("WARNING:  cannot reserve allocation for %s on %d proc%s\n",
            R->Name,
            R->AllocPC,
            (R->AllocPC == 1) ? "" : "s");
 
          if (Reason == mhrNoFunds)
            {
            MResDestroy(&R);
            }
          }
        }    /* END if (Mode == 0) */
      }
    else
      {
      DBG(1,fSTAT) DPrint("ALERT:    cannot charge %6.2f PS to account %s for reservation %s\n", 
        R->CIPS,
	(R->A != NULL) ? R->A->Name : NONE, 
        R->Name);
      }
    }    /* END for (rindex) */
 
  return(SUCCESS);
  }  /* END MResChargeAllocation() */




int MRLAND(

  mrange_t *RDst,
  mrange_t *R1,
  mrange_t *R2)

  {
  int index1;
  int index2;
  int cindex;
 
  mrange_t C[MAX_MRANGE];
 
  long ETime = 0;
  long STime = 0;

  long ETime2 = 0;
  long STime2 = 0;
 
  /* step through range events */
  /* if R2 not active, C not active */

  index1 = 0;
  index2 = 0;
  cindex = 0;

  if (R1[index1].EndTime != 0)
    {
    /* ignore early ranges */
 
    while ((R2[index2].EndTime < R1[index1].StartTime) &&
           (R2[index2 + 1].EndTime != 0))
      {
      index2++;
      }
 
    if (R2[index2].EndTime >= R1[index1].StartTime)
      {
      STime2 = R2[index2].StartTime;
 
      /* merge conjoined mask ranges */
 
      while ((R2[index2].EndTime == R2[index2 + 1].StartTime) &&
             (R2[index2 + 1].EndTime != 0))
        {
        index2++;
        }
 
      ETime2 = R2[index2].EndTime;
      }
    }  /* END if (R1[index1].EndTime != 0) */

  for (index1 = 0;R1[index1].EndTime != 0;index1++)
    {
    if (R2[index2].EndTime == 0)
      break;

    /* process R1 range */

    while (STime2 <= R1[index1].EndTime)
      {
      STime = STime2;
      ETime = ETime2;

      /* determine next range */

      /* if current mask range endtime is exceeded by R1 range endtime */

      if (ETime <= R1[index1].EndTime)
        {
        index2++;

        if (R2[index2].EndTime != 0)
          {
          /* ignore early ranges */
 
          while ((R2[index2].EndTime < R1[index1].StartTime) &&
                 (R2[index2 + 1].EndTime != 0))
            {
            index2++;
            }

          if (R2[index2].EndTime < R1[index1].StartTime)
            {
            /* end of R2 list detected.  no overlapping ranges */
   
            STime2 = MAX_MTIME + 1;
            }
          else
            { 
            STime2 = R2[index2].StartTime;
         
            /* merge conjoined mask ranges */
 
            while ((R2[index2].EndTime == R2[index2 + 1].StartTime) &&
                   (R2[index2 + 1].EndTime != 0))
              {
              index2++;
              }
 
            ETime2 = R2[index2].EndTime;
            }
          }
        else
          {
          STime2 = MAX_MTIME + 1;    
          }
        }

      /* ignore early ranges */

      if (ETime < R1[index1].StartTime)
        {
        continue;
        }

      /* overlapping range located */

      C[cindex].StartTime = MAX(STime,R1[index1].StartTime);
      C[cindex].EndTime   = MIN(ETime,R1[index1].EndTime); 
      C[cindex].TaskCount = R1[index1].TaskCount;
      C[cindex].NodeCount = R1[index1].NodeCount;

      cindex++;
 
      if (cindex >= (MAX_MRANGE - 1))
        break;

      if (R1[index1].EndTime < ETime)
        {
        /* advance R1 range */

        break;
        }
      }  /* END while (STime2 <= R1[index1].EndTime) */
    } /* END for (index1) */

  /* terminate range */

  C[cindex].EndTime = 0;

  if (RDst != NULL) 
    memcpy(RDst,C,MAX_MRANGE);
 
  return(SUCCESS);
  }  /* END MRLAND() */




int MNodeUpdateResExpression(
 
  mnode_t *N)  /* I */
 
  {
  int    rindex;
  int    nrindex;
  int    srindex;
 
  short  NodeList[2];
 
  int    NodeCount;
  int    TC;
 
  char   Buffer[MAX_MLINE];
 
  mres_t *R;

  const char *FName = "MNodeUpdateResExpression";

  DBG(3,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL");

  /* NOTE:  empty standing reservations should be supported */
 
  if (N == NULL)
    {
    return(FAILURE);
    }
 
  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    R = MRes[rindex];

    if ((R == NULL) || (R->Name[0] == '\0'))
      break;

    if (R->Name[0] == '\1')
      continue;

    if (R->StartTime == 0)
      continue;
 
    if ((R->Type == mrtJob) || (R->RegEx == NULL))
      continue; 
 
    DBG(5,fSTRUCT) DPrint("INFO:     checking R[%03d]: '%s'  end: %s\n",
      rindex,
      R->Name,
      MULToTString(R->EndTime - MSched.Time));

    if ((R->MaxTasks > 0) && (R->TaskCount >= R->MaxTasks))
      {
      continue;
      }

    /* verify node is not already connected to reservation */
 
    for (nrindex = 0;N->R[nrindex] != NULL;nrindex++)
      {
      if (N->R[nrindex] == R)
        break;
      }  /* END for (nrindex) */
 
    if (N->R[nrindex] == R)
      {
      /* reservation already exists on node */
 
      continue;
      }
 
    strcpy(Buffer,N->Name);
 
    if (MUREToList(
         R->RegEx,
         mxoNode,
         -1,
         NodeList,
         &NodeCount,
         Buffer) == FAILURE)
      {
      DBG(2,fSTRUCT) DPrint("ERROR:    cannot expand regex '%s' to check node '%s'\n",
        R->RegEx,
        N->Name);
 
      continue;
      }
 
    /* node belongs to reservation */ 
 
    if (NodeCount == 0)
      {
      continue;
      }

    TC = MNodeGetTC(
        N,
        &N->CRes,
        &N->CRes,
        &N->DRes,
        &R->DRes,
        MAX_MTIME);

    TC = MAX(TC,1);

    if (R->MaxTasks > 0)
      TC = MIN(TC,R->MaxTasks - R->TaskCount);

    /* find first available reservation slot on node */
 
    if (MResAddNode(R,N,TC,0) == FAILURE)
      {
      DBG(1,fSTRUCT) DPrint("ALERT:    cannot add node %s to reservation %s\n",
        N->Name,
        R->Name);
 
      continue;
      }
 
    R->NodeCount ++;
 
    R->TaskCount += TC;
 
    R->AllocPC += (R->DRes.Procs == -1) ?
      N->CRes.Procs :
      (TC * R->DRes.Procs);
 
    if (R->PtIndex == -1)
      R->PtIndex = N->PtIndex;
    else if (R->PtIndex != N->PtIndex)
      R->PtIndex = 0;
    }      /* END for (rindex)        */

  /* NOTE:  examine all empty standing reservations */

  for (srindex = 0;srindex < MAX_MSRES;srindex++)
    {
    sres_t    *SR;
    int        nindex;
    mnalloc_t *HL;
 
    SR = &SRes[srindex];

    if (SR->Name[0] == '\0')
      break;

    if (SR->R[0] != NULL)
      continue;

    if (SR->HostList == NULL)
      {
      continue;
      }

    HL = (mnalloc_t *)SR->HostList;

    /* rebuild SR if N in SR hostlist */

    for (nindex = 0;nindex < MAX_MNODE;nindex++)
      {
      if (HL[nindex].N == N)
        {
        /* node included in SR hostlist */

        MSRUpdate(SR);

        break;
        }

      if (HL[nindex].N == NULL)
        break;
      }  /* END for (nindex) */
    }  /* END for (srindex) */

  return(SUCCESS);
  }  /* END MNodeUpdateResExpression() */




int MResCheckJAccess(
 
  mres_t *R,         /* I (existing reservation) */
  mjob_t *J,         /* I (requesting consumer) */
  long    Overlap,   /* I */
  int    *Same,      /* O (optional) */
  char   *Affinity)  /* O (optional) */
 
  {
  int IsInclusive;
 
  char tmpAffinity;
 
  mjob_t *RJ;
 
  char  *tail;
 
  mreq_t *RQ;
 
  /* R:  existing job or user reservation */
  /* J:  job wrapper for new reservation  */
 
  /* attributes of a grid reservation */
 
  /*   container reservation          */
  /*   owned by user                  */
  /*   cannot be moved in time        */
  /*   queries should be allowed to request META res <X> or free resources */

  const char *FName = "MResCheckJAccess";
 
  DBG(8,fSTRUCT) DPrint("%s(%s,%s,%ld,Same,Affinity)\n",
    FName,
    (R != NULL) ? R->Name : "NULL",
    (J != NULL) ? J->Name : "NULL",
    Overlap);

  if ((R == NULL) || (J == NULL))
    {
    return(FAILURE);
    }
 
  IsInclusive = FALSE;
  tmpAffinity = nmPositiveAffinity;
 
  if (Same != NULL)
    *Same = FALSE; 

  if (J->RAList != NULL)
    {
    int aindex;
 
    for (aindex = 0;J->RAList[aindex][0] != '\0';aindex++)
      {
      if (!strcmp(J->RAList[aindex],R->Name) ||
          !strcmp(J->RAList[aindex],ALL) ||
         (!strcmp(J->RAList[aindex],"[ALLJOB]") && (R->J != NULL)))
        {
        /* reservation access is granted */
 
        if (Affinity != NULL)
          *Affinity = nmPositiveAffinity;
 
        return(TRUE);
        }
      }    /* END for (aindex) */
    }
 
  if (R->Type == mrtJob)
    {
    RJ = (mjob_t *)R->J;
 
    if (!(J->Flags & (1 << mjfResMap)))
      {
      if (!strcmp(R->Name,J->Name))
        {
        /* job located its own reservation */
 
        if (Affinity != NULL)
          *Affinity = nmPositiveAffinity;
 
        return(TRUE);
        }
      else
        {
        /* two job reservations (ALWAYS EXCLUSIVE) */
 
        DBG(8,fSTRUCT) DPrint("INFO:     exclusive (two job reservations)\n");
 
        return(FALSE);
        }
      }
    }
  else 
    {
    RJ = NULL;
    }
 
  RQ = J->Req[0];  /* FIXME:  only handles one req */
 
  if ((R->Flags & (1 << mrfByName)) || (J->Flags & (1 << mjfByName)))
    {
    if (R->Type != mrtJob)
      {
      if (strcmp(J->ResName,R->Name) != 0)
        {
        /* reservation is not specifically requested */
 
        DBG(8,fSTRUCT) DPrint("INFO:     exclusive (not by name)\n");
 
        return(FALSE);
        }
      }
    else
      {
      if ((RJ == NULL) || strcmp(J->Name,RJ->Name))
        {
        /* reservation is not specificailly requested */
 
        DBG(8,fSTRUCT) DPrint("INFO:     exclusive (not by name)\n");
 
        return(FALSE);
        }
      }
    }
 
  if ((R->Type == mrtUser) && (J->Flags & (1 << mjfResMap)))
    {
    /* two user reservations */
 
    if (R->Flags & (1 << mrfStandingRes))
      {
      /* check if both standing reservations are same */
 
      if ((tail = strrchr(R->Name,'.')) != NULL)
        { 
        if (!strncmp(R->Name,J->Name,(tail - R->Name)))
          {
          /* same standing reservation */
 
          if (Same != NULL)
            *Same = TRUE;
 
          if (Affinity != NULL)
            *Affinity = nmPositiveAffinity;
 
          return(TRUE);
          }
        }
      }
 
    if ((R->Flags & (1 << mrfDedicatedResource)) ||
       !(J->Flags & (1 << mjfSharedResource)))
      {
      DBG(8,fSTRUCT) DPrint("INFO:     exclusive (not shared resource)\n");
 
      return(FALSE);
      }
    else
      {
      /* at least one reservation will share resources */
 
      if (Affinity != NULL)
        *Affinity = nmPositiveAffinity;
 
      return(TRUE);
      }
    }
 
  /* check 'ResName' specification */
 
  if ((J->ResName[0] != '\0') && strcmp(R->Name,J->ResName))
    {
    DBG(8,fSTRUCT) DPrint("INFO:     exclusive (not 'ResName' reservation)\n");
 
    return(FALSE);
    } 
 
  if (R->Type == mrtJob)
    {
    if ((RJ->ResName[0] != '\0') && strcmp(J->Name,RJ->ResName))
      {
      DBG(8,fSTRUCT) DPrint("INFO:     exclusive (not 'ResName' reservation)\n");
 
      return(FALSE);
      }
    }
 
  if (J->Cred.CL == NULL)
    {
    DBG(0,fSTRUCT) DPrint("WARNING:  job '%s' has NULL cred list\n",
      J->Name);
 
    return(FALSE);
    }
 
  /* one reservation/one job */
 
  /* job seeks access to reservation */
 
  if ((R->ExpireTime <= 0) || (R->ExpireTime == MAX_MTIME))
    {
    long OTime;
    int  aindex;

    /* NOTE:  temporarily adjust 'overlap' time */

    OTime = -1;

    for (aindex = 0;aindex < MAX_MACL;aindex++)
      {
      if (J->Cred.CL[aindex].Type == maDuration)
        {
        OTime = J->Cred.CL[aindex].Value;

        J->Cred.CL[aindex].Value = Overlap;
       
        break;
        }
      }    /* END for (aindex) */

    if (aindex < MAX_MACL)
      {
      if (J->Cred.CL[aindex].Type == maNONE)
        {
        /* create new time CL */

        J->Cred.CL[aindex].Type     = maDuration;
        J->Cred.CL[aindex].Value    = Overlap;
        J->Cred.CL[aindex].Cmp      = mcmpEQ;
        J->Cred.CL[aindex].Affinity = nmPositiveAffinity;

        OTime = -1;
        }
      }

    /* NOTE:  res wrappers gain access if R(job) is inclusive on J(res) */

    if (J->SpecFlags & (1 << mjfResMap))
      MACLCheckAccess(J->Cred.CL,R->CL,Affinity,&IsInclusive);
    else
      MACLCheckAccess(R->ACL,J->Cred.CL,Affinity,&IsInclusive);

    if (aindex < MAX_MACL)
      {
      /* restore time CL */

      if (OTime == -1)
        J->Cred.CL[aindex].Type  = maNONE;
      else
        J->Cred.CL[aindex].Value = OTime;
      }
    }  /* END if ((R->ExpireTime <= 0) || (R->ExpireTime == MAX_MTIME)) */
 
  return(IsInclusive);
  }  /* END MResCheckJAccess() */




int MResCheckStatus(

  mres_t *SR)  /* I */
 
  {
  int rindex;
  int nindex;
  int index;
  int reindex;
  int reindex2;
  int RIndex;
 
  int Offset;
 
  mnode_t *N;
 
  mres_t  *R;
 
  mjob_t  *J;

  const char *FName = "MResCheckStatus";
 
  DBG(4,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    (SR != NULL) ? SR->Name : "NULL"); 
 
  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    R = MRes[rindex];

    if ((R == NULL) || (R->Name[0] == '\0'))
      break;

    if (R->Name[0] == '\1')
      continue;

    if ((SR != NULL) && (R != SR))
      continue;
 
    if (R->StartTime == 0)
      continue;
 
    DBG(5,fSTRUCT) DPrint("INFO:     checking R[%03d]: '%s'  end: %s\n",
      rindex,
      R->Name,
      MULToTString(R->EndTime - MSched.Time));
 
    if ((MSched.Time > R->EndTime) ||
       ((R->ExpireTime > 0) && (MSched.Time > R->ExpireTime)))
      {
      J = (mjob_t *)R->J;
 
      if ((R->Type == mrtJob) && (J != NULL) &&
         ((J->State == mjsStarting) || (J->State == mjsRunning)))
        {
        /* extend reservation */
 
        DBG(1,fSTRUCT) DPrint("INFO:     extending reservation for overrun job %s by %ld seconds\n",
          J->Name,
          MSched.Time + MSched.RMPollInterval - R->EndTime);
 
        R->EndTime = MSched.Time + MSched.RMPollInterval;
        }
      else
        {
        DBG(1,fSTRUCT) DPrint("INFO:     clearing expired%s reservation[%03d] '%s' on iteration %d  (start: %ld end: %ld)\n",
          ((R->ExpireTime > 0) && (R->ExpireTime < MAX_MTIME)) ? " courtesy" : "", 
          rindex,
          R->Name,
          MSched.Iteration,
          R->StartTime - MSched.Time,
          R->EndTime - MSched.Time);
 
        MResDestroy(&R);
        }
      }    /* END if ((MSched.Time) || ... ) */
    }      /* END for (rindex) */

  /* evaluate node RE structures */
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;
 
    for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
      {
      if (N->RE[reindex].Type == mreNONE)
        break;
 
      if (N->RE[reindex].Type != mreEnd)
        continue;
 
      if (N->RE[reindex].Time <= MSched.Time)
        {
        /* RE end reached.  Locate associated RE start */
 
        RIndex = N->RE[reindex].Index; 
 
        DBG(6,fSCHED)
          {
          MRECheck(N,"MResCheckStatus-Start",TRUE);
          }
 
        for (reindex2 = reindex;reindex2 >= 0;reindex2--)
          {
          if (N->RE[reindex2].Index != RIndex)
            continue;
 
          if (N->RE[reindex2].Type == mreStart)
            break;
          }  /* END for (reindex2) */
 
        if ((N->RE[reindex2].Type != mreStart) ||
            (N->RE[reindex2].Index != RIndex))
          {
          DBG(1,fSTRUCT) DPrint("ERROR:    RE table on node '%s' is corrupt.  (cannot locate start of res '%s'\n",
            N->Name,
            N->R[RIndex]->Name);
 
          continue;
          }
 
        /* remove RE entries from reindex2 to reindex */
 
        Offset = 0;
 
        for (index = reindex2;index < (MSched.ResDepth << 1);index++)
          {
          if (N->RE[index].Type == mreNONE)
            break;
 
          if (N->RE[index].Index == RIndex) 
            {
            if (Offset < 2)
              {
              Offset++;
 
              continue;
              }
            }
 
          if (Offset > 0)
            memcpy(&N->RE[index - Offset],&N->RE[index],sizeof(mre_t));
          }  /* END for (index) */
 
        N->RE[index - Offset].Type = mreNONE;
 
        /* remove reservation if all RE pointers expired */
 
        for (index = 0;index < (MSched.ResDepth << 1);index++)
          {
          if (N->RE[index].Type == mreNONE)
            break;
 
          if (N->RE[index].Index == RIndex)
            break;
          }
 
        if (N->RE[index].Type == mreNONE)
          {
          /* no event references to reservation */
 
          /* NOTE:  assumes flat reservation model */
 
          MResDestroy(&N->R[RIndex]);
          } 
 
        DBG(6,fSCHED)
          {
          MRECheck(N,"MResCheckStatus-End",TRUE);
          }
        }    /* END if (N->RE[reindex].Time < MSched.Time) */
      }      /* END for (reindex) */
    }        /* END for (nindex)  */
 
  return(SUCCESS);
  }  /* END MResCheckStatus() */




int MRECheck(
 
  mnode_t *SN,       /* I */
  char    *Location, /* O */
  int      Force)    /* I (boolean) */
 
  {
  int reindex;
  int rindex;
 
  int nindex;
  int cindex;
 
  int IsCorrupt;
 
  int StartEventFound;
 
  mnode_t *N;
 
  mcres_t ZRes;
 
  int Count[MAX_MRES_DEPTH];
  int TCount[MAX_MRES_DEPTH];
 
  int RC;
  int RIndex;
 
  mres_t *R;

  const char *FName = "MRECheck";
 
  DBG(4,(fSCHED|fSTRUCT)) DPrint("%s(%s,%s,%s)\n",
    FName,
    (SN != NULL) ? SN->Name : "",
    Location,
    (Force == TRUE) ? "FORCE" : "NOFORCE");
 
  memset(&ZRes,0,sizeof(ZRes));
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    if ((SN != NULL) && (SN != N))
      continue;
 
    memset(Count,0,sizeof(Count));
    memset(TCount,0,sizeof(TCount));
 
    IsCorrupt = FALSE;
 
    /* check reservation table */
 
    for (rindex = 0;rindex < MSched.ResDepth;rindex++)
      {
      if (N->R[rindex] == NULL)
        break;
 
      if (N->R[rindex] == (mres_t *)1)
        continue;
 
      StartEventFound = FALSE;
 
      for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
        {
        if (N->RE[reindex].Type == mreNONE)
          break;
 
        if ((N->RE[reindex].Index == rindex) &&
            (N->RE[reindex].Type == mreStart))
          {
          StartEventFound = TRUE;
 
          break;
          } 
        }  /* END for (reindex) */
 
      if (StartEventFound == FALSE)
        {
        IsCorrupt = TRUE;
        }
      }    /* END for (rindex)  */
 
    /* check reservation event table */
 
    for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
      {
      if (N->RE[reindex].Type == mreNONE)
        break;
 
      RIndex = N->RE[reindex].Index;
      R      = N->R[RIndex];
 
      if (N->RE[reindex].Type == mreStart)
        Count[RIndex]++;
      else if (N->RE[reindex].Type == mreEnd)
        Count[RIndex]--;
 
      TCount[RIndex]++;
 
      RC = N->RC[RIndex];
 
      if ((R == NULL) || (R == (mres_t *)1))
        {
        IsCorrupt = TRUE;
        }
      else if ((R->Type == mrtUser) &&
               (R->StartTime <= MSched.Time) &&
               (N->RE[reindex].DRes.Procs * RC < R->DRes.Procs * RC - N->DRes.Procs))
        {
        IsCorrupt = TRUE; 
        }
/*
      else if ((R->Type == mrtUser) &&
               (R->StartTime <= MSched.Time) &&
               (N->RE[reindex].DRes.Procs == 0) &&
               (N->DRes.Procs == 0))
        {
        IsCorrupt = TRUE;
        }
*/
 
      if ((Count[RIndex] < 0) || (Count[RIndex] > 1))
        {
        IsCorrupt = TRUE;
        }
      }  /* END for (reindex) */
 
    if (IsCorrupt == FALSE)
      {
      for (cindex = 0;cindex < MSched.ResDepth;cindex++)
        {
        if (N->R[cindex] == NULL)
          break;
 
        if (N->R[cindex] == (mres_t *)1)
          continue;
 
        if ((Count[cindex] != 0) || (TCount[cindex] < 2))
          {
          IsCorrupt = TRUE;
 
          break;
          }
        }  /* END for (cindex) */
      }
 
    if ((IsCorrupt == TRUE) || (Force == TRUE)) 
      {
      if (IsCorrupt == TRUE)
        {
        DBG(0,(fSCHED|fSTRUCT)) DPrint("ALERT:    corruption found on iteration %d in location %s on node %s\n",
          MSched.Iteration,
          Location,
          N->Name);
        }
 
      /* check reservation table */
 
      for (rindex = 0;rindex < MSched.ResDepth;rindex++)
        {
        if (N->R[rindex] == NULL)
          break;
 
        if (N->R[rindex] == (mres_t *)1)
          continue;
 
        StartEventFound = FALSE;
 
        for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
          {
          if (N->RE[reindex].Type == mreNONE)
            break;
 
          if ((N->RE[reindex].Index == rindex) &&
              (N->RE[reindex].Type == mreStart))
            {
            StartEventFound = TRUE;
 
            break;
            }
          }  /* END for (reindex) */
 
        if (StartEventFound == FALSE) 
          {
          DBG(1,fSTRUCT) DPrint("ALERT:    R[%03d] '%s' has no start event\n",
            rindex,
            N->R[rindex]->Name);
          }
        }    /* END for (rindex)  */
 
      memset(Count,0,sizeof(Count));
 
      for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
        {
        if (N->RE[reindex].Type == mreNONE)
          break;
 
        RIndex = N->RE[reindex].Index;
        R      = N->R[RIndex];
        RC     = N->RC[RIndex];
 
        if ((R == NULL) || (R == (mres_t *)1))
          {
          DBG(1,(fSTRUCT|fSCHED)) DPrint("ALERT:    bad pointer in RE[%3d] %c %2d R: %p\n",
            reindex,
            (N->RE[reindex].Type == mreStart) ? 'S' : 'E',
            RIndex,
            R);
          }
        else
          {
          DBG(7,(fSTRUCT|fSCHED)) DPrint("INFO:     N[%s]->RE[%03d] %c %s(%d)  %s R: '%s'x%d\n",
            N->Name,
            reindex,
            (N->RE[reindex].Type == mreStart) ? 'S' : 'E',
            R->Name,
            RIndex,
            MULToTString(N->RE[reindex].Time - MSched.Time), 
            MUCResToString(&N->RE[reindex].DRes,0,0,NULL),
            RC);
 
          if ((R->Type == mrtUser) &&
              (R->StartTime <= MSched.Time) &&
              (N->RE[reindex].DRes.Procs * RC < R->DRes.Procs * RC - N->DRes.Procs))
            {
            DBG(1,(fSTRUCT|fSCHED)) DPrint("ALERT:    corrupt resource structure in RE[%d]  (RED*RC < RD*RC - ND) RC: %d  RED: %d  RD: %d  ND: %d\n",
              reindex,
              RC,
              N->RE[reindex].DRes.Procs,
              R->DRes.Procs,
              N->DRes.Procs);
            }
          }
 
        if (N->RE[reindex].Type == mreStart)
          Count[RIndex]++;
        else if (N->RE[reindex].Type == mreEnd)
          Count[RIndex]--;
 
        if ((Count[RIndex] < 0) ||
            (Count[RIndex] > 1))
          {
          DBG(1,(fSTRUCT|fSCHED)) DPrint("ALERT:    RE[%03d] %c %2d  Count: %d\n",
            reindex,
            (N->RE[reindex].Type == mreStart) ? 'S' : 'E',
            RIndex,
            Count[RIndex]);
 
          Count[RIndex] =
            (Count[RIndex] < 0) ? 0 : 1;
          }
        }   /* END for (reindex)            */
 
      for (cindex = 0;cindex < MSched.ResDepth;cindex++) 
        {
        if (N->R[cindex] == NULL)
          break;
 
        if (N->R[cindex] == (mres_t *)1)
          continue;
 
        if (Count[cindex] != 0)
          {
          DBG(1,fSTRUCT) DPrint("ALERT:    R[%03d] %s started but not ended\n",
            cindex,
            N->R[cindex]->Name);
          }
 
        if (TCount[cindex] < 2)
          {
          DBG(1,(fSTRUCT|fSCHED)) DPrint("ALERT:    R[%03d] %s has no associated events\n",
            cindex,
            N->R[cindex]->Name);
          }
        }   /* END for (cindex)           */
      }     /* END if (IsCorrupt == TRUE) */
    }       /* END for (nindex)           */
 
  return(SUCCESS);
  }  /* END MRECheck() */





int MResPreempt(

  mres_t *R)

  {
  unsigned long  MinPriority;
 
  int            rindex;
 
  mres_t        *PR;
  mres_t        *MinPR;

  const char *FName = "MResPreempt";

  DBG(3,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    R->Name);
 
  MinPriority = 0xffffffff;
 
  MinPR = NULL;
 
  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    PR = MRes[rindex];
 
    if ((PR == NULL) || (PR->Name[0] == '\0'))
      break;
		    
    if (PR->Name[0] == '\1')
      continue;
 
    if (!(PR->Flags & (1 << mrfPreemptible)))
      continue;
 
    if (PR->Priority >= MIN(R->Priority,MinPriority))
      continue;
 
    MinPR = PR;
 
    MinPriority = PR->Priority;
    }  /* END for (rindex) */
 
  if (MinPriority >= R->Priority)
    {
    return(FAILURE);
    } 
 
  MResDestroy(&MinPR);
 
  return(SUCCESS);
  }  /* END MResPreempt() */





/* NOTE:  do not free reservation structure.  just mark it empty */

int MResDestroy(
 
  mres_t **RP)  /* I (modified) */
 
  {
  int sindex;
  int index;
 
  mjob_t   *J;
 
  char     Message[MAX_MLINE];
  char     WCString[MAX_MNAME];

  mres_t *R;
 
  const char *FName = "MResDestroy";
 
  DBG(3,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    ((RP != NULL) && (*RP != NULL)) ? (*RP)->Name : "NULL");
 
  /* invalid reservation specified */
 
  if ((RP == NULL) || (*RP == NULL) || (*RP == (mres_t *)1))
    {
    DBG(8,fSTRUCT) DPrint("INFO:     no reservation to release\n");
 
    return(SUCCESS);
    }

  R = *RP;

  if ((R->Name[0] == '\0') || (R->Name[0] == '\1'))
    {
    DBG(8,fSTRUCT) DPrint("INFO:     reservation already released\n");

    return(SUCCESS);
    }

  DBG(8,fSTRUCT)
    {
    strcpy(WCString,MULToTString(R->StartTime - MSched.Time));
 
    DPrint("INFO:     releasing reservation %s (%s -> %s : %d)\n",
      R->Name, 
      WCString,
      MULToTString(R->EndTime - MSched.Time),
      R->TaskCount);
    }
 
  MTRAPRES(R,FName);
 
  if (R->Flags & (1 << mrfStandingRes))
    {
    /* clear SR pointer */     

    for (sindex = 0;sindex < MAX_MSRES;sindex++)
      {
      for (index = 0;index < MAX_SRES_DEPTH;index++)
        {
        if (SRes[sindex].R[index] == R)
          {
          SRes[sindex].R[index] = NULL;
 
          break;
          }
        }    /* END for (index)   */
      }      /* END for (sindex)  */
    }        /* END if (R->Flags) */
 
  /* clear node reservation pointers */
 
  if (X.XResDestroy != (int (*)())0)
    (*X.XResDestroy)(X.xd,R);
 
  MResChargeAllocation(R,2);

  MResDeallocateResources(R);
 
  J = NULL;
 
  if (R->Type == mrtJob)
    {
    /* clear job reservation pointer */
 
    if (MJobFind(R->Name,&J,0) == SUCCESS)
      {
      J->R = NULL;
 
      DBG(5,fSTRUCT) DPrint("INFO:     job '%s' reservation released (tasks requested: %d)\n",
        J->Name,
        J->Request.TC);
      }
    else
      {
      DBG(1,fSTRUCT) DPrint("WARNING:  cannot locate job associated with reservation '%s' (nodes released)\n",
        R->Name);
      } 
    }
  else
    {
    /* send notification for non-job reservations only */
 
    sprintf(Message,"RESERVATIONDESTROYED:  %s %s %ld %ld %ld %d\n",
      R->Name,
      MResType[R->Type],
      MSched.Time,
      R->StartTime,
      R->EndTime,
      R->NodeCount);
 
    MSysRegEvent(Message,0,0,1);
    }      /* END if (R->Type == mrtJob) */
 
  /* clear reservation */
 
  MRERelease(MRE,R->Index,(MAX_MRES << 2));
 
  R->StartTime = 0;

  MUFree(&R->SystemID); 
  MUFree(&R->RegEx);
 
  if ((J != NULL) && (MSched.InitialLoad != TRUE))
    MResAdjustDRes(J->Name,1);
 
  if (R->Flags & (1 << mrfMeta))
    MResAdjustGResUsage(R,-1); 
 
  if (R->J != NULL)
    ((mjob_t *)(R->J))->RType = mjrNone;

  *RP = NULL;

  DBG(7,fSTRUCT) DPrint("INFO:     reservation '%s' released\n",
    R->Name);

  R->Name[0] = '\1';

  return(SUCCESS);
  }  /* END MResDestroy() */




int MResShowHostList(
 
  mres_t *R)  /* I */
 
  {
  int nindex;
  int sindex;

  int NC;
  int TC;
 
  const char *FName = "MResShowHostList";

  mnode_t *N;
 
  DBG(9,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");
 
  if (R == NULL)
    {
    return(FAILURE);
    }
 
  NC = 0;
  TC = 0;
 
  DBG(0,fSTRUCT) DPrint("INFO:     node list for res '%s'\n",
    R->Name);
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;
 
    if (N->Name[0] == '\1')
      continue;
 
    for (sindex = 0;sindex < MSched.ResDepth;sindex++)
      {
      DBG(6,fSTRUCT) DPrint("INFO:     checking status of node '%s', slot %d\n",
        N->Name,
        sindex);
 
      if (N->R[sindex] == NULL)
        break;
 
      if (N->R[sindex] == R)
        {
        fprintf(mlog.logfp,"[%s]",
          N->Name); 
 
        NC ++;
        TC += N->RC[sindex];
 
        break;
        }
      }   /* END for (sindex) */
    }     /* END for (nindex) */
 
  fprintf(mlog.logfp," (%d nodes/%d tasks located)\n",
    NC,
    TC);
 
  return(SUCCESS);
  }  /* END MResShowHostList() */




int MResDeallocateResources(

  mres_t *R)  /* I (modified) */

  { 
  int nindex;
  int sindex;
  int sindex2;

  int Offset;

  mnode_t *N;

  int      NC;
  int      TC;

  const char *FName = "MResDeallocateResources";

  if (R == NULL)
    {
    return(FAILURE);
    }

  NC = 0;
  TC = 0;
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];
 
    if ((N == NULL) || (N->Name[0] == '\0'))
      break;
 
    if (N->Name[0] == '\1')
      continue;
 
    for (sindex = 0;sindex < MSched.ResDepth;sindex++)
      {
      DBG(9,fSTRUCT) DPrint("INFO:     checking release of node '%s', slot %d\n",
        N->Name,
        sindex);
 
      if (N->R[sindex] == NULL)
        break;
 
      if (N->R[sindex] == R)
        {
        MTRAPNODE(N,FName);
 
        NC ++;
        TC += N->RC[sindex];
 
        /* remove RE objects */
 
        DBG(6,fSCHED)
          {
          MRECheck(N,"MResDeallocateResources-Start",TRUE);
          }
 
        N->R[sindex] = (mres_t *)1;
 
        Offset = 0;
 
        for (sindex2 = 0;sindex2 < (MSched.ResDepth << 1);sindex2++)
          {
          if (N->RE[sindex2].Type == mreNONE)
            break;
 
          if (N->RE[sindex2].Index == sindex)
            {
            Offset++;
 
            continue;
            }
 
          if (Offset > 0)
            {
            memcpy(
              &N->RE[sindex2 - Offset],
              &N->RE[sindex2],
              sizeof(mre_t));
            }
          }  /* END for (sindex2) */
 
        N->RE[sindex2 - Offset].Type = mreNONE;
 
        N->ResMTime = MSched.Time;
 
        DBG(6,fSCHED)
          {
          MRECheck(N,"MResDeallocateResources-End",TRUE);
          }
 
        DBG(5,fSTRUCT) DPrint("INFO:     node '%s' released from reservation\n",
          N->Name);
        }  /* END if (N->R[sindex] == R) */
      }    /* END for (sindex)           */
    }      /* END for (nindex)           */

  R->NodeCount = 0;      
  R->TaskCount = 0;
  R->AllocPC = 0;

  DBG(5,fSTRUCT) DPrint("INFO:     %d nodes/%d tasks released from reservation\n",
    NC,
    TC);

  return(SUCCESS);
  }  /* END MResDeallocateResources() */




int MResCheckJobMatch(
 
  mjob_t *J,  /* I */
  mres_t *R)  /* I */
 
  {
  int nindex;
  int nlindex;
  int sindex;
  int rqindex;
 
  mnode_t *N;
  mreq_t  *RQ;
 
  long    tmpEndTime;
 
  double NPFactor;
 
  int   NLTaskCount;

  const char *FName = "MResCheckJobMatch";
 
  DBG(6,fSTRUCT) DPrint("%s(%s,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (R != NULL) ? R->Name : "NULL");

  if ((J == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  /* return success if reservation adequately 'covers' up-to-date job requirements */
 
  /* NOTE:  DISABLE UNTIL FULLY TESTED */
 
  return(FAILURE);
 
  /*NOTREACHED*/
 
  /* verify reservation timeframe  */
 
  if (J->DispatchTime != R->StartTime)
    {
    /* job start time has changed */
 
    DBG(3,fSTRUCT) DPrint("ALERT:    start time has changed for job %s (%ld != %ld)\n",
      J->Name,
      J->DispatchTime, 
      R->StartTime);
 
    return(FAILURE);
    }
 
  NPFactor = 9999999.0;
 
  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];
 
    if (RQ->NodeList != NULL)
      {
      double tmpD;
 
      MUNLGetMinAVal(
        RQ->NodeList,
        mnaSpeed,
        NULL,
        (void **)&tmpD);
 
      NPFactor = MIN(NPFactor,tmpD);
      }
    }  /* END for (rqindex) */
 
  /* verify endtime */
 
  tmpEndTime = MAX(
    J->DispatchTime + (long)((double)J->SpecWCLimit[0] / NPFactor),
    MSched.Time + 1);
 
  if (tmpEndTime != R->EndTime)
    {
    /* job completion time has changed */
 
    DBG(3,fSTRUCT) DPrint("ALERT:    completion time has changed for job %s (%ld != %ld)\n",
      J->Name,
      J->DispatchTime + (long)((double)J->SpecWCLimit[0] / NPFactor),
      R->EndTime);
 
    return(FAILURE); 
    }
 
  /* verify reservation tasks match job tasks */
 
  /* verify all job tasks have matching reservation task */
  /* verify all node reservations have matching job task */
 
  NLTaskCount = 0;
 
  for (nlindex = 0;J->Req[0]->NodeList[nlindex].N != NULL;nlindex++)
    {
    NLTaskCount += J->Req[0]->NodeList[nlindex].TC;
    }
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];
 
    if ((N == NULL) || (N->Name[0] == '\0'))
      break;
 
    if (N->Name[0] == '\1')
      continue;
 
    for (sindex = 0;sindex < MSched.ResDepth;sindex++)
      {
      if (N->R[sindex] == NULL)
        break;
 
      if (N->R[sindex] != R)
        continue;
 
      /* matching reservation found, verify job requests node */
 
      for (nlindex = 0;J->Req[0]->NodeList[nlindex].N != NULL;nlindex++)
        {
        if (nindex == J->Req[0]->NodeList[nlindex].N->Index) 
          break;
        }
 
      if (J->Req[0]->NodeList[nlindex].N == NULL)
        {
        /* reserved node not in current job nodelist */
 
        DBG(3,fSTRUCT) DPrint("ALERT:    reserved node '%s' no longer in job %s nodelist\n",
          N->Name,
          J->Name);
 
        return(FAILURE);
        }
 
      NLTaskCount -= N->RC[sindex];
      }  /* END for (sindex) */
    }    /* END for (nindex) */
 
  if (NLTaskCount != 0)
    {
    /* node list task count mismatch */
 
    DBG(3,fSTRUCT) DPrint("ALERT:    node list taskcount mismatch for job %s (%d %s tasks)\n",
      J->Name,
      (NLTaskCount > 0) ? NLTaskCount : -NLTaskCount,
      (NLTaskCount > 0) ? "missing" : "additional");
 
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MResCheckJobMatch() */




int MRangeGetIntersection(
 
  mjob_t   *J,      /* I */
  mrange_t *Range1, /* I */
  mrange_t *Range2) /* O */
 
  {
  /* NYI */

  return(SUCCESS);
  }  /* END MRangeGetIntersection() */




int MRLMerge(

  mrange_t *R1,            /* I */
  mrange_t *R2,            /* I */
  int       MinTaskCount,  /* I */
  long     *StartTime)     /* O:   earliest time <MinTaskCount> tasks are available */

  {
  int index1;
  int index2;
  int cindex;

  mrange_t C[MAX_MRANGE + 2];

  enum { rNone = 0, rStart, rEnd, rInstant };

  long R1Time;
  long R2Time;

  int R1State;
  int R2State;

  long ETime;

  int TaskCount;
  int OldTaskCount;

  int NodeCount;
  int OldNodeCount;

  int IsCorrupt;

  const char *FName = "MRLMerge";

  DBG(8,fSTRUCT) DPrint("%s(R1,R2,%d,StartTime)\n",
    FName,
    MinTaskCount);

  /* initialize variables */

  index1 = 0;
  index2 = 0;
  cindex = 0;

  R1State = rStart; /* 'rStart' : next time to be processed is start */
  R2State = rStart;

  TaskCount = 0;
  NodeCount = 0;

  IsCorrupt = FALSE;

  /* TaskCount/NodeCount indicate resources in current range */

  if (StartTime != NULL)
    *StartTime = MAX_MTIME;

  while ((R1[index1].EndTime != 0) ||
         (R2[index2].EndTime != 0))
    {
    if (cindex >= MAX_MRANGE)
      {
      DBG(2,fSTRUCT) DPrint("ALERT:    range overflow in %s()\n",
        FName);

      IsCorrupt = TRUE;

      break;
      }

    if (R1[index1].EndTime == 0)
      {
      R1State = rNone;
      R1Time  = MAX_MTIME + 1;
      }
    else
      {
      R1Time = (R1State != rEnd) ? R1[index1].StartTime : R1[index1].EndTime;
      }

    if (R2[index2].EndTime == 0)
      {
      R2State = rNone;
      R2Time  = MAX_MTIME + 1;
      }
    else
      {
      R2Time = (R2State != rEnd) ? R2[index2].StartTime : R2[index2].EndTime;
      }

    ETime = MIN(R1Time,R2Time);

    if ((R1[index1].EndTime == ETime) && (R1[index1].StartTime == ETime))
      R1State = rInstant;

    if ((R2[index2].EndTime == ETime) && (R2[index2].StartTime == ETime))
      R2State = rInstant;

    /* handle instant states */

    if (((R1State == rInstant) && (R1Time == ETime)) ||
        ((R2State == rInstant) && (R2Time == ETime)))
      {
      if ((R1State == rInstant) && (R1Time == ETime))
        {
        /* R1 instant, R2 ??? */

        if ((R2State == rEnd) && (R2Time == ETime))
          {
          /* R1 instant, R2 bordering end */

          /* implied open range */

          C[cindex].EndTime = ETime;
          cindex++;

          OldTaskCount = TaskCount;
          OldNodeCount = NodeCount;

          /* advance R2 */

          TaskCount -= R2[index2].TaskCount;
          NodeCount -= R2[index2].NodeCount;

          index2++;

          R2State = rStart;

          /* create instant range */

          C[cindex].StartTime = ETime;

          if (R2[index2].EndTime == 0)
            R2Time = MAX_MTIME + 1;
          else
            R2Time = R2[index2].StartTime;

          /* R1 instant, R2 ??? (new)                    */
          /* if instant R2 exists, it must be this range */

          if (R2Time == ETime)
            {
            C[cindex].TaskCount = R1[index1].TaskCount + R2[index2].TaskCount;
            C[cindex].NodeCount = R1[index1].NodeCount + R2[index2].NodeCount;

            if (R2Time == R2[index2].EndTime)
              {
              /* R1 instant, R2 instant */

              TaskCount = 0;
              NodeCount = 0;

              index2++;

              R2State = rStart;
              }
            else
              {
              /* R1 instant, R2 start */

              /* DO NOTHING */
              }
            }
          else
            {
            /* R1 instant only */

            C[cindex].TaskCount = R1[index1].TaskCount + OldTaskCount;
            C[cindex].NodeCount = R1[index1].NodeCount + OldNodeCount;
            }

          C[cindex].EndTime   = ETime;
          }   /* END if ((R2State == rEnd) && (R2Time == ETime)) */
        else
          {
          /* implied closed range   */

          /* start instant range */

          C[cindex].StartTime = ETime;
          C[cindex].EndTime   = ETime;

          if (R2State == rInstant)
            {
            /* R1 instant, R2 instant */

            TaskCount = R1[index1].TaskCount + R2[index2].TaskCount;
            NodeCount = R1[index1].NodeCount + R2[index2].NodeCount;

            C[cindex].TaskCount = TaskCount;
            C[cindex].NodeCount = NodeCount;

            TaskCount = 0;
            NodeCount = 0;

            index2++;

            R2State = rStart;
            }
          else if ((R2State == rStart) && (R2Time == ETime))
            {
            /* R1 instant, R2 start */

            C[cindex].TaskCount = R1[index1].TaskCount + R2[index2].TaskCount;
            C[cindex].NodeCount = R1[index1].NodeCount + R2[index2].NodeCount;
            }
          else
            {
            /* R1 instant only */

            C[cindex].TaskCount = TaskCount + R1[index1].TaskCount;
            C[cindex].NodeCount = NodeCount + R1[index1].NodeCount;
            }
          }

        index1++;

        R1State = rStart;
        }
      else
        {
        /* R1 ???, R2 instant */
        /* R1 not instant     */

        if ((R1State == rEnd) && (R1Time == ETime))
          {
          /* R2 instant, R1 bordering end */

          /* implied open range */

          C[cindex].EndTime = ETime;
          cindex++;

          OldTaskCount = TaskCount;
          OldNodeCount = NodeCount;

          /* advance R1 */

          TaskCount -= R1[index1].TaskCount;
          NodeCount -= R1[index1].NodeCount;

          index1++;

          R1State = rStart;

          /* create instant range */

          C[cindex].StartTime = ETime;

          if (R1[index1].EndTime == 0)
            R1Time = MAX_MTIME + 1;
          else
            R1Time = R1[index1].StartTime;

          /* R2 instant, R1 ??? (new)                    */
          /* if instant R1 exists, it must be this range */

          if (R1Time == ETime)
            {
            C[cindex].TaskCount = R1[index1].TaskCount + R2[index2].TaskCount;
            C[cindex].NodeCount = R1[index1].NodeCount + R2[index2].NodeCount;

            if (R1Time == R1[index1].EndTime)
              {
              /* R1 instant, R2 instant */

              index1++;

              R1State = rStart;
              }
            else
              {
              /* R2 instant, R1 start */

              /* DO NOTHING */
              }
            }
          else
            {
            /* R2 instant only */

            C[cindex].TaskCount = R2[index2].TaskCount + OldTaskCount;
            C[cindex].NodeCount = R2[index2].NodeCount + OldNodeCount;
            }

          C[cindex].EndTime   = ETime;
          }   /* END if ((R1State == rEnd) && (R1Time == ETime)) */
        else
          {
          /* R1 not instant, R2 instant, R1 not end */

          if (TaskCount > 0)
            {
            C[cindex].EndTime = ETime;

            cindex++;
            }

          /* start instant range */

          C[cindex].StartTime = ETime;

          if ((R1State == rStart) && (R1Time == ETime))
            {
            /* R2 instant, R1 start */

            C[cindex].TaskCount = R2[index2].TaskCount + R1[index1].TaskCount;
            C[cindex].NodeCount = R2[index2].NodeCount + R1[index1].NodeCount;
            }
          else
            {
            /* R2 instant only */

            C[cindex].TaskCount = TaskCount + R2[index2].TaskCount;
            C[cindex].NodeCount = NodeCount + R2[index2].NodeCount;
            }

          C[cindex].EndTime   = ETime;
          }    /* END else ((R1State == rEnd) && (R1Time == ETime)) */

        index2++;

        R2State = rStart;
        }  /* END else ((R1State == rInstant) && (R1Time == ETime)) */

      cindex++;

      if (TaskCount > 0)
        {
        C[cindex].StartTime = ETime;
        C[cindex].TaskCount = TaskCount;
        C[cindex].NodeCount = NodeCount;
        }

      continue;
      }    /* END if (((R1State == rInstant) && (R1Time == ETime)) || ... */

    if (((R1State == rEnd) && (R1Time == ETime)) ||
        ((R2State == rEnd) && (R2Time == ETime)))
      {
      /* implied open range */

      C[cindex].EndTime = ETime;
      cindex++;

      if ((R1State == rEnd) && (R1Time == ETime))
        {
        TaskCount -= R1[index1].TaskCount;
        NodeCount -= R1[index1].NodeCount;

        index1++;

        R1State = rStart;
        }

      if ((R2State == rEnd) && (R2Time == ETime))
        {
        TaskCount -= R2[index2].TaskCount;
        NodeCount -= R2[index2].NodeCount;

        index2++;

        R2State = rStart;
        }

      if (TaskCount > 0)
        {
        C[cindex].StartTime = ETime;
        C[cindex].TaskCount = TaskCount;
        C[cindex].NodeCount = NodeCount;
        }

      continue;
      }  /* END if (((R1State == rEnd) && (R1Time == ETime)) || ... */

    if (((R1State == rStart) && (R1Time == ETime)) ||
        ((R2State == rStart) && (R2Time == ETime)))
      {
      if ((TaskCount > 0) && (ETime > C[cindex].StartTime))
        {
        C[cindex].EndTime = ETime;
        cindex++;
        }

      if ((R1State == rStart) && (R1Time == ETime))
        {
        TaskCount += R1[index1].TaskCount;
        NodeCount += R1[index1].NodeCount;

        R1State = rEnd;
        }

      if ((R2State == rStart) && (R2Time == ETime))
        {
        TaskCount += R2[index2].TaskCount;
        NodeCount += R2[index2].NodeCount;

        R2State = rEnd;
        }

      if (TaskCount > 0)
        {
        C[cindex].StartTime = ETime;
        C[cindex].TaskCount = TaskCount;
        C[cindex].NodeCount = NodeCount;
        }

      continue;
      }    /* END if (((R1State == rStart) && (R1Time == ETime)) || ...          */
    }      /* END while ((R1[index1].EndTime != 0) || (R2[index2].EndTime != 0)) */

  /* terminate range */

  cindex = MIN(cindex,MAX_MRANGE - 1);

  C[cindex].EndTime = 0;

  DBG(3,fALL)
    {
    DBG(6,fSCHED) DPrint("INFO:     range count: %d\n",
      cindex);

    for (index1 = 0;C[index1].EndTime != 0;index1++)
      {
      DBG(5,fSCHED) DPrint("INFO:     C[%02d]  S: %ld  E: %ld  T: %3d  N: %d\n",
        index1,
        C[index1].StartTime,
        C[index1].EndTime,
        C[index1].TaskCount,
        C[index1].NodeCount);

      if (C[index1].TaskCount <= 0)
        {
        IsCorrupt = TRUE;

        DPrint("ALERT:    corrupt taskcount detected (%d)\n",
          C[index1].TaskCount);
        }

      if ((C[index1].NodeCount <= 0) || (C[index1].NodeCount > C[index1].TaskCount))
        {
        IsCorrupt = TRUE;

        DPrint("ALERT:    corrupt nodecount detected (%d)\n",
          C[index1].NodeCount);
        }

      if (C[index1].EndTime < C[index1].StartTime)
        {
        IsCorrupt = TRUE;

        DPrint("ALERT:    corrupt time detected:  E(%ld) < S(%ld)\n",
          C[index1].EndTime,
          C[index1].StartTime);
        }

      if ((C[index1 + 1].EndTime != 0) &&
          (C[index1].EndTime > C[index1 + 1].StartTime))
        {
        IsCorrupt = TRUE;

        DPrint("ALERT:    corrupt time detected:  S2(%ld) < E1(%ld)\n",
          C[index1 + 1].StartTime,
          C[index1].EndTime);
        }
      }
    }

  if (IsCorrupt == TRUE)
    {
    DBG(3,fALL)
      {
      for (index1 = 0;R1[index1].EndTime != 0;index1++)
        {
        DPrint("INFO:     R1[%d]  S: %ld  E: %ld  T: %d  N: %d\n",
          index1,
          R1[index1].StartTime,
          R1[index1].EndTime,
          R1[index1].TaskCount,
          R1[index1].NodeCount);
        }

      for (index1 = 0;R2[index1].EndTime != 0;index1++)
        {
        DPrint("INFO:     R2[%d]  S: %ld  E: %ld  T: %d  N: %d\n",
          index1,
          R2[index1].StartTime,
          R2[index1].EndTime,
          R2[index1].TaskCount,
          R2[index1].NodeCount);
        }  /* END for (index1) */
      }
    }    /* END if (IsCorrupt == TRUE) */

  memcpy(R1,C,sizeof(mrange_t) * (cindex + 1));

  return(SUCCESS);
  }   /* END MRLMerge() */




int MRangeApplyGlobalDistributionConstraints(
 
  mrange_t *GRange,       /* I/O */
  mjob_t   *J,            /* I */
  int      *AvlTaskCount) /* O */
 
  {
  int rindex;
 
  if (GRange == NULL)
    {
    return(FAILURE);
    }
 
  for (rindex = 0;rindex < MAX_MRANGE;rindex++)
    {
    /* NYI */
    }  /* END for (rindex) */
 
  return(SUCCESS);
  }  /* END MRangeApplyGlobalDistributionConstraints() */ 
 
 
 
 
int MRangeApplyLocalDistributionConstraints(
 
  mrange_t *Range, /* I (modified) */
  mjob_t   *J,     /* I */
  mnode_t  *N)     /* I */
 
  {
  int rindex;
  int MinTPN;

  int RangeAdjusted = FALSE;

  mrm_t *R;

  if ((Range == NULL) || (J == NULL))
    {
    return(FAILURE);
    }

  R = (J->RM != NULL) ? J->RM : &MRM[0];

  switch (R->Type)
    {
    case mrmtLL:
 
      if (J->Req[0]->TasksPerNode > 1)
        MinTPN = J->Req[0]->TasksPerNode;
      else
        MinTPN = 1;
 
      if ((J->Req[0]->BlockingFactor != 1) && (J->Request.NC > 0))
        {
        MinTPN = MAX(MinTPN,J->Request.TC / J->Request.NC);
        }
 
      break; 
 
    default:
 
       MinTPN = 1;
 
       break;
    }  /* END switch(R->Type) */
 
  for (rindex = 0;Range[rindex].EndTime > 0;rindex++)
    {
    switch(R->Type)
      {
      case mrmtLL:
 
        if (Range[rindex].TaskCount < MinTPN)
          {  
          Range[rindex].TaskCount = 0;

          RangeAdjusted = TRUE;
 
          DBG(2,fSCHED) DPrint("INFO:     range[%d] taskcount for node %s violates task distribution policies (%d < %d)\n",
            rindex,
            N->Name,
            Range[rindex].TaskCount,
            MinTPN);
          }
 
        break;
 
      default:

        /* NO-OP */
 
        break;
      }  /* END switch(R->Type) */ 
    }    /* END for (rindex) */

  if (RangeAdjusted == TRUE)
    {
    int rindex2;

    /* remove empty ranges */

    rindex2 = 0;

    for (rindex = 0;Range[rindex].EndTime > 0;rindex++)
      {
      if (Range[rindex].TaskCount > 0)
        {
        if (rindex != rindex2)
          memcpy(&Range[rindex2],&Range[rindex],sizeof(mrange_t));
 
        rindex2++;
        }
      }    /* END for (rindex) */

    Range[rindex2].EndTime = 0; 
    }  /* END if (RangeAdjusted == TRUE) */
 
  return(SUCCESS);
  }  /* END MRangeApplyLocalDistributionConstraints() */




int MResAdjustGResUsage(

  mres_t *R,  /* I */
  int     TC) /* I */

  {
  mrange_t tmpRange[2];

  if (R == NULL)
    {
    return(FAILURE);
    }

  tmpRange[0].StartTime = R->StartTime;
  tmpRange[0].EndTime   = R->EndTime;
  tmpRange[0].TaskCount = TC;

  tmpRange[1].EndTime   = 0;     

  if (TC > 0)
    {
    /*
    MRLMerge(MRange,tmpRange,TC,NULL);
    */
    }
  else
    {
    /*
    MRLSubtract(MRange,tmpRange);
    */
    }

  return(SUCCESS);
  }  /* END MResAdjustGResUsage() */



 
int MResQSortHLEndTimeComp(
 
  int *A,  /* I */
  int *B)  /* I */
 
  {
  static int tmp;

  /* order low to high */     
 
  tmp = MRes[*A]->EndTime - MRes[*B]->EndTime;
 
  return(tmp);
  }  /* END MResQSortComp() */




int MResInitialize(

  mres_t **RP,    /* I (modified) */
  char    *RName) /* I (optional) */

  {
  if (RP == NULL)
    {
    return(FAILURE);
    }

  if (*RP == NULL)
    {
    *RP = calloc(1,sizeof(mres_t));
    }
  else
    {
    memset(*RP,0,sizeof(mres_t));
    }

  if (RName != NULL)
    MUStrCpy((*RP)->Name,RName,sizeof((*RP)->Name));
  else
    (*RP)->Name[0] = '\1';

  return(SUCCESS);
  }  /* END MResInitialize() */




int MResGetRID(

  mres_t *R,
  char   *RBase,
  char   *RUName)

  {
  mres_t *tmpR;

  int index;
  int rindex;

  int tmpI;

  char tmpName[MAX_MNAME];

  if ((R == NULL) || (RBase == NULL) || (RUName == NULL))
    {
    return(FAILURE);
    }

  index = 0;

  sprintf(tmpName,"%s.",
    RBase);

  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    tmpR = MRes[rindex];

    if ((tmpR == NULL) || (tmpR->Name[0] == '\0'))
      break;

    if (tmpR->Name[0] == '\1')
      continue;
 
    if (strncmp(tmpR->Name,tmpName,strlen(tmpName)))
      continue;

    tmpI = strtol(&tmpR->Name[strlen(tmpName)],NULL,0);

    index = MAX(index,tmpI + 1);
    }  /* for (rindex) */

  sprintf(RUName,"%s.%d",
    RBase,
    index);
 
  DBG(6,fSTRUCT) DPrint("INFO:     unique reservation ID '%s' selected\n",
    RUName);

  return(SUCCESS);
  }  /* END MResGetRID() */




int MResJCreate(

  mjob_t      *J,         /* I:  job reserving nodes     */
  mnodelist_t  MNodeList, /* I:  nodes to be reserved    */
  long         StartTime, /* I:  time reservation starts */
  int          ResType,   /* I:  purpose of reservation  */
  mres_t     **RP)        /* O:  reservation pointer (optional) */

  {
  int   rindex;

  int   rqindex;
  int   nindex;

  mres_t  *R;
  mnode_t *N;
  mreq_t  *RQ;

  int     RC;

  double  NPFactor;

  int     TaskCount;

  const char *FName = "MResJCreate";

  DBG(3,fSTRUCT) DPrint("%s(%s,MNodeList,%s,%s,Res)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    MULToTString(StartTime - MSched.Time),
    MJRType[ResType]);

  if ((J == NULL) || (J->Req[0] == NULL))
    {
    DBG(1,fSTRUCT) DPrint("ERROR:    invalid job passed to %s()\n",
      FName);

    return(FAILURE);
    }

  RQ = J->Req[0];

  /* verify nodelist */

  if ((MNodeList == NULL) &&
     ((RQ == NULL) || (RQ->NodeList == NULL)))
    {
    DBG(1,fSTRUCT) DPrint("ERROR:    invalid nodelist passed to %s()\n",
      FName);

    return(FAILURE);
    }

  MTRAPJOB(J,FName);

  /* find first available reservation slot */

  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    R = MRes[rindex];

    if ((R == NULL) || (R->Name[0] <= '\1'))
      break;
    }   /* END for (rindex) */

  if (rindex >= MAX_MRES)
    {
    DBG(1,fSTRUCT) DPrint("ERROR:    reservation overflow on reservation for job '%s'\n",
      J->Name);

    if (MSched.Mode == msmNormal)
      {
      MOSSyslog(LOG_ERR,"ERROR:  reservation overflow on job '%s'",
        J->Name);
      }

    return(FAILURE);
    }

  if (MResInitialize(&MRes[rindex],NULL) == FAILURE)
    {
    DBG(1,fSTRUCT) DPrint("ERROR:    cannot initialize reservation for job '%s'\n",
      J->Name);

    return(FAILURE);
    }

  R = MRes[rindex];

  R->Index = rindex;

  if (RP != NULL)
    *RP = R;

  /* link job to reservation */

  if (J->R != NULL)
    {
    DBG(0,fSTRUCT) DPrint("ERROR:    reservation created for reserved job '%s' (existing reservation '%s' deleted)\n",
      J->Name,
      J->R->Name);

    MResDestroy(&J->R);
    }

  J->R    = R;

  R->J    = J;
  R->Mode = 0;

  /* build reservation */

  strcpy(R->Name,J->Name);

  R->Type = mrtJob;

  /* get partition number from first node */

  R->StartTime = StartTime;

  NPFactor = 9999997.0;

  if (MNodeList != NULL)
    {
    double tmpD;

    /* get partition number from first node */

    if (MNodeList[0][0].N != NULL)
      R->PtIndex = MNodeList[0][0].N->PtIndex;
    else
      R->PtIndex = 0;

    for (rqindex = 0;MNodeList[rqindex][0].N != NULL;rqindex++)
      {
      MUNLGetMinAVal((mnalloc_t *)&MNodeList[rqindex][0],mnaSpeed,NULL,(void **)&tmpD);

      NPFactor = MIN(NPFactor,tmpD);
      }  /* END for (rqindex) */
    }
  else
    {
    double tmpD;

    /* get partition number from first node */

    RQ = J->Req[0]; 

    if (RQ->NodeList[0].N != NULL)
      R->PtIndex = RQ->NodeList[0].N->PtIndex;
    else
      R->PtIndex = 1;

    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      RQ = J->Req[rqindex];

      if (RQ->NodeList != NULL)
        {
        if (MUNLGetMinAVal(
            RQ->NodeList,
            mnaSpeed,
            NULL,
            (void *)&tmpD) == FAILURE)
          {
          continue;
          }

        NPFactor = MIN(NPFactor,tmpD);
        }
      }  /* END for (rqindex) */
    }    /* END (MNodeList != NULL) */

  if ((NPFactor <= 0.0001) || (NPFactor >= 9999990.0))
    {
    NPFactor = 1.0;
    }

  R->EndTime = MAX(
                 StartTime + (int)((double)J->SpecWCLimit[0] / NPFactor),
                 MSched.Time + 1);

  R->EndTime = MIN(R->EndTime,MAX_MTIME - 1);

  if (StartTime == 0)
    {
    DBG(0,fSTRUCT) DPrint("ERROR:    invalid StartTime specified for reservation on job res '%s'\n",
      R->Name);

    StartTime = 1;
    }

  /* NOTE:  only handle single task description per job */

  RQ = J->Req[0];

  MREInsert(MRE,R->StartTime,R->EndTime,rindex,&RQ->DRes,MAX_MRES << 2);

  /* link nodes to reservation */

  DBG(6,fSTRUCT) DPrint("INFO:     linking nodes to reservation '%s'\n",
    R->Name);

  R->NodeCount = 0;
  R->TaskCount = 0;
  R->AllocPC = MJobGetProcCount(J);

  /* copy ACL/CL info */

  memcpy(R->ACL,J->Cred.ACL,sizeof(R->ACL));
  memcpy(R->CL,J->Cred.CL,sizeof(R->CL));

  MACLSet(R->ACL,maJob,J->Name,mcmpSEQ,nmNeutralAffinity,0,0);

  nindex  = 0;
  rqindex = 0;

  while (1)
    {
    RQ = J->Req[rqindex];

    /* NYI : must handle class ACL management for multi-req jobs */

    if (MNodeList == NULL)
      {
      if ((RQ == NULL) ||
          (RQ->NodeList == NULL) ||
          (rqindex >= MAX_MREQ_PER_JOB))
        {
        break;
        }

      if ((RQ->NodeList[nindex].N == NULL) ||
          (RQ->NodeList[nindex].TC == 0))
        {
        rqindex++;

        nindex = 0;

        continue;
        }

      N         = RQ->NodeList[nindex].N;
      TaskCount = RQ->NodeList[nindex].TC;

      nindex++;
      }
    else
      {
      if (rqindex >= MAX_MREQ_PER_JOB)
        break;

      if ((MNodeList[rqindex][0].N == NULL) ||
          (MNodeList[rqindex][0].TC == 0))
        {
        break;
        }

      if ((MNodeList[rqindex][nindex].N == NULL) ||
          (MNodeList[rqindex][nindex].TC == 0))
        {
        rqindex++;

        nindex = 0;

        continue;
        }

      N         = MNodeList[rqindex][nindex].N;
      TaskCount = MNodeList[rqindex][nindex].TC;

      nindex++;
      }  /* END else (MNodeList == NULL) */

    if ((N->Name[0] == '\0') || (N->Name[0] == '\1'))
      {
      DBG(0,fSTRUCT) DPrint("ERROR:    job '%s' reservation nodelist contains invalid node at index %d\n",
        J->Name,
        nindex - 1);

      MResDestroy(&J->R);

      return(FAILURE);
      }

    if ((N->PtIndex != R->PtIndex) &&
        (N->PtIndex != 0) &&
       !(J->Flags & (1 << mjfSpan)))
      {
      DBG(0,fSTRUCT) DPrint("ERROR:    job '%s' reservation spans partitions (node %s: %d  node %s: %d)\n",
        J->Name,
        (MNodeList != NULL) ?
          MNodeList[rqindex][0].N->Name :
          RQ->NodeList[0].N->Name,
        R->PtIndex,
        N->Name,
        N->PtIndex);

      MResDestroy(&J->R);

      return(FAILURE);
      }

    MTRAPNODE(N,FName);

    if ((RQ->NAccessPolicy == mnacSingleJob) ||
        (RQ->NAccessPolicy == mnacSingleTask))
      {
      memset(&R->DRes,0,sizeof(R->DRes));

      R->DRes.Procs = -1;

      RC = 1;
      }
    else
      {
      memcpy(&R->DRes,&RQ->DRes,sizeof(R->DRes));

      RC = TaskCount;
      }

    R->NodeCount ++;
    R->TaskCount += RC;

    if (MResAddNode(R,N,RC,0) == FAILURE)
      {
      MResDestroy(&R);

      return(FAILURE);
      }
    }     /* END while (1) */

  J->RType = ResType;

  if (MSched.InitialLoad != TRUE)
    {
    MResAdjustDRes(J->Name,0);
    }

  R->CTime = MSched.Time;
  R->MTime = MSched.Time;

  return(SUCCESS);
  }  /* END MResJCreate() */




int MResFreeTable()

  {
  int rindex;

  mres_t *R;

  for (rindex = 0;rindex < MAX_MRES;rindex++)
    {
    R = MRes[rindex];

    if (R == NULL)
      {
      return(FAILURE);
      }

    if (R == (mres_t *)1)
      continue;

    if ((R->Name[0] == '\0') || (R->Name[0] == '\1'))
      continue;

    MResDestroy(&R);

    MUFree((char **)&MRes[rindex]);
    }  /* END for (rindex) */

  return(SUCCESS);
  }  /* END MResFreeTable() */




int MREInsert(

  mre_t   *RE,        /* I (array modified) */
  long     StartTime, /* I */
  long     EndTime,   /* I */
  int      ResIndex,  /* I */
  mcres_t *DRes,      /* I */
  int      TableSize) /* I */

  {
  int index;
  int startindex;
  int EndLocated;

  if (RE == NULL)
    {
    return(FAILURE);
    }

  /* NOTE:  only one start/end pair per job/user RE at creation time */

  if (RE[0].Type == mreNONE)
    {
    RE[0].Time  = StartTime;
    RE[0].Type  = mreStart;
    RE[0].Index = ResIndex;
    memcpy(&RE[0].DRes,DRes,sizeof(RE[0].DRes));

    RE[1].Time  = EndTime;
    RE[1].Type  = mreEnd;
    RE[1].Index = ResIndex;
    memcpy(&RE[1].DRes,DRes,sizeof(RE[0].DRes));

    RE[2].Type  = mreNONE;
    }
  else
    {
    /* locate last res-event */

    for (index = 0;index < TableSize;index++)
      {
      if (RE[index].Type == mreNONE)
        {
        /* terminate list at new end point */

        RE[MIN(index + 2,TableSize)].Type = mreNONE;

        break;
        }
      }    /* END for (index) */

    if (index == TableSize)
      {
      DBG(3,fSCHED) DPrint("ALERT:    RE table overflow detected - increase %s\n",
        MParam[pResDepth]);

      return(FAILURE);
      }

    /* insert end event */

    startindex = MAX(0,index - 1);

    EndLocated = FALSE;

    for (index = startindex;index >= 0;index--)
      {
      if (EndTime >= RE[index].Time)
        {
        /* insert end event */

        RE[index + 2].Time  = EndTime;
        RE[index + 2].Type  = mreEnd;
        RE[index + 2].Index = ResIndex;
        memcpy(&RE[index + 2].DRes,DRes,sizeof(RE[0].DRes));

        EndLocated = TRUE;

        break;
        }
      else
        {
        memcpy(&RE[index + 2],&RE[index],sizeof(RE[0]));
        }
      }    /* END for (index) */

    if (EndLocated == FALSE)
      {
      RE[1].Time  = EndTime;
      RE[1].Type  = mreEnd;
      RE[1].Index = ResIndex;
      memcpy(&RE[1].DRes,DRes,sizeof(RE[0].DRes));
      }

    /* insert start event */

    startindex = index;

    for (index = startindex;index >= 0;index--)
      {
      if (StartTime >= RE[index].Time)
        {
        /* insert start event */

        RE[index + 1].Time  = StartTime;
        RE[index + 1].Type  = mreStart;
        RE[index + 1].Index = ResIndex;
        memcpy(&RE[index + 1].DRes,DRes,sizeof(RE[0].DRes));

        break;
        }
      else
        {
        memcpy(&RE[index + 1],&RE[index],sizeof(RE[0]));
        }
      }   /* END for (index) */

    /* prepend start event if necessary */

    if (StartTime < RE[0].Time)
      {
      RE[0].Time  = StartTime;
      RE[0].Type  = mreStart;
      RE[0].Index = ResIndex;
      memcpy(&RE[0].DRes,DRes,sizeof(RE[0].DRes));
      }
    }   /* END else (RE[0].Type == mreNONE) */

  return(SUCCESS);
  }  /* END MREInsert() */




int MRERelease(

  mre_t  *RE,         /* I */
  int     ResIndex,
  int     TableSize)

  {
  int index;
  int Offset;

  if (RE == NULL)
    {
    return(FAILURE);
    }

  Offset = 0;

  for (index = 0;index < TableSize;index++)
    {
    if (RE[index].Type == mreNONE)
      break;

    if (RE[index].Index == ResIndex)
      {
      Offset++;

      continue;
      }

    if (Offset > 0)
      {
      memcpy(&RE[index - Offset],&RE[index],sizeof(RE[0]));
      }
    }  /* END for (index) */

  RE[index - Offset].Type = mreNONE;

  return(SUCCESS);
  }  /* END MRERelease() */




int MResShowState(

  mres_t  *R,
  int      Flags,
  char    *Buffer,
  int      BufSize,
  int      Mode)     /* I */

  {
  char *BPtr;
  int   BSpace;

  char tmpLine[MAX_MLINE];

  const char *FName = "MResShowState";

  DBG(5,fSTRUCT) DPrint("%s(%s,%d,Buffer,BufSize,%d)\n",
    FName,
    (R != NULL) ? "NONE" : R->Name,
    Flags,
    Mode);

  if (Buffer == NULL)
    {
    return(FAILURE);
    }

  BPtr   = Buffer;
  BSpace = BufSize;

  if (Mode == mSet)
    {
    Buffer[0] = '\0';
    }
  else
    {
    int len;

    len = strlen(Buffer);

    BPtr   += len;
    BSpace -= len;
    }

  if (R == NULL)
    {
    /* create header */

    MUSNPrintF(&BPtr,&BSpace,"%-20s %10s %3.3s %11s %11s  %11s %4s %4s %4s\n",
      "ResID",
      "Type",
      "Par",
      "StartTime",
      "EndTime",
      "Duration",
      "Node",
      "Task",
      "Proc");

    MUSNPrintF(&BPtr,&BSpace,"%-20s %10s %3.3s %11s %11s  %11s %4s %4s %4s\n",
      "-----",
      "----",
      "---",
      "---------",
      "-------",
      "--------",
      "----",
      "----",
      "----");

    return(SUCCESS);
    }  /* END if (R == NULL) */

  /* show base data */

  {
  char StartTime[MAX_MNAME];
  char EndTime[MAX_MNAME];
  char Duration[MAX_MNAME];

  strcpy(StartTime,MULToTString(R->StartTime - MSched.Time));
  strcpy(EndTime,MULToTString(R->EndTime - MSched.Time));
  strcpy(Duration,MULToTString(R->EndTime - R->StartTime));

  MUSNPrintF(&BPtr,&BSpace,"%-20s %10s %3.3s %11s %11s  %11s %4d %4d %4d\n",
    R->Name,
    MResType[R->Type],
    (R->PtIndex >= 0) ? MAList[ePartition][R->PtIndex] : "ALL",
    StartTime,
    EndTime,
    Duration,
    R->NodeCount,
    R->TaskCount,
    R->AllocPC);
  }  /* END BLOCK */

  /* display reservation flags */

  if ((R->Flags != 0) || (Flags & (1 << mcmVerbose)))
    {
    MUSNPrintF(&BPtr,&BSpace,"    Flags: %s\n",
      MUBMToString(R->Flags,MResFlags,' ',tmpLine,NONE));
    }

  /* display reservation ACL/CL */

  MACLListShow(R->ACL,-1,(1 << mfmHuman)|(1 << mfmVerbose),tmpLine);

  if (tmpLine[0] != '\0')
    {
    MUSNPrintF(&BPtr,&BSpace,"    ACL: %s\n",
      tmpLine);
    }

  MACLListShow(R->CL,-1,(1 << mfmHuman),tmpLine);

  if (tmpLine[0] != '\0')
    {
    MUSNPrintF(&BPtr,&BSpace,"    CL:  %s\n",
      tmpLine);
    }

  MResAToString(R,mraOwner,tmpLine,0,0);

  if (tmpLine[0] != '\0')
    {
    MUSNPrintF(&BPtr,&BSpace,"    Owner:  %s\n",
      tmpLine);
    }

  /* display accounting creds */

  if (R->Type == mrtUser)
    {
    if ((R->A != NULL) ||
        (R->G != NULL) ||
        (R->U != NULL) ||
        (Flags & (1 << mcmVerbose)))
      {
      MUSNPrintF(&BPtr,&BSpace,"    Accounting Creds: %s%s %s%s %s%s",
        (R->A != NULL) ? "Account:" : "",
        (R->A != NULL) ? R->A->Name : "",
        (R->G != NULL) ? "Group:" : "",
        (R->G != NULL) ? R->G->Name : "",
        (R->U != NULL) ? "User:" : "",
        (R->U != NULL) ? R->U->Name : "");
      }  /* END if (R->A != NULL) ... ) */
    }    /* END if (R->Type == mrtUser) */

  /* display per task dedicated resources */

  if ((R->Type == mrtUser) ||
     ((R->Type == mrtJob) && (R->DRes.Procs == -1)))
    {
    MUSNPrintF(&BPtr,&BSpace,"    Task Resources: %s\n",
      MUCResToString(&R->DRes,0,0,NULL));
    }

  /* display reservation attributes */

  tmpLine[0] = '\0';

  if (R->RegEx != NULL)
    {
     snprintf(tmpLine,MAX_MLINE,"%s   HostList='%s'",
      tmpLine,
      R->RegEx);
    }

  if (R->Priority > 0)
    {
    sprintf(tmpLine,"%s   Priority=%ld",
      tmpLine,
      R->Priority);
    }

  if (R->SystemID != NULL)
    {
    snprintf(tmpLine,MAX_MLINE,"%s   SystemID=%s",
      tmpLine,
      R->SystemID);
      }

  if (R->MaxTasks > 0)
    {
    sprintf(tmpLine,"%s   MaxTasks=%d",
      tmpLine,
      R->MaxTasks);
    }

  if (R->CBHost[0] != '\0')
    {
    int index;

    char tmpCBLine[MAX_MLINE];

    tmpCBLine[0] = '\0';

    for (index = 0;MCBType[index] != NULL;index++)
      {
      if (R->CBType & (1 << index))
        {
        if (tmpCBLine[0] != '\0')
          strcat(tmpCBLine,",");

        strcat(tmpCBLine,MCBType[index]);
        }
      }   /* END for (index) */

    sprintf(tmpLine,"%s   CBServer=%s:%d/%s",
      tmpLine,
      R->CBHost,
      R->CBPort,
      tmpCBLine);
    }  /* END if (R->CBHost[0] != '\0') */

  if (tmpLine[0] != '\0')
    {
    /* NOTE:  skip leading whitespace */

    MUSNPrintF(&BPtr,&BSpace,"    Attributes (%s)\n",
      &tmpLine[3]);
    }

  /* display statistics */

  if ((R->Type != mrtJob) &&
     ((R->TAPS + R->CAPS + R->TIPS + R->CIPS) > 0.0))
    {
    double E;

    if ((R->TAPS + R->CAPS) > 0.0)
      {
      E = (1.0 - (double)(R->TIPS + R->CIPS) /
          (R->TAPS + R->CAPS + R->TIPS + R->CIPS)) * 100.0;
      }
    else
      {
      E = 0.0;
      }

    MUSNPrintF(&BPtr,&BSpace,"    Active PH: %.2f/%-.2f (%.2f%c)\n",
      (R->TAPS + R->CAPS) / 3600.0,
      (R->TAPS + R->CAPS + R->TIPS + R->CIPS) / 3600.0,
      E,
      '%');
    }  /* END if (R->Type != mrtJob) && ...) */

  /* display SR attributes */

  if (R->Flags & (1 << mrfStandingRes))
    {
    int srindex;
    int dindex;

    char tmpLine[MAX_MLINE];
    char tmpTString[MAX_MNAME];

    sres_t *S;
    sres_t  tmpSR;

    for (srindex = 0;srindex < MAX_MSRES;srindex++)
      {
      S = &SRes[srindex];

      if (strncmp(S->Name,R->Name,strlen(S->Name)))
        continue;

      for (dindex = 0;dindex < MAX_SRES_DEPTH;dindex++)
        {
        if (S->R[dindex] == R)
          {
          MSRGetCurrentValues(S,&OSRes[srindex],&tmpSR);

          if (tmpSR.StartTime == -1)
            strcpy(tmpTString,MULToTString(0));
          else
            strcpy(tmpTString,MULToTString(tmpSR.StartTime));

          if (S->Days == 255)
            {
            strcpy(tmpLine,"ALL");
            }
          else
            {
            int index;

            tmpLine[0] = '\0';

            for (index = 0;MWeekDay[index] != NULL;index++)
              {
              if (S->Days & (1 << index))
                {
                if (tmpLine[0] != '\0')
                  strcat(tmpLine,",");

                strcat(tmpLine,MWeekDay[index]);
                }
              }    /* END for (index) */
            }      /* END else (S->Days == 255) */

          MUSNPrintF(&BPtr,&BSpace,"    SRAttributes (TaskCount: %d  StartTime: %s  EndTime: %s  Days: %s)\n",
            tmpSR.TaskCount,
            tmpTString,
            (tmpSR.EndTime == -1) ?
              MULToTString(86400) :
              MULToTString((tmpSR.EndTime > 0) ? tmpSR.EndTime : 86400),
            (tmpLine[0] != '\0') ? tmpLine : ALL);

          break;
          }  /* END if (SRes[srindex] == R) */
        }    /* END for (dindex)            */
      }      /* END for (srindex)           */
    }        /* END if (R->Flags & (1 << mrfStandingRes)) */

  /* display extension attributes */

  if ((R->xd != NULL) && (X.XResShow != (int (*)())0))
    {
    if (((*X.XResShow)(X.xd,R,tmpLine) == SUCCESS) &&
        (tmpLine[0] != '\0'))
      {
      MUStrNCat(&BPtr,&BSpace,tmpLine);
      }
    }

  return(SUCCESS);
  }  /* END MResShowState() */




int MResAdjustDRes(

  char *JName,     /* I (optional) */
  int   ForceEval) /* I (boolean) */

  {
  mres_t  *JR;
  mres_t  *R;

  mjob_t  *J;
  mnode_t *N;

  int     JRC;

  int     nindex;
  int     crindex;

  int     jrindex1;
  int     jrindex2;

  int     oreindex;
  int     nreindex;
  int     creindex;
  int     jreindex;
  int     treindex;

  int     NameFound;  /* (boolean) */

  long    Overlap;
  long    InitialJStartTime;
  long    CRStartTime;
  long    CREndTime;
  long    CROverlap;

  static short CRC[MAX_MRES_DEPTH];
  static short CRI[MAX_MRES_DEPTH];
  static short CRESI[MAX_MRES_DEPTH];
  static short CREEI[MAX_MRES_DEPTH];

  static mre_t mCRE[MAX_MRES_DEPTH];

  static mre_t  *CRE = NULL;  /* container RE */
  static mre_t  *ORE = NULL;  /* old RE */
  static mre_t  *NRE = NULL;  /* new RE */
  static mre_t  *JRE = NULL;  /* job RE */

  static int DRSize;

  long    JRETime;

  mcres_t BR;
  mcres_t IBR;  /* blocked resources */
  
  static mcres_t ZRes; /* null CRes */

  int        OPC;
  int        NPC;

  const char *FName = "MResAdjustDRes";

  DBG(4,fCORE) DPrint("%s(%s,%s)\n",
    FName,
    (JName == NULL) ? "NULL" : JName,
    (ForceEval == TRUE) ? "TRUE" : "FALSE");

  /* NOTE:  not thread safe */

  /* must calculate BRes for each 'container' reservation at         */
  /* each 'job' event boundary.  collapse container reservations     */
  /* where possible using memcmp.  Insert all container reservation  */
  /* events at BRes calculation completion                           */

  if (CRE == NULL || ORE == NULL || NRE == NULL || JRE == NULL )
    {
    memset(&ZRes,0,sizeof(ZRes));

    /* changed the mallocs to callocs because there is code below */
    /* that requires things to be zero'ed                         */
    if (CRE == NULL ) {
    	CRE = (mre_t *)calloc((MSched.ResDepth << 1),sizeof(mre_t));
    }
    if (ORE == NULL ){
    	ORE = (mre_t *)calloc((MSched.ResDepth << 1),sizeof(mre_t));
    }
    if (NRE == NULL ){
    	NRE = (mre_t *)calloc((MSched.ResDepth << 1),sizeof(mre_t));
  	}
    if (JRE == NULL ){
    	JRE = (mre_t *)calloc((MSched.ResDepth << 1),sizeof(mre_t));
    }

    DRSize = sizeof(mcres_t);
    }  /* END if (CRE == NULL) */

  if (CRE == NULL || ORE == NULL || NRE == NULL || JRE == NULL )
    {
    DBG(4,fCORE) DPrint("ALERT:    cannot allocate memory in %s\n",
      FName);

    return(FAILURE);
    }

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    if (N->ResMTime < MSched.Time)
      continue;

    CRI[0] = -1;

    NameFound = FALSE;

    jreindex = 0;

    JRETime  = 0;

    for (nreindex = 0;N->RE[nreindex].Type != mreNONE;nreindex++)
      {
      if (nreindex >= (MSched.ResDepth << 1))
        break;

      if (jreindex >= (MSched.ResDepth << 1) - 1)
        break;

      if ((N->R[N->RE[nreindex].Index] == NULL) || (N->R[N->RE[nreindex].Index] == (mrsv_t *)1))
        continue;

      if (N->R[N->RE[nreindex].Index]->Type == mrtJob)
        {
        JRETime = N->RE[nreindex].Time;

        /* job reservation located */

        memcpy(&JRE[jreindex],&N->RE[nreindex],sizeof(JRE[0]));

        J = (mjob_t *)(N->R[N->RE[nreindex].Index]->J);

        if ((NameFound == FALSE) && (JName != NULL))
          {
          if (!strcmp(J->Name,JName))
            {
            MTRAPJOB(J,FName);

            NameFound = TRUE;
            }
          }

        jreindex++;
        }
      else
        {
        /* record each unique container reservation */

        for (crindex = 0;crindex < MSched.ResDepth;crindex++)
          {
          if (N->RE[nreindex].Index == CRI[crindex])
            {
            /* verify job res event matches CR split start time */
            /* if not, both event should block same (MAX) resources */

            if (JRETime != N->RE[nreindex].Time)
              {
              /* look for matching job re entries */

              for (treindex = nreindex + 1;N->RE[treindex].Type != mreNONE;treindex++)
                {
                if (N->RE[treindex].Time != N->RE[nreindex].Time)
                  break;

                if (N->R[N->RE[nreindex].Index]->Type == mrtJob)
                  {
                  JRETime = N->RE[nreindex].Time;

                  break;
                  }
                }    /* END for (treindex) */
              }      /* END if (JRETime != N->RE[nreindex].Time) */

            if (JRETime != N->RE[nreindex].Time)
              {
              /* no matching job res event found */

              /* must update both start and end events */

              if (N->RE[nreindex].Type == mreStart)
                {
                OPC = (mCRE[crindex].DRes.Procs != -1) ? 
                  mCRE[crindex].DRes.Procs : 
                  N->CRes.Procs;

                NPC = (N->RE[nreindex].DRes.Procs != -1) ? 
                  N->RE[nreindex].DRes.Procs : 
                  N->CRes.Procs;

                if (OPC > NPC)
                  {
                  /* if previous event tuple is larger, locate outstanding end event */

                  for (treindex = nreindex + 1;N->RE[treindex].Type != mreNONE;treindex++)
                    {
                    if ((N->RE[treindex].Index != N->RE[nreindex].Index) ||
                        (N->RE[treindex].Type != mreEnd))
                      {
                      continue;
                      }

                    /* end event found */

                    break;
                    }  /* END for (treindex) */

                  memcpy(
                    &N->RE[nreindex].DRes,
                    &mCRE[crindex].DRes,
                    sizeof(N->RE[0].DRes));

                  memcpy(
                    &N->RE[treindex].DRes,
                    &mCRE[crindex].DRes,
                    sizeof(N->RE[0].DRes));

                  if ((N->RE[CRESI[crindex]].DRes.Procs == -1) || 
                      (N->RE[CREEI[crindex]].DRes.Procs == -1))
                    {
                    fprintf(stderr,"ALERT:    negative blocked resources detected\n");
                    }
                  }
                else if (OPC < NPC)
                  {
                  memcpy(
                    &mCRE[crindex].DRes,
                    &N->RE[nreindex].DRes,
                    sizeof(mCRE[crindex].DRes));

                  memcpy(
                    &N->RE[CRESI[crindex]].DRes,
                    &mCRE[crindex].DRes,
                    sizeof(N->RE[0].DRes));

                  memcpy(
                    &N->RE[CREEI[crindex]].DRes,
                    &mCRE[crindex].DRes,
                    sizeof(N->RE[0].DRes));

                  if ((N->RE[CRESI[crindex]].DRes.Procs == -1) || 
                      (N->RE[CREEI[crindex]].DRes.Procs == -1))
                    {
                    fprintf(stderr,"ALERT:    negative blocked resources detected\n");
                    }
                  }

                CRESI[crindex] = nreindex;
                }
              else
                {
                CREEI[crindex] = nreindex;
                }
              }    /* END if (JRETime != N->RE[nreindex].Time) */

            break;
            }

          if (CRI[crindex] == -1)
            {
            CRI[crindex]   = N->RE[nreindex].Index;
            CRC[crindex]   = N->RC[N->RE[nreindex].Index];
            CRESI[crindex] = nreindex;

            memcpy(&mCRE[crindex],&N->RE[nreindex],sizeof(mCRE[crindex]));

            CRI[crindex + 1] = -1;

            break;
            }
          }    /* END for (crindex)               */
        }      /* END else (R[N->RE[nreindex]...) */
      }        /* END for (nreindex)              */

    JRE[jreindex].Type = mreNONE;
    JRE[jreindex].Index = 0;

    InitialJStartTime = (jreindex > 0) ?
      N->R[JRE[0].Index]->StartTime :
      MAX_MTIME;

    if (((JName != NULL) && (NameFound == FALSE)) ||
         (jreindex == 0))
      {
      /* requested job name not found on node */

      /* issue, must update reservation utilization in MResDestroy()
       * situations where job RE's have just been removed.  ie,
       * job RE impact on inclusive reservations must be handled
       */

      if (ForceEval != TRUE)
        {
        continue;
        }
      }

    MTRAPNODE(N,FName);

    DBG(6,fSCHED)
      {
      MRECheck(N,"MResAdjustDRes-Start",TRUE);
      }

    memcpy(ORE,JRE,sizeof(ORE[0]) * (jreindex + 1));

    DBG(6,fSTRUCT) DPrint("INFO:     updating cr reservations for job '%s'\n",
      (JName != NULL) ? JName : "NULL");

    /* CR: Container Reservation */

    for (crindex = 0;CRI[crindex] != -1;crindex++)
      {
      creindex = 0;

      memset(CRE,0,sizeof(mre_t) * (MSched.ResDepth << 1));

      R = N->R[CRI[crindex]];

      CRStartTime = MAX(MSched.Time,R->StartTime);

      CREndTime   = R->EndTime;

      /* blocked resources =~ dedicated resource per task * taskcount */

      memset(&IBR,0,sizeof(BR));

      MCResAdd(&IBR,&N->CRes,&R->DRes,CRC[crindex],FALSE);

      if (CRStartTime < InitialJStartTime)
        {
        CRE[creindex].Index = CRI[crindex];
        CRE[creindex].Time  = CRStartTime;
        memcpy(&CRE[creindex].DRes,&IBR,sizeof(IBR));
        CRE[creindex].Type  = mreStart;

        creindex++;

        CRE[creindex].Index = CRI[crindex];
        CRE[creindex].Time  = MIN(CREndTime,InitialJStartTime);
        memcpy(&CRE[creindex].DRes,&IBR,sizeof(IBR));
        CRE[creindex].Type  = mreEnd;

        CRStartTime = InitialJStartTime;

        creindex++;
        }

      for (jrindex1 = 0;JRE[jrindex1].Type != mreNONE;jrindex1++)
        {
        /* locate smallest event interval */

        for (jrindex2 = 0;JRE[jrindex2].Type != mreNONE;jrindex2++)
          {
          if (JRE[jrindex2].Time > CRStartTime)
            {
            CREndTime = MIN(CREndTime,JRE[jrindex2].Time);

            break;
            }
          }    /* END for (jrindex2) */

        Overlap =
          MIN(CREndTime,R->EndTime) -
          MAX(CRStartTime,R->StartTime);

        if (Overlap <= 0)
          {
          if (CRStartTime >= R->EndTime)
            break;
          else
            continue;
          }

        memcpy(&BR,&IBR,sizeof(BR));

        for (jrindex2 = 0;JRE[jrindex2].Type != mreNONE;jrindex2++)
          {
          if (JRE[jrindex2].Type != mreStart)
            continue;

          JR = N->R[JRE[jrindex2].Index];

          if (JRE[jrindex2].Time >= CREndTime)
            break;

          Overlap =
            MIN(CREndTime,JR->EndTime) -
            MAX(CRStartTime,JR->StartTime);

          if (Overlap <= 0)
            {
            if (JR->StartTime > CREndTime)
              break;
            else
              continue;
            }

          JRC = N->RC[JRE[jrindex2].Index];

          J = (mjob_t *)JR->J;

          /* if job reservation overlaps container reservation component */

          switch(R->Type)
            {
            case mrtUser:

              CROverlap =
                MIN(R->EndTime,JR->EndTime) -
                MAX(R->StartTime,JR->StartTime);

              if (MResCheckJAccess(R,J,CROverlap,NULL,NULL) == TRUE)
                {
                if (JRE[jrindex2].Type == mreStart)
                  {
                  /* remove dedicated job resources from container blocked resources */

                  MCResRemove(&BR,&N->CRes,&JR->DRes,JRC,FALSE);
                  }
                else if (JRE[jrindex2].Type == mreEnd)
                  {
                  /* remove dedicated job resources from container blocked resources */

                  MCResAdd(&BR,&N->CRes,&JR->DRes,JRC,FALSE);
                  }
                }

              break;

            default:

              /* NO-OP */

              break;
            }     /* END switch (CR->Type) */
          }       /* END for (jrindex2)    */

        if (CRStartTime == R->EndTime)
          break;

        MUCResGetMax(&BR,&BR,&ZRes);

        if ((creindex > 0) &&
            (memcmp(&BR,&CRE[creindex - 1].DRes,sizeof(BR)) == 0))
          {
          /* merge ranges */

          CRE[creindex - 1].Time = CREndTime;
          }
        else
          {
          /* create new range */

          CRE[creindex].Index = CRI[crindex];
          CRE[creindex].Time  = CRStartTime;
          memcpy(&CRE[creindex].DRes,&BR,sizeof(CRE[0].DRes));
          CRE[creindex].Type  = mreStart;

          creindex++;

          CRE[creindex].Index = CRI[crindex];
          CRE[creindex].Time  = CREndTime;
          memcpy(&CRE[creindex].DRes,&BR,sizeof(CRE[0].DRes));
          CRE[creindex].Type  = mreEnd;

          creindex++;
          }

        if (CREndTime >= R->EndTime)
          break;

        /* advance start/end time for new CRes */

        CRStartTime = CREndTime;
        CREndTime   = R->EndTime;
        }         /* END for (jrindex1)    */

      if (creindex == 0)
        {
        /* no job overlap, use original CR events */

        CRE[creindex].Index = CRI[crindex];
        CRE[creindex].Time  = CRStartTime;
        memcpy(&CRE[creindex].DRes,&IBR,sizeof(CRE[0].DRes));
        CRE[creindex].Type  = mreStart;

        creindex++;

        CRE[creindex].Index = CRI[crindex];
        CRE[creindex].Time  = CREndTime;
        memcpy(&CRE[creindex].DRes,&IBR,sizeof(CRE[0].DRes));
        CRE[creindex].Type  = mreEnd;

        creindex++;
        }

      /* merge CR events with JR events */

      CRE[creindex].Type = mreNONE;

      creindex = 0;
      oreindex = 0;
      nreindex = 0;

      while ((CRE[creindex].Type != mreNONE) ||
             (ORE[oreindex].Type != mreNONE))
        {
        if ((creindex >= (MSched.ResDepth << 1)) ||
            (oreindex >= (MSched.ResDepth << 1)) ||
            (nreindex >= (MSched.ResDepth << 1)))
          {
          DBG(1,fSTRUCT) DPrint("ALERT:    node reservation event overflow (N: %d  C: %d  O: %d) - increase %s\n",
            nreindex,
            creindex,
            oreindex,
            MParam[pResDepth]);

          break;
          }

        if ((ORE[oreindex].Type == mreNONE) ||
            ((CRE[creindex].Type != mreNONE) &&
             (CRE[creindex].Time < ORE[oreindex].Time)))
          {
          memcpy(&NRE[nreindex],&CRE[creindex],sizeof(NRE[0]));

          nreindex++;

          creindex++;
          }
        else
          {
          memcpy(&NRE[nreindex],&ORE[oreindex],sizeof(NRE[0]));

          nreindex++;

          oreindex++;
          }
        }   /* END while ((CRE[creindex].Type != mreNONE) || ...) */

      memcpy(ORE,NRE,(sizeof(ORE[0]) * nreindex));

      ORE[nreindex].Type = mreNONE;
      }           /* END for (crindex)     */

    memcpy(N->RE,ORE,sizeof(N->RE[0]) * (nreindex + 1));

    /* perform sanity check on RE table */

    CRStartTime = 0;

    for (nreindex = 0;N->RE[nreindex].Type != mreNONE;nreindex++)
      {
      if (nreindex >= (MSched.ResDepth << 1))
        break;

      DBG(6,fSTRUCT) DPrint("INFO:     N[%s]->RE[%02d] (%s %s)\n",
        N->Name,
        nreindex,
        N->R[N->RE[nreindex].Index]->Name,
        MULToTString(N->RE[nreindex].Time - MSched.Time));

      if (CRStartTime > N->RE[nreindex].Time)
        {
        DBG(2,fSTRUCT) DPrint("ALERT:    node %s RE table is corrupt.  RE[%d] '%s' at %s is out of time order\n",
          N->Name,
          nreindex,
          N->R[N->RE[nreindex].Index]->Name,
          MULToTString(N->RE[nreindex].Time - MSched.Time));
        }

      CRStartTime = N->RE[nreindex].Time;
      }   /* END for (reindex) */

    DBG(6,fSCHED)
      {
      MRECheck(N,"MResAdjustDRes-End",TRUE);
      }
    }     /* END for (nindex)  */

  return(SUCCESS);
  }  /* END MResAdjustDRes() */





int MResDiagnoseState(

  mres_t  *R,        /* I */
  int      Flags,    /* I */
  char    *SBuffer,  /* O */
  int      SBufSize, /* I */
  int      Mode)     /* I */

  {
  int ResNC;
  int ResPC;
  int ResTC;
  int NodeRC;

  char *BPtr;
  int   BSpace;

  mcres_t *D;

  mcres_t ZRes;

  int   cindex;
  int   rindex;
  int   nindex;
  int   nrindex;
  int   reindex;

  int   IsCorrupt;  /* (boolean) */

  int   Count[MAX_MRES_DEPTH];
  int   TCount[MAX_MRES_DEPTH];

  int   RCount;
  int   RECount;
  
  mnode_t *N;
  mjob_t  *J;

  const char *FName = "MResDiagnoseState";

  DBG(5,fSTRUCT) DPrint("%s(%s,%d,SBuffer,SBufSize,%d)\n",
    FName,
    (R != NULL) ? "NONE" : R->Name,
    Flags,
    Mode);

  if ((R == NULL) || (SBuffer == NULL))
    {
    return(FAILURE);
    }

  BPtr   = SBuffer;
  BSpace = SBufSize;

  if (Mode == mSet)
    {
    BPtr[0] = '\0';
    }
  else
    {
    int len;

    len = strlen(SBuffer);

    BPtr   += len;
    BSpace -= len;
    }

  if ((R->Type == mrtUser) ||
     ((R->Type == mrtJob) && (R->DRes.Procs == -1)))
    {
    /* NO-OP */
    }     /* END if ((R->Type == mrtUser) || ... ) */
  else if (R->Type == mrtJob)
    {
    mres_t *tmpR;

    /* job reservation detected */

    /* check if reservation is duplicated */

    for (rindex = 0;rindex < R->Index;rindex++)
      {
      tmpR = MRes[rindex];

      if ((tmpR == NULL) || (tmpR->Name[0] == '\0'))
        break;

      if (!strcmp(R->Name,tmpR->Name))
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  res '%s' (%d) is duplicated at index %d\n",
          R->Name,
          R->Index,
          rindex);

        break;
        }
      }    /* END for (rindex) */

    /* verify associated job exists */

    if (MJobFind(R->Name,&J,0) == FAILURE)
      {
      MUSNPrintF(&BPtr,&BSpace,"WARNING:  cannot find job associated with res '%s'\n",
        R->Name);
      }
    else if (J->R != R)
      {
      MUSNPrintF(&BPtr,&BSpace,"WARNING:  job '%s' does not link to reservation\n",
        J->Name);
      }
    }    /* END else if (R->Type == mrtJob) */

  /* check nodes to validate reservation */

  ResNC = 0;
  ResPC = 0;
  ResTC = 0;

  D = &R->DRes;

  memset(&ZRes,0,sizeof(ZRes));

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    memset(Count,0,sizeof(Count));
    memset(TCount,0,sizeof(TCount));

    IsCorrupt = FALSE;

    RCount = 0;

    for (nrindex = 0;N->R[nrindex] != NULL;nrindex++)
      {
      if (N->R[nrindex] != (mres_t *)1)
        RCount++;
      }

    RECount = 0;

    for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
      {
      if (N->RE[reindex].Type == mreNONE)
        break;

      if (N->RE[reindex].Type == mreStart)
        Count[N->RE[reindex].Index]++;
      else if (N->RE[reindex].Type == mreEnd)
        Count[N->RE[reindex].Index]--;

      TCount[N->RE[reindex].Index]++;

      RECount++;

      if ((Count[N->RE[reindex].Index] < 0) ||
          (Count[N->RE[reindex].Index] > 1))
        {
        IsCorrupt = TRUE;
        }
      }  /* END for (reindex) */

    if (IsCorrupt == FALSE)
      {
      if ((RECount < (RCount << 1)) ||
         ((RECount % 2) == 1))
        {
        IsCorrupt = TRUE;
        }
      else
        {
        for (cindex = 0;cindex < MSched.ResDepth;cindex++)
          {
          if (N->R[cindex] == NULL)
            break;

          if (N->R[cindex] == (mres_t *)1)
            continue;

          if ((Count[cindex] != 0) || (TCount[cindex] < 2))
            {
            IsCorrupt = TRUE;

            break;
            }
          }  /* END for (cindex) */
        }
      }

    if (IsCorrupt == TRUE)
      {
      MUSNPrintF(&BPtr,&BSpace,"WARNING:  RE table on node %s is corrupt\n",
        N->Name);

      memset(Count,0,sizeof(Count));

      for (nrindex = 0;nrindex < MSched.ResDepth;nrindex++)
        {
        if (N->R[nrindex] == NULL)
          break;

        MUSNPrintF(&BPtr,&BSpace,"  R[%02d] '%s'\n",
          nrindex,
          (N->R[nrindex] != (mres_t *)1) ? N->R[nrindex]->Name : "EMPTY");
        }    /* END for (nrindex) */

      for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
        {
        if (N->RE[reindex].Type == mreNONE)
          break;

        MUSNPrintF(&BPtr,&BSpace,"  RE[%02d] %10s %5s %12s %s\n",
          reindex,
          MULToTString(N->RE[reindex].Time - MSched.Time),
          (N->RE[reindex].Type == mreStart) ?
            "Start" :
            "End",
          (N->R[N->RE[reindex].Index] > (mres_t *)1) ?
            N->R[N->RE[reindex].Index]->Name :
            "EMPTY",
          MUCResToString(&N->RE[reindex].DRes,0,0,NULL));

        if (N->RE[reindex].Type == mreStart)
          Count[N->RE[reindex].Index]++;
        else if (N->RE[reindex].Type == mreEnd)
          Count[N->RE[reindex].Index]--;

        if ((N->R[N->RE[reindex].Index]->Type == mrtUser) &&
            (!memcmp(&N->RE[reindex].DRes,&ZRes,sizeof(N->RE[reindex].DRes)) ||
            (N->RE[reindex].DRes.Procs == 0)))
          {
          /* empty dedicated resource structure */

          MUSNPrintF(&BPtr,&BSpace,"WARNING:  RE[%02d] user reservation has empty resource structure\n",
            reindex);

          break;
          }

        if ((Count[N->RE[reindex].Index] < 0) ||
            (Count[N->RE[reindex].Index] > 1))
          {
          MUSNPrintF(&BPtr,&BSpace,"WARNING:  RE[%02d] %s\n",
            reindex,
            (Count[N->RE[reindex].Index] < 0) ?
              "reservation end event located without accompanying start event" :
              "reservation start event located without accompanying end event");

          Count[N->RE[reindex].Index] = (Count[N->RE[reindex].Index] < 0) ? 0 : 1;
          }
        }   /* END for (reindex) */

      for (cindex = 0;cindex < MSched.ResDepth;cindex++)
        {
        if (N->R[cindex] == NULL)
          break;

        if (N->R[cindex] == (mres_t *)1)
          continue;

        if (Count[cindex] != 0)
          {
          MUSNPrintF(&BPtr,&BSpace,"WARNING:  R[%02d] %s %s\n",
            cindex,
            N->R[cindex]->Name,
            "reservation start event located but no accompanying end event found");
          }

        if (TCount[cindex] < 2)
          {
          MUSNPrintF(&BPtr,&BSpace,"WARNING:  R[%02d] %s %s\n",
            cindex,
            N->R[cindex]->Name,
            "reservation not referenced by event");
          }
        }   /* END for (cindex) */
      }     /* END if (IsCorrupt == TRUE) */

    NodeRC = 0;

    for (rindex = 0;rindex < MSched.ResDepth;rindex++)
      {
      if (N->R[rindex] == (mres_t *)1)
        continue;

      if (N->R[rindex] == NULL)
        break;

      if (N->R[rindex] != R)
        continue;

      ResNC++;

      for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
        {
        if (N->RE[reindex].Type == 0)
          break;

        if (N->R[N->RE[reindex].Index] != R)
          continue;

        D = &N->RE[reindex].DRes;

        break;
        }  /* END for (reindex) */

      ResPC += (D->Procs == -1) ?
        N->CRes.Procs :
        (D->Procs * N->RC[rindex]);

      ResTC += N->RC[rindex];

      if (NodeRC++ > 0)
        {
        MUSNPrintF(&BPtr,&BSpace,"WARNING:  node '%s' has multiple links to reservation '%s' (clearing extra links)\n",
          N->Name,
          R->Name);

        N->R[rindex] = (mres_t *)1;
        }  /* END if (NodeRC++ > 0) */
      }    /* END for (rindex)      */
    }      /* END for (nindex)      */

  if (ResPC != R->AllocPC)
    {
    MUSNPrintF(&BPtr,&BSpace,"WARNING:  reservation '%s' has %d proc(s) allocated but %d detected\n",
      R->Name,
      R->AllocPC,
      ResPC);
    }

  if (ResTC != R->TaskCount)
    {
    MUSNPrintF(&BPtr,&BSpace,"WARNING:  reservation '%s' has %d task(s) allocated but %d detected\n",
      R->Name,
      R->TaskCount,
      ResTC);
    }

  if (ResNC != R->NodeCount)
    {
    MUSNPrintF(&BPtr,&BSpace,"WARNING:  reservation '%s' has %d node(s) allocated but %d detected\n",
      R->Name,
      R->NodeCount,
      ResNC);
    }

  return(SUCCESS);
  }  /* END MResDiagnoseState() */




int MRLLimitTC(

  mrange_t *ARL,    /* I/O (available) */
  mrange_t *URL,    /* O   (utilized)  */
  mrange_t *DRL,    /* O   */
  int       TCMax)  /* I   */

  {
  int index1;
  int index2;
  int cindex;

  mrange_t *C;

  mrange_t tmpRL[MAX_MRANGE];

  long ATime;
  long UTime;

  int AType;
  int UType;

  long ETime;

  int OldTC;
  int TC;

  int IsInstantA; /* boolean */
  int IsInstantU; /* boolean */

  int ATC;
  int FTC;

  if ((ARL == NULL) || (URL == NULL))
    {
    return(FAILURE);
    }

  /* initialize variables */

  if (DRL != NULL)
    C = DRL;
  else
    C = tmpRL;

  index1 = 0;
  index2 = 0;
  cindex = 0;

  AType = mreStart;
  UType = mreStart;

  ATime = (ARL[0].EndTime == 0) ? MAX_MTIME + 1 : ARL[0].StartTime;
  UTime = (URL[0].EndTime == 0) ? MAX_MTIME + 1 : URL[0].StartTime;

  IsInstantA = ((ARL[index1].StartTime == ARL[index1].EndTime) && (ARL[index1].EndTime > 0)) ?
    TRUE :
    FALSE;

  IsInstantU = ((URL[index2].StartTime == URL[index2].EndTime) && (URL[index2].EndTime > 0)) ?
    TRUE :
    FALSE;

  FTC = TCMax;
  ATC = 0;

  TC = 0;

  while (TRUE)
    {
    if ((ARL[index1].EndTime == 0) && (URL[index2].EndTime == 0))
      {
      C[cindex].EndTime = 0;

      break;
      }

    OldTC = TC;

    ETime = MIN(ATime,UTime);

    if (ATime == ETime)
      {
      if (AType == mreStart)
        {
        ATC += ARL[index1].TaskCount;

        AType = mreEnd;
        ATime = ARL[index1].EndTime;
        }
      else
        {
        ATC -= ARL[index1].TaskCount;

        index1++;

        IsInstantA = ((ARL[index1].StartTime == ARL[index1].EndTime) && (ARL[index1].EndTime > 0)) ?
          TRUE :
          FALSE;

        AType = mreStart;
        ATime = (ARL[index1].EndTime == 0) ? MAX_MTIME + 1: ARL[index1].StartTime;
        }
      }

    if (UTime == ETime)
      {
      if (UType == mreStart)
        {
        FTC -= URL[index2].TaskCount;

        UType = mreEnd;
        UTime = URL[index2].EndTime;
        }
      else
        {
        FTC += URL[index2].TaskCount;

        index2++;

        IsInstantU = ((URL[index2].StartTime == URL[index2].EndTime) && (URL[index2].EndTime > 0)) ?
          TRUE :
          FALSE;

        UType = mreStart;
        UTime = (URL[index2].EndTime == 0) ? MAX_MTIME + 1: URL[index2].StartTime;
        }
      }

    TC = MIN(ATC,FTC);

    if (TC != OldTC)
      {
      if (OldTC > 0)
        {
        if ((ETime > C[cindex].StartTime) ||
            (IsInstantA == TRUE) ||
            (IsInstantU == TRUE))
          {
          /* terminate existing range */

          C[cindex].EndTime = ETime;

          cindex++;
          }
        }

      if (TC > 0)
        {
        /* start new range */

        C[cindex].StartTime = ETime;
        C[cindex].TaskCount = TC;
        }

      if (((ATime == ETime) && (IsInstantA == TRUE)) ||
          ((UTime == ETime) && (IsInstantU == TRUE)))
        {
        /* terminate instant range */

        C[cindex].EndTime = ETime;

        cindex++;

        if ((ATime == ETime) && (IsInstantA == TRUE))
          {
          ATC += ARL[index1].TaskCount;

          index1++;

          IsInstantA = ((ARL[index1].StartTime == ARL[index1].EndTime) && (ARL[index1].EndTime > 0)) ?
            TRUE :
            FALSE;

          AType = mreStart;
          ATime = (ARL[index1].EndTime == 0) ? MAX_MTIME + 1: ARL[index1].StartTime;
          }

        if ((UTime == ETime) && (IsInstantU == TRUE))
          {
          FTC -= URL[index2].TaskCount;

          index2++;

          IsInstantU = ((URL[index2].StartTime == URL[index2].EndTime) && (URL[index2].EndTime > 0)) ?
            TRUE :
            FALSE;

          UType = mreStart;
          UTime = (URL[index2].EndTime == 0) ? MAX_MTIME + 2: URL[index2].StartTime;
          }

        TC = MIN(ATC,FTC);

        if (TC > 0)
          {
          /* start new range */

          C[cindex].StartTime = ETime;
          C[cindex].TaskCount = TC;
          }
        }
      }      /* END if (TC != OldTC) */
    }        /* END while(TRUE)      */

  if (DRL == NULL)
    {
    memcpy(ARL,C,sizeof(mrange_t) * (cindex + 1));
    }

  return(SUCCESS);
  }  /* END MRLGetMinTC() */




int MRLSubtract(

  mrange_t *R1,  /* I/O */
  mrange_t *R2)  /* I */

  {
  int index1;
  int index2;
  int cindex;

  mrange_t C[MAX_MRANGE];

  enum { rNone = 0, rStart, rEnd, rInstant };

  int R1Time;
  int R2Time;

  int R1State;
  int R2State;

  int ETime;

  int TaskCount;

  /* initialize variables */

  index1 = 0;
  index2 = 0;
  cindex = 0;

  R1State = rStart;
  R2State = rStart;

  TaskCount = 0;

  while ((R1[index1].EndTime != 0) || (R2[index2].EndTime != 0))
    {
    if (R1[index1].EndTime == 0)
      {
      R1State = rNone;
      R1Time  = MAX_MTIME + 1;
      }
    else
      {
      R1Time = (R1State != rEnd) ? R1[index1].StartTime : R1[index1].EndTime;

      if (R1[index1].StartTime == R1[index1].EndTime)
        R1State = rInstant;
      }

    if (R2[index2].EndTime == 0)
      {
      R2State = rNone;
      R2Time  = MAX_MTIME + 1;
      }
    else
      {
      R2Time = (R2State != rEnd) ? R2[index2].StartTime : R2[index2].EndTime;

      if (R2[index2].StartTime == R2[index2].EndTime)
        R1State = rInstant;
      }

    ETime = MIN(R1Time,R2Time);

    /* handle instant states */

    if (((R1State == rInstant) && (R1Time == ETime)) ||
        ((R2State == rInstant) && (R2Time == ETime)))
      {
      if ((R1State == rInstant) && (R1Time == ETime))
        {
        /* R1 instant, R2 ??? */

        if ((R2State == mreEnd) && (R2Time == ETime))
          {
          /* R1 instant, R2 overlapping end */

          /* implied open range */

          C[cindex].EndTime = ETime;
          cindex++;

          TaskCount += R2[index2].TaskCount;

          index2++;

          /* start instant range */

          C[cindex].StartTime = ETime;
          C[cindex].EndTime   = ETime;

          /* R1 instant, R2 ??? (new)                    */
          /* if instant R2 exists, it must be this range */

          if (R2[index2].EndTime == 0)
            R2Time = MAX_MTIME + 1;
          else
            R2Time = R2[index2].StartTime;

          if (R2Time == ETime)
            {
            C[cindex].TaskCount = R1[index1].TaskCount + R2[index2].TaskCount;

            if (R2Time == R2[index2].EndTime)
              {
              /* R1 instant, R2 instant */

              index2++;

              R2State = rStart;
              }
            else
              {
              /* R1 instant, R2 start */

              /* DO NOTHING */
              }
            }
          else
            {
            /* R1 instant only */

            C[cindex].TaskCount = R1[index1].TaskCount;
            }
          }
        else
          {
          /* implied closed range   */

          /* start instant range */

          C[cindex].StartTime = ETime;
          C[cindex].EndTime   = ETime;

          if (R2State == rInstant)
            {
            /* R1 instant, R2 instant */

            C[cindex].TaskCount = R1[index1].TaskCount + R2[index2].TaskCount;

            index2++;

            R2State = rStart;
            }
          else if ((R2State == rStart) && (R2Time == ETime))
            {
            /* R1 instant, R2 start */

            C[cindex].TaskCount = R1[index1].TaskCount + R2[index2].TaskCount;
            }
          else
            {
            /* R1 instant only */

            C[cindex].TaskCount = TaskCount + R1[index1].TaskCount;
            }
          }

        index1++;

        R1State = rStart;
        }
      else
        {
        /* R1 ???, R2 instant */
        /* R1 not instant     */

        if ((R1State == mreEnd) && (R1Time == ETime))
          {
          /* R1 overlapping end, R2 instant */

          /* implied open range */

          C[cindex].EndTime   = ETime;
          cindex++;

          TaskCount -= R1[index1].TaskCount;

          index1++;

          /* start instant range */

          C[cindex].StartTime = ETime;
          C[cindex].EndTime   = ETime;

          /* R2 instant, R1 ??? (new)                    */
          /* if instant R1 exists, it must be this range */

          if (R1[index1].EndTime == 0)
            R1Time = MAX_MTIME + 1;
          else
            R1Time = R1[index1].StartTime;

          if (R1Time == ETime)
            {
            C[cindex].TaskCount = R1[index1].TaskCount + R2[index2].TaskCount;

            if (R1Time == R1[index1].EndTime)
              {
              /* R1 instant, R2 instant */

              index1++;

              R1State = rStart;
              }
            else
              {
              /* R2 instant, R1 start */

              /* DO NOTHING */
              }
            }
          else
            {
            /* R2 instant only */

            C[cindex].TaskCount = R2[index2].TaskCount;
            }
          }
        else
          {
          /* implied closed range   */

          /* start instant range */

          C[cindex].StartTime = ETime;
          C[cindex].EndTime   = ETime;

          if ((R1State == rStart) && (R1Time == ETime))
            {
            /* R2 instant, R1 start */

            C[cindex].TaskCount = R2[index2].TaskCount + R1[index1].TaskCount;
            }
          else
            {
            /* R2 instant only */

            C[cindex].TaskCount = TaskCount + R2[index2].TaskCount;
            }
          }

        index2++;

        R2State = rStart;
        }

      cindex++;

      continue;
      }

    if (((R1State == rEnd) && (R1Time == ETime)) ||
        ((R2State == rEnd) && (R2Time == ETime)))
      {
      /* implied open range */

      C[cindex].EndTime = ETime;
      cindex++;

      if ((R1State == rEnd) && (R1Time == ETime))
        {
        TaskCount -= R1[index1].TaskCount;

        index1++;

        R1State = rStart;
        }

      if ((R2State == rEnd) && (R2Time == ETime))
        {
        TaskCount -= R2[index2].TaskCount;

        index2++;

        R2State = rStart;
        }

      if (TaskCount > 0)
        {
        C[cindex].StartTime = ETime;
        C[cindex].TaskCount = TaskCount;
        }

      continue;
      }

    if (((R1State == rStart) && (R1Time == ETime)) ||
        ((R2State == rStart) && (R2Time == ETime)))
      {
      if ((TaskCount > 0) && (ETime > C[cindex].StartTime))
        {
        C[cindex].EndTime = ETime;
        cindex++;
        }

      if ((R1State == rStart) && (R1Time == ETime))
        {
        TaskCount += R1[index1].TaskCount;

        R1State = rEnd;
        }

      if ((R2State == rStart) && (R2Time == ETime))
        {
        TaskCount += R2[index2].TaskCount;

        R2State = rEnd;
        }

      if (TaskCount > 0)
        {
        C[cindex].StartTime = ETime;
        C[cindex].TaskCount = TaskCount;
        }

      continue;
      }  /* END if (((R1State == rStart) && (R1Time == ETime)) || ...          */
    }    /* END while ((R1[index1].EndTime != 0) || (R2[index2].EndTime != 0)) */

  /* terminate range */

  C[cindex].EndTime = 0;

  memcpy(R1,C,sizeof(mrange_t) * (cindex + 1));

  return(SUCCESS);
  }   /* END MRLSubtract() */




int MResDiagGrid(

  char *SBuffer,
  int   SBufSize,
  int   Mode)

  {
  int rindex;

  int   len;

  char *BPtr;
  int   BSpace;

  char  TimeLine[MAX_MLINE];

  const char *FName = "MResDiagGrid";

  DBG(5,fSTRUCT) DPrint("%s(SBuffer,%d,%d)\n",
    FName,
    SBufSize,
    Mode);

  if (SBuffer == NULL)
    {
    return(FAILURE);
    }

  len = strlen(SBuffer);

  BPtr   = SBuffer + len;
  BSpace = SBufSize - len;
  
  if (MPar[0].MaxMetaTasks > 0)
    {
    MUSNPrintF(&BPtr,&BSpace,"\nMeta Tasks\n\n");

    MUSNPrintF(&BPtr,&BSpace,"%5s  %10s -> %10s\n",
      "Tasks",
      "StartTime",
      "EndTime");

    MUSNPrintF(&BPtr,&BSpace,"%5s  %10s -- %10s\n",
      "-----",
      "----------",
      "----------");

    if (MRange[0].EndTime == 0)
      {
      strcpy(TimeLine,MULToTString(0));

      MUSNPrintF(&BPtr,&BSpace,"%5d  %10s -> %10s\n",
        0,
        TimeLine,
        MULToTString(MAX_MTIME));
      }
    else
      {
      for (rindex = 0;rindex < (MAX_MRES_DEPTH << 1);rindex++)
        {
        if (MRange[rindex].EndTime == 0)
          break;

        strcpy(TimeLine,MULToTString(MRange[rindex].StartTime - MSched.Time));

        MUSNPrintF(&BPtr,&BSpace,"%5d  %10s -> %10s\n",
          MRange[rindex].TaskCount,
          TimeLine,
          MULToTString(MRange[rindex].EndTime - MSched.Time));
        }  /* END for (rindex)                  */
      }    /* END else (MRange[0].EndTime == 0) */
    }      /* END if (MPar[0].MaxMetaTasks > 0) */

  return(SUCCESS);
  }  /* END MResDiagGrid() */




int MResToString(

  mres_t *R,        /* I */
  int    *RC,       /* I (optional) */
  char   *SBuf,     /* O */
  int     SBufSize,
  int    *RAList)   /* I (optional) */

  {
  mxml_t *E = NULL;

  if ((R == NULL) || (SBuf == NULL))
    {
    return(FAILURE);
    }

  if (MXMLCreateE(&E,(char *)MXO[mxoRsv]) == FAILURE)
    {
    return(FAILURE);
    }

  MResToXML(R,E,RAList);

  MXMLToString(E,SBuf,SBufSize,NULL,TRUE);

  return(SUCCESS);
  }  /* END MResToString() */




int MNResToString(

  mnode_t  *N,        /* I */
  mres_t   *R,        /* I (optional) */
  mxml_t **EP,       /* O (optional) */
  char     *SBuf,     /* O (optional) */
  int       SBufSize) /* I */

  {
  int rindex;

  static int DNRAList[] = {
    mnraDRes,
    mnraEnd,
    mnraName,
    mnraState,
    mnraStart,
    mnraTC,
    mnraType,
    -1 };

  mxml_t *NE  = NULL;

  if ((N == NULL) || 
     ((SBuf == NULL) && (EP == NULL)))
    {
    return(FAILURE);
    }

  if (MXMLCreateE(&NE,"node") == FAILURE)
    {
    return(FAILURE);
    }

  MXMLSetAttr(NE,"name",(void *)N->Name,mdfString);

  for (rindex = 0;rindex < MSched.ResDepth;rindex++)
    {
    mxml_t *NRE = NULL;

    if (N->R[rindex] == NULL)
      break;
 
    if (N->R[rindex] == (mres_t *)1)
      continue;

    if ((R != NULL) && (N->R[rindex] != R))
      continue;

    /* create XML */

    if (MXMLCreateE(&NRE,"nres") == FAILURE)
      {
      break;
      }

    MNResToXML(N,rindex,NRE,(int *)DNRAList);

    MXMLAddE(NE,NRE);
    }  /* END for (rindex) */

  if (SBuf != NULL)
    {
    MXMLToString(NE,SBuf,SBufSize,NULL,TRUE);
    }

  if (EP != NULL)
    {
    *EP = NE;    
    }
  else
    {
    MXMLDestroyE(&NE);
    }

  return(SUCCESS);
  }  /* END MNResToString() */




int MNResToXML(

  mnode_t *N,       /* I */
  int      RIndex,  /* I */
  mxml_t *E,       /* O */
  int     *AList)   /* I (optional) */

  {
  mres_t *R;
  char    tmpState[MAX_MNAME];

  char tmpLine[MAX_MLINE];

  if ((N == NULL) || (N->R[RIndex] == NULL) || (E == NULL))
    {
    return(FAILURE);
    }

  R = N->R[RIndex];

  MXMLSetAttr(E,(char *)MNResAttr[mnraName],R->Name,mdfString);

  sprintf(tmpLine,"%ld",
    R->StartTime);
 
  MXMLSetAttr(E,(char *)MNResAttr[mnraStart],tmpLine,mdfString);

  sprintf(tmpLine,"%ld",
    R->EndTime);

  MXMLSetAttr(E,(char *)MNResAttr[mnraEnd],tmpLine,mdfString);

  switch(R->Type)
    {
    case mrtJob:

      {
      mjob_t *J;

      strcpy(tmpLine,"Job");

      if ((J = (mjob_t *)R->J) == NULL)
        {
        DBG(2,fUI) DPrint("ERROR:    cannot locate host job '%s'\n",
          R->Name);

        strcpy(tmpState,MJobState[mjsUnknown]);
        }
      else
        {
        strcpy(tmpState,MJobState[J->State]);
        }
      }    /* END BLOCK */

      break;

    case mrtUser:

      strcpy(tmpLine,"User");

      strcpy(tmpState,"N/A");

      break;

    default:

      strcpy(tmpLine,"?");

      strcpy(tmpState,"N/A");

      break;
    }  /* END switch(R->Type) */

  MXMLSetAttr(E,(char *)MNResAttr[mnraState],tmpState,mdfString);

  MXMLSetAttr(E,(char *)MNResAttr[mnraType],tmpLine,mdfString);

  sprintf(tmpLine,"%d",
    N->RC[RIndex]);

  MXMLSetAttr(E,(char *)MNResAttr[mnraTC],tmpLine,mdfString);

  /* NOTE:  mnraDRes not handled */

  return(SUCCESS);
  }  /* END MNResToString() */




int MRLSFromA(

  long      MinDuration,  /* I */
  mrange_t *ARL,          /* I */
  mrange_t *SRL)          /* O */

  {
  int  aindex;
  int  sindex;

  int  rindex;

  long RangeTime;

  int TaskCount;

  /* NOTE:  assumes intra-node ranges */

  const char *FName = "MRLSFromA";

  DBG(6,fSCHED) DPrint("%s(%ld,ARL,SRL)\n",
    FName,
    MinDuration);

  if ((ARL == NULL) || (SRL == NULL))
    {
    return(FAILURE);
    }

  sindex = 0;

  for (aindex = 0;ARL[aindex].EndTime > 0;aindex++)
    {
    /* NOTE:  must take into account connected ranges */

    RangeTime = ARL[aindex].EndTime - ARL[aindex].StartTime;

    TaskCount = ARL[aindex].TaskCount;

    for (rindex = aindex + 1;ARL[rindex].EndTime != 0;rindex++)
      {
      if (ARL[rindex - 1].EndTime != ARL[rindex].StartTime)
        break;

      RangeTime += ARL[rindex].EndTime - ARL[rindex].StartTime;
      TaskCount = MIN(TaskCount,ARL[rindex].TaskCount);

      if (RangeTime > (ARL[aindex].EndTime - ARL[aindex].StartTime + MinDuration))
        break;
      }  /* END for (rindex) */

    if (RangeTime >= MinDuration)
      {
      SRL[sindex].StartTime = ARL[aindex].StartTime;
      SRL[sindex].EndTime   = MIN(ARL[aindex].EndTime,ARL[aindex].StartTime + RangeTime - MinDuration);
      SRL[sindex].TaskCount = TaskCount;
      SRL[sindex].NodeCount = 1;

      sindex++;
      }
    }    /* END for (aindex) */

  SRL[sindex].EndTime = 0;

  if ((SRL[0].EndTime == 0) || (SRL[0].TaskCount == 0))
    {
    if ((SRL[0].TaskCount == 0) && (SRL[0].EndTime != 0))
      {
      DBG(0,fSCHED) DPrint("ALERT:    corrupt range found in %s (initial range empty)\n",
        FName);

      for (aindex = 0;ARL[aindex].EndTime > 0;aindex++)
        {
        DBG(2,fSCHED) DPrint("INFO:     ARL[%d]  %ld - %ld  %d  %d\n",
          aindex,
          ARL[aindex].StartTime,
          ARL[aindex].EndTime,
          ARL[aindex].TaskCount,
          ARL[aindex].NodeCount);
        }  /* END for (aindex) */
      }

    return(FAILURE);
    }  /* END if ((SRL[0].EndTime == 0) || (SRL[0].TaskCount == 0)) */

  return(SUCCESS);
  }  /* MRLSFromA() */


/* END MRes.c */

