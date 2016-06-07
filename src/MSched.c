/* HEADER */

#include "moab.h"
#include "msched-proto.h"

extern mlog_t mlog;

extern msched_t MSched;
extern mnode_t *MNode[];
extern mstat_t MStat;
extern mattrlist_t MAList;
extern mpar_t MPar[];
extern mrm_t MRM[];
extern mam_t MAM[];
extern mqos_t MQOS[];
extern mjob_t *MJob[];
extern msim_t MSim;
extern msys_t MSystem;
extern mframe_t MFrame[];
extern mckpt_t MCP;
extern mres_t *MRes[];
extern m64_t M64;

extern int MAQ[];
extern int MUIQ[];
extern int MFQ[MAX_MJOB];

extern const char *MComp[];
extern const char *MPolicyMode[];
extern const char *MPolicyRejection[];
extern const char *MNAllocPolicy[];
extern const char *MAllocRejType[];
extern const char *MNodeState[];
extern const char *MNAccessPolicy[];
extern const char *MSchedAttr[];
extern const char *MSchedMode[];
extern const char *MPreemptPolicy[];
extern const char *MSockProtocol[];
extern const char *MCDisplayType[];
extern const char *MResCtlPolicy[];
extern const char *MWCVAction[];
extern const char *MAllocLocalityPolicy[];
extern const char *MTimePolicy[];
extern const char *MLogFacilityType[];
extern const char *MCredCfgParm[];

extern mx_t X;

#include "MClient.c"

int __MSchedQSortHLComp(

    mnalloc_t *A, /* I */
    mnalloc_t *B) /* I */

{
    /* order hi to low */

    return (B->TC - A->TC);
} /* END __MSchedQSortHLComp() */

int __MSchedQSortHLPComp(

    mnpri_t *A, /* I */
    mnpri_t *B) /* I */

{
    /* order hi to low */

    if (A->Prio > B->Prio) {
        return (-1);
    } else if (A->Prio < B->Prio) {
        return (1);
    }

    return (0);
} /* END __MSchedQSortHLPComp() */

int MJobAllocMNL(

    mjob_t *J,                           /* I: job requesting resources */
    mnodelist_t MFeasibleList,           /* I: feasible nodes           */
    char *NodeMap, mnodelist_t MOutList, /* O: allocated nodes          */
    int NAPolicy,                        /* I: node allocation policy   */
    time_t StartTime)                    /* I: time job must start      */

{
    int index;
    int rqindex;
    int rnindex;

    mreq_t *RQ;

    int nindex;
    int NIndex;
    int NTasks;

    static mnodelist_t MNodeList;

    int FeasibleNodeCount[MAX_MREQ_PER_JOB];
    int FeasibleTaskCount[MAX_MREQ_PER_JOB];

    int mem;

    int NodeIndex[MAX_MREQ_PER_JOB];
    int TaskCount[MAX_MREQ_PER_JOB];
    int NodeCount[MAX_MREQ_PER_JOB];

    int TotalAllocNodeCount;
    int TotalFeasibleNodeCount;

    int TotalAllocTaskCount;
    int TotalFeasibleTaskCount;

    mnalloc_t *NodeList;
    mnalloc_t *BestList[MAX_MREQ_PER_JOB];

    int tmpNAPolicy;

    int TC;

    int Delta;

    int MaxTPN[MAX_MREQ_PER_JOB];
    int MinTPN[MAX_MREQ_PER_JOB];

    mnode_t *N;

    int ResourceIteration;
    char AffinityLevel;

    int AffinityAvailNodeCount[10];

    int AllocComplete; /* boolean */

    int tmpRSS;

    int nsindex;

    int rc;

    /* NOTE:  once best nodes are selected, nodes should be sorted by tasks */
    /*        available */

    const char *FName = "MJobAllocMNL";

    DBG(4, fSCHED)
    DPrint("%s(%s,MFeasibleList,NodeMap,%s,%s,%ld)\n", FName,
           (J != NULL) ? J->Name : "NULL",
           (MOutList != NULL) ? "MOutList" : "NULL", MNAllocPolicy[NAPolicy],
           StartTime);

    if (J == NULL) {
        return (FAILURE);
    }

    MTRAPJOB(J, FName);

    if (X.XJobAllocateResources != (int (*)())0) {
        rc = (*X.XJobAllocateResources)(J, MFeasibleList, NodeMap, MOutList,
                                        NAPolicy, StartTime);

        return (rc);
    }

    memcpy(MNodeList, MFeasibleList, sizeof(MNodeList));
    memset(NodeIndex, 0, sizeof(NodeIndex));
    memset(TaskCount, 0, sizeof(TaskCount));
    memset(NodeCount, 0, sizeof(NodeCount));
    memset(FeasibleTaskCount, 0, sizeof(FeasibleTaskCount));
    memset(FeasibleNodeCount, 0, sizeof(FeasibleNodeCount));

    /* initialize MOutList */

    if (MOutList != NULL) {
        memset(MOutList, 0,
               (MAX_MNODE + 1) * MAX_MREQ_PER_JOB * sizeof(mnalloc_t));

        for (rqindex = 0; rqindex < MAX_MREQ_PER_JOB; rqindex++)
            MOutList[rqindex][0].N = NULL;
    }

    /* allocate host list nodes */

    if (J->Flags & (1 << mjfHostList)) {
        if ((J->ReqHList == NULL) || (J->ReqHList[0].N == NULL)) {
            DBG(1, fSCHED)
            DPrint(
                "ERROR:    hostlist specified but hostlist is NULL/EMPTY for "
                "job %s\n",
                J->Name);

            return (FAILURE);
        }

        RQ = J->Req[0]; /* NOTE:  hostlist jobs only have one req */

        DBG(5, fSCHED)
        DPrint("INFO:     using specified hostlist for job %s\n", J->Name);

        /* check if HostList taskcount exceeds job taskcount */

        TC = 0;

        for (nindex = 0; J->ReqHList[nindex].N != NULL; nindex++) {
            for (index = 0; MNodeList[0][index].N != NULL; index++) {
                if (J->ReqHList[nindex].N == MNodeList[0][index].N) {
                    if (J->ReqHLMode == mhlmSubset) {
                        /* NOTE:  if subset list specified, hostlist does not *
                         *              * provide task constraint info */

                        NodeMap[MNodeList[0][index].N->Index] = nmRequired;

                        TC += MNodeList[0][index].TC;
                    } else {
                        if (J->ReqHLMode != mhlmSuperset) {
                            NodeMap[MNodeList[0][index].N->Index] = nmRequired;

                            MNodeList[0][index].TC = MIN(
                                MNodeList[0][index].TC, J->ReqHList[nindex].TC);
                        }

                        TC +=
                            MIN(MNodeList[0][index].TC, J->ReqHList[nindex].TC);
                    }

                    break;
                }
            } /* END for (index) */
        }     /* END for (nindex) */

        if (((TC < J->Request.TC) && !(J->Flags & (1 << mjfBestEffort)) &&
             (J->ReqHLMode != mhlmSubset)) ||
            (TC == 0)) {
            DBG(2, fSCHED)
            DPrint(
                "WARNING:  inadequate tasks specified in hostlist for job %s "
                "(%d < %d)\n",
                J->Name, TC, J->Request.TC);

            return (FAILURE);
        }

        if (J->ReqHLMode != mhlmSubset) {
            /* distribute tasks TPN at a time to hosts firstfit */

            if (!(J->Flags & (1 << mjfBestEffort))) {
                TC = J->Request.TC;
            }

            rnindex = 0;

            /* add feasible nodes found in specified hostlist */

            for (nindex = 0; J->ReqHList[nindex].N != NULL; nindex++) {
                for (index = 0; MNodeList[0][index].N != NULL; index++) {
                    if (J->ReqHList[nindex].N == MNodeList[0][index].N) {
                        break;
                    }
                } /* END for (index) */

                if (MNodeList[0][index].N == NULL) {
                    /* node is node feasible */

                    continue;
                }

                Delta = MIN(J->ReqHList[nindex].TC, TC);

                if (RQ->TasksPerNode > 0) {
                    Delta -= (Delta % RQ->TasksPerNode);
                }

                if (Delta > 0) {
                    N = J->ReqHList[nindex].N;

                    RQ->NodeList[rnindex].N = N;
                    RQ->NodeList[rnindex].TC = Delta;

                    DBG(7, fSCHED)
                    DPrint("INFO:     hostlist node %sx%d added to job %s\n",
                           N->Name, Delta, J->Name);

                    if (MOutList != NULL)
                        memcpy(&MOutList[0][rnindex], &RQ->NodeList[rnindex],
                               sizeof(mnalloc_t));

                    rnindex++;

                    TC -= Delta;
                }

                if (TC == 0) break;
            } /* END for (nindex) */

            if (rnindex == 0) {
                DBG(3, fSCHED)
                DPrint("WARNING:  empty hostlist for job %s in %s()\n", J->Name,
                       FName);

                return (FAILURE);
            }

            RQ->NodeList[rnindex].N = NULL;

            if (MOutList != NULL) MOutList[0][rnindex].N = NULL;

            if (TC > 0) {
                DBG(3, fSCHED)
                DPrint(
                    "WARNING:  cannot allocate tasks specified in hostlist for "
                    "job %s (%d remain)\n",
                    J->Name, TC);

                return (FAILURE);
            }

            DBG(3, fSCHED)
            DPrint(
                "INFO:     %d requested hostlist tasks allocated for job %s "
                "(%d remain)\n",
                J->Request.TC, J->Name, TC);

            return (SUCCESS);
        } /* END if (J->ReqHLMode != mhlmSubset) */
    }     /* END if (J->Flags & (1 << mjfHostList)) */

    /* perform initial per-req analysis */

    for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
        RQ = J->Req[rqindex];

        NodeList = (mnalloc_t *)MNodeList[rqindex];

        for (nindex = 0; NodeList[nindex].N != NULL; nindex++) {
            if (MSched.ResourcePrefIsActive == TRUE) {
                N = NodeList[nindex].N;

                MReqGetPref(RQ, N, &N->IsPref);
            }

            /* determine pref */
        } /* END for (nindex) */

        if (MPar[0].NodeSetDelay > 0) {
            /* mandatory node set */

            tmpRSS = (RQ->SetSelection != mrssNONE) ? RQ->SetSelection
                                                    : MPar[0].NodeSetPolicy;

            if (MJobSelectResourceSet(
                    J, RQ, (RQ->SetType != mrstNONE) ? RQ->SetType
                                                     : MPar[0].NodeSetAttribute,
                    (tmpRSS != mrssOneOf) ? tmpRSS : mrssFirstOf,
                    (RQ->SetList[0] != NULL) ? RQ->SetList
                                             : MPar[0].NodeSetList,
                    NodeList, nindex) == FAILURE) {
                return (FAILURE);
            }
        }

        MinTPN[rqindex] = 1;
        MaxTPN[rqindex] = RQ->TaskCount;

        if (RQ->TasksPerNode > 1) {
            MinTPN[rqindex] = RQ->TasksPerNode;
        }

        if (RQ->NodeCount > 0) {
            if (MRM[RQ->RMIndex].Type == mrmtLL) {
                MinTPN[rqindex] =
                    MAX(MinTPN[rqindex], RQ->TaskCount / RQ->NodeCount);
                MaxTPN[rqindex] = MinTPN[rqindex];

                if (RQ->TaskCount % RQ->NodeCount) MaxTPN[rqindex]++;
            }
        }

        if (MOutList == NULL)
            BestList[rqindex] = RQ->NodeList;
        else
            BestList[rqindex] = (mnalloc_t *)MOutList[rqindex];

        for (index = 0; NodeList[index].N != NULL; index++) {
            FeasibleTaskCount[rqindex] += NodeList[index].TC;
        } /* END for (index) */

        FeasibleNodeCount[rqindex] = (long)index;

        if (FeasibleTaskCount[rqindex] < RQ->TaskCount) {
            DBG(0, fSCHED)
            DPrint(
                "ERROR:    invalid nodelist for job %s:%d (inadequate "
                "taskcount, %d < %d)\n",
                J->Name, rqindex, FeasibleTaskCount[rqindex], RQ->TaskCount);

            return (FAILURE);
        }

        if ((RQ->NodeCount > 0) &&
            (FeasibleNodeCount[rqindex] < RQ->NodeCount)) {
            DBG(0, fSCHED)
            DPrint(
                "ERROR:    invalid nodelist for job %s:%d (inadequate "
                "nodecount, %d < %d)\n",
                J->Name, rqindex, FeasibleNodeCount[rqindex], RQ->NodeCount);

            return (FAILURE);
        }
    } /* END for (rqindex) */

    if (J->Flags & (1 << mjfResMap)) {
        /* standing (container) reservations should always use MinResource */

        tmpNAPolicy = mnalMinResource;
    } else if ((StartTime > MSched.Time) && (NAPolicy == mnalCPULoad)) {
        tmpNAPolicy = mnalMinResource;
    } else {
        tmpNAPolicy = NAPolicy;
    }

    DBG(6, fSCHED) {
        memset(AffinityAvailNodeCount, 0, sizeof(AffinityAvailNodeCount));

        for (index = 0; index < MAX_MNODE; index++) {
            if (MNode[index] == NULL) break;

            switch ((int)NodeMap[index]) {
                case nmRequired:

                    AffinityAvailNodeCount[0]++;

                    break;

                case nmPositiveAffinity:

                    AffinityAvailNodeCount[1]++;

                    break;

                case nmNeutralAffinity:

                    AffinityAvailNodeCount[2]++;

                    break;

                case nmNone:

                    AffinityAvailNodeCount[3]++;

                    break;

                case nmNegativeAffinity:

                    AffinityAvailNodeCount[4]++;

                    break;

                case nmPreemptible:

                    AffinityAvailNodeCount[5]++;

                    break;

                default:

                    /* NO-OP */

                    break;
            } /* END switch((int)NodeMap[index]) */
        }     /* END for (index) */

        for (index = 0; index <= 5; index++) {
            DPrint("INFO:     affinity level %d nodes: %d\n", index,
                   AffinityAvailNodeCount[index]);
        } /* END for (index) */

        for (index = 0; MNodeList[0][index].N != NULL; index++) {
            DPrint("INFO:     nodelist[%d] %s  %d  %d\n", index,
                   MNodeList[0][index].N->Name, MNodeList[0][index].TC,
                   NodeMap[MNodeList[0][index].N->Index]);
        }
    } /* END DBG(6,fSCHED) */

    for (nsindex = 0; nsindex < 2; nsindex++) {
        /* PASS 0:  attempt node set allocation */
        /* PASS 1:  attempt any set allocation  */

        /* memcpy(MNodeList,MFeasibleList,sizeof(MNodeList)); */

        memset(NodeIndex, 0, sizeof(NodeIndex));
        memset(TaskCount, 0, sizeof(TaskCount));
        memset(NodeCount, 0, sizeof(NodeCount));

        /* check various nodesets */

        if (nsindex >= 1) memcpy(MNodeList, MFeasibleList, sizeof(MNodeList));

        for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
            RQ = J->Req[rqindex];

            RQ->NodeList[0].N = NULL;

            tmpRSS = (RQ->SetSelection != mrssNONE) ? RQ->SetSelection
                                                    : MPar[0].NodeSetPolicy;

            if (((tmpRSS == mrssNONE) && (nsindex == 0)) ||
                (MPar[0].NodeSetDelay > 0)) {
                /* if node set delay > 0 (no best effort) nodelist modification
                 * *
                 * handled in pre test */

                DBG(7, fSCHED)
                DPrint(
                    "INFO:     ignoring pass 1 for job %s:%d (node set forced "
                    "in feasible list)\n",
                    J->Name, rqindex);

                continue;
            } else if ((tmpRSS != mrssNONE) && (nsindex == 1)) {
                /* ns iteration 1 only for use if iteration 0 failed and *
                 * NodeSetDelay == 0 (ie, best effort allowed)           */

                DBG(7, fSCHED)
                DPrint(
                    "INFO:     ignoring pass 2 for job %s (node sets "
                    "enabled)\n",
                    J->Name);

                continue;
            }

            NodeList = (mnalloc_t *)MNodeList[rqindex];

            for (nindex = 0; NodeList[nindex].N != NULL; nindex++)
                ;

            if ((nsindex == 0) &&
                (MJobSelectResourceSet(
                     J, RQ,
                     (RQ->SetType != mrstNONE) ? RQ->SetType
                                               : MPar[0].NodeSetAttribute,
                     (tmpRSS != mrssOneOf) ? tmpRSS : mrssFirstOf,
                     (RQ->SetList[0] != NULL) ? RQ->SetList
                                              : MPar[0].NodeSetList,
                     NodeList, nindex) == FAILURE)) {
                /* cannot locate needed set based resources */

                DBG(4, fSCHED)
                DPrint(
                    "INFO:     cannot locate adequate resource set for job "
                    "%s\n",
                    J->Name);

                continue;
            }
        } /* END for (rqindex) */

        for (ResourceIteration = 0; ResourceIteration <= 5;
             ResourceIteration++) {
            /* NOTE:  apply job preferences 'within' affinity values */

            /* NOTE:

                 iteration 0:  Required
                 iteration 1:  PositiveAffinity
                 iteration 2:  NeutralAffinity
                 iteration 3:  None
                 iteration 4:  NegativeAffinity
                 iteration 5:  Preemptible
            */

            switch (ResourceIteration) {
                case 0:

                    AffinityLevel = (char)nmRequired;

                    break;

                case 1:

                    AffinityLevel = (char)nmPositiveAffinity;

                    break;

                case 2:

                    AffinityLevel = (char)nmNeutralAffinity;

                    break;

                case 3:
                default:

                    AffinityLevel = (char)nmNone;

                    break;

                case 4:

                    AffinityLevel = (char)nmNegativeAffinity;

                    break;

                case 5:

                    AffinityLevel = (char)nmPreemptible;

                    break;
            } /* END switch(ResourceIteration) */

            for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
                DBG(5, fSCHED)
                DPrint(
                    "INFO:     evaluating nodes on alloc iteration %d for job "
                    "%s:%d\n",
                    ResourceIteration, J->Name, rqindex);

                RQ = J->Req[rqindex];

                NodeList = (mnalloc_t *)MNodeList[rqindex];

                FeasibleTaskCount[rqindex] = 0;

                for (index = 0; NodeList[index].N != NULL; index++) {
                    FeasibleTaskCount[rqindex] += NodeList[index].TC;
                } /* END for (index) */

                FeasibleNodeCount[rqindex] = index;

                if (TaskCount[rqindex] >= RQ->TaskCount) {
                    /* all required procs found */

                    if ((RQ->NodeCount == 0) ||
                        (NodeCount[rqindex] >= RQ->NodeCount)) {
                        /* req is satisfied */

                        continue;
                    }
                }

                switch (tmpNAPolicy) {
                    case mnalLocal:

                        MLocalJobAllocateResources(
                            J, RQ, NodeList, StartTime, rqindex, MinTPN, MaxTPN,
                            NodeMap, AffinityLevel, NodeIndex, BestList,
                            TaskCount, NodeCount);

                        break;

                    case mnalContiguous:

                        MJobAllocateContiguous(J, RQ, NodeList, rqindex, MinTPN,
                                               MaxTPN, NodeMap, AffinityLevel,
                                               NodeIndex, BestList, TaskCount,
                                               NodeCount);

                        break;

                    case mnalMaxBalance:

                        MJobAllocateBalanced(J, RQ, NodeList, rqindex, MinTPN,
                                             MaxTPN, NodeMap, AffinityLevel,
                                             NodeIndex, BestList, TaskCount,
                                             NodeCount);

                        break;

                    case mnalFastest:

                        MJobAllocateFastest(J, RQ, NodeList, rqindex, MinTPN,
                                            MaxTPN, NodeMap, AffinityLevel,
                                            NodeIndex, BestList, TaskCount,
                                            NodeCount);

                        break;

                    case mnalMachinePrio:

                        MJobAllocatePriority(J, RQ, NodeList, rqindex, MinTPN,
                                             MaxTPN, NodeMap, AffinityLevel,
                                             NodeIndex, BestList, TaskCount,
                                             NodeCount, StartTime);

                        break;

                    case mnalCPULoad:

                        /* Allocate job to nodes with maximum idle processors,
                           ie
                             (N->CRes.Procs - N->Load)
                         */

                        {
                            double MaxCPUAvail;

                            while (1) {
                                MaxCPUAvail = 0.0;

                                /* determine best fit */

                                for (nindex = 0; NodeList[nindex].N != NULL;
                                     nindex++) {
                                    NIndex =
                                        NodeList[FeasibleNodeCount[rqindex] -
                                                 nindex - 1]
                                            .N->Index;

                                    if (NodeMap[NIndex] != AffinityLevel) {
                                        /* node unavailable */

                                        continue;
                                    }

                                    N = MNode[NIndex];

                                    if ((N->CRes.Procs -
                                         MAX(N->DRes.Procs, N->Load)) <=
                                        MaxCPUAvail) {
                                        /* better CPU availability already
                                         * located */

                                        continue;
                                    }

                                    MaxCPUAvail =
                                        (double)N->CRes.Procs -
                                        MAX((double)N->DRes.Procs, N->Load);
                                }

                                if (MaxCPUAvail <= 0.0) {
                                    /* no additional nodes found */

                                    break;
                                }

                                for (nindex = 0; NodeList[nindex].N != NULL;
                                     nindex++) {
                                    N = NodeList[FeasibleNodeCount[rqindex] -
                                                 nindex - 1]
                                            .N;

                                    if (NodeMap[N->Index] != AffinityLevel) {
                                        /* node unavailable */

                                        continue;
                                    }

                                    if (((double)N->CRes.Procs -
                                         MAX((double)N->DRes.Procs, N->Load)) <
                                        MaxCPUAvail - 0.1) {
                                        /* better CPU availability already
                                         * located */

                                        continue;
                                    }

                                    NTasks =
                                        NodeList[FeasibleNodeCount[rqindex] -
                                                 nindex - 1]
                                            .TC;

                                    if (NTasks < MinTPN[rqindex]) continue;

                                    NTasks = MIN(NTasks, MaxTPN[rqindex]);

                                    BestList[rqindex][NodeIndex[rqindex]].N = N;
                                    BestList[rqindex][NodeIndex[rqindex]].TC =
                                        NTasks;

                                    NodeIndex[rqindex]++;
                                    TaskCount[rqindex] += NTasks;
                                    NodeCount[rqindex]++;

                                    /* mark node as used */

                                    NodeMap[N->Index] = nmUnavailable;

                                    /* NOTE:  must incorporate TaskRequestList
                                     * information */

                                    if (TaskCount[rqindex] >= RQ->TaskCount) {
                                        /* all required procs found */

                                        /* NOTE:  HANDLED BY DIST */

                                        if ((RQ->NodeCount == 0) ||
                                            (NodeCount[rqindex] >=
                                             RQ->NodeCount)) {
                                            /* terminate BestList */

                                            BestList[rqindex]
                                                    [NodeIndex[rqindex]]
                                                        .N = NULL;

                                            break;
                                        }
                                    }
                                } /* END for (nindex) */

                                if (NodeList[nindex].N == NULL) {
                                    /* no node could be allocated */

                                    break;
                                }

                                if (TaskCount[rqindex] >= RQ->TaskCount) {
                                    /* all required procs found */

                                    if ((RQ->NodeCount == 0) ||
                                        (NodeCount[rqindex] >= RQ->NodeCount)) {
                                        break;
                                    }
                                }
                            } /* END while (1) */
                        }     /* END BLOCK     */

                        break;

                    case mnalLastAvailable:

                        /* nodes will be in First-to-Last available order
                         * because of */
                        /* MJobGetEStartTime() */

                        /* select last 'RQ->TaskCount' procs */

                        for (nindex = 0; NodeList[nindex].N != NULL; nindex++) {
                            int TC;

                            N = NodeList[FeasibleNodeCount[rqindex] - nindex -
                                         1]
                                    .N;

                            if (NodeMap[N->Index] != AffinityLevel) {
                                /* node unavailable */

                                continue;
                            }

                            NTasks = NodeList[FeasibleNodeCount[rqindex] -
                                              nindex - 1]
                                         .TC;

                            TC = (RQ->TaskCount > 0)
                                     ? MIN(NTasks, RQ->TaskCount)
                                     : NTasks;

                            if (NTasks < MinTPN[rqindex]) continue;

                            NTasks = MIN(NTasks, MaxTPN[rqindex]);

                            BestList[rqindex][NodeIndex[rqindex]].N = N;
                            BestList[rqindex][NodeIndex[rqindex]].TC = TC;

                            NodeIndex[rqindex]++;
                            TaskCount[rqindex] += TC;
                            NodeCount[rqindex]++;

                            /* mark node as used */

                            NodeList[FeasibleNodeCount[rqindex] - nindex - 1]
                                .TC -= TC;

                            if (NodeList[FeasibleNodeCount[rqindex] - nindex -
                                         1]
                                    .TC <= 0) {
                                NodeMap[N->Index] = nmUnavailable;
                            }

                            /* NOTE:  must incorporate TaskRequestList
                             * information */

                            if (TaskCount[rqindex] >= RQ->TaskCount) {
                                /* all required procs found */

                                /* NOTE:  HANDLED BY DIST */

                                if ((RQ->NodeCount == 0) ||
                                    (NodeCount[rqindex] >= RQ->NodeCount)) {
                                    /* terminate BestList */

                                    BestList[rqindex][NodeIndex[rqindex]].N =
                                        NULL;

                                    break;
                                }
                            }
                        } /* END for (nindex) */

                        break;

                    case mnalFirstAvailable:

                        /* select first 'RQ->TaskCount' procs */

                        for (nindex = 0; NodeList[nindex].N != NULL; nindex++) {
                            N = NodeList[nindex].N;

                            if (NodeMap[N->Index] == AffinityLevel) {
                                /* node unavailable */

                                continue;
                            }

                            TC = NodeList[nindex].TC;

                            if (TC < MinTPN[rqindex]) continue;

                            TC = MIN(TC, MaxTPN[rqindex]);

                            BestList[rqindex][NodeIndex[rqindex]].N = N;

                            BestList[rqindex][NodeIndex[rqindex]].TC =
                                (short)TC;

                            NodeIndex[rqindex]++;
                            TaskCount[rqindex] += TC;
                            NodeCount[rqindex]++;

                            /* mark node as used */

                            NodeMap[N->Index] = nmUnavailable;

                            if (TaskCount[rqindex] >= RQ->TaskCount) {
                                /* all required tasks found */

                                /* NOTE:  HANDLED BY DIST */

                                if ((RQ->NodeCount == 0) ||
                                    (NodeCount[rqindex] >= RQ->NodeCount)) {
                                    /* terminate BestList */

                                    BestList[rqindex][NodeIndex[rqindex]].N =
                                        NULL;

                                    break;
                                }
                            }
                        } /* END for (nindex) */

                        break;

                    case mnalMinLocal:

                        /* select 'RQ->TaskCount' nodes with minimal near-term
                         * idle queue requests */

                        /* NYI */

                        break;

                    case mnalMinGlobal:

                        /* select 'RQ->TaskCount' nodes with minimal long-term
                         * idle queue requests */

                        /* NYI */

                        break;

                    case mnalMinResource:

                        /* select 'RQ->TaskCount' tasks with minimal configured
                           resources
                           which meet job requirements (last available) */

                        /* loop through all memory sizes */

                        for (mem = 64; mem <= 8192; mem <<= 1) {
                            for (nindex = 0; NodeList[nindex].N != NULL;
                                 nindex++) {
                                N = NodeList[FeasibleNodeCount[rqindex] -
                                             nindex - 1]
                                        .N;

                                if (NodeMap[N->Index] != AffinityLevel) {
                                    /* node unavailable */

                                    continue;
                                }

                                if ((MNode[N->Index]->CRes.Mem > mem) &&
                                    (mem < 8192)) {
                                    /* node resources too great */

                                    continue;
                                }

                                NTasks = NodeList[FeasibleNodeCount[rqindex] -
                                                  nindex - 1]
                                             .TC;

                                if (NTasks < MinTPN[rqindex]) continue;

                                NTasks = MIN(NTasks, MaxTPN[rqindex]);

                                if (RQ->TasksPerNode > 0) {
                                    NTasks -= (NTasks % RQ->TasksPerNode);

                                    if (NTasks == 0) continue;
                                }

                                BestList[rqindex][NodeIndex[rqindex]].N = N;
                                BestList[rqindex][NodeIndex[rqindex]].TC =
                                    NTasks;

                                NodeIndex[rqindex]++;

                                TaskCount[rqindex] += NTasks;
                                NodeCount[rqindex]++;

                                /* mark node as used */

                                NodeMap[N->Index] = nmUnavailable;

                                if (TaskCount[rqindex] >= RQ->TaskCount) {
                                    /* NOTE: HANDLED BY DIST */

                                    /* all required tasks found */

                                    if ((RQ->NodeCount == 0) ||
                                        (NodeCount[rqindex] >= RQ->NodeCount)) {
                                        /* terminate list */

                                        BestList[rqindex][NodeIndex[rqindex]]
                                            .N = NULL;

                                        break;
                                    }
                                }
                            } /* END for (nindex) */

                            if (TaskCount[rqindex] >= RQ->TaskCount) {
                                /* all required procs found */

                                if ((RQ->NodeCount == 0) ||
                                    (NodeCount[rqindex] >= RQ->NodeCount))
                                    break;
                            }
                        } /* END for (mem)    */

                        break;

                    default:

                        DBG(1, fSCHED)
                        DPrint(
                            "ERROR:    invalid allocation policy (%d) in "
                            "%s()\n",
                            tmpNAPolicy, FName);

                        return (FAILURE);

                        /* break; */
                } /* END switch(tmpNAPolicy) */
            }     /* END for (rqindex)            */
        }         /* END for (ResourceIteration)  */

        AllocComplete = FALSE;

        for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
            RQ = J->Req[rqindex];

            if (TaskCount[rqindex] >= RQ->TaskCount) {
                /* all required procs found */

                if ((RQ->NodeCount == 0) ||
                    (NodeCount[rqindex] >= RQ->NodeCount)) {
                    AllocComplete = TRUE;

                    break;
                }
            } else {
                /* unable to locate adequate tasks */
                /*
                        fprintf(stderr,"NOTE:  cannot locate all required tasks
                   for job %s in nodeset iteration %d (TaskCount: %d)\n",
                          J->Name,
                          nsindex,
                          FeasibleTaskCount[rqindex]);
                */
            }
        } /* END for (rqindex) */

        if (AllocComplete == TRUE) break;
    } /* END for (nsindex) */

    /* determine available resources */

    TotalAllocNodeCount = 0;
    TotalFeasibleNodeCount = 0;

    TotalAllocTaskCount = 0;
    TotalFeasibleTaskCount = 0;

    for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
        RQ = J->Req[rqindex];

        if (TaskCount[rqindex] < RQ->TaskCount) {
            DBG(2, fSCHED)
            DPrint(
                "ALERT:    inadequate tasks to allocate to job %s:%d (%d < "
                "%d)\n",
                J->Name, rqindex, TaskCount[rqindex], RQ->TaskCount);

            return (FAILURE);
        }

        if ((RQ->NodeCount > 0) && (NodeCount[rqindex] < RQ->NodeCount)) {
            DBG(2, fSCHED)
            DPrint("ALERT:    inadequate nodes found for job %s:%d (%d < %d)\n",
                   J->Name, rqindex, NodeCount[rqindex], RQ->NodeCount);

            return (FAILURE);
        }

        TotalAllocNodeCount += NodeIndex[rqindex];
        TotalFeasibleNodeCount += FeasibleNodeCount[rqindex];

        TotalAllocTaskCount += TaskCount[rqindex];
        TotalFeasibleTaskCount += FeasibleTaskCount[rqindex];
    } /* END for (rqindex = 0;J->Req[rqindex] != NULL;rqindex++) */

    DBG(2, fSCHED)
    DPrint(
        "INFO:     tasks located for job %s:  %d of %d required (%d "
        "feasible)\n",
        J->Name, TotalAllocTaskCount, J->Request.TC, TotalFeasibleTaskCount);

    /* NOTE:  if input and output buffers are the same, copy AllocatedNode */
    /*        list to output buffer */

    if (MOutList != NULL) {
        /* this is wrong.  Must determine proper copying */

        /* memcpy(MOutList,MBestList,sizeof(MOutList)); */
    }

    DBG(4, fSCHED) {
        for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
            RQ = J->Req[rqindex];

            if (MOutList == NULL)
                BestList[rqindex] = (mnalloc_t *)RQ->NodeList;
            else
                BestList[rqindex] = (mnalloc_t *)MOutList[rqindex];

            for (index = 0; BestList[rqindex][index].N != NULL; index++) {
                DBG(4, fSCHED)
                DPrint("INFO:     allocated MNode[%03d]x%d '%s' to %s:%d\n",
                       index, BestList[rqindex][index].TC,
                       BestList[rqindex][index].N->Name, J->Name, rqindex);
            }
        } /* END for (rqindex) */
    }     /* END DBG(4,fSCHED) */

    return (SUCCESS);
} /* END MJobAllocMNL() */

/* return 'SUCCESS' and set pointer to list of all available nodes that */
/*   can run job at MSched.Time or 'FAILURE' if job cannot run now      */

int MJobSelectMNL(

    mjob_t *J,             /* I: job                               */
    mpar_t *P,             /* I: node partition                    */
    nodelist_t FNL,        /* I: list of feasible nodes (optional) */
    mnodelist_t MNodeList, /* O: array of nodes that can run job   */
    char *NodeMap,         /* O: node availability array           */
    int SAllowPreemption)  /* I */

{
    int ITC;
    int INC;

    int PTC;
    int PNC;

    int AvailableTaskCount[MAX_MREQ_PER_JOB];

    int TotalAvailINC = 0;
    int TotalAvailITC = 0;
    int TotalAvailIPC = 0;

    int TotalAvailPNC = 0;
    int TotalAvailPTC = 0;

    int PreempteeTCList[MAX_MJOB];
    int PreempteeNCList[MAX_MJOB];
    nodelist_t *PreempteeTaskList[MAX_MJOB];

    int ReqNC;
    int ReqTC;
    int ReqPC;

    mjob_t *FeasiblePreempteeJobList[MAX_MJOB];
    mjob_t *PreempteeJobList[MAX_MJOB];

    mnalloc_t IdleNode[MAX_MREQ_PER_JOB][MAX_MNODE];
    mnalloc_t PNL[MAX_MNODE];

    mnodelist_t LocalFNL;

    nodelist_t *TL;

    int index;
    int nindex;
    int jindex;

    int rqindex;

    int TotalJobProcs;

    int Reason[MAX_MREQ_PER_JOB][MAX_MREJREASON];

    nodelist_t tmpNL;

    mnodelist_t newMNL;
    mnodelist_t tmpMNL;

    mnalloc_t *NodeList;

    mbool_t PreemptNodesRequired = FALSE;

    mreq_t *RQ;

    int SC;

    int AllowPreemption = FALSE;
    int PIsConditional = FALSE;

    const char *FName = "MJobSelectMNL";

    DBG(4, fSCHED)
    DPrint("%s(%s,%s,%s,MNodeList,NodeMap,MaxSpeed,%lu)\n", FName,
           (J != NULL) ? J->Name : "NULL", (P != NULL) ? P->Name : "NULL",
           (FNL == NULL) ? "NULL" : "FNL", SAllowPreemption);

    if ((J == NULL) || (P == NULL)) {
        return (FAILURE);
    }

    /* NOTE:  locate all idle feasible tasks */
    /*        locate preemptible resources   */

    MTRAPJOB(J, FName);

    memset(NodeMap, nmUnavailable, sizeof(char) * MAX_MNODE);

    /* NOTE:  overcommit preemption requires externally signalling that
     * preemption *
     *        is occuring and that dedicated resource violations are acceptable
     */

    if (J->Flags & (1 << mjfSpan)) P = &MPar[0];

    for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
        RQ = J->Req[rqindex];

        /* populate local feasible node list */

        tmpNL[0].N = NULL;

        if (MReqGetFNL(J, RQ, P, FNL, /* I */
                       tmpNL, NULL, NULL, MAX_MTIME, 0) == FAILURE) {
            /* insufficient feasible nodes found */

            DBG(4, fSCHED)
            DPrint(
                "INFO:     cannot locate adequate feasible tasks for job "
                "%s:%d\n",
                J->Name, rqindex);

            return (FAILURE);
        }

        MJobGetINL(J, &(tmpNL[0]), IdleNode[rqindex], P->Index, &INC, &ITC);

        /* early feasibility check: feasible idle nodes found */

        if (ITC < RQ->TaskCount) {
            DBG(4, fSCHED)
            DPrint(
                "INFO:     insufficient idle tasks in partition %s for %s:%d: "
                "(%d of %d available)\n",
                P->Name, J->Name, RQ->Index, ITC, RQ->TaskCount);

            if (SAllowPreemption == FALSE) {
                /* idle tasks only allowed */

                return (FAILURE);
            } else {
                PreemptNodesRequired = TRUE;

                DBG(4, fSCHED)
                DPrint("INFO:     will attempt to locate preempt resources\n");
            }
        } else if (INC < RQ->NodeCount) {
            DBG(4, fSCHED)
            DPrint(
                "INFO:     insufficient idle nodes in partition %s for req %d: "
                "(%d of %d available)\n",
                P->Name, RQ->Index, INC, RQ->NodeCount);

            if (SAllowPreemption == FALSE) {
                /* idle tasks only allowed */

                return (FAILURE);
            } else {
                PreemptNodesRequired = TRUE;
            }
        } /* END else if (IdleNodeCount < RQ->NodeCount) */
        else {
            DBG(7, fSCHED)
            DPrint("INFO:     adequate idle nodes/tasks located\n");
        }

        /* select idle resources */

        memset(Reason, 0, sizeof(Reason));

        /* NOTE:  multi-req preemption not supported */

        TotalAvailITC = 0;
        TotalAvailINC = 0;
        TotalAvailIPC = 0;

        if (MNodeSelectIdleTasks(J, RQ, IdleNode[rqindex], tmpMNL,
                                 &TotalAvailITC, &TotalAvailINC, NodeMap,
                                 Reason) == FAILURE) {
            DBG(4, fSCHED)
            DPrint(
                "INFO:     insufficient idle resources selected in partition "
                "%s: (%d of %d tasks/%d of %d nodes)\n",
                P->Name, TotalAvailITC, J->Request.TC, TotalAvailINC,
                J->Request.NC);

            if (SAllowPreemption == FALSE) {
                /* idle tasks only allowed */

                return (FAILURE);
            } else {
                PreemptNodesRequired = TRUE;
            }
        } /* END if (MNodeSelectIdleTasks() == FAILURE) */

        for (nindex = 0; tmpNL[nindex].N != NULL; nindex++) {
            LocalFNL[rqindex][nindex].N = tmpNL[nindex].N;
            LocalFNL[rqindex][nindex].TC = tmpNL[nindex].TC;
        } /* END for (nindex) */

        LocalFNL[rqindex][nindex].N = NULL;
    } /* END for (rqindex) */

    /* adequate nodes located */

    if ((J->Flags & (1 << mjfHostList)) && (J->ReqHLMode == mhlmSubset)) {
        int hindex;
        int nindex;
        int rqindex;

        mbool_t HostFound; /* (boolean) */

        /* verify correct hosts are available */

        for (hindex = 0; J->ReqHList[hindex].N != NULL; hindex++) {
            HostFound = FALSE;

            for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
                for (nindex = 0; LocalFNL[rqindex][nindex].N != NULL;
                     nindex++) {
                    if (LocalFNL[rqindex][nindex].N != J->ReqHList[hindex].N)
                        continue;

                    HostFound = TRUE;

                    break;
                } /* END for (nindex) */

                if (HostFound == TRUE) break;
            } /* END for (rqindex) */

            if (HostFound == FALSE) {
                PreemptNodesRequired = TRUE;

                break;
            }
        } /* END for (hindex) */
    }     /* END if ((J->Flags & (1 << mjfHostList)) && ...) */

    /* NOTE:  multi-req not supported */

    RQ = J->Req[0];

    PNC = 0;
    PTC = 0;

    TotalAvailPNC = 0;
    TotalAvailPTC = 0;

    if (PreemptNodesRequired == TRUE) {
        PIsConditional = FALSE;

        if (SAllowPreemption == MBNOTSET) {
            /* determine if job can be preemptor */

            if (MISSET(J->Flags, mjfPreemptor) == TRUE) {
                AllowPreemption = TRUE;
            } else {
                int rindex;

                mres_t *R;

                AllowPreemption = FALSE;

                /* search reservation table */

                for (rindex = 0; MRes[rindex] != NULL; rindex++) {
                    R = MRes[rindex];

                    if ((R->Name[0] == '\0') || (R->Name[0] == '\1')) continue;

                    if ((MISSET(R->Flags, mrfOwnerPreempt) == FALSE) ||
                        (R->IsActive == FALSE))
                        continue;

                    if (MCredIsMatch(&J->Cred, R->O, R->OType) == FAILURE)
                        continue;

                    PIsConditional = TRUE;

                    AllowPreemption = TRUE;

                    break;
                }
            } /* END else (MISSET(J,mjfPreemptor) == TRUE) */

            if (AllowPreemption == FALSE) {
                return (FAILURE);
            }
        } /* END if (SAllowPreemption == MBNOTSET) */

        if (MSched.PreemptPolicy == mppOvercommit) {
            mnode_t *N;

            /* determine 'per queue' overcommit node */

            if ((J->Cred.C == NULL) || (J->Cred.C->OCNodeName == NULL) ||
                (MNodeFind(J->Cred.C->OCNodeName, &N) == FAILURE)) {
                /* cannot locate OC node */

                return (FAILURE);
            }

            /* NOTE:  only works for non-spanning jobs */

            TotalAvailITC += RQ->TaskCount;
            TotalAvailINC += 1;
            TotalAvailIPC +=
                RQ->TaskCount *
                ((RQ->DRes.Procs == -1) ? N->CRes.Procs : RQ->DRes.Procs);

            tmpMNL[0][0].N = N;
            tmpMNL[0][0].TC = RQ->TaskCount;

            tmpMNL[0][1].N = NULL;
            tmpMNL[1][0].N = NULL;
        } else {
            DBG(4, fSCHED)
            DPrint("INFO:     attempting to locate preemptible resources\n");

            /* determine jobs/resources available for preemption */

            MNodeGetPreemptList(J, (FNL == NULL) ? (mnalloc_t *)&LocalFNL[0]
                                                 : (mnalloc_t *)&FNL[0],
                                PNL, FeasiblePreempteeJobList, J->StartPriority,
                                P->Index, PIsConditional, &PNC, &PTC);

            if ((J->Req[1] == NULL) && ((PTC + ITC) < J->Req[0]->TaskCount)) {
                DBG(4, fSCHED)
                DPrint(
                    "INFO:     insufficient P+I tasks in partition %s: (%d of "
                    "%d available)\n",
                    P->Name, ITC + PTC, J->Request.TC);

                return (FAILURE);
            }

            if ((J->Req[1] == NULL) && (J->Request.NC > 0) &&
                ((PNC + INC) < J->Req[0]->NodeCount)) {
                DBG(4, fSCHED)
                DPrint(
                    "INFO:     insufficient P+I nodes in partition %s: (%d of "
                    "%d available)\n",
                    P->Name, INC + PNC, J->Request.NC);

                return (FAILURE);
            }

            /* check initial task availability */

            TotalJobProcs = MJobGetProcCount(J);

            if (J->Req[1] == NULL) {
                if (TotalJobProcs > (P->ARes.Procs + PTC * RQ->DRes.Procs)) {
                    DBG(4, fSCHED)
                    DPrint(
                        "INFO:     insufficient partition procs: (%d+%d "
                        "available %d needed)\n",
                        P->ARes.Procs, PTC * RQ->DRes.Procs, TotalJobProcs);

                    return (FAILURE);
                } else if (TotalJobProcs >
                           (MPar[0].ARes.Procs + PTC * RQ->DRes.Procs)) {
                    DBG(4, fSCHED)
                    DPrint(
                        "INFO:     insufficient GP procs: (%d+%d available %d "
                        "needed)\n",
                        MPar[0].ARes.Procs, PTC * RQ->DRes.Procs,
                        TotalJobProcs);

                    return (FAILURE);
                }
            }

            DBG(6, fSCHED)
            DPrint("INFO:     adequate tasks (P+I=%d+%d) located for job %s\n",
                   ITC, PTC, J->Name);

            /* locate available nodes */

            memset(Reason, 0, sizeof(Reason));

            /* NOTE:  broken for multi-req */

            TotalAvailPTC = 0;
            TotalAvailPNC = 0;

            /* inadequate feasible idle nodes located */

            if (AllowPreemption == FALSE) {
                return (FAILURE);
            }

            TotalAvailPTC = TotalAvailITC;
            TotalAvailPNC = TotalAvailINC;

            /* preempt tasks required */

            ReqTC = RQ->TaskCount - TotalAvailITC;
            ReqNC = RQ->NodeCount - TotalAvailINC;
            ReqPC = (RQ->TaskCount * RQ->DRes.Procs) - TotalAvailIPC;

            if (MJobSelectPJobList(J, ReqTC, ReqNC, FeasiblePreempteeJobList,
                                   (FNL == NULL) ? (mnalloc_t *)&LocalFNL[0]
                                                 : (mnalloc_t *)&FNL[0],
                                   PreempteeJobList, PreempteeTCList,
                                   PreempteeNCList,
                                   PreempteeTaskList) == FAILURE) {
                /* unable to select needed preempt jobs */

                return (FAILURE);
            }

/* job can preempt needed resources */

#ifdef __BYU
            /* determine earliest job could start without preemption */

            if (MJobReserve(J, jrPriority) == SUCCESS) {
                DBG(1, fALL)
                DPrint(
                    "NOTE:     job %s would start in %s without preemption "
                    "(PC: %d)\n",
                    J->Name, MULToTString(J->R->StartTime - MSched.Time),
                    J->PreemptCount);

                MResDestroy(&J->R);
            }

#endif /* __BYU */

            nindex = TotalAvailINC; /* HACK */

            NodeList = (mnalloc_t *)tmpMNL[0];

            for (jindex = 0; PreempteeJobList[jindex] != NULL; jindex++) {
                if ((ReqTC <= 0) && (ReqNC <= 0) && (ReqPC <= 0)) {
                    MUFree((char **)&PreempteeTaskList[jindex]);

                    continue;
                }

                if (MJobPreempt(PreempteeJobList[jindex], PreempteeJobList, -1,
                                NULL, &SC) == FAILURE) {
                    MUFree((char **)&PreempteeTaskList[jindex]);

                    DBG(2, fSCHED)
                    DPrint("ALERT:    job %s cannot preempt job %s\n", J->Name,
                           PreempteeJobList[jindex]->Name);

                    continue;
                }

                /* NOTE:  PreempteeTCList returns single proc tasks */

                ReqTC -= PreempteeTCList[jindex];
                ReqNC -= PreempteeNCList[jindex];
                ReqPC -= PreempteeTCList[jindex];

                /* determine nodes made available */

                TL = PreempteeTaskList[jindex];

                for (index = 0; (*TL)[index].N != NULL; index++) {
                    /* append preempted tasks to end of idle node list */

                    memcpy(&NodeList[nindex], &(*TL)[index], sizeof(mnalloc_t));

                    NodeMap[(*TL)[index].N->Index] = nmPreemptible;

                    nindex++;
                } /* END for (index) */

                MUFree((char **)&PreempteeTaskList[jindex]);
            } /* END for (jindex) */

            if ((ReqTC > 0) || (ReqNC > 0) || (ReqPC > 0)) {
                DBG(2, fSCHED)
                DPrint(
                    "ALERT:    unable to preempt adequate resources to start "
                    "job %s\n",
                    J->Name);

                return (FAILURE);
            }

            tmpMNL[0][nindex].N = NULL;

            TotalAvailITC += PreempteeTCList[jindex];
            TotalAvailIPC += PreempteeTCList[jindex];
            TotalAvailINC += PreempteeNCList[jindex];
        } /* END else (MSched.PreemptPolicy == mppOvercommit) */
    }     /* END if (PreemptTasksNeeded == TRUE) */

    /* apply locality constraints */

    if ((J->Flags & (1 << mjfAllocLocal)) ||
        (MSched.AllocLocalityPolicy != malpNONE)) {
        if (MJobProximateMNL(J, tmpMNL, newMNL, MSched.Time, FALSE) ==
            FAILURE) {
            DBG(4, fSCHED)
            DPrint("INFO:     inadequate proximity within available tasks\n");

            return (FAILURE);
        } else {
            memcpy(tmpMNL, newMNL, sizeof(newMNL));
        }
    }

    DBG(4, fSCHED)
    DPrint("INFO:     %d(%d) tasks/%d(%d) nodes found for job %s in %s\n",
           TotalAvailITC, TotalAvailPTC, TotalAvailINC, TotalAvailPNC, J->Name,
           FName);

    if (MJobNLDistribute(J, tmpMNL, MNodeList) == FAILURE) {
        /* insufficient nodes were found */

        DBG(4, fSCHED) {
            for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
                RQ = J->Req[rqindex];

                DBG(4, fSCHED)
                DPrint(
                    "INFO:     tasks found for job %s:%d  (%3d required  %3d "
                    "available)\n",
                    J->Name, rqindex, RQ->TaskCount,
                    AvailableTaskCount[rqindex]);

                for (index = 0; index < MAX_MREJREASON; index++) {
                    if (Reason[rqindex][index] > 0) {
                        fprintf(mlog.logfp, "[%s: %d]", MAllocRejType[index],
                                Reason[rqindex][index]);
                    }
                }

                fprintf(mlog.logfp, "\n");
            } /* END for (rqindex) */
        }

        return (FAILURE);
    } /* END if (MJobNLDistribute()) == FAILURE) */

    DBG(4, fSCHED)
    DPrint(
        "INFO:     resources found for job %s tasks: %d+%d of %d  nodes: %d+%d "
        "of %d\n",
        J->Name, TotalAvailITC, TotalAvailPTC, J->Request.TC, TotalAvailINC,
        TotalAvailPNC, J->Request.NC);

    return (SUCCESS);
} /* END MJobSelectMNL() */

int MJobDistributeTaskGeometry(

    mjob_t *J,     /* I */
    mreq_t *RQ,    /* I */
    mnalloc_t *NL, /* O */
    int *NI)       /* ? */

{
    const char *FName = "MJobDistributeTaskGeometry";

    DBG(4, fSCHED)
    DPrint("%s(%s,RQ,NL,NI)\n", FName, (J != NULL) ? J->Name : "NULL");

    if ((J == NULL) || (RQ == NULL)) {
        return (FAILURE);
    }

    /* NYI */

    return (FAILURE);
} /* END MJobDistributeTaskGeometry() */

int MSchedTest()

{
    mjob_t tmpJ;
    mreq_t tmpRQ;

    mnalloc_t tmpNodeList[MAX_MNODE_PER_JOB];

    short tmpTaskMap[MAX_MTASK_PER_JOB];
    mnalloc_t tmpNodeList2[MAX_MNODE_PER_JOB];

    memset(&tmpJ, 0, sizeof(tmpJ));
    strcpy(tmpJ.Name, "test");
    tmpJ.Req[0] = &tmpRQ;
    tmpJ.Req[1] = NULL;
    tmpJ.Request.TC = 27;

    tmpRQ.NodeList = tmpNodeList;
    tmpRQ.RMIndex = 0;
    tmpRQ.TasksPerNode = 4;
    tmpRQ.TaskCount = 27;
    tmpRQ.NodeCount = 7;

    tmpNodeList[0].N = MNode[0];
    tmpNodeList[0].TC = 4;

    tmpNodeList[1].N = MNode[0];
    tmpNodeList[1].TC = 4;

    tmpNodeList[2].N = MNode[0];
    tmpNodeList[2].TC = 4;

    tmpNodeList[3].N = MNode[0];
    tmpNodeList[3].TC = 4;

    tmpNodeList[4].N = MNode[0];
    tmpNodeList[4].TC = 4;

    tmpNodeList[5].N = MNode[0];
    tmpNodeList[5].TC = 4;

    tmpNodeList[6].N = MNode[0];
    tmpNodeList[6].TC = 4;

    tmpNodeList[7].N = MNode[0];
    tmpNodeList[7].TC = 4;

    tmpNodeList[7].N = NULL;
    tmpNodeList[7].TC = 0;

    MRM[0].Type = mrmtPBS;

    MJobDistributeTasks(&tmpJ, &MRM[0], tmpNodeList2, tmpTaskMap);

    exit(0);

    /*NOTREACHED*/

    return (SUCCESS);
} /* END MSchedTest() */

int MJobSelectResourceSet(

    mjob_t *J,           /* I */
    mreq_t *RQ,          /* I */
    int SetType,         /* I */
    int SetSelection,    /* I */
    char **SetList,      /* I */
    mnalloc_t *NodeList, /* O: ? */
    int ListSize)        /* I */

{
    int nindex;
    int sindex;
    int sindex2;
    int findex;

    int SetNC[MAX_MATTR];
    int SetCount[MAX_MATTR];
    int SetIndex[MAX_MATTR];
    int EffSetCount[MAX_MATTR];

    double SetDistance[MAX_MATTR];

    int MinSetLoss[MAX_MATTR];

    double Distance;

    int MaxSet;
    int SetMembers;

    int Offset;
    int NodeIsFeasible;

    int TasksRequired;
    int NodesRequired;

    int BestSet;
    int SetValue;

    int SetOK;

    int TC;

    mnode_t *N;

    const char *FName = "MJobSelectResourceSet";

    DBG(4, fSCHED)
    DPrint("%s(%s,%d,%d,SetList,NodeList,%d)\n", FName,
           (J != NULL) ? J->Name : "NULL", SetType, SetSelection, ListSize);

    if ((J == NULL) || (NodeList == NULL) || (ListSize < 1) ||
        (ListSize > MAX_MNODE)) {
        return (FAILURE);
    }

    if ((SetType == mrstNONE) || (SetSelection == mrssNONE) ||
        (J->Flags & (1 << mjfHostList))) {
        /* no node set specified or job hostlist specified */

        return (SUCCESS);
    }

    TasksRequired = RQ->TaskCount;
    NodesRequired = RQ->NodeCount;

    memset(SetCount, 0, sizeof(SetCount));
    memset(SetNC, 0, sizeof(SetNC));

    memset(SetIndex, 0, sizeof(SetIndex));
    memset(SetDistance, 0, sizeof(SetDistance));
    memset(MinSetLoss, 0, sizeof(MinSetLoss));

    MaxSet = 0;
    SetMembers = 0;

    /*
      if ((NL = calloc(1,sizeof(mnalloc_t) * ListSize)) == NULL)
        {
        return(FAILURE);
        }
    */

    switch (SetType) {
        case mrstFeature:

            MaxSet = 0;

            if ((SetList != NULL) && (SetList[0] != NULL)) {
                for (sindex = 0; sindex < MAX_MATTR; sindex++) {
                    if ((SetList[sindex] == NULL) ||
                        (SetList[sindex][0] == '\0'))
                        break;

                    for (findex = 0; findex < MAX_MATTR; findex++) {
                        if (MAList[eFeature][findex][0] == '\0') break;

                        if (strcmp(MAList[eFeature][findex], SetList[sindex]))
                            continue;

                        SetIndex[MaxSet] = findex;
                        MaxSet++;

                        break;
                    } /* END for (findex) */
                }     /* END for (sindex) */
            }         /* END if (SetList != NULL) */
            else {
                /* set not specified, determine all existing sets */

                for (findex = 0; findex < MAX_MATTR; findex++) {
                    if (MAList[eFeature][findex][0] == '\0') break;

                    SetIndex[MaxSet] = findex;
                    MaxSet++;
                } /* END for (findex) */
            }     /* END else (SetList != NULL) */

            SetIndex[MaxSet] = -1;

            break;

        case mrstProcSpeed:

            MaxSet = 0;

            if ((SetList != NULL) && (SetList[0] != NULL)) {
                for (sindex = 0; sindex < MAX_MATTR; sindex++) {
                    if ((SetList[sindex] == NULL) ||
                        (SetList[sindex][0] == '\0'))
                        break;

                    SetIndex[sindex] = atoi(SetList[sindex]);

                    if (SetIndex[sindex] == 0) break;
                } /* END for (sindex) */

                MaxSet = sindex;
            } /* END if (SetList != NULL) */
            else {
                /* set not specified, determine all existing sets */

                SetIndex[0] = -1;

                for (nindex = 0; nindex < ListSize; nindex++) {
                    if ((NodeList[nindex].TC == 0) ||
                        (NodeList[nindex].N == NULL))
                        break;

                    N = NodeList[nindex].N;

                    for (findex = 0; findex < MAX_MATTR; findex++) {
                        if ((SetIndex[findex] == -1) ||
                            (SetIndex[findex] == N->ProcSpeed))
                            break;
                    } /* END for (findex) */

                    if (findex >= MAX_MATTR - 1) break;

                    if (SetIndex[findex] == -1) {
                        SetIndex[findex] = N->ProcSpeed;
                        SetIndex[findex + 1] = -1;

                        MaxSet = findex + 1;
                    }
                } /* END for (sindex) */

                SetIndex[MAX_MATTR - 1] = -1;
            } /* END else (SetList != NULL) */

            SetIndex[MaxSet] = -1;

            break;

        case mrstNONE:

            return (SUCCESS);

            /*NOTREACHED*/

            break;

        default:

            /* set not handled */

            MaxSet = 0;

            return (FAILURE);

            /*NOTREACHED*/

            break;
    } /* END switch(SetType) */

    for (nindex = 0; nindex < ListSize; nindex++) {
        if ((NodeList[nindex].TC == 0) || (NodeList[nindex].N == NULL)) break;

        N = NodeList[nindex].N;
        TC = NodeList[nindex].TC;

        switch (SetType) {
            case mrstFeature:

                /* step through set list */

                for (sindex = 0; sindex < MaxSet; sindex++) {
                    if (N->FBM[SetIndex[sindex] >> M64.INTLBITS] &
                        (1 << (SetIndex[sindex] % M64.INTBITS))) {
                        SetCount[sindex] += TC;
                        SetNC[sindex]++;

                        SetMembers += TC;

                        if (SetSelection == mrssAnyOf) break;
                    }
                } /* END for (sindex) */

                break;

            case mrstMemory:

                break;

            case mrstProcSpeed:

                /* step through set list */

                for (sindex = 0; sindex < MaxSet; sindex++) {
                    if (N->ProcSpeed == SetIndex[sindex]) {
                        SetCount[sindex] += TC;
                        SetNC[sindex]++;

                        SetMembers += TC;

                        if (SetSelection == mrssAnyOf) break;
                    }
                } /* END for (sindex) */

                break;

            default:

                break;
        } /* END switch(SetType) */
    }     /* END for (nindex)    */

    /* perform set level operations */

    memcpy(EffSetCount, SetCount, sizeof(EffSetCount));

    switch (SetType) {
        case mrstProcSpeed:

            if (MPar[0].NodeSetTolerance > 0.0) {
                for (sindex = 0; sindex < MaxSet; sindex++) {
                    EffSetCount[sindex] = SetCount[sindex];

                    MinSetLoss[sindex] = 0;

                    for (sindex2 = sindex + 1; sindex2 < MaxSet; sindex2++) {
                        Distance =
                            (double)(SetIndex[sindex2] - SetIndex[sindex]) /
                            SetIndex[sindex];

                        if (Distance > MPar[0].NodeSetTolerance) {
                            break;
                        }

                        if (EffSetCount[sindex] < TasksRequired) {
                            MinSetLoss[sindex] +=
                                (SetIndex[sindex2] - SetIndex[sindex]) *
                                MIN(SetCount[sindex2],
                                    TasksRequired - EffSetCount[sindex]);
                        }

                        EffSetCount[sindex] += SetCount[sindex2];
                    } /* END for (sindex2) */
                }     /* END for (sindex) */
            }         /* END if (MPar[0].NodeSetTolerance > 0.0) */

            break;

        default:

            break;
    } /* END switch(SetType) */

    DBG(6, fSCHED) {
        for (sindex = 0; sindex < MaxSet; sindex++) {
            DPrint("INFO:     set[%d] %d %d\n", sindex, SetIndex[sindex],
                   SetCount[sindex]);
        }
    }

    if (SetMembers < TasksRequired) {
        DBG(2, fSCHED)
        DPrint("INFO:     inadequate resources found in any set (%d < %d)\n",
               SetMembers, TasksRequired);

        return (FAILURE);
    }

    /* select resources from various sets */

    switch (SetSelection) {
        case mrssOneOf:

            /* select all sets which contain adequate resources to support job
             */

            /* verify at least one set contains adequate resources */

            /* NOTE:  set resources are allowed if set tolerance is specified *
             *        and border sets lie within tolerances                   */

            /* perform feasibility check */

            for (sindex = 0; sindex < MaxSet; sindex++) {
                if (EffSetCount[sindex] >= TasksRequired) {
                    /* set contains adequate resources */

                    break;
                }
            } /* END for (sindex) */

            if (sindex == MaxSet) {
                /* no set contains adequate resources */

                return (FAILURE);
            }

            /* at least one adequate set exists */

            Offset = 0;

            for (nindex = 0; nindex < ListSize; nindex++) {
                if (NodeList[nindex].N == NULL) break;

                N = NodeList[nindex].N;

                /* determine if node is a member of any feasible sets */

                NodeIsFeasible = FALSE;

                for (sindex = 0; sindex < MaxSet; sindex++) {
                    if ((SetCount[sindex] < TasksRequired) ||
                        (SetNC[sindex] < NodesRequired))
                        continue;

                    switch (SetType) {
                        case mrstFeature:

                            if (N->FBM[SetIndex[sindex] >> M64.INTLBITS] &
                                (1 << (SetIndex[sindex] % M64.INTBITS))) {
                                /* node is feasible */

                                if (Offset > 0)
                                    memcpy(&NodeList[nindex - Offset],
                                           &NodeList[nindex],
                                           sizeof(NodeList[0]));

                                NodeIsFeasible = TRUE;
                            }

                            break;

                        case mrstProcSpeed:

                            if (N->ProcSpeed == SetIndex[sindex]) {
                                /* node is feasible */

                                if (Offset > 0)
                                    memcpy(&NodeList[nindex - Offset],
                                           &NodeList[nindex],
                                           sizeof(NodeList[0]));

                                NodeIsFeasible = TRUE;
                            }

                        default:

                            break;
                    } /* END switch(SetType) */

                    if (NodeIsFeasible == TRUE) break;
                } /* END for (sindex)    */

                if (NodeIsFeasible == FALSE) Offset++;
            } /* END for (nindex)    */

            NodeList[nindex - Offset].N = NULL;

            break;

        case mrssAnyOf:

            /* select resources from all sets contained in SetList */

            break;

        case mrssFirstOf:

            /* select best set which contains adequate resources to support job
             */

            SetValue = -1;
            BestSet = -1;

            for (sindex = 0; sindex < MaxSet; sindex++) {
                if ((EffSetCount[sindex] < TasksRequired) ||
                    (SetNC[sindex] < NodesRequired))
                    continue;

                switch (MPar[0].NodeSetPriorityType) {
                    case mrspBestFit:

                        if ((SetValue == -1) ||
                            (EffSetCount[sindex] < SetValue)) {
                            BestSet = sindex;
                            SetValue = EffSetCount[sindex];
                        }

                        break;

                    case mrspWorstFit:

                        if (EffSetCount[sindex] > SetValue) {
                            BestSet = sindex;
                            SetValue = EffSetCount[sindex];
                        }

                        break;

                    case mrspMinLoss:

                        /* select set of resources with minimum delta between
                         * fastest and slowest nodes */

                        if (SetType != mrstProcSpeed) {
                            BestSet = sindex;

                            break;
                        }

                        if ((SetValue == -1) ||
                            (MinSetLoss[sindex] < SetValue)) {
                            /* select min span nodes */

                            BestSet = sindex;
                            SetValue = MinSetLoss[sindex];
                        }

                        break;

                    case mrspBestResource:
                    default:

                        if (SetType != mrstProcSpeed) {
                            BestSet = sindex;

                            break;
                        }

                        if (SetIndex[sindex] > SetValue) {
                            /* select fastest nodes */

                            BestSet = sindex;
                            SetValue = SetIndex[sindex];
                        }

                        break;
                } /* END switch (BestSetSelection) */

                if ((MPar[0].NodeSetPriorityType == mrspBestResource) &&
                    (SetType != mrstProcSpeed)) {
                    break;
                }
            } /* END for (sindex) */

            if (BestSet == -1) {
                /* no set contains adequate resources */

                return (FAILURE);
            }

            Offset = 0;

            for (nindex = 0; nindex < ListSize; nindex++) {
                if (NodeList[nindex].N == NULL) break;

                N = NodeList[nindex].N;

                /* determine if node is a member of selected set */

                switch (SetType) {
                    case mrstFeature:

                        if (N->FBM[SetIndex[BestSet] >> M64.INTLBITS] &
                            (1 << (SetIndex[BestSet] % M64.INTBITS))) {
                            /* node is in set */

                            if (Offset > 0)
                                memcpy(&NodeList[nindex - Offset],
                                       &NodeList[nindex], sizeof(NodeList[0]));
                        } else {
                            Offset++;
                        }

                        break;

                    case mrstProcSpeed:

                        SetOK = FALSE;

                        if (MPar[0].NodeSetTolerance > 0.0) {
                            if (N->ProcSpeed >= SetIndex[BestSet]) {
                                Distance = (double)(SetIndex[BestSet] -
                                                    SetIndex[sindex]) /
                                           SetIndex[sindex];

                                if (Distance <= MPar[0].NodeSetTolerance)
                                    SetOK = TRUE;
                            }
                        } else {
                            if (N->ProcSpeed == SetIndex[BestSet]) SetOK = TRUE;
                        }

                        if (SetOK == TRUE) {
                            /* node is in set */

                            if (Offset > 0)
                                memcpy(&NodeList[nindex - Offset],
                                       &NodeList[nindex], sizeof(NodeList[0]));
                        } else {
                            Offset++;
                        }

                        break;

                    default:

                        break;
                } /* END switch(SetType) */
            }     /* END for (nindex)    */

            NodeList[nindex - Offset].N = NULL;

            break;

        case mrssNONE:
        default:

            /* no set selection mode specified */

            return (SUCCESS);

            /*NOTREACHED*/

            break;
    } /* END SetSelection() */

    if (NodeList[0].N == NULL) return (FAILURE);

    return (SUCCESS);
} /* END MJobSelectResourceSet() */

int MJobAllocateBalanced(

    mjob_t *J,  /* IN: job allocating nodes                           */
    mreq_t *RQ, /* IN: req allocating nodes                           */
    mnalloc_t *NodeList, /* IN: eligible nodes */
    int RQIndex,       /* IN: index of job req to evaluate                   */
    int *MinTPN,       /* IN: min tasks per node allowed                     */
    int *MaxTPN,       /* IN: max tasks per node allowed                     */
    char *NodeMap,     /* IN: array of node alloc states                     */
    int AffinityLevel, /* IN: current reservation affinity level to evaluate */
    int *NodeIndex,    /* IN/OUT: index of next node to find in BestList     */
    mnalloc_t *BestList[MAX_MREQ_PER_JOB], /* IN/OUT: list of selected nodes */
    int *TaskCount, /* IN/OUT: total tasks allocated to req               */
    int *NodeCount) /* IN/OUT: total nodes allocated to req               */

{
    int nindex;
    int TC;

    mnode_t *N;

    int MinSpeed;
    int LastSpeed = -1;

    /* select slowest nodes first */

    while (TaskCount[RQIndex] < RQ->TaskCount) {
        MinSpeed = MAX_INT;

        /* determine slowest node */

        for (nindex = 0; NodeList[nindex].N != NULL; nindex++) {
            N = NodeList[nindex].N;
            TC = NodeList[nindex].TC;

            if (NodeMap[N->Index] != AffinityLevel) continue;

            if ((N->ProcSpeed > LastSpeed) && (N->ProcSpeed < MinSpeed)) {
                MinSpeed = N->ProcSpeed;
            }
        } /* END for (nindex) */

        if (MinSpeed == MAX_INT) {
            return (FAILURE);
        }

        /* allocate slowest nodes */

        for (nindex = 0; NodeList[nindex].N != NULL; nindex++) {
            N = NodeList[nindex].N;
            TC = NodeList[nindex].TC;

            if (NodeMap[N->Index] != AffinityLevel) {
                /* node unavailable */

                continue;
            }

            if (TC < MinTPN[RQIndex]) continue;

            TC = MIN(TC, MaxTPN[RQIndex]);

            if (N->ProcSpeed != MinSpeed) continue;

            /* populate BestList with selected node */

            BestList[RQIndex][NodeIndex[RQIndex]].N = N;
            BestList[RQIndex][NodeIndex[RQIndex]].TC = TC;

            NodeIndex[RQIndex]++;
            TaskCount[RQIndex] += TC;
            NodeCount[RQIndex]++;

            /* mark node as used */

            NodeMap[N->Index] = nmUnavailable;

            if (TaskCount[RQIndex] >= RQ->TaskCount) {
                /* all required tasks found */

                /* NOTE:  HANDLED BY DIST */

                if ((RQ->NodeCount == 0) ||
                    (NodeCount[RQIndex] >= RQ->NodeCount)) {
                    /* terminate BestList */

                    BestList[RQIndex][NodeIndex[RQIndex]].N = NULL;

                    break;
                }
            }
        } /* END for (nindex) */

        LastSpeed = MinSpeed;
    } /* END while (TaskCount[RQIndex] < RQ->TaskCount) */

    return (SUCCESS);
} /* END MJobAllocateBalanced() */

int MJobAllocatePriority(

    mjob_t *J,         /* I: job allocating nodes                         */
    mreq_t *RQ,        /* I: req allocating nodes                         */
    mnalloc_t *NL,     /* I: eligible nodes                               */
    int RQIndex,       /* I: index of job req to evaluate                 */
    int *MinTPN,       /* I: min tasks per node allowed                   */
    int *MaxTPN,       /* I: max tasks per node allowed                   */
    char *NodeMap,     /* I: array of node alloc states                   */
    int AffinityLevel, /* I: current reservation affinity level to evaluate */
    int *NodeIndex,    /* I/O: index of next node to find in BestList     */
    mnalloc_t *BestList[MAX_MREQ_PER_JOB], /* I/O: list of selected nodes */
    int *TaskCount, /* I/O: total tasks allocated to req               */
    int *NodeCount, /* I/O: total nodes allocated to req               */
    long StartTime) /* I */

{
    int NIndex;
    int nindex;

    unsigned short TC;

    mnode_t *N;

    mnpri_t NodePrio[MAX_MNODE_PER_FRAG];

    if ((NL == NULL) || (NL[0].N == NULL)) {
        return (FAILURE);
    }

    /* select highest priority nodes first */

    /* sort list by node priority */

    NIndex = 0;

    for (nindex = 0; NL[nindex].N != NULL; nindex++) {
        N = NL[nindex].N;

        /* N is pointer in node list */
        /* taskcount is node's priority    */

        if (NodeMap[N->Index] != AffinityLevel) {
            /* node unavailable */

            continue;
        }

        NodePrio[NIndex].N = NL[nindex].N;
        NodePrio[NIndex].TC = NL[nindex].TC;

        MNodeGetPriority(N, NodeMap[N->Index], N->IsPref,
                         &NodePrio[NIndex].Prio, StartTime);

        NIndex++;
    } /* END for (nindex) */

    NodePrio[NIndex].N = NULL;

    if (NIndex == 0) {
        return (FAILURE);
    }

    /* HACK:  machine prio and fastest first both use same qsort algo */

    qsort((void *)&NodePrio[0], NIndex, sizeof(mnpri_t),
          (int (*)(const void *, const void *))__MSchedQSortHLPComp);

    /* select first 'RQ->TaskCount' procs */

    for (nindex = 0; NodePrio[nindex].N != NULL; nindex++) {
        TC = NodePrio[nindex].TC;

        if ((TC == 0) || (TC < MinTPN[RQIndex])) {
            /* node unavailable */

            continue;
        }

        TC = MIN(TC, MaxTPN[RQIndex]);

        N = NodePrio[nindex].N;

        BestList[RQIndex][NodeIndex[RQIndex]].N = N;

        BestList[RQIndex][NodeIndex[RQIndex]].TC = TC;

        NodeIndex[RQIndex]++;
        TaskCount[RQIndex] += TC;
        NodeCount[RQIndex]++;

        /* mark node as used */

        NodeMap[N->Index] = nmUnavailable;

        if (TaskCount[RQIndex] >= RQ->TaskCount) {
            /* all required tasks found */

            /* NOTE:  HANDLED BY DIST */

            if ((RQ->NodeCount == 0) || (NodeCount[RQIndex] >= RQ->NodeCount)) {
                /* terminate BestList */

                BestList[RQIndex][NodeIndex[RQIndex]].N = NULL;

                break;
            }
        }
    } /* END for (nindex) */

    return (SUCCESS);
} /* END MJobAllocatePriority() */

int MJobAllocateFastest(

    mjob_t *J,  /* IN: job allocating nodes                           */
    mreq_t *RQ, /* IN: req allocating nodes                           */
    mnalloc_t *NodeList, /* IN: eligible nodes */
    int RQIndex,       /* IN: index of job req to evaluate                   */
    int *MinTPN,       /* IN: min tasks per node allowed                     */
    int *MaxTPN,       /* IN: max tasks per node allowed                     */
    char *NodeMap,     /* IN: array of node alloc states                     */
    int AffinityLevel, /* IN: current reservation affinity level to evaluate */
    int *NodeIndex,    /* IN/OUT: index of next node to find in BestList     */
    mnalloc_t *BestList[MAX_MREQ_PER_JOB], /* IN/OUT: list of selected nodes */
    int *TaskCount, /* I/O: total tasks allocated to req                  */
    int *NodeCount) /* I/O: total nodes allocated to req                  */

{
    int NIndex;
    int nindex;

    int TC;

    mnode_t *N;

    int UseNodeSpeed;

    mnalloc_t NodeSpeed[MAX_MNODE_PER_FRAG];

    if ((NodeList == NULL) || (NodeList[0].N == NULL)) {
        return (FAILURE);
    }

    /* select fastest nodes first */

    /* sort list by node speed/procspeed */

    memset(NodeSpeed, 0, sizeof(NodeSpeed));

    /* verify node speed not identical for all nodes */

    UseNodeSpeed = FALSE;

    for (nindex = 1; NodeList[nindex].N != NULL; nindex++) {
        if (NodeList[nindex].N->Speed != NodeList[0].N->Speed) {
            UseNodeSpeed = TRUE;

            break;
        }
    } /* END for (nindex) */

    NIndex = 0;

    for (nindex = 0; NodeList[nindex].N != NULL; nindex++) {
        /* N is pointer in node list */
        /* taskcount is node's speed */

        N = NodeList[nindex].N;

        if (NodeMap[N->Index] != AffinityLevel) {
            /* node unavailable */

            continue;
        }

        NodeSpeed[NIndex].N = (mnode_t *)&NodeList[nindex];

        if (UseNodeSpeed == TRUE) {
            NodeSpeed[NIndex].TC = (int)(N->Speed * 100.0);
        } else {
            NodeSpeed[NIndex].TC = N->ProcSpeed;
        }

        NIndex++;
    } /* END for (nindex) */

    if (NIndex == 0) {
        return (FAILURE);
    }

    /* NOTE:  machine prio and fastest first both use same qsort algo */

    qsort((void *)&NodeSpeed[0], NIndex, sizeof(mnalloc_t),
          (int (*)(const void *, const void *))__MSchedQSortHLComp);

    /* select first 'RQ->TaskCount' procs */

    for (nindex = 0; NodeSpeed[nindex].N != NULL; nindex++) {
        TC = ((mnalloc_t *)NodeSpeed[nindex].N)->TC;

        if ((TC == 0) || (TC < MinTPN[RQIndex])) {
            /* node unavailable */

            continue;
        }

        TC = MIN(TC, MaxTPN[RQIndex]);

        N = ((mnalloc_t *)NodeSpeed[nindex].N)->N;

        BestList[RQIndex][NodeIndex[RQIndex]].N = N;

        BestList[RQIndex][NodeIndex[RQIndex]].TC = (short)TC;

        NodeIndex[RQIndex]++;
        TaskCount[RQIndex] += TC;
        NodeCount[RQIndex]++;

        /* mark node as used */

        NodeMap[N->Index] = nmUnavailable;

        if (TaskCount[RQIndex] >= RQ->TaskCount) {
            /* all required tasks found */

            /* NOTE:  HANDLED BY DIST */

            if ((RQ->NodeCount == 0) || (NodeCount[RQIndex] >= RQ->NodeCount)) {
                /* terminate BestList */

                BestList[RQIndex][NodeIndex[RQIndex]].N = NULL;

                break;
            }
        }
    } /* END for (nindex) */

    return (SUCCESS);
} /* END MJobAllocateFastest() */

int MQueueGetBestRQTime(

    int *AQ, long *Time)

{
    long ETime;
    long CTime;

    long OETime;

    int jindex;

    double tmpValue;

    long BestTime;
    double BestValue;

    mjob_t *J;

    if (Time != NULL) *Time = -1;

    if (AQ == NULL) return (FAILURE);

    OETime = -1;

    /* AQ is list of active jobs pre sorted by completion time */

    /* determine final completion time */

    CTime = -1;

    for (jindex = 0; AQ[jindex] > 0; jindex++) {
        if (AQ[jindex + 1] == -1)
            CTime = MJob[AQ[jindex]]->StartTime + MJob[AQ[jindex]]->WCLimit;
    } /* END for (jindex) */

    if (CTime == -1) {
        /* no active jobs found */

        if (Time != NULL) *Time = MSched.Time;

        return (SUCCESS);
    }

    if (MQueueGetRequeueValue(AQ, MSched.Time, CTime, &tmpValue) != FAILURE) {
        BestTime = MSched.Time;
        BestValue = tmpValue;

        DBG(3, fSCHED)
        DPrint(
            "INFO:     requeue value %.2lf found for immediate action (T: "
            "%s)\n",
            BestValue, MULToTString(0));
    } else {
        BestTime = -1;
        BestValue = -99999.0;
    }

    /* determine cost at all job completion event times */

    for (jindex = 0; AQ[jindex] > 0; jindex++) {
        J = MJob[AQ[jindex]];

        ETime = J->StartTime + J->WCLimit;

        if (ETime <= OETime) continue;

        OETime = ETime;

        if (MQueueGetRequeueValue(AQ, ETime, CTime, &tmpValue) == FAILURE)
            continue;

        if ((BestTime == -1) || (tmpValue > BestValue)) {
            BestTime = ETime;
            BestValue = tmpValue;

            DBG(3, fSCHED)
            DPrint(
                "INFO:     requeue value %.2lf found at completion of job %s "
                "(T: %s)\n",
                BestValue, J->Name, MULToTString(ETime - MSched.Time));
        }
    } /* END for (jindex) */

    if (BestTime == -1) return (FAILURE);

    if (Time != NULL) *Time = BestTime;

    return (SUCCESS);
} /* END MQueueGetBestRQTime() */

int MQueueGetRequeueValue(

    int *AQ, time_t ETime, time_t CTime, double *Value)

{
    int jindex;

    double IPS = 0.0;
    double CPS = 0.0;

    int PC;

    mjob_t *J;

    /* value = IPS[E -> Ec] - CPS */

    if (AQ == NULL) {
        return (FAILURE);
    }

    for (jindex = 0; AQ[jindex] != -1; jindex++) {
        J = MJob[AQ[jindex]];

        PC = MJobGetProcCount(J);

        if ((J->StartTime + J->WCLimit) > ETime) {
            CPS += (double)(ETime - J->StartTime) * PC;
        }

        IPS += (double)(CTime - ETime) * PC;
    } /* END for (jindex) */

    if (Value != NULL) *Value = IPS - CPS;

    return (SUCCESS);
} /* END MQueueGetRequeueValue() */

int MSchedConfigShow(

    msched_t *S, /* I */
    int VFlag,   /* I */
    char *Buf,   /* O */
    int BufSize) /* I */

{
    int aindex;

    char *BPtr;
    int BSpace;

    char tmpLine[MAX_MLINE];
    char tmpAttr[MAX_MLINE];

    const int AList[] = {msaFBServer, msaMode, msaName, msaServer, -1};

    if ((S == NULL) || (Buf == NULL)) {
        return (FAILURE);
    }

    BPtr = tmpLine;
    BSpace = sizeof(tmpLine);

    BPtr[0] = '\0';

    for (aindex = 0; AList[aindex] != -1; aindex++) {
        if (MSchedAToString(S, AList[aindex], tmpAttr,
                            (VFlag == TRUE) ? (1 << mcmVerbose) : 0) ==
            FAILURE) {
            continue;
        }

        if (tmpAttr[0] == '\0') {
            continue;
        }

        MUSNPrintF(&BPtr, &BSpace, "%s=%s ", MSchedAttr[AList[aindex]],
                   tmpAttr);
    } /* END for (aindex) */

    if ((VFlag == TRUE) || (tmpLine[0] != '\0')) {
        MUShowSSArray(MCredCfgParm[mxoSched], S->Name, tmpLine, Buf);
    }

    return (SUCCESS);
} /* END MSchedConfigShow() */

int MSchedAToString(

    msched_t *S,           /* I */
    int AIndex, char *Buf, /* O */
    int Mode)

{
    if (Buf == NULL) {
        return (FAILURE);
    }

    Buf[0] = '\0';

    if (S == NULL) {
        return (FAILURE);
    }

    switch (AIndex) {
        case msaFBServer:

            if (S->FBServerHost[0] != '\0') {
                MUURLCreate(NULL, S->FBServerHost, NULL, S->FBServerPort, Buf,
                            MAX_MLINE);
            }

            break;

        case msaMode:

            strcpy(Buf, MSchedMode[S->Mode]);

            break;

        case msaName:

            strcpy(Buf, S->Name);

            break;

        case msaServer:

            MUURLCreate(NULL, S->ServerHost, NULL, S->ServerPort, Buf,
                        MAX_MLINE);

            break;

        default:

            /* NO-OP */

            break;
    } /* END switch(AIndex) */

    return (SUCCESS);
} /* END MSchedAToString() */

int MSchedOConfigShow(

    char *Buffer, /* O */
    int PIndex,   /* I */
    int VFlag)    /* I */

{
    msched_t *S;

    int index;

    const char *FName = "MSchedOConfigShow";

    DBG(4, fSCHED) DPrint("%s(Buffer,PIndex,VFlag)\n", FName);

    if (Buffer == NULL) {
        return (FAILURE);
    }

    S = &MSched;

    sprintf(temp_str, "%-30s  %s\n", MParam[pRMPollInterval],
            MULToTString(S->RMPollInterval));
    strcat(Buffer, temp_str);

    if (S->ComputeHost[0] != NULL) {
        sprintf(temp_str, "%-30s  %s", MParam[mcoComputeHosts],
                S->ComputeHost[0]);
        strcat(Buffer, temp_str);

        for (index = 1; S->ComputeHost[index] != NULL; index++) {
            sprintf(temp_str, " %s", S->ComputeHost[index]);
            strcat(Buffer, temp_str);
        } /* END for (index) */

        strcat(Buffer, "\n");

        for (index = 0; index < MAX_MNODE; index++) {
        }
    } /* END if (S->ComputeHost[0] != NULL) */

    if ((S->RMJobAggregationTime > 0) || VFlag) {
        sprintf(temp_str, "%-30s  %s\n", MParam[pJobAggregationTime],
                MULToTString(S->RMJobAggregationTime));
        strcat(Buffer, temp_str);
    }

    if (VFlag || (PIndex == -1) || (PIndex == pNAPolicy)) {
        sprintf(temp_str, "%-30s  %s\n", MParam[pNAPolicy],
                MNAccessPolicy[MSched.DefaultNAccessPolicy]);
        strcat(Buffer, temp_str);
    }

    if (VFlag || (PIndex == -1) || (MSched.AllocLocalityPolicy != malpNONE)) {
        sprintf(temp_str, "%-30s  %s\n", MParam[mcoAllocLocalityPolicy],
                MAllocLocalityPolicy[MSched.AllocLocalityPolicy]);
        strcat(Buffer, temp_str);
    }

    if (VFlag || (PIndex == -1) || (MSched.TimePolicy != mtpNONE)) {
        sprintf(temp_str, "%-30s  %s\n", MParam[mcoTimePolicy],
                MTimePolicy[MSched.TimePolicy]);
        strcat(Buffer, temp_str);
    }

    /* display security parameters */

    sprintf(temp_str, "%-30s  %s", MParam[mcoAdmin1Users], S->Admin1User[0]);
    strcat(Buffer, temp_str);

    for (index = 1; S->Admin1User[index][0] != '\0'; index++) {
        sprintf(temp_str, " %s", S->Admin1User[index]);
        strcat(Buffer, temp_str);
    } /* END for (index) */

    strcat(Buffer, "\n");

    if (S->Admin2User[0][0] != '\0') {
        sprintf(temp_str, "%-30s  %s", MParam[mcoAdmin2Users],
                S->Admin2User[0]);
        strcat(Buffer, temp_str);

        for (index = 1; S->Admin2User[index][0] != '\0'; index++) {
            sprintf(temp_str, " %s", S->Admin2User[index]);
            strcat(Buffer, temp_str);
        } /* END for (index) */

        strcat(Buffer, "\n");
    }

    if (S->Admin3User[0][0] != '\0') {
        sprintf(temp_str, "%-30s  %s", MParam[mcoAdmin3Users],
                S->Admin3User[0]);
        strcat(Buffer, temp_str);

        for (index = 1; S->Admin3User[index][0] != '\0'; index++) {
            sprintf(temp_str, " %s", S->Admin3User[index]);
            strcat(Buffer, temp_str);
        } /* END for (index) */

        strcat(Buffer, "\n");
    }

    if (S->Admin4User[0][0] != '\0') {
        sprintf(temp_str, "%-30s  %s", MParam[mcoAdmin4Users],
                S->Admin4User[0]);
        strcat(Buffer, temp_str);

        for (index = 1; S->Admin4User[index][0] != '\0'; index++) {
            sprintf(temp_str, " %s", S->Admin4User[index]);
            strcat(Buffer, temp_str);
        } /* END for (index) */

        strcat(Buffer, "\n");
    }

    if ((S->AdminHost[0][0] != '\0') ||
        (VFlag || (PIndex == -1) || (PIndex == mcoAdminHosts))) {
        sprintf(temp_str, "%-30s  %s", MParam[mcoAdminHosts], S->AdminHost[0]);
        strcat(Buffer, temp_str);

        for (index = 1; index < MAX_MADMINHOSTS; index++) {
            if (S->AdminHost[index][0] == '\0') break;

            sprintf(temp_str, "  %s", S->AdminHost[index]);
            strcat(Buffer, temp_str);
        } /* END for (index) */

        strcat(Buffer, "\n");
    }

    sprintf(temp_str, "%-30s  %d\n", MParam[pNodePollFrequency],
            S->NodePollFrequency);
    strcat(Buffer, temp_str);

    if ((PIndex == pDisplayFlags) || (S->DisplayFlags != 0) ||
        (VFlag || (PIndex == -1))) {
        char tmpLine[MAX_MLINE];

        tmpLine[0] = '\0';

        for (index = 0; MCDisplayType[index] != NULL; index++) {
            if (S->DisplayFlags & (1 << index)) {
                if (tmpLine[0] != '\0') strcat(tmpLine, " ");

                strcat(tmpLine, MCDisplayType[index]);
            }
        }

        sprintf(temp_str, "%-30s  %s\n", MParam[pDisplayFlags], tmpLine);
        strcat(Buffer, temp_str);
    } /* END if ((PIndex == pDisplayFlags) || ...) */

    if ((S->DefaultDomain[0] != '\0') ||
        (VFlag || (PIndex == -1) || (PIndex == pDefaultDomain))) {
        sprintf(temp_str, "%-30s  %s\n", MParam[pDefaultDomain],
                S->DefaultDomain);
        strcat(Buffer, temp_str);
    }

    if (!strcmp(S->DefaultClassList, DEFAULT_CLASSLIST) ||
        (VFlag || (PIndex == -1) || (PIndex == pDefaultClassList))) {
        sprintf(temp_str, "%-30s  %s\n", MParam[pDefaultClassList],
                S->DefaultClassList);
        strcat(Buffer, temp_str);
    }

    if ((S->NodeTypeFeatureHeader[0] != '\0') ||
        (VFlag || (PIndex == -1) || (PIndex == pNodeTypeFeatureHeader))) {
        sprintf(temp_str, "%-30s  %s\n", MParam[pNodeTypeFeatureHeader],
                S->NodeTypeFeatureHeader);
        strcat(Buffer, temp_str);
    }

    if ((S->ProcSpeedFeatureHeader[0] != '\0') ||
        (VFlag || (PIndex == -1) || (PIndex == pProcSpeedFeatureHeader))) {
        sprintf(temp_str, "%-30s  %s\n", MParam[pProcSpeedFeatureHeader],
                S->ProcSpeedFeatureHeader);
        strcat(Buffer, temp_str);
    }

    if ((S->PartitionFeatureHeader[0] != '\0') ||
        (VFlag || (PIndex == -1) || (PIndex == pPartitionFeatureHeader))) {
        sprintf(temp_str, "%-30s  %s\n", MParam[pPartitionFeatureHeader],
                S->PartitionFeatureHeader);
        strcat(Buffer, temp_str);
    }

    /* general sched config */

    sprintf(temp_str, "%-30s  %s\n", MParam[mcoDeferTime],
            MULToTString(S->DeferTime));
    strcat(Buffer, temp_str);
    sprintf(temp_str, "%-30s  %d\n", MParam[pDeferCount], S->DeferCount);
    strcat(Buffer, temp_str);
    sprintf(temp_str, "%-30s  %d\n", MParam[pDeferStartCount],
            S->DeferStartCount);
    strcat(Buffer, temp_str);

    sprintf(temp_str, "%-30s  %ld\n", MParam[pJobPurgeTime], S->JobPurgeTime);
    strcat(Buffer, temp_str);
    sprintf(temp_str, "%-30s  %ld\n", MParam[pNodePurgeTime], S->NodePurgeTime);
    strcat(Buffer, temp_str);

    sprintf(temp_str, "%-30s  %d\n", MParam[pAPIFailureThreshhold],
            S->APIFailureThreshhold);
    strcat(Buffer, temp_str);

    sprintf(temp_str, "%-30s  %ld\n", MParam[pNodeSyncDeadline],
            S->NodeSyncDeadline);
    strcat(Buffer, temp_str);
    sprintf(temp_str, "%-30s  %ld\n", MParam[pJobSyncDeadline],
            S->JobSyncDeadline);
    strcat(Buffer, temp_str);
    sprintf(temp_str, "%-30s  %s\n", MParam[pJobMaxOverrun],
            MULToTString(S->JobMaxOverrun));
    strcat(Buffer, temp_str);

    if ((PIndex == pNodeMaxLoad) || (S->DefaultN.MaxLoad > 0.0) || VFlag ||
        (PIndex == -1)) {
        sprintf(temp_str, "%-30s  %.1lf\n", MParam[pNodeMaxLoad],
                S->DefaultN.MaxLoad);
        strcat(Buffer, temp_str);
    }

    strcat(Buffer, "\n");

    /* display stat graph parameters */

    sprintf(temp_str, "%-30s  %ld\n", MParam[pPlotMinTime], MStat.P.MinTime);
    strcat(Buffer, temp_str);
    sprintf(temp_str, "%-30s  %ld\n", MParam[pPlotMaxTime], MStat.P.MaxTime);
    strcat(Buffer, temp_str);
    sprintf(temp_str, "%-30s  %d\n", MParam[pPlotTimeScale],
            MStat.P.TimeStepCount);
    strcat(Buffer, temp_str);

    sprintf(temp_str, "%-30s  %d\n", MParam[pPlotMinNode], MStat.P.MinNode);
    strcat(Buffer, temp_str);
    sprintf(temp_str, "%-30s  %d\n", MParam[pPlotMaxNode], MStat.P.MaxNode);
    strcat(Buffer, temp_str);
    sprintf(temp_str, "%-30s  %d\n", MParam[pPlotNodeScale],
            MStat.P.NodeStepCount);
    strcat(Buffer, temp_str);

    DBG(4, fUI) DPrint("INFO:     plot parameters displayed\n");

    return (SUCCESS);
} /* END MSchedOConfigShow() */

int MSchedSetDefaults(

    msched_t *S) /* I */

{
    char *ptr;

    int count;
    char *buf;

    const char *FName = "MSchedSetDefaults";

    DBG(4, fSCHED) DPrint("%s(S)\n", FName);

    MSched.SecureMode = TRUE;

    /* set scheduler defaults */

    MSched.PID = MOSGetPID();

    strcpy(MSched.Version, MSCHED_VERSION);

    /* randomize seed */

    srand(MSched.PID);

    /* setup home directory */

    MSchedSetAttr(&MSched, msaHomeDir, (void **)MBUILD_HOMEDIR, mdfString, 0);

    if ((ptr = getenv(MSCHED_ENVHOMEVAR)) != NULL) {
        DBG(4, fCONFIG)
        DPrint("INFO:     using %s environment variable (value: %s)\n",
               MSCHED_ENVHOMEVAR, ptr);

        MSchedSetAttr(&MSched, msaHomeDir, (void **)ptr, mdfString, 0);

        MSched.SecureMode = FALSE;
    } else if ((ptr = getenv(MParam[pMServerHomeDir])) != NULL) {
        DBG(4, fCONFIG)
        DPrint("INFO:     using %s environment variable (value: %s)\n",
               MParam[pMServerHomeDir], ptr);

        MSchedSetAttr(&MSched, msaHomeDir, (void **)ptr, mdfString, 0);

        MSched.SecureMode = FALSE;
    } else {
        /* check master configfile */

        if ((buf = MFULoad(MASTER_CONFIGFILE, 1, macmRead, &count, NULL)) !=
            NULL) {
            if ((ptr = strstr(buf, MParam[pMServerHomeDir])) != NULL) {
                ptr += (strlen(MParam[pMServerHomeDir]) + 1);

                sscanf(ptr, "%128s", MSched.HomeDir);
            }
        }
    }

    if (MSched.HomeDir[strlen(MSched.HomeDir) - 1] != '/')
        strcat(MSched.HomeDir, "/");

    MSched.ServerPort = DEFAULT_MSERVERPORT;
    MSched.Mode = DEFAULT_MSERVERMODE;

    MSched.DefaultCSAlgo = MDEF_CSALGO;

    MSched.ResDepth = DEFAULT_RES_DEPTH;

    MCP.CPInterval = DEFAULT_CHECKPOINTINTERVAL;
    MCP.CPExpirationTime = DEFAULT_CHECKPOINTEXPIRATIONTIME;

    strcpy(S->ServerHost, DEFAULT_MSERVERHOST);

    strcpy(S->DefaultDomain, DEFAULT_DOMAIN);

    strcpy(S->Action[mactJobFB], DEFAULT_MJOBFBACTIONPROGRAM);

    S->DefaultNAccessPolicy = DEFAULT_MNACCESSPOLICY;
    S->AllocLocalityPolicy = DEFAULT_MSCHEDALLOCLOCALITYPOLICY;

    S->StepCount = DEFAULT_SchedSTEPCOUNT;
    S->LogFileMaxSize = DEFAULT_MLOGFILEMAXSIZE;
    S->LogFileRollDepth = DEFAULT_MLOGFILEROLLDEPTH;

    S->SPVJobIsPreemptible = MDEF_SPVJOBISPREEMPTIBLE;

    S->DefaultJ.SpecWCLimit[1] = MDEF_SYSJOBWCLIMIT;

    S->Iteration = -1;
    S->Schedule = TRUE;
    S->Interval = 0;

    gettimeofday(&S->SchedTime, NULL);

    strcpy(S->Admin1User[0], DEFAULT_SchedADMIN);
    S->Admin1User[1][0] = '\0';

    S->Admin2User[0][0] = '\0';
    S->Admin3User[0][0] = '\0';
    S->Admin4User[0][0] = '\0';

    strcpy(S->AdminHost[0], "ALL");
    S->AdminHost[1][0] = '\0';

    S->DeferTime = DEFAULT_DEFERTIME;
    S->DeferCount = DEFAULT_DEFERCOUNT;
    S->DeferStartCount = DEFAULT_DEFERSTARTCOUNT;

    S->JobPurgeTime = DEFAULT_JOBPURGETIME;
    S->NodePurgeTime = MDEF_NODEPURGETIME;

    S->APIFailureThreshhold = DEFAULT_APIFAILURETHRESHHOLD;

    S->NodeSyncDeadline = MDEF_NODESYNCDEADLINE;
    S->JobSyncDeadline = DEFAULT_JOBSYNCDEADLINE;
    S->JobMaxOverrun = DEFAULT_MAXJOBOVERRUN;

    S->TraceFlags = DEFAULT_TRACEFLAGS;

    S->MaxOSCPULoad = MAX_OS_CPU_OVERHEAD;

    S->StartTime = MSched.Time;
    S->RMPollInterval = DEFAULT_RMPOLLINTERVAL;
    S->MaxSleepIteration = DEFAULT_MAXSLEEPITERATION;
    S->PreemptPolicy = DEFAULT_MPREEMPTPOLICY;
    S->Mode = DEFAULT_MSERVERMODE;

    S->ReferenceProcSpeed = MAX_INT;

    strcpy(S->DefaultClassList, DEFAULT_CLASSLIST);

    MSched.ActionInterval = MDEF_ADMINACTIONINTERVAL;

    strcpy(S->Action[mactAdminEvent], DEFAULT_MADMINEACTIONPROGRAM);

    /* setup lock file */

    if (!strstr(S->HomeDir, DEFAULT_SchedLOCKFILE)) {
        sprintf(S->LockFile, "%s%s", S->HomeDir, DEFAULT_SchedLOCKFILE);
    } else {
        strcpy(S->LockFile, DEFAULT_SchedLOCKFILE);
    }

    DBG(5, fCONFIG) DPrint("INFO:     default LockFile: '%s'\n", S->LockFile);

    /* setup checkpoint file */

    if (!strstr(S->HomeDir, DEFAULT_CHECKPOINTFILE)) {
        sprintf(MCP.CPFileName, "%s%s", S->HomeDir, DEFAULT_CHECKPOINTFILE);
    } else {
        strcpy(MCP.CPFileName, DEFAULT_CHECKPOINTFILE);
    }

    DBG(5, fCONFIG)
    DPrint("INFO:     default CheckPointFile: '%s'\n", MCP.CPFileName);

    /* setup log directory */

    if (!strstr(S->HomeDir, DEFAULT_MLOGDIR)) {
        sprintf(S->LogDir, "%s%s", S->HomeDir, DEFAULT_MLOGDIR);
    } else {
        strcpy(S->LogDir, DEFAULT_MLOGDIR);
    }

    /* setup tools directory */

    if (!strstr(S->HomeDir, DEFAULT_SchedTOOLSDIR)) {
        sprintf(S->ToolsDir, "%s%s", S->HomeDir, DEFAULT_SchedTOOLSDIR);
    } else {
        strcpy(S->ToolsDir, DEFAULT_SchedTOOLSDIR);
    }

    /* setup config file */

    if (!strstr(S->HomeDir, DEFAULT_SchedCONFIGFILE)) {
        sprintf(S->ConfigFile, "%s%s", S->HomeDir, DEFAULT_SchedCONFIGFILE);
    } else {
        strcpy(S->ConfigFile, DEFAULT_SchedCONFIGFILE);
    }

    if ((ptr = getenv(MParam[pSchedConfigFile])) != NULL) {
        DBG(4, fCORE) {
            fprintf(stderr,
                    "INFO:     using %s environment variable (value: %s)\n",
                    MParam[pSchedConfigFile], ptr);
        }

        MUStrCpy(S->ConfigFile, ptr, sizeof(S->ConfigFile));

        S->SecureMode = FALSE;
    }

    /* set up default scheduler values */

    if ((ptr = getenv("LOADL_CONFIG")) != NULL) {
        MUStrCpy(S->LLConfigFile, ptr, sizeof(S->LLConfigFile));
    } else {
        S->LLConfigFile[0] = '\0';
    }

    S->GP = &MPar[0];

    if (getenv(MSCHED_ENVDBGVAR) != NULL) {
        MSched.DebugMode = TRUE;
    }

    return (SUCCESS);
} /* END MSchedSetDefaults() */

int MSchedToString(

    msched_t *S, /* I */
    char *Buf)   /* O */

{
    int aindex;

    char tmpString[MAX_MLINE];

    const int CPAList[] = {msaCPVersion, -1};

    mxml_t *E = NULL;

    if ((S == NULL) || (Buf == NULL)) {
        return (FAILURE);
    }

    MXMLCreateE(&E, "sched");

    for (aindex = 0; CPAList[aindex] != -1; aindex++) {
        switch (aindex) {
            case msaCPVersion:

                strcpy(tmpString, MCP.WVersion);

                break;

            default:

                tmpString[0] = '\0';

                break;
        } /* END switch(aindex) */

        if (tmpString[0] == '\0') continue;

        MXMLSetAttr(E, (char *)MSchedAttr[CPAList[aindex]], tmpString,
                    mdfString);
    } /* END for (aindex) */

    MXMLToString(E, Buf, MAX_MBUFFER, NULL, TRUE);

    MXMLDestroyE(&E);

    return (SUCCESS);
} /* END MSchedToString() */

int MSchedFromString(

    msched_t *S, /* I (modified) */
    char *Buf)   /* I */

{
    int aindex;

    char tmpString[MAX_MLINE];

    mxml_t *E = NULL;

    const int CPAList[] = {msaCPVersion, -1};

    if ((S == NULL) || (Buf == NULL)) {
        return (FAILURE);
    }

    MXMLFromString(&E, Buf, NULL, NULL);

    for (aindex = 0; CPAList[aindex] != -1; aindex++) {
        if (MXMLGetAttr(E, (char *)MSchedAttr[CPAList[aindex]], NULL, tmpString,
                        sizeof(tmpString)) == SUCCESS) {
            if (tmpString[0] != '\0')
                MSchedSetAttr(S, CPAList[aindex], (void **)tmpString, 0, 0);
        }
    } /* END for (aindex) */

    MXMLDestroyE(&E);

    return (SUCCESS);
} /* END MSchedFromString() */

int MSchedStatToString(

    msched_t *S, /* I */
    int Mode,    /* I */
    char *Buf,   /* O */
    int BufSize) /* I */

{
    const char *FName = "MSchedStatToString";

    DBG(2, fUI) DPrint("%s(S,%d,Buf,BufSize)\n", FName, Mode);

    if ((S == NULL) || (Buf == NULL)) {
        return (FAILURE);
    }

    switch (Mode) {
        case mwpXML:

        {
            int AList[] = {mstaTJobCount,  mstaTNJobCount,
                           mstaTQueueTime, mstaMQueueTime,
                           mstaTReqWTime,  mstaTExeWTime,
                           mstaTMSAvl,     mstaTMSDed,
                           mstaTPSReq,     mstaTPSExe,
                           mstaTPSDed,     mstaTPSUtl,
                           mstaTJobAcc,    mstaTNJobAcc,
                           mstaTXF,        mstaTNXF,
                           mstaMXF,        mstaTBypass,
                           mstaMBypass,    mstaTQOSAchieved,
                           mstaInitTime,   -1};

            MStatToString(&MPar[0].S, Buf, AList);

            /* NYI */
        } /* END BLOCK */

        break;

        default:

        {
            long RunTime;

            must_t *T;

            mpar_t *GP;

            /* build stat buffer */

            /* set up scheduler run time */

            RunTime = MStat.SchedRunTime;

            GP = &MPar[0];
            T = &GP->S;

            /*            CTM STM ITM RTM IJ EJ AJ UN UP UM IN IP IM CT SJ TPA
             * TPB SPH TMA TMD QP AQP NJA JAC PSX IT RPI WEF WI MXF ABP MBP AQT
             * MQT PSR PSD PSU MSA MSD JE */

            sprintf(
                Buf,
                "%ld\n%ld %ld %ld %d %d %d %d %d %d %d %d %d %d %d %lf %lf %lf "
                "%lf %lf %d %lu %lf %lf %lf %d %ld %lf %d %lf %lf %d %lf %lu "
                "%lf %lf %lf %lf %lf %d %d %lf\n",
                S->Time, S->StartTime, MStat.InitTime, RunTime,
                MStat.EligibleJobs, MStat.IdleJobs, MStat.ActiveJobs,
                GP->UpNodes, GP->URes.Procs, GP->URes.Mem, GP->IdleNodes,
                MIN(GP->ARes.Procs, (GP->URes.Procs - GP->DRes.Procs)),
                MIN(GP->ARes.Mem, (GP->URes.Mem - GP->DRes.Mem)), T->Count,
                MStat.SuccessfulJobsCompleted, MStat.TotalPHAvailable,
                MStat.TotalPHBusy, MStat.SuccessfulPH, T->MSAvail,
                T->MSDedicated, GP->L.IP->Usage[mptMaxPS][0],
                (S->Iteration > 0) ? MStat.AvgQueuePH / S->Iteration : 0,
                T->NJobAcc, T->JobAcc,
                (double)((T->PSRun > 0.0) ? T->PSXFactor / T->PSRun : 0.0),
                S->Iteration, S->RMPollInterval, MStat.MinEff,
                MStat.MinEffIteration, T->MaxXFactor,
                (double)((T->Count > 0) ? (double)T->Bypass / T->Count : 0.0),
                T->MaxBypass,
                (double)((T->Count > 0) ? (double)T->TotalQTS / T->Count : 0.0),
                T->MaxQTS, T->PSRun, T->PSDedicated, T->PSUtilized,
                T->MSDedicated, T->MSUtilized, MStat.JobsEvaluated,
                MStat.TotalPreemptJobs,
                (double)MStat.TotalPHPreempted / 3600.0);
        } /* END BLOCK */
    }     /* END switch(Mode) */

    return (SUCCESS);
} /* END MSchedStatToString() */

int MSchedSetAttr(

    msched_t *S, /* I (modified) */
    int AIndex,  /* I */
    void **AVal, /* I */
    int Format,  /* I */
    int Mode)    /* I */

{
    if ((S == NULL) || (AVal == NULL)) {
        return (FAILURE);
    }

    switch (AIndex) {
        case msaCPVersion:

        {
            char *ptr;

            if ((ptr = strrchr((char *)AVal, '.')) != NULL) {
                *ptr = '\0';
            }

            strcpy(MCP.DVersion, (char *)AVal);
        }

        break;

        case msaHomeDir:

            MUStrCpy(S->HomeDir, (char *)AVal, sizeof(S->HomeDir));

            /* append '/' if necessary */

            if (S->HomeDir[strlen(S->HomeDir) - 1] != '/')
                MUStrCat(S->HomeDir, "/", sizeof(S->HomeDir));

            /* set CWD to home directory */

            if (chdir(S->HomeDir) == -1) {
                perror("cannot change directory");

                DBG(0, fALL)
                DPrint(
                    "ERROR:    cannot change directory to '%s', errno: %d "
                    "(%s)\n",
                    S->HomeDir, errno, strerror(errno));
            }

            break;

        case msaName:

            MUStrCpy(S->Name, (char *)AVal, sizeof(S->Name));

            break;

        default:

            /* NO-OP */

            break;
    } /* END switch(AIndex) */

    return (SUCCESS);
} /* END MSchedSetAttr() */

int MResAllocateRE(

    char *PBuf,          /* I/O */
    int PBufSize,        /* I */
    int PIndex,          /* I */
    mnalloc_t *NodeList, /* O */
    int *TaskCount,      /* O (optional) */
    long StartTime,      /* I */
    long EndTime,        /* I */
    char *MsgBuf,        /* O (optional) */
    int MBufSize,        /* I (optional) */
    mreq_t *SpecRQ)      /* O */

{
    int cindex;
    int index;

    mjob_t tmpJ;
    mjob_t *J;

    macl_t tmpACL[MAX_MACL];
    macl_t tmpCL[MAX_MACL];

    mreq_t tmpRQ;
    mreq_t *RQ;

    mnalloc_t tmpNodeList[MAX_MNODE + 1];

    mnodelist_t MNodeList;

    int TaskReqCount;
    char TaskCmp[3];

    int tmpTaskCount;
    int tmpNodeCount;

    mpar_t *tmpP;

    long BestTime;

    char *ptr;

    char *BPtr;
    int BSpace;

    char *MPtr;
    int MSpace;

    /* FORMAT:  TASKS [ >= | == ] <TASKCOUNT> */

    const char *FName = "MResAllocateRE";

    DBG(3, fUI)
    DPrint("%s(%s,%d,%d,NodeList,TaskCount,%ld,%ld,MsgBuf,%d,DRes)\n", FName,
           PBuf, PBufSize, PIndex, StartTime, EndTime, MBufSize);

    if (TaskCount != NULL) *TaskCount = 0;

    if (PBuf == NULL) {
        return (FAILURE);
    }

    MPtr = MsgBuf;

    if (MPtr != NULL) {
        MSpace = MBufSize;

        MPtr[0] = '\0';
    }

    if ((ptr = strstr(PBuf, "TASKS")) != NULL) {
        ptr += strlen("TASKS");

        while ((isspace(*ptr) || (*ptr == '*')) && (*ptr != '\0')) ptr++;

        /* get memory comparison */

        TaskCmp[0] = *ptr;

        ptr++;

        if (*ptr == '=') {
            TaskCmp[1] = *ptr;
            TaskCmp[2] = '\0';

            ptr++;
        } else {
            TaskCmp[1] = '\0';
        }

        for (cindex = 0; MComp[cindex] != NULL; cindex++) {
            if (!strcmp(MComp[cindex], TaskCmp)) {
                break;
            }
        } /* END for (cindex) */

        if ((cindex != mcmpEQ) && (cindex != mcmpGE)) {
            if (MPtr != NULL) {
                MUSNPrintF(&MPtr, &MSpace,
                           "ERROR:    cannot parse expression '%s'\n", PBuf);
            }

            return (FAILURE);
        }

        while ((isspace(*ptr) || (*ptr == '*')) && (*ptr != '\0')) ptr++;

        TaskReqCount = (int)strtol(ptr, NULL, 10);

        if ((TaskReqCount == 0) || (TaskReqCount > MAX_MTASK)) {
            if (MPtr != NULL) {
                MUSNPrintF(&MPtr, &MSpace,
                           "ERROR:    cannot parse expression '%s'\n", PBuf);
            }

            return (FAILURE);
        }
    } else {
        if (MPtr != NULL) {
            MUSNPrintF(&MPtr, &MSpace,
                       "ERROR:    cannot parse expression '%s'\n", PBuf);
        }

        return (FAILURE);
    }

    DBG(3, fUI)
    DPrint("INFO:     setres requesting 'TASKS %s %d'\n", MComp[cindex],
           TaskReqCount);

    J = &tmpJ;
    RQ = &tmpRQ;

    if (SpecRQ != NULL) {
        memcpy(RQ, SpecRQ, sizeof(tmpRQ));
    } else {
        memset(RQ, 0, sizeof(tmpRQ));
    }

    MJobMkTemp(J, RQ, tmpACL, tmpCL, NULL, tmpNodeList);

    strcpy(J->Name, "setres");

    RQ->TaskCount = TaskReqCount;

    J->Request.TC = TaskReqCount;
    J->WCLimit = EndTime - StartTime;
    J->SpecWCLimit[0] = J->WCLimit;

    if (PIndex == 0)
        tmpP = NULL;
    else
        tmpP = &MPar[PIndex];

    BestTime = StartTime;

    if (MJobGetEStartTime(J, &tmpP, &tmpNodeCount, &tmpTaskCount, MNodeList,
                          &BestTime) == SUCCESS) {
        if (BestTime > StartTime) {
            if (MPtr != NULL) {
                MUSNPrintF(
                    &MPtr, &MSpace,
                    "ERROR:    cannot select %d tasks for reservation for %s\n",
                    TaskReqCount, MULToTString(BestTime - MSched.Time));
            }

            DBG(1, fSCHED)
            DPrint("WARNING:  cannot select requested tasks for reservation\n");

            return (FAILURE);
        }
    } else {
        if (MPtr != NULL) {
            MUSNPrintF(
                &MPtr, &MSpace,
                "ERROR:    only %d tasks/%d nodes available for reservation\n",
                tmpTaskCount, tmpNodeCount);
        }

        return (FAILURE);
    }

    BPtr = PBuf;
    BSpace = PBufSize;

    BPtr[0] = '\0';

    MUNLCopy(NodeList, MNodeList, 0, tmpNodeCount);

    for (index = 0; index < tmpNodeCount; index++) {
        if (BSpace < 100) {
            DBG(0, fSCHED) DPrint("ERROR:    regex buffer overflow\n");

            MUSNPrintF(&MPtr, &MSpace,
                       "ERROR:    cannot maintain regex for reservation\n");

            return (FAILURE);
        }

        MUSNPrintF(&MPtr, &MSpace, "node '%s' added\n",
                   NodeList[index].N->Name);

        if (index != 0) {
            MUSNPrintF(&BPtr, &BSpace, "|%s", NodeList[index].N->Name);
        } else {
            MUSNPrintF(&BPtr, &BSpace, "%s", NodeList[index].N->Name);
        }
    } /* END for (index) */

    if (TaskCount != NULL) *TaskCount = TaskReqCount;

    return (SUCCESS);
} /* END MResAllocateRE() */

int MSchedProcessOConfig(

    msched_t *S,                                             /* I */
    int PIndex,                                              /* I */
    int IVal,                                                /* I */
    double DVal, char *SVal, char **SArray, char *IndexName) /* I */

{
    mpar_t *P;

    if (S == NULL) {
        return (FAILURE);
    }

    P = &MPar[0];

    switch (PIndex) {
        case mcoAllocLocalityPolicy:

        {
            char *ptr;
            char *TokPtr;

            /* FORMAT:  <POLICY>[:<MARGIN>[:<OBJECT>]] */

            if ((ptr = MUStrTok(SVal, ":", &TokPtr)) != NULL) {
                S->AllocLocalityPolicy = MUGetIndex(
                    ptr, MAllocLocalityPolicy, FALSE, S->AllocLocalityPolicy);

                /* NYI:  Margin,Object */
            }
        } /* END BLOCK */

        break;

        case pCheckPointFile:

            if ((SVal[0] != '~') && (SVal[0] != '/')) {
                sprintf(MCP.CPFileName, "%s%s", S->HomeDir, SVal);
            } else {
                MUStrCpy(MCP.CPFileName, SVal, sizeof(MCP.CPFileName));
            }

            break;

        case pCheckPointInterval:

            MCP.CPInterval = MUTimeFromString(SVal);

            break;

        case pCheckPointExpirationTime:

            if (!strcmp(SVal, "-1") || strstr(SVal, "INFIN") ||
                strstr(SVal, "NONE") || strstr(SVal, "UNLIMITED"))
                MCP.CPExpirationTime = MAX_MTIME;
            else
                MCP.CPExpirationTime = MUTimeFromString(SVal);

            break;

        case mcoComputeHosts:

            if (SArray == NULL) break;

            {
                int index;

                for (index = 0; MAX_MNODE; index++) {
                    if (SArray[index] == NULL) break;

                    MUStrDup(&S->ComputeHost[index], SArray[index]);

                    DBG(3, fCONFIG)
                    DPrint("INFO:     compute host '%s' added\n",
                           SArray[index]);
                } /* END for (index) */
            }     /* END BLOCK */

            break;

        case pDefaultDomain:

            /* NOTE:  always proceed with '.' */

            if (SVal[0] == '.') {
                MUStrCpy(S->DefaultDomain, SVal, sizeof(S->DefaultDomain));
            } else if (SVal[0] != '\0') {
                sprintf(S->DefaultDomain, ".%s", SVal);
            }

            break;

        case pDefaultClassList:

        {
            int index;

            S->DefaultClassList[0] = '\0';

            if (SArray != NULL) {
                for (index = 0; SArray[index] != NULL; index++) {
                    sprintf(S->DefaultClassList, "%s[%s]", S->DefaultClassList,
                            SArray[index]);

                    DBG(3, fCONFIG)
                    DPrint("INFO:     default class '%s' added\n",
                           SArray[index]);
                } /* END for (index) */
            }     /* END if (SArray != NULL) */
        }         /* END BLOCK */

        break;

        case mcoDeferTime:

            S->DeferTime = MUTimeFromString(SVal);

            break;

        case pDeferCount:

            S->DeferCount = IVal;

            break;

        case pDeferStartCount:

            S->DeferStartCount = IVal;

            break;

        case pJobPurgeTime:

            S->JobPurgeTime = MUTimeFromString(SVal);

            break;

        case pNodePollFrequency:

            S->NodePollFrequency = MAX(0, IVal);

            break;

        case pNodePurgeTime:

            S->NodePurgeTime = MUTimeFromString(SVal);

            break;

        case pAPIFailureThreshhold:

            S->APIFailureThreshhold = IVal;

            break;

        case pNodeSyncDeadline:

            S->NodeSyncDeadline = MUTimeFromString(SVal);

            break;

        case pJobSyncDeadline:

            S->JobSyncDeadline = MUTimeFromString(SVal);

            break;

        case pParIgnQList:

            /* multi-string */

            {
                /* FORMAT:  <QNAME>[{ ,:}<QNAME>] */

                char *ptr;
                char *TokPtr;

                int pindex;
                int qindex;

                if (S->IgnQList == NULL)
                    S->IgnQList =
                        (char **)calloc(1, sizeof(char *) * MAX_MCLASS);

                qindex = 0;

                for (pindex = 0; SArray[pindex] != NULL; pindex++) {
                    ptr = MUStrTok(SArray[pindex], ",: \t", &TokPtr);

                    while (ptr != NULL) {
                        MUStrDup(&S->IgnQList[qindex], ptr);

                        ptr = MUStrTok(NULL, ",: \t", &TokPtr);

                        qindex++;
                    }
                } /* END for (pindex) */

                MUFree(&S->IgnQList[qindex]);
            }

            break;

        case pJobAggregationTime:

            S->RMJobAggregationTime = MUTimeFromString(SVal);

            break;

        case pJobMaxOverrun:

            if (!strcmp(SVal, "-1"))
                S->JobMaxOverrun = -1;
            else
                S->JobMaxOverrun = MUTimeFromString(SVal);

            break;

        case pLogFileMaxSize:

            S->LogFileMaxSize = IVal;

            MLogInitialize("N/A", S->LogFileMaxSize, S->Iteration);

            break;

        case pLogFileRollDepth:

            S->LogFileRollDepth = IVal;

            break;

        case pLogLevel:

            DBG(3, fCONFIG)
            DPrint("INFO:     changing old %s value (%d)\n", MParam[PIndex],
                   IVal);

            mlog.Threshold = IVal;

            DBG(3, fCONFIG)
            DPrint("INFO:     new %s value (%d)\n", MParam[PIndex], IVal);

            break;

        case pLogFacility:

        {
            int index;
            int vindex;

            mlog.FacilityList = 0;

            for (index = 0; SArray[index] != NULL; index++) {
                for (vindex = 0; MLogFacilityType[vindex] != NULL; vindex++) {
                    if (strstr(SArray[index], MLogFacilityType[vindex]) !=
                        NULL) {
                        mlog.FacilityList |= (1 << vindex);

                        if (vindex == dfALL) {
                            mlog.FacilityList |= fALL;

                            break;
                        }
                    }
                } /* END for (vindex) */
            }     /* END for (index) */

            DBG(2, fALL)
            DPrint("INFO:     %s set to %x\n", MParam[PIndex],
                   (int)mlog.FacilityList);
        } /* END BLOCK */

        break;

        case pMaxJobPerIteration:

            S->MaxJobPerIteration = IVal;

            break;

        case pMinDispatchTime:

            S->MinDispatchTime = MUTimeFromString(SVal);

            break;

        case pMaxSleepIteration:

            S->MaxSleepIteration = IVal;

            break;

        case pNodeMaxLoad:

            S->DefaultN.MaxLoad = DVal;

            break;

        case pNodeCPUOverCommitFactor:

            S->NodeCPUOverCommitFactor = DVal;

            break;

        case pNodeMemOverCommitFactor:

            S->NodeMemOverCommitFactor = DVal;

            break;

        case pNodeUntrackedProcFactor:

            MPar[0].UntrackedProcFactor = DVal;

            break;

        case pPlotMinTime:

            MStat.P.MinTime = MUTimeFromString(SVal);

            break;

        case pPlotMaxTime:

            MStat.P.MaxTime = MUTimeFromString(SVal);

            break;

        case pPlotTimeScale:

            MStat.P.TimeStepCount = MIN(IVal, MAX_MGRIDTIMES);

            break;

        case pPlotMinNode:

            MStat.P.MinNode = IVal;

            break;

        case pPlotMaxNode:

            MStat.P.MaxNode = IVal;

            break;

        case pPlotNodeScale:

            MStat.P.NodeStepCount = MIN(IVal, MAX_MGRIDSIZES);

            break;

        case pReservationDepth:

        {
            int index;

            index = MIN(MAX_MQOS - 1, (int)strtol(IndexName, NULL, 0));

            P->ResDepth[index] = IVal;
        } /* END BLOCK */

        break;

        case pResQOSList:

        {
            int aindex;
            int vindex;

            int index;

            mqos_t *Q;

            aindex = MIN(MAX_MQOS - 1, (int)strtol(IndexName, NULL, 0));

            vindex = 0;

            for (index = 0; SArray[index] != NULL; index++) {
                if (!strcmp(SArray[index], ALL)) {
                    P->ResQOSList[aindex][vindex] = (mqos_t *)MAX_MQOS;

                    vindex++;

                    break;
                }

                if ((MQOSFind(SArray[index], &Q) == SUCCESS) ||
                    (MQOSAdd(SArray[index], &Q) == SUCCESS)) {
                    if (vindex >= (MAX_MQOS - 1)) {
                        DBG(0, fCONFIG)
                        DPrint("ALERT:    max QOS (%d) reached\n", MAX_MQOS);

                        break;
                    }

                    P->ResQOSList[aindex][vindex] = Q;

                    vindex++;

                    DBG(3, fCONFIG)
                    DPrint("INFO:     QOS '%s' added to %s[%d]\n", Q->Name,
                           MParam[pResQOSList], aindex);
                } else {
                    DBG(3, fCONFIG)
                    DPrint(
                        "WARNING:  cannot locate QOS '%s' for parameter %s\n",
                        SArray[index], MParam[pResQOSList]);
                }
            } /* END for (index) */

            P->ResQOSList[aindex][vindex] = NULL;
        } /* END BLOCK */

        break;

        case pRMPollInterval:

            S->RMPollInterval = MUTimeFromString(SVal);

            break;

        case pSchedToolsDir:

            /* prepend HomeDir if necessary */

            if ((SVal[0] != '~') && (SVal[0] != '/')) {
                sprintf(S->ToolsDir, "%s%s", S->HomeDir, SVal);
            } else {
                MUStrCpy(S->ToolsDir, SVal, sizeof(S->ToolsDir));
            }

            if (SVal[strlen(SVal) - 1] != '/') {
                MUStrCat(S->ToolsDir, "/", sizeof(S->ToolsDir));
            }

            break;

        case pSchedLockFile:

            MUStrCpy(S->LockFile, SVal, sizeof(S->LockFile));

            break;

        case pStatDir:

            if ((SVal[0] != '~') && (SVal[0] != '/')) {
                sprintf(MStat.StatDir, "%s%s", S->HomeDir, SVal);
            } else {
                MUStrCpy(MStat.StatDir, SVal, sizeof(MStat.StatDir));
            }

            if (SVal[strlen(SVal) - 1] != '/')
                MUStrCat(MStat.StatDir, "/", sizeof(MStat.StatDir));

            if (S->Reload == TRUE) {
                MStatOpenFile(S->Time);
            }

            break;

        case mcoTimePolicy:

            S->TimePolicy = MUGetIndex(SVal, MTimePolicy, FALSE, mtpNONE);

            break;

        case pAdminEInterval:

            S->ActionInterval = MUTimeFromString(SVal);

            break;

        case pAdminEAction:

        {
            char tmpPathName[MAX_MLINE];
            int IsExe;

            MUStrCpy(S->Action[mactAdminEvent], SVal,
                     sizeof(S->Action[mactAdminEvent]));

            if (S->Action[mactAdminEvent][0] == '/') {
                MUStrCpy(tmpPathName, S->Action[mactAdminEvent],
                         sizeof(tmpPathName));
            } else if (S->ToolsDir[strlen(S->ToolsDir) - 1] == '/') {
                sprintf(tmpPathName, "%s%s", S->ToolsDir,
                        S->Action[mactAdminEvent]);
            } else {
                sprintf(tmpPathName, "%s/%s", S->ToolsDir,
                        S->Action[mactAdminEvent]);
            }

            if ((MFUGetInfo(tmpPathName, NULL, NULL, &IsExe) == FAILURE) ||
                (IsExe == FALSE)) {
                DBG(2, fCONFIG)
                DPrint("ALERT:    invalid action program '%s' requested\n",
                       tmpPathName);

                DBG(2, fCONFIG)
                DPrint("INFO:     disabling action program '%s'\n",
                       S->Action[mactAdminEvent]);

                S->Action[mactAdminEvent][0] = '\0';
            }
        } /* END BLOCK */

        break;

        case pClientTimeout:

            S->ClientTimeout = MIN(MUTimeFromString(SVal), 1000);

            break;

        case pDisplayFlags:

        {
            int index;
            int vindex;

            S->DisplayFlags = 0;

            for (index = 0; SArray[index] != NULL; index++) {
                for (vindex = 0; MCDisplayType[vindex] != NULL; vindex++) {
                    if (strstr(SArray[index], MCDisplayType[vindex]) != NULL) {
                        S->DisplayFlags |= (1 << vindex);
                    }
                }
            } /* END for (index) */

            DBG(2, fALL)
            DPrint("INFO:     %s set to %x\n", MParam[pDisplayFlags],
                   (int)S->DisplayFlags);
        } /* END BLOCK */

        break;

        case mcoDirectoryServer:

        {
            int tmpI;

            char *ptr;
            char *TokPtr;

            /* FORMAT:  <HOST>[:<PORT>] */

            TokPtr = NULL;

            ptr = MUStrTok(SVal, ": \t\n", &TokPtr);

            if (ptr != NULL) MUStrDup(&MSched.DS.HostName, ptr);

            ptr = MUStrTok(NULL, ": \t\n", &TokPtr);

            if (ptr != NULL) {
                tmpI = (int)strtol(ptr, NULL, 10);

                if (tmpI > 0) MSched.DS.Port = tmpI;
            }
        } /* END BLOCK */

        break;

        case mcoEventServer:

        {
            int tmpI;

            char *ptr;
            char *TokPtr;

            /* FORMAT:  <HOST>[:<PORT>] */

            TokPtr = NULL;

            ptr = MUStrTok(SVal, ": \t\n", &TokPtr);

            if (ptr != NULL) MUStrDup(&MSched.EM.HostName, ptr);

            ptr = MUStrTok(NULL, ": \t\n", &TokPtr);

            if (ptr != NULL) {
                tmpI = (int)strtol(ptr, NULL, 10);

                if (tmpI > 0) MSched.EM.Port = tmpI;
            }
        } /* END BLOCK */

        break;

        case mcoJobFBAction:

        {
            char tmpPathName[MAX_MLINE];
            int IsExe;

            MUStrCpy(S->Action[mactJobFB], SVal, sizeof(S->Action[mactJobFB]));

            if (S->Action[mactJobFB][0] == '/') {
                MUStrCpy(tmpPathName, S->Action[mactJobFB],
                         sizeof(tmpPathName));
            } else if (S->ToolsDir[strlen(S->ToolsDir) - 1] == '/') {
                sprintf(tmpPathName, "%s%s", S->ToolsDir, S->Action[mactJobFB]);
            } else {
                sprintf(tmpPathName, "%s/%s", S->ToolsDir,
                        S->Action[mactJobFB]);
            }

            if ((MFUGetInfo(tmpPathName, NULL, NULL, &IsExe) == FAILURE) ||
                (IsExe == FALSE)) {
                DBG(2, fCONFIG)
                DPrint("ALERT:    invalid job FB program '%s' requested\n",
                       tmpPathName);

                DBG(2, fCONFIG)
                DPrint("INFO:     disabling job FB program '%s'\n",
                       S->Action[mactJobFB]);

                S->Action[mactJobFB][0] = '\0';
            }
        } /* END BLOCK */

        break;

        case mcoMailAction:

            /* NYI */

            break;

        case pMCSocketProtocol:

            S->DefaultMCSocketProtocol = MUGetIndex(SVal, MSockProtocol, 0, 0);

            break;

        case pResCtlPolicy:

            S->ResCtlPolicy =
                MUGetIndex(SVal, (const char **)MResCtlPolicy, FALSE, 0);

            break;

        case mcoResLimitPolicy:

            S->ResLimitPolicy =
                MUGetIndex(SVal, (const char **)MPolicyMode, FALSE, 0);

            break;

        case mcoAdminUsers:
        case mcoAdmin1Users:

        {
            int aindex;

            char *ptr;
            char *TokPtr;

            for (aindex = 0; SArray[aindex] != NULL; aindex++) {
                if (aindex == MAX_MADMINUSERS) {
                    DBG(0, fCONFIG)
                    DPrint("ALERT:    max admin users (%d) reached\n",
                           MAX_MADMINUSERS);

                    break;
                }

                ptr = MUStrTok(SArray[aindex], ", \t", &TokPtr);

                while (ptr != NULL) {
                    MUStrCpy(S->Admin1User[aindex], ptr,
                             sizeof(S->Admin1User[aindex]));

                    ptr = MUStrTok(NULL, ", \t", &TokPtr);
                } /* END while (ptr != NULL) */

                DBG(3, fCONFIG)
                DPrint("INFO:     ADMIN1 '%s' added\n", S->Admin1User[aindex]);
            } /* END for (aindex) */

            S->Admin1User[aindex][0] = '\0';
        } /* END BLOCK */

        break;

        case mcoAdmin2Users:

        {
            int aindex;

            char *ptr;
            char *TokPtr;

            for (aindex = 0; SArray[aindex] != NULL; aindex++) {
                if (aindex == MAX_MADMINUSERS) {
                    DBG(0, fCONFIG)
                    DPrint("ALERT:    max admin users (%d) reached\n",
                           MAX_MADMINUSERS);

                    break;
                }

                ptr = MUStrTok(SArray[aindex], ", \t", &TokPtr);

                while (ptr != NULL) {
                    MUStrCpy(S->Admin2User[aindex], ptr,
                             sizeof(S->Admin2User[aindex]));

                    ptr = MUStrTok(NULL, ", \t", &TokPtr);
                } /* END while (ptr != NULL) */

                DBG(3, fCONFIG)
                DPrint("INFO:     ADMIN2 '%s' added\n", S->Admin2User[aindex]);
            } /* END for (aindex) */

            S->Admin2User[aindex][0] = '\0';
        } /* END BLOCK */

        break;

        case mcoAdmin3Users:

        {
            int aindex;

            char *ptr;
            char *TokPtr;

            for (aindex = 0; SArray[aindex] != NULL; aindex++) {
                if (aindex == MAX_MADMINUSERS) {
                    DBG(0, fCONFIG)
                    DPrint("ALERT:    max admin users (%d) reached\n",
                           MAX_MADMINUSERS);

                    break;
                }

                ptr = MUStrTok(SArray[aindex], ", \t", &TokPtr);

                while (ptr != NULL) {
                    MUStrCpy(S->Admin3User[aindex], ptr,
                             sizeof(S->Admin3User[aindex]));

                    ptr = MUStrTok(NULL, ", \t", &TokPtr);
                } /* END while (ptr != NULL) */

                DBG(3, fCONFIG)
                DPrint("INFO:     ADMIN3 '%s' added\n", S->Admin3User[aindex]);
            } /* END for (aindex) */

            S->Admin3User[aindex][0] = '\0';
        } /* END BLOCK */

        break;

        case mcoAdmin4Users:

        {
            int aindex;

            char *ptr;
            char *TokPtr;

            for (aindex = 0; SArray[aindex] != NULL; aindex++) {
                if (aindex == MAX_MADMINUSERS) {
                    DBG(0, fCONFIG)
                    DPrint("ALERT:    max admin users (%d) reached\n",
                           MAX_MADMINUSERS);

                    break;
                }

                ptr = MUStrTok(SArray[aindex], ", \t", &TokPtr);

                while (ptr != NULL) {
                    MUStrCpy(S->Admin4User[aindex], ptr,
                             sizeof(S->Admin4User[aindex]));

                    ptr = MUStrTok(NULL, ", \t", &TokPtr);
                } /* END while (ptr != NULL) */

                DBG(3, fCONFIG)
                DPrint("INFO:     ADMIN4 '%s' added\n", S->Admin4User[aindex]);
            } /* END for (aindex) */

            S->Admin4User[aindex][0] = '\0';
        } /* END BLOCK */

        break;

        case mcoAdminHosts:

        {
            int aindex;

            for (aindex = 0; SArray[aindex] != NULL; aindex++) {
                if (aindex == MAX_MADMINHOSTS) {
                    DBG(0, fCONFIG)
                    DPrint("ALERT:    max admin hosts (%d) reached\n",
                           MAX_MADMINHOSTS);

                    break;
                }

                MUStrCpy(S->AdminHost[aindex], SArray[aindex],
                         sizeof(S->AdminHost[aindex]));

                DBG(3, fCONFIG)
                DPrint("INFO:     admin host '%s' added\n",
                       S->AdminHost[aindex]);
            } /* END for (aindex) */

            S->AdminHost[aindex][0] = '\0';
        } /* END BLOCK */

        break;

        case pMServerHomeDir:

            MSchedSetAttr(S, PIndex, (void **)SVal, mdfString, mSet);

            break;

        case pServerHost:

        {
            char HostName[MAX_MLINE];

            MUStrCpy(S->ServerHost, SVal, sizeof(S->ServerHost));

            if (MOSGetHostName(NULL, HostName, NULL) == FAILURE) {
                DBG(0, fCONFIG)
                DPrint("ERROR:    cannot determine local hostname\n");

                fprintf(stderr, "ERROR:    cannot determine local hostname\n");

                exit(1);
            }

            if ((strncmp(HostName, S->ServerHost, strlen(S->ServerHost))) &&
                (strtol(S->ServerHost, NULL, 0) == 0)) {
                DBG(0, fCONFIG)
                DPrint(
                    "ERROR:    server must be started on host '%s' (currently "
                    "on '%s')\n",
                    S->ServerHost, HostName);

                fprintf(stderr,
                        "ERROR:    server must be started on host '%s' "
                        "(currently on '%s')\n",
                        S->ServerHost, HostName);

                exit(1);
            } else {
                DBG(2, fCONFIG)
                DPrint("INFO:     starting scheduler on '%s'\n", S->ServerHost);
            }
        } /* END BLOCK */

        break;

        case pServerName:

            MUStrCpy(S->Name, SVal, sizeof(S->Name));

            break;

        case pServerPort:

            S->ServerPort = IVal;

            break;

        case pMonitoredJob:

            MUStrCpy(S->MonitoredJob, SVal, sizeof(S->MonitoredJob));

            break;

        case pMonitoredNode:

            MUStrCpy(S->MonitoredNode, SVal, sizeof(S->MonitoredNode));

            break;

        case pMonitoredRes:

            MUStrCpy(S->MonitoredRes, SVal, sizeof(S->MonitoredRes));

            break;

        case pMonitoredFunction:

            MUStrCpy(S->MonitoredFunction, SVal, sizeof(S->MonitoredFunction));

            break;

        case pPreemptPolicy:

            S->PreemptPolicy =
                MUGetIndex(SVal, (const char **)MPreemptPolicy, 0, 0);

            break;

        case pProcSpeedFeatureHeader:

            MUStrCpy(S->ProcSpeedFeatureHeader, SVal,
                     sizeof(S->ProcSpeedFeatureHeader));

            break;

        case pResDepth:

            S->ResDepth = MIN(MAX_MRES_DEPTH, IVal);

            break;

        case pSchedLogDir:

            /* prepend HomeDir if necessary */

            if ((SVal[0] != '~') && (SVal[0] != '/')) {
                sprintf(S->LogDir, "%s%s", S->HomeDir, SVal);
            } else {
                MUStrCpy(S->LogDir, SVal, sizeof(S->LogDir));
            }

            if (SVal[strlen(SVal) - 1] != '/') {
                MUStrCat(S->LogDir, "/", sizeof(S->LogDir));
            }

            break;

        case pSchedLogFile:

        {
            char tmpLine[MAX_MLINE];

            MUStrCpy(S->LogFile, SVal, sizeof(S->LogFile));

            if ((S->LogFile[0] == '/') || (S->LogDir[0] == '\0')) {
                MUStrCpy(tmpLine, S->LogFile, sizeof(tmpLine));
            } else {
                MUStrCpy(tmpLine, S->LogDir, sizeof(tmpLine));

                if (S->LogDir[strlen(S->LogDir) - 1] != '/') {
                    MUStrCat(tmpLine, "/", sizeof(tmpLine));
                }

                MUStrCat(tmpLine, S->LogFile, sizeof(tmpLine));
            }

            MLogInitialize(tmpLine, -1, S->Iteration);

            DBG(0, fALL)
            DPrint("INFO:     starting %s version %s ##################\n",
                   MSCHED_NAME, MSCHED_VERSION);

            /* add moab version info */

            /* NYI */
        } /* END BLOCK */

        break;

        case pSchedStepCount:

            S->StepCount = IVal;

            break;

        case pNodeTypeFeatureHeader:

            MUStrCpy(S->NodeTypeFeatureHeader, SVal,
                     sizeof(S->NodeTypeFeatureHeader));

            break;

        case pPartitionFeatureHeader:

            MUStrCpy(S->PartitionFeatureHeader, SVal,
                     sizeof(S->PartitionFeatureHeader));

            break;

        case pNAPolicy:

            S->DefaultNAccessPolicy = (enum MNodeAccessPolicyEnum)MUGetIndex(
                SVal, MNAccessPolicy, FALSE, DEFAULT_MNACCESSPOLICY);

            break;

        case pNAMaxPS:

            S->NodeAllocMaxPS = MUBoolFromString(SVal, FALSE);

            break;

        case pSchedMode:

            S->Mode = MUGetIndex(SVal, MSchedMode, FALSE, S->Mode);
            S->SpecMode = S->Mode;

            break;

        case mcoUseSyslog:

        {
            char *ptr;
            char *TokPtr;

            /* FORMAT:  <BOOL>:<FACILITY> */

            if ((ptr = MUStrTok(SVal, ":", &TokPtr)) == NULL) break;

            S->UseSyslog = MUBoolFromString(ptr, FALSE);

            if ((ptr = MUStrTok(SVal, ":", &TokPtr)) == NULL) break;

            S->SyslogFacility = (int)strtol(ptr, NULL, 0);

            MOSSyslogInit(S);
        } /* END BLOCK */

        break;

        case pMaxJobPerUserPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultU != NULL) {
                    S->DefaultU->L.AP.SLimit[mptMaxJob][P->Index] = 0;
                    S->DefaultU->L.AP.HLimit[mptMaxJob][P->Index] = 0;
                }
            }

            break;

        case pMaxJobPerUserCount:

            if (S->DefaultU != NULL)
                S->DefaultU->L.AP.SLimit[mptMaxJob][P->Index] = IVal;

            break;

        case pHMaxJobPerUserCount:

            if (S->DefaultU != NULL)
                S->DefaultU->L.AP.HLimit[mptMaxJob][P->Index] = IVal;

            break;

        case pMaxNodePerUserPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultU != NULL) {
                    S->DefaultU->L.AP.SLimit[mptMaxNode][P->Index] = 0;
                    S->DefaultU->L.AP.HLimit[mptMaxNode][P->Index] = 0;
                }
            }

            break;

        case pMaxNodePerUserCount:

            if (S->DefaultU != NULL)
                S->DefaultU->L.AP.SLimit[mptMaxNode][P->Index] = IVal;

            break;

        case pHMaxNodePerUserCount:

            if (S->DefaultU != NULL)
                S->DefaultU->L.AP.HLimit[mptMaxNode][P->Index] = IVal;

            break;

        case pMaxProcPerUserPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultU != NULL) {
                    S->DefaultU->L.AP.SLimit[mptMaxProc][P->Index] = 0;
                    S->DefaultU->L.AP.HLimit[mptMaxProc][P->Index] = 0;
                }
            }

            break;

        case pMaxProcPerUserCount:

            if (S->DefaultU != NULL)
                S->DefaultU->L.AP.SLimit[mptMaxProc][P->Index] = IVal;

            break;

        case pHMaxProcPerUserCount:

            if (S->DefaultU != NULL)
                S->DefaultU->L.AP.HLimit[mptMaxProc][P->Index] = IVal;

            break;

        case pMaxPSPerUserPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultU != NULL) {
                    S->DefaultU->L.AP.SLimit[mptMaxPS][P->Index] = 0;
                    S->DefaultU->L.AP.HLimit[mptMaxPS][P->Index] = 0;
                }
            }

            break;

        case pMaxPSPerUserCount:

            if (S->DefaultU != NULL)
                S->DefaultU->L.AP.SLimit[mptMaxPS][P->Index] = IVal;

            break;

        case pHMaxPSPerUserCount:

            if (S->DefaultU != NULL)
                S->DefaultU->L.AP.HLimit[mptMaxPS][P->Index] = IVal;

            break;

        case pMaxJobQueuedPerUserPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultU != NULL) {
                    S->DefaultU->L.IP->SLimit[mptMaxJob][P->Index] = 0;
                    S->DefaultU->L.IP->HLimit[mptMaxJob][P->Index] = 0;
                }
            }

            break;

        case pMaxJobQueuedPerUserCount:

            if (S->DefaultU != NULL)
                S->DefaultU->L.IP->SLimit[mptMaxJob][P->Index] = IVal;

            break;

        case pHMaxJobQueuedPerUserCount:

            if (S->DefaultU != NULL)
                S->DefaultU->L.IP->HLimit[mptMaxJob][P->Index] = IVal;

            break;

        /* group policies */

        case pMaxJobPerGroupPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultG != NULL) {
                    S->DefaultG->L.AP.SLimit[mptMaxJob][P->Index] = 0;
                    S->DefaultG->L.AP.HLimit[mptMaxJob][P->Index] = 0;
                }
            }

            break;

        case pMaxJobPerGroupCount:

            if (S->DefaultG != NULL)
                S->DefaultG->L.AP.SLimit[mptMaxJob][P->Index] = IVal;

            break;

        case pHMaxJobPerGroupCount:

            if (S->DefaultG != NULL)
                S->DefaultG->L.AP.HLimit[mptMaxJob][P->Index] = IVal;

            break;

        case pMaxNodePerGroupPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultG != NULL) {
                    S->DefaultG->L.AP.SLimit[mptMaxNode][P->Index] = 0;
                    S->DefaultG->L.AP.HLimit[mptMaxNode][P->Index] = 0;
                }
            }

            break;

        case pMaxNodePerGroupCount:

            if (S->DefaultG != NULL)
                S->DefaultG->L.AP.SLimit[mptMaxNode][P->Index] = IVal;

            break;

        case pHMaxNodePerGroupCount:

            if (S->DefaultG != NULL)
                S->DefaultG->L.AP.HLimit[mptMaxNode][P->Index] = IVal;

            break;

        case pMaxPSPerGroupPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultG != NULL) {
                    S->DefaultG->L.AP.SLimit[mptMaxPS][P->Index] = 0;
                    S->DefaultG->L.AP.HLimit[mptMaxPS][P->Index] = 0;
                }
            }

            break;

        case pMaxPSPerGroupCount:

            if (S->DefaultG != NULL)
                S->DefaultG->L.AP.SLimit[mptMaxPS][P->Index] = IVal;

            break;

        case pHMaxPSPerGroupCount:

            if (S->DefaultG != NULL)
                S->DefaultG->L.AP.HLimit[mptMaxPS][P->Index] = IVal;

            break;

        case pMaxJobQueuedPerGroupPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultG != NULL) {
                    S->DefaultG->L.IP->SLimit[mptMaxJob][P->Index] = 0;
                    S->DefaultG->L.IP->HLimit[mptMaxJob][P->Index] = 0;
                }
            }

            break;

        case pMaxJobQueuedPerGroupCount:

            if (S->DefaultG != NULL)
                S->DefaultG->L.IP->SLimit[mptMaxJob][P->Index] = IVal;

            break;

        case pHMaxJobQueuedPerGroupCount:

            if (S->DefaultG != NULL)
                S->DefaultG->L.IP->HLimit[mptMaxJob][P->Index] = IVal;

            break;

        /* account Policies */

        case pMaxJobPerAccountPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultA != NULL) {
                    S->DefaultA->L.AP.SLimit[mptMaxJob][P->Index] = 0;
                    S->DefaultA->L.AP.HLimit[mptMaxJob][P->Index] = 0;
                }
            }

            break;

        case pMaxJobPerAccountCount:

            if (S->DefaultA != NULL)
                S->DefaultA->L.AP.SLimit[mptMaxJob][P->Index] = IVal;

            break;

        case pHMaxJobPerAccountCount:

            if (S->DefaultA != NULL)
                S->DefaultA->L.AP.HLimit[mptMaxJob][P->Index] = IVal;

            break;

        case pMaxNodePerAccountPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultA != NULL) {
                    S->DefaultA->L.AP.SLimit[mptMaxNode][P->Index] = 0;
                    S->DefaultA->L.AP.HLimit[mptMaxNode][P->Index] = 0;
                }
            }

            break;

        case pMaxNodePerAccountCount:

            if (S->DefaultA != NULL)
                S->DefaultA->L.AP.SLimit[mptMaxNode][P->Index] = IVal;

            break;

        case pHMaxNodePerAccountCount:

            if (S->DefaultA != NULL)
                S->DefaultA->L.AP.HLimit[mptMaxNode][P->Index] = IVal;

            break;

        case pMaxPSPerAccountPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultA != NULL) {
                    S->DefaultA->L.AP.SLimit[mptMaxPS][P->Index] = 0;
                    S->DefaultA->L.AP.HLimit[mptMaxPS][P->Index] = 0;
                }
            }

            break;

        case pMaxPSPerAccountCount:

            if (S->DefaultA != NULL)
                S->DefaultA->L.AP.SLimit[mptMaxPS][P->Index] = IVal;

            break;

        case pHMaxPSPerAccountCount:

            if (S->DefaultA != NULL)
                S->DefaultA->L.AP.HLimit[mptMaxPS][P->Index] = IVal;

            break;

        case pMaxJobQueuedPerAccountPolicy:

            if (MUBoolFromString(SVal, FALSE) == FALSE) {
                if (S->DefaultA != NULL) {
                    S->DefaultA->L.IP->SLimit[mptMaxJob][P->Index] = 0;
                    S->DefaultA->L.IP->HLimit[mptMaxJob][P->Index] = 0;
                }
            }

            break;

        case pMaxJobQueuedPerAccountCount:

            if (S->DefaultA != NULL)
                S->DefaultA->L.IP->SLimit[mptMaxJob][P->Index] = IVal;

            break;

        case pHMaxJobQueuedPerAccountCount:

            if (S->DefaultA != NULL)
                S->DefaultA->L.IP->HLimit[mptMaxJob][P->Index] = IVal;

            break;

        case mcoUseJobRegEx:

            S->UseJobRegEx = MUBoolFromString(SVal, FALSE);

            break;

        case mcoWCViolAction:

            MSched.WCViolAction =
                MUGetIndex(SVal, MWCVAction, FALSE, MSched.WCViolAction);

            break;

        default:

            /* not handled */

            return (FAILURE);

            /*NOTREACHED*/

            break;
    } /* END switch (PIndex) */

    return (SUCCESS);
} /* END MSchedProcessOConfig() */

int MJobNLDistribute(

    mjob_t *J,          /* I */
    mnodelist_t SrcMNL, /* I */
    mnodelist_t DstMNL) /* O */

{
    int nindex;

    int rqindex;

    int index;

    short NFreq[MAX_MNODE];
    char NFreqSet[MAX_MREQ_PER_JOB][MAX_MNODE];

    double TAvail[MAX_MREQ_PER_JOB];
    double TReq[MAX_MREQ_PER_JOB];

    int NIndex[MAX_MREQ_PER_JOB];

    mnalloc_t *NodeList;

    double mval;
    double val;

    int mindex;
    int TC;

    const char *FName = "MJobNLDistribute";

    DBG(4, fSCHED)
    DPrint("%s(%s,SrcMNL,DstMNL)\n", FName, (J != NULL) ? J->Name : "NULL");

    if (J == NULL) {
        return (FAILURE);
    }

    if (J->Req[1] == NULL) {
        /* only one req exists, modification not required */

        memcpy(DstMNL, SrcMNL, sizeof(mnodelist_t));

        return (SUCCESS);
    }

    memset(NFreq, 0, sizeof(NFreq));
    memset(NFreqSet, FALSE, sizeof(NFreqSet));

    memset(TAvail, 0, sizeof(TAvail));
    memset(TReq, 0, sizeof(TReq));

    /* NOTE:  must balance by node request as well */

    memset(NIndex, 0, sizeof(NIndex));

    /* determine node frequency sets */

    for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
        for (nindex = 0; SrcMNL[rqindex][nindex].N != NULL; nindex++) {
            NFreq[SrcMNL[rqindex][nindex].N->Index]++;

            NFreqSet[rqindex][SrcMNL[rqindex][nindex].N->Index] = TRUE;

            DBG(8, fSCHED)
            DPrint("INFO:     job %s:%d nodelist[%d]:  %sx%d\n", J->Name,
                   rqindex, nindex, SrcMNL[rqindex][nindex].N->Name,
                   SrcMNL[rqindex][nindex].TC);
        } /* END for (nindex) */

        DBG(6, fSCHED)
        DPrint("INFO:     job %s:%d nodes:  %d\n", J->Name, rqindex, nindex);
    } /* END for (rqindex) */

    /* calculate available tasks */

    for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
        TReq[rqindex] = (double)J->Req[rqindex]->TaskCount;

        for (nindex = 0; SrcMNL[rqindex][nindex].N != NULL; nindex++) {
            TAvail[rqindex] += (double)SrcMNL[rqindex][nindex].TC /
                               NFreq[SrcMNL[rqindex][nindex].N->Index];
        } /* END for (nindex) */

        DBG(6, fSCHED)
        DPrint("INFO:     job %s:%d avail:  %d\n", J->Name, rqindex, nindex);
    } /* END for (rqindex) */

    /* assign nodes */

    for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
        NodeList = (mnalloc_t *)SrcMNL[rqindex];

        for (nindex = 0; NodeList[nindex].N != NULL; nindex++) {
            if (NFreq[NodeList[nindex].N->Index] == 0) {
                /* node no longer requested */

                continue;
            }

            mval = -999999.0;
            mindex = 0;

            /* determine 'hungriest' req */

            for (index = 0; J->Req[index] != NULL; index++) {
                if (NFreqSet[index][NodeList[nindex].N->Index] == FALSE) {
                    /* node not valid for req */

                    continue;
                }

                val = TReq[index] / MAX(1, TAvail[index]);

                if (val > mval) {
                    mval = val;
                    mindex = index;
                }
            } /* END for (index) */

            DstMNL[mindex][NIndex[mindex]].N = NodeList[nindex].N;
            DstMNL[mindex][NIndex[mindex]].TC = NodeList[nindex].TC;

            NIndex[mindex]++;

            TC = MIN(J->Req[rqindex]->TaskCount, NodeList[nindex].TC);

            TReq[mindex] -= (double)TC;

            for (index = 0; J->Req[index] != NULL; index++) {
                if (NFreqSet[index][NodeList[nindex].N->Index] == TRUE) {
                    TAvail[index] -=
                        (double)TC / NFreq[NodeList[nindex].N->Index];
                }
            } /* END for (index) */

            /* decrement node frequency */

            NFreq[NodeList[nindex].N->Index]--;
        } /* END for (nindex)  */
    }     /* END for (rqindex) */

    /* terminate lists */

    for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
        DstMNL[rqindex][NIndex[rqindex]].N = NULL;

        for (nindex = 0; DstMNL[rqindex][nindex].N != NULL; nindex++) {
            DBG(7, fSCHED)
            DPrint("INFO:     job %s:%d nodelist[%d]: %sx%d\n", J->Name,
                   rqindex, nindex, DstMNL[rqindex][nindex].N->Name,
                   DstMNL[rqindex][nindex].TC);
        } /* END for (nindex) */

        if (TReq[rqindex] > 0.0) {
            DBG(2, fSCHED)
            DPrint(
                "INFO:     cannot locate nodes for job '%s' req[%d] (%d "
                "additional needed)\n",
                J->Name, rqindex, (int)TReq[rqindex]);

            return (FAILURE);
        } else {
            DBG(4, fSCHED)
            DPrint("INFO:     dist nodelist for job %s:%d (%d of %d)\n",
                   J->Name, rqindex, NIndex[rqindex],
                   J->Req[rqindex]->TaskCount);
        }
    } /* END for rqindex */

    return (SUCCESS);
} /* END MJobNLDistribute() */

int MJobDistributeTasks(

    mjob_t *J,           /* I */
    mrm_t *R,            /* I */
    mnalloc_t *NodeList, /* O: nodelist with taskcount information */
    short *TaskMap)      /* O: task distribution list              */

{
    int index;
    int sindex;

    int nindex;
    int pindex;
    int tindex;
    int tpnindex;

    int rqindex;

    int taskcount;

    int Distribution;

    mreq_t *RQ;

    int TPN;
    int Overflow;

    int RRTPN;

    mnalloc_t tmpNodeList[MAX_MNODE_PER_JOB + 1];

    int MaxTPN;
    int TasksAvail;
    int TasksRequired;

    mnode_t *MaxTPNN;
    int AllocIndex;

    int AttemptBalancedDistribution;
    int tmpOverflow;
    int tmpTaskCount;
    int tmpNodeCount;

    int rc;

    int DPolicy = 0;

    const char *FName = "MJobDistributeTasks";

    /* note: handle pre-loaded arbitrary geometry */

    DBG(3, fSCHED)
    DPrint("%s(%s,%s,NodeList,TaskMap)\n", FName,
           (J != NULL) ? J->Name : "NULL", (R != NULL) ? R->Name : "NULL");

    if (J == NULL) {
        return (FAILURE);
    }

    MTRAPJOB(J, FName);

    if (J->DistPolicy > 0)
        DPolicy = J->DistPolicy;
    else
        DPolicy = MPar[0].DistPolicy;

    /* NOTE:  RM specific distribution policies handled in Alloc */

    if ((J->Flags & (1 << mjfAllocLocal)) ||
        (MSched.AllocLocalityPolicy != malpNONE)) {
        /* activate local allocation policy */

        /* if J->TaskMap set, no distribution required */
    } /* END if ((J->Flags & (1 << mjfAllocLocal)) || ...) */

    switch (DPolicy) {
        case mtdLocal:

            rc = MLocalJobDistributeTasks(J, R, NodeList, TaskMap);

            return (rc);

            /*NOTREACHED*/

            break;

        case mtdDefault:
        default:

            /* NO-OP */

            break;
    } /* END switch(DPolicy) */

    Distribution = distRR;

    /* build initial tasklist */

    memset(tmpNodeList, 0, sizeof(tmpNodeList));

    index = 0;

    if (J->MReq != NULL) {
        tmpNodeList[index].N = J->MReq->NodeList[0].N;
        tmpNodeList[index].TC = J->MReq->NodeList[0].TC;

        index++;

        DBG(4, fSCHED)
        DPrint("INFO:     %d node(s)/%d proc(s) added for RQ[%d]\n", 1,
               J->MReq->NodeList[0].TC, J->MReq->Index);
    } /* END if (J->MReq != NULL) */

    tmpNodeCount = 0;

    for (rqindex = 0; J->Req[rqindex] != NULL; rqindex++) {
        if ((J->MReq != NULL) && (J->MReq->Index == rqindex)) continue;

        RQ = J->Req[rqindex];

        if (RQ->RMIndex != R->Index) continue;

        if (J->Geometry != NULL) {
            if (MJobDistributeTaskGeometry(J, RQ, NodeList, &index) ==
                SUCCESS) {
                continue;
            }
        }

        if (R->Type == mrmtLL) {
            sindex = index;

            if (RQ->NodeCount > 0) {
                /* NOTE: LL 1.x, 2.1 require single step, monotonically
                 * decreasing algorithm */
                /*       ie, 4,4,3,3 not 4,4,4,2 (psuedo round robin) */

                MaxTPN = 1;

                for (nindex = 0; RQ->NodeList[nindex].N != NULL; nindex++) {
                    if (nindex >= MAX_MNODE_PER_JOB) break;

                    MaxTPN = MAX(MaxTPN, RQ->NodeList[nindex].TC);
                }

                RRTPN = RQ->TaskCount / RQ->NodeCount;

                Overflow = RQ->TaskCount % RQ->NodeCount;

                /* assign nodes with RRTPN + 1 tasks */

                for (nindex = 0; RQ->NodeList[nindex].N != NULL; nindex++) {
                    if (nindex >= MAX_MNODE_PER_JOB) break;

                    if (Overflow == 0) break;

                    if (RQ->NodeList[nindex].TC > RRTPN) {
                        tmpNodeList[index].N = RQ->NodeList[nindex].N;

                        tmpNodeList[index].TC = RRTPN + 1;

                        RQ->NodeList[nindex].TC = 0;

                        DBG(6, fSCHED)
                        DPrint(
                            "INFO:     %d tasks assigned to overflow node[%d] "
                            "%s\n",
                            RRTPN + 1, index, tmpNodeList[index].N->Name);

                        index++;

                        Overflow--;
                    }
                } /* END for (nindex) */

                /* assign remaining nodes */

                for (nindex = 0; RQ->NodeList[nindex].N != NULL; nindex++) {
                    if (nindex >= MAX_MNODE_PER_JOB) break;

                    if (index == RQ->NodeCount) {
                        DBG(6, fSCHED)
                        DPrint("INFO:     nodecount %d reached\n",
                               RQ->NodeCount);

                        break;
                    }

                    if (RQ->NodeList[nindex].TC >= RRTPN) {
                        tmpNodeList[index].N = RQ->NodeList[nindex].N;

                        tmpNodeList[index].TC = RRTPN;

                        RQ->NodeList[nindex].TC = 0;

                        DBG(6, fSCHED)
                        DPrint("INFO:     %d tasks assigned to node[%d] %s\n",
                               RRTPN, index, tmpNodeList[index].N->Name);

                        index++;
                    } else {
                        DBG(6, fSCHED)
                        DPrint("INFO:     node %d ignored (%d < %d)\n", index,
                               RQ->NodeList[nindex].TC, RRTPN);
                    }
                } /* END for (nindex) */

                tmpNodeList[index].N = NULL;

                memcpy(RQ->NodeList, &tmpNodeList[sindex],
                       sizeof(mnalloc_t) * (index - sindex));

                RQ->NodeList[index - sindex].N = NULL;

                memcpy(NodeList, tmpNodeList, sizeof(tmpNodeList));

                tmpNodeCount = index;
            } else {
                /* RQ->NodeCount not specified */

                /* NOTE: use single step, monotonically decreasing algorithm for
                 * now */
                /*       ie, 4,4,3,3 not 4,4,4,2 */

                TasksAvail = 0;
                TasksRequired = J->Request.TC;

                AllocIndex = 0;

                while (TasksAvail < TasksRequired) {
                    /* select MaxTPN node */

                    MaxTPN = -1;
                    MaxTPNN = NULL;
                    tpnindex = 0;

                    for (nindex = sindex; RQ->NodeList[nindex].N != NULL;
                         nindex++) {
                        if (nindex >= MAX_MNODE_PER_JOB) break;

                        if ((int)RQ->NodeList[nindex].TC > MaxTPN) {
                            MaxTPN = RQ->NodeList[nindex].TC;
                            MaxTPNN = RQ->NodeList[nindex].N;
                            tpnindex = nindex;
                        }
                    }

                    if (MaxTPN <= 0) break;

                    tmpNodeList[AllocIndex].N = MaxTPNN;
                    tmpNodeList[AllocIndex].TC = MaxTPN;

                    RQ->NodeList[tpnindex].TC = 0;

                    AllocIndex++;

                    TasksAvail = 0;

                    /* constrain TC delta to X (currently 1) */

                    for (nindex = 0; nindex < AllocIndex; nindex++) {
                        if (nindex >= MAX_MNODE_PER_JOB) break;

                        if (RQ->BlockingFactor != 1) {
                            /* blocking factor specification removes single step
                             * constraint */

                            tmpNodeList[nindex].TC =
                                MIN(tmpNodeList[nindex].TC, MaxTPN + 1);
                        }

                        TasksAvail += tmpNodeList[nindex].TC;
                    }
                } /* END while (TasksAvail < TasksRequired) */

                if (TasksAvail < TasksRequired) {
                    DBG(2, fSCHED)
                    DPrint(
                        "ALERT:    inadequate tasks found on %d nodes in "
                        "DistributeTasks() (%d < %d)\n",
                        AllocIndex, TasksAvail, TasksRequired);

                    return (FAILURE);
                }

                if (TasksAvail > TasksRequired) {
                    /* remove excess tasks */

                    if (RQ->BlockingFactor == 1) {
                        tmpNodeList[AllocIndex - 1].TC -=
                            (TasksAvail - TasksRequired);

                        TasksAvail = TasksRequired;
                    } else if (tmpNodeList[0].TC !=
                               tmpNodeList[AllocIndex - 1].TC) {
                        /* locate n+1 -> n TC transition */

                        for (nindex = AllocIndex - 1; nindex > 0; nindex--) {
                            if (nindex >= MAX_MNODE_PER_JOB) break;

                            if (tmpNodeList[nindex].TC !=
                                tmpNodeList[nindex - 1].TC) {
                                tmpNodeList[nindex - 1].TC--;

                                TasksAvail--;

                                if (TasksAvail == TasksRequired) break;
                            }
                        } /* END for (nindex) */
                    }

                    if (TasksAvail > TasksRequired) {
                        /* reduce last task count */

                        for (nindex = AllocIndex - 1; nindex >= 0; nindex--) {
                            if (nindex >= MAX_MNODE_PER_JOB) break;

                            tmpNodeList[nindex].TC--;

                            TasksAvail--;

                            if (TasksAvail == TasksRequired) break;
                        }
                    }
                } /* END if (TasksAvail > TasksRequired) */

                tmpNodeList[AllocIndex].N = NULL;

                memcpy(RQ->NodeList, tmpNodeList, sizeof(tmpNodeList));

                tmpNodeCount = AllocIndex;
            } /* END else (J->Request.NC > 0) */
        }     /* END if (R->Type == rmLL) */
        else {
            AttemptBalancedDistribution = FALSE;

            if (RQ->NodeCount > 0) {
                TPN = RQ->TaskCount / RQ->NodeCount;
                Overflow = RQ->TaskCount % RQ->NodeCount;

                /* NOTE:  if nodecount is specified, attempt balanced task
                 * distribution */

                tmpTaskCount = RQ->TaskCount;
                tmpNodeCount = RQ->NodeCount;
                tmpOverflow = Overflow;

                for (nindex = 0; RQ->NodeList[nindex].N != NULL; nindex++) {
                    if (nindex >= MAX_MNODE_PER_JOB) break;

                    if (RQ->NodeList[nindex].TC >= TPN) {
                        tmpNodeCount--;
                        tmpTaskCount -= TPN;
                        tmpOverflow -= (RQ->NodeList[nindex].TC - TPN);
                    }
                } /* END for (nindex) */

                if ((tmpOverflow <= 0) && (tmpNodeCount <= 0) &&
                    (tmpTaskCount <= 0)) {
                    AttemptBalancedDistribution = TRUE;
                }
            } else {
                TPN = 1;
                Overflow = 0;
            }

            taskcount = 0;

            /* NOTE:  if nodecount is specified, attempt balanced task
             * distribution */

            /* obtain nodelist from job RQ nodelist */

            for (nindex = 0; RQ->NodeList[nindex].N != NULL; nindex++) {
                if (nindex >= MAX_MNODE_PER_JOB) break;

                tmpNodeList[index].N = RQ->NodeList[nindex].N;

                if (RQ->NodeCount > 0) {
                    if (AttemptBalancedDistribution == TRUE) {
                        MaxTPN = TPN;

                        if ((RQ->NodeList[nindex].TC > TPN) && (Overflow > 0)) {
                            tmpTaskCount =
                                MIN(RQ->NodeList[nindex].TC - TPN, Overflow);

                            MaxTPN += tmpTaskCount;

                            Overflow -= tmpTaskCount;
                        }
                    } else {
                        /* MaxTPN = 1 + RemainingTasks - RemainingNodes */

                        MaxTPN = 1 + (RQ->TaskCount - taskcount) -
                                 (RQ->NodeCount - index);
                    }

                    RQ->NodeList[nindex].TC =
                        MIN(MaxTPN, RQ->NodeList[nindex].TC);
                }

                if (RQ->TasksPerNode > 0) {
                    if (RQ->NodeList[nindex].TC >= RQ->TasksPerNode) {
                        RQ->NodeList[nindex].TC -=
                            RQ->NodeList[nindex].TC % RQ->TasksPerNode;
                    } else {
                        RQ->NodeList[nindex].TC = 0;
                    }
                }

                RQ->NodeList[nindex].TC = (short)MIN(RQ->TaskCount - taskcount,
                                                     RQ->NodeList[nindex].TC);

                if (RQ->NodeList[nindex].TC > 0) {
                    tmpNodeList[index].TC = RQ->NodeList[nindex].TC;

                    taskcount += tmpNodeList[index].TC;

                    DBG(6, fSCHED)
                    DPrint("INFO:     %d tasks assigned to node[%d] %s\n",
                           tmpNodeList[index].TC, index,
                           tmpNodeList[index].N->Name);

                    index++;
                } else {
                    tmpNodeList[index].N = NULL;

                    break;
                }
            } /* END for (nindex) */

            DBG(4, fSCHED)
            DPrint("INFO:     %d node(s)/%d task(s) added to %s:%d\n", index,
                   taskcount, J->Name, rqindex);

            if (index <= 0) {
                DBG(2, fSCHED)
                DPrint("ALERT:    inadequate tasks allocated to job\n");

                return (FAILURE);
            }

            J->NodeCount = index;

            tmpNodeCount = index;
        } /* END else (R->Type == rmLL) */
    }     /* END for (rqindex) */

    tmpNodeList[tmpNodeCount].N = NULL;

    if (NodeList != NULL) {
        memcpy(NodeList, tmpNodeList, sizeof(tmpNodeList));
    }

    /* distribute tasks */

    switch (Distribution) {
        case distRR:

            tindex = 0;

            for (nindex = 0; nindex <= MAX_MNODE_PER_JOB; nindex++) {
                if (tmpNodeList[nindex].N == NULL) {
                    DBG(4, fSCHED)
                    DPrint("INFO:     end of list reached.  %d nodes found\n",
                           nindex);

                    break;
                }

                DBG(4, fSCHED) {
                    DBG(4, fSCHED)
                    DPrint(
                        "INFO:     MNode[%03d] '%s'(x%d) added to job '%s'\n",
                        nindex, tmpNodeList[nindex].N->Name,
                        tmpNodeList[nindex].TC, J->Name);

                    MNodeShow(tmpNodeList[nindex].N);
                }

                DBG(6, fSCHED)
                DPrint("INFO:     adding %d task(s) from node %s\n",
                       (int)tmpNodeList[nindex].TC,
                       tmpNodeList[nindex].N->Name);

                for (pindex = 0; pindex < NodeList[nindex].TC; pindex++) {
                    TaskMap[tindex] = tmpNodeList[nindex].N->Index;

                    DBG(7, fSCHED)
                    DPrint("INFO:     task[%03d] '%s' added\n", tindex,
                           tmpNodeList[nindex].N->Name);

                    tindex++;

                    if (tindex > MAX_MTASK_PER_JOB) {
                        DBG(1, fSCHED)
                        DPrint("ERROR:    tasklist for job '%s' too large\n",
                               J->Name);

                        return (FAILURE);
                    }

                    if (tindex == J->Request.TC) {
                        DBG(7, fSCHED) DPrint("INFO:     all tasks located\n");

                        break;
                    }
                }
            } /* END for (nindex) */

            TaskMap[tindex] = -1;

            if (nindex > MAX_MNODE_PER_JOB) {
                DBG(1, fSCHED)
                DPrint(
                    "ERROR:    nodelist on job '%s' is corrupt "
                    "(non-terminated)\n",
                    J->Name);

                return (FAILURE);
            }

            DBG(4, fSCHED)
            DPrint("INFO:     tasks distributed: %d (Round-Robin)\n", tindex);

            break;

        default:

            TaskMap[0] = -1;

            DBG(4, fSCHED)
            DPrint("ERROR:    unexpected task distribution (%d) in %s()\n",
                   Distribution, FName);

            return (FAILURE);

            /*NOTREACHED*/

            break;
    } /* END switch(Distribution) */

    if ((J->Req[1] != NULL) && (NodeList != NULL)) {
        int nindex1;
        int nindex2;
        int nindex3;

        /* coallesce multi-req NodeList */

        nindex3 = 0;

        for (nindex1 = 0; NodeList[nindex1].N != NULL; nindex1++) {
            if (nindex >= MAX_MNODE_PER_JOB) break;

            if (NodeList[nindex1].TC == 0) continue;

            if (nindex3 != nindex1) {
                NodeList[nindex3].N = NodeList[nindex1].N;
                NodeList[nindex3].TC = NodeList[nindex1].TC;
            }

            for (nindex2 = nindex1 + 1; NodeList[nindex2].N != NULL;
                 nindex2++) {
                if (NodeList[nindex2].N != NodeList[nindex3].N) continue;

                NodeList[nindex3].TC += NodeList[nindex2].TC;

                NodeList[nindex2].TC = 0;
            } /* END for (nindex2) */

            nindex3++;
        } /* END for (nindex1) */

        NodeList[nindex3].N = NULL;
        NodeList[nindex3].TC = 0;
    } /* END if (J->Req[1] != NULL) */

    return (SUCCESS);
} /* END MJobDistributeTasks() */

/* Helper routine for MSchedProcessJobs() */
static void m_schedule_on_partitions(

    int OnlyDefPart, /* I */
    int DoBackfill,  /* I */
    int *CurrentQ)   /* I */

{
    int PIndex;
    int tmpQ[MAX_MJOB];

    for (PIndex = 0; PIndex < MAX_MPAR; PIndex++) {
        if (((PIndex == 0) && (MPar[2].ConfigNodes == 0)) ||
            (MPar[PIndex].ConfigNodes == 0)) {
            continue;
        }

        MOQueueInitialize(tmpQ);

        if (MQueueSelectJobs(CurrentQ, tmpQ, ptSOFT, MAX_MNODE, MAX_MTASK,
                             MAX_MTIME, PIndex, NULL, TRUE,
                             OnlyDefPart) == SUCCESS) {
            MQueueScheduleIJobs(tmpQ, &MPar[PIndex]);

            if (DoBackfill == TRUE && MPar[PIndex].BFPolicy != ptOFF) {
                /* backfill jobs using 'soft' policy constraints */

                MQueueBackFill(tmpQ, ptSOFT, &MPar[PIndex]);
            }
        }

        MOQueueDestroy(tmpQ, FALSE);
    } /* END for (PIndex) */
} /* END m_schedule_on_partitions() */

int MSchedProcessJobs(

    char *OldDay,  /* I */
    int *GlobalSQ, /* I */
    int *GlobalHQ) /* I */

{
    int PIndex;

    mpar_t *P;

    int tmpQ[MAX_MJOB];
    int CurrentQ[MAX_MJOB];

    long Value;

    const char *FName = "MSchedProcessJobs";

    DBG(4, fSCHED) DPrint("%s()\n", FName);

    if ((GlobalSQ == NULL) || (GlobalHQ == NULL)) {
        return (FAILURE);
    }

    if (MRMGetInfo() == SUCCESS) {
        MQueueGetBestRQTime(MAQ, &Value);

        if ((MSched.Iteration == 0) || (MSched.Reload == TRUE)) {
            if ((MSched.Mode != msmSim) ||
                (getenv(MSCHED_ENVCKTESTVAR) != NULL)) {
                MSched.EnvChanged = TRUE;

                MCPLoad(MCP.CPFileName, (int)mckptNonRes);

                MCP.LastCPTime = MSched.Time;
            }

            if (MAM[0].Type != mamtNONE)
                MAMSyncAlloc(&MAM[0],
                             &MRM[0]); /* FIXME: include all RM indexes */
        }

        if (strcmp(OldDay, MSched.Day) != 0) {
            MSRUpdate(NULL);
        } /* END if (strcmp(OldDay,MSched.Day)) */
        else {
            /* adjust all floating/incomplete reservations */

            MSRRefresh();
        } /* END else (strcmp(OldDay,MSched.Day) != 0) */

        MSched.Reload = FALSE;

        MResAdjustDRes(NULL, 0);

        /* prioritize all jobs, sort, select idle jobs meeting queue policies */

        MOQueueInitialize(GlobalSQ);
        MOQueueInitialize(GlobalHQ);

        MQueueSelectAllJobs(MJob, ptHARD, &MPar[0], GlobalHQ, TRUE, TRUE, TRUE,
                            NULL);

        MQueueSelectAllJobs(MJob, ptSOFT, &MPar[0], GlobalSQ, TRUE, FALSE, TRUE,
                            NULL);

        if ((((MSched.Mode != msmSim) && (MSched.EnvChanged == TRUE)) ||
             ((MSched.Mode == msmSim) && (MSim.QueueChanged == TRUE))) &&
            (MSched.Schedule == TRUE)) {
            /* select/prioritize jobs for reserved scheduling */

            MOQueueInitialize(tmpQ);

            if (MQueueSelectJobs(GlobalHQ, tmpQ, ptHARD, MAX_MNODE, MAX_MTASK,
                                 MAX_MTIME, -1, NULL, FALSE,
                                 FALSE) == SUCCESS) {
                memcpy(MFQ, tmpQ, sizeof(MFQ));

                MQueueScheduleSJobs(tmpQ);

                MQueueScheduleRJobs(tmpQ);

                MOQueueDestroy(tmpQ, FALSE);
            }

            /* update policy usage */

            MQueueSelectJobs(GlobalSQ, CurrentQ, ptSOFT, MAX_MNODE, MAX_MTASK,
                             MAX_MTIME, -1, NULL, TRUE, FALSE);

            /* schedule priority jobs */

            if (CurrentQ[0] != -1) {
                /* schedule jobs on their default partitions; skip backfilling
                 */

                m_schedule_on_partitions(TRUE, FALSE, CurrentQ);

                /* schedule jobs on all partitions; do backfilling  */

                m_schedule_on_partitions(FALSE, TRUE, CurrentQ);
            } /* END if (GlobalSQ[0] != -1) */

            MOQueueDestroy(CurrentQ, TRUE);

            MQueueSelectJobs(GlobalHQ, CurrentQ, ptHARD, MAX_MNODE, MAX_MTASK,
                             MAX_MTIME, -1, NULL, TRUE, FALSE);

            if (CurrentQ[0] != -1) {
                /* backfill jobs using 'hard' policy constraints */

                for (PIndex = 0; PIndex < MAX_MPAR; PIndex++) {
                    if (((PIndex == 0) && (MPar[2].ConfigNodes == 0)) ||
                        (MPar[PIndex].ConfigNodes == 0)) {
                        continue;
                    }

                    P = &MPar[PIndex];

                    if (P->BFPolicy == bfNONE) {
                        continue;
                    }

                    MOQueueInitialize(tmpQ);

                    if (MQueueSelectJobs(CurrentQ, tmpQ, ptHARD, MAX_MNODE,
                                         MAX_MTASK, MAX_MTIME, PIndex, NULL,
                                         TRUE, FALSE) == SUCCESS) {
                        MQueueBackFill(tmpQ, ptHARD, &MPar[PIndex]);
                    }

                    MOQueueDestroy(tmpQ, FALSE);
                } /* END for (PIndex)           */
            }     /* END if (GlobalHQ[0] != -1) */

            MOQueueDestroy(CurrentQ, TRUE);

            MRMFinalizeCycle();
        } /* END if (MSched.Mode != msmSim)   */
    }     /* END if (MRMGetInfo() == SUCCESS) */
    else {
        /* RM API failed */

        /* prioritize all jobs, sort, select idle jobs meeting queue policies */

        MQueueSelectAllJobs(MJob, ptHARD, &MPar[0], GlobalHQ, TRUE, TRUE, TRUE,
                            NULL);
    } /* END else (MRMGetInfo() == SUCCESS) */

    /* update UI queue to provide updated information for client commands */

    MQueueSelectJobs(GlobalHQ, MUIQ, ptSOFT, MAX_MNODE, MAX_MTASK, MAX_MTIME,
                     -1, NULL, TRUE, FALSE);

    /* must sort/order MUIQ */

    MQueuePrioritizeJobs(NULL, MUIQ);

    return (SUCCESS);
} /* END MSchedProcessJobs() */

int MSchedUpdateStats()

{
    struct timeval tvp;
    time_t schedtime;

    double efficiency;

    double SchedTime;

    long MidNight;

    mpar_t *GP;
    must_t *S;

    const char *FName = "MSchedUpdateStats";

    DBG(2, fCORE) DPrint("%s()\n", FName);

    gettimeofday(&tvp, NULL);

    schedtime = (tvp.tv_sec - MSched.SchedTime.tv_sec) * 1000 +
                (tvp.tv_usec - MSched.SchedTime.tv_usec) / 1000;

    DBG(2, fCORE)
    DPrint("INFO:     iteration: %4d   scheduling time: %6.3f seconds\n",
           MSched.Iteration, (double)schedtime / 1000.0);

    if (MSched.Mode == msmSim) {
        SchedTime = (double)MSched.RMPollInterval / 3600.0;
    } else {
        SchedTime = (double)MSched.Interval / 100.0 / 3600.0;
    }

    GP = &MPar[0];
    S = &GP->S;

    MStat.TotalPHAvailable += (double)GP->URes.Procs * SchedTime;
    MStat.TotalPHBusy += (double)(GP->URes.Procs - GP->ARes.Procs) * SchedTime;

    S->MSAvail += (double)GP->URes.Mem * SchedTime * 3600.0;
    S->MSDedicated += (double)GP->DRes.Mem * SchedTime * 3600.0;

    if (GP->UpNodes > 0)
        efficiency = (double)GP->DRes.Procs * 100.0 / GP->URes.Procs;
    else
        efficiency = 0.0;

    if (MSched.Iteration > 5) {
        if (efficiency < MStat.MinEff) {
            MStat.MinEff = efficiency;
            MStat.MinEffIteration = MSched.Iteration;

            DBG(2, fCORE)
            DPrint(
                "INFO:     minimum efficiency reached (%8.2f percent) on "
                "iteration %d\n",
                MStat.MinEff, MStat.MinEffIteration);
        }
    }

    MResUpdateStats();

    if (MSched.Time > MAM[0].FlushTime) {
        if (MAM[0].Type == mamtNONE) {
            MAM[0].FlushTime = MAX_MTIME;
        } else {
            MUGetPeriodStart(MSched.Time, 0, 0, mpDay, &MidNight);

            MAM[0].FlushTime =
                MidNight +
                (((MSched.Time - MidNight) / MAM[0].FlushInterval) + 1) *
                    MAM[0].FlushInterval;

            MResChargeAllocation(NULL, 0);
        }
    }

    DBG(1, fCORE)
    DPrint(
        "INFO:     current util[%d]:  %d/%-d (%.2f%c)  PH: %.2f%c  active "
        "jobs: %d of %d (completed: %d)\n",
        MSched.Iteration, (GP->UpNodes - GP->IdleNodes), GP->UpNodes,
        (GP->UpNodes > 0)
            ? (double)(GP->UpNodes - GP->IdleNodes) * 100.0 / GP->UpNodes
            : (double)0.0,
        '%', (MStat.TotalPHAvailable > 0.0)
                 ? MStat.TotalPHBusy * 100.0 / MStat.TotalPHAvailable
                 : 0.0,
        '%', MStat.ActiveJobs, MStat.EligibleJobs + MStat.ActiveJobs, S->Count);

    MRM[0].FailIteration = -1;

    return (SUCCESS);
} /* END MSchedUpdateStats() */

int MSchedDiag(

    msched_t *S, /* I */
    char *SBuffer, int SBufSize, int IFlags)

{
    char *BPtr;
    if ((S == NULL) || (SBuffer == NULL)) {
        return (FAILURE);
    }

    BPtr = SBuffer;

    BPtr[0] = '\0';

    /* NYI */

    return (FAILURE);
} /* END MSchedDiag() */

/* END MSched.c */
