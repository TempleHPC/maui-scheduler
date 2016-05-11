/* HEADER */


int MClusterUpdateNodeState()

  {
  int nindex;
  int rqindex;
 
  mreq_t *RQ;
  mjob_t *J;
 
  short TotalTaskDed[MAX_MNODE];
  short TotalProcDed[MAX_MNODE];
 
  double TotalProcUtl[MAX_MNODE];
 
  mnode_t *N;

  const char *FName = "MClusterUpdateNodeState";
 
  DBG(3,fSTAT) DPrint("%s()\n",
    FName);
 
  memset(TotalTaskDed,0,sizeof(TotalTaskDed));
  memset(TotalProcDed,0,sizeof(TotalProcDed)); 
  memset(TotalProcUtl,0,sizeof(TotalProcUtl));
 
  /* add procs of all active jobs (newly updated or not) */
 
  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    if (MJOBISACTIVE(J))
      {
      for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
        {
        RQ = J->Req[rqindex];
 
        for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
          {
          N = RQ->NodeList[nindex].N;
 
          TotalTaskDed[N->Index] += RQ->NodeList[nindex].TC;
          TotalProcDed[N->Index] += RQ->NodeList[nindex].TC *
              ((RQ->DRes.Procs >= 0) ? RQ->DRes.Procs : N->CRes.Procs);
          TotalProcUtl[N->Index] += RQ->NodeList[nindex].TC *
                                      RQ->URes.Procs / 100.0;
          } /* END for (nindex)   */
        }   /* END for (rqindex)  */
      }     /* END if (J->State)  */
    }       /* END for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next) */
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];
 
    if ((N == NULL) || (N->Name[0] == '\0'))
      break;
 
    if (N->Name[0] == '\1')
      continue;
 
    /* reset job info */
 
    N->JList[0] = NULL; 
 
    MNodeAdjustAvailResources(
      N,
      TotalProcUtl[N->Index],
      TotalProcDed[N->Index],
      TotalTaskDed[N->Index]);
 
    DBG(6,fPBS) DPrint("INFO:     node '%s' C/A/D procs:  %d/%d/%d\n",
      N->Name,
      N->CRes.Procs,
      N->ARes.Procs,
      N->DRes.Procs);
    }    /* END for (nindex) */
 
  /* associate job to node */
 
  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    if (MJOBISACTIVE(J))
      {
      MJobAddToNL(J,NULL);
      }
    }  /* END for (J) */
 
  return(SUCCESS);
  } /* END MClusterUpdateNodeState() */




int MClusterClearUsage()

  {
  int      nindex;
  mnode_t *N;

  const char *FName = "MClusterClearUsage";
 
  DBG(3,fSTRUCT) DPrint("%s()\n",
    FName);

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];
 
    if ((N == NULL) || (N->Name[0] == '\0'))
      break;
 
    if (N->Name[0] == '\1')
      continue;
 
    memset(&N->DRes,0,sizeof(N->DRes));
    }  /* END for (nindex) */

  /* adjust global node */

  if (MSched.GN != NULL)
    {
    N = MSched.GN;

    /* reset global node */

    N->MTime = MSched.Time;
    N->ATime = MSched.Time;

    memcpy(&N->ARes,&N->CRes,sizeof(N->ARes));

    MNodeSetState(N,mnsIdle,0);
    }  /* END if (MNodeFind(MDEF_GNNAME,&N) == SUCCESS) */
 
  return(SUCCESS);
  }  /* END MClusterClearUsage() */




int MClusterShowARes(

  char *ResDesc,      /* I */
  int   DFlags,       /* I */
  int   DisplayMode,  /* I */
  char *Buf,          /* O */
  int   BufSize)      /* O */

  {
  char  UName[MAX_MNAME];
  char  GName[MAX_MNAME];
  char  AName[MAX_MNAME];
  char  CName[MAX_MNAME];
  char  QName[MAX_MNAME];
  char  PName[MAX_MNAME];

  char FeatureString[MAX_MLINE];

  long       MinTime;
  int        MinNodes;
  int        MinProcs;

  int        ReqMem;
  char       ReqMemCmp[MAX_MNAME];

  int        ShowSMP; /* (boolean) */

  int        Flags;   /* (BM) */

  char *BPtr;
  int   BSpace;

  const char *FName = "MClusterShowARes";

  DBG(2,fUI) DPrint("%s(%s,%d,%d,Buf,BufSize)\n",
    FName,
    (ResDesc != NULL) ? ResDesc : "NULL",
    DFlags,
    DisplayMode);

  if ((ResDesc == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  BPtr   = Buf;
  BSpace = BufSize;

  BPtr[0] = '\0';

  /* initialize response */

  if (DisplayMode == mwpXML)
    {
    /* NYI */
    }

  /* parse resource description */

  if (MUSScanF(ResDesc,"%x%s %x%s %x%s %x%s %ld %ld %ld %d %d %x%s %d %d %x%s %x%s %x%s",
       sizeof(UName),
       UName,
       sizeof(GName),
       GName,
       sizeof(AName),
       AName,
       sizeof(PName),
       PName,
       &MinTime,
       &MinNodes,
       &MinProcs,
       &ReqMem,
       &ReqMem,
       sizeof(ReqMemCmp),
       ReqMemCmp,
       &ShowSMP,
       &Flags,
       sizeof(CName),
       CName,
       sizeof(FeatureString),
       FeatureString,
       sizeof(QName),
       QName) == FAILURE)
    {
    /* invalid request string */

    DBG(3,fUI) DPrint("INFO:     cannot parse request\n");

    switch (DisplayMode)
      {
      case mwpXML:

        /* NYI */

        break;

      default:

        sprintf(BPtr,"ERROR:    cannot parse request\n");

        break;
      }  /* END switch(DisplayMode) */

    return(FAILURE);
    }

  /* NYI */

  return(SUCCESS);
  }  /* END MClusterShowARes() */



/* END MCluster.c */

