/* HEADER */

#include "moab.h"
#include "msched-proto.h"  

extern mlog_t mlog;

extern const char *Service[];

int UIFormatShowAllJobs(

  char *SrcBuffer,
  char *DstBuffer,
  int   FormatMode)

  {
  char  *ptr;
  char   name[MAX_MNAME];
  long   stime;
  long   qtime;
  int    procs;
  long   cpulimit;
  char   tmpQOS[MAX_MNAME];
  int    count;
  int    priority;
  int    state;
 
  long   Now;
 
  char   UserName[MAX_MNAME];
 
  int    UpProcs;
  int    IdleProcs;
 
  int    UpNodes;
  int    IdleNodes;
  int    ActiveNodes;
 
  int    BusyNodes;
  int    BusyProcs;
 
  int    acount;
  int    icount;
  int    ncount;
 
  char   tmp[MAX_MLINE];

  char  *TokPtr;

  switch (FormatMode)
    {
    case mfmHTTP:

      return(UIFormatHShowAllJobs(SrcBuffer,DstBuffer));

      /*NOTREACHED*/

      break;

    default:

      break;
    }  /* END switch(FormatMode) */

  count = 0;
 
  /* get present time */
 
  sscanf(SrcBuffer,"%ld %d %d %d %d %d %d\n",
    &Now,
    &UpProcs,
    &IdleProcs,
    &UpNodes,
    &IdleNodes,
    &ActiveNodes,
    &BusyProcs);
 
  BusyNodes = ActiveNodes;

  BusyProcs = MIN(BusyProcs,UpProcs);
 
  ptr = MUStrTok(SrcBuffer,"\n",&TokPtr);
 
  /* display list of running jobs */
 
  strcpy(DstBuffer,"ACTIVE JOBS--------------------\n");
 
  sprintf(temp_str,"%18s %8s %10s %5s %11s %20s\n\n",
    "JOBNAME",
    "USERNAME",
    "STATE",
    "PROC",
    "REMAINING",
    "STARTTIME");
  strcat(DstBuffer,temp_str);
 
  /* read all running jobs */
 
  acount = 0;
 
  while ((ptr = MUStrTok(NULL,"\n",&TokPtr)) != NULL)
    {
    if (!strcmp(ptr,"[ENDACTIVE]"))
      break; 
 
    acount++;
    count++;
 
    DBG(3,fUI) DPrint("line: '%s'\n",
      ptr);
 
    /* Format:  <JOBNAME> <USERNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <STATE> <PRIO> */
 
    sscanf(ptr,"%s %s %ld %ld %d %ld %s %d %d",
      name,
      UserName,
      &stime,
      &qtime,
      &procs,
      &cpulimit,
      tmpQOS,
      &state,
      &priority);
 
    /* display job */
 
    sprintf(temp_str,"%18s %8s %10s %5d %11s  %19s",
      name,
      UserName,
      MJobState[state],
      procs,
      MULToTString(cpulimit - (Now - stime)),
      MULToDString((mulong *)&stime));
    strcat(DstBuffer,temp_str);
    }  /* END while (ptr) */
 
  sprintf(tmp,"%d Active Job%c   ",
    acount,
    (acount == 1) ? ' ' : 's');
 
  sprintf(temp_str,"\n%21s %4d of %4d Processors Active (%.2f%c)\n",
    tmp, 
    BusyProcs,
    UpProcs,
    (UpProcs > 0) ? (double)BusyProcs / UpProcs * 100.0 : 0.0,
    '%');
  strcat(DstBuffer,temp_str);
 
  if (UpProcs != UpNodes)
    {
    sprintf(temp_str,"%21s %4d of %4d Nodes Active      (%.2f%c)\n",
      " ",
      BusyNodes,
      UpNodes,
      (UpNodes > 0) ? (double)BusyNodes / UpNodes * 100.0 : 0.0,
      '%');
    strcat(DstBuffer,temp_str);
    }
 
  /* display list of idle jobs */
 
  strcat(DstBuffer,"\nIDLE JOBS----------------------\n");
 
  sprintf(temp_str,"%18s %8s %10s %5s %11s %20s\n\n",
    "JOBNAME",
    "USERNAME",
    "STATE",
    "PROC",
    "WCLIMIT",
    "QUEUETIME");
  strcat(DstBuffer,temp_str);
 
  /* read all idle jobs */
 
  icount = 0;
 
  while ((ptr = MUStrTok(NULL,"\n",&TokPtr)) != NULL)
    {
    if (!strcmp(ptr,"[ENDIDLE]"))
      break; 
 
    count++;
    icount++;
 
    DBG(3,fUI) DPrint("line: '%s'\n",
      ptr);
 
    /* Format:  <JOBNAME> <USERNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <STATE> <PRIO> */
 
    sscanf(ptr,"%s %s %ld %ld %d %ld %s %d %d",
      name,
      UserName,
      &stime,
      &qtime,
      &procs,
      &cpulimit,
      tmpQOS,
      &state,
      &priority);
 
    /* display job */
 
    fprintf(stdout,"%18s %8s %10s %5d %11s  %19s",
      name,
      UserName,
      MJobState[state],
      procs,
      MULToTString(cpulimit),
      MULToDString((mulong *)&qtime));
    }
 
  fprintf(stdout,"\n%d Idle Job%c\n",
    icount,
    (icount == 1) ? ' ' : 's');
 
  /* display list of non-queued jobs */ 
 
  strcat(DstBuffer,"\nBLOCKED JOBS-------------------\n");
 
  sprintf(temp_str,"%18s %8s %10s %5s %11s %20s\n\n",
    "JOBNAME",
    "USERNAME",
    "STATE",
    "PROC",
    "WCLIMIT",
    "QUEUETIME");
  strcat(DstBuffer,temp_str);
 
  /* read all non-queued jobs */
 
  ncount = 0;
 
  while ((ptr = MUStrTok(NULL,"\n",&TokPtr)) != NULL)
    {
    if (!strcmp(ptr,"[ENDNOTQUEUED]"))
      {
      break;
      }
 
    count++;
    ncount++;
 
    DBG(3,fUI) DPrint("line: '%s'\n",
      ptr);
 
    /* Format:  <JOBNAME> <USERNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <STATE> <PRIORITY>  */
 
    sscanf(ptr,"%s %s %ld %ld %d %ld %s %d %d",
      name,
      UserName,
      &stime,
      &qtime,
      &procs,
      &cpulimit, 
      tmpQOS,
      &state,
      &priority);
 
    /* display job */
 
    sprintf(temp_str,"%18s %8s %10s %5d %11s  %19s",
      name,
      UserName,
      MJobState[state],
      procs,
      MULToTString(cpulimit),
      MULToDString((mulong *)&qtime));
    strcat(DstBuffer,temp_str);
    }  /* END while (ptr) */
 
  sprintf(temp_str,"\nTotal Jobs: %d   Active Jobs: %d   Idle Jobs: %d   Non-Queued Jobs: %d\n",
    count,
    acount,
    icount,
    ncount);
  strcat(DstBuffer,temp_str);
 
  while ((ptr = MUStrTok(NULL,"\n",&TokPtr)) != NULL)
    {
    sprintf(temp_str,"\n%s\n",
      ptr);
    strcat(DstBuffer,temp_str);
    }
 
  return(SUCCESS);
  }  /* END UIFormatShowAllJobs() */





int UIFormatHShowAllJobs(
 
  char *SrcBuffer,
  char *DstBuffer)
 
  {
  char  *ptr;
  char   name[MAX_MNAME];
  long   stime;
  long   qtime;
  int    procs;
  long   cpulimit;
  char   tmpQOS[MAX_MNAME];
  int    count;
  int    priority;
  int    state;
 
  long   Now;
 
  char   UserName[MAX_MNAME];
 
  int    UpProcs;
  int    IdleProcs;
 
  int    UpNodes;
  int    IdleNodes;
  int    ActiveNodes;
 
  int    BusyNodes;
  int    BusyProcs;
 
  int    acount;
  int    icount;
  int    ncount;
 
  char  *TokPtr;
 
  count = 0;
 
  /* get present time */
 
  sscanf(SrcBuffer,"%ld %d %d %d %d %d %d\n",
    &Now,
    &UpProcs,
    &IdleProcs,
    &UpNodes,
    &IdleNodes,
    &ActiveNodes,
    &BusyProcs);
 
  BusyNodes = ActiveNodes;
 
  /* display list of running jobs */
 
  DstBuffer[0] = '\0';

  if (strlen(SrcBuffer) > 1000)
    strcat(DstBuffer,"<FONT SIZE=-1>");
  else
    strcat(DstBuffer,"<FONT>");

  ptr = MUStrTok(SrcBuffer,"\n",&TokPtr);   

  /* display active jobs */
 
  acount = 0;
 
  while ((ptr = MUStrTok(NULL,"\n",&TokPtr)) != NULL)
    {
    if (!strcmp(ptr,"[ENDACTIVE]"))
      break;

    if (acount == 0)
      {
      strcat(DstBuffer,"<TABLE BORDER=1>");
 
      sprintf(temp_str,"<TR><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD></TR>",
        "Active Jobs",
        "User Name",
        "Job State",
        "Processors",
        "Time Remaining",
        "Start Time");
      strcat(DstBuffer,temp_str);
      }
 
    acount++;
    count++;
 
    DBG(3,fUI) DPrint("line: '%s'\n",
      ptr);
 
    /* Format:  <JOBNAME> <USERNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <STATE> <PRIO> */
 
    sscanf(ptr,"%s %s %ld %ld %d %ld %s %d %d",
      name,
      UserName,
      &stime,
      &qtime, 
      &procs,
      &cpulimit,
      tmpQOS,
      &state,
      &priority);
 
    /* display job */

    sprintf(temp_str,"<TR><TD>%s</TD><TD>%s</TD><TD>%s</TD><TD>%d</TD><TD>%s</TD><TD>%s</TD></TR>",
      name,
      UserName,
      MJobState[state],
      procs,
      MULToTString(cpulimit - (Now - stime)),
      MULToDString((mulong *)&stime));
    strcat(DstBuffer,temp_str);
    }  /* END while (ptr) */

  if (acount > 0)
    {
    strcat(DstBuffer,"</TABLE><p>");
 
    sprintf(temp_str,"%d Active Job%c<br>",
      acount,
      (acount == 1) ? ' ' : 's');
    strcat(DstBuffer,temp_str);
 
    sprintf(temp_str,"%d of %d Processors Active (%.2f%c)<br>",
      BusyProcs,
      UpProcs,
      (UpProcs > 0) ? (double)BusyProcs / UpProcs * 100.0 : 0.0,
      '%');
    strcat(DstBuffer,temp_str);
 
    if (UpProcs != UpNodes)
      {
      sprintf(temp_str,"%d of %d Nodes Active (%.2f%c)<br>",
        BusyNodes,
        UpNodes,
        (UpNodes > 0) ? (double)BusyNodes / UpNodes * 100.0 : 0.0,
        '%');
      strcat(DstBuffer,temp_str);
      }

    strcat(DstBuffer,"<p>");
    }
 
  /* display idle jobs */
 
  icount = 0;
 
  while ((ptr = MUStrTok(NULL,"\n",&TokPtr)) != NULL)
    {
    if (!strcmp(ptr,"[ENDIDLE]"))
      break;

    if (icount == 0)
      {
      strcat(DstBuffer,"<TABLE BORDER=1>");

      sprintf(temp_str,"<TR><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD></TR>",
        "Idle Jobs",
        "User Name",
        "Job State",
        "Processors",
        "WallClock Limit",
        "Submission Time");
      strcat(DstBuffer,temp_str);
      }
  
    count++;
    icount++;
 
    DBG(3,fUI) DPrint("line: '%s'\n",
      ptr);
 
    /* Format:  <JOBNAME> <USERNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <STATE> <PRIO> */
 
    sscanf(ptr,"%s %s %ld %ld %d %ld %s %d %d",
      name,
      UserName,
      &stime,
      &qtime,
      &procs,
      &cpulimit,
      tmpQOS,
      &state,
      &priority);
 
    /* display job */

    sprintf(temp_str,"<TR><TD>%s</TD><TD>%s</TD><TD>%s</TD><TD>%d</TD><TD>%s</TD><TD>%s</TD></TR>",
      name,
      UserName,
      MJobState[state],
      procs,
      MULToTString(cpulimit),
      MULToDString((mulong *)&qtime));
    strcat(DstBuffer,temp_str);
    }  /* END while (ptr) */

  if (icount > 0)
    {
    strcat(DstBuffer,"</TABLE><p>"); 

    sprintf(temp_str,"%d Idle Job%c<p>",
      icount,
      (icount == 1) ? ' ' : 's');
    strcat(DstBuffer,temp_str);
    }

  /* display ineligible jobs */

  ncount = 0;
 
  while ((ptr = MUStrTok(NULL,"\n",&TokPtr)) != NULL)
    {
    if (!strcmp(ptr,"[ENDNOTQUEUED]"))
      {
      break;
      }

    if (ncount == 0)
      {
      strcat(DstBuffer,"<TABLE BORDER=1>");

      sprintf(temp_str,"<TR><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD><TD><B>%s</B></TD></TR>",
        "Ineligible Jobs",
        "User Name",
        "Job State",
        "Processors",
        "WallClock Limit",
        "Submission Time",
        "Reason");
      strcat(DstBuffer,temp_str);
      }
 
    count++;
    ncount++;
 
    DBG(3,fUI) DPrint("line: '%s'\n",
      ptr);
 
    /* Format:  <JOBNAME> <USERNAME> <START TIME> <QUEUE TIME> <PROCS> <CPULIMIT> <QOS> <STATE> <PRIORITY>  */
 
    sscanf(ptr,"%s %s %ld %ld %d %ld %s %d %d",
      name,
      UserName,
      &stime,
      &qtime,
      &procs,
      &cpulimit,
      tmpQOS,
      &state,
      &priority);
 
    /* display job */

   sprintf(temp_str,"<TR><TD>%s</TD><TD>%s</TD><TD>%s</TD><TD>%d</TD><TD>%s</TD><TD>%s</TD><TD>%s</TD></TR>",
      name,
      UserName,
      MJobState[state],
      procs,
      MULToTString(cpulimit),
      MULToDString((mulong *)&qtime),
      "N/A");
   strcat(DstBuffer,temp_str);
    }  /* END while (ptr) */

  if (ncount > 0)
    {
    strcat(DstBuffer,"</TABLE><p>");     
    }
 
  sprintf(temp_str,"<p>Total Jobs: %d &nbsp; Active Jobs: %d &nbsp; Idle Jobs: %d &nbsp; Ineligible Jobs: %d<p>",
    count,
    acount,
    icount,
    ncount);
  strcat(DstBuffer,temp_str);
 
  while ((ptr = MUStrTok(NULL,"\n",&TokPtr)) != NULL)
    {
    sprintf(temp_str,"<p>%s<p>",
      ptr);
    strcat(DstBuffer,temp_str);
    }

  strcat(DstBuffer,"</FONT>");         
 
  return(SUCCESS);
  }  /* END UIFormatHShowAllJobs() */

/* END MUI.c */

