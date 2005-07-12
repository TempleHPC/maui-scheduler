/* CONTRIB:  OSCProximityNodeAlloc.c */
 
/* code to be included in LocalJobAllocResources() in Local.c */

int ContribOSCProximityNodeAlloc(

  mjob_t *J,                /* IN: job allocating nodes                           */
  mreq_t *RQ,               /* IN: req allocating nodes                           */    
  nodealloc_t NodeList[],   /* IN: eligible nodes                                 */
  int     RQIndex,          /* IN: index of job req to evaluate                   */     
  int     MinTPN[],         /* IN: min tasks per node allowed                     */
  int     MaxTPN[],         /* IN: max tasks per node allowed                     */             
  char    NodeMap[],        /* IN: array of node alloc states                     */
  int     AffinityLevel,    /* IN: current reservation affinity level to evaluate */
  int     NodeIndex[],      /* IN/OUT: index of next node to find in BestList     */
  nodealloc_t BestList[MAX_REQ_PER_FRAG][], /* IN/OUT: list of selected nodes     */
  int     TaskCount[],      /* IN/OUT: total tasks allocated to req               */
  int     NodeCount[])      /* IN/OUT: total nodes allocated to req               */         

  {
  int NIndex;
  int nindex;
  int TC;

  mnode_t *N;

  nodealloc_t MyNodeList[MAX_MNODE];
  int         MyNIndex;

  /* select first 'RQ->TaskCount' procs */

  MyNIndex = 0;
 
  for (nindex = 0;NodeList[nindex].nodeindex != -1;nindex++)
    {
    NIndex = NodeList[nindex].nodeindex;
    TC     = NodeList[nindex].taskcount;         
 
    if (NodeMap[NIndex] != AffinityLevel)
      {
      /* node unavailable */
 
      continue;
      }
 
    if (TC < MinTPN[RQIndex])
      continue;
 
    TC = MIN(TC,MaxTPN[RQIndex]);

    N = &MNode[NIndex];

    /* determine node locality */

    /* NOT IMPLEMENTED */      

    /* determine node time availability */

    /* NOT IMPLEMENTED */      

    /* add node to private list */

    MyNodeList[MyNIndex].taskcount = TC;
    MyNodeList[MyNIndex].nodeindex = NIndex;
    }  /* END for (nindex) */

  /* select best nodes */

  /* NOT IMPLEMENTED */               

  /* populate BestList with selected nodes */

  for (nindex = 0;MyNodeList[nindex].nodeindex != -1;nindex++)
    {
    NIndex = MyNodeList[nindex].nodeindex;
    TC     = MyNodeList[nindex].taskcount; 

    BestList[RQIndex][NodeIndex[RQIndex]].nodeindex = NIndex;
    BestList[RQIndex][NodeIndex[RQIndex]].taskcount = TC;
 
    NodeIndex[RQIndex] ++;
    TaskCount[RQIndex] += TC;
    NodeCount[RQIndex] ++;
 
    /* mark node as used */
 
    NodeMap[NIndex] = nmUnavailable;
 
    if (TaskCount[RQIndex] >= RQ->TaskCount)
      {
      /* all required tasks found */ 
 
      /* NOTE:  HANDLED BY DIST */
 
      if ((RQ->NodeCount == 0) ||
          (NodeCount[RQIndex] >= RQ->NodeCount))
        {
        /* terminate BestList */
 
        BestList[RQIndex][NodeIndex[RQIndex]].nodeindex = -1;
 
        break;
        }
      }
    }     /* END for (nindex) */

  return(SUCCESS);
  }  /* END ContribOSCProximityNodeAlloc() */

/* END OSCProximityNodeAlloc.c */


