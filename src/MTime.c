/* HEADER */
        
/* Contains:                                 *
 *   int MUStringToE(TimeLine,EpochTime)     *
 *                                           */


#include "moab.h"
#include "msched-proto.h"  

extern mlog_t mlog;

char *MUStrTok(char *Line,char *Delims,char **Ptr);





int MUStringToE(

  char   *TimeLine,
  long   *EpochTime)

  {
  char       Second[MAX_MNAME];
  char       Minute[MAX_MNAME];
  char       Hour[MAX_MNAME];
  char       Day[MAX_MNAME];
  char       Month[MAX_MNAME];
  char       Year[MAX_MNAME];
  char       TZ[MAX_MNAME];

  char       StringTime[MAX_MNAME];
  char       StringDate[MAX_MNAME];
  char       Line[MAX_MLINE];

  char      *ptr;
  char      *tail;

  struct tm  Time;
  struct tm *DefaultTime;

  time_t     ETime;             /* calculated epoch time */
  time_t     Now;

  int        YearVal;

  char      *TokPtr;

  const char *FName = "MUStringToE";

  DBG(2,fCONFIG) DPrint("%s(%s,EpochTime)\n",
    FName,
    TimeLine);

  time(&Now);

  /* check 'NOW' keyword */

  if (!strcmp(TimeLine,"NOW"))
    {
    *EpochTime = (long)Now;
   
    return(SUCCESS);
    }

  /* check 'OFF' keyword */

  if (!strcmp(TimeLine,"OFF"))
    {
    *EpochTime = MAX_MTIME;
  
    return(SUCCESS);
    }

  if ((ptr = strchr(TimeLine,'+')) != NULL)
    {
    /* using relative time */

    /* Format [ +d<DAYS> ][ +h<HOURS> ][ +m<MINUTES> ][ +s<SECONDS> ] */

    ETime = Now + MUTimeFromString(ptr + 1);
    }
  else
    { 
    /* using absolute time */

    /* Format:  HH[:MM[:SS]][_MM[/DD[/YY]]] */

    setlocale(LC_TIME,"en_US.iso88591");

    DefaultTime = localtime(&Now);

    /* copy default values into time structure */

    strcpy(Second,"00");
    strcpy(Minute,"00");
    strftime(Hour  ,MAX_MNAME,"%H",DefaultTime);

    strftime(Day   ,MAX_MNAME,"%d",DefaultTime);
    strftime(Month ,MAX_MNAME,"%m",DefaultTime);
  
    strftime(Year  ,MAX_MNAME,"%Y",DefaultTime);
  
    strftime(TZ    ,MAX_MNAME,"%Z",DefaultTime);

    if ((tail = strchr(TimeLine,'_')) != NULL)
      {
      /* time and date specified */

      strncpy(StringTime,TimeLine,(tail - TimeLine));
      StringTime[(tail - TimeLine)] = '\0';

      strcpy(StringDate,(tail + 1));

      DBG(7,fCONFIG) DPrint("INFO:     time: '%s'  date: '%s'\n",
        StringTime,
        StringDate);

      /* parse date */

      if ((ptr = MUStrTok(StringDate,"/",&TokPtr)) != NULL)
        {
        strcpy(Month,ptr);

        if ((ptr = MUStrTok(NULL,"/",&TokPtr)) != NULL)
          {
          strcpy(Day,ptr);

          if ((ptr = MUStrTok(NULL,"/",&TokPtr)) != NULL)
            {
            YearVal = atoi(ptr);

            if (YearVal < 97)
              {
              sprintf(Year,"%d",
                YearVal + 2000);
              }
            else if (YearVal < 1900) 
              {
              sprintf(Year,"%d",
                YearVal + 1900);
              }
            else 
              {
              sprintf(Year,"%d",
                YearVal);
              }
            }
          }
        }
      }
    else
      {
      strcpy(StringTime,TimeLine);
      }

    /* parse time */

    if ((ptr = MUStrTok(StringTime,":_",&TokPtr)) != NULL)
      {
      strcpy(Hour,ptr);

      if ((ptr = MUStrTok(NULL,":_",&TokPtr)) != NULL)
        {
        strcpy(Minute,ptr);

        if ((ptr = MUStrTok(NULL,":_",&TokPtr)) != NULL)
          strcpy(Second,ptr);
        }
      }

    /* create time string */

    sprintf(Line,"%s:%s:%s %s/%s/%s %s",
      Hour,
      Minute,
      Second,
      Month,
      Day,
      Year,
      TZ);

    /* perform bounds checking */

    if ((atoi(Second) > 59) || 
        (atoi(Minute) > 59) || 
        (atoi(Hour)   > 23) || 
        (atoi(Month)  > 12) || 
        (atoi(Day)    > 31) || 
        (atoi(Year)   > 2097))
      {
      DBG(1,fCONFIG) DPrint("ERROR:    invalid time specified '%s' (bounds exceeded)\n",
        Line);

      return(FAILURE);
      }

    memset(&Time,0,sizeof(Time));

    Time.tm_hour = atoi(Hour);
    Time.tm_min  = atoi(Minute);
    Time.tm_sec  = atoi(Second);
    Time.tm_mon  = atoi(Month) - 1;
    Time.tm_mday = atoi(Day);
    Time.tm_year = atoi(Year) - 1900;

    /* adjust for TZ */

    Time.tm_isdst = -1;

    /* place current time into tm structure */

    DBG(5,fCONFIG) DPrint("INFO:     generated time line: '%s'\n",
      Line);

    /* strptime(Line,"%T %m/%d/%Y %Z",&Time); */

    if ((ETime = mktime(&Time)) == -1)
      {
      DBG(5,fCONFIG) DPrint("ERROR:    cannot determine epoch time for '%s', errno: %d (%s)\n",
        Line,
        errno,
        strerror(errno));

      return(FAILURE);
      }
    }  /* END else (strchr(TimeLine,'+')) */

  DBG(3,fCONFIG) DPrint("INFO:     current   epoch:  %lu  time:  %s\n",
    (unsigned long)Now,
    ctime(&Now));

  DBG(3,fCONFIG) DPrint("INFO:     calculated epoch: %lu  time:  %s\n",
    (unsigned long)ETime,
    ctime(&ETime));

  *EpochTime = (long)ETime;

  return(SUCCESS);
  }  /* END MUStringToE() */




long MUTimeFromString(

  char *TString)

  {
  long  val;

  char *ptr1;
  char *ptr2;
  char *ptr3;
  char *ptr4;

  char *TokPtr;

  char  Line[MAX_MLINE];

  const char *FName = "MUTimeFromString";

  DBG(2,fCONFIG) DPrint("%s(%s)\n",
    FName,
    (TString != NULL) ? TString : "NULL");

  if (TString == NULL)
    return(0);

  if (!strcmp(TString,"INFINITY"))
    return(MAX_MTIME);

  if (strchr(TString,':') == NULL)
    {
    /* line specified as 'raw' seconds */

    val = strtol(TString,NULL,0);

    DBG(4,fCONFIG) DPrint("INFO:     string '%s' specified as seconds\n",
      TString);

    return(val);
    }
  else if (strchr(TString,'_') != NULL)
    {
    /* line specified as 'absolute' time */

    MUStringToE(TString,&val);

    DBG(4,fCONFIG) DPrint("INFO:     string '%s' specified as absolute time\n",
      TString);
 
    return(val);
    }

  /* line specified in 'military' time */

  MUStrCpy(Line,TString,sizeof(Line));

  ptr1 = NULL;
  ptr2 = NULL;
  ptr3 = NULL;
  ptr4 = NULL;

  if ((ptr1 = MUStrTok(Line,":",&TokPtr)) != NULL)
    {
    if ((ptr2 = MUStrTok(NULL,":",&TokPtr)) != NULL)
      {
      if ((ptr3 = MUStrTok(NULL,":",&TokPtr)) != NULL)
        {
        ptr4 = MUStrTok(NULL,":",&TokPtr);
        }
      }
    }

  if (ptr1 == NULL)
    {
    DBG(4,fCONFIG) DPrint("INFO:     cannot read string '%s'\n",
      TString);

    return(0);
    }

  if (ptr4 == NULL)
    {
    /* adjust from HH:MM:SS to DD:HH:MM:SS notation */

    ptr4 = ptr3;
    ptr3 = ptr2;
    ptr2 = ptr1;
    ptr1 = NULL;
    }

  val = (((ptr1 != NULL) ? atoi(ptr1) : 0) * 86400) +
        (((ptr2 != NULL) ? atoi(ptr2) : 0) *  3600) +
        (((ptr3 != NULL) ? atoi(ptr3) : 0) *    60) +
        (((ptr4 != NULL) ? atoi(ptr4) : 0) *     1);

  DBG(4,fCONFIG) DPrint("INFO:     string '%s' -> %ld\n",
    TString,
    val);

  return(val);
  }  /* END MUTimeFromString() */


/* END MTime.c */
