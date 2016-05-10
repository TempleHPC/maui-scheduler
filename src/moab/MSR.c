/* HEADER */

#include "moab.h"
#include "msched-proto.h"  
 
extern msched_t    MSched;
extern msim_t      MSim;
extern mnode_t    *MNode[];
extern mstat_t     MStat;
extern mpar_t      MPar[];
extern mattrlist_t MAList;
extern mres_t     *MRes[];
extern sres_t      SRes[];
extern sres_t      OSRes[];
extern mqos_t      MQOS[];
extern mam_t       MAM[];
 
extern mlog_t      mlog;
 
#define MSRCfgParm "SRCFG"

extern const char *MWeekDay[];
extern const char *MPolicyMode[];
extern const char *MDefReason[];
extern const char *MWEEKDAY[];
extern const char *MSRPeriodType[];
extern const char *MResFlags[];
extern const char *MComp[];
extern const char *MSResAttr[];
extern const char *MXOC[];




int MSRLoadConfig( 

  char *SRName)
 
  {
  char    IndexName[MAX_MNAME];
 
  char    Value[MAX_MLINE];

  char   *ptr;
 
  sres_t *SR;
 
  /* FORMAT:  <KEY>=<VAL>[<WS><KEY>=<VAL>]...         */
  /*          <VAL> -> <ATTR>=<VAL>[:<ATTR>=<VAL>]... */
 
  /* load all/specified SR config info */
 
  if (MSched.ConfigBuffer == NULL)
    {
    return(FAILURE);
    }
 
  if ((SRName == NULL) || (SRName[0] == '\0'))
    {
    /* load ALL SR config info */
 
    ptr = MSched.ConfigBuffer;
 
    IndexName[0] = '\0';
 
    while (MCfgGetSVal(
             MSched.ConfigBuffer,
             &ptr,
             MSRCfgParm,
             IndexName,
             NULL,
             Value,
             sizeof(Value), 
             0,
             NULL) != FAILURE)
      {
      if ((MSRFind(IndexName,&SR) == FAILURE) &&
          (MSRAdd(IndexName,&SR) == FAILURE))
        {
        /* unable to locate/create SR */
 
        IndexName[0] = '\0';
 
        continue;
        }
 
      /* load SR specific attributes */

      MSRProcessConfig(SR,Value);
 
      IndexName[0] = '\0';
      }  /* END while (MCfgGetSVal() != FAILURE) */
    }    /* END if ((SRName == NULL) || (SRName[0] == '\0')) */
  else
    {
    /* load specified SR config info */

    ptr = MSched.ConfigBuffer;         

    SR = NULL;

    while (MCfgGetSVal(
             MSched.ConfigBuffer,
             &ptr, 
             MSRCfgParm,
             SRName,
             NULL,
             Value,
             sizeof(Value),
             0,
             NULL) == SUCCESS)
      {
      if ((SR == NULL) &&
          (MSRFind(SRName,&SR) == FAILURE) &&
          (MSRAdd(SRName,&SR) == FAILURE))
        {
        /* unable to add standing reservation */
 
        return(FAILURE);
        }
 
      /* load SR attributes */

      MSRProcessConfig(SR,Value);
      }  /* END while (MCfgGetSVal() == SUCCESS) */

    if (SR == NULL)
      {
      return(FAILURE);
      }

    if (MSRCheckConfig(SR) == FAILURE)
      {
      MSRDestroy(&SR);

      /* invalid standing reservation destroyed */

      return(FAILURE);
      }
    }  /* END else ((SRName == NULL) || (SRName[0] == '\0')) */
 
  return(SUCCESS); 
  }  /* END MSRLoadConfig() */




int MSRCheckConfig(

  sres_t *SR)  /* I */

  {
  /* no check required */

  return(SUCCESS);
  }  /* END MSRCheckConfig() */





int MSRDestroy(

  sres_t **SR)

  {
  if (SR == NULL)
    return(SUCCESS);

  if (*SR == NULL)
    return(SUCCESS);

  memset(*SR,0,sizeof(sres_t));

  return(SUCCESS);
  }  /* END MSRDestroy() */





int MSRFind(
 
  char    *SRName,
  sres_t **SRPtr)
 
  {
  /* if found, return success with SRPtr pointing to standing reservation. */
 
  int     srindex;
  sres_t *SR;
 
  if (SRPtr != NULL)
    *SRPtr = NULL;
 
  if ((SRName == NULL) ||
      (SRName[0] == '\0'))
    {
    return(FAILURE);
    }
 
  for (srindex = 0;srindex < MAX_MSRES;srindex++)
    {
    SR = &SRes[srindex];
 
    if ((SR == NULL) || (SR->Name[0] == '\0') || (SR->Name[0] == '\1'))
      {
      break;
      }
 
    if (strcmp(SR->Name,SRName) != 0)
      continue;
 
    /* reservation found */

    if (SRPtr != NULL) 
      *SRPtr = SR;
 
    return(SUCCESS);
    }  /* END for (srindex) */ 
 
  /* entire table searched */
 
  return(FAILURE);
  }  /* END MSRFind() */





int MSRAdd(

  char    *SRName,  /* I */
  sres_t **SRPtr)   /* O (optional) */

  {
  int     srindex;
 
  sres_t *SR;

  const char *FName = "MSRAdd";
 
  DBG(6,fSTRUCT) DPrint("%s(%s,%s)\n",
    FName,
    (SRName != NULL) ? "SRName" : "NULL",
    (SRPtr != NULL) ? "SRPtr" : "NULL");
 
  if ((SRName == NULL) || (SRName[0] == '\0'))
    {
    return(FAILURE);
    }
 
  if (SRPtr != NULL)
    *SRPtr = NULL;
 
  for (srindex = 0;srindex < MAX_MSRES;srindex++)
    {
    SR = &SRes[srindex];
 
    if ((SR != NULL) && !strcmp(SR->Name,SRName))
      {
      /* SR already exists */

      if (SRPtr != NULL)
        *SRPtr = SR;
 
      return(SUCCESS);
      }
 
    if ((SR != NULL) &&
        (SR->Name[0] != '\0') &&
        (SR->Name[0] != '\1'))
      {
      continue;
      }

    /* empty slot found */       

    /* create/initialize new record */
 
    if (SR == NULL)
      {
      if (MSRCreate(SRName,&SRes[srindex]) == FAILURE)
        {
        DBG(1,fALL) DPrint("ERROR:    cannot alloc memory for SR '%s'\n",
          SRName);
 
        return(FAILURE);
        }
 
      SR = &SR[srindex];
      }

    /* initialize empty record */

    MSRInitialize(SR,SRName);
 
    if (SRPtr != NULL)
      *SRPtr = SR;
 
    SR->Index = srindex;
 
    /* update SR record */
 
    if (MSched.Mode != msmSim)
      MCPRestore(mcpSRes,SRName,(void *)SR);
 
    DBG(5,fSTRUCT) DPrint("INFO:     SR %s added\n",
      SRName);
 
    return(SUCCESS);
    }    /* END for (srindex) */
 
  /* end of table reached */
 
  DBG(1,fSTRUCT) DPrint("ALERT:    SR table overflow.  cannot add %s\n",
    SRName);
 
  return(SUCCESS);
  }  /* END MSRAdd() */




int MSRCreate(

  char   *SRName,  /* I */
  sres_t *SR)      /* I (modified) */

  {
  if (SR == NULL)
    {
    return(FAILURE);
    }

  /* use static memory for now */

  memset(SR,0,sizeof(sres_t));

  if ((SRName != NULL) && (SRName[0] != '\0'))
    MUStrCpy(SR->Name,SRName,sizeof(SR->Name));

  return(SUCCESS);
  }  /* END MSRCreate() */




int MSRSelectNodeList(
 
  mjob_t        *J,          /* I */
  sres_t        *SR,         /* I */
  nodelist_t     DstNL,      /* O:  selected nodes */
  int           *NodeCount,  /* O:  number of nodes found */
  long           StartTime,  /* I */
  nodelist_t     ReqNL,      /* I */
  unsigned long  ResMode)    /* I */
 
  {
  int         TC;
  
  int         BestTC;
  mpar_t     *BestP;

  int         MaxTasks;
 
  int         rqindex;
  int         nindex;
  int         nindex2;
 
  int         pindex;
 
  nodelist_t *FNL; 
 
  mnodelist_t AvailMNL;
  mnodelist_t tmpMNL;

  mnodelist_t BestFMNL;
 
  char        NodeMap[MAX_MNODE];

  mpar_t     *P;
 
  mreq_t     *RQ;

  int         IsSizeFlex;  /* (boolean) */
  int         IsSpaceFlex; /* (boolean) */

  const char *FName = "MSRSelectNodeList";
 
  DBG(3,fSCHED) DPrint("%s(%s,%s,DstNL,NodeCount,%s,ReqNL,%ld)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    SR->Name,
    MULToTString(StartTime - MSched.Time),
    ResMode);

  if (J == NULL)
    return(FAILURE);

  /* NOTE:  only support single req SR's */
 
  TC     = 0;

  BestTC = 0;
  BestP  = NULL;

  IsSizeFlex = TRUE;

  MaxTasks   = J->Request.TC;

  if (IsSizeFlex == TRUE)
    {
    J->TaskCount  = 1;
    J->Req[0]->TaskCount = 1;
    }
 
  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];
 
    for (pindex = 0;pindex < MAX_MPAR;pindex++)
      {
      P = &MPar[pindex];

      if (P->CRes.Procs <= 0)
        continue;
 
      if (SR->PName[0] != '\0')
        {
        if (!strcmp(SR->PName,"ALL"))
          {
          /* reservation may span */

          if (pindex != 0)
            continue;
          }
        else
          {
          if (strcmp(P->Name,SR->PName))
            continue;
          }
        }
      else
        {
        /* reservation may not span */
		  
        if (pindex == 0) 
          continue;
        }  /* END else (SR->PName[0] != '\0') */
 
      if ((strcmp(SR->PName,"ALL") != 0) &&
          (P->Index == 0))
        {
        continue;
        }
 
      DBG(6,fSCHED) DPrint("INFO:     checking partition %s for resources for reservation %s\n",
        P->Name,
        SR->Name);
 
      memset(tmpMNL,0,sizeof(mnodelist_t));
 
      FNL = (nodelist_t *)tmpMNL[rqindex];
 
      if (MReqGetFNL(
            J,
            RQ,
            P,
            NULL,
            *FNL,
            NodeCount,
            &TC,
            MAX_MTIME,
            ResMode) == FAILURE)
        {
        DBG(6,fSCHED) DPrint("INFO:     cannot get feasible tasks for job %s:%d in partition %s\n",
          J->Name,
          rqindex,
          P->Name);
 
        TC = 0;
 
        continue;
        }

      if (TC > BestTC)
        {
        BestTC = TC;
        BestP  = P; 

        /* save best feasible list */

        memcpy(BestFMNL,tmpMNL,sizeof(mnodelist_t));
        }

      /* NOTE:  should be modified to select partition with most tasks */
      }     /* END for (pindex) */
 
    if (BestP == NULL) 
      {
      /* no feasible nodes located */
 
      DBG(6,fSCHED) DPrint("INFO:     no feasible tasks found for job %s:%d in partition %s\n",
        J->Name,
        rqindex,
        SR->PName);
 
      DstNL[0].N = NULL;
 
      return(FAILURE);
      }
    }     /* END for (rqindex) */

  /* NOTE:  SR->TaskCount should only 'cap' SR */

  if (!(SR->Flags & (1 << mrfSpaceFlex)) &&
     ((ResMode & (1 << stForce)) ||
      (SR->TaskCount == 0) ||
      (SR->Flags & (1 << mrfForce))))
    {
    /* NOTE: HostExpression specified or TaskCount not specified, */
    /* select from all feasible nodes */

    IsSpaceFlex = FALSE;
    }
  else
    {
    IsSpaceFlex = TRUE;
    }

  RQ = J->Req[0];  /* FIXME */
 
  if (IsSpaceFlex == FALSE)
    {
    /* NOTE:  make all feasible nodes available for allocation selection *
     * regardless of current state */

    /* if HostExpression specified or if TaskCount not specified, */
    /* select from all feasible nodes */
 
    memset(NodeMap,nmNone,sizeof(NodeMap));
 
    memcpy(AvailMNL,BestFMNL,sizeof(AvailMNL));

    TC = BestTC;
    }
  else
    {
    /* determine available nodes from feasible nodes */

    if (MJobGetAMNodeList(
          J,
          BestFMNL,
          AvailMNL,
          NodeMap, 
          NodeCount,
          &TC,
          StartTime) == FAILURE)
      {
      DBG(4,fSCHED) DPrint("INFO:     cannot locate available tasks in partition %s for job %s at time %ld\n",
        SR->PName,
        J->Name,
        StartTime);
 
      DstNL[0].N = NULL;
 
      return(FAILURE);
      }
    else
      {
      TC            = MIN(MaxTasks,TC);
 
      J->TaskCount  = TC;
      RQ->TaskCount = TC;
      }
    }    /* END else (IsSizeFlex == TRUE) */
 
  for (nindex = 0;ReqNL[nindex].N != NULL;nindex++)
    {
    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      for (nindex2 = 0;AvailMNL[rqindex][nindex2].N != NULL;nindex2++)
        {
        if (AvailMNL[rqindex][nindex2].N ==
              ReqNL[nindex].N)
          {
          NodeMap[ReqNL[nindex].N->Index] = nmRequired;
 
          break;
          }
        }    /* END for (nindex2) */
      }      /* END for (rqindex) */
    }        /* END for (nindex)  */

  if (IsSizeFlex == TRUE)
    { 
    /* adjust required task count */
 
    if (SR->TaskCount > 0)
      J->Request.TC = MIN(TC,SR->TaskCount);
    else
      J->Request.TC = TC;
 
    RQ->TaskCount = J->Request.TC;
    }  /* END if (SizeFlex == TRUE) */
 
  if (MJobAllocMNL(
        J,
        AvailMNL,
        NodeMap,
        tmpMNL,
        MPar[J->Req[0]->PtIndex].NAllocPolicy,
        StartTime) == FAILURE)
    {
    DBG(0,fSCHED) DPrint("ERROR:    cannot allocate best nodes for job '%s'\n",
      J->Name);
 
    return(FAILURE);
    }
 
  RQ->TaskCount = 0;  /* FIXME??? */
 
  for (nindex = 0;tmpMNL[0][nindex].N != NULL;nindex++)
    {
    if (tmpMNL[0][nindex].TC == 0)
      break;
 
    RQ->TaskCount += tmpMNL[0][nindex].TC;
 
    memcpy(&DstNL[nindex],&tmpMNL[0][nindex],sizeof(mnalloc_t));
    }  /* END for (nindex) */
 
  DstNL[nindex].N = NULL;
 
  if (NodeCount != NULL)
    *NodeCount = nindex; 
 
  if (nindex == 0)
    {
    DBG(2,fSCHED) DPrint("INFO:     no tasks found for job %s\n",
      J->Name);
 
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MSRSelectNodeList() */




int MSRCheckReservation(
 
  sres_t *SR,
  mres_t *R)
 
  {
  int nindex;
  int nrindex;
 
  int TC;
 
  mnode_t *N;

  char tmpLine[MAX_MLINE];
 
  /* return SUCCESS if complete reservation located    */
  /* return if reservation lacks adequate resources or */
  /* has invalid resources allocated                   */
 
  TC = 0;
 
  if ((SR == NULL) || (R == NULL))
    return(FAILURE);
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;
 
    for (nrindex = 0;nrindex < MSched.ResDepth;nrindex++)
      {
      if (N->R[nrindex] == NULL)
        break;
 
      if (N->R[nrindex] != R)
        continue;
 
      /* reservation located on node */
 
      if ((SR->TaskCount > 0) &&
          (R->StartTime <= MSched.Time) &&
          (N->State != mnsIdle) &&
          (N->State != mnsActive) &&
          (N->State != mnsBusy))
        {
        /* floating reservation contains node in invalid state */
 
        return(FAILURE);
        }
 
      TC += N->RC[nrindex];
 
      break;
      }  /* END for (nrindex) */
    }    /* END for (nindex)  */

  if (TC < R->TaskCount)
    {
    sprintf(tmpLine,"RESERVATIONCORRUPTION:  reservation corruption detected in reservation '%s'  Req/Detected TC %d/%d\n",
      R->Name,
      R->TaskCount,
      TC);

    MSysRegEvent(tmpLine,0,0,1);

    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MSRCheckReservation() */





int MSRRefresh()
 
  {
  sres_t  tmpSR;
  sres_t *SR;
 
  int     SRIndex;
  int     DIndex;
 
  char    Incomplete[MAX_MSRES];
  int     SRRefreshRequired;

  SR = &tmpSR;
 
  for (SRIndex = 0;SRIndex < MAX_MSRES;SRIndex++)
    {
    if (SRes[0].Name[0] == '\0')
      break;

    MSRGetCurrentValues(&SRes[SRIndex],&OSRes[SRIndex],SR);
 
    if (SR->TaskCount == 0)
      {
      if (SR->HostExpression[0] == '\0')
        {
        /* continue if SR not specified */
 
        DBG(7,fCORE) DPrint("INFO:     SR[%d] is empty (TC: %d  HL: '%s')\n",
          SRIndex,
          SR->TaskCount,
          SR->HostExpression);
        }
      else
        {
        /* continue if exact hosts specified */
 
        /* all hosts located and incorporated in reservation at node load time */
 
        DBG(5,fCORE) DPrint("INFO:     SR[%d] exactly specifies nodes (TC: %d  HL: '%s')\n",
          SRIndex,
          SR->TaskCount,
          SR->HostExpression);
        }
 
      continue;
      }
 
    memset(Incomplete,FALSE,sizeof(Incomplete));
    SRRefreshRequired = FALSE;
 
    for (DIndex = 0;DIndex < SR->Depth;DIndex++)
      {
      if ((SR->R[DIndex] == NULL) ||
          (SR->R[DIndex]->TaskCount < SR->TaskCount) ||
          (MSRCheckReservation(SR,SR->R[DIndex]) == FAILURE))
        {
        Incomplete[DIndex] = TRUE;
        SRRefreshRequired  = TRUE;
        }
      }  /* END for (DIndex) */

    if ((SRRefreshRequired == TRUE) ||
        (MACLGet(SR->ACL,maDuration,NULL,NULL) == FAILURE))
      {
      MSim.QueueChanged = TRUE;
      MSched.EnvChanged = TRUE;
      }
    else 
      {
      /* standing reservation does not need to be refreshed */
 
      continue;
      }
 
    for (DIndex = 0;DIndex < MAX_SRES_DEPTH;DIndex++)
      {
      if (DIndex >= SR->Depth)
        break;
 
      if (Incomplete[DIndex] == FALSE)
        {
        continue;
        }
 
      if (MSRSetRes(SR,SRIndex,DIndex) == SUCCESS)
        {
        if (SR->R[DIndex] != NULL)
          {
          while (SR->R[DIndex]->TaskCount < SR->TaskCount)
            {
            if (MResPreempt(SR->R[DIndex]) == FAILURE)
              break;
 
            MSRSetRes(SR,SRIndex,DIndex);
            }  /* END while (SR->R[DIndex]->TaskCount) */
          }
        }      /* END if (SRCreate() == SUCCESS)       */
      else
        {
        DBG(2,fSCHED) DPrint("ALERT:    cannot create standing reservation '%s'\n",
          SR->Name);
        }
      }      /* END for (DIndex)  */
    }        /* END for (SRIndex) */
 
  return(SUCCESS);
  }  /* END MSRRefresh() */




int MSRProcessConfig(

  sres_t *SR,
  char   *Value)

  {
  int   aindex;

  int   ValCmp;

  char *ptr;
  char *TokPtr;

  char *ptr2;
  char *TokPtr2;

  char  ValLine[MAX_MLINE];
  char *ValList[2];

  char  tmpLine[MAX_MLINE];

  const char *FName = "MSRProcessConfig";

  DBG(2,fSCHED) DPrint("%s(SR,%s)\n",
    FName,
    (Value != NULL) ? Value : "NULL");

  if ((SR == NULL) || 
      (Value == NULL) ||
      (Value[0] == '\0'))
    {
    return(FAILURE);
    }

  ValList[0] = tmpLine;
  ValList[1] = NULL;        

  /* process value line */
 
  ptr = MUStrTok(Value," \t\n",&TokPtr);
 
  while(ptr != NULL)
    {
    /* parse name-value pairs */
 
    /* FORMAT:  <ATTR><CMP><VALUE>[,<VALUE>] */
 
    if (MUGetPair(
          ptr,
          (const char **)MSResAttr,
          &aindex,
	  NULL,
          FALSE,
          &ValCmp,
          ValLine,
          MAX_MLINE) == FAILURE)
      {
      /* cannot parse value pair */

      DBG(2,fSCHED) DPrint("ALERT:    cannot parse SR[%s] config attribute '%s'\n",
        SR->Name,
        ptr);
 
      ptr = MUStrTok(NULL," \t\n",&TokPtr);
 
      continue;
      }

    sprintf(tmpLine,"%s%s",
      MComp[ValCmp],
      ValLine);

    switch(aindex)
      {
      case msraAccess:

        if (!strcmp(ValLine,"SHARED"))
          {
          if (SR->Flags & (1 << mrfDedicatedResource))
            SR->Flags ^= (1 << mrfDedicatedResource);
          }
        else if (!strcmp(ValLine,"DEDICATED"))
          {
          SR->Flags |= (1 << mrfDedicatedResource);
          }

        break;

      case msraAccountList:

        MACLLoadConfig(SR->ACL,ValList,1,maAcct);

        break;

      case msraChargeAccount:

	MAcctAdd(ValLine,&SR->A);

        break;

      case msraClassList:

        MACLLoadConfig(SR->ACL,ValList,1,maClass);  

        break;

      case msraDays:

        /* FORMAT:  ALL|MON|TUE|... */

        SR->Days = 0;

        if (strstr(ValLine,"ALL") != NULL)
          {
          SR->Days = 255;
          }
        else
          { 
          int dindex;

          for (dindex = 0;MWeekDay[dindex] != NULL;dindex++)
            {
            if ((strstr(ValLine,MWeekDay[dindex]) != NULL) ||
                (strstr(ValLine,MWEEKDAY[dindex]) != NULL))
              {
              SR->Days |= (1 << dindex);
              }
            }   /* END for (dindex) */
          }     /* END else (strstr(ValLine,"ALL") != NULL) */   
 
        break;

      case msraDepth:

        SR->Depth = (int)strtol(ValLine,NULL,0);

        break;

      case msraEndTime:

        SR->EndTime = MUTimeFromString(ValLine);

        break;

      case msraFlags:

        MUBMFromString(ValLine,MResFlags,(unsigned long *)&SR->Flags);

        break;

      case msraGroupList:

        MACLLoadConfig(SR->ACL,ValList,1,maGroup);   

        break;

      case msraHostList:

        {
        char *ptr;

        MUStrCpy(SR->HostExpression,ValLine,sizeof(SR->HostExpression));

        /* replace commas with spaces */

        while ((ptr = strchr(SR->HostExpression,',')) != NULL)
          {
          *ptr = ' ';
          }  /* END while() */
        }    /* END BLOCK */
 
        break;

      case msraIdleTime:

        SR->MaxIdleTime = MUTimeFromString(ValLine);   

        break;

      case msraJobAttrList:

        MACLLoadConfig(SR->ACL,ValList,1,maJFeature);    

        break;

      case msraMaxTime:

        { 
        long MaxTime = MUTimeFromString(ValLine);

        mbool_t Required = FALSE;

        if (strchr(ValLine,'*') != NULL)
          Required = TRUE;
 
        MACLSet(
          SR->ACL,
          maDuration,
          &MaxTime,
          mcmpLE,
          nmPositiveAffinity,
          (Required == TRUE) ? (1 << maclRequired) : 0,
          0);
        }  /* END BLOCK */
 
        break;

      case msraNodeFeatures:

        if (!strcmp(ValLine,NONE))
          break;
 
        ptr2 = MUStrTok(ValLine,"[] \t",&TokPtr2);
 
        while (ptr2 != NULL)
          {
          MUGetMAttr(eFeature,ptr2,mAdd,SR->FeatureMap,sizeof(SR->FeatureMap));
 
          ptr2 = MUStrTok(NULL,"[] \t",&TokPtr2);
          }

        break;

      case msraOwner:

        MSRSetAttr(SR,aindex,(void **)ValLine,mdfString,mSet);

        break;

      case msraPartition:

        MUStrCpy(SR->PName,ValLine,sizeof(SR->PName));       

        break;

      case msraPeriod:

        SR->Period = MUGetIndex(ValLine,MSRPeriodType,FALSE,mpDay);

        break;

      case msraPriority:

        SR->Priority = strtol(ValLine,NULL,0);

        break;

      case msraProcLimit:

        ValList[0] = ValLine;
 
        MACLLoadConfig(SR->ACL,ValList,1,maProc);
 
        break;

      case msraQOSList:

        MACLLoadConfig(SR->ACL,ValList,1,maQOS);  

        break;

      case msraResources:

        /* FORMAT: <X>=<Y>[+<X>=<Y>]... */

        memset(&SR->DRes,0,sizeof(SR->DRes));

        MUCResFromString(&SR->DRes,ValLine); 

        break;

      case msraStartTime:

        SR->StartTime = MUTimeFromString(ValLine);     

        break;

      case msraTaskCount:

        SR->TaskCount = strtol(ValLine,NULL,0);      

        break;

      case msraTaskLimit:

        ValList[0] = ValLine;
 
        MACLLoadConfig(SR->ACL,ValList,1,maTask);
 
        break;

      case msraTimeLimit:

        ValList[0] = ValLine;      
 
        MACLLoadConfig(SR->ACL,ValList,1,maDuration);
 
        break;

      case msraTPN:

        SR->TPN = (int)strtol(ValLine,NULL,0);          

        break;

      case msraUserList:

        MACLLoadConfig(SR->ACL,ValList,1,maUser);   

        break;

      case msraWEndTime:

        SR->WEndTime = MUTimeFromString(ValLine);    

        break;

      case msraWStartTime:

        SR->WStartTime = MUTimeFromString(ValLine);      

        break;
 
      default:
 
        DBG(4,fFS) DPrint("WARNING:  SR attribute '%s' not handled\n",
          MSResAttr[aindex]);
 
        break;
      }  /* END switch(aindex) */
 
    ptr = MUStrTok(NULL," \t\n",&TokPtr);
    }  /* END while (ptr != NULL) */

  return(SUCCESS);
  }  /* END MSRProcessConfig() */




int MSRShow(

  sres_t *SR,      /* I */
  char   *Buffer,  /* O */
  int     VFlag,   /* I */
  int     PIndex)  /* I */

  {
  char tmpLine[MAX_MLINE];
  char SRLine[MAX_MLINE];

  int  index;

  struct {
    int PIndex;
    int OIndex;
    } AList[] = { 
  { msraAccountList, maAcct },
  { msraClassList,   maClass },
  { msraUserList,    maUser },
  { msraGroupList,   maGroup },
  { msraQOSList,     maQOS },
  { -1,              -1 } };

  if ((SR == NULL) || (Buffer == NULL))
    return(FAILURE);

  /* display SR if enabled */

  if ((SR->Name[0] == '\0') ||
     (PIndex != -1))
    {
    return(SUCCESS);
    }

  SRLine[0] = '\0';

  /* show ACLs */

  for (index = 0;AList[index].PIndex != -1;index++)
    {
    if (MACLListShow(
          SR->ACL,
          AList[index].OIndex,
          (1 << mfmHuman),
          tmpLine) == NULL)
      {
      continue;
      }

    if ((tmpLine[0] == '\0') || !strcmp(tmpLine,NONE))
      {
      continue;
      }

    sprintf(temp_str,"%s=%s  ",
      MSResAttr[AList[index].PIndex],
      tmpLine);
    strcat(SRLine,temp_str);
    }  /* END for (index) */

  /* show general attributes */

  if (SR->TaskCount != 0)
    {
    sprintf(temp_str,"%s=%d  ",
      MSResAttr[msraTaskCount],
      SR->TaskCount);
    strcat(SRLine,temp_str);
    }  /* END if (SR->TaskCount != 0) */

  /* show hostlist */

  if (SR->HostExpression[0] != '\0')
    {
    sprintf(temp_str,"%s=%s  ",
      MSResAttr[msraHostList],
      SR->HostExpression);
    strcat(SRLine,temp_str);
    }  /* END if (SR->HostExpression[0] != '\0') */

  /* show all flags */

  if (SR->Flags != 0)
    {
    char tmpLine[MAX_MLINE];

    sprintf(temp_str,"%s=%s  ",
      MSResAttr[msraFlags],
      MUBMToString(SR->Flags,MResFlags,' ',tmpLine,NONE));
    strcat(SRLine,temp_str);
    }

  strcpy(tmpLine,MUMAList(eFeature,SR->FeatureMap,sizeof(SR->FeatureMap)));
 
  if (strcmp(tmpLine,NONE))
    {
    sprintf(temp_str,"%s=%s  ",
      MSResAttr[msraNodeFeatures],
      tmpLine);
    strcat(SRLine,temp_str);
    }

  MUShowSSArray(MSRCfgParm,SR->Name,SRLine,Buffer);      
  
  return(SUCCESS);
  }  /* END MSRShow() */




int MSRDiag(

  sres_t *SRS,
  char   *SBuffer,
  int     SBufSize,
  int     Flags)

  {
  int srindex;
  int rindex;

  char *BPtr;
  int   BSpace;

  sres_t *SR;
  mres_t *R;

  int   IsPartial;

  if (SBuffer == NULL)
    return(FAILURE);

  SBuffer[0] = '\0';

  BPtr   = SBuffer;
  BSpace = SBufSize;

  MUSNPrintF(&BPtr,&BSpace,"evaluating standing reservations\n");

  for (srindex = 0;srindex < MAX_MSRES;srindex++)
    {
    SR = &SRes[srindex];

    if (SR->Name[0] == '\0')
      continue;

    if ((SRS != NULL) && (SRS != SR))
      continue;

    /* evaluate configuration */

    if ((SR->TaskCount <= 0) &&
        (SR->HostExpression[0] == '\0'))
      {
      /* NOTE:  SR does not specify tasks */

      MUSNPrintF(&BPtr,&BSpace,"SR %s does not specify requested resources (taskcount/hostexpression not set\n",
        SR->Name);
 
      continue;
      }

    /* evaluate active reservations */

    for (rindex = 0;rindex < MAX_SRES_DEPTH;rindex++)
      {
      R = SR->R[rindex];

      if (R == NULL)
        {
        /* if reservation should exist, determine why missing */

        /* NYI */

        continue;
        }

      MUSNPrintF(&BPtr,&BSpace,"SR %s has reservation %s created\n",
        SR->Name,
        R->Name);

      IsPartial = FALSE;

      if (SR->TaskCount > 0)
        {
        if (R->TaskCount < SR->TaskCount)
          IsPartial = TRUE;

        MUSNPrintF(&BPtr,&BSpace,"SR reservation %s is %s (TC: %d requested/%d reserved)\n",
          R->Name,
          (IsPartial == TRUE) ? "partial" : "complete",
          SR->TaskCount,
          R->TaskCount);
        } 

      if (IsPartial == TRUE)
        {
        /* determine why resources are unavailable */

        /* NYI */
        }
      }    /* END for (rindex) */
    }      /* END for (srindex) */

  return(FAILURE);
  }  /* END MSRDiag() */




int MSRInitialize(
 
  sres_t *SR,
  char   *Name)
 
  {
  int srindex;

  if ((SR == NULL) || (Name == NULL))
    return(FAILURE);

  for (srindex = 0;srindex < MAX_MSRES;srindex++)
    {
    if (&SRes[srindex] == SR)
      {
      SR->Index = srindex;

      break;
      }
    }    /* END for (srindex) */
 
  MUStrCpy(SR->Name,Name,sizeof(SR->Name));
 
  SR->Type       = mrtUser;
  SR->Depth      = DEFAULT_SRDEPTH;
  SR->Days       = DEFAULT_SRDAYS;
 
  SR->StartTime  = DEFAULT_SRSTARTTIME;
  SR->EndTime    = DEFAULT_SRENDTIME;
 
  SR->WStartTime = DEFAULT_SRSTARTTIME;
  SR->WEndTime   = DEFAULT_SRENDTIME;
 
  SR->DRes.Procs = DEFAULT_SRPROCS;

  if (SR->HostList != NULL)
    MUFree((char **)&SR->HostList);

  SR->Priority   = DEFAULT_SRPRIORITY;
  SR->Flags      = DEFAULT_SRFLAGS;
 
  SR->Period     = DEFAULT_SRPERIOD;

  return(SUCCESS);
  }  /* END MSRInitialize() */




int MSRConfigShow(

  sres_t *SRP,     /* I (optional) */
  int     VFlag,   /* I */
  int     PIndex,  /* I */
  char   *Buffer)  /* O */

  {
  int srindex;

  sres_t *SR;

  char tmpLine[MAX_MLINE];

  if (Buffer == NULL)
    {
    return(FAILURE);
    }

  for (srindex = 0;srindex < MAX_MSRES;srindex++)
    {
    SR = &SRes[srindex];
 
    if ((SR->TaskCount == 0) &&
        (SR->HostExpression[0] == '\0') &&
        !VFlag)
      {
      continue;
      }
 
    if ((PIndex == -1) || (PIndex == pSRTaskCount))
      strcat(Buffer,MUShowIArray(MParam[pSRTaskCount],srindex,SR->TaskCount));
 
    if ((SR->TPN > 0) || (VFlag || (PIndex == -1) || (PIndex == pSRTPN)))
      strcat(Buffer,MUShowIArray(MParam[pSRTPN],srindex,SR->TPN));
 
    if ((SR->DRes.Procs != -1) || (VFlag || (PIndex == -1) || (PIndex == pSRResources)))
      {
      sprintf(tmpLine,"PROCS=%d;MEM=%d;DISK=%d;SWAP=%d",
        SR->DRes.Procs,
        SR->DRes.Mem,
        SR->DRes.Disk,
        SR->DRes.Swap);
 
      strcat(Buffer,MUShowSArray(MParam[pSRResources],srindex,tmpLine));
      }
 
    if ((SR->Depth != DEFAULT_SRDEPTH) || (VFlag || (PIndex == -1) || (PIndex == pSRDepth)))
      strcat(Buffer,MUShowIArray(MParam[pSRDepth],srindex,SR->Depth));
 
    if ((SR->StartTime != -1) || (VFlag || (PIndex == -1) || (PIndex == pSRStartTime)))
      strcat(Buffer,MUShowSArray(MParam[pSRStartTime],srindex,MULToTString(SR->StartTime))); 
 
    if ((SR->EndTime != -1) || (VFlag || (PIndex == -1) || (PIndex == pSREndTime)))
      strcat(Buffer,MUShowSArray(MParam[pSREndTime],srindex,MULToTString(SR->EndTime)));
 
    if ((SR->WStartTime != -1) || (VFlag || (PIndex == -1) || (PIndex == pSRWStartTime)))
      strcat(Buffer,MUShowSArray(MParam[pSRWStartTime],srindex,MULToTString(SR->WStartTime)));
 
    if ((SR->WEndTime != -1) || (VFlag || (PIndex == -1) || (PIndex == pSRWEndTime)))
      strcat(Buffer,MUShowSArray(MParam[pSRWEndTime],srindex,MULToTString(SR->WEndTime)));
 
    if ((SR->Days != 255) || (VFlag || (PIndex == -1) || (PIndex == pSRDays)))
      {
      if (SR->Days == 255)
        {
        strcpy(tmpLine,"ALL");
        }
      else
        {
        int index;

        tmpLine[0] = '\0';
 
        for (index = 0;MWeekDay[index] != NULL;index++)
          {
          if (SR->Days & (1 << index))
            {
            if (tmpLine[0] != '\0')
              strcat(tmpLine,"|");
 
            strcat(tmpLine,MWeekDay[index]);
            }
          }    /* END for (index) */
        }
 
      strcat(Buffer,MUShowSArray(MParam[pSRDays],srindex,tmpLine));
      }  /* END if (SRDays != 255) ... */ 
 
    if ((SR->HostExpression[0] != '\0') || 
        (VFlag || (PIndex == -1) || (PIndex == pSRHostList)))
      {
      strcat(Buffer,MUShowSArray(MParam[pSRHostList],srindex,SR->HostExpression));
      }
 
    if ((SR->A != NULL) || (VFlag || (PIndex == -1) || (PIndex == pSRChargeAccount)))
      strcat(Buffer,MUShowSArray(MParam[pSRChargeAccount],srindex,(SR->A != NULL) ? SR->A->Name : ""));
 
    MSRShow(SR,Buffer,VFlag,PIndex);      

    strcat(Buffer,"\n");       
    }  /* END for (srindex) */
  
  return(SUCCESS);
  }  /* END MSRConfigShow() */




int MSRGetAttributes(
 
  sres_t        *SR,          /* IN:  standing reservation description */
  int            PeriodIndex, /* IN:  period index                     */
  long          *SRStartTime, /* OUT: time reservation should start    */
  unsigned long *SRDuration)  /* OUT: duration of reservation          */
 
  {
  long       SRStartOffset;
  long       SREndOffset;
 
  long       DayTime;
  long       WeekTime;
  int        WeekDay;
 
  struct tm *Time;
 
  long       PeriodStart;
  long       tmpL;
 
  time_t     tmpTime;

  const char *FName = "MSRGetAttributes";
 
  DBG(3,fSTRUCT) DPrint("%s(%s,%d,Start,Duration)\n",
    FName,
    SR->Name,
    PeriodIndex); 
 
  MUGetPeriodStart(MSched.Time,0,PeriodIndex,SR->Period,&PeriodStart);
 
  if (PeriodIndex == 0)
    tmpTime = (time_t)MAX(PeriodStart,MSched.Time);
  else
    tmpTime = (time_t)PeriodStart;
 
  Time = localtime(&tmpTime);
 
  DayTime  = (3600 * Time->tm_hour) + (60 * Time->tm_min) + Time->tm_sec;
  WeekTime = DayTime + (Time->tm_wday * DAY_LEN);
  WeekDay  = Time->tm_wday;
 
  /* set reservation time boundaries */
 
  SRStartOffset = 0;
 
  switch (SR->Period)
    {
    case mpDay:
 
      SREndOffset = DAY_LEN;
 
      if ((SR->WStartTime != -1) &&
          (WeekDay == (SR->WStartTime / DAY_LEN)))
        {
        SRStartOffset = SR->WStartTime % DAY_LEN;
        }
 
      if (SR->StartTime != -1)
        {
        SRStartOffset = SR->StartTime;
        }
 
      if ((SR->WEndTime > 0) &&
          (WeekDay == (SR->WEndTime / DAY_LEN)))
        {
        SREndOffset = SR->WEndTime % DAY_LEN; 
        }
 
      if (SR->EndTime > 0)
        {
        SREndOffset = SR->EndTime;
        }
 
      if (PeriodIndex == 0)
        {
        SRStartOffset = MAX(SRStartOffset,DayTime);
        }
 
      break;
 
    case mpWeek:
 
      SREndOffset = WEEK_LEN;
 
      if (SR->WStartTime > 0)
        {
        SRStartOffset = SR->WStartTime;
        }
 
      if (SR->WEndTime > 0)
        {
        SREndOffset = SR->WEndTime;
        }
 
      if (PeriodIndex == 0)
        {
        SRStartOffset = MAX(SRStartOffset,WeekTime);
        }
 
      break;
 
    case mpInfinity:
    default:
 
      SREndOffset = MAX_MTIME; 
 
      break;
    }  /* END switch(SR->Period) */
 
  /* determine if reservation is needed for current period */
 
  switch (SR->Period)
    {
    case mpDay:
 
      if ((SR->Period == mpWeek) && ((SR->WStartTime / DAY_LEN) > WeekDay))
        {
        /* ignore, week reservation has not yet started */
 
        DBG(6,fSTRUCT) DPrint("INFO:     week reservation %s starts after day %d\n",
          SR->Name,
          WeekDay);
 
        return(FAILURE);
        }
 
      if ((SR->WEndTime > 0) && (SR->WEndTime < WeekTime))
        {
        /* ignore, week reservation has ended */
 
        DBG(6,fSTRUCT) DPrint("INFO:     week reservation %s ends before %s\n",
          SR->Name,
          MULToTString(WeekTime));
 
        return(FAILURE);
        }
 
      if (DayTime > SREndOffset)
        {
        /* ignore, day reservation has already ended */
 
        DBG(6,fSTRUCT) DPrint("INFO:     day reservation %s ends before %s\n",
          SR->Name,
          MULToTString(DayTime)); 
 
        return(FAILURE);
        }
 
      if (!(SR->Days & (1 << WeekDay)))
        {
        /* day period is not included in SRDays */
 
        DBG(6,fSTRUCT) DPrint("INFO:     day reservation %s does not include day %d\n",
          SR->Name,
          WeekDay);
 
        return(FAILURE);
        }
 
      break;
 
    case mpWeek:
 
      if (WeekTime > SREndOffset)
        {
        /* ignore, week reservation has ended */
 
        DBG(6,fSTRUCT) DPrint("INFO:     week reservation %s ends before %s\n",
          SR->Name,
          MULToTString(WeekTime));
 
        return(FAILURE);
        }
 
      break;
 
    case mpInfinity:
    default:
 
      if (PeriodIndex != 0)
        {
        /* ignore, only one infinite period reservation */ 
 
        DBG(6,fSTRUCT) DPrint("INFO:     ignoring infinite reservation %s for period %d\n",
          SR->Name,
          PeriodIndex);
 
        return(FAILURE);
        }
 
      /* reservation not ended */
 
      break;
    }  /* END switch (SR->Period) */
 
  tmpL = (PeriodStart + SRStartOffset);
 
  DBG(5,fSTRUCT) DPrint("INFO:     res start: %s",
    MULToDString((mulong *)&tmpL));
 
  DBG(5,fSTRUCT) DPrint("INFO:     standing res attributes: Start: %ld  Duration: %ld (W: %d  D: %s)\n",
    PeriodStart + SRStartOffset,
    SREndOffset - SRStartOffset,
    WeekDay,
    MULToTString(DayTime));
 
  if (SRDuration != NULL)
    *SRDuration = SREndOffset - SRStartOffset;
 
  if (SRStartTime != NULL)
    *SRStartTime = PeriodStart + SRStartOffset;
 
  return(SUCCESS);
  }  /* END MSRGetAttributes() */


       
 
int MSRGetCurrentValues(

  sres_t *Config,
  sres_t *Override,
  sres_t *SR)

  {
  const char *FName = "MSRGetCurrentValues";

  DBG(6,fSCHED) DPrint("%s(%s,Override,SR)\n",
    FName,
    Config->Name);

  memcpy(SR,Config,sizeof(sres_t));

  if (Override->TaskCount > 0)
    SR->TaskCount = Override->TaskCount;

  if (Override->A != NULL)
    SR->A = Override->A;

  if (Override->StartTime > 0)
    SR->StartTime = Override->StartTime;

  if (Override->EndTime > 0)
    SR->EndTime = Override->EndTime;

  return(SUCCESS);
  }    /* END MSRGetCurrentValues() */





int MSRSetRes(

  sres_t *SR,      /* I */
  int     SRIndex, /* I */
  int     DIndex)  /* I */

  {
  mjob_t      tmpJ;
  mjob_t     *J;

  long        BestTime;

  int         sindex;
  int         aindex;
  int         nindex;

  mnode_t    *N;

  long        SRStartTime;
  unsigned long SRDuration;
  unsigned long SREndTime;

  char        TempString[MAX_MNAME];

  int         TC;
  int         PC;

  nodelist_t  NodeList;

  mnalloc_t   tmpJNodeList[MAX_MNODE + 1];
  mnalloc_t   tmpJReqHList[MAX_MNODE + 1];
  nodelist_t  tmpANodeList;

  mnalloc_t   tmpRQNodeList[MAX_MNODE + 1];
  macl_t      tmpACL[MAX_MACL];
  macl_t      tmpCL[MAX_MACL];         

  mreq_t      tmpRQ;
  mreq_t     *RQ;
 
  mres_t      tmpR;
  mres_t     *R;

  int         Flags;

  int         TaskCount;
  int         ProcCount;
  int         NodeCount;            

  enum MHoldReasonEnum Reason;

  int         IsSpaceFlex; /* (boolean) */

  double      CurrentIdlePS;
  double      CurrentActivePS;
  double      TotalIdlePS;
  double      TotalActivePS;

  unsigned long Mode;

  mnalloc_t *HL;

  const char *FName = "MSRSetRes";

  DBG(3,fSCHED) DPrint("%s(%s,%d,%d)\n",
    FName,
    (SR != NULL) ? SR->Name : "NULL",
    SRIndex,
    DIndex);

#ifndef __MQ
  if (SR == NULL)
    {
    return(FAILURE);
    }
#endif  /* !__MQ */

  /* build 'standing reservation' job */

  J  = &tmpJ;
  RQ = &tmpRQ;

  tmpACL[0].Type = maNONE;

  memset(RQ,0,sizeof(tmpRQ));

  MJobMkTemp(J,RQ,tmpACL,tmpCL,tmpJNodeList,tmpRQNodeList);

  /* determine reservation name */

  if (SR->Name[0] == '\0')
    {
    sprintf(J->Name,"SR.%d",
      SR->Index);
    }
  else
    {
    strcpy(J->Name,SR->Name);
    }

  sprintf(temp_str,".%d",
    DIndex);
  strcat(J->Name,temp_str);

  J->SpecFlags |= (1 << mjfResMap);

  if (!(SR->Flags & (1 << mrfDedicatedResource)))
    J->SpecFlags |= (1 << mjfSharedResource);

  if (SR->Flags & (1 << mrfByName))
    J->SpecFlags |= (1 << mjfByName);

  if (!strcmp(SR->PName,"ALL"))
    J->SpecFlags |= (1 << mjfSpan);

  if (SR->HostExpression[0] != '\0')
    {
    /* verify hostlist */

    if (SR->HostList == NULL)
      {
      /* restore hostlist */

      SR->HostList = (void *)calloc(1,sizeof(mnalloc_t) * (MAX_MNODE + 1)); 
      }

    HL = (mnalloc_t *)SR->HostList;

    if (HL[0].N == NULL)
      {
      DBG(3,fSCHED) DPrint("INFO:     SR %s hostlist %s\n",
        SR->Name,
        (MSched.Iteration == 0) ? "initialized" : "lost");

      if (MSRBuildHostList(SR) == SUCCESS)
        {
        if (SR != &SRes[SRIndex])
          {
          if (SRes[SRIndex].HostList == NULL)
            {
            /* restore hostlist */

            SRes[SRIndex].HostList = (void *)calloc(
              1,
              sizeof(mnalloc_t) * (MAX_MNODE + 1)); 
            }

          memcpy(
            SRes[SRIndex].HostList,
            SR->HostList,
            (sizeof(mnalloc_t) * (MAX_MNODE + 1)));
          }
        }    /* END if (MSRBuildHostList(SR) == SUCCESS) */
      else
        {
        DBG(3,fSCHED) DPrint("ALERT:    cannot build hostlist for SR %s in %s\n",
          SR->Name,
          FName);
        }  /* END else (MSRBuildHostList(SR) == SUCCESS) */
      }    /* END if (HL[0].N == NULL) */
    }      /* END if (SR->HostExpression[0] != '\0') */

  Mode = 0;      

  /* use best effort in all cases */
 
  if (SR->TaskCount >= 0)
    {
    J->SpecFlags |= (1 << mjfBestEffort);
    Mode         |= (1 << stBestEffort);
    }

  /* initialize requirements */

  memcpy(RQ->ReqFBM,SR->FeatureMap,sizeof(RQ->ReqFBM));

  RQ->RMIndex = -1;

  TaskCount = 0;

  HL = (mnalloc_t *)SR->HostList;

  if ((HL != NULL) && (HL[0].N != NULL))
    {
    J->SpecFlags |= (1 << mjfHostList);

    if (SR->TaskCount == 0)
      J->SpecFlags |= (1 << mjfBestEffort);

    J->ReqHList = &tmpJReqHList[0];

    memcpy(J->ReqHList,HL,sizeof(tmpJReqHList));

    for (nindex = 0;HL[nindex].N != NULL;nindex++)
      {
      N = HL[nindex].N;

      TC = MNodeGetTC(N,&N->CRes,&N->CRes,&N->DRes,&SR->DRes,MAX_MTIME);

      TaskCount += TC;
  
      DBG(5,fUI) DPrint("INFO:     evaluating node %sx%d of hostlist for reservation %s (%d)\n",
        N->Name,
        TC,
        SR->Name,
        TaskCount);
      }  /* END for (nindex) */
    }    /* END if ((HL != NULL) && (HL[0].N != NULL)) */

  if (SR->TaskCount > 0)
    {
    if (SR->TaskCount != TaskCount)
      Mode |= (1 << stBestEffort);

    TaskCount = SR->TaskCount;
    }
  else
    {
    if (TaskCount < 1)
      {
      DBG(0,fUI) DPrint("ALERT:    empty hostlist in %s()\n",
        FName);

      return(FAILURE);
      }
    }    /* END if (SR->TaskCount > 0) */

  if (SR->HostExpression[0] != '\0')
    {
    /* if SR is dedicated, grant global job access */

    if (!(SR->Flags & (1 << mrfDedicatedResource)) || (SR->TaskCount == 0))
      Mode |= (1 << stForce);   
    else
      strcpy(J->RAList[0],"[ALLJOB]");
    }
  else
    {
    /* if no host expression set */

    if (!(SR->Flags & (1 << mrfDedicatedResource)) || 
       (SR->TaskCount == 0))
      {
      Mode |= (1 << stForce);
      }
    }

  /* set credentials */

  /* NOTE:  location sets previous updated job flags */

  if (MJobSetCreds(J,ALL,ALL,ALL) == FAILURE)
    {
    DBG(3,fUI) DPrint("INFO:     cannot setup standing reservation job creds\n");

    return(FAILURE);
    }

  MJobSetQOS(J,MSched.DefaultQ,0);

  if (MSRGetAttributes(
        SR,
        DIndex,
        &SRStartTime,
        &SRDuration) == FAILURE)
    {
    DBG(3,fUI) DPrint("INFO:     reservation not required for specified period\n");

    return(SUCCESS);
    }

  RQ->TasksPerNode  = SR->TPN;

  J->WCLimit        = MIN(SRDuration,MAX_MEFFINF);
  J->SpecWCLimit[0] = J->WCLimit;

  J->Request.TC = TaskCount;
  RQ->TaskCount     = TaskCount;

  BestTime          = MAX(MSched.Time,SRStartTime);

  if (SR->ACL[0].Name[0] != '\0')
    {
    memcpy(J->Cred.CL,SR->ACL,(sizeof(macl_t) * MAX_MACL));

    J->Cred.CredType |= (1 << ctACL);
    }
  else
    {
    J->Cred.CL = NULL;
    }

  memcpy(&RQ->DRes,&SR->DRes,sizeof(mcres_t));

  if (RQ->TaskCount * RQ->DRes.Procs == 0)
    {
    DBG(2,fSCHED) DPrint("ALERT:    no procs requested by reservation (%d:%d)\n",
      RQ->TaskCount,
      RQ->DRes.Procs);

    if (SR->R[DIndex] != NULL)
      {
      MResDestroy(&SR->R[DIndex]);
      }

    return(SUCCESS);
    }

  strcpy(TempString,MULToTString(SRDuration));

  PC = MJobGetProcCount(J);

  DBG(3,fSCHED) DPrint("INFO:     attempting standing reservation of %d procs in %s for %s\n",
    PC,
    MULToTString(SRStartTime - MSched.Time),
    TempString);

  /* SR job creation complete */

  aindex = 0;

  R = SR->R[DIndex];

  if (R != NULL)
    {
    CurrentIdlePS   = R->CIPS;
    CurrentActivePS = R->CAPS;
    TotalIdlePS     = R->TIPS;
    TotalActivePS   = R->TAPS;

    /* get current active reservation list */

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

        if (N->R[sindex] == R)
          {
          if ((N->State == mnsBusy) || (N->State == mnsActive))
            {
            /* add node to list */

            tmpANodeList[aindex].N  = N;
            tmpANodeList[aindex].TC = 1;

            aindex++;
            }
          }
        }    /* END for (sindex) */
      }      /* END for (nindex) */
    }        /* END if (R != NULL) */
  else
    {
    CurrentIdlePS   = 0.0;
    CurrentActivePS = 0.0;
    TotalIdlePS     = 0.0;
    TotalActivePS   = 0.0;
    }        /* END else (R != NULL) */

  tmpANodeList[aindex].N = NULL;

  if (MSRSelectNodeList(
        J,
        SR,
        NodeList,
        &NodeCount,
        BestTime,
        tmpANodeList,
        Mode) == FAILURE)
    {
    DBG(1,fSCHED) DPrint("ALERT:    cannot select %d procs in partition '%s' for SR '%s'\n",
      PC,
      (SR->PName[0] != '\0') ? SR->PName : ALL,
      SR->Name);

    return(FAILURE);
    }

  BestTime = MAX(BestTime,MSched.Time);

  Flags = ((1 << mrfStandingRes) | SR->Flags);

  if (SR->Flags & (1 << mrfSpaceFlex))
    IsSpaceFlex = TRUE;
  else
    IsSpaceFlex = FALSE;

  R = SRes[SRIndex].R[DIndex];

  if (R != NULL)
    {
    memcpy(&tmpR,R,sizeof(tmpR));

    R->A = NULL;
 
    MResDestroy(&SRes[SRIndex].R[DIndex]);

    R = NULL;
    }

  ProcCount = 0;

  if ((SR->A != NULL) &&
       strcmp(SR->A->Name,NONE))
    {
    for (nindex = 0;NodeList[nindex].N != NULL;nindex++)
      {
      if (RQ->DRes.Procs > 0)
        ProcCount += RQ->DRes.Procs * NodeList[nindex].TC;
      else
        ProcCount += NodeList[nindex].N->CRes.Procs;
      }
    }

  SREndTime = MIN(MAX_MTIME,(unsigned long)BestTime + SRDuration);

  if (MResCreate(
        mrtUser,
        SR->ACL,
        (SR->A != NULL) ? SR->A->Name : NULL,
        Flags,
        (mnalloc_t *)NodeList,
        BestTime,
        (long)SREndTime,
        NodeCount,
        ProcCount,
        J->Name,
        &SRes[SRIndex].R[DIndex],
        SR->HostExpression,
        &RQ->DRes) == FAILURE)
    {
    DBG(1,fSCHED) DPrint("ALERT:    cannot reserve %d procs in partition '%s' for standing reservation\n",
      SR->TaskCount * SR->DRes.Procs,
      (SR->PName[0] != '\0') ? SR->PName : "[ANY]");

    /* charge for previous segment */

    if (IsSpaceFlex == TRUE)
      {
      if (tmpR.CIPS > 0.0)
        {
        if (MAMAllocRDebit(&MAM[0],&tmpR,&Reason,NULL) == FAILURE)
          {
          DBG(1,fSTAT) DPrint("ALERT:    cannot charge %6.2lf PS to account %s for reservation %s\n",
            tmpR.CIPS,
            (tmpR.A != NULL) ? tmpR.A->Name : NONE,
            tmpR.Name);
          }
        }
      else
        {
        /* cancel account reservation */

        if (MAMAllocResCancel(
              (tmpR.A != NULL) ? tmpR.A->Name : NULL,
              tmpR.Name,
              "standing reservation removed",
              NULL,
              &Reason) == FAILURE)
          {
          DBG(1,fSCHED) DPrint("ERROR:    cannot cancel allocation reservation for reservation %s, reason: %s\n",
            tmpR.Name,
            MDefReason[Reason]);
          }
        }
      }      /* END if (IsSpaceFlex == TRUE) */ 

    return(FAILURE);
    }  /* END if (MResCreate() == FAILURE) */

  R = SRes[SRIndex].R[DIndex];

  R->CIPS = CurrentIdlePS;
  R->CAPS = CurrentActivePS;
  R->TIPS = TotalIdlePS;
  R->TAPS = TotalActivePS;

  R->Priority        = SR->Priority;

  if ((SR->A != NULL) && strcmp(SR->A->Name,NONE))
    {
    R->A = SR->A;
    }

  R->O     = SR->O;
  R->OType = SR->OType;

  PC = MJobGetProcCount(J);

  if (PC == R->AllocPC)
    {
    DBG(2,fSCHED) DPrint("INFO:     full SR reserved %d procs in partition '%s' to start in %s at (%ld) %s",
      PC,
      (SR->PName[0] != '\0') ? SR->PName : "[ALL]",
      MULToTString(BestTime - MSched.Time),
      BestTime,
      MULToDString((mulong *)&BestTime));
    }
  else
    {
    DBG(2,fSCHED) DPrint("WARNING:  partial standing reservation %s reserved %d of %d procs in partition '%s' to start in %s at (%ld) %s",
      SR->Name,
      R->AllocPC,
      PC,
      (SR->PName[0] != '\0') ? SR->PName : "[ALL]",
      MULToTString(BestTime - MSched.Time),
      BestTime,
      MULToDString((mulong *)&BestTime));
    }

  return(SUCCESS);
  }  /* END MSRSetRes() */






int MSRUpdate(

  sres_t *SSR) /* I (optional) */

  {
  int     SRIndex;
  sres_t *SR;

  int     DIndex;

  char    ResName[MAX_MNAME];

  sres_t  tmpSR;

  const char *FName = "MSRUpdate";

  DBG(4,fCORE) DPrint("%s()\n",
    FName);

  SR = &tmpSR;

  for (SRIndex = 0;SRIndex < MAX_MSRES;SRIndex++)
    {
    if (SRes[SRIndex].Name[0] == '\0')
      break;

    if ((SSR != NULL) && (SSR != &SRes[SRIndex]))
      continue;

    MSRGetCurrentValues(&SRes[SRIndex],&OSRes[SRIndex],SR);

    /* erase expired reservation */

    if (SR->R[0] != NULL)
      {
      if (SR->R[0]->EndTime > MSched.Time)
        continue;

      MResDestroy(&SR->R[0]);

      SRes[SRIndex].R[0] = NULL;
      }

    if (SR->HostExpression[0] != '\0')
      {
      if (MSRBuildHostList(SR) == SUCCESS)
        {
        if (SR != &SRes[SRIndex])
          {
          if (SRes[SRIndex].HostList == NULL)
            {
            SRes[SRIndex].HostList = 
              (void *)calloc(1,sizeof(mnalloc_t) * (MAX_MNODE + 1));
            }

          memcpy(
            SRes[SRIndex].HostList,
            SR->HostList,
            sizeof(SRes[SRIndex].HostList));
          }
        }
      else
        {
        DBG(4,fCORE) DPrint("ALERT:    cannot create hostlist for SR %s in %s\n",
          SR->Name,
          FName);
        } 
      }         /* END if (SR->HostExpression[0] == '\0') */
    else
      {
      if (SR->TaskCount == 0)
        continue;
      }

    /* roll standing reservations forward */

    for (DIndex = 0;DIndex < (MAX_SRES_DEPTH - 1);DIndex++)
      {
      if (DIndex >= (SR->Depth - 1))
        break;

      SR->R[DIndex]               = SR->R[DIndex + 1];
      SRes[SRIndex].R[DIndex]     = SR->R[DIndex + 1];

      SR->R[DIndex + 1]           = NULL;
      SRes[SRIndex].R[DIndex + 1] = NULL;

      if (SR->R[DIndex] == NULL)
        {
        /* create missing reservations */

        MSRSetRes(SR,SRIndex,DIndex);
        }
      else
        {
        /* update name of standing reservation */

        if (SR->Name[0] == '\0')
          {
          sprintf(ResName,"SR.%d",
            SR->Index);
          }
        else
          {
          strcpy(ResName,SR->Name);
          }

        sprintf(temp_str,".%d",
          DIndex);
        strcat(ResName,temp_str);

        /* determine unique reservation name */

        DBG(5,fSTRUCT) DPrint("INFO:     creating reservation '%s'\n",
          ResName);

        strcpy(SR->R[DIndex]->Name,ResName);
        }  /* END else (SR->R[DIndex] == NULL) */
      }    /* END for (DIndex)                 */

    if (MSRSetRes(SR,SRIndex,DIndex) == FAILURE)
      {
      DBG(2,fCORE) DPrint("ALERT:    cannot create SR[%d] %s at depth %d\n",
        SRIndex,
        SR->Name,
        DIndex);
      }
    }  /* END for (SRIndex) */

  return(SUCCESS);
  }  /* END MSRUpdate() */




int MSRBuildHostList(

  sres_t *SR)  /* I */

  {
  int     nindex;
  int     oindex;

  int     NodeCount;

  char    Expr[MAX_MBUFFER];
  char    Buffer[MAX_MBUFFER];

  short   ObjList[MAX_MNODE];

  int     TC;

  char   *ptr;
  char   *TokPtr;

  mnalloc_t *HL;

  mnode_t *N;

  mpar_t *P;

  nindex = 0;

  if (SR->HostList == NULL)
    {
    SR->HostList = (void *)calloc(1,sizeof(mnalloc_t) * (MAX_MNODE + 1));
    }

  HL = (mnalloc_t *)SR->HostList;

  HL[0].N = NULL;

  strcpy(Expr,SR->HostExpression);

  ptr = MUStrTok(Expr," ",&TokPtr);

  if (SR->PName[0] == '\0')
    {
    P = &MPar[0];
    }
  else
    {
    if (MParFind(SR->PName,&P) == FAILURE)
      {
      /* requested partition does not exist */

      return(FAILURE);
      }
    }

  while (ptr != NULL)
    {
    Buffer[0] = '\0';

    if (MUREToList(ptr,mxoNode,P->Index,ObjList,&NodeCount,Buffer) == FAILURE)
      {
      DBG(2,fCONFIG) DPrint("ALERT:    cannot expand hostlist '%s'\n",
        ptr);

      ptr = MUStrTok(NULL," ",&TokPtr);

      continue;
      }

    ptr = MUStrTok(NULL," ",&TokPtr);

    /* populate hostlist */

    for (oindex = 0;oindex < NodeCount;oindex++)
      {
      for (nindex = 0;nindex < MAX_MNODE;nindex++)
        {
        if (HL[nindex].N == NULL)
          {
          /* add node to list */

          HL[nindex].N  = MNode[ObjList[oindex]];
          HL[nindex].TC = 1;
  
          HL[nindex + 1].N = NULL;

          break;
          }
 
        if (HL[nindex].N->Index == ObjList[oindex])
          {
          /* node previously added */

          break;
          }
        }   /* END for (nindex)        */
      }     /* END for (oindex)        */
    }       /* END while (ptr != NULL) */

  if (HL[0].N == NULL)
    {
    return(FAILURE);
    }

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    if (HL[nindex].N == NULL)
      break;

    N = HL[nindex].N;

    TC = MNodeGetTC(N,&N->ARes,&N->CRes,&N->DRes,&SR->DRes,MAX_MTIME);

    if (SR->TaskCount > 0)
      {
      HL[nindex].TC = (short)MIN(TC,SR->TaskCount);
      }
    else
      {
      HL[nindex].TC = (short)TC;
      }
    }       /* END for (nindex) */

  return(SUCCESS);
  }  /* END MSRBuildHostList() */




int MSRProcessOConfig(

  sres_t *SR,
  int     PIndex,
  int     IVal,
  double  DVal,
  char   *SVal,
  char  **SArray)
 
  {
  if (SR == NULL)
    return(FAILURE);
 
  switch (PIndex)
    {
    case pSRAccess:
 
      if (!strcmp(SVal,"SHARED"))
        {
        if (SR->Flags & (1 << mrfDedicatedResource))
          SR->Flags ^= (1 << mrfDedicatedResource);
        }
 
      break;

    case pSRAccountList:
 
      {
      int index;
 
      for (index = 0;SArray[index] != NULL;index++);
 
      MACLLoadConfig(SR->ACL,SArray,index,maAcct);
      }
 
      break;

   case pSRChargeAccount:

      MAcctAdd(SVal,&SR->A); 
 
      break;

    case pSRClassList:
 
      {
      int index;
 
      for (index = 0;SArray[index] != NULL;index++);
 
      MACLLoadConfig(SR->ACL,SArray,index,maClass);
      }
 
      break;

    case pSRDays:

      {
      int index;
      int dindex;

      SR->Days = 0;
 
      for (index = 0;SArray[index] != NULL;index++)
        {
        if (strstr(SArray[index],"ALL") != NULL)
          {
          SR->Days |= 255;

          break;
          }
        else
          {
          for (dindex = 0;MWeekDay[dindex] != NULL;dindex++)
            {
            if ((strstr(SArray[index],MWeekDay[dindex]) != NULL) ||
                (strstr(SArray[index],MWEEKDAY[dindex]) != NULL))
              {
              SR->Days |= (1 << dindex);
              } 
            }   /* END for (dindex) */
          }     /* END else (strstr() != NULL) */
        }       /* END for (index) */

      DBG(2,fALL) DPrint("INFO:     SRes[%s].Days set to %x\n",
        SR->Name,
        SR->Days);
      }  /* END BLOCK */
    
      break;

    case pSRDepth:

      SR->Depth = IVal;
 
      break;

    case pSREndTime:
 
      SR->EndTime = MUTimeFromString(SVal);
 
      break;

    case pSRFeatures:

      {
      char *ptr;
      char *TokPtr;

      int index;
      int FMap[(MAX_MATTR >> 5) + 1];

      MUBMClear(FMap,MAX_MATTR);

      for (index = 0;SArray[index] != NULL;index++)
        {
        ptr = MUStrTok(SArray[index],"[] \t",&TokPtr);
 
        while (ptr != NULL)
          {
          MUGetMAttr(eFeature,ptr,mAdd,FMap,sizeof(FMap));

          MUBMOR(SR->FeatureMap,FMap,MAX_MATTR);
 
          ptr = MUStrTok(NULL,"[] \t",&TokPtr);
          }
        }    /* END for (index) */
      }      /* END BLOCK */
 
      break;

    case pSRFlags:
 
      MUBMFromString(SVal,MResFlags,(unsigned long *)&SR->Flags);
 
      break;

    case pSRGroupList:
 
      {
      int index;
 
      for (index = 0;SArray[index] != NULL;index++);
 
      MACLLoadConfig(SR->ACL,SArray,index,maGroup);
      }  /* END BLOCK */
 
      break;

    case pSRHostList:

      {
      int index;

      MUStrCpy(SR->HostExpression,SArray[0],sizeof(SR->HostExpression));
 
      for (index = 1;SArray[index] != NULL;index++)
        {
        MUStrCat(SR->HostExpression," ",sizeof(SR->HostExpression));

        MUStrCat(SR->HostExpression,SArray[index],sizeof(SR->HostExpression));
        }  /* END for (index) */
      }    /* END BLOCK */

      break;

    case pSRIdleTime:
 
      SR->MaxIdleTime = MUTimeFromString(SVal);
 
      break;

    case pSRMaxTime:
 
      {
      long MaxTime = MUTimeFromString(SVal);
 
      MACLSet(
        SR->ACL,
        maDuration,
        &MaxTime,
        mcmpLE,
        nmPositiveAffinity,
        (1 << maclRequired),
        0);
      }  /* END BLOCK */
 
      break;

    case pSRName:

      /* no longer supported */

      break;

    case pSRPartition:
 
      MUStrCpy(SR->PName,SVal,sizeof(SR->PName));
 
      break;

    case pSRPeriod:

      SR->Period = MUGetIndex(SVal,MSRPeriodType,FALSE,SR->Period);
 
      break;

    case pSRPriority:
 
      SR->Priority = IVal;
 
      break;

    case pSRQOSList:
 
      {
      int index;
 
      for (index = 0;SArray[index] != NULL;index++);
 
      MACLLoadConfig(SR->ACL,SArray,index,maQOS);
      }  /* END BLOCK */
 
      break;

    case pSRResources:

      {
      int index;
 
      memset(&SR->DRes,0,sizeof(SR->DRes));
 
      for (index = 0;SArray[index] != NULL;index++)
        {
        MUCResFromString(&SR->DRes,SArray[index]);
        }   /* END for (index) */
 
      DBG(2,fALL) DPrint("INFO:     SRes[%s].DRes set to  P: %d  M: %d  D: %d  S: %d\n",
        SR->Name,
        SR->DRes.Procs,
        SR->DRes.Mem,
        SR->DRes.Disk,
        SR->DRes.Swap);
      }
 
      break;

    case pSRStartTime:
 
      SR->StartTime = MUTimeFromString(SVal);
 
      break;

    case pSRTaskCount:
 
      SR->TaskCount = IVal;
 
      break;

    case pSRTPN:
 
      SR->TPN = IVal;
 
      break;

    case pSRUserList:

      {
      int index;

      for (index = 0;SArray[index] != NULL;index++);
 
      MACLLoadConfig(SR->ACL,SArray,index,maUser);
      }
 
      break;

    case pSRWEndTime:
 
      SR->WEndTime = MUTimeFromString(SVal);
 
      break;

    case pSRWStartTime:
 
      SR->WStartTime = MUTimeFromString(SVal);
 
      break;

    default:

      break;
    }  /* END switch(PIndex) */

  return(SUCCESS);
  }  /* END MSRProcessOConfig() */




 
int MSRAToString(
 
  sres_t *SR,     /* I */
  int     AIndex, /* I */
  char   *Buf,    /* O */
  int     Mode)   /* I */
 
  {
  if ((SR == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }
 
  Buf[0] = '\0';
 
  switch(AIndex)
    {
    case msraChargeAccount:

      if ((Mode & (1 << mcmVerbose)) || (OSRes[SR->Index].A != NULL))
        {
        strcpy(Buf,OSRes[SR->Index].A->Name);
        }

      break;

    case msraEndTime:

      if ((Mode & (1 << mcmVerbose)) || (OSRes[SR->Index].EndTime > 0))
        {
        sprintf(Buf,"%ld",
          OSRes[SR->Index].EndTime);
        }

      break;

    case msraOwner:

      if (SR->OType != mxoNONE)
        {
        char *ptr;

        MOGetName(SR->O,SR->OType,&ptr);

        sprintf(Buf,"%s:%s",
          MXOC[SR->OType],
          ptr);
        }

      break;

    case msraStartTime:

      if ((Mode & (1 << mcmVerbose)) || (OSRes[SR->Index].StartTime > 0))
        {
        sprintf(Buf,"%ld",
          OSRes[SR->Index].StartTime);
        }

      break;

    case msraStIdleTime:

      if ((Mode & (1 << mcmVerbose)) || (SR->IdleTime > 0.0))
        {
        sprintf(Buf,"%.2lf",
          SR->IdleTime);
        }

      break;

    case msraStTotalTime:

      if ((Mode & (1 << mcmVerbose)) || (SR->TotalTime > 0.0))
        {
        sprintf(Buf,"%.2lf",
          SR->TotalTime);
        }

      break;

    case msraTaskCount:

      /* display override */

      if ((Mode & (1 << mcmVerbose)) || (OSRes[SR->Index].TaskCount > 0))
        {
        sprintf(Buf,"%d",
          OSRes[SR->Index].TaskCount);
        }

      break;

    default:

      /* not supported */
 
      return(FAILURE);
 
      /*NOTREACHED*/
 
      break;
    }  /* END switch(AIndex) */
 
  return(SUCCESS);
  }  /* END MSRAToString() */




int MSRToString(

  sres_t *SR,  /* I */
  char   *Buf) /* O */

  {
  const int CPAList[] = {
    msraName,
    msraStIdleTime,
    msraStTotalTime,
    msraTaskCount,
    msraChargeAccount,
    msraStartTime,
    msraEndTime,
    -1 };

  mxml_t *E = NULL;

  if ((SR == NULL) || (Buf == NULL))
    return(FAILURE);

  MXMLCreateE(&E,"sres");

  MSRToXML(SR,E,(int *)CPAList);

  MXMLToString(E,Buf,MAX_MBUFFER,NULL,TRUE);

  MXMLDestroyE(&E);

  return(SUCCESS);
  }  /* END MSRToString() */




int MSRToXML(

  sres_t  *SR,     /* I */
  mxml_t *E,      /* O */
  int     *SAList)

  {
  int DAList[] = {
    msraName,
    msraStIdleTime,
    msraStTotalTime,
    msraTaskCount,
    msraChargeAccount,
    msraStartTime,
    msraEndTime,
    -1 };

  int  aindex;

  int *AList;

  char tmpString[MAX_MLINE];

  if ((SR == NULL) || (E == NULL))
    return(FAILURE);

  if (SAList != NULL)
    AList = SAList;
  else
    AList = DAList;

  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    if ((MSRAToString(SR,AList[aindex],tmpString,0) == FAILURE) ||
        (tmpString[0] == '\0'))
      {
      continue;
      }

    MXMLSetAttr(E,(char *)MSResAttr[AList[aindex]],tmpString,mdfString);
    }  /* END for (aindex) */

  return(FAILURE);
  }  /* END MSRToXML() */




int MSRFromString(

  sres_t *SR,   /* O (modified) */
  char   *Buf)  /* I */

  {
  mxml_t *E = NULL;

  int rc;

  if ((Buf == NULL) || (SR == NULL))
    {
    return(FAILURE);
    }

  rc = MXMLFromString(&E,Buf,NULL,NULL);

  if (rc == SUCCESS)
    {
    rc = MSRFromXML(SR,E);
    }

  MXMLDestroyE(&E);

  if (rc == FAILURE)
    {
    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MSRFromString() */




int MSRFromXML(

  sres_t  *SR,  /* O (modified) */
  mxml_t *E)   /* I */

  {
  int aindex;
  int saindex;

  if ((SR == NULL) || (E == NULL))
    {
    return(FAILURE);
    }

  /* NOTE:  do not initialize.  may be overlaying data */

  for (aindex = 0;aindex < E->ACount;aindex++)
    {
    saindex = MUGetIndex(E->AName[aindex],MSResAttr,FALSE,0);

    if (saindex == 0)
      continue;

    MSRSetAttr(SR,saindex,(void **)E->AVal[aindex],mdfString,mSet);
    }  /* END for (aindex) */
 
  return(SUCCESS);
  }  /* END MSRFromXML() */





int MSRSetAttr(
 
  sres_t *SR,     /* I (modified) */
  int     AIndex, /* I */
  void  **Value,  /* I */
  int     Format, /* I */
  int     Mode)   /* I */
 
  {
  long tmpL;
  int  tmpI;

  if (SR == NULL)
    {
    return(FAILURE);
    }

  tmpL = 0;
  tmpI = 0;

  if (Value != NULL)
    {
    switch (Format)
      {
      case mdfLong:

        tmpL = *(long *)Value;

        break;

      case mdfInt:

        tmpI = *(int *)Value;

        break;

      default:
 
        tmpL = strtol((char *)Value,NULL,0);
        tmpL = (int)tmpL;

        break;
      }  /* END switch(Format) */
    }    /* END if (Value != NULL) */

  switch(AIndex)
    {
    case msraChargeAccount:

      MAcctAdd((char *)Value,&OSRes[SR->Index].A);

      break;

    case msraEndTime:

      OSRes[SR->Index].EndTime = tmpL;

      break;

    case msraName:

      /* NYI */

      break;

    case msraOwner:

      {
      if (Format == mdfString)
        {
        char tmpLine[MAX_MLINE];

        int oindex;

        char *TokPtr;
        char *ptr;

        void *optr;

        /* FORMAT:  <CREDTYPE>:<CREDID> */

        MUStrCpy(tmpLine,(char *)Value,sizeof(tmpLine));

        ptr = MUStrTok(tmpLine,": \t\n",&TokPtr);

        if ((ptr == NULL) ||
           ((oindex = MUGetIndex(ptr,MXOC,FALSE,mxoNONE)) == mxoNONE))
          {
          /* invalid format */

          return(FAILURE);
          }

        ptr = MUStrTok(NULL,": \t\n",&TokPtr);

        if (MOGetObject(oindex,ptr,&optr,mAdd) == FAILURE)
          {
          return(FAILURE);
          }

        SR->O     = optr;
        SR->OType = oindex;
        }
      else
        {
        /* NYI */

        return(FAILURE);
        }
      }    /* END BLOCK */

      break;
 
    case msraStartTime:

      OSRes[SR->Index].EndTime = tmpL;

      break;

    case msraStIdleTime:
 
      SR->IdleTime = tmpL;

      break;

    case msraStTotalTime:

      SR->TotalTime = tmpL;

      break;

    case msraTaskCount:

      OSRes[SR->Index].TaskCount = tmpI;

      break;

    default:

      /* not supported */

      return(FAILURE);

      /*NOTREACHED*/
 
      break;
    }  /* END switch(AIndex) */
 
  return(SUCCESS);
  }  /* END MSRSetAttr() */


/* END MSR.c */

