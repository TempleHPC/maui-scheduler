/* HEADER */

/* Contains:                                    *
 *                                              *
 * int MCPCreate(CheckPointFile)                *
 * int MCPWriteStandingReservations(ckfp,Buffer) *
 * int MCPWriteUsers(ckfp)                      *
 * int MCPWriteGroups(ckfp)                     *
 * int MCPWriteAccounts(ckfp)                   *
 * int MCPWriteSystemStats(ckfp)                *
 * int MCPWritemust_tats(ckfp)                  *
 * int MCPLoad(CheckPointFile,Mode)             *
 * int MCPRestore(ckIndex,OName,O)              *
 * int MCPLoadStandingReservation(ckfp,Buffer)  *
 * int MCPLoadNode(Line,Nptr)                   *
 * int MCPLoadSysStats(Line)                    *
 *                                              */

#include "moab.h"
#include "msched-proto.h"

extern mlog_t mlog;

extern msched_t MSched;
extern mpar_t MPar[];
extern msim_t MSim;
extern mstat_t MStat;
extern mjob_t *MJob[];
extern mnode_t *MNode[];
extern mres_t *MRes[];
extern mckpt_t MCP;

extern sres_t SRes[];
extern sres_t OSRes[];

extern mgcred_t *MUser[];
extern mgcred_t MGroup[];
extern mgcred_t MAcct[];

extern const char *MCPType[];
extern const char *MResType[];

extern mx_t X;

const int DefCPStatAList[] = {mstaTJobCount,  mstaTNJobCount,   mstaTQueueTime,
                              mstaMQueueTime, mstaTReqWTime,    mstaTExeWTime,
                              mstaTPSReq,     mstaTPSExe,       mstaTPSDed,
                              mstaTPSUtl,     mstaTNJobAcc,     mstaTXF,
                              mstaTNXF,       mstaMXF,          mstaTBypass,
                              mstaMBypass,    mstaTQOSAchieved, -1};

int MCPStoreQueue(

    mckpt_t *CP, /* I */
    mjob_t **JL) /* I */

{
    mjob_t *J;

    char tmpLine[MAX_MLINE];

    const char *FName = "MCPStoreQueue";

    DBG(3, fCKPT) DPrint("%s(ckfp,Buffer)\n", FName);

    if ((JL == NULL) || (JL[0] == NULL) || (JL[0]->Next == NULL)) {
        return (SUCCESS);
    }

    for (J = JL[0]->Next; J != JL[0]; J = J->Next) {
        if (J->Name[0] == '\0') continue;

        DBG(4, fCKPT) DPrint("INFO:     checkpointing job '%s'\n", J->Name);

        /* create job XML */

        MJobStoreCP(J, tmpLine);

        fprintf(CP->fp, "%-9s %20s %9ld %s\n", MCPType[mcpJob], J->Name,
                MSched.Time, tmpLine);
    } /* END for (jindex) */

    return (SUCCESS);
} /* END MCPStoreQueue() */

int MCPCreate(

    char *CPFile)

{
    int count;

    mckpt_t *CP;

    char tmpCPFileName[MAX_MLINE];

    char tmpBuf[MAX_MBUFFER];

    const char *FName = "MCPCreate";

    DBG(3, fCKPT) DPrint("%s(%s)\n", FName, (CPFile != NULL) ? CPFile : "NULL");

    if ((CPFile == NULL) || (CPFile[0] == '\0')) return (FAILURE);

    CP = &MCP;

    /* load old checkpoint file */

    if (MCPIsSupported(CP, CP->DVersion) == TRUE) {
        if ((CP->OBuffer = MFULoad(CPFile, 1, macmRead, &count, NULL)) ==
            NULL) {
            DBG(1, fCKPT)
            DPrint("WARNING:  cannot load checkpoint file '%s'\n", CPFile);
        }
    } else {
        CP->OBuffer = NULL;
    }

    MUStrCpy(tmpCPFileName, CPFile, sizeof(tmpCPFileName));
    MUStrCat(tmpCPFileName, ".tmp", sizeof(tmpCPFileName));

    /* open checkpoint file */

    if ((CP->fp = fopen(tmpCPFileName, "w+")) == NULL) {
        DBG(1, fCKPT)
        DPrint("WARNING:  cannot open checkpoint file '%s'.  errno: %d (%s)\n",
               CPFile, errno, strerror(errno));

        return (FAILURE);
    }

    /* checkpoint scheduler, job, reservation, and node state information */

    MSchedToString(&MSched, tmpBuf);
    MCPStoreObj(CP->fp, mcpSched, "sched", tmpBuf);

    MSysToString(&MSched, tmpBuf, TRUE);
    MCPStoreObj(CP->fp, mcpSys, "sys", tmpBuf);

    MCPStoreQueue(CP, MJob);

    MCPStoreResList(CP, MRes);

    MCPStoreSRList(CP, SRes);

    MCPStoreCluster(CP, MNode);

    MCPStoreUserList(CP, MUser);
    MCPStoreGroupList(CP, MGroup);
    MCPStoreAcctList(CP, MAcct);

    MCPWriteGridStats(CP->fp);

    MCPWriteSystemStats(CP->fp);

    fclose(CP->fp);

    MUStrCpy(tmpCPFileName, CPFile, sizeof(tmpCPFileName));
    MUStrCat(tmpCPFileName, ".1", sizeof(tmpCPFileName));

    if (rename(CPFile, tmpCPFileName) == -1) {
        DBG(0, fCORE)
        DPrint(
            "ERROR:    cannot rename checkpoint file '%s' to '%s' errno: %d "
            "(%s)\n",
            CPFile, tmpCPFileName, errno, strerror(errno));
    }

    MUStrCpy(tmpCPFileName, CPFile, sizeof(tmpCPFileName));
    MUStrCat(tmpCPFileName, ".tmp", sizeof(tmpCPFileName));

    if (rename(tmpCPFileName, CPFile) == -1) {
        DBG(0, fCORE)
        DPrint(
            "ERROR:    cannot rename checkpoint file '%s' to '%s' errno: %d "
            "(%s)\n",
            tmpCPFileName, CPFile, errno, strerror(errno));
    }

    return (SUCCESS);
} /* END MCPCreate() */

int MCPRestore(

    int CKIndex, /* I */
    char *OName, /* I */
    void *O)     /* I (modified) */

{
    int count;
    char *ptr;
    char *tmp;

    char Line[MAX_MLINE];

    const char *FName = "MCPRestore";

    DBG(4, fCKPT) DPrint("%s(%s,%s,Optr)\n", FName, MCPType[CKIndex], OName);

    if ((O == NULL) || (OName == NULL)) {
        return (FAILURE);
    }

    /* load checkpoint file */

    if (MCP.Buffer == NULL) {
        if ((MCP.Buffer =
                 MFULoad(MCP.CPFileName, 1, macmWrite, &count, NULL)) == NULL) {
            DBG(1, fCKPT)
            DPrint("WARNING:  cannot load checkpoint file '%s'\n",
                   MCP.CPFileName);

            return (FAILURE);
        }
    }

    /* NOTE:  CP version should be verified */

    /* create header */

    sprintf(Line, "%-9s %20s ", MCPType[CKIndex], OName);

    if ((ptr = strstr(MCP.Buffer, Line)) == NULL) {
        /* no checkpoint entry for object */

        DBG(5, fCKPT)
        DPrint("INFO:     no checkpoint entry for object '%s'\n", Line);

        return (SUCCESS);
    }

    /* terminate line */

    if (((tmp = strchr(ptr, '\n')) != NULL) && (tmp - ptr < MAX_MLINE)) {
        MUStrCpy(Line, ptr, MIN((long)sizeof(Line), (tmp - ptr + 1)));
    } else {
        DBG(1, fCKPT)
        DPrint("WARNING:  cannot read checkpoint line '%s'\n", Line);

        return (FAILURE);
    }

    switch (CKIndex) {
        case mcpJob:

            MJobLoadCP((mjob_t *)O, Line);

            break;

        case mcpNode:

            MNodeLoadCP((mnode_t *)O, Line);

            break;

        case mcpUser:

            MUserLoadCP((mgcred_t *)O, Line);

            break;

        case mcpGroup:

            MGroupLoadCP((mgcred_t *)O, Line);

            break;

        case mcpAcct:

            MAcctLoadCP((mgcred_t *)O, Line);

            break;

        case mcpSched:

            MCPLoadSched(&MCP, Line, (msched_t *)O);

            break;

        case mcpSys:

            MCPLoadSys(&MCP, Line, (msched_t *)O);

            break;

        case mcpSRes:

            /* NYI */

            break;

        default:

            DBG(1, fCKPT)
            DPrint(
                "ERROR:    unexpected checkpoint type, %d, detected in %s()\n",
                CKIndex, FName);

            break;
    } /* END switch (CKIndex) */

    return (SUCCESS);
} /* END MCPRestore() */

int MCPLoad(

    char *CPFile, /* I (utilized) */
    int Mode)     /* I */

{
    char *ptr;

    char *head;

    int ckindex;
    int count;

    int buflen;

    char *tokptr;

    static int FailureIteration = -999;

    const char *FName = "MCPLoad";

    DBG(3, fCKPT)
    DPrint("%s(%s,%s)\n", FName, CPFile,
           (Mode == mckptResOnly) ? "ResOnly" : "NonRes");

    if ((MSched.Mode == msmSim) && (getenv(MSCHED_ENVCKTESTVAR) == NULL)) {
        /* checkpointing not enabled */

        return (SUCCESS);
    }

    if (FailureIteration == MSched.Iteration) {
        return (FAILURE);
    }

    /* load checkpoint file */

    if ((MCP.OBuffer = MFULoad(CPFile, 1, macmWrite, &count, NULL)) == NULL) {
        DBG(1, fCKPT)
        DPrint("WARNING:  cannot load checkpoint file '%s'\n", CPFile);

        FailureIteration = MSched.Iteration;

        return (FAILURE);
    }

    head = MCP.OBuffer;
    buflen = strlen(head);

    /* determine checkpoint version */

    if (MCP.DVersion[0] == '\0') {
        MCPRestore(mcpSched, "sched", (void *)&MSched);

        if (MCPIsSupported(&MCP, MCP.DVersion) == FALSE) {
            /* cannot load checkpoint file */

            return (FAILURE);
        }
    }

    ptr = MUStrTok(head, "\n", &tokptr);

    while (ptr != NULL) {
        head = ptr + strlen(ptr) + 1;

        if ((head - MCP.OBuffer) > buflen) {
            head = MCP.OBuffer + buflen;
        }

        for (ckindex = 0; MCPType[ckindex] != NULL; ckindex++) {
            if (strncmp(ptr, MCPType[ckindex], strlen(MCPType[ckindex])))
                continue;

            if ((Mode == mckptResOnly) && (ckindex != mcpRes) &&
                (ckindex != mcpSRes) && (ckindex != mcpSched)) {
                break;
            } else if ((Mode == mckptNonRes) && (ckindex == mcpRes)) {
                break;
            }

            DBG(5, fCKPT)
            DPrint("INFO:     loading %s checkpoint data '%s'\n",
                   MCPType[ckindex], ptr);

            switch (ckindex) {
                case mcpSched:

                    if (MCPLoadSched(&MCP, ptr, &MSched) == FAILURE) {
                        DBG(1, fCKPT)
                        DPrint(
                            "ALERT:    cannot load sched data.  aborting "
                            "checkpoint load\n");

                        free(MCP.OBuffer);

                        MCP.OBuffer = NULL;

                        return (FAILURE);
                    }

                    break;

                case mcpSys:

                    MCPLoadSys(&MCP, ptr, &MSched);

                    break;

                case mcpJob:

                    MJobLoadCP(NULL, ptr);

                    break;

                case mcpRes:

                    MResLoadCP(NULL, ptr);

                    break;

                case mcpSRes:

                    MCPLoadSR(ptr);

                    break;

                case mcpNode:

                    MNodeLoadCP(NULL, ptr);

                    break;

                case mcpUser:

                    MUserLoadCP(NULL, ptr);

                    break;

                case mcpGroup:

                    MGroupLoadCP(NULL, ptr);

                    break;

                case mcpAcct:

                    MAcctLoadCP(NULL, ptr);

                    break;

                case mcpTotal:
                case mcpRTotal:
                case mcpCTotal:
                case mcpGTotal:

                    MCPLoadStats(ptr);

                    break;

                case mcpSysStats:

                    MCPLoadSysStats(ptr);

                    break;

                default:

                    DBG(1, fCKPT)
                    DPrint(
                        "ERROR:    line '%s' not handled in checkPoint file "
                        "'%s'\n",
                        ptr, CPFile);

                    break;
            } /* END switch(ckindex) */

            break;
        } /* END for (ckindex) */

        if (MCPType[ckindex] == NULL) {
            DBG(3, fCKPT)
            DPrint("WARNING:  unexpected line '%s' in checkpoint file '%s'\n",
                   ptr, CPFile);
        }

        ptr = MUStrTok(NULL, "\n", &tokptr);
    } /* END while (ptr != NULL) */

    free(MCP.OBuffer);

    MCP.OBuffer = NULL;

    if (Mode == mckptNonRes) {
        MSysSynchronize();
    }

    return (SUCCESS);
} /* END MCPLoad() */

int MCPStoreResList(

    mckpt_t *CP, mres_t **RL)

{
    int rindex;

    mres_t *R;

    char tmpBuf[MAX_MBUFFER];

    mxml_t *E = NULL;

    const char *FName = "MCPStoreResList";

    DBG(3, fCKPT) DPrint("%s(CP,RL)\n", FName);

    if ((CP == NULL) || (RL == NULL)) {
        return (FAILURE);
    }

    for (rindex = 0; rindex < MAX_MRES; rindex++) {
        R = RL[rindex];

        if ((R == NULL) || (R->Name[0] == '\0')) break;

        if (R->Name[0] == '\1') {
            DBG(7, fCKPT)
            DPrint("INFO:     skipping empty reservation[%02d]\n", rindex);

            continue;
        }

        /* ignore job and SR based reservations */

        if ((R->Type == mrtJob) || (R->Flags & (1 << mrfStandingRes))) {
            DBG(6, fCKPT)
            DPrint("INFO:     ignoring reservation[%02d] '%s'\n", rindex,
                   R->Name);

            continue;
        }

        DBG(4, fCKPT)
        DPrint("INFO:     checkpointing Res[%02d] '%s'\n", rindex, R->Name);

        MXMLCreateE(&E, "res");

        MResToXML(R, E, NULL);

        MXMLToString(E, tmpBuf, MAX_MBUFFER, NULL, TRUE);

        MXMLDestroyE(&E);

        MCPStoreObj(CP->fp, mcpRes, R->Name, tmpBuf);
    } /* END for (rindex) */

    return (SUCCESS);
} /* END MCPStoreResList() */

int MCPStoreObj(

    FILE *CFP,   /* I (utilized) */
    int CPType,  /* I */
    char *OName, /* I */
    char *Buf)   /* I */

{
    fprintf(CFP, "%-9s %20s %9ld %s\n", MCPType[CPType], OName, MSched.Time,
            Buf);

    return (SUCCESS);
} /* END MCPStoreObj() */

int MCPStoreSRList(

    mckpt_t *CP, /* I (utilized) */
    sres_t *SRL) /* I */

{
    int srindex;
    sres_t *SR;

    const char *FName = "MCPStoreSRList";

    DBG(3, fCKPT) DPrint("%s(CP,SRL)\n", FName);

    if ((CP == NULL) || (SRL == NULL)) return (FAILURE);

    for (srindex = 0; srindex < MAX_MSRES; srindex++) {
        SR = &SRes[srindex];

        if ((SR->TaskCount == 0) && (SR->HostExpression[0] == '\0')) {
            DBG(9, fCKPT)
            DPrint("INFO:     skipping empty SR[%02d]\n", srindex);

            continue;
        }

        DBG(4, fCKPT) DPrint("INFO:     checkpointing SR '%s'\n", SR->Name);

        /*
            MSRToString(SR,tmpLine);

            MCPStoreObj(CP->fp,mcpSRes,SR->Name,tmpLine);
        */

        /* checkpoint statistics and override values */

        /* FORMAT:      TYPE NAME TIME IDTME TOTME TCT ACCT STAR END */

        fprintf(CP->fp, "%-9s %20s %9ld %6.2f %6.2f %3d %14s %9ld %9ld\n",
                MCPType[mcpSRes], SR->Name, MSched.Time, SR->IdleTime,
                SR->TotalTime, OSRes[srindex].TaskCount,
                (OSRes[srindex].A != NULL) ? OSRes[srindex].A->Name : NONE,
                OSRes[srindex].StartTime, OSRes[srindex].EndTime);
    } /* END for (srindex) */

    fflush(CP->fp);

    return (FAILURE);
} /* END MCPStoreSResList() */

int MCPStoreCluster(

    mckpt_t *CP, mnode_t **NL)

{
    int nindex;

    mnode_t *N;

    char *ptr;
    char *tail;
    char tmpLine[MAX_MLINE];
    char Name[MAX_MNAME];

    long CheckPointTime;

    const char *FName = "MCPStoreCluster";

    DBG(3, fCKPT) DPrint("%s(CP,NL)\n", FName);

    /* store active node info */

    for (nindex = 0; nindex < MAX_MNODE; nindex++) {
        N = NL[nindex];

        if ((N == NULL) || (N->Name[0] == '\0')) break;

        if (N->Name[0] == '\1') continue;

        DBG(4, fCKPT) DPrint("INFO:     checkpointing node '%s'\n", N->Name);

        MNodeToString(N, tmpLine);

        MCPStoreObj(CP->fp, mcpNode, N->Name, tmpLine);
    } /* END for (nindex) */

    /* maintain old node CP info */

    if (CP->OBuffer != NULL) {
        ptr = CP->OBuffer;

        while ((ptr = strstr(ptr, MCPType[mcpNode])) != NULL) {
            sscanf(ptr, "%s %s %ld", tmpLine, Name, &CheckPointTime);

            if ((MSched.Time - CheckPointTime) > CP->CPExpirationTime) {
                DBG(2, fCKPT)
                DPrint(
                    "INFO:     expiring checkpoint data for node %s.  not "
                    "updated in %s\n",
                    Name, MULToTString(MSched.Time - CheckPointTime));

                ptr += strlen(MCPType[mcpNode]);

                continue;
            }

            if (MNodeFind(Name, &N) == SUCCESS) {
                /* current node info already stored */

                ptr += strlen(MCPType[mcpNode]);

                continue;
            }

            if ((tail = strchr(ptr, '\n')) == NULL) {
                DBG(1, fCKPT)
                DPrint("ALERT:    checkpoint file corruption at offset %d\n",
                       (int)(ptr - CP->OBuffer));

                ptr += strlen(MCPType[mcpNode]);

                continue;
            }

            /* copy old data */

            MUStrCpy(tmpLine, ptr, MIN((long)sizeof(tmpLine), tail - ptr + 1));

            fprintf(CP->fp, "%s\n", tmpLine);

            ptr = tail;
        } /* END while (ptr = strstr()) */
    }     /* END if (Buffer != NULL)    */

    fflush(CP->fp);

    return (SUCCESS);
} /* END MCPStoreCluster() */

int MCPIsSupported(

    mckpt_t *CP, char *Version)

{
    if ((CP == NULL) || (Version == NULL)) return (FALSE);

    if (strstr(CP->SVersionList, Version)) {
        return (TRUE);
    }

    return (FALSE);
} /* END MCPIsSupported() */

int MCPLoadSched(

    mckpt_t *CP, char *Line, msched_t *S)

{
    char TempName[MAX_MNAME];
    char tmpLine[MAX_MLINE];
    long CkTime;

    char *ptr;

    const char *FName = "MCPLoadSched";

    DBG(4, fCKPT) DPrint("%s(CP,Line,S)\n", FName);

    /* FORMAT:   TY TM Name DATA */

    sscanf(Line, "%s %ld %s", TempName, &CkTime, tmpLine);

    if ((ptr = strchr(Line, '<')) == NULL) {
        return (FAILURE);
    }

    MSchedFromString(S, ptr);

    return (SUCCESS);
} /* END MCPLoadSched() */

int MCPLoadSys(

    mckpt_t *CP, /* I */
    char *Line,  /* I */
    msched_t *S) /* I (modified) */

{
    char TempName[MAX_MNAME];
    char tmpLine[MAX_MLINE];
    long CkTime;

    char *ptr;

    const char *FName = "MCPLoadSys";

    DBG(4, fCKPT) DPrint("%s(CP,Line,S)\n", FName);

    /* FORMAT:   TY TM Name DATA */

    sscanf(Line, "%s %ld %s", TempName, &CkTime, tmpLine);

    if ((ptr = strchr(Line, '<')) == NULL) {
        return (FAILURE);
    }

    MOFromString((void *)S, mxoSys, ptr);

    return (SUCCESS);
} /* END MCPLoadSys() */

int MCPStoreUserList(

    mckpt_t *CP,   /* I */
    mgcred_t **UL) /* I */

{
    int uindex;

    mgcred_t *U;

    char tmpLine[MAX_MLINE];

    const char *FName = "MCPStoreUserList";

    DBG(3, fCKPT) DPrint("%s(CP,UL)\n", FName);

    /* store active user info */

    for (uindex = 0; uindex < MAX_MUSER + MAX_MHBUF; uindex++) {
        U = UL[uindex];

        if ((U == NULL) || (U->Name[0] == '\0')) continue;

        if (U->Name[0] == '\1') continue;

        DBG(4, fCKPT) DPrint("INFO:     checkpointing user '%s'\n", U->Name);

        MUserToString(U, tmpLine);

        MCPStoreObj(CP->fp, mcpUser, U->Name, tmpLine);
    } /* END for (uindex) */

    return (SUCCESS);
} /* END MCPStoreUserList() */

int MCPStoreGroupList(

    mckpt_t *CP, mgcred_t *GL)

{
    int gindex;

    mgcred_t *G;

    char tmpLine[MAX_MLINE];

    const char *FName = "MCPStoreGroupList";

    DBG(3, fCKPT) DPrint("%s(CP,GL)\n", FName);

    /* store active group info */

    for (gindex = 0; gindex < MAX_MGROUP + MAX_MHBUF; gindex++) {
        G = &GL[gindex];

        if (G->Name[0] == '\0') continue;

        if (G->Name[0] == '\1') continue;

        DBG(4, fCKPT) DPrint("INFO:     checkpointing group '%s'\n", G->Name);

        MGroupToString(G, tmpLine);

        MCPStoreObj(CP->fp, mcpGroup, G->Name, tmpLine);
    } /* END for (gindex) */

    return (SUCCESS);
} /* END MCPStoreGroupList() */

int MCPStoreAcctList(

    mckpt_t *CP, mgcred_t *AL)

{
    int aindex;

    mgcred_t *A;

    char tmpLine[MAX_MLINE];

    const char *FName = "MCPStoreAcctList";

    DBG(3, fCKPT) DPrint("%s(CP,AL)\n", FName);

    /* store active account info */

    for (aindex = 0; aindex < MAX_MACCT + MAX_MHBUF; aindex++) {
        A = &AL[aindex];

        if (A->Name[0] == '\0') continue;

        if (A->Name[0] == '\1') continue;

        DBG(4, fCKPT) DPrint("INFO:     checkpointing account '%s'\n", A->Name);

        MAcctToString(A, tmpLine);

        MCPStoreObj(CP->fp, mcpAcct, A->Name, tmpLine);
    } /* END for (aindex) */

    return (SUCCESS);
} /* END MCPStoreAcctList() */

int CPRecordLostJobs(

    char *CPFile)

{
    int count;
    char *Buffer;
    char *head;
    char *ptr;

    char *tokptr;

    char Line[MAX_MLINE];

    int buflen;

    mjob_t tmpJ;
    mreq_t tmpRQ;

    mjob_t *J;

    char tmpLine[MAX_MLINE];
    char JobName[MAX_MLINE];

    DBG(3, fCKPT) DPrint("CPRecordLostJobs(%s)\n", CPFile);

    /* load checkpoint file */

    if ((Buffer = MFULoad(CPFile, 1, macmWrite, &count, NULL)) == NULL) {
        DBG(1, fCKPT)
        DPrint("WARNING:  cannot load checkpoint file '%s'\n", CPFile);

        return (FAILURE);
    }

    head = Buffer;
    buflen = strlen(head);

    ptr = MUStrTok(head, "\n", &tokptr);

    while (ptr != NULL) {
        head = ptr + strlen(ptr) + 1;

        if ((head - Buffer) > buflen) {
            head = Buffer + buflen;
        }

        if (!strncmp(ptr, MCPType[mcpJob], strlen(MCPType[mcpJob]))) {
            /* determine job completion time estimate */

            sscanf(Line, "%s %s", tmpLine, JobName);

            if (MJobFind(JobName, &J, 0) == FAILURE) {
                memset(&tmpJ, 0, sizeof(tmpJ));
                memset(&tmpRQ, 0, sizeof(tmpRQ));

                /* FIXME:  only one req handled in RecordLostJobs() */

                tmpJ.Req[0] = &tmpRQ;

                if (MJobLoadCP(&tmpJ, ptr) == SUCCESS) {
                    /* create temp record */

                    tmpJ.State = mjsLost;

                    MJobWriteStats(&tmpJ);
                }
            }
        } /* END if (!strcmp(ptr,MCPType[])) */
    }     /* END while (ptr != NULL) */

    return (SUCCESS);
} /* END CPRecordLostJobs() */

int MCPWriteGridStats(

    FILE *ckfp) /* I (utilized) */

{
    int rindex;
    int cindex;

    char tmpBuf[MAX_MBUFFER];

    const char *FName = "MCPWriteGridStats";

    DBG(3, fCKPT) DPrint("%s(ckfp)\n", FName);

    /* global total stats */

    MStatToString(&MPar[0].S, tmpBuf, (int *)DefCPStatAList);

    fprintf(ckfp, "%-9s %4d:%-4d %9ld %s\n", MCPType[mcpTotal], -1, -1,
            MSched.Time, tmpBuf);

    for (rindex = 0; rindex <= MStat.P.NodeStepCount; rindex++) {
        /* step through all rows */

        for (cindex = 0; cindex <= MStat.P.TimeStepCount; cindex++) {
            if (rindex == 0) {
                /* column total */

                MStatToString(&MStat.CTotal[cindex], tmpBuf,
                              (int *)DefCPStatAList);

                fprintf(ckfp, "%-9s %4d:%-4d %9ld %s\n", MCPType[mcpCTotal], -1,
                        cindex, MSched.Time, tmpBuf);
            }

            /* step through all columns */

            DBG(7, fCKPT)
            DPrint("INFO:     checkpointing Grid[%02d][%02d]\n", rindex,
                   cindex);

            MStatToString(&MStat.Grid[rindex][cindex], tmpBuf,
                          (int *)DefCPStatAList);

            fprintf(ckfp, "%-9s %4d:%-4d %9ld %s\n", MCPType[mcpGTotal], rindex,
                    cindex, MSched.Time, tmpBuf);
        } /* END for (cindex) */

        /* row total */

        MStatToString(&MStat.RTotal[rindex], tmpBuf, (int *)DefCPStatAList);

        fprintf(ckfp, "%-9s %4d:%-4d %9ld %s\n", MCPType[mcpRTotal], rindex, -1,
                MSched.Time, tmpBuf);
    } /* END for (rindex) */

    fflush(ckfp);

    return (SUCCESS);
} /* END MCPWriteGridStats() */

int MCPLoadStats(

    char *Buf) /* I */

{
    char tmpName[MAX_MNAME];

    int RIndex;
    int CIndex;

    long CkTime;

    int rc;

    char *ptr;

    must_t *S;

    const char *FName = "MCPLoadStats";

    DBG(4, fCKPT) DPrint("%s(%s)\n", FName, (Buf != NULL) ? Buf : "NULL");

    if (Buf == NULL) {
        return (FAILURE);
    }

    if ((rc = sscanf(Buf, "%32s %d:%d %ld ", tmpName, &RIndex, &CIndex,
                     &CkTime)) != 4) {
        return (FAILURE);
    }

    if ((MSched.Time - CkTime) > MCP.CPExpirationTime) {
        return (SUCCESS);
    }

    if ((ptr = strchr(Buf, '<')) == NULL) {
        return (FAILURE);
    }

    if (RIndex == -1) {
        if (CIndex == -1) {
            /* global total */

            S = &MPar[0].S;
        } else {
            /* column total */

            S = &MStat.CTotal[CIndex];
        }
    } else if (CIndex == -1) {
        /* row total */

        S = &MStat.RTotal[RIndex];
    } else {
        /* grid */

        S = &MStat.Grid[RIndex][CIndex];
    }

    if (MStatFromString(ptr, S) == FAILURE) {
        return (FAILURE);
    }

    return (SUCCESS);
} /* END MCPLoadStats() */

int MCPWriteSystemStats(

    FILE *ckfp)

{
    must_t *T;

    const char *FName = "MCPWriteSystemStats";

    DBG(3, fCKPT) DPrint("%s(ckfp)\n", FName);

    /* must restore all statistics required to maintain stats */

    T = &MPar[0].S;

    /*            LABL TIME PHAVAIL PHBUSY  ITM CT SJ MSA MSD NJA JAC MXF MB MQT
     * TXF TB TQT PSR PSD PSU JE SPH WEF WIT */

    fprintf(ckfp,
            "%-9s %9ld %10.3lf %10.3lf %ld %d %d %lf %lf %lf %lf %lf %d %ld "
            "%lf %d %ld %lf %lf %lf %d %lf %lf %d\n",
            MCPType[mcpSysStats], MSched.Time, MStat.TotalPHAvailable,
            MStat.TotalPHBusy, MStat.InitTime, T->Count,
            MStat.SuccessfulJobsCompleted, T->MSAvail, T->MSDedicated,
            T->NJobAcc, T->JobAcc, T->MaxXFactor, T->MaxBypass, T->MaxQTS,
            T->PSXFactor, T->Bypass, T->TotalQTS, T->PSRun, T->PSDedicated,
            T->PSUtilized, MStat.JobsEvaluated, MStat.SuccessfulPH,
            MStat.MinEff, MStat.MinEffIteration);

    fflush(ckfp);

    return (SUCCESS);
} /* END MCPWriteSystemStats() */

int MCPLoadSR(

    char *Line) /* I */

{
    int srindex;
    sres_t *SR;

    sres_t tmpSR;

    char tmp[MAX_MLINE];

    char tmpAName[MAX_MNAME];

    long CkTime;

    char *ptr;

    const char *FName = "MCPLoadSR";

    DBG(4, fCKPT) DPrint("%s(%s)\n", FName, Line);

    SR = &tmpSR;

    memset(SR, 0, sizeof(sres_t));

    sscanf(Line, "%s %s %ld", tmp, SR->Name, &CkTime);

    if ((MSched.Time - CkTime) > MCP.CPExpirationTime) {
        return (SUCCESS);
    }

    if ((ptr = strchr(Line, '<')) == NULL) {
        return (FAILURE);
    }

    MSRFromString(SR, ptr);

    if (strcmp(tmpAName, NONE)) {
        MAcctFind(tmpAName, &SR->A);
    }

    /* must locate existing standing reservation */

    for (srindex = 0; srindex < MAX_MSRES; srindex++) {
        SR = &SRes[srindex];

        if ((SR->TaskCount == 0) && (SR->HostExpression[0] == '\0')) {
            DBG(7, fCKPT)
            DPrint("INFO:     skipping empty standing reservation[%02d]\n",
                   srindex);

            continue;
        }

        if (!strcmp(SR->Name, tmpSR.Name)) break;
    }

    if (srindex == MAX_MSRES) {
        DBG(7, fCKPT)
        DPrint("INFO:     cannot locate checkpointed sres '%s', ignoring\n",
               tmpSR.Name);

        return (SUCCESS);
    }

    memcpy(&OSRes[srindex], &tmpSR, sizeof(sres_t));

    SR->IdleTime = tmpSR.IdleTime;
    SR->TotalTime = tmpSR.TotalTime;

    return (SUCCESS);
} /* END MCPLoadSR() */

int MCPLoadSysStats(

    char *Line) /* I */

{
    char tmpName[MAX_MNAME];

    must_t tmpG;
    mstat_t tmpS;

    long InitTime;

    long CkTime;

    must_t *T;
    must_t *S;

    time_t tmpTime;

    const char *FName = "MCPLoadSysStats";

    DBG(4, fCKPT) DPrint("%s(%s)\n", FName, Line);

    memset(&tmpG, 0, sizeof(tmpG));
    memset(&tmpS, 0, sizeof(tmpS));

    T = &tmpG;

    time(&tmpTime);

    /* must restore all statistics required to maintain ShowStats */

    /*         LABL TME PHA PHB ITM CT SJ MSA MSD NJA JAC MXF MB MQT TXF TB TQT
     * PSR PSD PSU JE SPH WEF WIT */

    sscanf(Line,
           "%s %ld %lf %lf %ld %d %d %lf %lf %lf %lf %lf %d %lu %lf %d %lu %lf "
           "%lf %lf %d %lf %lf %d\n",
           tmpName, &CkTime, &tmpS.TotalPHAvailable, &tmpS.TotalPHBusy,
           &InitTime, &T->Count, &tmpS.SuccessfulJobsCompleted, &T->MSAvail,
           &T->MSDedicated, &T->NJobAcc, &T->JobAcc, &T->MaxXFactor,
           &T->MaxBypass, &T->MaxQTS, &T->PSXFactor, &T->Bypass, &T->TotalQTS,
           &T->PSRun, &T->PSDedicated, &T->PSUtilized, &tmpS.JobsEvaluated,
           &tmpS.SuccessfulPH, &tmpS.MinEff, &tmpS.MinEffIteration);

    if ((MSched.Time - CkTime) > MCP.CPExpirationTime) {
        return (SUCCESS);
    }

    S = &MPar[0].S;

    S->Count = T->Count;
    S->MSAvail = T->MSAvail;
    S->MSDedicated = T->MSDedicated;
    S->NJobAcc = T->NJobAcc;
    S->JobAcc = T->JobAcc;
    S->MaxXFactor = T->MaxXFactor;
    S->MaxBypass = T->MaxBypass;
    S->MaxQTS = T->MaxQTS;
    S->PSRun = T->PSRun;
    S->PSDedicated = T->PSDedicated;
    S->PSUtilized = T->PSUtilized;

    MStat.InitTime = InitTime;
    MStat.TotalPHAvailable = tmpS.TotalPHAvailable;
    MStat.TotalPHBusy = tmpS.TotalPHBusy;
    MStat.JobsEvaluated = tmpS.JobsEvaluated;
    MStat.SuccessfulPH = tmpS.SuccessfulPH;
    MStat.MinEff = tmpS.MinEff;
    MStat.MinEffIteration = tmpS.MinEffIteration;

    MStat.SuccessfulJobsCompleted = tmpS.SuccessfulJobsCompleted;

    return (SUCCESS);
} /* END MCPLoadSysStats() */

/* MCP.c */
