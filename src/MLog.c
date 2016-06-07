/* HEADER */

/* Contains:                                    *
 *                                              *
 * #define DBG(X,F)                             *
 * int DPrint(Message,...)                      *
 * int MLogInitialize(NewLogFile,NewMaxFileSize) *
 * int MLogOpen(Iteration)                      *
 * int MLogRoll()                               *
 * char *MLogGetTime()                          *
 * void MLogLevelAdjust(signo)                  *
 *                                              */

#include "moab.h"
#include "msched-proto.h"

#define MAX_LOGNAME 256
#define MAX_MDEBUG 10

#ifndef SUCCESS
#define SUCCESS 1
#endif

#ifndef FAILURE
#define FAILURE 0
#endif

#ifndef DBG

typedef struct {
    FILE *logfp;
    int Threshold;
    int FacilityList;
} mlog_t;

#define DBG(X, F) if ((mlog.Threshold >= X) && ((mlog.FacilityList) & F))

#define MDEBUG(X) if (mlog.Threshold >= X)

#endif /* DBG */

mlog_t mlog = {0, 0, 0xffffffff};

char LogFile[MAX_LOGNAME + 1];
int MaxLogFileSize = -1;

char *MLogGetTime(void);
void MLogLevelAdjust(int);

int MLogInitialize(

    char *NewLogFile,   /* I */
    int NewMaxFileSize, /* I */
    int Iteration)      /* I */

{
    static int SigSet = 0;

    if (NewMaxFileSize != -1) MaxLogFileSize = NewMaxFileSize;

    if (NewLogFile == NULL) {
        LogFile[0] = '\0';

        MLogOpen(Iteration);
    } else if ((strcmp(LogFile, "N/A") != 0) &&
               (strcmp(NewLogFile, "N/A") != 0) &&
               (strcmp(LogFile, NewLogFile) != 0)) {
        strncpy(LogFile, NewLogFile, MAX_LOGNAME);
        LogFile[MAX_LOGNAME] = '\0';

        MLogOpen(Iteration);
    }

    if (SigSet == 0) {
        signal(SIGUSR1, MLogLevelAdjust);
        signal(SIGUSR2, MLogLevelAdjust);
        SigSet = 1;
    }

    return (SUCCESS);
} /* END MLogInitialize() */

int MLogOpen(

    int Iteration) /* I */

{
    const char *FName = "MLogOpen";

    DBG(4, fCORE) DPrint("%s()\n", FName);

    if ((mlog.logfp != NULL) && (mlog.logfp != stderr)) {
        fclose(mlog.logfp);

        mlog.logfp = NULL;
    }

    /* if null logfile, send logs to stderr */

    if (LogFile[0] == '\0') {
        mlog.logfp = stderr;

        return (SUCCESS);
    } else if ((getenv(MSCHED_ENVLOGSTDERRVAR) != NULL) && (Iteration == 0)) {
        fclose(stderr);
    }

    umask(0027);

    if (umask(0027) != 0027) {
        DBG(2, fCORE)
        DPrint("ERROR:    cannot set umask on logfile '%s', errno: %d (%s)\n",
               LogFile, errno, strerror(errno));
    }

    if ((mlog.logfp = fopen(LogFile, "a+")) == NULL) {
        DBG(1, fALL)
        fprintf(stderr, "WARNING:  cannot open log file '%s', errno: %d (%s)\n",
                LogFile, errno, strerror(errno));

        /* dump logs to stderr */

        mlog.logfp = stderr;

        return (FAILURE);
    }

    /* use smallest of line and 4K buffering */

    if (setvbuf(mlog.logfp, NULL, _IOLBF, 4097) != 0) {
        fprintf(stderr,
                "WARING:  cannot setup line mode buffering on logfile\n");
    }

    return (SUCCESS);
} /* END MLogOpen() */

int MLogShutdown()

{
    if (mlog.logfp != NULL) {
        fclose(mlog.logfp);

        mlog.logfp = NULL;
    }

    return (SUCCESS);
} /* END MLogShutdown() */

#ifndef __MTEST

int DPrint(

    char *Format, /* I */
    ...)

{
    va_list Args;

    if (mlog.logfp != NULL) {
        if (mlog.logfp != stderr) {
            fprintf(mlog.logfp, "%s", MLogGetTime());
        }

        va_start(Args, Format);

        vfprintf(mlog.logfp, Format, Args);

        va_end(Args);
    } /* END if (mlog.logfp != NULL) */

    return (SUCCESS);
} /* END DPrint() */

#endif /* __MTEST */

int MLogRoll(

    char *Suffix, int Flag, int Iteration, int Depth)

{
    struct stat buf;
    int dindex;

    char OldFile[MAX_LOGNAME];
    char NewFile[MAX_LOGNAME];

    const char *FName = "MLogRoll";

    DBG(5, fCORE)
    DPrint("%s(%s,%d,%d)\n", FName, (Suffix != NULL) ? Suffix : "NULL", Flag,
           Depth);

    if (LogFile[0] == '\0') {
        /* writing log data to stderr */

        return (SUCCESS);
    }

    if (Flag == 0) {
        if (MaxLogFileSize == -1) {
            /* no file size limit specified */

            return (SUCCESS);
        }

        if (stat(LogFile, &buf) == -1) {
            DBG(0, fCORE)
            DPrint("WARNING:  cannot get stats on file '%s', errno: %d (%s)\n",
                   LogFile, errno, strerror(errno));

            /* re-open log file if necessary */

            if (errno == ENOENT) {
                MDEBUG(1) {
                    fprintf(stderr,
                            "WARNING:  logfile '%s' was removed  (re-opening "
                            "file)\n",
                            LogFile);
                }

                MLogOpen(Iteration);
            }

            return (SUCCESS);
        }
    } /* END if (Flag == 0) */

    if (((LogFile[0] != '\0') || (mlog.logfp != stderr)) &&
        ((buf.st_size > MaxLogFileSize) || (Flag == 1))) {
        if ((Suffix != NULL) && (Suffix[0] == '\0')) {
            sprintf(NewFile, "%s%s", LogFile, Suffix);

            DBG(2, fCORE)
            DPrint("INFO:     rolling logfile '%s' to '%s'\n", LogFile,
                   NewFile);

            if (rename(LogFile, NewFile) == -1) {
                DBG(0, fCORE)
                DPrint(
                    "ERROR:    cannot rename logfile '%s' to '%s' errno: %d "
                    "(%s)\n",
                    LogFile, NewFile, errno, strerror(errno));
            }
        } else {
            for (dindex = Depth; dindex > 0; dindex--) {
                sprintf(NewFile, "%s.%d", LogFile, dindex);

                if (dindex == 1)
                    strcpy(OldFile, LogFile);
                else
                    sprintf(OldFile, "%s.%d", LogFile, dindex - 1);

                DBG(2, fCORE)
                DPrint("INFO:     rolling logfile '%s' to '%s'\n", OldFile,
                       NewFile);

                if (rename(OldFile, NewFile) == -1) {
                    DBG(0, fCORE)
                    DPrint(
                        "ERROR:    cannot rename logfile '%s' to '%s' errno: "
                        "%d (%s)\n",
                        OldFile, NewFile, errno, strerror(errno));
                }
            } /* END for (dindex) */
        }

        MLogOpen(Iteration);
    } else {
        DBG(3, fCORE)
        DPrint("INFO:     not rolling logs (%lu < %d)\n", buf.st_size,
               MaxLogFileSize);
    }

    return (SUCCESS);
} /* END MLogRoll() */

char *MLogGetTime()

{
    time_t epoch_time = 0;
    struct tm *present_time;
    static char line[MAX_MLINE];
    static time_t now = 0;

    MUGetTime(&epoch_time, mtmNONE, NULL);

    if (epoch_time != now) {
        now = epoch_time;

        if ((present_time = localtime(&epoch_time)) != NULL) {
            sprintf(line, "%2.2d/%2.2d %2.2d:%2.2d:%2.2d ",
                    present_time->tm_mon + 1, present_time->tm_mday,
                    present_time->tm_hour, present_time->tm_min,
                    present_time->tm_sec);
        } else {
            sprintf(line, "%2.2d/%2.2d %2.2d:%2.2d:%2.2d ", 0, 0, 0, 0, 0);
        }
    } /* END if (epoch_time != now) */

    return (line);
} /* END MLogGetTime() */

void MLogLevelAdjust(

    int signo)

{
    const char *FName = "MLogLevelAdjust";

    DBG(1, fCONFIG) DPrint("%s(%d)\n", FName, signo);

    if (signo == SIGUSR1) {
        if (mlog.Threshold < MAX_MDEBUG) {
            mlog.Threshold++;

            DBG(0, fCONFIG)
            DPrint("INFO:     received signal %d.  increasing LOGLEVEL to %d\n",
                   signo, mlog.Threshold);
        } else {
            DBG(0, fCONFIG)
            DPrint(
                "INFO:     received signal %d.  LOGLEVEL is already at max "
                "value %d\n",
                signo, mlog.Threshold);
        }

        signal(SIGUSR1, MLogLevelAdjust);
    } else if (signo == SIGUSR2) {
        if (mlog.Threshold > 0) {
            mlog.Threshold--;

            DBG(0, fCONFIG)
            DPrint("INFO:     received signal %d.  decreasing LOGLEVEL to %d\n",
                   signo, mlog.Threshold);
        } else {
            DBG(0, fCONFIG)
            DPrint(
                "INFO:     received signal %d.  LOGLEVEL is already at min "
                "value %d\n",
                signo, mlog.Threshold);
        }

        signal(SIGUSR2, MLogLevelAdjust);
    }

    fflush(mlog.logfp);

    return;
} /* END MLogLevelAdjust() */

/* END MLog.c */
