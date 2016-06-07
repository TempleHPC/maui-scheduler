/* HEADER */

/* Contains:                                 *
 *                                           */

#include "moab-proto.h"
#include "moab.h"

extern mlog_t mlog;
extern mjob_t *MJob[];
extern mpar_t MPar[];
extern msched_t MSched;

#ifdef __MAPPSIM
#include "../contrib/appsim/LocalStage.c"
#include "../contrib/appsim/Net1.c"
#endif /* __MAPPSIM */

/* prototypes */

int MASDefault(mjob_t *, int, void *, void **);
int MASDefaultCreate(mjob_t *, char *);
int MASDefaultShow(mjob_t *, char *);
int MASDefaultDestroy(mjob_t *);
int MASDefaultUpdate(mjob_t *);
int MASDefaultConfig(mjob_t *, char *);

/* structures */

/* #include "../contrib/appsim/Benchmark.c" */

typedef struct {
    char *Name;
    int (*Func)();
    int Type;
    int Default;
} asim_t;

asim_t SimDriver[] = {
    {DEFAULT, (int (*)())MASDefault, msdApplication, TRUE},
#ifdef __MAPPSIM
    {"LocalStage", (int (*)())MASLocalStage, msdApplication, FALSE},
    {"Net1", (int (*)())MASNet1, msdResource, TRUE},
#endif /* __MAPPSIM */
    {NULL, NULL, 0, FALSE},
};

typedef struct {
    char *Name;
    long ASState;
} masdefdata_t;

const char *MASDefAttributeType[] = {NONE, "STATE", NULL};

enum MASDefAttrEnum { masdefaNONE = 0, masdefaState };

int MGResFind(

    char *ResName, int ResType, xres_t **RPtr)

{
    int rindex;

    xres_t *R;

    for (rindex = 0; rindex < MAX_MNODE; rindex++) {
        R = &MPar[0].XRes[rindex];

        if (R->Name[0] == '\1') continue;

        if (R->Name[0] == '\0') break;

        if ((ResType != -1) && (ResType != R->Type)) {
            continue;
        }

        if ((ResName != NULL) && (ResName[0] == '\0') &&
            !strcmp(R->Name, ResName)) {
            continue;
        }

        if (RPtr != NULL) {
            *RPtr = R;
        }

        return (SUCCESS);
    } /* END for (rindex) */

    return (FAILURE);
} /* END MGResFind() */

int MASGetDriver(

    void **D, char *DName, int DType)

{
    int findex;

    if (D == NULL) {
        return (FAILURE);
    }

    *D = NULL;

    if (DType == 0) {
        return (FAILURE);
    }

    /* determine sim driver to use */

    for (findex = 0; SimDriver[findex].Name != NULL; findex++) {
        if ((DType != -1) && (SimDriver[findex].Type != DType)) {
            continue;
        }

        if ((SimDriver[findex].Default == TRUE) && (*D == NULL)) {
            /* load default if found */

            *D = (void *)SimDriver[findex].Func;
        }

        if ((DName == NULL) || (DName[0] == '\0')) {
            /* driver name not specified, only default used */

            continue;
        }

        if (!strcmp(DName, SimDriver[findex].Name)) {
            /* match found */

            *D = (void *)SimDriver[findex].Func;

            break;
        }
    } /* END for (findex) */

    if (*D == NULL) {
        /* default driver not located */

        return (FAILURE);
    }

    return (SUCCESS);
} /* END MASGetDriver() */

char *MASGetName(

    void *D)

{
    int findex;

    if (D != NULL) {
        for (findex = 0; SimDriver[findex].Name != NULL; findex++) {
            if (D == (void *)SimDriver[findex].Func) break;
        }

        if (SimDriver[findex].Name == NULL) findex = 0;
    } else {
        findex = 0;
    }

    return (SimDriver[findex].Name);
} /* END MASGetName() */

int MASDefault(

    mjob_t *J, int CmdIndex, void *IData, void **OData)

{
    int rc;

    if (J == NULL) return (FAILURE);

    switch (CmdIndex) {
        case mascCreate:

            rc = MASDefaultCreate(J, (char *)IData);

            break;

        case mascUpdate:

            rc = MASDefaultUpdate(J);

            break;

        case mascDestroy:

            rc = MASDefaultDestroy(J);

            break;

        case mascConfig:

            rc = MASDefaultConfig(J, (char *)IData);

            break;

        case mascShow:

            rc = MASDefaultShow(J, (char *)OData);

            break;

        default:

            rc = FAILURE;

            break;
    } /* END switch(CmdIndex) */

    return (rc);
} /* END MASDefault() */

int MASDefaultCreate(

    mjob_t *J, char *ConfigString)

{
    char *ptr;
    char *TokPtr;

    char ValLine[MAX_MLINE];

    int aindex;

    mreq_t *RQ;

    if (J == NULL) return (FAILURE);

    /* NOTE:  by default, each job uses 100% of all dedicated resources */

    /* set 'RQ->URes.Procs' */
    /* set 'RQ->URes.Mem'   */

    /* create AS data structure */

    if (J->ASData == NULL) {
        J->ASData = calloc(1, sizeof(masdefdata_t));
    }

    if (ConfigString != NULL) {
        /* process config data */

        /* FORMAT:
         * INPUT=<INPUTDATANAME>:<INPUTDATASIZE>;OUTPUT=<OUTPUTDATANAME>:<OUTPUTDATASIZE>
         */

        ptr = MUStrTok(ConfigString, "; \t\n", &TokPtr);

        while (ptr != NULL) {
            if (MUGetPair(ptr, (const char **)MASDefAttributeType, &aindex,
                          NULL, TRUE, NULL, ValLine, MAX_MNAME) == FAILURE) {
                /* cannot parse value pair */

                ptr = MUStrTok(NULL, "; \t\n", &TokPtr);

                continue;
            }

            switch (aindex) {
                default:

                    break;
            }
        } /* END while (ptr != NULL) */

        ptr = MUStrTok(NULL, "; \t\n", &TokPtr);
    } /* END if (ConfigString != NULL) */

    /* clear resource usage */

    RQ = J->Req[0];

    memset(&RQ->URes, 0, sizeof(RQ->URes));

    return (SUCCESS);
} /* END MASDefaultCreate() */

int MASDefaultShow(

    mjob_t *J, char *Buffer)

{
    if ((J == NULL) || (Buffer == NULL)) {
        return (FAILURE);
    }

    Buffer[0] = '\0';

    return (SUCCESS);
} /* END MASDefaultShow() */

int MASDefaultConfig(

    mjob_t *J, char *ConfigString)

{
    char *ptr;
    char *TokPtr;

    char ValLine[MAX_MLINE];

    int aindex;
    int index;

    masdefdata_t *D;

    if ((J == NULL) || (ConfigString == NULL)) {
        return (FAILURE);
    }

    D = (masdefdata_t *)J->ASData;

    if (D == NULL) {
        return (FAILURE);
    }

    /* process config data */

    /* FORMAT:  STATE=<STATE>; */

    ptr = MUStrTok(ConfigString, "; \t\n", &TokPtr);

    while (ptr != NULL) {
        if (MUGetPair(ptr, (const char **)MASDefAttributeType, &aindex, NULL,
                      TRUE, NULL, ValLine, MAX_MNAME) == FAILURE) {
            /* cannot parse value pair */

            ptr = MUStrTok(NULL, "; \t\n", &TokPtr);

            continue;
        }

        switch (aindex) {
            case masdefaState:

                index = MUGetIndex(ValLine, MJobState, 0, 0);

                switch (index) {
                    case mjsStarting:
                    case mjsRunning:

                        if ((D->ASState != mjsStarting) &&
                            (D->ASState != mjsRunning)) {
                            D->ASState = index;
                        }

                        break;

                    default:

                        break;
                }

                break;

            default:

                break;
        } /* END switch(aindex) */

        ptr = MUStrTok(NULL, "; \t\n", &TokPtr);
    } /* END while (ptr != NULL) */

    return (SUCCESS);
} /* END MASDefaultConfig() */

int MASDefaultDestroy(

    mjob_t *J)

{
    J->ASFunc = NULL;

    /* clear AS Data */

    if (J->ASData != NULL) {
        MUFree((char **)J->ASData);
    }

    return (SUCCESS);
} /* END MASDefaultDestroy() */

int MASDefaultUpdate(

    mjob_t *J)

{
    mreq_t *RQ;

    masdefdata_t *D;

    if (J == NULL) return (FAILURE);

    D = (masdefdata_t *)J->ASData;

    if (D == NULL) return (FAILURE);

    RQ = J->Req[0];

    /* default app sim jobs utilize 100% of dedicated resources */

    switch (J->State) {
        case mjsRunning:
        case mjsStarting:

            /* job is active */

            /* job data is static */

            if ((J->State != D->ASState) ||
                (RQ->URes.Procs != RQ->DRes.Procs * 100)) {
                RQ->URes.Procs = RQ->DRes.Procs * 100;
                RQ->URes.Mem = RQ->DRes.Mem;
                RQ->URes.Swap = RQ->DRes.Swap;
                RQ->URes.Disk = RQ->DRes.Disk;

                RQ->LURes.Procs = RQ->DRes.Procs * 100;
                RQ->LURes.Mem = RQ->DRes.Mem;
                RQ->LURes.Swap = RQ->DRes.Swap;
                RQ->LURes.Disk = RQ->DRes.Disk;

                D->ASState = J->State;
            }

            break;

        case mjsIdle:

            /* job is idle */

            /* job data is static */

            if (J->State != D->ASState) {
                memset(&RQ->URes, 0, sizeof(RQ->URes));

                D->ASState = J->State;
            }

            break;

        case mjsCompleted:
        case mjsRemoved:

            /* job is completed */

            /* job data is static */

            if (J->State != D->ASState) {
                memset(&RQ->URes, 0, sizeof(RQ->URes));

                D->ASState = J->State;
            }

            break;

        default:

            break;
    } /* END switch(J->State) */

    return (SUCCESS);
} /* END MASDefaultUpdate() */

/* END MAppSim.c */
