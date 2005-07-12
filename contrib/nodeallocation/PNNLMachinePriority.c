/* PNNLMachinePriority.c */

typedef struct {
  char *Name;
  int   Weight;
  } CPUType_t;


int ContribPNNLGetMachinePriority(

  job_t  *J,
  NodeSt *N)
  
  {
  int CPUSpeedWeight = 1000;
  int CPUCountWeight = 2000;
  int MemoryWeight   = 20;
  int SwapWeight     = 10;

  int CPUSpeed;
  int CPUTypeWeight;

  int index;

  int NodePriority;
  int FIndex;

  const CPUType_t CPUType[] = {
    "ii",   10,
    "iii",  100,
    NULL,   0};

  /* UofU Node Priority */

  /* Priority = CPUTYPEWEIGHT[CPUTYPE]    +
                CPUCOUNTWEIGHT * CPUCOUNT +
                MEMORYWEIGHT   * MEMORY   +
                SWAPWEIGHT     * SWAP
  */

  /* obtain cpu type via node feature 'p<CPUTYPE>'   */
  /* obtain cpu speed via node feature 's<CPUSPEED>' */

  DBG(8,fSCHED) DPrint("LocalGetNodePriority(%s,%s)\n",
    J->Name,
    N->Name);

  /* prioritize nodes for allocation */
  /* highest priority node is allocated first */

  /* get feature information */

  CPUSpeed      = 0;
  CPUTypeWeight = 0;

  for (FIndex = 0;FIndex < 32;FIndex++)
    {
    /* FIXME: FeatureMap usage */

    if ((N->FeatureMap[0] & (1 << FIndex)) == 0)
      continue;

    if ((AttrList[eFeature][FIndex][0] == 's') &&
        (isdigit(AttrList[eFeature][FIndex][1])))
      {
      CPUSpeed = strtol(&AttrList[eFeature][FIndex][2],NULL,0);
      }
    else if (AttrList[eFeature][FIndex][0] == 'p')
      {
      for (index = 0;CPUType[index].Name != NULL;index++)
        {
        if (!strcmp(&AttrList[eFeature][FIndex][1],CPUType[index].Name))
          {
          CPUTypeWeight = CPUType[index].Weight;

          break;
          }
        }
      }
    }

  NodePriority = CPUTypeWeight +
                 CPUSpeedWeight * CPUSpeed      +
                 CPUCountWeight * N->CRes.Procs +
                 MemoryWeight   * N->CRes.Mem   +
                 SwapWeight     * N->CRes.Swap;

  return(NodePriority);
  }  /* END ContribPNNLGetMachinePriority() */

/* END */
