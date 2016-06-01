/*
*/
        
int UIProcessCommand(

  msocket_t *S)  /* I */
 
  { 
  char      SBuffer[MAX_SBUFFER];
  char      Message[MAX_MLINE];
  int       HeadSize;

  int       index;
  int       sindex;

  int       FLAGS;
  int       hostcheck;
 
  char      ServiceType[MAX_MLINE];
  char      Auth[MAX_MNAME];
  char      Passwd[MAX_MNAME];

  int       scode;
  int       Align;

  char     *ptr;
  char     *ptr2;

  char     *args;
  char     *TokPtr;

  int       rc;

  long      tmpL;

  char     tmpLine[MMAX_LINE];

  const char *FName = "UIProcessCommand";

  DBG(3,fUI) DPrint("%s(S)\n",
    FName);

  if (S == NULL)
    {
    return(FAILURE);
    }

  if (MSURecvData(S,MAX_SOCKETWAIT,TRUE,NULL,NULL) == FAILURE)
    {
    DBG(3,fSOCK) DPrint("ALERT:    cannot read client packet\n");

    return(FAILURE);
    }

  switch(S->WireProtocol)
    {
    case mwpXML:
    
      rc = MUISProcessRequest(S,Message);

      return(rc);

      /*NOTREACHED*/

      break;

    default:

      break;
    }  /* END switch(S->WireProtocol) */

  memset(SBuffer,0,sizeof(SBuffer));

  S->SBuffer = SBuffer;

  strcpy(CurrentHostName,S->Host);  /* NOTE:  not very threadsafe :) */

  if ((X.XUIHandler != (int (*)())0) && 
    ((*X.XUIHandler)(X.xd,S,MSched.DefaultCSKey,0) == SUCCESS))
    {
    /* service handled externally */
 
    return(SUCCESS);
    }

  if (MUISProcessRequest(S,Message) == SUCCESS)
    {
    /* new style client request received and processed */

    return(SUCCESS);
    }

  /* locate/process args */

  if ((args = strstr(S->RBuffer,MCKeyword[mckArgs])) == NULL)
    {
    DBG(3,fSOCK) DPrint("ALERT:    cannot locate command arg\n");

    sprintf(S->SBuffer,"%s%d %s%s\n",
      MCKeyword[mckStatusCode],
      scFAILURE,
      MCKeyword[mckArgs],
      "ERROR:    cannot locate command args");

    S->SBufSize = strlen(S->SBuffer);

    MSUSendData(S,MAX_SOCKETWAIT,TRUE,TRUE);

    return(FAILURE);
    }

  *args = '\0';

  args += strlen(MCKeyword[mckArgs]);

  /* get service */

  ServiceType[0] = '\0';

  if ((ptr = strstr(S->RBuffer,MCKeyword[mckCommand])) == NULL)
    {
    DBG(3,fSOCK) DPrint("ALERT:    cannot locate command\n");

    sprintf(S->SBuffer,"%s%d %s%s\n",
      MCKeyword[mckStatusCode],
      scFAILURE,
      MCKeyword[mckArgs],
      "ERROR:    cannot locate command");

    S->SBufSize = strlen(S->SBuffer);

    MSUSendData(S,MAX_SOCKETWAIT,TRUE,TRUE);

    return(FAILURE);
    }

  ptr += strlen(MCKeyword[mckCommand]);

  MUStrCpy(ServiceType,ptr,sizeof(ServiceType));

  for (ptr2 = &ServiceType[0];*ptr2 != '\0';ptr2++)
    {
    if (isspace(*ptr2))
      {
      *ptr2 = '\0';

      break;
      }
    }  /* END for (ptr2) */

  /* get authentication */

  if ((ptr = strstr(S->RBuffer,MCKeyword[mckAuth])) == NULL)
    {
    DBG(3,fSOCK) DPrint("ALERT:    cannot locate authentication\n");

    sprintf(S->SBuffer,"%s%d %s%s\n",
      MCKeyword[mckStatusCode],
      scFAILURE,
      MCKeyword[mckArgs],
      "ERROR:    cannot locate authentication");

    S->SBufSize = (long)strlen(S->SBuffer);

    MSUSendData(S,MAX_SOCKETWAIT,TRUE,TRUE);

    return(FAILURE);
    }

  ptr += strlen(MCKeyword[mckAuth]);

  MUStrCpy(Auth,ptr,sizeof(Auth)); 

  /* FORMAT:  <USERNAME>[:<PASSWORD>] */

  for (ptr2 = &Auth[0];*ptr2 != '\0';ptr2++)
    {
    if (isspace(*ptr2))
      {
      *ptr2 = '\0';

      break;
      }
    }

  if ((ptr2 = MUStrTok(Auth,":",&TokPtr)) != NULL)
    {
    MUStrCpy(Passwd,ptr2,sizeof(Passwd));
    }
  else
    {
    Passwd[0] = '\0';
    }

  /* determine service */         

  if ((sindex = MUGetIndex(ServiceType,MService,0,0)) == 0)
    {
    DBG(3,fUI) DPrint("INFO:     service '%s' not handled in %s\n",
      ServiceType,
      FName);

    sprintf(Message,"ERROR:    cannot support service '%s'",
      ServiceType);

    sprintf(S->SBuffer,"%s%d %s%s\n",
      MCKeyword[mckStatusCode],
      scFAILURE,
      MCKeyword[mckArgs],
      Message);

    S->SBufSize = (long)strlen(S->SBuffer);

    MSUSendData(S,MAX_SOCKETWAIT,TRUE,TRUE);

    return(FAILURE);
    }

  DBG(3,fUI) DPrint("INFO:     client '%s' read (%ld bytes) initiating service call for '%s' (Auth: %s)\n",
    S->Name,
    S->SBufSize,
    MService[sindex],
    Auth);

  /* fail if name is not recognized */

  if (Auth[0] == '\0')
    {
    DBG(2,fUI) DPrint("WARNING:  client id '%s' is unknown\n",
      Auth);

    sprintf(S->SBuffer,"%s%d %s%s\n",
      MCKeyword[mckStatusCode],
      scFAILURE,
      MCKeyword[mckArgs],
      "ERROR:    cannot authenticate client");

    S->SBufSize = (long)strlen(S->SBuffer);

    MSUSendData(S,MAX_SOCKETWAIT,TRUE,TRUE);

    return(FAILURE);
    }

  ServerGetAuth(Auth,&tmpL);

  FLAGS = (int)tmpL;

  switch(sindex)
    {
    /* admin1 functions */

    case svcResetStats:
    case svcSched:
    case svcSetJobDeadline:
    case svcReleaseJobDeadline:
    case svcChangeParameter:
    case svcMigrateJob:
    case svcBNFQuery:
    case svcMGridCtl:
    case svcMJobCtl:
    case svcMNodeCtl:

      if (!(FLAGS & (1 << fAdmin1)))
        {
        sprintf(Message,"ERROR:    user '%s' is not authorized to run command '%s'\n",
          Auth,
          MService[sindex]);

        sprintf(S->SBuffer,"%s%d %s%s\n",
          MCKeyword[mckStatusCode],
          scFAILURE,
          MCKeyword[mckArgs],
          Message);

        S->SBufSize = (long)strlen(SBuffer);

        MSUSendData(S,MAX_SOCKETWAIT,TRUE,TRUE);

        return(FAILURE);

        /*NOTREACHED*/

        break;
        }

      hostcheck = FALSE;

      for (index = 0;index < MAX_MADMINHOSTS;index++)
        {
        if (MSched.AdminHost[index][0] == '\0')
          break;

        if (!strcasecmp(MSched.AdminHost[index],S->Host))
          {
          hostcheck = TRUE;

          break;
          }

        if (!strcasecmp(MSched.AdminHost[index],"ALL"))
          {
          hostcheck = TRUE;

          break;
          }
        }    /* END for (index) */

      if (hostcheck == FALSE)
        {
        sprintf(Message,"ERROR:    command '%s' cannot be executed from host '%s'\n",
          MService[sindex],
          S->Host);

        sprintf(S->SBuffer,"%s%d %s%s\n",
          MCKeyword[mckStatusCode],
          scFAILURE,
          MCKeyword[mckArgs],
          Message);

        S->SBufSize = (long)strlen(SBuffer);

        MSUSendData(S,MAX_SOCKETWAIT,TRUE,TRUE);

        return(FAILURE);

        /*NOTREACHED*/

        break;
        }

    /* admin1 or admin2 function */

    case svcSetJobSystemPrio:
    case svcRunJob:

      if (!(FLAGS & ((1 << fAdmin1) | (1 << fAdmin2))))
        {
        sprintf(Message,"ERROR:    user '%s' is not authorized to execute command '%s'\n",
          Auth,
          MService[sindex]);

        sprintf(S->SBuffer,"%s%d %s%s\n",
          MCKeyword[mckStatusCode],
          scFAILURE,
          MCKeyword[mckArgs],
          Message);

        S->SBufSize = (long)strlen(SBuffer);

        MSUSendData(S,MAX_SOCKETWAIT,TRUE,TRUE);

        return(FAILURE);

        /*NOTREACHED*/

        break;
        }

    /* admin1, admin2, or admin3 functions */

    case svcShowStats:
    case svcDiagnose:
    case svcShowJobDeadline:
    case svcShowConfig:
    case svcNodeShow:
    case svcShowEstimatedStartTime:
    case svcShowGrid:
    case svcClusterShow:

      if (!(FLAGS & ((1 << fAdmin1) | (1 << fAdmin2) | (1 << fAdmin3))))
        {
        sprintf(Message,"ERROR:    user '%s' is not authorized to execute command '%s'\n",
          Auth,
          MService[sindex]);

        sprintf(S->SBuffer,"%s%d %s%s\n",
          MCKeyword[mckStatusCode],
          scFAILURE,
          MCKeyword[mckArgs],
          Message);

        S->SBufSize = (long)strlen(SBuffer);

        MSUSendData(S,MAX_SOCKETWAIT,TRUE,TRUE);

        return(FAILURE);

        /*NOTREACHED*/

        break;
        }

    /* global functions or case specific functions */

    case svcResCreate:  
    case svcResShow:
    case svcSetJobQOS:
    case svcCancelJob:
    case svcSetJobUserPrio:
    case svcJobShow:
    case svcShowQ:
    case svcShowTasks:
    case svcSetJobHold:
    case svcReleaseJobHold:
    case svcResDestroy:
    case svcShowJobHold:
    case svcShowEarliestDeadline:
    case svcShowBackfillWindow:
    case svcMBal:

      if (sindex == svcMGridCtl)
        strcpy(Auth,S->Name);

      S->SBufSize = (long)sizeof(SBuffer);

      sprintf(tmpLine,"%s%d ",
        MCKeyword[mckStatusCode],
        scFAILURE);
      Align = (int)strlen(tmpLine) + (int)strlen(MCKeyword[mckArgs]);

      sprintf(S->SBuffer,"%s%*s%s",
        tmpLine,
	      16 - (Align % 16), 
	      " ",
        MCKeyword[mckArgs]);

      HeadSize = (int)strlen(SBuffer);
      S->SBufSize -= HeadSize + 1;
      if(sindex == svcShowTasks)
        scode = UIShowUserTasks(args,S->SBuffer + HeadSize,&S->SBufSize);
      else if (Function[sindex] != NULL)
        scode = (*Function[sindex])(args,S->SBuffer + HeadSize,FLAGS,Auth,&S->SBufSize);
      else
        scode = FAILURE;
      ptr = S->SBuffer + strlen(MCKeyword[mckStatusCode]);

      *ptr = scode + '0';

      S->SBufSize = (long)strlen(S->SBuffer);

      MSUSendData(S,MAX_SOCKETWAIT,TRUE,TRUE);


      break;

    default:

      DBG(2,fUI) DPrint("WARNING:  unexpected service (%d) requested  (ignoring request)\n",
        sindex);

      sprintf(Message,"ERROR:    service '%s' (%d) not implemented\n",
        ServiceType,
        sindex);

      sprintf(S->SBuffer,"%s%d %s%s\n",
        MCKeyword[mckStatusCode],
        scFAILURE,
        MCKeyword[mckArgs],
        Message);

      S->SBufSize = (long)strlen(S->SBuffer);

      MSUSendData(S,MAX_SOCKETWAIT,TRUE,TRUE);

      break;
    }  /* END switch(sindex) */

  fflush(mlog.logfp);

  return(SUCCESS);
  }  /* END UIProcessCommand() */




int ShowJobHold(

  char *RBuffer,  /* I */
  char *Buffer,   /* O */
  int   FLAGS,    /* I */
  char *Auth,     /* I */
  long *BufSize)  /* I */

  {
  char Line[MAX_MLINE];

  mjob_t *J;

  const char *FName = "ShowJobHold";

  DBG(2,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);

  sprintf(Buffer,"%ld\n",
    MSched.Time);

  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    if (J->Hold != 0)
      {
      DBG(2,fUI) DPrint("INFO:     job '%s' added to hold list\n",
        J->Name);

      sprintf(Line,"%s %d\n",
        J->Name,
        J->Hold);

      strcat(Buffer,Line);
      }
    }  /* END for (index) */

  DBG(2,fUI) DPrint("INFO:     ShowJobHold() buffer size: %d bytes\n",
    (int)strlen(Buffer));

  return(SUCCESS);
  }  /* END ShowJobHold() */




int SetJobUserPrio(

  char *RBuffer,
  char *Buffer,
  int   FLAGS,
  char *Auth,
  long *BufSize)

  {
  char Name[MAX_MNAME];
  int  Priority;

  mjob_t *J;

  const char *FName = "SetJobUserPrio";

  DBG(2,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);

  MUSScanF(RBuffer,"%x%s %d",
    sizeof(Name),
    Name,
    &Priority);

  if ((Priority < 0) || (Priority > 100))
    {
    strcpy(Buffer,"ERROR:  user priority must be in the range 0 - 100\n");

    return(FAILURE);
    }

  if (MJobFind(Name,&J,0) == FAILURE)
    {
    sprintf(Buffer,"Error:  cannot locate job '%s'\n",
      Name);

    return(FAILURE);
    }

  /* security check */

  if (!(FLAGS & ((1 << fAdmin1) | (1 << fAdmin2))))
    {
    if (strcasecmp(J->Cred.U->Name,Auth) != 0)
      {
      DBG(2,fUI) DPrint("INFO:     user %s is not authorized to set user priority on job %s\n",
        Auth,
        J->Name);

      sprintf(Buffer,"user %s is not authorized to set user priority on job %s\n",
        Auth,
        J->Name);

      return(FAILURE);
      }
    }

  J->UPriority = Priority;

  DBG(2,fUI) DPrint("INFO:     user priority on job '%s' set to %d\n",
    Name,
    Priority);

  return(SUCCESS);
  }  /* END JobSetUserPrio() */
     



int UIShowGrid(

  char *RBuffer,  /* I */
  char *Buffer,   /* O */
  int   FLAGS,    /* I */
  char *Auth,     /* I */
  long *BufSize)  /* I */

  {
  int  sindex;
  char GStat[MAX_MNAME];

  const char *FName = "UIShowGrid";

  DBG(3,fUI) DPrint("%s(RBuffer,Buffer,%d,%s,BufSize)\n",
    FName,
    FLAGS,
    Auth);

  MUSScanF(RBuffer,"%x%s",
    sizeof(GStat),
    GStat);

  /* determine statistics type requested */

  for (sindex = 0;MStatType[sindex] != 0;sindex++)
    {
    if (!strcasecmp(MStatType[sindex],GStat))
      break;
    }  /* END for (sindex) */

  if (MStatType[sindex] == NULL)
    {
    DBG(2,fUI) DPrint("INFO:     invalid stat type '%s' requested\n",
      GStat);

    sprintf(Buffer,"ERROR:  invalid statistics type requested\n");

    strcat(Buffer,"\nvalid statistics types:\n");

    for (sindex = 0;MStatType[sindex] != 0;sindex++)
      {
      strcat(Buffer,MStatType[sindex]);
      strcat(Buffer,"\n");
      }  /* END for (sindex) */

    return(FAILURE);
    }  /* END if (MStatType[sindex] == NULL) */

  MStatBuildGrid(sindex,Buffer,0);

  return(SUCCESS);
  }  /* END UIShowGrid() */

 


int ShowBackfillWindow(

  char *RBuffer,    /* I */
  char *Buffer,     /* O */
  int   FLAGS,      /* I */
  char *Auth,       /* I */
  long *BufSize)    /* I */

  {
  int  BFNodeCount;
  int  BFProcCount;
  nodelist_t BFNodeList;
  long BFTime;
  long MinTime;

  long RequiredTime;
  long RequiredNodes;
  long RequiredProcs;

  char UserName[MAX_MNAME];
  char GroupName[MAX_MNAME];
  char AccountName[MAX_MNAME];

  int  Memory;
  char MemCmp[MAX_MNAME];
  int  DMemory;

  char PName[MAX_MNAME];
  char CurrentPName[MAX_MNAME];

  int  index;
  int  pindex;
  int  mindex;
  int  nindex;

  int  ShowSMP;  /* (boolean) */

  char Affinity;
  int  Type;

  int  Flags;

  char QOSName[MAX_MNAME];

  char ClassString[MAX_MLINE];
  char FeatureString[MAX_MLINE];

  mnode_t     *N;

  mjob_t       tmpJ;
  mjob_t      *J;
  mreq_t       tmpRQ;

  mcres_t      DRes;
  mpar_t      *P;
  mpar_t      *SP;

  mrange_t     ARange[MAX_MRANGE];
  mrange_t     RRange[2];

  int          NodeHeaderPrinted;

  int          PCount;

  char         tmpBuffer[MAX_MBUFFER];

  const char *FName = "ShowBackfillWindow";

  DBG(2,fUI) DPrint("%s(%s,Buffer,%d,%s,BufSize)\n",
    FName,
    RBuffer,
    FLAGS,
    Auth);

  if (MUSScanF(RBuffer,"%x%s %x%s %x%s %x%s %ld %ld %ld %d %d %x%s %d %d %x%s %x%s %x%s",
       sizeof(UserName),
       UserName,
       sizeof(GroupName),
       GroupName,
       sizeof(AccountName),
       AccountName,
       sizeof(PName),
       PName,
       &RequiredTime,
       &RequiredNodes,
       &RequiredProcs,
       &DMemory,
       &Memory,
       sizeof(MemCmp),
       MemCmp,
       &ShowSMP,
       &Flags,
       sizeof(ClassString),
       ClassString,
       sizeof(FeatureString),
       FeatureString,
       sizeof(QOSName),
       QOSName) == FAILURE)
    {
    /* invalid request string */

    DBG(3,fUI) DPrint("INFO:     cannot parse request\n");
 
    sprintf(Buffer,"ERROR:    cannot parse request\n");
 
    return(FAILURE);
    }

  MParFind(PName,&SP);

  DBG(4,fUI) DPrint("INFO:     locating backfill window for u: %s  g: %s  a: %s  p: %s  c: %s  f: %s (%ld:%ld:%d)\n",
    UserName,
    GroupName,
    AccountName,
    MAList[ePartition][(SP != NULL) ? SP->Index : 0],
    ClassString,
    FeatureString,
    RequiredTime,
    RequiredNodes,
    Memory);

  if (ShowSMP == FALSE)  /* NON-SMP */
    {
    mindex = 0;

    if (Memory > 0)
      {
      for (mindex = 0;MComp[mindex] != NULL;mindex++)
        {
        if (!strcasecmp(MComp[mindex],MemCmp))
          break;
        }

      if (MComp[mindex] == NULL)
        mindex = 0;
      }

    memset(&DRes,0,sizeof(DRes));
    DRes.Procs = 1;
    DRes.Mem   = DMemory;
 
    sprintf(Buffer,"backfill window (user: '%s' group: '%s' partition: %s) %s\n",
      UserName,
      GroupName,
      PName,
      MULToDString((mulong *)&MSched.Time));

    PCount = 0;

    for (pindex = 1;pindex < MAX_MPAR;pindex++)
      {
      if (MPar[pindex].ConfigNodes == 0)
        continue;

      PCount++;
      }

    index = 0;

    for (pindex = 0;pindex < MAX_MPAR;pindex++)
      {
      P = &MPar[pindex];

      if ((SP != NULL) && (SP != P))
        continue;

      if ((PCount == 1) && (SP == NULL) && (P->Index != 0))
        break;

      if (P->ConfigNodes == 0)
        continue;

      MinTime = 0;

      DBG(7,fUI) DPrint("INFO:     checking window in partition %s\n",
        P->Name);

      strcpy(CurrentPName,GLOBAL_MPARNAME);

      index = 0;
 
      while (MBFGetWindow(
               &BFNodeCount,
               &BFProcCount,
               BFNodeList,
               &BFTime,
               MinTime,
               P,
               UserName,
               GroupName,
               AccountName,
               mindex,
               Memory,
               MAX(1,RequiredTime),
               &DRes,
               ClassString,
               FeatureString,
               QOSName,
               (Flags & (1 << mcmVerbose)) ? 
                 tmpBuffer : 
                 NULL) == SUCCESS)
        {
        DBG(4,fUI) DPrint("INFO:     located backfill window [%03d nodes : %06ld seconds]\n",
          BFNodeCount,
          BFTime);

        if ((BFTime >= RequiredTime) && 
            (BFNodeCount >= RequiredNodes) && 
            (BFProcCount >= RequiredProcs))
          {
          if (strcasecmp(P->Name,CurrentPName) != 0)
            {
            if ((strcasecmp(CurrentPName,GLOBAL_MPARNAME)) && (index == 0))
              {
              sprintf(temp_str,"no %s available\n",
                (MSched.DisplayFlags & (1 << dfNodeCentric)) ? "nodes" : "procs");
              }
            strcat(Buffer,temp_str);

            strcpy(CurrentPName,P->Name);

            sprintf(temp_str,"\npartition %s:\n",
              P->Name);
            strcat(Buffer,temp_str);

            index = 0;
            }

          if (MSched.DisplayFlags & (1 << dfNodeCentric))
            {
            /* node centric output */

            if (BFNodeCount == 0)
              {
              sprintf(temp_str,"no nodes available\n");
              strcat(Buffer,temp_str);
              }
            else if (BFTime < 100000000)
              {
              sprintf(temp_str,"%3d node%s available for   %11s\n",
                BFNodeCount,
                (BFNodeCount > 1) ? "s" : "",
                MULToTString(BFTime));
              strcat(Buffer,temp_str);
              }
            else
              {
              sprintf(temp_str,"%3d node%s available with no timelimit\n",
                BFNodeCount,
                (BFNodeCount > 1) ? "s" : "");
              strcat(Buffer,temp_str);
              }
            }
          else
            {
            /* proc centric output */

            if (BFProcCount == 0)
              {
              sprintf(temp_str,"no procs available\n");
              strcat(Buffer,temp_str);
              }
            else if (BFTime < 100000000)
              {
              sprintf(temp_str,"%3d proc%s available for   %11s\n",
                BFProcCount,
                (BFProcCount > 1) ? "s" : "",
                MULToTString(BFTime));
              strcat(Buffer,temp_str);
              }
            else
              {
              sprintf(temp_str,"%3d proc%s available with no timelimit\n",
                BFProcCount,
                (BFProcCount > 1) ? "s" : "");
              strcat(Buffer,temp_str);
              }
            }

          index++;
          }   /* END if (BFTime) */

        MinTime = BFTime;
        }     /* END while (MBFGetWindow() == SUCCESS) */

      if (Flags & (1 << mcmVerbose))
        {
        strcat(Buffer,"\n\n");
        strcat(Buffer,tmpBuffer);
        }
      }    /* END for (pindex) */

    if (index == 0)
      {
      sprintf(temp_str,"no procs available\n");
      strcat(Buffer,temp_str);
      }

    strcat(Buffer,"\n\n");
    }    /* END if (ShowSMP == FALSE) */
  else  
    {
    /* SMP Mode */

    RRange[0].StartTime = MSched.Time;
    RRange[0].EndTime   = MAX_MTIME;
    RRange[1].EndTime   = 0;

    memset(&tmpJ,0,sizeof(tmpJ));
    memset(&tmpRQ,0,sizeof(tmpRQ));

    J = &tmpJ;

    tmpRQ.DRes.Procs = 1;
    tmpRQ.DRes.Mem   = DMemory;

    J->Req[0] = &tmpRQ;

    if (MJobSetCreds(J,ALL,ALL,ALL) == FAILURE)
      {
      DBG(3,fUI) DPrint("INFO:     cannot setup showbf job creds\n");

      sprintf(Buffer,"ERROR:    cannot determine available resources\n");

      return(FAILURE);
      }

    if (MQOSFind(QOSName,&J->QReq) == FAILURE)
      {
      /* cannot locate requested QOS */

      MQOSFind(DEFAULT,&J->QReq);
      }

    MJobSetQOS(J,J->QReq,0);

    MJobBuildCL(J);

    sprintf(temp_str,"%20s %5s %6s %7s %7s   %14s\n",
      "HostName",
      "Procs",
      "Memory",
      "Disk",
      "Swap",
      "Time Available");
    strcat(Buffer,temp_str);

    sprintf(temp_str,"%20s %5s %6s %7s %7s   %14s\n",
      "----------",
      "-----",
      "------",
      "-------",
      "-------",
      "--------------");
    strcat(Buffer,temp_str);

    for (nindex = 0;nindex < MAX_MNODE;nindex++)
      {
      N = MNode[nindex];

      if ((N == NULL) || (N->Name[0] == '\0'))
        break;

      if (N->Name[0] == '\1')
         continue;

      if ((N->State != mnsActive) && (N->State != mnsIdle))
        continue;

      if ((SP != NULL) && (SP->Index != 0) && (N->PtIndex != SP->Index))
        continue;

      NodeHeaderPrinted = FALSE;

      for (RRange[0].TaskCount = 1;RRange[0].TaskCount < N->CRes.Procs;RRange[0].TaskCount = ARange[0].TaskCount + 1)
        {
        if (MJobGetSNRange(
              J,
              J->Req[0],
              N,
              RRange,
              1,
              &Affinity,
              &Type,
              ARange,
              &DRes,
              NULL) == SUCCESS)
          {
          if (ARange[0].StartTime != MSched.Time)
            {
            /* resources not available immediately */

            break;
            }

          if (NodeHeaderPrinted == FALSE)
            {
            strcat(Buffer,"\n");

            NodeHeaderPrinted = TRUE;
            }

          sprintf(temp_str,"%20s %5d %6d %7d %7d   %14s\n",
            N->Name,
            DRes.Procs,
            DRes.Mem,
            DRes.Disk,
            DRes.Swap,
            MULToTString(ARange[0].EndTime - MSched.Time));
          strcat(Buffer,temp_str);
          }
        else
          {
          /* resources not available */

          break;
          }
        }
      }   /* END for (nindex)            */
    }     /* END else (ShowSMP == FALSE) */

  return(SUCCESS);
  }  /* END ShowBackfillWindow() */




int UINodeStatShow(

  int   Flags,
  char *Buffer,
  long *BufSize)

  {
  int           nindex;
  int           JobLimit;
  int           cindex;
  unsigned long totalworkload;
  int           totalnodes;
  int           averageload;
  double        Average;
  double        IAverage;

  int           mem;
  unsigned long MemTotalUpTime;
  unsigned long MemTotalTotalTime;
  unsigned long MemTotalBusyTime;
  int           MemTotalNodeCount;
  int           MemTotalBusyCount;
  int           MemTotalUpCount;

  unsigned long SystemTotalUpTime;
  unsigned long SystemTotalTotalTime;
  unsigned long SystemTotalBusyTime;
  int           SystemTotalNodeCount;
  int           SystemTotalBusyCount;
  int           SystemTotalUpCount;

  int           mode;
 
  mnode_t      *N;

  const char *FName = "UINodeStatShow";

  DBG(2,fUI) DPrint("%s(%d,Buffer,BufSize)\n",
    FName,
    Flags);

  mode = Flags & 0x1;
  JobLimit = Flags >> 1;
  
  MStatBuildRClass(JobLimit,MRClass);

  sprintf(Buffer,"Memory Requirement Breakdown:\n\n");

  sprintf(temp_str,"%8s %5s %7s %9s %7s %10s %7s\n",
    "Memory",
    "Proc",
    "Percent",
    "InitialNH",
    "Percent",
    "ProcHours",
    "Percent");
  strcat(Buffer,temp_str);

  totalnodes = 0;
  totalworkload = 0;

  /* determine total node count/class workload */
 
  for (cindex = 0;cindex < MAX_MRCLASS;cindex++)
    {
    totalnodes    += MRClass[cindex].Nodes;

    totalworkload += MRClass[cindex].Workload;
    }  /* END for (cindex) */

  if (totalnodes == 0)
    {
    sprintf(Buffer,"No Nodes Detected\n");

    return(FAILURE);
    }

  averageload = totalworkload / totalnodes;

  /* loop thru all memory classes */

  for (cindex = 0;cindex < MAX_MRCLASS;cindex++)
    {
    if (MRClass[cindex].Memory == 0)
      break;            

    if ((MRClass[cindex].Nodes == 0) || (averageload == 0)) 
      {
      Average  = 0.0;
      IAverage = 0.0;
      }
    else
      {
      Average = (double)MRClass[cindex].Workload / MRClass[cindex].Nodes / averageload * 100.0;
      IAverage = (double)MRClass[cindex].InitialWorkload / MRClass[cindex].Nodes / averageload * 100.0;
      }

    sprintf(temp_str,"%8d %5d %7.2f %9ld %7.2f %10ld %7.2f\n",
      MRClass[cindex].Memory,
      MRClass[cindex].Nodes,
      (double)MRClass[cindex].Nodes / totalnodes * 100.0,
      MRClass[cindex].InitialWorkload / 3600,
      IAverage,
      MRClass[cindex].Workload / 3600,
      Average);
    strcat(Buffer,temp_str);
    }  /* END for (cindex) */

  sprintf(temp_str,"%8s %5d %7.2f %9d %7.2f %10d %7.2f\n\n",
    "TOTAL",
    totalnodes,
    100.0,
    (int)(totalworkload / 3600),
    100.0,
    (int)(totalworkload / 3600),
    100.0);
  strcat(Buffer,temp_str);

  strcat(Buffer,"\nNode Statistics\n\n");

  /* dump node statistics */

  /* loop through all memory sizes */

  SystemTotalUpTime    = 0;
  SystemTotalTotalTime = 0;
  SystemTotalBusyTime  = 0;
  SystemTotalNodeCount = 0;
  SystemTotalUpCount   = 0;
  SystemTotalBusyCount = 0;

  for (cindex = 0;cindex < MAX_MRCLASS;cindex++)
    {
    if (MRClass[cindex].Memory == 0)
      break;
 
    mem = MRClass[cindex].Memory;           

    MemTotalUpTime    = 0;
    MemTotalTotalTime = 0;
    MemTotalBusyTime  = 0;
    MemTotalNodeCount = 0;
    MemTotalUpCount   = 0;
    MemTotalBusyCount = 0;

    for (nindex = 0;nindex < MAX_MNODE;nindex++)
      {
      N = MNode[nindex];

      if ((N == NULL) || (N->Name[0] == '\0'))
        break;

      if (N->Name[0] == '\1')
        continue;

      if (N->CRes.Mem == mem)
        {
        if (MemTotalNodeCount == 0)
          {
          /* create memory class header */

          if (mode == 0)
            {
            sprintf(temp_str,"%4dMB Nodes\n",
              mem);
            strcat(Buffer,temp_str);

            sprintf(temp_str,"%20s %9s %9s %9s\n",
              "NodeName",
              "Available",
              "Busy",
              "NodeState");
            strcat(Buffer,temp_str);
            }
          }

        /* record mem stats in seconds */

        MemTotalUpTime    += N->SUTime;
        MemTotalTotalTime += N->STTime;
        MemTotalBusyTime  += N->SATime;
        MemTotalNodeCount++;

        if ((N->State == mnsIdle) || (N->State == mnsActive) || (N->State == mnsBusy))
          MemTotalUpCount++;

        if (N->State == mnsBusy)
          MemTotalBusyCount++;

        if (mode == 0)
          {
          if (N->SUTime != 0)
            {
            sprintf(temp_str,"%20s %8.2f%s %8.2f%s %9s\n",
              N->Name,
              (double)N->SUTime / N->STTime * 100.0,
              "%",
              (double)N->SATime / N->SUTime * 100.0,
              "%",
              MNodeState[N->State]);
            strcat(Buffer,temp_str);
            }
          else
            {
            sprintf(temp_str,"%20s %9.2f %9.2f %9s\n",
              N->Name,
              0.0,
              0.0,
              MNodeState[N->State]);
            strcat(Buffer,temp_str);
            }
          }    /* END if (mode == 0) */
        }      /* END if (N->CRes.Mem == mem) */
      }        /* END for (nindex) */

    SystemTotalUpTime    += MemTotalUpTime;
    SystemTotalTotalTime += MemTotalTotalTime;
    SystemTotalBusyTime  += MemTotalBusyTime;
    SystemTotalNodeCount += MemTotalNodeCount;
    SystemTotalUpCount   += MemTotalUpCount;
    SystemTotalBusyCount += MemTotalBusyCount;

    /* create memory summary */

    if (MemTotalNodeCount != 0)
      {
      sprintf(temp_str,"Summary:  %3d %4dMB Nodes  %6.2f%s Avail  %6.2f%s Busy  (Current: %6.2f%s Avail  %6.2f%s Busy)\n",
        MemTotalNodeCount,
        mem,
        (double)MemTotalUpTime / MemTotalTotalTime * 100.0,
        "%",
        (double)MemTotalBusyTime / MemTotalTotalTime * 100.0,
        "%",
        (double)MemTotalUpCount / MemTotalNodeCount * 100.0,
        "%",
        (double)MemTotalBusyCount / MemTotalNodeCount * 100.0,
        "%");
      strcat(Buffer,temp_str);
 
      if (mode == 0)
        strcat(Buffer,"\n");
      }
    }    /* for (cindex) */

  /* create system summary */

  if (SystemTotalTotalTime != 0)
    {
    sprintf(temp_str,"System Summary:  %3d Nodes  %6.2f%s Avail  %6.2f%s Busy  (Current: %6.2f%s Avail  %6.2f%s Busy)\n",
      SystemTotalNodeCount,
      (double)SystemTotalUpTime / SystemTotalTotalTime * 100.0,
      "%",
      (double)SystemTotalBusyTime / SystemTotalTotalTime * 100.0,
      "%",
      (double)SystemTotalUpCount / SystemTotalNodeCount * 100.0,
      "%",
      (double)SystemTotalBusyCount / SystemTotalNodeCount * 100.0,
      "%");
    strcat(Buffer,temp_str);
    }

  strcat(Buffer,"\n");
 
  return(SUCCESS);
  }  /* END UINodeStatShow() */

/* END OUserI.c */
