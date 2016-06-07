/*
*/

int LL2FreeData(

    mrm_t *R)

{
    /* free data */

    if (R->U.LL.JobQO != NULL) {
        ll_free_objs(R->U.LL.JobQO);

        ll_deallocate(R->U.LL.JobQO);

        R->U.LL.JobQO = NULL;
    }

    if (R->U.LL.NodeQO != NULL) {
        ll_free_objs(R->U.LL.NodeQO);

        ll_deallocate(R->U.LL.NodeQO);

        R->U.LL.NodeQO = NULL;
    }

    return (SUCCESS);
} /* END LL2FreeData() */

int LLGetClassInfo(

    mnode_t *N, char **CClass, char **AClass)

{
    int cindex;
    mclass_t *C;

    int ConsumedClasses;

    memset(N->ARes.PSlot, 0, sizeof(N->ARes.PSlot));
    memset(N->CRes.PSlot, 0, sizeof(N->CRes.PSlot));

    /* update configured classes */

    if (CClass == NULL) {
        return (FAILURE);
    }

    for (cindex = 0; CClass[cindex] != NULL; cindex++) {
#ifdef __SDSC
        if (labs((long)(CClass[cindex] - CClass[0])) > 0x40000000) {
            DBG(1, fLL)
            DPrint(
                "ALERT:    CClass may be corrupt (CClass[0] : %x  CClass[%d] : "
                "%x) (%ld)\n",
                CClass[0], cindex, CClass[cindex],
                labs((long)(CClass[cindex] - CClass[0])));

            break;
        }

        if (CClass[cindex][0] == '\0') {
            break;
        }
#endif /* __SDSC */

        DBG(8, fLL)
        DPrint("INFO:     configured class '%s' specified for node %s\n",
               CClass[cindex], N->Name);

        if ((MClassAdd(CClass[cindex], &C)) != FAILURE) {
            N->CRes.PSlot[C->Index].count++;
            N->CRes.PSlot[0].count++;
        } else {
            DBG(1, fLL)
            DPrint("ALERT:    cannot add configured class '%s' to node %s\n",
                   CClass[cindex], N->Name);
        }
    } /* END for (cindex) */

    /* update available classes */

    if (AClass == NULL) {
        return (FAILURE);
    }

    for (cindex = 0; AClass[cindex] != NULL; cindex++) {
#ifdef __SDSC
        if (labs((long)(AClass[cindex] - AClass[0])) > 0x1000000) {
            DBG(1, fLL)
            DPrint(
                "ALERT:    AClass may be corrupt (AClass[0] : %x  AClass[%d] : "
                "%x) (%ld)\n",
                AClass[0], cindex, AClass[cindex],
                labs((long)(AClass[cindex] - AClass[0])));

            break;
        }
#endif /* __SDSC */

        if (AClass[cindex][0] == '\0') {
            break;
        }

        DBG(8, fLL)
        DPrint("INFO:     available class '%s' specified for node %s\n",
               AClass[cindex], N->Name);

        if ((MClassAdd(AClass[cindex], &C)) != FAILURE) {
            if (N->CRes.PSlot[C->Index].count > N->ARes.PSlot[C->Index].count) {
                N->ARes.PSlot[C->Index].count++;
                N->ARes.PSlot[0].count++;
            } else {
                DBG(1, fLL)
                DPrint("ALERT:    class '%s' available but not configured\n",
                       AClass[cindex]);
            }
        } else {
            DBG(1, fLL)
            DPrint("ALERT:    cannot add available class '%s' to node %s\n",
                   AClass[cindex], N->Name);
        }
    } /* END for (cindex) */

    ConsumedClasses = 0;

    for (cindex = 1; cindex < MAX_MCLASS; cindex++) {
        ConsumedClasses +=
            MAX(0, N->CRes.PSlot[cindex].count - N->ARes.PSlot[cindex].count);
    }

    if (((N->State == mnsIdle) &&
         (MUNumListGetCount(MAX_PRIO_VAL, N->CRes.PSlot, N->ARes.PSlot, 0,
                            NULL) == FAILURE)) ||
        (MUNumListGetCount(MAX_PRIO_VAL, N->ARes.PSlot, N->CRes.PSlot, 0,
                           NULL) == FAILURE)) {
        /* loadleveler corruption */

        DBG(1, fLL)
        DPrint("ALERT:    %s node %s has class mismatch.  classes: %s\n",
               MAList[eNodeState][N->State], N->Name,
               MUCAListToString(N->ARes.PSlot, N->CRes.PSlot, NULL));

        if (MSched.Mode == msmNormal) {
            MOSSyslog(LOG_NOTICE,
                      "node %s in state %s has class mismatch.  classes: %s\n",
                      N->Name, MAList[eNodeState][N->State],
                      MUCAListToString(N->ARes.PSlot, N->CRes.PSlot, NULL));
        }
    }

    /* adjust class by max procs */

    if (N->AP.HLimit[mptMaxProc][0] > 1) {
        N->CRes.PSlot[0].count =
            MIN(N->CRes.PSlot[0].count, N->AP.HLimit[mptMaxProc][0]);

        N->ARes.PSlot[0].count = MIN(N->ARes.PSlot[0].count,
                                     N->CRes.PSlot[0].count - ConsumedClasses);

        if (N->ARes.PSlot[0].count > MAX_MTASK) N->ARes.PSlot[0].count = 0;
    }

    return (SUCCESS);
} /* END LLGetClassInfo() */

/* END OLLI.c */
