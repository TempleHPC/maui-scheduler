/* CONTRIB:  JobLength.c */

/* code to be included in LocalCheckFairnessPolicy() in Local.c */

# define MAX_LOCAL_JOB_POLICIES 12

/* uses global BQS and Job */

int ContribJobLengthFairnessPolicy(

  job_t *J,
  long   StartTime,
  char  *Message)

  {
  int    jindex;
  int    index;

  char  *buf;

  char  *ptr;

  char   FileName[MAX_NAME];
  int    count;
  int    SC;

  char   TimeString[MAX_LINE];
  char   ParameterName[MAX_NAME];

  static struct {
    long MinLength;
    long MaxLength;
    int  MaxCount;
    int  Count;
    } JobLengthPolicy[MAX_LOCAL_JOB_POLICIES];

  static int LocalPoliciesLoaded = bFALSE;

  static int MaxSmallJobCount;
  static int MaxSmallJobProcs;
  static int MaxSmallJobNodes;

  static int SmallJobCount;

  if (LocalPoliciesLoaded == bFALSE)
    {
    /* initialize policies */

    memset(JobLengthPolicy,0,sizeof(JobLengthPolicy));

    MaxSmallJobCount = 0;
    MaxSmallJobProcs = 0;
    MaxSmallJobNodes = 0;

    LocalPoliciesLoaded = bTRUE;

    /* load local config policy configuration */

    if (!strstr(BQS.ConfigFile,BQS.HomeDir))
      {
      if (BQS.HomeDir[strlen(BQS.HomeDir) - 1] == '/')
        sprintf(FileName,"%s%s",
          BQS.HomeDir,
          BQS.ConfigFile);
      else
        sprintf(FileName,"%s/%s",
          BQS.HomeDir,
          BQS.ConfigFile);
      }
    else
      {
      strcpy(FileName,BQS.ConfigFile);
      }

    if ((buf = LoadFile(FileName,1,FILEREAD,&count,&SC)) == NULL)
      {
      DBG(2,fCONFIG) DPrint("WARNING:  cannot open configuration file '%s' (using internal defaults)\n",
        FileName);

      return(FAILURE);
      }

    AdjustConfigBuffer(buf);

    ptr = buf;

    ConfigGetIntValue(buf,&ptr,"MAXSMALLJOBCOUNT",NULL,NULL,&MaxSmallJobCount,NULL);
    ConfigGetIntValue(buf,&ptr,"MAXSMALLJOBPROCS",NULL,NULL,&MaxSmallJobProcs,NULL);

    for (index = 0;index < MAX_LOCAL_JOB_POLICIES;index++)
      {
      sprintf(ParameterName,"MAXJOB%dCOUNT",
        index + 1);

      if (ConfigGetIntValue(
            buf,
            &ptr,
            ParameterName,
            NULL,
            NULL,
            &JobLengthPolicy[index].MaxCount,
            NULL) == FAILURE)
        {
        continue;
        }

      sprintf(ParameterName,"JOB%dMINLENGTH",
        index + 1);

      if (ConfigGetStringValue(
            buf,
            &ptr,
            ParameterName,
            NULL,
            NULL,
            TimeString,
            NULL) == FAILURE)
        {
        continue;
        }

      JobLengthPolicy[index].MinLength = TimeS2I(TimeString);

      sprintf(ParameterName,"JOB%dMAXLENGTH",
        index + 1);

      if (ConfigGetStringValue(
            buf,
            &ptr,
            ParameterName,
            NULL,
            NULL,
            TimeString,
            NULL) == FAILURE)
        {
        continue;
        }

      JobLengthPolicy[index].MaxLength = TimeS2I(TimeString);

      DBG(4,fSCHED) DPrint("INFO:     local policy joblength[%d] specified:  MaxCount %d  (%ld -> %ld)\n",
        index + 1,
        JobLengthPolicy[index].MaxCount,
        JobLengthPolicy[index].MinLength,
        JobLengthPolicy[index].MaxLength);
      }  /* END for (index) */
    }  /* END if (LocalPoliciesLoaded == bFALSE) */

  if (J == NULL)
    {
    /* initialize local policy counts */

    SmallJobCount = 0;

    for (index = 0;index < MAX_LOCAL_JOB_POLICIES;index++)
      {
      JobLengthPolicy[index].Count = 0;
      }

    /* incorporate active job information */

    for (jindex = Job[0].Next;jindex != 0;jindex = Job[jindex].Next)
      {
      if ((Job[jindex].State != jStarting) &&
          (Job[jindex].State != jRunning))
        continue;

      if ((Job[jindex].TasksRequested * Job[jindex].Req[0]->DRes.Procs) <= MaxSmallJobProcs)
        {
        SmallJobCount++;
        }

      for (index = 0;index < MAX_LOCAL_JOB_POLICIES;index++)
        {
        if (JobLengthPolicy[index].MaxCount <= 0)
          continue;

        if (JobLengthPolicy[index].MinLength <= 0)
          continue;

        if (JobLengthPolicy[index].MaxLength <= 0)
          continue;

        if (Job[jindex].SpecWCLimit[0] < JobLengthPolicy[index].MinLength)
          continue;

        if (Job[jindex].SpecWCLimit[0] > JobLengthPolicy[index].MaxLength)
          continue;

        JobLengthPolicy[index].Count++;
        }  /* END for (index)  */
      }    /* END for (jindex) */

    DBG(8,fSCHED) DPrint("INFO:     local policies initialized\n");

    for (index = 0;index < MAX_LOCAL_JOB_POLICIES;index++)
      {
      DBG(4,fSCHED) DPrint("INFO:     local policy joblength[%d] initialized:  %d of %d jobs%s (%ld -> %ld)\n",
        index + 1,
        JobLengthPolicy[index].Count,
        JobLengthPolicy[index].MaxCount,
        (JobLengthPolicy[index].Count >= JobLengthPolicy[index].MaxCount) ? "*" : "",
        JobLengthPolicy[index].MinLength,
        JobLengthPolicy[index].MaxLength);
      }

    return(SUCCESS);
    }  /* END if (J == NULL) */

  TRAPJOB(J,"LocalCheckFairnessPolicies");

  if ((MaxSmallJobCount > 0) && (MaxSmallJobProcs > 0))
    {
    /* count active small jobs */

    if (SmallJobCount >= MaxSmallJobCount)
      {
      DBG(3,fSCHED) DPrint("INFO:    local policy 'MaxSmallJob' would be violated, rejecting job %s\n",
        J->Name);

      if (Message != NULL)
        {
        }

      return(FAILURE);
      }
    }
  else
    {
    DBG(8,fSCHED) DPrint("INFO:     local policy 'MaxSmallJob' disabled (MaxCount: %d  MaxProcs: %d/MaxNodes: %d)\n",
      MaxSmallJobCount,
      MaxSmallJobProcs,
      MaxSmallJobNodes);
    }

  for (index = 0;index < MAX_LOCAL_JOB_POLICIES;index++)
    {
    if (JobLengthPolicy[index].MaxCount <= 0)
      continue;

    if (JobLengthPolicy[index].MinLength <= 0)
      continue;

    if (JobLengthPolicy[index].MaxLength <= 0)
      continue;

    if (J->SpecWCLimit[0] < JobLengthPolicy[index].MinLength)
      continue;

    if (J->SpecWCLimit[0] > JobLengthPolicy[index].MaxLength)
      continue;

    if (JobLengthPolicy[index].Count < JobLengthPolicy[index].MaxCount)
      {
      JobLengthPolicy[index].Count++;

      break;
      }
    else
      {
      DBG(4,fSCHED) DPrint("INFO:    job %s would violate local policy 'JobLength' (%ld <= %ld <= %ld) MaxCount: %d)\n",
        J->Name,
        JobLengthPolicy[index].MinLength,
        J->SpecWCLimit[0],
        JobLengthPolicy[index].MaxLength,
        JobLengthPolicy[index].MaxCount);

      if (Message != NULL)
        {
        }

      return(FAILURE);
      }
    }   /* END for (index) */

  /* all local policies passed */

  return(SUCCESS);
  }  /* END ContribJobLengthFairnessPolicy() */

/* END JobLength.c */
