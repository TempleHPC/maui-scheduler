/* HEADER */

/* Contains:                                 *
 *                                           */

#include "moab.h"
#include "msched-proto.h"

extern mlog_t mlog;

extern mnode_t *MNode[];
extern mjob_t *MJob[];
extern mstat_t MStat;
extern mattrlist_t MAList;

typedef struct {
    mjob_t *J;
    long Cost;
    int Tasks;
    int Nodes;
    nodelist_t *TL;
} __preempt_prio_t;

/* internal prototypes */

int __MJobPreemptPrioComp(__preempt_prio_t *, __preempt_prio_t *);

/* END internal prototypes */

int MJobSelectPJobList(

    mjob_t *PreemptorJ,       /* I */
    int RequiredTasks,        /* I */
    int RequiredNodes,        /* I */
    mjob_t **FeasibleJobList, /* I */
    mnalloc_t *FNL,           /* I */
    mjob_t *PreempteeJList[], /* O:  list of preemptible jobs */
    int *PreempteeTCList,     /* O (proc count) */
    int *PreempteeNCList, nodelist_t *PreempteeTaskList[])

{
    mjob_t *J;
    mnode_t *N;
    mres_t *R;

    mreq_t *RQ;

    int TC;

    __preempt_prio_t pJ[MAX_MJOB];

    nodelist_t tmpTaskList;
    mcres_t PreemptRes;

    int index;
    int jindex;
    int jindex2;
    int nindex;
    int tindex;
    int rindex;

    int TotalTasks;
    int TotalNodes;

    int NodeCount;

    mbool_t Preemptor;
    mbool_t OwnerPreemptor;

    double JobRunPriority;

    const char *FName = "MJobSelectPJobList";

    DBG(2, fSCHED)
    DPrint("%s(%s,%d,%d,FJobList,PJList,PTCList,PNCList,PTL)\n", FName,
           (PreemptorJ != NULL) ? PreemptorJ->Name : "NULL", RequiredTasks,
           RequiredNodes);

    if ((PreemptorJ == NULL) || (FeasibleJobList == NULL) ||
        (FeasibleJobList[0] == NULL) || (FNL == NULL) ||
        (PreempteeJList == NULL) || (PreempteeTCList == NULL) ||
        (PreempteeNCList == NULL)) {
        DBG(1, fSCHED)
        DPrint("ALERT:    invalid parameters passed to %s\n", FName);

        return (FAILURE);
    }

    /* NOTE:  select 'best' list of jobs to preempt so as to provide */
    /*        needed tasks/nodes for preemptorJ on feasiblenodelist  */
    /*        lower 'run' priority means better preempt candidate    */

    /* determine number of available tasks associated with each job */

    index = 0;

    TotalNodes = 0;
    TotalTasks = 0;

    RQ = PreemptorJ->Req[0]; /* only support single req preemption */

    for (jindex = 0; FeasibleJobList[jindex] != NULL; jindex++) {
        MJobGetRunPriority(FeasibleJobList[jindex], 0, &JobRunPriority, NULL);

        memset(&PreemptRes, 0, sizeof(PreemptRes));
        NodeCount = 0;

        tindex = 0;

        for (nindex = 0; FNL[nindex].N != NULL; nindex++) {
            N = FNL[nindex].N;

            OwnerPreemptor = FALSE;

            if (MISSET(PreemptorJ->Flags, mjfPreemptor) == TRUE) {
                Preemptor = TRUE;
            } else {
                Preemptor = FALSE;

                /* determine if 'ownerpreempt' is active */

                for (rindex = 0; N->R[rindex] != NULL; rindex++) {
                    R = N->R[rindex];

                    if ((R == (mres_t *)1) || (R->Name[0] == '\0') ||
                        (R == (mres_t *)1) || (R->Name[0] == '\1'))
                        continue;

                    if ((MISSET(R->Flags, mrfOwnerPreempt) == FALSE) ||
                        (R->IsActive == FALSE))
                        continue;

                    if (MCredIsMatch(&PreemptorJ->Cred, R->O, R->OType) ==
                        FAILURE)
                        continue;

                    Preemptor = TRUE;
                    OwnerPreemptor = TRUE;

                    break;
                } /* END for (rindex) */
            }

            if (Preemptor == FALSE) {
                continue;
            }

            for (jindex2 = 0; jindex2 < MAX_MJOB_PER_NODE; jindex2++) {
                J = N->JList[jindex2];

                if (J == NULL) break;

                if (J != FeasibleJobList[jindex]) continue;

                TC = N->JTC[jindex2];

                if (!(J->Flags & (1 << mjfPreemptee))) {
                    DBG(6, fSCHED)
                    DPrint(
                        "INFO:     job %s not considered for preemption "
                        "(PREEMPTEE flag not set)\n",
                        J->Name);

                    continue;
                } else if (OwnerPreemptor == TRUE) {
                    /* all criteria satisfied */

                    /* NO-OP */
                } else if (J->StartPriority > PreemptorJ->StartPriority) {
                    DBG(6, fSCHED)
                    DPrint(
                        "INFO:     job %s not considered for preemption "
                        "(preempt priority too low (%ld < %ld)\n",
                        J->Name, J->StartPriority, PreemptorJ->StartPriority);

                    continue;
                }

                MCResAdd(&PreemptRes, &N->CRes, &RQ->DRes, TC, FALSE);
                NodeCount += 1;

                tmpTaskList[tindex].N = N;
                tmpTaskList[tindex].TC = TC;

                tindex++;
            } /* END for (jindex2) */
        }     /* END for (nindex) */

        DBG(6, fSCHED)
        DPrint("INFO:     preemptible job %s provides %d/%d tasks/nodes\n",
               FeasibleJobList[jindex]->Name, PreemptRes.Procs, NodeCount);

        if (PreemptRes.Procs <= 0) {
            continue;
        }

        tmpTaskList[tindex].N = NULL;

        /* determine 'cost per task' associated with job */

        pJ[index].J = FeasibleJobList[jindex];
        pJ[index].Tasks = PreemptRes.Procs;
        pJ[index].Nodes = NodeCount;

        pJ[index].Cost = (long)JobRunPriority / PreemptRes.Procs;

        if (PreempteeTaskList != NULL) {
            pJ[index].TL = (nodelist_t *)calloc(1, sizeof(nodelist_t));

            memcpy(pJ[index].TL, &tmpTaskList,
                   sizeof(mnalloc_t) * (tindex + 1));
        }

        index++;
    } /* END for (jindex) */

    pJ[index].J = NULL;

    /* sort job list */

    if (index > 1) {
        qsort((void *)&pJ[0], index, sizeof(pJ[0]),
              (int (*)(const void *, const void *))__MJobPreemptPrioComp);
    }

    for (jindex = 0; jindex < index; jindex++) {
        PreempteeJList[jindex] = pJ[jindex].J;
        PreempteeTCList[jindex] = pJ[jindex].Tasks;
        PreempteeNCList[jindex] = pJ[jindex].Nodes;

        TotalTasks += pJ[jindex].Tasks;
        TotalNodes += pJ[jindex].Nodes;

        if (PreempteeTaskList != NULL)
            PreempteeTaskList[jindex] = pJ[jindex].TL;
    } /* END for (jindex) */

    PreempteeJList[jindex] = NULL;

    if ((jindex == 0) || (RequiredTasks > TotalTasks) ||
        (RequiredNodes > TotalNodes)) {
        /* inadequate preemptible resources located */

        DBG(2, fSCHED)
        DPrint(
            "INFO:     inadequate preempt jobs (%d) located (P: %d of %d,N: %d "
            "of %d)\n",
            jindex, TotalTasks, RequiredTasks, TotalNodes, RequiredNodes);

        return (FAILURE);
    }

    return (SUCCESS);
} /* END MJobSelectPJobList() */

int __MJobPreemptPrioComp(

    __preempt_prio_t *A, /* I */
    __preempt_prio_t *B) /* I */

{
    static int tmp;

    /* order low to high */

    tmp = (int)(A->Cost - B->Cost);

    return (tmp);
} /* END __MJobPreemptPrioComp() */

/* END MPreempt.c */
