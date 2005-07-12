/* HEADER */
        
/* Contains:                                               *
 *                                                         *
 *  int MLocalJobInit(J)                                   *
 *  int MLocalNodeInit(N)                                  *
 *  int MLocalJobCheckNRes(J,N,Time)                       *
 *  int MLocalCheckFairnessPolicy(J,StartTime,Message)     *
 *  int MLocalGetNodePriority(J,N)                         *
 *  int MLocalInitialize()                                 *
 *  int MLocalJobDistributeTasks(J,R,NodeList,TaskMap)     *
 *  int MLocalQueueScheduleIJobs(J)                        *
 *                                                         */

  
#include "moab.h"
#include "msched-proto.h"  

extern msched_t     MSched;
extern mnode_t     *MNode[];
extern mstat_t      MStat;
extern mattrlist_t  MAList;
extern mpar_t       MPar[];
extern mjob_t      *MJob[];
extern mlog_t       mlog;




/* #include "../../contrib/checkreq/AussieJobCheckNRes.c" */

int MLocalJobCheckNRes(

  mjob_t  *J,         /* I */
  mnode_t *N,         /* I */
  long     StartTime) /* I */

  {
  const char *FName = "MLocalJobCheckNRes";

  int rc = SUCCESS;

  DBG(8,fSCHED) DPrint("%s(%s,%s,%ld)\n",
    FName,
    J->Name,
    N->Name,
    StartTime);

/*
  rc = ContribAussieJobCheckNRes(J,N,StartTime);
*/

  return(rc);
  }  /* END MLocalJobCheckNRes */




/* #include "../../contrib/jobinit/AussieJobInit.c" */

int MLocalJobInit(

  mjob_t *J)  /* I */

  {
/*
  ContribAussieJobInit(J); 
*/

  return(SUCCESS);
  }  /* END MLocalJobInitialize() */




int MLocalNodeInit(

  mnode_t *N)

  {
  return(SUCCESS);
  }  /* END MLocalNodeInit() */




/* #include "../../contrib/fairness/JobLength.c" */
/* #include "../../contrib/fairness/ASCBackgroundJobPolicy.c" */

int MLocalCheckFairnessPolicy(

  mjob_t *J,
  long    StartTime,
  char   *Message)

  {
  const char *FName = "MLocalCheckFairnessPolicy";

  DBG(6,fSCHED) DPrint("%s(%s,%ld,Message)\n",
    FName,
    (J != NULL) ? J->Name : "NULL",
    (unsigned long)StartTime);

/*
  if (ContribJobLengthFairnessPolicy(J,StartTime,Message) == FAILURE)
    {
    return(FAILURE);
    }
*/

/*
  if (ContribASCBackgroundJobPolicy(J,StartTime,Message) == FAILURE)
    {
    return(FAILURE);
    }
*/

  /* all local policies passed */

  return(SUCCESS);
  }  /* END MLocalFairnessPolicy() */





/* #include "../../contrib/nodeallocation/PNNLGetNodePriority.c" */

int MLocalGetNodePriority(

  mjob_t  *J,
  mnode_t *N)

  {
  int Priority = 0;

/*
  Priority = ContribPNNLGetNodePriority(J,N));
*/

  return(Priority);
  }  /* END MLocalGetNodePriority() */





/* #include "../../contrib/nodeallocation/OSCProximityNodeAlloc.c" */
 
int MLocalJobAllocateResources(
 
  mjob_t *J,                /* I:  job allocating nodes                           */
  mreq_t *RQ,               /* I:  req allocating nodes                           */        
  mnalloc_t NodeList[],     /* I:  eligible nodes                                 */
  mulong  StartTime,        /* I                                                  */
  int     RQIndex,          /* I:  index of job req to evaluate                   */
  int     MinTPN[],         /* I:  min tasks per node allowed                     */
  int     MaxTPN[],         /* I:  max tasks per node allowed                     */
  char    NodeMap[],        /* I:  array of node alloc states                     */
  int     AffinityLevel,    /* I:  current reservation affinity level to evaluate */
  int     NodeIndex[],      /* I/OUT:  index of next node to find in BestList     */
  mnalloc_t *BestList[MAX_MREQ_PER_JOB], /* I/OUT:    list of selected nodes      */
  int     TaskCount[],      /* I/OUT:  total tasks allocated to job req           */
  int     NodeCount[])      /* I/OUT:  total nodes allocated to job req           */
 
  {
/*
  if (ContribOSCProximityNodeAlloc(
        J,
        RQ,
        NodeList,
        RQIndex,
        MinTPN,
        MaxTPN,
        NodeMap,
        AffinityLevel,
        NodeIndex,
        BestList,
        TaskCount,
        NodeCount) == FAILURE)
    {
    return(FAILURE);
    }
*/
 
  return(FAILURE);  
  }  /* END MLocalJobAllocateResources() */




/* #include "../../contrib/appsim/SCHSMSim.c" */  

int MLocalInitialize()

  {
/*
  if (ContribSCHSMSimInitialize() == FAILURE)
    {
    return(FAILURE);
    }
*/

  return(SUCCESS);
  }  /* END MLocalInitialize() */




int MLocalJobDistributeTasks(
 
  mjob_t    *J,
  mrm_t     *R,
  mnalloc_t *NodeList, /* OUT: nodelist with taskcount information */
  short     *TaskMap)  /* OUT: task distribution list              */
 
  {
  /* NYI */

  return(FAILURE);
  }  /* END MLocalJobDistributeTasks() */




/* #include "../../contrib/sched/XXX.c" */

int MLocalQueueScheduleIJobs(

  int    *Q,
  mpar_t *P)

  {
  mjob_t *J;

  int     jindex;

  if ((Q == NULL) || (P == NULL))
    {
    return(FAILURE);
    }

  /* NOTE:  insert call to scheduling algorithm here */

  for (jindex = 0;Q[jindex] != -1;jindex++)
    {
    J = MJob[Q[jindex]];

    /* NYI */

    DBG(7,fSCHED) DPrint("INFO:     checking job '%s'\n",
      J->Name);
    }  /* END for (jindex) */
   
  return(FAILURE);
  }  /* END MLocalQueueScheduleIJobs() */



/* END MLocal.c */

