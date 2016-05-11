/* HEADER */
        
/*  Contains:                                *
 *  int MStatBuildRClass(JobLimit,RClass)    *
 *  double MStatCalcCommunicationCost(J)     *
 *  double MStatGetCom(nindex1,nindex2)      *
 *  int MStatClearUsage(OType,PTypeBM,POnly) *
 *  int MStatUpdateActiveJobUsage(J)         *
 *  int MStatInitializeActiveSysUsage()      *
 *  int MStatBuildGrid(sindex,Buffer,Mode)   *
 *  char *MStatGetGrid(sindex,G,R,C,T,Mode)  *
 *  int MStatUpdateCompletedJobUsage(J,RunMode,ProfileMode) *
 *  int MStatUpdateRejectedJobUsage(J,ProfileMode) *
 *                                           */



#include "moab.h"
#include "msched-proto.h"

extern mlog_t   mlog;

extern msched_t  MSched;
extern mstat_t   MStat;
extern mqos_t    MQOS[];

extern mnode_t  *MNode[];
extern mjob_t   *MJob[];
extern msim_t    MSim;
extern mgcred_t  *MUser[];
extern mgcred_t  MGroup[];
extern mgcred_t   MAcct[];
extern mrclass_t MRClass[];
extern mpar_t    MPar[];
extern mclass_t  MClass[];
extern int       MFQ[];

extern const char *MStatType[];
extern const char *MXO[];
extern const char *MStatAttr[];
extern const char *MLimitAttr[];
extern const char *MSysAttr[];




int MStatProfInitialize(

  mprofcfg_t *P)  /* I */

  {
  int    index;
  int    distance;

  double tmp;
  double gstep;

  int    ScaleModFactor;

  const char *FName = "MStatProfInitialize";

  DBG(2,fSTAT) DPrint("%s(P)\n",
    FName);

  if (P == NULL)
    {
    return(FAILURE);
    }

  if (P->TimeStepSize > 0)
    P->MaxTime = (unsigned long)(P->MinTime * pow(P->TimeStepSize,P->TimeStepCount));

  if (P->NodeStepSize > 0)
    P->MaxNode = (unsigned long)(P->MinNode * pow(P->NodeStepSize,P->NodeStepCount));

  /* set up time scale */

  distance = P->MaxTime / P->MinTime;
  tmp      = (double)1.0 / P->TimeStepCount;

  gstep = pow((double)distance,tmp);

  tmp = (double)1.0;

  DBG(4,fSTAT) DPrint("INFO:     time min value: %4ld  distance: %4d  step: %6.2f\n",
    P->MinTime,
    distance,
    gstep);

  P->TimeStep[0] = P->MinTime;

  ScaleModFactor = 0;

  for (index = 1;index <= P->TimeStepCount;index++)
    {
    tmp *= gstep;

    P->TimeStep[index] = (int)(tmp * P->MinTime + 0.5);

    /* skip previously used values */
 
    if ((index != 0) && (P->TimeStep[index - ScaleModFactor] ==
                         P->TimeStep[index - ScaleModFactor - 1]))
      {
      ScaleModFactor++;
      }

    DBG(5,fSTAT) DPrint("INFO:     TimeStep[%02d]: %8ld\n",
      index,
      P->TimeStep[index]);
    }

  P->TimeStep[P->TimeStepCount + 1] = 999999999;

  DBG(4,fSTAT) DPrint("INFO:     time steps eliminated: %3d\n",
    ScaleModFactor);
  
  P->TimeStepCount -= ScaleModFactor;

  /* set up node scale */

  distance = P->MaxNode / P->MinNode;
  tmp      = (double)1.0 / P->NodeStepCount;

  gstep = pow((double)distance,tmp);

  tmp = (double)1.0;

  DBG(4,fSTAT) DPrint("INFO:     node min value: %3ld  distance: %3d  step: %4.2f\n",
    P->MinNode,
    distance,
    gstep);

  P->NodeStep[0] = P->MinNode;

  ScaleModFactor = 0;

  for (index = 1;index <= P->NodeStepCount;index++)
    {
    tmp *= gstep;

    P->NodeStep[index - ScaleModFactor] = (int)(tmp * P->MinNode + .5);

    /* if value is used previously, skip it */
  
    if ((index != 0) && 
        (P->NodeStep[index - ScaleModFactor] == P->NodeStep[index - ScaleModFactor - 1]))
      {
      ScaleModFactor++;
      }

    DBG(5,fSTAT) DPrint("INFO:     NodeStep[%02d]: %4ld\n",
      index,
      P->NodeStep[index]);
    }

  P->NodeStep[P->NodeStepCount + 1] = 999999999;

  DBG(4,fSTAT) DPrint("INFO:     node steps eliminated: %d\n",
    ScaleModFactor);

  P->NodeStepCount -= ScaleModFactor;

  /* set up accuracy scale */

  gstep = 0.0;

  for (index = 0;index < P->AccuracyScale;index++)
    {
    gstep += (double)100 / P->AccuracyScale;

    P->AccuracyStep[index] = (int)gstep;

    DBG(5,fSTAT) DPrint("INFO:     AccuracyStep[%02d]: %4ld\n",
      index,
      P->AccuracyStep[index]);
    }

  return(SUCCESS);
  }  /* END MStatProfInitialize() */




#ifndef __MPROF

int MStatBuildRClass(

  int        JobLimit, /* I */
  mrclass_t *RClass)   /* O */

  {
  int cindex;
  int cindex2;
  int nindex;
  int rqindex;

  unsigned long Nodes;
  unsigned long Workload;
  int           count;
  unsigned long AvgWorkLoad;
  unsigned long Average;

  mnode_t  *N;
  mjob_t   *J;

  mreq_t    *RQ;

  const char *FName = "MStatBuildRClass";

  DBG(3,fSTAT) DPrint("%s(%d)\n",
    FName,
    JobLimit);

  /* erase old class values */
 
  memset(RClass,0,sizeof(mrclass_t) * MAX_MRCLASS);
 
  /* determine memory class constraints */
 
  count = 0;
 
  for (nindex = 0;nindex < MAX_MNODE;nindex++)
    {
    N = MNode[nindex];
 
    if ((N == NULL) || (N->Name[0] == '\0'))
      break;
   
    if (N->Name[0] == '\1')
      continue;
 
    for (cindex = 0;cindex < MAX_MRCLASS;cindex++)
      {
      if (N->CRes.Mem == 0)
        {
        /* invalid node memory, ignore */
 
        break;
        }
 
      if (RClass[cindex].Memory == 0)
        {
        /* new 'highest' node class detected */
 
        RClass[cindex].Memory = N->CRes.Mem;
        RClass[cindex].Nodes  = 1;
 
        break;
        }
 
      if (N->CRes.Mem == RClass[cindex].Memory)
        {
        /* class already located */
 
        RClass[cindex].Nodes++;                    
 
        break;
        }
 
      if ((N->CRes.Mem < RClass[cindex].Memory) &&
         ((cindex == 0) ||
         ((cindex > 0) && (N->CRes.Mem > RClass[cindex - 1].Memory))))
        {
        /* new 'intermediate' node class detected */
 
        /* locate class tail */
 
        for (cindex2 = cindex + 1;cindex2 < MAX_MRCLASS - 1;cindex2++)
          {
          if (RClass[cindex2].Memory == 0)
            break;
          }
 
        /* extend class list */
 
        for (;cindex2 > cindex;cindex2--)
          {
          memcpy(&RClass[cindex2],&RClass[cindex2 - 1],sizeof(RClass[cindex2]));
          }  /* END for (cindex2) */
 
        /* insert new class */
 
        RClass[cindex].Memory = N->CRes.Mem;
        RClass[cindex].Nodes  = 1;

        break;
        }  /* END if (N->CRes.Mem < RClass[cindex].Memory) */
      }    /* END for (cindex) */
 
    count = MAX(count,cindex);
    }      /* END for (nindex) */
 
  DBG(3,fSTAT) DPrint("INFO:     classes created: %3d\n",
    count);                                   

  /* add job load to each class */

  count = 0;

  for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
    {
    if (count >= JobLimit)
      break;
    else
      count++;

    for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
      {
      RQ = J->Req[rqindex];

      DBG(4,fSTAT) DPrint("INFO:     evaluating requirements of %s:%d\n",
        J->Name,
        rqindex);

      for (cindex = 0;cindex < MAX_MRCLASS;cindex++)
        {
        if ((RQ->RequiredMemory <= RClass[cindex].Memory) && 
            (RClass[cindex].Nodes > 0))
          {
          RClass[cindex].Workload += 
            RQ->TaskCount * RQ->DRes.Procs * J->WCLimit;

          RClass[cindex].InitialWorkload = RClass[cindex].Workload;

          break;
          }
        }
      }    /* END for (rqindex) */
    }      /* END for (jindex)  */

  DBG(3,fSTAT) DPrint("INFO:     jobs evaluated: %4d\n",
    count);

  DBG(3,fSTAT)
    {
    DBG(3,fSTAT) DPrint("INFO:     initial resource class breakdown:\n");

    DBG(3,fSTAT) DPrint("INFO:     %8s %5s %9s %7s\n",
      "Class",
      "Nodes",
      "NodeHours",
      "Average");

    for (cindex = 0;cindex < MAX_MRCLASS;cindex++)
      {
      if (RClass[cindex].Nodes == 0)
        Average = 0;
      else
        Average = RClass[cindex].Workload / RClass[cindex].Nodes;

      DBG(3,fSTAT) DPrint("INFO:     %8d %5d %9ld %7lu\n",
        RClass[cindex].Memory,
        RClass[cindex].Nodes,
        RClass[cindex].Workload,
        Average);
      }
    }

  /* redistribute workload to more resource intensive nodes if possible */
 
  for (cindex = MAX_MRCLASS - 2;cindex >= 0;cindex--)
    {
    DBG(5,fSTAT) DPrint("INFO:     evaluating class %d\n",
      RClass[cindex].Memory);

    /* determine total nodes for redistribution */

    Nodes = RClass[cindex].Nodes;
    Workload = RClass[cindex].Workload;

    if (Nodes == 0)
      continue;

    AvgWorkLoad = Workload / Nodes;

    for (cindex2 = cindex + 1;cindex2 < MAX_MRCLASS;cindex2++)
      { 
      if (RClass[cindex2].Nodes > 0)
        {
        if (AvgWorkLoad >= RClass[cindex2].Workload / RClass[cindex2].Nodes)
          {
          DBG(6,fSTAT) DPrint("INFO:     class %d (avg: %ld) adding workload (%d nodes, %ld PS) to class %d (avg: %lu)\n",
            RClass[cindex2].Memory,
            RClass[cindex2].Workload / RClass[cindex2].Nodes,
            RClass[cindex2].Nodes,
            RClass[cindex2].Workload,
            RClass[cindex].Memory,
            AvgWorkLoad);

          Nodes    += RClass[cindex2].Nodes;
          Workload += RClass[cindex2].Workload;
          }
        }
      }    /* END for (cindex) */

    DBG(6,fSTAT) DPrint("INFO:     total node pool: %lu  workload: %lu  avg: %lu\n",
      Nodes,
      Workload,
      Workload / Nodes);

    /* re-distribute workload */

    if ((Nodes > 0) && (Workload > 0))
      { 
      for (cindex2 = cindex;cindex2 < MAX_MRCLASS;cindex2++)
        {
        DBG(5,fSTAT) DPrint("INFO:     redistributing workload from class %d to class %d\n",
          RClass[cindex].Memory,
          RClass[cindex2].Memory);

        if (RClass[cindex2].Nodes > 0)
          {
          if (AvgWorkLoad >= RClass[cindex2].Workload / RClass[cindex2].Nodes)
            {
            RClass[cindex2].Workload = 
              (unsigned long)((double)Workload * RClass[cindex2].Nodes / Nodes);

            DBG(5,fSTAT) DPrint("INFO:     new avg workload for class %d: %lu\n",
              RClass[cindex2].Memory,
              RClass[cindex2].Workload / RClass[cindex2].Nodes);
            }
          }
        }         
      }

    /* display load of each class */
  
    DBG(5,fSTAT)
      {
      DBG(5,fSTAT) DPrint("INFO:     resource class breakdown:\n");

      DBG(5,fSTAT) DPrint("INFO:     %8s %5s %9s %7s\n",
        "Class",
        "Nodes",
        "NodeHours",
        "Average");

      for (cindex2 = 0;cindex2 < MAX_MRCLASS;cindex2++)
        {
        if (RClass[cindex2].Nodes == 0)
          Average = 0;
        else
          Average = RClass[cindex2].Workload / RClass[cindex2].Nodes;

        DBG(5,fSTAT) DPrint("INFO:     %8d %5d %9ld %7lu\n",
          RClass[cindex2].Memory,
          RClass[cindex2].Nodes,
          RClass[cindex2].Workload,
          Average);
        }
      }
    }    /* END for (cindex) */

  /* display load of each class */

  DBG(3,fSTAT)
    {
    DBG(3,fSTAT) DPrint("INFO:     resource class breakdown:\n");

    DBG(3,fSTAT) DPrint("INFO:     %8s %5s %9s %7s\n",
      "Class",
      "Nodes",
      "NodeHours",
      "Average");

    for (cindex = 0;cindex < MAX_MRCLASS;cindex++)
      {
      if (RClass[cindex].Nodes == 0)
        Average = 0;
      else
        Average = RClass[cindex].Workload / RClass[cindex].Nodes;

      DBG(3,fSTAT) DPrint("INFO:     %8d %5d %9ld %7lu\n",
        RClass[cindex].Memory,
        RClass[cindex].Nodes,
        RClass[cindex].Workload,
        Average);
      }  /* END for (cindex) */
    }

  return(SUCCESS);
  }  /* END MStatBuildRClass() */

#endif /* __MPROF */
 




double MStatCalcCommunicationCost(

  mjob_t *J)  /* I */

  {
  int   nindex;
  int   index;
  double cost;

  const char *FName = "MStatCalcCommunicationCost";

  DBG(3,fSTAT) DPrint("%s(%s)\n",
    FName,
    J->Name);

  cost = 0.0;

  /* no communication cost for serial job */

  if (J->NodeList[1].N == NULL)
    {
    return(cost);
    }

  for (nindex = 0;J->NodeList[nindex].N != NULL;nindex++)
    {
    switch (MSim.CommunicationType)
      {
      case comRoundRobin:

        if (J->NodeList[nindex + 1].N == NULL)
          cost += MStatGetCom(J->NodeList[nindex].N,J->NodeList[0].N);
        else
          cost += MStatGetCom(J->NodeList[nindex].N,J->NodeList[nindex + 1].N);

        break;

      case comBroadcast:

        for (index = 0;J->NodeList[index].N != NULL;index++)
          {
          if (index == nindex)
            continue;

          cost += MStatGetCom(J->NodeList[nindex].N,J->NodeList[index].N);
          }

        break;

      case comMasterSlave:

        if (nindex != 0)
          cost += MStatGetCom(J->NodeList[nindex].N,J->NodeList[0].N);

        break;
      }
    }    /* END for (nindex) */
  
  return(cost);
  }  /* END MStatCalcCommunicationCosts() */






double MStatGetCom(

  mnode_t *N1,
  mnode_t *N2)

  {
  double cost;

  const char *FName = "MStatGetCom";

  DBG(5,fSTAT) DPrint("%s(%s,%s)\n",
    FName,
    N1->Name,
    N2->Name);

  cost = MSim.IntraFrameCost;

  if (N1->FrameIndex != N2->FrameIndex)
    {
    cost += MSim.InterFrameCost;
    }

  return(cost);
  }  /* END MStatGetCom() */

  



int MStatClearUsage(

  int OType,     /* I (BM, -1 = ALL, 0 = ALL but node) */
  int PTypeBM,   /* I */
  int PStatOnly) /* I: (boolean) */

  {
  int cindex;
  int oindex;

  int   MaxO;

  char  *NPtr;
  mpu_t *AP;
  mpu_t *IP;

  must_t *S;

  mpu_t *APU;
  mpu_t *APC;
  mpu_t *APG;
  mpu_t *APQ;

  mcredl_t *L;

  int OList[] = { 
    mxoUser, 
    mxoGroup, 
    mxoAcct, 
    mxoQOS, 
    mxoClass, 
    mxoPar, 
    mxoNode, 
    -1 };

  const char *FName = "MStatClearUsage";

  DBG(3,fSTAT) DPrint("%s(%s,%s)\n",
    FName,
    MXO[OType],
    (PTypeBM & (1 << mlActive)) ? "Active" : "Idle");

  if (PStatOnly != TRUE)
    {
    MStat.EligibleJobs = 0;
    MStat.IdleJobs     = 0;
    }

  for (oindex = 0;OList[oindex] != -1;oindex++)
    {
    if ((OType > 0) && !(OType & (1 << OList[oindex])))
      continue;

    if ((OType == 0) && (OList[oindex] == mxoNode))
      continue;

    MaxO = MSched.M[OList[oindex]];

    /* clear cred usage */

    for (cindex = 0;cindex < MaxO;cindex++)
      {
      AP   = NULL;
      IP   = NULL;

      APU  = NULL;
      APC  = NULL;
      APG  = NULL;
      APQ  = NULL;

      L    = NULL;

      S    = NULL;

      NPtr = NULL;

      switch(OList[oindex])
        {
        case mxoUser:
 
          if (MUser[cindex] != NULL) 
            {
            L = &MUser[cindex]->L;

            NPtr = MUser[cindex]->Name;
            S    = &MUser[cindex]->Stat;
            }

          break;

        case mxoGroup:

          L    = &MGroup[cindex].L;

          NPtr = MGroup[cindex].Name;
          S    = &MGroup[cindex].Stat;

          break;

        case mxoAcct:

          L    = &MAcct[cindex].L;

          NPtr = MAcct[cindex].Name;
          S    = &MAcct[cindex].Stat;    

          break;

        case mxoQOS:

          L    = &MQOS[cindex].L;

          NPtr = MQOS[cindex].Name;
          S    = &MQOS[cindex].Stat;    
 
          break;

        case mxoClass:

          L    = &MClass[cindex].L;
 
          NPtr = MClass[cindex].Name;
          S    = &MClass[cindex].Stat;    
           
          break;

        case mxoPar:

          L    = &MPar[cindex].L;

          NPtr = MPar[cindex].Name;
          S    = &MPar[cindex].S;    

          break;

        case mxoNode:

	  if (MNode[cindex] != NULL)
            {
            NPtr = MNode[cindex]->Name;
            AP   = &MNode[cindex]->AP;
            }

          break;

        default:

	  /* NO-OP */

          break;
        }  /* END switch(OType[oindex]) */

      if (L != NULL)
        {
        AP  = &L->AP;

        IP  = L->IP;

        APU = L->APU;
        APC = L->APC;
        APG = L->APG;
        APQ = L->APQ;
        }  /* END if (L != NULL) */
        
      if ((NPtr == NULL) || (NPtr[0] == '\0') || (NPtr[0] == '\1'))
        continue;

      DBG(8,fSTAT) DPrint("INFO:     clearing usage stats for %s %s\n",
        MXO[OList[oindex]],
        NPtr);

      if (PTypeBM & (1 << mlActive))
        {
        if (AP != NULL)
          memset(AP->Usage,0,sizeof(AP->Usage));

        if (APU != NULL)
          {
          int uindex;

          for (uindex = 0;uindex < MAX_MUSER + MAX_MHBUF;uindex++)
            {
            memset(APU[uindex].Usage,0,sizeof(APU[uindex].Usage));
            }  /* END for (uindex) */
          }

        if (APC != NULL)
          {
          int cindex;

          for (cindex = 0;cindex < MAX_MCLASS;cindex++)
            {
            memset(APC[cindex].Usage,0,sizeof(APC[cindex].Usage));
            }  /* END for (cindex) */
          }

        if (APG != NULL)
          {
          int cindex;

          for (cindex = 0;cindex < MAX_MGROUP + MAX_MHBUF;cindex++)
            {
            memset(APG[cindex].Usage,0,sizeof(APG[cindex].Usage));
            }  /* END for (cindex) */
          }

        if (APQ != NULL)
          {
          int cindex;

          for (cindex = 0;cindex < MAX_MQOS;cindex++)
            {
            memset(APQ[cindex].Usage,0,sizeof(APQ[cindex].Usage));
            }  /* END for (cindex) */
          }
        }    /* END if (PTypeBM & (1 << mlActive)) */

      if ((IP != NULL) && (PTypeBM & (1 << mlIdle)))
        memset(IP->Usage,0,sizeof(IP->Usage));

      if ((S != NULL) && (PTypeBM & (1 << mlSystem)))
        memset(S,0,sizeof(must_t));
      }  /* END for (cindex) */
    }    /* END for (oindex) */

  if (PTypeBM & (1 << mlSystem))
    {
    MStat.SuccessfulPH          = 0.0;
    MStat.TotalPHAvailable      = 0.0;
    MStat.TotalPHBusy           = 0.0;

    MStat.SuccessfulJobsCompleted = 0;
 
    memset(MStat.Grid,0,sizeof(MStat.Grid));
    memset(MStat.RTotal,0,sizeof(MStat.RTotal));
    memset(MStat.CTotal,0,sizeof(MStat.CTotal));
 
    MStat.MinEff          = 100.0;
    MStat.MinEffIteration = 0;
    }  /* END if (PTypeBM & (1 << mlSystem)) */
 
  return(SUCCESS);
  }  /* END MStatClearUsage() */





int MStatUpdateActiveJobUsage(

  mjob_t *J)  /* I */

  {
  int      nindex;
  int      rqindex;

  double   pesdedicated;

  double   psdedicated;
  double   psutilized;

  /* HvB */
  double   psutilized_cpu; 
  double   psutilized_load;

  double   msdedicated;
  double   msutilized;

  double   fsusage;

  
  int      timeindex;
  int      procindex;

  int      statindex;
  int      stattotal;

  int      TotalProcs;

  mgcred_t  *U;
  mgcred_t *G;
  mgcred_t  *A;
  mqos_t   *Q;
  mclass_t *C;

  int      TC;

  double   interval;
  double   PE;

  mnode_t *N;

  must_t  *S[16];
  mfs_t   *F[16];

  mreq_t  *RQ;
  mpar_t  *P;
  
  double   averagenodespeed = 0.0;
  double   totalnodespeed = 0.0;
  int      speedcounter = 0;

  const char *FName = "MStatUpdateActiveJobUsage";

  DBG(3,fSTAT) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if (J == NULL)
    {
    return(FAILURE);
    }

  /* update 
       adjust credential statistics (iteration based delta)
       fairshare usage (iteration based delta)
       policy based usage (full recalculation)
         policy usage cleared in MQueueSelectAllJobs()

     called from:         MQueueAddAJob()
  */

  if ((J->Req[0] != NULL) &&
      (J->Req[0]->NAccessPolicy == mnacSingleJob) && 
      (MSched.NodeAllocMaxPS == TRUE))
    {
    TotalProcs = J->NodesRequested;
    }
  else
    {
    TotalProcs = MJobGetProcCount(J);
    }

  if (TotalProcs == 0)
    {
    DBG(3,fSTAT) DPrint("INFO:     no tasks associated with job '%s' (no statistics available)\n",
      J->Name);

    return(FAILURE);
    }

  MJobGetPE(J,&MPar[0],&PE);  

  interval = MIN(
    (double)MSched.Interval / 100.0,
    (double)MSched.Time - J->StartTime);

  if ((J->Req[0] != NULL) &&
      (J->Req[0]->NAccessPolicy == mnacSingleJob) && 
      (MSched.NodeAllocMaxPS == TRUE))
    {
    pesdedicated = TotalProcs * interval;
    }
  else
    {
    pesdedicated = PE * interval;
    }

  for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++)
    { 

    RQ = J->Req[rqindex];

    P  = &MPar[RQ->PtIndex];

    totalnodespeed = 0.0;
    speedcounter   = 0;

    psdedicated = 0.0;
    psutilized  = 0.0;

    /* HvB */
    psutilized_cpu  = 0.0;
    psutilized_load  = 0.0;

    msdedicated = 0.0;
    msutilized  = 0.0;


    if ((J->StartTime != MSched.Time) && (J->CTime != MSched.Time))
      { 
      psdedicated = interval * TotalProcs;

      for (nindex = 0;nindex < MAX_MNODE_PER_JOB;nindex++)
        {
        N  = RQ->NodeList[nindex].N;
        TC = RQ->NodeList[nindex].TC;

        if ((N == NULL) || (TC == 0))
          {
          break;
          }
	
        speedcounter++;
        totalnodespeed += N->Speed;

        msdedicated += (double)(interval * TC * RQ->DRes.Mem);

	/*
	 *  HvB: There is a difference between shared nodes and NodeAllocMaxPS (exclusive)
	 *    exclusive : nodes * interval
	 *     shared   : total cores * interval
	 *
	 *  Load and cpu time is calculated for all cores in a node. So we have to normalize
	 *  for psutilized. if singlejob per node is set
	*/
	if ( (J->Req[0]->NAccessPolicy == mnacSingleJob) && 
	    (MSched.NodeAllocMaxPS == TRUE))	
          {
	  psutilized_cpu      += interval * RQ->URes.Procs / (100.0 * TotalProcs * N->CRes.Procs) ;
	  psutilized_load     += interval * (double)N->Load / N->CRes.Procs;
	  }
	else
	  /* Old calculation */
	  {
          if (RQ->URes.Procs > 0)
	    {
	    psutilized      += interval * TC * RQ->URes.Procs / 100.0;
	    RQ->MURes.Procs  = MAX(RQ->MURes.Procs,RQ->URes.Procs);
	    }
          else if (N->CRes.Procs > 1)
	    {
            /* cannot properly determine efficiency with available information */
	    /* make job 100% efficient                                         */ 
	    psutilized += (interval * TC * RQ->DRes.Procs);
            }
          else
	    { 
            if (N->Load >= 1.0)
	      {
	      psutilized += (interval * 1.0);
              }
	    else
              {
	      psutilized += (interval * (double)N->Load);
              }
             }
	   }
        }     /* END for (nindex) */

      /* HvB 
       * Some Jobs do not update the cputime properly , but the load is high. Then use this factor
       * for jobs that uses a single node exclusive
      */
      if ( (J->Req[0]->NAccessPolicy == mnacSingleJob) && 
	  (MSched.NodeAllocMaxPS == TRUE))	
        {
	DBG(8,fSTAT) DPrint("INFO:     psutilized_cpu = %f, psutilized_load = %f\n",
		psutilized_cpu, psutilized_load);

	if ( (psutilized_cpu * 10) < psutilized_load)
	  {
	  psutilized = psutilized_load;
	  }
	else
	  {
	  psutilized = psutilized_cpu;
	  }
	}

      if (RQ->URes.Mem > 0)
        {
        msutilized = interval * RQ->URes.Mem;
        RQ->MURes.Mem = MAX(RQ->MURes.Mem,RQ->URes.Mem);
        }
      else
        {
        msutilized = msdedicated; /* FIXME */
        }

      if (RQ->URes.Swap > 0)
        {
        RQ->MURes.Swap = MAX(RQ->MURes.Swap,RQ->URes.Swap);
        }

      if (RQ->URes.Disk > 0)
        {
        RQ->MURes.Disk = MAX(RQ->MURes.Disk,RQ->URes.Disk);
        }

      if (psdedicated == 0.0)
        {
        DBG(1,fSTAT) DPrint("ALERT:    job %s active for %s has no dedicated time on %d nodes\n",
          J->Name,
          MULToTString(MSched.Time - J->StartTime),
          nindex);
        }
      else
        {
        DBG(6,fSTAT) DPrint("INFO:     job '%18s'  nodes: %3d  PSDedicated: %lf  PSUtilized: %lf  (efficiency: %5.2f)\n",
          J->Name,
          nindex,
          psdedicated,
          psutilized,
          psutilized / psdedicated);
        } /* END for nindex      */
      }   /* END if J->StartTime */

    /* update job specific statistics */

    J->PSUtilized  += psutilized;

    if ((J->PSDedicated < 0) || (psdedicated < 0))
      {   
      DPrint("ALERT:    JPSD: %lf  PSD: %lf\n",
        J->PSDedicated,
        psdedicated);
      }

    J->PSDedicated += psdedicated;

    J->MSUtilized  += msutilized;

    J->MSDedicated += (double)msdedicated;

    /* determine stat grid location */

    for (timeindex = 0;MIN(J->WCLimit,MStat.P.MaxTime) > MStat.P.TimeStep[timeindex];timeindex++);
    for (procindex = 0;MIN(RQ->TaskCount * RQ->DRes.Procs,MStat.P.MaxNode) > MStat.P.NodeStep[procindex];procindex++);

    timeindex = MIN(timeindex,MStat.P.TimeStepCount - 1);

    DBG(7,fSTAT) DPrint("INFO:     updating statistics for Grid[time: %d][proc: %d]\n",
      timeindex,
      procindex);

    /* determine statistics to update */

    memset(S,0,sizeof(S));
    memset(F,0,sizeof(F));

    stattotal = 0;

    S[stattotal++] = &MStat.Grid[timeindex][procindex];
    S[stattotal++] = &MStat.RTotal[procindex];
    S[stattotal++] = &MStat.CTotal[timeindex];

    F[stattotal]   = &MPar[0].F;
    S[stattotal++] = &MPar[0].S;

    if (P->Index != 0)
      {
      F[stattotal]   = &P->F;
      S[stattotal++] = &P->S;
      }

    /* locate/adjust user stats */

    switch(MPar[0].FSC.FSPolicy)
      {
      case fspDPES:

        fsusage = pesdedicated;

        break;

      case fspUPS:

        fsusage = psutilized;

        break;

      case fspDPS:
      default:

        fsusage = psdedicated;

        break;
      }  /* END switch(MPar[0].FSC.FSPolicy) */

    MPolicyAdjustUsage(NULL,J,NULL,mlActive,NULL,-1,1,NULL);

    if (speedcounter == 0)
	    averagenodespeed = 1.0;
    else
      averagenodespeed = totalnodespeed / speedcounter;

    if (P->UseMachineSpeedForFS == TRUE) 
      {
      fsusage *= averagenodespeed;
      psdedicated *= averagenodespeed;
      psutilized *= averagenodespeed;
      msdedicated *= averagenodespeed;
      msutilized *= averagenodespeed;
      }
    
    if ((J != NULL) && (J->Cred.C != NULL))
    	DBG(1,fSTAT) DPrint("INFO: Average nodespeed for Job %s is  %f, %f, %i  \n",J->Name,averagenodespeed,totalnodespeed,speedcounter);
	  
    U = J->Cred.U;

    if (U != NULL)
      {
      DBG(7,fSTAT) DPrint("INFO:     updating statistics for user %s (UID: %ld) \n",
        U->Name,   
        U->OID);

      U->MTime = MSched.Time;

      F[stattotal]   = &U->F;
      S[stattotal++] = &U->Stat;
      }
    else
      {
      DBG(3,fSTAT) DPrint("ALERT:    cannot locate user record for job '%s'\n",
        J->Name);
      }

    /* locate/adjust group stats */

    G = J->Cred.G;

    if (G != NULL)
      {
      DBG(7,fSTAT) DPrint("INFO:     updating statistics for group %s (GID %ld)\n",
        G->Name,
        G->OID);

      G->MTime = MSched.Time;

      /* adjust FairShare record */

      F[stattotal]   = &G->F;     
      S[stattotal++] = &G->Stat;
      }
    else
      {
      DBG(3,fSTAT) DPrint("ALERT:    cannot locate group record for job '%s'\n",
        J->Name);
      }

    /* locate/adjust account stats */

    A = J->Cred.A;

    if (A != NULL)
      {
      DBG(7,fSTAT) DPrint("INFO:     updating statistics for account %s\n",
        A->Name);

      A->MTime          = MSched.Time;

      /* adjust FairShare record */

      F[stattotal]   = &A->F;     
      S[stattotal++] = &A->Stat;
      }
    else
      {
      DBG(7,fSTAT) DPrint("INFO:     job '%s' has no account assigned\n",
        J->Name);
      }

    /* locate/adjust QOS stats */

    if (J->Cred.Q != NULL)
      {
      Q = J->Cred.Q;

      DBG(7,fSTAT) DPrint("INFO:     updating statistics for QOS %s\n",
        Q->Name);
 
      Q->MTime = MSched.Time;
 
      /* adjust FairShare record */
 
      F[stattotal]   = &Q->F;    
      S[stattotal++] = &Q->Stat;
      }
    else
      {
      DBG(7,fSTAT) DPrint("INFO:     job '%s' has no QOS assigned\n",
        J->Name);
      }

    /* locate/adjust Class stats */

    C = J->Cred.C;  
 
    if (C != NULL)
      {
      DBG(7,fSTAT) DPrint("INFO:     updating statistics for class %s\n",
        C->Name);
 
      C->MTime = MSched.Time;
 
      /* adjust FairShare record */
 
      F[stattotal]   = &C->F;    
      S[stattotal++] = &C->Stat;
      }
    else
      {
      DBG(7,fSTAT) DPrint("INFO:     job '%s' has no class assigned\n",
        J->Name);
      }

    /* update all statistics */

    for (statindex = 0;statindex < stattotal;statindex++)
      {
      if (S[statindex] != NULL)
        {
        S[statindex]->PSDedicated += psdedicated;
        S[statindex]->PSUtilized  += psutilized;

        S[statindex]->MSDedicated += msdedicated;
        S[statindex]->MSUtilized  += msutilized;
        }

      if (F[statindex] != NULL)
        {
        F[statindex]->FSUsage[0] += fsusage;
        }
      }  /* END for (statindex) */
    }    /* END for (rqindex) */

  MStat.ActiveJobs++;     

  return(SUCCESS);
  }  /* END MStatUpdateActiveJobUsage() */




int MStatSetDefaults()

  {
  const char *FName = "MStatSetDefaults";

  DBG(3,fSTAT) DPrint("%s()\n",
    FName);

  memset(&MStat,0,sizeof(MStat));       

  /* set default stat values */
 
  MStat.SchedRunTime    = 0;
 
  MStat.MinEff          = 100.0;
  MStat.MinEffIteration = 0;
 
  /* set stats directory */
 
  if (!strstr(MSched.HomeDir,DEFAULT_MSTATDIR))
    {
    sprintf(MStat.StatDir,"%s%s",
      MSched.HomeDir,
      DEFAULT_MSTATDIR);
    }
  else
    {
    strcpy(MStat.StatDir,DEFAULT_MSTATDIR);
    }

    /* set up statistics grid scales */
 
  MStat.P.MaxTime       = DEFAULT_MAXTIME;
  MStat.P.MinTime       = DEFAULT_MINTIME;
  MStat.P.TimeStepCount = DEFAULT_TIMESCALE;
 
  MStat.P.MaxNode       = DEFAULT_MAXNODE;
  MStat.P.MinNode       = DEFAULT_MINNODE;
  MStat.P.NodeStepCount = MDEF_NODESCALE;
 
  MStat.P.AccuracyScale = DEFAULT_ACCURACYSCALE;
 
  return(SUCCESS);
  }  /* END MStatSetDefaults() */





int MStatInitialize(

  mprofcfg_t *P)  /* I */

  {
  const char *FName = "MStatInitialize";

  DBG(3,fSTAT) DPrint("%s(P)\n",
    FName);

  MStatProfInitialize(P);
 
  MStatOpenFile(MSched.Time);

  fprintf(MSched.statfp,"%s %d\n",
    TRACE_WORKLOAD_VERSION_MARKER,
    DEFAULT_WORKLOAD_TRACE_VERSION);

  return(SUCCESS);
  }  /* END MStatInitialize() */




int MStatOpenFile(
 
  long StatTime)
 
  {
  char  Line[MAX_MLINE];
  char  StatFile[MAX_MLINE];
  char  tmpfile[MAX_MLINE];
 
  char *ptr;
 
  int   rc;
 
  char *TokPtr;
 
  time_t tmpTime;
 
  char *FName = "MStatOpenFile";
 
  DBG(2,fSTAT) DPrint("%s(%ld)\n",
    FName, 
    StatTime);
 
  /* FORMAT:  WWW_MMM_DD_YYYY */
 
  if ((MSched.Mode != msmSim) && (MSched.Mode != msmTest))
    {
    tmpTime = (time_t)StatTime;
 
    strcpy(Line,ctime(&tmpTime));
 
    /* get day of week */
 
    ptr = MUStrTok(Line," \t\n",&TokPtr);
 
    strcpy(tmpfile,ptr);
 
    /* get month */
 
    ptr = MUStrTok(NULL," \t\n",&TokPtr);
 
    sprintf(temp_str,"_%s",
      ptr);
    strcat(tmpfile,temp_str);
 
    /* get day of month */
 
    ptr = MUStrTok(NULL," \t\n",&TokPtr);
 
    sprintf(temp_str,"_%02d",
      atoi(ptr));
    strcat(tmpfile,temp_str);

    /* ignore time */
 
    ptr = MUStrTok(NULL," \t\n",&TokPtr);
 
    /* get year */
 
    ptr = MUStrTok(NULL," \t\n",&TokPtr); 
 
    sprintf(temp_str,"_%s",
      ptr);
    strcat(tmpfile,temp_str);
    }
  else
    {
    MUStrCpy(tmpfile,MSim.StatFileName,sizeof(tmpfile));
    }
 
  sprintf(StatFile,"%s%s",
    MStat.StatDir,
    tmpfile);
 
  if ((MSched.statfp != NULL) && (MSched.statfp != mlog.logfp))
    {
    DBG(5,fSTAT) DPrint("INFO:     closing old stat file\n");
 
    fclose(MSched.statfp);
    }
 
  umask(027);
 
  if ((rc = umask(027)) != 027)
    {
    DBG(1,fSTAT) DPrint("ERROR:    cannot set umask before opening statfile '%s'(%o)\n",
      StatFile,
      rc);
    }
 
  if ((MSched.statfp = fopen(StatFile,"a+")) == NULL)
    {
    DBG(0,fSTAT) DPrint("WARNING:  cannot open statfile '%s', errno: %d (%s)\n",
      StatFile,
      errno,
      strerror(errno));
 
    /* on failure, send stats to logfile */ 
 
    MSched.statfp = mlog.logfp;
 
    return(FAILURE);
    }
 
  return(SUCCESS);
  }  /* END MStatOpenFile() */




int MStatInitializeActiveSysUsage()

  {
  const char *FName = "MStatInitializeActiveSysUsage";

  DBG(2,fSTAT) DPrint("%s()\n",
    FName);

  MStatClearUsage(0,(1 << mlIdle)|(1 << mlActive),FALSE);

  return(SUCCESS);
  }  /* END MStatInitializeActiveSysUsage() */





int MStatBuildGrid(

  int   SIndex,  /* I */
  char *Buffer,  /* O */
  int   Mode)    /* I */

  {
  int   timeindex;
  int   procindex;

  must_t *G;
  must_t *C;
  must_t *R;
  must_t *T;

  const char *FName = "MStatBuildGrid";

  DBG(3,fSTAT) DPrint("%s(%s,Buffer,%d)\n",
    FName,
    MStatType[SIndex],
    Mode);

  sprintf(Buffer,"statistics since %s\n",
    MULToDString((mulong *)&MStat.InitTime));

  switch(SIndex)
    {
    case stAvgXFactor:

      strcat(Buffer,"Average XFactor Grid\n\n");

      break;

    case stMaxXFactor:

      strcat(Buffer,"Maximum XFactor (hours)\n\n");

      break;

    case stAvgQTime:

      strcat(Buffer,"Average QueueTime (hours)\n\n");

      break;

    case stAvgBypass:

      strcat(Buffer,"Average Bypass (bypass count)\n\n");

      break;

    case stMaxBypass:

      strcat(Buffer,"Maximum Bypass (bypass count)\n\n");

      break;

    case stJobCount:

      strcat(Buffer,"Job Count (jobs)\n\n");

      break;

    case stPSRequest:

      strcat(Buffer,"ProcHour Request (percent of total)\n\n");

      break;

    case stPSRun:

      strcat(Buffer,"ProcHour Run (percent of total)\n\n");

      break;

    case stWCAccuracy:

      strcat(Buffer,"WallClock Accuracy (percent)\n\n");

      break;

    case stBFCount:

      strcat(Buffer,"BackFill (percent of jobs run)\n\n");

      break;

    case stBFPSRun:

      strcat(Buffer,"BackFill (percent of prochours delivered)\n\n");

      break;

    case stJobEfficiency:

      strcat(Buffer,"Average Job Efficiency (percent CPU utilization)\n\n");

      break;

    case stQOSDelivered:

      strcat(Buffer,"Quality of Service Delivered (percent of jobs meeting QOS)\n\n");

      break;

    default:

      strcat(Buffer,"Unknown Statistics Type\n\n");

      DBG(1,fSTAT) DPrint("ERROR:    unexpected statistics type, %d, in %s()\n",
        SIndex,
        FName);

      break;
    }  /* END switch(SIndex) */

  if (Mode & 8)
    {
    sprintf(temp_str,"         ");
    strcat(Buffer,temp_str);
    }
  else
    {
    sprintf(temp_str,"[ %5s ]",
      "PROCS");
    strcat(Buffer,temp_str);
    }

  for (timeindex = 0;timeindex <= MStat.P.TimeStepCount;timeindex++)
    {
    if (Mode & 8)
      {
      sprintf(temp_str,"   %8s   ",
        MUBStringTime(MStat.P.TimeStep[timeindex]));
      strcat(Buffer,temp_str);
      }
    else
      {
      sprintf(temp_str,"[  %8s  ]",
        MUBStringTime(MStat.P.TimeStep[timeindex]));
      strcat(Buffer,temp_str);
      }
    }    /* END for (timeindex) */

  if (Mode & 8)
    {
    sprintf(temp_str,"              \n");
    strcat(Buffer,temp_str);
    }
  else
    {
    sprintf(temp_str,"[  %8s  ]\n",
      "TOTAL");
    strcat(Buffer,temp_str);
    }

  DBG(3,fSTAT) DPrint("INFO:     stat header created\n");

  T = &MPar[0].S;

  R = &MStat.RTotal[0];

  for (procindex = 0;procindex <= MStat.P.NodeStepCount;procindex++)
    {
    if (Mode & 8)
      {
      sprintf(temp_str,"  %4ld   ",
        MStat.P.NodeStep[procindex]);
      strcat(Buffer,temp_str);
      }
    else
      {
      sprintf(temp_str,"[ %4ld  ]",
        MStat.P.NodeStep[procindex]);
      strcat(Buffer,temp_str);
      }

    R = &MStat.RTotal[procindex];

    C = &MStat.CTotal[0];
    
    for (timeindex = 0;timeindex <= MStat.P.TimeStepCount;timeindex++)
      {
      G = &MStat.Grid[timeindex][procindex];
      C = &MStat.CTotal[timeindex];

      strcat(Buffer,MStatGetGrid(SIndex,G,R,C,T,Mode));

      if (G->Count != 0)
        {
        switch(SIndex)
          {
          default:

            break;
          }
        }
      }      /* END for (timeindex = 0;...) */

    DBG(3,fSTAT) DPrint("INFO:     stat row[%02d] created\n",
      procindex);

    /* calculate row totals */

    strcat(Buffer,MStatGetGrid(SIndex,R,R,C,T,Mode));

    strcat(Buffer,"\n");
    }  /* END for (procindex) */

  /* calculate column totals */

  if (Mode & 8)
    {
    strcat(Buffer,"  TOTAL  ");
    }
  else
    {
    strcat(Buffer,"[ TOTAL ]");
    }

  for (timeindex = 0;timeindex <= MStat.P.TimeStepCount;timeindex++)
    {
    C = &MStat.CTotal[timeindex];

    strcat(Buffer,MStatGetGrid(SIndex,C,R,C,T,Mode));
    }

  DBG(3,fSTAT) DPrint("INFO:     stat column totals created\n");

  strcat(Buffer,"\n");

  /* calculate overall totals */

  switch(SIndex)
    {
    case stAvgXFactor:

      sprintf(temp_str,"%-26s %8.4f\n",
        "Job Weighted XFactor:",
        (T->Count > 0) ? (double)T->XFactor / T->Count : 0.0);
      strcat(Buffer,temp_str);

      sprintf(temp_str,"%-26s %8.4f\n",
        "Proc Weighted XFactor:",
        (T->NCount > 0) ? T->NXFactor / T->NCount : 0.0);
      strcat(Buffer,temp_str);

      sprintf(temp_str,"%-26s %8.4f\n",
        "PS Weighted XFactor:",
        (T->PSRun > 0.0) ? T->PSXFactor / T->PSRun : 0.0);
      strcat(Buffer,temp_str);

      break;

    case stMaxXFactor:

      sprintf(temp_str,"%-26s %8.4f\n",
        "Overall Max XFactor:",
        (double)T->MaxXFactor);
      strcat(Buffer,temp_str);

      break;

    case stAvgQTime:

      sprintf(temp_str,"%-26s %8.4f\n",
        "Job Weighted QueueTime:",
        (T->Count > 0) ? (double)T->TotalQTS / T->Count / 3600.0 : 0.0);
      strcat(Buffer,temp_str);

      break;

    case stAvgBypass:

      sprintf(temp_str,"%-26s %8.4f\n",
        "Job Weighted X Bypass:",
        (T->Count > 0) ? (double)T->Bypass / T->Count : 0.0);
      strcat(Buffer,temp_str);

      break;

    case stMaxBypass:

      sprintf(temp_str,"%-26s %8d\n",
        "Overall Max Bypass:",
        T->MaxBypass);
      strcat(Buffer,temp_str);

      break;

    case stJobCount:

      sprintf(temp_str,"%-26s %8d\n",
        "Total Jobs:",
        T->Count);
      strcat(Buffer,temp_str);

      break;

    case stPSRequest:

      sprintf(temp_str,"%-26s %8.2f\n",
        "Total PH Requested:",
        (double)T->PSRequest / 3600.0);
      strcat(Buffer,temp_str);
    
      break;

    case stPSRun:

      sprintf(temp_str,"%-26s %8.2f\n",
        "Total PH Run",
        (double)T->PSRun / 3600.0);
      strcat(Buffer,temp_str);
    
      break;

    case stWCAccuracy:

      sprintf(temp_str,"%-26s %8.3f\n",
        "Overall WallClock Accuracy:",
        (T->PSRequest > 0.0) ? (double)T->PSRun / T->PSRequest * 100.0 : 0.0);
      strcat(Buffer,temp_str);

      break;

    case stBFCount:

      sprintf(temp_str,"%-26s %8.4f (%d / %d)\n",
        "Job Weighted BackFill Job Percent:",
        (double)T->BFCount / T->Count * 100.0,
        T->BFCount,
        T->Count);
      strcat(Buffer,temp_str);

      break;

    case stBFPSRun:

      sprintf(temp_str,"%-26s %8.4f (%6.2f / %6.2f)\n",
        "PS Weighted BackFill PS Percent:",
        T->BFPSRun / T->PSRun * 100.0,
        T->BFPSRun,
        T->PSRun);
      strcat(Buffer,temp_str);

      break;

    case stJobEfficiency:

      sprintf(temp_str,"%-26s %8.4f (%6.2f / %6.2f)\n",
        "PS Weighted Job Efficiency Percent:",
        (T->PSDedicated > 0.0) ? T->PSUtilized / T->PSDedicated * 100.0 : 0.0,
        T->PSUtilized / 3600.0,
        T->PSDedicated / 3600.0);
      strcat(Buffer,temp_str);

      break;

    case stQOSDelivered:

      sprintf(temp_str,"%-26s %8.4f (%d / %d)\n",
        "Job Weighted QOS Success Rate:",
        (T->Count > 0) ? (double)T->QOSMet / T->Count * 100.0 : 0.0,
        T->QOSMet,
        T->Count);
      strcat(Buffer,temp_str);

      break;

    default:

      sprintf(temp_str,"ERROR:  stat type %d totals not handled\n",
        SIndex);
      strcat(Buffer,temp_str);

      break;
    }  /* END Switch(SIndex) */

  DBG(4,fSTAT) DPrint("INFO:     buildgrid() complete\n");

  sprintf(temp_str,"%-26s %8d\n",
    "Total Samples:",
    T->Count);
  strcat(Buffer,temp_str);

  strcat(Buffer,"\n\n");

  return(SUCCESS);
  }  /* END MStatBuildGrid() */







char *MStatGetGrid(

  int     SIndex,
  must_t *G,
  must_t *R,
  must_t *C,
  must_t *T,
  int     Mode)

  {
  static char Line[MAX_MNAME];

  char doubleline[MAX_MLINE];
  char intline[MAX_MLINE];
  char intonly[MAX_MLINE];
  char defline[MAX_MLINE];
  char nullline[MAX_MLINE];

  const char *FName = "MStatGetGrid";

  DBG(5,fSTAT) DPrint("%s(%d,G,R,C,T,%d)\n",
    FName,
    SIndex,
    Mode);

  if (!(Mode & 8))
    {
    strcpy(doubleline,"[%7.2f %4d]");
    strcpy(intline,   "[%7d %4d]");
    strcpy(intonly,   "[  %8d  ]");
    strcpy(defline,   "[  ????????  ]");
    strcpy(nullline,  "[------------]");
    }
  else
    {
    strcpy(doubleline,"    %7.2f   ");
    strcpy(intline,   "    %7d  ");
    strcpy(intonly,   "   %8d   ");
    strcpy(defline,   "          0   ");
    strcpy(nullline,  "          0   ");
    }

  if (G->Count == 0)
    {
    strcpy(Line,nullline);

    return(Line);
    }

  switch (SIndex)
    {
    case stAvgXFactor:

      sprintf(Line,doubleline,
        G->XFactor / G->Count,
        G->Count);

      break;

    case stMaxXFactor:

      sprintf(Line,doubleline,
        G->MaxXFactor,
        G->Count);

      break;

    case stAvgQTime:

      sprintf(Line,doubleline,
        (double)(G->TotalQTS) / G->Count / 3600.0,
        G->Count);

      break;

    case stAvgBypass:

      sprintf(Line,doubleline,
        (double)G->Bypass / G->Count,
        G->Count);

      break;

    case stMaxBypass:

      sprintf(Line,intline,
        G->MaxBypass,
        G->Count);

      break;

    case stJobCount:

      sprintf(Line,intonly,
        G->Count);

      break;

    case stPSRequest:

      sprintf(Line,doubleline,
        G->PSRequest / T->PSRequest * 100.0,
        G->Count);

      break;

    case stPSRun:

      sprintf(Line,doubleline,
        G->PSRun / T->PSRun * 100.0,
        G->Count);

      break;

    case stWCAccuracy:

      sprintf(Line,doubleline,
        G->PSRun / G->PSRequest * 100,
        G->Count);

      break;

    case stBFCount:

      sprintf(Line,doubleline,
        (double)G->BFCount / G->Count * 100.0,
        G->Count);

      break;

    case stBFPSRun:

      sprintf(Line,doubleline,
        G->BFPSRun / G->PSRun * 100.0,
        G->Count);

      break;

    case stJobEfficiency:

      sprintf(Line,doubleline,
        G->PSUtilized / G->PSDedicated * 100.0,
        G->Count);

      break;

    case stQOSDelivered:

      sprintf(Line,doubleline,
        (double)G->QOSMet / G->Count * 100.0,
        G->Count);

      break;

    default:

      strcpy(Line,defline);

      DBG(3,fSTAT) DPrint("ALERT:    stat type %d not handled\n",
        SIndex);

      break;
    }

  return(Line);
  }  /* END MStatGetGrid() */
 




int MStatUpdateCompletedJobUsage(

  mjob_t *J,           /* I */
  int     RunMode,     /* I */
  int     ProfileMode) /* I */

  {
  unsigned long run;
  unsigned long request;
  unsigned long queuetime;

  int   psrun;
  int   psrequest;
  int   psremaining;
  double accuracy;
  double xfactor;

  long  rval;
  int   timeindex;
  int   procindex;
  int   accindex;

  int   statindex;
  int   stattotal;

  int   QOSMet;

  int   TotalProcs;

  double  PE;

  double  cost;

  must_t *S[10];

  mreq_t  *RQ;

  must_t *GS = &MPar[0].S;

  const char *FName = "MStatUpdateCompletedJobUsage";

  DBG(5,fSTAT) DPrint("%s(%s,%x,%x)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    RunMode,
    ProfileMode);

  if (J == NULL)
    {
    return(FAILURE);
    }

  TotalProcs = MJobGetProcCount(J);

  MJobGetPE(J,&MPar[0],&PE);

  RQ = J->Req[0]; /* FIXME */

  /* if job failed to run, return */

  if (((J->SubmitTime == 0) && (MSched.Time > 100000)) || 
       (J->StartTime == 0) || 
       (J->CompletionTime == 0))
    {
    DBG(2,fSTAT) DPrint("ALERT:    job %s did not run. (Q: %ld S: %ld: C: %ld)\n",
      J->Name,
      J->SubmitTime,
      J->StartTime,
      J->CompletionTime);

    return(FAILURE);
    }

  /* determine base statistics */

  run = (J->CompletionTime > J->StartTime) ? 
    J->CompletionTime - J->StartTime : 0;

  run       = MIN(run,J->WCLimit);

  request   = J->WCLimit;

  queuetime = (J->StartTime > J->SystemQueueTime) ?
    J->StartTime - J->SystemQueueTime : 0;

  if (J->StartTime < J->SystemQueueTime)
    {
    DBG(0,fSTAT) DPrint("ALERT:    job '%18s' has invalid system queue time (SQ: %ld > ST: %ld)\n",
      J->Name,
      J->SystemQueueTime,
      J->StartTime);
    }
  
  psremaining = TotalProcs * (J->StartTime + J->WCLimit - MSched.Time);

  if (psremaining < 0)
    psremaining = 0;

  accuracy = (double)run / request;

  psrun     = run     * TotalProcs;
  psrequest = request * TotalProcs;

  /* determine XFactor */

  xfactor = (ProfileMode & (1 << mTrueXFactor)) ?
    (double)(queuetime + request) / request :
    (double)(queuetime + run) / request;

  QOSMet = TRUE;

  if (J->Cred.Q != NULL)
    {
    mqos_t *Q;

    Q = J->Cred.Q;

    if (((Q->XFTarget > 0.0) && (xfactor > (double)Q->XFTarget)) ||
        ((Q->QTTarget > 0) && (queuetime > Q->QTTarget)))
      {
      QOSMet = FALSE;
      }
    }    /* END if (J->Cred.Q != NULL) */
    
  DBG(3,fSTAT) DPrint("INFO:     job '%18s' completed.  QueueTime: %6ld  RunTime: %6ld  Accuracy: %5.2f  XFactor: %5.2f\n",
    J->Name,
    queuetime,
    run,
    accuracy * 100,
    xfactor);

  DBG(4,fSTAT) DPrint("INFO:     start: %8ld  complete: %8ld  SystemQueueTime: %8ld\n",
    (unsigned long)J->StartTime,
    (unsigned long)J->CompletionTime,
    (unsigned long)J->SystemQueueTime);

  DBG(3,fSTAT) DPrint("INFO:     overall statistics.  Accuracy: %5.2f  XFactor: %5.2f\n",
    (GS->Count > 0) ? (GS->JobAcc / GS->Count) : 0.0,
    (GS->Count > 0) ? (GS->XFactor / GS->Count) : 0.0);

  /* determine statistics grid location */

  if (ProfileMode & (1 << mUseRunTime))
    {
    rval = (J->CompletionTime > J->StartTime) ?
      J->CompletionTime - J->StartTime : 0;
    }
  else
    {
    rval = J->WCLimit;
    }

  rval = MIN(rval,MStat.P.MaxTime);

  for (timeindex = 0;timeindex < MAX_MGRIDTIMES;timeindex++)
    {
    if (rval <= MStat.P.TimeStep[timeindex])
      break;
    }

  timeindex = MIN(timeindex,MStat.P.TimeStepCount - 1);

  for (procindex = 0;procindex < MAX_MGRIDSIZES;procindex++)
    {
    if (MStat.P.NodeStep[procindex] >= MIN(TotalProcs,MStat.P.MaxNode))
      break;
    }

  for (accindex  = 0;accindex <= MAX_ACCURACY;accindex++)
    {
    if (MIN((int)(100 * accuracy),100) <= MStat.P.AccuracyStep[accindex])
      break;
    }

  if (MSched.Mode == msmSim)
    {
    /* adjust runtime statistics */
 
    MPolicyAdjustUsage(NULL,J,NULL,mlActive,NULL,-1,-1,NULL);
    }

  DBG(4,fSTAT) DPrint("INFO:     updating statistics for Grid[time: %d][proc: %d]\n",
    timeindex,
    procindex);

  /* determine statistics to update */

  stattotal = 0;

  S[stattotal++] = &MStat.Grid[timeindex][procindex];
  S[stattotal++] = &MStat.RTotal[procindex];
  S[stattotal++] = &MStat.CTotal[timeindex];
  S[stattotal++] = GS;

  if (RQ->PtIndex != 0)
    S[stattotal++] = &MPar[RQ->PtIndex].S;

  /* locate/adjust user stats */

  if (J->Cred.U != NULL)
    {
    DBG(7,fSTAT) DPrint("INFO:     updating statistics for UID %ld (user: %s)\n",
      J->Cred.U->OID,
      J->Cred.U->Name);

    S[stattotal++] = &J->Cred.U->Stat;
    }

  /* locate/adjust group stats */

  if (J->Cred.G != NULL)
    {
    DBG(7,fSTAT) DPrint("INFO:     updating statistics for GID %ld (group: %s)\n",
      J->Cred.G->OID,
      J->Cred.G->Name);

    S[stattotal++] = &J->Cred.G->Stat;
    }

  /* locate/adjust account stats */

  if (J->Cred.A != NULL)
    {
    DBG(7,fSTAT) DPrint("INFO:     updating statistics for account %s\n",
      J->Cred.A->Name);

    S[stattotal++] = &J->Cred.A->Stat;
    }

  if (J->Cred.Q != NULL)
    {
    DBG(7,fSTAT) DPrint("INFO:     updating statistics for QOS %s\n",
      J->Cred.Q->Name);
 
    S[stattotal++] = &J->Cred.Q->Stat;
    }

  if (J->Cred.C != NULL)
    {
    DBG(7,fSTAT) DPrint("INFO:     updating statistics for class %s\n",
      J->Cred.C->Name);
 
    S[stattotal++] = &J->Cred.C->Stat;
    }

  /* update all statistics */

  for (statindex = 0;statindex < stattotal;statindex++)
    {
    S[statindex]->Count++;
    S[statindex]->NCount    += TotalProcs;
    S[statindex]->JobCountSuccessful++;

    S[statindex]->TotalQTS  += queuetime;

    S[statindex]->TotalRequestTime += (double)request;
    S[statindex]->TotalRunTime     += (double)run;
    S[statindex]->PSRequest        += psrequest;
    S[statindex]->PSRun            += psrun;
    S[statindex]->PSRunSuccessful  += psrun;

    S[statindex]->JobAcc    += accuracy;
    S[statindex]->NJobAcc   += accuracy * psrun;

    S[statindex]->Accuracy[accindex]++;

    S[statindex]->XFactor   += xfactor;
    S[statindex]->NXFactor  += xfactor * TotalProcs;
    S[statindex]->PSXFactor += xfactor * psrun;

    S[statindex]->Bypass    += J->Bypass;

    if (QOSMet == TRUE)
      {
      S[statindex]->QOSMet++;
      }

    if (RunMode == msmProfile)
      {
      S[statindex]->PSDedicated += J->PSDedicated;
      S[statindex]->PSUtilized  += J->PSUtilized;

      S[statindex]->MSDedicated += J->MSDedicated;
      S[statindex]->MSUtilized  += J->MSUtilized;
      }

    if (J->Flags & (1 << mjfBackfill))
      {
      S[statindex]->BFCount++;
      S[statindex]->BFPSRun += psrun;
      }

    S[statindex]->MaxQTS     = MAX(S[statindex]->MaxQTS,queuetime);
    S[statindex]->MaxXFactor = MAX(S[statindex]->MaxXFactor,xfactor);
    S[statindex]->MaxBypass  = MAX(S[statindex]->MaxBypass,J->Bypass);

    if (MUDStatIsEnabled(&S[statindex]->DStat[dstatWCA]))
      {
      MUDStatAdd(&S[statindex]->DStat[dstatWCA],(char *)&accuracy);
      }
    }  /* END for (statindex) */

  DBG(2,fSTAT) DPrint("INFO:     job '%s' completed  X: %lf  T: %ld  PS: %d  A: %lf\n",
    J->Name,
    xfactor,
    run,
    psrun,
    accuracy);

  /* update consumption statistics */

  MStat.SuccessfulPH += (double)psrun / 36.0;

  if ((J->State == mjsCompleted) || (run >= (int)J->WCLimit))
    MStat.SuccessfulJobsCompleted++;

  if (MSched.Mode == msmSim)
    {
    cost = MStatCalcCommunicationCost(J);

    DBG(4,fSTAT) DPrint("INFO:     comcost:  job '%18s' procs: %3d  comcost: %7.3f  time: %5ld  com: %7.3f  proc: %7.3f\n",
      J->Name,
      TotalProcs,
      cost,
      run,
      (cost * run * MSim.ComRate / 3600),
      (double)psrun / 3600);

    MStat.TotalSimComCost += (cost * run * MSim.ComRate);
    }

  return(SUCCESS);
  }  /* END MStatUpdateCompletedJobUsage() */





int MStatUpdateRejectedJobUsage(

  mjob_t *J,
  int     ProfileMode)

  {
  int   timeindex;
  int   procindex;

  int   stattotal;
  int   statindex;

  int   rval;
  int   TotalProcs;

  mreq_t *RQ;

  must_t  *S[10];

  const char *FName = "MStatUpdateRejectedJobUsage";

  DBG(5,fSTAT) DPrint("%s(%s,%x)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    ProfileMode);

  if (J == NULL)
    {
    return(FAILURE);
    }

  DBG(1,fSTAT) DPrint("INFO:     job '%s' rejected\n",
    J->Name);

  RQ = J->Req[0];  /* FIXME */

  /* add statistics to grid */

  if (ProfileMode & (1 << mUseRunTime))
    {
    rval = (J->CompletionTime > J->StartTime) ?
      J->CompletionTime - J->StartTime : 0;
    }
  else
    {
    rval = J->WCLimit;
    }

  rval = MIN(rval,MStat.P.MaxTime);

  TotalProcs = MJobGetProcCount(J);

  for (timeindex = 0;rval > MStat.P.TimeStep[timeindex];timeindex++);
  for (procindex = 0;MIN(TotalProcs,MStat.P.MaxNode) > MStat.P.NodeStep[procindex];procindex++);

  timeindex = MIN(timeindex,MStat.P.TimeStepCount - 1);

  /* determine statistics to update */

  stattotal = 0;

  S[stattotal++] = &MStat.Grid[timeindex][procindex];
  S[stattotal++] = &MStat.RTotal[procindex];
  S[stattotal++] = &MStat.CTotal[timeindex];
  S[stattotal++] = &MPar[0].S;

  if (RQ->PtIndex != 0)
    S[stattotal++] = &MPar[RQ->PtIndex].S;

  /* locate cred stats */

  if (J->Cred.U != NULL)
    {
    S[stattotal++] = &J->Cred.U->Stat;
    }

  if (J->Cred.G != NULL)
    {
    S[stattotal++] = &J->Cred.G->Stat;
    }

  if (J->Cred.A != NULL)
    {
    S[stattotal++] = &J->Cred.A->Stat;
    }

  if (J->Cred.Q != NULL)
    {
    S[stattotal++] = &J->Cred.Q->Stat;
    }

  /* locate/adjust user/group/account stats */

  DBG(7,fSTAT) DPrint("INFO:     updating statistics (user %s  group %s  account %s)\n",
    J->Cred.U->Name,
    J->Cred.G->Name,
    (J->Cred.A != NULL) ? J->Cred.A->Name : "NONE");

  /* update all statistics */

  for (statindex = 0;statindex < stattotal;statindex++)
    {
    S[statindex]->RejectionCount++;
    }  /* END for (statindex) */

  return(SUCCESS);
  }  /* END MStatUpdateRejectedJobUsage() */





int MStatUpdateSubmitJobUsage(

  mjob_t *J)

  {
  must_t  *S[MAX_STATISTICS];

  int procindex;
  int timeindex;

  int stattotal;
  int statindex;

  int psrequest;
  
  unsigned long TotalProcs;

  const char *FName = "MStatUpdateSubmitJobUsage";

  DBG(5,fSTAT) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if (J == NULL)
    return(SUCCESS);

  TotalProcs = MJobGetProcCount(J);

  psrequest = J->WCLimit * TotalProcs;
  
  for (timeindex = 0;MIN(J->WCLimit,MStat.P.MaxTime) > MStat.P.TimeStep[timeindex];timeindex++);

  for (procindex = 0;MIN(TotalProcs,MStat.P.MaxNode) > MStat.P.NodeStep[procindex];procindex++);

  DBG(7,fSTAT) DPrint("INFO:     updating submit statistics for Grid[time: %d][proc: %d]\n",
    timeindex,
    procindex);

  /* determine statistics to update */

  stattotal = 0;

  S[stattotal++] = &MStat.Grid[timeindex][procindex];
  S[stattotal++] = &MStat.RTotal[procindex];
  S[stattotal++] = &MStat.CTotal[timeindex];
  S[stattotal++] = &MPar[0].S;

  /* adjust user/group/account stats */

  DBG(7,fSTAT) DPrint("INFO:     updating statistics (user %s  group %s  account %s)\n",
    J->Cred.U->Name,
    J->Cred.G->Name,
    (J->Cred.A != NULL) ? J->Cred.A->Name : "NONE");

  S[stattotal++] = &J->Cred.U->Stat;
  S[stattotal++] = &J->Cred.G->Stat;

  if (J->Cred.A != NULL)
    S[stattotal++] = &J->Cred.A->Stat;

  /* update all statistics */

  for (statindex = 0;statindex < stattotal;statindex++)
    {
    S[statindex]->JobCountSubmitted++;
    S[statindex]->PSRequestSubmitted += psrequest;
    }  /* END for (statindex) */

  return(SUCCESS);
  }  /* END MStatUpdateSubmitJobUsage() */




int MStatAddEJob(

  mjob_t *J)

  {
  MStat.EligibleJobs++;

  MPolicyAdjustUsage(NULL,J,NULL,mlIdle,MPar[0].L.IP,-1,1,NULL);
  MPolicyAdjustUsage(NULL,J,NULL,mlIdle,NULL,-1,1,NULL);

  return(SUCCESS);
  }  /* END MStatAddEJob() */




int MStatRemoveEJob(

  mjob_t *J)

  {
  MStat.IdleJobs--;
  MStat.EligibleJobs--;
 
  MPolicyAdjustUsage(NULL,J,NULL,mlIdle,MPar[0].L.IP,-1,-1,NULL);
  MPolicyAdjustUsage(NULL,J,NULL,mlIdle,NULL,-1,-1,NULL);
 
  return(SUCCESS);
  }  /* END MStatRemoveEJob() */




int MStatUpdateBFUsage(
 
  mjob_t *J)
 
  {
  int jindex;

  const char *FName = "MStatUpdateBFUsage";
 
  DBG(7,fSCHED) DPrint("%s(%s)\n",
    FName,
    (J != NULL) ? J->Name : "NULL");

  if (J == NULL)
    return(FAILURE);
 
  for (jindex = 0;MFQ[jindex] != -1;jindex++)
    {
    if (J->Index == MFQ[jindex])
      break;
 
    if ((MJob[MFQ[jindex]] != NULL) &&
        (MJob[MFQ[jindex]]->State == mjsIdle))
      {
      MJob[MFQ[jindex]]->Bypass++;
      }
    }  /* END for (jindex) */
 
  return(SUCCESS);
  }  /* END MStatUpdateBFUsage() */



int MStatShutdown()

  {
  if (MSched.statfp != NULL)
    {
    fclose(MSched.statfp);
 
    MSched.statfp = NULL;
    }

  return(SUCCESS);
  }  /* END MStatShutdown() */





char *MSysToString(

  msched_t *S,    /* I */
  char     *Buf,  /* O */
  int       IsCP) /* I (boolean) */

  {
  const int CPAList[] = {
    msysaStatInitTime,
    msysaVersion,
    msysaSyncTime,
    -1 };

  const int AList[] = {
    msysaPresentTime,
    msysaStatInitTime,
    msysaVersion,
    msysaSyncTime,
    -1 };

  const int CPDList[] = {
    -1 };

  mxml_t *E = NULL;

  if ((S == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  Buf[0] = '\0';

  MSysToXML(
    S,
    &E,
    (IsCP == TRUE) ? (int *)CPAList : (int *)AList,
    (IsCP == TRUE) ? (int *)CPDList : NULL,
    0);

  MXMLToString(E,Buf,MAX_MBUFFER,NULL,TRUE);
 
  MXMLDestroyE(&E);
 
  return(Buf);
  }  /* END MSysToString() */





int MSysToXML(

  msched_t  *S,      /* I */
  mxml_t  **EP,     /* O */
  int       *SAList, /* I */
  int       *SCList, /* I */
  int        Mode)   /* I */

  {
  int *AList;
  int *CList;

  int  aindex;
  int  cindex;

  void *C;

  mxml_t *XC = NULL;

  char tmpString[MAX_MLINE];

  int DAList[] = {
    msysaSyncTime,
    msysaVersion,
    -1 };

  int DCList[] = {
    mxoStats,
    mxoLimits,
    mxoFS,
    -1 };

    if (SAList != NULL)
    AList = SAList;
  else
    AList = (int *)DAList;
 
  if (SCList != NULL)
    CList = SCList;
  else
    CList = (int *)DCList;
 
  MXMLCreateE(EP,(char *)MXO[mxoSys]);
 
  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    if ((MSysAToString(S,AList[aindex],tmpString,0) == FAILURE) ||
        (tmpString[0] == '\0'))
      {
      continue;
      }
 
    MXMLSetAttr(*EP,(char *)MSysAttr[AList[aindex]],tmpString,mdfString);
    }  /* END for (aindex) */
 
  for (cindex = 0;CList[cindex] != -1;cindex++)
    {
    XC = NULL;
 
    if ((MOGetComponent((void *)S,mxoSys,&C,CList[cindex]) == FAILURE) ||
        (MOToXML(C,CList[cindex],&XC) == FAILURE))
      {
      continue;
      }
 
    MXMLAddE(*EP,XC);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MSysToXML() */




int MSysAToString(

  msched_t *S,      /* I */
  int       AIndex, /* I */
  char     *Buf,    /* O */
  int       Format) /* I */
 
  {
  if ((S == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }
 
  switch(AIndex)
    {
    case msysaPresentTime:

      sprintf(Buf,"%ld",
        MSched.Time);

      break;

    case msysaStatInitTime:

      sprintf(Buf,"%ld",
        MStat.InitTime);

      break;

    case msysaSyncTime:

      sprintf(Buf,"%ld",
        S->Sync.UpdateTime);

      break;

    case msysaVersion:

      sprintf(Buf,"%s",
        S->Version);

      break;

    default:

      /* NOT HANDLED */

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */
  
  return(SUCCESS);
  }  /* END MSysAToString() */




int MSysSetAttr(

  msched_t  *S,      /* I (modified) */
  int        AIndex, /* I */
  void     **Value,  /* I */
  int        Format, /* I */
  int        Mode)   /* I */
 
  {
  if (S == NULL)
    {
    return(FAILURE);
    }

  switch(AIndex)
    {
    case msysaPresentTime:

      S->Time = strtol((char *)Value,NULL,0);

      break;

    case msysaStatInitTime:

      MStat.InitTime = strtol((char *)Value,NULL,0);      

      break;

    case msysaSyncTime:

      S->Sync.UpdateTime = strtol((char *)Value,NULL,0);

      break;

    case msysaVersion:

      /* NOTE:  checkpoint version ignored.  use built-in version only */

      /* NO-OP */

      break;

    default:

      /* NOT HANDLED */

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* END switch(AIndex) */

  return(SUCCESS);
  }  /* END MSysSetAttr() */





int MLimitToXML(

  mcredl_t  *L,      /* I */
  mxml_t  **EP,     /* O */
  int       *SAList) /* I (optional) */

  {
  int DAList[] = {
    mlaAJobs,
    mlaAProcs,
    mlaAPS,
    -1 };

  int  aindex;
 
  int *AList;
 
  char tmpString[MAX_MLINE];
 
  if ((L == NULL) || (EP == NULL))
    {
    return(FAILURE);
    }
 
  if (SAList != NULL)
    AList = SAList;
  else
    AList = DAList;
 
  MXMLCreateE(EP,(char *)MXO[mxoLimits]);
 
  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    if ((MLimitAToString(L,AList[aindex],tmpString,0) == FAILURE) ||
        (tmpString[0] == '\0'))
      {
      continue;
      }
 
    MXMLSetAttr(*EP,(char *)MLimitAttr[AList[aindex]],tmpString,mdfString);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MLimitToXML() */




int MLimitAToString(

  mcredl_t *L,
  int       AIndex,
  char     *Buf,
  int       Format)

  {
  if ((L == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }
 
  switch(AIndex)
    {
    case mlaAJobs:
 
      sprintf(Buf,"%d",
        L->AP.Usage[mptMaxJob][0]);
 
      break;
 
    case mlaAProcs:
 
      sprintf(Buf,"%d",
        L->AP.Usage[mptMaxProc][0]);
 
      break;
 
    case mlaAPS:
 
      sprintf(Buf,"%d",
        L->AP.Usage[mptMaxPS][0]);
 
      break;

    default:

      /* NOT HANDLED */

      return(FAILURE);

      /*NOTREACHED*/
 
      break;
    }  /* END switch(AIndex) */
 
  return(SUCCESS);
  }  /* END MLimitAToString() */




int MLimitToString(
 
  mcredl_t *L,
  char     *Buf)
 
  {
  /* NYI */

  return(SUCCESS);
  }  /* END MLimitToString() */
 
 
 
 
int MLimitFromXML(
 
  mcredl_t *L,
  mxml_t  *E)
 
  {
  int aindex;
  int saindex;
 
  if ((L == NULL) || (E == NULL))
    return(FAILURE);
 
  /* NOTE:  do not initialize.  may be overlaying data */
 
  for (aindex = 0;aindex < E->ACount;aindex++)
    {
    saindex = MUGetIndex(E->AName[aindex],MLimitAttr,FALSE,0);
 
    if (saindex == 0)
      continue;
 
    MLimitSetAttr(L,saindex,(void **)E->AVal[aindex],mdfString,mSet);
    }  /* END for (aindex) */
 
  return(SUCCESS);
  }  /* END MStatFromXML() */

 


int MLimitSetAttr(
 
  mcredl_t  *L,
  int        AIndex,
  void     **Value,
  int        Format,
  int        Mode)
 
  {
  if (L == NULL)
    return(FAILURE);
 
  switch(AIndex)
    {
    case mlaAJobs:
 
      L->AP.Usage[mptMaxJob][0] = (int)strtol((char *)Value,NULL,0);
 
      break;

    case mlaAProcs:

      L->AP.Usage[mptMaxProc][0] = (int)strtol((char *)Value,NULL,0);        
 
      break;
 
    case mlaAPS:

      L->AP.Usage[mptMaxPS][0] = (int)strtol((char *)Value,NULL,0);     
 
      break;

    default:

      /* not handled */

      return(FAILURE);

      /*NOTREACHED*/

      break;
    }  /* switch(AIndex) */

  return(SUCCESS);
  }  /* MLimitSetAttr() */





int MStatToXML(

  must_t   *S,      /* I */
  mxml_t **EP,     /* O (allocated) */
  int      *SAList) /* I (optional) */

  {
  int DAList[] = {
    mstaTJobCount,
    mstaTNJobCount,
    mstaTQueueTime,
    mstaMQueueTime,
    mstaTReqWTime,
    mstaTExeWTime,
    mstaTMSAvl,
    mstaTMSDed,
    mstaTPSReq,
    mstaTPSExe,
    mstaTPSDed,
    mstaTPSUtl,
    mstaTJobAcc,
    mstaTNJobAcc,
    mstaTXF,
    mstaTNXF,
    mstaMXF,
    mstaTBypass,
    mstaMBypass,
    mstaTQOSAchieved,
    mstaInitTime,
    -1 };

  int  aindex;
 
  int *AList;
 
  char tmpString[MAX_MLINE];
 
  if ((S == NULL) || (EP == NULL))
    {
    return(FAILURE);
    }
 
  if (SAList != NULL)
    AList = SAList;
  else
    AList = DAList;
 
  MXMLCreateE(EP,(char *)MXO[mxoStats]);
 
  for (aindex = 0;AList[aindex] != -1;aindex++)
    {
    if ((MStatAToString(S,AList[aindex],tmpString,0) == FAILURE) ||
        (tmpString[0] == '\0'))
      {
      continue;
      }
 
    MXMLSetAttr(*EP,(char *)MStatAttr[AList[aindex]],tmpString,mdfString);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MStatToXML() */





int MStatAToString(
 
  must_t *S,      /* I */
  int     AIndex, /* I */
  char   *Buf,    /* O */
  int     Format) /* I */
 
  {
  if ((S == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }
 
  switch(AIndex)
    {
    case mstaTJobCount:
 
      sprintf(Buf,"%d",
        S->Count);
 
      break;
 
    case mstaTNJobCount:
 
      sprintf(Buf,"%d",
        S->NCount); 
 
      break;
 
    case mstaTQueueTime:
 
      sprintf(Buf,"%ld",
        S->TotalQTS);
 
      break;
 
    case mstaMQueueTime:
 
      sprintf(Buf,"%ld",
        S->MaxQTS);
 
      break;
 
    case mstaTReqWTime:
 
      sprintf(Buf,"%.3lf",
        S->TotalRequestTime);
 
      break;
 
    case mstaTExeWTime:
 
      sprintf(Buf,"%.3lf",
        S->TotalRunTime);
 
      break;

    case mstaTMSAvl:

      sprintf(Buf,"%.3lf",
        S->MSAvail);

      break;

    case mstaTMSDed:

      sprintf(Buf,"%.3lf",
        S->MSDedicated);

      break;
 
    case mstaTPSReq:
 
      sprintf(Buf,"%.3lf",
        S->PSRequest);
 
      break; 
 
    case mstaTPSExe:
 
      sprintf(Buf,"%.3lf",
        S->PSRun);
 
      break;
 
    case mstaTPSDed:
 
      sprintf(Buf,"%.3lf",
        S->PSDedicated);
 
      break;
 
    case mstaTPSUtl:
 
      sprintf(Buf,"%.3lf",
        S->PSUtilized);
 
      break;
 
    case mstaTJobAcc:
 
      sprintf(Buf,"%.3lf",
        S->JobAcc);
 
      break;
 
    case mstaTNJobAcc:
 
      sprintf(Buf,"%.3lf",
        S->NJobAcc);
 
      break;
 
    case mstaTXF: 
 
      sprintf(Buf,"%.3lf",
        S->XFactor);
 
      break;
 
    case mstaTNXF:
 
      sprintf(Buf,"%.3lf",
        S->NXFactor);
 
      break;
 
    case mstaMXF:
 
      sprintf(Buf,"%.3lf",
        S->MaxXFactor);
 
      break;
 
    case mstaTBypass:
 
      sprintf(Buf,"%d",
        S->Bypass);
 
      break;
 
    case mstaMBypass:
 
      sprintf(Buf,"%d",
        S->MaxBypass);
 
      break;
 
    case mstaTQOSAchieved:
 
      sprintf(Buf,"%d", 
        S->QOSMet);
 
      break;

    case mstaInitTime:

      sprintf(Buf,"%ld",
        MStat.InitTime);

      break;

    case mstaGCEJobs:      /* current eligible jobs */

      sprintf(Buf,"%d",
        MStat.EligibleJobs);

      break;

    case mstaGCIJobs:      /* current idle jobs */

      sprintf(Buf,"%d",
        MStat.IdleJobs);

      break;

    case mstaGCAJobs:      /* current active jobs */

      sprintf(Buf,"%d",
        MStat.ActiveJobs);

      break;

    case mstaGPHAvl:

      sprintf(Buf,"%lf",
        MStat.TotalPHAvailable);

      break;

    case mstaGPHUtl:

      sprintf(Buf,"%lf",
        MStat.TotalPHBusy);

      break;

    case mstaGPHSuc:

      sprintf(Buf,"%lf",
        MStat.SuccessfulPH);

      break;

    case mstaGMinEff:

      sprintf(Buf,"%lf",
        MStat.MinEff);

      break;

    case mstaGMinEffIteration:

      sprintf(Buf,"%d",
        MStat.MinEffIteration);

      break;

    case mstaTPHPreempt:

      sprintf(Buf,"%lf",
        MStat.TotalPHPreempted);

      break;

    case mstaTJPreempt:

      sprintf(Buf,"%d",
        MStat.TotalPreemptJobs);

      break;

    case mstaTJEval:

      sprintf(Buf,"%d",
        MStat.JobsEvaluated);

      break;

    case mstaTPHQueued:

      sprintf(Buf,"%lu",
        MStat.AvgQueuePH);

      break;

    case mstaSchedDuration:

      sprintf(Buf,"%ld",
        MStat.SchedRunTime);

      break;

    case mstaSchedCount:

      sprintf(Buf,"%d",
        MSched.Iteration);

      break;

    default:

      /* not handled */

      Buf[0] = '\0';

      return(FAILURE);

      /*NOTREACHED*/
 
      break;
    }  /* END switch(AIndex) */
 
  return(SUCCESS);
  }  /* END MStatAToString() */




int MStatToString(

  must_t  *S,      /* I */
  char    *Buf,    /* O */
  int     *SAList) /* I (optional) */

  {
  mxml_t *E = NULL;

  if ((S == NULL) || (Buf == NULL))
    {
    return(FAILURE);
    }

  MStatToXML(S,&E,(int *)SAList);

  MXMLToString(E,Buf,MAX_MBUFFER,NULL,TRUE);

  MXMLDestroyE(&E);

  return(SUCCESS);
  }  /* END MStatToString() */




int MStatFromString(

  char   *Buf, /* I */
  must_t *S)   /* I (modified) */

  {
  mxml_t *E = NULL;

  int rc;

  if ((Buf == NULL) || (S == NULL))
    {
    return(FAILURE);
    }

  rc = MXMLFromString(&E,Buf,NULL,NULL);

  if (rc == SUCCESS)
    {
    rc = MStatFromXML(S,E);
    }

  MXMLDestroyE(&E);

  if (rc == FAILURE)
    { 
    return(FAILURE);
    }
  
  return(SUCCESS);
  }  /* END MStatFromString() */




int MStatFromXML(

  must_t  *S,
  mxml_t *E)

  {
  int aindex;
  int saindex;
 
  if ((S == NULL) || (E == NULL))
    {
    return(FAILURE);
    }
 
  /* NOTE:  do not initialize.  may be overlaying data */
 
  for (aindex = 0;aindex < E->ACount;aindex++)
    {
    saindex = MUGetIndex(E->AName[aindex],MStatAttr,FALSE,0);
 
    if (saindex == 0)
      continue;
 
    MStatSetAttr(S,saindex,(void **)E->AVal[aindex],mdfString,mSet);
    }  /* END for (aindex) */

  return(SUCCESS);
  }  /* END MStatFromXML() */





int MStatSetAttr(

  must_t  *S,      /* I (modified) */
  int      AIndex, /* I */
  void   **Value,  /* I */
  int      Format, /* I */
  int      Mode)   /* I */
 
  {
  if (S == NULL)
    {
    return(FAILURE);
    }

  switch(AIndex) 
    {
    case mstaTJobCount:
 
      S->Count = (int)strtol((char *)Value,NULL,0);
 
      break;
 
    case mstaTNJobCount:
 
      S->NCount = (int)strtol((char *)Value,NULL,0);
 
      break;
 
    case mstaTQueueTime:
 
      S->TotalQTS = strtol((char *)Value,NULL,0);
 
      break;
 
    case mstaMQueueTime:
 
      S->MaxQTS = strtol((char *)Value,NULL,0);
 
      break;
 
    case mstaTReqWTime:
 
      S->TotalRequestTime = strtod((char *)Value,NULL);
 
      break;

    case mstaTExeWTime:
 
      S->TotalRunTime = strtod((char *)Value,NULL);
 
      break;

    case mstaTMSAvl:

      S->MSAvail = strtod((char *)Value,NULL);  

      break;

    case mstaTMSDed:

      S->MSDedicated = strtod((char *)Value,NULL);
 
      break;
      
    case mstaTPSReq:
 
      S->PSRequest = strtod((char *)Value,NULL);
 
      break;
 
    case mstaTPSExe:
 
      S->PSRun = strtod((char *)Value,NULL);
 
      break;
 
    case mstaTPSDed:
 
      S->PSDedicated = strtod((char *)Value,NULL);
 
      break;
 
    case mstaTPSUtl:
 
      S->PSUtilized = strtod((char *)Value,NULL);
 
      break;
 
    case mstaTJobAcc:
 
      S->JobAcc = strtod((char *)Value,NULL);
 
      break;
 
    case mstaTNJobAcc: 
 
      S->NJobAcc = strtod((char *)Value,NULL);
 
      break;
 
    case mstaTXF:
 
      S->XFactor = strtod((char *)Value,NULL);
 
      break;
 
    case mstaTNXF:
 
      S->NXFactor = strtod((char *)Value,NULL);
 
      break;
 
    case mstaMXF:
 
      S->NXFactor = strtod((char *)Value,NULL);
 
      break;
 
    case mstaTBypass:
 
      S->Bypass = (int)strtol((char *)Value,NULL,0);
 
      break;
 
    case mstaMBypass:
 
      S->MaxBypass = (int)strtol((char *)Value,NULL,0);
 
      break;
 
    case mstaTQOSAchieved: 
 
      S->QOSMet = (int)strtol((char *)Value,NULL,0);
 
      break;

    case mstaInitTime:

      /* NYI */

      break;
 
    default:

      /* not handled */
  
      return(FAILURE);

      /*NOTREACHED*/
 
      break;
    }  /* END switch(AIndex) */
 
  return(SUCCESS);
  }  /* END MStatSetAttr() */

 
/* END MStats.c */


