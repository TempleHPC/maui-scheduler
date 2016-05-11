/* HEADER */
        
/* Contains:                                 *
 *                                           */


#include "moab.h"
#include "msched-proto.h"  

extern mlog_t       mlog;

extern mnode_t     *MNode[];
extern mjob_t      *MJob[];
extern mqos_t       MQOS[];
extern msched_t     MSched;
extern mframe_t     MFrame[];
extern mpar_t       MPar[];
extern mgcred_t    *MUser[];
extern mgcred_t     MGroup[];
extern mgcred_t     MAcct[];
extern msys_t       MSys;  
extern mckpt_t      MCP;
extern mrm_t        MRM[];
extern mclass_t     MClass[];
extern mstat_t      MStat;
extern mattrlist_t  MAList;
extern int          MAQ[];

extern const char *MResourceType[];
extern const char *MNodeState[];
extern const char *MNodeAttr[];
extern const char *MNPComp[];
extern const char *MWikiNodeAttr[];
extern const char *MRMType[];
extern const char *MResType[];
extern const char *MNResAttr[];
extern const char *MXO[];
extern const char *MResAttr[];

#define MPARM_NODECFG "NODECFG"           

#include "MCluster.c"


int __MNodeGetPEPerTask(mcres_t *,mcres_t *);

char *MNodePrioFToString(mnode_t *,char *);

typedef struct
  {
  mnode_t *N;
  int      TC;
  long     ATime;
  } _mpnodealloc_t;

int __MNLATimeCompLToH(

  _mpnodealloc_t *a,
  _mpnodealloc_t *b)

  {
  /* order low to high */

  return(a->ATime - b->ATime);
  }  /* END __MNLATimeCompLToH() */





int MNodeCreate(

  mnode_t **NP)  /* I */

  {
  mnode_t *N;

  void *nr;
  void *nre;
  void *nrc;

  if (NP == NULL)
    {
    return(FAILURE);
    }

  if (*NP == NULL)
    {
    *NP = (mnode_t *)calloc(1, sizeof(mnode_t));

    N = *NP;      

    nr  = NULL;
    nre = NULL;
    nrc = NULL;    
    }
  else
    {
    N = *NP;

    nr  = (void *)N->R;
    nre = (void *)N->RE;
    nrc = (void *)N->RC;
    }

  MNodeInitialize(N,NULL);

  N->R = (mres_t **)((nr != NULL) ? 
    nr : 
    calloc(1,sizeof(mres_t *) * (MSched.ResDepth + 1)));

  N->RE = (mre_t *)((nre != NULL) ?
    nre :
    calloc(1,sizeof(mre_t) * ((MSched.ResDepth << 1) + 1)));

  N->RC = (short *)((nrc != NULL) ?
    nrc :
    calloc(1,sizeof(short) * (MSched.ResDepth + 1)));

  N->MaxJCount = MAX_MJOB_PER_NODE;

  N->JList = (void **)calloc(1,sizeof(void *) * (N->MaxJCount + 1));
  N->JTC   = (int *)calloc(1,sizeof(int) * (N->MaxJCount + 1));

  return(SUCCESS);
  }  /* END MNodeCreate() */





int MNodeAdd(
 
  char     *NName, /* I: node name    */
  mnode_t **NP)    /* O: node pointer */
 
  {
  int nindex;

  mnode_t *N;

  const char *FName = "MNodeAdd";
 
  DBG(5,fSTRUCT) DPrint("%s(%s,N)\n",
    FName,
    (NName != NULL) ? NName : NULL);

  if (NP != NULL)
    *NP = NULL;

  if ((NName == NULL) || (NName[0] == '\0'))
    {
    return(FAILURE);
    }

  if (MNodeFind(NName,&N) == SUCCESS)
    {
    /* node already exists */

    if (NP != NULL)
      *NP = N;
   
    return(SUCCESS);
    }
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    /* if available node slot is found */

    N = MNode[nindex];        
 
    if ((N == NULL) ||
        (N->Name[0] == '\0') || 
        (N->Name[0] == '\1'))
      {
      /* empty node slot located */

      break;
      }
    }    /* END for (nindex) */

  if (nindex >= MAX_MNODE)
    {
    /* no space found in node table */
 
    DBG(1,fSTRUCT) DPrint("WARNING:  node buffer overflow in %s()\n",
      FName);
 
    DBG(1,fSTRUCT) DPrint("INFO:     node table is full (cannot add node '%s')\n",
      NName);

    return(FAILURE);
    }

  /* available node slot found */

  MNodeCreate(&N);

  MNode[nindex] = N;

  /* initialize node */

  N->Index = nindex;
 
  MUStrCpy(N->Name,NName,sizeof(N->Name));
 
  if (NP != NULL)
    *NP = N;
 
  return(SUCCESS);
  }  /* END MNodeAdd() */




int MNodeLoadConfig(

  mnode_t *N,    /* I */
  char    *Buf)  /* I (optional) */

  {
  char *ptr;
  char *head;

  char  Value[MAX_MLINE];

  char  tmpSName[MAX_MNAME];
  char  tmpLName[MAX_MNAME];

  const char *FName = "MNodeLoadConfig";

  DBG(5,fSTRUCT) DPrint("%s(%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    (Buf != NULL) ? Buf : "NULL");

  /* load specified node config info */

  if ((N == NULL) || (N->Name[0] == '\0'))
    {
    return(FAILURE);
    }
 
  if (MSched.ConfigBuffer == NULL)
    {
    return(FAILURE);
    }

  /* search for parameters specified with both long and short names */

  /* search short names */

  strcpy(tmpSName,MNodeAdjustName(N->Name,0));

  if (Buf != NULL)
    head = Buf;
  else
    head = MSched.ConfigBuffer;

  ptr = head;
 
  while (MCfgGetSVal(
           head,
           &ptr,
           MPARM_NODECFG,
           tmpSName,
           NULL,
           Value,
           sizeof(Value),
           0,
           NULL) != FAILURE)
    {
    /* node attribute located */

    /* FORMAT:  <ATTR>=<VALUE>[ <ATTR>=<VALUE>]... */

    MNodeProcessConfig(N,Value); 
    }

  strcpy(tmpLName,MNodeAdjustName(N->Name,1));

  if (strcmp(tmpSName,tmpLName))
    {
    /* search long names */

    ptr = MSched.ConfigBuffer;
 
    while (MCfgGetSVal(
             head,
             &ptr,
             MPARM_NODECFG,
             tmpLName,
             NULL,
             Value,
             sizeof(Value),
             0,
             NULL) != FAILURE)
      {
      /* node attribute located */
 
      /* FORMAT:  <ATTR>=<VALUE>[ <ATTR>=<VALUE>]... */
 
      MNodeProcessConfig(N,Value);
      }
    }

  return(SUCCESS);
  }  /* END MNodeLoadConfig() */




int MNodeProcessConfig(

  mnode_t *N,     /* I (modified) */
  char    *Value) /* I */

  {
  char  *ptr;
  char  *TokPtr;

  char  *ptr2;
  char  *TokPtr2;

  char   ValLine[MAX_MNAME];
  char   AttrArray[MAX_MNAME];

  int    aindex;

  int    tmpI;
  double tmpD;

  mpar_t  *P;

  int    SlotsUsed = -1;

  if ((N == NULL) || 
      (Value == NULL) ||
      (Value[0] == '\0'))
    {
    return(FAILURE);
    }

  ptr = MUStrTokE(Value," \t\n",&TokPtr);
 
  while (ptr != NULL)
    {
    /* parse name-value pairs */
 
    /* FORMAT:  <ATTR>=<VALUE>[,<VALUE>] */
 
    if (MUGetPair(
          ptr,
          (const char **)MNodeAttr,
          &aindex,
	  AttrArray,
          TRUE,
          NULL,
          ValLine,
          MAX_MNAME) == FAILURE)
      {
      /* cannot parse value pair */
 
      ptr = MUStrTokE(NULL," \t\n",&TokPtr);
 
      continue;
      }
 
    switch(aindex)
      {
      case mnaAccess:
     
        /* NYI */

        break;

      case mnaAvlMemW:

        if ((N->P != NULL) || 
           ((N->P = calloc(1,sizeof(mnprio_t))) != NULL))
          {
          N->P->CW[mnpcAMem] = strtod(ValLine,NULL);
          N->P->CWIsSet = TRUE;
          }

        break;

      case mnaAvlProcW:

        if ((N->P != NULL) ||
           ((N->P = calloc(1,sizeof(mnprio_t))) != NULL))
          {
          N->P->CW[mnpcAProcs] = strtod(ValLine,NULL);    
          N->P->CWIsSet = TRUE;
          }
 
        break;

      case mnaCfgDisk:

        N->CRes.Disk = (int)strtol(ValLine,NULL,0);

        break;

      case mnaCfgMem:

        N->CRes.Mem = (int)strtol(ValLine,NULL,0);
 
        break;

      case mnaCfgMemW:

        if ((N->P != NULL) ||
           ((N->P = calloc(1,sizeof(mnprio_t))) != NULL))
          {
          N->P->CW[mnpcCMem] = strtod(ValLine,NULL);    
          N->P->CWIsSet = TRUE;
          }
 
        break;

      case mnaCfgProcW:

        if ((N->P != NULL) ||
           ((N->P = calloc(1,sizeof(mnprio_t))) != NULL))
          {
          N->P->CW[mnpcCProcs] = strtod(ValLine,NULL);    
          N->P->CWIsSet = TRUE;
          }
 
        break;

      case mnaCfgSwap:

        N->CRes.Swap = (int)strtol(ValLine,NULL,0);

        break;

      case mnaFeatures:
 
        /* FORMAT:  <FEATURE>[:<FEATURE>]... */
 
        ptr2 = MUStrTok(ValLine,":",&TokPtr2);
 
        while (ptr2 != NULL)
          {
          MUGetMAttr(eFeature,ptr2,mAdd,N->FBM,sizeof(N->FBM));
 
          ptr2 = MUStrTok(NULL,", \t",&TokPtr2);
          }
 
        break;

      case mnaFrame:

        { 
        /* FORMAT:  <FRAMEINDEX>|<FRAMENAME> */

        if (isdigit(ValLine[0]))
          {
          tmpI = (int)strtol(ValLine,NULL,0);
          }
        else
          {
          if (MFrameAdd(ValLine,&tmpI,NULL) == FAILURE)
            {
            /* cannot add frame */

            break;
            }
          }

        MNodeSetAttr(N,mnaFrame,(void **)&tmpI,mdfInt,mSet);
        }  /* END BLOCK */
 
        break;

      case mnaGRes:

        MNodeSetAttr(N,aindex,(void *)ValLine,mdfString,mSet);

        break;

      case mnaLoadW:
 
        if ((N->P != NULL) ||
           ((N->P = calloc(1,sizeof(mnprio_t))) != NULL))
          {
          N->P->CW[mnpcLoad] = strtod(ValLine,NULL);    
          N->P->CWIsSet = TRUE;
          }
 
        break;

      case mnaMaxJob:

        tmpI = (int)strtol(ValLine,NULL,0);

	{
        if (AttrArray[0] == '\0')
          {
          N->AP.HLimit[mptMaxJob][0] = tmpI;
          N->AP.SLimit[mptMaxJob][0] = tmpI;
          }
        else
          {
          /* NYI */
          }

        break;

      case mnaMaxProc:

	tmpI = (int)strtol(ValLine,NULL,0);

        if (AttrArray[0] == '\0')
          {
          N->AP.HLimit[mptMaxProc][0] = tmpI;
          N->AP.SLimit[mptMaxProc][0] = tmpI;
          }
	else
          {
          char *otype;
          char *oid;

          char *TokPtr;

          /* FORMAT:  {CLASS}[:<OID>] */

          /* NOTE:  temp non-generalized implementation */

          if (((otype = MUStrTok(AttrArray,":",&TokPtr)) == NULL) ||
              ((oid   = MUStrTok(NULL,":",&TokPtr)) == NULL))
            {
            break;
            }

          if (!strcmp(otype,"CLASS"))
            {
            mclass_t *C;

            if (N->MaxProcPerClass == NULL)
              N->MaxProcPerClass = (int *)calloc(1,sizeof(int) * MAX_MCLASS);

	    if (MClassAdd(oid,&C) == SUCCESS)
              N->MaxProcPerClass[C->Index] = tmpI;
            }
          }	
        }  /* END BLOCK */

        break;

      case mnaMaxJobPerUser:
 
        tmpI = (int)strtol(ValLine,NULL,0);
 
        N->MaxJobPerUser = tmpI;
 
        break;

      case mnaMaxLoad:
 
        N->MaxLoad = strtod(ValLine,NULL);
 
        break;

      case mnaMaxProcPerUser:

        tmpI = (int)strtol(ValLine,NULL,0);

        N->MaxProcPerUser = tmpI;

        break;

      case mnaNetwork:

        MNodeSetAttr(N,mnaNetwork,(void **)ValLine,mdfString,mSet);

        break;

      case mnaNodeType:
 
        MUStrCpy(N->NodeType,ValLine,sizeof(N->NodeType));
 
        break;

      case mnaPartition:
 
        if (MParAdd(ValLine,&P) != SUCCESS)
          {
          N->PtIndex = 1;
          P = &MPar[1];
          }

        MParAddNode(P,N);
 
        break;

      case mnaPrioF:

        MNodeProcessPrioF(N,ValLine);

        break;

      case mnaPriority:
 
        N->Priority = (int)strtol(ValLine,NULL,0);
 
        break;

      case mnaPrioW:
 
        if ((N->P != NULL) ||
           ((N->P = calloc(1,sizeof(mnprio_t))) != NULL))
          {
          N->P->CW[mnpcPriority] = strtod(ValLine,NULL);    
          N->P->CWIsSet = TRUE;
          }
 
        break;

      case mnaProcSpeed:
 
        tmpI = (int)strtol(ValLine,NULL,0);
 
        N->ProcSpeed = tmpI;
 
        break;

      case mnaRADisk:
      case mnaMaxPEPerJob:

        MNodeSetAttr(N,aindex,(void **)ValLine,mdfString,mSet);

        break;
 
      case mnaSize:

        tmpI = (int)strtol(ValLine,NULL,0);

        SlotsUsed = tmpI; 
 
        break;

      case mnaSlot:
 
        tmpI = (int)strtol(ValLine,NULL,0);

	MNodeSetAttr(N,mnaSlot,(void **)&tmpI,mdfInt,mSet);

        break;

      case mnaSpeed:
 
        tmpD = strtod(ValLine,NULL);
 
        N->Speed = MIN(100.0,tmpD);
 
        break;

      case mnaSpeedW:
 
        if ((N->P != NULL) ||
           ((N->P = calloc(1,sizeof(mnprio_t))) != NULL))
          {
          N->P->CW[mnpcSpeed] = strtod(ValLine,NULL);    
          N->P->CWIsSet = TRUE;
          }
 
        break;

      case mnaUsageW:
 
        if ((N->P != NULL) ||
           ((N->P = calloc(1,sizeof(mnprio_t))) != NULL))
          {
          N->P->CW[mnpcUsage] = strtod(ValLine,NULL);    
          N->P->CWIsSet = TRUE;
          }
 
        break;

      case mnaResource:

        /* FORMAT:  <RESNAME>:<RESCOUNT>[,<RESNAME>:<RESCOUNT>]... */

        {
        char *ptr;
        char *TokPtr;

        ptr = MUStrTok(ValLine,", \t",&TokPtr);

        while (ptr != NULL)
          {
          /* FORMAT:  <RESDESC>[{ \t,}<RESDESC]... */

          MNodeSetAttr(N,mnaGRes,(void **)ptr,mdfString,mSet);
          
          ptr = MUStrTok(NULL,", \t",&TokPtr);
          }
        }    /* END BLOCK */

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch(aindex) */

    ptr = MUStrTok(NULL," \t\n",&TokPtr);     
    }    /* END while (ptr != NULL) */

  if (SlotsUsed > 0)
    {
    N->SlotsUsed = SlotsUsed;

    if ((N->FrameIndex > 0) && (N->SlotIndex > 0))
      MSys[N->FrameIndex][N->SlotIndex].SlotsUsed = SlotsUsed;
    }

  return(SUCCESS);
  }  /* END MNodeProcessConfig() */




int MNodeRemove(

  mnode_t *N)  /* I */

  {
  const char *FName = "MNodeRemove";

  DBG(5,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL");
 
  if (N == NULL)
    {
    return(FAILURE);
    }

  if (N->FrameIndex > 0)
    MFrame[N->FrameIndex].NodeCount--;

  MNodeDestroy(&N);

  return(SUCCESS);
  }  /* MNodeRemove() */





int MNodeDestroy(

  mnode_t **NP)  /* I (modified) */

  {
  mnode_t *N;

  if ((NP == NULL) || (*NP == NULL))
    {
    return(SUCCESS);
    }

  N = *NP;

  /* release dynamic memory */

  MUFree(&N->FullName);

  if (N->R != NULL)
    free(N->R);

  if (N->RE != NULL)
    free(N->RE);

  if (N->RC != NULL)
    free(N->RC);

  MUFree(&N->Message);

  MUFree((char **)&N->JList);
  MUFree((char **)&N->JTC);

  memset(N,0,sizeof(mnode_t));

  /* node Name[0] set to value '1' indicates a deleted node */        
 
  N->Name[0] = '\1';
  N->Name[1] = '\0';

  return(SUCCESS);
  }  /* END MNodeDestroy() */





int MJobGetINL(

  mjob_t    *J,            /* I */
  mnalloc_t *FNL,          /* I */
  mnalloc_t *INL,          /* O */
  int        PIndex,       /* I */
  int       *NodeCount,    /* O */
  int       *TaskCount)    /* O */

  {
  static long      IdleListMTime[MAX_MPAR];      
  static int       ActiveJobCount[MAX_MPAR];     
  static mnalloc_t IdleNodeCache[MAX_MPAR][MAX_MNODE];         
  static int       IdleNodeCount[MAX_MPAR];

  static int       InitializeTimeStamp = TRUE;
  int index;
  int nindex;

  int tmpNC;
  int tmpTC;

  int tmpI;

  mnode_t *N;

  const char *FName = "MJobGetINL";

  mpar_t *P = &MPar[PIndex];

  DBG(4,fSCHED) DPrint("%s(%s,%s,%s,%s,%s,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (FNL != NULL) ? "FNL" : "NULL",
    (INL != NULL) ? "INL" : "NULL",
    (P != NULL) ? P->Name : "NULL",
    (NodeCount != NULL) ? "NodeCount" : "NULL",
    (TaskCount != NULL) ? "TaskCount" : "NULL");

  if ((J == NULL) || (INL == NULL))
    {
    return(FAILURE);
    }

  if (InitializeTimeStamp == TRUE)
    {
    memset(IdleListMTime,0,sizeof(IdleListMTime));
    memset(ActiveJobCount,0,sizeof(ActiveJobCount));

    InitializeTimeStamp = FALSE;
    }

  /* process feasible list if provided */

  if (FNL == NULL)
    {
    /* update IdleList on new iteration or if new job has been started */
 
    if ((MSched.Time > IdleListMTime[P->Index]) ||
        (ActiveJobCount[P->Index] != MStat.ActiveJobs))
      {
      DBG(5,fSCHED) DPrint("INFO:     refreshing IdleList for partition %s\n",
        P->Name);
 
      nindex = 0;

      tmpTC = 0;
 
      for (index = 0;index < MAX_MNODE;index++)
        {
        N = MNode[index];

        if ((N == NULL) || (N->Name[0] == '\0'))
	  break;

        if (N->Name[0] == '\1')
          continue;
 
        if ((N->PtIndex != P->Index) && (PIndex != 0))
          {
          continue;
          }

        if (((N->State != mnsIdle) && (N->State != mnsActive)) ||
            ((N->EState != mnsIdle) && (N->EState != mnsActive)) ||
            (N->DRes.Procs >= N->CRes.Procs))
          {
          DBG(6,fSCHED) DPrint("INFO:     node '%s' is not idle.  (state: %s  exp: %s)\n",
            N->Name,
            MAList[eNodeState][N->State],
            MAList[eNodeState][N->EState]);

          continue;
          }

        /* acceptable node found */

        IdleNodeCache[P->Index][nindex].N  = N;
        IdleNodeCache[P->Index][nindex].TC = N->CRes.Procs - N->DRes.Procs;
 
        tmpTC += IdleNodeCache[P->Index][nindex].TC;
 
        nindex++;
        }    /* END for (index) */ 

      IdleNodeCache[P->Index][nindex].N = NULL;
 
      DBG(5,fSCHED) DPrint("INFO:     nodes placed in INL[%s] list: %3d (procs: %d)\n",
        P->Name,
        nindex,
        tmpTC);
 
      IdleListMTime[P->Index] = MSched.Time;
 
      ActiveJobCount[P->Index] = MStat.ActiveJobs;
 
      IdleNodeCount[P->Index] = nindex;
      }  /* END if (MSched.Time > IdleListMTime[P->Index])) */

    /* determine available nodes from data */

    tmpNC = 0;
    tmpTC = 0;

    nindex = 0;

    for (index = 0;index < IdleNodeCount[P->Index];index++)
      {
      N = IdleNodeCache[P->Index][index].N;

      tmpI = IdleNodeCache[P->Index][index].TC;

      if (MNodeCheckPolicies(J,N,MSched.Time,&tmpI) == FAILURE)
        {
        continue;
        }

      memcpy(
        &INL[nindex],
        &IdleNodeCache[P->Index][index],
        sizeof(INL[nindex]));

      INL[nindex].TC = tmpI;

      tmpTC += tmpI;

      nindex++;
      }  /* END for (index) */

    INL[nindex].N = NULL;
 
    tmpNC = nindex;
    }    /* END if (FNL == NULL) */
  else
    {
    /* process feasible node list */
 
    tmpTC = 0;
    nindex       = 0;
 
    for (index = 0;FNL[index].N != NULL;index++)
      {
      N = FNL[index].N;

      if (((N->State != mnsIdle) && (N->State != mnsActive)) ||
          ((N->EState != mnsIdle) && (N->EState != mnsActive)) ||
          ((N->CRes.Procs > 0) && (N->DRes.Procs >= N->CRes.Procs)))
        {
        DBG(5,fSCHED) DPrint("INFO:     node %s is not available for idle list (state: %s  dprocs: %d)\n",
          N->Name,
          MNodeState[N->State],
          N->DRes.Procs);

        continue;
        }

      if (N->CRes.Procs != 0)
        {
        tmpI = N->CRes.Procs - N->DRes.Procs;
        }
      else 
        {
        /* node is non-compute resource */

        tmpI = 9999;
        }

      if (MNodeCheckPolicies(J,N,MSched.Time,&tmpI) == FAILURE)
        {
        DBG(5,fSCHED) DPrint("INFO:     job %s violates policies on idle node %s\n",
          J->Name,
          N->Name);

        continue;
        }

      INL[nindex].N  = N;
      INL[nindex].TC = tmpI;

      DBG(5,fSCHED) DPrint("INFO:     idle node %sx%d located (D: %d)\n",
        N->Name,
        tmpI,
        N->DRes.Procs);
 
      tmpTC += tmpI;
 
      nindex++;
      }  /* END for (nindex) */

    /* terminate list */

    INL[nindex].N = NULL;

    tmpNC = nindex;
    }    /* END else (FNL == NULL) */

  if (TaskCount != NULL)
    *TaskCount = tmpTC;

  if (NodeCount != NULL)
    *NodeCount = tmpNC;

  DBG(4,fSCHED) DPrint("INFO:     idle resources (%d tasks/%d nodes) found with %sfeasible list specified\n",
    tmpTC,
    tmpNC,
    (FNL == NULL) ? "no " : "");

  return(SUCCESS);
  }  /* END MJobGetINL() */




int MNodeGetPreemptList(

  mjob_t     *PreemptorJ,   /* I */
  mnalloc_t  *FNL,          /* I (optional) */
  mnalloc_t  *PNL,          /* O */
  mjob_t    **PreemptJob,   /* O: preemptible jobs */
  long        Priority,       
  int         PIndex,
  int         IsConditional,
  int        *NodeCount,
  int        *TaskCount)
 
  {
  static long       PreemptListMTime[MAX_MPAR];
  static int        ActiveJobCount[MAX_MPAR];

  static mnalloc_t  PreemptNodeCache[MAX_MPAR][MAX_MNODE];
  static int        PreemptTaskCount[MAX_MPAR];
  static int        PreemptNodeCount[MAX_MPAR];
  static mjob_t    *PreemptJobCache[MAX_MPAR][MAX_MJOB];

  static long       CachePriority;
 
  static mbool_t    InitializeTimeStamp = TRUE;

  int index;
  int index2;

  int nindex;
  int jindex;
  int jindex2;
 
  int tmpNC;
  int tmpTC;

  int TC;

  mbool_t JobIsPreemptible;

  int      TA;
  long     TD;
 
  mnode_t *N;
  mjob_t  *J;

  mjob_t  *tJ;

  int      rc;

  int      RATail;
  
#define MAX_MPJOB 16

  mjob_t  *tmpJ[MAX_MPJOB];

  mcres_t PreemptRes;

  const char *FName = "MNodeGetPreemptList";

  DBG(4,fSCHED) DPrint("%s(J,%s,PNode,PJob,%ld,%s,NC,TC)\n",
    FName,
    (FNL != NULL) ? "FNL" : "NULL",
    Priority, 
    MAList[ePartition][PIndex]);

  /* verify parameters */

  if (PNL == NULL)
    {
    return(FAILURE);
    }

  if (PreemptJob != NULL)
    PreemptJob[0] = NULL;
 
  if (InitializeTimeStamp == TRUE)
    {
    memset(PreemptListMTime,0,sizeof(PreemptListMTime));
    memset(ActiveJobCount,0,sizeof(ActiveJobCount));

    CachePriority = -1;
 
    InitializeTimeStamp = FALSE;
    }

  if (IsConditional == TRUE)
    {
    int rindex;
    int nindex2;

    mres_t *R;

    mbool_t NodeIsPreemptee;

    int PJCount = 0;
    int PNCount = 0;
    int PTCount = 0;

    /* locate active 'owned' reservations */

    /* all active jobs within reservation are preemptible */

    index = 0;

    for (nindex = 0;nindex < MMAX_NODE;nindex++)
      {
      if (FNL == NULL)
        N = MNode[nindex];
      else
        N = FNL[nindex].N;

      if (N == NULL)
        break;

      NodeIsPreemptee = FALSE;

      R = NULL;  /* NOTE: for compiler warnings only */

      for (rindex = 0;N->R[rindex] != NULL;rindex++)
        {
        R = N->R[rindex];

        if (R == NULL)
          break;

        if ((R == (mres_t *)1) ||
            (R->Name[0] == '\0') ||
            (R->Name[0] == '\1'))
          continue;

        if ((MISSET(R->Flags,mrfOwnerPreempt) == FALSE) ||
            (R->IsActive == FALSE))
          continue;

        if (MCredIsMatch(&PreemptorJ->Cred,R->O,R->OType) == FAILURE)
          continue;

        NodeIsPreemptee = TRUE;

        break;
        }  /* END for (rindex) */

      if (NodeIsPreemptee == FALSE)
        continue;

      /* select all non-owner jobs */

      for (jindex = 0;jindex < N->MaxJCount;jindex++)
        {
        tJ = N->JList[jindex];

        if (tJ == NULL)
          break;

        if (tJ == (mjob_t *)1)
          continue;

        if (MCredIsMatch(&tJ->Cred,R->O,R->OType) == SUCCESS)
          continue;

        NodeIsPreemptee = TRUE;

        break;
        }  /* END for (rindex) */

      if (NodeIsPreemptee == FALSE)
        continue;

      /* select all non-owner jobs */

      for (jindex = 0;jindex < N->MaxJCount;jindex++)
        {
        tJ = N->JList[jindex];

        if (tJ == NULL)
          break;

        if (tJ == (mjob_t *)1)
          continue;

        if (MCredIsMatch(&tJ->Cred,R->O,R->OType) == SUCCESS)
          continue;

        /* is job unique? */

        for (jindex2 = 0;jindex2 < PJCount;jindex2++)
          {
          if (PreemptJob[jindex2] == tJ)
            break;
          }  /* END for (jindex2) */

        if (jindex2 >= PJCount)
          {
          /* add job if unique */

          PreemptJob[jindex2] = tJ;

          PJCount++;
          }  /* END if (PreemptJob[jindex2] != tJ) */

        /* add new resources to PNL */

        for (nindex2 = 0;nindex2 < PNCount;nindex2++)
          {
          if (PNL[nindex2].N == N)
            break;
          }  /* END for (nindex2) */

        if (nindex2 == PNCount)
          {
          /* unique node located */

          /* NOTE:  currently only adding node evaluated */

          PNL[nindex2].N  = N;
          PNL[nindex2].TC = N->JTC[jindex];

          PTCount += N->JTC[jindex];
          PNCount++;
          }    /* END if (nindex2 == PNCount) */
        }      /* END for (jindex) */
      }        /* END for (nindex) */

    PNL[PNCount].N = NULL;
    PreemptJob[PJCount]   = NULL;

    DBG(4,fSCHED) DPrint("INFO:     preemptible resources found.  (%d nodes/%d tasks)\n",
      PNCount,
      PTCount);

    if (TaskCount != NULL)
      *TaskCount = PTCount;

    if (NodeCount != NULL)
      *NodeCount = PNCount;

    return(SUCCESS);
    }  /* END if (IsConditional == TRUE) */
 
  /* process feasible list if provided */
 
  if (FNL == NULL)
    {
    /* update PreemptList on new iteration or if new job has been started */
 
    if ((MSched.Time > PreemptListMTime[PIndex]) ||
        (ActiveJobCount[PIndex] != MStat.ActiveJobs) ||
        (Priority != CachePriority))
      {
      DBG(5,fSCHED) DPrint("INFO:     refreshing PreemptList for partition %s (prio: %ld)\n",
        MAList[ePartition][PIndex],
        Priority);

      PreemptJobCache[PIndex][0] = NULL;
 
      nindex = 0;
 
      tmpTC = 0;
 
      for (index = 0;index < MAX_MNODE;index++)
        {
        N = MNode[index];

        if ((N == NULL) || (N->Name[0] == '\0'))
          break;
 
        if (N->Name[0] == '\1')
          continue;
 
        if ((N->PtIndex != PIndex) && (PIndex != 0))
          {
          continue;
          }

        /* node is preemptible if              */
        /* - node is busy or running           */
        /* - active jobs are preemptible       */
        /* - active jobs are of lower priority */
 
        if ((N->State != mnsActive) && (N->State != mnsBusy))
          {
          continue;
          }

        memset(&PreemptRes,0,sizeof(PreemptRes));

        for (jindex = 0;jindex < N->MaxJCount;jindex++)
          {
          if (N->JList[jindex] == NULL)
            break;

          if (N->JList[jindex] == (mjob_t *)1)
            continue;

          J  = N->JList[jindex];
          TC = N->JTC[jindex];

          if ((J->State != mjsStarting) && (J->State != mjsRunning))
            continue;

          if (J->Flags & (1 << mjfPreemptee)) 
            {
            JobIsPreemptible = TRUE;
            }
          else
            {
            JobIsPreemptible = FALSE;
            }

          if ((JobIsPreemptible == TRUE) &&
              (J->StartPriority < Priority))
            {
            MCResAdd(&PreemptRes,&N->CRes,&J->Req[0]->DRes,TC,FALSE);  /* FIXME */

            for (jindex2 = 0;PreemptJobCache[PIndex][jindex2] != NULL;jindex2++)
              {
              if (PreemptJobCache[PIndex][jindex2] == J)
                break; 
              }  /* END for (jindex2) */

            if (PreemptJobCache[PIndex][jindex2] == NULL)
              {
              PreemptJobCache[PIndex][jindex2]     = J;
              PreemptJobCache[PIndex][jindex2 + 1] = NULL;
              }

            DBG(6,fSCHED) DPrint("INFO:     job %s on node %s added to preempt list (Proc: %d)\n",
              J->Name,
              N->Name,
              PreemptRes.Procs);
            }
          else
            {
            DBG(7,fSCHED) DPrint("INFO:     cannot preempt job %s on node %s (%s) P: %ld < %ld\n",
              J->Name,
              N->Name,
              (J->Flags & (1 << mjfPreemptee)) ? "priority too high" : "not preemptible",
              Priority,
              J->StartPriority);
            }
          }  /* END for (jindex) */

        if (PreemptRes.Procs <= 0)
          continue;

        PreemptNodeCache[PIndex][nindex].N  = N;
        PreemptNodeCache[PIndex][nindex].TC = PreemptRes.Procs;
 
        tmpTC += PreemptNodeCache[PIndex][nindex].TC;

        nindex++;
        }  /* END for (index) */

      PreemptNodeCache[PIndex][nindex].N = NULL;
 
      DBG(5,fSCHED) DPrint("INFO:     nodes placed in PNL[%d] list: %3d (procs: %d)\n",
        PIndex,
        nindex,
        tmpTC);
 
      PreemptListMTime[PIndex] = MSched.Time;
 
      ActiveJobCount[PIndex]        = MStat.ActiveJobs;
 
      PreemptTaskCount[PIndex]      = tmpTC;
      PreemptNodeCount[PIndex]      = nindex;
      }  /* END if (MSched.Time > PreemptListMTime[PIndex])) */
 
    /* copy cached data */

    for (nindex = 0;nindex < PreemptNodeCount[PIndex];nindex++)
      {
      memcpy(
        &PNL[nindex],
        &PreemptNodeCache[PIndex][nindex],
        sizeof(PNL[nindex]));
      }

    if (PreemptJob != NULL)
      {
      for (jindex = 0;PreemptJobCache[PIndex][jindex] != NULL;jindex++)
        {
        PreemptJob[jindex] = PreemptJobCache[PIndex][jindex];
        }

      PreemptJob[jindex] = NULL;
      }

    PNL[nindex].N = NULL;
 
    tmpTC = PreemptTaskCount[PIndex];
    tmpNC = PreemptNodeCount[PIndex];
    }    /* END if (FNL == NULL) */
  else
    {
    /* process feasible node list */
 
    tmpTC = 0;
    nindex       = 0;
 
    for (index = 0;FNL[index].N != NULL;index++)
      {
      N = FNL[index].N;

      if ((N->PtIndex != PIndex) && (PIndex != 0))
        {
        continue;
        }
 
      /* node is preemptible if              */
      /* - node is busy or running           */
      /* - active jobs are preemptible       */
      /* - active jobs are of lower priority */
 
      if ((N->State != mnsActive) && (N->State != mnsBusy))
        {
        continue;
        }
 
      memset(&PreemptRes,0,sizeof(PreemptRes));

      index2 = 0;
 
      for (jindex = 0;jindex < N->MaxJCount;jindex++)
        {
        if (N->JList[jindex] == NULL)
          break;

        if (N->JList[jindex] == (mjob_t *)1)
          continue;

        J  = (mjob_t *)N->JList[jindex];
        TC = N->JTC[jindex];
 
        if ((J->Flags & (1 << mjfPreemptee)) &&
            (J->StartPriority < Priority))
          {
          tmpJ[index2++] = J;

          MCResAdd(&PreemptRes,&N->CRes,&J->Req[0]->DRes,TC,FALSE);  /* FIXME */

          if (PreemptJob != NULL)
            {
            for (jindex2 = 0;PreemptJob[jindex2] != NULL;jindex2++)
              {
              if (PreemptJob[jindex2] == J)
                break;
              }  /* END for (jindex2) */
 
            if (PreemptJob[jindex2] == NULL)
              {
              DBG(5,fSCHED) DPrint("INFO:     new preemptible job %s located on node %s (%ld < %ld)\n",
                J->Name,
                N->Name,
                J->StartPriority,
                Priority);

              PreemptJob[jindex2]     = J;
              PreemptJob[jindex2 + 1] = NULL;
              }
            else
              {
              DBG(5,fSCHED) DPrint("INFO:     preemptible job %s located on node %s\n",
                J->Name,
                N->Name);
              }
            }
          }
        }  /* END for (jindex) */

      if (PreemptRes.Procs <= 0)
        continue;

      /* adequate resources available if all preemptible resources are */
      /* preempted.  Must now verify that other reservations do not block */
      /* use of node */

      RATail = -1;

      for (jindex = 0;jindex < index2;jindex++)
        {
        if (jindex >= MAX(MMAX_JOBRA,MAX_MPJOB))
          break;

        if (RATail == -1)
          {
          if (PreemptorJ->RAList[jindex][0] == '\0')
            RATail = jindex;
          else
            continue;
          }

        MUStrCpy(
          PreemptorJ->RAList[jindex],
          tmpJ[jindex - RATail]->Name,
          sizeof(PreemptorJ->RAList[jindex]));
        }  /* END for (jindex) */
    
      rc = MJobGetNRange(
             PreemptorJ,
             PreemptorJ->Req[0],
             N,
             MSched.Time + 1,
             &TA,
             &TD,
             NULL,
             NULL,
             NULL);

      /* erase temporary access list data */

      for (jindex = 0;jindex < index2;jindex++)
        {
        if (jindex >= MAX_MPJOB)
          break;

        PreemptorJ->RAList[jindex + RATail][0] = '\0';
        }
 
      if ((rc == FAILURE) || 
          (TA * PreemptorJ->Req[0]->DRes.Procs < N->DRes.Procs))
        {
        /* node cannot be completely preempted */

        /* why is dedicated node access an issue? */
	
	if (rc == FAILURE)	
          continue;
        }

      PNL[nindex].N  = N;
      PNL[nindex].TC = PreemptRes.Procs;
 
      tmpTC += PNL[nindex].TC;
 
      nindex++;

      DBG(5,fSCHED) DPrint("INFO:     node %s contains preemptible resources (P: %d)\n",
        N->Name,
        PreemptRes.Procs);
      }  /* END for (nindex) */

    /* terminate list */

    PNL[nindex].N = NULL;
 
    tmpNC = nindex;
    }    /* END else (FNL == NULL) */

  DBG(4,fSCHED) DPrint("INFO:     preemptible resources found.  (%d nodes/%d tasks)\n",
    tmpNC,
    tmpTC);
   
  if (TaskCount != NULL)
    *TaskCount = tmpTC;
 
  if (NodeCount != NULL)
    *NodeCount = tmpNC;
 
  return(SUCCESS);
  }  /* END MNodeGetPreemptList() */




int MJobAddToNL(

  mjob_t     *J,  /* I */
  nodelist_t  NL) /* I (optional) */

  {
  int      nindex;
  mnode_t *N;

  int      rqindex;
  mreq_t  *RQ;
  int      FIndex;

  int      jindex;

  int      PC;

  const char *FName = "MJobAddToNL";

  DBG(4,fSCHED) DPrint("%s(%s,%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (NL != NULL) ? "NL" : "NULL");

  if (J == NULL)
    {
    return(FAILURE);
    }

  if ((J->State != mjsRunning) && 
      (J->State != mjsStarting) &&
      (J->State != mjsSuspended))
    {
    /* no affect on resources */

    /* NOTE:  what is affect of suspended jobs? */

    return(SUCCESS);
    }

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];
 
    for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
      {
      N = RQ->NodeList[nindex].N;

      FIndex = -1;

      for (jindex = 0;jindex < N->MaxJCount;jindex++)
        { 
        if ((N->JList == NULL) || (N->JList[jindex] == NULL))
          break;

        if ((FIndex == -1) && (N->JList[jindex] == (mjob_t *)1))
          FIndex = jindex;
         
        if (N->JList[jindex] == J) 
          {
          if (rqindex == 0)
            N->JTC[jindex] = 0;

          break;
          }
        }    /* END for (jindex) */

      if (jindex >= N->MaxJCount)
        {
        /* node joblist is full */

        DBG(4,fSCHED) DPrint("INFO:     job table extended to size %d on node %s for job %s)\n",
          N->MaxJCount + MAX_MJOB_PER_NODE,
          N->Name,
          J->Name);

        N->JList = (void **)realloc(
          N->JList,
          sizeof(void *) * (N->MaxJCount + MAX_MJOB_PER_NODE + 1));

        N->JTC = (int *)realloc(
          N->JTC,
          sizeof(int) * (N->MaxJCount + MAX_MJOB_PER_NODE + 1));

        if ((N->JList == NULL) || (N->JTC == NULL))
          {
          DBG(4,fSCHED) DPrint("ERROR:    cannot extend node job tables\n");

          return(FAILURE);
          }

        N->JList[jindex] = NULL;

        N->MaxJCount = N->MaxJCount + MAX_MJOB_PER_NODE;
        }  /* END if (jindex >= N->MaxJCount) */

      if (N->JList[jindex] == NULL)
        {
        if (FIndex != -1)
          jindex = FIndex;

        N->JList[jindex] = J;
        N->JTC[jindex]   = 0;
 
        N->JList[jindex + 1] = NULL;
        }

      N->JTC[jindex] += RQ->NodeList[nindex].TC;

      if (RQ->DRes.Procs == -1)
        PC = N->CRes.Procs;
      else
        PC = RQ->NodeList[nindex].TC * RQ->DRes.Procs;

      {
      int JCount = 0;
      int PCount = 0;

      int tindex;

      mjob_t *tJ;

      for (jindex = 0;jindex < N->MaxJCount;jindex++)
        {
        if (N->JList[jindex] == NULL)
          break;
 
        if (N->JList[jindex] == (mjob_t *)1)
          continue;

        JCount++;

        tJ = (mjob_t *)N->JList[jindex];

        for (tindex = 0;tJ->TaskMap[tindex] != -1;tindex++)
          {
          if (tJ->TaskMap[tindex] != N->Index)
            continue;

          /* FIXME:  must adjust for multi-req jobs */

          PCount += tJ->Req[0]->DRes.Procs;
          }  /* END for (tindex) */
        }  /* END for (jindex) */

      N->AP.Usage[mptMaxJob][0]  = JCount;
      N->AP.Usage[mptMaxProc][0] = PCount;
      }  /* END BLOCK */
 
      /* adjust 'job slot' limits */

      if ((N->RM == NULL) || 
         ((N->RM->Type != mrmtLL) && (N->RM->Type != mrmtWiki)))
        {
        /* adjust available job slots */

        N->ARes.PSlot[0].count = MAX(0,N->ARes.PSlot[0].count - PC);

        if (J->Cred.C != NULL)
          {
          N->ARes.PSlot[J->Cred.C->Index].count = MAX(
            0,
            N->ARes.PSlot[J->Cred.C->Index].count - PC);
	  }

        DBG(4,fSCHED) DPrint("INFO:     node %s added to job %s.  PSlot: %s\n",
          N->Name,
          J->Name,
          MUCAListToString(N->ARes.PSlot,N->CRes.PSlot,NULL));
        }    /* END if (N->RM != NULL) */

      if (N == MSched.GN)
        {
        /* adjust GRes */

        MNodeSetState(N,mnsActive,0);

        MCResRemove(
          &N->ARes,
          &N->CRes,
          &RQ->DRes,
          RQ->NodeList[nindex].TC,
          TRUE);
        }
      }      /* END for (nindex) */
    }        /* END for (rqindex) */

  return(SUCCESS);
  }  /* END MJobAddToNL() */
   



int MJobRemoveFromNL(
 
  mjob_t     *J,  /* I */
  nodelist_t  NL) /* I */
 
  {
  int      nindex;
  mnode_t *N;
 
  int      rqindex;
  mreq_t  *RQ;
 
  int      jindex;
 
  if (J == NULL)
    {
    return(FAILURE);
    }
 
  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];
 
    for (nindex = 0;RQ->NodeList[nindex].N != NULL;nindex++)
      {
      N = RQ->NodeList[nindex].N;
 
      for (jindex = 0;jindex < N->MaxJCount;jindex++)
        {
        if (N->JList[jindex] == NULL)
          break;

        if (N->JList[jindex] != J)
          continue;

        /* set 'empty' marker */

        N->JList[jindex] = (mjob_t *)1;

        N->AP.Usage[mptMaxJob][0]--;

        N->JTC[jindex] = 0;
        }  /* END for (jindex) */
      }    /* END for (nindex) */
    }      /* END for (rqindex) */
 
  return(SUCCESS);
  }  /* END MJobRemoveFromNL() */




int MNodeSelectIdleTasks(

  mjob_t      *J,              /* I */
  mreq_t      *SRQ,            /* I (optional) */
  mnalloc_t   *FNL,            /* I (optional) */
  mnodelist_t  IMNL,           /* O: idle nodes */
  int         *TaskCount,      /* O */
  int         *NodeCount,      /* O */
  char        *NodeMap,
  int          RejReason[MAX_MREQ_PER_JOB][MAX_MREJREASON])

  {
  mreq_t  *RQ;
  mnode_t *N;

  int     rqindex;
  int     nindex;
  int     index;
  int     RIndex;
  char    NodeAffinity;

  int     TotalTaskCount;
  int     TotalNodeCount;

  mnuml_t tmpPSlot[MAX_MCLASS]; 

  long    ATime;

  mnalloc_t *NodeList;

  int     ANC[MAX_MREQ_PER_JOB];
  int     ATC[MAX_MREQ_PER_JOB];

  _mpnodealloc_t MPN[MAX_MNODE];

  int     tmpTC;

  mbool_t InadequateTasks = FALSE;

  const char *FName = "MNodeSelectIdleTasks";

  DBG(4,fSCHED) DPrint("%s(%s,%d,SrcNL,IdleMNL,TC,NC,NMap,RCount,RejReason)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (SRQ != NULL) ? SRQ->Index : -1);

  if (TaskCount != NULL)
    *TaskCount = 0;
 
  if (NodeCount != NULL)
    *NodeCount = 0;

  if (RejReason != NULL)
    MU2dMemSet((void **)RejReason,0,MAX_MREQ_PER_JOB,sizeof(RejReason[0]));

  if ((J == NULL) || 
      (FNL == NULL) ||
      (IMNL == NULL))
    {
    return(FAILURE);
    }

  TotalTaskCount = 0;
  TotalNodeCount = 0;

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    {
    RQ = J->Req[rqindex];

    if ((SRQ != NULL) && (RQ != SRQ))
      continue;

    if (J->State == mjsSuspended)
      {
      /* if job is suspended, PSlots are already allocated.  */
      /* do not request additional PSlots                    */

      memcpy(tmpPSlot,RQ->DRes.PSlot,sizeof(tmpPSlot));
      memset(RQ->DRes.PSlot,0,sizeof(tmpPSlot));
      }
 
    NodeList = (mnalloc_t *)IMNL[rqindex];
 
    nindex = 0;
 
    ATC[rqindex] = 0;
    ANC[rqindex] = 0;
 
    for (index = 0;index < MAX_MNODE;index++)
      {
      if (FNL[index].N == NULL)
        break;
 
      N = FNL[index].N;

      if (MJobCheckNRes(
            J,
            N,
            RQ,
            MSched.Time,
            &tmpTC,
            1.0,
            &RIndex,
            &NodeAffinity,
	    &ATime,
            FALSE) == FAILURE)
        {
        /* record why job failed requirements check */

        if (RejReason != NULL)
          RejReason[rqindex][RIndex]++;

        if (NodeMap != NULL) 
          NodeMap[N->Index] = nmUnavailable;

        continue;
        }
 
      if (J->Flags & (1 << mjfAdvReservation))
        {
        if ((NodeAffinity == nmPositiveAffinity) ||
            (NodeAffinity == nmNeutralAffinity) ||
            (NodeAffinity == nmNegativeAffinity) ||
            (NodeAffinity == nmRequired))
          {
          DBG(7,fSCHED) DPrint("INFO:     node '%s' added (reserved)\n",
            N->Name);
          }
        else
          {
          DBG(7,fSCHED) DPrint("INFO:     node '%s' rejected (not reserved)\n",
            N->Name);
 
          NodeMap[N->Index] = nmUnavailable;
 
          continue;
          }
        }

      if (MPar[0].NAllocPolicy == mnalLastAvailable)
        { 
        /* NOTE:  sort jobs in min avail time first order */
      
        MPN[nindex].N     = N;
        MPN[nindex].TC    = MIN(tmpTC,FNL[index].TC);
        MPN[nindex].ATime = ATime;

        NodeList[nindex].N  = N;
        NodeList[nindex].TC = MIN(tmpTC,FNL[index].TC);
        }
      else
        {
        NodeList[nindex].N  = N;
        NodeList[nindex].TC = MIN(tmpTC,FNL[index].TC);
        }
 
      if (RQ->TasksPerNode > 0)
        {
        NodeList[nindex].TC -=
          (short)(NodeList[nindex].TC % RQ->TasksPerNode);
        }
 
      ATC[rqindex] += NodeList[nindex].TC;
      ANC[rqindex] ++;
 
      NodeMap[NodeList[nindex].N->Index] = NodeAffinity;
 
      DBG(6,fSCHED) DPrint("INFO:     node[%d] %s added to task list (%d tasks : %d tasks total)\n",
        nindex,
        N->Name,
        NodeList[nindex].TC,
        ATC[rqindex]);
 
      nindex++;
      }    /* END for (index)  */

    if (J->State == mjsSuspended)
      {
      /* if job is suspended, PSlots are already allocated.  */
      /* do not request additional PSlots                    */

      memcpy(RQ->DRes.PSlot,tmpPSlot,sizeof(tmpPSlot));
      }

    if (MPar[0].NAllocPolicy == mnalLastAvailable)
      {
      int index;

      /* sort node list */

      qsort(
        (void *)MPN,
        nindex,
        sizeof(_mpnodealloc_t),
        (int(*)(const void *,const void *))__MNLATimeCompLToH);

      /* copy list into NodeList */

      for (index = 0;index < nindex;index++)
        {
        NodeList[index].N  = MPN[index].N;
        NodeList[index].TC = MPN[index].TC;
        }  /* END for (index) */ 
      }    /* END if (MPar[0].NAllocPolicy == mnalLastAvailable) */

    /* terminate list */

    NodeList[nindex].N = NULL;

    TotalTaskCount += ATC[rqindex];
    TotalNodeCount += ANC[rqindex];

    if (ATC[rqindex] < RQ->TaskCount)
      {
      DBG(3,fSCHED) DPrint("INFO:     inadequate feasible tasks found for job %s:%d (%d < %d)\n",
        J->Name,
        rqindex,
        ATC[rqindex],
        RQ->TaskCount);

      InadequateTasks = TRUE;
      }
 
    if ((RQ->NodeCount > 0) &&
        (ANC[rqindex] < RQ->NodeCount))
      {
      DBG(3,fSCHED) DPrint("INFO:     inadequate nodes found for job %s:%d (%d < %d)\n",
        J->Name,
        rqindex, 
        ANC[rqindex],
        RQ->NodeCount);

      InadequateTasks = TRUE; 
      }
    }      /* END for (rqindex) */

  NodeList = (mnalloc_t *)IMNL[rqindex];    
 
  NodeList[0].N = NULL;

  if (TaskCount != NULL)
    *TaskCount = TotalTaskCount;
 
  if (NodeCount != NULL)
    *NodeCount = TotalNodeCount;

  if (InadequateTasks == TRUE)
    {
    return(FAILURE);
    }

  if (J->Flags & (1 << mjfAllocLocal))
    {
    /* check nodelist locality */

    if (MJobProximateMNL(
         J,
         IMNL,
         IMNL,
         MSched.Time,
         FALSE) == FAILURE)
      {
      DBG(4,fSCHED) DPrint("INFO:     insufficient proximity in available tasks\n");

      IMNL[0][0].N = NULL;

      return(FAILURE);
      }
    }    /* END if (J->Flags & (1 << mjfAllocLocal)) */

  return(SUCCESS);
  }  /* END MNodeSelectIdleTasks() */




int MNodeSelectPreemptTasks(
 
  mjob_t      *J,
  mnalloc_t   *FNL,               /* I:  nodes w/preempt tasks    */
  mnodelist_t  PMNL,              /* O:  nodes w/J preempt tasks  */
  int         *TaskCount,         /* I/O:  tasks previously found */
  int         *NodeCount,         /* I/O:  nodes previously found */
  char        *NodeMap,
  int          RejReason[MAX_MREQ_PER_JOB][MAX_MREJREASON],
  long         Mode)
 
  {
  mreq_t  *RQ;
  mnode_t *N;
 
  int     rqindex;
  int     nindex;
  int     index;
  int     RIndex;
  char    NodeAffinity;
 
  int     TotalTaskCount;
  int     TotalNodeCount;
 
  mnalloc_t *NodeList;
 
  int     ANC[MAX_MREQ_PER_JOB];
  int     ATC[MAX_MREQ_PER_JOB];
 
  int     tmpTC;
 
  int     TasksNeeded;
  int     NodesNeeded;

  const char *FName = "MNodeSelectPreemptTasks";

  DBG(4,fSCHED) DPrint("%s(%s,%s,PreemptMNL,TC,NC,NMap,RCount,RejReason,%ld)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (FNL != NULL) ? "FNL" : "NULL",
    Mode);

  if ((J == NULL) ||
      (FNL == NULL) ||
      (PMNL == NULL))
    {
    return(FAILURE);
    }

  if ((TaskCount == NULL) || (NodeCount == NULL))
    {
    return(FAILURE);
    }

  RQ = J->Req[0];

  TotalNodeCount = 0;
  TotalTaskCount = 0;

  if ((*TaskCount >= RQ->TaskCount) && 
     ((RQ->NodeCount == 0) || (*NodeCount > RQ->NodeCount)))
    {
    return(SUCCESS);
    }
  else
    {
    TasksNeeded = RQ->TaskCount - *TaskCount;
    NodesNeeded = RQ->NodeCount - *NodeCount;
    }
 
  for (rqindex = 0;J->Req[rqindex] != 0;rqindex++)
    {
    RQ = J->Req[rqindex];
 
    NodeList = (mnalloc_t *)PMNL[rqindex];
 
    nindex = 0;
 
    ATC[rqindex] = 0;
    ANC[rqindex] = 0;
 
    for (index = 0;index < MAX_MNODE;index++)
      {
      if (FNL[index].N == NULL)
        break;
 
      N = FNL[index].N;

      /* OUCH:  for now, ignore near term reservations */
 
      if (MJobCheckNRes(
            J,
            N,
            RQ,
            MAX_MTIME,
            &tmpTC,
            1.0,
            &RIndex,
            &NodeAffinity,
	    NULL,
            FALSE) == FAILURE)
        {
        /* record why job failed requirements check */
 
        RejReason[rqindex][RIndex]++;
 
        if (NodeMap != NULL)
          NodeMap[N->Index] = nmUnavailable;
 
        continue;
        }
 
      if (J->Flags & (1 << mjfAdvReservation))
        {
        if ((NodeAffinity == nmPositiveAffinity) ||
            (NodeAffinity == nmNeutralAffinity) ||
            (NodeAffinity == nmNegativeAffinity) ||
            (NodeAffinity == nmRequired))
          {
          DBG(7,fSCHED) DPrint("INFO:     node '%s' added (reserved)\n",
            N->Name);
          }
        else
          {
          DBG(7,fSCHED) DPrint("INFO:     node '%s' rejected (not reserved)\n",
            N->Name);
 
          NodeMap[N->Index] = nmUnavailable;
 
          continue;
          }
        }
 
      NodeList[nindex].N  = N;
      NodeList[nindex].TC = MIN(tmpTC,FNL[index].TC);
 
      if (RQ->TasksPerNode > 0)
        {
        NodeList[nindex].TC -=
          (short)(NodeList[nindex].TC % RQ->TasksPerNode);
        }
 
      ATC[rqindex] += NodeList[nindex].TC;
      ANC[rqindex] ++;
 
      NodeMap[NodeList[nindex].N->Index] = NodeAffinity;
 
      DBG(6,fSCHED) DPrint("INFO:     node[%d] %s added to task list (%d tasks : %d tasks total)\n",
        nindex,
        N->Name,
        NodeList[nindex].TC,
        ATC[rqindex]);
 
      nindex++;
      }    /* END for (index)  */
 
    /* terminate list */
 
    NodeList[nindex].N = NULL;
 
    TotalTaskCount += ATC[rqindex];
    TotalNodeCount += ANC[rqindex];
    }      /* END for (rqindex) */
 
  NodeList = (mnalloc_t *)PMNL[rqindex];
 
  NodeList[0].N = NULL;

  if (TaskCount != NULL)
    *TaskCount = TotalTaskCount;
 
  if (NodeCount != NULL)
    *NodeCount = TotalNodeCount;
 
  if ((TotalTaskCount < TasksNeeded) || 
      (TotalNodeCount < NodesNeeded))
    {
    DBG(6,fSCHED) DPrint("INFO:     inadequate tasks found for job %s (T: %d < %d  N: %d < %d)\n",
      J->Name,
      TotalTaskCount,
      TasksNeeded,
      TotalNodeCount,
      NodesNeeded);
    
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MNodeSelectPreemptTasks() */




/* set index to correct structure if found */
 
int MNodeFind(
 
  char     *NName, /* IN:  Node name    */
  mnode_t **NP)    /* OUT: Node pointer */
 
  {
  int nindex;
  int len;
 
  char *ptr;
 
  char QHostName[MAX_MNAME];
  char LHostName[MAX_MNAME];
  char QNetwork[MAX_MNAME];
  char LNetwork[MAX_MNAME];

  mnode_t *N;

  const char *FName = "MNodeFind";
 
  DBG(5,fSTRUCT) DPrint("%s(%s,N)\n",
    FName,
    (NName != NULL) ? NName : "NULL");
 
  if (NP != NULL)
    *NP = NULL;
 
  if ((NName == NULL) || (NName[0] == '\0'))
    return(FAILURE);

  if (!strcmp(NName,"DEFAULT"))
    {
    if (NP != NULL)
      *NP = &MSched.DefaultN;
 
    if (MSched.DefaultN.Name[0] == '\0')
      strcpy(MSched.DefaultN.Name,"DEFAULT");
 
    return(SUCCESS);
    }
 
  if ((ptr = strchr(NName,'.')) != NULL)
    {
    MUStrCpy(QHostName,NName,MIN(sizeof(QHostName),ptr - NName + 1));
    MUStrCpy(QNetwork,ptr,sizeof(QNetwork));
    }
  else
    {
    MUStrCpy(QHostName,NName,sizeof(QHostName));
    MUStrCpy(QNetwork,MSched.DefaultDomain,sizeof(QNetwork));
    } 
 
  len = strlen(QHostName);
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    /* if end of table reached */
 
    if ((N == NULL) || (N->Name[0] == '\0'))
      {
      /* return next available slot */
 
      return(FAILURE);
      }
 
    if (N->Name[0] == '\1')
      {
      continue;
      }
 
    if (strncmp(QHostName,N->Name,len))
      {
      /* partial host names must match */
 
      continue;
      }
 
    /* determine list host name */
 
    if ((ptr = strchr(N->Name,'.')) != NULL)
      {
      MUStrCpy(LHostName,N->Name,MIN(sizeof(LHostName),ptr - N->Name + 1));
      MUStrCpy(LNetwork,ptr,sizeof(LNetwork));
      }
    else
      {
      strcpy(LHostName,N->Name);
      strcpy(LNetwork,MSched.DefaultDomain);
      }
 
    if (strcmp(QHostName,LHostName))
      {
      /* host names must match */ 
 
      continue;
      }
 
    if ((QNetwork[0] != '\0') &&
        (LNetwork[0] != '\0') &&
         strcmp(QNetwork,LNetwork))
      {
      /* network names must match */
 
      continue;
      }
 
    /* host/network name matches */

    if (NP != NULL) 
      *NP = MNode[nindex];
 
    return(SUCCESS);
    }  /* END for (nindex) */
 
  DBG(1,fSTRUCT) DPrint("WARNING:  node buffer overflow in %s()\n",
    FName);
 
  DBG(1,fSTRUCT) DPrint("INFO:     ignoring node '%s'\n",
    NName);
 
  return(FAILURE);
  }  /* END MNodeFind() */





int MNodeCopy(
 
  mnode_t *SrcN,  /* I */
  mnode_t *DstN)  /* O */
 
  {
  char Name[MAX_MNAME];
  int  Index;

  const char *FName = "MNodeCopy";

  DBG(6,fSTRUCT) DPrint("%s(%s,%s)\n",
    FName,
    (SrcN != NULL) ? SrcN->Name : "NULL",
    (DstN != NULL) ? DstN->Name : "NULL");

  if ((SrcN == NULL) || (DstN == NULL))
    {
    return(FAILURE);
    }
 
  strcpy(Name,SrcN->Name);
 
  Index = DstN->Index;
 
  if (SrcN->R == NULL)
    SrcN->R = DstN->R;
 
  if (SrcN->RE == NULL)
    SrcN->RE = DstN->RE;
 
  if (SrcN->RC == NULL)
    SrcN->RC = DstN->RC;
 
  memcpy(DstN,SrcN,sizeof(mnode_t));
 
  strcpy(DstN->Name,Name);
 
  DstN->Index = Index;
 
  return(SUCCESS);
  }  /* END MNodeCopy() */





int MNodeShow(
 
  mnode_t *N)  /* I */
 
  {
  const char *FName = "MNodeShow";

  DBG(9,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    (N->Name[0] == '\0') ? "NONE" : N->Name);
 
  fprintf(mlog.logfp,"[%03d] %s: (P:%d,S:%d,M:%d,D:%d) [%s][%5s][%5s]<%lf> C:%s",
    N->Index,
    N->Name,
    N->ARes.Procs,
    N->ARes.Swap,
    N->ARes.Mem,
    N->ARes.Disk,
    MAList[eNodeState][N->State],
    MAList[eOpsys][N->ActiveOS],
    MAList[eArch][N->Arch],
    N->Load,
    MUCAListToString(N->ARes.PSlot,N->CRes.PSlot,NULL));
 
  fprintf(mlog.logfp,"%s ",
    MUListAttrs(eNetwork,N->Network));
 
  fprintf(mlog.logfp,"%s ",
    MUMAList(eFeature,N->FBM,sizeof(N->FBM)));
 
  fprintf(mlog.logfp,"%s",
    MUCAListToString(N->ARes.PSlot,N->CRes.PSlot,NULL));
 
  fprintf(mlog.logfp,"\n");
 
  return(SUCCESS);
  }  /* END MNodeShow() */



 
int MNodeShowState(

  mnode_t *N,        /* I */
  int      Flags,    /* I */
  char    *Buffer,   /* O */
  int      BufSize,  /* I */
  int      Mode)     /* I */

  {
  char *BPtr;
  int   BSpace;

  char tmpLine[MAX_MLINE];

  const char *FName = "MNodeShowState";

  DBG(5,fSTRUCT) DPrint("%s(%s,%d,Buffer,BufSize,%d)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    Flags,
    Mode);

  if ((N == NULL) || (Buffer == NULL))
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

  if (Flags & (1 << mcmParse))
    {
    /* FORMAT:      NNAME STATE OPSYS  ARCH CPROC APROC    SPEED  CPULOAD */

    MUSNPrintF(&BPtr,&BSpace,"%s=%s;%s=%s;%s=%s;%s=%s;%s=%d;%s=%d;%s=%.2lf;%s=%.2lf;",
      MWikiNodeAttr[mwnaName],
      N->Name,
      MWikiNodeAttr[mwnaState],
      MNodeState[N->State],
      MWikiNodeAttr[mwnaOS],
      MAList[eOpsys][N->ActiveOS],
      MWikiNodeAttr[mwnaArch],
      MAList[eArch][N->Arch],
      MWikiNodeAttr[mwnaCProc],
      N->CRes.Procs,
      MWikiNodeAttr[mwnaAProc],
      N->ARes.Procs,
      MWikiNodeAttr[mwnaSpeed],
      N->Speed,
      MWikiNodeAttr[mwnaCPULoad],
      N->Load);

    MUStrNCat(&BPtr,&BSpace,"\n");

    return(SUCCESS);
    }  /* END if (Flags & (1 << modeParse)) */

  MUSNPrintF(&BPtr,&BSpace,"checking node %s\n\n",
    N->Name);

  if (N != &MSched.DefaultN)
    {
    MUSNPrintF(&BPtr,&BSpace,"State:  %8s  (in current state for %s)\n",
      MNodeState[N->State],
      MULToTString((long)MSched.Time - N->StateMTime));
    }

  if (N->State != N->EState)
    {
    MUSNPrintF(&BPtr,&BSpace,"Expected State: %8s   SyncDeadline: %s",
      MAList[eNodeState][N->EState],
      MULToDString((mulong *)&N->SyncDeadLine));
    }

  /* display consumed resources */

  if (N != &MSched.DefaultN)
    {
    mcres_t URes;

    MUSNPrintF(&BPtr,&BSpace,"Configured Resources: %s\n",
      MUCResToString(&N->CRes,0,0,NULL));

    memcpy(&URes,&N->CRes,sizeof(URes));

    MCResRemove(&URes,&N->CRes,&N->ARes,1,FALSE);

    MUSNPrintF(&BPtr,&BSpace,"Utilized   Resources: %s\n",
      MUCResToString(&URes,0,0,NULL));

    MUSNPrintF(&BPtr,&BSpace,"Dedicated  Resources: %s\n",
      MUCResToString(&N->DRes,0,0,NULL));
    }  /* END BLOCK */

  MUSNPrintF(&BPtr,&BSpace,"Opsys:      %8s  Arch:      %6s\n",
    MAList[eOpsys][N->ActiveOS],
    MAList[eArch][N->Arch]);

  sprintf(tmpLine,"Speed:    %6.2lf  Load:      %6.3lf",
    N->Speed,
    (N->STTime > 0) ? N->Load : 0.0);

  if (N->MaxLoad > 0.01)
    {
    sprintf(temp_str," (MaxLoad: %.2lf)",
      N->MaxLoad);
    strcat(tmpLine,temp_str);
    }

  if (N->ExtLoad > 0.01)
    {
    sprintf(temp_str," (ExtLoad: %.2lf)",
      N->ExtLoad);
    strcat(tmpLine,temp_str);
    }

  if (N->ProcSpeed > 0)
    {
    sprintf(temp_str," (ProcSpeed: %d)",
      N->ProcSpeed);
    strcat(tmpLine,temp_str);
    }

  strcat(tmpLine,"\n");

  MUStrNCat(&BPtr,&BSpace,tmpLine);

  if (Flags & (1 << mcmVerbose))
    {
    if (N->FrameIndex >= 0)
      {
      sprintf(tmpLine,"Location:   Partition: %s  Frame/Slot:  %d/%d\n",
        (N->PtIndex >= 0) ? MAList[ePartition][N->PtIndex] : NONE,
        N->FrameIndex,
        N->SlotIndex);
      }
    else
      {
      sprintf(tmpLine,"Location:   Partition: %s  Frame/Slot:  %s\n",
        (N->PtIndex >= 0) ? MAList[ePartition][N->PtIndex] : NONE,
        "NA");
      }

    MUStrNCat(&BPtr,&BSpace,tmpLine);
    }

  if (MRM[1].Type != mrmtNONE)
    {
    MUSNPrintF(&BPtr,&BSpace,"ResourceManager[%s]: %s\n",
      N->RM->Name,
      MRMType[N->RM->Type]);
    }

  if (N->Network != 0)
    {
    char *ptr;

    ptr = MUListAttrs(eNetwork,N->Network);

    MUSNPrintF(&BPtr,&BSpace,"Network:    %s\n",
      ((ptr != NULL) && (ptr[0] != '\0')) ? ptr : "NONE");
    }  /* END if (N->Network != 0) */

  MUSNPrintF(&BPtr,&BSpace,"Features:   %s\n",
    MUMAList(eFeature,N->FBM,sizeof(N->FBM)));

  if (N->NodeType[0] != '\0')
    {
    MUSNPrintF(&BPtr,&BSpace,"NodeType:   %s\n",
      N->NodeType);
    }
  else if ((N->FrameIndex > 0) && (MFrame[N->FrameIndex].NodeType[0] != '\0'))
    {
    MUSNPrintF(&BPtr,&BSpace,"NodeType:   %s\n",
      MFrame[N->FrameIndex].NodeType);
    }

  if ((N->FrameIndex > 0) && (MSys[N->FrameIndex][N->SlotIndex].Attributes > 0))
    {
    MUSNPrintF(&BPtr,&BSpace,"Attributes: %s\n",
      MUListAttrs(eSysAttr,MSys[N->FrameIndex][N->SlotIndex].Attributes));
    }

  MUSNPrintF(&BPtr,&BSpace,"Classes:    %s\n",
    MUCAListToString(N->ARes.PSlot,N->CRes.PSlot,NULL));

  if (N->Pool[0] > 0)
    {
    int index;

    MUSNPrintF(&BPtr,&BSpace,"PoolList:   %d\n",
      (int)(N->Pool[0] - 1));

    for (index = 1;index < MAX_MPOOL;index++)
      {
      if (N->Pool[index] == '\0')
        break;

      MUSNPrintF(&BPtr,&BSpace," %d",
        (int)(N->Pool[index] - 1));
      }

    MUStrNCat(&BPtr,&BSpace,"\n");
    }    /* END if (N->Pool[0] > 0) */

  /* display node policies */

  if ((N->AP.HLimit[mptMaxJob][0] > 0) || 
      (N->MaxJobPerUser > 0) ||
      (N->MaxPEPerJob > 0.01))
    {
    MUStrNCat(&BPtr,&BSpace,"Policies:   ");

    if (N->AP.HLimit[mptMaxJob][0] > 0)
      {
      MUSNPrintF(&BPtr,&BSpace,"MAXJOB=%d  ",
        N->AP.HLimit[mptMaxJob][0]);
      }

    if (N->MaxPEPerJob > 1.0)
      {
      MUSNPrintF(&BPtr,&BSpace,"%s=%d  ",
        MNodeAttr[mnaMaxPEPerJob],
        (int)N->MaxPEPerJob);
      }
    else if (N->MaxPEPerJob > 0.01)
      {
      MUSNPrintF(&BPtr,&BSpace,"%s=%d%  ",
        MNodeAttr[mnaMaxPEPerJob],
        (int)(N->MaxPEPerJob * 100.0));
      }

    if (N->MaxJobPerUser > 0)
      {
      MUSNPrintF(&BPtr,&BSpace,"%s=%d  ",
        MNodeAttr[mnaMaxJobPerUser],
        N->MaxJobPerUser);
      }

    MUStrNCat(&BPtr,&BSpace,"\n");
    }  /* END if ((N->AP.HLimit[mptMaxJob][0] > 0) || ...) */

  /* display node statistics */

  if (N->STTime > 0)
    {
    char STotalTime[MAX_MNAME];
    char SUpTime[MAX_MNAME];
    char SBusyTime[MAX_MNAME];

    strcpy(STotalTime,MULToTString(N->STTime));
    strcpy(SUpTime,MULToTString(N->SUTime));
    strcpy(SBusyTime,MULToTString(N->SATime));

    MUSNPrintF(&BPtr,&BSpace,"\nTotal Time: %s  Up: %s (%.2f%c)  Active: %s (%.2f%c)\n",
      STotalTime,
      SUpTime,
      (double)N->SUTime / N->STTime * 100.0,
      '%',
      SBusyTime,
      (double)N->SATime / N->STTime * 100.0,
      '%');
    }  /* END if (N->STTime > 0) */

  return(SUCCESS);
  }  /* END MNodeShowState() */




int MNodeDiagnoseState(

  mnode_t *N,        /* I */
  int      Flags,    /* I */
  char    *Buffer,   /* O */
  int      BufSize,  /* I */
  int      Mode)     /* I */

  {
  char *BPtr;
  int   BSpace;

  int     jindex;
  mjob_t *J;

  int     nindex;

  int     NodeAllocated;

  long    MinStartTime;

  char    tmpLine[MAX_MLINE];

  const char *FName = "MNodeDiagnoseState";

  DBG(5,fSTRUCT) DPrint("%s(%s,%d,Buffer,BufSize,%d)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    Flags,
    Mode);

  if ((N == NULL) || (Buffer == NULL))
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

  /* verify node information is not stale */

  if (((long)MSched.Time - N->ATime) > 600)
    {
    MUSNPrintF(&BPtr,&BSpace,"ALERT:  node has not been updated in %s\n",
      MULToTString((long)MSched.Time - N->ATime));
    }

  /* display job allocation information */

  NodeAllocated = FALSE;
  MinStartTime  = MAX_MTIME;

  tmpLine[0] = '\0';

  for (jindex = 0;MAQ[jindex] != -1;jindex++)
    {
    J = MJob[MAQ[jindex]];

    for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
      {
      if (J->NodeList[nindex].N == N)
        {
        if (tmpLine[0] != '\0')
          strcat(tmpLine,",");

        strcat(tmpLine,J->Name);
        
        NodeAllocated = TRUE;

        MinStartTime = MIN(MinStartTime,J->StartTime);
        }
      }
    }       /* END for (jindex = 0;MAQ[jindex] != -1;jindex++) */

  if (NodeAllocated == TRUE)
    {
    MUSNPrintF(&BPtr,&BSpace,"JobList:  %s\n",
      tmpLine);
    }

  /* evaluate node state */

  if ((MNODEISACTIVE(N) == TRUE) && (NodeAllocated == FALSE))
    {
    MUSNPrintF(&BPtr,&BSpace,"ALERT:  no jobs active on node for %s but state is %s\n",
      MULToTString((long)MSched.Time - MinStartTime),
      MNodeState[N->State]);
    }
  else if ((MNODEISACTIVE(N) == FALSE) && (NodeAllocated == TRUE)) 
    {
    MUSNPrintF(&BPtr,&BSpace,"ALERT:  jobs active on node but state is %s\n",
      MNodeState[N->State]);
    }

  /* evaluate utilized resources */

  if (MNODEISACTIVE(N) == TRUE)
    {
    /* check low thresholds */

    if (N->State == mnsBusy)
      {
      if ((N->Load - N->ExtLoad) / N->CRes.Procs < 0.2)
        {
        MUSNPrintF(&BPtr,&BSpace,"ALERT:  node is in state %s but load is low (%0.3f)\n",
          MNodeState[N->State],
          N->Load);
        }
      }
    else if (N->DRes.Procs > 0)
      {
      if ((N->Load - N->ExtLoad) / N->DRes.Procs < 0.2)
        {
        MUSNPrintF(&BPtr,&BSpace,"ALERT:  node has %d procs dedicated but load is low (%0.3f)\n",
          N->DRes.Procs,
          N->Load);
        }
      }

    /* check high thresholds */

    /* NYI */
    }
  else
    {
    /* check high thresholds */

    if ((N->Load / N->CRes.Procs) > 0.5)
      {
      MUSNPrintF(&BPtr,&BSpace,"ALERT:  node is in state %s but load is high (%0.3f)\n",
        MNodeState[N->State],
        N->Load);
      }
    }

  /* evaluate configured resources */

  if ((N->FrameIndex != -1) &&
      (MFrame[N->FrameIndex].Memory > 0) &&
      (N->CRes.Mem != MFrame[N->FrameIndex].Memory))
    {
    MUSNPrintF(&BPtr,&BSpace,"ALERT:  node memory does not match expected memory (%d != %d)\n",
      N->CRes.Mem,
      MFrame[N->FrameIndex].Memory);
    }

  if (N->Message != NULL)
    {
    MUSNPrintF(&BPtr,&BSpace,"NOTE:    node message '%s'\n",
      N->Message);
    }  /* END if (N->Message != NULL) */

  return(SUCCESS);
  }  /* END MNodeDiagnoseState() */




int MNodeShowReservations(

  mnode_t *N,
  int      Flags,
  char    *Buffer,   /* O */
  int      BufSize,
  int      Mode)     /* I */

  {
  char *BPtr;
  int   BSpace;

  int   ResFound;
  int   Header;

  char  StartTime[MAX_MNAME];
  char  EndTime[MAX_MNAME];
  char  Duration[MAX_MNAME];

  int   rindex;
  int   reindex;

  int   RC;

  mres_t  *R;
  mcres_t *BR;

  const char *FName = "MNodeShowReservations";

  DBG(5,fSTRUCT) DPrint("%s(%s,%d,Buffer,BufSize,%d)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    Flags,
    Mode);

  if ((N == NULL) || (Buffer == NULL))
    return(FAILURE);

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

  /* display node reservation information */

  ResFound = FALSE;

  MUStrNCat(&BPtr,&BSpace,"\nReservations:\n");

  for (rindex = 0;rindex < MSched.ResDepth;rindex++)
    {
    if (N->R[rindex] == NULL)
      break;

    if (N->R[rindex] == (mres_t *)1)
      continue;

    Header = FALSE;

    for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
      {
      if (N->RE[reindex].Type == mreNONE)
        break;

      if (N->RE[reindex].Type != mreStart)
        continue;

      if (N->RE[reindex].Index != rindex)
        continue;

      ResFound = TRUE;

      R = N->R[N->RE[reindex].Index];

      BR = &N->RE[reindex].DRes;
      RC = N->RC[N->RE[reindex].Index];

      if (Header == FALSE)
        {
        strcpy(StartTime,MULToTString(R->StartTime - MSched.Time));
        strcpy(EndTime,  MULToTString(R->EndTime - MSched.Time));
        strcpy(Duration, MULToTString(R->EndTime - R->StartTime));

        MUSNPrintF(&BPtr,&BSpace,"  %s '%s'(x%d)  %s -> %s (%s)\n",
          MResType[R->Type],
          R->Name,
          RC,
          StartTime,
          EndTime,
          Duration);

        Header = TRUE;
        }

      if (R->Type == mrtUser)
        {
        MUSNPrintF(&BPtr,&BSpace,"    Blocked Resources@%-11s %s\n",
          MULToTString(N->RE[reindex].Time - MSched.Time),
          MUCResRatioToString(&R->DRes,BR,&N->CRes,1));
        }  /* END if (R->Type)  */
      }    /* END for (reindex) */
    }      /* END for (rindex)  */

  if (ResFound == FALSE)
    {
    MUStrNCat(&BPtr,&BSpace,"NOTE:  no reservations on node\n");
    }

  return(SUCCESS);
  }  /* END MNodeShowReservations() */




int MNodeDiagnoseReservations(

  mnode_t *N,
  int      Flags,
  char    *Buffer,
  int      BufSize,
  int      Mode)     /* I */

  {
  char *BPtr;
  int   BSpace;

  int   rindex;
  int   reindex;

  int   ErrorDetected;

  int   RCount[MAX_MRES_DEPTH];
  int   RIndex;

  const char *FName = "MNodeDiagnoseReservations";

  DBG(5,fSTRUCT) DPrint("%s(%s,%d,Buffer,BufSize,%d)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    Flags,
    Mode);

  if ((N == NULL) || (Buffer == NULL))
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

  /* evaluate node reservation information */

  memset(RCount,0,sizeof(RCount));

  ErrorDetected = FALSE;

  for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
    {
    /* look for out-of-order events */

    if (N->RE[reindex].Type == mreNONE)
      break;

    RIndex = N->RE[reindex].Index;

    if (N->RE[reindex].Type == mreStart)
      RCount[RIndex]++;
    else if (N->RE[reindex].Type == mreEnd)
      RCount[RIndex]--;

    if ((RCount[RIndex] < 0) || (RCount[RIndex] > 1))
      {
      ErrorDetected = TRUE;

      break;
      }
    }  /* END for (reindex) */

  if (ErrorDetected == FALSE)
    {
    /* look for unmatched events */

    for (rindex = 0;rindex < MSched.ResDepth;rindex++)
      {
      if (RCount[rindex] != 0)
        {
        ErrorDetected = TRUE;

        break;
        }
      }  /* END for (rindex) */
    }    /* END if (ErrorDetected == FALSE) */

  if (ErrorDetected == TRUE)
    {
    /* display full table information */

    MUStrNCat(&BPtr,&BSpace,"ALERT:  RE table is corrupt\n");

    memset(RCount,0,sizeof(RCount));

    for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
      {
      if (N->RE[reindex].Type == mreNONE)
        break;

      RIndex = N->RE[reindex].Index;

      MUSNPrintF(&BPtr,&BSpace,"  RE[%d] %c %d\n",
        reindex,
        (N->RE[reindex].Type == mreStart) ? 'S' : 'E',
        RIndex);

      if (N->RE[reindex].Type == mreStart)
        RCount[RIndex]++;
      else if (N->RE[reindex].Type == mreEnd)
        RCount[RIndex]--;

      if ((RCount[RIndex] < 0) || 
          (RCount[RIndex] > 1))
        {
        MUSNPrintF(&BPtr,&BSpace,"ALERT:  RE[%d] %s\n",
          reindex,
          (RCount[RIndex] < 0) ?
            "reservation end event located without accompanying start event" :
            "reservation start event located without accompanying end event");

        RCount[RIndex] = (RCount[RIndex] < 0) ? 0 : 1;
        }
      }   /* END for (reindex)            */

    for (rindex = 0;rindex < MSched.ResDepth;rindex++)
      {
      if (RCount[rindex] != 0)
        {
        MUSNPrintF(&BPtr,&BSpace,"ALERT:  R[%d] %s\n",
          rindex,
          "reservation does not have end event");
        }
      }   /* END for (rindex) */
    }     /* END if (Corruption == TRUE) */

  /* evaluate reserved resources */

  {
  long    ETime;

  mcres_t NDRes;    /* non-dedicated resources */

  int     TC;

  mjob_t *J;
  mreq_t *RQ;

  memcpy(&NDRes,&N->CRes,sizeof(NDRes));

  ETime = 0;

  for (reindex = 0;reindex < (MSched.ResDepth << 1);reindex++)
    {
    if (N->RE[reindex].Type == mreNONE)
      break;

    TC = N->RC[N->RE[reindex].Index];

    if (N->RE[reindex].Time > ETime)
      {
      if (MUCResIsNeg(&NDRes) == SUCCESS)
        {
        MUSNPrintF(&BPtr,&BSpace,"ALERT:  node is overcommitted at time %s (P: %d)\n",
          MULToTString(ETime - MSched.Time),
          NDRes.Procs);
        }

      ETime = N->RE[reindex].Time;
      }

    if (N->RE[reindex].Type == mreStart)
      {
      if (N->R[N->RE[reindex].Index]->Type == mrtJob)
        {
        J = (mjob_t *)N->R[N->RE[reindex].Index]->J;

        RQ = J->Req[0]; /* FIXME */

        MCResRemove(&NDRes,&N->CRes,&RQ->DRes,TC,FALSE);
        }
      else
        {
        MCResRemove(&NDRes,&N->CRes,&N->RE[reindex].DRes,1,FALSE);
        }
      }
    else if (N->RE[reindex].Type == mreEnd)
      {
      if (N->R[N->RE[reindex].Index]->Type == mrtJob)
        {
        J = (mjob_t *)N->R[N->RE[reindex].Index]->J;

        RQ = J->Req[0];

        MCResAdd(&NDRes,&N->CRes,&RQ->DRes,TC,FALSE);
        }
      else
        {
        MCResAdd(&NDRes,&N->CRes,&N->RE[reindex].DRes,1,FALSE);
        }
      }
    }    /* END for (reindex) */
  }      /* END BLOCK */

  return(SUCCESS);
  }  /* END MNodeDiagnoseReservations() */




 
int MParAddNode(
 
  mpar_t  *P,  /* I */
  mnode_t *N)  /* I (modified) */
 
  {
  if ((P == NULL) || (N == NULL)) 
    {
    return(FAILURE);
    }
 
  N->PtIndex = P->Index;
 
  return(SUCCESS);
  }  /* END MParAddNode() */




int MNodeCheckPolicies(

  mjob_t  *J,         /* I */
  mnode_t *N,         /* I */
  long     StartTime, /* I */
  int     *TC)        /* I/O (optional) */

  {
  int     TotalJobCount;
  int     UserJobCount;
  int     TotalProcCount;
  int     UserProcCount;

  int     PCDelta;
  int     JPC;

  int     jindex;

  int     MJC;
  int     MUJC;
  int     MPC;
  int     MUPC;

  int     tindex;

  mjob_t *tmpJ;

  const char *FName = "MNodeCheckPolicies";

  DBG(6,fSTRUCT) DPrint("%s(%s,%s,%d)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (N != NULL) ? N->Name : "NULL",
    (TC != NULL) ? *TC : -1);

  if ((J == NULL) || (N == NULL))
    {
    return(FAILURE);
    }

  if (N->AP.HLimit[mptMaxJob][0] <= 0)
    MJC = MSched.DefaultN.AP.HLimit[mptMaxJob][0];
  else
    MJC = N->AP.HLimit[mptMaxJob][0];
 
  if (N->MaxJobPerUser <= 0)
    MUJC = MSched.DefaultN.MaxJobPerUser;
  else
    MUJC = N->MaxJobPerUser;

  if (N->AP.HLimit[mptMaxProc][0] <= 0)
    {
    MPC = MSched.DefaultN.AP.HLimit[mptMaxProc][0];
    }
  else
    {
    MPC = N->AP.HLimit[mptMaxProc][0];
    }

  if (N->MaxProcPerUser <= 0)
    MUPC = MSched.DefaultN.MaxProcPerUser;
  else
    MUPC = N->MaxProcPerUser;

  JPC = J->Req[0]->DRes.Procs;

  if (((MJC > 0) || (MUJC > 0) || (MPC > 0) || (MUPC > 0)) && (StartTime <= MSched.Time + 3600))
    {
    TotalJobCount  = JPC;
    TotalProcCount = 1;

    UserJobCount   = JPC;
    UserProcCount  = 1;

    for (jindex = 0;jindex < N->MaxJCount;jindex++)
      {
      if (N->JList[jindex] == NULL)
        break;
                                                                                
      if (N->JList[jindex] == (mjob_t *)1)
        continue;
 
      tmpJ = (mjob_t *)N->JList[jindex];

      if ((tmpJ->State != mjsStarting) &&
          (tmpJ->State != mjsRunning))
        {
        continue;
        }

      TotalJobCount++;

      if (tmpJ->Cred.U == J->Cred.U)
        UserJobCount++;

      for (tindex = 0;tmpJ->TaskMap[tindex] != -1;tindex++)
        {
        if (tmpJ->TaskMap[tindex] != N->Index)
          continue;

        /* FIXME:  must adjust for multi-req jobs */

        TotalProcCount += tmpJ->Req[0]->DRes.Procs;

        if (tmpJ->Cred.U == J->Cred.U)
          UserProcCount += tmpJ->Req[0]->DRes.Procs;
        }  /* END for (tindex) */
      }    /* END for (jindex) */

    if ((MJC > 0) && (TotalJobCount > MJC))
      {
      DBG(6,fSTRUCT) DPrint("INFO:     job violates MaxJobPerNode policy on node %s\n",
        N->Name);

      return(FAILURE);
      }

    if ((MUJC > 0) && (UserJobCount >= MUJC))
      {
      DBG(6,fSTRUCT) DPrint("INFO:     job violates MaxJobPerUser policy on node %s\n",
        N->Name);

      return(FAILURE);
      }

    if (MUPC > 0)
      {
      if (UserProcCount > MUPC)
        {
        DBG(6,fSTRUCT) DPrint("INFO:     job violates MaxProcPerUser policy on node %s\n",
          N->Name);

        return(FAILURE);
        }

      if ((TC != NULL) && (JPC > 0))
        {
        PCDelta = MUPC - UserProcCount;

        *TC = MIN(*TC,1 + PCDelta / JPC);
        }
      }  /* END if (MUPC > 0) */

    if (MPC > 0)
      {
      if (TotalProcCount > MPC)
        {
        DBG(6,fSTRUCT) DPrint("INFO:     job violates MaxProc policy on node %s\n",
          N->Name);

        return(FAILURE);
        }

      if ((TC != NULL) && (JPC > 0))
        {
        PCDelta = MPC - TotalProcCount;

        *TC = MIN(*TC,1 + PCDelta / JPC);
        }
      }  /* END if (MPC > 0) */
    }    /* END if ((MJC > 0) || (MUJC > 0)) */

  if (TC != NULL)
    {
    double MaxPEPerJob;

    /* check node task policies */

    if (!(J->Flags & (1 << mjfNASingleJob)))
      {
      if (N->MaxPEPerJob > 0.0)
        MaxPEPerJob = N->MaxPEPerJob;
      else
        MaxPEPerJob = MSched.DefaultN.MaxPEPerJob;

      if (MaxPEPerJob > 0.0)
        {
        int PE;
        int tmpI;

        /* NOTE:  only process primary req */

        tmpI = *TC;

        PE = __MNodeGetPEPerTask(&N->CRes,&J->Req[0]->DRes);

        if (MaxPEPerJob > 1.0)
          *TC = MIN(MaxPEPerJob,tmpI * PE);
        else
          *TC = MIN(MaxPEPerJob * N->CRes.Procs,tmpI * PE);
        }  /* END if (MaxPEPerJob) */
      }    /* END if (!(J->Flags & (1 << mjfDedicated))) */

    if (*TC <= 0)
      {
      DBG(6,fSTRUCT) DPrint("INFO:     inadequate tasks on node %s in %s\n",
        N->Name,
        FName);

      return(FAILURE);
      }
    }  /* END if (TC != NULL) */

  return(SUCCESS);
  }  /* END MNodeCheckPolicies() */




int __MNodeGetPEPerTask(

  mcres_t *Cfg,  /* I */
  mcres_t *Req)  /* I */

  {
  double PE;

  PE = (double)Req->Procs / Cfg->Procs;

  if ((Req->Mem > 0) && (Cfg->Mem > 0))
    PE = MAX(PE,(double)Req->Mem  / Cfg->Mem);

  if ((Req->Disk > 0) && (Cfg->Disk > 0))
    PE = MAX(PE,(double)Req->Disk / Cfg->Disk);

  if ((Req->Swap > 0) && (Cfg->Swap > 0))
    PE = MAX(PE,(double)Req->Swap / Cfg->Swap);

  PE *= Cfg->Procs;

  return((int)PE);
  }  /* END MNodeGetPE() */





int MNodeSetClass(

  mnode_t  *N,           /* I (modified) */
  mclass_t *C,           /* I (optional) */
  char     *ClassString, /* I (optional) */
  int       Mode)        /* I (mClear/mAdd/mSet) */

  {
  char *ptr = NULL;
  char *ptr2 = NULL;

  char *TokPtr;

  int   cindex;
  int   acount;
  int   ccount;

  int   oindex;

  if (N == NULL)
    {
    return(FAILURE);
    }

  /* format:  <CLASS>[:<COUNT>][+<CLASS>[:<COUNT>]]... */

  if ((Mode == mClear) || (Mode == mSet))
    {
    memset(N->ARes.PSlot,0,sizeof(N->ARes.PSlot));
    memset(N->CRes.PSlot,0,sizeof(N->CRes.PSlot));
    }

  if (ClassString != NULL)
    {
    ptr = MUStrTok(ClassString,"+",&TokPtr);
    }
  else
    {
    ptr = NULL;
    }

  while ((C != NULL) && (ptr != NULL))
    {
    if (C == NULL)
      {
      if ((ptr2 = strchr(ptr,':')) != NULL)
        {
        ccount = (int)strtol(ptr2 + 1,NULL,0);
        acount = MIN(ccount,N->ARes.Procs);

        *ptr2 = '\0';
        }
      else
        {
        acount = N->ARes.Procs;
        ccount = N->CRes.Procs;
        }
   
      cindex = MUMAGetIndex(eClass,ptr,mAdd);
      }
    else
      {
      acount = (N->ARes.Procs >= 0) ? N->ARes.Procs : N->CRes.Procs;
      ccount = N->CRes.Procs;

      cindex = C->Index;
      }

    switch(Mode)
      {
      case mAdd:
      case mSet:

        /* clear old class info */           

        oindex = N->CRes.PSlot[cindex].count;
 
        if (oindex > 0)
          {
          N->CRes.PSlot[0].count = MAX(0,N->CRes.PSlot[0].count - oindex);
          N->CRes.PSlot[cindex].count = 0;
          }
   
        oindex = N->ARes.PSlot[cindex].count;

        if (oindex > 0)
          {
          N->ARes.PSlot[0].count = MAX(0,N->ARes.PSlot[0].count - oindex);
          N->ARes.PSlot[cindex].count = 0;
          }
 
        /* add new class info */

        N->CRes.PSlot[cindex].count = ccount;
        N->CRes.PSlot[0].count += ccount;

        N->ARes.PSlot[cindex].count = acount;
        N->ARes.PSlot[0].count += acount;

        break;

      default:

        /* NO-OP */

        break;
      }  /* END switch(Mode) */

    if (C != NULL)
      break;

    ptr = MUStrTok(NULL,"+",&TokPtr);
    }    /* END while (ptr != NULL) */

  return(SUCCESS);
  }  /* END MNodeSetClass() */




char *MNodeAdjustName(
 
  char *SpecName,    /* I */
  int   AdjustMode)  /* I: 0: short  1: full */
 
  {
  static char  Line[MAX_MNAME];
  char        *ptr;

  Line[0] = '\0';
 
  switch (AdjustMode)
    {
    case 0:
    default:
 
      /* get short name */
 
      if ((ptr = strchr(SpecName,'.')) != NULL)
        {
        MUStrCpy(Line,SpecName,ptr - SpecName + 1);
 
        if (MSched.DefaultDomain[0] == '\0')
          MUStrCpy(MSched.DefaultDomain,ptr,sizeof(MSched.DefaultDomain)); 
 
        return(Line);
        }
      else
        {
        return(SpecName);
        }
 
      /*NOTREACHED*/
 
      break;
 
    case 1:
 
      /* get long name */
 
      if ((ptr = strchr(SpecName,'.')) != NULL)
        {
        return(SpecName);
        }
      else
        {
        if (MSched.DefaultDomain[0] != '\0')
          {
          if (MSched.DefaultDomain[0] == '.')
            {
            sprintf(Line,"%s%s",
              SpecName,
              MSched.DefaultDomain);
            }
          else
            {
            sprintf(Line,"%s.%s",
              SpecName,
              MSched.DefaultDomain);
            }
 
          return(Line);
          }
        else
          {
          return(SpecName);
          }
        }
 
      /*NOTREACHED*/
 
      break;
    }  /* END switch(AdjustMode) */

  return(Line);
  }    /* MNodeAdjustName() */




int MNodeTrap(
 
  mnode_t *N) /* I */
 
  {
  /* NO-OP */

  return(SUCCESS);
  }  /* END MNodeTrap() */




int MNodeSetAttr(

  mnode_t  *N,      /* I * (modified) */
  enum MNodeAttrEnum AIndex, /* I */
  void    **Value,  /* I */
  int       Format, /* I */
  int       Mode)   /* I */

  {
  if (N == NULL)
    {
    return(FAILURE);
    }

  switch(AIndex)
    {
    case mnaExtLoad:

      {
      double tmpD = 0.0;

      if (Value != NULL)
        {
        if (Format == mdfDouble)
          tmpD = *(double *)Value;
        else
          tmpD = strtod((char *)Value,NULL);
        }

      N->ExtLoad = tmpD;
      }  /* END BLOCK */

      break;

    case mnaFrame:

      {
      int tmpI;

      if (Format == mdfInt)
        {
        tmpI = *(int *)Value;
        }
      else
        {
	sscanf((char *)Value,"%d",
          &tmpI);
        }

      MFrameAddNode(&MFrame[tmpI],N,-1);
      }  /* END BLOCK */

      break;

    case mnaGRes:

      {
      int   RIndex;

      char *TokPtr;
      char *Name;
      char *Count;

      /* FORMAT:  <RESNAME>[:<RESCOUNT>] */
 
      Name = MUStrTok((char *)Value,":",&TokPtr);

      if ((RIndex = MUMAGetIndex(eGRes,Name,mAdd)) == 0)
        {
        break;
        }
 
      if ((Count = MUStrTok(NULL,":",&TokPtr)) != NULL)
        {
        N->CRes.GRes[RIndex].count = (int)strtol(Count,NULL,0);
        }
      else
        {
        N->CRes.GRes[RIndex].count = 9999;
        }
      
      N->CRes.GRes[0].count += N->CRes.GRes[RIndex].count;
      }  /* END BLOCK */

      break;

    case mnaLoad:

      {
      double tmpD = 0.0;

      if (Value != NULL)
        {
        if (Format == mdfDouble)
          tmpD = *(double *)Value;
        else
          tmpD = strtod((char *)Value,NULL);
        }

      N->Load = tmpD;
      }  /* END BLOCK */

      break;

    case mnaMaxPEPerJob:

      {
      if (Format == mdfString)
        {
        /* FORMAT:  <VAL>% | <VAL> */

        if (strchr((char *)Value,'%'))
          N->MaxPEPerJob = strtod((char *)Value,NULL) / 100.0;
        else
          N->MaxPEPerJob = strtod((char *)Value,NULL);
        }
      else
        {
        return(FAILURE);
        }
      }  /* END BLOCK */

      break;

    case mnaMaxProcPerClass:

      {
      int CCount;

      mclass_t *C;

      if (Format != mdfOther)
        break;

      /* extract class index/count */

      C      = (mclass_t *)((mnalloc_t *)Value)->N;
      CCount = ((mnalloc_t *)Value)->TC;

      if (C->OCDProcFactor > 0.0)
        {
        CCount *= C->OCDProcFactor;
        }

      if (C->MaxProcPerNode > 0)
        {
        CCount = MIN(CCount,C->MaxProcPerNode);
        }

      if (N->CRes.PSlot[C->Index].count > 0)
        N->CRes.PSlot[0].count -= N->CRes.PSlot[C->Index].count;

      N->CRes.PSlot[C->Index].count  = CCount;

      N->CRes.PSlot[0].count        += CCount;
      }  /* END BLOCK */

      break;

    case mnaNetwork:

      {
      char *ptr;
      char *TokPtr;

      N->Network = 0;

      ptr = MUStrTok((char *)Value,",: \t\n",&TokPtr);

      while (ptr != NULL)
        {
        N->Network |= MUMAGetBM(eNetwork,ptr,mAdd);

        ptr = MUStrTok(NULL,",: \t\n",&TokPtr);
        }  /* END while (ptr != NULL) */
      }  /* END BLOCK */

      break;
 
    case mnaNodeID:

      MUStrCpy(N->Name,(char *)Value,sizeof(N->Name));

      break;

    case mnaNodeState:

      if (Format == mdfString)
        N->State = MUGetIndex((char *)Value,MNodeState,FALSE,0);
      else
        N->State = *(int *)Value;

      N->EState = N->State;

      break;

    case mnaPriority:

      if (Format == mdfString)
        N->Priority = (int)strtol((char *)Value,NULL,0);
      else if (Format == mdfInt)
        N->Priority = (long)*(int *)Value;
      else
        N->Priority = *(long *)Value;
 
      break;

    case mnaRADisk:

      if (Format == mdfString)
        N->ARes.Disk = (int)MURSpecToL((char *)Value,mvmMega,mvmMega);
      else
        N->ARes.Disk = *(long *)Value;

      N->CRes.Disk = MAX(N->CRes.Disk,N->ARes.Disk);

      break;

    case mnaRAMem:
 
      if (Format == mdfString)
        N->ARes.Mem = (int)MURSpecToL((char *)Value,mvmMega,mvmMega);
      else
        N->ARes.Mem = *(long *)Value;

      N->CRes.Mem = MAX(N->CRes.Mem,N->ARes.Mem);

      break;

    case mnaRAProc:
 
      if (Format == mdfString)
        N->ARes.Procs = (int)strtol((char *)Value,NULL,0);
      else
        N->ARes.Procs = *(long *)Value;

      N->CRes.Procs = MAX(N->CRes.Procs,N->ARes.Procs);

      break;

    case mnaRASwap:
 
      if (Format == mdfString)
        N->ARes.Swap = (int)MURSpecToL((char *)Value,mvmMega,mvmMega);
      else
        N->ARes.Swap = *(long *)Value;

      N->CRes.Swap = MAX(N->CRes.Swap,N->ARes.Swap);

      break;

    case mnaRCDisk:
 
      if (Format == mdfString)
        N->CRes.Disk = (int)MURSpecToL((char *)Value,mvmMega,mvmMega);
      else
        N->CRes.Disk = *(long *)Value;

      break;
 
    case mnaRCMem:
 
      if (Format == mdfString)
        N->CRes.Mem = (int)MURSpecToL((char *)Value,mvmMega,mvmMega);
      else
        N->CRes.Mem = *(long *)Value;

      break;
 
    case mnaRCProc:
 
      if (Format == mdfString)
        N->CRes.Procs = (int)strtol((char *)Value,NULL,0);
      else
        N->CRes.Procs = *(long *)Value;

      break;
 
    case mnaRCSwap:
 
      if (Format == mdfString)
        N->CRes.Swap = (int)MURSpecToL((char *)Value,mvmMega,mvmMega);
      else
        N->CRes.Swap = *(long *)Value;

      break;

    case mnaSlot:

      {
      int tmpI;

      if (Format == mdfInt)
        tmpI = *(int *)Value;
      else
        tmpI = -1;

      if (N->FrameIndex >= 0)
        MFrameAddNode(&MFrame[N->FrameIndex],N,tmpI);
      else
        N->SlotIndex = tmpI;
      }  /* END BLOCK */

      break;

    case mnaStatATime:

      if (Format == mdfString)
        sscanf((char *)Value,"%lu",
          &N->SATime);
      else
        N->SATime = *(long *)Value;

      break;

    case mnaStatTTime:

      if (Format == mdfString)
        sscanf((char *)Value,"%lu",
          &N->STTime);
      else
        N->STTime = *(long *)Value;

      break;

    case mnaStatUTime:

      if (Format == mdfString)
        sscanf((char *)Value,"%lu",
          &N->SUTime);
      else
        N->SUTime = *(long *)Value;

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MNodeSetAttr() */




int MNodeSetState(

  mnode_t *N,       /* I */
  int      NState,  /* I */
  int      Mode)    /* I */

  {
  const char *FName = "MNodeSetState";

  DBG(7,fSTRUCT) DPrint("%s(%s,%s,%d)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    MNodeState[NState],
    Mode);

  if (N == NULL)
    {
    return(FAILURE);
    }

  if (N->State != NState)
    {
    N->StateMTime = MSched.Time;
    N->MTime      = MSched.Time;

    if (N->State == mnsDown)
      {
      switch(NState)
        {
        case mnsIdle:
        case mnsActive:
        case mnsBusy:

          /* incorporate into standing reservations */

          /* NYI */

          break;
        }  /* END switch(NState) */
      }    /* END if (N->State == mnsDown) */
    }      /* END if (N->State != NState) */

  N->State  = NState;
  N->EState = NState;

  return(SUCCESS);
  }  /* END MNodeSetState() */




int MNodeAToString( 
 
  mnode_t *N,      /* I */
  int      AIndex, /* I */
  char    *Buf,    /* O */
  int      Mode)   /* I */

  {
  if (Buf == NULL)
    {
    return(FAILURE);
    }
 
  Buf[0] = '\0';
 
  if (N == NULL)
    {
    return(FAILURE);
    }
 
  switch(AIndex)
    {
    case mnaCfgDisk:

      sprintf(Buf,"%d",
        N->CRes.Disk);

      break;

    case mnaCfgMem:

      sprintf(Buf,"%d",
        N->CRes.Mem);

      break;

    case mnaMaxLoad:

      if ((Mode & (1 << mcmVerbose)) || (N->MaxLoad > 0.0))
        {
        sprintf(Buf,"%lf",
          N->MaxLoad);
        }

      break;

    case mnaMaxPEPerJob:

      if ((Mode & (1 << mcmVerbose)) || (N->MaxPEPerJob > 0.0))
        {
        sprintf(Buf,"%lf",
          N->MaxPEPerJob);
        }

      break;

    case mnaPrioF:

      MNodePrioFToString(N,Buf);

      break;

    case mnaPriority:

      if ((Mode & (1 << mcmVerbose)) || (N->Priority != 0))
        {
        sprintf(Buf,"%d",
          N->Priority);
        }

      break;

    case mnaRADisk:

      sprintf(Buf,"%d",
        N->ARes.Disk);

      break;

    case mnaRAMem:
 
      sprintf(Buf,"%d",
        N->ARes.Mem);
 
      break;

    case mnaRAProc:
 
      sprintf(Buf,"%d",
        N->ARes.Procs);
 
      break;

    case mnaRASwap:
 
      sprintf(Buf,"%d",
        N->ARes.Swap);
 
      break;

    case mnaRCDisk:
 
      sprintf(Buf,"%d",
        N->CRes.Disk);
 
      break;
 
    case mnaRCMem:
 
      sprintf(Buf,"%d",
        N->CRes.Mem);
 
      break;
 
    case mnaRCProc:
 
      sprintf(Buf,"%d",
        N->CRes.Procs);
 
      break;
 
    case mnaRCSwap:
 
      sprintf(Buf,"%d",
        N->CRes.Swap);
 
      break;

    case mnaStatATime:

      sprintf(Buf,"%ld",
        N->SATime);
 
      break;

    case mnaStatTTime:
 
      sprintf(Buf,"%ld",
        N->STTime);
 
      break;

    case mnaStatUTime:
 
      sprintf(Buf,"%ld",
        N->SUTime);
 
      break;

    case mnaNetwork:
    
      strcpy(Buf,MUListAttrs(eNetwork,N->Network));

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MNodeAToString() */





int MNodeInitialize(

  mnode_t *N,    /* I (modified) */
  char    *Name) /* I (optional) */

  {
  if (N == NULL)
    {
    return(FAILURE);
    }

  memset(N,0,sizeof(mnode_t));

  if (Name != NULL)
    strcpy(N->Name,Name);

  N->FrameIndex = -1;
  N->SlotIndex  = -1;

  return(SUCCESS);
  }  /* END MNodeInitialize() */




int MNodeGetTC(

  mnode_t *N,    /* I (optional) */
  mcres_t *Avl,  /* I */
  mcres_t *Cfg,  /* I */
  mcres_t *Ded,  /* I */
  mcres_t *Req,  /* I */
  long     Time) /* I */

  {
  int TC;
  int tmpTC;
  int tmpRes;
 
  int UseUtil = FALSE;
  int UseDed  = FALSE;

  int ARes[MAX_MRESOURCETYPE];
  int DRes[MAX_MRESOURCETYPE];
  int CRes[MAX_MRESOURCETYPE];
  int RRes[MAX_MRESOURCETYPE];

  int rindex;
  int pindex;

  const char *FName = "MNodeGetTC";

  DBG(8,fSCHED) DPrint("%s(%s,%d,%d,%d,%d,%ld)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    Avl->Procs,
    Cfg->Procs,
    Ded->Procs,
    Req->Procs,
    Time - MSched.Time);

  /* NOTE:  NULL node allowed */

  TC = 99999;
 
  RRes[mrProc] = Req->Procs;
  CRes[mrProc] = Cfg->Procs;
  DRes[mrProc] = Ded->Procs;
  ARes[mrProc] = Avl->Procs;

  RRes[mrMem] = Req->Mem;
  CRes[mrMem] = Cfg->Mem;
  DRes[mrMem] = Ded->Mem;
  ARes[mrMem] = Avl->Mem;

  RRes[mrSwap] = Req->Swap;
  CRes[mrSwap] = Cfg->Swap;
  DRes[mrSwap] = Ded->Swap;
  ARes[mrSwap] = Avl->Swap;

  RRes[mrDisk] = Req->Disk;
  CRes[mrDisk] = Cfg->Disk;
  DRes[mrDisk] = Ded->Disk;
  ARes[mrDisk] = Avl->Disk;

  for (rindex = mrProc;rindex <= mrDisk;rindex++)
    {
    if (RRes[rindex] == 0)
      continue;

    if (CRes[rindex] < RRes[rindex])
      {
      DBG(8,fSCHED) DPrint("INFO:     inadequate %s (C:%d < R:%d)\n",
        MResourceType[rindex],
        CRes[rindex],
        RRes[rindex]);
 
      return(0);
      }

    /* incorporate availability policy information */

    /* determine effective specific/default policy */

    {
    char pval;

    UseUtil = FALSE;
    UseDed  = FALSE;

    pval = mrapNONE;

    if ((N != NULL) && (N->NAvailPolicy != NULL))
      {
      pval = (N->NAvailPolicy[rindex] != mrapNONE) ?
        N->NAvailPolicy[rindex] : N->NAvailPolicy[0];
      }

    if (pval == mrapNONE)
      {
      pval = (MPar[0].NAvailPolicy[rindex] != mrapNONE) ?
        MPar[0].NAvailPolicy[rindex] : MPar[0].NAvailPolicy[0];
      }

    /* determine policy impact */

    if ((pval == mrapUtilized) || (pval == mrapCombined))
      {
      if (Time <= MSched.Time)
        UseUtil = TRUE;
      }

    if ((pval == mrapDedicated) || (pval == mrapCombined))
      {
      if (Time <= MSched.Time)
        UseDed = TRUE;
      }
    }    /* END BLOCK */

    if (MPar[0].NAvailPolicy[rindex] != 0)
      pindex = rindex;
    else
      pindex = 0;

    if (RRes[rindex] == -1)
      {
      if (CRes[rindex] == 0)
        {
        DBG(8,fSCHED) DPrint("INFO:     cannot dedicate %s (C:%d == 0)\n",
          MResourceType[rindex],
          CRes[rindex]);

        return(0);
        }

      if ((UseUtil == TRUE) && (ARes[rindex] != CRes[rindex]))
        {
        DBG(8,fSCHED) DPrint("INFO:     cannot dedicate %s (A:%d != C:%d)\n",
          MResourceType[rindex],
          ARes[rindex],
          CRes[rindex]);

        return(0);
        }

      if ((UseDed == TRUE) && (DRes[rindex] > 0))
        {
        DBG(8,fSCHED) DPrint("INFO:     cannot dedicate %s (D:%d != 0)\n",
          MResourceType[rindex],
          DRes[rindex]);

        return(0);
        }

      TC = MIN(1,CRes[rindex]);
      }
    else
      {
      tmpRes = CRes[rindex];

      if (UseUtil == TRUE)
        tmpRes = ARes[rindex];
      else
        tmpRes = CRes[rindex];
 
      if (UseDed == TRUE)
        tmpRes = MIN(tmpRes,CRes[rindex] - DRes[rindex]);

      TC = MIN(TC,tmpRes / RRes[rindex]);
      }  /* END else (RRes[rindex] == -1) */
 
    if (TC <= 0)
      {
      DBG(9,fSCHED) DPrint("INFO:     TC from %s: %d\n",
        MResourceType[rindex],
        TC);

      return(0);
      }
    }  /* END for (rindex) */
  
  if (UseUtil == TRUE)
    {
    MUNumListGetCount(0,Req->PSlot,Avl->PSlot,0,&tmpTC);
    }
  else
    {
    MUNumListGetCount(0,Req->PSlot,Cfg->PSlot,0,&tmpTC);
    }
 
  TC = MIN(TC,tmpTC);
 
  if (UseUtil == TRUE)
    { 
    MUNumListGetCount(0,Req->GRes,Avl->GRes,0,&tmpTC);
    }
  else
    {
    MUNumListGetCount(0,Req->GRes,Cfg->GRes,0,&tmpTC);
    }

  TC = MIN(TC,tmpTC);

  DBG(8,fSCHED) DPrint("INFO:     %d tasks located\n",
    TC);
 
  return(TC);
  }  /* END MNodeGetTC() */




int MNodeAdjustAvailResources(

  mnode_t *N,       /* I (modified) */
  double   UProcs,  /* I */
  short    DProcs,  /* I */
  short    DTasks)  /* I */

  {
  int  UseUtil = FALSE;
  int  UseDed  = FALSE;

  int  rindex;

  int  CRes[MAX_MRESOURCETYPE];
  int  DRes[MAX_MRESOURCETYPE];
  int  URes[MAX_MRESOURCETYPE];
  int *ARes[MAX_MRESOURCETYPE];

  double MaxLoad;

  if (N == NULL)
    {
    return(FAILURE);
    }

  /* determine processor availability */

  if ((MPar[0].NAvailPolicy[mrProc] == mrapUtilized) ||
      (MPar[0].NAvailPolicy[mrProc] == mrapCombined)) 
    {
    UseUtil = TRUE;
    }
 
  if ((MPar[0].NAvailPolicy[mrProc] == mrapDedicated) ||
      (MPar[0].NAvailPolicy[mrProc] == mrapCombined))
    {
    UseDed = TRUE;
    }

  if (((N->RM != NULL) && (N->RM->Type == mrmtPBS)) || 
       (N->ARes.Procs == -1))
    {
    if ((N->State != mnsIdle) &&
        (N->State != mnsActive) &&
        (N->State != mnsUnknown))
      {
      N->ARes.Procs = 0;
      }
    else
      {
      /* determine available processors */

      if (UseUtil == TRUE)
        {
        if (UseDed == TRUE)
          {
          N->ARes.Procs = MAX(
            0,
            N->CRes.Procs - MAX(UProcs,DProcs) -
            (short)(MPar[0].UntrackedProcFactor *
            MAX(0.0,(N->Load - UProcs))));
          }
        else
          {
          N->ARes.Procs = MAX(
            0,
            N->CRes.Procs - UProcs -
            (short)(MPar[0].UntrackedProcFactor *
            MAX(0.0,(N->Load - UProcs))));
          }
        }
      else if (UseDed == TRUE)
        {
        /* use only dedicated proc information */

        N->ARes.Procs = N->CRes.Procs - DProcs;
        }
      else
        {
        if (N->ARes.Procs < 0)
          N->ARes.Procs = N->CRes.Procs;
        }
      }
    }      /* END if (N->RM->Type ...) */

  /* evaluate node task count */
 
  if ((UseDed == TRUE) &&
    (((N->State == mnsIdle) && (DProcs > 0)) ||
     ((N->State == mnsActive) && (DProcs > N->CRes.Procs))))
    {
    DBG(2,fRM) DPrint("ALERT:    node %s in RM state %s has %d:%d active tasks:procs\n",
      N->Name,
      MNodeState[N->State],
      DTasks,
      DProcs);
 
    if (DProcs >= N->CRes.Procs)
      N->State = mnsBusy; 
    else
      N->State = mnsActive;
    }

  if (N->DRes.Procs == -1)
    {
    N->DRes.Procs = DProcs;
    }
 
  if ((N->RM != NULL) && (N->RM->Type == mrmtLL))
    {
    if ((N->State == mnsIdle) || (N->State == mnsActive))
      {
      N->ARes.Procs = (int)MIN(
        N->CRes.Procs - N->Load + MIN_OS_CPU_OVERHEAD,
        N->CRes.Procs - N->DRes.Procs);
      }
    }

  /* adjust node state by 'maxload' constraints */    

  if (N->MaxLoad > 0.01)
    MaxLoad = N->MaxLoad;
  else if (MSched.DefaultN.MaxLoad > 0.01)
    MaxLoad = MSched.DefaultN.MaxLoad;
  else
    MaxLoad = 0.0;
 
  if (MaxLoad > 0.01)
    {
    double AvailLoad;

    AvailLoad = MAX(0.0,MaxLoad + MIN_OS_CPU_OVERHEAD - N->Load - N->ExtLoad);
 
    if (MPar[0].NodeLoadPolicy == nlpAdjustProcs)
      {
      N->ARes.Procs = MIN(N->ARes.Procs,(int)AvailLoad);

      MNodeAdjustState(N,&N->State);
      }
    else if ((N->Load >= MaxLoad) &&
            ((N->State == mnsIdle) || (N->State == mnsActive)))
      {
      N->State = mnsBusy;
      }
    }

  /* adjust node state */
 
  if (N->State == mnsUnknown)
    {
    if (N->ARes.Procs == N->CRes.Procs)
      N->State = mnsIdle;
    else if (N->ARes.Procs <= 0)
      N->State = mnsBusy;
    else
      N->State = mnsActive; 
 
    N->EState = N->State;
    }

  /* NOTE:  running indicates active workload on system regardless of UseDed/UseUtl */

  if ((N->State == mnsIdle) && (DProcs > 0))
    N->State = mnsActive;

  /* adjust non-proc consumable resources */

  URes[mrMem] = N->URes.Mem;
  CRes[mrMem] = N->CRes.Mem;
  DRes[mrMem] = N->DRes.Mem;
  ARes[mrMem] = &N->ARes.Mem;
 
  URes[mrSwap] = N->URes.Swap;
  CRes[mrSwap] = N->CRes.Swap;
  DRes[mrSwap] = N->DRes.Swap;
  ARes[mrSwap] = &N->ARes.Swap;
 
  URes[mrDisk] = N->URes.Disk;
  CRes[mrDisk] = N->CRes.Disk;
  DRes[mrDisk] = N->DRes.Disk;
  ARes[mrDisk] = &N->ARes.Disk;

  for (rindex = mrMem;rindex <= mrDisk;rindex++)
    {
    if ((ARes[rindex] == NULL) || (*ARes[rindex] != -1))
      continue;

    /* determine effective specific/default policy */

    {
    char pval;

    UseUtil = FALSE;
    UseDed  = FALSE;

    pval = mrapNONE;

    if (N->NAvailPolicy != NULL)
      {
      pval = (N->NAvailPolicy[rindex] != mrapNONE) ? 
        N->NAvailPolicy[rindex] : N->NAvailPolicy[0];
      }

    if (pval == mrapNONE)
      {
      pval = (MPar[0].NAvailPolicy[rindex] != mrapNONE) ?
        MPar[0].NAvailPolicy[rindex] : MPar[0].NAvailPolicy[0];
      }
 
    /* determine policy impact */
 
    if ((pval == mrapUtilized) || (pval == mrapCombined)) 
      {
      UseUtil = TRUE;
      }
 
    if ((pval == mrapDedicated) || (pval == mrapCombined))
      {
      UseDed = TRUE;
      }
    }    /* END BLOCK */

    if (UseUtil == TRUE)
      {
      if (UseDed == TRUE)
        *ARes[rindex] = MAX(0,CRes[rindex] - MAX(URes[rindex],DRes[rindex]));
      else
        *ARes[rindex] = MAX(0,CRes[rindex] - URes[rindex]);
      }
    else if (UseDed == TRUE)
      {
      /* dedicated resources only */

      *ARes[rindex] = MAX(0,CRes[rindex] - DRes[rindex]);  
      }
    else
      {
      *ARes[rindex] = CRes[rindex];
      }
    }  /* END for (rindex) */

  return(SUCCESS);
  }  /* END MNodeAdjustAvailResources() */




int MCResAdd(

  mcres_t *R,                   /* I/O */
  mcres_t *Cfg,                 /* I */
  mcres_t *Req,                 /* I */
  int      Cnt,                 /* I */
  int      EnforceConstraints)  /* I (boolean) */

  {
  int index;
 
  R->Procs += (Req->Procs == -1) ? Cfg->Procs : MIN(Cfg->Procs,Cnt * Req->Procs);
  R->Mem   += (Req->Mem   == -1) ? Cfg->Mem   : MIN(Cfg->Mem,  Cnt * Req->Mem);
  R->Disk  += (Req->Disk  == -1) ? Cfg->Disk  : MIN(Cfg->Disk, Cnt * Req->Disk);
  R->Swap  += (Req->Swap  == -1) ? Cfg->Swap  : MIN(Cfg->Swap, Cnt * Req->Swap);

  if (EnforceConstraints == TRUE)
    {
    R->Procs = MIN(Cfg->Procs,R->Procs);
    R->Mem   = MAX(Cfg->Mem,  R->Mem);
    R->Disk  = MAX(Cfg->Disk, R->Disk);
    R->Swap  = MAX(Cfg->Swap, R->Swap);
    }

  if (Req->PSlot[0].count != MAX_INT)
    {
    for (index = 0;index < MAX_MCLASS;index++)
      {
      if ((index > 0) && (MAList[eClass][index][0] == '\0'))
        break;

      R->PSlot[index].count += MIN(Cfg->PSlot[index].count,Req->PSlot[index].count * Cnt);

      if (EnforceConstraints == TRUE) 
        {
        R->PSlot[index].count = MIN(Cfg->PSlot[index].count,R->PSlot[index].count);
        }
      }  /* END for (index) */
    }    /* END if (Req->PSlot[0].count != MAX_INT) */
 
  for (index = 1;index < MAX_MCLASS;index++)
    {
    if (MAList[eGRes][index][0] == '\0')
      break;
 
    R->GRes[index].count += MIN(Cfg->GRes[index].count,Req->GRes[index].count * Cnt);
    R->GRes[0].count     += MIN(Cfg->GRes[0].count,Req->GRes[index].count * Cnt);
    }  /* END for (index) */
 
  return(SUCCESS);
  }  /* END MCResAdd() */




int MCResRemove(
 
  mcres_t *R,       /* I */
  mcres_t *Cfg,     /* I */
  mcres_t *Req,     /* I */
  int      Cnt,     /* I */
  int      EnforceConstraints) /* I (boolean) */
 
  {
  int index;

  if ((R == NULL) || (Cfg == NULL) || (Req == NULL))
    {
    return(FAILURE);
    }
 
  if (Cnt == 0)
    {
    return(SUCCESS);
    }
 
  R->Procs -= (Req->Procs == -1) ? Cfg->Procs : MIN(Cfg->Procs,Cnt * Req->Procs);
  R->Mem   -= (Req->Mem   == -1) ? Cfg->Mem   : MIN(Cfg->Mem,  Cnt * Req->Mem);
  R->Disk  -= (Req->Disk  == -1) ? Cfg->Disk  : MIN(Cfg->Disk, Cnt * Req->Disk);
  R->Swap  -= (Req->Swap  == -1) ? Cfg->Swap  : MIN(Cfg->Swap, Cnt * Req->Swap);
 
  if (EnforceConstraints == TRUE)
    {
    R->Procs = MAX(0,R->Procs);
    R->Mem   = MAX(0,R->Mem);
    R->Disk  = MAX(0,R->Disk);
    R->Swap  = MAX(0,R->Swap);
    }
 
  if (Req->PSlot[0].count != MAX_INT)
    {
    int ccount;

    for (index = 0;index < MAX_MCLASS;index++)
      {
      if (MAList[eClass][index][0] == '\0')
        break;

      ccount = Req->PSlot[index].count;

      if (ccount > 0)
        {
        if ((EnforceConstraints == FALSE) ||
           (R->PSlot[index].count > MIN(Cfg->PSlot[index].count,ccount * Cnt)))
          {
          R->PSlot[index].count -= MIN(Cfg->PSlot[index].count,ccount * Cnt);
          }
        else
          {
          R->PSlot[index].count = 0;
          }
        }
      }   /* END for (index) */
    }     /* END if (Req->PSlot[0].count != MAX_INT) */
 
  for (index = 1;index < MAX_MCLASS;index++)
    {
    if (MAList[eGRes][index][0] == '\0')
      break;
 
    R->GRes[index].count -= MIN(Cfg->GRes[index].count,Req->GRes[index].count * Cnt);
    R->GRes[0].count     -= MIN(Cfg->GRes[0].count,Req->GRes[index].count * Cnt);
    }  /* END for (index) */
 
  return(SUCCESS);
  }  /* END MCResRemove() */ 




int MResGetPE(

  mcres_t *R,      /* I */
  mpar_t  *P,      /* I */
  double  *PEPtr)  /* O */
 
  {
  double PE;
 
  PE = 0.0;

  if (P->CRes.Procs > 0)
    {
    PE = (double)R->Procs / P->CRes.Procs;
 
    if ((R->Mem > 0) && (P->CRes.Mem > 0))
      PE = MAX(PE,(double)R->Mem  / P->CRes.Mem);
 
    if ((R->Disk > 0) && (P->CRes.Disk > 0))
      PE = MAX(PE,(double)R->Disk / P->CRes.Disk);
 
    if ((R->Swap > 0) && (P->CRes.Swap > 0))
      PE = MAX(PE,(double)R->Swap / P->CRes.Swap);

    PE *= P->CRes.Procs;  
    }

  if (PEPtr != NULL)
    *PEPtr = PE;

  return(SUCCESS);
  }  /* END MResGetPE() */




int MNodeCheckStatus(

  mnode_t *NPtr)  /* I (optional) */

  {
  int  nindex;

  long Delta;

  mnode_t *N;

  const char *FName = "MNodeCheckStatus";
 
  DBG(3,fSTRUCT) DPrint("%s()\n",
    FName);
 
  /* update EState */
  /* purge expired node data */

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if ((NPtr != NULL) && (NPtr != N))
      continue;

    if (N->Name[0] == '\1')
      continue;
 
    DBG(7,fSTRUCT) DPrint("INFO:     checking node '%s'\n",
      N->Name);

    Delta = (long)MSched.Time - N->ATime;
 
    if (Delta > MSched.NodePurgeTime)
      {
      DBG(2,fSTRUCT) DPrint("WARNING:  node '%s' not detected in %s\n",
        N->Name,
        MULToTString((long)MSched.Time - N->ATime));
 
      DBG(2,fSTRUCT) DPrint("INFO:     removing 'stale' node '%s'\n",
        N->Name);
 
      MNodeRemove(N);

      continue;
      }

    if (N->EState == mnsNONE)
      N->EState = N->State;

    if ((N->State != N->EState) &&
        (MSched.Time > N->SyncDeadLine))
      {
      DBG(2,fSTRUCT) DPrint("ALERT:    node '%s' sync from expected state '%s' to state '%s' at %s",
        N->Name,
        MNodeState[N->EState],
        MNodeState[N->State],
        MULToDString(&MSched.Time));
 
      N->EState = N->State;
      }
    }    /* END for (nindex) */

  return(SUCCESS);
  }  /* END MNodeCheckStatus() */




int MUNLCopy(
 
  mnalloc_t   *NodeList,  /* O */
  mnodelist_t  MNodeList, /* I */
  int          NodeIndex,
  int          NodeCount)
 
  {
  int index;
 
  for (index = 0;index < NodeCount;index++)
    {
    NodeList[index].N  = MNodeList[NodeIndex][index].N;
    NodeList[index].TC = MNodeList[NodeIndex][index].TC;
    }  /* END for (index) */
 
  NodeList[index].N = NULL;
 
  return(SUCCESS);
  }  /* END MUNLCopy() */




int MUNLGetMinAVal(
 
  mnalloc_t  *NL,     /* I */
  int         NAttr,  /* I */
  mnode_t   **NPtr,   /* O (optional) */
  void      **Val)    /* O */
 
  {
  mnode_t *N;

  int nindex;
 
  const char *FName = "MUNLGetMinAVal";
 
  DBG(6,fSTRUCT) DPrint("%s(NL,NAttr,N,Val)\n",
    FName);
 
  if ((NL == NULL) || (NL[0].N == NULL) || (Val == NULL))
    {
    return(FAILURE);
    }

  if (NAttr == mnaNodeType)
    {
    ((char *)Val)[0] = '\0';
    }

  switch (NAttr)
    {
    case mnaSpeed:
 
      {
      double MinSpeed = 999999999999.0;
 
      if (MPar[0].UseMachineSpeed == FALSE)
        {
        *(double *)Val = 1.0;

        if (NPtr != NULL)
          *NPtr = NL[0].N;
 
        return(SUCCESS);
        }
 
      for (nindex = 0;NL[nindex].N != NULL;nindex++)
        {
        N = NL[nindex].N;

        if (N->Speed < MinSpeed)
          {
          MinSpeed = N->Speed;
  
          if (NPtr != NULL)
            *NPtr = N;
          }
        }    /* END for (nindex) */
 
      if ((MinSpeed <= 0.0) || (MinSpeed > 99999999)) 
        {
        if (NPtr != NULL)
          *NPtr = NL[0].N;  

        *(double *)Val = 1.0;

        return(FAILURE);
        }
 
      *(double *)Val = MinSpeed;
      }  /* END BLOCK */
 
      break;
 
    case mnaProcSpeed:
 
      {
      int MinSpeed = 99999999;

      for (nindex = 0;NL[nindex].N != NULL;nindex++)
        {
        N = NL[nindex].N;
 
        if (N->ProcSpeed < MinSpeed)
          {
          MinSpeed = N->ProcSpeed;
 
          if (NPtr != NULL)
            *NPtr = N;
          }
        }    /* END for (nindex) */
 
      if ((MinSpeed <= 0) || (MinSpeed > 99999999))
        {
        if (NPtr != NULL)
          *NPtr = NL[0].N;
 
        *(int *)Val = 1;

        return(FAILURE);
        }
 
      *(int *)Val = MinSpeed;
      }  /* END BLOCK */
 
      break;

    case mnaNodeType:

      {
      int    MinPSpeed;
      double MinSpeed;

      mnode_t *N1 = NULL;
      mnode_t *N2 = NULL;

      if ((MUNLGetMinAVal(NL,mnaSpeed,&N1,(void **)&MinSpeed) == FAILURE) &&
          (MUNLGetMinAVal(NL,mnaProcSpeed,&N2,(void **)&MinPSpeed) == FAILURE))
        {
        strcpy((char *)Val,MDEF_NODETYPE);     

        return(FAILURE);
        }

      if (Val != NULL)
        {
        if (N1 != NULL)
          strcpy((char *)Val,N1->NodeType);      
        else if (N2 != NULL)
          strcpy((char *)Val,N2->NodeType);
        else
          strcpy((char *)Val,MDEF_NODETYPE);

        if (Val[0] == '\0')
          strcpy((char *)Val,MDEF_NODETYPE);
        }  /* END if (Val != NULL) */
      }    /* END BLOCK */

      break;
 
    default:

      /* NO-OP */
 
      break;
    }  /* END switch(NAttr) */
 
  return(SUCCESS);
  }  /* END MUNLGetMinAVal() */




int MUNLGetMaxAVal(
 
  mnalloc_t  *NL,     /* I */
  int         NAttr,  /* I */
  mnode_t   **NPtr,   /* O (optional) */
  void      **Val)    /* O */
 
  {
  mnode_t *N;
 
  int nindex;
 
  const char *FName = "MUNLGetMaxAVal";
 
  DBG(6,fSTRUCT) DPrint("%s(NL,NAttr,N,Val)\n",
    FName);
 
  if ((NL == NULL) || (NL[0].N == NULL) || (Val == NULL))
    {
    return(FAILURE);
    }
 
  switch (NAttr)
    {
    case mnaSpeed:
 
      {
      double MaxSpeed = -1.0;
 
      if (MPar[0].UseMachineSpeed == FALSE)
        {
        *(double *)Val = 1.0;

        if (NPtr != NULL)
          *NPtr = N;
 
        return(SUCCESS);
        }
 
      for (nindex = 0;NL[nindex].N != NULL;nindex++) 
        {
        N = NL[nindex].N;
 
        if (N->Speed > MaxSpeed)
          {
          MaxSpeed = N->Speed;
 
          if (NPtr != NULL)
            *NPtr = N;
          }
        }    /* END for (nindex) */
 
      if ((MaxSpeed <= 0.0) || (MaxSpeed > 99999999))
        {
        if (NPtr != NULL)
          *NPtr = NL[0].N;
 
        *(double *)Val = 1.0;
 
        return(FAILURE);
        }
 
      *(double *)Val = MaxSpeed;
      }  /* END BLOCK */
 
      break;
 
    case mnaProcSpeed:
 
      {
      int MaxSpeed = -1;
 
      for (nindex = 0;NL[nindex].N != NULL;nindex++)
        {
        N = NL[nindex].N; 
 
        if (N->ProcSpeed > MaxSpeed)
          {
          MaxSpeed = N->ProcSpeed;
 
          if (NPtr != NULL)
            *NPtr = N;
          }
        }    /* END for (nindex) */
 
      if ((MaxSpeed <= 0) || (MaxSpeed > 99999999))
        {
        if (NPtr != NULL)
          *NPtr = NL[0].N;
 
        *(int *)Val = 1;
 
        return(FAILURE);
        }
 
      *(int *)Val = MaxSpeed;
      }  /* END BLOCK */
 
      break;
 
    case mnaNodeType:
 
      {
      int MaxPSpeed;
      int MaxSpeed;
 
      if ((MUNLGetMaxAVal(NL,mnaSpeed,&N,(void **)&MaxSpeed) == FAILURE) &&
          (MUNLGetMaxAVal(NL,mnaProcSpeed,&N,(void **)&MaxPSpeed) == FAILURE)) 
        {
        strcpy((char *)Val,MDEF_NODETYPE);
 
        return(FAILURE);
        }

      if (Val != NULL)
        {
        if ((N != NULL) && (N->NodeType[0] != '\0'))
          strcpy((char *)Val,N->NodeType);
        else
          strcpy((char *)Val,MDEF_NODETYPE);
        }
      }  /* END BLOCK */
 
      break;
 
    default:

      /* NO-OP */
 
      break;
    }  /* END switch(NAttr) */
 
  return(SUCCESS);
  }  /* END MUNLGetMaxAVal() */




int MNodeProcessFeature(
 
  mnode_t *N,         /* I */
  char    *FString)   /* I */
 
  {
  char *C;
  char *ptr;
 
  if ((N == NULL) || (FString == NULL))
    {
    return(FAILURE);
    }
 
  C = MSched.ProcSpeedFeatureHeader;
 
  if (C[0] != '\0')
    {
    if ((C[strlen(C) - 1] == '$') &&
        !strncmp(FString,C,strlen(C) - 1) &&
        isdigit(FString[strlen(C) - 1]))
      {
      ptr = &FString[strlen(C) - 1];
 
      N->ProcSpeed = strtol(ptr,NULL,0);
      }
    else if (!strncmp(FString,C,strlen(C)))
      {
      ptr = &FString[strlen(C)];
 
      N->ProcSpeed = strtol(ptr,NULL,0);
      }
    }
 
  C = MSched.NodeTypeFeatureHeader; 
 
  if (C[0] != '\0')
    {
    if ((C[strlen(C) - 1] == '$') &&
        !strncmp(FString,C,strlen(C) - 1) &&
        isdigit(FString[strlen(C) - 1]))
      {
      ptr = &FString[strlen(C) - 1];
 
      MUStrCpy(N->NodeType,ptr,sizeof(N->NodeType));
      }
    else if (!strncmp(FString,C,strlen(C)))
      {
      ptr = &FString[strlen(C)];
 
      MUStrCpy(N->NodeType,ptr,sizeof(N->NodeType));
      }
    }
 
  C = MSched.PartitionFeatureHeader;
 
  if ((C[0] != '\0') && !strncmp(FString,C,strlen(C)))
    {
    mpar_t *P;

    ptr = &FString[strlen(C)];

    if (MParAdd(ptr,&P) == SUCCESS)
      {
      N->PtIndex = P->Index;
      }
    else
      {
      DBG(6,fSTRUCT) DPrint("ALERT:    cannot add partition '%s', ignoring partition header\n",
        ptr);
      }
    }
 
  if (!strncmp(FString,"xm",2))
    {
    /* handle scheduler extension feature */
 
    ptr = &FString[strlen("xm")]; 
 
    if (!strncmp(ptr + 2,"ML",2))
      {
      /* max load */
 
      N->MaxLoad = strtod(ptr + 4,NULL);
      }
    }
 
  MUGetMAttr(eFeature,FString,mAdd,N->FBM,sizeof(N->FBM));
 
  return(SUCCESS);
  }  /* END MNodeProcessFeature() */




int MNodeToXML(
  
  mnode_t *N,       /* I */
  mxml_t *E,       /* O */
  int     *SAList)  /* I (optional) */

  {
  int DAList[] = {    
    mnaStatATime,
    mnaStatTTime,
    mnaStatUTime,
    -1 };

  int  aindex;
 
  int *AList;

  char tmpString[MAX_MLINE];          

  if ((N == NULL) || (E == NULL))
    {
    return(FAILURE);
    }

  if (SAList != NULL)
    AList = SAList;
  else
    AList = DAList;
 
  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    if ((MNodeAToString(N,AList[aindex],tmpString,0) == FAILURE) ||
        (tmpString[0] == '\0'))
      {
      continue;
      }
 
    MXMLSetAttr(E,(char *)MNodeAttr[AList[aindex]],tmpString,mdfString);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MNodeToXML() */




int MNodeFromXML(

  mnode_t *N,  /* I (modified) */
  mxml_t *E)  /* I */

  {
  int aindex;
  int naindex;

  if ((N == NULL) || (E == NULL))
    {
    return(FAILURE);
    }

  /* NOTE:  do not initialize.  may be overlaying data */
 
  for (aindex = 0;aindex < E->ACount;aindex++)
    {
    naindex = MUGetIndex(E->AName[aindex],MNodeAttr,FALSE,0);
 
    if (naindex == 0)
      continue;
 
    MNodeSetAttr(N,naindex,(void **)E->AVal[aindex],mdfString,mSet);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MNodeFromXML() */




int MNodeLoadCP(

  mnode_t *NP,   /* I (optional) */
  char    *Buf)  /* I */
 
  {
  char    tmpName[MAX_MNAME];
  char    NodeName[MAX_MNAME];

  char   *ptr;
 
  mnode_t *N;
 
  long    CkTime;

  mxml_t *E = NULL;

  const char *FName = "MNodeLoadCP";
 
  DBG(4,fCKPT) DPrint("%s(NP,%s)\n",
    FName,
    (Buf != NULL) ? Buf : "NULL");

  if (Buf == NULL)
    {
    return(FAILURE);
    }
 
  /* FORMAT:  <JOBHEADER> <JOBID> <CKTIME> <JOBSTRING> */
 
  /* determine job name */
 
  sscanf(Buf,"%s %s %ld",
    tmpName,
    NodeName,
    &CkTime);
 
  if (((long)MSched.Time - CkTime) > MCP.CPExpirationTime)
    return(SUCCESS);
 
  if (NP == NULL)
    {
    if (MNodeFind(NodeName,&N) != SUCCESS)
      {
      DBG(5,fCKPT) DPrint("INFO:     node '%s' no longer detected\n",
        NodeName);
 
      return(SUCCESS);
      }
    }
  else
    {
    N = NP;
    }

  if ((ptr = strchr(Buf,'<')) == NULL)
    {
    return(FAILURE);
    }
 
  MXMLCreateE(&E,"node");
 
  MXMLFromString(&E,ptr,NULL,NULL);
 
  MNodeFromXML(N,E);
 
  MXMLDestroyE(&E);
 
  return(SUCCESS);
  }  /* END MNodeLoadCP() */




int MNodeToString(
 
  mnode_t *N,   /* I */
  char    *Buf) /* O */
 
  {
  const int CPAList[] = {
    mnaStatATime,
    mnaStatTTime,
    mnaStatUTime,
    -1 };
 
  mxml_t *E = NULL;
 
  if ((N == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  MXMLCreateE(&E,"node");
 
  MNodeToXML(N,E,(int *)CPAList);
 
  MXMLToString(E,Buf,MAX_MBUFFER,NULL,TRUE);
 
  MXMLDestroyE(&E);
 
  return(SUCCESS);
  }  /* END MNodeToString() */




int MNodeEval(

  mnode_t *N)  /* I */

  {
  if (N == NULL)
    {
    return(FAILURE);
    }

  /* disk */

  if (N->ARes.Disk > 0)
    {
    if (N->CRes.Disk <= 0)
      N->CRes.Disk = N->ARes.Disk;
    }
  else
    {
    N->ARes.Disk = 1;
    N->CRes.Disk = 1;
    }

  /* mem */
 
  if (N->CRes.Mem > 0)
    {
    if (N->ARes.Mem <= 0)
      N->ARes.Mem = N->CRes.Mem;
    }
  else
    {
    N->ARes.Mem = 1;
    N->CRes.Mem = 1;
    }

  /* swap */
 
  if (N->CRes.Swap > 0)
    {
    if (N->ARes.Swap <= 0)
      N->ARes.Swap = N->CRes.Swap;
    }
  else
    {
    /* virtual memory always at least as large as real memory */
 
    N->ARes.Swap = MAX(MIN_SWAP,N->ARes.Mem);
    N->CRes.Swap = MAX(MIN_SWAP,N->CRes.Mem);
    }

  return(SUCCESS);
  }  /* END MNodeEval() */




int MNodeGetLocation(

  mnode_t *N)  /* I */
 
  {
  int   sindex;
  int   findex;

  int   nindex;
 
  char  tmpName[MAX_MNAME];
  tmpName[0]='\0';
 
  char *ptr;
 
  long  Address=0;

  mhost_t *S;
 
  const char *FName = "MNodeGetLocation";
 
  DBG(5,fCONFIG) DPrint("%s(%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL");

  if (N == NULL)
    {
    return(FAILURE);
    }

  if ((N->FrameIndex >= 0) && (N->SlotIndex >= 0))
    {
    /* frame/slot location previously determined */

    return(SUCCESS);
    } 

  if (MSched.Mode != msmSim)
    {
    if (MOSGetHostName(N->Name,NULL,(unsigned long *)&Address) == FAILURE)
      {
      DBG(2,fCONFIG) DPrint("ALERT:    cannot determine address for host '%s'\n",
        N->Name);
 
      Address = -1;
      }
    }
  else
    {
    Address = -1;
    }

  /* attempt to locate node in cluster table */
 
  for (findex = 1;findex < MAX_MFRAME;findex++)
    {
    for (sindex = 1;sindex <= MAX_MSLOTPERFRAME;sindex++)
      { 
      S = &MSys[findex][sindex];

      MUStrCpy(tmpName,N->Name,sizeof(tmpName));
 
      /* match long/short hostnames */
 
      if ((ptr = strchr(tmpName,'.')) != NULL)
        { 
        if (!strchr(S->NetName[mnetEN0],'.'))
          {
          /* make long name short */

          *ptr = '\0';
          }
        }
      else
        {
        if (strchr(S->NetName[mnetEN0],'.'))
          {
          /* make short name long */
      
          if (MSched.DefaultDomain[0] == '.')
            {
            sprintf(tmpName,"%s%s",
              N->Name,
              MSched.DefaultDomain);
            }
          else
            {
            sprintf(tmpName,"%s.%s",
              N->Name,
              MSched.DefaultDomain);
            }
          }
        }

      for (nindex = 1;nindex < MAX_MNETTYPE;nindex++) 
        {
        if (!strcmp(tmpName,S->NetName[nindex]) ||
           (Address == S->NetAddr[nindex]))
          {
          break;
          }
        }    /* END for (nindex) */

      if (nindex > MAX_MNETTYPE)
        {
        /* host does not match */

        continue;
        }

      /* host match located */
 
      if (S->RMName[0] == '\0')
        {
        DBG(6,fCONFIG) DPrint("INFO:     RMName '%s' set for node[%02d][%02d] '%s'\n",
          tmpName,
          findex,
          sindex,
          N->Name);
 
        MUStrCpy(S->RMName,tmpName,sizeof(S->RMName));
 
        S->MTime             = MSched.Time;
        MFrame[findex].MTime = MSched.Time;
 
        MRM[0].RMNI = nindex;
        }

      MNodeSetAttr(N,mnaFrame,(void **)&findex,mdfInt,0);
      MNodeSetAttr(N,mnaSlot,(void **)&sindex,mdfInt,0);
 
      return(SUCCESS);
      }  /* END for (sindex) */
    }    /* END for (findex) */

  /* no host match located */

  if (MNodeLocationFromName(N,&findex,&sindex) == FAILURE)
    {
    DBG(2,fSIM) DPrint("INFO:     cannot determine node/frame of host '%s'\n",
      N->Name);

    N->FrameIndex = -1;
    N->SlotIndex  = -1;
    }
  else
    {
    /* found node.  update system table */

    MNodeSetAttr(N,mnaFrame,(void **)&findex,mdfInt,0);
    MNodeSetAttr(N,mnaSlot,(void **)&sindex,mdfInt,0);

    return(SUCCESS);
    }

  return(FAILURE);
  }  /* END MNodeGetLocation() */




int MFrameAddNode(

  mframe_t *F,      /* I (modified) */
  mnode_t  *N,      /* I */
  int       SIndex) /* I */

  {
  mhost_t *S;

  if ((F == NULL) || (N == NULL))
    {
    return(FAILURE);
    }

  if ((N->FrameIndex == F->Index) && 
      (N->SlotIndex > 0) && 
     ((N->SlotIndex == SIndex) || (SIndex == -1)))
    {
    /* node already added */

    return(SUCCESS);
    }

  N->FrameIndex = F->Index;

  /* NOTE:  only add when both frame and slot are set */

  if ((SIndex == -1) && (N->SlotIndex == -1))
    {
    /* full location info not yet available */

    DBG(1,fSCHED) DPrint("INFO:     node slot not set on node '%s'\n",
      N->Name);

    return(SUCCESS);
    }

  if (SIndex != -1)
    N->SlotIndex = SIndex;

  /* adequate info available, add node to frame */

  F->NodeCount++;

  if (F->Name[0] == '\0')
    {
    sprintf(F->Name,"%02d",
      F->Index);
    } 

  F->MTime = MSched.Time;

  N->FrameIndex = F->Index;

  if ((N->PtIndex <= 0) && (F->PtIndex != 0))
    {
    N->PtIndex = F->PtIndex;
    }

  S = &MSys[F->Index][N->SlotIndex];

  if (S->NetName[mnetEN0][0] == '\0')
    {
    /* populate empty ethernet name */

    strcpy(S->NetName[mnetEN0],N->Name);
    }

  if (S->RMName[0] == '\0')
    {
    /* set resource manager name */

    strcpy(S->RMName,N->Name);
    }

  if (!(S->Attributes & (1 << attrSystem)))
    {
    /* compute node located */

    S->Attributes |= (1 << attrBatch);
    }

  if (N->NodeType[0] == '\0') 
    {
    strcpy(N->NodeType,F->NodeType);
    }

  /* set frame classes */

  if ((N->CRes.PSlot[0].count == 0) && (S->Classes != 0))
    {
    int cindex;

    for (cindex = 0;cindex < MAX_MCLASS;cindex++)
      {
      if (S->Classes & (1 << cindex))
        {
        N->CRes.PSlot[cindex].count = 1;
        N->CRes.PSlot[0].count      = 1;
        }
      }  /* END for (cindex) */
    }    /* END BLOCK */

  DBG(5,fRM) DPrint("INFO:     node '%s' assigned to location F:%d/S:%d\n",
    N->Name,
    N->FrameIndex,
    N->SlotIndex);

  return(SUCCESS);
  }  /* END MFrameAddNode() */





int MNodeProcessPrioF(

  mnode_t *N,     /* I (modified) */
  char    *PSpec) /* I */

  {
  char *ptr;
  char *tail;

  double PFactor;

  int   NegCount;

  int   pindex;

  /* FORMAT:  [<WS>][<FACTOR>][<WS>]<COMP>... */
  /*          <FACTOR> -> [+-]<DOUBLE>        */

  /* e.g.  NODEPRIO   6*LOAD + -.01 * CFGMEM - JOBCOUNT */

  if (N == NULL)
    {
    return(FAILURE);
    }

  ptr = PSpec;

  if (N->P == NULL)
    {
    if ((N->P = (mnprio_t *)calloc(1,sizeof(mnprio_t))) == NULL)
      {
      return(FAILURE);
      }

    N->P->CWIsSet = TRUE;
    }

  /* FORMAT:  <COEF1> * <FACTOR> [ {+|-} <COEF> * <FACTOR> ] ... */

  while ((ptr != NULL) && (*ptr != '\0'))
    {
    NegCount = 0;

    while (isspace(*ptr) || (*ptr == '-') || (*ptr == '+'))
      {
      if (*ptr == '-')
        NegCount++;

      ptr++;
      }

    if (isalpha(*ptr))
      {
      PFactor = 1.0;
      }
    else
      {
      PFactor = strtod(ptr,&tail);

      ptr = tail;
      }

    while ((isspace(*ptr)) || (*ptr == '*'))
      ptr++;

    pindex = MUGetIndex(ptr,MNPComp,TRUE,0);

    if (pindex > 0)
      {
      ptr += strlen(MNPComp[pindex]);
      }
    else
      {
      while (isalnum(*ptr))
        ptr++;
      }

    if ((PFactor != 0.0) && (pindex > 0))
      {
      N->P->CW[pindex] = (NegCount % 2) ? -PFactor : PFactor;
      }
    }  /* END while (ptr != NULL) */

  return(SUCCESS);
  }  /* END MNodeProcessPrioF() */




int MNodeGetPriority(

  mnode_t  *N,          /* I */
  int       RAff,       /* I */
  int       JPref,      /* I (boolean) */
  double   *Val,        /* O */
  long      StartTime)  /* I */

  {
  double    tmpP;

  double   *CW;

  if (Val != NULL)
    *Val = 0;

  if ((N == NULL) || (Val == NULL))
    {
    return(FAILURE);
    }

  if ((N->P != NULL) && (N->P->CWIsSet == TRUE))
    {
    CW = N->P->CW;
    }
  else if ((MSched.DefaultN.P != NULL) && 
           (MSched.DefaultN.P->CWIsSet == TRUE))
    {
    CW = MSched.DefaultN.P->CW;
    }
  else
    {
    *Val = (double)N->Priority + 
      100.0 * JPref + 
      100.0 * RAff;

    return(SUCCESS);
    }

  tmpP = 0.0;

  if ((N->P != NULL) && 
      (N->P->SPIsSet == TRUE))
    {
    tmpP += N->P->SP;
    }
  else
    {
    double tmpSP;

    /* calculate static priority component */

    tmpSP = 0.0;

    tmpSP += CW[mnpcCDisk]    * N->CRes.Disk;
    tmpSP += CW[mnpcCMem]     * N->CRes.Mem;
    tmpSP += CW[mnpcCProcs]   * N->CRes.Procs;
    tmpSP += CW[mnpcCSwap]    * N->CRes.Swap;
    tmpSP += CW[mnpcPriority] * N->Priority; 
    tmpSP += CW[mnpcSpeed]    * N->Speed;

    if ((N->P != NULL) || 
       ((N->P = (mnprio_t *)calloc(1,sizeof(mnprio_t))) != NULL))
      {
      N->P->SP      = tmpSP;
      N->P->SPIsSet = TRUE;
      }

    tmpP += tmpSP;
    }

  if (StartTime == MSched.Time)
    {
    if ((N->P != NULL) && 
        (N->P->DPIsSet == TRUE) && 
        (N->P->DPIteration == MSched.Iteration))
      {
      tmpP += N->P->DP;
      }
    else
      {
      double tmpDP;
 
      /* calculate dynamic priority component */
 
      tmpDP = 0.0;
 
      tmpDP += CW[mnpcADisk] * N->ARes.Disk;
      tmpDP += CW[mnpcAMem]  * N->ARes.Mem;
      tmpDP += CW[mnpcASwap] * N->ARes.Swap;
      tmpDP += CW[mnpcLoad]  * N->Load;    
      tmpDP += CW[mnpcUsage] * ((N->SUTime > 0) ? ((double)N->SATime / N->SUTime) : 0.0);  

      if ((N->P != NULL) ||
         ((N->P = (mnprio_t *)calloc(1,sizeof(mnprio_t))) != NULL))
        {
        N->P->DP          = tmpDP;
        N->P->DPIsSet     = TRUE;
        N->P->DPIteration = MSched.Iteration;
        }
 
      tmpP += tmpDP;
      }
    }    /* END if (StartTime <= MSched.Time) */

  /* add intra-iteration/context based components */

  tmpP += CW[mnpcPref]        * JPref;
  tmpP += CW[mnpcResAffinity] * RAff;  

  if (StartTime <= MSched.Time)
    {
    tmpP += CW[mnpcAProcs]      * N->ARes.Procs;   
    tmpP += CW[mnpcJobCount]    * N->AP.Usage[mptMaxJob][0];     
    }

  *Val = tmpP; 

  return(SUCCESS);
  }  /* END MNodeGetPriority() */




char *MNodePrioFToString(

  mnode_t *N,   /* I */
  char    *Buf) /* O (optional) */

  {
  static char tmpLine[MAX_MLINE];

  int aindex;

  char *ptr;

  if (N == NULL)
    {
    return(FAILURE);
    }

  if (Buf == NULL)
    ptr = tmpLine;
  else
    ptr = Buf;

  ptr[0] = '\0';

  if (N->P == NULL)
    {
    return(ptr);
    }
 
  for (aindex = 0;MNPComp[aindex] != NULL;aindex++)
    {
    if (N->P->CW[aindex] == 0.0)
      {
      continue;
      }

    if (N->P->CW[aindex] == 1.0)
      {
      sprintf(temp_str,"%s%s",
        ((N->P->CW[aindex] > 0.0) && (ptr[0] != '\0')) ? "+" : "",
        MNPComp[aindex]);
      strcat(ptr,temp_str);
      }
    else
      {
      sprintf(temp_str,"%s%.2lf*%s",
        ((N->P->CW[aindex] > 0.0) && (ptr[0] != '\0')) ? "+" : "",
        N->P->CW[aindex],
        MNPComp[aindex]);
      strcat(ptr,temp_str);
      }
    }    /* END for (aindex) */
  
  return(ptr);
  }  /* END MNodePrioFToString() */




int MNodeConfigShow(

  mnode_t *NS,     /* I */
  int      VFlag,  /* I */
  int      PIndex, /* I */
  char    *Buf)    /* O */

  {
  int AList[] = {
    mnaMaxLoad,
    mnaPriority,
    mnaPrioF,
    -1 };

  int aindex;
  int nindex;

  mnode_t *N;

  char NLine[MAX_MLINE];
  char tmpLine[MAX_MLINE];

  if (Buf == NULL)
    {
    return(FAILURE);
    }

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    if (NS != NULL)
      {
      if (nindex == 0)
        N = NS;
      else
        break;
      }
    else
      {
      N = MNode[nindex];
      }

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    NLine[0] = '\0';

    for (aindex = 0;AList[aindex] != -1;aindex++)
      {
      if ((MNodeAToString(
             N,
             AList[aindex],
             tmpLine,
             (VFlag == TRUE) ? (1 << mcmVerbose) : 0) == FAILURE) ||
          (tmpLine[0] == '\0'))
        {
        continue;
        }

      sprintf(temp_str,"%s%s=%s",
        (NLine[0] != '\0') ? " " : "",
        MNodeAttr[AList[aindex]],
        tmpLine);
      strcat(NLine, temp_str);
      }    /* END for (aindex) */

    if ((VFlag == TRUE) || (NLine[0] != '\0'))
      MUShowSSArray(MPARM_NODECFG,N->Name,NLine,Buf);    
    }  /* END for (nindex) */
     
  return(SUCCESS);
  }  /* END MNodeConfigShow() */




int MNodeCheckAllocation(
 
  mnode_t *N)  /* I */
 
  {
  int     jindex;
  int     nindex;
 
  mjob_t *J;
 
  const char *FName = "MNodeCheckAllocation";
 
  DBG(5,fRM) DPrint("%s(%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL");
 
  if (N == NULL)
    {
    return(FAILURE);
    }
 
  for (jindex = 0;MAQ[jindex] != -1;jindex++)
    {
    J = MJob[MAQ[jindex]];
 
    if ((J->State != mjsRunning) &&
        (J->State != mjsStarting))
      {
      continue;
      }
 
    for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
      {
      if (J->NodeList[nindex].N == N)
        {
        return(SUCCESS);
        }
      }   /* END for (nindex) */
    }     /* END for (jindex) */
 
  return(FAILURE);
  }  /* END MNodeCheckAllocation() */




int MNodeLocationFromName(

  mnode_t *N,
  int     *FIndex,
  int     *SIndex)

  {
  char *FName = "MNodeLocationFromName";

  DBG(7,fSTRUCT) DPrint("%s(%s,FIndex,SIndex)\n",
    FName,
    (N != NULL) ? N->Name : "NULL");

  if (N == NULL)
    {
    return(FAILURE);
    }

  /* NYI */

  return(FAILURE);
  }  /* END MNodeLocationFromName() */




int MNodeFreeTable()

  {
  int nindex;

  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    if (MNode[nindex] == NULL)
      continue;

    MNodeDestroy(&MNode[nindex]);

    MUFree((char **)&MNode[nindex]);
    }  /* END for (nindex) */

  return(SUCCESS);
  }  /* END MNodeFreeTable() */




int MFrameAdd(

  char      *FName,  /* I (optional) */
  int       *FIndex, /* I (optional) */
  mframe_t **FP)     /* O (optional) */

  {
  mframe_t *F;

  int       findex;

  if ((FName == NULL) && (FIndex == NULL))
    {
    return(FAILURE);
    }

  if (FP != NULL)
    *FP = NULL;

  if ((FIndex == NULL) || (*FIndex < 0))
    {
    /* locate available frame */

    for (findex = 0;findex < MAX_MFRAME;findex++)
      {
      F = &MFrame[findex];

      if (F->Name[0] == '\0')
        {
        break;
        }

      if ((FName != NULL) && !strcmp(FName,F->Name))
        {
        /* frame already exists */

        if (FP != NULL)
          *FP = &MFrame[findex];

        if (FIndex != NULL)
          *FIndex = findex;

        return(SUCCESS);
        }

      break;
      }  /* END for (findex) */

    if (findex >= MAX_MFRAME)
      {
      return(FAILURE);
      }

    if (FIndex != NULL)
      *FIndex = findex;

    F = &MFrame[findex];
    }  /* END if (FIndex != NULL) */
  else
    {
    findex = *FIndex;
    }

  F = &MFrame[findex];

  /* initialize frame */

  memset(F,0,sizeof(mframe_t));

  F->Index = findex;

  if (FName != NULL)
    {
    MUStrCpy(F->Name,FName,sizeof(F->Name));
    }
  else
    {
    sprintf(F->Name,"FRAME%02d",
      F->Index);
    }

  F->MTime = MSched.Time;

  return(SUCCESS);
  }  /* END MFrameAdd() */




int MFrameFind(

  char      *FName,
  mframe_t **F)

  {
  if (FName == NULL)
    {
    return(FAILURE);
    }

  /* NYI */

  return(FAILURE);
  }  /* END MFrameFind() */




int MFrameShow(

  char   *SFName,  /* I (optional) */
  mpar_t *SP,      /* I */
  char   *Buffer,  /* I */
  int     BufSize, /* I */
  int     Mode)    /* I */

  {
  int      findex;

  mframe_t *F;

  mpar_t *P;

  char *BPtr;
  int   BSpace;

  const char *FName = "MFrameShow";

  DBG(3,fSTRUCT) DPrint("%s(%s,SP,Buffer,%d,%d)\n",
    FName,
    (SFName != NULL) ? SFName : "NULL",
    BufSize,
    Mode);

  if (Buffer == NULL)
    {
    return(FAILURE);
    }

  BPtr   = Buffer;
  BSpace = BufSize;

  BPtr[0] = '\0';

  MUSNPrintF(&BPtr,&BSpace,"\ndiagnosing frame table (%d slots)\n\n",
    MAX_MFRAME);

  MUSNPrintF(&BPtr,&BSpace,"--------- %2s %4s %8s %10s %10s\n\n",
    "NC",
    "Mem",
    "Disk(MB)",
    "Partition",
    "UpdateTime");

  for (findex = 0;findex < MAX_MFRAME;findex++)
    {
    F = &MFrame[findex];

    if (F->PtIndex >= 0)
      P = &MPar[F->PtIndex];
    else
      P = NULL;

    if (F->NodeCount == 0)
      {
      /* frame is empty */

      continue;
      }

    if ((SFName != NULL) && strcmp(SFName,NONE))
      {
      if (strcmp(SFName,F->Name))
        continue;
      }

    if ((SP != NULL) && (P != NULL) && (P != SP))
      {
      continue;
      }

    MUSNPrintF(&BPtr,&BSpace,"Frame[%02d] %02d %04d %08d %10s %11s\n",
      F->Index,
      F->NodeCount,
      F->Memory,
      F->Disk,
      (P != NULL) ? P->Name : NONE,
      MULToTString(MSched.Time - F->MTime));
    }  /* END for (findex) */

  return(SUCCESS);
  }  /* END MFrameShow() */




int MNodeShowRes(

  mnode_t *NS,          /* I (optional) */
  char    *RID,         /* I (optional) */
  mpar_t  *P,           /* I (optional) */
  int      DFlags,      /* I (BM) */
  int      DisplayMode, /* I */
  char    *SBuf,        /* O */
  int      SBufSize)    /* I */

  {
  int    nindex;
  int    rindex;

  mjob_t  *J;
  mres_t  *R;
  mnode_t *N;

  mres_t *ARes[MAX_MRES];
  int     AResCount[MAX_MRES];

  int    ResFound;  /* (boolean) */

  char  *BPtr;
  int    BSpace;

  mxml_t *RE = NULL;
  mxml_t *DE = NULL;

  const char *FName = "MNodeShowRes";

  DBG(3,fSTRUCT) DPrint("%s(NS,RID,P,DFlags,DisplayMode,SBuf,SBufSize)\n",
    FName);

  if (SBuf == NULL)
    {
    return(FAILURE);
    }

  BPtr   = SBuf;
  BSpace = SBufSize;

  BPtr[0] = '\0';

  if ((RID == NULL) || !strcmp(RID,NONE))
    {
    ResFound = TRUE;
    }
  else
    {
    ResFound = FALSE;
    }

  switch(DisplayMode)
    {
    case mwpXML:

      {
      int SAList[] = {
        msysaPresentTime,
        -1 };

      int SCList[] = {
        -1 };

      mxml_t *tmpE = NULL;

      if ((MXMLCreateE(&RE,"response") == FAILURE) ||
          (MXMLCreateE(&DE,"data") == FAILURE))
        {
        return(FAILURE);
        }

      MXMLAddE(RE,DE);

      MUIXMLSetStatus(RE,SUCCESS,NULL,0);

      if (MSysToXML(&MSched,&tmpE,SAList,SCList,0) == SUCCESS)
        {
        MXMLAddE(DE,tmpE);
        }
      }  /* END BLOCK */

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(DisplayMode) */

  if (DFlags & (1 << mcmSummary))
    {
    memset(ARes,0,sizeof(ARes));
    memset(AResCount,0,sizeof(AResCount));

    if (ResFound == TRUE)
      {
      switch(DisplayMode)
        {
        case mwpXML:

          MUIXMLSetStatus(RE,FAILURE,"reservation not specified",-1);

          MXMLToString(RE,BPtr,BSpace,NULL,TRUE);

          break;

        default:

          MUSNPrintF(&BPtr,&BSpace,"ERROR:    reservation not specified\n");

          break;
        }  /* END switch(DisplayMode) */

      DBG(2,fSTRUCT) DPrint("INFO:     reservation not specified for summary\n");

      return(SUCCESS);
      }
    }    /* END if (DFlags & (1 << mcmSummary)) */

  /* NOTE:  default behavior, display only specified reservations */
  /*        verbose behavior, display all reservations on nodes with specified reservation */
  /*        summary behavior, display summary of all active reservations */

  for (nindex = 0;MNode[nindex] != NULL;nindex++)
    {
    N = MNode[nindex];

    if ((N == NULL) || (N->Name[0] == '\0'))
      break;

    if (N->Name[0] == '\1')
      continue;

    if ((NS != NULL) && (MNode[nindex] != NS))
      continue;

    if ((P != NULL) && (P->Index > 0) && (N->PtIndex != P->Index))
      continue;

    DBG(5,fSTRUCT) DPrint("INFO:     checking reservations for node '%s'\n",
      N->Name);

    for (rindex = 0;rindex < MSched.ResDepth;rindex++)
      {
      R = N->R[rindex];

      if (R == NULL)
        break;

      if (R == (mres_t *)1)
        continue;

      /* continue if specified res not matched */

      if ((RID != NULL) && strcmp(RID,NONE))
        {
        if (strcmp(RID,R->Name) && 
          !(DFlags & (1 << mcmVerbose)) &&
          !(DFlags & (1 << mcmSummary)))
          {
          continue;
          }

        ResFound = TRUE;
        }

      DBG(7,fSTRUCT) DPrint("INFO:     displaying res '%s' at slot %d on node %s\n",
        R->Name,
        rindex,
        N->Name);

      if (DFlags & (1 << mcmSummary))
        {
        int srindex;

        if (strcmp(RID,R->Name) && (R->Type == mrtJob))
          {
          for (srindex = 0;ARes[srindex] != NULL;srindex++)
            {
            if (!strcmp(ARes[srindex]->Name,R->Name))
              break;
            }

          if (ARes[srindex] == NULL)
            ARes[srindex] = R;

          AResCount[srindex]++;
          }

        continue;
        }   /* END if (DFlags & (1 << mcmSummary)) */

      switch(DisplayMode)
        {
        case mwpXML:

          {
          mxml_t *tE = NULL;

          MNResToString(
            N,
            (!(DFlags & (1 << mcmVerbose))) ? R : NULL,
            &tE, 
            NULL,
            0);

          if (tE != NULL)
            {
            MXMLAddE(DE,tE);
            }
          }  /* END BLOCK */

          break;

        default:

          /* NYI */

          break;
        }  /* END switch(DisplayMode) */      

      if (DFlags & (1 << mcmVerbose))
        {
        break;
        }
      }    /* END for (rindex) */
    }      /* END for (nindex) */

  if (ResFound == FALSE)
    {
    switch(DisplayMode)
      {
      case mwpXML:

        MUIXMLSetStatus(RE,FAILURE,"cannot locate reservation",-1);

        MXMLToString(RE,BPtr,BSpace,NULL,TRUE);

        break;

      default:

        MUSNPrintF(&BPtr,&BSpace,"ERROR:    cannot locate reservation '%s'\n",
          RID);

        break;
      }  /* END switch(DisplayMode) */

    DBG(2,fSTRUCT) DPrint("INFO:     cannot locate reservation '%s'\n",
      RID);

    return(SUCCESS);
    }  /* END if (ResFound == FALSE) */

  if (DFlags & (1 << mcmSummary))
    {
    int srindex;

    for (srindex = 0;ARes[srindex] != NULL;srindex++)
      {
      R = ARes[srindex];

      J = (mjob_t *)R->J;

      switch(DisplayMode)
        {
        case mwpXML:

          {
          mxml_t *tmpE = NULL;

          const int RAList[] = {
            mraName,
            mraCreds,
            mraJState,
            mraStartTime,
            mraEndTime,
            -1 };

          if (MXMLCreateE(&tmpE,(char *)MXO[mxoRsv]) == FAILURE)
            {
            return(FAILURE);
            }

          MResToXML(R,tmpE,(int *)RAList);

          MXMLSetAttr(
            tmpE,
            (char *)MResAttr[mraTaskCount],
            (void **)&AResCount[srindex],
            mdfInt);

          MXMLAddE(DE,tmpE);
          }  /* END BLOCK */

          break;

        default:

          /* NYI */

          break;
        }  /* END switch(DisplayMode) */
      }    /* END for (srindex) */
    }      /* END if (Flags & (1 << mcmSummary)) */

  switch(DisplayMode)
    {
    case mwpXML:

      if (RE != NULL)
        {
        MXMLToString(RE,BPtr,BSpace,NULL,TRUE);

        MXMLDestroyE(&RE);
        }

      break;

    default:

      /* NO-OP */

      break;
    }  /* END switch(DisplayMode) */

  return(SUCCESS);
  }  /* END MNodeShowRes() */

/* END MNode.c */


